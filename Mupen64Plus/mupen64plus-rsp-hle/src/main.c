/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-rsp-hle - main.c                                          *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 Bobby Smiles                                       *
 *   Copyright (C) 2009 Richard Goedeken                                   *
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

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_types.h"
#include "m64p_common.h"
#include "m64p_plugin.h"
#include "hle.h"

/* global variables */
RSP_INFO rsp;

/* local variables */
static const int AudioHle = 0, GraphicsHle = 1;
static void (*l_DebugCallback)(void *, int, const char *) = NULL;
static void *l_DebugCallContext = NULL;
static int l_PluginInit = 0;

/* local functions */

/**
 * Simulate the effect of setting the TASKDONE bit (aliased to SIG2)
 * and executing a break instruction (setting HALT and BROKE bits).
 **/
static void taskdone()
{
    // On hardware writing to SP_STATUS_REG is an indirect way of changing its content.
    // For instance, in order to set the TASKDONE bit (bit 9), one should write 0x4000
    // to the SP_STATUS_REG : Read Access & Write Access don't have the same semantic.
    //
    // Here, this indirect way of changing the status register is bypassed :
    // we modify the bits directly.
    //
    // 0x203 = TASKDONE | BROKE | HALT
    *rsp.SP_STATUS_REG |= 0x203;
}


static int audio_ucode_detect(OSTask_t *task)
{
    if (*(unsigned int*)(rsp.RDRAM + task->ucode_data + 0) != 0x1)
    {
        if (*(rsp.RDRAM + task->ucode_data + (0 ^ (3-S8))) == 0xF)
            return 4;
        else
            return 3;
    }
    else
    {
        if (*(unsigned int*)(rsp.RDRAM + task->ucode_data + 0x30) == 0xF0000F00)
            return 1;
        else
            return 2;
    }
}

extern void (*ABI1[0x20])();
extern void (*ABI2[0x20])();
extern void (*ABI3[0x20])();

static void (*ABI[0x20])();

u32 inst1, inst2;

static int audio_ucode(OSTask_t *task)
{
    unsigned int *p_alist = (unsigned int*)(rsp.RDRAM + task->data_ptr);
    unsigned int i;

    switch(audio_ucode_detect(task))
    {
    case 1: // mario ucode
        memcpy( ABI, ABI1, sizeof(ABI[0])*0x20 );
        break;
    case 2: // banjo kazooie ucode
        memcpy( ABI, ABI2, sizeof(ABI[0])*0x20 );
        break;
    case 3: // zelda ucode
        memcpy( ABI, ABI3, sizeof(ABI[0])*0x20 );
        break;
    default:
        {
        DebugMessage(M64MSG_WARNING, "unknown audio ucode");
        return -1;
        }
    }

//  data = (short*)(rsp.RDRAM + task->ucode_data);

    for (i = 0; i < (task->data_size/4); i += 2)
    {
        inst1 = p_alist[i];
        inst2 = p_alist[i+1];
        ABI[inst1 >> 24]();
    }

    return 0;
}

/* Global functions */
void DebugMessage(int level, const char *message, ...)
{
    char msgbuf[1024];
    va_list args;

    if (l_DebugCallback == NULL)
        return;

    va_start(args, message);
    vsprintf(msgbuf, message, args);

    (*l_DebugCallback)(l_DebugCallContext, level, msgbuf);

    va_end(args);
}

/* DLL-exported functions */
EXPORT m64p_error CALL PluginStartup(m64p_dynlib_handle CoreLibHandle, void *Context,
                                   void (*DebugCallback)(void *, int, const char *))
{
    if (l_PluginInit)
        return M64ERR_ALREADY_INIT;

    /* first thing is to set the callback function for debug info */
    l_DebugCallback = DebugCallback;
    l_DebugCallContext = Context;

    /* this plugin doesn't use any Core library functions (ex for Configuration), so no need to keep the CoreLibHandle */

    l_PluginInit = 1;
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginShutdown(void)
{
    if (!l_PluginInit)
        return M64ERR_NOT_INIT;

    /* reset some local variable */
    l_DebugCallback = NULL;
    l_DebugCallContext = NULL;

    l_PluginInit = 0;
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL PluginGetVersion(m64p_plugin_type *PluginType, int *PluginVersion, int *APIVersion, const char **PluginNamePtr, int *Capabilities)
{
    /* set version info */
    if (PluginType != NULL)
        *PluginType = M64PLUGIN_RSP;

    if (PluginVersion != NULL)
        *PluginVersion = RSP_HLE_VERSION;

    if (APIVersion != NULL)
        *APIVersion = RSP_PLUGIN_API_VERSION;
    
    if (PluginNamePtr != NULL)
        *PluginNamePtr = "Hacktarux/Azimer High-Level Emulation RSP Plugin";

    if (Capabilities != NULL)
    {
        *Capabilities = 0;
    }
                    
    return M64ERR_SUCCESS;
}

EXPORT unsigned int CALL DoRspCycles(unsigned int Cycles)
{
    OSTask_t *task = (OSTask_t*)(rsp.DMEM + 0xFC0);
    unsigned int i, sum=0;

    if( task->type == 1 && task->data_ptr != 0 && GraphicsHle)
    {
        if (rsp.ProcessDlistList != NULL)
        {
            rsp.ProcessDlistList();
        }
        taskdone();
        if ((*rsp.SP_STATUS_REG & 0x40) != 0 )
        {
            *rsp.MI_INTR_REG |= 0x1;
            rsp.CheckInterrupts();
        }

        *rsp.DPC_STATUS_REG &= ~0x0002;
        return Cycles;
    }
    else if (task->type == 2 && AudioHle)
    {
        if (rsp.ProcessAlistList != NULL)
        {
            rsp.ProcessAlistList();
        }
        taskdone();
        if ((*rsp.SP_STATUS_REG & 0x40) != 0 )
        {
            *rsp.MI_INTR_REG |= 0x1;
            rsp.CheckInterrupts();
        }
        return Cycles;
    }
    else if (task->type == 7)
    {
        rsp.ShowCFB();
    }

    taskdone();
    if ((*rsp.SP_STATUS_REG & 0x40) != 0 )
    {
        *rsp.MI_INTR_REG |= 0x1;
        rsp.CheckInterrupts();
    }

    if (task->ucode_size <= 0x1000)
        for (i=0; i<(task->ucode_size/2); i++)
            sum += *(rsp.RDRAM + task->ucode + i);
    else
        for (i=0; i<(0x1000/2); i++)
            sum += *(rsp.IMEM + i);


    if (task->ucode_size > 0x1000)
    {
        switch(sum)
        {
        case 0x9E2: // banjo tooie (U) boot code
            {
            int i,j;
            memcpy(rsp.IMEM + 0x120, rsp.RDRAM + 0x1e8, 0x1e8);
            for (j=0; j<0xfc; j++)
                for (i=0; i<8; i++)
                    *(rsp.RDRAM+((0x2fb1f0+j*0xff0+i)^S8))=*(rsp.IMEM+((0x120+j*8+i)^S8));
            }
            return Cycles;
       case 0x9F2: // banjo tooie (E) + zelda oot (E) boot code
            {
            int i,j;
            memcpy(rsp.IMEM + 0x120, rsp.RDRAM + 0x1e8, 0x1e8);
            for (j=0; j<0xfc; j++)
                for (i=0; i<8; i++)
                    *(rsp.RDRAM+((0x2fb1f0+j*0xff0+i)^S8))=*(rsp.IMEM+((0x120+j*8+i)^S8));
            }
            return Cycles;
        }
    }
    else
    {
        switch(task->type)
        {
        case 2: // audio
            if (audio_ucode(task) == 0)
                return Cycles;
            break;
        case 4: // jpeg
            switch(sum)
            {
            case 0x278: // used by zelda during boot
                taskdone();
                return Cycles;
            case 0x2e4fc: // used by pokemon stadium {1,2} for jpg decompression
                ps_jpg_uncompress(task);
                taskdone();
                return Cycles;
            case 0x130de: // used by ogre battle for background decompression
                ob_jpg_uncompress(task);
                taskdone();
                return Cycles;
            default:
                DebugMessage(M64MSG_WARNING, "unknown jpeg task:  sum:%x", sum);
            }
            break;
        }
    }

    {
    FILE *f;
    DebugMessage(M64MSG_WARNING, "unknown task:  type:%d  sum:%x  PC:%lx", (int)task->type, sum, (unsigned long) rsp.SP_PC_REG);

    if (task->ucode_size <= 0x1000)
    {
        f = fopen("imem.dat", "wb");
        if (f == NULL || fwrite(rsp.RDRAM + task->ucode, 1, task->ucode_size, f) != task->ucode_size)
            DebugMessage(M64MSG_WARNING, "couldn't write to RSP debugging file imem.dat");
        fclose(f);

        f = fopen("dmem.dat", "wb");
        if (f == NULL || fwrite(rsp.RDRAM + task->ucode_data, 1, task->ucode_data_size, f) != task->ucode_data_size)
            DebugMessage(M64MSG_WARNING, "couldn't write to RSP debugging file dmem.dat");
        fclose(f);
    }
    else
    {
        f = fopen("imem.dat", "wb");
        if (f == NULL || fwrite(rsp.IMEM, 1, 0x1000, f) != 0x1000)
            DebugMessage(M64MSG_WARNING, "couldn't write to RSP debugging file imem.dat");
        fclose(f);

        f = fopen("dmem.dat", "wb");
        if (f == NULL || fwrite(rsp.DMEM, 1, 0x1000, f) != 0x1000)
            DebugMessage(M64MSG_WARNING, "couldn't write to RSP debugging file dmem.dat");
        fclose(f);
    }
    }

    return Cycles;
}

EXPORT void CALL InitiateRSP(RSP_INFO Rsp_Info, unsigned int *CycleCount)
{
    rsp = Rsp_Info;
}

EXPORT void CALL RomClosed(void)
{
    int i;
    for (i=0; i<0x1000; i++)
        rsp.DMEM[i] = rsp.IMEM[i] = 0;

    //init_ucode1();
    init_ucode2();
}

