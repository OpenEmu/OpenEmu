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
#include "NstBoardBmcGoldenGame260in1.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bmc
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				GoldenGame260in1::GoldenGame260in1(const Context& c)
				:
				Board (c),
				selector
				(
					prg.Source().Size() == SIZE_1024K ? 0 :
					prg.Source().Size() == SIZE_2048K ? 1 :
					prg.Source().Size() == SIZE_3072K ? 2 : 3
				)
				{
				}

				void GoldenGame260in1::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xFFFFU, &GoldenGame260in1::Poke_8000 );

					if (selector != 3)
						Map( 0x8000U, 0xFFFFU, &GoldenGame260in1::Peek_8000 );

					if (hard)
					{
						open = false;
						NES_DO_POKE(8000,0x8000,0x00);
					}
				}

				void GoldenGame260in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (selector != 3)
					{
						NST_VERIFY( baseChunk == (AsciiId<'B','G','G'>::V) );

						if (baseChunk == AsciiId<'B','G','G'>::V)
						{
							while (const dword chunk = state.Begin())
							{
								if (chunk == AsciiId<'B','U','S'>::V)
									open = state.Read8() & 0x1;

								state.End();
							}
						}
					}
				}

				void GoldenGame260in1::SubSave(State::Saver& state) const
				{
					if (selector != 3)
						state.Begin( AsciiId<'B','G','G'>::V ).Begin( AsciiId<'B','U','S'>::V ).Write8( open != 0 ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_A(GoldenGame260in1,8000)
				{
					ppu.SetMirroring
					(
						(address & 0x0400) ? Ppu::NMT_0 :
						(address & 0x2000) ? Ppu::NMT_H :
                                             Ppu::NMT_V
					);

					static const byte slots[4][4][2] =
					{
						{ {0x00,0}, {0x00,1}, {0x00,1}, {0x00,1} },
						{ {0x00,0}, {0x00,1}, {0x20,0}, {0x00,1} },
						{ {0x00,0}, {0x00,1}, {0x20,0}, {0x40,0} },
						{ {0x00,0}, {0x20,0}, {0x40,0}, {0x60,0} }
					};

					uint bank = slots[selector][address >> 8 & 0x3][0] | (address & 0x1F);
					open = slots[selector][address >> 8 & 0x3][1];

					if (address & 0x800)
					{
						bank = (bank << 1) | (address >> 12 & 0x1);
						prg.SwapBanks<SIZE_16K,0x0000>( bank, bank );
					}
					else
					{
						prg.SwapBank<SIZE_32K,0x0000>( bank );
					}
				}

				NES_PEEK_A(GoldenGame260in1,8000)
				{
					return !open ? prg.Peek(address - 0x8000) : (address >> 8);
				}
			}
		}
	}
}
