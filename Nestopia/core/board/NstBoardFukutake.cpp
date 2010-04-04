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

#include <cstring>
#include "NstBoard.hpp"
#include "NstBoardFukutake.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Fukutake
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Sbx::SubReset(const bool hard)
				{
					Map( 0x4200U, 0x4201U, &Sbx::Peek_4200 );
					Map( 0x4202U,          &Sbx::Peek_4202 );
					Map( 0x4203U,          &Sbx::Peek_4200 );
					Map( 0x4204U, 0x43FFU, &Sbx::Peek_4204 );

					for (uint i=0x4200; i < 0x4400; i += 0x2)
					{
						Map( i + 0x0, &Sbx::Poke_4200 );
						Map( i + 0x1, PRG_SWAP_16K_0  );
					}

					if (board.GetWram() >= SIZE_1K)
						Map( 0x4400U, 0x4EFFU, &Sbx::Peek_4400, &Sbx::Poke_4400 );

					Map( 0x6000U, 0x7FFFU, &Sbx::Peek_6000 );

					if (hard)
					{
						wrk.Source(1).SwapBank<SIZE_8K,0x0000>(0);
						prg.SwapBanks<SIZE_16K,0x0000>(0,0);
					}
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK(Sbx,4200)
				{
					return 0x00;
				}

				NES_PEEK(Sbx,4202)
				{
					return 0x40;
				}

				NES_PEEK(Sbx,4204)
				{
					return 0xFF;
				}

				NES_POKE_D(Sbx,4200)
				{
					wrk.Source(1).SwapBank<SIZE_8K,0x0000>( data >> 6 );
				}

				NES_PEEK_A(Sbx,4400)
				{
					return *wrk.Source(0).Mem(address - 0x4400);
				}

				NES_POKE_AD(Sbx,4400)
				{
					*wrk.Source(0).Mem(address - 0x4400) = data;
				}

				NES_PEEK_A(Sbx,6000)
				{
					return wrk[0][address - 0x6000];
				}
			}
		}
	}
}
