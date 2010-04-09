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
#include "../NstSoundPlayer.hpp"
#include "NstBoardJalecoJf17.hpp"

namespace Nes
{
	namespace Core
	{
		namespace Boards
		{
			namespace Jaleco
			{
				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("s", on)
				#endif

				Jf17::Jf17(const Context& c)
				:
				Board (c),
				sound (Sound::Player::Create(*c.apu,c.chips,L"D7756C",board == Type::JALECO_JF17 ? Sound::Player::GAME_MOERO_PRO_TENNIS : Sound::Player::GAME_UNKNOWN,32))
				{
				}

				Jf17::~Jf17()
				{
					Sound::Player::Destroy( sound );
				}

				void Jf17::SubReset(bool)
				{
					Map( 0x8000U, 0xFFFFU, &Jf17::Poke_8000 );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_AD(Jf17,8000)
				{
					data = GetBusData(address,data);

					if (data & 0x40)
					{
						ppu.Update();
						chr.SwapBank<SIZE_8K,0x0000>( data & 0xF );
					}

					if (data & 0x80)
						prg.SwapBank<SIZE_16K,0x0000>( data & 0xF );

					if (sound && (data & 0x30) == 0x20)
						sound->Play( address & 0x1F );
				}
			}
		}
	}
}
