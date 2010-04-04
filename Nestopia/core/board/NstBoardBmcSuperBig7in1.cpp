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
#include "NstBoardBmcSuperBig7in1.hpp"

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

				void SuperBig7in1::SubReset(const bool hard)
				{
					if (hard)
						exReg = 0;

					Mmc3::SubReset( hard );

					for (uint i=0xA001; i < 0xC000; i += 0x2)
						Map( i, &SuperBig7in1::Poke_A001 );
				}

				void SuperBig7in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','S','B'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								exReg = state.Read8();
								exReg = NST_MIN(exReg & 7,6);
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void SuperBig7in1::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'B','S','B'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( exReg ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(SuperBig7in1,A001)
				{
					data &= 0x7;

					if (exReg != data)
					{
						exReg = data;
						Mmc3::UpdatePrg();
						Mmc3::UpdateChr();
					}
				}

				void NST_FASTCALL SuperBig7in1::UpdatePrg(uint address,uint bank)
				{
					prg.SwapBank<SIZE_8K>( address, (exReg << 4) | (bank & (exReg >= 6 ? 0x1F : 0x0F)) );
				}

				void NST_FASTCALL SuperBig7in1::UpdateChr(uint address,uint bank) const
				{
					chr.SwapBank<SIZE_1K>( address, (exReg << 7) | (bank & (exReg >= 6 ? 0xFF : 0x7F)) );
				}
			}
		}
	}
}
