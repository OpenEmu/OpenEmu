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

#include "nes.h"
#include "x6502.h"
#include "sound.h"
#include "debug.h"

X6502 X;

#ifdef WANT_DEBUGGER
void (*X6502_Run)(int32 cycles);
#endif

uint32 timestamp;
void (*MapIRQHook)(int a);

#define _PC              X.PC
#define _A               X.A
#define _X               X.X
#define _Y               X.Y
#define _S               X.S
#define _P               X.P
#define _PI              X.mooPI
#define _DB              X.DB
#define _count           X.count
#define _tcount          X.tcount
#define _IRQlow          X.IRQlow
#define _jammed          X.jammed

#define ADDCYC(x)       \
{       \
 int __x=x;     \
 _tcount+=__x;  \
 _count-=__x*48;        \
 timestamp+=__x;        \
}

extern uint8 *Page[32];

static ALWAYS_INLINE uint8 RdMemNorm(unsigned int A)
{
 return(_DB=ARead[A](A));
}

static ALWAYS_INLINE void WrMemNorm(unsigned int A, uint8 V)
{
 BWrite[A](A,V);
}

#ifdef WANT_DEBUGGER
X6502 XSave;     /* This is getting ugly. */
//#define RdMemHook(A)	( X.ReadHook?(_DB=X.ReadHook(&X,A)):(_DB=ARead[A](A)) )
//#define WrMemHook(A,V)	{ if(X.WriteHook) X.WriteHook(&X,A,V); else BWrite[A](A,V); }

static ALWAYS_INLINE uint8 RdMemHook(unsigned int A)
{
 if(X.ReadHook)
  return(_DB = X.ReadHook(&X,A) );
 else
  return(_DB=ARead[A](A));
}
 
static ALWAYS_INLINE void WrMemHook(unsigned int A, uint8 V)
{
 if(X.WriteHook)
  X.WriteHook(&X,A,V);
 else if(!X.preexec)
  BWrite[A](A,V);
}
#endif

uint8 X6502_DMR(uint32 A)
{
 ADDCYC(1);
 return(X.DB=ARead[A](A));
}

void X6502_DMW(uint32 A, uint8 V)
{
 ADDCYC(1);
 BWrite[A](A,V);
}

#define PUSH(V) \
{       \
 uint8 VTMP=V;  \
 WrMem(0x100+_S,VTMP);  \
 _S--;  \
}       

#define POP() RdMem(0x100+(++_S))

static uint8 ZNTable[256];
/* Some of these operations will only make sense if you know what the flag
   constants are. */

#define X_ZN(zort)      _P&=~(Z_FLAG|N_FLAG);_P|=ZNTable[zort]
#define X_ZNT(zort)	_P|=ZNTable[zort]

#define JR(cond);	\
{		\
 if(cond)	\
 {	\
  uint32 tmp;	\
  int32 disp;	\
  disp=(int8)RdMem(_PC);	\
  _PC++;	\
  ADDCYC(1);	\
  tmp=_PC;	\
  _PC+=disp;	\
  if((tmp^_PC)&0x100)   \
  ADDCYC(1);    \
 }	\
 else _PC++;	\
 ADDBT(_PC);	\
}


#define LDA	   _A=x;X_ZN(_A)
#define LDX	   _X=x;X_ZN(_X)
#define LDY        _Y=x;X_ZN(_Y)

/*  All of the freaky arithmetic operations. */
#define AND        _A&=x;X_ZN(_A)
#define BIT        _P&=~(Z_FLAG|V_FLAG|N_FLAG);_P|=ZNTable[x&_A]&Z_FLAG;_P|=x&(V_FLAG|N_FLAG)
#define EOR        _A^=x;X_ZN(_A)
#define ORA        _A|=x;X_ZN(_A)

#define ADC  {	\
	      uint32 l=_A+x+(_P&1);	\
	      _P&=~(Z_FLAG|C_FLAG|N_FLAG|V_FLAG);	\
              _P|=((((_A^x)&0x80)^0x80) & ((_A^l)&0x80))>>1;	\
              _P|=(l>>8)&C_FLAG;	\
	      _A=l;	\
	      X_ZNT(_A);	\
	     }

#define SBC  {	\
	      uint32 l=_A-x-((_P&1)^1);	\
	      _P&=~(Z_FLAG|C_FLAG|N_FLAG|V_FLAG);	\
	      _P|=((_A^l)&(_A^x)&0x80)>>1;	\
	      _P|=((l>>8)&C_FLAG)^C_FLAG;	\
	      _A=l;	\
	      X_ZNT(_A);	\
	     }

#define CMPL(a1,a2) {	\
		     uint32 t=a1-a2;	\
		     X_ZN(t&0xFF);	\
		     _P&=~C_FLAG;	\
		     _P|=((t>>8)&C_FLAG)^C_FLAG;	\
		    }

/* Special undocumented operation.  Very similar to CMP. */
#define AXS	    {	\
                     uint32 t=(_A&_X)-x;    \
                     X_ZN(t&0xFF);      \
                     _P&=~C_FLAG;       \
                     _P|=((t>>8)&C_FLAG)^C_FLAG;        \
		     _X=t;	\
                    }

#define CMP		CMPL(_A,x)
#define CPX		CMPL(_X,x)
#define CPY	      	CMPL(_Y,x)

/* The following operations modify the byte being worked on. */
#define DEC       	x--;X_ZN(x)
#define INC		x++;X_ZN(x)

#define ASL        _P&=~C_FLAG;_P|=x>>7;x<<=1;X_ZN(x)
#define LSR	_P&=~(C_FLAG|N_FLAG|Z_FLAG);_P|=x&1;x>>=1;X_ZNT(x)

/* For undocumented instructions, maybe for other things later... */
#define LSRA	_P&=~(C_FLAG|N_FLAG|Z_FLAG);_P|=_A&1;_A>>=1;X_ZNT(_A)

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
		 
/* Icky icky thing for some undocumented instructions.  Can easily be
   broken if names of local variables are changed.
*/

/* Absolute */
#define GetAB(target) 	\
{	\
 target=RdMem(_PC);	\
 _PC++;	\
 target|=RdMem(_PC)<<8;	\
 _PC++;	\
}

/* Absolute Indexed(for reads) */
#define GetABIRD(target, i)	\
{	\
 unsigned int tmp;	\
 GetAB(tmp);	\
 target=tmp;	\
 target+=i;	\
 if((target^tmp)&0x100)	\
 {	\
  RdMem(target - 0x100);	\
  ADDCYC(1);	\
 }	\
}

/* Absolute Indexed(for writes and rmws) */
#define GetABIWR(target, i)	\
{	\
 unsigned int rt;	\
 GetAB(rt);	\
 target=rt;	\
 target+=i;	\
 RdMem((target&0x00FF)|(rt&0xFF00));	\
}

/* Zero Page */
#define GetZP(target)	\
{	\
 target=RdMem(_PC); 	\
 _PC++;	\
}

/* Zero Page Indexed */
#define GetZPI(target,i)	\
{	\
 target=i+RdMem(_PC);	\
 _PC++;	\
}

/* Indexed Indirect */
#define GetIX(target)	\
{	\
 uint8 tmp;	\
 tmp=RdMem(_PC);	\
 _PC++;	\
 tmp+=_X;	\
 target=RdMem(tmp);	\
 tmp++;		\
 target|=RdMem(tmp)<<8;	\
}

/* Indirect Indexed(for reads) */
#define GetIYRD(target)	\
{	\
 unsigned int rt;	\
 uint8 tmp;	\
 tmp=RdMem(_PC);	\
 _PC++;	\
 rt=RdMem(tmp);	\
 tmp++;	\
 rt|=RdMem(tmp)<<8;	\
 target=rt;	\
 target+=_Y;	\
 if((target^rt)&0x100)	\
 {	\
  RdMem(target - 0x100);	\
  ADDCYC(1);	\
 }	\
}

/* Indirect Indexed(for writes and rmws) */
#define GetIYWR(target)	\
{	\
 unsigned int rt;	\
 uint8 tmp;	\
 tmp=RdMem(_PC);	\
 _PC++;	\
 rt=RdMem(tmp);	\
 tmp++;	\
 rt|=RdMem(tmp)<<8;	\
 target=rt;	\
 target+=_Y;	\
 RdMem((target&0x00FF)|(rt&0xFF00));	\
}

/* Now come the macros to wrap up all of the above stuff addressing mode functions
   and operation macros.  Note that operation macros will always operate(redundant
   redundant) on the variable "x".
*/

#define RMW_A(op) {uint8 x=_A; op; _A=x; break; } /* Meh... */
#define RMW_AB(op) {unsigned int A; uint8 x; GetAB(A); x=RdMem(A); WrMem(A,x); op; WrMem(A,x); break; }
#define RMW_ABI(reg,op) {unsigned int A; uint8 x; GetABIWR(A,reg); x=RdMem(A); WrMem(A,x); op; WrMem(A,x); break; }
#define RMW_ABX(op)	RMW_ABI(_X,op)
#define RMW_ABY(op)	RMW_ABI(_Y,op)
#define RMW_IX(op)  {unsigned int A; uint8 x; GetIX(A); x=RdMem(A); WrMem(A,x); op; WrMem(A,x); break; }
#define RMW_IY(op)  {unsigned int A; uint8 x; GetIYWR(A); x=RdMem(A); WrMem(A,x); op; WrMem(A,x); break; }
#define RMW_ZP(op)  {uint8 A; uint8 x; GetZP(A); x=RdMem(A); op; WrMem(A,x); break; }
#define RMW_ZPX(op) {uint8 A; uint8 x; GetZPI(A,_X); x=RdMem(A); op; WrMem(A,x); break;}

#define LD_IM(op)	{uint8 x; x=RdMem(_PC); _PC++; op; break;}
#define LD_ZP(op)	{uint8 A; uint8 x; GetZP(A); x=RdMem(A); op; break;}
#define LD_ZPX(op)  {uint8 A; uint8 x; GetZPI(A,_X); x=RdMem(A); op; break;}
#define LD_ZPY(op)  {uint8 A; uint8 x; GetZPI(A,_Y); x=RdMem(A); op; break;}
#define LD_AB(op)	{unsigned int A; uint8 x; GetAB(A); x=RdMem(A); op; break; }
#define LD_ABI(reg,op)  {unsigned int A; uint8 x; GetABIRD(A,reg); x=RdMem(A); op; break;}
#define LD_ABX(op)	LD_ABI(_X,op)
#define LD_ABY(op)	LD_ABI(_Y,op)
#define LD_IX(op)	{unsigned int A; uint8 x; GetIX(A); x=RdMem(A); op; break;}
#define LD_IY(op)	{unsigned int A; uint8 x; GetIYRD(A); x=RdMem(A); op; break;}

#define ST_ZP(r)	{uint8 A; GetZP(A); WrMem(A,r); break;}
#define ST_ZPX(r)	{uint8 A; GetZPI(A,_X); WrMem(A,r); break;}
#define ST_ZPY(r)	{uint8 A; GetZPI(A,_Y); WrMem(A,r); break;}
#define ST_AB(r)	{unsigned int A; GetAB(A); WrMem(A,r); break;}
#define ST_ABI(reg,r)	{unsigned int A; GetABIWR(A,reg); WrMem(A,r); break; }
#define ST_ABX(r)	ST_ABI(_X,r)
#define ST_ABY(r)	ST_ABI(_Y,r)
#define ST_IX(r)	{unsigned int A; GetIX(A); WrMem(A,r); break; }
#define ST_IY(r)	{unsigned int A; GetIYWR(A); WrMem(A,r); break; }

static uint8 CycTable[256] =
{                             
/*0x00*/ 7,6,2,8,3,3,5,5,3,2,2,2,4,4,6,6,
/*0x10*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x20*/ 6,6,2,8,3,3,5,5,4,2,2,2,4,4,6,6,
/*0x30*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x40*/ 6,6,2,8,3,3,5,5,3,2,2,2,3,4,6,6,
/*0x50*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x60*/ 6,6,2,8,3,3,5,5,4,2,2,2,5,4,6,6,
/*0x70*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0x80*/ 2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
/*0x90*/ 2,6,2,6,4,4,4,4,2,5,2,5,5,5,5,5,
/*0xA0*/ 2,6,2,6,3,3,3,3,2,2,2,2,4,4,4,4,
/*0xB0*/ 2,5,2,5,4,4,4,4,2,4,2,4,4,4,4,4,
/*0xC0*/ 2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
/*0xD0*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
/*0xE0*/ 2,6,2,8,3,3,5,5,2,2,2,2,4,4,6,6,
/*0xF0*/ 2,5,2,8,4,4,6,6,2,4,2,7,4,4,7,7,
};

void X6502_IRQBegin(int w)
{
 _IRQlow|=w;
}

void X6502_IRQEnd(int w)
{
 _IRQlow&=~w;
}

void TriggerNMI(void)
{
 _IRQlow|=MDFN_IQNMI;
}

void TriggerNMI2(void)
{ 
 _IRQlow|=MDFN_IQNMI2;
}

/* Called from debugger. */
void MDFNI_NMI(void)
{
 _IRQlow|=MDFN_IQNMI;
}

void MDFNI_IRQ(void)
{
 _IRQlow|=MDFN_IQTEMP;
}

void X6502_GetIVectors(uint16 *reset, uint16 *irq, uint16 *nmi)
{
 fceuindbg=1;

 *reset=RdMemNorm(0xFFFC);
 *reset|=RdMemNorm(0xFFFD)<<8;
 *nmi=RdMemNorm(0xFFFA);
 *nmi|=RdMemNorm(0xFFFB)<<8;
 *irq=RdMemNorm(0xFFFE);
 *irq|=RdMemNorm(0xFFFF)<<8;
 fceuindbg=0;
}
static int debugmode;

void X6502_Reset(void)
{
 _IRQlow=MDFN_IQRESET;
}
  
void X6502_Init(void)
{
	int x;

	memset((void *)&X,0,sizeof(X));
	for(x=0;x<256;x++)
	 if(!x) ZNTable[x]=Z_FLAG;
	 else if (x&0x80) ZNTable[x]=N_FLAG;
	 else ZNTable[x]=0;
	#ifdef WANT_DEBUGGER
	X6502_Debug(0,0,0);
	#endif
}

void X6502_Power(void)
{
 _count=_tcount=_IRQlow=_PC=_A=_X=_Y=_S=_P=_PI=_DB=_jammed=0;
 timestamp=0;
 X6502_Reset();
}

#ifdef WANT_DEBUGGER
static void X6502_RunDebug(int32 cycles)
{
	#define RdMem RdMemHook
	#define WrMem WrMemHook
        #define ADDBT(x) NESDBG_AddBranchTrace(x)

        if(PAL)
         cycles*=15;          // 15*4=60
        else
         cycles*=16;          // 16*4=64
 
        _count+=cycles;
 
	PenguinPower:
        while(_count>0)
        {
         int32 temp;   
         uint8 b1;

         if(_IRQlow && !X.cpoint) // Don't run IRQ stuff if we weren't here in a save state
         { 
          if(_IRQlow&MDFN_IQRESET)
          {
           _PC=RdMem(0xFFFC);
           _PC|=RdMem(0xFFFD)<<8;
	   ADDBT(_PC);
           _jammed=0;
           _PI=_P=I_FLAG;
           _IRQlow&=~MDFN_IQRESET;
          }
          else if(_IRQlow&MDFN_IQNMI2)
          {
           _IRQlow&=~MDFN_IQNMI2; 
           _IRQlow|=MDFN_IQNMI;
          }
          else if(_IRQlow&MDFN_IQNMI)
          {
           if(!_jammed)
           {
            ADDCYC(7);
            PUSH(_PC>>8);
            PUSH(_PC);
            PUSH((_P&~B_FLAG)|(U_FLAG));
            _P|=I_FLAG; 
            _PC=RdMem(0xFFFA); 
            _PC|=RdMem(0xFFFB)<<8;
	    ADDBT(_PC);
            _IRQlow&=~MDFN_IQNMI;
           }
          }
          else 
          { 
           if(!(_PI&I_FLAG) && !_jammed)
           {
            ADDCYC(7);
            PUSH(_PC>>8);
            PUSH(_PC);
            PUSH((_P&~B_FLAG)|(U_FLAG));
            _P|=I_FLAG;
            _PC=RdMem(0xFFFE);
            _PC|=RdMem(0xFFFF)<<8;
	    ADDBT(_PC);
           }
          }
          _IRQlow&=~(MDFN_IQTEMP);
          if(_count<=0) 
          {
           _PI=_P;
           return;
          } /* Should increase accuracy without a */
             /* major speed hit. */
         }

	 /* Ok, now the real fun starts. */
	 /* Do the pre-exec voodoo. */

         if(X.ReadHook || X.WriteHook)
	 {
	  uint32 tsave=timestamp;
          XSave=X;

	  fceuindbg=1;
	  X.preexec=1;
          b1=RdMem(_PC);
          _PC++;
          switch(b1)
          {   
           #include "ops.h"
          }  

	  timestamp=tsave;

	  X=XSave;
	  fceuindbg=0;
	 }

	 X.cpoint = 1;

	 NESDBG_TestFoundBPoint();
	 if(X.CPUHook) X.CPUHook(_PC);
	 if(!X.cpoint)
	  goto PenguinPower;
	 X.cpoint = 0;

         _PI=_P;
         b1=RdMem(_PC);
         ADDCYC(CycTable[b1]);

         temp=_tcount;
         _tcount=0;
         if(MapIRQHook) MapIRQHook(temp);

	 MDFN_SoundCPUHook(temp);

         _PC++;
         switch(b1)
         {
          #include "ops.h"
         } 
        }
        #undef RdMem
        #undef WrMem
	#undef ADDBT
}

static void X6502_RunNormal(int32 cycles)
{
#else
void X6502_Run(int32 cycles)
{
#endif
	#define RdMem RdMemNorm
	#define WrMem WrMemNorm
        #define ADDBT(x)

        uint32 pbackus;

        pbackus=_PC;

        #undef _PC
        #define _PC pbackus

	if(PAL)
	 cycles*=15;          // 15*4=60
	else
	 cycles*=16;          // 16*4=64

	_count+=cycles;
	
        // This handles the (rare) case of a save state being saved in step mode in the debugger
        if(X.cpoint && _count > 0)
        {
         X.cpoint = 0;
         goto SephirothBishie;
        }

	while(_count>0)
	{
	 int32 temp;
	 static uint8 b1;

	 if(_IRQlow)
	 {
	  if(_IRQlow&MDFN_IQRESET)
	  {
	   _PC=RdMem(0xFFFC);
	   _PC|=RdMem(0xFFFD)<<8;
	   _jammed=0;
	   _PI=_P=I_FLAG;
	   _IRQlow&=~MDFN_IQRESET;
	  }
	  else if(_IRQlow&MDFN_IQNMI2)
 	  {
	   _IRQlow&=~MDFN_IQNMI2; 
	   _IRQlow|=MDFN_IQNMI;
	  }
	  else if(_IRQlow&MDFN_IQNMI)
	  {
	   if(!_jammed)
	   {
	    ADDCYC(7);
	    PUSH(_PC>>8);
	    PUSH(_PC);
	    PUSH((_P&~B_FLAG)|(U_FLAG));
	    _P|=I_FLAG;		
	    _PC=RdMem(0xFFFA); 
	    _PC|=RdMem(0xFFFB)<<8;
	    _IRQlow&=~MDFN_IQNMI;
	   }
	  }
	  else
	  { 
	   if(!(_PI&I_FLAG) && !_jammed)
	   {
	    ADDCYC(7);
	    PUSH(_PC>>8);
	    PUSH(_PC);
	    PUSH((_P&~B_FLAG)|(U_FLAG));
	    _P|=I_FLAG;
	    _PC=RdMem(0xFFFE);
	    _PC|=RdMem(0xFFFF)<<8;
	   }
	  }
	  _IRQlow&=~(MDFN_IQTEMP);
	  if(_count<=0) 
	  {
	   _PI=_P;
	   X.PC = pbackus;
	   return;
	   } /* Should increase accuracy without a  major speed hit. */
	 }

 	 SephirothBishie:
	 _PI=_P;
	 b1=RdMem(_PC);

	 ADDCYC(CycTable[b1]);

	 temp=_tcount;
	 _tcount=0;
	 if(MapIRQHook) MapIRQHook(temp);
	 MDFN_SoundCPUHook(temp);
	 _PC++;

         switch(b1)
         {
          #include "ops.h"
         } 
	}

        #undef _PC
        #define _PC X.PC
	#undef ADDBT
        _PC=pbackus;
}

int X6502_StateAction(StateMem *sm, int load, int data_only)
{
 extern uint8 RAM[0x800]; // from nes.cpp

 SFORMAT SFCPU[]={
  SFVARN(X.PC, "PC"),
  SFVARN(X.A, "A"),
  SFVARN(X.P, "P"),
  SFVARN(X.X, "X"),
  SFVARN(X.Y, "Y"),
  SFVARN(X.S, "S"),
  SFARRAYN(RAM, 0x800, "RAM"),
  SFEND
 };

 SFORMAT SFCPUC[]={
  SFVARN(X.jammed, "JAMM"),
  SFVARN(X.IRQlow, "IQLB"),
  SFVARN(X.tcount, "ICoa"),
  SFVARN(X.count, "ICou"),
  SFVARN(timestampbase, "TSBS"),
  SFVARN(X.cpoint, "CPoint"),
  SFEND
 };


 if(load && load < 0x0603)
  X.cpoint = 0;

 std::vector <SSDescriptor> love;
 love.push_back(SSDescriptor(SFCPU, "CPU"));
 love.push_back(SSDescriptor(SFCPUC, "CPUC"));

 return(MDFNSS_StateAction(sm, load, data_only, love));
}

#ifdef WANT_DEBUGGER
void X6502_Debug(void (*CPUHook)(uint32),
		uint8 (*ReadHook)(X6502 *, unsigned int),
		void (*WriteHook)(X6502 *, unsigned int, uint8))
{
 debugmode=(ReadHook || WriteHook || CPUHook)?1:0;
 X.ReadHook=ReadHook;
 X.WriteHook=WriteHook;
 X.CPUHook=CPUHook;

 if(!debugmode)
  X6502_Run=X6502_RunNormal;
 else
  X6502_Run=X6502_RunDebug;
}
#endif
