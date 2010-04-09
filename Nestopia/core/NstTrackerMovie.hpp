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

#ifndef NST_TRACKER_MOVIE_H
#define NST_TRACKER_MOVIE_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Tracker::Movie
		{
			typedef bool (Machine::*EmuLoadState)(State::Loader&,bool);
			typedef void (Machine::*EmuSaveState)(State::Saver&) const;

		public:

			Movie(Machine&,EmuLoadState,EmuSaveState,Cpu&,dword);
			~Movie();

			bool Play(std::istream&);
			bool Record(std::iostream&,bool);
			void Stop();
			void Resync();
			void Reset();
			bool Execute();

		private:

			bool Stop(Result);

			class Player;
			class Recorder;

			Player* player;
			Recorder* recorder;
			Machine& emulator;
			const EmuSaveState saveState;
			const EmuLoadState loadState;
			Cpu& cpu;
			const dword prgCrc;

		public:

			bool IsPlaying() const
			{
				return player;
			}

			bool IsRecording() const
			{
				return recorder;
			}
		};
	}
}

#endif
