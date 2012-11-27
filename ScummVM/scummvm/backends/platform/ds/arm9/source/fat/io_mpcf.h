/*
	io_mpcf.h

	Hardware Routines for reading a compact flash card
	using the GBA Movie Player

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#ifndef IO_MPCF_H
#define IO_MPCF_H

// 'MPCF'
#define DEVICE_TYPE_MPCF 0x4643504D

#include "disc_io.h"

// export interface
extern LPIO_INTERFACE MPCF_GetInterface(void) ;

#endif	// define IO_MPCF_H
/*
	io_mpcf.h

	Hardware Routines for reading a compact flash card
	using the GBA Movie Player

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#ifndef IO_MPCF_H
#define IO_MPCF_H

// 'MPCF'
#define DEVICE_TYPE_MPCF 0x4643504D

#include "disc_io.h"

// export interface
extern LPIO_INTERFACE MPCF_GetInterface(void) ;

#endif	// define IO_MPCF_H
