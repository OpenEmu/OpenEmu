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
#include "NstBoardTxRom.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void TksRom::SubReset(const bool hard)
			{
				Mmc3::SubReset( hard );

				for (uint i=0x0000; i < 0x2000; i += 0x2)
					Map( 0xA000 + i, NOP_POKE );
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void NST_FASTCALL TksRom::UpdateChr(uint address,uint bank) const
			{
				if (address < 0x1000)
					nmt.SwapBank<SIZE_1K>( address, bank >> 7 ^ 0x1 );

				Mmc3::UpdateChr( address, bank );
			}

			void NST_FASTCALL TqRom::UpdateChr(uint address,uint bank) const
			{
				chr.Source( bank >> 6 & 0x1 ).SwapBank<SIZE_1K>( address, bank );
			}
		}
	}
}
