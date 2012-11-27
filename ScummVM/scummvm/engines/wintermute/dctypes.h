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

#ifndef WINTERMUTE_DCTYPES_H
#define WINTERMUTE_DCTYPES_H


#include "common/str.h"
#include "common/list.h"
#include "common/array.h"

namespace Wintermute {

//typedef std::string AnsiString;
//typedef std::string Utf8String;
//typedef std::wstring WideString;
typedef Common::String AnsiString;
typedef Common::String Utf8String;
typedef Common::String WideString; // NB: Not actually true I presume.

typedef Common::List<WideString> WideStringList;
typedef Common::List<AnsiString> AnsiStringList;

typedef Common::Array<WideString> WideStringArray;
typedef Common::Array<AnsiString> AnsiStringArray;


enum TGameState {
	GAME_RUNNING,
	GAME_FROZEN,
	GAME_SEMI_FROZEN
};


enum TImageType {
	IMG_PALETTED8,
	IMG_TRUECOLOR
};


enum TTextAlign {
	TAL_LEFT = 0,
	TAL_RIGHT,
	TAL_CENTER,
	NUM_TEXT_ALIGN
};


enum TVerticalAlign {
	VAL_TOP = 0,
	VAL_CENTER,
	VAL_BOTTOM,
	NUM_VERTICAL_ALIGN
};


enum TDirection {
	DI_UP           = 0,
	DI_UPRIGHT      = 1,
	DI_RIGHT        = 2,
	DI_DOWNRIGHT    = 3,
	DI_DOWN         = 4,
	DI_DOWNLEFT     = 5,
	DI_LEFT         = 6,
	DI_UPLEFT       = 7,
	NUM_DIRECTIONS  = 8,
	DI_NONE         = 9
};

enum TEventType {
	EVENT_NONE           = 0,
	EVENT_INIT           = 1,
	EVENT_SHUTDOWN       = 2,
	EVENT_LEFT_CLICK     = 3,
	EVENT_RIGHT_CLICK    = 4,
	EVENT_MIDDLE_CLICK   = 5,
	EVENT_LEFT_DBLCLICK  = 6,
	EVENT_PRESS          = 7,
	EVENT_IDLE           = 8,
	EVENT_MOUSE_OVER     = 9,
	EVENT_LEFT_RELEASE   = 10,
	EVENT_RIGHT_RELEASE  = 11,
	EVENT_MIDDLE_RELEASE = 12,
	NUM_EVENTS
};

enum TUIObjectType {
	UI_UNKNOWN,
	UI_BUTTON,
	UI_WINDOW,
	UI_STATIC,
	UI_EDIT,
	UI_HTML,
	UI_CUSTOM
};


enum TRendererState {
	RSTATE_3D,
	RSTATE_2D,
	RSTATE_LINES,
	RSTATE_NONE
};


enum TDynamicConstructor {
	DYNAMIC_CONSTRUCTOR
};


enum TVideoMode {
	VIDEO_WINDOW,
	VIDEO_FULLSCREEN,
	VIDEO_ANY
};


enum TVideoPlayback {
	VID_PLAY_POS        = 0,
	VID_PLAY_STRETCH    = 1,
	VID_PLAY_CENTER     = 2
};


enum TMouseEvent {
	MOUSE_CLICK,
	MOUSE_RELEASE,
	MOUSE_DBLCLICK
};


enum TMouseButton {
	MOUSE_BUTTON_LEFT,
	MOUSE_BUTTON_RIGHT,
	MOUSE_BUTTON_MIDDLE
};


enum TTransMgrState {
	TRANS_MGR_RUNNING,
	TRANS_MGR_READY
};


enum TTransitionType {
	TRANSITION_NONE     = 0,
	TRANSITION_FADE_OUT = 1,
	TRANSITION_FADE_IN  = 2,
	NUM_TRANSITION_TYPES
};


enum TWindowMode {
	WINDOW_NORMAL,
	WINDOW_EXCLUSIVE,
	WINDOW_SYSTEM_EXCLUSIVE
};

enum TSFXType {
	SFX_NONE,
	SFX_ECHO,
	SFX_REVERB
};


enum TSpriteCacheType {
	CACHE_ALL,
	CACHE_HALF
};

enum TTextEncoding {
	TEXT_ANSI = 0,
	TEXT_UTF8 = 1,
	NUM_TEXT_ENCODINGS
};

enum TSpriteBlendMode {
	BLEND_UNKNOWN       = -1,
	BLEND_NORMAL        = 0,
	BLEND_ADDITIVE      = 1,
	BLEND_SUBTRACTIVE   = 2,
	NUM_BLEND_MODES
};

enum TTTSType {
	TTS_CAPTION = 0,
	TTS_TALK,
	TTS_KEYPRESS
};

enum TShadowType {
	SHADOW_NONE     = 0,
	SHADOW_SIMPLE   = 1,
	SHADOW_FLAT     = 2,
	SHADOW_STENCIL  = 3
};

} // end of namespace Wintermute

#endif
