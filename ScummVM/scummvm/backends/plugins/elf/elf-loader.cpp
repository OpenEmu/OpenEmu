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

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER)

#include "backends/plugins/elf/elf-loader.h"
#include "backends/plugins/elf/memory-manager.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/ptr.h"

#include <malloc.h>	// for memalign()

DLObject::DLObject() :
	_file(0),
	_segment(0),
	_symtab(0),
	_strtab(0),
	_segmentSize(0),
	_segmentOffset(0),
	_segmentVMA(0),
	_symbol_cnt(0),
	_symtab_sect(-1),
	_dtors_start(0),
	_dtors_end(0) {
}

DLObject::~DLObject() {
	discardSymtab();
	ELFMemMan.pluginDeallocate(_segment);
	_segment = 0;
}

// Expel the symbol table from memory
void DLObject::discardSymtab() {
	free(_symtab);
	_symtab = 0;

	free(_strtab);
	_strtab = 0;

	_symbol_cnt = 0;
}

// Unload all objects from memory
void DLObject::unload() {
	discardSymtab();

	ELFMemMan.pluginDeallocate(_segment);

	_segment = 0;
	_segmentSize = 0;
	_segmentOffset = 0;
	_segmentVMA = 0;
}

bool DLObject::readElfHeader(Elf32_Ehdr *ehdr) {
	assert(_file);

	// Start reading the elf header. Check for errors and magic
	if (_file->read(ehdr, sizeof(*ehdr)) != sizeof(*ehdr) ||
			memcmp(ehdr->e_ident, ELFMAG, SELFMAG)) {
		warning("elfloader: No ELF file.");
		return false;
	}

	if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
		warning("elfloader: Wrong ELF file class.");
		return false;
	}

	if (ehdr->e_ident[EI_DATA] !=
#ifdef SCUMM_BIG_ENDIAN
			ELFDATA2MSB
#else
			ELFDATA2LSB
#endif
			) {
		warning("elfloader: Wrong ELF file endianess.");
		return false;
	}

	if (ehdr->e_ident[EI_VERSION] != EV_CURRENT) {
		warning("elfloader: Wrong ELF file version.");
		return false;
	}

	if (ehdr->e_type != ET_EXEC) {
		warning("elfloader: No executable ELF file.");
		return false;
	}

	if (ehdr->e_machine !=
#ifdef ARM_TARGET
			EM_ARM
#endif
#ifdef MIPS_TARGET
			EM_MIPS
#endif
#ifdef PPC_TARGET
			EM_PPC
#endif
			) {
		warning("elfloader: Wrong ELF file architecture.");
		return false;
	}

	if (ehdr->e_phentsize < sizeof(Elf32_Phdr)	 ||			// Check for size of program header
			ehdr->e_shentsize != sizeof(Elf32_Shdr)) {		// Check for size of section header
		warning("elfloader: Invalid ELF structure sizes.");
		return false;
	}

	debug(2, "elfloader: phoff = %d, phentsz = %d, phnum = %d",
			ehdr->e_phoff, ehdr->e_phentsize, ehdr->e_phnum);

	return true;
}

bool DLObject::readProgramHeaders(Elf32_Ehdr *ehdr, Elf32_Phdr *phdr, Elf32_Half num) {
	assert(_file);

	// Read program header
	if (!_file->seek(ehdr->e_phoff + sizeof(*phdr) * num, SEEK_SET) ||
			_file->read(phdr, sizeof(*phdr)) != sizeof(*phdr)) {
		warning("elfloader: Program header load failed.");
		return false;
	}

	// Check program header values
	if (phdr->p_type != PT_LOAD  || phdr->p_filesz > phdr->p_memsz) {
		warning("elfloader: Invalid program header.");
		return false;
	}

	debug(2, "elfloader: offs = %x, filesz = %x, memsz = %x, align = %x",
			phdr->p_offset, phdr->p_filesz, phdr->p_memsz, phdr->p_align);

	return true;
}

bool DLObject::loadSegment(Elf32_Phdr *phdr) {
	_segment = (byte *)ELFMemMan.pluginAllocate(phdr->p_align, phdr->p_memsz);

	if (!_segment) {
		warning("elfloader: Out of memory.");
		return false;
	}

	debug(2, "elfloader: Allocated segment @ %p", _segment);

	// Get offset to load segment into
	_segmentSize = phdr->p_memsz;
	_segmentVMA = phdr->p_vaddr;

	// Set .bss segment to 0 if necessary
	if (phdr->p_memsz > phdr->p_filesz) {
		debug(2, "elfloader: Setting %p to %p to 0 for bss",
				_segment + phdr->p_filesz, _segment + phdr->p_memsz);
		memset(_segment + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
	}

	debug(2, "elfloader: Reading the segment into memory");

	// Read the segment into memory
	if (!_file->seek(phdr->p_offset, SEEK_SET) ||
			_file->read(_segment, phdr->p_filesz) != phdr->p_filesz) {
		warning("elfloader: Segment load failed.");
		return false;
	}

	debug(2, "elfloader: Segment has been read into memory");

	return true;
}

Elf32_Shdr * DLObject::loadSectionHeaders(Elf32_Ehdr *ehdr) {
	assert(_file);

	Elf32_Shdr *shdr = 0;

	// Allocate memory for section headers
	if (!(shdr = (Elf32_Shdr *)malloc(ehdr->e_shnum * sizeof(*shdr)))) {
		warning("elfloader: Out of memory.");
		return 0;
	}

	// Read from file into section headers
	if (!_file->seek(ehdr->e_shoff, SEEK_SET) ||
			_file->read(shdr, ehdr->e_shnum * sizeof(*shdr)) !=
			ehdr->e_shnum * sizeof(*shdr)) {
		warning("elfloader: Section headers load failed.");
		free(shdr);
		return 0;
	}

	return shdr;
}

int DLObject::findSymbolTableSection(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) {
	int SymbolTableSection = -1;

	// Loop over sections, looking for symbol table linked to a string table
	for (uint32 i = 0; i < ehdr->e_shnum; i++) {
		if (shdr[i].sh_type == SHT_SYMTAB &&
				shdr[i].sh_entsize == sizeof(Elf32_Sym) &&
				shdr[i].sh_link < ehdr->e_shnum &&
				shdr[shdr[i].sh_link].sh_type == SHT_STRTAB) {
			SymbolTableSection = i;
			break;
		}
	}

	return SymbolTableSection;
}

int DLObject::loadSymbolTable(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) {
	assert(_file);

	_symtab_sect = findSymbolTableSection(ehdr, shdr);

	// Check for no symbol table
	if (_symtab_sect < 0) {
		warning("elfloader: No symbol table.");
		return -1;
	}

	debug(2, "elfloader: Symbol section at section %d, size %x",
			_symtab_sect, shdr[_symtab_sect].sh_size);

	// Allocate memory for symbol table
	if (!(_symtab = (Elf32_Sym *)malloc(shdr[_symtab_sect].sh_size))) {
		warning("elfloader: Out of memory.");
		return -1;
	}

	// Read symbol table into memory
	if (!_file->seek(shdr[_symtab_sect].sh_offset, SEEK_SET) ||
			_file->read(_symtab, shdr[_symtab_sect].sh_size) !=
			shdr[_symtab_sect].sh_size) {
		warning("elfloader: Symbol table load failed.");
		free(_symtab);
		_symtab = 0;
		return -1;
	}

	// Set number of symbols
	_symbol_cnt = shdr[_symtab_sect].sh_size / sizeof(Elf32_Sym);
	debug(2, "elfloader: Loaded %d symbols.", _symbol_cnt);

	return _symtab_sect;
}

bool DLObject::loadStringTable(Elf32_Shdr *shdr) {
	assert(_file);

	uint32 string_sect = shdr[_symtab_sect].sh_link;

	// Allocate memory for string table
	if (!(_strtab = (char *)malloc(shdr[string_sect].sh_size))) {
		warning("elfloader: Out of memory.");
		return false;
	}

	// Read string table into memory
	if (!_file->seek(shdr[string_sect].sh_offset, SEEK_SET) ||
			_file->read(_strtab, shdr[string_sect].sh_size) !=
			shdr[string_sect].sh_size) {
		warning("elfloader: Symbol table strings load failed.");
		free(_strtab);
		_strtab = 0;
		return false;
	}

	return true;
}

void DLObject::relocateSymbols(ptrdiff_t offset) {
	// Loop over symbols, add relocation offset
	Elf32_Sym *s = _symtab;

	for (uint32 c = _symbol_cnt; c--; s++) {
		// Make sure we don't relocate special valued symbols
		if (s->st_shndx < SHN_LOPROC) {
			s->st_value += offset;

			if (s->st_value < Elf32_Addr(_segment) ||
					s->st_value > Elf32_Addr(_segment) + _segmentSize)
				warning("elfloader: Symbol out of bounds! st_value = %x", s->st_value);
		}
	}
}

// Track the size of the plugin through memory manager without loading
// the plugin into memory.
//
void DLObject::trackSize(const char *path) {

	_file = Common::FSNode(path).createReadStream();

	if (!_file) {
		warning("elfloader: File %s not found.", path);
		return;
	}

	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;

	if (!readElfHeader(&ehdr)) {
		delete _file;
		_file = 0;
		return;
	}

	ELFMemMan.trackPlugin(true);	// begin tracking the plugin size

	// Load the segments
	for (uint32 i = 0; i < ehdr.e_phnum; i++) {
		debug(2, "elfloader: Loading segment %d", i);

		if (!readProgramHeaders(&ehdr, &phdr, i)) {
			delete _file;
			_file = 0;
			return;
		}

		if (phdr.p_flags & PF_X) {	// check for executable, allocated segment
			ELFMemMan.trackAlloc(phdr.p_align, phdr.p_memsz);
		}
	}

	ELFMemMan.trackPlugin(false);	// we're done tracking the plugin size

	delete _file;
	_file = 0;
	// No need to track the symbol table sizes -- they get discarded
}

bool DLObject::load() {
	Elf32_Ehdr ehdr;
	Elf32_Phdr phdr;

	if (readElfHeader(&ehdr) == false)
		return false;

	//Load the segments
	for (uint32 i = 0; i < ehdr.e_phnum; i++) {
		debug(2, "elfloader: Loading segment %d", i);

		if (readProgramHeaders(&ehdr, &phdr, i) == false)
			return false;

		if (!loadSegment(&phdr))
			return false;
	}

	Elf32_Shdr *shdr = loadSectionHeaders(&ehdr);
	if (!shdr)
		return false;

	_symtab_sect = loadSymbolTable(&ehdr, shdr);
	if (_symtab_sect < 0) {
		free(shdr);
		return false;
	}

	if (!loadStringTable(shdr)) {
		free(shdr);
		return false;
	}

	// Offset by our segment allocated address
	// must use _segmentVMA here for multiple segments (MIPS)
	_segmentOffset = ptrdiff_t(_segment) - _segmentVMA;
	relocateSymbols(_segmentOffset);

	if (!relocateRels(&ehdr, shdr)) {
		free(shdr);
		return false;
	}

	return true;
}

bool DLObject::open(const char *path) {
	void *ctors_start, *ctors_end;

	debug(2, "elfloader: open(\"%s\")", path);

	_file = Common::FSNode(path).createReadStream();

	if (!_file) {
		warning("elfloader: File %s not found.", path);
		return false;
	}

	debug(2, "elfloader: %s found!", path);

	/*Try to load and relocate*/
	if (!load()) {
		unload();
		return false;
	}

	debug(2, "elfloader: Loaded!");

	delete _file;
	_file = 0;

	flushDataCache(_segment, _segmentSize);

	ctors_start = symbol("___plugin_ctors");
	ctors_end = symbol("___plugin_ctors_end");
	_dtors_start = symbol("___plugin_dtors");
	_dtors_end = symbol("___plugin_dtors_end");

	if (!ctors_start || !ctors_end || !_dtors_start || !_dtors_end) {
		warning("elfloader: Missing ctors/dtors.");
		_dtors_start = _dtors_end = 0;
		unload();
		return false;
	}

	debug(2, "elfloader: Calling constructors.");
	for (void (**f)(void) = (void (**)(void))ctors_start; f != ctors_end; f++)
		(**f)();

	debug(2, "elfloader: %s opened ok.", path);

	return true;
}

bool DLObject::close() {
	if (_dtors_start && _dtors_end)
		for (void (**f)(void) = (void (**)(void))_dtors_start; f != _dtors_end; f++)
			(**f)();

	_dtors_start = _dtors_end = 0;
	unload();
	return true;
}

void *DLObject::symbol(const char *name) {
	debug(2, "elfloader: Symbol(\"%s\")", name);

	if (!_symtab || !_strtab || _symbol_cnt < 1) {
		warning("elfloader: No symbol table loaded.");
		return 0;
	}

	Elf32_Sym *s = _symtab;

	for (uint32 c = _symbol_cnt; c--; s++)
		// We can only import symbols that are global or weak in the plugin
		if ((SYM_BIND(s->st_info) == STB_GLOBAL ||
				SYM_BIND(s->st_info) == STB_WEAK) &&
				!strcmp(name, _strtab + s->st_name)) {
			// We found the symbol
			debug(2, "elfloader: => 0x%08x", s->st_value);
			return (void *)s->st_value;
		}

	// We didn't find the symbol
	warning("elfloader: Symbol \"%s\" not found.", name);
	return 0;
}

#endif /* defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) */
