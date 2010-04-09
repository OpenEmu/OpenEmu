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
#include "NstBoardBmcSuperVision16in1.hpp"
#include "../NstCrc32.hpp"

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

				SuperVision16in1::SuperVision16in1(const Context& c)
				:
				Board      (c),
				epromFirst (c.prg.Size() >= SIZE_32K && Crc32::Compute(c.prg.Mem(),SIZE_32K) == EPROM_CRC)
				{
				}

				void SuperVision16in1::SubReset(const bool hard)
				{
					if (hard)
					{
						regs[0] = 0;
						regs[1] = 0;

						UpdatePrg();
					}

					Map( 0x6000U, 0x7FFFU, &SuperVision16in1::Peek_6000, &SuperVision16in1::Poke_6000 );
					Map( 0x8000U, 0xFFFFU, &SuperVision16in1::Poke_8000  );
				}

				void SuperVision16in1::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','S','V'>::V) );

					if (baseChunk == AsciiId<'B','S','V'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								{
									State::Loader::Data<2> data( state );

									regs[0] = data[0];
									regs[1] = data[1];
								}

								UpdatePrg();
							}

							state.End();
						}
					}
				}

				void SuperVision16in1::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','S','V'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write16( regs[0] | uint(regs[1]) << 8 ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void SuperVision16in1::UpdatePrg()
				{
					const uint r = regs[0] << 3 & 0x78;

					wrk.SwapBank<SIZE_8K,0x0000>
					(
						(r << 1 | 0xF) + (epromFirst ? 0x4 : 0x0)
					);

					prg.SwapBanks<SIZE_16K,0x0000>
					(
						(regs[0] & 0x10) ? (r | (regs[1] & 0x7)) + (epromFirst ? 0x2 : 0x0) : epromFirst ? 0x00 : 0x80,
						(regs[0] & 0x10) ? (r | (0xFF    & 0x7)) + (epromFirst ? 0x2 : 0x0) : epromFirst ? 0x01 : 0x81
					);
				}

				NES_PEEK_A(SuperVision16in1,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_POKE_D(SuperVision16in1,6000)
				{
					regs[0] = data;
					UpdatePrg();
					ppu.SetMirroring( (data & 0x20) ? Ppu::NMT_H : Ppu::NMT_V );
				}

				NES_POKE_D(SuperVision16in1,8000)
				{
					regs[1] = data;
					UpdatePrg();
				}
			}
		}
	}
}
