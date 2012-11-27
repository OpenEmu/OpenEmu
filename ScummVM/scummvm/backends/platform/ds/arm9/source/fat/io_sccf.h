/*
	io_sccf.h

	Hardware Routines for reading a compact flash card
	using the Supercard CF

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#ifndef IO_SCCF_H
#define IO_SCCF_H

// 'SCCF'
#define DEVICE_TYPE_SCCF 0x46434353

#include "disc_io.h"

// export interface
extern LPIO_INTERFACE SCCF_GetInterface(void) ;

#endif	// define IO_SCCF_H
/*
	io_sccf.h

	Hardware Routines for reading a compact flash card
	using the Supercard CF

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.
*/

#ifndef IO_SCCF_H
#define IO_SCCF_H

// 'SCCF'
#define DEVICE_TYPE_SCCF 0x46434353

#include "disc_io.h"

// export interface
extern LPIO_INTERFACE SCCF_GetInterface(void) ;

#endif	// define IO_SCCF_H
