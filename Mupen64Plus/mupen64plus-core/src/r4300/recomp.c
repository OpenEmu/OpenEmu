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
#ifndef __MINGW32__
#include <sys/mman.h>
#endif
#endif

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "memory/memory.h"

#include "recomp.h"
#include "recomph.h" //include for function prototypes
#include "macros.h"
#include "r4300.h"
#include "ops.h"

static void *malloc_exec(size_t size);
static void free_exec(void *ptr, size_t length);

// global variables :
precomp_instr *dst; // destination structure for the recompiled instruction
int code_length; // current real recompiled code length
int max_code_length; // current recompiled code's buffer length
unsigned char **inst_pointer; // output buffer for recompiled code
precomp_block *dst_block; // the current block that we are recompiling
int src; // the current recompiled instruction
int fast_memory;

static void (*recomp_func)(void); // pointer to the dynarec's generator
                                  // function for the latest decoded opcode

#if defined(PROFILE_R4300)
FILE *pfProfile;
#endif

static int *SRC; // currently recompiled instruction in the input stream
static int check_nop; // next instruction is nop ?
static int delay_slot_compiled = 0;



static void RSV(void)
{
   dst->ops = current_instruction_table.RESERVED;
   recomp_func = genreserved;
}

static void RFIN_BLOCK(void)
{
   dst->ops = current_instruction_table.FIN_BLOCK;
   recomp_func = genfin_block;
}

static void RNOTCOMPILED(void)
{
   dst->ops = current_instruction_table.NOTCOMPILED;
   recomp_func = gennotcompiled;
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
   dst->ops = current_instruction_table.NOP;
   recomp_func = gennop;
}

static void RSLL(void)
{
   dst->ops = current_instruction_table.SLL;
   recomp_func = gensll;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RSRL(void)
{
   dst->ops = current_instruction_table.SRL;
   recomp_func = gensrl;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RSRA(void)
{
   dst->ops = current_instruction_table.SRA;
   recomp_func = gensra;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RSLLV(void)
{
   dst->ops = current_instruction_table.SLLV;
   recomp_func = gensllv;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RSRLV(void)
{
   dst->ops = current_instruction_table.SRLV;
   recomp_func = gensrlv;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RSRAV(void)
{
   dst->ops = current_instruction_table.SRAV;
   recomp_func = gensrav;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RJR(void)
{
   dst->ops = current_instruction_table.JR;
   recomp_func = genjr;
   recompile_standard_i_type();
}

static void RJALR(void)
{
   dst->ops = current_instruction_table.JALR;
   recomp_func = genjalr;
   recompile_standard_r_type();
}

static void RSYSCALL(void)
{
   dst->ops = current_instruction_table.SYSCALL;
   recomp_func = gensyscall;
}

static void RBREAK(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RSYNC(void)
{
   dst->ops = current_instruction_table.SYNC;
   recomp_func = gensync;
}

static void RMFHI(void)
{
   dst->ops = current_instruction_table.MFHI;
   recomp_func = genmfhi;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RMTHI(void)
{
   dst->ops = current_instruction_table.MTHI;
   recomp_func = genmthi;
   recompile_standard_r_type();
}

static void RMFLO(void)
{
   dst->ops = current_instruction_table.MFLO;
   recomp_func = genmflo;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RMTLO(void)
{
   dst->ops = current_instruction_table.MTLO;
   recomp_func = genmtlo;
   recompile_standard_r_type();
}

static void RDSLLV(void)
{
   dst->ops = current_instruction_table.DSLLV;
   recomp_func = gendsllv;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RDSRLV(void)
{
   dst->ops = current_instruction_table.DSRLV;
   recomp_func = gendsrlv;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RDSRAV(void)
{
   dst->ops = current_instruction_table.DSRAV;
   recomp_func = gendsrav;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RMULT(void)
{
   dst->ops = current_instruction_table.MULT;
   recomp_func = genmult;
   recompile_standard_r_type();
}

static void RMULTU(void)
{
   dst->ops = current_instruction_table.MULTU;
   recomp_func = genmultu;
   recompile_standard_r_type();
}

static void RDIV(void)
{
   dst->ops = current_instruction_table.DIV;
   recomp_func = gendiv;
   recompile_standard_r_type();
}

static void RDIVU(void)
{
   dst->ops = current_instruction_table.DIVU;
   recomp_func = gendivu;
   recompile_standard_r_type();
}

static void RDMULT(void)
{
   dst->ops = current_instruction_table.DMULT;
   recomp_func = gendmult;
   recompile_standard_r_type();
}

static void RDMULTU(void)
{
   dst->ops = current_instruction_table.DMULTU;
   recomp_func = gendmultu;
   recompile_standard_r_type();
}

static void RDDIV(void)
{
   dst->ops = current_instruction_table.DDIV;
   recomp_func = genddiv;
   recompile_standard_r_type();
}

static void RDDIVU(void)
{
   dst->ops = current_instruction_table.DDIVU;
   recomp_func = genddivu;
   recompile_standard_r_type();
}

static void RADD(void)
{
   dst->ops = current_instruction_table.ADD;
   recomp_func = genadd;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RADDU(void)
{
   dst->ops = current_instruction_table.ADDU;
   recomp_func = genaddu;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RSUB(void)
{
   dst->ops = current_instruction_table.SUB;
   recomp_func = gensub;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RSUBU(void)
{
   dst->ops = current_instruction_table.SUBU;
   recomp_func = gensubu;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RAND(void)
{
   dst->ops = current_instruction_table.AND;
   recomp_func = genand;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
}

static void ROR(void)
{
   dst->ops = current_instruction_table.OR;
   recomp_func = genor;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
}

static void RXOR(void)
{
   dst->ops = current_instruction_table.XOR;
   recomp_func = genxor;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
}

static void RNOR(void)
{
   dst->ops = current_instruction_table.NOR;
   recomp_func = gennor;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
}

static void RSLT(void)
{
   dst->ops = current_instruction_table.SLT;
   recomp_func = genslt;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
}

static void RSLTU(void)
{
   dst->ops = current_instruction_table.SLTU;
   recomp_func = gensltu;
   recompile_standard_r_type();
   if(dst->f.r.rd == reg) RNOP();
}

static void RDADD(void)
{
   dst->ops = current_instruction_table.DADD;
   recomp_func = gendadd;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RDADDU(void)
{
   dst->ops = current_instruction_table.DADDU;
   recomp_func = gendaddu;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RDSUB(void)
{
   dst->ops = current_instruction_table.DSUB;
   recomp_func = gendsub;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RDSUBU(void)
{
   dst->ops = current_instruction_table.DSUBU;
   recomp_func = gendsubu;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RTGE(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RTGEU(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RTLT(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RTLTU(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RTEQ(void)
{
   dst->ops = current_instruction_table.TEQ;
   recomp_func = genteq;
   recompile_standard_r_type();
}

static void RTNE(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RDSLL(void)
{
   dst->ops = current_instruction_table.DSLL;
   recomp_func = gendsll;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RDSRL(void)
{
   dst->ops = current_instruction_table.DSRL;
   recomp_func = gendsrl;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RDSRA(void)
{
   dst->ops = current_instruction_table.DSRA;
   recomp_func = gendsra;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RDSLL32(void)
{
   dst->ops = current_instruction_table.DSLL32;
   recomp_func = gendsll32;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RDSRL32(void)
{
   dst->ops = current_instruction_table.DSRL32;
   recomp_func = gendsrl32;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
}

static void RDSRA32(void)
{
   dst->ops = current_instruction_table.DSRA32;
   recomp_func = gendsra32;
   recompile_standard_r_type();
   if (dst->f.r.rd == reg) RNOP();
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
   dst->ops = current_instruction_table.BLTZ;
   recomp_func = genbltz;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BLTZ_IDLE;
         recomp_func = genbltz_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BLTZ_OUT;
      recomp_func = genbltz_out;
   }
}

static void RBGEZ(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BGEZ;
   recomp_func = genbgez;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BGEZ_IDLE;
         recomp_func = genbgez_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BGEZ_OUT;
      recomp_func = genbgez_out;
   }
}

static void RBLTZL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BLTZL;
   recomp_func = genbltzl;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BLTZL_IDLE;
         recomp_func = genbltzl_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BLTZL_OUT;
      recomp_func = genbltzl_out;
   }
}

static void RBGEZL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BGEZL;
   recomp_func = genbgezl;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BGEZL_IDLE;
         recomp_func = genbgezl_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BGEZL_OUT;
      recomp_func = genbgezl_out;
   }
}

static void RTGEI(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RTGEIU(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RTLTI(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RTLTIU(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RTEQI(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RTNEI(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
}

static void RBLTZAL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BLTZAL;
   recomp_func = genbltzal;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BLTZAL_IDLE;
         recomp_func = genbltzal_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BLTZAL_OUT;
      recomp_func = genbltzal_out;
   }
}

static void RBGEZAL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BGEZAL;
   recomp_func = genbgezal;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BGEZAL_IDLE;
         recomp_func = genbgezal_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BGEZAL_OUT;
      recomp_func = genbgezal_out;
   }
}

static void RBLTZALL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BLTZALL;
   recomp_func = genbltzall;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BLTZALL_IDLE;
         recomp_func = genbltzall_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BLTZALL_OUT;
      recomp_func = genbltzall_out;
   }
}

static void RBGEZALL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BGEZALL;
   recomp_func = genbgezall;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BGEZALL_IDLE;
         recomp_func = genbgezall_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BGEZALL_OUT;
      recomp_func = genbgezall_out;
   }
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
   dst->ops = current_instruction_table.TLBR;
   recomp_func = gentlbr;
}

static void RTLBWI(void)
{
   dst->ops = current_instruction_table.TLBWI;
   recomp_func = gentlbwi;
}

static void RTLBWR(void)
{
   dst->ops = current_instruction_table.TLBWR;
   recomp_func = gentlbwr;
}

static void RTLBP(void)
{
   dst->ops = current_instruction_table.TLBP;
   recomp_func = gentlbp;
}

static void RERET(void)
{
   dst->ops = current_instruction_table.ERET;
   recomp_func = generet;
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
   dst->ops = current_instruction_table.MFC0;
   recomp_func = genmfc0;
   recompile_standard_r_type();
   dst->f.r.rd = (long long*)(reg_cop0 + ((src >> 11) & 0x1F));
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (dst->f.r.rt == reg) RNOP();
}

static void RMTC0(void)
{
   dst->ops = current_instruction_table.MTC0;
   recomp_func = genmtc0;
   recompile_standard_r_type();
   dst->f.r.nrd = (src >> 11) & 0x1F;
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
   dst->ops = current_instruction_table.BC1F;
   recomp_func = genbc1f;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BC1F_IDLE;
         recomp_func = genbc1f_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BC1F_OUT;
      recomp_func = genbc1f_out;
   }
}

static void RBC1T(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BC1T;
   recomp_func = genbc1t;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BC1T_IDLE;
         recomp_func = genbc1t_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BC1T_OUT;
      recomp_func = genbc1t_out;
   }
}

static void RBC1FL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BC1FL;
   recomp_func = genbc1fl;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BC1FL_IDLE;
         recomp_func = genbc1fl_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BC1FL_OUT;
      recomp_func = genbc1fl_out;
   }
}

static void RBC1TL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BC1TL;
   recomp_func = genbc1tl;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BC1TL_IDLE;
         recomp_func = genbc1tl_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BC1TL_OUT;
      recomp_func = genbc1tl_out;
   }
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
   dst->ops = current_instruction_table.ADD_S;
   recomp_func = genadd_s;
   recompile_standard_cf_type();
}

static void RSUB_S(void)
{
   dst->ops = current_instruction_table.SUB_S;
   recomp_func = gensub_s;
   recompile_standard_cf_type();
}

static void RMUL_S(void)
{
   dst->ops = current_instruction_table.MUL_S;
   recomp_func = genmul_s;
   recompile_standard_cf_type();
}

static void RDIV_S(void)
{
   dst->ops = current_instruction_table.DIV_S;
   recomp_func = gendiv_s;
   recompile_standard_cf_type();
}

static void RSQRT_S(void)
{
   dst->ops = current_instruction_table.SQRT_S;
   recomp_func = gensqrt_s;
   recompile_standard_cf_type();
}

static void RABS_S(void)
{
   dst->ops = current_instruction_table.ABS_S;
   recomp_func = genabs_s;
   recompile_standard_cf_type();
}

static void RMOV_S(void)
{
   dst->ops = current_instruction_table.MOV_S;
   recomp_func = genmov_s;
   recompile_standard_cf_type();
}

static void RNEG_S(void)
{
   dst->ops = current_instruction_table.NEG_S;
   recomp_func = genneg_s;
   recompile_standard_cf_type();
}

static void RROUND_L_S(void)
{
   dst->ops = current_instruction_table.ROUND_L_S;
   recomp_func = genround_l_s;
   recompile_standard_cf_type();
}

static void RTRUNC_L_S(void)
{
   dst->ops = current_instruction_table.TRUNC_L_S;
   recomp_func = gentrunc_l_s;
   recompile_standard_cf_type();
}

static void RCEIL_L_S(void)
{
   dst->ops = current_instruction_table.CEIL_L_S;
   recomp_func = genceil_l_s;
   recompile_standard_cf_type();
}

static void RFLOOR_L_S(void)
{
   dst->ops = current_instruction_table.FLOOR_L_S;
   recomp_func = genfloor_l_s;
   recompile_standard_cf_type();
}

static void RROUND_W_S(void)
{
   dst->ops = current_instruction_table.ROUND_W_S;
   recomp_func = genround_w_s;
   recompile_standard_cf_type();
}

static void RTRUNC_W_S(void)
{
   dst->ops = current_instruction_table.TRUNC_W_S;
   recomp_func = gentrunc_w_s;
   recompile_standard_cf_type();
}

static void RCEIL_W_S(void)
{
   dst->ops = current_instruction_table.CEIL_W_S;
   recomp_func = genceil_w_s;
   recompile_standard_cf_type();
}

static void RFLOOR_W_S(void)
{
   dst->ops = current_instruction_table.FLOOR_W_S;
   recomp_func = genfloor_w_s;
   recompile_standard_cf_type();
}

static void RCVT_D_S(void)
{
   dst->ops = current_instruction_table.CVT_D_S;
   recomp_func = gencvt_d_s;
   recompile_standard_cf_type();
}

static void RCVT_W_S(void)
{
   dst->ops = current_instruction_table.CVT_W_S;
   recomp_func = gencvt_w_s;
   recompile_standard_cf_type();
}

static void RCVT_L_S(void)
{
   dst->ops = current_instruction_table.CVT_L_S;
   recomp_func = gencvt_l_s;
   recompile_standard_cf_type();
}

static void RC_F_S(void)
{
   dst->ops = current_instruction_table.C_F_S;
   recomp_func = genc_f_s;
   recompile_standard_cf_type();
}

static void RC_UN_S(void)
{
   dst->ops = current_instruction_table.C_UN_S;
   recomp_func = genc_un_s;
   recompile_standard_cf_type();
}

static void RC_EQ_S(void)
{
   dst->ops = current_instruction_table.C_EQ_S;
   recomp_func = genc_eq_s;
   recompile_standard_cf_type();
}

static void RC_UEQ_S(void)
{
   dst->ops = current_instruction_table.C_UEQ_S;
   recomp_func = genc_ueq_s;
   recompile_standard_cf_type();
}

static void RC_OLT_S(void)
{
   dst->ops = current_instruction_table.C_OLT_S;
   recomp_func = genc_olt_s;
   recompile_standard_cf_type();
}

static void RC_ULT_S(void)
{
   dst->ops = current_instruction_table.C_ULT_S;
   recomp_func = genc_ult_s;
   recompile_standard_cf_type();
}

static void RC_OLE_S(void)
{
   dst->ops = current_instruction_table.C_OLE_S;
   recomp_func = genc_ole_s;
   recompile_standard_cf_type();
}

static void RC_ULE_S(void)
{
   dst->ops = current_instruction_table.C_ULE_S;
   recomp_func = genc_ule_s;
   recompile_standard_cf_type();
}

static void RC_SF_S(void)
{
   dst->ops = current_instruction_table.C_SF_S;
   recomp_func = genc_sf_s;
   recompile_standard_cf_type();
}

static void RC_NGLE_S(void)
{
   dst->ops = current_instruction_table.C_NGLE_S;
   recomp_func = genc_ngle_s;
   recompile_standard_cf_type();
}

static void RC_SEQ_S(void)
{
   dst->ops = current_instruction_table.C_SEQ_S;
   recomp_func = genc_seq_s;
   recompile_standard_cf_type();
}

static void RC_NGL_S(void)
{
   dst->ops = current_instruction_table.C_NGL_S;
   recomp_func = genc_ngl_s;
   recompile_standard_cf_type();
}

static void RC_LT_S(void)
{
   dst->ops = current_instruction_table.C_LT_S;
   recomp_func = genc_lt_s;
   recompile_standard_cf_type();
}

static void RC_NGE_S(void)
{
   dst->ops = current_instruction_table.C_NGE_S;
   recomp_func = genc_nge_s;
   recompile_standard_cf_type();
}

static void RC_LE_S(void)
{
   dst->ops = current_instruction_table.C_LE_S;
   recomp_func = genc_le_s;
   recompile_standard_cf_type();
}

static void RC_NGT_S(void)
{
   dst->ops = current_instruction_table.C_NGT_S;
   recomp_func = genc_ngt_s;
   recompile_standard_cf_type();
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
   dst->ops = current_instruction_table.ADD_D;
   recomp_func = genadd_d;
   recompile_standard_cf_type();
}

static void RSUB_D(void)
{
   dst->ops = current_instruction_table.SUB_D;
   recomp_func = gensub_d;
   recompile_standard_cf_type();
}

static void RMUL_D(void)
{
   dst->ops = current_instruction_table.MUL_D;
   recomp_func = genmul_d;
   recompile_standard_cf_type();
}

static void RDIV_D(void)
{
   dst->ops = current_instruction_table.DIV_D;
   recomp_func = gendiv_d;
   recompile_standard_cf_type();
}

static void RSQRT_D(void)
{
   dst->ops = current_instruction_table.SQRT_D;
   recomp_func = gensqrt_d;
   recompile_standard_cf_type();
}

static void RABS_D(void)
{
   dst->ops = current_instruction_table.ABS_D;
   recomp_func = genabs_d;
   recompile_standard_cf_type();
}

static void RMOV_D(void)
{
   dst->ops = current_instruction_table.MOV_D;
   recomp_func = genmov_d;
   recompile_standard_cf_type();
}

static void RNEG_D(void)
{
   dst->ops = current_instruction_table.NEG_D;
   recomp_func = genneg_d;
   recompile_standard_cf_type();
}

static void RROUND_L_D(void)
{
   dst->ops = current_instruction_table.ROUND_L_D;
   recomp_func = genround_l_d;
   recompile_standard_cf_type();
}

static void RTRUNC_L_D(void)
{
   dst->ops = current_instruction_table.TRUNC_L_D;
   recomp_func = gentrunc_l_d;
   recompile_standard_cf_type();
}

static void RCEIL_L_D(void)
{
   dst->ops = current_instruction_table.CEIL_L_D;
   recomp_func = genceil_l_d;
   recompile_standard_cf_type();
}

static void RFLOOR_L_D(void)
{
   dst->ops = current_instruction_table.FLOOR_L_D;
   recomp_func = genfloor_l_d;
   recompile_standard_cf_type();
}

static void RROUND_W_D(void)
{
   dst->ops = current_instruction_table.ROUND_W_D;
   recomp_func = genround_w_d;
   recompile_standard_cf_type();
}

static void RTRUNC_W_D(void)
{
   dst->ops = current_instruction_table.TRUNC_W_D;
   recomp_func = gentrunc_w_d;
   recompile_standard_cf_type();
}

static void RCEIL_W_D(void)
{
   dst->ops = current_instruction_table.CEIL_W_D;
   recomp_func = genceil_w_d;
   recompile_standard_cf_type();
}

static void RFLOOR_W_D(void)
{
   dst->ops = current_instruction_table.FLOOR_W_D;
   recomp_func = genfloor_w_d;
   recompile_standard_cf_type();
}

static void RCVT_S_D(void)
{
   dst->ops = current_instruction_table.CVT_S_D;
   recomp_func = gencvt_s_d;
   recompile_standard_cf_type();
}

static void RCVT_W_D(void)
{
   dst->ops = current_instruction_table.CVT_W_D;
   recomp_func = gencvt_w_d;
   recompile_standard_cf_type();
}

static void RCVT_L_D(void)
{
   dst->ops = current_instruction_table.CVT_L_D;
   recomp_func = gencvt_l_d;
   recompile_standard_cf_type();
}

static void RC_F_D(void)
{
   dst->ops = current_instruction_table.C_F_D;
   recomp_func = genc_f_d;
   recompile_standard_cf_type();
}

static void RC_UN_D(void)
{
   dst->ops = current_instruction_table.C_UN_D;
   recomp_func = genc_un_d;
   recompile_standard_cf_type();
}

static void RC_EQ_D(void)
{
   dst->ops = current_instruction_table.C_EQ_D;
   recomp_func = genc_eq_d;
   recompile_standard_cf_type();
}

static void RC_UEQ_D(void)
{
   dst->ops = current_instruction_table.C_UEQ_D;
   recomp_func = genc_ueq_d;
   recompile_standard_cf_type();
}

static void RC_OLT_D(void)
{
   dst->ops = current_instruction_table.C_OLT_D;
   recomp_func = genc_olt_d;
   recompile_standard_cf_type();
}

static void RC_ULT_D(void)
{
   dst->ops = current_instruction_table.C_ULT_D;
   recomp_func = genc_ult_d;
   recompile_standard_cf_type();
}

static void RC_OLE_D(void)
{
   dst->ops = current_instruction_table.C_OLE_D;
   recomp_func = genc_ole_d;
   recompile_standard_cf_type();
}

static void RC_ULE_D(void)
{
   dst->ops = current_instruction_table.C_ULE_D;
   recomp_func = genc_ule_d;
   recompile_standard_cf_type();
}

static void RC_SF_D(void)
{
   dst->ops = current_instruction_table.C_SF_D;
   recomp_func = genc_sf_d;
   recompile_standard_cf_type();
}

static void RC_NGLE_D(void)
{
   dst->ops = current_instruction_table.C_NGLE_D;
   recomp_func = genc_ngle_d;
   recompile_standard_cf_type();
}

static void RC_SEQ_D(void)
{
   dst->ops = current_instruction_table.C_SEQ_D;
   recomp_func = genc_seq_d;
   recompile_standard_cf_type();
}

static void RC_NGL_D(void)
{
   dst->ops = current_instruction_table.C_NGL_D;
   recomp_func = genc_ngl_d;
   recompile_standard_cf_type();
}

static void RC_LT_D(void)
{
   dst->ops = current_instruction_table.C_LT_D;
   recomp_func = genc_lt_d;
   recompile_standard_cf_type();
}

static void RC_NGE_D(void)
{
   dst->ops = current_instruction_table.C_NGE_D;
   recomp_func = genc_nge_d;
   recompile_standard_cf_type();
}

static void RC_LE_D(void)
{
   dst->ops = current_instruction_table.C_LE_D;
   recomp_func = genc_le_d;
   recompile_standard_cf_type();
}

static void RC_NGT_D(void)
{
   dst->ops = current_instruction_table.C_NGT_D;
   recomp_func = genc_ngt_d;
   recompile_standard_cf_type();
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
   dst->ops = current_instruction_table.CVT_S_W;
   recomp_func = gencvt_s_w;
   recompile_standard_cf_type();
}

static void RCVT_D_W(void)
{
   dst->ops = current_instruction_table.CVT_D_W;
   recomp_func = gencvt_d_w;
   recompile_standard_cf_type();
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
   dst->ops = current_instruction_table.CVT_S_L;
   recomp_func = gencvt_s_l;
   recompile_standard_cf_type();
}

static void RCVT_D_L(void)
{
   dst->ops = current_instruction_table.CVT_D_L;
   recomp_func = gencvt_d_l;
   recompile_standard_cf_type();
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
   dst->ops = current_instruction_table.MFC1;
   recomp_func = genmfc1;
   recompile_standard_r_type();
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (dst->f.r.rt == reg) RNOP();
}

static void RDMFC1(void)
{
   dst->ops = current_instruction_table.DMFC1;
   recomp_func = gendmfc1;
   recompile_standard_r_type();
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (dst->f.r.rt == reg) RNOP();
}

static void RCFC1(void)
{
   dst->ops = current_instruction_table.CFC1;
   recomp_func = gencfc1;
   recompile_standard_r_type();
   dst->f.r.nrd = (src >> 11) & 0x1F;
   if (dst->f.r.rt == reg) RNOP();
}

static void RMTC1(void)
{
   dst->ops = current_instruction_table.MTC1;
   recompile_standard_r_type();
   recomp_func = genmtc1;
   dst->f.r.nrd = (src >> 11) & 0x1F;
}

static void RDMTC1(void)
{
   dst->ops = current_instruction_table.DMTC1;
   recompile_standard_r_type();
   recomp_func = gendmtc1;
   dst->f.r.nrd = (src >> 11) & 0x1F;
}

static void RCTC1(void)
{
   dst->ops = current_instruction_table.CTC1;
   recompile_standard_r_type();
   recomp_func = genctc1;
   dst->f.r.nrd = (src >> 11) & 0x1F;
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
   dst->ops = current_instruction_table.J;
   recomp_func = genj;
   recompile_standard_j_type();
   target = (dst->f.j.inst_index<<2) | (dst->addr & 0xF0000000);
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.J_IDLE;
         recomp_func = genj_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.J_OUT;
      recomp_func = genj_out;
   }
}

static void RJAL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.JAL;
   recomp_func = genjal;
   recompile_standard_j_type();
   target = (dst->f.j.inst_index<<2) | (dst->addr & 0xF0000000);
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.JAL_IDLE;
         recomp_func = genjal_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.JAL_OUT;
      recomp_func = genjal_out;
   }
}

static void RBEQ(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BEQ;
   recomp_func = genbeq;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BEQ_IDLE;
         recomp_func = genbeq_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BEQ_OUT;
      recomp_func = genbeq_out;
   }
}

static void RBNE(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BNE;
   recomp_func = genbne;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BNE_IDLE;
         recomp_func = genbne_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BNE_OUT;
      recomp_func = genbne_out;
   }
}

static void RBLEZ(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BLEZ;
   recomp_func = genblez;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BLEZ_IDLE;
         recomp_func = genblez_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BLEZ_OUT;
      recomp_func = genblez_out;
   }
}

static void RBGTZ(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BGTZ;
   recomp_func = genbgtz;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BGTZ_IDLE;
         recomp_func = genbgtz_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BGTZ_OUT;
      recomp_func = genbgtz_out;
   }
}

static void RADDI(void)
{
   dst->ops = current_instruction_table.ADDI;
   recomp_func = genaddi;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RADDIU(void)
{
   dst->ops = current_instruction_table.ADDIU;
   recomp_func = genaddiu;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RSLTI(void)
{
   dst->ops = current_instruction_table.SLTI;
   recomp_func = genslti;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RSLTIU(void)
{
   dst->ops = current_instruction_table.SLTIU;
   recomp_func = gensltiu;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RANDI(void)
{
   dst->ops = current_instruction_table.ANDI;
   recomp_func = genandi;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RORI(void)
{
   dst->ops = current_instruction_table.ORI;
   recomp_func = genori;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
}

static void RXORI(void)
{
   dst->ops = current_instruction_table.XORI;
   recomp_func = genxori;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
}

static void RLUI(void)
{
   dst->ops = current_instruction_table.LUI;
   recomp_func = genlui;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
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
   dst->ops = current_instruction_table.BEQL;
   recomp_func = genbeql;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BEQL_IDLE;
         recomp_func = genbeql_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BEQL_OUT;
      recomp_func = genbeql_out;
   }
}

static void RBNEL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BNEL;
   recomp_func = genbnel;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BNEL_IDLE;
         recomp_func = genbnel_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BNEL_OUT;
      recomp_func = genbnel_out;
   }
}

static void RBLEZL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BLEZL;
   recomp_func = genblezl;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BLEZL_IDLE;
         recomp_func = genblezl_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BLEZL_OUT;
      recomp_func = genblezl_out;
   }
}

static void RBGTZL(void)
{
   unsigned int target;
   dst->ops = current_instruction_table.BGTZL;
   recomp_func = genbgtzl;
   recompile_standard_i_type();
   target = dst->addr + dst->f.i.immediate*4 + 4;
   if (target == dst->addr)
   {
      if (check_nop)
      {
         dst->ops = current_instruction_table.BGTZL_IDLE;
         recomp_func = genbgtzl_idle;
      }
   }
   else if (r4300emu != CORE_PURE_INTERPRETER && (target < dst_block->start || target >= dst_block->end || dst->addr == (dst_block->end-4)))
   {
      dst->ops = current_instruction_table.BGTZL_OUT;
      recomp_func = genbgtzl_out;
   }
}

static void RDADDI(void)
{
   dst->ops = current_instruction_table.DADDI;
   recomp_func = gendaddi;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RDADDIU(void)
{
   dst->ops = current_instruction_table.DADDIU;
   recomp_func = gendaddiu;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RLDL(void)
{
   dst->ops = current_instruction_table.LDL;
   recomp_func = genldl;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RLDR(void)
{
   dst->ops = current_instruction_table.LDR;
   recomp_func = genldr;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RLB(void)
{
   dst->ops = current_instruction_table.LB;
   recomp_func = genlb;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
}

static void RLH(void)
{
   dst->ops = current_instruction_table.LH;
   recomp_func = genlh;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
}

static void RLWL(void)
{
   dst->ops = current_instruction_table.LWL;
   recomp_func = genlwl;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
}

static void RLW(void)
{
   dst->ops = current_instruction_table.LW;
   recomp_func = genlw;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
}

static void RLBU(void)
{
   dst->ops = current_instruction_table.LBU;
   recomp_func = genlbu;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RLHU(void)
{
   dst->ops = current_instruction_table.LHU;
   recomp_func = genlhu;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RLWR(void)
{
   dst->ops = current_instruction_table.LWR;
   recomp_func = genlwr;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RLWU(void)
{
   dst->ops = current_instruction_table.LWU;
   recomp_func = genlwu;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RSB(void)
{
   dst->ops = current_instruction_table.SB;
   recomp_func = gensb;
   recompile_standard_i_type();
}

static void RSH(void)
{
   dst->ops = current_instruction_table.SH;
   recomp_func = gensh;
   recompile_standard_i_type();
}

static void RSWL(void)
{
   dst->ops = current_instruction_table.SWL;
   recomp_func = genswl;
   recompile_standard_i_type();
}

static void RSW(void)
{
   dst->ops = current_instruction_table.SW;
   recomp_func = gensw;
   recompile_standard_i_type();
}

static void RSDL(void)
{
   dst->ops = current_instruction_table.SDL;
   recomp_func = gensdl;
   recompile_standard_i_type();
}

static void RSDR(void)
{
   dst->ops = current_instruction_table.SDR;
   recomp_func = gensdr;
   recompile_standard_i_type();
}

static void RSWR(void)
{
   dst->ops = current_instruction_table.SWR;
   recomp_func = genswr;
   recompile_standard_i_type();
}

static void RCACHE(void)
{
   recomp_func = gencache;
   dst->ops = current_instruction_table.CACHE;
}

static void RLL(void)
{
   recomp_func = genll;
   dst->ops = current_instruction_table.LL;
   recompile_standard_i_type();
   if(dst->f.i.rt == reg) RNOP();
}

static void RLWC1(void)
{
   dst->ops = current_instruction_table.LWC1;
   recomp_func = genlwc1;
   recompile_standard_lf_type();
}

static void RLLD(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
   recompile_standard_i_type();
}

static void RLDC1(void)
{
   dst->ops = current_instruction_table.LDC1;
   recomp_func = genldc1;
   recompile_standard_lf_type();
}

static void RLD(void)
{
   dst->ops = current_instruction_table.LD;
   recomp_func = genld;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
}

static void RSC(void)
{
   dst->ops = current_instruction_table.SC;
   recomp_func = gensc;
   recompile_standard_i_type();
   if (dst->f.i.rt == reg) RNOP();
}

static void RSWC1(void)
{
   dst->ops = current_instruction_table.SWC1;
   recomp_func = genswc1;
   recompile_standard_lf_type();
}

static void RSCD(void)
{
   dst->ops = current_instruction_table.NI;
   recomp_func = genni;
   recompile_standard_i_type();
}

static void RSDC1(void)
{
   dst->ops = current_instruction_table.SDC1;
   recomp_func = gensdc1;
   recompile_standard_lf_type();
}

static void RSD(void)
{
   dst->ops = current_instruction_table.SD;
   recomp_func = gensd;
   recompile_standard_i_type();
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
void init_block(precomp_block *block)
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
      if (r4300emu == CORE_DYNAREC) recomp_func();
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
      dst->ops = current_instruction_table.NOTCOMPILED;
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
    init_block(blocks[paddr>>12]);
    
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
    init_block(blocks[paddr>>12]);
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
      init_block(blocks[(block->start+0x20000000)>>12]);
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
      init_block(blocks[(block->start-0x20000000)>>12]);
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
         if(blocks[address2>>12]->block[(address2&0xFFF)/4].ops == current_instruction_table.NOTCOMPILED)
           blocks[address2>>12]->block[(address2&0xFFF)/4].ops = current_instruction_table.NOTCOMPILED2;
      }
    
    SRC = source + i;
    src = source[i];
    check_nop = source[i+1] == 0;
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
    recomp_func = NULL;
    recomp_ops[((src >> 26) & 0x3F)]();
    if (r4300emu == CORE_DYNAREC) recomp_func();
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
    if (dst->ops == current_instruction_table.ERET || finished == 1) finished = 2;
    if (/*i >= length &&*/ 
        (dst->ops == current_instruction_table.J ||
         dst->ops == current_instruction_table.J_OUT ||
         dst->ops == current_instruction_table.JR) &&
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
    if (r4300emu == CORE_DYNAREC) recomp_func();
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
         if (r4300emu == CORE_DYNAREC) recomp_func();
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
   recomp_ops[((src >> 26) & 0x3F)]();
   return
      (dst->ops == current_instruction_table.J ||
       dst->ops == current_instruction_table.J_OUT ||
       dst->ops == current_instruction_table.J_IDLE ||
       dst->ops == current_instruction_table.JAL ||
       dst->ops == current_instruction_table.JAL_OUT ||
       dst->ops == current_instruction_table.JAL_IDLE ||
       dst->ops == current_instruction_table.BEQ ||
       dst->ops == current_instruction_table.BEQ_OUT ||
       dst->ops == current_instruction_table.BEQ_IDLE ||
       dst->ops == current_instruction_table.BNE ||
       dst->ops == current_instruction_table.BNE_OUT ||
       dst->ops == current_instruction_table.BNE_IDLE ||
       dst->ops == current_instruction_table.BLEZ ||
       dst->ops == current_instruction_table.BLEZ_OUT ||
       dst->ops == current_instruction_table.BLEZ_IDLE ||
       dst->ops == current_instruction_table.BGTZ ||
       dst->ops == current_instruction_table.BGTZ_OUT ||
       dst->ops == current_instruction_table.BGTZ_IDLE ||
       dst->ops == current_instruction_table.BEQL ||
       dst->ops == current_instruction_table.BEQL_OUT ||
       dst->ops == current_instruction_table.BEQL_IDLE ||
       dst->ops == current_instruction_table.BNEL ||
       dst->ops == current_instruction_table.BNEL_OUT ||
       dst->ops == current_instruction_table.BNEL_IDLE ||
       dst->ops == current_instruction_table.BLEZL ||
       dst->ops == current_instruction_table.BLEZL_OUT ||
       dst->ops == current_instruction_table.BLEZL_IDLE ||
       dst->ops == current_instruction_table.BGTZL ||
       dst->ops == current_instruction_table.BGTZL_OUT ||
       dst->ops == current_instruction_table.BGTZL_IDLE ||
       dst->ops == current_instruction_table.JR ||
       dst->ops == current_instruction_table.JALR ||
       dst->ops == current_instruction_table.BLTZ ||
       dst->ops == current_instruction_table.BLTZ_OUT ||
       dst->ops == current_instruction_table.BLTZ_IDLE ||
       dst->ops == current_instruction_table.BGEZ ||
       dst->ops == current_instruction_table.BGEZ_OUT ||
       dst->ops == current_instruction_table.BGEZ_IDLE ||
       dst->ops == current_instruction_table.BLTZL ||
       dst->ops == current_instruction_table.BLTZL_OUT ||
       dst->ops == current_instruction_table.BLTZL_IDLE ||
       dst->ops == current_instruction_table.BGEZL ||
       dst->ops == current_instruction_table.BGEZL_OUT ||
       dst->ops == current_instruction_table.BGEZL_IDLE ||
       dst->ops == current_instruction_table.BLTZAL ||
       dst->ops == current_instruction_table.BLTZAL_OUT ||
       dst->ops == current_instruction_table.BLTZAL_IDLE ||
       dst->ops == current_instruction_table.BGEZAL ||
       dst->ops == current_instruction_table.BGEZAL_OUT ||
       dst->ops == current_instruction_table.BGEZAL_IDLE ||
       dst->ops == current_instruction_table.BLTZALL ||
       dst->ops == current_instruction_table.BLTZALL_OUT ||
       dst->ops == current_instruction_table.BLTZALL_IDLE ||
       dst->ops == current_instruction_table.BGEZALL ||
       dst->ops == current_instruction_table.BGEZALL_OUT ||
       dst->ops == current_instruction_table.BGEZALL_IDLE ||
       dst->ops == current_instruction_table.BC1F ||
       dst->ops == current_instruction_table.BC1F_OUT ||
       dst->ops == current_instruction_table.BC1F_IDLE ||
       dst->ops == current_instruction_table.BC1T ||
       dst->ops == current_instruction_table.BC1T_OUT ||
       dst->ops == current_instruction_table.BC1T_IDLE ||
       dst->ops == current_instruction_table.BC1FL ||
       dst->ops == current_instruction_table.BC1FL_OUT ||
       dst->ops == current_instruction_table.BC1FL_IDLE ||
       dst->ops == current_instruction_table.BC1TL ||
       dst->ops == current_instruction_table.BC1TL_OUT ||
       dst->ops == current_instruction_table.BC1TL_IDLE);
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
     recomp_func = NULL;
     recomp_ops[((src >> 26) & 0x3F)]();
     if (r4300emu == CORE_DYNAREC) recomp_func();
   }
   else
   {
     RNOP();
     if (r4300emu == CORE_DYNAREC) recomp_func();
   }
   delay_slot_compiled = 2;
}

/**********************************************************************
 ************** decode one opcode (for the interpreter) ***************
 **********************************************************************/
void prefetch_opcode(unsigned int op, unsigned int nextop)
{
   dst = PC;
   src = op;
   check_nop = nextop == 0;
   recomp_ops[((src >> 26) & 0x3F)]();
}

/**********************************************************************
 ************** allocate memory with executable bit set ***************
 **********************************************************************/
static void *malloc_exec(size_t size)
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
static void free_exec(void *ptr, size_t length)
{
#if defined(WIN32)
   VirtualFree(ptr, 0, MEM_RELEASE);
#elif defined(__GNUC__)
   munmap(ptr, length);
#else
   free(ptr);
#endif
}
