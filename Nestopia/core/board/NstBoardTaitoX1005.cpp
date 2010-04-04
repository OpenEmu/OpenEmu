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

#include <cstring>
#include "NstBoard.hpp"
#include "NstBoardTaitoX1005.hpp"
#include "../NstFile.hpp"

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

				X1005::X1005(const Context& c)
				: Board(c), version(DetectVersion(c))
				{
					std::memset( ram, 0, sizeof(ram) );
				}

				X1005::Version X1005::DetectVersion(const Context& c)
				{
					if (const Chips::Type* const type = c.chips.Find(L"X1-005"))
					{
						if (type->Pin(17).C(L"CIRAM").A() == 10 && type->Pin(31) == L"NC")
							return VERSION_B;
					}

					return VERSION_A;
				}

				void X1005::SubReset(const bool hard)
				{
					if (hard)
						security = 0;

					if (version == VERSION_A)
					{
						Map( 0x7EF0U, 0x7EF1U, &X1005::Poke_7EF0_0 );

						Map( 0x7EF2U, CHR_SWAP_1K_4 );
						Map( 0x7EF3U, CHR_SWAP_1K_5 );
						Map( 0x7EF4U, CHR_SWAP_1K_6 );
						Map( 0x7EF5U, CHR_SWAP_1K_7 );

						Map( 0x7EF6U, 0x7EF7U, NMT_SWAP_VH );

						ppu.SetMirroring( Ppu::NMT_H );
					}
					else
					{
						Map( 0x7EF0U, 0x7EF1U, &X1005::Poke_7EF0_1 );
						Map( 0x7EF2U, 0x7EF5U, &X1005::Poke_7EF2   );

						ppu.SetMirroring( Ppu::NMT_0 );
					}

					Map( 0x7EF8U, 0x7EF9U, &X1005::Peek_7EF8, &X1005::Poke_7EF8 );

					Map( 0x7EFAU, 0x7EFBU, PRG_SWAP_8K_0 );
					Map( 0x7EFCU, 0x7EFDU, PRG_SWAP_8K_1 );
					Map( 0x7EFEU, 0x7EFFU, PRG_SWAP_8K_2 );

					Map( 0x7F00U, 0x7FFFU, &X1005::Peek_7F00, &X1005::Poke_7F00 );
				}

				void X1005::Load(File& file)
				{
					if (board.HasBattery())
						file.Load( File::BATTERY, ram, sizeof(ram) );
				}

				void X1005::Save(File& file) const
				{
					if (board.HasBattery())
						file.Save( File::BATTERY, ram, sizeof(ram) );
				}

				void X1005::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'T','X','1'>::V) );

					if (baseChunk == AsciiId<'T','X','1'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:

									security = state.Read8();
									break;

								case AsciiId<'R','A','M'>::V:

									state.Uncompress( ram );
									break;
							}

							state.End();
						}
					}
				}

				void X1005::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'T','X','1'>::V );
					state.Begin( AsciiId<'R','E','G'>::V ).Write8( security ).End();
					state.Begin( AsciiId<'R','A','M'>::V ).Compress( ram ).End();
					state.End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(X1005,7EF0_0)
				{
					ppu.Update();
					chr.SwapBank<SIZE_2K>( address << 11 & 0x800, data >> 1 );
				}

				NES_POKE_AD(X1005,7EF0_1)
				{
					ppu.SetMirroring( (data & 0x80) ? Ppu::NMT_1 : Ppu::NMT_0 );
					chr.SwapBank<SIZE_2K>( address << 11 & 0x800, data >> 1 );
				}

				NES_POKE_AD(X1005,7EF2)
				{
					ppu.SetMirroring( (data & 0x80) ? Ppu::NMT_1 : Ppu::NMT_0 );
					chr.SwapBank<SIZE_1K>( 0x1000 | (address - 0x7EF2) << 10, data );
				}

				NES_PEEK(X1005,7EF8)
				{
					return security;
				}

				NES_POKE_D(X1005,7EF8)
				{
					security = data;
				}

				NES_PEEK_A(X1005,7F00)
				{
					NST_VERIFY( security == SECURITY_DATA );

					return (security == SECURITY_DATA) ? ram[address & 0x7F] : (address >> 8);
				}

				NES_POKE_AD(X1005,7F00)
				{
					NST_VERIFY( security == SECURITY_DATA );

					if (security == SECURITY_DATA)
						ram[address & 0x7F] = data;
				}
			}
		}
	}
}
