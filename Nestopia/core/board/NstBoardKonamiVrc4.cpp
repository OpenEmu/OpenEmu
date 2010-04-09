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
#include "NstBoardKonamiVrc4.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Konami
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				uint Vrc4::GetPrgLineShift(const Context& c,const uint pin,const uint def)
				{
					if (const Chips::Type* const vrc4 = c.chips.Find(L"Konami VRC IV"))
					{
						const uint line = *vrc4->Pin(pin).C(L"PRG").A();
						NST_VERIFY( line < 8 );

						if (line < 8)
							return line;
					}

					return def;
				}

				Vrc4::Vrc4(const Context& c)
				:
				Board    (c),
				irq      (*c.cpu),
				prgLineA (GetPrgLineShift(c,3,1)),
				prgLineB (GetPrgLineShift(c,4,0))
				{
				}

				void Vrc4::BaseIrq::Reset(bool)
				{
					ctrl = 0;
					count[0] = 0;
					count[1] = 0;
					latch = 0;
				}

				void Vrc4::SubReset(const bool hard)
				{
					if (hard)
						prgSwap = 0;

					irq.Reset( hard, hard ? false : irq.Connected() );

					if (const uint wram = board.GetWram())
						Map( 0x6000U, 0x6000 + NST_MIN(wram,SIZE_8K) - 1, &Vrc4::Peek_6000, &Vrc4::Poke_6000 );

					Map( 0x8000U, 0x8FFFU, &Vrc4::Poke_8000 );
					Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_1 );

					for (dword i=0x9000, a=9-prgLineA, b=8-prgLineB; i <= 0xFFFF; ++i)
					{
						switch ((i & 0xF000) | (i << a & 0x0200) | (i << b & 0x0100))
						{
							case 0x9000:
							case 0x9100: Map( i, NMT_SWAP_VH01    ); break;
							case 0x9200:
							case 0x9300: Map( i, &Vrc4::Poke_9000 ); break;
							case 0xB000: Map( i, &Vrc4::Poke_B000 ); break;
							case 0xB100: Map( i, &Vrc4::Poke_B001 ); break;
							case 0xB200: Map( i, &Vrc4::Poke_B002 ); break;
							case 0xB300: Map( i, &Vrc4::Poke_B003 ); break;
							case 0xC000: Map( i, &Vrc4::Poke_C000 ); break;
							case 0xC100: Map( i, &Vrc4::Poke_C001 ); break;
							case 0xC200: Map( i, &Vrc4::Poke_C002 ); break;
							case 0xC300: Map( i, &Vrc4::Poke_C003 ); break;
							case 0xD000: Map( i, &Vrc4::Poke_D000 ); break;
							case 0xD100: Map( i, &Vrc4::Poke_D001 ); break;
							case 0xD200: Map( i, &Vrc4::Poke_D002 ); break;
							case 0xD300: Map( i, &Vrc4::Poke_D003 ); break;
							case 0xE000: Map( i, &Vrc4::Poke_E000 ); break;
							case 0xE100: Map( i, &Vrc4::Poke_E001 ); break;
							case 0xE200: Map( i, &Vrc4::Poke_E002 ); break;
							case 0xE300: Map( i, &Vrc4::Poke_E003 ); break;
							case 0xF000: Map( i, &Vrc4::Poke_F000 ); break;
							case 0xF100: Map( i, &Vrc4::Poke_F001 ); break;
							case 0xF200: Map( i, &Vrc4::Poke_F002 ); break;
							case 0xF300: Map( i, &Vrc4::Poke_F003 ); break;
						}
					}
				}

				void Vrc4::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'K','V','4'>::V) );

					if (baseChunk == AsciiId<'K','V','4'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:

									prgSwap = state.Read8() & 0x2;
									break;

								case AsciiId<'I','R','Q'>::V:

									irq.LoadState( state );
									break;
							}

							state.End();
						}
					}
				}

				void Vrc4::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'K','V','4'>::V );

					state.Begin( AsciiId<'R','E','G'>::V ).Write8( prgSwap ).End();
					irq.SaveState( state, AsciiId<'I','R','Q'>::V );

					state.End();
				}

				void Vrc4::Irq::LoadState(State::Loader& state)
				{
					State::Loader::Data<5> data( state );

					unit.ctrl = data[0] & (BaseIrq::ENABLE_1|BaseIrq::NO_PPU_SYNC);
					Connect( data[0] & BaseIrq::ENABLE_0 );
					unit.latch = data[1];
					unit.count[0] = NST_MIN(340,data[2] | data[3] << 8);
					unit.count[1] = data[4];
				}

				void Vrc4::Irq::SaveState(State::Saver& state,const dword chunk) const
				{
					const byte data[5] =
					{
						unit.ctrl | (Connected() ? BaseIrq::ENABLE_0 : 0),
						unit.latch,
						unit.count[0] & 0xFF,
						unit.count[0] >> 8,
						unit.count[1]
					};

					state.Begin( chunk ).Write( data ).End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(Vrc4,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_POKE_AD(Vrc4,6000)
				{
					wrk[0][address - 0x6000] = data;
				}

				NES_POKE_D(Vrc4,8000)
				{
					prg.SwapBank<SIZE_8K>( prgSwap << 13, data );
				}

				NES_POKE_D(Vrc4,9000)
				{
					data &= 0x2;

					if (prgSwap != data)
					{
						prgSwap = data;
						prg.SwapPages<SIZE_8K,0x0000,0x4000>();
					}
				}

				template<uint OFFSET>
				void Vrc4::SwapChr(uint address,uint subBank) const
				{
					ppu.Update();
					chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & 0xF0U >> OFFSET) | ((subBank & 0xF) << OFFSET) );
				}

				NES_POKE_D(Vrc4,B000) { SwapChr<0>( 0x0000, data ); }
				NES_POKE_D(Vrc4,B001) { SwapChr<4>( 0x0000, data ); }
				NES_POKE_D(Vrc4,B002) { SwapChr<0>( 0x0400, data ); }
				NES_POKE_D(Vrc4,B003) { SwapChr<4>( 0x0400, data ); }
				NES_POKE_D(Vrc4,C000) { SwapChr<0>( 0x0800, data ); }
				NES_POKE_D(Vrc4,C001) { SwapChr<4>( 0x0800, data ); }
				NES_POKE_D(Vrc4,C002) { SwapChr<0>( 0x0C00, data ); }
				NES_POKE_D(Vrc4,C003) { SwapChr<4>( 0x0C00, data ); }
				NES_POKE_D(Vrc4,D000) { SwapChr<0>( 0x1000, data ); }
				NES_POKE_D(Vrc4,D001) { SwapChr<4>( 0x1000, data ); }
				NES_POKE_D(Vrc4,D002) { SwapChr<0>( 0x1400, data ); }
				NES_POKE_D(Vrc4,D003) { SwapChr<4>( 0x1400, data ); }
				NES_POKE_D(Vrc4,E000) { SwapChr<0>( 0x1800, data ); }
				NES_POKE_D(Vrc4,E001) { SwapChr<4>( 0x1800, data ); }
				NES_POKE_D(Vrc4,E002) { SwapChr<0>( 0x1C00, data ); }
				NES_POKE_D(Vrc4,E003) { SwapChr<4>( 0x1C00, data ); }

				void Vrc4::Irq::WriteLatch0(const uint data)
				{
					Update();
					unit.latch = (unit.latch & 0xF0) | (data << 0 & 0x0F);
				}

				void Vrc4::Irq::WriteLatch1(const uint data)
				{
					Update();
					unit.latch = (unit.latch & 0x0F) | (data << 4 & 0xF0);
				}

				void Vrc4::Irq::Toggle(const uint data)
				{
					Update();
					unit.ctrl = data & (BaseIrq::ENABLE_1|BaseIrq::NO_PPU_SYNC);

					if (Connect( data & BaseIrq::ENABLE_0 ))
					{
						unit.count[0] = 0;
						unit.count[1] = unit.latch;
					}

					ClearIRQ();
				}

				void Vrc4::Irq::Toggle()
				{
					Update();
					Connect( unit.ctrl & BaseIrq::ENABLE_1 );
					ClearIRQ();
				}

				NES_POKE_D(Vrc4,F000)
				{
					irq.WriteLatch0( data );
				}

				NES_POKE_D(Vrc4,F001)
				{
					irq.WriteLatch1( data );
				}

				NES_POKE_D(Vrc4,F002)
				{
					irq.Toggle( data );
				}

				NES_POKE(Vrc4,F003)
				{
					irq.Toggle();
				}

				bool Vrc4::BaseIrq::Clock()
				{
					if (!(ctrl & NO_PPU_SYNC))
					{
						if (count[0] < 341-3)
						{
							count[0] += 3;
							return false;
						}

						count[0] -= 341-3;
					}

					if (count[1] != 0xFF)
					{
						count[1]++;
						return false;
					}

					count[1] = latch;

					return true;
				}

				void Vrc4::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
