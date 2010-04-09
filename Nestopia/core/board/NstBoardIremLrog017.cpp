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
#include "NstBoardIremLrog017.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Irem
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Lrog017::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xFFFFU, &Lrog017::Poke_8000 );

					chr.Source(1).SwapBank<SIZE_2K,0x0800>( 0 );
					chr.Source(1).SwapBank<SIZE_2K,0x1000>( 1 );
					chr.Source(1).SwapBank<SIZE_2K,0x1800>( 2 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(Lrog017,8000)
				{
					ppu.Update();
					data = GetBusData(address,data);
					prg.SwapBank<SIZE_32K,0x0000>(data);
					chr.SwapBank<SIZE_2K,0x0000>(data >> 4);
				}
			}
		}
	}
}
