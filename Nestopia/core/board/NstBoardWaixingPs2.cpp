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
#include "NstBoardWaixing.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Waixing
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Ps2::SubReset(bool)
				{
					Map( 0x8000U, 0xFFFFU, &Ps2::Poke_8000 );

					prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(Ps2,8000)
				{
					ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_H : Ppu::NMT_V );

					const uint flip = data >> 7;
					data = data << 1 & 0xFE;

					switch (address & 0xFFF)
					{
						case 0x000:

							prg.SwapBanks<SIZE_8K,0x0000>( (data+0) ^ flip, (data+1) ^ flip, (data+2) ^ flip, (data+3) ^ flip );
							break;

						case 0x002:

							data |= flip;
							prg.SwapBanks<SIZE_8K,0x0000>( data, data, data, data );
							break;

						case 0x001:
						case 0x003:

							data |= flip;
							prg.SwapBanks<SIZE_8K,0x0000>( data, data+1, data + (~address >> 1 & 1), data+1 );
							break;
					}
				}
			}
		}
	}
}
