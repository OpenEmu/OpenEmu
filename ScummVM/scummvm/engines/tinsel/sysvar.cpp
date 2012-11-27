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
 * System variable handling.
 */

#include "tinsel/dw.h"
#include "tinsel/graphics.h"
#include "tinsel/dialogs.h"
#include "tinsel/strres.h"
#include "tinsel/sysvar.h"
#include "tinsel/tinsel.h"

#include "common/textconsole.h"

namespace Tinsel {

// Return for SYS_Platform
typedef enum { DOS_PC, WIN_PC, APPLE_MAC, SONY_PSX, SEGA_SATURN } platform;

//----------------- GLOBAL GLOBAL DATA --------------------

extern int NewestSavedGame();

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

static int g_systemVars[SV_TOPVALID] = {

		INV_1,		// Default inventory

		10,		// Y-offset of Conversation(TOP)
		320,		// Y-offset of Conversation(BOT)
		15,		// Minimum distance from side
		10,		// Minimum distance from top
		115,		// Distance above actor
		10,		// Distance below actor

		0,		// Current language **READ ONLY**
		0,		// Sample language **READ ONLY**
		0,		// Current state **READ ONLY**
		0,		// Saved Game Exists **READ ONLY**

		true,		// Should Conversation() wait for scroll? [TRUE]
		true,		// Should Talk()/Say() wait for scroll? [TRUE]

		true,		// Enable PointTag()
		true,		// Enable cursor with PrintCursor()

		100,		// SV_SCROLL_XTRIGGER
		0,			// SV_SCROLL_XDISTANCE
		16,			// SV_SCROLL_XSPEED
		40,			// SV_SCROLL_YTRIGGERTOP
		40,			// SV_SCROLL_YTRIGGERBOT
		0,			// SV_SCROLL_YDISTANCE
		16,			// SV_SCROLL_YSPEED

		2,		// Speech Delay
		2,		// Music dim factor

		0,		// if set, default actor's text color gets poked in here

		0,		// user 1
		0,		// user 2
		0,		// user 3
		0,		// user 4
		0,		// user 5
		0,		// user 6

		0,		// SYS_MinimumXoffset
		0,		// SYS_MaximumXoffset
		0,		// SYS_MinimumYoffset
		0,		// SYS_MaximumYoffset

		0,		// SYS_DefaultFxDimFactor
		0,		// SYS_SceneFxDimFactor

		0x606060,	// SYS_HighlightRGB
		WIN_PC,		// SYS_Platform,
		0,		// SYS_Debug

		0,		// ISV_DIVERT_ACTOR
		false,		// ISV_NO_BLOCKING

		0,		// ISV_GHOST_ACTOR
		0,		// ISV_GHOST_BASE
		0		// ISV_GHOST_COLOR
};

static SCNHANDLE g_systemStrings[SS_MAX_VALID];	// FIXME: Avoid non-const global vars

//static bool bFlagNoBlocking = false;

//----------------- FUNCTIONS --------------------------------

/**
 * Initializes the system variable list
 */

void InitSysVars() {
	g_systemVars[SV_SCROLL_XDISTANCE] = SCREEN_WIDTH / 2;
	g_systemVars[SV_SCROLL_YDISTANCE] = SCREEN_BOX_HEIGHT1 / 2;
}

/**
 * SetSysVar
 */

void SetSysVar(int varId, int newValue) {
	if (varId < 0 || varId >= SV_TOPVALID)
		error("SetSystemVar(): out of range identifier");

	switch (varId) {
	case SV_LANGUAGE:
	case SV_SAMPLE_LANGUAGE:
	case SV_SUBTITLES:
	case SV_SAVED_GAME_EXISTS:
	case SYS_Platform:
	case SYS_Debug:
		error("SetSystemVar(): read only identifier");

	default:
		g_systemVars[varId] = newValue;
	}
}

int SysVar(int varId) {
	if (varId < 0 || varId >= SV_TOPVALID)
		error("SystemVar(): out of range identifier");

	switch (varId) {
	case SV_LANGUAGE:
		return TextLanguage();

	case SV_SAMPLE_LANGUAGE:
		return SampleLanguage();

	case SV_SUBTITLES:
		// FIXME: This isn't currently defined
		return false;
		//return _vm->_config->_useSubtitles;

	case SV_SAVED_GAME_EXISTS:
		return NewestSavedGame() != -1;

	case SYS_Debug:
		// FIXME: This isn't currently defined
		return false;
		//return bDebuggingAllowed;

	default:
		return g_systemVars[varId];
	}
}

void SaveSysVars(int *pSv) {
	memcpy(pSv, g_systemVars, sizeof(g_systemVars));
}

void RestoreSysVars(int *pSv) {
	memcpy(g_systemVars, pSv, sizeof(g_systemVars));
}

void SetSysString(int number, SCNHANDLE hString) {
	assert(number >= 0 && number < SS_MAX_VALID);

	g_systemStrings[number] = hString;
}

SCNHANDLE SysString(int number) {
	assert(number >= 0 && number < SS_MAX_VALID);

	return g_systemStrings[number];
}

/**
 * Gets the no blocking flag. Note that for convenience, the systemVars array
 * entry is used even for Tinsel 1, which originally used a separate variable.
 */
bool GetNoBlocking() {
	return SysVar(ISV_NO_BLOCKING);
}

/**
 * Sets the no blocking flag. Note that for convenience, the systemVars array
 * entry is used even for Tinsel 1, which originally used a separate variable.
 */
void SetNoBlocking(bool flag) {
	SetSysVar(ISV_NO_BLOCKING, flag);
}

} // End of namespace Tinsel
