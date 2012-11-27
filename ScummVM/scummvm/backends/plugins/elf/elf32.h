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

#ifndef BACKENDS_ELF_H
#define BACKENDS_ELF_H

#include "common/scummsys.h"

#if defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER)

/**
 *  ELF stuff:
 *  The contents of this file were gathered mainly from the SYSTEM V APPLICATION BINARY INTERFACE.
 *  Processor-specific things were garnered from processor-specific supplements to the abi.
 */

typedef uint16 Elf32_Half, Elf32_Section;
typedef uint32 Elf32_Word, Elf32_Addr, Elf32_Off;
typedef int32 Elf32_Sword;
typedef Elf32_Half Elf32_Versym;

#define EI_NIDENT (16)
#define SELFMAG   4

/* ELF File format structures. Look up ELF structure for more details */

// ELF header (contains info about the file)
typedef struct {
	byte		e_ident[EI_NIDENT];	/* Magic number and other info */
	Elf32_Half	e_type;				/* Object file type */
	Elf32_Half	e_machine;			/* Architecture */
	Elf32_Word	e_version;			/* Object file version */
	Elf32_Addr	e_entry;				/* Entry point virtual address */
	Elf32_Off	e_phoff;				/* Program header table file offset */
	Elf32_Off	e_shoff;				/* Section header table file offset */
	Elf32_Word	e_flags;				/* Processor-specific flags */
	Elf32_Half	e_ehsize;				/* ELF header size in bytes */
	Elf32_Half	e_phentsize;			/* Program header table entry size */
	Elf32_Half	e_phnum;				/* Program header table entry count */
	Elf32_Half	e_shentsize;			/* Section header table entry size */
	Elf32_Half	e_shnum;				/* Section header table entry count */
	Elf32_Half	e_shstrndx;			/* Section header string table index */
} Elf32_Ehdr;

// Should be in e_ident
#define ELFMAG		"\177ELF"	/* ELF Magic number */

#define EI_CLASS	4			/* File class byte index */
#define ELFCLASS32	1			/* 32-bit objects */

#define EI_DATA		5			/* Data encoding byte index */
#define ELFDATA2LSB	1			/* 2's complement, little endian */
#define ELFDATA2MSB	2			/* 2's complement, big endian */

#define EI_VERSION	6
#define EV_CURRENT	1			/* Current version */

// e_type values
#define ET_NONE		0			/* no file type */
#define ET_REL		1			/* relocatable */
#define ET_EXEC		2			/* executable */
#define ET_DYN		3			/* shared object */
#define ET_CORE		4			/* core file */

// e_machine values
#define EM_MIPS		8
#define EM_PPC		20
#define EM_ARM		40
#define EM_SH		42

// Program header (contains info about segment)
typedef struct {
	Elf32_Word	p_type;		/* Segment type */
	Elf32_Off	p_offset;	/* Segment file offset */
	Elf32_Addr	p_vaddr;	/* Segment virtual address */
	Elf32_Addr	p_paddr;	/* Segment physical address */
	Elf32_Word	p_filesz;	/* Segment size in file */
	Elf32_Word	p_memsz;	/* Segment size in memory */
	Elf32_Word	p_flags;	/* Segment flags */
	Elf32_Word	p_align;	/* Segment alignment */
} Elf32_Phdr;

// p_type values
#define PT_NULL 		0			/* ignored */
#define PT_LOAD			1			/* loadable segment */
#define PT_DYNAMIC		2			/* dynamic linking info */
#define PT_INTERP		3			/* info about interpreter */
#define PT_NOTE			4			/* note segment */
#define PT_SHLIB		5			/* reserved */
#define PT_PHDR			6			/* Program header table */
#define PT_MIPS_REGINFO	0x70000000	/* Register usage info for MIPS */
#define PT_ARM_ARCHEXT	0x70000000	/* Platform architecture compatibility info for ARM */
#define PT_ARM_EXIDX	0x70000001	/* Exception unwind tables for ARM */

// p_flags value
#define PF_X	1	/* execute */
#define PF_W	2	/* write */
#define PF_R	4	/* read */

// Section header (contains info about section)
typedef struct {
	Elf32_Word	sh_name;		/* Section name (string tbl index) */
	Elf32_Word	sh_type;		/* Section type */
	Elf32_Word	sh_flags;		/* Section flags */
	Elf32_Addr	sh_addr;		/* Section virtual addr at execution */
	Elf32_Off	sh_offset;		/* Section file offset */
	Elf32_Word	sh_size;		/* Section size in bytes */
	Elf32_Word	sh_link;		/* Link to another section */
	Elf32_Word	sh_info;		/* Additional section information */
	Elf32_Word	sh_addralign;	/* Section alignment */
	Elf32_Word	sh_entsize;		/* Entry size if section holds table */
} Elf32_Shdr;

// sh_type values
#define SHT_NULL			0			/* Inactive section */
#define SHT_PROGBITS		1			/* Proprietary */
#define SHT_SYMTAB			2			/* Symbol table */
#define SHT_STRTAB			3			/* String table */
#define SHT_RELA			4			/* Relocation entries with addend */
#define SHT_HASH			5			/* Symbol hash table */
#define SHT_DYNAMIC			6			/* Info for dynamic linking */
#define SHT_NOTE			7			/* Note section */
#define SHT_NOBITS			8			/* Occupies no space */
#define SHT_REL				9			/* Relocation entries without addend */
#define SHT_SHLIB			10			/* Reserved */
#define SHT_DYNSYM			11			/* Minimal set of dynamic linking symbols */
#define SHT_MIPS_LIBLSIT	0x70000000	/* Info about dynamic shared object libs for MIPS*/
#define SHT_MIPS_CONFLICT	0x70000002	/* Conflicts btw executables and shared objects for MIPS */
#define SHT_MIPS_GPTAB		0x70000003	/* Global pointer table for MIPS*/
#define SHT_ARM_EXIDX		0x70000001	/* Exception Index table for ARM*/
#define SHT_ARM_PREEMPTMAP	0x70000002	/* BPABI DLL dynamic linking pre-emption map for ARM */
#define SHT_ARM_ATTRIBUTES	0x70000003	/* Object file compatibility attributes for ARM*/

// sh_flags values
#define SHF_WRITE		0			/* writable section */
#define SHF_ALLOC		2			/* section occupies memory */
#define SHF_EXECINSTR	4			/* machine instructions */
#define SHF_MIPS_GPREL	0x10000000	/* Must be made part of global data area for MIPS */

// Symbol entry (contain info about a symbol)
typedef struct {
	Elf32_Word		st_name;	/* Symbol name (string tbl index) */
	Elf32_Addr		st_value;	/* Symbol value */
	Elf32_Word		st_size;	/* Symbol size */
	byte			st_info;	/* Symbol type and binding */
	byte			st_other;	/* Symbol visibility */
	Elf32_Section	st_shndx;	/* Section index */
} Elf32_Sym;

// Extract from the st_info
#define SYM_TYPE(x)		((x) & 0xf)
#define SYM_BIND(x)		((x) >> 4)

// Symbol binding values from st_info
#define STB_LOCAL	0	/* Symbol not visible outside object */
#define STB_GLOBAL	1	/* Symbol visible to all object files */
#define STB_WEAK	2	/* Similar to STB_GLOBAL */

// Symbol type values from st_info
#define STT_NOTYPE	0	/* Not specified */
#define STT_OBJECT	1	/* Data object e.g. variable */
#define STT_FUNC	2	/* Function */
#define STT_SECTION	3	/* Section */
#define STT_FILE	4	/* Source file associated with object file */

// Special section header index values from st_shndex
#define SHN_UNDEF		0
#define SHN_LOPROC		0xFF00	/* Extended values */
#define SHN_ABS			0xFFF1	/* Absolute value: don't relocate */
#define SHN_COMMON		0xFFF2	/* Common block. Not allocated yet */
#define SHN_HIPROC		0xFF1F
#define SHN_HIRESERVE	0xFFFF

// Relocation entry with implicit addend (info about how to relocate)
typedef struct {
	Elf32_Addr	r_offset;	/* Address */
	Elf32_Word	r_info;		/* Relocation type and symbol index */
} Elf32_Rel;

// Relocation entry with explicit addend (info about how to relocate)
typedef struct {
	Elf32_Addr	r_offset;	/* Address */
	Elf32_Word	r_info;		/* Relocation type and symbol index */
	Elf32_Sword	r_addend;	/* Addend */
} Elf32_Rela;

// Access macros for the relocation info
#define REL_TYPE(x)		((byte) (x))	/* Extract relocation type */
#define REL_INDEX(x)	((x)>>8)		/* Extract relocation index into symbol table */

//MIPS relocation types
#define R_MIPS_NONE		0
#define R_MIPS_16		1
#define R_MIPS_32		2
#define R_MIPS_REL32	3
#define R_MIPS_26		4
#define R_MIPS_HI16		5
#define R_MIPS_LO16		6
#define R_MIPS_GPREL16	7
#define R_MIPS_LITERAL	8
#define R_MIPS_GOT16	9
#define R_MIPS_PC16		10
#define R_MIPS_CALL16	11
#define R_MIPS_GPREL32	12
#define R_MIPS_GOTHI16	13
#define R_MIPS_GOTLO16	14
#define R_MIPS_CALLHI16	15
#define R_MIPS_CALLLO16	16

// ARM relocation types
#define R_ARM_NONE			0
#define R_ARM_ABS32			2
#define R_ARM_THM_CALL		10
#define R_ARM_CALL			28
#define R_ARM_JUMP24		29
#define R_ARM_TARGET1		38
#define R_ARM_V4BX 			40

// PPC relocation types
#define R_PPC_ADDR32		1
#define R_PPC_ADDR16_LO		4
#define R_PPC_ADDR16_HI		5
#define R_PPC_ADDR16_HA		6
#define R_PPC_REL24			10
#define R_PPC_REL32			26

#endif // defined(DYNAMIC_MODULES) && defined(USE_ELF_LOADER)

#endif /* BACKENDS_ELF_H */
