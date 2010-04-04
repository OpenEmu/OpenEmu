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
#include "NstBoardRexSoftDb5z.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace RexSoft
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Dbz5::SubReset(const bool hard)
				{
					if (hard)
						exReg = 0;

					Mmc3::SubReset( hard );

					Map( 0x4100U, 0x5FFFU, &Dbz5::Peek_4100, &Dbz5::Poke_4100 );
					Map( 0x6000U, 0x7FFFU, &Dbz5::Peek_4100 );
				}

				void Dbz5::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'R','Z','5'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								exReg = state.Read8();

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Dbz5::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'R','Z','5'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( exReg ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Dbz5,4100)
				{
					if (exReg != data)
					{
						exReg = data;
						Mmc3::UpdateChr();
					}
				}

				NES_PEEK(Dbz5,4100)
				{
					return 0x01;
				}

				void NST_FASTCALL Dbz5::UpdateChr(uint address,uint bank) const
				{
					chr.SwapBank<SIZE_1K>( address, (exReg << ((address & 0x1000) ? 4 : 8) & 0x100) | bank );
				}
			}
		}
	}
}
