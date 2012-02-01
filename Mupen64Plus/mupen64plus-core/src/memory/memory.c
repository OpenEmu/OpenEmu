/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - memory.c                                                *
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

#include "api/m64p_types.h"

#include "memory.h"
#include "dma.h"
#include "pif.h"
#include "flashram.h"

#include "r4300/r4300.h"
#include "r4300/macros.h"
#include "r4300/interupt.h"
#include "r4300/recomph.h"
#include "r4300/ops.h"

#include "api/callbacks.h"
#include "main/main.h"
#include "main/rom.h"
#include "osal/preproc.h"
#include "plugin/plugin.h"

#ifdef DBG
#include "debugger/dbg_types.h"
#include "debugger/dbg_memory.h"
#include "debugger/dbg_breakpoints.h"
#endif

/* definitions of the rcp's structures and memory area */
RDRAM_register rdram_register;
mips_register MI_register;
PI_register pi_register;
SP_register sp_register;
RSP_register rsp_register;
SI_register si_register;
VI_register vi_register;
RI_register ri_register;
AI_register ai_register;
DPC_register dpc_register;
DPS_register dps_register;

ALIGN(16, unsigned int rdram[0x800000/4]);

unsigned char *rdramb = (unsigned char *)(rdram);
unsigned int SP_DMEM[0x1000/4*2];
unsigned int *SP_IMEM = SP_DMEM+0x1000/4;
unsigned char *SP_DMEMb = (unsigned char *)(SP_DMEM);
unsigned char *SP_IMEMb = (unsigned char*)(SP_DMEM+0x1000/4);
unsigned int PIF_RAM[0x40/4];
unsigned char *PIF_RAMb = (unsigned char *)(PIF_RAM);

// address : address of the read/write operation being done
unsigned int address = 0;
// *address_low = the lower 16 bit of the address :
#ifdef M64P_BIG_ENDIAN
static unsigned short *address_low = (unsigned short *)(&address)+1; 
#else
static unsigned short *address_low = (unsigned short *)(&address);
#endif

// values that are being written are stored in these variables
unsigned int word;
unsigned char cpu_byte;
unsigned short hword;
unsigned long long int dword;

// addresse where the read value will be stored
unsigned long long int* rdword;

// trash : when we write to unmaped memory it is written here
static unsigned int trash;

// hash tables of read functions
void (*readmem[0x10000])(void);
void (*readmemb[0x10000])(void);
void (*readmemh[0x10000])(void);
void (*readmemd[0x10000])(void);

// hash tables of write functions
void (*writemem[0x10000])(void);
void (*writememb[0x10000])(void);
void (*writememd[0x10000])(void);
void (*writememh[0x10000])(void);

// memory sections
static unsigned int *readrdramreg[0x10000];
static unsigned int *readrspreg[0x10000];
static unsigned int *readrsp[0x10000];
static unsigned int *readmi[0x10000];
static unsigned int *readvi[0x10000];
static unsigned int *readai[0x10000];
static unsigned int *readpi[0x10000];
static unsigned int *readri[0x10000];
static unsigned int *readsi[0x10000];
static unsigned int *readdp[0x10000];
static unsigned int *readdps[0x10000];

// the frameBufferInfos
static FrameBufferInfo frameBufferInfos[6];
static char framebufferRead[0x800];
static int firstFrameBufferSetting;

int init_memory(int DoByteSwap)
{
   int i;

   if (DoByteSwap != 0)
   {
    //swap rom
     unsigned int *roml = (unsigned int *) rom;
     for (i=0; i<(rom_size/4); i++) roml[i] = sl(roml[i]);
   }
   
   //init hash tables
   for (i=0; i<(0x10000); i++)
     {
    readmem[i] = read_nomem;
    readmemb[i] = read_nomemb;
    readmemd[i] = read_nomemd;
    readmemh[i] = read_nomemh;
    writemem[i] = write_nomem;
    writememb[i] = write_nomemb;
    writememd[i] = write_nomemd;
    writememh[i] = write_nomemh;
     }
   
   //init RDRAM
   for (i=0; i<(0x800000/4); i++) rdram[i]=0;
   for (i=0; i</*0x40*/0x80; i++) 
     {
    readmem[(0x8000+i)] = read_rdram;
    readmem[(0xa000+i)] = read_rdram;
    readmemb[(0x8000+i)] = read_rdramb;
    readmemb[(0xa000+i)] = read_rdramb;
    readmemh[(0x8000+i)] = read_rdramh;
    readmemh[(0xa000+i)] = read_rdramh;
    readmemd[(0x8000+i)] = read_rdramd;
    readmemd[(0xa000+i)] = read_rdramd;
    writemem[(0x8000+i)] = write_rdram;
    writemem[(0xa000+i)] = write_rdram;
    writememb[(0x8000+i)] = write_rdramb;
    writememb[(0xa000+i)] = write_rdramb;
    writememh[(0x8000+i)] = write_rdramh;
    writememh[(0xa000+i)] = write_rdramh;
    writememd[(0x8000+i)] = write_rdramd;
    writememd[(0xa000+i)] = write_rdramd;
     }
   
   for (i=/*0x40*/0x80; i<0x3F0; i++) 
     {
    readmem[0x8000+i] = read_nothing;
    readmem[0xa000+i] = read_nothing;
    readmemb[0x8000+i] = read_nothingb;
    readmemb[0xa000+i] = read_nothingb;
    readmemh[0x8000+i] = read_nothingh;
    readmemh[0xa000+i] = read_nothingh;
    readmemd[0x8000+i] = read_nothingd;
    readmemd[0xa000+i] = read_nothingd;
    writemem[0x8000+i] = write_nothing;
    writemem[0xa000+i] = write_nothing;
    writememb[0x8000+i] = write_nothingb;
    writememb[0xa000+i] = write_nothingb;
    writememh[0x8000+i] = write_nothingh;
    writememh[0xa000+i] = write_nothingh;
    writememd[0x8000+i] = write_nothingd;
    writememd[0xa000+i] = write_nothingd;
     }
   
   //init RDRAM registers
   readmem[0x83f0] = read_rdramreg;
   readmem[0xa3f0] = read_rdramreg;
   readmemb[0x83f0] = read_rdramregb;
   readmemb[0xa3f0] = read_rdramregb;
   readmemh[0x83f0] = read_rdramregh;
   readmemh[0xa3f0] = read_rdramregh;
   readmemd[0x83f0] = read_rdramregd;
   readmemd[0xa3f0] = read_rdramregd;
   writemem[0x83f0] = write_rdramreg;
   writemem[0xa3f0] = write_rdramreg;
   writememb[0x83f0] = write_rdramregb;
   writememb[0xa3f0] = write_rdramregb;
   writememh[0x83f0] = write_rdramregh;
   writememh[0xa3f0] = write_rdramregh;
   writememd[0x83f0] = write_rdramregd;
   writememd[0xa3f0] = write_rdramregd;
   rdram_register.rdram_config=0;
   rdram_register.rdram_device_id=0;
   rdram_register.rdram_delay=0;
   rdram_register.rdram_mode=0;
   rdram_register.rdram_ref_interval=0;
   rdram_register.rdram_ref_row=0;
   rdram_register.rdram_ras_interval=0;
   rdram_register.rdram_min_interval=0;
   rdram_register.rdram_addr_select=0;
   rdram_register.rdram_device_manuf=0;
   readrdramreg[0x0] = &rdram_register.rdram_config;
   readrdramreg[0x4] = &rdram_register.rdram_device_id;
   readrdramreg[0x8] = &rdram_register.rdram_delay;
   readrdramreg[0xc] = &rdram_register.rdram_mode;
   readrdramreg[0x10] = &rdram_register.rdram_ref_interval;
   readrdramreg[0x14] = &rdram_register.rdram_ref_row;
   readrdramreg[0x18] = &rdram_register.rdram_ras_interval;
   readrdramreg[0x1c] = &rdram_register.rdram_min_interval;
   readrdramreg[0x20] = &rdram_register.rdram_addr_select;
   readrdramreg[0x24] = &rdram_register.rdram_device_manuf;
   
   for (i=0x28; i<0x10000; i++) readrdramreg[i] = &trash;
   for (i=1; i<0x10; i++)
     {
    readmem[0x83f0+i] = read_nothing;
    readmem[0xa3f0+i] = read_nothing;
    readmemb[0x83f0+i] = read_nothingb;
    readmemb[0xa3f0+i] = read_nothingb;
    readmemh[0x83f0+i] = read_nothingh;
    readmemh[0xa3f0+i] = read_nothingh;
    readmemd[0x83f0+i] = read_nothingd;
    readmemd[0xa3f0+i] = read_nothingd;
    writemem[0x83f0+i] = write_nothing;
    writemem[0xa3f0+i] = write_nothing;
    writememb[0x83f0+i] = write_nothingb;
    writememb[0xa3f0+i] = write_nothingb;
    writememh[0x83f0+i] = write_nothingh;
    writememh[0xa3f0+i] = write_nothingh;
    writememd[0x83f0+i] = write_nothingd;
    writememd[0xa3f0+i] = write_nothingd;
     }
   
   //init RSP memory
   readmem[0x8400] = read_rsp_mem;
   readmem[0xa400] = read_rsp_mem;
   readmemb[0x8400] = read_rsp_memb;
   readmemb[0xa400] = read_rsp_memb;
   readmemh[0x8400] = read_rsp_memh;
   readmemh[0xa400] = read_rsp_memh;
   readmemd[0x8400] = read_rsp_memd;
   readmemd[0xa400] = read_rsp_memd;
   writemem[0x8400] = write_rsp_mem;
   writemem[0xa400] = write_rsp_mem;
   writememb[0x8400] = write_rsp_memb;
   writememb[0xa400] = write_rsp_memb;
   writememh[0x8400] = write_rsp_memh;
   writememh[0xa400] = write_rsp_memh;
   writememd[0x8400] = write_rsp_memd;
   writememd[0xa400] = write_rsp_memd;
   for (i=0; i<(0x1000/4); i++) SP_DMEM[i]=0;
   for (i=0; i<(0x1000/4); i++) SP_IMEM[i]=0;
   
   for (i=1; i<0x4; i++)
     {
    readmem[0x8400+i] = read_nothing;
    readmem[0xa400+i] = read_nothing;
    readmemb[0x8400+i] = read_nothingb;
    readmemb[0xa400+i] = read_nothingb;
    readmemh[0x8400+i] = read_nothingh;
    readmemh[0xa400+i] = read_nothingh;
    readmemd[0x8400+i] = read_nothingd;
    readmemd[0xa400+i] = read_nothingd;
    writemem[0x8400+i] = write_nothing;
    writemem[0xa400+i] = write_nothing;
    writememb[0x8400+i] = write_nothingb;
    writememb[0xa400+i] = write_nothingb;
    writememh[0x8400+i] = write_nothingh;
    writememh[0xa400+i] = write_nothingh;
    writememd[0x8400+i] = write_nothingd;
    writememd[0xa400+i] = write_nothingd;
     }
   
   //init RSP registers
   readmem[0x8404] = read_rsp_reg;
   readmem[0xa404] = read_rsp_reg;
   readmemb[0x8404] = read_rsp_regb;
   readmemb[0xa404] = read_rsp_regb;
   readmemh[0x8404] = read_rsp_regh;
   readmemh[0xa404] = read_rsp_regh;
   readmemd[0x8404] = read_rsp_regd;
   readmemd[0xa404] = read_rsp_regd;
   writemem[0x8404] = write_rsp_reg;
   writemem[0xa404] = write_rsp_reg;
   writememb[0x8404] = write_rsp_regb;
   writememb[0xa404] = write_rsp_regb;
   writememh[0x8404] = write_rsp_regh;
   writememh[0xa404] = write_rsp_regh;
   writememd[0x8404] = write_rsp_regd;
   writememd[0xa404] = write_rsp_regd;
   sp_register.sp_mem_addr_reg=0;
   sp_register.sp_dram_addr_reg=0;
   sp_register.sp_rd_len_reg=0;
   sp_register.sp_wr_len_reg=0;
   sp_register.sp_status_reg=1;
   sp_register.w_sp_status_reg=0;
   sp_register.halt=1;
   sp_register.broke=0;
   sp_register.dma_busy=0;
   sp_register.dma_full=0;
   sp_register.io_full=0;
   sp_register.single_step=0;
   sp_register.intr_break=0;
   sp_register.signal0=0;
   sp_register.signal1=0;
   sp_register.signal2=0;
   sp_register.signal3=0;
   sp_register.signal4=0;
   sp_register.signal5=0;
   sp_register.signal6=0;
   sp_register.signal7=0;
   sp_register.sp_dma_full_reg=0;
   sp_register.sp_dma_busy_reg=0;
   sp_register.sp_semaphore_reg=0;
   readrspreg[0x0] = &sp_register.sp_mem_addr_reg;
   readrspreg[0x4] = &sp_register.sp_dram_addr_reg;
   readrspreg[0x8] = &sp_register.sp_rd_len_reg;
   readrspreg[0xc] = &sp_register.sp_wr_len_reg;
   readrspreg[0x10] = &sp_register.sp_status_reg;
   readrspreg[0x14] = &sp_register.sp_dma_full_reg;
   readrspreg[0x18] = &sp_register.sp_dma_busy_reg;
   readrspreg[0x1c] = &sp_register.sp_semaphore_reg;
   
   for (i=0x20; i<0x10000; i++) readrspreg[i] = &trash;
   for (i=5; i<8; i++)
     {
    readmem[0x8400+i] = read_nothing;
    readmem[0xa400+i] = read_nothing;
    readmemb[0x8400+i] = read_nothingb;
    readmemb[0xa400+i] = read_nothingb;
    readmemh[0x8400+i] = read_nothingh;
    readmemh[0xa400+i] = read_nothingh;
    readmemd[0x8400+i] = read_nothingd;
    readmemd[0xa400+i] = read_nothingd;
    writemem[0x8400+i] = write_nothing;
    writemem[0xa400+i] = write_nothing;
    writememb[0x8400+i] = write_nothingb;
    writememb[0xa400+i] = write_nothingb;
    writememh[0x8400+i] = write_nothingh;
    writememh[0xa400+i] = write_nothingh;
    writememd[0x8400+i] = write_nothingd;
    writememd[0xa400+i] = write_nothingd;
     }
   
   readmem[0x8408] = read_rsp;
   readmem[0xa408] = read_rsp;
   readmemb[0x8408] = read_rspb;
   readmemb[0xa408] = read_rspb;
   readmemh[0x8408] = read_rsph;
   readmemh[0xa408] = read_rsph;
   readmemd[0x8408] = read_rspd;
   readmemd[0xa408] = read_rspd;
   writemem[0x8408] = write_rsp;
   writemem[0xa408] = write_rsp;
   writememb[0x8408] = write_rspb;
   writememb[0xa408] = write_rspb;
   writememh[0x8408] = write_rsph;
   writememh[0xa408] = write_rsph;
   writememd[0x8408] = write_rspd;
   writememd[0xa408] = write_rspd;
   rsp_register.rsp_pc=0;
   rsp_register.rsp_ibist=0;
   readrsp[0x0] = &rsp_register.rsp_pc;
   readrsp[0x4] = &rsp_register.rsp_ibist;
   
   for (i=0x8; i<0x10000; i++) readrsp[i] = &trash;
   for (i=9; i<0x10; i++)
     {
    readmem[0x8400+i] = read_nothing;
    readmem[0xa400+i] = read_nothing;
    readmemb[0x8400+i] = read_nothingb;
    readmemb[0xa400+i] = read_nothingb;
    readmemh[0x8400+i] = read_nothingh;
    readmemh[0xa400+i] = read_nothingh;
    readmemd[0x8400+i] = read_nothingd;
    readmemd[0xa400+i] = read_nothingd;
    writemem[0x8400+i] = write_nothing;
    writemem[0xa400+i] = write_nothing;
    writememb[0x8400+i] = write_nothingb;
    writememb[0xa400+i] = write_nothingb;
    writememh[0x8400+i] = write_nothingh;
    writememh[0xa400+i] = write_nothingh;
    writememd[0x8400+i] = write_nothingd;
    writememd[0xa400+i] = write_nothingd;
     }
   
   //init rdp command registers
   readmem[0x8410] = read_dp;
   readmem[0xa410] = read_dp;
   readmemb[0x8410] = read_dpb;
   readmemb[0xa410] = read_dpb;
   readmemh[0x8410] = read_dph;
   readmemh[0xa410] = read_dph;
   readmemd[0x8410] = read_dpd;
   readmemd[0xa410] = read_dpd;
   writemem[0x8410] = write_dp;
   writemem[0xa410] = write_dp;
   writememb[0x8410] = write_dpb;
   writememb[0xa410] = write_dpb;
   writememh[0x8410] = write_dph;
   writememh[0xa410] = write_dph;
   writememd[0x8410] = write_dpd;
   writememd[0xa410] = write_dpd;
   dpc_register.dpc_start=0;
   dpc_register.dpc_end=0;
   dpc_register.dpc_current=0;
   dpc_register.w_dpc_status=0;
   dpc_register.dpc_status=0;
   dpc_register.xbus_dmem_dma=0;
   dpc_register.freeze=0;
   dpc_register.flush=0;
   dpc_register.start_glck=0;
   dpc_register.tmem_busy=0;
   dpc_register.pipe_busy=0;
   dpc_register.cmd_busy=0;
   dpc_register.cbuf_busy=0;
   dpc_register.dma_busy=0;
   dpc_register.end_valid=0;
   dpc_register.start_valid=0;
   dpc_register.dpc_clock=0;
   dpc_register.dpc_bufbusy=0;
   dpc_register.dpc_pipebusy=0;
   dpc_register.dpc_tmem=0;
   readdp[0x0] = &dpc_register.dpc_start;
   readdp[0x4] = &dpc_register.dpc_end;
   readdp[0x8] = &dpc_register.dpc_current;
   readdp[0xc] = &dpc_register.dpc_status;
   readdp[0x10] = &dpc_register.dpc_clock;
   readdp[0x14] = &dpc_register.dpc_bufbusy;
   readdp[0x18] = &dpc_register.dpc_pipebusy;
   readdp[0x1c] = &dpc_register.dpc_tmem;
   
   for (i=0x20; i<0x10000; i++) readdp[i] = &trash;
   for (i=1; i<0x10; i++)
     {
    readmem[0x8410+i] = read_nothing;
    readmem[0xa410+i] = read_nothing;
    readmemb[0x8410+i] = read_nothingb;
    readmemb[0xa410+i] = read_nothingb;
    readmemh[0x8410+i] = read_nothingh;
    readmemh[0xa410+i] = read_nothingh;
    readmemd[0x8410+i] = read_nothingd;
    readmemd[0xa410+i] = read_nothingd;
    writemem[0x8410+i] = write_nothing;
    writemem[0xa410+i] = write_nothing;
    writememb[0x8410+i] = write_nothingb;
    writememb[0xa410+i] = write_nothingb;
    writememh[0x8410+i] = write_nothingh;
    writememh[0xa410+i] = write_nothingh;
    writememd[0x8410+i] = write_nothingd;
    writememd[0xa410+i] = write_nothingd;
     }
   
   //init rsp span registers
   readmem[0x8420] = read_dps;
   readmem[0xa420] = read_dps;
   readmemb[0x8420] = read_dpsb;
   readmemb[0xa420] = read_dpsb;
   readmemh[0x8420] = read_dpsh;
   readmemh[0xa420] = read_dpsh;
   readmemd[0x8420] = read_dpsd;
   readmemd[0xa420] = read_dpsd;
   writemem[0x8420] = write_dps;
   writemem[0xa420] = write_dps;
   writememb[0x8420] = write_dpsb;
   writememb[0xa420] = write_dpsb;
   writememh[0x8420] = write_dpsh;
   writememh[0xa420] = write_dpsh;
   writememd[0x8420] = write_dpsd;
   writememd[0xa420] = write_dpsd;
   dps_register.dps_tbist=0;
   dps_register.dps_test_mode=0;
   dps_register.dps_buftest_addr=0;
   dps_register.dps_buftest_data=0;
   readdps[0x0] = &dps_register.dps_tbist;
   readdps[0x4] = &dps_register.dps_test_mode;
   readdps[0x8] = &dps_register.dps_buftest_addr;
   readdps[0xc] = &dps_register.dps_buftest_data;
   
   for (i=0x10; i<0x10000; i++) readdps[i] = &trash;
   for (i=1; i<0x10; i++)
     {
    readmem[0x8420+i] = read_nothing;
    readmem[0xa420+i] = read_nothing;
    readmemb[0x8420+i] = read_nothingb;
    readmemb[0xa420+i] = read_nothingb;
    readmemh[0x8420+i] = read_nothingh;
    readmemh[0xa420+i] = read_nothingh;
    readmemd[0x8420+i] = read_nothingd;
    readmemd[0xa420+i] = read_nothingd;
    writemem[0x8420+i] = write_nothing;
    writemem[0xa420+i] = write_nothing;
    writememb[0x8420+i] = write_nothingb;
    writememb[0xa420+i] = write_nothingb;
    writememh[0x8420+i] = write_nothingh;
    writememh[0xa420+i] = write_nothingh;
    writememd[0x8420+i] = write_nothingd;
    writememd[0xa420+i] = write_nothingd;
     }
   
   //init mips registers
   readmem[0xa830] = read_mi;
   readmem[0xa430] = read_mi;
   readmemb[0xa830] = read_mib;
   readmemb[0xa430] = read_mib;
   readmemh[0xa830] = read_mih;
   readmemh[0xa430] = read_mih;
   readmemd[0xa830] = read_mid;
   readmemd[0xa430] = read_mid;
   writemem[0x8430] = write_mi;
   writemem[0xa430] = write_mi;
   writememb[0x8430] = write_mib;
   writememb[0xa430] = write_mib;
   writememh[0x8430] = write_mih;
   writememh[0xa430] = write_mih;
   writememd[0x8430] = write_mid;
   writememd[0xa430] = write_mid;
   MI_register.w_mi_init_mode_reg = 0;
   MI_register.mi_init_mode_reg = 0;
   MI_register.init_length = 0;
   MI_register.init_mode = 0;
   MI_register.ebus_test_mode = 0;
   MI_register.RDRAM_reg_mode = 0;
   MI_register.mi_version_reg = 0x02020102;
   MI_register.mi_intr_reg = 0;
   MI_register.w_mi_intr_mask_reg = 0;
   MI_register.mi_intr_mask_reg = 0;
   MI_register.SP_intr_mask = 0;
   MI_register.SI_intr_mask = 0;
   MI_register.AI_intr_mask = 0;
   MI_register.VI_intr_mask = 0;
   MI_register.PI_intr_mask = 0;
   MI_register.DP_intr_mask = 0;
   readmi[0x0] = &MI_register.mi_init_mode_reg;
   readmi[0x4] = &MI_register.mi_version_reg;
   readmi[0x8] = &MI_register.mi_intr_reg;
   readmi[0xc] = &MI_register.mi_intr_mask_reg;
   
   for (i=0x10; i<0x10000; i++) readmi[i] = &trash;
   for (i=1; i<0x10; i++)
     {
    readmem[0x8430+i] = read_nothing;
    readmem[0xa430+i] = read_nothing;
    readmemb[0x8430+i] = read_nothingb;
    readmemb[0xa430+i] = read_nothingb;
    readmemh[0x8430+i] = read_nothingh;
    readmemh[0xa430+i] = read_nothingh;
    readmemd[0x8430+i] = read_nothingd;
    readmemd[0xa430+i] = read_nothingd;
    writemem[0x8430+i] = write_nothing;
    writemem[0xa430+i] = write_nothing;
    writememb[0x8430+i] = write_nothingb;
    writememb[0xa430+i] = write_nothingb;
    writememh[0x8430+i] = write_nothingh;
    writememh[0xa430+i] = write_nothingh;
    writememd[0x8430+i] = write_nothingd;
    writememd[0xa430+i] = write_nothingd;
     }
   
   //init VI registers
   readmem[0x8440] = read_vi;
   readmem[0xa440] = read_vi;
   readmemb[0x8440] = read_vib;
   readmemb[0xa440] = read_vib;
   readmemh[0x8440] = read_vih;
   readmemh[0xa440] = read_vih;
   readmemd[0x8440] = read_vid;
   readmemd[0xa440] = read_vid;
   writemem[0x8440] = write_vi;
   writemem[0xa440] = write_vi;
   writememb[0x8440] = write_vib;
   writememb[0xa440] = write_vib;
   writememh[0x8440] = write_vih;
   writememh[0xa440] = write_vih;
   writememd[0x8440] = write_vid;
   writememd[0xa440] = write_vid;
   vi_register.vi_status = 0;
   vi_register.vi_origin = 0;
   vi_register.vi_width = 0;
   vi_register.vi_v_intr = 0;
   vi_register.vi_current = 0;
   vi_register.vi_burst = 0;
   vi_register.vi_v_sync = 0;
   vi_register.vi_h_sync = 0;
   vi_register.vi_leap = 0;
   vi_register.vi_h_start = 0;
   vi_register.vi_v_start = 0;
   vi_register.vi_v_burst = 0;
   vi_register.vi_x_scale = 0;
   vi_register.vi_y_scale = 0;
   readvi[0x0] = &vi_register.vi_status;
   readvi[0x4] = &vi_register.vi_origin;
   readvi[0x8] = &vi_register.vi_width;
   readvi[0xc] = &vi_register.vi_v_intr;
   readvi[0x10] = &vi_register.vi_current;
   readvi[0x14] = &vi_register.vi_burst;
   readvi[0x18] = &vi_register.vi_v_sync;
   readvi[0x1c] = &vi_register.vi_h_sync;
   readvi[0x20] = &vi_register.vi_leap;
   readvi[0x24] = &vi_register.vi_h_start;
   readvi[0x28] = &vi_register.vi_v_start;
   readvi[0x2c] = &vi_register.vi_v_burst;
   readvi[0x30] = &vi_register.vi_x_scale;
   readvi[0x34] = &vi_register.vi_y_scale;
   
   for (i=0x38; i<0x10000; i++) readvi[i] = &trash;
   for (i=1; i<0x10; i++)
     {
    readmem[0x8440+i] = read_nothing;
    readmem[0xa440+i] = read_nothing;
    readmemb[0x8440+i] = read_nothingb;
    readmemb[0xa440+i] = read_nothingb;
    readmemh[0x8440+i] = read_nothingh;
    readmemh[0xa440+i] = read_nothingh;
    readmemd[0x8440+i] = read_nothingd;
    readmemd[0xa440+i] = read_nothingd;
    writemem[0x8440+i] = write_nothing;
    writemem[0xa440+i] = write_nothing;
    writememb[0x8440+i] = write_nothingb;
    writememb[0xa440+i] = write_nothingb;
    writememh[0x8440+i] = write_nothingh;
    writememh[0xa440+i] = write_nothingh;
    writememd[0x8440+i] = write_nothingd;
    writememd[0xa440+i] = write_nothingd;
     }
   
   //init AI registers
   readmem[0x8450] = read_ai;
   readmem[0xa450] = read_ai;
   readmemb[0x8450] = read_aib;
   readmemb[0xa450] = read_aib;
   readmemh[0x8450] = read_aih;
   readmemh[0xa450] = read_aih;
   readmemd[0x8450] = read_aid;
   readmemd[0xa450] = read_aid;
   writemem[0x8450] = write_ai;
   writemem[0xa450] = write_ai;
   writememb[0x8450] = write_aib;
   writememb[0xa450] = write_aib;
   writememh[0x8450] = write_aih;
   writememh[0xa450] = write_aih;
   writememd[0x8450] = write_aid;
   writememd[0xa450] = write_aid;
   ai_register.ai_dram_addr = 0;
   ai_register.ai_len = 0;
   ai_register.ai_control = 0;
   ai_register.ai_status = 0;
   ai_register.ai_dacrate = 0;
   ai_register.ai_bitrate = 0;
   ai_register.next_delay = 0;
   ai_register.next_len = 0;
   ai_register.current_delay = 0;
   ai_register.current_len = 0;
   readai[0x0] = &ai_register.ai_dram_addr;
   readai[0x4] = &ai_register.ai_len;
   readai[0x8] = &ai_register.ai_control;
   readai[0xc] = &ai_register.ai_status;
   readai[0x10] = &ai_register.ai_dacrate;
   readai[0x14] = &ai_register.ai_bitrate;
   
   for (i=0x18; i<0x10000; i++) readai[i] = &trash;
   for (i=1; i<0x10; i++)
     {
    readmem[0x8450+i] = read_nothing;
    readmem[0xa450+i] = read_nothing;
    readmemb[0x8450+i] = read_nothingb;
    readmemb[0xa450+i] = read_nothingb;
    readmemh[0x8450+i] = read_nothingh;
    readmemh[0xa450+i] = read_nothingh;
    readmemd[0x8450+i] = read_nothingd;
    readmemd[0xa450+i] = read_nothingd;
    writemem[0x8450+i] = write_nothing;
    writemem[0xa450+i] = write_nothing;
    writememb[0x8450+i] = write_nothingb;
    writememb[0xa450+i] = write_nothingb;
    writememh[0x8450+i] = write_nothingh;
    writememh[0xa450+i] = write_nothingh;
    writememd[0x8450+i] = write_nothingd;
    writememd[0xa450+i] = write_nothingd;
     }
   
   //init PI registers
   readmem[0x8460] = read_pi;
   readmem[0xa460] = read_pi;
   readmemb[0x8460] = read_pib;
   readmemb[0xa460] = read_pib;
   readmemh[0x8460] = read_pih;
   readmemh[0xa460] = read_pih;
   readmemd[0x8460] = read_pid;
   readmemd[0xa460] = read_pid;
   writemem[0x8460] = write_pi;
   writemem[0xa460] = write_pi;
   writememb[0x8460] = write_pib;
   writememb[0xa460] = write_pib;
   writememh[0x8460] = write_pih;
   writememh[0xa460] = write_pih;
   writememd[0x8460] = write_pid;
   writememd[0xa460] = write_pid;
   pi_register.pi_dram_addr_reg = 0;
   pi_register.pi_cart_addr_reg = 0;
   pi_register.pi_rd_len_reg = 0;
   pi_register.pi_wr_len_reg = 0;
   pi_register.read_pi_status_reg = 0;
   pi_register.pi_bsd_dom1_lat_reg = 0;
   pi_register.pi_bsd_dom1_pwd_reg = 0;
   pi_register.pi_bsd_dom1_pgs_reg = 0;
   pi_register.pi_bsd_dom1_rls_reg = 0;
   pi_register.pi_bsd_dom2_lat_reg = 0;
   pi_register.pi_bsd_dom2_pwd_reg = 0;
   pi_register.pi_bsd_dom2_pgs_reg = 0;
   pi_register.pi_bsd_dom2_rls_reg = 0;
   readpi[0x0] = &pi_register.pi_dram_addr_reg;
   readpi[0x4] = &pi_register.pi_cart_addr_reg;
   readpi[0x8] = &pi_register.pi_rd_len_reg;
   readpi[0xc] = &pi_register.pi_wr_len_reg;
   readpi[0x10] = &pi_register.read_pi_status_reg;
   readpi[0x14] = &pi_register.pi_bsd_dom1_lat_reg;
   readpi[0x18] = &pi_register.pi_bsd_dom1_pwd_reg;
   readpi[0x1c] = &pi_register.pi_bsd_dom1_pgs_reg;
   readpi[0x20] = &pi_register.pi_bsd_dom1_rls_reg;
   readpi[0x24] = &pi_register.pi_bsd_dom2_lat_reg;
   readpi[0x28] = &pi_register.pi_bsd_dom2_pwd_reg;
   readpi[0x2c] = &pi_register.pi_bsd_dom2_pgs_reg;
   readpi[0x30] = &pi_register.pi_bsd_dom2_rls_reg;
   
   for (i=0x34; i<0x10000; i++) readpi[i] = &trash;
   for (i=1; i<0x10; i++)
     {
    readmem[0x8460+i] = read_nothing;
    readmem[0xa460+i] = read_nothing;
    readmemb[0x8460+i] = read_nothingb;
    readmemb[0xa460+i] = read_nothingb;
    readmemh[0x8460+i] = read_nothingh;
    readmemh[0xa460+i] = read_nothingh;
    readmemd[0x8460+i] = read_nothingd;
    readmemd[0xa460+i] = read_nothingd;
    writemem[0x8460+i] = write_nothing;
    writemem[0xa460+i] = write_nothing;
    writememb[0x8460+i] = write_nothingb;
    writememb[0xa460+i] = write_nothingb;
    writememh[0x8460+i] = write_nothingh;
    writememh[0xa460+i] = write_nothingh;
    writememd[0x8460+i] = write_nothingd;
    writememd[0xa460+i] = write_nothingd;
     }
   
   //init RI registers
   readmem[0x8470] = read_ri;
   readmem[0xa470] = read_ri;
   readmemb[0x8470] = read_rib;
   readmemb[0xa470] = read_rib;
   readmemh[0x8470] = read_rih;
   readmemh[0xa470] = read_rih;
   readmemd[0x8470] = read_rid;
   readmemd[0xa470] = read_rid;
   writemem[0x8470] = write_ri;
   writemem[0xa470] = write_ri;
   writememb[0x8470] = write_rib;
   writememb[0xa470] = write_rib;
   writememh[0x8470] = write_rih;
   writememh[0xa470] = write_rih;
   writememd[0x8470] = write_rid;
   writememd[0xa470] = write_rid;
   ri_register.ri_mode = 0;
   ri_register.ri_config = 0;
   ri_register.ri_select = 0;
   ri_register.ri_current_load = 0;
   ri_register.ri_refresh = 0;
   ri_register.ri_latency = 0;
   ri_register.ri_error = 0;
   ri_register.ri_werror = 0;
   readri[0x0] = &ri_register.ri_mode;
   readri[0x4] = &ri_register.ri_config;
   readri[0x8] = &ri_register.ri_current_load;
   readri[0xc] = &ri_register.ri_select;
   readri[0x10] = &ri_register.ri_refresh;
   readri[0x14] = &ri_register.ri_latency;
   readri[0x18] = &ri_register.ri_error;
   readri[0x1c] = &ri_register.ri_werror;
   
   for (i=0x20; i<0x10000; i++) readri[i] = &trash;
   for (i=1; i<0x10; i++)
     {
    readmem[0x8470+i] = read_nothing;
    readmem[0xa470+i] = read_nothing;
    readmemb[0x8470+i] = read_nothingb;
    readmemb[0xa470+i] = read_nothingb;
    readmemh[0x8470+i] = read_nothingh;
    readmemh[0xa470+i] = read_nothingh;
    readmemd[0x8470+i] = read_nothingd;
    readmemd[0xa470+i] = read_nothingd;
    writemem[0x8470+i] = write_nothing;
    writemem[0xa470+i] = write_nothing;
    writememb[0x8470+i] = write_nothingb;
    writememb[0xa470+i] = write_nothingb;
    writememh[0x8470+i] = write_nothingh;
    writememh[0xa470+i] = write_nothingh;
    writememd[0x8470+i] = write_nothingd;
    writememd[0xa470+i] = write_nothingd;
     }
   
   //init SI registers
   readmem[0x8480] = read_si;
   readmem[0xa480] = read_si;
   readmemb[0x8480] = read_sib;
   readmemb[0xa480] = read_sib;
   readmemh[0x8480] = read_sih;
   readmemh[0xa480] = read_sih;
   readmemd[0x8480] = read_sid;
   readmemd[0xa480] = read_sid;
   writemem[0x8480] = write_si;
   writemem[0xa480] = write_si;
   writememb[0x8480] = write_sib;
   writememb[0xa480] = write_sib;
   writememh[0x8480] = write_sih;
   writememh[0xa480] = write_sih;
   writememd[0x8480] = write_sid;
   writememd[0xa480] = write_sid;
   si_register.si_dram_addr = 0;
   si_register.si_pif_addr_rd64b = 0;
   si_register.si_pif_addr_wr64b = 0;
   si_register.si_stat = 0;
   readsi[0x0] = &si_register.si_dram_addr;
   readsi[0x4] = &si_register.si_pif_addr_rd64b;
   readsi[0x8] = &trash;
   readsi[0x10] = &si_register.si_pif_addr_wr64b;
   readsi[0x14] = &trash;
   readsi[0x18] = &si_register.si_stat;
   
   for (i=0x1c; i<0x10000; i++) readsi[i] = &trash;
   for (i=0x481; i<0x800; i++)
     {
    readmem[0x8000+i] = read_nothing;
    readmem[0xa000+i] = read_nothing;
    readmemb[0x8000+i] = read_nothingb;
    readmemb[0xa000+i] = read_nothingb;
    readmemh[0x8000+i] = read_nothingh;
    readmemh[0xa000+i] = read_nothingh;
    readmemd[0x8000+i] = read_nothingd;
    readmemd[0xa000+i] = read_nothingd;
    writemem[0x8000+i] = write_nothing;
    writemem[0xa000+i] = write_nothing;
    writememb[0x8000+i] = write_nothingb;
    writememb[0xa000+i] = write_nothingb;
    writememh[0x8000+i] = write_nothingh;
    writememh[0xa000+i] = write_nothingh;
    writememd[0x8000+i] = write_nothingd;
    writememd[0xa000+i] = write_nothingd;
     }
   
   //init flashram / sram
   readmem[0x8800] = read_flashram_status;
   readmem[0xa800] = read_flashram_status;
   readmemb[0x8800] = read_flashram_statusb;
   readmemb[0xa800] = read_flashram_statusb;
   readmemh[0x8800] = read_flashram_statush;
   readmemh[0xa800] = read_flashram_statush;
   readmemd[0x8800] = read_flashram_statusd;
   readmemd[0xa800] = read_flashram_statusd;
   writemem[0x8800] = write_flashram_dummy;
   writemem[0xa800] = write_flashram_dummy;
   writememb[0x8800] = write_flashram_dummyb;
   writememb[0xa800] = write_flashram_dummyb;
   writememh[0x8800] = write_flashram_dummyh;
   writememh[0xa800] = write_flashram_dummyh;
   writememd[0x8800] = write_flashram_dummyd;
   writememd[0xa800] = write_flashram_dummyd;
   readmem[0x8801] = read_nothing;
   readmem[0xa801] = read_nothing;
   readmemb[0x8801] = read_nothingb;
   readmemb[0xa801] = read_nothingb;
   readmemh[0x8801] = read_nothingh;
   readmemh[0xa801] = read_nothingh;
   readmemd[0x8801] = read_nothingd;
   readmemd[0xa801] = read_nothingd;
   writemem[0x8801] = write_flashram_command;
   writemem[0xa801] = write_flashram_command;
   writememb[0x8801] = write_flashram_commandb;
   writememb[0xa801] = write_flashram_commandb;
   writememh[0x8801] = write_flashram_commandh;
   writememh[0xa801] = write_flashram_commandh;
   writememd[0x8801] = write_flashram_commandd;
   writememd[0xa801] = write_flashram_commandd;
   
   for (i=0x802; i<0x1000; i++)
     {
    readmem[0x8000+i] = read_nothing;
    readmem[0xa000+i] = read_nothing;
    readmemb[0x8000+i] = read_nothingb;
    readmemb[0xa000+i] = read_nothingb;
    readmemh[0x8000+i] = read_nothingh;
    readmemh[0xa000+i] = read_nothingh;
    readmemd[0x8000+i] = read_nothingd;
    readmemd[0xa000+i] = read_nothingd;
    writemem[0x8000+i] = write_nothing;
    writemem[0xa000+i] = write_nothing;
    writememb[0x8000+i] = write_nothingb;
    writememb[0xa000+i] = write_nothingb;
    writememh[0x8000+i] = write_nothingh;
    writememh[0xa000+i] = write_nothingh;
    writememd[0x8000+i] = write_nothingd;
    writememd[0xa000+i] = write_nothingd;
     }
   
   //init rom area
   for (i=0; i<(rom_size >> 16); i++) 
     {
    readmem[0x9000+i] = read_rom;
    readmem[0xb000+i] = read_rom;
    readmemb[0x9000+i] = read_romb;
    readmemb[0xb000+i] = read_romb;
    readmemh[0x9000+i] = read_romh;
    readmemh[0xb000+i] = read_romh;
    readmemd[0x9000+i] = read_romd;
    readmemd[0xb000+i] = read_romd;
    writemem[0x9000+i] = write_nothing;
    writemem[0xb000+i] = write_rom;
    writememb[0x9000+i] = write_nothingb;
    writememb[0xb000+i] = write_nothingb;
    writememh[0x9000+i] = write_nothingh;
    writememh[0xb000+i] = write_nothingh;
    writememd[0x9000+i] = write_nothingd;
    writememd[0xb000+i] = write_nothingd;
     }
   for (i=(rom_size >> 16); i<0xfc0; i++) 
     {
    readmem[0x9000+i] = read_nothing;
    readmem[0xb000+i] = read_nothing;
    readmemb[0x9000+i] = read_nothingb;
    readmemb[0xb000+i] = read_nothingb;
    readmemh[0x9000+i] = read_nothingh;
    readmemh[0xb000+i] = read_nothingh;
    readmemd[0x9000+i] = read_nothingd;
    readmemd[0xb000+i] = read_nothingd;
    writemem[0x9000+i] = write_nothing;
    writemem[0xb000+i] = write_nothing;
    writememb[0x9000+i] = write_nothingb;
    writememb[0xb000+i] = write_nothingb;
    writememh[0x9000+i] = write_nothingh;
    writememh[0xb000+i] = write_nothingh;
    writememd[0x9000+i] = write_nothingd;
    writememd[0xb000+i] = write_nothingd;
     }
   
   //init PIF_RAM
   readmem[0x9fc0] = read_pif;
   readmem[0xbfc0] = read_pif;
   readmemb[0x9fc0] = read_pifb;
   readmemb[0xbfc0] = read_pifb;
   readmemh[0x9fc0] = read_pifh;
   readmemh[0xbfc0] = read_pifh;
   readmemd[0x9fc0] = read_pifd;
   readmemd[0xbfc0] = read_pifd;
   writemem[0x9fc0] = write_pif;
   writemem[0xbfc0] = write_pif;
   writememb[0x9fc0] = write_pifb;
   writememb[0xbfc0] = write_pifb;
   writememh[0x9fc0] = write_pifh;
   writememh[0xbfc0] = write_pifh;
   writememd[0x9fc0] = write_pifd;
   writememd[0xbfc0] = write_pifd;
   for (i=0; i<(0x40/4); i++) PIF_RAM[i]=0;
   
   for (i=0xfc1; i<0x1000; i++) 
     {
    readmem[0x9000+i] = read_nothing;
    readmem[0xb000+i] = read_nothing;
    readmemb[0x9000+i] = read_nothingb;
    readmemb[0xb000+i] = read_nothingb;
    readmemh[0x9000+i] = read_nothingh;
    readmemh[0xb000+i] = read_nothingh;
    readmemd[0x9000+i] = read_nothingd;
    readmemd[0xb000+i] = read_nothingd;
    writemem[0x9000+i] = write_nothing;
    writemem[0xb000+i] = write_nothing;
    writememb[0x9000+i] = write_nothingb;
    writememb[0xb000+i] = write_nothingb;
    writememh[0x9000+i] = write_nothingh;
    writememh[0xb000+i] = write_nothingh;
    writememd[0x9000+i] = write_nothingd;
    writememd[0xb000+i] = write_nothingd;
     }
   
   use_flashram = 0;
   init_flashram();
   
   frameBufferInfos[0].addr = 0;
   fast_memory = 1;
   firstFrameBufferSetting = 1;
   
   DebugMessage(M64MSG_VERBOSE, "Memory initialized");
   return 0;
}

void free_memory(void)
{
}

void update_MI_init_mode_reg(void)
{
   MI_register.init_length = MI_register.w_mi_init_mode_reg & 0x7F;
   if (MI_register.w_mi_init_mode_reg & 0x80) 
     MI_register.init_mode = 0;
   if (MI_register.w_mi_init_mode_reg & 0x100)
     MI_register.init_mode = 1;
   if (MI_register.w_mi_init_mode_reg & 0x200)
     MI_register.ebus_test_mode = 0;
   if (MI_register.w_mi_init_mode_reg & 0x400)
     MI_register.ebus_test_mode = 1;
   if (MI_register.w_mi_init_mode_reg & 0x800)
     {
    MI_register.mi_intr_reg &= 0xFFFFFFDF;
    check_interupt();
     }
   if (MI_register.w_mi_init_mode_reg & 0x1000)
     MI_register.RDRAM_reg_mode=0;
   if (MI_register.w_mi_init_mode_reg & 0x2000)
     MI_register.RDRAM_reg_mode=1;
   MI_register.mi_init_mode_reg = ((MI_register.init_length) |
                   (MI_register.init_mode << 7) |
                   (MI_register.ebus_test_mode << 8) |
                   (MI_register.RDRAM_reg_mode << 9)
                   );
}

void update_MI_intr_mask_reg(void)
{
   if (MI_register.w_mi_intr_mask_reg & 0x1)   MI_register.SP_intr_mask = 0;
   if (MI_register.w_mi_intr_mask_reg & 0x2)   MI_register.SP_intr_mask = 1;
   if (MI_register.w_mi_intr_mask_reg & 0x4)   MI_register.SI_intr_mask = 0;
   if (MI_register.w_mi_intr_mask_reg & 0x8)   MI_register.SI_intr_mask = 1;
   if (MI_register.w_mi_intr_mask_reg & 0x10)  MI_register.AI_intr_mask = 0;
   if (MI_register.w_mi_intr_mask_reg & 0x20)  MI_register.AI_intr_mask = 1;
   if (MI_register.w_mi_intr_mask_reg & 0x40)  MI_register.VI_intr_mask = 0;
   if (MI_register.w_mi_intr_mask_reg & 0x80)  MI_register.VI_intr_mask = 1;
   if (MI_register.w_mi_intr_mask_reg & 0x100) MI_register.PI_intr_mask = 0;
   if (MI_register.w_mi_intr_mask_reg & 0x200) MI_register.PI_intr_mask = 1;
   if (MI_register.w_mi_intr_mask_reg & 0x400) MI_register.DP_intr_mask = 0;
   if (MI_register.w_mi_intr_mask_reg & 0x800) MI_register.DP_intr_mask = 1;
   MI_register.mi_intr_mask_reg = ((MI_register.SP_intr_mask) |
                     (MI_register.SI_intr_mask << 1) |
                     (MI_register.AI_intr_mask << 2) |
                     (MI_register.VI_intr_mask << 3) |
                     (MI_register.PI_intr_mask << 4) |
                     (MI_register.DP_intr_mask << 5)
                     );
}


void update_SP(void)
{
    if (sp_register.w_sp_status_reg & 0x1)
        sp_register.halt = 0;
    if (sp_register.w_sp_status_reg & 0x2)
        sp_register.halt = 1;
    if (sp_register.w_sp_status_reg & 0x4)
        sp_register.broke = 0;
    if (sp_register.w_sp_status_reg & 0x8)
    {
        MI_register.mi_intr_reg &= 0xFFFFFFFE;
        check_interupt();
    }
    
    if (sp_register.w_sp_status_reg & 0x10)
    {
        MI_register.mi_intr_reg |= 1;
        check_interupt();
    }
    
    if (sp_register.w_sp_status_reg & 0x20)
        sp_register.single_step = 0;
    if (sp_register.w_sp_status_reg & 0x40)
        sp_register.single_step = 1;
    if (sp_register.w_sp_status_reg & 0x80)
        sp_register.intr_break = 0;
    if (sp_register.w_sp_status_reg & 0x100)
        sp_register.intr_break = 1;
    if (sp_register.w_sp_status_reg & 0x200)
        sp_register.signal0 = 0;
    if (sp_register.w_sp_status_reg & 0x400)
        sp_register.signal0 = 1;
    if (sp_register.w_sp_status_reg & 0x800)
        sp_register.signal1 = 0;
    if (sp_register.w_sp_status_reg & 0x1000)
        sp_register.signal1 = 1;
    if (sp_register.w_sp_status_reg & 0x2000)
        sp_register.signal2 = 0;
    if (sp_register.w_sp_status_reg & 0x4000)
        sp_register.signal2 = 1;
    if (sp_register.w_sp_status_reg & 0x8000)
        sp_register.signal3 = 0;
    if (sp_register.w_sp_status_reg & 0x10000)
        sp_register.signal3 = 1;
    if (sp_register.w_sp_status_reg & 0x20000)
        sp_register.signal4 = 0;
    if (sp_register.w_sp_status_reg & 0x40000)
        sp_register.signal4 = 1;
    if (sp_register.w_sp_status_reg & 0x80000)
        sp_register.signal5 = 0;
    if (sp_register.w_sp_status_reg & 0x100000)
        sp_register.signal5 = 1;
    if (sp_register.w_sp_status_reg & 0x200000)
        sp_register.signal6 = 0;
    if (sp_register.w_sp_status_reg & 0x400000)
        sp_register.signal6 = 1;
    if (sp_register.w_sp_status_reg & 0x800000)
        sp_register.signal7 = 0;
    if (sp_register.w_sp_status_reg & 0x1000000)
        sp_register.signal7 = 1;
    sp_register.sp_status_reg = ((sp_register.halt) |
                (sp_register.broke << 1) |
                (sp_register.dma_busy << 2) |
                (sp_register.dma_full << 3) |
                (sp_register.io_full << 4) |
                (sp_register.single_step << 5) |
                (sp_register.intr_break << 6) |
                (sp_register.signal0 << 7) |
                (sp_register.signal1 << 8) |
                (sp_register.signal2 << 9) |
                (sp_register.signal3 << 10) |
                (sp_register.signal4 << 11) |
                (sp_register.signal5 << 12) |
                (sp_register.signal6 << 13) |
                (sp_register.signal7 << 14));
    
    //if (get_event(SP_INT)) return;
    if (!(sp_register.w_sp_status_reg & 0x1) && 
        !(sp_register.w_sp_status_reg & 0x4)) return;
    if (!sp_register.halt && !sp_register.broke)
    {
        int save_pc = rsp_register.rsp_pc & ~0xFFF;
        if (SP_DMEM[0xFC0/4] == 1)
        {
            // unprotecting old frame buffers
            if(fBGetFrameBufferInfo && fBRead && fBWrite && 
            frameBufferInfos[0].addr)
            {
                int i;
                for(i=0; i<6; i++)
                {
                    if(frameBufferInfos[i].addr)
                    {
                        int j;
                        int start = frameBufferInfos[i].addr & 0x7FFFFF;
                        int end = start + frameBufferInfos[i].width*
                                    frameBufferInfos[i].height*
                                    frameBufferInfos[i].size - 1;
                        start = start >> 16;
                        end = end >> 16;

                        for(j=start; j<=end; j++)
                        {
#ifdef DBG
                            if(lookup_breakpoint(0x80000000 + j * 0x10000, 0xFFFF, 
                                BPT_FLAG_ENABLED |  BPT_FLAG_READ ) != -1)
                            {
                                readmem[0x8000+j] = read_rdram_break;
                                readmemb[0x8000+j] = read_rdramb_break;
                                readmemh[0x8000+j] = read_rdramh_break;
                                readmemd[0xa000+j] = read_rdramd_break;
                            }
                            else
                            {
#endif
                            readmem[0x8000+j] = read_rdram;
                            readmemb[0x8000+j] = read_rdramb;
                            readmemh[0x8000+j] = read_rdramh;
                            readmemd[0xa000+j] = read_rdramd;
#ifdef DBG
                            }
                            if(lookup_breakpoint(0xa0000000 + j * 0x10000, 0xFFFF, 
                                BPT_FLAG_ENABLED |  BPT_FLAG_READ ) != -1)
                            {
                                readmem[0xa000+j] = read_rdram_break;
                                readmemb[0xa000+j] = read_rdramb_break;
                                readmemh[0xa000+j] = read_rdramh_break;
                                readmemd[0x8000+j] = read_rdramd_break;
                            }
                            else
                            {
#endif
                            readmem[0xa000+j] = read_rdram;
                            readmemb[0xa000+j] = read_rdramb;
                            readmemh[0xa000+j] = read_rdramh;
                            readmemd[0x8000+j] = read_rdramd;
#ifdef DBG
                            }
                            if(lookup_breakpoint(0x80000000 + j * 0x10000, 0xFFFF, 
                                BPT_FLAG_ENABLED |  BPT_FLAG_WRITE ) != -1)
                            {
                                writemem[0x8000+j] = write_rdram_break;
                                writememb[0x8000+j] = write_rdramb_break;
                                writememh[0x8000+j] = write_rdramh_break;
                                writememd[0x8000+j] = write_rdramd_break;
                            }
                            else
                            {
#endif
                            writemem[0x8000+j] = write_rdram;
                            writememb[0x8000+j] = write_rdramb;
                            writememh[0x8000+j] = write_rdramh;
                            writememd[0x8000+j] = write_rdramd;
#ifdef DBG
                            }
                            if(lookup_breakpoint(0xa0000000 + j * 0x10000, 0xFFFF, 
                                BPT_FLAG_ENABLED |  BPT_FLAG_WRITE ) != -1)
                            {
                                writemem[0xa000+j] = write_rdram_break;
                                writememb[0xa000+j] = write_rdramb_break;
                                writememh[0xa000+j] = write_rdramh_break;
                                writememd[0xa000+j] = write_rdramd_break;
                            }
                            else
                            {
#endif
                            writemem[0xa000+j] = write_rdram;
                            writememb[0xa000+j] = write_rdramb;
                            writememh[0xa000+j] = write_rdramh;
                            writememd[0xa000+j] = write_rdramd;
#ifdef DBG
                            }
#endif
                        }
                    }
                }
            }
         
            //processDList();
            rsp_register.rsp_pc &= 0xFFF;
            start_section(GFX_SECTION);
            doRspCycles(100);
            end_section(GFX_SECTION);
            rsp_register.rsp_pc |= save_pc;
            new_frame();
         
            MI_register.mi_intr_reg &= ~0x21;
            sp_register.sp_status_reg &= ~0x303;
            update_count();
            add_interupt_event(SP_INT, 1000);
            add_interupt_event(DP_INT, 1000);
         
            // protecting new frame buffers
            if(fBGetFrameBufferInfo && fBRead && fBWrite) 
                fBGetFrameBufferInfo(frameBufferInfos);
            if(fBGetFrameBufferInfo && fBRead && fBWrite 
                && frameBufferInfos[0].addr)
           {
                int i;
                for(i=0; i<6; i++)
                {
                    if(frameBufferInfos[i].addr)
                    {
                        int j;
                        int start = frameBufferInfos[i].addr & 0x7FFFFF;
                        int end = start + frameBufferInfos[i].width*
                                        frameBufferInfos[i].height*
                                        frameBufferInfos[i].size - 1;
                        int start1 = start;
                        int end1 = end;
                        start >>= 16;
                        end >>= 16;
                        for(j=start; j<=end; j++)
                        {
 #ifdef DBG
                            if(lookup_breakpoint(0x80000000 + j * 0x10000, 0xFFFF, 
                                BPT_FLAG_ENABLED |  BPT_FLAG_READ ) != -1)
                            {
                                readmem[0x8000+j] = read_rdramFB_break;
                                readmemb[0x8000+j] = read_rdramFBb_break;
                                readmemh[0x8000+j] = read_rdramFBh_break;
                                readmemd[0xa000+j] = read_rdramFBd_break;
                            }
                            else
                            {
#endif
                            readmem[0x8000+j] = read_rdramFB;
                            readmemb[0x8000+j] = read_rdramFBb;
                            readmemh[0x8000+j] = read_rdramFBh;
                            readmemd[0xa000+j] = read_rdramFBd;
#ifdef DBG
                            }
                            if(lookup_breakpoint(0xa0000000 + j * 0x10000, 0xFFFF, 
                                BPT_FLAG_ENABLED |  BPT_FLAG_READ ) != -1)
                            {
                                readmem[0xa000+j] = read_rdramFB_break;
                                readmemb[0xa000+j] = read_rdramFBb_break;
                                readmemh[0xa000+j] = read_rdramFBh_break;
                                readmemd[0x8000+j] = read_rdramFBd_break;
                            }
                            else
                            {
#endif
                            readmem[0xa000+j] = read_rdramFB;
                            readmemb[0xa000+j] = read_rdramFBb;
                            readmemh[0xa000+j] = read_rdramFBh;
                            readmemd[0x8000+j] = read_rdramFBd;
#ifdef DBG
                            }
                            if(lookup_breakpoint(0x80000000 + j * 0x10000, 0xFFFF, 
                                BPT_FLAG_ENABLED |  BPT_FLAG_WRITE ) != -1)
                            {
                                writemem[0x8000+j] = write_rdramFB_break;
                                writememb[0x8000+j] = write_rdramFBb_break;
                                writememh[0x8000+j] = write_rdramFBh_break;
                                writememd[0x8000+j] = write_rdramFBd_break;
                            }
                            else
                            {
#endif
                            writemem[0x8000+j] = write_rdramFB;
                            writememb[0x8000+j] = write_rdramFBb;
                            writememh[0x8000+j] = write_rdramFBh;
                            writememd[0x8000+j] = write_rdramFBd;
#ifdef DBG
                            }
                            if(lookup_breakpoint(0xa0000000 + j * 0x10000, 0xFFFF, 
                                BPT_FLAG_ENABLED |  BPT_FLAG_WRITE ) != -1)
                            {
                                writemem[0xa000+j] = write_rdramFB_break;
                                writememb[0xa000+j] = write_rdramFBb_break;
                                writememh[0xa000+j] = write_rdramFBh_break;
                                writememd[0xa000+j] = write_rdramFBd_break;
                            }
                            else
                            {
#endif
                            writemem[0xa000+j] = write_rdramFB;
                            writememb[0xa000+j] = write_rdramFBb;
                            writememh[0xa000+j] = write_rdramFBh;
                            writememd[0xa000+j] = write_rdramFBd;
#ifdef DBG
                            }
#endif
                        }
                        start <<= 4;
                        end <<= 4;
                        for(j=start; j<=end; j++)
                        {
                            if(j>=start1 && j<=end1) framebufferRead[j]=1;
                            else framebufferRead[j] = 0;
                        }
                        
                        if(firstFrameBufferSetting)
                        {
                            firstFrameBufferSetting = 0;
                            fast_memory = 0;
                            for(j=0; j<0x100000; j++)
                                invalid_code[j] = 1;
                        }
                    }
                }
            }
        }
        else if (SP_DMEM[0xFC0/4] == 2)
        {
            //processAList();
            rsp_register.rsp_pc &= 0xFFF;
            start_section(AUDIO_SECTION);
            doRspCycles(100);
            end_section(AUDIO_SECTION);
            rsp_register.rsp_pc |= save_pc;
         
            MI_register.mi_intr_reg &= ~0x1;
            sp_register.sp_status_reg &= ~0x303;
            update_count();
            //add_interupt_event(SP_INT, 500);
            add_interupt_event(SP_INT, 4000);
        }
        else
        {
            rsp_register.rsp_pc &= 0xFFF;
            doRspCycles(100);
            rsp_register.rsp_pc |= save_pc;
         
            MI_register.mi_intr_reg &= ~0x1;
            sp_register.sp_status_reg &= ~0x203;
            update_count();
            add_interupt_event(SP_INT, 0/*100*/);
        }
    }
}

void update_DPC(void)
{
   if (dpc_register.w_dpc_status & 0x1)
     dpc_register.xbus_dmem_dma = 0;
   if (dpc_register.w_dpc_status & 0x2)
     dpc_register.xbus_dmem_dma = 1;
   if (dpc_register.w_dpc_status & 0x4)
     dpc_register.freeze = 0;
   if (dpc_register.w_dpc_status & 0x8)
     dpc_register.freeze = 1;
   if (dpc_register.w_dpc_status & 0x10)
     dpc_register.flush = 0;
   if (dpc_register.w_dpc_status & 0x20)
     dpc_register.flush = 1;
   dpc_register.dpc_status = ((dpc_register.xbus_dmem_dma) |
                  (dpc_register.freeze << 1) |
                  (dpc_register.flush << 2) |
                  (dpc_register.start_glck << 3) |
                  (dpc_register.tmem_busy << 4) |
                  (dpc_register.pipe_busy << 5) |
                  (dpc_register.cmd_busy << 6) |
                  (dpc_register.cbuf_busy << 7) |
                  (dpc_register.dma_busy << 8) |
                  (dpc_register.end_valid << 9) |
                  (dpc_register.start_valid << 10)
                  );
}

void read_nothing(void)
{
   if (address == 0xa5000508) *rdword = 0xFFFFFFFF;
   else *rdword = 0;
}

void read_nothingb(void)
{
   *rdword = 0;
}

void read_nothingh(void)
{
   *rdword = 0;
}

void read_nothingd(void)
{
   *rdword = 0;
}

void write_nothing(void)
{
}

void write_nothingb(void)
{
}

void write_nothingh(void)
{
}

void write_nothingd(void)
{
}

void read_nomem(void)
{
   address = virtual_to_physical_address(address,0);
   if (address == 0x00000000) return;
   read_word_in_memory();
}

void read_nomemb(void)
{
   address = virtual_to_physical_address(address,0);
   if (address == 0x00000000) return;
   read_byte_in_memory();
}

void read_nomemh(void)
{
   address = virtual_to_physical_address(address,0);
   if (address == 0x00000000) return;
   read_hword_in_memory();
}

void read_nomemd(void)
{
   address = virtual_to_physical_address(address,0);
   if (address == 0x00000000) return;
   read_dword_in_memory();
}

void write_nomem(void)
{
   if (r4300emu != CORE_PURE_INTERPRETER && !invalid_code[address>>12])
     if (blocks[address>>12]->block[(address&0xFFF)/4].ops != NOTCOMPILED)
       invalid_code[address>>12] = 1;
   address = virtual_to_physical_address(address,1);
   if (address == 0x00000000) return;
   write_word_in_memory();
}

void write_nomemb(void)
{
   if (r4300emu != CORE_PURE_INTERPRETER && !invalid_code[address>>12])
     if (blocks[address>>12]->block[(address&0xFFF)/4].ops != NOTCOMPILED)
       invalid_code[address>>12] = 1;
   address = virtual_to_physical_address(address,1);
   if (address == 0x00000000) return;
   write_byte_in_memory();
}

void write_nomemh(void)
{
   if (r4300emu != CORE_PURE_INTERPRETER && !invalid_code[address>>12])
     if (blocks[address>>12]->block[(address&0xFFF)/4].ops != NOTCOMPILED)
       invalid_code[address>>12] = 1;
   address = virtual_to_physical_address(address,1);
   if (address == 0x00000000) return;
   write_hword_in_memory();
}

void write_nomemd(void)
{
   if (r4300emu != CORE_PURE_INTERPRETER && !invalid_code[address>>12])
     if (blocks[address>>12]->block[(address&0xFFF)/4].ops != NOTCOMPILED)
       invalid_code[address>>12] = 1;
   address = virtual_to_physical_address(address,1);
   if (address == 0x00000000) return;
   write_dword_in_memory();
}

void read_rdram(void)
{
   *rdword = *((unsigned int *)(rdramb + (address & 0xFFFFFF)));
}

void read_rdramb(void)
{
   *rdword = *(rdramb + ((address & 0xFFFFFF)^S8));
}

void read_rdramh(void)
{
   *rdword = *((unsigned short *)(rdramb + ((address & 0xFFFFFF)^S16)));
}

void read_rdramd(void)
{
   *rdword = ((unsigned long long int)(*(unsigned int *)(rdramb + (address & 0xFFFFFF))) << 32) |
     ((*(unsigned int *)(rdramb + (address & 0xFFFFFF) + 4)));
}

void read_rdramFB(void)
{
   int i;
   for(i=0; i<6; i++)
     {
    if(frameBufferInfos[i].addr)
      {
         unsigned int start = frameBufferInfos[i].addr & 0x7FFFFF;
         unsigned int end = start + frameBufferInfos[i].width*
           frameBufferInfos[i].height*
           frameBufferInfos[i].size - 1;
         if((address & 0x7FFFFF) >= start && (address & 0x7FFFFF) <= end &&
        framebufferRead[(address & 0x7FFFFF)>>12])
           {
          fBRead(address);
          framebufferRead[(address & 0x7FFFFF)>>12] = 0;
           }
      }
     }
   read_rdram();
}

void read_rdramFBb(void)
{
   int i;
   for(i=0; i<6; i++)
     {
    if(frameBufferInfos[i].addr)
      {
         unsigned int start = frameBufferInfos[i].addr & 0x7FFFFF;
         unsigned int end = start + frameBufferInfos[i].width*
           frameBufferInfos[i].height*
           frameBufferInfos[i].size - 1;
         if((address & 0x7FFFFF) >= start && (address & 0x7FFFFF) <= end &&
        framebufferRead[(address & 0x7FFFFF)>>12])
           {
          fBRead(address);
          framebufferRead[(address & 0x7FFFFF)>>12] = 0;
           }
      }
     }
   read_rdramb();
}

void read_rdramFBh(void)
{
   int i;
   for(i=0; i<6; i++)
     {
    if(frameBufferInfos[i].addr)
      {
         unsigned int start = frameBufferInfos[i].addr & 0x7FFFFF;
         unsigned int end = start + frameBufferInfos[i].width*
           frameBufferInfos[i].height*
           frameBufferInfos[i].size - 1;
         if((address & 0x7FFFFF) >= start && (address & 0x7FFFFF) <= end &&
        framebufferRead[(address & 0x7FFFFF)>>12])
           {
          fBRead(address);
          framebufferRead[(address & 0x7FFFFF)>>12] = 0;
           }
      }
     }
   read_rdramh();
}

void read_rdramFBd(void)
{
   int i;
   for(i=0; i<6; i++)
     {
    if(frameBufferInfos[i].addr)
      {
         unsigned int start = frameBufferInfos[i].addr & 0x7FFFFF;
         unsigned int end = start + frameBufferInfos[i].width*
           frameBufferInfos[i].height*
           frameBufferInfos[i].size - 1;
         if((address & 0x7FFFFF) >= start && (address & 0x7FFFFF) <= end &&
        framebufferRead[(address & 0x7FFFFF)>>12])
           {
          fBRead(address);
          framebufferRead[(address & 0x7FFFFF)>>12] = 0;
           }
      }
     }
   read_rdramd();
}

void write_rdram(void)
{
   *((unsigned int *)(rdramb + (address & 0xFFFFFF))) = word;
}

void write_rdramb(void)
{
   *((rdramb + ((address & 0xFFFFFF)^S8))) = cpu_byte;
}

void write_rdramh(void)
{
   *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S16))) = hword;
}

void write_rdramd(void)
{
   *((unsigned int *)(rdramb + (address & 0xFFFFFF))) = (unsigned int) (dword >> 32);
   *((unsigned int *)(rdramb + (address & 0xFFFFFF) + 4 )) = (unsigned int) (dword & 0xFFFFFFFF);
}

void write_rdramFB(void)
{
   int i;
   for(i=0; i<6; i++)
     {
    if(frameBufferInfos[i].addr)
      {
         unsigned int start = frameBufferInfos[i].addr & 0x7FFFFF;
         unsigned int end = start + frameBufferInfos[i].width*
           frameBufferInfos[i].height*
           frameBufferInfos[i].size - 1;
         if((address & 0x7FFFFF) >= start && (address & 0x7FFFFF) <= end)
           fBWrite(address, 4);
      }
     }
   write_rdram();
}

void write_rdramFBb(void)
{
   int i;
   for(i=0; i<6; i++)
     {
    if(frameBufferInfos[i].addr)
      {
         unsigned int start = frameBufferInfos[i].addr & 0x7FFFFF;
         unsigned int end = start + frameBufferInfos[i].width*
           frameBufferInfos[i].height*
           frameBufferInfos[i].size - 1;
         if((address & 0x7FFFFF) >= start && (address & 0x7FFFFF) <= end)
           fBWrite(address^S8, 1);
      }
     }
   write_rdramb();
}

void write_rdramFBh(void)
{
   int i;
   for(i=0; i<6; i++)
     {
    if(frameBufferInfos[i].addr)
      {
         unsigned int start = frameBufferInfos[i].addr & 0x7FFFFF;
         unsigned int end = start + frameBufferInfos[i].width*
           frameBufferInfos[i].height*
           frameBufferInfos[i].size - 1;
         if((address & 0x7FFFFF) >= start && (address & 0x7FFFFF) <= end)
           fBWrite(address^S16, 2);
      }
     }
   write_rdramh();
}

void write_rdramFBd(void)
{
   int i;
   for(i=0; i<6; i++)
     {
    if(frameBufferInfos[i].addr)
      {
         unsigned int start = frameBufferInfos[i].addr & 0x7FFFFF;
         unsigned int end = start + frameBufferInfos[i].width*
           frameBufferInfos[i].height*
           frameBufferInfos[i].size - 1;
         if((address & 0x7FFFFF) >= start && (address & 0x7FFFFF) <= end)
           fBWrite(address, 8);
      }
     }
   write_rdramd();
}

void read_rdramreg(void)
{
   *rdword = *(readrdramreg[*address_low]);
}

void read_rdramregb(void)
{
   *rdword = *((unsigned char*)readrdramreg[*address_low & 0xfffc]
           + ((*address_low&3)^S8) );
}

void read_rdramregh(void)
{
   *rdword = *((unsigned short*)((unsigned char*)readrdramreg[*address_low & 0xfffc]
           + ((*address_low&3)^S16) ));
}

void read_rdramregd(void)
{
   *rdword = ((unsigned long long int)(*readrdramreg[*address_low])<<32) |
     *readrdramreg[*address_low+4];
}

void write_rdramreg(void)
{
   *readrdramreg[*address_low] = word;
}

void write_rdramregb(void)
{
   *((unsigned char*)readrdramreg[*address_low & 0xfffc]
     + ((*address_low&3)^S8) ) = cpu_byte;
}

void write_rdramregh(void)
{
   *((unsigned short*)((unsigned char*)readrdramreg[*address_low & 0xfffc]
               + ((*address_low&3)^S16) )) = hword;
}

void write_rdramregd(void)
{
   *readrdramreg[*address_low] = (unsigned int) (dword >> 32);
   *readrdramreg[*address_low+4] = (unsigned int) (dword & 0xFFFFFFFF);
}

void read_rsp_mem(void)
{
   if (*address_low < 0x1000)
     *rdword = *((unsigned int *)(SP_DMEMb + (*address_low)));
   else if (*address_low < 0x2000)
     *rdword = *((unsigned int *)(SP_IMEMb + (*address_low&0xFFF)));
   else
     read_nomem();
}

void read_rsp_memb(void)
{
   if (*address_low < 0x1000)
     *rdword = *(SP_DMEMb + (*address_low^S8));
   else if (*address_low < 0x2000)
     *rdword = *(SP_IMEMb + ((*address_low&0xFFF)^S8));
   else
     read_nomemb();
}

void read_rsp_memh(void)
{
   if (*address_low < 0x1000)
     *rdword = *((unsigned short *)(SP_DMEMb + (*address_low^S16)));
   else if (*address_low < 0x2000)
     *rdword = *((unsigned short *)(SP_IMEMb + ((*address_low&0xFFF)^S16)));
   else
     read_nomemh();
}

void read_rsp_memd(void)
{
   if (*address_low < 0x1000)
     {
    *rdword = ((unsigned long long int)(*(unsigned int *)(SP_DMEMb + (*address_low))) << 32) |
      ((*(unsigned int *)(SP_DMEMb + (*address_low) + 4)));
     }
   else if (*address_low < 0x2000)
     {
    *rdword = ((unsigned long long int)(*(unsigned int *)(SP_IMEMb + (*address_low&0xFFF))) << 32) |
      ((*(unsigned int *)(SP_IMEMb + (*address_low&0xFFF) + 4)));
     }
   else
     read_nomemd();
}

void write_rsp_mem(void)
{
   if (*address_low < 0x1000)
     *((unsigned int *)(SP_DMEMb + (*address_low))) = word;
   else if (*address_low < 0x2000)
     *((unsigned int *)(SP_IMEMb + (*address_low&0xFFF))) = word;
   else
     write_nomem();
}

void write_rsp_memb(void)
{
   if (*address_low < 0x1000)
     *(SP_DMEMb + (*address_low^S8)) = cpu_byte;
   else if (*address_low < 0x2000)
     *(SP_IMEMb + ((*address_low&0xFFF)^S8)) = cpu_byte;
   else
     write_nomemb();
}

void write_rsp_memh(void)
{
   if (*address_low < 0x1000)
     *((unsigned short *)(SP_DMEMb + (*address_low^S16))) = hword;
   else if (*address_low < 0x2000)
     *((unsigned short *)(SP_IMEMb + ((*address_low&0xFFF)^S16))) = hword;
   else
     write_nomemh();
}

void write_rsp_memd(void)
{
   if (*address_low < 0x1000)
     {
    *((unsigned int *)(SP_DMEMb + *address_low)) = (unsigned int) (dword >> 32);
    *((unsigned int *)(SP_DMEMb + *address_low + 4 )) = (unsigned int) (dword & 0xFFFFFFFF);
     }
   else if (*address_low < 0x2000)
     {
    *((unsigned int *)(SP_IMEMb + (*address_low&0xFFF))) = (unsigned int) (dword >> 32);
    *((unsigned int *)(SP_IMEMb + (*address_low&0xFFF) + 4 )) = (unsigned int) (dword & 0xFFFFFFFF);
     }
   else
     read_nomemd();
}

void read_rsp_reg(void)
{
   *rdword = *(readrspreg[*address_low]);
   switch(*address_low)
     {
      case 0x1c:
    sp_register.sp_semaphore_reg = 1;
    break;
     }
}

void read_rsp_regb(void)
{
   *rdword = *((unsigned char*)readrspreg[*address_low & 0xfffc]
           + ((*address_low&3)^S8) );
   switch(*address_low)
     {
      case 0x1c:
      case 0x1d:
      case 0x1e:
      case 0x1f:
    sp_register.sp_semaphore_reg = 1;
    break;
     }
}

void read_rsp_regh(void)
{
   *rdword = *((unsigned short*)((unsigned char*)readrspreg[*address_low & 0xfffc]
           + ((*address_low&3)^S16) ));
   switch(*address_low)
     {
      case 0x1c:
      case 0x1e:
    sp_register.sp_semaphore_reg = 1;
    break;
     }
}

void read_rsp_regd(void)
{
   *rdword = ((unsigned long long int)(*readrspreg[*address_low])<<32) |
     *readrspreg[*address_low+4];
   switch(*address_low)
     {
      case 0x18:
    sp_register.sp_semaphore_reg = 1;
    break;
     }
}

void write_rsp_reg(void)
{
   switch(*address_low)
     {
      case 0x10:
    sp_register.w_sp_status_reg = word;
    update_SP();
      case 0x14:
      case 0x18:
    return;
    break;
     }
   *readrspreg[*address_low] = word;
   switch(*address_low)
     {
      case 0x8:
    dma_sp_write();
    break;
      case 0xc:
    dma_sp_read();
    break;
      case 0x1c:
    sp_register.sp_semaphore_reg = 0;
    break;
     }
}

void write_rsp_regb(void)
{
   switch(*address_low)
     {
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
    *((unsigned char*)&sp_register.w_sp_status_reg
      + ((*address_low&3)^S8) ) = cpu_byte;
      case 0x14:
      case 0x15:
      case 0x16:
      case 0x17:
      case 0x18:
      case 0x19:
      case 0x1a:
      case 0x1b:
    return;
    break;
     }
   *((unsigned char*)readrspreg[*address_low & 0xfffc]
     + ((*address_low&3)^S8) ) = cpu_byte;
   switch(*address_low)
     {
      case 0x8:
      case 0x9:
      case 0xa:
      case 0xb:
    dma_sp_write();
    break;
      case 0xc:
      case 0xd:
      case 0xe:
      case 0xf:
    dma_sp_read();
    break;
      case 0x1c:
      case 0x1d:
      case 0x1e:
      case 0x1f:
    sp_register.sp_semaphore_reg = 0;
    break;
     }
}

void write_rsp_regh(void)
{
   switch(*address_low)
     {
      case 0x10:
      case 0x12:
    *((unsigned short*)((unsigned char*)&sp_register.w_sp_status_reg
                + ((*address_low&3)^S16) )) = hword;
      case 0x14:
      case 0x16:
      case 0x18:
      case 0x1a:
    return;
    break;
     }
   *((unsigned short*)((unsigned char*)readrspreg[*address_low & 0xfffc]
               + ((*address_low&3)^S16) )) = hword;
   switch(*address_low)
     {
      case 0x8:
      case 0xa:
    dma_sp_write();
    break;
      case 0xc:
      case 0xe:
    dma_sp_read();
    break;
      case 0x1c:
      case 0x1e:
    sp_register.sp_semaphore_reg = 0;
    break;
     }
}

void write_rsp_regd(void)
{
   switch(*address_low)
     {
      case 0x10:
    sp_register.w_sp_status_reg = (unsigned int) (dword >> 32);
    update_SP();
    return;
    break;
      case 0x18:
    sp_register.sp_semaphore_reg = 0;
    return;
    break;
     }
   *readrspreg[*address_low] = (unsigned int) (dword >> 32);
   *readrspreg[*address_low+4] = (unsigned int) (dword & 0xFFFFFFFF);
   switch(*address_low)
     {
      case 0x8:
    dma_sp_write();
    dma_sp_read();
    break;
     }
}

void read_rsp(void)
{
   *rdword = *(readrsp[*address_low]);
}

void read_rspb(void)
{
   *rdword = *((unsigned char*)readrsp[*address_low & 0xfffc]
           + ((*address_low&3)^S8) );
}

void read_rsph(void)
{
   *rdword = *((unsigned short*)((unsigned char*)readrsp[*address_low & 0xfffc]
                 + ((*address_low&3)^S16) ));
}

void read_rspd(void)
{
   *rdword = ((unsigned long long int)(*readrsp[*address_low])<<32) |
     *readrsp[*address_low+4];
}

void write_rsp(void)
{
   *readrsp[*address_low] = word;
}

void write_rspb(void)
{
   *((unsigned char*)readrsp[*address_low & 0xfffc]
     + ((*address_low&3)^S8) ) = cpu_byte;
}

void write_rsph(void)
{
   *((unsigned short*)((unsigned char*)readrsp[*address_low & 0xfffc]
               + ((*address_low&3)^S16) )) = hword;
}

void write_rspd(void)
{
   *readrsp[*address_low] = (unsigned int) (dword >> 32);
   *readrsp[*address_low+4] = (unsigned int) (dword & 0xFFFFFFFF);
}

void read_dp(void)
{
   *rdword = *(readdp[*address_low]);
}

void read_dpb(void)
{
   *rdword = *((unsigned char*)readdp[*address_low & 0xfffc]
           + ((*address_low&3)^S8) );
}

void read_dph(void)
{
   *rdword = *((unsigned short*)((unsigned char*)readdp[*address_low & 0xfffc]
                 + ((*address_low&3)^S16) ));
}

void read_dpd(void)
{
   *rdword = ((unsigned long long int)(*readdp[*address_low])<<32) |
     *readdp[*address_low+4];
}

void write_dp(void)
{
   switch(*address_low)
     {
      case 0xc:
    dpc_register.w_dpc_status = word;
    update_DPC();
      case 0x8:
      case 0x10:
      case 0x14:
      case 0x18:
      case 0x1c:
    return;
    break;
     }
   *readdp[*address_low] = word;
   switch(*address_low)
     {
      case 0x0:
    dpc_register.dpc_current = dpc_register.dpc_start;
    break;
      case 0x4:
    processRDPList();
    MI_register.mi_intr_reg |= 0x20;
    check_interupt();
    break;
     }
}

void write_dpb(void)
{
   switch(*address_low)
     {
      case 0xc:
      case 0xd:
      case 0xe:
      case 0xf:
    *((unsigned char*)&dpc_register.w_dpc_status
      + ((*address_low&3)^S8) ) = cpu_byte;
    update_DPC();
      case 0x8:
      case 0x9:
      case 0xa:
      case 0xb:
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
      case 0x14:
      case 0x15:
      case 0x16:
      case 0x17:
      case 0x18:
      case 0x19:
      case 0x1a:
      case 0x1b:
      case 0x1c:
      case 0x1d:
      case 0x1e:
      case 0x1f:
    return;
    break;
     }
   *((unsigned char*)readdp[*address_low & 0xfffc]
     + ((*address_low&3)^S8) ) = cpu_byte;
   switch(*address_low)
     {
      case 0x0:
      case 0x1:
      case 0x2:
      case 0x3:
    dpc_register.dpc_current = dpc_register.dpc_start;
    break;
      case 0x4:
      case 0x5:
      case 0x6:
      case 0x7:
    processRDPList();
    MI_register.mi_intr_reg |= 0x20;
    check_interupt();
    break;
     }
}

void write_dph(void)
{
   switch(*address_low)
     {
      case 0xc:
      case 0xe:
    *((unsigned short*)((unsigned char*)&dpc_register.w_dpc_status
                + ((*address_low&3)^S16) )) = hword;
    update_DPC();
      case 0x8:
      case 0xa:
      case 0x10:
      case 0x12:
      case 0x14:
      case 0x16:
      case 0x18:
      case 0x1a:
      case 0x1c:
      case 0x1e:
    return;
    break;
     }
   *((unsigned short*)((unsigned char*)readdp[*address_low & 0xfffc]
               + ((*address_low&3)^S16) )) = hword;
   switch(*address_low)
     {
      case 0x0:
      case 0x2:
    dpc_register.dpc_current = dpc_register.dpc_start;
    break;
      case 0x4:
      case 0x6:
    processRDPList();
    MI_register.mi_intr_reg |= 0x20;
    check_interupt();
    break;
     }
}

void write_dpd(void)
{
   switch(*address_low)
     {
      case 0x8:
    dpc_register.w_dpc_status = (unsigned int) (dword & 0xFFFFFFFF);
    update_DPC();
    return;
    break;
      case 0x10:
      case 0x18:
    return;
    break;
     }
   *readdp[*address_low] = (unsigned int) (dword >> 32);
   *readdp[*address_low+4] = (unsigned int) (dword & 0xFFFFFFFF);
   switch(*address_low)
     {
      case 0x0:
    dpc_register.dpc_current = dpc_register.dpc_start;
    processRDPList();
    MI_register.mi_intr_reg |= 0x20;
    check_interupt();
    break;
     }
}

void read_dps(void)
{
   *rdword = *(readdps[*address_low]);
}

void read_dpsb(void)
{
   *rdword = *((unsigned char*)readdps[*address_low & 0xfffc]
           + ((*address_low&3)^S8) );
}

void read_dpsh(void)
{
   *rdword = *((unsigned short*)((unsigned char*)readdps[*address_low & 0xfffc]
                 + ((*address_low&3)^S16) ));
}

void read_dpsd(void)
{
   *rdword = ((unsigned long long int)(*readdps[*address_low])<<32) |
     *readdps[*address_low+4];
}

void write_dps(void)
{
   *readdps[*address_low] = word;
}

void write_dpsb(void)
{
   *((unsigned char*)readdps[*address_low & 0xfffc]
     + ((*address_low&3)^S8) ) = cpu_byte;
}

void write_dpsh(void)
{
   *((unsigned short*)((unsigned char*)readdps[*address_low & 0xfffc]
               + ((*address_low&3)^S16) )) = hword;
}

void write_dpsd(void)
{
   *readdps[*address_low] = (unsigned int) (dword >> 32);
   *readdps[*address_low+4] = (unsigned int) (dword & 0xFFFFFFFF);
}

void read_mi(void)
{
   *rdword = *(readmi[*address_low]);
}

void read_mib(void)
{
   *rdword = *((unsigned char*)readmi[*address_low & 0xfffc]
           + ((*address_low&3)^S8) );
}

void read_mih(void)
{
   *rdword = *((unsigned short*)((unsigned char*)readmi[*address_low & 0xfffc]
                 + ((*address_low&3)^S16) ));
}

void read_mid(void)
{
   *rdword = ((unsigned long long int)(*readmi[*address_low])<<32) |
     *readmi[*address_low+4];
}

void write_mi(void)
{
   switch(*address_low)
     {
      case 0x0:
    MI_register.w_mi_init_mode_reg = word;
    update_MI_init_mode_reg();
    break;
      case 0xc:
    MI_register.w_mi_intr_mask_reg = word;
    update_MI_intr_mask_reg();
    
    check_interupt();
    update_count();
    if (next_interupt <= Count) gen_interupt();
    break;
     }
}

void write_mib(void)
{
   switch(*address_low)
     {
      case 0x0:
      case 0x1:
      case 0x2:
      case 0x3:
    *((unsigned char*)&MI_register.w_mi_init_mode_reg
      + ((*address_low&3)^S8) ) = cpu_byte;
    update_MI_init_mode_reg();
    break;
      case 0xc:
      case 0xd:
      case 0xe:
      case 0xf:
    *((unsigned char*)&MI_register.w_mi_intr_mask_reg
      + ((*address_low&3)^S8) ) = cpu_byte;
    update_MI_intr_mask_reg();
    
    check_interupt();
    update_count();
    if (next_interupt <= Count) gen_interupt();
    break;
     }
}

void write_mih(void)
{
   switch(*address_low)
     {
      case 0x0:
      case 0x2:
    *((unsigned short*)((unsigned char*)&MI_register.w_mi_init_mode_reg
                + ((*address_low&3)^S16) )) = hword;
    update_MI_init_mode_reg();
    break;
      case 0xc:
      case 0xe:
    *((unsigned short*)((unsigned char*)&MI_register.w_mi_intr_mask_reg
                + ((*address_low&3)^S16) )) = hword;
    update_MI_intr_mask_reg();
    
    check_interupt();
    update_count();
    if (next_interupt <= Count) gen_interupt();
    break;
     }
}

void write_mid(void)
{
   switch(*address_low)
     {
      case 0x0:
    MI_register.w_mi_init_mode_reg = (unsigned int) (dword >> 32);
    update_MI_init_mode_reg();
    break;
      case 0x8:
    MI_register.w_mi_intr_mask_reg = (unsigned int) (dword & 0xFFFFFFFF);
    update_MI_intr_mask_reg();
    
    check_interupt();
    update_count();
    if (next_interupt <= Count) gen_interupt();
    break;
     }
}

void read_vi(void)
{
   switch(*address_low)
     {
      case 0x10:
    update_count();
    vi_register.vi_current = (vi_register.vi_delay-(next_vi-Count))/1500;
    vi_register.vi_current = (vi_register.vi_current&(~1))|vi_field;
    break;
     }
   *rdword = *(readvi[*address_low]);
}

void read_vib(void)
{
   switch(*address_low)
     {
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
    update_count();
    vi_register.vi_current = (vi_register.vi_delay-(next_vi-Count))/1500;
    vi_register.vi_current = (vi_register.vi_current&(~1))|vi_field;
    break;
     }
   *rdword = *((unsigned char*)readvi[*address_low & 0xfffc]
           + ((*address_low&3)^S8) );
}

void read_vih(void)
{
   switch(*address_low)
     {
      case 0x10:
      case 0x12:
    update_count();
    vi_register.vi_current = (vi_register.vi_delay-(next_vi-Count))/1500;
    vi_register.vi_current = (vi_register.vi_current&(~1))|vi_field;
    break;
     }
   *rdword = *((unsigned short*)((unsigned char*)readvi[*address_low & 0xfffc]
                 + ((*address_low&3)^S16) ));
}

void read_vid(void)
{
   switch(*address_low)
     {
      case 0x10:
    update_count();
    vi_register.vi_current = (vi_register.vi_delay-(next_vi-Count))/1500;
    vi_register.vi_current = (vi_register.vi_current&(~1))|vi_field;
    break;
     }
   *rdword = ((unsigned long long int)(*readvi[*address_low])<<32) |
     *readvi[*address_low+4];
}

void write_vi(void)
{
   switch(*address_low)
     {
      case 0x0:
    if (vi_register.vi_status != word)
      {
         update_vi_status(word);
      }
    return;
    break;
      case 0x8:
    if (vi_register.vi_width != word)
      {
         update_vi_width(word);
      }
    return;
    break;
      case 0x10:
    MI_register.mi_intr_reg &= 0xFFFFFFF7;
    check_interupt();
    return;
    break;
     }
   *readvi[*address_low] = word;
}

void update_vi_status(unsigned int word)
{
    vi_register.vi_status = word;
    viStatusChanged();
}

void update_vi_width(unsigned int word)
{
    vi_register.vi_width = word;
    viWidthChanged();
}

void write_vib(void)
{
   int temp;
   switch(*address_low)
     {
      case 0x0:
      case 0x1:
      case 0x2:
      case 0x3:
    temp = vi_register.vi_status;
    *((unsigned char*)&temp
      + ((*address_low&3)^S8) ) = cpu_byte;
    if (vi_register.vi_status != temp)
      {
         update_vi_status(temp);
      }
    return;
    break;
      case 0x8:
      case 0x9:
      case 0xa:
      case 0xb:
    temp = vi_register.vi_status;
    *((unsigned char*)&temp
      + ((*address_low&3)^S8) ) = cpu_byte;
    if (vi_register.vi_width != temp)
      {
         update_vi_width(temp);
      }
    return;
    break;
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
    MI_register.mi_intr_reg &= 0xFFFFFFF7;
    check_interupt();
    return;
    break;
     }
   *((unsigned char*)readvi[*address_low & 0xfffc]
     + ((*address_low&3)^S8) ) = cpu_byte;
}

void write_vih(void)
{
   int temp;
   switch(*address_low)
     {
      case 0x0:
      case 0x2:
    temp = vi_register.vi_status;
    *((unsigned short*)((unsigned char*)&temp
                + ((*address_low&3)^S16) )) = hword;
    if (vi_register.vi_status != temp)
      {
         update_vi_status(temp);
      }
    return;
    break;
      case 0x8:
      case 0xa:
    temp = vi_register.vi_status;
    *((unsigned short*)((unsigned char*)&temp
                + ((*address_low&3)^S16) )) = hword;
    if (vi_register.vi_width != temp)
      {
         update_vi_width(temp);
      }
    return;
    break;
      case 0x10:
      case 0x12:
    MI_register.mi_intr_reg &= 0xFFFFFFF7;
    check_interupt();
    return;
    break;
     }
   *((unsigned short*)((unsigned char*)readvi[*address_low & 0xfffc]
               + ((*address_low&3)^S16) )) = hword;
}

void write_vid(void)
{
   switch(*address_low)
     {
      case 0x0:
    if (vi_register.vi_status != dword >> 32)
      {
         update_vi_status((unsigned int) (dword >> 32));
      }
    vi_register.vi_origin = (unsigned int) (dword & 0xFFFFFFFF);
    return;
    break;
      case 0x8:
    if (vi_register.vi_width != dword >> 32)
      {
         update_vi_width((unsigned int) (dword >> 32));
      }
    vi_register.vi_v_intr = (unsigned int) (dword & 0xFFFFFFFF);
    return;
    break;
      case 0x10:
    MI_register.mi_intr_reg &= 0xFFFFFFF7;
    check_interupt();
    vi_register.vi_burst = (unsigned int) (dword & 0xFFFFFFFF);
    return;
    break;
     }
   *readvi[*address_low] = (unsigned int) (dword >> 32);
   *readvi[*address_low+4] = (unsigned int) (dword & 0xFFFFFFFF);
}

void read_ai(void)
{
   switch(*address_low)
     {
      case 0x4:
    update_count();
    if (ai_register.current_delay != 0 && get_event(AI_INT) != 0 && (get_event(AI_INT)-Count) < 0x80000000)
      *rdword = ((get_event(AI_INT)-Count)*(long long)ai_register.current_len)/
      ai_register.current_delay;
    else
      *rdword = 0;
    return;
    break;
     }
   *rdword = *(readai[*address_low]);
}

void read_aib(void)
{
   unsigned int len;
   switch(*address_low)
     {
      case 0x4:
      case 0x5:
      case 0x6:
      case 0x7:
    update_count();
    if (ai_register.current_delay != 0 && get_event(AI_INT) != 0)
      len = (unsigned int) (((get_event(AI_INT) - Count) * (long long)ai_register.current_len) / ai_register.current_delay);
    else
      len = 0;
    *rdword = *((unsigned char*)&len + ((*address_low&3)^S8) );
    return;
    break;
     }
   *rdword = *((unsigned char*)readai[*address_low & 0xfffc]
           + ((*address_low&3)^S8) );
}

void read_aih(void)
{
   unsigned int len;
   switch(*address_low)
     {
      case 0x4:
      case 0x6:
    update_count();
    if (ai_register.current_delay != 0 && get_event(AI_INT) != 0)
      len = (unsigned int) (((get_event(AI_INT)-Count) * (long long)ai_register.current_len) / ai_register.current_delay);
    else
      len = 0;
    *rdword = *((unsigned short*)((unsigned char*)&len
                 + ((*address_low&3)^S16) ));
    return;
    break;
     }
   *rdword = *((unsigned short*)((unsigned char*)readai[*address_low & 0xfffc]
                 + ((*address_low&3)^S16) ));
}

void read_aid(void)
{
   switch(*address_low)
     {
      case 0x0:
    update_count();
    if (ai_register.current_delay != 0 && get_event(AI_INT) != 0)
      *rdword = ((get_event(AI_INT)-Count)*(long long)ai_register.current_len)/
      ai_register.current_delay;
    else
      *rdword = 0;
    *rdword |= (unsigned long long)ai_register.ai_dram_addr << 32;
    return;
    break;
     }
   *rdword = ((unsigned long long int)(*readai[*address_low])<<32) |
     *readai[*address_low+4];
}

void write_ai(void)
{
   unsigned int delay=0;
   switch(*address_low)
     {
      case 0x4:
    ai_register.ai_len = word;
    aiLenChanged();
    switch(ROM_HEADER->Country_code&0xFF)
      {
       case 0x44:
       case 0x46:
       case 0x49:
       case 0x50:
       case 0x53:
       case 0x55:
       case 0x58:
       case 0x59:
           {
          unsigned int f = 49656530/(ai_register.ai_dacrate+1);
          if (f)
            delay = (unsigned int) (((unsigned long long)ai_register.ai_len * vi_register.vi_delay*50)/(f*4));
           }
         break;
       case 0x37:
       case 0x41:
       case 0x45:
       case 0x4a:
           {
          unsigned int f = 48681812/(ai_register.ai_dacrate+1);
          if (f)
            delay = (unsigned int) (((unsigned long long)ai_register.ai_len*vi_register.vi_delay*60)/(f*4));
           }
         break;
      }
    if (ai_register.ai_status & 0x40000000) // busy
      {
         ai_register.next_delay = delay;
         ai_register.next_len = ai_register.ai_len;
         ai_register.ai_status |= 0x80000000;
      }
    else
      {
         ai_register.current_delay = delay;
         ai_register.current_len = ai_register.ai_len;
         update_count();
         add_interupt_event(AI_INT, delay);
         ai_register.ai_status |= 0x40000000;
      }
    return;
    break;
      case 0xc:
    MI_register.mi_intr_reg &= 0xFFFFFFFB;
    check_interupt();
    return;
    break;
      case 0x10:
    if (ai_register.ai_dacrate != word)
      {
         update_ai_dacrate(word);
      }
    return;
    break;
     }
   *readai[*address_low] = word;
}

void update_ai_dacrate(unsigned int word)
{
    ai_register.ai_dacrate = word;
    switch(ROM_HEADER->Country_code&0xFF)
    {
    case 0x44:
    case 0x46:
    case 0x49:
    case 0x50:
    case 0x53:
    case 0x55:
    case 0x58:
    case 0x59:
        aiDacrateChanged(SYSTEM_PAL);
        break;
    case 0x37:
    case 0x41:
    case 0x45:
    case 0x4a:
        aiDacrateChanged(SYSTEM_NTSC);
        break;
    }
}

void write_aib(void)
{
   int temp;
   unsigned int delay=0;
   switch(*address_low)
     {
      case 0x4:
      case 0x5:
      case 0x6:
      case 0x7:
    temp = ai_register.ai_len;
    *((unsigned char*)&temp
      + ((*address_low&3)^S8) ) = cpu_byte;
    ai_register.ai_len = temp;
    aiLenChanged();
    switch(ROM_HEADER->Country_code&0xFF)
      {
       case 0x44:
       case 0x46:
       case 0x49:
       case 0x50:
       case 0x53:
       case 0x55:
       case 0x58:
       case 0x59:
         delay = (unsigned int) (((unsigned long long)ai_register.ai_len*(ai_register.ai_dacrate+1)*vi_register.vi_delay*50)/49656530);
         break;
       case 0x37:
       case 0x41:
       case 0x45:
       case 0x4a:
         delay = (unsigned int) (((unsigned long long)ai_register.ai_len*(ai_register.ai_dacrate+1)*
              vi_register.vi_delay*60)/48681812);
         break;
      }
    //delay = 0;
    if (ai_register.ai_status & 0x40000000) // busy
      {
         ai_register.next_delay = delay;
         ai_register.next_len = ai_register.ai_len;
         ai_register.ai_status |= 0x80000000;
      }
    else
      {
         ai_register.current_delay = delay;
         ai_register.current_len = ai_register.ai_len;
         update_count();
         add_interupt_event(AI_INT, delay/2);
         ai_register.ai_status |= 0x40000000;
      }
    return;
    break;
      case 0xc:
      case 0xd:
      case 0xe:
      case 0xf:
    MI_register.mi_intr_reg &= 0xFFFFFFFB;
    check_interupt();
    return;
    break;
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
    temp = ai_register.ai_dacrate;
    *((unsigned char*)&temp
      + ((*address_low&3)^S8) ) = cpu_byte;
    if (ai_register.ai_dacrate != temp)
      {
         update_ai_dacrate(temp);
      }
    return;
    break;
     }
   *((unsigned char*)readai[*address_low & 0xfffc]
     + ((*address_low&3)^S8) ) = cpu_byte;
}

void write_aih(void)
{
   int temp;
   unsigned int delay=0;
   switch(*address_low)
     {
      case 0x4:
      case 0x6:
    temp = ai_register.ai_len;
    *((unsigned short*)((unsigned char*)&temp
                + ((*address_low&3)^S16) )) = hword;
    ai_register.ai_len = temp;
    aiLenChanged();
    switch(ROM_HEADER->Country_code&0xFF)
      {
       case 0x44:
       case 0x46:
       case 0x49:
       case 0x50:
       case 0x53:
       case 0x55:
       case 0x58:
       case 0x59:
         delay = (unsigned int) (((unsigned long long)ai_register.ai_len*(ai_register.ai_dacrate+1)*
              vi_register.vi_delay*50)/49656530);
         break;
       case 0x37:
       case 0x41:
       case 0x45:
       case 0x4a:
         delay = (unsigned int) (((unsigned long long)ai_register.ai_len*(ai_register.ai_dacrate+1)*
              vi_register.vi_delay*60)/48681812);
         break;
      }
    if (ai_register.ai_status & 0x40000000) // busy
      {
         ai_register.next_delay = delay;
         ai_register.next_len = ai_register.ai_len;
         ai_register.ai_status |= 0x80000000;
      }
    else
      {
         ai_register.current_delay = delay;
         ai_register.current_len = ai_register.ai_len;
         update_count();
         add_interupt_event(AI_INT, delay/2);
         ai_register.ai_status |= 0x40000000;
      }
    return;
    break;
      case 0xc:
      case 0xe:
    MI_register.mi_intr_reg &= 0xFFFFFFFB;
    check_interupt();
    return;
    break;
      case 0x10:
      case 0x12:
    temp = ai_register.ai_dacrate;
    *((unsigned short*)((unsigned char*)&temp
                + ((*address_low&3)^S16) )) = hword;
    if (ai_register.ai_dacrate != temp)
      {
         update_ai_dacrate(temp);
      }
    return;
    break;
     }
   *((unsigned short*)((unsigned char*)readai[*address_low & 0xfffc]
               + ((*address_low&3)^S16) )) = hword;
}

void write_aid(void)
{
   unsigned int delay=0;
   switch(*address_low)
     {
      case 0x0:
    ai_register.ai_dram_addr = (unsigned int) (dword >> 32);
    ai_register.ai_len = (unsigned int) (dword & 0xFFFFFFFF);
    aiLenChanged();
    switch(ROM_HEADER->Country_code&0xFF)
      {
       case 0x44:
       case 0x46:
       case 0x49:
       case 0x50:
       case 0x53:
       case 0x55:
       case 0x58:
       case 0x59:
         delay = (unsigned int) (((unsigned long long)ai_register.ai_len*(ai_register.ai_dacrate+1)*
              vi_register.vi_delay*50)/49656530);
         break;
       case 0x37:
       case 0x41:
       case 0x45:
       case 0x4a:
         delay = (unsigned int) (((unsigned long long)ai_register.ai_len*(ai_register.ai_dacrate+1)*
              vi_register.vi_delay*60)/48681812);
         break;
      }
    if (ai_register.ai_status & 0x40000000) // busy
      {
         ai_register.next_delay = delay;
         ai_register.next_len = ai_register.ai_len;
         ai_register.ai_status |= 0x80000000;
      }
    else
      {
         ai_register.current_delay = delay;
         ai_register.current_len = ai_register.ai_len;
         update_count();
         add_interupt_event(AI_INT, delay/2);
         ai_register.ai_status |= 0x40000000;
      }
    return;
    break;
      case 0x8:
    ai_register.ai_control = (unsigned int) (dword >> 32);
    MI_register.mi_intr_reg &= 0xFFFFFFFB;
    check_interupt();
    return;
    break;
      case 0x10:
    if (ai_register.ai_dacrate != dword >> 32)
      {
         update_ai_dacrate((unsigned int) (dword >> 32));
      }
    ai_register.ai_bitrate = (unsigned int) (dword & 0xFFFFFFFF);
    return;
    break;
     }
   *readai[*address_low] = (unsigned int) (dword >> 32);
   *readai[*address_low+4] = (unsigned int) (dword & 0xFFFFFFFF);
}

void read_pi(void)
{
   *rdword = *(readpi[*address_low]);
}

void read_pib(void)
{
   *rdword = *((unsigned char*)readpi[*address_low & 0xfffc]
           + ((*address_low&3)^S8) );
}

void read_pih(void)
{
   *rdword = *((unsigned short*)((unsigned char*)readpi[*address_low & 0xfffc]
                 + ((*address_low&3)^S16) ));
}

void read_pid(void)
{
   *rdword = ((unsigned long long int)(*readpi[*address_low])<<32) |
     *readpi[*address_low+4];
}

void write_pi(void)
{
   switch(*address_low)
     {
      case 0x8:
    pi_register.pi_rd_len_reg = word;
    dma_pi_read();
    return;
    break;
      case 0xc:
    pi_register.pi_wr_len_reg = word;
    dma_pi_write();
    return;
    break;
      case 0x10:
    if (word & 2) MI_register.mi_intr_reg &= 0xFFFFFFEF;
    check_interupt();
    return;
    break;
      case 0x14:
      case 0x18:
      case 0x1c:
      case 0x20:
      case 0x24:
      case 0x28:
      case 0x2c:
      case 0x30:
    *readpi[*address_low] = word & 0xFF;
    return;
    break;
     }
   *readpi[*address_low] = word;
}

void write_pib(void)
{
   switch(*address_low)
     {
      case 0x8:
      case 0x9:
      case 0xa:
      case 0xb:
    *((unsigned char*)&pi_register.pi_rd_len_reg
      + ((*address_low&3)^S8) ) = cpu_byte;
    dma_pi_read();
    return;
    break;
      case 0xc:
      case 0xd:
      case 0xe:
      case 0xf:
    *((unsigned char*)&pi_register.pi_wr_len_reg
      + ((*address_low&3)^S8) ) = cpu_byte;
    dma_pi_write();
    return;
    break;
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
    if (word) MI_register.mi_intr_reg &= 0xFFFFFFEF;
    check_interupt();
    return;
    break;
      case 0x14:
      case 0x15:
      case 0x16:
      case 0x18:
      case 0x19:
      case 0x1a:
      case 0x1c:
      case 0x1d:
      case 0x1e:
      case 0x20:
      case 0x21:
      case 0x22:
      case 0x24:
      case 0x25:
      case 0x26:
      case 0x28:
      case 0x29:
      case 0x2a:
      case 0x2c:
      case 0x2d:
      case 0x2e:
      case 0x30:
      case 0x31:
      case 0x32:
    return;
    break;
     }
   *((unsigned char*)readpi[*address_low & 0xfffc]
     + ((*address_low&3)^S8) ) = cpu_byte;
}

void write_pih(void)
{
   switch(*address_low)
     {
      case 0x8:
      case 0xa:
    *((unsigned short*)((unsigned char*)&pi_register.pi_rd_len_reg
                + ((*address_low&3)^S16) )) = hword;
    dma_pi_read();
    return;
    break;
      case 0xc:
      case 0xe:
    *((unsigned short*)((unsigned char*)&pi_register.pi_wr_len_reg
                + ((*address_low&3)^S16) )) = hword;
    dma_pi_write();
    return;
    break;
      case 0x10:
      case 0x12:
    if (word) MI_register.mi_intr_reg &= 0xFFFFFFEF;
    check_interupt();
    return;
    break;
      case 0x16:
      case 0x1a:
      case 0x1e:
      case 0x22:
      case 0x26:
      case 0x2a:
      case 0x2e:
      case 0x32:
    *((unsigned short*)((unsigned char*)readpi[*address_low & 0xfffc]
               + ((*address_low&3)^S16) )) = hword & 0xFF;
    return;
    break;
      case 0x14:
      case 0x18:
      case 0x1c:
      case 0x20:
      case 0x24:
      case 0x28:
      case 0x2c:
      case 0x30:
    return;
    break;
     }
   *((unsigned short*)((unsigned char*)readpi[*address_low & 0xfffc]
               + ((*address_low&3)^S16) )) = hword;
}

void write_pid(void)
{
   switch(*address_low)
     {
      case 0x8:
    pi_register.pi_rd_len_reg = (unsigned int) (dword >> 32);
    dma_pi_read();
    pi_register.pi_wr_len_reg = (unsigned int) (dword & 0xFFFFFFFF);
    dma_pi_write();
    return;
    break;
      case 0x10:
    if (word) MI_register.mi_intr_reg &= 0xFFFFFFEF;
    check_interupt();
    *readpi[*address_low+4] = (unsigned int) (dword & 0xFF);
    return;
    break;
      case 0x18:
      case 0x20:
      case 0x28:
      case 0x30:
    *readpi[*address_low] = (unsigned int) (dword >> 32) & 0xFF;
    *readpi[*address_low+4] = (unsigned int) (dword & 0xFF);
    return;
    break;
     }
   *readpi[*address_low] = (unsigned int) (dword >> 32);
   *readpi[*address_low+4] = (unsigned int) (dword & 0xFFFFFFFF);
}

void read_ri(void)
{
   *rdword = *(readri[*address_low]);
}

void read_rib(void)
{
   *rdword = *((unsigned char*)readri[*address_low & 0xfffc]
           + ((*address_low&3)^S8) );
}

void read_rih(void)
{
   *rdword = *((unsigned short*)((unsigned char*)readri[*address_low & 0xfffc]
                 + ((*address_low&3)^S16) ));
}

void read_rid(void)
{
   *rdword = ((unsigned long long int)(*readri[*address_low])<<32) |
     *readri[*address_low+4];
}

void write_ri(void)
{
   *readri[*address_low] = word;
}

void write_rib(void)
{
   *((unsigned char*)readri[*address_low & 0xfffc]
     + ((*address_low&3)^S8) ) = cpu_byte;
}

void write_rih(void)
{
   *((unsigned short*)((unsigned char*)readri[*address_low & 0xfffc]
               + ((*address_low&3)^S16) )) = hword;
}

void write_rid(void)
{
   *readri[*address_low] = (unsigned int) (dword >> 32);
   *readri[*address_low+4] = (unsigned int) (dword & 0xFFFFFFFF);
}

void read_si(void)
{
   *rdword = *(readsi[*address_low]);
}

void read_sib(void)
{
   *rdword = *((unsigned char*)readsi[*address_low & 0xfffc]
           + ((*address_low&3)^S8) );
}

void read_sih(void)
{
   *rdword = *((unsigned short*)((unsigned char*)readsi[*address_low & 0xfffc]
                 + ((*address_low&3)^S16) ));
}

void read_sid(void)
{
   *rdword = ((unsigned long long int)(*readsi[*address_low])<<32) |
     *readsi[*address_low+4];
}

void write_si(void)
{
   switch(*address_low)
     {
      case 0x0:
    si_register.si_dram_addr = word;
    return;
    break;
      case 0x4:
    si_register.si_pif_addr_rd64b = word;
    dma_si_read();
    return;
    break;
      case 0x10:
    si_register.si_pif_addr_wr64b = word;
    dma_si_write();
    return;
    break;
      case 0x18:
    MI_register.mi_intr_reg &= 0xFFFFFFFD;
    si_register.si_stat &= ~0x1000;
    check_interupt();
    return;
    break;
     }
}

void write_sib(void)
{
   switch(*address_low)
     {
      case 0x0:
      case 0x1:
      case 0x2:
      case 0x3:
    *((unsigned char*)&si_register.si_dram_addr
      + ((*address_low&3)^S8) ) = cpu_byte;
    return;
    break;
      case 0x4:
      case 0x5:
      case 0x6:
      case 0x7:
    *((unsigned char*)&si_register.si_pif_addr_rd64b
      + ((*address_low&3)^S8) ) = cpu_byte;
    dma_si_read();
    return;
    break;
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
    *((unsigned char*)&si_register.si_pif_addr_wr64b
      + ((*address_low&3)^S8) ) = cpu_byte;
    dma_si_write();
    return;
    break;
      case 0x18:
      case 0x19:
      case 0x1a:
      case 0x1b:
    MI_register.mi_intr_reg &= 0xFFFFFFFD;
    si_register.si_stat &= ~0x1000;
    check_interupt();
    return;
    break;
     }
}

void write_sih(void)
{
   switch(*address_low)
     {
      case 0x0:
      case 0x2:
    *((unsigned short*)((unsigned char*)&si_register.si_dram_addr
                + ((*address_low&3)^S16) )) = hword;
    return;
    break;
      case 0x4:
      case 0x6:
    *((unsigned short*)((unsigned char*)&si_register.si_pif_addr_rd64b
                + ((*address_low&3)^S16) )) = hword;
    dma_si_read();
    return;
    break;
      case 0x10:
      case 0x12:
    *((unsigned short*)((unsigned char*)&si_register.si_pif_addr_wr64b
                + ((*address_low&3)^S16) )) = hword;
    dma_si_write();
    return;
    break;
      case 0x18:
      case 0x1a:
    MI_register.mi_intr_reg &= 0xFFFFFFFD;
    si_register.si_stat &= ~0x1000;
    check_interupt();
    return;
    break;
     }
}

void write_sid(void)
{
   switch(*address_low)
     {
      case 0x0:
    si_register.si_dram_addr = (unsigned int) (dword >> 32);
    si_register.si_pif_addr_rd64b = (unsigned int) (dword & 0xFFFFFFFF);
    dma_si_read();
    return;
    break;
      case 0x10:
    si_register.si_pif_addr_wr64b = (unsigned int) (dword >> 32);
    dma_si_write();
    return;
    break;
      case 0x18:
    MI_register.mi_intr_reg &= 0xFFFFFFFD;
    si_register.si_stat &= ~0x1000;
    check_interupt();
    return;
    break;
     }
}

void read_flashram_status(void)
{
   if (use_flashram != -1 && *address_low == 0)
     {
    *rdword = flashram_status();
    use_flashram = 1;
     }
   else
     DebugMessage(M64MSG_ERROR, "unknown read in read_flashram_status()");
}

void read_flashram_statusb(void)
{
   DebugMessage(M64MSG_ERROR, "read_flashram_statusb() not implemented");
}

void read_flashram_statush(void)
{
   DebugMessage(M64MSG_ERROR, "read_flashram_statush() not implemented");
}

void read_flashram_statusd(void)
{
   DebugMessage(M64MSG_ERROR, "read_flashram_statusd() not implemented");
}

void write_flashram_dummy(void)
{
}

void write_flashram_dummyb(void)
{
}

void write_flashram_dummyh(void)
{
}

void write_flashram_dummyd(void)
{
}

void write_flashram_command(void)
{
   if (use_flashram != -1 && *address_low == 0)
     {
    flashram_command(word);
    use_flashram = 1;
     }
   else
     DebugMessage(M64MSG_ERROR, "unknown write in write_flashram_command()");
}

void write_flashram_commandb(void)
{
   DebugMessage(M64MSG_ERROR, "write_flashram_commandb() not implemented");
}

void write_flashram_commandh(void)
{
   DebugMessage(M64MSG_ERROR, "write_flashram_commandh() not implemented");
}

void write_flashram_commandd(void)
{
   DebugMessage(M64MSG_ERROR, "write_flashram_commandd() not implemented");
}

static unsigned int lastwrite = 0;

void read_rom(void)
{
   if (lastwrite)
     {
    *rdword = lastwrite;
    lastwrite = 0;
     }
   else
     *rdword = *((unsigned int *)(rom + (address & 0x03FFFFFF)));
}

void read_romb(void)
{
   *rdword = *(rom + ((address^S8) & 0x03FFFFFF));
}

void read_romh(void)
{
   *rdword = *((unsigned short *)(rom + ((address^S16) & 0x03FFFFFF)));
}

void read_romd(void)
{
   *rdword = ((unsigned long long)(*((unsigned int *)(rom+(address&0x03FFFFFF))))<<32)|
     *((unsigned int *)(rom + ((address+4)&0x03FFFFFF)));
}

void write_rom(void)
{
   lastwrite = word;
}

void read_pif(void)
{
   if ((*address_low > 0x7FF) || (*address_low < 0x7C0))
   {
       DebugMessage(M64MSG_ERROR, "reading a word in PIF at invalid address 0x%x", address);
       *rdword = 0;
       return;
   }

   *rdword = sl(*((unsigned int *)(PIF_RAMb + (address & 0x7FF) - 0x7C0)));
}

void read_pifb(void)
{
   if ((*address_low > 0x7FF) || (*address_low < 0x7C0))
   {
       DebugMessage(M64MSG_ERROR, "reading a byte in PIF at invalid address 0x%x", address);
       *rdword = 0;
       return;
   }

   *rdword = *(PIF_RAMb + ((address & 0x7FF) - 0x7C0));
}

void read_pifh(void)
{
   if ((*address_low > 0x7FF) || (*address_low < 0x7C0))
   {
       DebugMessage(M64MSG_ERROR, "reading a hword in PIF at invalid address 0x%x", address);
       *rdword = 0;
       return;
   }

   *rdword = (*(PIF_RAMb + ((address & 0x7FF) - 0x7C0)) << 8) |
     *(PIF_RAMb + (((address+1) & 0x7FF) - 0x7C0));
}

void read_pifd(void)
{
   if ((*address_low > 0x7FF) || (*address_low < 0x7C0))
   {
       DebugMessage(M64MSG_ERROR, "reading a double word in PIF in invalid address 0x%x", address);
       *rdword = 0;
       return;
   }

   *rdword = ((unsigned long long)sl(*((unsigned int *)(PIF_RAMb + (address & 0x7FF) - 0x7C0))) << 32)|
     sl(*((unsigned int *)(PIF_RAMb + ((address+4) & 0x7FF) - 0x7C0)));
}

void write_pif(void)
{
   if ((*address_low > 0x7FF) || (*address_low < 0x7C0))
   {
       DebugMessage(M64MSG_ERROR, "writing a word in PIF at invalid address 0x%x", address);
       return;
   }

   *((unsigned int *)(PIF_RAMb + (address & 0x7FF) - 0x7C0)) = sl(word);
   if ((address & 0x7FF) == 0x7FC)
     {
    if (PIF_RAMb[0x3F] == 0x08)
      {
         PIF_RAMb[0x3F] = 0;
         update_count();
         add_interupt_event(SI_INT, /*0x100*/0x900);
      }
    else
      update_pif_write();
     }
}

void write_pifb(void)
{
   if ((*address_low > 0x7FF) || (*address_low < 0x7C0))
   {
       DebugMessage(M64MSG_ERROR, "writing a byte in PIF at invalid address 0x%x", address);
       return;
   }

   *(PIF_RAMb + (address & 0x7FF) - 0x7C0) = cpu_byte;
   if ((address & 0x7FF) == 0x7FF)
     {
    if (PIF_RAMb[0x3F] == 0x08)
      {
         PIF_RAMb[0x3F] = 0;
         update_count();
         add_interupt_event(SI_INT, /*0x100*/0x900);
      }
    else
      update_pif_write();
     }
}

void write_pifh(void)
{
   if ((*address_low > 0x7FF) || (*address_low < 0x7C0))
   {
       DebugMessage(M64MSG_ERROR, "writing a hword in PIF at invalid address 0x%x", address);
       return;
   }

   *(PIF_RAMb + (address & 0x7FF) - 0x7C0) = hword >> 8;
   *(PIF_RAMb + ((address+1) & 0x7FF) - 0x7C0) = hword & 0xFF;
   if ((address & 0x7FF) == 0x7FE)
     {
    if (PIF_RAMb[0x3F] == 0x08)
      {
         PIF_RAMb[0x3F] = 0;
         update_count();
         add_interupt_event(SI_INT, /*0x100*/0x900);
      }
    else
      update_pif_write();
     }
}

void write_pifd(void)
{
   if ((*address_low > 0x7FF) || (*address_low < 0x7C0))
   {
       DebugMessage(M64MSG_ERROR, "writing a double word in PIF at 0x%x", address);
       return;
   }

   *((unsigned int *)(PIF_RAMb + (address & 0x7FF) - 0x7C0)) =
     sl((unsigned int)(dword >> 32));
   *((unsigned int *)(PIF_RAMb + (address & 0x7FF) - 0x7C0)) =
     sl((unsigned int)(dword & 0xFFFFFFFF));
   if ((address & 0x7FF) == 0x7F8)
     {
    if (PIF_RAMb[0x3F] == 0x08)
      {
         PIF_RAMb[0x3F] = 0;
         update_count();
         add_interupt_event(SI_INT, /*0x100*/0x900);
      }
    else
      update_pif_write();
     }
}

