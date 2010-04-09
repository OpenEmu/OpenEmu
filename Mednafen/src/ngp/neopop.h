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

#ifndef __NEOPOP__
#define __NEOPOP__
//=============================================================================

#include "../mednafen.h"
#include "../mempatcher.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "TLCS900h_disassemble.h"
#include "TLCS900h_interpret_dst.h"
#include "TLCS900h_interpret.h"
#include "TLCS900h_interpret_reg.h"
#include "TLCS900h_interpret_single.h"
#include "TLCS900h_interpret_src.h"
#include "TLCS900h_registers.h"


// I put the TLCS900h code in its own namespace, so it doesn't
// pollute the global namespace with all of its CARAZZZY short-named global variables.
// (I'm too lazy to clean up and turn it into an object :b)
using namespace TLCS900H;

//=============================================================================


//===========================
// GCC specific 
//===========================

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

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
	uint8* data;		//Pointer to the rom data
	uint8 *orig_data;	// Original data(without flash writes during emulation; necessary for save states)

	uint32 length;	//Length of the rom

	uint8 name[16];	//Null terminated string, holding the Game name
}
RomInfo;

//RomHeader
typedef struct
{
	uint8		licence[28];		// 0x00 - 0x1B
	uint32	startPC;			// 0x1C - 0x1F
	uint16	catalog;			// 0x20 - 0x21
	uint8		subCatalog;			// 0x22
	uint8		mode;				// 0x23
	uint8		name[12];			// 0x24 - 0x2F

	uint32	reserved1;			// 0x30 - 0x33
	uint32	reserved2;			// 0x34 - 0x37
	uint32	reserved3;			// 0x38 - 0x3B
	uint32	reserved4;			// 0x3C - 0x3F
} __attribute__((__packed__)) RomHeader;

//=============================================================================

//-----------------------------------------------------------------------------
// Core <--> System-Main Interface
//-----------------------------------------------------------------------------

	void reset(void);

/* Fill the bios rom area with a bios. call once at program start */
	bool bios_install(void);

	extern RomInfo ngpc_rom;

	extern RomHeader* rom_header;

/*!	Emulate a single instruction with correct TLCS900h:Z80 timing */

	void emulate(void);

/*! Call this function when a rom has just been loaded, it will perform
	the system independent actions required. */

	void rom_loaded(void);

/*!	Tidy up the rom and free the resources used. */

	void rom_unload(void);

		//=========================================

/*! Used to generate a critical message for the user. After the message
	has been displayed, the function should return. The message is not
	necessarily a fatal error. */
	
	void system_message(char* vaMessage,...);

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

	extern COLOURMODE system_colour;

	
//-----------------------------------------------------------------------------
// Core <--> System-Sound Interface
//-----------------------------------------------------------------------------

	// Speed of DAC playback
#define DAC_FREQUENCY		8000 //hz

	extern bool mute;

/*!	Fills the given buffer with sound data */

	void sound_update(uint16* chip_buffer, int length_bytes);
	void dac_update(uint8* dac_buffer, int length_bytes);

/*! Initialises the sound chips using the given SampleRate */
	
	void sound_init(int SampleRate);

		//=========================================

//-----------------------------------------------------------------------------
// Core <--> System-IO Interface
//-----------------------------------------------------------------------------

/*! Reads a byte from the other system. If no data is available or no
	high-level communications have been established, then return FALSE.
	If buffer is NULL, then no data is read, only status is returned */

	bool system_comms_read(uint8* buffer);


/*! Peeks at any data from the other system. If no data is available or
	no high-level communications have been established, then return FALSE.
	If buffer is NULL, then no data is read, only status is returned */

	bool system_comms_poll(uint8* buffer);


/*! Writes a byte from the other system. This function should block until
	the data is written. USE RELIABLE COMMS! Data cannot be re-requested. */

	void system_comms_write(uint8 data);


/*! Reads as much of the file specified by 'filename' into the given, 
	preallocated buffer. This is rom data */

	bool system_io_rom_read(char* filename, uint8* buffer, uint32 bufferLength);


/*! Reads the "appropriate" (system specific) flash data into the given
	preallocated buffer. The emulation core doesn't care where from. */

	bool system_io_flash_read(uint8* buffer, uint32 bufferLength);


/*! Writes the given flash data into an "appropriate" (system specific)
	place. The emulation core doesn't care where to. */

	bool system_io_flash_write(uint8* buffer, uint32 bufferLength);

void int_redo_icache(void);

#include "gfx.h"

extern NGPGFX_CLASS *NGPGfx;
extern uint8 NGPJoyLatch;
#endif
