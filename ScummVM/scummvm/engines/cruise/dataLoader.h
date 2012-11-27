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

#ifndef CRUISE_DATALOADER_H
#define CRUISE_DATALOADER_H

namespace Cruise {

int loadFNTSub(uint8 *ptr, int destIdx);
int loadSPLSub(uint8 *ptr, int destIdx);
int loadSetEntry(const char *name, uint8 *ptr, int currentEntryIdx, int currentDestEntry);
int loadFile(const char* name, int idx, int destIdx);
int loadData(const char * name, int startIdx);
int loadFileRange(const char * name, int param, int startIdx, int numIdx);
int loadFileSub1(uint8 ** ptr, const char * name, uint8 * ptr2);
int loadFullBundle(const char * name, int startIdx);

} // End of namespace Cruise

#endif
