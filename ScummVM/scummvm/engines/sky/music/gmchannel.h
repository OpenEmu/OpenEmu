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

#ifndef SKY_MUSIC_GMCHANNEL_H
#define SKY_MUSIC_GMCHANNEL_H

#include "sky/music/musicbase.h"

class MidiDriver;

namespace Sky {

typedef struct {
	uint16 eventDataPtr;
	int32 nextEventTime;
	uint16 loopPoint;
	uint8 midiChannelNumber;
	uint8 note;
	bool channelActive;
} MidiChannelType;

class GmChannel : public ChannelBase {
public:
	GmChannel(uint8 *pMusicData, uint16 startOfData, MidiDriver *pMidiDrv, const byte *pInstMap, const byte *veloTab);
	~GmChannel();
	virtual void stopNote();
	virtual uint8 process(uint16 aktTime);
	virtual void updateVolume(uint16 pVolume);
	virtual bool isActive();
private:
	const byte *_instMap;
	const byte *_veloTab;
	MidiDriver *_midiDrv;
	uint8 *_musicData;
	uint16 _musicVolume;
	MidiChannelType _channelData;
	uint8 _currentChannelVolume;
	//-                          normal subs
	void setRegister(uint8 regNum, uint8 value);
	int32 getNextEventTime();
	uint16 getNextNote(uint8 param);
	void adlibSetupInstrument();
	void setupInstrument(uint8 opcode);
	void setupChannelVolume(uint8 volume);
	//-                          Streamfunctions from Command90hTable
	void com90_caseNoteOff();        // 0
	void com90_stopChannel();        // 1
	void com90_setupInstrument();    // 2
	uint8 com90_updateTempo();       // 3
	//void com90_dummy();            // 4
	void com90_getPitch();           // 5
	void com90_getChannelVolume();   // 6
	//void com90_skipTremoVibro();   // 7
	void com90_loopMusic();        // 8
	void com90_keyOff();             // 9
	//void com90_error();            // 10
	void com90_getChannelPanValue(); // 11
	void com90_setLoopPoint();     // 12
	void com90_getChannelControl();     // 13
};

} // End of namespace Sky

#endif //SKYGMCHANNEL_H
