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
#include "NstBoardKonamiVrc3.hpp"

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

				Vrc3::Vrc3(const Context& c)
				: Board(c), irq(*c.cpu) {}

				void Vrc3::Irq::Reset(const bool hard)
				{
					if (hard)
					{
						enabled = false;
						count = 0;
					}
				}

				void Vrc3::SubReset(const bool hard)
				{
					irq.Reset( hard, true );

					Map( 0x8000U, 0x8FFFU, &Vrc3::Poke_8000 );
					Map( 0x9000U, 0x9FFFU, &Vrc3::Poke_9000 );
					Map( 0xA000U, 0xAFFFU, &Vrc3::Poke_A000 );
					Map( 0xB000U, 0xBFFFU, &Vrc3::Poke_B000 );
					Map( 0xC000U, 0xCFFFU, &Vrc3::Poke_C000 );
					Map( 0xD000U, 0xDFFFU, &Vrc3::Poke_D000 );
					Map( 0xF000U, 0xFFFFU, PRG_SWAP_16K_0 );
				}

				void Vrc3::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'K','V','3'>::V) );

					if (baseChunk == AsciiId<'K','V','3'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
							{
								State::Loader::Data<3> data( state );
								irq.unit.enabled = data[0] & 0x1;
								irq.unit.count = data[1] | data[2] << 8;
							}

							state.End();
						}
					}
				}

				void Vrc3::SubSave(State::Saver& state) const
				{
					const byte data[3] =
					{
						irq.unit.enabled != 0,
						irq.unit.count & 0xFF,
						irq.unit.count >> 8
					};

					state.Begin( AsciiId<'K','V','3'>::V ).Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				bool Vrc3::Irq::Clock()
				{
					if (enabled)
					{
						count = (count + 1) & 0xFFFF;

						if (!count)
						{
							enabled = false;
							return true;
						}
					}

					return false;
				}

				NES_POKE_D(Vrc3,8000)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0xFFF0) | (data & 0xF) << 0;
				}

				NES_POKE_D(Vrc3,9000)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0xFF0F) | (data & 0xF) << 4;
				}

				NES_POKE_D(Vrc3,A000)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0xF0FF) | (data & 0xF) << 8;
				}

				NES_POKE_D(Vrc3,B000)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0x0FFF) | (data & 0xF) << 12;
				}

				NES_POKE_D(Vrc3,C000)
				{
					irq.Update();
					irq.unit.enabled = data & 0x2;
					irq.ClearIRQ();
				}

				NES_POKE(Vrc3,D000)
				{
					irq.Update();
					irq.unit.enabled = false;
					irq.ClearIRQ();
				}

				void Vrc3::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
