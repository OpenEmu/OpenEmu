/* Cygne
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Dox dox@space.pl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "wswan.h"
#include "v30mz.h"
#include "debug.h"
#include "dis/disasm.h"
#include "memory.h"
#include "gfx.h"
#include <stdarg.h>
#include <trio/trio.h>

namespace MDFN_IEN_WSWAN
{

#define NUMBT 32

static int BTIndex = 0;

struct BTEntry
{
 uint16 from_CS;
 uint16 from_IP;
 uint16 to_CS;
 uint16 to_IP;

 bool interrupt;
 uint32 branch_count;
};

static BTEntry BTEntries[NUMBT];

struct WSWAN_BPOINT
{
        unsigned int A[2];
        int type;
	bool logical;
};

static std::vector<WSWAN_BPOINT> BreakPointsPC, BreakPointsRead, BreakPointsWrite, BreakPointsIORead, BreakPointsIOWrite, BreakPointsAux0Read, BreakPointsAux0Write;

static void (*CPUHook)(uint32 PC) = NULL;
static bool FoundBPoint = 0;
static void (*BPCallB)(uint32 PC) = NULL;

void WSwanDBG_IRQ(int level)
{
 if(level >= 0 && level < 8)
 {
  //WSwan_Interrupt(level);
  WSwan_InterruptDebugForce(level);
 }
}

static uint8 ReadHandler(uint32 A)
{
 std::vector<WSWAN_BPOINT>::iterator bpit;

 for(bpit = BreakPointsRead.begin(); bpit != BreakPointsRead.end(); bpit++)
 {
  unsigned int testA = A;

  if(testA >= bpit->A[0] && testA <= bpit->A[1])
  {
   FoundBPoint = 1;
   break;
  }
 }

 return(WSwan_readmem20(A));
}

static void WriteHandler(uint32 A, uint8 V)
{
 std::vector<WSWAN_BPOINT>::iterator bpit;

 for(bpit = BreakPointsWrite.begin(); bpit != BreakPointsWrite.end(); bpit++)
 {
  unsigned int testA = A;

  if(testA >= bpit->A[0] && testA <= bpit->A[1])
  {
   FoundBPoint = 1;
   break;
  }
 }
}

static uint8 PortReadHandler(uint32 A)
{
 std::vector<WSWAN_BPOINT>::iterator bpit;

 for(bpit = BreakPointsIORead.begin(); bpit != BreakPointsIORead.end(); bpit++)
 {
  unsigned int testA = A & 0xFF;

  if(testA >= (bpit->A[0] & 0xFF) && testA <= (bpit->A[1] & 0xFF))
  {
   FoundBPoint = 1;
   break;
  }
 }

 return(WSwan_readport(A));
}

static void PortWriteHandler(uint32 A, uint8 V)
{
 std::vector<WSWAN_BPOINT>::iterator bpit;

 for(bpit = BreakPointsIOWrite.begin(); bpit != BreakPointsIOWrite.end(); bpit++)
 {
  unsigned int testA = A & 0xFF;

  if(testA >= (bpit->A[0] & 0xFF) && testA <= (bpit->A[1] & 0xFF))
  {
   FoundBPoint = 1;
   break;
  }
 }
}

static void CPUHandler(uint32 PC)
{
 std::vector<WSWAN_BPOINT>::iterator bpit;

 if(!FoundBPoint)
  for(bpit = BreakPointsPC.begin(); bpit != BreakPointsPC.end(); bpit++)
  {
   if(PC >= bpit->A[0] && PC <= bpit->A[1])
   {
    BPCallB(PC);
    break;
   }
  }

 if(FoundBPoint)
 {
  BPCallB(PC);
  FoundBPoint = 0;
 }

 if(CPUHook)
  CPUHook(PC);
}

static void RedoDH(void)
{
 bool needch = BreakPointsPC.size() || BreakPointsRead.size() || BreakPointsAux0Read.size() || 
	BreakPointsWrite.size() || BreakPointsAux0Write.size() ||
	BreakPointsIORead.size() || BreakPointsIOWrite.size();

 v30mz_debug(needch ? CPUHandler : CPUHook,
        (BreakPointsRead.size() || BreakPointsAux0Read.size()) ? ReadHandler : NULL,
        (BreakPointsWrite.size() || BreakPointsAux0Write.size()) ? WriteHandler : 0,
	(BreakPointsIORead.size()) ? PortReadHandler : NULL,
	(BreakPointsIOWrite.size()) ? PortWriteHandler : NULL,
        (CPUHook) ? WSwanDBG_AddBranchTrace : NULL);
}

void WSwanDBG_SetCPUCallback(void (*callb)(uint32 PC))
{
 CPUHook = callb;
 RedoDH();
}

void WSwanDBG_SetBPCallback(void (*callb)(uint32 PC))
{
 BPCallB = callb;
}

void WSwanDBG_FlushBreakPoints(int type)
{
 if(type == BPOINT_READ)
  BreakPointsRead.clear();
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.clear();
 else if(type == BPOINT_IO_READ)
  BreakPointsIORead.clear();
 else if(type == BPOINT_IO_WRITE)
  BreakPointsIOWrite.clear();
 else if(type == BPOINT_PC)
  BreakPointsPC.clear();
 else if(type == BPOINT_AUX_READ)
  BreakPointsAux0Read.clear();
 else if(type == BPOINT_AUX_WRITE)
  BreakPointsAux0Write.clear();

 RedoDH();
}

void WSwanDBG_AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical)
{
 WSWAN_BPOINT tmp;

 tmp.A[0] = A1;
 tmp.A[1] = A2;
 tmp.type =type;
 tmp.logical = logical;

 if(type == BPOINT_READ)
  BreakPointsRead.push_back(tmp);
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.push_back(tmp);
 else if(type == BPOINT_IO_READ)
  BreakPointsIORead.push_back(tmp);
 else if(type == BPOINT_IO_WRITE)
  BreakPointsIOWrite.push_back(tmp);
 else if(type == BPOINT_PC)
  BreakPointsPC.push_back(tmp);
 else if(type == BPOINT_AUX_READ)
  BreakPointsAux0Read.push_back(tmp);
 else if(type == BPOINT_AUX_WRITE)
  BreakPointsAux0Write.push_back(tmp);

 RedoDH();
}

uint32 WSwanDBG_MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical)
{
 uint32 ss = v30mz_get_reg(NEC_SS);
 uint32 ret = 0;

 for(unsigned int i = 0; i < bsize; i++)
 {
  uint8 zebyte;

  if(logical)
   zebyte = WSwan_readmem20(((ss << 4) + A + i) & 0xFFFFF);
  else
   zebyte = WSwan_readmem20((A + i) & 0xFFFFF);
  ret |= zebyte << (i * 8);
 }

 return(ret);
}

void WSwanDBG_AddBranchTrace(uint16 from_CS, uint16 from_IP, uint16 to_CS, uint16 to_IP, bool interrupt)
{
 BTEntry *prevbt = &BTEntries[(BTIndex + NUMBT - 1) % NUMBT];

 if(prevbt->from_CS == from_CS && prevbt->to_CS == to_CS &&
    prevbt->from_IP == from_IP && prevbt->to_IP == to_IP &&
	 prevbt->interrupt == interrupt && prevbt->branch_count < 0xFFFFFFFF)
  prevbt->branch_count++;
 else
 {
  BTEntries[BTIndex].from_CS = from_CS;
  BTEntries[BTIndex].to_CS = to_CS;
  BTEntries[BTIndex].from_IP = from_IP;
  BTEntries[BTIndex].to_IP = to_IP;

  BTEntries[BTIndex].interrupt = interrupt;
  BTEntries[BTIndex].branch_count = 1;

  BTIndex = (BTIndex + 1) % NUMBT;
 }
}

std::vector<BranchTraceResult> WSwanDBG_GetBranchTrace(void)
{
 BranchTraceResult tmp;
 std::vector<BranchTraceResult> ret;

 for(int x = 0; x < NUMBT; x++)
 {
  const BTEntry *bt = &BTEntries[(x + BTIndex) % NUMBT];

  tmp.count = bt->branch_count;
  trio_snprintf(tmp.from, sizeof(tmp.from), "%04X:%04X", bt->from_CS, bt->from_IP);
  trio_snprintf(tmp.to, sizeof(tmp.to), "%04X:%04X", bt->to_CS, bt->to_IP);
  trio_snprintf(tmp.code, sizeof(tmp.code), bt->interrupt ? "INT" : "");

  ret.push_back(tmp);
 }
 return(ret);
}


void WSwanDBG_CheckBP(int type, uint32 address, unsigned int len)
{


}

void WSwanDBG_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{


}

void WSwanDBG_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, const uint8 *Buffer)
{


}

static disassembler zedis;

void WSwanDBG_Disassemble(uint32 &a, uint32 SpecialA, char *text_buffer)
{
 uint32 mis = MDFNGameInfo->Debugger->MaxInstructionSize;
 mis = 256;
 uint8 instr_buffer[mis];
 uint32 ps = v30mz_get_reg(NEC_PS);
 int consumed;

 for(unsigned int i = 0; i < mis; i++)
 {
  instr_buffer[i] = WSwan_readmem20(((ps << 4) + a + i) & 0xFFFFF);
 }

 consumed = zedis.disasm(0x0000, a, instr_buffer, text_buffer);

 int x;
 for(x = strlen(text_buffer); x < 40; x++)
  text_buffer[x] = ' ';
 text_buffer[x] = 0;

 for(int i = 0; i < consumed; i++)
 {
  char tmp[16];
  trio_snprintf(tmp, 16, " %02x", instr_buffer[i]);
  strcat(text_buffer, tmp);
 }

 for(int i = 1; i < consumed; i++)
  if(((a + i) & 0xFFFF) == SpecialA)
  {
   a = SpecialA;
   strcpy(text_buffer, "BORKBORK");
   return;
  }

 a = (a + consumed) & 0xFFFF;
}

void WSwanDBG_ToggleSyntax(void)
{
 zedis.toggle_syntax_mode();
}



//
//
//

static RegType V30MZ_Regs[] =
{
        { NEC_PC, "IP", "Instruction Pointer", 2 },
        { NEC_FLAGS, "PSW", "Program Status Word", 2 },
        { NEC_AW, "AX", "Accumulator", 2 },
        { NEC_BW, "BX", "Base", 2 },
        { NEC_CW, "CX", "Counter", 2 },
        { NEC_DW, "DX", "Data", 2 },
        { NEC_SP, "SP", "Stack Pointer", 2 },
        { NEC_BP, "BP", "Base Pointer", 2 },
        { NEC_IX, "SI", "Source Index", 2 },
        { NEC_IY, "DI", "Dest Index", 2 },
        { NEC_PS, "CS", "Program Segment", 2 },
        { NEC_SS, "SS", "Stack Segment", 2 },
        { NEC_DS0, "DS", "Data Segment", 2 },
        { NEC_DS1, "ES", "Extra Segment(Destination)", 2 },
        { 0, "", "", 0 },
};


static uint32 V30MZ_GetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 return(v30mz_get_reg(id));
}

static void V30MZ_SetRegister(const unsigned int id, uint32 value)
{
 v30mz_set_reg(id, value);
}


static RegGroupType V30MZRegsGroup =
{
 "V30MZ",
 V30MZ_Regs,
 V30MZ_GetRegister,
 V30MZ_SetRegister,
};

//
//

static RegType MiscRegs[] =
{
 { MEMORY_GSREG_ROMBBSLCT, "ROMBBSLCT", "ROM Bank Base Selector for 64KiB banks 0x4-0xF", 1 },
 { MEMORY_GSREG_BNK1SLCT, "BNK1SLCT", "???", 1 },
 { MEMORY_GSREG_BNK2SLCT, "BNK2SLCT", "ROM Bank Selector for 64KiB bank 0x2", 1 },
 { MEMORY_GSREG_BNK3SLCT, "BNK3SLCT", "ROM Bank Selector for 64KiB bank 0x3", 1 },

 { 0x8000 | INT_GSREG_ISTATUS, "IStatus", "Interrupt Status", 1 },
 { 0x8000 | INT_GSREG_IENABLE, "IEnable", "Interrupt Enable", 1 },
 { 0x8000 | INT_GSREG_IVECTORBASE, "IVectorBase", "Interrupt Vector Base", 1 },
 { 0, "", "", 0 },
};

static uint32 Misc_GetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 if(id & 0x8000)
  return(WSwan_InterruptGetRegister(id & ~0x8000, special, special_len));
 else
  return(WSwan_MemoryGetRegister(id, special, special_len));
} 

static void Misc_SetRegister(const unsigned int id, uint32 value)
{
 if(id & 0x8000)
  WSwan_InterruptSetRegister(id & ~0x8000, value);
 else
  WSwan_MemorySetRegister(id, value);
}


static RegGroupType MiscRegsGroup =
{
 "Misc",
 MiscRegs,
 Misc_GetRegister,
 Misc_SetRegister,
};

void WSwanDBG_Init(void)
{
 MDFNDBG_AddRegGroup(&V30MZRegsGroup);
 MDFNDBG_AddRegGroup(&MiscRegsGroup);
}


}
