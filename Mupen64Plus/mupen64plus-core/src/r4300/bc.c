/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - bc.c                                                    *
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

#include "r4300.h"
#include "ops.h"
#include "macros.h"
#include "interupt.h"

#ifdef DBG
  #include "debugger/dbg_types.h"
  #include "debugger/debugger.h"
#endif

void BC1F(void)
{
   if (check_cop1_unusable()) return;
   PC++;
   delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
   PC->ops();
   update_count();
   delay_slot=0;
   if ((FCR31 & 0x800000)==0 && !skip_jump)
     PC += (PC-2)->f.i.immediate-1;
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BC1F_OUT(void)
{
   if (check_cop1_unusable()) return;
   jump_target = (int)PC->f.i.immediate;
   PC++;
   delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
   PC->ops();
   update_count();
   delay_slot=0;
   if (!skip_jump && (FCR31 & 0x800000)==0)
     jump_to(PC->addr + ((jump_target-1)<<2));
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BC1F_IDLE(void)
{
   int skip;
   if ((FCR31 & 0x800000)==0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BC1F();
     }
   else BC1F();
}

void BC1T(void)
{
   if (check_cop1_unusable()) return;
   PC++;
   delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
   PC->ops();
   update_count();
   delay_slot=0;
   if ((FCR31 & 0x800000)!=0 && !skip_jump)
     PC += (PC-2)->f.i.immediate-1;
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BC1T_OUT(void)
{
   if (check_cop1_unusable()) return;
   jump_target = (int)PC->f.i.immediate;
   PC++;
   delay_slot=1;
#ifdef DBG
            if (g_DebuggerActive) update_debugger(PC->addr);
#endif
   PC->ops();
   update_count();
   delay_slot=0;
   if (!skip_jump && (FCR31 & 0x800000)!=0)
     jump_to(PC->addr + ((jump_target-1)<<2));
   last_addr = PC->addr;
   if (next_interupt <= Count) gen_interupt();
}

void BC1T_IDLE(void)
{
   int skip;
   if ((FCR31 & 0x800000)!=0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BC1T();
     }
   else BC1T();
}

void BC1FL(void)
{
   if (check_cop1_unusable()) return;
   if ((FCR31 & 0x800000)==0)
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

void BC1FL_OUT(void)
{
   if (check_cop1_unusable()) return;
   if ((FCR31 & 0x800000)==0)
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

void BC1FL_IDLE(void)
{
   int skip;
   if ((FCR31 & 0x800000)==0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BC1FL();
     }
   else BC1FL();
}

void BC1TL(void)
{
   if (check_cop1_unusable()) return;
   if ((FCR31 & 0x800000)!=0)
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

void BC1TL_OUT(void)
{
   if (check_cop1_unusable()) return;
   if ((FCR31 & 0x800000)!=0)
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

void BC1TL_IDLE(void)
{
   int skip;
   if ((FCR31 & 0x800000)!=0)
     {
    update_count();
    skip = next_interupt - Count;
    if (skip > 3) Count += (skip & 0xFFFFFFFC);
    else BC1TL();
     }
   else BC1TL();
}

