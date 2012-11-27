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

#include "tinsel/tinsel.h"
#include "tinsel/debugger.h"
#include "tinsel/dialogs.h"
#include "tinsel/pcode.h"
#include "tinsel/scene.h"
#include "tinsel/sound.h"
#include "tinsel/music.h"
#include "tinsel/font.h"
#include "tinsel/strres.h"

namespace Tinsel {

//----------------- EXTERNAL FUNCTIONS ---------------------

// In PDISPLAY.CPP
extern void TogglePathDisplay();
// In tinsel.cpp
extern void SetNewScene(SCNHANDLE scene, int entrance, int transition);
// In scene.cpp
extern SCNHANDLE GetSceneHandle();

//----------------- SUPPORT FUNCTIONS ---------------------

//static
int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp;
	if (!sscanf(s, "%xh", &tmp))
		tmp = 0;
	return (int)tmp;
}

//----------------- CONSOLE CLASS  ---------------------

Console::Console() : GUI::Debugger() {
	DCmd_Register("item",		WRAP_METHOD(Console, cmd_item));
	DCmd_Register("scene",		WRAP_METHOD(Console, cmd_scene));
	DCmd_Register("music",		WRAP_METHOD(Console, cmd_music));
	DCmd_Register("sound",		WRAP_METHOD(Console, cmd_sound));
	DCmd_Register("string",		WRAP_METHOD(Console, cmd_string));
}

Console::~Console() {
}

bool Console::cmd_item(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("%s item_number\n", argv[0]);
		DebugPrintf("Sets the currently active 'held' item\n");
		return true;
	}

	HoldItem(INV_NOICON);
	HoldItem(strToInt(argv[1]));
	return false;
}

bool Console::cmd_scene(int argc, const char **argv) {
	if (argc < 1 || argc > 3) {
		DebugPrintf("%s [scene_number [entry number]]\n", argv[0]);
		DebugPrintf("If no parameters are given, prints the current scene.\n");
		DebugPrintf("Otherwise changes to the specified scene number. Entry number defaults to 1 if none provided\n");
		return true;
	}

	if (argc == 1) {
		DebugPrintf("Current scene is %d\n", GetSceneHandle() >> SCNHANDLE_SHIFT);
		return true;
	}

	uint32 sceneNumber = (uint32)strToInt(argv[1]) << SCNHANDLE_SHIFT;
	int entryNumber = (argc >= 3) ? strToInt(argv[2]) : 1;

	SetNewScene(sceneNumber, entryNumber, TRANS_CUT);
	return false;
}

bool Console::cmd_music(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("%s track_number or %s -offset\n", argv[0], argv[0]);
		DebugPrintf("Plays the MIDI track number provided, or the offset inside midi.dat\n");
		DebugPrintf("A positive number signifies a track number, whereas a negative signifies an offset\n");
		return true;
	}

	int param = strToInt(argv[1]);
	if (param == 0) {
		DebugPrintf("Track number/offset can't be 0!\n");
	} else if (param > 0) {
		// Track provided
		PlayMidiSequence(GetTrackOffset(param - 1), false);
	} else if (param < 0) {
		// Offset provided
		param = param * -1;
		PlayMidiSequence(param, false);
	}
	return true;
}

bool Console::cmd_sound(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("%s id\n", argv[0]);
		DebugPrintf("Plays the sound with the given ID\n");
		return true;
	}

	int id = strToInt(argv[1]);
	if (_vm->_sound->sampleExists(id)) {
		if (!TinselV2)
			_vm->_sound->playSample(id, Audio::Mixer::kSpeechSoundType);
		else
			_vm->_sound->playSample(id, 0, false, 0, 0, PRIORITY_TALK, Audio::Mixer::kSpeechSoundType);
	} else {
		DebugPrintf("Sample %d does not exist!\n", id);
	}

	return true;
}

bool Console::cmd_string(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("%s id\n", argv[0]);
		DebugPrintf("Prints the string with the given ID\n");
		return true;
	}

	char tmp[TBUFSZ];
	int id = strToInt(argv[1]);
	LoadStringRes(id, tmp, TBUFSZ);
	DebugPrintf("%s\n", tmp);

	return true;
}

} // End of namespace Tinsel
