////////////////////////////////////////////////////////////////////////////////////////
//
// Nestopia - NES/Famicom emulator written in C++
//
// Copyright (C) 2003-2008 Martin Freij
//
// This file is part of Nestopia.
//
// Nestopia is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Nestopia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Nestopia; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
////////////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>
#include "NstMachine.hpp"
#include "NstState.hpp"
#include "NstTrackerRewinder.hpp"
#include "api/NstApiRewinder.hpp"
#include "NstZlib.hpp"

namespace Nes
{
	namespace Core
	{
		class Tracker::Rewinder::ReverseVideo::Mutex
		{
			Ppu& ppu;
			Video::Screen::Pixel* const pixels;

		public:

			explicit Mutex(const ReverseVideo& r)
			: ppu(r.ppu), pixels(r.ppu.GetOutputPixels()) {}

			void Flush(Video::Screen::Pixel* src) const
			{
				std::memcpy( pixels, src, Video::Screen::PIXELS * sizeof(Video::Screen::Pixel) );
			}

			~Mutex()
			{
				ppu.SetOutputPixels( pixels );
			}
		};

		class Tracker::Rewinder::ReverseVideo::Buffer
		{
			typedef Video::Screen::Pixel Pixel;

			enum
			{
				PIXELS    = Video::Screen::PIXELS,
				SIZE      = PIXELS * dword(NUM_FRAMES),
				FULL_SIZE = dword(SIZE) + Video::Screen::PIXELS_PADDING
			};

			Pixel pixels[FULL_SIZE];

		public:

			Buffer()
			{
				std::fill( pixels + SIZE, pixels + FULL_SIZE, Pixel(0) );
			}

			Pixel* operator [] (dword i)
			{
				NST_ASSERT( i < NUM_FRAMES );
				return pixels + (PIXELS * i);
			}
		};

		class Tracker::Rewinder::ReverseSound::Mutex
		{
			Output::LockCallback funcLock;
			void* userLock;
			Output::UnlockCallback funcUnlock;
			void* userUnlock;

		public:

			Mutex()
			{
				Output::lockCallback.Get( funcLock, userLock );
				Output::unlockCallback.Get( funcUnlock, userUnlock );
				Output::lockCallback.Set( NULL, NULL );
				Output::unlockCallback.Set( NULL, NULL );
			}

			bool Lock(Output& output) const
			{
				return funcLock ? funcLock( userLock, output ) : true;
			}

			void Unlock(Output& output) const
			{
				if (funcUnlock)
					funcUnlock( userUnlock, output );
			}

			~Mutex()
			{
				Output::lockCallback.Set( funcLock, userLock );
				Output::unlockCallback.Set( funcUnlock, userUnlock );
			}
		};

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Tracker::Rewinder::ReverseVideo::ReverseVideo(Ppu& p)
		:
		pingpong (1),
		frame    (0),
		ppu      (p),
		buffer   (NULL)
		{}

		Tracker::Rewinder::ReverseSound::ReverseSound(const Apu& a,bool e)
		:
		enabled (e),
		good    (false),
		stereo  (false),
		bits    (0),
		rate    (0),
		index   (0),
		buffer  (NULL),
		size    (0),
		input   (NULL),
		apu     (a)
		{}

		Tracker::Rewinder::Rewinder(Machine& e,EmuExecute x,EmuLoadState l,EmuSaveState s,Cpu& c,const Apu& a,Ppu& p,bool b)
		:
		rewinding    (false),
		sound        (a,b),
		video        (p),
		emulator     (e),
		emuExecute   (x),
		emuLoadState (l),
		emuSaveState (s),
		cpu          (c),
		ppu          (p)
		{
			Reset( true );
		}

		Tracker::Rewinder::ReverseVideo::~ReverseVideo()
		{
			End();
		}

		Tracker::Rewinder::ReverseSound::~ReverseSound()
		{
			End();
		}

		Tracker::Rewinder::~Rewinder()
		{
			LinkPorts( false );
		}

		void Tracker::Rewinder::LinkPorts(bool on)
		{
			for (uint i=0; i < 2; ++i)
			{
				cpu.Unlink( 0x4016+i, this, &Rewinder::Peek_Port_Get, &Rewinder::Poke_Port );
				cpu.Unlink( 0x4016+i, this, &Rewinder::Peek_Port_Put, &Rewinder::Poke_Port );
			}

			if (on)
			{
				for (uint i=0; i < 2; ++i)
					ports[i] = cpu.Link( 0x4016+i, Cpu::LEVEL_HIGHEST, this, rewinding ? &Rewinder::Peek_Port_Get : &Rewinder::Peek_Port_Put, &Rewinder::Poke_Port );
			}
		}

		Tracker::Rewinder::Key::Key()
		{
		}

		Tracker::Rewinder::Key::~Key()
		{
		}

		void Tracker::Rewinder::Key::Input::Reset()
		{
			pos = BAD_POS;
			buffer.Destroy();
		}

		void Tracker::Rewinder::Key::Reset()
		{
			stream.str( std::string() );
			input.Reset();
		}

		void Tracker::Rewinder::Reset(bool on)
		{
			video.End();
			sound.End();

			if (rewinding)
			{
				rewinding = false;
				Api::Rewinder::stateCallback( Api::Rewinder::STOPPED );
			}

			uturn = false;
			frame = LAST_FRAME;
			key = keys + LAST_KEY;

			for (uint i=0; i < NUM_FRAMES; ++i)
				keys[i].Reset();

			LinkPorts( on );
		}

		void Tracker::Rewinder::ReverseVideo::Begin()
		{
			pingpong = 1;
			frame = 0;

			if (buffer == NULL)
				buffer = new Buffer;
		}

		void Tracker::Rewinder::ReverseVideo::End()
		{
			delete buffer;
			buffer = NULL;
		}

		void Tracker::Rewinder::ReverseSound::Begin()
		{
			good = true;
			index = 0;
		}

		void Tracker::Rewinder::ReverseSound::End()
		{
			std::free( buffer );
			buffer = NULL;
		}

		void Tracker::Rewinder::ReverseSound::Enable(bool state)
		{
			enabled = state;

			if (!state)
				End();
		}

		bool Tracker::Rewinder::ReverseSound::Update()
		{
			const dword old = (bits == 16 ? size * sizeof(iword) : size * sizeof(byte));

			bits = apu.GetSampleBits();
			rate = apu.GetSampleRate();
			stereo = apu.InStereo();
			size = rate << (stereo+1);

			const dword total = (bits == 16 ? size * sizeof(iword) : size * sizeof(byte));
			NST_ASSERT( total );

			if (!buffer || total != old)
			{
				if (void* const next = std::realloc( buffer, total ))
				{
					buffer = next;
				}
				else
				{
					End();

					good = false;
					return false;
				}
			}

			good = true;
			index = 0;

			if (bits == 16)
				std::fill( static_cast<iword*>(buffer), static_cast<iword*>(buffer) + size, iword(0) );
			else
				std::memset( buffer, 0x80, size );

			return true;
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		inline void Tracker::Rewinder::Key::Input::Invalidate()
		{
			pos = BAD_POS;
		}

		inline void Tracker::Rewinder::Key::Input::BeginForward()
		{
			const dword hint = pos;
			pos = 0;
			buffer.Clear();

			if (hint != BAD_POS)
				buffer.Reserve( hint );
		}

		bool Tracker::Rewinder::Key::Input::EndForward()
		{
			if (pos == 0)
			{
				pos = buffer.Size();

				if (Zlib::AVAILABLE && pos >= MIN_COMPRESSION_SIZE)
				{
					Buffer tmp( pos - 1 );

					if (const dword size = Zlib::Compress( buffer.Begin(), buffer.Size(), tmp.Begin(), tmp.Size(), Zlib::NORMAL_COMPRESSION ))
					{
						NST_ASSERT( size < pos );
						tmp.SetTo( size );
						Buffer::Swap( tmp, buffer );
					}
					else
					{
						NST_DEBUG_MSG("compress() in Tracker::Rewinder::Key::Input failed!");
					}

					buffer.Defrag();
				}

				return true;
			}

			return false;
		}

		void Tracker::Rewinder::Key::Input::BeginBackward()
		{
			dword size = pos;
			pos = 0;

			if (Zlib::AVAILABLE && size > buffer.Size())
			{
				Buffer tmp( size );
				size = Zlib::Uncompress( buffer.Begin(), buffer.Size(), tmp.Begin(), tmp.Size() );

				if (!size)
					throw RESULT_ERR_CORRUPT_FILE;

				NST_VERIFY( size == tmp.Size() );
				Buffer::Swap( tmp, buffer );
			}
		}

		inline void Tracker::Rewinder::Key::Input::EndBackward()
		{
			pos = 0;
		}

		inline void Tracker::Rewinder::Key::Input::ResumeForward()
		{
			NST_VERIFY( pos != BAD_POS );
			dword size = pos;
			pos = 0;
			buffer.Resize( size != BAD_POS ? size : 0 );
		}

		inline bool Tracker::Rewinder::Key::Input::CanRewind() const
		{
			return pos != BAD_POS;
		}

		inline uint Tracker::Rewinder::Key::Input::Put(const uint data)
		{
			if (pos != BAD_POS)
			{
				try
				{
					buffer.Append( data );
				}
				catch (...)
				{
					NST_DEBUG_MSG("buffer << data failed!");
					pos = BAD_POS;
				}
			}

			return data;
		}

		inline uint Tracker::Rewinder::Key::Input::Get()
		{
			if (pos < buffer.Size())
			{
				return buffer[pos++];
			}
			else
			{
				NST_DEBUG_MSG("buffer >> data failed!");
				pos = BAD_POS;
				return OPEN_BUS;
			}
		}

		inline void Tracker::Rewinder::Key::Invalidate()
		{
			input.Invalidate();
		}

		inline bool Tracker::Rewinder::Key::CanRewind() const
		{
			return input.CanRewind();
		}

		inline void Tracker::Rewinder::Key::ResumeForward()
		{
			input.ResumeForward();
		}

		void Tracker::Rewinder::Key::BeginForward(Machine& emulator,EmuSaveState saveState,EmuLoadState loadState)
		{
			NST_ASSERT( !saveState || !loadState );

			input.BeginForward();

			if (saveState)
			{
				stream.clear();
				stream.seekp( 0, std::stringstream::beg );
				stream.clear();

				State::Saver saver( &static_cast<std::ostream&>(stream), false, true );
				(emulator.*saveState)( saver );
			}
			else if (loadState)
			{
				TurnForward( emulator, loadState );
			}
		}

		void Tracker::Rewinder::Key::EndForward()
		{
			if (!input.EndForward())
				Reset();
		}

		void Tracker::Rewinder::Key::TurnForward(Machine& emulator,EmuLoadState loadState)
		{
			stream.clear();
			stream.seekg( 0, std::stringstream::beg );
			stream.clear();

			State::Loader loader( &static_cast<std::istream&>(stream), false );
			(emulator.*loadState)( loader, true );
		}

		void Tracker::Rewinder::Key::BeginBackward(Machine& emulator,EmuLoadState loadState)
		{
			NST_VERIFY( CanRewind() );

			TurnForward( emulator, loadState );
			input.BeginBackward();
		}

		inline void Tracker::Rewinder::Key::EndBackward()
		{
			input.EndBackward();
		}

		inline uint Tracker::Rewinder::Key::Put(uint data)
		{
			return input.Put( data );
		}

		inline uint Tracker::Rewinder::Key::Get()
		{
			return input.Get();
		}

		inline Tracker::Rewinder::Key* Tracker::Rewinder::PrevKey(Key* k)
		{
			return (k != keys ? k-1 : keys+LAST_KEY);
		}

		inline Tracker::Rewinder::Key* Tracker::Rewinder::PrevKey()
		{
			return PrevKey( key );
		}

		inline Tracker::Rewinder::Key* Tracker::Rewinder::NextKey(Key* k)
		{
			return (k != keys+LAST_KEY ? k+1 : keys);
		}

		inline Tracker::Rewinder::Key* Tracker::Rewinder::NextKey()
		{
			return NextKey( key );
		}

		inline void Tracker::Rewinder::ReverseVideo::Flush(const Mutex& mutex)
		{
			mutex.Flush( (*buffer)[frame] );
		}

		void Tracker::Rewinder::ReverseVideo::Store()
		{
			NST_ASSERT( frame < NUM_FRAMES && (pingpong == 1U-0U || pingpong == 0U-1U) );

			ppu.SetOutputPixels( (*buffer)[frame] );
			frame += pingpong;

			if (frame == NUM_FRAMES)
			{
				frame = LAST_FRAME;
				pingpong = 0U-1U;
			}
			else if (frame == 0U-1U)
			{
				frame = 0;
				pingpong = 1U-0U;
			}
		}

		template<typename T>
		NST_FORCE_INLINE Sound::Output* Tracker::Rewinder::ReverseSound::StoreType()
		{
			NST_ASSERT( index <= NUM_FRAMES+LAST_FRAME );

			switch (index++)
			{
				case 0:

					*output.length = rate / NUM_FRAMES;
					*output.samples = buffer;
					input = static_cast<T*>(buffer) + (size / 1);
					break;

				case LAST_FRAME:

					*output.samples = static_cast<T*>(*output.samples) + (*output.length << stereo);
					*output.length = dword(static_cast<T*>(buffer) + (size / 2) - static_cast<T*>(*output.samples)) >> stereo;
					break;

				case NUM_FRAMES:

					*output.length = rate / NUM_FRAMES;
					*output.samples = static_cast<T*>(buffer) + (size / 2);
					input = *output.samples;
					break;

				case NUM_FRAMES+LAST_FRAME:

					index = 0;
					*output.samples = static_cast<T*>(*output.samples) + (*output.length << stereo);
					*output.length = dword(static_cast<T*>(buffer) + (size / 1) - static_cast<T*>(*output.samples)) >> stereo;
					break;

				default:

					*output.samples = static_cast<T*>(*output.samples) + (*output.length << stereo);
					break;
			}

			return &output;
		}

		Sound::Output* Tracker::Rewinder::ReverseSound::Store()
		{
			NST_COMPILE_ASSERT( NUM_FRAMES % 2 == 0 );

			if (!buffer || (bits ^ apu.GetSampleBits()) | (rate ^ apu.GetSampleRate()) | (stereo ^ uint(bool(apu.InStereo()))))
			{
				if (!enabled || !good || !Update())
					return NULL;
			}

			return bits == 16 ? StoreType<iword>() : StoreType<byte>();
		}

		template<typename T,int SILENCE>
		void Tracker::Rewinder::ReverseSound::ReverseSilence(const Output& target) const
		{
			for (uint i=0; i < 2; ++i)
				std::fill( static_cast<T*>(target.samples[i]), static_cast<T*>(target.samples[i]) + (target.length[i] << stereo), SILENCE );
		}

		template<typename T>
		const void* Tracker::Rewinder::ReverseSound::ReverseCopy(const Output& target) const
		{
			const T* NST_RESTRICT src = static_cast<const T*>(input);

			for (uint i=0; i < 2; ++i)
			{
				if (const dword length = (target.length[i] << stereo))
				{
					T* NST_RESTRICT dst = static_cast<T*>(target.samples[i]);
					T* const dstEnd = dst + length;

					for (const T* const srcEnd = dword(src - static_cast<const T*>(buffer)) >= length ? src - length : static_cast<const T*>(buffer); src != srcEnd; )
						*dst++ = *--src;

					const T last( *src );
					std::fill( dst, dstEnd, last );
				}
			}

			return src;
		}

		void Tracker::Rewinder::ReverseSound::Flush(Output* const target,const Mutex& mutex)
		{
			if (target && mutex.Lock( *target ))
			{
				if (enabled & good)
				{
					input = (bits == 16) ? ReverseCopy<iword>( *target ) : ReverseCopy<byte>( *target );
				}
				else
				{
					if (bits == 16)
						ReverseSilence<iword,0>( *target );
					else
						ReverseSilence<byte,0x80>( *target );
				}

				mutex.Unlock( *target );
			}
		}

		void Tracker::Rewinder::Execute(Video::Output* videoOut,Sound::Output* soundOut,Input::Controllers* inputOut)
		{
			try
			{
				if (uturn)
					ChangeDirection();

				NST_ASSERT( frame < NUM_FRAMES );

				if (!rewinding)
				{
					if (++frame == NUM_FRAMES)
					{
						frame = 0;
						key->EndForward();
						key = NextKey();
						key->BeginForward( emulator, emuSaveState, NULL );
					}
				}
				else
				{
					if (++frame == NUM_FRAMES)
					{
						frame = 0;
						key->EndBackward();

						Key* const prev = PrevKey();

						if (prev->CanRewind())
						{
							prev->BeginBackward( emulator, emuLoadState );
							key = prev;
						}
						else
						{
							rewinding = false;

							key->Invalidate();
							key = NextKey();
							key->BeginForward( emulator, NULL, emuLoadState );

							Api::Rewinder::stateCallback( Api::Rewinder::STOPPED );

							LinkPorts();
						}
					}

					if (rewinding)
					{
						const ReverseVideo::Mutex videoMutex( video );
						video.Flush( videoMutex );
						video.Store();

						const ReverseSound::Mutex soundMutex;
						sound.Flush( soundOut, soundMutex );
						soundOut = sound.Store();

						(emulator.*emuExecute)( videoOut, soundOut, inputOut );
						return;
					}
				}
			}
			catch (...)
			{
				Reset();
				throw;
			}

			(emulator.*emuExecute)( videoOut, soundOut, inputOut );
		}

		void Tracker::Rewinder::ChangeDirection()
		{
			Api::Rewinder::stateCallback( Api::Rewinder::PREPARING );

			uturn = false;

			if (rewinding)
			{
				for (uint i=frame; i < LAST_FRAME; ++i)
					(emulator.*emuExecute)( NULL, NULL, NULL );

				NextKey()->Invalidate();

				video.Begin();
				sound.Begin();

				key->BeginBackward( emulator, emuLoadState );
				LinkPorts();

				{
					const ReverseVideo::Mutex videoMutex( video );
					const ReverseSound::Mutex soundMutex;

					for (uint i=0; i < NUM_FRAMES; ++i)
					{
						video.Store();
						(emulator.*emuExecute)( NULL, sound.Store(), NULL );
					}
				}

				uint align = LAST_FRAME - frame;
				frame = LAST_FRAME;

				while (align--)
				{
					Execute( NULL, NULL, NULL );

					if (!rewinding)
						throw RESULT_ERR_CORRUPT_FILE;
				}

				Api::Rewinder::stateCallback( Api::Rewinder::REWINDING );
			}
			else
			{
				for (uint i=NUM_FRAMES+LAST_FRAME-frame*2; i; --i)
				{
					if (++frame == NUM_FRAMES)
					{
						frame = 0;
						key = NextKey();
						key->TurnForward( emulator, emuLoadState );
					}

					(emulator.*emuExecute)( NULL, NULL, NULL );
				}

				key->ResumeForward();

				LinkPorts();

				video.End();
				sound.End();

				Api::Rewinder::stateCallback( Api::Rewinder::STOPPED );
			}
		}

		Result Tracker::Rewinder::Start()
		{
			if (rewinding)
				return RESULT_NOP;

			if (uturn || !PrevKey()->CanRewind())
				return RESULT_ERR_NOT_READY;

			uturn = true;
			rewinding = true;

			return RESULT_OK;
		}

		Result Tracker::Rewinder::Stop()
		{
			if (!rewinding)
				return RESULT_NOP;

			if (uturn)
				return RESULT_ERR_NOT_READY;

			uturn = true;
			rewinding = false;

			return RESULT_OK;
		}

		NES_PEEK_A(Tracker::Rewinder,Port_Put)
		{
			return key->Put( ports[address-0x4016]->Peek( address ) );
		}

		NES_PEEK(Tracker::Rewinder,Port_Get)
		{
			return key->Get();
		}

		NES_POKE_AD(Tracker::Rewinder,Port)
		{
			ports[address-0x4016]->Poke( address, data );
		}
	}
}
