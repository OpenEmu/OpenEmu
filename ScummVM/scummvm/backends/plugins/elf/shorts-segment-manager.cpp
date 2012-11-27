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

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(MIPS_TARGET)

#include "backends/plugins/elf/shorts-segment-manager.h"

#include "common/debug.h"
#include "common/textconsole.h"

extern char __plugin_hole_start;	// Indicates start of hole in program file for shorts
extern char __plugin_hole_end;		// Indicates end of hole in program file
extern char _gp[];					// Value of gp register

namespace Common {
DECLARE_SINGLETON(ShortSegmentManager);	// For singleton
}

ShortSegmentManager::ShortSegmentManager() {
	_shortsStart = &__plugin_hole_start ;	//shorts segment begins at the plugin hole we made when linking
	_shortsEnd = &__plugin_hole_end;		//and ends at the end of that hole.
}

ShortSegmentManager::Segment *ShortSegmentManager::newSegment(uint32 size, char *origAddr) {
	char *lastAddress = origAddr;
	Common::List<Segment *>::iterator i;

	// Find a block that fits, starting from the beginning
	for (i = _list.begin(); i != _list.end(); ++i) {
		char *currAddress = (*i)->getStart();

		if (uint32(currAddress) - uint32(lastAddress) >= size)
			break;

		lastAddress = (*i)->getEnd();
	}

	if ((Elf32_Addr)lastAddress & 3)
		lastAddress += 4 - ((Elf32_Addr)lastAddress & 3);	// Round up to multiple of 4

	if (lastAddress + size > _shortsEnd) {
		warning("elfloader: No space in shorts segment for %x bytes. Last address is %p, max address is %p.",
				size, lastAddress, _shortsEnd);
		return 0;
	}

	Segment *seg = new Segment(lastAddress, size, origAddr);	// Create a new segment

	if (lastAddress + size > _highestAddress)
		_highestAddress = lastAddress + size;	// Keep track of maximum

	_list.insert(i, seg);

	debug(2, "elfloader: Shorts segment size %x allocated. End = %p. Remaining space = %x. Highest so far is %p.",
			size, lastAddress + size, _shortsEnd - _list.back()->getEnd(), _highestAddress);

	return seg;
}

void ShortSegmentManager::deleteSegment(ShortSegmentManager::Segment *seg) {
	debug(2, "elfloader: Deleting shorts segment from %p to %p.", seg->getStart(), seg->getEnd());
	_list.remove(seg);
	delete seg;
}

#endif // defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(MIPS_TARGET)
