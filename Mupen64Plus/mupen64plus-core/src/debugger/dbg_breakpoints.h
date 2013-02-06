/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dbg_breakpoints.h                                       *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 DarkJeztr HyperHacker                              *
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

#ifndef __BREAKPOINTS_H__
#define __BREAKPOINTS_H__

#include "../api/m64p_types.h"

extern int g_NumBreakpoints;
extern breakpoint g_Breakpoints[];

int add_breakpoint( uint32 address );
int add_breakpoint_struct(breakpoint* newbp);
void remove_breakpoint_by_address( uint32 address );
void remove_breakpoint_by_num( int bpt );
void enable_breakpoint( int breakpoint );
void disable_breakpoint( int breakpoint );
int check_breakpoints( uint32 address );
int check_breakpoints_on_mem_access( uint32 pc, uint32 address, uint32 size, uint32 flags );
int lookup_breakpoint( uint32 address, uint32 size, uint32 flags );
int log_breakpoint(uint32 PC, uint32 Flag, uint32 Access);
void replace_breakpoint_num( int, breakpoint* );

#endif  /* __BREAKPOINTS_H__ */

