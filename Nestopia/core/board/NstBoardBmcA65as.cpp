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
#include "NstBoardBmcA65as.hpp"

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

				void A65as::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xFFFFU, &A65as::Poke_8000 );

					if (hard)
						NES_DO_POKE(8000,0x8000,0x00);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(A65as,8000)
				{
					if (data & 0x40)
						prg.SwapBank<SIZE_32K,0x0000>( data >> 1 );
					else
						prg.SwapBanks<SIZE_16K,0x0000>( (data >> 1 & 0x18) | (data & 0x7), (data >> 1 & 0x18) | 0x7 );

					ppu.SetMirroring
					(
						(data & 0x80) ? (data & 0x20) ? Ppu::NMT_1 : Ppu::NMT_0 :
										(data & 0x08) ? Ppu::NMT_H : Ppu::NMT_V
					);
				}
			}
		}
	}
}
