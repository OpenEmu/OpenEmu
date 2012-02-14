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
	UBYTE	backup;
	UBYTE	count;
	UBYTE	controlA;
	UBYTE	controlB;
	bool	linkedlastcarry;
}MTIMER;

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			UBYTE unused:4;
			UBYTE Colour:1;
			UBYTE FourColour:1;
			UBYTE Flip:1;
			UBYTE DMAEnable:1;
#else
            
			UBYTE DMAEnable:1;
			UBYTE Flip:1;
			UBYTE FourColour:1;
			UBYTE Colour:1;
			UBYTE unused:4;
#endif
		}Bits;
		UBYTE Byte;
	};
}TDISPCTL;

typedef struct
{
	union
	{
		struct
		{
#ifdef MSB_FIRST
			UBYTE unused:8;
			UBYTE unused2:8;
			UBYTE unused3:4;
			UBYTE Blue:4;
			UBYTE Red:4;
			UBYTE Green:4;
#else
			UBYTE Green:4;
			UBYTE Red:4;
			UBYTE Blue:4;
#endif
        }Colours;
        ULONG     Index;
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

class CMikie : public CLynxBase
{
public:
    CMikie(CSystem& parent);
    ~CMikie();
	
    bool	ContextSave(FILE *fp);
    bool	ContextLoad(LSS_FILE *fp);
    void	Reset(void);
    
    UBYTE	Peek(ULONG addr);
    void	Poke(ULONG addr,UBYTE data);
    ULONG	ReadCycle(void) {return 5;};
    ULONG	WriteCycle(void) {return 5;};
    ULONG	ObjectSize(void) {return MIKIE_SIZE;};
    void	PresetForHomebrew(void);
    ULONG	GetLfsrNext(ULONG current);
    
    void	ComLynxCable(int status);
    void	ComLynxRxData(int data);
    void	ComLynxTxLoopback(int data);
    void	ComLynxTxCallback(void (*function)(int data,ULONG objref),ULONG objref);
    
    void	DisplaySetAttributes(ULONG Rotate, ULONG Format, ULONG Pitch, UBYTE* (*DisplayCallback)(ULONG objref),ULONG objref);
    
    void	BlowOut(void);
    
    ULONG	DisplayRenderLine(void);
    ULONG	DisplayEndOfFrame(void);
    
    inline void SetCPUSleep(void) {gSystemCPUSleep=TRUE;};
    inline void ClearCPUSleep(void) {gSystemCPUSleep=FALSE;gSystemCPUSleep_Saved=FALSE;};
    
    inline void	Update(void)
    {
        SLONG divide;
        SLONG decval;
        ULONG tmp;
        ULONG mikie_work_done=0;
        
        //
        // To stop problems with cycle count wrap we will check and then correct the
        // cycle counter.
        //
        
        //			TRACE_MIKIE0("Update()");
        
        if(gSystemCycleCount>0xf0000000)
        {
            gSystemCycleCount-=0x80000000;
            gThrottleNextCycleCheckpoint-=0x80000000;
            gAudioLastUpdateCycle-=0x80000000;
            mTIM_0_LAST_COUNT-=0x80000000;
            mTIM_1_LAST_COUNT-=0x80000000;
            mTIM_2_LAST_COUNT-=0x80000000;
            mTIM_3_LAST_COUNT-=0x80000000;
            mTIM_4_LAST_COUNT-=0x80000000;
            mTIM_5_LAST_COUNT-=0x80000000;
            mTIM_6_LAST_COUNT-=0x80000000;
            mTIM_7_LAST_COUNT-=0x80000000;
            mAUDIO_0_LAST_COUNT-=0x80000000;
            mAUDIO_1_LAST_COUNT-=0x80000000;
            mAUDIO_2_LAST_COUNT-=0x80000000;
            mAUDIO_3_LAST_COUNT-=0x80000000;
            // Only correct if sleep is active
            if(gCPUWakeupTime)
            {
                gCPUWakeupTime-=0x80000000;
                gIRQEntryCycle-=0x80000000;
            }
        }
        
        gNextTimerEvent=0xffffffff;
        
        //
        // Check if the CPU needs to be woken up from sleep mode
        //
        if(gCPUWakeupTime)
        {
            if(gSystemCycleCount>=gCPUWakeupTime)
            {
                TRACE_MIKIE0("*********************************************************");
                TRACE_MIKIE0("****              CPU SLEEP COMPLETED                ****");
                TRACE_MIKIE0("*********************************************************");
                ClearCPUSleep();
                gCPUWakeupTime=0;			
            }
            else
            {
                if(gCPUWakeupTime>gSystemCycleCount) gNextTimerEvent=gCPUWakeupTime;
            }
        }
        
        //	Timer updates, rolled out flat in group order
        //
        //	Group A:
        //	Timer 0 -> Timer 2 -> Timer 4. 
        //
        //	Group B:
        //	Timer 1 -> Timer 3 -> Timer 5 -> Timer 7 -> Audio 0 -> Audio 1-> Audio 2 -> Audio 3 -> Timer 1. 
        //
        
        //
        // Within each timer code block we will predict the cycle count number of
        // the next timer event
        //
        // We don't need to count linked timers as the timer they are linked
        // from will always generate earlier events.
        //
        // As Timer 4 (UART) will generate many events we will ignore it
        //
        // We set the next event to the end of time at first and let the timers
        // overload it. Any writes to timer controls will force next event to
        // be immediate and hence a new preidction will be done. The prediction
        // causes overflow as opposed to zero i.e. current+1
        // (In reality T0 line counter should always be running.)
        //
        
        
        //
        // Timer 0 of Group A
        //
        
        //
        // Optimisation, assume T0 (Line timer) is never in one-shot,
        // never placed in link mode
        //
        
        // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
        //			if(mTIM_0_ENABLE_COUNT && (mTIM_0_ENABLE_RELOAD || !mTIM_0_TIMER_DONE))
        if(mTIM_0_ENABLE_COUNT)
        {
            // Timer 0 has no linking
            //				if(mTIM_0_LINKING!=0x07)
            {
                // Ordinary clocked mode as opposed to linked mode
                // 16MHz clock downto 1us == cyclecount >> 4 
                divide=(4+mTIM_0_LINKING);
                decval=(gSystemCycleCount-mTIM_0_LAST_COUNT)>>divide;
                
                if(decval)
                {
                    mTIM_0_LAST_COUNT+=decval<<divide;
                    mTIM_0_CURRENT-=decval;
                    
                    if(mTIM_0_CURRENT&0x80000000)
                    {
                        // Set carry out
                        mTIM_0_BORROW_OUT=TRUE;
                        
                        //							// Reload if neccessary
                        //							if(mTIM_0_ENABLE_RELOAD)
                        //							{
                        mTIM_0_CURRENT+=mTIM_0_BKUP+1;
                        //							}
                        //							else
                        //							{
                        //								mTIM_0_CURRENT=0;
                        //							}
                        
                        mTIM_0_TIMER_DONE=TRUE;
                        
                        // Interupt flag setting code moved into DisplayRenderLine()
                        
                        // Line timer has expired, render a line, we cannot incrememnt
                        // the global counter at this point as it will screw the other timers
                        // so we save under work done and inc at the end.
                        mikie_work_done+=DisplayRenderLine();
                        
                    }
                    else
                    {
                        mTIM_0_BORROW_OUT=FALSE;
                    }
                    // Set carry in as we did a count
                    mTIM_0_BORROW_IN=TRUE;
                }
                else
                {
                    // Clear carry in as we didn't count
                    mTIM_0_BORROW_IN=FALSE;
                    // Clear carry out
                    mTIM_0_BORROW_OUT=FALSE;
                }
            }
            
            // Prediction for next timer event cycle number
            
            //				if(mTIM_0_LINKING!=7)
            {
                // Sometimes timeupdates can be >2x rollover in which case
                // then CURRENT may still be negative and we can use it to
                // calc the next timer value, we just want another update ASAP
                tmp=(mTIM_0_CURRENT&0x80000000)?1:((mTIM_0_CURRENT+1)<<divide);
                tmp+=gSystemCycleCount;
                if(tmp<gNextTimerEvent)
                {
                    gNextTimerEvent=tmp;
                    //						TRACE_MIKIE1("Update() - TIMER 0 Set NextTimerEvent = %012d",gNextTimerEvent);
                }
            }
            //				TRACE_MIKIE1("Update() - mTIM_0_CURRENT = %012d",mTIM_0_CURRENT);
            //				TRACE_MIKIE1("Update() - mTIM_0_BKUP    = %012d",mTIM_0_BKUP);
            //				TRACE_MIKIE1("Update() - mTIM_0_LASTCNT = %012d",mTIM_0_LAST_COUNT);
            //				TRACE_MIKIE1("Update() - mTIM_0_LINKING = %012d",mTIM_0_LINKING);
        }
        
        //
        // Timer 2 of Group A
        //
        
        //
        // Optimisation, assume T2 (Frame timer) is never in one-shot
        // always in linked mode i.e clocked by Line Timer
        //
        
        // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
        //			if(mTIM_2_ENABLE_COUNT && (mTIM_2_ENABLE_RELOAD || !mTIM_2_TIMER_DONE))
        if(mTIM_2_ENABLE_COUNT)
        {
            decval=0;
            
            //				if(mTIM_2_LINKING==0x07)
            {
                if(mTIM_0_BORROW_OUT) decval=1;
                mTIM_2_LAST_LINK_CARRY=mTIM_0_BORROW_OUT;
            }
            //				else
            //				{
            //					// Ordinary clocked mode as opposed to linked mode
            //					// 16MHz clock downto 1us == cyclecount >> 4 
            //					divide=(4+mTIM_2_LINKING);
            //					decval=(gSystemCycleCount-mTIM_2_LAST_COUNT)>>divide;
            //				}
            
            if(decval)
            {
                //					mTIM_2_LAST_COUNT+=decval<<divide;
                mTIM_2_CURRENT-=decval;
                if(mTIM_2_CURRENT&0x80000000)
                {
                    // Set carry out
                    mTIM_2_BORROW_OUT=TRUE;
                    
                    //						// Reload if neccessary
                    //						if(mTIM_2_ENABLE_RELOAD)
                    //						{
                    mTIM_2_CURRENT+=mTIM_2_BKUP+1;
                    //						}
                    //						else
                    //						{
                    //							mTIM_2_CURRENT=0;
                    //						}
                    mTIM_2_TIMER_DONE=TRUE;
                    
                    // Interupt flag setting code moved into DisplayEndOfFrame(), also
                    // park any CPU cycles lost for later inclusion
                    mikie_work_done+=DisplayEndOfFrame();
                }
                else
                {
                    mTIM_2_BORROW_OUT=FALSE;
                }
                // Set carry in as we did a count
                mTIM_2_BORROW_IN=TRUE;
            }
            else
            {
                // Clear carry in as we didn't count
                mTIM_2_BORROW_IN=FALSE;
                // Clear carry out
                mTIM_2_BORROW_OUT=FALSE;
            }
            
            // Prediction for next timer event cycle number
            // We dont need to predict this as its the frame timer and will always
            // be beaten by the line timer on Timer 0
            //				if(mTIM_2_LINKING!=7)
            //				{
            //					tmp=gSystemCycleCount+((mTIM_2_CURRENT+1)<<divide);
            //					if(tmp<gNextTimerEvent)	gNextTimerEvent=tmp;
            //				}
            //				TRACE_MIKIE1("Update() - mTIM_2_CURRENT = %012d",mTIM_2_CURRENT);
            //				TRACE_MIKIE1("Update() - mTIM_2_BKUP    = %012d",mTIM_2_BKUP);
            //				TRACE_MIKIE1("Update() - mTIM_2_LASTCNT = %012d",mTIM_2_LAST_COUNT);
            //				TRACE_MIKIE1("Update() - mTIM_2_LINKING = %012d",mTIM_2_LINKING);
        }
		
        //
        // Timer 4 of Group A
        //
        // For the sake of speed it is assumed that Timer 4 (UART timer)
        // never uses one-shot mode, never uses linking, hence the code
        // is commented out. Timer 4 is at the end of a chain and seems
        // no reason to update its carry in-out variables
        //
        
        // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
        //			if(mTIM_4_ENABLE_COUNT && (mTIM_4_ENABLE_RELOAD || !mTIM_4_TIMER_DONE))
        if(mTIM_4_ENABLE_COUNT)
        {
            decval=0;
            
            //				if(mTIM_4_LINKING==0x07)
            //				{
            ////				if(mTIM_2_BORROW_OUT && !mTIM_4_LAST_LINK_CARRY) decval=1;
            //					if(mTIM_2_BORROW_OUT) decval=1;
            //					mTIM_4_LAST_LINK_CARRY=mTIM_2_BORROW_OUT;
            //				}
            //				else
            {
                // Ordinary clocked mode as opposed to linked mode
                // 16MHz clock downto 1us == cyclecount >> 4 
                // Additional /8 (+3) for 8 clocks per bit transmit
                divide=4+3+mTIM_4_LINKING;
                decval=(gSystemCycleCount-mTIM_4_LAST_COUNT)>>divide;
            }
            
            if(decval)
            {
                mTIM_4_LAST_COUNT+=decval<<divide;
                mTIM_4_CURRENT-=decval;
                if(mTIM_4_CURRENT&0x80000000)
                {
                    // Set carry out
                    mTIM_4_BORROW_OUT=TRUE;
                    
                    //
                    // Update the UART counter models for Rx & Tx
                    //
                    
                    //
                    // According to the docs IRQ's are level triggered and hence will always assert
                    // what a pain in the arse
                    //
                    // Rx & Tx are loopedback due to comlynx structure
                    
                    //
                    // Receive
                    //
                    if(!mUART_RX_COUNTDOWN)
                    {
                        // Fetch a byte from the input queue
                        if(mUART_Rx_waiting>0)
                        {
                            mUART_RX_DATA=mUART_Rx_input_queue[mUART_Rx_output_ptr];
                            mUART_Rx_output_ptr=(++mUART_Rx_output_ptr)%UART_MAX_RX_QUEUE;
                            mUART_Rx_waiting--;
                            TRACE_MIKIE2("Update() - RX Byte output ptr=%02d waiting=%02d",mUART_Rx_output_ptr,mUART_Rx_waiting);
                        }
                        else
                        {
                            TRACE_MIKIE0("Update() - RX Byte but no data waiting ????");
                        }
                        
                        // Retrigger input if more bytes waiting
                        if(mUART_Rx_waiting>0)
                        {
                            mUART_RX_COUNTDOWN=UART_RX_TIME_PERIOD+UART_RX_NEXT_DELAY;
                            TRACE_MIKIE1("Update() - RX Byte retriggered, %d waiting",mUART_Rx_waiting);
                        }
                        else
                        {
                            mUART_RX_COUNTDOWN=UART_RX_INACTIVE;
                            TRACE_MIKIE0("Update() - RX Byte nothing waiting, deactivated");
                        }
                        
                        // If RX_READY already set then we have an overrun
                        // as previous byte hasnt been read
                        if(mUART_RX_READY) mUART_Rx_overun_error=1;
                        
                        // Flag byte as being recvd
                        mUART_RX_READY=1;
                    }
                    else if(!(mUART_RX_COUNTDOWN&UART_RX_INACTIVE))
                    {
                        mUART_RX_COUNTDOWN--;
                    }
                    
                    if(!mUART_TX_COUNTDOWN)
                    {
                        if(mUART_SENDBREAK)
                        {
                            mUART_TX_DATA=UART_BREAK_CODE;
                            // Auto-Respawn new transmit
                            mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
                            // Loop back what we transmitted
                            ComLynxTxLoopback(mUART_TX_DATA);
                        }
                        else
                        {
                            // Serial activity finished 
                            mUART_TX_COUNTDOWN=UART_TX_INACTIVE;
                        }
                        
                        // If a networking object is attached then use its callback to send the data byte.
                        if(mpUART_TX_CALLBACK)
                        {
                            TRACE_MIKIE0("Update() - UART_TX_CALLBACK");
                            (*mpUART_TX_CALLBACK)(mUART_TX_DATA,mUART_TX_CALLBACK_OBJECT);
                        }
                        
                    }
                    else if(!(mUART_TX_COUNTDOWN&UART_TX_INACTIVE))
                    {
                        mUART_TX_COUNTDOWN--;
                    }
                    
                    // Set the timer status flag
                    // Timer 4 is the uart timer and doesn't generate IRQ's using this method
                    
                    // 16 Clocks = 1 bit transmission. Hold separate Rx & Tx counters
                    
                    // Reload if neccessary
                    //						if(mTIM_4_ENABLE_RELOAD)
                    //						{
                    mTIM_4_CURRENT+=mTIM_4_BKUP+1;
                    // The low reload values on TIM4 coupled with a longer
                    // timer service delay can sometimes cause
                    // an underun, check and fix
                    if(mTIM_4_CURRENT&0x80000000)
                    {
                        mTIM_4_CURRENT=mTIM_4_BKUP;
                        mTIM_4_LAST_COUNT=gSystemCycleCount;
                    }
                    //						}
                    //						else
                    //						{
                    //							mTIM_4_CURRENT=0;
                    //						}
                    //						mTIM_4_TIMER_DONE=TRUE;
                }
                //					else
                //					{
                //						mTIM_4_BORROW_OUT=FALSE;
                //					}
                //					// Set carry in as we did a count
                //					mTIM_4_BORROW_IN=TRUE;
            }
            //				else
            //				{
            //					// Clear carry in as we didn't count
            //					mTIM_4_BORROW_IN=FALSE;
            //					// Clear carry out
            //					mTIM_4_BORROW_OUT=FALSE;
            //				}
            //
            //				// Prediction for next timer event cycle number
            //
            //				if(mTIM_4_LINKING!=7)
            //				{
            // Sometimes timeupdates can be >2x rollover in which case
            // then CURRENT may still be negative and we can use it to
            // calc the next timer value, we just want another update ASAP
            tmp=(mTIM_4_CURRENT&0x80000000)?1:((mTIM_4_CURRENT+1)<<divide);
            tmp+=gSystemCycleCount;
            if(tmp<gNextTimerEvent)
            {
                gNextTimerEvent=tmp;
                TRACE_MIKIE1("Update() - TIMER 4 Set NextTimerEvent = %012d",gNextTimerEvent);
            }
            //				}
            //				TRACE_MIKIE1("Update() - mTIM_4_CURRENT = %012d",mTIM_4_CURRENT);
            //				TRACE_MIKIE1("Update() - mTIM_4_BKUP    = %012d",mTIM_4_BKUP);
            //				TRACE_MIKIE1("Update() - mTIM_4_LASTCNT = %012d",mTIM_4_LAST_COUNT);
            //				TRACE_MIKIE1("Update() - mTIM_4_LINKING = %012d",mTIM_4_LINKING);
        }
        
        // Emulate the UART bug where UART IRQ is level sensitive
        // in that it will continue to generate interrupts as long
        // as they are enabled and the interrupt condition is true
        
        // If Tx is inactive i.e ready for a byte to eat and the
        // IRQ is enabled then generate it always
        if((mUART_TX_COUNTDOWN&UART_TX_INACTIVE) && mUART_TX_IRQ_ENABLE)
        {
            TRACE_MIKIE0("Update() - UART TX IRQ Triggered");
            mTimerStatusFlags|=0x10;
            gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
        }
        // Is data waiting and the interrupt enabled, if so then
        // what are we waiting for....
        if(mUART_RX_READY && mUART_RX_IRQ_ENABLE)
        {
            TRACE_MIKIE0("Update() - UART RX IRQ Triggered");
            mTimerStatusFlags|=0x10;
            gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
        }
		
        //
        // Timer 1 of Group B
        //
        // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
        if(mTIM_1_ENABLE_COUNT && (mTIM_1_ENABLE_RELOAD || !mTIM_1_TIMER_DONE))
        {
            if(mTIM_1_LINKING!=0x07)
            {
                // Ordinary clocked mode as opposed to linked mode
                // 16MHz clock downto 1us == cyclecount >> 4 
                divide=(4+mTIM_1_LINKING);
                decval=(gSystemCycleCount-mTIM_1_LAST_COUNT)>>divide;
                
                if(decval)
                {
                    mTIM_1_LAST_COUNT+=decval<<divide;
                    mTIM_1_CURRENT-=decval;
                    if(mTIM_1_CURRENT&0x80000000)
                    {
                        // Set carry out
                        mTIM_1_BORROW_OUT=TRUE;
                        
                        // Set the timer status flag
                        if(mTimerInterruptMask&0x02)
                        {
                            TRACE_MIKIE0("Update() - TIMER1 IRQ Triggered");
                            mTimerStatusFlags|=0x02;
                            gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
                        }
                        
                        // Reload if neccessary
                        if(mTIM_1_ENABLE_RELOAD)
                        {
                            mTIM_1_CURRENT+=mTIM_1_BKUP+1;
                        }
                        else
                        {
                            mTIM_1_CURRENT=0;
                        }
                        mTIM_1_TIMER_DONE=TRUE;
                    }
                    else
                    {
                        mTIM_1_BORROW_OUT=FALSE;
                    }
                    // Set carry in as we did a count
                    mTIM_1_BORROW_IN=TRUE;
                }
                else
                {
                    // Clear carry in as we didn't count
                    mTIM_1_BORROW_IN=FALSE;
                    // Clear carry out
                    mTIM_1_BORROW_OUT=FALSE;
                }
            }
            
            // Prediction for next timer event cycle number
            
            if(mTIM_1_LINKING!=7)
            {
                // Sometimes timeupdates can be >2x rollover in which case
                // then CURRENT may still be negative and we can use it to
                // calc the next timer value, we just want another update ASAP
                tmp=(mTIM_1_CURRENT&0x80000000)?1:((mTIM_1_CURRENT+1)<<divide);
                tmp+=gSystemCycleCount;
                if(tmp<gNextTimerEvent)
                {
                    gNextTimerEvent=tmp;
                    TRACE_MIKIE1("Update() - TIMER 1 Set NextTimerEvent = %012d",gNextTimerEvent);
                }
            }
            //				TRACE_MIKIE1("Update() - mTIM_1_CURRENT = %012d",mTIM_1_CURRENT);
            //				TRACE_MIKIE1("Update() - mTIM_1_BKUP    = %012d",mTIM_1_BKUP);
            //				TRACE_MIKIE1("Update() - mTIM_1_LASTCNT = %012d",mTIM_1_LAST_COUNT);
            //				TRACE_MIKIE1("Update() - mTIM_1_LINKING = %012d",mTIM_1_LINKING);
        }
		
        //
        // Timer 3 of Group A
        //
        // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
        if(mTIM_3_ENABLE_COUNT && (mTIM_3_ENABLE_RELOAD || !mTIM_3_TIMER_DONE))
        {
            decval=0;
            
            if(mTIM_3_LINKING==0x07)
            {
                if(mTIM_1_BORROW_OUT) decval=1;
                mTIM_3_LAST_LINK_CARRY=mTIM_1_BORROW_OUT;
            }
            else
            {
                // Ordinary clocked mode as opposed to linked mode
                // 16MHz clock downto 1us == cyclecount >> 4 
                divide=(4+mTIM_3_LINKING);
                decval=(gSystemCycleCount-mTIM_3_LAST_COUNT)>>divide;
            }
            
            if(decval)
            {
                mTIM_3_LAST_COUNT+=decval<<divide;
                mTIM_3_CURRENT-=decval;
                if(mTIM_3_CURRENT&0x80000000)
                {
                    // Set carry out
                    mTIM_3_BORROW_OUT=TRUE;
                    
                    // Set the timer status flag
                    if(mTimerInterruptMask&0x08)
                    {
                        TRACE_MIKIE0("Update() - TIMER3 IRQ Triggered");
                        mTimerStatusFlags|=0x08;
                        gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
                    }
                    
                    // Reload if neccessary
                    if(mTIM_3_ENABLE_RELOAD)
                    {
                        mTIM_3_CURRENT+=mTIM_3_BKUP+1;
                    }
                    else
                    {
                        mTIM_3_CURRENT=0;
                    }
                    mTIM_3_TIMER_DONE=TRUE;
                }
                else
                {
                    mTIM_3_BORROW_OUT=FALSE;
                }
                // Set carry in as we did a count
                mTIM_3_BORROW_IN=TRUE;
            }
            else
            {
                // Clear carry in as we didn't count
                mTIM_3_BORROW_IN=FALSE;
                // Clear carry out
                mTIM_3_BORROW_OUT=FALSE;
            }
            
            // Prediction for next timer event cycle number
            
            if(mTIM_3_LINKING!=7)
            {
                // Sometimes timeupdates can be >2x rollover in which case
                // then CURRENT may still be negative and we can use it to
                // calc the next timer value, we just want another update ASAP
                tmp=(mTIM_3_CURRENT&0x80000000)?1:((mTIM_3_CURRENT+1)<<divide);
                tmp+=gSystemCycleCount;
                if(tmp<gNextTimerEvent)
                {
                    gNextTimerEvent=tmp;
                    TRACE_MIKIE1("Update() - TIMER 3 Set NextTimerEvent = %012d",gNextTimerEvent);
                }
            }
            //				TRACE_MIKIE1("Update() - mTIM_3_CURRENT = %012d",mTIM_3_CURRENT);
            //				TRACE_MIKIE1("Update() - mTIM_3_BKUP    = %012d",mTIM_3_BKUP);
            //				TRACE_MIKIE1("Update() - mTIM_3_LASTCNT = %012d",mTIM_3_LAST_COUNT);
            //				TRACE_MIKIE1("Update() - mTIM_3_LINKING = %012d",mTIM_3_LINKING);
        }
		
        //
        // Timer 5 of Group A
        //
        // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
        if(mTIM_5_ENABLE_COUNT && (mTIM_5_ENABLE_RELOAD || !mTIM_5_TIMER_DONE))
        {
            decval=0;
            
            if(mTIM_5_LINKING==0x07)
            {
                if(mTIM_3_BORROW_OUT) decval=1;
                mTIM_5_LAST_LINK_CARRY=mTIM_3_BORROW_OUT;
            }
            else
            {
                // Ordinary clocked mode as opposed to linked mode
                // 16MHz clock downto 1us == cyclecount >> 4 
                divide=(4+mTIM_5_LINKING);
                decval=(gSystemCycleCount-mTIM_5_LAST_COUNT)>>divide;
            }
            
            if(decval)
            {
                mTIM_5_LAST_COUNT+=decval<<divide;
                mTIM_5_CURRENT-=decval;
                if(mTIM_5_CURRENT&0x80000000)
                {
                    // Set carry out
                    mTIM_5_BORROW_OUT=TRUE;
                    
                    // Set the timer status flag
                    if(mTimerInterruptMask&0x20)
                    {
                        TRACE_MIKIE0("Update() - TIMER5 IRQ Triggered");
                        mTimerStatusFlags|=0x20;
                        gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
                    }
                    
                    // Reload if neccessary
                    if(mTIM_5_ENABLE_RELOAD)
                    {
                        mTIM_5_CURRENT+=mTIM_5_BKUP+1;
                    }
                    else
                    {
                        mTIM_5_CURRENT=0;
                    }
                    mTIM_5_TIMER_DONE=TRUE;
                }
                else
                {
                    mTIM_5_BORROW_OUT=FALSE;
                }
                // Set carry in as we did a count
                mTIM_5_BORROW_IN=TRUE;
            }
            else
            {
                // Clear carry in as we didn't count
                mTIM_5_BORROW_IN=FALSE;
                // Clear carry out
                mTIM_5_BORROW_OUT=FALSE;
            }
            
            // Prediction for next timer event cycle number
            
            if(mTIM_5_LINKING!=7)
            {
                // Sometimes timeupdates can be >2x rollover in which case
                // then CURRENT may still be negative and we can use it to
                // calc the next timer value, we just want another update ASAP
                tmp=(mTIM_5_CURRENT&0x80000000)?1:((mTIM_5_CURRENT+1)<<divide);
                tmp+=gSystemCycleCount;
                if(tmp<gNextTimerEvent)
                {
                    gNextTimerEvent=tmp;
                    TRACE_MIKIE1("Update() - TIMER 5 Set NextTimerEvent = %012d",gNextTimerEvent);
                }
            }
            //				TRACE_MIKIE1("Update() - mTIM_5_CURRENT = %012d",mTIM_5_CURRENT);
            //				TRACE_MIKIE1("Update() - mTIM_5_BKUP    = %012d",mTIM_5_BKUP);
            //				TRACE_MIKIE1("Update() - mTIM_5_LASTCNT = %012d",mTIM_5_LAST_COUNT);
            //				TRACE_MIKIE1("Update() - mTIM_5_LINKING = %012d",mTIM_5_LINKING);
        }
		
        //
        // Timer 7 of Group A
        //
        // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
        if(mTIM_7_ENABLE_COUNT && (mTIM_7_ENABLE_RELOAD || !mTIM_7_TIMER_DONE))
        {
            decval=0;
            
            if(mTIM_7_LINKING==0x07)
            {
                if(mTIM_5_BORROW_OUT) decval=1;
                mTIM_7_LAST_LINK_CARRY=mTIM_5_BORROW_OUT;
            }
            else
            {
                // Ordinary clocked mode as opposed to linked mode
                // 16MHz clock downto 1us == cyclecount >> 4 
                divide=(4+mTIM_7_LINKING);
                decval=(gSystemCycleCount-mTIM_7_LAST_COUNT)>>divide;
            }
            
            if(decval)
            {
                mTIM_7_LAST_COUNT+=decval<<divide;
                mTIM_7_CURRENT-=decval;
                if(mTIM_7_CURRENT&0x80000000)
                {
                    // Set carry out
                    mTIM_7_BORROW_OUT=TRUE;
                    
                    // Set the timer status flag
                    if(mTimerInterruptMask&0x80)
                    {
                        TRACE_MIKIE0("Update() - TIMER7 IRQ Triggered");
                        mTimerStatusFlags|=0x80;
                        gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
                    }
                    
                    // Reload if neccessary
                    if(mTIM_7_ENABLE_RELOAD)
                    {
                        mTIM_7_CURRENT+=mTIM_7_BKUP+1;
                    }
                    else
                    {
                        mTIM_7_CURRENT=0;
                    }
                    mTIM_7_TIMER_DONE=TRUE;
                    
                }
                else
                {
                    mTIM_7_BORROW_OUT=FALSE;
                }
                // Set carry in as we did a count
                mTIM_7_BORROW_IN=TRUE;
            }
            else
            {
                // Clear carry in as we didn't count
                mTIM_7_BORROW_IN=FALSE;
                // Clear carry out
                mTIM_7_BORROW_OUT=FALSE;
            }
            
            // Prediction for next timer event cycle number
            
            if(mTIM_7_LINKING!=7)
            {
                // Sometimes timeupdates can be >2x rollover in which case
                // then CURRENT may still be negative and we can use it to
                // calc the next timer value, we just want another update ASAP
                tmp=(mTIM_7_CURRENT&0x80000000)?1:((mTIM_7_CURRENT+1)<<divide);
                tmp+=gSystemCycleCount;
                if(tmp<gNextTimerEvent)
                {
                    gNextTimerEvent=tmp;
                    TRACE_MIKIE1("Update() - TIMER 7 Set NextTimerEvent = %012d",gNextTimerEvent);
                }
            }
            //				TRACE_MIKIE1("Update() - mTIM_7_CURRENT = %012d",mTIM_7_CURRENT);
            //				TRACE_MIKIE1("Update() - mTIM_7_BKUP    = %012d",mTIM_7_BKUP);
            //				TRACE_MIKIE1("Update() - mTIM_7_LASTCNT = %012d",mTIM_7_LAST_COUNT);
            //				TRACE_MIKIE1("Update() - mTIM_7_LINKING = %012d",mTIM_7_LINKING);
        }
		
        //
        // Timer 6 has no group
        //
        // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..) 
        if(mTIM_6_ENABLE_COUNT && (mTIM_6_ENABLE_RELOAD || !mTIM_6_TIMER_DONE))
        {
            //				if(mTIM_6_LINKING!=0x07)
            {
                // Ordinary clocked mode as opposed to linked mode
                // 16MHz clock downto 1us == cyclecount >> 4 
                divide=(4+mTIM_6_LINKING);
                decval=(gSystemCycleCount-mTIM_6_LAST_COUNT)>>divide;
                
                if(decval)
                {
                    mTIM_6_LAST_COUNT+=decval<<divide;
                    mTIM_6_CURRENT-=decval;
                    if(mTIM_6_CURRENT&0x80000000)
                    {
                        // Set carry out
                        mTIM_6_BORROW_OUT=TRUE;
                        
                        // Set the timer status flag
                        if(mTimerInterruptMask&0x40)
                        {
                            TRACE_MIKIE0("Update() - TIMER6 IRQ Triggered");
                            mTimerStatusFlags|=0x40;
                            gSystemIRQ=TRUE;	// Added 19/09/06 fix for IRQ issue
                        }
                        
                        // Reload if neccessary
                        if(mTIM_6_ENABLE_RELOAD)
                        {
                            mTIM_6_CURRENT+=mTIM_6_BKUP+1;
                        }
                        else
                        {
                            mTIM_6_CURRENT=0;
                        }
                        mTIM_6_TIMER_DONE=TRUE;
                    }
                    else
                    {
                        mTIM_6_BORROW_OUT=FALSE;
                    }
                    // Set carry in as we did a count
                    mTIM_6_BORROW_IN=TRUE;
                }
                else
                {
                    // Clear carry in as we didn't count
                    mTIM_6_BORROW_IN=FALSE;
                    // Clear carry out
                    mTIM_6_BORROW_OUT=FALSE;
                }
            }
            
            // Prediction for next timer event cycle number
            // (Timer 6 doesn't support linking)
            
            //				if(mTIM_6_LINKING!=7)
            {
                // Sometimes timeupdates can be >2x rollover in which case
                // then CURRENT may still be negative and we can use it to
                // calc the next timer value, we just want another update ASAP
                tmp=(mTIM_6_CURRENT&0x80000000)?1:((mTIM_6_CURRENT+1)<<divide);
                tmp+=gSystemCycleCount;
                if(tmp<gNextTimerEvent)
                {
                    gNextTimerEvent=tmp;
                    TRACE_MIKIE1("Update() - TIMER 6 Set NextTimerEvent = %012d",gNextTimerEvent);
                }
            }
            //				TRACE_MIKIE1("Update() - mTIM_6_CURRENT = %012d",mTIM_6_CURRENT);
            //				TRACE_MIKIE1("Update() - mTIM_6_BKUP    = %012d",mTIM_6_BKUP);
            //				TRACE_MIKIE1("Update() - mTIM_6_LASTCNT = %012d",mTIM_6_LAST_COUNT);
            //				TRACE_MIKIE1("Update() - mTIM_6_LINKING = %012d",mTIM_6_LINKING);
        }
        
        //
        // If sound is enabled then update the sound subsystem
        //
        if(gAudioEnabled)
        {
            static SLONG sample=0;
            ULONG mix=0;
            
            //
            // Catch audio buffer up to current time
            //
            
            // Mix the sample
            sample=0;
            if(mSTEREO&0x11) { sample+=mAUDIO_0_OUTPUT; mix++; }
            if(mSTEREO&0x22) { sample+=mAUDIO_1_OUTPUT; mix++; }
            if(mSTEREO&0x44) { sample+=mAUDIO_2_OUTPUT; mix++; }
            if(mSTEREO&0x88) { sample+=mAUDIO_3_OUTPUT; mix++; }
            if(mix)
            {
                sample+=128*mix; // Correct for sign
                sample/=mix;	// Keep the audio volume at max
            }
            else
            {
                sample=128;
            }
            
            //				sample+=(mSTEREO&0x11)?mAUDIO_0_OUTPUT:0;
            //				sample+=(mSTEREO&0x22)?mAUDIO_1_OUTPUT:0;
            //				sample+=(mSTEREO&0x44)?mAUDIO_2_OUTPUT:0;
            //				sample+=(mSTEREO&0x88)?mAUDIO_3_OUTPUT:0;
            //				sample=sample>>2;
            //				sample+=128;
            
            for(;gAudioLastUpdateCycle+HANDY_AUDIO_SAMPLE_PERIOD<gSystemCycleCount;gAudioLastUpdateCycle+=HANDY_AUDIO_SAMPLE_PERIOD)
            {
                // Output audio sample
                gAudioBuffer[gAudioBufferPointer++]=(UBYTE)sample;
                
                // Check buffer overflow condition, stick at the endpoint
                // teh audio output system will reset the input pointer
                // when it reads out the data.
                
                // We should NEVER overflow, this buffer holds 0.25 seconds
                // of data if this happens the the multimedia system above
                // has failed so the corruption of the buffer contents wont matter
                
                gAudioBufferPointer%=HANDY_AUDIO_BUFFER_SIZE;
            }
            
            //
            // Audio 0 
            //
            //				if(mAUDIO_0_ENABLE_COUNT && !mAUDIO_0_TIMER_DONE && mAUDIO_0_VOLUME && mAUDIO_0_BKUP)
            if(mAUDIO_0_ENABLE_COUNT && (mAUDIO_0_ENABLE_RELOAD || !mAUDIO_0_TIMER_DONE) && mAUDIO_0_VOLUME && mAUDIO_0_BKUP)
            {
                decval=0;
                
                if(mAUDIO_0_LINKING==0x07)
                {
                    if(mTIM_7_BORROW_OUT) decval=1;
                    mAUDIO_0_LAST_LINK_CARRY=mTIM_7_BORROW_OUT;
                }
                else
                {
                    // Ordinary clocked mode as opposed to linked mode
                    // 16MHz clock downto 1us == cyclecount >> 4 
                    divide=(4+mAUDIO_0_LINKING);
                    decval=(gSystemCycleCount-mAUDIO_0_LAST_COUNT)>>divide;
                }
                
                if(decval)
                {
                    mAUDIO_0_LAST_COUNT+=decval<<divide;
                    mAUDIO_0_CURRENT-=decval;
                    if(mAUDIO_0_CURRENT&0x80000000)
                    {
                        // Set carry out
                        mAUDIO_0_BORROW_OUT=TRUE;
                        
                        // Reload if neccessary
                        if(mAUDIO_0_ENABLE_RELOAD)
                        {
                            mAUDIO_0_CURRENT+=mAUDIO_0_BKUP+1;
                            if(mAUDIO_0_CURRENT&0x80000000) mAUDIO_0_CURRENT=0;
                        }
                        else
                        {
                            // Set timer done
                            mAUDIO_0_TIMER_DONE=TRUE;
                            mAUDIO_0_CURRENT=0;
                        }
                        
                        //
                        // Update audio circuitry
                        //
                        mAUDIO_0_WAVESHAPER=GetLfsrNext(mAUDIO_0_WAVESHAPER);
                        
                        if(mAUDIO_0_INTEGRATE_ENABLE)
                        {
                            SLONG temp=mAUDIO_0_OUTPUT;
                            if(mAUDIO_0_WAVESHAPER&0x0001) temp+=mAUDIO_0_VOLUME; else temp-=mAUDIO_0_VOLUME;
                            if(temp>127) temp=127;
                            if(temp<-128) temp=-128;
                            mAUDIO_0_OUTPUT=(SBYTE)temp;
                        }
                        else
                        {
                            if(mAUDIO_0_WAVESHAPER&0x0001) mAUDIO_0_OUTPUT=mAUDIO_0_VOLUME; else mAUDIO_0_OUTPUT=-mAUDIO_0_VOLUME;
                        }
                    }
                    else
                    {
                        mAUDIO_0_BORROW_OUT=FALSE;
                    }
                    // Set carry in as we did a count
                    mAUDIO_0_BORROW_IN=TRUE;
                }
                else
                {
                    // Clear carry in as we didn't count
                    mAUDIO_0_BORROW_IN=FALSE;
                    // Clear carry out
                    mAUDIO_0_BORROW_OUT=FALSE;
                }
                
                // Prediction for next timer event cycle number
                
                if(mAUDIO_0_LINKING!=7)
                {
                    // Sometimes timeupdates can be >2x rollover in which case
                    // then CURRENT may still be negative and we can use it to
                    // calc the next timer value, we just want another update ASAP
                    tmp=(mAUDIO_0_CURRENT&0x80000000)?1:((mAUDIO_0_CURRENT+1)<<divide);
                    tmp+=gSystemCycleCount;
                    if(tmp<gNextTimerEvent)
                    {
                        gNextTimerEvent=tmp;
                        TRACE_MIKIE1("Update() - AUDIO 0 Set NextTimerEvent = %012d",gNextTimerEvent);
                    }
                }
                //					TRACE_MIKIE1("Update() - mAUDIO_0_CURRENT = %012d",mAUDIO_0_CURRENT);
                //					TRACE_MIKIE1("Update() - mAUDIO_0_BKUP    = %012d",mAUDIO_0_BKUP);
                //					TRACE_MIKIE1("Update() - mAUDIO_0_LASTCNT = %012d",mAUDIO_0_LAST_COUNT);
                //					TRACE_MIKIE1("Update() - mAUDIO_0_LINKING = %012d",mAUDIO_0_LINKING);
            }
            
            //
            // Audio 1 
            //
            //				if(mAUDIO_1_ENABLE_COUNT && !mAUDIO_1_TIMER_DONE && mAUDIO_1_VOLUME && mAUDIO_1_BKUP)
            if(mAUDIO_1_ENABLE_COUNT && (mAUDIO_1_ENABLE_RELOAD || !mAUDIO_1_TIMER_DONE) && mAUDIO_1_VOLUME && mAUDIO_1_BKUP)
            {
                decval=0;
                
                if(mAUDIO_1_LINKING==0x07)
                {
                    if(mAUDIO_0_BORROW_OUT) decval=1;
                    mAUDIO_1_LAST_LINK_CARRY=mAUDIO_0_BORROW_OUT;
                }
                else
                {
                    // Ordinary clocked mode as opposed to linked mode
                    // 16MHz clock downto 1us == cyclecount >> 4 
                    divide=(4+mAUDIO_1_LINKING);
                    decval=(gSystemCycleCount-mAUDIO_1_LAST_COUNT)>>divide;
                }
                
                if(decval)
                {
                    mAUDIO_1_LAST_COUNT+=decval<<divide;
                    mAUDIO_1_CURRENT-=decval;
                    if(mAUDIO_1_CURRENT&0x80000000)
                    {
                        // Set carry out
                        mAUDIO_1_BORROW_OUT=TRUE;
                        
                        // Reload if neccessary
                        if(mAUDIO_1_ENABLE_RELOAD)
                        {
                            mAUDIO_1_CURRENT+=mAUDIO_1_BKUP+1;
                            if(mAUDIO_1_CURRENT&0x80000000) mAUDIO_1_CURRENT=0;
                        }
                        else
                        {
                            // Set timer done
                            mAUDIO_1_TIMER_DONE=TRUE;
                            mAUDIO_1_CURRENT=0;
                        }
                        
                        //
                        // Update audio circuitry
                        //
                        mAUDIO_1_WAVESHAPER=GetLfsrNext(mAUDIO_1_WAVESHAPER);
                        
                        if(mAUDIO_1_INTEGRATE_ENABLE)
                        {
                            SLONG temp=mAUDIO_1_OUTPUT;
                            if(mAUDIO_1_WAVESHAPER&0x0001) temp+=mAUDIO_1_VOLUME; else temp-=mAUDIO_1_VOLUME;
                            if(temp>127) temp=127;
                            if(temp<-128) temp=-128;
                            mAUDIO_1_OUTPUT=(SBYTE)temp;
                        }
                        else
                        {
                            if(mAUDIO_1_WAVESHAPER&0x0001) mAUDIO_1_OUTPUT=mAUDIO_1_VOLUME; else mAUDIO_1_OUTPUT=-mAUDIO_1_VOLUME;
                        }
                    }
                    else
                    {
                        mAUDIO_1_BORROW_OUT=FALSE;
                    }
                    // Set carry in as we did a count
                    mAUDIO_1_BORROW_IN=TRUE;
                }
                else
                {
                    // Clear carry in as we didn't count
                    mAUDIO_1_BORROW_IN=FALSE;
                    // Clear carry out
                    mAUDIO_1_BORROW_OUT=FALSE;
                }
                
                // Prediction for next timer event cycle number
                
                if(mAUDIO_1_LINKING!=7)
                {
                    // Sometimes timeupdates can be >2x rollover in which case
                    // then CURRENT may still be negative and we can use it to
                    // calc the next timer value, we just want another update ASAP
                    tmp=(mAUDIO_1_CURRENT&0x80000000)?1:((mAUDIO_1_CURRENT+1)<<divide);
                    tmp+=gSystemCycleCount;
                    if(tmp<gNextTimerEvent)
                    {
                        gNextTimerEvent=tmp;
                        TRACE_MIKIE1("Update() - AUDIO 1 Set NextTimerEvent = %012d",gNextTimerEvent);
                    }
                }
                //					TRACE_MIKIE1("Update() - mAUDIO_1_CURRENT = %012d",mAUDIO_1_CURRENT);
                //					TRACE_MIKIE1("Update() - mAUDIO_1_BKUP    = %012d",mAUDIO_1_BKUP);
                //					TRACE_MIKIE1("Update() - mAUDIO_1_LASTCNT = %012d",mAUDIO_1_LAST_COUNT);
                //					TRACE_MIKIE1("Update() - mAUDIO_1_LINKING = %012d",mAUDIO_1_LINKING);
            }
            
            //
            // Audio 2 
            //
            //				if(mAUDIO_2_ENABLE_COUNT && !mAUDIO_2_TIMER_DONE && mAUDIO_2_VOLUME && mAUDIO_2_BKUP)
            if(mAUDIO_2_ENABLE_COUNT && (mAUDIO_2_ENABLE_RELOAD || !mAUDIO_2_TIMER_DONE) && mAUDIO_2_VOLUME && mAUDIO_2_BKUP)
            {
                decval=0;
                
                if(mAUDIO_2_LINKING==0x07)
                {
                    if(mAUDIO_1_BORROW_OUT) decval=1;
                    mAUDIO_2_LAST_LINK_CARRY=mAUDIO_1_BORROW_OUT;
                }
                else
                {
                    // Ordinary clocked mode as opposed to linked mode
                    // 16MHz clock downto 1us == cyclecount >> 4 
                    divide=(4+mAUDIO_2_LINKING);
                    decval=(gSystemCycleCount-mAUDIO_2_LAST_COUNT)>>divide;
                }
                
                if(decval)
                {
                    mAUDIO_2_LAST_COUNT+=decval<<divide;
                    mAUDIO_2_CURRENT-=decval;
                    if(mAUDIO_2_CURRENT&0x80000000)
                    {
                        // Set carry out
                        mAUDIO_2_BORROW_OUT=TRUE;
                        
                        // Reload if neccessary
                        if(mAUDIO_2_ENABLE_RELOAD)
                        {
                            mAUDIO_2_CURRENT+=mAUDIO_2_BKUP+1;
                            if(mAUDIO_2_CURRENT&0x80000000) mAUDIO_2_CURRENT=0;
                        }
                        else
                        {
                            // Set timer done
                            mAUDIO_2_TIMER_DONE=TRUE;
                            mAUDIO_2_CURRENT=0;
                        }
                        
                        //
                        // Update audio circuitry
                        //
                        mAUDIO_2_WAVESHAPER=GetLfsrNext(mAUDIO_2_WAVESHAPER);
                        
                        if(mAUDIO_2_INTEGRATE_ENABLE)
                        {
                            SLONG temp=mAUDIO_2_OUTPUT;
                            if(mAUDIO_2_WAVESHAPER&0x0001) temp+=mAUDIO_2_VOLUME; else temp-=mAUDIO_2_VOLUME;
                            if(temp>127) temp=127;
                            if(temp<-128) temp=-128;
                            mAUDIO_2_OUTPUT=(SBYTE)temp;
                        }
                        else
                        {
                            if(mAUDIO_2_WAVESHAPER&0x0001) mAUDIO_2_OUTPUT=mAUDIO_2_VOLUME; else mAUDIO_2_OUTPUT=-mAUDIO_2_VOLUME;
                        }
                    }
                    else
                    {
                        mAUDIO_2_BORROW_OUT=FALSE;
                    }
                    // Set carry in as we did a count
                    mAUDIO_2_BORROW_IN=TRUE;
                }
                else
                {
                    // Clear carry in as we didn't count
                    mAUDIO_2_BORROW_IN=FALSE;
                    // Clear carry out
                    mAUDIO_2_BORROW_OUT=FALSE;
                }
                
                // Prediction for next timer event cycle number
                
                if(mAUDIO_2_LINKING!=7)
                {
                    // Sometimes timeupdates can be >2x rollover in which case
                    // then CURRENT may still be negative and we can use it to
                    // calc the next timer value, we just want another update ASAP
                    tmp=(mAUDIO_2_CURRENT&0x80000000)?1:((mAUDIO_2_CURRENT+1)<<divide);
                    tmp+=gSystemCycleCount;
                    if(tmp<gNextTimerEvent)
                    {
                        gNextTimerEvent=tmp;
                        TRACE_MIKIE1("Update() - AUDIO 2 Set NextTimerEvent = %012d",gNextTimerEvent);
                    }
                }
                //					TRACE_MIKIE1("Update() - mAUDIO_2_CURRENT = %012d",mAUDIO_2_CURRENT);
                //					TRACE_MIKIE1("Update() - mAUDIO_2_BKUP    = %012d",mAUDIO_2_BKUP);
                //					TRACE_MIKIE1("Update() - mAUDIO_2_LASTCNT = %012d",mAUDIO_2_LAST_COUNT);
                //					TRACE_MIKIE1("Update() - mAUDIO_2_LINKING = %012d",mAUDIO_2_LINKING);
            }
            
            //
            // Audio 3
            //
            //				if(mAUDIO_3_ENABLE_COUNT && !mAUDIO_3_TIMER_DONE && mAUDIO_3_VOLUME && mAUDIO_3_BKUP)
            if(mAUDIO_3_ENABLE_COUNT && (mAUDIO_3_ENABLE_RELOAD || !mAUDIO_3_TIMER_DONE) && mAUDIO_3_VOLUME && mAUDIO_3_BKUP)
            {
                decval=0;
                
                if(mAUDIO_3_LINKING==0x07)
                {
                    if(mAUDIO_2_BORROW_OUT) decval=1;
                    mAUDIO_3_LAST_LINK_CARRY=mAUDIO_2_BORROW_OUT;
                }
                else
                {
                    // Ordinary clocked mode as opposed to linked mode
                    // 16MHz clock downto 1us == cyclecount >> 4 
                    divide=(4+mAUDIO_3_LINKING);
                    decval=(gSystemCycleCount-mAUDIO_3_LAST_COUNT)>>divide;
                }
                
                if(decval)
                {
                    mAUDIO_3_LAST_COUNT+=decval<<divide;
                    mAUDIO_3_CURRENT-=decval;
                    if(mAUDIO_3_CURRENT&0x80000000)
                    {
                        // Set carry out
                        mAUDIO_3_BORROW_OUT=TRUE;
                        
                        // Reload if neccessary
                        if(mAUDIO_3_ENABLE_RELOAD)
                        {
                            mAUDIO_3_CURRENT+=mAUDIO_3_BKUP+1;
                            if(mAUDIO_3_CURRENT&0x80000000) mAUDIO_3_CURRENT=0;
                        }
                        else
                        {
                            // Set timer done
                            mAUDIO_3_TIMER_DONE=TRUE;
                            mAUDIO_3_CURRENT=0;
                        }
                        
                        //
                        // Update audio circuitry
                        //
                        mAUDIO_3_WAVESHAPER=GetLfsrNext(mAUDIO_3_WAVESHAPER);
                        
                        if(mAUDIO_3_INTEGRATE_ENABLE)
                        {
                            SLONG temp=mAUDIO_3_OUTPUT;
                            if(mAUDIO_3_WAVESHAPER&0x0001) temp+=mAUDIO_3_VOLUME; else temp-=mAUDIO_3_VOLUME;
                            if(temp>127) temp=127;
                            if(temp<-128) temp=-128;
                            mAUDIO_3_OUTPUT=(SBYTE)temp;
                        }
                        else
                        {
                            if(mAUDIO_3_WAVESHAPER&0x0001) mAUDIO_3_OUTPUT=mAUDIO_3_VOLUME; else mAUDIO_3_OUTPUT=-mAUDIO_3_VOLUME;
                        }
                    }
                    else
                    {
                        mAUDIO_3_BORROW_OUT=FALSE;
                    }
                    // Set carry in as we did a count
                    mAUDIO_3_BORROW_IN=TRUE;
                }
                else
                {
                    // Clear carry in as we didn't count
                    mAUDIO_3_BORROW_IN=FALSE;
                    // Clear carry out
                    mAUDIO_3_BORROW_OUT=FALSE;
                }
                
                // Prediction for next timer event cycle number
                
                if(mAUDIO_3_LINKING!=7)
                {
                    // Sometimes timeupdates can be >2x rollover in which case
                    // then CURRENT may still be negative and we can use it to
                    // calc the next timer value, we just want another update ASAP
                    tmp=(mAUDIO_3_CURRENT&0x80000000)?1:((mAUDIO_3_CURRENT+1)<<divide);
                    tmp+=gSystemCycleCount;
                    if(tmp<gNextTimerEvent)
                    {
                        gNextTimerEvent=tmp;
                        TRACE_MIKIE1("Update() - AUDIO 3 Set NextTimerEvent = %012d",gNextTimerEvent);
                    }
                }
                //					TRACE_MIKIE1("Update() - mAUDIO_3_CURRENT = %012d",mAUDIO_3_CURRENT);
                //					TRACE_MIKIE1("Update() - mAUDIO_3_BKUP    = %012d",mAUDIO_3_BKUP);
                //					TRACE_MIKIE1("Update() - mAUDIO_3_LASTCNT = %012d",mAUDIO_3_LAST_COUNT);
                //					TRACE_MIKIE1("Update() - mAUDIO_3_LINKING = %012d",mAUDIO_3_LINKING);
            }
        }
        
        //			if(gSystemCycleCount==gNextTimerEvent) gError->Warning("CMikie::Update() - gSystemCycleCount==gNextTimerEvent, system lock likely");
        //			TRACE_MIKIE1("Update() - NextTimerEvent = %012d",gNextTimerEvent);
        
        // Now all the timer updates are done we can increment the system
        // counter for any work done within the Update() function, gSystemCycleCounter
        // cannot be updated until this point otherwise it screws up the counters.
        gSystemCycleCount+=mikie_work_done;
    }
    
private:
    CSystem		&mSystem;
    
    // Hardware storage
    
    ULONG		mDisplayAddress;
    ULONG		mAudioInputComparator;
    ULONG		mTimerStatusFlags;
    ULONG		mTimerInterruptMask;
    
    TPALETTE	mPalette[16];
    ULONG		mColourMap[4096];
    
    ULONG		mIODAT;
    ULONG		mIODIR;
    ULONG		mIODAT_REST_SIGNAL;
    
    ULONG		mDISPCTL_DMAEnable;
    ULONG		mDISPCTL_Flip;
    ULONG		mDISPCTL_FourColour;
    ULONG		mDISPCTL_Colour;
    
    ULONG		mTIM_0_BKUP;
    ULONG		mTIM_0_ENABLE_RELOAD;
    ULONG		mTIM_0_ENABLE_COUNT;
    ULONG		mTIM_0_LINKING;
    ULONG		mTIM_0_CURRENT;
    ULONG		mTIM_0_TIMER_DONE;
    ULONG		mTIM_0_LAST_CLOCK;
    ULONG		mTIM_0_BORROW_IN;
    ULONG		mTIM_0_BORROW_OUT;
    ULONG		mTIM_0_LAST_LINK_CARRY;
    ULONG		mTIM_0_LAST_COUNT;
    
    ULONG		mTIM_1_BKUP;
    ULONG		mTIM_1_ENABLE_RELOAD;
    ULONG		mTIM_1_ENABLE_COUNT;
    ULONG		mTIM_1_LINKING;
    ULONG		mTIM_1_CURRENT;
    ULONG		mTIM_1_TIMER_DONE;
    ULONG		mTIM_1_LAST_CLOCK;
    ULONG		mTIM_1_BORROW_IN;
    ULONG		mTIM_1_BORROW_OUT;
    ULONG		mTIM_1_LAST_LINK_CARRY;
    ULONG		mTIM_1_LAST_COUNT;
    
    ULONG		mTIM_2_BKUP;
    ULONG		mTIM_2_ENABLE_RELOAD;
    ULONG		mTIM_2_ENABLE_COUNT;
    ULONG		mTIM_2_LINKING;
    ULONG		mTIM_2_CURRENT;
    ULONG		mTIM_2_TIMER_DONE;
    ULONG		mTIM_2_LAST_CLOCK;
    ULONG		mTIM_2_BORROW_IN;
    ULONG		mTIM_2_BORROW_OUT;
    ULONG		mTIM_2_LAST_LINK_CARRY;
    ULONG		mTIM_2_LAST_COUNT;
    
    ULONG		mTIM_3_BKUP;
    ULONG		mTIM_3_ENABLE_RELOAD;
    ULONG		mTIM_3_ENABLE_COUNT;
    ULONG		mTIM_3_LINKING;
    ULONG		mTIM_3_CURRENT;
    ULONG		mTIM_3_TIMER_DONE;
    ULONG		mTIM_3_LAST_CLOCK;
    ULONG		mTIM_3_BORROW_IN;
    ULONG		mTIM_3_BORROW_OUT;
    ULONG		mTIM_3_LAST_LINK_CARRY;
    ULONG		mTIM_3_LAST_COUNT;
    
    ULONG		mTIM_4_BKUP;
    ULONG		mTIM_4_ENABLE_RELOAD;
    ULONG		mTIM_4_ENABLE_COUNT;
    ULONG		mTIM_4_LINKING;
    ULONG		mTIM_4_CURRENT;
    ULONG		mTIM_4_TIMER_DONE;
    ULONG		mTIM_4_LAST_CLOCK;
    ULONG		mTIM_4_BORROW_IN;
    ULONG		mTIM_4_BORROW_OUT;
    ULONG		mTIM_4_LAST_LINK_CARRY;
    ULONG		mTIM_4_LAST_COUNT;
    
    ULONG		mTIM_5_BKUP;
    ULONG		mTIM_5_ENABLE_RELOAD;
    ULONG		mTIM_5_ENABLE_COUNT;
    ULONG		mTIM_5_LINKING;
    ULONG		mTIM_5_CURRENT;
    ULONG		mTIM_5_TIMER_DONE;
    ULONG		mTIM_5_LAST_CLOCK;
    ULONG		mTIM_5_BORROW_IN;
    ULONG		mTIM_5_BORROW_OUT;
    ULONG		mTIM_5_LAST_LINK_CARRY;
    ULONG		mTIM_5_LAST_COUNT;
    
    ULONG		mTIM_6_BKUP;
    ULONG		mTIM_6_ENABLE_RELOAD;
    ULONG		mTIM_6_ENABLE_COUNT;
    ULONG		mTIM_6_LINKING;
    ULONG		mTIM_6_CURRENT;
    ULONG		mTIM_6_TIMER_DONE;
    ULONG		mTIM_6_LAST_CLOCK;
    ULONG		mTIM_6_BORROW_IN;
    ULONG		mTIM_6_BORROW_OUT;
    ULONG		mTIM_6_LAST_LINK_CARRY;
    ULONG		mTIM_6_LAST_COUNT;
    
    ULONG		mTIM_7_BKUP;
    ULONG		mTIM_7_ENABLE_RELOAD;
    ULONG		mTIM_7_ENABLE_COUNT;
    ULONG		mTIM_7_LINKING;
    ULONG		mTIM_7_CURRENT;
    ULONG		mTIM_7_TIMER_DONE;
    ULONG		mTIM_7_LAST_CLOCK;
    ULONG		mTIM_7_BORROW_IN;
    ULONG		mTIM_7_BORROW_OUT;
    ULONG		mTIM_7_LAST_LINK_CARRY;
    ULONG		mTIM_7_LAST_COUNT;
    
    ULONG		mAUDIO_0_BKUP;
    ULONG		mAUDIO_0_ENABLE_RELOAD;
    ULONG		mAUDIO_0_ENABLE_COUNT;
    ULONG		mAUDIO_0_LINKING;
    ULONG		mAUDIO_0_CURRENT;
    ULONG		mAUDIO_0_TIMER_DONE;
    ULONG		mAUDIO_0_LAST_CLOCK;
    ULONG		mAUDIO_0_BORROW_IN;
    ULONG		mAUDIO_0_BORROW_OUT;
    ULONG		mAUDIO_0_LAST_LINK_CARRY;
    ULONG		mAUDIO_0_LAST_COUNT;
    SBYTE		mAUDIO_0_VOLUME;
    SBYTE		mAUDIO_0_OUTPUT;
    ULONG		mAUDIO_0_INTEGRATE_ENABLE;
    ULONG		mAUDIO_0_WAVESHAPER;
    
    ULONG		mAUDIO_1_BKUP;
    ULONG		mAUDIO_1_ENABLE_RELOAD;
    ULONG		mAUDIO_1_ENABLE_COUNT;
    ULONG		mAUDIO_1_LINKING;
    ULONG		mAUDIO_1_CURRENT;
    ULONG		mAUDIO_1_TIMER_DONE;
    ULONG		mAUDIO_1_LAST_CLOCK;
    ULONG		mAUDIO_1_BORROW_IN;
    ULONG		mAUDIO_1_BORROW_OUT;
    ULONG		mAUDIO_1_LAST_LINK_CARRY;
    ULONG		mAUDIO_1_LAST_COUNT;
    SBYTE		mAUDIO_1_VOLUME;
    SBYTE		mAUDIO_1_OUTPUT;
    ULONG		mAUDIO_1_INTEGRATE_ENABLE;
    ULONG		mAUDIO_1_WAVESHAPER;
    
    ULONG		mAUDIO_2_BKUP;
    ULONG		mAUDIO_2_ENABLE_RELOAD;
    ULONG		mAUDIO_2_ENABLE_COUNT;
    ULONG		mAUDIO_2_LINKING;
    ULONG		mAUDIO_2_CURRENT;
    ULONG		mAUDIO_2_TIMER_DONE;
    ULONG		mAUDIO_2_LAST_CLOCK;
    ULONG		mAUDIO_2_BORROW_IN;
    ULONG		mAUDIO_2_BORROW_OUT;
    ULONG		mAUDIO_2_LAST_LINK_CARRY;
    ULONG		mAUDIO_2_LAST_COUNT;
    SBYTE		mAUDIO_2_VOLUME;
    SBYTE		mAUDIO_2_OUTPUT;
    ULONG		mAUDIO_2_INTEGRATE_ENABLE;
    ULONG		mAUDIO_2_WAVESHAPER;
    
    ULONG		mAUDIO_3_BKUP;
    ULONG		mAUDIO_3_ENABLE_RELOAD;
    ULONG		mAUDIO_3_ENABLE_COUNT;
    ULONG		mAUDIO_3_LINKING;
    ULONG		mAUDIO_3_CURRENT;
    ULONG		mAUDIO_3_TIMER_DONE;
    ULONG		mAUDIO_3_LAST_CLOCK;
    ULONG		mAUDIO_3_BORROW_IN;
    ULONG		mAUDIO_3_BORROW_OUT;
    ULONG		mAUDIO_3_LAST_LINK_CARRY;
    ULONG		mAUDIO_3_LAST_COUNT;
    SBYTE		mAUDIO_3_VOLUME;
    SBYTE		mAUDIO_3_OUTPUT;
    ULONG		mAUDIO_3_INTEGRATE_ENABLE;
    ULONG		mAUDIO_3_WAVESHAPER;
    
    ULONG		mSTEREO;
    
    //
    // Serial related variables
    //
    ULONG		mUART_RX_IRQ_ENABLE;
    ULONG		mUART_TX_IRQ_ENABLE;
    
    ULONG		mUART_RX_COUNTDOWN;
    ULONG		mUART_TX_COUNTDOWN;
    
    ULONG		mUART_SENDBREAK;
    ULONG		mUART_TX_DATA;
    ULONG		mUART_RX_DATA;
    ULONG		mUART_RX_READY;
    
    ULONG		mUART_PARITY_ENABLE;
    ULONG		mUART_PARITY_EVEN;
    
    int			mUART_CABLE_PRESENT;
    void		(*mpUART_TX_CALLBACK)(int data,ULONG objref);
    ULONG		mUART_TX_CALLBACK_OBJECT;
    
    int			mUART_Rx_input_queue[UART_MAX_RX_QUEUE];
    unsigned int mUART_Rx_input_ptr;
    unsigned int mUART_Rx_output_ptr;
    int			mUART_Rx_waiting;
    int			mUART_Rx_framing_error;
    int			mUART_Rx_overun_error;
    
    //
    // Screen related
    //
    
    UBYTE		*mpDisplayBits;
    UBYTE		*mpDisplayCurrent;
    UBYTE		*mpRamPointer;
    ULONG		mLynxLine;
    ULONG		mLynxLineDMACounter;
    ULONG		mLynxAddr;
    
    ULONG		mDisplayRotate;
    ULONG		mDisplayFormat;
    ULONG		mDisplayPitch;
    UBYTE*		(*mpDisplayCallback)(ULONG objref);
    ULONG		mDisplayCallbackObject;
};


#endif