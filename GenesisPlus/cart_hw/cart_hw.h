/****************************************************************************
 *  Genesis Plus
 *  Cartridge Hardware support
 *
 *  Copyright (C) 2007, 2008, 2009  Eke-Eke (GCN/Wii port)
 *
 *  Lots of protection mechanism have been discovered by Haze
 *  (http://haze.mameworld.info/)
 *
 *  Realtec mapper has been figured out by TascoDeluxe
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/

#ifndef _CART_HW_H_
#define _CART_HW_H_

/* Hardware description */
typedef struct
{
  uint8 regs[4];                                            /* internal registers (R/W) */
  uint32 mask[4];                                           /* registers address mask */
  uint32 addr[4];                                           /* registers address */
  uint32 realtec;                                           /* bit 0: realtec mapper detected, bit 1: bootrom enabled */
  uint32 bankshift;                                         /* cartridge with bankshift mecanism */
  unsigned int (*time_r)(unsigned int address);             /* !TIME signal ($a130xx) read handler  */
  void (*time_w)(unsigned int address, unsigned int data);  /* !TIME signal ($a130xx) write handler */
  unsigned int (*regs_r)(unsigned int address);             /* cart hardware region ($400000-$7fffff) read handler  */
  void (*regs_w)(unsigned int address, unsigned int data);  /* cart hardware region ($400000-$7fffff) write handler */
} T_CART_HW;

/* global variables */
extern T_CART_HW cart_hw;
extern uint8 j_cart;
extern uint8 *default_rom;
extern int old_system[2];

/* Function prototypes */
extern void cart_hw_reset();
extern void cart_hw_init();

#endif


