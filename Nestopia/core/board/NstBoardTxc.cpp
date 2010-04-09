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
#include "NstBoardTxc.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Txc
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void T22211A::SubReset(const bool hard)
				{
					Map( 0x4100U,          &T22211A::Peek_4100 );
					Map( 0x4100U, 0x4103U, &T22211A::Poke_4100 );
					Map( 0x8000U, 0xFFFFU, &T22211A::Poke_8000 );

					if (hard)
					{
						for (uint i=0; i < 4; ++i)
							regs[i] = 0;

						prg.SwapBank<SIZE_32K,0x0000>(0);
					}
				}

				void T22211B::SubReset(const bool hard)
				{
					T22211A::SubReset( hard );

					Map( 0x8000U, 0xFFFFU, &T22211B::Poke_8000 );
				}

				void T22211C::SubReset(const bool hard)
				{
					T22211A::SubReset( hard );

					Map( 0x4100U, &T22211C::Peek_4100 );
				}

				void T22211A::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'T','2','1'>::V) );

					if (baseChunk == AsciiId<'T','2','1'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								state.Read( regs );

							state.End();
						}
					}
				}

				void T22211A::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'T','2','1'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( regs ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK(T22211A,4100)
				{
					return (uint(regs[1]) ^ regs[2]) | 0x40;
				}

				NES_PEEK(T22211C,4100)
				{
					return (uint(regs[1]) ^ regs[2]) | 0x41;
				}

				NES_POKE_AD(T22211A,4100)
				{
					regs[address & 0x3] = data;
				}

				NES_POKE(T22211A,8000)
				{
					ppu.Update();
					prg.SwapBank<SIZE_32K,0x0000>( regs[2] >> 2 );
					chr.SwapBank<SIZE_8K,0x0000>( regs[2] );
				}

				NES_POKE_D(T22211B,8000)
				{
					ppu.Update();
					prg.SwapBank<SIZE_32K,0x0000>( regs[2] >> 2 );
					chr.SwapBank<SIZE_8K,0x0000>( ((data^regs[2]) >> 3 & 0x2) | ((data^regs[2]) >> 5 & 0x1) );
				}
			}
		}
	}
}
