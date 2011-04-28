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

#include "pce.h"
#include "vdc.h"

namespace PCE_Fast
{

HuC6280 HuCPU;
uint8 *HuCPUFastMap[0x100];

#define HU_PC              PC_local //HuCPU.PC
#define HU_PC_base	 HuCPU.PC_base
#define HU_A               HuCPU.A
#define HU_X               X_local	//HuCPU.X
#define HU_Y               Y_local	//HuCPU.Y
#define HU_S               HuCPU.S
#define HU_P               P_local	//HuCPU.P
#define HU_PI              HuCPU.mooPI
#define HU_IRQlow          HuCPU.IRQlow
#define HU_Page1		Page1_local
//HuCPU.Page1
//Page1_local	//HuCPU.Page1

#ifdef HUC6280_LAZY_FLAGS
 #define HU_ZNFlags	 HuCPU.ZNFlags
#endif


#ifdef HUC6280_CRAZY_VERSION
#define LOAD_LOCALS_PC()        register uint8 *PC_local = HuCPU.PC;
#else
#define LOAD_LOCALS_PC()        register uint32 PC_local /*asm ("edi")*/ = HuCPU.PC; // asm ("edi") = HuCPU.PC;
#endif

#define LOAD_LOCALS()				\
	LOAD_LOCALS_PC();			\
        uint8 X_local = HuCPU.X;		\
        uint8 Y_local = HuCPU.Y;		\
        uint8 P_local = HuCPU.P;		\
	uint8 *Page1_local = HuCPU.Page1;

#define SAVE_LOCALS()	HuCPU.PC = PC_local;	\
			HuCPU.X = X_local;	\
			HuCPU.Y = Y_local;	\
			HuCPU.P = P_local;	\
			HuCPU.Page1 = Page1_local;

#ifdef HUC6280_LAZY_FLAGS
 #define COMPRESS_FLAGS()	HU_P &= ~(N_FLAG | Z_FLAG); HU_P |= ((HU_ZNFlags >> 24) & 0x80) | ((HU_ZNFlags & 0xFF) ? 0 : Z_FLAG);
 //((((HU_ZNFlags & 0xFF) - 1) >> 8) & Z_FLAG);
 #define EXPAND_FLAGS()	HU_ZNFlags = (HU_P << 24) | ((HU_P & Z_FLAG) ^ Z_FLAG);
#else
 #define COMPRESS_FLAGS()
 #define EXPAND_FLAGS()
#endif

#ifdef HUC6280_CRAZY_VERSION
 #define GetRealPC() ((unsigned int)(HU_PC - HU_PC_base))
 #define GetRealPC_EXTERNAL() ((unsigned int)(HuCPU.PC - HuCPU.PC_base))
#else
 #define GetRealPC() (HU_PC)
 #define GetRealPC_EXTERNAL() (HuCPU.PC)
#endif

#ifdef HUC6280_CRAZY_VERSION
 #define SetPC(value) { unsigned int tempmoo = value; HU_PC = &HuCPU.FastPageR[tempmoo >> 13][tempmoo]; 	\
	HU_PC_base = HU_PC - tempmoo; }
 #define SetPC_EXTERNAL(value) { unsigned int tempmoo = value; 	\
	HuCPU.PC = &HuCPU.FastPageR[tempmoo >> 13][tempmoo]; HuCPU.PC_base = HuCPU.PC - tempmoo; }
#else
 #define SetPC(value) { HU_PC = (value); }
 #define SetPC_EXTERNAL(value) { HuCPU.PC = (value); }
#endif

// Page change PC, GET IT?!
#ifdef HUC6280_CRAZY_VERSION
 #define FixPC_PC() SetPC(GetRealPC());
#else
 #define FixPC_PC()
#endif

//#define IncPC() { HU_PC++; if(!(GetRealPC() & 0x1FFF)) printf("Bank crossing: %04x\n", GetRealPC()); }
//#define IncPC() HU_PC++;
#if 0
#define IncPC() { HU_PC++; if(!(GetRealPC() & 0x1FFF) && 	\
	HuCPU.MPR[(GetRealPC() - 1) >> 13] != (HuCPU.MPR[(GetRealPC()) >> 13] - 1)) \
	printf("Bank crossing: %04x, %02x, %02x\n", GetRealPC(), HuCPU.MPR[(GetRealPC() - 1) >> 13], 	\
	HuCPU.MPR[GetRealPC() >> 13]); }
#else
#define IncPC() HU_PC++;
#endif

#ifdef HUC6280_CRAZY_VERSION
 #define RdAtPC() (*HU_PC)
 //#define RdAtAndIncPC_16() (HU_PC += 2, *(uint16 *)(HU_PC - 2))
#else
 #define RdAtPC() RdOp(HU_PC)
 //#define RdAtAndIncPC_16() (RdOp(HU_PC++) | ((RdOp(HU_PC++) << 8)))
#endif

// If we change this definition, we'll need to also fix HuC6280_StealCycle() in huc6280.h
#define ADDCYC(x) { HuCPU.timestamp += x; }

static uint8 dummy_bank[8192 + 8192];  // + 8192 for PC-as-ptr safety padding

#define SET_MPR(arg_i, arg_v)				\
{							\
 const unsigned int wmpr = arg_i, wbank = arg_v;	\
 if(wmpr == 1)						\
 {							\
  if(wbank != 0xF8 || !HuCPUFastMap[wbank])		\
    printf("Crazy page 1: %02x\n", wbank);		\
  HU_Page1 = HuCPUFastMap[wbank] ? HuCPUFastMap[wbank] + wbank * 8192 : dummy_bank;	\
 }							\
 HuCPU.MPR[wmpr] = wbank;					\
 HuCPU.FastPageR[wmpr] = HuCPUFastMap[wbank] ? (HuCPUFastMap[wbank] + wbank * 8192) - wmpr * 8192 : (dummy_bank - wmpr * 8192);	\
}

void HuC6280_SetMPR(int i, int v)
{
 uint8 *Page1_local = HuCPU.Page1;

 SET_MPR(i, v);

 HuCPU.Page1 = Page1_local;
}


static void HuC6280_FlushMPRCache(void)
{
 for(int x = 0; x < 9; x++)
  HuC6280_SetMPR(x, HuCPU.MPR[x & 0x7]);
}

static INLINE uint8 RdMem(unsigned int A)
{
 uint8 wmpr = HuCPU.MPR[A >> 13];
 return(PCERead[wmpr]((wmpr << 13) | (A & 0x1FFF)));
}

static INLINE uint16 RdMem16(unsigned int A)
{
 return(RdMem(A) | (RdMem(A + 1) << 8));
}

static INLINE void WrMem(unsigned int A, uint8 V)
{
 uint8 wmpr = HuCPU.MPR[A >> 13];
 PCEWrite[wmpr]((wmpr << 13) | (A & 0x1FFF), V);
}

static INLINE uint8 RdOp(unsigned int A)
{
 return(HuCPU.FastPageR[A >> 13][A]);
}

#define PUSH(V) \
{       \
 HU_Page1[0x100 + HU_S] = V; \
 HU_S--;  \
}       

#define PUSH_PC()	\
{	\
 unsigned int real_pc = GetRealPC();	\
 PUSH(real_pc >> 8);	\
 PUSH(real_pc);		\
}

#define POP() HU_Page1[0x100 + ++HU_S]

#define POP_PC()	\
{	\
 unsigned int npc;	\
 npc = POP();	\
 npc |= POP() << 8;	\
 SetPC(npc);	\
}

// Hopefully we never RTS to 0x0000. ;)
#define POP_PC_AP()        \
{       \
 uint32 npc;      \
 npc = POP();   \
 npc |= POP() << 8;     \
 npc++;		\
 SetPC(npc);    \
}

/* Some of these operations will only make sense if you know what the flag
   constants are. */

#ifdef HUC6280_LAZY_FLAGS
 #define X_ZN(zort)      { HU_ZNFlags = (int32)(int8)(uint8)(zort); }
 #define X_ZN_BIT(opres, argie)	 { HU_ZNFlags = (opres) | ((argie) << 24); }
#else
 static uint8 ZNTable[256];
 #define X_ZN(zort)      HU_P&=~(Z_FLAG|N_FLAG);HU_P|=ZNTable[zort]
 #define X_ZN_BIT(opres, argie)	{ HU_P &= ~(Z_FLAG | N_FLAG); HU_P |= ZNTable[opres] & Z_FLAG; HU_P |= argie & N_FLAG; }
#endif

#define JR(cond)        \
{               \
 if(cond)       \
 {      \
  int32 disp;   \
  disp = 1 + (int8)RdAtPC();      \
  ADDCYC(2);    \
  HU_PC+=disp;    \
 }      \
 else IncPC();  \
}

#define BRA            \
{                      \
 int32 disp;           \
 disp = 1 + (int8)RdAtPC();      \
 HU_PC+=disp;            \
}

#define BBRi(bitto) JR(!(x & (1 << bitto)))
#define BBSi(bitto) JR(x & (1 << bitto))

#define ST0 VDC_Write_ST(0, x)
#define ST1 VDC_Write_ST(2, x)
#define ST2 VDC_Write_ST(3, x)

#define LDA	   HU_A=x;X_ZN(HU_A)
#define LDX	   HU_X=x;X_ZN(HU_X)
#define LDY        HU_Y=x;X_ZN(HU_Y)

/*  All of the freaky arithmetic operations. */
#define AND        HU_A&=x;X_ZN(HU_A);

// FIXME:
#define BIT        HU_P&=~V_FLAG; X_ZN_BIT(x & HU_A, x); HU_P |= x & V_FLAG;
#define EOR        HU_A^=x;X_ZN(HU_A);
#define ORA        HU_A|=x;X_ZN(HU_A);

#define ADC  {	\
	      if(HU_P & D_FLAG)	\
	      {		\
		uint32 low = (HU_A & 0x0F) + (x & 0x0F) + (HU_P & 1);	\
		uint32 high = (HU_A & 0xF0) + (x & 0xF0);	\
		HU_P &= ~C_FLAG;	\
		if(low > 0x09) { high += 0x10; low += 0x06; }	\
		if(high > 0x90) { high += 0x60; }	\
		HU_P |= (high >> 8) & C_FLAG;	\
		HU_A = (low & 0x0F) | (high & 0xF0);	\
		X_ZN(HU_A);	\
	      }	\
	      else	\
	      {	\
	       uint32 l=HU_A+x+(HU_P&1);	\
	       HU_P&=~(C_FLAG|V_FLAG);	\
               HU_P|=((((HU_A^x)&0x80)^0x80) & ((HU_A^l)&0x80))>>1;	\
               HU_P|=(l>>8)&C_FLAG;	\
	       HU_A=l;	\
	       X_ZN(HU_A);	\
	      }	\
	     }

#define SBC  if(HU_P & D_FLAG)	\
	     {		\
	      uint32 c = (HU_P & 1) ^ 1;	\
	      uint32 l = HU_A - x - c;	\
	      uint32 low = (HU_A & 0x0f) - (x & 0x0f) - c;	\
	      uint32 high = (HU_A & 0xf0) - (x & 0xf0);	\
	      HU_P &= ~(C_FLAG);	\
	      if(low & 0xf0) low -= 0x06;	\
	      if(low & 0x80) high -= 0x10;	\
	      if(high & 0x0f00) high -= 0x60;	\
	      HU_P |= ((l >> 8) & C_FLAG) ^ C_FLAG;	\
	      HU_A = (low & 0x0F) | (high & 0xf0);	\
	      X_ZN(HU_A);	\
	     }	else {	\
	      uint32 l=HU_A-x-((HU_P&1)^1);	\
	      HU_P&=~(C_FLAG|V_FLAG);	\
	      HU_P|=((HU_A^l)&(HU_A^x)&0x80)>>1;	\
	      HU_P|=((l>>8)&C_FLAG)^C_FLAG;	\
	      HU_A=l;	\
	      X_ZN(HU_A);	\
	     }

#define CMPL(a1,a2) {	\
		     uint32 t=a1-a2;	\
		     X_ZN(t&0xFF);	\
		     HU_P&=~C_FLAG;	\
		     HU_P|=((t>>8)&C_FLAG)^C_FLAG;	\
		    }

#define TAM     for(int i = 0; i < 8; i ++) {               \
                        if(x & (1 << i))        \
			{	\
				SET_MPR(i, HU_A);	\
			}	\
	        } SET_MPR(8, HuCPU.MPR[0]);

#define TMA	for(int i = 0; i < 8; i ++) {		\
			if(x & (1 << i))	\
				HU_A = HuCPU.MPR[i];	\
		}	

#define CSL	
#define CSH	

#define RMB(bitto)	x &= ~(1 << (bitto & 7))
#define SMB(bitto)	x |= 1 << (bitto & 7)

// FIXME
#define TSB   { HU_P &= ~V_FLAG; X_ZN_BIT(x | HU_A, x); HU_P |= x & V_FLAG; x |= HU_A; }
#define TRB     { HU_P &= ~V_FLAG; X_ZN_BIT(x & ~HU_A, x); HU_P |= x & V_FLAG; x &= ~HU_A; }
#define TST	{ HU_P &= ~V_FLAG; X_ZN_BIT(x & zoomhack, x); HU_P |= x & V_FLAG; }

#define CMP		CMPL(HU_A,x)
#define CPX		CMPL(HU_X,x)
#define CPY	      	CMPL(HU_Y,x)

/* The following operations modify the byte being worked on. */
#define DEC       	x--;X_ZN(x)
#define INC		x++;X_ZN(x)

#define ASL        HU_P&=~C_FLAG;HU_P|=x>>7;x<<=1;X_ZN(x)
#define LSR	HU_P&=~C_FLAG;HU_P|=x&1;x>>=1;X_ZN(x)

#define ROL	{	\
		 uint8 l=x>>7;	\
		 x<<=1;	\
		 x|=HU_P&C_FLAG;	\
		 HU_P&=~C_FLAG;	\
		 HU_P|=l;	\
		 X_ZN(x);	\
		}
#define ROR	{	\
		 uint8 l=x&1;	\
		 x>>=1;	\
		 x|=(HU_P&C_FLAG)<<7;	\
		 HU_P&=~C_FLAG;	\
		 HU_P|=l;	\
		 X_ZN(x);	\
		}

/* Absolute */
#define GetAB(target)   \
{       		\
 target=RdAtPC();       \
 IncPC();       	\
 target|=RdAtPC()<<8;   \
 IncPC();       	\
}

/* Absolute Indexed(for reads) */
#define GetABI(target, i)	\
{				\
 unsigned int tmp;		\
 GetAB(tmp);			\
 target=tmp;			\
 target+=i;			\
}

/* Zero Page */
#define GetZP(target)	\
{			\
 target=RdAtPC(); 	\
 IncPC();		\
}

/* Zero Page Indexed */
#define GetZPI(target,i)	\
{				\
 target=i+RdAtPC();		\
 IncPC();			\
}

/* Indirect */
#define GetIND(target)		\
{       			\
 uint8 tmp;			\
 tmp=RdAtPC();			\
 IncPC();			\
 target=HU_Page1[tmp];		\
 tmp++;				\
 target|=HU_Page1[tmp]<<8;       	\
}


/* Indexed Indirect */
#define GetIX(target)		\
{				\
 uint8 tmp;			\
 tmp=RdAtPC();			\
 IncPC();			\
 tmp+=HU_X;			\
 target=HU_Page1[tmp];		\
 tmp++;				\
 target|=HU_Page1[tmp] <<8;	\
}

/* Indirect Indexed(for reads) */
#define GetIY(target)	\
{			\
 unsigned int rt;	\
 uint8 tmp;		\
 tmp=RdAtPC();		\
 rt=HU_Page1[tmp];	\
 tmp++;			\
 rt|=HU_Page1[tmp]<<8;	\
 target = (rt + HU_Y);	\
 IncPC();		\
}

/* Now come the macros to wrap up all of the above stuff addressing mode functions
   and operation macros.  Note that operation macros will always operate(redundant
   redundant) on the variable "x".
*/

#define RMW_A(op) {uint8 x=HU_A; op; HU_A=x; break; } /* Meh... */
#define RMW_AB(op) {unsigned int EA; uint8 x; GetAB(EA); x=RdMem(EA); op; WrMem(EA,x); break; }
#define RMW_ABI(reg,op) {unsigned int EA; uint8 x; GetABI(EA,reg); x=RdMem(EA); op; WrMem(EA,x); break; }
#define RMW_ABX(op)	RMW_ABI(HU_X,op)
#define RMW_ABY(op)	RMW_ABI(HU_Y,op)
#define RMW_IND(op) { unsigned int EA; uint8 x; GetIND(EA); x = RdMem(EA); op; WrMem(EA, x); break; }
#define RMW_IX(op)  { unsigned int EA; uint8 x; GetIX(EA); x=RdMem(EA); op; WrMem(EA,x); break; }
#define RMW_IY(op)  { unsigned int EA; uint8 x; GetIY(EA); x=RdMem(EA); op; WrMem(EA,x); break; }
#define RMW_ZP(op)  { uint8 EA; uint8 x; GetZP(EA); x=HU_Page1[EA]; op; HU_Page1[EA] = x; break; }
#define RMW_ZPX(op) { uint8 EA; uint8 x; GetZPI(EA,HU_X); x=HU_Page1[EA]; op; HU_Page1[EA] = x; break;}

#define LD_IM(op)	{ uint8 x; x=RdAtPC(); IncPC(); op; break; }
#define LD_ZP(op)	{ uint8 EA; uint8 x; GetZP(EA); x=HU_Page1[EA]; op; break; }
#define LD_ZPX(op) 	{ uint8 EA; uint8 x; GetZPI(EA,HU_X); x=HU_Page1[EA]; op; break; }
#define LD_ZPY(op)  	{ uint8 EA; uint8 x; GetZPI(EA,HU_Y); x=HU_Page1[EA]; op; break; }
#define LD_AB(op)	{ unsigned int EA; uint8 x; GetAB(EA); x=RdMem(EA); op; break; }
#define LD_ABI(reg,op)  { unsigned int EA; uint8 x; GetABI(EA,reg); x=RdMem(EA); op; break; }
#define LD_ABX(op)	LD_ABI(HU_X,op)
#define LD_ABY(op)	LD_ABI(HU_Y,op)

#define LD_IND(op)	{ unsigned int EA; uint8 x; GetIND(EA); x=RdMem(EA); op; break; }
#define LD_IX(op)	{ unsigned int EA; uint8 x; GetIX(EA); x=RdMem(EA); op; break; }
#define LD_IY(op)	{ unsigned int EA; uint8 x; GetIY(EA); x=RdMem(EA); op; break; }

#define BMT_PREHONK(pork) HuCPU.in_block_move = IBM_##pork;
#define BMT_HONKHONK(pork) if(HuCPU.timestamp >= next_user_event) goto GetOutBMT; continue_the_##pork:

#define BMT_TDD	BMT_PREHONK(TDD); do { ADDCYC(6); WrMem(HuCPU.bmt_dest, RdMem(HuCPU.bmt_src)); HuCPU.bmt_src--; HuCPU.bmt_dest--; BMT_HONKHONK(TDD); HuCPU.bmt_length--; } while(HuCPU.bmt_length);
#define BMT_TAI BMT_PREHONK(TAI); {HuCPU.bmt_alternate = 0; do { ADDCYC(6); WrMem(HuCPU.bmt_dest, RdMem(HuCPU.bmt_src + HuCPU.bmt_alternate)); HuCPU.bmt_dest++; HuCPU.bmt_alternate ^= 1; BMT_HONKHONK(TAI); HuCPU.bmt_length--; } while(HuCPU.bmt_length); }
#define BMT_TIA BMT_PREHONK(TIA); {HuCPU.bmt_alternate = 0; do { ADDCYC(6); WrMem(HuCPU.bmt_dest + HuCPU.bmt_alternate, RdMem(HuCPU.bmt_src)); HuCPU.bmt_src++; HuCPU.bmt_alternate ^= 1; BMT_HONKHONK(TIA); HuCPU.bmt_length--; } while(HuCPU.bmt_length); } 
#define BMT_TII BMT_PREHONK(TII); do { ADDCYC(6); WrMem(HuCPU.bmt_dest, RdMem(HuCPU.bmt_src)); HuCPU.bmt_src++; HuCPU.bmt_dest++; BMT_HONKHONK(TII); HuCPU.bmt_length--; } while(HuCPU.bmt_length); 
#define BMT_TIN BMT_PREHONK(TIN); do { ADDCYC(6); WrMem(HuCPU.bmt_dest, RdMem(HuCPU.bmt_src)); HuCPU.bmt_src++; BMT_HONKHONK(TIN); HuCPU.bmt_length--; } while(HuCPU.bmt_length);

// Block memory transfer load
#define LD_BMT(op)	{ PUSH(HU_Y); PUSH(HU_A); PUSH(HU_X); GetAB(HuCPU.bmt_src); GetAB(HuCPU.bmt_dest); GetAB(HuCPU.bmt_length); op; HuCPU.in_block_move = 0; HU_X = POP(); HU_A = POP(); HU_Y = POP(); break; }

#define ST_ZP(r)	{uint8 EA; GetZP(EA); HU_Page1[EA] = r; break;}
#define ST_ZPX(r)	{uint8 EA; GetZPI(EA,HU_X); HU_Page1[EA] = r; break;}
#define ST_ZPY(r)	{uint8 EA; GetZPI(EA,HU_Y); HU_Page1[EA] = r; break;}
#define ST_AB(r)	{unsigned int EA; GetAB(EA); WrMem(EA, r); break;}
#define ST_ABI(reg,r)	{unsigned int EA; GetABI(EA,reg); WrMem(EA,r); break; }
#define ST_ABX(r)	ST_ABI(HU_X,r)
#define ST_ABY(r)	ST_ABI(HU_Y,r)

#define ST_IND(r)	{unsigned int EA; GetIND(EA); WrMem(EA,r); break; }
#define ST_IX(r)	{unsigned int EA; GetIX(EA); WrMem(EA,r); break; }
#define ST_IY(r)	{unsigned int EA; GetIY(EA); WrMem(EA,r); break; }

static const uint8 CycTable[256] =
{                             
 /*0x00*/ 8, 7, 3, 4, 6, 4, 6, 7, 3, 2, 2, 2, 7, 5, 7, 6, 
 /*0x10*/ 2, 7, 7, 4, 6, 4, 6, 7, 2, 5, 2, 2, 7, 5, 7, 6, 
 /*0x20*/ 7, 7, 3, 4, 4, 4, 6, 7, 4, 2, 2, 2, 5, 5, 7, 6, 
 /*0x30*/ 2, 7, 7, 2, 4, 4, 6, 7, 2, 5, 2, 2, 5, 5, 7, 6, 
 /*0x40*/ 7, 7, 3, 4, 8, 4, 6, 7, 3, 2, 2, 2, 4, 5, 7, 6, 
 /*0x50*/ 2, 7, 7, 5, 3, 4, 6, 7, 2, 5, 3, 2, 2, 5, 7, 6, 
 /*0x60*/ 7, 7, 2, 2, 4, 4, 6, 7, 4, 2, 2, 2, 7, 5, 7, 6, 
 /*0x70*/ 2, 7, 7, 17, 4, 4, 6, 7, 2, 5, 4, 2, 7, 5, 7, 6, 

 /*0x80*/ 4, 7, 2, 7, 4, 4, 4, 7, 2, 2, 2, 2, 5, 5, 5, 6, 
 /*0x90*/ 2, 7, 7, 8, 4, 4, 4, 7, 2, 5, 2, 2, 5, 5, 5, 6, 
 /*0xA0*/ 2, 7, 2, 7, 4, 4, 4, 7, 2, 2, 2, 2, 5, 5, 5, 6, 
 /*0xB0*/ 2, 7, 7, 8, 4, 4, 4, 7, 2, 5, 2, 2, 5, 5, 5, 6, 
 /*0xC0*/ 2, 7, 2, 17, 4, 4, 6, 7, 2, 2, 2, 2, 5, 5, 7, 6, 
 /*0xD0*/ 2, 7, 7, 17, 3, 4, 6, 7, 2, 5, 3, 2, 2, 5, 7, 6, 
 /*0xE0*/ 2, 7, 2, 17, 4, 4, 6, 7, 2, 2, 2, 2, 5, 5, 7, 6, 
 /*0xF0*/ 2, 7, 7, 17, 2, 4, 6, 7, 2, 5, 4, 2, 2, 5, 7, 6, 
};
#if 0
static bool WillIRQOccur(void) NO_INLINE;
static bool WillIRQOccur(void)
{
 bool ret = false;

 if(HU_IRQlow)
 {
  if(!(HU_PI&I_FLAG))
  {
   if(HU_IRQlow & MDFN_IQTIMER & HuCPU.IRQMaskDelay)
    ret = true;
   else if((HU_IRQlow & MDFN_IQIRQ1 & HuCPU.IRQMaskDelay) || ((HU_IRQlow >> 8) & MDFN_IQIRQ1 & HuCPU.IRQMaskDelay))
    ret = true;
   else if(HU_IRQlow & MDFN_IQIRQ2 & HuCPU.IRQMaskDelay)
    ret = true;
  }
 }

 return(true);
}
#endif

void HuC6280_IRQBegin(int w)
{
 HU_IRQlow|=w;
}

void HuC6280_IRQEnd(int w)
{
 HU_IRQlow&=~w;
}

void HuC6280_Reset(void)
{
 HuCPU.timer_next_timestamp = HuCPU.timestamp + 1024;

 HuCPU.timer_load = 0;
 HuCPU.timer_value = 0;
 HuCPU.timer_status = 0;
 HuCPU.in_block_move = 0;

 unsigned int npc;

 HuCPU.IRQMask = HuCPU.IRQMaskDelay = 7;

 HuC6280_SetMPR(0, 0xFF);
 HuC6280_SetMPR(1, 0xF8);

 for(int i = 2; i < 8; i++)
  HuC6280_SetMPR(i, 0);

 npc = RdMem16(0xFFFE);

 #define PC_local HuCPU.PC
 SetPC(npc);
 #undef PC_local

 HuCPU.mooPI = I_FLAG;
 HuCPU.P = I_FLAG;

 HU_IRQlow = 0;
}
  
void HuC6280_Init(void)
{
	memset((void *)&HuCPU,0,sizeof(HuCPU));
	memset(dummy_bank, 0, sizeof(dummy_bank));

	#ifdef HUC6280_LAZY_FLAGS

	#else
         for(int x=0; x < 256; x++)
          if(!x) ZNTable[x]=Z_FLAG;
          else if (x&0x80) ZNTable[x]=N_FLAG;
          else ZNTable[x]=0;
	#endif
}

void HuC6280_Power(void)
{
 HuCPU.IRQlow = 0;

 HuCPU.A = 0;
 HuCPU.X = 0;
 HuCPU.Y = 0;
 HuCPU.S = 0;
 HuCPU.P = 0;
 HuCPU.mooPI = 0;

 #if 0
 HU_PC = HU_PC_base = NULL;
 #else
 HuCPU.PC = 0;
 #endif

 HuCPU.timestamp = 0;

 for(int i = 0; i < 9; i++)
 {
  HuCPU.MPR[i] = 0;
  HuCPU.FastPageR[i] = NULL;
 }  
 HuC6280_Reset();
}

void HuC6280_Run(int32 cycles)
{
	const int32 next_user_event = HuCPU.previous_next_user_event + cycles * pce_overclocked;

	HuCPU.previous_next_user_event = next_user_event;

	LOAD_LOCALS();

	if(HuCPU.timestamp >= next_user_event)
	 return;

	int32 next_event;

	if(HuCPU.in_block_move)
	{
         next_event = (next_user_event < HuCPU.timer_next_timestamp) ? next_user_event : HuCPU.timer_next_timestamp;

	 switch(HuCPU.in_block_move)
	 {
	  case IBM_TIA: goto continue_the_TIA;
	  case IBM_TAI: goto continue_the_TAI;
	  case IBM_TDD: goto continue_the_TDD;
	  case IBM_TII: goto continue_the_TII;
	  case IBM_TIN:	goto continue_the_TIN;
	 }
	}

	while(HuCPU.timestamp < next_user_event)
	{
	 next_event = (next_user_event < HuCPU.timer_next_timestamp) ? next_user_event : HuCPU.timer_next_timestamp;

	 while(HuCPU.timestamp < next_event)
	 {
	  uint8 b1;

	  if(HU_IRQlow)
	  {
	   if(!(HU_PI&I_FLAG))
	   {
	    uint32 tmpa = 0;

	    if(HU_IRQlow & MDFN_IQTIMER & HuCPU.IRQMaskDelay)
	     tmpa = 0xFFFA;
            else if((HU_IRQlow & MDFN_IQIRQ1 & HuCPU.IRQMaskDelay) || ((HU_IRQlow >> 8) & MDFN_IQIRQ1 & HuCPU.IRQMaskDelay))
	     tmpa = 0xFFF8;
	    else if(HU_IRQlow & MDFN_IQIRQ2 & HuCPU.IRQMaskDelay)
	     tmpa = 0xFFF6;

	    if(tmpa)
	    {
	     unsigned int npc;

	     ADDCYC(8);
	     PUSH_PC();

	     COMPRESS_FLAGS();
	     PUSH((HU_P&~B_FLAG));
	     HU_P |= I_FLAG;
	     HU_P &= ~(T_FLAG | D_FLAG);
	     HU_PI = HU_P;

	     npc = RdMem16(tmpa);
	     SetPC(npc);

	     if(tmpa == 0xFFF8)
	      HU_IRQlow &= ~0x200;

	     continue;
	    }
	   }
	  }	// end if(HU_IRQlow)

	  //printf("%04x\n", GetRealPC());
	  HU_PI = HU_P;
	  HuCPU.IRQMaskDelay = HuCPU.IRQMask;

	  b1 = RdAtPC();

	  ADDCYC(CycTable[b1]);

	  IncPC();

          switch(b1)
          {
           #include "huc6280_ops.inc"
          } 

	  #ifndef HUC6280_EXTRA_CRAZY
 	  FixPC_PC();
	  #endif
	 }	// end while(HuCPU.timestamp < next_event)

	 while(HuCPU.timestamp >= HuCPU.timer_next_timestamp)
	 {
	  HuCPU.timer_next_timestamp += 1024 * pce_overclocked;

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
	} // end while(HuCPU.timestamp < next_user_event)

	GetOutBMT:

	SAVE_LOCALS();
}

void HuC6280_ResetTS(void)
{
 HuCPU.timer_next_timestamp -= HuCPU.timestamp;
 HuCPU.previous_next_user_event -= HuCPU.timestamp;
 HuCPU.timestamp = 0;
}

int HuC6280_StateAction(StateMem *sm, int load, int data_only)
{
 uint16 tmp_PC = GetRealPC_EXTERNAL();

 #define P_local HuCPU.P
 COMPRESS_FLAGS();

 SFORMAT SFCPU[]=
 {
  SFVARN(tmp_PC, "PC"),
  SFVARN(HuCPU.A, "A"),
  SFVARN(HuCPU.P, "P"),
  SFVARN(HuCPU.X, "X"),
  SFVARN(HuCPU.Y, "Y"),
  SFVARN(HuCPU.S, "S"),
  SFVARN(HuCPU.mooPI, "PI"),

  SFVARN(HuCPU.IRQMask, "IRQMask"),
  SFVARN(HuCPU.IRQMaskDelay, "IRQMaskDelay"),
  SFARRAYN(HuCPU.MPR, 8, "MPR"),
  SFVARN(HuCPU.timer_status, "timer_status"),
  SFVARN(HuCPU.timer_value, "timer_value"),
  SFVARN(HuCPU.timer_load, "timer_load"),


  SFVARN(HuCPU.IRQlow, "IRQlow"),
  SFVARN(HuCPU.in_block_move, "IBM"),
  SFVARN(HuCPU.bmt_src, "IBM_SRC"),
  SFVARN(HuCPU.bmt_dest, "IBM_DEST"),
  SFVARN(HuCPU.bmt_length, "IBM_LENGTH"),
  SFVARN(HuCPU.bmt_alternate, "IBM_ALTERNATE"),

  SFVARN(HuCPU.timestamp, "timestamp"),
  SFVARN(HuCPU.timer_next_timestamp, "timer_next_timestamp"),
  SFVARN(HuCPU.previous_next_user_event, "previous_next_user_event"),

  SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, SFCPU, "CPU");

 if(load)
 {
  // Update MPR cache
  HuC6280_FlushMPRCache();

  // This must be after the MPR cache is updated:
  SetPC_EXTERNAL(tmp_PC);
 }

 EXPAND_FLAGS();
 #undef P_local

 return(ret);
}


};
