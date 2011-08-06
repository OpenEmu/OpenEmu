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

#include "huc6280/huc6280.h"
#include "debug.h"
#include "vce.h"
#include "huc.h"
#include "huc6270/vdc.h"
#include "../cdrom/pcecd.h"
#include "pce_psg/pce_psg.h"
#include "../dis6280.h"
#include "../cdrom/scsicd.h"
#include "arcade_card/arcade_card.h"

namespace MDFN_IEN_PCE
{

static HuC6280 *ShadowCPU = NULL;


extern VCE *vce;
extern ArcadeCard *arcade_card;

static PCE_PSG *psg = NULL;

static bool IsSGX;

static int BTIndex = 0;
static uint32 BTEntries[32];

typedef struct __PCE_BPOINT 
{
        unsigned int A[2];
        int type;
        bool logical;
} PCE_BPOINT;

static std::vector<PCE_BPOINT> BreakPointsRead, BreakPointsWrite, BreakPointsAux0Read, BreakPointsAux0Write;

static uint8 BreakPointsPC[65536 / 8];
static bool BreakPointsPCUsed;

static uint8 BreakPointsOp[256];
static bool BreakPointsOpUsed;


static bool NeedExecSimu;	// Cache variable, recalculated in RedoDH().

static void (*CPUCB)(uint32 PC) = NULL;
static bool FoundBPoint = 0;
static void (*BPCallB)(uint32 PC) = NULL;
static void (*LogFunc)(const char *, const char *);
bool PCE_LoggingOn = FALSE;
static uint16 LastPC = 0xBEEF;

static void AddBranchTrace(uint32 PC)
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

static INLINE bool TestOpBP(uint8 opcode)
{
 return(BreakPointsOp[opcode]);
}

static INLINE bool TestPCBP(uint16 PC)
{
 return((BreakPointsPC[PC >> 3] >> (PC & 0x7)) & 1);
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

 PCE_InDebug++;

 for(unsigned int i = 0; i < bsize; i++)
 {
  if(logical)
  {
   A &= 0xFFFF;

   ret |= HuCPU->PeekLogical(A) << (i * 8);
  }
  else
  {
   A &= (1 << 21) - 1;

   ret |= HuCPU->PeekPhysical(A) << (i * 8);
  }

  A++;
 }

 PCE_InDebug--;

 return(ret);
}

void PCEDBG_IRQ(int level)
{
 if(level == 0) // NMI
 {

 }
 else if(level == 1)
  HuCPU->IRQBegin(HuC6280::IQIRQ1);
 else if(level == 2)
  HuCPU->IRQBegin(HuC6280::IQIRQ2);
 else if(level == 3)
  HuCPU->IRQBegin(HuC6280::IQTIMER);
}

class DisPCE : public Dis6280
{
	public:
	DisPCE(void)
	{

	}

	uint8 GetX(void)
	{
	 return(HuCPU->GetRegister(HuC6280::GSREG_X));
	}

	uint8 GetY(void)
	{
	 return(HuCPU->GetRegister(HuC6280::GSREG_Y));
	}

	uint8 Read(uint16 A)
	{
	 uint8 ret;

	 PCE_InDebug++;

	 ret = HuCPU->PeekLogical(A);

	 PCE_InDebug--;

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

static void TestRWBP(void) NO_INLINE;
static void TestRWBP(void)
{
 ShadowCPU->LoadShadow(*HuCPU);

 vce->ResetSimulateVDC();

 ShadowCPU->Run(TRUE);

 //printf("%d, %02x\n",ShadowCPU->IRQlow);
 //assert(!ShadowCPU->IRQlow);
}

static void CPUHandler(uint32 PC)
{
 PCE_InDebug++;

 FoundBPoint = TestPCBP(PC) | TestOpBP(HuCPU->PeekLogical(PC));

 if(NeedExecSimu)
  TestRWBP();

 if(FoundBPoint)
  BPCallB(PC);

 if(CPUCB)
  CPUCB(PC);

 if(PC == 0xe060 && PCE_LoggingOn && PCE_IsCD)
 {
  uint16 sjis_glyph;

  sjis_glyph = HuCPU->PeekLogical(0x20F8) | (HuCPU->PeekLogical(0x20F9) << 8);
  PCEDBG_DoLog("BIOS", "Call EX_GETFNT from $%04X, ax=0x%04x = %s", LastPC, sjis_glyph, PCEDBG_ShiftJIS_to_UTF8(sjis_glyph));
 }
 LastPC = PC;
 PCE_InDebug--;
 assert(!PCE_InDebug);
}

static DECLFR(ReadHandler)
{
 std::vector<PCE_BPOINT>::iterator bpit;

 if((A & 0x1FFFFF) >= (0xFF * 8192) && (A & 0x1FFFFF) <= (0xFF * 8192 + 0x3FF))
 {
  VDC_SimulateResult result;

  int which_vdc = vce->SimulateReadVDC(A & 0x80000003, &result);

  if(result.ReadCount)
   PCEDBG_CheckBP(BPOINT_AUX_READ, (which_vdc << 16) | result.ReadStart, result.ReadCount);

  if(result.WriteCount)
   PCEDBG_CheckBP(BPOINT_AUX_WRITE, (which_vdc << 16) | result.WriteStart, result.WriteCount);
 }

 for(bpit = BreakPointsRead.begin(); bpit != BreakPointsRead.end(); bpit++)
 {
  unsigned int testA = bpit->logical ? ShadowCPU->GetLastLogicalReadAddr() : A;

  if(testA >= bpit->A[0] && testA <= bpit->A[1])
  {
   FoundBPoint = 1;
   break;
  }
 }

 return(HuCPU->PeekPhysical(A));
}

static DECLFW(WriteHandler)
{
 std::vector<PCE_BPOINT>::iterator bpit;

 if((A & 0x1FFFFF) >= (0xFF * 8192) && (A & 0x1FFFFF) <= (0xFF * 8192 + 0x3FF))
 {
  VDC_SimulateResult result;

  int which_vdc = vce->SimulateWriteVDC(A & 0x80000003, V, &result);

  if(result.ReadCount)
   PCEDBG_CheckBP(BPOINT_AUX_READ, (which_vdc << 16) | result.ReadStart, result.ReadCount);

  if(result.WriteCount)
   PCEDBG_CheckBP(BPOINT_AUX_WRITE, (which_vdc << 16) | result.WriteStart, result.WriteCount);
 }


 for(bpit = BreakPointsWrite.begin(); bpit != BreakPointsWrite.end(); bpit++)
 {
  unsigned int testA;

  if(!bpit->logical)
   testA = A;
  else
  {
   if(A & 0x80000000) continue;		// Ignore ST0/ST1/ST2 writes, which always use hardcoded physical addresses.

   testA = ShadowCPU->GetLastLogicalWriteAddr();
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
 bool BPointsUsed;

 NeedExecSimu = BreakPointsRead.size() || BreakPointsWrite.size() || BreakPointsAux0Read.size() || BreakPointsAux0Write.size();

 BPointsUsed = BreakPointsPCUsed || BreakPointsOpUsed || BreakPointsRead.size() || BreakPointsWrite.size() || 
		BreakPointsAux0Read.size() || BreakPointsAux0Write.size();

 if(BPointsUsed || CPUCB || PCE_LoggingOn)
  HuCPU->SetCPUHook(CPUHandler, AddBranchTrace);
 else
  HuCPU->SetCPUHook(NULL, NULL);
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
 {
  for(unsigned int i = A1; i <= A2; i++)
  {
   if((unsigned int)i < 65536)
   {
    BreakPointsPCUsed = true;
    BreakPointsPC[i >> 3] |= 1 << (i & 0x7);
   }
  }
 }
 else if(type == BPOINT_AUX_READ)
  BreakPointsAux0Read.push_back(tmp);
 else if(type == BPOINT_AUX_WRITE)
  BreakPointsAux0Write.push_back(tmp);
 else if(type == BPOINT_OP)
 {
  for(unsigned int i = A1; i <= A2; i++)
  {
   if((unsigned int)i < 256)
   {
    BreakPointsOpUsed = true;
    BreakPointsOp[i] = 1;
   }
  }   
 }

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
 {
  memset(BreakPointsPC, 0, sizeof(BreakPointsPC));
  BreakPointsPCUsed = false;
 }
 else if(type == BPOINT_AUX_READ)
  BreakPointsAux0Read.clear();
 else if(type == BPOINT_AUX_WRITE)
  BreakPointsAux0Write.clear();
 else if(type == BPOINT_OP)
 {
  memset(BreakPointsOp, 0, sizeof(BreakPointsOp));
  BreakPointsOpUsed = false;
 }

 RedoDH();
}

void PCEDBG_SetCPUCallback(void (*callb)(uint32 PC))
{
 CPUCB = callb;
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

extern uint64 PCE_TimestampBase;
static uint32 GetRegister_HuC6280(const unsigned int id, char *special, const uint32 special_len)
{
 if(id == HuC6280::GSREG_STAMP)
 {
  return(PCE_TimestampBase + HuCPU->GetRegister(id, special, special_len));
 }
 return(HuCPU->GetRegister(id, special, special_len));
}

static void SetRegister_HuC6280(const unsigned int id, uint32 value)
{
 HuCPU->SetRegister(id, value);
}

static uint32 GetRegister_PSG(const unsigned int id, char *special, const uint32 special_len)
{
 return(psg->GetRegister(id, special, special_len));
}

static void SetRegister_PSG(const unsigned int id, uint32 value)
{
 psg->SetRegister(id, value);
}

static uint32 GetRegister_VDC(const unsigned int id, char *special, const uint32 special_len)
{
 if(id & 0x8000)
  return(vce->GetRegister(id &~ 0x8000, special, special_len));

 return(vce->GetRegisterVDC(0, id, special, special_len));
}

static void SetRegister_VDC(const unsigned int id, uint32 value)
{
 if(id & 0x8000)
 {
  vce->SetRegister(id &~ 0x8000, value);
  return;
 }

 vce->SetRegisterVDC(0, id, value);
}

static uint32 GetRegister_SGXVDC(const unsigned int id, char *special, const uint32 special_len)
{
 if(id & 0x8000)
  return(vce->GetRegister(id &~ 0x8000, special, special_len));

 return(vce->GetRegisterVDC(1, id, special, special_len));
}

static void SetRegister_SGXVDC(const unsigned int id, uint32 value)
{
 if(id & 0x8000)
 {
  vce->SetRegister(id &~ 0x8000, value);
  return;
 }

 vce->SetRegisterVDC(1, id, value);
}

static uint32 GetRegister_CD(const unsigned int id, char *special, const uint32 special_len)
{
 return(PCECD_GetRegister(id, special, special_len));
}

static void SetRegister_CD(const unsigned int id, uint32 value)
{
 PCECD_SetRegister(id, value);
}


static RegType Regs_HuC6280[] =
{
        { HuC6280::GSREG_PC, "PC", "Program Counter", 2 },
        { HuC6280::GSREG_A, "A", "Accumulator", 1 },
        { HuC6280::GSREG_X, "X", "X Index Register", 1 },
        { HuC6280::GSREG_Y, "Y", "Y Index Register", 1 },
        { HuC6280::GSREG_SP, "SP", "Stack Pointer", 1 },
        { HuC6280::GSREG_P, "P", "Status", 1 },
        { HuC6280::GSREG_MPR0, "MPR0", "MPR0", 1},
        { HuC6280::GSREG_MPR1, "MPR1", "MPR1", 1},
        { HuC6280::GSREG_MPR2, "MPR2", "MPR2", 1},
        { HuC6280::GSREG_MPR3, "MPR3", "MPR3", 1},
        { HuC6280::GSREG_MPR4, "MPR4", "MPR4", 1},
        { HuC6280::GSREG_MPR5, "MPR5", "MPR5", 1},
        { HuC6280::GSREG_MPR6, "MPR6", "MPR6", 1},
        { HuC6280::GSREG_MPR7, "MPR7", "MPR7", 1},
        { HuC6280::GSREG_SPD, "SPD", "CPU Speed", 1 },
        { HuC6280::GSREG_IRQM, "IRQM", "IRQ Mask", 1},
        { HuC6280::GSREG_TIMS, "TIMS", "Timer Status", 1},
        { HuC6280::GSREG_TIMV, "TIMV", "Timer Value", 1},
        { HuC6280::GSREG_TIML, "TIML", "Timer Load", 1},
        { HuC6280::GSREG_TIMD, "TIMD", "Timer Div Counter", 2},
	{ 0, "------", "", 0xFFFF },   \
	{ HuC6280::GSREG_STAMP, "TS", "Timestamp", 4 },
        { 0, "", "", 0 },
};

static RegGroupType RegsGroup_HuC6280 =
{
	"HuC6280",
        Regs_HuC6280,
        GetRegister_HuC6280,
        SetRegister_HuC6280
};


#define CHPDMOO(n)      \
 { 0, "------", "", 0xFFFF },	\
 { PSG_GSREG_CH0_FREQ | (n << 8), "CH"#n"Freq", "PSG Ch"#n" Frequency(Period)", 2 },   \
 { PSG_GSREG_CH0_CTRL | (n << 8), "CH"#n"Ctrl", "PSG Ch"#n" Control", 1 },     \
 { PSG_GSREG_CH0_BALANCE | (n << 8), "CH"#n"Balance", "PSG Ch"#n" Balance", 1 },  \
 { PSG_GSREG_CH0_WINDEX | (n << 8), "CH"#n"WIndex", "PSG Ch"#n" Waveform Index", 1},     \
 { PSG_GSREG_CH0_SCACHE | (n << 8), "CH"#n"SCache", "PSG Ch"#n" Sample Cache", 1 }

static RegType Regs_PSG[] =
{
 { PSG_GSREG_SELECT, "Select", "PSG Channel Select", 1 },
 { PSG_GSREG_GBALANCE, "GBalance", "PSG Global Balance", 1 },
 { PSG_GSREG_LFOFREQ, "LFOFreq", "PSG LFO Freq", 1 },
 { PSG_GSREG_LFOCTRL, "LFOCtrl", "PSG LFO Control", 1 },

 CHPDMOO(0),
 CHPDMOO(1),
 CHPDMOO(2),
 CHPDMOO(3),
 CHPDMOO(4),
 { PSG_GSREG_CH4_NCTRL, "CH4NCtrl", "PSG Ch4 Noise Control", 1 },
 { PSG_GSREG_CH4_LFSR, "CH4LFSR", "PSG Ch4 Noise LFSR", 2 },
 CHPDMOO(5),
 { PSG_GSREG_CH5_NCTRL, "CH5NCtrl", "PSG Ch5 Noise Control", 1 },
 { PSG_GSREG_CH5_LFSR, "CH5LFSR", "PSG Ch5 Noise LFSR", 2 },

 { 0, "", "", 0 },
};

static RegGroupType RegsGroup_PSG =
{
 "PSG",
 Regs_PSG,
 GetRegister_PSG,
 SetRegister_PSG
};

static RegType Regs_VDC[] =
{
	{ VDC::GSREG_SELECT, "Select", "Register Select", 1 },
        { VDC::GSREG_STATUS, "Status", "Status", 1 },

        { VDC::GSREG_MAWR, "MAWR", "Memory Write Address", 2 },
        { VDC::GSREG_MARR, "MARR", "Memory Read Address", 2 },
        { VDC::GSREG_CR, "CR", "Control", 2 },
        { VDC::GSREG_RCR, "RCR", "Raster Counter", 2 },
        { VDC::GSREG_BXR, "BXR", "X Scroll", 2 },
        { VDC::GSREG_BYR, "BYR", "Y Scroll", 2 },
        { VDC::GSREG_MWR, "MWR", "Memory Width", 2 },

        { VDC::GSREG_HSR, "HSR", "HSR", 2 },
        { VDC::GSREG_HDR, "HDR", "HDR", 2 },
        { VDC::GSREG_VSR, "VSR", "VSR", 2 },
        { VDC::GSREG_VDR, "VDR", "VDR", 2 },

        { VDC::GSREG_VCR, "VCR", "VCR", 2 },
        { VDC::GSREG_DCR, "DCR", "DMA Control", 2 },
        { VDC::GSREG_SOUR, "SOUR", "VRAM DMA Source Address", 2 },
        { VDC::GSREG_DESR, "DESR", "VRAM DMA Dest Address", 2 },
        { VDC::GSREG_LENR, "LENR", "VRAM DMA Length", 2 },
        { VDC::GSREG_DVSSR, "DVSSR", "DVSSR Update Address", 2 },
	{ 0, "------", "", 0xFFFF },

	{ 0x8000 | VCE::GSREG_CR, "VCECR", "VCE Control Register", 1 },
	{ 0x8000 | VCE::GSREG_CTA, "VCECTA", "VCE Color/Palette Table Address", 2 },
	{ 0x8000 | VCE::GSREG_SCANLINE, "Line", "Current Scanline", 2 },

        { 0, "", "", 0 },
};

static RegGroupType RegsGroup_VDC =
{
	"VDC",
        Regs_VDC,
        GetRegister_VDC,
        SetRegister_VDC
};


static RegType Regs_SGXVDC[] =
{
        { VDC::GSREG_SELECT, "Select", "Register Select, VDC-B", 1 },
	{ VDC::GSREG_STATUS, "Status", "Status, VDC-B", 1 },

        { VDC::GSREG_MAWR, "MAWR-B", "Memory Write Address, VDC-B", 2 },
        { VDC::GSREG_MARR, "MARR-B", "Memory Read Address, VDC-B", 2 },
        { VDC::GSREG_CR, "CR-B", "Control, VDC-B", 2 },
        { VDC::GSREG_RCR, "RCR-B", "Raster Counter, VDC-B", 2 },
        { VDC::GSREG_BXR, "BXR-B", "X Scroll, VDC-B", 2 },
        { VDC::GSREG_BYR, "BYR-B", "Y Scroll, VDC-B", 2 },
        { VDC::GSREG_MWR, "MWR-B", "Memory Width, VDC-B", 2 },

        { VDC::GSREG_HSR, "HSR-B", "HSR, VDC-B", 2 },
        { VDC::GSREG_HDR, "HDR-B", "HDR, VDC-B", 2 },
        { VDC::GSREG_VSR, "VSR-B", "VSR, VDC-B", 2 },
        { VDC::GSREG_VDR, "VDR-B", "VDR, VDC-B", 2 },

        { VDC::GSREG_VCR, "VCR-B", "VCR, VDC-B", 2 },
        { VDC::GSREG_DCR, "DCR-B", "DMA Control, VDC-B", 2 },
        { VDC::GSREG_SOUR, "SOUR-B", "VRAM DMA Source Address, VDC-B", 2 },
        { VDC::GSREG_DESR, "DESR-B", "VRAM DMA Dest Address, VDC-B", 2 },
        { VDC::GSREG_LENR, "LENR-B", "VRAM DMA Length, VDC-B", 2 },
        { VDC::GSREG_DVSSR, "DVSSR-B", "DVSSR Update Address, VDC-B", 2 },
        { 0, "------", "", 0xFFFF },

	{ 0x8000 | VCE::GSREG_ST_MODE, "STMode", "ST(ST0/ST1/ST2) Mode/Target", 1 },
        { 0x8000 | VCE::GSREG_PRIORITY_0, "PRIO0", "VPC Priority Register 0", 1 },
	{ 0x8000 | VCE::GSREG_PRIORITY_1, "PRIO1", "VPC Priority Register 1", 1 },
	{ 0x8000 | VCE::GSREG_WINDOW_WIDTH_0, "WIND0", "VPC Window Width Register 0", 2 },
	{ 0x8000 | VCE::GSREG_WINDOW_WIDTH_1, "WIND1", "VPC Window Width Register 1", 2 },

        { 0, "", "", 0 },
};

static RegGroupType RegsGroup_SGXVDC =
{
	"VDC-B",
        Regs_SGXVDC,
        GetRegister_SGXVDC,
        SetRegister_SGXVDC
};

static RegType Regs_CD[] =
{
 { CD_GSREG_BSY, "BSY", "SCSI BSY", 1 },
 { CD_GSREG_REQ, "REQ", "SCSI REQ", 1 },
 { CD_GSREG_MSG, "MSG", "SCSI MSG", 1 },
 { CD_GSREG_IO, "IO", "SCSI IO", 1 },
 { CD_GSREG_CD, "CD", "SCSI CD", 1 },
 { CD_GSREG_SEL, "SEL", "SCSI SEL", 1 },

 { 0, "---ADPCM:---", "", 0xFFFF },
 { CD_GSREG_ADPCM_CONTROL, "Control", "ADPCM Control", 1 },
 { CD_GSREG_ADPCM_FREQ, "Freq", "ADPCM Frequency", 1 },
 { CD_GSREG_ADPCM_CUR, "CUR", "ADPCM Current 12-bit Value", 2 },
 { CD_GSREG_ADPCM_WRADDR, "WrAddr", "ADPCM Write Address", 2 },
 { CD_GSREG_ADPCM_RDADDR, "RdAddr", "ADPCM Read Address", 2 },
 { CD_GSREG_ADPCM_LENGTH, "Length", "ADPCM Length", 2 },
 { CD_GSREG_ADPCM_PLAYNIBBLE, "PlNibble", "ADPCM Play Nibble Select", 1 },

 { CD_GSREG_ADPCM_PLAYING, "Playing", "ADPCM Playing Flag", 1 },
 { CD_GSREG_ADPCM_HALFREACHED, "Half", "ADPCM Half-point Reached Flag", 1 },
 { CD_GSREG_ADPCM_ENDREACHED, "End", "ADPCM End Reached Flag", 1 } ,
 { 0, "", "", 0 },
};

static RegGroupType RegsGroup_CD =
{
 "CD",
 Regs_CD,
 GetRegister_CD,
 SetRegister_CD
};

static void Do16BitGet(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 int wc = 0;

 if(!strcmp(name, "vram0"))
  wc = 0;
 else if(!strcmp(name, "vram1"))
  wc = 1;
 else if(!strcmp(name, "sat0"))
  wc = 2;
 else if(!strcmp(name, "sat1"))
  wc = 3;
 else if(!strcmp(name, "pram"))
  wc = 4;

 while(Length)
 {
  uint16 data;

  if(wc == 4)
   data = vce->PeekPRAM((Address >> 1) & 0x1FF);
  else if(wc & 2) 
   data = vce->PeekVDCSAT(wc & 1, (Address >> 1) & 0xFF);
  else
   data = vce->PeekVDCVRAM(wc & 1, (Address >> 1) & 0x7FFF);

  if((Address & 1) || Length == 1)
  {
   *Buffer = data >> ((Address & 1) << 3);
   Buffer++;
   Address++;
   Length--;
  }
  else
  {
   Buffer[0] = data & 0xFF;
   Buffer[1] = data >> 8;

   Buffer += 2;
   Address += 2;
   Length -= 2;
  }
 }
}

static void Do16BitPut(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 int wc = 0;

 if(!strcmp(name, "vram0"))
  wc = 0;
 else if(!strcmp(name, "vram1"))
  wc = 1;
 else if(!strcmp(name, "sat0"))
  wc = 2;
 else if(!strcmp(name, "sat1"))
  wc = 3;
 else if(!strcmp(name, "pram"))
  wc = 4;

 while(Length)
 {
  uint16 data;
  int inc_amount;

  if((Address & 1) || Length == 1)
  {
   if(wc == 4)
    data = vce->PeekPRAM((Address >> 1) & 0x1FF);
   else if(wc & 2)
    data = vce->PeekVDCSAT(wc & 1, (Address >> 1) & 0xFF);
   else
    data = vce->PeekVDCVRAM(wc & 1, (Address >> 1) & 0x7FFF);

   data &= ~(0xFF << ((Address & 1) << 3));
   data |= *Buffer << ((Address & 1) << 3);

   inc_amount = 1;
  }
  else
  {
   data = Buffer[0] | (Buffer[1] << 8);
   inc_amount = 2;
  }

  if(wc == 4)
   vce->PokePRAM((Address >> 1) & 0x1FF, data);
  else if(wc & 2)
   vce->PokeVDCSAT(wc & 1, (Address >> 1) & 0xFF, data);
  else
   vce->PokeVDCVRAM(wc & 1, (Address >> 1) & 0x7FFF, data);

  Buffer += inc_amount;
  Address += inc_amount;
  Length -= inc_amount;
 }
}


static void GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 PCE_InDebug++;

 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFF;

   *Buffer = HuCPU->PeekLogical(Address);

   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "physical"))
 {
  while(Length--)
  {
   Address &= 0x1FFFFF;

   *Buffer = HuCPU->PeekPhysical(Address);

   Address++;
   Buffer++;
  }

 }
 else if(!strcmp(name, "ram"))
 {
  while(Length--)
  {
   Address &= (IsSGX ? 32768 : 8192) - 1;
   // FIXME
   //*Buffer = BaseRAM[Address];
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "adpcm"))
  ADPCM_PeekRAM(Address, Length, Buffer);
 else if(!strcmp(name, "acram"))
  arcade_card->PeekRAM(Address, Length, Buffer);
 else if(!strncmp(name, "psgram", 6))
  psg->PeekWave(name[6] - '0', Address, Length, Buffer);

 PCE_InDebug--;
}

static void PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 PCE_InDebug++;

 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFF;

   HuCPU->PokeLogical(Address, *Buffer, hl);

   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "physical"))
 {
  while(Length--)
  {
   Address &= 0x1FFFFF;

   HuCPU->PokePhysical(Address, *Buffer, hl);

   Address++;
   Buffer++;
  }

 }
 else if(!strcmp(name, "ram"))
 {
  while(Length--)
  {
   Address &= (IsSGX ? 32768 : 8192) - 1;
   // FIXME
   //BaseRAM[Address] = *Buffer;
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "adpcm"))
  ADPCM_PokeRAM(Address, Length, Buffer);
 else if(!strcmp(name, "acram"))
  arcade_card->PokeRAM(Address, Length, Buffer);
 else if(!strncmp(name, "psgram", 6))
  psg->PokeWave(name[6] - '0', Address, Length, Buffer);

 PCE_InDebug--;
}


static void SetGraphicsDecode(MDFN_Surface *surface, int line, int which, int xscroll, int yscroll, int pbn)
{
 vce->SetGraphicsDecode(surface, line, which, xscroll, yscroll, pbn);
}

DebuggerInfoStruct PCEDBGInfo =
{
 "shift_jis",
 7,
 1,             // Instruction alignment(bytes)
 16,
 21,
 0x2000,
 0x2000, // ZP

 PCEDBG_MemPeek,
 PCEDBG_Disassemble,
 NULL,
 PCEDBG_IRQ,
 NULL, //NESDBG_GetVector,
 PCEDBG_FlushBreakPoints,
 PCEDBG_AddBreakPoint,
 PCEDBG_SetCPUCallback,
 PCEDBG_SetBPCallback,
 PCEDBG_GetBranchTrace,
 SetGraphicsDecode,
 PCEDBG_SetLogFunc,
};

bool PCEDBG_Init(bool sgx, PCE_PSG *new_psg)
{
 BreakPointsPCUsed = false;
 BreakPointsOpUsed = false;

 memset(BreakPointsOp, 0, sizeof(BreakPointsOp));
 memset(BreakPointsPC, 0, sizeof(BreakPointsPC));

 ShadowCPU = new HuC6280();

 for(int x = 0; x < 0x100; x++)
 {
  ShadowCPU->SetFastRead(x, NULL);

  ShadowCPU->SetReadHandler(x, ReadHandler);
  ShadowCPU->SetWriteHandler(x, WriteHandler);
 }

 ShadowCPU->Power();

 psg = new_psg;

 IsSGX = sgx;

 MDFNDBG_AddRegGroup(&RegsGroup_HuC6280);
 MDFNDBG_AddRegGroup(&RegsGroup_VDC);

 if(IsSGX)
  MDFNDBG_AddRegGroup(&RegsGroup_SGXVDC);

 ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "cpu", "CPU Logical", 16);
 ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "physical", "CPU Physical", 21);
 ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "ram", "RAM", IsSGX ? 15 : 13);

 ASpace_Add(Do16BitGet, Do16BitPut, "pram", "VCE Palette RAM", 10);

 if(IsSGX)
 {
  ASpace_Add(Do16BitGet, Do16BitPut, "vram0", "VDC-A VRAM", 15 + 1);
  ASpace_Add(Do16BitGet, Do16BitPut, "sat0", "VDC-A SAT", 8 + 1);
  ASpace_Add(Do16BitGet, Do16BitPut, "vram1", "VDC-B VRAM", 15 + 1);
  ASpace_Add(Do16BitGet, Do16BitPut, "sat1", "VDC-B SAT", 8 + 1);
 }
 else
 {
  ASpace_Add(Do16BitGet, Do16BitPut, "vram0", "VDC VRAM", 15 + 1);
  ASpace_Add(Do16BitGet, Do16BitPut, "sat0", "VDC SAT", 8 + 1);
 }

 if(PCE_IsCD)
 {
  ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "adpcm", "ADPCM RAM", 16);
  MDFNDBG_AddRegGroup(&RegsGroup_CD);
 }

 if(arcade_card)
 {
  ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "acram", "Arcade Card RAM", 21);
 }

 for(int x = 0; x < 6; x++)
 {
     AddressSpaceType newt;
     char tmpname[128], tmpinfo[128];

     trio_snprintf(tmpname, 128, "psgram%d", x);
     trio_snprintf(tmpinfo, 128, "PSG Ch %d RAM", x);

     memset(&newt, 0, sizeof(AddressSpaceType));

     newt.GetAddressSpaceBytes = GetAddressSpaceBytes;
     newt.PutAddressSpaceBytes = PutAddressSpaceBytes;

     newt.name = strdup(tmpname);
     newt.long_name = strdup(tmpinfo);
     newt.TotalBits = 5;
     newt.NP2Size = 0;
     newt.IsSegmented = FALSE;
     newt.SegmentBits = 0;
     newt.OffsetBits = 0;
     newt.BitsOverlapped = 0;

     newt.IsWave = TRUE;
     newt.WaveFormat = ASPACE_WFMT_UNSIGNED;
     newt.WaveBits = 5;
     ASpace_Add(newt); //PSG_GetAddressSpaceBytes, PSG_PutAddressSpaceBytes, tmpname, tmpinfo, 5);
 }
 MDFNDBG_AddRegGroup(&RegsGroup_PSG);

 return(TRUE);
}


};
