/*
	io_mpcf.c based on

	compact_flash.c
	By chishm (Michael Chisholm)

	Hardware Routines for reading a compact flash card
	using the GBA Movie Player

	CF routines modified with help from Darkfader

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/


#include "io_mpcf.h"

#ifdef SUPPORT_MPCF

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

//---------------------------------------------------------------
// CF Addresses & Commands

#define GAME_PAK		0x08000000			// Game pack start address

// GBAMP CF Addresses
#define MP_REG_STS		*(vu16*)(GAME_PAK + 0x018C0000)	// Status of the CF Card / Device control
#define MP_REG_CMD		*(vu16*)(GAME_PAK + 0x010E0000)	// Commands sent to control chip and status return
#define MP_REG_ERR		*(vu16*)(GAME_PAK + 0x01020000)	// Errors / Features

#define MP_REG_SEC		*(vu16*)(GAME_PAK + 0x01040000)	// Number of sector to transfer
#define MP_REG_LBA1		*(vu16*)(GAME_PAK + 0x01060000)	// 1st byte of sector address
#define MP_REG_LBA2		*(vu16*)(GAME_PAK + 0x01080000)	// 2nd byte of sector address
#define MP_REG_LBA3		*(vu16*)(GAME_PAK + 0x010A0000)	// 3rd byte of sector address
#define MP_REG_LBA4		*(vu16*)(GAME_PAK + 0x010C0000)	// last nibble of sector address | 0xE0

#define MP_DATA			(vu16*)(GAME_PAK + 0x01000000)		// Pointer to buffer of CF data transered from card

// CF Card status
#define CF_STS_INSERTED		0x50
#define CF_STS_REMOVED		0x00
#define CF_STS_READY		0x58

#define CF_STS_DRQ			0x08
#define CF_STS_BUSY			0x80

// CF Card commands
#define CF_CMD_LBA			0xE0
#define CF_CMD_READ			0x20
#define CF_CMD_WRITE		0x30

#define CARD_TIMEOUT	10000000		// Updated due to suggestion from SaTa, otherwise card will timeout sometimes on a write


/*-----------------------------------------------------------------
MPCF_IsInserted
Is a compact flash card inserted?
bool return OUT:  true if a CF card is inserted
-----------------------------------------------------------------*/
bool MPCF_IsInserted (void)
{
	// Change register, then check if value did change
	MP_REG_STS = CF_STS_INSERTED;
	return ((MP_REG_STS & 0xff) == CF_STS_INSERTED);
}


/*-----------------------------------------------------------------
MPCF_ClearStatus
Tries to make the CF card go back to idle mode
bool return OUT:  true if a CF card is idle
-----------------------------------------------------------------*/
bool MPCF_ClearStatus (void)
{
	int i;

	// Wait until CF card is finished previous commands
	i=0;
	while ((MP_REG_CMD & CF_STS_BUSY) && (i < CARD_TIMEOUT))
	{
		i++;
	}

	// Wait until card is ready for commands
	i = 0;
	while ((!(MP_REG_STS & CF_STS_INSERTED)) && (i < CARD_TIMEOUT))
	{
		i++;
	}
	if (i >= CARD_TIMEOUT)
		return false;

	return true;
}


/*-----------------------------------------------------------------
MPCF_ReadSectors
Read 512 byte sector numbered "sector" into "buffer"
u32 sector IN: address of first 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer OUT: pointer to 512 byte buffer to store data in
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool MPCF_ReadSectors (u32 sector, u8 numSecs, void* buffer)
{
	int i;
	int j = (numSecs > 0 ? numSecs : 256);
	u16 *buff = (u16*)buffer;
#ifdef _CF_ALLOW_UNALIGNED
	u8 *buff_u8 = (u8*)buffer;
	int temp;
#endif

#if (defined _CF_USE_DMA) && (defined NDS) && (defined ARM9)
	DC_FlushRange( buffer, j * BYTE_PER_READ);
#endif

	// Wait until CF card is finished previous commands
	i=0;
	while ((MP_REG_CMD & CF_STS_BUSY) && (i < CARD_TIMEOUT))
	{
		i++;
	}

	// Wait until card is ready for commands
	i = 0;
	while ((!(MP_REG_STS & CF_STS_INSERTED)) && (i < CARD_TIMEOUT))
	{
		i++;
	}
	if (i >= CARD_TIMEOUT)
		return false;

	// Set number of sectors to read
	MP_REG_SEC = numSecs;

	// Set read sector
	MP_REG_LBA1 = sector & 0xFF;						// 1st byte of sector number
	MP_REG_LBA2 = (sector >> 8) & 0xFF;					// 2nd byte of sector number
	MP_REG_LBA3 = (sector >> 16) & 0xFF;				// 3rd byte of sector number
	MP_REG_LBA4 = ((sector >> 24) & 0x0F )| CF_CMD_LBA;	// last nibble of sector number

	// Set command to read
	MP_REG_CMD = CF_CMD_READ;


	while (j--)
	{
		// Wait until card is ready for reading
		i = 0;
		while (((MP_REG_STS & 0xff)!= CF_STS_READY) && (i < CARD_TIMEOUT))
		{
			i++;
		}
		if (i >= CARD_TIMEOUT)
			return false;

		// Read data
#ifdef _CF_USE_DMA
 #ifdef NDS
		DMA3_SRC = (u32)MP_DATA;
		DMA3_DEST = (u32)buff;
		DMA3_CR = 256 | DMA_COPY_HALFWORDS | DMA_SRC_FIX;
 #else
		DMA3COPY ( MP_DATA, buff, 256 | DMA16 | DMA_ENABLE | DMA_SRC_FIXED);
 #endif
		buff += BYTE_PER_READ / 2;
#elif defined _CF_ALLOW_UNALIGNED
		i=256;
		if ((u32)buff_u8 & 0x01) {
			while(i--)
			{
				temp = *MP_DATA;
				*buff_u8++ = temp & 0xFF;
				*buff_u8++ = temp >> 8;
			}
		} else {
		while(i--)
			*buff++ = *MP_DATA;
		}
#else
		i=256;
		while(i--)
			*buff++ = *MP_DATA;
#endif
	}
#if (defined _CF_USE_DMA) && (defined NDS)
	// Wait for end of transfer before returning
	while(DMA3_CR & DMA_BUSY);
#endif
	return true;
}



/*-----------------------------------------------------------------
MPCF_WriteSectors
Write 512 byte sector numbered "sector" from "buffer"
u32 sector IN: address of 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer IN: pointer to 512 byte buffer to read data from
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool MPCF_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{
	int i;
	int j = (numSecs > 0 ? numSecs : 256);
	u16 *buff = (u16*)buffer;
#ifdef _CF_ALLOW_UNALIGNED
	u8 *buff_u8 = (u8*)buffer;
	int temp;
#endif

#if defined _CF_USE_DMA && defined NDS && defined ARM9
	DC_FlushRange( buffer, j * BYTE_PER_READ);
#endif

	// Wait until CF card is finished previous commands
	i=0;
	while ((MP_REG_CMD & CF_STS_BUSY) && (i < CARD_TIMEOUT))
	{
		i++;
	}

	// Wait until card is ready for commands
	i = 0;
	while ((!(MP_REG_STS & CF_STS_INSERTED)) && (i < CARD_TIMEOUT))
	{
		i++;
	}
	if (i >= CARD_TIMEOUT)
		return false;

	// Set number of sectors to write
	MP_REG_SEC = numSecs;

	// Set write sector
	MP_REG_LBA1 = sector & 0xFF;						// 1st byte of sector number
	MP_REG_LBA2 = (sector >> 8) & 0xFF;					// 2nd byte of sector number
	MP_REG_LBA3 = (sector >> 16) & 0xFF;				// 3rd byte of sector number
	MP_REG_LBA4 = ((sector >> 24) & 0x0F )| CF_CMD_LBA;	// last nibble of sector number

	// Set command to write
	MP_REG_CMD = CF_CMD_WRITE;

	while (j--)
	{
		// Wait until card is ready for writing
		i = 0;
		while (((MP_REG_STS & 0xff) != CF_STS_READY) && (i < CARD_TIMEOUT))
		{
			i++;
		}
		if (i >= CARD_TIMEOUT)
			return false;

		// Write data
#ifdef _CF_USE_DMA
 #ifdef NDS
		DMA3_SRC = (u32)buff;
		DMA3_DEST = (u32)MP_DATA;
		DMA3_CR = 256 | DMA_COPY_HALFWORDS | DMA_DST_FIX;
 #else
		DMA3COPY( buff, MP_DATA, 256 | DMA16 | DMA_ENABLE | DMA_DST_FIXED);
 #endif
		buff += BYTE_PER_READ / 2;
#elif defined _CF_ALLOW_UNALIGNED
		i=256;
		if ((u32)buff_u8 & 0x01) {
			while(i--)
			{
				temp = *buff_u8++;
				temp |= *buff_u8++ << 8;
				*MP_DATA = temp;
			}
		} else {
		while(i--)
			*MP_DATA = *buff++;
		}
#else
		i=256;
		while(i--)
			*MP_DATA = *buff++;
#endif
	}
#if defined _CF_USE_DMA && defined NDS
	// Wait for end of transfer before returning
	while(DMA3_CR & DMA_BUSY);
#endif

	return true;
}

/*-----------------------------------------------------------------
MPCF_Shutdown
unload the GBAMP CF interface
-----------------------------------------------------------------*/
bool MPCF_Shutdown(void)
{
	return MPCF_ClearStatus() ;
}

/*-----------------------------------------------------------------
MPCF_StartUp
initializes the CF interface, returns true if successful,
otherwise returns false
-----------------------------------------------------------------*/
bool MPCF_StartUp(void)
{
	u8 temp = MP_REG_LBA1;
	MP_REG_LBA1 = (~temp & 0xFF);
	temp = (~temp & 0xFF);
	// NDM: Added GBA ROM header check so that this doesn't detect a Max Media Dock!
	return (MP_REG_LBA1 == temp) && ( *((u8 *) (0x080000B2)) == 0x96);
}

/*-----------------------------------------------------------------
the actual interface structure
-----------------------------------------------------------------*/
IO_INTERFACE io_mpcf = {
	DEVICE_TYPE_MPCF,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&MPCF_StartUp,
	(FN_MEDIUM_ISINSERTED)&MPCF_IsInserted,
	(FN_MEDIUM_READSECTORS)&MPCF_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&MPCF_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&MPCF_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&MPCF_Shutdown
} ;

/*-----------------------------------------------------------------
MPCF_GetInterface
returns the interface structure to host
-----------------------------------------------------------------*/
LPIO_INTERFACE MPCF_GetInterface(void) {
	return &io_mpcf ;
} ;

#endif // SUPPORT_MPCF
