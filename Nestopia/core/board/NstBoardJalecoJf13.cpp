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
#include "NstBoardJalecoJf13.hpp"

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

				Jf13::Jf13(const Context& c)
				:
				Board (c),
				sound (Sound::Player::Create(*c.apu,c.chips,L"D7756C",board == Type::JALECO_JF13 ? Sound::Player::GAME_MOERO_PRO_YAKYUU : Sound::Player::GAME_UNKNOWN,32))
				{
				}

				Jf13::~Jf13()
				{
					Sound::Player::Destroy( sound );
				}

				void Jf13::SubReset(const bool hard)
				{
					Map( 0x6000U, &Jf13::Poke_6000 );

					if (sound)
						Map( 0x7000U, &Jf13::Poke_7000 );

					if (hard)
						prg.SwapBank<SIZE_32K,0x0000>(0);
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(Jf13,6000)
				{
					ppu.Update();
					prg.SwapBank<SIZE_32K,0x0000>( data >> 4 & 0x3 );
					chr.SwapBank<SIZE_8K,0x0000>( (data >> 4 & 0x4) | (data & 0x3) );
				}

				NES_POKE_D(Jf13,7000)
				{
					NST_ASSERT( sound );

					if ((data & 0x30) == 0x20)
						sound->Play( data & 0x1F );
				}
			}
		}
	}
}
