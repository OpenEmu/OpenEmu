/****************************************************************************
 *  Genesis Plus
 *  Master System cartridge hardware support
 *
 *  Copyright (C) 1998-2007  Charles MacDonald (SMS Plus original  code)
 *  Eke-Eke (2007-2011), additional code & fixes for the GCN/Wii port
 *
 *  Realtec mapper has been figured out by TascoDeluxe
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

#ifndef _SMS_CART_H_
#define _SMS_CART_H_

/* Function prototypes */
extern void sms_cart_init(void);
extern void sms_cart_reset(void);
extern void sms_cart_switch(int enabled);
extern int sms_cart_region_detect(void);
extern int sms_cart_context_save(uint8 *state);
extern int sms_cart_context_load(uint8 *state, char *version);

#endif


