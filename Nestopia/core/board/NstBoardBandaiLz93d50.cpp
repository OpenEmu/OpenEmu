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
#include "../NstClock.hpp"
#include "NstBoard.hpp"
#include "NstBoardBandaiLz93d50.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bandai
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Lz93d50::Lz93d50(const Context& c)
				: Board(c), irq(*c.cpu)
				{
					if (board.GetWram())
						wrk.Source().Fill(0xFF);
				}

				void Lz93d50::Irq::Reset(const bool hard)
				{
					if (hard)
					{
						latch = 0;
						count = 0;
					}
				}

				void Lz93d50::SubReset(const bool hard)
				{
					irq.Reset( hard, hard ? false : irq.Connected() );

					for (uint i=0; i < 8; ++i)
						regs[i] = 0;

					if (hard)
					{
						for (uint i=board.GetSavableWram(), n=board.GetWram(); i < n; ++i)
							*wrk.Source().Mem(i) = 0xFF;
					}

					const uint offset = (board.GetWram() ? 0x8000 : 0x6000);

					for (dword i=offset; i <= 0xFFFF; i += 0x10)
					{
						Map( i + 0x9, NMT_SWAP_VH01       );
						Map( i + 0xA, &Lz93d50::Poke_800A );
						Map( i + 0xB, &Lz93d50::Poke_800B );
						Map( i + 0xC, &Lz93d50::Poke_800C );
					}

					if (prg.Source().Size() < SIZE_512K)
					{
						for (dword i=offset; i <= 0xFFFF; i += 0x10)
							Map( i + 0x8, PRG_SWAP_16K_0 );
					}
					else
					{
						for (dword i=offset; i <= 0xFFFF; i += 0x10)
						{
							Map( uint(i + 0x0), uint(i + 0x7), &Lz93d50::Poke_8000 );
							Map( uint(i + 0x8),                &Lz93d50::Poke_8008 );
						}

						if (hard)
							prg.SwapBank<SIZE_16K,0x4000>( 0xF );
					}

					if (chr.Source().Size() > SIZE_8K)
					{
						for (dword i=offset; i <= 0xFFFF; i += 0x10)
						{
							Map( i + 0x0, CHR_SWAP_1K_0 );
							Map( i + 0x1, CHR_SWAP_1K_1 );
							Map( i + 0x2, CHR_SWAP_1K_2 );
							Map( i + 0x3, CHR_SWAP_1K_3 );
							Map( i + 0x4, CHR_SWAP_1K_4 );
							Map( i + 0x5, CHR_SWAP_1K_5 );
							Map( i + 0x6, CHR_SWAP_1K_6 );
							Map( i + 0x7, CHR_SWAP_1K_7 );
						}
					}
				}

				void Lz93d50::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','L','Z'>::V) );

					if (baseChunk == AsciiId<'B','L','Z'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:

									NST_VERIFY( prg.Source().Size() >= SIZE_512K );

									if (prg.Source().Size() >= SIZE_512K)
										state.Read( regs );

									break;

								case AsciiId<'I','R','Q'>::V:
								{
									State::Loader::Data<5> data( state );

									irq.Connect( data[0] & 0x1 );
									irq.unit.latch = data[1] | data[2] << 8;
									irq.unit.count = data[3] | data[4] << 8;
									break;
								}
							}

							state.End();
						}
					}
				}

				void Lz93d50::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'B','L','Z'>::V );

					if (prg.Source().Size() >= SIZE_512K)
						state.Begin( AsciiId<'R','E','G'>::V ).Write( regs ).End();

					const byte data[5] =
					{
						irq.Connected() ? 0x1 : 0x0,
						irq.unit.latch >> 0 & 0xFF,
						irq.unit.latch >> 8 & 0xFF,
						irq.unit.count >> 0 & 0xFF,
						irq.unit.count >> 8 & 0xFF
					};

					state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();

					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(Lz93d50,8000)
				{
					regs[address & 0x7] = data;
					data = 0;

					for (uint i=0; i < 8; ++i)
						data |= regs[i] << 4 & 0x10U;

					prg.SwapBanks<SIZE_16K,0x0000>( data | (prg.GetBank<SIZE_16K,0x0000>() & 0x0F), data | 0xF );
				}

				NES_POKE_D(Lz93d50,8008)
				{
					prg.SwapBank<SIZE_16K,0x0000>( (prg.GetBank<SIZE_16K,0x0000>() & 0x10) | (data & 0x0F) );
				}

				NES_POKE_D(Lz93d50,800A)
				{
					irq.Update();
					irq.unit.count = irq.unit.latch;
					irq.Connect( data & 0x1 );
					irq.ClearIRQ();
				}

				NES_POKE_D(Lz93d50,800B)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xFF00) | data << 0;
				}

				NES_POKE_D(Lz93d50,800C)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0x00FF) | data << 8;
				}

				bool Lz93d50::Irq::Clock()
				{
					return (count-- & 0xFFFF) == 0;
				}

				void Lz93d50::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
