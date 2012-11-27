/*
  io_mmcf.c based on

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


#include "io_mmcf.h"

#ifdef SUPPORT_MMCF

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

#define CF_RD_DATA          (*(volatile u16*)(0x08000000 + 0x00000))
#define CF_RD_ERROR         (*(volatile u16*)(0x08000000 + 0x20000))
#define CF_RD_SECTOR_COUNT  (*(volatile u16*)(0x08000000 + 0x40000))
#define CF_RD_SECTOR_NO     (*(volatile u16*)(0x08000000 + 0x60000))
#define CF_RD_CYLINDER_LOW  (*(volatile u16*)(0x08000000 + 0x80000))
#define CF_RD_CYLINDER_HIGH (*(volatile u16*)(0x08000000 + 0xA0000))
#define CF_RD_SEL_HEAD      (*(volatile u16*)(0x08000000 + 0xC0000))
#define CF_RD_STATUS        (*(volatile u16*)(0x08000000 + 0xE0000))

#define CF_WR_DATA          (*(volatile u16*)(0x08000000 + 0x00000))
#define CF_WR_FEATURES      (*(volatile u16*)(0x08000000 + 0x20000))
#define CF_WR_SECTOR_COUNT  (*(volatile u16*)(0x08000000 + 0x40000))
#define CF_WR_SECTOR_NO     (*(volatile u16*)(0x08000000 + 0x60000))
#define CF_WR_CYLINDER_LOW  (*(volatile u16*)(0x08000000 + 0x80000))
#define CF_WR_CYLINDER_HIGH (*(volatile u16*)(0x08000000 + 0xA0000))
#define CF_WR_SEL_HEAD      (*(volatile u16*)(0x08000000 + 0xC0000))
#define CF_WR_COMMAND       (*(volatile u16*)(0x08000000 + 0xE0000))


#define GAME_PAK		0x08000000			// Game pack start address
#define MP_DATA			(vu16*)(GAME_PAK + 0x01000000)		// Pointer to buffer of CF data transered from card
#define MP_REG_LBA1		*(vu16*)(GAME_PAK + 0x01060000)	// 1st byte of sector address
#define CARD_TIMEOUT	10000000		// Updated due to suggestion from SaTa, otherwise card will timeout sometimes on a write


static bool cf_block_ready(void)
{
  int i;

  i = 0;

  /*
  do
  {
    while (!(CF_RD_STATUS & 0x40));
  } while (CF_RD_STATUS & 0x80);
  */

  do
  {
    i++;
    while ( (!(CF_RD_STATUS & 0x40)) && (i < CARD_TIMEOUT) ) i++;
  } while ( (CF_RD_STATUS & 0x80) && (i < CARD_TIMEOUT) );

  if (i >= CARD_TIMEOUT) {
	return false;
  }

  return true;
}


static bool cf_set_features(u32 feature)
{
  if ( !cf_block_ready() ) return false;

  CF_WR_FEATURES = feature;
  CF_WR_SECTOR_COUNT = 0x00;  // config???
  CF_WR_SEL_HEAD = 0x00;
  CF_WR_COMMAND = 0xEF;

  return true;
}



/*-----------------------------------------------------------------
MMCF_IsInserted
Is a compact flash card inserted?
bool return OUT:  true if a CF card is inserted
-----------------------------------------------------------------*/
bool MMCF_IsInserted (void)
{
  if ( !cf_set_features(0xAA) ) return false;

  return true;
}


/*-----------------------------------------------------------------
MMCF_ClearStatus
Tries to make the CF card go back to idle mode
bool return OUT:  true if a CF card is idle
-----------------------------------------------------------------*/
bool MMCF_ClearStatus (void)
{
	return true;
}


/*-----------------------------------------------------------------
MMCF_ReadSectors
Read 512 byte sector numbered "sector" into "buffer"
u32 sector IN: address of first 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer OUT: pointer to 512 byte buffer to store data in
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool MMCF_ReadSectors (u32 sector, u8 numSecs, void* buffer)
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

  if ( !cf_block_ready() ) return false;

  CF_WR_SECTOR_COUNT = numSecs;
  CF_WR_SECTOR_NO = sector;
  CF_WR_CYLINDER_LOW = sector >> 8;
  CF_WR_CYLINDER_HIGH = sector >> 16;
  CF_WR_SEL_HEAD = ((sector >> 24) & 0x0F) | 0xE0;
  CF_WR_COMMAND = 0x20; // read sectors

  while (j--)
  {
    if ( !cf_block_ready() ) return false;

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
MMCF_WriteSectors
Write 512 byte sector numbered "sector" from "buffer"
u32 sector IN: address of 512 byte sector on CF card to read
u8 numSecs IN: number of 512 byte sectors to read,
 1 to 256 sectors can be read, 0 = 256
void* buffer IN: pointer to 512 byte buffer to read data from
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool MMCF_WriteSectors (u32 sector, u8 numSecs, void* buffer)
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

	if (numSecs > 1)
	{
		int r = 0;

		for (r = 0; r < numSecs; r++)
		{
			MMCF_WriteSectors(sector + r, 1, ((unsigned char *) (buffer)) + 512);
		}
  }

  if ( !cf_block_ready() ) return false;

  CF_WR_SECTOR_COUNT = numSecs;
  CF_WR_SECTOR_NO = sector;
  CF_WR_CYLINDER_LOW = sector >> 8;
  CF_WR_CYLINDER_HIGH = sector >> 16;
  CF_WR_SEL_HEAD = ((sector >> 24) & 0x0F) | 0xE0;
  CF_WR_COMMAND = 0x30; // write sectors

  while (j--)
  {
    if ( !cf_block_ready() ) return false;

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

//#define _CF_VERIFY

#ifdef _CF_VERIFY
	char* tmp = malloc(512);
	int r;

	for (r = 0; r < numSecs; r++)
	{
		MMCF_ReadSectors(sector + r, 1, tmp);
		while (memcmp(temp, ((unsigned char *) (buffer)) + 512 * r, 512) != 0)
		{
			consolePrintf("Rewriting sector %d\n", r);
			MMCF_WriteSectors(sector + r, 1, ((unsigned char *) (buffer)) + 512 * r);
			MMCF_ReadSectors(sector + r, 1, tmp);
		}
	}

	free(temp);
#endif

	return true;
}

/*-----------------------------------------------------------------
MMCF_Shutdown
unload the GBAMP CF interface
-----------------------------------------------------------------*/
bool MMCF_Shutdown(void)
{
	return MMCF_ClearStatus() ;
}

/*-----------------------------------------------------------------
MMCF_StartUp
initializes the CF interface, returns true if successful,
otherwise returns false
-----------------------------------------------------------------*/
bool MMCF_StartUp(void)
{
  /*
	u8 temp = MP_REG_LBA1;
	MP_REG_LBA1 = (~temp & 0xFF);
	temp = (~temp & 0xFF);
	return (MP_REG_LBA1 == temp);
  */
  if ( (CF_RD_STATUS != 0x0050) || ( *((u8 *) (0x080000B2)) == 0x96) )
  {
	return false;
  }

  return true;
}

/*-----------------------------------------------------------------
the actual interface structure
-----------------------------------------------------------------*/
IO_INTERFACE io_mmcf = {
	DEVICE_TYPE_MMCF,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&MMCF_StartUp,
	(FN_MEDIUM_ISINSERTED)&MMCF_IsInserted,
	(FN_MEDIUM_READSECTORS)&MMCF_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&MMCF_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&MMCF_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&MMCF_Shutdown
} ;

/*-----------------------------------------------------------------
MPCF_GetInterface
returns the interface structure to host
-----------------------------------------------------------------*/
LPIO_INTERFACE MMCF_GetInterface(void) {
	return &io_mmcf ;
} ;

#endif // SUPPORT_MMCF
