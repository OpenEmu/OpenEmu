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

#ifndef NST_INPUT_ROB_H
#define NST_INPUT_ROB_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		class Ppu;

		namespace Input
		{
			class Rob : public Device
			{
			public:

				Rob(const Cpu&,const Ppu&);

			private:

				enum
				{
					SIGNAL_COLOR = 0x29,
					SIGNAL_RANGE = 3
				};

				enum
				{
					CODE_OPEN_ARM   = 0x0EE8,
					CODE_CLOSE_ARM  = 0x0FA8,
					CODE_TEST       = 0x1AE8,
					CODE_UNUSED     = 0x0AAA,
					CODE_RAISE_ARM  = 0x1BA8,
					CODE_LOWER_ARM  = 0x1BE8,
					CODE_TURN_LEFT  = 0x0BA8,
					CODE_TURN_RIGHT = 0x0AE8
				};

				void BeginFrame(Controllers*);
				void Reset();
				void Poke(uint);
				uint Peek(uint);
				void LoadState(State::Loader&,dword);
				void SaveState(State::Saver&,byte) const;

				uint strobe;
				uint stream;
				uint state;
				uint shifter;
				uint code;

				const byte (&palette)[32];
			};
		}
	}
}

#endif
