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
#include "NstBoardUnlMortalKombat2.hpp"

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

				MortalKombat2::MortalKombat2(const Context& c)
				: Board(c), irq(*c.cpu,*c.ppu,false) {}

				void MortalKombat2::SubReset(const bool hard)
				{
					irq.Reset( hard );

					for (uint i=0x0000; i < 0x1000; i += 0x4)
					{
						Map( 0x6000 + i, CHR_SWAP_2K_0 );
						Map( 0x6001 + i, CHR_SWAP_2K_1 );
						Map( 0x6002 + i, CHR_SWAP_2K_2 );
						Map( 0x6003 + i, CHR_SWAP_2K_3 );
						Map( 0x7000 + i, PRG_SWAP_8K_0 );
						Map( 0x7001 + i, PRG_SWAP_8K_1 );
						Map( 0x7002 + i, &MortalKombat2::Poke_7002 );
						Map( 0x7003 + i, &MortalKombat2::Poke_7003 );
					}
				}

				void MortalKombat2::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'U','M','2'>::V) );

					if (baseChunk == AsciiId<'U','M','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
								irq.unit.LoadState( state );

							state.End();
						}
					}
				}

				void MortalKombat2::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'U','M','2'>::V );
					irq.unit.SaveState( state, AsciiId<'I','R','Q'>::V );
					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE(MortalKombat2,7002)
				{
					irq.Update();
					irq.unit.Disable( cpu );
					irq.unit.SetLatch( 0 );
				}

				NES_POKE(MortalKombat2,7003)
				{
					irq.Update();
					irq.unit.Enable();
					irq.unit.SetLatch( 7 );
				}

				void MortalKombat2::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
