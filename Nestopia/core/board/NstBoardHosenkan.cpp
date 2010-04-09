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

#include "NstBoard.hpp"
#include "NstBoardMmc3.hpp"
#include "NstBoardHosenkan.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Hosenkan
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Standard::Standard(const Context& c)
				:
				Board (c),
				irq   (*c.cpu,*c.ppu,false)
				{}

				void Standard::SubReset(const bool hard)
				{
					if (hard)
						command = 0;

					irq.Reset( hard );

					for (uint i=0x0000; i < 0x1000; i += 0x4)
					{
						Map( 0x8001 + i, NMT_SWAP_HV );
						Map( 0xA000 + i, &Standard::Poke_A000 );
						Map( 0xC000 + i, &Standard::Poke_C000 );
						Map( 0xE003 + i, &Standard::Poke_E003 );
					}
				}

				void Standard::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'H','S','N'>::V) );

					if (baseChunk == AsciiId<'H','S','N'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:

									command = state.Read8();
									break;

								case AsciiId<'I','R','Q'>::V:

									irq.unit.LoadState( state );
									break;
							}

							state.End();
						}
					}
				}

				void Standard::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'H','S','N'>::V );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( command ).End();
					irq.unit.SaveState( state, AsciiId<'I','R','Q'>::V );
					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Standard,A000)
				{
					command = data;
				}

				NES_POKE_D(Standard,C000)
				{
					ppu.Update();

					switch (command & 0x7)
					{
						case 0x0: chr.SwapBank<SIZE_2K,0x0000>(data >> 1);  break;
						case 0x1: chr.SwapBank<SIZE_1K,0x1400>(data);       break;
						case 0x2: chr.SwapBank<SIZE_2K,0x0800>(data >> 1);  break;
						case 0x3: chr.SwapBank<SIZE_1K,0x1C00>(data);       break;
						case 0x4: prg.SwapBank<SIZE_8K,0x0000>(data);       break;
						case 0x5: prg.SwapBank<SIZE_8K,0x2000>(data);       break;
						case 0x6: chr.SwapBank<SIZE_1K,0x1000>(data);       break;
						case 0x7: chr.SwapBank<SIZE_1K,0x1800>(data);       break;
					}
				}

				NES_POKE_D(Standard,E003)
				{
					irq.Update();

					if (data)
					{
						irq.ClearIRQ();
						irq.unit.Enable();
						irq.unit.SetLatch( data );
						irq.unit.Reload();
					}
					else
					{
						irq.unit.Disable( cpu );
					}
				}

				void Standard::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
