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

#ifndef NST_INPUT_CRAZYCLIMBER_H
#define NST_INPUT_CRAZYCLIMBER_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			class CrazyClimber : public Device
			{
			public:

				explicit CrazyClimber(const Cpu&);

			private:

				void Reset();
				void Poke(uint);
				uint Peek(uint);
				void LoadState(State::Loader&,dword);
				void SaveState(State::Saver&,byte) const;

				enum {LEFT,RIGHT};

				enum
				{
					BUTTON_RIGHT = 0x10,
					BUTTON_LEFT  = 0x20,
					BUTTON_UP    = 0x40,
					BUTTON_DOWN  = 0x80
				};

				uint shifter;
				uint stream[2];
				uint state[2];
			};
		}
	}
}

#endif
