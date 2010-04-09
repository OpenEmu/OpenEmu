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
#include "NstBoardUnlEdu2000.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Unlicensed
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Edu2000::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x7FFFU, &Edu2000::Peek_6000, &Edu2000::Poke_6000 );
					Map( 0x8000U, 0xFFFFU, &Edu2000::Poke_8000 );

					if (hard)
						NES_DO_POKE(8000,0x8000,0x00);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(Edu2000,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_POKE_AD(Edu2000,6000)
				{
					wrk[0][address - 0x6000] = data;
				}

				NES_POKE_D(Edu2000,8000)
				{
					prg.SwapBank<SIZE_32K,0x0000>( data );
					wrk.SwapBank<SIZE_8K,0x0000>( data >> 6 );
				}
			}
		}
	}
}
