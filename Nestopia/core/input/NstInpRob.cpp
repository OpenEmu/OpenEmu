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
#include "NstInpRob.hpp"
#include "../NstPpu.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Rob::Rob(const Cpu& c,const Ppu& ppu)
			: Device(c,Api::Input::ROB), palette(ppu.GetPalette().ram)
			{
				Rob::Reset();
			}

			void Rob::Reset()
			{
				strobe = 0;
				stream = 0xFF;
				state = 0;
				shifter = 1;
				code = 0;
			}

			void Rob::SaveState(State::Saver& saver,const byte id) const
			{
				byte data[6] =
				{
					strobe,
					stream ^ 0xFF,
					state,
					0,
					code & 0xFF,
					code >> 8
				};

				while (!(shifter & 1U << data[3]))
					++data[3];

				saver.Begin( AsciiId<'R','O'>::R(0,0,id) ).Write( data ).End();
			}

			void Rob::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'R','O'>::V)
				{
					State::Loader::Data<6> data( loader );

					strobe = data[0] & 0x1;
					stream = data[1] ^ 0xFF;
					state = data[2];
					shifter = 1U << NST_MIN(data[3],13);
					code = data[4] | (data[5] << 8 & 0x100);
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Rob::BeginFrame(Controllers*)
			{
				for (uint i=1; i < 16; ++i)
				{
					if (palette[0] != palette[i])
					{
						shifter = 1;
						code = 0;
						return;
					}
				}

				if (palette[0] - uint(SIGNAL_COLOR) < uint(SIGNAL_RANGE))
					code |= shifter;

				if (shifter != 0x1000)
				{
					shifter <<= 1;

					if (shifter == 0x40 && code == 0)
						shifter = 1;
				}
				else
				{
					uint tmp = code;
					code = 0;
					shifter = 1;

					switch (tmp)
					{
						case CODE_OPEN_ARM:   state ^= 0x01; break;
						case CODE_CLOSE_ARM:  state ^= 0x02; break;
						case CODE_TEST:       state ^= 0x04; break;
						case CODE_UNUSED:     state ^= 0x08; break;
						case CODE_RAISE_ARM:  state ^= 0x10; break;
						case CODE_LOWER_ARM:  state ^= 0x20; break;
						case CODE_TURN_LEFT:  state ^= 0x40; break;
						case CODE_TURN_RIGHT: state ^= 0x80; break;
					}
				}
			}

			uint Rob::Peek(uint)
			{
				if (strobe == 0)
				{
					const uint data = stream;
					stream >>= 1;

					return ~data & 0x1;
				}
				else
				{
					return state & 0x1;
				}
			}

			void Rob::Poke(uint data)
			{
				const uint prev = strobe;
				strobe = data & 0x1;

				if (prev > strobe)
					stream = state ^ 0xFF;
			}
		}
	}
}

