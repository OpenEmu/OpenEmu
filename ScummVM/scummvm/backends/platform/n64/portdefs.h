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

#ifndef __N64_PORTDEFS__
#define __N64_PORTDEFS__

#include <n64utils.h>

#include <sys/types.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <new>

#undef assert
#define assert(x)  ((x) ? 0 : (print_error("ASSERT TRIGGERED:\n\n("#x")\n%s\nline: %d", __FILE__, __LINE__)))

// Typedef basic data types in a way that is compatible with the N64 SDK.
typedef unsigned char byte;
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short int uint16;
typedef signed short int int16;
typedef unsigned int uint32;
typedef signed int int32;

// Define SCUMMVM_DONT_DEFINE_TYPES to prevent scummsys.h from trying to
// re-define those data types.
#define SCUMMVM_DONT_DEFINE_TYPES

#endif
