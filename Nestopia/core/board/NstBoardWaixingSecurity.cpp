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
#include "NstBoardWaixing.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Waixing
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Security::SubReset(const bool hard)
				{
					if (hard)
						exReg = 0;

					Mmc3::SubReset( hard );

					Map( 0x5000U, &Security::Poke_5000 );
				}

				void Security::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'W','S','C'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								exReg = state.Read8() & 0x2;

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Security::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( exReg ).End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Security,5000)
				{
					data &= 0x2;

					if (exReg != data)
					{
						exReg = data;
						Mmc3::UpdatePrg();
						Mmc3::UpdateChr();
					}
				}

				void Security::UpdatePrg(uint address,uint bank)
				{
					prg.SwapBank<SIZE_8K>
					(
						address, !exReg ? bank :
						(
							(bank << 0 & 0x01) |
							(bank >> 3 & 0x02) |
							(bank >> 1 & 0x04) |
							(bank << 2 & 0x18)
						)
					);
				}

				void Security::UpdateChr(uint address,uint bank) const
				{
					chr.SwapBank<SIZE_1K>
					(
						address, !exReg ? bank :
						(
							(bank >> 0 & 0x03) |
							(bank >> 1 & 0x04) |
							(bank >> 4 & 0x08) |
							(bank >> 2 & 0x10) |
							(bank << 3 & 0x20) |
							(bank << 2 & 0x40) |
							(bank << 2 & 0x80)
						)
					);
				}
			}
		}
	}
}
