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
#include "NstInpPad.hpp"
#include "../NstCpu.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Input
		{
			uint Pad::mic;

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Pad::Pad(const Cpu& c,uint i)
			: Device(c,Type(uint(Api::Input::PAD1) + i))
			{
				NST_ASSERT( i < 4 );

				NST_COMPILE_ASSERT
				(
					( Api::Input::PAD2 - Api::Input::PAD1 ) == 1 &&
					( Api::Input::PAD3 - Api::Input::PAD1 ) == 2 &&
					( Api::Input::PAD4 - Api::Input::PAD1 ) == 3
				);

				Pad::Reset();
			}

			void Pad::Reset()
			{
				strobe = 0;
				stream = 0xFF;
				state = 0;
				timeStamp = 0;
				mic = 0;
			}

			void Pad::SaveState(State::Saver& saver,const byte id) const
			{
				const byte data[2] =
				{
					strobe, stream ^ 0xFF
				};

				saver.Begin( AsciiId<'P','D'>::R(0,0,id) ).Write( data ).End();
			}

			void Pad::LoadState(State::Loader& loader,const dword id)
			{
				if (id == AsciiId<'P','D'>::V)
				{
					State::Loader::Data<2> data( loader );

					strobe = data[0] & 0x1;
					stream = data[1] ^ 0xFF;

					timeStamp = 0;
				}
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Pad::BeginFrame(Controllers* i)
			{
				input = i;
				mic = 0;

				if (timeStamp)
					--timeStamp;
			}

			void Pad::Poll()
			{
				const uint nextStamp = cpu.GetCycles() / 0x10000;

				if (timeStamp <= nextStamp)
				{
					timeStamp = nextStamp;

					if (input)
					{
						Controllers::Pad& pad = input->pad[type - Api::Input::PAD1];
						input = NULL;

						if (Controllers::Pad::callback( pad, type - Api::Input::PAD1 ))
						{
							uint buttons = pad.buttons;

							enum
							{
								UP    = Controllers::Pad::UP,
								RIGHT = Controllers::Pad::RIGHT,
								DOWN  = Controllers::Pad::DOWN,
								LEFT  = Controllers::Pad::LEFT
							};

							if (!pad.allowSimulAxes)
							{
								if ((buttons & (UP|DOWN)) == (UP|DOWN))
									buttons &= (UP|DOWN) ^ 0xFFU;

								if ((buttons & (LEFT|RIGHT)) == (LEFT|RIGHT))
									buttons &= (LEFT|RIGHT) ^ 0xFFU;
							}

							state = buttons;
						}

						mic |= pad.mic;
					}
				}
			}

			uint Pad::Peek(uint port)
			{
				if (strobe == 0)
				{
					const uint data = stream;
					stream >>= 1;

					return (~data & 0x1) | (mic & ~port << 2);
				}
				else
				{
					Poll();
					return state & 0x1;
				}
			}

			void Pad::Poke(const uint data)
			{
				const uint prev = strobe;
				strobe = data & 0x1;

				if (prev > strobe)
				{
					Poll();
					stream = state ^ 0xFF;
				}
			}
		}
	}
}
