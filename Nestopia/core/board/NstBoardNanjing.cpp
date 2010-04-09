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
#include "NstBoardNanjing.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Nanjing
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Standard::SubReset(bool)
				{
					strobe = 0xFF;
					regs[0] = 0xFF;
					regs[1] = 0x00;
					trigger = 0x00;
					security = 0x00;

					ppu.SetHBlankHook( Hook(this,&Standard::Hook_HBlank) );

					for (uint i=0x5000; i < 0x6000; i += 0x800)
					{
						Map( i + 0x000, i + 0x0FF, &Standard::Peek_5000 );
						Map( i + 0x100, i + 0x1FF, &Standard::Peek_5100 );
						Map( i + 0x200, i + 0x4FF, &Standard::Peek_5000 );
						Map( i + 0x500, i + 0x5FF, &Standard::Peek_5500 );
						Map( i + 0x600, i + 0x7FF, &Standard::Peek_5000 );
					}

					Map( 0x5100U, &Standard::Poke_5100 );
					Map( 0x5101U, &Standard::Poke_5101 );

					for (uint i=0x5000; i < 0x6000; i += 0x400)
					{
						Map( i + 0x000, i + 0x0FF, &Standard::Poke_5000 );
						Map( i + 0x200, i + 0x2FF, &Standard::Poke_5000 );
						Map( i + 0x300, i + 0x3FF, &Standard::Poke_5300 );
					}
				}

				void Standard::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'N','J','N'>::V) );

					if (baseChunk == AsciiId<'N','J','N'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:
								{
									State::Loader::Data<2> data( state );

									regs[0] = data[0];
									regs[1] = data[1];
									break;
								}

								case AsciiId<'S','E','C'>::V:
								{
									State::Loader::Data<3> data( state );

									strobe = data[0];
									trigger = (data[1] & 0x1) ? 0xFF : 0x00;
									security = data[2];
									break;
								}
							}

							state.End();
						}
					}
				}

				void Standard::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'N','J','N'>::V );

					{
						const byte data[2] = { regs[0], regs[1] };
						state.Begin( AsciiId<'R','E','G'>::V ).Write( data ).End();
					}

					{
						const byte data[3] = { strobe, trigger ? 0x1 : 0x0, security };
						state.Begin( AsciiId<'S','E','C'>::V ).Write( data ).End();
					}

					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_PEEK(Standard,5000)
				{
					return 0x4;
				}

				NES_POKE_AD(Standard,5000)
				{
					regs[address >> 9 & 0x1] = data;
					prg.SwapBank<SIZE_32K,0x0000>( (regs[0] & 0xFU) | (regs[1] << 4) );

					if (!((address & 0x0300) | (regs[0] & 0x80U)))
					{
						ppu.Update();

						if (ppu.GetScanline() <= 127)
							chr.SwapBank<SIZE_8K,0x0000>(0);
					}
				}

				NES_PEEK(Standard,5100)
				{
					return security;
				}

				NES_POKE_D(Standard,5100)
				{
					if (data == 0x6)
						prg.SwapBank<SIZE_32K,0x0000>( 0x3 );
				}

				NES_POKE_D(Standard,5101)
				{
					const uint address = strobe;
					strobe = data;

					if (address && !data)
						trigger ^= 0xFFU;
				}

				NES_POKE_D(Standard,5300)
				{
					security = data;
				}

				NES_PEEK(Standard,5500)
				{
					return security & trigger;
				}

				NES_HOOK(Standard,HBlank)
				{
					if ((regs[0] & 0x80U) && ppu.IsEnabled())
					{
						switch (const int scanline=ppu.GetScanline())
						{
							case 127:
							case 239:

								chr.SwapBanks<SIZE_4K,0x0000>( scanline == 127, scanline == 127 );
								break;
						}
					}
				}
			}
		}
	}
}
