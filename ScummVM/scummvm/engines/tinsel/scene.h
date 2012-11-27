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
 * Scene parsing defines
 */

#ifndef	TINSEL_SCENE_H
#define	TINSEL_SCENE_H

#include "tinsel/dw.h"
#include "tinsel/events.h"

namespace Tinsel {

enum {
	MAX_NODES		= 32,	///< maximum nodes in a Node Path
	MAX_NOSCROLL	= 16,	///< maximum number of NoScroll commands in a scene
	MAX_ENTRANCE	= 25,	///< maximum number of entrances in a scene
	MAX_POLY		= 256,	///< maximum number of polygons in a scene
	MAX_ACTOR		= 32	///< maximum number of actors in a scene
};

// ENTRANCE_STRUC bitflags
enum ENTRANCE_FLAGS {
	fCall = 0x00000001L,
	fHook = 0x00000002L
};

/** reference direction */
enum REFTYPE {
	REF_DEFAULT, REF_UP, REF_DOWN, REF_LEFT, REF_RIGHT, REF_POINT
};

enum TFTYPE {
	TF_NONE, TF_UP, TF_DOWN, TF_LEFT, TF_RIGHT, TF_FILM
};

/** different actor masks */
enum MASK_TYPE{
	ACT_DEFAULT,
	ACT_MASK = -1,
	ACT_ALWAYS = -2
};

/** different scales */
enum SCALE {
	SCALE_DEFAULT, SCALE_LARGE, SCALE_MEDIUM, SCALE_SMALL,
	SCALE_COMPACT, SCALE_TINY,
	SCALE_AUX1, SCALE_AUX2, SCALE_AUX3,
	SCALE_AUX4, SCALE_AUX5
};

/** different reels */
enum REEL {
	REEL_DEFAULT, REEL_ALL, REEL_HORIZ, REEL_VERT
};

typedef enum { TRANS_DEF, TRANS_CUT, TRANS_FADE } TRANSITS;

// amount to shift scene handles by
#define	SCNHANDLE_SHIFT ((TinselV2 && !TinselV2Demo) ? 25 : 23)
#define	OFFSETMASK ((TinselV2 && !TinselV2Demo) ? 0x01ffffffL : 0x007fffffL)
#define HANDLEMASK ((TinselV2 && !TinselV2Demo) ? 0xFE000000L : 0xFF800000L)

void DoHailScene(SCNHANDLE scene);

void WrapScene();

void StartNewScene(SCNHANDLE scene, int entry);

void EndScene();

void SendSceneTinselProcess(TINSEL_EVENT event);

} // End of namespace Tinsel

#endif	// TINSEL_SCENE_H
