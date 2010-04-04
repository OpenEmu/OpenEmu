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
#include "NstBoardRexSoftSl1632.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace RexSoft
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Sl1632::SubReset(const bool hard)
				{
					exMode = 0;

					if (hard)
					{
						for (uint i=0; i < 2; ++i)
							exPrg[i] = 0;

						for (uint i=0; i < 8; ++i)
							exChr[i] = 0;

						exNmt = 0;
					}

					Mmc3::SubReset( hard );

					Map( 0x8000U, 0xFFFFU, &Sl1632::Poke_8000 );
				}

				void Sl1632::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'R','1','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<12> data( state );

								exMode = data[0];

								for (uint i=0; i < 2; ++i)
									exPrg[i] = data[1+i];

								for (uint i=0; i < 8; ++i)
									exChr[i] = data[1+2+i];

								exNmt = data[11];
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Sl1632::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );

					const byte data[12] =
					{
						exMode,
						exPrg[0],
						exPrg[1],
						exChr[0],
						exChr[1],
						exChr[2],
						exChr[3],
						exChr[4],
						exChr[5],
						exChr[6],
						exChr[7],
						exNmt
					};

					state.Begin( AsciiId<'R','1','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void NST_FASTCALL Sl1632::UpdatePrg(uint address,uint bank)
				{
					if (exMode & 0x2U)
						Mmc3::UpdatePrg( address, bank );
					else
						prg.SwapBanks<SIZE_8K,0x0000>( exPrg[0], exPrg[1], ~1U, ~0U );
				}

				void NST_FASTCALL Sl1632::UpdateChr(uint address,uint bank) const
				{
					if (exMode & 0x2U)
					{
						static const byte modes[4] = {5,5,3,1};
						bank |= uint(exMode) << modes[address >> 11 ^ (regs.ctrl0 >> 6 & 0x2)] & 0x100;
					}
					else
					{
						bank = exChr[address >> 10];
					}

					chr.SwapBank<SIZE_1K>( address, bank );
				}

				NES_POKE_AD(Sl1632,8000)
				{
					if ((address & 0xA131) == 0xA131 && exMode != data)
					{
						exMode = data;

						Mmc3::UpdatePrg();
						Mmc3::UpdateChr();

						if (!(exMode & 0x2U))
							SetMirroringHV( exNmt );
					}

					if (exMode & 0x2U)
					{
						switch (address & 0xE001)
						{
							case 0x8000: Mmc3::NES_DO_POKE( 8000, address, data ); break;
							case 0x8001: Mmc3::NES_DO_POKE( 8001, address, data ); break;
							case 0xA000: SetMirroringVH( exNmt );                  break;
							case 0xA001: Mmc3::NES_DO_POKE( A001, address, data ); break;
							case 0xC000: Mmc3::NES_DO_POKE( C000, address, data ); break;
							case 0xC001: Mmc3::NES_DO_POKE( C001, address, data ); break;
							case 0xE000: Mmc3::NES_DO_POKE( E000, address, data ); break;
							case 0xE001: Mmc3::NES_DO_POKE( E001, address, data ); break;
						}
					}
					else if (address >= 0xB000 && address <= 0xE003)
					{
						const uint offset = address << 2 & 0x4;
						address = ((((address & 0x2) | address >> 10) >> 1) + 2) & 0x7;
						exChr[address] = (exChr[address] & 0xF0U >> offset) | ((data & 0x0F) << offset);

						Mmc3::UpdateChr();
					}
					else switch (address & 0xF003)
					{
						case 0x8000:

							if (exPrg[0] != data)
							{
								exPrg[0] = data;
								Mmc3::UpdatePrg();
							}
							break;

						case 0x9000:

							if (exNmt != data)
							{
								exNmt = data;
								SetMirroringHV( exNmt );
							}
							break;

						case 0xA000:

							if (exPrg[1] != data)
							{
								exPrg[1] = data;
								Mmc3::UpdatePrg();
							}
							break;
					}
				}
			}
		}
	}
}
