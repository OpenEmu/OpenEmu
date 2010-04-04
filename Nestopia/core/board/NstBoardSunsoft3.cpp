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
#include "NstBoardSunsoft3.hpp"

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

				S3::S3(const Context& c)
				:
				Board (c),
				irq   (*c.cpu)
				{}

				void S3::Irq::Reset(const bool hard)
				{
					if (hard)
					{
						enabled = false;
						count = 0;
						toggle = 0;
					}
				}

				void S3::SubReset(const bool hard)
				{
					irq.Reset( hard, true );

					Map( 0x8800U, 0x8FFFU, CHR_SWAP_2K_0  );
					Map( 0x9800U, 0x9FFFU, CHR_SWAP_2K_1  );
					Map( 0xA800U, 0xAFFFU, CHR_SWAP_2K_2  );
					Map( 0xB800U, 0xBFFFU, CHR_SWAP_2K_3  );
					Map( 0xC000U, 0xCFFFU, &S3::Poke_C000 );
					Map( 0xD800U, 0xDFFFU, &S3::Poke_D800 );
					Map( 0xE800U, 0xEFFFU, NMT_SWAP_VH01  );
					Map( 0xF800U, 0xFFFFU, PRG_SWAP_16K_0 );
				}

				void S3::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'S','3'>::V) );

					if (baseChunk == AsciiId<'S','3'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'I','R','Q'>::V)
							{
								State::Loader::Data<3> data( state );

								irq.unit.enabled = data[0] & 0x1;
								irq.unit.toggle = data[0] >> 1 & 0x1;
								irq.unit.count = data[1] | data[2] << 8;
							}

							state.End();
						}
					}
				}

				void S3::SubSave(State::Saver& state) const
				{
					const byte data[3] =
					{
						(irq.unit.enabled ? 0x1U : 0x0U) | (irq.unit.toggle ? 0x2U : 0x0U),
						irq.unit.count & 0xFF,
						irq.unit.count >> 8
					};

					state.Begin( AsciiId<'S','3'>::V ).Begin( AsciiId<'I','R','Q'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(S3,C000)
				{
					irq.Update();

					if (irq.unit.toggle ^= 1)
						irq.unit.count = (irq.unit.count & 0x00FF) | data << 8;
					else
						irq.unit.count = (irq.unit.count & 0xFF00) | data << 0;
				}

				NES_POKE_D(S3,D800)
				{
					irq.Update();

					irq.unit.toggle = 0;
					irq.unit.enabled = data & 0x10;
					irq.ClearIRQ();
				}

				bool S3::Irq::Clock()
				{
					if (enabled && count && !--count)
					{
						enabled = false;
						count = 0xFFFF;
						return true;
					}

					return false;
				}

				void S3::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_END_FRAME)
						irq.VSync();

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
