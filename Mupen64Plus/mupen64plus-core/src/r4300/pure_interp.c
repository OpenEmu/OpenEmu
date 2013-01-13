/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - pure_interp.c                                           *
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
#include <math.h>

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "api/debugger.h"
#include "memory/memory.h"
#include "main/rom.h"
#include "osal/preproc.h"

#include "r4300.h"
#include "ops.h"
#include "exception.h"
#include "macros.h"
#include "interupt.h"

#ifdef DBG
#include "debugger/dbg_types.h"
#include "debugger/debugger.h"
#endif

static precomp_instr interp_PC;
unsigned int op;

static void prefetch(void);

#define PCADDR interp_PC.addr
#define ADD_TO_PC(x) interp_PC.addr += x*4;
#define DECLARE_INSTRUCTION(name) static void name(void)
#define DECLARE_JUMP(name, destination, condition, link, likely, cop1) \
   static void name(void) \
   { \
      const int take_jump = (condition); \
      const unsigned int jump_target = (destination); \
      long long int *link_register = (link); \
      if (cop1 && check_cop1_unusable()) return; \
      if (!likely || take_jump) \
      { \
        interp_PC.addr += 4; \
        delay_slot=1; \
        prefetch(); \
        PC->ops(); \
        update_count(); \
        delay_slot=0; \
        if (take_jump && !skip_jump) \
        { \
          if (link_register != &reg[0]) \
          { \
              *link_register=interp_PC.addr; \
              sign_extended(*link_register); \
          } \
          interp_PC.addr = jump_target; \
        } \
      } \
      else \
      { \
         interp_PC.addr += 8; \
         update_count(); \
      } \
      last_addr = interp_PC.addr; \
      if (next_interupt <= Count) gen_interupt(); \
   } \
   static void name##_IDLE(void) \
   { \
      const int take_jump = (condition); \
      int skip; \
      if (cop1 && check_cop1_unusable()) return; \
      if (take_jump) \
      { \
         update_count(); \
         skip = next_interupt - Count; \
         if (skip > 3) Count += (skip & 0xFFFFFFFC); \
         else name(); \
      } \
      else name(); \
   }
#define CHECK_MEMORY(x)

#include "interpreter.def"

// two functions are defined from the macros above but never used
// these prototype declarations will prevent a warning
#if defined(__GNUC__)
  void JR_IDLE(void) __attribute__((used));
  void JALR_IDLE(void) __attribute__((used));
#endif

static cpu_instruction_table pure_interpreter_table = {
   LB,
   LBU,
   LH,
   LHU,
   LW,
   LWL,
   LWR,
   SB,
   SH,
   SW,
   SWL,
   SWR,

   LD,
   LDL,
   LDR,
   LL,
   LWU,
   SC,
   SD,
   SDL,
   SDR,
   SYNC,

   ADDI,
   ADDIU,
   SLTI,
   SLTIU,
   ANDI,
   ORI,
   XORI,
   LUI,

   DADDI,
   DADDIU,

   ADD,
   ADDU,
   SUB,
   SUBU,
   SLT,
   SLTU,
   AND,
   OR,
   XOR,
   NOR,

   DADD,
   DADDU,
   DSUB,
   DSUBU,

   MULT,
   MULTU,
   DIV,
   DIVU,
   MFHI,
   MTHI,
   MFLO,
   MTLO,

   DMULT,
   DMULTU,
   DDIV,
   DDIVU,

   J,
   J, // _OUT (unused)
   J_IDLE,
   JAL,
   JAL, // _OUT (unused)
   JAL_IDLE,
   JR,
   JALR,
   BEQ,
   BEQ, // _OUT (unused)
   BEQ_IDLE,
   BNE,
   BNE, // _OUT (unused)
   BNE_IDLE,
   BLEZ,
   BLEZ, // _OUT (unused)
   BLEZ_IDLE,
   BGTZ,
   BGTZ, // _OUT (unused)
   BGTZ_IDLE,
   BLTZ,
   BLTZ, // _OUT (unused)
   BLTZ_IDLE,
   BGEZ,
   BGEZ, // _OUT (unused)
   BGEZ_IDLE,
   BLTZAL,
   BLTZAL, // _OUT (unused)
   BLTZAL_IDLE,
   BGEZAL,
   BGEZAL, // _OUT (unused)
   BGEZAL_IDLE,

   BEQL,
   BEQL, // _OUT (unused)
   BEQL_IDLE,
   BNEL,
   BNEL, // _OUT (unused)
   BNEL_IDLE,
   BLEZL,
   BLEZL, // _OUT (unused)
   BLEZL_IDLE,
   BGTZL,
   BGTZL, // _OUT (unused)
   BGTZL_IDLE,
   BLTZL,
   BLTZL, // _OUT (unused)
   BLTZL_IDLE,
   BGEZL,
   BGEZL, // _OUT (unused)
   BGEZL_IDLE,
   BLTZALL,
   BLTZALL, // _OUT (unused)
   BLTZALL_IDLE,
   BGEZALL,
   BGEZALL, // _OUT (unused)
   BGEZALL_IDLE,
   BC1TL,
   BC1TL, // _OUT (unused)
   BC1TL_IDLE,
   BC1FL,
   BC1FL, // _OUT (unused)
   BC1FL_IDLE,

   SLL,
   SRL,
   SRA,
   SLLV,
   SRLV,
   SRAV,

   DSLL,
   DSRL,
   DSRA,
   DSLLV,
   DSRLV,
   DSRAV,
   DSLL32,
   DSRL32,
   DSRA32,

   MTC0,
   MFC0,

   TLBR,
   TLBWI,
   TLBWR,
   TLBP,
   CACHE,
   ERET,

   LWC1,
   SWC1,
   MTC1,
   MFC1,
   CTC1,
   CFC1,
   BC1T,
   BC1T, // _OUT (unused)
   BC1T_IDLE,
   BC1F,
   BC1F, // _OUT (unused)
   BC1F_IDLE,

   DMFC1,
   DMTC1,
   LDC1,
   SDC1,

   CVT_S_D,
   CVT_S_W,
   CVT_S_L,
   CVT_D_S,
   CVT_D_W,
   CVT_D_L,
   CVT_W_S,
   CVT_W_D,
   CVT_L_S,
   CVT_L_D,

   ROUND_W_S,
   ROUND_W_D,
   ROUND_L_S,
   ROUND_L_D,

   TRUNC_W_S,
   TRUNC_W_D,
   TRUNC_L_S,
   TRUNC_L_D,

   CEIL_W_S,
   CEIL_W_D,
   CEIL_L_S,
   CEIL_L_D,

   FLOOR_W_S,
   FLOOR_W_D,
   FLOOR_L_S,
   FLOOR_L_D,

   ADD_S,
   ADD_D,

   SUB_S,
   SUB_D,

   MUL_S,
   MUL_D,

   DIV_S,
   DIV_D,
   
   ABS_S,
   ABS_D,

   MOV_S,
   MOV_D,

   NEG_S,
   NEG_D,

   SQRT_S,
   SQRT_D,

   C_F_S,
   C_F_D,
   C_UN_S,
   C_UN_D,
   C_EQ_S,
   C_EQ_D,
   C_UEQ_S,
   C_UEQ_D,
   C_OLT_S,
   C_OLT_D,
   C_ULT_S,
   C_ULT_D,
   C_OLE_S,
   C_OLE_D,
   C_ULE_S,
   C_ULE_D,
   C_SF_S,
   C_SF_D,
   C_NGLE_S,
   C_NGLE_D,
   C_SEQ_S,
   C_SEQ_D,
   C_NGL_S,
   C_NGL_D,
   C_LT_S,
   C_LT_D,
   C_NGE_S,
   C_NGE_D,
   C_LE_S,
   C_LE_D,
   C_NGT_S,
   C_NGT_D,

   SYSCALL,

   TEQ,

   NOP,
   RESERVED,
   NI,

   NULL, // FIN_BLOCK
   NULL, // NOTCOMPILED
   NULL, // NOTCOMPILED2
};

static void prefetch(void)
{
   unsigned int *mem = fast_mem_access(interp_PC.addr);
   if (mem != NULL)
   {
      prefetch_opcode(mem[0], mem[1]);
   }
   else
   {
      DebugMessage(M64MSG_ERROR, "prefetch() execute address :%x", PC->addr);
      stop=1;
   }
}

void pure_interpreter(void)
{
   stop=0;
   PC = &interp_PC;
   PC->addr = last_addr = 0xa4000040;

/*#ifdef DBG
         if (g_DebuggerActive)
           update_debugger(PC->addr);
#endif*/

   current_instruction_table = pure_interpreter_table;

   while (!stop)
   {
     prefetch();
#ifdef COMPARE_CORE
     CoreCompareCallback();
#endif
#ifdef DBG
     if (g_DebuggerActive) update_debugger(PC->addr);
#endif
     PC->ops();
   }
}
