/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - dbg_memory.c                                            *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 DarkJeztr                                          *
 *   Copyright (C) 2002 Blight                                             *
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

#include <string.h>

#include "dbg_types.h"
#include "dbg_memory.h"
#include "dbg_breakpoints.h"

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "memory/memory.h"
#include "r4300/r4300.h"
#include "r4300/ops.h"
#include "main/rom.h"

#if !defined(NO_ASM) && (defined(__i386__) || defined(__x86_64__))

#include <dis-asm.h>
#include <stdarg.h>

int  lines_recompiled;
uint32 addr_recompiled;
int  num_decoded;

char opcode_recompiled[564][MAX_DISASSEMBLY];
char args_recompiled[564][MAX_DISASSEMBLY*4];
void *opaddr_recompiled[564];

disassemble_info dis_info;

#define CHECK_MEM(address) \
   if (!invalid_code[(address) >> 12] && blocks[(address) >> 12]->block[((address) & 0xFFF) / 4].ops != NOTCOMPILED) \
     invalid_code[(address) >> 12] = 1;

void process_opcode_out(void *strm, const char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  char *arg;
  char buff[256];
  
  if(num_decoded==0)
    {
      if(strcmp(fmt,"%s")==0)
    {
      arg = va_arg(ap, char*);
      strcpy(opcode_recompiled[lines_recompiled],arg);
    }
      else
    strcpy(opcode_recompiled[lines_recompiled],"OPCODE-X");
      num_decoded++;
      *(args_recompiled[lines_recompiled])=0;
    }
  else
    {
      vsprintf(buff, fmt, ap);
      sprintf(args_recompiled[lines_recompiled],"%s%s", 
          args_recompiled[lines_recompiled],buff);
    }
  va_end(ap);
}

// Callback function that will be called by libopcodes to read the 
// bytes to disassemble ('read_memory_func' member of 'disassemble_info').
int read_memory_func(bfd_vma memaddr, bfd_byte *myaddr, 
                            unsigned int length, disassemble_info *info) {
  char* from = (char*)(long)(memaddr);
  char* to =   (char*)myaddr;
  
  while (length-- != 0) {
    *to++ = *from++;
  }
  return (0);
}

void init_host_disassembler(void){


  INIT_DISASSEMBLE_INFO(dis_info, stderr, process_opcode_out);
  dis_info.fprintf_func = (fprintf_ftype) process_opcode_out;
  dis_info.stream = stderr;
  dis_info.bytes_per_line=1;
  dis_info.endian = 1;
  dis_info.mach = bfd_mach_i386_i8086;
  dis_info.disassembler_options = (char*) "i386,suffix";
  dis_info.read_memory_func = read_memory_func;
}

void decode_recompiled(uint32 addr)
{
    unsigned char *assemb, *end_addr;

    lines_recompiled=0;

    if(blocks[addr>>12] == NULL)
        return;

    if(blocks[addr>>12]->block[(addr&0xFFF)/4].ops == NOTCOMPILED)
    //      recompile_block((int *) SP_DMEM, blocks[addr>>12], addr);
      {
    strcpy(opcode_recompiled[0],"INVLD");
    strcpy(args_recompiled[0],"NOTCOMPILED");
    opaddr_recompiled[0] = (void *) 0;
    addr_recompiled=0;
    lines_recompiled++;
    return;
      }

    assemb = (blocks[addr>>12]->code) + 
      (blocks[addr>>12]->block[(addr&0xFFF)/4].local_addr);

    end_addr = blocks[addr>>12]->code;

    if( (addr & 0xFFF) >= 0xFFC)
        end_addr += blocks[addr>>12]->code_length;
    else
        end_addr += blocks[addr>>12]->block[(addr&0xFFF)/4+1].local_addr;

    while(assemb < end_addr)
      {
        opaddr_recompiled[lines_recompiled] = assemb;
        num_decoded=0;

        assemb += print_insn_i386((bfd_vma)(long) assemb, &dis_info);

        lines_recompiled++;
      }

    addr_recompiled = addr;
}

char* get_recompiled_opcode(uint32 addr, int index)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    if(index < lines_recompiled)
        return opcode_recompiled[index];
    else
        return NULL;
}

char* get_recompiled_args(uint32 addr, int index)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    if(index < lines_recompiled)
        return args_recompiled[index];
    else
        return NULL;
}

void * get_recompiled_addr(uint32 addr, int index)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    if(index < lines_recompiled)
        return opaddr_recompiled[index];
    else
        return 0;
}

int get_num_recompiled(uint32 addr)
{
    if(addr != addr_recompiled)
        decode_recompiled(addr);

    return lines_recompiled;
}

int get_has_recompiled(uint32 addr)
{
    unsigned char *assemb, *end_addr;

    if(r4300emu != CORE_DYNAREC || blocks[addr>>12] == NULL)
        return FALSE;

    assemb = (blocks[addr>>12]->code) + 
      (blocks[addr>>12]->block[(addr&0xFFF)/4].local_addr);

    end_addr = blocks[addr>>12]->code;

    if( (addr & 0xFFF) >= 0xFFC)
        end_addr += blocks[addr>>12]->code_length;
    else
        end_addr += blocks[addr>>12]->block[(addr&0xFFF)/4+1].local_addr;
    if(assemb==end_addr)
      return FALSE;

    return TRUE;
}

#else

#define CHECK_MEM(address)

char* get_recompiled(uint32 addr, int index)
{
    return NULL;
}

int get_num_recompiled(uint32 addr)
{
    return 0;
}

char* get_recompiled_opcode(uint32 addr, int index)
{
    return NULL;
}

char* get_recompiled_args(uint32 addr, int index)
{
    return NULL;
}

void * get_recompiled_addr(uint32 addr, int index)
{
    return 0;
}

int get_has_recompiled(uint32 addr)
{
    return 0;
}

void init_host_disassembler(void)
{

}

#endif


void update_memory(void){
  int i;
  for(i=0; i<0x10000; i++)
    get_memory_flags(i*0x10000);
}

uint64 read_memory_64(uint32 addr)
{
    return ((uint64)read_memory_32(addr) << 32) | (uint64)read_memory_32(addr + 4);
}

uint64 read_memory_64_unaligned(uint32 addr)
{
    uint64 w[2];
    
    w[0] = read_memory_32_unaligned(addr);
    w[1] = read_memory_32_unaligned(addr + 4);
    return (w[0] << 32) | w[1];
}

void write_memory_64(uint32 addr, uint64 value)
{
    write_memory_32(addr, (uint32) (value >> 32));
    write_memory_32(addr + 4, (uint32) (value & 0xFFFFFFFF));
}

void write_memory_64_unaligned(uint32 addr, uint64 value)
{
    write_memory_32_unaligned(addr, (uint32) (value >> 32));
    write_memory_32_unaligned(addr + 4, (uint32) (value & 0xFFFFFFFF));
}

uint32 read_memory_32(uint32 addr){
  switch(get_memory_type(addr))
    {
    case MEM_NOMEM:
      if(tlb_LUT_r[addr>>12])
    return read_memory_32((tlb_LUT_r[addr>>12]&0xFFFFF000)|(addr&0xFFF));
      return MEM_INVALID;
    case MEM_RDRAM:
      return *((uint32 *)(rdramb + (addr & 0xFFFFFF)));
    case MEM_RSPMEM:
      if ((addr & 0xFFFF) < 0x1000)
    return *((uint32 *)(SP_DMEMb + (addr&0xFFF)));
      else if ((addr&0xFFFF) < 0x2000)
    return *((uint32 *)(SP_IMEMb + (addr&0xFFF)));
      else
    return MEM_INVALID;
    case MEM_ROM:
      return *((uint32 *)(rom + (addr & 0x03FFFFFF)));
    default:
      return MEM_INVALID;
    }
}

uint32 read_memory_32_unaligned(uint32 addr)
{
    uint8 i, b[4];
    
    for(i=0; i<4; i++) b[i] = read_memory_32(addr + i);
    return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
}

void write_memory_32(uint32 addr, uint32 value){
  switch(get_memory_type(addr))
    {
    case MEM_RDRAM:
      *((uint32 *)(rdramb + (addr & 0xFFFFFF))) = value;
      CHECK_MEM(addr)
      break;
    }
}

void write_memory_32_unaligned(uint32 addr, uint32 value)
{
    write_memory_8(addr + 3, value >> 24);
    write_memory_8(addr + 2, (value >> 16) & 0xFF);
    write_memory_8(addr + 1, (value >> 8) & 0xFF);
    write_memory_8(addr + 0, value & 0xFF);
}



//read_memory_16_unaligned and write_memory_16_unaligned don't exist because
//read_memory_16 and write_memory_16 work unaligned already.
uint16 read_memory_16(uint32 addr)
{
    return ((uint16)read_memory_8(addr) << 8) | (uint16)read_memory_8(addr+1); //cough cough hack hack
}

void write_memory_16(uint32 addr, uint16 value)
{
    write_memory_8(addr, value >> 8); //this isn't much better
    write_memory_8(addr + 1, value & 0xFF); //then again, it works unaligned
}

uint8 read_memory_8(uint32 addr)
{
    uint32 word;
    
    word = read_memory_32(addr & ~3);
    return (word >> ((3 - (addr & 3)) * 8)) & 0xFF;
}

void write_memory_8(uint32 addr, uint8 value)
{
    uint32 word, mask;
    
    word = read_memory_32(addr & ~3);
    mask = 0xFF << ((3 - (addr & 3)) * 8);
    word = (word & ~mask) | (value << ((3 - (addr & 3)) * 8));
    write_memory_32(addr & ~3, word);
}

uint32 get_memory_flags(uint32 addr){
  int type=get_memory_type(addr);
  uint32 flags = 0;

  switch(type)
    {
    case MEM_NOMEM:
      if(tlb_LUT_r[addr>>12])
    flags|=MEM_FLAG_READABLE;
      break;
    case MEM_RSPMEM:
      if((addr & 0xFFFF) < 0x2000)
    flags|=MEM_FLAG_READABLE;
      break;
    case MEM_RDRAM:
    case MEM_ROM:
      flags|=MEM_FLAG_READABLE;
    }

  switch(type)
    {
    case MEM_RDRAM:
      flags|=MEM_FLAG_WRITABLE;
    }

  return flags;
}

int get_memory_type(uint32 addr){
  void (*readfunc)() = readmem[addr >> 16];

  if((readfunc == read_nomem) || (readfunc == read_nomem_break))
    return MEM_NOMEM;
  else if((readfunc == read_nothing) || (readfunc == read_nothing_break))
    return MEM_NOTHING;
  else if((readfunc == read_rdram) || (readfunc == read_rdram_break))
    return MEM_RDRAM;
  else if((readfunc == read_rdramreg) || (readfunc == read_rdramreg_break))
    return MEM_RDRAMREG;
  else if((readfunc == read_rsp_mem) || (readfunc == read_rsp_mem_break))
    return MEM_RSPMEM;
  else if((readfunc == read_rsp_reg) || (readfunc == read_rsp_reg_break))
    return MEM_RSPREG;
  else if((readfunc == read_rsp) || (readfunc == read_rsp_break))
    return MEM_RSP;
  else if((readfunc == read_dp) || (readfunc == read_dp_break))
    return MEM_DP;
  else if((readfunc == read_dps) || (readfunc == read_dps_break))
    return MEM_DPS;
  else if((readfunc == read_vi) || (readfunc == read_vi_break))
    return MEM_VI;
  else if((readfunc == read_ai) || (readfunc == read_ai_break))
    return MEM_AI;
  else if((readfunc == read_pi) || (readfunc == read_pi_break))
    return MEM_PI;
  else if((readfunc == read_ri) || (readfunc == read_ri_break))
    return MEM_RI;
  else if((readfunc == read_si) || (readfunc == read_si_break))
    return MEM_SI;
  else if((readfunc == read_flashram_status) ||
          (readfunc == read_flashram_status_break))
    return MEM_FLASHRAMSTAT;
  else if((readfunc == read_rom) || (readfunc == read_rom_break))
    return MEM_ROM;
  else if((readfunc == read_pif) || (readfunc == read_pif_break))
    return MEM_PIF;
  else if((readfunc == read_mi) || (readfunc == read_mi_break))
    return MEM_MI;
  else
    DebugMessage(M64MSG_ERROR, "Unknown memory type in debugger get_memory_type(): %x", readfunc);
  return MEM_NOMEM;
}

void activate_memory_break_read(uint32 addr) {
    void (*readfunc)() = readmem[addr >> 16];

    if(readfunc == read_nomem) {
        readmem[addr >> 16]  = read_nomem_break;
        readmemb[addr >> 16] = read_nomemb_break;
        readmemh[addr >> 16] = read_nomemh_break;
        readmemd[addr >> 16] = read_nomemd_break;
    }
    else if(readfunc == read_nothing) {
        readmem[addr >> 16]  = read_nothing_break;
        readmemb[addr >> 16] = read_nothingb_break;
        readmemh[addr >> 16] = read_nothingh_break;
        readmemd[addr >> 16] = read_nothingd_break;
    }
    else if(readfunc == read_rdram) {
        readmem[addr >> 16]  = read_rdram_break;
        readmemb[addr >> 16] = read_rdramb_break;
        readmemh[addr >> 16] = read_rdramh_break;
        readmemd[addr >> 16] = read_rdramd_break;
    }
    else if(readfunc == read_rdramFB) {
        readmem[addr >> 16]  = read_rdramFB_break;
        readmemb[addr >> 16] = read_rdramFBb_break;
        readmemh[addr >> 16] = read_rdramFBh_break;
        readmemd[addr >> 16] = read_rdramFBd_break;
    }
    else if(readfunc == read_rdramreg) {
        readmem[addr >> 16]  = read_rdramreg_break;
        readmemb[addr >> 16] = read_rdramregb_break;
        readmemh[addr >> 16] = read_rdramregh_break;
        readmemd[addr >> 16] = read_rdramregd_break;
    }
    else if(readfunc == read_rsp_mem) {
        readmem[addr >> 16]  = read_rsp_mem_break;
        readmemb[addr >> 16] = read_rsp_memb_break;
        readmemh[addr >> 16] = read_rsp_memh_break;
        readmemd[addr >> 16] = read_rsp_memd_break;
    }
    else if(readfunc == read_rsp_reg) {
        readmem[addr >> 16] = read_rsp_reg_break;
        readmemb[addr >> 16] = read_rsp_regh_break;
        readmemh[addr >> 16] = read_rsp_regb_break;
        readmemd[addr >> 16] = read_rsp_regd_break;
    }
    else if(readfunc == read_rsp) {
        readmem[addr >> 16] = read_rsp_break;
        readmemb[addr >> 16] = read_rsph_break;
        readmemh[addr >> 16] = read_rspb_break;
        readmemd[addr >> 16] = read_rspd_break;
    }
    else if(readfunc == read_dp) {
        readmem[addr >> 16] = read_dp_break;
        readmemb[addr >> 16] = read_dpb_break;
        readmemh[addr >> 16] = read_dph_break;
        readmemd[addr >> 16] = read_dpd_break;
    }
    else if(readfunc == read_dps) {
        readmem[addr >> 16] = read_dps_break;
        readmemb[addr >> 16] = read_dpsb_break;
        readmemh[addr >> 16] = read_dpsh_break;
        readmemd[addr >> 16] = read_dpsd_break;
    }
    else if(readfunc == read_mi) {
        readmem[addr >> 16] = read_mi_break;
        readmemb[addr >> 16] = read_mib_break;
        readmemh[addr >> 16] = read_mih_break;
        readmemd[addr >> 16] = read_mid_break;
    }
    else if(readfunc == read_vi) {
        readmem[addr >> 16] = read_vi_break;
        readmemb[addr >> 16] = read_vib_break;
        readmemh[addr >> 16] = read_vih_break;
        readmemd[addr >> 16] = read_vid_break;
    }
    else if(readfunc == read_ai) {
        readmem[addr >> 16] = read_ai_break;
        readmemb[addr >> 16] = read_aib_break;
        readmemh[addr >> 16] = read_aih_break;
        readmemd[addr >> 16] = read_aid_break;
    }
    else if(readfunc == read_pi) {
        readmem[addr >> 16] = read_pi_break;
        readmemb[addr >> 16] = read_pib_break;
        readmemh[addr >> 16] = read_pih_break;
        readmemd[addr >> 16] = read_pid_break;
    }
    else if(readfunc == read_ri) {
        readmem[addr >> 16] = read_ri_break;
        readmemb[addr >> 16] = read_rib_break;
        readmemh[addr >> 16] = read_rih_break;
        readmemd[addr >> 16] = read_rid_break;
    }
    else if(readfunc == read_si) {
        readmem[addr >> 16] = read_si_break;
        readmemb[addr >> 16] = read_sib_break;
        readmemh[addr >> 16] = read_sih_break;
        readmemd[addr >> 16] = read_sid_break;
    }
    else if(readfunc == read_pif) {
        readmem[addr >> 16] = read_pif_break;
        readmemb[addr >> 16] = read_pifb_break;
        readmemh[addr >> 16] = read_pifh_break;
        readmemd[addr >> 16] = read_pifd_break;
    }
    else if(readfunc == read_flashram_status) {
        readmem[addr >> 16] = read_flashram_status_break;
        readmemb[addr >> 16] = read_flashram_statusb_break;
        readmemh[addr >> 16] = read_flashram_statush_break;
        readmemd[addr >> 16] = read_flashram_statusd_break;
    }
    else if(readfunc == read_rom) {
        readmem[addr >> 16] = read_rom_break;
        readmemb[addr >> 16] = read_romb_break;
        readmemh[addr >> 16] = read_romh_break;
        readmemd[addr >> 16] = read_romd_break;
   }
   else
        DebugMessage(M64MSG_ERROR, "Unknown memory type in debugger activate_memory_break_read(): %x", readfunc);
}

void deactivate_memory_break_read(uint32 addr) {
    void (*readfunc)() = readmem[addr >> 16];

    if(readfunc == read_nomem_break) {
        readmem[addr >> 16]  = read_nomem;
        readmemb[addr >> 16] = read_nomemb;
        readmemh[addr >> 16] = read_nomemh;
        readmemd[addr >> 16] = read_nomemd;
    }
    else if(readfunc == read_nothing_break) {
        readmem[addr >> 16]  = read_nothing;
        readmemb[addr >> 16] = read_nothingb;
        readmemh[addr >> 16] = read_nothingh;
        readmemd[addr >> 16] = read_nothingd;
    }
    else if(readfunc == read_rdram_break) {
        readmem[addr >> 16]  = read_rdram;
        readmemb[addr >> 16] = read_rdramb;
        readmemh[addr >> 16] = read_rdramh;
        readmemd[addr >> 16] = read_rdramd;
    }
    else if(readfunc == read_rdramFB_break) {
        readmem[addr >> 16]  = read_rdramFB;
        readmemb[addr >> 16] = read_rdramFBb;
        readmemh[addr >> 16] = read_rdramFBh;
        readmemd[addr >> 16] = read_rdramFBd;
    }
    else if(readfunc == read_rdramreg_break) {
        readmem[addr >> 16]  = read_rdramreg;
        readmemb[addr >> 16] = read_rdramregb;
        readmemh[addr >> 16] = read_rdramregh;
        readmemd[addr >> 16] = read_rdramregd;
    }
    else if(readfunc == read_rsp_mem_break) {
        readmem[addr >> 16]  = read_rsp_mem;
        readmemb[addr >> 16] = read_rsp_memb;
        readmemh[addr >> 16] = read_rsp_memh;
        readmemd[addr >> 16] = read_rsp_memd;
    }
    else if(readfunc == read_rsp_reg_break) {
        readmem[addr >> 16] = read_rsp_reg;
        readmemb[addr >> 16] = read_rsp_regh;
        readmemh[addr >> 16] = read_rsp_regb;
        readmemd[addr >> 16] = read_rsp_regd;
    }
    else if(readfunc == read_rsp_break) {
        readmem[addr >> 16] = read_rsp;
        readmemb[addr >> 16] = read_rsph;
        readmemh[addr >> 16] = read_rspb;
        readmemd[addr >> 16] = read_rspd;
    }
    else if(readfunc == read_dp_break) {
        readmem[addr >> 16] = read_dp;
        readmemb[addr >> 16] = read_dpb;
        readmemh[addr >> 16] = read_dph;
        readmemd[addr >> 16] = read_dpd;
    }
    else if(readfunc == read_dps_break) {
        readmem[addr >> 16] = read_dps;
        readmemb[addr >> 16] = read_dpsb;
        readmemh[addr >> 16] = read_dpsh;
        readmemd[addr >> 16] = read_dpsd;
    }
    else if(readfunc == read_mi_break) {
        readmem[addr >> 16] = read_mi;
        readmemb[addr >> 16] = read_mib;
        readmemh[addr >> 16] = read_mih;
        readmemd[addr >> 16] = read_mid;
    }
    else if(readfunc == read_vi_break) {
        readmem[addr >> 16] = read_vi;
        readmemb[addr >> 16] = read_vib;
        readmemh[addr >> 16] = read_vih;
        readmemd[addr >> 16] = read_vid;
    }
    else if(readfunc == read_ai_break) {
        readmem[addr >> 16] = read_ai;
        readmemb[addr >> 16] = read_aib;
        readmemh[addr >> 16] = read_aih;
        readmemd[addr >> 16] = read_aid;
    }
    else if(readfunc == read_pi_break) {
        readmem[addr >> 16] = read_pi;
        readmemb[addr >> 16] = read_pib;
        readmemh[addr >> 16] = read_pih;
        readmemd[addr >> 16] = read_pid;
    }
    else if(readfunc == read_ri_break) {
        readmem[addr >> 16] = read_ri;
        readmemb[addr >> 16] = read_rib;
        readmemh[addr >> 16] = read_rih;
        readmemd[addr >> 16] = read_rid;
    }
    else if(readfunc == read_si_break) {
        readmem[addr >> 16] = read_si;
        readmemb[addr >> 16] = read_sib;
        readmemh[addr >> 16] = read_sih;
        readmemd[addr >> 16] = read_sid;
    }
    else if(readfunc == read_pif_break) {
        readmem[addr >> 16] = read_pif;
        readmemb[addr >> 16] = read_pifb;
        readmemh[addr >> 16] = read_pifh;
        readmemd[addr >> 16] = read_pifd;
    }
    else if(readfunc == read_flashram_status_break) {
        readmem[addr >> 16] = read_flashram_status;
        readmemb[addr >> 16] = read_flashram_statusb;
        readmemh[addr >> 16] = read_flashram_statush;
        readmemd[addr >> 16] = read_flashram_statusd;
    }
    else if(readfunc == read_rom_break) {
        readmem[addr >> 16] = read_rom;
        readmemb[addr >> 16] = read_romb;
        readmemh[addr >> 16] = read_romh;
        readmemd[addr >> 16] = read_romd;
    }
    else
        DebugMessage(M64MSG_ERROR, "Unknown memory type in debugger deactivate_memory_break_read(): %x", readfunc);
}

void activate_memory_break_write(uint32 addr) {
    void (*writefunc)() = writemem[addr >> 16];

    if(writefunc == write_nomem) {
        writemem[addr >> 16]  = write_nomem_break;
        writememb[addr >> 16] = write_nomemb_break;
        writememh[addr >> 16] = write_nomemh_break;
        writememd[addr >> 16] = write_nomemd_break;
    }
    else if(writefunc == write_nothing) {
        writemem[addr >> 16]  = write_nothing_break;
        writememb[addr >> 16] = write_nothingb_break;
        writememh[addr >> 16] = write_nothingh_break;
        writememd[addr >> 16] = write_nothingd_break;
    }
    else if(writefunc == write_rdram) {
        writemem[addr >> 16]  = write_rdram_break;
        writememb[addr >> 16] = write_rdramb_break;
        writememh[addr >> 16] = write_rdramh_break;
        writememd[addr >> 16] = write_rdramd_break;
    }
    else if(writefunc == write_rdramFB) {
        writemem[addr >> 16]  = write_rdramFB_break;
        writememb[addr >> 16] = write_rdramFBb_break;
        writememh[addr >> 16] = write_rdramFBh_break;
        writememd[addr >> 16] = write_rdramFBd_break;
    }
    else if(writefunc == write_rdramreg) {
        writemem[addr >> 16]  = write_rdramreg_break;
        writememb[addr >> 16] = write_rdramregb_break;
        writememh[addr >> 16] = write_rdramregh_break;
        writememd[addr >> 16] = write_rdramregd_break;
    }
    else if(writefunc == write_rsp_mem) {
        writemem[addr >> 16]  = write_rsp_mem_break;
        writememb[addr >> 16] = write_rsp_memb_break;
        writememh[addr >> 16] = write_rsp_memh_break;
        writememd[addr >> 16] = write_rsp_memd_break;
    }
    else if(writefunc == write_rsp_reg) {
        writemem[addr >> 16] = write_rsp_reg_break;
        writememb[addr >> 16] = write_rsp_regh_break;
        writememh[addr >> 16] = write_rsp_regb_break;
        writememd[addr >> 16] = write_rsp_regd_break;
    }
    else if(writefunc == write_rsp) {
        writemem[addr >> 16] = write_rsp_break;
        writememb[addr >> 16] = write_rsph_break;
        writememh[addr >> 16] = write_rspb_break;
        writememd[addr >> 16] = write_rspd_break;
    }
    else if(writefunc == write_dp) {
        writemem[addr >> 16] = write_dp_break;
        writememb[addr >> 16] = write_dpb_break;
        writememh[addr >> 16] = write_dph_break;
        writememd[addr >> 16] = write_dpd_break;
    }
    else if(writefunc == write_dps) {
        writemem[addr >> 16] = write_dps_break;
        writememb[addr >> 16] = write_dpsb_break;
        writememh[addr >> 16] = write_dpsh_break;
        writememd[addr >> 16] = write_dpsd_break;
    }
    else if(writefunc == write_mi) {
        writemem[addr >> 16] = write_mi_break;
        writememb[addr >> 16] = write_mib_break;
        writememh[addr >> 16] = write_mih_break;
        writememd[addr >> 16] = write_mid_break;
    }
    else if(writefunc == write_vi) {
        writemem[addr >> 16] = write_vi_break;
        writememb[addr >> 16] = write_vib_break;
        writememh[addr >> 16] = write_vih_break;
        writememd[addr >> 16] = write_vid_break;
    }
    else if(writefunc == write_ai) {
        writemem[addr >> 16] = write_ai_break;
        writememb[addr >> 16] = write_aib_break;
        writememh[addr >> 16] = write_aih_break;
        writememd[addr >> 16] = write_aid_break;
    }
    else if(writefunc == write_pi) {
        writemem[addr >> 16] = write_pi_break;
        writememb[addr >> 16] = write_pib_break;
        writememh[addr >> 16] = write_pih_break;
        writememd[addr >> 16] = write_pid_break;
    }
    else if(writefunc == write_ri) {
        writemem[addr >> 16] = write_ri_break;
        writememb[addr >> 16] = write_rib_break;
        writememh[addr >> 16] = write_rih_break;
        writememd[addr >> 16] = write_rid_break;
    }
    else if(writefunc == write_si) {
        writemem[addr >> 16] = write_si_break;
        writememb[addr >> 16] = write_sib_break;
        writememh[addr >> 16] = write_sih_break;
        writememd[addr >> 16] = write_sid_break;
    }
    else if(writefunc == write_pif) {
        writemem[addr >> 16] = write_pif_break;
        writememb[addr >> 16] = write_pifb_break;
        writememh[addr >> 16] = write_pifh_break;
        writememd[addr >> 16] = write_pifd_break;
    }
    else if(writefunc == write_flashram_dummy) {
        writemem[addr >> 16] = write_flashram_dummy_break;
        writememb[addr >> 16] = write_flashram_dummyb_break;
        writememh[addr >> 16] = write_flashram_dummyh_break;
        writememd[addr >> 16] = write_flashram_dummyd_break;
    }
    else if(writefunc == write_flashram_command) {
        writemem[addr >> 16] = write_flashram_command_break;
        writememb[addr >> 16] = write_flashram_commandb_break;
        writememh[addr >> 16] = write_flashram_commandh_break;
        writememd[addr >> 16] = write_flashram_commandd_break;
    }
    else if(writefunc == write_rom) {
        writemem[addr >> 16] = write_rom_break;
        writememb[addr >> 16] = write_nothingb_break;
        writememh[addr >> 16] = write_nothingh_break;
        writememd[addr >> 16] = write_nothingd_break;
   }
   else
        DebugMessage(M64MSG_ERROR, "Unknown memory type in debugger activate_memory_break_write(): %x", writefunc);
}

void deactivate_memory_break_write(uint32 addr) {
    void (*writefunc)() = writemem[addr >> 16];

    if(writefunc == write_nomem_break) {
        writemem[addr >> 16]  = write_nomem;
        writememb[addr >> 16] = write_nomemb;
        writememh[addr >> 16] = write_nomemh;
        writememd[addr >> 16] = write_nomemd;
    }
    else if(writefunc == write_nothing_break) {
        writemem[addr >> 16]  = write_nothing;
        writememb[addr >> 16] = write_nothingb;
        writememh[addr >> 16] = write_nothingh;
        writememd[addr >> 16] = write_nothingd;
    }
    else if(writefunc == write_rdram_break) {
        writemem[addr >> 16]  = write_rdram;
        writememb[addr >> 16] = write_rdramb;
        writememh[addr >> 16] = write_rdramh;
        writememd[addr >> 16] = write_rdramd;
    }
    else if(writefunc == write_rdramFB_break) {
        writemem[addr >> 16]  = write_rdramFB;
        writememb[addr >> 16] = write_rdramFBb;
        writememh[addr >> 16] = write_rdramFBh;
        writememd[addr >> 16] = write_rdramFBd;
    }
    else if(writefunc == write_rdramreg_break) {
        writemem[addr >> 16]  = write_rdramreg;
        writememb[addr >> 16] = write_rdramregb;
        writememh[addr >> 16] = write_rdramregh;
        writememd[addr >> 16] = write_rdramregd;
    }
    else if(writefunc == write_rsp_mem_break) {
        writemem[addr >> 16]  = write_rsp_mem;
        writememb[addr >> 16] = write_rsp_memb;
        writememh[addr >> 16] = write_rsp_memh;
        writememd[addr >> 16] = write_rsp_memd;
    }
    else if(writefunc == write_rsp_reg_break) {
        writemem[addr >> 16] = write_rsp_reg;
        writememb[addr >> 16] = write_rsp_regh;
        writememh[addr >> 16] = write_rsp_regb;
        writememd[addr >> 16] = write_rsp_regd;
    }
    else if(writefunc == write_rsp_break) {
        writemem[addr >> 16] = write_rsp;
        writememb[addr >> 16] = write_rsph;
        writememh[addr >> 16] = write_rspb;
        writememd[addr >> 16] = write_rspd;
    }
    else if(writefunc == write_dp_break) {
        writemem[addr >> 16] = write_dp;
        writememb[addr >> 16] = write_dpb;
        writememh[addr >> 16] = write_dph;
        writememd[addr >> 16] = write_dpd;
    }
    else if(writefunc == write_dps_break) {
        writemem[addr >> 16] = write_dps;
        writememb[addr >> 16] = write_dpsb;
        writememh[addr >> 16] = write_dpsh;
        writememd[addr >> 16] = write_dpsd;
    }
    else if(writefunc == write_mi_break) {
        writemem[addr >> 16] = write_mi;
        writememb[addr >> 16] = write_mib;
        writememh[addr >> 16] = write_mih;
        writememd[addr >> 16] = write_mid;
    }
    else if(writefunc == write_vi_break) {
        writemem[addr >> 16] = write_vi;
        writememb[addr >> 16] = write_vib;
        writememh[addr >> 16] = write_vih;
        writememd[addr >> 16] = write_vid;
    }
    else if(writefunc == write_ai_break) {
        writemem[addr >> 16] = write_ai;
        writememb[addr >> 16] = write_aib;
        writememh[addr >> 16] = write_aih;
        writememd[addr >> 16] = write_aid;
    }
    else if(writefunc == write_pi_break) {
        writemem[addr >> 16] = write_pi;
        writememb[addr >> 16] = write_pib;
        writememh[addr >> 16] = write_pih;
        writememd[addr >> 16] = write_pid;
    }
    else if(writefunc == write_ri_break) {
        writemem[addr >> 16] = write_ri;
        writememb[addr >> 16] = write_rib;
        writememh[addr >> 16] = write_rih;
        writememd[addr >> 16] = write_rid;
    }
    else if(writefunc == write_si_break) {
        writemem[addr >> 16] = write_si;
        writememb[addr >> 16] = write_sib;
        writememh[addr >> 16] = write_sih;
        writememd[addr >> 16] = write_sid;
    }
    else if(writefunc == write_pif_break) {
        writemem[addr >> 16] = write_pif;
        writememb[addr >> 16] = write_pifb;
        writememh[addr >> 16] = write_pifh;
        writememd[addr >> 16] = write_pifd;
    }
    else if(writefunc == write_flashram_dummy_break) {
        writemem[addr >> 16] = write_flashram_dummy;
        writememb[addr >> 16] = write_flashram_dummyb;
        writememh[addr >> 16] = write_flashram_dummyh;
        writememd[addr >> 16] = write_flashram_dummyd;
    }
    else if(writefunc == write_flashram_command_break) {
        writemem[addr >> 16] = write_flashram_command;
        writememb[addr >> 16] = write_flashram_commandb;
        writememh[addr >> 16] = write_flashram_commandh;
        writememd[addr >> 16] = write_flashram_commandd;
    }
    else if(writefunc == write_rom_break) {
        writemem[addr >> 16] = write_rom;
        writememb[addr >> 16] = write_nothingb;
        writememh[addr >> 16] = write_nothingh;
        writememd[addr >> 16] = write_nothingd;
    }
    else
        DebugMessage(M64MSG_ERROR, "Unknown memory type in debugger deactivate_memory_break_write(): %x", writefunc);
}

/* Following are the breakpoint functions for memory access calls.  See debugger/memory.h
 * These macros generate the memory breakpoint function calls.*/
MEMBREAKALL(nothing);
MEMBREAKALL(nomem);
MEMBREAKALL(rdram);
MEMBREAKALL(rdramFB);
MEMBREAKALL(rdramreg);
MEMBREAKALL(rsp_mem);
MEMBREAKALL(rsp_reg);
MEMBREAKALL(rsp);
MEMBREAKALL(dp);
MEMBREAKALL(dps);
MEMBREAKALL(mi);
MEMBREAKALL(vi);
MEMBREAKALL(ai);
MEMBREAKALL(pi);
MEMBREAKALL(ri);
MEMBREAKALL(si);
MEMBREAKALL(pif);

MEMBREAKREAD(read_flashram_status, 4);
MEMBREAKREAD(read_flashram_statusb, 1);
MEMBREAKREAD(read_flashram_statush, 2);
MEMBREAKREAD(read_flashram_statusd, 8);
MEMBREAKWRITE(write_flashram_dummy, 4);
MEMBREAKWRITE(write_flashram_dummyb, 1);
MEMBREAKWRITE(write_flashram_dummyh, 2);
MEMBREAKWRITE(write_flashram_dummyd, 8);
MEMBREAKWRITE(write_flashram_command, 4);
MEMBREAKWRITE(write_flashram_commandb, 1);
MEMBREAKWRITE(write_flashram_commandh, 2);
MEMBREAKWRITE(write_flashram_commandd, 8);

MEMBREAKREAD(read_rom, 4);
MEMBREAKREAD(read_romb, 1);
MEMBREAKREAD(read_romh, 2);
MEMBREAKREAD(read_romd, 8);

MEMBREAKWRITE(write_rom, 8);
