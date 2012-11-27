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

#ifndef AUDIO_MPU401_H
#define AUDIO_MPU401_H

#include "audio/mididrv.h"

////////////////////////////////////////
//
// Common MPU401 implementation methods
//
////////////////////////////////////////

class MidiChannel_MPU401 : public MidiChannel {

private:
	MidiDriver *_owner;
	bool _allocated;
	byte _channel;

public:
	MidiDriver *device();
	byte getNumber() { return _channel; }
	virtual void release() { _allocated = false; }

	virtual void send(uint32 b);

	// Regular messages
	virtual void noteOff(byte note);
	virtual void noteOn(byte note, byte velocity);
	virtual void programChange(byte program);
	virtual void pitchBend(int16 bend);

	// Control Change messages
	virtual void controlChange(byte control, byte value);
	virtual void pitchBendFactor(byte value);

	// SysEx messages
	virtual void sysEx_customInstrument(uint32 type, const byte *instr);

	// Only to be called by the owner
	void init(MidiDriver *owner, byte channel);
	bool allocate();
};



class MidiDriver_MPU401 : public MidiDriver {
private:
	MidiChannel_MPU401 _midi_channels[16];
	Common::TimerManager::TimerProc _timer_proc;
	uint16 _channel_mask;

public:
	MidiDriver_MPU401();
	virtual ~MidiDriver_MPU401();

	virtual void close();
	virtual void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc);
	virtual uint32 getBaseTempo(void) { return 10000; }
	virtual uint32 property(int prop, uint32 param);

	virtual MidiChannel *allocateChannel();
	virtual MidiChannel *getPercussionChannel() { return &_midi_channels[9]; }
};


#endif
