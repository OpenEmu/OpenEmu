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
#include "NstBoardBmc22Games.hpp"

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

				void B22Games::SubReset(const bool hard)
				{
					if (hard)
						romSwitch = 0;
					else
						romSwitch ^= 1;

					if (romSwitch)
					{
						prg.SwapBanks<SIZE_16K,0x0000>( 0, 7 );
						ppu.SetMirroring( Ppu::NMT_V );
					}
					else
					{
						prg.SwapBanks<SIZE_16K,0x0000>( 8, 39 );
					}

					Map( 0x8000U, 0xFFFFU, &B22Games::Poke_8000 );
				}

				void B22Games::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','2','G'>::V) );

					if (baseChunk == AsciiId<'B','2','G'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								romSwitch = state.Read8() & 0x1;

							state.End();
						}
					}
				}

				void B22Games::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','2','G'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( romSwitch ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(B22Games,8000)
				{
					if (romSwitch)
					{
						prg.SwapBank<SIZE_16K,0x0000>( data & 0x7 );
					}
					else
					{
						prg.SwapBanks<SIZE_16K,0x0000>( 8 + (data & 0x1F), (8 + (data & 0x1F)) | (~data >> 5 & 0x1) );
						ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_V : Ppu::NMT_H );
					}
				}
			}
		}
	}
}
