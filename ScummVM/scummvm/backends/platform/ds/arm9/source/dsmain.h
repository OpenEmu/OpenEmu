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

#ifndef _DSMAIN_H
#define _DSMAIN_H

#include <nds.h>
#include "osystem_ds.h"

namespace DS {


enum controlType {
	CONT_SCUMM_ORIGINAL,
	CONT_SCUMM_SAMNMAX,
	CONT_SKY,
	CONT_SIMON,
	CONT_FUTURE_WARS,
	CONT_AGI,
	CONT_GOBLINS,
	CONT_NIPPON
};

struct gameListType {
	char 			gameId[16];
	controlType 	control;
};

// Pen reading functions
void 	penInit();
void 	penUpdate();
bool 	getPenDown();
bool 	getPenHeld();
bool 	getPenReleased();
int 	getPenX();
int 	getPenY();
GLvector getPenPos();
void 	consumePenEvents();
controlType getControlType();

// Pad reading
int 	getKeysHeld();
void 	keysUpdate();
int 	getKeysDown();
int 	getKeysReleased();
void 	consumeKeys();
int 	leftHandedSwap(int keys);
void	setGameScreenSwap(bool enable);
void	setSensitivity(int sensitivity);

// Video
void 	displayMode8Bit();											// Switch to 8-bit mode5
void 	displayMode16Bit();										// Switch to 16-bit mode5

// Flip double buffer
void 	displayMode16BitFlipBuffer();

// Get address of current back buffer
u16 *	get16BitBackBuffer();
u16 *	get8BitBackBuffer();
s32 	get8BitBackBufferStride();
u16*	getScalerBuffer();

void 	setTalkPos(int x, int y);
void 	setTopScreenTarget(int x, int y);
void	set200PercentFixedScale(bool on);
void	setTopScreenZoom(int percentage);
void	setGamma(int gamma);

// Timers
void 	setTimerCallback(OSystem_DS::TimerProc proc, int interval);		// Setup a callback function at a regular interval
int 	getMillis();													// Return the current runtime in milliseconds
void 	doTimerCallback();												// Call callback function if required

// Sound
void 	doSoundCallback();
void 	startSound(int freq, int buffer);	// Start sound hardware
// Call function if sound buffers need more data
void 	playSound(const void *data, u32 length, bool loop, bool adpcm = false, int rate = 22050);		// Start a sound
void 	stopSound(int channel);
int		getSoundFrequency();

// Event queue
void 	addEventsToQueue();
void 	VBlankHandler();

// Sam and Max Stuff
void 	setGameID(int id);
void 	setCursorIcon(const u8 *icon, uint w, uint h, byte keycolor, int hotspotX, int hotspotY);
void	setShowCursor(bool enable);
void	setMouseCursorVisible(bool visible);

// Shake
void 	setShakePos(int shakePos);

// Reports
void 	memoryReport();

// GBAMP
bool 	isGBAMPAvailable();

// Sleep (I'd like some of that right now)
void 	checkSleepMode();

// Virtual keyboard
void 	setKeyboardIcon(bool enable);
bool 	getKeyboardIcon();
void 	setKeyboardEnable(bool en);
bool 	getKeyboardEnable();

// Options
void 	setLeftHanded(bool enable);
void 	setTouchXOffset(int x);
void 	setTouchYOffset(int y);
void 	setUnscaledMode(bool enable);
void	setSnapToBorder(bool enable);
void 	setIndyFightState(bool st);
bool 	getIndyFightState();
bool    isCpuScalerEnabled();
void	setCpuScalerEnable(bool enable);
void	setTrackPadStyleEnable(bool enable);
void	setTapScreenClicksEnable(bool enable);

// Display
bool 	getIsDisplayMode8Bit();
void 	setGameSize(int width, int height);
int		getGameWidth();
int		getGameHeight();

// Fast RAM allocation (ITCM)
void	fastRamReset();
void*	fastRamAlloc(int size);

void 	exitGame();


}	// End of namespace DS



int cygprofile_getHBlanks();

#endif
