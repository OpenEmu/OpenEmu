/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dbg_decoder.h                                           *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2010 Marshall B. Rogers <mbr@64.vg>                     *
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

#ifndef __DECODER_H__
#define __DECODER_H__

#include "dbg_types.h"

#if defined(WIN32)
typedef unsigned int uint32_t;
typedef unsigned char bool;
#define false 0
#define true 1
#else
#include <stdint.h>
#include <stdbool.h>
#endif

/* Disassembler lookup handler */
typedef char * (*r4k_lookup_func)(uint32_t, void *);

/* Disassembler state */
typedef
struct r4k_dis_t
{
    r4k_lookup_func  lookup_sym;
    void *           lookup_sym_d;
    r4k_lookup_func  lookup_rel_hi16;
    void *           lookup_rel_hi16_d;   
    r4k_lookup_func  lookup_rel_lo16;
    void *           lookup_rel_lo16_d;
    
    /* Private */
    char * dest;
    int length;
}
R4kDis;

extern void r4300_decode_op ( uint32, char *, char *, int );


#endif /* __DECODER_H__ */

