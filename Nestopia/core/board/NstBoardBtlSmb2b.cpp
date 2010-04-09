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
#include "NstBoardBtlSmb2b.hpp"

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

				Smb2b::Smb2b(const Context& c)
				: Board(c), irq(*c.cpu) {}

				void Smb2b::Irq::Reset(const bool hard)
				{
					if (hard)
						count = 0;
				}

				void Smb2b::SubReset(const bool hard)
				{
					if (hard)
					{
						wrk.SwapBank<SIZE_8K,0x0000>( 15 );
						prg.SwapBanks<SIZE_8K,0x0000>( 8, 9, 0, 11 );
					}

					irq.Reset( hard, hard ? false : irq.Connected() );

					for (uint i=0x4020; i < 0x6000; i += 0x80)
						Map( i+0x00, i+0x20, (i & 0x100) ? &Smb2b::Poke_4120 : &Smb2b::Poke_4020 );

					Map( 0x6000U, 0x7FFFU, &Smb2b::Peek_6000 );
				}

				void Smb2b::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'S','2','B'>::V) );

					if (baseChunk == AsciiId<'S','2','B'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
							{
								State::Loader::Data<3> data( state );

								irq.Connect( data[0] & 0x1 );
								irq.unit.count = data[1] | data[2] << 8;
							}

							state.End();
						}
					}
				}

				void Smb2b::SubSave(State::Saver& state) const
				{
					const byte data[3] =
					{
						irq.Connected() ? 0x1 : 0x0,
						irq.unit.count & 0xFF,
						irq.unit.count >> 8
					};

					state.Begin( AsciiId<'S','2','B'>::V ).Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK_A(Smb2b,6000)
				{
					return wrk[0][address - 0x6000];
				}

				NES_POKE_D(Smb2b,4020)
				{
					prg.SwapBank<SIZE_8K,0x4000>
					(
						(data << 0 & 0x8) |
						(data << 2 & 0x4) |
						(data >> 1 & 0x3)
					);
				}

				NES_POKE_D(Smb2b,4120)
				{
					irq.Update();
					irq.Connect( data & 0x1 );
					irq.ClearIRQ();
				}

				bool Smb2b::Irq::Clock()
				{
					return ++count == 0x1000;
				}

				void Smb2b::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
					{
						irq.unit.count = 0;
						irq.VSync();
					}

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
