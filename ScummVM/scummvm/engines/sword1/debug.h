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

#ifndef SWORD1_DEBUG_H
#define SWORD1_DEBUG_H

#include "common/scummsys.h"

namespace Sword1 {

class Debug {
public:
	static void interpretScript(uint32 id, uint32 level, uint32 script, uint32 pc);
	static void callMCode(uint32 mcodeNum, uint32 paramCount, int32 a, int32 b, int32 c, int32 d, int32 e, int32 f);

private:
	static const char _mCodeNames[100][35];
};

} // End of namespace Sword1

#endif // BSDEBUG_H
