/*
 * atari.c - main high-level routines
 *
 * Copyright (c) 1995-1998 David Firth
 * Copyright (c) 1998-2008 Atari800 development team (see DOC/CREDITS)
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

#include "afile.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# elif defined(HAVE_TIME_H)
#  include <time.h>
# endif
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#ifdef __EMX__
#define INCL_DOS
#include <os2.h>
#endif
#ifdef __BEOS__
#include <OS.h>
#endif
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif
#ifdef R_SERIAL
#include <sys/stat.h>
#endif
#ifdef SDL
#include <SDL.h>
#endif

#include "akey.h"
#include "antic.h"
#include "atari.h"
#include "binload.h"
#include "cartridge.h"
#include "cassette.h"
#include "cfg.h"
#include "cpu.h"
#include "devices.h"
#include "esc.h"
#include "gtia.h"
#include "input.h"
#include "log.h"
#include "memory.h"
#include "monitor.h"
#ifdef IDE
#  include "ide.h"
#endif
#include "pia.h"
#include "platform.h"
#include "pokey.h"
#include "rtime.h"
#include "pbi.h"
#include "sio.h"
#include "util.h"
#if !defined(BASIC) && !defined(CURSES_BASIC)
#include "colours.h"
#include "screen.h"
#endif
#ifndef BASIC
#include "statesav.h"
#ifndef __PLUS
#include "ui.h"
#endif
#endif /* BASIC */
#if defined(SOUND) && !defined(__PLUS)
#include "pokeysnd.h"
#include "sndsave.h"
#include "sound.h"
#endif
#ifdef R_IO_DEVICE
#include "rdevice.h"
#endif
#ifdef __PLUS
#ifdef _WX_
#include "export.h"
#else /* _WX_ */
#include "globals.h"
#include "macros.h"
#include "display_win.h"
#include "misc_win.h"
#include "registry.h"
#include "timing.h"
#include "FileService.h"
#include "Helpers.h"
#endif /* _WX_ */
#endif /* __PLUS */
#ifdef PBI_BB
#include "pbi_bb.h"
#endif
#if defined(PBI_XLD) || defined (VOICEBOX)
#include "votraxsnd.h"
#endif
#ifdef XEP80_EMULATION
#include "xep80.h"
#endif
#ifdef AF80
#include "af80.h"
#endif
#ifdef NTSC_FILTER
#include "filter_ntsc.h"
#endif
#ifdef VOICEBOX
#include "voicebox.h"
#endif
#if SUPPORTS_CHANGE_VIDEOMODE
#include "videomode.h"
#endif
#ifdef DIRECTX
#include "win32\main.h"
#endif

int Atari800_machine_type = Atari800_MACHINE_XLXE;
int Atari800_tv_mode = Atari800_TV_NTSC;
int Atari800_disable_basic = TRUE;

int verbose = FALSE;

int Atari800_display_screen = FALSE;
int Atari800_nframes = 0;
int Atari800_refresh_rate = 1;
int Atari800_collisions_in_skipped_frames = FALSE;
int Atari800_turbo = FALSE;
int Atari800_auto_frameskip = FALSE;

#ifdef BENCHMARK
static double benchmark_start_time;
static double Atari_time(void);
#endif

int emuos_mode = 1;	/* 0 = never use EmuOS, 1 = use EmuOS if real OS not available, 2 = always use EmuOS */

#ifdef HAVE_SIGNAL
volatile sig_atomic_t sigint_flag = FALSE;

static RETSIGTYPE sigint_handler(int num)
{
	sigint_flag = TRUE;
	/* Avoid restoring the original signal handler. */
	signal(SIGINT, sigint_handler);
	return;
}
#endif

void Atari800_Warmstart(void)
{
	if (Atari800_machine_type == Atari800_MACHINE_OSA || Atari800_machine_type == Atari800_MACHINE_OSB) {
		/* A real Axlon homebanks on reset */
		/* XXX: what does Mosaic do? */
		if (MEMORY_axlon_enabled) MEMORY_PutByte(0xcfff, 0);
		/* RESET key in 400/800 does not reset chips,
		   but only generates RNMI interrupt */
		ANTIC_NMIST = 0x3f;
		CPU_NMI();
	}
	else {
		PBI_Reset();
		PIA_Reset();
		ANTIC_Reset();
		/* CPU_Reset() must be after PIA_Reset(),
		   because Reset routine vector must be read from OS ROM */
		CPU_Reset();
		/* note: POKEY and GTIA have no Reset pin */
	}
#ifdef __PLUS
	HandleResetEvent();
#endif
}

void Atari800_Coldstart(void)
{
	PBI_Reset();
	PIA_Reset();
	ANTIC_Reset();
	/* CPU_Reset() must be after PIA_Reset(),
	   because Reset routine vector must be read from OS ROM */
	CPU_Reset();
	/* note: POKEY and GTIA have no Reset pin */
#ifdef __PLUS
	HandleResetEvent();
#endif
	/* reset cartridge to power-up state */
	CARTRIDGE_Start();
	/* set Atari OS Coldstart flag */
	MEMORY_dPutByte(0x244, 1);
	/* handle Option key (disable BASIC in XL/XE)
	   and Start key (boot from cassette) */
	GTIA_consol_index = 2;
	GTIA_consol_table[2] = 0x0f;
	if (Atari800_disable_basic && !BINLOAD_loading_basic) {
		/* hold Option during reboot */
		GTIA_consol_table[2] &= ~INPUT_CONSOL_OPTION;
	}
	if (CASSETTE_hold_start) {
		/* hold Start during reboot */
		GTIA_consol_table[2] &= ~INPUT_CONSOL_START;
	}
	GTIA_consol_table[1] = GTIA_consol_table[2];
#ifdef AF80
	if (AF80_enabled) {
		AF80_Reset();
		AF80_InsertRightCartridge();
	}
#endif
}

int Atari800_LoadImage(const char *filename, UBYTE *buffer, int nbytes)
{
	FILE *f;
	int len;

	f = fopen(filename, "rb");
	if (f == NULL) {
		Log_print("Error loading ROM image: %s", filename);
		return FALSE;
	}
	len = fread(buffer, 1, nbytes, f);
	fclose(f);
	if (len != nbytes) {
		Log_print("Error reading %s", filename);
		return FALSE;
	}
	return TRUE;
}

#include "emuos.h"

#define COPY_EMUOS(padding) do { \
		memset(MEMORY_os, 0, padding); \
		memcpy(MEMORY_os + (padding), emuos_h, 0x2000); \
	} while (0)

static int load_roms(void)
{
	switch (Atari800_machine_type) {
	case Atari800_MACHINE_OSA:
		if (emuos_mode == 2)
			COPY_EMUOS(0x0800);
		else if (!Atari800_LoadImage(CFG_osa_filename, MEMORY_os, 0x2800)) {
			if (emuos_mode == 1)
				COPY_EMUOS(0x0800);
			else
				return FALSE;
		}
		else
			MEMORY_have_basic = Atari800_LoadImage(CFG_basic_filename, MEMORY_basic, 0x2000);
		break;
	case Atari800_MACHINE_OSB:
		if (emuos_mode == 2)
			COPY_EMUOS(0x0800);
		else if (!Atari800_LoadImage(CFG_osb_filename, MEMORY_os, 0x2800)) {
			if (emuos_mode == 1)
				COPY_EMUOS(0x0800);
			else
				return FALSE;
		}
		else
			MEMORY_have_basic = Atari800_LoadImage(CFG_basic_filename, MEMORY_basic, 0x2000);
		break;
	case Atari800_MACHINE_XLXE:
		if (emuos_mode == 2)
			COPY_EMUOS(0x2000);
		else if (!Atari800_LoadImage(CFG_xlxe_filename, MEMORY_os, 0x4000)) {
			if (emuos_mode == 1)
				COPY_EMUOS(0x2000);
			else
				return FALSE;
		}
		else {
			/* if you really don't want built-in BASIC */
			if (!strcmp(CFG_basic_filename,"none"))
				memset(MEMORY_basic, 0, 0x2000);
			else if (!Atari800_LoadImage(CFG_basic_filename, MEMORY_basic, 0x2000))
				return FALSE;
		}
		MEMORY_xe_bank = 0;
		break;
	case Atari800_MACHINE_5200:
		if (!Atari800_LoadImage(CFG_5200_filename, MEMORY_os, 0x800))
			return FALSE;
		break;
	}
	return TRUE;
}

int Atari800_InitialiseMachine(void)
{
	ESC_ClearAll();
	if (!load_roms())
		return FALSE;
	MEMORY_InitialiseMachine();
	Devices_UpdatePatches();
	return TRUE;
}

/* Initialise any modules before loading the config file. */
static void PreInitialise(void)
{
#if !defined(BASIC) && !defined(CURSES_BASIC)
	Colours_PreInitialise();
#endif
#ifdef NTSC_FILTER
	FILTER_NTSC_PreInitialise();
#endif
}

int Atari800_Initialise(int *argc, char *argv[])
{
	int i, j;
	const char *rom_filename = NULL;
	const char *rom2_filename = NULL;
	const char *run_direct = NULL;
#ifndef BASIC
	const char *state_file = NULL;
#endif
#ifdef __PLUS
	/* Atari800Win PLus doesn't use configuration files,
	   it reads configuration from the Registry */
#ifndef _WX_
	int bUpdateRegistry = (*argc > 1);
#endif
	int bTapeFile = FALSE;
	int nCartType = CARTRIDGE_type;

	/* It is necessary because of the CARTRIDGE_Start (there must not be the
	   registry-read value available at startup) */
	CARTRIDGE_type = CARTRIDGE_NONE;

#ifndef _WX_
	/* Print the time info in the "Log file" window */
	Misc_PrintTime();

	/* Force screen refreshing */
	g_nTestVal = _GetRefreshRate() - 1;

	g_ulAtariState = ATARI_UNINITIALIZED;
#endif /* _WX_ */
	PreInitialise();
#else /* __PLUS */
	const char *rtconfig_filename = NULL;
	int got_config;
	int help_only = FALSE;

	PreInitialise();

	if (*argc > 1) {
		for (i = j = 1; i < *argc; i++) {
			if (strcmp(argv[i], "-config") == 0) {
				if (i + 1 < *argc)
					rtconfig_filename = argv[++i];
				else {
					Log_print("Missing argument for '%s'", argv[i]);
					return FALSE;
				}
			}
			else if (strcmp(argv[i], "-v") == 0 ||
					 strcmp(argv[i], "-version") == 0 ||
					 strcmp(argv[i], "--version") == 0) {
				printf("%s\n", Atari800_TITLE);
				return FALSE;
			}
			else if (strcmp(argv[i], "--usage") == 0 ||
					 strcmp(argv[i], "--help") == 0) {
				argv[j++] = "-help";
			}
			else if (strcmp(argv[i], "-verbose") == 0) {
				verbose = TRUE;
			}
			else {
				argv[j++] = argv[i];
			}
		}
		*argc = j;
	}
#ifndef ANDROID
	got_config = CFG_LoadConfig(rtconfig_filename);
#else
	got_config = TRUE; /* pretend we got a config file -- not needed in Android */
#endif

	/* try to find ROM images if the configuration file is not found
	   or it does not specify some ROM paths (blank paths count as specified) */
	CFG_FindROMImages("", TRUE); /* current directory */
#if defined(unix) || defined(__unix__) || defined(__linux__)
	CFG_FindROMImages("/usr/share/atari800", TRUE);
#endif
	if (*argc > 0 && argv[0] != NULL) {
		char atari800_exe_dir[FILENAME_MAX];
		char atari800_exe_rom_dir[FILENAME_MAX];
		/* the directory of the Atari800 program */
		Util_splitpath(argv[0], atari800_exe_dir, NULL);
		CFG_FindROMImages(atari800_exe_dir, TRUE);
		/* "rom" and "ROM" subdirectories of this directory */
		Util_catpath(atari800_exe_rom_dir, atari800_exe_dir, "rom");
		CFG_FindROMImages(atari800_exe_rom_dir, TRUE);
/* skip "ROM" on systems that are known to be case-insensitive */
#if !defined(DJGPP) && !defined(HAVE_WINDOWS_H)
		Util_catpath(atari800_exe_rom_dir, atari800_exe_dir, "ROM");
		CFG_FindROMImages(atari800_exe_rom_dir, TRUE);
#endif
	}
	/* finally if nothing is found, set some defaults to make
	   the configuration file easier to edit */
	if (Util_filenamenotset(CFG_osa_filename))
		strcpy(CFG_osa_filename, "atariosa.rom");
	if (Util_filenamenotset(CFG_osb_filename))
		strcpy(CFG_osb_filename, "atariosb.rom");
	if (Util_filenamenotset(CFG_xlxe_filename))
		strcpy(CFG_xlxe_filename, "atarixl.rom");
	if (Util_filenamenotset(CFG_5200_filename))
		strcpy(CFG_5200_filename, "5200.rom");
	if (Util_filenamenotset(CFG_basic_filename))
		strcpy(CFG_basic_filename, "ataribas.rom");

	/* if no configuration file read, try to save one with the defaults */
	if (!got_config)
		CFG_WriteConfig();

#endif /* __PLUS */

	for (i = j = 1; i < *argc; i++) {
		if (strcmp(argv[i], "-atari") == 0) {
			if (Atari800_machine_type != Atari800_MACHINE_OSA) {
				Atari800_machine_type = Atari800_MACHINE_OSB;
				MEMORY_ram_size = 48;
			}
		}
		else if (strcmp(argv[i], "-xl") == 0) {
			Atari800_machine_type = Atari800_MACHINE_XLXE;
			MEMORY_ram_size = 64;
		}
		else if (strcmp(argv[i], "-xe") == 0) {
			Atari800_machine_type = Atari800_MACHINE_XLXE;
			MEMORY_ram_size = 128;
		}
		else if (strcmp(argv[i], "-320xe") == 0) {
			Atari800_machine_type = Atari800_MACHINE_XLXE;
			MEMORY_ram_size = MEMORY_RAM_320_COMPY_SHOP;
		}
		else if (strcmp(argv[i], "-rambo") == 0) {
			Atari800_machine_type = Atari800_MACHINE_XLXE;
			MEMORY_ram_size = MEMORY_RAM_320_RAMBO;
		}
		else if (strcmp(argv[i], "-5200") == 0) {
			Atari800_machine_type = Atari800_MACHINE_5200;
			MEMORY_ram_size = 16;
		}
		else if (strcmp(argv[i], "-nobasic") == 0)
			Atari800_disable_basic = TRUE;
		else if (strcmp(argv[i], "-basic") == 0)
			Atari800_disable_basic = FALSE;
		else if (strcmp(argv[i], "-nopatch") == 0)
			ESC_enable_sio_patch = FALSE;
		else if (strcmp(argv[i], "-nopatchall") == 0)
			ESC_enable_sio_patch = Devices_enable_h_patch = Devices_enable_p_patch = Devices_enable_r_patch = FALSE;
		else if (strcmp(argv[i], "-pal") == 0)
			Atari800_tv_mode = Atari800_TV_PAL;
		else if (strcmp(argv[i], "-ntsc") == 0)
			Atari800_tv_mode = Atari800_TV_NTSC;
		else if (strcmp(argv[i], "-a") == 0) {
			Atari800_machine_type = Atari800_MACHINE_OSA;
			MEMORY_ram_size = 48;
		}
		else if (strcmp(argv[i], "-b") == 0) {
			Atari800_machine_type = Atari800_MACHINE_OSB;
			MEMORY_ram_size = 48;
		}
		else if (strcmp(argv[i], "-emuos") == 0)
			emuos_mode = 2;
		else if (strcmp(argv[i], "-c") == 0) {
			if (MEMORY_ram_size == 48)
				MEMORY_ram_size = 52;
		}
		else if (strcmp(argv[i], "-axlon0f") == 0) {
			MEMORY_axlon_0f_mirror = TRUE;
		}
#ifdef STEREO_SOUND
		else if (strcmp(argv[i], "-stereo") == 0) {
			POKEYSND_stereo_enabled = TRUE;
		}
		else if (strcmp(argv[i], "-nostereo") == 0) {
			POKEYSND_stereo_enabled = FALSE;
		}
#endif /* STEREO_SOUND */
		else if (strcmp(argv[i], "-turbo") == 0) {
			Atari800_turbo = TRUE;
		}
		else {
			/* parameters that take additional argument follow here */
			int i_a = (i + 1 < *argc);		/* is argument available? */
			int a_m = FALSE;			/* error, argument missing! */

			if (strcmp(argv[i], "-osa_rom") == 0) {
				if (i_a) Util_strlcpy(CFG_osa_filename, argv[++i], sizeof(CFG_osa_filename)); else a_m = TRUE;
			}
#ifdef R_IO_DEVICE
			else if (strcmp(argv[i], "-rdevice") == 0) {
				Devices_enable_r_patch = TRUE;
#ifdef R_SERIAL
				if (i_a && i + 2 < *argc && *argv[i + 1] != '-') {  /* optional serial device name */
					struct stat statbuf;
					if (! stat(argv[i + 1], &statbuf)) {
						if (S_ISCHR(statbuf.st_mode)) { /* only accept devices as serial device */
							Util_strlcpy(RDevice_serial_device, argv[++i], FILENAME_MAX);
							RDevice_serial_enabled = TRUE;
						}
					}
				}
#endif /* R_SERIAL */
			}
#endif
			else if (strcmp(argv[i], "-osb_rom") == 0) {
				if (i_a) Util_strlcpy(CFG_osb_filename, argv[++i], sizeof(CFG_osb_filename)); else a_m = TRUE;
			}
			else if (strcmp(argv[i], "-xlxe_rom") == 0) {
				if (i_a) Util_strlcpy(CFG_xlxe_filename, argv[++i], sizeof(CFG_xlxe_filename)); else a_m = TRUE;
			}
			else if (strcmp(argv[i], "-5200_rom") == 0) {
				if (i_a) Util_strlcpy(CFG_5200_filename, argv[++i], sizeof(CFG_5200_filename)); else a_m = TRUE;
			}
			else if (strcmp(argv[i], "-basic_rom") == 0) {
				if (i_a) Util_strlcpy(CFG_basic_filename, argv[++i], sizeof(CFG_basic_filename)); else a_m = TRUE;
			}
			else if (strcmp(argv[i], "-cart") == 0) {
				if (i_a) rom_filename = argv[++i]; else a_m = TRUE;
			}
			else if (strcmp(argv[i], "-cart2") == 0) {
				if (i_a) rom2_filename = argv[++i]; else a_m = TRUE;
			}
			else if (strcmp(argv[i], "-run") == 0) {
				if (i_a) run_direct = argv[++i]; else a_m = TRUE;
			}
			else if (strcmp(argv[i], "-mosaic") == 0) {
				if (i_a) {
					int total_ram = Util_sscandec(argv[++i]);
					MEMORY_mosaic_enabled = TRUE;
					MEMORY_mosaic_maxbank = (total_ram - 48)/4 - 1;
					if (((total_ram - 48) % 4 != 0) || (MEMORY_mosaic_maxbank > 0x3e) || (MEMORY_mosaic_maxbank < 0)) {
						Log_print("Invalid Mosaic total RAM size");
						return FALSE;
					}
					if (MEMORY_axlon_enabled) {
						Log_print("Axlon and Mosaic can not both be enabled, because they are incompatible");
						return FALSE;
					}
				}
				else a_m = TRUE;
			}
			else if (strcmp(argv[i], "-axlon") == 0) {
				if (i_a) {
					int total_ram = Util_sscandec(argv[++i]);
					int banks = ((total_ram) - 32) / 16;
					MEMORY_axlon_enabled = TRUE;
					if (((total_ram - 32) % 16 != 0) || ((banks != 8) && (banks != 16) && (banks != 32) && (banks != 64) && (banks != 128) && (banks != 256))) {
						Log_print("Invalid Axlon total RAM size");
						return FALSE;
					}
					if (MEMORY_mosaic_enabled) {
						Log_print("Axlon and Mosaic can not both be enabled, because they are incompatible");
						return FALSE;
					}
					MEMORY_axlon_bankmask = banks - 1;
				}
				else a_m = TRUE;
			}
#ifndef BASIC
			/* The BASIC version does not support state files, because:
			   1. It has no ability to save state files, because of lack of UI.
			   2. It uses a simplified emulation, so the state files would be
			      incompatible with other versions.
			   3. statesav is not compiled in to make the executable smaller. */
			else if (strcmp(argv[i], "-state") == 0) {
				if (i_a) state_file = argv[++i]; else a_m = TRUE;
			}
			else if (strcmp(argv[i], "-refresh") == 0) {
				if (i_a) {
					Atari800_refresh_rate = Util_sscandec(argv[++i]);
					if (Atari800_refresh_rate < 1) {
						Log_print("Invalid refresh rate, using 1");
						Atari800_refresh_rate = 1;
					}
				}
				else
					a_m = TRUE;
			}
#endif /* BASIC */
			else {
				/* all options known to main module tried but none matched */

				if (strcmp(argv[i], "-help") == 0) {
#ifndef __PLUS
					help_only = TRUE;
					Log_print("\t-config <file>   Specify Alternate Configuration File");
#endif
					Log_print("\t-atari           Emulate Atari 800");
					Log_print("\t-xl              Emulate Atari 800XL");
					Log_print("\t-xe              Emulate Atari 130XE");
					Log_print("\t-320xe           Emulate Atari 320XE (COMPY SHOP)");
					Log_print("\t-rambo           Emulate Atari 320XE (RAMBO)");
					Log_print("\t-5200            Emulate Atari 5200 Games System");
					Log_print("\t-nobasic         Turn off Atari BASIC ROM");
					Log_print("\t-basic           Turn on Atari BASIC ROM");
					Log_print("\t-pal             Enable PAL TV mode");
					Log_print("\t-ntsc            Enable NTSC TV mode");
					Log_print("\t-osa_rom <file>  Load OS A ROM from file");
					Log_print("\t-osb_rom <file>  Load OS B ROM from file");
					Log_print("\t-xlxe_rom <file> Load XL/XE ROM from file");
					Log_print("\t-5200_rom <file> Load 5200 ROM from file");
					Log_print("\t-basic_rom <fil> Load BASIC ROM from file");
					Log_print("\t-cart <file>     Install cartridge (raw or CART format)");
					Log_print("\t-run <file>      Run Atari program (COM, EXE, XEX, BAS, LST)");
#ifndef BASIC
					Log_print("\t-state <file>    Load saved-state file");
					Log_print("\t-refresh <rate>  Specify screen refresh rate");
#endif
					Log_print("\t-nopatch         Don't patch SIO routine in OS");
					Log_print("\t-nopatchall      Don't patch OS at all, H: device won't work");
					Log_print("\t-a               Use OS A");
					Log_print("\t-b               Use OS B");
					Log_print("\t-c               Enable RAM between 0xc000 and 0xcfff in Atari 800");
					Log_print("\t-axlon <n>       Use Atari 800 Axlon memory expansion: <n> k total RAM");
					Log_print("\t-axlon0f         Use Axlon shadow at 0x0fc0-0x0fff");
					Log_print("\t-mosaic <n>      Use 400/800 Mosaic memory expansion: <n> k total RAM");
#ifdef R_IO_DEVICE
					Log_print("\t-rdevice [<dev>] Enable R: emulation (using serial device <dev>)");
#endif
					Log_print("\t-turbo           Run emulated Atari as fast as possible");
					Log_print("\t-v               Show version/release number");
				}

				/* copy this option for platform/module specific evaluation */
				argv[j++] = argv[i];
			}

			/* this is the end of the additional argument check */
			if (a_m) {
				printf("Missing argument for '%s'\n", argv[i]);
				return FALSE;
			}
		}
	}

	*argc = j;
	if (
#if !defined(BASIC) && !defined(CURSES_BASIC)
		!Colours_Initialise(argc, argv) ||
#endif
		!Devices_Initialise(argc, argv)
		|| !RTIME_Initialise(argc, argv)
#ifdef IDE
		|| !IDE_Initialise(argc, argv)
#endif
		|| !SIO_Initialise (argc, argv)
		|| !CASSETTE_Initialise(argc, argv)
		|| !PBI_Initialise(argc,argv)
#ifdef VOICEBOX
		|| !VOICEBOX_Initialise(argc, argv)
#endif
#ifndef BASIC
		|| !INPUT_Initialise(argc, argv)
#endif
#ifdef XEP80_EMULATION
		|| !XEP80_Initialise(argc, argv)
#endif
#ifdef AF80
		|| !AF80_Initialise(argc, argv)
#endif
#ifdef NTSC_FILTER
		|| !FILTER_NTSC_Initialise(argc, argv)
#endif
#if SUPPORTS_CHANGE_VIDEOMODE
		|| !VIDEOMODE_Initialise(argc, argv)
#endif
#ifndef DONT_DISPLAY
		/* Platform Specific Initialisation */
		|| !PLATFORM_Initialise(argc, argv)
#endif
#if !defined(BASIC) && !defined(CURSES_BASIC)
		|| !Screen_Initialise(argc, argv)
#endif
		/* Initialise Custom Chips */
		|| !ANTIC_Initialise(argc, argv)
		|| !GTIA_Initialise(argc, argv)
		|| !PIA_Initialise(argc, argv)
		|| !POKEY_Initialise(argc, argv)
	)
		return FALSE;

#ifndef __PLUS

	if (help_only) {
		Atari800_Exit(FALSE);
		return FALSE;
	}

#if SUPPORTS_CHANGE_VIDEOMODE
#ifndef DONT_DISPLAY
	if (!VIDEOMODE_InitialiseDisplay()) {
		Atari800_Exit(FALSE);
		return FALSE;
	}
#endif
#endif
	/* Configure Atari System */
	Atari800_InitialiseMachine();
#else /* __PLUS */

	if (!InitialiseMachine()) {
#ifndef _WX_
		if (bUpdateRegistry)
			WriteAtari800Registry();
#endif
		return FALSE;
	}

#endif /* __PLUS */

	/* Auto-start files left on the command line */
	j = 1; /* diskno */
	for (i = 1; i < *argc; i++) {
		if (j > 8) {
			/* The remaining arguments are not necessary disk images, but ignore them... */
			Log_print("Too many disk image filenames on the command line (max. 8).");
			break;
		}
		switch (AFILE_OpenFile(argv[i], i == 1, j, FALSE)) {
			case AFILE_ERROR:
				Log_print("Error opening \"%s\"", argv[i]);
				break;
			case AFILE_ATR:
			case AFILE_XFD:
			case AFILE_ATR_GZ:
			case AFILE_XFD_GZ:
			case AFILE_DCM:
			case AFILE_PRO:
				j++;
				break;
			default:
				break;
		}
	}

	/* Install requested ROM cartridge */
	if (rom_filename) {
		int r = CARTRIDGE_Insert(rom_filename);
		if (r < 0) {
			Log_print("Error inserting cartridge \"%s\": %s", rom_filename,
			r == CARTRIDGE_CANT_OPEN ? "Can't open file" :
			r == CARTRIDGE_BAD_FORMAT ? "Bad format" :
			r == CARTRIDGE_BAD_CHECKSUM ? "Bad checksum" :
			"Unknown error");
		}
		if (r > 0) {
#ifdef BASIC
			Log_print("Raw cartridge images not supported in BASIC version!");
#else /* BASIC */

#ifndef __PLUS
			UI_is_active = TRUE;
			CARTRIDGE_type = UI_SelectCartType(r);
			UI_is_active = FALSE;
#else /* __PLUS */
			CARTRIDGE_type = (CARTRIDGE_NONE == nCartType ? UI_SelectCartType(r) : nCartType);
#endif /* __PLUS */
			CARTRIDGE_Start();

#endif /* BASIC */
		}
#ifndef __PLUS
		if (CARTRIDGE_type != CARTRIDGE_NONE) {
			int for5200 = CARTRIDGE_IsFor5200(CARTRIDGE_type);
			if (for5200 && Atari800_machine_type != Atari800_MACHINE_5200) {
				Atari800_machine_type = Atari800_MACHINE_5200;
				MEMORY_ram_size = 16;
				Atari800_InitialiseMachine();
			}
			else if (!for5200 && Atari800_machine_type == Atari800_MACHINE_5200) {
				Atari800_machine_type = Atari800_MACHINE_XLXE;
				MEMORY_ram_size = 64;
				Atari800_InitialiseMachine();
			}
		}
#endif /* __PLUS */
	}

	/* Install requested second ROM cartridge, if first is SpartaX */
	if (((CARTRIDGE_type == CARTRIDGE_SDX_64) || (CARTRIDGE_type == CARTRIDGE_SDX_128)) && rom2_filename) {
		int r = CARTRIDGE_Insert_Second(rom2_filename);
		if (r < 0) {
			Log_print("Error inserting cartridge \"%s\": %s", rom2_filename,
			r == CARTRIDGE_CANT_OPEN ? "Can't open file" :
			r == CARTRIDGE_BAD_FORMAT ? "Bad format" :
			r == CARTRIDGE_BAD_CHECKSUM ? "Bad checksum" :
			"Unknown error");
		}
		if (r > 0) {
#ifdef BASIC
			Log_print("Raw cartridge images not supported in BASIC version!");
#else /* BASIC */

#ifndef __PLUS
			UI_is_active = TRUE;
			CARTRIDGE_second_type = UI_SelectCartType(r);
			UI_is_active = FALSE;
#else /* __PLUS */
			CARTRIDGE_second_type = (CARTRIDGE_NONE == nCartType ? SelectCartType(r) : nCartType);
#endif /* __PLUS */
#endif /* BASIC */
		}
	}
#ifdef AF80
	/* Insert Austin Franklin 80 column cartridge */
		if (AF80_enabled) {
			AF80_InsertRightCartridge();
		}
#endif
	
	/* Load Atari executable, if any */
	if (run_direct != NULL)
		BINLOAD_Loader(run_direct);

#ifndef BASIC
	/* Load state file */
	if (state_file != NULL) {
		if (StateSav_ReadAtariState(state_file, "rb"))
			/* Don't press Option */
			GTIA_consol_table[1] = GTIA_consol_table[2] = 0x0f;
	}
#endif

#ifdef HAVE_SIGNAL
	/* Install CTRL-C Handler */
	signal(SIGINT, sigint_handler);
#endif

#ifdef __PLUS
#ifndef _WX_
	/* Update the Registry if any parameters were specified */
	if (bUpdateRegistry)
		WriteAtari800Registry();
	Timer_Start(FALSE);
#endif /* _WX_ */
	g_ulAtariState &= ~ATARI_UNINITIALIZED;
#endif /* __PLUS */

#ifdef BENCHMARK
	benchmark_start_time = Atari_time();
#endif
	return TRUE;
}

UNALIGNED_STAT_DEF(Screen_atari_write_long_stat)
UNALIGNED_STAT_DEF(pm_scanline_read_long_stat)
UNALIGNED_STAT_DEF(memory_read_word_stat)
UNALIGNED_STAT_DEF(memory_write_word_stat)
UNALIGNED_STAT_DEF(memory_read_aligned_word_stat)
UNALIGNED_STAT_DEF(memory_write_aligned_word_stat)

int Atari800_Exit(int run_monitor)
{
	int restart;

#ifdef __PLUS
	if (CPU_cim_encountered)
		g_ulAtariState |= ATARI_CRASHED;
#endif

#ifdef STAT_UNALIGNED_WORDS
	printf("(ptr&7) Screen_atari  pm_scanline  _____ memory ______  memory (aligned addr)\n");
	printf("          32-bit W      32-bit R   16-bit R   16-bit W   16-bit R   16-bit W\n");
	{
		unsigned int sums[6] = {0, 0, 0, 0, 0, 0};
		int i;
		for (i = 0; i < 8; i++) {
			printf("%6d%12u%14u%11u%11u%11u%11u\n", i,
				Screen_atari_write_long_stat[i], pm_scanline_read_long_stat[i],
				memory_read_word_stat[i], memory_write_word_stat[i],
				memory_read_aligned_word_stat[i], memory_write_aligned_word_stat[i]);
			sums[0] += Screen_atari_write_long_stat[i];
			sums[1] += pm_scanline_read_long_stat[i];
			sums[2] += memory_read_word_stat[i];
			sums[3] += memory_write_word_stat[i];
			sums[4] += memory_read_aligned_word_stat[i];
			sums[5] += memory_write_aligned_word_stat[i];
		}
		printf("total:%12u%14u%11u%11u%11u%11u\n",
			sums[0], sums[1], sums[2], sums[3], sums[4], sums[5]);
	}
#endif /* STAT_UNALIGNED_WORDS */
	restart = PLATFORM_Exit(run_monitor);
#ifdef HAVE_SIGNAL
	/* If a user pressed Ctrl+C in the monitor, avoid immediate return to it. */
	sigint_flag = FALSE;
#endif /* HAVE_SIGNAL */
#ifndef __PLUS
	if (!restart) {
		SIO_Exit();	/* umount disks, so temporary files are deleted */
#ifndef BASIC
		INPUT_Exit();	/* finish event recording */
#endif
#ifdef R_IO_DEVICE
		RDevice_Exit(); /* R: Device cleanup */
#endif
#ifdef SOUND
		SndSave_CloseSoundFile();
#endif
#if SUPPORTS_CHANGE_VIDEOMODE
		VIDEOMODE_Exit();
#endif
		MONITOR_Exit();
	}
#endif /* __PLUS */
	return restart;
}

#ifndef __PLUS

#ifdef PS2

double Atari_time(void);
void Atari_sleep(double s);

#else /* PS2 */

static double Atari_time(void)
{
#ifdef SDL
	return SDL_GetTicks() * 1e-3;
#elif defined(HAVE_WINDOWS_H)
	return GetTickCount() * 1e-3;
#elif defined(DJGPP)
	/* DJGPP has gettimeofday, but it's not more accurate than uclock */
	return uclock() * (1.0 / UCLOCKS_PER_SEC);
#elif defined(HAVE_GETTIMEOFDAY)
	struct timeval tp;
	gettimeofday(&tp, NULL);
	return tp.tv_sec + 1e-6 * tp.tv_usec;
#elif defined(HAVE_UCLOCK)
	return uclock() * (1.0 / UCLOCKS_PER_SEC);
#elif defined(HAVE_CLOCK)
	return clock() * (1.0 / CLK_TCK);
#else
#error No function found for Atari_time()
#endif
}

/* FIXME: Ports should use SUPPORTS_PLATFORM_SLEEP and SUPPORTS_PLATFORM_TIME */
/* and not this mess */
#ifndef SUPPORTS_PLATFORM_SLEEP

static void Atari_sleep(double s)
{
	if (s > 0) {
#ifdef HAVE_WINDOWS_H
		Sleep((DWORD) (s * 1e3));
#elif defined(DJGPP)
		/* DJGPP has usleep and select, but they don't work that good */
		/* XXX: find out why */
		double curtime = Atari_time();
		while ((curtime + s) > Atari_time());
#elif defined(HAVE_NANOSLEEP)
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = s * 1e9;
		nanosleep(&ts, NULL);
#elif defined(HAVE_USLEEP)
		usleep(s * 1e6);
#elif defined(__BEOS__)
		/* added by Walter Las for BeOS */
		snooze(s * 1e6);
#elif defined(__EMX__)
		/* added by Brian Smith for os/2 */
		DosSleep(s);
#elif defined(HAVE_SELECT)
		/* linux */
		struct timeval tp;
		tp.tv_sec = 0;
		tp.tv_usec = s * 1e6;
		select(1, NULL, NULL, NULL, &tp);
#else
		double curtime = Atari_time();
		while ((curtime + s) > Atari_time());
#endif
	}
}

#endif /* SUPPORTS_PLATFORM_SLEEP */

#endif /* PS2 */

static void autoframeskip(double curtime, double lasttime)
{
	static int afs_lastframe = 0, afs_discard = 0;
	static double afs_lasttime = 0.0, afs_sleeptime = 0.0;
	double afs_speedpct, afs_sleeppct, afs_ataritime, afs_realtime;

	if (lasttime - curtime > 0)
		afs_sleeptime += lasttime - curtime;
	if (curtime - afs_lasttime > 0.5) {
		afs_ataritime = ((double) (Atari800_nframes - afs_lastframe)) /
						((double) (Atari800_tv_mode == Atari800_TV_PAL ? 50 : 60));
		afs_realtime = curtime - afs_lasttime;
		afs_speedpct = 100.0 * afs_ataritime / afs_realtime;
		afs_sleeppct = 100.0 * afs_sleeptime / afs_realtime;

		if (afs_discard < 3 && (afs_realtime > 2.0 * afs_ataritime)) {
			afs_discard++;
		} else {
			afs_discard = 0;
			if (afs_speedpct < 90.0) {
				if (Atari800_refresh_rate < 4)
					Atari800_refresh_rate++;
			} else {
				if (afs_sleeppct > 20.0 && Atari800_refresh_rate > 1)
					Atari800_refresh_rate--;
			}
		}

		afs_sleeptime = 0.0;
		afs_lastframe = Atari800_nframes;
		afs_lasttime = Atari_time();
	}
}

void Atari800_Sync(void)
{
	static double lasttime = 0;
	double deltatime = 1.0 / ((Atari800_tv_mode == Atari800_TV_PAL) ? Atari800_FPS_PAL : Atari800_FPS_NTSC);
	double curtime;

#ifdef SYNCHRONIZED_SOUND
	deltatime *= PLATFORM_AdjustSpeed();
#endif
#ifdef ALTERNATE_SYNC_WITH_HOST
	if (! UI_is_active)
		deltatime *= Atari800_refresh_rate;
#endif
	lasttime += deltatime;
	curtime = Atari_time();
	if (Atari800_auto_frameskip)
		autoframeskip(curtime, lasttime);
#ifdef SUPPORTS_PLATFORM_SLEEP
	PLATFORM_Sleep(lasttime - curtime);
#else
	Atari_sleep(lasttime - curtime);
#endif
	curtime = Atari_time();

	if ((lasttime + deltatime) < curtime)
		lasttime = curtime;
}

#ifdef USE_CURSES
void curses_clear_screen(void);
#endif

#if defined(BASIC) || defined(VERY_SLOW) || defined(CURSES_BASIC)

#ifdef CURSES_BASIC
void curses_display_line(int anticmode, const UBYTE *screendata);
#endif

static int scanlines_to_dl;

/* steal cycles and generate DLI */
static void basic_antic_scanline(void)
{
	static UBYTE IR = 0;
	static const UBYTE mode_scanlines[16] =
		{ 0, 0, 8, 10, 8, 16, 8, 16, 8, 4, 4, 2, 1, 2, 1, 1 };
	static const UBYTE mode_bytes[16] =
		{ 0, 0, 40, 40, 40, 40, 20, 20, 10, 10, 20, 20, 20, 40, 40, 40 };
	static const UBYTE font40_cycles[4] = { 0, 32, 40, 47 };
	static const UBYTE font20_cycles[4] = { 0, 16, 20, 24 };
#ifdef CURSES_BASIC
	static int scanlines_to_curses_display = 0;
	static UWORD screenaddr = 0;
	static UWORD newscreenaddr = 0;
#endif

	int bytes = 0;
	if (--scanlines_to_dl <= 0) {
		if (ANTIC_DMACTL & 0x20) {
			IR = ANTIC_GetDLByte(&ANTIC_dlist);
			ANTIC_xpos++;
		}
		else
			IR &= 0x7f;	/* repeat last instruction, but don't generate DLI */
		switch (IR & 0xf) {
		case 0:
			scanlines_to_dl = ((IR >> 4) & 7) + 1;
			break;
		case 1:
			if (ANTIC_DMACTL & 0x20) {
				ANTIC_dlist = ANTIC_GetDLWord(&ANTIC_dlist);
				ANTIC_xpos += 2;
			}
			scanlines_to_dl = (IR & 0x40) ? 1024 /* no more DL in this frame */ : 1;
			break;
		default:
			if (IR & 0x40 && ANTIC_DMACTL & 0x20) {
#ifdef CURSES_BASIC
				screenaddr =
#endif
					ANTIC_GetDLWord(&ANTIC_dlist);
				ANTIC_xpos += 2;
			}
			/* can't steal cycles now, because DLI must come first */
			/* just an approximation: doesn't check HSCROL */
			switch (ANTIC_DMACTL & 3) {
			case 1:
				bytes = mode_bytes[IR & 0xf] * 8 / 10;
				break;
			case 2:
				bytes = mode_bytes[IR & 0xf];
				break;
			case 3:
				bytes = mode_bytes[IR & 0xf] * 12 / 10;
				break;
			default:
				break;
			}
#ifdef CURSES_BASIC
			/* Normally, we would call curses_display_line here,
			   and not use scanlines_to_curses_display at all.
			   That would however cause incorrect color of the "MEMORY"
			   menu item in Self Test - it isn't set properly
			   in the first scanline. We therefore postpone
			   curses_display_line call to the next scanline. */
			scanlines_to_curses_display = 2;
			newscreenaddr = screenaddr + bytes;
#endif
			/* just an approximation: doesn't check VSCROL */
			scanlines_to_dl = mode_scanlines[IR & 0xf];
			break;
		}
	}
	if (scanlines_to_dl == 1 && (IR & 0x80)) {
		CPU_GO(ANTIC_NMIST_C);
		ANTIC_NMIST = 0x9f;
		if (ANTIC_NMIEN & 0x80) {
			CPU_GO(ANTIC_NMI_C);
			CPU_NMI();
		}
	}
#ifdef CURSES_BASIC
	if (--scanlines_to_curses_display == 0) {
		curses_display_line(IR & 0xf, MEMORY_mem + screenaddr);
		/* 4k wrap */
		if (((screenaddr ^ newscreenaddr) & 0x1000) != 0)
			screenaddr = newscreenaddr - 0x1000;
		else
			screenaddr = newscreenaddr;
	}
#endif
	ANTIC_xpos += bytes;
	/* steal cycles in font modes */
	switch (IR & 0xf) {
	case 2:
	case 3:
	case 4:
	case 5:
		ANTIC_xpos += font40_cycles[ANTIC_DMACTL & 3];
		break;
	case 6:
	case 7:
		ANTIC_xpos += font20_cycles[ANTIC_DMACTL & 3];
		break;
	default:
		break;
	}
}

#define BASIC_LINE CPU_GO(ANTIC_LINE_C); ANTIC_xpos -= ANTIC_LINE_C - ANTIC_DMAR; ANTIC_screenline_cpu_clock += ANTIC_LINE_C; ANTIC_ypos++

static void basic_frame(void)
{
	/* scanlines 0 - 7 */
	ANTIC_ypos = 0;
	do {
		POKEY_Scanline();		/* check and generate IRQ */
		BASIC_LINE;
	} while (ANTIC_ypos < 8);

	scanlines_to_dl = 1;
	/* scanlines 8 - 247 */
	do {
		POKEY_Scanline();		/* check and generate IRQ */
		basic_antic_scanline();
		BASIC_LINE;
	} while (ANTIC_ypos < 248);

	/* scanline 248 */
	POKEY_Scanline();			/* check and generate IRQ */
	CPU_GO(ANTIC_NMIST_C);
	ANTIC_NMIST = 0x5f;				/* Set VBLANK */
	if (ANTIC_NMIEN & 0x40) {
		CPU_GO(ANTIC_NMI_C);
		CPU_NMI();
	}
	BASIC_LINE;

	/* scanlines 249 - 261(311) */
	do {
		POKEY_Scanline();		/* check and generate IRQ */
		BASIC_LINE;
	} while (ANTIC_ypos < Atari800_tv_mode);
}

#endif /* defined(BASIC) || defined(VERY_SLOW) || defined(CURSES_BASIC) */

void Atari800_Frame(void)
{
#ifndef BASIC
	static int refresh_counter = 0;

#ifdef HAVE_SIGNAL
	if (sigint_flag) {
		sigint_flag = FALSE;
		INPUT_key_code = AKEY_UI;
		UI_alt_function = UI_MENU_MONITOR;
	}
#endif /* HAVE_SIGNAL */

	switch (INPUT_key_code) {
	case AKEY_COLDSTART:
		Atari800_Coldstart();
		break;
	case AKEY_WARMSTART:
		Atari800_Warmstart();
		break;
	case AKEY_EXIT:
		Atari800_Exit(FALSE);
		exit(0);
	case AKEY_TURBO:
		Atari800_turbo = !Atari800_turbo;
		break;
	case AKEY_UI:
#ifdef SOUND
		Sound_Pause();
#endif
		UI_Run();
#ifdef SOUND
		Sound_Continue();
#endif
		break;
#ifndef CURSES_BASIC
	case AKEY_SCREENSHOT:
		Screen_SaveNextScreenshot(FALSE);
		break;
	case AKEY_SCREENSHOT_INTERLACE:
		Screen_SaveNextScreenshot(TRUE);
		break;
#endif /* CURSES_BASIC */
	case AKEY_PBI_BB_MENU:
#ifdef PBI_BB
		PBI_BB_Menu();
#endif
		break;
	default:
		break;
	}
#endif /* BASIC */

#ifdef PBI_BB
	PBI_BB_Frame(); /* just to make the menu key go up automatically */
#endif
#if defined(PBI_XLD) || defined (VOICEBOX)
	VOTRAXSND_Frame(); /* for the Votrax */
#endif
	Devices_Frame();
#ifndef BASIC
	INPUT_Frame();
#endif
	GTIA_Frame();

#ifdef BASIC
	basic_frame();
#else /* BASIC */
	if (++refresh_counter >= Atari800_refresh_rate) {
		refresh_counter = 0;
#ifdef USE_CURSES
		curses_clear_screen();
#endif
#ifdef CURSES_BASIC
		basic_frame();
#else
		ANTIC_Frame(TRUE);
		INPUT_DrawMousePointer();
		Screen_DrawAtariSpeed(Atari_time());
		Screen_DrawDiskLED();
#endif /* CURSES_BASIC */
#ifdef DONT_DISPLAY
		Atari800_display_screen = FALSE;
#else
		Atari800_display_screen = TRUE;
#endif /* DONT_DISPLAY */
	}
	else {
#if defined(VERY_SLOW) || defined(CURSES_BASIC)
		basic_frame();
#else
		ANTIC_Frame(Atari800_collisions_in_skipped_frames);
#endif
		Atari800_display_screen = FALSE;
	}
#endif /* BASIC */
	POKEY_Frame();
#ifdef SOUND
	Sound_Update();
#endif
	Atari800_nframes++;
#ifdef BENCHMARK
	if (Atari800_nframes >= BENCHMARK) {
		double benchmark_time = Atari_time() - benchmark_start_time;
		Atari800_Exit(FALSE);
		printf("%d frames emulated in %.2f seconds\n", BENCHMARK, benchmark_time);
		exit(0);
	}
#else

#ifdef ALTERNATE_SYNC_WITH_HOST
	if (refresh_counter == 0)
#endif
		if (Atari800_turbo == FALSE) Atari800_Sync();
#endif /* BENCHMARK */
}

#endif /* __PLUS */

#ifndef BASIC

void Atari800_StateSave(void)
{
	UBYTE temp;
	int default_tv_mode;
	int os = 0;
	int default_system = 3;
	int pil_on = FALSE;

	if (Atari800_tv_mode == Atari800_TV_PAL) {
		temp = 0;
		default_tv_mode = 1;
	}
	else {
		temp = 1;
		default_tv_mode = 2;
	}
	StateSav_SaveUBYTE(&temp, 1);

	switch (Atari800_machine_type) {
	case Atari800_MACHINE_OSA:
		temp = MEMORY_ram_size == 16 ? 5 : 0;
		os = 1;
		default_system = 1;
		break;
	case Atari800_MACHINE_OSB:
		temp = MEMORY_ram_size == 16 ? 5 : 0;
		os = 2;
		default_system = 2;
		break;
	case Atari800_MACHINE_XLXE:
		switch (MEMORY_ram_size) {
		case 16:
			temp = 6;
			default_system = 3;
			break;
		case 64:
			temp = 1;
			default_system = 3;
			break;
		case 128:
			temp = 2;
			default_system = 4;
			break;
		case 192:
			temp = 9;
			default_system = 8;
			break;
		case MEMORY_RAM_320_RAMBO:
		case MEMORY_RAM_320_COMPY_SHOP:
			temp = 3;
			default_system = 5;
			break;
		case 576:
			temp = 7;
			default_system = 6;
			break;
		case 1088:
			temp = 8;
			default_system = 7;
			break;
		}
		break;
	case Atari800_MACHINE_5200:
		temp = 4;
		default_system = 6;
		break;
	}
	StateSav_SaveUBYTE(&temp, 1);

	StateSav_SaveINT(&os, 1);
	StateSav_SaveINT(&pil_on, 1);
	StateSav_SaveINT(&default_tv_mode, 1);
	StateSav_SaveINT(&default_system, 1);
}

void Atari800_StateRead(void)
{
	int new_tv_mode;
	/* these are all for compatibility with previous versions */
	UBYTE temp;
	int default_tv_mode;
	int os;
	int default_system;
	int pil_on;

	StateSav_ReadUBYTE(&temp, 1);
	new_tv_mode = (temp == 0) ? Atari800_TV_PAL : Atari800_TV_NTSC;
	Atari800_SetTVMode(new_tv_mode);

	StateSav_ReadUBYTE(&temp, 1);
	StateSav_ReadINT(&os, 1);
	switch (temp) {
	case 0:
		Atari800_machine_type = os == 1 ? Atari800_MACHINE_OSA : Atari800_MACHINE_OSB;
		MEMORY_ram_size = 48;
		break;
	case 1:
		Atari800_machine_type = Atari800_MACHINE_XLXE;
		MEMORY_ram_size = 64;
		break;
	case 2:
		Atari800_machine_type = Atari800_MACHINE_XLXE;
		MEMORY_ram_size = 128;
		break;
	case 3:
		Atari800_machine_type = Atari800_MACHINE_XLXE;
		MEMORY_ram_size = MEMORY_RAM_320_COMPY_SHOP;
		break;
	case 4:
		Atari800_machine_type = Atari800_MACHINE_5200;
		MEMORY_ram_size = 16;
		break;
	case 5:
		Atari800_machine_type = os == 1 ? Atari800_MACHINE_OSA : Atari800_MACHINE_OSB;
		MEMORY_ram_size = 16;
		break;
	case 6:
		Atari800_machine_type = Atari800_MACHINE_XLXE;
		MEMORY_ram_size = 16;
		break;
	case 7:
		Atari800_machine_type = Atari800_MACHINE_XLXE;
		MEMORY_ram_size = 576;
		break;
	case 8:
		Atari800_machine_type = Atari800_MACHINE_XLXE;
		MEMORY_ram_size = 1088;
		break;
	case 9:
		Atari800_machine_type = Atari800_MACHINE_XLXE;
		MEMORY_ram_size = 192;
		break;
	default:
		Atari800_machine_type = Atari800_MACHINE_XLXE;
		MEMORY_ram_size = 64;
		Log_print("Warning: Bad machine type read in from state save, defaulting to 800 XL");
		break;
	}

	StateSav_ReadINT(&pil_on, 1);
	StateSav_ReadINT(&default_tv_mode, 1);
	StateSav_ReadINT(&default_system, 1);
	load_roms();
	/* XXX: what about patches? */
}

#endif

void Atari800_SetTVMode(int mode)
{
	if (mode != Atari800_tv_mode) {
		Atari800_tv_mode = mode;
#if !defined(BASIC) && !defined(CURSES_BASIC)
		Colours_SetVideoSystem(mode);
#endif
#if SUPPORTS_CHANGE_VIDEOMODE
		VIDEOMODE_SetVideoSystem(mode);
#endif
#if defined(SOUND) && defined(SUPPORTS_SOUND_REINIT)
		Sound_Reinit();
#endif
#if defined(DIRECTX)
		SetTVModeMenuItem(mode);
#endif
	}
}
