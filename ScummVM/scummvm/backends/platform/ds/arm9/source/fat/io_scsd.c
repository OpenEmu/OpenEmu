/*
	io_scsd.c by SaTa.
	based on io_sccf.c


*/

/*
	io_sccf.c based on

	compact_flash.c
	By chishm (Michael Chisholm)

	Hardware Routines for reading a compact flash card
	using the Super Card CF

	CF routines modified with help from Darkfader

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/


#include "io_scsd.h"

#ifdef SUPPORT_SCSD

/*-----------------------------------------------------------------
Since all CF addresses and commands are the same for the GBAMP,
simply use it's functions instead.
-----------------------------------------------------------------*/

extern bool MPCF_IsInserted (void);
extern bool MPCF_ClearStatus (void);
extern bool MPCF_ReadSectors (u32 sector, u8 numSecs, void* buffer);
extern bool MPCF_WriteSectors (u32 sector, u8 numSecs, void* buffer);

//	add by SaTa.
extern void InitSCMode(void);	//	CF‚Æ“¯‚¶
extern void ReadSector(u16 *buff,u32 sector,u8 ReadNumber);
extern void WriteSector(u16 *buff,u32 sector,u8 writeNumber);
extern bool MemoryCard_IsInserted(void);	//	CF‚Æˆá‚¤
//

/*-----------------------------------------------------------------
SCSD_Unlock
Returns true if SuperCard was unlocked, false if failed
Added by MightyMax
Modified by Chishm
-----------------------------------------------------------------*/
bool SCSD_Unlock(void)
{
	InitSCMode();
	return MemoryCard_IsInserted();
}

bool SCSD_Shutdown(void) {
	return MPCF_ClearStatus() ;
} ;

bool SCSD_StartUp(void) {
	return SCSD_Unlock() ;
} ;

bool SCSD_ReadSectors (u32 sector, u8 ReadNumber, void* buff)
{
	ReadSector((u16 *)buff,sector,ReadNumber);
	return true;
}

bool SCSD_WriteSectors (u32 sector, u8 writeNumber, void* buff)
{
	u16* alignedBuffer = (u16 *) malloc(512);
	int r;

	for (r = 0; r < writeNumber; r++)
	{
		memcpy(alignedBuffer, buff, 512);
		WriteSector(((u16 *)(buff)) + (r * 256), sector + r, 1);
	}

	free(alignedBuffer);
	return true;
}


IO_INTERFACE io_scsd = {
	0x44534353,	// 'SCSD'
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE,
	(FN_MEDIUM_STARTUP)&SCSD_StartUp,
	(FN_MEDIUM_ISINSERTED)&SCSD_Unlock,
	(FN_MEDIUM_READSECTORS)&SCSD_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&SCSD_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&MPCF_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&SCSD_Shutdown
} ;


LPIO_INTERFACE SCSD_GetInterface(void) {
	return &io_scsd ;
} ;

#endif
