/****************************************************************************
 *  Genesis Plus
 *  Game Genie Hardware emulation
 *
 *  Copyright (C) 2009  Eke-Eke (GCN/Wii port)
 *
 *  Based on documentation from Charles McDonald
 *  (http://cgfm2.emuviews.com/txt/genie.txt)
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
 ***************************************************************************/

#ifndef _GGENIE_H_
#define _GGENIE_H_

/* Function prototypes */
extern void ggenie_init(void);
extern void ggenie_shutdown(void);
extern void ggenie_reset(int hard);
extern void ggenie_switch(int enable);

#endif
