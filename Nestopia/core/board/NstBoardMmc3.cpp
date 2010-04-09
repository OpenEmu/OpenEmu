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

#include "../NstLog.hpp"
#include "NstBoard.hpp"
#include "NstBoardMmc3.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Mmc3::Mmc3(const Context& c,Revision revision)
			:
			Board (c),
			irq   (*c.cpu,*c.ppu,revision != REV_A)
			{
				switch (revision)
				{
					case Mmc3::REV_A: Log::Flush( "Board: MMC rev. A" NST_LINEBREAK ); break;
					case Mmc3::REV_B: Log::Flush( "Board: MMC rev. B" NST_LINEBREAK ); break;
					case Mmc3::REV_C: Log::Flush( "Board: MMC rev. C" NST_LINEBREAK ); break;
				}
			}

			void Mmc3::BaseIrq::Reset(const bool hard)
			{
				if (hard)
				{
					count = 0;
					latch = 0;
					reload = false;
					enabled = false;
				}
			}

			void Mmc3::SubReset(const bool hard)
			{
				if (hard)
				{
					regs.ctrl0 = 0;
					regs.ctrl1 = 0;

					banks.prg[0] = 0x00;
					banks.prg[1] = 0x01;
					banks.prg[2] = 0x3E;
					banks.prg[3] = 0x3F;

					for (uint i=0; i < 8; ++i)
						banks.chr[i] = i;

					wrk.Source().SetSecurity( false, false );
				}

				irq.Reset( hard );

				for (uint i=0x0000; i < 0x2000; i += 0x2)
				{
					Map( 0x8000 + i, &Mmc3::Poke_8000 );
					Map( 0x8001 + i, &Mmc3::Poke_8001 );
					Map( 0xA001 + i, &Mmc3::Poke_A001 );
					Map( 0xC000 + i, &Mmc3::Poke_C000 );
					Map( 0xC001 + i, &Mmc3::Poke_C001 );
					Map( 0xE000 + i, &Mmc3::Poke_E000 );
					Map( 0xE001 + i, &Mmc3::Poke_E001 );
				}

				if (board.GetNmt() != Type::NMT_FOURSCREEN)
				{
					for (uint i=0x0000; i < 0x2000; i += 0x2)
						Map( 0xA000 + i, NMT_SWAP_HV );
				}

				UpdatePrg();
				UpdateChr();
			}

			void Mmc3::BaseIrq::LoadState(State::Loader& state)
			{
				State::Loader::Data<3> data( state );

				enabled = data[0] & 0x1;
				reload = data[0] & 0x2;
				count = data[1];
				latch = data[2];
			}

			void Mmc3::BaseIrq::SaveState(State::Saver& state,const dword chunk) const
			{
				const byte data[3] =
				{
					(enabled ? 0x1U : 0x0U) | (reload ? 0x2U : 0x0U),
					count,
					latch
				};

				state.Begin( chunk ).Write( data ).End();
			}

			void Mmc3::SubLoad(State::Loader& state,const dword baseChunk)
			{
				NST_VERIFY( baseChunk == (AsciiId<'M','M','3'>::V) );

				if (baseChunk == AsciiId<'M','M','3'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						switch (chunk)
						{
							case AsciiId<'R','E','G'>::V:
							{
								State::Loader::Data<12> data( state );

								regs.ctrl0   = data[0];
								regs.ctrl1   = data[1];
								banks.prg[0] = data[2] & 0x3FU;
								banks.prg[1] = data[3] & 0x3FU;
								banks.chr[0] = (data[6] & 0x7FU) << 1;
								banks.chr[1] = banks.chr[0] | 0x01U;
								banks.chr[2] = (data[7] & 0x7FU) << 1;
								banks.chr[3] = banks.chr[2] | 0x01U;
								banks.chr[4] = data[8];
								banks.chr[5] = data[9];
								banks.chr[6] = data[10];
								banks.chr[7] = data[11];
								break;
							}

							case AsciiId<'I','R','Q'>::V:

								irq.unit.LoadState( state );
								break;
						}

						state.End();
					}
				}
			}

			void Mmc3::SubSave(State::Saver& state) const
			{
				state.Begin( AsciiId<'M','M','3'>::V );

				const byte data[12] =
				{
					regs.ctrl0,
					regs.ctrl1,
					banks.prg[0],
					banks.prg[1],
					0x3E,
					0x3F,
					banks.chr[0] >> 1,
					banks.chr[2] >> 1,
					banks.chr[4],
					banks.chr[5],
					banks.chr[6],
					banks.chr[7]
				};

				state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();

				irq.unit.SaveState( state, AsciiId<'I','R','Q'>::V );

				state.End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			NES_POKE_D(Mmc3,8000)
			{
				const uint diff = regs.ctrl0 ^ data;
				regs.ctrl0 = data;

				if (diff & 0x40)
				{
					const uint v[2] =
					{
						banks.prg[(data >> 5 & 0x2) ^ 0],
						banks.prg[(data >> 5 & 0x2) ^ 2]
					};

					UpdatePrg( 0x0000, v[0] );
					UpdatePrg( 0x4000, v[1] );
				}

				if (diff & 0x80)
					UpdateChr();
			}

			NES_POKE_D(Mmc3,8001)
			{
				uint address = regs.ctrl0 & 0x7;

				if (address < 6)
				{
					ppu.Update();

					uint base = regs.ctrl0 << 5 & 0x1000;

					if (address < 2)
					{
						address <<= 1;
						base |= address << 10;
						UpdateChr( base | 0x0000, (banks.chr[address+0] = data & 0xFE) );
						UpdateChr( base | 0x0400, (banks.chr[address+1] = data | 0x01) );
					}
					else
					{
						UpdateChr( (base ^ 0x1000) | (address-2) << 10, (banks.chr[address+2] = data) );
					}
				}
				else
				{
					UpdatePrg( (address == 6) ? (regs.ctrl0 << 8 & 0x4000) : 0x2000, (banks.prg[address-6] = data & 0x3F) );
				}
			}

			NES_POKE_D(Mmc3,A001)
			{
				regs.ctrl1 = data;

				wrk.Source().SetSecurity
				(
					(data & CTRL1_WRAM_ENABLED),
					(data & CTRL1_WRAM) == CTRL1_WRAM_ENABLED && board.GetWram()
				);
			}

			NES_POKE_D(Mmc3,C000)
			{
				irq.Update();
				irq.unit.SetLatch( data );
			}

			NES_POKE(Mmc3,C001)
			{
				irq.Update();
				irq.unit.Reload();
			}

			NES_POKE(Mmc3,E000)
			{
				irq.Update();
				irq.unit.Disable( cpu );
			}

			NES_POKE(Mmc3,E001)
			{
				irq.Update();
				irq.unit.Enable();
			}

			void NST_FASTCALL Mmc3::UpdatePrg(uint address,uint bank)
			{
				prg.SwapBank<SIZE_8K>( address, bank );
			}

			void NST_FASTCALL Mmc3::UpdateChr(uint address,uint bank) const
			{
				chr.SwapBank<SIZE_1K>( address, bank );
			}

			void Mmc3::UpdatePrg()
			{
				const uint x = regs.ctrl0 >> 5 & 0x2;

				UpdatePrg( 0x0000, banks.prg[0^x] );
				UpdatePrg( 0x2000, banks.prg[1^0] );
				UpdatePrg( 0x4000, banks.prg[2^x] );
				UpdatePrg( 0x6000, banks.prg[3^0] );
			}

			void Mmc3::UpdateChr() const
			{
				ppu.Update();

				const uint x = regs.ctrl0 >> 5 & 0x4;

				for (uint i=0; i < 8; ++i)
					UpdateChr( i * SIZE_1K, banks.chr[i^x] );
			}

			void Mmc3::Sync(Event event,Input::Controllers* controllers)
			{
				if (event == EVENT_END_FRAME)
					irq.VSync();

				Board::Sync( event, controllers );
			}
		}
	}
}
