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

#ifndef SHORTS_SEGMENT_MANAGER_H
#define SHORTS_SEGMENT_MANAGER_H

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(MIPS_TARGET)

#include "backends/plugins/elf/elf32.h"

#include "common/singleton.h"
#include "common/list.h"

#define ShortsMan ShortSegmentManager::instance()

/**
 * ShortSegmentManager
 *
 * Since MIPS is limited to 32 bits per instruction, loading data that's further away than 16 bits
 * takes several instructions. Thus, small global data (which is likely to be accessed a lot from
 * multiple locations) is often put into a GP-relative area (GP standing for the global pointer register)
 * in MIPS processors. This class manages these segments of small global data, and is used by the
 * member functions of MIPSDLObject, which query in information from this manager in order to deal with
 * this segment during the loading/unloading of plugins.
 *
 * Since there's no true dynamic linker to change the GP register between plugins and the main engine,
 * custom ld linker scripts for both the main executable and the plugins ensure the GP-area is in the
 * same place for both. The ShortSegmentManager accesses this place via the symbols __plugin_hole_start
 * and __plugin_hole_end, which are defined in those custom ld linker scripts.
 */
class ShortSegmentManager : public Common::Singleton<ShortSegmentManager> {
private:
	char *_shortsStart;
	char *_shortsEnd;

public:
	char *getShortsStart() {
		return _shortsStart;
	}

	// Returns whether or not an absolute address is in the GP-relative section.
	bool inGeneralSegment(char *addr) {
		return (addr >= _shortsStart && addr < _shortsEnd);
	}

	class Segment {
	private:
		friend class ShortSegmentManager;
		Segment(char *start, uint32 size, char *origAddr) :
			_startAddress(start),
			_size(size),
			_origAddress(origAddr) {
		}

		virtual ~Segment() {
		}

		char *_startAddress;		// Start of shorts segment in memory
		uint32 _size;				// Size of shorts segment
		char *_origAddress;			// Original address this segment was supposed to be at

	public:
		char *getStart() {
			return _startAddress;
		}

		char *getEnd() {
			return (_startAddress + _size);
		}

		Elf32_Addr getOffset() {
			return (Elf32_Addr)(_startAddress - _origAddress);
		}

		bool inSegment(char *addr) {
			return (addr >= _startAddress && addr <= _startAddress + _size);
		}
	};

	Segment *newSegment(uint32 size, char *origAddr);
	void deleteSegment(Segment *);

private:
	ShortSegmentManager();
	friend class Common::Singleton<ShortSegmentManager>;
	Common::List<Segment *> _list;
	char *_highestAddress;
};

#endif // defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(MIPS_TARGET)

#endif /* SHORTS_SEGMENT_MANAGER_H */
