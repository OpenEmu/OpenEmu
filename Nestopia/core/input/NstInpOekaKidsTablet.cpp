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
#include "NstInpOekaKidsTablet.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			OekaKidsTablet::OekaKidsTablet(const Cpu& c)
			: Device(c,Api::Input::OEKAKIDSTABLET)
			{
				OekaKidsTablet::Reset();
			}

			void OekaKidsTablet::Reset()
			{
				latch  = 0;
				state  = 0;
				stream = 0;
				bits   = 0;
			}

			void OekaKidsTablet::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'O','T'>::R(0,0,id) ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void OekaKidsTablet::Poke(uint data)
			{
				if (data & 0x1)
				{
					if (~latch & data & 0x2)
						stream <<= 1;

					state = (data & 0x2) ? (~stream >> 15 & 0x8) : 0x4;
					latch = data;
				}
				else
				{
					state = 0;

					if (input)
					{
						Controllers::OekaKidsTablet& tablet = input->oekaKidsTablet;
						input = NULL;

						if (Controllers::OekaKidsTablet::callback( tablet ))
						{
							if (tablet.x <= 255 && tablet.y <= 239)
							{
								dword data = tablet.y * 256U / 240;

								if (data > 12)
									data = (data - 12) << 2;
								else
									data = 0;

								data |= (tablet.x * 240U / 256 + 8UL) << 10;

								if (tablet.button)
								{
									data |= 0x3;
								}
								else if (tablet.y >= 48)
								{
									data |= 0x2;
								}

								bits = data;
							}
						}
					}

					stream = bits;
				}
			}

			uint OekaKidsTablet::Peek(uint port)
			{
				return port ? state : 0;
			}
		}
	}
}
