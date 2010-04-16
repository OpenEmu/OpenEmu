//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------
//=========================================================================

	neopop.h

  Defines the interface between the Core and the System. All of the
  definitions and declarations given here are used by both sides.

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- Tidied up and removed obsolete functions.
- Added declarations for the new IO functions.

26 JUL 2002 - neopop_uk
=======================================
- Modified the definition of the two flash io functions to
	include the fact that the system code governs the actual location of
	storage, all the core code needs is the data back.
- Added clearer naming for the colour mode options.

28 JUL 2002 - neopop_uk
=======================================
- Shortened the debugger window caption, so the speed percentage fits.

01 AUG 2002 - neopop_uk
=======================================
- Added support for frameskipping.

02 AUG 2002 - neopop_uk
=======================================
- Removed the build options - Quick/HLE is the only possibility now.

03 AUG 2002 - neopop_uk
=======================================
- Made 'mute' a common variable.
- Added more declarations to link the Core and System though this file.

09 AUG 2002 - neopop_uk
=======================================
- Added slightly better documentation

15 AUG 2002 - neopop_uk
=======================================
- Replaced 'bool', 'true' and 'false' with BOOL, TRUE and FALSE for
	better compatibility with other compilers.

18 AUG 2002 - neopop_uk
=======================================
- Moved RomInfo, RomHeader, rom and rom_header from rom.h
- Moved state_restore/state_store prototypes from state.h
- Moved RAM_START/RAM_END definition and ram[] declaration from ram.h
- Moved rom_loaded() prototype from rom.h
- Moved reset() and biosInstall (now bios_install) from bios.h

22 AUG 2002 - neopop_uk
=======================================
- Improved compatibility with GCC.

30 AUG 2002 - neopop_uk
=======================================
- Removed frameskipping.

06 SEP 2002 - neopop_uk
=======================================
- Removed the date section from NEOPOP_VERSION
- New comms function 'system_comms_poll'

08 SEP 2002 - neopop_uk
=======================================
- Added some more strings for translation.

10 SEP 2002 - neopop_uk
=======================================
- Added better sound functions, one for silence and one to reset the chips.

//---------------------------------------------------------------------------
*/

#ifndef __NEOPOP__
#define __NEOPOP__
//=============================================================================

#define NEOPOP_VERSION	"v0.71"

#ifdef NEOPOP_DEBUG
#define PROGRAM_NAME "NeoPop (D)"
#else
#define PROGRAM_NAME "NeoPop"
#endif

//=============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
/* Add this to deal with already defined bool type */
#include <objc/objc.h>

//=============================================================================


//===========================
// GCC specific 
//===========================
#ifdef __GNUC__

	typedef unsigned long long	_u64;
	typedef long long			_s64;

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

//===========================
#endif


//===========================
// Visual C specific 
//===========================
#ifdef _MSC_VER

	typedef unsigned __int64	_u64;
	typedef signed __int64		_s64;

//===========================
#endif


//-----------------------------------------------------------------------------
// Global Definitions
//-----------------------------------------------------------------------------

//TYPES
typedef unsigned char		_u8;
typedef unsigned short		_u16;
typedef unsigned int		_u32;
typedef	signed char			_s8;
typedef signed short		_s16;
typedef signed int			_s32;

//BOOL
/* Comment this to deal with already defined bool type */
//typedef char					BOOL;
#define TRUE				1
#define FALSE				0

//COLOURMODE
typedef enum
{
	COLOURMODE_GREYSCALE,
	COLOURMODE_COLOUR,
	COLOURMODE_AUTO
}
COLOURMODE;

//RomInfo
typedef struct 
{
	_u8* data;		//Pointer to the rom data
	_u32 length;	//Length of the rom

	_u8 name[16];	//Null terminated string, holding the Game name

	//For use as flash file and default state name
	_u8 filename[256];	
}
RomInfo;

//RomHeader
typedef struct
{
	_u8		licence[28];		// 0x00 - 0x1B
	_u32	startPC;			// 0x1C - 0x1F
	_u16	catalog;			// 0x20 - 0x21
	_u8		subCatalog;			// 0x22
	_u8		mode;				// 0x23
	_u8		name[12];			// 0x24 - 0x2F

	_u32	reserved1;			// 0x30 - 0x33
	_u32	reserved2;			// 0x34 - 0x37
	_u32	reserved3;			// 0x38 - 0x3B
	_u32	reserved4;			// 0x3C - 0x3F
}
RomHeader;

//=============================================================================

//-----------------------------------------------------------------------------
// Core <--> System-Main Interface
//-----------------------------------------------------------------------------

	void reset(void);

/* Fill the bios rom area with a bios. call once at program start */
	BOOL bios_install(void);

#define RAM_START	0x000000
#define RAM_END		0x00BFFF

	extern _u8 ram[1 + RAM_END - RAM_START];

	extern RomInfo rom;

	extern RomHeader* rom_header;

	// false = Japanese.
	extern BOOL language_english;

/*!	Emulate a single instruction with correct TLCS900h:Z80 timing */

	void emulate(void);

/*! Call this function when a rom has just been loaded, it will perform
	the system independent actions required. */

	void rom_loaded(void);

/*!	Tidy up the rom and free the resources used. */

	void rom_unload(void);

		//=========================================

	typedef enum 
	{
		IDS_DEFAULT,
		IDS_ROMFILTER,
		IDS_STATEFILTER,
		IDS_FLASHFILTER,
		IDS_BADFLASH,
		IDS_POWER,
		IDS_BADSTATE,
		IDS_ERROR1,		//Application init
		IDS_ERROR2,		//DirectDraw
		IDS_ERROR3,		//DirectInput
		IDS_TIMER,
		IDS_WRONGROM,
		IDS_EROMFIND,
		IDS_EROMOPEN,
		IDS_EZIPNONE,
		IDS_EZIPBAD,
		IDS_EZIPFIND,

		IDS_ABORT,
		IDS_DISCONNECT,
		IDS_CONNECTED,

		STRINGS_MAX,
	} 
	STRINGS;

/*! Get a string that may possibly be translated */

	char* system_get_string(STRINGS string_id);

/*! Used to generate a critical message for the user. After the message
	has been displayed, the function should return. The message is not
	necessarily a fatal error. */
	
	void __cdecl system_message(char* vaMessage,...);

/*! Called at the start of the vertical blanking period, this function is
	designed to perform many of the critical hardware interface updates
	Here is a list of recommended actions to take:
	
	- The frame buffer should be copied to the screen.
	- The frame rate should be throttled to 59.95hz
	- The sound chips should be polled for the next chunk of data
	- Input should be polled and the current status written to "ram[0x6F82]" */
	
	void system_VBL(void);


//-----------------------------------------------------------------------------
// Core <--> System-Graphics Interface
//-----------------------------------------------------------------------------

	// Physical screen dimensions
#define SCREEN_WIDTH	160
#define SCREEN_HEIGHT	152

	//16-bit Frame buffer: Format X4B4G4R4
	extern _u16 cfb[256*256];

	extern COLOURMODE system_colour;

/*! Increased in 'interrupt.c', never more than system_frameskip_key */

	extern _u8 frameskip_count;

		//=========================================

/*! The system sets this value to the number of the frameskip keyframe */

	extern _u8 system_frameskip_key;

	
//-----------------------------------------------------------------------------
// Core <--> System-Sound Interface
//-----------------------------------------------------------------------------

	// Speed of DAC playback
#define DAC_FREQUENCY		8000 //hz

	extern BOOL mute;

/*!	Fills the given buffer with sound data */

	void sound_update(_u16* chip_buffer, int length_bytes);
	void dac_update(_u8* dac_buffer, int length_bytes);

/*! Initialises the sound chips using the given SampleRate */
	
	void sound_init(int SampleRate);

		//=========================================

/*! Callback for "sound_init" with the system sound frequency */
	
	void system_sound_chipreset(void);

/*! Clears the sound output. */
	
	void system_sound_silence(void);


//-----------------------------------------------------------------------------
// Core <--> System-IO Interface
//-----------------------------------------------------------------------------

	void state_restore(const char *filename);
	void state_store(const char *filename);

		//=========================================

/*! Reads a byte from the other system. If no data is available or no
	high-level communications have been established, then return FALSE.
	If buffer is NULL, then no data is read, only status is returned */

	BOOL system_comms_read(_u8* buffer);


/*! Peeks at any data from the other system. If no data is available or
	no high-level communications have been established, then return FALSE.
	If buffer is NULL, then no data is read, only status is returned */

	BOOL system_comms_poll(_u8* buffer);


/*! Writes a byte from the other system. This function should block until
	the data is written. USE RELIABLE COMMS! Data cannot be re-requested. */

	void system_comms_write(_u8 data);


/*! Reads as much of the file specified by 'filename' into the given, 
	preallocated buffer. This is rom data */

	BOOL system_io_rom_read(const char* filename, _u8* buffer, _u32 bufferLength);


/*! Reads the "appropriate" (system specific) flash data into the given
	preallocated buffer. The emulation core doesn't care where from. */

	BOOL system_io_flash_read(_u8* buffer, _u32 bufferLength);


/*! Writes the given flash data into an "appropriate" (system specific)
	place. The emulation core doesn't care where to. */

	BOOL system_io_flash_write(_u8* buffer, _u32 bufferLength);


/*! Reads from the file specified by 'filename' into the given preallocated
	buffer. This is state data. */

	BOOL system_io_state_read(const char* filename, _u8* buffer, _u32 bufferLength);
	

/*! Writes to the file specified by 'filename' from the given buffer.
	This is state data. */

	BOOL system_io_state_write(const char* filename, _u8* buffer, _u32 bufferLength);


//-----------------------------------------------------------------------------
// Core <--> System-Debugger Interface
//-----------------------------------------------------------------------------

#ifdef NEOPOP_DEBUG

	//Debugger message filters
	//(TRUE = allow messages to be generated)
	extern BOOL filter_mem;
	extern BOOL filter_bios;
	extern BOOL filter_comms;
	extern BOOL filter_dma;
	extern BOOL filter_sound;


/*! Emulate a single instruction in Debug mode, checking for exceptions */

	void emulate_debug(BOOL dis_TLCS900h, BOOL dis_Z80);


/*!	Disassembles a single instruction from $PC, as TLCS-900h or Z80
	according to whether it lies in the 0x7000 - 0x7FFF region. 
	$PC is incremented to the start of the next instruction. */
	
	char* disassemble(void);

		//=========================================

/*!	Generates a debugger specific message that is not relevant to the
	main build. For example the memory module (mem.c) uses this function
	to indicate a memory exception */

	void __cdecl system_debug_message(char* vaMessage,...);


/*! This function pairs with the function above. This is used to associate
	an additional address to the last debug message. It was added so that
	later the message could be selected and an appropriate address retrieved.
	For example a memory exception message might have the address of the
	instruction that caused the error, this way the instruction could be
	displayed, or used as a breakpoint, etc. */

	void system_debug_message_associate_address(_u32 address);


/*!	Signals the debugger to stop execution of program code and await user
	instruction. This is called when resetting, or after a instruction or 
	memory error. */

	void system_debug_stop(void);


/*! Signals the debugger to refresh it's information as the system state
	has changed. For example after loading a saved state, a memory viewer
	will no longer be displaying current information. */

	void system_debug_refresh(void);
	

/*!	Indicates to the debugger that the instruction located at the current
	program counter value is about to be executed, and this address should
	be stored in the execution history. */

	void system_debug_history_add(void);
	

/*! Signals the debugger to clear it's list of instruction history. This
	is called by the reset function to correctly indicate that there have
	been no previous instructions executed. */

	void system_debug_history_clear(void);


/*! Clears the main debugger message display list */

	void system_debug_clear(void);

#endif

/*! Macros for big- and little-endian support (defaults to little-endian). */

#ifdef MSB_FIRST
#ifdef LSB_FIRST
#error Only define one of LSB_FIRST and MSB_FIRST
#endif

#ifndef le32toh
#define le32toh(l)	((((l)>>24) & 0xff) | (((l)>>8) & 0xff00) \
			 | (((l)<<8) & 0xff0000) | (((l)<<24) & 0xff000000))
#endif
#ifndef le16toh
#define le16toh(l)	((((l)>>8) & 0xff) | (((l)<<8) & 0xff00))
#endif
#else
#ifndef le32toh
#define le32toh(l)	(l)
#endif
#ifndef le16toh
#define le16toh(l)	(l)
#endif
#endif
#ifndef htole32
#define htole32	le32toh
#endif
#ifndef htole16
#define htole16	le16toh
#endif

//=============================================================================
#endif
