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
#include "NstBoardWaixing.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Waixing
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Ffv::SubReset(bool)
				{
					for (uint i=0x5000; i < 0x6000; i += 0x400)
						Map( i + 0x00, i + 0x1FF, &Ffv::Poke_5000 );

					for (uint i=0; i < 2; ++i)
						regs[i] = 0;

					prg.SwapBank<SIZE_16K,0x4000>(0x1F);
				}

				void Ffv::SubSave(State::Saver& state) const
				{
					const byte data[2] = { regs[0], regs[1] };
					state.Begin( AsciiId<'W','F','V'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				void Ffv::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'W','F','V'>::V) );

					if (baseChunk == AsciiId<'W','F','V'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<2> data( state );

								regs[0] = data[0];
								regs[1] = data[1];
							}

							state.End();
						}
					}
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(Ffv,5000)
				{
					address = address >> 8 & 0x1;

					if (regs[address] != data)
					{
						regs[address] = data;
						data = regs[1] << 5 & 0x20;

						switch (regs[0] & 0x70)
						{
							case 0x00:
							case 0x20:
							case 0x40:
							case 0x60:

								prg.SwapBanks<SIZE_16K,0x0000>( data | (regs[0] >> 1 & 0x10) | (regs[0] & 0xF), data | 0x1F );
								break;

							case 0x50:

								prg.SwapBank<SIZE_32K,0x0000>( (data >> 1) | (regs[0] & 0xF) );
								break;

							case 0x70:

								prg.SwapBanks<SIZE_16K,0x0000>( data | (regs[0] << 1 & 0x10) | (regs[0] & 0xF), data | 0x1F );
								break;
						}
					}
				}
			}
		}
	}
}
