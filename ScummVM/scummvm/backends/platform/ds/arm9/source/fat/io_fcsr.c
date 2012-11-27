/*
	io_fcsr.c based on

	compact_flash.c
	By chishm (Michael Chisholm)

	Hardware Routines for using a GBA Flash Cart and SRAM as a
	block device.

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.

	The file system must be 512 byte aligned, in cart address space.
	SRAM is supported.
*/


#include "io_fcsr.h"

#ifdef SUPPORT_FCSR
#include <string.h>

//---------------------------------------------------------------
// DMA
#ifdef _CF_USE_DMA
 #ifndef NDS
  #include "gba_dma.h"
 #else
  #include <nds/dma.h>
  #ifdef ARM9
   #include <nds/arm9/cache.h>
  #endif
 #endif
#endif

#ifdef NDS
 #define SRAM_START 0x0A000000
#else
 #define SRAM_START 0x0E000000
#endif

#define NO_SRAM 0xFFFFFFFF

#define FCSR 0x52534346
const char FCSR_LabelString[] = " Chishm FAT";

u8* FCSR_FileSysPointer = 0;
u8* FCSR_SramSectorPointer[4] = {0,0,0,0};
u32 FCSR_SramSectorStart[4] = {0,0,0,0};
u32 FCSR_SramSectorEnd[4] = {0,0,0,0};

/*-----------------------------------------------------------------
FCSR_IsInserted
Is a GBA Flash Cart with a valid file system inserted?
bool return OUT:  true if a GBA FC card is inserted
-----------------------------------------------------------------*/
bool FCSR_IsInserted (void)
{
	bool flagFoundFileSys = false;

	u32* fileSysPointer = (u32*)0x08000100;		// Start at beginning of cart address space, offset by expected location of string

	// Search for file system
	while ((fileSysPointer < (u32*)0x0A000000) && !flagFoundFileSys)	// Only search while not at end of cart address space
	{
		while ((*fileSysPointer != FCSR) && (fileSysPointer < (u32*)0x0A000000))
			fileSysPointer += 0x40;
		if ((strncmp(FCSR_LabelString, (char*)(fileSysPointer + 1), 12) == 0) && (fileSysPointer < (u32*)0x0A000000))
		{
			flagFoundFileSys = true;
		} else {
			fileSysPointer += 0x80;
		}
	}

	return flagFoundFileSys;
}


/*-----------------------------------------------------------------
FCSR_ClearStatus
Finish any pending operations
bool return OUT:  always true for GBA FC
-----------------------------------------------------------------*/
bool FCSR_ClearStatus (void)
{
	return true;
}


/*-----------------------------------------------------------------
FCSR_ReadSectors
Read 512 byte sector numbered "sector" into "buffer"
u32 sector IN: address of first 512 byte sector on Flash Cart to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer OUT: pointer to 512 byte buffer to store data in
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool FCSR_ReadSectors (u32 sector, u8 numSecs, void* buffer)
{
	int i;
	bool flagSramSector = false;
	int numSectors = (numSecs > 0 ? numSecs : 256);
	int readLength = numSectors * BYTE_PER_READ;
	u8* src;
	u8* dst;

	// Find which region this read is in
	for (i = 0; (i < 4) && !flagSramSector; i++)
	{
		if ((sector >= FCSR_SramSectorStart[i]) && (sector < FCSR_SramSectorEnd[i]))
		{
			flagSramSector = true;
			break;
		}
	}

	// Make sure read will be completely in SRAM range if it is partially there
	if ( flagSramSector && ((sector + numSectors) > FCSR_SramSectorEnd[i]))
		return false;

	// Copy data to buffer
	if (flagSramSector)
	{
		src = FCSR_SramSectorPointer[i] + (sector - FCSR_SramSectorStart[i]) * BYTE_PER_READ;
	} else {
		src = FCSR_FileSysPointer + sector * BYTE_PER_READ;
	}
	dst = (u8*)buffer;

	if (flagSramSector)
	{
		while (readLength--)
		{
			*dst++ = *src++;
		}
	} else {	// Reading from Cart ROM

#ifdef _CF_USE_DMA
 #ifdef NDS
  #ifdef ARM9
		DC_FlushRange( buffer, readLength);
  #endif	// ARM9
		DMA3_SRC = (u32)src;
		DMA3_DEST = (u32)buffer;
		DMA3_CR = (readLength >> 1) | DMA_COPY_HALFWORDS;
 #else	// ! NDS
		DMA3COPY ( src, buffer, (readLength >> 1) | DMA16 | DMA_ENABLE);
 #endif	// NDS
#else	// !_CF_USE_DMA
		memcpy (buffer, src, readLength);
#endif	// _CF_USE_DMA

	}	// if (flagSramSector)

	return true;
}

/*-----------------------------------------------------------------
FCSR_WriteSectors
Write 512 byte sector numbered "sector" from "buffer"
u32 sector IN: address of 512 byte sector on Flash Cart to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer IN: pointer to 512 byte buffer to read data from
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool FCSR_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{
	int i;
	bool flagSramSector = false;
	int writeLength = (numSecs > 0 ? numSecs : 256) * BYTE_PER_READ;
	u8* src = (u8*) buffer;
	u8* dst;

	// Find which region this sector belongs in
	for (i = 0; (i < 4) && !flagSramSector; i++)
	{
		if ((sector >= FCSR_SramSectorStart[i]) && (sector < FCSR_SramSectorEnd[i]))
		{
			flagSramSector = true;
			break;
		}
	}

	if (!flagSramSector)
		return false;

	// Entire write must be within an SRAM region
	if ((sector + (numSecs > 0 ? numSecs : 256)) > FCSR_SramSectorEnd[i])
		return false;

	// Copy data to SRAM
	dst = FCSR_SramSectorPointer[i] + (sector - FCSR_SramSectorStart[i]) * BYTE_PER_READ;
	while (writeLength--)
	{
		*dst++ = *src++;
	}

	return true;
}

/*-----------------------------------------------------------------
FCSR_Shutdown
unload the Flash Cart interface
-----------------------------------------------------------------*/
bool FCSR_Shutdown(void)
{
	int i;
	if (FCSR_ClearStatus() == false)
		return false;

	FCSR_FileSysPointer = 0;

	for (i=0; i < 4; i++)
	{
		FCSR_SramSectorPointer[i] = 0;
		FCSR_SramSectorStart[i] = 0;
		FCSR_SramSectorEnd[i] = 0;
	}
	return true;
}

/*-----------------------------------------------------------------
FCSR_StartUp
initializes the Flash Cart interface, returns true if successful,
otherwise returns false
-----------------------------------------------------------------*/
bool FCSR_StartUp(void)
{
	bool flagFoundFileSys = false;
	int i;
	int SramRegionSize[4];
	u8* srcByte;
	u8* destByte;

	u32* fileSysPointer = (u32*)0x08000100;		// Start at beginning of cart address space, offset by expected location of string

	// Search for file system
	while ((fileSysPointer < (u32*)0x0A000000) && !flagFoundFileSys)	// Only search while not at end of cart address space
	{
		while ((*fileSysPointer != FCSR) && (fileSysPointer < (u32*)0x0A000000))
			fileSysPointer += 0x40;
		if ((strncmp(FCSR_LabelString, (char*)(fileSysPointer + 1), 12) == 0) && (fileSysPointer < (u32*)0x0A000000))
		{
			flagFoundFileSys = true;
		} else {
			fileSysPointer += 0x80;
		}
	}

	if (!flagFoundFileSys)
		return false;

	// Flash cart file system pointer has been found
	FCSR_FileSysPointer = (u8*)(fileSysPointer - 0x40);

	// Get SRAM sector regions from header block
	for (i = 0; i < 4; i++)
	{
		FCSR_SramSectorStart[i] = fileSysPointer[i+4];
		SramRegionSize[i] = fileSysPointer[i+8];
		FCSR_SramSectorEnd[i] = FCSR_SramSectorStart[i] + SramRegionSize[i];
	}

	// Calculate SRAM region pointers
	FCSR_SramSectorPointer[0] = (u8*)(SRAM_START + 4);
	for (i = 1; i < 4; i++)
	{
		FCSR_SramSectorPointer[i] = FCSR_SramSectorPointer[i-1] + (SramRegionSize[i-1] * BYTE_PER_READ);
	}

	// Initialise SRAM with overlay if it hasn't been done so
	if ( (*((u8*)SRAM_START) != 'F')  || (*((u8*)(SRAM_START+1)) != 'C') || (*((u8*)(SRAM_START+2)) != 'S') || (*((u8*)(SRAM_START+3)) != 'R') )
	{
		*((u8*)SRAM_START) = 'F';
		*((u8*)(SRAM_START+1)) = 'C';
		*((u8*)(SRAM_START+2)) = 'S';
		*((u8*)(SRAM_START+3)) = 'R';

		for (i = 0; i < 4; i++)
		{
			srcByte = FCSR_FileSysPointer + (FCSR_SramSectorStart[i] * BYTE_PER_READ);
			destByte = FCSR_SramSectorPointer[i];
			while (srcByte < FCSR_FileSysPointer + (FCSR_SramSectorEnd[i] * BYTE_PER_READ) )
				*destByte++ = *srcByte++;
		}
	}

		// Get SRAM sector regions from header block
	for (i = 0; i < 4; i++)
	{
		if (SramRegionSize[i] == 0)
		{
			FCSR_SramSectorStart[i] = NO_SRAM;
			FCSR_SramSectorEnd[i] = NO_SRAM;
		}
	}

	return true;
}

/*-----------------------------------------------------------------
the actual interface structure
-----------------------------------------------------------------*/
IO_INTERFACE io_fcsr = {
	DEVICE_TYPE_FCSR,	// 'FCSR'
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&FCSR_StartUp,
	(FN_MEDIUM_ISINSERTED)&FCSR_IsInserted,
	(FN_MEDIUM_READSECTORS)&FCSR_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&FCSR_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&FCSR_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&FCSR_Shutdown
} ;

/*-----------------------------------------------------------------
FCSR_GetInterface
returns the interface structure to host
-----------------------------------------------------------------*/
LPIO_INTERFACE FCSR_GetInterface(void) {
	return &io_fcsr ;
} ;

#endif // SUPPORT_FCSR
