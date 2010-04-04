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
#include "NstBoardFujiya.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Fujiya
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Standard::SubReset(bool)
				{
					reg = 0x00;

					Map( 0x6502U, &Standard::Poke_7000 );
					Map( 0x7000U, &Standard::Poke_7000 );
					Map( 0x7001U, &Standard::Peek_7001 );
					Map( 0x7777U, &Standard::Peek_7001 );
				}

				void Standard::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'F','U','A'>::V) );

					if (baseChunk == AsciiId<'F','U','A'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								reg = state.Read8() & 0x80;

							state.End();
						}
					}
				}

				void Standard::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'F','U','A'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Standard,7000)
				{
					reg = data << 1 & 0x80;
				}

				NES_PEEK_A(Standard,7001)
				{
					return reg | (address >> 8 & 0x7F);
				}
			}
		}
	}
}
