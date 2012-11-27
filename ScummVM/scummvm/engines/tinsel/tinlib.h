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
 * Text utility defines
 */

#ifndef TINSEL_TINLIB_H	// prevent multiple includes
#define TINSEL_TINLIB_H

#include "tinsel/dw.h"
#include "tinsel/object.h"
#include "tinsel/palette.h"

namespace Tinsel {

enum EXTREME {
	EX_USEXY, EX_BOTTOM, EX_BOTTOMLEFT,
	EX_BOTTOMRIGHT, EX_LEFT, EX_RIGHT,
	EX_TOP, EX_TOPLEFT, EX_TOPRIGHT
};

enum WHICH_VER {VER_GLITTER, VER_COMPILE};
#define VER_LEN 10

// Support functions
void TinGetVersion(WHICH_VER which, char *buffer, int length);

// Library functions in TINLIB.C

void ActorBrightness(int actor, int brightness);
void ActorPalette(int actor, int startColor, int length);
void Control(int param);
void HookScene(SCNHANDLE scene, int entrance, int transition);
void NewScene(CORO_PARAM, SCNHANDLE scene, int entrance, int transition);
void Offset(EXTREME extreme, int x, int y);
void FnRestartGame();
void RestoreScene();
void ResumeLastGame();
void SaveScene(CORO_PARAM);
void Stand(CORO_PARAM, int actor, int x, int y, SCNHANDLE film);
void SetTextPal(COLORREF col);

void KeepOnScreen(OBJECT *pText, int *pTextX, int *pTextY);

enum SPEECH_TYPE { IS_SAY, IS_SAYAT, IS_TALK, IS_TALKAT };

} // End of namespace Tinsel

#endif		// TINSEL_TINLIB_H
