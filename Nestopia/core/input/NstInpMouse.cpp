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
#include "NstInpMouse.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Mouse::Mouse(const Cpu& c)
			: Device(c,Api::Input::MOUSE)
			{
				Mouse::Reset();
			}

			void Mouse::Reset()
			{
				strobe = 0;
				stream = 0;
				state = 0;
			}

			void Mouse::SaveState(State::Saver& saver,const byte id) const
			{
				const byte data[2] =
				{
					strobe, stream ^ 0xFF
				};

				saver.Begin( AsciiId<'M','S'>::R(0,0,id) ).Write( data ).End();
			}

			void Mouse::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'M','S'>::V)
				{
					State::Loader::Data<2> data( loader );

					strobe = data[0] & 0x1;
					stream = data[1] ^ 0xFF;
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			uint Mouse::Peek(uint)
			{
				const uint data = stream;
				stream >>= 1;
				return ~data & 0x1;
			}

			void Mouse::Poke(uint data)
			{
				const uint prev = strobe;
				strobe = data & 0x1;

				if (prev > strobe)
				{
					if (input)
					{
						Controllers::Mouse& mouse = input->mouse;
						input = NULL;

						if (Controllers::Mouse::callback( mouse ))
						{
							data = 0x00;

							if (mouse.button)
								data = 0x01;

							int ox = x;
							int oy = y;

							x = NST_MIN(mouse.x,255);
							y = NST_MIN(mouse.y,239);

							ox -= x;
							oy -= y;

							if (ox > 0)
							{
								data |= 0x0C;
							}
							else if (ox < 0)
							{
								data |= 0x04;
							}

							if (oy > 0)
							{
								data |= 0x30;
							}
							else if (oy < 0)
							{
								data |= 0x10;
							}

							state = data ^ 0xFF;
						}
					}

					stream = state;
				}
			}
		}
	}
}
