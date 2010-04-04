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
#include "NstInpMahjong.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Mahjong::Mahjong(const Cpu& c)
			: Device(c,Api::Input::MAHJONG)
			{
				Mahjong::Reset();
			}

			void Mahjong::Reset()
			{
				stream = 0;
			}

			void Mahjong::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'M','J'>::R(0,0,id) ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Mahjong::Poke(uint data)
			{
				data &= PART;

				if (data && input)
				{
					Controllers::Mahjong::callback( input->mahjong, data );
					stream = input->mahjong.buttons << 1;
				}
				else
				{
					stream = 0;
				}
			}

			uint Mahjong::Peek(uint port)
			{
				if (port)
				{
					port = stream & 0x2;
					stream >>= 1;
				}

				return port;
			}
		}
	}
}
