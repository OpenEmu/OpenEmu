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

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_ADTYPES_H
#define WINTERMUTE_ADTYPES_H

namespace Wintermute {

typedef enum {
	GAME_NORMAL,
	GAME_WAITING_RESPONSE
} TGameStateEx;

typedef enum {
	OBJECT_ENTITY,
	OBJECT_REGION,
	OBJECT_ACTOR,
	OBJECT_NONE
} TObjectType;

typedef enum {
	ENTITY_NORMAL,
	ENTITY_SOUND
} TEntityType;

typedef enum {
	STATE_NONE,
	STATE_IDLE,
	STATE_PLAYING_ANIM,
	STATE_READY,
	STATE_FOLLOWING_PATH,
	STATE_SEARCHING_PATH,
	STATE_WAITING_PATH,
	STATE_TURNING_LEFT,
	STATE_TURNING_RIGHT,
	STATE_TURNING,
	STATE_TALKING,
	STATE_DIRECT_CONTROL,
	STATE_PLAYING_ANIM_SET
} TObjectState;

typedef enum {
	DIRECT_WALK_NONE,
	DIRECT_WALK_FW,
	DIRECT_WALK_BK
} TDirectWalkMode;

typedef enum {
	DIRECT_TURN_NONE,
	DIRECT_TURN_CW,
	DIRECT_TURN_CCW
} TDirectTurnMode;

typedef enum {
	RESPONSE_TEXT,
	RESPONSE_ICON
} TResponseStyle;

typedef enum {
	RESPONSE_ALWAYS,
	RESPONSE_ONCE,
	RESPONSE_ONCE_GAME
} TResponseType;


typedef enum {
	TALK_SKIP_LEFT = 0,
	TALK_SKIP_RIGHT = 1,
	TALK_SKIP_BOTH = 2,
	TALK_SKIP_NONE = 3
} TTalkSkipButton;

typedef enum {
	GEOM_WAYPOINT,
	GEOM_WALKPLANE,
	GEOM_BLOCKED,
	GEOM_GENERIC
} TGeomNodeType;

} // end of namespace Wintermute

#endif
