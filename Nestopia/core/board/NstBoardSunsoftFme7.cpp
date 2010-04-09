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
#include "../NstClock.hpp"
#include "NstBoardSunsoftFme7.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Sunsoft
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Fme7::Fme7(const Context& c)
				:
				Board  (c),
				irq    (*c.cpu)
				{}

				void Fme7::Irq::Reset(const bool hard)
				{
					if (hard)
					{
						enabled = false;
						count = 0;
					}
				}

				void Fme7::SubReset(const bool hard)
				{
					if (hard)
						command = 0x0;

					irq.Reset( hard, hard ? false : irq.Connected() );

					Map( 0x6000U, 0x7FFFU, &Fme7::Peek_6000 );
					Map( 0x8000U, 0x9FFFU, &Fme7::Poke_8000 );
					Map( 0xA000U, 0xBFFFU, &Fme7::Poke_A000 );
				}

				void Fme7::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'S','F','7'>::V) );

					if (baseChunk == AsciiId<'S','F','7'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:

									command = state.Read8();
									break;

								case AsciiId<'I','R','Q'>::V:
								{
									State::Loader::Data<3> data( state );

									irq.Connect( data[0] & 0x80 );
									irq.unit.enabled = data[0] & 0x01;
									irq.unit.count = data[1] | data[2] << 8;

									break;
								}
							}

							state.End();
						}
					}
				}

				void Fme7::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'S','F','7'>::V );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( command ).End();

					{
						const byte data[3] =
						{
							(irq.Connected() ? 0x80U : 0x00U) | (irq.unit.enabled ? 0x1U : 0x0U),
							irq.unit.count & 0xFF,
							irq.unit.count >> 8
						};

						state.Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End();
					}

					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(Fme7,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_POKE_D(Fme7,8000)
				{
					command = data;
				}

				NES_POKE_D(Fme7,A000)
				{
					switch (const uint bank = (command & 0xF))
					{
						case 0x0:
						case 0x1:
						case 0x2:
						case 0x3:
						case 0x4:
						case 0x5:
						case 0x6:
						case 0x7:

							ppu.Update();
							chr.SwapBank<SIZE_1K>( bank << 10, data );
							break;

						case 0x8:

							if (!(data & 0x40) || (data & 0x80))
								wrk.Source( !(data & 0x40) ).SwapBank<SIZE_8K,0x0000>( data );

							break;

						case 0x9:
						case 0xA:
						case 0xB:

							prg.SwapBank<SIZE_8K>( (command - 0x9) << 13, data );
							break;

						case 0xC:

							SetMirroringVH01( data );
							break;

						case 0xD:

							irq.Update();
							irq.unit.enabled = data & 0x01;

							if (!irq.Connect( data & 0x80 ))
								irq.ClearIRQ();

							break;

						case 0xE:

							irq.Update();
							irq.unit.count = (irq.unit.count & 0xFF00) | data << 0;
							break;

						case 0xF:

							irq.Update();
							irq.unit.count = (irq.unit.count & 0x00FF) | data << 8;
							break;
					}
				}

				bool Fme7::Irq::Clock()
				{
					count = (count - 1U) & 0xFFFF;
					return count < enabled;
				}

				void Fme7::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
