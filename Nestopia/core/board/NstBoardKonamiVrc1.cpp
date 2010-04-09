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
#include "NstBoardKonamiVrc1.hpp"

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

				void Vrc1::SubReset(bool)
				{
					Map( 0x8000U, 0x8FFFU, PRG_SWAP_8K_0    );
					Map( 0x9000U, 0x9FFFU, &Vrc1::Poke_9000 );
					Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_1    );
					Map( 0xC000U, 0xCFFFU, PRG_SWAP_8K_2    );
					Map( 0xE000U, 0xEFFFU, &Vrc1::Poke_E000 );
					Map( 0xF000U, 0xFFFFU, &Vrc1::Poke_F000 );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Vrc1,9000)
				{
					ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_H : Ppu::NMT_V );

					chr.SwapBanks<SIZE_4K,0x0000>
					(
						(data << 3 & 0x10) | (chr.GetBank<SIZE_4K,0x0000>() & 0xF),
						(data << 2 & 0x10) | (chr.GetBank<SIZE_4K,0x1000>() & 0xF)
					);
				}

				NES_POKE_D(Vrc1,E000)
				{
					ppu.Update();
					chr.SwapBank<SIZE_4K,0x0000>( (chr.GetBank<SIZE_4K,0x0000>() & 0x10) | (data & 0xF) );
				}

				NES_POKE_D(Vrc1,F000)
				{
					ppu.Update();
					chr.SwapBank<SIZE_4K,0x1000>( (chr.GetBank<SIZE_4K,0x1000>() & 0x10) | (data & 0xF) );
				}
			}
		}
	}
}
