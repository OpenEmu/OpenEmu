/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

//////////////////////////////////////////////////////////////////////
// Simple ARM7 stub (sends RTC, TSC, and X/Y data to the ARM 9)
// -- joat
// -- modified by Darkain and others
//////////////////////////////////////////////////////////////////////

//#define USE_LIBCARTRESET

#include <nds.h>

#include <bios.h>
#include <arm7/touch.h>
#include <arm7/clock.h>
#include <arm7/audio.h>
#include <system.h>
#include <stdlib.h>
#include <string.h>
#include <registers_alt.h>		// Needed for SOUND_CR
#include <NDS/scummvm_ipc.h>
//////////////////////////////////////////////////////////////////////
#ifdef USE_DEBUGGER
#include <dswifi7.h>
#endif

#include "cartreset_nolibfat.h"

#define TOUCH_CAL_X1 (*(vs16 *)0x027FFCD8)
#define TOUCH_CAL_Y1 (*(vs16 *)0x027FFCDA)
#define TOUCH_CAL_X2 (*(vs16 *)0x027FFCDE)
#define TOUCH_CAL_Y2 (*(vs16 *)0x027FFCE0)
#define SCREEN_WIDTH    256
#define SCREEN_HEIGHT   192
s32 TOUCH_WIDTH  = TOUCH_CAL_X2 - TOUCH_CAL_X1;
s32 TOUCH_HEIGHT = TOUCH_CAL_Y2 - TOUCH_CAL_Y1;
s32 TOUCH_OFFSET_X = ( ((SCREEN_WIDTH -60) * TOUCH_CAL_X1) / TOUCH_WIDTH  ) - 28;
s32 TOUCH_OFFSET_Y = ( ((SCREEN_HEIGHT-60) * TOUCH_CAL_Y1) / TOUCH_HEIGHT ) - 28;

vu8 *soundData;

vu8 *soundBuffer;
vu8 *arm9Buffer;
bool soundFilled[4];

int playingSection;

bool needSleep = false;
int temp;

int adpcmBufferNum = 0;

// those are pixel positions of the two points you click when calibrating
#define TOUCH_CNTRL_X1   (*(vu8 *)0x027FFCDC)
#define TOUCH_CNTRL_Y1   (*(vu8 *)0x027FFCDD)
#define TOUCH_CNTRL_X2   (*(vu8 *)0x027FFCE2)
#define TOUCH_CNTRL_Y2   (*(vu8 *)0x027FFCE3)


//////////////////////////////////////////////////////////////////////

/*
void startSound(int sampleRate, const void *data, uint32 bytes, u8 channel=0, u8 vol=0x7F,  u8 pan=63, u8 format=0) {
  SCHANNEL_TIMER(channel)  = SOUND_FREQ(sampleRate);
  SCHANNEL_SOURCE(channel) = (uint32)data;
  SCHANNEL_LENGTH(channel) = bytes;
  SCHANNEL_CR(channel)     = SOUND_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(vol) | SOUND_PAN(pan) | (format==1?SOUND_8BIT:SOUND_16BIT);
}


s8 getFreeSoundChannel() {
  for (int i=0; i<16; i++) {
    if ( (SCHANNEL_CR(i) & SOUND_ENABLE) == 0 ) return i;
  }
  return -1;
}
*/


s8 getFreeSoundChannel() {
//  return 0;
  for (int i=0; i<16; i++) {
    if ( (SCHANNEL_CR(i) & SCHANNEL_ENABLE) == 0 ) return i;
  }
  return -1;
}

void startSound(int sampleRate, const void *data, uint32 bytes, u8 channel=0, u8 vol=0x7F,  u8 pan=63, u8 format=0) {
//  REG_IME = IME_DISABLE;

  channel = getFreeSoundChannel();
/*  if (format == 2) {
	channel = 1;
  } else {
	channel = 0;
  }*/

  if (channel > 1) channel = 1;

  bytes &= ~7;		// Multiple of 4 bytes!
//  bytes += 4;

  SCHANNEL_CR(channel) = 0;
  SCHANNEL_TIMER(channel)  = SOUND_FREQ(sampleRate);
  SCHANNEL_SOURCE(channel) = ((uint32) (data));
  SCHANNEL_LENGTH(channel) = ((bytes & 0x7FFFFFFF) >> 2);
  SCHANNEL_REPEAT_POINT(channel) = 0;

  SCHANNEL_CR(channel + 2) = 0;
  SCHANNEL_TIMER(channel + 2)  = SOUND_FREQ(sampleRate);
  SCHANNEL_SOURCE(channel + 2) = ((uint32) (data));
  SCHANNEL_LENGTH(channel + 2) = ((bytes & 0x7FFFFFFF) >> 2);
  SCHANNEL_REPEAT_POINT(channel + 2) = 0;

  uint32 flags = SCHANNEL_ENABLE | SOUND_VOL(vol) | SOUND_PAN(pan);

  switch (format) {
	case 1: {
		flags |= SOUND_FORMAT_8BIT;
		flags |= SOUND_REPEAT;// | (1 << 15);
		break;
	}

	case 0: {
		flags |= SOUND_FORMAT_16BIT;
		flags |= SOUND_REPEAT;// | (1 << 15);
		break;
	}

	case 2: {
		flags |= SOUND_FORMAT_ADPCM;
		flags |= SOUND_ONE_SHOT;// | (1 << 15);

		SCHANNEL_SOURCE(channel) = (unsigned int) IPC->adpcm.buffer[0];
		//bytes += 32;
		SCHANNEL_LENGTH(channel) = (((bytes + 4) & 0x7FFFFFFF) >> 2);

		SCHANNEL_CR(channel + 1) = 0;
		SCHANNEL_SOURCE(channel + 1) = (unsigned int) IPC->adpcm.buffer[0];
		SCHANNEL_LENGTH(channel + 1) = (((bytes + 4) & 0x7FFFFFFF) >> 2);
		SCHANNEL_TIMER(channel + 1) = SOUND_FREQ(sampleRate);
		SCHANNEL_REPEAT_POINT(channel + 1) = 0;
		SCHANNEL_CR(channel + 1) = flags;
		temp = bytes;
		adpcmBufferNum = 0;
		break;
	}
  }


//  if (bytes & 0x80000000) {
//    flags |= SOUND_REPEAT;
//  } else {
//  }




  soundData = (vu8 *) data;

  SCHANNEL_CR(channel)     = flags;
  SCHANNEL_CR(channel + 2)     = flags;



  if (channel == 0) {
	for (volatile int i = 0; i < 16384 * 2; i++) {
		// Delay loop - this makes everything stay in sync!
	}

	TIMER0_CR = 0;
	TIMER0_DATA = SOUND_FREQ(sampleRate) * 2;
	TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1;

	TIMER1_CR = 0;
	TIMER1_DATA = 65536 - ((bytes & 0x7FFFFFFF) >> 3);		// Trigger four times during the length of the buffer
	TIMER1_CR = TIMER_ENABLE | TIMER_IRQ_REQ | TIMER_CASCADE;

    playingSection = 0;
  } else {
	for (volatile int i = 0; i < 16384 * 2; i++) {
		// Delay loop - this makes everything stay in sync!
	}

	TIMER2_CR = 0;
	TIMER2_DATA = SOUND_FREQ(sampleRate) * 2;
	TIMER2_CR = TIMER_ENABLE | TIMER_DIV_1;

	TIMER3_CR = 0;
	TIMER3_DATA = 65536 - ((bytes & 0x7FFFFFFF) >> 3);		// Trigger four times during the length of the buffer
	TIMER3_CR = TIMER_ENABLE | TIMER_IRQ_REQ | TIMER_CASCADE;

	for (int r = 0; r < 4; r++) {
//		IPC->streamFillNeeded[r] = true;
	}

	IPC->streamPlayingSection = 0;
  }



//  IPC->fillSoundFirstHalf = true;
//  IPC->fillSoundSecondHalf = true;
//  soundFirstHalf = true;

//  REG_IME = IME_ENABLE;
}

void stopSound(int chan) {
 SCHANNEL_CR(chan) = 0;
}

void DummyHandler() {
	REG_IF = REG_IF;
}

void powerManagerWrite(uint32 command, u32 data, bool enable) {

  uint16 result;
  SerialWaitBusy();

  // Write the command and wait for it to complete
  REG_SPICNT = SPI_ENABLE | SPI_BAUD_1MHz | (1 << 11);
  REG_SPIDATA = command | 0x80;
  SerialWaitBusy();

  // Write the second command and clock in the data
  REG_SPICNT = SPI_ENABLE | SPI_BAUD_1MHz;
  REG_SPIDATA = 0;
  SerialWaitBusy();

  result = REG_SPIDATA & 0xFF;



  // Write the command and wait for it to complete
  REG_SPICNT = SPI_ENABLE | SPI_BAUD_1MHz | (1 << 11);
  REG_SPIDATA = command;
  SerialWaitBusy();

  // Write the second command and clock in the data
  REG_SPICNT = SPI_ENABLE | SPI_BAUD_1MHz;
  REG_SPIDATA = enable? (result | data): (result & ~data);
  SerialWaitBusy();
}

/*
void performSleep() {

  powerManagerWrite(0, 0x30, true);

  // Here, I set up a dummy interrupt handler, then trigger all interrupts.
  // These are just aknowledged by the handler without doing anything else.
  // Why?  Because without it the sleep mode will only happen once, and then
  // never again.  I got the idea from reading the MoonShell source.
  IME = 0;
  u32 irq = (u32) IRQ_HANDLER;
  IRQ_HANDLER = DummyHandler;
  IF = ~0;
  IME = 1;


  // Now save which interrupts are enabled, then set only the screens unfolding
  // interrupt to be enabled, so that the first interrupt that happens is the
  // one I want.
  int saveInts = IE;



  IE = IRQ_TIMER0;		// Screens unfolding interrupt

  // Now call the sleep function in the bios
  bool b;
  do {
    TIMER0_CR = 0;
	TIMER0_DATA = TIMER_FREQ(20);
	TIMER0_CR = TIMER_ENABLE | TIMER_DIV_64;

	swiDelay(100);

	swiSleep();

	swiDelay(100);

	powerManagerWrite(0, 0x30, b = !b);
  } while (!(TIMER0_CR & TIMER_ENABLE));

  TIMER0_CR = 0;

  // We're back from sleep, now restore the interrupt state and IRQ handler
  IRQ_HANDLER = (void (*)()) irq;
  IE = saveInts;
  IF = ~0;
  IME = 1;



  powerManagerWrite(0, 0x30, false);
}

*/
void performSleep() {
  powerManagerWrite(0, 0x30, true);

  IPC->performArm9SleepMode = true;	// Tell ARM9 to sleep

//  u32 irq = (u32) IRQ_HANDLER;
//  IRQ_HANDLER = DummyHandler;
//  POWER_CR &= ~POWER_SOUND;

//  int saveInts = REG_IE;
//  REG_IE = (1 << 22) | IRQ_VBLANK;		// Lid open
//  *((u32 *) (0x0380FFF8)) = *((u32 *) (0x0380FFF8)) | (REG_IE & REG_IF);
//  VBLANK_INTR_WAIT_FLAGS = IRQ_VBLANK;


  int r = 0;
  while ((REG_KEYXY & (1 << 7))) {		// Wait for lid to open
	swiDelay(1000000);
	r++;
  }

//  IRQ_HANDLER = (void (*)()) irq;
  IPC->performArm9SleepMode = false;	// Tell ARM9 to wake up
//  REG_IE = saveInts;

//  POWER_CR |= POWER_SOUND;

  powerManagerWrite(0, 0x30, false);
}

void powerOff() {
	powerManagerWrite(0, 0x40, true);
}
//////////////////////////////////////////////////////////////////////



void InterruptTimer1() {

	IPC->fillNeeded[playingSection] = true;
	soundFilled[playingSection] = false;

	if (playingSection == 3) {
//		IME = IME_DISABLED;

	//	while (SCHANNEL_CR(0) & SCHANNEL_ENABLE) {
	//	}
//		SCHANNEL_CR(0) &= ~SCHANNEL_ENABLE;

//		SCHANNEL_CR(0) |= SCHANNEL_ENABLE;
//		TIMER1_CR = 0;
//		TIMER1_CR = TIMER_ENABLE | TIMER_IRQ_REQ | TIMER_CASCADE;

		playingSection = 0;

//		IME = IME_ENABLED;
	} else {
		playingSection++;
	}

	IPC->playingSection = playingSection;

/*	for (int r = 0; r < 4; r++) {
		//if ((!soundFilled[r]) && (!IPC->fillNeeded[playingSection])) {
			memcpy((void *) (soundBuffer + (r * 1024)), (void *) (arm9Buffer + (r * 1024)), 1024);

			vu16 *p = (vu16 *) (soundBuffer);
			//for (int t = 0; t < 2048; t++) {
		//		*(p + t) = (t & 1)? 0xF000: 0x0000;
			//}
			soundFilled[r] = true;
		//}
	}*/
}

void InterruptTimer3() {
	while (IPC->adpcm.semaphore);		// Wait for buffer to become free if needed
	IPC->adpcm.semaphore = true;		// Lock the buffer structure to prevent clashing with the ARM7

	IPC->streamFillNeeded[IPC->streamPlayingSection] = true;

	if (IPC->streamPlayingSection == 3) {
		IPC->streamPlayingSection = 0;
	} else {
		IPC->streamPlayingSection++;
	}


	IPC->adpcm.semaphore = false;
}

//  IPC->performArm9SleepMode = false;

	// precalculate some values
//  static int16 TOUCH_WIDTH  = TOUCH_CAL_X2 - TOUCH_CAL_X1;
//  static int16 TOUCH_HEIGHT = TOUCH_CAL_Y2 - TOUCH_CAL_Y1;
//  static int16 CNTRL_WIDTH  = TOUCH_CNTRL_X2 - (TOUCH_CNTRL_X1 - 8);
//  static int16 CNTRL_HEIGHT = TOUCH_CNTRL_Y2 - (TOUCH_CNTRL_Y1 - 8);




 void InterruptVBlank() {
    uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0, batt=0, aux=0;
    int t1=0, t2=0;
    uint32 temp=0;
    uint8 ct[sizeof(IPC->curtime)];
	static int heartbeat = 0;
    // Update the heartbeat
    heartbeat++;

    // Read the X/Y buttons and the /PENIRQ line
    but = REG_KEYXY;
    if (!(but & 0x40)) {
      // Read the touch screen
	touchPosition p;
	touchReadXY(&p);

//      x = touchRead(TSC_MEASURE_X);
 //     y = touchRead(TSC_MEASURE_Y);

	  x = p.rawx;
	  y = p.rawy;

	  xpx = p.px;
	  ypx = p.py;

//      xpx = ( ((SCREEN_WIDTH -60) * x) / TOUCH_WIDTH  ) - TOUCH_OFFSET_X;
  //    ypx = ( ((SCREEN_HEIGHT-60) * y) / TOUCH_HEIGHT ) - TOUCH_OFFSET_Y;

//	  xpx = (IPC->touchX - (int16) TOUCH_CAL_X1) * CNTRL_WIDTH  / TOUCH_WIDTH  + (int16) (TOUCH_CNTRL_X1 - 8);
	//  ypx = (IPC->touchY - (int16) TOUCH_CAL_Y1) * CNTRL_HEIGHT / TOUCH_HEIGHT + (int16) (TOUCH_CNTRL_Y1 - 8);


      z1 = touchRead(TSC_MEASURE_Z1);
      z2 = touchRead(TSC_MEASURE_Z2);
    }

    if (but & (1 << 7)) {		// Check if screen is folded
	  needSleep = true;
	}


    batt = touchRead(TSC_MEASURE_BATTERY);
    aux  = touchRead(TSC_MEASURE_AUX);

    // Read the time
    rtcGetTime((uint8 *)ct);
    BCDToInteger((uint8 *)&(ct[1]), 7);

    // Read the temperature
    temp = touchReadTemperature(&t1, &t2);


    // Update the IPC struct
    IPC->heartbeat = heartbeat;
    IPC->buttons   = but;
    IPC->touchX    = x;
    IPC->touchY    = y;
    IPC->touchXpx  = xpx;
    IPC->touchYpx  = ypx;
    IPC->touchZ1   = z1;
    IPC->touchZ2   = z2;
    IPC->battery   = batt;
    IPC->aux       = aux;

    for (u32 i=0; i<sizeof(ct); i++) {
      IPC->curtime[i] = ct[i];
    }

    IPC->temperature = temp;
    IPC->tdiode1 = t1;
    IPC->tdiode2 = t2;



	//sound code  :)
    TransferSound *snd = IPC->soundData;
    IPC->soundData = 0;
    if (snd) {
      for (int i=0; i<snd->count; i++) {
        s8 chan = getFreeSoundChannel();
		if (snd->data[i].rate > 0) {
			if (chan >= 0) {
			  startSound(snd->data[i].rate, snd->data[i].data, snd->data[i].len, chan, snd->data[i].vol, snd->data[i].pan, snd->data[i].format);
			}
		} else {
			stopSound(-snd->data[i].rate);
		}
      }
    }


 #ifdef USE_DEBUGGER
    Wifi_Update(); // update wireless in vblank
 #endif
}

//////////////////////////////////////////////////////////////////////


#ifdef USE_DEBUGGER

// callback to allow wifi library to notify arm9
void arm7_synctoarm9() { // send fifo message
   REG_IPC_FIFO_TX = 0x87654321;
}
// interrupt handler to allow incoming notifications from arm9
void arm7_fifo() { // check incoming fifo messages
   u32 msg = REG_IPC_FIFO_RX;
   if (msg==0x87654321) Wifi_Sync();
}



void initDebugger() {

	// set up the wifi irq
	irqSet(IRQ_WIFI, Wifi_Interrupt); // set up wifi interrupt
	irqEnable(IRQ_WIFI);

    //get them talking together

	// sync with arm9 and init wifi
	u32 fifo_temp;

	while (1) { // wait for magic number
		while (REG_IPC_FIFO_CR&IPC_FIFO_RECV_EMPTY) swiWaitForVBlank();
		fifo_temp=REG_IPC_FIFO_RX;
		if (fifo_temp==0x12345678) break;
	}

	while (REG_IPC_FIFO_CR&IPC_FIFO_RECV_EMPTY) swiWaitForVBlank();
	fifo_temp=REG_IPC_FIFO_RX; // give next value to wifi_init
	Wifi_Init(fifo_temp);

	irqSet(IRQ_FIFO_NOT_EMPTY,arm7_fifo); // set up fifo irq
	irqEnable(IRQ_FIFO_NOT_EMPTY);
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;

	Wifi_SetSyncHandler(arm7_synctoarm9); // allow wifi lib to notify arm9
	// arm7 wifi init complete

}
#endif

#ifdef USE_LIBCARTRESET
void reboot() {
	cartExecute();
}
#endif


int main(int argc, char ** argv) {


#ifdef USE_DEBUGGER
  REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;
#endif

  // Reset the clock if needed
  rtcReset();

  //enable sound
//  powerOn(POWER_SOUND);
  SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);
  IPC->soundData = 0;
  IPC->reset = false;


 //fifoInit();

  for (int r = 0; r < 8; r++) {
	IPC->adpcm.arm7Buffer[r] = (u8 *) malloc(512);
  }

  for (int r = 0; r < 4; r++) {
	soundFilled[r] = false;
  }


  // Set up the interrupt handler

  irqInit();

  irqSet(IRQ_VBLANK, InterruptVBlank);
  irqEnable(IRQ_VBLANK);

  irqSet(IRQ_TIMER1, InterruptTimer1);
  irqEnable(IRQ_TIMER1);

  irqSet(IRQ_TIMER3, InterruptTimer3);
  irqEnable(IRQ_TIMER3);

/*  REG_IME = 0;
  IRQ_HANDLER = &InterruptHandler;
  REG_IE = IRQ_VBLANK | IRQ_TIMER1 | IRQ_TIMER3;
  REG_IF = ~0;
  DISP_SR = DISP_VBLANK_IRQ;
  REG_IME = 1;
  */


#ifdef USE_DEBUGGER
  initDebugger();
#endif

  // Keep the ARM7 out of main RAM
  while ((1)) {
	if (needSleep) {
		performSleep();
		needSleep = false;
	}

#ifdef USE_LIBCARTRESET
	if (passmeloopQuery()) {
		reboot();
	}
#endif

	if (IPC->reset) {
		powerOff();
	}

	swiWaitForVBlank();
  }
  return 0;
}


//////////////////////////////////////////////////////////////////////
