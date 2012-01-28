/***************************************************************************************
 *  Genesis Plus
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
 *  Sound Hardware
 ****************************************************************************************/

#ifndef _SOUND_H_
#define _SOUND_H_

/* Function prototypes */
extern void sound_init(void);
extern void sound_reset(void);
extern int sound_context_save(uint8 *state);
extern int sound_context_load(uint8 *state, char *version);
extern int sound_update(unsigned int cycles);
extern void fm_reset(unsigned int cycles);
extern void fm_write(unsigned int cycles, unsigned int address, unsigned int data);
extern unsigned int fm_read(unsigned int cycles, unsigned int address);
extern void psg_write(unsigned int cycles, unsigned int data);

#endif /* _SOUND_H_ */
