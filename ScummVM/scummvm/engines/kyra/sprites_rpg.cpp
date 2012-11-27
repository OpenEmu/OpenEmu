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

#if defined(ENABLE_EOB) || defined(ENABLE_LOL)

#include "kyra/kyra_rpg.h"

namespace Kyra {

int KyraRpgEngine::getBlockDistance(uint16 block1, uint16 block2) {
	int b1x = block1 & 0x1f;
	int b1y = block1 >> 5;
	int b2x = block2 & 0x1f;
	int b2y = block2 >> 5;

	uint8 dy = ABS(b2y - b1y);
	uint8 dx = ABS(b2x - b1x);

	if (dx > dy)
		SWAP(dx, dy);

	return (dx >> 1) + dy;
}

} // namespace Kyra

#endif
