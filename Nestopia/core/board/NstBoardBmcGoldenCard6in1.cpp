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
#include "NstBoardBmcGoldenCard6in1.hpp"

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

				void GoldenCard6in1::SubReset(const bool hard)
				{
					if (hard)
					{
						exRegs[0] = 0x00;
						exRegs[1] = 0xFF;
						exRegs[2] = 0x03;
					}

					exRegs[3] = false;

					Mmc3::SubReset( hard );

					Map( 0x5000U, &GoldenCard6in1::Poke_5000 );
					Map( 0x5001U, &GoldenCard6in1::Poke_5001 );
					Map( 0x5007U, &GoldenCard6in1::Poke_5007 );

					for (uint i=0x0000; i < 0x2000; i += 0x2)
					{
						Map( 0x8000 + i, &GoldenCard6in1::Poke_8000 );
						Map( 0x8001 + i, &GoldenCard6in1::Poke_8001 );
						Map( 0xA000 + i, &GoldenCard6in1::Poke_A000 );
						Map( 0xA001 + i, &GoldenCard6in1::Poke_A001 );
					}
				}

				void GoldenCard6in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','G','C'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								state.Read( exRegs );
								exRegs[3] &= 0x1U;
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void GoldenCard6in1::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'B','G','C'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( exRegs ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void NST_FASTCALL GoldenCard6in1::UpdatePrg(uint address,uint bank)
				{
					if (exRegs[1] & 0x8U)
						bank = (bank & 0x1F);
					else
						bank = (bank & 0x0F) | (exRegs[1] & 0x10U);

					prg.SwapBank<SIZE_8K>( address, (exRegs[1] << 5 & 0x60U) | bank );
				}

				void NST_FASTCALL GoldenCard6in1::UpdateChr(uint address,uint bank) const
				{
					if (!(exRegs[1] & 0x8U))
						bank = (exRegs[1] << 3 & 0x80U) | (bank & 0x7F);

					chr.SwapBank<SIZE_1K>( address, (exRegs[1] << 8 & 0x300U) | bank );
				}

				NES_POKE_D(GoldenCard6in1,5000)
				{
					exRegs[0] = data;

					if (data & 0x80)
					{
						data = (data & 0x0F) | (exRegs[1] << 4 & 0x30U);
						prg.SwapBanks<SIZE_16K,0x0000>( data, data );
					}
					else
					{
						Mmc3::UpdatePrg();
					}
				}

				NES_POKE_D(GoldenCard6in1,5001)
				{
					if (exRegs[1] != data)
					{
						exRegs[1] = data;
						Mmc3::UpdatePrg();
					}
				}

				NES_POKE_D(GoldenCard6in1,5007)
				{
					exRegs[2] = data;
				}

				NES_POKE_D(GoldenCard6in1,8000)
				{
					if (exRegs[2])
						Mmc3::NES_DO_POKE(C000,0xC000,data);
					else
						Mmc3::NES_DO_POKE(8000,0x8000,data);
				}

				NES_POKE_D(GoldenCard6in1,8001)
				{
					if (exRegs[2])
					{
						static const byte lut[8] = {0,6,3,7,5,2,4,1};

						data = (data & 0xC0) | lut[data & 0x07];
						exRegs[3] = true;

						Mmc3::NES_DO_POKE(8000,0x8000,data);
					}
					else
					{
						Mmc3::NES_DO_POKE(8001,0x8001,data);
					}
				}

				NES_POKE_D(GoldenCard6in1,A000)
				{
					if (exRegs[2])
					{
						if (exRegs[3] && ((exRegs[0] & 0x80U) == 0 || (regs.ctrl0 & 0x7) < 6))
						{
							exRegs[3] = false;
							Mmc3::NES_DO_POKE(8001,0x8001,data);
						}
					}
					else
					{
						SetMirroringHV( data );
					}
				}

				NES_POKE_D(GoldenCard6in1,A001)
				{
					if (exRegs[2])
						SetMirroringHV( data );
					else
						Mmc3::NES_DO_POKE(A001,0xA001,data);
				}
			}
		}
	}
}
