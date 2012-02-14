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
// Suzy emulation class                                                     //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This class emulates the Suzy chip within the lynx. This provides math    //
// and sprite painting facilities. SpritePaint() is called from within      //
// the Mikey POKE functions when SPRGO is set and is called via the system  //
// object to keep the interface clean.                                      //
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

#define SUSIE_CPP

//#include <crtdbg.h>
//#define TRACE_SUSIE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "susie.h"
#include "lynxdef.h"

//
// As the Susie sprite engine only ever sees system RAM
// wa can access this directly without the hassle of
// going through the system object, much faster
//
//#define RAM_PEEK(m)			(mSystem.Peek_RAM((m)))
//#define RAM_POKE(m1,m2)		(mSystem.Poke_RAM((m1),(m2)))
//#define RAM_PEEKW(m)			(mSystem.PeekW_RAM((m)))

#define RAM_PEEK(m)				(mRamPointer[(m)])
#define RAM_PEEKW(m)			(mRamPointer[(m)]+(mRamPointer[(m)+1]<<8))
#define RAM_POKE(m1,m2)			{mRamPointer[(m1)]=(m2);}

ULONG cycles_used=0;

CSusie::CSusie(CSystem& parent)
	:mSystem(parent)
{
	TRACE_SUSIE0("CSusie()");
	Reset();
}

CSusie::~CSusie()
{
	TRACE_SUSIE0("~CSusie()");
}

void CSusie::Reset(void)
{
	TRACE_SUSIE0("Reset()");

	// Fetch pointer to system RAM, faster than object access
	// and seeing as Susie only ever sees RAM.

	mRamPointer=mSystem.GetRamPointer();

	// Reset ALL variables

	mTMPADR.Word=0;
	mTILTACUM.Word=0;
	mHOFF.Word=0;
	mVOFF.Word=0;
	mVIDBAS.Word=0;
	mCOLLBAS.Word=0;
	mVIDADR.Word=0;
	mCOLLADR.Word=0;
	mSCBNEXT.Word=0;
	mSPRDLINE.Word=0;
	mHPOSSTRT.Word=0;
	mVPOSSTRT.Word=0;
	mSPRHSIZ.Word=0;
	mSPRVSIZ.Word=0;
	mSTRETCH.Word=0;
	mTILT.Word=0;
	mSPRDOFF.Word=0;
	mSPRVPOS.Word=0;
	mCOLLOFF.Word=0;
	mVSIZACUM.Word=0;
	mHSIZACUM.Word=0;
	mHSIZOFF.Word=0x007f;
	mVSIZOFF.Word=0x007f;
	mSCBADR.Word=0;
	mPROCADR.Word=0;

	// Must be initialised to this due to
	// stun runner math initialisation bug
	// see whatsnew for 0.7
	mMATHABCD.Long=0xffffffff;
	mMATHEFGH.Long=0xffffffff;
	mMATHJKLM.Long=0xffffffff;
	mMATHNP.Long=0xffff;

	mMATHAB_sign=1;
	mMATHCD_sign=1;
	mMATHEFGH_sign=1;

	mSPRCTL0_Type=0;
	mSPRCTL0_Vflip=0;
	mSPRCTL0_Hflip=0;
	mSPRCTL0_PixelBits=0;

	mSPRCTL1_StartLeft=0;
	mSPRCTL1_StartUp=0;
	mSPRCTL1_SkipSprite=0;
	mSPRCTL1_ReloadPalette=0;
	mSPRCTL1_ReloadDepth=0;
	mSPRCTL1_Sizing=0;
	mSPRCTL1_Literal=0;

	mSPRCOLL_Number=0;
	mSPRCOLL_Collide=0;

	mSPRSYS_StopOnCurrent=0;
	mSPRSYS_LeftHand=0;
	mSPRSYS_VStretch=0;
	mSPRSYS_NoCollide=0;
	mSPRSYS_Accumulate=0;
	mSPRSYS_SignedMath=0;
	mSPRSYS_Status=0;
	mSPRSYS_UnsafeAccess=0;
	mSPRSYS_LastCarry=0;
	mSPRSYS_Mathbit=0;
	mSPRSYS_MathInProgress=0;

	mSUZYBUSEN=FALSE;

	mSPRINIT.Byte=0;

	mSPRGO=FALSE;
	mEVERON=FALSE;

	for(int loop=0;loop<16;loop++) mPenIndex[loop]=loop;

	mJOYSTICK.Byte=0;
	mSWITCHES.Byte=0;
}

bool CSusie::ContextSave(FILE *fp)
{	
	TRACE_SUSIE0("ContextSave()");

	if(!fprintf(fp,"CSusie::ContextSave")) return 0;

	if(!fwrite(&mTMPADR,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mTILTACUM,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mHOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mVOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mVIDBAS,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mCOLLBAS,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mVIDADR,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mCOLLADR,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mSCBNEXT,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mSPRDLINE,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mHPOSSTRT,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mVPOSSTRT,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mSPRHSIZ,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mSPRVSIZ,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mSTRETCH,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mTILT,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mSPRDOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mSPRVPOS,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mCOLLOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mVSIZACUM,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mHSIZACUM,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mHSIZOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mVSIZOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mSCBADR,sizeof(UUWORD),1,fp)) return 0;
	if(!fwrite(&mPROCADR,sizeof(UUWORD),1,fp)) return 0;

	if(!fwrite(&mMATHABCD,sizeof(TMATHABCD),1,fp)) return 0;
	if(!fwrite(&mMATHEFGH,sizeof(TMATHEFGH),1,fp)) return 0;
	if(!fwrite(&mMATHJKLM,sizeof(TMATHJKLM),1,fp)) return 0;
	if(!fwrite(&mMATHNP,sizeof(TMATHNP),1,fp)) return 0;

	if(!fwrite(&mSPRCTL0_Type,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRCTL0_Vflip,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRCTL0_Hflip,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRCTL0_PixelBits,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mSPRCTL1_StartLeft,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRCTL1_StartUp,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRCTL1_SkipSprite,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRCTL1_ReloadPalette,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRCTL1_ReloadDepth,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRCTL1_Sizing,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRCTL1_Literal,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mSPRCOLL_Number,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRCOLL_Collide,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mSPRSYS_StopOnCurrent,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRSYS_LeftHand,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRSYS_VStretch,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRSYS_NoCollide,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRSYS_Accumulate,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRSYS_SignedMath,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRSYS_Status,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRSYS_UnsafeAccess,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRSYS_LastCarry,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRSYS_Mathbit,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mSPRSYS_MathInProgress,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mSUZYBUSEN,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mSPRINIT,sizeof(TSPRINIT),1,fp)) return 0;

	if(!fwrite(&mSPRGO,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mEVERON,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(mPenIndex,sizeof(UBYTE),16,fp)) return 0;

	if(!fwrite(&mLineType,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mLineShiftRegCount,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mLineShiftReg,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mLineRepeatCount,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mLinePixel,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mLinePacketBitsLeft,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mCollision,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mLineBaseAddress,sizeof(ULONG),1,fp)) return 0;
	if(!fwrite(&mLineCollisionAddress,sizeof(ULONG),1,fp)) return 0;

	if(!fwrite(&mJOYSTICK,sizeof(TJOYSTICK),1,fp)) return 0;
	if(!fwrite(&mSWITCHES,sizeof(TSWITCHES),1,fp)) return 0;

	return 1;
}

bool CSusie::ContextLoad(LSS_FILE *fp)
{
	TRACE_SUSIE0("ContextLoad()");

	char teststr[100]="XXXXXXXXXXXXXXXXXXX";
	if(!lss_read(teststr,sizeof(char),19,fp)) return 0;
	if(strcmp(teststr,"CSusie::ContextSave")!=0) return 0;

	if(!lss_read(&mTMPADR,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mTILTACUM,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mHOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mVOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mVIDBAS,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mCOLLBAS,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mVIDADR,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mCOLLADR,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mSCBNEXT,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mSPRDLINE,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mHPOSSTRT,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mVPOSSTRT,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mSPRHSIZ,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mSPRVSIZ,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mSTRETCH,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mTILT,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mSPRDOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mSPRVPOS,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mCOLLOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mVSIZACUM,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mHSIZACUM,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mHSIZOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mVSIZOFF,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mSCBADR,sizeof(UUWORD),1,fp)) return 0;
	if(!lss_read(&mPROCADR,sizeof(UUWORD),1,fp)) return 0;

	if(!lss_read(&mMATHABCD,sizeof(TMATHABCD),1,fp)) return 0;
	if(!lss_read(&mMATHEFGH,sizeof(TMATHEFGH),1,fp)) return 0;
	if(!lss_read(&mMATHJKLM,sizeof(TMATHJKLM),1,fp)) return 0;
	if(!lss_read(&mMATHNP,sizeof(TMATHNP),1,fp)) return 0;

	if(!lss_read(&mSPRCTL0_Type,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRCTL0_Vflip,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRCTL0_Hflip,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRCTL0_PixelBits,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mSPRCTL1_StartLeft,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRCTL1_StartUp,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRCTL1_SkipSprite,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRCTL1_ReloadPalette,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRCTL1_ReloadDepth,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRCTL1_Sizing,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRCTL1_Literal,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mSPRCOLL_Number,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRCOLL_Collide,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mSPRSYS_StopOnCurrent,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRSYS_LeftHand,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRSYS_VStretch,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRSYS_NoCollide,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRSYS_Accumulate,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRSYS_SignedMath,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRSYS_Status,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRSYS_UnsafeAccess,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRSYS_LastCarry,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRSYS_Mathbit,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mSPRSYS_MathInProgress,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mSUZYBUSEN,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mSPRINIT,sizeof(TSPRINIT),1,fp)) return 0;

	if(!lss_read(&mSPRGO,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mEVERON,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(mPenIndex,sizeof(UBYTE),16,fp)) return 0;

	if(!lss_read(&mLineType,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mLineShiftRegCount,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mLineShiftReg,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mLineRepeatCount,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mLinePixel,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mLinePacketBitsLeft,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mCollision,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mLineBaseAddress,sizeof(ULONG),1,fp)) return 0;
	if(!lss_read(&mLineCollisionAddress,sizeof(ULONG),1,fp)) return 0;

	if(!lss_read(&mJOYSTICK,sizeof(TJOYSTICK),1,fp)) return 0;
	if(!lss_read(&mSWITCHES,sizeof(TSWITCHES),1,fp)) return 0;

	return 1;
}


void CSusie::DoMathMultiply(void)
{
	mSPRSYS_Mathbit=FALSE;

	// Multiplies with out sign or accumulate take 44 ticks to complete.
	// Multiplies with sign and accumulate take 54 ticks to complete. 
	//
	//    AB                                    EFGH
	//  * CD                                  /   NP
	// -------                            -----------
	//  EFGH                                    ABCD
	// Accumulate in JKLM         Remainder in (JK)LM
	//


	ULONG result;

	// Basic multiply is ALWAYS unsigned, sign conversion is done later
	result=(ULONG)mMATHABCD.Words.AB*(ULONG)mMATHABCD.Words.CD;
	mMATHEFGH.Long=result;

	if(mSPRSYS_SignedMath)
	{
		TRACE_SUSIE0("DoMathMultiply() - SIGNED");
		// Add the sign bits, only >0 is +ve result
		mMATHEFGH_sign=mMATHAB_sign+mMATHCD_sign;
		if(!mMATHEFGH_sign)
		{
			mMATHEFGH.Long^=0xffffffff;
			mMATHEFGH.Long++;
		}
	}
	else
	{
		TRACE_SUSIE0("DoMathMultiply() - UNSIGNED");
	}

	TRACE_SUSIE2("DoMathMultiply() AB=$%04x * CD=$%04x",mMATHABCD.Words.AB,mMATHABCD.Words.CD);

	// Check overflow, if B31 has changed from 1->0 then its overflow time
	if(mSPRSYS_Accumulate)
	{
		TRACE_SUSIE0("DoMathMultiply() - ACCUMULATED JKLM+=EFGH");
		ULONG tmp=mMATHJKLM.Long+mMATHEFGH.Long;
		// Let sign change indicate overflow
		if((tmp&0x80000000)!=(mMATHJKLM.Long&0x80000000))
		{
			TRACE_SUSIE0("DoMathMultiply() - OVERFLOW DETECTED");
//			mSPRSYS_Mathbit=TRUE;
		}
		else
		{
//			mSPRSYS_Mathbit=FALSE;
		}
		// Save accumulated result
		mMATHJKLM.Long=tmp;
	}

	TRACE_SUSIE1("DoMathMultiply() Results (raw - no sign) Result=$%08x",result);
	TRACE_SUSIE1("DoMathMultiply() Results (Multi) EFGH=$%08x",mMATHEFGH.Long);
	TRACE_SUSIE1("DoMathMultiply() Results (Accum) JKLM=$%08x",mMATHJKLM.Long);
}

void CSusie::DoMathDivide(void)
{
	mSPRSYS_Mathbit=FALSE;

	//
	// Divides take 176 + 14*N ticks
	// (N is the number of most significant zeros in the divisor.)
	//
	//    AB                                    EFGH
	//  * CD                                  /   NP
	// -------                            -----------
	//  EFGH                                    ABCD
	// Accumulate in JKLM         Remainder in (JK)LM
	//

	// Divide is ALWAYS unsigned arithmetic...
	if(mMATHNP.Long)
	{
		TRACE_SUSIE0("DoMathDivide() - UNSIGNED");
		mMATHABCD.Long=mMATHEFGH.Long/mMATHNP.Long;
		mMATHJKLM.Long=mMATHEFGH.Long%mMATHNP.Long;
	}
	else
	{
		TRACE_SUSIE0("DoMathDivide() - DIVIDE BY ZERO ERROR");
		mMATHABCD.Long=0xffffffff;
		mMATHJKLM.Long=0;
		mSPRSYS_Mathbit=TRUE;
	}
	TRACE_SUSIE2("DoMathDivide() EFGH=$%08x / NP=%04x",mMATHEFGH.Long,mMATHNP.Long);
	TRACE_SUSIE1("DoMathDivide() Results (div) ABCD=$%08x",mMATHABCD.Long);
	TRACE_SUSIE1("DoMathDivide() Results (mod) JKLM=$%08x",mMATHJKLM.Long);
}


ULONG CSusie::PaintSprites(void)
{
	int	sprcount=0;
	int data=0;
	int everonscreen=0;

	TRACE_SUSIE0("                                                              ");
	TRACE_SUSIE0("                                                              ");
	TRACE_SUSIE0("                                                              ");
	TRACE_SUSIE0("**************************************************************");
	TRACE_SUSIE0("********************** PaintSprites **************************");
	TRACE_SUSIE0("**************************************************************");
	TRACE_SUSIE0("                                                              ");

	TRACE_SUSIE1("PaintSprites() VIDBAS  $%04x",mVIDBAS.Word);
	TRACE_SUSIE1("PaintSprites() COLLBAS $%04x",mCOLLBAS.Word);
	TRACE_SUSIE1("PaintSprites() SPRSYS  $%02x",Peek(SPRSYS));

	if(!mSUZYBUSEN || !mSPRGO)
	{
		TRACE_SUSIE0("PaintSprites() Returned !mSUZYBUSEN || !mSPRGO");
		return 0;
	}

	cycles_used=0;
	everonscreen=0;

	do
	{
		TRACE_SUSIE1("PaintSprites() ************ Rendering Sprite %03d ************",sprcount);

		// Step 1 load up the SCB params into Susie

		// And thus it is documented that only the top byte of SCBNEXT is used.
		// Its mentioned under the bits that are broke section in the bluebook
		if(!(mSCBNEXT.Word&0xff00))
		{
			TRACE_SUSIE0("PaintSprites() mSCBNEXT==0 - FINISHED");
			mSPRSYS_Status=0;	// Engine has finished
			mSPRGO=FALSE;
			break;
		}
		else
		{
			mSPRSYS_Status=1;
		}

		mTMPADR.Word=mSCBNEXT.Word;	// Copy SCB pointer
		mSCBADR.Word=mSCBNEXT.Word;	// Copy SCB pointer
		TRACE_SUSIE1("PaintSprites() SCBADDR $%04x",mSCBADR.Word);

		data=RAM_PEEK(mTMPADR.Word);			// Fetch control 0
		TRACE_SUSIE1("PaintSprites() SPRCTL0 $%02x",data);
		mSPRCTL0_Type=data&0x0007;
		mSPRCTL0_Vflip=data&0x0010;
		mSPRCTL0_Hflip=data&0x0020;
		mSPRCTL0_PixelBits=((data&0x00c0)>>6)+1;
		mTMPADR.Word+=1;

		data=RAM_PEEK(mTMPADR.Word);			// Fetch control 1
		TRACE_SUSIE1("PaintSprites() SPRCTL1 $%02x",data);
		mSPRCTL1_StartLeft=data&0x0001;
		mSPRCTL1_StartUp=data&0x0002;
		mSPRCTL1_SkipSprite=data&0x0004;
		mSPRCTL1_ReloadPalette=data&0x0008;
		mSPRCTL1_ReloadDepth=(data&0x0030)>>4;
		mSPRCTL1_Sizing=data&0x0040;	
		mSPRCTL1_Literal=data&0x0080;
		mTMPADR.Word+=1;

		data=RAM_PEEK(mTMPADR.Word);			// Collision num
		TRACE_SUSIE1("PaintSprites() SPRCOLL $%02x",data);
		mSPRCOLL_Number=data&0x000f;
		mSPRCOLL_Collide=data&0x0020;
		mTMPADR.Word+=1;

		mSCBNEXT.Word=RAM_PEEKW(mTMPADR.Word);	// Next SCB
		TRACE_SUSIE1("PaintSprites() SCBNEXT $%04x",mSCBNEXT.Word);
		mTMPADR.Word+=2;

		cycles_used+=5*SPR_RDWR_CYC;

		// Initialise the collision depositary

// Although Tom Schenck says this is correct, it doesnt appear to be
//		if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide)
//		{
//			mCollision=RAM_PEEK((mSCBADR.Word+mCOLLOFF.Word)&0xffff);
//			mCollision&=0x0f;
//		}
		mCollision=0;

		// Check if this is a skip sprite

		if(!mSPRCTL1_SkipSprite)
		{

			mSPRDLINE.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite pack data
			TRACE_SUSIE1("PaintSprites() SPRDLINE $%04x",mSPRDLINE.Word);
			mTMPADR.Word+=2;

			mHPOSSTRT.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite horizontal start position
			TRACE_SUSIE1("PaintSprites() HPOSSTRT $%04x",mHPOSSTRT.Word);
			mTMPADR.Word+=2;

			mVPOSSTRT.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite vertical start position
			TRACE_SUSIE1("PaintSprites() VPOSSTRT $%04x",mVPOSSTRT.Word);
			mTMPADR.Word+=2;

			cycles_used+=6*SPR_RDWR_CYC;

			bool enable_sizing=FALSE;
			bool enable_stretch=FALSE;
			bool enable_tilt=FALSE;
		
			// Optional section defined by reload type in Control 1

			TRACE_SUSIE1("PaintSprites() mSPRCTL1.Bits.ReloadDepth=%d",mSPRCTL1_ReloadDepth);
			switch(mSPRCTL1_ReloadDepth)
			{
				case 1:
					TRACE_SUSIE0("PaintSprites() Sizing Enabled");
					enable_sizing=TRUE;

					mSPRHSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
					mTMPADR.Word+=2;

					mSPRVSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Verticalal size
					mTMPADR.Word+=2;

					cycles_used+=4*SPR_RDWR_CYC;
					break;

				case 2:
					TRACE_SUSIE0("PaintSprites() Sizing Enabled");
					TRACE_SUSIE0("PaintSprites() Stretch Enabled");
					enable_sizing=TRUE;
					enable_stretch=TRUE;

					mSPRHSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
					mTMPADR.Word+=2;

					mSPRVSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Verticalal size
					mTMPADR.Word+=2;

					mSTRETCH.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite stretch
					mTMPADR.Word+=2;

					cycles_used+=6*SPR_RDWR_CYC;
					break;

				case 3:
					TRACE_SUSIE0("PaintSprites() Sizing Enabled");
					TRACE_SUSIE0("PaintSprites() Stretch Enabled");
					TRACE_SUSIE0("PaintSprites() Tilt Enabled");
					enable_sizing=TRUE;
					enable_stretch=TRUE;
					enable_tilt=TRUE;

					mSPRHSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Horizontal size
					mTMPADR.Word+=2;

					mSPRVSIZ.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite Verticalal size
					mTMPADR.Word+=2;

					mSTRETCH.Word=RAM_PEEKW(mTMPADR.Word);	// Sprite stretch
					mTMPADR.Word+=2;

					mTILT.Word=RAM_PEEKW(mTMPADR.Word);		// Sprite tilt
					mTMPADR.Word+=2;

					cycles_used+=8*SPR_RDWR_CYC;
					break;

				default:
					break;
			}

			TRACE_SUSIE1("PaintSprites() SPRHSIZ $%04x",mSPRHSIZ.Word);
			TRACE_SUSIE1("PaintSprites() SPRVSIZ $%04x",mSPRVSIZ.Word);
			TRACE_SUSIE1("PaintSprites() STRETCH $%04x",mSTRETCH.Word);
			TRACE_SUSIE1("PaintSprites() TILT    $%04x",mTILT.Word);


			// Optional Palette reload

			if(!mSPRCTL1_ReloadPalette)
			{
				TRACE_SUSIE0("PaintSprites() Palette reloaded");
				for(int loop=0;loop<8;loop++)
				{
					UBYTE data=RAM_PEEK(mTMPADR.Word++);
					mPenIndex[loop*2]=(data>>4)&0x0f;
					mPenIndex[(loop*2)+1]=data&0x0f;
				}
				// Increment cycle count for the reads
				cycles_used+=8*SPR_RDWR_CYC;
			}

			// Now we can start painting
		
			// Quadrant drawing order is: SE,NE,NW,SW
			// start quadrant is given by sprite_control1:0 & 1

			// Setup screen start end variables

			int screen_h_start=(SWORD)mHOFF.Word;
			int screen_h_end=(SWORD)mHOFF.Word+SCREEN_WIDTH;
			int screen_v_start=(SWORD)mVOFF.Word;
			int screen_v_end=(SWORD)mVOFF.Word+SCREEN_HEIGHT;

			int world_h_mid=screen_h_start+0x8000+(SCREEN_WIDTH/2);
			int world_v_mid=screen_v_start+0x8000+(SCREEN_HEIGHT/2);

			TRACE_SUSIE2("PaintSprites() screen_h_start $%04x screen_h_end $%04x",screen_h_start,screen_h_end);
			TRACE_SUSIE2("PaintSprites() screen_v_start $%04x screen_v_end $%04x",screen_v_start,screen_v_end);
			TRACE_SUSIE2("PaintSprites() world_h_mid    $%04x world_v_mid  $%04x",world_h_mid,world_v_mid);

			bool superclip=FALSE;
			int quadrant=0;
			int hsign,vsign;

			if(mSPRCTL1_StartLeft)
			{
				if(mSPRCTL1_StartUp) quadrant=2; else quadrant=3;
			}
			else
			{
				if(mSPRCTL1_StartUp) quadrant=1; else quadrant=0;
			}
			TRACE_SUSIE1("PaintSprites() Quadrant=%d",quadrant);

			// Check ref is inside screen area

			if((SWORD)mHPOSSTRT.Word<screen_h_start || (SWORD)mHPOSSTRT.Word>=screen_h_end ||
				(SWORD)mVPOSSTRT.Word<screen_v_start || (SWORD)mVPOSSTRT.Word>=screen_v_end) superclip=TRUE;

			TRACE_SUSIE1("PaintSprites() Superclip=%d",superclip);


			// Quadrant mapping is:	SE	NE	NW	SW
			//						0	1	2	3
			// hsign				+1	+1	-1	-1
			// vsign				+1	-1	-1	+1
			//
			//
			//		2 | 1
			//     -------
			//      3 | 0
			//

			// Loop for 4 quadrants

			for(int loop=0;loop<4;loop++)	
			{
				TRACE_SUSIE1("PaintSprites() -------- Rendering Quadrant %03d --------",quadrant);

				int sprite_v=mVPOSSTRT.Word;
				int sprite_h=mHPOSSTRT.Word;

				bool render=FALSE;

				// Set quadrand multipliers
				hsign=(quadrant==0 || quadrant==1)?1:-1;
				vsign=(quadrant==0 || quadrant==3)?1:-1;

// Preflip		TRACE_SUSIE2("PaintSprites() hsign=%d vsign=%d",hsign,vsign);

				//Use h/v flip to invert v/hsign

				if(mSPRCTL0_Vflip) vsign=-vsign;
				if(mSPRCTL0_Hflip) hsign=-hsign;

				TRACE_SUSIE2("PaintSprites() Hflip=%d Vflip=%d",mSPRCTL0_Hflip,mSPRCTL0_Vflip);
				TRACE_SUSIE2("PaintSprites() Hsign=%d   Vsign=%d",hsign,vsign);
				TRACE_SUSIE2("PaintSprites() Hpos =%04x Vpos =%04x",mHPOSSTRT.Word,mVPOSSTRT.Word);
				TRACE_SUSIE2("PaintSprites() Hsizoff =%04x Vsizoff =%04x",mHSIZOFF.Word,mVSIZOFF.Word);

				// Two different rendering algorithms used, on-screen & superclip
				// when on screen we draw in x until off screen then skip to next
				// line, BUT on superclip we draw all the way to the end of any
				// given line checking each pixel is on screen.

				if(superclip)
				{
					// Check on the basis of each quad, we only render the quad
					// IF the screen is in the quad, relative to the centre of
					// the screen which is calculated below.

					// Quadrant mapping is:	SE	NE	NW	SW
					//						0	1	2	3
					// hsign				+1	+1	-1	-1
					// vsign				+1	-1	-1	+1
					//
					//
					//		2 | 1
					//     -------
					//      3 | 0
					//
					// Quadrant mapping for superclipping must also take into account
					// the hflip, vflip bits & negative tilt to be able to work correctly
					//
					int	modquad=quadrant;
					static int vquadflip[4]={1,0,3,2};
					static int hquadflip[4]={3,2,1,0};

					if(mSPRCTL0_Vflip) modquad=vquadflip[modquad];
					if(mSPRCTL0_Hflip) modquad=hquadflip[modquad];

// This is causing Eurosoccer to fail!!
//					if(enable_tilt && mTILT.Word&0x8000) modquad=hquadflip[modquad];

					switch(modquad)
					{
						case 3:
							if((sprite_h>=screen_h_start || sprite_h<world_h_mid) && (sprite_v<screen_v_end   || sprite_v>world_v_mid)) render=TRUE;
							break;
						case 2:
							if((sprite_h>=screen_h_start || sprite_h<world_h_mid) && (sprite_v>=screen_v_start || sprite_v<world_v_mid)) render=TRUE;
							break;
						case 1:
							if((sprite_h<screen_h_end   || sprite_h>world_h_mid) && (sprite_v>=screen_v_start || sprite_v<world_v_mid)) render=TRUE;
							break;
						default:
							if((sprite_h<screen_h_end   || sprite_h>world_h_mid) && (sprite_v<screen_v_end   || sprite_v>world_v_mid)) render=TRUE;
							break;
					}
				}
				else
				{
					render=TRUE;
				}

				// Is this quad to be rendered ??

				TRACE_SUSIE1("PaintSprites() Render status %d",render);

				static int pixel_height=0;
				static int pixel_width=0;
				static int pixel=0;
				static int hoff=0,voff=0;
				static int hloop=0,vloop=0;
				static bool onscreen=0;
				static int vquadoff=0;
				static int hquadoff=0;

				if(render)
				{
					// Set the vertical position & offset
					voff=(SWORD)mVPOSSTRT.Word-screen_v_start;

					// Zero the stretch,tilt & acum values
					mTILTACUM.Word=0;

					// Perform the SIZOFF
					if(vsign==1) mVSIZACUM.Word=mVSIZOFF.Word; else mVSIZACUM.Word=0;

					// Take the sign of the first quad (0) as the basic
					// sign, all other quads drawing in the other direction
					// get offset by 1 pixel in the other direction, this
					// fixes the squashed look on the multi-quad sprites.
//					if(vsign==-1 && loop>0) voff+=vsign;
					if(loop==0)	vquadoff=vsign;
					if(vsign!=vquadoff) voff+=vsign;
					
					for(;;)
					{
						// Vertical scaling is done here
						mVSIZACUM.Word+=mSPRVSIZ.Word;
						pixel_height=mVSIZACUM.Byte.High;
						mVSIZACUM.Byte.High=0;

						// Update the next data line pointer and initialise our line
						mSPRDOFF.Word=(UWORD)LineInit(0);

						// If 1 == next quad, ==0 end of sprite, anyways its END OF LINE
						if(mSPRDOFF.Word==1)		// End of quad
						{
							mSPRDLINE.Word+=mSPRDOFF.Word;
							break;
						}

						if(mSPRDOFF.Word==0)		// End of sprite
						{
							loop=4;		// Halt the quad loop
							break;
						}

						// Draw one horizontal line of the sprite 
						for(vloop=0;vloop<pixel_height;vloop++)
						{
							// Early bailout if the sprite has moved off screen, terminate quad
							if(vsign==1 && voff>=SCREEN_HEIGHT)	break;
							if(vsign==-1 && voff<0)	break;

							// Only allow the draw to take place if the line is visible
							if(voff>=0 && voff<SCREEN_HEIGHT)
							{
								// Work out the horizontal pixel start position, start + tilt
								mHPOSSTRT.Word+=((SWORD)mTILTACUM.Word>>8);
								mTILTACUM.Byte.High=0;
								hoff=(int)((SWORD)mHPOSSTRT.Word)-screen_h_start;

					 			// Zero/Force the horizontal scaling accumulator
								if(hsign==1) mHSIZACUM.Word=mHSIZOFF.Word; else mHSIZACUM.Word=0;

								// Take the sign of the first quad (0) as the basic
								// sign, all other quads drawing in the other direction
								// get offset by 1 pixel in the other direction, this
								// fixes the squashed look on the multi-quad sprites.
//								if(hsign==-1 && loop>0) hoff+=hsign;
								if(loop==0)	hquadoff=hsign;
								if(hsign!=hquadoff) hoff+=hsign;

								// Initialise our line
								LineInit(voff);
								onscreen=FALSE;

								// Now render an individual destination line
								while((pixel=LineGetPixel())!=LINE_END)
								{
									// This is allowed to update every pixel
									mHSIZACUM.Word+=mSPRHSIZ.Word;
									pixel_width=mHSIZACUM.Byte.High;
									mHSIZACUM.Byte.High=0;

									for(hloop=0;hloop<pixel_width;hloop++)
									{
										// Draw if onscreen but break loop on transition to offscreen
										if(hoff>=0 && hoff<SCREEN_WIDTH)
										{
											ProcessPixel(hoff,pixel);
											onscreen=everonscreen=TRUE;
										}
										else
										{
											if(onscreen) break;
										}
										hoff+=hsign;
									}
								}
							}
							voff+=vsign;

							// For every destination line we can modify SPRHSIZ & SPRVSIZ & TILTACUM
							if(enable_stretch)
							{
								mSPRHSIZ.Word+=mSTRETCH.Word;
//								if(mSPRSYS_VStretch) mSPRVSIZ.Word+=mSTRETCH.Word;
							}
							if(enable_tilt)
							{
								// Manipulate the tilt stuff
								mTILTACUM.Word+=mTILT.Word;
							}
						}
						// According to the docs this increments per dest line 
						// but only gets set when the source line is read
						if(mSPRSYS_VStretch) mSPRVSIZ.Word+=mSTRETCH.Word*pixel_height;

						// Update the line start for our next run thru the loop
						mSPRDLINE.Word+=mSPRDOFF.Word;
					}
				}
				else
				{
					// Skip thru data to next quad
					for(;;)
					{
						// Read the start of line offset

						mSPRDOFF.Word=(UWORD)LineInit(0);

						// We dont want to process data so mSPRDLINE is useless to us
						mSPRDLINE.Word+=mSPRDOFF.Word;

						// If 1 == next quad, ==0 end of sprite, anyways its END OF LINE

						if(mSPRDOFF.Word==1) break;	// End of quad
						if(mSPRDOFF.Word==0)		// End of sprite
						{
							loop=4;		// Halt the quad loop
							break;
						}

					}
				}

				// Increment quadrant and mask to 2 bit value (0-3)
				quadrant++;
				quadrant&=0x03;
			}

			// Write the collision depositary if required

			if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide)
			{
				switch(mSPRCTL0_Type)
				{
					case sprite_xor_shadow:
					case sprite_boundary:
					case sprite_normal:
					case sprite_boundary_shadow:
					case sprite_shadow:
						{
							UWORD coldep=mSCBADR.Word+mCOLLOFF.Word;
							RAM_POKE(coldep,(UBYTE)mCollision);
							TRACE_SUSIE2("PaintSprites() COLLOFF=$%04x SCBADR=$%04x",mCOLLOFF.Word,mSCBADR.Word);
							TRACE_SUSIE2("PaintSprites() Wrote $%02x to SCB collision depositary at $%04x",(UBYTE)mCollision,coldep);
						}
						break;
					default:
						break;
				}
			}

			if(mEVERON)
			{
				UWORD coldep=mSCBADR.Word+mCOLLOFF.Word;
				UBYTE coldat=RAM_PEEK(coldep);
				if(!everonscreen) coldat|=0x80; else coldat&=0x7f;
				RAM_POKE(coldep,coldat);
				TRACE_SUSIE0("PaintSprites() EVERON IS ACTIVE");
				TRACE_SUSIE2("PaintSprites() Wrote $%02x to SCB collision depositary at $%04x",coldat,coldep);
			}

			// Perform Sprite debugging if required, single step on sprite draw
			if(gSingleStepModeSprites)
			{
				char message[256];
				sprintf(message,"CSusie:PaintSprites() - Rendered Sprite %03d",sprcount);
				if(!gError->Warning(message)) gSingleStepModeSprites=0;
			}
		}
		else
		{
			TRACE_SUSIE0("PaintSprites() mSPRCTL1.Bits.SkipSprite==TRUE");
		}

		// Increase sprite number
		sprcount++;

		// Check if we abort after 1st sprite is complete

//		if(mSPRSYS.Read.StopOnCurrent) 
//		{
//			mSPRSYS.Read.Status=0;	// Engine has finished
//			mSPRGO=FALSE;
//			break;
//		}

		// Check sprcount for looping SCB, random large number chosen
		if(sprcount>4096)
		{
			// Stop the system, otherwise we may just come straight back in.....
			gSystemHalt=TRUE;
			// Display warning message
			gError->Warning("CSusie:PaintSprites(): Single draw sprite limit exceeded (>4096). The SCB is most likely looped back on itself. Reset/Exit is recommended");
			// Signal error to the caller
			return 0;
		}
	}
	while(1);

	// Fudge factor to fix many flickering issues, also the keypress
	// problem with Hard Drivin and the strange pause in Dirty Larry.
//	cycles_used>>=2;

	return cycles_used;
}

//
// Collision code modified by KW 22/11/98
// Collision buffer cler added if there is no
// apparent collision, I have a gut feeling this
// is the wrong solution to the inv07.com bug but
// it seems to work OK.
//
// Shadow-------------------------------|
// Boundary-Shadow--------------------| |
// Normal---------------------------| | |
// Boundary-----------------------| | | |
// Background-Shadow------------| | | | |
// Background-No Collision----| | | | | |
// Non-Collideable----------| | | | | | |
// Exclusive-or-Shadow----| | | | | | | |
//                        | | | | | | | |
//                        1 1 1 1 0 1 0 1   F is opaque 
//                        0 0 0 0 1 1 0 0   E is collideable 
//                        0 0 1 1 0 0 0 0   0 is opaque and collideable 
//                        1 0 0 0 1 1 1 1   allow collision detect 
//                        1 0 0 1 1 1 1 1   allow coll. buffer access 
//                        1 0 0 0 0 0 0 0   exclusive-or the data 
//

inline void CSusie::ProcessPixel(ULONG hoff,ULONG pixel)
{
	switch(mSPRCTL0_Type)
	{
		// BACKGROUND SHADOW
		// 1   F is opaque 
		// 0   E is collideable 
		// 1   0 is opaque and collideable 
		// 0   allow collision detect 
		// 1   allow coll. buffer access 
		// 0   exclusive-or the data 
		case sprite_background_shadow:
			WritePixel(hoff,pixel);
			if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide && pixel!=0x0e)
			{
				WriteCollision(hoff,mSPRCOLL_Number);
			}
			break;

		// BACKGROUND NOCOLLIDE
		// 1   F is opaque 
		// 0   E is collideable 
		// 1   0 is opaque and collideable 
		// 0   allow collision detect 
		// 0   allow coll. buffer access 
		// 0   exclusive-or the data 
		case sprite_background_noncollide:
			WritePixel(hoff,pixel);
			break;

		// NOCOLLIDE
		// 1   F is opaque 
		// 0   E is collideable 
		// 0   0 is opaque and collideable 
		// 0   allow collision detect 
		// 0   allow coll. buffer access 
		// 0   exclusive-or the data 
		case sprite_noncollide:
			if(pixel!=0x00) WritePixel(hoff,pixel);
			break;

		// BOUNDARY
		// 0   F is opaque 
		// 1   E is collideable 
		// 0   0 is opaque and collideable 
		// 1   allow collision detect 
		// 1   allow coll. buffer access 
		// 0   exclusive-or the data 
		case sprite_boundary:
			if(pixel!=0x00 && pixel!=0x0f)
			{
				WritePixel(hoff,pixel);
			}
			if(pixel!=0x00)
			{
				if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide)
				{
					int collision=ReadCollision(hoff);
					if(collision>mCollision)
					{
						mCollision=collision;
					}
// 01/05/00 V0.7	if(mSPRCOLL_Number>collision)
					{
						WriteCollision(hoff,mSPRCOLL_Number);
					}
				}
			}
			break;

		// NORMAL
		// 1   F is opaque 
		// 1   E is collideable 
		// 0   0 is opaque and collideable 
		// 1   allow collision detect 
		// 1   allow coll. buffer access 
		// 0   exclusive-or the data 
		case sprite_normal:
			if(pixel!=0x00)
			{
				WritePixel(hoff,pixel);
				if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide)
				{
					int collision=ReadCollision(hoff);
					if(collision>mCollision)
					{
						mCollision=collision;
					}
// 01/05/00 V0.7	if(mSPRCOLL_Number>collision)
					{
						WriteCollision(hoff,mSPRCOLL_Number);
					}
				}
			}
			break;

		// BOUNDARY_SHADOW
		// 0   F is opaque 
		// 0   E is collideable 
		// 0   0 is opaque and collideable 
		// 1   allow collision detect 
		// 1   allow coll. buffer access 
		// 0   exclusive-or the data 
		case sprite_boundary_shadow:
			if(pixel!=0x00 && pixel!=0x0e && pixel!=0x0f)
			{
				WritePixel(hoff,pixel);
			}
			if(pixel!=0x00 && pixel!=0x0e)
			{
				if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide)
				{
					int collision=ReadCollision(hoff);
					if(collision>mCollision)
					{
						mCollision=collision;
					}
// 01/05/00 V0.7	if(mSPRCOLL_Number>collision)
					{
						WriteCollision(hoff,mSPRCOLL_Number);
					}
				}
			}
			break;

		// SHADOW
		// 1   F is opaque 
		// 0   E is collideable 
		// 0   0 is opaque and collideable 
		// 1   allow collision detect 
		// 1   allow coll. buffer access 
		// 0   exclusive-or the data 
		case sprite_shadow:
			if(pixel!=0x00)
			{
				WritePixel(hoff,pixel);
			}
			if(pixel!=0x00 && pixel!=0x0e)
			{
				if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide)
				{
					int collision=ReadCollision(hoff);
					if(collision>mCollision)
					{
						mCollision=collision;
					}
// 01/05/00 V0.7	if(mSPRCOLL_Number>collision)
					{
						WriteCollision(hoff,mSPRCOLL_Number);
					}
				}
			}
			break;

		// XOR SHADOW
		// 1   F is opaque 
		// 0   E is collideable 
		// 0   0 is opaque and collideable 
		// 1   allow collision detect 
		// 1   allow coll. buffer access 
		// 1   exclusive-or the data 
		case sprite_xor_shadow:
			if(pixel!=0x00)
			{
				WritePixel(hoff,ReadPixel(hoff)^pixel);
			}
			if(pixel!=0x00 && pixel!=0x0e)
			{
				if(!mSPRCOLL_Collide && !mSPRSYS_NoCollide && pixel!=0x0e)
				{
					int collision=ReadCollision(hoff);
					if(collision>mCollision)
					{
						mCollision=collision;
					}
// 01/05/00 V0.7	if(mSPRCOLL_Number>collision)
					{
						WriteCollision(hoff,mSPRCOLL_Number);
					}
				}
			}
			break;
		default:
//			_asm int 3;
			break;
	}
}

inline void CSusie::WritePixel(ULONG hoff,ULONG pixel)
{
	ULONG scr_addr=mLineBaseAddress+(hoff/2);
	
	UBYTE dest=RAM_PEEK(scr_addr);
	if(!(hoff&0x01))
	{
		// Upper nibble screen write
		dest&=0x0f;
		dest|=pixel<<4;
	}
	else
	{
		// Lower nibble screen write
		dest&=0xf0;
		dest|=pixel;
	}
	RAM_POKE(scr_addr,dest);

	// Increment cycle count for the read/modify/write
	cycles_used+=2*SPR_RDWR_CYC;
}

inline ULONG CSusie::ReadPixel(ULONG hoff)
{
	ULONG scr_addr=mLineBaseAddress+(hoff/2);
	
	ULONG data=RAM_PEEK(scr_addr);
	if(!(hoff&0x01))
	{
		// Upper nibble read
		data>>=4;
	}
	else
	{
		// Lower nibble read
		data&=0x0f;
	}

	// Increment cycle count for the read/modify/write
	cycles_used+=SPR_RDWR_CYC;

	return data;
}

inline void CSusie::WriteCollision(ULONG hoff,ULONG pixel)
{
	ULONG col_addr=mLineCollisionAddress+(hoff/2);
	
	UBYTE dest=RAM_PEEK(col_addr);
	if(!(hoff&0x01))
	{
		// Upper nibble screen write
		dest&=0x0f;
		dest|=pixel<<4;
	}
	else
	{
		// Lower nibble screen write
		dest&=0xf0;
		dest|=pixel;
	}
	RAM_POKE(col_addr,dest);

	// Increment cycle count for the read/modify/write
	cycles_used+=2*SPR_RDWR_CYC;
}

inline ULONG CSusie::ReadCollision(ULONG hoff)
{
	ULONG col_addr=mLineCollisionAddress+(hoff/2);
	
	ULONG data=RAM_PEEK(col_addr);
	if(!(hoff&0x01))
	{
		// Upper nibble read
		data>>=4;
	}
	else
	{
		// Lower nibble read
		data&=0x0f;
	}

	// Increment cycle count for the read/modify/write
	cycles_used+=SPR_RDWR_CYC;

	return data;
}


inline ULONG CSusie::LineInit(ULONG voff)
{
//	TRACE_SUSIE0("LineInit()");

	mLineShiftReg=0;
	mLineShiftRegCount=0;
	mLineRepeatCount=0;
	mLinePixel=0;
	mLineType=line_error;
	mLinePacketBitsLeft=0xffff;

	// Initialise the temporary pointer

	mTMPADR=mSPRDLINE;

	// First read the Offset to the next line

	ULONG offset=LineGetBits(8);
//	TRACE_SUSIE1("LineInit() Offset=%04x",offset);

	// Specify the MAXIMUM number of bits in this packet, it
	// can terminate early but can never use more than this
	// without ending the current packet, we count down in LineGetBits()

	mLinePacketBitsLeft=(offset-1)*8;

	// Literals are a special case and get their count set on a line basis
	
	if(mSPRCTL1_Literal)
	{
		mLineType=line_abs_literal;
		mLineRepeatCount=((offset-1)*8)/mSPRCTL0_PixelBits;
		// Why is this necessary, is this compensating for the 1,1 offset bug
//		mLineRepeatCount--;
	}
//	TRACE_SUSIE1("LineInit() mLineRepeatCount=$%04x",mLineRepeatCount);

	// Set the line base address for use in the calls to pixel painting

	if(voff>101)
	{
		gError->Warning("CSusie::LineInit() Out of bounds (voff)");
		voff=0;
	}

	mLineBaseAddress=mVIDBAS.Word+(voff*(SCREEN_WIDTH/2));
	mLineCollisionAddress=mCOLLBAS.Word+(voff*(SCREEN_WIDTH/2));
//	TRACE_SUSIE1("LineInit() mLineBaseAddress=$%04x",mLineBaseAddress);
//	TRACE_SUSIE1("LineInit() mLineCollisionAddress=$%04x",mLineCollisionAddress);

	// Return the offset to the next line

	return offset;
}

inline ULONG CSusie::LineGetPixel()
{
	if(!mLineRepeatCount)
	{
		// Normal sprites fetch their counts on a packet basis
		if(mLineType!=line_abs_literal)
		{
			ULONG literal=LineGetBits(1);
			if(literal) mLineType=line_literal; else mLineType=line_packed;
		}

		// Pixel store is empty what should we do
		switch(mLineType)
		{
			case line_abs_literal:
				// This means end of line for us
				mLinePixel=LINE_END;
				return mLinePixel;		// SPEEDUP
				break;
			case line_literal:
				mLineRepeatCount=LineGetBits(4);
				mLineRepeatCount++;
				break;
			case line_packed:
				//
				// From reading in between the lines only a packed line with
				// a zero size i.e 0b00000 as a header is allowable as a packet end
				//
				mLineRepeatCount=LineGetBits(4);
				if(!mLineRepeatCount)
				{
					mLinePixel=LINE_END;
				}
				else
				{
					mLinePixel=mPenIndex[LineGetBits(mSPRCTL0_PixelBits)];
				}
				mLineRepeatCount++;
				break;
			default:
				return 0;
		}
	}

	if(mLinePixel!=LINE_END)
	{
		mLineRepeatCount--;

		switch(mLineType)
		{
			case line_abs_literal:
				mLinePixel=LineGetBits(mSPRCTL0_PixelBits);
				// Check the special case of a zero in the last pixel
				if(!mLineRepeatCount && !mLinePixel)
					mLinePixel=LINE_END;
				else
					mLinePixel=mPenIndex[mLinePixel];
				break;
			case line_literal:
				mLinePixel=mPenIndex[LineGetBits(mSPRCTL0_PixelBits)];
				break;
			case line_packed:
				break;
			default:
				return 0;
		}
	}

	return mLinePixel;
}

inline ULONG CSusie::LineGetBits(ULONG bits)
{
	ULONG retval;

	// Sanity, not really needed
	// if(bits>32) return 0;

	// Only return data IF there is enought bits left in the packet

	if(mLinePacketBitsLeft<bits) return 0;

	// Make sure shift reg has enough bits to fulfil the request

	if(mLineShiftRegCount<bits)
	{
		// This assumes data comes into LSB and out of MSB
//		mLineShiftReg&=0x000000ff;	// Has no effect
		mLineShiftReg<<=24;
		mLineShiftReg|=RAM_PEEK(mTMPADR.Word++)<<16;
		mLineShiftReg|=RAM_PEEK(mTMPADR.Word++)<<8;
		mLineShiftReg|=RAM_PEEK(mTMPADR.Word++);

		mLineShiftRegCount+=24;

		// Increment cycle count for the read
		cycles_used+=3*SPR_RDWR_CYC;
	}

	// Extract the return value
	retval=mLineShiftReg>>(mLineShiftRegCount-bits);
	retval&=(1<<bits)-1;

	// Update internal vars;
	mLineShiftRegCount-=bits;
	mLinePacketBitsLeft-=bits;

	return retval;
}


void CSusie::Poke(ULONG addr,UBYTE data)
{
	switch(addr&0xff)
	{
		case (TMPADRL&0xff):
			mTMPADR.Byte.Low=data;
			mTMPADR.Byte.High=0;
			TRACE_SUSIE2("Poke(TMPADRL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TMPADRH&0xff):
			mTMPADR.Byte.High=data;
			TRACE_SUSIE2("Poke(TMPADRH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TILTACUML&0xff):
			mTILTACUM.Byte.Low=data;
			mTILTACUM.Byte.High=0;
			TRACE_SUSIE2("Poke(TILTACUML,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TILTACUMH&0xff):
			mTILTACUM.Byte.High=data;
			TRACE_SUSIE2("Poke(TILTACUMH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (HOFFL&0xff):
			mHOFF.Byte.Low=data;
			mHOFF.Byte.High=0;
			TRACE_SUSIE2("Poke(HOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (HOFFH&0xff):
			mHOFF.Byte.High=data;
			TRACE_SUSIE2("Poke(HOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VOFFL&0xff):
			mVOFF.Byte.Low=data;
			mVOFF.Byte.High=0;
			TRACE_SUSIE2("Poke(VOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VOFFH&0xff):
			mVOFF.Byte.High=data;
			TRACE_SUSIE2("Poke(VOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VIDBASL&0xff):
			mVIDBAS.Byte.Low=data;
			mVIDBAS.Byte.High=0;
			TRACE_SUSIE2("Poke(VIDBASL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VIDBASH&0xff):
			mVIDBAS.Byte.High=data;
			TRACE_SUSIE2("Poke(VIDBASH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (COLLBASL&0xff):
			mCOLLBAS.Byte.Low=data;
			mCOLLBAS.Byte.High=0;
			TRACE_SUSIE2("Poke(COLLBASL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (COLLBASH&0xff):
			mCOLLBAS.Byte.High=data;
			TRACE_SUSIE2("Poke(COLLBASH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VIDADRL&0xff):
			mVIDADR.Byte.Low=data;
			mVIDADR.Byte.High=0;
			TRACE_SUSIE2("Poke(VIDADRL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VIDADRH&0xff):
			mVIDADR.Byte.High=data;
			TRACE_SUSIE2("Poke(VIDADRH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (COLLADRL&0xff):
			mCOLLADR.Byte.Low=data;
			mCOLLADR.Byte.High=0;
			TRACE_SUSIE2("Poke(COLLADRL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (COLLADRH&0xff):
			mCOLLADR.Byte.High=data;
			TRACE_SUSIE2("Poke(COLLADRH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SCBNEXTL&0xff):
			mSCBNEXT.Byte.Low=data;
			mSCBNEXT.Byte.High=0;
			TRACE_SUSIE2("Poke(SCBNEXTL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SCBNEXTH&0xff):
			mSCBNEXT.Byte.High=data;
			TRACE_SUSIE2("Poke(SCBNEXTH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRDLINEL&0xff):
			mSPRDLINE.Byte.Low=data;
			mSPRDLINE.Byte.High=0;
			TRACE_SUSIE2("Poke(SPRDLINEL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRDLINEH&0xff):
			mSPRDLINE.Byte.High=data;
			TRACE_SUSIE2("Poke(SPRDLINEH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (HPOSSTRTL&0xff):
			mHPOSSTRT.Byte.Low=data;
			mHPOSSTRT.Byte.High=0;
			TRACE_SUSIE2("Poke(HPOSSTRTL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (HPOSSTRTH&0xff):
			mHPOSSTRT.Byte.High=data;
			TRACE_SUSIE2("Poke(HPOSSTRTH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VPOSSTRTL&0xff):
			mVPOSSTRT.Byte.Low=data;
			mVPOSSTRT.Byte.High=0;
			TRACE_SUSIE2("Poke(VPOSSTRTL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VPOSSTRTH&0xff):
			mVPOSSTRT.Byte.High=data;
			TRACE_SUSIE2("Poke(VPOSSTRTH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRHSIZL&0xff):
			mSPRHSIZ.Byte.Low=data;
			mSPRHSIZ.Byte.High=0;
			TRACE_SUSIE2("Poke(SPRHSIZL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRHSIZH&0xff):
			mSPRHSIZ.Byte.High=data;
			TRACE_SUSIE2("Poke(SPRHSIZH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRVSIZL&0xff):
			mSPRVSIZ.Byte.Low=data;
			mSPRVSIZ.Byte.High=0;
			TRACE_SUSIE2("Poke(SPRVSIZL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRVSIZH&0xff):
			mSPRVSIZ.Byte.High=data;
			TRACE_SUSIE2("Poke(SPRVSIZH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (STRETCHL&0xff):
			mSTRETCH.Byte.Low=data;
			mSTRETCH.Byte.High=0;
			TRACE_SUSIE2("Poke(STRETCHL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (STRETCHH&0xff):
			TRACE_SUSIE2("Poke(STRETCHH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			mSTRETCH.Byte.High=data;
			break;
		case (TILTL&0xff):
			mTILT.Byte.Low=data;
			mTILT.Byte.High=0;
			TRACE_SUSIE2("Poke(TILTL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (TILTH&0xff):
			mTILT.Byte.High=data;
			TRACE_SUSIE2("Poke(TILTH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRDOFFL&0xff):
			TRACE_SUSIE2("Poke(SPRDOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			mSPRDOFF.Byte.Low=data;
			mSPRDOFF.Byte.High=0;
			break;
		case (SPRDOFFH&0xff):
			TRACE_SUSIE2("Poke(SPRDOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			mSPRDOFF.Byte.High=data;
			break;
		case (SPRVPOSL&0xff):
			TRACE_SUSIE2("Poke(SPRVPOSL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			mSPRVPOS.Byte.Low=data;
			mSPRVPOS.Byte.High=0;
			break;
		case (SPRVPOSH&0xff):
			mSPRVPOS.Byte.High=data;
			TRACE_SUSIE2("Poke(SPRVPOSH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (COLLOFFL&0xff):
			mCOLLOFF.Byte.Low=data;
			mCOLLOFF.Byte.High=0;
			TRACE_SUSIE2("Poke(COLLOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (COLLOFFH&0xff):
			mCOLLOFF.Byte.High=data;
			TRACE_SUSIE2("Poke(COLLOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VSIZACUML&0xff):
			mVSIZACUM.Byte.Low=data;
			mVSIZACUM.Byte.High=0;
			TRACE_SUSIE2("Poke(VSIZACUML,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VSIZACUMH&0xff):
			mVSIZACUM.Byte.High=data;
			TRACE_SUSIE2("Poke(VSIZACUMH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (HSIZOFFL&0xff):
			mHSIZOFF.Byte.Low=data;
			mHSIZOFF.Byte.High=0;
			TRACE_SUSIE2("Poke(HSIZOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (HSIZOFFH&0xff):
			mHSIZOFF.Byte.High=data;
			TRACE_SUSIE2("Poke(HSIZOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VSIZOFFL&0xff):
			mVSIZOFF.Byte.Low=data;
			mVSIZOFF.Byte.High=0;
			TRACE_SUSIE2("Poke(VSIZOFFL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (VSIZOFFH&0xff):
			mVSIZOFF.Byte.High=data;
			TRACE_SUSIE2("Poke(VSIZOFFH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SCBADRL&0xff):
			mSCBADR.Byte.Low=data;
			mSCBADR.Byte.High=0;
			TRACE_SUSIE2("Poke(SCBADRL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SCBADRH&0xff):
			mSCBADR.Byte.High=data;
			TRACE_SUSIE2("Poke(SCBADRH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (PROCADRL&0xff):
			mPROCADR.Byte.Low=data;
			mPROCADR.Byte.High=0;
			TRACE_SUSIE2("Poke(PROCADRL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (PROCADRH&0xff):
			mPROCADR.Byte.High=data;
			TRACE_SUSIE2("Poke(PROCADRH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;

		case (MATHD&0xff):
			TRACE_SUSIE2("Poke(MATHD,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			mMATHABCD.Bytes.D=data;
//			mMATHABCD.Bytes.C=0;
			// The hardware manual says that the sign shouldnt change
			// but if I dont do this then stun runner will hang as it
			// does the init in the wrong order and if the previous
			// calc left a zero there then we'll get a sign error
			Poke(MATHC,0);
			break;
		case (MATHC&0xff):
			TRACE_SUSIE2("Poke(MATHC,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			mMATHABCD.Bytes.C=data;
			// Perform sign conversion if required
			if(mSPRSYS_SignedMath)
			{
				// Account for the math bug that 0x8000 is +ve & 0x0000 is -ve by subracting 1
				if((mMATHABCD.Words.CD-1)&0x8000)
				{
					UWORD conv;
					conv=mMATHABCD.Words.CD^0xffff;
					conv++;
					mMATHCD_sign=-1;
					TRACE_SUSIE2("MATH CD signed conversion complete %04x to %04x",mMATHABCD.Words.CD,conv);
					mMATHABCD.Words.CD=conv;
				}
				else
				{
					mMATHCD_sign=1;
				}
			}
			break;
		case (MATHB&0xff):
			TRACE_SUSIE2("Poke(MATHB,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			mMATHABCD.Bytes.B=data;
			mMATHABCD.Bytes.A=0;
			break;
		case (MATHA&0xff):
			TRACE_SUSIE2("Poke(MATHA,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			mMATHABCD.Bytes.A=data;
			// Perform sign conversion if required
			if(mSPRSYS_SignedMath)
			{
				// Account for the math bug that 0x8000 is +ve & 0x0000 is -ve by subracting 1
				if((mMATHABCD.Words.AB-1)&0x8000)
				{
					UWORD conv;
					conv=mMATHABCD.Words.AB^0xffff;
					conv++;
					mMATHAB_sign=-1;
					TRACE_SUSIE2("MATH AB signed conversion complete %04x to %04x",mMATHABCD.Words.AB,conv);
					mMATHABCD.Words.AB=conv;
				}
				else
				{
					mMATHAB_sign=1;
				}
			}
			DoMathMultiply();
			break;

		case (MATHP&0xff):
			mMATHNP.Bytes.P=data;
			mMATHNP.Bytes.N=0;
			TRACE_SUSIE2("Poke(MATHP,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (MATHN&0xff):
			mMATHNP.Bytes.N=data;
			TRACE_SUSIE2("Poke(MATHN,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;

		case (MATHH&0xff):
			mMATHEFGH.Bytes.H=data;
			mMATHEFGH.Bytes.G=0;
			TRACE_SUSIE2("Poke(MATHH,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (MATHG&0xff):
			mMATHEFGH.Bytes.G=data;
			TRACE_SUSIE2("Poke(MATHG,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (MATHF&0xff):
			mMATHEFGH.Bytes.F=data;
			mMATHEFGH.Bytes.E=0;
			TRACE_SUSIE2("Poke(MATHF,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (MATHE&0xff):
			mMATHEFGH.Bytes.E=data;
			TRACE_SUSIE2("Poke(MATHE,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			DoMathDivide();
			break;

		case (MATHM&0xff):
			mMATHJKLM.Bytes.M=data;
			mMATHJKLM.Bytes.L=0;
			mSPRSYS_Mathbit=FALSE;
			TRACE_SUSIE2("Poke(MATHM,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (MATHL&0xff):
			mMATHJKLM.Bytes.L=data;
			TRACE_SUSIE2("Poke(MATHL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (MATHK&0xff):
			mMATHJKLM.Bytes.K=data;
			mMATHJKLM.Bytes.J=0;
			TRACE_SUSIE2("Poke(MATHK,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (MATHJ&0xff):
			mMATHJKLM.Bytes.J=data;
			TRACE_SUSIE2("Poke(MATHJ,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;

		case (SPRCTL0&0xff):
			mSPRCTL0_Type=data&0x0007;
			mSPRCTL0_Vflip=data&0x0010;
			mSPRCTL0_Hflip=data&0x0020;
			mSPRCTL0_PixelBits=((data&0x00c0)>>6)+1;
			TRACE_SUSIE2("Poke(SPRCTL0,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRCTL1&0xff):
			mSPRCTL1_StartLeft=data&0x0001;
			mSPRCTL1_StartUp=data&0x0002;
			mSPRCTL1_SkipSprite=data&0x0004;
			mSPRCTL1_ReloadPalette=data&0x0008;
			mSPRCTL1_ReloadDepth=(data&0x0030)>>4;
			mSPRCTL1_Sizing=data&0x0040;	
			mSPRCTL1_Literal=data&0x0080;
			TRACE_SUSIE2("Poke(SPRCTL1,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRCOLL&0xff):
			mSPRCOLL_Number=data&0x000f;
			mSPRCOLL_Collide=data&0x0020;
			TRACE_SUSIE2("Poke(SPRCOLL,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRINIT&0xff):
			mSPRINIT.Byte=data;
			TRACE_SUSIE2("Poke(SPRINIT,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SUZYBUSEN&0xff):
			mSUZYBUSEN=data&0x01;
			TRACE_SUSIE2("Poke(SUZYBUSEN,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRGO&0xff):
			mSPRGO=data&0x01;
			mEVERON=data&0x04;
			TRACE_SUSIE2("Poke(SPRGO,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (SPRSYS&0xff):
			mSPRSYS_StopOnCurrent=data&0x0002;
			if(data&0x0004) mSPRSYS_UnsafeAccess=0;
			mSPRSYS_LeftHand=data&0x0008;
			mSPRSYS_VStretch=data&0x0010;
			mSPRSYS_NoCollide=data&0x0020;
			mSPRSYS_Accumulate=data&0x0040;
			mSPRSYS_SignedMath=data&0x0080;
			TRACE_SUSIE2("Poke(SPRSYS,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;

// Cartridge writing ports

		case (RCART0&0xff):
			mSystem.Poke_CARTB0(data);
			TRACE_SUSIE2("Poke(RCART0,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
		case (RCART1&0xff):
			mSystem.Poke_CARTB1(data);
			TRACE_SUSIE2("Poke(RCART1,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;
			
// These are not so important, so lets ignore them for the moment

		case (LEDS&0xff):
		case (PPORTSTAT&0xff):
		case (PPORTDATA&0xff):
		case (HOWIE&0xff):
			TRACE_SUSIE2("Poke(LEDS/PPORTSTST/PPORTDATA/HOWIE,%02x) at PC=$%04x",data,mSystem.mCpu->GetPC());
			break;

// Errors on read only register accesses

		case (SUZYHREV&0xff):
		case (JOYSTICK&0xff):
		case (SWITCHES&0xff):
			TRACE_SUSIE3("Poke(%04x,%02x) - Poke to read only register location at PC=%04x",addr,data,mSystem.mCpu->GetPC());
			break;

// Errors on illegal location accesses

		default:
			TRACE_SUSIE3("Poke(%04x,%02x) - Poke to illegal location at PC=%04x",addr,data,mSystem.mCpu->GetPC());
			break;
	}
}

UBYTE CSusie::Peek(ULONG addr)
{
	UBYTE	retval=0;
	switch(addr&0xff)
	{
		case (TMPADRL&0xff):
			retval=mTMPADR.Byte.Low;
			TRACE_SUSIE2("Peek(TMPADRL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (TMPADRH&0xff):
			retval=mTMPADR.Byte.High;
			TRACE_SUSIE2("Peek(TMPADRH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (TILTACUML&0xff):
			retval=mTILTACUM.Byte.Low;
			TRACE_SUSIE2("Peek(TILTACUML)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (TILTACUMH&0xff):
			retval=mTILTACUM.Byte.High;
			TRACE_SUSIE2("Peek(TILTACUMH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (HOFFL&0xff):
			retval=mHOFF.Byte.Low;
			TRACE_SUSIE2("Peek(HOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (HOFFH&0xff):
			retval=mHOFF.Byte.High;
			TRACE_SUSIE2("Peek(HOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VOFFL&0xff):
			retval=mVOFF.Byte.Low;
			TRACE_SUSIE2("Peek(VOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VOFFH&0xff):
			retval=mVOFF.Byte.High;
			TRACE_SUSIE2("Peek(VOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VIDBASL&0xff):
			retval=mVIDBAS.Byte.Low;
			TRACE_SUSIE2("Peek(VIDBASL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VIDBASH&0xff):
			retval=mVIDBAS.Byte.High;
			TRACE_SUSIE2("Peek(VIDBASH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (COLLBASL&0xff):
			retval=mCOLLBAS.Byte.Low;
			TRACE_SUSIE2("Peek(COLLBASL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (COLLBASH&0xff):
			retval=mCOLLBAS.Byte.High;
			TRACE_SUSIE2("Peek(COLLBASH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VIDADRL&0xff):
			retval=mVIDADR.Byte.Low;
			TRACE_SUSIE2("Peek(VIDADRL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VIDADRH&0xff):
			retval=mVIDADR.Byte.High;
			TRACE_SUSIE2("Peek(VIDADRH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (COLLADRL&0xff):
			retval=mCOLLADR.Byte.Low;
			TRACE_SUSIE2("Peek(COLLADRL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (COLLADRH&0xff):
			retval=mCOLLADR.Byte.High;
			TRACE_SUSIE2("Peek(COLLADRH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SCBNEXTL&0xff):
			retval=mSCBNEXT.Byte.Low;
			TRACE_SUSIE2("Peek(SCBNEXTL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SCBNEXTH&0xff):
			retval=mSCBNEXT.Byte.High;
			TRACE_SUSIE2("Peek(SCBNEXTH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SPRDLINEL&0xff):
			retval=mSPRDLINE.Byte.Low;
			TRACE_SUSIE2("Peek(SPRDLINEL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SPRDLINEH&0xff):
			retval=mSPRDLINE.Byte.High;
			TRACE_SUSIE2("Peek(SPRDLINEH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (HPOSSTRTL&0xff):
			retval=mHPOSSTRT.Byte.Low;
			TRACE_SUSIE2("Peek(HPOSSTRTL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (HPOSSTRTH&0xff):
			retval=mHPOSSTRT.Byte.High;
			TRACE_SUSIE2("Peek(HPOSSTRTH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VPOSSTRTL&0xff):
			retval=mVPOSSTRT.Byte.Low;
			TRACE_SUSIE2("Peek(VPOSSTRTL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VPOSSTRTH&0xff):
			retval=mVPOSSTRT.Byte.High;
			TRACE_SUSIE2("Peek(VPOSSTRTH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SPRHSIZL&0xff):
			retval=mSPRHSIZ.Byte.Low;
			TRACE_SUSIE2("Peek(SPRHSIZL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SPRHSIZH&0xff):
			retval=mSPRHSIZ.Byte.High;
			TRACE_SUSIE2("Peek(SPRHSIZH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SPRVSIZL&0xff):
			retval=mSPRVSIZ.Byte.Low;
			TRACE_SUSIE2("Peek(SPRVSIZL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SPRVSIZH&0xff):
			retval=mSPRVSIZ.Byte.High;
			TRACE_SUSIE2("Peek(SPRVSIZH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (STRETCHL&0xff):
			retval=mSTRETCH.Byte.Low;
			TRACE_SUSIE2("Peek(STRETCHL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (STRETCHH&0xff):
			retval=mSTRETCH.Byte.High;
			TRACE_SUSIE2("Peek(STRETCHH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (TILTL&0xff):
			retval=mTILT.Byte.Low;
			TRACE_SUSIE2("Peek(TILTL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (TILTH&0xff):
			retval=mTILT.Byte.High;
			TRACE_SUSIE2("Peek(TILTH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SPRDOFFL&0xff):
			retval=mSPRDOFF.Byte.Low;
			TRACE_SUSIE2("Peek(SPRDOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SPRDOFFH&0xff):
			retval=mSPRDOFF.Byte.High;
			TRACE_SUSIE2("Peek(SPRDOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SPRVPOSL&0xff):
			retval=mSPRVPOS.Byte.Low;
			TRACE_SUSIE2("Peek(SPRVPOSL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SPRVPOSH&0xff):
			retval=mSPRVPOS.Byte.High;
			TRACE_SUSIE2("Peek(SPRVPOSH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (COLLOFFL&0xff):
			retval=mCOLLOFF.Byte.Low;
			TRACE_SUSIE2("Peek(COLLOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (COLLOFFH&0xff):
			retval=mCOLLOFF.Byte.High;
			TRACE_SUSIE2("Peek(COLLOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VSIZACUML&0xff):
			retval=mVSIZACUM.Byte.Low;
			TRACE_SUSIE2("Peek(VSIZACUML)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VSIZACUMH&0xff):
			retval=mVSIZACUM.Byte.High;
			TRACE_SUSIE2("Peek(VSIZACUMH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (HSIZOFFL&0xff):
			retval=mHSIZOFF.Byte.Low;
			TRACE_SUSIE2("Peek(HSIZOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (HSIZOFFH&0xff):
			retval=mHSIZOFF.Byte.High;
			TRACE_SUSIE2("Peek(HSIZOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VSIZOFFL&0xff):
			retval=mVSIZOFF.Byte.Low;
			TRACE_SUSIE2("Peek(VSIZOFFL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (VSIZOFFH&0xff):
			retval=mVSIZOFF.Byte.High;
			TRACE_SUSIE2("Peek(VSIZOFFH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SCBADRL&0xff):
			retval=mSCBADR.Byte.Low;
			TRACE_SUSIE2("Peek(SCBADRL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (SCBADRH&0xff):
			retval=mSCBADR.Byte.High;
			TRACE_SUSIE2("Peek(SCBADRH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (PROCADRL&0xff):
			retval=mPROCADR.Byte.Low;
			TRACE_SUSIE2("Peek(PROCADRL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (PROCADRH&0xff):
			retval=mPROCADR.Byte.High;
			TRACE_SUSIE2("Peek(PROCADRH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;

		case (MATHD&0xff):
			retval=mMATHABCD.Bytes.D;
			TRACE_SUSIE2("Peek(MATHD)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (MATHC&0xff):
			retval=mMATHABCD.Bytes.C;
			TRACE_SUSIE2("Peek(MATHC)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (MATHB&0xff):
			retval=mMATHABCD.Bytes.B;
			TRACE_SUSIE2("Peek(MATHB)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (MATHA&0xff):
			retval=mMATHABCD.Bytes.A;
			TRACE_SUSIE2("Peek(MATHA)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;

		case (MATHP&0xff):
			retval=mMATHNP.Bytes.P;
			TRACE_SUSIE2("Peek(MATHP)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (MATHN&0xff):
			retval=mMATHNP.Bytes.N;
			TRACE_SUSIE2("Peek(MATHN)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;

		case (MATHH&0xff):
			retval=mMATHEFGH.Bytes.H;
			TRACE_SUSIE2("Peek(MATHH)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (MATHG&0xff):
			retval=mMATHEFGH.Bytes.G;
			TRACE_SUSIE2("Peek(MATHG)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (MATHF&0xff):
			retval=mMATHEFGH.Bytes.F;
			TRACE_SUSIE2("Peek(MATHF)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (MATHE&0xff):
			retval=mMATHEFGH.Bytes.E;
			TRACE_SUSIE2("Peek(MATHE)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;

		case (MATHM&0xff):
			retval=mMATHJKLM.Bytes.M;
			TRACE_SUSIE2("Peek(MATHM)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (MATHL&0xff):
			retval=mMATHJKLM.Bytes.L;
			TRACE_SUSIE2("Peek(MATHL)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (MATHK&0xff):
			retval=mMATHJKLM.Bytes.K;
			TRACE_SUSIE2("Peek(MATHK)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (MATHJ&0xff):
			retval=mMATHJKLM.Bytes.J;
			TRACE_SUSIE2("Peek(MATHJ)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;

		case (SUZYHREV&0xff):
			retval=0x01;
			TRACE_SUSIE2("Peek(SUZYHREV)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;

		case (SPRSYS&0xff):
			retval=0x0000;
			//	retval+=(mSPRSYS_Status)?0x0001:0x0000;
			// Use gSystemCPUSleep to signal the status instead, if we are asleep then
			// we must be rendering sprites
			retval+=(gSystemCPUSleep)?0x0001:0x0000;
			retval+=(mSPRSYS_StopOnCurrent)?0x0002:0x0000;
			retval+=(mSPRSYS_UnsafeAccess)?0x0004:0x0000;
			retval+=(mSPRSYS_LeftHand)?0x0008:0x0000;
			retval+=(mSPRSYS_VStretch)?0x0010:0x0000;
			retval+=(mSPRSYS_LastCarry)?0x0020:0x0000;
			retval+=(mSPRSYS_Mathbit)?0x0040:0x0000;
			retval+=(mSPRSYS_MathInProgress)?0x0080:0x0000;
			TRACE_SUSIE2("Peek(SPRSYS)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;

		case (JOYSTICK&0xff):
			if(mSPRSYS_LeftHand)
			{
				retval= mJOYSTICK.Byte;
			}
			else
			{
				TJOYSTICK Modified=mJOYSTICK;
				Modified.Bits.Left=mJOYSTICK.Bits.Right;
				Modified.Bits.Right=mJOYSTICK.Bits.Left;
				Modified.Bits.Down=mJOYSTICK.Bits.Up;
				Modified.Bits.Up=mJOYSTICK.Bits.Down;
				retval= Modified.Byte;
			}
//			TRACE_SUSIE2("Peek(JOYSTICK)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;


		case (SWITCHES&0xff):
			retval=mSWITCHES.Byte;
//			TRACE_SUSIE2("Peek(SWITCHES)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;

// Cartridge reading ports

		case (RCART0&0xff):
			retval=mSystem.Peek_CARTB0();
//			TRACE_SUSIE2("Peek(RCART0)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;
		case (RCART1&0xff):
			retval=mSystem.Peek_CARTB1();
//			TRACE_SUSIE2("Peek(RCART1)=$%02x at PC=$%04x",retval,mSystem.mCpu->GetPC());
			return retval;
			break;

// These are no so important so lets ignore them for the moment

		case (LEDS&0xff):
		case (PPORTSTAT&0xff):
		case (PPORTDATA&0xff):
		case (HOWIE&0xff):
			TRACE_SUSIE1("Peek(LEDS/PPORTSTAT/PPORTDATA) at PC=$%04x",mSystem.mCpu->GetPC());
			break;

// Errors on write only register accesses

		case (SPRCTL0&0xff):
		case (SPRCTL1&0xff):
		case (SPRCOLL&0xff):
		case (SPRINIT&0xff):
		case (SUZYBUSEN&0xff):
		case (SPRGO&0xff):
			TRACE_SUSIE2("Peek(%04x) - Peek from write only register location at PC=$%04x",addr,mSystem.mCpu->GetPC());
			break;
		
// Errors on illegal location accesses

		default:
			TRACE_SUSIE2("Peek(%04x) - Peek from illegal location at PC=$%04x",addr,mSystem.mCpu->GetPC());
			break;
	}

	return 0xff;
}

