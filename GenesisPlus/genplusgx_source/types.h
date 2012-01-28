/*
    Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003  Charles Mac Donald

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* OpenEmu change
#undef uint8
#undef uint16
#undef uint32
 
#define uint8  unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define int8  signed char
#define int16 signed short
#define int32 signed long int
 */

#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>

#undef uint8
#undef uint16
#undef uint32

#undef uint8
#undef uint16
#undef uint32

#define uint8  uint8_t
#define uint16 uint16_t
#define uint32 uint32_t
#define int8  int8_t
#define int16 int16_t
#define int32 int32_t

#endif /* _TYPES_H_ */
