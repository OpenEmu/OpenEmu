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

#ifndef TINSEL_SYSVAR_H	// prevent multiple includes
#define TINSEL_SYSVAR_H

namespace Tinsel {

typedef enum {	SV_DEFAULT_INV,

		SV_CONV_TOPY,		// Y-offset of Conversation(TOP)
		SV_CONV_BOTY,		// Y-offset of Conversation(BOT)
		SV_CONV_MINX,		// Minimum distance from side
		SV_CONV_MINY,		// Minimum distance from top
		SV_CONV_ABOVE_Y,	// Distance above actor
		SV_CONV_BELOW_Y,	// Distance below actor

		SV_LANGUAGE,
		SV_SAMPLE_LANGUAGE,
		SV_SUBTITLES,
		SV_SAVED_GAME_EXISTS,

		SV_CONVERSATIONWAITS,	// } Do they wait for
		SV_SPEECHWAITS,		// } scrolls to complete?

		SV_ENABLEPOINTTAG,	// Enable PointTag()
		SV_ENABLEPRINTCURSOR,	// Enable cursor with PrintCursor()

		SV_SCROLL_XTRIGGER,	// }
		SV_SCROLL_XDISTANCE,	// }
		SV_SCROLL_XSPEED,	// } Scroll parameters!
		SV_SCROLL_YTRIGGERTOP,	// }
		SV_SCROLL_YTRIGGERBOT,	// }
		SV_SCROLL_YDISTANCE,	// }
		SV_SCROLL_YSPEED,	// }

		SV_SPEECHDELAY,		// Delay 'twixt text/animation and sample
		SV_MUSICDIMFACTOR,	// dimVolume = volume - volume/SV_MDF

		SV_TAGCOLOR,		// if set, default actor's text color gets poked in here

		SV_USER1,
		SV_USER2,
		SV_USER3,
		SV_USER4,
		SV_USER5,
		SV_USER6,

		SV_MinimumXoffset,
		SV_MaximumXoffset,
		SV_MinimumYoffset,
		SV_MaximumYoffset,
						// dimVolume = volume - volume/DF
		SYS_DefaultFxDimFactor,		// To this at start of scene
		SYS_SceneFxDimFactor,		// Alter within scene

		SYS_HighlightRGB,
		SYS_Platform,			// Hardware platform     **READ ONLY**
		SYS_Debug,			// TRUE for debug build/'cheat'**READ ONLY**

		ISV_DIVERT_ACTOR,
		ISV_NO_BLOCKING,
		ISV_GHOST_ACTOR,
		ISV_GHOST_BASE,
		ISV_GHOST_COLOR,


		SV_TOPVALID } SYSVARS;

typedef enum {

					// Main Menu
	SS_LOAD_OPTION,			//
	SS_SAVE_OPTION,			//
	SS_RESTART_OPTION,		//
	SS_SOUND_OPTION,		//
	SS_CONTROL_OPTION,		//
	SS_SUBTITLES_OPTION,		//
	SS_QUIT_OPTION,			//
	SS_RESUME_OPTION,		//

	SS_LOAD_HEADING,
	SS_SAVE_HEADING,
	SS_RESTART_HEADING,
	SS_QUIT_HEADING,

	SS_MVOL_SLIDER,
	SS_SVOL_SLIDER,
	SS_VVOL_SLIDER,

	SS_DCLICK_SLIDER,
	SS_DCLICK_TEST,
	SS_SWAP_TOGGLE,

	SS_TSPEED_SLIDER,
	SS_STITLE_TOGGLE,

	SS_HOPPER1,			// Hopper scene menu heading

	SS_SOUND_HEADING,
	SS_CONTROLS_HEADING,
	SS_LANGUAGE_SELECT,

	SS_MAX_VALID
} BOLLOX;

void InitSysVars();

void SetSysVar(int varId, int newValue);

int SysVar(int varId);

void SaveSysVars(int *pSv);
void RestoreSysVars(int *pSv);

void SetSysString(int number, SCNHANDLE hString);

SCNHANDLE SysString(int number);

bool GetNoBlocking();

void SetNoBlocking(bool flag);

} // End of namespace Tinsel

#endif
