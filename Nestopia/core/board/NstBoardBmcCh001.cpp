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
#include "NstBoardBmcCh001.hpp"

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

				void Ch001::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xBFFFU, &Ch001::Peek_8000, &Ch001::Poke_8000 );
					Map( 0x8000U, 0xFFFFU, &Ch001::Poke_8000 );

					openBus = false;

					if (hard)
						NES_DO_POKE(8000,0x8000,0x00);
				}

				void Ch001::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','P','F'>::V) );

					if (baseChunk == AsciiId<'B','P','F'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
								openBus = state.Read8() & 0x1;

							state.End();
						}
					}
				}

				void Ch001::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','P','F'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( openBus ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(Ch001,8000)
				{
					return !openBus ? prg.Peek( address - 0x8000 ) : (address >> 8);
				}

				NES_POKE_A(Ch001,8000)
				{
					openBus = ((address & 0x300) == 0x300);

					prg.SwapBanks<SIZE_8K,0x0000>
					(
						(address >> 1 & 0x1FC) | ((address & 0x2) ? 0x0 : (address >> 1 & 0x2) | 0x0),
						(address >> 1 & 0x1FC) | ((address & 0x2) ? 0x1 : (address >> 1 & 0x2) | 0x1),
						(address >> 1 & 0x1FC) | ((address & 0x2) ? 0x2 : (address >> 1 & 0x2) | 0x0),

						(address & 0x800) ? (address >> 0 & 0x07C) | ((address & 0x6) ? 0x3 : 0x1) :
											(address >> 1 & 0x1FC) | ((address & 0x2) ? 0x3 : ((address >> 1 & 0x2) | 0x1))
					);

					ppu.SetMirroring( (address & 0x1) ? Ppu::NMT_H : Ppu::NMT_V );
				}
			}
		}
	}
}
