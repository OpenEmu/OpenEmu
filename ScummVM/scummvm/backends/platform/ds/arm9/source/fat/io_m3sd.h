/*
	io_m3sd.h  by SaTa.

	Hardware Routines for reading a compact flash card
	using the GBA Movie Player

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#ifndef IO_M3SD_H
#define IO_M3SD_H

#include "disc_io.h"

u16 M3_SetChipReg(u32 Data);
void M3_SelectSaver(u8 Bank);

// export interface
extern LPIO_INTERFACE M3SD_GetInterface(void) ;

#endif
