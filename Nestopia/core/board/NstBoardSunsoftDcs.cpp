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
#include "NstBoardSunsoftDcs.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sunsoft
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Dcs::SubReset(const bool hard)
				{
					counter = SIGNAL;
					prgBank = 0;

					S4::SubReset( hard );

					prg.SwapBanks<SIZE_16K,0x0000>( 0x0, 0x7 );

					Map( 0x6000U,          &Dcs::Poke_6000 );
					Map( 0x8000U, 0xBFFFU, &Dcs::Peek_8000 );
					Map( 0xF000U, 0xFFFFU, &Dcs::Poke_F000 );
				}

				void Dcs::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'S','D','C'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'D','B','C'>::V)
							{
								prgBank = state.Read8() & 0xF;
								counter = state.Read16();

								if (counter > SIGNAL)
									counter = SIGNAL;
							}

							state.End();
						}
					}
					else
					{
						S4::SubLoad( state, baseChunk );
					}
				}

				void Dcs::SubSave(State::Saver& state) const
				{
					S4::SubSave( state );
					state.Begin( AsciiId<'S','D','C'>::V ).Begin( AsciiId<'D','B','C'>::V ).Write8( prgBank ).Write16( counter ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Dcs,6000)
				{
					if (data == 0x00)
					{
						counter = 0;
						prg.SwapBank<SIZE_16K,0x0000>( prgBank );
					}
				}

				NES_PEEK_A(Dcs,8000)
				{
					if (const uint bank = ((prgBank & 0x8 && counter < SIGNAL && ++counter == SIGNAL) ? (prgBank & 0x7 | 0x10) : 0))
						prg.SwapBank<SIZE_16K,0x0000>( bank & 0xF );

					return prg.Peek( address - 0x8000 );
				}

				NES_POKE_D(Dcs,F000)
				{
					prgBank = (data & 0x7) | (~data & 0x8);
					prg.SwapBank<SIZE_16K,0x0000>( prgBank );
				}
			}
		}
	}
}
