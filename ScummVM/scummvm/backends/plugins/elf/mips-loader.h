
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

#ifndef BACKENDS_PLUGINS_MIPS_LOADER_H
#define BACKENDS_PLUGINS_MIPS_LOADER_H

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(MIPS_TARGET)

#include "backends/plugins/elf/elf-loader.h"
#include "backends/plugins/elf/shorts-segment-manager.h"

class MIPSDLObject : public DLObject {
protected:
	ShortSegmentManager::Segment *_shortsSegment;	// For assigning shorts ranges
	uint32 _gpVal;									// Value of Global Pointer

	virtual bool relocate(Elf32_Off offset, Elf32_Word size, byte *relSegment);
	virtual bool relocateRels(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr);
	virtual void relocateSymbols(ptrdiff_t offset);
	virtual bool loadSegment(Elf32_Phdr *phdr);
	virtual void unload();

	void freeShortsSegment();

public:
	MIPSDLObject() :
		DLObject() {
		_shortsSegment = NULL;
		_gpVal = 0;
	}
	~MIPSDLObject() {
		freeShortsSegment();
	}
};

#endif /* defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(MIPS_TARGET) */

#endif /* BACKENDS_PLUGINS_MIPS_LOADER_H */
