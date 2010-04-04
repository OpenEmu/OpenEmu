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
#include "NstBoardSubor.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Subor
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void Type0::SubReset(bool)
				{
					Map( 0x8000U, 0xFFFFU, &Type0::Poke_8000 );

					for (uint i=0; i < 4; ++i)
						regs[i] = 0;

					NES_DO_POKE(8000,0x8000,0x00);
				}

				void StudyNGame::SubReset(const bool hard)
				{
					Map( 0x8000U, 0xFFFFU, PRG_SWAP_32K );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				void Type0::SubSave(State::Saver& state) const
				{
					state.Begin( AsciiId<'S','B','R'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( regs ).End().End();
				}

				void Type0::SubLoad(State::Loader& state,const dword baseChunk)
				{
					NST_VERIFY( baseChunk == (AsciiId<'S','B','R'>::V) );

					if (baseChunk == AsciiId<'S','B','R'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
								state.Read( regs );

							state.End();
						}
					}
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				uint Type0::GetMode() const
				{
					return 0;
				}

				uint Type1::GetMode() const
				{
					return 1;
				}

				NES_POKE_AD(Type0,8000)
				{
					regs[address >> 13 & 0x3] = data;

					uint banks[2] =
					{
						(uint(regs[0]) ^ regs[1]) << 1 & 0x20,
						(uint(regs[2]) ^ regs[3]) << 0 & 0x1F
					};

					const uint mode = GetMode();

					if (regs[1] & 0x8U)
					{
						banks[0] += banks[1] & 0xFE;
						banks[1] = banks[0];
						banks[0] += mode ^ 1;
						banks[1] += mode ^ 0;
					}
					else if (regs[1] & 0x4U)
					{
						banks[1] = banks[0] + banks[1];
						banks[0] = 0x1F;
					}
					else
					{
						banks[0] = banks[0] + banks[1];
						banks[1] = mode ? 0x07 : 0x20;
					}

					prg.SwapBanks<SIZE_16K,0x0000>( banks[0], banks[1] );
				}
			}
		}
	}
}
