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
#include "NstBoardBtlShuiGuanPipe.hpp"

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

				ShuiGuanPipe::ShuiGuanPipe(const Context& c)
				: Board(c), irq(*c.cpu) {}

				void ShuiGuanPipe::Irq::Reset(const bool hard)
				{
					if (hard)
					{
						enabled = false;
						count[0] = 0;
						count[1] = 0;
					}
				}

				void ShuiGuanPipe::SubReset(const bool hard)
				{
					irq.Reset( hard, true );

					Map( 0x6000U, 0x7FFFU, &ShuiGuanPipe::Peek_6000 );

					for (uint i=0x0000; i < 0x0800; i += 0x10)
					{
						Map( 0x8800 + i, 0x8803 + i, PRG_SWAP_8K_0 );
						Map( 0x9800 + i, 0x9803 + i, NMT_SWAP_VH01 );
						Map( 0xA000 + i, 0xA003 + i, PRG_SWAP_8K_2 );
						Map( 0xA800 + i, 0xA803 + i, PRG_SWAP_8K_1 );
					}

					for (uint i=0x000; i < 0x1000; i += 0x10)
					{
						Map( 0xB000 + i, 0xE00F + i, &ShuiGuanPipe::Poke_B000 );
						Map( 0xF000 + i, 0xF003 + i, &ShuiGuanPipe::Poke_F000 );
						Map( 0xF004 + i, 0xF007 + i, &ShuiGuanPipe::Poke_F004 );
						Map( 0xF008 + i, 0xF00B + i, &ShuiGuanPipe::Poke_F008 );
					}
				}

				void ShuiGuanPipe::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','S','P'>::V) );

					if (baseChunk == AsciiId<'B','S','P'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
							{
								State::Loader::Data<2> data( state );

								irq.unit.enabled = data[0] & 0x1;
								irq.unit.count[1] = data[1];
							}

							state.End();
						}
					}
				}

				void ShuiGuanPipe::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','S','P'>::V ).Begin( AsciiId<'I','R','Q'>::V ).Write16( (irq.unit.enabled != false) | (irq.unit.count[1] & 0xFF) << 8 ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				bool ShuiGuanPipe::Irq::Clock()
				{
					if (++count[0] < 114)
						return false;

					count[0] = 0;
					return enabled && (++count[1] & 0xFF) == 0;
				}

				NES_PEEK_A(ShuiGuanPipe,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_POKE_AD(ShuiGuanPipe,B000)
				{
					ppu.Update();
					const uint part = address & 0x4;
					address = ((address - 0x3000) >> 1 | (address << 7)) & 0x1C00;
					chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & 0xF0 >> part) | (data & 0x0F) << part );
				}

				NES_POKE_D(ShuiGuanPipe,F000)
				{
					irq.Update();
					irq.unit.count[1] = (irq.unit.count[1] & 0xF0) | (data << 0 & 0x0F);
				}

				NES_POKE_D(ShuiGuanPipe,F004)
				{
					irq.Update();
					irq.unit.count[1] = (irq.unit.count[1] & 0x0F) | (data << 4 & 0xF0);
				}

				NES_POKE_D(ShuiGuanPipe,F008)
				{
					irq.Update();
					irq.unit.enabled = data;

					if (!data)
						irq.ClearIRQ();
				}

				void ShuiGuanPipe::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
					{
						irq.VSync();
						irq.unit.count[0] = 0;
					}

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
