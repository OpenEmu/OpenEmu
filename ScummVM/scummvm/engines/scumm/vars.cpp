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


#include "common/config-manager.h"
#include "scumm/scumm.h"
#include "scumm/scumm_v0.h"
#include "scumm/scumm_v8.h"
#include "scumm/sound.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

#include "audio/mididrv.h"

namespace Scumm {

void ScummEngine::setupScummVars() {
	VAR_KEYPRESS = 0;
	VAR_EGO = 1;
	VAR_CAMERA_POS_X = 2;
	VAR_HAVE_MSG = 3;
	VAR_ROOM = 4;
	VAR_OVERRIDE = 5;
	VAR_MACHINE_SPEED = 6;
	VAR_ME = 7;
	VAR_NUM_ACTOR = 8;
	VAR_CURRENTDRIVE = 10;
	VAR_TMR_1 = 11;
	VAR_TMR_2 = 12;
	VAR_TMR_3 = 13;
	VAR_MUSIC_TIMER = 14;
	VAR_ACTOR_RANGE_MIN = 15;
	VAR_ACTOR_RANGE_MAX = 16;
	VAR_CAMERA_MIN_X = 17;
	VAR_CAMERA_MAX_X = 18;
	VAR_TIMER_NEXT = 19;
	VAR_VIRT_MOUSE_X = 20;
	VAR_VIRT_MOUSE_Y = 21;
	VAR_ROOM_RESOURCE = 22;
	VAR_LAST_SOUND = 23;
	VAR_CUTSCENEEXIT_KEY = 24;
	VAR_TALK_ACTOR = 25;
	VAR_CAMERA_FAST_X = 26;
	VAR_ENTRY_SCRIPT = 28;
	VAR_ENTRY_SCRIPT2 = 29;
	VAR_EXIT_SCRIPT = 30;
	VAR_EXIT_SCRIPT2 = 31;
	VAR_VERB_SCRIPT = 32;
	VAR_SENTENCE_SCRIPT = 33;
	VAR_INVENTORY_SCRIPT = 34;
	VAR_CUTSCENE_START_SCRIPT = 35;
	VAR_CUTSCENE_END_SCRIPT = 36;
	VAR_CHARINC = 37;
	VAR_WALKTO_OBJ = 38;
	VAR_HEAPSPACE = 40;
	VAR_RESTART_KEY = 42;
	VAR_PAUSE_KEY = 43;
	VAR_MOUSE_X = 44;
	VAR_MOUSE_Y = 45;
	VAR_TIMER = 46;
	VAR_TIMER_TOTAL = 47;
	VAR_SOUNDCARD = 48;
	VAR_VIDEOMODE = 49;

	if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine) {
		VAR_MAINMENU_KEY = 50;
	}

	if (_game.version >= 4) {
		VAR_SCROLL_SCRIPT = 27;
		VAR_DEBUGMODE = 39;
		VAR_MAINMENU_KEY = 50;
		VAR_FIXEDDISK = 51;
		VAR_CURSORSTATE = 52;
		VAR_USERPUT = 53;
	}

	if (_game.version >= 5) {
		VAR_SOUNDRESULT = 56;
		VAR_TALKSTOP_KEY = 57;
		VAR_FADE_DELAY = 59;
		VAR_SOUNDPARAM = 64;
		VAR_SOUNDPARAM2 = 65;
		VAR_SOUNDPARAM3 = 66;
		VAR_INPUTMODE = 67;	// 1 is keyboard, 2 is joystick, 3 is mouse
		VAR_MEMORY_PERFORMANCE = 68;
		VAR_VIDEO_PERFORMANCE = 69;
		VAR_ROOM_FLAG = 70;
		VAR_GAME_LOADED = 71;
		VAR_NEW_ROOM = 72;
	}
}

void ScummEngine_v0::setupScummVars() {
	VAR_EGO = 0;
	VAR_CAMERA_POS_X = 2;
	VAR_HAVE_MSG = 3;
	VAR_ROOM = 4;
	VAR_ACTIVE_OBJECT2 = 5;
	VAR_OVERRIDE = 6;
	VAR_IS_SOUND_RUNNING = 8;
	VAR_ACTIVE_VERB = 9;
	VAR_CHARCOUNT = 10;
}

void ScummEngine_v2::setupScummVars() {
	VAR_EGO = 0;
	VAR_CAMERA_POS_X = 2;
	VAR_HAVE_MSG = 3;
	VAR_ROOM = 4;
	VAR_OVERRIDE = 5;
	VAR_MACHINE_SPEED = 6;
	VAR_CHARCOUNT = 7;
	VAR_ACTIVE_VERB = 8;
	VAR_ACTIVE_OBJECT1 = 9;
	VAR_ACTIVE_OBJECT2 = 10;
	VAR_NUM_ACTOR = 11;
	VAR_CURRENT_LIGHTS = 12;
	VAR_CURRENTDRIVE = 13;
	VAR_MUSIC_TIMER = 17;
	VAR_VERB_ALLOWED = 18;
	VAR_ACTOR_RANGE_MIN = 19;
	VAR_ACTOR_RANGE_MAX = 20;
	VAR_CURSORSTATE = 21;
	VAR_CAMERA_MIN_X = 23;
	VAR_CAMERA_MAX_X = 24;
	VAR_TIMER_NEXT = 25;
	VAR_SENTENCE_VERB = 26;
	VAR_SENTENCE_OBJECT1 = 27;
	VAR_SENTENCE_OBJECT2 = 28;
	VAR_SENTENCE_PREPOSITION = 29;
	VAR_VIRT_MOUSE_X = 30;
	VAR_VIRT_MOUSE_Y = 31;
	VAR_CLICK_AREA = 32;
	VAR_CLICK_VERB = 33;
	VAR_CLICK_OBJECT = 35;
	VAR_ROOM_RESOURCE = 36;
	VAR_LAST_SOUND = 37;
	VAR_BACKUP_VERB = 38;
	VAR_KEYPRESS = 39;
	VAR_CUTSCENEEXIT_KEY = 40;
	VAR_TALK_ACTOR = 41;
}

void ScummEngine_v5::setupScummVars() {
	// Many vars are the same as in V5 & V6 games, so just call the inherited method first
	ScummEngine::setupScummVars();

	VAR_CURRENT_LIGHTS = 9;

	if (_game.version >= 4) {
		VAR_V5_TALK_STRING_Y = 54;
	}
	if ((_game.id == GID_LOOM && _game.version == 4) || _game.version >= 5) {
		VAR_NOSUBTITLES = 60;
	}
}

void ScummEngine_v6::setupScummVars() {
	// Many vars are the same as in V5 & V6 games, so just call the inherited method first
	ScummEngine::setupScummVars();

	VAR_ROOM_WIDTH = 41;
	VAR_ROOM_HEIGHT = 54;

	if (_game.heversion >= 60) {
		VAR_NOSUBTITLES = 60;
	} else {
		VAR_VOICE_MODE = 60; // 0 is voice, 1 is voice+text, 2 is text only
		VAR_SAVELOAD_SCRIPT = 61;
		VAR_SAVELOAD_SCRIPT2 = 62;
	}

	VAR_LEFTBTN_HOLD = 74;
	VAR_RIGHTBTN_HOLD = 75;

	VAR_V6_EMSSPACE = 76;
	VAR_RANDOM_NR = 118;

	VAR_TIMEDATE_YEAR = 119;
	VAR_TIMEDATE_MONTH = 129;
	VAR_TIMEDATE_DAY = 128;
	VAR_TIMEDATE_HOUR = 125;
	VAR_TIMEDATE_MINUTE = 126;

	// Sam & Max specific
	if (_game.id == GID_SAMNMAX) {
		VAR_V6_SOUNDMODE = 9;
		VAR_CHARSET_MASK = 123;
	}
}

void ScummEngine_v70he::setupScummVars() {
	ScummEngine_v6::setupScummVars();

	VAR_CURRENTDRIVE = 0xFF;
	VAR_MUSIC_TIMER = 0xFF;

	VAR_NUM_SOUND_CHANNELS = 9;
	VAR_TALK_CHANNEL = 10;
	VAR_SOUND_CHANNEL = 14;
}

#ifdef ENABLE_HE
void ScummEngine_v72he::setupScummVars() {
	VAR_KEYPRESS = 0;
	VAR_DEBUGMODE = 1;
	VAR_TIMER_NEXT = 2;
	VAR_OVERRIDE = 3;
	VAR_WALKTO_OBJ = 4;
	VAR_RANDOM_NR = 5;

	VAR_GAME_LOADED = 8;
	VAR_EGO = 9;
	VAR_NUM_ACTOR = 10;

	VAR_VIRT_MOUSE_X = 13;
	VAR_VIRT_MOUSE_Y = 14;
	VAR_MOUSE_X = 15;
	VAR_MOUSE_Y = 16;
	VAR_LEFTBTN_HOLD = 17;
	VAR_RIGHTBTN_HOLD = 18;

	VAR_CURSORSTATE = 19;
	VAR_USERPUT = 20;
	VAR_ROOM = 21;
	VAR_ROOM_WIDTH = 22;
	VAR_ROOM_HEIGHT = 23;
	VAR_CAMERA_POS_X = 24;
	VAR_CAMERA_MIN_X = 25;
	VAR_CAMERA_MAX_X = 26;
	VAR_ROOM_RESOURCE = 27;
	VAR_SCROLL_SCRIPT = 28;
	VAR_ENTRY_SCRIPT = 29;
	VAR_ENTRY_SCRIPT2 = 30;
	VAR_EXIT_SCRIPT = 31;
	VAR_EXIT_SCRIPT2 = 32;
	VAR_VERB_SCRIPT = 33;
	VAR_SENTENCE_SCRIPT = 34;
	VAR_INVENTORY_SCRIPT = 35;
	VAR_CUTSCENE_START_SCRIPT = 36;
	VAR_CUTSCENE_END_SCRIPT = 37;

	VAR_RESTART_KEY = 42;
	VAR_PAUSE_KEY = 43;
	VAR_CUTSCENEEXIT_KEY = 44;
	VAR_TALKSTOP_KEY = 45;
	VAR_HAVE_MSG = 46;
	VAR_SUBTITLES = 47;
	VAR_CHARINC = 48;
	VAR_TALK_ACTOR = 49;
	VAR_LAST_SOUND = 50;
	VAR_TALK_CHANNEL = 51;
	VAR_SOUND_CHANNEL = 52;

	VAR_MEMORY_PERFORMANCE = 57;
	VAR_VIDEO_PERFORMANCE = 58;
	VAR_NEW_ROOM = 59;
	VAR_TMR_1 = 60;
	VAR_TMR_2 = 61;
	VAR_TMR_3 = 62;
	VAR_TIMEDATE_HOUR = 63;
	VAR_TIMEDATE_MINUTE = 64;
	VAR_TIMEDATE_DAY = 65;
	VAR_TIMEDATE_MONTH = 66;
	VAR_TIMEDATE_YEAR = 67;

	VAR_NUM_ROOMS = 68;
	VAR_NUM_SCRIPTS = 69;
	VAR_NUM_SOUNDS = 70;
	VAR_NUM_COSTUMES = 71;
	VAR_NUM_IMAGES = 72;
	VAR_NUM_CHARSETS = 73;
	VAR_NUM_GLOBAL_OBJS = 74;
	VAR_MOUSE_STATE = 75;
	VAR_POLYGONS_ONLY = 76;

	if (_game.heversion <= 74) {
		VAR_SOUND_ENABLED = 54;
		VAR_NUM_SOUND_CHANNELS = 56;
	}

	if (_game.heversion >= 74) {
		VAR_PLATFORM = 78;
	}
}

void ScummEngine_v80he::setupScummVars() {
	ScummEngine_v72he::setupScummVars();

	VAR_PLATFORM_VERSION = 79;
	VAR_CURRENT_CHARSET = 80;
	VAR_SOUNDCODE_TMR = 84;
	VAR_KEY_STATE = 86;
	VAR_NUM_SOUND_CHANNELS = 88;
	VAR_COLOR_DEPTH = 89;
	VAR_REDRAW_ALL_ACTORS = 95;
}

void ScummEngine_v90he::setupScummVars() {
	ScummEngine_v80he::setupScummVars();

	VAR_TIMER = 97;
	VAR_SCRIPT_CYCLE = 103;
	VAR_NUM_SCRIPT_CYCLES = 104;

	if (_game.heversion >= 95) {
		VAR_NUM_SPRITE_GROUPS = 105;
		VAR_NUM_SPRITES = 106;
		VAR_U32_VERSION = 107;
		VAR_U32_ARRAY_UNK = 116;
		VAR_WIZ_TCOLOR = 117;
		VAR_RESERVED_SOUND_CHANNELS = 120;
	}
	if (_game.heversion >= 98) {
		VAR_SKIP_RESET_TALK_ACTOR = 125;
	}
	if (_game.heversion >= 99) {
		VAR_MAIN_SCRIPT = 127;
		VAR_NUM_PALETTES = 130;
		VAR_NUM_UNK = 131;
	}
}
#endif

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::setupScummVars() {
	VAR_MOUSE_X = 1;
	VAR_MOUSE_Y = 2;
	VAR_VIRT_MOUSE_X = 3;
	VAR_VIRT_MOUSE_Y = 4;
	VAR_ROOM_WIDTH = 5;
	VAR_ROOM_HEIGHT = 6;
	VAR_CAMERA_POS_X = 7;
	VAR_CAMERA_POS_Y = 8;
	VAR_OVERRIDE = 9;
	VAR_ROOM = 10;
	VAR_ROOM_RESOURCE = 11;
	VAR_TALK_ACTOR = 12;
	VAR_HAVE_MSG = 13;
	VAR_TIMER = 14;
	VAR_TIMER_TOTAL = 15;

	VAR_TIMEDATE_YEAR = 16;
	VAR_TIMEDATE_MONTH = 17;
	VAR_TIMEDATE_DAY = 18;
	VAR_TIMEDATE_HOUR = 19;
	VAR_TIMEDATE_MINUTE = 20;
	VAR_TIMEDATE_SECOND = 21;

	VAR_LEFTBTN_DOWN = 22;
	VAR_RIGHTBTN_DOWN = 23;
	VAR_LEFTBTN_HOLD = 24;
	VAR_RIGHTBTN_HOLD = 25;

	VAR_MEMORY_PERFORMANCE = 26;
	VAR_VIDEO_PERFORMANCE = 27;
	VAR_GAME_LOADED = 29;
	VAR_V6_EMSSPACE = 32;
	VAR_VOICE_MODE = 33; // 0 is voice, 1 is voice+text, 2 is text only
	VAR_RANDOM_NR = 34;
	VAR_NEW_ROOM = 35;
	VAR_WALKTO_OBJ = 36;

	VAR_NUM_GLOBAL_OBJS = 37;

	VAR_CAMERA_DEST_X = 38;
	VAR_CAMERA_DEST_Y = 39;
	VAR_CAMERA_FOLLOWED_ACTOR = 40;

	VAR_SCROLL_SCRIPT = 50;
	VAR_ENTRY_SCRIPT = 51;
	VAR_ENTRY_SCRIPT2 = 52;
	VAR_EXIT_SCRIPT = 53;
	VAR_EXIT_SCRIPT2 = 54;
	VAR_VERB_SCRIPT = 55;
	VAR_SENTENCE_SCRIPT = 56;
	VAR_INVENTORY_SCRIPT = 57;
	VAR_CUTSCENE_START_SCRIPT = 58;
	VAR_CUTSCENE_END_SCRIPT = 59;
	VAR_SAVELOAD_SCRIPT = 60;
	VAR_SAVELOAD_SCRIPT2 = 61;

	VAR_CUTSCENEEXIT_KEY = 62;
	VAR_RESTART_KEY = 63;
	VAR_PAUSE_KEY = 64;
	VAR_MAINMENU_KEY = 65;
	VAR_VERSION_KEY = 66;
	VAR_TALKSTOP_KEY = 67;
	VAR_KEYPRESS = 118;

	VAR_TIMER_NEXT = 97;
	VAR_TMR_1 = 98;
	VAR_TMR_2 = 99;
	VAR_TMR_3 = 100;

	VAR_CAMERA_MIN_X = 101;
	VAR_CAMERA_MAX_X = 102;
	VAR_CAMERA_MIN_Y = 103;
	VAR_CAMERA_MAX_Y = 104;
	VAR_CAMERA_THRESHOLD_X = 105;
	VAR_CAMERA_THRESHOLD_Y = 106;
	VAR_CAMERA_SPEED_X = 107;
	VAR_CAMERA_SPEED_Y = 108;
	VAR_CAMERA_ACCEL_X = 109;
	VAR_CAMERA_ACCEL_Y = 110;

	VAR_EGO = 111;

	VAR_CURSORSTATE = 112;
	VAR_USERPUT = 113;
	VAR_DEFAULT_TALK_DELAY = 114;
	VAR_CHARINC = 115;
	VAR_DEBUGMODE = 116;
	VAR_FADE_DELAY = 117;

	// Full Throttle specific
	if (_game.id == GID_FT) {
		VAR_CHARSET_MASK = 119;
	}

	VAR_VIDEONAME = 123;

	VAR_STRING2DRAW = 130;
	VAR_CUSTOMSCALETABLE = 131;

	VAR_BLAST_ABOVE_TEXT = 133;

	VAR_MUSIC_BUNDLE_LOADED = 135;
	VAR_VOICE_BUNDLE_LOADED = 136;

}

void ScummEngine_v8::setupScummVars() {
	VAR_ROOM_WIDTH = 1;
	VAR_ROOM_HEIGHT = 2;

	VAR_MOUSE_X = 3;
	VAR_MOUSE_Y = 4;
	VAR_VIRT_MOUSE_X = 5;
	VAR_VIRT_MOUSE_Y = 6;

	VAR_CURSORSTATE = 7;
	VAR_USERPUT = 8;

	VAR_CAMERA_POS_X = 9;
	VAR_CAMERA_POS_Y = 10;
	VAR_CAMERA_DEST_X = 11;
	VAR_CAMERA_DEST_Y = 12;
	VAR_CAMERA_FOLLOWED_ACTOR = 13;

	VAR_TALK_ACTOR = 14;
	VAR_HAVE_MSG = 15;

	VAR_LEFTBTN_DOWN = 16;
	VAR_RIGHTBTN_DOWN = 17;
	VAR_LEFTBTN_HOLD = 18;
	VAR_RIGHTBTN_HOLD = 19;

	VAR_TIMEDATE_YEAR = 24;
	VAR_TIMEDATE_MONTH = 25;
	VAR_TIMEDATE_DAY = 26;
	VAR_TIMEDATE_HOUR = 27;
	VAR_TIMEDATE_MINUTE = 28;
	VAR_TIMEDATE_SECOND = 29;

	VAR_OVERRIDE = 30;
	VAR_ROOM = 31;
	VAR_NEW_ROOM = 32;
	VAR_WALKTO_OBJ = 33;
	VAR_TIMER = 34;

	VAR_VOICE_MODE = 39; // 0 is voice, 1 is voice+text, 2 is text only
	VAR_GAME_LOADED = 40;
	VAR_LANGUAGE = 41;

	VAR_CURRENTDISK = 42;
	VAR_MUSIC_BUNDLE_LOADED = 45;
	VAR_VOICE_BUNDLE_LOADED = 46;

	VAR_SCROLL_SCRIPT = 50;
	VAR_ENTRY_SCRIPT = 51;
	VAR_ENTRY_SCRIPT2 = 52;
	VAR_EXIT_SCRIPT = 53;
	VAR_EXIT_SCRIPT2 = 54;
	VAR_VERB_SCRIPT = 55;
	VAR_SENTENCE_SCRIPT = 56;
	VAR_INVENTORY_SCRIPT = 57;
	VAR_CUTSCENE_START_SCRIPT = 58;
	VAR_CUTSCENE_END_SCRIPT = 59;

	VAR_CUTSCENEEXIT_KEY = 62;

	VAR_PAUSE_KEY = 64;
	VAR_MAINMENU_KEY = 65;
	VAR_VERSION_KEY = 66;
	VAR_TALKSTOP_KEY = 67;

	VAR_CUSTOMSCALETABLE = 111;

	VAR_TIMER_NEXT = 112;
	VAR_TMR_1 = 113;
	VAR_TMR_2 = 114;
	VAR_TMR_3 = 115;

	VAR_CAMERA_MIN_X = 116;
	VAR_CAMERA_MAX_X = 117;
	VAR_CAMERA_MIN_Y = 118;
	VAR_CAMERA_MAX_Y = 119;
	VAR_CAMERA_SPEED_X = 120;
	VAR_CAMERA_SPEED_Y = 121;
	VAR_CAMERA_ACCEL_X = 122;
	VAR_CAMERA_ACCEL_Y = 123;
	VAR_CAMERA_THRESHOLD_X = 124;
	VAR_CAMERA_THRESHOLD_Y = 125;

	VAR_EGO = 126;

	VAR_DEFAULT_TALK_DELAY = 128;
	VAR_CHARINC = 129;

	VAR_DEBUGMODE = 130;
	VAR_KEYPRESS = 132;
	VAR_BLAST_ABOVE_TEXT = 133;
	VAR_SYNC = 134;
}
#endif

void ScummEngine_v0::resetScummVars() {
	resetSentence();

	VAR(VAR_EGO) = 3;

	// Setup light
	_currentLights = LIGHTMODE_actor_use_base_palette | LIGHTMODE_actor_use_colors | LIGHTMODE_room_lights_on;
}

void ScummEngine_v2::resetScummVars() {
	// This needs to be at least greater than 40 to get the more
	// elaborate version of the EGA Zak into. I don't know where
	// else it makes any difference.
	if (_game.id == GID_ZAK)
		VAR(VAR_MACHINE_SPEED) = 0x7FFF;
}

void ScummEngine_v5::resetScummVars() {
	ScummEngine::resetScummVars();

	if (_game.version >= 4 && _game.version <= 5)
		VAR(VAR_V5_TALK_STRING_Y) = -0x50;

	// Setup light
	VAR(VAR_CURRENT_LIGHTS) = LIGHTMODE_actor_use_base_palette | LIGHTMODE_actor_use_colors | LIGHTMODE_room_lights_on;

	if (_game.id == GID_MONKEY)
		_scummVars[74] = 1225;
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::resetScummVars() {
	ScummEngine::resetScummVars();

	if (_game.version != 8) {
		VAR(VAR_V6_EMSSPACE) = 10000;
		VAR(VAR_NUM_GLOBAL_OBJS) = _numGlobalObjects - 1;
	}

	VAR(VAR_DEFAULT_TALK_DELAY) = 60;
}

void ScummEngine_v8::resetScummVars() {
	ScummEngine_v7::resetScummVars();

	// FIXME: How do we deal with non-cd installs?
	VAR(VAR_CURRENTDISK) = 1;

	switch (_language) {
	case Common::EN_ANY:
	case Common::EN_USA:
	case Common::EN_GRB:
		VAR(VAR_LANGUAGE) = 0;
		break;
	case Common::DE_DEU:
		VAR(VAR_LANGUAGE) = 1;
		break;
	case Common::FR_FRA:
		VAR(VAR_LANGUAGE) = 2;
		break;
	case Common::IT_ITA:
		VAR(VAR_LANGUAGE) = 3;
		break;
	case Common::PT_BRA:
		VAR(VAR_LANGUAGE) = 4;
		break;
	case Common::ES_ESP:
		VAR(VAR_LANGUAGE) = 5;
		break;
	case Common::JA_JPN:
		VAR(VAR_LANGUAGE) = 6;
		break;
	case Common::ZH_TWN:
		VAR(VAR_LANGUAGE) = 7;
		break;
	case Common::KO_KOR:
		VAR(VAR_LANGUAGE) = 8;
		break;
	default:
		VAR(VAR_LANGUAGE) = 0;	// Default to english
	}
}
#endif

void ScummEngine_v70he::resetScummVars() {
	ScummEngine::resetScummVars();

	if (VAR_MACHINE_SPEED != 0xFF)
		VAR(VAR_MACHINE_SPEED) = 13;

	VAR(VAR_NUM_SOUND_CHANNELS) = 8;
	VAR(VAR_SOUND_CHANNEL) = 1;
	VAR(VAR_TALK_CHANNEL) = 2;
}

#ifdef ENABLE_HE
void ScummEngine_v72he::resetScummVars() {
	ScummEngine_v70he::resetScummVars();

	VAR(VAR_VIDEO_PERFORMANCE) = 26;

	VAR(VAR_NUM_ROOMS) = _numRooms - 1;
	VAR(VAR_NUM_SCRIPTS) = _numScripts - 1;
	VAR(VAR_NUM_SOUNDS) = _numSounds - 1;
	VAR(VAR_NUM_COSTUMES) = _numCostumes - 1;
	VAR(VAR_NUM_IMAGES) = _numImages - 1;
	VAR(VAR_NUM_CHARSETS) = _numCharsets - 1;
	VAR(VAR_NUM_GLOBAL_OBJS) = _numGlobalObjects - 1;

	if (_game.heversion <= 74) {
		// Songs are disabled, if sound is disabled.
		VAR(VAR_SOUND_ENABLED) = 1;
	}

	if (_game.heversion == 74) {
		// Uses different values, compared to later HE80+ games.
		if (_game.platform == Common::kPlatformMacintosh) {
			VAR(VAR_PLATFORM) = 3;
		} else {
			VAR(VAR_PLATFORM) = 2;
		}
	}
}

void ScummEngine_v80he::resetScummVars() {
	ScummEngine_v72he::resetScummVars();

	if (_game.platform == Common::kPlatformMacintosh) {
		VAR(VAR_PLATFORM) = 2;
		VAR(VAR_PLATFORM_VERSION) = 70;
	} else {
		VAR(VAR_PLATFORM) = 1;
		VAR(VAR_PLATFORM_VERSION) = 40;		// 31 is Windows 3.1, 40 is Windows 95+
	}
	VAR(VAR_COLOR_DEPTH) = 256;
}

void ScummEngine_v90he::resetScummVars() {
	ScummEngine_v80he::resetScummVars();

	VAR(VAR_SCRIPT_CYCLE) = 1;
	VAR(VAR_NUM_SCRIPT_CYCLES) = 1;

	if (_game.heversion >= 95) {
		VAR(VAR_NUM_SPRITE_GROUPS) = MAX(64, _numSprites / 4) - 1;
		VAR(VAR_NUM_SPRITES) = _numSprites - 1;
		VAR(VAR_WIZ_TCOLOR) = 5;
		VAR(VAR_RESERVED_SOUND_CHANNELS) = 9;
	}
	if (_game.heversion >= 98) {
		VAR(VAR_U32_VERSION) = _logicHE->versionID();
		VAR(VAR_U32_ARRAY_UNK) = 0;
	}
}

void ScummEngine_v99he::resetScummVars() {
	ScummEngine_v90he::resetScummVars();

	VAR(VAR_NUM_PALETTES) = _numPalettes;
	VAR(VAR_NUM_UNK) = _numUnk;

	if (_game.heversion >= 100 && (_game.features & GF_16BIT_COLOR)) {
		// Enable Bink video in 16bit color games
#ifdef USE_BINK
		VAR(140) = 1;
#else
		VAR(140) = 0;
#endif
	}
}
#endif

void ScummEngine::resetScummVars() {
	if (_game.heversion < 70 && _game.version <= 6) {
		// VAR_SOUNDCARD modes
		// 0 PC Speaker
		// 1 Tandy
		// 2 CMS
		// 3 AdLib
		// 4 Roland
		switch (_sound->_musicType) {
		case MDT_NONE:
		case MDT_PCSPK:
			VAR(VAR_SOUNDCARD) = 0;
			break;
		case MDT_PCJR:
			VAR(VAR_SOUNDCARD) = 1;
			break;
		case MDT_CMS:
			VAR(VAR_SOUNDCARD) = 2;
			break;
		case MDT_ADLIB:
			VAR(VAR_SOUNDCARD) = 3;
			break;
		default:
			if ((_game.id == GID_MONKEY_EGA || _game.id == GID_MONKEY_VGA || (_game.id == GID_LOOM && _game.version == 3))
			   &&  (_game.platform == Common::kPlatformPC)) {
				VAR(VAR_SOUNDCARD) = 4;
			} else {
				VAR(VAR_SOUNDCARD) = 3;
			}
			break;
		}

		if (_game.platform == Common::kPlatformFMTowns)
			VAR(VAR_VIDEOMODE) = 42;
		// Value only used by the Macintosh version of Indiana Jones and the Last Crusade
		else if (_game.platform == Common::kPlatformMacintosh && _game.version == 3)
			VAR(VAR_VIDEOMODE) = 50;
		// Value only used by the Amiga version of Monkey Island 2
		else if (_game.platform == Common::kPlatformAmiga)
			VAR(VAR_VIDEOMODE) = 82;
		else if (_renderMode == Common::kRenderCGA)
			VAR(VAR_VIDEOMODE) = 4;
		else if (_renderMode == Common::kRenderHercA || _renderMode == Common::kRenderHercG)
			VAR(VAR_VIDEOMODE) = 30;
		else if (_renderMode == Common::kRenderEGA)
			VAR(VAR_VIDEOMODE) = 13;
		else
			VAR(VAR_VIDEOMODE) = 19;

		if (_game.platform == Common::kPlatformMacintosh && (_game.features & GF_OLD_BUNDLE)) {
			// Set screen size for the Macintosh version of Indy3/Loom
			VAR(39) = 320;
		}
		if (_game.platform == Common::kPlatformPC && _game.id == GID_LOOM && _game.version == 3) {
			// Set number of sound resources
			VAR(39) = 80;
		}

		if (_game.id == GID_LOOM || _game.version >= 4)
			VAR(VAR_HEAPSPACE) = 1400;
		if (_game.version >= 4)
			VAR(VAR_FIXEDDISK) = 1;
		if (_game.version >= 5)
			VAR(VAR_INPUTMODE) = 3;
		if (_game.version == 6)
			VAR(VAR_V6_EMSSPACE) = 10000;

		if (_game.heversion >= 60) {
			// Set fast speed, to enable all animations
			VAR(VAR_MACHINE_SPEED) = 2;

			VAR(VAR_SOUNDPARAM) = 1;  // Soundblaster for music
			VAR(VAR_SOUNDPARAM2) = 1; // Soundblaster for sound effects
		}
	}

	if (VAR_ROOM_WIDTH != 0xFF && VAR_ROOM_HEIGHT != 0xFF) {
		VAR(VAR_ROOM_WIDTH) = _screenWidth;
		VAR(VAR_ROOM_HEIGHT) = _screenHeight;
	}

	if (VAR_DEBUGMODE != 0xFF) {
		VAR(VAR_DEBUGMODE) = _debugMode;
		if (_game.heversion >= 80 && _debugMode)
			VAR(85) = 1;
	}

	if (VAR_FADE_DELAY != 0xFF)
		VAR(VAR_FADE_DELAY) = 3;

	VAR(VAR_CHARINC) = 4;
	setTalkingActor(0);
}

} // End of namespace Scumm
