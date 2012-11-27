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

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(ARM_TARGET)

#include "backends/plugins/elf/arm-loader.h"

#include "common/debug.h"

bool ARMDLObject::relocate(Elf32_Off offset, Elf32_Word size, byte *relSegment) {
	Elf32_Rel *rel = 0; //relocation entry

	// Allocate memory for relocation table
	if (!(rel = (Elf32_Rel *)malloc(size))) {
		warning("elfloader: Out of memory.");
		return false;
	}

	// Read in our relocation table
	if (!_file->seek(offset, SEEK_SET) || _file->read(rel, size) != size) {
		warning("elfloader: Relocation table load failed.");
		free(rel);
		return false;
	}

	// Treat each relocation entry. Loop over all of them
	uint32 cnt = size / sizeof(*rel);

	debug(2, "elfloader: Loaded relocation table. %d entries. base address=%p", cnt, relSegment);

	int32 a = 0;
	uint32 relocation = 0;

	// Loop over relocation entries
	for (uint32 i = 0; i < cnt; i++) {
		// Get the symbol this relocation entry is referring to
		Elf32_Sym *sym = _symtab + (REL_INDEX(rel[i].r_info));

		// Get the target instruction in the code. TODO: repect _segmentVMA
		uint32 *target = (uint32 *)((byte *)relSegment + rel[i].r_offset);

		uint32 origTarget = *target;	//Save for debugging

		// Act differently based on the type of relocation
		switch (REL_TYPE(rel[i].r_info)) {
		case R_ARM_ABS32:
			if (sym->st_shndx < SHN_LOPROC) {			// Only shift for plugin section.
				a = *target;							// Get full 32 bits of addend
				relocation = a + Elf32_Addr(_segment);	// Shift by main offset

				*target = relocation;

				debug(8, "elfloader: R_ARM_ABS32: i=%d, a=%x, origTarget=%x, target=%x", i, a, origTarget, *target);
			}
			break;

		case R_ARM_THM_CALL:
			debug(8, "elfloader: R_ARM_THM_CALL: PC-relative jump, ld takes care of necessary relocation work for us.");
			break;

		case R_ARM_CALL:
			debug(8, "elfloader: R_ARM_CALL: PC-relative jump, ld takes care of necessary relocation work for us.");
			break;

		case R_ARM_JUMP24:
			debug(8, "elfloader: R_ARM_JUMP24: PC-relative jump, ld takes care of all relocation work for us.");
			break;

		case R_ARM_V4BX:
			debug(8, "elfloader: R_ARM_V4BX: No relocation calculation necessary.");
			break;

		default:
			warning("elfloader: Unknown relocation type %d.", REL_TYPE(rel[i].r_info));
			free(rel);
			return false;
		}
	}

	free(rel);
	return true;
}

bool ARMDLObject::relocateRels(Elf32_Ehdr *ehdr, Elf32_Shdr *shdr) {
	// Loop over sections, finding relocation sections
	for (uint32 i = 0; i < ehdr->e_shnum; i++) {
		Elf32_Shdr *curShdr = &(shdr[i]);

		if ((curShdr->sh_type == SHT_REL || curShdr->sh_type == SHT_RELA) &&		// Check for a relocation section
				curShdr->sh_entsize == sizeof(Elf32_Rel) &&			// Check for proper relocation size
				int32(curShdr->sh_link) == _symtab_sect &&			// Check that the sh_link connects to our symbol table
				curShdr->sh_info < ehdr->e_shnum &&					// Check that the relocated section exists
				(shdr[curShdr->sh_info].sh_flags & SHF_ALLOC)) {	// Check if relocated section resides in memory

			if (curShdr->sh_type == SHT_RELA) {
				warning("elfloader: RELA entries not supported yet!");
				return false;
			}

			if (!relocate(curShdr->sh_offset, curShdr->sh_size, _segment))
				return false;
		}
	}

	return true;
}

#endif /* defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER) && defined(ARM_TARGET) */
