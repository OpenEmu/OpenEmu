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
#include "NstInpPowerGlove.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			PowerGlove::PowerGlove(const Cpu& c)
			: Device(c,Api::Input::POWERGLOVE)
			{
				buffer[0x0] = 0xA0;
				buffer[0x1] = 0x00;
				buffer[0x2] = 0x00;
				buffer[0x3] = 0x00;
				buffer[0x4] = 0x00;
				buffer[0x5] = 0x00;
				buffer[0x6] = 0x00;
				buffer[0x7] = 0x00;
				buffer[0x8] = 0x00;
				buffer[0x9] = 0x3F;
				buffer[0xA] = 0xFF;
				buffer[0xB] = 0xFF;

				PowerGlove::Reset();
			}

			void PowerGlove::Reset()
			{
				latch = 0;
				stream = ~0U;
				output = 0;
				counter = 0;

				buffer[1] = 0;
				buffer[2] = 0;
				buffer[3] = 0;
				buffer[4] = 0;
				buffer[5] = 0;
				buffer[6] = 0;

				z = 32;
				r = 32;
			}

			void PowerGlove::SaveState(State::Saver& saver,const byte id) const
			{
				const byte data[4] =
				{
					latch,
					stream == ~0U ? 0xFF : stream,
					output,
					counter
				};

				saver.Begin( AsciiId<'P','G'>::R(0,0,id) ).Write( data ).End();
			}

			void PowerGlove::LoadState(State::Loader& loader,const dword id)
			{
				buffer[3] = 0;
				buffer[4] = 0;

				z = 32;
				r = 32;

				if (id == AsciiId<'P','G'>::V)
				{
					State::Loader::Data<4> data( loader );

					latch = data[0];
					stream = data[1] < (12U << 3) ? data[1] : ~0U;
					output = data[2];
					counter = NST_MIN(data[3],8+3);
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void PowerGlove::BeginFrame(Controllers* i)
			{
				input = i;
			}

			void PowerGlove::Poll()
			{
				Controllers::PowerGlove& glove = input->powerGlove;
				input = NULL;

				if (Controllers::PowerGlove::callback( glove ))
				{
					buffer[1] = (glove.x - 128U) & 0xFF;
					buffer[2] = (128U - glove.y) & 0xFF;

					if (glove.distance < 0)
					{
						z += (z < 63);
					}
					else if (glove.distance > 0)
					{
						z -= (z > 0);
					}

					buffer[3] = (z/2 - 16U) & 0xFF;

					if (glove.distance < 0)
					{
						r += (r < 63);
					}
					else if (glove.distance > 0)
					{
						r -= (r > 0);
					}
					else if (r < 32)
					{
						++r;
					}
					else if (r > 32)
					{
						--r;
					}

					buffer[4] = (r/2 - 16U) & 0xFF;
					buffer[5] = glove.gesture;

					if (glove.buttons & Controllers::PowerGlove::START)
					{
						buffer[6] = 0x82;
					}
					else if (glove.buttons & Controllers::PowerGlove::SELECT)
					{
						buffer[6] = 0x83;
					}
					else
					{
						buffer[6] = 0xFF;
					}
				}
			}

			uint PowerGlove::Peek(uint)
			{
				uint data = 0;

				if (stream != ~0U)
				{
					data = stream++;

					if (!(data & 0x7))
					{
						if (input)
							Poll();

						output = buffer[data >> 3] ^ 0xFFU;
					}
					else if (data == (12U << 3) - 1)
					{
						stream = 0;
					}

					data = output >> 7;
					output = output << 1 & 0xFFU;
				}

				return data;
			}

			void PowerGlove::Poke(const uint data)
			{
				latch = (latch << 1 & 0xFE) | (data & 0x01);

				if (latch == 0x06 && counter == 0)
				{
					stream = ~0U;
				}
				else if (latch == 0xFF)
				{
					stream = ~0U;
					counter = 1;
				}
				else if (counter && counter++ == 8+3)
				{
					stream = 0;
					counter = 0;
				}
			}
		}
	}
}
