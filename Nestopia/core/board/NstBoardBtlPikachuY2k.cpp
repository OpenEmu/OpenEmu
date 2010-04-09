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
#include "NstBoardBtlPikachuY2k.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Btl
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void PikachuY2k::SubReset(const bool hard)
				{
					security = ~0U;

					Mmc3::SubReset( hard );

					Map( 0x6000U, 0x7FFFU, &PikachuY2k::Peek_6000, &PikachuY2k::Poke_6000 );

					for (uint i=0x8000; i < 0xA000; i += 0x2)
						Map( i, &PikachuY2k::Poke_8000 );
				}

				void PikachuY2k::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'B','P','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								security = (state.Read8() & 0x1) ? ~0U : 0U;

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void PikachuY2k::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'B','P','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( security & 0x1 ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(PikachuY2k,6000)
				{
					return wrk[0][address - 0x6000] ^ (regs.ctrl1 & security);
				}

				NES_POKE_AD(PikachuY2k,6000)
				{
					wrk[0][address - 0x6000] = data;
				}

				NES_POKE_AD(PikachuY2k,8000)
				{
					security = 0U;
					Mmc3::NES_DO_POKE(8000,address,data);
				}
			}
		}
	}
}
