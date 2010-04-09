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
#include "NstInpPartyTap.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			PartyTap::PartyTap(const Cpu& c)
			: Device(c,Api::Input::PARTYTAP)
			{
				PartyTap::Reset();
			}

			void PartyTap::Reset()
			{
				strobe = 0;
				state = 0;
				mode = 0xE0;
				stream = 0;
			}

			void PartyTap::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'P','T'>::R(0,0,id) ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void PartyTap::Poke(const uint data)
			{
				mode = 0xE0 - ((data & 0x4) << 4);

				const uint prev = strobe;
				strobe = data & 0x1;

				if (prev > strobe)
				{
					if (input)
					{
						Controllers::PartyTap::callback( input->partyTap );
						state = input->partyTap.units;
						input = NULL;
					}

					stream = state;
				}
			}

			uint PartyTap::Peek(uint port)
			{
				if (port)
				{
					port = stream & 0x1C;
					stream = stream >> 3 | mode;
				}

				return port;
			}
		}
	}
}
