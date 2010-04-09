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
#include "NstBoardBmcY2k64in1.hpp"

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

				void Y2k64in1::SubReset(bool)
				{
					Map( 0x5000U, 0x5003U, &Y2k64in1::Poke_5000 );
					Map( 0x8000U, 0xFFFFU, &Y2k64in1::Poke_8000 );

					regs[0] = 0x80;
					regs[1] = 0x43;
					regs[2] = 0x00;
					regs[3] = 0x00;

					Update();
				}

				void Y2k64in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','Y','2'>::V) );

					if (baseChunk == AsciiId<'B','Y','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								state.Read( regs );

							state.End();
						}
					}
				}

				void Y2k64in1::SubSave(State::Saver& state) const
				{
					state.Begin(AsciiId<'B','Y','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( regs ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void Y2k64in1::Update()
				{
					uint bank = regs[1] & 0x1FU;

					if (regs[0] & 0x80U & regs[1])
					{
						prg.SwapBank<SIZE_32K,0x0000>( bank );
					}
					else
					{
						bank = (bank << 1) | (regs[1] >> 6 & 0x1U);
						prg.SwapBank<SIZE_16K,0x4000>( bank );

						if (regs[0] & 0x80U)
							prg.SwapBank<SIZE_16K,0x0000>( bank );
					}

					ppu.SetMirroring( (regs[0] & 0x20U) ? Ppu::NMT_H : Ppu::NMT_V );
					chr.SwapBank<SIZE_8K,0x0000>( (regs[2] << 2) | (regs[0] >> 1 & 0x3U) );
				}

				NES_POKE_AD(Y2k64in1,5000)
				{
					regs[address & 0x3] = data;
					Update();
				}

				NES_POKE_D(Y2k64in1,8000)
				{
					regs[3] = data;
					Update();
				}
			}
		}
	}
}
