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

#ifndef NST_INPUT_TOPRIDER_H
#define NST_INPUT_TOPRIDER_H

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			class TopRider : public Device
			{
			public:

				explicit TopRider(const Cpu&);

			private:

				void Reset();
				void BeginFrame(Controllers*);
				void Poke(uint);
				uint Peek(uint);
				void SaveState(State::Saver&,byte) const;

				enum
				{
					DEADZONE_MAX = 16,
					DEADZONE_MID = 10,
					DEADZONE_MIN = 4,
					MAX_STEER    = 20,
					MAX_BRAKE    = 20,
					MAX_ACCEL    = 20,
					BRAKE        = 0x01,
					ACCEL        = 0x02,
					SELECT       = 0x04,
					START        = 0x08,
					SHIFT_GEAR   = 0x10,
					REAR         = 0x20,
					STEER_LEFT   = 0x40,
					STEER_RIGHT  = 0x80,
					STEERING     = STEER_LEFT|STEER_RIGHT
				};

				uint stream[2];
				uint state[2];
				uint strobe;
				int pos;
				uint accel;
				uint brake;
				uint buttons;
			};
		}
	}
}

#endif
