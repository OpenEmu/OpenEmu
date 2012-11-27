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

// The hash map (associative array) implementation in this file is
// based on the PyDict implementation of CPython. The erase() method
// is based on example code in the Wikipedia article on Hash tables.

#include "common/hashmap.h"

namespace Common {

// Hash function for strings, taken from CPython.
uint hashit(const char *p) {
	uint hash = *p << 7;
	byte c;
	int size = 0;
	while ((c = *p++)) {
		hash = (1000003 * hash) ^ c;
		size++;
	}
	return hash ^ size;
}

// Like hashit, but converts every char to lowercase before hashing.
uint hashit_lower(const char *p) {
	uint hash = tolower(*p) << 7;
	byte c;
	int size = 0;
	while ((c = *p++)) {
		hash = (1000003 * hash) ^ tolower(c);
		size++;
	}
	return hash ^ size;
}

#ifdef DEBUG_HASH_COLLISIONS
static double
	g_collisions = 0,
	g_dummyHits = 0,
	g_lookups = 0,
	g_collPerLook = 0,
	g_capacity = 0,
	g_size = 0;
static int g_max_capacity = 0, g_max_size = 0;
static int g_totalHashmaps = 0;
static int g_stats[4] = {0,0,0,0};

void updateHashCollisionStats(int collisions, int dummyHits, int lookups, int arrsize, int nele) {
	g_collisions += collisions;
	g_lookups += lookups;
	g_dummyHits += dummyHits;
	if (lookups)
		g_collPerLook += (double)collisions / (double)lookups;
	g_capacity += arrsize;
	g_size += nele;
	g_totalHashmaps++;

	if (3*nele <= 2*8)
		g_stats[0]++;
	if (3*nele <= 2*16)
		g_stats[1]++;
	if (3*nele <= 2*32)
		g_stats[2]++;
	if (3*nele <= 2*64)
		g_stats[3]++;

	g_max_capacity = MAX(g_max_capacity, arrsize);
	g_max_size = MAX(g_max_size, nele);

	debug("%d hashmaps: colls %.1f; dummies hit %.1f, lookups %.1f; ratio %.3f%%; size %f (max: %d); capacity %f (max: %d)",
		g_totalHashmaps,
		g_collisions / g_totalHashmaps,
		g_dummyHits / g_totalHashmaps,
		g_lookups / g_totalHashmaps,
		100 * g_collPerLook / g_totalHashmaps,
		g_size / g_totalHashmaps, g_max_size,
		g_capacity / g_totalHashmaps, g_max_capacity);
	debug("  %d less than %d; %d less than %d; %d less than %d; %d less than %d",
			g_stats[0], 2*8/3,
			g_stats[1],2*16/3,
			g_stats[2],2*32/3,
			g_stats[3],2*64/3);

	// TODO:
	// * Should record the maximal size of the map during its lifetime, not that at its death
	// * Should do some statistics: how many maps are less than 2/3*8, 2/3*16, 2/3*32, ...
}
#endif

}	// End of namespace Common
