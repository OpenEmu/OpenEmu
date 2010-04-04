/* Mednafen - Multi-system Emulator
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

#include <string.h>
#include <trio/trio.h>

#include "nes.h"
#include "x6502.h"
#include "debug.h"
#include "ppu/ppu.h"
#include "cart.h"
#include "../dis6502.h"

static int BTIndex = 0;
static uint32 BTEntries[32];

void NESDBG_AddBranchTrace(uint32 PC)
{
 PC &= 0xFFFF;

 if(BTEntries[(BTIndex - 1) & 0x1F] == PC) return;

 BTEntries[BTIndex] = PC;
 BTIndex = (BTIndex + 1) & 0x1F;
}

std::vector<std::string> NESDBG_GetBranchTrace(void)
{
 std::vector<std::string> ret;

 for(int x = 0; x < 32; x++)
 {
  char *tmps = trio_aprintf("%04X", BTEntries[(x + BTIndex) & 0x1F]);
  ret.push_back(std::string(tmps));
  free(tmps);
 }
 return(ret);
}

uint32 NESDBG_MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical)
{
 uint32 ret = 0;

 for(unsigned int i = 0; i < bsize; i++)
 {
  A &= 0xFFFF;

  if(hl)
   fceuindbg = 1;
  ret |= ARead[A](A) << (i * 8);
  fceuindbg = 0;

  A++;
 }
 return(ret);
}

void NESDBG_MemPoke(uint32 A, uint32 V, unsigned int bsize, bool hl, bool logical)
{
 extern uint8 *Page[32];

 for(unsigned int i = 0; i < bsize; i++)
 {
  A &= 0xFFFF;
  if(hl)
  {
   if(Page[A/2048])
    Page[A/2048][A] = V & 0xFF;
  }
  else
   BWrite[A](A,V & 0xFF);

  V >>= 8;
  A++;
 }
}

extern NESGameType *GameInterface;       

void NESDBG_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 if(!strcmp(name, "cpu"))
 {
  fceuindbg = 1;
  while(Length--)
  {
   Address &= 0xFFFF;
   *Buffer = ARead[Address](Address);

   Address++;
   Buffer++;
  }
  fceuindbg = 0;
 }
}

void NESDBG_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 if(!strcmp(name, "cpu"))
 {
  fceuindbg = 1;
  while(Length--)
  {
   Address &= 0xFFFF;
   uint8 *page = GetCartPagePtr(Address);

   if(page && hl)
    page[Address] = *Buffer;
   else
    BWrite[Address](Address,*Buffer);

   Address++;
   Buffer++;
  }
  fceuindbg = 0;
 }
}


void NESDBG_IRQ(int level)
{


}

uint32 NESDBG_GetVector(int level)
{


}

class Dis2A03 : public Dis6502
{
	public:
	Dis2A03(void) : Dis6502(0)
	{

	}

	uint8 GetX(void)
	{
	 return(X.X);
	}

	uint8 GetY(void)
	{
	 return(X.Y);
	}

	uint8 Read(uint16 A)
	{
	 uint8 ret;
	 fceuindbg = 1;
	 ret = ARead[A](A);
	 fceuindbg = 0;
	 return(ret);
	}
};

static Dis2A03 DisObj;

void NESDBG_Disassemble(uint32 &a, uint32 SpecialA, char *TextBuf)
{
	uint16 tmpa = a;
	std::string ret;

	DisObj.Disassemble(tmpa, SpecialA, TextBuf);

	a = tmpa;
}

typedef struct __NES_BPOINT {
	unsigned int A[2];
	int type;
} NES_BPOINT;

static std::vector<NES_BPOINT> BreakPointsPC, BreakPointsRead, BreakPointsWrite;

static void (*CPUHook)(uint32 PC) = NULL;
static bool FoundBPoint = 0;
static void (*BPCallB)(uint32 PC) = NULL;

void NESDBG_TestFoundBPoint(void)
{
 if(FoundBPoint)
 {
  BPCallB(X.PC);
 }
 FoundBPoint = 0;
}

static void CPUHandler(uint32 PC)
{
 std::vector<NES_BPOINT>::iterator bpit;

 if(!FoundBPoint)
  for(bpit = BreakPointsPC.begin(); bpit != BreakPointsPC.end(); bpit++)
  {
   if(PC >= bpit->A[0] && PC <= bpit->A[1])
   {
    BPCallB(PC);
    break;
   }
  }

 if(CPUHook)
  CPUHook(PC);
}

static uint8 ReadHandler(X6502 *X, unsigned int A)
{
 if(X->preexec)
 {
  std::vector<NES_BPOINT>::iterator bpit;

  for(bpit = BreakPointsRead.begin(); bpit != BreakPointsRead.end(); bpit++)
  {
   if(A >= bpit->A[0] && A <= bpit->A[1])
   {
    FoundBPoint = 1;
    break;
   }
  }
 }
 return(ARead[A](A));
}

static void WriteHandler(X6502 *X, unsigned int A, uint8 V)
{
 if(X->preexec)
 {
  std::vector<NES_BPOINT>::iterator bpit;

  for(bpit = BreakPointsWrite.begin(); bpit != BreakPointsWrite.end(); bpit++)
  {
   if(A >= bpit->A[0] && A <= bpit->A[1])
   {
    FoundBPoint = 1;
    break;
   }
  }
 }
 else
  BWrite[A](A,V);
}

void NESDBG_AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical)
{
 NES_BPOINT tmp;

 tmp.A[0] = A1;
 tmp.A[1] = A2;
 tmp.type =type;


 if(type == BPOINT_READ)
  BreakPointsRead.push_back(tmp);
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.push_back(tmp);
 else if(type == BPOINT_PC)
  BreakPointsPC.push_back(tmp);

 X6502_Debug(BreakPointsPC.size() ? CPUHandler : CPUHook, BreakPointsRead.size() ? ReadHandler : NULL, BreakPointsWrite.size() ? WriteHandler : 0);
}

void NESDBG_FlushBreakPoints(int type)
{
 std::vector<NES_BPOINT>::iterator bpit;

 if(type == BPOINT_READ)
  BreakPointsRead.clear();
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.clear();
 else if(type == BPOINT_PC)
  BreakPointsPC.clear();

 X6502_Debug(BreakPointsPC.size() ? CPUHandler : CPUHook, BreakPointsRead.size() ? ReadHandler : NULL, BreakPointsWrite.size() ? WriteHandler : 0);
}

void NESDBG_SetCPUCallback(void (*callb)(uint32 PC))
{
 CPUHook = callb;
 X6502_Debug(BreakPointsPC.size() ? CPUHandler : CPUHook, BreakPointsRead.size() ? ReadHandler : NULL, BreakPointsWrite.size() ? WriteHandler : 0);
}

void NESDBG_SetBPCallback(void (*callb)(uint32 PC))
{
 BPCallB = callb;
}

uint32 NESDBG_GetRegister(const std::string &name, std::string *special)
{
 if(name == "PC")
  return(X.PC);
 else if(name == "A")
  return(X.A);
 else if(name == "X")
  return(X.X);
 else if(name == "Y")
  return(X.Y);
 else if(name == "SP")
  return(X.S);
 else if(name == "P")
  return(X.P);
 else 
  return(NESPPU_GetRegister(name));
}

void NESDBG_SetRegister(const std::string &name, uint32 value)
{
 if(name == "PC")
 {
  X.PC = value & 0xFFFF;
 }
 else if(name == "A")
 {
  X.A = value & 0xFF;
 }
 else if(name == "X")
 {
  X.X = value & 0xFF;
 }
 else if(name == "Y")
 {
  X.Y = value & 0xFF;
 }
 else if(name == "SP")
 {
  X.S = value & 0xFF;
 }
 else if(name == "P")
 {
  X.P = value & 0xFF & ~(B_FLAG | U_FLAG);
 }
 else
  NESPPU_SetRegister(name, value);


}

