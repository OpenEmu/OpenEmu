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
#include "NstInpPaddle.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Paddle::Paddle(const Cpu& c,bool p)
			: Device(c,Api::Input::PADDLE), expPort(p)
			{
				Paddle::Reset();
			}

			void Paddle::Reset()
			{
				stream[0] = 0;
				stream[1] = 0;
				shifter = 1;
				x = 0;
				button = 0;
			}

			void Paddle::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'P','L'>::R(0,0,id) ).Write8( shifter ^ 1 ).End();
			}

			void Paddle::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'P','L'>::V)
					shifter = ~loader.Read8() & 0x1;
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			uint Paddle::Peek(uint port)
			{
				if (!expPort)
				{
					port = stream[0];
					stream[0] >>= shifter;
					return (port & 0x10) | stream[1];
				}
				else if (port)
				{
					port = stream[0];
					stream[0] >>= shifter;
					return port & 0x02;
				}
				else
				{
					return stream[1];
				}
			}

			void Paddle::Poke(uint data)
			{
				const uint prev = shifter;
				shifter = ~data & 0x1;

				if (prev < shifter)
				{
					if (input)
					{
						Controllers::Paddle& paddle = input->paddle;
						input = NULL;

						if (Controllers::Paddle::callback( paddle ))
						{
							data = 0xFF - ((82 + 172 * (Clamp<32,176>(paddle.x) - 32U) / 144) & 0xFF);

							x =
							(
								( data & 0x01 ) << 7 |
								( data & 0x02 ) << 5 |
								( data & 0x04 ) << 3 |
								( data & 0x08 ) << 1 |
								( data & 0x10 ) >> 1 |
								( data & 0x20 ) >> 3 |
								( data & 0x40 ) >> 5 |
								( data & 0x80 ) >> 7
							)   << (expPort ? 1 : 4);

							button = (paddle.button ? expPort ? 0x2 : 0x8 : 0x0);
						}
					}

					stream[0] = x;
					stream[1] = button;
				}
			}
		}
	}
}
