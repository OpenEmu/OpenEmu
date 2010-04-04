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
#include "NstBoardBandaiKaraokeStudio.hpp"
#include "../api/NstApiInput.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Bandai
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				void KaraokeStudio::SubReset(const bool hard)
				{
					Map( 0x6000U, 0x7FFFU, &KaraokeStudio::Peek_6000 );
					Map( 0x8000U, 0xFFFFU, &KaraokeStudio::Poke_8000 );

					if (hard)
						prg.SwapBank<SIZE_16K,0x4000>(0x7);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(KaraokeStudio,8000)
				{
					prg.SwapBank<SIZE_16K,0x0000>
					(
						data ? (data & 0x7) | (~data >> 1 & 0x8) : (prg.Source().Size() >> 18) + 0x7
					);
				}

				NES_PEEK(KaraokeStudio,6000)
				{
					return mic;
				}

				void KaraokeStudio::Sync(Event event,Input::Controllers* controllers)
				{
					if (event == EVENT_BEGIN_FRAME)
					{
						if (controllers)
						{
							Input::Controllers::KaraokeStudio::callback( controllers->karaokeStudio );
							mic = controllers->karaokeStudio.buttons & 0x7 ^ 0x3;
						}
						else
						{
							mic = 0x3;
						}
					}

					Board::Sync( event, controllers );
				}
			}
		}
	}
}
