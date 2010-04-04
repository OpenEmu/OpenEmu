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
#include "NstBoardCxRom.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			CnRom::Ce::Ce(const Context& c)
			: mask(0x0), state(0x0)
			{
				if (c.chr.Pin(26) == L"CE")
				{
					mask |= 0x1;
					state |= 0x1;
				}
				else if (c.chr.Pin(26) == L"/CE")
				{
					mask |= 0x1;
					state |= 0x0;
				}

				if (c.chr.Pin(27) == L"CE")
				{
					mask |= 0x2;
					state |= 0x2;
				}
				else if (c.chr.Pin(27) == L"/CE")
				{
					mask |= 0x2;
					state |= 0x0;
				}
			}

			CnRom::CnRom(const Context& c)
			: Board(c), ce(c) {}

			void CnRom::SubReset(bool)
			{
				if (ce.mask)
				{
					Map( 0x8000U, 0xFFFFU, &CnRom::Poke_8000 );
				}
				else if (board == Type::STD_CNROM)
				{
					Map( CHR_SWAP_8K_BC );
				}
				else
				{
					Map( 0x8000U, 0xFFFFU, CHR_SWAP_8K );
				}
			}

			void CpRom::SubReset(const bool hard)
			{
				Map( CHR_SWAP_4K_1_BC );

				if (hard)
					chr.SwapBank<SIZE_4K,0x1000>(0);
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_ACCESSOR(CnRom,ChrOpenBus)
			{
				return 0xFF;
			}

			NES_POKE_AD(CnRom,8000)
			{
				data = GetBusData(address,data);

				ppu.Update();

				chr.SwapBank<SIZE_8K,0x0000>( data & ~ce.mask );

				if ((data & ce.mask) == ce.state)
					chr.ResetAccessors();
				else
					chr.SetAccessors( this, &CnRom::Access_ChrOpenBus, &CnRom::Access_ChrOpenBus );
			}
		}
	}
}
