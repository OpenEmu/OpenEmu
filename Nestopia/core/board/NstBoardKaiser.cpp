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

#include "../NstClock.hpp"
#include "NstBoard.hpp"
#include "NstBoardKaiser.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Kaiser
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Ks7058::SubReset(bool)
				{
					for (uint i=0x000; i < 0x1000; i += 0x100)
					{
						Map( 0xF000+i, 0xF07F+i, CHR_SWAP_4K_0 );
						Map( 0xF080+i, 0xF0FF+i, CHR_SWAP_4K_1 );
					}
				}

				void Ks7022::SubReset(const bool hard)
				{
					reg = 0;

					if (hard)
						prg.SwapBanks<SIZE_16K,0x0000>( 0, 0 );

					Map( 0x8000, &Ks7022::Poke_8000 );
					Map( 0xA000, &Ks7022::Poke_A000 );
					Map( 0xFFFC, &Ks7022::Peek_FFFC );
				}

				Ks202::Ks202(const Context& c)
				: Board(c), irq(*c.cpu) {}

				void Ks202::Irq::Reset(const bool hard)
				{
					if (hard)
					{
						count = 0;
						latch = 0;
						ctrl = 0;
					}
				}

				void Ks202::SubReset(const bool hard)
				{
					Map( 0x8000U, 0x8FFFU, &Ks202::Poke_8000 );
					Map( 0x9000U, 0x9FFFU, &Ks202::Poke_9000 );
					Map( 0xA000U, 0xAFFFU, &Ks202::Poke_A000 );
					Map( 0xB000U, 0xBFFFU, &Ks202::Poke_B000 );
					Map( 0xC000U, 0xCFFFU, &Ks202::Poke_C000 );
					Map( 0xD000U, 0xDFFFU, &Ks202::Poke_D000 );
					Map( 0xE000U, 0xEFFFU, &Ks202::Poke_E000 );
					Map( 0xF000U, 0xFFFFU, &Ks202::Poke_F000 );

					if (hard)
						ctrl = 0;

					irq.Reset( hard, hard ? false : irq.Connected() );
				}

				void Ks7032::SubReset(const bool hard)
				{
					Ks202::SubReset( hard );
					Map( 0x6000U, 0x7FFFU, &Ks7032::Peek_6000 );
				}

				void Ks7022::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'K','7','2'>::V) );

					if (baseChunk == AsciiId<'K','7','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								reg = state.Read8();

							state.End();
						}
					}
				}

				void Ks202::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'K','0','2'>::V) );

					if (baseChunk == AsciiId<'K','0','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:

									ctrl = state.Read8();
									break;

								case AsciiId<'I','R','Q'>::V:
								{
									State::Loader::Data<5> data( state );

									irq.unit.ctrl = data[0];
									irq.unit.count = data[1] | data[2] << 8;
									irq.unit.latch = data[3] | data[4] << 8;
									irq.Connect( data[0] & 0xF );

									break;
								}
							}

							state.End();
						}
					}
				}

				void Ks7022::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','7','2'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( reg ).End().End();
				}

				void Ks202::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','0','2'>::V );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( ctrl ).End();

					const byte data[5] =
					{
						irq.unit.ctrl,
						irq.unit.count & 0xFF,
						irq.unit.count >> 8,
						irq.unit.latch & 0xFF,
						irq.unit.latch >> 8
					};

					state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(Ks7032,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_POKE_D(Ks7022,8000)
				{
					ppu.SetMirroring( (data & 0x4) ? Ppu::NMT_H : Ppu::NMT_V );
				}

				NES_POKE_D(Ks7022,A000)
				{
					reg = data & 0xF;
				}

				NES_PEEK(Ks7022,FFFC)
				{
					ppu.Update();
					chr.SwapBank<SIZE_8K,0x0000>( reg );
					prg.SwapBanks<SIZE_16K,0x0000>( reg, reg );

					return prg.Peek(0x7FFC);
				}

				NES_POKE_D(Ks202,8000)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xFFF0) | (data & 0xF) << 0;
				}

				NES_POKE_D(Ks202,9000)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xFF0F) | (data & 0xF) << 4;
				}

				NES_POKE_D(Ks202,A000)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0xF0FF) | (data & 0xF) << 8;
				}

				NES_POKE_D(Ks202,B000)
				{
					irq.Update();
					irq.unit.latch = (irq.unit.latch & 0x0FFF) | (data & 0xF) << 12;
				}

				NES_POKE_D(Ks202,C000)
				{
					irq.Update();

					irq.unit.ctrl = data;

					if (irq.Connect( data & 0xF ))
						irq.unit.count = irq.unit.latch;

					irq.ClearIRQ();
				}

				NES_POKE(Ks202,D000)
				{
					irq.Update();
					irq.ClearIRQ();
				}

				NES_POKE_D(Ks202,E000)
				{
					ctrl = data;
				}

				NES_POKE_AD(Ks202,F000)
				{
					{
						uint offset = (ctrl & 0xF) - 1;

						if (offset < 3)
						{
							offset <<= 13;
							prg.SwapBank<SIZE_8K>( offset, (data & 0x0F) | (prg.GetBank<SIZE_8K>(offset) & 0x10) );
						}
						else if (offset < 4)
						{
							wrk.SwapBank<SIZE_8K,0x0000>( data );
						}
					}

					switch (address & 0xC00)
					{
						case 0x000:

							address &= 0x3;

							if (address < 3)
							{
								address <<= 13;
								prg.SwapBank<SIZE_8K>( address, (prg.GetBank<SIZE_8K>(address) & 0x0F) | (data & 0x10) );
							}
							break;

						case 0x800:

							ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_V : Ppu::NMT_H );
							break;

						case 0xC00:

							ppu.Update();
							chr.SwapBank<SIZE_1K>( (address & 0x7) << 10, data );
							break;
					}
				}

				bool Ks202::Irq::Clock()
				{
					return (count++ == 0xFFFF) ? (count=latch, true) : false;
				}

				void Ks202::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
