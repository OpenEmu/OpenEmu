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
#include "NstBoardBtlSmb3.hpp"

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

				Smb3::Smb3(const Context& c)
				: Board(c), irq(*c.cpu) {}

				void Smb3::Irq::Reset(const bool hard)
				{
					if (hard)
					{
						enabled = false;
						count = 0;
					}
				}

				void Smb3::SubReset(const bool hard)
				{
					irq.Reset( hard, true );

					for (uint i=0x0000; i < 0x8000; i += 0x10)
					{
						Map( i + 0x8000U,              &Smb3::Poke_8000 );
						Map( i + 0x8001U,              &Smb3::Poke_8001 );
						Map( i + 0x8002U,              &Smb3::Poke_8000 );
						Map( i + 0x8003U,              &Smb3::Poke_8001 );
						Map( i + 0x8004U, i + 0x8007U, &Smb3::Poke_8004 );
						Map( i + 0x8008U, i + 0x800BU, &Smb3::Poke_8008 );
						Map( i + 0x800CU,              &Smb3::Poke_800C );
						Map( i + 0x800DU,              &Smb3::Poke_800D );
						Map( i + 0x800EU,              &Smb3::Poke_800E );
						Map( i + 0x800FU,              &Smb3::Poke_800F );
					}
				}

				void Smb3::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( (baseChunk == AsciiId<'B','S','3'>::V) );

					if (baseChunk == AsciiId<'B','S','3'>::V)
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

				void Smb3::SubSave(State::Saver& state) const
				{
					const byte data[3] =
					{
						irq.unit.enabled ? 0x1 : 0x0,
						irq.unit.count & 0xFF,
						irq.unit.count >> 8
					};

					state.Begin( AsciiId<'B','S','3'>::V ).Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void Smb3::UpdateChr(uint address,uint data) const
				{
					ppu.Update();
					chr.SwapBank<SIZE_1K>( address << 10 & 0x1C00, data );
				}

				NES_POKE_AD(Smb3,8000)
				{
					UpdateChr( address, data & 0xFE );
				}

				NES_POKE_AD(Smb3,8001)
				{
					UpdateChr( address, data | 0x01 );
				}

				NES_POKE_AD(Smb3,8004)
				{
					UpdateChr( address, data );
				}

				NES_POKE_AD(Smb3,8008)
				{
					address = address << 13 & 0x6000;
					prg.SwapBank<SIZE_8K>( address, data | (address == 0x0000 || address == 0x6000 ? 0x10 : 0x00) );
				}

				NES_POKE_D(Smb3,800C)
				{
					ppu.SetMirroring( (data & 0x1) ? Ppu::NMT_H : Ppu::NMT_V );
				}

				NES_POKE_D(Smb3,800D)
				{
					irq.Update();
					irq.unit.count = 0;
					irq.unit.enabled = false;
					irq.ClearIRQ();
				}

				NES_POKE_D(Smb3,800E)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0xFF00) | (data << 0);
				}

				NES_POKE_D(Smb3,800F)
				{
					irq.Update();
					irq.unit.count = (irq.unit.count & 0x00FF) | (data << 8);
					irq.unit.enabled = true;
				}

				bool Smb3::Irq::Clock()
				{
					return enabled && (count = (count + 1) & 0xFFFF) == 0x0000 ? (enabled=false, true) : false;
				}

				void Smb3::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
