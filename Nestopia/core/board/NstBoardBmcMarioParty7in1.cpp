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
#include "NstBoardBmcMarioParty7in1.hpp"

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

				void MarioParty7in1::SubReset(const bool hard)
				{
					if (hard)
					{
						exRegs[0] = 0;
						exRegs[1] = 0;
					}

					Mmc3::SubReset( hard );

					Map( 0x6000U, 0x7FFFU, &MarioParty7in1::Poke_6000 );
				}

				void MarioParty7in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','M','P'>::V)
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

				void MarioParty7in1::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );

					const byte data[2] =
					{
						exRegs[0],
						exRegs[1]
					};

					state.Begin( AsciiId<'B','M','P'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(MarioParty7in1,6000)
				{
					if (exRegs[1])
					{
						NST_VERIFY( wrk.Writable(0) );

						if (wrk.Writable(0))
							wrk[0][address - 0x6000] = data;
					}
					else
					{
						exRegs[1] = 1;
						exRegs[0] = data;

						Mmc3::UpdatePrg();
						Mmc3::UpdateChr();
					}
				}

				void NST_FASTCALL MarioParty7in1::UpdatePrg(uint address,uint bank)
				{
					prg.SwapBank<SIZE_8K>
					(
						address,
						(((exRegs[0] & 0x6) | (exRegs[0] >> 3 & exRegs[0] & 0x1)) << 4) |
						(bank & ((exRegs[0] << 1 & 0x10) ^ 0x1F))
					);
				}

				void NST_FASTCALL MarioParty7in1::UpdateChr(uint address,uint bank) const
				{
					chr.SwapBank<SIZE_1K>
					(
						address,
						(((exRegs[0] >> 3 & 0x4) | (exRegs[0] >> 1 & 0x2) | ((exRegs[0] >> 6) & (exRegs[0] >> 4) & 0x1)) << 7) |
						(bank & (((exRegs[0] & 0x40) << 1) ^ 0xFF))
					);
				}
			}
		}
	}
}
