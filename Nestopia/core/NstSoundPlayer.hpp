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

#ifndef NST_SOUND_PLAYER_H
#define NST_SOUND_PLAYER_H

#include "NstSoundPcm.hpp"
#include "api/NstApiSound.hpp"

#ifdef NST_PRAGMA_ONCE
#pragma once
#endif

namespace Nes
{
	namespace Core
	{
		namespace Sound
		{
			class Player : public Pcm
			{
				enum
				{
					GAME_NUM_SAMPLES_SHIFT = 4
				};

			public:

				enum Game
				{
					GAME_UNKNOWN,
					GAME_MOERO_PRO_YAKYUU         = 16U << GAME_NUM_SAMPLES_SHIFT | 1,
					GAME_MOERO_PRO_YAKYUU_88      = 20U << GAME_NUM_SAMPLES_SHIFT | 2,
					GAME_MOERO_PRO_TENNIS         = 19U << GAME_NUM_SAMPLES_SHIFT | 3,
					GAME_TERAO_NO_DOSUKOI_OOZUMOU = 6U  << GAME_NUM_SAMPLES_SHIFT | 4,
					GAME_AEROBICS_STUDIO          = 8U  << GAME_NUM_SAMPLES_SHIFT | 5
				};

				static Player* Create(Apu&,const Chips&,wcstring,Game,uint);
				static void Destroy(Player*);

			private:

				Player(Apu&,uint);
				~Player();

				struct Slot
				{
					Slot();
					~Slot();

					const iword* data;
					dword length;
					dword rate;
				};

				Slot* const slots;
				const uint numSlots;

			public:

				void Play(uint i)
				{
					if (i < numSlots && slots[i].data)
						Pcm::Play( slots[i].data, slots[i].length, slots[i].rate );
				}
			};
		}
	}
}

#endif

