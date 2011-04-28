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

#include "shared.h"
#include "debug.h"
#include <desa68/desa68.h>

namespace MDFN_IEN_MD
{
bool MD_DebugMode = FALSE;

static void (*DriverCPUHook)(uint32);


static c68k_struc Main68K_BP;
static void (*BPCallB)(uint32 PC) = NULL;
static bool BPActive = FALSE; // Any breakpoints on?
static bool BPNonPCActive = FALSE;	// Any breakpoints other than PC on?
static bool FoundBPoint;

typedef struct
{
 uint32 A[2];
 int type;
} MD_BPOINT;

static std::vector<MD_BPOINT> BreakPointsPC, BreakPointsRead, BreakPointsWrite;
static std::vector<MD_BPOINT> BreakPointsAux0Read, BreakPointsAux0Write;

enum
{
 C68K_GSREG_PC = 0,
 C68K_GSREG_SR,

 C68K_GSREG_D0,
 C68K_GSREG_D7 = C68K_GSREG_D0 + 7,

 C68K_GSREG_A0,
 C68K_GSREG_A7 = C68K_GSREG_A0 + 7,


};


static uint32 M68K_GetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 uint32 ret = 0xDEADBEEF;

 if(id == C68K_GSREG_PC)
 {
  ret = C68k_Get_PC(&Main68K);
 } 
 else if(id == C68K_GSREG_SR)
 {
  ret = C68k_Get_SR(&Main68K);
 }
 else if(id >= C68K_GSREG_D0 && id <= C68K_GSREG_D7)
 {
  ret = C68k_Get_DReg(&Main68K, id - C68K_GSREG_D0);
 }
 else if(id >= C68K_GSREG_A0 && id <= C68K_GSREG_A7)
 {
  ret = C68k_Get_AReg(&Main68K, id - C68K_GSREG_A0);
 }
 return(ret);
}

void M68K_SetRegister(const unsigned int id, uint32 value)
{

}


static RegType M68K_Regs[] =
{
        { C68K_GSREG_PC, "PC", "Program Counter", 4 },

        { 0, "------", "", 0xFFFF },

	{ C68K_GSREG_D0 + 0, "D0", "D0(Data Register 0)", 4 },
        { C68K_GSREG_D0 + 1, "D1", "D1(Data Register 1)", 4 },
        { C68K_GSREG_D0 + 2, "D2", "D2(Data Register 2)", 4 },
        { C68K_GSREG_D0 + 3, "D3", "D3(Data Register 3)", 4 },
        { C68K_GSREG_D0 + 4, "D4", "D4(Data Register 4)", 4 },
        { C68K_GSREG_D0 + 5, "D5", "D5(Data Register 5)", 4 },
        { C68K_GSREG_D0 + 6, "D6", "D6(Data Register 6)", 4 },
        { C68K_GSREG_D0 + 7, "D7", "D7(Data Register 7)", 4 },

        { 0, "------", "", 0xFFFF },

        { C68K_GSREG_A0 + 0, "A0", "A0(Address Register 0)", 4 },
        { C68K_GSREG_A0 + 1, "A1", "A1(Address Register 1)", 4 },
        { C68K_GSREG_A0 + 2, "A2", "A2(Address Register 2)", 4 },
        { C68K_GSREG_A0 + 3, "A3", "A3(Address Register 3)", 4 },
        { C68K_GSREG_A0 + 4, "A4", "A4(Address Register 4)", 4 },
        { C68K_GSREG_A0 + 5, "A5", "A5(Address Register 5)", 4 },
        { C68K_GSREG_A0 + 6, "A6", "A6(Address Register 6)", 4 },
        { C68K_GSREG_A0 + 7, "A7", "A7/USP(Address Register 7 / User Stack Pointer)", 4 },

        { 0, "------", "", 0xFFFF },
	{ C68K_GSREG_SR, "SR", "Status Register", 2 },

        { 0, "", "", 0 },
};

static RegGroupType M68K_RegsGroup =
{
	"M68K",
        M68K_Regs,
        M68K_GetRegister,
        M68K_SetRegister
};


uint32 MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical)
{
 uint32 ret = 0;

 MD_HackyHackyMode++;

 for(unsigned int i = 0; i < bsize; i++)
 {
  A &= 0xFFFFFF;
  ret |= MD_ReadMemory8(A) << ((bsize - 1 - i) * 8);

  A++;
 }

 MD_HackyHackyMode--;
 return(ret);
}

static uint16_t dis_callb(uint32_t A, void *private_data)
{
 return(MD_ReadMemory16(A & 0xFFFFFF));
}

void Disassemble(uint32 &a, uint32 SpecialA, char *TextBuf)
{
 DESA68parm_t d;

 a &= 0xFFFFFE;

 //printf("Disassemble %06x: ", a);

 strcpy(TextBuf, "Invalid");
 memset(&d, 0, sizeof(DESA68parm_t));

 d.mem_callb = dis_callb;
 d.memmsk = 0xFFFFFF;
 d.pc = a;
 d.str = TextBuf;
 d.strmax = 255;	// FIXME, MDFN API change

 MD_HackyHackyMode++;
 desa68(&d);
 MD_HackyHackyMode--;

 a = d.pc & 0xFFFFFF;

 if(d.pc & 1)
  puts("Oops");

 if(!d.status)
  strcpy(TextBuf, "Invalid");
 //printf("%d\n", d.status);
// puts(TextBuf);
}

static inline void C68k_Copy_State2(const c68k_struc *source, c68k_struc *dest)
{
 memcpy(&dest->D[0], &source->D[0], ((uint8 *)&(source->dirty1)) - ((uint8 *)&(source->D[0])));
}

void MDDBG_CPUHook(void)	//uint32 PC, uint16 op)
{
 uint32 PC = C68k_Get_PC(&Main68K);
 std::vector<MD_BPOINT>::iterator bpit;

 FoundBPoint = 0;

 for(bpit = BreakPointsPC.begin(); bpit != BreakPointsPC.end(); bpit++)
 {
  if(PC >= bpit->A[0] && PC <= bpit->A[1])
  {
   FoundBPoint = TRUE;
   break;
  }
 }

 if(BPNonPCActive)
 {
  MD_HackyHackyMode++;

  C68k_Copy_State2(&Main68K, &Main68K_BP);

  //printf("Moo: %08x\n", C68k_Get_PC(&Main68K_BP)); //, (int)(((uint8 *)&(Main68K.dirty1)) - ((uint8 *)&(Main68K.D[0]))));

  C68k_Exec(&Main68K_BP);

  MD_HackyHackyMode--;
 }

 if(FoundBPoint)
  BPCallB(PC);

 if(DriverCPUHook)
  DriverCPUHook(PC);
}

static void RedoCPUHook(void)
{
 //C68k_Set_Debug(&Main68K, DriverCPUHook ? CPUHookHandler : NULL);

 BPNonPCActive = BreakPointsRead.size() || BreakPointsWrite.size() || BreakPointsAux0Read.size() || BreakPointsAux0Write.size();
 BPActive = BPNonPCActive || BreakPointsPC.size();

 MD_DebugMode = (DriverCPUHook || BPActive);
}

static void AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical)
{
 MD_BPOINT tmp;

 tmp.A[0] = A1;
 tmp.A[1] = A2;
 tmp.type = type;

 if(type == BPOINT_READ)
  BreakPointsRead.push_back(tmp);
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.push_back(tmp);
 else if(type == BPOINT_IO_READ)
  BreakPointsAux0Read.push_back(tmp);
 else if(type == BPOINT_AUX_WRITE)
  BreakPointsAux0Write.push_back(tmp);
 else if(type == BPOINT_PC)
  BreakPointsPC.push_back(tmp);

 RedoCPUHook();
}


void FlushBreakPoints(int type)
{
 std::vector<MD_BPOINT>::iterator bpit;

 if(type == BPOINT_READ)
  BreakPointsRead.clear();
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.clear();
 else if(type == BPOINT_AUX_READ)
  BreakPointsAux0Read.clear();
 else if(type == BPOINT_AUX_WRITE)
  BreakPointsAux0Write.clear();
 else if(type == BPOINT_PC)
  BreakPointsPC.clear();

 RedoCPUHook();
}

void SetCPUCallback(void (*callb)(uint32 PC))
{
 DriverCPUHook = callb;
 RedoCPUHook();
}

void SetBPCallback(void (*callb)(uint32 PC))
{
 BPCallB = callb;
}

std::vector<std::string> GetBranchTrace(void)
{
 std::vector<std::string> ret;

 return(ret);
}

static void GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 uint32 ws = 0;

 MD_HackyHackyMode++;

 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFFFF;
   *Buffer = MD_ReadMemory8(Address);
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "ram"))
 {
  while(Length--)
  {
   *Buffer = MD_ReadMemory8((Address & 0xFFFF) | 0xFF0000);
   Address++;
   Buffer++;
  }
 }

 MD_HackyHackyMode--;

}

static void PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFFFF;
  }
 }
}

static int dbg_int_ack_callback(int int_level)
{
    return C68K_INT_ACK_AUTOVECTOR;
}

static uint8 MDDBG_ReadMemory8(uint32 address)
{
 std::vector<MD_BPOINT>::iterator bpit;
 address &= 0xFFFFFF;

 for(bpit = BreakPointsRead.begin(); bpit != BreakPointsRead.end(); bpit++)
 {
  if(address >= bpit->A[0] && address <= bpit->A[1])
  {
   FoundBPoint = TRUE;
   break;
  }
 }


 return(MD_ReadMemory8(address));
}

static uint16 MDDBG_ReadMemory16(uint32 address)
{
 std::vector<MD_BPOINT>::iterator bpit;

 address &= 0xFFFFFF;

 for(bpit = BreakPointsRead.begin(); bpit != BreakPointsRead.end(); bpit++)
 {
  if((address | 1) >= bpit->A[0] && address <= bpit->A[1])
  {
   FoundBPoint = TRUE;
   break;
  }
 }

 //printf("Read: %08x\n", address);

 return(MD_ReadMemory16(address));
}

static void MDDBG_WriteMemory8(uint32 address, uint8 value)
{
 std::vector<MD_BPOINT>::iterator bpit;

 address &= 0xFFFFFF;

 for(bpit = BreakPointsWrite.begin(); bpit != BreakPointsWrite.end(); bpit++)
 {
  if(address >= bpit->A[0] && address <= bpit->A[1])
  {
   FoundBPoint = TRUE;
   break;
  }
 }



}

static void MDDBG_WriteMemory16(uint32 address, uint16 value)
{
 std::vector<MD_BPOINT>::iterator bpit;

 address &= 0xFFFFFF;


 for(bpit = BreakPointsWrite.begin(); bpit != BreakPointsWrite.end(); bpit++)
 {
  if((address | 1) >= bpit->A[0] && address <= bpit->A[1])
  {
   FoundBPoint = TRUE;
   break;
  }
 }


}



bool MDDBG_Init(void)
{
 MDFNDBG_AddRegGroup(&M68K_RegsGroup);

 ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "cpu", "CPU Physical", 24);
 ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "ram", "Work RAM", 16);

 
 C68k_Init(&Main68K_BP, dbg_int_ack_callback);

 C68k_Set_TAS_Hack(&Main68K_BP, 1);

 C68k_Set_ReadB(&Main68K_BP, MDDBG_ReadMemory8);
 C68k_Set_ReadW(&Main68K_BP, MDDBG_ReadMemory16);

 C68k_Set_WriteB(&Main68K_BP, MDDBG_WriteMemory8);
 C68k_Set_WriteW(&Main68K_BP, MDDBG_WriteMemory16);

 MD_DebugMode = FALSE;

 return(TRUE);
}

DebuggerInfoStruct DBGInfo =
{
 "shift_jis",
 10,		// Max instruction size(bytes)
 2,		// Instruction alignment(bytes)
 24,		// Logical address bits
 24,		// Physical address bits
 0xFF0000,	// Default watch address
 ~0,		// ZP

 MemPeek,
 Disassemble,
 NULL,
 NULL, // IRQ,
 NULL, // NESDBG_GetVector,
 FlushBreakPoints,
 AddBreakPoint,
 SetCPUCallback,
 SetBPCallback,
 GetBranchTrace,
 NULL, //SetGraphicsDecode,
 NULL, //GetGraphicsDecodeBuffer,
 NULL, //SetLogFunc,
};



};
