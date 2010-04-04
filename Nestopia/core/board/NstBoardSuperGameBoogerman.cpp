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
#include "NstBoardSuperGameBoogerman.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace SuperGame
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Boogerman::SubReset(const bool hard)
				{
					exRegs[0] = 0x00;
					exRegs[1] = 0xFF;
					exRegs[2] = 0x04;
					exRegs[3] = false;

					Mmc3::SubReset( hard );

					Map( 0x5000U, &Boogerman::Poke_5000 );
					Map( 0x5001U, &Boogerman::Poke_5001 );
					Map( 0x5007U, &Boogerman::Poke_5007 );

					if (board != Type::SUPERGAME_MK3E)
					{
						Map( 0x6000U, &Boogerman::Poke_5000 );
						Map( 0x6001U, &Boogerman::Poke_5001 );
						Map( 0x6007U, &Boogerman::Poke_5007 );
					}

					for (uint i=0x0000; i < 0x2000; i += 0x2)
					{
						Map( 0x8000U + i, &Boogerman::Poke_8000 );
						Map( 0x8001U + i, &Boogerman::Poke_8001 );
						Map( 0xA000U + i, &Boogerman::Poke_A000 );
						Map( 0xC000U + i, &Boogerman::Poke_C000 );
						Map( 0xC001U + i, &Boogerman::Poke_C001 );
						Map( 0xE001U + i, &Boogerman::Poke_E001 );
					}
				}

				void Boogerman::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'S','B','G'>::V)
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

				void Boogerman::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'S','B','G'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( exRegs ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void NST_FASTCALL Boogerman::UpdatePrg(uint address,uint bank)
				{
					if (!(exRegs[0] & 0x80U))
					{
						if (exRegs[1] & 0x8U)
							bank = (bank & 0x1F) | 0x20;
						else
							bank = (bank & 0x0F) | (exRegs[1] & 0x10U);

						prg.SwapBank<SIZE_8K>( address, bank );
					}
				}

				void NST_FASTCALL Boogerman::UpdateChr(uint address,uint bank) const
				{
					if (exRegs[1] & 0x4U)
						bank = (bank | 0x100);
					else
						bank = (bank & 0x7F) | (exRegs[1] << 3 & 0x80U);

					chr.SwapBank<SIZE_1K>( address, bank );
				}

				void Boogerman::UpdatePrg()
				{
					if (exRegs[0] & 0x80U)
					{
						uint bank = (exRegs[0] & 0x0FU) | (exRegs[1] & 0x10U);
						prg.SwapBanks<SIZE_16K,0x0000>( bank, bank );
					}
					else
					{
						Mmc3::UpdatePrg();
					}
				}

				NES_POKE_D(Boogerman,5000)
				{
					if (exRegs[0] != data)
					{
						exRegs[0] = data;
						UpdatePrg();
					}
				}

				NES_POKE_D(Boogerman,5001)
				{
					if (exRegs[1] != data)
					{
						exRegs[1] = data;
						Mmc3::UpdateChr();
					}
				}

				NES_POKE_D(Boogerman,5007)
				{
					regs.ctrl0 = 0;

					if (exRegs[2] != data)
					{
						exRegs[2] = data;
						UpdatePrg();
						Mmc3::UpdateChr();
					}
				}

				NES_POKE_D(Boogerman,8000)
				{
					if (!exRegs[2])
						Mmc3::NES_DO_POKE(8000,0x8000,data);
				}

				NES_POKE_D(Boogerman,8001)
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
						Mmc3::NES_DO_POKE(8001,0x8001,data);
					}
				}

				NES_POKE_D(Boogerman,A000)
				{
					if (exRegs[2])
					{
						static const byte security[8] = {0,2,5,3,6,1,7,4};

						data = (data & 0xC0) | security[data & 0x07];
						exRegs[3] = true;

						Mmc3::NES_DO_POKE(8000,0x8000,data);
					}
					else
					{
						SetMirroringHV( data );
					}
				}

				NES_POKE_D(Boogerman,C000)
				{
					if (exRegs[2])
						SetMirroringHV( data >> 7 | data );
					else
						Mmc3::NES_DO_POKE(C000,0xC000,data);
				}

				NES_POKE_D(Boogerman,C001)
				{
					if (exRegs[2])
						Mmc3::NES_DO_POKE(E001,0xE001,data);
					else
						Mmc3::NES_DO_POKE(C001,0xC001,data);
				}

				NES_POKE_D(Boogerman,E001)
				{
					if (exRegs[2])
					{
						Mmc3::NES_DO_POKE(C000,0xC000,data);
						Mmc3::NES_DO_POKE(C001,0xC001,data);
					}
					else
					{
						Mmc3::NES_DO_POKE(E001,0xE001,data);
					}
				}
			}
		}
	}
}
