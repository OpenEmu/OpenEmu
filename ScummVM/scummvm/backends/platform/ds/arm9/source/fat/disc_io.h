#ifndef DISC_IO_H
#define DISC_IO_H

//----------------------------------------------------------------------
// Customisable features

// Use DMA to read the card, remove this line to use normal reads/writes
// #define _CF_USE_DMA

// Allow buffers not aligned to 16 bits when reading files.
// Note that this will slow down access speed, so only use if you have to.
// It is also incompatible with DMA
#define _CF_ALLOW_UNALIGNED
#define _IO_ALLOW_UNALIGNED

// Device support options, added by www.neoflash.com

//#define SUPPORT_MPCF		// comment out this line to remove GBA Movie Player support
//#define SUPPORT_M3CF		// comment out this line to remove M3 Perfect CF support
//#define SUPPORT_M3SD		// comment out this line to remove M3 Perfect SD support
//#define SUPPORT_SCCF		// comment out this line to remove Supercard CF support
//#define SUPPORT_SCSD		// comment out this line to remove Supercard SD support
//#define SUPPORT_NJSD
//#define SUPPORT_MMCF

//#define SUPPORT_EFA2		// comment out this line to remove EFA2 linker support
//#define SUPPORT_FCSR		// comment out this line to remove GBA Flash Cart support
//#define SUPPORT_NMMC		// comment out this line to remove Neoflash MK2 MMC Card support


// Disk caching options, added by www.neoflash.com
// Each additional sector cache uses 512 bytes of memory
// Disk caching is disabled on GBA to conserve memory

#define DISC_CACHE				// uncomment this line to enable disc caching
#ifdef DS_BUILD_F
#define DISC_CACHE_COUNT	128	// maximum number of sectors to cache (512 bytes per sector)
#else
#define DISC_CACHE_COUNT	32	// maximum number of sectors to cache (512 bytes per sector)
#endif
//#define DISK_CACHE_DMA		// use DMA for cache copies. If this is enabled, the data buffers must be word aligned


// This allows the code to build on an earlier version of libnds, before the register was renamed
#ifndef REG_EXMEMCNT
#define REG_EXMEMCNT REG_EXEMEMCNT
#endif

#ifndef REG_EXEMEMCNT
#define REG_EXEMEMCNT REG_EXMEMCNT
#endif

//----------------------------------------------------------------------

#if defined _CF_USE_DMA && defined _CF_ALLOW_UNALIGNED
 #error You can not use both DMA and unaligned memory
#endif

// When compiling for NDS, make sure NDS is defined
#ifndef NDS
 #if defined ARM9 || defined ARM7
  #define NDS
 #endif
#endif

#ifdef NDS
 #include <nds/ndstypes.h>
#else
 #include "gba_types.h"
#endif

// Disable NDS specific hardware and features if running on a GBA
#ifndef NDS
 #undef SUPPORT_NMMC
 #undef DISC_CACHE
#endif

/*

	Interface for host program

*/

#define BYTE_PER_READ 512

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	DEVICE_NONE = 0,
	DEVICE_M3SD,
	DEVICE_MMCF,
	DEVICE_M3CF,
	DEVICE_MPCF,
	DEVICE_SCCF,
	DEVICE_NJSD,
	DEVICE_SCSD,
	DEVICE_NMMC,
	DEVICE_DLDI
} FATDevice;

/*-----------------------------------------------------------------
disc_Init
Detects the inserted hardware and initialises it if necessary
bool return OUT:  true if a suitable device was found
-----------------------------------------------------------------*/
extern bool disc_Init(void) ;

/*-----------------------------------------------------------------
disc_IsInserted
Is a usable disc inserted?
bool return OUT:  true if a disc is inserted
-----------------------------------------------------------------*/
extern bool disc_IsInserted(void) ;


extern void disc_setEnable(int en);
extern FATDevice disc_getDeviceId();
void disc_getDldiId(char* id);

/*-----------------------------------------------------------------
disc_ReadSectors
Read 512 byte sector numbered "sector" into "buffer"
u32 sector IN: address of first 512 byte sector on disc to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer OUT: pointer to 512 byte buffer to store data in
bool return OUT: true if successful
-----------------------------------------------------------------*/
extern bool disc_ReadSectors(u32 sector, u8 numSecs, void* buffer) ;
#define disc_ReadSector(sector,buffer)	disc_ReadSectors(sector,1,buffer)

/*-----------------------------------------------------------------
disc_WriteSectors
Write 512 byte sector numbered "sector" from "buffer"
u32 sector IN: address of 512 byte sector on disc to write
u8 numSecs IN: number of 512 byte sectors to write	,
 1 to 256 sectors can be read, 0 = 256
void* buffer IN: pointer to 512 byte buffer to read data from
bool return OUT: true if successful
-----------------------------------------------------------------*/
extern bool disc_WriteSectors(u32 sector, u8 numSecs, void* buffer) ;
#define disc_WriteSector(sector,buffer)	disc_WriteSectors(sector,1,buffer)

/*-----------------------------------------------------------------
disc_ClearStatus
Tries to make the disc go back to idle mode
bool return OUT:  true if the disc is idle
-----------------------------------------------------------------*/
extern bool disc_ClearStatus(void) ;

/*-----------------------------------------------------------------
disc_Shutdown
unload the disc interface
bool return OUT: true if successful
-----------------------------------------------------------------*/
extern bool disc_Shutdown(void) ;

/*-----------------------------------------------------------------
disc_HostType
Returns a unique u32 number identifying the host type
u32 return OUT: 0 if no host initialised, else the identifier of
	the host
-----------------------------------------------------------------*/
extern u32 disc_HostType(void);

/*-----------------------------------------------------------------
disc_CacheFlush
Flushes any cache writes to disc
bool return OUT: true if successful, false if an error occurs
Added by www.neoflash.com
-----------------------------------------------------------------*/
#ifdef DISC_CACHE
extern bool disc_CacheFlush(void);
#else
static inline bool disc_CacheFlush(void)
{
	return true;
}
#endif // DISC_CACHE


/*

	Interface for IO libs

*/

#define FEATURE_MEDIUM_CANREAD		0x00000001
#define FEATURE_MEDIUM_CANWRITE		0x00000002
#define FEATURE_SLOT_GBA			0x00000010
#define FEATURE_SLOT_NDS			0x00000020


typedef bool (* FN_MEDIUM_STARTUP)(void) ;
typedef bool (* FN_MEDIUM_ISINSERTED)(void) ;
typedef bool (* FN_MEDIUM_READSECTORS)(u32 sector, u8 numSecs, void* buffer) ;
typedef bool (* FN_MEDIUM_WRITESECTORS)(u32 sector, u8 numSecs, void* buffer) ;
typedef bool (* FN_MEDIUM_CLEARSTATUS)(void) ;
typedef bool (* FN_MEDIUM_SHUTDOWN)(void) ;


typedef struct {
	unsigned long			ul_ioType ;
	unsigned long			ul_Features ;
	FN_MEDIUM_STARTUP		fn_StartUp ;
	FN_MEDIUM_ISINSERTED	fn_IsInserted ;
	FN_MEDIUM_READSECTORS	fn_ReadSectors ;
	FN_MEDIUM_WRITESECTORS	fn_WriteSectors ;
	FN_MEDIUM_CLEARSTATUS	fn_ClearStatus ;
	FN_MEDIUM_SHUTDOWN		fn_Shutdown ;
} IO_INTERFACE, *LPIO_INTERFACE ;

#ifdef __cplusplus
}
#endif

#endif	// define DISC_IO_H
