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
#include "NstBoardBmcSuperHiK4in1.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bmc
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void SuperHiK4in1::SubReset(const bool hard)
				{
					if (hard)
						exReg = 0x00;

					Mmc3::SubReset( hard );

					Map( 0x6000U, 0x7FFFU, &SuperHiK4in1::Poke_6000 );

					prg.SwapBank<SIZE_32K,0x0000>( 0 );
				}

				void SuperHiK4in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','H','4'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								exReg = state.Read8();

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void SuperHiK4in1::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'B','H','4'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( exReg ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(SuperHiK4in1,6000)
				{
					if (regs.ctrl1 & 0x80)
					{
						if (exReg != data)
						{
							exReg = data;

							if (exReg & 0x1)
								Mmc3::UpdatePrg();
							else
								prg.SwapBank<SIZE_32K,0x0000>( exReg >> 4 & 0x3 );

							Mmc3::UpdateChr();
						}
					}
				}

				void NST_FASTCALL SuperHiK4in1::UpdatePrg(uint address,uint bank)
				{
					if (exReg & 0x1)
						prg.SwapBank<SIZE_8K>( address, (exReg >> 2 & 0x30) | (bank & 0x0F) );
				}

				void NST_FASTCALL SuperHiK4in1::UpdateChr(uint address,uint bank) const
				{
					chr.SwapBank<SIZE_1K>( address, (exReg << 1 & 0x180) | (bank & 0x7F) );
				}
			}
		}
	}
}
