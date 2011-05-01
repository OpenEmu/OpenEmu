//
// Copyright (c) 2004 K. Wilkins
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//

//////////////////////////////////////////////////////////////////////////////
//                       Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                                 K. Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// 65C02 Emulation class                                                    //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates a 65C02 processor. It is interfaced to the rest of   //
// the system via the PEEK/POKE macros and a number of global variables     //
//                                                                          //
//    K. Wilkins                                                            //
// August 1997                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
// Revision History:                                                        //
// -----------------                                                        //
//                                                                          //
// 01Aug1997 KW Document header added & class documented.                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef C65C02_H
#define C65C02_H

//#include <crtdbg.h>
//#define	TRACE_CPU

#ifdef TRACE_CPU

#define TRACE_CPU0(msg)					_RPT1(_CRT_WARN,"C65C02::"msg" (Time=%012d)\n",gSystemCycleCount)
#define TRACE_CPU1(msg,arg1)			_RPT2(_CRT_WARN,"C65C02::"msg" (Time=%012d)\n",arg1,gSystemCycleCount)
#define TRACE_CPU2(msg,arg1,arg2)		_RPT3(_CRT_WARN,"C65C02::"msg" (Time=%012d)\n",arg1,arg2,gSystemCycleCount)
#define TRACE_CPU3(msg,arg1,arg2,arg3)	_RPT4(_CRT_WARN,"C65C02::"msg" (Time=%012d)\n",arg1,arg2,arg3,gSystemCycleCount)

#else

#define TRACE_CPU0(msg)
#define TRACE_CPU1(msg,arg1)
#define TRACE_CPU2(msg,arg1,arg2)
#define TRACE_CPU3(msg,arg1,arg2,arg3)

#endif

//
// Handy definitions
//

#define NMI_VECTOR	0xfffa
#define BOOT_VECTOR	0xfffc
#define IRQ_VECTOR	0xfffe

#define MAX_CPU_BREAKPOINTS	8

//
// ACCESS MACROS
//

//#define CPU_PEEK(m)				(mSystem.Peek_CPU(m))
//#define CPU_PEEKW(m)			(mSystem.PeekW_CPU(m))
//#define CPU_POKE(m1,m2)			(mSystem.Poke_CPU(m1,m2))

#define CPU_PEEK(m)				(((m<0xfc00)?mRamPointer[m]:mSystem.Peek_CPU(m)))
#define CPU_PEEKW(m)			(((m<0xfc00)?(mRamPointer[m]+(mRamPointer[m+1]<<8)):mSystem.PeekW_CPU(m)))
#define CPU_POKE(m1,m2)			{if(m1<0xfc00) mRamPointer[m1]=m2; else mSystem.Poke_CPU(m1,m2);}


enum {	illegal=0,
		accu,
		imm,
		absl,
		zp,
		zpx,
		zpy,
		absx,
		absy,
		iabsx,
		impl,
		rel,
		zrel,
		indx,
		indy,
		iabs,
		ind
};

typedef struct
{
	int PS;		// Processor status register   8 bits
	int A;		// Accumulator                 8 bits
	int X;		// X index register            8 bits
	int Y;		// Y index register            8 bits
	int SP;		// Stack Pointer               8 bits
	int Opcode;	// Instruction opcode          8 bits
	int Operand;// Intructions operand		  16 bits
	int PC;		// Program Counter            16 bits
	bool NMI;
	bool IRQ;
	bool WAIT;
}C6502_REGS;

//
// The CPU emulation macros
//
#include "c6502mak.h"
//
// The CPU emulation macros
//

class C65C02
{
	public:
		C65C02(CSystemBase& parent)
			:mSystem(parent)
		{
			TRACE_CPU0("C65C02()");
			// Compute the BCD lookup table
			for(uint16 t=0;t<256;++t)
			{
				mBCDTable[0][t]=((t >> 4) * 10) + (t & 0x0f);
				mBCDTable[1][t]=(((t % 100) / 10) << 4) | (t % 10);
			}
			Reset();
			
		}

		~C65C02()
		{
			TRACE_CPU0("~C65C02()");
		}

	public:
		inline void Reset(void)
		{
			TRACE_CPU0("Reset()");
			mRamPointer=mSystem.GetRamPointer();
			mA=0;
		    mX=0;
		    mY=0;
			mSP=0xff;
			mOpcode=0;
			mOperand=0;
			mPC=CPU_PEEKW(BOOT_VECTOR);
			mN=FALSE;
			mV=FALSE;
			mB=FALSE;
			mD=FALSE;
			mI=TRUE;
			mZ=TRUE;
			mC=FALSE;
			mIRQActive=FALSE;

			gSystemNMI=FALSE;
			gSystemIRQ=FALSE;
			gSystemCPUSleep=FALSE;
		}

                inline 	int StateAction(StateMem *sm, int load, int data_only)
                {
			uint8 mPS;
			std::vector <SSDescriptor> love;

			SFORMAT CPURegs[] =
			{
			 SFVAR(mA), SFVAR(mY), SFVAR(mX), SFVAR(mSP), SFVAR(mPS), SFVAR(mPC), SFVAR(mIRQActive),
			 SFEND
			};

			if(!load)
			{
	                        mPS=PS();
			}
			love.push_back(SSDescriptor(CPURegs, "CPU"));
			MDFNSS_StateAction(sm, load, data_only, love);
			if(load)
			{
				PS(mPS);
			}
                        return 1;
                }

	void Update(void);

//		inline void SetBreakpoint(uint32 breakpoint) {mPcBreakpoint=breakpoint;};

		INLINE void SetRegs(C6502_REGS &regs)
		{
			PS(regs.PS);
			mA=regs.A;
			mX=regs.X;
			mY=regs.Y;
			mSP=regs.SP;
			mOpcode=regs.Opcode;
			mOperand=regs.Operand;
			mPC=regs.PC;
			gSystemCPUSleep=regs.WAIT;
			gSystemNMI=regs.NMI;
			gSystemIRQ=regs.IRQ;
		}

		INLINE void GetRegs(C6502_REGS &regs)
		{
			regs.PS=PS();
			regs.A=mA;
			regs.X=mX;
			regs.Y=mY;
			regs.SP=mSP;
			regs.Opcode=mOpcode;
			regs.Operand=mOperand;
			regs.PC=mPC;
			regs.WAIT=(gSystemCPUSleep)?true:false;
			regs.NMI=(gSystemNMI)?true:false;
			regs.IRQ=(gSystemIRQ)?true:false;
		}

		inline int GetPC(void) { return mPC; }

	private:
		CSystemBase	&mSystem;

		// CPU Flags & status

		int mA;		// Accumulator                 8 bits
		int mX;		// X index register            8 bits
		int mY;		// Y index register            8 bits
		int mSP;		// Stack Pointer               8 bits
		int mOpcode;  // Instruction opcode          8 bits
		int mOperand; // Intructions operand		  16 bits
		int mPC;		// Program Counter            16 bits

		int mN;		// N flag for processor status register
		int mV;		// V flag for processor status register
		int mB;		// B flag for processor status register
		int mD;		// D flag for processor status register
		int mI;		// I flag for processor status register
		int mZ;		// Z flag for processor status register
		int mC;		// C flag for processor status register

		int mIRQActive;

		uint8 *mRamPointer;

		// Associated lookup tables

	    int mBCDTable[2][256];

	//
	// Opcode prototypes
	//

	private:

		// Answers value of the Processor Status register
		INLINE int PS(void) const
		{
			uint8 ps = 0x20;
			if(mN) ps|=0x80;
			if(mV) ps|=0x40;
			if(mB) ps|=0x10;
			if(mD) ps|=0x08;
			if(mI) ps|=0x04;
			if(mZ) ps|=0x02;
			if(mC) ps|=0x01;
			return ps;
		}


		// Change the processor flags to correspond to the given value
		INLINE void PS(int ps)
		{
			mN=ps&0x80;
			mV=ps&0x40;
			mB=ps&0x10;
			mD=ps&0x08;
			mI=ps&0x04;
			mZ=ps&0x02;
			mC=ps&0x01;
		}

};


#endif

