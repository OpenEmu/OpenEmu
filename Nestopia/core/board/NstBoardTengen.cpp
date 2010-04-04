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

#include "../NstClock.hpp"
#include "NstBoard.hpp"
#include "NstBoardNRom.hpp"
#include "NstBoardNamcot.hpp"
#include "NstBoardSunsoft.hpp"
#include "NstBoardTengen.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Tengen
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void T800008::SubReset(bool)
				{
					Map( 0x8000U, 0xFFFFU, &T800008::Poke_8000 );
				}

				void T800037::SubReset(const bool hard)
				{
					Rambo1::SubReset( hard );

					for (uint i=0x0000; i < 0x1000; i += 0x2)
						Map( 0xA000 + i, NOP_POKE );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(T800008,8000)
				{
					ppu.Update();
					data = GetBusData(address,data);
					chr.SwapBank<SIZE_8K,0x0000>( data );
					prg.SwapBank<SIZE_32K,0x0000>( data >> 3 );
				}

				void T800037::UpdateChr() const
				{
					Rambo1::UpdateChr();

					nmt.SwapBanks<SIZE_1K,0x0000>
					(
						regs.chr[(regs.ctrl & 0x80U) ? 2 : 0] >> 7 ^ 1U,
						regs.chr[(regs.ctrl & 0x80U) ? 3 : 0] >> 7 ^ 1U,
						regs.chr[(regs.ctrl & 0x80U) ? 4 : 1] >> 7 ^ 1U,
						regs.chr[(regs.ctrl & 0x80U) ? 5 : 1] >> 7 ^ 1U
					);
				}
			}
		}
	}
}
