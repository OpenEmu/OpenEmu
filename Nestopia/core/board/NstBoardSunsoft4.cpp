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
#include "NstBoardSunsoft4.hpp"

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

				void S4::SubReset(const bool hard)
				{
					if (hard)
					{
						regs.ctrl = 0;
						regs.nmt[0] = Regs::BANK_OFFSET;
						regs.nmt[1] = Regs::BANK_OFFSET;
					}

					Map( 0x8000U, 0x8FFFU, CHR_SWAP_2K_0  );
					Map( 0x9000U, 0x9FFFU, CHR_SWAP_2K_1  );
					Map( 0xA000U, 0xAFFFU, CHR_SWAP_2K_2  );
					Map( 0xB000U, 0xBFFFU, CHR_SWAP_2K_3  );
					Map( 0xC000U, 0xCFFFU, &S4::Poke_C000 );
					Map( 0xD000U, 0xDFFFU, &S4::Poke_D000 );
					Map( 0xE000U, 0xEFFFU, &S4::Poke_E000 );
					Map( 0xF000U, 0xFFFFU, PRG_SWAP_16K_0 );
				}

				void S4::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'S','4'>::V) );

					if (baseChunk == AsciiId<'S','4'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							switch (chunk)
							{
								case AsciiId<'R','E','G'>::V:
								{
									State::Loader::Data<3> data( state );

									regs.ctrl = data[0];
									regs.nmt[0] = data[1] | Regs::BANK_OFFSET;
									regs.nmt[1] = data[2] | Regs::BANK_OFFSET;

									break;
								}
							}

							state.End();
						}
					}
				}

				void S4::SubSave(State::Saver& state) const
				{
					const byte data[3] =
					{
						regs.ctrl,
						regs.nmt[0] & ~uint(Regs::BANK_OFFSET),
						regs.nmt[1] & ~uint(Regs::BANK_OFFSET)
					};

					state.Begin( AsciiId<'S','4'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void S4::UpdateMirroring() const
				{
					ppu.Update();

					static const byte select[4][4] =
					{
						{0,1,0,1},
						{0,0,1,1},
						{0,0,0,0},
						{1,1,1,1}
					};

					const uint isCrom = (regs.ctrl & Regs::CTRL_CROM) >> 4;
					const byte (&index)[4] = select[regs.ctrl & Regs::CTRL_MIRRORING];

					for (uint i=0; i < 4; ++i)
						nmt.Source( isCrom ).SwapBank<SIZE_1K>( i * SIZE_1K, isCrom ? regs.nmt[index[i]] : index[i] );
				}

				NES_POKE_D(S4,C000)
				{
					regs.nmt[0] = Regs::BANK_OFFSET | data;
					UpdateMirroring();
				}

				NES_POKE_D(S4,D000)
				{
					regs.nmt[1] = Regs::BANK_OFFSET | data;
					UpdateMirroring();
				}

				NES_POKE_D(S4,E000)
				{
					regs.ctrl = data;
					UpdateMirroring();
				}
			}
		}
	}
}
