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

#ifndef TINSEL_MUSIC_H
#define TINSEL_MUSIC_H

#include "audio/midiplayer.h"
#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/mutex.h"

class MidiParser;

namespace Tinsel {

bool PlayMidiSequence(		// Plays the specified MIDI sequence through the sound driver
	uint32 dwFileOffset,		// handle of MIDI sequence data
	bool bLoop);			// Whether to loop the sequence

bool MidiPlaying();		// Returns TRUE if a Midi tune is currently playing

bool StopMidi();		// Stops any currently playing midi

void SetMidiVolume(		// Sets the volume of the MIDI music. Returns the old volume
	int vol);		// new volume - 0..MAXMIDIVOL

int GetMidiVolume();

void OpenMidiFiles();
void DeleteMidiBuffer();

void CurrentMidiFacts(SCNHANDLE	*pMidi, bool *pLoop);
void RestoreMidiFacts(SCNHANDLE	Midi, bool Loop);

int GetTrackNumber(SCNHANDLE hMidi);
SCNHANDLE GetTrackOffset(int trackNumber);

void dumpMusic();

class MidiMusicPlayer : public Audio::MidiPlayer {
public:
	MidiMusicPlayer();

	virtual void setVolume(int volume);

	void playMIDI(uint32 size, bool loop);

//	void stop();
	void pause();
	void resume();

	// MidiDriver_BASE interface implementation
	virtual void send(uint32 b);

	// The original sets the "sequence timing" to 109 Hz, whatever that
	// means. The default is 120.
	uint32 getBaseTempo()	{ return _driver ? (109 * _driver->getBaseTempo()) / 120 : 0; }

private:
	void playXMIDI(uint32 size, bool loop);
	void playSEQ(uint32 size, bool loop);
};

class PCMMusicPlayer : public Audio::AudioStream {
public:
	PCMMusicPlayer();
	~PCMMusicPlayer();

	bool isPlaying() const;

	bool isDimmed() const;

	void getTunePlaying(void *voidPtr, int length);
	void restoreThatTune(void *voidPtr);

	void setMusicSceneDetails(SCNHANDLE hScript, SCNHANDLE hSegment, const char *fileName);

	void setVolume(int volume);

	void startPlay(int id);
	void stopPlay();

	bool getMusicTinselDimmed() const;
	void dim(bool bTinselDim);
	void unDim(bool bTinselUnDim);
	void dimIteration();

	void startFadeOut(int ticks);
	void fadeOutIteration();

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return _end; }
	bool endOfStream() const { return false; }
	int getRate() const { return 22050; }

protected:
	enum State {
		S_IDLE,
		S_NEW,
		S_MID,
		S_END1,
		S_END2,
		S_END3,
		S_NEXT,
		S_STOP
	};

	struct MusicSegment {
		uint32 numChannels;
		uint32 bitsPerSec;
		uint32 bitsPerSample;
		uint32 sampleLength;
		uint32 sampleOffset;
	};

	Audio::SoundHandle _handle;
	Audio::AudioStream *_curChunk;
	Common::Mutex _mutex;

	bool _end;

	int _silenceSamples;

	State _state, _mState;
	bool _forcePlay;
	int32 _scriptNum;
	int32 _scriptIndex;
	SCNHANDLE _hScript;
	SCNHANDLE _hSegment;
	Common::String _filename;

	uint8 _volume;

	bool _dimmed;
	bool _dimmedTinsel;
	uint8 _dimmedVolume;
	int _dimIteration;
	int _dimPosition;

	uint8 _fadeOutVolume;
	int _fadeOutIteration;

	void play();
	void stop();
	void setVol(uint8 volume);

	bool getNextChunk();
};

} // End of namespace Tinsel

#endif
