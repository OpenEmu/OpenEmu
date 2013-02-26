/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - rjump.c                                                 *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2007 Richard Goedeken (Richard42)                       *
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

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "r4300/recomp.h"
#include "r4300/r4300.h"
#include "r4300/macros.h"
#include "r4300/ops.h"
#include "r4300/recomph.h"

// that's where the dynarec will restart when going back from a C function
static unsigned long long *return_address;

void dyna_jump(void)
{
    if (stop == 1)
    {
        dyna_stop();
        return;
    }

    if (PC->reg_cache_infos.need_map)
        *return_address = (unsigned long long) (PC->reg_cache_infos.jump_wrapper);
    else
        *return_address = (unsigned long long) (actual->code + PC->local_addr);
}

static long long save_rsp = 0;
static long long save_rip = 0;

void dyna_start(void *code)
{
  /* save the base and stack pointers */
  /* make a call and a pop to retrieve the instruction pointer and save it too */
  /* then call the code(), which should theoretically never return.  */
  /* When dyna_stop() sets the *return_address to the saved RIP, the emulator thread will come back here. */
  /* It will jump to label 2, restore the base and stack pointers, and exit this function */
  DebugMessage(M64MSG_INFO, "R4300: starting 64-bit dynamic recompiler at: %p", code);
#if defined(__GNUC__) && defined(__x86_64__)
  asm volatile
    (" push %%rbx              \n"  /* we must push an even # of registers to keep stack 16-byte aligned */
     " push %%r12              \n"
     " push %%r13              \n"
     " push %%r14              \n"
     " push %%r15              \n"
     " push %%rbp              \n"
     " mov  %%rsp, %[save_rsp] \n"
     " lea  %[reg], %%r15      \n" /* store the base location of the r4300 registers in r15 for addressing */
     " call 1f                 \n"
     " jmp 2f                  \n"
     "1:                       \n"
     " pop  %%rax              \n"
     " mov  %%rax, %[save_rip] \n"

     " sub $0x10, %%rsp        \n"
     " and $-16, %%rsp         \n" /* ensure that stack is 16-byte aligned */
     " mov %%rsp, %%rax        \n"
     " sub $8, %%rax           \n"
     " mov %%rax, %[return_address]\n"

     " call *%%rbx             \n"
     "2:                       \n"
     " mov  %[save_rsp], %%rsp \n"
     " pop  %%rbp              \n"
     " pop  %%r15              \n"
     " pop  %%r14              \n"
     " pop  %%r13              \n"
     " pop  %%r12              \n"
     " pop  %%rbx              \n"
     : [save_rsp]"=m"(save_rsp), [save_rip]"=m"(save_rip), [return_address]"=m"(return_address)
     : "b" (code), [reg]"m"(*reg)
     : "%rax", "memory"
     );
#endif

    /* clear the registers so we don't return here a second time; that would be a bug */
    save_rsp=0;
    save_rip=0;
}

void dyna_stop(void)
{
  if (save_rip == 0)
    DebugMessage(M64MSG_WARNING, "Instruction pointer is 0 at dyna_stop()");
  else
  {
    *return_address = (unsigned long long) save_rip;
  }
}

