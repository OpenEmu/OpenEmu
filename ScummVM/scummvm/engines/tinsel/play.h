/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Plays films within a scene, takes into account the actor in each 'column'.
 */

#ifndef TINSEL_PLAY_H	// prevent multiple includes
#define TINSEL_PLAY_H

#include "common/coroutines.h"
#include "tinsel/dw.h"
#include "tinsel/multiobj.h"

namespace Tinsel {

#define MAX_SOUNDREELS	5

struct SOUNDREELS {
	SCNHANDLE hFilm;	// The 'film'
	int	column;		// Column number
	int	actorCol;
};
typedef SOUNDREELS *PSOUNDREELS;

void PlayFilm(CORO_PARAM, SCNHANDLE film, int x, int y, int actorid, bool splay, bool sfact, bool escOn,
			int myescEvent, bool bTop);

void PlayFilm(CORO_PARAM, SCNHANDLE hFilm, int x, int y, int myescEvent, bool bTop);

void PlayFilmc(CORO_PARAM, SCNHANDLE hFilm, int x, int y, int actorid, bool splay, bool sfact,
			bool escOn, int myescEvent, bool bTop);

void RestoreActorReels(SCNHANDLE hFilm, short reelnum, short z, int x, int y);
void RestoreActorReels(SCNHANDLE hFilm, int actor, int x, int y);

void PokeInPalette(const MULTI_INIT *pmi);

void NoSoundReels();
void SaveSoundReels(PSOUNDREELS psr);
void RestoreSoundReels(PSOUNDREELS psr);

int ExtractActor(SCNHANDLE hFilm);

} // End of namespace Tinsel

#endif
