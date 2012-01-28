/***************************************************************************************
 *  Genesis Plus
 *  I/O controller (MD & MS compatibility modes)
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Eke-Eke (2007-2011), additional code & fixes for the GCN/Wii port
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
 *
 ****************************************************************************************/

#ifndef _GEN_IO_H_
#define _GEN_IO_H_

#define REGION_JAPAN_NTSC 0x00
#define REGION_JAPAN_PAL  0x40
#define REGION_USA        0x80
#define REGION_EUROPE     0xC0

/* Global variables */
extern uint8 io_reg[0x10];
extern uint8 region_code;

/* Function prototypes */
extern void io_init(void);
extern void io_reset(void);
extern void io_68k_write(unsigned int offset, unsigned int data);
extern unsigned int io_68k_read(unsigned int offset);
extern void io_z80_write(unsigned int data);
extern unsigned int io_z80_read(unsigned int offset);

#endif /* _IO_H_ */

