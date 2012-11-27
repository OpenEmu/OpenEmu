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

#ifndef SCUMM_PLAYER_V2CMS_H
#define SCUMM_PLAYER_V2CMS_H

#include "scumm/player_v2base.h"	// for channel_data

class CMSEmulator;

namespace Scumm {

/**
 * Scumm V2 CMS/Gameblaster MIDI driver.
 */
class Player_V2CMS : public Player_V2Base {
public:
	Player_V2CMS(ScummEngine *scumm, Audio::Mixer *mixer);
	virtual ~Player_V2CMS();

	// MusicEngine API
	virtual void setMusicVolume(int vol);
	virtual void startSound(int sound);
	virtual void stopSound(int sound);
	virtual void stopAllSounds();
	virtual int  getMusicTimer();
	virtual int  getSoundStatus(int sound) const;

	// AudioStream API
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return true; }

private:
	struct Voice {
		byte attack;
		byte decay;
		byte sustain;
		byte release;
		byte octadd;
		int16 vibrato;
		int16 vibrato2;
		int16 noise;
	};

	struct Voice2 {
		byte *amplitudeOutput;
		byte *freqOutput;
		byte *octaveOutput;

		uint8 channel;
		int8 sustainLevel;
		uint8 attackRate;
		uint8 maxAmpl;
		uint8 decayRate;
		uint8 sustainRate;
		uint8 releaseRate;
		uint8 releaseTime;
		int8 vibratoRate;
		int8 vibratoDepth;

		int8 curVibratoRate;
		int8 curVibratoUnk;

		int8 unkVibratoRate;
		int8 unkVibratoDepth;

		int8 unkRate;
		int8 unkCount;

		enum EnvelopeState {
			kEnvelopeAttack,
			kEnvelopeDecay,
			kEnvelopeSustain,
			kEnvelopeRelease
		};

		EnvelopeState nextProcessState;
		uint8 curVolume;
		uint8 curOctave;
		uint8 curFreq;

		int8 octaveAdd;

		int8 playingNote;
		Voice2 *nextVoice;

		byte chanNumber;
	};

	struct MusicChip {
		byte ampl[4];
		byte freq[4];
		byte octave[2];
	};

	Voice _cmsVoicesBase[16];
	Voice2 _cmsVoices[8];
	MusicChip _cmsChips[2];

	uint8 _tempo;
	uint8 _tempoSum;
	byte _looping;
	byte _octaveMask;
	int16 _midiDelay;
	Voice2 *_midiChannel[16];
	byte _midiChannelUse[16];
	byte *_midiData;
	byte *_midiSongBegin;

	int _loadedMidiSong;

	byte _sfxFreq[4], _sfxAmpl[4], _sfxOctave[2];

	byte _lastMidiCommand;
	uint _outputTableReady;
	byte _voiceTimer;

	int _musicTimer, _musicTimerTicks;

	void loadMidiData(byte *data, int sound);
	void play();

	void processChannel(Voice2 *channel);
	void processRelease(Voice2 *channel);
	void processAttack(Voice2 *channel);
	void processDecay(Voice2 *channel);
	void processSustain(Voice2 *channel);
	void processVibrato(Voice2 *channel);

	void playMusicChips(const MusicChip *table);
	void playNote(byte *&data);
	void clearNote(byte *&data);
	void offAllChannels();
	void playVoice();
	void processMidiData();

	Voice2 *getFreeVoice();
	Voice2 *getPlayVoice(byte param);

	struct MidiNote {
		byte frequency;
		byte baseOctave;
	};

	static const MidiNote _midiNotes[132];
	static const byte _attackRate[16];
	static const byte _decayRate[16];
	static const byte _sustainRate[16];
	static const byte _releaseRate[16];
	static const byte _volumeTable[16];
	static const byte _cmsInitData[26];

	CMSEmulator *_cmsEmu;
};

} // End of namespace Scumm

#endif
