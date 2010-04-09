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
#include "NstBoardMmc3.hpp"
#include "NstBoardUnlSuperFighter3.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Unlicensed
			{
				void NST_FASTCALL SuperFighter3::UpdateChr(uint address,uint bank) const
				{
					switch (address)
					{
						case 0x0000: chr.SwapBank<SIZE_4K,0x0000>( bank >> 1 ); break;
						case 0x1000: chr.SwapBank<SIZE_2K,0x1000>( bank >> 0 ); break;
						case 0x1400: chr.SwapBank<SIZE_2K,0x1800>( bank >> 0 ); break;
					}
				}
			}
		}
	}
}
