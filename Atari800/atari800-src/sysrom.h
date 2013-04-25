#ifndef SYSROM_H_
#define SYSROM_H_

#include "atari.h"

/* ROM IDs for all supported ROM images. */
enum {
	/* --- OS ROMs from released Atari computers --- */
	/* OS rev. A (1979) from early NTSC 400/800. Part no. C012499A + C014599A + C012399B */
	SYSROM_A_NTSC,
	/* OS rev. A (1979) from PAL 400/800. Part no. C015199 + C015299 + C012399B */
	SYSROM_A_PAL,
	/* OS rev. B (1981) from late NTSC 400/800. Part no. C012499B + C014599B + C012399B */
	SYSROM_B_NTSC,
	/* OS rev. 10 (1982-10-26) from 1200XL. Part no. C060616A + C060617A */
	SYSROM_AA00R10,
	/* OS rev. 11 (1982-12-23) from 1200XL. Part no. C060616B + C060617B */
	SYSROM_AA01R11,
	/* OS rev. 1 (1983-03-11) from 600XL. Part no. C062024 */
	SYSROM_BB00R1,
	/* OS rev. 2 (1983-05-10) from 800XL and early 65XE/130XE. Part no. C061598B */
	SYSROM_BB01R2,
	/* OS rev. 3 (1984-03-23) from prototype 1450XLD. Known as 1540OS3.V0 and 1450R3V0.ROM */
	SYSROM_BB02R3,
	/* OS rev. 3 ver. 4 (1984-06-21) from prototype 1450XLD. Known as os1450.128 and 1450R3VX.ROM */
	SYSROM_BB02R3V4,
	/* OS rev. 5 ver. 0 (1984-09-06) compiled from sources:
	   http://www.atariage.com/forums/topic/78579-a800ossrc/page__view__findpost__p__961535 */
	SYSROM_CC01R4,
	/* OS rev. 3 (1985-03-01) from late 65XE/130XE. Part no. C300717 */
	SYSROM_BB01R3,
	/* OS rev. 4 (1987-05-07) from XEGS - OS only. Part no. C101687 (2nd half) */
	SYSROM_BB01R4_OS,
	/* OS rev. 59 (1987-07-21) from Arabic 65XE. Part no. C101700 */
	SYSROM_BB01R59,
	/* OS rev. 59 (1987-07-21) from Kevin Savetz' Arabic 65XE (prototype?):
	   http://www.savetz.com/vintagecomputers/arabic65xe/ */
	SYSROM_BB01R59A,
	/* --- BIOS ROMs from Atari 5200 --- */
	/* BIOS from 4-port and early 2-port 5200 (1982). Part no. C019156 */
	SYSROM_5200,
	/* BIOS from late 2-port 5200 (1983). Part no. C019156A */
	SYSROM_5200A,
	/* --- Atari BASIC ROMs --- */
	/* Rev. A (1979), sold on cartridge. Part no. C012402 + C014502 */
	SYSROM_BASIC_A,
	/* Rev. B (1983), from 600XL/early 800XL, also on cartridge. Part no. C060302A */
	SYSROM_BASIC_B,
	/* Rev. C (1984), from late 800XL and all XE/XEGS, also on cartridge, Part no. C024947A */
	SYSROM_BASIC_C,
	/* builtin XEGS Missile Command. Part no. C101687 (1st quarter) */
	SYSROM_XEGAME,
	/* --- Custom ROMs --- */
	SYSROM_800_CUSTOM, /* Custom 400/800 OS */
	SYSROM_XL_CUSTOM, /* Custom XL/XE OS */
	SYSROM_5200_CUSTOM, /* Custom 5200 BIOS */
	SYSROM_BASIC_CUSTOM,/* Custom BASIC */
	SYSROM_XEGAME_CUSTOM, /* Custom XEGS game */
	SYSROM_SIZE, /* Number of available OS ROMs */
	SYSROM_AUTO = SYSROM_SIZE /* Use to indicate that OS revision should be chosen automatically */
};
typedef struct SYSROM_t {
	char *filename; /* Path to the ROM image file */
	size_t size; /* Expected size of the ROM image */
	ULONG crc32; /* Expected CRC32 of the ROM image */
	int unset; /* During initialisation indicates that no filename was given for this ROM image in config/command line */
} SYSROM_t;

/* Table of all supported ROM images with their sizes and CRCs, indexed by ROM IDs. */
extern SYSROM_t SYSROM_roms[SYSROM_SIZE];

/* OS version preference chosen by user. Indexed by value of Atari800_machine_type.
   Set these to SYSROM_AUTO to let the emulator choose the OS revision automatically. */
extern int SYSROM_os_versions[Atari800_MACHINE_SIZE];
/* BASIC version preference chosen by user. Set this to SYSROM_AUTO to let the emulator
   choose the BASIC revision automatically. */
extern int SYSROM_basic_version;

/* XEGS game version preference chosen by user. Set this to SYSROM_AUTO to let the emulator
   choose the game ROM automatically. */
extern int SYSROM_xegame_version;

/* Values returned by SYSROM_SetPath(). */
enum{
	SYSROM_OK, /* No error. */
	SYSROM_ERROR, /* File read error */
	SYSROM_BADSIZE, /* File has inappropriate size */
	SYSROM_BADCRC /* File has invalid checksum */
};

/* Set path to a ROM image. The ... parameters are ROM IDs (NUM indicates
   number of parameters). The file pointed by FILENAME is checked against the
   expected size and CRC for each of the roms in ..., and the path is set to
   the first match. See the above enum for possible return values. */
int SYSROM_SetPath(char const *filename, int num, ...);

/* Find ROM images in DIRECTORY, checking each file against known file sizes
   and CRCs. For each ROM for which a matching file is found, its filename is
   updated.
   If ONLY_IF_NOT_SET is TRUE, only paths that weren't given in the config
   file/command line are updated.
   Returns FALSE if it couldn't open DIRECTORY; otherwise returns TRUE. */
int SYSROM_FindInDir(char const *directory, int only_if_not_set);

/* Return ROM ID of the "best" available OS ROM for a given machine_type/
   ram_size/tv_system. If no OS for the given system is available, returns -1;
   otherwise returns a ROM ID. */
int SYSROM_AutoChooseOS(int machine_type, int ram_size, int tv_system);
/* Return ROM ID of the "best" available BASIC ROM. If no BASIC ROM is
   available, returns -1; otherwise returns a ROM ID. */
int SYSROM_AutoChooseBASIC(void);
/* Return ROM ID of the "best" available XEGS game ROM. If no game ROM is
   available, returns -1; otherwise returns a ROM ID. */
int SYSROM_AutoChooseXEGame(void);

/* Called after initialisation ended. Fills the filenames with default values,
   so that when writing the config file all config options will be written,
   making edits to the config file easier. */
void SYSROM_SetDefaults(void);

/* Called from Atari800_InitialiseMachine(). Determines OS BASIC, and XEGS game ROM
   versions based on given machine_type, ram_size and tv_system, and depending
   on settings in SYSROM_os_versions, SYSROM_basic_version and SYSROM_xegame_version.
   Returns OS version in *os_version,  BASIC version in *basic_version and XEGS game
   version in *xegame_version - all can be -1 if no ROM is available or no path
   is configured for the chosen ROM.
 */
void SYSROM_ChooseROMs(int machine_type, int ram_size, int tv_system, int *os_version, int *basic_version, int *xegame_version);

/* Read/write from/to configuration file. */
int SYSROM_ReadConfig(char *string, char *ptr);
void SYSROM_WriteConfig(FILE *fp);

/* Processing of command line arguments and initialisation of the module. */
int SYSROM_Initialise(int *argc, char *argv[]);

#endif /* SYSROM_H_ */
