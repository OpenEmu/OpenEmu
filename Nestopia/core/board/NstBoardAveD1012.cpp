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
#include "NstBoardAveD1012.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Ave
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void D1012::SubReset(const bool hard)
				{
					Map( 0xFF80U, 0xFF9FU, &D1012::Peek_FF80, &D1012::Poke_FF80 );
					Map( 0xFFE8U, 0xFFF7U, &D1012::Peek_FFE8, &D1012::Poke_FFE8 );

					if (hard)
					{
						regs[0] = 0;
						regs[1] = 0;

						Update();
					}
				}

				void D1012::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'A','D','1'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<2> data( state );

								regs[0] = data[0];
								regs[1] = data[1];
							}

							state.End();
						}
					}
				}

				void D1012::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'A','D','1'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write16( regs[0] | uint(regs[1]) << 8 ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void D1012::Update()
				{
					prg.SwapBank<SIZE_32K,0x0000>( (regs[0] & 0xE) | (regs[regs[0] >> 6 & 0x1] & 0x1) );
					chr.SwapBank<SIZE_8K,0x0000>( (regs[0] << 2 & (regs[0] >> 4 & 0x4 ^ 0x3C)) | (regs[1] >> 4 & (regs[0] >> 4 & 0x4 | 0x3)) );
				}

				NES_POKE_D(D1012,FF80)
				{
					if (!(regs[0] & 0x3F))
					{
						regs[0] = data;
						ppu.SetMirroring( (data & 0x80) ? Ppu::NMT_H : Ppu::NMT_V );
						Update();
					}
				}

				NES_PEEK_A(D1012,FF80)
				{
					const uint data = prg[3][address - 0xE000];
					NES_DO_POKE(FF80,address,data);
					return data;
				}

				NES_POKE_D(D1012,FFE8)
				{
					regs[1] = data;
					ppu.Update();
					Update();
				}

				NES_PEEK_A(D1012,FFE8)
				{
					const uint data = prg[3][address - 0xE000];
					NES_DO_POKE(FFE8,address,data);
					return data;
				}
			}
		}
	}
}
