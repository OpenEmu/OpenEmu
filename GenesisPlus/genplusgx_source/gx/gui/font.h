/*****************************************************************************
 * font.c
 *
 *   IPL font engine (using GX rendering)
 *
 *   Eke-Eke (2009,2010)
 * 
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************************/

#ifndef _FONT_H
#define _FONT_H

extern int  FONT_Init(void);
extern void FONT_Shutdown(void);
extern int  FONT_write(char *string, int size, int x, int y, int max_width, GXColor color);
extern int  FONT_writeCenter(char *string, int size, int x1, int x2, int y, GXColor color);
extern int  FONT_alignRight(char *string, int size, int x, int y, GXColor color);

#endif
