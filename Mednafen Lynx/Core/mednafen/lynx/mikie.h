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
// Mikey class header file                                                  //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This header file provides the interface definition and some of the code  //
// for the Mikey chip within the Lynx. The most crucial code is the         //
// Update() function which as you can probably guess updates all of the     //
// Mikey hardware counters and screen DMA from the prevous time it was      //
// called. Yes I know how to spell Mikey but I cant be bothered to change   //
// it everywhere.                                                           //
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

#ifndef MIKIE_H
#define MIKIE_H

//#include <crtdbg.h>
//#define	TRACE_MIKIE
#include <math.h>

#ifdef TRACE_MIKIE

#define TRACE_MIKIE0(msg)					_RPT1(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",gSystemCycleCount)
#define TRACE_MIKIE1(msg,arg1)				_RPT2(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",arg1,gSystemCycleCount)
#define TRACE_MIKIE2(msg,arg1,arg2)			_RPT3(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",arg1,arg2,gSystemCycleCount)
#define TRACE_MIKIE3(msg,arg1,arg2,arg3)	_RPT4(_CRT_WARN,"CMikie::"msg" (Time=%012d)\n",arg1,arg2,arg3,gSystemCycleCount)

#else

#define TRACE_MIKIE0(msg)
#define TRACE_MIKIE1(msg,arg1)
#define TRACE_MIKIE2(msg,arg1,arg2)
#define TRACE_MIKIE3(msg,arg1,arg2,arg3)

#endif

class CSystem;

#define MIKIE_START	0xfd00
#define MIKIE_SIZE	0x100

//
// Define counter types and defines
//

#define CTRL_A_IRQEN	0x80
#define CTRL_A_RTD		0x40
#define CTRL_A_RELOAD	0x10
#define CTRL_A_COUNT	0x08
#define CTRL_A_DIVIDE	0x07

#define CTRL_B_TDONE	0x08
#define CTRL_B_LASTCK	0x04
#define CTRL_B_CIN		0x02
#define CTRL_B_COUT		0x01

#define LINE_TIMER		0x00
#define SCREEN_TIMER	0x02

#define LINE_WIDTH		160
#define	LINE_SIZE		80

#define UART_TX_INACTIVE	0x80000000
#define UART_RX_INACTIVE	0x80000000
#define UART_BREAK_CODE		0x00008000
#define	UART_MAX_RX_QUEUE	32
#define UART_TX_TIME_PERIOD	(11)
#define UART_RX_TIME_PERIOD	(11)
#define UART_RX_NEXT_DELAY	(44)

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			uint8 unused:4;
			uint8 Colour:1;
			uint8 FourColour:1;
			uint8 Flip:1;
			uint8 DMAEnable:1;
#else

			uint8 DMAEnable:1;
			uint8 Flip:1;
			uint8 FourColour:1;
			uint8 Colour:1;
			uint8 unused:4;
#endif
		}Bits;
		uint8 Byte;
	};
}TDISPCTL;

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			uint8 unused:8;
			uint8 unused2:8;
			uint8 unused3:4;
			uint8 Blue:4;
			uint8 Red:4;
			uint8 Green:4;
#else
			uint8 Green:4;
			uint8 Red:4;
			uint8 Blue:4;
#endif
          }Colours;
          uint32     Index;
     };
}TPALETTE;


//
// Emumerated types for possible mikie windows independant modes
//
enum
{
	MIKIE_BAD_MODE=0,
	MIKIE_NO_ROTATE,
	MIKIE_ROTATE_L,
	MIKIE_ROTATE_R
};

enum
{
	MIKIE_PIXEL_FORMAT_8BPP=0,
	MIKIE_PIXEL_FORMAT_16BPP_555,
	MIKIE_PIXEL_FORMAT_16BPP_565,
	MIKIE_PIXEL_FORMAT_24BPP,
	MIKIE_PIXEL_FORMAT_32BPP,
};

#include <blip/Blip_Buffer.h>

typedef Blip_Synth<blip_good_quality, 256 * 4> Synth;

class CMikie : public CLynxBase
{
	public:
		CMikie(CSystem& parent);
		~CMikie();
	
		uint32 startTS;
		Synth miksynth;
		Blip_Buffer mikbuf;

		void	Reset(void);

		uint8	Peek(uint32 addr);
		void	Poke(uint32 addr,uint8 data);
		uint32	ReadCycle(void) {return 5;};
		uint32	WriteCycle(void) {return 5;};
		uint32	ObjectSize(void) {return MIKIE_SIZE;};
		void	PresetForHomebrew(void);
		uint32	GetLfsrNext(uint32 current);

		void	ComLynxCable(int status);
		void	ComLynxRxData(int data);
		void	ComLynxTxLoopback(int data);
		void	ComLynxTxCallback(void (*function)(int data,uint32 objref),uint32 objref);
		
		void	DisplaySetAttributes(const MDFN_PixelFormat &format, uint32 Pitch);
		
		void	BlowOut(void);

		uint32	DisplayRenderLine(void);
		uint32	DisplayEndOfFrame(void);

		int StateAction(StateMem *sm, int load, int data_only);

		inline void SetCPUSleep(void) {gSystemCPUSleep=TRUE;};
		inline void ClearCPUSleep(void) {gSystemCPUSleep=FALSE;};

		void CombobulateSound(uint32 teatime);
		void Update(void);

		bool		mpSkipFrame;
                uint32         *mpDisplayCurrent;
		uint32		mpDisplayCurrentLine;

	private:
		CSystem		&mSystem;

		// Hardware storage
		
		uint32		mDisplayAddress;
		uint32		mAudioInputComparator;
		uint32		mTimerStatusFlags;
		uint32		mTimerInterruptMask;

		TPALETTE	mPalette[16];
		uint32		mColourMap[4096];

		uint32		mIODAT;
		uint32		mIODIR;
		uint32		mIODAT_REST_SIGNAL;

		uint32		mDISPCTL_DMAEnable;
		uint32		mDISPCTL_Flip;
		uint32		mDISPCTL_FourColour;
		uint32		mDISPCTL_Colour;

		uint32		mTIM_0_BKUP;
		uint32		mTIM_0_ENABLE_RELOAD;
		uint32		mTIM_0_ENABLE_COUNT;
		uint32		mTIM_0_LINKING;
		uint32		mTIM_0_CURRENT;
		uint32		mTIM_0_TIMER_DONE;
		uint32		mTIM_0_LAST_CLOCK;
		uint32		mTIM_0_BORROW_IN;
		uint32		mTIM_0_BORROW_OUT;
		uint32		mTIM_0_LAST_LINK_CARRY;
		uint32		mTIM_0_LAST_COUNT;

		uint32		mTIM_1_BKUP;
		uint32		mTIM_1_ENABLE_RELOAD;
		uint32		mTIM_1_ENABLE_COUNT;
		uint32		mTIM_1_LINKING;
		uint32		mTIM_1_CURRENT;
		uint32		mTIM_1_TIMER_DONE;
		uint32		mTIM_1_LAST_CLOCK;
		uint32		mTIM_1_BORROW_IN;
		uint32		mTIM_1_BORROW_OUT;
		uint32		mTIM_1_LAST_LINK_CARRY;
		uint32		mTIM_1_LAST_COUNT;

		uint32		mTIM_2_BKUP;
		uint32		mTIM_2_ENABLE_RELOAD;
		uint32		mTIM_2_ENABLE_COUNT;
		uint32		mTIM_2_LINKING;
		uint32		mTIM_2_CURRENT;
		uint32		mTIM_2_TIMER_DONE;
		uint32		mTIM_2_LAST_CLOCK;
		uint32		mTIM_2_BORROW_IN;
		uint32		mTIM_2_BORROW_OUT;
		uint32		mTIM_2_LAST_LINK_CARRY;
		uint32		mTIM_2_LAST_COUNT;

		uint32		mTIM_3_BKUP;
		uint32		mTIM_3_ENABLE_RELOAD;
		uint32		mTIM_3_ENABLE_COUNT;
		uint32		mTIM_3_LINKING;
		uint32		mTIM_3_CURRENT;
		uint32		mTIM_3_TIMER_DONE;
		uint32		mTIM_3_LAST_CLOCK;
		uint32		mTIM_3_BORROW_IN;
		uint32		mTIM_3_BORROW_OUT;
		uint32		mTIM_3_LAST_LINK_CARRY;
		uint32		mTIM_3_LAST_COUNT;

		uint32		mTIM_4_BKUP;
		uint32		mTIM_4_ENABLE_RELOAD;
		uint32		mTIM_4_ENABLE_COUNT;
		uint32		mTIM_4_LINKING;
		uint32		mTIM_4_CURRENT;
		uint32		mTIM_4_TIMER_DONE;
		uint32		mTIM_4_LAST_CLOCK;
		uint32		mTIM_4_BORROW_IN;
		uint32		mTIM_4_BORROW_OUT;
		uint32		mTIM_4_LAST_LINK_CARRY;
		uint32		mTIM_4_LAST_COUNT;

		uint32		mTIM_5_BKUP;
		uint32		mTIM_5_ENABLE_RELOAD;
		uint32		mTIM_5_ENABLE_COUNT;
		uint32		mTIM_5_LINKING;
		uint32		mTIM_5_CURRENT;
		uint32		mTIM_5_TIMER_DONE;
		uint32		mTIM_5_LAST_CLOCK;
		uint32		mTIM_5_BORROW_IN;
		uint32		mTIM_5_BORROW_OUT;
		uint32		mTIM_5_LAST_LINK_CARRY;
		uint32		mTIM_5_LAST_COUNT;

		uint32		mTIM_6_BKUP;
		uint32		mTIM_6_ENABLE_RELOAD;
		uint32		mTIM_6_ENABLE_COUNT;
		uint32		mTIM_6_LINKING;
		uint32		mTIM_6_CURRENT;
		uint32		mTIM_6_TIMER_DONE;
		uint32		mTIM_6_LAST_CLOCK;
		uint32		mTIM_6_BORROW_IN;
		uint32		mTIM_6_BORROW_OUT;
		uint32		mTIM_6_LAST_LINK_CARRY;
		uint32		mTIM_6_LAST_COUNT;

		uint32		mTIM_7_BKUP;
		uint32		mTIM_7_ENABLE_RELOAD;
		uint32		mTIM_7_ENABLE_COUNT;
		uint32		mTIM_7_LINKING;
		uint32		mTIM_7_CURRENT;
		uint32		mTIM_7_TIMER_DONE;
		uint32		mTIM_7_LAST_CLOCK;
		uint32		mTIM_7_BORROW_IN;
		uint32		mTIM_7_BORROW_OUT;
		uint32		mTIM_7_LAST_LINK_CARRY;
		uint32		mTIM_7_LAST_COUNT;

		uint32		mAUDIO_BKUP[4];
		uint32		mAUDIO_ENABLE_RELOAD[4];
		uint32		mAUDIO_ENABLE_COUNT[4];
		uint32		mAUDIO_LINKING[4];
		uint32		mAUDIO_CURRENT[4];
		uint32		mAUDIO_TIMER_DONE[4];
		uint32		mAUDIO_LAST_CLOCK[4];
		uint32		mAUDIO_BORROW_IN[4];
		uint32		mAUDIO_BORROW_OUT[4];
		uint32		mAUDIO_LAST_LINK_CARRY[4];
		uint32		mAUDIO_LAST_COUNT[4];
		int8		mAUDIO_VOLUME[4];
		uint32		mAUDIO_INTEGRATE_ENABLE[4];
		uint32		mAUDIO_WAVESHAPER[4];

		int8		mAUDIO_OUTPUT[4];
                uint8           mAUDIO_ATTEN[4];
		uint32		mSTEREO;
		uint32		mPAN;

		//
		// Serial related variables
		//
		uint32		mUART_RX_IRQ_ENABLE;
		uint32		mUART_TX_IRQ_ENABLE;

		uint32		mUART_RX_COUNTDOWN;
		uint32		mUART_TX_COUNTDOWN;

		uint32		mUART_SENDBREAK;
		uint32		mUART_TX_DATA;
		uint32		mUART_RX_DATA;
		uint32		mUART_RX_READY;

		uint32		mUART_PARITY_ENABLE;
		uint32		mUART_PARITY_EVEN;

		int			mUART_CABLE_PRESENT;
		void		(*mpUART_TX_CALLBACK)(int data,uint32 objref);
		uint32		mUART_TX_CALLBACK_OBJECT;

		int			mUART_Rx_input_queue[UART_MAX_RX_QUEUE];
		unsigned int mUART_Rx_input_ptr;
		unsigned int mUART_Rx_output_ptr;
		int			mUART_Rx_waiting;
		int			mUART_Rx_framing_error;
		int			mUART_Rx_overun_error;

		//
		// Screen related
		//
		
		uint8		*mpRamPointer;
		uint32		mLynxLine;
		uint32		mLynxLineDMACounter;
		uint32		mLynxAddr;

		uint32		mDisplayRotate;
		uint32		mDisplayFormat;
		uint32		mDisplayPitch;
		uint8*		(*mpDisplayCallback)(uint32 objref);
		uint32		mDisplayCallbackObject;
};


#endif

