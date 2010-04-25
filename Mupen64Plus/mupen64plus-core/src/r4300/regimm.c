/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - regimm.h                                                *
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
#include "macros.h"

#ifdef DBG
  #include "debugger/dbg_types.h"
  #include "debugger/debugger.h"
#endif

void BLTZ(void)
{
   local_rs = irs;
   PC++;
   delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
   PC->ops();
   update_count();
   delay_slot=0;
   if (local_rs < 0 && !skip_jump)
     PC += (PC-2)->f.i.immediate-1;
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BLTZ_OUT(void)
{
   local_rs = irs;
   jump_target = (int)PC->f.i.immediate;
   PC++;
   delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
   PC->ops();
   update_count();
   delay_slot=0;
   if (!skip_jump && local_rs < 0)
     jump_to(PC->addr + ((jump_target-1)<<2));
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BLTZ_IDLE(void)
{
    int skip;
   if (irs < 0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BLTZ();
     }
   else BLTZ();
}

void BGEZ(void)
{
   local_rs = irs;
   PC++;
   delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
   PC->ops();
   update_count();
   delay_slot=0;
   if (local_rs >= 0 && !skip_jump)
     PC += (PC-2)->f.i.immediate - 1;
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BGEZ_OUT(void)
{
   local_rs = irs;
   jump_target = (int)PC->f.i.immediate;
   PC++;
   delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
   PC->ops();
   update_count();
   delay_slot=0;
   if (!skip_jump && local_rs >= 0)
     jump_to(PC->addr + ((jump_target-1)<<2));
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BGEZ_IDLE(void)
{
    int skip;
   if (irs >= 0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BGEZ();
     }
   else BGEZ();
}

void BLTZL(void)
{
   if (irs < 0)
     {
    PC++;
    delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
    PC->ops();
    update_count();
    delay_slot=0;
    if(!skip_jump)
      PC += (PC-2)->f.i.immediate-1;
     }
   else
     PC+=2;
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BLTZL_OUT(void)
{
   if (irs < 0)
     {
    jump_target = (int)PC->f.i.immediate;
    PC++;
    delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
    PC->ops();
    update_count();
    delay_slot=0;
    if (!skip_jump)
      jump_to(PC->addr + ((jump_target-1)<<2));
     }
   else
     PC+=2;
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BLTZL_IDLE(void)
{
   int skip;
   if (irs < 0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BLTZL();
     }
   else BLTZL();
}

void BGEZL(void)
{
   if (irs >= 0)
     {
    PC++;
    delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
    PC->ops();
    update_count();
    delay_slot=0;
    if(!skip_jump)
      PC += (PC-2)->f.i.immediate-1;
     }
   else
     PC+=2;
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BGEZL_OUT(void)
{
   if (irs >= 0)
     {
    jump_target = (int)PC->f.i.immediate;
    PC++;
    delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
    PC->ops();
    update_count();
    delay_slot=0;
    if (!skip_jump)
      jump_to(PC->addr + ((jump_target-1)<<2));
     }
   else
     PC+=2;
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BGEZL_IDLE(void)
{
   int skip;
   if (irs >= 0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BGEZL();
     }
   else BGEZL();
}

void BLTZAL(void)
{
   local_rs = irs;
   reg[31]=PC->addr+8;
   if((&irs)!=(reg+31))
     {
    PC++;
    delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
    PC->ops();
    update_count();
    delay_slot=0;
    if(local_rs < 0 && !skip_jump)
      PC += (PC-2)->f.i.immediate-1;
     }
   else DebugMessage(M64MSG_ERROR, "error in BLTZAL");
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BLTZAL_OUT(void)
{
   local_rs = irs;
   reg[31]=PC->addr+8;
   if((&irs)!=(reg+31))
     {
    jump_target = (int)PC->f.i.immediate;
    PC++;
    delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
    PC->ops();
    update_count();
    delay_slot=0;
    if(!skip_jump && local_rs < 0)
      jump_to(PC->addr + ((jump_target-1)<<2));
     }
   else DebugMessage(M64MSG_ERROR, "error in BLTZAL_OUT");
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BLTZAL_IDLE(void)
{
   int skip;
   if (irs < 0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BLTZAL();
     }
   else BLTZAL();
}

void BGEZAL(void)
{
   local_rs = irs;
   reg[31]=PC->addr+8;
   if((&irs)!=(reg+31))
     {
    PC++;
    delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
    PC->ops();
    update_count();
    delay_slot=0;
    if(local_rs >= 0 && !skip_jump)
      PC += (PC-2)->f.i.immediate-1;
     }
   else DebugMessage(M64MSG_ERROR, "error in BGEZAL");
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BGEZAL_OUT(void)
{
   local_rs = irs;
   reg[31]=PC->addr+8;
   if((&irs)!=(reg+31))
     {
    jump_target = (int)PC->f.i.immediate;
    PC++;
    delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
    PC->ops();
    update_count();
    delay_slot=0;
    if(!skip_jump && local_rs >= 0)
      jump_to(PC->addr + ((jump_target-1)<<2));
     }
   else DebugMessage(M64MSG_ERROR, "error in BGEZAL_OUT");
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BGEZAL_IDLE(void)
{
   int skip;
   if (irs >=0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BGEZAL();
     }
   else BGEZAL();
}

void BLTZALL(void)
{
   local_rs = irs;
   reg[31]=PC->addr+8;
   if((&irs)!=(reg+31))
     {
    if (local_rs < 0)
      {
         PC++;
         delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
         PC->ops();
         update_count();
         delay_slot=0;
         if(!skip_jump)
           PC += (PC-2)->f.i.immediate-1;
      }
    else
      PC+=2;
     }
   else DebugMessage(M64MSG_ERROR, "error in BLTZALL");
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BLTZALL_OUT(void)
{
   local_rs = irs;
   reg[31]=PC->addr+8;
   if((&irs)!=(reg+31))
     {
    if (local_rs < 0)
      {
         jump_target = (int)PC->f.i.immediate;
         PC++;
         delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
         PC->ops();
         update_count();
         delay_slot=0;
         if (!skip_jump)
           jump_to(PC->addr + ((jump_target-1)<<2));
      }
    else
      PC+=2;
     }
   else DebugMessage(M64MSG_ERROR, "error in BLTZALL_OUT");
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BLTZALL_IDLE(void)
{
   int skip;
   if (irs < 0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BLTZALL();
     }
   else BLTZALL();
}

void BGEZALL(void)
{
   local_rs = irs;
   reg[31]=PC->addr+8;
   if((&irs)!=(reg+31))
     {
    if (local_rs >= 0)
      {
         PC++;
         delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
         PC->ops();
         update_count();
         delay_slot=0;
         if(!skip_jump)
           PC += (PC-2)->f.i.immediate-1;
      }
    else
      PC+=2;
     }
   else DebugMessage(M64MSG_ERROR, "error in BGEZALL");
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BGEZALL_OUT(void)
{
   local_rs = irs;
   reg[31]=PC->addr+8;
   if((&irs)!=(reg+31))
     {
    if (local_rs >= 0)
      {
         jump_target = (int)PC->f.i.immediate;
         PC++;
         delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
         PC->ops();
         update_count();
         delay_slot=0;
         if (!skip_jump)
           jump_to(PC->addr + ((jump_target-1)<<2));
      }
    else
      PC+=2;
     }
   else DebugMessage(M64MSG_ERROR, "error in BGEZALL_OUT");
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BGEZALL_IDLE(void)
{
   int skip;
   if (irs >= 0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BGEZALL();
     }
   else BGEZALL();
}

