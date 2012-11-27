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

#ifndef TSAGE_STATICRES_H
#define TSAGE_STATICRES_H

#include "common/scummsys.h"

namespace TsAGE {

extern const byte CURSOR_ARROW_DATA[];

extern const byte CURSOR_WALK_DATA[];

extern char const *const LOOK_SCENE_HOTSPOT;
extern char const *const USE_SCENE_HOTSPOT;
extern char const *const TALK_SCENE_HOTSPOT;
extern char const *const SPECIAL_SCENE_HOTSPOT;
extern char const *const DEFAULT_SCENE_HOTSPOT;
extern char const *const SAVE_ERROR_MSG;
extern char const *const SAVING_NOT_ALLOWED_MSG;
extern char const *const RESTORING_NOT_ALLOWED_MSG;

// Dialogs
extern char const *const QUIT_CONFIRM_MSG;
extern char const *const RESTART_MSG;
extern char const *const GAME_PAUSED_MSG;
extern char const *const OK_BTN_STRING;
extern char const *const CANCEL_BTN_STRING;
extern char const *const QUIT_BTN_STRING;
extern char const *const RESTART_BTN_STRING;
extern char const *const SAVE_BTN_STRING;
extern char const *const RESTORE_BTN_STRING;
extern char const *const SOUND_BTN_STRING;
extern char const *const RESUME_BTN_STRING;
extern char const *const LOOK_BTN_STRING;
extern char const *const PICK_BTN_STRING;
extern char const *const INV_EMPTY_MSG;

namespace Ringworld {

// Dialog resources
extern char const *const HELP_MSG;
extern char const *const WATCH_INTRO_MSG;
extern char const *const START_PLAY_BTN_STRING;
extern char const *const INTRODUCTION_BTN_STRING;
extern char const *const OPTIONS_MSG;

// Scene specific resources
extern char const *const EXIT_MSG;
extern char const *const SCENE6100_CAREFUL;
extern char const *const SCENE6100_TOUGHER;
extern char const *const SCENE6100_ONE_MORE_HIT;
extern char const *const SCENE6100_DOING_BEST;
extern char const *const SCENE6100_REPAIR;
extern char const *const SCENE6100_ROCKY_AREA;
extern char const *const SCENE6100_REPLY;
extern char const *const SCENE6100_TAKE_CONTROLS;
extern char const *const SCENE6100_SURPRISE;
extern char const *const SCENE6100_SWEAT;
extern char const *const SCENE6100_VERY_WELL;

// Demo messages
extern char const *const DEMO_HELP_MSG;
extern char const *const DEMO_PAUSED_MSG;
extern char const *const DEMO_HELP_MSG;
extern char const *const DEMO_PAUSED_MSG;
extern char const *const DEMO_EXIT_MSG;
extern char const *const EXIT_BTN_STRING;
extern char const *const DEMO_BTN_STRING;
extern char const *const DEMO_RESUME_BTN_STRING;

} // End of namespace Ringworld

namespace BlueForce {

// Dialog resources
extern char const *const HELP_MSG;
extern char const *const WATCH_INTRO_MSG;
extern char const *const START_PLAY_BTN_STRING;
extern char const *const INTRODUCTION_BTN_STRING;
extern char const *const OPTIONS_MSG;

// Blue Force messages
extern char const *const BF_NAME;
extern char const *const BF_COPYRIGHT;
extern char const *const BF_ALL_RIGHTS_RESERVED;
extern char const *const BF_19840518;
extern char const *const BF_19840515;
extern char const *const BF_3_DAYS;
extern char const *const BF_11_YEARS;
extern char const *const BF_NEXT_DAY;
extern char const *const BF_ACADEMY;

// Scene 50 tooltips
extern char const *const GRANDMA_FRANNIE;
extern char const *const MARINA;
extern char const *const POLICE_DEPARTMENT;
extern char const *const TONYS_BAR;
extern char const *const CHILD_PROTECTIVE_SERVICES;
extern char const *const ALLEY_CAT;
extern char const *const CITY_HALL_JAIL;
extern char const *const JAMISON_RYAN;
extern char const *const BIKINI_HUT;

extern char const *const SCENE570_PASSWORD;
extern char const *const SCENE570_C_DRIVE;
extern char const *const SCENE570_RING;
extern char const *const SCENE570_PROTO;
extern char const *const SCENE570_WACKY;
extern char const *const SCENE570_COBB;
extern char const *const SCENE570_LETTER;
extern char const *const SCENE570_RINGEXE;
extern char const *const SCENE570_RINGDATA;
extern char const *const SCENE570_PROTOEXE;
extern char const *const SCENE570_PROTODATA;
extern char const *const SCENE570_WACKYEXE;
extern char const *const SCENE570_WACKYDATA;

// Scene 60 radio dispatch buttons
extern char const *const RADIO_BTN_LIST[8];

// Scene 180 message
extern char const *const THE_NEXT_DAY;

} // End of namespace BlueForce

namespace Ringworld2 {

// Scene 125 - Console messages
extern char const *const CONSOLE125_MESSAGES[];

// Scene 325 - Console messages
extern char const *const CONSOLE325_MESSAGES[];

// Scene 825 - Autodoc Messages
extern char const *const MAIN_MENU;
extern char const *const DIAGNOSIS;
extern char const *const ADVANCED_PROCEDURES;
extern char const *const VITAL_SIGNS;
extern char const *const OPEN_DOOR;
extern char const *const TREATMENTS;
extern char const *const NO_MALADY_DETECTED;
extern char const *const NO_TREATMENT_REQUIRED;
extern char const *const ACCESS_CODE_REQUIRED;
extern char const *const INVALID_ACCESS_CODE;
extern char const *const FOREIGN_OBJECT_EXTRACTED;

// Dialog messages
extern char const *const HELP_MSG;
extern char const *const CHAR_TITLE;
extern char const *const CHAR_QUINN_MSG;
extern char const *const CHAR_SEEKER_MSG;
extern char const *const CHAR_MIRANDA_MSG;
extern char const *const CHAR_CANCEL_MSG;

extern char const *const GAME_VERSION;
extern char const *const SOUND_OPTIONS;
extern char const *const QUIT_GAME;
extern char const *const RESTART_GAME;
extern char const *const SAVE_GAME;
extern char const *const RESTORE_GAME;
extern char const *const SHOW_CREDITS;
extern char const *const PAUSE_GAME;
extern char const *const RESUME_PLAY;
extern char const *const F2;
extern char const *const F3;
extern char const *const F4;
extern char const *const F5;
extern char const *const F7;
extern char const *const F8;
extern char const *const F10;

extern char const *const DONE_MSG;
extern char const *const YES_MSG;
extern char const *const NO_MSG;
extern char const *const USE_INTERCEPTOR;
extern char const *const USE_DOUBLE_AGENT;
extern char const *const NEED_INSTRUCTIONS;
extern char const *const WRONG_ANSWER_MSG;

// Scene 1550 arrays of constants
extern const byte k562CC[];
extern const byte k5A4D6[];
extern const byte k5A72E[];
extern const byte k5A73F[];
extern const byte k5A750[];
extern const byte k5A76D[];
extern const byte k5A78A[];
extern const byte k5A79B[];
extern const byte k5A7F6[];

} // End of namespace Ringworld2

} // End of namespace TsAGE

#endif
