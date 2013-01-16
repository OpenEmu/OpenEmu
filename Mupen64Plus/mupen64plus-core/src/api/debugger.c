/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-core - api/debugger.c                                     *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Richard Goedeken                                   *
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
                       
/* This file contains the Core debugger functions which will be exported
 * outside of the core library.
 */
   
#include <stdlib.h>

#define M64P_CORE_PROTOTYPES 1
#include "m64p_types.h"
#include "m64p_debugger.h"
#include "callbacks.h"
#include "debugger.h"

#include "debugger/dbg_types.h"
#include "debugger/dbg_breakpoints.h"
#include "debugger/dbg_decoder.h"
#include "debugger/dbg_memory.h"
#include "debugger/debugger.h"
#include "memory/memory.h"
#include "r4300/r4300.h"

extern unsigned int op; /* this is in r4300/pure_interp.c */

/* local variables */
static void (*callback_ui_init)(void) = NULL;
static void (*callback_ui_update)(unsigned int) = NULL;
static void (*callback_ui_vi)(void) = NULL;

static void (*callback_core_compare)(unsigned int) = NULL;
static void (*callback_core_data_sync)(int, void *) = NULL;

/* global Functions for use by the Core */

void DebuggerCallback(eDbgCallbackType type, unsigned int param)
{
    if (type == DEBUG_UI_INIT)
    {
        if (callback_ui_init != NULL)
            (*callback_ui_init)();
    }
    else if (type == DEBUG_UI_UPDATE)
    {
        if (callback_ui_update != NULL)
            (*callback_ui_update)(param);
    }
    else if (type == DEBUG_UI_VI)
    {
        if (callback_ui_vi != NULL)
            (*callback_ui_vi)();
    }
}

void CoreCompareCallback(void)
{
    if (callback_core_compare != NULL)
        (*callback_core_compare)(op);
}

void CoreCompareDataSync(int length, void *ptr)
{
    if (callback_core_data_sync != NULL)
        (*callback_core_data_sync)(length, ptr);
}

/* exported functions for use by the front-end User Interface */

EXPORT m64p_error CALL DebugSetCoreCompare(void (*dbg_core_compare)(unsigned int), void (*dbg_core_data_sync)(int, void *))
{
    callback_core_compare = dbg_core_compare;
    callback_core_data_sync = dbg_core_data_sync;
    return M64ERR_SUCCESS;
}
 
EXPORT m64p_error CALL DebugSetCallbacks(void (*dbg_frontend_init)(void), void (*dbg_frontend_update)(unsigned int pc), void (*dbg_frontend_vi)(void))
{
#ifdef DBG
    callback_ui_init = dbg_frontend_init;
    callback_ui_update = dbg_frontend_update;
    callback_ui_vi = dbg_frontend_vi;
    return M64ERR_SUCCESS;
#else
    return M64ERR_UNSUPPORTED;
#endif
}

EXPORT m64p_error CALL DebugSetRunState(int runstate)
{
#ifdef DBG
    run = runstate; /* in debugger/debugger.c */
    return M64ERR_SUCCESS;
#else
    return M64ERR_UNSUPPORTED;
#endif
}

EXPORT int CALL DebugGetState(m64p_dbg_state statenum)
{
#ifdef DBG
    switch (statenum)
    {
        case M64P_DBG_RUN_STATE:
            return run;
        case M64P_DBG_PREVIOUS_PC:
            return previousPC;
        case M64P_DBG_NUM_BREAKPOINTS:
            return g_NumBreakpoints;
        case M64P_DBG_CPU_DYNACORE:
            return r4300emu;
        case M64P_DBG_CPU_NEXT_INTERRUPT:
            return next_interupt;
        default:
            DebugMessage(M64MSG_WARNING, "Bug: invalid m64p_dbg_state input in DebugGetState()");
            return 0;
    }
    return 0;
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugGetState() called, but Debugger not supported in Core library");
    return 0;
#endif
}

EXPORT m64p_error CALL DebugStep(void)
{
#ifdef DBG
    if (!g_DebuggerActive)
        return M64ERR_INVALID_STATE;
    debugger_step(); /* in debugger/debugger.c */
    return M64ERR_SUCCESS;
#else
    return M64ERR_UNSUPPORTED;
#endif
}

EXPORT void CALL DebugDecodeOp(unsigned int instruction, char *op, char *args, int pc)
{
#ifdef DBG
    r4300_decode_op(instruction, op, args, pc);
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugDecodeOp() called, but Debugger not supported in Core library");
#endif
}

EXPORT void * CALL DebugMemGetRecompInfo(m64p_dbg_mem_info recomp_type, unsigned int address, int index)
{
#ifdef DBG
    switch (recomp_type)
    {
        case M64P_DBG_RECOMP_OPCODE:
            return get_recompiled_opcode(address, index);
        case M64P_DBG_RECOMP_ARGS:
            return get_recompiled_args(address, index);
        case M64P_DBG_RECOMP_ADDR:
            return get_recompiled_addr(address, index);
        default:
            DebugMessage(M64MSG_ERROR, "Bug: DebugMemGetRecompInfo() called with invalid m64p_dbg_mem_info");
            return NULL;
    }
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugMemGetRecompInfo() called, but Debugger not supported in Core library");
    return NULL;
#endif
}

EXPORT int CALL DebugMemGetMemInfo(m64p_dbg_mem_info mem_info_type, unsigned int address)
{
#ifdef DBG
    switch (mem_info_type)
    {
        case M64P_DBG_MEM_TYPE:
            return get_memory_type(address);
        case M64P_DBG_MEM_FLAGS:
            return get_memory_flags(address);
        case M64P_DBG_MEM_HAS_RECOMPILED:
            return get_has_recompiled(address);
        case M64P_DBG_MEM_NUM_RECOMPILED:
            return get_num_recompiled(address);
        default:
            DebugMessage(M64MSG_ERROR, "Bug: DebugMemGetMemInfo() called with invalid m64p_dbg_mem_info");
            return 0;
    }
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugMemGetMemInfo() called, but Debugger not supported in Core library");
    return 0;
#endif
}

EXPORT void * CALL DebugMemGetPointer(m64p_dbg_memptr_type mem_ptr_type)
{
    switch (mem_ptr_type)
    {
        case M64P_DBG_PTR_RDRAM:
            return rdram;
        case M64P_DBG_PTR_PI_REG:
            return &pi_register;
        case M64P_DBG_PTR_SI_REG:
            return &si_register;
        case M64P_DBG_PTR_VI_REG:
            return &vi_register;
        case M64P_DBG_PTR_RI_REG:
            return &ri_register;
        case M64P_DBG_PTR_AI_REG:
            return &ai_register;
        default:
            DebugMessage(M64MSG_ERROR, "Bug: DebugMemGetPointer() called with invalid m64p_dbg_memptr_type");
            return NULL;
    }
}

EXPORT unsigned long long CALL DebugMemRead64(unsigned int address)
{
#ifdef DBG
    if ((address & 3) == 0)
        return read_memory_64(address);
    else
        return read_memory_64_unaligned(address);
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugMemRead64() called, but Debugger not supported in Core library");
    return 0LL;
#endif
}

EXPORT unsigned int CALL DebugMemRead32(unsigned int address)
{
#ifdef DBG
    if ((address & 3) == 0)
        return read_memory_32(address);
    else
        return read_memory_32_unaligned(address);
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugMemRead32() called, but Debugger not supported in Core library");
    return 0;
#endif
}

EXPORT unsigned short CALL DebugMemRead16(unsigned int address)
{
#ifdef DBG
    return read_memory_16(address);
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugMemRead16() called, but Debugger not supported in Core library");
    return 0;
#endif
}

EXPORT unsigned char CALL DebugMemRead8(unsigned int address)
{
#ifdef DBG
    return read_memory_8(address);
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugMemRead8() called, but Debugger not supported in Core library");
    return 0;
#endif
}

EXPORT void CALL DebugMemWrite64(unsigned int address, unsigned long long value)
{
#ifdef DBG
    if ((address & 3) == 0)
        write_memory_64(address, value);
    else
        write_memory_64_unaligned(address, value);
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugMemWrite64() called, but Debugger not supported in Core library");
#endif
}

EXPORT void CALL DebugMemWrite32(unsigned int address, unsigned int value)
{
#ifdef DBG
    if ((address & 3) == 0)
        write_memory_32(address, value);
    else
        write_memory_32_unaligned(address, value);
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugMemWrite32() called, but Debugger not supported in Core library");
#endif
}

EXPORT void CALL DebugMemWrite16(unsigned int address, unsigned short value)
{
#ifdef DBG
    write_memory_16(address, value);
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugMemWrite16() called, but Debugger not supported in Core library");
#endif
}

EXPORT void CALL DebugMemWrite8(unsigned int address, unsigned char value)
{
#ifdef DBG
    write_memory_8(address, value);
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugMemWrite8() called, but Debugger not supported in Core library");
#endif
}

EXPORT void * CALL DebugGetCPUDataPtr(m64p_dbg_cpu_data cpu_data_type)
{
    switch (cpu_data_type)
    {
        case M64P_CPU_PC:
            return &PC->addr;
        case M64P_CPU_REG_REG:
            return reg;
        case M64P_CPU_REG_HI:
            return &hi;
        case M64P_CPU_REG_LO:
            return &lo;
        case M64P_CPU_REG_COP0:
            return reg_cop0;
        case M64P_CPU_REG_COP1_DOUBLE_PTR:
            return reg_cop1_double;
        case M64P_CPU_REG_COP1_SIMPLE_PTR:
            return reg_cop1_simple;
        case M64P_CPU_REG_COP1_FGR_64:
            return reg_cop1_fgr_64;
        case M64P_CPU_TLB:
            return tlb_e;
        default:
            DebugMessage(M64MSG_ERROR, "Bug: DebugGetCPUDataPtr() called with invalid input m64p_dbg_cpu_data");
            return NULL;
    }
}

EXPORT int CALL DebugBreakpointLookup(unsigned int address, unsigned int size, unsigned int flags)
{
#ifdef DBG
    return lookup_breakpoint(address, size, flags);
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugBreakpointLookup() called, but Debugger not supported in Core library");
    return -1;
#endif
}

EXPORT int CALL DebugBreakpointCommand(m64p_dbg_bkp_command command, unsigned int index, void *ptr)
{
#ifdef DBG
    switch (command)
    {
        case M64P_BKP_CMD_ADD_ADDR:
            return add_breakpoint(index);
        case M64P_BKP_CMD_ADD_STRUCT:
            return add_breakpoint_struct((breakpoint *) ptr);
        case M64P_BKP_CMD_REPLACE:
            replace_breakpoint_num(index, (breakpoint *) ptr);
            return 0;
        case M64P_BKP_CMD_REMOVE_ADDR:
            remove_breakpoint_by_address(index);
            return 0;
        case M64P_BKP_CMD_REMOVE_IDX:
            remove_breakpoint_by_num(index);
            return 0;
        case M64P_BKP_CMD_ENABLE:
            enable_breakpoint(index);
            return 0;
        case M64P_BKP_CMD_DISABLE:
            disable_breakpoint(index);
            return 0;
        case M64P_BKP_CMD_CHECK:
            return check_breakpoints(index);
        default:
            DebugMessage(M64MSG_ERROR, "Bug: DebugBreakpointCommand() called with invalid input m64p_dbg_bkp_command");
            return -1;
    }
#else
    DebugMessage(M64MSG_ERROR, "Bug: DebugBreakpointCommand() called, but Debugger not supported in Core library");
    return -1;
#endif
}

