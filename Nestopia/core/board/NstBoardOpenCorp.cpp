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
#include "NstBoardOpenCorp.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace OpenCorp
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Daou306::SubReset(bool)
				{
					Map( 0xC000U, CHR_SWAP_1K_0  );
					Map( 0xC001U, CHR_SWAP_1K_1  );
					Map( 0xC002U, CHR_SWAP_1K_2  );
					Map( 0xC003U, CHR_SWAP_1K_3  );
					Map( 0xC008U, CHR_SWAP_1K_4  );
					Map( 0xC009U, CHR_SWAP_1K_5  );
					Map( 0xC00AU, CHR_SWAP_1K_6  );
					Map( 0xC00BU, CHR_SWAP_1K_7  );
					Map( 0xC010U, PRG_SWAP_16K_0 );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif
			}
		}
	}
}
