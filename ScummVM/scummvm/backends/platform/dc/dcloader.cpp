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

#include <ronin/ronin.h>
#include <string.h>
#include <stdarg.h>

#include "dcloader.h"

#ifdef DL_DEBUG
#define DBG(x) reportf x
#else
#define DBG(x) do{}while(0)
#endif


/* ELF stuff */

typedef unsigned short Elf32_Half, Elf32_Section;
typedef unsigned long Elf32_Word, Elf32_Addr, Elf32_Off;
typedef signed long  Elf32_Sword;
typedef Elf32_Half Elf32_Versym;

#define EI_NIDENT (16)
#define ELFMAG          "\177ELF\1\1"
#define SELFMAG         6

typedef struct
{
  unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
  Elf32_Half    e_type;                 /* Object file type */
  Elf32_Half    e_machine;              /* Architecture */
  Elf32_Word    e_version;              /* Object file version */
  Elf32_Addr    e_entry;                /* Entry point virtual address */
  Elf32_Off     e_phoff;                /* Program header table file offset */
  Elf32_Off     e_shoff;                /* Section header table file offset */
  Elf32_Word    e_flags;                /* Processor-specific flags */
  Elf32_Half    e_ehsize;               /* ELF header size in bytes */
  Elf32_Half    e_phentsize;            /* Program header table entry size */
  Elf32_Half    e_phnum;                /* Program header table entry count */
  Elf32_Half    e_shentsize;            /* Section header table entry size */
  Elf32_Half    e_shnum;                /* Section header table entry count */
  Elf32_Half    e_shstrndx;             /* Section header string table index */
} Elf32_Ehdr;

typedef struct
{
  Elf32_Word    p_type;                 /* Segment type */
  Elf32_Off     p_offset;               /* Segment file offset */
  Elf32_Addr    p_vaddr;                /* Segment virtual address */
  Elf32_Addr    p_paddr;                /* Segment physical address */
  Elf32_Word    p_filesz;               /* Segment size in file */
  Elf32_Word    p_memsz;                /* Segment size in memory */
  Elf32_Word    p_flags;                /* Segment flags */
  Elf32_Word    p_align;                /* Segment alignment */
} Elf32_Phdr;

typedef struct
{
  Elf32_Word    sh_name;                /* Section name (string tbl index) */
  Elf32_Word    sh_type;                /* Section type */
  Elf32_Word    sh_flags;               /* Section flags */
  Elf32_Addr    sh_addr;                /* Section virtual addr at execution */
  Elf32_Off     sh_offset;              /* Section file offset */
  Elf32_Word    sh_size;                /* Section size in bytes */
  Elf32_Word    sh_link;                /* Link to another section */
  Elf32_Word    sh_info;                /* Additional section information */
  Elf32_Word    sh_addralign;           /* Section alignment */
  Elf32_Word    sh_entsize;             /* Entry size if section holds table */
} Elf32_Shdr;

typedef struct
{
  Elf32_Word    st_name;                /* Symbol name (string tbl index) */
  Elf32_Addr    st_value;               /* Symbol value */
  Elf32_Word    st_size;                /* Symbol size */
  unsigned char st_info;                /* Symbol type and binding */
  unsigned char st_other;               /* Symbol visibility */
  Elf32_Section st_shndx;               /* Section index */
} Elf32_Sym;

typedef struct
{
  Elf32_Addr    r_offset;               /* Address */
  Elf32_Word    r_info;                 /* Relocation type and symbol index */
  Elf32_Sword   r_addend;               /* Addend */
} Elf32_Rela;



extern "C" void flush_instruction_cache();

static void purge_copyback()
{
  int i;
  for (i=0; i!=(1<<14); i+=(1<<5))
    *(volatile unsigned int *)(0xf4000000+i) &= ~3;
}


void DLObject::seterror(const char *fmt, ...)
{
  if (errbuf) {
    va_list va;
    va_start(va, fmt);
    vsnprintf(errbuf, MAXDLERRLEN, fmt, va);
    va_end(va);
  }
}

void DLObject::discard_symtab()
{
  free(symtab);
  free(strtab);
  symtab = NULL;
  strtab = NULL;
  symbol_cnt = 0;
}

void DLObject::unload()
{
  discard_symtab();
  free(segment);
  segment = NULL;
}

bool DLObject::relocate(int fd, unsigned long offset, unsigned long size)
{
  Elf32_Rela *rela;

  if (!(rela = (Elf32_Rela *)malloc(size))) {
    seterror("Out of memory.");
    return false;
  }

  if (lseek(fd, offset, SEEK_SET)<0 ||
      read(fd, rela, size) != (ssize_t)size) {
    seterror("Relocation table load failed.");
    free(rela);
    return false;
  }

  int cnt = size / sizeof(*rela);
  for (int i=0; i<cnt; i++) {

    Elf32_Sym *sym = (Elf32_Sym *)(void *)(((char *)symtab)+(rela[i].r_info>>4));

    void *target = ((char *)segment)+rela[i].r_offset;

    switch(rela[i].r_info & 0xf) {
    case 1: /* DIR32 */
      if (sym->st_shndx < 0xff00)
	*(unsigned long *)target += (unsigned long)segment;
      break;
    default:
      seterror("Unknown relocation type %d.", rela[i].r_info & 0xf);
      free(rela);
      return false;
    }

  }

  free(rela);
  return true;
}


bool DLObject::load(int fd)
{
  Elf32_Ehdr ehdr;
  Elf32_Phdr phdr;
  Elf32_Shdr *shdr;
  int symtab_sect = -1;

  if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr) ||
     memcmp(ehdr.e_ident, ELFMAG, SELFMAG) ||
     ehdr.e_type != 2 ||  ehdr.e_machine != 42 ||
     ehdr.e_phentsize < sizeof(phdr) || ehdr.e_shentsize != sizeof(*shdr) ||
     ehdr.e_phnum != 1) {
    seterror("Invalid file type.");
    return false;
  }

  DBG(("phoff = %d, phentsz = %d, phnum = %d\n",
       ehdr.e_phoff, ehdr.e_phentsize, ehdr.e_phnum));

  if (lseek(fd, ehdr.e_phoff, SEEK_SET)<0 ||
     read(fd, &phdr, sizeof(phdr)) != sizeof(phdr)) {
    seterror("Program header load failed.");
    return false;
  }

  if (phdr.p_type != 1 || phdr.p_vaddr != 0 || phdr.p_paddr != 0 ||
     phdr.p_filesz > phdr.p_memsz) {
    seterror("Invalid program header.");
    return false;
  }

  DBG(("offs = %d, filesz = %d, memsz = %d, align = %d\n",
       phdr.p_offset, phdr.p_filesz, phdr.p_memsz, phdr.p_align));

  if (!(segment = memalign(phdr.p_align, phdr.p_memsz))) {
    seterror("Out of memory.");
    return false;
  }

  DBG(("segment @ %p\n", segment));

  if (phdr.p_memsz > phdr.p_filesz)
    memset(((char *)segment) + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);

  if (lseek(fd, phdr.p_offset, SEEK_SET)<0 ||
      read(fd, segment, phdr.p_filesz) != (ssize_t)phdr.p_filesz) {
    seterror("Segment load failed.");
    return false;
  }

  DBG(("shoff = %d, shentsz = %d, shnum = %d\n",
       ehdr.e_shoff, ehdr.e_shentsize, ehdr.e_shnum));

  if (!(shdr = (Elf32_Shdr *)malloc(ehdr.e_shnum * sizeof(*shdr)))) {
    seterror("Out of memory.");
    return false;
  }

  if (lseek(fd, ehdr.e_shoff, SEEK_SET)<0 ||
      read(fd, shdr, ehdr.e_shnum * sizeof(*shdr)) !=
      (ssize_t)(ehdr.e_shnum * sizeof(*shdr))) {
    seterror("Section headers load failed.");
    free(shdr);
    return false;
  }

  for (int i=0; i<ehdr.e_shnum; i++) {
    DBG(("Section %d: type = %d, size = %d, entsize = %d, link = %d\n",
	 i, shdr[i].sh_type, shdr[i].sh_size, shdr[i].sh_entsize, shdr[i].sh_link));
    if (shdr[i].sh_type == 2 && shdr[i].sh_entsize == sizeof(Elf32_Sym) &&
       shdr[i].sh_link < ehdr.e_shnum && shdr[shdr[i].sh_link].sh_type == 3 &&
       symtab_sect < 0)
      symtab_sect = i;
  }

  if (symtab_sect < 0) {
    seterror("No symbol table.");
    free(shdr);
    return false;
  }

  if (!(symtab = malloc(shdr[symtab_sect].sh_size))) {
    seterror("Out of memory.");
    free(shdr);
    return false;
  }

  if (lseek(fd, shdr[symtab_sect].sh_offset, SEEK_SET)<0 ||
      read(fd, symtab, shdr[symtab_sect].sh_size) !=
      (ssize_t)shdr[symtab_sect].sh_size){
    seterror("Symbol table load failed.");
    free(shdr);
    return false;
  }

  if (!(strtab = (char *)malloc(shdr[shdr[symtab_sect].sh_link].sh_size))) {
    seterror("Out of memory.");
    free(shdr);
    return false;
  }

  if (lseek(fd, shdr[shdr[symtab_sect].sh_link].sh_offset, SEEK_SET)<0 ||
      read(fd, strtab, shdr[shdr[symtab_sect].sh_link].sh_size) !=
      (ssize_t)shdr[shdr[symtab_sect].sh_link].sh_size){
    seterror("Symbol table strings load failed.");
    free(shdr);
    return false;
  }

  symbol_cnt = shdr[symtab_sect].sh_size / sizeof(Elf32_Sym);
  DBG(("Loaded %d symbols.\n", symbol_cnt));

  Elf32_Sym *s = (Elf32_Sym *)symtab;
  for (int c = symbol_cnt; c--; s++)
    if (s->st_shndx < 0xff00)
      s->st_value += (Elf32_Addr)segment;

  for (int i=0; i<ehdr.e_shnum; i++)
    if (shdr[i].sh_type == 4 && shdr[i].sh_entsize == sizeof(Elf32_Rela) &&
	(int)shdr[i].sh_link == symtab_sect && shdr[i].sh_info < ehdr.e_shnum &&
	(shdr[shdr[i].sh_info].sh_flags & 2))
      if (!relocate(fd, shdr[i].sh_offset, shdr[i].sh_size)) {
	free(shdr);
	return false;
      }

  free(shdr);

  return true;
}

bool DLObject::open(const char *path)
{
  int fd;
  void *ctors_start, *ctors_end;

  DBG(("open(\"%s\")\n", path));

  if ((fd = ::open(path, O_RDONLY))<0) {
    seterror("%s not found.", path);
    return false;
  }

  if (!load(fd)) {
    ::close(fd);
    unload();
    return false;
  }

  ::close(fd);

  int oldmask = getimask();
  setimask(15);
  purge_copyback();
  flush_instruction_cache();
  setimask(oldmask);

  ctors_start = symbol("__plugin_ctors");
  ctors_end = symbol("__plugin_ctors_end");
  dtors_start = symbol("__plugin_dtors");
  dtors_end = symbol("__plugin_dtors_end");

  if (ctors_start == NULL || ctors_end == NULL || dtors_start == NULL ||
     dtors_end == NULL) {
    seterror("Missing ctors/dtors.");
    dtors_start = dtors_end = NULL;
    unload();
    return false;
  }

  DBG(("Calling constructors.\n"));
  for (void (**f)(void) = (void (**)(void))ctors_start; f != ctors_end; f++)
    (**f)();

  DBG(("%s opened ok.\n", path));
  return true;
}

bool DLObject::close()
{
  if (dtors_start != NULL && dtors_end != NULL)
    for (void (**f)(void) = (void (**)(void))dtors_start; f != dtors_end; f++)
      (**f)();
  dtors_start = dtors_end = NULL;
  unload();
  return true;
}

void *DLObject::symbol(const char *name)
{
  DBG(("symbol(\"%s\")\n", name));

  if (symtab == NULL || strtab == NULL || symbol_cnt < 1) {
    seterror("No symbol table loaded.");
    return NULL;
  }

  Elf32_Sym *s = (Elf32_Sym *)symtab;
  for (int c = symbol_cnt; c--; s++)
    if ((s->st_info>>4 == 1 || s->st_info>>4 == 2) &&
       strtab[s->st_name] == '_' && !strcmp(name, strtab+s->st_name+1)) {
      DBG(("=> %p\n", (void *)s->st_value));
      return (void *)s->st_value;
    }

  seterror("Symbol \"%s\" not found.", name);
  return NULL;
}


static char dlerr[MAXDLERRLEN];

void *dlopen(const char *filename, int flags)
{
  DLObject *obj = new DLObject(dlerr);
  if (obj->open(filename))
    return (void *)obj;
  delete obj;
  return NULL;
}

int dlclose(void *handle)
{
  DLObject *obj = (DLObject *)handle;
  if (obj == NULL) {
    strcpy(dlerr, "Handle is NULL.");
    return -1;
  }
  if (obj->close()) {
    delete obj;
    return 0;
  }
  return -1;
}

void *dlsym(void *handle, const char *symbol)
{
  if (handle == NULL) {
    strcpy(dlerr, "Handle is NULL.");
    return NULL;
  }
  return ((DLObject *)handle)->symbol(symbol);
}

const char *dlerror()
{
  return dlerr;
}

void dlforgetsyms(void *handle)
{
  if (handle != NULL)
    ((DLObject *)handle)->discard_symtab();
}
