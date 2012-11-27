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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "scumm/util.h"
#include "common/util.h"
#include "common/textconsole.h"

namespace Scumm {

#pragma mark -
#pragma mark --- Utilities ---
#pragma mark -

void assertRange(int min, int value, int max, const char *desc) {
	if (value < min || value > max) {
		error("%s %d is out of bounds (%d,%d)", desc, value, min, max);
	}
}

/**
 * Convert an old style direction to a new style one (angle),
 */
int newDirToOldDir(int dir) {
	if (dir >= 71 && dir <= 109)
		return 1;
	if (dir >= 109 && dir <= 251)
		return 2;
	if (dir >= 251 && dir <= 289)
		return 0;
	return 3;
}

/**
 * Convert an new style (angle) direction to an old style one.
 */
int oldDirToNewDir(int dir) {
	assert(0 <= dir && dir <= 3);
	const int new_dir_table[4] = { 270, 90, 180, 0 };
	return new_dir_table[dir];
}

/**
 * Convert an angle to a simple direction.
 */
int toSimpleDir(int dirType, int dir) {
	if (dirType) {
		const int16 directions[] = { 22,  72, 107, 157, 202, 252, 287, 337 };
		for (int i = 0; i < 7; i++)
			if (dir >= directions[i] && dir <= directions[i+1])
				return i+1;
	} else {
		const int16 directions[] = { 71, 109, 251, 289 };
		for (int i = 0; i < 3; i++)
			if (dir >= directions[i] && dir <= directions[i+1])
				return i+1;
	}

	return 0;
}

/**
 * Convert a simple direction to an angle.
 */
int fromSimpleDir(int dirType, int dir) {
	if (dirType)
		return dir * 45;
	else
		return dir * 90;
}

/**
 * Normalize the given angle - that means, ensure it is positive, and
 * change it to the closest multiple of 45 degree by abusing toSimpleDir.
 */
int normalizeAngle(int angle) {
	int temp;

	temp = (angle + 360) % 360;

	return toSimpleDir(1, temp) * 45;
}

} // End of namespace Scumm
