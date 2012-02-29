// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 2008 VBA-M development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef __VBA_SOUND_DRIVER_H__
#define __VBA_SOUND_DRIVER_H__

#include "Types.h"

/**
 * Sound driver abstract interface for the core to use to output sound.
 * Subclass this to implement a new sound driver.
 */
class SoundDriver
{
public:

	/**
	 * Destructor. Free the resources allocated by the sound driver.
	 */
	virtual ~SoundDriver() { };

	/**
	 * Initialize the sound driver.
	 * @param sampleRate In Hertz
	 */
	virtual bool init(long sampleRate) = 0;

	/**
	 * Tell the driver that the sound stream has paused
	 */
	virtual void pause() = 0;

	/**
	 * Reset the sound driver
	 */
	virtual void reset() = 0;

	/**
	 * Tell the driver that the sound stream has resumed
	 */
	virtual void resume() = 0;

	/**
	 * Write length bytes of data from the finalWave buffer to the driver output buffer.
	 */
	virtual void write(u16 * finalWave, int length) = 0;

	virtual void setThrottle(unsigned short throttle) { };
};

#endif // __VBA_SOUND_DRIVER_H__
