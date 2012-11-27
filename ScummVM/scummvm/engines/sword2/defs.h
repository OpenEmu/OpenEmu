/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef	SWORD2_DEFS_H
#define	SWORD2_DEFS_H

#define	SIZE	0x10000			// 65536 items per section
#define	NuSIZE	0xffff			// & with this

// Return codes

enum {
	// Generic error codes
	RD_OK				= 0x00000000,
	RDERR_UNKNOWN			= 0x00000001,
	RDERR_OUTOFMEMORY		= 0x00000003,
	RDERR_INVALIDFILENAME		= 0x00000004,

	// Drawing error codes
	RDERR_DECOMPRESSION		= 0x00010007,

	// Sprite drawing error codes
	RDERR_NOTIMPLEMENTED		= 0x00060001,
	RDERR_NOTCLOSED			= 0x00050005,
	RDERR_NOTOPEN			= 0x00050006,

	// Menubar error codes
	RDERR_INVALIDMENU		= 0x00060000,
	RDERR_INVALIDPOCKET		= 0x00060001,
	RDERR_INVALIDCOMMAND		= 0x00060002,

	// Palette fading error codes
	RDERR_FADEINCOMPLETE		= 0x00070000,

	// Sound engine error codes
	RDERR_SPEECHPLAYING		= 0x00080004,
	RDERR_SPEECHNOTPLAYING		= 0x00080005,
	RDERR_INVALIDWAV		= 0x00080006,
	RDERR_FXALREADYOPEN		= 0x00080009,
	RDERR_FXNOTOPEN			= 0x0008000B,
	RDERR_INVALIDID			= 0x0008000D
};

// Text ids for the control panel etc.

enum {
	TEXT_OK				= 0x08EB0000,
	TEXT_CANCEL			= 0x08EB0001,
	TEXT_RESTORE			= 0x08EB0002,
	TEXT_SAVE			= 0x08EB0003,
	TEXT_QUIT			= 0x08EB0004,
	TEXT_RESTART			= 0x08EB0005,
	TEXT_OPTIONS			= 0x08EB000A,
	TEXT_SUBTITLES			= 0x08EB000B,
	TEXT_OBJECT_LABELS		= 0x08EB000C,
	TEXT_MUSIC_VOLUME		= 0x08EB000E,
	TEXT_SPEECH_VOLUME		= 0x08EB000F,
	TEXT_FX_VOLUME			= 0x08EB0010,
	TEXT_GFX_QUALITY		= 0x08EB0011,
	TEXT_REVERSE_STEREO		= 0x08EB0015,
	TEXT_RESTORE_CANT_OPEN		= 0x0CBA017E,
	TEXT_RESTORE_INCOMPATIBLE	= 0x0CBA017F,
	TEXT_RESTORE_FAILED		= 0x0CBA0181,
	TEXT_SAVE_CANT_OPEN		= 0x0CBA0182,
	TEXT_SAVE_FAILED		= 0x0CBA0184
};

// Always 8 (George object used for Nico player character as well)
#define CUR_PLAYER_ID 8

// Global variable references

enum {
	ID				= 0,
	RESULT				= 1,
	PLAYER_ACTION			= 2,
	// CUR_PLAYER_ID		= 3,
	PLAYER_ID			= 305,
	TALK_FLAG			= 13,

	MOUSE_X				= 4,
	MOUSE_Y				= 5,
	LEFT_BUTTON			= 109,
	RIGHT_BUTTON			= 110,
	CLICKED_ID			= 178,

	IN_SUBJECT			= 6,
	COMBINE_BASE			= 7,
	OBJECT_HELD			= 14,

	SPEECH_ID			= 9,
	INS1				= 10,
	INS2				= 11,
	INS3				= 12,
	INS4				= 60,
	INS5				= 61,
	INS_COMMAND			= 59,

	PLAYER_FEET_X			= 141,
	PLAYER_FEET_Y			= 142,
	PLAYER_CUR_DIR			= 937,

	// for debug.cpp
	LOCATION			= 62,

	// so scripts can force scroll offsets
	SCROLL_X			= 345,
	SCROLL_Y			= 346,

	EXIT_CLICK_ID			= 710,
	EXIT_FADING			= 713,

	SYSTEM_TESTING_ANIMS		= 912,
	SYSTEM_TESTING_TEXT		= 1230,
	SYSTEM_WANT_PREVIOUS_LINE	= 1245,

	// 1=on 0=off (set in fnAddHuman and fnNoHuman)
	MOUSE_AVAILABLE			= 686,

	// used in fnChoose
	AUTO_SELECTED			= 1115,

	// see fnStartConversation and fnChooser
	CHOOSER_COUNT_FLAG		= 15,

	// signifies a demo mode
	DEMO				= 1153,

	// Indicates to script whether this is the Playstation version.
	// PSXFLAG			= 1173,

	// for the poor PSX so it knows what language is running.
	// GAME_LANGUAGE		= 111,

	// 1 = dead
	DEAD				= 1256,

	// If set indicates that the speech anim is to run through only once.
	SPEECHANIMFLAG			= 1278,

	// for the engine
	SCROLL_OFFSET_X			= 1314
};

// Resource IDs

enum {
	// mouse mointers - It's pretty much safe to do it like this
	NORMAL_MOUSE_ID			= 17,
	SCROLL_LEFT_MOUSE_ID		= 1440,
	SCROLL_RIGHT_MOUSE_ID		= 1441,

	// Console Font - does not use game text - only English required
	CONSOLE_FONT_ID			= 340,

	// Speech Font
	ENGLISH_SPEECH_FONT_ID		= 341,
	FINNISH_SPEECH_FONT_ID		= 956,
	POLISH_SPEECH_FONT_ID		= 955,

	// Control Panel Font (and un-selected savegame descriptions)
	ENGLISH_CONTROLS_FONT_ID	= 2005,
	FINNISH_CONTROLS_FONT_ID	= 959,
	POLISH_CONTROLS_FONT_ID		= 3686,

	// Red Font (for selected savegame descriptions)
	// BS2 doesn't draw selected savegames in red, so I guess this is a
	// left-over from BS1
	ENGLISH_RED_FONT_ID		= 2005,		// 1998	// Redfont
	FINNISH_RED_FONT_ID		= 959,		// 960	// FinRedFn
	POLISH_RED_FONT_ID		= 3686,		// 3688	// PolRedFn

	// Control panel palette resource id
	CONTROL_PANEL_PALETTE		= 261,

	// res id's of the system menu icons
	OPTIONS_ICON			= 344,
	QUIT_ICON			= 335,
	SAVE_ICON			= 366,
	RESTORE_ICON			= 364,
	RESTART_ICON			= 342,

	// conversation exit icon, 'EXIT' menu icon (used in fnChoose)
	EXIT_ICON			= 65
};

#endif
