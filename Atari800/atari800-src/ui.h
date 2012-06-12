/*
 * ui.h - Atari user interface definitions
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2008 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef UI_H_
#define UI_H_

#include "config.h"
#include <stdio.h> /* FILENAME_MAX */
#include "atari.h"

/* Three legitimate entries to UI module. */
int UI_SelectCartType(int k);
void UI_Run(void);

extern int UI_is_active;
extern int UI_alt_function;
extern int UI_current_function;

#ifdef CRASH_MENU
extern int UI_crash_code;
extern UWORD UI_crash_address;
extern UWORD UI_crash_afterCIM;
#endif

#define UI_MAX_DIRECTORIES 8

extern char UI_atari_files_dir[UI_MAX_DIRECTORIES][FILENAME_MAX];
extern char UI_saved_files_dir[UI_MAX_DIRECTORIES][FILENAME_MAX];
extern int UI_n_atari_files_dir;
extern int UI_n_saved_files_dir;

#ifdef SDL
void PLATFORM_SetJoystickKey(int joystick, int direction, int value);
void PLATFORM_GetJoystickKeyName(int joystick, int direction, char *buffer, int bufsize);
int GetRawKey(void);
#endif

#ifdef DIRECTX
void PLATFORM_GetButtonAssignments(int stick, int button, char *buffer, int bufsize);
void PLATFORM_SetButtonAssignment(int stick, int button, int value);
int GetKeyName(void);
#endif

/* Menu codes for Alt+letter shortcuts.
   Store in UI_alt_function and put AKEY_UI in INPUT_key_code. */
#define UI_MENU_DISK             0
#define UI_MENU_CARTRIDGE        1
#define UI_MENU_RUN              2
#define UI_MENU_SYSTEM           3
#define UI_MENU_SOUND            4
#define UI_MENU_SOUND_RECORDING  5
#define UI_MENU_DISPLAY          6
#define UI_MENU_SETTINGS         7
#define UI_MENU_SAVESTATE        8
#define UI_MENU_LOADSTATE        9
#define UI_MENU_PCX              10
#define UI_MENU_PCXI             11
#define UI_MENU_BACK             12
#define UI_MENU_RESETW           13
#define UI_MENU_RESETC           14
#define UI_MENU_MONITOR          15
#define UI_MENU_ABOUT            16
#define UI_MENU_EXIT             17
#define UI_MENU_CASSETTE         18
#define UI_MENU_CONTROLLER       19
#define UI_MENU_WINDOWS	         20

#ifdef DIRECTX
	#define UI_MENU_SAVE_CONFIG      21
	#define UI_MENU_FUNCT_KEY_HELP   22
	#define UI_MENU_HOT_KEY_HELP     23
#endif

/* Structure of menu item. Each menu is just an array of items of this structure
   terminated by UI_MENU_END */
typedef struct
{
	UWORD flags;   /* Flags, see values below */
	SWORD retval;  /* Value returned by Select when this item is selected */
	               /* < 0 means that item is strictly informative and cannot be selected */
	char *prefix;  /* Text to prepend the item */
	char *item;    /* Main item text */
	const char *suffix;  /* Optional text to show after the item text (e.g. key shortcut) */
	               /* or (if (flags & UI_ITEM_TIP) != 0) "tooltip" */
} UI_tMenuItem;

/* The following are item types, mutually exclusive. */
#define UI_ITEM_HIDDEN  0x00  /* Item does not appear in the menu */
#define UI_ITEM_ACTION  0x01  /* Item invokes an action */
#define UI_ITEM_CHECK   0x02  /* Item represents a boolean value */
#define UI_ITEM_FILESEL 0x03  /* Item invokes file/directory selection */
#define UI_ITEM_SUBMENU 0x04  /* Item opens a submenu */
/* UI_ITEM_CHECK means that the value of UI_ITEM_CHECKED is shown.
   UI_ITEM_FILESEL and UI_ITEM_SUBMENU are just for optional decorations,
   so the user knows what happens when he/she selects this item. */
#define UI_ITEM_TYPE    0x0f

/* The following are bit masks and should be combined with one of the above item types. */
#define UI_ITEM_CHECKED 0x10  /* The boolean value for UI_ITEM_CHECK is true */
#define UI_ITEM_TIP     0x20  /* suffix is shown when the item is selected rather than on its right */

#if defined(_WIN32_WCE) || defined(DREAMCAST)
/* No function keys nor Alt+letter on Windows CE, Sega DC */
#define UI_MENU_ACCEL(keystroke) NULL
#else
#define UI_MENU_ACCEL(keystroke) keystroke
#endif

#define UI_MENU_LABEL(item)                                    { UI_ITEM_ACTION, -1, NULL, item, NULL }
#define UI_MENU_ACTION(retval, item)                           { UI_ITEM_ACTION, retval, NULL, item, NULL }
#define UI_MENU_ACTION_PREFIX(retval, prefix, item)            { UI_ITEM_ACTION, retval, prefix, item, NULL }
#define UI_MENU_ACTION_PREFIX_TIP(retval, prefix, item, tip)   { UI_ITEM_ACTION | UI_ITEM_TIP, retval, prefix, item, tip }
#define UI_MENU_ACTION_ACCEL(retval, item, keystroke)          { UI_ITEM_ACTION, retval, NULL, item, UI_MENU_ACCEL(keystroke) }
#define UI_MENU_ACTION_TIP(retval, item, tip)                  { UI_ITEM_ACTION | UI_ITEM_TIP, retval, NULL, item, tip }
#define UI_MENU_CHECK(retval, item)                            { UI_ITEM_CHECK, retval, NULL, item, NULL }
#define UI_MENU_FILESEL(retval, item)                          { UI_ITEM_FILESEL, retval, NULL, item, NULL }
#define UI_MENU_FILESEL_PREFIX(retval, prefix, item)           { UI_ITEM_FILESEL, retval, prefix, item, NULL }
#define UI_MENU_FILESEL_PREFIX_TIP(retval, prefix, item, tip)  { UI_ITEM_FILESEL | UI_ITEM_TIP, retval, prefix, item, tip }
#define UI_MENU_FILESEL_ACCEL(retval, item, keystroke)         { UI_ITEM_FILESEL, retval, NULL, item, UI_MENU_ACCEL(keystroke) }
#define UI_MENU_FILESEL_TIP(retval, item, tip)                 { UI_ITEM_FILESEL | UI_ITEM_TIP, retval, NULL, item, tip }
#define UI_MENU_SUBMENU(retval, item)                          { UI_ITEM_SUBMENU, retval, NULL, item, NULL }
#define UI_MENU_SUBMENU_SUFFIX(retval, item, suffix)           { UI_ITEM_SUBMENU, retval, NULL, item, suffix }
#define UI_MENU_SUBMENU_ACCEL(retval, item, keystroke)         { UI_ITEM_SUBMENU, retval, NULL, item, UI_MENU_ACCEL(keystroke) }
#define UI_MENU_END                                            { UI_ITEM_HIDDEN, 0, NULL, NULL, NULL }

/* UI driver entry prototypes */

/* Select provides simple selection from menu.
   title can be used as a caption.
   If (flags & UI_SELECT_POPUP) != 0 the menu is a popup menu.
   default_item is initially selected item (not item # but rather retval from menu structure).
   menu is array of items of type UI_tMenuItem, must be termintated by UI_MENU_END.
   If seltype is non-null, it is used to return selection type (see UI_USER_* below).
   UI_USER_DRAG_UP and UI_USER_DRAG_DOWN are returned only if (flags & UI_SELECT_DRAG) != 0.
   Returned is retval of the selected item or -1 if the user cancelled selection,
   or -2 if the user pressed "magic key" (Tab). */
typedef int (*UI_fnSelect)(const char *title, int flags, int default_item, const UI_tMenuItem *menu, int *seltype);
/* SelectInt returns an integer chosen by the user from the range min_value..max_value.
   default_value is the initial selection and the value returned if the selection is cancelled. */
typedef int (*UI_fnSelectInt)(int default_value, int min_value, int max_value);
/* SelectSlider selects integer chosen by user from the range 0..max_value.
   start_value is the slider's initial value. The label displayed at the slider is created
   by calling label_fun. This function takes three parameters:
   - *label - a buffer into which the label shuld be written to (not longer than 10 chars,
     excluding the trailing \0);
   - value - the slider's current value, on which the label should be based;
   - *user_data - a pointer to any data provided by user in SelectSlider's
     *user_data parameter.
   Returns -1 when the user has cancelled.
   The return value should be converted by user to a usable range. */
typedef int (*UI_fnSelectSlider)(char const *title, int start_value, int max_value,
				 void (*label_fun)(char *label, int value, void *user_data),
				 void *user_data);
/* EditString provides string input. pString is shown initially and can be modified by the user.
   It won't exceed nSize characters, including NUL. Note that pString may be modified even
   when the user pressed Esc. */
typedef int (*UI_fnEditString)(const char *title, char *string, int size);
/* GetSaveFilename and GetLoadFilename return fully qualified file name via pFilename.
   pDirectories are "favourite" directories (there are nDirectories of them).
   Selection starts in the directory of the passed pFilename (i.e. pFilename must be initialized)
   or (if pFilename[0] == '\0') in the first "favourite" directory. */
typedef int (*UI_fnGetSaveFilename)(char *filename, char directories[][FILENAME_MAX], int n_directories);
typedef int (*UI_fnGetLoadFilename)(char *filename, char directories[][FILENAME_MAX], int n_directories);
/* GetDirectoryPath is a directory browser */
typedef int (*UI_fnGetDirectoryPath)(char *directory);
/* Message is just some kind of MessageBox */
typedef void (*UI_fnMessage)(const char *message, int waitforkey);
/* InfoScreen displays a "long" message.
   Caution: lines in pMessage should be ended with '\0', the message should be terminated with '\n'. */
typedef void (*UI_fnInfoScreen)(const char *title, const char *message);
/* Init is called to initialize driver every time UI code is executed.
   Driver must be protected against multiple inits. */
typedef void (*UI_fnInit)(void);

/* Bit masks for flags */
#define UI_SELECT_POPUP   0x01
#define UI_SELECT_DRAG    0x02

/* Values returned via seltype */
#define UI_USER_SELECT    1
#define UI_USER_TOGGLE    2
#define UI_USER_DELETE    3
#define UI_USER_DRAG_UP   4
#define UI_USER_DRAG_DOWN 5

typedef struct
{
	UI_fnSelect           fSelect;
	UI_fnSelectInt        fSelectInt;
	UI_fnSelectSlider     fSelectSlider;
	UI_fnEditString       fEditString;
	UI_fnGetSaveFilename  fGetSaveFilename;
	UI_fnGetLoadFilename  fGetLoadFilename;
	UI_fnGetDirectoryPath fGetDirectoryPath;
	UI_fnMessage          fMessage;
	UI_fnInfoScreen       fInfoScreen;
	UI_fnInit             fInit;
} UI_tDriver;

/* Current UI driver. Port can override it and set pointer to port
   specific driver */
extern UI_tDriver *UI_driver;

#endif /* UI_H_ */
