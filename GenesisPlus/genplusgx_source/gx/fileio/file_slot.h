/*
 *  file_slot.c
 *
 *  FAT and Memory Card SRAM/Savestate files managment
 *
 *  Softdev (2006)
 *  Eke-Eke (2007,2008,2009)
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
 ***************************************************************************/

#ifndef _FILE_SLOT_H
#define _FILE_SLOT_H

typedef struct 
{
  int valid;
  u16 year;
  u8 month;
  u8 day;
  u8 hour;
  u8 min;
} t_slot;

extern void slot_autoload(int slot, int device);
extern void slot_autosave(int slot, int device);
extern void slot_autodetect(int slot, int device, t_slot *ptr);
extern int slot_delete(int slot, int device);
extern int slot_load(int slot, int device);
extern int slot_save(int slot, int device);

#endif
