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
#include "NstBoardTaitoTc0190fmcPal16r4.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Taito
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Tc0190fmcPal16r4::Tc0190fmcPal16r4(const Context& c)
				: Tc0190fmc(c), irq(*c.cpu,*c.ppu,false) {}

				void Tc0190fmcPal16r4::SubReset(const bool hard)
				{
					Tc0190fmc::SubReset( hard );

					irq.Reset( hard );

					for (uint i=0x0000; i < 0x1000; i += 0x4)
					{
						Map( 0x8000 + i, PRG_SWAP_8K_0 );
						Map( 0xC000 + i, &Tc0190fmcPal16r4::Poke_C000 );
						Map( 0xC001 + i, &Tc0190fmcPal16r4::Poke_C001 );
						Map( 0xC002 + i, &Tc0190fmcPal16r4::Poke_C002 );
						Map( 0xC003 + i, &Tc0190fmcPal16r4::Poke_C003 );
						Map( 0xE000 + i, &Tc0190fmcPal16r4::Poke_E000 );
					}
				}

				void Tc0190fmcPal16r4::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'T','T','C'>::V) );

					if (baseChunk == AsciiId<'T','T','C'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
								irq.unit.LoadState( state );

							state.End();
						}
					}
				}

				void Tc0190fmcPal16r4::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'T','T','C'>::V );
					irq.unit.SaveState( state, AsciiId<'I','R','Q'>::V );
					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Tc0190fmcPal16r4,C000)
				{
					irq.Update();
					irq.unit.SetLatch( (0x100 - data) & 0xFF );
				}

				NES_POKE(Tc0190fmcPal16r4,C001)
				{
					irq.Update();
					irq.unit.Reload();
				}

				NES_POKE(Tc0190fmcPal16r4,C002)
				{
					irq.Update();
					irq.unit.Enable();
				}

				NES_POKE(Tc0190fmcPal16r4,C003)
				{
					irq.Update();
					irq.unit.Disable( cpu );
				}

				NES_POKE_D(Tc0190fmcPal16r4,E000)
				{
					ppu.SetMirroring( (data & 0x40) ? Ppu::NMT_H : Ppu::NMT_V );
				}

				void Tc0190fmcPal16r4::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Tc0190fmc::Sync( event, controllers );
				}
			}
		}
	}
}
