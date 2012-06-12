/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* 
 This file includes parts(thus is modified from) of SDL_keysym.h, whose file header is shown below:

 Everything from VIRTB_KB_UNKNOWN to VIRTB_KB_UNDO should map to SDLK_UNKNOWN to SDLK_KB_UNDO, so the driver
 side can use the raw VIRTB_KB_* values for SDLK_* if less than VIRTB_KB_LAST if using SDL 1.2(if not SDL 1.3),
 but probably really should have a translation table.
*/

/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2006 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#ifndef __MDFN_GIT_VIRTB_H
#define __MDFN_GIT_VIRTB_H

/* What we really want is a mapping of every raw key on the keyboard.
   To support international keyboards, we use the range 0xA1 - 0xFF
   as international virtual keycodes.  We'll follow in the footsteps of X11...
   The names of the keys
 */
 
typedef enum {
	VIRTB_UNKNOWN	= 0,

	/* The keyboard syms have been cleverly chosen to map to ASCII */
	VIRTB_KB_UNKNOWN		= 0,
	VIRTB_KB_FIRST		= 0,
	VIRTB_KB_BACKSPACE		= 8,
	VIRTB_KB_TAB		= 9,
	VIRTB_KB_CLEAR		= 12,
	VIRTB_KB_RETURN		= 13,
	VIRTB_KB_PAUSE		= 19,
	VIRTB_KB_ESCAPE		= 27,
	VIRTB_KB_SPACE		= 32,
	VIRTB_KB_EXCLAIM		= 33,
	VIRTB_KB_QUOTEDBL		= 34,
	VIRTB_KB_HASH		= 35,
	VIRTB_KB_DOLLAR		= 36,
	VIRTB_KB_AMPERSAND		= 38,
	VIRTB_KB_QUOTE		= 39,
	VIRTB_KB_LEFTPAREN		= 40,
	VIRTB_KB_RIGHTPAREN		= 41,
	VIRTB_KB_ASTERISK		= 42,
	VIRTB_KB_PLUS		= 43,
	VIRTB_KB_COMMA		= 44,
	VIRTB_KB_MINUS		= 45,
	VIRTB_KB_PERIOD		= 46,
	VIRTB_KB_SLASH		= 47,
	VIRTB_KB_0			= 48,
	VIRTB_KB_1			= 49,
	VIRTB_KB_2			= 50,
	VIRTB_KB_3			= 51,
	VIRTB_KB_4			= 52,
	VIRTB_KB_5			= 53,
	VIRTB_KB_6			= 54,
	VIRTB_KB_7			= 55,
	VIRTB_KB_8			= 56,
	VIRTB_KB_9			= 57,
	VIRTB_KB_COLON		= 58,
	VIRTB_KB_SEMICOLON		= 59,
	VIRTB_KB_LESS		= 60,
	VIRTB_KB_EQUALS		= 61,
	VIRTB_KB_GREATER		= 62,
	VIRTB_KB_QUESTION		= 63,
	VIRTB_KB_AT			= 64,
	/* 
	   Skip uppercase letters
	 */
	VIRTB_KB_LEFTBRACKET	= 91,
	VIRTB_KB_BACKSLASH		= 92,
	VIRTB_KB_RIGHTBRACKET	= 93,
	VIRTB_KB_CARET		= 94,
	VIRTB_KB_UNDERSCORE		= 95,
	VIRTB_KB_BACKQUOTE		= 96,
	VIRTB_KB_a			= 97,
	VIRTB_KB_b			= 98,
	VIRTB_KB_c			= 99,
	VIRTB_KB_d			= 100,
	VIRTB_KB_e			= 101,
	VIRTB_KB_f			= 102,
	VIRTB_KB_g			= 103,
	VIRTB_KB_h			= 104,
	VIRTB_KB_i			= 105,
	VIRTB_KB_j			= 106,
	VIRTB_KB_k			= 107,
	VIRTB_KB_l			= 108,
	VIRTB_KB_m			= 109,
	VIRTB_KB_n			= 110,
	VIRTB_KB_o			= 111,
	VIRTB_KB_p			= 112,
	VIRTB_KB_q			= 113,
	VIRTB_KB_r			= 114,
	VIRTB_KB_s			= 115,
	VIRTB_KB_t			= 116,
	VIRTB_KB_u			= 117,
	VIRTB_KB_v			= 118,
	VIRTB_KB_w			= 119,
	VIRTB_KB_x			= 120,
	VIRTB_KB_y			= 121,
	VIRTB_KB_z			= 122,
	VIRTB_KB_DELETE		= 127,
	/* End of ASCII mapped keysyms */

	/* International keyboard syms */
	VIRTB_KB_WORLD_0		= 160,		/* 0xA0 */
	VIRTB_KB_WORLD_1		= 161,
	VIRTB_KB_WORLD_2		= 162,
	VIRTB_KB_WORLD_3		= 163,
	VIRTB_KB_WORLD_4		= 164,
	VIRTB_KB_WORLD_5		= 165,
	VIRTB_KB_WORLD_6		= 166,
	VIRTB_KB_WORLD_7		= 167,
	VIRTB_KB_WORLD_8		= 168,
	VIRTB_KB_WORLD_9		= 169,
	VIRTB_KB_WORLD_10		= 170,
	VIRTB_KB_WORLD_11		= 171,
	VIRTB_KB_WORLD_12		= 172,
	VIRTB_KB_WORLD_13		= 173,
	VIRTB_KB_WORLD_14		= 174,
	VIRTB_KB_WORLD_15		= 175,
	VIRTB_KB_WORLD_16		= 176,
	VIRTB_KB_WORLD_17		= 177,
	VIRTB_KB_WORLD_18		= 178,
	VIRTB_KB_WORLD_19		= 179,
	VIRTB_KB_WORLD_20		= 180,
	VIRTB_KB_WORLD_21		= 181,
	VIRTB_KB_WORLD_22		= 182,
	VIRTB_KB_WORLD_23		= 183,
	VIRTB_KB_WORLD_24		= 184,
	VIRTB_KB_WORLD_25		= 185,
	VIRTB_KB_WORLD_26		= 186,
	VIRTB_KB_WORLD_27		= 187,
	VIRTB_KB_WORLD_28		= 188,
	VIRTB_KB_WORLD_29		= 189,
	VIRTB_KB_WORLD_30		= 190,
	VIRTB_KB_WORLD_31		= 191,
	VIRTB_KB_WORLD_32		= 192,
	VIRTB_KB_WORLD_33		= 193,
	VIRTB_KB_WORLD_34		= 194,
	VIRTB_KB_WORLD_35		= 195,
	VIRTB_KB_WORLD_36		= 196,
	VIRTB_KB_WORLD_37		= 197,
	VIRTB_KB_WORLD_38		= 198,
	VIRTB_KB_WORLD_39		= 199,
	VIRTB_KB_WORLD_40		= 200,
	VIRTB_KB_WORLD_41		= 201,
	VIRTB_KB_WORLD_42		= 202,
	VIRTB_KB_WORLD_43		= 203,
	VIRTB_KB_WORLD_44		= 204,
	VIRTB_KB_WORLD_45		= 205,
	VIRTB_KB_WORLD_46		= 206,
	VIRTB_KB_WORLD_47		= 207,
	VIRTB_KB_WORLD_48		= 208,
	VIRTB_KB_WORLD_49		= 209,
	VIRTB_KB_WORLD_50		= 210,
	VIRTB_KB_WORLD_51		= 211,
	VIRTB_KB_WORLD_52		= 212,
	VIRTB_KB_WORLD_53		= 213,
	VIRTB_KB_WORLD_54		= 214,
	VIRTB_KB_WORLD_55		= 215,
	VIRTB_KB_WORLD_56		= 216,
	VIRTB_KB_WORLD_57		= 217,
	VIRTB_KB_WORLD_58		= 218,
	VIRTB_KB_WORLD_59		= 219,
	VIRTB_KB_WORLD_60		= 220,
	VIRTB_KB_WORLD_61		= 221,
	VIRTB_KB_WORLD_62		= 222,
	VIRTB_KB_WORLD_63		= 223,
	VIRTB_KB_WORLD_64		= 224,
	VIRTB_KB_WORLD_65		= 225,
	VIRTB_KB_WORLD_66		= 226,
	VIRTB_KB_WORLD_67		= 227,
	VIRTB_KB_WORLD_68		= 228,
	VIRTB_KB_WORLD_69		= 229,
	VIRTB_KB_WORLD_70		= 230,
	VIRTB_KB_WORLD_71		= 231,
	VIRTB_KB_WORLD_72		= 232,
	VIRTB_KB_WORLD_73		= 233,
	VIRTB_KB_WORLD_74		= 234,
	VIRTB_KB_WORLD_75		= 235,
	VIRTB_KB_WORLD_76		= 236,
	VIRTB_KB_WORLD_77		= 237,
	VIRTB_KB_WORLD_78		= 238,
	VIRTB_KB_WORLD_79		= 239,
	VIRTB_KB_WORLD_80		= 240,
	VIRTB_KB_WORLD_81		= 241,
	VIRTB_KB_WORLD_82		= 242,
	VIRTB_KB_WORLD_83		= 243,
	VIRTB_KB_WORLD_84		= 244,
	VIRTB_KB_WORLD_85		= 245,
	VIRTB_KB_WORLD_86		= 246,
	VIRTB_KB_WORLD_87		= 247,
	VIRTB_KB_WORLD_88		= 248,
	VIRTB_KB_WORLD_89		= 249,
	VIRTB_KB_WORLD_90		= 250,
	VIRTB_KB_WORLD_91		= 251,
	VIRTB_KB_WORLD_92		= 252,
	VIRTB_KB_WORLD_93		= 253,
	VIRTB_KB_WORLD_94		= 254,
	VIRTB_KB_WORLD_95		= 255,		/* 0xFF */

	/* Numeric keypad */
	VIRTB_KB_KP0		= 256,
	VIRTB_KB_KP1		= 257,
	VIRTB_KB_KP2		= 258,
	VIRTB_KB_KP3		= 259,
	VIRTB_KB_KP4		= 260,
	VIRTB_KB_KP5		= 261,
	VIRTB_KB_KP6		= 262,
	VIRTB_KB_KP7		= 263,
	VIRTB_KB_KP8		= 264,
	VIRTB_KB_KP9		= 265,
	VIRTB_KB_KP_PERIOD		= 266,
	VIRTB_KB_KP_DIVIDE		= 267,
	VIRTB_KB_KP_MULTIPLY	= 268,
	VIRTB_KB_KP_MINUS		= 269,
	VIRTB_KB_KP_PLUS		= 270,
	VIRTB_KB_KP_ENTER		= 271,
	VIRTB_KB_KP_EQUALS		= 272,

	/* Arrows + Home/End pad */
	VIRTB_KB_UP			= 273,
	VIRTB_KB_DOWN		= 274,
	VIRTB_KB_RIGHT		= 275,
	VIRTB_KB_LEFT		= 276,
	VIRTB_KB_INSERT		= 277,
	VIRTB_KB_HOME		= 278,
	VIRTB_KB_END		= 279,
	VIRTB_KB_PAGEUP		= 280,
	VIRTB_KB_PAGEDOWN		= 281,

	/* Function keys */
	VIRTB_KB_F1			= 282,
	VIRTB_KB_F2			= 283,
	VIRTB_KB_F3			= 284,
	VIRTB_KB_F4			= 285,
	VIRTB_KB_F5			= 286,
	VIRTB_KB_F6			= 287,
	VIRTB_KB_F7			= 288,
	VIRTB_KB_F8			= 289,
	VIRTB_KB_F9			= 290,
	VIRTB_KB_F10		= 291,
	VIRTB_KB_F11		= 292,
	VIRTB_KB_F12		= 293,
	VIRTB_KB_F13		= 294,
	VIRTB_KB_F14		= 295,
	VIRTB_KB_F15		= 296,

	/* Key state modifier keys */
	VIRTB_KB_NUMLOCK		= 300,
	VIRTB_KB_CAPSLOCK		= 301,
	VIRTB_KB_SCROLLOCK		= 302,
	VIRTB_KB_RSHIFT		= 303,
	VIRTB_KB_LSHIFT		= 304,
	VIRTB_KB_RCTRL		= 305,
	VIRTB_KB_LCTRL		= 306,
	VIRTB_KB_RALT		= 307,
	VIRTB_KB_LALT		= 308,
	VIRTB_KB_RMETA		= 309,
	VIRTB_KB_LMETA		= 310,
	VIRTB_KB_LSUPER		= 311,		/* Left "Windows" key */
	VIRTB_KB_RSUPER		= 312,		/* Right "Windows" key */
	VIRTB_KB_MODE		= 313,		/* "Alt Gr" key */
	VIRTB_KB_COMPOSE		= 314,		/* Multi-key compose key */

	/* Miscellaneous function keys */
	VIRTB_KB_HELP		= 315,
	VIRTB_KB_PRINT		= 316,
	VIRTB_KB_SYSREQ		= 317,
	VIRTB_KB_BREAK		= 318,
	VIRTB_KB_MENU		= 319,
	VIRTB_KB_POWER		= 320,		/* Power Macintosh power key */
	VIRTB_KB_EURO		= 321,		/* Some european keyboards */
	VIRTB_KB_UNDO		= 322,		/* Atari keyboard has Undo */

	/* Add any other keys here */

	VIRTB_KB_LAST,

	/* Mednafen-specific virtual buttons */

	// D-pad 0
	VIRTB_DP0_U = 0x1000,
	VIRTB_DP0_D,
	VIRTB_DP0_L,
	VIRTB_DP0_R,

	// D-pad 1
        VIRTB_DP1_U = 0x1100,
        VIRTB_DP1_D,
        VIRTB_DP1_L,
        VIRTB_DP1_R,

	// Single row of buttons.  Don't use if VIRTB_TOP_* or VIRTB_BOTTOM_* virtual buttons are also used with the device.
	// This is for simple devices with one row of action buttons.
	VIRTB_0 = 0x2000,
	VIRTB_1,
	VIRTB_2,
	VIRTB_3,

	// Top row of buttons, left to right.
	VIRTB_TOP_0 = 0x2100,
	VIRTB_TOP_1,
	VIRTB_TOP_2,
	VIRTB_TOP_3,

	// Bottom row of buttons.
	VIRTB_BOT_0	    = 0x2200,
	VIRTB_BOT_1,
	VIRTB_BOT_2,
	VIRTB_BOT_3,

	VIRTB_SELECT	    = 0x2800,
	VIRTB_START,

	// Diamond-shaped button arrangement
	VIRTB_DMD_L	= VIRTB_BOT_0, /*0x3000*/	// Treated equivalent to VIRTB_BOT_0 for now.
	VIRTB_DMD_R	= VIRTB_TOP_1,			// Treated equivalent to VIRTB_TOP_1, for now.
	VIRTB_DMD_U	= VIRTB_TOP_0,			// Treated equivalent to VIRTB_TOP_0, for now.
	VIRTB_DMD_D	= VIRTB_BOT_1,		// Treated equivalent to VIRTB_BOT_1, for now.

	// Shoulder buttons

	VIRTB_SHLDR_L	= 0x3100,
	VIRTB_SHLDR_R,

	VIRTB_LAST
} InputDeviceInputVB;

#endif
