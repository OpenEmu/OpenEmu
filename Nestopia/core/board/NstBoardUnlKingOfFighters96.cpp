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
#include "NstBoardUnlKingOfFighters96.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Unlicensed
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void KingOfFighters96::SubReset(const bool hard)
				{
					for (uint i=0; i < 4; ++i)
						exRegs[i] = 0;

					Mmc3::SubReset( hard );

					Map( 0x5000U,          &KingOfFighters96::Peek_5000, &KingOfFighters96::Poke_5000 );
					Map( 0x5001U, 0x5FFFU, &KingOfFighters96::Peek_5000, &KingOfFighters96::Poke_5001 );

					for (uint i=0x8000; i < 0xA000; i += 0x4)
					{
						Map( i + 0x0, &KingOfFighters96::Poke_8000 );
						Map( i + 0x1, &KingOfFighters96::Poke_8001 );
						Map( i + 0x2, NOP_POKE );
						Map( i + 0x3, &KingOfFighters96::Poke_8003 );
					}
				}

				void KingOfFighters96::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'U','K','6'>::V)
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

				void KingOfFighters96::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'U','K','6'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( exRegs ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void NST_FASTCALL KingOfFighters96::UpdatePrg(uint address,uint bank)
				{
					if (!(exRegs[0] & 0x80U))
						prg.SwapBank<SIZE_8K>( address, bank );
				}

				void NST_FASTCALL KingOfFighters96::UpdateChr(uint address,uint bank) const
				{
					if ((address & 0x1000) == (regs.ctrl0 << 5 & 0x1000))
						bank |= 0x100;

					chr.SwapBank<SIZE_1K>( address, bank );
				}

				NES_PEEK(KingOfFighters96,5000)
				{
					static const byte security[4] =
					{
						0x83,0x83,0x42,0x00
					};

					return security[exRegs[1] & 0x3U];
				}

				NES_POKE_D(KingOfFighters96,5000)
				{
					exRegs[1] = data;

					if (exRegs[0] != data)
					{
						exRegs[0] = data;

						if (exRegs[0] & 0x80U)
						{
							const uint bank = exRegs[0] & 0x1FU;

							if (exRegs[0] & 0x20U)
								prg.SwapBank<SIZE_32K,0x0000>( bank >> 2 );
							else
								prg.SwapBanks<SIZE_16K,0x0000>( bank, bank );
						}
						else
						{
							Mmc3::UpdatePrg();
						}
					}
				}

				NES_POKE_D(KingOfFighters96,5001)
				{
					exRegs[1] = data;

					if (!exRegs[3])
					{
						exRegs[3] = true;
						cpu.Poke( 0x4017, 0x40 );
					}
				}

				NES_POKE_AD(KingOfFighters96,8000)
				{
					exRegs[2] = true;
					Mmc3::NES_DO_POKE(8000,address,data);
				}

				NES_POKE_AD(KingOfFighters96,8001)
				{
					if (exRegs[2])
						Mmc3::NES_DO_POKE(8001,address,data);
				}

				NES_POKE_D(KingOfFighters96,8003)
				{
					exRegs[2] = false;

					if (data == 0x28)
					{
						prg.SwapBank<SIZE_8K,0x4000>( 0x17 );
					}
					else if (data == 0x2A)
					{
						prg.SwapBank<SIZE_8K,0x2000>( 0x0F );
					}
				}
			}
		}
	}
}
