/***************************************************************************************
 *  Genesis Plus
 *  Z80 bank access to 68k bus
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
#ifndef _MEMBNK_H_
#define _MEMBNK_H_

extern unsigned int zbank_unused_r(unsigned int address);
extern void zbank_unused_w(unsigned int address, unsigned int data);
extern unsigned int zbank_lockup_r(unsigned int address);
extern void zbank_lockup_w(unsigned int address, unsigned int data);
extern unsigned int zbank_read_ctrl_io(unsigned int address);
extern void zbank_write_ctrl_io(unsigned int address, unsigned int data);
extern unsigned int zbank_read_vdp(unsigned int address);
extern void zbank_write_vdp(unsigned int address, unsigned int data);

struct _zbank_memory_map
{
  unsigned int (*read)(unsigned int address);
  void (*write)(unsigned int address, unsigned int data);
} zbank_memory_map[256];

#endif /* _MEMBNK_H_ */
