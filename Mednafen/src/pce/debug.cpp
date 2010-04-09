/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITPCES FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "pce.h"
#include <string.h>
#include <trio/trio.h>
#include <iconv.h>

#include "huc6280.h"
#include "debug.h"
#include "vdc.h"
#include "huc.h"
#include "../dis6280.h"
#include "../cdrom/scsicd.h"

static int BTIndex = 0;
static uint32 BTEntries[32];

typedef struct __PCE_BPOINT 
{
        unsigned int A[2];
        int type;
        bool logical;
} PCE_BPOINT;

static std::vector<PCE_BPOINT> BreakPointsPC, BreakPointsRead, BreakPointsWrite, BreakPointsAux0Read, BreakPointsAux0Write, BreakPointsOp;

static void (*CPUHook)(uint32 PC) = NULL;
static bool FoundBPoint = 0;
static void (*BPCallB)(uint32 PC) = NULL;
static void (*LogFunc)(const char *, const char *);
bool PCE_LoggingOn = FALSE;
static uint16 LastPC = 0xBEEF;

void PCEDBG_AddBranchTrace(uint32 PC)
{
 PC &= 0xFFFF;

 if(BTEntries[(BTIndex - 1) & 0x1F] == PC) return;

 BTEntries[BTIndex] = PC;
 BTIndex = (BTIndex + 1) & 0x1F;
}


std::vector<std::string> PCEDBG_GetBranchTrace(void)
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

static ALWAYS_INLINE bool TestOpBP(uint8 opcode)
{
 std::vector<PCE_BPOINT>::iterator bpit, bpit_end;

 bpit = BreakPointsOp.begin();
 bpit_end = BreakPointsOp.end();

 for(; bpit != bpit_end; bpit++)
 {
  if(opcode >= bpit->A[0] && opcode <= bpit->A[1])
  {
   return(TRUE);
  }
 }
 return(FALSE);
}

void PCEDBG_CheckBP(int type, uint32 address, unsigned int len)
{
 std::vector<PCE_BPOINT>::iterator bpit, bpit_end;

 if(type == BPOINT_READ)
 {
  bpit = BreakPointsRead.begin();
  bpit_end = BreakPointsRead.end();
 }
 else if(type == BPOINT_WRITE)
 {
  bpit = BreakPointsWrite.begin();
  bpit_end = BreakPointsWrite.end();
 }
 else if(type == BPOINT_AUX_READ)
 {
  bpit = BreakPointsAux0Read.begin();
  bpit_end = BreakPointsAux0Read.end();
 }
 else if(type == BPOINT_AUX_WRITE)
 {
  bpit = BreakPointsAux0Write.begin();
  bpit_end = BreakPointsAux0Write.end();
 }
 else if(type == BPOINT_OP)
 {
  bpit = BreakPointsOp.begin();
  bpit_end = BreakPointsOp.end();
 }
 else
  return;

 for(; bpit != bpit_end; bpit++)
 {
  uint32 tmp_address = address;
  uint32 tmp_len = len;

  while(tmp_len--)
  {
   if(tmp_address >= bpit->A[0] && tmp_address <= bpit->A[1])
   {
    FoundBPoint = TRUE;
    break;
   }
   tmp_address++;
  }
 }
}


uint32 PCEDBG_MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical)
{
 uint32 ret = 0;
 uint8 wmpr;

 PCE_InDebug = 1;

 for(unsigned int i = 0; i < bsize; i++)
 {
  if(logical)
  {
   A &= 0xFFFF;
   wmpr = HuCPU.MPR[A >> 13];
   ret |= PCERead[wmpr]((wmpr << 13) | (A & 0x1FFF)) << (i * 8);
  }
  else
  {
   A &= (1 << 21) - 1;
   ret |= PCERead[A >> 13](A) << (i * 8);
  }

  A++;
 }

 PCE_InDebug = 0;

 return(ret);
}

void PCEDBG_IRQ(int level)
{
 if(level == 0) // NMI
 {

 }
 else if(level == 1)
  HuC6280_IRQBegin(MDFN_IQIRQ1);
 else if(level == 2)
  HuC6280_IRQBegin(MDFN_IQIRQ2);
 else if(level == 3)
  HuC6280_IRQBegin(MDFN_IQTIMER);
  


}

uint32 PCEDBG_GetVector(int level)
{
  

}

class DisPCE : public Dis6280
{
	public:
	DisPCE(void)
	{

	}

	uint8 GetX(void)
	{
	 return(HuCPU.X);
	}

	uint8 GetY(void)
	{
	 return(HuCPU.Y);
	}

	uint8 Read(uint16 A)
	{
	 uint8 ret;
	 uint8 wmpr = HuCPU.MPR[A >> 13];

	 PCE_InDebug = 1;
	 ret = PCERead[wmpr]((wmpr << 13) | (A & 0x1FFF));
	 PCE_InDebug = 0;

	 return(ret);
	}
};

static DisPCE DisObj;

void PCEDBG_Disassemble(uint32 &a, uint32 SpecialA, char *TextBuf)
{
	uint16 tmpa = a;
	std::string ret;

	DisObj.Disassemble(tmpa, SpecialA, TextBuf);

	a = tmpa;
}

void PCEDBG_TestFoundBPoint(void)
{
 if(FoundBPoint)
 {
  BPCallB(HuCPU.PC);
 }
 FoundBPoint = 0;
}

static void CPUHandler(uint32 PC, uint8 opcode)
{
 std::vector<PCE_BPOINT>::iterator bpit;

 if(!FoundBPoint)
 {
  for(bpit = BreakPointsPC.begin(); bpit != BreakPointsPC.end(); bpit++)
  {
   if(PC >= bpit->A[0] && PC <= bpit->A[1])
   {
    BPCallB(PC);
    goto SkipTheDonkeyCat;
   }
  }
  if(TestOpBP(opcode))
   BPCallB(PC);

  SkipTheDonkeyCat: ;
 }

 if(CPUHook)
  CPUHook(PC);

 if(PC == 0xe060 && PCE_LoggingOn && PCE_IsCD)
 {
  PCE_InDebug = 1;
  uint8 wmpr = HuCPU.MPR[0x2000 >> 13];
  uint16 axeme;

  axeme = PCERead[wmpr]((wmpr << 13) | 0x00f8) | (PCERead[wmpr]((wmpr << 13) | 0x00f9) << 8);
  
  PCEDBG_DoLog("BIOS", "Call EX_GETFNT from $%04X, ax=0x%04x = %s", LastPC, axeme, PCEDBG_ShiftJIS_to_UTF8(axeme));
  PCE_InDebug = 0;
 }
 LastPC = PC;
}

static uint8 ReadHandler(HuC6280 *X, unsigned int A)
{
 std::vector<PCE_BPOINT>::iterator bpit;

 if(X->MPR[A >> 13] == 0xFF)
 {
  uint32 NeoA = A & 0x1FFF;

  if(NeoA < 0x400)
  {
   VDC_Read_TestBP(NeoA);
  }
 }

 for(bpit = BreakPointsRead.begin(); bpit != BreakPointsRead.end(); bpit++)
 {
  unsigned int testA;

  if(!bpit->logical)
   testA = (X->MPR[A >> 13] << 13) | (A & 0x1FFF);
  else
   testA = A;

  if(testA >= bpit->A[0] && testA <= bpit->A[1])
  {
   FoundBPoint = 1;
   break;
  }
 }

 uint8 wmpr = HuCPU.MPR[A >> 13];
 return(PCERead[wmpr]((wmpr << 13) | (A & 0x1FFF)));
}

static void WriteHandler(HuC6280 *X, uint32 A, uint8 V)
{
 std::vector<PCE_BPOINT>::iterator bpit;
 uint32 phys_addr;

 // Don't test VDC VRAM write breakpoints here if ST0, ST1, ST2 instructions are used
 // as it is handled directly in huc6280.cpp due to the SuperGrafx control register controlling
 // which VDC ST instructions go to.
 if(A & 0x80000000)
  phys_addr = A & 0x1FFFFF;
 else
 {
  phys_addr = (X->MPR[A >> 13] * 8192) | (A & 0x1FFF);

  if(phys_addr >= (0xFF * 8192) && phys_addr <= (0xFF * 8192 + 0x3FF))
  {
   uint32 NeoA = A & 0x1FFF;

   if(NeoA < 0x400)
   {
    VDC_Write_TestBP(NeoA, V);
   }
  }
 }

 for(bpit = BreakPointsWrite.begin(); bpit != BreakPointsWrite.end(); bpit++)
 {
  unsigned int testA;

  if(!bpit->logical)
   testA = phys_addr;
  else
  {
   if(A & 0x80000000) continue;

   testA = A;
  }

  if(testA >= bpit->A[0] && testA <= bpit->A[1])
  {
   FoundBPoint = 1;
   break;
  }
 }
}

static void RedoDH(void)
{
 HuC6280_Debug((BreakPointsPC.size() || BreakPointsOp.size() || CPUHook || PCE_LoggingOn) ? CPUHandler : NULL,
	(BreakPointsRead.size() || BreakPointsAux0Read.size()) ? ReadHandler : NULL, 
	(BreakPointsWrite.size() || BreakPointsAux0Write.size()) ? WriteHandler : 0);
}

void PCEDBG_AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical)
{
 PCE_BPOINT tmp;

 tmp.A[0] = A1;
 tmp.A[1] = A2;
 tmp.type =type;
 tmp.logical = logical;

 if(type == BPOINT_READ)
  BreakPointsRead.push_back(tmp);
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.push_back(tmp);
 else if(type == BPOINT_PC)
  BreakPointsPC.push_back(tmp);
 else if(type == BPOINT_AUX_READ)
  BreakPointsAux0Read.push_back(tmp);
 else if(type == BPOINT_AUX_WRITE)
  BreakPointsAux0Write.push_back(tmp);
 else if(type == BPOINT_OP)
  BreakPointsOp.push_back(tmp);

 RedoDH();
}

void PCEDBG_FlushBreakPoints(int type)
{
 std::vector<PCE_BPOINT>::iterator bpit;

 if(type == BPOINT_READ)
  BreakPointsRead.clear();
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.clear();
 else if(type == BPOINT_PC)
  BreakPointsPC.clear();
 else if(type == BPOINT_AUX_READ)
  BreakPointsAux0Read.clear();
 else if(type == BPOINT_AUX_WRITE)
  BreakPointsAux0Write.clear();
 else if(type == BPOINT_OP)
  BreakPointsOp.clear();

 RedoDH();
}

void PCEDBG_SetCPUCallback(void (*callb)(uint32 PC))
{
 CPUHook = callb;
 RedoDH();
}

void PCEDBG_SetBPCallback(void (*callb)(uint32 PC))
{
 BPCallB = callb;
}

void PCEDBG_DoLog(const char *type, const char *format, ...)
{
 if(LogFunc)
 {
  char *temp;

  va_list ap;
  va_start(ap, format);

  temp = trio_vaprintf(format, ap);
  LogFunc(type, temp);
  free(temp);

  va_end(ap);
 }
}

static iconv_t sjis_ict = (iconv_t)-1;

void PCEDBG_SetLogFunc(void (*func)(const char *, const char *))
{
 LogFunc = func;

 PCE_LoggingOn = func ? TRUE : FALSE;
 SCSICD_SetLog(func ? PCEDBG_DoLog : NULL);

 if(PCE_LoggingOn)
 {
  if(sjis_ict == (iconv_t)-1)
   sjis_ict = iconv_open("UTF-8", "shift_jis");
 }
 else
 {
  if(sjis_ict != (iconv_t)-1)
  {
   iconv_close(sjis_ict);
   sjis_ict = (iconv_t)-1;
  }
 }
 RedoDH();
}

char *PCEDBG_ShiftJIS_to_UTF8(const uint16 sjc)
{
 static char ret[16];
 char inbuf[3];
 char *in_ptr, *out_ptr;
 size_t ibl, obl;

 ret[0] = 0;

 if(sjc < 256)
 {
  inbuf[0] = sjc;
  inbuf[1] = 0;
  ibl = 1;
 }
 else
 {
  inbuf[0] = sjc >> 8;
  inbuf[1] = sjc >> 0;
  inbuf[2] = 0;
  ibl = 2;
 }

 in_ptr = inbuf;
 out_ptr = ret;
 obl = 16;

 iconv(sjis_ict, (ICONV_CONST char **)&in_ptr, &ibl, &out_ptr, &obl);

 return(ret);
}

uint32 PCEDBG_GetRegister(const std::string &name, std::string *special)
{
 if(name == "PC")
  return(HuCPU.PC & 0xFFFF);
 else if(name == "A")
  return(HuCPU.A);
 else if(name == "X")
  return(HuCPU.X);
 else if(name == "Y")
  return(HuCPU.Y);
 else if(name == "SP")
  return(HuCPU.S);
 else if(name == "P")
 {
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "N: %d, V: %d, T: %d, D: %d, I: %d, Z: %d, C: %d", (int)(bool)(HuCPU.P & N_FLAG), (int)(bool)(HuCPU.P & V_FLAG),
	(int)(bool)(HuCPU.P & T_FLAG),(int)(bool)(HuCPU.P & D_FLAG),(int)(bool)(HuCPU.P & I_FLAG), (int)(bool)(HuCPU.P & Z_FLAG),
	(int)(bool)(HuCPU.P & C_FLAG));
   *special = std::string(buf);
  }
  return(HuCPU.P);
 }
 else if(name == "SPD")
 {
  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "%s(%s)", HuCPU.speed ? "High" : "Low", HuCPU.speed ? "7.16MHz" : "1.79MHz");
   *special = std::string(buf);
  }
  return(HuCPU.speed);
 }
 else if(!strncmp(name.c_str(), "MPR", 3))
 {
  int which = name.c_str()[3] - '0';
  uint8 value = HuCPU.MPR[which];

  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "0x%02X * 0x2000 = 0x%06X", value, (uint32)value * 0x2000);
   *special = std::string(buf);
  }

  return(value);
 }
 else if(name == "IRQM")
 {
  uint8 value = HuCPU.IRQMask ^ 0x7;

  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "IRQ2: %s, IRQ1: %s, Timer: %s", (value & MDFN_IQIRQ2) ? "Disabled" : "Enabled", (value & MDFN_IQIRQ1) ? "Disabled" : "Enabled",
	(value & MDFN_IQTIMER) ? "Disabled" : "Enabled");
   *special = std::string(buf);
  }
  return(value);
 }
 else if(name == "TIMS")
 {
  uint8 value = HuCPU.timer_status;

  if(special)
  {
   char buf[256];
   snprintf(buf, 256, "%s", (value & 1) ? "Enabled" : "Disabled");
   *special = std::string(buf);
  }
  return(value);
 }
 else if(name == "TIMV")
  return(HuCPU.timer_value);
 else if(name == "TIML")
 {
  uint8 value = HuCPU.timer_load;
  if(special)
  {
   char buf[256];
   uint32 meowval = (value + 1) * 1024;
   snprintf(buf, 256, "(%d + 1) * 1024 = %d; 7,159,090.90... Hz / %d = %f Hz", value, meowval, meowval, (double)7159090.909090909091 / meowval);
   *special = std::string(buf);
  }
  return(value);
 }
 else if(name == "TIMD")
  return(HuCPU.timer_div);
 else 
  return(VDC_GetRegister(name, special));
}

void PCEDBG_SetRegister(const std::string &name, uint32 value)
{
 if(name == "PC")
  HuCPU.PC = value & 0xFFFF;
 else if(name == "A")
  HuCPU.A = value & 0xFF;
 else if(name == "X")
  HuCPU.X = value & 0xFF;
 else if(name == "Y")
  HuCPU.Y = value & 0xFF;
 else if(name == "SP")
  HuCPU.S = value & 0xFF;
 else if(name == "P")
  HuCPU.P = value & 0xFF;
 else if(name == "SPD")
 {
  // FIXME later, duplicate macro
  #define REDOSPEEDCACHE() { HuCPU.speed_shift_cache = ((HuCPU.speed ^ 1) << 1); }
  HuCPU.speed = value & 0x01;
  REDOSPEEDCACHE();
 }
 else if(name == "MPR0")
 {
  HuCPU.MPR[0] = value & 0xFF;
  HuC6280_FlushMPRCache();
 }
 else if(name == "MPR1")
 {
  HuCPU.MPR[1] = value & 0xFF;
  HuC6280_FlushMPRCache();
 }
 else if(name == "MPR2")
 {
  HuCPU.MPR[2] = value & 0xFF;
  HuC6280_FlushMPRCache();
 }
 else if(name == "MPR3")
 {
  HuCPU.MPR[3] = value & 0xFF;
  HuC6280_FlushMPRCache();
 }
 else if(name == "MPR4")
 {
  HuCPU.MPR[4] = value & 0xFF;
  HuC6280_FlushMPRCache();
 }
 else if(name == "MPR5")
 {
  HuCPU.MPR[5] = value & 0xFF;
  HuC6280_FlushMPRCache();
 }
 else if(name == "MPR6")
 {
  HuCPU.MPR[6] = value & 0xFF;
  HuC6280_FlushMPRCache();
 }
 else if(name == "MPR7")
 {
  HuCPU.MPR[7] = value & 0xFF;
  HuC6280_FlushMPRCache();
 }
 else if(name == "IRQM")
  HuCPU.IRQMask = (value & 0x7) ^ 0x7;
 else if(name == "TIMS")
  HuCPU.timer_status = value & 0x1;
 else if(name == "TIMV")
  HuCPU.timer_value = value & 0x7F;
 else if(name == "TIML")
  HuCPU.timer_load = value & 0x7F;
 else if(name == "TIMD")
  HuCPU.timer_div = value & 1023;
 else
  VDC_SetRegister(name, value);
}
