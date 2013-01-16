/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dbg_breakpoints.c                                       *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 DarkJeztr HyperHacker                              *
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

#include <SDL.h>
#include <SDL_thread.h>

#include "dbg_types.h"
#include "debugger.h"
#include "dbg_breakpoints.h"
#include "dbg_memory.h"

#include "api/m64p_types.h"
#include "api/callbacks.h"

int g_NumBreakpoints=0;
breakpoint g_Breakpoints[BREAKPOINTS_MAX_NUMBER];


int add_breakpoint( uint32 address )
{
    if( g_NumBreakpoints == BREAKPOINTS_MAX_NUMBER ) {
        DebugMessage(M64MSG_ERROR, "BREAKPOINTS_MAX_NUMBER have been reached.");
        return -1;
    }
    g_Breakpoints[g_NumBreakpoints].address=address;
    g_Breakpoints[g_NumBreakpoints].endaddr=address;
    BPT_SET_FLAG(g_Breakpoints[g_NumBreakpoints], BPT_FLAG_EXEC);

    enable_breakpoint(g_NumBreakpoints);

    return g_NumBreakpoints++;
}

int add_breakpoint_struct(breakpoint* newbp)
{
     if( g_NumBreakpoints == BREAKPOINTS_MAX_NUMBER ) {
        DebugMessage(M64MSG_ERROR, "BREAKPOINTS_MAX_NUMBER have been reached.");
        return -1;
    }

    memcpy(&g_Breakpoints[g_NumBreakpoints], newbp, sizeof(breakpoint));

    if(BPT_CHECK_FLAG(g_Breakpoints[g_NumBreakpoints], BPT_FLAG_ENABLED))
    {
        BPT_CLEAR_FLAG(g_Breakpoints[g_NumBreakpoints], BPT_FLAG_ENABLED);
        enable_breakpoint( g_NumBreakpoints );
    }
    
    return g_NumBreakpoints++;
}

void enable_breakpoint( int bpt)
{
    breakpoint *curBpt = g_Breakpoints + bpt;
    uint64 bptAddr;
    
    if(BPT_CHECK_FLAG((*curBpt), BPT_FLAG_READ)) {
        for(bptAddr = curBpt->address; bptAddr <= (curBpt->endaddr | 0xFFFF); bptAddr+=0x10000)
            if(lookup_breakpoint((uint32) bptAddr & 0xFFFF0000, 0x10000, BPT_FLAG_ENABLED | BPT_FLAG_READ) == -1)
                activate_memory_break_read((uint32) bptAddr);
    }

    if(BPT_CHECK_FLAG((*curBpt), BPT_FLAG_WRITE)) {
        for(bptAddr = curBpt->address; bptAddr <= (curBpt->endaddr | 0xFFFF); bptAddr+=0x10000)
            if(lookup_breakpoint((uint32) bptAddr & 0xFFFF0000, 0x10000, BPT_FLAG_ENABLED | BPT_FLAG_WRITE) == -1)
                activate_memory_break_write((uint32) bptAddr);
    }
    
    BPT_SET_FLAG(g_Breakpoints[bpt], BPT_FLAG_ENABLED);
}

void disable_breakpoint( int bpt )
{
    breakpoint *curBpt = g_Breakpoints + bpt;
    uint64 bptAddr;

    BPT_CLEAR_FLAG(g_Breakpoints[bpt], BPT_FLAG_ENABLED);

    if(BPT_CHECK_FLAG((*curBpt), BPT_FLAG_READ)) {
        for(bptAddr = curBpt->address; bptAddr <= ((unsigned long)(curBpt->endaddr | 0xFFFF)); bptAddr+=0x10000)
            if(lookup_breakpoint((uint32) bptAddr & 0xFFFF0000, 0x10000, BPT_FLAG_ENABLED | BPT_FLAG_READ) == -1)
                deactivate_memory_break_read((uint32) bptAddr);
    }

    if(BPT_CHECK_FLAG((*curBpt), BPT_FLAG_WRITE)) {
        for(bptAddr = curBpt->address; bptAddr <= ((unsigned long)(curBpt->endaddr | 0xFFFF)); bptAddr+=0x10000)
            if(lookup_breakpoint((uint32) bptAddr & 0xFFFF0000, 0x10000, BPT_FLAG_ENABLED | BPT_FLAG_WRITE) == -1)
                deactivate_memory_break_write((uint32) bptAddr);
    }

    BPT_CLEAR_FLAG(g_Breakpoints[bpt], BPT_FLAG_ENABLED);
}

void remove_breakpoint_by_num( int bpt )
{
    int curBpt;
    
    if(BPT_CHECK_FLAG(g_Breakpoints[bpt], BPT_FLAG_ENABLED))
        disable_breakpoint( bpt );

    for(curBpt=bpt+1; curBpt<g_NumBreakpoints; curBpt++)
        g_Breakpoints[curBpt-1]=g_Breakpoints[curBpt];
    
    g_NumBreakpoints--;
}

void remove_breakpoint_by_address( uint32 address )
{
    int bpt = lookup_breakpoint( address, 1, 0 );
    if(bpt==-1)
    {
        DebugMessage(M64MSG_ERROR, "Tried to remove Nonexistant breakpoint %x!", address);
    }
    else
        remove_breakpoint_by_num( bpt );
}

void replace_breakpoint_num( int bpt, breakpoint* copyofnew )
{
    
    if(BPT_CHECK_FLAG(g_Breakpoints[bpt], BPT_FLAG_ENABLED))
        disable_breakpoint( bpt );

    memcpy(&(g_Breakpoints[bpt]), copyofnew, sizeof(breakpoint));

    if(BPT_CHECK_FLAG(g_Breakpoints[bpt], BPT_FLAG_ENABLED))
    {
        BPT_CLEAR_FLAG(g_Breakpoints[bpt], BPT_FLAG_ENABLED);
        enable_breakpoint( bpt );
    }
}

int lookup_breakpoint( uint32 address, uint32 size, uint32 flags)
{
    int i;
    uint64 endaddr = ((uint64)address) + ((uint64)size) - 1;
    
    for( i=0; i < g_NumBreakpoints; i++)
    {
        if((g_Breakpoints[i].flags & flags) == flags)
        {
            if(g_Breakpoints[i].endaddr < g_Breakpoints[i].address)
            {
                if((endaddr >= g_Breakpoints[i].address) || 
                    (address <= g_Breakpoints[i].endaddr))
                        return i;
            }
            else // endaddr >= address
            {
                if((endaddr >= g_Breakpoints[i].address) && 
                    (address <= g_Breakpoints[i].endaddr))
                        return i;
            }
        }
    }
    return -1;
}

int check_breakpoints( uint32 address )
{
    return lookup_breakpoint( address, 1, BPT_FLAG_ENABLED | BPT_FLAG_EXEC );
}


int check_breakpoints_on_mem_access( uint32 pc, uint32 address, uint32 size, uint32 flags )
{
    //This function handles memory read/write breakpoints. size specifies the address
    //range to check, flags specifies the flags that all need to be set.
    //It automatically stops and updates the debugger on hit, so the memory access
    //functions only need to call it and can discard the result.
    int bpt;
    if(run == 2)
    {
        bpt=lookup_breakpoint( address, size, flags );
        if(bpt != -1)
        {
            if(BPT_CHECK_FLAG(g_Breakpoints[bpt], BPT_FLAG_LOG))
                log_breakpoint(pc, flags, address);
            
            run = 0;
            update_debugger(pc);
        
            return bpt;
        }
    }
    return -1;
}

int log_breakpoint(uint32 PC, uint32 Flag, uint32 Access)
{
    char msg[32];
    
    if(Flag & BPT_FLAG_READ) sprintf(msg, "0x%08X read 0x%08X", PC, Access);
    else if(Flag & BPT_FLAG_WRITE) sprintf(msg, "0x%08X wrote 0x%08X", PC, Access);
    else sprintf(msg, "0x%08X executed", PC);
    DebugMessage(M64MSG_INFO, "BPT: %s", msg);
    return 0;
}

