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

#include "NstInpDevice.hpp"
#include "NstInpFamilyKeyboard.hpp"
#include "../NstCpu.hpp"
#include "../NstHook.hpp"
#include "../NstFile.hpp"
#include "../api/NstApiTapeRecorder.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			class FamilyKeyboard::DataRecorder
			{
			public:

				Result Stop();
				void   EndFrame();
				void   SaveState(State::Saver&,dword) const;
				void   LoadState(State::Loader&);

			private:

				void Start();

				NES_DECL_HOOK( Tape );

				enum
				{
					MAX_LENGTH = SIZE_4096K,
					CLOCK = 32000
				};

				enum Status
				{
					STOPPED,
					PLAYING,
					RECORDING
				};

				Cpu& cpu;
				Cycle cycles;
				Status status;
				Vector<byte> stream;
				dword pos;
				uint in;
				uint out;
				File file;

				static const dword clocks[2][2];

			public:

				DataRecorder(Cpu& c)
				: cpu(c), cycles(Cpu::CYCLE_MAX), status(STOPPED), pos(0), in(0), out(0)
				{
					NST_COMPILE_ASSERT( CPU_RP2A03 == 0 && CPU_RP2A07 == 1 );
					file.Load( File::TAPE, stream, MAX_LENGTH );
				}

				~DataRecorder()
				{
					Stop();
					cpu.RemoveHook( Hook(this,&DataRecorder::Hook_Tape) );

					if (stream.Size())
						file.Save( File::TAPE, stream.Begin(), stream.Size() );
				}

				void Reset()
				{
					Stop();
					cycles = Cpu::CYCLE_MAX;
				}

				bool IsStopped() const
				{
					return status == STOPPED;
				}

				bool IsRecording() const
				{
					return status == RECORDING;
				}

				bool IsPlaying() const
				{
					return status == PLAYING;
				}

				bool Playable() const
				{
					return stream.Size();
				}

				Result Record()
				{
					if (status == RECORDING)
						return RESULT_NOP;

					if (status == PLAYING)
						return RESULT_ERR_NOT_READY;

					status = RECORDING;
					in = 0;
					out = 0;
					cycles = 0;
					stream.Destroy();

					Start();

					return RESULT_OK;
				}

				Result Play()
				{
					if (status == PLAYING)
						return RESULT_NOP;

					if (status == RECORDING || !Playable())
						return RESULT_ERR_NOT_READY;

					status = PLAYING;
					pos = 0;
					in = 0;
					out = 0;
					cycles = 0;

					Start();

					return RESULT_OK;
				}

				void Poke(uint data)
				{
					out = data;
				}

				uint Peek() const
				{
					return in;
				}
			};

			const dword FamilyKeyboard::DataRecorder::clocks[2][2] =
			{
				{ CLK_NTSC_DIV * 16, CLK_NTSC * (16UL/1) / (CLOCK/1)   },
				{ CLK_PAL_DIV * 320, CLK_PAL * (320UL/16) / (CLOCK/16) }
			};

			FamilyKeyboard::FamilyKeyboard(Cpu& c,bool connectDataRecorder)
			:
			Device       (c,Api::Input::FAMILYKEYBOARD),
			dataRecorder (connectDataRecorder ? new DataRecorder(c) : NULL)
			{
				FamilyKeyboard::Reset();
			}

			FamilyKeyboard::~FamilyKeyboard()
			{
				delete dataRecorder;
			}

			void FamilyKeyboard::Reset()
			{
				scan = 0;
				mode = 0;

				if (dataRecorder)
					dataRecorder->Reset();
			}

			void FamilyKeyboard::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'F','B'>::R(0,0,id) );
				saver.Begin( AsciiId<'K','B','D'>::V ).Write8( mode | (scan << 1) ).End();

				if (dataRecorder)
					dataRecorder->SaveState( saver, AsciiId<'D','T','R'>::V );

				saver.End();
			}

			void FamilyKeyboard::DataRecorder::SaveState(State::Saver& state,const dword baseChunk) const
			{
				if (stream.Size())
				{
					state.Begin( baseChunk );

					if (status != STOPPED)
					{
						const dword p = (status == PLAYING ? pos : 0);
						Cycle c = cycles / clocks[cpu.GetModel()][0];

						if (c > cpu.GetCycles())
							c -= cpu.GetCycles();
						else
							c = 0;

						c /= cpu.GetClock();

						const byte data[] =
						{
							status,
							in,
							out,
							p >>  0 & 0xFF,
							p >>  8 & 0xFF,
							p >> 16 & 0xFF,
							p >> 24 & 0xFF,
							c >>  0 & 0xFF,
							c >>  8 & 0xFF,
							c >> 16 & 0xFF,
							c >> 24 & 0xFF
						};

						state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
					}

					state.Begin( AsciiId<'D','A','T'>::V ).Write32( stream.Size() ).Compress( stream.Begin(), stream.Size() ).End();

					state.End();
				}
			}

			void FamilyKeyboard::LoadState(State::Loader& loader,const dword id)
			{
				if (dataRecorder)
					dataRecorder->Stop();

				if (id == AsciiId<'F','B'>::V)
				{
					while (const dword chunk = loader.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'K','B','D'>::V:
							{
								const uint data = loader.Read8();

								mode = data & 0x1;
								scan = data >> 1 & 0xF;

								if (scan > 9)
									scan = 0;

								break;
							}

							case AsciiId<'D','T','R'>::V:

								NST_VERIFY( dataRecorder );

								if (dataRecorder)
									dataRecorder->LoadState( loader );

								break;
						}

						loader.End();
					}
				}
			}

			void FamilyKeyboard::DataRecorder::LoadState(State::Loader& state)
			{
				Stop();

				while (const dword chunk = state.Begin())
				{
					switch (chunk)
					{
						case AsciiId<'R','E','G'>::V:
						{
							State::Loader::Data<11> data( state );

							status = (data[0] == 1 ? PLAYING : data[0] == 2 ? RECORDING : STOPPED);
							in = data[1] & 0x2;
							out = data[2];

							if (status == PLAYING)
								pos = data[3] | data[4] << 8 | dword(data[5]) << 16 | dword(data[6]) << 24;

							if (status != STOPPED)
							{
								cycles  = data[7] | data[8] << 8 | dword(data[9]) << 16 | dword(data[10]) << 24;
								cycles *= cpu.GetClock() * clocks[cpu.GetModel()][0];
								cycles += cpu.GetCycles() * clocks[cpu.GetModel()][0];
							}

							break;
						}

						case AsciiId<'D','A','T'>::V:
						{
							const dword size = state.Read32();

							NST_VERIFY( size > 0 && size <= MAX_LENGTH );

							if (size > 0 && size <= MAX_LENGTH)
							{
								stream.Resize( size );
								state.Uncompress( stream.Begin(), size );
							}

							break;
						}
					}

					state.End();
				}

				if (status != STOPPED)
				{
					if (stream.Size() && pos < stream.Size() && cycles <= clocks[cpu.GetModel()][1] * 2)
					{
						Start();
					}
					else
					{
						status = STOPPED;
						cycles = Cpu::CYCLE_MAX-1UL;
					}
				}
			}

			void FamilyKeyboard::DataRecorder::Start()
			{
				cpu.AddHook( Hook(this,&DataRecorder::Hook_Tape) );
				Api::TapeRecorder::eventCallback( status == PLAYING ? Api::TapeRecorder::EVENT_PLAYING : Api::TapeRecorder::EVENT_RECORDING );
			}

			Result FamilyKeyboard::DataRecorder::Stop()
			{
				if (status == STOPPED)
					return RESULT_NOP;

				status = STOPPED;
				cycles = Cpu::CYCLE_MAX-1UL;
				in = 0;
				out = 0;

				Api::TapeRecorder::eventCallback( Api::TapeRecorder::EVENT_STOPPED );

				return RESULT_OK;
			}

			Result FamilyKeyboard::PlayTape()
			{
				return dataRecorder ? dataRecorder->Play() : RESULT_ERR_NOT_READY;
			}

			Result FamilyKeyboard::RecordTape()
			{
				return dataRecorder ? dataRecorder->Record() : RESULT_ERR_NOT_READY;
			}

			Result FamilyKeyboard::StopTape()
			{
				return dataRecorder ? dataRecorder->Stop() : RESULT_NOP;
			}

			bool FamilyKeyboard::IsTapeRecording() const
			{
				return dataRecorder ? dataRecorder->IsRecording() : false;
			}

			bool FamilyKeyboard::IsTapePlaying() const
			{
				return dataRecorder ? dataRecorder->IsPlaying() : false;
			}

			bool FamilyKeyboard::IsTapeStopped() const
			{
				return dataRecorder ? dataRecorder->IsStopped() : false;
			}

			bool FamilyKeyboard::IsTapePlayable() const
			{
				return dataRecorder ? dataRecorder->Playable() : false;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void FamilyKeyboard::DataRecorder::EndFrame()
			{
				if (cycles == Cpu::CYCLE_MAX)
					return;

				if (cycles != Cpu::CYCLE_MAX-1UL)
				{
					const Cycle frame = cpu.GetFrameCycles() * clocks[cpu.GetModel()][0];

					if (cycles > frame)
						cycles -= frame;
					else
						cycles = 0;
				}
				else
				{
					cycles = Cpu::CYCLE_MAX;
					cpu.RemoveHook( Hook(this,&DataRecorder::Hook_Tape) );
				}
			}

			void FamilyKeyboard::EndFrame()
			{
				if (dataRecorder)
					dataRecorder->EndFrame();
			}

			void FamilyKeyboard::Poke(const uint data)
			{
				if (dataRecorder)
					dataRecorder->Poke( data );

				if (data & COMMAND_KEY)
				{
					const uint out = data & COMMAND_SCAN;

					if (mode && !out && ++scan > 9)
						scan = 0;

					mode = out >> 1;

					if (data & COMMAND_RESET)
						scan = 0;
				}
			}

			uint FamilyKeyboard::Peek(uint port)
			{
				if (port == 0)
				{
					return dataRecorder ? dataRecorder->Peek() : 0;
				}
				else if (input && scan < 9)
				{
					Controllers::FamilyKeyboard::callback( input->familyKeyboard, scan, mode );
					return ~uint(input->familyKeyboard.parts[scan]) & 0x1E;
				}
				else
				{
					return 0x1E;
				}
			}

			NES_HOOK(FamilyKeyboard::DataRecorder,Tape)
			{
				for (const Cycle next = cpu.GetCycles() * clocks[cpu.GetModel()][0]; cycles < next; cycles += clocks[cpu.GetModel()][1])
				{
					if (status == PLAYING)
					{
						if (pos < stream.Size())
						{
							const uint data = stream[pos++];

							if (data >= 0x8C)
							{
								in = 0x2;
							}
							else if (data <= 0x74)
							{
								in = 0x0;
							}
						}
						else
						{
							Stop();
							break;
						}
					}
					else
					{
						NST_ASSERT( status == RECORDING );

						if (stream.Size() < MAX_LENGTH)
						{
							stream.Append( (out & 0x7) == 0x7 ? 0x90 : 0x70 );
						}
						else
						{
							Stop();
							break;
						}
					}
				}
			}
		}
	}
}
