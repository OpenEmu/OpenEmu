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

#ifndef SCI_MIDIPARSER_H
#define SCI_MIDIPARSER_H

#include "sci/resource.h"
#include "sci/sound/music.h"
#include "audio/midiparser.h"

/*
 Sound drivers info: (from driver cmd0)
 AdLib/SB  : track 0 , voices 9 , patch 3	ah=1
 ProAudioSp: track 0 , voices 9 , patch 3	ah=17
 GenerlMIDI: track 7 , voices 32, patch 4	ah=1   SCI1.1
 Game Blast: track 9 , voices 12, patch 101	ah=1
 MT-32	  : track 12, voices 32, patch 1	ah=1
 PC Speaker: track 18, voices 1 , patch 0xFF ah=1
 Tandy	  : track 19, voices 3 , patch 101	ah=1
 IBM PS/1  : track 19, voices 3 , patch 101	ah=1

 */

namespace Sci {

/**
 * An extended standard MIDI (SMF) parser. Sierra used an extra channel
 * with special commands for extended functionality and animation syncing.
 * Refer to MidiParser_SMF() in /sound/midiparser_smf.cpp for the standard
 * MIDI (SMF) parser functionality that the SCI MIDI parser is based on
 */
class MidiParser_SCI : public MidiParser {
public:
	MidiParser_SCI(SciVersion soundVersion, SciMusic *music);
	~MidiParser_SCI();

	void mainThreadBegin();
	void mainThreadEnd();

	bool loadMusic(SoundResource::Track *track, MusicEntry *psnd, int channelFilterMask, SciVersion soundVersion);
	bool loadMusic(byte *, uint32) {
		return false;
	}
	void sendInitCommands();
	void unloadMusic();
	void setMasterVolume(byte masterVolume);
	void setVolume(byte volume);
	void stop() {
		_abortParse = true;
		allNotesOff();
	}
	void pause() {
		allNotesOff();
		if (_resetOnPause)
			jumpToTick(0);
	}

	void allNotesOff();

	const byte *getMixedData() const { return _mixedData; }
	byte getSongReverb();

	void tryToOwnChannels();
	void lostChannels();
	void sendFromScriptToDriver(uint32 midi);
	void sendToDriver(uint32 midi);
	void sendToDriver(byte status, byte firstOp, byte secondOp) {
		sendToDriver(status | ((uint32)firstOp << 8) | ((uint32)secondOp << 16));
	}

protected:
	void parseNextEvent(EventInfo &info);
	byte *midiMixChannels();
	byte *midiFilterChannels(int channelMask);
	byte midiGetNextChannel(long ticker);

	SciMusic *_music;

	// this is set, when main thread calls us -> we send commands to queue instead to driver
	bool _mainThreadCalled;

	SciVersion _soundVersion;
	byte *_mixedData;
	SoundResource::Track *_track;
	MusicEntry *_pSnd;
	uint32 _loopTick;
	byte _masterVolume; // the overall master volume (same for all tracks)
	byte _volume; // the global volume of the current track

	bool _signalSet;
	int16 _signalToSet;
	bool _dataincAdd;
	int16 _dataincToAdd;
	bool _resetOnPause;

	bool _channelUsed[16];
	int16 _channelRemap[16];
	bool _channelMuted[16];
	byte _channelVolume[16];
};

} // End of namespace Sci

#endif
