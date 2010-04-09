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
#include "dis/disasm.h"
#include "memory.h"
#include "gfx.h"
#include <stdarg.h>
#include <trio/trio.h>

static int BTIndex = 0;
static uint32 BTEntries[16];
static uint32 BTEntriesCS[16];

typedef struct __WSWAN_BPOINT
{
        unsigned int A[2];
        int type;
	bool logical;
} WSWAN_BPOINT;

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
	(BreakPointsIOWrite.size()) ? PortWriteHandler : NULL);
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

uint32 WSwanDBG_GetRegister(const std::string &name, std::string *special)
{
 if(name == "IP")
  return(v30mz_get_reg(NEC_PC));
 if(name == "AX")
  return(v30mz_get_reg(NEC_AW));
 if(name == "BX")
  return(v30mz_get_reg(NEC_BW));
 if(name == "CX")
  return(v30mz_get_reg(NEC_CW));
 if(name == "DX")
  return(v30mz_get_reg(NEC_DW));
 if(name == "SP")
  return(v30mz_get_reg(NEC_SP));
 if(name == "BP")
  return(v30mz_get_reg(NEC_BP));
 if(name == "SI")
  return(v30mz_get_reg(NEC_IX));
 if(name == "DI")
  return(v30mz_get_reg(NEC_IY));
 if(name == "CS")
  return(v30mz_get_reg(NEC_PS));
 if(name == "SS")
  return(v30mz_get_reg(NEC_SS));
 if(name == "DS")
  return(v30mz_get_reg(NEC_DS0));
 if(name == "ES")
  return(v30mz_get_reg(NEC_DS1));

 return(0);
}

void WSwanDBG_SetRegister(const std::string &name, uint32 value)
{
 if(name == "IP")
  v30mz_set_reg(NEC_PC, value);
 else if(name == "AX")
  v30mz_set_reg(NEC_AW, value);
 else if(name == "BX")
  v30mz_set_reg(NEC_BW, value);
 else if(name == "CX")
  v30mz_set_reg(NEC_CW, value);
 else if(name == "DX")
  v30mz_set_reg(NEC_DW, value);
 else if(name == "SP")
  v30mz_set_reg(NEC_SP, value);
 else if(name == "BP")
  v30mz_set_reg(NEC_BP, value);
 else if(name == "SI")
  v30mz_set_reg(NEC_IX, value);
 else if(name == "DI")
  v30mz_set_reg(NEC_IY, value);
 else if(name == "CS")
  v30mz_set_reg(NEC_PS, value);
 else if(name == "SS")
  v30mz_set_reg(NEC_SS, value);
 else if(name == "DS")
  v30mz_set_reg(NEC_DS0, value);
 else if(name == "ES")
  v30mz_set_reg(NEC_DS1, value);
}

void WSwanDBG_AddBranchTrace(uint16 CS, uint16 IP)
{
 if(BTEntries[(BTIndex - 1) & 0xF] == IP && BTEntriesCS[(BTIndex - 1) & 0xF] == CS) return;

 BTEntries[BTIndex] = IP;
 BTEntriesCS[BTIndex] = CS;

 BTIndex = (BTIndex + 1) & 0xF;
}

std::vector<std::string> WSwanDBG_GetBranchTrace(void)
{
 std::vector<std::string> ret;

 for(int x = 0; x < 16; x++)
 {
  char *tmps = trio_aprintf("%04x:%04X", BTEntriesCS[(x + BTIndex) & 0xF], BTEntries[(x + BTIndex) & 0xF]);
  ret.push_back(std::string(tmps));
  free(tmps);
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
  snprintf(tmp, 16, " %02x", instr_buffer[i]);
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
