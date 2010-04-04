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
#include "NstBoardSunsoft2.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sunsoft
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void S2a::SubReset(bool)
				{
					Map( 0x8000U, 0xFFFFU, &S2a::Poke_8000 );
				}

				void S2b::SubReset(bool)
				{
					Map( 0x8000U, 0xFFFFU, &S2b::Poke_8000 );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(S2a,8000)
				{
					ppu.Update();
					data = GetBusData(address,data);
					prg.SwapBank<SIZE_16K,0x0000>( data >> 4 );
					chr.SwapBank<SIZE_8K,0x0000>( (data >> 4 & 0x8) | (data & 0x7) );
				}

				NES_POKE_AD(S2b,8000)
				{
					data = GetBusData(address,data);
					ppu.SetMirroring( (data & 0x8) ? Ppu::NMT_1 : Ppu::NMT_0 );
					prg.SwapBank<SIZE_16K,0x0000>( data >> 4 );
					chr.SwapBank<SIZE_8K,0x0000>( (data >> 4 & 0x8) | (data & 0x7) );
				}
			}
		}
	}
}
