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
#include "NstBoardBtlSmb2a.hpp"

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

				Smb2a::Smb2a(const Context& c)
				: Board(c), irq(*c.cpu) {}

				void Smb2a::SubReset(const bool hard)
				{
					if (hard)
					{
						wrk.SwapBank<SIZE_8K,0x0000>( 6 );
						prg.SwapBanks<SIZE_8K,0x0000>( 4, 5, 0, 7 );
					}

					irq.Reset( hard, true );

					Map( 0x6000U, 0x7FFFU, &Smb2a::Peek_6000 );
					Map( 0x8000U, 0x9FFFU, &Smb2a::Poke_8000 );
					Map( 0xA000U, 0xBFFFU, &Smb2a::Poke_A000 );
					Map( 0xE000U, 0xFFFFU, PRG_SWAP_8K_2     );
				}

				void Smb2a::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'B','2','A'>::V) );

					if (baseChunk == AsciiId<'B','2','A'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
							{
								State::Loader::Data<3> data( state );

								irq.unit.enabled = data[0] & 0x1U;
								irq.unit.count = data[1] | (data[2] << 8 & 0xF00);
							}

							state.End();
						}
					}
				}

				void Smb2a::SubSave(State::Saver& state) const
				{
					const byte data[3] =
					{
						irq.unit.enabled != 0,
						irq.unit.count & 0xFF,
						irq.unit.count >> 8
					};

					state.Begin( AsciiId<'B','2','A'>::V ).Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void Smb2a::Irq::Reset(const bool hard)
				{
					if (hard)
					{
						enabled = false;
						count = 0;
					}
				}

				bool Smb2a::Irq::Clock()
				{
					if (enabled)
					{
						count = (count + 1) & 0xFFF;

						if (!count)
						{
							enabled = false;
							return true;
						}
					}

					return false;
				}

				NES_PEEK_A(Smb2a,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_POKE(Smb2a,8000)
				{
					irq.Update();
					irq.unit.enabled = false;
					irq.unit.count = 0;
					irq.ClearIRQ();
				}

				NES_POKE(Smb2a,A000)
				{
					irq.Update();
					irq.unit.enabled = true;
				}

				void Smb2a::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
