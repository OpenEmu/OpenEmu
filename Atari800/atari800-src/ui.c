/*
 * ui.c - main user interface
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2011 Atari800 development team (see DOC/CREDITS)
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

#define _POSIX_C_SOURCE 200112L /* for snprintf */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* free() */

#include "afile.h"
#include "antic.h"
#include "atari.h"
#include "binload.h"
#include "cartridge.h"
#include "cassette.h"
#include "util.h"
#if SUPPORTS_PLATFORM_PALETTEUPDATE
#include "colours.h"
#include "colours_external.h"
#include "colours_ntsc.h"
#endif
#include "compfile.h"
#include "cfg.h"
#include "cpu.h"
#include "devices.h" /* Devices_SetPrintCommand */
#include "esc.h"
#if NTSC_FILTER
#include "filter_ntsc.h"
#endif /* NTSC_FILTER */
#include "gtia.h"
#include "input.h"
#include "akey.h"
#include "log.h"
#include "memory.h"
#include "platform.h"
#include "rtime.h"
#include "screen.h"
#include "sio.h"
#include "statesav.h"
#include "sysrom.h"
#include "ui.h"
#include "ui_basic.h"
#ifdef XEP80_EMULATION
#include "xep80.h"
#endif /* XEP80_EMULATION */
#ifdef PBI_PROTO80
#include "pbi_proto80.h"
#endif /* PBI_PROTO80 */
#ifdef AF80
#include "af80.h"
#endif /* AF80 */
#ifdef SOUND
#include "pokeysnd.h"
#include "sndsave.h"
#include "sound.h"
#endif /* SOUND */
#ifdef DIRECTX
#include "win32\main.h"
#include "win32\joystick.h"
#include "win32\screen_win32.h"
#endif /* DIRECTX */
#if SUPPORTS_CHANGE_VIDEOMODE
#include "videomode.h"
#endif /* SUPPORTS_CHANGE_VIDEOMODE */
#if GUI_SDL
#include "sdl/video.h"
#include "sdl/video_sw.h"
#if HAVE_OPENGL
#include "sdl/video_gl.h"
#endif /* HAVE_OPENGL */
#endif /* GUI_SDL */

#ifdef DIRECTX
/* Display Settings */
extern RENDERMODE rendermode;
extern FRAMEPARAMS frameparams;
extern DISPLAYMODE displaymode;
extern FSRESOLUTION fsresolution;
extern SCREENMODE screenmode;
extern ASPECTMODE scalingmethod;
extern ASPECTRATIO aspectmode;
extern CROP crop;
extern OFFSET offset;
extern BOOL usecustomfsresolution;
extern BOOL hidecursor;
extern BOOL lockaspect;
extern BOOL showmenu;
extern int windowscale;
extern int fullscreenWidth;
extern int fullscreenHeight;
extern int origScreenWidth;
extern int origScreenHeight;
extern int origScreenDepth;

/* Controller Settings */
extern BOOL mapController1Buttons;
extern BOOL mapController2Buttons;

/* local variables */
static char desktopreslabel[30];
static char hcrop_label[4];
static char vcrop_label[4];
static char hshift_label[4];
static char vshift_label[4];
static char monitor_label[40];
static char native_width_label[10];
static char native_height_label[20];
#endif /* DIRECTX */

#ifdef _WIN32_WCE
extern int smooth_filter;
extern int filter_available;
extern int virtual_joystick;
extern void AboutPocketAtari(void);
#endif /* _WIN32_WCE */

#ifdef DREAMCAST
extern int db_mode;
extern int screen_tv_mode;
extern int emulate_paddles;
extern int glob_snd_ena;
extern void ButtonConfiguration(void);
extern void AboutAtariDC(void);
extern void update_vidmode(void);
extern void update_screen_updater(void);
#ifdef HZ_TEST
extern void do_hz_test(void);
#endif /* HZ_TEST */
#endif /* DREAMCAST */

UI_tDriver *UI_driver = &UI_BASIC_driver;

int UI_is_active = FALSE;
int UI_alt_function = -1;
int UI_current_function = -1;

#ifdef CRASH_MENU
int UI_crash_code = -1;
UWORD UI_crash_address;
UWORD UI_crash_afterCIM;
int CrashMenu(void);
#endif

char UI_atari_files_dir[UI_MAX_DIRECTORIES][FILENAME_MAX];
char UI_saved_files_dir[UI_MAX_DIRECTORIES][FILENAME_MAX];
int UI_n_atari_files_dir = 0;
int UI_n_saved_files_dir = 0;

static UI_tMenuItem *FindMenuItem(UI_tMenuItem *mip, int option)
{
	while (mip->retval != option)
		mip++;
	return mip;
}

static void SetItemChecked(UI_tMenuItem *mip, int option, int checked)
{
	FindMenuItem(mip, option)->flags = checked ? (UI_ITEM_CHECK | UI_ITEM_CHECKED) : UI_ITEM_CHECK;
}

static void FilenameMessage(const char *format, const char *filename)
{
	char msg[FILENAME_MAX + 30];
	snprintf(msg, sizeof(msg), format, filename);
	UI_driver->fMessage(msg, 1);
}

static const char * const cant_load = "Can't load \"%s\"";
static const char * const cant_save = "Can't save \"%s\"";
static const char * const created = "Created \"%s\"";

#define CantLoad(filename) FilenameMessage(cant_load, filename)
#define CantSave(filename) FilenameMessage(cant_save, filename)
#define Created(filename) FilenameMessage(created, filename)

/* Callback function that writes a text label to *LABEL, for use by
   the Select Mosaic RAM slider. */
static void MosaicSliderLabel(char *label, int value, void *user_data)
{
	sprintf(label, "%i KB", value * 4); /* WARNING: No more that 10 chars! */
}

static void SystemSettings(void)
{
	static UI_tMenuItem ram800_menu_array[] = {
		UI_MENU_ACTION(8, "8 KB"),
		UI_MENU_ACTION(16, "16 KB"),
		UI_MENU_ACTION(24, "24 KB"),
		UI_MENU_ACTION(32, "32 KB"),
		UI_MENU_ACTION(40, "40 KB"),
		UI_MENU_ACTION(48, "48 KB"),
		UI_MENU_ACTION(52, "52 KB"),
		UI_MENU_END
	};

	enum { MOSAIC_OTHER = 65 }; /* must be a value that's illegal for MEMORY_mosaic_num_banks */
	static UI_tMenuItem mosaic_ram_menu_array[] = {
		UI_MENU_ACTION(0, "Disabled"),
		UI_MENU_ACTION(4, "1 64K RAM Select board (16 KB)"),
		UI_MENU_ACTION(20, "2 64K RAM Select boards (80 KB)"),
		UI_MENU_ACTION(36, "3 64K RAM Select boards (144 KB)"),
		UI_MENU_ACTION(MOSAIC_OTHER, "Other"),
		UI_MENU_END
	};
	static UI_tMenuItem axlon_ram_menu_array[] = {
		UI_MENU_ACTION(0, "Disabled"),
		UI_MENU_ACTION(8, "128 KB"),
		UI_MENU_ACTION(16, "256 KB"),
		UI_MENU_ACTION(32, "512 KB"),
		UI_MENU_ACTION(64, "1 MB"),
		UI_MENU_ACTION(128, "2 MB"),
		UI_MENU_ACTION(256, "4 MB"),
		UI_MENU_END
	};
	static UI_tMenuItem ramxl_menu_array[] = {
		UI_MENU_ACTION(16, "16 KB"),
		UI_MENU_ACTION(32, "32 KB"),
		UI_MENU_ACTION(48, "48 KB"),
		UI_MENU_ACTION(64, "64 KB"),
		UI_MENU_ACTION(128, "128 KB"),
		UI_MENU_ACTION(192, "192 KB"),
		UI_MENU_ACTION(MEMORY_RAM_320_RAMBO, "320 KB (Rambo)"),
		UI_MENU_ACTION(MEMORY_RAM_320_COMPY_SHOP, "320 KB (Compy-Shop)"),
		UI_MENU_ACTION(576, "576 KB"),
		UI_MENU_ACTION(1088, "1088 KB"),
		UI_MENU_END
	};
	static UI_tMenuItem os800_menu_array[] = {
		UI_MENU_ACTION(SYSROM_AUTO, "Choose automatically"),
		UI_MENU_ACTION(SYSROM_A_NTSC, "Rev. A NTSC"),
		UI_MENU_ACTION(SYSROM_A_PAL, "Rev. A PAL"),
		UI_MENU_ACTION(SYSROM_B_NTSC, "Rev. B NTSC"),
		UI_MENU_ACTION(SYSROM_800_CUSTOM, "Custom"),
		UI_MENU_END
	};
	static UI_tMenuItem osxl_menu_array[] = {
		UI_MENU_ACTION(SYSROM_AUTO, "Choose automatically"),
		UI_MENU_ACTION(SYSROM_AA00R10, "AA00 Rev. 10"),
		UI_MENU_ACTION(SYSROM_AA01R11, "AA01 Rev. 11"),
		UI_MENU_ACTION(SYSROM_BB00R1, "BB00 Rev. 1"),
		UI_MENU_ACTION(SYSROM_BB01R2, "BB01 Rev. 2"),
		UI_MENU_ACTION(SYSROM_BB02R3, "BB02 Rev. 3"),
		UI_MENU_ACTION(SYSROM_BB02R3V4, "BB02 Rev. 3 Ver. 4"),
		UI_MENU_ACTION(SYSROM_CC01R4, "CC01 Rev. 4"),
		UI_MENU_ACTION(SYSROM_BB01R3, "BB01 Rev. 3"),
		UI_MENU_ACTION(SYSROM_BB01R4_OS, "BB01 Rev. 4"),
		UI_MENU_ACTION(SYSROM_BB01R59, "BB01 Rev. 59"),
		UI_MENU_ACTION(SYSROM_BB01R59A, "BB01 Rev. 59 alt."),
		UI_MENU_ACTION(SYSROM_XL_CUSTOM, "Custom"),
		UI_MENU_END
	};
	static UI_tMenuItem os5200_menu_array[] = {
		UI_MENU_ACTION(SYSROM_AUTO, "Choose automatically"),
		UI_MENU_ACTION(SYSROM_5200, "Original"),
		UI_MENU_ACTION(SYSROM_5200A, "Rev. A"),
		UI_MENU_ACTION(SYSROM_5200_CUSTOM, "Custom"),
		UI_MENU_END
	};
	static UI_tMenuItem * const os_menu_arrays[Atari800_MACHINE_SIZE] = {
		os800_menu_array,
		osxl_menu_array,
		os5200_menu_array
	};
	static UI_tMenuItem basic_menu_array[] = {
		UI_MENU_ACTION(SYSROM_AUTO, "Choose automatically"),
		UI_MENU_ACTION(SYSROM_BASIC_A, "Rev. A"),
		UI_MENU_ACTION(SYSROM_BASIC_B, "Rev. B"),
		UI_MENU_ACTION(SYSROM_BASIC_C, "Rev. C"),
		UI_MENU_ACTION(SYSROM_BASIC_CUSTOM, "Custom"),
		UI_MENU_END
	};
	static UI_tMenuItem xegame_menu_array[] = {
		UI_MENU_ACTION(0, "None"),
		UI_MENU_ACTION(SYSROM_AUTO, "Choose automatically"),
		UI_MENU_ACTION(SYSROM_XEGAME, "Missile Command"),
		UI_MENU_ACTION(SYSROM_XEGAME_CUSTOM, "Custom"),
		UI_MENU_END
	};
	static struct {
		int type;
		int ram;
		int basic;
		int leds;
		int f_keys;
		int jumper;
		int game;
		int keyboard;
	} const machine[] = {
		{ Atari800_MACHINE_800, 16, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },
		{ Atari800_MACHINE_800, 48, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE },
		{ Atari800_MACHINE_XLXE, 64, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE },
		{ Atari800_MACHINE_XLXE, 16, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE },
		{ Atari800_MACHINE_XLXE, 64, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE },
		{ Atari800_MACHINE_XLXE, 128, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE },
		{ Atari800_MACHINE_XLXE, 64, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE },
		{ Atari800_MACHINE_5200, 16, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE }
	};
	static UI_tMenuItem machine_menu_array[] = {
		UI_MENU_ACTION(0, "Atari 400 (16 KB)"),
		UI_MENU_ACTION(1, "Atari 800 (48 KB)"),
		UI_MENU_ACTION(2, "Atari 1200XL (64 KB)"),
		UI_MENU_ACTION(3, "Atari 600XL (16 KB)"),
		UI_MENU_ACTION(4, "Atari 800XL (64 KB)"),
		UI_MENU_ACTION(5, "Atari 130XE (128 KB)"),
		UI_MENU_ACTION(6, "Atari XEGS (64 KB)"),
		UI_MENU_ACTION(7, "Atari 5200 (16 KB)"),
		UI_MENU_END
	};
	enum { N_MACHINES = (int) (sizeof(machine) / sizeof(machine[0])) };

	static UI_tMenuItem menu_array[] = {
		UI_MENU_SUBMENU_SUFFIX(0, "Machine:", NULL),
		UI_MENU_SUBMENU_SUFFIX(1, "OS version:", NULL),
		UI_MENU_ACTION(2, "BASIC:"),
		UI_MENU_SUBMENU_SUFFIX(3, "BASIC version:", NULL),
		UI_MENU_SUBMENU_SUFFIX(4, "XEGS game:", NULL),
		UI_MENU_SUBMENU_SUFFIX(5, "RAM size:", NULL),
		UI_MENU_ACTION(6, "Video system:"),
		UI_MENU_SUBMENU_SUFFIX(7, "Mosaic RAM:", NULL),
		UI_MENU_SUBMENU_SUFFIX(8, "Axlon RAMDisk:", NULL),
		UI_MENU_ACTION(9, "Axlon RAMDisk page $0F shadow:"),
		UI_MENU_SUBMENU(10, "1200XL keyboard LEDs:"),
		UI_MENU_ACTION(11, "1200XL F1-F4 keys:"),
		UI_MENU_ACTION(12, "1200XL option jumper J1:"),
		UI_MENU_ACTION(13, "Keyboard:"),
		UI_MENU_ACTION(14, "MapRAM:"),
		UI_MENU_END
	};

	/* Size must be long enough to store "<longest OS label> (auto)". */
	char default_os_label[26];
	/* Size must be long enough to store "<longest BASIC label> (auto)". */
	char default_basic_label[14];
	/* Size must be long enough to store "<longest XEGAME label> (auto)". */
	char default_xegame_label[23];
	char mosaic_label[7]; /* Fits "256 KB" */

	int option = 0;
	int option2 = 0;
	int new_tv_mode = Atari800_tv_mode;
	int need_initialise = FALSE;

	for (;;) {
		int sys_id;
		/* Set label for the "Machine" action. */
		for (sys_id = 0; sys_id < N_MACHINES; ++sys_id) {
			if (Atari800_machine_type == machine[sys_id].type
			    && MEMORY_ram_size == machine[sys_id].ram
			    && Atari800_builtin_basic == machine[sys_id].basic
			    && Atari800_keyboard_leds == machine[sys_id].leds
			    && Atari800_f_keys == machine[sys_id].f_keys
			    && (machine[sys_id].jumper || !Atari800_jumper)
			    && Atari800_builtin_game == machine[sys_id].game
			    && (machine[sys_id].keyboard || !Atari800_keyboard_detached)) {
				menu_array[0].suffix = machine_menu_array[sys_id].item;
				break;
			}
		}
		if (sys_id >= N_MACHINES) { /* Loop ended without break */
			if (Atari800_machine_type == Atari800_MACHINE_XLXE)
				menu_array[0].suffix = "Custom XL/XE";
			else
				menu_array[0].suffix = "Custom 400/800";
		}

		/* Set label for the "OS version" action. */
		if (SYSROM_os_versions[Atari800_machine_type] == SYSROM_AUTO) {
			int auto_os = SYSROM_AutoChooseOS(Atari800_machine_type, MEMORY_ram_size, new_tv_mode);
			if (auto_os == -1)
				menu_array[1].suffix = "ROM missing";
			else {
				sprintf(default_os_label, "%s (auto)", FindMenuItem(os_menu_arrays[Atari800_machine_type], auto_os)->item);
				menu_array[1].suffix = default_os_label;
			}
		}
		else if (SYSROM_roms[SYSROM_os_versions[Atari800_machine_type]].filename[0] == '\0')
			menu_array[1].suffix = "ROM missing";
		else
			menu_array[1].suffix = FindMenuItem(os_menu_arrays[Atari800_machine_type], SYSROM_os_versions[Atari800_machine_type])->item;

		/* Set label for the "BASIC" action. */
		menu_array[2].suffix = Atari800_machine_type == Atari800_MACHINE_5200
		                       ? "N/A"
		                       : Atari800_builtin_basic ? "built in" : "external";

		/* Set label for the "BASIC version" action. */
		if (Atari800_machine_type == Atari800_MACHINE_5200)
			menu_array[3].suffix = "N/A";
		else {
			if (SYSROM_basic_version == SYSROM_AUTO) {
				int auto_basic = SYSROM_AutoChooseBASIC();
				if (auto_basic == -1)
					menu_array[3].suffix = "ROM missing";
				else {
					sprintf(default_basic_label, "%s (auto)", FindMenuItem(basic_menu_array, auto_basic)->item);
					menu_array[3].suffix = default_basic_label;
				}
			}
			else if (SYSROM_roms[SYSROM_basic_version].filename[0] == '\0')
				menu_array[3].suffix = "ROM missing";
			else {
				menu_array[3].suffix = FindMenuItem(basic_menu_array, SYSROM_basic_version)->item;
			}
		}

		/* Set label for the "Builtin XEGS game" action. */
		if (Atari800_machine_type != Atari800_MACHINE_XLXE)
			menu_array[4].suffix = "N/A";
		else if (Atari800_builtin_game) {
			if (SYSROM_xegame_version == SYSROM_AUTO) {
				int auto_xegame = SYSROM_AutoChooseXEGame();
				if (auto_xegame == -1)
					menu_array[4].suffix = "ROM missing";
				else {
					sprintf(default_xegame_label, "%s (auto)", FindMenuItem(basic_menu_array, auto_xegame)->item);
					menu_array[4].suffix = default_xegame_label;
				}
			}
			else if (SYSROM_roms[SYSROM_xegame_version].filename[0] == '\0')
				menu_array[4].suffix = "ROM missing";
			else
				menu_array[4].suffix = FindMenuItem(xegame_menu_array, SYSROM_xegame_version)->item;
		}
		else
			menu_array[4].suffix = xegame_menu_array[0].item;

		/* Set label for the "RAM size" action. */
		switch (Atari800_machine_type) {
		case Atari800_MACHINE_800:
			menu_array[5].suffix = FindMenuItem(ram800_menu_array, MEMORY_ram_size)->item;
			break;
		case Atari800_MACHINE_XLXE:
			menu_array[5].suffix = FindMenuItem(ramxl_menu_array, MEMORY_ram_size)->item;
			break;
		case Atari800_MACHINE_5200:
			menu_array[5].suffix = "16 KB";
			break;
		}

		menu_array[6].suffix = (new_tv_mode == Atari800_TV_PAL) ? "PAL" : "NTSC";

		/* Set label for the "Mosaic" action. */
		if (Atari800_machine_type == Atari800_MACHINE_800) {
			if (MEMORY_mosaic_num_banks == 0)
				menu_array[7].suffix = mosaic_ram_menu_array[0].item;
			else {
				sprintf(mosaic_label, "%i KB", MEMORY_mosaic_num_banks * 4);
				menu_array[7].suffix = mosaic_label;
			}
		}
		else
			menu_array[7].suffix = "N/A";

		/* Set label for the "Axlon RAM" action. */
		menu_array[8].suffix = Atari800_machine_type != Atari800_MACHINE_800
		                       ? "N/A"
		                       : FindMenuItem(axlon_ram_menu_array, MEMORY_axlon_num_banks)->item;

		/* Set label for the "Axlon $0F shadow" action. */
		menu_array[9].suffix = Atari800_machine_type != Atari800_MACHINE_800
		                       ? "N/A"
		                       : MEMORY_axlon_0f_mirror ? "on" : "off";

		/* Set label for the "keyboard LEDs" action. */
		menu_array[10].suffix = Atari800_machine_type != Atari800_MACHINE_XLXE
		                        ? "N/A"
		                        : Atari800_keyboard_leds ? "Yes" : "No";

		/* Set label for the "F keys" action. */
		menu_array[11].suffix = Atari800_machine_type != Atari800_MACHINE_XLXE
		                        ? "N/A"
		                        : Atari800_f_keys ? "Yes" : "No";

		/* Set label for the "1200XL option jumper" action. */
		menu_array[12].suffix = Atari800_machine_type != Atari800_MACHINE_XLXE ? "N/A" :
		                        Atari800_jumper ? "installed" : "none";

		/* Set label for the "XEGS keyboard" action. */
		menu_array[13].suffix = Atari800_machine_type != Atari800_MACHINE_XLXE ? "N/A" :
		                        Atari800_keyboard_detached ? "detached (XEGS)" : "integrated/attached";

		/* Set label for the "XL/XE MapRAM" action. */
		menu_array[14].suffix = (Atari800_machine_type != Atari800_MACHINE_XLXE || MEMORY_ram_size < 20)
		                        ? "N/A"
		                        : MEMORY_enable_mapram ? "Yes" : "No";

		option = UI_driver->fSelect("System Settings", 0, option, menu_array, NULL);
		switch (option) {
		case 0:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, sys_id, machine_menu_array, NULL);
			if (option2 >= 0) {
				Atari800_machine_type = machine[option2].type;
				MEMORY_ram_size = machine[option2].ram;
				Atari800_builtin_basic = machine[option2].basic;
				Atari800_keyboard_leds = machine[option2].leds;
				Atari800_f_keys = machine[option2].f_keys;
				if (!machine[option2].jumper)
					Atari800_jumper = FALSE;
				Atari800_builtin_game = machine[option2].game;
				if (!machine[option2].keyboard)
					Atari800_keyboard_detached = FALSE;
				need_initialise = TRUE;
			}
			break;
		case 1:
			{
				int rom_available = FALSE;
				/* Start from index 1, to skip the "Choose automatically" option,
				   as it can never be hidden. */
				UI_tMenuItem *menu_ptr = os_menu_arrays[Atari800_machine_type] + 1;
				do {
					if (SYSROM_roms[menu_ptr->retval].filename[0] != '\0') {
						menu_ptr->flags = UI_ITEM_ACTION;
						rom_available = TRUE;
					}
					else
						menu_ptr->flags = UI_ITEM_HIDDEN;
				} while ((++menu_ptr)->flags != UI_ITEM_END);
				if (!rom_available)
					UI_driver->fMessage("No OS version available, ROMs missing", 1);
				else {
					option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, SYSROM_os_versions[Atari800_machine_type], os_menu_arrays[Atari800_machine_type], NULL);
					if (option2 >= 0) {
						SYSROM_os_versions[Atari800_machine_type] = option2;
						need_initialise = TRUE;
					}
				}
			}
			break;
		case 2:
			if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
				Atari800_builtin_basic = !Atari800_builtin_basic;
				need_initialise = TRUE;
			}
			break;
		case 3:
			if (Atari800_machine_type != Atari800_MACHINE_5200) {
				int rom_available = FALSE;
				/* Start from index 1, to skip the "Choose automatically" option,
				   as it can never be hidden. */
				UI_tMenuItem *menu_ptr = basic_menu_array + 1;
				do {
					if (SYSROM_roms[menu_ptr->retval].filename[0] != '\0') {
						menu_ptr->flags = UI_ITEM_ACTION;
						rom_available = TRUE;
					}
					else
						menu_ptr->flags = UI_ITEM_HIDDEN;
				} while ((++menu_ptr)->flags != UI_ITEM_END);

				if (!rom_available)
					UI_driver->fMessage("No BASIC available, ROMs missing", 1);
				else {
					option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, SYSROM_basic_version, basic_menu_array, NULL);
					if (option2 >= 0) {
						SYSROM_basic_version = option2;
						need_initialise = TRUE;
					}
				}
			}
			break;
		case 4:
			if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
				/* Start from index 2, to skip the "None" and "Choose automatically" options,
				   as they can never be hidden. */
				UI_tMenuItem *menu_ptr = xegame_menu_array + 2;
				do {
					if (SYSROM_roms[menu_ptr->retval].filename[0] != '\0') {
						menu_ptr->flags = UI_ITEM_ACTION;
					}
					else
						menu_ptr->flags = UI_ITEM_HIDDEN;
				} while ((++menu_ptr)->flags != UI_ITEM_END);

				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, Atari800_builtin_game ? SYSROM_xegame_version : 0, xegame_menu_array, NULL);
				if (option2 >= 0) {
					if (option2 > 0) {
						Atari800_builtin_game = TRUE;
						SYSROM_xegame_version = option2;
					}
					else
						Atari800_builtin_game = FALSE;
					need_initialise = TRUE;
				}
			}
			break;
		case 5:
			{
				UI_tMenuItem *menu_ptr;
				switch (Atari800_machine_type) {
				case Atari800_MACHINE_5200:
					goto leave;
				case Atari800_MACHINE_800:
					menu_ptr = ram800_menu_array;
					break;
				default: /* Atari800_MACHINE_XLXE */
					menu_ptr = ramxl_menu_array;
					break;
				}
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, MEMORY_ram_size, menu_ptr, NULL);
				if (option2 >= 0) {
					MEMORY_ram_size = option2;
					need_initialise = TRUE;
				}
			}
			leave:
			break;
		case 6:
			new_tv_mode = (new_tv_mode == Atari800_TV_PAL) ? Atari800_TV_NTSC : Atari800_TV_PAL;
			break;
		case 7:
			if (Atari800_machine_type == Atari800_MACHINE_800) {
				if (MEMORY_mosaic_num_banks == 0 || MEMORY_mosaic_num_banks == 4 || MEMORY_mosaic_num_banks == 20 || MEMORY_mosaic_num_banks == 36)
					option2 = MEMORY_mosaic_num_banks;
				else
					option2 = MOSAIC_OTHER;
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, option2, mosaic_ram_menu_array, NULL);
				if (option2 >= 0) {
					if (option2 == MOSAIC_OTHER) {
						int offset = 0;
						int value = UI_driver->fSelectSlider("Select amount of Mosaic RAM",
						                      MEMORY_mosaic_num_banks,
						                      64, &MosaicSliderLabel, &offset);
						if (value != -1) {
							MEMORY_mosaic_num_banks = value;
						}
					}
					else
						MEMORY_mosaic_num_banks = option2;
					if (option2 > 0)
						/* Can't have both Mosaic and Axlon active together. */
						MEMORY_axlon_num_banks = 0;
					need_initialise = TRUE;
				}
			}
			break;
		case 8:
			if (Atari800_machine_type == Atari800_MACHINE_800) {
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, MEMORY_axlon_num_banks, axlon_ram_menu_array, NULL);
				if (option2 >= 0) {
					MEMORY_axlon_num_banks = option2;
					if (option2 > 0)
						/* Can't have both Mosaic and Axlon active together. */
						MEMORY_mosaic_num_banks = 0;
					need_initialise = TRUE;
				}
			}
			break;
		case 9:
			if (Atari800_machine_type == Atari800_MACHINE_800) {
				MEMORY_axlon_0f_mirror = !MEMORY_axlon_0f_mirror;
				need_initialise = TRUE;
			}
			break;
		case 10:
			if (Atari800_machine_type == Atari800_MACHINE_XLXE)
				Atari800_keyboard_leds = !Atari800_keyboard_leds;
			break;
		case 11:
			if (Atari800_machine_type == Atari800_MACHINE_XLXE)
				Atari800_f_keys = !Atari800_f_keys;
			break;
		case 12:
			if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
				Atari800_jumper = !Atari800_jumper;
				Atari800_UpdateJumper();
			}
			break;
		case 13:
			if (Atari800_machine_type == Atari800_MACHINE_XLXE) {
				Atari800_keyboard_detached = !Atari800_keyboard_detached;
				Atari800_UpdateKeyboardDetached();
			}
			break;
		case 14:
			if (Atari800_machine_type == Atari800_MACHINE_XLXE && MEMORY_ram_size > 20) {
				MEMORY_enable_mapram = !MEMORY_enable_mapram;
				need_initialise = TRUE;
			}
			break;
		default:
			if (new_tv_mode != Atari800_tv_mode) {
				Atari800_SetTVMode(new_tv_mode);
				need_initialise = TRUE;
			}
			if (need_initialise)
				Atari800_InitialiseMachine();
			return;
		}
	}
}

/* Inspired by LNG (lng.sourceforge.net) */
/* Writes a blank ATR. The ATR must by formatted by an Atari DOS
   before files are written to it. */
static void MakeBlankDisk(FILE *setFile)
{
/* 720, so it's a standard Single Density disk,
   which can be formatted by 2.x DOSes.
   It will be resized when formatted in another density. */
#define BLANK_DISK_SECTORS  720
#define BLANK_DISK_PARAS    (BLANK_DISK_SECTORS * 128 / 16)
	int i;
	struct AFILE_ATR_Header hdr;
	UBYTE sector[128];

	memset(&hdr, 0, sizeof(hdr));
	hdr.magic1 = 0x96;
	hdr.magic2 = 0x02;
	hdr.seccountlo = (UBYTE) BLANK_DISK_PARAS;
	hdr.seccounthi = (UBYTE) (BLANK_DISK_PARAS >> 8);
	hdr.hiseccountlo = (UBYTE) (BLANK_DISK_PARAS >> 16);
	hdr.secsizelo = 128;
	fwrite(&hdr, 1, sizeof(hdr), setFile);

	memset(sector, 0, sizeof(sector));
	for (i = 1; i <= BLANK_DISK_SECTORS; i++)
		fwrite(sector, 1, sizeof(sector), setFile);
}

static void DiskManagement(void)
{
	static char drive_array[8][5] = { " D1:", " D2:", " D3:", " D4:", " D5:", " D6:", " D7:", " D8:" };

	static UI_tMenuItem menu_array[] = {
		UI_MENU_FILESEL_PREFIX_TIP(0, drive_array[0], SIO_filename[0], NULL),
		UI_MENU_FILESEL_PREFIX_TIP(1, drive_array[1], SIO_filename[1], NULL),
		UI_MENU_FILESEL_PREFIX_TIP(2, drive_array[2], SIO_filename[2], NULL),
		UI_MENU_FILESEL_PREFIX_TIP(3, drive_array[3], SIO_filename[3], NULL),
		UI_MENU_FILESEL_PREFIX_TIP(4, drive_array[4], SIO_filename[4], NULL),
		UI_MENU_FILESEL_PREFIX_TIP(5, drive_array[5], SIO_filename[5], NULL),
		UI_MENU_FILESEL_PREFIX_TIP(6, drive_array[6], SIO_filename[6], NULL),
		UI_MENU_FILESEL_PREFIX_TIP(7, drive_array[7], SIO_filename[7], NULL),
		UI_MENU_FILESEL(8, "Save Disk Set"),
		UI_MENU_FILESEL(9, "Load Disk Set"),
		UI_MENU_ACTION(10, "Rotate Disks"),
		UI_MENU_FILESEL(11, "Make Blank ATR Disk"),
		UI_MENU_FILESEL_TIP(12, "Uncompress Disk Image", "Convert GZ or DCM to ATR"),
		UI_MENU_END
	};

	int dsknum = 0;

	for (;;) {
		static char disk_filename[FILENAME_MAX];
		static char set_filename[FILENAME_MAX];
		int i;
		int seltype;

		for (i = 0; i < 8; i++) {
			drive_array[i][0] = ' ';
			switch (SIO_drive_status[i]) {
			case SIO_OFF:
				menu_array[i].suffix = "Return:insert";
				break;
			case SIO_NO_DISK:
				menu_array[i].suffix = "Return:insert Backspace:off";
				break;
			case SIO_READ_ONLY:
				drive_array[i][0] = '*';
				/* FALLTHROUGH */
			default:
				menu_array[i].suffix = "Ret:insert Bksp:eject Space:read-only";
				break;
			}
		}

		dsknum = UI_driver->fSelect("Disk Management", 0, dsknum, menu_array, &seltype);

		switch (dsknum) {
		case 8:
			if (UI_driver->fGetSaveFilename(set_filename, UI_saved_files_dir, UI_n_saved_files_dir)) {
				FILE *fp = fopen(set_filename, "w");
				if (fp == NULL) {
					CantSave(set_filename);
					break;
				}
				for (i = 0; i < 8; i++)
					fprintf(fp, "%s\n", SIO_filename[i]);
				fclose(fp);
				Created(set_filename);
			}
			break;
		case 9:
			if (UI_driver->fGetLoadFilename(set_filename, UI_saved_files_dir, UI_n_saved_files_dir)) {
				FILE *fp = fopen(set_filename, "r");
				if (fp == NULL) {
					CantLoad(set_filename);
					break;
				}
				for (i = 0; i < 8; i++) {
					char filename[FILENAME_MAX];
					if (fgets(filename, FILENAME_MAX, fp) != NULL) {
						Util_chomp(filename);
						if (strcmp(filename, "Empty") != 0 && strcmp(filename, "Off") != 0)
							SIO_Mount(i + 1, filename, FALSE);
					}
				}
				fclose(fp);
			}
			break;
		case 10:
			SIO_RotateDisks();
			break;
		case 11:
			if (UI_driver->fGetSaveFilename(disk_filename, UI_atari_files_dir, UI_n_atari_files_dir)) {
				FILE *fp = fopen(disk_filename, "wb");
				if (fp == NULL) {
					CantSave(disk_filename);
					break;
				}
				MakeBlankDisk(fp);
				fclose(fp);
				Created(disk_filename);
			}
			break;
		case 12:
			if (UI_driver->fGetLoadFilename(disk_filename, UI_atari_files_dir, UI_n_atari_files_dir)) {
				char uncompr_filename[FILENAME_MAX];
				FILE *fp = fopen(disk_filename, "rb");
				const char *p;
				if (fp == NULL) {
					CantLoad(disk_filename);
					break;
				}
				/* propose an output filename to make user's life easier */
				p = strrchr(disk_filename, '.');
				if (p != NULL) {
					char *q;
					p++;
					q = uncompr_filename + (p - disk_filename);
					if (Util_stricmp(p, "atz") == 0) {
						/* change last 'z' to 'r', preserving case */
						p += 2;
						q[2] = p[0] == 'z' ? 'r' : 'R';
						q[3] = '\0';
					}
					else if (Util_stricmp(p, "xfz") == 0) {
						/* change last 'z' to 'd', preserving case */
						p += 2;
						q[2] = p[0] == 'z' ? 'd' : 'D';
						q[3] = '\0';
					}
					else if (Util_stricmp(p, "gz") == 0) {
						/* strip ".gz" */
						p--;
						q[-1] = '\0';
					}
					else if (Util_stricmp(p, "atr") == 0) {
						/* ".atr" ? Probably won't work, anyway cut the extension but leave the dot */
						q[0] = '\0';
					}
					else {
						/* replace extension with "atr", preserving case */
						strcpy(q, p[0] <= 'Z' ? "ATR" : "atr");
					}
					memcpy(uncompr_filename, disk_filename, p - disk_filename);
				}
				else
					/* was no extension -> propose no filename */
					uncompr_filename[0] = '\0';
				/* recognize file type and uncompress */
				switch (fgetc(fp)) {
				case 0x1f:
					fclose(fp);
					if (UI_driver->fGetSaveFilename(uncompr_filename, UI_atari_files_dir, UI_n_atari_files_dir)) {
						FILE *fp2 = fopen(uncompr_filename, "wb");
						int success;
						if (fp2 == NULL) {
							CantSave(uncompr_filename);
							continue;
						}
						success = CompFile_ExtractGZ(disk_filename, fp2);
						fclose(fp2);
						UI_driver->fMessage(success ? "Conversion successful" : "Cannot convert this file", 1);
					}
					break;
				case 0xf9:
				case 0xfa:
					if (UI_driver->fGetSaveFilename(uncompr_filename, UI_atari_files_dir, UI_n_atari_files_dir)) {
						FILE *fp2 = fopen(uncompr_filename, "wb");
						int success;
						if (fp2 == NULL) {
							fclose(fp);
							CantSave(uncompr_filename);
							continue;
						}
						Util_rewind(fp);
						success = CompFile_DCMtoATR(fp, fp2);
						fclose(fp2);
						fclose(fp);
						UI_driver->fMessage(success ? "Conversion successful" : "Cannot convert this file", 1);
					}
					break;
				default:
					fclose(fp);
					UI_driver->fMessage("This is not a compressed disk image", 1);
					break;
				}
			}
			break;
		default:
			if (dsknum < 0)
				return;
			/* dsknum = 0..7 */
			switch (seltype) {
			case UI_USER_SELECT: /* "Enter" */
				if (SIO_drive_status[dsknum] != SIO_OFF && SIO_drive_status[dsknum] != SIO_NO_DISK)
					strcpy(disk_filename, SIO_filename[dsknum]);
				if (UI_driver->fGetLoadFilename(disk_filename, UI_atari_files_dir, UI_n_atari_files_dir))
					if (!SIO_Mount(dsknum + 1, disk_filename, FALSE))
						CantLoad(disk_filename);
				break;
			case UI_USER_TOGGLE: /* "Space" */
				i = FALSE;
				switch (SIO_drive_status[dsknum]) {
				case SIO_READ_WRITE:
					i = TRUE;
					/* FALLTHROUGH */
				case SIO_READ_ONLY:
					strcpy(disk_filename, SIO_filename[dsknum]);
					SIO_Mount(dsknum + 1, disk_filename, i);
					break;
				default:
					break;
				}
				break;
			default: /* Backspace */
				switch (SIO_drive_status[dsknum]) {
				case SIO_OFF:
					break;
				case SIO_NO_DISK:
					SIO_DisableDrive(dsknum + 1);
					break;
				default:
					SIO_Dismount(dsknum + 1);
					break;
				}
			}
			break;
		}
	}
}

int UI_SelectCartType(int k)
{
	UI_tMenuItem menu_array[] = {
		UI_MENU_ACTION(CARTRIDGE_STD_8, CARTRIDGE_TextDesc[CARTRIDGE_STD_8]),
		UI_MENU_ACTION(CARTRIDGE_STD_16, CARTRIDGE_TextDesc[CARTRIDGE_STD_16]),
		UI_MENU_ACTION(CARTRIDGE_OSS_034M_16, CARTRIDGE_TextDesc[CARTRIDGE_OSS_034M_16]),
		UI_MENU_ACTION(CARTRIDGE_5200_32, CARTRIDGE_TextDesc[CARTRIDGE_5200_32]),
		UI_MENU_ACTION(CARTRIDGE_DB_32, CARTRIDGE_TextDesc[CARTRIDGE_DB_32]),
		UI_MENU_ACTION(CARTRIDGE_5200_EE_16, CARTRIDGE_TextDesc[CARTRIDGE_5200_EE_16]),
		UI_MENU_ACTION(CARTRIDGE_5200_40, CARTRIDGE_TextDesc[CARTRIDGE_5200_40]),
		UI_MENU_ACTION(CARTRIDGE_WILL_64, CARTRIDGE_TextDesc[CARTRIDGE_WILL_64]),
		UI_MENU_ACTION(CARTRIDGE_EXP_64, CARTRIDGE_TextDesc[CARTRIDGE_EXP_64]),
		UI_MENU_ACTION(CARTRIDGE_DIAMOND_64, CARTRIDGE_TextDesc[CARTRIDGE_DIAMOND_64]),
		UI_MENU_ACTION(CARTRIDGE_SDX_64, CARTRIDGE_TextDesc[CARTRIDGE_SDX_64]),
		UI_MENU_ACTION(CARTRIDGE_XEGS_32, CARTRIDGE_TextDesc[CARTRIDGE_XEGS_32]),
		UI_MENU_ACTION(CARTRIDGE_XEGS_64, CARTRIDGE_TextDesc[CARTRIDGE_XEGS_64]),
		UI_MENU_ACTION(CARTRIDGE_XEGS_128, CARTRIDGE_TextDesc[CARTRIDGE_XEGS_128]),
		UI_MENU_ACTION(CARTRIDGE_OSS_M091_16, CARTRIDGE_TextDesc[CARTRIDGE_OSS_M091_16]),
		UI_MENU_ACTION(CARTRIDGE_5200_NS_16, CARTRIDGE_TextDesc[CARTRIDGE_5200_NS_16]),
		UI_MENU_ACTION(CARTRIDGE_ATRAX_128, CARTRIDGE_TextDesc[CARTRIDGE_ATRAX_128]),
		UI_MENU_ACTION(CARTRIDGE_BBSB_40, CARTRIDGE_TextDesc[CARTRIDGE_BBSB_40]),
		UI_MENU_ACTION(CARTRIDGE_5200_8, CARTRIDGE_TextDesc[CARTRIDGE_5200_8]),
		UI_MENU_ACTION(CARTRIDGE_5200_4, CARTRIDGE_TextDesc[CARTRIDGE_5200_4]),
		UI_MENU_ACTION(CARTRIDGE_RIGHT_8, CARTRIDGE_TextDesc[CARTRIDGE_RIGHT_8]),
		UI_MENU_ACTION(CARTRIDGE_WILL_32, CARTRIDGE_TextDesc[CARTRIDGE_WILL_32]),
		UI_MENU_ACTION(CARTRIDGE_XEGS_256, CARTRIDGE_TextDesc[CARTRIDGE_XEGS_256]),
		UI_MENU_ACTION(CARTRIDGE_XEGS_512, CARTRIDGE_TextDesc[CARTRIDGE_XEGS_512]),
		UI_MENU_ACTION(CARTRIDGE_XEGS_1024, CARTRIDGE_TextDesc[CARTRIDGE_XEGS_1024]),
		UI_MENU_ACTION(CARTRIDGE_MEGA_16, CARTRIDGE_TextDesc[CARTRIDGE_MEGA_16]),
		UI_MENU_ACTION(CARTRIDGE_MEGA_32, CARTRIDGE_TextDesc[CARTRIDGE_MEGA_32]),
		UI_MENU_ACTION(CARTRIDGE_MEGA_64, CARTRIDGE_TextDesc[CARTRIDGE_MEGA_64]),
		UI_MENU_ACTION(CARTRIDGE_MEGA_128, CARTRIDGE_TextDesc[CARTRIDGE_MEGA_128]),
		UI_MENU_ACTION(CARTRIDGE_MEGA_256, CARTRIDGE_TextDesc[CARTRIDGE_MEGA_256]),
		UI_MENU_ACTION(CARTRIDGE_MEGA_512, CARTRIDGE_TextDesc[CARTRIDGE_MEGA_512]),
		UI_MENU_ACTION(CARTRIDGE_MEGA_1024, CARTRIDGE_TextDesc[CARTRIDGE_MEGA_1024]),
		UI_MENU_ACTION(CARTRIDGE_SWXEGS_32, CARTRIDGE_TextDesc[CARTRIDGE_SWXEGS_32]),
		UI_MENU_ACTION(CARTRIDGE_SWXEGS_64, CARTRIDGE_TextDesc[CARTRIDGE_SWXEGS_64]),
		UI_MENU_ACTION(CARTRIDGE_SWXEGS_128, CARTRIDGE_TextDesc[CARTRIDGE_SWXEGS_128]),
		UI_MENU_ACTION(CARTRIDGE_SWXEGS_256, CARTRIDGE_TextDesc[CARTRIDGE_SWXEGS_256]),
		UI_MENU_ACTION(CARTRIDGE_SWXEGS_512, CARTRIDGE_TextDesc[CARTRIDGE_SWXEGS_512]),
		UI_MENU_ACTION(CARTRIDGE_SWXEGS_1024, CARTRIDGE_TextDesc[CARTRIDGE_SWXEGS_1024]),
		UI_MENU_ACTION(CARTRIDGE_PHOENIX_8, CARTRIDGE_TextDesc[CARTRIDGE_PHOENIX_8]),
		UI_MENU_ACTION(CARTRIDGE_BLIZZARD_16, CARTRIDGE_TextDesc[CARTRIDGE_BLIZZARD_16]),
		UI_MENU_ACTION(CARTRIDGE_ATMAX_128, CARTRIDGE_TextDesc[CARTRIDGE_ATMAX_128]),
		UI_MENU_ACTION(CARTRIDGE_ATMAX_1024, CARTRIDGE_TextDesc[CARTRIDGE_ATMAX_1024]),
		UI_MENU_ACTION(CARTRIDGE_SDX_128, CARTRIDGE_TextDesc[CARTRIDGE_SDX_128]),
		UI_MENU_ACTION(CARTRIDGE_OSS_8, CARTRIDGE_TextDesc[CARTRIDGE_OSS_8]),
		UI_MENU_ACTION(CARTRIDGE_OSS_043M_16, CARTRIDGE_TextDesc[CARTRIDGE_OSS_043M_16]),
		UI_MENU_ACTION(CARTRIDGE_BLIZZARD_4, CARTRIDGE_TextDesc[CARTRIDGE_BLIZZARD_4]),
		UI_MENU_ACTION(CARTRIDGE_AST_32, CARTRIDGE_TextDesc[CARTRIDGE_AST_32]),
		UI_MENU_ACTION(CARTRIDGE_ATRAX_SDX_64, CARTRIDGE_TextDesc[CARTRIDGE_ATRAX_SDX_64]),
		UI_MENU_ACTION(CARTRIDGE_ATRAX_SDX_128, CARTRIDGE_TextDesc[CARTRIDGE_ATRAX_SDX_128]),
		UI_MENU_ACTION(CARTRIDGE_TURBOSOFT_64, CARTRIDGE_TextDesc[CARTRIDGE_TURBOSOFT_64]),
		UI_MENU_ACTION(CARTRIDGE_TURBOSOFT_128, CARTRIDGE_TextDesc[CARTRIDGE_TURBOSOFT_128]),
		UI_MENU_ACTION(CARTRIDGE_ULTRACART_32, CARTRIDGE_TextDesc[CARTRIDGE_ULTRACART_32]),
		UI_MENU_ACTION(CARTRIDGE_LOW_BANK_8, CARTRIDGE_TextDesc[CARTRIDGE_LOW_BANK_8]),
		UI_MENU_ACTION(CARTRIDGE_SIC_128, CARTRIDGE_TextDesc[CARTRIDGE_SIC_128]),
		UI_MENU_ACTION(CARTRIDGE_SIC_256, CARTRIDGE_TextDesc[CARTRIDGE_SIC_256]),
		UI_MENU_ACTION(CARTRIDGE_SIC_512, CARTRIDGE_TextDesc[CARTRIDGE_SIC_512]),
		UI_MENU_ACTION(CARTRIDGE_STD_2, CARTRIDGE_TextDesc[CARTRIDGE_STD_2]),
		UI_MENU_ACTION(CARTRIDGE_STD_4, CARTRIDGE_TextDesc[CARTRIDGE_STD_4]),
		UI_MENU_ACTION(CARTRIDGE_RIGHT_4, CARTRIDGE_TextDesc[CARTRIDGE_RIGHT_4]),
		UI_MENU_END
	};

	int i;
	int option = 0;

	UI_driver->fInit();

	for (i = 1; i <= CARTRIDGE_LAST_SUPPORTED; i++)
		if (CARTRIDGE_kb[i] == k) {
			if (option == 0)
				option = i;
			menu_array[i - 1].flags = UI_ITEM_ACTION;
		}
		else
			menu_array[i - 1].flags = UI_ITEM_HIDDEN;

	if (option == 0)
		return CARTRIDGE_NONE;

	option = UI_driver->fSelect("Select Cartridge Type", 0, option, menu_array, NULL);
	if (option > 0)
		return option;

	return CARTRIDGE_NONE;
}

static void CartManagement(void)
{
	static UI_tMenuItem menu_array[] = {
		UI_MENU_FILESEL(0, "Create Cartridge from ROM image"),
		UI_MENU_FILESEL(1, "Extract ROM image from Cartridge"),
		UI_MENU_FILESEL_PREFIX_TIP(2, "Cartridge:", NULL, NULL),
		UI_MENU_FILESEL_PREFIX_TIP(3, "Piggyback:", NULL, NULL),
		UI_MENU_CHECK(4, "Reboot after cartridge change:"),
		UI_MENU_END
	};
	
	typedef struct {
		UBYTE id[4];
		UBYTE type[4];
		UBYTE checksum[4];
		UBYTE gash[4];
	} Header;
	
	int option = 2;
	int seltype;

	for (;;) {
		static char cart_filename[FILENAME_MAX];
		
		if (CARTRIDGE_main.type == CARTRIDGE_NONE) {
			menu_array[2].item = "None";
			menu_array[2].suffix = "Return:insert";
		}
		else {
			menu_array[2].item = CARTRIDGE_main.filename;
			menu_array[2].suffix = "Return:insert Backspace:remove";
		}

		if (CARTRIDGE_main.type == CARTRIDGE_SDX_64 || CARTRIDGE_main.type == CARTRIDGE_SDX_128) {
			menu_array[3].flags = UI_ITEM_FILESEL | UI_ITEM_TIP;
			if (CARTRIDGE_piggyback.type == CARTRIDGE_NONE) {
				menu_array[3].item = "None";
				menu_array[3].suffix = "Return:insert";
			}
			else {
				menu_array[3].item = CARTRIDGE_piggyback.filename;
				menu_array[3].suffix = "Return:insert Backspace:remove";
			}
		} else {
			menu_array[3].flags = UI_ITEM_HIDDEN;
		}

		SetItemChecked(menu_array, 4, CARTRIDGE_autoreboot);

		option = UI_driver->fSelect("Cartridge Management", 0, option, menu_array, &seltype);

		switch (option) {
		case 0:
			if (UI_driver->fGetLoadFilename(cart_filename, UI_atari_files_dir, UI_n_atari_files_dir)) {
				FILE *f;
				int nbytes;
				int type;
				UBYTE *image;
				int checksum;
				Header header;

				f = fopen(cart_filename, "rb");
				if (f == NULL) {
					CantLoad(cart_filename);
					break;
				}
				nbytes = Util_flen(f);
				if ((nbytes & 0x3ff) != 0) {
					fclose(f);
					UI_driver->fMessage("ROM image must be full kilobytes long", 1);
					break;
				}
				type = UI_SelectCartType(nbytes >> 10);
				if (type == CARTRIDGE_NONE) {
					fclose(f);
					break;
				}

				image = (UBYTE *) Util_malloc(nbytes);
				Util_rewind(f);
				if ((int) fread(image, 1, nbytes, f) != nbytes) {
					fclose(f);
					CantLoad(cart_filename);
					break;
				}
				fclose(f);

				if (!UI_driver->fGetSaveFilename(cart_filename, UI_atari_files_dir, UI_n_atari_files_dir))
					break;

				checksum = CARTRIDGE_Checksum(image, nbytes);
				header.id[0] = 'C';
				header.id[1] = 'A';
				header.id[2] = 'R';
				header.id[3] = 'T';
				header.type[0] = '\0';
				header.type[1] = '\0';
				header.type[2] = '\0';
				header.type[3] = (UBYTE) type;
				header.checksum[0] = (UBYTE) (checksum >> 24);
				header.checksum[1] = (UBYTE) (checksum >> 16);
				header.checksum[2] = (UBYTE) (checksum >> 8);
				header.checksum[3] = (UBYTE) checksum;
				header.gash[0] = '\0';
				header.gash[1] = '\0';
				header.gash[2] = '\0';
				header.gash[3] = '\0';

				f = fopen(cart_filename, "wb");
				if (f == NULL) {
					CantSave(cart_filename);
					break;
				}
				fwrite(&header, 1, sizeof(header), f);
				fwrite(image, 1, nbytes, f);
				fclose(f);
				free(image);
				Created(cart_filename);
			}
			break;
		case 1:
			if (UI_driver->fGetLoadFilename(cart_filename, UI_atari_files_dir, UI_n_atari_files_dir)) {
				FILE *f;
				int nbytes;
				Header header;
				UBYTE *image;

				f = fopen(cart_filename, "rb");
				if (f == NULL) {
					CantLoad(cart_filename);
					break;
				}
				nbytes = Util_flen(f) - sizeof(header);
				Util_rewind(f);
				if (nbytes <= 0 || fread(&header, 1, sizeof(header), f) != sizeof(header)
				 || header.id[0] != 'C' || header.id[1] != 'A' || header.id[2] != 'R' || header.id[3] != 'T') {
					fclose(f);
					UI_driver->fMessage("Not a CART file", 1);
					break;
				}
				image = (UBYTE *) Util_malloc(nbytes);
				if (fread(image, 1, nbytes, f) < nbytes) {
					UI_driver->fMessage("Error reading CART file", 1);
					break;
				}
				fclose(f);

				if (!UI_driver->fGetSaveFilename(cart_filename, UI_atari_files_dir, UI_n_atari_files_dir))
					break;

				f = fopen(cart_filename, "wb");
				if (f == NULL) {
					CantSave(cart_filename);
					break;
				}
				fwrite(image, 1, nbytes, f);
				fclose(f);
				free(image);
				Created(cart_filename);
			}
			break;
		case 2:
			switch (seltype) {
			case UI_USER_SELECT: /* Enter */
				if (UI_driver->fGetLoadFilename(CARTRIDGE_main.filename, UI_atari_files_dir, UI_n_atari_files_dir)) {
					int r = CARTRIDGE_InsertAutoReboot(CARTRIDGE_main.filename);
					switch (r) {
					case CARTRIDGE_CANT_OPEN:
						CantLoad(CARTRIDGE_main.filename);
						break;
					case CARTRIDGE_BAD_FORMAT:
						UI_driver->fMessage("Unknown cartridge format", 1);
						break;
					case CARTRIDGE_BAD_CHECKSUM:
						UI_driver->fMessage("Warning: bad CART checksum", 1);
						break;
					case 0:
						/* ok */
						break;
					default:
						/* r > 0 */
						CARTRIDGE_SetTypeAutoReboot(&CARTRIDGE_main, UI_SelectCartType(r));
						break;
					}
				}
				break;
			case UI_USER_DELETE: /* Backspace */
				CARTRIDGE_RemoveAutoReboot();
				break;
			}
			break;
		case 3:
			switch (seltype) {
			case UI_USER_SELECT: /* Enter */
				if (UI_driver->fGetLoadFilename(CARTRIDGE_piggyback.filename, UI_atari_files_dir, UI_n_atari_files_dir)) {
					int r = CARTRIDGE_Insert_Second(CARTRIDGE_piggyback.filename);
					switch (r) {
					case CARTRIDGE_CANT_OPEN:
						CantLoad(CARTRIDGE_piggyback.filename);
						break;
					case CARTRIDGE_BAD_FORMAT:
						UI_driver->fMessage("Unknown cartridge format", 1);
						break;
					case CARTRIDGE_BAD_CHECKSUM:
						UI_driver->fMessage("Warning: bad CART checksum", 1);
						break;
					case 0:
						/* ok */
						break;
					default:
						/* r > 0 */
						CARTRIDGE_SetType(&CARTRIDGE_piggyback, UI_SelectCartType(r));
						break;
					}
				}
				break;
			case UI_USER_DELETE: /* Backspace */
				CARTRIDGE_Remove_Second();
				break;
			}
			break;
		case 4:
			CARTRIDGE_autoreboot = !CARTRIDGE_autoreboot;
			break;
		default:
			return;
		}
	}
}

#if defined(SOUND) && !defined(DREAMCAST)
static void SoundRecording(void)
{
	if (!SndSave_IsSoundFileOpen()) {
		int no = 0;
		do {
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "atari%03d.wav", no);
			if (!Util_fileexists(buffer)) {
				/* file does not exist - we can create it */
				FilenameMessage(SndSave_OpenSoundFile(buffer)
					? "Recording sound to file \"%s\""
					: "Can't write to file \"%s\"", buffer);
				return;
			}
		} while (++no < 1000);
		UI_driver->fMessage("All atariXXX.wav files exist!", 1);
	}
	else {
		SndSave_CloseSoundFile();
		UI_driver->fMessage("Recording stopped", 1);
	}
}
#endif /* defined(SOUND) && !defined(DREAMCAST) */

static int AutostartFile(void)
{
	static char filename[FILENAME_MAX];
	if (UI_driver->fGetLoadFilename(filename, UI_atari_files_dir, UI_n_atari_files_dir)) {
		if (AFILE_OpenFile(filename, TRUE, 1, FALSE))
			return TRUE;
		CantLoad(filename);
	}
	return FALSE;
}

static void MakeBlankTapeMenu(void)
{
	char filenm[FILENAME_MAX];
	char description[CASSETTE_DESCRIPTION_MAX];
	description[0] = '\0';
	strncpy(filenm, CASSETTE_filename, FILENAME_MAX);
	if (!UI_driver->fGetSaveFilename(filenm, UI_atari_files_dir, UI_n_atari_files_dir))
		return;
	if (!UI_driver->fEditString("Enter tape's description", description, sizeof(description)))
		return;
	if (!CASSETTE_CreateCAS(filenm, description))
		CantSave(filenm);
}

/* Callback function that writes a text label to *LABEL, for use by
   any slider that adjusts tape position. */
static void TapeSliderLabel(char *label, int value, void *user_data)
{
	if (value >= CASSETTE_GetSize())
		sprintf(label, "End");
	else
		snprintf(label, 10, "%i", value + 1);
}

static void TapeManagement(void)
{
	static char position_string[17];
	static char cas_symbol[] = " C:";

	static UI_tMenuItem menu_array[] = {
		UI_MENU_FILESEL_PREFIX_TIP(0, cas_symbol, NULL, NULL),
		UI_MENU_LABEL("Description:"),
		UI_MENU_LABEL(CASSETTE_description),
		UI_MENU_ACTION_PREFIX_TIP(1, "Position: ", position_string, NULL),
		UI_MENU_CHECK(2, "Record:"),
		UI_MENU_SUBMENU(3, "Make blank tape"),
		UI_MENU_END
	};

	int option = 0;
	int seltype;

	for (;;) {

		int position = CASSETTE_GetPosition();
		int size = CASSETTE_GetSize();

		/* Set the cassette file description and set the Select Tape tip */
		switch (CASSETTE_status) {
		case CASSETTE_STATUS_NONE:
			menu_array[0].item = "None";
			menu_array[0].suffix = "Return:insert";
			menu_array[3].suffix = "Tape not loaded";
			cas_symbol[0] = ' ';
			break;
		case CASSETTE_STATUS_READ_ONLY:
			menu_array[0].item = CASSETTE_filename;
			menu_array[0].suffix = "Return:insert Backspace:eject";
			menu_array[3].suffix = "Return:change Backspace:rewind";
			cas_symbol[0] = '*';
			break;
		default: /* CASSETTE_STATUS_READ_WRITE */
			menu_array[0].item = CASSETTE_filename;
			menu_array[0].suffix = "Ret:insert Bksp:eject Space:read-only";
			menu_array[3].suffix = "Return:change Backspace:rewind";
			cas_symbol[0] = CASSETTE_write_protect ? '*' : ' ';
			break;
		}

		SetItemChecked(menu_array, 2, CASSETTE_record);

		if (CASSETTE_status == CASSETTE_STATUS_NONE)
			memcpy(position_string, "N/A", 4);
		else {
			if (position > size)
				snprintf(position_string, sizeof(position_string) - 1, "End/%u blocks", size);
			else
				snprintf(position_string, sizeof(position_string) - 1, "%u/%u blocks", position, size);
		}

		option = UI_driver->fSelect("Tape Management", 0, option, menu_array, &seltype);

		switch (option) {
		case 0:
			switch (seltype) {
			case UI_USER_SELECT: /* Enter */
				if (UI_driver->fGetLoadFilename(CASSETTE_filename, UI_atari_files_dir, UI_n_atari_files_dir)) {
					UI_driver->fMessage("Please wait while inserting...", 0);
					if (!CASSETTE_Insert(CASSETTE_filename)) {
						CantLoad(CASSETTE_filename);
						break;
					}
				}
				break;
			case UI_USER_DELETE: /* Backspace */
				if (CASSETTE_status != CASSETTE_STATUS_NONE)
					CASSETTE_Remove();
				break;
			case UI_USER_TOGGLE: /* Space */
				/* Toggle only if the cassette is mounted. */
				if (CASSETTE_status != CASSETTE_STATUS_NONE && !CASSETTE_ToggleWriteProtect())
					/* The file is read-only. */
					UI_driver->fMessage("Cannot switch to read/write", 1);
				break;
			}
			break;
		case 1:
			/* The Current Block control is inactive if no cassette file present */
			if (CASSETTE_status == CASSETTE_STATUS_NONE)
				break;

			switch (seltype) {
			case UI_USER_SELECT: { /* Enter */
					int value = UI_driver->fSelectSlider("Position tape",
					                                     position - 1,
					                                     size, &TapeSliderLabel, NULL);
					if (value != -1)
						CASSETTE_Seek(value + 1);
				}
				break;
			case UI_USER_DELETE: /* Backspace */
				CASSETTE_Seek(1);
				break;
			}
			break;
		case 2:
			/* Toggle only if the cassette is mounted. */
			if (CASSETTE_status != CASSETTE_STATUS_NONE && !CASSETTE_ToggleRecord())
				UI_driver->fMessage("Tape is read-only, recording will fail", 1);
			break;
		case 3:
			MakeBlankTapeMenu();
			break;
		default:
			return;
		}
	}

}

static void AdvancedHOptions(void)
{
	static char open_info[] = "0 currently open files";
	static UI_tMenuItem menu_array[] = {
		UI_MENU_ACTION(0, "Atari executables path"),
		UI_MENU_ACTION_TIP(1, open_info, NULL),
		UI_MENU_LABEL("Current directories:"),
		UI_MENU_ACTION_PREFIX_TIP(2, "H1:", Devices_h_current_dir[0], NULL),
		UI_MENU_ACTION_PREFIX_TIP(3, "H2:", Devices_h_current_dir[1], NULL),
		UI_MENU_ACTION_PREFIX_TIP(4, "H3:", Devices_h_current_dir[2], NULL),
		UI_MENU_ACTION_PREFIX_TIP(5, "H4:", Devices_h_current_dir[3], NULL),
		UI_MENU_END
	};
	int option = 0;
	for (;;) {
		int i;
		int seltype;
		i = Devices_H_CountOpen();
		open_info[0] = (char) ('0' + i);
		open_info[21] = (i != 1) ? 's' : '\0';
		menu_array[1].suffix = (i > 0) ? ((i == 1) ? "Backspace: close" : "Backspace: close all") : NULL;
		for (i = 0; i < 4; i++)
			menu_array[3 + i].suffix = Devices_h_current_dir[i][0] != '\0' ? "Backspace: reset to root" : NULL;
		option = UI_driver->fSelect("Advanced H: options", 0, option, menu_array, &seltype);
		switch (option) {
		case 0:
			{
				char tmp_path[FILENAME_MAX];
				strcpy(tmp_path, Devices_h_exe_path);
				if (UI_driver->fEditString("Atari executables path", tmp_path, FILENAME_MAX))
					strcpy(Devices_h_exe_path, tmp_path);
			}
			break;
		case 1:
			if (seltype == UI_USER_DELETE)
				Devices_H_CloseAll();
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			if (seltype == UI_USER_DELETE)
				Devices_h_current_dir[option - 2][0] = '\0';
			break;
		default:
			return;
		}
	}
}

static void ConfigureDirectories(void)
{
	static UI_tMenuItem menu_array[] = {
		UI_MENU_LABEL("Directories with Atari software:"),
		UI_MENU_FILESEL(0, UI_atari_files_dir[0]),
		UI_MENU_FILESEL(1, UI_atari_files_dir[1]),
		UI_MENU_FILESEL(2, UI_atari_files_dir[2]),
		UI_MENU_FILESEL(3, UI_atari_files_dir[3]),
		UI_MENU_FILESEL(4, UI_atari_files_dir[4]),
		UI_MENU_FILESEL(5, UI_atari_files_dir[5]),
		UI_MENU_FILESEL(6, UI_atari_files_dir[6]),
		UI_MENU_FILESEL(7, UI_atari_files_dir[7]),
		UI_MENU_FILESEL(8, "[add directory]"),
		UI_MENU_LABEL("Directories for emulator-saved files:"),
		UI_MENU_FILESEL(10, UI_saved_files_dir[0]),
		UI_MENU_FILESEL(11, UI_saved_files_dir[1]),
		UI_MENU_FILESEL(12, UI_saved_files_dir[2]),
		UI_MENU_FILESEL(13, UI_saved_files_dir[3]),
		UI_MENU_FILESEL(14, UI_saved_files_dir[4]),
		UI_MENU_FILESEL(15, UI_saved_files_dir[5]),
		UI_MENU_FILESEL(16, UI_saved_files_dir[6]),
		UI_MENU_FILESEL(17, UI_saved_files_dir[7]),
		UI_MENU_FILESEL(18, "[add directory]"),
		UI_MENU_ACTION(9, "What's this?"),
		UI_MENU_ACTION(19, "Back to Emulator Settings"),
		UI_MENU_END
	};
	int option = 9;
	int flags = 0;
	for (;;) {
		int i;
		int seltype;
		char tmp_dir[FILENAME_MAX];
		for (i = 0; i < 8; i++) {
			menu_array[1 + i].flags = (i < UI_n_atari_files_dir) ? (UI_ITEM_FILESEL | UI_ITEM_TIP) : UI_ITEM_HIDDEN;
			menu_array[11 + i].flags = (i < UI_n_saved_files_dir) ? (UI_ITEM_FILESEL | UI_ITEM_TIP) : UI_ITEM_HIDDEN;
			menu_array[1 + i].suffix = menu_array[11 + i].suffix = (flags != 0)
				? "Up/Down:move Space:release"
				: "Ret:change Bksp:delete Space:reorder";
		}
		if (UI_n_atari_files_dir < 2)
			menu_array[1].suffix = "Return:change Backspace:delete";
		if (UI_n_saved_files_dir < 2)
			menu_array[11].suffix = "Return:change Backspace:delete";
		menu_array[9].flags = (UI_n_atari_files_dir < 8) ? UI_ITEM_FILESEL : UI_ITEM_HIDDEN;
		menu_array[19].flags = (UI_n_saved_files_dir < 8) ? UI_ITEM_FILESEL : UI_ITEM_HIDDEN;
		option = UI_driver->fSelect("Configure Directories", flags, option, menu_array, &seltype);
		if (option < 0)
			return;
		if (flags != 0) {
			switch (seltype) {
			case UI_USER_DRAG_UP:
				if (option != 0 && option != 10) {
					strcpy(tmp_dir, menu_array[1 + option].item);
					strcpy(menu_array[1 + option].item, menu_array[option].item);
					strcpy(menu_array[option].item, tmp_dir);
					option--;
				}
				break;
			case UI_USER_DRAG_DOWN:
				if (option != UI_n_atari_files_dir - 1 && option != 10 + UI_n_saved_files_dir - 1) {
					strcpy(tmp_dir, menu_array[1 + option].item);
					strcpy(menu_array[1 + option].item, menu_array[2 + option].item);
					strcpy(menu_array[2 + option].item, tmp_dir);
					option++;
				}
				break;
			default:
				flags = 0;
				break;
			}
			continue;
		}
		switch (option) {
		case 8:
			tmp_dir[0] = '\0';
			if (UI_driver->fGetDirectoryPath(tmp_dir)) {
				strcpy(UI_atari_files_dir[UI_n_atari_files_dir], tmp_dir);
				option = UI_n_atari_files_dir++;
			}
			break;
		case 18:
			tmp_dir[0] = '\0';
			if (UI_driver->fGetDirectoryPath(tmp_dir)) {
				strcpy(UI_saved_files_dir[UI_n_saved_files_dir], tmp_dir);
				option = 10 + UI_n_saved_files_dir++;
			}
			break;
		case 9:
#if 0
			{
				static const UI_tMenuItem help_menu_array[] = {
					UI_MENU_LABEL("You can configure directories,"),
					UI_MENU_LABEL("where you usually store files used by"),
					UI_MENU_LABEL("Atari800, to make them available"),
					UI_MENU_LABEL("at Tab key press in file selectors."),
					UI_MENU_LABEL(""),
					UI_MENU_LABEL("\"Directories with Atari software\""),
					UI_MENU_LABEL("are for disk images, cartridge images,"),
					UI_MENU_LABEL("tape images, executables and BASIC"),
					UI_MENU_LABEL("programs."),
					UI_MENU_LABEL(""),
					UI_MENU_LABEL("\"Directories for emulator-saved files\""),
					UI_MENU_LABEL("are for state files, disk sets and"),
					UI_MENU_LABEL("screenshots taken via User Interface."),
					UI_MENU_LABEL(""),
					UI_MENU_LABEL(""),
					UI_MENU_ACTION(0, "Back"),
					UI_MENU_END
				};
				UI_driver->fSelect("Configure Directories - Help", 0, 0, help_menu_array, NULL);
			}
#else
			UI_driver->fInfoScreen("Configure Directories - Help",
				"You can configure directories,\0"
				"where you usually store files used by\0"
				"Atari800, to make them available\0"
				"at Tab key press in file selectors.\0"
				"\0"
				"\"Directories with Atari software\"\0"
				"are for disk images, cartridge images,\0"
				"tape images, executables\0"
				"and BASIC programs.\0"
				"\0"
				"\"Directories for emulator-saved files\"\0"
				"are for state files, disk sets and\0"
				"screenshots taken via User Interface.\0"
				"\n");
#endif
			break;
		case 19:
			return;
		default:
			if (seltype == UI_USER_TOGGLE) {
				if ((option < 10 ? UI_n_atari_files_dir : UI_n_saved_files_dir) > 1)
					flags = UI_SELECT_DRAG;
			}
			else if (seltype == UI_USER_DELETE) {
				if (option < 10) {
					if (option >= --UI_n_atari_files_dir) {
						option = 8;
						break;
					}
					for (i = option; i < UI_n_atari_files_dir; i++)
						strcpy(UI_atari_files_dir[i], UI_atari_files_dir[i + 1]);
				}
				else {
					if (option >= --UI_n_saved_files_dir) {
						option = 18;
						break;
					}
					for (i = option - 10; i < UI_n_saved_files_dir; i++)
						strcpy(UI_saved_files_dir[i], UI_saved_files_dir[i + 1]);
				}
			}
			else
				UI_driver->fGetDirectoryPath(menu_array[1 + option].item);
			break;
		}
	}
}

static void ROMLocations(char const *title, UI_tMenuItem *menu_array)
{
	int option = 0;

	for (;;) {
		int seltype;

		UI_tMenuItem *item;
		for (item = menu_array; item->flags != UI_ITEM_END; ++item) {
			int i = item->retval;
			if (SYSROM_roms[i].filename[0] == '\0')
				item->item = "None";
			else
				item->item = SYSROM_roms[i].filename;
		}

		option = UI_driver->fSelect(title, 0, option, menu_array, &seltype);
		if (option < 0)
			return;
		if (seltype == UI_USER_DELETE)
			SYSROM_roms[option].filename[0] = '\0';
		else {
			char filename[FILENAME_MAX] = "";
			if (SYSROM_roms[option].filename[0] != '\0')
				strcpy(filename, SYSROM_roms[option].filename);
			else {
				/* Use first non-empty ROM path as a starting filename for the dialog. */
				int i;
				for (i = 0; i < SYSROM_SIZE; ++i) {
					if (SYSROM_roms[i].filename[0] != '\0') {
						strcpy(filename, SYSROM_roms[i].filename);
						break;
					}
				}
			}
			for (;;) {
				if (!UI_driver->fGetLoadFilename(filename, NULL, 0))
					break;
				switch(SYSROM_SetPath(filename, 1, option)) {
				case SYSROM_ERROR:
					CantLoad(filename);
					continue;
				case SYSROM_BADSIZE:
					UI_driver->fMessage("Can't load, incorrect file size", 1);
					continue;
				case SYSROM_BADCRC:
					UI_driver->fMessage("Can't load, incorrect checksum", 1);
					continue;
				}
				break;
			}
		}
	}

}

static void ROMLocations800(void)
{
	static UI_tMenuItem menu_array[] = {
		UI_MENU_FILESEL_PREFIX(SYSROM_A_NTSC, " Rev. A NTSC:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_A_PAL, " Rev. A PAL:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_B_NTSC, " Rev. B NTSC:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_800_CUSTOM, " Custom:", NULL),
		UI_MENU_END
	};
	ROMLocations("400/800 OS ROM Locations", menu_array);
}

static void ROMLocationsXL(void)
{
	static UI_tMenuItem menu_array[] = {
		UI_MENU_FILESEL_PREFIX(SYSROM_AA00R10, " AA00 Rev. 10:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_AA01R11, " AA01 Rev. 11:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_BB00R1, " BB00 Rev. 1:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_BB01R2, " BB01 Rev. 2:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_BB02R3, " BB02 Rev. 3:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_BB02R3V4, " BB02 Rev. 3 Ver. 4:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_CC01R4, " CC01 Rev. 4:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_BB01R3, " BB01 Rev. 3:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_BB01R4_OS, " BB01 Rev. 4:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_BB01R59, " BB01 Rev. 59:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_BB01R59A, " BB01 Rev. 59 alt.:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_XL_CUSTOM, " Custom:", NULL),
		UI_MENU_END
	};
	ROMLocations("XL/XE OS ROM Locations", menu_array);
}

static void ROMLocations5200(void)
{
	static UI_tMenuItem menu_array[] = {
		UI_MENU_FILESEL_PREFIX(SYSROM_5200, " Original:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_5200A, " Rev. A:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_5200_CUSTOM, " Custom:", NULL),
		UI_MENU_END
	};
	ROMLocations("5200 BIOS ROM Locations", menu_array);
}

static void ROMLocationsBASIC(void)
{
	static UI_tMenuItem menu_array[] = {
		UI_MENU_FILESEL_PREFIX(SYSROM_BASIC_A, " Rev. A:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_BASIC_B, " Rev. B:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_BASIC_C, " Rev. C:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_BASIC_CUSTOM, " Custom:", NULL),
		UI_MENU_END
	};
	ROMLocations("BASIC ROM Locations", menu_array);
}

static void ROMLocationsXEGame(void)
{
	static UI_tMenuItem menu_array[] = {
		UI_MENU_FILESEL_PREFIX(SYSROM_XEGAME, " Original:", NULL),
		UI_MENU_FILESEL_PREFIX(SYSROM_XEGAME_CUSTOM, " Custom:", NULL),
		UI_MENU_END
	};
	ROMLocations("XEGS Builtin Game ROM Locations", menu_array);
}

static void SystemROMSettings(void)
{
	static UI_tMenuItem menu_array[] = {
		UI_MENU_FILESEL(0, "Find ROM images in a directory"),
		UI_MENU_SUBMENU(1, "400/800 OS ROM locations"),
		UI_MENU_SUBMENU(2, "XL/XE OS ROM locations"),
		UI_MENU_SUBMENU(3, "5200 BIOS ROM locations"),
		UI_MENU_SUBMENU(4, "BASIC ROM locations"),
		UI_MENU_SUBMENU(5, "XEGS builtin game ROM locations"),
		UI_MENU_END
	};

	int option = 0;

	for (;;) {
		int seltype;

		option = UI_driver->fSelect("System ROM Settings", 0, option, menu_array, &seltype);

		switch (option) {
		case 0:
			{
				char rom_dir[FILENAME_MAX] = "";
				int i;
				/* Use first non-empty ROM path as a starting filename for the dialog. */
				for (i = 0; i < SYSROM_SIZE; ++i) {
					if (SYSROM_roms[i].filename[0] != '\0') {
						Util_splitpath(SYSROM_roms[i].filename, rom_dir, NULL);
						break;
					}
				}
				if (UI_driver->fGetDirectoryPath(rom_dir))
					SYSROM_FindInDir(rom_dir, FALSE);
			}
			break;
		case 1:
			ROMLocations800();
			break;
		case 2:
			ROMLocationsXL();
			break;
		case 3:
			ROMLocations5200();
			break;
		case 4:
			ROMLocationsBASIC();
			break;
		case 5:
			ROMLocationsXEGame();
			break;
		default:
			return;
		}
	}
}

static void AtariSettings(void)
{
#ifdef XEP80_EMULATION
	static const UI_tMenuItem xep80_menu_array[] = {
		UI_MENU_ACTION(0, "off"),
		UI_MENU_ACTION(1, "port 1"),
		UI_MENU_ACTION(2, "port 2"),
		UI_MENU_END
	};
#endif /* XEP80_EMULATION */

	static UI_tMenuItem menu_array[] = {
		UI_MENU_CHECK(0, "Disable BASIC when booting Atari:"),
		UI_MENU_CHECK(1, "Boot from tape (hold Start):"),
		UI_MENU_CHECK(2, "Enable R-Time 8:"),
#ifdef XEP80_EMULATION
		UI_MENU_SUBMENU_SUFFIX(18, "Enable XEP80:", NULL),
#endif /* XEP80_EMULATION */
		UI_MENU_CHECK(3, "SIO patch (fast disk access):"),
		UI_MENU_CHECK(17, "Turbo (F12):"),
		UI_MENU_ACTION(4, "H: device (hard disk):"),
		UI_MENU_CHECK(5, "P: device (printer):"),
#ifdef R_IO_DEVICE
		UI_MENU_CHECK(6, "R: device (Atari850 via net):"),
#endif
		UI_MENU_FILESEL_PREFIX(7, "H1: ", Devices_atari_h_dir[0]),
		UI_MENU_FILESEL_PREFIX(8, "H2: ", Devices_atari_h_dir[1]),
		UI_MENU_FILESEL_PREFIX(9, "H3: ", Devices_atari_h_dir[2]),
		UI_MENU_FILESEL_PREFIX(10, "H4: ", Devices_atari_h_dir[3]),
		UI_MENU_SUBMENU(11, "Advanced H: options"),
		UI_MENU_ACTION_PREFIX(12, "Print command: ", Devices_print_command),
		UI_MENU_SUBMENU(13, "System ROM settings"),
		UI_MENU_SUBMENU(14, "Configure directories"),
		UI_MENU_ACTION(15, "Save configuration file"),
		UI_MENU_CHECK(16, "Save configuration on exit:"),
		UI_MENU_END
	};
	char tmp_command[256];

	int option = 0;

	for (;;) {
		int seltype;
		SetItemChecked(menu_array, 0, Atari800_disable_basic);
		SetItemChecked(menu_array, 1, CASSETTE_hold_start_on_reboot);
		SetItemChecked(menu_array, 2, RTIME_enabled);
		SetItemChecked(menu_array, 3, ESC_enable_sio_patch);
#ifdef XEP80_EMULATION
		FindMenuItem(menu_array, 18)->suffix = xep80_menu_array[XEP80_enabled ? XEP80_port + 1 : 0].item;
#endif /* XEP80_EMULATION */
		SetItemChecked(menu_array, 17, Atari800_turbo);
		FindMenuItem(menu_array, 4)->suffix = Devices_enable_h_patch ? (Devices_h_read_only ? "Read-only" : "Read/write") : "No ";
		SetItemChecked(menu_array, 5, Devices_enable_p_patch);
#ifdef R_IO_DEVICE
		SetItemChecked(menu_array, 6, Devices_enable_r_patch);
#endif
		SetItemChecked(menu_array, 16, CFG_save_on_exit);

		option = UI_driver->fSelect("Emulator Settings", 0, option, menu_array, &seltype);

		switch (option) {
		case 0:
			Atari800_disable_basic = !Atari800_disable_basic;
			break;
		case 1:
			CASSETTE_hold_start_on_reboot = !CASSETTE_hold_start_on_reboot;
			CASSETTE_hold_start = CASSETTE_hold_start_on_reboot;
			break;
		case 2:
			RTIME_enabled = !RTIME_enabled;
			break;
		case 3:
			ESC_enable_sio_patch = !ESC_enable_sio_patch;
			break;
		case 4:
			if (!Devices_enable_h_patch) {
				Devices_enable_h_patch = TRUE;
				Devices_h_read_only = TRUE;
			}
			else if (Devices_h_read_only)
				Devices_h_read_only = FALSE;
			else {
				Devices_enable_h_patch = FALSE;
				Devices_h_read_only = TRUE;
			}
			break;
		case 5:
			Devices_enable_p_patch = !Devices_enable_p_patch;
			break;
#ifdef R_IO_DEVICE
		case 6:
			Devices_enable_r_patch = !Devices_enable_r_patch;
			break;
#endif
		case 7:
		case 8:
		case 9:
		case 10:
			if (seltype == UI_USER_DELETE)
				FindMenuItem(menu_array, option)->item[0] = '\0';
			else
				UI_driver->fGetDirectoryPath(FindMenuItem(menu_array, option)->item);
			break;
		case 11:
			AdvancedHOptions();
			break;
		case 12:
			strcpy(tmp_command, Devices_print_command);
			if (UI_driver->fEditString("Print command", tmp_command, sizeof(tmp_command)))
				if (!Devices_SetPrintCommand(tmp_command))
					UI_driver->fMessage("Specified command is not allowed", 1);
			break;
		case 13:
			SystemROMSettings();
			break;
		case 14:
			ConfigureDirectories();
			break;
		case 15:
			UI_driver->fMessage(CFG_WriteConfig() ? "Configuration file updated" : "Error writing configuration file", 1);
			break;
		case 16:
			CFG_save_on_exit = !CFG_save_on_exit;
			break;
		case 17:
			Atari800_turbo = !Atari800_turbo;
			break;
#ifdef XEP80_EMULATION
		case 18:
			{
				int option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, XEP80_enabled ? XEP80_port + 1 : 0, xep80_menu_array, NULL);
				if (option2 == 0)
					XEP80_SetEnabled(FALSE);
				else if (option2 > 0) {
					if (XEP80_SetEnabled(TRUE))
						XEP80_port = option2 - 1;
					else
						UI_driver->fMessage("Error: Missing XEP80 charset ROM.", 1);
				}
			}
			break;
#endif /* XEP80_EMULATION */
		default:
			ESC_UpdatePatches();
			return;
		}
	}
}

static char state_filename[FILENAME_MAX];

static void SaveState(void)
{
	if (UI_driver->fGetSaveFilename(state_filename, UI_saved_files_dir, UI_n_saved_files_dir)) {
		int result;
		UI_driver->fMessage("Please wait while saving...", 0);
		result = StateSav_SaveAtariState(state_filename, "wb", TRUE);
		if (!result)
			CantSave(state_filename);
	}
}

static void LoadState(void)
{
	if (UI_driver->fGetLoadFilename(state_filename, UI_saved_files_dir, UI_n_saved_files_dir)) {
		UI_driver->fMessage("Please wait while loading...", 0);
		if (!StateSav_ReadAtariState(state_filename, "rb"))
			CantLoad(state_filename);
	}
}

/* CURSES_BASIC doesn't use artifacting or Atari800_collisions_in_skipped_frames,
   but does use Atari800_refresh_rate. However, changing Atari800_refresh_rate on CURSES is mostly
   useless, as the text-mode screen is normally updated infrequently by Atari.
   In case you wonder how artifacting affects CURSES version without
   CURSES_BASIC: it is visible on the screenshots (yes, they are bitmaps). */
#ifndef CURSES_BASIC

#if SUPPORTS_CHANGE_VIDEOMODE
/* Scrollable submenu for choosing fullscreen reslution. */
static int ChooseVideoResolution(int current_res)
{
	UI_tMenuItem *menu_array;
	char (*res_strings)[10];
	int num_res = VIDEOMODE_NumAvailableResolutions();

	unsigned int i;

	menu_array = Util_malloc((num_res+1) * sizeof(UI_tMenuItem));
	res_strings = Util_malloc(num_res * sizeof(char(*[10])));

	for (i = 0; i < num_res; i ++) {
		VIDEOMODE_CopyResolutionName(i, res_strings[i], 10);
		menu_array[i].flags = UI_ITEM_ACTION;
		menu_array[i].retval = i;
		menu_array[i].prefix = NULL;
		menu_array[i].item = res_strings[i];
		menu_array[i].suffix = NULL;
	}
	menu_array[num_res].flags = UI_ITEM_END;
	menu_array[num_res].retval = 0;
	menu_array[i].prefix = NULL;
	menu_array[i].item = NULL;
	menu_array[i].suffix = NULL;

	current_res = UI_driver->fSelect(NULL, UI_SELECT_POPUP, current_res, menu_array, NULL);
	free(res_strings);
	free(menu_array);
	return current_res;
}

/* Callback function that writes a text label to *LABEL, for use by
   any slider that adjusts an integer value. */
static void IntSliderLabel(char *label, int value, void *user_data)
{
	value += *((int *)user_data);
	sprintf(label, "%i", value); /* WARNING: No more that 10 chars! */
}

/* Callback function that writes a text label to *LABEL, for use by
   the Set Horizontal Offset slider. */
static void HorizOffsetSliderLabel(char *label, int value, void *user_data)
{
	value += *((int *)user_data);
	sprintf(label, "%i", value); /* WARNING: No more that 10 chars! */
	VIDEOMODE_SetHorizontalOffset(value);
}

/* Callback function that writes a text label to *LABEL, for use by
   the Set Vertical Offset slider. */
static void VertOffsetSliderLabel(char *label, int value, void *user_data)
{
	value += *((int *)user_data);
	sprintf(label, "%i", value); /* WARNING: No more that 10 chars! */
	VIDEOMODE_SetVerticalOffset(value);
}

#if GUI_SDL
/* Callback function that writes a text label to *LABEL, for use by
   the Scanlines Visibility slider. */
static void ScanlinesSliderLabel(char *label, int value, void *user_data)
{
	sprintf(label, "%i", value);
	SDL_VIDEO_SetScanlinesPercentage(value);
}
#endif /* GUI_SDL */

static void VideoModeSettings(void)
{
	static const UI_tMenuItem host_aspect_menu_array[] = {
		UI_MENU_ACTION(0, "autodetect"),
		UI_MENU_ACTION(1, "4:3"),
		UI_MENU_ACTION(2, "5:4"),
		UI_MENU_ACTION(3, "16:9"),
		UI_MENU_ACTION(4, "custom"),
		UI_MENU_END
	};
	static const UI_tMenuItem stretch_menu_array[] = {
		UI_MENU_ACTION(0, "none (1x)"),
		UI_MENU_ACTION(1, "2x"),
		UI_MENU_ACTION(2, "3x"),
		UI_MENU_ACTION(3, "fit screen - integral"),
		UI_MENU_ACTION(4, "fit screen - full"),
		UI_MENU_ACTION(5, "custom"),
		UI_MENU_END
	};
	char stretch_string[10];
	static const UI_tMenuItem fit_menu_array[] = {
		UI_MENU_ACTION(0, "fit width"),
		UI_MENU_ACTION(1, "fit height"),
		UI_MENU_ACTION(2, "fit both"),
		UI_MENU_END
	};
	static const UI_tMenuItem aspect_menu_array[] = {
		UI_MENU_ACTION(0, "don't keep"),
		UI_MENU_ACTION(1, "square pixels"),
		UI_MENU_ACTION(2, "authentic"),
		UI_MENU_END
	};
	static const UI_tMenuItem width_menu_array[] = {
		UI_MENU_ACTION(0, "narrow"),
		UI_MENU_ACTION(1, "like on TV"),
		UI_MENU_ACTION(2, "full overscan"),
		UI_MENU_ACTION(3, "custom"),
		UI_MENU_END
	};
	char horiz_area_string[4];
	static const UI_tMenuItem height_menu_array[] = {
		UI_MENU_ACTION(0, "short"),
		UI_MENU_ACTION(1, "like on TV"),
		UI_MENU_ACTION(2, "full overscan"),
		UI_MENU_ACTION(3, "custom"),
		UI_MENU_END
	};
	char vert_area_string[4];
	static char res_string[10];
	static char ratio_string[10];
	static char horiz_offset_string[4];
	static char vert_offset_string[4];
#if GUI_SDL
	static const UI_tMenuItem bpp_menu_array[] = {
		UI_MENU_ACTION(0, "autodetect"),
		UI_MENU_ACTION(1, "8"),
		UI_MENU_ACTION(2, "16"),
		UI_MENU_ACTION(3, "32"),
		UI_MENU_END
	};
	static char bpp_string[3];
#if HAVE_OPENGL
	static const UI_tMenuItem pixel_format_menu_array[] = {
		UI_MENU_ACTION(0, "16-bit BGR"),
		UI_MENU_ACTION(1, "16-bit RGB"),
		UI_MENU_ACTION(2, "32-bit BGRA"),
		UI_MENU_ACTION(3, "32-bit ARGB"),
		UI_MENU_END
	};
#endif /* HAVE_OPENGL */
	static char scanlines_string[4];
#endif /* GUI_SDL */

	static UI_tMenuItem menu_array[] = {
		UI_MENU_SUBMENU_SUFFIX(0, "Host display aspect ratio:", ratio_string),
#if GUI_SDL && HAVE_OPENGL
		UI_MENU_CHECK(1, "Hardware acceleration:"),
		UI_MENU_CHECK(2, " Bilinear filtering:"),
		UI_MENU_CHECK(3, " Use pixel buffer objects:"),
#endif /* GUI_SDL && HAVE_OPENGL */
		UI_MENU_CHECK(4, "Fullscreen:"),
		UI_MENU_SUBMENU_SUFFIX(5, " Fullscreen resolution:", res_string),
#if SUPPORTS_ROTATE_VIDEOMODE
		UI_MENU_CHECK(6, "Rotate sideways:"),
#endif /* SUPPORTS_ROTATE_VIDEOMODE */
#if GUI_SDL
		UI_MENU_SUBMENU_SUFFIX(7, "Bits per pixel:", bpp_string),
#if HAVE_OPENGL
		UI_MENU_SUBMENU_SUFFIX(8, "Pixel format:", NULL),
#endif /* HAVE_OPENGL */
		UI_MENU_CHECK(9, "Vertical synchronization:"),
#endif /* GUI_SDL */
		UI_MENU_SUBMENU_SUFFIX(10, "Image aspect ratio:", NULL),
		UI_MENU_SUBMENU_SUFFIX(11, "Stretch image:", NULL),
		UI_MENU_SUBMENU_SUFFIX(12, "Fit screen method:", NULL),
		UI_MENU_SUBMENU_SUFFIX(13, "Horizontal view area:", NULL),
		UI_MENU_SUBMENU_SUFFIX(14, "Vertical view area:", NULL),
		UI_MENU_SUBMENU_SUFFIX(15, "Horizontal shift:", horiz_offset_string),
		UI_MENU_SUBMENU_SUFFIX(16, "Vertical shift:", vert_offset_string),
#if GUI_SDL
		UI_MENU_SUBMENU_SUFFIX(17, "Scanlines visibility:", scanlines_string),
		UI_MENU_CHECK(18, " Interpolate scanlines:"),
#endif /* GUI_SDL */
		UI_MENU_END
	};
	int option = 0;
	int option2 = 0;
	int seltype;

	for (;;) {
		VIDEOMODE_CopyHostAspect(ratio_string, 10);
#if GUI_SDL
		snprintf(bpp_string, sizeof(bpp_string), "%d", SDL_VIDEO_SW_bpp);
#if HAVE_OPENGL
		SetItemChecked(menu_array, 1, SDL_VIDEO_opengl);
		SetItemChecked(menu_array, 2, SDL_VIDEO_GL_filtering);
		SetItemChecked(menu_array, 3, SDL_VIDEO_GL_pbo);
		if (SDL_VIDEO_opengl) {
			FindMenuItem(menu_array, 7)->flags = UI_ITEM_HIDDEN;
			FindMenuItem(menu_array, 8)->flags = UI_ITEM_SUBMENU;
			FindMenuItem(menu_array, 8)->suffix = pixel_format_menu_array[SDL_VIDEO_GL_pixel_format].item;
		} else {
			FindMenuItem(menu_array, 7)->flags = UI_ITEM_SUBMENU;
			FindMenuItem(menu_array, 8)->flags = UI_ITEM_HIDDEN;
		}
#endif /* HAVE_OPENGL */
		if (SDL_VIDEO_vsync && !SDL_VIDEO_vsync_available) {
			FindMenuItem(menu_array, 9)->flags = UI_ITEM_ACTION;
			FindMenuItem(menu_array, 9)->suffix = "N/A";
		} else {
			FindMenuItem(menu_array, 9)->flags = UI_ITEM_CHECK;
			SetItemChecked(menu_array, 9, SDL_VIDEO_vsync);
		}
		snprintf(scanlines_string, sizeof(scanlines_string), "%d", SDL_VIDEO_scanlines_percentage);
		SetItemChecked(menu_array, 18, SDL_VIDEO_interpolate_scanlines);
#endif /* GUI_SDL */
		SetItemChecked(menu_array, 4, !VIDEOMODE_windowed);
		VIDEOMODE_CopyResolutionName(VIDEOMODE_GetFullscreenResolution(), res_string, 10);
#if SUPPORTS_ROTATE_VIDEOMODE
		SetItemChecked(menu_array, 6, VIDEOMODE_rotate90);
#endif
		FindMenuItem(menu_array, 10)->suffix = aspect_menu_array[VIDEOMODE_keep_aspect].item;
		if (VIDEOMODE_stretch < VIDEOMODE_STRETCH_CUSTOM)
			FindMenuItem(menu_array, 11)->suffix = stretch_menu_array[VIDEOMODE_stretch].item;
		else {
			FindMenuItem(menu_array, 11)->suffix = stretch_string;
			snprintf(stretch_string, sizeof(stretch_string), "%f", VIDEOMODE_custom_stretch);
		}
		FindMenuItem(menu_array, 12)->suffix = fit_menu_array[VIDEOMODE_fit].item;
		if (VIDEOMODE_horizontal_area < VIDEOMODE_HORIZONTAL_CUSTOM)
			FindMenuItem(menu_array, 13)->suffix = width_menu_array[VIDEOMODE_horizontal_area].item;
		else {
			FindMenuItem(menu_array, 13)->suffix = horiz_area_string;
			snprintf(horiz_area_string, sizeof(horiz_area_string), "%d", VIDEOMODE_custom_horizontal_area);
		}
		if (VIDEOMODE_vertical_area < VIDEOMODE_VERTICAL_CUSTOM)
			FindMenuItem(menu_array, 14)->suffix = height_menu_array[VIDEOMODE_vertical_area].item;
		else {
			FindMenuItem(menu_array, 14)->suffix = vert_area_string;
			snprintf(vert_area_string, sizeof(vert_area_string), "%d", VIDEOMODE_custom_vertical_area);
		}
		snprintf(horiz_offset_string, sizeof(horiz_offset_string), "%d", VIDEOMODE_horizontal_offset);
		snprintf(vert_offset_string, sizeof(vert_offset_string), "%d", VIDEOMODE_vertical_offset);

		option = UI_driver->fSelect("Video Mode Settings", 0, option, menu_array, &seltype);
		switch (option) {
		case 0:
			{
				int current;
				for (current = 1; current < 4; ++current) {
					/* Find the currently-chosen host aspect ratio. */
					if (strcmp(ratio_string, host_aspect_menu_array[current].item) == 0)
						break;
				}
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, current, host_aspect_menu_array, NULL);
				if (option2 == 4) {
					char buffer[sizeof(ratio_string)];
					memcpy(buffer, ratio_string, sizeof(buffer));
					if (UI_driver->fEditString("Enter value in x:y format", buffer, sizeof(buffer)))
						VIDEOMODE_SetHostAspectString(buffer);
				}
				else if (option2 >= 1)
					VIDEOMODE_SetHostAspectString(host_aspect_menu_array[option2].item);
				else if (option2 == 0)
					VIDEOMODE_AutodetectHostAspect();
			}
			break;
#if GUI_SDL && HAVE_OPENGL
		case 1:
			SDL_VIDEO_ToggleOpengl();
			if (!SDL_VIDEO_opengl_available)
				UI_driver->fMessage("Error: OpenGL is not available.", 1);
				
			break;
		case 2:
			if (!SDL_VIDEO_opengl)
				UI_driver->fMessage("Works only with hardware acceleration.", 1);
			SDL_VIDEO_GL_ToggleFiltering();
			break;
		case 3:
			if (!SDL_VIDEO_opengl)
				UI_driver->fMessage("Works only with hardware acceleration.", 1);
			if (!SDL_VIDEO_GL_TogglePbo())
				UI_driver->fMessage("Pixel buffer objects not available.", 1);
			break;
#endif /* GUI_SDL && HAVE_OPENGL */
		case 4:
			VIDEOMODE_ToggleWindowed();
			break;
		case 5:
			option2 = ChooseVideoResolution(VIDEOMODE_GetFullscreenResolution());
			if (option2 >= 0)
				VIDEOMODE_SetFullscreenResolution(option2);
			break;
#if SUPPORTS_ROTATE_VIDEOMODE
		case 6:
			VIDEOMODE_ToggleRotate90();
			break;
#endif
#if GUI_SDL
		case 7:
			{
				int current;
				switch (SDL_VIDEO_SW_bpp) {
				case 0:
					current = 0;
					break;
				case 8:
					current = 1;
					break;
				case 16:
					current = 2;
					break;
				default: /* 32 */
					current = 3;
				}
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, current, bpp_menu_array, NULL);
				switch(option2) {
				case 0:
					SDL_VIDEO_SW_SetBpp(0);
					break;
				case 1:
					SDL_VIDEO_SW_SetBpp(8);
					break;
				case 2:
					SDL_VIDEO_SW_SetBpp(16);
					break;
				case 3:
					SDL_VIDEO_SW_SetBpp(32);
				}
			}
			break;
#if HAVE_OPENGL
		case 8:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, SDL_VIDEO_GL_pixel_format, pixel_format_menu_array, NULL);
			if (option2 >= 0)
				SDL_VIDEO_GL_SetPixelFormat(option2);
			break;
#endif /* HAVE_OPENGL */
		case 9:
			if (!SDL_VIDEO_ToggleVsync())
				UI_driver->fMessage("Not available in this video mode.", 1);
			break;
#endif /* GUI_SDL */
		case 10:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, VIDEOMODE_keep_aspect, aspect_menu_array, NULL);
			if (option2 >= 0)
				VIDEOMODE_SetKeepAspect(option2);
			break;
		case 11:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, VIDEOMODE_stretch, stretch_menu_array, NULL);
			if (option2 >= 0) {
				if (option2 < VIDEOMODE_STRETCH_CUSTOM)
					VIDEOMODE_SetStretch(option2);
				else {
					char buffer[sizeof(stretch_string)];
					snprintf(buffer, sizeof(buffer), "%f", VIDEOMODE_custom_stretch);
					if (UI_driver->fEditString("Enter multiplier", buffer, sizeof(buffer)))
						VIDEOMODE_SetCustomStretch(atof(buffer));
				}
			}
			break;
		case 12:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, VIDEOMODE_fit, fit_menu_array, NULL);
			if (option2 >= 0)
				VIDEOMODE_SetFit(option2);
			break;
		case 13:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, VIDEOMODE_horizontal_area, width_menu_array, NULL);
			if (option2 >= 0) {
				if (option2 < VIDEOMODE_HORIZONTAL_CUSTOM)
					VIDEOMODE_SetHorizontalArea(option2);
				else {
					int offset = VIDEOMODE_MIN_HORIZONTAL_AREA;
					int value = UI_driver->fSelectSlider("Adjust horizontal area",
					                                     VIDEOMODE_custom_horizontal_area - offset,
					                                     VIDEOMODE_MAX_HORIZONTAL_AREA - VIDEOMODE_MIN_HORIZONTAL_AREA,
					                                     &IntSliderLabel, &offset);
					if (value != -1)
						VIDEOMODE_SetCustomHorizontalArea(value + offset);
				}
			}
			break;
		case 14:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, VIDEOMODE_vertical_area, height_menu_array, NULL);
			if (option2 >= 0) {
				if (option2 < VIDEOMODE_VERTICAL_CUSTOM)
					VIDEOMODE_SetVerticalArea(option2);
				else {
					int offset = VIDEOMODE_MIN_VERTICAL_AREA;
					int value = UI_driver->fSelectSlider("Adjust vertical area",
					                                     VIDEOMODE_custom_vertical_area - offset,
					                                     VIDEOMODE_MAX_VERTICAL_AREA - VIDEOMODE_MIN_VERTICAL_AREA,
					                                     &IntSliderLabel, &offset);
					if (value != -1)
						VIDEOMODE_SetCustomVerticalArea(value + offset);
				}
			}
			break;
		case 15:
			switch (seltype) {
			case UI_USER_SELECT:
				{
					int range = VIDEOMODE_MAX_HORIZONTAL_AREA - VIDEOMODE_custom_horizontal_area;
					int offset = - range / 2;
					int value = UI_driver->fSelectSlider("Adjust horizontal shift",
					                                     VIDEOMODE_horizontal_offset - offset,
					                                     range, &HorizOffsetSliderLabel, &offset);
					if (value != -1)
						VIDEOMODE_SetHorizontalOffset(value + offset);
				}
				break;
			case UI_USER_DELETE:
				VIDEOMODE_SetHorizontalOffset(0);
				break;
			}
			break;
		case 16:
			switch (seltype) {
			case UI_USER_SELECT:
				{
					int range = VIDEOMODE_MAX_VERTICAL_AREA - VIDEOMODE_custom_vertical_area;
					int offset = - range / 2;
					int value = UI_driver->fSelectSlider("Adjust vertical shift",
					                                     VIDEOMODE_vertical_offset - offset,
					                                     range, &VertOffsetSliderLabel, &offset);
					if (value != -1)
						VIDEOMODE_SetVerticalOffset(value + offset);
				}
				break;
			case UI_USER_DELETE:
				VIDEOMODE_SetVerticalOffset(0);
				break;
			}
			break;
#if GUI_SDL
		case 17:
			{
				int value = UI_driver->fSelectSlider("Adjust scanlines visibility",
				                                     SDL_VIDEO_scanlines_percentage,
				                                     100, &ScanlinesSliderLabel, NULL);
				if (value != -1)
					SDL_VIDEO_SetScanlinesPercentage(value);
			}
			break;
		case 18:
			SDL_VIDEO_ToggleInterpolateScanlines();
			break;
#endif /* GUI_SDL */
		default:
			return;
		}
	}
}
#endif /* SUPPORTS_CHANGE_VIDEOMODE */

#if SUPPORTS_PLATFORM_PALETTEUPDATE
/* An array of pointers to colour controls (brightness, contrast, NTSC filter
   controls, etc.). Used to display values of controls in GUI. */
static struct {
	double min; /* Minimum allowed value */
	double max; /* Maximum allowed value */
	double *setting; /* Pointer to a setup value */
	char string[10]; /* string representation, for displaying */
} colour_controls[] = {
	{ COLOURS_BRIGHTNESS_MIN, COLOURS_BRIGHTNESS_MAX },
	{ COLOURS_CONTRAST_MIN, COLOURS_CONTRAST_MAX },
	{ COLOURS_SATURATION_MIN, COLOURS_SATURATION_MAX },
	{ COLOURS_HUE_MIN, COLOURS_HUE_MAX },
	{ COLOURS_GAMMA_MIN, COLOURS_GAMMA_MAX },
	{ COLOURS_DELAY_MIN, COLOURS_DELAY_MAX }
#if NTSC_FILTER
	,
	{ FILTER_NTSC_SHARPNESS_MIN, FILTER_NTSC_SHARPNESS_MAX },
	{ FILTER_NTSC_RESOLUTION_MIN, FILTER_NTSC_RESOLUTION_MAX },
	{ FILTER_NTSC_ARTIFACTS_MIN, FILTER_NTSC_ARTIFACTS_MAX },
	{ FILTER_NTSC_FRINGING_MIN, FILTER_NTSC_FRINGING_MAX },
	{ FILTER_NTSC_BLEED_MIN, FILTER_NTSC_BLEED_MAX },
	{ FILTER_NTSC_BURST_PHASE_MIN, FILTER_NTSC_BURST_PHASE_MAX }
#endif /* NTSC_FILTER */
};

/* Updates the string representation of a single colour control. */
static void UpdateColourControl(const int idx)
{
	snprintf(colour_controls[idx].string,
		 sizeof(colour_controls[0].string),
		 "%.2f",
		 *(colour_controls[idx].setting));
}

/* Sets pointers to colour controls properly. */
static void UpdateColourControls(UI_tMenuItem menu_array[])
{
	int i;
	colour_controls[0].setting = &Colours_setup->brightness;
	colour_controls[1].setting = &Colours_setup->contrast;
	colour_controls[2].setting = &Colours_setup->saturation;
	colour_controls[3].setting = &Colours_setup->hue;
	colour_controls[4].setting = &Colours_setup->gamma;
	colour_controls[5].setting = &Colours_setup->color_delay;
	for (i = 0; i < 6; i ++)
		UpdateColourControl(i);
}

/* Converts value of a colour setting to range usable by slider (0..100). */
static int ColourSettingToSlider(int index)
{
	/* min <= value <= max */
	return (int)Util_round((*(colour_controls[index].setting) - colour_controls[index].min) * 100.0 /
	                       (colour_controls[index].max - colour_controls[index].min));
}

/* Converts a slider value (0..100) to a range of a given colour setting. */
static double SliderToColourSetting(int value, int index)
{
	/* 0 <= value <= 100 */
	return (double)value * (colour_controls[index].max - colour_controls[index].min) / 100.0 + colour_controls[index].min;
}

/* Callback function that writes a text label to *LABEL, for use by a slider. */
static void ColourSliderLabel(char *label, int value, void *user_data)
{
	int index = *((int *)user_data);
	double setting = SliderToColourSetting(value, index);
	sprintf(label, "% .2f", setting);
	*colour_controls[index].setting = setting;
	UpdateColourControl(index);
	Colours_Update();
}

#if NTSC_FILTER
/* Submenu with controls for NTSC filter. */
static void NTSCFilterSettings(void)
{
	int i;
	int preset;

	static const UI_tMenuItem preset_menu_array[] = {
		UI_MENU_ACTION(0, "Composite"),
		UI_MENU_ACTION(1, "S-Video"),
		UI_MENU_ACTION(2, "RGB"),
		UI_MENU_ACTION(3, "Monochrome"),
		UI_MENU_END
	};
	static UI_tMenuItem menu_array[] = {
		UI_MENU_SUBMENU_SUFFIX(0, "Filter preset: ", NULL),
		UI_MENU_ACTION_PREFIX(1, "Sharpness: ", colour_controls[6].string),
		UI_MENU_ACTION_PREFIX(2, "Resolution: ", colour_controls[7].string),
		UI_MENU_ACTION_PREFIX(3, "Artifacts: ", colour_controls[8].string),
		UI_MENU_ACTION_PREFIX(4, "Fringing: ", colour_controls[9].string),
		UI_MENU_ACTION_PREFIX(5, "Bleed: ", colour_controls[10].string),
		UI_MENU_ACTION_PREFIX(6, "Burst phase: ", colour_controls[11].string),
		UI_MENU_ACTION(7, "Restore default settings"),
		UI_MENU_END
	};

	int option = 0;
	int option2;

	/* Set pointers to colour controls. */
	colour_controls[6].setting = &FILTER_NTSC_setup.sharpness;
	colour_controls[7].setting = &FILTER_NTSC_setup.resolution;
	colour_controls[8].setting = &FILTER_NTSC_setup.artifacts;
	colour_controls[9].setting = &FILTER_NTSC_setup.fringing;
	colour_controls[10].setting = &FILTER_NTSC_setup.bleed;
	colour_controls[11].setting = &FILTER_NTSC_setup.burst_phase;
	for (i = 6; i < 12; i ++)
		UpdateColourControl(i);

	for (;;) {
		preset = FILTER_NTSC_GetPreset();
		if (preset == FILTER_NTSC_PRESET_CUSTOM)
			FindMenuItem(menu_array, 0)->suffix = "Custom";
		else
			FindMenuItem(menu_array, 0)->suffix = preset_menu_array[preset].item;

		option = UI_driver->fSelect("NTSC Filter Settings", 0, option, menu_array, NULL);
		switch (option) {
		case 0:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, preset, preset_menu_array, NULL);
			if (option2 >= 0) {
				FILTER_NTSC_SetPreset(option2);
				Colours_Update();
				for (i=6; i<12; i++)
					UpdateColourControl(i);
			}
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			{
				int index = option + 5;
				int value = UI_driver->fSelectSlider("Adjust value",
								     ColourSettingToSlider(index),
								     100, &ColourSliderLabel, &index);
				if (value != -1) {
					*(colour_controls[index].setting) = SliderToColourSetting(value, index);
					UpdateColourControl(index);
					Colours_Update();
				}
			}
			break;
		case 7:
			FILTER_NTSC_RestoreDefaults();
			Colours_Update();
			for (i = 6; i < 12; i ++)
				UpdateColourControl(i);
			break;
		default:
			return;
		}
	}
}
#endif /* NTSC_FILTER */

/* Saves the current colours, including adjustments, in a palette file chosen
   by user. */
static void SavePalette(void)
{
	static char filename[FILENAME_MAX];
	if (UI_driver->fGetSaveFilename(filename, UI_saved_files_dir, UI_n_saved_files_dir)) {
		UI_driver->fMessage("Please wait while saving...", 0);
		if (!Colours_Save(filename))
			CantSave(filename);
	}
}
#endif /* SUPPORTS_PLATFORM_PALETTEUPDATE */

static void DisplaySettings(void)
{
	static const UI_tMenuItem artif_quality_menu_array[] = {
		UI_MENU_ACTION(0, "off"),
		UI_MENU_ACTION(1, "original"),
		UI_MENU_ACTION(2, "new"),
#if NTSC_FILTER
		UI_MENU_ACTION(3, "NTSC filter"),
#endif
		UI_MENU_END
	};
	static const UI_tMenuItem artif_mode_menu_array[] = {
		UI_MENU_ACTION(0, "blue/brown 1"),
		UI_MENU_ACTION(1, "blue/brown 2"),
		UI_MENU_ACTION(2, "GTIA"),
		UI_MENU_ACTION(3, "CTIA"),
		UI_MENU_END
	};

#if SUPPORTS_PLATFORM_PALETTEUPDATE	
	static const UI_tMenuItem colours_preset_menu_array[] = {
		UI_MENU_ACTION(0, "Standard (most realistic)"),
		UI_MENU_ACTION(1, "Deep black (allows pure black color)"),
		UI_MENU_ACTION(2, "Vibrant (enhanced colors and levels)"),
		UI_MENU_END
	};
	static char const * const colours_preset_names[] = { "Standard", "Deep black", "Vibrant", "Custom" };
#endif
	
	static char refresh_status[16];
	static UI_tMenuItem menu_array[] = {
#if SUPPORTS_CHANGE_VIDEOMODE
		UI_MENU_SUBMENU(24, "Video mode settings"),
#endif /* SUPPORTS_CHANGE_VIDEOMODE */
		UI_MENU_SUBMENU_SUFFIX(0, "NTSC artifacting quality:", NULL),
		UI_MENU_SUBMENU_SUFFIX(11, "NTSC artifacting mode:", NULL),
#if SUPPORTS_CHANGE_VIDEOMODE && (defined(XEP80_EMULATION) || defined(PBI_PROTO80) || defined(AF80))
		UI_MENU_CHECK(25, "Show output of 80 column device:"),
#endif
		UI_MENU_SUBMENU_SUFFIX(1, "Current refresh rate:", refresh_status),
		UI_MENU_CHECK(2, "Accurate skipped frames:"),
		UI_MENU_CHECK(3, "Show percents of Atari speed:"),
		UI_MENU_CHECK(4, "Show disk/tape activity:"),
		UI_MENU_CHECK(5, "Show sector/block counter:"),
		UI_MENU_CHECK(26, "Show 1200XL LEDs:"),
#ifdef _WIN32_WCE
		UI_MENU_CHECK(6, "Enable linear filtering:"),
#endif
#ifdef DREAMCAST
		UI_MENU_CHECK(7, "Double buffer video data:"),
		UI_MENU_ACTION(8, "Emulator video mode:"),
		UI_MENU_ACTION(9, "Display video mode:"),
#ifdef HZ_TEST
		UI_MENU_ACTION(10, "DO HZ TEST:"),
#endif
#endif
#if SUPPORTS_PLATFORM_PALETTEUPDATE
		UI_MENU_SUBMENU_SUFFIX(12, "Color preset: ", NULL),
		UI_MENU_ACTION_PREFIX(13, " Brightness: ", colour_controls[0].string),
		UI_MENU_ACTION_PREFIX(14, " Contrast: ", colour_controls[1].string),
		UI_MENU_ACTION_PREFIX(15, " Saturation: ", colour_controls[2].string),
		UI_MENU_ACTION_PREFIX(16, " Tint: ", colour_controls[3].string),
		UI_MENU_ACTION_PREFIX(17, " Gamma: ", colour_controls[4].string),
		UI_MENU_ACTION_PREFIX(18, " GTIA delay: ", colour_controls[5].string),
#if NTSC_FILTER
		UI_MENU_SUBMENU(19, "NTSC filter settings"),
#endif
		UI_MENU_ACTION(20, "Restore default colors"),
		UI_MENU_FILESEL_PREFIX_TIP(21, "External palette: ", NULL, NULL),
		UI_MENU_CHECK(22, "Also adjust external palette: "),
		UI_MENU_FILESEL(23, "Save current palette"),
#endif /* SUPPORTS_PLATFORM_PALETTEUPDATE */
		UI_MENU_END
	};

#if SUPPORTS_CHANGE_VIDEOMODE
	int option = 24;
#else
	int option = 0;
#endif
	int option2;
	int seltype;

	/* Current artifacting quality, computed from
	   PLATFORM_artifacting and ANTIC_artif_new */
	int artif_quality;

#if SUPPORTS_PLATFORM_PALETTEUPDATE
	Colours_preset_t colours_preset;
	int i;
	UpdateColourControls(menu_array);
#endif

	for (;;) {
#if NTSC_FILTER
		/* Computing current artifacting quality... */
		if (VIDEOMODE_ntsc_filter) {
			/* NTSC filter is on */
			FindMenuItem(menu_array, 0)->suffix = artif_quality_menu_array[3].item;
			FindMenuItem(menu_array, 11)->suffix = "N/A";
			artif_quality = 3;
		} else
#endif /* NTSC_FILTER */
		if (ANTIC_artif_mode == 0) { /* artifacting is off */
			FindMenuItem(menu_array, 0)->suffix = artif_quality_menu_array[0].item;
			FindMenuItem(menu_array, 11)->suffix = "N/A";
			artif_quality = 0;
		} else { /* ANTIC artifacting is on */
			FindMenuItem(menu_array, 0)->suffix = artif_quality_menu_array[1 + ANTIC_artif_new].item;
			FindMenuItem(menu_array, 11)->suffix = artif_mode_menu_array[ANTIC_artif_mode - 1].item;
			artif_quality = 1 + ANTIC_artif_new;
		}

#if SUPPORTS_PLATFORM_PALETTEUPDATE
		colours_preset = Colours_GetPreset();
		FindMenuItem(menu_array, 12)->suffix = colours_preset_names[colours_preset];

		/* Set the palette file description */
		if (Colours_external->loaded) {
			FindMenuItem(menu_array, 21)->item = Colours_external->filename;
			FindMenuItem(menu_array, 21)->suffix = "Return:load Backspace:remove";
			FindMenuItem(menu_array, 22)->flags = UI_ITEM_CHECK;
			SetItemChecked(menu_array, 22, Colours_external->adjust);
		} else {
			FindMenuItem(menu_array, 21)->item = "None";
			FindMenuItem(menu_array, 21)->suffix = "Return:load";
			FindMenuItem(menu_array, 22)->flags = UI_ITEM_ACTION;
			FindMenuItem(menu_array, 22)->suffix = "N/A";
		}
#endif

#if SUPPORTS_CHANGE_VIDEOMODE && (defined(XEP80_EMULATION) || defined(PBI_PROTO80) || defined(AF80))
		SetItemChecked(menu_array, 25, VIDEOMODE_80_column);
#endif
		snprintf(refresh_status, sizeof(refresh_status), "1:%-2d", Atari800_refresh_rate);
		SetItemChecked(menu_array, 2, Atari800_collisions_in_skipped_frames);
		SetItemChecked(menu_array, 3, Screen_show_atari_speed);
		SetItemChecked(menu_array, 4, Screen_show_disk_led);
		SetItemChecked(menu_array, 5, Screen_show_sector_counter);
		SetItemChecked(menu_array, 26, Screen_show_1200_leds);
#ifdef _WIN32_WCE
		FindMenuItem(menu_array, 6)->flags = filter_available ? (smooth_filter ? (UI_ITEM_CHECK | UI_ITEM_CHECKED) : UI_ITEM_CHECK) : UI_ITEM_HIDDEN;
#endif
#ifdef DREAMCAST
		SetItemChecked(menu_array, 7, db_mode);
		FindMenuItem(menu_array, 8)->suffix = Atari800_tv_mode == Atari800_TV_NTSC ? "NTSC" : "PAL";
		FindMenuItem(menu_array, 9)->suffix = screen_tv_mode == Atari800_TV_NTSC ? "NTSC" : "PAL";
#endif
		option = UI_driver->fSelect("Display Settings", 0, option, menu_array, &seltype);
		switch (option) {
#if SUPPORTS_CHANGE_VIDEOMODE
		case 24:
			VideoModeSettings();
			break;
#endif
		case 0:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, artif_quality, artif_quality_menu_array, NULL);
			if (option2 >= 0)
			{
#if NTSC_FILTER && SUPPORTS_CHANGE_VIDEOMODE
				/* If switched between non-filter and NTSC filter,
				   VIDEOMODE_ntsc_filter must be updated. */
				if (option2 >= 3 && artif_quality < 3)
					VIDEOMODE_SetNtscFilter(TRUE);
				else if (option2 < 3 && artif_quality >= 3)
					VIDEOMODE_SetNtscFilter(FALSE);
#endif /* NTSC_FILTER && SUPPORTS_CHANGE_VIDEOMODE */
				/* ANTIC artifacting settings cannot be turned on
				   when artifacting is off or NTSC filter. */
				if (option2 == 0 || option2 >= 3) {
					ANTIC_artif_new = ANTIC_artif_mode = 0;
				} else {
					/* Do not reset artifacting mode when switched between original and new. */
					if (artif_quality >= 3 || artif_quality == 0)
						/* switched from off/ntsc filter to ANTIC artifacting */
						ANTIC_artif_mode = 1;

					ANTIC_artif_new = option2 - 1;
				}
				ANTIC_UpdateArtifacting();
			}
			break;
		case 11:
			/* The artifacting mode option is only active for ANTIC artifacting. */
			if (artif_quality > 0 && artif_quality < 3)
			{
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, ANTIC_artif_mode - 1, artif_mode_menu_array, NULL);
				if (option2 >= 0) {
					ANTIC_artif_mode = option2 + 1;
					ANTIC_UpdateArtifacting();
				}
			}
			break;
#if SUPPORTS_CHANGE_VIDEOMODE && (defined(XEP80_EMULATION) || defined(PBI_PROTO80) || defined(AF80))
		case 25:
			VIDEOMODE_Toggle80Column();
			if (TRUE
#ifdef XEP80_EMULATION
			    && !XEP80_enabled
#endif /* XEP80_EMULATION */
#ifdef PBI_PROTO80
			    && !PBI_PROTO80_enabled
#endif /* PBI_PROTO80 */
#ifdef AF80
			    && !AF80_enabled
#endif /* AF80 */
			   )
				UI_driver->fMessage("No 80 column hardware available now.", 1);
			break;
#endif /* SUPPORTS_CHANGE_VIDEOMODE && (defined(XEP80_EMULATION) || defined(PBI_PROTO80) || defined(AF80)) */
		case 1:
			Atari800_refresh_rate = UI_driver->fSelectInt(Atari800_refresh_rate, 1, 99);
			break;
		case 2:
			if (Atari800_refresh_rate == 1)
				UI_driver->fMessage("No effect with refresh rate 1", 1);
			Atari800_collisions_in_skipped_frames = !Atari800_collisions_in_skipped_frames;
			break;
		case 3:
			Screen_show_atari_speed = !Screen_show_atari_speed;
			break;
		case 4:
			Screen_show_disk_led = !Screen_show_disk_led;
			break;
		case 5:
			Screen_show_sector_counter = !Screen_show_sector_counter;
			break;
		case 26:
			Screen_show_1200_leds = !Screen_show_1200_leds;
			break;
#ifdef _WIN32_WCE
		case 6:
			smooth_filter = !smooth_filter;
			break;
#endif
#ifdef DREAMCAST
		case 7:
			if (db_mode)
				db_mode = FALSE;
			else if (Atari800_tv_mode == screen_tv_mode)
				db_mode = TRUE;
			update_screen_updater();
			Screen_EntireDirty();
			break;
		case 8:
			Atari800_tv_mode = (Atari800_tv_mode == Atari800_TV_PAL) ? Atari800_TV_NTSC : Atari800_TV_PAL;
			if (Atari800_tv_mode != screen_tv_mode) {
				db_mode = FALSE;
				update_screen_updater();
			}
			update_vidmode();
			Screen_EntireDirty();
			break;
		case 9:
			Atari800_tv_mode = screen_tv_mode = (screen_tv_mode == Atari800_TV_PAL) ? Atari800_TV_NTSC : Atari800_TV_PAL;
			update_vidmode();
			Screen_EntireDirty();
			break;
#ifdef HZ_TEST
		case 10:
			do_hz_test();
			Screen_EntireDirty();
			break;
#endif
#endif /* DREAMCAST */
#if SUPPORTS_PLATFORM_PALETTEUPDATE
		case 12:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, colours_preset, colours_preset_menu_array, NULL);
			if (option2 >= 0) {
				Colours_SetPreset(option2);
				Colours_Update();
				for (i=0; i<6; i++) {
					UpdateColourControl(i);
				}					
			}	
			break;
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
			{
				int index = option - 13;
				int value = UI_driver->fSelectSlider("Adjust value",
								     ColourSettingToSlider(index),
								     100, &ColourSliderLabel, &index);
				if (value != -1) {
					*(colour_controls[index].setting) = SliderToColourSetting(value, index);
					UpdateColourControl(index);
					Colours_Update();
				}
			}
			break;
#if NTSC_FILTER
		case 19:
			if (VIDEOMODE_ntsc_filter) {
				NTSCFilterSettings();
				/* While in NTSC Filter menu, the "Filter preset" option also changes the "standard" colour
				   controls (saturation etc.) - so we need to call UpdateColourControls to update the menu. */
				UpdateColourControls(menu_array);
			}
			else
				UI_driver->fMessage("Available only when NTSC filter is on", 1);
			break;
#endif
		case 20:
			Colours_RestoreDefaults();
#if NTSC_FILTER
			if (VIDEOMODE_ntsc_filter)
				FILTER_NTSC_RestoreDefaults();
#endif
			UpdateColourControls(menu_array);
			Colours_Update();
			break;
		case 21:
			switch (seltype) {
			case UI_USER_SELECT:
				if (UI_driver->fGetLoadFilename(Colours_external->filename, UI_saved_files_dir, UI_n_saved_files_dir)) {
					if (COLOURS_EXTERNAL_Read(Colours_external))
						Colours_Update();
					else
						CantLoad(Colours_external->filename);
				}
				break;
			case UI_USER_DELETE: /* Backspace */
				COLOURS_EXTERNAL_Remove(Colours_external);
				Colours_Update();
				break;
			}
			break;
		case 22:
			if (Colours_external->loaded) {
				Colours_external->adjust = !Colours_external->adjust;
				Colours_Update();
			}
			break;
		case 23:
			SavePalette();
			break;
#endif /* SUPPORTS_PLATFORM_PALETTEUPDATE */
		default:
			return;
		}
	}
}

#ifdef DIRECTX
static void WindowsOptions(void)
{
	static const UI_tMenuItem screen_mode_menu_array[] = {
		UI_MENU_ACTION(0, "Fullscreen"),
		UI_MENU_ACTION(1, "Window"),
		UI_MENU_END
	};
	
	static const UI_tMenuItem display_mode_menu_array[] = {
		UI_MENU_ACTION(0, "GDI"),
		UI_MENU_ACTION(1, "GDI+"),
		UI_MENU_ACTION(2, "GDI+/Bilinear"),
		UI_MENU_ACTION(3, "GDI+/Bilinear(HQ)"),
		UI_MENU_ACTION(4, "GDI+/Bicubic(HQ)"),
		UI_MENU_ACTION(5, "Direct3D"),
		UI_MENU_ACTION(6, "Direct3D/Bilinear"),
		UI_MENU_END
	};
	
	static const UI_tMenuItem window_scale_menu_array[] = {
		UI_MENU_ACTION(0, "100% [320x240]"),
		UI_MENU_ACTION(1, "150% [480x360]"),
		UI_MENU_ACTION(2, "200% [640x480]"),
		UI_MENU_ACTION(3, "250% [800x600]"),
		UI_MENU_ACTION(4, "300% [960x720]"),
		UI_MENU_ACTION(5, "350% [1120x840]"),
		UI_MENU_ACTION(6, "400% [1280x960]"),
		UI_MENU_ACTION(7, "450% [1440x1080]"),
		UI_MENU_ACTION(8, "500% [1600x1200]"),
		UI_MENU_END
	};
	
	static const UI_tMenuItem fsresolution_menu_array[] = {
		UI_MENU_ACTION(0, desktopreslabel),
		UI_MENU_ACTION(1, "VGA     [640x480]   (2x)"),
		UI_MENU_ACTION(2, "SXGA    [1280x960]  (4x)"),
		UI_MENU_ACTION(3, "UXGA    [1600x1200] (5x)"),
		UI_MENU_END
	};
	
	static const UI_tMenuItem scaling_method_menu_array[] = {
		UI_MENU_ACTION(0, "Off"),
		UI_MENU_ACTION(1, "Normal"),
		UI_MENU_ACTION(2, "Simple"),
		UI_MENU_ACTION(3, "Adaptive"),
		UI_MENU_END
	};
	
	static const UI_tMenuItem aspect_mode_menu_array[] = {
		UI_MENU_ACTION(0, "Auto       [7:5/4:3]"),
		UI_MENU_ACTION(1, "Wide       [7:5]"),
		UI_MENU_ACTION(2, "Cropped    [4:3]"),
		UI_MENU_ACTION(3, "Compressed [4:3]"),
		UI_MENU_END
	};
	
	static const UI_tMenuItem scanline_mode_menu_array[] = {
		UI_MENU_ACTION(0, "Off"),
		UI_MENU_ACTION(1, "Low     [1x]"),
		UI_MENU_ACTION(2, "Medium  [2x]"),
		UI_MENU_ACTION(3, "High    [3x]"),
		UI_MENU_END
	};

	static char refresh_status[16];
	static UI_tMenuItem menu_array[] = {
	    UI_MENU_SUBMENU_SUFFIX(0, "Display rendering: ", NULL),
		UI_MENU_SUBMENU_SUFFIX(1, "Screen mode: ", NULL),
		UI_MENU_SUBMENU_SUFFIX(2, "Window scale: ", NULL),
		UI_MENU_SUBMENU_SUFFIX(3, "Fullscreen resolution:", NULL),
		UI_MENU_SUBMENU_SUFFIX(4, "Scaling method:", NULL),
		UI_MENU_SUBMENU_SUFFIX(5, "Aspect mode:", NULL),		
		UI_MENU_ACTION_PREFIX(6, "Horizontal crop: ", native_width_label),		
		UI_MENU_ACTION_PREFIX(7, "Vertical crop:   ", native_height_label),
		UI_MENU_CHECK(8, "Lock aspect mode when cropping:"),
		UI_MENU_SUBMENU_SUFFIX(9, "Horizontal offset: ", NULL),
		UI_MENU_SUBMENU_SUFFIX(10, "Vertical offset: ", NULL),
		UI_MENU_SUBMENU_SUFFIX(11, "Scanline mode:", NULL),
		UI_MENU_CHECK(12, "Hide cursor in fullscreen UI:"),
		UI_MENU_CHECK(13, "Show menu in window mode:"),
		UI_MENU_END
	};

	int option = 0;
	int option2;
	int seltype;
	int prev_value;
	char current_scale[5], trim_value[4], shift_value[4];
	char displaymodename[20];
	int i;

	for (;;) {
		if (rendermode == DIRECTDRAW) {
			for (i = 0; i <= 7; i++) {
				FindMenuItem(menu_array, i)->suffix = "N/A";
			}
		}
		else {
			/*SetDisplayMode(GetActiveDisplayMode());*/
			GetDisplayModeName(displaymodename);
			FindMenuItem(menu_array, 0)->suffix = displaymodename; 
			FindMenuItem(menu_array, 1)->suffix = screen_mode_menu_array[screenmode].item;
			memcpy(current_scale, window_scale_menu_array[(int)((windowscale/100.0f-1)*2)].item, 5);
			current_scale[4] = '\0'; 
			
			FindMenuItem(menu_array, 2)->suffix = current_scale;
			
			if (fsresolution == VGA)
				FindMenuItem(menu_array, 3)->suffix = "VGA";
			else if (fsresolution == SXGA)
				FindMenuItem(menu_array, 3)->suffix = "SXGA";
			else if (fsresolution == UXGA)
				FindMenuItem(menu_array, 3)->suffix = "UXGA";
			else	
				FindMenuItem(menu_array, 3)->suffix = "Desktop";
			
			FindMenuItem(menu_array, 4)->suffix = scaling_method_menu_array[scalingmethod].item;
			
			if (aspectmode == AUTO)
				FindMenuItem(menu_array, 5)->suffix = "Auto";
			else if (aspectmode == WIDE)
				FindMenuItem(menu_array, 5)->suffix = "Wide";
			else if (aspectmode == CROPPED)
				FindMenuItem(menu_array, 5)->suffix = "Cropped";
			else if (aspectmode == COMPRESSED)
				FindMenuItem(menu_array, 5)->suffix = "Compressed";

			snprintf(hcrop_label, sizeof(hcrop_label), "%d", crop.horizontal);
			snprintf(vcrop_label, sizeof(vcrop_label), "%d", crop.vertical);
		    FindMenuItem(menu_array, 6)->suffix = hcrop_label;
			FindMenuItem(menu_array, 7)->suffix = vcrop_label; 
			
			SetItemChecked(menu_array, 8, lockaspect);
			snprintf(hshift_label, sizeof(hshift_label), "%d", offset.horizontal);
			snprintf(vshift_label, sizeof(vshift_label), "%d", offset.vertical);
			FindMenuItem(menu_array, 9)->suffix = hshift_label;
			FindMenuItem(menu_array, 10)->suffix = vshift_label;
			
			if (frameparams.scanlinemode == NONE)
				FindMenuItem(menu_array, 11)->suffix = "Off";
			else if (frameparams.scanlinemode == LOW)
				FindMenuItem(menu_array, 11)->suffix = "Low";
			else if (frameparams.scanlinemode == MEDIUM)
				FindMenuItem(menu_array, 11)->suffix = "Medium";
			else if (frameparams.scanlinemode == HIGH)
				FindMenuItem(menu_array, 11)->suffix = "High";
				
			SetItemChecked(menu_array, 12, hidecursor);
			SetItemChecked(menu_array, 13, showmenu);
		}

		option = UI_driver->fSelect("Windows Display Options", 0, option, menu_array, &seltype);
		switch (option) {

		case 0:
			if (rendermode != DIRECTDRAW) {
				prev_value = displaymode;
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, displaymode, display_mode_menu_array, NULL);
				if (option2 >= 0) {
					displaymode = option2;					
					if (prev_value != option2)
						UI_driver->fMessage("Save the config and restart emulator", 1);
				}
			}
			break;
		case 1:
			if (rendermode != DIRECTDRAW) {
				prev_value = screenmode;
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, screenmode, screen_mode_menu_array, NULL);
				if (option2 >= 0)
					if (prev_value != option2)
						togglewindowstate();
			}
			break;
		case 2:
			if (rendermode != DIRECTDRAW) {
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, (int)((windowscale/100.0f-1)*2), window_scale_menu_array, NULL);
				if (option2 >= 0) {					
					changewindowsize(SET, (int)((option2/2.0f+1)*100));
					prev_value = windowscale;
					windowscale = (int)((option2/2.0f+1)*100);
					if (windowscale != prev_value) {
						if (screenmode == WINDOW)
							UI_driver->fMessage("Cannot display at this size", 1);
						else
							UI_driver->fMessage("Cannot preview in fullscreen mode", 1);
					}
				}
			}
			break;
		case 3:
			if (rendermode != DIRECTDRAW) {
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, fsresolution, fsresolution_menu_array, NULL);
				if (option2 >= 0)
					fsresolution = option2;
				if (fsresolution == DESKTOP)
					usecustomfsresolution = FALSE;
				else if (fsresolution == UXGA) {
					usecustomfsresolution = TRUE;
					fullscreenWidth = 1600;
					fullscreenHeight = 1200;
				}
				else if (fsresolution == SXGA) {
					usecustomfsresolution = TRUE;
					fullscreenWidth = 1280;
					fullscreenHeight = 960;
				}
				else if (fsresolution == VGA) {
					usecustomfsresolution = TRUE;
					fullscreenWidth = 640;
					fullscreenHeight = 480;
				}
			}
			break;
		case 4:
			if (rendermode != DIRECTDRAW) {
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, scalingmethod, scaling_method_menu_array, NULL);
				if (option2 >= 0) {
					scalingmethod = option2;
					changewindowsize(RESET, 0);
					refreshframe();
				}
			}
			break;
		case 5:
			if (rendermode != DIRECTDRAW) {
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, aspectmode, aspect_mode_menu_array, NULL);
				if (option2 >= 0) {
					aspectmode = option2;					
					changewindowsize(RESET, 0);
					refreshframe();
					PLATFORM_DisplayScreen(); /* force rebuild of the clipping frame */
					snprintf(native_height_label, sizeof(native_height_label), "[Height: %d]", frameparams.view.bottom - frameparams.view.top);
					snprintf(native_width_label, sizeof(native_width_label), "[Width:  %d]", frameparams.view.right - frameparams.view.left);
				}
			}
			break;
		case 6:
			if (rendermode != DIRECTDRAW)  {
				snprintf(trim_value, sizeof(trim_value), "%d", crop.horizontal);
				if (UI_driver->fEditString("Enter value", trim_value, sizeof(trim_value))) {
					if (atoi(trim_value) > 150) 
						UI_driver->fMessage("Maximum X-Trim value is 150", 1);
					else if (atoi(trim_value) < -24) 
						UI_driver->fMessage("Minimum X-Trim value is -24", 1);
					else {
						crop.horizontal = atoi(trim_value);	
						changewindowsize(RESET, 0);
						refreshframe();
						PLATFORM_DisplayScreen(); /* force rebuild of the clipping frame */
						snprintf(native_width_label, sizeof(native_width_label), "[Width:  %d]", frameparams.view.right - frameparams.view.left);
					}
				}
			}
			break;
		case 7:
			if (rendermode != DIRECTDRAW)  {
				snprintf(trim_value, sizeof(trim_value), "%d", crop.vertical);
				if (UI_driver->fEditString("Enter value", trim_value, sizeof(trim_value))) {
					if (atoi(trim_value) < 0) 
						UI_driver->fMessage("Minimum Y-Trim value is 0", 1);
					else if (atoi(trim_value) > 108)
						UI_driver->fMessage("Maximum Y-Trim value is 108", 1);
					else {
						crop.vertical = atoi(trim_value);
						changewindowsize(RESET, 0);
						refreshframe();
						PLATFORM_DisplayScreen(); /* force rebuild of the clipping frame */
						snprintf(native_height_label, sizeof(native_height_label), "[Height: %d]", frameparams.view.bottom - frameparams.view.top);
					}
				}
			}
			break;
		case 8:
			if (rendermode != DIRECTDRAW) {
				lockaspect = !lockaspect;
				changewindowsize(RESET, 0);
				refreshframe();
			}
			break;
		case 9:
			if (rendermode != DIRECTDRAW)  {
				snprintf(shift_value, sizeof(shift_value), "%d", offset.horizontal);
				if (UI_driver->fEditString("Enter value", shift_value, sizeof(shift_value))) {
					if (atoi(shift_value) > 24) 
						UI_driver->fMessage("Maximum horizontal offset is 24", 1);
					else if (atoi(shift_value) < -24) 
						UI_driver->fMessage("Minimum horizontal offset is -24", 1);
					else {
						offset.horizontal = atoi(shift_value);						
						changewindowsize(RESET, 0);
						refreshframe();
					}
				}
			}
			break;
		case 10:
			if (rendermode != DIRECTDRAW)  {
				snprintf(shift_value, sizeof(shift_value), "%d", offset.vertical);
				if (UI_driver->fEditString("Enter value", shift_value, sizeof(shift_value))) {
					if (atoi(shift_value) > 50) 
						UI_driver->fMessage("Maximum vertical offset is 50", 1);
					else if (atoi(shift_value) < -50) 
						UI_driver->fMessage("Minimum vertical offset is 50", 1);
					else {
						offset.vertical = atoi(shift_value);					
						changewindowsize(RESET, 0);
						refreshframe();
					}
				}
			}
			break;
		case 11:
			if (rendermode != DIRECTDRAW)  {
				option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, frameparams.scanlinemode, scanline_mode_menu_array, NULL);
				if (option2 >= 0) {
					frameparams.scanlinemode = option2;
					refreshframe();
				}
			}
			break;
		case 12:
			if (rendermode != DIRECTDRAW) {
				hidecursor = !hidecursor;
				setcursor();
			}
			break;
		case 13:
			if (rendermode != DIRECTDRAW) {
				togglemenustate();
			}
			break;
			
		default:
			return;
		}
	}
}
#endif /* DIRECTX */

#endif /* CURSES_BASIC */

#ifndef USE_CURSES

#ifdef GUI_SDL
static char joys[2][5][16];
static const UI_tMenuItem joy0_menu_array[] = {
	UI_MENU_LABEL("Select joy direction"),
	UI_MENU_LABEL("\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022"),
	UI_MENU_SUBMENU_SUFFIX(0, "Left   : ", joys[0][0]),
	UI_MENU_SUBMENU_SUFFIX(1, "Up     : ", joys[0][1]),
	UI_MENU_SUBMENU_SUFFIX(2, "Right  : ", joys[0][2]),
	UI_MENU_SUBMENU_SUFFIX(3, "Down   : ", joys[0][3]),
	UI_MENU_SUBMENU_SUFFIX(4, "Trigger: ", joys[0][4]),
	UI_MENU_END
};
static const UI_tMenuItem joy1_menu_array[] = {
	UI_MENU_LABEL("Select joy direction"),
	UI_MENU_LABEL("\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022"),
	UI_MENU_SUBMENU_SUFFIX(0, "Left   : ", joys[1][0]),
	UI_MENU_SUBMENU_SUFFIX(1, "Up     : ", joys[1][1]),
	UI_MENU_SUBMENU_SUFFIX(2, "Right  : ", joys[1][2]),
	UI_MENU_SUBMENU_SUFFIX(3, "Down   : ", joys[1][3]),
	UI_MENU_SUBMENU_SUFFIX(4, "Trigger: ", joys[1][4]),
	UI_MENU_END
};

static void KeyboardJoystickConfiguration(int joystick)
{
	char title[40];
	int option2 = 0;
	snprintf(title, sizeof(title), "Define keys for joystick %d", joystick);
	for(;;) {
		int j0d;
		for(j0d = 0; j0d <= 4; j0d++)
			PLATFORM_GetJoystickKeyName(joystick, j0d, joys[joystick][j0d], sizeof(joys[joystick][j0d]));
		option2 = UI_driver->fSelect(title, UI_SELECT_POPUP, option2, joystick == 0 ? joy0_menu_array : joy1_menu_array, NULL);
		if (option2 >= 0 && option2 <= 4) {
			PLATFORM_SetJoystickKey(joystick, option2, GetRawKey());
		}
		if (option2 < 0) break;
		if (++option2 > 4) option2 = 0;
	}
}
#endif

#ifdef DIRECTX
static char buttons[9][2][16];
static const UI_tMenuItem joy0_menu_array[] = {
	UI_MENU_LABEL("Select button to map"),
	UI_MENU_LABEL("\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022"),
	UI_MENU_SUBMENU_SUFFIX(0, "Button 2  : ", buttons[0][0]),
	UI_MENU_SUBMENU_SUFFIX(1, "Button 3  : ", buttons[0][1]),
	UI_MENU_SUBMENU_SUFFIX(2, "Button 4  : ", buttons[0][2]),
	UI_MENU_SUBMENU_SUFFIX(3, "Button 5  : ", buttons[0][3]),
	UI_MENU_SUBMENU_SUFFIX(4, "Button 6  : ", buttons[0][4]),
	UI_MENU_SUBMENU_SUFFIX(5, "Button 7  : ", buttons[0][5]),
	UI_MENU_SUBMENU_SUFFIX(6, "Button 8  : ", buttons[0][6]),
	UI_MENU_SUBMENU_SUFFIX(7, "Button 9  : ", buttons[0][7]),
	UI_MENU_SUBMENU_SUFFIX(8, "Button 10 : ", buttons[0][8]),
	UI_MENU_END
};
static const UI_tMenuItem joy1_menu_array[] = {
	UI_MENU_LABEL("Select button to map"),
	UI_MENU_LABEL("\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022\022"),
	UI_MENU_SUBMENU_SUFFIX(0, "Button 2  : ", buttons[1][0]),
	UI_MENU_SUBMENU_SUFFIX(1, "Button 3  : ", buttons[1][1]),
	UI_MENU_SUBMENU_SUFFIX(2, "Button 4  : ", buttons[1][2]),
	UI_MENU_SUBMENU_SUFFIX(3, "Button 5  : ", buttons[1][3]),
	UI_MENU_SUBMENU_SUFFIX(4, "Button 6  : ", buttons[1][4]),
	UI_MENU_SUBMENU_SUFFIX(5, "Button 7  : ", buttons[1][5]),
	UI_MENU_SUBMENU_SUFFIX(6, "Button 8  : ", buttons[1][6]),
	UI_MENU_SUBMENU_SUFFIX(7, "Button 9  : ", buttons[1][7]),
	UI_MENU_SUBMENU_SUFFIX(8, "Button 10 : ", buttons[1][8]),
	UI_MENU_END
};

static void ConfigureControllerButtons(int stick)
{
	int i;
	char title[40];
	int option2 = 0;
	
	snprintf(title, sizeof(title), "Define keys for controller %d", stick + 1);
	for(;;) {
		for(i = 0; i <= 8; i++) 
			PLATFORM_GetButtonAssignments(stick, i, buttons[stick][i], sizeof(buttons[stick][i]));
		option2 = UI_driver->fSelect(title, UI_SELECT_POPUP, option2, stick == 0 ? joy0_menu_array : joy1_menu_array, NULL);
		if (option2 >= 0 && option2 <= 8)
			PLATFORM_SetButtonAssignment(stick, option2, GetKeyName());
		if (option2 < 0) break;
		if (++option2 > 8) option2 = 0;
	}
}
#endif

static void ControllerConfiguration(void)
{
#ifdef DIRECTX
	static const UI_tMenuItem keyboard_joystick_mode_array[] = {
		UI_MENU_ACTION(0, "Keypad"),
		UI_MENU_ACTION(1, "Keypad+"),
		UI_MENU_ACTION(2, "Arrows"),
		UI_MENU_END
	};
	
	static const UI_tMenuItem alternate_joystick_mode_array[] = {
		UI_MENU_ACTION(0, "Normal"),
		UI_MENU_ACTION(1, "Dual"),
		UI_MENU_ACTION(2, "Shared"),
		UI_MENU_END
	};
#endif

#if !defined(_WIN32_WCE) && !defined(DREAMCAST)
	static const UI_tMenuItem mouse_mode_menu_array[] = {
		UI_MENU_ACTION(0, "None"),
		UI_MENU_ACTION(1, "Paddles"),
		UI_MENU_ACTION(2, "Touch tablet"),
		UI_MENU_ACTION(3, "Koala pad"),
		UI_MENU_ACTION(4, "Light pen"),
		UI_MENU_ACTION(5, "Light gun"),
		UI_MENU_ACTION(6, "Amiga mouse"),
		UI_MENU_ACTION(7, "ST mouse"),
		UI_MENU_ACTION(8, "Trak-ball"),
		UI_MENU_ACTION(9, "Joystick"),
		UI_MENU_END
	};
	static char mouse_port_status[2] = { '1', '\0' };
	static char mouse_speed_status[2] = { '1', '\0' };
#endif
	static UI_tMenuItem menu_array[] = {
		UI_MENU_ACTION(0, "Joystick autofire:"),
		UI_MENU_CHECK(1, "Enable MultiJoy4:"),
#if defined(_WIN32_WCE)
		UI_MENU_CHECK(5, "Virtual joystick:"),
#elif defined(DREAMCAST)
		UI_MENU_CHECK(9, "Emulate Paddles:"),
		UI_MENU_ACTION(10, "Button configuration"),
#else
		UI_MENU_SUBMENU_SUFFIX(2, "Mouse device: ", NULL),
		UI_MENU_SUBMENU_SUFFIX(3, "Mouse port:", mouse_port_status),
		UI_MENU_SUBMENU_SUFFIX(4, "Mouse speed:", mouse_speed_status),
#endif
#ifdef GUI_SDL
		UI_MENU_CHECK(5, "Enable keyboard joystick 1:"),
		UI_MENU_SUBMENU(6, "Define layout of keyboard joystick 1"),
		UI_MENU_CHECK(7, "Enable keyboard joystick 2:"),
		UI_MENU_SUBMENU(8, "Define layout of keyboard joystick 2"),
#endif
#ifdef DIRECTX
		UI_MENU_SUBMENU_SUFFIX(5, "Keyboard joystick mode: ", NULL),
		UI_MENU_SUBMENU_SUFFIX(6, "Alternate joystick mode: ", NULL),
		UI_MENU_CHECK(7, "Enable custom buttons (joy 1):"),
		UI_MENU_SUBMENU(8, "Assign custom buttons (joy 1):"),
		UI_MENU_CHECK(9, "Enable custom buttons (joy 2):"),
		UI_MENU_SUBMENU(10, "Assign custom buttons (joy 2):"),
#endif
		UI_MENU_END
	};

	int option = 0;
#if !defined(_WIN32_WCE) && !defined(DREAMCAST)
	int option2;
#endif
#ifdef DIRECTX
    int prev_option;
#endif
	for (;;) {
		menu_array[0].suffix = INPUT_joy_autofire[0] == INPUT_AUTOFIRE_FIRE ? "Fire"
		                     : INPUT_joy_autofire[0] == INPUT_AUTOFIRE_CONT ? "Always"
		                     : "No ";
		SetItemChecked(menu_array, 1, INPUT_joy_multijoy);
#if defined(_WIN32_WCE)
		/* XXX: not on smartphones? */
		SetItemChecked(menu_array, 5, virtual_joystick);
#elif defined(DREAMCAST)
		SetItemChecked(menu_array, 9, emulate_paddles);
#else
		menu_array[2].suffix = mouse_mode_menu_array[INPUT_mouse_mode].item;
		mouse_port_status[0] = (char) ('1' + INPUT_mouse_port);
		mouse_speed_status[0] = (char) ('0' + INPUT_mouse_speed);
#endif
#ifdef GUI_SDL
		SetItemChecked(menu_array, 5, PLATFORM_kbd_joy_0_enabled);
		SetItemChecked(menu_array, 7, PLATFORM_kbd_joy_1_enabled);
#endif
#ifdef DIRECTX
		menu_array[5].suffix = keyboard_joystick_mode_array[keyboardJoystickMode].item;
		menu_array[6].suffix = alternate_joystick_mode_array[alternateJoystickMode].item;
		SetItemChecked(menu_array, 7, mapController1Buttons);
		SetItemChecked(menu_array, 9, mapController2Buttons);
#endif
		option = UI_driver->fSelect("Controller Configuration", 0, option, menu_array, NULL);
		switch (option) {
		case 0:
			switch (INPUT_joy_autofire[0]) {
			case INPUT_AUTOFIRE_FIRE:
				INPUT_joy_autofire[0] = INPUT_joy_autofire[1] = INPUT_joy_autofire[2] = INPUT_joy_autofire[3] = INPUT_AUTOFIRE_CONT;
				break;
			case INPUT_AUTOFIRE_CONT:
				INPUT_joy_autofire[0] = INPUT_joy_autofire[1] = INPUT_joy_autofire[2] = INPUT_joy_autofire[3] = INPUT_AUTOFIRE_OFF;
				break;
			default:
				INPUT_joy_autofire[0] = INPUT_joy_autofire[1] = INPUT_joy_autofire[2] = INPUT_joy_autofire[3] = INPUT_AUTOFIRE_FIRE;
				break;
			}
			break;
		case 1:
			INPUT_joy_multijoy = !INPUT_joy_multijoy;
			break;
#if defined(_WIN32_WCE)
		case 5:
			virtual_joystick = !virtual_joystick;
			break;
#elif defined(DREAMCAST)
		case 9:
			emulate_paddles = !emulate_paddles;
			break;
		case 10:
			ButtonConfiguration();
			break;
#else
		case 2:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, INPUT_mouse_mode, mouse_mode_menu_array, NULL);
			if (option2 >= 0)
				INPUT_mouse_mode = option2;
			break;
		case 3:
			INPUT_mouse_port = UI_driver->fSelectInt(INPUT_mouse_port + 1, 1, 4) - 1;
			break;
		case 4:
			INPUT_mouse_speed = UI_driver->fSelectInt(INPUT_mouse_speed, 1, 9);
			break;
#endif
#ifdef GUI_SDL
		case 5:
			PLATFORM_kbd_joy_0_enabled = !PLATFORM_kbd_joy_0_enabled;
			break;
		case 6:
			KeyboardJoystickConfiguration(0);
			break;
		case 7:
			PLATFORM_kbd_joy_1_enabled = !PLATFORM_kbd_joy_1_enabled;
			break;
		case 8:
			KeyboardJoystickConfiguration(1);
			break;
#endif
#ifdef DIRECTX
		case 5:
		    prev_option = keyboardJoystickMode;
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, keyboardJoystickMode, keyboard_joystick_mode_array, NULL);
			if (option2 >= 0)
				keyboardJoystickMode = option2;
			break;
		case 6:
			option2 = UI_driver->fSelect(NULL, UI_SELECT_POPUP, alternateJoystickMode, alternate_joystick_mode_array, NULL);
			if (option2 >= 0)
				alternateJoystickMode = option2;
			break;
		case 7:
			mapController1Buttons = !mapController1Buttons;
			break;
		case 8:
			ConfigureControllerButtons(0);
			break;
		case 9:
			mapController2Buttons = !mapController2Buttons;
			break;
		case 10:
			ConfigureControllerButtons(1);
			break;
#endif
		default:
			return;
		}
	}
}

#endif /* USE_CURSES */

#ifdef SOUND

static int SoundSettings(void)
{
	static UI_tMenuItem menu_array[] = {
		/* XXX: don't allow on smartphones? */
#ifndef SYNCHRONIZED_SOUND
		UI_MENU_CHECK(0, "High Fidelity POKEY:"),
#endif
#ifdef STEREO_SOUND
		UI_MENU_CHECK(1, "Dual POKEY (Stereo):"),
#endif
#ifdef CONSOLE_SOUND
		UI_MENU_CHECK(2, "Speaker (Key Click):"),
#endif
#ifdef SERIO_SOUND
		UI_MENU_CHECK(3, "Serial IO Sound:"),
#endif
#ifndef SYNCHRONIZED_SOUND
		UI_MENU_ACTION(4, "Enable higher frequencies:"),
#endif
#ifdef DREAMCAST
		UI_MENU_CHECK(5, "Enable sound:"),
#endif
		UI_MENU_END
	};

	int option = 0;

	for (;;) {
#ifndef SYNCHRONIZED_SOUND
		SetItemChecked(menu_array, 0, POKEYSND_enable_new_pokey);
#endif
#ifdef STEREO_SOUND
		SetItemChecked(menu_array, 1, POKEYSND_stereo_enabled);
#endif
#ifdef CONSOLE_SOUND
		SetItemChecked(menu_array, 2, POKEYSND_console_sound_enabled);
#endif
#ifdef SERIO_SOUND
		SetItemChecked(menu_array, 3, POKEYSND_serio_sound_enabled);
#endif
#ifndef SYNCHRONIZED_SOUND
		FindMenuItem(menu_array, 4)->suffix = POKEYSND_enable_new_pokey ? "N/A" : POKEYSND_bienias_fix ? "Yes" : "No ";
#endif
#ifdef DREAMCAST
		SetItemChecked(menu_array, 5, glob_snd_ena);
#endif

#if 0
		option = UI_driver->fSelect(NULL, UI_SELECT_POPUP, option, menu_array, NULL);
#else
		option = UI_driver->fSelect("Sound Settings", 0, option, menu_array, NULL);
#endif

		switch (option) {
#ifndef SYNCHRONIZED_SOUND
		case 0:
			POKEYSND_enable_new_pokey = !POKEYSND_enable_new_pokey;
			POKEYSND_DoInit();
			/* According to the PokeySnd doc the POKEY switch can occur on
			   a cold-restart only */
			UI_driver->fMessage("Will reboot to apply the change", 1);
			return TRUE; /* reboot required */
#endif
#ifdef STEREO_SOUND
		case 1:
			POKEYSND_stereo_enabled = !POKEYSND_stereo_enabled;
#ifdef SUPPORTS_SOUND_REINIT
			Sound_Reinit();
#endif
			break;
#endif
#ifdef CONSOLE_SOUND
		case 2:
			POKEYSND_console_sound_enabled = !POKEYSND_console_sound_enabled;
			break;
#endif
#ifdef SERIO_SOUND
		case 3:
			POKEYSND_serio_sound_enabled = !POKEYSND_serio_sound_enabled;
			break;
#endif
#ifndef SYNCHRONIZED_SOUND
		case 4:
			if (! POKEYSND_enable_new_pokey) POKEYSND_bienias_fix = !POKEYSND_bienias_fix;
			break;
#endif
#ifdef DREAMCAST
		case 5:
			glob_snd_ena = !glob_snd_ena;
			break;
#endif
		default:
			return FALSE;
		}
	}
}

#endif /* SOUND */

#if !defined(CURSES_BASIC) && !defined(DREAMCAST)

static void Screenshot(int interlaced)
{
	static char filename[FILENAME_MAX];
	if (UI_driver->fGetSaveFilename(filename, UI_saved_files_dir, UI_n_saved_files_dir)) {
#ifdef USE_CURSES
		/* must clear, otherwise in case of a failure we'll see parts
		   of Atari screen on top of UI screen */
		curses_clear_screen();
#endif
		ANTIC_Frame(TRUE);
		if (!Screen_SaveScreenshot(filename, interlaced))
			CantSave(filename);
	}
}

#endif /* !defined(CURSES_BASIC) && !defined(DREAMCAST) */

static void AboutEmulator(void)
{
	UI_driver->fInfoScreen("About the Emulator",
		Atari800_TITLE "\0"
		"Copyright (c) 1995-1998 David Firth\0"
		"and\0"
		"(c)1998-2010 Atari800 Development Team\0"
		"See CREDITS file for details.\0"
		"http://atari800.atari.org/\0"
		"\0"
		"\0"
		"\0"
		"This program is free software; you can\0"
		"redistribute it and/or modify it under\0"
		"the terms of the GNU General Public\0"
		"License as published by the Free\0"
		"Software Foundation; either version 2,\0"
		"or (at your option) any later version.\0"
		"\n");
}

#ifdef DIRECTX
static void FunctionKeyHelp(void)
{
	UI_driver->fInfoScreen("Function Key List",
		Atari800_TITLE "\0"
		"\0"
		"Function Key Assignments   \0"
		"------------------------   \0"
		"\0"
		"F1  - User Interface       \0"
		"F2  - Option key           \0"
		"F3  - Select key           \0"
		"F4  - Start key            \0"
		"F5  - Reset key            \0"
		"F6  - Help key (XL/XE only)\0"
		"F7  - Break key            \0"
		"F8  - Enter monitor        \0"
		"      (-console required)  \0"
		"F9  - Exit emulator        \0"
		"F10 - Save screenshot      \0"
		"\n");
}

static void HotKeyHelp(void)
{
	UI_driver->fInfoScreen("Hot Key List",
		Atari800_TITLE "\0"
		"\0"
		"Hot Key Assignments \0"
		"------------------- \0"
		"\0"
		"Alt+Enter   - Toggle Fullscreen/Window\0"
		"Alt+PgUp    - Increase window size    \0"
		"Alt+PgDn    - Decrease window size    \0"
		"Alt+I       - Next scanline mode      \0"
		"Alt+M       - Hide/Show main menu     \0"
		"Alt+Shift+Z - 3D Tilt                 \0"
		"              (Direct3D modes only)   \0"
		"Alt+Z       - 3D Screensaver          \0"
		"              (Direct3D modes only)   \0"
		"\n");
}
#endif

void UI_Run(void)
{
	static UI_tMenuItem menu_array[] = {
		UI_MENU_FILESEL_ACCEL(UI_MENU_RUN, "Run Atari Program", "Alt+R"),
		UI_MENU_SUBMENU_ACCEL(UI_MENU_DISK, "Disk Management", "Alt+D"),
		UI_MENU_SUBMENU_ACCEL(UI_MENU_CARTRIDGE, "Cartridge Management", "Alt+C"),
		UI_MENU_SUBMENU_ACCEL(UI_MENU_CASSETTE, "Tape Management", "Alt+T"),
		UI_MENU_SUBMENU_ACCEL(UI_MENU_SYSTEM, "System Settings", "Alt+Y"),
#ifdef SOUND
		UI_MENU_SUBMENU_ACCEL(UI_MENU_SOUND, "Sound Settings", "Alt+O"),
#ifndef DREAMCAST
		UI_MENU_ACTION_ACCEL(UI_MENU_SOUND_RECORDING, "Sound Recording Start/Stop", "Alt+W"),
#endif
#endif
#ifndef CURSES_BASIC
		UI_MENU_SUBMENU(UI_MENU_DISPLAY, "Display Settings"),
#endif
#ifdef DIRECTX
		UI_MENU_SUBMENU(UI_MENU_WINDOWS, "Windows Display Options"),
#endif
#ifndef USE_CURSES
		UI_MENU_SUBMENU(UI_MENU_CONTROLLER, "Controller Configuration"),
#endif
		UI_MENU_SUBMENU(UI_MENU_SETTINGS, "Emulator Configuration"),
		UI_MENU_FILESEL_ACCEL(UI_MENU_SAVESTATE, "Save State", "Alt+S"),
		UI_MENU_FILESEL_ACCEL(UI_MENU_LOADSTATE, "Load State", "Alt+L"),
#if !defined(CURSES_BASIC) && !defined(DREAMCAST)
#ifdef HAVE_LIBPNG
		UI_MENU_FILESEL_ACCEL(UI_MENU_PCX, "Save Screenshot", "F10"),
		/* there isn't enough space for "PNG/PCX Interlaced Screenshot Shift+F10" */
		UI_MENU_FILESEL_ACCEL(UI_MENU_PCXI, "Save Interlaced Screenshot", "Shift+F10"),
#else
		UI_MENU_FILESEL_ACCEL(UI_MENU_PCX, "PCX Screenshot", "F10"),
		UI_MENU_FILESEL_ACCEL(UI_MENU_PCXI, "PCX Interlaced Screenshot", "Shift+F10"),
#endif
#endif
		UI_MENU_ACTION_ACCEL(UI_MENU_BACK, "Back to Emulated Atari", "Esc"),
		UI_MENU_ACTION_ACCEL(UI_MENU_RESETW, "Reset (Warm Start)", "F5"),
		UI_MENU_ACTION_ACCEL(UI_MENU_RESETC, "Reboot (Cold Start)", "Shift+F5"),
#if defined(_WIN32_WCE)
		UI_MENU_ACTION(UI_MENU_MONITOR, "About Pocket Atari"),
#elif defined(DREAMCAST)
		UI_MENU_ACTION(UI_MENU_MONITOR, "About AtariDC"),
#elif defined(DIRECTX)
		UI_MENU_ACTION_ACCEL(UI_MENU_MONITOR, monitor_label, "F8"),
#else
		UI_MENU_ACTION_ACCEL(UI_MENU_MONITOR, "Enter Monitor", "F8"),
#endif
		UI_MENU_ACTION_ACCEL(UI_MENU_ABOUT, "About the Emulator", "Alt+A"),
		UI_MENU_ACTION_ACCEL(UI_MENU_EXIT, "Exit Emulator", "F9"),
		UI_MENU_END
	};

	int option = UI_MENU_RUN;
	int done = FALSE;
#if SUPPORTS_CHANGE_VIDEOMODE
	VIDEOMODE_ForceStandardScreen(TRUE);
#endif

	UI_is_active = TRUE;

#ifdef DIRECTX
	setcursor();
	snprintf(desktopreslabel, sizeof(desktopreslabel), "Desktop [%dx%d]", origScreenWidth, origScreenHeight);
	snprintf(hcrop_label, sizeof(hcrop_label), "%d", crop.horizontal);
	snprintf(vcrop_label, sizeof(vcrop_label), "%d", crop.vertical);
	snprintf(hshift_label, sizeof(hshift_label), "%d", offset.horizontal);
	snprintf(vshift_label, sizeof(vshift_label), "%d", offset.vertical);
	snprintf(native_width_label, sizeof(native_width_label), "[Width:  %d]", frameparams.view.right - frameparams.view.left);
	snprintf(native_height_label, sizeof(native_height_label), "[Height: %d]", frameparams.view.bottom - frameparams.view.top);
	if (useconsole)
		strcpy(monitor_label, "Enter Monitor");
	else
		strcpy(monitor_label, "Enter Monitor (need -console)"); 
#endif
	

	/* Sound_Active(FALSE); */
	UI_driver->fInit();

#ifdef CRASH_MENU
	if (UI_crash_code >= 0) {
		done = CrashMenu();
		UI_crash_code = -1;
	}
#endif

	while (!done) {
		
		if (UI_alt_function != -1)
			UI_current_function = UI_alt_function;		
		if (UI_alt_function < 0)
			option = UI_driver->fSelect(Atari800_TITLE, 0, option, menu_array, NULL);
		if (UI_alt_function >= 0) {
			option = UI_alt_function;
			UI_alt_function = -1;
			done = TRUE;
		}

		switch (option) {
		case -2:
		case -1:				/* ESC key */
			done = TRUE;
			break;
		case UI_MENU_DISK:
			DiskManagement();
			break;
		case UI_MENU_CARTRIDGE:
			CartManagement();
			break;
		case UI_MENU_RUN:
			/* if (RunExe()) */
			if (AutostartFile())
				done = TRUE;	/* reboot immediately */
			break;
		case UI_MENU_CASSETTE:
			TapeManagement();
			break;
		case UI_MENU_SYSTEM:
			SystemSettings();
			break;
		case UI_MENU_SETTINGS:
			AtariSettings();
			break;
#ifdef SOUND
		case UI_MENU_SOUND:
			if (SoundSettings()) {
				Atari800_Coldstart();
				done = TRUE;	/* reboot immediately */
			}
			break;
#ifndef DREAMCAST
		case UI_MENU_SOUND_RECORDING:
			SoundRecording();
			break;
#endif
#endif
		case UI_MENU_SAVESTATE:
			SaveState();
			break;
		case UI_MENU_LOADSTATE:
			/* Note: AutostartFile() handles state files, too,
			   so we can remove LoadState() now. */
			LoadState();
			break;
#ifndef CURSES_BASIC
		case UI_MENU_DISPLAY:
			DisplaySettings();
			break;
#ifndef DREAMCAST
		case UI_MENU_PCX:
			Screenshot(FALSE);
			break;
		case UI_MENU_PCXI:
			Screenshot(TRUE);
			break;
#endif
#endif
#ifdef DIRECTX
		case UI_MENU_WINDOWS:
			WindowsOptions();
			break;
		case UI_MENU_SAVE_CONFIG:
			CFG_WriteConfig();
			return;
#endif
#ifndef USE_CURSES
		case UI_MENU_CONTROLLER:
			ControllerConfiguration();
			break;
#endif
		case UI_MENU_BACK:
			done = TRUE;		/* back to emulator */
			break;
		case UI_MENU_RESETW:
			Atari800_Warmstart();
			done = TRUE;		/* reboot immediately */
			break;
		case UI_MENU_RESETC:
			Atari800_Coldstart();
			done = TRUE;		/* reboot immediately */
			break;
		case UI_MENU_ABOUT:
			AboutEmulator();
			break;
#ifdef DIRECTX
		case UI_MENU_FUNCT_KEY_HELP:
			FunctionKeyHelp();
			break;
		case UI_MENU_HOT_KEY_HELP:
			HotKeyHelp();
			break;
#endif
		case UI_MENU_MONITOR:
#if defined(_WIN32_WCE)
			AboutPocketAtari();
			break;
#elif defined(DREAMCAST)
			AboutAtariDC();
			break;
#else
#if defined(DIRECTX)
			if (!useconsole) {
				UI_driver->fMessage("Console required for monitor", 1);
				break;
			}
#endif /* DIRECTX */
			if (Atari800_Exit(TRUE))
				break;
			/* if 'quit' typed in monitor, exit emulator */
			exit(0);
#endif
		case UI_MENU_EXIT:
			Atari800_Exit(FALSE);
			exit(0);
		}
	}

	/* Sound_Active(TRUE); */
	UI_is_active = FALSE;
#ifdef DIRECTX
	setcursor();
#endif
	
	/* flush keypresses */
	while (PLATFORM_Keyboard() != AKEY_NONE)
		Atari800_Sync();

	UI_alt_function = UI_current_function = -1;
	/* restore 80 column screen */
#if SUPPORTS_CHANGE_VIDEOMODE
	VIDEOMODE_ForceStandardScreen(FALSE);
#endif
}


#ifdef CRASH_MENU

int CrashMenu(void)
{
	static char cim_info[42];
	static UI_tMenuItem menu_array[] = {
		UI_MENU_LABEL(cim_info),
		UI_MENU_ACTION_ACCEL(0, "Reset (Warm Start)", "F5"),
		UI_MENU_ACTION_ACCEL(1, "Reboot (Cold Start)", "Shift+F5"),
		UI_MENU_ACTION_ACCEL(2, "Menu", "F1"),
#if !defined(_WIN32_WCE) && !defined(DREAMCAST) && !defined(DIRECTX)
		UI_MENU_ACTION_ACCEL(3, "Enter Monitor", "F8"),
#endif
#ifdef DIRECTX
		UI_MENU_ACTION_ACCEL(3, monitor_label, "F8"),
#endif
		UI_MENU_ACTION_ACCEL(4, "Continue After CIM", "Esc"),
		UI_MENU_ACTION_ACCEL(5, "Exit Emulator", "F9"),
		UI_MENU_END
	};

	int option = 0;
	snprintf(cim_info, sizeof(cim_info), "Code $%02X (CIM) at address $%04X", UI_crash_code, UI_crash_address);

	for (;;) {
		option = UI_driver->fSelect("!!! The Atari computer has crashed !!!", 0, option, menu_array, NULL);

		if (UI_alt_function >= 0) /* pressed F5, Shift+F5 or F9 */
			return FALSE;

		switch (option) {
		case 0:				/* Power On Reset */
			UI_alt_function = UI_MENU_RESETW;
			return FALSE;
		case 1:				/* Power Off Reset */
			UI_alt_function = UI_MENU_RESETC;
			return FALSE;
		case 2:				/* Menu */
			return FALSE;
#if !defined(_WIN32_WCE) && !defined(DREAMCAST)
		case 3:				/* Monitor */
			UI_alt_function = UI_MENU_MONITOR;
			return FALSE;
#endif
		case -2:
		case -1:			/* ESC key */
		case 4:				/* Continue after CIM */
			CPU_regPC = UI_crash_afterCIM;
			return TRUE;
		case 5:				/* Exit */
			UI_alt_function = UI_MENU_EXIT;
			return FALSE;
		}
	}
}
#endif

/*
vim:ts=4:sw=4:
*/
