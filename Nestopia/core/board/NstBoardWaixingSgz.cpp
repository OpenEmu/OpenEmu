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
#include "NstBoardWaixing.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Waixing
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Sgz::Sgz(const Context& c)
				: Board(c), irq(*c.cpu) {}

				void Sgz::SubReset(const bool hard)
				{
					irq.Reset( hard, hard ? false : irq.Connected() );

					Map( 0x8000U, 0x8FFFU, PRG_SWAP_8K_0 );
					Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_1 );
					Map( 0xB000U, 0xEFFFU, &Sgz::Poke_B000 );

					for (uint i=0x0000; i < 0x1000; i += 0x10)
					{
						Map( 0xF000 + i, 0xF003 + i, &Sgz::Poke_F000 );
						Map( 0xF004 + i, 0xF007 + i, &Sgz::Poke_F004 );
						Map( 0xF008 + i, 0xF00B + i, &Sgz::Poke_F008 );
						Map( 0xF00C + i, 0xF00F + i, &Sgz::Poke_F00C );
					}

					// hack
					chr.Source().WriteEnable( true );
				}

				void Sgz::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'W','S','Z'>::V) );

					if (baseChunk == AsciiId<'W','S','Z'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
								irq.LoadState( state );

							state.End();
						}
					}
				}

				void Sgz::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'W','S','Z'>::V );
					irq.SaveState( state, AsciiId<'I','R','Q'>::V );
					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(Sgz,B000)
				{
					ppu.Update();
					const uint part = address & 0x4;
					address = ((address - 0xB000) >> 1 & 0x1800) | (address << 7 & 0x0400);
					chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & 0xF0U >> part) | (data & 0xF) << part );
				}

				NES_POKE_D(Sgz,F000)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xF0) | (data & 0x0F);
				}

				NES_POKE_D(Sgz,F004)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0x0F) | (data & 0x0F) << 4;
				}

				NES_POKE_D(Sgz,F008)
				{
					irq.Toggle( data );
				}

				NES_POKE(Sgz,F00C)
				{
					irq.Toggle();
				}

				void Sgz::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
