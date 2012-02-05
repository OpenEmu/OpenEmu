/***************************************************************************************
 *  Genesis Plus
 *  Internal hardware & Bus controllers
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

#ifndef _GENESIS_H_
#define _GENESIS_H_

/* Global variables */
extern uint8 tmss[4];
extern uint8 bios_rom[0x800];
extern uint8 work_ram[0x10000];
extern uint8 zram[0x2000];
extern uint32 zbank;
extern uint8 zstate;
extern uint8 pico_current;
extern uint8 pico_page[7];

/* Function prototypes */
extern void gen_init(void);
extern void gen_reset(int hard_reset);
extern void gen_shutdown(void);
extern void gen_tmss_w(unsigned int offset, unsigned int data);
extern void gen_bankswitch_w(unsigned int data);
extern unsigned int gen_bankswitch_r(void);
extern void gen_zbusreq_w(unsigned int state, unsigned int cycles);
extern void gen_zreset_w(unsigned int state, unsigned int cycles);
extern void gen_zbank_w(unsigned int state);
extern int z80_irq_callback(int param);

#endif /* _GEN_H_ */

