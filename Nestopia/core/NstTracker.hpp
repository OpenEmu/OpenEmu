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

#ifndef NST_TRACKER_H
#define NST_TRACKER_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Machine;

		namespace Video
		{
			class Output;
		}

		namespace Sound
		{
			class Output;
		}

		namespace Input
		{
			class Controllers;
		}

		class Tracker
		{
		public:

			Tracker();
			~Tracker();

			void   Reset();
			void   PowerOff();
			Result Execute(Machine&,Video::Output*,Sound::Output*,Input::Controllers*);
			void   Resync(bool=false) const;
			Result TryResync(Result,bool=false) const;
			void   Unload();
			bool   IsActive() const;
			bool   IsLocked(bool=false) const;

			Result EnableRewinder(Machine*);
			void   EnableRewinderSound(bool);
			void   ResetRewinder() const;
			Result StartRewinding() const;
			Result StopRewinding() const;
			bool   IsRewinding() const;

			Result PlayMovie(Machine&,std::istream&);
			Result RecordMovie(Machine&,std::iostream&,bool);
			void   StopMovie();
			bool   IsMoviePlaying() const;
			bool   IsMovieRecording() const;

		private:

			void UpdateRewinderState(bool);

			class Movie;
			class Rewinder;

			dword frame;
			ibool rewinderSound;
			Machine* rewinderEnabled;
			Rewinder* rewinder;
			Movie* movie;

		public:

			bool IsRewinderEnabled() const
			{
				return rewinderEnabled;
			}

			bool IsRewinderSoundEnabled() const
			{
				return rewinderSound;
			}

			bool IsFrameLocked() const
			{
				return movie;
			}

			dword Frame() const
			{
				return frame;
			}
		};
	}
}

#endif
