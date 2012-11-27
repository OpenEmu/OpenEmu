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

#include "tsage/staticres.h"

namespace TsAGE {

const byte CURSOR_ARROW_DATA[] = {
	15, 0, 15, 0, 0, 0, 0, 0, 9, 0,
	0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF,	0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0xFF, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,	0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,	0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09
};

const byte CURSOR_WALK_DATA[] = {
	15, 0, 15, 0, 7, 0, 7, 0, 9, 0,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09,	0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x09,	0x09, 0x09, 0x09, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x09, 0x09, 0x09, 0x09, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x09,	0x09, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09,	0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09,	0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00, 0xFF, 0x00, 0x09, 0x09, 0x09, 0x09, 0x09,	0x09,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09
};

char const *const LOOK_SCENE_HOTSPOT = "You see nothing special.";
char const *const USE_SCENE_HOTSPOT = "That accomplishes nothing.";
char const *const TALK_SCENE_HOTSPOT = "Yak, yak.";
char const *const SPECIAL_SCENE_HOTSPOT = "That is a unique use for that.";
char const *const DEFAULT_SCENE_HOTSPOT = "That accomplishes nothing.";
char const *const SAVE_ERROR_MSG = "Error occurred saving game. Please do not try to restore this game!";
char const *const SAVING_NOT_ALLOWED_MSG = "Saving is not allowed at this time.";
char const *const RESTORING_NOT_ALLOWED_MSG = "Restoring is not allowed at this time.";
char const *const INV_EMPTY_MSG = "You have nothing in your possesion.";

char const *const QUIT_CONFIRM_MSG = "Do you want to quit playing this game?";
char const *const RESTART_MSG = "Do you want to restart this game?";
char const *const GAME_PAUSED_MSG = "Game is paused.";
char const *const OK_BTN_STRING = " Ok ";
char const *const CANCEL_BTN_STRING = "Cancel";
char const *const QUIT_BTN_STRING = " Quit ";
char const *const RESTART_BTN_STRING = "Restart";
char const *const SAVE_BTN_STRING = "Save";
char const *const RESTORE_BTN_STRING = "Restore";
char const *const SOUND_BTN_STRING = "Sound";
char const *const RESUME_BTN_STRING = " Resume \rplay";
char const *const LOOK_BTN_STRING = "Look";
char const *const PICK_BTN_STRING = "Pick";


namespace Ringworld {

// Dialog resources
char const *const HELP_MSG = "Ringworld\rRevenge of the Patriarch\x14\rScummVM Version\r\r\
\x01 Keyboard shortcuts...\rF2 - Sound options\rF3 - Quit\r\
F4 - Restart\rF5 - Save game\rF7 - Restore Game\rF10 - Pause game";
char const *const WATCH_INTRO_MSG = "Do you wish to watch the introduction?";
char const *const START_PLAY_BTN_STRING = " Start Play ";
char const *const INTRODUCTION_BTN_STRING = "Introduction";
char const *const OPTIONS_MSG = "\x01Options...";

// Scene specific resources
char const *const EXIT_MSG = "   EXIT   ";
char const *const SCENE6100_CAREFUL = "Be careful! The probe cannot handle too much of that.";
char const *const SCENE6100_TOUGHER = "Hey! This is tougher than it looks!";
char const *const SCENE6100_ONE_MORE_HIT = "You had better be more careful. One more hit like that \
and the probe may be destroyed.";
char const *const SCENE6100_DOING_BEST = "I'm doing the best I can. I just hope it holds together!";
char const *const SCENE6100_REPAIR = "\r\rQuinn and Seeker repair the probe....";
char const *const SCENE6100_ROCKY_AREA = "The rocky area should be directly ahead of you. Do you see it?";
char const *const SCENE6100_REPLY = "Yes. Now if I can just avoid those sunbeams.";
char const *const SCENE6100_TAKE_CONTROLS = "You had better take the controls Seeker. My hands are sweating.";
char const *const SCENE6100_SURPRISE = "You surprise me Quinn. I would have thought you of hardier stock.";
char const *const SCENE6100_SWEAT = "Humans sweat, Kzin twitch their tail. What's the difference?";
char const *const SCENE6100_VERY_WELL = "Very well. I will retrieve the stasis box and return the probe. \
Wait for it's return in the lander bay.";

char const *const DEMO_HELP_MSG = " Help...\rF2 - Sound Options\rF3 - Exit demo\r\rPress ENTER\rto continue";
char const *const DEMO_PAUSED_MSG = "Ringworld\x14 demo is paused";
char const *const DEMO_EXIT_MSG = "Press ENTER to resume the Ringworld\x14 demo. Press ESC to exit";
char const *const EXIT_BTN_STRING = "Exit";
char const *const DEMO_BTN_STRING = "Demo";
char const *const DEMO_RESUME_BTN_STRING = "Resume";

} // End of namespace Ringworld

namespace BlueForce {

// Dialog resources
char const *const HELP_MSG = "Blue Force\x14\rScummVM Version\r\r\
Keyboard shortcuts...\rF2 - Sound options\rF3 - Quit\r\
F4 - Restart\rF5 - Save game\rF7 - Restore Game\rF10 - Pause game";
char const *const WATCH_INTRO_MSG = "Do you wish to watch the introduction?";
char const *const START_PLAY_BTN_STRING = " Play ";
char const *const INTRODUCTION_BTN_STRING = " Watch ";
char const *const OPTIONS_MSG = "Options...";

// Blue Force general messages
char const *const BF_NAME = "Blue Force";
char const *const BF_COPYRIGHT = " Copyright, 1993 Tsunami Media, Inc.";
char const *const BF_ALL_RIGHTS_RESERVED = "All Rights Reserved";
char const *const BF_19840518 = "May 18, 1984";
char const *const BF_19840515 = "May 15, 1984";
char const *const BF_3_DAYS = "Three days later";
char const *const BF_11_YEARS = "Eleven years later.";
char const *const BF_NEXT_DAY = "The Next Day";
char const *const BF_ACADEMY = "Here we are at the Academy";

// Scene 50 hotspots
char const *const GRANDMA_FRANNIE = "Grandma Frannie";
char const *const MARINA = "Marina";
char const *const POLICE_DEPARTMENT = "Police Department";
char const *const TONYS_BAR = "Tony's Bar";
char const *const CHILD_PROTECTIVE_SERVICES = "Child Protective Services";
char const *const ALLEY_CAT = "Alley Cat";
char const *const CITY_HALL_JAIL = "City Hall & Jail";
char const *const JAMISON_RYAN = "Jamison & Ryan";
char const *const BIKINI_HUT = "Bikini Hut";

// Scene 60 radio messages
char const *const RADIO_BTN_LIST[8] = { "10-2 ", "10-4 ", "10-13", "10-15", "10-27", "10-35", "10-97", "10-98" };

// Scene 570 computer messageS
char const *const SCENE570_PASSWORD = "PASSWORD -> ";
char const *const SCENE570_C_DRIVE = "C:\\";
char const *const SCENE570_RING = "RING";
char const *const SCENE570_PROTO = "PROTO";
char const *const SCENE570_WACKY = "WACKY";
char const *const SCENE570_COBB = "COBB";
char const *const SCENE570_LETTER = "LETTER";
char const *const SCENE570_RINGEXE = "RINGEXE";
char const *const SCENE570_RINGDATA = "RINGDATA";
char const *const SCENE570_PROTOEXE = "PROTOEXE";
char const *const SCENE570_PROTODATA = "PROTODATA";
char const *const SCENE570_WACKYEXE = "WACKYEXE";
char const *const SCENE570_WACKYDATA = "WACKYDATA";

// Scene 180 messages
char const *const THE_NEXT_DAY = "The Next Day";

} // End of namespace BlueForce

namespace Ringworld2 {

char const *const CONSOLE125_MESSAGES[] = {
	NULL, "Select Language", "Computer Services", "Food Services", "Entertainment Services",
	"Main Menu", "Exit Menu", "Previous Menu", "Interworld", "Hero's Tongue", "Personal Log",
	"Database", "Starchart", "Iso-Opto Disk Reader", "Eject Disk", "Meals", "Snacks",
	"Beverages", "Human Basic Snacks", "Kzin Basic Snacks", "Hot Beverages", "Cold Beverages",
	"Music", "Outpost Alpha", " ", " ", "Done", "A-G", "H-O", "P-S", "T-Z", "Tchaikovsky",
	"Mozart", "Bach", "Rossini"
};

char const *const CONSOLE325_MESSAGES[] = {
	NULL, "Select Language", "Database", "Star Chart", "Scan Ops", "Deep Scan",
	"Short Scan", "Main Menu", "Previous Menu", "Exit Menu", "Interworld", "Hero's Tongue",
	"Done", " ", " ", "Passive Enabled", "Active Enabled", "Technological", "Biological",
	"Geographical", "Astronomical", "Dipole Anomaly Sweep", "Structural Analysis",
	"A-G", "N-O", "P-S", "T-Z", "Tchaikovsky", "Mozart", "Bach", "Rossini"
};

// Scene 825 Autodoc messages
char const *const MAIN_MENU = "main menu";
char const *const DIAGNOSIS = "diagnosis";
char const *const ADVANCED_PROCEDURES = "advanced procedures";
char const *const VITAL_SIGNS = "vital signs";
char const *const OPEN_DOOR = "open door";
char const *const TREATMENTS = "treatments";
char const *const NO_MALADY_DETECTED =  "no malady detected";
char const *const NO_TREATMENT_REQUIRED = "no treatment required";
char const *const ACCESS_CODE_REQUIRED = "access code required";
char const *const INVALID_ACCESS_CODE = "invalid access code";
char const *const FOREIGN_OBJECT_EXTRACTED = "foreign object extracted";

char const *const HELP_MSG = "\x1\rRETURN TO\r RINGWORLD\x14";
char const *const CHAR_TITLE = "\x01Select Character:";
char const *const CHAR_QUINN_MSG = "  Quinn  ";
char const *const CHAR_SEEKER_MSG = "  Seeker  ";
char const *const CHAR_MIRANDA_MSG = "Miranda";
char const *const CHAR_CANCEL_MSG = "  Cancel  ";

char const *const GAME_VERSION = "ScummVM Version";
char const *const SOUND_OPTIONS = "Sound options";
char const *const QUIT_GAME = "Quit";
char const *const RESTART_GAME = "Restart";
char const *const SAVE_GAME = "Save game";
char const *const RESTORE_GAME = "Restore game";
char const *const SHOW_CREDITS = "Show credits";
char const *const PAUSE_GAME = "Pause game";
char const *const RESUME_PLAY = "  Resume play  ";
char const *const F2 = "F2";
char const *const F3 = "F3";
char const *const F4 = "F4";
char const *const F5 = "F5";
char const *const F7 = "F7";
char const *const F8 = "F8";
char const *const F10 = "F10";

char const *const DONE_MSG = "Done";
char const *const YES_MSG = " Yes ";
char const *const NO_MSG = " No ";
char const *const USE_INTERCEPTOR = "Do you want to use your interceptor card?";
char const *const USE_DOUBLE_AGENT = "Do you want to use your double agent?";
char const *const NEED_INSTRUCTIONS = "Do you want instructions?";
char const *const WRONG_ANSWER_MSG = "Wrong respond value sent.";
const byte k562CC[] = {
	20, 7,  41, 6,
	3,  6,  42, 11,
	10, 15, 43, 6,
	15, 1,  44, 7,
	1,  1,  2,  1,
	1,  1,  21, 12,
	5,  1,  36, 1,
	5,  1,  28, 2,
	9,  1,  10, 7,
	9,  1,  12, 10,
	19, 1,  10, 8,
	19, 1,  2,  13,
	25, 1,  31, 1,
	27, 1,  15, 6,
	27, 1,  20, 7,
	28, 1,  24, 4,
	6,  2,  22, 1,
	6,  2,  16, 5,
	12, 2,  40, 1,
	12, 2,  6,  11,
	18, 2,  21, 5,
	20, 5,  19, 4,
	20, 5,  18, 8,
	1,  6,  20, 11,
	1,  6,  18, 15,
	1,  6,  16, 4,
	7,  6,  6,  16,
	8,  6,  23, 9,
	8,  6,  38, 10,
	8,  6,  14, 13,
	8,  6,  6,  14,
	8,  6,  11, 15,
	10, 6,  3,  1,
	10, 6,  8,  2,
	10, 6,  13, 3,
	10, 6,  6,  15,
	17, 6,  4,  15,
	5,  7,  26, 11,
	25, 7,  27, 3,
	28, 7,  21, 5,
	2,  8,  23, 5,
	14, 8,  21, 5,
	14, 8,  22, 16,
	22, 8,  34, 3,
	22, 8,  24, 7,
	6,  9,  38, 5,
	6,  9,  32, 6,
	6,  9,  18, 7,
	9,  9,  34, 15,
	9,  9,  35, 16,
	18, 9,  1,  5,
	18, 9,  24, 11,
	26, 9,  21, 10,
	1,  10, 21, 9,
	1,  10, 12, 13,
	11, 10, 21, 16,
	15, 10, 8,  1,
	15, 10, 12, 6,
	15, 10, 14, 10,
	20, 10, 14, 10,
	20, 10, 39, 11,
	3,  11, 5,  9,
	3,  11, 4,  13,
	5,  11, 32, 7,
	5,  11, 20, 8,
	5,  11, 20, 11,
	7,  12, 22, 11,
	7,  12, 2,  12,
	7,  12, 23, 16,
	8,  12, 25, 9,
	13, 12, 23, 10,
	16, 12, 3,  11,
	17, 12, 25, 10,
	17, 12, 28, 15,
	25, 12, 8,  15,
	26, 12, 7,  14,
	1,  13, 8,  6,
	10, 13, 28, 11,
	21, 13, 25, 13,
	21, 13, 24, 16,
	25, 13, 25, 3,
	5,  14, 17, 6,
	5,  14, 23, 16,
	12, 14, 36, 15,
	12, 14, 17, 16,
	13, 14, 40, 13,
	13, 14, 38, 14,
	19, 14, 18, 10,
	2,  15, 4,  14,
	8,  15, 1,  13,
	12, 15, 21, 4,
	12, 15, 27, 7,
	12, 15, 28, 8,
	13, 15, 34, 1,
	13, 15, 31, 5,
	14, 15, 21, 16,
	15, 15, 29, 1,
	15, 15, 23, 13,
	25, 15, 24, 6,
	25, 15, 23, 11,
	28, 15, 8,  6,
	28, 15, 9,  10,
	1,  16, 1,  1,
	1,  16, 22, 5,
	7,  16, 3,  4,
	8,  16, 7,  1,
	11, 17, 4,  12,
	18, 17, 1,  13,
	22, 17, 21, 6,
	22, 17, 28, 15,
	27, 17, 12, 7,
	27, 17, 8,  8,
	27, 17, 14, 11,
	27, 17, 18, 15,
	27, 17, 2,  16,
	6,  18, 24, 7,
	14, 18, 21, 13,
	27, 18, 38, 4,
	28, 18, 20, 1,
	1,  18, 11, 15,
	9,  18, 7,  1,
	9,  18, 13, 12,
	16, 18, 32, 10,
	16, 18, 25, 13,
	16, 18, 31, 14,
	25, 18, 20, 7,
	28, 18, 21, 1
};

const byte k5A4D6[] = {
	2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  3,
	16, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  18,
	17, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  19,
	7,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  8,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	6,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  10, 0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	11, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  12,
	13, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14
};

const byte k5A72E[] = {0,  98, 135, 183, 229, 81, 133, 185, 235, 75, 131, 187, 241, 70,  129, 190, 247};
const byte k5A73F[] = {0,  42, 42,  42,  42,  67, 67,  67,  67,  92, 92,  92,  92,  116, 116, 116, 116};
const byte k5A750[] = {
	9, 10, 7, 13, 7, 8, 9,  7, 9, 10,
	2, 3,  3, 2,  2, 2, 4,  3, 3, 4,
	3, 2,  3, 4,  3, 8, 10, 4, 0
};
const byte k5A76D[] = {
	3, 3,  3, 4,  3, 3, 3,  3, 1, 3,
	3, 3,  3, 7,  3, 7, 3,  3, 3, 3,
	3, 3,  3, 3,  3, 3, 3,  3, 3
};
const byte k5A78A[] = {0,  8,  15,  16,  12,  7,  18,  17,  13,  6,  19,  20,  14,  5,   11,  10,  9};
const byte k5A79B[] = {
	23, 3,  1,
	23, 4,  1,
	26, 3,  1,
	26, 4,  1,
	15, 16, 2,
	2,  16, 3,
	3,  16, 4,
	10, 7,  5,
	11, 7,  6,
	3,  0,  7,
	4,  0,  8,
	24, 11, 9,
	25, 11, 10,
	12, 8,  11,
	9,  11, 12
};

const byte k5A7F6[] = {
	4,  11, 6,
	5,  14, 5,
	5,  11, 6,
	5,  7,  5,
	6,  9,  6,
	7,  16, 7,
	7,  12, 7,
	8,  6,  5,
	9,  9,  7,
	10, 13, 9,
	10, 6,  8,
	11, 10, 9,
	12, 15, 10,
	13, 12, 11,
	14, 8,  9,
	15, 16, 1,
	15, 10, 11
};

} // End of namespace Ringworld2

} // End of namespace TsAGE
