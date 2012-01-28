/*
 * file_fat.c
 * 
 *  FAT loading support
 *
 *  Eke-Eke (2008,2009)
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
 ********************************************************************************/

#ifndef _FILE_FAT_H
#define _FILE_FAT_H


#define TYPE_SD (0)
#ifdef HW_RVL
#define TYPE_USB (1)
#define TYPE_DVD (2)
#else
#define TYPE_DVD (1)
#endif
#define TYPE_RECENT (TYPE_DVD + 1)

#define FILECHUNK (2048)

extern int OpenDirectory(int device);
extern int UpdateDirectory(bool go_up, char *filename);
extern int ParseDirectory(void);
extern int LoadFile(u8* buffer,u32 selection, char *filename);

#endif
