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
#include "NstBoardBmcResetBased4in1.hpp"

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

				void ResetBased4in1::SubReset(const bool hard)
				{
					if (hard)
						resetSwitch = 0;
					else
						resetSwitch = (resetSwitch + 1) & 0x3;

					chr.SwapBank<SIZE_8K,0x0000>( resetSwitch );
					prg.SwapBanks<SIZE_16K,0x0000>( resetSwitch, resetSwitch );
				}

				void ResetBased4in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','R','4'>::V) );

					if (baseChunk == AsciiId<'B','R','4'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								resetSwitch = state.Read8() & 0x3;

							state.End();
						}
					}
				}

				void ResetBased4in1::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','R','4'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( resetSwitch ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif
			}
		}
	}
}
