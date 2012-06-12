/*
 * cfg.c - Emulator Configuration
 *
 * Copyright (c) 1995-1998 David Firth
 * Copyright (c) 1998-2010 Atari800 development team (see DOC/CREDITS)
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
#include "config.h"
#include "atari.h"
#include <stdlib.h>
#include "cfg.h"
#include "devices.h"
#include "esc.h"
#include "log.h"
#include "memory.h"
#include "pbi.h"
#ifdef AF80
#include "af80.h"
#endif
#include "platform.h"
#include "pokeysnd.h"
#include "ui.h"
#include "util.h"
#if !defined(BASIC) && !defined(CURSES_BASIC)
#include "colours.h"
#include "screen.h"
#endif
#ifdef NTSC_FILTER
#include "filter_ntsc.h"
#endif
#if SUPPORTS_CHANGE_VIDEOMODE
#include "videomode.h"
#endif

char CFG_osa_filename[FILENAME_MAX] = Util_FILENAME_NOT_SET;
char CFG_osb_filename[FILENAME_MAX] = Util_FILENAME_NOT_SET;
char CFG_xlxe_filename[FILENAME_MAX] = Util_FILENAME_NOT_SET;
char CFG_5200_filename[FILENAME_MAX] = Util_FILENAME_NOT_SET;
char CFG_basic_filename[FILENAME_MAX] = Util_FILENAME_NOT_SET;


void CFG_FindROMImages(const char *directory, int only_if_not_set)
{
	static char * const rom_filenames[5] = {
		CFG_osa_filename,
		CFG_osb_filename,
		CFG_xlxe_filename,
		CFG_5200_filename,
		CFG_basic_filename
	};
	static const char * const common_filenames[] = {
		"atariosa.rom", "atari_osa.rom", "atari_os_a.rom",
		"ATARIOSA.ROM", "ATARI_OSA.ROM", "ATARI_OS_A.ROM",
		NULL,
		"atariosb.rom", "atari_osb.rom", "atari_os_b.rom",
		"ATARIOSB.ROM", "ATARI_OSB.ROM", "ATARI_OS_B.ROM",
		NULL,
		"atarixlxe.rom", "atarixl.rom", "atari_xlxe.rom", "atari_xl_xe.rom",
		"ATARIXLXE.ROM", "ATARIXL.ROM", "ATARI_XLXE.ROM", "ATARI_XL_XE.ROM",
		NULL,
		"atari5200.rom", "atar5200.rom", "5200.rom", "5200.bin", "atari_5200.rom",
		"ATARI5200.ROM", "ATAR5200.ROM", "5200.ROM", "5200.BIN", "ATARI_5200.ROM",
		NULL,
		"ataribasic.rom", "ataribas.rom", "basic.rom", "atari_basic.rom",
		"ATARIBASIC.ROM", "ATARIBAS.ROM", "BASIC.ROM", "ATARI_BASIC.ROM",
		NULL
	};
	const char * const *common_filename = common_filenames;
	int i;
	for (i = 0; i < 5; i++) {
		if (!only_if_not_set || Util_filenamenotset(rom_filenames[i])) {
			do {
				char full_filename[FILENAME_MAX];
				Util_catpath(full_filename, directory, *common_filename);
				if (Util_fileexists(full_filename)) {
					strcpy(rom_filenames[i], full_filename);
					break;
				}
			} while (*++common_filename != NULL);
		}
		while (*common_filename++ != NULL);
	}
}

/* If another default path config path is defined use it
   otherwise use the default one */
#ifndef DEFAULT_CFG_NAME
#define DEFAULT_CFG_NAME ".atari800.cfg"
#endif

#ifndef SYSTEM_WIDE_CFG_FILE
#define SYSTEM_WIDE_CFG_FILE "/etc/atari800.cfg"
#endif

static char rtconfig_filename[FILENAME_MAX];

int CFG_LoadConfig(const char *alternate_config_filename)
{
	FILE *fp;
	const char *fname = rtconfig_filename;
	char string[256];
#ifndef BASIC
	int was_obsolete_dir = FALSE;
#endif

#ifdef SUPPORTS_PLATFORM_CONFIGINIT
	PLATFORM_ConfigInit();
#endif

	/* if alternate config filename is passed then use it */
	if (alternate_config_filename != NULL && *alternate_config_filename > 0) {
		Util_strlcpy(rtconfig_filename, alternate_config_filename, FILENAME_MAX);
	}
	/* else use the default config name under the HOME folder */
	else {
		char *home = getenv("HOME");
		if (home != NULL)
			Util_catpath(rtconfig_filename, home, DEFAULT_CFG_NAME);
		else
			strcpy(rtconfig_filename, DEFAULT_CFG_NAME);
	}

	fp = fopen(fname, "r");
	if (fp == NULL) {
		Log_print("User config file '%s' not found.", rtconfig_filename);

#ifdef SYSTEM_WIDE_CFG_FILE
		/* try system wide config file */
		fname = SYSTEM_WIDE_CFG_FILE;
		Log_print("Trying system wide config file: %s", fname);
		fp = fopen(fname, "r");
#endif
		if (fp == NULL) {
			Log_print("No configuration file found, will create fresh one from scratch:");
			return FALSE;
		}
	}

	if (fgets(string, sizeof(string), fp) != NULL) {
		Log_print("Using Atari800 config file: %s\nCreated by %s", fname, string);
	}

	while (fgets(string, sizeof(string), fp)) {
		char *ptr;
		Util_chomp(string);
		ptr = strchr(string, '=');
		if (ptr != NULL) {
			*ptr++ = '\0';
			Util_trim(string);
			Util_trim(ptr);

			if (strcmp(string, "OS/A_ROM") == 0)
				Util_strlcpy(CFG_osa_filename, ptr, sizeof(CFG_osa_filename));
			else if (strcmp(string, "OS/B_ROM") == 0)
				Util_strlcpy(CFG_osb_filename, ptr, sizeof(CFG_osb_filename));
			else if (strcmp(string, "XL/XE_ROM") == 0)
				Util_strlcpy(CFG_xlxe_filename, ptr, sizeof(CFG_xlxe_filename));
			else if (strcmp(string, "BASIC_ROM") == 0)
				Util_strlcpy(CFG_basic_filename, ptr, sizeof(CFG_basic_filename));
			else if (strcmp(string, "5200_ROM") == 0)
				Util_strlcpy(CFG_5200_filename, ptr, sizeof(CFG_5200_filename));
#ifdef BASIC
			else if (strcmp(string, "ATARI_FILES_DIR") == 0
				  || strcmp(string, "SAVED_FILES_DIR") == 0
				  || strcmp(string, "DISK_DIR") == 0 || strcmp(string, "ROM_DIR") == 0
				  || strcmp(string, "EXE_DIR") == 0 || strcmp(string, "STATE_DIR") == 0)
				/* do nothing */;
#else
			else if (strcmp(string, "ATARI_FILES_DIR") == 0) {
				if (UI_n_atari_files_dir >= UI_MAX_DIRECTORIES)
					Log_print("All ATARI_FILES_DIR slots used!");
				else
					Util_strlcpy(UI_atari_files_dir[UI_n_atari_files_dir++], ptr, FILENAME_MAX);
			}
			else if (strcmp(string, "SAVED_FILES_DIR") == 0) {
				if (UI_n_saved_files_dir >= UI_MAX_DIRECTORIES)
					Log_print("All SAVED_FILES_DIR slots used!");
				else
					Util_strlcpy(UI_saved_files_dir[UI_n_saved_files_dir++], ptr, FILENAME_MAX);
			}
			else if (strcmp(string, "DISK_DIR") == 0 || strcmp(string, "ROM_DIR") == 0
				  || strcmp(string, "EXE_DIR") == 0 || strcmp(string, "STATE_DIR") == 0) {
				/* ignore blank and "." values */
				if (ptr[0] != '\0' && (ptr[0] != '.' || ptr[1] != '\0'))
					was_obsolete_dir = TRUE;
			}
#endif
			else if (strcmp(string, "H1_DIR") == 0)
				Util_strlcpy(Devices_atari_h_dir[0], ptr, FILENAME_MAX);
			else if (strcmp(string, "H2_DIR") == 0)
				Util_strlcpy(Devices_atari_h_dir[1], ptr, FILENAME_MAX);
			else if (strcmp(string, "H3_DIR") == 0)
				Util_strlcpy(Devices_atari_h_dir[2], ptr, FILENAME_MAX);
			else if (strcmp(string, "H4_DIR") == 0)
				Util_strlcpy(Devices_atari_h_dir[3], ptr, FILENAME_MAX);
			else if (strcmp(string, "HD_READ_ONLY") == 0)
				Devices_h_read_only = Util_sscandec(ptr);

			else if (strcmp(string, "PRINT_COMMAND") == 0) {
				if (!Devices_SetPrintCommand(ptr))
					Log_print("Unsafe PRINT_COMMAND ignored");
			}

			else if (strcmp(string, "SCREEN_REFRESH_RATIO") == 0)
				Atari800_refresh_rate = Util_sscandec(ptr);
			else if (strcmp(string, "DISABLE_BASIC") == 0)
				Atari800_disable_basic = Util_sscanbool(ptr);

			else if (strcmp(string, "ENABLE_SIO_PATCH") == 0) {
				ESC_enable_sio_patch = Util_sscanbool(ptr);
			}
			else if (strcmp(string, "ENABLE_H_PATCH") == 0) {
				Devices_enable_h_patch = Util_sscanbool(ptr);
			}
			else if (strcmp(string, "ENABLE_P_PATCH") == 0) {
				Devices_enable_p_patch = Util_sscanbool(ptr);
			}
			else if (strcmp(string, "ENABLE_R_PATCH") == 0) {
				Devices_enable_r_patch = Util_sscanbool(ptr);
			}

			else if (strcmp(string, "ENABLE_NEW_POKEY") == 0) {
#ifdef SOUND
#ifndef SYNCHRONIZED_SOUND
				POKEYSND_enable_new_pokey = Util_sscanbool(ptr);
#endif /* SYNCHRONIZED_SOUND */
#endif /* SOUND */
			}
			else if (strcmp(string, "STEREO_POKEY") == 0) {
#ifdef STEREO_SOUND
				POKEYSND_stereo_enabled = Util_sscanbool(ptr);
#endif
			}
			else if (strcmp(string, "SPEAKER_SOUND") == 0) {
#ifdef CONSOLE_SOUND
				POKEYSND_console_sound_enabled = Util_sscanbool(ptr);
#endif
			}
			else if (strcmp(string, "SERIO_SOUND") == 0) {
#ifdef SERIO_SOUND
				POKEYSND_serio_sound_enabled = Util_sscanbool(ptr);
#endif
			}
			else if (strcmp(string, "MACHINE_TYPE") == 0) {
				if (strcmp(ptr, "Atari OS/A") == 0)
					Atari800_machine_type = Atari800_MACHINE_OSA;
				else if (strcmp(ptr, "Atari OS/B") == 0)
					Atari800_machine_type = Atari800_MACHINE_OSB;
				else if (strcmp(ptr, "Atari XL/XE") == 0)
					Atari800_machine_type = Atari800_MACHINE_XLXE;
				else if (strcmp(ptr, "Atari 5200") == 0)
					Atari800_machine_type = Atari800_MACHINE_5200;
				else
					Log_print("Invalid machine type: %s", ptr);
			}
			else if (strcmp(string, "RAM_SIZE") == 0) {
				if (strcmp(ptr, "16") == 0)
					MEMORY_ram_size = 16;
				else if (strcmp(ptr, "48") == 0)
					MEMORY_ram_size = 48;
				else if (strcmp(ptr, "52") == 0)
					MEMORY_ram_size = 52;
				else if (strcmp(ptr, "64") == 0)
					MEMORY_ram_size = 64;
				else if (strcmp(ptr, "128") == 0)
					MEMORY_ram_size = 128;
				else if (strcmp(ptr, "192") == 0)
					MEMORY_ram_size = 192;
				else if (strcmp(ptr, "320 (RAMBO)") == 0)
					MEMORY_ram_size = MEMORY_RAM_320_RAMBO;
				else if (strcmp(ptr, "320 (COMPY SHOP)") == 0)
					MEMORY_ram_size = MEMORY_RAM_320_COMPY_SHOP;
				else if (strcmp(ptr, "576") == 0)
					MEMORY_ram_size = 576;
				else if (strcmp(ptr, "1088") == 0)
					MEMORY_ram_size = 1088;
				else
					Log_print("Invalid RAM size: %s", ptr);
			}
			else if (strcmp(string, "DEFAULT_TV_MODE") == 0) {
				if (strcmp(ptr, "PAL") == 0)
					Atari800_tv_mode = Atari800_TV_PAL;
				else if (strcmp(ptr, "NTSC") == 0)
					Atari800_tv_mode = Atari800_TV_NTSC;
				else
					Log_print("Invalid TV Mode: %s", ptr);
			}
			/* Add module-specific configurations here */
			else if (PBI_ReadConfig(string,ptr)) {
			}
#ifdef AF80
			else if (AF80_ReadConfig(string,ptr)) {
			}
#endif
#if !defined(BASIC) && !defined(CURSES_BASIC)
			else if (Colours_ReadConfig(string, ptr)) {
			}
			else if (Screen_ReadConfig(string, ptr)) {
			}
#endif
#ifdef NTSC_FILTER
			else if (FILTER_NTSC_ReadConfig(string, ptr)) {
			}
#endif
#if SUPPORTS_CHANGE_VIDEOMODE
			else if (VIDEOMODE_ReadConfig(string, ptr)) {
			}
#endif
			else {
#ifdef SUPPORTS_PLATFORM_CONFIGURE
				if (!PLATFORM_Configure(string, ptr)) {
					Log_print("Unrecognized variable or bad parameters: '%s=%s'", string, ptr);
				}
#else
				Log_print("Unrecognized variable: %s", string);
#endif
			}
		}
		else {
			Log_print("Ignored config line: %s", string);
		}
	}

	fclose(fp);
#ifndef BASIC
	if (was_obsolete_dir) {
		Log_print(
			"DISK_DIR, ROM_DIR, EXE_DIR and STATE_DIR configuration options\n"
			"are no longer supported. Please use ATARI_FILES_DIR\n"
			"and SAVED_FILES_DIR in your Atari800 configuration file.");
	}
#endif
	return TRUE;
}

int CFG_WriteConfig(void)
{
	FILE *fp;
	int i;
	static const char * const machine_type_string[4] = {
		"OS/A", "OS/B", "XL/XE", "5200"
	};

	fp = fopen(rtconfig_filename, "w");
	if (fp == NULL) {
		perror(rtconfig_filename);
		Log_print("Cannot write to config file: %s", rtconfig_filename);
		return FALSE;
	}
	Log_print("Writing config file: %s", rtconfig_filename);

	fprintf(fp, "%s\n", Atari800_TITLE);
	fprintf(fp, "OS/A_ROM=%s\n", CFG_osa_filename);
	fprintf(fp, "OS/B_ROM=%s\n", CFG_osb_filename);
	fprintf(fp, "XL/XE_ROM=%s\n", CFG_xlxe_filename);
	fprintf(fp, "BASIC_ROM=%s\n", CFG_basic_filename);
	fprintf(fp, "5200_ROM=%s\n", CFG_5200_filename);
#ifndef BASIC
	for (i = 0; i < UI_n_atari_files_dir; i++)
		fprintf(fp, "ATARI_FILES_DIR=%s\n", UI_atari_files_dir[i]);
	for (i = 0; i < UI_n_saved_files_dir; i++)
		fprintf(fp, "SAVED_FILES_DIR=%s\n", UI_saved_files_dir[i]);
#endif
	for (i = 0; i < 4; i++)
		fprintf(fp, "H%c_DIR=%s\n", '1' + i, Devices_atari_h_dir[i]);
	fprintf(fp, "HD_READ_ONLY=%d\n", Devices_h_read_only);

#ifdef HAVE_SYSTEM
	fprintf(fp, "PRINT_COMMAND=%s\n", Devices_print_command);
#endif

#ifndef BASIC
	fprintf(fp, "SCREEN_REFRESH_RATIO=%d\n", Atari800_refresh_rate);
#endif

	fprintf(fp, "MACHINE_TYPE=Atari %s\n", machine_type_string[Atari800_machine_type]);

	fprintf(fp, "RAM_SIZE=");
	switch (MEMORY_ram_size) {
	case MEMORY_RAM_320_RAMBO:
		fprintf(fp, "320 (RAMBO)\n");
		break;
	case MEMORY_RAM_320_COMPY_SHOP:
		fprintf(fp, "320 (COMPY SHOP)\n");
		break;
	default:
		fprintf(fp, "%d\n", MEMORY_ram_size);
		break;
	}

	fprintf(fp, (Atari800_tv_mode == Atari800_TV_PAL) ? "DEFAULT_TV_MODE=PAL\n" : "DEFAULT_TV_MODE=NTSC\n");

	fprintf(fp, "DISABLE_BASIC=%d\n", Atari800_disable_basic);
	fprintf(fp, "ENABLE_SIO_PATCH=%d\n", ESC_enable_sio_patch);
	fprintf(fp, "ENABLE_H_PATCH=%d\n", Devices_enable_h_patch);
	fprintf(fp, "ENABLE_P_PATCH=%d\n", Devices_enable_p_patch);
#ifdef R_IO_DEVICE
	fprintf(fp, "ENABLE_R_PATCH=%d\n", Devices_enable_r_patch);
#endif

#ifdef SOUND
#ifndef SYNCHRONIZED_SOUND
	fprintf(fp, "ENABLE_NEW_POKEY=%d\n", POKEYSND_enable_new_pokey);
#endif /* SYNCHRONIZED_SOUND */
#ifdef STEREO_SOUND
	fprintf(fp, "STEREO_POKEY=%d\n", POKEYSND_stereo_enabled);
#endif
#ifdef CONSOLE_SOUND
	fprintf(fp, "SPEAKER_SOUND=%d\n", POKEYSND_console_sound_enabled);
#endif
#ifdef SERIO_SOUND
	fprintf(fp, "SERIO_SOUND=%d\n", POKEYSND_serio_sound_enabled);
#endif
#endif /* SOUND */
	/* Add module-specific configurations here */
	PBI_WriteConfig(fp);
#ifdef AF80
	AF80_WriteConfig(fp);
#endif
#if !defined(BASIC) && !defined(CURSES_BASIC)
	Colours_WriteConfig(fp);
	Screen_WriteConfig(fp);
#endif
#ifdef NTSC_FILTER
	FILTER_NTSC_WriteConfig(fp);
#endif
#if SUPPORTS_CHANGE_VIDEOMODE
	VIDEOMODE_WriteConfig(fp);
#endif
#ifdef SUPPORTS_PLATFORM_CONFIGSAVE
	PLATFORM_ConfigSave(fp);
#endif
	fclose(fp);
	return TRUE;
}

int CFG_MatchTextParameter(char const *param, char const * const cfg_strings[], int cfg_strings_size)
{
	int i;
	for (i = 0; i < cfg_strings_size; i ++) {
		if (Util_stricmp(param, cfg_strings[i]) == 0)
			return i;
	}
	/* Unrecognised value */
	return -1;
}

/*
vim:ts=4:sw=4:
*/
