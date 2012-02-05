/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - macros.h                                                *
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

#ifndef MACROS_H
#define MACROS_H

#define sign_extended(a) a = (long long)((int)a)
#define sign_extendedb(a) a = (long long)((char)a)
#define sign_extendedh(a) a = (long long)((short)a)

#define rrt *PC->f.r.rt
#define rrd *PC->f.r.rd
#define rfs PC->f.r.nrd
#define rrs *PC->f.r.rs
#define rsa PC->f.r.sa
#define irt *PC->f.i.rt
#define ioffset PC->f.i.immediate
#define iimmediate PC->f.i.immediate
#define lsaddr ((PC-1)->f.i.immediate+(*(PC-1)->f.i.rs))
#define lsrt *(PC-1)->f.i.rt
#define irs *PC->f.i.rs
#define ibase *PC->f.i.rs
#define jinst_index PC->f.j.inst_index
#define lfbase PC->f.lf.base
#define lfft PC->f.lf.ft
#define lfoffset PC->f.lf.offset
#define lslfaddr ((PC-1)->f.lf.offset+reg[(PC-1)->f.lf.base])
#define lslfft (PC-1)->f.lf.ft
#define cfft PC->f.cf.ft
#define cffs PC->f.cf.fs
#define cffd PC->f.cf.fd

// 32 bits macros
#ifndef M64P_BIG_ENDIAN
#define rrt32 *((int*)PC->f.r.rt)
#define rrd32 *((int*)PC->f.r.rd)
#define rrs32 *((int*)PC->f.r.rs)
#define irs32 *((int*)PC->f.i.rs)
#define irt32 *((int*)PC->f.i.rt)
#else
#define rrt32 *((int*)PC->f.r.rt+1)
#define rrd32 *((int*)PC->f.r.rd+1)
#define rrs32 *((int*)PC->f.r.rs+1)
#define irs32 *((int*)PC->f.i.rs+1)
#define irt32 *((int*)PC->f.i.rt+1)
#endif

//cop0 macros
#define Index reg_cop0[0]
#define Random reg_cop0[1]
#define EntryLo0 reg_cop0[2]
#define EntryLo1 reg_cop0[3]
#define Context reg_cop0[4]
#define PageMask reg_cop0[5]
#define Wired reg_cop0[6]
#define BadVAddr reg_cop0[8]
#define Count reg_cop0[9]
#define EntryHi reg_cop0[10]
#define Compare reg_cop0[11]
#define Status reg_cop0[12]
#define Cause reg_cop0[13]
#define EPC reg_cop0[14]
#define PRevID reg_cop0[15]
#define Config reg_cop0[16]
#define LLAddr reg_cop0[17]
#define WatchLo reg_cop0[18]
#define WatchHi reg_cop0[19]
#define XContext reg_cop0[20]
#define PErr reg_cop0[26]
#define CacheErr reg_cop0[27]
#define TagLo reg_cop0[28]
#define TagHi reg_cop0[29]
#define ErrorEPC reg_cop0[30]

#endif

