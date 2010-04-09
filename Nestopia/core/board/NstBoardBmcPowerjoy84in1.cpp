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
#include "NstBoardBmcPowerjoy84in1.hpp"

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

				void Powerjoy84in1::SubReset(const bool hard)
				{
					if (hard)
					{
						for (uint i=0; i < 4; ++i)
							exRegs[i] = 0;
					}

					Mmc3::SubReset( hard );

					for (uint i=0x6000; i < 0x8000; i += 0x4)
					{
						Map( i + 0x0, &Powerjoy84in1::Poke_6000 );
						Map( i + 0x1, &Powerjoy84in1::Poke_6001 );
						Map( i + 0x2, &Powerjoy84in1::Poke_6001 );
						Map( i + 0x3, &Powerjoy84in1::Poke_6000 );
					}
				}

				void Powerjoy84in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','P','J'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								state.Read( exRegs );

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Powerjoy84in1::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'B','P','J'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( exRegs ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				uint Powerjoy84in1::GetExChrExBank() const
				{
					return
					(
						(~uint(exRegs[0]) << 0 & 0x080 & uint(exRegs[2])) |
						( uint(exRegs[0]) << 4 & 0x080 & uint(exRegs[0])) |
						( uint(exRegs[0]) << 3 & 0x100) |
						( uint(exRegs[0]) << 5 & 0x200)
					);
				}

				void NST_FASTCALL Powerjoy84in1::UpdatePrg(uint address,uint bank)
				{
					bank &= ~uint(exRegs[0]) >> 2 & 0x10 | 0x0F;
					bank |= (exRegs[0] & (0x6U | (exRegs[0] & 0x40U) >> 6)) << 4 | (exRegs[0] & 0x10U) << 3;

					if (!(exRegs[3] & 0x3U))
					{
						prg.SwapBank<SIZE_8K>( address, bank );
					}
					else if (address == (regs.ctrl0 << 8 & 0x4000))
					{
						if ((exRegs[3] & 0x3U) == 0x3)
							prg.SwapBank<SIZE_32K,0x0000>( bank >> 2 );
						else
							prg.SwapBanks<SIZE_16K,0x0000>( bank >> 1, bank >> 1 );
					}
				}

				void NST_FASTCALL Powerjoy84in1::UpdateChr(uint address,uint bank) const
				{
					if (!(exRegs[3] & 0x10U))
						chr.SwapBank<SIZE_1K>( address, GetExChrExBank() | (bank & ((exRegs[0] & 0x80U) - 1)) );
				}

				NES_POKE_AD(Powerjoy84in1,6000)
				{
					if (!(exRegs[3] & 0x80U))
						NES_DO_POKE(6001,address,data);
				}

				NES_POKE_AD(Powerjoy84in1,6001)
				{
					address &= 0x3;

					if (exRegs[address] != data)
					{
						exRegs[address] = data;

						if (exRegs[3] & 0x10U)
							chr.SwapBank<SIZE_8K,0x0000>( GetExChrExBank() >> 3 | (exRegs[2] & 0xFU) );
						else
							Mmc3::UpdateChr();

						Mmc3::UpdatePrg();
					}
				}
			}
		}
	}
}
