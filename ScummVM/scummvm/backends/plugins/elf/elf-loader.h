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

#ifndef BACKENDS_PLUGINS_ELF_LOADER_H
#define BACKENDS_PLUGINS_ELF_LOADER_H

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER)

#include <stddef.h>

#include "backends/plugins/elf/elf32.h"
#include "backends/plugins/dynamic-plugin.h"

#include "common/stream.h"

/**
 * DLObject
 *
 * Class that most directly handles operations on a plugin file
 * (opening it for reading, loading/unloading it in memory, finding a specific symbol in the file, etc.)
 * Subclasses have the same functionality, but implementations specific to different processors/platforms.
 */
class DLObject {
protected:
	Common::SeekableReadStream *_file;

	byte *_segment;
	Elf32_Sym *_symtab;
	char *_strtab;

	uint32 _segmentSize;
	ptrdiff_t _segmentOffset;
	uint32 _segmentVMA;

	uint32 _symbol_cnt;
	int32 _symtab_sect;
	void *_dtors_start, *_dtors_end;

	virtual void unload();
	bool load();

	bool readElfHeader(Elf32_Ehdr *ehdr);
	bool readProgramHeaders(Elf32_Ehdr *ehdr, Elf32_Phdr *phdr, Elf32_Half num);
	virtual bool loadSegment(Elf32_Phdr *phdr);
	Elf32_Shdr *loadSectionHeaders(Elf32_Ehdr *ehdr);
	int findSymbolTableSection(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr);
	int loadSymbolTable(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr);
	bool loadStringTable(Elf32_Shdr *shdr);
	virtual void relocateSymbols(ptrdiff_t offset);

	// architecture specific

	/**
	 * Follow the instruction of a relocation section.
	 *
	 * @param fileOffset	Offset into the File
	 * @param size			Size of relocation section
	 * @param relSegment	Base address of relocated segment in memory (memory offset)
	 */
	virtual bool relocate(Elf32_Off offset, Elf32_Word size, byte *relSegment) = 0;
	virtual bool relocateRels(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) = 0;

	// platform specific
	virtual void flushDataCache(void *ptr, uint32 len) const = 0;

public:
	DLObject();
	virtual ~DLObject();

	/**
	 * Test the size of the plugin in memory using the memory manager.
	 * @param path			Path of file
	 */
	void trackSize(const char *path);
	bool open(const char *path);
	bool close();
	void *symbol(const char *name);
	void discardSymtab();
};

#endif /* defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) */

#endif /* BACKENDS_PLUGINS_ELF_LOADER_H */
