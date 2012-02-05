/***************************************************************************************
 *  Genesis Plus
 *  3-Buttons & 6-Buttons pad support (incl. 4-WayPlay & J-Cart handlers)
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

#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

/* Function prototypes */
extern void gamepad_reset(int port);
extern void gamepad_refresh(int port);
extern unsigned char gamepad_1_read(void);
extern unsigned char gamepad_2_read(void);
extern void gamepad_1_write(unsigned char data, unsigned char mask);
extern void gamepad_2_write(unsigned char data, unsigned char mask);
extern unsigned char wayplay_1_read(void);
extern unsigned char wayplay_2_read(void);
extern void wayplay_1_write(unsigned char data, unsigned char mask);
extern void wayplay_2_write(unsigned char data, unsigned char mask);
extern unsigned int jcart_read(unsigned int address);
extern void jcart_write(unsigned int address, unsigned int data);

#endif
