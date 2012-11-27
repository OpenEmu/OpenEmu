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

#ifndef CRUISE_VOLUME_H
#define CRUISE_VOLUME_H

namespace Cruise {

int16 readVolCnf();
int closeCnf();
int16 findFileInDisks(const char *name);
void freeDisk();
int16 findFileInList(const char *fileName);

////////////////

void strToUpper(char *string);
void drawMsgString(const char *string);
void askDisk(int16 discNumber);
void setObjectPosition(int16 param1, int16 param2, int16 param3, int16 param4);

int closeBase();

} // End of namespace Cruise

#endif
