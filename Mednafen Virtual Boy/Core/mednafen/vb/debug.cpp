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

#include "vb.h"
#include <mednafen/hw_cpu/v810/v810_cpuD.h>
#include <string.h>
#include <trio/trio.h>
#include <stdarg.h>
#include <iconv.h>

#include "debug.h"
#include "timer.h"
//#include "input.h"
#include "vip.h"
#include "vsu.h"
#include "timer.h"

namespace MDFN_IEN_VB
{

extern V810 *VB_V810;
extern VSU *VB_VSU;

static void (*CPUHook)(uint32);
static void (*BPCallB)(uint32 PC) = NULL;
static void (*LogFunc)(const char *, const char *);
bool VB_LoggingOn = FALSE;

typedef struct __VB_BPOINT {
        uint32 A[2];
        int type;
        bool logical;
} VB_BPOINT;

static std::vector<VB_BPOINT> BreakPointsPC, BreakPointsRead, BreakPointsWrite;
static bool FoundBPoint = 0;


#if 0
static int BTIndex = 0;
static uint32 BTEntries[16];

static void AddBranchTrace(uint32 old_PC, uint32 new_PC, uint32 special)
{
 if(BTEntries[(BTIndex - 1) & 0xF] == new_PC) return;

 BTEntries[BTIndex] = new_PC;
 BTIndex = (BTIndex + 1) & 0xF;
}

std::vector<std::string> VBDBG_GetBranchTrace(void)
{
 std::vector<std::string> ret;

 for(int x = 0; x < 16; x++)
 {
  char *tmps = trio_aprintf("%08X", BTEntries[(x + BTIndex) & 0xF]);
  ret.push_back(std::string(tmps));
  free(tmps);
 }
 return(ret);
}
#endif

struct BTEntry
{
 uint32 from;
 uint32 to;
 uint32 branch_count;
 uint32 ecode;
};

#define NUMBT 24
static BTEntry BTEntries[NUMBT];
static int BTIndex = 0;

static void AddBranchTrace(uint32 from, uint32 to, uint32 ecode)
{
 BTEntry *prevbt = &BTEntries[(BTIndex + NUMBT - 1) % NUMBT];

 //if(BTEntries[(BTIndex - 1) & 0xF] == PC) return;

 if(prevbt->from == from && prevbt->to == to && prevbt->ecode == ecode && prevbt->branch_count < 0xFFFFFFFF)
  prevbt->branch_count++;
 else
 {
  BTEntries[BTIndex].from = from;
  BTEntries[BTIndex].to = to;
  BTEntries[BTIndex].ecode = ecode;
  BTEntries[BTIndex].branch_count = 1;

  BTIndex = (BTIndex + 1) % NUMBT;
 }
}


std::vector<BranchTraceResult> VBDBG_GetBranchTrace(void)
{
 BranchTraceResult tmp;
 std::vector<BranchTraceResult> ret;

 for(int x = 0; x < NUMBT; x++)
 {
  const BTEntry *bt = &BTEntries[(x + BTIndex) % NUMBT];

  tmp.count = bt->branch_count;
  trio_snprintf(tmp.from, sizeof(tmp.from), "%08x", bt->from);
  trio_snprintf(tmp.to, sizeof(tmp.to), "%08x", bt->to);

  tmp.code[0] = 0;

  switch(bt->ecode)
  {
   case 0: break;
   default: trio_snprintf(tmp.code, sizeof(tmp.code), "e");
            break;

   case 0xFFF0: // Reset
        trio_snprintf(tmp.code, sizeof(tmp.code), "R");
        break;

   case 0xFFD0: // NMI
        trio_snprintf(tmp.code, sizeof(tmp.code), "NMI");
        break;

   case 0xFFC0: // Address trap
        trio_snprintf(tmp.code, sizeof(tmp.code), "ADTR");
        break;

   case 0xFFA0 ... 0xFFBF:      // TRAP
        trio_snprintf(tmp.code, sizeof(tmp.code), "TRAP");
        break;

   case 0xFF90: // Illegal/invalid instruction code
        trio_snprintf(tmp.code, sizeof(tmp.code), "ILL");
        break;

   case 0xFF80: // Zero division
        trio_snprintf(tmp.code, sizeof(tmp.code), "ZD");
        break;

   case 0xFF70:
        trio_snprintf(tmp.code, sizeof(tmp.code), "FIV");       // FIV
        break;

   case 0xFF68:
        trio_snprintf(tmp.code, sizeof(tmp.code), "FZD");       // FZD
        break;

   case 0xFF64:
        trio_snprintf(tmp.code, sizeof(tmp.code), "FOV");       // FOV
        break;

   case 0xFF62:
        trio_snprintf(tmp.code, sizeof(tmp.code), "FUD");       // FUD
        break;

   case 0xFF61:
        trio_snprintf(tmp.code, sizeof(tmp.code), "FPR");       // FPR
        break;

   case 0xFF60:
        trio_snprintf(tmp.code, sizeof(tmp.code), "FRO");       // FRO
        break;

   case 0xFE00 ... 0xFEFF:
        trio_snprintf(tmp.code, sizeof(tmp.code), "INT%d", (bt->ecode >> 4) & 0xF);
        break;
  }

  ret.push_back(tmp);
 }
 return(ret);
}


void VBDBG_CheckBP(int type, uint32 address, unsigned int len)
{
 std::vector<VB_BPOINT>::iterator bpit, bpit_end;

 if(type == BPOINT_READ || type == BPOINT_IO_READ)
 {
  bpit = BreakPointsRead.begin();
  bpit_end = BreakPointsRead.end();
 }
 else if(type == BPOINT_WRITE || type == BPOINT_IO_WRITE)
 {
  bpit = BreakPointsWrite.begin();
  bpit_end = BreakPointsWrite.end();
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

static uint16 MDFN_FASTCALL MemPeek8(v810_timestamp_t timestamp, uint32 A)
{
 uint8 ret;

 // TODO: VB_InDebugPeek(implement elsewhere)
 VB_InDebugPeek++;
 ret = MemRead8(timestamp, A);
 VB_InDebugPeek--;

 return(ret);
}

static uint16 MDFN_FASTCALL MemPeek16(v810_timestamp_t timestamp, uint32 A)
{
 uint16 ret;

 // TODO: VB_InDebugPeek(implement elsewhere)
 VB_InDebugPeek++;
 ret = MemRead16(timestamp, A);
 VB_InDebugPeek--;

 return(ret);
}

static void CPUHandler(uint32 PC)
{
 std::vector<VB_BPOINT>::iterator bpit;

 // FIXME/TODO: Call ForceEventUpdates() somewhere

 for(bpit = BreakPointsPC.begin(); bpit != BreakPointsPC.end(); bpit++)
 {
  if(PC >= bpit->A[0] && PC <= bpit->A[1])
  {
   FoundBPoint = TRUE;
   break;
  }
 }
 VB_V810->CheckBreakpoints(VBDBG_CheckBP, MemPeek16, NULL);

 if(FoundBPoint)
 {
  BPCallB(PC);
  FoundBPoint = 0;
 }

 if(CPUHook)
  CPUHook(PC);
}

static void RedoCPUHook(void)
{
 bool HappyTest;

 HappyTest = VB_LoggingOn || BreakPointsPC.size() || BreakPointsRead.size() || BreakPointsWrite.size();

 void (*cpuh)(uint32);

 cpuh = HappyTest ? CPUHandler : CPUHook;

 VB_V810->SetCPUHook(cpuh, cpuh ? AddBranchTrace : NULL);
}

void VBDBG_FlushBreakPoints(int type)
{
 std::vector<VB_BPOINT>::iterator bpit;

 if(type == BPOINT_READ)
  BreakPointsRead.clear();
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.clear();
 else if(type == BPOINT_PC)
  BreakPointsPC.clear();

 RedoCPUHook();
}

void VBDBG_AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical)
{
 VB_BPOINT tmp;

 tmp.A[0] = A1;
 tmp.A[1] = A2;
 tmp.type = type;

 if(type == BPOINT_READ)
  BreakPointsRead.push_back(tmp);
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.push_back(tmp);
 else if(type == BPOINT_PC)
  BreakPointsPC.push_back(tmp);

 RedoCPUHook();
}

static uint16 dis_readhw(uint32 A)
{
 int32 timestamp = 0;
 return(MemPeek16(timestamp, A));
}

void VBDBG_Disassemble(uint32 &a, uint32 SpecialA, char *TextBuf)
{
 return(v810_dis(a, 1, TextBuf, dis_readhw));
}

uint32 VBDBG_MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical)
{
 uint32 ret = 0;
 int32 ws = 0;

 for(unsigned int i = 0; i < bsize; i++)
 {
  A &= 0xFFFFFFFF;
  //ret |= mem_peekbyte(A, ws) << (i * 8);
  ret |= MemRead8(ws, A) << (i * 8);
  A++;
 }

 return(ret);
}

static void GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   *Buffer = MemPeek8(0, Address);

   Address++;
   Buffer++;
  }
 }
 else if(!strncmp(name, "vsuwd", 5))
 {
  const unsigned int which = name[5] - '0';

  while(Length--)
  {
   *Buffer = VB_VSU->PeekWave(which, Address);

   Address++;
   Buffer++;
  }
 }
}

static void PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer)
{
 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   int32 dummy_ts = 0;

   MemWrite8(dummy_ts, Address, *Buffer);

   Address++;
   Buffer++;
  }
 }
 else if(!strncmp(name, "vsuwd", 5))
 {
  const unsigned int which = name[5] - '0';

  while(Length--)
  {
   VB_VSU->PokeWave(which, Address, *Buffer);

   Address++;
   Buffer++;
  }
 }
}

uint32 VBDBG_GetRegister(const std::string &name, std::string *special)
{
 if(name == "PC")
 {
  return(VB_V810->GetPC());
 }
 const char *thestring = name.c_str();

 if(!strncmp(thestring, "PR", 2))
 {
  return(VB_V810->GetPR(atoi(thestring + 2)));
 }
 else if(!strcmp(thestring, "HSP"))
  return(VB_V810->GetPR(2));
 else if(!strcmp(thestring, "SP"))
  return(VB_V810->GetPR(3));
 else if(!strcmp(thestring, "GP"))
  return(VB_V810->GetPR(4));
 else if(!strcmp(thestring, "TP"))
  return(VB_V810->GetPR(5));
 else if(!strcmp(thestring, "LP"))
  return(VB_V810->GetPR(31));
 else if(!strcmp(thestring, "TStamp"))
  return(VB_V810->v810_timestamp);
 else if(!strncmp(thestring, "SR", 2))
 {
  int which_one = atoi(thestring + 2);
  uint32 val =  VB_V810->GetSR(which_one);

  if(special && which_one == PSW)
  {
   char buf[256];
   snprintf(buf, 256, "Z: %d, S: %d, OV: %d, CY: %d, ID: %d, AE: %d, EP: %d, NP: %d, IA: %2d",
	(int)(bool)(val & PSW_Z), (int)(bool)(val & PSW_S), (int)(bool)(val & PSW_OV), (int)(bool)(val & PSW_CY),
	(int)(bool)(val & PSW_ID), (int)(bool)(val & PSW_AE), (int)(bool)(val & PSW_EP), (int)(bool)(val & PSW_NP),
	(val & PSW_IA) >> 16);
   *special = std::string(buf);
  }
  return(val);
 }
 uint32 val = 0; 

 return(val);
}

void VBDBG_SetRegister(const std::string &name, uint32 value)
{
 if(name == "PC")
 {
  VB_V810->SetPC(value & ~1);
  return;
 }

 const char *thestring = name.c_str();

 if(!strncmp(thestring, "PR", 2))
 {
  VB_V810->SetPR(atoi(thestring + 2), value);
 }
 else if(!strcmp(thestring, "HSP"))
  VB_V810->SetPR(2, value);
 else if(!strcmp(thestring, "SP"))
  VB_V810->SetPR(3, value);
 else if(!strcmp(thestring, "GP"))
  VB_V810->SetPR(4, value);
 else if(!strcmp(thestring, "TP"))
  VB_V810->SetPR(5, value);
 else if(!strcmp(thestring, "LP"))
  VB_V810->SetPR(31, value);
 else if(!strncmp(thestring, "SR", 2))
 {
  VB_V810->SetSR(atoi(thestring + 2), value);
 }
}

void VBDBG_SetCPUCallback(void (*callb)(uint32 PC))
{
 CPUHook = callb;
 RedoCPUHook();
}

void VBDBG_SetBPCallback(void (*callb)(uint32 PC))
{
 BPCallB = callb;
}

void VBDBG_DoLog(const char *type, const char *format, ...)
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

void VBDBG_SetLogFunc(void (*func)(const char *, const char *))
{
 LogFunc = func;

 VB_LoggingOn = func ? TRUE : FALSE;

 if(VB_LoggingOn)
 {

 }
 else
 {

 }
 RedoCPUHook();
}

static RegType V810Regs[] =
{
        { 0, "PC", "Program Counter", 4 },
        { 0, "PR1", "Program Register 1", 4 },
        { 0, "HSP", "Program Register 2(Handler Stack Pointer)", 4 },
        { 0, "SP", "Program Register 3(Stack Pointer)", 4 },
        { 0, "GP", "Program Register 4(Global Pointer)", 4 },
        { 0, "TP", "Program Register 5(Text Pointer)", 4 },
        { 0, "PR6", "Program Register 6", 4 },
        { 0, "PR7", "Program Register 7", 4 },
        { 0, "PR8", "Program Register 8", 4 },
        { 0, "PR9", "Program Register 9", 4 },
        { 0, "PR10", "Program Register 10", 4 },
        { 0, "PR11", "Program Register 11", 4 },
        { 0, "PR12", "Program Register 12", 4 },
        { 0, "PR13", "Program Register 13", 4 },
        { 0, "PR14", "Program Register 14", 4 },
        { 0, "PR15", "Program Register 15", 4 },
        { 0, "PR16", "Program Register 16", 4 },
        { 0, "PR17", "Program Register 17", 4 },
        { 0, "PR18", "Program Register 18", 4 },
        { 0, "PR19", "Program Register 19", 4 },
        { 0, "PR20", "Program Register 20", 4 },
        { 0, "PR21", "Program Register 21", 4 },
        { 0, "PR22", "Program Register 22", 4 },
        { 0, "PR23", "Program Register 23", 4 },
        { 0, "PR24", "Program Register 24", 4 },
        { 0, "PR25", "Program Register 25", 4 },
        { 0, "PR26", "Program Register 26(String Dest Bit Offset)", 4 },
        { 0, "PR27", "Program Register 27(String Source Bit Offset)", 4 },
        { 0, "PR28", "Program Register 28(String Length)", 4 },
        { 0, "PR29", "Program Register 29(String Dest)", 4 },
        { 0, "PR30", "Program Register 30(String Source)", 4 },
        { 0, "LP", "Program Register 31(Link Pointer)", 4 },
        { 0, "SR0", "Exception/Interrupt PC", 4 },
        { 0, "SR1", "Exception/Interrupt PSW", 4 },
        { 0, "SR2", "Fatal Error PC", 4 },
        { 0, "SR3", "Fatal Error PSW", 4 },
        { 0, "SR4", "Exception Cause Register", 4 },
        { 0, "SR5", "Program Status Word", 4 },
        { 0, "SR6", "Processor ID Register", 4 },
        { 0, "SR7", "Task Control Word", 4 },
        { 0, "SR24", "Cache Control Word", 4 },
        { 0, "SR25", "Address Trap Register", 4 },

        //{ 0, "IPEND", "Interrupts Pending", 2 },
        //{ 0, "IMASK", "Interrupt Mask", 2 },
        //{ 0, "TCTRL", "Timer Control", 2 },
        //{ 0, "TPRD", "Timer Period", 2 },
        //{ 0, "TCNTR", "Timer Counter", 3 },

        { 0, "TStamp", "Timestamp", 3 },
        { 0, "", "", 0 },
};


static RegGroupType V810RegsGroup =
{
 NULL,
 V810Regs,
 NULL,
 NULL,
 VBDBG_GetRegister,
 VBDBG_SetRegister,
};

static uint32 MISC_GetRegister(const unsigned int id, char *special, const uint32 special_len)
{
 return(TIMER_GetRegister(id, special, special_len));
}

static void MISC_SetRegister(const unsigned int id, const uint32 value)
{
 TIMER_SetRegister(id, value);
}


static RegType Regs_Misc[] =
{
	{ TIMER_GSREG_TCR,	"TCR", "Timer Control Register", 1 },
	{ TIMER_GSREG_DIVCOUNTER, "DivCounter", "Timer Clock Divider Counter", 2 },
	{ TIMER_GSREG_RELOAD_VALUE, "ReloadValue", "Timer Reload Value", 2 },
	{ TIMER_GSREG_COUNTER, "Counter", "Timer Counter Value", 2 },
        { 0, "", "", 0 },
};

static RegGroupType RegsGroup_Misc =
{
        "Misc",
        Regs_Misc,
        MISC_GetRegister,
        MISC_SetRegister
};


static RegType Regs_VIP[] =
{
	{ VIP_GSREG_IPENDING,	"IPending", "Interrupts Pending", 2 },
	{ VIP_GSREG_IENABLE,	"IEnable", "Interrupts Enabled", 2 },

	{ VIP_GSREG_DPCTRL,	"DPCTRL", "DPCTRL", 2 },

	{ VIP_GSREG_BRTA,	"BRTA", "BRTA", 1 },
	{ VIP_GSREG_BRTB,	"BRTB", "BRTB", 1 },
	{ VIP_GSREG_BRTC,	"BRTC", "BRTC", 1 },
	{ VIP_GSREG_REST,	"REST", "REST", 1 },
	{ VIP_GSREG_FRMCYC,	"FRMCYC", "FRMCYC", 1 },
	{ VIP_GSREG_XPCTRL,	"XPCTRL", "XPCTRL", 2 },

	{ VIP_GSREG_SPT0,	"SPT0", "SPT0", 2 },
	{ VIP_GSREG_SPT1,	"SPT1", "SPT1", 2 },
	{ VIP_GSREG_SPT2,	"SPT2", "SPT2", 2 },
	{ VIP_GSREG_SPT3,	"SPT3", "SPT3", 2 },

	{ VIP_GSREG_GPLT0,	"GPLT0", "GPLT0", 1 },
	{ VIP_GSREG_GPLT1,	"GPLT1", "GPLT1", 1 },
	{ VIP_GSREG_GPLT2,	"GPLT2", "GPLT2", 1 },
	{ VIP_GSREG_GPLT3,	"GPLT3", "GPLT3", 1 },

	{ VIP_GSREG_JPLT0,	"JPLT0", "JPLT0", 1 },
	{ VIP_GSREG_JPLT1,	"JPLT1", "JPLT1", 1 },
	{ VIP_GSREG_JPLT2,	"JPLT2", "JPLT2", 1 },
	{ VIP_GSREG_JPLT3,	"JPLT3", "JPLT3", 1 },

	{ VIP_GSREG_BKCOL,	"BKCOL", "BKCOL", 1 },

        { 0, "", "", 0 },
};

static RegGroupType RegsGroup_VIP =
{
        "VIP",
        Regs_VIP,
        VIP_GetRegister,
        VIP_SetRegister
};


bool VBDBG_Init(void)
{
 MDFNDBG_AddRegGroup(&V810RegsGroup);
 MDFNDBG_AddRegGroup(&RegsGroup_Misc);
 MDFNDBG_AddRegGroup(&RegsGroup_VIP);

 ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "cpu", "CPU Physical", 32);
// ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "ram", "RAM", 21);


 for(int x = 0; x < 5; x++)
 {
     AddressSpaceType newt;
     char tmpname[128], tmpinfo[128];

     trio_snprintf(tmpname, 128, "vsuwd%d", x);
     trio_snprintf(tmpinfo, 128, "VSU Wave Data %d", x);

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
     newt.WaveBits = 6;
     ASpace_Add(newt); //PSG_GetAddressSpaceBytes, PSG_PutAddressSpaceBytes, tmpname, tmpinfo, 5);
 }



 return(true);
}

}
