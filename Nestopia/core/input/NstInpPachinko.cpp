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
#include "NstInpPachinko.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Pachinko::Pachinko(const Cpu& c)
			: Device(c,Api::Input::PACHINKO)
			{
				Pachinko::Reset();
			}

			void Pachinko::Reset()
			{
				strobe = 0;
				stream = 0;
				state = 0xFF0000;
			}

			void Pachinko::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'P','A'>::R(0,0,id) ).Write8( strobe ).Write32( stream ).End();
			}

			void Pachinko::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'P','A'>::V)
				{
					strobe = loader.Read8() & 0x1;
					stream = loader.Read32();
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			uint Pachinko::Peek(uint port)
			{
				if (port == 0)
				{
					port = stream;
					stream >>= 1;
					return port & 0x2;
				}
				else
				{
					return 0;
				}
			}

			void Pachinko::Poke(uint data)
			{
				const uint prev = strobe;
				strobe = data & 0x1;

				if (prev > strobe)
				{
					if (input)
					{
						Controllers::Pachinko& pachinko = input->pachinko;
						input = NULL;

						if (Controllers::Pachinko::callback( pachinko ))
						{
							uint throttle = Clamp<-64,+63>(pachinko.throttle) + 192;

							throttle =
							(
								(throttle >> 7 & 0x01) |
								(throttle >> 5 & 0x02) |
								(throttle >> 3 & 0x04) |
								(throttle >> 1 & 0x08) |
								(throttle << 1 & 0x10) |
								(throttle << 3 & 0x20) |
								(throttle << 5 & 0x40) |
								(throttle << 7 & 0x80)
							);

							state = ((pachinko.buttons & 0xFF) | (throttle << 8) | 0xFF0000UL) << 1;
						}
					}

					stream = state;
				}
			}
		}
	}
}
