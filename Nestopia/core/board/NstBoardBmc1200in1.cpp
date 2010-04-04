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
#include "NstBoardBmc1200in1.hpp"

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

				void B1200in1::SubReset(bool)
				{
					Map( 0x8000U, 0xFFFFU, &B1200in1::Poke_8000 );
					NES_DO_POKE(8000,0x8000,0x00);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_A(B1200in1,8000)
				{
					const uint bank = (address >> 4 & 0x10) | (address >> 3 & 0x0F);

					if (address & 0x1)
					{
						prg.SwapBank<SIZE_32K,0x0000>( bank );
					}
					else
					{
						const uint offset = (bank << 1) | (address >> 2 & 0x1);
						prg.SwapBanks<SIZE_16K,0x0000>( offset, offset );
					}

					if (!(address & 0x80))
						prg.SwapBank<SIZE_16K,0x4000>( ((address & 0x200) ? 0x7 : 0x0) | (bank << 1 & 0x38) );

					ppu.SetMirroring( (address & 0x2) ? Ppu::NMT_H : Ppu::NMT_V );
				}
			}
		}
	}
}
