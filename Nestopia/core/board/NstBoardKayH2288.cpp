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
#include "NstBoardKay.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Kay
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void H2288::SubReset(const bool hard)
				{
					if (hard)
					{
						exRegs[0] = 0;
						exRegs[1] = 0;
					}

					Mmc3::SubReset( hard );

					Map( 0x5000U, 0x57FFU, &H2288::Peek_5000 );
					Map( 0x5800U, 0x5FFFU, &H2288::Peek_5000, &H2288::Poke_5800 );

					for (uint i=0x8000; i < 0x9000; i += 0x2)
						Map( i, &H2288::Poke_8000 );
				}

				void H2288::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'K','H','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<2> data( state );

								exRegs[0] = data[0];
								exRegs[1] = data[1];
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void H2288::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );

					const byte data[2] =
					{
						exRegs[0],
						exRegs[1]
					};

					state.Begin( AsciiId<'K','H','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void NST_FASTCALL H2288::UpdatePrg(uint address,uint bank)
				{
					if (!(exRegs[0] & 0x40))
						Mmc3::UpdatePrg( address, bank );
				}

				NES_PEEK_A(H2288,5000)
				{
					return (address >> 8 & 0xFE) | (((~address & 0x1) & (address >> 8 & 0x1)) ^ 0x1);
				}

				NES_POKE_AD(H2288,5800)
				{
					address &= 0x1;

					if (exRegs[address] != data)
					{
						exRegs[address] = data;

						if (exRegs[0] & 0x40)
						{
							data = (exRegs[0] & 0x5) | (exRegs[0] >> 2 & 0x2) | (exRegs[0] >> 2 & 0x8);
							address = exRegs[0] >> 1 & 0x1;

							prg.SwapBanks<SIZE_16K,0x0000>( data & ~address, data | address );
						}
						else
						{
							Mmc3::UpdatePrg();
						}
					}
				}

				NES_POKE_AD(H2288,8000)
				{
					static const byte security[8] = {0,3,1,5,6,7,2,4};
					Mmc3::NES_DO_POKE(8000,address,(data & 0xC0) | security[data & 0x07]);
				}
			}
		}
	}
}
