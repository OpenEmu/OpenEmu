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
#include "NstBoardMmc3.hpp"
#include "NstBoardKasing.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Kasing
			{
				void Standard::SubReset(const bool hard)
				{
					if (hard)
					{
						exRegs[0] = 0;
						exRegs[1] = 0;
					}

					Mmc3::SubReset( hard );

					Map( 0x6000U, &Standard::Poke_6000 );
					Map( 0x6001U, &Standard::Poke_6001 );
				}

				void Standard::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'K','A','S'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								State::Loader::Data<2> data( state );

								exRegs[0] = data[0];
								exRegs[1] = data[1];
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void Standard::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );

					const byte data[2] =
					{
						exRegs[0],
						exRegs[1]
					};

					state.Begin( AsciiId<'K','A','S'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write( data ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void NST_FASTCALL Standard::UpdatePrg(uint address,uint bank)
				{
					if (exRegs[0] & 0x80)
						prg.SwapBank<SIZE_32K,0x0000>( exRegs[0] >> 1 );
					else
						Mmc3::UpdatePrg( address, bank );
				}

				void NST_FASTCALL Standard::UpdateChr(uint address,uint bank) const
				{
					chr.SwapBank<SIZE_1K>( address, (exRegs[1] << 8 & 0x100) | bank );
				}

				NES_POKE_D(Standard,6000)
				{
					if (exRegs[0] != data)
					{
						exRegs[0] = data;
						Mmc3::UpdatePrg();
					}
				}

				NES_POKE_D(Standard,6001)
				{
					if (exRegs[1] != data)
					{
						exRegs[1] = data;
						Mmc3::UpdateChr();
					}
				}
			}
		}
	}
}
