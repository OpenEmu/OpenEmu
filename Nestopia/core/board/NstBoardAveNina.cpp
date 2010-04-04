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
#include "NstBoardAveNina.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Ave
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Nina001::SubReset(const bool hard)
				{
					Map( 0x7FFDU, PRG_SWAP_32K  );
					Map( 0x7FFEU, CHR_SWAP_4K_0 );
					Map( 0x7FFFU, CHR_SWAP_4K_1 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				void Nina06::SubReset(const bool hard)
				{
					for (uint i=0x4100; i < 0x6000; i += 0x200)
						Map( i+0x00, i+0xFF, &Nina06::Poke_4100 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Nina06,4100)
				{
					ppu.Update();
					chr.SwapBank<SIZE_8K,0x0000>( data );
					prg.SwapBank<SIZE_32K,0x0000>( data >> 3 );
				}
			}
		}
	}
}
