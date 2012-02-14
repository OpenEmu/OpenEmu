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
// Mikey chip emulation class                                               //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates all of the Mikey hardware with the exception of the  //
// CPU and memory selector. Update() does most of the work and does screen  //
// DMA and counter updates, it also schecules in which cycle the next timer //
// update will occur so that the CSystem->Update() doesnt have to call it   //
// every cycle, massive speedup but big complexity headache.                //
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

#define MIKIE_CPP

//#include <crtdbg.h>
//#define	TRACE_MIKIE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "mikie.h"
#include "lynxdef.h"


void CMikie::BlowOut(void)
{
	char addr[100];
	C6502_REGS regs;
	mSystem.GetRegs(regs);
	sprintf(addr,"Runtime Error - System Halted\nCMikie::Poke() - Read/Write to counter clocks at PC=$%04x.",regs.PC);
	gError->Warning(addr);
	gSystemHalt=TRUE;
}


CMikie::CMikie(CSystem& parent)
	:mSystem(parent)
{
	TRACE_MIKIE0("CMikie()");

	mpDisplayBits=NULL;
	mpDisplayCurrent=NULL;
	mpRamPointer=NULL;

	mDisplayRotate=MIKIE_BAD_MODE;
	mDisplayFormat=MIKIE_PIXEL_FORMAT_16BPP_555;
	mpDisplayCallback=NULL;
	mDisplayCallbackObject=0;

	mUART_CABLE_PRESENT=FALSE;
	mpUART_TX_CALLBACK=NULL;

	int loop;
	for(loop=0;loop<16;loop++) mPalette[loop].Index=loop;
	for(loop=0;loop<4096;loop++) mColourMap[loop]=0;

	Reset();
}

CMikie::~CMikie()
{
	TRACE_MIKIE0("~CMikie()");
}


void CMikie::Reset(void)
{
	TRACE_MIKIE0("Reset()");

	mAudioInputComparator=FALSE;	// Initialises to unknown
	mDisplayAddress=0x00;			// Initialises to unknown
	mLynxLine=0;
	mLynxLineDMACounter=0;
	mLynxAddr=0;

	mTimerStatusFlags=0x00;		// Initialises to ZERO, i.e No IRQ's
	mTimerInterruptMask=0x00;

	mpRamPointer=mSystem.GetRamPointer();	// Fetch pointer to system RAM

	mTIM_0_BKUP=0;
	mTIM_0_ENABLE_RELOAD=0;
	mTIM_0_ENABLE_COUNT=0;
	mTIM_0_LINKING=0;
	mTIM_0_CURRENT=0;
	mTIM_0_TIMER_DONE=0;
	mTIM_0_LAST_CLOCK=0;
	mTIM_0_BORROW_IN=0;
	mTIM_0_BORROW_OUT=0;
	mTIM_0_LAST_LINK_CARRY=0;
	mTIM_0_LAST_COUNT=0;

	mTIM_1_BKUP=0;
	mTIM_1_ENABLE_RELOAD=0;
	mTIM_1_ENABLE_COUNT=0;
	mTIM_1_LINKING=0;
	mTIM_1_CURRENT=0;
	mTIM_1_TIMER_DONE=0;
	mTIM_1_LAST_CLOCK=0;
	mTIM_1_BORROW_IN=0;
	mTIM_1_BORROW_OUT=0;
	mTIM_1_LAST_LINK_CARRY=0;
	mTIM_1_LAST_COUNT=0;

	mTIM_2_BKUP=0;
	mTIM_2_ENABLE_RELOAD=0;
	mTIM_2_ENABLE_COUNT=0;
	mTIM_2_LINKING=0;
	mTIM_2_CURRENT=0;
	mTIM_2_TIMER_DONE=0;
	mTIM_2_LAST_CLOCK=0;
	mTIM_2_BORROW_IN=0;
	mTIM_2_BORROW_OUT=0;
	mTIM_2_LAST_LINK_CARRY=0;
	mTIM_2_LAST_COUNT=0;

	mTIM_3_BKUP=0;
	mTIM_3_ENABLE_RELOAD=0;
	mTIM_3_ENABLE_COUNT=0;
	mTIM_3_LINKING=0;
	mTIM_3_CURRENT=0;
	mTIM_3_TIMER_DONE=0;
	mTIM_3_LAST_CLOCK=0;
	mTIM_3_BORROW_IN=0;
	mTIM_3_BORROW_OUT=0;
	mTIM_3_LAST_LINK_CARRY=0;
	mTIM_3_LAST_COUNT=0;

	mTIM_4_BKUP=0;
	mTIM_4_ENABLE_RELOAD=0;
	mTIM_4_ENABLE_COUNT=0;
	mTIM_4_LINKING=0;
	mTIM_4_CURRENT=0;
	mTIM_4_TIMER_DONE=0;
	mTIM_4_LAST_CLOCK=0;
	mTIM_4_BORROW_IN=0;
	mTIM_4_BORROW_OUT=0;
	mTIM_4_LAST_LINK_CARRY=0;
	mTIM_4_LAST_COUNT=0;

	mTIM_5_BKUP=0;
	mTIM_5_ENABLE_RELOAD=0;
	mTIM_5_ENABLE_COUNT=0;
	mTIM_5_LINKING=0;
	mTIM_5_CURRENT=0;
	mTIM_5_TIMER_DONE=0;
	mTIM_5_LAST_CLOCK=0;
	mTIM_5_BORROW_IN=0;
	mTIM_5_BORROW_OUT=0;
	mTIM_5_LAST_LINK_CARRY=0;
	mTIM_5_LAST_COUNT=0;

	mTIM_6_BKUP=0;
	mTIM_6_ENABLE_RELOAD=0;
	mTIM_6_ENABLE_COUNT=0;
	mTIM_6_LINKING=0;
	mTIM_6_CURRENT=0;
	mTIM_6_TIMER_DONE=0;
	mTIM_6_LAST_CLOCK=0;
	mTIM_6_BORROW_IN=0;
	mTIM_6_BORROW_OUT=0;
	mTIM_6_LAST_LINK_CARRY=0;
	mTIM_6_LAST_COUNT=0;

	mTIM_7_BKUP=0;
	mTIM_7_ENABLE_RELOAD=0;
	mTIM_7_ENABLE_COUNT=0;
	mTIM_7_LINKING=0;
	mTIM_7_CURRENT=0;
	mTIM_7_TIMER_DONE=0;
	mTIM_7_LAST_CLOCK=0;
	mTIM_7_BORROW_IN=0;
	mTIM_7_BORROW_OUT=0;
	mTIM_7_LAST_LINK_CARRY=0;
	mTIM_7_LAST_COUNT=0;

	mAUDIO_0_BKUP=0;
	mAUDIO_0_ENABLE_RELOAD=0;
	mAUDIO_0_ENABLE_COUNT=0;
	mAUDIO_0_LINKING=0;
	mAUDIO_0_CURRENT=0;
	mAUDIO_0_TIMER_DONE=0;
	mAUDIO_0_LAST_CLOCK=0;
	mAUDIO_0_BORROW_IN=0;
	mAUDIO_0_BORROW_OUT=0;
	mAUDIO_0_LAST_LINK_CARRY=0;
	mAUDIO_0_LAST_COUNT=0;
	mAUDIO_0_VOLUME=0;
	mAUDIO_0_OUTPUT=0;
	mAUDIO_0_INTEGRATE_ENABLE=0;
	mAUDIO_0_WAVESHAPER=0;

	mAUDIO_1_BKUP=0;
	mAUDIO_1_ENABLE_RELOAD=0;
	mAUDIO_1_ENABLE_COUNT=0;
	mAUDIO_1_LINKING=0;
	mAUDIO_1_CURRENT=0;
	mAUDIO_1_TIMER_DONE=0;
	mAUDIO_1_LAST_CLOCK=0;
	mAUDIO_1_BORROW_IN=0;
	mAUDIO_1_BORROW_OUT=0;
	mAUDIO_1_LAST_LINK_CARRY=0;
	mAUDIO_1_LAST_COUNT=0;
	mAUDIO_1_VOLUME=0;
	mAUDIO_1_OUTPUT=0;
	mAUDIO_1_INTEGRATE_ENABLE=0;
	mAUDIO_1_WAVESHAPER=0;

	mAUDIO_2_BKUP=0;
	mAUDIO_2_ENABLE_RELOAD=0;
	mAUDIO_2_ENABLE_COUNT=0;
	mAUDIO_2_LINKING=0;
	mAUDIO_2_CURRENT=0;
	mAUDIO_2_TIMER_DONE=0;
	mAUDIO_2_LAST_CLOCK=0;
	mAUDIO_2_BORROW_IN=0;
	mAUDIO_2_BORROW_OUT=0;
	mAUDIO_2_LAST_LINK_CARRY=0;
	mAUDIO_2_LAST_COUNT=0;
	mAUDIO_2_VOLUME=0;
	mAUDIO_2_OUTPUT=0;
	mAUDIO_2_INTEGRATE_ENABLE=0;
	mAUDIO_2_WAVESHAPER=0;

	mAUDIO_3_BKUP=0;
	mAUDIO_3_ENABLE_RELOAD=0;
	mAUDIO_3_ENABLE_COUNT=0;
	mAUDIO_3_LINKING=0;
	mAUDIO_3_CURRENT=0;
	mAUDIO_3_TIMER_DONE=0;
	mAUDIO_3_LAST_CLOCK=0;
	mAUDIO_3_BORROW_IN=0;
	mAUDIO_3_BORROW_OUT=0;
	mAUDIO_3_LAST_LINK_CARRY=0;
	mAUDIO_3_LAST_COUNT=0;
	mAUDIO_3_VOLUME=0;
	mAUDIO_3_OUTPUT=0;
	mAUDIO_3_INTEGRATE_ENABLE=0;
	mAUDIO_3_WAVESHAPER=0;

	mSTEREO=0xff;	// All channels enabled

	// Start with an empty palette

	for(int loop=0;loop<16;loop++)
	{
		mPalette[loop].Index=loop;
	}

	// Initialise IODAT register

	mIODAT=0x00;
	mIODIR=0x00;
	mIODAT_REST_SIGNAL=0x00;

	//
	// Initialise display control register vars
	//
	mDISPCTL_DMAEnable=FALSE;
	mDISPCTL_Flip=FALSE;
	mDISPCTL_FourColour=0;
	mDISPCTL_Colour=0;

	//
	// Initialise the UART variables
	//
	mUART_RX_IRQ_ENABLE=0;
	mUART_TX_IRQ_ENABLE=0;

	mUART_TX_COUNTDOWN=UART_TX_INACTIVE;
	mUART_RX_COUNTDOWN=UART_RX_INACTIVE;

	mUART_Rx_input_ptr=0;
	mUART_Rx_output_ptr=0;
	mUART_Rx_waiting=0;
	mUART_Rx_framing_error=0;
	mUART_Rx_overun_error=0;

	mUART_SENDBREAK=0;
	mUART_TX_DATA=0;
	mUART_RX_DATA=0;
	mUART_RX_READY=0;

	mUART_PARITY_ENABLE=0;
	mUART_PARITY_EVEN=0;
}

ULONG CMikie::GetLfsrNext(ULONG current)
{
	// The table is built thus:
	//	Bits 0-11  LFSR					(12 Bits)
	//  Bits 12-20 Feedback switches	(9 Bits)
	//     (Order = 7,0,1,2,3,4,5,10,11)
	//  Order is mangled to make peek/poke easier as
	//  bit 7 is in a seperate register
	//
	// Total 21 bits = 2MWords @ 4 Bytes/Word = 8MB !!!!!
	//
	// If the index is a combination of Current LFSR+Feedback the
	// table will give the next value.

	static ULONG switches,lfsr,next,swloop,result;
	static ULONG switchbits[9]={7,0,1,2,3,4,5,10,11};

	switches=current>>12;
	lfsr=current&0xfff;
	result=0;
	for(swloop=0;swloop<9;swloop++)
	{
		if((switches>>swloop)&0x001) result^=(lfsr>>switchbits[swloop])&0x001;
	}
	result=(result)?0:1;
	next=(switches<<12)|((lfsr<<1)&0xffe)|result;
	return next;
}

bool CMikie::ContextSave(FILE *fp)
{	
	TRACE_MIKIE0("ContextSave()");

	if(!fprintf(fp,"CMikie::ContextSave")) return 0;

	if(!fwrite(&mDisplayAddress,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAudioInputComparator,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTimerStatusFlags,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTimerInterruptMask,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(mPalette,sizeof(TPALETTE),16,fp)) return 0;
	if(!fwrite(mColourMap,sizeof(ULONG),4096,fp)) return 0;

	if(!fwrite(&mIODAT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mIODAT_REST_SIGNAL,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mIODIR,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mDISPCTL_DMAEnable,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mDISPCTL_Flip,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mDISPCTL_FourColour,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mDISPCTL_Colour,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mTIM_0_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_0_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_0_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_0_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_0_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_0_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_0_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_0_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_0_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_0_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_0_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mTIM_1_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_1_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_1_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_1_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_1_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_1_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_1_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_1_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_1_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_1_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_1_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mTIM_2_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_2_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_2_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_2_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_2_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_2_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_2_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_2_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_2_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_2_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_2_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mTIM_3_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_3_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_3_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_3_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_3_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_3_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_3_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_3_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_3_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_3_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_3_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mTIM_4_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_4_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_4_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_4_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_4_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_4_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_4_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_4_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_4_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_4_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_4_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mTIM_5_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_5_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_5_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_5_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_5_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_5_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_5_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_5_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_5_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_5_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_5_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mTIM_6_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_6_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_6_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_6_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_6_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_6_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_6_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_6_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_6_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_6_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_6_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mTIM_7_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_7_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_7_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_7_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_7_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_7_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_7_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_7_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_7_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_7_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mTIM_7_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mAUDIO_0_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_VOLUME,sizeof(SBYTE),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_OUTPUT,sizeof(SBYTE),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_0_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mAUDIO_1_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_VOLUME,sizeof(SBYTE),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_OUTPUT,sizeof(SBYTE),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_1_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mAUDIO_2_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_VOLUME,sizeof(SBYTE),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_OUTPUT,sizeof(SBYTE),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_2_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mAUDIO_3_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_VOLUME,sizeof(SBYTE),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_OUTPUT,sizeof(SBYTE),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mAUDIO_3_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mSTEREO,sizeof(ULONG),1,fp)) return 0;

	//
	// Serial related variables
	//
	if(!fwrite(&mUART_RX_IRQ_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mUART_TX_IRQ_ENABLE,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mUART_TX_COUNTDOWN,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mUART_RX_COUNTDOWN,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mUART_SENDBREAK,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mUART_TX_DATA,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mUART_RX_DATA,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mUART_RX_READY,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mUART_PARITY_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mUART_PARITY_EVEN,sizeof(ULONG),1,fp)) return 0;

	return 1;
}

bool CMikie::ContextLoad(LSS_FILE *fp)
{
	TRACE_MIKIE0("ContextLoad()");

	char teststr[100]="XXXXXXXXXXXXXXXXXXX";
	if(!lss_read(teststr,sizeof(char),19,fp)) return 0;
	if(strcmp(teststr,"CMikie::ContextSave")!=0) return 0;

	if(!lss_read(&mDisplayAddress,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAudioInputComparator,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTimerStatusFlags,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTimerInterruptMask,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(mPalette,sizeof(TPALETTE),16,fp)) return 0;
	if(!lss_read(mColourMap,sizeof(ULONG),4096,fp)) return 0;

	if(!lss_read(&mIODAT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mIODAT_REST_SIGNAL,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mIODIR,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mDISPCTL_DMAEnable,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mDISPCTL_Flip,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mDISPCTL_FourColour,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mDISPCTL_Colour,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mTIM_0_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_0_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_0_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_0_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_0_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_0_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_0_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_0_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_0_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_0_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_0_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mTIM_1_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_1_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_1_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_1_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_1_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_1_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_1_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_1_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_1_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_1_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_1_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mTIM_2_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_2_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_2_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_2_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_2_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_2_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_2_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_2_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_2_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_2_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_2_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mTIM_3_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_3_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_3_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_3_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_3_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_3_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_3_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_3_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_3_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_3_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_3_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mTIM_4_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_4_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_4_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_4_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_4_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_4_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_4_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_4_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_4_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_4_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_4_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mTIM_5_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_5_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_5_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_5_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_5_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_5_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_5_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_5_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_5_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_5_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_5_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mTIM_6_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_6_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_6_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_6_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_6_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_6_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_6_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_6_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_6_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_6_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_6_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mTIM_7_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_7_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_7_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_7_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_7_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_7_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_7_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_7_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_7_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_7_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mTIM_7_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mAUDIO_0_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_VOLUME,sizeof(SBYTE),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_OUTPUT,sizeof(SBYTE),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_0_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mAUDIO_1_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_VOLUME,sizeof(SBYTE),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_OUTPUT,sizeof(SBYTE),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_1_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mAUDIO_2_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_VOLUME,sizeof(SBYTE),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_OUTPUT,sizeof(SBYTE),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_2_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mAUDIO_3_BKUP,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_ENABLE_RELOAD,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_ENABLE_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_LINKING,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_CURRENT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_TIMER_DONE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_LAST_CLOCK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_BORROW_IN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_BORROW_OUT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_LAST_LINK_CARRY,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_LAST_COUNT,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_VOLUME,sizeof(SBYTE),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_OUTPUT,sizeof(SBYTE),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_INTEGRATE_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mAUDIO_3_WAVESHAPER,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mSTEREO,sizeof(ULONG),1,fp)) return 0;

	//
	// Serial related variables
	//
	if(!lss_read(&mUART_RX_IRQ_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mUART_TX_IRQ_ENABLE,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mUART_TX_COUNTDOWN,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mUART_RX_COUNTDOWN,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mUART_SENDBREAK,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mUART_TX_DATA,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mUART_RX_DATA,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mUART_RX_READY,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mUART_PARITY_ENABLE,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mUART_PARITY_EVEN,sizeof(ULONG),1,fp)) return 0;
	return 1;
}

void CMikie::PresetForHomebrew(void)
{
	TRACE_MIKIE0("PresetForHomebrew()");

	//
	// After all of that nice timer init we'll start timers running as some homebrew
	// i.e LR.O doesn't bother to setup the timers

	mTIM_0_BKUP=0x9e;
	mTIM_0_ENABLE_RELOAD=TRUE;
	mTIM_0_ENABLE_COUNT=TRUE;

	mTIM_2_BKUP=0x68;
	mTIM_2_ENABLE_RELOAD=TRUE;
	mTIM_2_ENABLE_COUNT=TRUE;
	mTIM_2_LINKING=7;

	mDISPCTL_DMAEnable=TRUE;
	mDISPCTL_Flip=FALSE;
	mDISPCTL_FourColour=0;
	mDISPCTL_Colour=TRUE;
}

void CMikie::ComLynxCable(int status)
{
	mUART_CABLE_PRESENT=status;
}

void CMikie::ComLynxRxData(int data)
{
	TRACE_MIKIE1("ComLynxRxData() - Received %04x",data);
	// Copy over the data
	if(mUART_Rx_waiting<UART_MAX_RX_QUEUE)
	{
		// Trigger incoming receive IF none waiting otherwise
		// we NEVER get to receive it!!!
		if(!mUART_Rx_waiting) mUART_RX_COUNTDOWN=UART_RX_TIME_PERIOD;

		// Receive the byte
		mUART_Rx_input_queue[mUART_Rx_input_ptr]=data;
		mUART_Rx_input_ptr=(++mUART_Rx_input_ptr)%UART_MAX_RX_QUEUE;
		mUART_Rx_waiting++;
		TRACE_MIKIE2("ComLynxRxData() - input ptr=%02d waiting=%02d",mUART_Rx_input_ptr,mUART_Rx_waiting);
	}
	else
	{
		TRACE_MIKIE0("ComLynxRxData() - UART RX Overun");
	}
}

void CMikie::ComLynxTxLoopback(int data)
{
	TRACE_MIKIE1("ComLynxTxLoopback() - Received %04x",data);

	if(mUART_Rx_waiting<UART_MAX_RX_QUEUE)
	{
		// Trigger incoming receive IF none waiting otherwise
		// we NEVER get to receive it!!!
		if(!mUART_Rx_waiting) mUART_RX_COUNTDOWN=UART_RX_TIME_PERIOD;

		// Receive the byte - INSERT into front of queue
		mUART_Rx_output_ptr=(--mUART_Rx_output_ptr)%UART_MAX_RX_QUEUE;
		mUART_Rx_input_queue[mUART_Rx_output_ptr]=data;
		mUART_Rx_waiting++;
		TRACE_MIKIE2("ComLynxTxLoopback() - input ptr=%02d waiting=%02d",mUART_Rx_input_ptr,mUART_Rx_waiting);
	}
	else
	{
		TRACE_MIKIE0("ComLynxTxLoopback() - UART RX Overun");
	}
}

void CMikie::ComLynxTxCallback(void (*function)(int data,ULONG objref),ULONG objref)
{
	mpUART_TX_CALLBACK=function;
	mUART_TX_CALLBACK_OBJECT=objref;
}


void CMikie::DisplaySetAttributes(ULONG Rotate,ULONG Format,ULONG Pitch,UBYTE* (*RenderCallback)(ULONG objref),ULONG objref)
{
	mDisplayRotate=Rotate;
	mDisplayFormat=Format;
	mDisplayPitch=Pitch;
	mpDisplayCallback=RenderCallback;
	mDisplayCallbackObject=objref;

	mpDisplayCurrent=NULL;

	if(mpDisplayCallback)
	{
		mpDisplayBits=(*mpDisplayCallback)(mDisplayCallbackObject);
	}
	else
	{
		mpDisplayBits=NULL;
	}

	//
	// Calculate the colour lookup tabes for the relevant mode
	//
	TPALETTE Spot;

	switch(mDisplayFormat)
	{
		case MIKIE_PIXEL_FORMAT_8BPP:
			for(Spot.Index=0;Spot.Index<4096;Spot.Index++)
			{
				mColourMap[Spot.Index]=(Spot.Colours.Red<<4)&0xe0;
				mColourMap[Spot.Index]|=(Spot.Colours.Green<<1)&0x1c;
				mColourMap[Spot.Index]|=(Spot.Colours.Blue>>2)&0x03;
			}
			break;
		case MIKIE_PIXEL_FORMAT_16BPP_555:
			for(Spot.Index=0;Spot.Index<4096;Spot.Index++)
			{
				mColourMap[Spot.Index]=(Spot.Colours.Red<<11)&0x7c00;
				mColourMap[Spot.Index]|=(Spot.Colours.Green<<6)&0x03e0;
				mColourMap[Spot.Index]|=(Spot.Colours.Blue<<1)&0x001f;
			}
			break;
		case MIKIE_PIXEL_FORMAT_16BPP_565:
			for(Spot.Index=0;Spot.Index<4096;Spot.Index++)
			{
				mColourMap[Spot.Index]=(Spot.Colours.Red<<12)&0xf800;
				mColourMap[Spot.Index]|=(Spot.Colours.Green<<7)&0x07e0;
				mColourMap[Spot.Index]|=(Spot.Colours.Blue<<1)&0x001f;
			}
			break;
		case MIKIE_PIXEL_FORMAT_24BPP:
		case MIKIE_PIXEL_FORMAT_32BPP:
			for(Spot.Index=0;Spot.Index<4096;Spot.Index++)
			{
				mColourMap[Spot.Index]=(Spot.Colours.Red<<20)&0x00ff0000;
				mColourMap[Spot.Index]|=(Spot.Colours.Green<<12)&0x0000ff00;
				mColourMap[Spot.Index]|=(Spot.Colours.Blue<<4)&0x000000ff;
			}
			break;
		default:
			gError->Warning("CMikie::SetScreenAttributes() - Unrecognised display format");
			for(Spot.Index=0;Spot.Index<4096;Spot.Index++) mColourMap[Spot.Index]=0;
			break;
	}

	// Reset screen related counters/vars
	mTIM_0_CURRENT=0;
	mTIM_2_CURRENT=0;

	// Fix lastcount so that timer update will definately occur
	mTIM_0_LAST_COUNT-=(1<<(4+mTIM_0_LINKING))+1;
	mTIM_2_LAST_COUNT-=(1<<(4+mTIM_2_LINKING))+1;

	// Force immediate timer update
	gNextTimerEvent=gSystemCycleCount;
}


ULONG CMikie::DisplayRenderLine(void)
{
	UBYTE *bitmap_tmp=NULL;
	ULONG source,loop;
	ULONG work_done=0;

	if(!mpDisplayBits) return 0;
	if(!mpDisplayCurrent) return 0;
	if(!mDISPCTL_DMAEnable) return 0;

//	if(mLynxLine&0x80000000) return 0;

	// Set the timer interrupt flag
	if(mTimerInterruptMask&0x01)
	{
		TRACE_MIKIE0("Update() - TIMER0 IRQ Triggered (Line Timer)");
		mTimerStatusFlags|=0x01;
		gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
	}

// Logic says it should be 101 but testing on an actual lynx shows the rest
// persiod is between lines 102,101,100 with the new line being latched at
// the beginning of count==99 hence the code below !!

	// Emulate REST signal
	if(mLynxLine==mTIM_2_BKUP-2 || mLynxLine==mTIM_2_BKUP-3 || mLynxLine==mTIM_2_BKUP-4) mIODAT_REST_SIGNAL=TRUE; else mIODAT_REST_SIGNAL=FALSE;

	if(mLynxLine==(mTIM_2_BKUP-3))
	{
		if(mDISPCTL_Flip)
		{
			mLynxAddr=mDisplayAddress&0xfffc;
			mLynxAddr+=3;
		}
		else
		{
			mLynxAddr=mDisplayAddress&0xfffc;
		}
		// Trigger line rending to start
		mLynxLineDMACounter=102;
	}

	// Decrement line counter logic
	if(mLynxLine) mLynxLine--;

	// Do 102 lines, nothing more, less is OK.
	if(mLynxLineDMACounter)
	{
//		TRACE_MIKIE1("Update() - Screen DMA, line %03d",line_count);
		mLynxLineDMACounter--;

		// Cycle hit for a 80 RAM access in rendering a line
		work_done+=80*DMA_RDWR_CYC;

		// Mikie screen DMA can only see the system RAM....
		// (Step through bitmap, line at a time)

		// Assign the temporary pointer;
		bitmap_tmp=mpDisplayCurrent;

		switch(mDisplayRotate)
		{
			case MIKIE_NO_ROTATE:
				if(mDisplayFormat==MIKIE_PIXEL_FORMAT_8BPP)
				{
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=sizeof(UBYTE);
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=sizeof(UBYTE);
						}
						else
						{
							mLynxAddr++;
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=sizeof(UBYTE);
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=sizeof(UBYTE);
						}
					}
				}
				else if(mDisplayFormat==MIKIE_PIXEL_FORMAT_16BPP_555 || mDisplayFormat==MIKIE_PIXEL_FORMAT_16BPP_565)
				{
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=sizeof(UWORD);
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=sizeof(UWORD);
						}
						else
						{
							mLynxAddr++;
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=sizeof(UWORD);
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=sizeof(UWORD);
						}
					}	
				}
				else if(mDisplayFormat==MIKIE_PIXEL_FORMAT_24BPP)
				{
					ULONG pixel;
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							pixel=mColourMap[mPalette[source&0x0f].Index];
							*bitmap_tmp++=(UBYTE)pixel;	pixel>>=8;
							*bitmap_tmp++=(UBYTE)pixel;	pixel>>=8;
							*bitmap_tmp++=(UBYTE)pixel;
							pixel=mColourMap[mPalette[source>>4].Index];
							*bitmap_tmp++=(UBYTE)pixel;	pixel>>=8;
							*bitmap_tmp++=(UBYTE)pixel;	pixel>>=8;
							*bitmap_tmp++=(UBYTE)pixel;
						}
						else
						{
							mLynxAddr++;
							pixel=mColourMap[mPalette[source>>4].Index];
							*bitmap_tmp++=(UBYTE)pixel;	pixel>>=8;
							*bitmap_tmp++=(UBYTE)pixel;	pixel>>=8;
							*bitmap_tmp++=(UBYTE)pixel;
							pixel=mColourMap[mPalette[source&0x0f].Index];
							*bitmap_tmp++=(UBYTE)pixel;	pixel>>=8;
							*bitmap_tmp++=(UBYTE)pixel;	pixel>>=8;
							*bitmap_tmp++=(UBYTE)pixel;
						}
					}	
				}
				else if(mDisplayFormat==MIKIE_PIXEL_FORMAT_32BPP)
				{
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=sizeof(ULONG);
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=sizeof(ULONG);
						}
						else
						{
							mLynxAddr++;
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=sizeof(ULONG);
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=sizeof(ULONG);
						}
					}	
				}
				mpDisplayCurrent+=mDisplayPitch;
				break;
					case MIKIE_ROTATE_L:
				if(mDisplayFormat==MIKIE_PIXEL_FORMAT_8BPP)
				{
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=mDisplayPitch;
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=mDisplayPitch;
						}
						else
						{
							mLynxAddr++;
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=mDisplayPitch;
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=mDisplayPitch;
						}
					}
					mpDisplayCurrent-=sizeof(UBYTE);
				}
				else if(mDisplayFormat==MIKIE_PIXEL_FORMAT_16BPP_555 || mDisplayFormat==MIKIE_PIXEL_FORMAT_16BPP_565)
				{
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=mDisplayPitch;
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=mDisplayPitch;
						}
						else
						{
							mLynxAddr++;
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=mDisplayPitch;
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=mDisplayPitch;
						}
					}
					mpDisplayCurrent-=sizeof(UWORD);
				}
				else if(mDisplayFormat==MIKIE_PIXEL_FORMAT_24BPP)
				{
					ULONG pixel;
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							pixel=mColourMap[mPalette[source&0x0f].Index];
							*(bitmap_tmp)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+1)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+2)=(UBYTE)pixel;
							bitmap_tmp+=mDisplayPitch;
							pixel=mColourMap[mPalette[source>>4].Index];
							*(bitmap_tmp)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+1)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+2)=(UBYTE)pixel;
							bitmap_tmp+=mDisplayPitch;
						}
						else
						{
							mLynxAddr++;
							pixel=mColourMap[mPalette[source>>4].Index];
							*(bitmap_tmp)=(UBYTE)pixel;	pixel>>=8;
							*(bitmap_tmp+1)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+2)=(UBYTE)pixel;
							bitmap_tmp+=mDisplayPitch;
							pixel=mColourMap[mPalette[source&0x0f].Index];
							*(bitmap_tmp)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+1)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+2)=(UBYTE)pixel;
							bitmap_tmp+=mDisplayPitch;
						}
					}
					mpDisplayCurrent-=3;
				}
				else if(mDisplayFormat==MIKIE_PIXEL_FORMAT_32BPP)
				{
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=mDisplayPitch;
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=mDisplayPitch;
						}
						else
						{
							mLynxAddr++;
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source>>4].Index];
							bitmap_tmp+=mDisplayPitch;
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp+=mDisplayPitch;
						}
					}	
					mpDisplayCurrent-=sizeof(ULONG);
				}
				break;
			case MIKIE_ROTATE_R:
				if(mDisplayFormat==MIKIE_PIXEL_FORMAT_8BPP)
				{
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp-=mDisplayPitch;
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp-=mDisplayPitch;
						}
						else
						{
							mLynxAddr++;
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp-=mDisplayPitch;
							*(bitmap_tmp)=(UBYTE)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp-=mDisplayPitch;
						}
					}
					mpDisplayCurrent+=sizeof(UBYTE);
				}
				else if(mDisplayFormat==MIKIE_PIXEL_FORMAT_16BPP_555 || mDisplayFormat==MIKIE_PIXEL_FORMAT_16BPP_565)
				{
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp-=mDisplayPitch;
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp-=mDisplayPitch;
						}
						else
						{
							mLynxAddr++;
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source>>4].Index];
							bitmap_tmp-=mDisplayPitch;
							*((UWORD*)(bitmap_tmp))=(UWORD)mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp-=mDisplayPitch;
						}
					}
					mpDisplayCurrent+=sizeof(UWORD);
				}
				else if(mDisplayFormat==MIKIE_PIXEL_FORMAT_24BPP)
				{
					ULONG pixel;
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							pixel=mColourMap[mPalette[source&0x0f].Index];
							*(bitmap_tmp)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+1)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+2)=(UBYTE)pixel;
							bitmap_tmp-=mDisplayPitch;
							pixel=mColourMap[mPalette[source>>4].Index];
							*(bitmap_tmp)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+1)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+2)=(UBYTE)pixel;
							bitmap_tmp-=mDisplayPitch;
						}
						else
						{
							mLynxAddr++;
							pixel=mColourMap[mPalette[source>>4].Index];
							*(bitmap_tmp)=(UBYTE)pixel;	pixel>>=8;
							*(bitmap_tmp+1)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+2)=(UBYTE)pixel;
							bitmap_tmp-=mDisplayPitch;
							pixel=mColourMap[mPalette[source&0x0f].Index];
							*(bitmap_tmp)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+1)=(UBYTE)pixel; pixel>>=8;
							*(bitmap_tmp+2)=(UBYTE)pixel;
							bitmap_tmp-=mDisplayPitch;
						}
					}
					mpDisplayCurrent+=3;
				}
				else if(mDisplayFormat==MIKIE_PIXEL_FORMAT_32BPP)
				{
					for(loop=0;loop<SCREEN_WIDTH/2;loop++)
					{
						source=mpRamPointer[mLynxAddr];
						if(mDISPCTL_Flip)
						{
							mLynxAddr--;
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp-=mDisplayPitch;
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source>>4].Index];
							bitmap_tmp-=mDisplayPitch;
						}
						else
						{
							mLynxAddr++;
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source>>4].Index];
							bitmap_tmp-=mDisplayPitch;
							*((ULONG*)(bitmap_tmp))=mColourMap[mPalette[source&0x0f].Index];
							bitmap_tmp-=mDisplayPitch;
						}
					}	
					mpDisplayCurrent+=sizeof(ULONG);
				}
				break;
			default:
				break;
		}
	}
	return work_done;
}

ULONG CMikie::DisplayEndOfFrame(void)
{
	// Stop any further line rendering
	mLynxLineDMACounter=0;
	mLynxLine=mTIM_2_BKUP;

	// Set the timer status flag
	if(mTimerInterruptMask&0x04)
	{
		TRACE_MIKIE0("Update() - TIMER2 IRQ Triggered (Frame Timer)");
		mTimerStatusFlags|=0x04;
		gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
	}

//	TRACE_MIKIE0("Update() - Frame end");
	// Trigger the callback to the display sub-system to render the
	// display and fetch the new pointer to be used for the lynx
	// display buffer for the forthcoming frame
	if(mpDisplayCallback) mpDisplayBits=(*mpDisplayCallback)(mDisplayCallbackObject);

	// Reinitialise the screen buffer pointer
	// Make any necessary adjustment for rotation
	switch(mDisplayRotate)
	{
		case MIKIE_ROTATE_L:
			mpDisplayCurrent=mpDisplayBits;
			switch(mDisplayFormat)
			{
				case MIKIE_PIXEL_FORMAT_8BPP:
					mpDisplayCurrent+=1*(HANDY_SCREEN_HEIGHT-1);
					break;
				case MIKIE_PIXEL_FORMAT_16BPP_555:
				case MIKIE_PIXEL_FORMAT_16BPP_565:
					mpDisplayCurrent+=2*(HANDY_SCREEN_HEIGHT-1);
					break;
				case MIKIE_PIXEL_FORMAT_24BPP:
					mpDisplayCurrent+=3*(HANDY_SCREEN_HEIGHT-1);
					break;
				case MIKIE_PIXEL_FORMAT_32BPP:
					mpDisplayCurrent+=4*(HANDY_SCREEN_HEIGHT-1);
					break;
				default:
					break;
			}
			break;
		case MIKIE_ROTATE_R:
			mpDisplayCurrent=mpDisplayBits+(mDisplayPitch*(HANDY_SCREEN_WIDTH-1));
			break;
		case MIKIE_NO_ROTATE:
			default:
			mpDisplayCurrent=mpDisplayBits;
			break;
	}
	return 0;
}

// Peek/Poke memory handlers

void CMikie::Poke(ULONG addr,UBYTE data)
{
	switch(addr&0xff)
	{
		case (TIM0BKUP&0xff): 
			mTIM_0_BKUP=data;
			TRACE_MIKIE2("Poke(TIM0BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM1BKUP&0xff): 
			mTIM_1_BKUP=data;
			TRACE_MIKIE2("Poke(TIM1BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM2BKUP&0xff): 
			mTIM_2_BKUP=data;
			TRACE_MIKIE2("Poke(TIM2BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM3BKUP&0xff): 
			mTIM_3_BKUP=data;
			TRACE_MIKIE2("Poke(TIM3BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM4BKUP&0xff): 
			mTIM_4_BKUP=data;
			TRACE_MIKIE2("Poke(TIM4BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM5BKUP&0xff): 
			mTIM_5_BKUP=data;
			TRACE_MIKIE2("Poke(TIM5BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM6BKUP&0xff): 
			mTIM_6_BKUP=data;
			TRACE_MIKIE2("Poke(TIM6BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM7BKUP&0xff):
			mTIM_7_BKUP=data;
			TRACE_MIKIE2("Poke(TIM7BKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;


		case (TIM0CTLA&0xff):
			mTimerInterruptMask&=(0x01^0xff);
			mTimerInterruptMask|=(data&0x80)?0x01:0x00;
			mTIM_0_ENABLE_RELOAD=data&0x10;
			mTIM_0_ENABLE_COUNT=data&0x08;
			mTIM_0_LINKING=data&0x07;
			if(data&0x40) mTIM_0_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_0_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM0CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM1CTLA&0xff): 
			mTimerInterruptMask&=(0x02^0xff);
			mTimerInterruptMask|=(data&0x80)?0x02:0x00;
			mTIM_1_ENABLE_RELOAD=data&0x10;
			mTIM_1_ENABLE_COUNT=data&0x08;
			mTIM_1_LINKING=data&0x07;
			if(data&0x40) mTIM_1_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_1_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM1CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM2CTLA&0xff): 
			mTimerInterruptMask&=(0x04^0xff);
			mTimerInterruptMask|=(data&0x80)?0x04:0x00;
			mTIM_2_ENABLE_RELOAD=data&0x10;
			mTIM_2_ENABLE_COUNT=data&0x08;
			mTIM_2_LINKING=data&0x07;
			if(data&0x40) mTIM_2_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_2_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM2CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM3CTLA&0xff): 
			mTimerInterruptMask&=(0x08^0xff);
			mTimerInterruptMask|=(data&0x80)?0x08:0x00;
			mTIM_3_ENABLE_RELOAD=data&0x10;
			mTIM_3_ENABLE_COUNT=data&0x08;
			mTIM_3_LINKING=data&0x07;
			if(data&0x40) mTIM_3_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_3_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM3CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM4CTLA&0xff): 
			// Timer 4 can never generate interrupts as its timer output is used
			// to drive the UART clock generator
			mTIM_4_ENABLE_RELOAD=data&0x10;
			mTIM_4_ENABLE_COUNT=data&0x08;
			mTIM_4_LINKING=data&0x07;
			if(data&0x40) mTIM_4_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_4_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM4CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM5CTLA&0xff): 
			mTimerInterruptMask&=(0x20^0xff);
			mTimerInterruptMask|=(data&0x80)?0x20:0x00;
			mTIM_5_ENABLE_RELOAD=data&0x10;
			mTIM_5_ENABLE_COUNT=data&0x08;
			mTIM_5_LINKING=data&0x07;
			if(data&0x40) mTIM_5_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_5_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM5CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM6CTLA&0xff): 
			mTimerInterruptMask&=(0x40^0xff);
			mTimerInterruptMask|=(data&0x80)?0x40:0x00;
			mTIM_6_ENABLE_RELOAD=data&0x10;
			mTIM_6_ENABLE_COUNT=data&0x08;
			mTIM_6_LINKING=data&0x07;
			if(data&0x40) mTIM_6_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_6_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM6CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM7CTLA&0xff):
			mTimerInterruptMask&=(0x80^0xff);
			mTimerInterruptMask|=(data&0x80)?0x80:0x00;
			mTIM_7_ENABLE_RELOAD=data&0x10;
			mTIM_7_ENABLE_COUNT=data&0x08;
			mTIM_7_LINKING=data&0x07;
			if(data&0x40) mTIM_7_TIMER_DONE=0;
			if(data&0x48)
			{
				mTIM_7_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(TIM7CTLA,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;


		case (TIM0CNT&0xff): 
			mTIM_0_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM0CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM1CNT&0xff): 
			mTIM_1_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM1CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM2CNT&0xff): 
			mTIM_2_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM2CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM3CNT&0xff): 
			mTIM_3_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM3CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM4CNT&0xff): 
			mTIM_4_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM4CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM5CNT&0xff): 
			mTIM_5_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM5CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM6CNT&0xff): 
			mTIM_6_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM6CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TIM7CNT&0xff): 
			mTIM_7_CURRENT=data;
			gNextTimerEvent=gSystemCycleCount;
			TRACE_MIKIE2("Poke(TIM7CNT ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;

		case (TIM0CTLB&0xff): 
			mTIM_0_TIMER_DONE=data&0x08;
			mTIM_0_LAST_CLOCK=data&0x04;
			mTIM_0_BORROW_IN=data&0x02;
			mTIM_0_BORROW_OUT=data&0x01;
			TRACE_MIKIE2("Poke(TIM0CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
//			BlowOut();
			break;
		case (TIM1CTLB&0xff): 
			mTIM_1_TIMER_DONE=data&0x08;
			mTIM_1_LAST_CLOCK=data&0x04;
			mTIM_1_BORROW_IN=data&0x02;
			mTIM_1_BORROW_OUT=data&0x01;
			TRACE_MIKIE2("Poke(TIM1CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
//			BlowOut();
			break;
		case (TIM2CTLB&0xff): 
			mTIM_2_TIMER_DONE=data&0x08;
			mTIM_2_LAST_CLOCK=data&0x04;
			mTIM_2_BORROW_IN=data&0x02;
			mTIM_2_BORROW_OUT=data&0x01;
			TRACE_MIKIE2("Poke(TIM2CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
//			BlowOut();
			break;
		case (TIM3CTLB&0xff): 
			mTIM_3_TIMER_DONE=data&0x08;
			mTIM_3_LAST_CLOCK=data&0x04;
			mTIM_3_BORROW_IN=data&0x02;
			mTIM_3_BORROW_OUT=data&0x01;
			TRACE_MIKIE2("Poke(TIM3CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
//			BlowOut();
			break;
		case (TIM4CTLB&0xff): 
			mTIM_4_TIMER_DONE=data&0x08;
			mTIM_4_LAST_CLOCK=data&0x04;
			mTIM_4_BORROW_IN=data&0x02;
			mTIM_4_BORROW_OUT=data&0x01;
			TRACE_MIKIE2("Poke(TIM4CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
//			BlowOut();
			break;
		case (TIM5CTLB&0xff): 
			mTIM_5_TIMER_DONE=data&0x08;
			mTIM_5_LAST_CLOCK=data&0x04;
			mTIM_5_BORROW_IN=data&0x02;
			mTIM_5_BORROW_OUT=data&0x01;
			TRACE_MIKIE2("Poke(TIM5CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
//			BlowOut();
			break;
		case (TIM6CTLB&0xff): 
			mTIM_6_TIMER_DONE=data&0x08;
			mTIM_6_LAST_CLOCK=data&0x04;
			mTIM_6_BORROW_IN=data&0x02;
			mTIM_6_BORROW_OUT=data&0x01;
			TRACE_MIKIE2("Poke(TIM6CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
//			BlowOut();
			break;
		case (TIM7CTLB&0xff):
			mTIM_7_TIMER_DONE=data&0x08;
			mTIM_7_LAST_CLOCK=data&0x04;
			mTIM_7_BORROW_IN=data&0x02;
			mTIM_7_BORROW_OUT=data&0x01;
			TRACE_MIKIE2("Poke(TIM7CTLB ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
//			BlowOut();
			break;

		case (AUD0VOL&0xff): 
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if(!mAUDIO_0_VOLUME && data)
			{
				mAUDIO_0_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			mAUDIO_0_VOLUME=(SBYTE)data;
			TRACE_MIKIE2("Poke(AUD0VOL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD0SHFTFB&0xff):	
			mAUDIO_0_WAVESHAPER&=0x001fff;
			mAUDIO_0_WAVESHAPER|=(ULONG)data<<13;
			TRACE_MIKIE2("Poke(AUD0SHFTB,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD0OUTVAL&0xff): 
			mAUDIO_0_OUTPUT=data;
			TRACE_MIKIE2("Poke(AUD0OUTVAL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD0L8SHFT&0xff): 
			mAUDIO_0_WAVESHAPER&=0x1fff00;
			mAUDIO_0_WAVESHAPER|=data;
			TRACE_MIKIE2("Poke(AUD0L8SHFT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD0TBACK&0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if(!mAUDIO_0_BKUP && data)
			{
				mAUDIO_0_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			mAUDIO_0_BKUP=data;
			TRACE_MIKIE2("Poke(AUD0TBACK,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD0CTL&0xff):
			mAUDIO_0_ENABLE_RELOAD=data&0x10;
			mAUDIO_0_ENABLE_COUNT=data&0x08;
			mAUDIO_0_LINKING=data&0x07;
			mAUDIO_0_INTEGRATE_ENABLE=data&0x20;
			if(data&0x40) mAUDIO_0_TIMER_DONE=0;
			mAUDIO_0_WAVESHAPER&=0x1fefff;
			mAUDIO_0_WAVESHAPER|=(data&0x80)?0x001000:0x000000;
			if(data&0x48)
			{
				mAUDIO_0_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(AUD0CTL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD0COUNT&0xff): 
			mAUDIO_0_CURRENT=data;
			TRACE_MIKIE2("Poke(AUD0COUNT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD0MISC&0xff):
			mAUDIO_0_WAVESHAPER&=0x1ff0ff;
			mAUDIO_0_WAVESHAPER|=(data&0xf0)<<4;
			mAUDIO_0_BORROW_IN=data&0x02;
			mAUDIO_0_BORROW_OUT=data&0x01;
			mAUDIO_0_LAST_CLOCK=data&0x04;
			TRACE_MIKIE2("Poke(AUD0MISC,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		  
		case (AUD1VOL&0xff): 
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if(!mAUDIO_1_VOLUME && data)
			{
				mAUDIO_1_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			mAUDIO_1_VOLUME=(SBYTE)data;
			TRACE_MIKIE2("Poke(AUD1VOL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD1SHFTFB&0xff):	
			mAUDIO_1_WAVESHAPER&=0x001fff;
			mAUDIO_1_WAVESHAPER|=(ULONG)data<<13;
			TRACE_MIKIE2("Poke(AUD1SHFTFB,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD1OUTVAL&0xff): 
			mAUDIO_1_OUTPUT=data;
			TRACE_MIKIE2("Poke(AUD1OUTVAL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD1L8SHFT&0xff): 
			mAUDIO_1_WAVESHAPER&=0x1fff00;
			mAUDIO_1_WAVESHAPER|=data;
			TRACE_MIKIE2("Poke(AUD1L8SHFT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD1TBACK&0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if(!mAUDIO_1_BKUP && data)
			{
				mAUDIO_1_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			mAUDIO_1_BKUP=data;
			TRACE_MIKIE2("Poke(AUD1TBACK,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD1CTL&0xff):
			mAUDIO_1_ENABLE_RELOAD=data&0x10;
			mAUDIO_1_ENABLE_COUNT=data&0x08;
			mAUDIO_1_LINKING=data&0x07;
			mAUDIO_1_INTEGRATE_ENABLE=data&0x20;
			if(data&0x40) mAUDIO_1_TIMER_DONE=0;
			mAUDIO_1_WAVESHAPER&=0x1fefff;
			mAUDIO_1_WAVESHAPER|=(data&0x80)?0x001000:0x000000;
			if(data&0x48)
			{
				mAUDIO_1_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(AUD1CTL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD1COUNT&0xff): 
			mAUDIO_1_CURRENT=data;
			TRACE_MIKIE2("Poke(AUD1COUNT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD1MISC&0xff):
			mAUDIO_1_WAVESHAPER&=0x1ff0ff;
			mAUDIO_1_WAVESHAPER|=(data&0xf0)<<4;
			mAUDIO_1_BORROW_IN=data&0x02;
			mAUDIO_1_BORROW_OUT=data&0x01;
			mAUDIO_1_LAST_CLOCK=data&0x04;
			TRACE_MIKIE2("Poke(AUD1MISC,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;

		case (AUD2VOL&0xff): 
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if(!mAUDIO_2_VOLUME && data)
			{
				mAUDIO_2_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			mAUDIO_2_VOLUME=(SBYTE)data;
			TRACE_MIKIE2("Poke(AUD2VOL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD2SHFTFB&0xff):	
			mAUDIO_2_WAVESHAPER&=0x001fff;
			mAUDIO_2_WAVESHAPER|=(ULONG)data<<13;
			TRACE_MIKIE2("Poke(AUD2VSHFTFB,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD2OUTVAL&0xff): 
			mAUDIO_2_OUTPUT=data;
			TRACE_MIKIE2("Poke(AUD2OUTVAL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD2L8SHFT&0xff): 
			mAUDIO_2_WAVESHAPER&=0x1fff00;
			mAUDIO_2_WAVESHAPER|=data;
			TRACE_MIKIE2("Poke(AUD2L8SHFT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD2TBACK&0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if(!mAUDIO_2_BKUP && data)
			{
				mAUDIO_2_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			mAUDIO_2_BKUP=data;
			TRACE_MIKIE2("Poke(AUD2TBACK,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD2CTL&0xff):
			mAUDIO_2_ENABLE_RELOAD=data&0x10;
			mAUDIO_2_ENABLE_COUNT=data&0x08;
			mAUDIO_2_LINKING=data&0x07;
			mAUDIO_2_INTEGRATE_ENABLE=data&0x20;
			if(data&0x40) mAUDIO_2_TIMER_DONE=0;
			mAUDIO_2_WAVESHAPER&=0x1fefff;
			mAUDIO_2_WAVESHAPER|=(data&0x80)?0x001000:0x000000;
			if(data&0x48)
			{
				mAUDIO_2_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(AUD2CTL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD2COUNT&0xff): 
			mAUDIO_2_CURRENT=data;
			TRACE_MIKIE2("Poke(AUD2COUNT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD2MISC&0xff):
			mAUDIO_2_WAVESHAPER&=0x1ff0ff;
			mAUDIO_2_WAVESHAPER|=(data&0xf0)<<4;
			mAUDIO_2_BORROW_IN=data&0x02;
			mAUDIO_2_BORROW_OUT=data&0x01;
			mAUDIO_2_LAST_CLOCK=data&0x04;
			TRACE_MIKIE2("Poke(AUD2MISC,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;

		case (AUD3VOL&0xff): 
			// Counter is disabled when volume is zero for optimisation
			// reasons, we must update the last use position to stop problems
			if(!mAUDIO_3_VOLUME && data)
			{
				mAUDIO_3_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			mAUDIO_3_VOLUME=(SBYTE)data;
			TRACE_MIKIE2("Poke(AUD3VOL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD3SHFTFB&0xff):	
			mAUDIO_3_WAVESHAPER&=0x001fff;
			mAUDIO_3_WAVESHAPER|=(ULONG)data<<13;
			TRACE_MIKIE2("Poke(AUD3SHFTFB,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD3OUTVAL&0xff): 
			mAUDIO_3_OUTPUT=data;
			TRACE_MIKIE2("Poke(AUD3OUTVAL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD3L8SHFT&0xff): 
			mAUDIO_3_WAVESHAPER&=0x1fff00;
			mAUDIO_3_WAVESHAPER|=data;
			TRACE_MIKIE2("Poke(AUD3L8SHFT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD3TBACK&0xff):
			// Counter is disabled when backup is zero for optimisation
			// due to the fact that the output frequency will be above audio
			// range, we must update the last use position to stop problems
			if(!mAUDIO_3_BKUP && data)
			{
				mAUDIO_3_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			mAUDIO_3_BKUP=data;
			TRACE_MIKIE2("Poke(AUD3TBACK,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD3CTL&0xff):
			mAUDIO_3_ENABLE_RELOAD=data&0x10;
			mAUDIO_3_ENABLE_COUNT=data&0x08;
			mAUDIO_3_LINKING=data&0x07;
			mAUDIO_3_INTEGRATE_ENABLE=data&0x20;
			if(data&0x40) mAUDIO_3_TIMER_DONE=0;
			mAUDIO_3_WAVESHAPER&=0x1fefff;
			mAUDIO_3_WAVESHAPER|=(data&0x80)?0x001000:0x000000;
			if(data&0x48)
			{
				mAUDIO_3_LAST_COUNT=gSystemCycleCount;
				gNextTimerEvent=gSystemCycleCount;
			}
			TRACE_MIKIE2("Poke(AUD3CTL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD3COUNT&0xff): 
			mAUDIO_3_CURRENT=data;
			TRACE_MIKIE2("Poke(AUD3COUNT,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (AUD3MISC&0xff):
			mAUDIO_3_WAVESHAPER&=0x1ff0ff;
			mAUDIO_3_WAVESHAPER|=(data&0xf0)<<4;
			mAUDIO_3_BORROW_IN=data&0x02;
			mAUDIO_3_BORROW_OUT=data&0x01;
			mAUDIO_3_LAST_CLOCK=data&0x04;
			TRACE_MIKIE2("Poke(AUD3MISC,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;

		case (ATTEN_A&0xff):
		case (ATTEN_B&0xff):
		case (ATTEN_C&0xff):
		case (ATTEN_D&0xff):
		case (MPAN&0xff):
			TRACE_MIKIE2("Poke(ATTEN_A/B/C/D/MPAN,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;

		case (MSTEREO&0xff):
			data^=0xff;
//			if(!(mSTEREO&0x11) && (data&0x11))
//			{
//				mAUDIO_0_LAST_COUNT=gSystemCycleCount;
//				gNextTimerEvent=gSystemCycleCount;
//			}
//			if(!(mSTEREO&0x22) && (data&0x22))
//			{
//				mAUDIO_1_LAST_COUNT=gSystemCycleCount;
//				gNextTimerEvent=gSystemCycleCount;
//			}
//			if(!(mSTEREO&0x44) && (data&0x44))
//			{
//				mAUDIO_2_LAST_COUNT=gSystemCycleCount;
//				gNextTimerEvent=gSystemCycleCount;
//			}
//			if(!(mSTEREO&0x88) && (data&0x88))
//			{
//				mAUDIO_3_LAST_COUNT=gSystemCycleCount;
//				gNextTimerEvent=gSystemCycleCount;
//			}
			mSTEREO=data;
			TRACE_MIKIE2("Poke(MSTEREO,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;

		case (INTRST&0xff):
			data^=0xff;
			mTimerStatusFlags&=data;
			gNextTimerEvent=gSystemCycleCount;
// 22/09/06 Fix to championship rally, IRQ not getting cleared with edge triggered system
			gSystemIRQ=(mTimerStatusFlags&mTimerInterruptMask)?TRUE:FALSE;
// 22/09/06 Fix to championship rally, IRQ not getting cleared with edge triggered system
			TRACE_MIKIE2("Poke(INTRST  ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;

		case (INTSET&0xff): 
			TRACE_MIKIE2("Poke(INTSET  ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			mTimerStatusFlags|=data;
// 22/09/06 Fix to championship rally, IRQ not getting cleared with edge triggered system
			gSystemIRQ=(mTimerStatusFlags&mTimerInterruptMask)?TRUE:FALSE;
// 22/09/06 Fix to championship rally, IRQ not getting cleared with edge triggered system
			gNextTimerEvent=gSystemCycleCount;
			break;

		case (SYSCTL1&0xff):
			TRACE_MIKIE2("Poke(SYSCTL1 ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			if(!(data&0x02))
			{
        // Wii - what happens if we don't halt?
        /*
				char addr[256];
				C6502_REGS regs;
				mSystem.GetRegs(regs);
				sprintf(addr,"Runtime Alert - System Halted\nCMikie::Poke(SYSCTL1) - Lynx power down occured at PC=$%04x.\nResetting system.",regs.PC);
				gError->Warning(addr);
				mSystem.Reset();
				gSystemHalt=TRUE;
        */
			}
			mSystem.CartAddressStrobe((data&0x01)?TRUE:FALSE);
			break;

		case (MIKEYSREV&0xff):
			TRACE_MIKIE2("Poke(MIKEYSREV,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;

		case (IODIR&0xff):
			TRACE_MIKIE2("Poke(IODIR   ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			mIODIR=data;
			break;

		case (IODAT&0xff):
			TRACE_MIKIE2("Poke(IODAT   ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			mIODAT=data;
			mSystem.CartAddressData((mIODAT&0x02)?TRUE:FALSE);
			// Enable cart writes to BANK1 on AUDIN if AUDIN is set to output
			if(mIODIR&0x10) mSystem.mCart->mWriteEnableBank1=(mIODAT&0x10)?TRUE:FALSE;
			break;

		case (SERCTL&0xff): 
			TRACE_MIKIE2("Poke(SERCTL  ,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			mUART_TX_IRQ_ENABLE=(data&0x80)?true:false;
			mUART_RX_IRQ_ENABLE=(data&0x40)?true:false;
			mUART_PARITY_ENABLE=(data&0x10)?true:false;
			mUART_SENDBREAK=data&0x02;
			mUART_PARITY_EVEN=data&0x01;

			// Reset all errors if required
			if(data&0x08)
			{
				mUART_Rx_overun_error=0;
				mUART_Rx_framing_error=0;
			}

			if(mUART_SENDBREAK)
			{
				// Trigger send break, it will self sustain as long as sendbreak is set
				mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
				// Loop back what we transmitted
				ComLynxTxLoopback(UART_BREAK_CODE);
			}
			break;

		case (SERDAT&0xff):
			TRACE_MIKIE2("Poke(SERDAT ,%04x) at PC=%04x",data,mSystem.mCpu->GetPC());
			//
			// Fake transmission, set counter to be decremented by Timer 4
			//
			// ComLynx only has one output pin, hence Rx & Tx are shorted
			// therefore any transmitted data will loopback
			//
			mUART_TX_DATA=data;
			// Calculate Parity data
			if(mUART_PARITY_ENABLE)
			{
				// Calc parity value
				// Leave at zero !!
			}
			else
			{
				// If disabled then the PAREVEN bit is sent
				if(mUART_PARITY_EVEN) data|=0x0100;
			}
			// Set countdown to transmission
			mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
			// Loop back what we transmitted
			ComLynxTxLoopback(mUART_TX_DATA);
			break;

		case (SDONEACK&0xff):
			TRACE_MIKIE2("Poke(SDONEACK,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;
		case (CPUSLEEP&0xff):
			TRACE_MIKIE2("Poke(CPUSLEEP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			//
			// We must do "cycles_used" cycles of the system with the CPU sleeping
			// to compensate for the sprite painting, Mikie update will autowake the
			// CPU at the right time.
			//
			{
				TRACE_MIKIE0("*********************************************************");
				TRACE_MIKIE0("****               CPU SLEEP STARTED                 ****");
				TRACE_MIKIE0("*********************************************************");
				SLONG cycles_used=(SLONG)mSystem.PaintSprites();
				gCPUWakeupTime=gSystemCycleCount+cycles_used;
				SetCPUSleep();
				TRACE_MIKIE2("Poke(CPUSLEEP,%02x) wakeup at cycle =%012d",data,gCPUWakeupTime);
			}
			break;

		case (DISPCTL&0xff): 
			TRACE_MIKIE2("Poke(DISPCTL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			{
				TDISPCTL tmp;
				tmp.Byte=data;
				mDISPCTL_DMAEnable=tmp.Bits.DMAEnable;
				mDISPCTL_Flip=tmp.Bits.Flip;
				mDISPCTL_FourColour=tmp.Bits.FourColour;
				mDISPCTL_Colour=tmp.Bits.Colour;
			}
			break;
		case (PBKUP&0xff): 
			TRACE_MIKIE2("Poke(PBKUP,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;

		case (DISPADRL&0xff):
			TRACE_MIKIE2("Poke(DISPADRL,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			mDisplayAddress&=0xff00;
			mDisplayAddress+=data;
			break;

		case (DISPADRH&0xff): 
			TRACE_MIKIE2("Poke(DISPADRH,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			mDisplayAddress&=0x00ff;
			mDisplayAddress+=(data<<8);
			break;

		case (Mtest0&0xff): 
		case (Mtest1&0xff): 
			// Test registers are unimplemented
			// lets hope no programs use them.
			TRACE_MIKIE2("Poke(MTEST0/1,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
		case (Mtest2&0xff): 
			// Test registers are unimplemented
			// lets hope no programs use them.
			gError->Warning("CMikie::Poke() - Write to MTEST2");
			TRACE_MIKIE2("Poke(MTEST2,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			break;

		case (GREEN0&0xff): 
		case (GREEN1&0xff): 
		case (GREEN2&0xff): 
		case (GREEN3&0xff): 
		case (GREEN4&0xff): 
		case (GREEN5&0xff): 
		case (GREEN6&0xff): 
		case (GREEN7&0xff): 
		case (GREEN8&0xff): 
		case (GREEN9&0xff): 
		case (GREENA&0xff): 
		case (GREENB&0xff): 
		case (GREENC&0xff): 
		case (GREEND&0xff): 
		case (GREENE&0xff): 
		case (GREENF&0xff):
			TRACE_MIKIE2("Poke(GREENPAL0-F,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			mPalette[addr&0x0f].Colours.Green=data&0x0f;
			break;

		case (BLUERED0&0xff): 
		case (BLUERED1&0xff): 
		case (BLUERED2&0xff): 
		case (BLUERED3&0xff): 
		case (BLUERED4&0xff): 
		case (BLUERED5&0xff): 
		case (BLUERED6&0xff): 
		case (BLUERED7&0xff): 
		case (BLUERED8&0xff): 
		case (BLUERED9&0xff): 
		case (BLUEREDA&0xff): 
		case (BLUEREDB&0xff): 
		case (BLUEREDC&0xff): 
		case (BLUEREDD&0xff): 
		case (BLUEREDE&0xff): 
		case (BLUEREDF&0xff): 
			TRACE_MIKIE2("Poke(BLUEREDPAL0-F,%02x) at PC=%04x",data,mSystem.mCpu->GetPC());
			mPalette[addr&0x0f].Colours.Blue=(data&0xf0)>>4;
			mPalette[addr&0x0f].Colours.Red=data&0x0f;
			break;

// Errors on read only register accesses

		case (MAGRDY0&0xff): 
		case (MAGRDY1&0xff): 
		case (AUDIN&0xff): 
		case (MIKEYHREV&0xff): 
			TRACE_MIKIE3("Poke(%04x,%02x) - Poke to read only register location at PC=%04x",addr,data,mSystem.mCpu->GetPC());
			break;

// Errors on illegal location accesses

		default:
			TRACE_MIKIE3("Poke(%04x,%02x) - Poke to illegal location at PC=%04x",addr,data,mSystem.mCpu->GetPC());
			break;
	}
}



UBYTE CMikie::Peek(ULONG addr)
{
	switch(addr&0xff)
	{

// Timer control registers

		case (TIM0BKUP&0xff): 
			TRACE_MIKIE2("Peek(TIM0KBUP ,%02x) at PC=%04x",mTIM_0_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_0_BKUP;
			break;
		case (TIM1BKUP&0xff): 
			TRACE_MIKIE2("Peek(TIM1KBUP ,%02x) at PC=%04x",mTIM_1_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_1_BKUP;
			break;
		case (TIM2BKUP&0xff): 
			TRACE_MIKIE2("Peek(TIM2KBUP ,%02x) at PC=%04x",mTIM_2_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_2_BKUP;
			break;
		case (TIM3BKUP&0xff): 
			TRACE_MIKIE2("Peek(TIM3KBUP ,%02x) at PC=%04x",mTIM_3_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_3_BKUP;
			break;
		case (TIM4BKUP&0xff): 
			TRACE_MIKIE2("Peek(TIM4KBUP ,%02x) at PC=%04x",mTIM_4_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_4_BKUP;
			break;
		case (TIM5BKUP&0xff): 
			TRACE_MIKIE2("Peek(TIM5KBUP ,%02x) at PC=%04x",mTIM_5_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_5_BKUP;
			break;
		case (TIM6BKUP&0xff): 
			TRACE_MIKIE2("Peek(TIM6KBUP ,%02x) at PC=%04x",mTIM_6_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_6_BKUP;
			break;
		case (TIM7BKUP&0xff):
			TRACE_MIKIE2("Peek(TIM7KBUP ,%02x) at PC=%04x",mTIM_7_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_7_BKUP;
			break;

		case (TIM0CTLA&0xff):
			{
				UBYTE retval=0;
				retval|=(mTimerInterruptMask&0x01)?0x80:0x00;
				retval|=(mTIM_0_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_0_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_0_LINKING;
				TRACE_MIKIE2("Peek(TIM0CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM1CTLA&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTimerInterruptMask&0x02)?0x80:0x00;
				retval|=(mTIM_1_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_1_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_1_LINKING;
				TRACE_MIKIE2("Peek(TIM1CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM2CTLA&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTimerInterruptMask&0x04)?0x80:0x00;
				retval|=(mTIM_2_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_2_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_2_LINKING;
				TRACE_MIKIE2("Peek(TIM2CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM3CTLA&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTimerInterruptMask&0x08)?0x80:0x00;
				retval|=(mTIM_3_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_3_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_3_LINKING;
				TRACE_MIKIE2("Peek(TIM3CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM4CTLA&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTimerInterruptMask&0x10)?0x80:0x00;
				retval|=(mTIM_4_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_4_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_4_LINKING;
				TRACE_MIKIE2("Peek(TIM4CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM5CTLA&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTimerInterruptMask&0x20)?0x80:0x00;
				retval|=(mTIM_5_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_5_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_5_LINKING;
				TRACE_MIKIE2("Peek(TIM5CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM6CTLA&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTimerInterruptMask&0x40)?0x80:0x00;
				retval|=(mTIM_6_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_6_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_6_LINKING;
				TRACE_MIKIE2("Peek(TIM6CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (TIM7CTLA&0xff):
			{
				UBYTE retval=0;
				retval|=(mTimerInterruptMask&0x80)?0x80:0x00;
				retval|=(mTIM_7_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mTIM_7_ENABLE_COUNT)?0x08:0x00;
				retval|=mTIM_7_LINKING;
				TRACE_MIKIE2("Peek(TIM7CTLA ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;

		case (TIM0CNT&0xff): 
			Update();
			TRACE_MIKIE2("Peek(TIM0CNT  ,%02x) at PC=%04x",mTIM_0_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_0_CURRENT;
			break;
		case (TIM1CNT&0xff): 
			Update();
			TRACE_MIKIE2("Peek(TIM1CNT  ,%02x) at PC=%04x",mTIM_1_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_1_CURRENT;
			break;
		case (TIM2CNT&0xff): 
			Update();
			TRACE_MIKIE2("Peek(TIM2CNT  ,%02x) at PC=%04x",mTIM_2_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_2_CURRENT;
			break;
		case (TIM3CNT&0xff): 
			Update();
			TRACE_MIKIE2("Peek(TIM3CNT  ,%02x) at PC=%04x",mTIM_3_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_3_CURRENT;
			break;
		case (TIM4CNT&0xff): 
			Update();
			TRACE_MIKIE2("Peek(TIM4CNT  ,%02x) at PC=%04x",mTIM_4_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_4_CURRENT;
			break;
		case (TIM5CNT&0xff): 
			Update();
			TRACE_MIKIE2("Peek(TIM5CNT  ,%02x) at PC=%04x",mTIM_5_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_5_CURRENT;
			break;
		case (TIM6CNT&0xff): 
			Update();
			TRACE_MIKIE2("Peek(TIM6CNT  ,%02x) at PC=%04x",mTIM_6_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_6_CURRENT;
			break;
		case (TIM7CNT&0xff): 
			Update();
			TRACE_MIKIE2("Peek(TIM7CNT  ,%02x) at PC=%04x",mTIM_7_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mTIM_7_CURRENT;
			break;

		case (TIM0CTLB&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTIM_0_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_0_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_0_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_0_BORROW_OUT)?0x01:0x00;
				TRACE_MIKIE2("Peek(TIM0CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
//			BlowOut();
			break;
		case (TIM1CTLB&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTIM_1_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_1_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_1_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_1_BORROW_OUT)?0x01:0x00;
				TRACE_MIKIE2("Peek(TIM1CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
//			BlowOut();
			break;
		case (TIM2CTLB&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTIM_2_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_2_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_2_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_2_BORROW_OUT)?0x01:0x00;
				TRACE_MIKIE2("Peek(TIM2CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
//			BlowOut();
			break;
		case (TIM3CTLB&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTIM_3_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_3_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_3_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_3_BORROW_OUT)?0x01:0x00;
				TRACE_MIKIE2("Peek(TIM3CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
//			BlowOut();
			break;
		case (TIM4CTLB&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTIM_4_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_4_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_4_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_4_BORROW_OUT)?0x01:0x00;
				TRACE_MIKIE2("Peek(TIM4CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
//			BlowOut();
			break;
		case (TIM5CTLB&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTIM_5_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_5_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_5_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_5_BORROW_OUT)?0x01:0x00;
				TRACE_MIKIE2("Peek(TIM5CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
//			BlowOut();
			break;
		case (TIM6CTLB&0xff): 
			{
				UBYTE retval=0;
				retval|=(mTIM_6_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_6_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_6_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_6_BORROW_OUT)?0x01:0x00;
				TRACE_MIKIE2("Peek(TIM6CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
//			BlowOut();
			break;
		case (TIM7CTLB&0xff):
			{
				UBYTE retval=0;
				retval|=(mTIM_7_TIMER_DONE)?0x08:0x00;
				retval|=(mTIM_7_LAST_CLOCK)?0x04:0x00;
				retval|=(mTIM_7_BORROW_IN)?0x02:0x00;
				retval|=(mTIM_7_BORROW_OUT)?0x01:0x00;
				TRACE_MIKIE2("Peek(TIM7CTLB ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
//			BlowOut();
			break;

// Audio control registers

		case (AUD0VOL&0xff):
			TRACE_MIKIE2("Peek(AUD0VOL,%02x) at PC=%04x",(UBYTE)mAUDIO_0_VOLUME,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_0_VOLUME;
			break;
		case (AUD0SHFTFB&0xff):
			TRACE_MIKIE2("Peek(AUD0SHFTFB,%02x) at PC=%04x",(UBYTE)(mAUDIO_0_WAVESHAPER>>13)&0xff,mSystem.mCpu->GetPC());
			return (UBYTE)((mAUDIO_0_WAVESHAPER>>13)&0xff);
			break;
		case (AUD0OUTVAL&0xff): 
			TRACE_MIKIE2("Peek(AUD0OUTVAL,%02x) at PC=%04x",(UBYTE)mAUDIO_0_OUTPUT,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_0_OUTPUT;
			break;
		case (AUD0L8SHFT&0xff):
			TRACE_MIKIE2("Peek(AUD0L8SHFT,%02x) at PC=%04x",(UBYTE)(mAUDIO_0_WAVESHAPER&0xff),mSystem.mCpu->GetPC());
			return (UBYTE)(mAUDIO_0_WAVESHAPER&0xff);
			break;
		case (AUD0TBACK&0xff): 
			TRACE_MIKIE2("Peek(AUD0TBACK,%02x) at PC=%04x",(UBYTE)mAUDIO_0_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_0_BKUP;
			break;
		case (AUD0CTL&0xff):
			{
				UBYTE retval=0;
				retval|=(mAUDIO_0_INTEGRATE_ENABLE)?0x20:0x00;
				retval|=(mAUDIO_0_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mAUDIO_0_ENABLE_COUNT)?0x08:0x00;
				retval|=(mAUDIO_0_WAVESHAPER&0x001000)?0x80:0x00;
				retval|=mAUDIO_0_LINKING;
				TRACE_MIKIE2("Peek(AUD0CTL,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (AUD0COUNT&0xff): 
			TRACE_MIKIE2("Peek(AUD0COUNT,%02x) at PC=%04x",(UBYTE)mAUDIO_0_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_0_CURRENT;
			break;
		case (AUD0MISC&0xff): 
			{
				UBYTE retval=0;
				retval|=(mAUDIO_0_BORROW_OUT)?0x01:0x00;
				retval|=(mAUDIO_0_BORROW_IN)?0x02:0x00;
				retval|=(mAUDIO_0_LAST_CLOCK)?0x08:0x00;
				retval|=(mAUDIO_0_WAVESHAPER>>4)&0xf0;
				TRACE_MIKIE2("Peek(AUD0MISC,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;

		case (AUD1VOL&0xff):
			TRACE_MIKIE2("Peek(AUD1VOL,%02x) at PC=%04x",(UBYTE)mAUDIO_1_VOLUME,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_1_VOLUME;
			break;
		case (AUD1SHFTFB&0xff):
			TRACE_MIKIE2("Peek(AUD1SHFTFB,%02x) at PC=%04x",(UBYTE)(mAUDIO_1_WAVESHAPER>>13)&0xff,mSystem.mCpu->GetPC());
			return (UBYTE)((mAUDIO_1_WAVESHAPER>>13)&0xff);
			break;
		case (AUD1OUTVAL&0xff): 
			TRACE_MIKIE2("Peek(AUD1OUTVAL,%02x) at PC=%04x",(UBYTE)mAUDIO_1_OUTPUT,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_1_OUTPUT;
			break;
		case (AUD1L8SHFT&0xff):
			TRACE_MIKIE2("Peek(AUD1L8SHFT,%02x) at PC=%04x",(UBYTE)(mAUDIO_1_WAVESHAPER&0xff),mSystem.mCpu->GetPC());
			return (UBYTE)(mAUDIO_1_WAVESHAPER&0xff);
			break;
		case (AUD1TBACK&0xff): 
			TRACE_MIKIE2("Peek(AUD1TBACK,%02x) at PC=%04x",(UBYTE)mAUDIO_1_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_1_BKUP;
			break;
		case (AUD1CTL&0xff):
			{
				UBYTE retval=0;
				retval|=(mAUDIO_1_INTEGRATE_ENABLE)?0x20:0x00;
				retval|=(mAUDIO_1_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mAUDIO_1_ENABLE_COUNT)?0x08:0x00;
				retval|=(mAUDIO_1_WAVESHAPER&0x001000)?0x80:0x00;
				retval|=mAUDIO_1_LINKING;
				TRACE_MIKIE2("Peek(AUD1CTL,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (AUD1COUNT&0xff): 
			TRACE_MIKIE2("Peek(AUD1COUNT,%02x) at PC=%04x",(UBYTE)mAUDIO_1_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_1_CURRENT;
			break;
		case (AUD1MISC&0xff): 
			{
				UBYTE retval=0;
				retval|=(mAUDIO_1_BORROW_OUT)?0x01:0x00;
				retval|=(mAUDIO_1_BORROW_IN)?0x02:0x00;
				retval|=(mAUDIO_1_LAST_CLOCK)?0x08:0x00;
				retval|=(mAUDIO_1_WAVESHAPER>>4)&0xf0;
				TRACE_MIKIE2("Peek(AUD1MISC,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;

		case (AUD2VOL&0xff):
			TRACE_MIKIE2("Peek(AUD2VOL,%02x) at PC=%04x",(UBYTE)mAUDIO_2_VOLUME,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_2_VOLUME;
			break;
		case (AUD2SHFTFB&0xff):
			TRACE_MIKIE2("Peek(AUD2SHFTFB,%02x) at PC=%04x",(UBYTE)(mAUDIO_2_WAVESHAPER>>13)&0xff,mSystem.mCpu->GetPC());
			return (UBYTE)((mAUDIO_2_WAVESHAPER>>13)&0xff);
			break;
		case (AUD2OUTVAL&0xff): 
			TRACE_MIKIE2("Peek(AUD2OUTVAL,%02x) at PC=%04x",(UBYTE)mAUDIO_2_OUTPUT,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_2_OUTPUT;
			break;
		case (AUD2L8SHFT&0xff):
			TRACE_MIKIE2("Peek(AUD2L8SHFT,%02x) at PC=%04x",(UBYTE)(mAUDIO_2_WAVESHAPER&0xff),mSystem.mCpu->GetPC());
			return (UBYTE)(mAUDIO_2_WAVESHAPER&0xff);
			break;
		case (AUD2TBACK&0xff): 
			TRACE_MIKIE2("Peek(AUD2TBACK,%02x) at PC=%04x",(UBYTE)mAUDIO_2_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_2_BKUP;
			break;
		case (AUD2CTL&0xff):
			{
				UBYTE retval=0;
				retval|=(mAUDIO_2_INTEGRATE_ENABLE)?0x20:0x00;
				retval|=(mAUDIO_2_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mAUDIO_2_ENABLE_COUNT)?0x08:0x00;
				retval|=(mAUDIO_2_WAVESHAPER&0x001000)?0x80:0x00;
				retval|=mAUDIO_2_LINKING;
				TRACE_MIKIE2("Peek(AUD2CTL,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (AUD2COUNT&0xff): 
			TRACE_MIKIE2("Peek(AUD2COUNT,%02x) at PC=%04x",(UBYTE)mAUDIO_2_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_2_CURRENT;
			break;
		case (AUD2MISC&0xff): 
			{
				UBYTE retval=0;
				retval|=(mAUDIO_2_BORROW_OUT)?0x01:0x00;
				retval|=(mAUDIO_2_BORROW_IN)?0x02:0x00;
				retval|=(mAUDIO_2_LAST_CLOCK)?0x08:0x00;
				retval|=(mAUDIO_2_WAVESHAPER>>4)&0xf0;
				TRACE_MIKIE2("Peek(AUD2MISC,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;

		case (AUD3VOL&0xff):
			TRACE_MIKIE2("Peek(AUD3VOL,%02x) at PC=%04x",(UBYTE)mAUDIO_3_VOLUME,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_3_VOLUME;
			break;
		case (AUD3SHFTFB&0xff):
			TRACE_MIKIE2("Peek(AUD3SHFTFB,%02x) at PC=%04x",(UBYTE)(mAUDIO_3_WAVESHAPER>>13)&0xff,mSystem.mCpu->GetPC());
			return (UBYTE)((mAUDIO_3_WAVESHAPER>>13)&0xff);
			break;
		case (AUD3OUTVAL&0xff): 
			TRACE_MIKIE2("Peek(AUD3OUTVAL,%02x) at PC=%04x",(UBYTE)mAUDIO_3_OUTPUT,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_3_OUTPUT;
			break;
		case (AUD3L8SHFT&0xff):
			TRACE_MIKIE2("Peek(AUD3L8SHFT,%02x) at PC=%04x",(UBYTE)(mAUDIO_3_WAVESHAPER&0xff),mSystem.mCpu->GetPC());
			return (UBYTE)(mAUDIO_3_WAVESHAPER&0xff);
			break;
		case (AUD3TBACK&0xff): 
			TRACE_MIKIE2("Peek(AUD3TBACK,%02x) at PC=%04x",(UBYTE)mAUDIO_3_BKUP,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_3_BKUP;
			break;
		case (AUD3CTL&0xff):
			{
				UBYTE retval=0;
				retval|=(mAUDIO_3_INTEGRATE_ENABLE)?0x20:0x00;
				retval|=(mAUDIO_3_ENABLE_RELOAD)?0x10:0x00;
				retval|=(mAUDIO_3_ENABLE_COUNT)?0x08:0x00;
				retval|=(mAUDIO_3_WAVESHAPER&0x001000)?0x80:0x00;
				retval|=mAUDIO_3_LINKING;
				TRACE_MIKIE2("Peek(AUD3CTL,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;
		case (AUD3COUNT&0xff): 
			TRACE_MIKIE2("Peek(AUD3COUNT,%02x) at PC=%04x",(UBYTE)mAUDIO_3_CURRENT,mSystem.mCpu->GetPC());
			return (UBYTE)mAUDIO_3_CURRENT;
			break;
		case (AUD3MISC&0xff): 
			{
				UBYTE retval=0;
				retval|=(mAUDIO_3_BORROW_OUT)?0x01:0x00;
				retval|=(mAUDIO_3_BORROW_IN)?0x02:0x00;
				retval|=(mAUDIO_3_LAST_CLOCK)?0x08:0x00;
				retval|=(mAUDIO_3_WAVESHAPER>>4)&0xf0;
				TRACE_MIKIE2("Peek(AUD3MISC,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return retval;
			}
			break;

		case (ATTEN_A&0xff):
		case (ATTEN_B&0xff):
		case (ATTEN_C&0xff):
		case (ATTEN_D&0xff):
		case (MPAN&0xff):
			TRACE_MIKIE1("Peek(ATTEN_A/B/C/D/MPAN) at PC=%04x",mSystem.mCpu->GetPC());
			break;

		case (MSTEREO&0xff):
			TRACE_MIKIE2("Peek(MSTEREO,%02x) at PC=%04x",(UBYTE)mSTEREO^0xff,mSystem.mCpu->GetPC());
			return (UBYTE) mSTEREO^0xff;
			break;

// Miscellaneous registers

		case (SERCTL&0xff): 
			{
				ULONG retval=0;
				retval|=(mUART_TX_COUNTDOWN&UART_TX_INACTIVE)?0xA0:0x00;	// Indicate TxDone & TxAllDone
				retval|=(mUART_RX_READY)?0x40:0x00;							// Indicate Rx data ready
				retval|=(mUART_Rx_overun_error)?0x08:0x0;					// Framing error
				retval|=(mUART_Rx_framing_error)?0x04:0x00;					// Rx overrun
				retval|=(mUART_RX_DATA&UART_BREAK_CODE)?0x02:0x00;			// Indicate break received
				retval|=(mUART_RX_DATA&0x0100)?0x01:0x00;					// Add parity bit
				TRACE_MIKIE2("Peek(SERCTL  ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return (UBYTE)retval;
			}
			break;

		case (SERDAT&0xff):
			mUART_RX_READY=0;
			TRACE_MIKIE2("Peek(SERDAT  ,%02x) at PC=%04x",(UBYTE)mUART_RX_DATA,mSystem.mCpu->GetPC());
			return (UBYTE)(mUART_RX_DATA&0xff);
			break;

		case (IODAT&0xff): 
			{
				ULONG retval=0;
				retval|=(mIODIR&0x10)?mIODAT&0x10:0x10;									// IODIR  = output bit : input high (eeprom write done)
				retval|=(mIODIR&0x08)?(((mIODAT&0x08)&&mIODAT_REST_SIGNAL)?0x00:0x08):0x00;									// REST   = output bit : input low
				retval|=(mIODIR&0x04)?mIODAT&0x04:((mUART_CABLE_PRESENT)?0x04:0x00);	// NOEXP  = output bit : input low
				retval|=(mIODIR&0x02)?mIODAT&0x02:0x00;									// CARTAD = output bit : input low
				retval|=(mIODIR&0x01)?mIODAT&0x01:0x01;									// EXTPW  = output bit : input high (Power connected)
				TRACE_MIKIE2("Peek(IODAT   ,%02x) at PC=%04x",retval,mSystem.mCpu->GetPC());
				return (UBYTE)retval;
			}
			break;

		case (INTRST&0xff):
		case (INTSET&0xff):
			TRACE_MIKIE2("Peek(INTSET  ,%02x) at PC=%04x",mTimerStatusFlags,mSystem.mCpu->GetPC());
			return (UBYTE)mTimerStatusFlags;
			break;

		case (MAGRDY0&0xff): 
		case (MAGRDY1&0xff): 
			TRACE_MIKIE2("Peek(MAGRDY0/1,%02x) at PC=%04x",0x00,mSystem.mCpu->GetPC());
			return 0x00;
			break;

		case (AUDIN&0xff):
//			TRACE_MIKIE2("Peek(AUDIN,%02x) at PC=%04x",mAudioInputComparator?0x80:0x00,mSystem.mCpu->GetPC());
//			if(mAudioInputComparator) return 0x80; else return 0x00;
			TRACE_MIKIE2("Peek(AUDIN,%02x) at PC=%04x",0x80,mSystem.mCpu->GetPC());
			return 0x80;
			break;

		case (MIKEYHREV&0xff): 
			TRACE_MIKIE2("Peek(MIKEYHREV,%02x) at PC=%04x",0x01,mSystem.mCpu->GetPC());
			return 0x01;
			break;

// Pallette registers

		case (GREEN0&0xff): 
		case (GREEN1&0xff): 
		case (GREEN2&0xff): 
		case (GREEN3&0xff): 
		case (GREEN4&0xff): 
		case (GREEN5&0xff): 
		case (GREEN6&0xff): 
		case (GREEN7&0xff): 
		case (GREEN8&0xff): 
		case (GREEN9&0xff): 
		case (GREENA&0xff): 
		case (GREENB&0xff): 
		case (GREENC&0xff): 
		case (GREEND&0xff): 
		case (GREENE&0xff): 
		case (GREENF&0xff):
			TRACE_MIKIE2("Peek(GREENPAL0-F,%02x) at PC=%04x",mPalette[addr&0x0f].Colours.Green,mSystem.mCpu->GetPC());
			return mPalette[addr&0x0f].Colours.Green;
			break;

		case (BLUERED0&0xff): 
		case (BLUERED1&0xff): 
		case (BLUERED2&0xff): 
		case (BLUERED3&0xff): 
		case (BLUERED4&0xff): 
		case (BLUERED5&0xff): 
		case (BLUERED6&0xff): 
		case (BLUERED7&0xff): 
		case (BLUERED8&0xff): 
		case (BLUERED9&0xff): 
		case (BLUEREDA&0xff): 
		case (BLUEREDB&0xff): 
		case (BLUEREDC&0xff): 
		case (BLUEREDD&0xff): 
		case (BLUEREDE&0xff): 
		case (BLUEREDF&0xff):
			TRACE_MIKIE2("Peek(BLUEREDPAL0-F,%02x) at PC=%04x",(mPalette[addr&0x0f].Colours.Red | (mPalette[addr&0x0f].Colours.Blue<<4)),mSystem.mCpu->GetPC());
			return (mPalette[addr&0x0f].Colours.Red | (mPalette[addr&0x0f].Colours.Blue<<4));
			break;

// Errors on write only register accesses

		// For easier debugging

		case (DISPADRL&0xff): 
			TRACE_MIKIE2("Peek(DISPADRL,%02x) at PC=%04x",(UBYTE)(mDisplayAddress&0xff),mSystem.mCpu->GetPC());
			return (UBYTE)(mDisplayAddress&0xff);
		case (DISPADRH&0xff): 
			TRACE_MIKIE2("Peek(DISPADRH,%02x) at PC=%04x",(UBYTE)(mDisplayAddress>>8)&0xff,mSystem.mCpu->GetPC());
			return (UBYTE)(mDisplayAddress>>8)&0xff;

		case (DISPCTL&0xff): 
		case (SYSCTL1&0xff):
		case (MIKEYSREV&0xff): 
		case (IODIR&0xff): 
		case (SDONEACK&0xff): 
		case (CPUSLEEP&0xff): 
		case (PBKUP&0xff): 
		case (Mtest0&0xff): 
		case (Mtest1&0xff): 
		case (Mtest2&0xff): 
			TRACE_MIKIE2("Peek(%04x) - Peek from write only register location at PC=$%04x",addr,mSystem.mCpu->GetPC());
			break;

// Register to let programs know handy is running

		case (0xfd97&0xff):
			TRACE_MIKIE2("Peek(%04x) - **** HANDY DETECT ATTEMPTED **** at PC=$%04x",addr,mSystem.mCpu->GetPC());
//			gError->Warning("EMULATOR DETECT REGISTER HAS BEEN READ");
			return 0x42;
			break;

// Errors on illegal location accesses

		default:
			TRACE_MIKIE2("Peek(%04x) - Peek from illegal location at PC=$%04x",addr,mSystem.mCpu->GetPC());
			break;
	}
	return 0xff;
}
