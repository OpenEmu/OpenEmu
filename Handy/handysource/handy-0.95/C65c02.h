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
#ifdef _LYNXDBG
	int	cpuBreakpoints[MAX_CPU_BREAKPOINTS];
#endif
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
#ifndef SDL_PATCH
		C65C02::C65C02(CSystemBase& parent)
			:mSystem(parent)
#else
		C65C02(CSystemBase& parent)
			:mSystem(parent)
#endif
		{
			TRACE_CPU0("C65C02()");
			// Compute the BCD lookup table
			for(UWORD t=0;t<256;++t)
			{
				mBCDTable[0][t]=((t >> 4) * 10) + (t & 0x0f);
				mBCDTable[1][t]=(((t % 100) / 10) << 4) | (t % 10);
			}
#ifdef _LYNXDBG
			for(int loop=0;loop<MAX_CPU_BREAKPOINTS;loop++)	mPcBreakpoints[loop]=0xfffffff;
			mDbgFlag=0;
#endif
			Reset();
			
		}

#ifndef ANSI_GCC
		C65C02::~C65C02()
#else
		~C65C02()
#endif
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
			gSystemCPUSleep_Saved=FALSE;
		}


		inline bool ContextSave(FILE *fp)
		{	
			TRACE_CPU0("ContextSave()");
			int mPS;
			mPS=PS();
			if(!fprintf(fp,"C6502::ContextSave")) return 0;
			if(!fwrite(&mA,sizeof(ULONG),1,fp)) return 0;
			if(!fwrite(&mX,sizeof(ULONG),1,fp)) return 0;
			if(!fwrite(&mY,sizeof(ULONG),1,fp)) return 0;
			if(!fwrite(&mSP,sizeof(ULONG),1,fp)) return 0;
			if(!fwrite(&mPS,sizeof(ULONG),1,fp)) return 0;
			if(!fwrite(&mPC,sizeof(ULONG),1,fp)) return 0;
			if(!fwrite(&mIRQActive,sizeof(ULONG),1,fp)) return 0;
			return 1;
		}

		inline bool ContextLoad(LSS_FILE *fp)
		{
			TRACE_CPU0("ContextLoad()");
			int mPS;
			char teststr[100]="XXXXXXXXXXXXXXXXXX";
			if(!lss_read(teststr,sizeof(char),18,fp)) return 0;
			if(strcmp(teststr,"C6502::ContextSave")!=0) return 0;
			if(!lss_read(&mA,sizeof(ULONG),1,fp)) return 0;
			if(!lss_read(&mX,sizeof(ULONG),1,fp)) return 0;
			if(!lss_read(&mY,sizeof(ULONG),1,fp)) return 0;
			if(!lss_read(&mSP,sizeof(ULONG),1,fp)) return 0;
			if(!lss_read(&mPS,sizeof(ULONG),1,fp)) return 0;
			if(!lss_read(&mPC,sizeof(ULONG),1,fp)) return 0;
			if(!lss_read(&mIRQActive,sizeof(ULONG),1,fp)) return 0;
			PS(mPS);
			return 1;
		}

		inline void Update(void)
		{
//
// NMI is currently unused by the lynx so lets save some time
//
//			Check NMI & IRQ status, prioritise NMI then IRQ
//			if(mNMI)
//			{
//				// Mark the NMI as services
//				mNMI=FALSE;
//				mProcessingInterrupt++;
//
//				// Push processor status
//				CPU_POKE(0x0100+mSP--,mPC>>8);
//				CPU_POKE(0x0100+mSP--,mPC&0x00ff);
//				CPU_POKE(0x0100+mSP--,PS());
//
//				// Pick up the new PC
//				mPC=CPU_PEEKW(NMI_VECTOR);
//			}

	if(gSystemIRQ && !mI)
	{
		TRACE_CPU1("Update() IRQ taken at PC=%04x",mPC);
		// IRQ signal clearance is handled by CMikie::Update() as this
		// is the only source of interrupts

		// Push processor status
	    PUSH(mPC>>8);
		PUSH(mPC&0xff);
		PUSH(PS()&0xef);		// Clear B flag on stack

		mI=TRUE;				// Stop further interrupts
		mD=FALSE;				// Clear decimal mode

		// Pick up the new PC
		mPC=CPU_PEEKW(IRQ_VECTOR);

		// Save the sleep state as an irq has possibly woken the processor
		gSystemCPUSleep_Saved=gSystemCPUSleep;
		gSystemCPUSleep=FALSE;

		// Log the irq entry time
		gIRQEntryCycle=gSystemCycleCount;

		// Clear the interrupt status line
		gSystemIRQ=FALSE;
	}

	//
	// If the CPU is asleep then skip to the next timer event
	//			
	if(gSystemCPUSleep) return;

	// Fetch opcode
	mOpcode=CPU_PEEK(mPC);
	TRACE_CPU2("Update() PC=$%04x, Opcode=%02x",mPC,mOpcode);
	mPC++;

	// Execute Opcode

	switch(mOpcode)
	{

//
// 0x00
//
		case 0x00:
			gSystemCycleCount+=(1+(6*CPU_RDWR_CYC));
			// IMPLIED
			xBRK();
			break;
		case 0x01:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xINDIRECT_X();
			xORA();
			break;
		case 0x02:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x03:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x04:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xZEROPAGE();
			xTSB();
			break;
		case 0x05:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xORA();
			break;
		case 0x06:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xZEROPAGE();
			xASL();
			break;
		case 0x07:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0x08:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			// IMPLIED
			xPHP();
			break;
		case 0x09:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xIMMEDIATE();
			xORA();
			break;
		case 0x0A:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xASLA();
			break;
		case 0x0B:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x0C:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xABSOLUTE();
			xTSB();
			break;
		case 0x0D:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xORA();
			break;
		case 0x0E:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xABSOLUTE();
			xASL();
			break;
		case 0x0F:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0x10
//
		case 0x10:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// RELATIVE (IN FUNCTION)
			xBPL();
			break;
		case 0x11:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT_Y();
			xORA();
			break;
		case 0x12:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT();
			xORA();
			break;
		case 0x13:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x14:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xZEROPAGE();
			xTRB();
			break;
		case 0x15:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xORA();
			break;
		case 0x16:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xASL();
			break;
		case 0x17:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0x18:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xCLC();
			break;
		case 0x19:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_Y();
			xORA();
			break;
		case 0x1A:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xINCA();
			break;
		case 0x1B:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x1C:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xABSOLUTE();
			xTRB();
			break;
		case 0x1D:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xORA();
			break;
		case 0x1E:
			gSystemCycleCount+=(1+(6*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xASL();
			break;
		case 0x1F:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0x20
//
		case 0x20:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xABSOLUTE();
			xJSR();
			break;
		case 0x21:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xINDIRECT_X();
			xAND();
			break;
		case 0x22:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x23:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x24:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xBIT();
			break;
		case 0x25:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xAND();
			break;
		case 0x26:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xZEROPAGE();
			xROL();
			break;
		case 0x27:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0x28:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			// IMPLIED
			xPLP();
			break;
		case 0x29:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			xIMMEDIATE();
			xAND();
			break;
		case 0x2A:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xROLA();
			break;
		case 0x2B:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x2C:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xBIT();
			break;
		case 0x2D:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xAND();
			break;
		case 0x2E:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xABSOLUTE();
			xROL();
			break;
		case 0x2F:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0x30
//
		case 0x30:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// RELATIVE (IN FUNCTION)
			xBMI();
			break;
		case 0x31:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT_Y();
			xAND();
			break;
		case 0x32:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT();
			xAND();
			break;
		case 0x33:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x34:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xBIT();
			break;
		case 0x35:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xAND();
			break;
		case 0x36:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xROL();
			break;
		case 0x37:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0x38:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xSEC();
			break;
		case 0x39:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_Y();
			xAND();
			break;
		case 0x3A:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xDECA();
			break;
		case 0x3B:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x3C:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xBIT();
			break;
		case 0x3D:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xAND();
			break;
		case 0x3E:
			gSystemCycleCount+=(1+(6*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xROL();
			break;
		case 0x3F:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0x40
//
		case 0x40:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			// Only clear IRQ if this is not a BRK instruction based RTI

			// B flag is on the stack cant test the flag
			int tmp;
			PULL(tmp);
			PUSH (tmp);
			if(!(tmp&0x10))
			{
				gSystemCPUSleep=gSystemCPUSleep_Saved;

				// If were in sleep mode then we need to push the 
				// wakeup counter along by the same number of cycles
				// we have used during the sleep period
				if(gSystemCPUSleep)
				{
					gCPUWakeupTime+=gSystemCycleCount-gIRQEntryCycle;
				}
			}
			// IMPLIED
			xRTI();
			break;
		case 0x41:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xINDIRECT_X();
			xEOR();
			break;
		case 0x42:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x43:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x44:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x45:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xEOR();
			break;
		case 0x46:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xZEROPAGE();
			xLSR();
			break;
		case 0x47:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0x48:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			// IMPLIED
			xPHA();
			break;
		case 0x49:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			xIMMEDIATE();
			xEOR();
			break;
		case 0x4A:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xLSRA();
			break;
		case 0x4B:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x4C:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xABSOLUTE();
			xJMP();
			break;
		case 0x4D:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xEOR();
			break;
		case 0x4E:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xABSOLUTE();
			xLSR();
			break;
		case 0x4F:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0x50
//
		case 0x50:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// RELATIVE (IN FUNCTION)
			xBVC();
			break;
		case 0x51:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT_Y();
			xEOR();
			break;
		case 0x52:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT();
			xEOR();
			break;
		case 0x53:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x54:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x55:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xEOR();
			break;
		case 0x56:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xLSR();
			break;
		case 0x57:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0x58:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xCLI();
			break;
		case 0x59:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_Y();
			xEOR();
			break;
		case 0x5A:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			// IMPLIED
			xPHY();
			break;
		case 0x5B:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x5C:
			gSystemCycleCount+=(1+(7*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x5D:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xEOR();
			break;
		case 0x5E:
			gSystemCycleCount+=(1+(6*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xLSR();
			break;
		case 0x5F:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0x60
//
		case 0x60:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			// IMPLIED
			xRTS();
			break;
		case 0x61:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xINDIRECT_X();
			xADC();
			break;
		case 0x62:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x63:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x64:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xSTZ();
			break;
		case 0x65:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xADC();
			break;
		case 0x66:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xZEROPAGE();
			xROR();
			break;
		case 0x67:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0x68:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			// IMPLIED
			xPLA();
			break;
		case 0x69:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			xIMMEDIATE();
			xADC();
			break;
		case 0x6A:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xRORA();
			break;
		case 0x6B:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x6C:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xINDIRECT_ABSOLUTE();
			xJMP();
			break;
		case 0x6D:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xADC();
			break;
		case 0x6E:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xABSOLUTE();
			xROR();
			break;
		case 0x6F:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0x70
//
		case 0x70:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// RELATIVE (IN FUNCTION)
			xBVS();
			break;
		case 0x71:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT_Y();
			xADC();
			break;
		case 0x72:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT();
			xADC();
			break;
		case 0x73:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x74:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xSTZ();
			break;
		case 0x75:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xADC();
			break;
		case 0x76:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xROR();
			break;
		case 0x77:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0x78:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xSEI();
			break;
		case 0x79:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_Y();
			xADC();
			break;
		case 0x7A:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			// IMPLIED
			xPLY();
			break;
		case 0x7B:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x7C:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xINDIRECT_ABSOLUTE_X();
			xJMP();
			break;
		case 0x7D:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xADC();
			break;
		case 0x7E:
			gSystemCycleCount+=(1+(6*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xROR();
			break;
		case 0x7F:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0x80
//
		case 0x80:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			// RELATIVE (IN FUNCTION)
			xBRA();
			break;
		case 0x81:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xINDIRECT_X();
			xSTA();
			break;
		case 0x82:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x83:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x84:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xSTY();
			break;
		case 0x85:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xSTA();
			break;
		case 0x86:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xSTX();
			break;
		case 0x87:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0x88:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xDEY();
			break;
		case 0x89:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			xIMMEDIATE();
			xBIT();
			break;
		case 0x8A:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xTXA();
			break;
		case 0x8B:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x8C:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xSTY();
			break;
		case 0x8D:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xSTA();
			break;
		case 0x8E:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xSTX();
			break;
		case 0x8F:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0x90
//
		case 0x90:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// RELATIVE (IN FUNCTION)
			xBCC();
			break;
		case 0x91:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xINDIRECT_Y();
			xSTA();
			break;
		case 0x92:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT();
			xSTA();
			break;
		case 0x93:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x94:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xSTY();
			break;
		case 0x95:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xSTA();
			break;
		case 0x96:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_Y();
			xSTX();
			break;
		case 0x97:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0x98:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xTYA();
			break;
		case 0x99:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xABSOLUTE_Y();
			xSTA();
			break;
		case 0x9A:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xTXS();
			break;
		case 0x9B:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0x9C:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xSTZ();
			break;
		case 0x9D:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xSTA();
			break;
		case 0x9E:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xSTZ();
			break;
		case 0x9F:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0xA0
//
		case 0xA0:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			xIMMEDIATE();
			xLDY();
			break;
		case 0xA1:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xINDIRECT_X();
			xLDA();
			break;
		case 0xA2:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			xIMMEDIATE();
			xLDX();
			break;
		case 0xA3:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xA4:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xLDY();
			break;
		case 0xA5:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xLDA();
			break;
		case 0xA6:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xLDX();
			break;
		case 0xA7:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0xA8:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xTAY();
			break;
		case 0xA9:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			xIMMEDIATE();
			xLDA();
			break;
		case 0xAA:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xTAX();
			break;
		case 0xAB:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xAC:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xLDY();
			break;
		case 0xAD:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xLDA();
			break;
		case 0xAE:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xLDX();
			break;
		case 0xAF:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0xB0
//
		case 0xB0:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// RELATIVE (IN FUNCTION)
			xBCS();
			break;
		case 0xB1:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT_Y();
			xLDA();
			break;
		case 0xB2:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT();
			xLDA();
			break;
		case 0xB3:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xB4:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xLDY();
			break;
		case 0xB5:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xLDA();
			break;
		case 0xB6:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_Y();
			xLDX();
			break;
		case 0xB7:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0xB8:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xCLV();
			break;
		case 0xB9:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_Y();
			xLDA();
			break;
		case 0xBA:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xTSX();
			break;
		case 0xBB:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xBC:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xLDY();
			break;
		case 0xBD:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xLDA();
			break;
		case 0xBE:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_Y();
			xLDX();
			break;
		case 0xBF:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0xC0
//
		case 0xC0:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			xIMMEDIATE();
			xCPY();
			break;
		case 0xC1:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xINDIRECT_X();
			xCMP();
			break;
		case 0xC2:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xC3:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xC4:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xCPY();
			break;
		case 0xC5:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xCMP();
			break;
		case 0xC6:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xZEROPAGE();
			xDEC();
			break;
		case 0xC7:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0xC8:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xINY();
			break;
		case 0xC9:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			xIMMEDIATE();
			xCMP();
			break;
		case 0xCA:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xDEX();
			break;
		case 0xCB:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xWAI();
			break;
		case 0xCC:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xCPY();
			break;
		case 0xCD:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xCMP();
			break;
		case 0xCE:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xABSOLUTE();
			xDEC();
			break;
		case 0xCF:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0xD0
//
		case 0xD0:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// RELATIVE (IN FUNCTION)
			xBNE();
			break;
		case 0xD1:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT_Y();
			xCMP();
			break;
		case 0xD2:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT();
			xCMP();
			break;
		case 0xD3:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xD4:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xD5:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xCMP();
			break;
		case 0xD6:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xDEC();
			break;
		case 0xD7:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0xD8:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xCLD();
			break;
		case 0xD9:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_Y();
			xCMP();
			break;
		case 0xDA:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			// IMPLIED
			xPHX();
			break;
		case 0xDB:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xSTP();
			break;
		case 0xDC:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xDD:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xCMP();
			break;
		case 0xDE:
			gSystemCycleCount+=(1+(6*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xDEC();
			break;
		case 0xDF:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0xE0
//
		case 0xE0:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			xIMMEDIATE();
			xCPX();
			break;
		case 0xE1:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xINDIRECT_X();
			xSBC();
			break;
		case 0xE2:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xE3:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xE4:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xCPX();
			break;
		case 0xE5:
			gSystemCycleCount+=(1+(2*CPU_RDWR_CYC));
			xZEROPAGE();
			xSBC();
			break;
		case 0xE6:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xZEROPAGE();
			xINC();
			break;
		case 0xE7:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0xE8:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xINX();
			break;
		case 0xE9:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			xIMMEDIATE();
			xSBC();
			break;
		case 0xEA:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xNOP();
			break;
		case 0xEB:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xEC:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xCPX();
			break;
		case 0xED:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE();
			xSBC();
			break;
		case 0xEE:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xABSOLUTE();
			xINC();
			break;
		case 0xEF:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

//
// 0xF0
//
		case 0xF0:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// RELATIVE (IN FUNCTION)
			xBEQ();
			break;
		case 0xF1:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT_Y();
			xSBC();
			break;
		case 0xF2:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			xINDIRECT();
			xSBC();
			break;
		case 0xF3:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xF4:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xF5:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xSBC();
			break;
		case 0xF6:
			gSystemCycleCount+=(1+(5*CPU_RDWR_CYC));
			xZEROPAGE_X();
			xINC();
			break;
		case 0xF7:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;

		case 0xF8:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// IMPLIED
			xSED();
			break;
		case 0xF9:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_Y();
			xSBC();
			break;
		case 0xFA:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			// IMPLIED
			xPLX();
			break;
		case 0xFB:
			gSystemCycleCount+=(1+(1*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xFC:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
		case 0xFD:
			gSystemCycleCount+=(1+(3*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xSBC();
			break;
		case 0xFE:
			gSystemCycleCount+=(1+(6*CPU_RDWR_CYC));
			xABSOLUTE_X();
			xINC();
			break;
		case 0xFF:
			gSystemCycleCount+=(1+(4*CPU_RDWR_CYC));
			// *** ILLEGAL ***
			xILLEGAL();
			break;
	}

#ifdef _LYNXDBG

	// Trigger breakpoint if required

	for(int loop=0;loop<MAX_CPU_BREAKPOINTS;loop++)
	{
		if(mPcBreakpoints[loop]==mPC)
		{
			gBreakpointHit=TRUE;
			mSystem.DebugTrace(0);
		}
	}

	// Check code level debug features
	// back to back CPX ($Absolute)
	// on the 2nd Occurance we do some debug
	if(mOpcode==0xec)
	{
		if(mDbgFlag)
		{
			// We shoud do some debug now
			if(!mOperand)
			{
				// Trigger a breakpoint
				gBreakpointHit=TRUE;
				// Generate a debug trail output
				mSystem.DebugTrace(0);
			}
			else
			{
				// Generate a debug trail output
				mSystem.DebugTrace(mOperand);
			}
			mDbgFlag=0;
		}
		else
		{
			if(mOperand==0x5aa5) mDbgFlag=1; else mDbgFlag=0;
		}
	}
	else
	{
		mDbgFlag=0;
	}
#endif
}

//		inline void SetBreakpoint(ULONG breakpoint) {mPcBreakpoint=breakpoint;};

		inline void SetRegs(C6502_REGS &regs)
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
#ifdef _LYNXDBG
			for(int loop=0;loop<MAX_CPU_BREAKPOINTS;loop++)	mPcBreakpoints[loop]=regs.cpuBreakpoints[loop];
#endif
			gSystemNMI=regs.NMI;
			gSystemIRQ=regs.IRQ;
		}

		inline void GetRegs(C6502_REGS &regs)
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
#ifdef _LYNXDBG
			for(int loop=0;loop<MAX_CPU_BREAKPOINTS;loop++)	regs.cpuBreakpoints[loop]=mPcBreakpoints[loop];
#endif
			regs.NMI=(gSystemNMI)?true:false;
			regs.IRQ=(gSystemIRQ)?true:false;
		}

		inline int GetPC(void) { return mPC; }

		inline void xILLEGAL(void)
		{
			char addr[1024];
			sprintf(addr,"C65C02::Update() - Illegal opcode (%02x) at PC=$%04x.",mOpcode,mPC);
			gError->Warning(addr);
		}

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

#ifdef _LYNXDBG
		int mPcBreakpoints[MAX_CPU_BREAKPOINTS];
		int mDbgFlag;
#endif
		UBYTE *mRamPointer;

		// Associated lookup tables

	    int mBCDTable[2][256];

	//
	// Opcode prototypes
	//

	private:

		// Answers value of the Processor Status register
		int PS() const
		{
			UBYTE ps = 0x20;
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
		void PS(int ps)
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

