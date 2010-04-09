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
#include "NstBoardNitra.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Nitra
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Tda::SubReset(const bool hard)
				{
					Mmc3::SubReset( hard );

					Map( 0x8000U, 0xFFFFU, &Tda::Poke_8000 );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(Tda,8000)
				{
					data = address & 0xFF;
					address = (address & 0xE000) | (address >> 10 & 0x1);

					switch (address)
					{
						case 0x8000: Mmc3::NES_DO_POKE( 8000, address, data ); break;
						case 0x8001: Mmc3::NES_DO_POKE( 8001, address, data ); break;
						case 0xA000: SetMirroringHV( data );                   break;
						case 0xA001: Mmc3::NES_DO_POKE( A001, address, data ); break;
						case 0xC000: Mmc3::NES_DO_POKE( C000, address, data ); break;
						case 0xC001: Mmc3::NES_DO_POKE( C001, address, data ); break;
						case 0xE000: Mmc3::NES_DO_POKE( E000, address, data ); break;
						case 0xE001: Mmc3::NES_DO_POKE( E001, address, data ); break;
					}
				}
			}
		}
	}
}
