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

#ifndef UTIL_H
#define UTIL_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"

#ifdef WIN32
#include <io.h>
#include <process.h>
#endif


/* File I/O */
uint8 readByte(FILE *fp);
uint16 readUint16BE(FILE *fp);
uint16 readUint16LE(FILE *fp);
uint32 readUint32BE(FILE *fp);
uint32 readUint32LE(FILE *fp);
void writeByte(FILE *fp, uint8 b);
void writeUint16BE(FILE *fp, uint16 value);
void writeUint16LE(FILE *fp, uint16 value);
void writeUint32BE(FILE *fp, uint32 value);
void writeUint32LE(FILE *fp, uint32 value);
uint32 fileSize(FILE *fp);

/* Misc stuff */
void NORETURN_PRE error(const char *s, ...) NORETURN_POST;
void warning(const char *s, ...);
void debug(int level, const char *s, ...);
int scumm_stricmp(const char *s1, const char *s2);

using namespace Common;

#endif
