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
//	                  Handy - An Atari Lynx Emulator                     //
//                          Copyright (c) 1996,1997                         //
//                                 K. Wilkins                               //
//////////////////////////////////////////////////////////////////////////////
// Susie object header file                                                 //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition for the Suzy class    //
// which provides math and sprite support to the emulator                   //
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

#ifndef SUSIE_H
#define SUSIE_H

#ifdef TRACE_SUSIE

#define TRACE_SUSIE0(msg)					_RPT1(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",gSystemCycleCount)
#define TRACE_SUSIE1(msg,arg1)				_RPT2(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",arg1,gSystemCycleCount)
#define TRACE_SUSIE2(msg,arg1,arg2)			_RPT3(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",arg1,arg2,gSystemCycleCount)
#define TRACE_SUSIE3(msg,arg1,arg2,arg3)	_RPT4(_CRT_WARN,"CSusie::"msg" (Time=%012d)\n",arg1,arg2,arg3,gSystemCycleCount)

#else

#define TRACE_SUSIE0(msg)
#define TRACE_SUSIE1(msg,arg1)
#define TRACE_SUSIE2(msg,arg1,arg2)
#define TRACE_SUSIE3(msg,arg1,arg2,arg3)

#endif

class CSystem;

#define SUSIE_START		0xfc00
#define SUSIE_SIZE		0x100

#define SCREEN_WIDTH	160
#define SCREEN_HEIGHT	102

#define LINE_END		0x80

//
// Define button values
//

#define BUTTON_A		0x0001
#define BUTTON_B		0x0002
#define BUTTON_OPT2		0x0004
#define BUTTON_OPT1		0x0008
#define BUTTON_LEFT		0x0010
#define BUTTON_RIGHT	0x0020
#define BUTTON_UP		0x0040
#define BUTTON_DOWN		0x0080
#define BUTTON_PAUSE	0x0100


enum {line_error=0,line_abs_literal,line_literal,line_packed};
enum {math_finished=0,math_divide,math_multiply,math_init_divide,math_init_multiply};

enum {sprite_background_shadow=0,
      sprite_background_noncollide,
      sprite_boundary_shadow,
      sprite_boundary,
	  sprite_normal,
	  sprite_noncollide,
	  sprite_xor_shadow,
	  sprite_shadow};

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			uint8	Fc1:1;
			uint8	Fc2:1;
			uint8	Fc3:1;
			uint8	reserved:1;
			uint8	Ac1:1;
			uint8	Ac2:1;
			uint8	Ac3:1;
			uint8	Ac4:1;
#else
			uint8	Ac4:1;
			uint8	Ac3:1;
			uint8	Ac2:1;
			uint8	Ac1:1;
			uint8	reserved:1;
			uint8	Fc3:1;
			uint8	Fc2:1;
			uint8	Fc1:1;
#endif
		}Bits;
		uint8	Byte;
	};
}TSPRINIT;

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			uint8	Up:1;
			uint8	Down:1;
			uint8	Left:1;
			uint8	Right:1;
			uint8	Option1:1;
			uint8	Option2:1;
			uint8	Inside:1;
			uint8	Outside:1;
#else
			uint8	Outside:1;
			uint8	Inside:1;
			uint8	Option2:1;
			uint8	Option1:1;
			uint8	Right:1;
			uint8	Left:1;
			uint8	Down:1;
			uint8	Up:1;
#endif
		}Bits;
		uint8	Byte;
	};
}TJOYSTICK;

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			uint8	spare:5;
			uint8	Cart1IO:1;
			uint8	Cart0IO:1;
			uint8	Pause:1;
#else
			uint8	Pause:1;
			uint8	Cart0IO:1;
			uint8	Cart1IO:1;
			uint8	spare:5;
#endif
		}Bits;
		uint8	Byte;
	};
}TSWITCHES;

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			uint8	A;
			uint8	B;
			uint8	C;
			uint8	D;
#else
			uint8	D;
			uint8	C;
			uint8	B;
			uint8	A;
#endif
		}Bytes;
		struct
		{
#ifdef MSB_FIRST
			uint16	AB;
			uint16	CD;
#else
			uint16	CD;
			uint16	AB;
#endif
		}Words;
		uint32	Long;
	};
}TMATHABCD;

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			uint8	E;
			uint8	F;
			uint8	G;
			uint8	H;
#else
			uint8	H;
			uint8	G;
			uint8	F;
			uint8	E;
#endif
		}Bytes;
		struct
		{
#ifdef MSB_FIRST
			uint16	EF;
			uint16	GH;
#else
			uint16	GH;
			uint16	EF;
#endif
		}Words;
		uint32	Long;
	};
}TMATHEFGH;

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			uint8	J;
			uint8	K;
			uint8	L;
			uint8	M;
#else
			uint8	M;
			uint8	L;
			uint8	K;
			uint8	J;
#endif
		}Bytes;
		struct
		{
#ifdef MSB_FIRST
			uint16	JK;
			uint16	LM;
#else
			uint16	LM;
			uint16	JK;
#endif
		}Words;
		uint32	Long;
	};
}TMATHJKLM;

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			uint8	xx2;
			uint8	xx1;
			uint8	N;
			uint8	P;
#else
			uint8	P;
			uint8	N;
			uint8	xx1;
			uint8	xx2;
#endif
		}Bytes;
		struct
		{
#ifdef MSB_FIRST
			uint16	xx1;
			uint16	NP;
#else
			uint16	NP;
			uint16	xx1;
#endif
		}Words;
		uint32	Long;
	};
}TMATHNP;


class CSusie : public CLynxBase
{
	public:
		CSusie(CSystem& parent);
		~CSusie();

		void	Reset(void);

		uint8	Peek(uint32 addr);
		void	Poke(uint32 addr,uint8 data);
		uint32	ReadCycle(void) {return 9;};
		uint32	WriteCycle(void) {return 5;};
		uint32	ObjectSize(void) {return SUSIE_SIZE;};

		void	SetButtonData(uint32 data) {mJOYSTICK.Byte=(uint8)data;mSWITCHES.Byte=(uint8)(data>>8);};
		uint32	GetButtonData(void) {return mJOYSTICK.Byte+(mSWITCHES.Byte<<8);};

		uint32	PaintSprites(void);

		int	StateAction(StateMem *sm, int load, int data_only);

	private:
		void	DoMathDivide(void);
		void	DoMathMultiply(void);
		uint32	LineInit(uint32 voff);
		uint32	LineGetPixel(void);
		uint32	LineGetBits(uint32 bits);

		void	ProcessPixel(uint32 hoff,uint32 pixel);
		void	WritePixel(uint32 hoff,uint32 pixel);
		uint32	ReadPixel(uint32 hoff);
		void	WriteCollision(uint32 hoff,uint32 pixel);
		uint32	ReadCollision(uint32 hoff);

	private:
		CSystem&	mSystem;

		Uuint16		mTMPADR;		// ENG
		Uuint16		mTILTACUM;		// ENG
		Uuint16		mHOFF;			// CPU
		Uuint16		mVOFF;			// CPU
		Uuint16		mVIDBAS;		// CPU
		Uuint16		mCOLLBAS;		// CPU
		Uuint16		mVIDADR;		// ENG
		Uuint16		mCOLLADR;		// ENG
		Uuint16		mSCBNEXT;		// SCB
		Uuint16		mSPRDLINE;		// SCB
		Uuint16		mHPOSSTRT;		// SCB
		Uuint16		mVPOSSTRT;		// SCB
		Uuint16		mSPRHSIZ;		// SCB
		Uuint16		mSPRVSIZ;		// SCB
		Uuint16		mSTRETCH;		// ENG
		Uuint16		mTILT;			// ENG
		Uuint16		mSPRDOFF;		// ENG
		Uuint16		mSPRVPOS;		// ENG
		Uuint16		mCOLLOFF;		// CPU
		Uuint16		mVSIZACUM;		// ENG
		Uuint16		mHSIZACUM;		//    K.s creation
		Uuint16		mHSIZOFF;		// CPU
		Uuint16		mVSIZOFF;		// CPU
		Uuint16		mSCBADR;		// ENG
		Uuint16		mPROCADR;		// ENG

		TMATHABCD	mMATHABCD;		// ENG
		TMATHEFGH	mMATHEFGH;		// ENG
		TMATHJKLM	mMATHJKLM;		// ENG
		TMATHNP		mMATHNP;		// ENG
		int			mMATHAB_sign;
		int			mMATHCD_sign;
		int			mMATHEFGH_sign;

		int			mSPRCTL0_Type;			// SCB
		int			mSPRCTL0_Vflip;
		int			mSPRCTL0_Hflip;
		int			mSPRCTL0_PixelBits;

		int			mSPRCTL1_StartLeft;		// SCB
		int			mSPRCTL1_StartUp;
		int			mSPRCTL1_SkipSprite;
		int			mSPRCTL1_ReloadPalette;
		int			mSPRCTL1_ReloadDepth;
		int			mSPRCTL1_Sizing;
		int			mSPRCTL1_Literal;

		int			mSPRCOLL_Number;		//CPU
		int			mSPRCOLL_Collide;

		int			mSPRSYS_StopOnCurrent;	//CPU
		int			mSPRSYS_LeftHand;
		int			mSPRSYS_VStretch;
		int			mSPRSYS_NoCollide;
		int			mSPRSYS_Accumulate;
		int			mSPRSYS_SignedMath;
		int			mSPRSYS_Status;
		int			mSPRSYS_UnsafeAccess;
		int			mSPRSYS_LastCarry;
		int			mSPRSYS_Mathbit;
		int			mSPRSYS_MathInProgress;

		uint32		mSUZYBUSEN;		// CPU

		TSPRINIT	mSPRINIT;		// CPU

		uint32		mSPRGO;			// CPU
		int			mEVERON;

		uint8		mPenIndex[16];	// SCB

		// Line rendering related variables

		uint32		mLineType;
		uint32		mLineShiftRegCount;
		uint32		mLineShiftReg;
		uint32		mLineRepeatCount;
		uint32		mLinePixel;
		uint32		mLinePacketBitsLeft;

		int			mCollision;

		uint8		*mRamPointer;

		uint32		mLineBaseAddress;
		uint32		mLineCollisionAddress;

	        int hquadoff, vquadoff;

		// Joystick switches

		TJOYSTICK	mJOYSTICK;
		TSWITCHES	mSWITCHES;
};

#endif

