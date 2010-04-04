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

#include <cstring>
#include "NstBoard.hpp"
#include "NstBoardTaitoX1017.hpp"
#include "../NstFile.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Taito
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				X1017::X1017(const Context& c)
				: Board(c)
				{
					std::memset( ram, 0, sizeof(ram) );
				}

				void X1017::SubReset(const bool hard)
				{
					if (hard)
					{
						regs.ctrl = 0;

						regs.security[0] = 0;
						regs.security[1] = 0;
						regs.security[2] = 0;

						regs.unused[0] = 0;
						regs.unused[1] = 0;
						regs.unused[2] = 0;

						StoreChr();
					}

					Map( 0x6000U, 0x73FFU, &X1017::Peek_6000, &X1017::Poke_6000 );
					Map( 0x7EF0U, 0x7EF1U, &X1017::Poke_7EF0 );
					Map( 0x7EF2U, 0x7EF5U, &X1017::Poke_7EF2 );
					Map( 0x7EF6U,          &X1017::Poke_7EF6 );
					Map( 0x7EF7U, 0x7EF9U, &X1017::Poke_7EF7 );
					Map( 0x7EFAU,          &X1017::Poke_7EFA );
					Map( 0x7EFBU,          &X1017::Poke_7EFB );
					Map( 0x7EFCU,          &X1017::Poke_7EFC );
					Map( 0x7EFDU, 0x7EFFU, &X1017::Poke_7EFD );
				}

				void X1017::StoreChr()
				{
					for (uint i=0; i < 2; ++i)
						regs.chr[i] = chr.GetBank<SIZE_2K>( i << 11 );

					for (uint i=2; i < 6; ++i)
						regs.chr[i] = chr.GetBank<SIZE_1K>( 0x1000 | (i - 2) << 10 );
				}

				void X1017::Load(File& file)
				{
					if (board.HasBattery())
						file.Load( File::BATTERY, ram, sizeof(ram) );
				}

				void X1017::Save(File& file) const
				{
					if (board.HasBattery())
						file.Save( File::BATTERY, ram, sizeof(ram) );
				}

				void X1017::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'T','1','7'>::V) );

					if (baseChunk == AsciiId<'T','1','7'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:
								{
									State::Loader::Data<7> data( state );

									regs.ctrl = data[0];
									regs.security[0] = data[1];
									regs.security[1] = data[2];
									regs.security[2] = data[3];
									regs.unused[0] = data[4];
									regs.unused[1] = data[5];
									regs.unused[2] = data[6];
									break;
								}

								case AsciiId<'R','A','M'>::V:

									state.Uncompress( ram );
									break;
							}

							state.End();
						}
					}

					StoreChr();
				}

				void X1017::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'T','1','7'>::V );

					const byte data[7] =
					{
						regs.ctrl,
						regs.security[0],
						regs.security[1],
						regs.security[2],
						regs.unused[0],
						regs.unused[1],
						regs.unused[2]
					};

					state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();

					state.Begin( AsciiId<'R','A','M'>::V ).Compress( ram ).End();

					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void X1017::UpdateChr() const
				{
					ppu.Update();

					const uint swap = regs.ctrl << 11 & 0x1000;

					chr.SwapBanks<SIZE_2K>( 0x0000 ^ swap, regs.chr[0], regs.chr[1] );
					chr.SwapBanks<SIZE_1K>( 0x1000 ^ swap, regs.chr[2], regs.chr[3], regs.chr[4], regs.chr[5] );
				}

				NES_PEEK_A(X1017,6000)
				{
					return ram[address - 0x6000];
				}

				NES_POKE_AD(X1017,6000)
				{
					ram[address - 0x6000] = data;
				}

				NES_POKE_AD(X1017,7EF2)
				{
					address &= 0x7;

					if (regs.chr[address] != data)
					{
						regs.chr[address] = data;
						UpdateChr();
					}
				}

				NES_POKE_AD(X1017,7EF0)
				{
					NES_DO_POKE( 7EF2, address, data >> 1 );
				}

				NES_POKE_D(X1017,7EF6)
				{
					if (regs.ctrl != data)
					{
						regs.ctrl = data;
						UpdateChr();
						ppu.SetMirroring( (regs.ctrl & 0x1) ? Ppu::NMT_V : Ppu::NMT_H );
					}
				}

				NES_POKE_AD(X1017,7EF7)
				{
					regs.security[address - 0x7EF7] = data;
				}

				NES_POKE_D(X1017,7EFA)
				{
					prg.SwapBank<SIZE_8K,0x0000>( data >> 2 );
				}

				NES_POKE_D(X1017,7EFB)
				{
					prg.SwapBank<SIZE_8K,0x2000>( data >> 2 );
				}

				NES_POKE_D(X1017,7EFC)
				{
					prg.SwapBank<SIZE_8K,0x4000>( data >> 2 );
				}

				NES_POKE_AD(X1017,7EFD)
				{
					regs.unused[address - 0x7EFD] = data;
				}
			}
		}
	}
}
