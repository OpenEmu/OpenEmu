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
#include "NstBoardCnePsb.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Cne
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Psb::SubReset(bool)
				{
					for (uint i=0x6000; i < 0x6800; i += 0x8)
					{
						Map( i + 0x0, PRG_SWAP_8K_0 );
						Map( i + 0x1, PRG_SWAP_8K_1 );
						Map( i + 0x2, PRG_SWAP_8K_2 );
						Map( i + 0x3, PRG_SWAP_8K_3 );
						Map( i + 0x4, CHR_SWAP_2K_0 );
						Map( i + 0x5, CHR_SWAP_2K_1 );
						Map( i + 0x6, CHR_SWAP_2K_2 );
						Map( i + 0x7, CHR_SWAP_2K_3 );
					}
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif
			}
		}
	}
}
