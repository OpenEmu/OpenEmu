/*
  io_efa2.h by CyteX

  Based on io_mpfc.h by chishm (Michael Chisholm)

  Hardware Routines for reading the NAND flash located on
  EFA2 flash carts

  This software is completely free. No warranty is provided.
  If you use it, please give me credit and email me about your
  project at cytex <at> gmx <dot> de and do not forget to also
  drop chishm <at> hotmail <dot> com a line

  See gba_nds_fat.txt for help and license details.
*/

#ifndef IO_EFA2_H
#define IO_EFA2_H

// 'EFA2'
#define DEVICE_TYPE_EFA2 0x32414645

#include "disc_io.h"

// export interface
extern LPIO_INTERFACE EFA2_GetInterface(void);

#endif	// define IO_EFA2_H
