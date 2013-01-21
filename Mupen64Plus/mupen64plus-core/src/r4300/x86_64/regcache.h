/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - regcache.c                                              *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2007 Richard Goedeken (Richard42)                       *
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

#ifndef __REGCACHE_H__
#define __REGCACHE_H__

#include "r4300/recomp.h"

void init_cache(precomp_instr* start);
void free_registers_move_start(void);
void free_all_registers(void);
void free_register(int reg);
int is64(unsigned int *addr);
int lru_register(void);
int lru_base_register(void);
void set_register_state(int reg, unsigned int *addr, int dirty, int is64bits);
int lock_register(int reg);
void unlock_register(int reg);
int allocate_register_32(unsigned int *addr);
int allocate_register_64(unsigned long long *addr);
int allocate_register_32_w(unsigned int *addr);
int allocate_register_64_w(unsigned long long *addr);
void allocate_register_32_manually(int reg, unsigned int *addr);
void allocate_register_32_manually_w(int reg, unsigned int *addr);
void build_wrappers(precomp_instr*, int, int, precomp_block*);

#endif /* __REGCACHE_H__ */

