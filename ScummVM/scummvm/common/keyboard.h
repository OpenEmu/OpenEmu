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

#ifndef COMMON_KEYBOARD_H
#define COMMON_KEYBOARD_H

#include "common/scummsys.h"

#if defined(__amigaos4__)
// KEYCODE_LESS and KEYCODE_GREATER are already defined in AmigaOS, inside
// include/include_h/intuition/intuition.h (bug #3121350)
#if defined(KEYCODE_LESS) && defined(KEYCODE_GREATER)
#undef KEYCODE_LESS
#undef KEYCODE_GREATER
#endif
#endif

namespace Common {

enum KeyCode {
	KEYCODE_INVALID     = 0,

	KEYCODE_BACKSPACE   = 8,
	KEYCODE_TAB         = 9,
	KEYCODE_CLEAR       = 12,
	KEYCODE_RETURN      = 13,
	KEYCODE_PAUSE       = 19,
	KEYCODE_ESCAPE      = 27,
	KEYCODE_SPACE       = 32,
	KEYCODE_EXCLAIM     = 33,      // !
	KEYCODE_QUOTEDBL    = 34,      // "
	KEYCODE_HASH        = 35,      // #
	KEYCODE_DOLLAR      = 36,      // $
	KEYCODE_AMPERSAND   = 38,      // &
	KEYCODE_QUOTE       = 39,      // '
	KEYCODE_LEFTPAREN   = 40,
	KEYCODE_RIGHTPAREN  = 41,
	KEYCODE_ASTERISK    = 42,      // *
	KEYCODE_PLUS        = 43,
	KEYCODE_COMMA       = 44,
	KEYCODE_MINUS       = 45,
	KEYCODE_PERIOD      = 46,
	KEYCODE_SLASH       = 47,
	KEYCODE_0           = 48,
	KEYCODE_1           = 49,
	KEYCODE_2           = 50,
	KEYCODE_3           = 51,
	KEYCODE_4           = 52,
	KEYCODE_5           = 53,
	KEYCODE_6           = 54,
	KEYCODE_7           = 55,
	KEYCODE_8           = 56,
	KEYCODE_9           = 57,
	KEYCODE_COLON       = 58,
	KEYCODE_SEMICOLON   = 59,
	KEYCODE_LESS        = 60,
	KEYCODE_EQUALS      = 61,
	KEYCODE_GREATER     = 62,
	KEYCODE_QUESTION    = 63,
	KEYCODE_AT          = 64,

	KEYCODE_LEFTBRACKET = 91,
	KEYCODE_BACKSLASH   = 92,
	KEYCODE_RIGHTBRACKET= 93,
	KEYCODE_CARET       = 94,
	KEYCODE_UNDERSCORE  = 95,
	KEYCODE_BACKQUOTE   = 96,
	KEYCODE_a           = 97,
	KEYCODE_b           = 98,
	KEYCODE_c           = 99,
	KEYCODE_d           = 100,
	KEYCODE_e           = 101,
	KEYCODE_f           = 102,
	KEYCODE_g           = 103,
	KEYCODE_h           = 104,
	KEYCODE_i           = 105,
	KEYCODE_j           = 106,
	KEYCODE_k           = 107,
	KEYCODE_l           = 108,
	KEYCODE_m           = 109,
	KEYCODE_n           = 110,
	KEYCODE_o           = 111,
	KEYCODE_p           = 112,
	KEYCODE_q           = 113,
	KEYCODE_r           = 114,
	KEYCODE_s           = 115,
	KEYCODE_t           = 116,
	KEYCODE_u           = 117,
	KEYCODE_v           = 118,
	KEYCODE_w           = 119,
	KEYCODE_x           = 120,
	KEYCODE_y           = 121,
	KEYCODE_z           = 122,
	KEYCODE_DELETE      = 127,
	KEYCODE_TILDE       = 176,      // ~

	// Numeric keypad
	KEYCODE_KP0         = 256,
	KEYCODE_KP1         = 257,
	KEYCODE_KP2         = 258,
	KEYCODE_KP3         = 259,
	KEYCODE_KP4         = 260,
	KEYCODE_KP5         = 261,
	KEYCODE_KP6         = 262,
	KEYCODE_KP7         = 263,
	KEYCODE_KP8         = 264,
	KEYCODE_KP9         = 265,
	KEYCODE_KP_PERIOD   = 266,
	KEYCODE_KP_DIVIDE   = 267,
	KEYCODE_KP_MULTIPLY = 268,
	KEYCODE_KP_MINUS    = 269,
	KEYCODE_KP_PLUS     = 270,
	KEYCODE_KP_ENTER    = 271,
	KEYCODE_KP_EQUALS   = 272,

	// Arrows + Home/End pad
	KEYCODE_UP          = 273,
	KEYCODE_DOWN        = 274,
	KEYCODE_RIGHT       = 275,
	KEYCODE_LEFT        = 276,
	KEYCODE_INSERT      = 277,
	KEYCODE_HOME        = 278,
	KEYCODE_END         = 279,
	KEYCODE_PAGEUP      = 280,
	KEYCODE_PAGEDOWN    = 281,

	// Function keys
	KEYCODE_F1          = 282,
	KEYCODE_F2          = 283,
	KEYCODE_F3          = 284,
	KEYCODE_F4          = 285,
	KEYCODE_F5          = 286,
	KEYCODE_F6          = 287,
	KEYCODE_F7          = 288,
	KEYCODE_F8          = 289,
	KEYCODE_F9          = 290,
	KEYCODE_F10         = 291,
	KEYCODE_F11         = 292,
	KEYCODE_F12         = 293,
	KEYCODE_F13         = 294,
	KEYCODE_F14         = 295,
	KEYCODE_F15         = 296,

	// Key state modifier keys
	KEYCODE_NUMLOCK     = 300,
	KEYCODE_CAPSLOCK    = 301,
	KEYCODE_SCROLLOCK   = 302,
	KEYCODE_RSHIFT      = 303,
	KEYCODE_LSHIFT      = 304,
	KEYCODE_RCTRL       = 305,
	KEYCODE_LCTRL       = 306,
	KEYCODE_RALT        = 307,
	KEYCODE_LALT        = 308,
	KEYCODE_RMETA       = 309,
	KEYCODE_LMETA       = 310,
	KEYCODE_LSUPER      = 311,      // Left "Windows" key
	KEYCODE_RSUPER      = 312,      // Right "Windows" key
	KEYCODE_MODE        = 313,      // "Alt Gr" key
	KEYCODE_COMPOSE     = 314,      // Multi-key compose key

	// Miscellaneous function keys
	KEYCODE_HELP        = 315,
	KEYCODE_PRINT       = 316,
	KEYCODE_SYSREQ      = 317,
	KEYCODE_BREAK       = 318,
	KEYCODE_MENU        = 319,
	KEYCODE_POWER       = 320,      // Power Macintosh power key
	KEYCODE_EURO        = 321,      // Some european keyboards
	KEYCODE_UNDO        = 322       // Atari keyboard has Undo
};

/**
 * List of certain special and some fake 'ascii' values used in keyboard events.
 * The values for the function keys listed here are based on what certain SCUMM
 * games expect in their scripts.
 * @todo Get rid of the function key values, and instead enforce that engines use
 * the keycode value to handle these.
 */
enum {
	ASCII_BACKSPACE     = 8,
	ASCII_TAB           = 9,
	ASCII_RETURN        = 13,
	ASCII_ESCAPE        = 27,
	ASCII_SPACE         = 32,

	ASCII_F1            = 315,
	ASCII_F2            = 316,
	ASCII_F3            = 317,
	ASCII_F4            = 318,
	ASCII_F5            = 319,
	ASCII_F6            = 320,
	ASCII_F7            = 321,
	ASCII_F8            = 322,
	ASCII_F9            = 323,
	ASCII_F10           = 324,
	ASCII_F11           = 325,
	ASCII_F12           = 326
};

/**
 * Keyboard modifier flags, used for Event::kbd::flags.
 */
enum {
	// Non-sticky modifier flags
	KBD_CTRL  = 1 << 0,
	KBD_ALT   = 1 << 1,
	KBD_SHIFT = 1 << 2,
	KBD_META  = 1 << 3,
	KBD_NON_STICKY = (KBD_CTRL|KBD_ALT|KBD_SHIFT|KBD_META),

	// Sticky modifier flags
	KBD_NUM   = 1 << 4,
	KBD_CAPS  = 1 << 5,
	KBD_SCRL  = 1 << 6,
	KBD_STICKY = (KBD_NUM|KBD_CAPS|KBD_SCRL)

};

/**
 * Keyboard status, as used in the Event struct.
 */
struct KeyState {
	/**
	 * Abstract key code (will be the same for any given key regardless
	 * of modifiers being held at the same time.
	 */
	KeyCode keycode;

	/**
	 * ASCII-value of the pressed key (if any).
	 * This depends on modifiers, i.e. pressing the 'A' key results in
	 * different values here depending on the status of shift, alt and
	 * caps lock. This should be used rather than keycode for text input
	 * to avoid keyboard layout issues. For example you cannot assume that
	 * KEYCODE_0 without a modifier will be '0' (on AZERTY keyboards it is
	*  not).
	 */
	uint16 ascii;

	/**
	 * Status of the modifier keys. Bits are set in this for each
	 * pressed modifier.
	 * We distinguish 'non-sticky' and 'sticky' modifiers flags. The former
	 * are only set while certain keys (ctrl, alt, shift) are pressed by the
	 * user; the latter (num lock, caps lock, scroll lock) are activated when
	 * certain keys are pressed and released; and deactivated when that key
	 * is pressed and released a second time.
	 *
	 * @see KBD_CTRL, KBD_ALT, KBD_SHIFT, KBD_NUM, KBD_CAPS, KBD_SCRL
	 */
	byte flags;

	KeyState(KeyCode kc = KEYCODE_INVALID) {
		keycode = kc;
		ascii = (uint16)kc;
		flags = 0;
	}

	KeyState(KeyCode kc, uint16 asc, byte f = 0) {
		keycode = kc;
		ascii = asc;
		flags = f;
	}

	void reset() {
		keycode = KEYCODE_INVALID;
		ascii = flags = 0;
	}

	/**
	 * Check whether the non-sticky flags are *exactly* as specified by f.
	 * This ignores the sticky flags (KBD_NUM, KBD_CAPS, KBD_SCRL).
	 * Sticky flags should never be passed to this function.
	 * If you just want to check whether a modifier flag is set, just bit-and
	 * the flag. E.g. to check whether the control key modifier is set,
	 * you can write
	 *    if (keystate.flags & KBD_CTRL) { ... }
	 */
	bool hasFlags(byte f) const {
		assert(!(f & KBD_STICKY));
		return f == (flags & ~KBD_STICKY);
	}

	/**
	 * Check if two key states are equal. This implementation ignores the state
	 * of the sticky flags (caps lock, num lock, scroll lock) completely. This
	 * functionality is currently only used by the keymapper.
	 */
	bool operator==(const KeyState &x) const {
		// Intentionally ignore ASCII, as the keycode and non-sticky flag
		// combination should suffice.
		return keycode == x.keycode && hasFlags(x.flags & ~KBD_STICKY);
	}
};

} // End of namespace Common

#endif
