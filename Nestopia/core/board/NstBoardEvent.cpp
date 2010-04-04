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

#include <cstring>
#include "NstBoard.hpp"
#include "../NstClock.hpp"
#include "NstBoardMmc1.hpp"
#include "NstBoardEvent.hpp"
#include "../api/NstApiUser.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Event::CartSwitches::CartSwitches()
			: time(DEFAULT_DIP), showTime(true) {}

			Event::Event(const Context& c)
			: Mmc1(c,REV_B2), irq(*c.cpu)
			{
				NST_COMPILE_ASSERT( TIME_TEXT_MIN_OFFSET == 11 && TIME_TEXT_SEC_OFFSET == 13 );
				std::strcpy( text, "Time left: x:xx" );
			}

			uint Event::CartSwitches::NumDips() const
			{
				return 2;
			}

			uint Event::CartSwitches::NumValues(uint dip) const
			{
				NST_ASSERT( dip < 2 );
				return (dip == 0) ? 16 : 2;
			}

			cstring Event::CartSwitches::GetDipName(uint dip) const
			{
				NST_ASSERT( dip < 2 );
				return (dip == 0) ? "Time" : "Show Time";
			}

			cstring Event::CartSwitches::GetValueName(uint dip,uint value) const
			{
				NST_ASSERT( dip < 2 );

				if (dip == 0)
				{
					NST_ASSERT( value < 16 );

					static const char times[16][7] =
					{
						"5:00.4",
						"5:19.2",
						"5:38.0",
						"5:56.7",
						"6:15.5",
						"6:34.3",
						"6:53.1",
						"7:11.9",
						"7:30.6",
						"7:49.4",
						"8:08.2",
						"8:27.0",
						"8:45.8",
						"9:04.5",
						"9:23.3",
						"9:42.1"
					};

					return times[value];
				}
				else
				{
					NST_ASSERT( value < 2 );

					return (value == 0) ? "no" : "yes";
				}
			}

			uint Event::CartSwitches::GetValue(uint dip) const
			{
				NST_ASSERT( dip < 2 );
				return (dip == 0) ? time : showTime;
			}

			void Event::CartSwitches::SetValue(uint dip,uint value)
			{
				NST_ASSERT( dip < 2 );

				if (dip == 0)
				{
					NST_ASSERT( value < 16 );
					time = value;
				}
				else
				{
					NST_ASSERT( value < 2 );
					showTime = value;
				}
			}

			inline dword Event::CartSwitches::GetTime() const
			{
				return BASE_TIME * (time + 16UL) - 1;
			}

			inline bool Event::CartSwitches::ShowTime() const
			{
				return showTime;
			}

			Event::Device Event::QueryDevice(DeviceType type)
			{
				if (type == DEVICE_DIP_SWITCHES)
					return &cartSwitches;
				else
					return Board::QueryDevice( type );
			}

			void Event::SubReset(const bool hard)
			{
				irq.Reset( hard, true );
				time = 0;

				Mmc1::SubReset( hard );

				prg.SwapBank<SIZE_16K,0x4000>( 1 );
			}

			void Event::SubLoad(State::Loader& state,const dword baseChunk)
			{
				time = 0;

				if (baseChunk == AsciiId<'E','V','T'>::V)
				{
					irq.unit.count = 0;

					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'I','R','Q'>::V)
							irq.unit.count = state.Read32();

						state.End();
					}
				}
				else
				{
					Mmc1::SubLoad( state, baseChunk );
				}
			}

			void Event::SubSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'E','V','T'>::V ).Begin( AsciiId<'I','R','Q'>::V ).Write32( irq.unit.count ).End().End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void NST_FASTCALL Event::UpdateRegisters(const uint index)
			{
				NST_ASSERT( index < 4 );

				if (index != 2)
				{
					if (regs[1] & 0x8U)
					{
						switch (regs[0] & 0xCU)
						{
							case 0x0:
							case 0x4:

								prg.SwapBank<SIZE_32K,0x0000>( 0x4 | (regs[3] >> 1 & 0x3U) );
								break;

							case 0x8:

								prg.SwapBanks<SIZE_16K,0x0000>( 0x8, 0x8 | (regs[3] & 0x7U) );
								break;

							case 0xC:

								prg.SwapBanks<SIZE_16K,0x0000>( 0x8 | (regs[3] & 0x7U), 0xF );
								break;
						}
					}
					else
					{
						prg.SwapBank<SIZE_32K,0x0000>( regs[1] >> 1 & 0x3U );
					}

					UpdateWrk();

					if (index == 0)
					{
						UpdateNmt();
					}
					else
					{
						irq.Update();

						if (regs[1] & 0x10U)
						{
							irq.unit.count = 0;
							irq.ClearIRQ();
						}
						else if (irq.unit.count == 0)
						{
							irq.unit.count = cartSwitches.GetTime();
						}
					}
				}
			}

			void Event::Irq::Reset(bool)
			{
				count = 0;
			}

			bool Event::Irq::Clock()
			{
				return count && --count == 0;
			}

			void Event::Sync(Board::Event event,Input::Controllers* controllers)
			{
				if (event == EVENT_END_FRAME)
				{
					if (cartSwitches.ShowTime() && irq.unit.count)
					{
						const dword t =
						(
							(cpu.GetModel() == CPU_RP2A03) ? irq.unit.count * qword( CPU_RP2A03_CC * CLK_NTSC_DIV ) / CLK_NTSC :
                                                             irq.unit.count * qword( CPU_RP2A07_CC * CLK_PAL_DIV  ) / CLK_PAL
						);

						if (time != t)
						{
							time = t;

							text[TIME_TEXT_MIN_OFFSET+0] = '0' + t / 60;
							text[TIME_TEXT_SEC_OFFSET+0] = '0' + t % 60 / 10;
							text[TIME_TEXT_SEC_OFFSET+1] = '0' + t % 60 % 10;

							Api::User::eventCallback( Api::User::EVENT_DISPLAY_TIMER, text );
						}
					}

					irq.VSync();
					Mmc1::Sync( event, controllers );
				}
			}
		}
	}
}
