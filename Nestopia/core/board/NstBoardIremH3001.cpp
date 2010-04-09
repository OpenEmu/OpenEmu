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
#include "../NstClock.hpp"
#include "NstBoardIremH3001.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Irem
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				H3001::H3001(const Context& c)
				: Board(c), irq(*c.cpu) {}

				void H3001::Irq::Reset(const bool hard)
				{
					if (hard)
					{
						enabled = false;
						count = 0;
						latch = 0;
					}
				}

				void H3001::SubReset(const bool hard)
				{
					irq.Reset( hard, true );

					Map( 0x9001U, &H3001::Poke_9001 );
					Map( 0x9003U, &H3001::Poke_9003 );
					Map( 0x9004U, &H3001::Poke_9004 );
					Map( 0x9005U, &H3001::Poke_9005 );
					Map( 0x9006U, &H3001::Poke_9006 );

					Map( 0x8000U, PRG_SWAP_8K_0 );
					Map( 0xA000U, PRG_SWAP_8K_1 );
					Map( 0xC000U, PRG_SWAP_8K_2 );
					Map( 0xB000U, CHR_SWAP_1K_0 );
					Map( 0xB001U, CHR_SWAP_1K_1 );
					Map( 0xB002U, CHR_SWAP_1K_2 );
					Map( 0xB003U, CHR_SWAP_1K_3 );
					Map( 0xB004U, CHR_SWAP_1K_4 );
					Map( 0xB005U, CHR_SWAP_1K_5 );
					Map( 0xB006U, CHR_SWAP_1K_6 );
					Map( 0xB007U, CHR_SWAP_1K_7 );
				}

				void H3001::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'I','H','3'>::V) );

					if (baseChunk == AsciiId<'I','H','3'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
							{
								State::Loader::Data<5> data( state );

								irq.unit.enabled = data[0] & 0x1;
								irq.unit.latch = data[1] | data[2] << 8;
								irq.unit.count = data[3] | data[4] << 8;
							}

							state.End();
						}
					}
				}

				void H3001::SubSave(State::Saver& state) const
				{
					const byte data[5] =
					{
						irq.unit.enabled ? 0x1 : 0x0,
						irq.unit.latch & 0xFF,
						irq.unit.latch >> 8,
						irq.unit.count & 0xFF,
						irq.unit.count >> 8
					};

					state.Begin( AsciiId<'I','H','3'>::V ).Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(H3001,9001)
				{
					ppu.SetMirroring( (data & 0x80) ? Ppu::NMT_H : Ppu::NMT_V );
				}

				NES_POKE_D(H3001,9003)
				{
					irq.Update();
					irq.unit.enabled = data & 0x80;
					irq.ClearIRQ();
				}

				NES_POKE(H3001,9004)
				{
					irq.Update();
					irq.unit.count = irq.unit.latch;
					irq.ClearIRQ();
				}

				NES_POKE_D(H3001,9005)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0x00FF) | data << 8;
				}

				NES_POKE_D(H3001,9006)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xFF00) | data << 0;
				}

				bool H3001::Irq::Clock()
				{
					if (enabled && count && !--count)
					{
						enabled = false;
						return true;
					}

					return false;
				}

				void H3001::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
