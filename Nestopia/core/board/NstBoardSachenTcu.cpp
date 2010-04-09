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
#include "NstBoardSachenTcu.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sachen
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Tcu01::SubReset(const bool hard)
				{
					for (dword i=0x4100; i < 0x10000; i += 0x200)
					{
						for (uint j=0x2; j < 0x100; j += 0x4)
							Map( i + j, &Tcu01::Poke_4102 );
					}

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				void Tcu02::SubReset(const bool hard)
				{
					for (uint i=0x4100; i < 0x6000; i += 0x200)
					{
						for (uint j=0x0; j < 0x100; j += 0x4)
						{
							Map( i + j + 0x0, &Tcu02::Peek_4100 );
							Map( i + j + 0x2, &Tcu02::Poke_4102 );
						}
					}

					if (hard)
						reg = 0;
				}

				void Tcu02::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'S','0','2'>::V) );

					if (baseChunk == AsciiId<'S','0','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								reg = state.Read8();

							state.End();
						}
					}
				}

				void Tcu02::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'S','0','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK(Tcu02,4100)
				{
					return reg | 0x40;
				}

				NES_POKE_D(Tcu01,4102)
				{
					ppu.Update();
					prg.SwapBank<SIZE_32K,0x0000>( (data >> 6 & 0x2) | (data >> 2 & 0x1) );
					chr.SwapBank<SIZE_8K,0x0000>( data >> 3 );
				}

				NES_POKE_D(Tcu02,4102)
				{
					ppu.Update();
					reg = (data & 0x30) | ((data+3) & 0x0F);
					chr.SwapBank<SIZE_8K,0x0000>( reg );
				}
			}
		}
	}
}
