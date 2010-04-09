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
#include "NstBoardKay.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Kay
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void PandaPrince::SubReset(const bool hard)
				{
					exMode = 0;

					Mmc3::SubReset( hard );

					Map( 0x5000U, 0x5FFFU, &PandaPrince::Peek_5000 );
					Map( 0x8000U, 0x9FFFU, &PandaPrince::Poke_8000 );
				}

				void PandaPrince::SubLoad(State::Loader& state,const dword baseChunk)
				{
					if (baseChunk == AsciiId<'K','P','P'>::V)
					{
						while (const dword chunk = state.Begin())
						{
							if (chunk == AsciiId<'R','E','G'>::V)
							{
								exMode = state.Read8();

								if (exMode != 0xAB && exMode != 0xFF)
									exMode = 0;
							}

							state.End();
						}
					}
					else
					{
						Mmc3::SubLoad( state, baseChunk );
					}
				}

				void PandaPrince::SubSave(State::Saver& state) const
				{
					Mmc3::SubSave( state );
					state.Begin( AsciiId<'K','P','P'>::V ).Begin( AsciiId<'R','E','G'>::V ).Write8( exMode ).End().End();
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				void NST_FASTCALL PandaPrince::UpdatePrg(uint address,uint bank)
				{
					if (address == 0x6000)
					{
						if (exMode == 0xAB)
						{
							bank = 7;
						}
						else if (exMode == 0xFF)
						{
							bank = 9;
						}
					}

					prg.SwapBank<SIZE_8K>( address, bank );
				}

				NES_PEEK(PandaPrince,5000)
				{
					return 0x9F;
				}

				NES_POKE_AD(PandaPrince,8000)
				{
					if ((address & 0xF003) == 0x8003 && (data == 0xAB || data == 0xFF))
					{
						exMode = data;
					}
					else
					{
						exMode = 0;

						if (address & 0x1)
							Mmc3::NES_DO_POKE(8001,address,data);
						else
							Mmc3::NES_DO_POKE(8000,address,data);
					}

					Mmc3::UpdatePrg();
				}
			}
		}
	}
}
