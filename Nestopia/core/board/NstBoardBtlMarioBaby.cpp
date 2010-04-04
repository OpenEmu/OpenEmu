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
#include "NstBoardBtlMarioBaby.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Btl
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				MarioBaby::MarioBaby(const Context& c)
				: Board(c), irq(*c.cpu,*c.cpu) {}

				void MarioBaby::Irq::Reset(const bool hard)
				{
					if (hard)
						count = 0;
				}

				void MarioBaby::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x7FFFU, &MarioBaby::Peek_6000 );

					for (uint i=0x0000; i < 0x2000; i += 0x4)
					{
						Map( 0x8000 + i, CHR_SWAP_8K          );
						Map( 0xE000 + i, &MarioBaby::Poke_E000 );
						Map( 0xE001 + i, &MarioBaby::Poke_E001 );
						Map( 0xE002 + i, &MarioBaby::Poke_E002 );
					}

					irq.Reset( hard, hard ? false : irq.Connected() );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(~0U);
				}

				void MarioBaby::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','M','B'>::V) );

					if (baseChunk == AsciiId<'B','M','B'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
							{
								State::Loader::Data<3> data( state );

								irq.Connect( data[0] & 0x1 );
								irq.unit.count = data[1] | (data[2] << 8 & 0x7F00);
							}

							state.End();
						}
					}
				}

				void MarioBaby::SubSave(State::Saver& state) const
				{
					const byte data[3] =
					{
						irq.Connected() ? 0x1 : 0x0,
						irq.unit.count >> 0 & 0xFF,
						irq.unit.count >> 8 & 0x7F
					};

					state.Begin( AsciiId<'B','M','B'>::V ).Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(MarioBaby,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_POKE_D(MarioBaby,E000)
				{
					wrk.SwapBank<SIZE_8K,0x0000>(data & 0xF);
				}

				NES_POKE_D(MarioBaby,E001)
				{
					ppu.SetMirroring( (data & 0x8) ? Ppu::NMT_H : Ppu::NMT_V );
				}

				NES_POKE_D(MarioBaby,E002)
				{
					irq.Update();

					if (!irq.Connect( data & 0x2 ))
					{
						irq.unit.count = 0;
						irq.ClearIRQ();
					}
				}

				bool MarioBaby::Irq::Clock()
				{
					const uint prev = count++;

					if ((count & 0x6000) != (prev & 0x6000))
					{
						if ((count & 0x6000) == 0x6000)
							return true;
						else
							cpu.ClearIRQ();
					}

					return false;
				}

				void MarioBaby::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
