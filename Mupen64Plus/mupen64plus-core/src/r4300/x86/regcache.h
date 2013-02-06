/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - regcache.h                                              *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef REGCACHE_H
#define REGCACHE_H

#include "r4300/recomp.h"

void init_cache(precomp_instr* start);
void free_all_registers(void);
void free_register(int reg);
int allocate_register(unsigned int *addr);
int allocate_64_register1(unsigned int *addr);
int allocate_64_register2(unsigned int *addr);
int is64(unsigned int *addr);
void build_wrappers(precomp_instr*, int, int, precomp_block*);
int lru_register(void);
int allocate_register_w(unsigned int *addr);
int allocate_64_register1_w(unsigned int *addr);
int allocate_64_register2_w(unsigned int *addr);
void set_register_state(int reg, unsigned int *addr, int dirty);
void set_64_register_state(int reg1, int reg2, unsigned int *addr, int dirty);
void allocate_register_manually(int reg, unsigned int *addr);
void allocate_register_manually_w(int reg, unsigned int *addr, int load);
void force_32(int reg);
int lru_register_exc1(int exc1);
void simplify_access(void);

#endif // REGCACHE_H

