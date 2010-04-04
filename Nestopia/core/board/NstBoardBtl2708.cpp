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
#include "NstBoardBtl2708.hpp"

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

				void B2708::SubReset(const bool hard)
				{
					if (hard)
					{
						mode = 0;
						wrk.Source(1).SwapBank<SIZE_8K,0x0000>( 0 );
						prg.SwapBank<SIZE_32K,0x0000>( ~0U );
					}

					Map( 0x6000U, 0x7FFFU, &B2708::Peek_6000, &B2708::Poke_6000 );
					Map( 0x8000U, 0x8FFFU, &B2708::Poke_8000 );
					Map( 0xB800U, 0xBFFFU, &B2708::Peek_B800, &B2708::Poke_B800 );
					Map( 0xC000U, 0xD7FFU, &B2708::Peek_C000, &B2708::Poke_B800 );
					Map( 0xE000U, 0xEFFFU, &B2708::Poke_E000 );
					Map( 0xF000U, 0xFFFFU, &B2708::Poke_F000 );
				}

				void B2708::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'2','7','8'>::V) );

					if (baseChunk == AsciiId<'2','7','8'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								mode = state.Read8() & 0x10;

							state.End();
						}
					}
				}

				void B2708::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'2','7','8'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( mode ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(B2708,6000)
				{
					return mode ? wrk[0][address - 0x6000] : wrk.Source()[address - 0x6000];
				}

				NES_POKE_AD(B2708,6000)
				{
					wrk.Source()[address - 0x6000] = data;
				}

				NES_POKE_D(B2708,8000)
				{
					wrk.Source(1).SwapBank<SIZE_8K,0x0000>( data & 0xF );
				}

				NES_PEEK_A(B2708,B800)
				{
					return mode ? prg[1][address - 0xA000] : wrk.Source()[address - 0x9800];
				}

				NES_POKE_AD(B2708,B800)
				{
					wrk.Source()[address - 0x9800] = data;
				}

				NES_PEEK_A(B2708,C000)
				{
					return mode ? prg[2][address - 0xC000] : wrk.Source()[address - 0x9800];
				}

				NES_POKE_D(B2708,E000)
				{
					ppu.SetMirroring( (data & 0x8) ? Ppu::NMT_H : Ppu::NMT_V );
				}

				NES_POKE_D(B2708,F000)
				{
					mode = data & 0x10;
				}
			}
		}
	}
}
