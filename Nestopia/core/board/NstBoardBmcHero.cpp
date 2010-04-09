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
#include "NstBoardBmcHero.hpp"

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

				void Hero::SubReset(const bool hard)
				{
					if (hard)
					{
						for (uint i=0; i < 5; ++i)
							exRegs[i] = 0;
					}

					Mmc3::SubReset( hard );

					Map( 0x6000U, 0x7FFFU, &Hero::Poke_6000 );
				}

				void Hero::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','H','R'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<5> data( state );

								for (uint i=0; i < 5; ++i)
									exRegs[i] = data[i];
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Hero::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );

					const byte data[5] =
					{
						exRegs[0],
						exRegs[1],
						exRegs[2],
						exRegs[3],
						exRegs[4]
					};

					state.Begin( AsciiId<'B','H','R'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(Hero,6000)
				{
					if (exRegs[3] & 0x40)
					{
						NST_VERIFY( wrk.Writable(0) );

						if (wrk.Writable(0))
							wrk[0][address - 0x6000] = data;
					}
					else
					{
						exRegs[exRegs[4]] = data;
						exRegs[4] = (exRegs[4] + 1) & 0x3;

						Mmc3::UpdatePrg();
						Mmc3::UpdateChr();
					}
				}

				void NST_FASTCALL Hero::UpdatePrg(uint address,uint bank)
				{
					prg.SwapBank<SIZE_8K>( address, exRegs[1] | (bank & (exRegs[3] ^ 0x3F)) );
				}

				void NST_FASTCALL Hero::UpdateChr(uint address,uint bank) const
				{
					if (chr.Source().GetType() == Ram::ROM)
					{
						chr.SwapBank<SIZE_1K>
						(
							address,
							(exRegs[0] | (exRegs[2] << 4 & 0xF00)) |
							((exRegs[2] & 0x8) ? bank & ((1U << ((exRegs[2] & 0x7) + 1)) - 1) : exRegs[2] ? 0 : bank)
						);
					}
				}
			}
		}
	}
}
