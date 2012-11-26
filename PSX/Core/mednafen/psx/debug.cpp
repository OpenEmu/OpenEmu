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

#include "psx.h"
#include "timer.h"
#include "cdc.h"
#include "spu.h"

namespace MDFN_IEN_PSX
{

extern PS_GPU *GPU;
extern PS_SPU *SPU;

static void (*CPUHook)(uint32) = NULL;
static void (*BPCallB)(uint32 PC) = NULL;

struct PSX_BPOINT
{
	uint32 A[2];
	int type;
};

static std::vector<PSX_BPOINT> BreakPointsPC, BreakPointsRead, BreakPointsWrite;
static bool FoundBPoint;

static int BTIndex = 0;

struct BTEntry
{
 uint32 from;
 uint32 to;
 uint32 branch_count;
 bool exception;
};

#define NUMBT 24
static BTEntry BTEntries[NUMBT];

void DBG_Break(void)
{
 FoundBPoint = true;
}

static void AddBranchTrace(uint32 from, uint32 to, bool exception)
{
 BTEntry *prevbt = &BTEntries[(BTIndex + NUMBT - 1) % NUMBT];

 //if(BTEntries[(BTIndex - 1) & 0xF] == PC) return;

 if(prevbt->from == from && prevbt->to == to && prevbt->exception == exception && prevbt->branch_count < 0xFFFFFFFF)
  prevbt->branch_count++;
 else
 {
  BTEntries[BTIndex].from = from;
  BTEntries[BTIndex].to = to;
  BTEntries[BTIndex].exception = exception;
  BTEntries[BTIndex].branch_count = 1;

  BTIndex = (BTIndex + 1) % NUMBT;
 }
}

static std::vector<BranchTraceResult> GetBranchTrace(void)
{
 BranchTraceResult tmp;
 std::vector<BranchTraceResult> ret;

 for(int x = 0; x < NUMBT; x++)
 {
  const BTEntry *bt = &BTEntries[(x + BTIndex) % NUMBT];

  tmp.count = bt->branch_count;
  trio_snprintf(tmp.from, sizeof(tmp.from), "%08x", bt->from);
  trio_snprintf(tmp.to, sizeof(tmp.to), "%08x", bt->to);
  trio_snprintf(tmp.code, sizeof(tmp.code), "%s", bt->exception ? "e" : "");

  ret.push_back(tmp);
 }
 return(ret);
}

void CheckCPUBPCallB(bool write, uint32 address, unsigned int len)
{
 std::vector<PSX_BPOINT>::iterator bpit;
 std::vector<PSX_BPOINT>::iterator bpit_end;

 if(write)
 {
  bpit = BreakPointsWrite.begin();
  bpit_end = BreakPointsWrite.end();
 }
 else
 {
  bpit = BreakPointsRead.begin();
  bpit_end = BreakPointsRead.end();
 }

 while(bpit != bpit_end)
 {
  if(address >= bpit->A[0] && address <= bpit->A[1])
  {
   FoundBPoint = true;
   break;
  }
  bpit++;
 }
}

static void CPUHandler(uint32 PC)
{
 std::vector<PSX_BPOINT>::iterator bpit;

 if(PC == 0xB0 && CPU->GetRegister(PS_CPU::GSREG_GPR + 9, NULL, 0) == 0x3D)
 {
  putchar(CPU->GetRegister(PS_CPU::GSREG_GPR + 4, NULL, 0));
  //exit(1);
  //puts((const char *)&MainRAM[CPU->GetRegister(PS_CPU::GSREG_GPR + 4, NULL, 0) & 0x1FFFFF]);
 }
  

 // FIXME/TODO: Call ForceEventUpdates() somewhere

 for(bpit = BreakPointsPC.begin(); bpit != BreakPointsPC.end(); bpit++)
 {
  if(PC >= bpit->A[0] && PC <= bpit->A[1])
  {
   FoundBPoint = true;
   break;
  }
 }

 CPU->CheckBreakpoints(CheckCPUBPCallB, PSX_MemPeek32(PC));

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
 const bool HappyTest = BreakPointsPC.size() || BreakPointsRead.size() || BreakPointsWrite.size();
 void (*cpuh)(uint32);

 cpuh = HappyTest ? CPUHandler : CPUHook;

 CPU->SetCPUHook(cpuh, cpuh ? AddBranchTrace : NULL);
}

static void FlushBreakPoints(int type)
{
 if(type == BPOINT_READ)
  BreakPointsRead.clear();
 else if(type == BPOINT_WRITE)
  BreakPointsWrite.clear();
 else if(type == BPOINT_PC)
  BreakPointsPC.clear();

 RedoCPUHook();
}

static void AddBreakPoint(int type, unsigned int A1, unsigned int A2, bool logical)
{
 PSX_BPOINT tmp;

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

static void SetCPUCallback(void (*callb)(uint32 PC))
{
 CPUHook = callb;
 RedoCPUHook();
}

static void SetBPCallback(void (*callb)(uint32 PC))
{
 BPCallB = callb;
}


static void GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer)
{
 if(!strcmp(name, "cpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFFFFFF;
   *Buffer = PSX_MemPeek8(Address);
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "ram"))
 {
  while(Length--)
  {
   Address &= 0x1FFFFF;
   *Buffer = PSX_MemPeek8(Address);
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "spu"))
 {
  while(Length--)
  {
   Address &= 0x7FFFF;
   *Buffer = SPU->PeekSPURAM(Address >> 1) >> ((Address & 1) * 8);
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "gpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFFF;
   *Buffer = GPU->PeekRAM(Address >> 1) >> ((Address & 1) * 8);
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
   Address &= 0xFFFFFFFF;

   // TODO
   PSX_MemWrite8(0, Address, *Buffer);

   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "gpu"))
 {
  while(Length--)
  {
   Address &= 0xFFFFF;

   uint16 peeko = GPU->PeekRAM(Address >> 1);

   GPU->PokeRAM(Address >> 1, (*Buffer << ((Address & 1) * 8)) | (peeko & (0xFF00 >> ((Address & 1) * 8))) );
   Address++;
   Buffer++;
  }
 }
 else if(!strcmp(name, "spu"))
 {
  while(Length--)
  {
   Address &= 0x7FFFF;

   uint16 peeko = SPU->PeekSPURAM(Address >> 1);

   SPU->PokeSPURAM(Address >> 1, (*Buffer << ((Address & 1) * 8)) | (peeko & (0xFF00 >> ((Address & 1) * 8))) );
   Address++;
   Buffer++;
  }
 }

}

static uint32 MemPeek(uint32 A, unsigned int bsize, bool hl, bool logical)
{
 uint32 ret = 0;

 for(unsigned int i = 0; i < bsize; i++)
  ret |= PSX_MemPeek8(A + i) << (i * 8);

 return(ret);
}

static void Disassemble(uint32 &A, uint32 SpecialA, char *TextBuf)
{
 assert(!(A & 0x3));

 const uint32 instr = PSX_MemPeek32(A);


 strncpy(TextBuf, DisassembleMIPS(A, instr).c_str(), 256);
 TextBuf[255] = 0;

// trio_snprintf(TextBuf, 256, "0x%08x", instr);

 A += 4;
}

static MDFN_Surface *GfxDecode_Buf = NULL;
static int GfxDecode_Line = -1;
static int GfxDecode_Layer = 0;
static int GfxDecode_Scroll = 0;
static int GfxDecode_PBN = 0;

static void DoGfxDecode(void)
{
 unsigned tp_w, tp_h;

 tp_w = 256;
 tp_h = 256;

 if(GfxDecode_Buf)
 {
  for(int sy = 0; sy < GfxDecode_Buf->h; sy++)
  {
   for(int sx = 0; sx < GfxDecode_Buf->w; sx++)
   {
    unsigned fb_x = ((sx % GfxDecode_Buf->w) + ((sy + GfxDecode_Scroll) / GfxDecode_Buf->w * GfxDecode_Buf->w)) & 1023;
    unsigned fb_y = (((sy + GfxDecode_Scroll) % GfxDecode_Buf->w) + ((((sx % GfxDecode_Buf->w) + ((sy + GfxDecode_Scroll) / GfxDecode_Buf->w * GfxDecode_Buf->w)) / 1024) * 256)) & 511;

    uint16 pixel = GPU->PeekRAM(fb_y * 1024 + fb_x);

    GfxDecode_Buf->pixels[(sy * GfxDecode_Buf->w * 3) + sx] = GfxDecode_Buf->MakeColor(((pixel >> 0) & 0x1F) * 255 / 31,
											((pixel >> 5) & 0x1F) * 255 / 31,
											((pixel >> 10) & 0x1F) * 255 / 31, 0xFF);
   }
  }
 }
}


void DBG_GPUScanlineHook(unsigned scanline)
{
 if((int)scanline == GfxDecode_Line)
 {
  DoGfxDecode();
 }
}


static void SetGraphicsDecode(MDFN_Surface *surface, int line, int which, int xscroll, int yscroll, int pbn)
{
 GfxDecode_Buf = surface;
 GfxDecode_Line = line;
 GfxDecode_Layer = which;
 GfxDecode_Scroll = yscroll;
 GfxDecode_PBN = pbn;

 if(GfxDecode_Buf && GfxDecode_Line == -1)
  DoGfxDecode();
}

DebuggerInfoStruct PSX_DBGInfo =
{
 "shift_jis",
 4,		// Max instruction byte size
 4,             // Instruction alignment(bytes)
 32,		// Logical address bits
 32,		// Physical address bits
 0x00000000,	// Default watch addr
 ~0U,		// ZP addr

 MemPeek,
 Disassemble,
 NULL,
 NULL,	//ForceIRQ,
 NULL, //NESDBG_GetVector,
 FlushBreakPoints,
 AddBreakPoint,
 SetCPUCallback,
 SetBPCallback,
 GetBranchTrace,
 SetGraphicsDecode,
 NULL, //PCFXDBG_SetLogFunc,
};

static RegType Regs_Misc[] =
{
 { TIMER_GSREG_COUNTER0,	"COUNTER0", "Counter 0", 2 	},
 { TIMER_GSREG_MODE0,		"MODE0", "Mode 0", 	2   	},
 { TIMER_GSREG_TARGET0,		"TARGET0", "Target 0",	2	},

 { 0, "------", "", 0xFFFF },


 { TIMER_GSREG_COUNTER1,	"COUNTER1", "Counter 1", 2 	},
 { TIMER_GSREG_MODE1,		"MODE1", "Mode 1", 	2   	},
 { TIMER_GSREG_TARGET1,		"TARGET1", "Target 1",	2	},

 { 0, "------", "", 0xFFFF },

 { TIMER_GSREG_COUNTER2,	"COUNTER2", "Counter 2", 2 	},
 { TIMER_GSREG_MODE2,		"MODE2", "Mode 2", 	2   	},
 { TIMER_GSREG_TARGET2,		"TARGET2", "Target 2",	2	},

 { 0, "------", "", 0xFFFF },
 { 0, "------", "", 0xFFFF },

 { 0x10000 | IRQ_GSREG_ASSERTED,		"ASSERTED",	"IRQ Asserted",	2 },
 { 0x10000 | IRQ_GSREG_STATUS,		"STATUS",	"IRQ Status", 2 },
 { 0x10000 | IRQ_GSREG_MASK,		"MASK",		"IRQ Mask", 2 },

 { 0, "", "", 0 }
};


static uint32 GetRegister_Misc(const unsigned int id, char *special, const uint32 special_len)
{
 if(id & 0x10000)
  return(IRQ_GetRegister(id & 0xFFFF, special, special_len));
 else
  return(TIMER_GetRegister(id & 0xFFFF, special, special_len));
}

static void SetRegister_Misc(const unsigned int id, uint32 value)
{
 if(id & 0x10000)
  IRQ_SetRegister(id & 0xFFFF, value);
 else
  TIMER_SetRegister(id & 0xFFFF, value);
}

static RegGroupType MiscRegsGroup =
{
 NULL,
 Regs_Misc,
 GetRegister_Misc,
 SetRegister_Misc
};

static RegType Regs_SPU[] =
{
 { PS_SPU::GSREG_SPUCONTROL, "SPUCTRL", "SPU Control", 2 },

 { PS_SPU::GSREG_FM_ON, "FMOn", "FM Enable", 3 },
 { PS_SPU::GSREG_NOISE_ON, "NoiseOn", "Noise Enable", 3 },
 { PS_SPU::GSREG_REVERB_ON, "ReverbOn", "Reverb Enable", 3 },

 { PS_SPU::GSREG_CDVOL_L, "CDVolL", "CD Volume Left", 2 },
 { PS_SPU::GSREG_CDVOL_R, "CDVolR", "CD Volume Right", 2 },

 { PS_SPU::GSREG_DRYVOL_CTRL_L, "DryVolCL", "Dry Volume Control Left", 2 },
 { PS_SPU::GSREG_DRYVOL_CTRL_R, "DryVolCR", "Dry Volume Control Right", 2 },

 { PS_SPU::GSREG_DRYVOL_L, "DryVolL", "Dry Volume Left", 2 },
 { PS_SPU::GSREG_DRYVOL_R, "DryVolR", "Dry Volume Right", 2 },

 { PS_SPU::GSREG_WETVOL_L, "WetVolL", "Wet Volume Left", 2 },
 { PS_SPU::GSREG_WETVOL_R, "WetVolR", "Wet Volume Right", 2 },

 { PS_SPU::GSREG_RWADDR, "RWAddr", "SPURAM Read/Write Address", 3 },

 { PS_SPU::GSREG_IRQADDR, "IRQAddr", "IRQ Compare Address", 3 },

 { PS_SPU::GSREG_REVERBWA, "ReverbWA", "Reverb Work Area(Raw)", 2 },

 { PS_SPU::GSREG_VOICEON, "VoiceOn", "Voice On", 3 },
 { PS_SPU::GSREG_VOICEOFF, "VoiceOff", "Voice Off", 3 },
 { PS_SPU::GSREG_BLOCKEND, "BlockEnd", "Block End", 3 },


 { 0, "------", "", 0xFFFF },

 { PS_SPU::GSREG_FB_SRC_A, "FB_SRC_A", "", 2 },
 { PS_SPU::GSREG_FB_SRC_B, "FB_SRC_B", "", 2 },
 { PS_SPU::GSREG_IIR_ALPHA, "IIR_ALPHA", "", 2 },
 { PS_SPU::GSREG_ACC_COEF_A, "ACC_COEF_A", "", 2 },
 { PS_SPU::GSREG_ACC_COEF_B, "ACC_COEF_B", "", 2 },
 { PS_SPU::GSREG_ACC_COEF_C, "ACC_COEF_C", "", 2 },
 { PS_SPU::GSREG_ACC_COEF_D, "ACC_COEF_D", "", 2 },
 { PS_SPU::GSREG_IIR_COEF, "IIR_COEF", "", 2 },
 { PS_SPU::GSREG_FB_ALPHA, "FB_ALPHA", "", 2 },
 { PS_SPU::GSREG_FB_X, "FB_X", "", 2 },
 { PS_SPU::GSREG_IIR_DEST_A0, "IIR_DEST_A0", "", 2 },
 { PS_SPU::GSREG_IIR_DEST_A1, "IIR_DEST_A1", "", 2 },
 { PS_SPU::GSREG_ACC_SRC_A0, "ACC_SRC_A0", "", 2 },
 { PS_SPU::GSREG_ACC_SRC_A1, "ACC_SRC_A1", "", 2 },
 { PS_SPU::GSREG_ACC_SRC_B0, "ACC_SRC_B0", "", 2 },
 { PS_SPU::GSREG_ACC_SRC_B1, "ACC_SRC_B1", "", 2 },
 { PS_SPU::GSREG_IIR_SRC_A0, "IIR_SRC_A0", "", 2 },
 { PS_SPU::GSREG_IIR_SRC_A1, "IIR_SRC_A1", "", 2 },
 { PS_SPU::GSREG_IIR_DEST_B0, "IIR_DEST_B0", "", 2 },
 { PS_SPU::GSREG_IIR_DEST_B1, "IIR_DEST_B1", "", 2 },
 { PS_SPU::GSREG_ACC_SRC_C0, "ACC_SRC_C0", "", 2 },
 { PS_SPU::GSREG_ACC_SRC_C1, "ACC_SRC_C1", "", 2 },
 { PS_SPU::GSREG_ACC_SRC_D0, "ACC_SRC_D0", "", 2 },
 { PS_SPU::GSREG_ACC_SRC_D1, "ACC_SRC_D1", "", 2 },
 { PS_SPU::GSREG_IIR_SRC_B1, "IIR_SRC_B1", "", 2 },
 { PS_SPU::GSREG_IIR_SRC_B0, "IIR_SRC_B0", "", 2 },
 { PS_SPU::GSREG_MIX_DEST_A0, "MIX_DEST_A0", "", 2 },
 { PS_SPU::GSREG_MIX_DEST_A1, "MIX_DEST_A1", "", 2 },
 { PS_SPU::GSREG_MIX_DEST_B0, "MIX_DEST_B0", "", 2 },
 { PS_SPU::GSREG_MIX_DEST_B1, "MIX_DEST_B1", "", 2 },
 { PS_SPU::GSREG_IN_COEF_L, "IN_COEF_L", "", 2 },
 { PS_SPU::GSREG_IN_COEF_R, "IN_COEF_R", "", 2 },

 { 0, "", "", 0 },
};

#define VOICE_HELPER(v)									\
 { 0, "--V"#v"--", "", 0xFFFF },							\
 { PS_SPU:: GSREG_V0_VOL_CTRL_L + v * 256, "VolCL", "Volume Control Left", 2 },		\
 { PS_SPU:: GSREG_V0_VOL_CTRL_R + v * 256, "VolCR", "Volume Control Right", 2 },	\
 { PS_SPU:: GSREG_V0_VOL_L + v * 256, "VolL", "Volume Left", 2 },			\
 { PS_SPU:: GSREG_V0_VOL_R + v * 256, "VolR", "Volume Right", 2 },			\
 { PS_SPU:: GSREG_V0_PITCH + v * 256, "Pitch", "Pitch", 2 },				\
 { PS_SPU:: GSREG_V0_STARTADDR + v * 256, "SAddr", "Start Address", 3 },		\
 { PS_SPU:: GSREG_V0_ADSR_CTRL + v * 256, "ADSRCTRL", "ADSR Control", 4 },		\
 { PS_SPU:: GSREG_V0_ADSR_LEVEL + v * 256, "ADSRLev", "ADSR Level", 2 },		\
 { PS_SPU:: GSREG_V0_LOOP_ADDR + v * 256, "LAddr", "Loop Address", 3 },			\
 { PS_SPU:: GSREG_V0_READ_ADDR + v * 256, "RAddr", "Read Address", 3 }


static RegType Regs_SPU_Voices[] =
{
#if 1
 VOICE_HELPER(0),
 VOICE_HELPER(1),
 VOICE_HELPER(2),
 VOICE_HELPER(3),
#else
 VOICE_HELPER(9),
 VOICE_HELPER(12),
 VOICE_HELPER(17),
 VOICE_HELPER(22),

 //VOICE_HELPER(20),
 //VOICE_HELPER(21),
 //VOICE_HELPER(22),
 //VOICE_HELPER(23),
#endif
 { 0, "", "", 0 },
};


static uint32 GetRegister_SPU(const unsigned int id, char *special, const uint32 special_len)
{
 return(SPU->GetRegister(id, special, special_len));
}

static void SetRegister_SPU(const unsigned int id, uint32 value)
{
 SPU->SetRegister(id, value);
}

static RegGroupType SPURegsGroup =
{
 NULL,
 Regs_SPU,
 GetRegister_SPU,
 SetRegister_SPU
};


static RegGroupType SPUVoicesRegsGroup =
{
 NULL,
 Regs_SPU_Voices,
 GetRegister_SPU,
 SetRegister_SPU
};

static RegType Regs_CPU[] =
{
	{ PS_CPU::GSREG_PC, 	   "PC", "PC", 4 },
	{ PS_CPU::GSREG_PC_NEXT,   "NPC", "Next PC", 4 },
	{ PS_CPU::GSREG_IN_BD_SLOT, "INBD", "In Branch Delay Slot", 1 },
        { 0, "------", "", 0xFFFF },
        { PS_CPU::GSREG_GPR + 1,   "at", "Assembler Temporary", 4 },
        { PS_CPU::GSREG_GPR + 2,   "v0", "Return Value 0", 4 },
        { PS_CPU::GSREG_GPR + 3,   "v1", "Return Value 1", 4 },
        { PS_CPU::GSREG_GPR + 4,   "a0", "Argument 0", 4 },
        { PS_CPU::GSREG_GPR + 5,   "a1", "Argument 1", 4 },
        { PS_CPU::GSREG_GPR + 6,   "a2", "Argument 2", 4 },
        { PS_CPU::GSREG_GPR + 7,   "a3", "Argument 3", 4 },
        { PS_CPU::GSREG_GPR + 8,   "t0", "Temporary 0", 4 },
        { PS_CPU::GSREG_GPR + 9,   "t1", "Temporary 1", 4 },
        { PS_CPU::GSREG_GPR + 10,   "t2", "Temporary 2", 4 },
        { PS_CPU::GSREG_GPR + 11,   "t3", "Temporary 3", 4 },
        { PS_CPU::GSREG_GPR + 12,   "t4", "Temporary 4", 4 },
        { PS_CPU::GSREG_GPR + 13,   "t5", "Temporary 5", 4 },
        { PS_CPU::GSREG_GPR + 14,   "t6", "Temporary 6", 4 },
        { PS_CPU::GSREG_GPR + 15,   "t7", "Temporary 7", 4 },
        { PS_CPU::GSREG_GPR + 16,   "s0", "Subroutine Reg Var 0", 4 },
        { PS_CPU::GSREG_GPR + 17,   "s1", "Subroutine Reg Var 1", 4 },
        { PS_CPU::GSREG_GPR + 18,   "s2", "Subroutine Reg Var 2", 4 },
        { PS_CPU::GSREG_GPR + 19,   "s3", "Subroutine Reg Var 3", 4 },
        { PS_CPU::GSREG_GPR + 20,   "s4", "Subroutine Reg Var 4", 4 },
        { PS_CPU::GSREG_GPR + 21,   "s5", "Subroutine Reg Var 5", 4 },
        { PS_CPU::GSREG_GPR + 22,   "s6", "Subroutine Reg Var 6", 4 },
        { PS_CPU::GSREG_GPR + 23,   "s7", "Subroutine Reg Var 7", 4 },
        { PS_CPU::GSREG_GPR + 24,   "t8", "Temporary 8", 4 },
        { PS_CPU::GSREG_GPR + 25,   "t9", "Temporary 9", 4 },
        { PS_CPU::GSREG_GPR + 26,   "k0", "Interrupt/Trap Handler Reg 0", 4 },
        { PS_CPU::GSREG_GPR + 27,   "k1", "Interrupt/Trap Handler Reg 1", 4 },
        { PS_CPU::GSREG_GPR + 28,   "gp", "Global Pointer", 4 },
        { PS_CPU::GSREG_GPR + 29,   "sp", "Stack Pointer", 4 },
        { PS_CPU::GSREG_GPR + 30,   "s8", "Subroutine Reg Var 8/Frame Pointer", 4 },
        { PS_CPU::GSREG_GPR + 31,   "ra", "Return Address", 4 },
        { 0, "------", "", 0xFFFF },

	{ PS_CPU::GSREG_SR,	"SR",	"Status Register", 4 },
	{ PS_CPU::GSREG_CAUSE,	"CAU","Cause Register", 4 },
	{ PS_CPU::GSREG_EPC,	"EPC",	"EPC Register", 4 },
	{ 0, "", "", 0 }
};

static uint32 GetRegister_CPU(const unsigned int id, char *special, const uint32 special_len)
{
 return(CPU->GetRegister(id, special, special_len));
}

static void SetRegister_CPU(const unsigned int id, uint32 value)
{
 CPU->SetRegister(id, value);
}

static RegGroupType CPURegsGroup =
{
 NULL,
 Regs_CPU,
 GetRegister_CPU,
 SetRegister_CPU
};


bool DBG_Init(void)
{
 CPUHook = NULL;
 BPCallB = NULL;
 FoundBPoint = false;

 MDFNDBG_AddRegGroup(&CPURegsGroup);
 MDFNDBG_AddRegGroup(&MiscRegsGroup);
 MDFNDBG_AddRegGroup(&SPURegsGroup);
 MDFNDBG_AddRegGroup(&SPUVoicesRegsGroup);
 ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "cpu", "CPU Physical", 32);
 ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "ram", "CPU Main Ram", 21);
 ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "spu", "SPU RAM", 19);
 ASpace_Add(GetAddressSpaceBytes, PutAddressSpaceBytes, "gpu", "GPU RAM", 20);
 return(true);
}



}

