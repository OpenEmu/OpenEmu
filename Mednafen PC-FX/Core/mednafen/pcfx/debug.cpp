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

#include "pcfx.h"

#include <string.h>
#include <trio/trio.h>
#include <stdarg.h>
#include <iconv.h>

#include "debug.h"
#include "v810/v810_cpuD.h"
#include "interrupt.h"
#include "timer.h"
#include "king.h"
#include "input.h"
#include "../cdrom/scsicd.h"

static void (*CPUHook)(uint32);
static void (*BPCallB)(uint32 PC) = NULL;
static void (*LogFunc)(const char *, const char *);
static iconv_t sjis_ict = (iconv_t)-1;
bool PCFX_LoggingOn = FALSE;

typedef struct __PCFX_BPOINT {
        uint32 A[2];
        int type;
        bool logical;
} PCFX_BPOINT;

static std::vector<PCFX_BPOINT> BreakPointsPC, BreakPointsRead, BreakPointsWrite, BreakPointsIORead, BreakPointsIOWrite;
static std::vector<PCFX_BPOINT> BreakPointsAux0Read, BreakPointsAux0Write;
static bool FoundBPoint = 0;

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

std::vector<BranchTraceResult> PCFXDBG_GetBranchTrace(void)
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

   case 0xFFC0:	// Address trap
	trio_snprintf(tmp.code, sizeof(tmp.code), "ADTR");
	break;

   case 0xFFA0 ... 0xFFBF:	// TRAP
	trio_snprintf(tmp.code, sizeof(tmp.code), "TRAP");
	break;

   case 0xFF90:	// Illegal/invalid instruction code
	trio_snprintf(tmp.code, sizeof(tmp.code), "ILL");
	break;

   case 0xFF80:	// Zero division
	trio_snprintf(tmp.code, sizeof(tmp.code), "ZD");
	break;

   case 0xFF70:
	trio_snprintf(tmp.code, sizeof(tmp.code), "FIV");	// FIV
	break;

   case 0xFF68:
	trio_snprintf(tmp.code, sizeof(tmp.code), "FZD");	// FZD
	break;

   case 0xFF64:
	trio_snprintf(tmp.code, sizeof(tmp.code), "FOV");	// FOV
	break;

   case 0xFF62:
	trio_snprintf(tmp.code, sizeof(tmp.code), "FUD");	// FUD
	break;

   case 0xFF61:
	trio_snprintf(tmp.code, sizeof(tmp.code), "FPR");	// FPR
	break;

   case 0xFF60:
	trio_snprintf(tmp.code, sizeof(tmp.code), "FRO");	// FRO
	break;

   case 0xFE00 ... 0xFEFF:
	trio_snprintf(tmp.code, sizeof(tmp.code), "INT%d", (bt->ecode >> 4) & 0xF);
	break;
  }

  ret.push_back(tmp);
 }
 return(ret);
}

void PCFXDBG_CheckBP(int type, uint32 address, unsigned int len)
{
 std::vector<PCFX_BPOINT>::iterator bpit, bpit_end;

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
 else if(type == BPOINT_IO_READ)
 {
  bpit = BreakPointsIORead.begin();
  bpit_end = BreakPointsIORead.end();
 }
 else if(type == BPOINT_IO_WRITE)
 {
  bpit = BreakPointsIOWrite.begin();
  bpit_end = BreakPointsIOWrite.end();
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

enum
{
 SVT_NONE = 0,
 SVT_PTR,
 SVT_STRINGPTR,
 SVT_INT,
 SVT_UCHAR,
 SVT_LONG,
 SVT_ULONG,
};

typedef struct
{
 unsigned int number;
 const char *name;
 int arguments;
 int argument_types[16];
} syscall_t;

static const syscall_t SysDefs[] =
{
 {  0, "fsys_init", 3, { SVT_PTR, SVT_PTR, SVT_PTR} },
 {  1, "fsys_mount", 2, { SVT_PTR, SVT_PTR } },
 {  2, "fsys_ctrl", 4, { SVT_STRINGPTR, SVT_INT, SVT_PTR, SVT_INT } },
 {  3, "fsys_getfsys", 1, { SVT_PTR } },
 {  4, "fsys_format", 2, { SVT_PTR, SVT_PTR } },
 {  5, "fsys_diskfree", 1, { SVT_STRINGPTR } },
 {  6, "fsys_getblocks", 1, { SVT_PTR } },
 {  7, "fsys_open", 2, { SVT_STRINGPTR, SVT_INT } },
 {  8, "fsys_read", 3, { SVT_INT, SVT_PTR, SVT_INT } },
 {  9, "fsys_write", 3, { SVT_INT, SVT_PTR, SVT_INT } },
 { 10, "fsys_seek", 3, { SVT_INT, SVT_LONG, SVT_INT } },
 { 11, "fsys_htime", 2, { SVT_INT, SVT_LONG} },
 { 12, "fsys_close", 1, { SVT_INT } },
 { 13, "fsys_delete", 1, { SVT_STRINGPTR } },
 { 14, "fsys_rename", 2, { SVT_STRINGPTR } },
 { 15, "fsys_mkdir", 1, { SVT_STRINGPTR } },
 { 16, "fsys_rmdir", 1, { SVT_STRINGPTR } },
 { 17, "fsys_chdir", 1, { SVT_STRINGPTR } },
 { 18, "fsys_curdir", 1, { SVT_PTR } },
 { 19, "fsys_ffiles", 2, { SVT_PTR, SVT_PTR } },
 { 20, "fsys_nfiles", 1, { SVT_PTR } },
 { 21, "fsys_efiles", 1, { SVT_PTR } },
 { 22, "fsys_datetime", 1, { SVT_ULONG } },
 { 23, "fsys_m_init", 2, { SVT_PTR, SVT_PTR } },
 { 24, "fsys_malloc", 1, { SVT_INT } },
 { 25, "fsys_free", 1, { SVT_INT } },
 { 26, "fsys_setblock", 2, { SVT_PTR, SVT_INT } },
};

static void DoSyscallLog(void)
{
 uint32 ws = 0;
 unsigned int which = 0;
 unsigned int nargs = 0;
 const char *func_name = "<unknown>";
 char argsbuffer[2048];

 for(unsigned int i = 0; i < sizeof(SysDefs) / sizeof(syscall_t); i++)
 {
  if(SysDefs[i].number == PCFX_V810.GetPR(10))
  {
   nargs = SysDefs[i].arguments;
   func_name = SysDefs[i].name;
   which = i;
   break;
  }
 }

 {
  char *pos = argsbuffer;

  argsbuffer[0] = 0;

  pos += trio_sprintf(pos, "(");
  for(unsigned int i = 0; i < nargs; i++)
  {
   if(SysDefs[which].argument_types[i] == SVT_STRINGPTR)
   {
    uint8 quickiebuf[64 + 1];
    int qbuf_index = 0;
    bool error_thing = FALSE;

    do
    {
     uint32 A = PCFX_V810.GetPR(6 + i) + qbuf_index;

     quickiebuf[qbuf_index] = 0;

     if(A >= 0x80000000 && A < 0xF0000000)
     {
      error_thing = TRUE;
      break;
     }

     quickiebuf[qbuf_index] = mem_peekbyte(ws, A);
    } while(quickiebuf[qbuf_index] && ++qbuf_index < 64);

    if(qbuf_index == 64) 
     error_thing = TRUE;

    quickiebuf[64] = 0;

    if(error_thing)
     pos += trio_sprintf(pos, "0x%08x, ", PCFX_V810.GetPR(6 + i));
    else
    {
	uint8 quickiebuf_utf8[64 * 6 + 1];
	char *in_ptr, *out_ptr;
	size_t ibl, obl;

	ibl = qbuf_index;
	obl = sizeof(quickiebuf_utf8) - 1;

	in_ptr = (char *)quickiebuf;
	out_ptr = (char *)quickiebuf_utf8;

	if(iconv(sjis_ict, (ICONV_CONST char **)&in_ptr, &ibl, &out_ptr, &obl) == (size_t) -1)
	{
	 pos += trio_sprintf(pos, "0x%08x, ", PCFX_V810.GetPR(6 + i));
	}
	else
	{
	 *out_ptr = 0;
	 pos += trio_sprintf(pos, "@0x%08x=\"%s\", ", PCFX_V810.GetPR(6 + i), quickiebuf_utf8);
	}
    }
   }
   else
    pos += trio_sprintf(pos, "0x%08x, ", PCFX_V810.GetPR(6 + i));
  }

  // Get rid of the trailing comma and space
  if(nargs)
   pos-=2;

  trio_sprintf(pos, ");");
 }

 PCFXDBG_DoLog("SYSCALL", "0x%02x, %s: %s", PCFX_V810.GetPR(10), func_name, argsbuffer);
}

static void CPUHandler(uint32 PC)
{
 std::vector<PCFX_BPOINT>::iterator bpit;

 for(bpit = BreakPointsPC.begin(); bpit != BreakPointsPC.end(); bpit++)
 {
  if(PC >= bpit->A[0] && PC <= bpit->A[1])
  {
   FoundBPoint = TRUE;
   break;
  }
 }
 PCFX_V810.CheckBreakpoints(PCFXDBG_CheckBP, mem_peekhword, NULL);	// FIXME: mem_peekword

 if(PCFX_LoggingOn)
 {
  // FIXME:  There is a little race condition if a user turns on logging right between jump instruction and the first
  // instruction at 0xFFF0000C, in which case the call-from address will be wrong.
  static uint32 lastPC = ~0;

  if(PC == 0xFFF0000C)
  {
   static const char *font_sizes[6] =
   {
    "KANJI16x16", "KANJI12x12", "ANK8x16", "ANK6x12", "ANK8x8", "ANK8x12"
   };

   // FIXME, overflow possible and speed
   PCFXDBG_DoLog("ROMFONT", "0x%08x->0xFFF0000C, PR7=0x%08x=%s, PR6=0x%04x = %s", lastPC, PCFX_V810.GetPR(7), (PCFX_V810.GetPR(7) > 5) ? "?" : font_sizes[PCFX_V810.GetPR(7)], PCFX_V810.GetPR(6) & 0xFFFF, PCFXDBG_ShiftJIS_to_UTF8(PCFX_V810.GetPR(6) & 0xFFFF));
   setvbuf(stdout, NULL, _IONBF, 0);
   printf("%s", PCFXDBG_ShiftJIS_to_UTF8(PCFX_V810.GetPR(6) & 0xFFFF));
  }
  else if(PC == 0xFFF00008)
   DoSyscallLog();

  lastPC = PC;
 }

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

 HappyTest = PCFX_LoggingOn || BreakPointsPC.size() || BreakPointsRead.size() || BreakPointsWrite.size() ||
		BreakPointsIOWrite.size() || BreakPointsIORead.size() || BreakPointsAux0Read.size() || BreakPointsAux0Write.size();

 void (*cpuh)(uint32);

 cpuh = HappyTest ? CPUHandler : CPUHook;

 PCFX_V810.SetCPUHook(cpuh, cpuh ? AddBranchTrace : NULL);
}

void PCFXDBG_FlushBreakPoints(int type)
{
 std::vector<PCFX_BPOINT>::iterator bpit;

 if(type == BPOINT_READ)
  BreakPointsRead.clear();
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.clear();
 else if(type == BPOINT_IO_READ)
  BreakPointsIORead.clear();
 else if(type == BPOINT_IO_WRITE)
  BreakPointsIOWrite.clear();
 else if(type == BPOINT_AUX_READ)
  BreakPointsAux0Read.clear();
 else if(type == BPOINT_AUX_WRITE)
  BreakPointsAux0Write.clear();
 else if(type == BPOINT_PC)
  BreakPointsPC.clear();

 RedoCPUHook();
 KING_NotifyOfBPE(BreakPointsAux0Read.size(), BreakPointsAux0Write.size());
}

void PCFXDBG_AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical)
{
 PCFX_BPOINT tmp;

 tmp.A[0] = A1;
 tmp.A[1] = A2;
 tmp.type = type;

 if(type == BPOINT_READ)
  BreakPointsRead.push_back(tmp);
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.push_back(tmp);
 else if(type == BPOINT_IO_READ)
  BreakPointsIORead.push_back(tmp);
 else if(type == BPOINT_IO_WRITE)
  BreakPointsIOWrite.push_back(tmp);
 else if(type == BPOINT_AUX_READ)
  BreakPointsAux0Read.push_back(tmp);
 else if(type == BPOINT_AUX_WRITE)
  BreakPointsAux0Write.push_back(tmp);
 else if(type == BPOINT_PC)
  BreakPointsPC.push_back(tmp);

 RedoCPUHook();
 KING_NotifyOfBPE(BreakPointsAux0Read.size(), BreakPointsAux0Write.size());
}

static uint16 dis_readhw(uint32 A)
{
 return(mem_peekhword(0, A));
}

void PCFXDBG_Disassemble(uint32 &a, uint32 SpecialA, char *TextBuf)
{
 return(v810_dis(a, 1, TextBuf, dis_readhw));
}

uint32 PCFXDBG_MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical)
{
 uint32 ret = 0;
 uint32 ws = 0;

 for(unsigned int i = 0; i < bsize; i++)
 {
  A &= 0xFFFFFFFF;
  ret |= mem_peekbyte(ws, A) << (i * 8);
  A++;
 }

 return(ret);
}

uint32 PCFXDBG_GetRegister(const std::string &name, std::string *special)
{
 if(name == "PC")
 {
  return(PCFX_V810.GetPC());
 }
 const char *thestring = name.c_str();

 if(!strncmp(thestring, "PR", 2))
 {
  return(PCFX_V810.GetPR(atoi(thestring + 2)));
 }
 else if(!strcmp(thestring, "HSP"))
  return(PCFX_V810.GetPR(2));
 else if(!strcmp(thestring, "SP"))
  return(PCFX_V810.GetPR(3));
 else if(!strcmp(thestring, "GP"))
  return(PCFX_V810.GetPR(4));
 else if(!strcmp(thestring, "TP"))
  return(PCFX_V810.GetPR(5));
 else if(!strcmp(thestring, "LP"))
  return(PCFX_V810.GetPR(31));
 else if(!strcmp(thestring, "TStamp"))
  return(PCFX_V810.v810_timestamp);
 else if(!strncmp(thestring, "SR", 2))
 {
  int which_one = atoi(thestring + 2);
  uint32 val =  PCFX_V810.GetSR(which_one);

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

 if(PCFXIRQ_GetRegister(name, val, special) || FXTIMER_GetRegister(name, val, special) || FXINPUT_GetRegister(name, val, special))
  return(val);

 return(val);
}

void PCFXDBG_SetRegister(const std::string &name, uint32 value)
{
 if(name == "PC")
 {
  PCFX_V810.SetPC(value & ~1);
  return;
 }

 const char *thestring = name.c_str();

 if(!strncmp(thestring, "PR", 2))
 {
  PCFX_V810.SetPR(atoi(thestring + 2), value);
 }
 else if(!strcmp(thestring, "HSP"))
  PCFX_V810.SetPR(2, value);
 else if(!strcmp(thestring, "SP"))
  PCFX_V810.SetPR(3, value);
 else if(!strcmp(thestring, "GP"))
  PCFX_V810.SetPR(4, value);
 else if(!strcmp(thestring, "TP"))
  PCFX_V810.SetPR(5, value);
 else if(!strcmp(thestring, "LP"))
  PCFX_V810.SetPR(31, value);
 else if(!strncmp(thestring, "SR", 2))
 {
  PCFX_V810.SetSR(atoi(thestring + 2), value);
 }
 else if(PCFXIRQ_SetRegister(name, value))
 {

 }

}

void PCFXDBG_SetCPUCallback(void (*callb)(uint32 PC))
{
 CPUHook = callb;
 RedoCPUHook();
}

void PCFXDBG_SetBPCallback(void (*callb)(uint32 PC))
{
 BPCallB = callb;
}

void PCFXDBG_DoLog(const char *type, const char *format, ...)
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

void PCFXDBG_SetLogFunc(void (*func)(const char *, const char *))
{
 LogFunc = func;

 PCFX_LoggingOn = func ? TRUE : FALSE;
 SCSICD_SetLog(func ? PCFXDBG_DoLog : NULL);
 KING_SetLogFunc(func ? PCFXDBG_DoLog : NULL);

 if(PCFX_LoggingOn)
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
 RedoCPUHook();
}

char *PCFXDBG_ShiftJIS_to_UTF8(const uint16 sjc)
{
 static char ret[16];
 char inbuf[3];
 char *in_ptr, *out_ptr;
 size_t ibl, obl;

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

 *out_ptr = 0;

 return(ret);
}
