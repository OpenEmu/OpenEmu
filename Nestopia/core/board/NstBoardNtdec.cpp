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
#include "NstBoardNtdec.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Ntdec
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Asder::SubReset(const bool hard)
				{
					if (hard)
					{
						command = 0;

						for (uint i=0; i < 8; ++i)
							banks.chr[i] = 0;
					}

					for (uint i=0x0000; i < 0x2000; i += 0x2)
					{
						Map( 0x8000U + i, &Asder::Poke_8000 );
						Map( 0xA000U + i, &Asder::Poke_A000 );
						Map( 0xC000U + i, &Asder::Poke_C000 );
						Map( 0xE000U + i, &Asder::Poke_E000 );
					}
				}

				void Asder::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'N','A','S'>::V) );

					if (baseChunk == AsciiId<'N','A','S'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
							case AsciiId<'R','E','G'>::V:

								command = state.Read8();
								break;

							case AsciiId<'B','N','K'>::V:

								state.Read( banks.chr );
								break;
							}

							state.End();
						}
					}
				}

				void Asder::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'N','A','S'>::V );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( command ).End();
					state.Begin( AsciiId<'B','N','K'>::V ).Write( banks.chr ).End();
					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void Asder::UpdateChr() const
				{
					ppu.Update();

					const uint extChr = (banks.chr[1] & 0x2U) ? banks.chr[0] : 0;

					chr.SwapBanks<SIZE_2K,0x0000>
					(
						banks.chr[2] | (extChr << 5 & 0x80),
						banks.chr[3] | (extChr << 4 & 0x80)
					);

					chr.SwapBanks<SIZE_1K,0x1000>
					(
						banks.chr[4] | (extChr << 4 & 0x100),
						banks.chr[5] | (extChr << 3 & 0x100),
						banks.chr[6] | (extChr << 2 & 0x100),
						banks.chr[7] | (extChr << 1 & 0x100)
					);
				}

				NES_POKE_D(Asder,8000)
				{
					command = data;
				}

				NES_POKE_D(Asder,A000)
				{
					const uint address = command & 0x7;

					if (address < 2)
					{
						prg.SwapBank<SIZE_8K>( address << 13, data );
					}
					else
					{
						banks.chr[address] = data >> uint(address < 4);
						UpdateChr();
					}
				}

				NES_POKE_D(Asder,C000)
				{
					banks.chr[0] = data;
					UpdateChr();
				}

				NES_POKE_D(Asder,E000)
				{
					banks.chr[1] = data;
					UpdateChr();
					SetMirroringHV( data );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void FightingHero::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x7FFFU, &FightingHero::Poke_6000 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(~0U);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(FightingHero,6000)
				{
					ppu.Update();

					switch (address & 0x3)
					{
						case 0x0: chr.SwapBank<SIZE_4K,0x0000>( data >> 2 ); break;
						case 0x1: chr.SwapBank<SIZE_2K,0x1000>( data >> 1 ); break;
						case 0x2: chr.SwapBank<SIZE_2K,0x1800>( data >> 1 ); break;
						case 0x3: prg.SwapBank<SIZE_8K,0x0000>( data      ); break;
					}
				}
			}
		}
	}
}
