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

#ifndef CINE_PART_H
#define CINE_PART_H

namespace Cine {

struct PartBuffer {
	char partName[14];
	uint32 offset;
	uint32 packedSize;
	uint32 unpackedSize;
};

#define NUM_MAX_PARTDATA 255

void loadPart(const char *partName);
void closePart();

int16 findFileInBundle(const char *fileName);

void readFromPart(int16 idx, byte *dataPtr, uint32 maxSize);

byte *readBundleFile(int16 foundFileIdx, uint32 *size = NULL);
byte *readBundleSoundFile(const char *entryName, uint32 *size = 0);
byte *readFile(const char *filename, bool crypted = false);

void checkDataDisk(int16 param);

void dumpBundle(const char *filename);

} // End of namespace Cine

#endif
