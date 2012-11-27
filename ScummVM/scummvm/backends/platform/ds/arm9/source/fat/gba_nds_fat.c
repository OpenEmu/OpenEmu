/*
	gba_nds_fat.c
	By chishm (Michael Chisholm)

	Routines for reading a compact flash card
	using the GBA Movie Player or M3.

	Some FAT routines are based on those in fat.c, which
	is part of avrlib by Pascal Stang.

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

//---------------------------------------------------------------
// Includes

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "gba_nds_fat.h"
#include "disc_io.h"
#include <string.h>
#ifdef NDS
// #include <nds/ipc.h>	// Time on the NDS
 #include <NDS/scummvm_ipc.h>
#endif
//----------------------------------------------------------------
// Data	types
#ifndef	NULL
 #define	NULL	0
#endif

//----------------------------------------------------------------
// NDS memory access control register
#ifdef NDS
 #ifndef WAIT_CR
  #define WAIT_CR (*(vu16*)0x04000204)
 #endif
#endif

//---------------------------------------------------------------
// Appropriate placement of CF functions and data
#ifdef NDS
 #define _VARS_IN_RAM
#else
 #define _VARS_IN_RAM __attribute__ ((section (".sbss")))
#endif


//-----------------------------------------------------------------
// FAT constants
#define CLUSTER_EOF_16	0xFFFF
#define	CLUSTER_EOF	0x0FFFFFFF
#define CLUSTER_FREE	0x0000
#define CLUSTER_FIRST	0x0002

#define FILE_LAST 0x00
#define FILE_FREE 0xE5

#define FAT16_ROOT_DIR_CLUSTER 0x00


//-----------------------------------------------------------------
// long file name constants
#define LFN_END 0x40
#define LFN_DEL 0x80

//-----------------------------------------------------------------
// Data Structures

// Take care of packing for GCC - it doesn't obey pragma pack()
// properly for ARM targets.
#ifdef __GNUC__
 #define __PACKED __attribute__ ((__packed__))
#else
 #define __PACKED
 #pragma pack(1)
#endif

// Boot Sector - must be packed
typedef struct
{
	u8	jmpBoot[3];
	u8	OEMName[8];
	// BIOS Parameter Block
	u16	bytesPerSector;
	u8	sectorsPerCluster;
	u16	reservedSectors;
	u8	numFATs;
	u16	rootEntries;
	u16	numSectorsSmall;
	u8	mediaDesc;
	u16	sectorsPerFAT;
	u16	sectorsPerTrk;
	u16	numHeads;
	u32	numHiddenSectors;
	u32	numSectors;
	union	// Different types of extended BIOS Parameter Block for FAT16 and FAT32
	{
		struct
		{
			// Ext BIOS Parameter Block for FAT16
			u8	driveNumber;
			u8	reserved1;
			u8	extBootSig;
			u32	volumeID;
			u8	volumeLabel[11];
			u8	fileSysType[8];
			// Bootcode
			u8	bootCode[448];
		}	__PACKED fat16;
		struct
		{
			// FAT32 extended block
			u32	sectorsPerFAT32;
			u16	extFlags;
			u16	fsVer;
			u32	rootClus;
			u16	fsInfo;
			u16	bkBootSec;
			u8	reserved[12];
			// Ext BIOS Parameter Block for FAT16
			u8	driveNumber;
			u8	reserved1;
			u8	extBootSig;
			u32	volumeID;
			u8	volumeLabel[11];
			u8	fileSysType[8];
			// Bootcode
			u8	bootCode[420];
		}	__PACKED fat32;
	}	__PACKED extBlock;

	u16	bootSig;

}	__PACKED BOOT_SEC;

// Directory entry - must be packed
typedef struct
{
	u8	name[8];
	u8	ext[3];
	u8	attrib;
	u8	reserved;
	u8	cTime_ms;
	u16	cTime;
	u16	cDate;
	u16	aDate;
	u16	startClusterHigh;
	u16	mTime;
	u16	mDate;
	u16	startCluster;
	u32	fileSize;
}	__PACKED DIR_ENT;

// Long file name directory entry - must be packed
typedef struct
{
	u8 ordinal;	// Position within LFN
	u16 char0;
	u16 char1;
	u16 char2;
	u16 char3;
	u16 char4;
	u8 flag;	// Should be equal to ATTRIB_LFN
	u8 reserved1;	// Always 0x00
	u8 checkSum;	// Checksum of short file name (alias)
	u16 char5;
	u16 char6;
	u16 char7;
	u16 char8;
	u16 char9;
	u16 char10;
	u16 reserved2;	// Always 0x0000
	u16 char11;
	u16 char12;
}	__PACKED DIR_ENT_LFN;

const char lfn_offset_table[13]={0x01,0x03,0x05,0x07,0x09,0x0E,0x10,0x12,0x14,0x16,0x18,0x1C,0x1E};

// End of packed structs
#ifdef __PACKED
 #undef __PACKED
#endif
#ifndef __GNUC__
 #pragma pack()
#endif

//-----------------------------------------------------------------
// Global Variables

// _VARS_IN_RAM variables are stored in the largest section of WRAM
// available: IWRAM on NDS ARM7, EWRAM on NDS ARM9 and GBA

// Files
FAT_FILE openFiles[MAX_FILES_OPEN] __attribute__((section(".itcm")));
//_VARS_IN_RAM

// Long File names
_VARS_IN_RAM char lfnName[MAX_FILENAME_LENGTH];
bool lfnExists;

// Locations on card
int filesysRootDir;
int filesysRootDirClus;
int filesysFAT;
int filesysSecPerFAT;
int filesysNumSec;
int filesysData;
int filesysBytePerSec;
int filesysSecPerClus;
int filesysBytePerClus;

FS_TYPE filesysType = FS_UNKNOWN;
u32 filesysTotalSize;

// Info about FAT
u32 fatLastCluster;
u32 fatFirstFree;

// fatBuffer used to reduce wear on the CF card from multiple writes
_VARS_IN_RAM char fatBuffer[BYTE_PER_READ];
u32 fatBufferCurSector;

// Current working directory
u32 curWorkDirCluster;

// Position of the directory entry last retreived with FAT_GetDirEntry
u32 wrkDirCluster;
int wrkDirSector;
int wrkDirOffset;

// Global sector buffer to save on stack space
_VARS_IN_RAM unsigned char globalBuffer[BYTE_PER_READ];

//-----------------------------------------------------------------
// Functions contained in this file - predeclarations
char ucase (char character);
u16 getRTCtoFileTime (void);
u16 getRTCtoFileDate (void);

bool FAT_AddDirEntry (const char* path, DIR_ENT newDirEntry);
bool FAT_ClearLinks (u32 cluster);
DIR_ENT FAT_DirEntFromPath (const char* path);
u32 FAT_FirstFreeCluster(void);
DIR_ENT FAT_GetDirEntry ( u32 dirCluster, int entry, int origin);
u32 FAT_LinkFreeCluster(u32 cluster);
u32 FAT_NextCluster(u32 cluster);
bool FAT_WriteFatEntry (u32 cluster, u32 value);
bool FAT_GetFilename (DIR_ENT dirEntry, char* alias);

bool FAT_InitFiles (void);
bool FAT_FreeFiles (void);
int FAT_remove (const char* path);
bool FAT_chdir (const char* path);
FILE_TYPE FAT_FindFirstFile (char* filename);
FILE_TYPE FAT_FindNextFile (char* filename);
FILE_TYPE FAT_FileExists (const char* filename);
bool FAT_GetAlias (char* alias);
bool FAT_GetLongFilename (char* filename);
u32 FAT_GetFileSize (void);
u32 FAT_GetFileCluster (void);

FAT_FILE* FAT_fopen(const char* path, const char* mode);
bool FAT_fclose (FAT_FILE* file);
bool FAT_feof(FAT_FILE* file);
int FAT_fseek(FAT_FILE* file, s32 offset, int origin);
u32 FAT_ftell (FAT_FILE* file);
u32 FAT_fread (void* buffer, u32 size, u32 count, FAT_FILE* file);
u32 FAT_fwrite (const void* buffer, u32 size, u32 count, FAT_FILE* file);
char FAT_fgetc (FAT_FILE* file);
char FAT_fputc (char c, FAT_FILE* file);

/*-----------------------------------------------------------------
ucase
Returns the uppercase version of the given char
char IN: a character
char return OUT: uppercase version of character
-----------------------------------------------------------------*/
char ucase (char character)
{
	if ((character > 0x60) && (character < 0x7B))
		character = character - 0x20;
	return (character);
}


/*-----------------------------------------------------------------
getRTCtoFileTime and getRTCtoFileDate
Returns the time / date in Dir Entry styled format
u16 return OUT: time / date in Dir Entry styled format
-----------------------------------------------------------------*/
u16 getRTCtoFileTime (void)
{
#ifdef NDS
	return (
		( ( (IPC->rtc.hours > 11 ? IPC->rtc.hours - 40 : IPC->rtc.hours) & 0x1F) << 11) |
		( (IPC->rtc.minutes & 0x3F) << 5) |
		( (IPC->rtc.seconds >> 1) & 0x1F) );
#else
	return 0;
#endif
}

u16 getRTCtoFileDate (void)
{
#ifdef NDS
	return (
		( ((IPC->rtc.year + 20) & 0x7F) <<9) |
		( (IPC->rtc.month & 0xF) << 5) |
		(IPC->rtc.day & 0x1F) );
#else
	return 0;
#endif
}


/*-----------------------------------------------------------------
Disc level FAT routines
-----------------------------------------------------------------*/
#define FAT_ClustToSect(m) \
	(((m-2) * filesysSecPerClus) + filesysData)

/*-----------------------------------------------------------------
FAT_NextCluster
Internal function - gets the cluster linked from input cluster
-----------------------------------------------------------------*/
u32 FAT_NextCluster(u32 cluster)
{
	u32 nextCluster = CLUSTER_FREE;
	u32 sector;
	int offset;

	switch (filesysType)
	{
		case FS_UNKNOWN:
			nextCluster = CLUSTER_FREE;
			break;

		case FS_FAT12:
			sector = filesysFAT + (((cluster * 3) / 2) / BYTE_PER_READ);
			offset = ((cluster * 3) / 2) % BYTE_PER_READ;

			// If FAT buffer contains wrong sector
			if (sector != fatBufferCurSector)
			{
				// Load correct sector to buffer
				fatBufferCurSector = sector;
				disc_ReadSector(fatBufferCurSector, fatBuffer);
			}

			nextCluster = ((u8*)fatBuffer)[offset];
			offset++;

			if (offset >= BYTE_PER_READ) {
				offset = 0;
				fatBufferCurSector++;
				disc_ReadSector(fatBufferCurSector, fatBuffer);
			}

			nextCluster |= (((u8*)fatBuffer)[offset]) << 8;

			if (cluster & 0x01) {
				nextCluster = nextCluster >> 4;
			} else 	{
				nextCluster &= 0x0FFF;
			}

			if (nextCluster >= 0x0FF7)
			{
				nextCluster = CLUSTER_EOF;
			}

			break;

		case FS_FAT16:
			sector = filesysFAT + ((cluster << 1) / BYTE_PER_READ);
			offset = cluster % (BYTE_PER_READ >> 1);

			// If FAT buffer contains wrong sector
			if (sector != fatBufferCurSector)
			{
				// Load correct sector to buffer
				fatBufferCurSector = sector;
				disc_ReadSector(fatBufferCurSector, fatBuffer);
			}

			// read the nextCluster value
			nextCluster = ((u16*)fatBuffer)[offset];

			if (nextCluster >= 0xFFF7)
			{
				nextCluster = CLUSTER_EOF;
			}
			break;

		case FS_FAT32:
			sector = filesysFAT + ((cluster << 2) / BYTE_PER_READ);
			offset = cluster % (BYTE_PER_READ >> 2);

			// If FAT buffer contains wrong sector
			if (sector != fatBufferCurSector)
			{
				// Load correct sector to buffer
				fatBufferCurSector = sector;
				disc_ReadSector(fatBufferCurSector, fatBuffer);
			}

			// read the nextCluster value
			nextCluster = (((u32*)fatBuffer)[offset]) & 0x0FFFFFFF;

			if (nextCluster >= 0x0FFFFFF7)
			{
				nextCluster = CLUSTER_EOF;
			}
			break;

		default:
			nextCluster = CLUSTER_FREE;
			break;
	}

	return nextCluster;
}

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_WriteFatEntry
Internal function - writes FAT information about a cluster
-----------------------------------------------------------------*/
bool FAT_WriteFatEntry (u32 cluster, u32 value)
{
	u32 sector;
	int offset;

	if ((cluster < 0x0002) || (cluster > fatLastCluster))
	{
		return false;
	}

	switch (filesysType)
	{
		case FS_UNKNOWN:
			return false;
			break;

		case FS_FAT12:
			sector = filesysFAT + (((cluster * 3) / 2) / BYTE_PER_READ);
			offset = ((cluster * 3) / 2) % BYTE_PER_READ;

			// If FAT buffer contains wrong sector
			if (sector != fatBufferCurSector)
			{
				// Load correct sector to buffer
				fatBufferCurSector = sector;
				disc_ReadSector(fatBufferCurSector, fatBuffer);
			}

			if (cluster & 0x01) {

				((u8*)fatBuffer)[offset] = (((u8*)fatBuffer)[offset] & 0x0F) | ((value & 0x0F) << 4);

				offset++;
				if (offset >= BYTE_PER_READ) {
					offset = 0;
					// write the buffer back to disc
					disc_WriteSector(fatBufferCurSector, fatBuffer);
					// read the next sector
					fatBufferCurSector++;
					disc_ReadSector(fatBufferCurSector, fatBuffer);
				}

				((u8*)fatBuffer)[offset] =  (value & 0x0FF0) >> 4;

			} else {

				((u8*)fatBuffer)[offset] = value & 0xFF;

				offset++;
				if (offset >= BYTE_PER_READ) {
					offset = 0;
					// write the buffer back to disc
					disc_WriteSector(fatBufferCurSector, fatBuffer);
					// read the next sector
					fatBufferCurSector++;
					disc_ReadSector(fatBufferCurSector, fatBuffer);
				}

				((u8*)fatBuffer)[offset] = (((u8*)fatBuffer)[offset] & 0xF0) | ((value >> 8) & 0x0F);
			}

			break;

		case FS_FAT16:
			sector = filesysFAT + ((cluster << 1) / BYTE_PER_READ);
			offset = cluster % (BYTE_PER_READ >> 1);

			// If FAT buffer contains wrong sector
			if (sector != fatBufferCurSector)
			{
				// Load correct sector to buffer
				fatBufferCurSector = sector;
				disc_ReadSector(fatBufferCurSector, fatBuffer);
			}

			// write the value to the FAT buffer
			((u16*)fatBuffer)[offset] = (value & 0xFFFF);

			break;

		case FS_FAT32:
			sector = filesysFAT + ((cluster << 2) / BYTE_PER_READ);
			offset = cluster % (BYTE_PER_READ >> 2);

			// If FAT buffer contains wrong sector
			if (sector != fatBufferCurSector)
			{
				// Load correct sector to buffer
				fatBufferCurSector = sector;
				disc_ReadSector(fatBufferCurSector, fatBuffer);
			}

			// write the value to the FAT buffer
			(((u32*)fatBuffer)[offset]) =  value;

			break;

		default:
			return false;
			break;
	}

	// write the buffer back to disc
	disc_WriteSector(fatBufferCurSector, fatBuffer);

	return true;
}
#endif

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_ReadWriteFatEntryBuffered
Internal function - writes FAT information about a cluster to a
 buffer that should then be flushed to disc using
 FAT_WriteFatEntryFlushBuffer()
 Call FAT_WriteFatEntry first so as not to ruin the disc.
 Also returns the entry being replaced
-----------------------------------------------------------------*/
u32 FAT_ReadWriteFatEntryBuffered (u32 cluster, u32 value)
{
	u32 sector;
	int offset;
	u32 oldValue;

	if ((cluster < 0x0002) || (cluster > fatLastCluster))
		return CLUSTER_FREE;


	switch (filesysType)
	{
		case FS_UNKNOWN:
			oldValue = CLUSTER_FREE;
			break;

		case FS_FAT12:
			sector = filesysFAT + (((cluster * 3) / 2) / BYTE_PER_READ);
			offset = ((cluster * 3) / 2) % BYTE_PER_READ;

			// If FAT buffer contains wrong sector
			if (sector != fatBufferCurSector)
			{
				// write the old buffer to disc
				if ((fatBufferCurSector >= filesysFAT) && (fatBufferCurSector < (filesysFAT + filesysSecPerFAT)))
					disc_WriteSector(fatBufferCurSector, fatBuffer);
				// Load correct sector to buffer
				fatBufferCurSector = sector;
				disc_ReadSector(fatBufferCurSector, fatBuffer);
			}

			if (cluster & 0x01) {

				oldValue = (((u8*)fatBuffer)[offset] & 0xF0) >> 4;
				((u8*)fatBuffer)[offset] = (((u8*)fatBuffer)[offset] & 0x0F) | ((value & 0x0F) << 4);

				offset++;
				if (offset >= BYTE_PER_READ) {
					offset = 0;
					// write the buffer back to disc
					disc_WriteSector(fatBufferCurSector, fatBuffer);
					// read the next sector
					fatBufferCurSector++;
					disc_ReadSector(fatBufferCurSector, fatBuffer);
				}

				oldValue |= ((((u8*)fatBuffer)[offset]) << 4) & 0x0FF0;
				((u8*)fatBuffer)[offset] =  (value & 0x0FF0) >> 4;

			} else {

				oldValue = ((u8*)fatBuffer)[offset] & 0xFF;
				((u8*)fatBuffer)[offset] = value & 0xFF;

				offset++;
				if (offset >= BYTE_PER_READ) {
					offset = 0;
					// write the buffer back to disc
					disc_WriteSector(fatBufferCurSector, fatBuffer);
					// read the next sector
					fatBufferCurSector++;
					disc_ReadSector(fatBufferCurSector, fatBuffer);
				}

				oldValue |= (((u8*)fatBuffer)[offset] & 0x0F) << 8;
				((u8*)fatBuffer)[offset] = (((u8*)fatBuffer)[offset] & 0xF0) | ((value >> 8) & 0x0F);
			}

			if (oldValue >= 0x0FF7)
			{
				oldValue = CLUSTER_EOF;
			}

			break;

		case FS_FAT16:
			sector = filesysFAT + ((cluster << 1) / BYTE_PER_READ);
			offset = cluster % (BYTE_PER_READ >> 1);

			// If FAT buffer contains wrong sector
			if (sector != fatBufferCurSector)
			{
				// write the old buffer to disc
				if ((fatBufferCurSector >= filesysFAT) && (fatBufferCurSector < (filesysFAT + filesysSecPerFAT)))
					disc_WriteSector(fatBufferCurSector, fatBuffer);
				// Load correct sector to buffer
				fatBufferCurSector = sector;
				disc_ReadSector(fatBufferCurSector, fatBuffer);
			}

			// write the value to the FAT buffer
			oldValue = ((u16*)fatBuffer)[offset];
			((u16*)fatBuffer)[offset] = value;

			if (oldValue >= 0xFFF7)
			{
				oldValue = CLUSTER_EOF;
			}

			break;

		case FS_FAT32:
			sector = filesysFAT + ((cluster << 2) / BYTE_PER_READ);
			offset = cluster % (BYTE_PER_READ >> 2);

			// If FAT buffer contains wrong sector
			if (sector != fatBufferCurSector)
			{
				// write the old buffer to disc
				if ((fatBufferCurSector >= filesysFAT) && (fatBufferCurSector < (filesysFAT + filesysSecPerFAT)))
					disc_WriteSector(fatBufferCurSector, fatBuffer);
				// Load correct sector to buffer
				fatBufferCurSector = sector;
				disc_ReadSector(fatBufferCurSector, fatBuffer);
			}

			// write the value to the FAT buffer
			oldValue = ((u32*)fatBuffer)[offset];
			((u32*)fatBuffer)[offset] =  value;

			if (oldValue >= 0x0FFFFFF7)
			{
				oldValue = CLUSTER_EOF;
			}

			break;

		default:
			oldValue = CLUSTER_FREE;
			break;
	}

	return oldValue;
}
#endif

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_WriteFatEntryFlushBuffer
Flush the FAT buffer back to the disc
-----------------------------------------------------------------*/
bool FAT_WriteFatEntryFlushBuffer (void)
{
	// write the buffer disc
	if ((fatBufferCurSector >= filesysFAT) && (fatBufferCurSector < (filesysFAT + filesysSecPerFAT)))
	{
		disc_WriteSector(fatBufferCurSector, fatBuffer);
		return true;
	} else {
		return false;
	}
}
#endif

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_FirstFreeCluster
Internal function - gets the first available free cluster
-----------------------------------------------------------------*/
u32 FAT_FirstFreeCluster(void)
{
	// Start at first valid cluster
	if (fatFirstFree < CLUSTER_FIRST)
		fatFirstFree = CLUSTER_FIRST;

	while ((FAT_NextCluster(fatFirstFree) != CLUSTER_FREE) && (fatFirstFree <= fatLastCluster))
	{
		fatFirstFree++;
	}
	if (fatFirstFree > fatLastCluster)
	{
		return CLUSTER_EOF;
	}
	return fatFirstFree;
}
#endif

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_LinkFreeCluster
Internal function - gets the first available free cluster, sets it
to end of file, links the input cluster to it then returns the
cluster number
-----------------------------------------------------------------*/
u32 FAT_LinkFreeCluster(u32 cluster)
{
	u32 firstFree;
	u32 curLink;

	if (cluster > fatLastCluster)
	{
		return CLUSTER_FREE;
	}

	// Check if the cluster already has a link, and return it if so
	curLink = FAT_NextCluster (cluster);
	if ((curLink >= CLUSTER_FIRST) && (curLink < fatLastCluster))
	{
		return curLink;	// Return the current link - don't allocate a new one
	}

	// Get a free cluster
	firstFree = FAT_FirstFreeCluster();

	// If couldn't get a free cluster then return
	if (firstFree == CLUSTER_EOF)
	{
		return CLUSTER_FREE;
	}

	if ((cluster >= CLUSTER_FIRST) && (cluster < fatLastCluster))
	{
		// Update the linked from FAT entry
		FAT_WriteFatEntry (cluster, firstFree);
	}
	// Create the linked to FAT entry
	FAT_WriteFatEntry (firstFree, CLUSTER_EOF);

	return firstFree;
}
#endif


#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_ClearLinks
Internal function - frees any cluster used by a file
-----------------------------------------------------------------*/
bool FAT_ClearLinks (u32 cluster)
{
	u32 nextCluster;

	if ((cluster < 0x0002) || (cluster > fatLastCluster))
		return false;

	// Store next cluster before erasing the link
	nextCluster = FAT_NextCluster (cluster);

	// Erase the link
	FAT_WriteFatEntry (cluster, CLUSTER_FREE);

	// Move onto next cluster
	cluster = nextCluster;

	while ((cluster != CLUSTER_EOF) && (cluster != CLUSTER_FREE))
	{
		cluster = FAT_ReadWriteFatEntryBuffered (cluster, CLUSTER_FREE);
	}

	// Flush fat write buffer
	FAT_WriteFatEntryFlushBuffer ();

	return true;
}
#endif


/*-----------------------------------------------------------------
FAT_InitFiles
Reads the FAT information from the CF card.
You need to call this before reading any files.
bool return OUT: true if successful.
-----------------------------------------------------------------*/
bool FAT_InitFiles (void)
{
	int i;
	int bootSector;
	BOOT_SEC* bootSec;

	if (!disc_Init())
	{
		return (false);
	}
	// Read first sector of CF card
	if ( !disc_ReadSector(0, globalBuffer)) {
		return false;
	}


	// Make sure it is a valid MBR or boot sector
/*	if ( (globalBuffer[0x1FE] != 0x55) || (globalBuffer[0x1FF] != 0xAA)) {
		return false;
	}*/



	// Check if there is a FAT string, which indicates this is a boot sector
	if ((globalBuffer[0x36] == 'F') && (globalBuffer[0x37] == 'A') && (globalBuffer[0x38] == 'T'))
	{
		bootSector = 0;
	}
	// Check for FAT32
	else if ((globalBuffer[0x52] == 'F') && (globalBuffer[0x53] == 'A') && (globalBuffer[0x54] == 'T'))
	{
		bootSector = 0;
	}
	else	// This is an MBR
	{
		// Find first valid partition from MBR
		// First check for an active partition
		for (i=0x1BE; (i < 0x1FE) && (globalBuffer[i] != 0x80); i+= 0x10);
		// If it didn't find an active partition, search for any valid partition
		if (i == 0x1FE)
			for (i=0x1BE; (i < 0x1FE) && (globalBuffer[i+0x04] == 0x00); i+= 0x10);

		// Go to first valid partition
		if ( i != 0x1FE)	// Make sure it found a partition
		{
			bootSector = globalBuffer[0x8 + i] + (globalBuffer[0x9 + i] << 8) + (globalBuffer[0xA + i] << 16) + ((globalBuffer[0xB + i] << 24) & 0x0F);
		} else {
			bootSector = 0;	// No partition found, assume this is a MBR free disk
		}
	}

	// Read in boot sector
	bootSec = (BOOT_SEC*) globalBuffer;
	if (!disc_ReadSector (bootSector,  bootSec)) {
		return false;
	}

	// Store required information about the file system
	if (bootSec->sectorsPerFAT != 0)
	{
		filesysSecPerFAT = bootSec->sectorsPerFAT;
	}
	else
	{
		filesysSecPerFAT = bootSec->extBlock.fat32.sectorsPerFAT32;
	}

	if (bootSec->numSectorsSmall != 0)
	{
		filesysNumSec = bootSec->numSectorsSmall;
	}
	else
	{
		filesysNumSec = bootSec->numSectors;
	}

	filesysBytePerSec = BYTE_PER_READ;	// Sector size is redefined to be 512 bytes
	filesysSecPerClus = bootSec->sectorsPerCluster * bootSec->bytesPerSector / BYTE_PER_READ;
	filesysBytePerClus = filesysBytePerSec * filesysSecPerClus;
	filesysFAT = bootSector + bootSec->reservedSectors;

	filesysRootDir = filesysFAT + (bootSec->numFATs * filesysSecPerFAT);
	filesysData = filesysRootDir + ((bootSec->rootEntries * sizeof(DIR_ENT)) / filesysBytePerSec);

	filesysTotalSize = (filesysNumSec - filesysData) * filesysBytePerSec;

	// Store info about FAT
	fatLastCluster = (filesysNumSec - filesysData) / bootSec->sectorsPerCluster;
	fatFirstFree = CLUSTER_FIRST;
	fatBufferCurSector = 0;
	disc_ReadSector(fatBufferCurSector, fatBuffer);

	if (fatLastCluster < 4085)
	{
		filesysType = FS_FAT12;	// FAT12 volume - unsupported
	}
	else if (fatLastCluster < 65525)
	{
		filesysType = FS_FAT16;	// FAT16 volume
	}
	else
	{
		filesysType = FS_FAT32;	// FAT32 volume
	}

	if (filesysType != FS_FAT32)
	{
		filesysRootDirClus = FAT16_ROOT_DIR_CLUSTER;
	}
	else	// Set up for the FAT32 way
	{
		filesysRootDirClus = bootSec->extBlock.fat32.rootClus;
		// Check if FAT mirroring is enabled
		if (!(bootSec->extBlock.fat32.extFlags & 0x80))
		{
			// Use the active FAT
			filesysFAT = filesysFAT + ( filesysSecPerFAT * (bootSec->extBlock.fat32.extFlags & 0x0F));
		}
	}

	// Set current directory to the root
	curWorkDirCluster = filesysRootDirClus;
	wrkDirCluster = filesysRootDirClus;
	wrkDirSector = 0;
	wrkDirOffset = 0;

	// Set all files to free
	for (i=0; i < MAX_FILES_OPEN; i++)
	{
		openFiles[i].inUse = false;
	}

	// No long filenames so far
	lfnExists = false;
	for (i = 0; i < MAX_FILENAME_LENGTH; i++)
	{
		lfnName[i] = '\0';
	}

	return (true);
}

/*-----------------------------------------------------------------
FAT_FreeFiles
Closes all open files then resets the CF card.
Call this before exiting back to the GBAMP
bool return OUT: true if successful.
-----------------------------------------------------------------*/
bool FAT_FreeFiles (void)
{
	int i;

	// Close all open files
	for (i=0; i < MAX_FILES_OPEN; i++)
	{
		if (openFiles[i].inUse == true)
		{
			FAT_fclose(&openFiles[i]);
		}
	}

	// Flush any sectors in disc cache
	disc_CacheFlush();

	// Clear card status
	disc_ClearStatus();

	// Return status of card
	return disc_IsInserted();
}


/*-----------------------------------------------------------------
FAT_GetDirEntry
Return the file info structure of the next valid file entry
u32 dirCluster: IN cluster of subdirectory table
int entry: IN the desired file entry
int origin IN: relative position of the entry
DIR_ENT return OUT: desired dirEntry. First char will be FILE_FREE if
	the entry does not exist.
-----------------------------------------------------------------*/
DIR_ENT FAT_GetDirEntry ( u32 dirCluster, int entry, int origin)
{
	DIR_ENT dir;
	DIR_ENT_LFN lfn;
	int firstSector = 0;
	bool notFound = false;
	bool found = false;
	int maxSectors;
	int lfnPos, aliasPos;
	u8 lfnChkSum, chkSum;

	int i;

	dir.name[0] = FILE_FREE; // default to no file found
	dir.attrib = 0x00;

	// Check if fat has been initialized
	if (filesysBytePerSec == 0)
	{
		return (dir);
	}

	switch (origin)
	{
	case SEEK_SET:
		wrkDirCluster = dirCluster;
		wrkDirSector = 0;
		wrkDirOffset = -1;
		break;
	case SEEK_CUR:	// Don't change anything
		break;
	case SEEK_END:	// Find entry signifying end of directory
		// Subtraction will never reach 0, so it keeps going
		// until reaches end of directory
		wrkDirCluster = dirCluster;
		wrkDirSector = 0;
		wrkDirOffset = -1;
		entry = -1;
		break;
	default:
		return dir;
	}

	lfnChkSum = 0;
	maxSectors = (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? (filesysData - filesysRootDir) : filesysSecPerClus);

	// Scan Dir for correct entry
	firstSector = (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster));
	disc_ReadSector (firstSector + wrkDirSector, globalBuffer);
	found = false;
	notFound = false;
	do {
		wrkDirOffset++;
		if (wrkDirOffset == BYTE_PER_READ / sizeof (DIR_ENT))
		{
			wrkDirOffset = 0;
			wrkDirSector++;
			if ((wrkDirSector == filesysSecPerClus) && (wrkDirCluster != FAT16_ROOT_DIR_CLUSTER))
			{
				wrkDirSector = 0;
				wrkDirCluster = FAT_NextCluster(wrkDirCluster);
				if (wrkDirCluster == CLUSTER_EOF)
				{
					notFound = true;
				}
				firstSector = FAT_ClustToSect(wrkDirCluster);
			}
			else if ((wrkDirCluster == FAT16_ROOT_DIR_CLUSTER) && (wrkDirSector == (filesysData - filesysRootDir)))
			{
				notFound = true;	// Got to end of root dir
			}
			disc_ReadSector (firstSector + wrkDirSector, globalBuffer);
		}
		dir = ((DIR_ENT*) globalBuffer)[wrkDirOffset];
		if ((dir.name[0] != FILE_FREE) && (dir.name[0] > 0x20) && ((dir.attrib & ATTRIB_VOL) != ATTRIB_VOL))
		{
			entry--;
			if (lfnExists)
			{
				// Calculate file checksum
				chkSum = 0;
				for (aliasPos=0; aliasPos < 11; aliasPos++)
				{
					// NOTE: The operation is an unsigned char rotate right
					chkSum = ((chkSum & 1) ? 0x80 : 0) + (chkSum >> 1) + (aliasPos < 8 ? dir.name[aliasPos] : dir.ext[aliasPos - 8]);
				}
				if (chkSum != lfnChkSum)
				{
					lfnExists = false;
					lfnName[0] = '\0';
				}
			}
			if (entry == 0)
			{
				if (!lfnExists)
				{
					FAT_GetFilename (dir, lfnName);
				}
				found = true;
			}
		}
		else if (dir.name[0] == FILE_LAST)
		{
			if (origin == SEEK_END)
			{
				found = true;
			}
			else
			{
				notFound = true;
			}
		}
		else if (dir.attrib == ATTRIB_LFN)
		{
			lfn = ((DIR_ENT_LFN*) globalBuffer)[wrkDirOffset];
			if (lfn.ordinal & LFN_DEL)
			{
				lfnExists = false;
			}
			else if (lfn.ordinal & LFN_END)	// Last part of LFN, make sure it isn't deleted (Thanks MoonLight)
			{
				lfnExists = true;
				lfnName[(lfn.ordinal & ~LFN_END) * 13] = '\0';	// Set end of lfn to null character
				lfnChkSum = lfn.checkSum;
			}
			if (lfnChkSum != lfn.checkSum)
			{
				lfnExists = false;
			}
			if (lfnExists)
			{
				lfnPos = ((lfn.ordinal & ~LFN_END) - 1) * 13;
				for (i = 0; i < 13; i++) {
					lfnName[lfnPos + i] = ((u8*)&lfn)[(int)(lfn_offset_table[i])] /* | ((u8*)&lfn)[(int)(lfn_offset_table[i]) + 1]  include this for unicode support*/;
				}
			}
		}
	} while (!found && !notFound);

	// If no file is found, return FILE_FREE
	if (notFound)
	{
		dir.name[0] = FILE_FREE;
	}

	return (dir);
}


/*-----------------------------------------------------------------
FAT_GetLongFilename
Get the long name of the last file or directory retrived with
	GetDirEntry. Also works for FindFirstFile and FindNextFile.
	If a long name doesn't exist, it returns the short name
	instead.
char* filename: OUT will be filled with the filename, should be at
	least 256 bytes long
bool return OUT: return true if successful
-----------------------------------------------------------------*/
bool FAT_GetLongFilename (char* filename)
{
	if (filename == NULL)
		return false;

	strncpy (filename, lfnName, MAX_FILENAME_LENGTH - 1);
	filename[MAX_FILENAME_LENGTH - 1] = '\0';

	return true;
}


/*-----------------------------------------------------------------
FAT_GetFilename
Get the alias (short name) of the file or directory stored in
	dirEntry
DIR_ENT dirEntry: IN a valid directory table entry
char* alias OUT: will be filled with the alias (short filename),
	should be at least 13 bytes long
bool return OUT: return true if successful
-----------------------------------------------------------------*/
bool FAT_GetFilename (DIR_ENT dirEntry, char* alias)
{
	int i=0;
	int j=0;

	alias[0] = '\0';
	if (dirEntry.name[0] != FILE_FREE)
	{
		if (dirEntry.name[0] == '.')
		{
			alias[0] = '.';
			if (dirEntry.name[1] == '.')
			{
				alias[1] = '.';
				alias[2] = '\0';
			}
			else
			{
				alias[1] = '\0';
			}
		}
		else
		{
			// Copy the filename from the dirEntry to the string
			for (i = 0; (i < 8) && (dirEntry.name[i] != ' '); i++)
			{
				alias[i] = dirEntry.name[i];
			}
			// Copy the extension from the dirEntry to the string
			if (dirEntry.ext[0] != ' ')
			{
				alias[i++] = '.';
				for ( j = 0; (j < 3) && (dirEntry.ext[j] != ' '); j++)
				{
					alias[i++] = dirEntry.ext[j];
				}
			}
			alias[i] = '\0';
		}
	}

	return (alias[0] != '\0');
}

/*-----------------------------------------------------------------
FAT_GetAlias
Get the alias (short name) of the last file or directory entry read
	using GetDirEntry. Works for FindFirstFile and FindNextFile
char* alias OUT: will be filled with the alias (short filename),
	should be at least 13 bytes long
bool return OUT: return true if successful
-----------------------------------------------------------------*/
bool FAT_GetAlias (char* alias)
{
	if (alias == NULL)
	{
		return false;
	}
	// Read in the last accessed directory entry
	disc_ReadSector ((wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector, globalBuffer);

	return 	FAT_GetFilename (((DIR_ENT*)globalBuffer)[wrkDirOffset], alias);
}

/*-----------------------------------------------------------------
FAT_GetFileSize
Get the file size of the last file found or openned.
This idea is based on a modification by MoonLight
u32 return OUT: the file size
-----------------------------------------------------------------*/
u32 FAT_GetFileSize (void)
{
	// Read in the last accessed directory entry
	disc_ReadSector ((wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector, globalBuffer);

	return 	((DIR_ENT*)globalBuffer)[wrkDirOffset].fileSize;
}

/*-----------------------------------------------------------------
FAT_GetFileCluster
Get the first cluster of the last file found or openned.
u32 return OUT: the file start cluster
-----------------------------------------------------------------*/
u32 FAT_GetFileCluster (void)
{
	// Read in the last accessed directory entry
	disc_ReadSector ((wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector, globalBuffer);

	return 	(((DIR_ENT*)globalBuffer)[wrkDirOffset].startCluster) | (((DIR_ENT*)globalBuffer)[wrkDirOffset].startClusterHigh << 16);
}

/*-----------------------------------------------------------------
FAT_GetFileAttributes
Get the attributes of the last file found or openned.
u8 return OUT: the file's attributes
-----------------------------------------------------------------*/
u8 FAT_GetFileAttributes (void)
{
	// Read in the last accessed directory entry
	disc_ReadSector ((wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector, globalBuffer);

	return 	((DIR_ENT*)globalBuffer)[wrkDirOffset].attrib;
}

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_SetFileAttributes
Set the attributes of a file.
const char* filename IN: The name and path of the file to modify
u8 attributes IN: The attribute values to assign
u8 mask IN: Detemines which attributes are changed
u8 return OUT: the file's new attributes
-----------------------------------------------------------------*/
u8 FAT_SetFileAttributes (const char* filename, u8 attributes, u8 mask)
{
	// Get the file
	if (!FAT_FileExists(filename)) {
		return 0xff;
	}

	// Read in the last accessed directory entry
	disc_ReadSector ((wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector, globalBuffer);

	((DIR_ENT*)globalBuffer)[wrkDirOffset].attrib = (((DIR_ENT*)globalBuffer)[wrkDirOffset].attrib & ~(mask & 0x27)) | (attributes & 0x27);	// 0x27 is he settable attributes

	disc_WriteSector ((wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector, globalBuffer);

	return 	((DIR_ENT*)globalBuffer)[wrkDirOffset].attrib;
}
#endif

#ifdef FILE_TIME_SUPPORT
time_t FAT_FileTimeToCTime (u16 fileTime, u16 fileDate)
{
	struct tm timeInfo;

	timeInfo.tm_year = (fileDate >> 9) + 80;		// years since midnight January 1970
	timeInfo.tm_mon = ((fileDate >> 5) & 0xf) - 1;	// Months since january
	timeInfo.tm_mday = fileDate & 0x1f;				// Day of the month

	timeInfo.tm_hour = fileTime >> 11;				// hours past midnight
	timeInfo.tm_min = (fileTime >> 5) & 0x3f;		// minutes past the hour
	timeInfo.tm_sec = (fileTime & 0x1f) * 2;		// seconds past the minute

	return mktime(&timeInfo);
}

/*-----------------------------------------------------------------
FAT_GetFileCreationTime
Get the creation time of the last file found or openned.
time_t return OUT: the file's creation time
-----------------------------------------------------------------*/
time_t FAT_GetFileCreationTime (void)
{
	// Read in the last accessed directory entry
	disc_ReadSector ((wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector, globalBuffer);

	return 	FAT_FileTimeToCTime(((DIR_ENT*)globalBuffer)[wrkDirOffset].cTime, ((DIR_ENT*)globalBuffer)[wrkDirOffset].cDate);
}

/*-----------------------------------------------------------------
FAT_GetFileLastWriteTime
Get the creation time of the last file found or openned.
time_t return OUT: the file's creation time
-----------------------------------------------------------------*/
time_t FAT_GetFileLastWriteTime (void)
{
	// Read in the last accessed directory entry
	disc_ReadSector ((wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector, globalBuffer);

	return 	FAT_FileTimeToCTime(((DIR_ENT*)globalBuffer)[wrkDirOffset].mTime, ((DIR_ENT*)globalBuffer)[wrkDirOffset].mDate);
}
#endif

/*-----------------------------------------------------------------
FAT_DirEntFromPath
Finds the directory entry for a file or directory from a path
Path separator is a forward slash /
const char* path: IN null terminated string of path.
DIR_ENT return OUT: dirEntry of found file. First char will be FILE_FREE
	if the file was not found
-----------------------------------------------------------------*/
DIR_ENT FAT_DirEntFromPath (const char* path)
{
	int pathPos;
	char name[MAX_FILENAME_LENGTH];
	char alias[13];
	int namePos;
	bool found, notFound;
	DIR_ENT dirEntry;
	u32 dirCluster;
	bool flagLFN, dotSeen;
	// Start at beginning of path
	pathPos = 0;

#ifdef DS_BUILD_F
	// Problems with Kyrandia doing a load of path lookups are reduced by this hack.
	if (strstr(path, ".voc") || strstr(path, ".voc"))
	{
		dirEntry.name[0] = FILE_FREE;
		dirEntry.attrib = 0x00;
		return dirEntry;
	}
#endif

	if (path[pathPos] == '/')
	{
		dirCluster = filesysRootDirClus;	// Start at root directory
	}
	else
	{
		dirCluster = curWorkDirCluster;	// Start at current working dir
	}

	// Eat any slash /
	while ((path[pathPos] == '/') && (path[pathPos] != '\0'))
	{
		pathPos++;
	}

	// Search until can't continue
	found = false;
	notFound = false;
	while (!notFound && !found)
	{
		flagLFN = false;
		// Copy name from path
		namePos = 0;
		if ((path[pathPos] == '.') && ((path[pathPos + 1] == '\0') || (path[pathPos + 1] == '/'))) {
			// Dot entry
			name[namePos++] = '.';
			pathPos++;
		} else if ((path[pathPos] == '.') && (path[pathPos + 1] == '.') && ((path[pathPos + 2] == '\0') || (path[pathPos + 2] == '/'))){
			// Double dot entry
			name[namePos++] = '.';
			pathPos++;
			name[namePos++] = '.';
			pathPos++;
		} else {
			// Copy name from path
			if (path[pathPos] == '.') {
				flagLFN = true;
			}
			dotSeen = false;
			while ((namePos < MAX_FILENAME_LENGTH - 1) && (path[pathPos] != '\0') && (path[pathPos] != '/'))
			{
				name[namePos] = ucase(path[pathPos]);
				if ((name[namePos] <= ' ') || ((name[namePos] >= ':') && (name[namePos] <= '?'))) // Invalid character
				{
					flagLFN = true;
				}
				if (name[namePos] == '.') {
					if (!dotSeen) {
						dotSeen = true;
					} else {
						flagLFN = true;
					}
				}
				namePos++;
				pathPos++;
			}
			// Check if a long filename was specified
			if (namePos > 12)
			{
				flagLFN = true;
			}
		}

		// Add end of string char
		name[namePos] = '\0';

		// Move through path to correct place
		while ((path[pathPos] != '/') && (path[pathPos] != '\0'))
			pathPos++;
		// Eat any slash /
		while ((path[pathPos] == '/') && (path[pathPos] != '\0'))
		{
			pathPos++;
		}

		// Search current Dir for correct entry
		dirEntry = FAT_GetDirEntry (dirCluster, 1, SEEK_SET);
		while ( !found && !notFound)
		{
			// Match filename
			found = true;
			for (namePos = 0; (namePos < MAX_FILENAME_LENGTH) && found && (name[namePos] != '\0') && (lfnName[namePos] != '\0'); namePos++)
			{
				if (name[namePos] != ucase(lfnName[namePos]))
				{
					found = false;
				}
			}
			if ((name[namePos] == '\0') != (lfnName[namePos] == '\0'))
			{
				found = false;
			}

			// Check against alias as well.
			if (!found)
			{
				FAT_GetFilename(dirEntry, alias);
				found = true;
				for (namePos = 0; (namePos < 13) && found && (name[namePos] != '\0') && (alias[namePos] != '\0'); namePos++)
				{
					if (name[namePos] != ucase(alias[namePos]))
					{
						found = false;
					}
				}
				if ((name[namePos] == '\0') != (alias[namePos] == '\0'))
				{
					found = false;
				}
			}

			if (dirEntry.name[0] == FILE_FREE)
				// Couldn't find specified file
			{
				found = false;
				notFound = true;
			}
			if (!found && !notFound)
			{
				dirEntry = FAT_GetDirEntry (dirCluster, 1, SEEK_CUR);
			}
		}

		if (found && ((dirEntry.attrib & ATTRIB_DIR) == ATTRIB_DIR) && (path[pathPos] != '\0'))
			// It has found a directory from within the path that needs to be followed
		{
			found = false;
			dirCluster = dirEntry.startCluster | (dirEntry.startClusterHigh << 16);
		}
	}

	if (notFound)
	{
		dirEntry.name[0] = FILE_FREE;
		dirEntry.attrib = 0x00;
	}

	return (dirEntry);
}


#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_AddDirEntry
Creates a new dir entry for a file
Path separator is a forward slash /
const char* path: IN null terminated string of path to file.
DIR_ENT newDirEntry IN: The directory entry to use.
int file IN: The file being added (optional, use -1 if not used)
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool FAT_AddDirEntry (const char* path, DIR_ENT newDirEntry)
{
	char filename[MAX_FILENAME_LENGTH];
	int filePos, pathPos, aliasPos;
	char tempChar;
	bool flagLFN, dotSeen;
	char fileAlias[13] = {0};
	int tailNum;

	unsigned char chkSum = 0;

	u32 oldWorkDirCluster;

	DIR_ENT* dirEntries = (DIR_ENT*)globalBuffer;
	u32 dirCluster;
	int secOffset;
	int entryOffset;
	int maxSectors;
	u32 firstSector;

	DIR_ENT_LFN lfnEntry;
	int lfnPos = 0;

	int dirEntryLength = 0;
	int dirEntryRemain = 0;
	u32 tempDirCluster;
	int tempSecOffset;
	int tempEntryOffset;
	bool dirEndFlag = false;

	int i;

	// Store current working directory
	oldWorkDirCluster = curWorkDirCluster;

	// Find filename within path and change to correct directory
	if (path[0] == '/')
	{
		curWorkDirCluster = filesysRootDirClus;
	}

	pathPos = 0;
	filePos = 0;
	flagLFN = false;

	while (path[pathPos + filePos] != '\0')
	{
		if (path[pathPos + filePos] == '/')
		{
			filename[filePos] = '\0';
			if (FAT_chdir(filename) == false)
			{
				curWorkDirCluster = oldWorkDirCluster;
				return false; // Couldn't change directory
			}
			pathPos += filePos + 1;
			filePos = 0;
		}
		filename[filePos] = path[pathPos + filePos];
		filePos++;
	}

	// Skip over last slashes
	while (path[pathPos] == '/')
		pathPos++;

	// Check if the filename has a leading "."
	// If so, it is an LFN
	if (path[pathPos] == '.') {
		flagLFN = true;
	}

	// Copy name from path
	filePos = 0;
	dotSeen = false;

	while ((filePos < MAX_FILENAME_LENGTH - 1) && (path[pathPos] != '\0'))
	{
		filename[filePos] = path[pathPos];
		if ((filename[filePos] <= ' ') || ((filename[filePos] >= ':') && (filename[filePos] <= '?'))) // Invalid character
		{
			flagLFN = true;
		}
		if (filename[filePos] == '.') {
			if (!dotSeen) {
				dotSeen = true;
			} else {
				flagLFN = true;
			}
		}
		filePos++;
		pathPos++;
		if ((filePos > 8) && !dotSeen) {
			flagLFN = true;
		}
	}

	if (filePos == 0)	// No filename
	{
		return false;
	}

	// Check if a long filename was specified
	if (filePos > 12)
	{
		flagLFN = true;
	}

	// Check if extension is > 3 characters long
	if (!flagLFN && (strrchr (filename, '.') != NULL) && (strlen(strrchr(filename, '.')) > 4)) {
		flagLFN = true;
	}

	lfnPos = (filePos - 1) / 13;

	// Add end of string char
	filename[filePos++] = '\0';
	// Clear remaining chars
	while (filePos < MAX_FILENAME_LENGTH)
		filename[filePos++] = 0x01;	// Set for LFN compatibility


	if (flagLFN)
	{
		// Generate short filename - always a 2 digit number for tail
		// Get first 5 chars of alias from LFN
		aliasPos = 0;
		filePos = 0;
		if (filename[filePos] == '.') {
			filePos++;
		}
		for ( ; (aliasPos < 5) && (filename[filePos] != '\0') && (filename[filePos] != '.') ; filePos++)
		{
			tempChar = ucase(filename[filePos]);
			if (((tempChar > ' ' && tempChar < ':') || tempChar > '?') && tempChar != '.')
				fileAlias[aliasPos++] = tempChar;
		}
		// Pad Alias with underscores
		while (aliasPos < 5)
			fileAlias[aliasPos++] = '_';

		fileAlias[5] = '~';
		fileAlias[8] = '.';
		fileAlias[9] = ' ';
		fileAlias[10] = ' ';
		fileAlias[11] = ' ';
		if (strchr (filename, '.') != NULL) {
			while(filename[filePos] != '\0')
			{
				filePos++;
				if (filename[filePos] == '.')
				{
					pathPos = filePos;
				}
			}
			filePos = pathPos + 1;	//pathPos is used as a temporary variable
			// Copy first 3 characters of extension
			for (aliasPos = 9; (aliasPos < 12) && (filename[filePos] != '\0'); filePos++)
			{
				tempChar = ucase(filename[filePos]);
				if ((tempChar > ' ' && tempChar < ':') || tempChar > '?')
					fileAlias[aliasPos++] = tempChar;
			}
		} else {
			aliasPos = 9;
		}

		// Pad Alias extension with spaces
		while (aliasPos < 12)
			fileAlias[aliasPos++] = ' ';

		fileAlias[12] = '\0';


		// Get a valid tail number
		tailNum = 0;
		do {
			tailNum++;
			fileAlias[6] = 0x30 + ((tailNum / 10) % 10);	// 10's digit
			fileAlias[7] = 0x30 + (tailNum % 10);	// 1's digit
		} while ((FAT_DirEntFromPath(fileAlias).name[0] != FILE_FREE) && (tailNum < 100));

		if (tailNum < 100)	// Found an alias not being used
		{
			// Calculate file checksum
			chkSum = 0;
			for (aliasPos=0; aliasPos < 12; aliasPos++)
			{
				// Skip '.'
				if (fileAlias[aliasPos] == '.')
					aliasPos++;
				// NOTE: The operation is an unsigned char rotate right
				chkSum = ((chkSum & 1) ? 0x80 : 0) + (chkSum >> 1) + fileAlias[aliasPos];
			}
		}
		else	// Couldn't find a valid alias
		{
			return false;
		}

		dirEntryLength = lfnPos + 2;
	}
	else	// Its not a long file name
	{
		// Just copy alias straight from filename
		for (aliasPos = 0; aliasPos < 13; aliasPos++)
		{
			tempChar = ucase(filename[aliasPos]);
			if ((tempChar > ' ' && tempChar < ':') || tempChar > '?')
				fileAlias[aliasPos] = tempChar;
		}
		fileAlias[12] = '\0';

		lfnPos = -1;

		dirEntryLength = 1;
	}

	// Change dirEntry name to match alias
	for (aliasPos = 0; ((fileAlias[aliasPos] != '.') && (fileAlias[aliasPos] != '\0') && (aliasPos < 8)); aliasPos++)
	{
		newDirEntry.name[aliasPos] = fileAlias[aliasPos];
	}
	while (aliasPos < 8)
	{
		newDirEntry.name[aliasPos++] = ' ';
	}
	aliasPos = 0;
	while ((fileAlias[aliasPos] != '.') && (fileAlias[aliasPos] != '\0'))
		aliasPos++;
	filePos = 0;
	while (( filePos < 3 ) && (fileAlias[aliasPos] != '\0'))
	{
		tempChar = fileAlias[aliasPos++];
		if ((tempChar > ' ' && tempChar < ':' && tempChar!='.') || tempChar > '?')
			newDirEntry.ext[filePos++] = tempChar;
	}
	while (filePos < 3)
	{
		newDirEntry.ext[filePos++] = ' ';
	}

	// Scan Dir for free entry
	dirCluster = curWorkDirCluster;
	secOffset = 0;
	entryOffset = 0;
	maxSectors = (dirCluster == FAT16_ROOT_DIR_CLUSTER ? (filesysData - filesysRootDir) : filesysSecPerClus);
	firstSector = (dirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(dirCluster));
	disc_ReadSector (firstSector + secOffset, dirEntries);

	dirEntryRemain = dirEntryLength;
	tempDirCluster = dirCluster;
	tempSecOffset = secOffset;
	tempEntryOffset = entryOffset;

	// Search for a large enough space to fit in new directory entry
	while ((dirEntries[entryOffset].name[0] != FILE_LAST) && (dirEntryRemain > 0))
	{

		entryOffset++;

		if (entryOffset == BYTE_PER_READ / sizeof (DIR_ENT))
		{
			entryOffset = 0;
			secOffset++;
			if ((secOffset == filesysSecPerClus) && (dirCluster != FAT16_ROOT_DIR_CLUSTER))
			{
				secOffset = 0;
				if (FAT_NextCluster(dirCluster) == CLUSTER_EOF)
				{
					dirCluster = FAT_LinkFreeCluster(dirCluster);
					dirEntries[0].name[0] = FILE_LAST;
				}
				else
				{
					dirCluster = FAT_NextCluster(dirCluster);
				}
				firstSector = FAT_ClustToSect(dirCluster);
			}
			else if ((dirCluster == FAT16_ROOT_DIR_CLUSTER) && (secOffset == (filesysData - filesysRootDir)))
			{
				return false;	// Got to end of root dir - can't fit in more files
			}
			disc_ReadSector (firstSector + secOffset, dirEntries);
		}

		if ((dirEntries[entryOffset].name[0] == FILE_FREE) || (dirEntries[entryOffset].name[0] == FILE_LAST) )
		{
			dirEntryRemain--;
		} else {
			dirEntryRemain = dirEntryLength;
			tempDirCluster = dirCluster;
			tempSecOffset = secOffset;
			tempEntryOffset = entryOffset;
		}
	}

	// Modifying the last directory is a special case - have to erase following entries
	if (dirEntries[entryOffset].name[0] == FILE_LAST)
	{
		dirEndFlag = true;
	}

	// Recall last used entry
	dirCluster = tempDirCluster;
	secOffset = tempSecOffset;
	entryOffset = tempEntryOffset;
	dirEntryRemain = dirEntryLength;

	// Re-read in first sector that will be written to
	if (dirEndFlag && (entryOffset == 0))	{
		memset (dirEntries, FILE_LAST, BYTE_PER_READ);
	} else {
		disc_ReadSector (firstSector + secOffset, dirEntries);
	}

	// Add new directory entry
	while (dirEntryRemain > 0)
	{
		// Move to next entry, first pass advances from last used entry
		entryOffset++;
		if (entryOffset == BYTE_PER_READ / sizeof (DIR_ENT))
		{
			// Write out the current sector if we need to
			entryOffset = 0;
			if (dirEntryRemain < dirEntryLength) // Don't write out sector on first pass
			{
				disc_WriteSector (firstSector + secOffset, dirEntries);
			}
			secOffset++;
			if ((secOffset == filesysSecPerClus) && (dirCluster != FAT16_ROOT_DIR_CLUSTER))
			{
				secOffset = 0;
				if (FAT_NextCluster(dirCluster) == CLUSTER_EOF)
				{
					dirCluster = FAT_LinkFreeCluster(dirCluster);
					dirEntries[0].name[0] = FILE_LAST;
				}
				else
				{
					dirCluster = FAT_NextCluster(dirCluster);
				}
				firstSector = FAT_ClustToSect(dirCluster);
			}
			else if ((dirCluster == FAT16_ROOT_DIR_CLUSTER) && (secOffset == (filesysData - filesysRootDir)))
			{
				return false;	// Got to end of root dir - can't fit in more files
			}
			if (dirEndFlag)
			{
				memset (dirEntries, FILE_LAST, BYTE_PER_READ);
			} else {
				disc_ReadSector (firstSector + secOffset, dirEntries);
			}
		}

		// Generate LFN entries
		if (lfnPos >= 0)
		{
			lfnEntry.ordinal = (lfnPos + 1) | (dirEntryRemain == dirEntryLength ? LFN_END : 0);
			for (i = 0; i < 13; i++) {
				if (filename [lfnPos * 13 + i] == 0x01) {
					((u8*)&lfnEntry)[(int)lfn_offset_table[i]] = 0xff;
					((u8*)&lfnEntry)[(int)(lfn_offset_table[i]) + 1] = 0xff;
				} else {
					((u8*)&lfnEntry)[(int)lfn_offset_table[i]] = filename [lfnPos * 13 + i];
					((u8*)&lfnEntry)[(int)(lfn_offset_table[i]) + 1] = 0x00;
				}
			}

			lfnEntry.checkSum = chkSum;
			lfnEntry.flag = ATTRIB_LFN;
			lfnEntry.reserved1 = 0;
			lfnEntry.reserved2 = 0;

			*((DIR_ENT_LFN*)&dirEntries[entryOffset]) = lfnEntry;
			lfnPos --;
			lfnEntry.ordinal = 0;
		}	// end writing long filename entries
		else
		{
			dirEntries[entryOffset] = newDirEntry;
			if (dirEndFlag && (entryOffset < (BYTE_PER_READ / sizeof (DIR_ENT))) )
				dirEntries[entryOffset+1].name[0] = FILE_LAST;
		}

		dirEntryRemain--;
	}

	// Write directory back to disk
	disc_WriteSector (firstSector + secOffset, dirEntries);

	// Change back to Working DIR
	curWorkDirCluster = oldWorkDirCluster;

	return true;
}
#endif

/*-----------------------------------------------------------------
FAT_FindNextFile
Gets the name of the next directory entry
	(can be a file or subdirectory)
char* filename: OUT filename, must be at least 13 chars long
FILE_TYPE return: OUT returns FT_NONE if failed,
	FT_FILE if it found a file and FT_DIR if it found a directory
-----------------------------------------------------------------*/
FILE_TYPE FAT_FindNextFile(char* filename)
{
	// Get the next directory entry
	DIR_ENT file;
	file = FAT_GetDirEntry (curWorkDirCluster, 1, SEEK_CUR);

	if (file.name[0] == FILE_FREE)
	{
		return FT_NONE;	// Did not find a file
	}

	// Get the filename
	if (filename != NULL)
		FAT_GetFilename (file, filename);

	if ((file.attrib & ATTRIB_DIR) != 0)
	{
		return FT_DIR;	// Found a directory
	}
	else
	{
		return FT_FILE;	// Found a file
	}
}

/*-----------------------------------------------------------------
FAT_FindFirstFile
Gets the name of the first directory entry and resets the count
	(can be a file or subdirectory)
char* filename: OUT filename, must be at least 13 chars long
FILE_TYPE return: OUT returns FT_NONE if failed,
	FT_FILE if it found a file and FT_DIR if it found a directory
-----------------------------------------------------------------*/
FILE_TYPE FAT_FindFirstFile(char* filename)
{
	// Get the first directory entry
	DIR_ENT file;
	file = FAT_GetDirEntry (curWorkDirCluster, 1, SEEK_SET);

	if (file.name[0] == FILE_FREE)
	{
		return FT_NONE;	// Did not find a file
	}

	// Get the filename
	if (filename != NULL)
		FAT_GetFilename (file, filename);

	if ((file.attrib & ATTRIB_DIR) != 0)
	{
		return FT_DIR;	// Found a directory
	}
	else
	{
		return FT_FILE;	// Found a file
	}
}

/*-----------------------------------------------------------------
FAT_FindFirstFileLFN
Gets the long file name of the first directory entry and resets
	the count (can be a file or subdirectory)
char* lfn: OUT long file name, must be at least 256 chars long
FILE_TYPE return: OUT returns FT_NONE if failed,
	FT_FILE if it found a file and FT_DIR if it found a directory
-----------------------------------------------------------------*/
FILE_TYPE FAT_FindFirstFileLFN(char* lfn)
{
	FILE_TYPE type;
	type = FAT_FindFirstFile(NULL);
	FAT_GetLongFilename (lfn);
	return type;
}

/*-----------------------------------------------------------------
FAT_FindNextFileLFN
Gets the long file name of the next directory entry
	(can be a file or subdirectory)
char* lfn: OUT long file name, must be at least 256 chars long
FILE_TYPE return: OUT returns FT_NONE if failed,
	FT_FILE if it found a file and FT_DIR if it found a directory
-----------------------------------------------------------------*/
FILE_TYPE FAT_FindNextFileLFN(char* lfn)
{
	FILE_TYPE type;
	type = FAT_FindNextFile(NULL);
	FAT_GetLongFilename (lfn);
	return type;
}


/*-----------------------------------------------------------------
FAT_FileExists
Returns the type of file
char* filename: IN filename of the file to look for
FILE_TYPE return: OUT returns FT_NONE if there is now file with
	that name, FT_FILE if it is a file and FT_DIR if it is a directory
-----------------------------------------------------------------*/
FILE_TYPE FAT_FileExists(const char* filename)
{
    DIR_ENT dirEntry;
    // Get the dirEntry for the path specified
    dirEntry = FAT_DirEntFromPath (filename);

    if (dirEntry.name[0] == FILE_FREE)
    {
        return FT_NONE;
    }
    else if (dirEntry.attrib & ATTRIB_DIR)
    {
        return FT_DIR;
    }
    else
    {
         return FT_FILE;
    }
}

/*-----------------------------------------------------------------
FAT_GetFileSystemType
FS_TYPE return: OUT returns the current file system type
-----------------------------------------------------------------*/
FS_TYPE FAT_GetFileSystemType (void)
{
	return filesysType;
}

/*-----------------------------------------------------------------
FAT_GetFileSystemTotalSize
u32 return: OUT returns the total disk space (used + free)
-----------------------------------------------------------------*/
u32 FAT_GetFileSystemTotalSize (void)
{
	return filesysTotalSize;
}



/*-----------------------------------------------------------------
FAT_chdir
Changes the current working directory
const char* path: IN null terminated string of directory separated by
	forward slashes, / is root
bool return: OUT returns true if successful
-----------------------------------------------------------------*/
bool FAT_chdir (const char* path)
{
	DIR_ENT dir;
	if (path[0] == '/' && path[1] == '\0')
	{
		curWorkDirCluster = filesysRootDirClus;
		return true;
	}
	if (path[0] == '\0')	// Return true if changing relative to nothing
	{
		return true;
	}

	dir = FAT_DirEntFromPath (path);

	if (((dir.attrib & ATTRIB_DIR) == ATTRIB_DIR) && (dir.name[0] != FILE_FREE))
	{
		// Change directory
		curWorkDirCluster = dir.startCluster | (dir.startClusterHigh << 16);

		// Move to correct cluster for root directory
		if (curWorkDirCluster == FAT16_ROOT_DIR_CLUSTER)
		{
			curWorkDirCluster = filesysRootDirClus;
		}

		// Reset file position in directory
		wrkDirCluster = curWorkDirCluster;
		wrkDirSector = 0;
		wrkDirOffset = -1;
		return true;
	}
	else
	{
		// Couldn't change directory - wrong path specified
		return false;
	}
}

/*-----------------------------------------------------------------
FAT_fopen(filename, mode)
Opens a file
const char* path: IN null terminated string of filename and path
	separated by forward slashes, / is root
const char* mode: IN mode to open file in
	Supported modes: "r", "r+", "w", "w+", "a", "a+", don't use
	"b" or "t" in any mode, as all files are openned in binary mode
FAT_FILE* return: OUT handle to open file, returns NULL if the file
	couldn't be openned
-----------------------------------------------------------------*/
FAT_FILE* FAT_fopen(const char* path, const char* mode)
{
	int fileNum;
	FAT_FILE* file;
	DIR_ENT dirEntry;
#ifdef CAN_WRITE_TO_DISC
	u32 startCluster;
	int clusCount;
#endif

	char* pchTemp;
	// Check that a valid mode was specified
	pchTemp = strpbrk ( mode, "rRwWaA" );
	if (pchTemp == NULL)
	{
		return NULL;
	}
	if (strpbrk ( pchTemp+1, "rRwWaA" ) != NULL)
	{
		return NULL;
	}

	// Get the dirEntry for the path specified
	dirEntry = FAT_DirEntFromPath (path);

	// Check that it is not a directory
	if (dirEntry.attrib & ATTRIB_DIR)
	{
		return NULL;
	}

#ifdef CAN_WRITE_TO_DISC
	// Check that it is not a read only file being openned in a writing mode
	if ( (strpbrk(mode, "wWaA+") != NULL) && (dirEntry.attrib & ATTRIB_RO))
	{
		return NULL;
	}
#else
	if ( (strpbrk(mode, "wWaA+") != NULL))
	{
		return NULL;
	}
#endif

	// Find a free file buffer
	for (fileNum = 0; (fileNum < MAX_FILES_OPEN) && (openFiles[fileNum].inUse == true); fileNum++);

	if (fileNum == MAX_FILES_OPEN) // No free files
	{
		return NULL;
	}

	file = &openFiles[fileNum];
	// Remember where directory entry was
	file->dirEntSector = (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector;
	file->dirEntOffset = wrkDirOffset;

	if ( strpbrk(mode, "rR") != NULL )  //(ucase(mode[0]) == 'R')
	{
		if (dirEntry.name[0] == FILE_FREE)	// File must exist
		{
			return NULL;
		}

		file->read = true;
#ifdef CAN_WRITE_TO_DISC
		file->write = ( strchr(mode, '+') != NULL ); //(mode[1] == '+');
#else
		file->write = false;
#endif
		file->append = false;

		// Store information about position within the file, for use
		// by FAT_fread, FAT_fseek, etc.
		file->firstCluster = dirEntry.startCluster | (dirEntry.startClusterHigh << 16);

#ifdef CAN_WRITE_TO_DISC
		// Check if file is openned for random. If it is, and currently has no cluster, one must be
		// assigned to it.
		if (file->write && file->firstCluster == CLUSTER_FREE)
		{
			file->firstCluster = FAT_LinkFreeCluster (CLUSTER_FREE);
			if (file->firstCluster == CLUSTER_FREE)	// Couldn't get a free cluster
			{
				return NULL;
			}

			// Store cluster position into the directory entry
			dirEntry.startCluster = (file->firstCluster & 0xFFFF);
			dirEntry.startClusterHigh = ((file->firstCluster >> 16) & 0xFFFF);
			disc_ReadSector (file->dirEntSector, globalBuffer);
			((DIR_ENT*) globalBuffer)[file->dirEntOffset] = dirEntry;
			disc_WriteSector (file->dirEntSector, globalBuffer);
		}
#endif

		file->length = dirEntry.fileSize;
		file->curPos = 0;
		file->curClus = dirEntry.startCluster | (dirEntry.startClusterHigh << 16);
		file->curSect = 0;
		file->curByte = 0;

		// Not appending
		file->appByte = 0;
		file->appClus = 0;
		file->appSect = 0;

		disc_ReadSector( FAT_ClustToSect( file->curClus), file->readBuffer);
		file->inUse = true;	// We're using this file now

		return file;
	}	// mode "r"

#ifdef CAN_WRITE_TO_DISC
	if ( strpbrk(mode, "wW") != NULL ) // (ucase(mode[0]) == 'W')
	{
		if (dirEntry.name[0] == FILE_FREE)	// Create file if it doesn't exist
		{
			dirEntry.attrib = ATTRIB_ARCH;
			dirEntry.reserved = 0;

			// Time and date set to system time and date
			dirEntry.cTime_ms = 0;
			dirEntry.cTime = getRTCtoFileTime();
			dirEntry.cDate = getRTCtoFileDate();
			dirEntry.aDate = getRTCtoFileDate();
			dirEntry.mTime = getRTCtoFileTime();
			dirEntry.mDate = getRTCtoFileDate();
		}
		else	// Already a file entry
		{
			// Free any clusters used
			FAT_ClearLinks (dirEntry.startCluster | (dirEntry.startClusterHigh << 16));
		}

		// Get a cluster to use
		startCluster = FAT_LinkFreeCluster (CLUSTER_FREE);
		if (startCluster == CLUSTER_FREE)	// Couldn't get a free cluster
		{
			return NULL;
		}

		// Store cluster position into the directory entry
		dirEntry.startCluster = (startCluster & 0xFFFF);
		dirEntry.startClusterHigh = ((startCluster >> 16) & 0xFFFF);

		// The file has no data in it - its over written so should be empty
		dirEntry.fileSize = 0;

		if (dirEntry.name[0] == FILE_FREE)	// No file
		{
			// Have to create a new entry
			if(!FAT_AddDirEntry (path, dirEntry))
			{
				return NULL;
			}
			// Get the newly created dirEntry
			dirEntry = FAT_DirEntFromPath (path);

			// Remember where directory entry was
			file->dirEntSector = (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector;
			file->dirEntOffset = wrkDirOffset;
		}
		else	// Already a file
		{
			// Just modify the old entry
			disc_ReadSector (file->dirEntSector, globalBuffer);
			((DIR_ENT*) globalBuffer)[file->dirEntOffset] = dirEntry;
			disc_WriteSector (file->dirEntSector, globalBuffer);
		}


		// Now that file is created, open it
		file->read = ( strchr(mode, '+') != NULL ); //(mode[1] == '+');
		file->write = true;
		file->append = false;

		// Store information about position within the file, for use
		// by FAT_fread, FAT_fseek, etc.
		file->firstCluster = startCluster;
		file->length = 0;	// Should always have 0 bytes if openning in "w" mode
		file->curPos = 0;
		file->curClus = startCluster;
		file->curSect = 0;
		file->curByte = 0;

		// Not appending
		file->appByte = 0;
		file->appClus = 0;
		file->appSect = 0;

		// Empty file, so empty read buffer
		memset (file->readBuffer, 0, BYTE_PER_READ);
		file->inUse = true;	// We're using this file now

		return file;
	}

	if ( strpbrk(mode, "aA") != NULL ) // (ucase(mode[0]) == 'A')
	{
		if (dirEntry.name[0] == FILE_FREE)	// Create file if it doesn't exist
		{
			dirEntry.attrib = ATTRIB_ARCH;
			dirEntry.reserved = 0;

			// Time and date set to system time and date
			dirEntry.cTime_ms = 0;
			dirEntry.cTime = getRTCtoFileTime();
			dirEntry.cDate = getRTCtoFileDate();
			dirEntry.aDate = getRTCtoFileDate();
			dirEntry.mTime = getRTCtoFileTime();
			dirEntry.mDate = getRTCtoFileDate();

			// The file has no data in it
			dirEntry.fileSize = 0;

			// Get a cluster to use
			startCluster = FAT_LinkFreeCluster (CLUSTER_FREE);
			if (startCluster == CLUSTER_FREE)	// Couldn't get a free cluster
			{
				return NULL;
			}
			dirEntry.startCluster = (startCluster & 0xFFFF);
			dirEntry.startClusterHigh = ((startCluster >> 16) & 0xFFFF);

			if(!FAT_AddDirEntry (path, dirEntry))
				return NULL;

			// Get the newly created dirEntry
			dirEntry = FAT_DirEntFromPath (path);

			// Store append cluster
			file->appClus = startCluster;

			// Remember where directory entry was
			file->dirEntSector = (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector;
			file->dirEntOffset = wrkDirOffset;
		}
		else	// File already exists - reuse the old directory entry
		{
			startCluster = dirEntry.startCluster | (dirEntry.startClusterHigh << 16);
			// If it currently has no cluster, one must be assigned to it.
			if (startCluster == CLUSTER_FREE)
			{
				file->firstCluster = FAT_LinkFreeCluster (CLUSTER_FREE);
				if (file->firstCluster == CLUSTER_FREE)	// Couldn't get a free cluster
				{
					return NULL;
				}

				// Store cluster position into the directory entry
				dirEntry.startCluster = (file->firstCluster & 0xFFFF);
				dirEntry.startClusterHigh = ((file->firstCluster >> 16) & 0xFFFF);
				disc_ReadSector (file->dirEntSector, globalBuffer);
				((DIR_ENT*) globalBuffer)[file->dirEntOffset] = dirEntry;
				disc_WriteSector (file->dirEntSector, globalBuffer);

				// Store append cluster
				file->appClus = startCluster;

			} else {

				// Follow cluster list until last one is found
				clusCount = dirEntry.fileSize / filesysBytePerClus;
				file->appClus = startCluster;
				while ((clusCount--) && (FAT_NextCluster (file->appClus) != CLUSTER_FREE) && (FAT_NextCluster (file->appClus) != CLUSTER_EOF))
				{
					file->appClus = FAT_NextCluster (file->appClus);
				}
				if (clusCount >= 0) // Check if ran out of clusters
				{
					// Set flag to allocate new cluster when needed
					file->appSect = filesysSecPerClus;
					file->appByte = 0;
				}
			}
		}

		// Now that file is created, open it
		file->read = ( strchr(mode, '+') != NULL );
		file->write = false;
		file->append = true;

		// Calculate the sector and byte of the current position,
		// and store them
		file->appSect = (dirEntry.fileSize % filesysBytePerClus) / BYTE_PER_READ;
		file->appByte = dirEntry.fileSize % BYTE_PER_READ;

		// Store information about position within the file, for use
		// by FAT_fread, FAT_fseek, etc.
		file->firstCluster = startCluster;
		file->length = dirEntry.fileSize;
		file->curPos = dirEntry.fileSize;
		file->curClus = file->appClus;
		file->curSect = file->appSect;
		file->curByte = file->appByte;

		// Read into buffer
		disc_ReadSector( FAT_ClustToSect(file->curClus) + file->curSect, file->readBuffer);
		file->inUse = true;	// We're using this file now
		return file;
	}
#endif

	// Can only reach here if a bad mode was specified
	return NULL;
}

/*-----------------------------------------------------------------
FAT_fclose(file)
Closes a file
FAT_FILE* file: IN handle of the file to close
bool return OUT: true if successful, false if not
-----------------------------------------------------------------*/
bool FAT_fclose (FAT_FILE* file)
{
	// Clear memory used by file information
	if ((file != NULL) && (file->inUse == true))
	{
#ifdef CAN_WRITE_TO_DISC
		if (file->write || file->append)
		{
			// Write new length, time and date back to directory entry
			disc_ReadSector (file->dirEntSector, globalBuffer);

			((DIR_ENT*)globalBuffer)[file->dirEntOffset].fileSize = file->length;
			((DIR_ENT*)globalBuffer)[file->dirEntOffset].mTime = getRTCtoFileTime();
			((DIR_ENT*)globalBuffer)[file->dirEntOffset].mDate = getRTCtoFileDate();
			((DIR_ENT*)globalBuffer)[file->dirEntOffset].aDate = getRTCtoFileDate();

			disc_WriteSector (file->dirEntSector, globalBuffer);

			// Flush any sectors in disc cache
			disc_CacheFlush();
		}
#endif
		file->inUse = false;
		return true;
	}
	else
	{
		return false;
	}
}

/*-----------------------------------------------------------------
FAT_ftell(file)
Returns the current position in a file
FAT_FILE* file: IN handle of an open file
u32 OUT: Current position
-----------------------------------------------------------------*/
u32 FAT_ftell (FAT_FILE* file)
{
	// Return the position as specified in the FAT_FILE structure
	if ((file != NULL) && (file->inUse == true))
	{
		return file->curPos;
	}
	else
	{
		// Return -1 if no file was given
		return -1;
	}
}

/*-----------------------------------------------------------------
FAT_fseek(file, offset, origin)
Seeks to specified byte position in file
FAT_FILE* file: IN handle of an open file
s32 offset IN: position to seek to, relative to origin
int origin IN: origin to seek from
int OUT: Returns 0 if successful, -1 if not
-----------------------------------------------------------------*/
int FAT_fseek(FAT_FILE* file, s32 offset, int origin)
{
	u32 cluster, nextCluster;
	int clusCount;
	u32 position;
	u32 curPos;


	if ((file == NULL) || (file->inUse == false))	// invalid file
	{
		return -1;
	}

	// Can't seek in append only mode
	if (!file->read && !file->write)
	{
		return -1;
	}

	curPos = file->curPos;

	switch (origin)
	{
	case SEEK_SET:
		if (offset >= 0)
		{
			position = offset;
		} else {
			// Tried to seek before start of file
			position = 0;
		}
		break;
	case SEEK_CUR:
		if (offset >= 0)
		{
			position = curPos + offset;
		}
		else if ( (u32)(offset * -1) >= curPos )
		{
			// Tried to seek before start of file
			position = 0;
		}
		else
		{
			// Using u32 to maintain 32 bits of accuracy
			position = curPos - (u32)(offset * -1);
		}
		break;
	case SEEK_END:
		if (offset >= 0)
		{
			// Seeking to end of file
			position = file->length;	// Fixed thanks to MoonLight
		}
		else if ( (u32)(offset * -1) >= file->length )
		{
			// Tried to seek before start of file
			position = 0;
		}
		else
		{
			// Using u32 to maintain 32 bits of accuracy
			position = file->length - (u32)(offset * -1);
		}
		break;
	default:
		return -1;
	}

	if (position > file->length)
	{
		// Tried to go past end of file
		position = file->length;
	}

	// Save position
	file->curPos = position;


	// Calculate where the correct cluster is
	if (position > curPos)
	{
		clusCount = (position - curPos + (file->curSect * filesysBytePerSec) + file->curByte) / filesysBytePerClus;	// Fixed thanks to AgentQ
		cluster = file->curClus;
	} else {
		clusCount = position / filesysBytePerClus;
		cluster = file->firstCluster;
	}

	// Calculate the sector and byte of the current position,
	// and store them
	file->curSect = (position % filesysBytePerClus) / BYTE_PER_READ;
	file->curByte = position % BYTE_PER_READ;

	// Follow cluster list until desired one is found
	if (clusCount > 0)	// Only look at next cluster if need to
	{
		nextCluster = FAT_NextCluster (cluster);
	} else {
		nextCluster = cluster;
	}
	while ((clusCount--) && (nextCluster != CLUSTER_FREE) && (nextCluster != CLUSTER_EOF))
	{
		cluster = nextCluster;
		nextCluster = FAT_NextCluster (cluster);
	}
	// Check if ran out of clusters, and the file is being written to
	if ((clusCount >= 0) && (file->write || file->append))
	{
		// Set flag to allocate a new cluster
		file->curSect = filesysSecPerClus;
		file->curByte = 0;
	}
	file->curClus = cluster;

	// Reload sector buffer for new position in file, if it is a different sector
	if ((curPos ^ position) >= BYTE_PER_READ)
	{
		disc_ReadSector( file->curSect + FAT_ClustToSect(file->curClus), file->readBuffer);
	}

	return 0;

}

/*-----------------------------------------------------------------
FAT_fread(buffer, size, count, file)
Reads in size * count bytes into buffer from file, starting
	from current position. It then sets the current position to the
	byte after the last byte read. If it reaches the end of file
	before filling the buffer then it stops reading.
void* buffer OUT: Pointer to buffer to fill. Should be at least as
	big as the number of bytes required
u32 size IN: size of each item to read
u32 count IN: number of items to read
FAT_FILE* file IN: Handle of an open file
u32 OUT: returns the actual number of bytes read
-----------------------------------------------------------------*/
u32 FAT_fread (void* buffer, u32 size, u32 count, FAT_FILE* file)
{
	int curByte;
	int curSect;
	u32 curClus;
	u32 tempNextCluster;

	int tempVar;

	char* data = (char*)buffer;

	u32 length = size * count;
	u32 remain;

	bool flagNoError = true;

	// Can't read non-existant files
	if ((file == NULL) || (file->inUse == false) || size == 0 || count == 0 || buffer == NULL)
		return 0;

	// Can only read files openned for reading
	if (!file->read)
		return 0;

	// Don't read past end of file
	if (length + file->curPos > file->length)
		length = file->length - file->curPos;

	remain = length;

	curByte = file->curByte;
	curSect = file->curSect;
	curClus = file->curClus;

	// Align to sector
	tempVar = BYTE_PER_READ - curByte;
	if (tempVar > remain)
		tempVar = remain;

	if ((tempVar < BYTE_PER_READ) && flagNoError)
	{
		memcpy(data, &(file->readBuffer[curByte]), tempVar);
		remain -= tempVar;
		data += tempVar;

		curByte += tempVar;
		if (curByte >= BYTE_PER_READ)
		{
			curByte = 0;
			curSect++;
		}
	}

	// align to cluster
	// tempVar is number of sectors to read
	if (remain > (filesysSecPerClus - curSect) * BYTE_PER_READ)
	{
		tempVar = filesysSecPerClus - curSect;
	} else {
		tempVar = remain / BYTE_PER_READ;
	}

	if ((tempVar > 0) && flagNoError)
	{
		disc_ReadSectors ( curSect + FAT_ClustToSect(curClus), tempVar, data);
		data += tempVar * BYTE_PER_READ;
		remain -= tempVar * BYTE_PER_READ;

		curSect += tempVar;
	}

	// Move onto next cluster
	// It should get to here without reading anything if a cluster is due to be allocated
	if (curSect >= filesysSecPerClus)
	{
		tempNextCluster = FAT_NextCluster(curClus);
		if ((remain == 0) && (tempNextCluster == CLUSTER_EOF))
		{
			curSect = filesysSecPerClus;
		} else {
			curSect = 0;
			curClus = tempNextCluster;
			if (curClus == CLUSTER_FREE)
			{
				flagNoError = false;
			}
		}
	}

	// Read in whole clusters
	while ((remain >= filesysBytePerClus) && flagNoError)
	{
		disc_ReadSectors (FAT_ClustToSect(curClus), filesysSecPerClus, data);
		data += filesysBytePerClus;
		remain -= filesysBytePerClus;

		// Advance to next cluster
		tempNextCluster = FAT_NextCluster(curClus);
		if ((remain == 0) && (tempNextCluster == CLUSTER_EOF))
		{
			curSect = filesysSecPerClus;
		} else {
			curSect = 0;
			curClus = tempNextCluster;
			if (curClus == CLUSTER_FREE)
			{
				flagNoError = false;
			}
		}
	}

	// Read remaining sectors
	tempVar = remain / BYTE_PER_READ; // Number of sectors left
	if ((tempVar > 0) && flagNoError)
	{
		disc_ReadSectors (FAT_ClustToSect(curClus), tempVar, data);
		data += tempVar * BYTE_PER_READ;
		remain -= tempVar * BYTE_PER_READ;
		curSect += tempVar;
	}

	// Last remaining sector
	// Check if sector wanted is different to the one started with
	if ( ((file->curByte + length) >= BYTE_PER_READ) && flagNoError)
	{
		disc_ReadSector( curSect + FAT_ClustToSect( curClus), file->readBuffer);
		if (remain > 0)
		{
			memcpy(data, file->readBuffer, remain);
			curByte += remain;
			remain = 0;
		}
	}

	// Length read is the wanted length minus the stuff not read
	length = length - remain;

	// Update file information
	file->curByte = curByte;
	file->curSect = curSect;
	file->curClus = curClus;
	file->curPos = file->curPos + length;
	return length;
}

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_fwrite(buffer, size, count, file)
Writes size * count bytes into file from buffer, starting
	from current position. It then sets the current position to the
	byte after the last byte written. If the file was openned in
	append mode it always writes to the end of the file.
const void* buffer IN: Pointer to buffer containing data. Should be
	at least as big as the number of bytes to be written.
u32 size IN: size of each item to write
u32 count IN: number of items to write
FAT_FILE* file IN: Handle of an open file
u32 OUT: returns the actual number of bytes written
-----------------------------------------------------------------*/
u32 FAT_fwrite (const void* buffer, u32 size, u32 count, FAT_FILE* file)
{
	int curByte;
	int curSect;
	u32 curClus;

	u32 tempNextCluster;
	int tempVar;
	u32 length = size * count;
	u32 remain = length;
	char* data = (char*)buffer;

	char* writeBuffer;

	bool flagNoError = true;
	bool flagAppending = false;

	if ((file == NULL) || (file->inUse == false) || length == 0 || buffer == NULL)
		return 0;

	if (file->write)
	{
		// Write at current read pointer
		curByte = file->curByte;
		curSect = file->curSect;
		curClus = file->curClus;

		// Use read buffer as write buffer
		writeBuffer = file->readBuffer;

		// If it is writing past the current end of file, set appending flag
		if (length + file->curPos > file->length)
		{
			flagAppending = true;
		}
	}
	else if (file->append)
	{
		// Write at end of file
		curByte = file->appByte;
		curSect = file->appSect;
		curClus = file->appClus;
		flagAppending = true;

		// Use global buffer as write buffer, don't touch read buffer
		writeBuffer = (char*)globalBuffer;
		disc_ReadSector(curSect + FAT_ClustToSect(curClus), writeBuffer);
	}
	else
	{
		return 0;
	}

	// Move onto next cluster if needed
	if (curSect >= filesysSecPerClus)
	{
		curSect = 0;
		tempNextCluster = FAT_NextCluster(curClus);
		if ((tempNextCluster == CLUSTER_EOF) || (tempNextCluster == CLUSTER_FREE))
		{
			// Ran out of clusters so get a new one
			curClus = FAT_LinkFreeCluster(curClus);
			if (curClus == CLUSTER_FREE) // Couldn't get a cluster, so abort
			{
				flagNoError = false;
			}
			memset(writeBuffer, 0, BYTE_PER_READ);
		} else {
			curClus = tempNextCluster;
			disc_ReadSector( FAT_ClustToSect( curClus), writeBuffer);
		}
	}

	// Align to sector
	tempVar = BYTE_PER_READ - curByte;
	if (tempVar > remain)
		tempVar = remain;

	if ((tempVar < BYTE_PER_READ) && flagNoError)
	{
		memcpy(&(writeBuffer[curByte]), data, tempVar);
		remain -= tempVar;
		data += tempVar;
		curByte += tempVar;

		// Write buffer back to disk
		disc_WriteSector (curSect + FAT_ClustToSect(curClus), writeBuffer);

		// Move onto next sector
		if (curByte >= BYTE_PER_READ)
		{
			curByte = 0;
			curSect++;
		}
	}

	// Align to cluster
	// tempVar is number of sectors to write
	if (remain > (filesysSecPerClus - curSect) * BYTE_PER_READ)
	{
		tempVar = filesysSecPerClus - curSect;
	} else {
		tempVar = remain / BYTE_PER_READ;
	}

	if ((tempVar > 0) && flagNoError)
	{
		disc_WriteSectors ( curSect + FAT_ClustToSect(curClus), tempVar, data);
		data += tempVar * BYTE_PER_READ;
		remain -= tempVar * BYTE_PER_READ;
		curSect += tempVar;
	}

	if (((curSect >= filesysSecPerClus) && flagNoError) && (remain > 0))
	{
		curSect = 0;
		tempNextCluster = FAT_NextCluster(curClus);
		if ((tempNextCluster == CLUSTER_EOF) || (tempNextCluster == CLUSTER_FREE))
		{
			// Ran out of clusters so get a new one
			curClus = FAT_LinkFreeCluster(curClus);
			if (curClus == CLUSTER_FREE) // Couldn't get a cluster, so abort
			{
				flagNoError = false;
			}
		} else {
			curClus = tempNextCluster;
		}
	}

	// Write whole clusters
	while ((remain >= filesysBytePerClus) && flagNoError)
	{
		disc_WriteSectors (FAT_ClustToSect(curClus), filesysSecPerClus, data);
		data += filesysBytePerClus;
		remain -= filesysBytePerClus;
		if (remain > 0)
		{
			tempNextCluster = FAT_NextCluster(curClus);
			if ((tempNextCluster == CLUSTER_EOF) || (tempNextCluster == CLUSTER_FREE))
			{
				// Ran out of clusters so get a new one
				curClus = FAT_LinkFreeCluster(curClus);
				if (curClus == CLUSTER_FREE) // Couldn't get a cluster, so abort
				{
					flagNoError = false;
					break;
				}
			} else {
				curClus = tempNextCluster;
			}
		} else {
			// Allocate a new cluster when next writing the file
			curSect = filesysSecPerClus;
		}
	}

	// Write remaining sectors
	tempVar = remain / BYTE_PER_READ; // Number of sectors left
	if ((tempVar > 0) && flagNoError)
	{
		disc_WriteSectors (FAT_ClustToSect(curClus), tempVar, data);
		data += tempVar * BYTE_PER_READ;
		remain -= tempVar * BYTE_PER_READ;
		curSect += tempVar;
	}

	// Last remaining sector
	// Check if sector wanted is different to the one started with
	if ( (( (file->append ? file->appByte : file->curByte) + length) >= BYTE_PER_READ) && flagNoError)
	{
		if (flagAppending)
		{
			// Zero sector before using it
			memset (writeBuffer, 0, BYTE_PER_READ);
		} else {
			// Modify existing sector
			disc_ReadSector( curSect + FAT_ClustToSect( curClus), writeBuffer);
		}
		if (remain > 0) {
			memcpy(writeBuffer, data, remain);
			curByte += remain;
			remain = 0;
			disc_WriteSector( curSect + FAT_ClustToSect( curClus), writeBuffer);
		}
	}

	// Amount read is the originally requested amount minus stuff remaining
	length = length - remain;

	// Update file information
	if (file->write)	// Writing also shifts the read pointer
	{
		file->curByte = curByte;
		file->curSect = curSect;
		file->curClus = curClus;
		file->curPos = file->curPos + length;
		if (file->length < file->curPos)
		{
			file->length = file->curPos;
		}
	}
	else if (file->append)	// Appending doesn't affect the read pointer
	{
		file->appByte = curByte;
		file->appSect = curSect;
		file->appClus = curClus;
		file->length = file->length + length;
	}

	return length;
}
#endif


/*-----------------------------------------------------------------
FAT_feof(file)
Returns true if the end of file has been reached
FAT_FILE* file IN: Handle of an open file
bool return OUT: true if EOF, false if not
-----------------------------------------------------------------*/
bool FAT_feof(FAT_FILE* file)
{
	if ((file == NULL) || (file->inUse == false))
		return true;	// Return eof on invalid files

	return (file->length == file->curPos);
}


#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_remove (path)
Deletes the file or empty directory sepecified in path
const char* path IN: Path of item to delete
int return OUT: zero if successful, non-zero if not
-----------------------------------------------------------------*/
int FAT_remove (const char* path)
{
	DIR_ENT dirEntry;
	u32 oldWorkDirCluster;
	char checkFilename[13];
	FILE_TYPE checkFiletype;

	dirEntry = FAT_DirEntFromPath (path);

	if (dirEntry.name[0] == FILE_FREE)
	{
		return -1;
	}

	// Only delete directories if the directory is entry
	if (dirEntry.attrib & ATTRIB_DIR)
	{
		// Change to the directory temporarily
		oldWorkDirCluster = curWorkDirCluster;
		FAT_chdir(path);

		// Search for files or directories, excluding the . and .. entries
		checkFiletype = FAT_FindFirstFile (checkFilename);
		while ((checkFilename[0] == '.')  && (checkFiletype != FT_NONE))
		{
			checkFiletype = FAT_FindNextFile (checkFilename);
		}

		// Change back to working directory
		curWorkDirCluster = oldWorkDirCluster;

		// Check that the directory is empty
		if (checkFiletype != FT_NONE)
		{
			// Directory isn't empty
			return -1;
		}
	}

	// Refresh directory information
	dirEntry = FAT_DirEntFromPath (path);

	// Free any clusters used
	FAT_ClearLinks (dirEntry.startCluster | (dirEntry.startClusterHigh << 16));

	// Remove Directory entry
	disc_ReadSector ( (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector , globalBuffer);
	((DIR_ENT*)globalBuffer)[wrkDirOffset].name[0] = FILE_FREE;
	disc_WriteSector ( (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector , globalBuffer);

	// Flush any sectors in disc cache
	disc_CacheFlush();

	return 0;
}
#endif

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_mkdir (path)
Makes a new directory, so long as no other directory or file has
	the same name.
const char* path IN: Path and filename of directory to make
int return OUT: zero if successful, non-zero if not
-----------------------------------------------------------------*/
int FAT_mkdir (const char* path)
{
	u32 newDirCluster;
	u32 parentDirCluster;
	DIR_ENT dirEntry;
	DIR_ENT* entries = (DIR_ENT*)globalBuffer;
	int i;

	int pathPos, filePos;
	char pathname[MAX_FILENAME_LENGTH];
	u32 oldDirCluster;

	if (FAT_FileExists(path) != FT_NONE)
	{
		return -1;	// File or directory exists with that name
	}

	// Find filename within path and change to that directory
	oldDirCluster = curWorkDirCluster;
	if (path[0] == '/')
	{
		curWorkDirCluster = filesysRootDirClus;
	}

	pathPos = 0;
	filePos = 0;

	while (path[pathPos + filePos] != '\0')
	{
		if (path[pathPos + filePos] == '/')
		{
			pathname[filePos] = '\0';
			if (FAT_chdir(pathname) == false)
			{
				curWorkDirCluster = oldDirCluster;
				return -1; // Couldn't change directory
			}
			pathPos += filePos + 1;
			filePos = 0;
		}
		pathname[filePos] = path[pathPos + filePos];
		filePos++;
	}

	// Now grab the parent directory's cluster
	parentDirCluster = curWorkDirCluster;
	curWorkDirCluster = oldDirCluster;

	// Get a new cluster for the file
	newDirCluster = FAT_LinkFreeCluster(CLUSTER_FREE);

	if (newDirCluster == CLUSTER_FREE)
	{
		return -1;	// Couldn't get a new cluster for the directory
	}
	// Fill in directory entry's information
	dirEntry.attrib = ATTRIB_DIR;
	dirEntry.reserved = 0;
	// Time and date set to system time and date
	dirEntry.cTime_ms = 0;
	dirEntry.cTime = getRTCtoFileTime();
	dirEntry.cDate = getRTCtoFileDate();
	dirEntry.aDate = getRTCtoFileDate();
	dirEntry.mTime = getRTCtoFileTime();
	dirEntry.mDate = getRTCtoFileDate();
	// Store cluster position into the directory entry
	dirEntry.startCluster = (newDirCluster & 0xFFFF);
	dirEntry.startClusterHigh = ((newDirCluster >> 16) & 0xFFFF);
	// The file has no data in it - its over written so should be empty
	dirEntry.fileSize = 0;

	if (FAT_AddDirEntry (path, dirEntry) == false)
	{
		return -1;	// Couldn't add the directory entry
	}

	// Create the new directory itself
	memset(entries, FILE_LAST, BYTE_PER_READ);

	// Create . directory entry
	dirEntry.name[0] = '.';
	// Fill name and extension with spaces
	for (i = 1; i < 11; i++)
	{
		dirEntry.name[i] = ' ';
	}

	memcpy(entries, &dirEntry, sizeof(dirEntry));

	// Create .. directory entry
	dirEntry.name[1] = '.';
	dirEntry.startCluster = (parentDirCluster & 0xFFFF);
	dirEntry.startClusterHigh = ((parentDirCluster >> 16) & 0xFFFF);

	memcpy(&entries[1], &dirEntry, sizeof(dirEntry));

	// Write entry to disc
	disc_WriteSector(FAT_ClustToSect(newDirCluster), entries);

	// Flush any sectors in disc cache
	disc_CacheFlush();
	return 0;
}
#endif

/*-----------------------------------------------------------------
FAT_fgetc (handle)
Gets the next character in the file
FAT_FILE* file IN: Handle of open file
bool return OUT: character if successful, EOF if not
-----------------------------------------------------------------*/
char FAT_fgetc (FAT_FILE* file)
{
	char c;
	return (FAT_fread(&c, 1, 1, file) == 1) ? c : EOF;
}

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_fputc (character, handle)
Writes the given character into the file
char c IN: Character to be written
FAT_FILE* file IN: Handle of open file
bool return OUT: character if successful, EOF if not
-----------------------------------------------------------------*/
char FAT_fputc (char c, FAT_FILE* file)
{
	return (FAT_fwrite(&c, 1, 1, file) == 1) ? c : EOF;
}
#endif

/*-----------------------------------------------------------------
FAT_fgets (char *tgtBuffer, int num, FAT_FILE* file)
Gets a up to num bytes from file, stopping at the first
 newline.

CAUTION: does not do strictly streaming. I.e. it's
 reading more then needed bytes and seeking back.
 shouldn't matter for random access

char *tgtBuffer OUT: buffer to write to
int num IN: size of target buffer
FAT_FILE* file IN: Handle of open file
bool return OUT: character if successful, EOF if not

  Written by MightyMax
  Modified by Chishm - 2005-11-17
	* Added check for unix style text files
	* Removed seek when no newline is found, since it isn't necessary
-------------------------------------------------------------------*/
char *FAT_fgets(char *tgtBuffer, int num, FAT_FILE* file)
{
	u32 curPos;
	u32 readLength;
	char *returnChar;

	// invalid filehandle
	if (file == NULL)
	{
		return NULL ;
	}

	// end of file
	if (FAT_feof(file)==true)
	{
		return NULL ;
	}

	// save current position
	curPos = FAT_ftell(file);

	// read the full buffer (max string chars is num-1 and one end of string \0
	readLength = FAT_fread(tgtBuffer,1,num-1,file) ;

	// mark least possible end of string
	tgtBuffer[readLength] = '\0' ;

	if (readLength==0) {
		// return error
		return NULL ;
	}

	// get position of first return '\r'
	returnChar = strchr(tgtBuffer,'\r');

	// if no return is found, search for a newline
	if (returnChar == NULL)
	{
		returnChar = strchr(tgtBuffer,'\n');
	}

	// Mark the return, if existant, as end of line/string
	if (returnChar!=NULL) {
		*returnChar++ = 0 ;
		if (*returnChar=='\n') { // catch newline too when jumping over the end
			// return to location after \r\n (strlen+2)
			FAT_fseek(file,curPos+strlen(tgtBuffer)+2,SEEK_SET) ;
			return tgtBuffer ;
		} else {
			// return to location after \r (strlen+1)
			FAT_fseek(file,curPos+strlen(tgtBuffer)+1,SEEK_SET) ;
			return tgtBuffer ;
		}
	}

	return tgtBuffer ;
}

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_fputs (const char *string, FAT_FILE* file)
Writes string to file, excluding end of string character
const char *string IN: string to write
FAT_FILE* file IN: Handle of open file
bool return OUT: number of characters written if successful,
	EOF if not

  Written by MightyMax
  Modified by Chishm - 2005-11-17
	* Uses FAT_FILE instead of int
	* writtenBytes is now u32 instead of int
-------------------------------------------------------------------*/
int FAT_fputs (const char *string, FAT_FILE* file)
{
   u32 writtenBytes;
	// save string except end of string '\0'
   writtenBytes = FAT_fwrite((void *)string, 1, strlen(string), file);

   // check if we had an error
   if (writtenBytes != strlen(string))
   {
      // return EOF error
      return EOF;
   }

   // return the charcount written
   return writtenBytes ;
}
#endif
