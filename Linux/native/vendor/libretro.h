/*!
 * libretro.h is a simple API that allows for the creation of games and emulators.
 *
 * @file libretro.h
 * @version 1
 * @author libretro
 * @copyright Copyright (C) 2010-2024 The RetroArch team
 *
 * @paragraph LICENSE
 * The following license statement only applies to this libretro API header (libretro.h).
 *
 * Copyright (C) 2010-2024 The RetroArch team
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LIBRETRO_H__
#define LIBRETRO_H__

#include <stdint.h>
#include <stddef.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
#if defined(_MSC_VER) && _MSC_VER < 1800 && !defined(SN_TARGET_PS3)
/* Hack applied for MSVC when compiling in C89 mode
 * as it isn't C99-compliant. */
#define bool unsigned char
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif
#endif

#ifndef RETRO_CALLCONV
#  if defined(__GNUC__) && defined(__i386__) && !defined(__x86_64__)
#    define RETRO_CALLCONV __attribute__((cdecl))
#  elif defined(_MSC_VER) && defined(_M_X86) && !defined(_M_X64)
#    define RETRO_CALLCONV __cdecl
#  else
#    define RETRO_CALLCONV /* all other platforms only have one calling convention each */
#  endif
#endif

#ifndef RETRO_API
#  if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#    ifdef RETRO_IMPORT_SYMBOLS
#      ifdef __GNUC__
#        define RETRO_API RETRO_CALLCONV __attribute__((__dllimport__))
#      else
#        define RETRO_API RETRO_CALLCONV __declspec(dllimport)
#      endif
#    else
#      ifdef __GNUC__
#        define RETRO_API RETRO_CALLCONV __attribute__((__dllexport__))
#      else
#        define RETRO_API RETRO_CALLCONV __declspec(dllexport)
#      endif
#    endif
#  else
#      if defined(__GNUC__) && __GNUC__ >= 4
#        define RETRO_API RETRO_CALLCONV __attribute__((__visibility__("default")))
#      else
#        define RETRO_API RETRO_CALLCONV
#      endif
#  endif
#endif

/**
 * The major version of the libretro API and ABI.
 * Cores may support multiple versions,
 * or they may reject cores with unsupported versions.
 * It is only incremented for incompatible API/ABI changes;
 * this generally implies a function was removed or changed,
 * or that a \c struct had fields removed or changed.
 * @note A design goal of libretro is to avoid having to increase this value at all costs.
 * This is why there are APIs that are "extended" or "V2".
 */
#define RETRO_API_VERSION         1

/**
 * @defgroup RETRO_DEVICE Input Devices
 * @brief Libretro's fundamental device abstractions.
 *
 * Libretro's input system consists of abstractions over standard device types,
 * such as a joypad (with or without analog), mouse, keyboard, light gun, or an abstract pointer.
 * Instead of managing input devices themselves,
 * cores need only to map their own concept of a controller to libretro's abstractions.
 * This makes it possible for frontends to map the abstract types to a real input device
 * without having to worry about the correct use of arbitrary (real) controller layouts.
 * @{
 */

#define RETRO_DEVICE_TYPE_SHIFT         8
#define RETRO_DEVICE_MASK               ((1 << RETRO_DEVICE_TYPE_SHIFT) - 1)

/**
 * Defines an ID for a subclass of a known device type.
 *
 * To define a subclass ID, use this macro like so:
 * @code{c}
 * #define RETRO_DEVICE_SUPER_SCOPE RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_LIGHTGUN, 1)
 * #define RETRO_DEVICE_JUSTIFIER RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_LIGHTGUN, 2)
 * @endcode
 *
 * Correct use of this macro allows a frontend to select a suitable physical device
 * to map to the emulated device.
 *
 * @note Cores must use the base ID when polling for input,
 * and frontends must only accept the base ID for this purpose.
 * Polling for input using subclass IDs is reserved for future definition.
 *
 * @param base One of the \ref RETRO_DEVICE "base device types".
 * @param id A unique ID, with respect to \c base.
 * Must be a non-negative integer.
 * @return A unique subclass ID.
 * @see retro_controller_description
 * @see retro_set_controller_port_device
 */
#define RETRO_DEVICE_SUBCLASS(base, id) (((id + 1) << RETRO_DEVICE_TYPE_SHIFT) | base)

/**
 * @defgroup RETRO_DEVICE Input Device Classes
 * @{
 */

/**
 * Indicates no input.
 *
 * When provided as the \c device argument to \c retro_input_state_t,
 * all other arguments are ignored and zero is returned.
 *
 * @see retro_input_state_t
 */
#define RETRO_DEVICE_NONE         0

/**
 * An abstraction around a game controller, known as a "RetroPad".
 *
 * The RetroPad is modelled after a SNES controller,
 * but with additional L2/R2/L3/R3 buttons
 * (similar to a PlayStation controller).
 *
 * When provided as the \c device argument to \c retro_input_state_t,
 * the \c id argument denotes the button (including D-Pad directions) to query.
 * The result of said query will be 1 if the button is down, 0 if not.
 *
 * There is one exception; if \c RETRO_DEVICE_ID_JOYPAD_MASK is queried
 * (and the frontend supports this query),
 * the result will be a bitmask of all pressed buttons.
 *
 * @see retro_input_state_t
 * @see RETRO_DEVICE_ANALOG
 * @see RETRO_DEVICE_ID_JOYPAD
 * @see RETRO_DEVICE_ID_JOYPAD_MASK
 * @see RETRO_ENVIRONMENT_GET_INPUT_BITMASKS
 */
#define RETRO_DEVICE_JOYPAD       1

/**
 * An abstraction around a mouse, similar to the SNES Mouse but with more buttons.
 *
 * When provided as the \c device argument to \c retro_input_state_t,
 * the \c id argument denotes the button or axis to query.
 * For buttons, the result of said query
 * will be 1 if the button is down or 0 if not.
 * For mouse wheel axes, the result
 * will be 1 if the wheel was rotated in that direction and 0 if not.
 * For the mouse pointer axis, the result will be thee mouse's movement
 * relative to the last poll.
 * The core is responsible for tracking the mouse's position,
 * and the frontend is responsible for preventing interference
 * by the real hardware pointer (if applicable).
 *
 * @note This should only be used for cores that emulate mouse input,
 * such as for home computers
 * or consoles with mouse attachments.
 * Cores that emulate light guns should use \c RETRO_DEVICE_LIGHTGUN,
 * and cores that emulate touch screens should use \c RETRO_DEVICE_POINTER.
 *
 * @see RETRO_DEVICE_POINTER
 * @see RETRO_DEVICE_LIGHTGUN
 */
#define RETRO_DEVICE_MOUSE        2

/**
 * An abstraction around a keyboard.
 *
 * When provided as the \c device argument to \c retro_input_state_t,
 * the \c id argument denotes the key to poll.
 *
 * @note This should only be used for cores that emulate keyboard input,
 * such as for home computers
 * or consoles with keyboard attachments.
 * Cores that emulate gamepads should use \c RETRO_DEVICE_JOYPAD or \c RETRO_DEVICE_ANALOG,
 * and leave keyboard compatibility to the frontend.
 *
 * @see RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK
 * @see retro_key
 */
#define RETRO_DEVICE_KEYBOARD     3

/**
 * An abstraction around a light gun, similar to the PlayStation's Guncon.
 *
 * When provided as the \c device argument to \c retro_input_state_t,
 * the \c id argument denotes one of several possible inputs.
 *
 * The gun's coordinates are reported in screen space (similar to the pointer)
 * in the range of [-0x8000, 0x7fff].
 * Zero is the center of the game's screen
 * and -0x8000 represents out-of-bounds.
 * The trigger and various auxiliary buttons are also reported.
 *
 * @note A forced off-screen shot can be requested for auto-reloading
 * function in some games.
 *
 * @see RETRO_DEVICE_POINTER
 */
#define RETRO_DEVICE_LIGHTGUN     4

/**
 * An extension of the RetroPad that supports analog input.
 *
 * The analog RetroPad provides two virtual analog sticks (similar to DualShock controllers)
 * and allows any button to be treated as analog (similar to Xbox shoulder triggers).
 *
 * When provided as the \c device argument to \c retro_input_state_t,
 * the \c id argument denotes an analog axis or an analog button.
 *
 * Analog axes are reported in the range of [-0x8000, 0x7fff],
 * with the X axis being positive towards the right
 * and the Y axis being positive towards the bottom.
 *
 * Analog buttons are reported in the range of [0, 0x7fff],
 * where 0 is unpressed and 0x7fff is fully pressed.
 *
 * @note Cores should only use this type if they need analog input.
 * Otherwise, \c RETRO_DEVICE_JOYPAD should be used.
 * @see RETRO_DEVICE_JOYPAD
 */
#define RETRO_DEVICE_ANALOG       5

/**
 * Input Device: Pointer.
 *
 * Abstracts the concept of a pointing mechanism, e.g. touch.
 * This allows libretro to query in absolute coordinates where on the
 * screen a mouse (or something similar) is being placed.
 * For a touch centric device, coordinates reported are the coordinates
 * of the press.
 *
 * Coordinates in X and Y are reported as:
 * [-0x7fff, 0x7fff]: -0x7fff corresponds to the far left/top of the screen,
 * and 0x7fff corresponds to the far right/bottom of the screen.
 * The "screen" is here defined as area that is passed to the frontend and
 * later displayed on the monitor. If the pointer is outside this screen,
 * such as in the black surrounding areas when actual display is larger,
 * edge position is reported. An explicit edge detection is also provided,
 * that will return 1 if the pointer is near the screen edge or actually outside it.
 *
 * The frontend is free to scale/resize this screen as it sees fit, however,
 * (X, Y) = (-0x7fff, -0x7fff) will correspond to the top-left pixel of the
 * game image, etc.
 *
 * To check if the pointer coordinates are valid (e.g. a touch display
 * actually being touched), \c RETRO_DEVICE_ID_POINTER_PRESSED returns 1 or 0.
 *
 * If using a mouse on a desktop, \c RETRO_DEVICE_ID_POINTER_PRESSED will
 * usually correspond to the left mouse button, but this is a frontend decision.
 * \c RETRO_DEVICE_ID_POINTER_PRESSED will only return 1 if the pointer is
 * inside the game screen.
 *
 * For multi-touch, the index variable can be used to successively query
 * more presses.
 * If index = 0 returns true for \c _PRESSED, coordinates can be extracted
 * with \c _X, \c _Y for index = 0. One can then query \c _PRESSED, \c _X, \c _Y with
 * index = 1, and so on.
 * Eventually \c _PRESSED will return false for an index. No further presses
 * are registered at this point.
 *
 * @see RETRO_DEVICE_MOUSE
 * @see RETRO_DEVICE_ID_POINTER_X
 * @see RETRO_DEVICE_ID_POINTER_Y
 * @see RETRO_DEVICE_ID_POINTER_PRESSED
 */
#define RETRO_DEVICE_POINTER      6

/** @} */

/** @defgroup RETRO_DEVICE_ID_JOYPAD RetroPad Input
 * @brief Digital buttons for the RetroPad.
 *
 * Button placement is comparable to that of a SNES controller,
 * combined with the shoulder buttons of a PlayStation controller.
 * These values can also be used for the \c id field of \c RETRO_DEVICE_INDEX_ANALOG_BUTTON
 * to represent analog buttons (usually shoulder triggers).
 * @{
 */

/** The equivalent of the SNES controller's south face button. */
#define RETRO_DEVICE_ID_JOYPAD_B        0

/** The equivalent of the SNES controller's west face button. */
#define RETRO_DEVICE_ID_JOYPAD_Y        1

/** The equivalent of the SNES controller's left-center button. */
#define RETRO_DEVICE_ID_JOYPAD_SELECT   2

/** The equivalent of the SNES controller's right-center button. */
#define RETRO_DEVICE_ID_JOYPAD_START    3

/** Up on the RetroPad's D-pad. */
#define RETRO_DEVICE_ID_JOYPAD_UP       4

/** Down on the RetroPad's D-pad. */
#define RETRO_DEVICE_ID_JOYPAD_DOWN     5

/** Left on the RetroPad's D-pad. */
#define RETRO_DEVICE_ID_JOYPAD_LEFT     6

/** Right on the RetroPad's D-pad. */
#define RETRO_DEVICE_ID_JOYPAD_RIGHT    7

/** The equivalent of the SNES controller's east face button. */
#define RETRO_DEVICE_ID_JOYPAD_A        8

/** The equivalent of the SNES controller's north face button. */
#define RETRO_DEVICE_ID_JOYPAD_X        9

/** The equivalent of the SNES controller's left shoulder button. */
#define RETRO_DEVICE_ID_JOYPAD_L       10

/** The equivalent of the SNES controller's right shoulder button. */
#define RETRO_DEVICE_ID_JOYPAD_R       11

/** The equivalent of the PlayStation's rear left shoulder button. */
#define RETRO_DEVICE_ID_JOYPAD_L2      12

/** The equivalent of the PlayStation's rear right shoulder button. */
#define RETRO_DEVICE_ID_JOYPAD_R2      13

/**
 * The equivalent of the PlayStation's left analog stick button,
 * although the actual button need not be in this position.
 */
#define RETRO_DEVICE_ID_JOYPAD_L3      14

/**
 * The equivalent of the PlayStation's right analog stick button,
 * although the actual button need not be in this position.
 */
#define RETRO_DEVICE_ID_JOYPAD_R3      15

/**
 * Represents a bitmask that describes the state of all \c RETRO_DEVICE_ID_JOYPAD button constants,
 * rather than the state of a single button.
 *
 * @see RETRO_ENVIRONMENT_GET_INPUT_BITMASKS
 * @see RETRO_DEVICE_JOYPAD
 */
#define RETRO_DEVICE_ID_JOYPAD_MASK    256

/** @} */

/** @defgroup RETRO_DEVICE_ID_ANALOG Analog RetroPad Input
 * @{
 */

/* Index / Id values for ANALOG device. */
#define RETRO_DEVICE_INDEX_ANALOG_LEFT       0
#define RETRO_DEVICE_INDEX_ANALOG_RIGHT      1
#define RETRO_DEVICE_INDEX_ANALOG_BUTTON     2
#define RETRO_DEVICE_ID_ANALOG_X             0
#define RETRO_DEVICE_ID_ANALOG_Y             1

/** @} */

/* Id values for MOUSE. */
#define RETRO_DEVICE_ID_MOUSE_X                0
#define RETRO_DEVICE_ID_MOUSE_Y                1
#define RETRO_DEVICE_ID_MOUSE_LEFT             2
#define RETRO_DEVICE_ID_MOUSE_RIGHT            3
#define RETRO_DEVICE_ID_MOUSE_WHEELUP          4
#define RETRO_DEVICE_ID_MOUSE_WHEELDOWN        5
#define RETRO_DEVICE_ID_MOUSE_MIDDLE           6
#define RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELUP    7
#define RETRO_DEVICE_ID_MOUSE_HORIZ_WHEELDOWN  8
#define RETRO_DEVICE_ID_MOUSE_BUTTON_4         9
#define RETRO_DEVICE_ID_MOUSE_BUTTON_5         10

/* Id values for LIGHTGUN. */
#define RETRO_DEVICE_ID_LIGHTGUN_SCREEN_X        13 /*Absolute Position*/
#define RETRO_DEVICE_ID_LIGHTGUN_SCREEN_Y        14 /*Absolute Position*/
/** Indicates if lightgun points off the screen or near the edge */
#define RETRO_DEVICE_ID_LIGHTGUN_IS_OFFSCREEN    15 /*Status Check*/
#define RETRO_DEVICE_ID_LIGHTGUN_TRIGGER          2
#define RETRO_DEVICE_ID_LIGHTGUN_RELOAD          16 /*Forced off-screen shot*/
#define RETRO_DEVICE_ID_LIGHTGUN_AUX_A            3
#define RETRO_DEVICE_ID_LIGHTGUN_AUX_B            4
#define RETRO_DEVICE_ID_LIGHTGUN_START            6
#define RETRO_DEVICE_ID_LIGHTGUN_SELECT           7
#define RETRO_DEVICE_ID_LIGHTGUN_AUX_C            8
#define RETRO_DEVICE_ID_LIGHTGUN_DPAD_UP          9
#define RETRO_DEVICE_ID_LIGHTGUN_DPAD_DOWN       10
#define RETRO_DEVICE_ID_LIGHTGUN_DPAD_LEFT       11
#define RETRO_DEVICE_ID_LIGHTGUN_DPAD_RIGHT      12
/* deprecated */
#define RETRO_DEVICE_ID_LIGHTGUN_X                0 /*Relative Position*/
#define RETRO_DEVICE_ID_LIGHTGUN_Y                1 /*Relative Position*/
#define RETRO_DEVICE_ID_LIGHTGUN_CURSOR           3 /*Use Aux:A instead*/
#define RETRO_DEVICE_ID_LIGHTGUN_TURBO            4 /*Use Aux:B instead*/
#define RETRO_DEVICE_ID_LIGHTGUN_PAUSE            5 /*Use Start instead*/

/* Id values for POINTER. */
#define RETRO_DEVICE_ID_POINTER_X             0
#define RETRO_DEVICE_ID_POINTER_Y             1
#define RETRO_DEVICE_ID_POINTER_PRESSED       2
#define RETRO_DEVICE_ID_POINTER_COUNT         3
/** Indicates if pointer is off the screen or near the edge */
#define RETRO_DEVICE_ID_POINTER_IS_OFFSCREEN 15
/** @} */

/* Returned from retro_get_region(). */
#define RETRO_REGION_NTSC  0
#define RETRO_REGION_PAL   1

/**
 * Identifiers for supported languages.
 * @see RETRO_ENVIRONMENT_GET_LANGUAGE
 */
enum retro_language
{
   RETRO_LANGUAGE_ENGLISH             = 0,
   RETRO_LANGUAGE_JAPANESE            = 1,
   RETRO_LANGUAGE_FRENCH              = 2,
   RETRO_LANGUAGE_SPANISH             = 3,
   RETRO_LANGUAGE_GERMAN              = 4,
   RETRO_LANGUAGE_ITALIAN             = 5,
   RETRO_LANGUAGE_DUTCH               = 6,
   RETRO_LANGUAGE_PORTUGUESE_BRAZIL   = 7,
   RETRO_LANGUAGE_PORTUGUESE_PORTUGAL = 8,
   RETRO_LANGUAGE_RUSSIAN             = 9,
   RETRO_LANGUAGE_KOREAN              = 10,
   RETRO_LANGUAGE_CHINESE_TRADITIONAL = 11,
   RETRO_LANGUAGE_CHINESE_SIMPLIFIED  = 12,
   RETRO_LANGUAGE_ESPERANTO           = 13,
   RETRO_LANGUAGE_POLISH              = 14,
   RETRO_LANGUAGE_VIETNAMESE          = 15,
   RETRO_LANGUAGE_ARABIC              = 16,
   RETRO_LANGUAGE_GREEK               = 17,
   RETRO_LANGUAGE_TURKISH             = 18,
   RETRO_LANGUAGE_SLOVAK              = 19,
   RETRO_LANGUAGE_PERSIAN             = 20,
   RETRO_LANGUAGE_HEBREW              = 21,
   RETRO_LANGUAGE_ASTURIAN            = 22,
   RETRO_LANGUAGE_FINNISH             = 23,
   RETRO_LANGUAGE_INDONESIAN          = 24,
   RETRO_LANGUAGE_SWEDISH             = 25,
   RETRO_LANGUAGE_UKRAINIAN           = 26,
   RETRO_LANGUAGE_CZECH               = 27,
   RETRO_LANGUAGE_CATALAN_VALENCIA    = 28,
   RETRO_LANGUAGE_CATALAN             = 29,
   RETRO_LANGUAGE_BRITISH_ENGLISH     = 30,
   RETRO_LANGUAGE_HUNGARIAN           = 31,
   RETRO_LANGUAGE_BELARUSIAN          = 32,
   RETRO_LANGUAGE_GALICIAN            = 33,
   RETRO_LANGUAGE_NORWEGIAN           = 34,
   RETRO_LANGUAGE_IRISH               = 35,
   RETRO_LANGUAGE_THAI                = 36,
   RETRO_LANGUAGE_LAST,

   /** Defined to ensure that <tt>sizeof(retro_language) == sizeof(int)</tt>. Do not use. */
   RETRO_LANGUAGE_DUMMY          = INT_MAX
};

/** @defgroup RETRO_MEMORY Memory Types
 * @{
 */

/* Passed to retro_get_memory_data/size().
 * If the memory type doesn't apply to the
 * implementation NULL/0 can be returned.
 */
#define RETRO_MEMORY_MASK        0xff

/* Regular save RAM. This RAM is usually found on a game cartridge,
 * backed up by a battery.
 * If save game data is too complex for a single memory buffer,
 * the SAVE_DIRECTORY (preferably) or SYSTEM_DIRECTORY environment
 * callback can be used. */
#define RETRO_MEMORY_SAVE_RAM    0

/* Some games have a built-in clock to keep track of time.
 * This memory is usually just a couple of bytes to keep track of time.
 */
#define RETRO_MEMORY_RTC         1

/* System ram lets a frontend peek into a game systems main RAM. */
#define RETRO_MEMORY_SYSTEM_RAM  2

/* Video ram lets a frontend peek into a game systems video RAM (VRAM). */
#define RETRO_MEMORY_VIDEO_RAM   3

/* ROM lets a frontend peek into a game systems ROM. */
#define RETRO_MEMORY_ROM   4

/** @} */

/* Keysyms used for ID in input state callback when polling RETRO_KEYBOARD. */
enum retro_key
{
   RETROK_UNKNOWN        = 0,
   RETROK_FIRST          = 0,
   RETROK_BACKSPACE      = 8,
   RETROK_TAB            = 9,
   RETROK_CLEAR          = 12,
   RETROK_RETURN         = 13,
   RETROK_PAUSE          = 19,
   RETROK_ESCAPE         = 27,
   RETROK_SPACE          = 32,
   RETROK_EXCLAIM        = 33,
   RETROK_QUOTEDBL       = 34,
   RETROK_HASH           = 35,
   RETROK_DOLLAR         = 36,
   RETROK_AMPERSAND      = 38,
   RETROK_QUOTE          = 39,
   RETROK_LEFTPAREN      = 40,
   RETROK_RIGHTPAREN     = 41,
   RETROK_ASTERISK       = 42,
   RETROK_PLUS           = 43,
   RETROK_COMMA          = 44,
   RETROK_MINUS          = 45,
   RETROK_PERIOD         = 46,
   RETROK_SLASH          = 47,
   RETROK_0              = 48,
   RETROK_1              = 49,
   RETROK_2              = 50,
   RETROK_3              = 51,
   RETROK_4              = 52,
   RETROK_5              = 53,
   RETROK_6              = 54,
   RETROK_7              = 55,
   RETROK_8              = 56,
   RETROK_9              = 57,
   RETROK_COLON          = 58,
   RETROK_SEMICOLON      = 59,
   RETROK_LESS           = 60,
   RETROK_EQUALS         = 61,
   RETROK_GREATER        = 62,
   RETROK_QUESTION       = 63,
   RETROK_AT             = 64,
   RETROK_LEFTBRACKET    = 91,
   RETROK_BACKSLASH      = 92,
   RETROK_RIGHTBRACKET   = 93,
   RETROK_CARET          = 94,
   RETROK_UNDERSCORE     = 95,
   RETROK_BACKQUOTE      = 96,
   RETROK_a              = 97,
   RETROK_b              = 98,
   RETROK_c              = 99,
   RETROK_d              = 100,
   RETROK_e              = 101,
   RETROK_f              = 102,
   RETROK_g              = 103,
   RETROK_h              = 104,
   RETROK_i              = 105,
   RETROK_j              = 106,
   RETROK_k              = 107,
   RETROK_l              = 108,
   RETROK_m              = 109,
   RETROK_n              = 110,
   RETROK_o              = 111,
   RETROK_p              = 112,
   RETROK_q              = 113,
   RETROK_r              = 114,
   RETROK_s              = 115,
   RETROK_t              = 116,
   RETROK_u              = 117,
   RETROK_v              = 118,
   RETROK_w              = 119,
   RETROK_x              = 120,
   RETROK_y              = 121,
   RETROK_z              = 122,
   RETROK_LEFTBRACE      = 123,
   RETROK_BAR            = 124,
   RETROK_RIGHTBRACE     = 125,
   RETROK_TILDE          = 126,
   RETROK_DELETE         = 127,

   RETROK_KP0            = 256,
   RETROK_KP1            = 257,
   RETROK_KP2            = 258,
   RETROK_KP3            = 259,
   RETROK_KP4            = 260,
   RETROK_KP5            = 261,
   RETROK_KP6            = 262,
   RETROK_KP7            = 263,
   RETROK_KP8            = 264,
   RETROK_KP9            = 265,
   RETROK_KP_PERIOD      = 266,
   RETROK_KP_DIVIDE      = 267,
   RETROK_KP_MULTIPLY    = 268,
   RETROK_KP_MINUS       = 269,
   RETROK_KP_PLUS        = 270,
   RETROK_KP_ENTER       = 271,
   RETROK_KP_EQUALS      = 272,

   RETROK_UP             = 273,
   RETROK_DOWN           = 274,
   RETROK_RIGHT          = 275,
   RETROK_LEFT           = 276,
   RETROK_INSERT         = 277,
   RETROK_HOME           = 278,
   RETROK_END            = 279,
   RETROK_PAGEUP         = 280,
   RETROK_PAGEDOWN       = 281,

   RETROK_F1             = 282,
   RETROK_F2             = 283,
   RETROK_F3             = 284,
   RETROK_F4             = 285,
   RETROK_F5             = 286,
   RETROK_F6             = 287,
   RETROK_F7             = 288,
   RETROK_F8             = 289,
   RETROK_F9             = 290,
   RETROK_F10            = 291,
   RETROK_F11            = 292,
   RETROK_F12            = 293,
   RETROK_F13            = 294,
   RETROK_F14            = 295,
   RETROK_F15            = 296,

   RETROK_NUMLOCK        = 300,
   RETROK_CAPSLOCK       = 301,
   RETROK_SCROLLOCK      = 302,
   RETROK_RSHIFT         = 303,
   RETROK_LSHIFT         = 304,
   RETROK_RCTRL          = 305,
   RETROK_LCTRL          = 306,
   RETROK_RALT           = 307,
   RETROK_LALT           = 308,
   RETROK_RMETA          = 309,
   RETROK_LMETA          = 310,
   RETROK_LSUPER         = 311,
   RETROK_RSUPER         = 312,
   RETROK_MODE           = 313,
   RETROK_COMPOSE        = 314,

   RETROK_HELP           = 315,
   RETROK_PRINT          = 316,
   RETROK_SYSREQ         = 317,
   RETROK_BREAK          = 318,
   RETROK_MENU           = 319,
   RETROK_POWER          = 320,
   RETROK_EURO           = 321,
   RETROK_UNDO           = 322,
   RETROK_OEM_102        = 323,

   RETROK_BROWSER_BACK      = 324,
   RETROK_BROWSER_FORWARD   = 325,
   RETROK_BROWSER_REFRESH   = 326,
   RETROK_BROWSER_STOP      = 327,
   RETROK_BROWSER_SEARCH    = 328,
   RETROK_BROWSER_FAVORITES = 329,
   RETROK_BROWSER_HOME      = 330,
   RETROK_VOLUME_MUTE       = 331,
   RETROK_VOLUME_DOWN       = 332,
   RETROK_VOLUME_UP         = 333,
   RETROK_MEDIA_NEXT        = 334,
   RETROK_MEDIA_PREV        = 335,
   RETROK_MEDIA_STOP        = 336,
   RETROK_MEDIA_PLAY_PAUSE  = 337,
   RETROK_LAUNCH_MAIL       = 338,
   RETROK_LAUNCH_MEDIA      = 339,
   RETROK_LAUNCH_APP1       = 340,
   RETROK_LAUNCH_APP2       = 341,

   RETROK_LAST,

   RETROK_DUMMY          = INT_MAX /* Ensure sizeof(enum) == sizeof(int) */
};

enum retro_mod
{
   RETROKMOD_NONE       = 0x0000,

   RETROKMOD_SHIFT      = 0x01,
   RETROKMOD_CTRL       = 0x02,
   RETROKMOD_ALT        = 0x04,
   RETROKMOD_META       = 0x08,

   RETROKMOD_NUMLOCK    = 0x10,
   RETROKMOD_CAPSLOCK   = 0x20,
   RETROKMOD_SCROLLOCK  = 0x40,

   RETROKMOD_DUMMY = INT_MAX /* Ensure sizeof(enum) == sizeof(int) */
};

/**
 * @defgroup RETRO_ENVIRONMENT Environment Callbacks
 * @{
 */

/**
 * This bit indicates that the associated environment call is experimental,
 * and may be changed or removed in the future.
 * Frontends should mask out this bit before handling the environment call.
 */
#define RETRO_ENVIRONMENT_EXPERIMENTAL 0x10000

/** Frontend-internal environment callbacks should include this bit. */
#define RETRO_ENVIRONMENT_PRIVATE 0x20000

/* Environment commands. */
/**
 * Requests the frontend to set the screen rotation.
 *
 * @param[in] data <tt>const unsigned*</tt>.
 * Valid values are 0, 1, 2, and 3.
 * These numbers respectively set the screen rotation to 0, 90, 180, and 270 degrees counter-clockwise.
 * @returns \c true if the screen rotation was set successfully.
 */
#define RETRO_ENVIRONMENT_SET_ROTATION  1

/**
 * Queries whether the core should use overscan or not.
 *
 * @param[out] data <tt>bool*</tt>.
 * Set to \c true if the core should use overscan,
 * \c false if it should be cropped away.
 * @returns \c true if the environment call is available.
 * Does \em not indicate whether overscan should be used.
 * @deprecated As of 2019 this callback is considered deprecated in favor of
 * using core options to manage overscan in a more nuanced, core-specific way.
 */
#define RETRO_ENVIRONMENT_GET_OVERSCAN  2

/**
 * Queries whether the frontend supports frame duping,
 * in the form of passing \c NULL to the video frame callback.
 *
 * @param[out] data <tt>bool*</tt>.
 * Set to \c true if the frontend supports frame duping.
 * @returns \c true if the environment call is available.
 * @see retro_video_refresh_t
 */
#define RETRO_ENVIRONMENT_GET_CAN_DUPE  3

/*
 * Environ 4, 5 are no longer supported (GET_VARIABLE / SET_VARIABLES),
 * and reserved to avoid possible ABI clash.
 */

/**
 * @brief Displays a user-facing message for a short time.
 *
 * Use this callback to convey important status messages,
 * such as errors or the result of long-running operations.
 * For trivial messages or logging, use \c RETRO_ENVIRONMENT_GET_LOG_INTERFACE or \c stderr.
 *
 * \code{.c}
 * void set_message_example(void)
 * {
 *    struct retro_message msg;
 *    msg.frames = 60 * 5; // 5 seconds
 *    msg.msg = "Hello world!";
 *
 *    environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE, &msg);
 * }
 * \endcode
 *
 * @deprecated Prefer using \c RETRO_ENVIRONMENT_SET_MESSAGE_EXT for new code,
 * as it offers more features.
 * Only use this environment call for compatibility with older cores or frontends.
 *
 * @param[in] data <tt>const struct retro_message*</tt>.
 * Details about the message to show to the user.
 * Behavior is undefined if <tt>NULL</tt>.
 * @returns \c true if the environment call is available.
 * @see retro_message
 * @see RETRO_ENVIRONMENT_GET_LOG_INTERFACE
 * @see RETRO_ENVIRONMENT_SET_MESSAGE_EXT
 * @see RETRO_ENVIRONMENT_SET_MESSAGE
 * @see RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION
 * @note The frontend must make its own copy of the message and the underlying string.
 */
#define RETRO_ENVIRONMENT_SET_MESSAGE   6

/**
 * Requests the frontend to shutdown the core.
 * Should only be used if the core can exit on its own,
 * such as from a menu item in a game
 * or an emulated power-off in an emulator.
 *
 * @param data Ignored.
 * @returns \c true if the environment call is available.
 */
#define RETRO_ENVIRONMENT_SHUTDOWN      7

/**
 * Gives a hint to the frontend of how demanding this core is on the system.
 * For example, reporting a level of 2 means that
 * this implementation should run decently on frontends
 * of level 2 and above.
 *
 * It can be used by the frontend to potentially warn
 * about too demanding implementations.
 *
 * The levels are "floating".
 *
 * This function can be called on a per-game basis,
 * as a core may have different demands for different games or settings.
 * If called, it should be called in <tt>retro_load_game()</tt>.
 * @param[in] data <tt>const unsigned*</tt>.
*/
#define RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL 8

/**
 * Returns the path to the frontend's system directory,
 * which can be used to store system-specific configuration
 * such as BIOS files or cached data.
 *
 * @param[out] data <tt>const char**</tt>.
 * Pointer to the \c char* in which the system directory will be saved.
 * The string is managed by the frontend and must not be modified or freed by the core.
 * May be \c NULL if no system directory is defined,
 * in which case the core should find an alternative directory.
 * @return \c true if the environment call is available,
 * even if the value returned in \c data is <tt>NULL</tt>.
 * @note Historically, some cores would use this folder for save data such as memory cards or SRAM.
 * This is now discouraged in favor of \c RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY.
 * @see RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY
 */
#define RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY 9

/**
 * Sets the internal pixel format used by the frontend for rendering.
 * The default pixel format is \c RETRO_PIXEL_FORMAT_0RGB1555 for compatibility reasons,
 * although it's considered deprecated and shouldn't be used by new code.
 *
 * @param[in] data <tt>const enum retro_pixel_format *</tt>.
 * Pointer to the pixel format to use.
 * @returns \c true if the pixel format was set successfully,
 * \c false if it's not supported or this callback is unavailable.
 * @note This function should be called inside \c retro_load_game()
 * or <tt>retro_get_system_av_info()</tt>.
 * @see retro_pixel_format
 */
#define RETRO_ENVIRONMENT_SET_PIXEL_FORMAT 10

/**
 * Sets an array of input descriptors for the frontend
 * to present to the user for configuring the core's controls.
 *
 * This function can be called at any time,
 * preferably early in the core's life cycle.
 * Ideally, no later than \c retro_load_game().
 *
 * @param[in] data <tt>const struct retro_input_descriptor *</tt>.
 * An array of input descriptors terminated by one whose
 * \c retro_input_descriptor::description field is set to \c NULL.
 * Behavior is undefined if \c NULL.
 * @return \c true if the environment call is recognized.
 * @see retro_input_descriptor
 */
#define RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS 11

/**
 * Sets a callback function used to notify the core about keyboard events.
 * This should only be used for cores that specifically need keyboard input,
 * such as for home computer emulators or games with text entry.
 *
 * @param[in] data <tt>const struct retro_keyboard_callback *</tt>.
 * Pointer to the callback function.
 * Behavior is undefined if <tt>NULL</tt>.
 * @return \c true if the environment call is recognized.
 * @see retro_keyboard_callback
 * @see retro_key
 */
#define RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK 12

/**
 * Sets an interface that the frontend can use to insert and remove disks
 * from the emulated console's disk drive.
 * Can be used for optical disks, floppy disks, or any other game storage medium
 * that can be swapped at runtime.
 *
 * This is intended for multi-disk games that expect the player
 * to manually swap disks at certain points in the game.
 *
 * @deprecated Prefer using \c RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE
 * over this environment call, as it supports additional features.
 * Only use this callback to maintain compatibility
 * with older cores or frontends.
 *
 * @param[in] data <tt>const struct retro_disk_control_callback *</tt>.
 * Pointer to the callback functions to use.
 * May be \c NULL, in which case the existing disk callback is deregistered.
 * @return \c true if this environment call is available,
 * even if \c data is \c NULL.
 * @see retro_disk_control_callback
 * @see RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE
 */
#define RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE 13

/**
 * Requests that a frontend enable a particular hardware rendering API.
 *
 * If successful, the frontend will create a context (and other related resources)
 * that the core can use for rendering.
 * The framebuffer will be at least as large as
 * the maximum dimensions provided in <tt>retro_get_system_av_info</tt>.
 *
 * @param[in, out] data <tt>struct retro_hw_render_callback *</tt>.
 * Pointer to the hardware render callback struct.
 * Used to define callbacks for the hardware-rendering life cycle,
 * as well as to request a particular rendering API.
 * @return \c true if the environment call is recognized
 * and the requested rendering API is supported.
 * \c false if \c data is \c NULL
 * or the frontend can't provide the requested rendering API.
 * @see retro_hw_render_callback
 * @see retro_video_refresh_t
 * @see RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER
 * @note Should be called in <tt>retro_load_game()</tt>.
 * @note If HW rendering is used, pass only \c RETRO_HW_FRAME_BUFFER_VALID or
 * \c NULL to <tt>retro_video_refresh_t</tt>.
 */
#define RETRO_ENVIRONMENT_SET_HW_RENDER 14

/**
 * Retrieves a core option's value from the frontend.
 * \c retro_variable::key should be set to an option key
 * that was previously set in \c RETRO_ENVIRONMENT_SET_VARIABLES
 * (or a similar environment call).
 *
 * @param[in,out] data <tt>struct retro_variable *</tt>.
 * Pointer to a single \c retro_variable struct.
 * See the documentation for \c retro_variable for details
 * on which fields are set by the frontend or core.
 * May be \c NULL.
 * @returns \c true if the environment call is available,
 * even if \c data is \c NULL or the key it specifies is not found.
 * @note Passing \c NULL in to \c data can be useful to
 * test for support of this environment call without looking up any variables.
 * @see retro_variable
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 * @see RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE
 */
#define RETRO_ENVIRONMENT_GET_VARIABLE 15

/**
 * Notifies the frontend of the core's available options.
 *
 * The core may check these options later using \c RETRO_ENVIRONMENT_GET_VARIABLE.
 * The frontend may also present these options to the user
 * in its own configuration UI.
 *
 * This should be called the first time as early as possible,
 * ideally in \c retro_set_environment.
 * The core may later call this function again
 * to communicate updated options to the frontend,
 * but the number of core options must not change.
 *
 * Here's an example that sets two options.
 *
 * @code
 * void set_variables_example(void)
 * {
 *    struct retro_variable options[] = {
 *        { "foo_speedhack", "Speed hack; false|true" }, // false by default
 *        { "foo_displayscale", "Display scale factor; 1|2|3|4" }, // 1 by default
 *        { NULL, NULL },
 *    };
 *
 *    environ_cb(RETRO_ENVIRONMENT_SET_VARIABLES, &options);
 * }
 * @endcode
 *
 * The possible values will generally be displayed and stored as-is by the frontend.
 *
 * @deprecated Prefer using \c RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2 for new code,
 * as it offers more features such as categories and translation.
 * Only use this environment call to maintain compatibility
 * with older frontends or cores.
 * @note Keep the available options (and their possible values) as low as possible;
 * it should be feasible to cycle through them without a keyboard.
 * @param[in] data <tt>const struct retro_variable *</tt>.
 * Pointer to an array of \c retro_variable structs that define available core options,
 * terminated by a <tt>{ NULL, NULL }</tt> element.
 * The frontend must maintain its own copy of this array.
 *
 * @returns \c true if the environment call is available,
 * even if \c data is <tt>NULL</tt>.
 * @see retro_variable
 * @see RETRO_ENVIRONMENT_GET_VARIABLE
 * @see RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 */
#define RETRO_ENVIRONMENT_SET_VARIABLES 16

/**
 * Queries whether at least one core option was updated by the frontend
 * since the last call to \ref RETRO_ENVIRONMENT_GET_VARIABLE.
 * This typically means that the user opened the core options menu and made some changes.
 *
 * Cores usually call this each frame before the core's main emulation logic.
 * Specific options can then be queried with \ref RETRO_ENVIRONMENT_GET_VARIABLE.
 *
 * @param[out] data <tt>bool *</tt>.
 * Set to \c true if at least one core option was updated
 * since the last call to \ref RETRO_ENVIRONMENT_GET_VARIABLE.
 * Behavior is undefined if this pointer is \c NULL.
 * @returns \c true if the environment call is available.
 * @see RETRO_ENVIRONMENT_GET_VARIABLE
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 */
#define RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE 17

/**
 * Notifies the frontend that this core can run without loading any content,
 * such as when emulating a console that has built-in software.
 * When a core is loaded without content,
 * \c retro_load_game receives an argument of <tt>NULL</tt>.
 * This should be called within \c retro_set_environment() only.
 *
 * @param[in] data <tt>const bool *</tt>.
 * Pointer to a single \c bool that indicates whether this frontend can run without content.
 * Can point to a value of \c false but this isn't necessary,
 * as contentless support is opt-in.
 * The behavior is undefined if \c data is <tt>NULL</tt>.
 * @returns \c true if the environment call is available.
 * @see retro_load_game
 */
#define RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME 18

/**
 * Retrieves the absolute path from which this core was loaded.
 * Useful when loading assets from paths relative to the core,
 * as is sometimes the case when using <tt>RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME</tt>.
 *
 * @param[out] data <tt>const char **</tt>.
 * Pointer to a string in which the core's path will be saved.
 * The string is managed by the frontend and must not be modified or freed by the core.
 * May be \c NULL if the core is statically linked to the frontend
 * or if the core's path otherwise cannot be determined.
 * Behavior is undefined if \c data is <tt>NULL</tt>.
 * @returns \c true if the environment call is available.
 */
#define RETRO_ENVIRONMENT_GET_LIBRETRO_PATH 19

/* Environment call 20 was an obsolete version of SET_AUDIO_CALLBACK.
 * It was not used by any known core at the time, and was removed from the API.
 * The number 20 is reserved to prevent ABI clashes.
 */

/**
 * Sets a callback that notifies the core of how much time has passed
 * since the last iteration of <tt>retro_run</tt>.
 * If the frontend is not running the core in real time
 * (e.g. it's frame-stepping or running in slow motion),
 * then the reference value will be provided to the callback instead.
 *
 * @param[in] data <tt>const struct retro_frame_time_callback *</tt>.
 * Pointer to a single \c retro_frame_time_callback struct.
 * Behavior is undefined if \c data is <tt>NULL</tt>.
 * @returns \c true if the environment call is available.
 * @note Frontends may disable this environment call in certain situations.
 * It will return \c false in those cases.
 * @see retro_frame_time_callback
 */
#define RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK 21

/**
 * Registers a set of functions that the frontend can use
 * to tell the core it's ready for audio output.
 *
 * It is intended for games that feature asynchronous audio.
 * It should not be used for emulators unless their audio is asynchronous.
 *
 *
 * The callback only notifies about writability; the libretro core still
 * has to call the normal audio callbacks
 * to write audio. The audio callbacks must be called from within the
 * notification callback.
 * The amount of audio data to write is up to the core.
 * Generally, the audio callback will be called continuously in a loop.
 *
 * A frontend may disable this callback in certain situations.
 * The core must be able to render audio with the "normal" interface.
 *
 * @param[in] data <tt>const struct retro_audio_callback *</tt>.
 * Pointer to a set of functions that the frontend will call to notify the core
 * when it's ready to receive audio data.
 * May be \c NULL, in which case the frontend will return
 * whether this environment callback is available.
 * @return \c true if this environment call is available,
 * even if \c data is \c NULL.
 * @warning The provided callbacks can be invoked from any thread,
 * so their implementations \em must be thread-safe.
 * @note If a core uses this callback,
 * it should also use <tt>RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK</tt>.
 * @see retro_audio_callback
 * @see retro_audio_sample_t
 * @see retro_audio_sample_batch_t
 * @see RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK
 */
#define RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK 22

/**
 * Gets an interface that a core can use to access a controller's rumble motors.
 *
 * The interface supports two independently-controlled motors,
 * one strong and one weak.
 *
 * Should be called from either \c retro_init() or \c retro_load_game(),
 * but not from \c retro_set_environment().
 *
 * @param[out] data <tt>struct retro_rumble_interface *</tt>.
 * Pointer to the interface struct.
 * Behavior is undefined if \c NULL.
 * @returns \c true if the environment call is available,
 * even if the current device doesn't support vibration.
 * @see retro_rumble_interface
 * @defgroup GET_RUMBLE_INTERFACE Rumble Interface
 */
#define RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE 23

/**
 * Returns the frontend's supported input device types.
 *
 * The supported device types are returned as a bitmask,
 * with each value of \ref RETRO_DEVICE corresponding to a bit.
 *
 * Should only be called in \c retro_run().
 *
 * @code
 * #define REQUIRED_DEVICES ((1 << RETRO_DEVICE_JOYPAD) | (1 << RETRO_DEVICE_ANALOG))
 * void get_input_device_capabilities_example(void)
 * {
 *    uint64_t capabilities;
 *    environ_cb(RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES, &capabilities);
 *    if ((capabilities & REQUIRED_DEVICES) == REQUIRED_DEVICES)
 *      printf("Joypad and analog device types are supported");
 * }
 * @endcode
 *
 * @param[out] data <tt>uint64_t *</tt>.
 * Pointer to a bitmask of supported input device types.
 * If the frontend supports a particular \c RETRO_DEVICE_* type,
 * then the bit <tt>(1 << RETRO_DEVICE_*)</tt> will be set.
 *
 * Each bit represents a \c RETRO_DEVICE constant,
 * e.g. bit 1 represents \c RETRO_DEVICE_JOYPAD,
 * bit 2 represents \c RETRO_DEVICE_MOUSE, and so on.
 *
 * Bits that do not correspond to known device types will be set to zero
 * and are reserved for future use.
 *
 * Behavior is undefined if \c NULL.
 * @returns \c true if the environment call is available.
 * @note If the frontend supports multiple input drivers,
 * availability of this environment call (and the reported capabilities)
 * may depend on the active driver.
 * @see RETRO_DEVICE
 */
#define RETRO_ENVIRONMENT_GET_INPUT_DEVICE_CAPABILITIES 24

/**
 * Returns an interface that the core can use to access and configure available sensors,
 * such as an accelerometer or gyroscope.
 *
 * @param[out] data <tt>struct retro_sensor_interface *</tt>.
 * Pointer to the sensor interface that the frontend will populate.
 * Behavior is undefined if is \c NULL.
 * @returns \c true if the environment call is available,
 * even if the device doesn't have any supported sensors.
 * @see retro_sensor_interface
 * @see retro_sensor_action
 * @see RETRO_SENSOR
 * @addtogroup RETRO_SENSOR
 */
#define RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE (25 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Gets an interface to the device's video camera.
 *
 * The frontend delivers new video frames via a user-defined callback
 * that runs in the same thread as \c retro_run().
 * Should be called in \c retro_load_game().
 *
 * @param[in,out] data <tt>struct retro_camera_callback *</tt>.
 * Pointer to the camera driver interface.
 * Some fields in the struct must be filled in by the core,
 * others are provided by the frontend.
 * Behavior is undefined if \c NULL.
 * @returns \c true if this environment call is available,
 * even if an actual camera isn't.
 * @note This API only supports one video camera at a time.
 * If the device provides multiple cameras (e.g. inner/outer cameras on a phone),
 * the frontend will choose one to use.
 * @see retro_camera_callback
 * @see RETRO_ENVIRONMENT_SET_HW_RENDER
 */
#define RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE (26 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Gets an interface that the core can use for cross-platform logging.
 * Certain platforms don't have a console or <tt>stderr</tt>,
 * or they have their own preferred logging methods.
 * The frontend itself may also display log output.
 *
 * @attention This should not be used for information that the player must immediately see,
 * such as major errors or warnings.
 * In most cases, this is best for information that will help you (the developer)
 * identify problems when debugging or providing support.
 * Unless a core or frontend is intended for advanced users,
 * the player might not check (or even know about) their logs.
 *
 * @param[out] data <tt>struct retro_log_callback *</tt>.
 * Pointer to the callback where the function pointer will be saved.
 * Behavior is undefined if \c data is <tt>NULL</tt>.
 * @returns \c true if the environment call is available.
 * @see retro_log_callback
 * @note Cores can fall back to \c stderr if this interface is not available.
 */
#define RETRO_ENVIRONMENT_GET_LOG_INTERFACE 27

/**
 * Returns an interface that the core can use for profiling code
 * and to access performance-related information.
 *
 * This callback supports performance counters, a high-resolution timer,
 * and listing available CPU features (mostly SIMD instructions).
 *
 * @param[out] data <tt>struct retro_perf_callback *</tt>.
 * Pointer to the callback interface.
 * Behavior is undefined if \c NULL.
 * @returns \c true if the environment call is available.
 * @see retro_perf_callback
 */
#define RETRO_ENVIRONMENT_GET_PERF_INTERFACE 28

/**
 * Returns an interface that the core can use to retrieve the device's location,
 * including its current latitude and longitude.
 *
 * @param[out] data <tt>struct retro_location_callback *</tt>.
 * Pointer to the callback interface.
 * Behavior is undefined if \c NULL.
 * @return \c true if the environment call is available,
 * even if there's no location information available.
 * @see retro_location_callback
 */
#define RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE 29

/**
 * @deprecated An obsolete alias to \c RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY kept for compatibility.
 * @see RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY
 **/
#define RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY 30

/**
 * Returns the frontend's "core assets" directory,
 * which can be used to store assets that the core needs
 * such as art assets or level data.
 *
 * @param[out] data <tt>const char **</tt>.
 * Pointer to a string in which the core assets directory will be saved.
 * This string is managed by the frontend and must not be modified or freed by the core.
 * May be \c NULL if no core assets directory is defined,
 * in which case the core should find an alternative directory.
 * Behavior is undefined if \c data is <tt>NULL</tt>.
 * @returns \c true if the environment call is available,
 * even if the value returned in \c data is <tt>NULL</tt>.
 */
#define RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY 30

/**
 * Returns the frontend's save data directory, if available.
 * This directory should be used to store game-specific save data,
 * including memory card images.
 *
 * Although libretro provides an interface for cores to expose SRAM to the frontend,
 * not all cores can support it correctly.
 * In this case, cores should use this environment callback
 * to save their game data to disk manually.
 *
 * Cores that use this environment callback
 * should flush their save data to disk periodically and when unloading.
 *
 * @param[out] data <tt>const char **</tt>.
 * Pointer to the string in which the save data directory will be saved.
 * This string is managed by the frontend and must not be modified or freed by the core.
 * May return \c NULL if no save data directory is defined.
 * Behavior is undefined if \c data is <tt>NULL</tt>.
 * @returns \c true if the environment call is available,
 * even if the value returned in \c data is <tt>NULL</tt>.
 * @note Early libretro cores used \c RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY for save data.
 * This is still supported for backwards compatibility,
 * but new cores should use this environment call instead.
 * \c RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY should be used for game-agnostic data
 * such as BIOS files or core-specific configuration.
 * @note The returned directory may or may not be the same
 * as the one used for \c retro_get_memory_data.
 *
 * @see retro_get_memory_data
 * @see RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY
 */
#define RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY 31

/**
 * Sets new video and audio parameters for the core.
 * This can only be called from within <tt>retro_run</tt>.
 *
 * This environment call may entail a full reinitialization of the frontend's audio/video drivers,
 * hence it should \em only be used if the core needs to make drastic changes
 * to audio/video parameters.
 *
 * This environment call should \em not be used when:
 * <ul>
 * <li>Changing the emulated system's internal resolution,
 * within the limits defined by the existing values of \c max_width and \c max_height.
 * Use \c RETRO_ENVIRONMENT_SET_GEOMETRY instead,
 * and adjust \c retro_get_system_av_info to account for
 * supported scale factors and screen layouts
 * when computing \c max_width and \c max_height.
 * Only use this environment call if \c max_width or \c max_height needs to increase.
 * <li>Adjusting the screen's aspect ratio,
 * e.g. when changing the layout of the screen(s).
 * Use \c RETRO_ENVIRONMENT_SET_GEOMETRY or \c RETRO_ENVIRONMENT_SET_ROTATION instead.
 * </ul>
 *
 * The frontend will reinitialize its audio and video drivers within this callback;
 * after that happens, audio and video callbacks will target the newly-initialized driver,
 * even within the same \c retro_run call.
 *
 * This callback makes it possible to support configurable resolutions
 * while avoiding the need to compute the "worst case" values of \c max_width and \c max_height.
 *
 * @param[in] data <tt>const struct retro_system_av_info *</tt>.
 * Pointer to the new video and audio parameters that the frontend should adopt.
 * @returns \c true if the environment call is available
 * and the new av_info struct was accepted.
 * \c false if the environment call is unavailable or \c data is <tt>NULL</tt>.
 * @see retro_system_av_info
 * @see RETRO_ENVIRONMENT_SET_GEOMETRY
 */
#define RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO 32

/**
 * Provides an interface that a frontend can use
 * to get function pointers from the core.
 *
 * This allows cores to define their own extensions to the libretro API,
 * or to expose implementations of a frontend's libretro extensions.
 *
 * @param[in] data <tt>const struct retro_get_proc_address_interface *</tt>.
 * Pointer to the interface that the frontend can use to get function pointers from the core.
 * The frontend must maintain its own copy of this interface.
 * @returns \c true if the environment call is available
 * and the returned interface was accepted.
 * @note The provided interface may be called at any time,
 * even before this environment call returns.
 * @note Extensions should be prefixed with the name of the frontend or core that defines them.
 * For example, a frontend named "foo" that defines a debugging extension
 * should expect the core to define functions prefixed with "foo_debug_".
 * @warning If a core wants to use this environment call,
 * it \em must do so from within \c retro_set_environment().
 * @see retro_get_proc_address_interface
 */
#define RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK 33

/**
 * Registers a core's ability to handle "subsystems",
 * which are secondary platforms that augment a core's primary emulated hardware.
 *
 * A core doesn't need to emulate a secondary platform
 * in order to use it as a subsystem;
 * as long as it can load a secondary file for some practical use,
 * then this environment call is most likely suitable.
 *
 * Possible use cases of a subsystem include:
 *
 * \li Installing software onto an emulated console's internal storage,
 * such as the Nintendo DSi.
 * \li Emulating accessories that are used to support another console's games,
 * such as the Super Game Boy or the N64 Transfer Pak.
 * \li Inserting a secondary ROM into a console
 * that features multiple cartridge ports,
 * such as the Nintendo DS's Slot-2.
 * \li Loading a save data file created and used by another core.
 *
 * Cores should \em not use subsystems for:
 *
 * \li Emulators that support multiple "primary" platforms,
 * such as a Game Boy/Game Boy Advance core
 * or a Sega Genesis/Sega CD/32X core.
 * Use \c retro_system_content_info_override, \c retro_system_info,
 * and/or runtime detection instead.
 * \li Selecting different memory card images.
 * Use dynamically-populated core options instead.
 * \li Different variants of a single console,
 * such the Game Boy vs. the Game Boy Color.
 * Use core options or runtime detection instead.
 * \li Games that span multiple disks.
 * Use \c RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE
 * and m3u-formatted playlists instead.
 * \li Console system files (BIOS, firmware, etc.).
 * Use \c RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY
 * and a common naming convention instead.
 *
 * When the frontend loads a game via a subsystem,
 * it must call \c retro_load_game_special() instead of \c retro_load_game().
 *
 * @param[in] data <tt>const struct retro_subsystem_info *</tt>.
 * Pointer to an array of subsystem descriptors,
 * terminated by a zeroed-out \c retro_subsystem_info struct.
 * The frontend should maintain its own copy
 * of this array and the strings within it.
 * Behavior is undefined if \c NULL.
 * @returns \c true if this environment call is available.
 * @note This environment call \em must be called from within \c retro_set_environment(),
 * as frontends may need the registered information before loading a game.
 * @see retro_subsystem_info
 * @see retro_load_game_special
 */
#define RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO 34

/**
 * Declares one or more types of controllers supported by this core.
 * The frontend may then allow the player to select one of these controllers in its menu.
 *
 * Many consoles had controllers that came in different versions,
 * were extensible with peripherals,
 * or could be held in multiple ways;
 * this environment call can be used to represent these differences
 * and adjust the core's behavior to match.
 *
 * Possible use cases include:
 *
 * \li Supporting different classes of a single controller that supported their own sets of games.
 *     For example, the SNES had two different lightguns (the Super Scope and the Justifier)
 *     whose games were incompatible with each other.
 * \li Representing a platform's alternative controllers.
 *     For example, several platforms had music/rhythm games that included controllers
 *     shaped like musical instruments.
 * \li Representing variants of a standard controller with additional inputs.
 *     For example, numerous consoles in the 90's introduced 6-button controllers for fighting games,
 *     steering wheels for racing games,
 *     or analog sticks for 3D platformers.
 * \li Representing add-ons for consoles or standard controllers.
 *     For example, the 3DS had a Circle Pad Pro attachment that added a second analog stick.
 * \li Selecting different configurations for a single controller.
 *     For example, the Wii Remote could be held sideways like a traditional game pad
 *     or in one hand like a wand.
 * \li Providing multiple ways to simulate the experience of using a particular controller.
 *     For example, the Game Boy Advance featured several games
 *     with motion or light sensors in their cartridges;
 *     a core could provide controller configurations
 *     that allow emulating the sensors with either analog axes
 *     or with their host device's sensors.
 *
 * Should be called in retro_load_game.
 * The frontend must maintain its own copy of the provided array,
 * including all strings and subobjects.
 * A core may exclude certain controllers for known incompatible games.
 *
 * When the frontend changes the active device for a particular port,
 * it must call \c retro_set_controller_port_device() with that port's index
 * and one of the IDs defined in its retro_controller_info::types field.
 *
 * Input ports are generally associated with different players
 * (and the frontend's UI may reflect this with "Player 1" labels),
 * but this is not required.
 * Some games use multiple controllers for a single player,
 * or some cores may use port indexes to represent an emulated console's
 * alternative input peripherals.
 *
 * @param[in] data <tt>const struct retro_controller_info *</tt>.
 * Pointer to an array of controller types defined by this core,
 * terminated by a zeroed-out \c retro_controller_info.
 * Each element of this array represents a controller port on the emulated device.
 * Behavior is undefined if \c NULL.
 * @returns \c true if this environment call is available.
 * @see retro_controller_info
 * @see retro_set_controller_port_device
 * @see RETRO_DEVICE_SUBCLASS
 */
#define RETRO_ENVIRONMENT_SET_CONTROLLER_INFO 35

/**
 * Notifies the frontend of the address spaces used by the core's emulated hardware,
 * and of the memory maps within these spaces.
 * This can be used by the frontend to provide cheats, achievements, or debugging capabilities.
 * Should only be used by emulators, as it makes little sense for game engines.
 *
 * @note Cores should also expose these address spaces
 * through retro_get_memory_data and \c retro_get_memory_size if applicable;
 * this environment call is not intended to replace those two functions,
 * as the emulated hardware may feature memory regions outside of its own address space
 * that are nevertheless useful for the frontend.
 *
 * @param[in] data <tt>const struct retro_memory_map *</tt>.
 * Pointer to a single memory-map listing.
 * The frontend must maintain its own copy of this object and its contents,
 * including strings and nested objects.
 * Behavior is undefined if \c NULL.
 * @returns \c true if this environment call is available.
 * @see retro_memory_map
 * @see retro_get_memory_data
 * @see retro_memory_descriptor
 */
#define RETRO_ENVIRONMENT_SET_MEMORY_MAPS (36 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Resizes the viewport without reinitializing the video driver.
 *
 * Similar to \c RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO,
 * but any changes that would require video reinitialization will not be performed.
 * Can only be called from within \c retro_run().
 *
 * This environment call allows a core to revise the size of the viewport at will,
 * which can be useful for emulated platforms that support dynamic resolution changes
 * or for cores that support multiple screen layouts.
 *
 * A frontend must guarantee that this environment call completes in
 * constant time.
 *
 * @param[in] data <tt>const struct retro_game_geometry *</tt>.
 * Pointer to the new video parameters that the frontend should adopt.
 * \c retro_game_geometry::max_width and \c retro_game_geometry::max_height
 * will be ignored.
 * Behavior is undefined if \c data is <tt>NULL</tt>.
 * @return \c true if the environment call is available.
 * @see RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO
 */
#define RETRO_ENVIRONMENT_SET_GEOMETRY 37

/**
 * Returns the name of the user, if possible.
 * This callback is suitable for cores that offer personalization,
 * such as online facilities or user profiles on the emulated system.
 * @param[out] data <tt>const char **</tt>.
 * Pointer to the user name string.
 * May be \c NULL, in which case the core should use a default name.
 * The returned pointer is owned by the frontend and must not be modified or freed by the core.
 * Behavior is undefined if \c NULL.
 * @returns \c true if the environment call is available,
 * even if the frontend couldn't provide a name.
 */
#define RETRO_ENVIRONMENT_GET_USERNAME 38

/**
 * Returns the frontend's configured language.
 * It can be used to localize the core's UI,
 * or to customize the emulated firmware if applicable.
 *
 * @param[out] data <tt>retro_language *</tt>.
 * Pointer to the language identifier.
 * Behavior is undefined if \c NULL.
 * @returns \c true if the environment call is available.
 * @note The returned language may not be the same as the operating system's language.
 * Cores should fall back to the operating system's language (or to English)
 * if the environment call is unavailable or the returned language is unsupported.
 * @see retro_language
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL
 */
#define RETRO_ENVIRONMENT_GET_LANGUAGE 39

/**
 * Returns a frontend-managed framebuffer
 * that the core may render directly into
 *
 * This environment call is provided as an optimization
 * for cores that use software rendering
 * (i.e. that don't use \refitem RETRO_ENVIRONMENT_SET_HW_RENDER "a graphics hardware API");
 * specifically, the intended use case is to allow a core
 * to render directly into frontend-managed video memory,
 * avoiding the bandwidth use that copying a whole framebuffer from core to video memory entails.
 *
 * Must be called every frame if used,
 * as this may return a different framebuffer each frame
 * (e.g. for swap chains).
 * However, a core may render to a different buffer even if this call succeeds.
 *
 * @param[in,out] data <tt>struct retro_framebuffer *</tt>.
 * Pointer to a frontend's frame buffer and accompanying data.
 * Some fields are set by the core, others are set by the frontend.
 * Only guaranteed to be valid for the duration of the current \c retro_run call,
 * and must not be used afterwards.
 * Behavior is undefined if \c NULL.
 * @return \c true if the environment call was recognized
 * and the framebuffer was successfully returned.
 * @see retro_framebuffer
 */
#define RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER (40 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Returns an interface for accessing the data of specific rendering APIs.
 * Not all hardware rendering APIs support or need this.
 *
 * The details of these interfaces are specific to each rendering API.
 *
 * @note \c retro_hw_render_callback::context_reset must be called by the frontend
 * before this environment call can be used.
 * Additionally, the contents of the returned interface are invalidated
 * after \c retro_hw_render_callback::context_destroyed has been called.
 * @param[out] data <tt>const struct retro_hw_render_interface **</tt>.
 * The render interface for the currently-enabled hardware rendering API, if any.
 * The frontend will store a pointer to the interface at the address provided here.
 * The returned interface is owned by the frontend and must not be modified or freed by the core.
 * Behavior is undefined if \c NULL.
 * @return \c true if this environment call is available,
 * the active graphics API has a libretro rendering interface,
 * and the frontend is able to return said interface.
 * \c false otherwise.
 * @see RETRO_ENVIRONMENT_SET_HW_RENDER
 * @see retro_hw_render_interface
 * @note Since not every libretro-supported hardware rendering API
 * has a \c retro_hw_render_interface implementation,
 * a result of \c false is not necessarily an error.
 */
#define RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE (41 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Explicitly notifies the frontend of whether this core supports achievements.
 * The core must expose its emulated address space via
 * \c retro_get_memory_data or \c RETRO_ENVIRONMENT_GET_MEMORY_MAPS.
 * Must be called before the first call to <tt>retro_run</tt>.
 *
 * If \ref retro_get_memory_data returns a valid address
 * but this environment call is not used,
 * the frontend (at its discretion) may or may not opt in the core to its achievements support.
 * whether this core is opted in to the frontend's achievement support
 * is left to the frontend's discretion.
 * @param[in] data <tt>const bool *</tt>.
 * Pointer to a single \c bool that indicates whether this core supports achievements.
 * Behavior is undefined if \c data is <tt>NULL</tt>.
 * @returns \c true if the environment call is available.
 * @see RETRO_ENVIRONMENT_SET_MEMORY_MAPS
 * @see retro_get_memory_data
 */
#define RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS (42 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Defines an interface that the frontend can use
 * to ask the core for the parameters it needs for a hardware rendering context.
 * The exact semantics depend on \ref RETRO_ENVIRONMENT_SET_HW_RENDER "the active rendering API".
 * Will be used some time after \c RETRO_ENVIRONMENT_SET_HW_RENDER is called,
 * but before \c retro_hw_render_callback::context_reset is called.
 *
 * @param[in] data <tt>const struct retro_hw_render_context_negotiation_interface *</tt>.
 * Pointer to the context negotiation interface.
 * Will be populated by the frontend.
 * Behavior is undefined if \c NULL.
 * @return \c true if this environment call is supported,
 * even if the current graphics API doesn't use
 * a context negotiation interface (in which case the argument is ignored).
 * @see retro_hw_render_context_negotiation_interface
 * @see RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT
 * @see RETRO_ENVIRONMENT_SET_HW_RENDER
 */
#define RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE (43 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * The frontend will try to use a "shared" context when setting up a hardware context.
 * Mostly applicable to OpenGL.
 *
 * In order for this to have any effect,
 * the core must call \c RETRO_ENVIRONMENT_SET_HW_RENDER at some point
 * if it hasn't already.
 *
 * @param data Ignored.
 * @returns \c true if the environment call is available
 * and the frontend supports shared hardware contexts.
 */
#define RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT (44 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Returns an interface that the core can use to access the file system.
 * Should be called as early as possible.
 *
 * @param[in,out] data <tt>struct retro_vfs_interface_info *</tt>.
 * Information about the desired VFS interface,
 * as well as the interface itself.
 * Behavior is undefined if \c NULL.
 * @return \c true if this environment call is available
 * and the frontend can provide a VFS interface of the requested version or newer.
 * @see retro_vfs_interface_info
 * @see file_path
 * @see retro_dirent
 * @see file_stream
 */
#define RETRO_ENVIRONMENT_GET_VFS_INTERFACE (45 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Returns a list of frontend-authorized filesystem locations.
 *
 * Paths returned by this call must be directly usable with the VFS interface,
 * for example saf://... on Android.
 *
 * @param[out] data <tt>struct retro_vfs_authorized_locations *</tt>.
 * The frontend owns the returned pointers. The core must copy strings
 * if it needs to retain them.
 * If \c data is \c NULL, the frontend should only return whether this
 * environment callback is available.
 *
 * @return \c true if this environment call is available,
 * \c false otherwise.
 * @see RETRO_ENVIRONMENT_GET_VFS_INTERFACE
 */
#define RETRO_ENVIRONMENT_GET_VFS_AUTHORIZED_LOCATIONS (93 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Returns an interface that the core can use
 * to set the state of any accessible device LEDs.
 *
 * @param[out] data <tt>struct retro_led_interface *</tt>.
 * Pointer to the LED interface that the frontend will populate.
 * May be \c NULL, in which case the frontend will only return
 * whether this environment callback is available.
 * @returns \c true if the environment call is available,
 * even if \c data is \c NULL
 * or no LEDs are accessible.
 * @see retro_led_interface
 */
#define RETRO_ENVIRONMENT_GET_LED_INTERFACE (46 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Returns hints about certain steps that the core may skip for this frame.
 *
 * A frontend may not need a core to generate audio or video in certain situations;
 * this environment call sets a bitmask that indicates
 * which steps the core may skip for this frame.
 *
 * This can be used to increase performance for some frontend features.
 *
 * @note Emulation accuracy should not be compromised;
 * for example, if a core emulates a platform that supports display capture
 * (i.e. looking at its own VRAM), then it should perform its rendering as normal
 * unless it can prove that the emulated game is not using display capture.
 *
 * @param[out] data <tt>retro_av_enable_flags *</tt>.
 * Pointer to the bitmask of steps that the frontend will skip.
 * Other bits are set to zero and are reserved for future use.
 * If \c NULL, the frontend will only return whether this environment callback is available.
 * @returns \c true if the environment call is available,
 * regardless of the value output to \c data.
 * If \c false, the core should assume that the frontend will not skip any steps.
 * @see retro_av_enable_flags
 */
#define RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE (47 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Gets an interface that the core can use for raw MIDI I/O.
 *
 * @param[out] data <tt>struct retro_midi_interface *</tt>.
 * Pointer to the MIDI interface.
 * May be \c NULL.
 * @return \c true if the environment call is available,
 * even if \c data is \c NULL.
 * @see retro_midi_interface
 */
#define RETRO_ENVIRONMENT_GET_MIDI_INTERFACE (48 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Asks the frontend if it's currently in fast-forward mode.
 * @param[out] data <tt>bool *</tt>.
 * Set to \c true if the frontend is currently fast-forwarding its main loop.
 * Behavior is undefined if \c data is <tt>NULL</tt>.
 * @returns \c true if this environment call is available,
 * regardless of the value returned in \c data.
 *
 * @see RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE
 */
#define RETRO_ENVIRONMENT_GET_FASTFORWARDING (49 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Returns the refresh rate the frontend is targeting, in Hz.
 * The intended use case is for the core to use the result to select an ideal refresh rate.
 *
 * @param[out] data <tt>float *</tt>.
 * Pointer to the \c float in which the frontend will store its target refresh rate.
 * Behavior is undefined if \c data is <tt>NULL</tt>.
 * @return \c true if this environment call is available,
 * regardless of the value returned in \c data.
*/
#define RETRO_ENVIRONMENT_GET_TARGET_REFRESH_RATE (50 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Returns whether the frontend can return the state of all buttons at once as a bitmask,
 * rather than requiring a series of individual calls to \c retro_input_state_t.
 *
 * If this callback returns \c true,
 * you can get the state of all buttons by passing \c RETRO_DEVICE_ID_JOYPAD_MASK
 * as the \c id parameter to \c retro_input_state_t.
 * Bit #N represents the RETRO_DEVICE_ID_JOYPAD constant of value N,
 * e.g. <tt>(1 << RETRO_DEVICE_ID_JOYPAD_A)</tt> represents the A button.
 *
 * @param data Ignored.
 * @returns \c true if the frontend can report the complete digital joypad state as a bitmask.
 * @see retro_input_state_t
 * @see RETRO_DEVICE_JOYPAD
 * @see RETRO_DEVICE_ID_JOYPAD_MASK
 */
#define RETRO_ENVIRONMENT_GET_INPUT_BITMASKS (51 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Returns the version of the core options API supported by the frontend.
 *
 * Over the years, libretro has used several interfaces
 * for allowing cores to define customizable options.
 * \ref SET_CORE_OPTIONS_V2 "Version 2 of the interface"
 * is currently preferred due to its extra features,
 * but cores and frontends should strive to support
 * versions \ref RETRO_ENVIRONMENT_SET_CORE_OPTIONS "1"
 * and \ref RETRO_ENVIRONMENT_SET_VARIABLES "0" as well.
 * This environment call provides the information that cores need for that purpose.
 *
 * If this environment call returns \c false,
 * then the core should assume version 0 of the core options API.
 *
 * @param[out] data <tt>unsigned *</tt>.
 * Pointer to the integer that will store the frontend's
 * supported core options API version.
 * Behavior is undefined if \c NULL.
 * @returns \c true if the environment call is available,
 * \c false otherwise.
 * @see RETRO_ENVIRONMENT_SET_VARIABLES
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 */
#define RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION 52

/**
 * @copybrief RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 *
 * @deprecated This environment call has been superseded
 * by RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2,
 * which supports categorizing options into groups.
 * This environment call should only be used to maintain compatibility
 * with older cores and frontends.
 *
 * This environment call is intended to replace \c RETRO_ENVIRONMENT_SET_VARIABLES,
 * and should only be called if \c RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION
 * returns an API version of at least 1.
 *
 * This should be called the first time as early as possible,
 * ideally in \c retro_set_environment (but \c retro_load_game is acceptable).
 * It may then be called again later to update
 * the core's options and their associated values,
 * as long as the number of options doesn't change
 * from the number given in the first call.
 *
 * The core can retrieve option values at any time with \c RETRO_ENVIRONMENT_GET_VARIABLE.
 * If a saved value for a core option doesn't match the option definition's values,
 * the frontend may treat it as incorrect and revert to the default.
 *
 * Core options and their values are usually defined in a large static array,
 * but they may be generated at runtime based on the loaded game or system state.
 * Here are some use cases for that:
 *
 * @li Selecting a particular file from one of the
 *     \ref RETRO_ENVIRONMENT_GET_ASSET_DIRECTORY "frontend's"
 *     \ref RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY "content"
 *     \ref RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY "directories",
 *     such as a memory card image or figurine data file.
 * @li Excluding options that are not relevant to the current game,
 *     for cores that define a large number of possible options.
 * @li Choosing a default value at runtime for a specific game,
 *     such as a BIOS file whose region matches that of the loaded content.
 *
 * @note A guiding principle of libretro's API design is that
 * all common interactions (gameplay, menu navigation, etc.)
 * should be possible without a keyboard.
 * This implies that cores should keep the number of options and values
 * as low as possible.
 *
 * Example entry:
 * @code
 * {
 *     "foo_option",
 *     "Speed hack coprocessor X",
 *     "Provides increased performance at the expense of reduced accuracy",
 *     {
 *         { "false",    NULL },
 *         { "true",     NULL },
 *         { "unstable", "Turbo (Unstable)" },
 *         { NULL, NULL },
 *     },
 *     "false"
 * }
 * @endcode
 *
 * @param[in] data <tt>const struct retro_core_option_definition *</tt>.
 * Pointer to one or more core option definitions,
 * terminated by a \ref retro_core_option_definition whose values are all zero.
 * May be \c NULL, in which case the frontend will remove all existing core options.
 * The frontend must maintain its own copy of this object,
 * including all strings and subobjects.
 * @return \c true if this environment call is available.
 *
 * @see retro_core_option_definition
 * @see RETRO_ENVIRONMENT_GET_VARIABLE
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL
 */
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS 53

/**
 * A variant of \ref RETRO_ENVIRONMENT_SET_CORE_OPTIONS
 * that supports internationalization.
 *
 * @deprecated This environment call has been superseded
 * by \ref RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL,
 * which supports categorizing options into groups
 * (plus translating the groups themselves).
 * Only use this environment call to maintain compatibility
 * with older cores and frontends.
 *
 * This should be called instead of \c RETRO_ENVIRONMENT_SET_CORE_OPTIONS
 * if the core provides translations for its options.
 * General use is largely the same,
 * but see \ref retro_core_options_intl for some important details.
 *
 * @param[in] data <tt>const struct retro_core_options_intl *</tt>.
 * Pointer to a core's option values and their translations.
 * @see retro_core_options_intl
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS
 */
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL 54

/**
 * Notifies the frontend that it should show or hide the named core option.
 *
 * Some core options aren't relevant in all scenarios,
 * such as a submenu for hardware rendering flags
 * when the software renderer is configured.
 * This environment call asks the frontend to stop (or start)
 * showing the named core option to the player.
 * This is only a hint, not a requirement;
 * the frontend may ignore this environment call.
 * By default, all core options are visible.
 *
 * @note This environment call must \em only affect a core option's visibility,
 * not its functionality or availability.
 * \ref RETRO_ENVIRONMENT_GET_VARIABLE "Getting an invisible core option"
 * must behave normally.
 *
 * @param[in] data <tt>const struct retro_core_option_display *</tt>.
 * Pointer to a descriptor for the option that the frontend should show or hide.
 * May be \c NULL, in which case the frontend will only return
 * whether this environment callback is available.
 * @return \c true if this environment call is available,
 * even if \c data is \c NULL
 * or the specified option doesn't exist.
 * @see retro_core_option_display
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK
 */
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY 55

/**
 * Returns the frontend's preferred hardware rendering API.
 * Cores should use this information to decide which API to use with \c RETRO_ENVIRONMENT_SET_HW_RENDER.
 * @param[out] data <tt>retro_hw_context_type *</tt>.
 * Pointer to the hardware context type.
 * Behavior is undefined if \c data is <tt>NULL</tt>.
 * This value will be set even if the environment call returns <tt>false</tt>,
 * unless the frontend doesn't implement it.
 * @returns \c true if the environment call is available
 * and the frontend is able to use a hardware rendering API besides the one returned.
 * If \c false is returned and the core cannot use the preferred rendering API,
 * then it should exit or fall back to software rendering.
 * @note The returned value does not indicate which API is currently in use.
 * For example, the frontend may return \c RETRO_HW_CONTEXT_OPENGL
 * while a Direct3D context from a previous session is active;
 * this would signal that the frontend's current preference is for OpenGL,
 * possibly because the user changed their frontend's video driver while a game is running.
 * @see retro_hw_context_type
 * @see RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE
 * @see RETRO_ENVIRONMENT_SET_HW_RENDER
 */
#define RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER 56

/**
 * Returns the minimum version of the disk control interface supported by the frontend.
 *
 * If this environment call returns \c false or \c data is 0 or greater,
 * then cores may use disk control callbacks
 * with \c RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE.
 * If the reported version is 1 or greater,
 * then cores should use \c RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE instead.
 *
 * @param[out] data <tt>unsigned *</tt>.
 * Pointer to the unsigned integer that the frontend's supported disk control interface version will be stored in.
 * Behavior is undefined if \c NULL.
 * @return \c true if this environment call is available.
 * @see RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE
 */
#define RETRO_ENVIRONMENT_GET_DISK_CONTROL_INTERFACE_VERSION 57

/**
 * @copybrief RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE
 *
 * This is intended for multi-disk games that expect the player
 * to manually swap disks at certain points in the game.
 * This version of the disk control interface provides
 * more information about disk images.
 * Should be called in \c retro_init.
 *
 * @param[in] data <tt>const struct retro_disk_control_ext_callback *</tt>.
 * Pointer to the callback functions to use.
 * May be \c NULL, in which case the existing disk callback is deregistered.
 * @return \c true if this environment call is available,
 * even if \c data is \c NULL.
 * @see retro_disk_control_ext_callback
 */
#define RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE 58

/**
 * Returns the version of the message interface supported by the frontend.
 *
 * A version of 0 indicates that the frontend
 * only supports the legacy \c RETRO_ENVIRONMENT_SET_MESSAGE interface.
 * A version of 1 indicates that the frontend
 * supports \c RETRO_ENVIRONMENT_SET_MESSAGE_EXT as well.
 * If this environment call returns \c false,
 * the core should behave as if it had returned 0.
 *
 * @param[out] data <tt>unsigned *</tt>.
 * Pointer to the result returned by the frontend.
 * Behavior is undefined if \c NULL.
 * @return \c true if this environment call is available.
 * @see RETRO_ENVIRONMENT_SET_MESSAGE_EXT
 * @see RETRO_ENVIRONMENT_SET_MESSAGE
 */
#define RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION 59

/**
 * Displays a user-facing message for a short time.
 *
 * Use this callback to convey important status messages,
 * such as errors or the result of long-running operations.
 * For trivial messages or logging, use \c RETRO_ENVIRONMENT_GET_LOG_INTERFACE or \c stderr.
 *
 * This environment call supersedes \c RETRO_ENVIRONMENT_SET_MESSAGE,
 * as it provides many more ways to customize
 * how a message is presented to the player.
 * However, a frontend that supports this environment call
 * must still support \c RETRO_ENVIRONMENT_SET_MESSAGE.
 *
 * @param[in] data <tt>const struct retro_message_ext *</tt>.
 * Pointer to the message to display to the player.
 * Behavior is undefined if \c NULL.
 * @returns \c true if this environment call is available.
 * @see retro_message_ext
 * @see RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION
 */
#define RETRO_ENVIRONMENT_SET_MESSAGE_EXT 60

/**
 * Returns the number of active input devices currently provided by the frontend.
 *
 * This may change between frames,
 * but will remain constant for the duration of each frame.
 *
 * If this callback returns \c true,
 * a core need not poll any input device
 * with an index greater than or equal to the returned value.
 *
 * If callback returns \c false,
 * the number of active input devices is unknown.
 * In this case, all input devices should be considered active.
 *
 * @param[out] data <tt>unsigned *</tt>.
 * Pointer to the result returned by the frontend.
 * Behavior is undefined if \c NULL.
 * @return \c true if this environment call is available.
 */
#define RETRO_ENVIRONMENT_GET_INPUT_MAX_USERS 61

/**
 * Registers a callback that the frontend can use to notify the core
 * of the audio output buffer's occupancy.
 * Can be used by a core to attempt frame-skipping to avoid buffer under-runs
 * (i.e. "crackling" sounds).
 *
 * @param[in] data <tt>const struct retro_audio_buffer_status_callback *</tt>.
 * Pointer to the the buffer status callback,
 * or \c NULL to unregister any existing callback.
 * @return \c true if this environment call is available,
 * even if \c data is \c NULL.
 *
 * @see retro_audio_buffer_status_callback
 */
#define RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK 62

/**
 * Requests a minimum frontend audio latency in milliseconds.
 *
 * This is a hint; the frontend may assign a different audio latency
 * to accommodate hardware limits,
 * although it should try to honor requests up to 512ms.
 *
 * This callback has no effect if the requested latency
 * is less than the frontend's current audio latency.
 * If value is zero or \c data is \c NULL,
 * the frontend should set its default audio latency.
 *
 * May be used by a core to increase audio latency and
 * reduce the risk of buffer under-runs (crackling)
 * when performing 'intensive' operations.
 *
 * A core using RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK
 * to implement audio-buffer-based frame skipping can get good results
 * by setting the audio latency to a high (typically 6x or 8x)
 * integer multiple of the expected frame time.
 *
 * This can only be called from within \c retro_run().
 *
 * @warning This environment call may require the frontend to reinitialize its audio system.
 * This environment call should be used sparingly.
 * If the driver is reinitialized,
 * \ref retro_audio_callback_t "all audio callbacks" will be updated
 * to target the newly-initialized driver.
 *
 * @param[in] data <tt>const unsigned *</tt>.
 * Minimum audio latency, in milliseconds.
 * @return \c true if this environment call is available,
 * even if \c data is \c NULL.
 *
 * @see RETRO_ENVIRONMENT_SET_AUDIO_BUFFER_STATUS_CALLBACK
 */
#define RETRO_ENVIRONMENT_SET_MINIMUM_AUDIO_LATENCY 63

/**
 * Allows the core to tell the frontend when it should enable fast-forwarding,
 * rather than relying solely on the frontend and user interaction.
 *
 * Possible use cases include:
 *
 * \li Temporarily disabling a core's fastforward support
 *     while investigating a related bug.
 * \li Disabling fastforward during netplay sessions,
 *     or when using an emulated console's network features.
 * \li Automatically speeding up the game when in a loading screen
 *     that cannot be shortened with high-level emulation.
 *
 * @param[in] data <tt>const struct retro_fastforwarding_override *</tt>.
 * Pointer to the parameters that decide when and how
 * the frontend is allowed to enable fast-forward mode.
 * May be \c NULL, in which case the frontend will return \c true
 * without updating the fastforward state,
 * which can be used to detect support for this environment call.
 * @return \c true if this environment call is available,
 * even if \c data is \c NULL.
 *
 * @see retro_fastforwarding_override
 * @see RETRO_ENVIRONMENT_GET_FASTFORWARDING
 */
#define RETRO_ENVIRONMENT_SET_FASTFORWARDING_OVERRIDE 64

#define RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE 65
                                           /* const struct retro_system_content_info_override * --
                                            * Allows an implementation to override 'global' content
                                            * info parameters reported by retro_get_system_info().
                                            * Overrides also affect subsystem content info parameters
                                            * set via RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO.
                                            * This function must be called inside retro_set_environment().
                                            * If callback returns false, content info overrides
                                            * are unsupported by the frontend, and will be ignored.
                                            * If callback returns true, extended game info may be
                                            * retrieved by calling RETRO_ENVIRONMENT_GET_GAME_INFO_EXT
                                            * in retro_load_game() or retro_load_game_special().
                                            *
                                            * 'data' points to an array of retro_system_content_info_override
                                            * structs terminated by a { NULL, false, false } element.
                                            * If 'data' is NULL, no changes will be made to the frontend;
                                            * a core may therefore pass NULL in order to test whether
                                            * the RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE and
                                            * RETRO_ENVIRONMENT_GET_GAME_INFO_EXT callbacks are supported
                                            * by the frontend.
                                            *
                                            * For struct member descriptions, see the definition of
                                            * struct retro_system_content_info_override.
                                            *
                                            * Example:
                                            *
                                            * - struct retro_system_info:
                                            * {
                                            *    "My Core",                      // library_name
                                            *    "v1.0",                         // library_version
                                            *    "m3u|md|cue|iso|chd|sms|gg|sg", // valid_extensions
                                            *    true,                           // need_fullpath
                                            *    false                           // block_extract
                                            * }
                                            *
                                            * - Array of struct retro_system_content_info_override:
                                            * {
                                            *    {
                                            *       "md|sms|gg", // extensions
                                            *       false,       // need_fullpath
                                            *       true         // persistent_data
                                            *    },
                                            *    {
                                            *       "sg",        // extensions
                                            *       false,       // need_fullpath
                                            *       false        // persistent_data
                                            *    },
                                            *    { NULL, false, false }
                                            * }
                                            *
                                            * Result:
                                            * - Files of type m3u, cue, iso, chd will not be
                                            *   loaded by the frontend. Frontend will pass a
                                            *   valid path to the core, and core will handle
                                            *   loading internally
                                            * - Files of type md, sms, gg will be loaded by
                                            *   the frontend. A valid memory buffer will be
                                            *   passed to the core. This memory buffer will
                                            *   remain valid until retro_deinit() returns
                                            * - Files of type sg will be loaded by the frontend.
                                            *   A valid memory buffer will be passed to the core.
                                            *   This memory buffer will remain valid until
                                            *   retro_load_game() (or retro_load_game_special())
                                            *   returns
                                            *
                                            * NOTE: If an extension is listed multiple times in
                                            * an array of retro_system_content_info_override
                                            * structs, only the first instance will be registered
                                            */

#define RETRO_ENVIRONMENT_GET_GAME_INFO_EXT 66
                                           /* const struct retro_game_info_ext ** --
                                            * Allows an implementation to fetch extended game
                                            * information, providing additional content path
                                            * and memory buffer status details.
                                            * This function may only be called inside
                                            * retro_load_game() or retro_load_game_special().
                                            * If callback returns false, extended game information
                                            * is unsupported by the frontend. In this case, only
                                            * regular retro_game_info will be available.
                                            * RETRO_ENVIRONMENT_GET_GAME_INFO_EXT is guaranteed
                                            * to return true if RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE
                                            * returns true.
                                            *
                                            * 'data' points to an array of retro_game_info_ext structs.
                                            *
                                            * For struct member descriptions, see the definition of
                                            * struct retro_game_info_ext.
                                            *
                                            * - If function is called inside retro_load_game(),
                                            *   the retro_game_info_ext array is guaranteed to
                                            *   have a size of 1 - i.e. the returned pointer may
                                            *   be used to access directly the members of the
                                            *   first retro_game_info_ext struct, for example:
                                            *
                                            *      struct retro_game_info_ext *game_info_ext;
                                            *      if (environ_cb(RETRO_ENVIRONMENT_GET_GAME_INFO_EXT, &game_info_ext))
                                            *         printf("Content Directory: %s\n", game_info_ext->dir);
                                            *
                                            * - If the function is called inside retro_load_game_special(),
                                            *   the retro_game_info_ext array is guaranteed to have a
                                            *   size equal to the num_info argument passed to
                                            *   retro_load_game_special()
                                            */

/**
 * Defines a set of core options that can be shown and configured by the frontend,
 * so that the player may customize their gameplay experience to their liking.
 *
 * @note This environment call is intended to replace
 * \c RETRO_ENVIRONMENT_SET_VARIABLES and \c RETRO_ENVIRONMENT_SET_CORE_OPTIONS,
 * and should only be called if \c RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION
 * returns an API version of at least 2.
 *
 * This should be called the first time as early as possible,
 * ideally in \c retro_set_environment (but \c retro_load_game is acceptable).
 * It may then be called again later to update
 * the core's options and their associated values,
 * as long as the number of options doesn't change
 * from the number given in the first call.
 *
 * The core can retrieve option values at any time with \c RETRO_ENVIRONMENT_GET_VARIABLE.
 * If a saved value for a core option doesn't match the option definition's values,
 * the frontend may treat it as incorrect and revert to the default.
 *
 * Core options and their values are usually defined in a large static array,
 * but they may be generated at runtime based on the loaded game or system state.
 * Here are some use cases for that:
 *
 * @li Selecting a particular file from one of the
 *     \ref RETRO_ENVIRONMENT_GET_ASSET_DIRECTORY "frontend's"
 *     \ref RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY "content"
 *     \ref RETRO_ENVIRONMENT_GET_CORE_ASSETS_DIRECTORY "directories",
 *     such as a memory card image or figurine data file.
 * @li Excluding options that are not relevant to the current game,
 *     for cores that define a large number of possible options.
 * @li Choosing a default value at runtime for a specific game,
 *     such as a BIOS file whose region matches that of the loaded content.
 *
 * @note A guiding principle of libretro's API design is that
 * all common interactions (gameplay, menu navigation, etc.)
 * should be possible without a keyboard.
 * This implies that cores should keep the number of options and values
 * as low as possible.
 *
 * @param[in] data <tt>const struct retro_core_options_v2 *</tt>.
 * Pointer to a core's options and their associated categories.
 * May be \c NULL, in which case the frontend will remove all existing core options.
 * The frontend must maintain its own copy of this object,
 * including all strings and subobjects.
 * @return \c true if this environment call is available
 * and the frontend supports categories.
 * Note that this environment call is guaranteed to successfully register
 * the provided core options,
 * so the return value does not indicate success or failure.
 *
 * @see retro_core_options_v2
 * @see retro_core_option_v2_category
 * @see retro_core_option_v2_definition
 * @see RETRO_ENVIRONMENT_GET_VARIABLE
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL
 */
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2 67

/**
 * A variant of \ref RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 * that supports internationalization.
 *
 * This should be called instead of \c RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 * if the core provides translations for its options.
 * General use is largely the same,
 * but see \ref retro_core_options_v2_intl for some important details.
 *
 * @param[in] data <tt>const struct retro_core_options_v2_intl *</tt>.
 * Pointer to a core's option values and categories,
 * plus a translation for each option and category.
 * @see retro_core_options_v2_intl
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 */
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL 68

/**
 * Registers a callback that the frontend can use
 * to notify the core that at least one core option
 * should be made hidden or visible.
 * Allows a frontend to signal that a core must update
 * the visibility of any dynamically hidden core options,
 * and enables the frontend to detect visibility changes.
 * Used by the frontend to update the menu display status
 * of core options without requiring a call of retro_run().
 * Must be called in retro_set_environment().
 *
 * @param[in] data <tt>const struct retro_core_options_update_display_callback *</tt>.
 * The callback that the frontend should use.
 * May be \c NULL, in which case the frontend will unset any existing callback.
 * Can be used to query visibility support.
 * @return \c true if this environment call is available,
 * even if \c data is \c NULL.
 * @see retro_core_options_update_display_callback
 */
#define RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK 69

/**
 * Forcibly sets a core option's value.
 *
 * After changing a core option value with this callback,
 * it will be reflected in the frontend
 * and \ref RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE will return \c true.
 * \ref retro_variable::key must match
 * a \ref RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2 "previously-set core option",
 * and \ref retro_variable::value must match one of its defined values.
 *
 * Possible use cases include:
 *
 * @li Allowing the player to set certain core options
 *     without entering the frontend's option menu,
 *     using an in-core hotkey.
 * @li Adjusting invalid combinations of settings.
 * @li Migrating settings from older releases of a core.
 *
 * @param[in] data <tt>const struct retro_variable *</tt>.
 * Pointer to a single option that the core is changing.
 * May be \c NULL, in which case the frontend will return \c true
 * to indicate that this environment call is available.
 * @return \c true if this environment call is available
 * and the option named by \c key was successfully
 * set to the given \c value.
 * \c false if the \c key or \c value fields are \c NULL, empty,
 * or don't match a previously set option.
 *
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 * @see RETRO_ENVIRONMENT_GET_VARIABLE
 * @see RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE
 */
#define RETRO_ENVIRONMENT_SET_VARIABLE 70

#define RETRO_ENVIRONMENT_GET_THROTTLE_STATE (71 | RETRO_ENVIRONMENT_EXPERIMENTAL)
                                           /* struct retro_throttle_state * --
                                            * Allows an implementation to get details on the actual rate
                                            * the frontend is attempting to call retro_run().
                                            */

/**
 * Returns information about how the frontend will use savestates.
 *
 * @param[out] data <tt>retro_savestate_context *</tt>.
 * Pointer to the current savestate context.
 * May be \c NULL, in which case the environment call
 * will return \c true to indicate its availability.
 * @returns \c true if the environment call is available,
 * even if \c data is \c NULL.
 * @see retro_savestate_context
 */
#define RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT (72 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Before calling \c SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE, will query which interface is supported.
 *
 * Frontend looks at \c retro_hw_render_interface_type and returns the maximum supported
 * context negotiation interface version. If the \c retro_hw_render_interface_type is not
 * supported or recognized by the frontend, a version of 0 must be returned in
 * \c retro_hw_render_interface's \c interface_version and \c true is returned by frontend.
 *
 * If this environment call returns true with a \c interface_version greater than 0,
 * a core can always use a negotiation interface version larger than what the frontend returns,
 * but only earlier versions of the interface will be used by the frontend.
 *
 * A frontend must not reject a negotiation interface version that is larger than what the
 * frontend supports. Instead, the frontend will use the older entry points that it recognizes.
 * If this is incompatible with a particular core's requirements, it can error out early.
 *
 * @note Regarding backwards compatibility, this environment call was introduced after Vulkan v1
 * context negotiation. If this environment call is not supported by frontend, i.e. the environment
 * call returns \c false , only Vulkan v1 context negotiation is supported (if Vulkan HW rendering
 * is supported at all). If a core uses Vulkan negotiation interface with version > 1, negotiation
 * may fail unexpectedly. All future updates to the context negotiation interface implies that
 * frontend must support this environment call to query support.
 *
 * @param[out] data <tt>struct retro_hw_render_context_negotiation_interface *</tt>.
 * @return \c true if the environment call is available.
 * @see SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE
 * @see retro_hw_render_interface_type
 * @see retro_hw_render_context_negotiation_interface
 */
#define RETRO_ENVIRONMENT_GET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_SUPPORT (73 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Asks the frontend whether JIT compilation can be used.
 * Primarily used by iOS and tvOS.
 * @param[out] data <tt>bool *</tt>.
 * Set to \c true if the frontend has verified that JIT compilation is possible.
 * @return \c true if the environment call is available.
 */
#define RETRO_ENVIRONMENT_GET_JIT_CAPABLE 74

/**
 * Returns an interface that the core can use to receive microphone input.
 *
 * @param[out] data <tt>retro_microphone_interface *</tt>.
 * Pointer to the microphone interface.
 * @return \true if microphone support is available,
 * even if no microphones are plugged in.
 * \c false if microphone support is disabled unavailable,
 * or if \c data is \c NULL.
 * @see retro_microphone_interface
 */
#define RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE (75 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/* Environment 76 was an obsolete version of RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE.
* It was not used by any known core at the time, and was removed from the API. */

/**
 * Returns the device's current power state as reported by the frontend.
 *
 * This is useful for emulating the battery level in handheld consoles,
 * or for reducing power consumption when on battery power.
 *
 * @note This environment call describes the power state for the entire device,
 * not for individual peripherals like controllers.
 *
 * @param[out] data <struct retro_device_power *>.
 * Indicates whether the frontend can provide this information, even if the parameter
 * is \c NULL. If the frontend does not support this functionality, then the provided
 * argument will remain unchanged.
 * @return \c true if the environment call is available.
 * @see retro_device_power
 */
#define RETRO_ENVIRONMENT_GET_DEVICE_POWER (77 | RETRO_ENVIRONMENT_EXPERIMENTAL)

#define RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE 78
                                           /* const struct retro_netpacket_callback * --
                                            * When set, a core gains control over network packets sent and
                                            * received during a multiplayer session. This can be used to
                                            * emulate multiplayer games that were originally played on two
                                            * or more separate consoles or computers connected together.
                                            *
                                            * The frontend will take care of connecting players together,
                                            * and the core only needs to send the actual data as needed for
                                            * the emulation, while handshake and connection management happen
                                            * in the background.
                                            *
                                            * When two or more players are connected and this interface has
                                            * been set, time manipulation features (such as pausing, slow motion,
                                            * fast forward, rewinding, save state loading, etc.) are disabled to
                                            * avoid interrupting communication.
                                            *
                                            * Should be set in either retro_init or retro_load_game, but not both.
                                            *
                                            * When not set, a frontend may use state serialization-based
                                            * multiplayer, where a deterministic core supporting multiple
                                            * input devices does not need to take any action on its own.
                                            */

/**
 * Returns the device's current power state as reported by the frontend.
 * This is useful for emulating the battery level in handheld consoles,
 * or for reducing power consumption when on battery power.
 *
 * The return value indicates whether the frontend can provide this information,
 * even if the parameter is \c NULL.
 *
 * If the frontend does not support this functionality,
 * then the provided argument will remain unchanged.
 * @param[out] data <tt>retro_device_power *</tt>.
 * Pointer to the information that the frontend returns about its power state.
 * May be \c NULL.
 * @return \c true if the environment call is available,
 * even if \c data is \c NULL.
 * @see retro_device_power
 * @note This environment call describes the power state for the entire device,
 * not for individual peripherals like controllers.
*/
#define RETRO_ENVIRONMENT_GET_DEVICE_POWER (77 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Returns the "playlist" directory of the frontend.
 *
 * This directory can be used to store core generated playlists, in case
 * this internal functionality is available (e.g. internal core game detection
 * engine).
 *
 * @param[out] data <tt>const char **</tt>.
 * May be \c NULL. If so, no such directory is defined, and it's up to the
 * implementation to find a suitable directory.
 * @return \c true if the environment call is available.
 */
#define RETRO_ENVIRONMENT_GET_PLAYLIST_DIRECTORY 79

/**
 * Returns the "file browser" start directory of the frontend.
 *
 * This directory can serve as a start directory for the core in case it
 * provides an internal way of loading content.
 *
 * @param[out] data <tt>const char **</tt>.
 * May be \c NULL. If so, no such directory is defined, and it's up to the
 * implementation to find a suitable directory.
 * @return \c true if the environment call is available.
 */
#define RETRO_ENVIRONMENT_GET_FILE_BROWSER_START_DIRECTORY 80

/**
 * Returns the audio sample rate the frontend is targeting, in Hz.
 * The intended use case is for the core to use the result to select an ideal sample rate.
 *
 * @param[out] data <tt>unsigned *</tt>.
 * Pointer to the \c unsigned integer in which the frontend will store its target sample rate.
 * Behavior is undefined if \c data is <tt>NULL</tt>.
 * @return \c true if this environment call is available,
 * regardless of the value returned in \c data.
*/
#define RETRO_ENVIRONMENT_GET_TARGET_SAMPLE_RATE (81 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Returns the local player's netplay client index when using frontend-managed
 * multiplayer/rollback netplay.
 *
 * @param[out] data <tt>unsigned *</tt>.
 * Pointer to an unsigned integer where the frontend stores the local client index.
 * 0 indicates host. Values > 0 indicate connected clients.
 * @return \\c true if the environment call is available and value was written,
 * \\c false otherwise.
*/
#define RETRO_ENVIRONMENT_GET_NETPLAY_CLIENT_INDEX (82 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Allocates a region of executable memory, optionally dual-mapped.
 *
 * The frontend allocates memory suitable for JIT code generation and returns
 * it to the core.  The returned mode tells the core how to use the memory:
 *
 *  - \c RETRO_EXEC_MEM_MODE_UNRESTRICTED: The platform has no restrictions
 *    on executable memory.  The core should self-allocate.  Do not call
 *    this environment with a non-zero size in this mode.
 *  - \c RETRO_EXEC_MEM_MODE_RWX: Single mapping, read-write-execute.
 *    \c rx and \c rw point to the same region.
 *  - \c RETRO_EXEC_MEM_MODE_WX_TOGGLE: Single mapping, write XOR execute.
 *    \c rx and \c rw point to the same region; the core must toggle
 *    protections itself (e.g. mprotect) before writing or executing.
 *  - \c RETRO_EXEC_MEM_MODE_DUAL_MAP: Separate read-execute and read-write
 *    mappings of the same physical pages.  The core writes through \c rw
 *    and executes from \c rx.
 *
 * Returned memory is page-aligned.  The frontend tracks all allocations
 * and frees any outstanding ones when the core is unloaded.
 *
 * The core sets \c version and \c size before calling.  The frontend fills
 * in \c mode, \c rx, and \c rw on success.
 *
 * If \c size is 0, this is a probe: the frontend returns \c true and sets
 * \c mode to indicate what kind of memory it would provide, but does not
 * allocate.  \c rx and \c rw will be NULL.  Cores can use this to decide
 * whether to take a JIT code path before committing to an allocation.
 *
 * @param[in,out] data <tt>struct retro_exec_mem_alloc *</tt>.
 * @return \c true if the allocation succeeded, \c false otherwise.
 *         If the frontend does not support this call, returns \c false
 *         and the core should fall back to managing its own executable memory.
 * @see retro_exec_mem_alloc
 * @see RETRO_ENVIRONMENT_EXEC_MEM_FREE
 */
#define RETRO_ENVIRONMENT_EXEC_MEM_ALLOC 83

/**
 * Frees a region of executable memory previously allocated with
 * \c RETRO_ENVIRONMENT_EXEC_MEM_ALLOC.
 *
 * This is optional; the frontend will free all outstanding allocations
 * when the core is unloaded.  Cores that never need to release memory
 * mid-session need not call this.
 *
 * @param[in] data <tt>struct retro_exec_mem_free *</tt>.
 * @return \c true if the memory was freed, \c false otherwise.
 * @see retro_exec_mem_free
 * @see RETRO_ENVIRONMENT_EXEC_MEM_ALLOC
 */
#define RETRO_ENVIRONMENT_EXEC_MEM_FREE  84

/**
 * Queries whether the frontend can accept audio samples in 32-bit
 * native-endian IEEE-754 float format, and, if so, obtains a float
 * sample-batch callback the core may use in place of the standard
 * int16 \c retro_audio_sample_batch_t callback.
 *
 * Rationale: frontend resamplers and DSP chains operate on float, and
 * most modern audio drivers expose a native float output path. A core
 * whose audio is float-native (e.g. one with a float software mixer or
 * a float decoder) currently has to squash its output down to int16 at
 * the libretro boundary, only for the frontend to immediately widen it
 * back to float. Negotiating float output here removes that redundant
 * int16<->float round-trip on both sides of the boundary.
 *
 * On success the frontend sets \c batch in the supplied
 * \c retro_audio_sample_float_callback. The core may call that function
 * from within \c retro_run() (or from the audio callback registered via
 * \c RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK), passing interleaved stereo
 * frames of float samples normalized to the range [-1.0, 1.0]. The
 * return value has the same meaning as \c retro_audio_sample_batch_t.
 *
 * Contract:
 *  - The core must commit to a single output format for the lifetime of
 *    a loaded game; it must not mix int16 and float batch calls. Perform
 *    negotiation once, during \c retro_load_game() (after any
 *    \c RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK call).
 *  - If this returns \c false the core must keep using the int16
 *    \c retro_audio_sample_batch_t / \c retro_audio_sample_t callbacks.
 *  - The \c batch function pointer is owned by the frontend and remains
 *    valid until \c retro_unload_game().
 *  - Frontends that do not recognize this call return \c false, so older
 *    frontends transparently keep the int16 path.
 *
 * @param[out] data <tt>struct retro_audio_sample_float_callback *</tt>.
 * @return \c true if float audio output is supported, \c false otherwise.
 * @see retro_audio_sample_batch_float_t
 * @see retro_audio_sample_float_callback
 */
#define RETRO_ENVIRONMENT_GET_AUDIO_SAMPLE_BATCH_FLOAT (85 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Queries how much system memory the frontend has available.
 *
 * A core may use this to size large internal allocations (a memory pool,
 * heap or asset cache) to the running machine instead of to a fixed
 * compile-time default. The reported values are advisory snapshots: \c free
 * in particular may include reclaimable cache and can change immediately
 * after the call, so a core should take a fraction of it and clamp the
 * result -- it must never assume it can allocate the whole amount.
 *
 * Frontends that do not implement this return \c false, in which case the
 * core is expected to fall back to its own defaults.
 *
 * @param[out] data <tt>struct retro_memory_status *</tt>.
 * @return \c true if the frontend filled in the structure, \c false otherwise.
 * @see retro_memory_status
 */
#define RETRO_ENVIRONMENT_GET_MEMORY_STATUS (86 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Notifies the frontend of any quirks associated with serialization.
 *
 * Should be set in either \c retro_init or \c retro_load_game, but not both.
 * @param[in, out] data <tt>uint64_t *</tt>.
 * Pointer to the core's serialization quirks.
 * The frontend will set the flags of the quirks it supports
 * and clear the flags of those it doesn't.
 * Behavior is undefined if \c NULL.
 * @return \c true if this environment call is supported.
 * @see retro_serialize
 * @see retro_unserialize
 * @see RETRO_SERIALIZATION_QUIRK
 */
#define RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS 87

/**
 * Queries whether the active video driver can present a 10-bit-per-channel
 * (30-bit) source surface end to end, i.e. whether a frame submitted as
 * #RETRO_PIXEL_FORMAT_XRGB2101010 reaches the display without the frontend
 * narrowing it to 8 bits per channel.
 *
 * Unlike SET_PIXEL_FORMAT, which accepts XRGB2101010 unconditionally and
 * transparently down-converts when the driver cannot present 10-bit, this
 * lets a core discover the real capability so it can avoid pointless work:
 * a core that has both a 10-bit and an 8-bit output path should prefer the
 * 8-bit path when this returns \c false, since emitting 10-bit only to have
 * the frontend narrow it wastes effort and, for content that starts at 8
 * bits, is a no-op round trip; going straight to 8 bits also rounds rather
 * than truncates.
 *
 * The result may change across a driver reinit (e.g. the user switches
 * video driver or toggles HDR), so a core that cares should query it when
 * (re)choosing its pixel format rather than caching it indefinitely.
 *
 * @param[out] data <tt>bool *</tt>.
 * Set to \c true if a 10-bit source surface is presented natively, \c false
 * if XRGB2101010 frames are down-converted to 8-bit.
 * @return \c true if the environment call is recognised (the value at
 * \c data is then valid), \c false if it is unsupported (an older frontend);
 * a core must treat "unsupported" as "no guarantee of native 10-bit".
 */
#define RETRO_ENVIRONMENT_GET_SCREEN_10BPC_CAPABLE (88 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Queries the luminance, in nits, that the frontend treats as SDR paper
 * white when presenting HDR content.
 *
 * Only meaningful together with #RETRO_PIXEL_FORMAT_HDR10_2101010: a core
 * encoding absolute luminance itself has to know where the user expects
 * ordinary, non-glowing image content to sit, otherwise the whole frame is
 * either dim or searing.  The value corresponds to the frontend's HDR paper
 * white setting; typical values are 100-400.
 *
 * The user can change it at any time, so a core should re-query it when it
 * rebuilds its colour tables rather than caching it forever.
 *
 * @param[out] data <tt>float *</tt>.
 * Set to the paper white luminance in nits.
 * @return \c true if the call is recognised and the value is valid, \c false
 * on a frontend that does not implement it; callers should then assume a
 * sensible default (200 nits).
 */
#define RETRO_ENVIRONMENT_GET_HDR_PAPER_WHITE_NITS (89 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Queries the colour-gamut treatment the frontend applies to SDR content
 * when presenting HDR.
 *
 * Only meaningful together with #RETRO_PIXEL_FORMAT_HDR10_2101010.  A core
 * emitting that format performs its own Rec.709 -> Rec.2020 rotation, and
 * has to make the same choice the frontend makes for SDR content, or a
 * scene will change saturation when the user switches the core between an
 * SDR format and HDR10.  The values match the frontend's "Colour Boost"
 * setting:
 *
 *   0 Accurate -- proper Rec.709 -> Rec.2020 conversion, no boost
 *   1 Expanded -- Rec.709 -> a slightly wider space
 *   2 Wide     -- Rec.709 -> DCI-P3
 *   3 Super    -- no rotation at all; values stay Rec.709 and the boost
 *                 comes from the display interpreting them as Rec.2020
 *
 * A core should re-query this when it rebuilds its colour tables, since the
 * user can change it at any time.
 *
 * @param[out] data <tt>unsigned *</tt>.
 * Set to one of the values above.
 * @return \c true if the call is recognised, \c false on a frontend that
 * does not implement it; callers should then assume 0 (Accurate).
 */
#define RETRO_ENVIRONMENT_GET_HDR_EXPAND_GAMUT (90 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Queries which HDR output mode the frontend is presenting with.
 *
 * Only meaningful together with #RETRO_PIXEL_FORMAT_HDR10_2101010.  Both
 * output modes accept the same PQ Rec.2020 frame, but they do not treat its
 * primaries identically: an HDR10 swapchain presents the samples as-is,
 * while an scRGB swapchain converts them and applies a Rec.2020 -> Rec.709
 * rotation on the way.  A core that encodes its own gamut -- which it must,
 * to honour #RETRO_ENVIRONMENT_GET_HDR_EXPAND_GAMUT -- therefore has to know
 * which of the two it is feeding, or its colour choice is undone by that
 * rotation on one of them.
 *
 * Values:
 *   0  HDR output is off
 *   1  HDR10 (PQ, Rec.2020 swapchain); samples are presented unchanged
 *   2  scRGB (linear FP16, Rec.709); the frontend applies Rec.2020 ->
 *      Rec.709 to the decoded samples
 *
 * The user can switch this at any time, so a core should re-query it when it
 * rebuilds its colour tables rather than caching it indefinitely.
 *
 * @param[out] data <tt>unsigned *</tt>.
 * Set to one of the values above.
 * @return \c true if the call is recognised, \c false on a frontend that
 * does not implement it; callers should then assume 1 (HDR10), which is the
 * mode that needs no compensation.
 */
#define RETRO_ENVIRONMENT_GET_HDR_OUTPUT_MODE (91 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Queries the peak luminance the frontend is presenting to, in cd/m2 (nits).
 *
 * Only meaningful together with #RETRO_PIXEL_FORMAT_HDR10_2101010.  A core
 * emitting HDR10 encodes absolute luminance itself, so it decides where its
 * brightest content lands.  #RETRO_ENVIRONMENT_GET_HDR_PAPER_WHITE_NITS says
 * where ordinary content sits; this says how much room there is above it.  The
 * gap between the two is the entire headroom a core has for highlights, and
 * without it a core has to guess - a guess that is too dark on a bright display
 * and clips on a dim one.
 *
 * This is what the *display* can do, not what the content wants, so it is a
 * ceiling to roll off toward rather than a level to target.  A core should not
 * assume anything it emits below this value is reproduced exactly; displays
 * tone map internally, and many report a peak they can only hold over a small
 * window.
 *
 * May be lower than paper white if the user has configured it so.  A core
 * should treat that as zero headroom and clamp to paper white rather than
 * producing a negative range.
 *
 * The user can change this at any time, so a core should re-query it rather
 * than caching it indefinitely.
 *
 * @param[out] data <tt>float *</tt>.
 * Set to the peak luminance in nits.
 * @return \c true if the call is recognised, \c false on a frontend that does
 * not implement it; callers should then fall back to a sensible default.
 * 1000 nits is a reasonable assumption, being both the HDR10 reference peak
 * and roughly what mid-range HDR displays achieve.
 */
#define RETRO_ENVIRONMENT_GET_HDR_MAX_NITS (92 | RETRO_ENVIRONMENT_EXPERIMENTAL)

/**
 * Result of \c RETRO_ENVIRONMENT_GET_MEMORY_STATUS.
 *
 * Sizes are in bytes; a field the frontend cannot determine is left at 0.
 */
struct retro_memory_status
{
   uint64_t free;   /**< Physical memory currently available to allocate. */
   uint64_t total;  /**< Total physical memory installed. */
};

/**@}*/

/**
 * @defgroup GET_VFS_INTERFACE File System Interface
 * @brief File system functionality.
 *
 * @section File Paths
 * File paths passed to all libretro filesystem APIs shall be well formed UNIX-style,
 * using "/" (unquoted forward slash) as the directory separator
 * regardless of the platform's native separator.
 *
 * Paths shall also include at least one forward slash
 * (e.g. use "./game.bin" instead of "game.bin").
 *
 * Other than the directory separator, cores shall not make assumptions about path format.
 * The following paths are all valid:
 * @li \c C:/path/game.bin
 * @li \c http://example.com/game.bin
 * @li \c #game/game.bin
 * @li \c ./game.bin
 *
 * Cores may replace the basename or remove path components from the end, and/or add new components;
 * however, cores shall not append "./", "../" or multiple consecutive forward slashes ("//") to paths they request from the front end.
 *
 * The frontend is encouraged to do the best it can when given an ill-formed path,
 * but it is allowed to give up.
 *
 * Frontends are encouraged, but not required, to support native file system paths
 * (including replacing the directory separator, if applicable).
 *
 * Cores are allowed to try using them, but must remain functional if the frontend rejects such requests.
 *
 * Cores are encouraged to use the libretro-common filestream functions for file I/O,
 * as they seamlessly integrate with VFS,
 * deal with directory separator replacement as appropriate
 * and provide platform-specific fallbacks
 * in cases where front ends do not provide their own VFS interface.
 *
 * @see RETRO_ENVIRONMENT_GET_VFS_INTERFACE
 * @see retro_vfs_interface_info
 * @see file_path
 * @see retro_dirent
 * @see file_stream
 *
 * @{
 */

/**
 * Opaque file handle.
 * @since VFS API v1
 */
struct retro_vfs_file_handle;

/**
 * Opaque directory handle.
 * @since VFS API v3
 */
struct retro_vfs_dir_handle;

/** @defgroup RETRO_VFS_FILE_ACCESS File Access Flags
 * File access flags.
 * @since VFS API v1
 * @{
 */

/** Opens a file for read-only access. */
#define RETRO_VFS_FILE_ACCESS_READ            (1 << 0)

/**
 * Opens a file for write-only access.
 * Any existing file at this path will be discarded and overwritten
 * unless \c RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING is also specified.
 */
#define RETRO_VFS_FILE_ACCESS_WRITE           (1 << 1)

/**
 * Opens a file for reading and writing.
 * Any existing file at this path will be discarded and overwritten
 * unless \c RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING is also specified.
 */
#define RETRO_VFS_FILE_ACCESS_READ_WRITE      (RETRO_VFS_FILE_ACCESS_READ | RETRO_VFS_FILE_ACCESS_WRITE)

/**
 * Opens a file without discarding its existing contents.
 * Only meaningful if \c RETRO_VFS_FILE_ACCESS_WRITE is specified.
 */
#define RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING (1 << 2) /* Prevents discarding content of existing files opened for writing */

/** @} */

/** @defgroup RETRO_VFS_FILE_ACCESS_HINT File Access Hints
 *
 * Hints to the frontend for how a file will be accessed.
 * The VFS implementation may use these to optimize performance,
 * react to external interference (such as concurrent writes),
 * or it may ignore them entirely.
 *
 * Hint flags do not change the behavior of each VFS function
 * unless otherwise noted.
 * @{
 */

/** No particular hints are given. */
#define RETRO_VFS_FILE_ACCESS_HINT_NONE              (0)

/**
 * Indicates that the file will be accessed frequently.
 *
 * The frontend should cache it or map it into memory.
 */
#define RETRO_VFS_FILE_ACCESS_HINT_FREQUENT_ACCESS   (1 << 0)

/**
 * Indicates that the file will be read once, from start to finish,
 * and then closed.
 *
 * No mapping or caching is wanted: the caller already keeps the bytes
 * it asked for, so anything the frontend holds on to beyond the call
 * is dead weight.  A frontend that buffers its reads may wish to skip
 * doing so for such a stream, since a whole-file read gains nothing
 * from being split across a buffer and copied twice.
 *
 * Only meaningful together with \c RETRO_VFS_FILE_ACCESS_READ.
 */
#define RETRO_VFS_FILE_ACCESS_HINT_SEQUENTIAL_BULK   (1 << 1)

/** @} */

/** @defgroup RETRO_VFS_SEEK_POSITION File Seek Positions
 * File access flags and hints.
 * @{
 */

/**
 * Indicates a seek relative to the start of the file.
 */
#define RETRO_VFS_SEEK_POSITION_START    0

/**
 * Indicates a seek relative to the current stream position.
 */
#define RETRO_VFS_SEEK_POSITION_CURRENT  1

/**
 * Indicates a seek relative to the end of the file.
 * @note The offset passed to \c retro_vfs_seek_t should be negative.
 */
#define RETRO_VFS_SEEK_POSITION_END      2

/** @} */

/** @defgroup RETRO_VFS_STAT File Status Flags
 * File stat flags.
 * @see retro_vfs_stat_t
 * @since VFS API v3
 * @{
 */

/** Indicates that the given path refers to a valid file. */
#define RETRO_VFS_STAT_IS_VALID               (1 << 0)

/** Indicates that the given path refers to a directory. */
#define RETRO_VFS_STAT_IS_DIRECTORY           (1 << 1)

/**
 * Indicates that the given path refers to a character special file,
 * such as \c /dev/null.
 */
#define RETRO_VFS_STAT_IS_CHARACTER_SPECIAL   (1 << 2)

/** @} */

/**
 * Returns the path that was used to open this file.
 *
 * @param stream The opened file handle to get the path of.
 * Behavior is undefined if \c NULL or closed.
 * @return The path that was used to open \c stream.
 * The string is owned by \c stream and must not be modified.
 * @since VFS API v1
 * @see filestream_get_path
 */
typedef const char *(RETRO_CALLCONV *retro_vfs_get_path_t)(struct retro_vfs_file_handle *stream);

/**
 * Open a file for reading or writing.
 *
 * @param path The path to open.
 * @param mode A bitwise combination of \c RETRO_VFS_FILE_ACCESS flags.
 * At a minimum, one of \c RETRO_VFS_FILE_ACCESS_READ or \c RETRO_VFS_FILE_ACCESS_WRITE must be specified.
 * If \c RETRO_VFS_FILE_ACCESS_WRITE is specified and \c RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING is not specified,
 * and no file or directory exists at \c path, this function will attempt to create an empty file at \c path.
 * If either \c RETRO_VFS_FILE_ACCESS_WRITE is not specified or \c RETRO_VFS_FILE_ACCESS_UPDATE_EXISTING is specified,
 * and no file or directory exists at \c path, this function will return \c NULL without attempting to create a file at \c path.
 * @param hints A bitwise combination of \c RETRO_VFS_FILE_ACCESS_HINT flags.
 * @return A handle to the opened file,
 * or \c NULL upon failure.
 * Note that this will return \c NULL if \c path names a directory.
 * The returned file handle must be closed with \c retro_vfs_close_t.
 * @since VFS API v1
 * @see File Paths
 * @see RETRO_VFS_FILE_ACCESS
 * @see RETRO_VFS_FILE_ACCESS_HINT
 * @see retro_vfs_close_t
 * @see filestream_open
 */
typedef struct retro_vfs_file_handle *(RETRO_CALLCONV *retro_vfs_open_t)(const char *path, unsigned mode, unsigned hints);

/**
 * Close the file and release its resources.
 * All files returned by \c retro_vfs_open_t must be closed with this function.
 *
 * @param stream The file handle to close.
 * Behavior is undefined if already closed.
 * Upon completion of this function, \c stream is no longer valid
 * (even if it returns failure).
 * @return 0 on success, -1 on failure or if \c stream is \c NULL.
 * @see retro_vfs_open_t
 * @see filestream_close
 * @since VFS API v1
 */
typedef int (RETRO_CALLCONV *retro_vfs_close_t)(struct retro_vfs_file_handle *stream);

/**
 * Return the size of the file in bytes.
 *
 * @param stream The file to query the size of.
 * @return Size of the file in bytes, or -1 if there was an error.
 * @see filestream_get_size
 * @since VFS API v1
 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_size_t)(struct retro_vfs_file_handle *stream);

/**
 * Set the file's length.
 *
 * @param stream The file whose length will be adjusted.
 * @param length The new length of the file, in bytes.
 * If shorter than the original length, the extra bytes will be discarded.
 * If longer, the file's padding is unspecified (and likely platform-dependent).
 * @return 0 on success,
 * -1 on failure.
 * @see filestream_truncate
 * @since VFS API v2
 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_truncate_t)(struct retro_vfs_file_handle *stream, int64_t length);

/**
 * Gets the given file's current read/write position.
 * This position is advanced with each call to \c retro_vfs_read_t or \c retro_vfs_write_t.
 *
 * @param stream The file to query the position of.
 * @return The current stream position in bytes
 * or -1 if there was an error.
 * @see filestream_tell
 * @since VFS API v1
 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_tell_t)(struct retro_vfs_file_handle *stream);

/**
 * Sets the given file handle's current read/write position.
 *
 * @param stream The file to set the position of.
 * @param offset The new position, in bytes.
 * @param seek_position The position to seek from.
 * @return 0 on success, -1 on failure.
 * @since VFS API v1
 * @see File Seek Positions
 * @see filestream_seek
 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_seek_t)(struct retro_vfs_file_handle *stream, int64_t offset, int seek_position);

/**
 * Read data from a file, if it was opened for reading.
 *
 * @param stream The file to read from.
 * @param s The buffer to read into.
 * @param len The number of bytes to read.
 * The buffer pointed to by \c s must be this large.
 * @return The number of bytes read,
 * or -1 if there was an error.
 * @since VFS API v1
 * @see filestream_read
 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_read_t)(struct retro_vfs_file_handle *stream, void *s, uint64_t len);

/**
 * Write data to a file, if it was opened for writing.
 *
 * @param stream The file handle to write to.
 * @param s The buffer to write from.
 * @param len The number of bytes to write.
 * The buffer pointed to by \c s must be this large.
 * @return The number of bytes written,
 * or -1 if there was an error.
 * @since VFS API v1
 * @see filestream_write
 */
typedef int64_t (RETRO_CALLCONV *retro_vfs_write_t)(struct retro_vfs_file_handle *stream, const void *s, uint64_t len);

/**
 * Flush pending writes to the file, if applicable.
 *
 * This does not mean that the changes will be immediately persisted to disk;
 * that may be scheduled for later, depending on the platform.
 *
 * @param stream The file handle to flush.
 * @return 0 on success, -1 on failure.
 * @since VFS API v1
 * @see filestream_flush
 */
typedef int (RETRO_CALLCONV *retro_vfs_flush_t)(struct retro_vfs_file_handle *stream);

/**
 * Deletes the file at the given path.
 *
 * @param path The path to the file that will be deleted.
 * @return 0 on success, -1 on failure.
 * @see filestream_delete
 * @since VFS API v1
 */
typedef int (RETRO_CALLCONV *retro_vfs_remove_t)(const char *path);

/**
 * Rename the specified file.
 *
 * @param old_path Path to an existing file.
 * @param new_path The destination path.
 * Must not name an existing file.
 * @return 0 on success, -1 on failure
 * @see filestream_rename
 * @since VFS API v1
 */
typedef int (RETRO_CALLCONV *retro_vfs_rename_t)(const char *old_path, const char *new_path);

/**
 * Gets information about the given file.
 *
 * @param path The path to the file to query.
 * @param[out] size The reported size of the file in bytes.
 * May be \c NULL, in which case this value is ignored.
 * @return A bitmask of \c RETRO_VFS_STAT flags,
 * or 0 if \c path doesn't refer to a valid file.
 * @see path_stat
 * @see path_get_size
 * @see RETRO_VFS_STAT
 * @since VFS API v3
 */
typedef int (RETRO_CALLCONV *retro_vfs_stat_t)(const char *path, int32_t *size);

/**
 * Gets information about the given file (64-bit size).
 *
 * @param path The path to the file to query.
 * @param[out] size The reported size of the file in bytes.
 * May be \c NULL, in which case this value is ignored.
 * @return A bitmask of \c RETRO_VFS_STAT flags,
 * or 0 if \c path doesn't refer to a valid file.
 * @see RETRO_VFS_STAT
 * @since VFS API v4
 */
typedef int (RETRO_CALLCONV *retro_vfs_stat_64_t)(const char *path, int64_t *size);

/**
 * Creates a directory at the given path.
 *
 * @param dir The desired location of the new directory.
 * @return 0 if the directory was created,
 * -2 if the directory already exists,
 * or -1 if some other error occurred.
 * @see path_mkdir
 * @since VFS API v3
 */
typedef int (RETRO_CALLCONV *retro_vfs_mkdir_t)(const char *dir);

/**
 * Opens a handle to a directory so its contents can be inspected.
 *
 * @param dir The path to the directory to open.
 * Must be an existing directory.
 * @param include_hidden Whether to include hidden files in the directory listing.
 * The exact semantics of this flag will depend on the platform.
 * @return A handle to the opened directory,
 * or \c NULL if there was an error.
 * @see retro_opendir
 * @since VFS API v3
 */
typedef struct retro_vfs_dir_handle *(RETRO_CALLCONV *retro_vfs_opendir_t)(const char *dir, bool include_hidden);

/**
 * Gets the next dirent ("directory entry")
 * within the given directory.
 *
 * @param[in,out] dirstream The directory to read from.
 * Updated to point to the next file, directory, or other path.
 * @return \c true when the next dirent was retrieved,
 * \c false if there are no more dirents to read.
 * @note This API iterates over all files and directories within \c dirstream.
 * Remember to check what the type of the current dirent is.
 * @note This function does not recurse,
 * i.e. it does not return the contents of subdirectories.
 * @note This may include "." and ".." on Unix-like platforms.
 * @see retro_readdir
 * @see retro_vfs_dirent_is_dir_t
 * @since VFS API v3
 */
typedef bool (RETRO_CALLCONV *retro_vfs_readdir_t)(struct retro_vfs_dir_handle *dirstream);

/**
 * Gets the filename of the current dirent.
 *
 * The returned string pointer is valid
 * until the next call to \c retro_vfs_readdir_t or \c retro_vfs_closedir_t.
 *
 * @param dirstream The directory to read from.
 * @return The current dirent's name,
 * or \c NULL if there was an error.
 * @note This function only returns the file's \em name,
 * not a complete path to it.
 * @see retro_dirent_get_name
 * @since VFS API v3
 */
typedef const char *(RETRO_CALLCONV *retro_vfs_dirent_get_name_t)(struct retro_vfs_dir_handle *dirstream);

/**
 * Checks whether the current dirent names a directory.
 *
 * @param dirstream The directory to read from.
 * @return \c true if \c dirstream's current dirent points to a directory,
 * \c false if not or if there was an error.
 * @see retro_dirent_is_dir
 * @since VFS API v3
 */
typedef bool (RETRO_CALLCONV *retro_vfs_dirent_is_dir_t)(struct retro_vfs_dir_handle *dirstream);

/**
 * Closes the given directory and release its resources.
 *
 * Must be called on any \c retro_vfs_dir_handle returned by \c retro_vfs_open_t.
 *
 * @param dirstream The directory to close.
 * When this function returns (even failure),
 * \c dirstream will no longer be valid and must not be used.
 * @return 0 on success, -1 on failure.
 * @see retro_closedir
 * @since VFS API v3
 */
typedef int (RETRO_CALLCONV *retro_vfs_closedir_t)(struct retro_vfs_dir_handle *dirstream);

/**
 * File system interface exposed by the frontend.
 *
 * @see dirent_vfs_init
 * @see filestream_vfs_init
 * @see path_vfs_init
 * @see RETRO_ENVIRONMENT_GET_VFS_INTERFACE
 */
struct retro_vfs_interface
{
   /* VFS API v1 */
   /** @copydoc retro_vfs_get_path_t */
	retro_vfs_get_path_t get_path;

   /** @copydoc retro_vfs_open_t */
	retro_vfs_open_t open;

   /** @copydoc retro_vfs_close_t */
	retro_vfs_close_t close;

   /** @copydoc retro_vfs_size_t */
	retro_vfs_size_t size;

   /** @copydoc retro_vfs_tell_t */
	retro_vfs_tell_t tell;

   /** @copydoc retro_vfs_seek_t */
	retro_vfs_seek_t seek;

   /** @copydoc retro_vfs_read_t */
	retro_vfs_read_t read;

   /** @copydoc retro_vfs_write_t */
	retro_vfs_write_t write;

   /** @copydoc retro_vfs_flush_t */
	retro_vfs_flush_t flush;

   /** @copydoc retro_vfs_remove_t */
	retro_vfs_remove_t remove;

   /** @copydoc retro_vfs_rename_t */
	retro_vfs_rename_t rename;
   /* VFS API v2 */

   /** @copydoc retro_vfs_truncate_t */
   retro_vfs_truncate_t truncate;
   /* VFS API v3 */

   /** @copydoc retro_vfs_stat_t */
   retro_vfs_stat_t stat;

   /** @copydoc retro_vfs_mkdir_t */
   retro_vfs_mkdir_t mkdir;

   /** @copydoc retro_vfs_opendir_t */
   retro_vfs_opendir_t opendir;

   /** @copydoc retro_vfs_readdir_t */
   retro_vfs_readdir_t readdir;

   /** @copydoc retro_vfs_dirent_get_name_t */
   retro_vfs_dirent_get_name_t dirent_get_name;

   /** @copydoc retro_vfs_dirent_is_dir_t */
   retro_vfs_dirent_is_dir_t dirent_is_dir;

   /** @copydoc retro_vfs_closedir_t */
   retro_vfs_closedir_t closedir;

   /* VFS API v4 */
   /** @copydoc retro_vfs_stat_64_t */
   retro_vfs_stat_64_t stat_64;
};

/**
 * Represents a request by the core for the frontend's file system interface,
 * as well as the interface itself returned by the frontend.
 *
 * You do not need to use these functions directly;
 * you may pass this struct to \c dirent_vfs_init,
 * \c filestream_vfs_init, or \c path_vfs_init
 * so that you can use the wrappers provided by these modules.
 *
 * @see dirent_vfs_init
 * @see filestream_vfs_init
 * @see path_vfs_init
 * @see RETRO_ENVIRONMENT_GET_VFS_INTERFACE
 */
struct retro_vfs_interface_info
{
   /**
    * The minimum version of the VFS API that the core requires.
    * libretro-common's wrapper API initializers will check this value as well.
    *
    * Set to the core's desired VFS version when requesting an interface,
    * and set by the frontend to indicate its actual API version.
    *
    * If the core asks for a newer VFS API version than the frontend supports,
    * the frontend must return \c false within the \c RETRO_ENVIRONMENT_GET_VFS_INTERFACE call.
    * @since VFS API v1
    */
   uint32_t required_interface_version;

   /**
    * Set by the frontend.
    * The frontend will set this to the VFS interface it provides.
    *
    * The interface is owned by the frontend
    * and must not be modified or freed by the core.
    * @since VFS API v1 */
   struct retro_vfs_interface *iface;
};

/**
 * Represents a single frontend-authorized filesystem location.
 *
 * The \c path field must be directly usable through the frontend VFS
 * interface, for example saf://... on Android.
 *
 * The frontend owns all returned pointers. Cores must copy strings if they
 * need to retain them after the environment callback returns.
 */
struct retro_vfs_authorized_location
{
   const char *path;
   const char *label;
   unsigned flags;
};

/**
 * Represents the list of frontend-authorized filesystem locations.
 *
 * This is returned by RETRO_ENVIRONMENT_GET_VFS_AUTHORIZED_LOCATIONS.
 */
struct retro_vfs_authorized_locations
{
   const struct retro_vfs_authorized_location *locations;
   size_t count;
};

/** @} */

/** @defgroup GET_HW_RENDER_INTERFACE Hardware Rendering Interface
 * @{
 */

/**
 * Describes the hardware rendering API supported by
 * a particular subtype of \c retro_hw_render_interface.
 *
 * Not every rendering API supported by libretro has its own interface,
 * or even needs one.
 *
 * @see RETRO_ENVIRONMENT_SET_HW_RENDER
 * @see RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE
 */
enum retro_hw_render_interface_type
{
   /**
    * Indicates a \c retro_hw_render_interface for Vulkan.
    * @see retro_hw_render_interface_vulkan
    */
   RETRO_HW_RENDER_INTERFACE_VULKAN     = 0,

   /** Indicates a \c retro_hw_render_interface for Direct3D 9. */
   RETRO_HW_RENDER_INTERFACE_D3D9       = 1,

   /** Indicates a \c retro_hw_render_interface for Direct3D 10. */
   RETRO_HW_RENDER_INTERFACE_D3D10      = 2,

   /**
    * Indicates a \c retro_hw_render_interface for Direct3D 11.
    * @see retro_hw_render_interface_d3d11
    */
   RETRO_HW_RENDER_INTERFACE_D3D11      = 3,

   /**
    * Indicates a \c retro_hw_render_interface for Direct3D 12.
    * @see retro_hw_render_interface_d3d12
    */
   RETRO_HW_RENDER_INTERFACE_D3D12      = 4,

   /**
    * Indicates a \c retro_hw_render_interface for
    * the PlayStation's 2 PSKit API.
    * @see retro_hw_render_interface_gskit_ps2
    */
   RETRO_HW_RENDER_INTERFACE_GSKIT_PS2  = 5,

   /** @private Defined to ensure <tt>sizeof(retro_hw_render_interface_type) == sizeof(int)</tt>.
    * Do not use. */
   RETRO_HW_RENDER_INTERFACE_DUMMY      = INT_MAX
};

/**
 * Base render interface type.
 * All \c retro_hw_render_interface implementations
 * will start with these two fields set to particular values.
 *
 * @see retro_hw_render_interface_type
 * @see RETRO_ENVIRONMENT_SET_HW_RENDER
 * @see RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE
 */
struct retro_hw_render_interface
{
   /**
    * Denotes the particular rendering API that this interface is for.
    * Each interface requires this field to be set to a particular value.
    * Use it to cast this interface to the appropriate pointer.
    */
   enum retro_hw_render_interface_type interface_type;

   /**
    * The version of this rendering interface.
    * @note This is not related to the version of the API itself.
    */
   unsigned interface_version;
};

/** @} */

/**
 * @defgroup GET_LED_INTERFACE LED Interface
 * @{
 */

/** @copydoc retro_led_interface::set_led_state */
typedef void (RETRO_CALLCONV *retro_set_led_state_t)(int led, int state);

/**
 * Interface that the core can use to set the state of available LEDs.
 * @see RETRO_ENVIRONMENT_GET_LED_INTERFACE
 */
struct retro_led_interface
{
   /**
    * Sets the state of an LED.
    *
    * @param led The LED to set the state of.
    * @param state The state to set the LED to.
    * \c true to enable, \c false to disable.
    */
   retro_set_led_state_t set_led_state;
};

/** @} */

/** @defgroup GET_AUDIO_VIDEO_ENABLE Skipped A/V Steps
 * @{
 */

/**
 * Flags that define A/V steps that the core may skip for this frame.
 *
 * @see RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE
 */
enum retro_av_enable_flags
{
   /**
    * If set, the core should render video output with \c retro_video_refresh_t as normal.
    *
    * Otherwise, the frontend will discard any video data received this frame,
    * including frames presented via hardware acceleration.
    * \c retro_video_refresh_t will do nothing.
    *
    * @note After running the frame, the video output of the next frame
    * should be no different than if video was enabled,
    * and saving and loading state should have no issues.
    * This implies that the emulated console's graphics pipeline state
    * should not be affected by this flag.
    *
    * @note If emulating a platform that supports display capture
    * (i.e. reading its own VRAM),
    * the core may not be able to completely skip rendering,
    * as the VRAM is part of the graphics pipeline's state.
    */
   RETRO_AV_ENABLE_VIDEO = (1 << 0),

   /**
    * If set, the core should render audio output
    * with \c retro_audio_sample_t or \c retro_audio_sample_batch_t as normal.
    *
    * Otherwise, the frontend will discard any audio data received this frame.
    * The core should skip audio rendering if possible.
    *
    * @note After running the frame, the audio output of the next frame
    * should be no different than if audio was enabled,
    * and saving and loading state should have no issues.
    * This implies that the emulated console's audio pipeline state
    * should not be affected by this flag.
    */
   RETRO_AV_ENABLE_AUDIO = (1 << 1),

   /**
    * If set, indicates that any savestates taken this frame
    * are guaranteed to be created by the same binary that will load them,
    * and will not be written to or read from the disk.
    *
    * The core may use these guarantees to:
    *
    * @li Assume that loading state will succeed.
    * @li Update its memory buffers in-place if possible.
    * @li Skip clearing memory.
    * @li Skip resetting the system.
    * @li Skip validation steps.
    *
    * @deprecated Use \c RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT instead,
    * except for compatibility purposes.
    */
   RETRO_AV_ENABLE_FAST_SAVESTATES = (1 << 2),

   /**
    * If set, indicates that the frontend will never need audio from the core.
    * Used by a frontend for implementing runahead via a secondary core instance.
    *
    * The core may stop synthesizing audio if it can do so
    * without compromising emulation accuracy.
    *
    * Audio output for the next frame does not matter,
    * and the frontend will never need an accurate audio state in the future.
    *
    * State will never be saved while this flag is set.
    */
   RETRO_AV_ENABLE_HARD_DISABLE_AUDIO = (1 << 3),

   /**
    * @private Defined to ensure <tt>sizeof(retro_av_enable_flags) == sizeof(int)</tt>.
    * Do not use.
    */
   RETRO_AV_ENABLE_DUMMY = INT_MAX
};

/** @} */

/**
 * @defgroup GET_MIDI_INTERFACE MIDI Interface
 * @{
 */

/** @copydoc retro_midi_interface::input_enabled */
typedef bool (RETRO_CALLCONV *retro_midi_input_enabled_t)(void);

/** @copydoc retro_midi_interface::output_enabled */
typedef bool (RETRO_CALLCONV *retro_midi_output_enabled_t)(void);

/** @copydoc retro_midi_interface::read */
typedef bool (RETRO_CALLCONV *retro_midi_read_t)(uint8_t *byte);

/** @copydoc retro_midi_interface::write */
typedef bool (RETRO_CALLCONV *retro_midi_write_t)(uint8_t byte, uint32_t delta_time);

/** @copydoc retro_midi_interface::flush */
typedef bool (RETRO_CALLCONV *retro_midi_flush_t)(void);

/**
 * Interface that the core can use for raw MIDI I/O.
 */
struct retro_midi_interface
{
   /**
    * Retrieves the current state of MIDI input.
    *
    * @return \c true if MIDI input is enabled.
    */
   retro_midi_input_enabled_t input_enabled;

   /**
    * Retrieves the current state of MIDI output.
    * @return \c true if MIDI output is enabled.
    */
   retro_midi_output_enabled_t output_enabled;

   /**
    * Reads a byte from the MIDI input stream.
    *
    * @param[out] byte The byte received from the input stream.
    * @return \c true if a byte was read,
    * \c false if MIDI input is disabled or \c byte is \c NULL.
    */
   retro_midi_read_t read;

   /**
    * Writes a byte to the output stream.
    *
    * @param byte The byte to write to the output stream.
    * @param delta_time Time since the previous write, in microseconds.
    * @return \c true if c\ byte was written, false otherwise.
    */
   retro_midi_write_t write;

   /**
    * Flushes previously-written data.
    *
    * @return \c true if successful.
    */
   retro_midi_flush_t flush;
};

/** @} */

/** @defgroup SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE Render Context Negotiation
 * @{
 */

/**
 * Describes the hardware rendering API used by
 * a particular subtype of \c retro_hw_render_context_negotiation_interface.
 *
 * Not every rendering API supported by libretro has a context negotiation interface,
 * or even needs one.
 *
 * @see RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE
 * @see RETRO_ENVIRONMENT_SET_HW_RENDER
 * @see RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE
 */
enum retro_hw_render_context_negotiation_interface_type
{
   /**
    * Denotes a context negotiation interface for Vulkan.
    * @see retro_hw_render_context_negotiation_interface_vulkan
    */
   RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_VULKAN = 0,

   /**
    * @private Defined to ensure <tt>sizeof(retro_hw_render_context_negotiation_interface_type) == sizeof(int)</tt>.
    * Do not use.
    */
   RETRO_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE_DUMMY = INT_MAX
};

/**
 * Base context negotiation interface type.
 * All \c retro_hw_render_context_negotiation_interface implementations
 * will start with these two fields set to particular values.
 *
 * @see retro_hw_render_interface_type
 * @see RETRO_ENVIRONMENT_SET_HW_RENDER
 * @see RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE
 * @see RETRO_ENVIRONMENT_SET_HW_RENDER_CONTEXT_NEGOTIATION_INTERFACE
 */
struct retro_hw_render_context_negotiation_interface
{
   /**
    * Denotes the particular rendering API that this interface is for.
    * Each interface requires this field to be set to a particular value.
    * Use it to cast this interface to the appropriate pointer.
    */
   enum retro_hw_render_context_negotiation_interface_type interface_type;

   /**
    * The version of this negotiation interface.
    * @note This is not related to the version of the API itself.
    */
   unsigned interface_version;
};

/** @} */

/** @defgroup RETRO_SERIALIZATION_QUIRK Serialization Quirks
 * @{
 */

/**
 * Indicates that serialized state is incomplete in some way.
 *
 * Set if serialization is usable for the common case of saving and loading game state,
 * but should not be relied upon for frame-sensitive frontend features
 * such as netplay or rerecording.
 */
#define RETRO_SERIALIZATION_QUIRK_INCOMPLETE (1 << 0)

/**
 * Indicates that core must spend some time initializing before serialization can be done.
 *
 * \c retro_serialize(), \c retro_unserialize(), and \c retro_serialize_size() will initially fail.
 */
#define RETRO_SERIALIZATION_QUIRK_MUST_INITIALIZE (1 << 1)

/** Set by the core to indicate that serialization size may change within a session. */
#define RETRO_SERIALIZATION_QUIRK_CORE_VARIABLE_SIZE (1 << 2)

/** Set by the frontend to acknowledge that it supports variable-sized states. */
#define RETRO_SERIALIZATION_QUIRK_FRONT_VARIABLE_SIZE (1 << 3)

/** Serialized state can only be loaded during the same session. */
#define RETRO_SERIALIZATION_QUIRK_SINGLE_SESSION (1 << 4)

/**
 * Serialized state cannot be loaded on an architecture
 * with a different endianness from the one it was saved on.
 */
#define RETRO_SERIALIZATION_QUIRK_ENDIAN_DEPENDENT (1 << 5)

/**
 * Serialized state cannot be loaded on a different platform
 * from the one it was saved on for reasons other than endianness,
 * such as word size dependence.
 */
#define RETRO_SERIALIZATION_QUIRK_PLATFORM_DEPENDENT (1 << 6)

/** @} */

/** @defgroup SET_MEMORY_MAPS Memory Descriptors
 * @{
 */

/** @defgroup RETRO_MEMDESC Memory Descriptor Flags
 * Information about how the emulated hardware uses this portion of its address space.
 * @{
 */

/**
 * Indicates that this memory area won't be modified
 * once \c retro_load_game has returned.
 */
#define RETRO_MEMDESC_CONST      (1 << 0)

/**
 * Indicates a memory area with big-endian byte ordering,
 * as opposed to the default of little-endian.
 */
#define RETRO_MEMDESC_BIGENDIAN  (1 << 1)

/**
 * Indicates a memory area that is used for the emulated system's main RAM.
 */
#define RETRO_MEMDESC_SYSTEM_RAM (1 << 2)

/**
 * Indicates a memory area that is used for the emulated system's save RAM,
 * usually found on a game cartridge as battery-backed RAM or flash memory.
 */
#define RETRO_MEMDESC_SAVE_RAM   (1 << 3)

/**
 * Indicates a memory area that is used for the emulated system's video RAM,
 * usually found on a console's GPU (or local equivalent).
 */
#define RETRO_MEMDESC_VIDEO_RAM  (1 << 4)

/**
 * Indicates a memory area that requires all accesses
 * to be aligned to 2 bytes or their own size
 * (whichever is smaller).
 */
#define RETRO_MEMDESC_ALIGN_2    (1 << 16)

/**
 * Indicates a memory area that requires all accesses
 * to be aligned to 4 bytes or their own size
 * (whichever is smaller).
 */
#define RETRO_MEMDESC_ALIGN_4    (2 << 16)

/**
 * Indicates a memory area that requires all accesses
 * to be aligned to 8 bytes or their own size
 * (whichever is smaller).
 */
#define RETRO_MEMDESC_ALIGN_8    (3 << 16)

/**
 * Indicates a memory area that requires all accesses
 * to be at least 2 bytes long.
 */
#define RETRO_MEMDESC_MINSIZE_2  (1 << 24)

/**
 * Indicates a memory area that requires all accesses
 * to be at least 4 bytes long.
 */
#define RETRO_MEMDESC_MINSIZE_4  (2 << 24)

/**
 * Indicates a memory area that requires all accesses
 * to be at least 8 bytes long.
 */
#define RETRO_MEMDESC_MINSIZE_8  (3 << 24)

/** @} */

/**
 * A mapping from a region of the emulated console's address space
 * to the host's address space.
 *
 * Can be used to map an address in the console's address space
 * to the host's address space, like so:
 *
 * @code
 * void* emu_to_host(void* addr, struct retro_memory_descriptor* descriptor)
 * {
 *     return descriptor->ptr + (addr & ~descriptor->disconnect) - descriptor->start;
 * }
 * @endcode
 *
 * @see RETRO_ENVIRONMENT_SET_MEMORY_MAPS
 */
struct retro_memory_descriptor
{
   /**
    * A bitwise \c OR of one or more \ref RETRO_MEMDESC "flags"
    * that describe how the emulated system uses this descriptor's address range.
    *
    * @note If \c ptr is \c NULL,
    * then no flags should be set.
    * @see RETRO_MEMDESC
    */
   uint64_t flags;

   /**
    * Pointer to the start of this memory region's buffer
    * within the \em host's address space.
    * The address listed here must be valid for the duration of the session;
    * it must not be freed or modified by the frontend
    * and it must not be moved by the core.
    *
    * May be \c NULL to indicate a lack of accessible memory
    * at the emulated address given in \c start.
    *
    * @note Overlapping descriptors that include the same byte
    * must have the same \c ptr value.
    */
   void *ptr;

   /**
    * The offset of this memory region,
    * relative to the address given by \c ptr.
    *
    * @note It is recommended to use this field for address calculations
    * instead of performing arithmetic on \c ptr.
    */
   size_t offset;

   /**
    * The starting address of this memory region
    * <em>within the emulated hardware's address space</em>.
    *
    * @note Not represented as a pointer
    * because it's unlikely to be valid on the host device.
    */
   size_t start;

   /**
    * A bitmask that specifies which bits of an address must match
    * the bits of the \ref start address.
    *
    * Combines with \c disconnect to map an address to a memory block.
    *
    * If multiple memory descriptors can claim a particular byte,
    * the first one defined in the \ref retro_memory_descriptor array applies.
    * A bit which is set in \c start must also be set in this.
    *
    * Can be zero, in which case \c start and \c len represent
    * the complete mapping for this region of memory
    * (i.e. each byte is mapped exactly once).
    * In this case, \c len must be a power of two.
    */
   size_t select;

   /**
    * A bitmask of bits that are \em not used for addressing.
    *
    * Any set bits are assumed to be disconnected from
    * the emulated memory chip's address pins,
    * and are therefore ignored when memory-mapping.
    */
   size_t disconnect;

   /**
    * The length of this memory region, in bytes.
    *
    * If applying \ref start and \ref disconnect to an address
    * results in a value higher than this,
    * the highest bit of the address is cleared.
    *
    * If the address is still too high, the next highest bit is cleared.
    * Can be zero, in which case it's assumed to be
    * bounded only by \ref select and \ref disconnect.
    */
   size_t len;

   /**
    * A short name for this address space.
    *
    * Names must meet the following requirements:
    *
    * \li Characters must be in the set <tt>[a-zA-Z0-9_-]</tt>.
    * \li No more than 8 characters, plus a \c NULL terminator.
    * \li Names are case-sensitive, but lowercase characters are discouraged.
    * \li A name must not be the same as another name plus a character in the set \c [A-F0-9]
    *     (i.e. if an address space named "RAM" exists,
    *     then the names "RAM0", "RAM1", ..., "RAMF" are forbidden).
    *     This is to allow addresses to be named by each descriptor unambiguously,
    *     even if the areas overlap.
    * \li May be \c NULL or empty (both are considered equivalent).
    *
    * Here are some examples of pairs of address space names:
    *
    * \li \em blank + \em blank: valid (multiple things may be mapped in the same namespace)
    * \li \c Sp + \c Sp: valid (multiple things may be mapped in the same namespace)
    * \li \c SRAM + \c VRAM: valid (neither is a prefix of the other)
    * \li \c V + \em blank: valid (\c V is not in \c [A-F0-9])
    * \li \c a + \em blank: valid but discouraged (\c a is not in \c [A-F0-9])
    * \li \c a + \c A: valid but discouraged (neither is a prefix of the other)
    * \li \c AR + \em blank: valid (\c R is not in \c [A-F0-9])
    * \li \c ARB + \em blank: valid (there's no \c AR namespace,
    *     so the \c B doesn't cause ambiguity).
    * \li \em blank + \c B: invalid, because it's ambiguous which address space \c B1234 would refer to.
    *
    * The length of the address space's name can't be used to disambugiate,
    * as extra information may be appended to it without a separator.
    */
   const char *addrspace;

   /* TODO: When finalizing this one, add a description field, which should be
    * "WRAM" or something roughly equally long. */

   /* TODO: When finalizing this one, replace 'select' with 'limit', which tells
    * which bits can vary and still refer to the same address (limit = ~select).
    * TODO: limit? range? vary? something else? */

   /* TODO: When finalizing this one, if 'len' is above what 'select' (or
    * 'limit') allows, it's bankswitched. Bankswitched data must have both 'len'
    * and 'select' != 0, and the mappings don't tell how the system switches the
    * banks. */

   /* TODO: When finalizing this one, fix the 'len' bit removal order.
    * For len=0x1800, pointer 0x1C00 should go to 0x1400, not 0x0C00.
    * Algorithm: Take bits highest to lowest, but if it goes above len, clear
    * the most recent addition and continue on the next bit.
    * TODO: Can the above be optimized? Is "remove the lowest bit set in both
    * pointer and 'len'" equivalent? */

   /* TODO: Some emulators (MAME?) emulate big endian systems by only accessing
    * the emulated memory in 32-bit chunks, native endian. But that's nothing
    * compared to Darek Mihocka <http://www.emulators.com/docs/nx07_vm101.htm>
    * (section Emulation 103 - Nearly Free Byte Reversal) - he flips the ENTIRE
    * RAM backwards! I'll want to represent both of those, via some flags.
    *
    * I suspect MAME either didn't think of that idea, or don't want the #ifdef.
    * Not sure which, nor do I really care. */

   /* TODO: Some of those flags are unused and/or don't really make sense. Clean
    * them up. */
};

/**
 * A list of regions within the emulated console's address space.
 *
 * The frontend may use the largest value of
 * \ref retro_memory_descriptor::start + \ref retro_memory_descriptor::select
 * in a certain namespace to infer the overall size of the address space.
 * If the address space is larger than that,
 * the last mapping in \ref descriptors should have \ref retro_memory_descriptor::ptr set to \c NULL
 * and \ref retro_memory_descriptor::select should have all bits used in the address space set to 1.
 *
 * Here's an example set of descriptors for the SNES.
 *
 * @code{.c}
 * struct retro_memory_map snes_descriptors = retro_memory_map
 * {
 *    .descriptors = (struct retro_memory_descriptor[])
 *    {
 *       // WRAM; must usually be mapped before the ROM,
 *       // as some SNES ROM mappers try to claim 0x7E0000
 *       { .addrspace="WRAM", .start=0x7E0000, .len=0x20000 },
 *
 *       // SPC700 RAM
 *       { .addrspace="SPC700", .len=0x10000 },
 *
 *       // WRAM mirrors
 *       { .addrspace="WRAM", .start=0x000000, .select=0xC0E000, .len=0x2000 },
 *       { .addrspace="WRAM", .start=0x800000, .select=0xC0E000, .len=0x2000 },
 *
 *       // WRAM mirror, alternate equivalent descriptor
 *       // (Various similar constructions can be created by combining parts of the above two.)
 *       { .addrspace="WRAM", .select=0x40E000, .disconnect=~0x1FFF },
 *
 *       // LoROM (512KB, mirrored a couple of times)
 *       { .addrspace="LoROM", .start=0x008000, .select=0x408000, .disconnect=0x8000, .len=512*1024, .flags=RETRO_MEMDESC_CONST },
 *       { .addrspace="LoROM", .start=0x400000, .select=0x400000, .disconnect=0x8000, .len=512*1024, .flags=RETRO_MEMDESC_CONST },
 *
 *       // HiROM (4MB)
 *       { .addrspace="HiROM", .start=0x400000, .select=0x400000, .len=4*1024*1024, .flags=RETRO_MEMDESC_CONST },
 *       { .addrspace="HiROM", .start=0x008000, .select=0x408000, .len=4*1024*1024, .offset=0x8000, .flags=RETRO_MEMDESC_CONST },
 *
 *       // ExHiROM (8MB)
 *       { .addrspace="ExHiROM", .start=0xC00000, .select=0xC00000, .len=4*1024*1024, .offset=0, .flags=RETRO_MEMDESC_CONST },
 *       { .addrspace="ExHiROM", .start=0x400000, .select=0xC00000, .len=4*1024*1024, .offset=4*1024*1024, .flags=RETRO_MEMDESC_CONST },
 *       { .addrspace="ExHiROM", .start=0x808000, .select=0xC08000, .len=4*1024*1024, .offset=0x8000, .flags=RETRO_MEMDESC_CONST },
 *       { .addrspace="ExHiROM", .start=0x008000, .select=0xC08000, .len=4*1024*1024, .offset=4*1024*1024+0x8000, .flags=RETRO_MEMDESC_CONST },
 *
 *       // Clarifying the full size of the address space
 *       { .select=0xFFFFFF, .ptr=NULL },
 *    },
 *    .num_descriptors = 14,
 * };
 * @endcode
 *
 * @see RETRO_ENVIRONMENT_SET_MEMORY_MAPS
 */
struct retro_memory_map
{
   /**
    * Pointer to an array of memory descriptors,
    * each of which describes part of the emulated console's address space.
    */
   const struct retro_memory_descriptor *descriptors;

   /** The number of descriptors in \c descriptors. */
   unsigned num_descriptors;
};

/** @} */

/** @defgroup SET_CONTROLLER_INFO Controller Info
 * @{
 */

/**
 * Details about a controller (or controller configuration)
 * supported by one of a core's emulated input ports.
 *
 * @see RETRO_ENVIRONMENT_SET_CONTROLLER_INFO
 */
struct retro_controller_description
{
   /**
    * A human-readable label for the controller or configuration
    * represented by this device type.
    * Most likely the device's original brand name.
    */
   const char *desc;

   /**
    * A unique identifier that will be passed to \c retro_set_controller_port_device()'s \c device parameter.
    * May be the ID of one of \ref RETRO_DEVICE "the generic controller types",
    * or a subclass ID defined with \c RETRO_DEVICE_SUBCLASS.
    *
    * @see RETRO_DEVICE_SUBCLASS
    */
   unsigned id;
};

/**
 * Lists the types of controllers supported by
 * one of core's emulated input ports.
 *
 * @see RETRO_ENVIRONMENT_SET_CONTROLLER_INFO
 */
struct retro_controller_info
{

   /**
    * A pointer to an array of device types supported by this controller port.
    *
    * @note Ports that support the same devices
    * may share the same underlying array.
    */
   const struct retro_controller_description *types;

   /** The number of elements in \c types. */
   unsigned num_types;
};

/** @} */

/** @defgroup SET_SUBSYSTEM_INFO Subsystems
 * @{
 */

/**
 * Information about a type of memory associated with a subsystem.
 * Usually used for SRAM (save RAM).
 *
 * @see RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO
 * @see retro_get_memory_data
 * @see retro_get_memory_size
 */
struct retro_subsystem_memory_info
{
   /**
    * The file extension the frontend should use
    * to save this memory region to disk, e.g. "srm" or "sav".
    */
   const char *extension;

   /**
    * A constant that identifies this type of memory.
    * Should be at least 0x100 (256) to avoid conflict
    * with the standard libretro memory types,
    * unless a subsystem uses the main platform's memory region.
    * @see RETRO_MEMORY
    */
   unsigned type;
};

/**
 * Information about a type of ROM that a subsystem may use.
 * Subsystems may use one or more ROMs at once,
 * possibly of different types.
 *
 * @see RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO
 * @see retro_subsystem_info
 */
struct retro_subsystem_rom_info
{
   /**
    * Human-readable description of what the content represents,
    * e.g. "Game Boy ROM".
    */
   const char *desc;

   /** @copydoc retro_system_info::valid_extensions */
   const char *valid_extensions;

   /** @copydoc retro_system_info::need_fullpath */
   bool need_fullpath;

   /** @copydoc retro_system_info::block_extract */
   bool block_extract;

   /**
    * Indicates whether this particular subsystem ROM is required.
    * If \c true and the user doesn't provide a ROM,
    * the frontend should not load the core.
    * If \c false and the user doesn't provide a ROM,
    * the frontend should pass a zeroed-out \c retro_game_info
    * to the corresponding entry in \c retro_load_game_special().
    */
   bool required;

   /**
    * Pointer to an array of memory descriptors that this subsystem ROM type uses.
    * Useful for secondary cartridges that have their own save data.
    * May be \c NULL, in which case this subsystem ROM's memory is not persisted by the frontend
    * and \c num_memory should be zero.
    */
   const struct retro_subsystem_memory_info *memory;

   /** The number of elements in the array pointed to by \c memory. */
   unsigned num_memory;
};

/**
 * Information about a secondary platform that a core supports.
 * @see RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO
 */
struct retro_subsystem_info
{
   /**
    * A human-readable description of the subsystem type,
    * usually the brand name of the original platform
    * (e.g. "Super Game Boy").
    */
   const char *desc;

   /**
    * A short machine-friendly identifier for the subsystem,
    * usually an abbreviation of the platform name.
    * For example, a Super Game Boy subsystem for a SNES core
    * might use an identifier of "sgb".
    * This identifier can be used for command-line interfaces,
    * configuration, or other purposes.
    * Must use lower-case alphabetical characters only (i.e. from a-z).
    */
   const char *ident;

   /**
    * The list of ROM types that this subsystem may use.
    *
    * The first entry is considered to be the "most significant" content,
    * for the purposes of the frontend's categorization.
    * E.g. with Super GameBoy, the first content should be the GameBoy ROM,
    * as it is the most "significant" content to a user.
    *
    * If a frontend creates new files based on the content used (e.g. for savestates),
    * it should derive the filenames from the name of the first ROM in this list.
    *
    * @note \c roms can have a single element,
    * but this is usually a sign that the core should broaden its
    * primary system info instead.
    *
    * @see \c retro_system_info
    */
   const struct retro_subsystem_rom_info *roms;

   /** The length of the array given in \c roms. */
   unsigned num_roms;

   /** A unique identifier passed to retro_load_game_special(). */
   unsigned id;
};

/** @} */

/** @defgroup SET_PROC_ADDRESS_CALLBACK Core Function Pointers
 * @{ */

/**
 * The function pointer type that \c retro_get_proc_address_t returns.
 *
 * Despite the signature shown here, the original function may include any parameters and return type
 * that respects the calling convention and C ABI.
 *
 * The frontend is expected to cast the function pointer to the correct type.
 */
typedef void (RETRO_CALLCONV *retro_proc_address_t)(void);

/**
 * Get a symbol from a libretro core.
 *
 * Cores should only return symbols that serve as libretro extensions.
 * Frontends should not use this to obtain symbols to standard libretro entry points;
 * instead, they should link to the core statically or use \c dlsym (or local equivalent).
 *
 * The symbol name must be equal to the function name.
 * e.g. if <tt>void retro_foo(void);</tt> exists, the symbol in the compiled library must be called \c retro_foo.
 * The returned function pointer must be cast to the corresponding type.
 *
 * @param \c sym The name of the symbol to look up.
 * @return Pointer to the exposed function with the name given in \c sym,
 * or \c NULL if one couldn't be found.
 * @note The frontend is expected to know the returned pointer's type in advance
 * so that it can be cast correctly.
 * @note The core doesn't need to expose every possible function through this interface.
 * It's enough to only expose the ones that it expects the frontend to use.
 * @note The functions exposed through this interface
 * don't need to be publicly exposed in the compiled library
 * (e.g. via \c __declspec(dllexport)).
 * @see RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK
 */
typedef retro_proc_address_t (RETRO_CALLCONV *retro_get_proc_address_t)(const char *sym);

/**
 * An interface that the frontend can use to get function pointers from the core.
 *
 * @note The returned function pointer will be invalidated once the core is unloaded.
 * How and when that happens is up to the frontend.
 *
 * @see retro_get_proc_address_t
 * @see RETRO_ENVIRONMENT_SET_PROC_ADDRESS_CALLBACK
 */
struct retro_get_proc_address_interface
{
   /** Set by the core. */
   retro_get_proc_address_t get_proc_address;
};

/** @} */

/** @defgroup GET_LOG_INTERFACE Logging
 * @{
 */

/**
 * The severity of a given message.
 * The frontend may log messages differently depending on the level.
 * It may also ignore log messages of a certain level.
 * @see retro_log_callback
 */
enum retro_log_level
{
   /** The logged message is most likely not interesting to the user. */
   RETRO_LOG_DEBUG = 0,

   /** Information about the core operating normally. */
   RETRO_LOG_INFO,

   /** Indicates a potential problem, possibly one that the core can recover from. */
   RETRO_LOG_WARN,

   /** Indicates a degraded experience, if not failure. */
   RETRO_LOG_ERROR,

   /** Defined to ensure that sizeof(enum retro_log_level) == sizeof(int). Do not use. */
   RETRO_LOG_DUMMY = INT_MAX
};

/**
 * Logs a message to the frontend.
 *
 * @param level The log level of the message.
 * @param fmt The format string to log.
 * Same format as \c printf.
 * Behavior is undefined if this is \c NULL.
 * @param ... Zero or more arguments used by the format string.
 * Behavior is undefined if these don't match the ones expected by \c fmt.
 * @see retro_log_level
 * @see retro_log_callback
 * @see RETRO_ENVIRONMENT_GET_LOG_INTERFACE
 * @see printf
 */
typedef void (RETRO_CALLCONV *retro_log_printf_t)(enum retro_log_level level,
      const char *fmt, ...);

/**
 * Details about how to make log messages.
 *
 * @see retro_log_printf_t
 * @see RETRO_ENVIRONMENT_GET_LOG_INTERFACE
 */
struct retro_log_callback
{
   /**
    * Called when logging a message.
    *
    * @note Set by the frontend.
    */
   retro_log_printf_t log;
};

/** @} */

/** @defgroup GET_PERF_INTERFACE Performance Interface
 * @{
 */

/** @defgroup RETRO_SIMD CPU Features
 * @{
 */

/**
 * Indicates CPU support for the SSE instruction set.
 *
 * @see https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ssetechs=SSE
 */
#define RETRO_SIMD_SSE      (1 << 0)

/**
 * Indicates CPU support for the SSE2 instruction set.
 *
 * @see https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ssetechs=SSE2
 */
#define RETRO_SIMD_SSE2     (1 << 1)

/** Indicates CPU support for the AltiVec (aka VMX or Velocity Engine) instruction set. */
#define RETRO_SIMD_VMX      (1 << 2)

/** Indicates CPU support for the VMX128 instruction set. Xbox 360 only. */
#define RETRO_SIMD_VMX128   (1 << 3)

/**
 * Indicates CPU support for the AVX instruction set.
 *
 * @see https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avxnewtechs=AVX
 */
#define RETRO_SIMD_AVX      (1 << 4)

/**
 * Indicates CPU support for the NEON instruction set.
 * @see https://developer.arm.com/architectures/instruction-sets/intrinsics/#f:@navigationhierarchiessimdisa=[Neon]
 */
#define RETRO_SIMD_NEON     (1 << 5)

/**
 * Indicates CPU support for the SSE3 instruction set.
 *
 * @see https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ssetechs=SSE3
 */
#define RETRO_SIMD_SSE3     (1 << 6)

/**
 * Indicates CPU support for the SSSE3 instruction set.
 *
 * @see https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ssetechs=SSSE3
 */
#define RETRO_SIMD_SSSE3    (1 << 7)

/**
 * Indicates CPU support for the MMX instruction set.
 * @see https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#techs=MMX
 */
#define RETRO_SIMD_MMX      (1 << 8)

/** Indicates CPU support for the MMXEXT instruction set. */
#define RETRO_SIMD_MMXEXT   (1 << 9)

/**
 * Indicates CPU support for the SSE4 instruction set.
 *
 * @see https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ssetechs=SSE4_1
 */
#define RETRO_SIMD_SSE4     (1 << 10)

/**
 * Indicates CPU support for the SSE4.2 instruction set.
 *
 * @see https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#ssetechs=SSE4_2
 */
#define RETRO_SIMD_SSE42    (1 << 11)

/**
 * Indicates CPU support for the AVX2 instruction set.
 *
 * @see https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avxnewtechs=AVX2
 */
#define RETRO_SIMD_AVX2     (1 << 12)

/** Indicates CPU support for the VFPU instruction set. PS2 and PSP only.
 *
 * @see https://pspdev.github.io/vfpu-docs
 */
#define RETRO_SIMD_VFPU     (1 << 13)

/**
 * Indicates CPU support for Gekko SIMD extensions. GameCube only.
 */
#define RETRO_SIMD_PS       (1 << 14)

/**
 * Indicates CPU support for AES instructions.
 *
 * @see https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#aestechs=AES&othertechs=AES
 */
#define RETRO_SIMD_AES      (1 << 15)

/**
 * Indicates CPU support for the VFPv3 instruction set.
 */
#define RETRO_SIMD_VFPV3    (1 << 16)

/**
 * Indicates CPU support for the VFPv4 instruction set.
 */
#define RETRO_SIMD_VFPV4    (1 << 17)

/** Indicates CPU support for the POPCNT instruction. */
#define RETRO_SIMD_POPCNT   (1 << 18)

/** Indicates CPU support for the MOVBE instruction. */
#define RETRO_SIMD_MOVBE    (1 << 19)

/** Indicates CPU support for the CMOV instruction. */
#define RETRO_SIMD_CMOV     (1 << 20)

/** Indicates CPU support for the ASIMD instruction set. */
#define RETRO_SIMD_ASIMD    (1 << 21)

/** Indicates CPU support for the AVX512 instruction set. */
#define RETRO_SIMD_AVX512   (1 << 22)

/** Indicates CPU support for the LZCNT instruction (x86 ABM / ARM CLZ). */
#define RETRO_SIMD_LZCNT    (1 << 23)

/**
 * Indicates CPU support for the PCLMULQDQ carry-less multiply instruction.
 *
 * Distinct from \c RETRO_SIMD_AES: AES-NI is CPUID.1:ECX[25] and
 * PCLMULQDQ is CPUID.1:ECX[1]. They shipped together on most parts but
 * hypervisors mask them independently and some early Westmere SKUs had
 * AES fused off, so one must not be used as a proxy for the other.
 */
#define RETRO_SIMD_PCLMUL   (1 << 24)

/**
 * Indicates CPU support for the ARMv8 CRC32 instructions
 * (\c crc32b / \c crc32h / \c crc32w / \c crc32x).
 *
 * These compute CRC-32/ISO-HDLC, the gzip and PNG polynomial, not
 * CRC-32C. Optional in ARMv8.0 and mandatory from ARMv8.1, so a
 * 64-bit ARM CPU does not imply their presence: Apple's A7 through
 * A10 lack them, for instance.
 */
#define RETRO_SIMD_CRC32    (1 << 25)

/**
 * Indicates CPU support for hardware SHA-512 acceleration.
 *
 * On AArch64 this is FEAT_SHA512, optional from Armv8.1 and A64-only.
 * On x86 it is the SHA512 instruction group enumerated by
 * CPUID.(EAX=07H,ECX=1):EAX[0], which is separate from the SHA-NI
 * instructions covering SHA-1 and SHA-256.
 */
#define RETRO_SIMD_SHA512   (1 << 26)

/**
 * Indicates CPU support for hardware SHA-1 acceleration.
 *
 * On AArch64 this is FEAT_SHA1; on x86 it is part of SHA-NI, which
 * covers SHA-1 and SHA-256 in one CPUID bit and therefore always
 * reports alongside \c RETRO_SIMD_SHA256 there.
 */
#define RETRO_SIMD_SHA1     (1 << 27)

/**
 * Indicates CPU support for hardware SHA-256 acceleration.
 *
 * On AArch64 this is FEAT_SHA256; on x86 it is the other half of
 * SHA-NI. Separate from \c RETRO_SIMD_SHA1 because AArch64 enumerates
 * the two independently.
 */
#define RETRO_SIMD_SHA256   (1 << 28)

/**
 * Indicates CPU support for the FMA3 fused multiply-add instructions.
 *
 * CPUID.(EAX=01H):ECX[12]. They operate on YMM state, so this reports
 * only where the operating system preserves it, as \c RETRO_SIMD_AVX
 * does.
 */
#define RETRO_SIMD_FMA3     (1 << 29)

/**
 * Indicates CPU support for the FMA4 fused multiply-add instructions.
 *
 * CPUID.(EAX=80000001H):ECX[16], an AMD extension dropped from Zen, and
 * a different encoding from \c RETRO_SIMD_FMA3 rather than a superset
 * of it. Gated on the same operating system state.
 */
#define RETRO_SIMD_FMA4     (1 << 30)

/** @} */

/**
 * An abstract unit of ticks.
 *
 * Usually nanoseconds or CPU cycles,
 * but it depends on the platform and the frontend.
 */
typedef uint64_t retro_perf_tick_t;

/** Time in microseconds. */
typedef int64_t retro_time_t;

/**
 * A performance counter.
 *
 * Use this to measure the execution time of a region of code.
 * @see retro_perf_callback
 */
struct retro_perf_counter
{
   /**
    * A human-readable identifier for the counter.
    *
    * May be displayed by the frontend.
    * Behavior is undefined if this is \c NULL.
    */
   const char *ident;

   /**
    * The time of the most recent call to \c retro_perf_callback::perf_start
    * on this performance counter.
    *
    * @see retro_perf_start_t
    */
   retro_perf_tick_t start;

   /**
    * The total time spent within this performance counter's measured code,
    * i.e. between calls to \c retro_perf_callback::perf_start and \c retro_perf_callback::perf_stop.
    *
    * Updated after each call to \c retro_perf_callback::perf_stop.
    * @see retro_perf_stop_t
    */
   retro_perf_tick_t total;

   /**
    * The number of times this performance counter has been started.
    *
    * Updated after each call to \c retro_perf_callback::perf_start.
    * @see retro_perf_start_t
    */
   retro_perf_tick_t call_cnt;

   /**
    * \c true if this performance counter has been registered by the frontend.
    * Must be initialized to \c false by the core before registering it.
    * @see retro_perf_register_t
    */
   bool registered;
};

/**
 * @returns The current system time in microseconds.
 * @note Accuracy may vary by platform.
 * The frontend should use the most accurate timer possible.
 * @see RETRO_ENVIRONMENT_GET_PERF_INTERFACE
 */
typedef retro_time_t (RETRO_CALLCONV *retro_perf_get_time_usec_t)(void);

/**
 * @returns The number of ticks since some unspecified epoch.
 * The exact meaning of a "tick" depends on the platform,
 * but it usually refers to nanoseconds or CPU cycles.
 * @see RETRO_ENVIRONMENT_GET_PERF_INTERFACE
 */
typedef retro_perf_tick_t (RETRO_CALLCONV *retro_perf_get_counter_t)(void);

/**
 * Returns a bitmask of detected CPU features.
 *
 * Use this for runtime dispatching of CPU-specific code.
 *
 * @returns A bitmask of detected CPU features.
 * @see RETRO_ENVIRONMENT_GET_PERF_INTERFACE
 * @see RETRO_SIMD
 */
typedef uint64_t (RETRO_CALLCONV *retro_get_cpu_features_t)(void);

/**
 * Asks the frontend to log or display the state of performance counters.
 * How this is done depends on the frontend.
 * Performance counters can be reviewed manually as well.
 *
 * @see RETRO_ENVIRONMENT_GET_PERF_INTERFACE
 * @see retro_perf_counter
 */
typedef void (RETRO_CALLCONV *retro_perf_log_t)(void);

/**
 * Registers a new performance counter.
 *
 * If \c counter has already been registered beforehand,
 * this function does nothing.
 *
 * @param counter The counter to register.
 * \c counter::ident must be set to a unique identifier,
 * and all other values in \c counter must be set to zero or \c false.
 * Behavior is undefined if \c NULL.
 * @post If \c counter is successfully registered,
 * then \c counter::registered will be set to \c true.
 * Otherwise, it will be set to \c false.
 * Registration may fail if the frontend's maximum number of counters (if any) has been reached.
 * @note The counter is owned by the core and must not be freed by the frontend.
 * The frontend must also clean up any references to a core's performance counters
 * before unloading it, otherwise undefined behavior may occur.
 * @see retro_perf_start_t
 * @see retro_perf_stop_t
 */
typedef void (RETRO_CALLCONV *retro_perf_register_t)(struct retro_perf_counter *counter);

/**
 * Starts a registered performance counter.
 *
 * Call this just before the code you want to measure.
 *
 * @param counter The counter to start.
 * Behavior is undefined if \c NULL.
 * @see retro_perf_stop_t
 */
typedef void (RETRO_CALLCONV *retro_perf_start_t)(struct retro_perf_counter *counter);

/**
 * Stops a registered performance counter.
 *
 * Call this just after the code you want to measure.
 *
 * @param counter The counter to stop.
 * Behavior is undefined if \c NULL.
 * @see retro_perf_start_t
 * @see retro_perf_stop_t
 */
typedef void (RETRO_CALLCONV *retro_perf_stop_t)(struct retro_perf_counter *counter);

/**
 * An interface that the core can use to get performance information.
 *
 * Here's a usage example:
 *
 * @code{.c}
 * #ifdef PROFILING
 * // Wrapper macros to simplify using performance counters.
 * // Optional; tailor these to your project's needs.
 * #define RETRO_PERFORMANCE_INIT(perf_cb, name) static struct retro_perf_counter name = {#name}; if (!name.registered) perf_cb.perf_register(&(name))
 * #define RETRO_PERFORMANCE_START(perf_cb, name) perf_cb.perf_start(&(name))
 * #define RETRO_PERFORMANCE_STOP(perf_cb, name) perf_cb.perf_stop(&(name))
 * #else
 * // Exclude the performance counters if profiling is disabled.
 * #define RETRO_PERFORMANCE_INIT(perf_cb, name) ((void)0)
 * #define RETRO_PERFORMANCE_START(perf_cb, name) ((void)0)
 * #define RETRO_PERFORMANCE_STOP(perf_cb, name) ((void)0)
 * #endif
 *
 * // Defined somewhere else in the core.
 * extern struct retro_perf_callback perf_cb;
 *
 * void retro_run(void)
 * {
 *    RETRO_PERFORMANCE_INIT(cb, interesting);
 *    RETRO_PERFORMANCE_START(cb, interesting);
 *    interesting_work();
 *    RETRO_PERFORMANCE_STOP(cb, interesting);
 *
 *    RETRO_PERFORMANCE_INIT(cb, maybe_slow);
 *    RETRO_PERFORMANCE_START(cb, maybe_slow);
 *    more_interesting_work();
 *    RETRO_PERFORMANCE_STOP(cb, maybe_slow);
 * }
 *
 * void retro_deinit(void)
 * {
 *    // Asks the frontend to log the results of all performance counters.
 *    perf_cb.perf_log();
 * }
 * @endcode
 *
 * All functions are set by the frontend.
 *
 * @see RETRO_ENVIRONMENT_GET_PERF_INTERFACE
 */
struct retro_perf_callback
{
   /** @copydoc retro_perf_get_time_usec_t */
   retro_perf_get_time_usec_t    get_time_usec;

   /** @copydoc retro_perf_get_counter_t */
   retro_get_cpu_features_t      get_cpu_features;

   /** @copydoc retro_perf_get_counter_t */
   retro_perf_get_counter_t      get_perf_counter;

   /** @copydoc retro_perf_register_t */
   retro_perf_register_t         perf_register;

   /** @copydoc retro_perf_start_t */
   retro_perf_start_t            perf_start;

   /** @copydoc retro_perf_stop_t */
   retro_perf_stop_t             perf_stop;

   /** @copydoc retro_perf_log_t */
   retro_perf_log_t              perf_log;
};

/** @} */

/**
 * @defgroup RETRO_SENSOR Sensor Interface
 * @{
 */

/**
 * Defines actions that can be performed on sensors.
 * @note Cores should only enable sensors while they're actively being used;
 * depending on the frontend and platform,
 * enabling these sensors may impact battery life.
 *
 * @see RETRO_ENVIRONMENT_GET_SENSOR_INTERFACE
 * @see retro_sensor_interface
 * @see retro_set_sensor_state_t
 */
enum retro_sensor_action
{
   /** Enables accelerometer input, if one exists. */
   RETRO_SENSOR_ACCELEROMETER_ENABLE = 0,

   /** Disables accelerometer input, if one exists. */
   RETRO_SENSOR_ACCELEROMETER_DISABLE,

   /** Enables gyroscope input, if one exists. */
   RETRO_SENSOR_GYROSCOPE_ENABLE,

   /** Disables gyroscope input, if one exists. */
   RETRO_SENSOR_GYROSCOPE_DISABLE,

   /** Enables ambient light input, if a luminance sensor exists. */
   RETRO_SENSOR_ILLUMINANCE_ENABLE,

   /** Disables ambient light input, if a luminance sensor exists. */
   RETRO_SENSOR_ILLUMINANCE_DISABLE,

   /** @private Defined to ensure <tt>sizeof(enum retro_sensor_action) == sizeof(int)</tt>. Do not use. */
   RETRO_SENSOR_DUMMY = INT_MAX
};

/** @defgroup RETRO_SENSOR_ID Sensor Value IDs
 * @{
 */
/* Id values for SENSOR types. */

/**
 * Returns the device's acceleration along its local X axis, in g (standard gravity, 9.80665 m/s^2).
 * Includes the effect of gravity;
 * a device at rest on a table will have values close to 0, 0, 1.
 *
 * Positive values mean that the device is accelerating to the right,
 * assuming the user is looking at it head-on.
 */
#define RETRO_SENSOR_ACCELEROMETER_X 0

/**
 * Returns the device's acceleration along its local Y axis, in g (standard gravity, 9.80665 m/s^2).
 * Includes the effect of gravity.
 *
 * Positive values mean that the device is accelerating upwards,
 * assuming the user is looking at it head-on.
 */
#define RETRO_SENSOR_ACCELEROMETER_Y 1

/**
 * Returns the device's acceleration along its local Z axis, in g (standard gravity, 9.80665 m/s^2).
 * Includes the effect of gravity.
 *
 * Positive values indicate forward acceleration towards the user,
 * assuming the user is looking at the device head-on.
 */
#define RETRO_SENSOR_ACCELEROMETER_Z 2

/**
 * Returns the angular velocity of the device around its local X axis, in radians per second.
 *
 * Positive values indicate counter-clockwise rotation.
 *
 * @note A radian is about 57 degrees, and a full 360-degree rotation is 2*pi radians.
 * @see https://developer.android.com/reference/android/hardware/SensorEvent#sensor.type_gyroscope
 * for guidance on using this value to derive a device's orientation.
 */
#define RETRO_SENSOR_GYROSCOPE_X 3

/**
 * Returns the angular velocity of the device around its local Z axis, in radians per second.
 *
 * Positive values indicate counter-clockwise rotation.
 *
 * @note A radian is about 57 degrees, and a full 360-degree rotation is 2*pi radians.
 * @see https://developer.android.com/reference/android/hardware/SensorEvent#sensor.type_gyroscope
 * for guidance on using this value to derive a device's orientation.
 */
#define RETRO_SENSOR_GYROSCOPE_Y 4

/**
 * Returns the angular velocity of the device around its local Z axis, in radians per second.
 *
 * Positive values indicate counter-clockwise rotation.
 *
 * @note A radian is about 57 degrees, and a full 360-degree rotation is 2*pi radians.
 * @see https://developer.android.com/reference/android/hardware/SensorEvent#sensor.type_gyroscope
 * for guidance on using this value to derive a device's orientation.
 */
#define RETRO_SENSOR_GYROSCOPE_Z 5

/**
 * Returns the ambient illuminance (light intensity) of the device's environment, in lux.
 *
 * @see https://en.wikipedia.org/wiki/Lux for a table of common lux values.
 */
#define RETRO_SENSOR_ILLUMINANCE 6
/** @} */

/**
 * Adjusts the state of a sensor.
 *
 * @param port The device port of the controller that owns the sensor given in \c action.
 * @param action The action to perform on the sensor.
 * Different devices support different sensors.
 * @param rate The rate at which the underlying sensor should be updated, in Hz.
 * This should be treated as a hint,
 * as some device sensors may not support the requested rate
 * (if it's configurable at all).
 * @returns \c true if the sensor state was successfully adjusted, \c false otherwise.
 * @note If one of the \c RETRO_SENSOR_*_ENABLE actions fails,
 * this likely means that the given sensor is not available
 * on the provided \c port.
 * @see retro_sensor_action
 */
typedef bool (RETRO_CALLCONV *retro_set_sensor_state_t)(unsigned port,
      enum retro_sensor_action action, unsigned rate);

/**
 * Retrieves the current value reported by sensor.
 * @param port The device port of the controller that owns the sensor given in \c id.
 * @param id The sensor value to query.
 * @returns The current sensor value.
 * Exact semantics depend on the value given in \c id,
 * but will return 0 for invalid arguments.
 *
 * @see RETRO_SENSOR_ID
 */
typedef float (RETRO_CALLCONV *retro_sensor_get_input_t)(unsigned port, unsigned id);

/**
 * An interface that cores can use to access device sensors.
 *
 * All function pointers are set by the frontend.
 */
struct retro_sensor_interface
{
   /** @copydoc retro_set_sensor_state_t */
   retro_set_sensor_state_t set_sensor_state;

   /** @copydoc retro_sensor_get_input_t */
   retro_sensor_get_input_t get_sensor_input;
};

/** @} */

/** @defgroup GET_CAMERA_INTERFACE Camera Interface
 * @{
 */

/**
 * Denotes the type of buffer in which the camera will store its input.
 *
 * Different camera drivers may support different buffer types.
 *
 * @see RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE
 * @see retro_camera_callback
 */
enum retro_camera_buffer
{
   /**
    * Indicates that camera frames should be delivered to the core as an OpenGL texture.
    *
    * Requires that the core is using an OpenGL context via \c RETRO_ENVIRONMENT_SET_HW_RENDER.
    *
    * @see retro_camera_frame_opengl_texture_t
    */
   RETRO_CAMERA_BUFFER_OPENGL_TEXTURE = 0,

   /**
    * Indicates that camera frames should be delivered to the core as a raw buffer in memory.
    *
    * @see retro_camera_frame_raw_framebuffer_t
    */
   RETRO_CAMERA_BUFFER_RAW_FRAMEBUFFER,

   /**
    * @private Defined to ensure <tt>sizeof(enum retro_camera_buffer) == sizeof(int)</tt>.
    * Do not use.
    */
   RETRO_CAMERA_BUFFER_DUMMY = INT_MAX
};

/**
 * Starts an initialized camera.
 * The camera is disabled by default,
 * and must be enabled with this function before being used.
 *
 * Set by the frontend.
 *
 * @returns \c true if the camera was successfully started, \c false otherwise.
 * Failure may occur if no actual camera is available,
 * or if the frontend doesn't have permission to access it.
 * @note Must be called in \c retro_run().
 * @see retro_camera_callback
 */
typedef bool (RETRO_CALLCONV *retro_camera_start_t)(void);

/**
 * Stops the running camera.
 *
 * Set by the frontend.
 *
 * @note Must be called in \c retro_run().
 * @warning The frontend may close the camera on its own when unloading the core,
 * but this behavior is not guaranteed.
 * Cores should clean up the camera before exiting.
 * @see retro_camera_callback
 */
typedef void (RETRO_CALLCONV *retro_camera_stop_t)(void);

/**
 * Called by the frontend to report the state of the camera driver.
 *
 * @see retro_camera_callback
 */
typedef void (RETRO_CALLCONV *retro_camera_lifetime_status_t)(void);

/**
 * Called by the frontend to report a new camera frame,
 * delivered as a raw buffer in memory.
 *
 * Set by the core.
 *
 * @param buffer Pointer to the camera's most recent video frame.
 * Each pixel is in XRGB8888 format.
 * The first pixel represents the top-left corner of the image
 * (i.e. the Y axis goes downward).
 * @param width The width of the frame given in \c buffer, in pixels.
 * @param height The height of the frame given in \c buffer, in pixels.
 * @param pitch The width of the frame given in \c buffer, in bytes.
 * @warning \c buffer may be invalidated when this function returns,
 * so the core should make its own copy of \c buffer if necessary.
 * @see RETRO_CAMERA_BUFFER_RAW_FRAMEBUFFER
 */
typedef void (RETRO_CALLCONV *retro_camera_frame_raw_framebuffer_t)(const uint32_t *buffer,
      unsigned width, unsigned height, size_t pitch);

/**
 * Called by the frontend to report a new camera frame,
 * delivered as an OpenGL texture.
 *
 * @param texture_id The ID of the OpenGL texture that represents the camera's most recent frame.
 * Owned by the frontend, and must not be modified by the core.
 * @param texture_target The type of the texture given in \c texture_id.
 * Usually either \c GL_TEXTURE_2D or \c GL_TEXTURE_RECTANGLE,
 * but other types are allowed.
 * @param affine A pointer to a 3x3 column-major affine matrix
 * that can be used to transform pixel coordinates to texture coordinates.
 * After transformation, the bottom-left corner should have coordinates of <tt>(0, 0)</tt>
 * and the top-right corner should have coordinates of <tt>(1, 1)</tt>
 * (or <tt>(width, height)</tt> for \c GL_TEXTURE_RECTANGLE).
 *
 * @note GL-specific typedefs (e.g. \c GLfloat and \c GLuint) are avoided here
 * so that the API doesn't rely on gl.h.
 * @warning \c texture_id and \c affine may be invalidated when this function returns,
 * so the core should make its own copy of them if necessary.
 */
typedef void (RETRO_CALLCONV *retro_camera_frame_opengl_texture_t)(unsigned texture_id,
      unsigned texture_target, const float *affine);

/**
 * An interface that the core can use to access a device's camera.
 *
 * @see RETRO_ENVIRONMENT_GET_CAMERA_INTERFACE
 */
struct retro_camera_callback
{
   /**
    * Requested camera capabilities,
    * given as a bitmask of \c retro_camera_buffer values.
    * Set by the core.
    *
    * Here's a usage example:
    * @code
    * // Requesting support for camera data delivered as both an OpenGL texture and a pixel buffer:
    * struct retro_camera_callback callback;
    * callback.caps = (1 << RETRO_CAMERA_BUFFER_OPENGL_TEXTURE) | (1 << RETRO_CAMERA_BUFFER_RAW_FRAMEBUFFER);
    * @endcode
    */
   uint64_t caps;

   /**
    * The desired width of the camera frame, in pixels.
    * This is only a hint; the frontend may provide a different size.
    * Set by the core.
    * Use zero to let the frontend decide.
    */
   unsigned width;

   /**
    * The desired height of the camera frame, in pixels.
    * This is only a hint; the frontend may provide a different size.
     * Set by the core.
    * Use zero to let the frontend decide.
    */
   unsigned height;

   /**
    * @copydoc retro_camera_start_t
    * @see retro_camera_callback
    */
   retro_camera_start_t start;

   /**
    * @copydoc retro_camera_stop_t
    * @see retro_camera_callback
    */
   retro_camera_stop_t stop;

   /**
    * @copydoc retro_camera_frame_raw_framebuffer_t
    * @note If \c NULL, this function will not be called.
    */
   retro_camera_frame_raw_framebuffer_t frame_raw_framebuffer;

   /**
    * @copydoc retro_camera_frame_opengl_texture_t
    * @note If \c NULL, this function will not be called.
    */
   retro_camera_frame_opengl_texture_t frame_opengl_texture;

   /**
    * Core-defined callback invoked by the frontend right after the camera driver is initialized
    * (\em not when calling \c start).
    * May be \c NULL, in which case this function is skipped.
    */
   retro_camera_lifetime_status_t initialized;

   /**
    * Core-defined callback invoked by the frontend
    * right before the video camera driver is deinitialized
    * (\em not when calling \c stop).
    * May be \c NULL, in which case this function is skipped.
    */
   retro_camera_lifetime_status_t deinitialized;
};

/** @} */

/** @defgroup GET_LOCATION_INTERFACE Location Interface
 * @{
 */

/** @copydoc retro_location_callback::set_interval */
typedef void (RETRO_CALLCONV *retro_location_set_interval_t)(unsigned interval_ms,
      unsigned interval_distance);

/** @copydoc retro_location_callback::start */
typedef bool (RETRO_CALLCONV *retro_location_start_t)(void);

/** @copydoc retro_location_callback::stop */
typedef void (RETRO_CALLCONV *retro_location_stop_t)(void);

/** @copydoc retro_location_callback::get_position */
typedef bool (RETRO_CALLCONV *retro_location_get_position_t)(double *lat, double *lon,
      double *horiz_accuracy, double *vert_accuracy);

/** Function type that reports the status of the location service. */
typedef void (RETRO_CALLCONV *retro_location_lifetime_status_t)(void);

/**
 * An interface that the core can use to access a device's location.
 *
 * @note It is the frontend's responsibility to request the necessary permissions
 * from the operating system.
 * @see RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE
 */
struct retro_location_callback
{
   /**
    * Starts listening the device's location service.
    *
    * The frontend will report changes to the device's location
    * at the interval defined by \c set_interval.
    * Set by the frontend.
    *
    * @return true if location services were successfully started, false otherwise.
    * Note that this will return \c false if location services are disabled
    * or the frontend doesn't have permission to use them.
    * @note The device's location service may or may not have been enabled
    * before the core calls this function.
    */
   retro_location_start_t         start;

   /**
    * Stop listening to the device's location service.
    *
    * Set by the frontend.
    *
    * @note The location service itself may or may not
    * be turned off by this function,
    * depending on the platform and the frontend.
    * @post The core will stop receiving location service updates.
    */
   retro_location_stop_t          stop;

   /**
    * Returns the device's current coordinates.
    *
    * Set by the frontend.
    *
    * @param[out] lat Pointer to latitude, in degrees.
    * Will be set to 0 if no change has occurred since the last call.
    * Behavior is undefined if \c NULL.
    * @param[out] lon Pointer to longitude, in degrees.
    * Will be set to 0 if no change has occurred since the last call.
    * Behavior is undefined if \c NULL.
    * @param[out] horiz_accuracy Pointer to horizontal accuracy.
    * Will be set to 0 if no change has occurred since the last call.
    * Behavior is undefined if \c NULL.
    * @param[out] vert_accuracy Pointer to vertical accuracy.
    * Will be set to 0 if no change has occurred since the last call.
    * Behavior is undefined if \c NULL.
    */
   retro_location_get_position_t  get_position;

   /**
    * Sets the rate at which the location service should report updates.
    *
    * This is only a hint; the actual rate may differ.
    * Sets the interval of time and/or distance at which to update/poll
    * location-based data.
    *
    * Some platforms may only support one of the two parameters;
    * cores should provide both to ensure compatibility.
    *
    * Set by the frontend.
    *
    * @param interval_ms The desired period of time between location updates, in milliseconds.
    * @param interval_distance The desired distance between location updates, in meters.
    */
   retro_location_set_interval_t  set_interval;

   /** Called when the location service is initialized. Set by the core. Optional. */
   retro_location_lifetime_status_t initialized;

   /** Called when the location service is deinitialized. Set by the core. Optional. */
   retro_location_lifetime_status_t deinitialized;
};

/** @} */

/** @addtogroup GET_RUMBLE_INTERFACE
 * @{ */

/**
 * The type of rumble motor in a controller.
 *
 * Both motors can be controlled independently,
 * and the strong motor does not override the weak motor.
 * @see RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE
 */
enum retro_rumble_effect
{
   RETRO_RUMBLE_STRONG = 0,
   RETRO_RUMBLE_WEAK = 1,

   /** @private Defined to ensure <tt>sizeof(enum retro_rumble_effect) == sizeof(int)</tt>. Do not use. */
   RETRO_RUMBLE_DUMMY = INT_MAX
};

/**
 * Requests a rumble state change for a controller.
 * Set by the frontend.
 *
 * @param port The controller port to set the rumble state for.
 * @param effect The rumble motor to set the strength of.
 * @param strength The desired intensity of the rumble motor, ranging from \c 0 to \c 0xffff (inclusive).
 * @return \c true if the requested rumble state was honored.
 * If the controller doesn't support rumble, will return \c false.
 * @note Calling this before the first \c retro_run() may return \c false.
 * @see RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE
 */
typedef bool (RETRO_CALLCONV *retro_set_rumble_state_t)(unsigned port,
      enum retro_rumble_effect effect, uint16_t strength);

/**
 * An interface that the core can use to set the rumble state of a controller.
 * @see RETRO_ENVIRONMENT_GET_RUMBLE_INTERFACE
 */
struct retro_rumble_interface
{
   /** @copydoc retro_set_rumble_state_t */
   retro_set_rumble_state_t set_rumble_state;
};

/** @} */

/**
 * Called by the frontend to request audio samples.
 * The core should render audio within this function
 * using the callback provided by \c retro_set_audio_sample or \c retro_set_audio_sample_batch.
 *
 * @warning This function may be called by any thread,
 * therefore it must be thread-safe.
 * @see RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK
 * @see retro_audio_callback
 * @see retro_audio_sample_batch_t
 * @see retro_audio_sample_t
 */
typedef void (RETRO_CALLCONV *retro_audio_callback_t)(void);

/**
 * Called by the frontend to notify the core that it should pause or resume audio rendering.
 * The initial state of the audio driver after registering this callback is \c false (inactive).
 *
 * @param enabled \c true if the frontend's audio driver is active.
 * If so, the registered audio callback will be called regularly.
 * If not, the audio callback will not be invoked until the next time
 * the frontend calls this function with \c true.
 * @warning This function may be called by any thread,
 * therefore it must be thread-safe.
 * @note Even if no audio samples are rendered,
 * the core should continue to update its emulated platform's audio engine if necessary.
 * @see RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK
 * @see retro_audio_callback
 * @see retro_audio_callback_t
 */
typedef void (RETRO_CALLCONV *retro_audio_set_state_callback_t)(bool enabled);

/**
 * An interface that the frontend uses to request audio samples from the core.
 * @note To unregister a callback, pass a \c retro_audio_callback_t
 * with both fields set to <tt>NULL</tt>.
 * @see RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK
 */
struct retro_audio_callback
{
   /** @see retro_audio_callback_t */
   retro_audio_callback_t callback;

   /** @see retro_audio_set_state_callback_t */
   retro_audio_set_state_callback_t set_state;
};

typedef int64_t retro_usec_t;

/**
 * Called right before each iteration of \c retro_run
 * if registered via <tt>RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK</tt>.
 *
 * @param usec Time since the last call to <tt>retro_run</tt>, in microseconds.
 * If the frontend is manipulating the frame time
 * (e.g. via fast-forward or slow motion),
 * this value will be the reference value initially provided to the environment call.
 * @see RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK
 * @see retro_frame_time_callback
 */
typedef void (RETRO_CALLCONV *retro_frame_time_callback_t)(retro_usec_t usec);

/**
 * @see RETRO_ENVIRONMENT_SET_FRAME_TIME_CALLBACK
 */
struct retro_frame_time_callback
{
   /**
    * Called to notify the core of the current frame time.
    * If <tt>NULL</tt>, the frontend will clear its registered callback.
    */
   retro_frame_time_callback_t callback;

   /**
    * The ideal duration of one frame, in microseconds.
    * Compute it as <tt>1000000 / fps</tt>.
    * The frontend will resolve rounding to ensure that framestepping, etc is exact.
    */
   retro_usec_t reference;
};

/** @defgroup SET_AUDIO_BUFFER_STATUS_CALLBACK Audio Buffer Occupancy
 * @{
 */

/**
 * Notifies a libretro core of how full the frontend's audio buffer is.
 * Set by the core, called by the frontend.
 * It will be called right before \c retro_run() every frame.
 *
 * @param active \c true if the frontend's audio buffer is currently in use,
 * \c false if audio is disabled in the frontend.
 * @param occupancy A value between 0 and 100 (inclusive),
 * corresponding to the frontend's audio buffer occupancy percentage.
 * @param underrun_likely \c true if the frontend expects an audio buffer underrun
 * during the next frame, which indicates that a core should attempt frame-skipping.
 */
typedef void (RETRO_CALLCONV *retro_audio_buffer_status_callback_t)(
      bool active, unsigned occupancy, bool underrun_likely);

/**
 * A callback to register with the frontend to receive audio buffer occupancy information.
 */
struct retro_audio_buffer_status_callback
{
   /** @copydoc retro_audio_buffer_status_callback_t */
   retro_audio_buffer_status_callback_t callback;
};

/** @} */

/* Pass this to retro_video_refresh_t if rendering to hardware.
 * Passing NULL to retro_video_refresh_t is still a frame dupe as normal.
 * */
#define RETRO_HW_FRAME_BUFFER_VALID ((void*)-1)

/* Invalidates the current HW context.
 * Any GL state is lost, and must not be deinitialized explicitly.
 * If explicit deinitialization is desired by the libretro core,
 * it should implement context_destroy callback.
 * If called, all GPU resources must be reinitialized.
 * Usually called when frontend reinits video driver.
 * Also called first time video driver is initialized,
 * allowing libretro core to initialize resources.
 */
typedef void (RETRO_CALLCONV *retro_hw_context_reset_t)(void);

/* Gets current framebuffer which is to be rendered to.
 * Could change every frame potentially.
 */
typedef uintptr_t (RETRO_CALLCONV *retro_hw_get_current_framebuffer_t)(void);

/* Get a symbol from HW context. */
typedef retro_proc_address_t (RETRO_CALLCONV *retro_hw_get_proc_address_t)(const char *sym);

enum retro_hw_context_type
{
   RETRO_HW_CONTEXT_NONE             = 0,
   /* OpenGL 2.x. Driver can choose to use latest compatibility context. */
   RETRO_HW_CONTEXT_OPENGL           = 1,
   /* OpenGL ES 2.0. */
   RETRO_HW_CONTEXT_OPENGLES2        = 2,
   /* Modern desktop core GL context. Use version_major/
    * version_minor fields to set GL version. */
   RETRO_HW_CONTEXT_OPENGL_CORE      = 3,
   /* OpenGL ES 3.0 */
   RETRO_HW_CONTEXT_OPENGLES3        = 4,
   /* OpenGL ES 3.1+. Set version_major/version_minor. For GLES2 and GLES3,
    * use the corresponding enums directly. */
   RETRO_HW_CONTEXT_OPENGLES_VERSION = 5,

   /* Vulkan, see RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE. */
   RETRO_HW_CONTEXT_VULKAN           = 6,

   /* Direct3D11, see RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE */
   RETRO_HW_CONTEXT_D3D11            = 7,

   /* Direct3D10, see RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE */
   RETRO_HW_CONTEXT_D3D10            = 8,

   /* Direct3D12, see RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE */
   RETRO_HW_CONTEXT_D3D12            = 9,

   /* Direct3D9, see RETRO_ENVIRONMENT_GET_HW_RENDER_INTERFACE */
   RETRO_HW_CONTEXT_D3D9             = 10,

   /** Dummy value to ensure sizeof(enum retro_hw_context_type) == sizeof(int). Do not use. */
   RETRO_HW_CONTEXT_DUMMY = INT_MAX
};

struct retro_hw_render_callback
{
   /* Which API to use. Set by libretro core. */
   enum retro_hw_context_type context_type;

   /* Called when a context has been created or when it has been reset.
    * An OpenGL context is only valid after context_reset() has been called.
    *
    * When context_reset is called, OpenGL resources in the libretro
    * implementation are guaranteed to be invalid.
    *
    * It is possible that context_reset is called multiple times during an
    * application lifecycle.
    * If context_reset is called without any notification (context_destroy),
    * the OpenGL context was lost and resources should just be recreated
    * without any attempt to "free" old resources.
    */
   retro_hw_context_reset_t context_reset;

   /* Set by frontend.
    * TODO: This is rather obsolete. The frontend should not
    * be providing preallocated framebuffers. */
   retro_hw_get_current_framebuffer_t get_current_framebuffer;

   /* Set by frontend.
    * Can return all relevant functions, including glClear on Windows. */
   retro_hw_get_proc_address_t get_proc_address;

   /* Set if render buffers should have depth component attached.
    * TODO: Obsolete. */
   bool depth;

   /* Set if stencil buffers should be attached.
    * TODO: Obsolete. */
   bool stencil;

   /* If depth and stencil are true, a packed 24/8 buffer will be added.
    * Only attaching stencil is invalid and will be ignored. */

   /* Use conventional bottom-left origin convention. If false,
    * standard libretro top-left origin semantics are used.
    * TODO: Move to GL specific interface. */
   bool bottom_left_origin;

   /* Major version number for core GL context or GLES 3.1+. */
   unsigned version_major;

   /* Minor version number for core GL context or GLES 3.1+. */
   unsigned version_minor;

   /* If this is true, the frontend will go very far to avoid
    * resetting context in scenarios like toggling fullscreen, etc.
    * TODO: Obsolete? Maybe frontend should just always assume this ...
    */
   bool cache_context;

   /* The reset callback might still be called in extreme situations
    * such as if the context is lost beyond recovery.
    *
    * For optimal stability, set this to false, and allow context to be
    * reset at any time.
    */

   /* A callback to be called before the context is destroyed in a
    * controlled way by the frontend. */
   retro_hw_context_reset_t context_destroy;

   /* OpenGL resources can be deinitialized cleanly at this step.
    * context_destroy can be set to NULL, in which resources will
    * just be destroyed without any notification.
    *
    * Even when context_destroy is non-NULL, it is possible that
    * context_reset is called without any destroy notification.
    * This happens if context is lost by external factors (such as
    * notified by GL_ARB_robustness).
    *
    * In this case, the context is assumed to be already dead,
    * and the libretro implementation must not try to free any OpenGL
    * resources in the subsequent context_reset.
    */

   /* Creates a debug context. */
   bool debug_context;
};

/* Callback type passed in RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK.
 * Called by the frontend in response to keyboard events.
 * down is set if the key is being pressed, or false if it is being released.
 * keycode is the RETROK value of the char.
 * character is the text character of the pressed key. (UTF-32).
 * key_modifiers is a set of RETROKMOD values or'ed together.
 *
 * The pressed/keycode state can be independent of the character.
 * It is also possible that multiple characters are generated from a
 * single keypress.
 * Keycode events should be treated separately from character events.
 * However, when possible, the frontend should try to synchronize these.
 * If only a character is posted, keycode should be RETROK_UNKNOWN.
 *
 * Similarly if only a keycode event is generated with no corresponding
 * character, character should be 0.
 */
typedef void (RETRO_CALLCONV *retro_keyboard_event_t)(bool down, unsigned keycode,
      uint32_t character, uint16_t key_modifiers);

struct retro_keyboard_callback
{
   retro_keyboard_event_t callback;
};

/** @defgroup SET_DISK_CONTROL_INTERFACE Disk Control
 *
 * Callbacks for inserting and removing disks from the emulated console at runtime.
 * Should be provided by cores that support doing so.
 * Cores should automate this process if possible,
 * but some cases require the player's manual input.
 *
 * The steps for swapping disk images are generally as follows:
 *
 * \li Eject the emulated console's disk drive with \c set_eject_state(true).
 * \li Insert the new disk image with \c set_image_index(index).
 * \li Close the virtual disk tray with \c set_eject_state(false).
 *
 * @{
 */

/**
 * Called by the frontend to open or close the emulated console's virtual disk tray.
 *
 * The frontend may only set the disk image index
 * while the emulated tray is opened.
 *
 * If the emulated console's disk tray is already in the state given by \c ejected,
 * then this function should return \c true without doing anything.
 * The core should return \c false if it couldn't change the disk tray's state;
 * this may happen if the console itself limits when the disk tray can be open or closed
 * (e.g. to wait for the disc to stop spinning).
 *
 * @param ejected \c true if the virtual disk tray should be "ejected",
 * \c false if it should be "closed".
 * @return \c true if the virtual disk tray's state has been set to the given state,
 * false if there was an error.
 * @see retro_get_eject_state_t
 */
typedef bool (RETRO_CALLCONV *retro_set_eject_state_t)(bool ejected);

/**
 * Gets the current ejected state of the disk drive.
 * The initial state is closed, i.e. \c false.
 *
 * @return \c true if the virtual disk tray is "ejected",
 * i.e. it's open and a disk can be inserted.
 * @see retro_set_eject_state_t
 */
typedef bool (RETRO_CALLCONV *retro_get_eject_state_t)(void);

/**
 * Gets the index of the current disk image,
 * as determined by however the frontend orders disk images
 * (such as m3u-formatted playlists or special directories).
 *
 * @return The index of the current disk image
 * (starting with 0 for the first disk),
 * or a value greater than or equal to \c get_num_images() if no disk is inserted.
 * @see retro_get_num_images_t
 */
typedef unsigned (RETRO_CALLCONV *retro_get_image_index_t)(void);

/**
 * Inserts the disk image at the given index into the emulated console's drive.
 * Can only be called while the disk tray is ejected
 * (i.e. \c retro_get_eject_state_t returns \c true).
 *
 * If the emulated disk tray is ejected
 * and already contains the disk image named by \c index,
 * then this function should do nothing and return \c true.
 *
 * @param index The index of the disk image to insert,
 * starting from 0 for the first disk.
 * A value greater than or equal to \c get_num_images()
 * represents the frontend removing the disk without inserting a new one.
 * @return \c true if the disk image was successfully set.
 * \c false if the disk tray isn't ejected or there was another error
 * inserting a new disk image.
 */
typedef bool (RETRO_CALLCONV *retro_set_image_index_t)(unsigned index);

/**
 * @return The number of disk images which are available to use.
 * These are most likely defined in a playlist file.
 */
typedef unsigned (RETRO_CALLCONV *retro_get_num_images_t)(void);

struct retro_game_info;

/**
 * Replaces the disk image at the given index with a new disk.
 *
 * Replaces the disk image associated with index.
 * Arguments to pass in info have same requirements as retro_load_game().
 * Virtual disk tray must be ejected when calling this.
 *
 * Passing \c NULL to this function indicates
 * that the frontend has removed this disk image from its internal list.
 * As a result, calls to this function can change the number of available disk indexes.
 *
 * For example, calling <tt>replace_image_index(1, NULL)</tt>
 * will remove the disk image at index 1,
 * and the disk image at index 2 (if any)
 * will be moved to the newly-available index 1.
 *
 * @param index The index of the disk image to replace.
 * @param info Details about the new disk image,
 * or \c NULL if the disk image at the given index should be discarded.
 * The semantics of each field are the same as in \c retro_load_game.
 * @return \c true if the disk image was successfully replaced
 * or removed from the playlist,
 * \c false if the tray is not ejected
 * or if there was an error.
 */
typedef bool (RETRO_CALLCONV *retro_replace_image_index_t)(unsigned index,
      const struct retro_game_info *info);

/**
 * Adds a new index to the core's internal disk list.
 * This will increment the return value from \c get_num_images() by 1.
 * This image index cannot be used until a disk image has been set
 * with \c replace_image_index.
 *
 * @return \c true if the core has added space for a new disk image
 * and is ready to receive one.
 */
typedef bool (RETRO_CALLCONV *retro_add_image_index_t)(void);

/**
 * Sets the disk image that will be inserted into the emulated disk drive
 * before \c retro_load_game is called.
 *
 * \c retro_load_game does not provide a way to ensure
 * that a particular disk image in a playlist is inserted into the console;
 * this function makes up for that.
 * Frontends should call it immediately before \c retro_load_game,
 * and the core should use the arguments
 * to validate the disk image in \c retro_load_game.
 *
 * When content is loaded, the core should verify that the
 * disk specified by \c index can be found at \c path.
 * This is to guard against auto-selecting the wrong image
 * if (for example) the user should modify an existing M3U playlist.
 * We have to let the core handle this because
 * \c set_initial_image() must be called before loading content,
 * i.e. the frontend cannot access image paths in advance
 * and thus cannot perform the error check itself.
 * If \c index is invalid (i.e. <tt>index >= get_num_images()</tt>)
 * or the disk image doesn't match the value given in \c path,
 * the core should ignore the arguments
 * and insert the disk at index 0 into the virtual disk tray.
 *
 * @warning If \c RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE is called within \c retro_load_game,
 * then this function may not be executed.
 * Set the disk control interface in \c retro_init if possible.
 *
 * @param index The index of the disk image within the playlist to set.
 * @param path The path of the disk image to set as the first.
 * The core should not load this path immediately;
 * instead, it should use it within \c retro_load_game
 * to verify that the correct disk image was loaded.
 * @return \c true if the initial disk index was set,
 * \c false if the arguments are invalid
 * or the core doesn't support this function.
 */
typedef bool (RETRO_CALLCONV *retro_set_initial_image_t)(unsigned index, const char *path);

/**
 * Returns the path of the disk image at the given index
 * on the host's file system.
 *
 * @param index The index of the disk image to get the path of.
 * @param s A buffer to store the path in.
 * @param len The size of \c s, in bytes.
 * @return \c true if the disk image's location was successfully
 * queried and copied into \c s,
 * \c false if the index is invalid
 * or the core couldn't locate the disk image.
 */
typedef bool (RETRO_CALLCONV *retro_get_image_path_t)(unsigned index, char *s, size_t len);

/**
 * Returns a friendly label for the given disk image.
 *
 * In the simplest case, this may be the disk image's file name
 * with the extension omitted.
 * For cores or games with more complex content requirements,
 * the label can be used to provide information to help the player
 * select a disk image to insert;
 * for example, a core may label different kinds of disks
 * (save data, level disk, installation disk, bonus content, etc.).
 * with names that correspond to in-game prompts,
 * so that the frontend can provide better guidance to the player.
 *
 * @param index The index of the disk image to return a label for.
 * @param s A buffer to store the resulting label in.
 * @param len The length of \c s, in bytes.
 * @return \c true if the disk image at \c index is valid
 * and a label was copied into \c s.
 */
typedef bool (RETRO_CALLCONV *retro_get_image_label_t)(unsigned index, char *s, size_t len);

/**
 * An interface that the frontend can use to exchange disks
 * within the emulated console's disk drive.
 *
 * All function pointers are required.
 *
 * @deprecated This struct is superseded by \ref retro_disk_control_ext_callback.
 * Only use this one to maintain compatibility
 * with older cores and frontends.
 *
 * @see RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE
 * @see retro_disk_control_ext_callback
 */
struct retro_disk_control_callback
{
   /** @copydoc retro_set_eject_state_t */
   retro_set_eject_state_t set_eject_state;

   /** @copydoc retro_get_eject_state_t */
   retro_get_eject_state_t get_eject_state;

   /** @copydoc retro_get_image_index_t */
   retro_get_image_index_t get_image_index;

   /** @copydoc retro_set_image_index_t */
   retro_set_image_index_t set_image_index;

   /** @copydoc retro_get_num_images_t */
   retro_get_num_images_t  get_num_images;

   /** @copydoc retro_replace_image_index_t */
   retro_replace_image_index_t replace_image_index;

   /** @copydoc retro_add_image_index_t */
   retro_add_image_index_t add_image_index;
};

/**
 * @copybrief retro_disk_control_callback
 *
 * All function pointers are required unless otherwise noted.
 *
 * @see RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE
 */
struct retro_disk_control_ext_callback
{
   /** @copydoc retro_set_eject_state_t */
   retro_set_eject_state_t set_eject_state;

   /** @copydoc retro_get_eject_state_t */
   retro_get_eject_state_t get_eject_state;

   /** @copydoc retro_get_image_index_t */
   retro_get_image_index_t get_image_index;

   /** @copydoc retro_set_image_index_t */
   retro_set_image_index_t set_image_index;

   /** @copydoc retro_get_num_images_t */
   retro_get_num_images_t  get_num_images;

   /** @copydoc retro_replace_image_index_t */
   retro_replace_image_index_t replace_image_index;

   /** @copydoc retro_add_image_index_t */
   retro_add_image_index_t add_image_index;

   /** @copydoc retro_set_initial_image_t
    *
    * Optional; not called if \c NULL.
    *
    * @note The frontend will only try to record/restore the last-used disk index
    * if both \c set_initial_image and \c get_image_path are implemented.
    */
   retro_set_initial_image_t set_initial_image;

   /**
    * @copydoc retro_get_image_path_t
    *
    * Optional; not called if \c NULL.
    */
   retro_get_image_path_t get_image_path;

   /**
    * @copydoc retro_get_image_label_t
    *
    * Optional; not called if \c NULL.
    */
   retro_get_image_label_t get_image_label;
};

/** @} */

/* Definitions for RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE.
 * A core can set it if sending and receiving custom network packets
 * during a multiplayer session is desired.
 */

/* Netpacket flags for retro_netpacket_send_t */
#define RETRO_NETPACKET_UNRELIABLE  0        /* Packet to be sent unreliable, depending on network quality it might not arrive. */
#define RETRO_NETPACKET_RELIABLE    (1 << 0) /* Reliable packets are guaranteed to arrive at the target in the order they were sent. */
#define RETRO_NETPACKET_UNSEQUENCED (1 << 1) /* Packet will not be sequenced with other packets and may arrive out of order. Cannot be set on reliable packets. */
#define RETRO_NETPACKET_FLUSH_HINT  (1 << 2) /* Request the packet and any previously buffered ones to be sent immediately */

/* Broadcast client_id for retro_netpacket_send_t */
#define RETRO_NETPACKET_BROADCAST 0xFFFF

/* Used by the core to send a packet to one or all connected players.
 * A single packet sent via this interface can contain up to 64 KB of data.
 *
 * The client_id RETRO_NETPACKET_BROADCAST sends the packet as a broadcast to
 * all connected players. This is supported from the host as well as clients.
*  Otherwise, the argument indicates the player to send the packet to.
 *
 * A frontend must support sending reliable packets (RETRO_NETPACKET_RELIABLE).
 * Unreliable packets might not be supported by the frontend, but the flags can
 * still be specified. Reliable transmission will be used instead.
 *
 * Calling this with the flag RETRO_NETPACKET_FLUSH_HINT will send off the
 * packet and any previously buffered ones immediately and without blocking.
 * To only flush previously queued packets, buf or len can be passed as NULL/0.
 *
 * This function is not guaranteed to be thread-safe and must be called during
 * retro_run or any of the netpacket callbacks passed with this interface.
 */
typedef void (RETRO_CALLCONV *retro_netpacket_send_t)(int flags, const void* buf, size_t len, uint16_t client_id);

/* Optionally read any incoming packets without waiting for the end of the
 * frame. While polling, retro_netpacket_receive_t and retro_netpacket_stop_t
 * can be called. The core can perform this in a loop to do a blocking read,
 * i.e., wait for incoming data, but needs to handle stop getting called and
 * also give up after a short while to avoid freezing on a connection problem.
 * It is a good idea to manually flush outgoing packets before calling this.
 *
 * This function is not guaranteed to be thread-safe and must be called during
 * retro_run or any of the netpacket callbacks passed with this interface.
 */
typedef void (RETRO_CALLCONV *retro_netpacket_poll_receive_t)(void);

/* Called by the frontend to signify that a multiplayer session has started.
 * If client_id is 0 the local player is the host of the session and at this
 * point no other player has connected yet.
 *
 * If client_id is > 0 the local player is a client connected to a host and
 * at this point is already fully connected to the host.
 *
 * The core must store the function pointer send_fn and use it whenever it
 * wants to send a packet. Optionally poll_receive_fn can be stored and used
 * when regular receiving between frames is not enough. These function pointers
 * remain valid until the frontend calls retro_netpacket_stop_t.
 */
typedef void (RETRO_CALLCONV *retro_netpacket_start_t)(uint16_t client_id, retro_netpacket_send_t send_fn, retro_netpacket_poll_receive_t poll_receive_fn);

/* Called by the frontend when a new packet arrives which has been sent from
 * another player with retro_netpacket_send_t. The client_id argument indicates
 * who has sent the packet.
 */
typedef void (RETRO_CALLCONV *retro_netpacket_receive_t)(const void* buf, size_t len, uint16_t client_id);

/* Called by the frontend when the multiplayer session has ended.
 * Once this gets called the function pointers passed to
 * retro_netpacket_start_t will not be valid anymore.
 */
typedef void (RETRO_CALLCONV *retro_netpacket_stop_t)(void);

/* Called by the frontend every frame (between calls to retro_run while
 * updating the state of the multiplayer session.
 * This is a good place for the core to call retro_netpacket_send_t from.
 */
typedef void (RETRO_CALLCONV *retro_netpacket_poll_t)(void);

/* Called by the frontend when a new player connects to the hosted session.
 * This is only called on the host side, not for clients connected to the host.
 * If this function returns false, the newly connected player gets dropped.
 * This can be used for example to limit the number of players.
 */
typedef bool (RETRO_CALLCONV *retro_netpacket_connected_t)(uint16_t client_id);

/* Called by the frontend when a player leaves or disconnects from the hosted session.
 * This is only called on the host side, not for clients connected to the host.
 */
typedef void (RETRO_CALLCONV *retro_netpacket_disconnected_t)(uint16_t client_id);

/**
 * A callback interface for giving a core the ability to send and receive custom
 * network packets during a multiplayer session between two or more instances
 * of a libretro frontend.
 *
 * Normally during connection handshake the frontend will compare library_version
 * used by both parties and show a warning if there is a difference. When the core
 * supplies protocol_version, the frontend will check against this instead.
 *
 * @see RETRO_ENVIRONMENT_SET_NETPACKET_INTERFACE
 */
struct retro_netpacket_callback
{
   retro_netpacket_start_t        start;
   retro_netpacket_receive_t      receive;
   retro_netpacket_stop_t         stop;         /* Optional - may be NULL */
   retro_netpacket_poll_t         poll;         /* Optional - may be NULL */
   retro_netpacket_connected_t    connected;    /* Optional - may be NULL */
   retro_netpacket_disconnected_t disconnected; /* Optional - may be NULL */
   const char* protocol_version; /* Optional - if not NULL will be used instead of core version to decide if communication is compatible */
};

/**
 * The pixel format used for rendering.
 * @see RETRO_ENVIRONMENT_SET_PIXEL_FORMAT
 */
enum retro_pixel_format
{
   /**
    * 0RGB1555, native endian.
    * Used as the default if \c RETRO_ENVIRONMENT_SET_PIXEL_FORMAT is not called.
    * The most significant bit must be set to 0.
    * @deprecated This format remains supported to maintain compatibility.
    * New code should use <tt>RETRO_PIXEL_FORMAT_RGB565</tt> instead.
    * @see RETRO_PIXEL_FORMAT_RGB565
    */
   RETRO_PIXEL_FORMAT_0RGB1555 = 0,

   /**
    * XRGB8888, native endian.
    * The most significant byte (the <tt>X</tt>) is ignored.
    */
   RETRO_PIXEL_FORMAT_XRGB8888 = 1,

   /**
    * RGB565, native endian.
    * This format is recommended if 16-bit pixels are desired,
    * as it is available on a variety of devices and APIs.
    */
   RETRO_PIXEL_FORMAT_RGB565   = 2,

   /**
    * XRGB2101010, native endian.
    * 32-bit packed: 2 ignored high bits followed by 10-bit R, G, B
    * (i.e. bits [29:20]=R, [19:10]=G, [9:0]=B; the top 2 bits are ignored).
    * Intended for cores that decode 10-bit-per-channel content (e.g. HDR10
    * sources) and want to pass it through without narrowing to 8 bits.
    *
    * A frontend is not required to render this natively: if the active video
    * driver does not support a 10-bit source surface, the frontend transparently
    * down-converts to XRGB8888, so a core may rely on SET_PIXEL_FORMAT accepting
    * this value but should not assume the display path is 10-bit end to end.
    */
   RETRO_PIXEL_FORMAT_XRGB2101010 = 3,

   /**
    * HDR10: PQ-encoded Rec.2020, 10 bits per channel, native endian.
    *
    * Bit layout is identical to #RETRO_PIXEL_FORMAT_XRGB2101010 -- 2 ignored
    * high bits then 10-bit R, G, B (bits [29:20]=R, [19:10]=G, [9:0]=B) --
    * but the *encoding* differs, and that is the whole point of a separate
    * value: the samples are SMPTE ST.2084 (PQ) over Rec.2020 primaries,
    * covering 0..10000 nits absolute, exactly as HDR10 video does.
    *
    * XRGB2101010 is 10-bit SDR: the frontend treats 1.0 as paper white and
    * cannot represent anything brighter, so a core has no way to make a
    * highlight exceed the SDR white level.  With this format the core
    * chooses absolute luminance per pixel, so specular highlights, muzzle
    * flashes, explosions and emissive surfaces can sit well above paper
    * white while the rest of the image stays where it was.
    *
    * A frontend that accepts this format MUST pass the samples through to an
    * HDR10 (PQ / Rec.2020) swapchain without re-encoding them: no inverse
    * tonemap, no Rec.709->Rec.2020 rotation, no paper-white scaling, since
    * the core has already applied all of it.  A frontend that cannot present
    * HDR10 natively must reject the format from SET_PIXEL_FORMAT rather than
    * silently down-converting -- PQ samples interpreted as SDR look badly
    * wrong, so the usual transparent narrowing is not safe here.  Cores
    * should therefore keep an SDR path and fall back when this is refused.
    *
    * Cores should query #RETRO_ENVIRONMENT_GET_HDR_PAPER_WHITE_NITS to learn
    * the luminance the user considers "SDR white" and map their normal
    * output to it; content authored above that value is what produces the
    * HDR effect.
    */
   RETRO_PIXEL_FORMAT_HDR10_2101010 = 4,

   /** 
    * @private Defined to ensure that <tt>sizeof(retro_pixel_format) == sizeof(int)</tt>.
	* Do not use.
	*/
   RETRO_PIXEL_FORMAT_UNKNOWN  = INT_MAX
};

/** @defgroup GET_SAVESTATE_CONTEXT Savestate Context
 * @{
 */

/**
 * Details about how the frontend will use savestates.
 *
 * @see RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT
 * @see retro_serialize
 */
enum retro_savestate_context
{
   /**
    * Standard savestate written to disk.
    * May be loaded at any time,
    * even in a separate session or on another device.
    *
    * Should not contain any pointers to code or data.
    */
   RETRO_SAVESTATE_CONTEXT_NORMAL                 = 0,

   /**
    * The savestate is guaranteed to be loaded
    * within the same session, address space, and binary.
    * Will not be written to disk or sent over the network;
    * therefore, internal pointers to code or data are acceptable.
    * May still be loaded or saved at any time.
    *
    * @note This context generally implies the use of runahead or rewinding,
    * which may work by taking savestates multiple times per second.
    * Savestate code that runs in this context should be fast.
    */
   RETRO_SAVESTATE_CONTEXT_RUNAHEAD_SAME_INSTANCE = 1,

   /**
    * The savestate is guaranteed to be loaded
    * in the same session and by the same binary,
    * but possibly by a different address space
    * (e.g. for "second instance" runahead)
    *
    * Will not be written to disk or sent over the network,
    * but may be loaded in a different address space.
    * Therefore, the savestate <em>must not</em> contain pointers.
    */
   RETRO_SAVESTATE_CONTEXT_RUNAHEAD_SAME_BINARY   = 2,

   /**
    * The savestate will not be written to disk,
    * but no other guarantees are made.
    * The savestate will almost certainly be loaded
    * by a separate binary, device, and address space.
    *
    * This context is intended for use with frontends that support rollback netplay.
    * Serialized state should omit any data that would unnecessarily increase bandwidth usage.
    * Must not contain pointers, and integers must be saved in big-endian format.
    * @see retro_endianness.h
    * @see network_stream
    */
   RETRO_SAVESTATE_CONTEXT_ROLLBACK_NETPLAY       = 3,

   /**
    * @private Defined to ensure <tt>sizeof(retro_savestate_context) == sizeof(int)</tt>.
    * Do not use.
    */
   RETRO_SAVESTATE_CONTEXT_UNKNOWN                = INT_MAX
};

/** @} */

/** @defgroup SET_MESSAGE User-Visible Messages
 *
 * @{
 */

/**
 * Defines a message that the frontend will display to the user,
 * as determined by <tt>RETRO_ENVIRONMENT_SET_MESSAGE</tt>.
 *
 * @deprecated This struct is superseded by \ref retro_message_ext,
 * which provides more control over how a message is presented.
 * Only use it for compatibility with older cores and frontends.
 *
 * @see RETRO_ENVIRONMENT_SET_MESSAGE
 * @see retro_message_ext
 */
struct retro_message
{
   /**
    * Null-terminated message to be displayed.
    * If \c NULL or empty, the message will be ignored.
    */
   const char *msg;

   /** Duration to display \c msg in frames. */
   unsigned    frames;
};

/**
 * The method that the frontend will use to display a message to the player.
 * @see retro_message_ext
 */
enum retro_message_target
{
   /**
    * Indicates that the frontend should display the given message
    * using all other targets defined by \c retro_message_target at once.
    */
   RETRO_MESSAGE_TARGET_ALL = 0,

   /**
    * Indicates that the frontend should display the given message
    * using the frontend's on-screen display, if available.
    *
    * @attention If the frontend allows players to customize or disable notifications,
    * then they may not see messages sent to this target.
    */
   RETRO_MESSAGE_TARGET_OSD,

   /**
    * Indicates that the frontend should log the message
    * via its usual logging mechanism, if available.
    *
    * This is not intended to be a substitute for \c RETRO_ENVIRONMENT_SET_LOG_INTERFACE.
    * It is intended for the common use case of
    * logging a player-facing message.
    *
    * This target should not be used for messages
    * of type \c RETRO_MESSAGE_TYPE_STATUS or \c RETRO_MESSAGE_TYPE_PROGRESS,
    * as it may add unnecessary noise to a log file.
    *
    * @see RETRO_ENVIRONMENT_SET_LOG_INTERFACE
    */
   RETRO_MESSAGE_TARGET_LOG
};

/**
 * A broad category for the type of message that the frontend will display.
 *
 * Each message type has its own use case,
 * therefore the frontend should present each one differently.
 *
 * @note This is a hint that the frontend may ignore.
 * The frontend should fall back to \c RETRO_MESSAGE_TYPE_NOTIFICATION
 * for message types that it doesn't support.
 */
enum retro_message_type
{
   /**
    * A standard on-screen message.
    *
    * Suitable for a variety of use cases,
    * such as messages about errors
    * or other important events.
    *
    * Frontends that display their own messages
    * should display this type of core-generated message the same way.
    */
   RETRO_MESSAGE_TYPE_NOTIFICATION = 0,

   /**
    * An on-screen message that should be visually distinct
    * from \c RETRO_MESSAGE_TYPE_NOTIFICATION messages.
    *
    * The exact meaning of "visually distinct" is left to the frontend,
    * but this usually implies that the frontend shows the message
    * in a way that it doesn't typically use for its own notices.
    */
   RETRO_MESSAGE_TYPE_NOTIFICATION_ALT,

   /**
    * Indicates a frequently-updated status display,
    * rather than a standard notification.
    * Status messages are intended to be displayed permanently while a core is running
    * in a way that doesn't suggest user action is required.
    *
    * Here are some possible use cases for status messages:
    *
    * @li An internal framerate counter.
    * @li Debugging information.
    *     Remember to let the player disable it in the core options.
    * @li Core-specific state, such as when a microphone is active.
    *
    * The status message is displayed for the given duration,
    * unless another status message of equal or greater priority is shown.
    */
   RETRO_MESSAGE_TYPE_STATUS,

   /**
    * Denotes a message that reports the progress
    * of a long-running asynchronous task,
    * such as when a core loads large files from disk or the network.
    *
    * The frontend should display messages of this type as a progress bar
    * (or a progress spinner for indefinite tasks),
    * where \c retro_message_ext::msg is the progress bar's title
    * and \c retro_message_ext::progress sets the progress bar's length.
    *
    * This message type shouldn't be used for tasks that are expected to complete quickly.
    */
   RETRO_MESSAGE_TYPE_PROGRESS
};

/**
 * A core-provided message that the frontend will display to the player.
 *
 * @note The frontend is encouraged store these messages in a queue.
 * However, it should not empty the queue of core-submitted messages upon exit;
 * if a core exits with an error, it may want to use this API
 * to show an error message to the player.
 *
 * The frontend should maintain its own copy of the submitted message
 * and all subobjects, including strings.
 *
 * @see RETRO_ENVIRONMENT_SET_MESSAGE_EXT
 */
struct retro_message_ext
{
   /**
    * The \c NULL-terminated text of a message to show to the player.
    * Must not be \c NULL.
    *
    * @note The frontend must honor newlines in this string
    * when rendering text to \c RETRO_MESSAGE_TARGET_OSD.
    */
   const char *msg;

   /**
    * The duration that \c msg will be displayed on-screen, in milliseconds.
    *
    * Ignored for \c RETRO_MESSAGE_TARGET_LOG.
    */
   unsigned duration;

   /**
    * The relative importance of this message
    * when targeting \c RETRO_MESSAGE_TARGET_OSD.
    * Higher values indicate higher priority.
    *
    * The frontend should use this to prioritize messages
    * when it can't show all active messages at once,
    * or to remove messages from its queue if it's full.
    *
    * The relative display order of messages with the same priority
    * is left to the frontend's discretion,
    * although we suggest breaking ties
    * in favor of the most recently-submitted message.
    *
    * Frontends may handle deprioritized messages at their discretion;
    * such messages may have their \c duration altered,
    * be hidden without being delayed,
    * or even be discarded entirely.
    *
    * @note In the reference frontend (RetroArch),
    * the same priority values are used for frontend-generated notifications,
    * which are typically between 0 and 3 depending upon importance.
    *
    * Ignored for \c RETRO_MESSAGE_TARGET_LOG.
    */
   unsigned priority;

   /**
    * The severity level of this message.
    *
    * The frontend may use this to filter or customize messages
    * depending on the player's preferences.
    * Here are some ideas:
    *
    * @li Use this to prioritize errors and warnings
    *     over higher-ranking info and debug messages.
    * @li Render warnings or errors with extra visual feedback,
    *     e.g. with brighter colors or accompanying sound effects.
    *
    * @see RETRO_ENVIRONMENT_SET_LOG_INTERFACE
    */
   enum retro_log_level level;

   /**
    * The intended destination of this message.
    *
    * @see retro_message_target
    */
   enum retro_message_target target;

   /**
    * The intended semantics of this message.
    *
    * Ignored for \c RETRO_MESSAGE_TARGET_LOG.
    *
    * @see retro_message_type
    */
   enum retro_message_type type;

   /**
    * The progress of an asynchronous task.
    *
    * A value between 0 and 100 (inclusive) indicates the task's percentage,
    * and a value of -1 indicates a task of unknown completion.
    *
    * @note Since message type is a hint, a frontend may ignore progress values.
    * Where relevant, a core should include progress percentage within the message string,
    * such that the message intent remains clear when displayed
    * as a standard frontend-generated notification.
    *
    * Ignored for \c RETRO_MESSAGE_TARGET_LOG and for
    * message types other than \c RETRO_MESSAGE_TYPE_PROGRESS.
    */
   int8_t progress;
};

/** @} */

/* Describes how the libretro implementation maps a libretro input bind
 * to its internal input system through a human readable string.
 * This string can be used to better let a user configure input. */
struct retro_input_descriptor
{
   /* Associates given parameters with a description. */
   unsigned port;
   unsigned device;
   unsigned index;
   unsigned id;

   /* Human readable description for parameters.
    * The pointer must remain valid until
    * retro_unload_game() is called. */
   const char *description;
};

/**
 * Contains basic information about the core.
 *
 * @see retro_get_system_info
 * @warning All pointers are owned by the core
 * and must remain valid throughout its lifetime.
 */
struct retro_system_info
{
   /**
    * Descriptive name of the library.
    *
    * @note Should not contain any version numbers, etc.
    */
   const char *library_name;

   /**
    * Descriptive version of the core.
    */
   const char *library_version;

   /**
    * A pipe-delimited string list of file extensions that this core can load, e.g. "bin|rom|iso".
    * Typically used by a frontend for filtering or core selection.
    */
   const char *valid_extensions;

   /* Libretro cores that need to have direct access to their content
    * files, including cores which use the path of the content files to
    * determine the paths of other files, should set need_fullpath to true.
    *
    * Cores should strive for setting need_fullpath to false,
    * as it allows the frontend to perform patching, etc.
    *
    * If need_fullpath is true and retro_load_game() is called:
    *    - retro_game_info::path is guaranteed to have a valid path
    *    - retro_game_info::data and retro_game_info::size are invalid
    *
    * If need_fullpath is false and retro_load_game() is called:
    *    - retro_game_info::path may be NULL
    *    - retro_game_info::data and retro_game_info::size are guaranteed
    *      to be valid
    *
    * See also:
    *    - RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY
    *    - RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY
    */
   bool        need_fullpath;

   /* If true, the frontend is not allowed to extract any archives before
    * loading the real content.
    * Necessary for certain libretro implementations that load games
    * from zipped archives. */
   bool        block_extract;
};

/* Defines overrides which modify frontend handling of
 * specific content file types.
 * An array of retro_system_content_info_override is
 * passed to RETRO_ENVIRONMENT_SET_CONTENT_INFO_OVERRIDE
 * NOTE: In the following descriptions, references to
 *       retro_load_game() may be replaced with
 *       retro_load_game_special() */
struct retro_system_content_info_override
{
   /* A list of file extensions for which the override
    * should apply, delimited by a 'pipe' character
    * (e.g. "md|sms|gg")
    * Permitted file extensions are limited to those
    * included in retro_system_info::valid_extensions
    * and/or retro_subsystem_rom_info::valid_extensions */
   const char *extensions;

   /* Overrides the need_fullpath value set in
    * retro_system_info and/or retro_subsystem_rom_info.
    * To reiterate:
    *
    * If need_fullpath is true and retro_load_game() is called:
    *    - retro_game_info::path is guaranteed to contain a valid
    *      path to an existent file
    *    - retro_game_info::data and retro_game_info::size are invalid
    *
    * If need_fullpath is false and retro_load_game() is called:
    *    - retro_game_info::path may be NULL
    *    - retro_game_info::data and retro_game_info::size are guaranteed
    *      to be valid
    *
    * In addition:
    *
    * If need_fullpath is true and retro_load_game() is called:
    *    - retro_game_info_ext::full_path is guaranteed to contain a valid
    *      path to an existent file
    *    - retro_game_info_ext::archive_path may be NULL
    *    - retro_game_info_ext::archive_file may be NULL
    *    - retro_game_info_ext::dir is guaranteed to contain a valid path
    *      to the directory in which the content file exists
    *    - retro_game_info_ext::name is guaranteed to contain the
    *      basename of the content file, without extension
    *    - retro_game_info_ext::ext is guaranteed to contain the
    *      extension of the content file in lower case format
    *    - retro_game_info_ext::data and retro_game_info_ext::size
    *      are invalid
    *
    * If need_fullpath is false and retro_load_game() is called:
    *    - If retro_game_info_ext::file_in_archive is false:
    *       - retro_game_info_ext::full_path is guaranteed to contain
    *         a valid path to an existent file
    *       - retro_game_info_ext::archive_path may be NULL
    *       - retro_game_info_ext::archive_file may be NULL
    *       - retro_game_info_ext::dir is guaranteed to contain a
    *         valid path to the directory in which the content file exists
    *       - retro_game_info_ext::name is guaranteed to contain the
    *         basename of the content file, without extension
    *       - retro_game_info_ext::ext is guaranteed to contain the
    *         extension of the content file in lower case format
    *    - If retro_game_info_ext::file_in_archive is true:
    *       - retro_game_info_ext::full_path may be NULL
    *       - retro_game_info_ext::archive_path is guaranteed to
    *         contain a valid path to an existent compressed file
    *         inside which the content file is located
    *       - retro_game_info_ext::archive_file is guaranteed to
    *         contain a valid path to an existent content file
    *         inside the compressed file referred to by
    *         retro_game_info_ext::archive_path
    *            e.g. for a compressed file '/path/to/foo.zip'
    *            containing 'bar.sfc'
    *             > retro_game_info_ext::archive_path will be '/path/to/foo.zip'
    *             > retro_game_info_ext::archive_file will be 'bar.sfc'
    *       - retro_game_info_ext::dir is guaranteed to contain a
    *         valid path to the directory in which the compressed file
    *         (containing the content file) exists
    *       - retro_game_info_ext::name is guaranteed to contain
    *         EITHER
    *         1) the basename of the compressed file (containing
    *            the content file), without extension
    *         OR
    *         2) the basename of the content file inside the
    *            compressed file, without extension
    *         In either case, a core should consider 'name' to
    *         be the canonical name/ID of the the content file
    *       - retro_game_info_ext::ext is guaranteed to contain the
    *         extension of the content file inside the compressed file,
    *         in lower case format
    *    - retro_game_info_ext::data and retro_game_info_ext::size are
    *      guaranteed to be valid */
   bool need_fullpath;

   /* If need_fullpath is false, specifies whether the content
    * data buffer available in retro_load_game() is 'persistent'
    *
    * If persistent_data is false and retro_load_game() is called:
    *    - retro_game_info::data and retro_game_info::size
    *      are valid only until retro_load_game() returns
    *    - retro_game_info_ext::data and retro_game_info_ext::size
    *      are valid only until retro_load_game() returns
    *
    * If persistent_data is true and retro_load_game() is called:
    *    - retro_game_info::data and retro_game_info::size
    *      are valid until retro_deinit() returns
    *    - retro_game_info_ext::data and retro_game_info_ext::size
    *      are valid until retro_deinit() returns */
   bool persistent_data;
};

/* Similar to retro_game_info, but provides extended
 * information about the source content file and
 * game memory buffer status.
 * And array of retro_game_info_ext is returned by
 * RETRO_ENVIRONMENT_GET_GAME_INFO_EXT
 * NOTE: In the following descriptions, references to
 *       retro_load_game() may be replaced with
 *       retro_load_game_special() */
struct retro_game_info_ext
{
   /* - If file_in_archive is false, contains a valid
    *   path to an existent content file (UTF-8 encoded)
    * - If file_in_archive is true, may be NULL */
   const char *full_path;

   /* - If file_in_archive is false, may be NULL
    * - If file_in_archive is true, contains a valid path
    *   to an existent compressed file inside which the
    *   content file is located (UTF-8 encoded) */
   const char *archive_path;

   /* - If file_in_archive is false, may be NULL
    * - If file_in_archive is true, contain a valid path
    *   to an existent content file inside the compressed
    *   file referred to by archive_path (UTF-8 encoded)
    *      e.g. for a compressed file '/path/to/foo.zip'
    *      containing 'bar.sfc'
    *      > archive_path will be '/path/to/foo.zip'
    *      > archive_file will be 'bar.sfc' */
   const char *archive_file;

   /* - If file_in_archive is false, contains a valid path
    *   to the directory in which the content file exists
    *   (UTF-8 encoded)
    * - If file_in_archive is true, contains a valid path
    *   to the directory in which the compressed file
    *   (containing the content file) exists (UTF-8 encoded) */
   const char *dir;

   /* Contains the canonical name/ID of the content file
    * (UTF-8 encoded). Intended for use when identifying
    * 'complementary' content named after the loaded file -
    * i.e. companion data of a different format (a CD image
    * required by a ROM), texture packs, internally handled
    * save files, etc.
    * - If file_in_archive is false, contains the basename
    *   of the content file, without extension
    * - If file_in_archive is true, then string is
    *   implementation specific. A frontend may choose to
    *   set a name value of:
    *   EITHER
    *   1) the basename of the compressed file (containing
    *      the content file), without extension
    *   OR
    *   2) the basename of the content file inside the
    *      compressed file, without extension
    *   RetroArch sets the 'name' value according to (1).
    *   A frontend that supports routine loading of
    *   content from archives containing multiple unrelated
    *   content files may set the 'name' value according
    *   to (2). */
   const char *name;

   /* - If file_in_archive is false, contains the extension
    *   of the content file in lower case format
    * - If file_in_archive is true, contains the extension
    *   of the content file inside the compressed file,
    *   in lower case format */
   const char *ext;

   /* String of implementation specific meta-data. */
   const char *meta;

   /* Memory buffer of loaded game content. Will be NULL:
    * IF
    * - retro_system_info::need_fullpath is true and
    *   retro_system_content_info_override::need_fullpath
    *   is unset
    * OR
    * - retro_system_content_info_override::need_fullpath
    *   is true */
   const void *data;

   /* Size of game content memory buffer, in bytes */
   size_t size;

   /* True if loaded content file is inside a compressed
    * archive */
   bool file_in_archive;

   /* - If data is NULL, value is unset/ignored
    * - If data is non-NULL:
    *   - If persistent_data is false, data and size are
    *     valid only until retro_load_game() returns
    *   - If persistent_data is true, data and size are
    *     are valid until retro_deinit() returns */
   bool persistent_data;
};

/**
 * Parameters describing the size and shape of the video frame.
 * @see retro_system_av_info
 * @see RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO
 * @see RETRO_ENVIRONMENT_SET_GEOMETRY
 * @see retro_get_system_av_info
 */
struct retro_game_geometry
{
   /**
    * Nominal video width of game, in pixels.
    * This will typically be the emulated platform's native video width
    * (or its smallest, if the original hardware supports multiple resolutions).
    */
   unsigned base_width;

   /**
    * Nominal video height of game, in pixels.
    * This will typically be the emulated platform's native video height
    * (or its smallest, if the original hardware supports multiple resolutions).
    */
   unsigned base_height;

   /**
    * Maximum possible width of the game screen, in pixels.
    * This will typically be the emulated platform's maximum video width.
    * For cores that emulate platforms with multiple screens (such as the Nintendo DS),
    * this should assume the core's widest possible screen layout (e.g. side-by-side).
    * For cores that support upscaling the resolution,
    * this should assume the highest supported scale factor is active.
    */
   unsigned max_width;

   /**
    * Maximum possible height of the game screen, in pixels.
    * This will typically be the emulated platform's maximum video height.
    * For cores that emulate platforms with multiple screens (such as the Nintendo DS),
    * this should assume the core's tallest possible screen layout (e.g. vertical).
    * For cores that support upscaling the resolution,
    * this should assume the highest supported scale factor is active.
    */
   unsigned max_height;    /* Maximum possible height of game. */

   /**
    * Nominal aspect ratio of game.
    * If zero or less,
    * an aspect ratio of <tt>base_width / base_height</tt> is assumed.
    *
    * @note A frontend may ignore this setting.
    */
   float    aspect_ratio;
};

/**
 * Parameters describing the timing of the video and audio.
 * @see retro_system_av_info
 * @see RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO
 * @see retro_get_system_av_info
 */
struct retro_system_timing
{
   /** Video output refresh rate, in frames per second. */
   double fps;

   /** The audio output sample rate, in Hz. */
   double sample_rate;
};

/**
 * Configures how the core's audio and video should be updated.
 * @see RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO
 * @see retro_get_system_av_info
 */
struct retro_system_av_info
{
   /** Parameters describing the size and shape of the video frame. */
   struct retro_game_geometry geometry;

   /** Parameters describing the timing of the video and audio. */
   struct retro_system_timing timing;
};

/** @defgroup SET_CORE_OPTIONS Core Options
 *  @{
 */

/**
 * Represents \ref RETRO_ENVIRONMENT_GET_VARIABLE "a core option query".
 *
 * @note In \ref RETRO_ENVIRONMENT_SET_VARIABLES
 * (which is a deprecated API),
 * this \c struct serves as an option definition.
 *
 * @see RETRO_ENVIRONMENT_GET_VARIABLE
 */
struct retro_variable
{
   /**
    * A unique key identifying this option.
    *
    * Should be a key for an option that was previously defined
    * with \ref RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2 or similar.
    *
    * Should be prefixed with the core's name
    * to minimize the risk of collisions with another core's options,
    * as frontends are not required to use a namespacing scheme for storing options.
    * For example, a core named "foo" might define an option named "foo_option".
    *
    * @note In \ref RETRO_ENVIRONMENT_SET_VARIABLES
    * (which is a deprecated API),
    * this field is used to define an option
    * named by this key.
    */
   const char *key;

   /**
    * Value to be obtained.
    *
    * Set by the frontend to \c NULL if
    * the option named by \ref key does not exist.
    *
    * @note In \ref RETRO_ENVIRONMENT_SET_VARIABLES
    * (which is a deprecated API),
    * this field is set by the core to define the possible values
    * for an option named by \ref key.
    * When used this way, it must be formatted as follows:
    * @li The text before the first ';' is the option's human-readable title.
    * @li A single space follows the ';'.
    * @li The rest of the string is a '|'-delimited list of possible values,
    * with the first one being the default.
    */
   const char *value;
};

/**
 * An argument that's used to show or hide a core option in the frontend.
 *
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY
 */
struct retro_core_option_display
{
   /**
    * The key for a core option that was defined with \ref RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2,
    * \ref RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL,
    * or their legacy equivalents.
    */
   const char *key;

   /**
    * Whether the option named by \c key
    * should be displayed to the player in the frontend's core options menu.
    *
    * @note This value is a hint, \em not a requirement;
    * the frontend is free to ignore this field.
    */
   bool visible;
};

/**
 * The maximum number of choices that can be defined for a given core option.
 *
 * This limit was chosen as a compromise between
 * a core's flexibility and a streamlined user experience.
 *
 * @note A guiding principle of libretro's API design is that
 * all common interactions (gameplay, menu navigation, etc.)
 * should be possible without a keyboard.
 *
 * If you need more than 128 choices for a core option,
 * consider simplifying your option structure.
 * Here are some ideas:
 *
 * \li If a core option represents a numeric value,
 *     consider reducing the option's granularity
 *     (e.g. define time limits in increments of 5 seconds instead of 1 second).
 *     Providing a fixed set of values based on experimentation
 *     is also a good idea.
 * \li If a core option represents a dynamically-built list of files,
 *     consider leaving out files that won't be useful.
 *     For example, if a core allows the player to choose a specific BIOS file,
 *     it can omit files of the wrong length or without a valid header.
 *
 * @see retro_core_option_definition
 * @see retro_core_option_v2_definition
 */
#define RETRO_NUM_CORE_OPTION_VALUES_MAX 128

/**
 * A descriptor for a particular choice within a core option.
 *
 * @note All option values are represented as strings.
 * If you need to represent any other type,
 * parse the string in \ref value.
 *
 * @see retro_core_option_v2_category
 */
struct retro_core_option_value
{
   /**
    * The option value that the frontend will serialize.
    *
    * Must not be \c NULL or empty.
    * No other hard limits are placed on this value's contents,
    * but here are some suggestions:
    *
    * \li If the value represents a number,
    *     don't include any non-digit characters (units, separators, etc.).
    *     Instead, include that information in \c label.
    *     This will simplify parsing.
    * \li If the value represents a file path,
    *     store it as a relative path with respect to one of the common libretro directories
    *     (e.g. \ref RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY "the system directory"
    *     or \ref RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY "the save directory"),
    *     and use forward slashes (\c "/") as directory separators.
    *     This will simplify cloud storage if supported by the frontend,
    *     as the same file may be used on multiple devices.
    */
   const char *value;

   /**
    * Human-readable name for \c value that the frontend should show to players.
    *
    * May be \c NULL, in which case the frontend
    * should display \c value itself.
    *
    * Here are some guidelines for writing a good label:
    *
    * \li Make the option labels obvious
    *     so that they don't need to be explained in the description.
    * \li Keep labels short, and don't use unnecessary words.
    *     For example, "OpenGL" is a better label than "OpenGL Mode".
    * \li If the option represents a number,
    *     consider adding units, separators, or other punctuation
    *     into the label itself.
    *     For example, "5 seconds" is a better label than "5".
    * \li If the option represents a number, use intuitive units
    *     that don't take a lot of digits to express.
    *     For example, prefer "1 minute" over "60 seconds" or "60,000 milliseconds".
    */
   const char *label;
};

/**
 * @copybrief retro_core_option_v2_definition
 *
 * @deprecated Use \ref retro_core_option_v2_definition instead,
 * as it supports categorizing options into groups.
 * Only use this \c struct to support older frontends or cores.
 *
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL
 */
struct retro_core_option_definition
{
   /** @copydoc retro_core_option_v2_definition::key */
   const char *key;

   /** @copydoc retro_core_option_v2_definition::desc */
   const char *desc;

   /** @copydoc retro_core_option_v2_definition::info */
   const char *info;

   /** @copydoc retro_core_option_v2_definition::values */
   struct retro_core_option_value values[RETRO_NUM_CORE_OPTION_VALUES_MAX];

   /** @copydoc retro_core_option_v2_definition::default_value */
   const char *default_value;
};

#ifdef __PS3__
#undef local
#endif

/**
 * A variant of \ref retro_core_options that supports internationalization.
 *
 * @deprecated Use \ref retro_core_options_v2_intl instead,
 * as it supports categorizing options into groups.
 * Only use this \c struct to support older frontends or cores.
 *
 * @see retro_core_options
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_INTL
 * @see RETRO_ENVIRONMENT_GET_LANGUAGE
 * @see retro_language
 */
struct retro_core_options_intl
{
   /** @copydoc retro_core_options_v2_intl::us */
   struct retro_core_option_definition *us;

   /** @copydoc retro_core_options_v2_intl::local */
   struct retro_core_option_definition *local;
};

/**
 * A descriptor for a group of related core options.
 *
 * Here's an example category:
 *
 * @code
 * {
 *     "cpu",
 *     "CPU Emulation",
 *     "Settings for CPU quirks."
 * }
 * @endcode
 *
 * @see retro_core_options_v2
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL
 */
struct retro_core_option_v2_category
{
   /**
    * A string that uniquely identifies this category within the core's options.
    * Any \c retro_core_option_v2_definition whose \c category_key matches this
    * is considered to be within this category.
    * Different cores may use the same category keys,
    * so namespacing them is not necessary.
    * Valid characters are <tt>[a-zA-Z0-9_-]</tt>.
    *
    * Frontends should use this category to organize core options,
    * but may customize this category's presentation in other ways.
    * For example, a frontend may use common keys like "audio" or "gfx"
    * to select an appropriate icon in its UI.
    *
    * Required; must not be \c NULL.
    */
   const char *key;

   /**
    * A brief human-readable name for this category,
    * intended for the frontend to display to the player.
    * This should be a name that's concise and descriptive, such as "Audio" or "Video".
    *
    * Required; must not be \c NULL.
    */
   const char *desc;

   /**
    * A human-readable description for this category,
    * intended for the frontend to display to the player
    * as secondary help text (e.g. a sublabel or a tooltip).
    * Optional; may be \c NULL or an empty string.
    */
   const char *info;
};

/**
 * A descriptor for a particular core option and the values it may take.
 *
 * Supports categorizing options into groups,
 * so as not to overwhelm the player.
 *
 * @see retro_core_option_v2_category
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL
 */
struct retro_core_option_v2_definition
{
   /**
    * A unique identifier for this option that cores may use
    * \ref RETRO_ENVIRONMENT_GET_VARIABLE "to query its value from the frontend".
    * Must be unique within this core.
    *
    * Should be unique globally;
    * the recommended method for doing so
    * is to prefix each option with the core's name.
    * For example, an option that controls the resolution for a core named "foo"
    * should be named \c "foo_resolution".
    *
    * Valid key characters are in the set <tt>[a-zA-Z0-9_-]</tt>.
    */
   const char *key;

   /**
    * A human-readable name for this option,
    * intended to be displayed by frontends that don't support
    * categorizing core options.
    *
    * Required; must not be \c NULL or empty.
    */
   const char *desc;

   /**
    * A human-readable name for this option,
    * intended to be displayed by frontends that support
    * categorizing core options.
    *
    * This version may be slightly more concise than \ref desc,
    * as it can rely on the structure of the options menu.
    * For example, "Interface" is a good \c desc_categorized,
    * as it can be displayed as a sublabel for a "Network" category.
    * For \c desc, "Network Interface" would be more suitable.
    *
    * Optional; if this field or \c category_key is empty or \c NULL,
    * \c desc will be used instead.
    */
   const char *desc_categorized;

   /**
    * A human-readable description of this option and its effects,
    * intended to be displayed by frontends that don't support
    * categorizing core options.
    *
    * @details Intended to be displayed as secondary help text,
    * such as a tooltip or a sublabel.
    *
    * Here are some suggestions for writing a good description:
    *
    * \li Avoid technical jargon unless this option is meant for advanced users.
    *     If unavoidable, suggest one of the default options for those unsure.
    * \li Don't repeat the option name in the description;
    *     instead, describe what the option name means.
    * \li If an option requires a core restart or game reset to take effect,
    *     be sure to say so.
    * \li Try to make the option labels obvious
    *     so that they don't need to be explained in the description.
    *
    * Optional; may be \c NULL.
    */
   const char *info;

   /**
    * @brief A human-readable description of this option and its effects,
    * intended to be displayed by frontends that support
    * categorizing core options.
    *
    * This version is provided to accommodate descriptions
    * that reference other options by name,
    * as options may have different user-facing names
    * depending on whether the frontend supports categorization.
    *
    * @copydetails info
    *
    * If empty or \c NULL, \c info will be used instead.
    * Will be ignored if \c category_key is empty or \c NULL.
    */
   const char *info_categorized;

   /**
    * The key of the category that this option belongs to.
    *
    * Optional; if equal to \ref retro_core_option_v2_category::key "a defined category",
    * then this option shall be displayed by the frontend
    * next to other options in this same category,
    * assuming it supports doing so.
    * Option categories are intended to be displayed in a submenu,
    * but this isn't a hard requirement.
    *
    * If \c NULL, empty, or not equal to a defined category,
    * then this option is considered uncategorized
    * and the frontend shall display it outside of any category
    * (most likely at a top-level menu).
    *
    * @see retro_core_option_v2_category
    */
   const char *category_key;

   /**
    * One or more possible values for this option,
    * up to the limit of \ref RETRO_NUM_CORE_OPTION_VALUES_MAX.
    *
    * Terminated by a \c { NULL, NULL } element,
    * although frontends should work even if all elements are used.
    */
   struct retro_core_option_value values[RETRO_NUM_CORE_OPTION_VALUES_MAX];

   /**
    * The default value for this core option.
    * Used if it hasn't been set, e.g. for new cores.
    * Must equal one of the \ref value members in the \c values array,
    * or else this option will be ignored.
    */
   const char *default_value;
};

/**
 * A set of core option descriptors and the categories that group them,
 * suitable for enabling a core to be customized.
 *
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2
 */
struct retro_core_options_v2
{
   /**
    * An array of \ref retro_core_option_v2_category "option categories",
    * terminated by a zeroed-out category \c struct.
    *
    * Will be ignored if the frontend doesn't support core option categories.
    *
    * If \c NULL or ignored, all options will be treated as uncategorized.
    * This most likely means that a frontend will display them at a top-level menu
    * without any kind of hierarchy or grouping.
    */
   struct retro_core_option_v2_category *categories;

   /**
    * An array of \ref retro_core_option_v2_definition "core option descriptors",
    * terminated by a zeroed-out definition \c struct.
    *
    * Required; must not be \c NULL.
    */
   struct retro_core_option_v2_definition *definitions;
};

/**
 * A variant of \ref retro_core_options_v2 that supports internationalization.
 *
 * @see retro_core_options_v2
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL
 * @see RETRO_ENVIRONMENT_GET_LANGUAGE
 * @see retro_language
 */
struct retro_core_options_v2_intl
{
   /**
    * Pointer to a core options set
    * whose text is written in American English.
    *
    * This may be passed to \c RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2 as-is
    * if not using \c RETRO_ENVIRONMENT_SET_CORE_OPTIONS_V2_INTL.
    *
    * Required; must not be \c NULL.
    */
   struct retro_core_options_v2 *us;

   /**
    * Pointer to a core options set
    * whose text is written in one of libretro's \ref retro_language "supported languages",
    * most likely the one returned by \ref RETRO_ENVIRONMENT_GET_LANGUAGE.
    *
    * Structure is the same, but usage is slightly different:
    *
    * \li All text (except for keys and option values)
    *     should be written in whichever language
    *     is returned by \c RETRO_ENVIRONMENT_GET_LANGUAGE.
    * \li All fields besides keys and option values may be \c NULL,
    *     in which case the corresponding string in \c us
    *     is used instead.
    * \li All \ref retro_core_option_v2_definition::default_value "default option values"
    *     are taken from \c us.
    *     The defaults in this field are ignored.
    *
    * May be \c NULL, in which case \c us is used instead.
    */
   struct retro_core_options_v2 *local;
};

/**
 * Called by the frontend to determine if any core option's visibility has changed.
 *
 * Each time a frontend sets a core option,
 * it should call this function to see if
 * any core option should be made visible or invisible.
 *
 * May also be called after \ref retro_load_game "loading a game",
 * to determine what the initial visibility of each option should be.
 *
 * Within this function, the core must update the visibility
 * of any dynamically-hidden options
 * using \ref RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY.
 *
 * @note All core options are visible by default,
 * even during this function's first call.
 *
 * @return \c true if any core option's visibility was adjusted
 * since the last call to this function.
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY
 * @see retro_core_option_display
 */
typedef bool (RETRO_CALLCONV *retro_core_options_update_display_callback_t)(void);

/**
 * Callback registered by the core for the frontend to use
 * when setting the visibility of each core option.
 *
 * @see RETRO_ENVIRONMENT_SET_CORE_OPTIONS_DISPLAY
 * @see retro_core_option_display
 */
struct retro_core_options_update_display_callback
{
   /**
    * @copydoc retro_core_options_update_display_callback_t
    *
    * Set by the core.
    */
   retro_core_options_update_display_callback_t callback;
};

/** @} */

struct retro_game_info
{
   const char *path;       /* Path to game, UTF-8 encoded.
                            * Sometimes used as a reference for building other paths.
                            * May be NULL if game was loaded from stdin or similar,
                            * but in this case some cores will be unable to load `data`.
                            * So, it is preferable to fabricate something here instead
                            * of passing NULL, which will help more cores to succeed.
                            * retro_system_info::need_fullpath requires
                            * that this path is valid. */
   const void *data;       /* Memory buffer of loaded game. Will be NULL
                            * if need_fullpath was set. */
   size_t      size;       /* Size of memory buffer. */
   const char *meta;       /* String of implementation specific meta-data. */
};

/** @defgroup GET_CURRENT_SOFTWARE_FRAMEBUFFER Frontend-Owned Framebuffers
 * @{
 */

/** @defgroup RETRO_MEMORY_ACCESS Framebuffer Memory Access Types
 * @{
 */

/** Indicates that core will write to the framebuffer returned by the frontend. */
#define RETRO_MEMORY_ACCESS_WRITE (1 << 0)

/** Indicates that the core will read from the framebuffer returned by the frontend. */
#define RETRO_MEMORY_ACCESS_READ (1 << 1)

/** @} */

/** @defgroup RETRO_MEMORY_TYPE Framebuffer Memory Types
 * @{
 */

/**
 * Indicates that the returned framebuffer's memory is cached.
 * If not set, random access to the buffer may be very slow.
 */
#define RETRO_MEMORY_TYPE_CACHED (1 << 0)

/** @} */

/**
 * A frame buffer owned by the frontend that a core may use for rendering.
 *
 * @see GET_CURRENT_SOFTWARE_FRAMEBUFFER
 * @see retro_video_refresh_t
 */
struct retro_framebuffer
{
   /**
    * Pointer to the beginning of the framebuffer provided by the frontend.
    * The initial contents of this buffer are unspecified,
    * as is the means used to map the memory;
    * this may be defined in software,
    * or it may be GPU memory mapped to RAM.
    *
    * If the framebuffer is used,
    * this pointer must be passed to \c retro_video_refresh_t as-is.
    * It is undefined behavior to pass an offset to this pointer.
    *
    * @warning This pointer is only guaranteed to be valid
    * for the duration of the same \c retro_run iteration
    * \ref GET_CURRENT_SOFTWARE_FRAMEBUFFER "that requested the framebuffer".
    * Reuse of this pointer is undefined.
    */
   void *data;

   /**
    * The width of the framebuffer given in \c data, in pixels.
    * Set by the core.
    *
    * @warning If the framebuffer is used,
    * this value must be passed to \c retro_video_refresh_t as-is.
    * It is undefined behavior to try to render \c data with any other width.
    */
   unsigned width;

   /**
    * The height of the framebuffer given in \c data, in pixels.
    * Set by the core.
    *
    * @warning If the framebuffer is used,
    * this value must be passed to \c retro_video_refresh_t as-is.
    * It is undefined behavior to try to render \c data with any other height.
    */
   unsigned height;

   /**
    * The distance between the start of one scanline and the beginning of the next, in bytes.
    * In practice this is usually equal to \c width times the pixel size,
    * but that's not guaranteed.
    * Sometimes called the "stride".
    *
    * @setby{frontend}
    * @warning If the framebuffer is used,
    * this value must be passed to \c retro_video_refresh_t as-is.
    * It is undefined to try to render \c data with any other pitch.
    */
   size_t pitch;

   /**
    * The pixel format of the returned framebuffer.
    * May be different than the format specified by the core in \c RETRO_ENVIRONMENT_SET_PIXEL_FORMAT,
    * e.g. due to conversions.
    * Set by the frontend.
    *
    * @see RETRO_ENVIRONMENT_SET_PIXEL_FORMAT
    */
   enum retro_pixel_format format;

   /**
    * One or more \ref RETRO_MEMORY_ACCESS "memory access flags"
    * that specify how the core will access the memory in \c data.
    *
    * @setby{core}
    */
   unsigned access_flags;

   /**
    * Zero or more \ref RETRO_MEMORY_TYPE "memory type flags"
    * that describe how the framebuffer's memory has been mapped.
    *
    * @setby{frontend}
    */
   unsigned memory_flags;
};

/** @} */

/** @defgroup SET_FASTFORWARDING_OVERRIDE Fast-Forward Override
 * @{
 */

/**
 * Parameters that govern when and how the core takes control
 * of fast-forwarding mode.
 */
struct retro_fastforwarding_override
{
   /**
    * The factor by which the core will be sped up
    * when \c fastforward is \c true.
    * This value is used as follows:
    *
    * @li A value greater than 1.0 will run the core at
    *     the specified multiple of normal speed.
    *     For example, a value of 5.0
    *     combined with a normal target rate of 60 FPS
    *     will result in a target rate of 300 FPS.
    *     The actual rate may be lower if the host's hardware can't keep up.
    * @li A value of 1.0 will run the core at normal speed.
    * @li A value between 0.0 (inclusive) and 1.0 (exclusive)
    *     will run the core as fast as the host system can manage.
    * @li A negative value will let the frontend choose a factor.
    * @li An infinite value or \c NaN results in undefined behavior.
    *
    * @attention Setting this value to less than 1.0 will \em not
    * slow down the core.
    */
   float ratio;

   /**
    * If \c true, the frontend should activate fast-forwarding
    * until this field is set to \c false or the core is unloaded.
    */
   bool fastforward;

   /**
    * If \c true, the frontend should display an on-screen notification or icon
    * while \c fastforward is \c true (where supported).
    * Otherwise, the frontend should not display any such notification.
    */
   bool notification;

   /**
    * If \c true, the core has exclusive control
    * over enabling and disabling fast-forwarding
    * via the \c fastforward field.
    * The frontend will not be able to start or stop fast-forwarding
    * until this field is set to \c false or the core is unloaded.
    */
   bool inhibit_toggle;
};

/** @} */

/**
 * During normal operation.
 *
 * @note Rate will be equal to the core's internal FPS.
 */
#define RETRO_THROTTLE_NONE              0

/**
 * While paused or stepping single frames.
 *
 * @note Rate will be 0.
 */
#define RETRO_THROTTLE_FRAME_STEPPING    1

/**
 * During fast forwarding.
 *
 * @note Rate will be 0 if not specifically limited to a maximum speed.
 */
#define RETRO_THROTTLE_FAST_FORWARD      2

/**
 * During slow motion.
 *
 * @note Rate will be less than the core's internal FPS.
 */
#define RETRO_THROTTLE_SLOW_MOTION       3

/**
 * While rewinding recorded save states.
 *
 * @note Rate can vary depending on the rewind speed or be 0 if the frontend
 * is not aiming for a specific rate.
 */
#define RETRO_THROTTLE_REWINDING         4

/**
 * While vsync is active in the video driver, and the target refresh rate is lower than the core's internal FPS.
 *
 * @note Rate is the target refresh rate.
 */
#define RETRO_THROTTLE_VSYNC             5

/**
 * When the frontend does not throttle in any way.
 *
 * @note Rate will be 0. An example could be if no vsync or audio output is active.
 */
#define RETRO_THROTTLE_UNBLOCKED         6

/**
 * Details about the actual rate an implementation is calling \c retro_run() at.
 *
 * @see RETRO_ENVIRONMENT_GET_THROTTLE_STATE
 */
struct retro_throttle_state
{
   /**
    * The current throttling mode.
    *
    * @note Should be one of the \c RETRO_THROTTLE_* values.
    * @see RETRO_THROTTLE_NONE
    * @see RETRO_THROTTLE_FRAME_STEPPING
    * @see RETRO_THROTTLE_FAST_FORWARD
    * @see RETRO_THROTTLE_SLOW_MOTION
    * @see RETRO_THROTTLE_REWINDING
    * @see RETRO_THROTTLE_VSYNC
    * @see RETRO_THROTTLE_UNBLOCKED
    */
   unsigned mode;

   /**
    * How many times per second the frontend aims to call retro_run.
    *
    * @note Depending on the mode, it can be 0 if there is no known fixed rate.
    * This won't be accurate if the total processing time of the core and
    * the frontend is longer than what is available for one frame.
    */
   float rate;
};

/** @defgroup GET_MICROPHONE_INTERFACE Microphone Interface
 * @{
 */

/**
 * Opaque handle to a microphone that's been opened for use.
 * The underlying object is accessed or created with \c retro_microphone_interface_t.
 */
typedef struct retro_microphone retro_microphone_t;

/**
 * Parameters for configuring a microphone.
 * Some of these might not be honored,
 * depending on the available hardware and driver configuration.
 */
typedef struct retro_microphone_params
{
   /**
    * The desired sample rate of the microphone's input, in Hz.
    * The microphone's input will be resampled,
    * so cores can ask for whichever frequency they need.
    *
    * If zero, some reasonable default will be provided by the frontend
    * (usually from its config file).
    *
    * @see retro_get_mic_rate_t
    */
   unsigned rate;
} retro_microphone_params_t;

/**
 * @copydoc retro_microphone_interface::open_mic
 */
typedef retro_microphone_t *(RETRO_CALLCONV *retro_open_mic_t)(const retro_microphone_params_t *params);

/**
 * @copydoc retro_microphone_interface::close_mic
 */
typedef void (RETRO_CALLCONV *retro_close_mic_t)(retro_microphone_t *microphone);

/**
 * @copydoc retro_microphone_interface::get_params
 */
typedef bool (RETRO_CALLCONV *retro_get_mic_params_t)(const retro_microphone_t *microphone, retro_microphone_params_t *params);

/**
 * @copydoc retro_microphone_interface::set_mic_state
 */
typedef bool (RETRO_CALLCONV *retro_set_mic_state_t)(retro_microphone_t *microphone, bool state);

/**
 * @copydoc retro_microphone_interface::get_mic_state
 */
typedef bool (RETRO_CALLCONV *retro_get_mic_state_t)(const retro_microphone_t *microphone);

/**
 * @copydoc retro_microphone_interface::read_mic
 */
typedef int (RETRO_CALLCONV *retro_read_mic_t)(retro_microphone_t *microphone, int16_t* samples, size_t num_samples);

/**
 * The current version of the microphone interface.
 * Will be incremented whenever \c retro_microphone_interface or \c retro_microphone_params_t
 * receive new fields.
 *
 * Frontends using cores built against older mic interface versions
 * should not access fields introduced in newer versions.
 */
#define RETRO_MICROPHONE_INTERFACE_VERSION 1

/**
 * An interface for querying the microphone and accessing data read from it.
 *
 * @see RETRO_ENVIRONMENT_GET_MICROPHONE_INTERFACE
 */
struct retro_microphone_interface
{
   /**
    * The version of this microphone interface.
    * Set by the core to request a particular version,
    * and set by the frontend to indicate the returned version.
    * 0 indicates that the interface is invalid or uninitialized.
    */
   unsigned interface_version;

   /**
    * Initializes a new microphone.
    * Assuming that microphone support is enabled and provided by the frontend,
    * cores may call this function whenever necessary.
    * A microphone could be opened throughout a core's lifetime,
    * or it could wait until a microphone is plugged in to the emulated device.
    *
    * The returned handle will be valid until it's freed,
    * even if the audio driver is reinitialized.
    *
    * This function is not guaranteed to be thread-safe.
    *
    * @param[in] args Parameters used to create the microphone.
    * May be \c NULL, in which case the default value of each parameter will be used.
    *
    * @returns Pointer to the newly-opened microphone,
    * or \c NULL if one couldn't be opened.
    * This likely means that no microphone is plugged in and recognized,
    * or the maximum number of supported microphones has been reached.
    *
    * @note Microphones are \em inactive by default;
    * to begin capturing audio, call \c set_mic_state.
    * @see retro_microphone_params_t
    */
   retro_open_mic_t open_mic;

   /**
    * Closes a microphone that was initialized with \c open_mic.
    * Calling this function will stop all microphone activity
    * and free up the resources that it allocated.
    * Afterwards, the handle is invalid and must not be used.
    *
    * A frontend may close opened microphones when unloading content,
    * but this behavior is not guaranteed.
    * Cores should close their microphones when exiting, just to be safe.
    *
    * @param microphone Pointer to the microphone that was allocated by \c open_mic.
    * If \c NULL, this function does nothing.
    *
    * @note The handle might be reused if another microphone is opened later.
    */
   retro_close_mic_t close_mic;

   /**
    * Returns the configured parameters of this microphone.
    * These may differ from what was requested depending on
    * the driver and device configuration.
    *
    * Cores should check these values before they start fetching samples.
    *
    * Will not change after the mic was opened.
    *
    * @param[in] microphone Opaque handle to the microphone
    * whose parameters will be retrieved.
    * @param[out] params The parameters object that the
    * microphone's parameters will be copied to.
    *
    * @return \c true if the parameters were retrieved,
    * \c false if there was an error.
    */
   retro_get_mic_params_t get_params;

   /**
    * Enables or disables the given microphone.
    * Microphones are disabled by default
    * and must be explicitly enabled before they can be used.
    * Disabled microphones will not process incoming audio samples,
    * and will therefore have minimal impact on overall performance.
    * Cores may enable microphones throughout their lifetime,
    * or only for periods where they're needed.
    *
    * Cores that accept microphone input should be able to operate without it;
    * we suggest substituting silence in this case.
    *
    * @param microphone Opaque handle to the microphone
    * whose state will be adjusted.
    * This will have been provided by \c open_mic.
    * @param state \c true if the microphone should receive audio input,
    * \c false if it should be idle.
    * @returns \c true if the microphone's state was successfully set,
    * \c false if \c microphone is invalid
    * or if there was an error.
    */
   retro_set_mic_state_t set_mic_state;

   /**
    * Queries the active state of a microphone at the given index.
    * Will return whether the microphone is enabled,
    * even if the driver is paused.
    *
    * @param microphone Opaque handle to the microphone
    * whose state will be queried.
    * @return \c true if the provided \c microphone is valid and active,
    * \c false if not or if there was an error.
    */
   retro_get_mic_state_t get_mic_state;

   /**
    * Retrieves the input processed by the microphone since the last call.
    * \em Must be called every frame unless \c microphone is disabled,
    * similar to how \c retro_audio_sample_batch_t works.
    *
    * @param[in] microphone Opaque handle to the microphone
    * whose recent input will be retrieved.
    * @param[out] samples The buffer that will be used to store the microphone's data.
    * Microphone input is in mono (i.e. one number per sample).
    * Should be large enough to accommodate the expected number of samples per frame;
    * for example, a 44.1kHz sample rate at 60 FPS would require space for 735 samples.
    * @param[in] num_samples The size of the data buffer in samples (\em not bytes).
    * Microphone input is in mono, so a "frame" and a "sample" are equivalent in length here.
    *
    * @return The number of samples that were copied into \c samples.
    * If \c microphone is pending driver initialization,
    * this function will copy silence of the requested length into \c samples.
    *
    * Will return -1 if the microphone is disabled,
    * the audio driver is paused,
    * or there was an error.
    */
   retro_read_mic_t read_mic;
};

/** @} */

/** @defgroup GET_DEVICE_POWER Device Power
 * @{
 */

/**
 * Describes how a device is being powered.
 * @see RETRO_ENVIRONMENT_GET_DEVICE_POWER
 */
enum retro_power_state
{
   /**
    * Indicates that the frontend cannot report its power state at this time,
    * most likely due to a lack of support.
    *
    * \c RETRO_ENVIRONMENT_GET_DEVICE_POWER will not return this value;
    * instead, the environment callback will return \c false.
    */
   RETRO_POWERSTATE_UNKNOWN = 0,

   /**
    * Indicates that the device is running on its battery.
    * Usually applies to portable devices such as handhelds, laptops, and smartphones.
    */
   RETRO_POWERSTATE_DISCHARGING,

   /**
    * Indicates that the device's battery is currently charging.
    */
   RETRO_POWERSTATE_CHARGING,

   /**
    * Indicates that the device is connected to a power source
    * and that its battery has finished charging.
    */
   RETRO_POWERSTATE_CHARGED,

   /**
    * Indicates that the device is connected to a power source
    * and that it does not have a battery.
    * This usually suggests a desktop computer or a non-portable game console.
    */
   RETRO_POWERSTATE_PLUGGED_IN
};

/**
 * Indicates that an estimate is not available for the battery level or time remaining,
 * even if the actual power state is known.
 */
#define RETRO_POWERSTATE_NO_ESTIMATE (-1)

/**
 * Describes the power state of the device running the frontend.
 * @see RETRO_ENVIRONMENT_GET_DEVICE_POWER
 */
struct retro_device_power
{
   /**
    * The current state of the frontend's power usage.
    */
   enum retro_power_state state;

   /**
    * A rough estimate of the amount of time remaining (in seconds)
    * before the device powers off.
    * This value depends on a variety of factors,
    * so it is not guaranteed to be accurate.
    *
    * Will be set to \c RETRO_POWERSTATE_NO_ESTIMATE if \c state does not equal \c RETRO_POWERSTATE_DISCHARGING.
    * May still be set to \c RETRO_POWERSTATE_NO_ESTIMATE if the frontend is unable to provide an estimate.
    */
   int seconds;

   /**
    * The approximate percentage of battery charge,
    * ranging from 0 to 100 (inclusive).
    * The device may power off before this reaches 0.
    *
    * The user might have configured their device
    * to stop charging before the battery is full,
    * so do not assume that this will be 100 in the \c RETRO_POWERSTATE_CHARGED state.
    */
   int8_t percent;
};

/** @} */

/** @defgroup Executable Memory Modes
 * Describes how the frontend provisions executable memory.
 * @{
 */

#define RETRO_EXEC_MEM_MODE_UNAVAILABLE  0 /**< No executable memory available */
#define RETRO_EXEC_MEM_MODE_UNRESTRICTED 1 /**< No restrictions; core should self-allocate */
#define RETRO_EXEC_MEM_MODE_RWX          2 /**< Single mapping, read-write-execute */
#define RETRO_EXEC_MEM_MODE_WX_TOGGLE    3 /**< Single mapping, write XOR execute (core toggles) */
#define RETRO_EXEC_MEM_MODE_DUAL_MAP     4 /**< Separate R-X and R-W mappings of same pages */

/**
 * Parameters for \c RETRO_ENVIRONMENT_EXEC_MEM_ALLOC.
 *
 * The core fills in \c version and \c size before calling.
 * The frontend fills in \c mode, \c rx, and \c rw on success.
 * @see RETRO_ENVIRONMENT_EXEC_MEM_ALLOC
 */
struct retro_exec_mem_alloc
{
   unsigned version;  /**< Set by core (currently 1). */
   size_t   size;     /**< Set by core: requested bytes.  */
   unsigned mode;     /**< Set by frontend: one of \c RETRO_EXEC_MEM_MODE_*. */
   void    *rx;       /**< Set by frontend: execute from this pointer. */
   void    *rw;       /**< Set by frontend: write through this pointer.
                           Equal to \c rx when mode is RWX or WX_TOGGLE. */
};

/**
 * Parameters for \c RETRO_ENVIRONMENT_EXEC_MEM_FREE.
 * @see RETRO_ENVIRONMENT_EXEC_MEM_FREE
 */
struct retro_exec_mem_free
{
   void *rx;  /**< The \c rx pointer returned by a previous alloc call.
                   The matching \c rw pointer is also accepted. */
};

/** @} */

/**
 * @defgroup Callbacks
 * @{
 */

/**
 * Environment callback to give implementations a way of performing uncommon tasks.
 *
 * @note Extensible.
 *
 * @param cmd The command to run.
 * @param data A pointer to the data associated with the command.
 *
 * @return Varies by callback,
 * but will always return \c false if the command is not recognized.
 *
 * @see RETRO_ENVIRONMENT_SET_ROTATION
 * @see retro_set_environment()
 */
typedef bool (RETRO_CALLCONV *retro_environment_t)(unsigned cmd, void *data);

/**
 * Render a frame.
 *
 * @note For performance reasons, it is highly recommended to have a frame
 * that is packed in memory, i.e. pitch == width * byte_per_pixel.
 * Certain graphic APIs, such as OpenGL ES, do not like textures
 * that are not packed in memory.
 *
 * @param data A pointer to the frame buffer data with a pixel format of 15-bit \c 0RGB1555 native endian, unless changed with \c RETRO_ENVIRONMENT_SET_PIXEL_FORMAT.
 * @param width The width of the frame buffer, in pixels.
 * @param height The height frame buffer, in pixels.
 * @param pitch The width of the frame buffer, in bytes.
 *
 * @see retro_set_video_refresh()
 * @see RETRO_ENVIRONMENT_SET_PIXEL_FORMAT
 * @see retro_pixel_format
 */
typedef void (RETRO_CALLCONV *retro_video_refresh_t)(const void *data, unsigned width,
      unsigned height, size_t pitch);

/**
 * Renders a single audio frame. Should only be used if implementation generates a single sample at a time.
 *
 * @param left The left audio sample represented as a signed 16-bit native endian.
 * @param right The right audio sample represented as a signed 16-bit native endian.
 *
 * @see retro_set_audio_sample()
 * @see retro_set_audio_sample_batch()
 */
typedef void (RETRO_CALLCONV *retro_audio_sample_t)(int16_t left, int16_t right);

/**
 * Renders multiple audio frames in one go.
 *
 * @note Only one of the audio callbacks must ever be used.
 *
 * @param data A pointer to the audio sample data pairs to render.
 * @param frames The number of frames that are represented in the data. One frame
 *     is defined as a sample of left and right channels, interleaved.
 *     For example: <tt>int16_t buf[4] = { l, r, l, r };</tt> would be 2 frames.
 *
 * @return The number of frames that were processed.
 *
 * @see retro_set_audio_sample_batch()
 * @see retro_set_audio_sample()
 */
typedef size_t (RETRO_CALLCONV *retro_audio_sample_batch_t)(const int16_t *data,
      size_t frames);

/**
 * Renders multiple audio frames in one go, in float format.
 *
 * This is the float counterpart of \c retro_audio_sample_batch_t. It is
 * only valid after the frontend has answered \c true to
 * \c RETRO_ENVIRONMENT_GET_AUDIO_SAMPLE_BATCH_FLOAT, and must not be
 * mixed with the int16 callbacks within the same loaded game.
 *
 * @param data A pointer to interleaved stereo float sample frames,
 *     normalized to the range [-1.0, 1.0]. One frame is a left/right
 *     pair, e.g. <tt>float buf[4] = { l, r, l, r };</tt> is 2 frames.
 * @param frames The number of frames represented in \c data.
 *
 * @return The number of frames that were processed.
 *
 * @see RETRO_ENVIRONMENT_GET_AUDIO_SAMPLE_BATCH_FLOAT
 * @see retro_audio_sample_batch_t
 */
typedef size_t (RETRO_CALLCONV *retro_audio_sample_batch_float_t)(
      const float *data, size_t frames);

/**
 * Float audio sample-batch callback handed to the core in response to
 * \c RETRO_ENVIRONMENT_GET_AUDIO_SAMPLE_BATCH_FLOAT.
 *
 * @see RETRO_ENVIRONMENT_GET_AUDIO_SAMPLE_BATCH_FLOAT
 */
struct retro_audio_sample_float_callback
{
   /* Set by the frontend. The core calls this instead of the int16
    * batch callback once float output has been negotiated. */
   retro_audio_sample_batch_float_t batch;
};

/**
 * Polls input.
 *
 * @see retro_set_input_poll()
 */
typedef void (RETRO_CALLCONV *retro_input_poll_t)(void);

/**
 * Queries for input for player 'port'.
 *
 * @param port Which player 'port' to query.
 * @param device Which device to query for. Will be masked with \c RETRO_DEVICE_MASK.
 * @warning Poll with a base device ID only; passing an ID created via
 * \c RETRO_DEVICE_SUBCLASS is reserved for future definition, and the masking
 * noted above is a frontend convenience that must not be relied upon.
 * @param index The input index to retrieve.
 * The exact semantics depend on the device type given in \c device.
 * @param id The ID of which value to query, like \c RETRO_DEVICE_ID_JOYPAD_B.
 * @returns Depends on the provided arguments,
 * but will return 0 if their values are unsupported
 * by the frontend or the backing physical device.
 * @note Specialization of devices such as \c RETRO_DEVICE_JOYPAD_MULTITAP that
 * have been set with \c retro_set_controller_port_device() will still use the
 * higher level \c RETRO_DEVICE_JOYPAD to request input.
 *
 * @see retro_set_input_state()
 * @see RETRO_DEVICE_NONE
 * @see RETRO_DEVICE_JOYPAD
 * @see RETRO_DEVICE_MOUSE
 * @see RETRO_DEVICE_KEYBOARD
 * @see RETRO_DEVICE_LIGHTGUN
 * @see RETRO_DEVICE_ANALOG
 * @see RETRO_DEVICE_POINTER
 */
typedef int16_t (RETRO_CALLCONV *retro_input_state_t)(unsigned port, unsigned device,
      unsigned index, unsigned id);

/**
 * Sets the environment callback.
 *
 * @param cb The function which is used when making environment calls.
 *
 * @note Guaranteed to be called before \c retro_init().
 *
 * @see RETRO_ENVIRONMENT
 */
RETRO_API void retro_set_environment(retro_environment_t cb);

/**
 * Sets the video refresh callback.
 *
 * @param cb The function which is used when rendering a frame.
 *
 * @note Guaranteed to have been called before the first call to \c retro_run() is made.
 */
RETRO_API void retro_set_video_refresh(retro_video_refresh_t cb);

/**
 * Sets the audio sample callback.
 *
 * @param cb The function which is used when rendering a single audio frame.
 *
 * @note Guaranteed to have been called before the first call to \c retro_run() is made.
 */
RETRO_API void retro_set_audio_sample(retro_audio_sample_t cb);

/**
 * Sets the audio sample batch callback.
 *
 * @param cb The function which is used when rendering multiple audio frames in one go.
 *
 * @note Guaranteed to have been called before the first call to \c retro_run() is made.
 */
RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb);

/**
 * Sets the input poll callback.
 *
 * @param cb The function which is used to poll the active input.
 *
 * @note Guaranteed to have been called before the first call to \c retro_run() is made.
 */
RETRO_API void retro_set_input_poll(retro_input_poll_t cb);

/**
 * Sets the input state callback.
 *
 * @param cb The function which is used to query the input state.
 *
 *@note Guaranteed to have been called before the first call to \c retro_run() is made.
 */
RETRO_API void retro_set_input_state(retro_input_state_t cb);

/**
 * @}
 */

/**
 * Called by the frontend when initializing a libretro core.
 *
 * @warning There are many possible "gotchas" with global state in dynamic libraries.
 * Here are some to keep in mind:
 * <ul>
 * <li>Do not assume that the core was loaded by the operating system
 * for the first time within this call.
 * It may have been statically linked or retained from a previous session.
 * Consequently, cores must not rely on global variables being initialized
 * to their default values before this function is called;
 * this also goes for object constructors in C++.
 * <li>Although C++ requires that constructors be called for global variables,
 * it does not require that their destructors be called
 * if stored within a dynamic library's global scope.
 * <li>If the core is statically linked to the frontend,
 * global variables may be initialized when the frontend itself is initially executed.
 * </ul>
 * @see retro_deinit
 */
RETRO_API void retro_init(void);

/**
 * Called by the frontend when deinitializing a libretro core.
 * The core must release all of its allocated resources before this function returns.
 *
 * @warning There are many possible "gotchas" with global state in dynamic libraries.
 * Here are some to keep in mind:
 * <ul>
 * <li>Do not assume that the operating system will unload the core after this function returns,
 * as the core may be linked statically or retained in memory.
 * Cores should use this function to clean up all allocated resources
 * and reset all global variables to their default states.
 * <li>Do not assume that this core won't be loaded again after this function returns.
 * It may be kept in memory by the frontend for later use,
 * or it may be statically linked.
 * Therefore, all global variables should be reset to their default states within this function.
 * <li>C++ does not require that destructors be called
 * for variables within a dynamic library's global scope.
 * Therefore, global objects that own dynamically-managed resources
 * (such as \c std::string or <tt>std::vector</tt>)
 * should be kept behind pointers that are explicitly deallocated within this function.
 * </ul>
 * @see retro_init
 */
RETRO_API void retro_deinit(void);

/**
 * Retrieves which version of the libretro API is being used.
 *
 * @note This is used to validate ABI compatibility when the API is revised.
 *
 * @return Must return \c RETRO_API_VERSION.
 *
 * @see RETRO_API_VERSION
 */
RETRO_API unsigned retro_api_version(void);

/**
 * Gets statically known system info.
 *
 * @note Can be called at any time, even before retro_init().
 *
 * @param info A pointer to a \c retro_system_info where the info is to be loaded into. This must be statically allocated.
 */
RETRO_API void retro_get_system_info(struct retro_system_info *info);

/**
 * Gets information about system audio/video timings and geometry.
 *
 * @note Can be called only after \c retro_load_game() has successfully completed.
 *
 * @note The implementation of this function might not initialize every variable
 * if needed. For example, \c geom.aspect_ratio might not be initialized if
 * the core doesn't desire a particular aspect ratio.
 *
 * @param info A pointer to a \c retro_system_av_info where the audio/video information should be loaded into.
 *
 * @see retro_system_av_info
 */
RETRO_API void retro_get_system_av_info(struct retro_system_av_info *info);

/**
 * Sets device to be used for player 'port'.
 *
 * By default, \c RETRO_DEVICE_JOYPAD is assumed to be plugged into all
 * available ports.
 *
 * @note Setting a particular device type is not a guarantee that libretro cores
 * will only poll input based on that particular device type. It is only a
 * hint to the libretro core when a core cannot automatically detect the
 * appropriate input device type on its own. It is also relevant when a
 * core can change its behavior depending on device type.
 *
 * @note As part of the core's implementation of retro_set_controller_port_device,
 * the core should call \c RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS to notify the
 * frontend if the descriptions for any controls have changed as a
 * result of changing the device type.
 *
 * @param port Which port to set the device for, usually indicates the player number.
 * @param device Which device the given port is using. By default, \c RETRO_DEVICE_JOYPAD is assumed for all ports.
 *
 * @see RETRO_DEVICE_NONE
 * @see RETRO_DEVICE_JOYPAD
 * @see RETRO_DEVICE_MOUSE
 * @see RETRO_DEVICE_KEYBOARD
 * @see RETRO_DEVICE_LIGHTGUN
 * @see RETRO_DEVICE_ANALOG
 * @see RETRO_DEVICE_POINTER
 * @see RETRO_ENVIRONMENT_SET_CONTROLLER_INFO
 */
RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device);

/**
 * Resets the currently-loaded game.
 * Cores should treat this as a soft reset (i.e. an emulated reset button) if possible,
 * but hard resets are acceptable.
 */
RETRO_API void retro_reset(void);

/**
 * Runs the game for one video frame.
 *
 * During \c retro_run(), the \c retro_input_poll_t callback must be called at least once.
 *
 * @note If a frame is not rendered for reasons where a game "dropped" a frame,
 * this still counts as a frame, and \c retro_run() should explicitly dupe
 * a frame if \c RETRO_ENVIRONMENT_GET_CAN_DUPE returns true. In this case,
 * the video callback can take a NULL argument for data.
 *
 * @see retro_input_poll_t
 */
RETRO_API void retro_run(void);

/**
 * Returns the amount of data the implementation requires to serialize internal state (save states).
 *
 * @note Between calls to \c retro_load_game() and \c retro_unload_game(), the
 * returned size is never allowed to be larger than a previous returned
 * value, to ensure that the frontend can allocate a save state buffer once.
 *
 * @return The amount of data the implementation requires to serialize the internal state.
 *
 * @see retro_serialize()
 */
RETRO_API size_t retro_serialize_size(void);

/**
 * Serializes the internal state.
 *
 * @param data A pointer to where the serialized data should be saved to.
 * @param size The size of the memory.
 *
 * @return If failed, or size is lower than \c retro_serialize_size(), it
 * should return false. On success, it will return true.
 *
 * @see retro_serialize_size()
 * @see retro_unserialize()
 */
RETRO_API bool retro_serialize(void *data, size_t len);

/**
 * Unserialize the given state data, and load it into the internal state.
 *
 * @return Returns true if loading the state was successful, false otherwise.
 *
 * @see retro_serialize()
 */
RETRO_API bool retro_unserialize(const void *data, size_t len);

/**
 * Reset all the active cheats to their default disabled state.
 *
 * @see retro_cheat_set()
 */
RETRO_API void retro_cheat_reset(void);

/**
 * Enable or disable a cheat.
 *
 * @param index The index of the cheat to act upon.
 * @param enabled Whether to enable or disable the cheat.
 * @param code A string of the code used for the cheat.
 *
 * @see retro_cheat_reset()
 */
RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *code);

/**
 * Loads a game.
 *
 * @param game A pointer to a \c retro_game_info detailing information about the game to load.
 * May be \c NULL if the core is loaded without content.
 *
 * @return Will return true when the game was loaded successfully, or false otherwise.
 *
 * @see retro_game_info
 * @see RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME
 */
RETRO_API bool retro_load_game(const struct retro_game_info *game);

/**
 * Called when the frontend has loaded one or more "special" content files,
 * typically through subsystems.
 *
 * @note Only necessary for cores that support subsystems.
 * Others may return \c false or delegate to <tt>retro_load_game</tt>.
 *
 * @param game_type The type of game to load,
 * as determined by \c retro_subsystem_info.
 * @param info A pointer to an array of \c retro_game_info objects
 * providing information about the loaded content.
 * @param num_info The number of \c retro_game_info objects passed into the info parameter.
 * @return \c true if loading is successful, false otherwise.
 * If the core returns \c false,
 * the frontend should abort the core
 * and return to its main menu (if applicable).
 *
 * @see RETRO_ENVIRONMENT_GET_GAME_INFO_EXT
 * @see RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO
 * @see retro_load_game()
 * @see retro_subsystem_info
 */
RETRO_API bool retro_load_game_special(
  unsigned game_type,
  const struct retro_game_info *info, size_t num_info
);

/**
 * Unloads the currently loaded game.
 *
 * @note This is called before \c retro_deinit(void).
 *
 * @see retro_load_game()
 * @see retro_deinit()
 */
RETRO_API void retro_unload_game(void);

/**
 * Gets the region of the actively loaded content as either \c RETRO_REGION_NTSC or \c RETRO_REGION_PAL.
 * @note This refers to the region of the content's intended television standard,
 * not necessarily the region of the content's origin.
 * For emulated consoles that don't use either standard
 * (e.g. handhelds or post-HD platforms),
 * the core should return \c RETRO_REGION_NTSC.
 * @return The region of the actively loaded content.
 *
 * @see RETRO_REGION_NTSC
 * @see RETRO_REGION_PAL
 */
RETRO_API unsigned retro_get_region(void);

/**
 * Get a region of memory.
 *
 * @param id The ID for the memory block that's desired to retrieve. Can be \c RETRO_MEMORY_SAVE_RAM, \c RETRO_MEMORY_RTC, \c RETRO_MEMORY_SYSTEM_RAM, or \c RETRO_MEMORY_VIDEO_RAM.
 *
 * @return A pointer to the desired region of memory, or NULL when not available.
 *
 * @see RETRO_MEMORY_SAVE_RAM
 * @see RETRO_MEMORY_RTC
 * @see RETRO_MEMORY_SYSTEM_RAM
 * @see RETRO_MEMORY_VIDEO_RAM
 */
RETRO_API void *retro_get_memory_data(unsigned id);

/**
 * Gets the size of the given region of memory.
 *
 * @param id The ID for the memory block to check the size of. Can be RETRO_MEMORY_SAVE_RAM, RETRO_MEMORY_RTC, RETRO_MEMORY_SYSTEM_RAM, or RETRO_MEMORY_VIDEO_RAM.
 *
 * @return The size of the region in memory, or 0 when not available.
 *
 * @see RETRO_MEMORY_SAVE_RAM
 * @see RETRO_MEMORY_RTC
 * @see RETRO_MEMORY_SYSTEM_RAM
 * @see RETRO_MEMORY_VIDEO_RAM
 */
RETRO_API size_t retro_get_memory_size(unsigned id);

#ifdef __cplusplus
}
#endif

#endif
