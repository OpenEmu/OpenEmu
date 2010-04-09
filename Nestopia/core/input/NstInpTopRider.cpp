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
#include "NstInpTopRider.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			TopRider::TopRider(const Cpu& c)
			: Device(c,Api::Input::TOPRIDER)
			{
				TopRider::Reset();
			}

			void TopRider::Reset()
			{
				state[0] = 0;
				state[1] = 0;
				stream[0] = 0;
				stream[1] = 0;
				strobe = 0;
				buttons = 0;
				brake = 0;
				accel = 0;
				pos = 0;
			}

			void TopRider::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'T','R'>::R(0,0,id) ).End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void TopRider::BeginFrame(Controllers* const controllers)
			{
				if (controllers)
				{
					Controllers::TopRider::callback( controllers->topRider );

					uint data = controllers->topRider.buttons;

					if ((data & STEERING) == STEERING)
						data &= STEERING ^ 0xFFU;

                         if ( !(data & STEERING) ) pos += (pos > 0 ? -1 : pos < 0 ? +1 : 0);
					else if ( data & STEER_LEFT  ) pos -= (pos > -MAX_STEER);
					else if ( data & STEER_RIGHT ) pos += (pos < +MAX_STEER);

					if (data & BRAKE) brake += (brake < MAX_BRAKE);
					else              brake -= (brake > 0);

					if (data & ACCEL) accel += (accel < MAX_ACCEL);
					else              accel -= (accel > 0);

					buttons &= (0x80U|0x40U);

					if (data & SHIFT_GEAR)
					{
						if (!(buttons & 0x40))
						{
							buttons ^= 0x80;
							buttons |= 0x40;
						}
					}
					else
					{
						buttons &= 0x40U ^ 0xFFU;
					}

					buttons |=
					(
						(( data & REAR   ) >> 5) |
						(( data & SELECT ) << 3) |
						(( data & START  ) << 1)
					);

					data = 0;

					if (pos > 0)
					{
                             if (pos > DEADZONE_MAX) data = (0x20U | 0x080U);
						else if (pos > DEADZONE_MID) data = (0x20U | 0x000U);
						else if (pos > DEADZONE_MIN) data = (0x00U | 0x080U);
					}
					else
					{
                             if (pos < -DEADZONE_MAX) data = (0x40U | 0x100U);
						else if (pos < -DEADZONE_MID) data = (0x40U | 0x000U);
						else if (pos < -DEADZONE_MIN) data = (0x00U | 0x100U);
					}

					state[0] = data | ((buttons & 0x01) << (4+7))  | ((buttons & 0x80) << (4-1));

					data = 0;

					if (accel > 8 || brake < 8)
					{
                             if (accel > DEADZONE_MAX) data = 0x008;
						else if (accel > DEADZONE_MID) data = 0x080;
						else if (accel > DEADZONE_MIN) data = 0x100;
					}
					else
					{
						state[0] |= 0x200;

                             if (brake > DEADZONE_MAX) data = 0x10;
						else if (brake > DEADZONE_MID) data = 0x20;
						else if (brake > DEADZONE_MIN) data = 0x40;
					}

					state[1] = data | ((buttons & 0x30) << (3+2));
				}
				else
				{
					buttons = 0;
					brake = 0;
					accel = 0;
					pos = 0;
					state[0] = 0;
					state[1] = 0;
				}
			}

			void TopRider::Poke(uint data)
			{
				const uint prev = strobe;
				strobe = data & 0x1;

				if (prev > strobe)
				{
					stream[0] = state[0];
					stream[1] = state[1];
				}
			}

			uint TopRider::Peek(uint port)
			{
				if (port)
				{
					port = (stream[0] & 0x10) | (stream[1] & 0x08);
					stream[0] >>= 1, stream[1] >>= 1;
				}

				return port;
			}
		}
	}
}
