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
#include "NstBoardCamerica.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Camerica
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Bf9093::SubReset(bool)
				{
					Map( 0xC000U, 0xFFFFU, PRG_SWAP_16K_0 );
				}

				void Bf9096::SubReset(bool)
				{
					Map( 0x8000U, 0xBFFFU, &Bf9096::Poke_8000 );
					Map( 0xC000U, 0xFFFFU, &Bf9096::Poke_A000 );
				}

				void Bf9097::SubReset(const bool hard)
				{
					Bf9093::SubReset( hard );

					Map( 0x8000U, 0x9FFFU, &Bf9097::Poke_8000 );
				}

				void GoldenFive::SubReset(const bool hard)
				{
					Map( 0x8000U, 0x9FFFU, &GoldenFive::Poke_8000 );
					Map( 0xC000U, 0xFFFFU, &GoldenFive::Poke_C000 );

					if (hard)
						prg.SwapBank<SIZE_16K,0x4000>( 0x0F );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void Bf9096::SwapBasePrg(uint base)
				{
					prg.SwapBanks<SIZE_16K,0x0000>( base | (prg.GetBank<SIZE_16K,0x0000>() & 0x3), base | 0x3 );
				}

				NES_POKE_D(Bf9096,8000)
				{
					SwapBasePrg( data >> 1 & 0xC );
				}

				NES_POKE_D(Bf9096,A000)
				{
					prg.SwapBank<SIZE_16K,0x0000>( (prg.GetBank<SIZE_16K,0x0000>() & 0xC) | (data & 0x3) );
				}

				NES_POKE_D(Bf9097,8000)
				{
					ppu.SetMirroring( (data & 0x10) ? Ppu::NMT_1 : Ppu::NMT_0 );
				}

				NES_POKE_D(GoldenFive,8000)
				{
					if (data & 0x8)
					{
						prg.SwapBank<SIZE_16K,0x0000>( (data << 4 & 0x70) | (prg.GetBank<SIZE_16K,0x0000>() & 0x0F) );
						prg.SwapBank<SIZE_16K,0x4000>( (data << 4 & 0x70) | 0x0F );
					}
				}

				NES_POKE_D(GoldenFive,C000)
				{
					prg.SwapBank<SIZE_16K,0x0000>( (prg.GetBank<SIZE_16K,0x0000>() & 0x70) | (data & 0x0F) );
				}
			}
		}
	}
}
