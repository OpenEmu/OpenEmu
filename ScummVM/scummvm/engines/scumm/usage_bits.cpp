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


#include "scumm/scumm.h"
#include "scumm/usage_bits.h"

namespace Scumm {

void ScummEngine::upgradeGfxUsageBits() {
	int i;

	for (i = 409; i >= 0; i--) {
		bool dirty_bit = ((gfxUsageBits[i] & 0x80000000) != 0);
		bool restored_bit = ((gfxUsageBits[i] & 0x40000000) != 0);

		gfxUsageBits[3 * i] = gfxUsageBits[i] & 0x3FFFFFFF;
		if (dirty_bit)
			setGfxUsageBit(i, USAGE_BIT_DIRTY);
		if (restored_bit)
			setGfxUsageBit(i, USAGE_BIT_RESTORED);
	}
}

void ScummEngine::setGfxUsageBit(int strip, int bit) {
	assert(strip >= 0 && strip < ARRAYSIZE(gfxUsageBits) / 3);
	assert(1 <= bit && bit <= 96);
	bit--;
	gfxUsageBits[3 * strip + bit / 32] |= (1 << (bit % 32));
}

void ScummEngine::clearGfxUsageBit(int strip, int bit) {
	assert(strip >= 0 && strip < ARRAYSIZE(gfxUsageBits) / 3);
	assert(1 <= bit && bit <= 96);
	bit--;
	gfxUsageBits[3 * strip + bit / 32] &= ~(1 << (bit % 32));
}

bool ScummEngine::testGfxUsageBit(int strip, int bit) {
	assert(strip >= 0 && strip < ARRAYSIZE(gfxUsageBits) / 3);
	assert(1 <= bit && bit <= 96);
	bit--;
	return (gfxUsageBits[3 * strip + bit / 32] & (1 << (bit % 32))) != 0;
}

bool ScummEngine::testGfxAnyUsageBits(int strip) {
	// Exclude the DIRTY and RESTORED bits from the test
	uint32 bitmask[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 0x3FFFFFFF };
	int i;

	assert(strip >= 0 && strip < ARRAYSIZE(gfxUsageBits) / 3);
	for (i = 0; i < 3; i++)
		if (gfxUsageBits[3 * strip + i] & bitmask[i])
			return true;

	return false;
}

bool ScummEngine::testGfxOtherUsageBits(int strip, int bit) {
	// Don't exclude the DIRTY and RESTORED bits from the test
	uint32 bitmask[3] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
	int i;

	assert(strip >= 0 && strip < ARRAYSIZE(gfxUsageBits) / 3);
	assert(1 <= bit && bit <= 96);
	bit--;
	bitmask[bit / 32] &= ~(1 << (bit % 32));

	for (i = 0; i < 3; i++)
		if (gfxUsageBits[3 * strip + i] & bitmask[i])
			return true;

	return false;
}

} // End of namespace Scumm
