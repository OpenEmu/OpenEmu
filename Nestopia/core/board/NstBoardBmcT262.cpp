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
#include "NstBoardBmcT262.hpp"

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

				void T262::SubReset(bool)
				{
					Map( 0x8000U, 0xFFFFU, &T262::Poke_8000 );

					mode = false;
					NES_DO_POKE(8000,0x8001,0x00);
					mode = false;
				}

				void T262::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','T','6'>::V) );

					if (baseChunk == AsciiId<'B','T','6'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								mode = state.Read8() & 0x1;

							state.End();
						}
					}
				}

				void T262::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','T','6'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( mode ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(T262,8000)
				{
					if (mode || address == 0x8000)
					{
						prg.SwapBank<SIZE_16K,0x0000>( (prg.GetBank<SIZE_16K,0x0000>() & 0x38) | (data & 0x07) );
					}
					else
					{
						mode = true;
						data = (address >> 3 & 0x20) | (address >> 2 & 0x18);
						prg.SwapBanks<SIZE_16K,0x0000>( data | (prg.GetBank<SIZE_16K,0x0000>() & 0x07), data | 0x07 );
						ppu.SetMirroring( (address & 0x2) ? Ppu::NMT_H : Ppu::NMT_V );
					}
				}
			}
		}
	}
}
