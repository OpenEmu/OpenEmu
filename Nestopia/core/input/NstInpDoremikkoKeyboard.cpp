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
#include "NstInpDoremikkoKeyboard.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			DoremikkoKeyboard::DoremikkoKeyboard(const Cpu& c)
			: Device(c,Api::Input::DOREMIKKOKEYBOARD)
			{
				DoremikkoKeyboard::Reset();
			}

			void DoremikkoKeyboard::Reset()
			{
				reg = 0;
				mode = 0;
				part = 0;
			}

			void DoremikkoKeyboard::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'D','K'>::R(0,0,id) ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void DoremikkoKeyboard::Poke(const uint data)
			{
				if ((data & 0x2) > (reg & 0x2))
				{
					part = 0;
					mode = 0;
				}

				if ((data & 0x1) > (reg & 0x1))
				{
					++part;
					mode = 0;
				}

				reg = data;
			}

			uint DoremikkoKeyboard::Peek(uint port)
			{
				if (port)
				{
					port = mode;
					mode ^= 1;

					if (input)
					{
						Controllers::DoremikkoKeyboard::callback( input->doremikkoKeyboard, part, port );
						return input->doremikkoKeyboard.keys & 0x1E;
					}
				}

				return 0;
			}
		}
	}
}
