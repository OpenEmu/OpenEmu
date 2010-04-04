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
#include "NstBoardRcm.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Rcm
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Gs2015::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xFFFFU, &Gs2015::Poke_8000 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				void Gs2013::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x7FFFU, &Gs2013::Peek_6000 );
					Map( 0x8000U, 0xFFFFU, &Gs2013::Poke_8000 );

					if (hard)
					{
						wrk.SwapBank<SIZE_8K,0x0000>( 0x1F );
						prg.SwapBank<SIZE_32K,0x0000>( ~0U );
					}
				}

				void Gs2004::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x7FFFU, &Gs2004::Peek_6000 );
					Map( 0x8000U, 0xFFFFU, PRG_SWAP_32K       );

					if (hard)
					{
						wrk.SwapBank<SIZE_8K,0x0000>( ~0U );
						prg.SwapBank<SIZE_32K,0x0000>( prg.Source().Size() / SIZE_32K - 1 );
					}
				}

				void TetrisFamily::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xFFFFU, &TetrisFamily::Poke_8000 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(Gs2013,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_PEEK_A(Gs2004,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_POKE_D(Gs2013,8000)
				{
					prg.SwapBank<SIZE_32K,0x0000>( (data & 0x8) ? (data & 0x9) : (data & 0x7) );
				}

				NES_POKE_A(Gs2015,8000)
				{
					ppu.Update();
					prg.SwapBank<SIZE_32K,0x0000>( address );
					chr.SwapBank<SIZE_8K,0x0000>( address >> 1 );
				}

				NES_POKE_A(TetrisFamily,8000)
				{
					ppu.SetMirroring( (address & 0x80) ? Ppu::NMT_H : Ppu::NMT_V );

					switch (address & 0x30)
					{
						case 0x00:
						case 0x30:

							prg.SwapBank<SIZE_32K,0x0000>( address & 0xF );
							break;

						case 0x20:
						case 0x10:

							address = (address << 1 & 0x1E) | (address >> 4 & 0x02);
							prg.SwapBanks<SIZE_16K,0x0000>( address, address );
							break;
					}
				}
			}
		}
	}
}
