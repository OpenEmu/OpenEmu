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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SCUMMVM_IPC_INCLUDE
#define SCUMMVM_IPC_INCLUDE

//////////////////////////////////////////////////////////////////////

#include <nds/ndstypes.h>
#include <nds/ipc.h>

//////////////////////////////////////////////////////////////////////


typedef struct {
	const void *data;
	u32 len;
	u32 rate;
	u8 vol;
	u8 pan;
	u8 format;
	u8 PADDING;
} TransferSoundData;




//---------------------------------------------------------------------------------
typedef struct {
	TransferSoundData data[16];
	u8 count;
	u8 PADDING[3];
} TransferSound;



typedef struct {
	u8 *buffer[8];
	bool filled[8];
	u8 *arm7Buffer[8];
	bool arm7Dirty[8];
	bool semaphore;
} adpcmBuffer;

//////////////////////////////////////////////////////////////////////

typedef struct scummvmTransferRegion {
  uint32 heartbeat;          // counts frames

   int16 touchX,   touchY;   // TSC X, Y
   int16 touchXpx, touchYpx; // TSC X, Y pixel values
   int16 touchZ1,  touchZ2;  // TSC x-panel measurements
  uint16 tdiode1,  tdiode2;  // TSC temperature diodes
  uint32 temperature;        // TSC computed temperature

  uint16 buttons;            // X, Y, /PENIRQ buttons

  union {
    uint8 curtime[8];        // current time response from RTC

    struct {
      u8 command;
      u8 year;           //add 2000 to get 4 digit year
      u8 month;          //1 to 12
      u8 day;            //1 to (days in month)

      u8 incr;
      u8 hours;          //0 to 11 for AM, 52 to 63 for PM
      u8 minutes;        //0 to 59
      u8 seconds;        //0 to 59
    } rtc;
  };

  uint16 battery;            // battery life ??  hopefully.  :)
  uint16 aux;                // i have no idea...

  TransferSound *soundData;

  adpcmBuffer adpcm;


  // Don't rely on these below, will change or be removed in the future
  vuint32 mailAddr;
  vuint32 mailData;
  vuint8 mailRead;
  vuint8 mailBusy;
  vuint32 mailSize;

  bool performArm9SleepMode;

  u32 test;
  int tweak;
  bool tweakChanged;

//  bool fillSoundFirstHalf;
//  bool fillSoundSecondHalf;

  // These are used for ScummVMs sound output
  bool fillNeeded[4];
  int playingSection;

  bool reset;

  // Streaming sound
  bool streamFillNeeded[4];
  int streamPlayingSection;
} scummTransferRegion;

//////////////////////////////////////////////////////////////////////

#undef IPC
#define IPC ((scummTransferRegion volatile *)(0x027FF000))


#endif
