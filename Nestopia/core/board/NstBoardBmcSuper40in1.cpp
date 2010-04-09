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
#include "NstBoardBmcSuper40in1.hpp"

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

				void Super40in1::SubReset(const bool hard)
				{
					reg = 0x00;

					for (uint i=0x6000; i < 0x7000; i += 0x2)
					{
						Map( i + 0x0, &Super40in1::Poke_6000 );
						Map( i + 0x1, &Super40in1::Poke_6001 );
					}

					if (hard)
						NES_DO_POKE(6000,0x6000,0x00);
				}

				void Super40in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','S','4'>::V) );

					if (baseChunk == AsciiId<'B','S','4'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								reg = state.Read8() & 0x20;

							state.End();
						}
					}
				}

				void Super40in1::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','S','4'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Super40in1,6000)
				{
					if (!reg)
					{
						reg = data & 0x20;
						prg.SwapBanks<SIZE_16K,0x0000>( data & ~(~data >> 3 & 0x1), data | (~data >> 3 & 0x1) );
						ppu.SetMirroring( (data & 0x10) ? Ppu::NMT_H : Ppu::NMT_V );
					}
				}

				NES_POKE_D(Super40in1,6001)
				{
					if (!reg)
					{
						ppu.Update();
						chr.SwapBank<SIZE_8K,0x0000>( data );
					}
				}
			}
		}
	}
}
