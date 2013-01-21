/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus -- dbg_decoder.c                                          *
 *   Copyright (c) 2010  Marshall B. Rogers <mbr@64.vg>                    *
 *   http://64.vg/                                                         *
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

/*
 *  This is a heavily modified reentrant version of the MIPS disassembler found
 *  in the NetBSD operating system. I chose to use this as a base due to the 
 *  small, compact, and easily manageable code.
 *  
 *  Original copyright/license information is contained below.
 */

/*	$NetBSD: db_disasm.c,v 1.21 2009/12/14 00:46:06 matt Exp $	*/

/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: @(#)kadb.c	8.1 (Berkeley) 6/10/93
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#ifndef MIPS32
  #define MIPS32
#endif
#include "dbg_decoder.h"
#include "dbg_decoder_local.h"
#include "osal/preproc.h"


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Data types
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

typedef uint32_t db_addr_t;



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Local variables
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

static const char * const r4k_str_op_name[64] = 
{
/* 0 */ "spec", "bcond","j",	"jal",	"beq",	"bne",	"blez", "bgtz",
/* 8 */ "addi", "addiu","slti", "sltiu","andi", "ori",	"xori", "lui",
/*16 */ "cop0", "cop1", "cop2", "cop3", "beql", "bnel", "blezl","bgtzl",
/*24 */ "daddi","daddiu","ldl", "ldr",	"op34", "op35", "op36", "op37",
/*32 */ "lb",	"lh",	"lwl",	"lw",	"lbu",	"lhu",	"lwr",	"lwu",
/*40 */ "sb",	"sh",	"swl",	"sw",	"sdl",	"sdr",	"swr",	"cache",
/*48 */ "ll",	"lwc1", "lwc2", "lwc3", "lld",	"ldc1", "ldc2", "ld",
/*56 */ "sc",	"swc1", "swc2", "swc3", "scd",	"sdc1", "sdc2", "sd"
};

static const char * const r4k_str_spec_name[64] = 
{
/* 0 */ "sll",	"spec01","srl", "sra",	"sllv", "spec05","srlv","srav",
/* 8 */ "jr",	"jalr", "spec12","spec13","syscall","break","spec16","sync",
/*16 */ "mfhi", "mthi", "mflo", "mtlo", "dsllv","spec25","dsrlv","dsrav",
/*24 */ "mult", "multu","div",	"divu", "dmult","dmultu","ddiv","ddivu",
/*32 */ "add",	"addu", "sub",	"subu", "and",	"or",	"xor",	"nor",
/*40 */ "spec50","spec51","slt","sltu", "dadd","daddu","dsub","dsubu",
/*48 */ "tge","tgeu","tlt","tltu","teq","spec65","tne","spec67",
/*56 */ "dsll","spec71","dsrl","dsra","dsll32","spec75","dsrl32","dsra32"
};

static const char * const r4k_str_spec2_name[4] = /* QED RM4650, R5000, etc. */
{		
/* 0 */ "mad", "madu", "mul", "spec3"
};

static const char * const r4k_str_bcond_name[32] = 
{
/* 0 */ "bltz", "bgez", "bltzl", "bgezl", "?", "?", "?", "?",
/* 8 */ "tgei", "tgeiu", "tlti", "tltiu", "teqi", "?", "tnei", "?",
/*16 */ "bltzal", "bgezal", "bltzall", "bgezall", "?", "?", "?", "?",
/*24 */ "?", "?", "?", "?", "?", "?", "?", "?",
};

static const char * const r4k_str_cop1_name[64] = 
{
/* 0 */ "add",   "sub", "mul", "div", "sqrt","abs", "mov", "neg",
/* 8 */ "fop08", "trunc.l","fop0a","fop0b","fop0c","trunc.w","fop0e","fop0f",
/*16 */ "fop10", "fop11","fop12","fop13","fop14","fop15","fop16","fop17",
/*24 */ "fop18", "fop19","fop1a","fop1b","fop1c","fop1d","fop1e","fop1f",
/*32 */ "cvt.s", "cvt.d","fop22","fop23","cvt.w","cvt.l","fop26","fop27",
/*40 */ "fop28", "fop29","fop2a","fop2b","fop2c","fop2d","fop2e","fop2f",
/*48 */ "c.f",   "c.un","c.eq","c.ueq","c.olt","c.ult",
	"c.ole", "c.ule",
/*56 */ "c.sf",  "c.ngle","c.seq","c.ngl","c.lt","c.nge",
	"c.le",  "c.ngt"
};

static const char * const r4k_str_fmt_name[16] = 
{
    
	"s",	"d",	"e",	"fmt3",
	"w",	"l",    "fmt6", "fmt7",
	"fmt8", "fmt9", "fmta", "fmtb",
	"fmtc", "fmtd", "fmte", "fmtf"
};


static const char * const r4k_str_reg_name[32] = 
{
	"$zero", "$at",	"v0",	"v1",	"a0",	"a1",	"a2",	"a3",
	"t0",	"t1",	"t2",	"t3",	"t4",	"t5",	"t6",	"t7",
	"s0",	"s1",	"s2",	"s3",	"s4",	"s5",	"s6",	"s7",
	"t8",	"t9",	"k0",	"k1",	"$gp",	"$sp",	"s8",	"$ra"
};

static const char * const r4k_str_c0_opname[64] = 
{
	"c0op00","tlbr",  "tlbwi", "c0op03","c0op04","c0op05","tlbwr", "c0op07",
	"tlbp",	 "c0op11","c0op12","c0op13","c0op14","c0op15","c0op16","c0op17",
	"rfe",	 "c0op21","c0op22","c0op23","c0op24","c0op25","c0op26","c0op27",
	"eret",  "c0op31","c0op32","c0op33","c0op34","c0op35","c0op36","c0op37",
	"c0op40","c0op41","c0op42","c0op43","c0op44","c0op45","c0op46","c0op47",
	"c0op50","c0op51","c0op52","c0op53","c0op54","c0op55","c0op56","c0op57",
	"c0op60","c0op61","c0op62","c0op63","c0op64","c0op65","c0op66","c0op67",
	"c0op70","c0op71","c0op72","c0op73","c0op74","c0op75","c0op77","c0op77",
};

static const char * const r4k_str_c0_reg[32] = 
{
	"C0_INX",      "C0_RAND",     "C0_ENTRYLO0",  "C0_ENTRYLO1",
	"C0_CONTEXT",  "C0_PAGEMASK", "C0_WIRED",     "cp0r7",
	"C0_BADVADDR", "C0_COUNT",    "C0_ENTRYHI",   "C0_COMPARE",
	"C0_SR",       "C0_CAUSE",    "C0_EPC",       "C0_PRID",
	"C0_CONFIG",   "C0_LLADDR",   "C0_WATCHLO",   "C0_WATCHHI",
	"xcontext",    "cp0r21",      "cp0r22",       "debug",
	"depc",        "perfcnt",     "C0_ECC",       "C0_CACHE_ERR",
	"C0_TAGLO",    "C0_TAGHI",    "C0_ERROR_EPC", "desave"
};



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Local functions - lookup
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Look up a symbol */
static char *
lookup_sym ( struct r4k_dis_t * state,
             uint32_t address          )
{
    if( state->lookup_sym )
        return state->lookup_sym( address, state->lookup_sym_d );
    
    return NULL;
}

/* Look up an upper 16-bits relocation */
static char *
lookup_rel_hi16 ( struct r4k_dis_t * state,
                  uint32_t address          )
{
    if( state->lookup_rel_hi16 )
        return state->lookup_rel_hi16( address, state->lookup_rel_hi16_d );
    
    return NULL;
}

/* Look up a lower 16-bits relocation */
static char *
lookup_rel_lo16 ( struct r4k_dis_t * state,
                  uint32_t address          )
{
    if( state->lookup_rel_lo16 )
        return state->lookup_rel_lo16( address, state->lookup_rel_lo16_d );
    
    return NULL;
}



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Local functions - disassembler
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Print text into the destination buffer */
static int
db_printf ( struct r4k_dis_t * state,
            char * fmt, 
            ...                       )
{
    int l;
    va_list ap;
    char buffer[1024];
    
    
    /* Prepare user provided */
    va_start( ap, fmt );
    l = vsnprintf( buffer, sizeof(buffer), fmt, ap );
    va_end( ap );
    
    /* Add it to our string */
    state->dest += sprintf(
        state->dest,
        "%s",
        buffer
    );
    state->length += l;
    
    return l;
}


/* Print an address to a string. If there's a symbol, the name will be printed */
static int
print_addr ( struct r4k_dis_t * state,
             uint32_t address          )
{
    int len;
    char * sym;
    
    
    /* Try to lookup symbol */
    if( (sym = lookup_sym(state, address)) )
    {
        len = db_printf( state, "%s", sym );
    }
    else
    {
        len = db_printf( state, "0x%08X", address );
    }
    
    return len;
}


/* Disassemble an instruction */
static db_addr_t
db_disasm_insn ( struct r4k_dis_t * state,
                 int insn, 
                 db_addr_t loc, 
                 bool altfmt               )
{
    char * rel;
    InstFmt i;

    i.word = insn;

    switch (i.JType.op) {
    case OP_SPECIAL:
            if (i.word == 0) {
                    db_printf(state, "nop");
                    break;
            }
            /* XXX
             * "addu" is a "move" only in 32-bit mode.  What's the correct
             * answer - never decode addu/daddu as "move"?
             */
            if (i.RType.func == OP_ADDU && i.RType.rt == 0) {
                    db_printf(state, "%-16s%s,%s",
                        "move",
                        r4k_str_reg_name[i.RType.rd],
                        r4k_str_reg_name[i.RType.rs]);
                    break;
            }
            db_printf(state, "%-16s", r4k_str_spec_name[i.RType.func]);
            switch (i.RType.func) {
            case OP_SLL:
            case OP_SRL:
            case OP_SRA:
            case OP_DSLL:

            case OP_DSRL:
            case OP_DSRA:
            case OP_DSLL32:
            case OP_DSRL32:
            case OP_DSRA32:
                    db_printf(state, "%s,%s,%d",
                        r4k_str_reg_name[i.RType.rd],
                        r4k_str_reg_name[i.RType.rt],
                        i.RType.shamt);
                    break;

            case OP_SLLV:
            case OP_SRLV:
            case OP_SRAV:
            case OP_DSLLV:
            case OP_DSRLV:
            case OP_DSRAV:
                    db_printf(state, "%s,%s,%s",
                        r4k_str_reg_name[i.RType.rd],
                        r4k_str_reg_name[i.RType.rt],
                        r4k_str_reg_name[i.RType.rs]);
                    break;

            case OP_MFHI:
            case OP_MFLO:
                    db_printf(state, "%s", r4k_str_reg_name[i.RType.rd]);
                    break;

            case OP_JR:
            case OP_JALR:
                    db_printf(state, "%s", r4k_str_reg_name[i.RType.rs]);
                    break;
            case OP_MTLO:
            case OP_MTHI:
                    db_printf(state, "%s", r4k_str_reg_name[i.RType.rs]);
                    break;

            case OP_MULT:
            case OP_MULTU:
            case OP_DMULT:
            case OP_DMULTU:
                    db_printf(state, "%s,%s",
                        r4k_str_reg_name[i.RType.rs],
                        r4k_str_reg_name[i.RType.rt]);
                    break;
                    
            case OP_DIV:
            case OP_DIVU:
            case OP_DDIV:
            case OP_DDIVU:
                    db_printf(state, "$zero,%s,%s",
                        r4k_str_reg_name[i.RType.rs],
                        r4k_str_reg_name[i.RType.rt]);
                    break;


            case OP_SYSCALL:
            case OP_SYNC:
                    break;

            case OP_BREAK:
                    db_printf(state, "%d", (i.RType.rs << 5) | i.RType.rt);
                    break;

            default:
                    db_printf(state, "%s,%s,%s",
                        r4k_str_reg_name[i.RType.rd],
                        r4k_str_reg_name[i.RType.rs],
                        r4k_str_reg_name[i.RType.rt]);
            }
            break;

    case OP_SPECIAL2:
            if (i.RType.func == OP_MUL)
                    db_printf(state, "%s\t%s,%s,%s",
                            r4k_str_spec2_name[i.RType.func & 0x3],
                            r4k_str_reg_name[i.RType.rd],
                            r4k_str_reg_name[i.RType.rs],
                            r4k_str_reg_name[i.RType.rt]);
            else
                    db_printf(state, "%s\t%s,%s",
                            r4k_str_spec2_name[i.RType.func & 0x3],
                            r4k_str_reg_name[i.RType.rs],
                            r4k_str_reg_name[i.RType.rt]);
                    
            break;

    case OP_BCOND:
            db_printf(state, "%-16s%s,", r4k_str_bcond_name[i.IType.rt],
                r4k_str_reg_name[i.IType.rs]);
            goto pr_displ;

    case OP_BLEZ:
    case OP_BLEZL:
    case OP_BGTZ:
    case OP_BGTZL:
            db_printf(state, "%-16s%s,", r4k_str_op_name[i.IType.op],
                r4k_str_reg_name[i.IType.rs]);
            goto pr_displ;

    case OP_BEQ:
    case OP_BEQL:
            if (i.IType.rs == 0 && i.IType.rt == 0) {
                    db_printf(state, "%-16s", "b");
                    goto pr_displ;
            }
            /* FALLTHROUGH */
    case OP_BNE:
    case OP_BNEL:
            db_printf(state, "%-16s%s,%s,", r4k_str_op_name[i.IType.op],
                r4k_str_reg_name[i.IType.rs],
                r4k_str_reg_name[i.IType.rt]);
    pr_displ:
            print_addr( state, loc + 4 + ((short)i.IType.imm << 2) );
            break;

    case OP_COP0:
            switch (i.RType.rs) {
            case OP_BCx:
            case OP_BCy:

                    db_printf(state, "bc0%c\t",
                        "ft"[i.RType.rt & COPz_BC_TF_MASK]);
                    goto pr_displ;

            case OP_MT:
                    db_printf(state, "%-16s%s,%s",
                        "mtc0",
                        r4k_str_reg_name[i.RType.rt],
                        r4k_str_c0_reg[i.RType.rd]);
                    break;

            case OP_DMT:
                    db_printf(state, "%-16s%s,%s",
                        "dmtc0",
                        r4k_str_reg_name[i.RType.rt],
                        r4k_str_c0_reg[i.RType.rd]);
                    break;

            case OP_MF:
                    db_printf(state, "%-16s%s,%s", "mfc0",
                        r4k_str_reg_name[i.RType.rt],
                        r4k_str_c0_reg[i.RType.rd]);
                    break;

            case OP_DMF:
                    db_printf(state, "%-16s%s,%s","dmfc0",
                        r4k_str_reg_name[i.RType.rt],
                        r4k_str_c0_reg[i.RType.rd]);
                    break;

            default:
                    db_printf(state, "%s", r4k_str_c0_opname[i.FRType.func]);
            }
            break;

    case OP_COP1:
            switch (i.RType.rs) {
            case OP_BCx:
            case OP_BCy:
                    db_printf(state, "bc1%c%s\t\t",
                        "ft"[i.RType.rt & COPz_BC_TF_MASK],
                        (insn >> 16 & 0x1F) == 2 || (insn >> 16 & 0x1F)  == 3 ? "l" : "");
                    goto pr_displ;

            case OP_MT:
                    db_printf(state, "mtc1\t\t%s,$f%d",
                        r4k_str_reg_name[i.RType.rt],
                        i.RType.rd);
                    break;

            case OP_MF:
                    db_printf(state, "mfc1\t\t%s,$f%d",
                        r4k_str_reg_name[i.RType.rt],
                        i.RType.rd);
                    break;

            case OP_CT:
                    db_printf(state, "ctc1\t\t%s,$f%d",
                        r4k_str_reg_name[i.RType.rt],
                        i.RType.rd);
                    break;

            case OP_CF:
                    db_printf(state, "cfc1\t\t%s,$f%d",
                        r4k_str_reg_name[i.RType.rt],
                        i.RType.rd);
                    break;

            case OP_DMT:
                    db_printf(state, "dmtc1\t\t%s,$f%d",
                        r4k_str_reg_name[i.RType.rt],
                        i.RType.rd);
                    break;

            case OP_DMF:
                    db_printf(state, "dmfc1\t\t%s,$f%d",
                        r4k_str_reg_name[i.RType.rt],
                        i.RType.rd);
                    break;

            case OP_MTH:
                    db_printf(state, "mthc1\t\t%s,$f%d",
                        r4k_str_reg_name[i.RType.rt],
                        i.RType.rd);
                    break;

            case OP_MFH:
                    db_printf(state, "mfhc1\t\t%s,$f%d",
                        r4k_str_reg_name[i.RType.rt],
                        i.RType.rd);
                    break;
            

            default:
                
                if( i.FRType.func == 0x21 || i.FRType.func == 0x20 || i.FRType.func == 0x24 || i.FRType.func == 0x25 || 
                    i.FRType.func == 7 || i.FRType.func == 6 || i.FRType.func == 0xd || 
                    i.FRType.func == 4 || i.FRType.func == 5 || i.FRType.func == 9 )
                {/*NEG.fmt fd, fs*/
                    
                    db_printf(state, "%s.%s\t\t$f%d,$f%d",
                        r4k_str_cop1_name[i.FRType.func],
                        r4k_str_fmt_name[i.FRType.fmt],
                        i.FRType.fd, i.FRType.fs);
                }
                else if( i.FRType.func != 1 && i.FRType.func != 2 && (insn & 0x3F) && !(insn >> 6 & 0x1F) ) /* C */
                {
                    db_printf(state, "%s.%s\t\t$f%d,$f%d",
                        r4k_str_cop1_name[i.FRType.func],
                        r4k_str_fmt_name[i.FRType.fmt],
                        i.FRType.fs, i.FRType.ft);
                }
                else
                {
                    db_printf(state, "%s.%s\t\t$f%d,$f%d,$f%d",
                        r4k_str_cop1_name[i.FRType.func],
                        r4k_str_fmt_name[i.FRType.fmt],
                        i.FRType.fd, i.FRType.fs, i.FRType.ft);
                }
            }
            break;

    case OP_J:
    case OP_JAL:
            db_printf(state, "%-16s", r4k_str_op_name[i.JType.op]);
            print_addr(state, (loc & 0xF0000000) | (i.JType.target << 2));
            break;
    
    case OP_LDC1:
    case OP_LWC1:
    case OP_SWC1:
    case OP_SDC1:
            db_printf(state, "%-16s$f%d,", r4k_str_op_name[i.IType.op],
                i.IType.rt);
            goto loadstore;

    case OP_LB:
    case OP_LH:
    case OP_LW:
    case OP_LWL:
    case OP_LWR:
    case OP_LD:
    case OP_LBU:
    case OP_LHU:
    case OP_LWU:
    case OP_SB:
    case OP_SH:
    case OP_SW:
    case OP_SWL:
    case OP_SWR:
    case OP_SD:
            db_printf(state, "%-16s%s,", r4k_str_op_name[i.IType.op],
                r4k_str_reg_name[i.IType.rt]);
    loadstore:
        
        /* Part of a relocation? */
        if( (rel = lookup_rel_lo16(state, loc)) )
        {
            /* Yes. */
            db_printf(state, 
                "%%lo(%s)(%s)",
                rel,
                r4k_str_reg_name[i.IType.rs]
            );
            
            break;
        }
        
        
            db_printf(state, "%d(%s)", (short)i.IType.imm,
                r4k_str_reg_name[i.IType.rs]);
            break;

    case OP_ORI:
    case OP_XORI:
            if( i.IType.op == OP_ORI )
            {
                /* Part of a relocation? */
                if( (rel = lookup_rel_lo16(state, loc)) )
                {
                    /* Yes. */
                    db_printf(state, 
                        "%-16s%s,%s,%%lo(%s)", 
                        r4k_str_op_name[i.IType.op],
                        r4k_str_reg_name[i.IType.rt],
                        r4k_str_reg_name[i.IType.rs],
                        rel
                    );
                    
                    break;
                }
                else
                {
                    db_printf(state, "%-16s%s,%s,0x%x", r4k_str_op_name[i.IType.op],
                        r4k_str_reg_name[i.IType.rt],
                        r4k_str_reg_name[i.IType.rs],
                        i.IType.imm);
                    
                    break;
                }
            }
            else
            if (i.IType.rs == 0) {
                    db_printf(state, "%-16s%s,0x%x",
                        "li",
                        r4k_str_reg_name[i.IType.rt],
                        i.IType.imm);
                    break;
            }
            /* FALLTHROUGH */
    case OP_ANDI:
            db_printf(state, "%-16s%s,%s,0x%x", r4k_str_op_name[i.IType.op],
                r4k_str_reg_name[i.IType.rt],
                r4k_str_reg_name[i.IType.rs],
                i.IType.imm);
            break;

    case OP_LUI:
    {
        /* Part of a relocation? */
        if( (rel = lookup_rel_hi16(state, loc)) )
        {
            /* Yes. */
            db_printf(state, 
                "%-16s%s,%%hi(%s)",
                r4k_str_op_name[i.IType.op],
                r4k_str_reg_name[i.IType.rt],
                rel
            );
        }
        else
        {
            db_printf(state, "%-16s%s,0x%x", r4k_str_op_name[i.IType.op],
                r4k_str_reg_name[i.IType.rt],
                i.IType.imm);
        }
    }
    break;

    case OP_CACHE:
            db_printf(state, "%-16s0x%x,0x%x(%s)",
                r4k_str_op_name[i.IType.op],
                i.IType.rt,
                i.IType.imm,
                r4k_str_reg_name[i.IType.rs]);
            break;

    case OP_ADDI:
    case OP_DADDI:
    case OP_ADDIU:
    case OP_DADDIU:
    {
        
        /* Part of a relocation? */
        if( (rel = lookup_rel_lo16(state, loc)) )
        {
            /* Yes. */
            db_printf(state, 
                "%-16s%s,%s,%%lo(%s)", 
                r4k_str_op_name[i.IType.op],
                r4k_str_reg_name[i.IType.rt],
                r4k_str_reg_name[i.IType.rs],
                rel
            );
            
            break;
        }
        
            if (i.IType.rs == 0) {
                    db_printf(state, "%-16s%s,%d", "li",
                        r4k_str_reg_name[i.IType.rt],
                        (short)i.IType.imm);
                    break;
            }
            /* FALLTHROUGH */
    
    default:
        
            db_printf(state, "%-16s%s,%s,%d", r4k_str_op_name[i.IType.op],
                r4k_str_reg_name[i.IType.rt],
                r4k_str_reg_name[i.IType.rs],
                (short)i.IType.imm);
    } }
    /*db_printf(state, "\n");*/
    
    return (loc + 4);
}



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Global functions
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Disassemble an instruction with state */
static int
r4k_disassemble ( struct r4k_dis_t * state,
                  uint32_t instruction,
                  uint32_t location,
                  char * dest               )
{
    state->dest = dest;
    db_disasm_insn( state, instruction, location, 0 );
    
    return state->length;
}



/* Disassemble an instruction but split the opcode/operands into two char *'s */
static int
r4k_disassemble_split ( struct r4k_dis_t * state,
                        uint32_t instruction,
                        uint32_t location,
                        char ** opcode,
                        char ** operands         )
{
    int v, i;
    char buff[128], * dupd;
    
    v = r4k_disassemble(
        state,
        instruction,
        location,
        buff
    );
    
    dupd = strdup( buff );
    *opcode = &dupd[0];
    
    for( i = 0; buff[i] && buff[i] != ' '; i++ );
    
    dupd[i] = '\0';
    
    for( ; buff[i] && buff[i] == ' '; i++ );
    
    *operands = &dupd[i];
    
    return v;
}



/* Disassemble an instruction with a blank state but split op/operands */
static int
r4k_disassemble_split_quick ( uint32_t instruction,
                              uint32_t location,
                              char ** opcode,
                              char ** operands      )
{
    struct r4k_dis_t state;
    
    /* Init state */
    memset( &state, 0, sizeof(state) );
    
    /* Perform */
    return r4k_disassemble_split(
        &state,
        instruction,
        location,
        opcode,
        operands
    );
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=[ DECODE_OP ]=-=-=-=-=-=-=-=-=-=-=-=-=-=-=[//

void r4300_decode_op ( uint32 instr, char * opcode, char * arguments, int counter )
{
    char * _op, * _args;
    
    _op = NULL;
    _args = NULL;
    
    r4k_disassemble_split_quick(
        instr,
        counter,
        &_op,
        &_args
    );
    
    strcpy( opcode, _op );
    strcpy( arguments, _args );
    
    free( _op );
}

