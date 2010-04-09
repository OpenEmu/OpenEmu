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
#include "NstBoardKonamiVrc2.hpp"

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

				uint Vrc2::GetPrgLineShift(const Context& c,const uint pin,const uint def)
				{
					if (const Chips::Type* const vrc2 = c.chips.Find(L"Konami VRC II"))
					{
						const uint line = *vrc2->Pin(pin).C(L"PRG").A();
						NST_VERIFY( line < 8 );

						if (line < 8)
							return line;
					}

					return def;
				}

				uint Vrc2::GetChrLineShift(const Context& c)
				{
					if (const Chips::Type* const vrc2 = c.chips.Find(L"Konami VRC II"))
					{
						if (vrc2->Pin(21).C(L"CHR").A() != 10)
							return 1;
					}

					return 0;
				}

				Vrc2::Vrc2(const Context& c)
				:
				Board    (c),
				chrShift (GetChrLineShift(c)),
				prgLineA (GetPrgLineShift(c,3,1)),
				prgLineB (GetPrgLineShift(c,4,0))
				{
				}

				void Vrc2::SubReset(const bool hard)
				{
					if (hard)
						security = 0;

					if (!board.GetWram())
						Map( 0x6000U, &Vrc2::Peek_6000, &Vrc2::Poke_6000 );

					Map( 0x8000U, 0x8FFFU, PRG_SWAP_8K_0 );
					Map( 0x9000U, 0x9FFFU, NMT_SWAP_VH01 );
					Map( 0xA000U, 0xAFFFU, PRG_SWAP_8K_1 );

					for (uint i=0xB000, a=9-prgLineA, b=8-prgLineB; i < 0xF000; ++i)
					{
						switch ((i & 0xF000) | (i << a & 0x0200) | (i << b & 0x0100))
						{
							case 0xB000: Map( i, &Vrc2::Poke_B000 ); break;
							case 0xB100: Map( i, &Vrc2::Poke_B001 ); break;
							case 0xB200: Map( i, &Vrc2::Poke_B002 ); break;
							case 0xB300: Map( i, &Vrc2::Poke_B003 ); break;
							case 0xC000: Map( i, &Vrc2::Poke_C000 ); break;
							case 0xC100: Map( i, &Vrc2::Poke_C001 ); break;
							case 0xC200: Map( i, &Vrc2::Poke_C002 ); break;
							case 0xC300: Map( i, &Vrc2::Poke_C003 ); break;
							case 0xD000: Map( i, &Vrc2::Poke_D000 ); break;
							case 0xD100: Map( i, &Vrc2::Poke_D001 ); break;
							case 0xD200: Map( i, &Vrc2::Poke_D002 ); break;
							case 0xD300: Map( i, &Vrc2::Poke_D003 ); break;
							case 0xE000: Map( i, &Vrc2::Poke_E000 ); break;
							case 0xE100: Map( i, &Vrc2::Poke_E001 ); break;
							case 0xE200: Map( i, &Vrc2::Poke_E002 ); break;
							case 0xE300: Map( i, &Vrc2::Poke_E003 ); break;
						}
					}
				}

				void Vrc2::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'K','V','2'>::V) );

					if (baseChunk == AsciiId<'K','V','2'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'S','E','C'>::V)
							{
								NST_VERIFY( !board.GetWram() );
								security = state.Read8() & 0x1;
							}

							state.End();
						}
					}
				}

				void Vrc2::SubSave(State::Saver& state) const
				{
					if (!board.GetWram())
						state.Begin( AsciiId<'K','V','2'>::V ).Begin( AsciiId<'S','E','C'>::V ).Write8( security ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Vrc2,6000)
				{
					NST_DEBUG_MSG( "VRC security write" );
					security = data & 0x1;
				}

				NES_PEEK(Vrc2,6000)
				{
					NST_DEBUG_MSG( "VRC security read" );
					return security;
				}

				template<uint OFFSET>
				void Vrc2::SwapChr(uint address,uint subBank) const
				{
					ppu.Update();
					chr.SwapBank<SIZE_1K>( address, (chr.GetBank<SIZE_1K>(address) & 0xF0U >> OFFSET) | ((subBank >> chrShift & 0xF) << OFFSET) );
				}

				NES_POKE_D(Vrc2,B000) { SwapChr<0>( 0x0000, data ); }
				NES_POKE_D(Vrc2,B001) { SwapChr<4>( 0x0000, data ); }
				NES_POKE_D(Vrc2,B002) { SwapChr<0>( 0x0400, data ); }
				NES_POKE_D(Vrc2,B003) { SwapChr<4>( 0x0400, data ); }
				NES_POKE_D(Vrc2,C000) { SwapChr<0>( 0x0800, data ); }
				NES_POKE_D(Vrc2,C001) { SwapChr<4>( 0x0800, data ); }
				NES_POKE_D(Vrc2,C002) { SwapChr<0>( 0x0C00, data ); }
				NES_POKE_D(Vrc2,C003) { SwapChr<4>( 0x0C00, data ); }
				NES_POKE_D(Vrc2,D000) { SwapChr<0>( 0x1000, data ); }
				NES_POKE_D(Vrc2,D001) { SwapChr<4>( 0x1000, data ); }
				NES_POKE_D(Vrc2,D002) { SwapChr<0>( 0x1400, data ); }
				NES_POKE_D(Vrc2,D003) { SwapChr<4>( 0x1400, data ); }
				NES_POKE_D(Vrc2,E000) { SwapChr<0>( 0x1800, data ); }
				NES_POKE_D(Vrc2,E001) { SwapChr<4>( 0x1800, data ); }
				NES_POKE_D(Vrc2,E002) { SwapChr<0>( 0x1C00, data ); }
				NES_POKE_D(Vrc2,E003) { SwapChr<4>( 0x1C00, data ); }
			}
		}
	}
}
