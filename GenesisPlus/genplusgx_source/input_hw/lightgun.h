/***************************************************************************************
 *  Genesis Plus
 *  Sega Light Phaser, Menacer & Konami Justifiers support
 *
 *  Copyright Eke-Eke (2007-2011)
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

#ifndef _LIGHTGUN_H_
#define _LIGHTGUN_H_

/* Input devices port handlers */
extern void lightgun_reset(int index);
extern void lightgun_refresh(int port);
extern unsigned char phaser_1_read(void);
extern unsigned char phaser_2_read(void);
extern unsigned char menacer_read(void);
extern unsigned char justifier_read(void);
extern void justifier_write(unsigned char data, unsigned char mask);

#endif
