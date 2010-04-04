/***************************************************************************************
 *  Genesis Plus
 *  68k memory handlers
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

#ifndef _MEM68K_H_
#define _MEM68K_H_

/* unused areas */
extern uint32 m68k_read_bus_8(uint32 address);
extern uint32 m68k_read_bus_16(uint32 address);
extern void m68k_unused_8_w (uint32 address, uint32 data);
extern void m68k_unused_16_w (uint32 address, uint32 data);

/* illegal areas */
extern uint32 m68k_lockup_r_8 (uint32 address);
extern uint32 m68k_lockup_r_16 (uint32 address);
extern void m68k_lockup_w_8 (uint32 address, uint32 data);
extern void m68k_lockup_w_16 (uint32 address, uint32 data);

/* eeprom */
extern uint32 eeprom_read_byte(uint32 address);
extern uint32 eeprom_read_word(uint32 address);
extern void eeprom_write_byte(uint32 address, uint32 data);
extern void eeprom_write_word(uint32 address, uint32 data);

/* Z80 bus */
extern uint32 z80_read_byte(uint32 address);
extern uint32 z80_read_word(uint32 address);
extern void z80_write_byte(uint32 address, uint32 data);
extern void z80_write_word(uint32 address, uint32 data);

/* I/O & Control registers */
extern uint32 ctrl_io_read_byte(uint32 address);
extern uint32 ctrl_io_read_word(uint32 address);
extern void ctrl_io_write_byte(uint32 address, uint32 data);
extern void ctrl_io_write_word(uint32 address, uint32 data);

/* VDP */
extern uint32 vdp_read_byte(uint32 address);
extern uint32 vdp_read_word(uint32 address);
extern void vdp_write_byte(uint32 address, uint32 data);
extern void vdp_write_word(uint32 address, uint32 data);

/* PICO */
extern uint32 pico_read_byte(uint32 address);
extern uint32 pico_read_word(uint32 address);

uint8 m68k_readmap[256];
uint8 m68k_writemap[256];
uint32 pico_current;

#endif /* _MEM68K_H_ */
