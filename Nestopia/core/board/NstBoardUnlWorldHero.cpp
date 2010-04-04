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

#include "../NstClock.hpp"
#include "NstBoard.hpp"
#include "NstBoardKonamiVrc4.hpp"
#include "NstBoardUnlWorldHero.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Unlicensed
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				WorldHero::WorldHero(const Context& c)
				: Board(c), irq(*c.cpu) {}

				void WorldHero::SubReset(const bool hard)
				{
					if (hard)
						prgSwap = 0;

					irq.Reset( hard, hard ? false : irq.Connected() );

					for (dword i=0x8000; i <= 0xFFFF; ++i)
					{
						switch (i & 0xF0C3)
						{
							case 0x8000: Map( i, &WorldHero::Poke_8000 ); break;
							case 0x9000: Map( i, NMT_SWAP_VH01         ); break;
							case 0x9002:
							case 0x9080: Map( i, &WorldHero::Poke_9000 ); break;
							case 0xA000: Map( i, PRG_SWAP_8K_1         ); break;
							case 0xB000:
							case 0xB001:
							case 0xB002:
							case 0xB003:
							case 0xC000:
							case 0xC001:
							case 0xC002:
							case 0xC003:
							case 0xD000:
							case 0xD001:
							case 0xD002:
							case 0xD003:
							case 0xE000:
							case 0xE001:
							case 0xE002:
							case 0xE003: Map( i, &WorldHero::Poke_B000 ); break;
							case 0xF000: Map( i, &WorldHero::Poke_F000 ); break;
							case 0xF001: Map( i, &WorldHero::Poke_F001 ); break;
							case 0xF002: Map( i, &WorldHero::Poke_F002 ); break;
							case 0xF003: Map( i, &WorldHero::Poke_F003 ); break;
						}
					}
				}

				void WorldHero::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'U','W','H'>::V) );

					if (baseChunk == AsciiId<'U','W','H'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:

									prgSwap = state.Read8() & 0x2;
									break;

								case AsciiId<'I','R','Q'>::V:

									irq.LoadState( state );
									break;
							}

							state.End();
						}
					}
				}

				void WorldHero::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'U','W','H'>::V );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( prgSwap ).End();
					irq.SaveState( state, AsciiId<'I','R','Q'>::V );
					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(WorldHero,8000)
				{
					prg.SwapBank<SIZE_8K>( prgSwap << 13, data );
				}

				NES_POKE_D(WorldHero,9000)
				{
					data &= 0x2;

					if (prgSwap != data)
					{
						prgSwap = data;

						prg.SwapBanks<SIZE_8K,0x0000>
						(
							prg.GetBank<SIZE_8K,0x4000>(),
							prg.GetBank<SIZE_8K,0x0000>()
						);
					}
				}

				NES_POKE_AD(WorldHero,B000)
				{
					ppu.Update();
					const bool part = address & 0x1;
					address = ((address - 0xB000) >> 1 & 0x1800) | (address << 9 & 0x0400);
					chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & (part ? 0x00F : 0xFF0)) | (part ? data << 4 : data & 0xF) );
				}

				NES_POKE_D(WorldHero,F000)
				{
					irq.WriteLatch0( data );
				}

				NES_POKE_D(WorldHero,F001)
				{
					irq.WriteLatch1( data );
				}

				NES_POKE_D(WorldHero,F002)
				{
					irq.Toggle( data );
				}

				NES_POKE(WorldHero,F003)
				{
					irq.Toggle();
				}

				void WorldHero::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
