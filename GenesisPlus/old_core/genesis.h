/***************************************************************************************
 *  Genesis Plus
 *  Genesis internals & Bus controller
 *
 *  Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald (original code)
 *  Eke-Eke (2007,2008,2009), additional code & fixes for the GCN/Wii port
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
extern uint8 *cart_rom;
extern uint8 bios_rom[0x10000];
extern uint8 work_ram[0x10000];
extern uint8 zram[0x2000];
extern uint8 zbusreq;
extern uint8 zbusack;
extern uint8 zreset;
extern uint8 zirq;
extern uint32 zbank;
extern uint8 gen_running;
extern uint32 genromsize;
extern int32 resetline;

/* Function prototypes */
extern void gen_init(void);
extern void gen_reset(uint32 hard_reset);
extern void gen_shutdown(void);
extern void gen_busreq_w(uint32 state);
extern void gen_reset_w(uint32 state);
extern void gen_bank_w(uint32 state);
extern int z80_irq_callback(int param);
extern void set_softreset(void);

#endif /* _GEN_H_ */

