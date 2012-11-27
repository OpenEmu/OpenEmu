/*
	io_m3cf.c based on

	compact_flash.c
	By chishm (Michael Chisholm)

	Hardware Routines for reading a compact flash card
	using the M3 Perfect CF Adapter

	CF routines modified with help from Darkfader

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/


#include "io_m3cf.h"

#ifdef SUPPORT_M3CF

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
#define M3_REG_STS		*(vu16*)(GAME_PAK + 0x000C0000)	// Status of the CF Card / Device control
#define M3_REG_CMD		*(vu16*)(GAME_PAK + 0x008E0000)	// Commands sent to control chip and status return
#define M3_REG_ERR		*(vu16*)(GAME_PAK + 0x00820000)	// Errors / Features

#define M3_REG_SEC		*(vu16*)(GAME_PAK + 0x00840000)	// Number of sector to transfer
#define M3_REG_LBA1		*(vu16*)(GAME_PAK + 0x00860000)	// 1st byte of sector address
#define M3_REG_LBA2		*(vu16*)(GAME_PAK + 0x00880000)	// 2nd byte of sector address
#define M3_REG_LBA3		*(vu16*)(GAME_PAK + 0x008A0000)	// 3rd byte of sector address
#define M3_REG_LBA4		*(vu16*)(GAME_PAK + 0x008C0000)	// last nibble of sector address | 0xE0

#define M3_DATA			(vu16*)(GAME_PAK + 0x00800000)		// Pointer to buffer of CF data transered from card

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
M3CF_IsInserted
Is a compact flash card inserted?
bool return OUT:  true if a CF card is inserted
-----------------------------------------------------------------*/
bool M3CF_IsInserted (void)
{
	// Change register, then check if value did change
	M3_REG_STS = CF_STS_INSERTED;
	return ((M3_REG_STS & 0xff) == CF_STS_INSERTED);
}


/*-----------------------------------------------------------------
M3CF_ClearStatus
Tries to make the CF card go back to idle mode
bool return OUT:  true if a CF card is idle
-----------------------------------------------------------------*/
bool M3CF_ClearStatus (void)
{
	int i;

	// Wait until CF card is finished previous commands
	i=0;
	while ((M3_REG_CMD & CF_STS_BUSY) && (i < CARD_TIMEOUT))
	{
		i++;
	}

	// Wait until card is ready for commands
	i = 0;
	while ((!(M3_REG_STS & CF_STS_INSERTED)) && (i < CARD_TIMEOUT))
	{
		i++;
	}
	if (i >= CARD_TIMEOUT)
		return false;

	return true;
}


/*-----------------------------------------------------------------
M3CF_ReadSectors
Read 512 byte sector numbered "sector" into "buffer"
u32 sector IN: address of first 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer OUT: pointer to 512 byte buffer to store data in
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool M3CF_ReadSectors (u32 sector, u8 numSecs, void* buffer)
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
	while ((M3_REG_CMD & CF_STS_BUSY) && (i < CARD_TIMEOUT))
	{
		i++;
	}

	// Wait until card is ready for commands
	i = 0;
	while ((!(M3_REG_STS & CF_STS_INSERTED)) && (i < CARD_TIMEOUT))
	{
		i++;
	}
	if (i >= CARD_TIMEOUT)
		return false;

	// Set number of sectors to read
	M3_REG_SEC = numSecs;

	// Set read sector
	M3_REG_LBA1 = sector & 0xFF;						// 1st byte of sector number
	M3_REG_LBA2 = (sector >> 8) & 0xFF;					// 2nd byte of sector number
	M3_REG_LBA3 = (sector >> 16) & 0xFF;				// 3rd byte of sector number
	M3_REG_LBA4 = ((sector >> 24) & 0x0F )| CF_CMD_LBA;	// last nibble of sector number

	// Set command to read
	M3_REG_CMD = CF_CMD_READ;


	while (j--)
	{
		// Wait until card is ready for reading
		i = 0;
		while (((M3_REG_STS & 0xff) != CF_STS_READY) && (i < CARD_TIMEOUT))
		{
			i++;
		}
		if (i >= CARD_TIMEOUT)
			return false;

		// Read data
#ifdef _CF_USE_DMA
 #ifdef NDS
		DMA3_SRC = (u32)M3_DATA;
		DMA3_DEST = (u32)buff;
		DMA3_CR = 256 | DMA_COPY_HALFWORDS | DMA_SRC_FIX;
 #else
		DMA3COPY ( M3_DATA, buff, 256 | DMA16 | DMA_ENABLE | DMA_SRC_FIXED);
 #endif
		buff += BYTE_PER_READ / 2;
#elif defined _CF_ALLOW_UNALIGNED
		i=256;
		if ((u32)buff_u8 & 0x01) {
			while(i--)
			{
				temp = *M3_DATA;
				*buff_u8++ = temp & 0xFF;
				*buff_u8++ = temp >> 8;
			}
		} else {
		while(i--)
			*buff++ = *M3_DATA;
		}
#else
		i=256;
		while(i--)
			*buff++ = *M3_DATA;
#endif
	}
#if defined _CF_USE_DMA && defined NDS
	// Wait for end of transfer before returning
	while(DMA3_CR & DMA_BUSY);
#endif

	return true;
}



/*-----------------------------------------------------------------
M3CF_WriteSectors
Write 512 byte sector numbered "sector" from "buffer"
u32 sector IN: address of 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer IN: pointer to 512 byte buffer to read data from
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool M3CF_WriteSectors (u32 sector, u8 numSecs, void* buffer)
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
	while ((M3_REG_CMD & CF_STS_BUSY) && (i < CARD_TIMEOUT))
	{
		i++;
	}

	// Wait until card is ready for commands
	i = 0;
	while ((!(M3_REG_STS & CF_STS_INSERTED)) && (i < CARD_TIMEOUT))
	{
		i++;
	}
	if (i >= CARD_TIMEOUT)
		return false;

	// Set number of sectors to write
	M3_REG_SEC = numSecs;

	// Set write sector
	M3_REG_LBA1 = sector & 0xFF;						// 1st byte of sector number
	M3_REG_LBA2 = (sector >> 8) & 0xFF;					// 2nd byte of sector number
	M3_REG_LBA3 = (sector >> 16) & 0xFF;				// 3rd byte of sector number
	M3_REG_LBA4 = ((sector >> 24) & 0x0F )| CF_CMD_LBA;	// last nibble of sector number

	// Set command to write
	M3_REG_CMD = CF_CMD_WRITE;

	while (j--)
	{
		// Wait until card is ready for writing
		i = 0;
		while (((M3_REG_STS & 0xff) != CF_STS_READY) && (i < CARD_TIMEOUT))
		{
			i++;
		}
		if (i >= CARD_TIMEOUT)
			return false;

		// Write data
#ifdef _CF_USE_DMA
 #ifdef NDS
		DMA3_SRC = (u32)buff;
		DMA3_DEST = (u32)M3_DATA;
		DMA3_CR = 256 | DMA_COPY_HALFWORDS | DMA_DST_FIX;
 #else
		DMA3COPY( buff, M3_DATA, 256 | DMA16 | DMA_ENABLE | DMA_DST_FIXED);
 #endif
		buff += BYTE_PER_READ / 2;
#elif defined _CF_ALLOW_UNALIGNED
		i=256;
		if ((u32)buff_u8 & 0x01) {
			while(i--)
			{
				temp = *buff_u8++;
				temp |= *buff_u8++ << 8;
				*M3_DATA = temp;
			}
		} else {
		while(i--)
			*M3_DATA = *buff++;
		}
#else
		i=256;
		while(i--)
			*M3_DATA = *buff++;
#endif
	}
#if defined _CF_USE_DMA && defined NDS
	// Wait for end of transfer before returning
	while(DMA3_CR & DMA_BUSY);
#endif

	return true;
}


/*-----------------------------------------------------------------
M3_Unlock
Returns true if M3 was unlocked, false if failed
Added by MightyMax
-----------------------------------------------------------------*/
bool M3_Unlock(void)
{
	// run unlock sequence
	volatile unsigned short tmp ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08E00002 ;
	tmp = *(volatile unsigned short *)0x0800000E ;
	tmp = *(volatile unsigned short *)0x08801FFC ;
	tmp = *(volatile unsigned short *)0x0800104A ;
	tmp = *(volatile unsigned short *)0x08800612 ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	tmp = *(volatile unsigned short *)0x08801B66 ;
	tmp = *(volatile unsigned short *)0x08800006 ;
	tmp = *(volatile unsigned short *)0x08000000 ;
	// test that we have register access
	tmp = M3_REG_LBA1;
	M3_REG_LBA1 = (~tmp & 0xFF);
	tmp = (~tmp & 0xFF);
	// did it change?
	return (M3_REG_LBA1 == tmp) ;
}

bool M3CF_Shutdown(void) {
	return M3CF_ClearStatus() ;
} ;

bool M3CF_StartUp(void) {
	return M3_Unlock() ;
} ;


IO_INTERFACE io_m3cf = {
	DEVICE_TYPE_M3CF,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&M3CF_StartUp,
	(FN_MEDIUM_ISINSERTED)&M3CF_IsInserted,
	(FN_MEDIUM_READSECTORS)&M3CF_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&M3CF_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&M3CF_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&M3CF_Shutdown
} ;


LPIO_INTERFACE M3CF_GetInterface(void) {
	return &io_m3cf ;
} ;

#endif // SUPPORT_M3CF
