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
#include "NstBoardUnlTf1201.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Unlicensed
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Tf1201::Tf1201(const Context& c)
				: Board(c), irq(*c.cpu,*c.ppu) {}

				void Tf1201::Irq::Reset(bool)
				{
					enabled = false;
					count = 0;
				}

				void Tf1201::SubReset(const bool hard)
				{
					irq.Reset( true );

					if (hard)
						prgSelect = 0;

					for (uint i=0x0000; i < 0x1000; i += 0x4)
					{
						Map( 0x8000 + i, &Tf1201::Poke_8000 );
						Map( 0x9000 + i, NMT_SWAP_HV        );
						Map( 0x9001 + i, &Tf1201::Poke_9001 );
						Map( 0xA000 + i, PRG_SWAP_8K_1      );
						Map( 0xF000 + i, &Tf1201::Poke_F000 );
						Map( 0xF001 + i, &Tf1201::Poke_F001 );
						Map( 0xF002 + i, &Tf1201::Poke_F002 );
						Map( 0xF003 + i, &Tf1201::Poke_F001 );
					}

					for (uint i=0x0000; i < 0x3004; i += 0x4)
					{
						Map( 0xB000 + i, 0xB001 + i, &Tf1201::Poke_B000 );
						Map( 0xB002 + i, 0xB003 + i, &Tf1201::Poke_B002 );
					}
				}

				void Tf1201::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'U','T','2'>::V) );

					if (baseChunk == AsciiId<'U','T','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
							case AsciiId<'R','E','G'>::V:

								prgSelect = state.Read8();
								break;

							case AsciiId<'I','R','Q'>::V:
								{
									State::Loader::Data<2> data( state );

									irq.unit.enabled = data[0] & 0x1;
									irq.unit.count = data[2];
									break;
								}
							}

							state.End();
						}
					}
				}

				void Tf1201::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'U','T','2'>::V );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( prgSelect ).End();

					const byte data[2] =
					{
						irq.unit.enabled ? 0x1 : 0x0,
						irq.unit.count & 0xFF
					};

					state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void Tf1201::UpdatePrg(uint bank)
				{
					prg.SwapBank<SIZE_8K,0x0000>( (prgSelect & 0x2) ? ~1U : bank );
					prg.SwapBank<SIZE_8K,0x4000>( (prgSelect & 0x2) ? bank : ~1U );
				}

				NES_POKE_D(Tf1201,8000)
				{
					UpdatePrg( data );
				}

				NES_POKE_D(Tf1201,9001)
				{
					prgSelect = data;
					UpdatePrg( prg.GetBank<SIZE_8K,0x0000>() );
				}

				NES_POKE_AD(Tf1201,B000)
				{
					ppu.Update();
					address = (((address >> 11) - 6) | (address & 0x1)) << 10 & 0x1FFF;
					chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & 0xF0) | (data << 0 & 0x0F) );
				}

				NES_POKE_AD(Tf1201,B002)
				{
					ppu.Update();
					address = (((address >> 11) - 6) | (address & 0x1)) << 10 & 0x1FFF;
					chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & 0x0F) | (data << 4 & 0xF0) );
				}

				NES_POKE_D(Tf1201,F000)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0xF0) | (data << 0 & 0x0F);
				}

				NES_POKE_D(Tf1201,F002)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0x0F) | (data << 4 & 0xF0);
				}

				NES_POKE_D(Tf1201,F001)
				{
					irq.Update();
					irq.unit.enabled = data & 0x2;
					irq.ClearIRQ();

					ppu.Update();

					if (ppu.GetScanline() < 240)
						irq.unit.count -= 8;
				}

				bool Tf1201::Irq::Clock()
				{
					return enabled && (++count & 0xFF) == 238;
				}

				void Tf1201::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
