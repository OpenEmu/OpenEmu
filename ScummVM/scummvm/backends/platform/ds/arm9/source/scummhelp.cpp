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

#include "scummhelp.h"

#define ADD_BIND(k,d) do { key[i] = k; dsc[i] = d; i++; } while (0)
#define ADD_TEXT(d) ADD_BIND("",d)
#define ADD_LINE ADD_BIND("","")

#define HELP_NUM_LINES 15

namespace DS {

void updateStrings(byte gameId, byte version, Common::Platform platform,
			int page, Common::String &title, Common::String *&key, Common::String *&dsc) {
	key = new Common::String[HELP_NUM_LINES];
	dsc = new Common::String[HELP_NUM_LINES];
	int i = 0;
	switch (page) {
	case 1: {
		title = "DS Controls (right handed):";
		ADD_BIND("Pad Left", "Left mouse button");
		ADD_BIND("Pad Right", "Right mouse button");
		ADD_BIND("Pad Up", "Mouse hover mode (no click)");
		ADD_BIND("Pad Down", "Skip dialog line (some games)");
		ADD_BIND("Start", "Pause/Game menu");
		ADD_BIND("Select", "DS Options menu");
		ADD_BIND("B", "Skip cutscenes");
		ADD_BIND("A", "Switch screens");
		ADD_BIND("Y", "Show/hide debug console");
		ADD_BIND("X", "Show/hide keyboard");
		ADD_BIND("L+Pad/Pen", "Scroll current touch screen view");
		ADD_BIND("L+B/A", "Zoom in/out");
		break;
	}

	case 2: {
		title = "DS Controls (left handed):";
		ADD_BIND("Y", "Left mouse button");
		ADD_BIND("A", "Right mouse button");
		ADD_BIND("X", "Mouse hover mode (no click)");
		ADD_BIND("B", "Skip dialog line (some games)");
		ADD_BIND("Start", "Pause/Game menu");
		ADD_BIND("Select", "DS Options menu");
		ADD_BIND("Pad Down", "Skip cutscenes");
		ADD_BIND("Pad Up", "Show/hide keyboard");
		ADD_BIND("Pad Left", "Show/hide debug console");
		ADD_BIND("Pad Right", "Swap screens");
		ADD_BIND("R+Pad/Pen", "Scroll current touch screen view");
		ADD_BIND("R+dwn/rgt", "Zoom in/out");
		break;
	}

	case 3: {
		title = "Indiana Jones Fight controls:";
		ADD_BIND("Pad Left", "Move left");
		ADD_BIND("Pad Right", "Move right");
		ADD_BIND("Pad Up", "High guard");
		ADD_BIND("Pad Down", "Guard down");
		ADD_BIND("Y", "Guard middle");
		ADD_BIND("X", "Punch high");
		ADD_BIND("A", "Punch middle");
		ADD_BIND("B", "Punch low");
		break;
	}
	}


	while (i < HELP_NUM_LINES) {
		ADD_LINE;
	}

}

}	// End of namespace DS



#undef ADD_BIND
#undef ADD_TEXT
#undef ADD_LINE
