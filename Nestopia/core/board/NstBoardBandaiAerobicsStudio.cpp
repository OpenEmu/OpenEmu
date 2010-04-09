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
#include "NstBoardBandaiAerobicsStudio.hpp"

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

				AerobicsStudio::AerobicsStudio(const Context& c)
				:
				CnRom (c),
				sound (Sound::Player::Create(*c.apu,c.chips,NULL,Sound::Player::GAME_AEROBICS_STUDIO,8))
				{}

				AerobicsStudio::~AerobicsStudio()
				{
					Sound::Player::Destroy( sound );
				}

				void AerobicsStudio::SubReset(const bool hard)
				{
					CnRom::SubReset( hard );

					if (sound)
						Map( 0x6000U, &AerobicsStudio::Poke_6000 );
				}

				#ifdef NST_MSVC_OPTIMIZE
				#pragma optimize("", on)
				#endif

				NES_POKE_D(AerobicsStudio,6000)
				{
					NST_ASSERT( sound );

					if ((data & 0x40) == 0x00)
						sound->Play( data & 0x07 );
				}
			}
		}
	}
}
