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
#include "NstBoardBtlDragonNinja.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Btl
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				DragonNinja::DragonNinja(const Context& c)
				: Board(c), irq(*c.cpu,*c.ppu) {}

				void DragonNinja::Irq::Reset(bool)
				{
					count = 0;
				}

				void DragonNinja::SubReset(bool)
				{
					irq.Reset( true );

					for (uint i=0x0000; i < 0x1000; i += 0x4)
					{
						Map( 0x8000 + i, PRG_SWAP_8K_0 );
						Map( 0x9000 + i, NMT_SWAP_HV   );
						Map( 0xA000 + i, PRG_SWAP_8K_1 );
						Map( 0xB000 + i, CHR_SWAP_1K_0 );
						Map( 0xB002 + i, CHR_SWAP_1K_1 );
						Map( 0xC000 + i, CHR_SWAP_1K_2 );
						Map( 0xC002 + i, CHR_SWAP_1K_3 );
						Map( 0xD000 + i, CHR_SWAP_1K_4 );
						Map( 0xD002 + i, CHR_SWAP_1K_5 );
						Map( 0xE000 + i, CHR_SWAP_1K_6 );
						Map( 0xE002 + i, CHR_SWAP_1K_7 );
						Map( 0xF000 + i, &DragonNinja::Poke_F000 );
					}
				}

				void DragonNinja::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','D','N'>::V) );

					if (baseChunk == AsciiId<'B','D','N'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
								irq.unit.count = state.Read8();

							state.End();
						}
					}
				}

				void DragonNinja::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','D','N'>::V ).Begin( AsciiId<'I','R','Q'>::V ).Write8( irq.unit.count ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				bool DragonNinja::Irq::Clock()
				{
					if (!count || ++count < 240)
						return false;

					count = 0;
					return true;
				}

				NES_POKE_D(DragonNinja,F000)
				{
					irq.Update();
					irq.ClearIRQ();
					irq.unit.count = data;
				}

				void DragonNinja::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}

			}
		}
	}
}
