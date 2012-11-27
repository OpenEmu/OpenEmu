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

// Music class

#ifndef AGI_SOUND_MIDI_H
#define AGI_SOUND_MIDI_H

#include "agi/sound.h"

#include "audio/midiplayer.h"

namespace Agi {

class MIDISound : public AgiSound {
public:
	MIDISound(uint8 *data, uint32 len, int resnum, SoundMgr &manager);
	~MIDISound() { free(_data); }
	virtual uint16 type() { return _type; }
	uint8 *_data; ///< Raw sound resource data
	uint32 _len;  ///< Length of the raw sound resource

protected:
	uint16 _type; ///< Sound resource type
};

class SoundGenMIDI : public SoundGen, public Audio::MidiPlayer {
public:
	SoundGenMIDI(AgiBase *vm, Audio::Mixer *pMixer);

	void play(int resnum);
	// We must overload stop() here to implement the pure virtual
	// stop() method of the SoundGen class.
	void stop() { Audio::MidiPlayer::stop(); }

	// MidiDriver_BASE interface implementation
	virtual void send(uint32 b);

	// Overload Audio::MidiPlayer method
	virtual void sendToChannel(byte channel, uint32 b);
	virtual void endOfTrack();

private:
	bool _isGM;

	SoundMgr *_manager;
};

} // End of namespace Agi

#endif
