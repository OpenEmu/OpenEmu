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
#include "NstBoardJalecoJf11.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Jaleco
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Jf11::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x7FFFU, &Jf11::Poke_6000 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Jf11,6000)
				{
					ppu.Update();
					chr.SwapBank<SIZE_8K,0x0000>( data );
					prg.SwapBank<SIZE_32K,0x0000>( data >> 4 );
				}
			}
		}
	}
}
