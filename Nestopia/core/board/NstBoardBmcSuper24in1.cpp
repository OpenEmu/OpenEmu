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
#include "NstBoardBmcSuper24in1.hpp"

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

				void Super24in1::SubReset(const bool hard)
				{
					if (hard)
					{
						exRegs[0] = 0x24;
						exRegs[1] = 0x9F;
						exRegs[2] = 0x00;
					}

					Mmc3::SubReset( hard );

					Map( 0x5FF0U, &Super24in1::Poke_5FF0 );
					Map( 0x5FF1U, &Super24in1::Poke_5FF1 );
					Map( 0x5FF2U, &Super24in1::Poke_5FF2 );
				}

				void Super24in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','2','4'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<3> data( state );

								exRegs[0] = data[0];
								exRegs[1] = data[1];
								exRegs[2] = data[2];
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Super24in1::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );

					const byte data[3] =
					{
						exRegs[0],
						exRegs[1],
						exRegs[2]
					};

					state.Begin( AsciiId<'B','2','4'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Super24in1,5FF0)
				{
					if (exRegs[0] != data)
					{
						exRegs[0] = data;
						Mmc3::UpdateChr();
						Mmc3::UpdatePrg();
					}
				}

				NES_POKE_D(Super24in1,5FF1)
				{
					if (exRegs[1] != data)
					{
						exRegs[1] = data;
						Mmc3::UpdatePrg();
					}
				}

				NES_POKE_D(Super24in1,5FF2)
				{
					if (exRegs[2] != data)
					{
						exRegs[2] = data;
						Mmc3::UpdateChr();
					}
				}

				void NST_FASTCALL Super24in1::UpdatePrg(uint address,uint bank)
				{
					static const byte masks[8] = {0x3F,0x1F,0xF,0x1,0x3,0x0,0x0,0x0};

					prg.SwapBank<SIZE_8K>( address, (exRegs[1] << 1) | (bank & masks[exRegs[0] & 0x7]) );
				}

				void NST_FASTCALL Super24in1::UpdateChr(uint address,uint bank) const
				{
					chr.Source( exRegs[0] >> 5 & 0x1 ).SwapBank<SIZE_1K>( address, (exRegs[2] << 3 & 0xF00) | bank );
				}
			}
		}
	}
}
