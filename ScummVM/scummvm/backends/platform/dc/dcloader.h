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

#ifndef DC_DCLOADER_H
#define DC_DCLOADER_H

#include "dc.h"

#define MAXDLERRLEN 80

class DLObject {
 private:
  char *errbuf; /* For error messages, at least MAXDLERRLEN in size */

  void *segment, *symtab;
  char *strtab;
  int symbol_cnt;
  void *dtors_start, *dtors_end;

  void seterror(const char *fmt, ...);
  void unload();
  bool relocate(int fd, unsigned long offset, unsigned long size);
  bool load(int fd);

 public:
  bool open(const char *path);
  bool close();
  void *symbol(const char *name);
  void discard_symtab();

  DLObject(char *_errbuf = NULL) : errbuf(_errbuf), segment(NULL),symtab(NULL),
    strtab(NULL), symbol_cnt(0), dtors_start(NULL), dtors_end(NULL) {}
};

#define RTLD_LAZY 0

extern "C" {
  void *dlopen(const char *filename, int flags);
  int dlclose(void *handle);
  void *dlsym(void *handle, const char *symbol);
  const char *dlerror();
  void dlforgetsyms(void *handle);
}

#endif
