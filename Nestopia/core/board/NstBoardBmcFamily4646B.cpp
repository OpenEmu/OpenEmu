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
#include "NstBoardBmcFamily4646B.hpp"

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

				void Family4646B::SubReset(const bool hard)
				{
					if (hard)
						exReg = 0;

					Mmc3::SubReset( hard );

					Map( 0x6001U, 0x6001U, &Family4646B::Poke_6001 );
				}

				void Family4646B::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','F','K'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								exReg = state.Read8();

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Family4646B::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'B','F','K'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( exReg ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Family4646B,6001)
				{
					if (exReg != data)
					{
						exReg = data;
						Mmc3::UpdatePrg();
						Mmc3::UpdateChr();
					}
				}

				void NST_FASTCALL Family4646B::UpdatePrg(uint address,uint bank)
				{
					prg.SwapBank<SIZE_8K>( address, (exReg << 4 & 0x20) | (bank & 0x1F) );
				}

				void NST_FASTCALL Family4646B::UpdateChr(uint address,uint bank) const
				{
					chr.SwapBank<SIZE_1K>( address, (exReg << 3 & 0x100) | bank );
				}
			}
		}
	}
}
