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
#include "NstInpPokkunMoguraa.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			PokkunMoguraa::PokkunMoguraa(const Cpu& c)
			: Device(c,Api::Input::POKKUNMOGURAA)
			{
				PokkunMoguraa::Reset();
			}

			void PokkunMoguraa::Reset()
			{
				state = 0x1E;
			}

			void PokkunMoguraa::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'P','M'>::R(0,0,id) ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void PokkunMoguraa::Poke(const uint data)
			{
				if (input)
				{
					Controllers::PokkunMoguraa::callback( input->pokkunMoguraa, ~data & 0x7 );
					state = ~input->pokkunMoguraa.buttons & 0x1E;
				}
				else
				{
					state = 0x1E;
				}
			}

			uint PokkunMoguraa::Peek(const uint port)
			{
				return port ? state : 0;
			}
		}
	}
}
