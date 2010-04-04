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
#include "NstInpPowerPad.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			PowerPad::PowerPad(const Cpu& c)
			: Device(c,Api::Input::POWERPAD)
			{
				PowerPad::Reset();
			}

			void PowerPad::Reset()
			{
				shifter = 2;
				stream = 0x55FFUL << 3;
				state = 0x55FFUL << 3;
			}

			void PowerPad::SaveState(State::Saver& saver,const byte id) const
			{
				saver.Begin( AsciiId<'P','P'>::R(0,0,id) ).Write8( shifter >> 1 ^ 0x1 ).Write16( stream >> 3 ^ 0x55FFU ).End();
			}

			void PowerPad::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'P','P'>::V)
				{
					shifter = (~loader.Read8() & 0x1) << 1;
					stream = ((loader.Read16() & 0x55FFUL) ^ 0x55FFUL) << 3;
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			uint PowerPad::Peek(uint)
			{
				const uint data = stream;
				stream >>= shifter;
				return ~data & 0x18;
			}

			void PowerPad::Poke(uint data)
			{
				const uint prev = shifter;
				shifter = (~data & 0x1) << 1;

				if (prev > shifter)
				{
					if (input)
					{
						Controllers::PowerPad& power = input->powerPad;
						input = NULL;

						if (Controllers::PowerPad::callback( power ))
						{
							static const dword lut[Controllers::PowerPad::NUM_SIDE_A_BUTTONS] =
							{
								( 0x02UL << 4  | 0x00UL      ),
								( 0x01UL << 3  | 0x00UL      ),
								( 0x00UL       | 0x02UL << 5 ),
								( 0x00UL       | 0x01UL << 4 ),
								( 0x04UL << 5  | 0x00UL      ),
								( 0x10UL << 7  | 0x00UL      ),
								( 0x80UL << 10 | 0x00UL      ),
								( 0x00UL       | 0x08UL << 7 ),
								( 0x08UL << 6  | 0x00UL      ),
								( 0x20UL << 8  | 0x00UL      ),
								( 0x40UL << 9  | 0x00UL      ),
								( 0x00UL       | 0x04UL << 6 )
							};

							data = 0;

							for (uint i=0; i < Controllers::PowerPad::NUM_SIDE_A_BUTTONS; ++i)
							{
								if (power.sideA[i])
									data |= lut[i];
							}

							static const byte index[Controllers::PowerPad::NUM_SIDE_B_BUTTONS] =
							{
								2,1,7,6,5,4,10,9
							};

							for (uint i=0; i < Controllers::PowerPad::NUM_SIDE_B_BUTTONS; ++i)
							{
								if (power.sideB[i])
									data |= lut[index[i]];
							}

							state = data ^ (0x55FFUL << 3);
						}
					}

					stream = state;
				}
			}
		}
	}
}
