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
#include "NstBoardBmcVrc4.hpp"

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

				void Vrc4::SubReset(const bool hard)
				{
					Konami::Vrc4::SubReset( hard );

					Map( 0x8000U, 0x8FFFU, &Vrc4::Poke_8000 );
					Map( 0xA000U, 0xAFFFU, &Vrc4::Poke_A000 );
					Map( 0xB000U, 0xEFFFU, &Vrc4::Poke_B000 );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Vrc4,8000)
				{
					prg.SwapBank<SIZE_8K>( prgSwap << 13, (prg.GetBank<SIZE_8K,0x0000>() & 0x20) | (data & 0x1F) );
				}

				NES_POKE_D(Vrc4,A000)
				{
					prg.SwapBank<SIZE_8K,0x2000>( (prg.GetBank<SIZE_8K,0x0000>() & 0x20) | (data & 0x1F) );
				}

				NES_POKE_D(Vrc4,B000)
				{
					data = data << 2 & 0x20;

					prg.SwapBanks<SIZE_8K,0x0000>
					(
						data | (prg.GetBank<SIZE_8K,0x0000>() & 0x1F),
						data | (prg.GetBank<SIZE_8K,0x2000>() & 0x1F),
						data | (prg.GetBank<SIZE_8K,0x4000>() & 0x1F),
						data | (prg.GetBank<SIZE_8K,0x6000>() & 0x1F)
					);
				}
			}
		}
	}
}
