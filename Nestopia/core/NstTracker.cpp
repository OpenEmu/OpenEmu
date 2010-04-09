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

#include <new>
#include "NstMachine.hpp"
#include "NstTrackerMovie.hpp"
#include "NstTrackerRewinder.hpp"
#include "NstImage.hpp"
#include "api/NstApiMachine.hpp"

namespace Nes
{
	namespace Core
	{
		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("s", on)
		#endif

		Tracker::Tracker()
		:
		frame           (0),
		rewinderSound   (false),
		rewinderEnabled (NULL),
		rewinder        (NULL),
		movie           (NULL)
		{}

		Tracker::~Tracker()
		{
			delete rewinder;
			delete movie;
		}

		void Tracker::Unload()
		{
			frame = 0;

			if (rewinder)
				rewinder->Unload();
			else
				StopMovie();
		}

		void Tracker::Reset()
		{
			frame = 0;

			if (rewinder)
			{
				rewinder->Reset();
			}
			else if (movie)
			{
				movie->Reset();
			}
		}

		void Tracker::PowerOff()
		{
			StopMovie();
		}

		void Tracker::Resync(bool excludeFrame) const
		{
			if (rewinder)
			{
				rewinder->Reset();
			}
			else if (movie && !excludeFrame)
			{
				movie->Resync();
			}
		}

		Result Tracker::TryResync(Result lastResult,bool excludeFrame) const
		{
			NST_VERIFY( NES_SUCCEEDED(lastResult) );

			if (NES_SUCCEEDED(lastResult) && lastResult != RESULT_NOP)
				Resync( excludeFrame );

			return lastResult;
		}

		Result Tracker::EnableRewinder(Machine* const emulator)
		{
			if (rewinderEnabled == emulator)
				return RESULT_NOP;

			rewinderEnabled = emulator;
			UpdateRewinderState( true );

			return RESULT_OK;
		}

		void Tracker::EnableRewinderSound(bool enable)
		{
			rewinderSound = enable;

			if (rewinder)
				rewinder->EnableSound( enable );
		}

		void Tracker::ResetRewinder() const
		{
			if (rewinder)
				rewinder->Reset();
		}

		void Tracker::UpdateRewinderState(bool enable)
		{
			if (enable && rewinderEnabled && !movie)
			{
				if (!rewinder)
				{
					rewinder = new Rewinder
					(
						*rewinderEnabled,
						&Machine::Execute,
						&Machine::LoadState,
						&Machine::SaveState,
						rewinderEnabled->cpu,
						rewinderEnabled->cpu.GetApu(),
						rewinderEnabled->ppu,
						rewinderSound
					);
				}
			}
			else
			{
				delete rewinder;
				rewinder = NULL;
			}
		}

		Result Tracker::PlayMovie(Machine& emulator,std::istream& stream)
		{
			if (!emulator.Is(Api::Machine::GAME))
				return RESULT_ERR_NOT_READY;

			UpdateRewinderState( false );

			Result result;

			try
			{
				if (movie == NULL)
				{
					movie = new Movie
					(
						emulator,
						&Machine::LoadState,
						&Machine::SaveState,
						emulator.cpu,
						emulator.Is(Api::Machine::CARTRIDGE) ? emulator.image->GetPrgCrc() : 0
					);
				}

				if (movie->Play( stream ))
				{
					if (emulator.Is(Api::Machine::ON))
						emulator.Reset( true );

					return RESULT_OK;
				}
				else
				{
					return RESULT_NOP;
				}
			}
			catch (Result r)
			{
				result = r;
			}
			catch (const std::bad_alloc&)
			{
				result = RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				result = RESULT_ERR_GENERIC;
			}

			StopMovie();

			return result;
		}

		Result Tracker::RecordMovie(Machine& emulator,std::iostream& stream,const bool append)
		{
			if (!emulator.Is(Api::Machine::GAME))
				return RESULT_ERR_NOT_READY;

			UpdateRewinderState( false );

			Result result;

			try
			{
				if (movie == NULL)
				{
					movie = new Movie
					(
						emulator,
						&Machine::LoadState,
						&Machine::SaveState,
						emulator.cpu,
						emulator.image->GetPrgCrc()
					);
				}

				return movie->Record( stream, append ) ? RESULT_OK : RESULT_NOP;
			}
			catch (Result r)
			{
				result = r;
			}
			catch (const std::bad_alloc&)
			{
				result = RESULT_ERR_OUT_OF_MEMORY;
			}
			catch (...)
			{
				result = RESULT_ERR_GENERIC;
			}

			StopMovie();

			return result;
		}

		void Tracker::StopMovie()
		{
			delete movie;
			movie = NULL;

			UpdateRewinderState( true );
		}

		#ifdef NST_MSVC_OPTIMIZE
		#pragma optimize("", on)
		#endif

		Result Tracker::StartRewinding() const
		{
			return rewinder ? rewinder->Start() : RESULT_ERR_NOT_READY;
		}

		Result Tracker::StopRewinding() const
		{
			return rewinder ? rewinder->Stop() : RESULT_NOP;
		}

		bool Tracker::IsRewinding() const
		{
			return rewinder && rewinder->IsRewinding();
		}

		bool Tracker::IsMoviePlaying() const
		{
			return movie && movie->IsPlaying();
		}

		bool Tracker::IsMovieRecording() const
		{
			return movie && movie->IsRecording();
		}

		bool Tracker::IsLocked(bool excludeFrame) const
		{
			return IsRewinding() || (!excludeFrame && IsMoviePlaying());
		}

		bool Tracker::IsActive() const
		{
			return IsRewinding() || movie;
		}

		Result Tracker::Execute
		(
			Machine& emulator,
			Video::Output* const video,
			Sound::Output* const sound,
			Input::Controllers* input
		)
		{
			if (emulator.Is(Api::Machine::ON))
			{
				++frame;

				if (emulator.Is(Api::Machine::GAME))
				{
					if (rewinder)
					{
						rewinder->Execute( video, sound, input );
						return RESULT_OK;
					}
					else if (movie)
					{
						if (!movie->Execute())
						{
							StopMovie();
						}
						else if (movie->IsPlaying())
						{
							input = NULL;
						}
					}
				}

				emulator.Execute( video, sound, input );
				return RESULT_OK;
			}
			else
			{
				return RESULT_ERR_NOT_READY;
			}
		}
	}
}
