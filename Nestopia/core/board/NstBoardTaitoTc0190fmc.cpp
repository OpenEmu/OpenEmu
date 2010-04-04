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
#include "NstBoardTaitoTc0190fmc.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Taito
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Tc0190fmc::SubReset(bool)
				{
					for (uint i=0x0000; i < 0x1000; i += 0x4)
					{
						Map( 0x8000 + i, &Tc0190fmc::Poke_8000 );
						Map( 0x8001 + i, PRG_SWAP_8K_1 );
						Map( 0x8002 + i, CHR_SWAP_2K_0 );
						Map( 0x8003 + i, CHR_SWAP_2K_1 );
						Map( 0xA000 + i, CHR_SWAP_1K_4 );
						Map( 0xA001 + i, CHR_SWAP_1K_5 );
						Map( 0xA002 + i, CHR_SWAP_1K_6 );
						Map( 0xA003 + i, CHR_SWAP_1K_7 );
					}
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Tc0190fmc,8000)
				{
					prg.SwapBank<SIZE_8K,0x0000>( data );
					ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_H : Ppu::NMT_V );
				}
			}
		}
	}
}
