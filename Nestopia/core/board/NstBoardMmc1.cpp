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
#include "NstBoardMmc1.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("s", on)
			#endif

			Mmc1::Mmc1(const Context& c,Revision rev)
			: Board(c), revision(rev)
			{
				switch (rev)
				{
					case REV_A:  Log::Flush( "Board: MMC rev. A"  NST_LINEBREAK ); break;
					case REV_B1: Log::Flush( "Board: MMC rev. B1" NST_LINEBREAK ); break;
					case REV_B2: Log::Flush( "Board: MMC rev. B2" NST_LINEBREAK ); break;
					case REV_B3: Log::Flush( "Board: MMC rev. B3" NST_LINEBREAK ); break;
				}
			}

			void Mmc1::ResetRegisters()
			{
				serial.buffer = 0;
				serial.shifter = 0;

				regs[CTRL] = CTRL_HARD_RESET;
				regs[CHR0] = 0;
				regs[CHR1] = 0;
				regs[PRG0] = (revision == REV_B3 ? PRG0_WRAM_DISABLED : 0);
			}

			void Mmc1::SubReset(const bool hard)
			{
				Map( 0x8000U, 0xFFFFU, &Mmc1::Poke_8000 );

				serial.ready = cpu.GetClock(Serial::RESET_CYCLES);

				if (hard)
				{
					ResetRegisters();

					for (uint i=0; i < 4; ++i)
						UpdateRegisters( i );
				}
			}

			void Mmc1::SubLoad(State::Loader& state,const dword baseChunk)
			{
				NST_VERIFY( baseChunk == (AsciiId<'M','M','1'>::V) );

				serial.ready = 0;

				if (baseChunk == AsciiId<'M','M','1'>::V)
				{
					while (const dword chunk = state.Begin())
					{
						if (chunk == AsciiId<'R','E','G'>::V)
						{
							State::Loader::Data<4+2> data( state );

							for (uint i=0; i < 4; ++i)
								regs[i] = data[i] & 0x1F;

							serial.buffer = data[4] & 0x1F;
							serial.shifter = NST_MIN(data[5],5);
						}

						state.End();
					}
				}
			}

			void Mmc1::SubSave(State::Saver& state) const
			{
				const byte data[4+2] =
				{
					regs[0],
					regs[1],
					regs[2],
					regs[3],
					serial.buffer,
					serial.shifter
				};

				state.Begin( AsciiId<'M','M','1'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
			}

			#ifdef NST_MSVC_OPTIMIZE
			#pragma optimize("", on)
			#endif

			void Mmc1::UpdatePrg()
			{
				prg.SwapBanks<SIZE_16K,0x0000>
				(
					(regs[CHR0] & 0x10U) | ((regs[PRG0] | 0x0U) & ((regs[CTRL] & uint(CTRL_PRG_SWAP_16K)) ? (regs[CTRL] & uint(CTRL_PRG_SWAP_LOW)) ? 0xF : 0x0 : 0xE)),
					(regs[CHR0] & 0x10U) | ((regs[PRG0] & 0xFU) | ((regs[CTRL] & uint(CTRL_PRG_SWAP_16K)) ? (regs[CTRL] & uint(CTRL_PRG_SWAP_LOW)) ? 0xF : 0x0 : 0x1))
				);
			}

			void Mmc1::UpdateWrk()
			{
				const dword size = board.GetWram();

				if (revision != REV_A)
				{
					const uint enable = ~uint(regs[PRG0]) & PRG0_WRAM_DISABLED;
					wrk.Source().SetSecurity( enable, enable && size );
				}

				if (size >= SIZE_16K)
					wrk.SwapBank<SIZE_8K,0x0000>( regs[CHR0] >> (2 + (size == SIZE_16K)) );
			}

			void Mmc1::UpdateChr() const
			{
				ppu.Update();

				const uint mode = regs[CTRL] >> 4 & 0x1U;

				chr.SwapBanks<SIZE_4K,0x0000>
				(
					regs[CHR0] & (0x1E | mode),
					regs[CHR0+mode] & 0x1FU | (mode^1)
				);
			}

			void Mmc1::UpdateNmt()
			{
				static const byte lut[4][4] =
				{
					{0,0,0,0},
					{1,1,1,1},
					{0,1,0,1},
					{0,0,1,1}
				};

				ppu.SetMirroring( lut[regs[CTRL] & uint(CTRL_MIRRORING)] );
			}

			void Mmc1::UpdateRegisters(const uint index)
			{
				NST_ASSERT( index < 4 );

				if (index != CHR1)
				{
					UpdatePrg();
					UpdateWrk();
				}

				if (index != PRG0)
				{
					if (index == CTRL)
						UpdateNmt();

					UpdateChr();
				}
			}

			NES_POKE_AD(Mmc1,8000)
			{
				if (cpu.GetCycles() >= serial.ready)
				{
					if (!(data & Serial::RESET_BIT))
					{
						serial.buffer |= (data & 0x1) << serial.shifter++;

						if (serial.shifter != 5)
							return;

						serial.shifter = 0;
						data = serial.buffer;
						serial.buffer = 0;

						address = address >> 13 & 0x3;

						if (regs[address] != data)
						{
							regs[address] = data;
							UpdateRegisters( address );
						}
					}
					else
					{
						serial.ready = cpu.GetCycles() + cpu.GetClock(Serial::RESET_CYCLES);
						serial.buffer = 0;
						serial.shifter = 0;

						if ((regs[CTRL] & uint(CTRL_WRITE_RESET)) != CTRL_WRITE_RESET)
						{
							regs[CTRL] |= uint(CTRL_WRITE_RESET);
							UpdateRegisters( CTRL );
						}
					}
				}
				else
				{
					// looks like there's some protection from rapid writes on register
					// reset, otherwise games like 'AD&D Hillsfar' and 'Bill & Ted' will break

					NST_DEBUG_MSG("MMC1 PRG write ignored!");
				}
			}

			void Mmc1::Sync(Event event,Input::Controllers* controllers)
			{
				if (event == EVENT_END_FRAME)
				{
					if (serial.ready <= cpu.GetFrameCycles())
						serial.ready = 0;
					else
						serial.ready -= cpu.GetFrameCycles();
				}

				Board::Sync( event, controllers );
			}
		}
	}
}
