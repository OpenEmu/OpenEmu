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

#ifndef SKY_MUSIC_MT32MUSIC_H
#define SKY_MUSIC_MT32MUSIC_H

#include "sky/music/musicbase.h"

class MidiDriver;

namespace Sky {

class MT32Music : public MusicBase {
public:
	MT32Music(MidiDriver *pMidiDrv, Audio::Mixer *pMixer, Disk *pDisk);
	~MT32Music();
private:
	static void passTimerFunc(void *param);
	void timerCall();
	bool processPatchSysEx(uint8 *sysExData);
	virtual void setVolume(uint16 volume);

	uint32 _timerCount;
	uint8 *_sysExSequence;
	MidiDriver *_midiDrv;

	virtual void setupPointers();
	virtual void setupChannels(uint8 *channelData);
	virtual void startDriver();
};

} // End of namespace Sky

#endif //MT32MUSIC_H
