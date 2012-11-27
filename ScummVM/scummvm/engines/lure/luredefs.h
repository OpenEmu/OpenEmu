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

#ifndef LUREDEFS_H
#define LUREDEFS_H


#include "common/scummsys.h"
#include "common/list.h"

namespace Lure {

#define SUPPORT_FILENAME "lure.dat"
#define LURE_DAT_MAJOR 1
#define LURE_DAT_MINOR 29
#define LURE_MIN_SAVEGAME_MINOR 25
#define LURE_SAVEGAME_MINOR 33

#define LURE_DEBUG 1

#define READ_LE_INT16(x) (int16) READ_LE_UINT16(x)
#define READ_LE_INT32(x) (int32) READ_LE_UINT32(x)

enum {
	kLureDebugScripts = 1 << 0,
	kLureDebugAnimations = 1 << 1,
	kLureDebugHotspots = 1 << 2,
	kLureDebugFights = 1 << 3,
	kLureDebugSounds = 1 << 4,
	kLureDebugStrings = 1 << 5
};

#define ERROR_BASIC 1
#define ERROR_INTERMEDIATE 2
#define ERROR_DETAILED 3

enum {
	GF_FLOPPY	= 1 <<  0,
	GF_EGA		= 1 <<  1,
	GF_LNGUNK	= 1 << 15
};

enum {
	GI_LURE = 0
};

enum Action {
	NONE = 0,
	GET = 1,
	DROP = 0,
	PUSH = 3,
	PULL = 4,
	OPERATE = 5,
	OPEN = 6,
	CLOSE = 7,
	LOCK = 8,
	UNLOCK = 9,
	USE = 10,
	GIVE = 11,
	TALK_TO = 12,
	TELL = 13,
	BUY = 14,
	LOOK = 15,
	LOOK_AT = 16,
	LOOK_THROUGH = 17,
	ASK = 18,
	EAT = 0,
	DRINK = 20,
	STATUS = 21,
	GO_TO = 22,
	RETURN = 23,
	BRIBE = 24,
	EXAMINE = 25,
	NPC_SET_ROOM_AND_OFFSET = 28,
	NPC_TALK_TO_PLAYER = 29,
	NPC_EXEC_SCRIPT = 30,
	NPC_RESET_PAUSED_LIST = 31,
	NPC_SET_RAND_DEST = 32,
	NPC_WALKING_CHECK = 33,
	NPC_SET_SUPPORT_OFFSET = 34,
	NPC_SUPPORT_OFFSET_COND = 35,
	NPC_DISPATCH_ACTION = 36,
	NPC_TALK_NPC_TO_NPC = 37,
	NPC_PAUSE = 38,
	NPC_START_TALKING = 39,
	NPC_JUMP_ADDRESS = 40
};

// Basic game dimensions
#define FULL_SCREEN_WIDTH 320
#define FULL_SCREEN_HEIGHT 200
#define GAME_COLORS 256
#define SCREEN_SIZE (FULL_SCREEN_HEIGHT * FULL_SCREEN_WIDTH)

// Some resources include multiple packed palettes of 64 entries each
#define SUB_PALETTE_SIZE 64
// Palette resources have 220 palette entries
#define RES_PALETTE_ENTRIES 220
// Main working palette size
#define MAIN_PALETTE_SIZE 228
// Palette color increment amouns for palette fade in/outs
#define PALETTE_FADE_INC_SIZE 4

// EGA constants
#define EGA_PALETTE_SIZE 16
#define EGA_NUM_LAYERS 4
#define EGA_PIXELS_PER_BYTE 8

// Palette and animation for Skorl catching player
#define SKORL_CATCH_PALETTE_ID 0x4060
#define SKORL_CATCH_ANIM_ID 0x4061
// Palette and animation for chute animation
#define CHUTE_PALETTE_ID 0x404C
#define CHUTE_ANIM_ID 0x404D
#define CHUTE2_ANIM_ID 0x404f
#define CHUTE3_ANIM_ID 0x4051
// Palette and animation for hiding in barrel
#define BARREL_PALETTE_ID 0xE9F0
#define BARREL_ANIM_ID 0xE9F1
// Endgame animation constants
#define ENDGAME_PALETTE_ID 0xFF00
#define ENDGAME_ANIM_ID 0xFF01
// Miscellaneous resources
#define COPY_PROTECTION_RESOURCE_ID 0x4139
#define CREDITS_RESOURCE_ID 0x7800
#define RESTART_RESOURCE_ID 0x7900

// Specifies the maximum buffer sized allocated for decoding animation data
#define MAX_ANIM_DECODER_BUFFER_SIZE 300000

#define MAX_DESC_SIZE 1024
#define MAX_HOTSPOT_NAME_SIZE 80
#define MAX_ACTION_NAME_SIZE 15

// Menubar constants
#define MENUBAR_Y_SIZE 8

// Cursor definitions
#define CURSOR_WIDTH 16
#define CURSOR_HEIGHT 16
#define CURSOR_SIZE 256
#define CURSOR_RESOURCE_ID 1

enum CursorType {CURSOR_ARROW = 0, CURSOR_DISK = 1, CURSOR_TIME_START = 2,
	CURSOR_TIME_END = 9, CURSOR_CROSS = 10, CURSOR_UP_ARROW = 11, CURSOR_DOWN_ARROW = 12,
	CURSOR_LEFT_ARROW = 13, CURSOR_RIGHT_ARROW = 14, CURSOR_CAMERA = 15, CURSOR_TALK = 16,
	CURSOR_MENUBAR = 17, CURSOR_FIGHT_UPPER = 23, CURSOR_FIGHT_MIDDLE = 24,
	CURSOR_FIGHT_LOWER = 25};

// Font details
#define FONT_RESOURCE_ID 4
#define FONT_WIDTH 8
#define FONT_HEIGHT 8

// Menu constants
#define MENU_RESOURCE_ID 5
#define MENUITEM_NONE 0
#define MENUITEM_CREDITS 1
#define MENUITEM_RESTART_GAME 2
#define MENUITEM_SAVE_GAME 3
#define MENUITEM_RESTORE_GAME 4
#define MENUITEM_QUIT 5
#define MENUITEM_TEXT_SPEED 6
#define MENUITEM_SOUND 7

// Mouse change needed to change an item in a popup menu
#define POPMENU_CHANGE_SENSITIVITY 5

// Dialog related defines
#define EGA_DIALOG_TEXT_COLOR 6
#define EGA_DIALOG_WHITE_COLOR 2
#define	VGA_DIALOG_TEXT_COLOR 0xe2
#define VGA_DIALOG_WHITE_COLOR 0xe3
#define EGA_DIALOG_BG_COLOR 13
#define INFO_DIALOG_X 69
#define INFO_DIALOG_Y 61
#define INFO_DIALOG_WIDTH 191
#define TALK_DIALOG_WIDTH 130
#define TALK_DIALOG_EDGE_SIZE 3
#define TALK_DIALOG_Y 33
#define SAVE_DIALOG_X 69
#define SAVE_DIALOG_Y 28

// Strings defines
#define STRINGS_RESOURCE_ID 0x10
#define STRINGS_2_RESOURCE_ID 0x11
#define STRINGS_3_RESOURCE_ID 0x12
#define STRING_ID_RANGE 0x7d0
#define STRING_ID_UPPER 0xfa0

// Custom resources stored in lure.dat
#define GAME_PALETTE_RESOURCE_ID 0x3f01
#define ALT_PALETTE_RESOURCE_ID 0x3f02
#define DIALOG_RESOURCE_ID 0x3f03
#define TALK_DIALOG_RESOURCE_ID 0x3f04
#define ROOM_DATA_RESOURCE_ID 0x3f05
#define NPC_SCHEDULES_RESOURCE_ID 0x3f06
#define HOTSPOT_DATA_RESOURCE_ID 0x3f07
#define HOTSPOT_OVERRIDE_DATA_RESOURCE_ID 0x3f08
#define ROOM_EXITS_RESOURCE_ID 0x3f09
#define ROOM_EXIT_JOINS_RESOURCE_ID 0x3f0a
#define ANIM_DATA_RESOURCE_ID 0x3f0b
#define SCRIPT_DATA_RESOURCE_ID 0x3f0c
#define SCRIPT2_DATA_RESOURCE_ID 0x3f0d
#define HOTSPOT_SCRIPT_LIST_RESOURCE_ID 0x3f0e
#define	MESSAGES_LIST_RESOURCE_ID 0x3f0f
#define ACTION_LIST_RESOURCE_ID 0x3f10
#define TALK_HEADER_RESOURCE_ID 0x3f11
#define TALK_DATA_RESOURCE_ID 0x3f12
#define ROOM_PATHS_RESOURCE_ID 0x3f13
#define EXIT_COORDINATES_RESOURCE_ID 0x3f14
#define EXIT_HOTSPOT_ID_LIST 0x3f15
#define FIGHT_DATA_RESOURCE_ID 0x3f16
#define STRING_LIST_RESOURCE_ID 0x3f17
#define SOUND_DESC_RESOURCE_ID 0x3f18
#define STRING_DECODER_RESOURCE_ID 0x3f19

// Script constants
#define STARTUP_SCRIPT 0x23FC

// Miscellaneous resources
#define NAMES_RESOURCE_ID 9
#define ROLAND_MAIN_SOUND_RESOURCE_ID 0xC
#define ADLIB_MAIN_SOUND_RESOURCE_ID 0xD
#define ROLAND_INTRO_SOUND_RESOURCE_ID 0x30
#define ADLIB_INTRO_SOUND_RESOURCE_ID 0x31
#define ROLAND_ENDGAME_SOUND_RESOURCE_ID 0xff10

#define NOONE_ID 0x3E7
#define PLAYER_ID 0x3E8
#define RATPOUCH_ID 0x3E9
#define SKORL_ID 0x3EA
#define BLACKSMITH_ID 0x3EB
#define GWEN_ID 0x3EC
#define MALLIN_ID 0x3ED
#define MONK1_ID 0x3EE
#define GOEWIN_ID 0x3EF
#define MONK2_ID 0x3F0
#define WAYNE_ID 0x3f1
#define CASTLE_SKORL_ID 0x3F3
#define FIRST_NONCHARACTER_ID 0x408
#define SACK_ID 0x40D
#define RACK_SERF_ID 0x411
#define PRISONER_ID 0x412
#define SID_ID 0x420
#define OIL_BURNER_ID 0x424
#define TRANSFORM_ID 0x425
#define NELLIE_ID 0x429
#define EWAN_ID 0x436
#define PIG_ID 0x43F
#define SKORL_FIGHTER_ID 0x444
#define START_EXIT_ID 0x2710
#define BOTTLE_HOTSPOT_ID 0x2710
#define CELL_DOOR_HOTSPOT_ID 0x2712
#define BRICKS_ID 0x2714
#define BOOK_ID 0x2723
#define START_NONVISUAL_HOTSPOT_ID 0x7530

// Milliseconds delay between game frames
#define GAME_FRAME_DELAY 80
#define GAME_TICK_DELAY 20

// Tick proc constants
#define NULL_TICK_PROC_ID 1
#define STANDARD_CHARACTER_TICK_PROC 2
#define PLAYER_TICK_PROC_ID 3
#define VOICE_TICK_PROC_ID 4
#define PUZZLED_TICK_PROC_ID 5
#define FOLLOWER_TICK_PROC_2 8
#define JAILOR_TICK_PROC_ID 9
#define STANDARD_ANIM_2_TICK_PROC 11
#define STANDARD_ANIM_TICK_PROC 12
#define GOEWIN_SHOP_TICK_PROC 27
#define TALK_TICK_PROC_ID 37
#define PLAYER_FIGHT_TICK_PROC_ID 39

// String constants
#define TALK_MAGIC_ID 0x424
#define TALK_RESPONSE_MAGIC_ID 0x1092

// Misc constants
#define GENERAL_MAGIC_ID 42
#define VOICE_ANIM_IDX 1
#define PUZZLED_ANIM_IDX 2
#define EXCLAMATION_ANIM_IDX 3
#define DEFAULT_VOLUME 192

// Animation record indexes
#define PLAYER_FIGHT_ANIM_INDEX 10
#define VOICE_ANIM_INDEX 21
#define BLACKSMITH_HAMMERING_ANIM_INDEX 22
#define EWAN_ANIM_INDEX 23
#define EWAN_ALT_ANIM_INDEX 24
#define PLAYER_ANIM_INDEX 32
#define SELENA_ANIM_INDEX 34
#define BLACKSMITH_DEFAULT_ANIM_INDEX 35

#define CONVERSE_COUNTDOWN_SIZE 40
#define IDLE_COUNTDOWN_SIZE 15
#define MAX_TELL_COMMANDS 8
#define MAX_SAVEGAME_SLOTS 10

#define ROOMNUM_VILLAGE_SHOP 35
#define ROOMNUM_CAVE 38
#define ROOMNUM_CELLAR 42
#define ROOMNUM_DINING_HALL 45

// Countdown for # operations in path finder before breaking until next
// tick - set it to 0 if you'd like all pathfinding to be done at once
//#define PATHFIND_COUNTDOWN 4000
#define PATHFIND_COUNTDOWN 0

// Pixel record flags
#define PIXELFLAG_HAS_TABLE 4

// Hotspot flags
#define HOTSPOTFLAG_FOUND 0x80
#define HOTSPOTFLAG_SKIP 0x40
#define HOTSPOTFLAG_MENU_EXCLUSION 0x20
#define HOTSPOTFLAG_ROOM_SPECIFIC 0x10

// Constants used to reference entries in the reworked support data entry lists
#define RETURN_SUPPORT_ID 0x400
#define EXIT_BLOCKED_SUPPORT_ID 0x800
#define JUMP_ADDR_2_SUPPORT_ID 0x1403
#define GOEWIN_CAVE_SUPPORT_ID 0x1800
#define GOEWIN_STANDARD_SUPPORT_ID 0x1C00

// Constants used in animation Serf on the rack
#define RACK_SERF_SCRIPT_ID_1 0x35C
#define RACK_SERF_SCRIPT_ID_2 0x384

} // End of namespace Lure

#endif
