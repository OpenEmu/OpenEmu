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
#include "NstBoardDiscrete.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Discrete
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Ic74x161x161x32::SubReset(bool)
				{
					if (board == Type::DISCRETE_74_161_161_32_A)
						Map( 0x8000U, 0xFFFFU, &Ic74x161x161x32::Poke_8000_0 );
					else
						Map( 0x8000U, 0xFFFFU, &Ic74x161x161x32::Poke_8000_1 );
				}

				void Ic74x139x74::SubReset(bool)
				{
					Map( 0x6000U, 0x7FFFU, &Ic74x139x74::Poke_6000 );
				}

				void Ic74x161x138::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x7FFFU, &Ic74x161x138::Poke_6000 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				void Ic74x377::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xFFFFU, &Ic74x377::Poke_8000 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(Ic74x161x161x32,8000_0)
				{
					ppu.Update();
					data = GetBusData(address,data);
					chr.SwapBank<SIZE_8K,0x0000>( data );
					prg.SwapBank<SIZE_16K,0x0000>( data >> 4 );
				}

				NES_POKE_AD(Ic74x161x161x32,8000_1)
				{
					data = GetBusData(address,data);
					ppu.SetMirroring( (data & 0x80) ? Ppu::NMT_1 : Ppu::NMT_0 );
					chr.SwapBank<SIZE_8K,0x0000>( data );
					prg.SwapBank<SIZE_16K,0x0000>( data >> 4 );
				}

				NES_POKE_D(Ic74x139x74,6000)
				{
					ppu.Update();
					chr.SwapBank<SIZE_8K,0x0000>( (data >> 1 & 0x1) | (data << 1 & 0x2) );
				}

				NES_POKE_D(Ic74x161x138,6000)
				{
					ppu.Update();
					chr.SwapBank<SIZE_8K,0x0000>( data >> 2 );
					prg.SwapBank<SIZE_32K,0x0000>( data );
				}

				NES_POKE_AD(Ic74x377,8000)
				{
					ppu.Update();
					data = GetBusData(address,data);
					prg.SwapBank<SIZE_32K,0x0000>( data );
					chr.SwapBank<SIZE_8K,0x0000>( data >> 4 );
				}
			}
		}
	}
}
