/*  cdrdao - write audio CD-Rs in disc-at-once mode
 *
 *  Copyright (C) 1998-2002 Andreas Mueller <mueller@daneb.ping.de>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __LEC_H__
#define __LEC_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <inttypes.h>

typedef uint32_t u_int32_t;
typedef uint16_t u_int16_t;
typedef uint8_t u_int8_t;

#ifndef TRUE
#define TRUE 1
#endif

/* Encodes a MODE 0 sector.
 * 'adr' is the current physical sector address
 * 'sector' must be 2352 byte wide
 */
void lec_encode_mode0_sector(u_int32_t adr, u_int8_t *sector);

/* Encodes a MODE 1 sector.
 * 'adr' is the current physical sector address
 * 'sector' must be 2352 byte wide containing 2048 bytes user data at
 * offset 16
 */
void lec_encode_mode1_sector(u_int32_t adr, u_int8_t *sector);

/* Encodes a MODE 2 sector.
 * 'adr' is the current physical sector address
 * 'sector' must be 2352 byte wide containing 2336 bytes user data at
 * offset 16
 */
void lec_encode_mode2_sector(u_int32_t adr, u_int8_t *sector);

/* Encodes a XA form 1 sector.
 * 'adr' is the current physical sector address
 * 'sector' must be 2352 byte wide containing 2048+8 bytes user data at
 * offset 16
 */
void lec_encode_mode2_form1_sector(u_int32_t adr, u_int8_t *sector);

/* Encodes a XA form 2 sector.
 * 'adr' is the current physical sector address
 * 'sector' must be 2352 byte wide containing 2324+8 bytes user data at
 * offset 16
 */
void lec_encode_mode2_form2_sector(u_int32_t adr, u_int8_t *sector);

/* Scrambles and byte swaps an encoded sector.
 * 'sector' must be 2352 byte wide.
 */
void lec_scramble(u_int8_t *sector);

#endif
