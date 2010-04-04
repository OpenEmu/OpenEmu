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
#include "NstBoardBtlGeniusMerioBros.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Btl
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void GeniusMerioBros::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x6FFFU, &GeniusMerioBros::Peek_6000  );
					Map( 0x7000U, 0x7FFFU, &GeniusMerioBros::Peek_7000, &GeniusMerioBros::Poke_7000 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(GeniusMerioBros,6000)
				{
					return *prg.Source().Mem(0x8000 + (address & 0x7FF));
				}

				NES_PEEK_A(GeniusMerioBros,7000)
				{
					return wrk[0][address & 0x7FF];
				}

				NES_POKE_AD(GeniusMerioBros,7000)
				{
					wrk[0][address & 0x7FF] = data;
				}
			}
		}
	}
}
