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
#include "NstBoardZz.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			void Zz::SubReset(const bool hard)
			{
				if (hard)
					exReg = 0x0;

				Mmc3::SubReset( hard );

				Map( 0x6000U, 0x7FFFU, &Zz::Poke_6000 );
			}

			void Zz::SubLoad(State::Loader& state,const dword baseChunk)
			{
				if (baseChunk == AsciiId<'Z','Z'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'R','E','G'>::V)
							exReg = state.Read8() & 0x7;

						state.End();
					}
				}
				else
				{
					Mmc3::SubLoad( state, baseChunk );
				}
			}

			void Zz::SubSave(State::Saver& state) const
			{
				Mmc3::SubSave( state );
				state.Begin( AsciiId<'Z','Z'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( exReg ).End().End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void NST_FASTCALL Zz::UpdatePrg(uint address,uint bank)
			{
				prg.SwapBank<SIZE_8K>
				(
					address,
					(exReg << 2 & 0x10) | ((exReg & 0x3) == 0x3 ? 0x08 : 0x00) | (bank & (exReg << 1 | 0x7))
				);
			}

			void NST_FASTCALL Zz::UpdateChr(uint address,uint bank) const
			{
				chr.SwapBank<SIZE_1K>( address, (exReg << 5 & 0x80) | (bank & 0x7F) );
			}

			NES_POKE_D(Zz,6000)
			{
				data &= 0x7;

				if (exReg != data)
				{
					exReg = data;
					Mmc3::UpdatePrg();
					Mmc3::UpdateChr();
				}
			}
		}
	}
}
