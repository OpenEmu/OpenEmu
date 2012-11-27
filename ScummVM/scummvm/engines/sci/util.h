/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SCI_UTIL_H
#define SCI_UTIL_H

#include "common/scummsys.h"

namespace Sci {

// Wrappers for reading/writing 16-bit values in the endianness
// of the original game platform.
uint16 READ_SCIENDIAN_UINT16(const void *ptr);
void WRITE_SCIENDIAN_UINT16(void *ptr, uint16 val);

// Wrappers for reading integer values for SCI1.1+.
// Mac versions have big endian data for some fields.
uint16 READ_SCI11ENDIAN_UINT16(const void *ptr);
uint32 READ_SCI11ENDIAN_UINT32(const void *ptr);
void WRITE_SCI11ENDIAN_UINT16(void *ptr, uint16 val);

// Wrappers for reading integer values in resources that are
// LE in SCI1.1 Mac, but BE in SCI32 Mac
uint16 READ_SCI32ENDIAN_UINT16(const void *ptr);

} // End of namespace Sci

#endif
