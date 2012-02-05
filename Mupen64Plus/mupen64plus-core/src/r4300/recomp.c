/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - recomp.h                                                *
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

#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__)
#include <unistd.h>
#include <sys/mman.h>
#endif

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "memory/memory.h"

#include "recomp.h"
#include "recomph.h" //include for function prototypes
#include "macros.h"
#include "r4300.h"
#include "ops.h"

// global variables :
precomp_instr *dst; // destination structure for the recompiled instruction
int code_length; // current real recompiled code length
int max_code_length; // current recompiled code's buffer length
unsigned char **inst_pointer; // output buffer for recompiled code
precomp_block *dst_block; // the current block that we are recompiling
int src; // the current recompiled instruction
int fast_memory;

unsigned long *return_address; // that's where the dynarec will restart when
                               // going back from a C function

#if defined(PROFILE_R4300)
FILE *pfProfile;
#endif

static int *SRC; // currently recompiled instruction in the input stream
static int check_nop; // next instruction is nop ?
static int delay_slot_compiled = 0;



static void RSV(void)
{
   dst->ops = RESERVED;
   if (r4300emu == CORE_DYNAREC) genreserved();
}

static void RFIN_BLOCK(void)
{
   dst->ops = FIN_BLOCK;
   if (r4300emu == CORE_DYNAREC) genfin_block();
}

static void RNOTCOMPILED(void)
{
   dst->ops = NOTCOMPILED;
   if (r4300emu == CORE_DYNAREC) gennotcompiled();
}

static void recompile_standard_i_type(void)
{
   dst->f.i.rs = reg + ((src >> 21) & 0x1F);
   dst->f.i.rt = reg + ((src >> 16) & 0x1F);
   dst->f.i.immediate = src & 0xFFFF;
}

static void recompile_standard_j_type(void)
{
   dst->f.j.inst_index = src & 0x3FFFFFF;
}

static void recompile_standard_r_type(void)
{
   dst->f.r.rs = reg + ((src >> 21) & 0x1F);
   dst->f.r.rt = reg + ((src >> 16) & 0x1F);
   dst->f.r.rd = reg + ((src >> 11) & 0x1F);
   dst->f.r.sa = (src >>  6) & 0x1F;
}

static void recompile_standard_lf_type(void)
{
   dst->f.lf.base = (src >> 21) & 0x1F;
   dst->f.lf.ft = (src >> 16) & 0x1F;
   dst->f.lf.offset = src & 0xFFFF;
}

static void recompile_standard_cf_type(void)
{
   dst->f.cf.ft = (src >> 16) & 0x1F;
   dst->f.cf.fs = (src >> 11) & 0x1F;
   dst->f.cf.fd = (src >>  6) & 0x1F;
}

//-------------------------------------------------------------------------
//                                  SPECIAL                                
//-------------------------------------------------------------------------

static void RNOP(void)
{
   dst->ops = NOP;
   if (r4300emu == CORE_DYNAREC) gennop();
}

static void RSLL(void)
{
   dst->ops = SLL;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gensll();
}

static void RSRL(void)
{
   dst->ops = SRL;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gensrl();
}

static void RSRA(void)
{
   dst->ops = SRA;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gensra();
}

static void RSLLV(void)
{
   dst->ops = SLLV;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gensllv();
}

static void RSRLV(void)
{
   dst->ops = SRLV;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gensrlv();
}

static void RSRAV(void)
{
   dst->ops = SRAV;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gensrav();
}

static void RJR(void)
{
   dst->ops = JR;
   recompile_standard_i_type();
   if (r4300emu == CORE_DYNAREC) genjr();
}

static void RJALR(void)
{
   dst->ops = JALR;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) genjalr();
}

static void RSYSCALL(void)
{
   dst->ops = SYSCALL;
   if (r4300emu == CORE_DYNAREC) gensyscall();
}

static void RBREAK(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RSYNC(void)
{
   dst->ops = SYNC;
   if (r4300emu == CORE_DYNAREC) gensync();
}

static void RMFHI(void)
{
   dst->ops = MFHI;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genmfhi();
}

static void RMTHI(void)
{
   dst->ops = MTHI;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) genmthi();
}

static void RMFLO(void)
{
   dst->ops = MFLO;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genmflo();
}

static void RMTLO(void)
{
   dst->ops = MTLO;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) genmtlo();
}

static void RDSLLV(void)
{
   dst->ops = DSLLV;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendsllv();
}

static void RDSRLV(void)
{
   dst->ops = DSRLV;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendsrlv();
}

static void RDSRAV(void)
{
   dst->ops = DSRAV;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendsrav();
}

static void RMULT(void)
{
   dst->ops = MULT;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) genmult();
}

static void RMULTU(void)
{
   dst->ops = MULTU;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) genmultu();
}

static void RDIV(void)
{
   dst->ops = DIV;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) gendiv();
}

static void RDIVU(void)
{
   dst->ops = DIVU;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) gendivu();
}

static void RDMULT(void)
{
   dst->ops = DMULT;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) gendmult();
}

static void RDMULTU(void)
{
   dst->ops = DMULTU;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) gendmultu();
}

static void RDDIV(void)
{
   dst->ops = DDIV;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) genddiv();
}

static void RDDIVU(void)
{
   dst->ops = DDIVU;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) genddivu();
}

static void RADD(void)
{
   dst->ops = ADD;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genadd();
}

static void RADDU(void)
{
   dst->ops = ADDU;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genaddu();
}

static void RSUB(void)
{
   dst->ops = SUB;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   if (r4300emu == CORE_DYNAREC) gensub();
}

static void RSUBU(void)
{
   dst->ops = SUBU;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gensubu();
}

static void RAND(void)
{
   dst->ops = AND;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genand();
}

static void ROR(void)
{
   dst->ops = OR;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genor();
}

static void RXOR(void)
{
   dst->ops = XOR;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genxor();
}

static void RNOR(void)
{
   dst->ops = NOR;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
   if (r4300emu == CORE_DYNAREC) gennor();
}

static void RSLT(void)
{
   dst->ops = SLT;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genslt();
}

static void RSLTU(void)
{
   dst->ops = SLTU;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gensltu();
}

static void RDADD(void)
{
   dst->ops = DADD;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendadd();
}

static void RDADDU(void)
{
   dst->ops = DADDU;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendaddu();
}

static void RDSUB(void)
{
   dst->ops = DSUB;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendsub();
}

static void RDSUBU(void)
{
   dst->ops = DSUBU;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendsubu();
}

static void RTGE(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RTGEU(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RTLT(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RTLTU(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RTEQ(void)
{
   dst->ops = TEQ;
   recompile_standard_r_type();
   if (r4300emu == CORE_DYNAREC) genteq();
}

static void RTNE(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RDSLL(void)
{
   dst->ops = DSLL;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendsll();
}

static void RDSRL(void)
{
   dst->ops = DSRL;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendsrl();
}

static void RDSRA(void)
{
   dst->ops = DSRA;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendsra();
}

static void RDSLL32(void)
{
   dst->ops = DSLL32;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendsll32();
}

static void RDSRL32(void)
{
   dst->ops = DSRL32;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendsrl32();
}

static void RDSRA32(void)
{
   dst->ops = DSRA32;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendsra32();
}

static void (*recomp_special[64])(void) =
{
   RSLL , RSV   , RSRL , RSRA , RSLLV   , RSV    , RSRLV  , RSRAV  ,
   RJR  , RJALR , RSV  , RSV  , RSYSCALL, RBREAK , RSV    , RSYNC  ,
   RMFHI, RMTHI , RMFLO, RMTLO, RDSLLV  , RSV    , RDSRLV , RDSRAV ,
   RMULT, RMULTU, RDIV , RDIVU, RDMULT  , RDMULTU, RDDIV  , RDDIVU ,
   RADD , RADDU , RSUB , RSUBU, RAND    , ROR    , RXOR   , RNOR   ,
   RSV  , RSV   , RSLT , RSLTU, RDADD   , RDADDU , RDSUB  , RDSUBU ,
   RTGE , RTGEU , RTLT , RTLTU, RTEQ    , RSV    , RTNE   , RSV    ,
   RDSLL, RSV   , RDSRL, RDSRA, RDSLL32 , RSV    , RDSRL32, RDSRA32
};

//-------------------------------------------------------------------------
//                                   REGIMM                                
//-------------------------------------------------------------------------

static void RBLTZ(void)
{
    unsigned int target;
   dst->ops = BLTZ;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BLTZ_IDLE;
         if (r4300emu == CORE_DYNAREC) genbltz_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbltz();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BLTZ_OUT;
    if (r4300emu == CORE_DYNAREC) genbltz_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbltz();
}

static void RBGEZ(void)
{
    unsigned int target;
   dst->ops = BGEZ;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BGEZ_IDLE;
         if (r4300emu == CORE_DYNAREC) genbgez_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbgez();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BGEZ_OUT;
    if (r4300emu == CORE_DYNAREC) genbgez_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbgez();
}

static void RBLTZL(void)
{
    unsigned int target;
   dst->ops = BLTZL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BLTZL_IDLE;
         if (r4300emu == CORE_DYNAREC) genbltzl_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbltzl();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BLTZL_OUT;
    if (r4300emu == CORE_DYNAREC) genbltzl_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbltzl();
}

static void RBGEZL(void)
{
    unsigned int target;
   dst->ops = BGEZL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BGEZL_IDLE;
         if (r4300emu == CORE_DYNAREC) genbgezl_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbgezl();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BGEZL_OUT;
    if (r4300emu == CORE_DYNAREC) genbgezl_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbgezl();
}

static void RTGEI(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RTGEIU(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RTLTI(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RTLTIU(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RTEQI(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RTNEI(void)
{
   dst->ops = NI;
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RBLTZAL(void)
{
    unsigned int target;
   dst->ops = BLTZAL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BLTZAL_IDLE;
         if (r4300emu == CORE_DYNAREC) genbltzal_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbltzal();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BLTZAL_OUT;
    if (r4300emu == CORE_DYNAREC) genbltzal_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbltzal();
}

static void RBGEZAL(void)
{
    unsigned int target;
   dst->ops = BGEZAL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BGEZAL_IDLE;
         if (r4300emu == CORE_DYNAREC) genbgezal_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbgezal();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BGEZAL_OUT;
    if (r4300emu == CORE_DYNAREC) genbgezal_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbgezal();
}

static void RBLTZALL(void)
{
    unsigned int target;
   dst->ops = BLTZALL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BLTZALL_IDLE;
         if (r4300emu == CORE_DYNAREC) genbltzall_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbltzall();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BLTZALL_OUT;
    if (r4300emu == CORE_DYNAREC) genbltzall_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbltzall();
}

static void RBGEZALL(void)
{
    unsigned int target;
   dst->ops = BGEZALL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BGEZALL_IDLE;
         if (r4300emu == CORE_DYNAREC) genbgezall_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbgezall();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BGEZALL_OUT;
    if (r4300emu == CORE_DYNAREC) genbgezall_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbgezall();
}

static void (*recomp_regimm[32])(void) =
{
   RBLTZ  , RBGEZ  , RBLTZL  , RBGEZL  , RSV  , RSV, RSV  , RSV,
   RTGEI  , RTGEIU , RTLTI   , RTLTIU  , RTEQI, RSV, RTNEI, RSV,
   RBLTZAL, RBGEZAL, RBLTZALL, RBGEZALL, RSV  , RSV, RSV  , RSV,
   RSV    , RSV    , RSV     , RSV     , RSV  , RSV, RSV  , RSV
};

//-------------------------------------------------------------------------
//                                     TLB                                 
//-------------------------------------------------------------------------

static void RTLBR(void)
{
   dst->ops = TLBR;
   if (r4300emu == CORE_DYNAREC) gentlbr();
}

static void RTLBWI(void)
{
   dst->ops = TLBWI;
   if (r4300emu == CORE_DYNAREC) gentlbwi();
}

static void RTLBWR(void)
{
   dst->ops = TLBWR;
   if (r4300emu == CORE_DYNAREC) gentlbwr();
}

static void RTLBP(void)
{
   dst->ops = TLBP;
   if (r4300emu == CORE_DYNAREC) gentlbp();
}

static void RERET(void)
{
   dst->ops = ERET;
   if (r4300emu == CORE_DYNAREC) generet();
}

static void (*recomp_tlb[64])(void) =
{
   RSV  , RTLBR, RTLBWI, RSV, RSV, RSV, RTLBWR, RSV, 
   RTLBP, RSV  , RSV   , RSV, RSV, RSV, RSV   , RSV, 
   RSV  , RSV  , RSV   , RSV, RSV, RSV, RSV   , RSV, 
   RERET, RSV  , RSV   , RSV, RSV, RSV, RSV   , RSV, 
   RSV  , RSV  , RSV   , RSV, RSV, RSV, RSV   , RSV, 
   RSV  , RSV  , RSV   , RSV, RSV, RSV, RSV   , RSV, 
   RSV  , RSV  , RSV   , RSV, RSV, RSV, RSV   , RSV, 
   RSV  , RSV  , RSV   , RSV, RSV, RSV, RSV   , RSV
};

//-------------------------------------------------------------------------
//                                    COP0                                 
//-------------------------------------------------------------------------

static void RMFC0(void)
{
   dst->ops = MFC0;
   recompile_standard_r_type();
   dst->f.r.rd = (long long*)(reg_cop0 + ((src >> 11) & 0x1F));
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (dst->f.r.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genmfc0();
}

static void RMTC0(void)
{
   dst->ops = MTC0;
   recompile_standard_r_type();
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (r4300emu == CORE_DYNAREC) genmtc0();
}

static void RTLB(void)
{
   recomp_tlb[(src & 0x3F)]();
}

static void (*recomp_cop0[32])(void) =
{
   RMFC0, RSV, RSV, RSV, RMTC0, RSV, RSV, RSV,
   RSV  , RSV, RSV, RSV, RSV  , RSV, RSV, RSV,
   RTLB , RSV, RSV, RSV, RSV  , RSV, RSV, RSV,
   RSV  , RSV, RSV, RSV, RSV  , RSV, RSV, RSV
};

//-------------------------------------------------------------------------
//                                     BC                                  
//-------------------------------------------------------------------------

static void RBC1F(void)
{
    unsigned int target;
   dst->ops = BC1F;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BC1F_IDLE;
         if (r4300emu == CORE_DYNAREC) genbc1f_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbc1f();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BC1F_OUT;
    if (r4300emu == CORE_DYNAREC) genbc1f_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbc1f();
}

static void RBC1T(void)
{
    unsigned int target;
   dst->ops = BC1T;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BC1T_IDLE;
         if (r4300emu == CORE_DYNAREC) genbc1t_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbc1t();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BC1T_OUT;
    if (r4300emu == CORE_DYNAREC) genbc1t_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbc1t();
}

static void RBC1FL(void)
{
    unsigned int target;
   dst->ops = BC1FL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BC1FL_IDLE;
         if (r4300emu == CORE_DYNAREC) genbc1fl_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbc1fl();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BC1FL_OUT;
    if (r4300emu == CORE_DYNAREC) genbc1fl_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbc1fl();
}

static void RBC1TL(void)
{
    unsigned int target;
   dst->ops = BC1TL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BC1TL_IDLE;
         if (r4300emu == CORE_DYNAREC) genbc1tl_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbc1tl();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BC1TL_OUT;
    if (r4300emu == CORE_DYNAREC) genbc1tl_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbc1tl();
}

static void (*recomp_bc[4])(void) =
{
   RBC1F , RBC1T ,
   RBC1FL, RBC1TL
};

//-------------------------------------------------------------------------
//                                     S                                   
//-------------------------------------------------------------------------

static void RADD_S(void)
{
   dst->ops = ADD_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genadd_s();
}

static void RSUB_S(void)
{
   dst->ops = SUB_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gensub_s();
}

static void RMUL_S(void)
{
   dst->ops = MUL_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genmul_s();
}

static void RDIV_S(void)
{
   dst->ops = DIV_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gendiv_s();
}

static void RSQRT_S(void)
{
   dst->ops = SQRT_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gensqrt_s();
}

static void RABS_S(void)
{
   dst->ops = ABS_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genabs_s();
}

static void RMOV_S(void)
{
   dst->ops = MOV_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genmov_s();
}

static void RNEG_S(void)
{
   dst->ops = NEG_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genneg_s();
}

static void RROUND_L_S(void)
{
   dst->ops = ROUND_L_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genround_l_s();
}

static void RTRUNC_L_S(void)
{
   dst->ops = TRUNC_L_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gentrunc_l_s();
}

static void RCEIL_L_S(void)
{
   dst->ops = CEIL_L_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genceil_l_s();
}

static void RFLOOR_L_S(void)
{
   dst->ops = FLOOR_L_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genfloor_l_s();
}

static void RROUND_W_S(void)
{
   dst->ops = ROUND_W_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genround_w_s();
}

static void RTRUNC_W_S(void)
{
   dst->ops = TRUNC_W_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gentrunc_w_s();
}

static void RCEIL_W_S(void)
{
   dst->ops = CEIL_W_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genceil_w_s();
}

static void RFLOOR_W_S(void)
{
   dst->ops = FLOOR_W_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genfloor_w_s();
}

static void RCVT_D_S(void)
{
   dst->ops = CVT_D_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gencvt_d_s();
}

static void RCVT_W_S(void)
{
   dst->ops = CVT_W_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gencvt_w_s();
}

static void RCVT_L_S(void)
{
   dst->ops = CVT_L_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gencvt_l_s();
}

static void RC_F_S(void)
{
   dst->ops = C_F_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_f_s();
}

static void RC_UN_S(void)
{
   dst->ops = C_UN_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_un_s();
}

static void RC_EQ_S(void)
{
   dst->ops = C_EQ_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_eq_s();
}

static void RC_UEQ_S(void)
{
   dst->ops = C_UEQ_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ueq_s();
}

static void RC_OLT_S(void)
{
   dst->ops = C_OLT_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_olt_s();
}

static void RC_ULT_S(void)
{
   dst->ops = C_ULT_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ult_s();
}

static void RC_OLE_S(void)
{
   dst->ops = C_OLE_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ole_s();
}

static void RC_ULE_S(void)
{
   dst->ops = C_ULE_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ule_s();
}

static void RC_SF_S(void)
{
   dst->ops = C_SF_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_sf_s();
}

static void RC_NGLE_S(void)
{
   dst->ops = C_NGLE_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ngle_s();
}

static void RC_SEQ_S(void)
{
   dst->ops = C_SEQ_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_seq_s();
}

static void RC_NGL_S(void)
{
   dst->ops = C_NGL_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ngl_s();
}

static void RC_LT_S(void)
{
   dst->ops = C_LT_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_lt_s();
}

static void RC_NGE_S(void)
{
   dst->ops = C_NGE_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_nge_s();
}

static void RC_LE_S(void)
{
   dst->ops = C_LE_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_le_s();
}

static void RC_NGT_S(void)
{
   dst->ops = C_NGT_S;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ngt_s();
}

static void (*recomp_s[64])(void) =
{
   RADD_S    , RSUB_S    , RMUL_S   , RDIV_S    , RSQRT_S   , RABS_S    , RMOV_S   , RNEG_S    , 
   RROUND_L_S, RTRUNC_L_S, RCEIL_L_S, RFLOOR_L_S, RROUND_W_S, RTRUNC_W_S, RCEIL_W_S, RFLOOR_W_S, 
   RSV       , RSV       , RSV      , RSV       , RSV       , RSV       , RSV      , RSV       , 
   RSV       , RSV       , RSV      , RSV       , RSV       , RSV       , RSV      , RSV       , 
   RSV       , RCVT_D_S  , RSV      , RSV       , RCVT_W_S  , RCVT_L_S  , RSV      , RSV       , 
   RSV       , RSV       , RSV      , RSV       , RSV       , RSV       , RSV      , RSV       , 
   RC_F_S    , RC_UN_S   , RC_EQ_S  , RC_UEQ_S  , RC_OLT_S  , RC_ULT_S  , RC_OLE_S , RC_ULE_S  , 
   RC_SF_S   , RC_NGLE_S , RC_SEQ_S , RC_NGL_S  , RC_LT_S   , RC_NGE_S  , RC_LE_S  , RC_NGT_S
};

//-------------------------------------------------------------------------
//                                     D                                   
//-------------------------------------------------------------------------

static void RADD_D(void)
{
   dst->ops = ADD_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genadd_d();
}

static void RSUB_D(void)
{
   dst->ops = SUB_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gensub_d();
}

static void RMUL_D(void)
{
   dst->ops = MUL_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genmul_d();
}

static void RDIV_D(void)
{
   dst->ops = DIV_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gendiv_d();
}

static void RSQRT_D(void)
{
   dst->ops = SQRT_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gensqrt_d();
}

static void RABS_D(void)
{
   dst->ops = ABS_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genabs_d();
}

static void RMOV_D(void)
{
   dst->ops = MOV_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genmov_d();
}

static void RNEG_D(void)
{
   dst->ops = NEG_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genneg_d();
}

static void RROUND_L_D(void)
{
   dst->ops = ROUND_L_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genround_l_d();
}

static void RTRUNC_L_D(void)
{
   dst->ops = TRUNC_L_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gentrunc_l_d();
}

static void RCEIL_L_D(void)
{
   dst->ops = CEIL_L_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genceil_l_d();
}

static void RFLOOR_L_D(void)
{
   dst->ops = FLOOR_L_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genfloor_l_d();
}

static void RROUND_W_D(void)
{
   dst->ops = ROUND_W_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genround_w_d();
}

static void RTRUNC_W_D(void)
{
   dst->ops = TRUNC_W_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gentrunc_w_d();
}

static void RCEIL_W_D(void)
{
   dst->ops = CEIL_W_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genceil_w_d();
}

static void RFLOOR_W_D(void)
{
   dst->ops = FLOOR_W_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genfloor_w_d();
}

static void RCVT_S_D(void)
{
   dst->ops = CVT_S_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gencvt_s_d();
}

static void RCVT_W_D(void)
{
   dst->ops = CVT_W_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gencvt_w_d();
}

static void RCVT_L_D(void)
{
   dst->ops = CVT_L_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gencvt_l_d();
}

static void RC_F_D(void)
{
   dst->ops = C_F_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_f_d();
}

static void RC_UN_D(void)
{
   dst->ops = C_UN_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_un_d();
}

static void RC_EQ_D(void)
{
   dst->ops = C_EQ_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_eq_d();
}

static void RC_UEQ_D(void)
{
   dst->ops = C_UEQ_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ueq_d();
}

static void RC_OLT_D(void)
{
   dst->ops = C_OLT_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_olt_d();
}

static void RC_ULT_D(void)
{
   dst->ops = C_ULT_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ult_d();
}

static void RC_OLE_D(void)
{
   dst->ops = C_OLE_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ole_d();
}

static void RC_ULE_D(void)
{
   dst->ops = C_ULE_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ule_d();
}

static void RC_SF_D(void)
{
   dst->ops = C_SF_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_sf_d();
}

static void RC_NGLE_D(void)
{
   dst->ops = C_NGLE_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ngle_d();
}

static void RC_SEQ_D(void)
{
   dst->ops = C_SEQ_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_seq_d();
}

static void RC_NGL_D(void)
{
   dst->ops = C_NGL_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ngl_d();
}

static void RC_LT_D(void)
{
   dst->ops = C_LT_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_lt_d();
}

static void RC_NGE_D(void)
{
   dst->ops = C_NGE_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_nge_d();
}

static void RC_LE_D(void)
{
   dst->ops = C_LE_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_le_d();
}

static void RC_NGT_D(void)
{
   dst->ops = C_NGT_D;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) genc_ngt_d();
}

static void (*recomp_d[64])(void) =
{
   RADD_D    , RSUB_D    , RMUL_D   , RDIV_D    , RSQRT_D   , RABS_D    , RMOV_D   , RNEG_D    ,
   RROUND_L_D, RTRUNC_L_D, RCEIL_L_D, RFLOOR_L_D, RROUND_W_D, RTRUNC_W_D, RCEIL_W_D, RFLOOR_W_D,
   RSV       , RSV       , RSV      , RSV       , RSV       , RSV       , RSV      , RSV       ,
   RSV       , RSV       , RSV      , RSV       , RSV       , RSV       , RSV      , RSV       ,
   RCVT_S_D  , RSV       , RSV      , RSV       , RCVT_W_D  , RCVT_L_D  , RSV      , RSV       ,
   RSV       , RSV       , RSV      , RSV       , RSV       , RSV       , RSV      , RSV       ,
   RC_F_D    , RC_UN_D   , RC_EQ_D  , RC_UEQ_D  , RC_OLT_D  , RC_ULT_D  , RC_OLE_D , RC_ULE_D  ,
   RC_SF_D   , RC_NGLE_D , RC_SEQ_D , RC_NGL_D  , RC_LT_D   , RC_NGE_D  , RC_LE_D  , RC_NGT_D
};

//-------------------------------------------------------------------------
//                                     W                                   
//-------------------------------------------------------------------------

static void RCVT_S_W(void)
{
   dst->ops = CVT_S_W;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gencvt_s_w();
}

static void RCVT_D_W(void)
{
   dst->ops = CVT_D_W;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gencvt_d_w();
}

static void (*recomp_w[64])(void) =
{
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
   RCVT_S_W, RCVT_D_W, RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV
};

//-------------------------------------------------------------------------
//                                     L                                   
//-------------------------------------------------------------------------

static void RCVT_S_L(void)
{
   dst->ops = CVT_S_L;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gencvt_s_l();
}

static void RCVT_D_L(void)
{
   dst->ops = CVT_D_L;
   recompile_standard_cf_type();
   if (r4300emu == CORE_DYNAREC) gencvt_d_l();
}

static void (*recomp_l[64])(void) =
{
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV,
   RCVT_S_L, RCVT_D_L, RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
   RSV     , RSV     , RSV, RSV, RSV, RSV, RSV, RSV, 
};

//-------------------------------------------------------------------------
//                                    COP1                                 
//-------------------------------------------------------------------------

static void RMFC1(void)
{
   dst->ops = MFC1;
   recompile_standard_r_type();
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (dst->f.r.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genmfc1();
}

static void RDMFC1(void)
{
   dst->ops = DMFC1;
   recompile_standard_r_type();
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (dst->f.r.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendmfc1();
}

static void RCFC1(void)
{
   dst->ops = CFC1;
   recompile_standard_r_type();
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (dst->f.r.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gencfc1();
}

static void RMTC1(void)
{
   dst->ops = MTC1;
   recompile_standard_r_type();
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (r4300emu == CORE_DYNAREC) genmtc1();
}

static void RDMTC1(void)
{
   dst->ops = DMTC1;
   recompile_standard_r_type();
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (r4300emu == CORE_DYNAREC) gendmtc1();
}

static void RCTC1(void)
{
   dst->ops = CTC1;
   recompile_standard_r_type();
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (r4300emu == CORE_DYNAREC) genctc1();
}

static void RBC(void)
{
   recomp_bc[((src >> 16) & 3)]();
}

static void RS(void)
{
   recomp_s[(src & 0x3F)]();
}

static void RD(void)
{
   recomp_d[(src & 0x3F)]();
}

static void RW(void)
{
   recomp_w[(src & 0x3F)]();
}

static void RL(void)
{
   recomp_l[(src & 0x3F)]();
}

static void (*recomp_cop1[32])(void) =
{
   RMFC1, RDMFC1, RCFC1, RSV, RMTC1, RDMTC1, RCTC1, RSV,
   RBC  , RSV   , RSV  , RSV, RSV  , RSV   , RSV  , RSV,
   RS   , RD    , RSV  , RSV, RW   , RL    , RSV  , RSV,
   RSV  , RSV   , RSV  , RSV, RSV  , RSV   , RSV  , RSV
};

//-------------------------------------------------------------------------
//                                   R4300                                 
//-------------------------------------------------------------------------

static void RSPECIAL(void)
{
   recomp_special[(src & 0x3F)]();
}

static void RREGIMM(void)
{
   recomp_regimm[((src >> 16) & 0x1F)]();
}

static void RJ(void)
{
    unsigned int target;
   dst->ops = J_OUT;
   recompile_standard_j_type();
   target = (dst->f.j.inst_index<<2) | (dst->addr & 0xF0000000);
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = J_IDLE;
         if (r4300emu == CORE_DYNAREC) genj_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genj();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = J_OUT;
    if (r4300emu == CORE_DYNAREC) genj_out();
     }
   else if (r4300emu == CORE_DYNAREC) genj();
}

static void RJAL(void)
{
    unsigned int target;
   dst->ops = JAL_OUT;
   recompile_standard_j_type();
   target = (dst->f.j.inst_index<<2) | (dst->addr & 0xF0000000);
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = JAL_IDLE;
         if (r4300emu == CORE_DYNAREC) genjal_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genjal();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = JAL_OUT;
    if (r4300emu == CORE_DYNAREC) genjal_out();
     }
   else if (r4300emu == CORE_DYNAREC) genjal();
}

static void RBEQ(void)
{
    unsigned int target;
   dst->ops = BEQ;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BEQ_IDLE;
         if (r4300emu == CORE_DYNAREC) genbeq_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbeq();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BEQ_OUT;
    if (r4300emu == CORE_DYNAREC) genbeq_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbeq();
}

static void RBNE(void)
{
    unsigned int target;
   dst->ops = BNE;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BNE_IDLE;
         if (r4300emu == CORE_DYNAREC) genbne_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbne();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BNE_OUT;
    if (r4300emu == CORE_DYNAREC) genbne_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbne();
}

static void RBLEZ(void)
{
    unsigned int target;
   dst->ops = BLEZ;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BLEZ_IDLE;
         if (r4300emu == CORE_DYNAREC) genblez_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genblez();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BLEZ_OUT;
    if (r4300emu == CORE_DYNAREC) genblez_out();
     }
   else if (r4300emu == CORE_DYNAREC) genblez();
}

static void RBGTZ(void)
{
    unsigned int target;
   dst->ops = BGTZ;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BGTZ_IDLE;
         if (r4300emu == CORE_DYNAREC) genbgtz_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbgtz();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BGTZ_OUT;
    if (r4300emu == CORE_DYNAREC) genbgtz_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbgtz();
}

static void RADDI(void)
{
   dst->ops = ADDI;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genaddi();
}

static void RADDIU(void)
{
   dst->ops = ADDIU;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genaddiu();
}

static void RSLTI(void)
{
   dst->ops = SLTI;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genslti();
}

static void RSLTIU(void)
{
   dst->ops = SLTIU;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gensltiu();
}

static void RANDI(void)
{
   dst->ops = ANDI;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genandi();
}

static void RORI(void)
{
   dst->ops = ORI;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genori();
}

static void RXORI(void)
{
   dst->ops = XORI;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genxori();
}

static void RLUI(void)
{
   dst->ops = LUI;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genlui();
}

static void RCOP0(void)
{
   recomp_cop0[((src >> 21) & 0x1F)]();
}

static void RCOP1(void)
{
   recomp_cop1[((src >> 21) & 0x1F)]();
}

static void RBEQL(void)
{
    unsigned int target;
   dst->ops = BEQL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BEQL_IDLE;
         if (r4300emu == CORE_DYNAREC) genbeql_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbeql();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BEQL_OUT;
    if (r4300emu == CORE_DYNAREC) genbeql_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbeql();
}

static void RBNEL(void)
{
    unsigned int target;
   dst->ops = BNEL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BNEL_IDLE;
         if (r4300emu == CORE_DYNAREC) genbnel_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbnel();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BNEL_OUT;
    if (r4300emu == CORE_DYNAREC) genbnel_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbnel();
}

static void RBLEZL(void)
{
    unsigned int target;
   dst->ops = BLEZL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BLEZL_IDLE;
         if (r4300emu == CORE_DYNAREC) genblezl_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genblezl();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BLEZL_OUT;
    if (r4300emu == CORE_DYNAREC) genblezl_out();
     }
   else if (r4300emu == CORE_DYNAREC) genblezl();
}

static void RBGTZL(void)
{
    unsigned int target;
   dst->ops = BGTZL;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
     {
    if (check_nop)
      {
         dst->ops = BGTZL_IDLE;
         if (r4300emu == CORE_DYNAREC) genbgtzl_idle();
      }
    else if (r4300emu == CORE_DYNAREC) genbgtzl();
     }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
     {
    dst->ops = BGTZL_OUT;
    if (r4300emu == CORE_DYNAREC) genbgtzl_out();
     }
   else if (r4300emu == CORE_DYNAREC) genbgtzl();
}

static void RDADDI(void)
{
   dst->ops = DADDI;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendaddi();
}

static void RDADDIU(void)
{
   dst->ops = DADDIU;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gendaddiu();
}

static void RLDL(void)
{
   dst->ops = LDL;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genldl();
}

static void RLDR(void)
{
   dst->ops = LDR;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genldr();
}

static void RLB(void)
{
   dst->ops = LB;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genlb();
}

static void RLH(void)
{
   dst->ops = LH;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genlh();
}

static void RLWL(void)
{
   dst->ops = LWL;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genlwl();
}

static void RLW(void)
{
   dst->ops = LW;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genlw();
}

static void RLBU(void)
{
   dst->ops = LBU;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genlbu();
}

static void RLHU(void)
{
   dst->ops = LHU;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genlhu();
}

static void RLWR(void)
{
   dst->ops = LWR;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genlwr();
}

static void RLWU(void)
{
   dst->ops = LWU;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genlwu();
}

static void RSB(void)
{
   dst->ops = SB;
   recompile_standard_i_type();
   if (r4300emu == CORE_DYNAREC) gensb();
}

static void RSH(void)
{
   dst->ops = SH;
   recompile_standard_i_type();
   if (r4300emu == CORE_DYNAREC) gensh();
}

static void RSWL(void)
{
   dst->ops = SWL;
   recompile_standard_i_type();
   if (r4300emu == CORE_DYNAREC) genswl();
}

static void RSW(void)
{
   dst->ops = SW;
   recompile_standard_i_type();
   if (r4300emu == CORE_DYNAREC) gensw();
}

static void RSDL(void)
{
   dst->ops = SDL;
   recompile_standard_i_type();
   if (r4300emu == CORE_DYNAREC) gensdl();
}

static void RSDR(void)
{
   dst->ops = SDR;
   recompile_standard_i_type();
   if (r4300emu == CORE_DYNAREC) gensdr();
}

static void RSWR(void)
{
   dst->ops = SWR;
   recompile_standard_i_type();
   if (r4300emu == CORE_DYNAREC) genswr();
}

static void RCACHE(void)
{
   dst->ops = CACHE;
   if (r4300emu == CORE_DYNAREC) gencache();
}

static void RLL(void)
{
   dst->ops = LL;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genll();
}

static void RLWC1(void)
{
   dst->ops = LWC1;
   recompile_standard_lf_type();
   if (r4300emu == CORE_DYNAREC) genlwc1();
}

static void RLLD(void)
{
   dst->ops = NI;
   recompile_standard_i_type();
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RLDC1(void)
{
   dst->ops = LDC1;
   recompile_standard_lf_type();
   if (r4300emu == CORE_DYNAREC) genldc1();
}

static void RLD(void)
{
   dst->ops = LD;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) genld();
}

static void RSC(void)
{
   dst->ops = SC;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
   else if (r4300emu == CORE_DYNAREC) gensc();
}

static void RSWC1(void)
{
   dst->ops = SWC1;
   recompile_standard_lf_type();
   if (r4300emu == CORE_DYNAREC) genswc1();
}

static void RSCD(void)
{
   dst->ops = NI;
   recompile_standard_i_type();
   if (r4300emu == CORE_DYNAREC) genni();
}

static void RSDC1(void)
{
   dst->ops = SDC1;
   recompile_standard_lf_type();
   if (r4300emu == CORE_DYNAREC) gensdc1();
}

static void RSD(void)
{
   dst->ops = SD;
   recompile_standard_i_type();
   if (r4300emu == CORE_DYNAREC) gensd();
}

static void (*recomp_ops[64])(void) =
{
   RSPECIAL, RREGIMM, RJ   , RJAL  , RBEQ , RBNE , RBLEZ , RBGTZ ,
   RADDI   , RADDIU , RSLTI, RSLTIU, RANDI, RORI , RXORI , RLUI  ,
   RCOP0   , RCOP1  , RSV  , RSV   , RBEQL, RBNEL, RBLEZL, RBGTZL,
   RDADDI  , RDADDIU, RLDL , RLDR  , RSV  , RSV  , RSV   , RSV   ,
   RLB     , RLH    , RLWL , RLW   , RLBU , RLHU , RLWR  , RLWU  ,
   RSB     , RSH    , RSWL , RSW   , RSDL , RSDR , RSWR  , RCACHE,
   RLL     , RLWC1  , RSV  , RSV   , RLLD , RLDC1, RSV   , RLD   ,
   RSC     , RSWC1  , RSV  , RSV   , RSCD , RSDC1, RSV   , RSD
};

static int get_block_length(const precomp_block *block)
{
  return (block->end-block->start)/4;
}

static size_t get_block_memsize(const precomp_block *block)
{
  int length = get_block_length(block);
  return ((length+1)+(length>>2)) * sizeof(precomp_instr);
}

/**********************************************************************
 ******************** initialize an empty block ***********************
 **********************************************************************/
void init_block(int *source, precomp_block *block)
{
  int i, length, already_exist = 1;
  static int init_length;
  start_section(COMPILER_SECTION);
#ifdef CORE_DBG
  DebugMessage(M64MSG_INFO, "init block %x - %x", (int) block->start, (int) block->end);
#endif

  length = get_block_length(block);
   
  if (!block->block)
  {
    size_t memsize = get_block_memsize(block);
    if (r4300emu == CORE_DYNAREC) {
        block->block = (precomp_instr *) malloc_exec(memsize);
        if (!block->block) {
            DebugMessage(M64MSG_ERROR, "Memory error: couldn't allocate executable memory for dynamic recompiler. Try to use an interpreter mode.");
            return;
        }
    }
    else {
        block->block = (precomp_instr *) malloc(memsize);
        if (!block->block) {
            DebugMessage(M64MSG_ERROR, "Memory error: couldn't allocate memory for cached interpreter.");
            return;
        }
    }

    memset(block->block, 0, memsize);
    already_exist = 0;
  }

  if (r4300emu == CORE_DYNAREC)
  {
    if (!block->code)
    {
#if defined(PROFILE_R4300)
      max_code_length = 524288; /* allocate so much code space that we'll never have to realloc(), because this may */
                                /* cause instruction locations to move, and break our profiling data                */
#else
      max_code_length = 32768;
#endif
      block->code = (unsigned char *) malloc_exec(max_code_length);
    }
    else
    {
      max_code_length = block->max_code_length;
    }
    code_length = 0;
    inst_pointer = &block->code;
    
    if (block->jumps_table)
    {
      free(block->jumps_table);
      block->jumps_table = NULL;
    }
    if (block->riprel_table)
    {
      free(block->riprel_table);
      block->riprel_table = NULL;
    }
    init_assembler(NULL, 0, NULL, 0);
    init_cache(block->block);
  }
   
  if (!already_exist)
  {
#if defined(PROFILE_R4300)
    pfProfile = fopen("instructionaddrs.dat", "ab");
    long x86addr = (long) block->code;
    int mipsop = -2; /* -2 == NOTCOMPILED block at beginning of x86 code */
    if (fwrite(&mipsop, 1, 4, pfProfile) != 4 || // write 4-byte MIPS opcode
        fwrite(&x86addr, 1, sizeof(char *), pfProfile) != sizeof(char *)) // write pointer to dynamically generated x86 code for this MIPS instruction
        DebugMessage(M64MSG_ERROR, "Error writing R4300 instruction address profiling data");
#endif

    for (i=0; i<length; i++)
    {
      dst = block->block + i;
      dst->addr = block->start + i*4;
      dst->reg_cache_infos.need_map = 0;
      dst->local_addr = code_length;
#ifdef COMPARE_CORE
      if (r4300emu == CORE_DYNAREC) gendebug();
#endif
      RNOTCOMPILED();
    }
#if defined(PROFILE_R4300)
  fclose(pfProfile);
  pfProfile = NULL;
#endif
  init_length = code_length;
  }
  else
  {
#if defined(PROFILE_R4300)
    code_length = block->code_length; /* leave old instructions in their place */
#else
    code_length = init_length; /* recompile everything, overwrite old recompiled instructions */
#endif
    for (i=0; i<length; i++)
    {
      dst = block->block + i;
      dst->reg_cache_infos.need_map = 0;
      dst->local_addr = i * (init_length / length);
      dst->ops = NOTCOMPILED;
    }
  }
   
  if (r4300emu == CORE_DYNAREC)
  {
    free_all_registers();
    /* calling pass2 of the assembler is not necessary here because all of the code emitted by
       gennotcompiled() and gendebug() is position-independent and contains no jumps . */
    block->code_length = code_length;
    block->max_code_length = max_code_length;
    free_assembler(&block->jumps_table, &block->jumps_number, &block->riprel_table, &block->riprel_number);
  }
   
  /* here we're marking the block as a valid code even if it's not compiled
   * yet as the game should have already set up the code correctly.
   */
  invalid_code[block->start>>12] = 0;
  if (block->end < 0x80000000 || block->start >= 0xc0000000)
  { 
    unsigned int paddr;
    
    paddr = virtual_to_physical_address(block->start, 2);
    invalid_code[paddr>>12] = 0;
    if (!blocks[paddr>>12])
    {
      blocks[paddr>>12] = (precomp_block *) malloc(sizeof(precomp_block));
      blocks[paddr>>12]->code = NULL;
      blocks[paddr>>12]->block = NULL;
      blocks[paddr>>12]->jumps_table = NULL;
      blocks[paddr>>12]->riprel_table = NULL;
      blocks[paddr>>12]->start = paddr & ~0xFFF;
      blocks[paddr>>12]->end = (paddr & ~0xFFF) + 0x1000;
    }
    init_block(NULL, blocks[paddr>>12]);
    
    paddr += block->end - block->start - 4;
    invalid_code[paddr>>12] = 0;
    if (!blocks[paddr>>12])
    {
      blocks[paddr>>12] = (precomp_block *) malloc(sizeof(precomp_block));
      blocks[paddr>>12]->code = NULL;
      blocks[paddr>>12]->block = NULL;
      blocks[paddr>>12]->jumps_table = NULL;
      blocks[paddr>>12]->riprel_table = NULL;
      blocks[paddr>>12]->start = paddr & ~0xFFF;
      blocks[paddr>>12]->end = (paddr & ~0xFFF) + 0x1000;
    }
    init_block(NULL, blocks[paddr>>12]);
  }
  else
  {
    if (block->start >= 0x80000000 && block->end < 0xa0000000 && invalid_code[(block->start+0x20000000)>>12])
    {
      if (!blocks[(block->start+0x20000000)>>12])
      {
        blocks[(block->start+0x20000000)>>12] = (precomp_block *) malloc(sizeof(precomp_block));
        blocks[(block->start+0x20000000)>>12]->code = NULL;
        blocks[(block->start+0x20000000)>>12]->block = NULL;
        blocks[(block->start+0x20000000)>>12]->jumps_table = NULL;
        blocks[(block->start+0x20000000)>>12]->riprel_table = NULL;
        blocks[(block->start+0x20000000)>>12]->start = (block->start+0x20000000) & ~0xFFF;
        blocks[(block->start+0x20000000)>>12]->end = ((block->start+0x20000000) & ~0xFFF) + 0x1000;
      }
      init_block(NULL, blocks[(block->start+0x20000000)>>12]);
    }
    if (block->start >= 0xa0000000 && block->end < 0xc0000000 && invalid_code[(block->start-0x20000000)>>12])
    {
      if (!blocks[(block->start-0x20000000)>>12])
      {
        blocks[(block->start-0x20000000)>>12] = (precomp_block *) malloc(sizeof(precomp_block));
        blocks[(block->start-0x20000000)>>12]->code = NULL;
        blocks[(block->start-0x20000000)>>12]->block = NULL;
        blocks[(block->start-0x20000000)>>12]->jumps_table = NULL;
        blocks[(block->start-0x20000000)>>12]->riprel_table = NULL;
        blocks[(block->start-0x20000000)>>12]->start = (block->start-0x20000000) & ~0xFFF;
        blocks[(block->start-0x20000000)>>12]->end = ((block->start-0x20000000) & ~0xFFF) + 0x1000;
      }
      init_block(NULL, blocks[(block->start-0x20000000)>>12]);
    }
  }
  end_section(COMPILER_SECTION);
}

void free_block(precomp_block *block)
{
    size_t memsize = get_block_memsize(block);

    if (block->block) {
        if (r4300emu == CORE_DYNAREC)
            free_exec(block->block, memsize);
        else
            free(block->block);
        block->block = NULL;
    }
    if (block->code) { free_exec(block->code, block->max_code_length); block->code = NULL; }
    if (block->jumps_table) { free(block->jumps_table); block->jumps_table = NULL; }
    if (block->riprel_table) { free(block->riprel_table); block->riprel_table = NULL; }
}

/**********************************************************************
 ********************* recompile a block of code **********************
 **********************************************************************/
void recompile_block(int *source, precomp_block *block, unsigned int func)
{
   int i, length, finished=0;
   start_section(COMPILER_SECTION);
   length = (block->end-block->start)/4;
   dst_block = block;
   
   //for (i=0; i<16; i++) block->md5[i] = 0;
   block->adler32 = 0;
   
   if (r4300emu == CORE_DYNAREC)
     {
    code_length = block->code_length;
    max_code_length = block->max_code_length;
    inst_pointer = &block->code;
    init_assembler(block->jumps_table, block->jumps_number, block->riprel_table, block->riprel_number);
    init_cache(block->block + (func & 0xFFF) / 4);
     }

#if defined(PROFILE_R4300)
   pfProfile = fopen("instructionaddrs.dat", "ab");
#endif

   for (i = (func & 0xFFF) / 4; finished != 2; i++)
     {
    if(block->start < 0x80000000 || block->start >= 0xc0000000)
      {
          unsigned int address2 =
           virtual_to_physical_address(block->start + i*4, 0);
         if(blocks[address2>>12]->block[(address2&0xFFF)/4].ops == NOTCOMPILED)
           blocks[address2>>12]->block[(address2&0xFFF)/4].ops = NOTCOMPILED2;
      }
    
    SRC = source + i;
    src = source[i];
    if (!source[i+1]) check_nop = 1; else check_nop = 0;
    dst = block->block + i;
    dst->addr = block->start + i*4;
    dst->reg_cache_infos.need_map = 0;
    dst->local_addr = code_length;
#ifdef COMPARE_CORE
    if (r4300emu == CORE_DYNAREC) gendebug();
#endif
#if defined(PROFILE_R4300)
    long x86addr = (long) (block->code + block->block[i].local_addr);
    if (fwrite(source + i, 1, 4, pfProfile) != 4 || // write 4-byte MIPS opcode
        fwrite(&x86addr, 1, sizeof(char *), pfProfile) != sizeof(char *)) // write pointer to dynamically generated x86 code for this MIPS instruction
        DebugMessage(M64MSG_ERROR, "Error writing R4300 instruction address profiling data");
#endif
    recomp_ops[((src >> 26) & 0x3F)]();
    dst = block->block + i;

    /*if ((dst+1)->ops != NOTCOMPILED && !delay_slot_compiled &&
        i < length)
      {
         if (r4300emu == CORE_DYNAREC) genlink_subblock();
         finished = 2;
      }*/
    if (delay_slot_compiled) 
      {
         delay_slot_compiled--;
         free_all_registers();
      }
    
    if (i >= length-2+(length>>2)) finished = 2;
    if (i >= (length-1) && (block->start == 0xa4000000 ||
                block->start >= 0xc0000000 ||
                block->end   <  0x80000000)) finished = 2;
    if (dst->ops == ERET || finished == 1) finished = 2;
    if (/*i >= length &&*/ 
        (dst->ops == J || dst->ops == J_OUT || dst->ops == JR) &&
        !(i >= (length-1) && (block->start >= 0xc0000000 ||
                  block->end   <  0x80000000)))
      finished = 1;
     }

#if defined(PROFILE_R4300)
    long x86addr = (long) (block->code + code_length);
    int mipsop = -3; /* -3 == block-postfix */
    if (fwrite(&mipsop, 1, 4, pfProfile) != 4 || // write 4-byte MIPS opcode
        fwrite(&x86addr, 1, sizeof(char *), pfProfile) != sizeof(char *)) // write pointer to dynamically generated x86 code for this MIPS instruction
        DebugMessage(M64MSG_ERROR, "Error writing R4300 instruction address profiling data");
#endif

   if (i >= length)
     {
    dst = block->block + i;
    dst->addr = block->start + i*4;
    dst->reg_cache_infos.need_map = 0;
    dst->local_addr = code_length;
#ifdef COMPARE_CORE
    if (r4300emu == CORE_DYNAREC) gendebug();
#endif
    RFIN_BLOCK();
    i++;
    if (i < length-1+(length>>2)) // useful when last opcode is a jump
      {
         dst = block->block + i;
         dst->addr = block->start + i*4;
         dst->reg_cache_infos.need_map = 0;
         dst->local_addr = code_length;
#ifdef COMPARE_CORE
         if (r4300emu == CORE_DYNAREC) gendebug();
#endif
         RFIN_BLOCK();
         i++;
      }
     }
   else if (r4300emu == CORE_DYNAREC) genlink_subblock();

   if (r4300emu == CORE_DYNAREC)
     {
    free_all_registers();
    passe2(block->block, (func&0xFFF)/4, i, block);
    block->code_length = code_length;
    block->max_code_length = max_code_length;
    free_assembler(&block->jumps_table, &block->jumps_number, &block->riprel_table, &block->riprel_number);
     }
#ifdef CORE_DBG
   DebugMessage(M64MSG_INFO, "block recompiled (%x-%x)", (int)func, (int)(block->start+i*4));
#endif
#if defined(PROFILE_R4300)
   fclose(pfProfile);
   pfProfile = NULL;
#endif
   end_section(COMPILER_SECTION);
}

static int is_jump(void)
{
   int dyn=0;
   int jump=0;
   if(r4300emu == CORE_DYNAREC) dyn=1;
   if(dyn) r4300emu = CORE_INTERPRETER;
   recomp_ops[((src >> 26) & 0x3F)]();
   if(dst->ops == J ||
      dst->ops == J_OUT ||
      dst->ops == J_IDLE ||
      dst->ops == JAL ||
      dst->ops == JAL_OUT ||
      dst->ops == JAL_IDLE ||
      dst->ops == BEQ ||
      dst->ops == BEQ_OUT ||
      dst->ops == BEQ_IDLE ||
      dst->ops == BNE ||
      dst->ops == BNE_OUT ||
      dst->ops == BNE_IDLE ||
      dst->ops == BLEZ ||
      dst->ops == BLEZ_OUT ||
      dst->ops == BLEZ_IDLE ||
      dst->ops == BGTZ ||
      dst->ops == BGTZ_OUT ||
      dst->ops == BGTZ_IDLE ||
      dst->ops == BEQL ||
      dst->ops == BEQL_OUT ||
      dst->ops == BEQL_IDLE ||
      dst->ops == BNEL ||
      dst->ops == BNEL_OUT ||
      dst->ops == BNEL_IDLE ||
      dst->ops == BLEZL ||
      dst->ops == BLEZL_OUT ||
      dst->ops == BLEZL_IDLE ||
      dst->ops == BGTZL ||
      dst->ops == BGTZL_OUT ||
      dst->ops == BGTZL_IDLE ||
      dst->ops == JR ||
      dst->ops == JALR ||
      dst->ops == BLTZ ||
      dst->ops == BLTZ_OUT ||
      dst->ops == BLTZ_IDLE ||
      dst->ops == BGEZ ||
      dst->ops == BGEZ_OUT ||
      dst->ops == BGEZ_IDLE ||
      dst->ops == BLTZL ||
      dst->ops == BLTZL_OUT ||
      dst->ops == BLTZL_IDLE ||
      dst->ops == BGEZL ||
      dst->ops == BGEZL_OUT ||
      dst->ops == BGEZL_IDLE ||
      dst->ops == BLTZAL ||
      dst->ops == BLTZAL_OUT ||
      dst->ops == BLTZAL_IDLE ||
      dst->ops == BGEZAL ||
      dst->ops == BGEZAL_OUT ||
      dst->ops == BGEZAL_IDLE ||
      dst->ops == BLTZALL ||
      dst->ops == BLTZALL_OUT ||
      dst->ops == BLTZALL_IDLE ||
      dst->ops == BGEZALL ||
      dst->ops == BGEZALL_OUT ||
      dst->ops == BGEZALL_IDLE ||
      dst->ops == BC1F ||
      dst->ops == BC1F_OUT ||
      dst->ops == BC1F_IDLE ||
      dst->ops == BC1T ||
      dst->ops == BC1T_OUT ||
      dst->ops == BC1T_IDLE ||
      dst->ops == BC1FL ||
      dst->ops == BC1FL_OUT ||
      dst->ops == BC1FL_IDLE ||
      dst->ops == BC1TL ||
      dst->ops == BC1TL_OUT ||
      dst->ops == BC1TL_IDLE)
     jump = 1;
   if(dyn) r4300emu = CORE_DYNAREC;
   return jump;
}

/**********************************************************************
 ************ recompile only one opcode (use for delay slot) **********
 **********************************************************************/
void recompile_opcode(void)
{
   SRC++;
   src = *SRC;
   dst++;
   dst->addr = (dst-1)->addr + 4;
   dst->reg_cache_infos.need_map = 0;
   if(!is_jump())
   {
#if defined(PROFILE_R4300)
     long x86addr = (long) ((*inst_pointer) + code_length);
     if (fwrite(&src, 1, 4, pfProfile) != 4 || // write 4-byte MIPS opcode
         fwrite(&x86addr, 1, sizeof(char *), pfProfile) != sizeof(char *)) // write pointer to dynamically generated x86 code for this MIPS instruction
        DebugMessage(M64MSG_ERROR, "Error writing R4300 instruction address profiling data");
#endif
     recomp_ops[((src >> 26) & 0x3F)]();
   }
   else
   {
     RNOP();
   }
   delay_slot_compiled = 2;
}

/**********************************************************************
 ************** decode one opcode (for the interpreter) ***************
 **********************************************************************/
void prefetch_opcode(unsigned int op)
{
   dst = PC;
   src = op;
   recomp_ops[((src >> 26) & 0x3F)]();
}

/**********************************************************************
 ************** allocate memory with executable bit set ***************
 **********************************************************************/
void *malloc_exec(size_t size)
{
#if defined(WIN32)
	return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#elif defined(__GNUC__)

   #ifndef  MAP_ANONYMOUS
      #ifdef MAP_ANON
         #define MAP_ANONYMOUS MAP_ANON
      #endif
   #endif

   void *block = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if (block == MAP_FAILED)
       { DebugMessage(M64MSG_ERROR, "Memory error: couldn't allocate %zi byte block of aligned RWX memory.", size); return NULL; }

   return block;
#else
   return malloc(size);
#endif
}

/**********************************************************************
 ************* reallocate memory with executable bit set **************
 **********************************************************************/
void *realloc_exec(void *ptr, size_t oldsize, size_t newsize)
{
   void* block = malloc_exec(newsize);
   if (block != NULL)
   {
      size_t copysize;
      if (oldsize < newsize)
         copysize = oldsize;
      else
         copysize = newsize;
      memcpy(block, ptr, copysize);
   }
   free_exec(ptr, oldsize);
   return block;
}

/**********************************************************************
 **************** frees memory with executable bit set ****************
 **********************************************************************/
void free_exec(void *ptr, size_t length)
{
#if defined(WIN32)
	VirtualFree(ptr, 0, MEM_RELEASE);
#else
	munmap(ptr, length);
#endif
}
