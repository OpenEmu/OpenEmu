/* V810 Emulator
 *
 * Copyright (C) 2006 David Tucker
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

/* Alternatively, the V810 emulator code(and all V810 emulation header files) can be used/distributed under the following license(you can adopt either
   license exclusively for your changes by removing one of these license headers, but it's STRONGLY preferable
   to keep your changes dual-licensed as well):

This Reality Boy emulator is copyright (C) David Tucker 1997-2008, all rights
reserved.   You may use this code as long as you make no money from the use of
this code and you acknowledge the original author (Me).  I reserve the right to
dictate who can use this code and how (Just so you don't do something stupid
with it).
   Most Importantly, this code is swap ware.  If you use It send along your new
program (with code) or some other interesting tidbits you wrote, that I might be
interested in.
   This code is in beta, there are bugs!  I am not responsible for any damage
done to your computer, reputation, ego, dog, or family life due to the use of
this code.  All source is provided as is, I make no guaranties, and am not
responsible for anything you do with the code (legal or otherwise).
   Virtual Boy is a trademark of Nintendo, and V810 is a trademark of NEC.  I am
in no way affiliated with either party and all information contained hear was
found freely through public domain sources.
*/

//////////////////////////////////////////////////////////
// CPU routines

#include "mednafen/mednafen.h"
#include <mednafen/masmem.h>

//#include "pcfx.h"
//#include "debug.h"

#include <string.h>
#include <errno.h>

#include "v810_opt.h"
#include "v810_cpu.h"
#include "v810_cpuD.h"

//#include "fpu-new/softfloat.h"

V810::V810()
{
 #ifdef WANT_DEBUGGER
 CPUHook = NULL;
 ADDBT = NULL;
 #endif

 MemRead8 = NULL;
 MemRead16 = NULL;
 MemRead32 = NULL;

 IORead8 = NULL;
 IORead16 = NULL;
 IORead32 = NULL;

 MemWrite8 = NULL;
 MemWrite16 = NULL;
 MemWrite32 = NULL;

 IOWrite8 = NULL;
 IOWrite16 = NULL;
 IOWrite32 = NULL;

 memset(FastMap, 0, sizeof(FastMap));

 memset(MemReadBus32, 0, sizeof(MemReadBus32));
 memset(MemWriteBus32, 0, sizeof(MemWriteBus32));
}

V810::~V810()
{

}

INLINE void V810::RecalcIPendingCache(void)
{
 IPendingCache = 0;

 // Of course don't generate an interrupt if there's not one pending!
 if(ilevel < 0)
  return;

 // If CPU is halted because of a fatal exception, don't let an interrupt
 // take us out of this halted status.
 if(Halted == HALT_FATAL_EXCEPTION) 
  return;

 // If the NMI pending, exception pending, and/or interrupt disabled bit
 // is set, don't accept any interrupts.
 if(S_REG[PSW] & (PSW_NP | PSW_EP | PSW_ID))
  return;

 // If the interrupt level is lower than the interrupt enable level, don't
 // accept it.
 if(ilevel < (int)((S_REG[PSW] & PSW_IA) >> 16))
  return;

 IPendingCache = 0xFF;
}


// TODO: "An interrupt that occurs during restore/dump/clear operation is internally held and is accepted after the
// operation in progress is finished. The maskable interrupt is held internally only when the EP, NP, and ID flags
// of PSW are all 0."
//
// This behavior probably doesn't have any relevance on the PC-FX, unless we're sadistic
// and try to restore cache from an interrupt acknowledge register or dump it to a register
// controlling interrupt masks...  I wanna be sadistic~

void V810::CacheClear(v810_timestamp_t &timestamp, uint32 start, uint32 count)
{
 //printf("Cache clear: %08x %08x\n", start, count);
 for(uint32 i = 0; i < count && (i + start) < 128; i++)
  memset(&Cache[i + start], 0, sizeof(V810_CacheEntry_t));
}

INLINE void V810::CacheOpMemStore(v810_timestamp_t &timestamp, uint32 A, uint32 V)
{
 if(MemWriteBus32[A >> 24])
 {
  timestamp += 2;
  MemWrite32(timestamp, A, V);
 }
 else
 {
  timestamp += 2;
  MemWrite16(timestamp, A, V & 0xFFFF);

  timestamp += 2;
  MemWrite16(timestamp, A | 2, V >> 16);
 }
}

INLINE uint32 V810::CacheOpMemLoad(v810_timestamp_t &timestamp, uint32 A)
{
 if(MemReadBus32[A >> 24])
 {
  timestamp += 2;
  return(MemRead32(timestamp, A));
 }
 else
 {
  uint32 ret;

  timestamp += 2;
  ret = MemRead16(timestamp, A);

  timestamp += 2;
  ret |= MemRead16(timestamp, A | 2) << 16;
  return(ret);
 }
}

void V810::CacheDump(v810_timestamp_t &timestamp, const uint32 SA)
{
 printf("Cache dump: %08x\n", SA);

 for(int i = 0; i < 128; i++)
 {
  CacheOpMemStore(timestamp, SA + i * 8 + 0, Cache[i].data[0]);
  CacheOpMemStore(timestamp, SA + i * 8 + 4, Cache[i].data[1]);
 }

 for(int i = 0; i < 128; i++)
 {
  uint32 icht = Cache[i].tag | ((int)Cache[i].data_valid[0] << 22) | ((int)Cache[i].data_valid[1] << 23);

  CacheOpMemStore(timestamp, SA + 1024 + i * 4, icht);
 }

}

void V810::CacheRestore(v810_timestamp_t &timestamp, const uint32 SA)
{
 printf("Cache restore: %08x\n", SA);

 for(int i = 0; i < 128; i++)
 {
  Cache[i].data[0] = CacheOpMemLoad(timestamp, SA + i * 8 + 0);
  Cache[i].data[1] = CacheOpMemLoad(timestamp, SA + i * 8 + 4);
 }

 for(int i = 0; i < 128; i++)
 {
  uint32 icht;

  icht = CacheOpMemLoad(timestamp, SA + 1024 + i * 4);

  Cache[i].tag = icht & ((1 << 22) - 1);
  Cache[i].data_valid[0] = (icht >> 22) & 1;
  Cache[i].data_valid[1] = (icht >> 23) & 1;
 }
}


INLINE uint32 V810::RDCACHE(v810_timestamp_t &timestamp, uint32 addr)
{
 const int CI = (addr >> 3) & 0x7F;
 const int SBI = (addr & 4) >> 2;

 if(Cache[CI].tag == (addr >> 10))
 {
  if(!Cache[CI].data_valid[SBI])
  {
   timestamp += 2;       // or higher?  Penalty for cache miss seems to be higher than having cache disabled.
   if(MemReadBus32[addr >> 24])
    Cache[CI].data[SBI] = MemRead32(timestamp, addr & ~0x3);
   else
   {
    timestamp++;
    Cache[CI].data[SBI] = MemRead16(timestamp, addr & ~0x3) | ((MemRead16(timestamp, (addr & ~0x3) | 0x2) << 16));
   }
   Cache[CI].data_valid[SBI] = TRUE;
  }
 }
 else
 {
  Cache[CI].tag = addr >> 10;

  timestamp += 2;	// or higher?  Penalty for cache miss seems to be higher than having cache disabled.
  if(MemReadBus32[addr >> 24])
   Cache[CI].data[SBI] = MemRead32(timestamp, addr & ~0x3);
  else
  {
   timestamp++;
   Cache[CI].data[SBI] = MemRead16(timestamp, addr & ~0x3) | ((MemRead16(timestamp, (addr & ~0x3) | 0x2) << 16));
  }
  //Cache[CI].data[SBI] = MemRead32(timestamp, addr & ~0x3);
  Cache[CI].data_valid[SBI] = TRUE;
  Cache[CI].data_valid[SBI ^ 1] = FALSE;
 }

 //{
 // // Caution: This can mess up DRAM page change penalty timings
 // uint32 dummy_timestamp = 0;
 // if(Cache[CI].data[SBI] != mem_rword(addr & ~0x3, dummy_timestamp))
 // {
 //  printf("Cache/Real Memory Mismatch: %08x %08x/%08x\n", addr & ~0x3, Cache[CI].data[SBI], mem_rword(addr & ~0x3, dummy_timestamp));
 // }
 //}

 return(Cache[CI].data[SBI]);
}

INLINE uint16 V810::RDOP(v810_timestamp_t &timestamp, uint32 addr, uint32 meow)
{
 uint16 ret;

 if(S_REG[CHCW] & 0x2)
 {
  uint32 d32 = RDCACHE(timestamp, addr);
  ret = d32 >> ((addr & 2) * 8);
 }
 else
 {
  timestamp += meow; //++;
  ret = MemRead16(timestamp, addr);
 }
 return(ret);
}

#define BRANCH_ALIGN_CHECK(x)	{ if((S_REG[CHCW] & 0x2) && (x & 0x2)) { ADDCLOCK(1); } }

// Reinitialize the defaults in the CPU
void V810::Reset() 
{
#ifdef WANT_DEBUGGER
 if(ADDBT)
  ADDBT(GetPC(), 0xFFFFFFF0, 0xFFF0);
#endif
 v810_timestamp = 0;
 next_event_ts = 0x7FFFFFFF; // fixme

 memset(&Cache, 0, sizeof(Cache));

 memset(P_REG, 0, sizeof(P_REG));
 memset(S_REG, 0, sizeof(S_REG));
 memset(Cache, 0, sizeof(Cache));

 P_REG[0]      =  0x00000000;
 SetPC(0xFFFFFFF0);

 S_REG[ECR]    =  0x0000FFF0;
 S_REG[PSW]    =  0x00008000;

 if(VBMode)
  S_REG[PIR]	= 0x00005346;
 else
  S_REG[PIR]    =  0x00008100;

 S_REG[TKCW]   =  0x000000E0;
 Halted = HALT_NONE;
 ilevel = -1;

 lastop = 0;

 in_bstr = FALSE;

 RecalcIPendingCache();
}

bool V810::Init(V810_Emu_Mode mode, bool vb_mode)
{
 EmuMode = mode;
 VBMode = vb_mode;

 in_bstr = FALSE;
 in_bstr_to = 0;

 if(mode == V810_EMU_MODE_FAST)
 {
  memset(DummyRegion, 0, V810_FAST_MAP_PSIZE);

  for(unsigned int i = V810_FAST_MAP_PSIZE; i < V810_FAST_MAP_PSIZE + V810_FAST_MAP_TRAMPOLINE_SIZE; i += 2)
  {
   DummyRegion[i + 0] = 0;
   DummyRegion[i + 1] = 0x36 << 2;
  }

  for(uint64 A = 0; A < (1ULL << 32); A += V810_FAST_MAP_PSIZE)
   FastMap[A / V810_FAST_MAP_PSIZE] = DummyRegion - A;
 }

 return(TRUE);
}

void V810::Kill(void)
{
 for(unsigned int i = 0; i < FastMapAllocList.size(); i++)
  MDFN_free(FastMapAllocList[i]);

 FastMapAllocList.clear();
}

void V810::SetInt(int level)
{
 assert(level >= -1 && level <= 15);

 ilevel = level;
 RecalcIPendingCache();
}

uint8 *V810::SetFastMap(uint32 addresses[], uint32 length, unsigned int num_addresses, const char *name)
{
 uint8 *ret = NULL;

 for(unsigned int i = 0; i < num_addresses; i++)
 {
  assert((addresses[i] & (V810_FAST_MAP_PSIZE - 1)) == 0);
 }
 assert((length & (V810_FAST_MAP_PSIZE - 1)) == 0);

 if(!(ret = (uint8 *)MDFN_malloc(length + V810_FAST_MAP_TRAMPOLINE_SIZE, name)))
 {
  return(NULL);
 }

 for(unsigned int i = length; i < length + V810_FAST_MAP_TRAMPOLINE_SIZE; i += 2)
 {
  ret[i + 0] = 0;
  ret[i + 1] = 0x36 << 2;
 }

 for(unsigned int i = 0; i < num_addresses; i++)
 {  
  for(uint64 addr = addresses[i]; addr != (uint64)addresses[i] + length; addr += V810_FAST_MAP_PSIZE)
  {
   //printf("%08x, %d, %s\n", addr, length, name);

   FastMap[addr / V810_FAST_MAP_PSIZE] = ret - addresses[i];
  }
 }

 FastMapAllocList.push_back(ret);

 return(ret);
}


void V810::SetMemReadBus32(uint8 A, bool value)
{
 MemReadBus32[A] = value;
}

void V810::SetMemWriteBus32(uint8 A, bool value)
{
 MemWriteBus32[A] = value;
}

void V810::SetMemReadHandlers(uint8 MDFN_FASTCALL (*read8)(v810_timestamp_t &, uint32), uint16 MDFN_FASTCALL (*read16)(v810_timestamp_t &, uint32), uint32 MDFN_FASTCALL (*read32)(v810_timestamp_t &, uint32))
{
 MemRead8 = read8;
 MemRead16 = read16;
 MemRead32 = read32;
}

void V810::SetMemWriteHandlers(void MDFN_FASTCALL (*write8)(v810_timestamp_t &, uint32, uint8), void MDFN_FASTCALL (*write16)(v810_timestamp_t &, uint32, uint16), void MDFN_FASTCALL (*write32)(v810_timestamp_t &, uint32, uint32))
{
 MemWrite8 = write8;
 MemWrite16 = write16;
 MemWrite32 = write32;
}

void V810::SetIOReadHandlers(uint8 MDFN_FASTCALL (*read8)(v810_timestamp_t &, uint32), uint16 MDFN_FASTCALL (*read16)(v810_timestamp_t &, uint32), uint32 MDFN_FASTCALL (*read32)(v810_timestamp_t &, uint32))
{
 IORead8 = read8;
 IORead16 = read16;
 IORead32 = read32;
}

void V810::SetIOWriteHandlers(void MDFN_FASTCALL (*write8)(v810_timestamp_t &, uint32, uint8), void MDFN_FASTCALL (*write16)(v810_timestamp_t &, uint32, uint16), void MDFN_FASTCALL (*write32)(v810_timestamp_t &, uint32, uint32))
{
 IOWrite8 = write8;
 IOWrite16 = write16;
 IOWrite32 = write32;
}


INLINE void V810::SetFlag(uint32 n, bool condition)
{
 S_REG[PSW] &= ~n;

 if(condition)
  S_REG[PSW] |= n;
}
	
INLINE void V810::SetSZ(uint32 value)
{
 SetFlag(PSW_Z, !value);
 SetFlag(PSW_S, value & 0x80000000);
}

#ifdef WANT_DEBUGGER
void V810::CheckBreakpoints(void (*callback)(int type, uint32 address, unsigned int len), uint16 MDFN_FASTCALL (*peek16)(const v810_timestamp_t, uint32), uint32 MDFN_FASTCALL (*peek32)(const v810_timestamp_t, uint32))
{
 unsigned int opcode;
 uint16 tmpop;
 uint16 tmpop_high;
 int32 ws_dummy = v810_timestamp;
 uint32 tmp_PC = GetPC();

 tmpop      = peek16(ws_dummy, tmp_PC);
 tmpop_high = peek16(ws_dummy, tmp_PC + 2);

 opcode = tmpop >> 10;

 // Uncomment this out later if necessary.
 //if((tmpop & 0xE000) == 0x8000)        // Special opcode format for
 // opcode = (tmpop >> 9) & 0x7F;    // type III instructions.

 switch(opcode)
 {
	case CAXI: break;

	default: break;

	case LD_B: callback(BPOINT_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFF, 1); break;
	case LD_H: callback(BPOINT_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFE, 2); break;
	case LD_W: callback(BPOINT_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFC, 4); break;

	case ST_B: callback(BPOINT_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFF, 1); break;
	case ST_H: callback(BPOINT_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFE, 2); break;
	case ST_W: callback(BPOINT_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFC, 4); break;

	case IN_B: callback(BPOINT_IO_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFF, 1); break;
	case IN_H: callback(BPOINT_IO_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFE, 2); break;
	case IN_W: callback(BPOINT_IO_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFC, 4); break;

	case OUT_B: callback(BPOINT_IO_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFF, 1); break; 
	case OUT_H: callback(BPOINT_IO_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFE, 2); break;
	case OUT_W: callback(BPOINT_IO_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFC, 4); break;
 }

}
#endif

#define SetPREG(n, val) { P_REG[n] = val; }

INLINE void V810::SetSREG(v810_timestamp_t &timestamp, unsigned int which, uint32 value)
{
	switch(which)
	{
	 default:	// Reserved
		printf("LDSR to reserved system register: 0x%02x : 0x%08x\n", which, value);
		break;

         case ECR:      // Read-only
                break;

         case PIR:      // Read-only (obviously)
                break;

         case TKCW:     // Read-only
                break;

	 case EIPSW:
	 case FEPSW:
              	S_REG[which] = value & 0xFF3FF;
		break;

	 case PSW:
              	S_REG[which] = value & 0xFF3FF;
		RecalcIPendingCache();
		break;

	 case EIPC:
	 case FEPC:
		S_REG[which] = value & 0xFFFFFFFE;
		break;

	 case ADDTRE:
  	        S_REG[ADDTRE] = value & 0xFFFFFFFE;
        	printf("Address trap(unemulated): %08x\n", value);
		break;

	 case CHCW:
              	S_REG[CHCW] = value & 0x2;

              	switch(value & 0x31)
              	{
              	 default: printf("Undefined cache control bit combination: %08x\n", value);
                          break;

              	 case 0x00: break;

              	 case 0x01: CacheClear(timestamp, (value >> 20) & 0xFFF, (value >> 8) & 0xFFF);
                            break;

              	 case 0x10: CacheDump(timestamp, value & ~0xFF);
                            break;

              	 case 0x20: CacheRestore(timestamp, value & ~0xFF);
                            break;
               	}
		break;
	}
}

INLINE uint32 V810::GetSREG(unsigned int which)
{
	uint32 ret;

	if(which != 24 && which != 25 && which >= 8)
	{
	 printf("STSR from reserved system register: 0x%02x", which);
        }

	ret = S_REG[which];

	return(ret);
}

#define RB_SETPC(new_pc_raw) 										\
			  {										\
			   const uint32 new_pc = new_pc_raw;	/* So RB_SETPC(RB_GETPC()) won't mess up */	\
			   if(RB_AccurateMode)								\
			    PC = new_pc;								\
			   else										\
			   {										\
			    PC_ptr = &FastMap[(new_pc) >> V810_FAST_MAP_SHIFT][(new_pc)];		\
			    PC_base = PC_ptr - (new_pc);						\
			   }										\
			  }

#define RB_PCRELCHANGE(delta) { 				\
				if(RB_AccurateMode)		\
				 PC += (delta);			\
				else				\
				{				\
				 uint32 PC_tmp = RB_GETPC();	\
				 PC_tmp += (delta);		\
				 RB_SETPC(PC_tmp);		\
				}					\
			      }

#define RB_INCPCBY2()	{ if(RB_AccurateMode) PC += 2; else PC_ptr += 2; }
#define RB_INCPCBY4()   { if(RB_AccurateMode) PC += 4; else PC_ptr += 4; }

#define RB_DECPCBY2()   { if(RB_AccurateMode) PC -= 2; else PC_ptr -= 2; }
#define RB_DECPCBY4()   { if(RB_AccurateMode) PC -= 4; else PC_ptr -= 4; }


// Define accurate mode defines
#define RB_GETPC()      PC
#define RB_RDOP(PC_offset, ...) RDOP(timestamp, PC + PC_offset, ## __VA_ARGS__)


void V810::Run_Accurate(int32 MDFN_FASTCALL (*event_handler)(const v810_timestamp_t timestamp))
{
 const bool RB_AccurateMode = true;

 #define RB_ADDBT(n,o,p)
 #define RB_CPUHOOK(n)

 #include "v810_oploop.inc"

 #undef RB_CPUHOOK
 #undef RB_ADDBT
}

#ifdef WANT_DEBUGGER
void V810::Run_Accurate_Debug(int32 MDFN_FASTCALL (*event_handler)(const v810_timestamp_t timestamp))
{
 const bool RB_AccurateMode = true;

 #define RB_ADDBT(n,o,p) ADDBT(n,o,p)
 #define RB_CPUHOOK(n) {if(CPUHook) CPUHook(n); }
 #define RB_DEBUGMODE

 #include "v810_oploop.inc"

 #undef RB_DEBUGMODE
 #undef RB_CPUHOOK
 #undef RB_ADDBT
}
#endif

//
// Undefine accurate mode defines
//
#undef RB_GETPC
#undef RB_RDOP



//
// Define fast mode defines
//
#define RB_GETPC()      	((uint32)(PC_ptr - PC_base))

#define RB_RDOP(PC_offset, ...) LoadU16_LE((uint16 *)&PC_ptr[PC_offset])

void V810::Run_Fast(int32 MDFN_FASTCALL (*event_handler)(const v810_timestamp_t timestamp))
{
 const bool RB_AccurateMode = false;

 #define RB_ADDBT(n,o,p)
 #define RB_CPUHOOK(n)

 #include "v810_oploop.inc"

 #undef RB_CPUHOOK
 #undef RB_ADDBT
}

#ifdef WANT_DEBUGGER
void V810::Run_Fast_Debug(int32 MDFN_FASTCALL (*event_handler)(const v810_timestamp_t timestamp))
{
 const bool RB_AccurateMode = false;

 #define RB_ADDBT(n,o,p) ADDBT(n,o,p)
 #define RB_CPUHOOK(n) { if(CPUHook) CPUHook(n); }
 #define RB_DEBUGMODE

 #include "v810_oploop.inc"

 #undef RB_DEBUGMODE
 #undef RB_CPUHOOK
 #undef RB_ADDBT
}
#endif

//
// Undefine fast mode defines
//
#undef RB_GETPC
#undef RB_RDOP

v810_timestamp_t V810::Run(int32 MDFN_FASTCALL (*event_handler)(const v810_timestamp_t timestamp))
{
 Running = true;

 #ifdef WANT_DEBUGGER
 if(CPUHook)
 {
  if(EmuMode == V810_EMU_MODE_FAST)
   Run_Fast_Debug(event_handler);
  else
   Run_Accurate_Debug(event_handler);
 }
 else
 #endif
 {
  if(EmuMode == V810_EMU_MODE_FAST)
   Run_Fast(event_handler);
  else
   Run_Accurate(event_handler);
 }
 return(v810_timestamp);
}

void V810::Exit(void)
{
 Running = false;
}

#ifdef WANT_DEBUGGER
void V810::SetCPUHook(void (*newhook)(uint32 PC), void (*new_ADDBT)(uint32 old_PC, uint32 new_PC, uint32))
{
 CPUHook = newhook;
 ADDBT = new_ADDBT;
}
#endif

uint32 V810::GetPC(void)
{
 if(EmuMode == V810_EMU_MODE_ACCURATE)
  return(PC);
 else
 {
  return(PC_ptr - PC_base);
 }
}

void V810::SetPC(uint32 new_pc)
{
 if(EmuMode == V810_EMU_MODE_ACCURATE)
  PC = new_pc;
 else
 {
  PC_ptr = &FastMap[new_pc >> V810_FAST_MAP_SHIFT][new_pc];
  PC_base = PC_ptr - new_pc;
 }
}

uint32 V810::GetPR(const unsigned int which)
{
 assert(which <= 0x1F);


 return(which ? P_REG[which] : 0);
}

void V810::SetPR(const unsigned int which, uint32 value)
{
 assert(which <= 0x1F);

 if(which)
  P_REG[which] = value;
}

uint32 V810::GetSR(const unsigned int which)
{
 assert(which <= 0x1F);

 return(GetSREG(which));
}

void V810::SetSR(const unsigned int which, uint32 value)
{
 assert(which <= 0x1F);

// SetSREG(timestamp, which, value);
}


#define BSTR_OP_MOV dst_cache &= ~(1 << dstoff); dst_cache |= ((src_cache >> srcoff) & 1) << dstoff;
#define BSTR_OP_NOT dst_cache &= ~(1 << dstoff); dst_cache |= (((src_cache >> srcoff) & 1) ^ 1) << dstoff;

#define BSTR_OP_XOR dst_cache ^= ((src_cache >> srcoff) & 1) << dstoff;
#define BSTR_OP_OR dst_cache |= ((src_cache >> srcoff) & 1) << dstoff;
#define BSTR_OP_AND dst_cache &= ~((((src_cache >> srcoff) & 1) ^ 1) << dstoff);

#define BSTR_OP_XORN dst_cache ^= (((src_cache >> srcoff) & 1) ^ 1) << dstoff;
#define BSTR_OP_ORN dst_cache |= (((src_cache >> srcoff) & 1) ^ 1) << dstoff;
#define BSTR_OP_ANDN dst_cache &= ~(((src_cache >> srcoff) & 1) << dstoff);

INLINE uint32 V810::BSTR_RWORD(v810_timestamp_t &timestamp, uint32 A)
{
 if(MemReadBus32[A >> 24])
 {
  timestamp += 2;
  return(MemRead32(timestamp, A));
 }
 else
 {
  uint32 ret;

  timestamp += 2;
  ret = MemRead16(timestamp, A);
 
  timestamp += 2;
  ret |= MemRead16(timestamp, A | 2) << 16;
  return(ret);
 }
}

INLINE void V810::BSTR_WWORD(v810_timestamp_t &timestamp, uint32 A, uint32 V)
{
 if(MemWriteBus32[A >> 24])
 {
  timestamp += 2;
  MemWrite32(timestamp, A, V);
 }
 else
 {
  timestamp += 2;
  MemWrite16(timestamp, A, V & 0xFFFF);

  timestamp += 2;
  MemWrite16(timestamp, A | 2, V >> 16);
 }
}

#define DO_BSTR(op) { 						\
                while(len)					\
                {						\
                 if(!have_src_cache)                            \
                 {                                              \
		  have_src_cache = TRUE;			\
                  src_cache = BSTR_RWORD(timestamp, src);       \
                 }                                              \
								\
		 if(!have_dst_cache)				\
		 {						\
		  have_dst_cache = TRUE;			\
                  dst_cache = BSTR_RWORD(timestamp, dst);       \
                 }                                              \
								\
		 op;						\
                 srcoff = (srcoff + 1) & 0x1F;			\
                 dstoff = (dstoff + 1) & 0x1F;			\
		 len--;						\
								\
		 if(!srcoff)					\
		 {                                              \
		  src += 4;					\
		  have_src_cache = FALSE;			\
		 }                                              \
								\
                 if(!dstoff)                                    \
                 {                                              \
                  BSTR_WWORD(timestamp, dst, dst_cache);        \
                  dst += 4;                                     \
		  have_dst_cache = FALSE;			\
		  if(timestamp >= next_event_ts)		\
		   break;					\
                 }                                              \
                }						\
                if(have_dst_cache)				\
                 BSTR_WWORD(timestamp, dst, dst_cache);		\
		}

INLINE bool V810::Do_BSTR_Search(v810_timestamp_t &timestamp, const int inc_mul, unsigned int bit_test)
{
        uint32 srcoff = (P_REG[27] & 0x1F);
        uint32 len = P_REG[28];
        uint32 bits_skipped = P_REG[29];
        uint32 src = (P_REG[30] & 0xFFFFFFFC);
	bool found = false;

	#if 0
	// TODO: Better timing.
	if(!in_bstr)	// If we're just starting the execution of this instruction(kind of spaghetti-code), so FIXME if we change
			// bstr handling in v810_oploop.inc
	{
	 timestamp += 13 - 1;
	}
	#endif

	while(len)
	{
		if(!have_src_cache)
		{
		 have_src_cache = TRUE;
		 timestamp++;
		 src_cache = BSTR_RWORD(timestamp, src);
		}

		if(((src_cache >> srcoff) & 1) == bit_test)
		{
		 found = true;

		 /* Fix the bit offset and word address to "1 bit before" it was found */
		 srcoff -= inc_mul * 1;
		 if(srcoff & 0x20)		/* Handles 0x1F->0x20(0x00) and 0x00->0xFFFF... */
		 {
		  src -= inc_mul * 4;
		  srcoff &= 0x1F;
		 }
		 break;
		}
	        srcoff = (srcoff + inc_mul * 1) & 0x1F;
		bits_skipped++;
	        len--;

	        if(!srcoff)
		{
	         have_src_cache = FALSE;
		 src += inc_mul * 4;
		 if(timestamp >= next_event_ts)
		  break;
		}
	}

        P_REG[27] = srcoff;
        P_REG[28] = len;
        P_REG[29] = bits_skipped;
        P_REG[30] = src;


        if(found)               // Set Z flag to 0 if the bit was found
         SetFlag(PSW_Z, 0);
        else if(!len)           // ...and if the search is over, and the bit was not found, set it to 1
         SetFlag(PSW_Z, 1);

        if(found)               // Bit found, so don't continue the search.
         return(false);

        return((bool)len);      // Continue the search if any bits are left to search.
}

bool V810::bstr_subop(v810_timestamp_t &timestamp, int sub_op, int arg1)
{
 if((sub_op >= 0x10) || (!(sub_op & 0x8) && sub_op >= 0x4))
 {
  printf("%08x\tBSR Error: %04x\n", PC,sub_op);

  SetPC(GetPC() - 2);
  Exception(INVALID_OP_HANDLER_ADDR, ECODE_INVALID_OP);

  return(false);
 }

// printf("BSTR: %02x, %02x %02x; src: %08x, dst: %08x, len: %08x\n", sub_op, P_REG[27], P_REG[26], P_REG[30], P_REG[29], P_REG[28]);

 if(sub_op & 0x08)
 {
	uint32 dstoff = (P_REG[26] & 0x1F);
	uint32 srcoff = (P_REG[27] & 0x1F);
	uint32 len =     P_REG[28];
	uint32 dst =    (P_REG[29] & 0xFFFFFFFC);
	uint32 src =    (P_REG[30] & 0xFFFFFFFC);

	switch(sub_op)
	{
	 case ORBSU: DO_BSTR(BSTR_OP_OR); break;

	 case ANDBSU: DO_BSTR(BSTR_OP_AND); break;

	 case XORBSU: DO_BSTR(BSTR_OP_XOR); break;

	 case MOVBSU: DO_BSTR(BSTR_OP_MOV); break;

	 case ORNBSU: DO_BSTR(BSTR_OP_ORN); break;

	 case ANDNBSU: DO_BSTR(BSTR_OP_ANDN); break;

	 case XORNBSU: DO_BSTR(BSTR_OP_XORN); break;

	 case NOTBSU: DO_BSTR(BSTR_OP_NOT); break;
	}

        P_REG[26] = dstoff; 
        P_REG[27] = srcoff;
        P_REG[28] = len;
        P_REG[29] = dst;
        P_REG[30] = src;

	return((bool)P_REG[28]);
 }
 else
 {
  printf("BSTR Search: %02x\n", sub_op);
  return(Do_BSTR_Search(timestamp, ((sub_op & 1) ? -1 : 1), (sub_op & 0x2) >> 1));
 }
 assert(0);
 return(false);
}

INLINE void V810::SetFPUOPNonFPUFlags(uint32 result)
{
                 // Now, handle flag setting
                 SetFlag(PSW_OV, 0);

                 if(!(result & 0x7FFFFFFF)) // Check to see if exponent and mantissa are 0
		 {
		  // If Z flag is set, S and CY should be clear, even if it's negative 0(confirmed on real thing with subf.s, at least).
                  SetFlag(PSW_Z, 1);
                  SetFlag(PSW_S, 0);
                  SetFlag(PSW_CY, 0);
		 }
                 else
		 {
                  SetFlag(PSW_Z, 0);
                  SetFlag(PSW_S, result & 0x80000000);
                  SetFlag(PSW_CY, result & 0x80000000);
		 }
                 //printf("MEOW: %08x\n", S_REG[PSW] & (PSW_S | PSW_CY));
}

INLINE bool V810::CheckFPInputException(uint32 fpval)
{
 // Zero isn't a subnormal! (OR IS IT *DUN DUN DUNNN* ;b)
 if(!(fpval & 0x7FFFFFFF))
  return(false);

 switch((fpval >> 23) & 0xFF)
 {
  case 0x00: // Subnormal		
  case 0xFF: // NaN or infinity
	{
	 //puts("New FPU FRO");

	 S_REG[PSW] |= PSW_FRO;

	 SetPC(GetPC() - 4);
	 Exception(FPU_HANDLER_ADDR, ECODE_FRO);
	}
	return(true);	// Yes, exception occurred
 }
 return(false);	// No, no exception occurred.
}

bool V810::FPU_DoesExceptionKillResult(void)
{
 if(float_exception_flags & float_flag_invalid)
  return(true);

 if(float_exception_flags & float_flag_divbyzero)
  return(true);


 // Return false here, so that the result of this calculation IS put in the output register.
 // (Incidentally, to get the result of operations on overflow to match a real V810, required a bit of hacking of the SoftFloat code to "wrap" the exponent
 // on overflow,
 // rather than generating an infinity.  The wrapping behavior is specified in IEE 754 AFAIK, and is useful in cases where you divide a huge number
 // by another huge number, and fix the result afterwards based on the number of overflows that occurred.  Probably requires some custom assembly code,
 // though.  And it's the kind of thing you'd see in an engineering or physics program, not in a perverted video game :b).
 // Oh, and just a note to self, FPR is NOT set when an overflow occurs.  Or it is in certain cases?
 if(float_exception_flags & float_flag_overflow)
  return(false);

 return(false);
}

void V810::FPU_DoException(void)
{
 if(float_exception_flags & float_flag_invalid)
 {
  //puts("New FPU Invalid");

  S_REG[PSW] |= PSW_FIV;

  SetPC(GetPC() - 4);
  Exception(FPU_HANDLER_ADDR, ECODE_FIV);

  return;
 }

 if(float_exception_flags & float_flag_divbyzero)
 {
  //puts("New FPU Divide by Zero");

  S_REG[PSW] |= PSW_FZD;

  SetPC(GetPC() - 4);
  Exception(FPU_HANDLER_ADDR, ECODE_FZD);

  return;
 }

 if(float_exception_flags & float_flag_underflow)
 {
  //puts("New FPU Underflow");

  S_REG[PSW] |= PSW_FUD;
 }

 if(float_exception_flags & float_flag_inexact)
 {
  S_REG[PSW] |= PSW_FPR;
  //puts("New FPU Precision Degradation");
 }

 // FPR can be set along with overflow, so put the overflow exception handling at the end here(for Exception() messes with PSW).
 if(float_exception_flags & float_flag_overflow)
 {
  //puts("New FPU Overflow");

  S_REG[PSW] |= PSW_FOV;

  SetPC(GetPC() - 4);
  Exception(FPU_HANDLER_ADDR, ECODE_FOV);
 }
}

bool V810::IsSubnormal(uint32 fpval)
{
 if( ((fpval >> 23) & 0xFF) == 0 && (fpval & ((1 << 23) - 1)) )
  return(true);

 return(false);
}

INLINE void V810::FPU_Math_Template(float32 (*func)(float32, float32), uint32 arg1, uint32 arg2)
{
 if(CheckFPInputException(P_REG[arg1]) || CheckFPInputException(P_REG[arg2]))
 {

 }
 else
 {
  uint32 result;

  float_exception_flags = 0;
  result = func(P_REG[arg1], P_REG[arg2]);

  if(IsSubnormal(result))
  {
   float_exception_flags |= float_flag_underflow;
   float_exception_flags |= float_flag_inexact;
  }

  //printf("Result: %08x, %02x; %02x\n", result, (result >> 23) & 0xFF, float_exception_flags);

  if(!FPU_DoesExceptionKillResult())
  {
   // Force it to +/- zero before setting S/Z based off of it(confirmed with subf.s on real V810, at least).
   if(float_exception_flags & float_flag_underflow)
    result &= 0x80000000;

   SetFPUOPNonFPUFlags(result);
   SetPREG(arg1, result);
  }
  FPU_DoException();
 }
}

void V810::fpu_subop(v810_timestamp_t &timestamp, int sub_op, int arg1, int arg2)
{
 //printf("FPU: %02x\n", sub_op);
 if(VBMode)
 {
  switch(sub_op)
  {
   case XB: timestamp++;	// Unknown
	    P_REG[arg1] = (P_REG[arg1] & 0xFFFF0000) | ((P_REG[arg1] & 0xFF) << 8) | ((P_REG[arg1] & 0xFF00) >> 8);
	    return;

   case XH: timestamp++;	// Unknown
	    P_REG[arg1] = (P_REG[arg1] << 16) | (P_REG[arg1] >> 16);
	    return;

   // Does REV use arg1 or arg2 for the source register?
   case REV: timestamp++;	// Unknown
		printf("Revvie bits\n");
	     {
	      // Public-domain code snippet from: http://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
      	      uint32 v = P_REG[arg2]; // 32-bit word to reverse bit order

	      // swap odd and even bits
	      v = ((v >> 1) & 0x55555555) | ((v & 0x55555555) << 1);
	      // swap consecutive pairs
	      v = ((v >> 2) & 0x33333333) | ((v & 0x33333333) << 2);
	      // swap nibbles ... 
	      v = ((v >> 4) & 0x0F0F0F0F) | ((v & 0x0F0F0F0F) << 4);
	      // swap bytes
	      v = ((v >> 8) & 0x00FF00FF) | ((v & 0x00FF00FF) << 8);
	      // swap 2-byte long pairs
	      v = ( v >> 16             ) | ( v               << 16);

	      P_REG[arg1] = v;
	     }
	     return;

   case MPYHW: timestamp += 9 - 1;	// Unknown?
	       P_REG[arg1] = (int32)(int16)(P_REG[arg1] & 0xFFFF) * (int32)(int16)(P_REG[arg2] & 0xFFFF);
	       return;
  }
 }

 switch(sub_op) 
 {
        // Virtual-Boy specific(probably!)
	default:
		{
		 SetPC(GetPC() - 4);
                 Exception(INVALID_OP_HANDLER_ADDR, ECODE_INVALID_OP);
		}
		break;

	case CVT_WS: 
		timestamp += 5;
		{
		 uint32 result;

                 float_exception_flags = 0;
		 result = int32_to_float32((int32)P_REG[arg2]);

		 if(!FPU_DoesExceptionKillResult())
		 {
		  SetPREG(arg1, result);
		  SetFPUOPNonFPUFlags(result);
		 }
		 else
		 {
		  puts("Exception on CVT.WS?????");	// This shouldn't happen, but just in case there's a bug...
		 }
		 FPU_DoException();
		}
		break;	// End CVT.WS

	case CVT_SW:
		timestamp += 8;
                if(CheckFPInputException(P_REG[arg2]))
                {

                }
		else
		{
		 int32 result;

                 float_exception_flags = 0;
		 result = float32_to_int32(P_REG[arg2]);

		 if(!FPU_DoesExceptionKillResult())
		 {
		  SetPREG(arg1, result);
                  SetFlag(PSW_OV, 0);
                  SetSZ(result);
		 }
		 FPU_DoException();
		}
		break;	// End CVT.SW

	case ADDF_S: timestamp += 8;
		     FPU_Math_Template(float32_add, arg1, arg2);
		     break;

	case SUBF_S: timestamp += 11;
		     FPU_Math_Template(float32_sub, arg1, arg2);
		     break;

        case CMPF_S: timestamp += 6;
		     // Don't handle this like subf.s because the flags
		     // have slightly different semantics(mostly regarding underflow/subnormal results) (confirmed on real V810).
                     if(CheckFPInputException(P_REG[arg1]) || CheckFPInputException(P_REG[arg2]))
                     {

                     }
		     else
		     {
		      SetFlag(PSW_OV, 0);

		      if(float32_eq(P_REG[arg1], P_REG[arg2]))
		      {
		       SetFlag(PSW_Z, 1);
		       SetFlag(PSW_S, 0);
		       SetFlag(PSW_CY, 0);
		      }
		      else
		      {
		       SetFlag(PSW_Z, 0);

		       if(float32_lt(P_REG[arg1], P_REG[arg2]))
		       {
		        SetFlag(PSW_S, 1);
		        SetFlag(PSW_CY, 1);
		       }
		       else
		       {
		        SetFlag(PSW_S, 0);
		        SetFlag(PSW_CY, 0);
                       }
		      }
		     }	// else of if(CheckFP...
                     break;

	case MULF_S: timestamp += 7;
		     FPU_Math_Template(float32_mul, arg1, arg2);
		     break;

	case DIVF_S: timestamp += 43;
		     FPU_Math_Template(float32_div, arg1, arg2);
		     break;

	case TRNC_SW:
                timestamp += 7;

		if(CheckFPInputException(P_REG[arg2]))
		{

		}
		else
                {
                 int32 result;

		 float_exception_flags = 0;
                 result = float32_to_int32_round_to_zero(P_REG[arg2]);

                 if(!FPU_DoesExceptionKillResult())
                 {
                  SetPREG(arg1, result);
		  SetFlag(PSW_OV, 0);
		  SetSZ(result);
                 }
		 FPU_DoException();
                }
                break;	// end TRNC.SW
	}
}

// Generate exception
void V810::Exception(uint32 handler, uint16 eCode) 
{
 // Exception overhead is unknown.

#ifdef WANT_DEBUGGER
 if(ADDBT)
 {
  uint32 old_PC = GetPC();

  if((eCode & 0xFFE0) == 0xFFA0) // Trap instruction(PC is pointing to next instruction at this point)
   old_PC -= 2;

  ADDBT(old_PC, handler, eCode);
 }
#endif

    printf("Exception: %08x %04x\n", handler, eCode);

    // Invalidate our bitstring state(forces the instruction to be re-read, and the r/w buffers reloaded).
    in_bstr = FALSE;
    have_src_cache = FALSE;
    have_dst_cache = FALSE;

    if(S_REG[PSW] & PSW_NP) // Fatal exception
    {
     printf("Fatal exception; Code: %08x, ECR: %08x, PSW: %08x, PC: %08x\n", eCode, S_REG[ECR], S_REG[PSW], PC);
     Halted = HALT_FATAL_EXCEPTION;
     IPendingCache = 0;
     return;
    }
    else if(S_REG[PSW] & PSW_EP)  //Double Exception
    {
     S_REG[FEPC] = GetPC();
     S_REG[FEPSW] = S_REG[PSW];

     S_REG[ECR] = (S_REG[ECR] & 0xFFFF) | (eCode << 16);
     S_REG[PSW] |= PSW_NP;
     S_REG[PSW] |= PSW_ID;
     S_REG[PSW] &= ~PSW_AE;

     SetPC(0xFFFFFFD0);
     IPendingCache = 0;
     return;
    }
    else 	// Regular exception
    {
     S_REG[EIPC] = GetPC();
     S_REG[EIPSW] = S_REG[PSW];
     S_REG[ECR] = (S_REG[ECR] & 0xFFFF0000) | eCode;
     S_REG[PSW] |= PSW_EP;
     S_REG[PSW] |= PSW_ID;
     S_REG[PSW] &= ~PSW_AE;

     SetPC(handler);
     IPendingCache = 0;
     return;
    }
}

int V810::StateAction(StateMem *sm, int load, int data_only)
{
 uint32 *cache_tag_temp = NULL;
 uint32 *cache_data_temp = NULL;
 bool *cache_data_valid_temp = NULL;
 uint32 PC_tmp = GetPC();

 if(EmuMode == V810_EMU_MODE_ACCURATE)
 {
  cache_tag_temp = (uint32 *)malloc(sizeof(uint32 *) * 128);
  cache_data_temp = (uint32 *)malloc(sizeof(uint32 *) * 128 * 2);
  cache_data_valid_temp = (bool *)malloc(sizeof(bool *) * 128 * 2);

  if(!cache_tag_temp || !cache_data_temp || !cache_data_valid_temp)
  {
   if(cache_tag_temp)
    free(cache_tag_temp);

   if(cache_data_temp)
    free(cache_data_temp);

   if(cache_data_valid_temp)
    free(cache_data_valid_temp);

   return(0);
  }
  if(!load)
  {
   for(int i = 0; i < 128; i++)
   {
    cache_tag_temp[i] = Cache[i].tag;

    cache_data_temp[i * 2 + 0] = Cache[i].data[0];
    cache_data_temp[i * 2 + 1] = Cache[i].data[1];

    cache_data_valid_temp[i * 2 + 0] = Cache[i].data_valid[0];
    cache_data_valid_temp[i * 2 + 1] = Cache[i].data_valid[1];
   }
  }
  else // If we're loading, go ahead and clear the cache temporaries,
       // in case the save state was saved while in fast mode
       // and the cache data isn't present and thus won't be loaded.
  {
   memset(cache_tag_temp, 0, sizeof(uint32) * 128);
   memset(cache_data_temp, 0, sizeof(uint32) * 128 * 2);
   memset(cache_data_valid_temp, 0, sizeof(bool) * 128 * 2);
  }
 }

 SFORMAT StateRegs[] =
 {
  SFARRAY32(P_REG, 32),
  SFARRAY32(S_REG, 32),
  SFVARN(PC_tmp, "PC"),
  SFVAR(Halted),

  SFVAR(lastop),

  SFARRAY32(cache_tag_temp, 128),
  SFARRAY32(cache_data_temp, 128 * 2),
  SFARRAYB(cache_data_valid_temp, 128 * 2),

  SFVAR(ilevel),	// Perhaps remove in future?
  SFVAR(next_event_ts),	// This too

  //SFVAR(tmp_timestamp),
  SFVAR(v810_timestamp),

  // Bitstring stuff:
  SFVAR(src_cache),
  SFVAR(dst_cache),
  SFVAR(have_src_cache),
  SFVAR(have_dst_cache),
  SFVAR(in_bstr),
  SFVAR(in_bstr_to),

  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "V810");

 if(load)
 {
  //clamp(&PCFX_V810.v810_timestamp, 0, 30 * 1000 * 1000);
  RecalcIPendingCache();

  SetPC(PC_tmp);
  if(EmuMode == V810_EMU_MODE_ACCURATE)
  {
   for(int i = 0; i < 128; i++)
   {
    Cache[i].tag = cache_tag_temp[i];

    Cache[i].data[0] = cache_data_temp[i * 2 + 0];
    Cache[i].data[1] = cache_data_temp[i * 2 + 1];

    Cache[i].data_valid[0] = cache_data_valid_temp[i * 2 + 0];
    Cache[i].data_valid[1] = cache_data_valid_temp[i * 2 + 1];

    //printf("%d %08x %08x %08x %d %d\n", i, Cache[i].tag << 10, Cache[i].data[0], Cache[i].data[1], Cache[i].data_valid[0], Cache[i].data_valid[1]);
   }
  }
 }

 if(EmuMode == V810_EMU_MODE_ACCURATE)
 {
  free(cache_tag_temp);
  free(cache_data_temp);
  free(cache_data_valid_temp);
 }

 return(ret);
}
