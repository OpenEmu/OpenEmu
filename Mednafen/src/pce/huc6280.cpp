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

#include "pce.h"
#include "vdc.h"
#include "hes.h"
#include "debug.h"
#include "huc.h"
#include "cdrom.h"


HuC6280 HuCPU;
uint8 *HuCPUFastMap[0x100];

#ifdef WANT_DEBUGGER
void (*HuC6280_Run)(int32 cycles) = NULL;
#endif

#define _PC              HuCPU.PC
#define _A               HuCPU.A
#define _X               HuCPU.X
#define _Y               HuCPU.Y
#define _S               HuCPU.S
#define _P               HuCPU.P
#define _PI              HuCPU.mooPI
#define _count           HuCPU.count
#define _tcount          HuCPU.tcount
#define _IRQlow          HuCPU.IRQlow

#define REDOSPEEDCACHE() { HuCPU.speed_shift_cache = ((HuCPU.speed ^ 1) << 1); }
#define ADDCYC(x)       \
{       \
 int __x=x << HuCPU.speed_shift_cache;	\
 _tcount+=__x;  \
 _count-=__x;        \
 HuCPU.timestamp+=__x;        \
}


//#define TBOL(_chk)      if(!(_chk & 0x1FFF)) { printf("RAWR: %04x\n", _PC); ADDCYC(1); }
#define TBOL(_chk)

void HuC6280_StealCycles(int count)
{
 ADDCYC(count);
}

void HuC6280_SetMPR(int i, int v)
{
 HuCPU.MPR[i] = v;
 HuCPU.FastPageR[i] = HuCPUFastMap[v] ? (HuCPUFastMap[v] + v * 8192) - i * 8192 : NULL;
}

void HuC6280_FlushMPRCache(void)
{
 for(int x = 0; x < 9; x++)
  HuC6280_SetMPR(x, HuCPU.MPR[x & 0x7]);
}

static ALWAYS_INLINE uint8 RdMemNorm(unsigned int A)
{
 if(HuCPU.FastPageR[A >> 13])
  return(HuCPU.FastPageR[A >> 13][A]);

 uint8 wmpr = HuCPU.MPR[A >> 13];
 return(PCERead[wmpr]((wmpr << 13) | (A & 0x1FFF)));
}

static ALWAYS_INLINE uint8 RdOpNorm(unsigned int A)
{
 if(HuCPU.FastPageR[A >> 13])
  return(HuCPU.FastPageR[A >> 13][A]);

 HuCPU.isopread = 1;
 uint8 wmpr = HuCPU.MPR[A >> 13];
 uint8 ret = PCERead[wmpr]((wmpr << 13) | (A & 0x1FFF));
 HuCPU.isopread = 0;
 return(ret);
}

static ALWAYS_INLINE void WrMemNorm(unsigned int A, uint8 V)
{
 uint8 wmpr = HuCPU.MPR[A >> 13];
 PCEWrite[wmpr]((wmpr << 13) | (A & 0x1FFF), V);
}

#ifdef WANT_DEBUGGER
static ALWAYS_INLINE uint8 RdMemHook(unsigned int A)
{
 if(HuCPU.ReadHook)
  return(HuCPU.ReadHook(&HuCPU,A));
 else
 {
  uint8 wmpr = HuCPU.MPR[A >> 13];
  return(PCERead[wmpr]((wmpr << 13) | (A & 0x1FFF)));
 }
}

static ALWAYS_INLINE uint8 RdOpHook(unsigned int A)
{
 uint8 ret;

 HuCPU.isopread = 1;

 if(HuCPU.ReadHook)
  ret = HuCPU.ReadHook(&HuCPU,A);
 else
 {
  uint8 wmpr = HuCPU.MPR[A >> 13];
  return(PCERead[wmpr]((wmpr << 13) | (A & 0x1FFF)));
 }

 HuCPU.isopread = 0;
 return(ret);
}

static ALWAYS_INLINE void WrMemHook(unsigned int A, uint8 V)
{
 if(!HuCPU.preexec)
 {
  uint8 wmpr = HuCPU.MPR[A >> 13];
  PCEWrite[wmpr]((wmpr << 13) | (A & 0x1FFF), V);
 }
 else if(HuCPU.WriteHook)
 {
  HuCPU.WriteHook(&HuCPU,A,V);
 }
}
#endif

#define PUSH(V) \
{       \
 uint8 VTMP=V;  \
 WrMem(0x2100 + _S, VTMP); \
 _S--;  \
}       

#define POP() RdMem(0x2100 + ++_S)

static uint8 ZNTable[256];
/* Some of these operations will only make sense if you know what the flag
   constants are. */

#define X_ZN(zort)      _P&=~(Z_FLAG|N_FLAG);_P|=ZNTable[zort]
#define X_ZNT(zort)	_P|=ZNTable[zort]

#define JR(cond)	\
{		\
 if(cond)	\
 {	\
  int32 disp;	\
  disp=(int8)RdOp(_PC);	\
  _PC++;	\
  ADDCYC(2);	\
  _PC+=disp;	\
 }	\
 else _PC++; 	\
 ADDBT(_PC);	\
}


#define BBRi(bitto) JR(!(x & (1 << bitto)))
#define BBSi(bitto) JR(x & (1 << bitto))

#define ST0_norm VDC_Write_ST(0, x)
#define ST1_norm VDC_Write_ST(2, x)
#define ST2_norm VDC_Write_ST(3, x)

#define ST0_testbp { if(HuCPU.WriteHook) { HuCPU.WriteHook(&HuCPU, 0x80000000 | (0xFF * 8192 + 0), x); } VDC_Write_ST_TestBP(0, x); }
#define ST1_testbp { if(HuCPU.WriteHook) { HuCPU.WriteHook(&HuCPU, 0x80000000 | (0xFF * 8192 + 2), x); } VDC_Write_ST_TestBP(2, x); }
#define ST2_testbp { if(HuCPU.WriteHook) { HuCPU.WriteHook(&HuCPU, 0x80000000 | (0xFF * 8192 + 3), x); } VDC_Write_ST_TestBP(3, x); }


#define LDA	   _A=x;X_ZN(_A)
#define LDX	   _X=x;X_ZN(_X)
#define LDY        _Y=x;X_ZN(_Y)

#define TPREFIX { uint8 Abackup = _A; if(_P & T_FLAG) { ADDCYC(3); _A = RdMem(0x2000 + _X); }

#define TPOSTFIX if(_P & T_FLAG) { WrMem(0x2000 + _X, _A); _A = Abackup; } }

/*  All of the freaky arithmetic operations. */
#define AND        TPREFIX; _A&=x;X_ZN(_A); TPOSTFIX;
#define BIT        _P&=~(Z_FLAG|V_FLAG|N_FLAG);_P|=ZNTable[x&_A]&Z_FLAG;_P|=x&(V_FLAG|N_FLAG);
#define EOR        TPREFIX; _A^=x;X_ZN(_A); TPOSTFIX;
#define ORA        TPREFIX; _A|=x;X_ZN(_A); TPOSTFIX;

#define ADC  TPREFIX; {	\
	      if(_P & D_FLAG)	\
	      {		\
		uint32 low = (_A & 0x0F) + (x & 0x0F) + (_P & 1);	\
		uint32 high = (_A & 0xF0) + (x & 0xF0);	\
		_P &= ~(Z_FLAG | C_FLAG | N_FLAG);	\
		if(low > 0x09) { high += 0x10; low += 0x06; }	\
		if(high > 0x90) { high += 0x60; }	\
		_P |= (high >> 8) & C_FLAG;	\
		_A = (low & 0x0F) | (high & 0xF0);	\
		X_ZNT(_A);	\
	      }	\
	      else	\
	      {	\
	       uint32 l=_A+x+(_P&1);	\
	       _P&=~(Z_FLAG|C_FLAG|N_FLAG|V_FLAG);	\
               _P|=((((_A^x)&0x80)^0x80) & ((_A^l)&0x80))>>1;	\
               _P|=(l>>8)&C_FLAG;	\
	       _A=l;	\
	       X_ZNT(_A);	\
	      }	\
	     } TPOSTFIX;

#define SBC  TPREFIX; {if(_P & D_FLAG)	\
	     {		\
	      uint32 c = (_P & 1) ^ 1;	\
	      uint32 l = _A - x - c;	\
	      uint32 low = (_A & 0x0f) - (x & 0x0f) - c;	\
	      uint32 high = (_A & 0xf0) - (x & 0xf0);	\
	      _P &= ~(Z_FLAG | C_FLAG | N_FLAG);	\
	      if(low & 0xf0) low -= 0x06;	\
	      if(low & 0x80) high -= 0x10;	\
	      if(high & 0x0f00) high -= 0x60;	\
	      _P |= ((l >> 8) & C_FLAG) ^ C_FLAG;	\
	      _A = (low & 0x0F) | (high & 0xf0);	\
	      X_ZNT(_A);	\
	     }	else {	\
	      uint32 l=_A-x-((_P&1)^1);	\
	      _P&=~(Z_FLAG|C_FLAG|N_FLAG|V_FLAG);	\
	      _P|=((_A^l)&(_A^x)&0x80)>>1;	\
	      _P|=((l>>8)&C_FLAG)^C_FLAG;	\
	      _A=l;	\
	      X_ZNT(_A);	\
	     } }TPOSTFIX;

#define CMPL(a1,a2) {	\
		     uint32 t=a1-a2;	\
		     X_ZN(t&0xFF);	\
		     _P&=~C_FLAG;	\
		     _P|=((t>>8)&C_FLAG)^C_FLAG;	\
		    }

#define TAM     for(int i = 0; i < 8; i ++) {               \
                        if(x & (1 << i))        \
			{	\
				HuC6280_SetMPR(i, _A);	\
			}	\
	        } HuC6280_SetMPR(8, HuCPU.MPR[0]);

#define TMA	for(int i = 0; i < 8; i ++) {		\
			if(x & (1 << i))	\
				_A = HuCPU.MPR[i];	\
		}	

#define CSL	HuCPU.speed = 0; REDOSPEEDCACHE()
#define CSH	HuCPU.speed = 1; REDOSPEEDCACHE()

#define RMB(bitto)	x &= ~(1 << (bitto & 7))
#define SMB(bitto)	x |= 1 << (bitto & 7)

//#define TSB	 { _P &= ~(Z_FLAG | V_FLAG | N_FLAG); x |= _A; X_ZNT(x); _P |= x & V_FLAG; }
//#define TRB	{ _P &= ~(Z_FLAG | V_FLAG | N_FLAG); x &= ~_A; X_ZNT(x); _P |= x & V_FLAG; }
//#define TSB	{ _P &= ~(Z_FLAG | V_FLAG | N_FLAG); _P |= (x & _A) ? 0 : Z_FLAG; _P |= x & (N_FLAG | V_FLAG); x |= _A; }
//#define TRB     { _P &= ~(Z_FLAG | V_FLAG | N_FLAG); _P |= (x & _A) ? 0 : Z_FLAG; _P |= x & (N_FLAG | V_FLAG); x &= ~_A; }
#define TSB   { _P &= ~(Z_FLAG | V_FLAG | N_FLAG); _P |= (x | _A) ? 0 : Z_FLAG; _P |= x & (N_FLAG | V_FLAG); x |= _A; }
#define TRB     { _P &= ~(Z_FLAG | V_FLAG | N_FLAG); _P |= (x & ~_A) ? 0 : Z_FLAG; _P |= x & (N_FLAG | V_FLAG); x &= ~_A; }

#define TST	{ _P &= ~(Z_FLAG | V_FLAG | N_FLAG); _P |= (x & zoomhack) ? 0: Z_FLAG; _P |= x & (V_FLAG | N_FLAG); }

#define CMP		CMPL(_A,x)
#define CPX		CMPL(_X,x)
#define CPY	      	CMPL(_Y,x)

/* The following operations modify the byte being worked on. */
#define DEC       	x--;X_ZN(x)
#define INC		x++;X_ZN(x)

#define ASL        _P&=~C_FLAG;_P|=x>>7;x<<=1;X_ZN(x)
#define LSR	_P&=~(C_FLAG|N_FLAG|Z_FLAG);_P|=x&1;x>>=1;X_ZNT(x)

#define ROL	{	\
		 uint8 l=x>>7;	\
		 x<<=1;	\
		 x|=_P&C_FLAG;	\
		 _P&=~(Z_FLAG|N_FLAG|C_FLAG);	\
		 _P|=l;	\
		 X_ZNT(x);	\
		}
#define ROR	{	\
		 uint8 l=x&1;	\
		 x>>=1;	\
		 x|=(_P&C_FLAG)<<7;	\
		 _P&=~(Z_FLAG|N_FLAG|C_FLAG);	\
		 _P|=l;	\
		 X_ZNT(x);	\
		}
		 
/* Absolute */
#define GetAB(target) 	\
{	\
 target=RdOp(_PC);	\
 _PC++;	\
 TBOL(_PC);  \
 target|=RdOp(_PC)<<8;	\
 _PC++;	\
}

/* Absolute Indexed(for reads) */
#define GetABI(target, i)	\
{	\
 unsigned int tmp;	\
 GetAB(tmp);	\
 target=tmp;	\
 target+=i;	\
}

/* Zero Page */
#define GetZP(target)	\
{	\
 target=RdOp(_PC); 	\
 _PC++;	\
}

/* Zero Page Indexed */
#define GetZPI(target,i)	\
{	\
 target=i+RdOp(_PC);	\
 _PC++;	\
}

/* Indirect */
#define GetIND(target)   \
{       \
 uint8 tmp;     \
 tmp=RdOp(_PC);        \
 _PC++; \
 target=RdMem(0x2000 + tmp);	\
 tmp++;         \
 target|=RdMem(0x2000 + tmp)<<8;       \
}


/* Indexed Indirect */
#define GetIX(target)	\
{	\
 uint8 tmp;	\
 tmp=RdOp(_PC);	\
 _PC++;	\
 tmp+=_X;	\
 target=RdMem(0x2000 + tmp);	\
 tmp++;		\
 target|=RdMem(0x2000 + tmp) <<8;	\
}

/* Indirect Indexed(for reads) */
#define GetIY(target)	\
{	\
 unsigned int rt;	\
 uint8 tmp;	\
 tmp=RdOp(_PC);	\
 rt=RdMem(0x2000 + tmp);	\
 tmp++;	\
 rt|=RdMem(0x2000 + tmp)<<8;	\
 target = (rt + _Y);	\
 _PC++;	\
}

/* Now come the macros to wrap up all of the above stuff addressing mode functions
   and operation macros.  Note that operation macros will always operate(redundant
   redundant) on the variable "x".
*/

#define RMW_A(op) {uint8 x=_A; op; _A=x; break; } /* Meh... */
#define RMW_AB(op) {unsigned int EA; uint8 x; GetAB(EA); x=RdMem(EA); op; WrMem(EA,x); break; }
#define RMW_ABI(reg,op) {unsigned int EA; uint8 x; GetABI(EA,reg); x=RdMem(EA); op; WrMem(EA,x); break; }
#define RMW_ABX(op)	RMW_ABI(_X,op)
#define RMW_ABY(op)	RMW_ABI(_Y,op)
#define RMW_IND(op) { unsigned int EA; uint8 x; GetIND(EA); x = RdMem(EA); op; WrMem(EA, x); break; }
#define RMW_IX(op)  {unsigned int EA; uint8 x; GetIX(EA); x=RdMem(EA); op; WrMem(EA,x); break; }
#define RMW_IY(op)  {unsigned int EA; uint8 x; GetIY(EA); x=RdMem(EA); op; WrMem(EA,x); break; }
#define RMW_ZP(op)  {uint8 EA; uint8 x; GetZP(EA); x=RdMem(0x2000 + EA); op; WrMem(0x2000 + EA, x); break; }
#define RMW_ZPX(op) {uint8 EA; uint8 x; GetZPI(EA,_X); x=RdMem(0x2000 + EA); op; WrMem(0x2000 + EA, x); break;}

#define LD_IM(op)	{uint8 x; x=RdOp(_PC); _PC++; op; break;}
#define LD_ZP(op)	{uint8 EA; uint8 x; GetZP(EA); x=RdMem(0x2000 + EA); op; break;}
#define LD_ZPX(op)  {uint8 EA; uint8 x; GetZPI(EA,_X); x=RdMem(0x2000 + EA); op; break;}
#define LD_ZPY(op)  {uint8 EA; uint8 x; GetZPI(EA,_Y); x=RdMem(0x2000 + EA); op; break;}
#define LD_AB(op)	{unsigned int EA; uint8 x; GetAB(EA); x=RdMem(EA); op; break; }
#define LD_ABI(reg,op)  {unsigned int EA; uint8 x; GetABI(EA,reg); x=RdMem(EA); op; break;}
#define LD_ABX(op)	LD_ABI(_X,op)
#define LD_ABY(op)	LD_ABI(_Y,op)

#define LD_IND(op)	{unsigned int EA; uint8 x; GetIND(EA); x=RdMem(EA); op; break;}
#define LD_IX(op)	{unsigned int EA; uint8 x; GetIX(EA); x=RdMem(EA); op; break;}
#define LD_IY(op)	{unsigned int EA; uint8 x; GetIY(EA); x=RdMem(EA); op; break;}

// Normal version:
#define BMT_PREHONK_N(pork) HuCPU.in_block_move = IBM_##pork;
#define BMT_HONKHONK_N(pork) if(PCE_IsCD) { PCECD_Run(); } if(_count < 0) { HuCPU.PC = pbackus; return;} continue_the_##pork:

// Version for breakpoints preexec:
#define BMT_PREHONK_P(pork)	HuCPU.in_block_move = 1;
#define BMT_HONKHONK_P(pork)  ;

#define BMT_TDD	BMT_PREHONK(TDD); do { ADDCYC(6); WrMem(HuCPU.bmt_dest, RdMem(HuCPU.bmt_src)); HuCPU.bmt_src--; HuCPU.bmt_dest--; BMT_HONKHONK(TDD); HuCPU.bmt_length--; } while(HuCPU.bmt_length);
#define BMT_TAI BMT_PREHONK(TAI); {HuCPU.bmt_alternate = 0; do { ADDCYC(6); WrMem(HuCPU.bmt_dest, RdMem(HuCPU.bmt_src + HuCPU.bmt_alternate)); HuCPU.bmt_dest++; HuCPU.bmt_alternate ^= 1; BMT_HONKHONK(TAI); HuCPU.bmt_length--; } while(HuCPU.bmt_length); }
#define BMT_TIA BMT_PREHONK(TIA); {HuCPU.bmt_alternate = 0; do { ADDCYC(6); WrMem(HuCPU.bmt_dest + HuCPU.bmt_alternate, RdMem(HuCPU.bmt_src)); HuCPU.bmt_src++; HuCPU.bmt_alternate ^= 1; BMT_HONKHONK(TIA); HuCPU.bmt_length--; } while(HuCPU.bmt_length); } 
#define BMT_TII BMT_PREHONK(TII); do { ADDCYC(6); WrMem(HuCPU.bmt_dest, RdMem(HuCPU.bmt_src)); HuCPU.bmt_src++; HuCPU.bmt_dest++; BMT_HONKHONK(TII); HuCPU.bmt_length--; } while(HuCPU.bmt_length); 
#define BMT_TIN BMT_PREHONK(TIN); do { ADDCYC(6); WrMem(HuCPU.bmt_dest, RdMem(HuCPU.bmt_src)); HuCPU.bmt_src++; BMT_HONKHONK(TIN); HuCPU.bmt_length--; } while(HuCPU.bmt_length);

// Block memory transfer load
#define LD_BMT(op)	{ PUSH(_Y); PUSH(_A); PUSH(_X); GetAB(HuCPU.bmt_src); TBOL(_PC); GetAB(HuCPU.bmt_dest); TBOL(_PC); GetAB(HuCPU.bmt_length); TBOL(_PC); op; HuCPU.in_block_move = 0; _X = POP(); _A = POP(); _Y = POP(); break; }

#define ST_ZP(r)	{uint8 EA; GetZP(EA); WrMem(0x2000 + EA, r); break;}
#define ST_ZPX(r)	{uint8 EA; GetZPI(EA,_X); WrMem(0x2000 + EA, r); break;}
#define ST_ZPY(r)	{uint8 EA; GetZPI(EA,_Y); WrMem(0x2000 + EA, r); break;}
#define ST_AB(r)	{unsigned int EA; GetAB(EA); WrMem(EA, r); break;}
#define ST_ABI(reg,r)	{unsigned int EA; GetABI(EA,reg); WrMem(EA,r); break; }
#define ST_ABX(r)	ST_ABI(_X,r)
#define ST_ABY(r)	ST_ABI(_Y,r)

#define ST_IND(r)	{unsigned int EA; GetIND(EA); WrMem(EA,r); break; }
#define ST_IX(r)	{unsigned int EA; GetIX(EA); WrMem(EA,r); break; }
#define ST_IY(r)	{unsigned int EA; GetIY(EA); WrMem(EA,r); break; }

static uint8 CycTable[256] =
{                             
 // NOTE:  0x80(BRA) and 0x44(BSR) always ADDCYC(2) in their emulation code.
 /*0x00*/ 8, 7, 3, 4, 6, 4, 6, 7, 3, 2, 2, 2, 7, 5, 7, 6, 
 /*0x10*/ 2, 7, 7, 4, 6, 4, 6, 7, 2, 5, 2, 2, 7, 5, 7, 6, 
 /*0x20*/ 7, 7, 3, 4, 4, 4, 6, 7, 4, 2, 2, 2, 5, 5, 7, 6, 
 /*0x30*/ 2, 7, 7, 2, 4, 4, 6, 7, 2, 5, 2, 2, 5, 5, 7, 6, 
 /*0x40*/ 7, 7, 3, 4, 6, 4, 6, 7, 3, 2, 2, 2, 4, 5, 7, 6, 
 /*0x50*/ 2, 7, 7, 5, 3, 4, 6, 7, 2, 5, 3, 2, 2, 5, 7, 6, 
 /*0x60*/ 7, 7, 2, 2, 4, 4, 6, 7, 4, 2, 2, 2, 7, 5, 7, 6, 
 /*0x70*/ 2, 7, 7, 17, 4, 4, 6, 7, 2, 5, 4, 2, 7, 5, 7, 6, 

 /*0x80*/ 2, 7, 2, 7, 4, 4, 4, 7, 2, 2, 2, 2, 5, 5, 5, 6, 
 /*0x90*/ 2, 7, 7, 8, 4, 4, 4, 7, 2, 5, 2, 2, 5, 5, 5, 6, 
 /*0xA0*/ 2, 7, 2, 7, 4, 4, 4, 7, 2, 2, 2, 2, 5, 5, 5, 6, 
 /*0xB0*/ 2, 7, 7, 8, 4, 4, 4, 7, 2, 5, 2, 2, 5, 5, 5, 6, 
 /*0xC0*/ 2, 7, 2, 17, 4, 4, 6, 7, 2, 2, 2, 2, 5, 5, 7, 6, 
 /*0xD0*/ 2, 7, 7, 17, 3, 4, 6, 7, 2, 5, 3, 2, 2, 5, 7, 6, 
 /*0xE0*/ 2, 7, 2, 17, 4, 4, 6, 7, 2, 2, 2, 2, 5, 5, 7, 6, 
 /*0xF0*/ 2, 7, 7, 17, 2, 4, 6, 7, 2, 5, 4, 2, 2, 5, 7, 6, 
};

void HuC6280_IRQBegin(int w)
{
 _IRQlow|=w;
}

void HuC6280_IRQEnd(int w)
{
 _IRQlow&=~w;
}

void HuC6280_Reset(void)
{
 HuCPU.timer_div = 1023;
 HuCPU.timer_load = 0;
 HuCPU.timer_value = 0;
 HuCPU.timer_status = 0;
 HuCPU.in_block_move = 0;
 HuCPU.cpoint = 0;
 HuCPU.preexec = 0;

 _IRQlow=MDFN_IQRESET;
}
  
void HuC6280_Init(void)
{
	int x;

	memset((void *)&HuCPU,0,sizeof(HuCPU));
	for(x=0;x<256;x++)
	 if(!x) ZNTable[x]=Z_FLAG;
	 else if (x&0x80) ZNTable[x]=N_FLAG;
	 else ZNTable[x]=0;

	#ifdef WANT_DEBUGGER
	HuC6280_Debug(NULL, NULL, NULL);
	#endif
}

void HuC6280_Power(void)
{
 _count=_tcount=_IRQlow=_PC=_A=_X=_Y=_S=_P=_PI=0;

 HuCPU.timestamp = 0;

 for(int i = 0; i < 9; i++)
 {
  HuCPU.MPR[i] = 0;
  HuCPU.FastPageR[i] = NULL;
 }  
 HuC6280_Reset();
}

#ifdef WANT_DEBUGGER
static void HuC6280_RunDebug(int32 cycles)
{
        #define RdMem RdMemHook
        #define RdOp  RdOpHook
        #define WrMem WrMemHook
	#define ADDBT(x) PCEDBG_AddBranchTrace(x)

	#define pbackus _PC

        _count+=cycles * pce_overclocked;

	PenguinPower:

        if(HuCPU.in_block_move && _count > 0 && !HuCPU.cpoint)
        {
         switch(HuCPU.in_block_move)
         {
          case IBM_TIA: goto continue_the_TIA;
          case IBM_TAI: goto continue_the_TAI;
          case IBM_TDD: goto continue_the_TDD;
          case IBM_TII: goto continue_the_TII;
          case IBM_TIN: goto continue_the_TIN;
         }
        }

        while(_count>0)
        {
         int32 temp;
         static uint8 b1;

         if(_IRQlow && !HuCPU.cpoint)
         {
          if(_IRQlow&MDFN_IQRESET)
          {
           HuCPU.speed = 0;
	   REDOSPEEDCACHE();

           HuCPU.IRQMask = HuCPU.IRQMaskDelay = 7;
           HuC6280_SetMPR(7, 0);
           _PC=RdMem(0xFFFE);
           _PC|=RdMem(0xFFFF)<<8;
	   ADDBT(_PC);
           _PI =_P=I_FLAG;
           _IRQlow&=~MDFN_IQRESET;
          }
          else
          {
           if(!(_PI&I_FLAG))
           {
            uint32 tmpa = 0;

            if(_IRQlow & MDFN_IQTIMER & HuCPU.IRQMaskDelay)
            tmpa = 0xFFFA;
            else if(_IRQlow & MDFN_IQIRQ1 & HuCPU.IRQMaskDelay)
             tmpa = 0xFFF8;
            else if(_IRQlow & MDFN_IQIRQ2 & HuCPU.IRQMaskDelay)
             tmpa = 0xFFF6;

            if(tmpa)
            {
             ADDCYC(8);
             PUSH(_PC>>8);
             PUSH(_PC);
             PUSH((_P&~B_FLAG));
             _P|=I_FLAG;
             _P&= ~(T_FLAG | D_FLAG);
             _PC=RdMem(tmpa);
             _PC|=RdMem(tmpa + 1) << 8;
	     ADDBT(_PC);
            }
           }
          }
          if(_count<=0)
          {
           _PI=_P;
           return;
           } /* Should increase accuracy without a */
                                           /* major speed hit. */
         }

         _PC &= 0xFFFF;     // Our cpu core can only handle PC going about 8192 bytes over, so make sure it never gets that far...

	 PCE_InDebug = 1;
	 b1 = RdOp(_PC);

	 if(HuCPU.ReadHook || HuCPU.WriteHook)
	 {
	  HuC6280 HuSave = HuCPU;
	  HuCPU.preexec = 1;
	  VDC_BP_Start();

	  _PC++;
          #define BMT_PREHONK     BMT_PREHONK_P
          #define BMT_HONKHONK    BMT_HONKHONK_P
	  #define ST0 ST0_testbp
	  #define ST1 ST1_testbp
	  #define ST2 ST2_testbp

	  switch(b1)
	  {
	   #include "ops.h"
	  }

	  #undef BMT_PREHONK
	  #undef BMT_HONKHONK
	  PCE_InDebug = 0;
	  HuCPU = HuSave;
	 }
	
	 PCE_InDebug = 0;
	 #undef ST0
	 #undef ST1
	 #undef ST2
	 #define ST0 ST0_norm
	 #define ST1 ST1_norm
	 #define ST2 ST2_norm
         #undef RdMem
         #undef RdOp
         #undef WrMem
         #define RdMem RdMemNorm
         #define RdOp  RdOpNorm
         #define WrMem WrMemNorm

	 HuCPU.cpoint = 1;
         PCEDBG_TestFoundBPoint();
         if(HuCPU.CPUHook) HuCPU.CPUHook(_PC, b1);
	 if(!HuCPU.cpoint) goto PenguinPower;
	 HuCPU.cpoint = 0;

	 _PI=_P;
         HuCPU.IRQMaskDelay = HuCPU.IRQMask;
         b1=RdOp(_PC);

         ADDCYC(CycTable[b1]);

	 if(PCE_IsCD)
	  PCECD_Run();

         _PC++;
         #define BMT_PREHONK     BMT_PREHONK_N
         #define BMT_HONKHONK    BMT_HONKHONK_N
         switch(b1)
         {
          #include "ops.h"
         }
         #undef BMT_PREHONK
         #undef BMT_HONKHONK
         _P &= ~T_FLAG;
         skip_T_flag_clear:;    // goto'd by the SET code
         temp=_tcount;
         _tcount=0;

         HuCPU.timer_div -= temp;
         while(HuCPU.timer_div < 0)
         {
          HuCPU.timer_div += 1024 * pce_overclocked;
          if(HuCPU.timer_status)
          {
           HuCPU.timer_value --;
           if(HuCPU.timer_value < 0)
           {
                HuCPU.timer_value = HuCPU.timer_load;
                HuC6280_IRQBegin(MDFN_IQTIMER);
           }
          }
         }
        }

	#undef pbackus

        #undef RdMem
        #undef RdOp
        #undef WrMem
	#undef ADDBT
}


static void HuC6280_RunNormal(int32 cycles)
#else
         #define ST0 ST0_norm
         #define ST1 ST1_norm
         #define ST2 ST2_norm
void HuC6280_Run(int32 cycles)
#endif
{
        #define RdMem RdMemNorm
	#define RdOp  RdOpNorm
        #define WrMem WrMemNorm
	#define ADDBT(x)

	_count+=cycles * pce_overclocked;

        uint32 pbackus;

        pbackus=_PC;

        #undef _PC
        #define _PC pbackus


	// This handles the (rare) case of a save state being saved in step mode in the debugger
	if(HuCPU.cpoint && _count > 0)
	{
	 HuCPU.cpoint = 0;
	 goto SephirothBishie;
	}


	if(HuCPU.in_block_move && _count > 0)
	{
	 switch(HuCPU.in_block_move)
	 {
	  case IBM_TIA: goto continue_the_TIA;
	  case IBM_TAI: goto continue_the_TAI;
	  case IBM_TDD: goto continue_the_TDD;
	  case IBM_TII: goto continue_the_TII;
	  case IBM_TIN:	goto continue_the_TIN;
	 }
	}

	while(_count>0)
	{
	 int32 temp;
	 static uint8 b1;

	 if(_IRQlow)
	 {
	  if(_IRQlow&MDFN_IQRESET)
	  {
	   HuCPU.speed = 0;
	   REDOSPEEDCACHE();

           HuCPU.IRQMask = HuCPU.IRQMaskDelay = 7;
	   HuC6280_SetMPR(7, 0);
	   _PC=RdMem(0xFFFE);
	   _PC|=RdMem(0xFFFF)<<8;
	   _PI =_P=I_FLAG;
	   _IRQlow&=~MDFN_IQRESET;
	  }
	  else
	  { 
	   if(!(_PI&I_FLAG))
	   {
	    uint32 tmpa = 0;

	    if(_IRQlow & MDFN_IQTIMER & HuCPU.IRQMaskDelay)
	     tmpa = 0xFFFA;
	    else if(_IRQlow & MDFN_IQIRQ1 & HuCPU.IRQMaskDelay)
	     tmpa = 0xFFF8;
	    else if(_IRQlow & MDFN_IQIRQ2 & HuCPU.IRQMaskDelay)
	     tmpa = 0xFFF6;

	    if(tmpa)
	    {
	     ADDCYC(8);
	     PUSH(_PC>>8);
	     PUSH(_PC);
	     PUSH((_P&~B_FLAG));
	     _P|=I_FLAG;
	     _P&= ~(T_FLAG | D_FLAG);
	     _PC=RdMem(tmpa);
	     _PC|=RdMem(tmpa + 1) << 8;
	    }
	   }
	  }
	  if(_count<=0) 
	  {
	   _PI=_P;
	   HuCPU.PC = pbackus;
	   return;
	   } /* Should increase accuracy without a */
 	                                   /* major speed hit. */
	 }

	 SephirothBishie:

         _PC &= 0xFFFF;     // Our cpu core can only handle PC going about 8192 bytes over, so make sure it never gets that far...
	 _PI=_P;
	 HuCPU.IRQMaskDelay = HuCPU.IRQMask;
	 b1=RdOp(_PC);

	 ADDCYC(CycTable[b1]);

         if(PCE_IsCD)
          PCECD_Run();

	 _PC++;
	 TBOL(_PC);

         #define BMT_PREHONK     BMT_PREHONK_N
         #define BMT_HONKHONK    BMT_HONKHONK_N
         switch(b1)
         {
          #include "ops.h"
         } 
	 #undef BMT_PREHONK
	 #undef BMT_HONKHONK
	 _P &= ~T_FLAG;
	 skip_T_flag_clear:;	// goto'd by the SET code
         temp=_tcount;
         _tcount=0;

         HuCPU.timer_div -= temp;
         while(HuCPU.timer_div < 0)
         {
          HuCPU.timer_div += 1024 * pce_overclocked;
          if(HuCPU.timer_status)
          {
           HuCPU.timer_value --;
           if(HuCPU.timer_value < 0)
           {
                HuCPU.timer_value = HuCPU.timer_load;
                HuC6280_IRQBegin(MDFN_IQTIMER);
           }
          }
         }
	}

        #undef _PC
        #define _PC HuCPU.PC
        _PC=pbackus;

	#undef RdMem
	#undef RdOp
	#undef WrMem
	#undef ADDBT
}

uint8 HuC6280_TimerRead(unsigned int A)
{
 #if 0
 return(HuCPU.timer_value | (PCEIODataBuffer & 0x80));
 #endif

 uint8 tvr = HuCPU.timer_value;
 int32 time_moo = HuCPU.timer_div - _tcount;

 if(time_moo <= -1 && HuCPU.timer_status)
 {
  if(time_moo == -1)
   tvr = (tvr - 1) & 0x7F;
  else
   tvr = HuCPU.timer_load;
 }

 return(tvr | (PCEIODataBuffer & 0x80));
}

void HuC6280_TimerWrite(unsigned int A, uint8 V)
{
 switch(A & 1)
 {
  case 0: HuCPU.timer_load = (V & 0x7F); break;
  case 1: if(V & 1) // Enable counter
	  {
	  	if(HuCPU.timer_status == 0)
		{
			HuCPU.timer_div = 1023;
			HuCPU.timer_value = HuCPU.timer_load;
		}
	  }
	  HuCPU.timer_status = V & 1;
	  break;
 }
}


uint8 HuC6280_IRQStatusRead(unsigned int A)
{
 if(!(A & 2)) 
  return(PCEIODataBuffer);

 switch(A & 1)
 {
  case 0:
	 if(!PCE_InDebug) HuC6280_IRQEnd(MDFN_IQTIMER); 
	 return(HuCPU.IRQMask ^ 0x7);
  case 1: 
	{
	 int status = 0;
	 if(HuCPU.IRQlow & MDFN_IQIRQ1) status |= 2;
	 if(HuCPU.IRQlow & MDFN_IQIRQ2) status |= 1;
	 if(HuCPU.IRQlow & MDFN_IQTIMER) status |= 4;
	 return(status | (PCEIODataBuffer & ~(1 | 2 | 4)));
	}
 }
 return(PCEIODataBuffer);
}

void HuC6280_IRQStatusWrite(unsigned int A, uint8 V)
{
 if(!(A & 2)) return;
 switch(A & 1)
 {
  case 0: HuCPU.IRQMask = (V & 0x7) ^ 0x7; break;
  case 1: if(!PCE_InDebug) HuC6280_IRQEnd(MDFN_IQTIMER); break;
 }
}

int HuC6280_StateAction(StateMem *sm, int load, int data_only)
{
 uint16 tmp_PC = HuCPU.PC;

 SFORMAT SFCPU[]=
 {
  SFVARN(tmp_PC, "PC"),
  SFVARN(HuCPU.A, "A"),
  SFVARN(HuCPU.P, "P"),
  SFVARN(HuCPU.X, "X"),
  SFVARN(HuCPU.Y, "Y"),
  SFVARN(HuCPU.S, "S"),
  SFVARN(HuCPU.IRQMask, "IRQMask"),
  SFVARN(HuCPU.IRQMaskDelay, "IRQMaskDelay"),
  SFARRAYN(HuCPU.MPR, 8, "MPR"),
  SFVARN(HuCPU.speed, "speed"),
  SFVARN(HuCPU.timer_status, "timer_status"),
  SFVARN(HuCPU.timer_value, "timer_value"),
  SFVARN(HuCPU.timer_load, "timer_load"),
  SFVARN(HuCPU.timer_div, "timer_div"),
  SFEND
 };

 SFORMAT SFCPUC[]=
 {
  SFVARN(HuCPU.IRQlow, "IRQlow"),
  SFVARN(HuCPU.tcount, "tcount"),
  SFVARN(HuCPU.count, "count"),
  SFVARN(HuCPU.in_block_move, "IBM"),
  SFVARN(HuCPU.bmt_src, "IBM_SRC"),
  SFVARN(HuCPU.bmt_dest, "IBM_DEST"),
  SFVARN(HuCPU.bmt_length, "IBM_LENGTH"),
  SFVARN(HuCPU.bmt_alternate, "IBM_ALTERNATE"),
  SFVARN(HuCPU.cpoint, "CPoint"),
  SFEND
 };

 std::vector <SSDescriptor> love;
 love.push_back(SSDescriptor(SFCPU, "CPU"));
 love.push_back(SSDescriptor(SFCPUC, "CPUC"));


 if(load)
 {
  HuCPU.cpoint = 0; // For older save states.
 }

 HuCPU.IRQMask ^= 7;
 HuCPU.IRQMaskDelay ^= 7;
 int ret = MDFNSS_StateAction(sm, load, data_only, love);
 HuCPU.IRQMask ^= 7;
 HuCPU.IRQMaskDelay ^= 7;

 if(load)
 {
  HuCPU.PC = tmp_PC;

  // Update MPR cache
  HuC6280_FlushMPRCache();
  REDOSPEEDCACHE();
 }
 return(ret);
}

void HuC6280_DumpMem(char *filename, uint32 start, uint32 end)
{
 FILE *fp=fopen(filename, "wb");

 for(uint32 x = start; x <= end; x++)
  fputc(RdMemNorm(x), fp);

 fclose(fp);
}

#ifdef WANT_DEBUGGER
void HuC6280_Debug(void (*CPUHook)(uint32, uint8),
                uint8 (*ReadHook)(HuC6280 *, unsigned int),
                void (*WriteHook)(HuC6280 *, uint32, uint8))
{
 if(CPUHook || ReadHook || WriteHook)
  HuC6280_Run = HuC6280_RunDebug;
 else
  HuC6280_Run = HuC6280_RunNormal;

 HuCPU.CPUHook = CPUHook;
 HuCPU.ReadHook = ReadHook;
 HuCPU.WriteHook = WriteHook;

}
#endif
