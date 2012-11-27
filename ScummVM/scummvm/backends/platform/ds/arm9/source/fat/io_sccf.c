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


#include "io_sccf.h"

#ifdef SUPPORT_SCCF

#ifndef SUPPORT_MPCF
 #error Supercard CF support requires GBAMP CF support
#endif // SUPPORT_MPCF

/*-----------------------------------------------------------------
Since all CF addresses and commands are the same for the GBAMP,
simply use it's functions instead.
-----------------------------------------------------------------*/

extern bool MPCF_IsInserted (void);
extern bool MPCF_ClearStatus (void);
extern bool MPCF_ReadSectors (u32 sector, u8 numSecs, void* buffer);
extern bool MPCF_WriteSectors (u32 sector, u8 numSecs, void* buffer);


/*-----------------------------------------------------------------
SCCF_Unlock
Returns true if SuperCard was unlocked, false if failed
Added by MightyMax
Modified by Chishm
-----------------------------------------------------------------*/
bool SCCF_Unlock(void)
{
#define CF_REG_LBA1 *(volatile unsigned short *)0x09060000
	unsigned char temp;
	volatile short *unlockAddress = (volatile short *)0x09FFFFFE;
	*unlockAddress = 0xA55A ;
	*unlockAddress = 0xA55A ;
	*unlockAddress = 0x3 ;
	*unlockAddress = 0x3 ;
	// provoke a ready reply
	temp = CF_REG_LBA1;
	CF_REG_LBA1 = (~temp & 0xFF);
	temp = (~temp & 0xFF);
	return (CF_REG_LBA1 == temp);
#undef CF_REG_LBA1
}

bool SCCF_Shutdown(void) {
	return MPCF_ClearStatus() ;
} ;

bool SCCF_StartUp(void) {
	return SCCF_Unlock() ;
} ;


IO_INTERFACE io_sccf = {
	DEVICE_TYPE_SCCF,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&SCCF_StartUp,
	(FN_MEDIUM_ISINSERTED)&MPCF_IsInserted,
	(FN_MEDIUM_READSECTORS)&MPCF_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&MPCF_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&MPCF_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&SCCF_Shutdown
} ;


LPIO_INTERFACE SCCF_GetInterface(void) {
	return &io_sccf ;
} ;

#endif // SUPPORT_SCCF
