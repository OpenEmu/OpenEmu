/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - special.c                                               *
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

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "memory/memory.h"

#include "r4300.h"
#include "interupt.h"
#include "ops.h"
#include "exception.h"
#include "macros.h"

#ifdef DBG
  #include "debugger/dbg_types.h"
  #include "debugger/debugger.h"
#endif

void NOP(void)
{
   PC++;
}

void SLL(void)
{
   rrd32 = (unsigned int)(rrt32) << rsa;
   sign_extended(rrd);
   PC++;
}

void SRL(void)
{
   rrd32 = (unsigned int)rrt32 >> rsa;
   sign_extended(rrd);
   PC++;
}

void SRA(void)
{
   rrd32 = (signed int)rrt32 >> rsa;
   sign_extended(rrd);
   PC++;
}

void SLLV(void)
{
   rrd32 = (unsigned int)(rrt32) << (rrs32&0x1F);
   sign_extended(rrd);
   PC++;
}

void SRLV(void)
{
   rrd32 = (unsigned int)rrt32 >> (rrs32 & 0x1F);
   sign_extended(rrd);
   PC++;
}

void SRAV(void)
{
   rrd32 = (signed int)rrt32 >> (rrs32 & 0x1F);
   sign_extended(rrd);
   PC++;
}

void JR(void)
{
   local_rs32 = irs32;
   PC++;
   delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
   PC->ops();
   update_count();
   delay_slot=0;
   jump_to(local_rs32);
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void JALR(void)
{
   unsigned long long *dest = (unsigned long long *) PC->f.r.rd;
   local_rs32 = rrs32;
   PC++;
   delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
   PC->ops();
   update_count();
   delay_slot=0;
   if (!skip_jump)
     {
    *dest = PC->addr;
    sign_extended(*dest);
    
    jump_to(local_rs32);
     }
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void SYSCALL(void)
{
   Cause = 8 << 2;
   exception_general();
}

void SYNC(void)
{
   PC++;
}

void MFHI(void)
{
   rrd = hi;
   PC++;
}

void MTHI(void)
{
   hi = rrs;
   PC++;
}

void MFLO(void)
{
   rrd = lo;
   PC++;
}

void MTLO(void)
{
   lo = rrs;
   PC++;
}

void DSLLV(void)
{
   rrd = rrt << (rrs32&0x3F);
   PC++;
}

void DSRLV(void)
{
   rrd = (unsigned long long)rrt >> (rrs32 & 0x3F);
   PC++;
}

void DSRAV(void)
{
   rrd = (long long)rrt >> (rrs32 & 0x3F);
   PC++;
}

void MULT(void)
{
   long long int temp;
   temp = rrs * rrt;
   hi = temp >> 32;
   lo = temp;
   sign_extended(lo);
   PC++;
}

void MULTU(void)
{
   unsigned long long int temp;
   temp = (unsigned int)rrs * (unsigned long long)((unsigned int)rrt);
   hi = (long long)temp >> 32;
   lo = temp;
   sign_extended(lo);
   PC++;
}

void DIV(void)
{
   if (rrt32)
   {
     lo = rrs32 / rrt32;
     hi = rrs32 % rrt32;
     sign_extended(lo);
     sign_extended(hi);
   }
   else DebugMessage(M64MSG_ERROR, "DIV: divide by 0");
   PC++;
}

void DIVU(void)
{
   if (rrt32)
   {
     lo = (unsigned int)rrs32 / (unsigned int)rrt32;
     hi = (unsigned int)rrs32 % (unsigned int)rrt32;
     sign_extended(lo);
     sign_extended(hi);
   }
   else DebugMessage(M64MSG_ERROR, "DIVU: divide by 0");
   PC++;
}

void DMULT(void)
{
   unsigned long long int op1, op2, op3, op4;
   unsigned long long int result1, result2, result3, result4;
   unsigned long long int temp1, temp2, temp3, temp4;
   int sign = 0;
   
   if (rrs < 0)
     {
    op2 = -rrs;
    sign = 1 - sign;
     }
   else op2 = rrs;
   if (rrt < 0)
     {
    op4 = -rrt;
    sign = 1 - sign;
     }
   else op4 = rrt;
   
   op1 = op2 & 0xFFFFFFFF;
   op2 = (op2 >> 32) & 0xFFFFFFFF;
   op3 = op4 & 0xFFFFFFFF;
   op4 = (op4 >> 32) & 0xFFFFFFFF;
   
   temp1 = op1 * op3;
   temp2 = (temp1 >> 32) + op1 * op4;
   temp3 = op2 * op3;
   temp4 = (temp3 >> 32) + op2 * op4;
   
   result1 = temp1 & 0xFFFFFFFF;
   result2 = temp2 + (temp3 & 0xFFFFFFFF);
   result3 = (result2 >> 32) + temp4;
   result4 = (result3 >> 32);
   
   lo = result1 | (result2 << 32);
   hi = (result3 & 0xFFFFFFFF) | (result4 << 32);
   if (sign)
     {
    hi = ~hi;
    if (!lo) hi++;
    else lo = ~lo + 1;
     }
   PC++;
}

void DMULTU(void)
{
   unsigned long long int op1, op2, op3, op4;
   unsigned long long int result1, result2, result3, result4;
   unsigned long long int temp1, temp2, temp3, temp4;
   
   op1 = rrs & 0xFFFFFFFF;
   op2 = (rrs >> 32) & 0xFFFFFFFF;
   op3 = rrt & 0xFFFFFFFF;
   op4 = (rrt >> 32) & 0xFFFFFFFF;
   
   temp1 = op1 * op3;
   temp2 = (temp1 >> 32) + op1 * op4;
   temp3 = op2 * op3;
   temp4 = (temp3 >> 32) + op2 * op4;
   
   result1 = temp1 & 0xFFFFFFFF;
   result2 = temp2 + (temp3 & 0xFFFFFFFF);
   result3 = (result2 >> 32) + temp4;
   result4 = (result3 >> 32);
   
   lo = result1 | (result2 << 32);
   hi = (result3 & 0xFFFFFFFF) | (result4 << 32);
   
   PC++;
}

void DDIV(void)
{
   if (rrt)
   {
     lo = (long long int)rrs / (long long int)rrt;
     hi = (long long int)rrs % (long long int)rrt;
   }
   else DebugMessage(M64MSG_ERROR, "DDIV: divide by 0");
   PC++;
}

void DDIVU(void)
{
   if (rrt)
   {
     lo = (unsigned long long int)rrs / (unsigned long long int)rrt;
     hi = (unsigned long long int)rrs % (unsigned long long int)rrt;
   }
   else DebugMessage(M64MSG_ERROR, "DDIVU: divide by 0");
   PC++;
}

void ADD(void)
{
   rrd32 = rrs32 + rrt32;
   sign_extended(rrd);
   PC++;
}

void ADDU(void)
{
   rrd32 = rrs32 + rrt32;
   sign_extended(rrd);
   PC++;
}

void SUB(void)
{
   rrd32 = rrs32 - rrt32;
   sign_extended(rrd);
   PC++;
}

void SUBU(void)
{
   rrd32 = rrs32 - rrt32;
   sign_extended(rrd);
   PC++;
}

void AND(void)
{
   rrd = rrs & rrt;
   PC++;
}

void OR(void)
{
   rrd = rrs | rrt;
   PC++;
}

void XOR(void)
{
   rrd = rrs ^ rrt;
   PC++;
}

void NOR(void)
{
   rrd = ~(rrs | rrt);
   PC++;
}

void SLT(void)
{
   if (rrs < rrt) rrd = 1;
   else rrd = 0;
   PC++;
}

void SLTU(void)
{
   if ((unsigned long long)rrs < (unsigned long long)rrt) 
     rrd = 1;
   else rrd = 0;
   PC++;
}

void DADD(void)
{
   rrd = rrs + rrt;
   PC++;
}

void DADDU(void)
{
   rrd = rrs + rrt;
   PC++;
}

void DSUB(void)
{
   rrd = rrs - rrt;
   PC++;
}

void DSUBU(void)
{
   rrd = rrs - rrt;
   PC++;
}

void TEQ(void)
{
   if (rrs == rrt)
   {
     DebugMessage(M64MSG_ERROR, "trap exception in TEQ");
     stop=1;
   }
   PC++;
}

void DSLL(void)
{
   rrd = rrt << rsa;
   PC++;
}

void DSRL(void)
{
   rrd = (unsigned long long)rrt >> rsa;
   PC++;
}

void DSRA(void)
{
   rrd = rrt >> rsa;
   PC++;
}

void DSLL32(void)
{
   rrd = rrt << (32+rsa);
   PC++;
}

void DSRL32(void)
{
   rrd = (unsigned long long int)rrt >> (32+rsa);
   PC++;
}

void DSRA32(void)
{
   rrd = (signed long long int)rrt >> (32+rsa);
   PC++;
}

