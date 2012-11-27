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

#ifndef KYRA_UTIL_H
#define KYRA_UTIL_H

#include "common/scummsys.h"

namespace Kyra {

class Util {
public:
	static int decodeString1(const char *src, char *dst);
	static void decodeString2(const char *src, char *dst);

	// Since our current GUI font uses ISO-8859-1, this
	// conversion functionallty uses that as a base.
	static void convertDOSToISO(char *str);
	static void convertISOToDOS(char *str);
	static void convertISOToDOS(char &c);

private:
	static const uint8 _charMapDOSToISO[128];
	static const uint8 _charMapISOToDOS[128];
};

} // End of namespace Kyra

#endif
