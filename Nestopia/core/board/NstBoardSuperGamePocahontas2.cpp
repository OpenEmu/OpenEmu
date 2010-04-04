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
#include "NstBoardSuperGamePocahontas2.hpp"

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

				void Pocahontas2::SubReset(const bool hard)
				{
					if (hard)
					{
						exRegs[0] = 0x00;
						exRegs[1] = 0x00;
					}

					exRegs[2] = false;

					Mmc3::SubReset( hard );

					Map( 0x5000U,          &Pocahontas2::Poke_5000 );
					Map( 0x5001U,          &Pocahontas2::Poke_5001 );
					Map( 0x8000U, 0x9FFFU, &Pocahontas2::Poke_8000 );
					Map( 0xA000U, 0xBFFFU, &Pocahontas2::Poke_A000 );
					Map( 0xC000U, 0xDFFFU, &Pocahontas2::Poke_C000 );
					Map( 0xE000U, 0xEFFFU, &Pocahontas2::Poke_E000 );
					Map( 0xF000U, 0xFFFFU, &Pocahontas2::Poke_F000 );
				}

				void Pocahontas2::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'S','P','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<3> data( state );

								exRegs[0] = data[0];
								exRegs[1] = data[1];
								exRegs[2] = data[2] & 0x1;
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Pocahontas2::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );

					const byte data[3] =
					{
						exRegs[0],
						exRegs[1],
						exRegs[2]
					};

					state.Begin( AsciiId<'S','P','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void NST_FASTCALL Pocahontas2::UpdatePrg(uint address,uint bank)
				{
					if (!(exRegs[0] & 0x80))
						prg.SwapBank<SIZE_8K>( address, bank );
				}

				void NST_FASTCALL Pocahontas2::UpdateChr(uint address,uint bank) const
				{
					chr.SwapBank<SIZE_1K>( address, (exRegs[1] << 6 & 0x100) | bank );
				}

				NES_POKE_D(Pocahontas2,5000)
				{
					if (exRegs[0] != data)
					{
						exRegs[0] = data;

						if (exRegs[0] & 0x80)
						{
							const uint bank = exRegs[0] & 0xF;

							if (exRegs[0] & 0x20)
								prg.SwapBank<SIZE_32K,0x0000>( bank >> 1 );
							else
								prg.SwapBanks<SIZE_16K,0x0000>( bank, bank );
						}
						else
						{
							Mmc3::UpdatePrg();
						}
					}
				}

				NES_POKE_D(Pocahontas2,5001)
				{
					if (exRegs[1] != data)
					{
						exRegs[1] = data;
						Mmc3::UpdateChr();
					}
				}

				NES_POKE_D(Pocahontas2,8000)
				{
					SetMirroringHV( data >> 7 | data );
				}

				NES_POKE_D(Pocahontas2,A000)
				{
					static const byte lut[8] = {0,2,6,1,7,3,4,5};

					data = (data & 0xC0) | lut[data & 0x07];
					exRegs[2] = true;

					Mmc3::NES_DO_POKE(8000,0x8000,data);
				}

				NES_POKE_D(Pocahontas2,C000)
				{
					if (exRegs[2])
					{
						exRegs[2] = false;
						Mmc3::NES_DO_POKE(8001,0x8001,data);
					}
				}

				NES_POKE_D(Pocahontas2,F000)
				{
					Mmc3::NES_DO_POKE(E001,0xE001,data);
					Mmc3::NES_DO_POKE(C000,0xC000,data);
					Mmc3::NES_DO_POKE(C001,0xC001,data);
				}
			}
		}
	}
}
