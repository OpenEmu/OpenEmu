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
#include "NstBoardKonamiVsSystem.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Konami
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void VsSystem::SubReset(bool)
				{
					Map( 0x8000U, 0x8FFFU, PRG_SWAP_8K_0 );
					Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_1 );
					Map( 0xC000U, 0xCFFFU, PRG_SWAP_8K_2 );
					Map( 0xE000U, 0xEFFFU, CHR_SWAP_4K_0 );
					Map( 0xF000U, 0xFFFFU, CHR_SWAP_4K_1 );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif
			}
		}
	}
}
