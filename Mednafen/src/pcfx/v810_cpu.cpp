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

//////////////////////////////////////////////////////////
// CPU routines

#include "pcfx.h"
#include "interrupt.h"
#include "debug.h"

#include "input.h"
#include "timer.h"
#include "../cdrom/scsicd.h"
#include "soundbox.h"
#include "king.h"

#include <string.h>

#include "v810_opt.h"
#include "v810_cpu.h"
#include "v810_cpuD.h"

#include "fpu/soft-fp.h"
#include "fpu/single.h"

#ifdef WANT_PCFX_MMAP
static uint8 *v810_mmap_base = NULL;
#endif

#if 0
static uint64 op_usage[128];
#endif

uint32 P_REG[32];  // Program registers pr0-pr31
uint32 S_REG[32];  // System registers sr0-sr31
static uint32 PC;  // Program Counter
static void (*CPUHook)(uint32 PC) = NULL;
static int lastop, lastclock;

enum
{
 HALT_NONE = 0,
 HALT_HALT = 1,
 HALT_FATAL_EXCEPTION = 2
};

static uint8 Halted;

uint32 v810_timestamp;
static int32 CycleCounter;
static int ilevel = 0;

static int bstr_subop(int sub_op, int arg1);
static int fpu_subop(int sub_op, int arg1, int arg2);
static void v810_exp(uint32 handler, uint16 eCode);

// Reinitialize the defaults in the CPU
void v810_reset() 
{
    memset(P_REG, 0, sizeof(P_REG));
    memset(S_REG, 0, sizeof(S_REG));

    P_REG[0]      =  0x00000000;
    PC		  =  0xFFFFFFF0;
    S_REG[ECR]    =  0x0000FFF0;
    S_REG[PSW]    =  0x00008000;
    S_REG[PIR]    =  0x00005346;
    S_REG[TKCW]   =  0x000000E0;
    Halted = HALT_NONE;
    ilevel = 0;
    lastop = 0;
    lastclock = 0;
}

void v810_kill(void)
{
 #if 0
 for(int i = 0; i < 80; i++)
 {
  printf("%016lld, %02x\n", op_usage[i], i);
 }
 #endif
}

void v810_setint(int level)
{
 ilevel = level;
}

#define SetFlag(n, condition) { S_REG[PSW] &= ~n; if(condition) S_REG[PSW] |= n; } 
	
static ALWAYS_INLINE void SetSZ(uint32 value)
{
 SetFlag(PSW_Z, !value);
 SetFlag(PSW_S, value & 0x80000000);
}

int32 event_pad = 0;
int32 event_timer = 0;
int32 event_scsi = 0;
int32 event_adpcm = 0;
int32 event_king = 0;

void v810_setevent(int type, int32 thetime)
{
 if(thetime == V810_EVENT_NONONO)
  thetime = 0;

 if(type == V810_EVENT_PAD)
  event_pad = thetime;
 else if(type == V810_EVENT_TIMER)
  event_timer = thetime;
 else if(type == V810_EVENT_SCSI)
  event_scsi = thetime;
 else if(type == V810_EVENT_ADPCM)
  event_adpcm = thetime;
 else if(type == V810_EVENT_KING)
  event_king = thetime;

 //MDFN_DispMessage("%8d %8d %8d %8d", event_pad, event_timer, event_scsi, event_adpcm);
}

#ifdef WANT_DEBUGGER
static ALWAYS_INLINE void CheckBreakpoints(void)
{
 unsigned int opcode;
 uint16 tmpop;
 uint16 tmpop_high;

 tmpop      = mem_rhword(PC);
 tmpop_high = mem_rhword(PC + 2);

 opcode = tmpop >> 10;

 // Uncomment this out later if necessary.
 //if((tmpop & 0xE000) == 0x8000)        // Special opcode format for
 // opcode = (tmpop >> 9) & 0x7F;    // type III instructions.

 switch(opcode)
 {
	case CAXI: break;

	default: break;

	case LD_B: PCFXDBG_CheckBP(BPOINT_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFF, 1); break;
	case LD_H: PCFXDBG_CheckBP(BPOINT_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFE, 2); break;
	case LD_W: PCFXDBG_CheckBP(BPOINT_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFC, 4); break;

	case ST_B: PCFXDBG_CheckBP(BPOINT_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFF, 1); break;
	case ST_H: PCFXDBG_CheckBP(BPOINT_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFE, 2); break;
	case ST_W: PCFXDBG_CheckBP(BPOINT_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFC, 4); break;

	case IN_B: PCFXDBG_CheckBP(BPOINT_IO_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFF, 1); break;
	case IN_H: PCFXDBG_CheckBP(BPOINT_IO_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFE, 2); break;
	case IN_W: PCFXDBG_CheckBP(BPOINT_IO_READ, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFC, 4); break;

	case OUT_B: PCFXDBG_CheckBP(BPOINT_IO_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFF, 1); break; 
	case OUT_H: PCFXDBG_CheckBP(BPOINT_IO_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFE, 2); break;
	case OUT_W: PCFXDBG_CheckBP(BPOINT_IO_WRITE, (sign_16(tmpop_high)+P_REG[tmpop & 0x1F])&0xFFFFFFFC, 4); break;
 }

}
#endif

#define SetPREG(n, val) { P_REG[n] = val; }

#define ADDBT(x)
#define CPUHOOKTEST
#define CHECKBPS()

#ifdef WANT_PCFX_MMAP
#define RDOP(addr) (*(uint16 *)&v810_mmap_base[addr])
static void v810_run_mmap(int32 RunCycles)
{
	#include "v810_eskimo.h"
}
#undef RDOP
#endif

#define RDOP(addr) mem_rhword(addr)
static void v810_run_normal(int32 RunCycles)
{
        #include "v810_eskimo.h"
}


#undef ADDBT
#undef CPUHOOKTEST
#undef CHECKBPS

void (*v810_run)(int32) = v810_run_normal;


#define ADDBT(x) PCFXDBG_AddBranchTrace(x)
#define CPUHOOKTEST { if(CPUHook) CPUHook(PC); }
#define CHECKBPS() CheckBreakpoints()

#ifdef WANT_DEBUGGER
static void v810_run_debug(int32 RunCycles)
{
        #include "v810_eskimo.h"
}

void v810_setCPUHook(void (*newhook)(uint32 PC))
{
 CPUHook = newhook;
 v810_run = newhook ? v810_run_debug : v810_run_normal;

 #ifdef WANT_PCFX_MMAP
 if(v810_run == v810_run_normal && v810_mmap_base)
  v810_run = v810_run_mmap;
 #endif
}
#endif

#ifdef WANT_PCFX_MMAP 
void v810_set_mmap_base(uint8 *ptr)
{
 v810_mmap_base = ptr;
 if(v810_run == v810_run_normal && v810_mmap_base)
  v810_run = v810_run_mmap;
}
#endif

uint32 v810_getPC(void)
{
 return(PC);
}

void v810_setPC(uint32 newPC)
{
 PC = newPC;
 #ifdef WANT_DEBUGGER 
 ADDBT(PC);
 #endif
}

#define BSTR_OP_MOV dst_cache &= ~(1 << dstoff); dst_cache |= ((src_cache >> srcoff) & 1) << dstoff;
#define BSTR_OP_NOT dst_cache &= ~(1 << dstoff); dst_cache |= (((src_cache >> srcoff) & 1) ^ 1) << dstoff;

#define BSTR_OP_XOR dst_cache ^= ((src_cache >> srcoff) & 1) << dstoff;
#define BSTR_OP_OR dst_cache |= ((src_cache >> srcoff) & 1) << dstoff;
#define BSTR_OP_AND dst_cache &= ~((((src_cache >> srcoff) & 1) ^ 1) << dstoff);

#define BSTR_OP_XORN dst_cache ^= (((src_cache >> srcoff) & 1) ^ 1) << dstoff;
#define BSTR_OP_ORN dst_cache |= (((src_cache >> srcoff) & 1) ^ 1) << dstoff;
#define BSTR_OP_ANDN dst_cache &= ~(((src_cache >> srcoff) & 1) << dstoff);


#define DO_BSTR(op) { 	\
		uint32 src_cache;	\
                uint32 dst_cache;	\
                src_cache = mem_rword(src);	\
                dst_cache = mem_rword(dst);	\
                while(len--)	\
                {	\
		 op;	\
                 srcoff = (srcoff + 1) & 0x1F;	\
                 dstoff = (dstoff + 1) & 0x1F;	\
                 if(!srcoff)	\
                 {	\
                  src+=4;       \
                  src_cache = mem_rword(src);	\
                 }	\
                 if(!dstoff)	\
                 {	\
                  mem_wword(dst, dst_cache);	\
                  dst+=4;	\
                  dst_cache = mem_rword(dst);	\
                 }	\
                }	\
                if(dstoff)	\
                 mem_wword(dst, dst_cache);	\
		}


static int bstr_subop(int sub_op, int arg1) 
{
	uint32 dstoff = (P_REG[26] & 0x1F);
	uint32 srcoff = (P_REG[27] & 0x1F);
	uint32 len =     P_REG[28];
	uint32 dst =    (P_REG[29] & 0xFFFFFFFC);
	uint32 src =    (P_REG[30] & 0xFFFFFFFC);


    if(sub_op > 15) {
        printf("%08x\tBSR Error: %04x\n", PC,sub_op);
		return 0;
    }

    //printf("BSTR: %02x, %02x %02x; src: %08x, dst: %08x, len: %08x\n", sub_op, srcoff, dstoff, src, dst, len);

    switch(sub_op) {
	case SCH0BSU:
		printf("\nSCH0BSU, len: %08X, src: %08X, srcoff: %08X, dst: %08X, dstoff: %08X\n",len,src,srcoff,dst,dstoff);
		break;

	case SCH0BSD:
		printf("\nSCH0BSD, len: %08X, src: %08X, srcoff: %08X, dst: %08X, dstoff: %08X\n",len,src,srcoff,dst,dstoff);
		break;

	case SCH1BSU:
		printf("\nSCH1BSU, len: %08X, src: %08X, srcoff: %08X, dst: %08X, dstoff: %08X\n",len,src,srcoff,dst,dstoff);
		break;

	case SCH1BSD:
		printf("\nSCH1BSD, len: %08X, src: %08X, srcoff: %08X, dst: %08X, dstoff: %08X\n",len,src,srcoff,dst,dstoff);
		break;

	case ORBSU: DO_BSTR(BSTR_OP_OR); break;

	case ANDBSU: DO_BSTR(BSTR_OP_AND); break;

	case XORBSU: DO_BSTR(BSTR_OP_XOR); break;

	//case MOVBSU: while(len-=8) { mem_wbyte(dst + (dstoff >> 3),mem_rbyte(src + (srcoff >> 3))); dst+=1; src+=1; }; /*DO_BSTR(BSTR_OP_MOV);*/ break;

	case MOVBSU: DO_BSTR(BSTR_OP_MOV); break;

	case ORNBSU: DO_BSTR(BSTR_OP_ORN); break;

	case ANDNBSU: DO_BSTR(BSTR_OP_ANDN); break;

	case XORNBSU: DO_BSTR(BSTR_OP_XORN); break;

	case NOTBSU: DO_BSTR(BSTR_OP_NOT); break;

	default:
	        printf("%08x\tBSR Error: %04x\n", PC,sub_op);
		break;
	}

        P_REG[26] = dstoff; 
        P_REG[27] = srcoff;
        P_REG[28] = len;
        P_REG[29] = dst;
        P_REG[30] = src;

	return 0;
}

static INLINE void SetFPUOPNonFPUFlags(uint32 result)
{
                 // Now, handle flag setting
                 SetFlag(PSW_OV, 0);
                 if(!(result & 0x7FFFFFFF)) // Check to see if exponent and mantissa are 0
                 {
                  SetFlag(PSW_Z, 1);
                  SetFlag(PSW_CY, 1);
                 }
                 else
                 {
                  SetFlag(PSW_Z, 0);
                  SetFlag(PSW_CY, 0);
                 }
                 SetFlag(PSW_S, result & 0x80000000);
}

// Returns TRUE if other flags and register should not be set
// FIXME:  what exceptions to generate on denorm and NaN?
static INLINE bool CheckExceptions(uint32 result, uint32 fex)
{
 bool ret = FALSE;

 if(fex & FP_EX_DENORM)
 {
  S_REG[PSW] |= PSW_FRO;   
  ret = TRUE;

  //puts("FPU DENORM");
 }

 if(fex & FP_EX_INVALID)
 {
  // See if it's NaN first
  if((result & (0xFF << 23)) == (0xFF << 23))
  {
   S_REG[PSW] |= PSW_FRO;
   puts("FPU NAN");
  }
  else
  {
   S_REG[PSW] |= PSW_FIV;
   puts("FPU Invalid");
  }
  ret = TRUE;
 }

 if(fex & FP_EX_OVERFLOW)
 {
  S_REG[PSW] |= PSW_FOV;

  PC -= 4;
  v810_exp(FPU_HANDLER_ADDR, ECODE_FOV);

  return(TRUE);
 }

 if(fex & FP_EX_DIVZERO)
 {
  S_REG[PSW] |= PSW_FZD;

  PC -= 4;
  v810_exp(FPU_HANDLER_ADDR, ECODE_FZD);

  return(TRUE);
 }

 if(fex & FP_EX_UNDERFLOW)
 {
  S_REG[PSW] |= PSW_FUD;
  //puts("FPU Precision");
 }

 if(fex & FP_EX_INEXACT)
 {
  S_REG[PSW] |= PSW_FPR;  
  //puts("FPU Precision Degradation");
 }
 return(ret);
}

static INLINE uint32 MULF_S_Func(uint32 arg1, uint32 arg2)
{
 FP_DECL_EX;
 FP_DECL_S(SA); FP_DECL_S(SB); FP_DECL_S(SR);

 FP_INIT_ROUNDMODE;

 uint32 result = 0;

 FP_UNPACK_S(SA, P_REG[arg1]);
 FP_UNPACK_S(SB, P_REG[arg2]);

 FP_MUL_S(SR, SA, SB);

 FP_PACK_S(result, SR);

 if(!CheckExceptions(result, _fex))
 {
  SetPREG(arg1, result);
  SetFPUOPNonFPUFlags(result);
 }

 return(7);
}

static INLINE uint32 DIVF_S_Func(uint32 arg1, uint32 arg2)
{
 FP_DECL_EX;
 FP_DECL_S(SA); FP_DECL_S(SB); FP_DECL_S(SR);

 FP_INIT_ROUNDMODE;

 uint32 result = 0;

 FP_UNPACK_S(SA, P_REG[arg1]);
 FP_UNPACK_S(SB, P_REG[arg2]);

 FP_DIV_S(SR, SA, SB);

 FP_PACK_S(result, SR);

 if(!CheckExceptions(result, _fex))
 {
  SetPREG(arg1, result);
  SetFPUOPNonFPUFlags(result);
 }

 return(43);
}


static INLINE uint32 ADDF_S_Func(uint32 arg1, uint32 arg2)
{
 FP_DECL_EX;
 FP_DECL_S(SA); FP_DECL_S(SB); FP_DECL_S(SR);

 FP_INIT_ROUNDMODE;

 uint32 result = 0;

 FP_UNPACK_SEMIRAW_S(SA, P_REG[arg1]);
 FP_UNPACK_SEMIRAW_S(SB, P_REG[arg2]);

 FP_ADD_S(SR, SA, SB);

 FP_PACK_SEMIRAW_S(result, SR);

 if(!CheckExceptions(result, _fex))
 {
  SetPREG(arg1, result);
  SetFPUOPNonFPUFlags(result);
 }

 return(8);
}

static INLINE uint32 SUBF_S_Func(uint32 arg1, uint32 arg2)
{
 FP_DECL_EX;
 FP_DECL_S(SA); FP_DECL_S(SB); FP_DECL_S(SR);

 FP_INIT_ROUNDMODE;

 uint32 result = 0;

 FP_UNPACK_SEMIRAW_S(SA, P_REG[arg1]);
 FP_UNPACK_SEMIRAW_S(SB, P_REG[arg2]);

 FP_SUB_S(SR, SA, SB);

 FP_PACK_SEMIRAW_S(result, SR);

 if(!CheckExceptions(result, _fex))
 {
  SetPREG(arg1, result);
  SetFPUOPNonFPUFlags(result);
 }

 return(11);
}

static INLINE uint32 CMPF_S_Func(uint32 arg1, uint32 arg2)
{
 FP_DECL_EX;
 FP_DECL_S(SA); FP_DECL_S(SB); FP_DECL_S(SR);

 FP_INIT_ROUNDMODE;

 uint32 result = 0;

 FP_UNPACK_SEMIRAW_S(SA, P_REG[arg1]);
 FP_UNPACK_SEMIRAW_S(SB, P_REG[arg2]);

 FP_SUB_S(SR, SA, SB);

 FP_PACK_SEMIRAW_S(result, SR);

 if(!CheckExceptions(result, _fex))
  SetFPUOPNonFPUFlags(result);

 return(6);
}

static int fpu_subop(int sub_op, int arg1, int arg2) 
{
 int extra_clocks = 0;

 switch(sub_op) 
 {
	default:
		{
	         // Is FIV right for this?
		 PC -= 4;
		 S_REG[PSW] |= PSW_FIV;
		 v810_exp(FPU_HANDLER_ADDR, ECODE_FIV);
		 return(0);
		}
		break;

	case CMPF_S: extra_clocks = CMPF_S_Func(arg1, arg2); break;
	case CVT_WS:
		{
                 uint32 result = 0;
		 int32 fromfrom = (int32)P_REG[arg2];

	         FP_DECL_EX;
		 FP_DECL_S(SR);

		 FP_INIT_ROUNDMODE;

		 FP_FROM_INT_S(SR, fromfrom, 32, uint32);
		 FP_PACK_RAW_S(result, SR);

		 if(!CheckExceptions(result, _fex))
		 {
                  SetPREG(arg1, result);
                  SetFPUOPNonFPUFlags(result);
		 }
 	  	 extra_clocks = 5;
		}
		break;

	case CVT_SW:
		{
		 int32 result;
		 uint32 half_raw = 0x3f000000;

                 FP_DECL_EX;
                 FP_DECL_S(SR);
		 FP_DECL_S(H);
                 FP_INIT_ROUNDMODE;

		 half_raw |= P_REG[arg2] & 0x80000000;

                 FP_UNPACK_SEMIRAW_S(H, half_raw);
                 FP_UNPACK_SEMIRAW_S(SR, P_REG[arg2]);
                 FP_ADD_S(SR, SR, H);
		
		 FP_PACK_SEMIRAW_S(result, SR);
		 FP_UNPACK_RAW_S(SR, result);

		 FP_TO_INT_S(result, SR, 32, 1);

                 if(!CheckExceptions(result, _fex))
                 {
                  SetPREG(arg1, result);
                  SetFPUOPNonFPUFlags(result);
		 }
 	 	 extra_clocks = 8;
		}
		break;

	case ADDF_S: extra_clocks = ADDF_S_Func(arg1, arg2); break;
	case SUBF_S: extra_clocks = SUBF_S_Func(arg1, arg2); break;
	case MULF_S: extra_clocks = MULF_S_Func(arg1, arg2); break;
	case DIVF_S: extra_clocks = DIVF_S_Func(arg1, arg2); break;

	case TRNC_SW:
                {
		 FP_DECL_EX;
		 FP_DECL_S(SR);

	         FP_INIT_ROUNDMODE;
                 int32 result;

                 FP_UNPACK_RAW_S(SR, P_REG[arg2]);
                 FP_TO_INT_S(result, SR, 32, 1);

                 if(!CheckExceptions(result, _fex))
                 {
                  SetPREG(arg1, result);
                  SetFPUOPNonFPUFlags(result);
		 }
                 extra_clocks = 7;
                }
		break;
	}

	return extra_clocks;
}

// Generate Interupt #n
int v810_int(uint32 iNum) 
{
    if(Halted == HALT_FATAL_EXCEPTION) 
     return(0);

    if (iNum > 0x0F) return(0);  // Invalid Interupt number...
    if((S_REG[PSW] & PSW_NP)) return(0);
    if((S_REG[PSW] & PSW_EP)) return(0); // Exception pending?
    if((S_REG[PSW] & PSW_ID)) return(0); // Interupt disabled
    if(iNum < ((S_REG[PSW] & PSW_IA)>>16)) return(0); // Interupt too low on the chain

    //if(iNum == 14)
    //printf("V810 Int: %d\n", iNum);
    //Ready to Generate the Interupts
    S_REG[EIPC]  = PC;
    S_REG[EIPSW] = S_REG[PSW];

    PC = 0xFFFFFE00 | (iNum << 4);
    
    S_REG[ECR] = 0xFE00 | (iNum << 4);

    S_REG[PSW] |= PSW_EP;
    S_REG[PSW] |= PSW_ID;
    S_REG[PSW] &= ~PSW_AE;

    iNum++;
    if(iNum > 0x0F) 
	iNum = 0x0F;

    S_REG[PSW] &= ~PSW_IA;
    S_REG[PSW] |= iNum << 16;

    Halted = HALT_NONE;

    return(0);
}


// Generate exception
static void v810_exp(uint32 handler, uint16 eCode) 
{
    printf("Exception: %08x %04x\n", handler, eCode);

    if(S_REG[PSW] & PSW_NP) // Fatal exception
    {
     printf("Fatal exception; Code: %08x, ECR: %08x, PSW: %08x, PC: %08x\n", eCode, S_REG[ECR], S_REG[PSW], PC);
     Halted = HALT_FATAL_EXCEPTION;
     return;
    }
    else if(S_REG[PSW] & PSW_EP)  //Double Exception
    {
     S_REG[FEPC] = PC;
     S_REG[FEPSW] = S_REG[PSW];

     S_REG[ECR] = (S_REG[ECR] & 0xFFFF) | (eCode << 16);
     S_REG[PSW] |= PSW_NP;
     S_REG[PSW] |= PSW_ID;
     S_REG[PSW] &= ~PSW_AE;

     PC = 0xFFFFFFD0;
     return;
    }
    else 	// Regular exception
    {
     S_REG[EIPC] = PC;
     S_REG[EIPSW] = S_REG[PSW];
     S_REG[ECR] = (S_REG[ECR] & 0xFFFF0000) | eCode;
     S_REG[PSW] |= PSW_EP;
     S_REG[PSW] |= PSW_ID;
     S_REG[PSW] &= ~PSW_AE;

     PC = handler;
     return;
    }
}

int V810_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY32(P_REG, 32),
  SFARRAY32(S_REG, 32),
  SFVARN(PC, "PC"),
  SFVAR(Halted),
  SFVAR(lastop),
  SFVAR(lastclock),
  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "V810");

 if(load)
 {

 }
 return(ret);
}


