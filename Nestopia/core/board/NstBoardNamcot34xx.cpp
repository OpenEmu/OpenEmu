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
#include "NstBoardNamcot34xx.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Namcot
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void N34x3::SubReset(const bool hard)
				{
					if (hard)
						ctrl = 0;

					for (uint i=0x8000; i < 0xA000; i += 0x2)
					{
						Map( i + 0x0, &N34x3::Poke_8000 );
						Map( i + 0x1, &N34x3::Poke_8001 );
					}
				}

				void N34xx::SubReset(const bool hard)
				{
					N34x3::SubReset( hard );

					for (uint i=0x0000; i < 0x8000; i += 0x2)
						Map( 0x8000 + i, &N34xx::Poke_8000 );
				}

				void N34x3::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'N','3','4'>::V) );

					if (baseChunk == AsciiId<'N','3','4'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								ctrl = state.Read8();

							state.End();
						}
					}
				}

				void N34x3::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'N','3','4'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( ctrl ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(N34x3,8000)
				{
					ctrl = data;
				}

				NES_POKE_D(N34x3,8001)
				{
					const uint mode = ctrl & 0x7;

					if (mode >= 6)
					{
						prg.SwapBank<SIZE_8K>( (mode-6) << 13, data );
					}
					else
					{
						ppu.Update();
						UpdateChr( mode, data & 0x3F );
					}
				}

				void NST_FASTCALL N34x3::UpdateChr(uint mode,uint data) const
				{
					if (mode >= 2)
						chr.SwapBank<SIZE_1K>( (mode+2) << 10, data | 0x40 );
					else
						chr.SwapBank<SIZE_2K>( mode << 11, data >> 1 );
				}

				void NST_FASTCALL N3446::UpdateChr(uint mode,uint data) const
				{
					NST_VERIFY( mode >= 2 );

					if (mode >= 2)
						chr.SwapBank<SIZE_2K>( (mode-2) << 11, data );
				}

				void NST_FASTCALL N3425::UpdateChr(uint mode,uint data) const
				{
					nmt.SwapBank<SIZE_1K>( mode << 9 & 0xC00, data >> 5 );
					N34x3::UpdateChr( mode, data );
				}

				NES_POKE_AD(N34xx,8000)
				{
					ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_1 : Ppu::NMT_0 );
					N34x3::NES_DO_POKE(8000,address,data);
				}
			}
		}
	}
}
