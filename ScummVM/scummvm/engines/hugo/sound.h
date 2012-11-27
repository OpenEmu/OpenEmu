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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_SOUND_H
#define HUGO_SOUND_H

#include "audio/mixer.h"
#include "audio/midiplayer.h"
#include "audio/softsynth/pcspk.h"

namespace Hugo {

class MidiPlayer : public Audio::MidiPlayer {
public:
	MidiPlayer();

	void pause(bool p);
	void play(uint8 *stream, uint16 size);

	// The following line prevents compiler warnings about hiding the pause()
	// method from the parent class.
	// FIXME: Maybe the pause(bool p) method should be removed and the
	// pause/resume methods of the parent class be used instead?
	virtual void pause() { Audio::MidiPlayer::pause(); }

	uint32 getBaseTempo();

	// Overload Audio::MidiPlayer method
	virtual void sendToChannel(byte channel, uint32 b);
	virtual void onTimer();

private:
	bool _paused;
};

class SoundHandler {
public:
	SoundHandler(HugoEngine *vm);
	~SoundHandler();

	static const int kHugoCNT = 1190000;

	int8 _pcspkrTimer;                              // Timer (ticks) for note being played
	int8 _pcspkrOctave;                             // Current octave 1..7
	int8 _pcspkrNoteDuration;                       // Current length of note (ticks)

	const char *_DOSSongPtr;
	const char *_DOSIntroSong;

	void toggleMusic();
	void toggleSound();
	void setMusicVolume();
	static void loopPlayer(void *refCon);
	void pcspkr_player();
	void playMusic(int16 tune);
	void playSound(int16 sound, const byte priority);
	void initSound();
	void syncVolume();
	void checkMusic();
	void loadIntroSong(Common::ReadStream &in);
	void initPcspkrPlayer();
protected:
	byte _curPriority;                              // Priority of currently playing sound

private:
	HugoEngine *_vm;
	Audio::SoundHandle _soundHandle;
	MidiPlayer *_midiPlayer;
	Audio::PCSpeaker *_speakerStream;
	Audio::SoundHandle _speakerHandle;

	void stopSound();
	void stopMusic();
	void playMIDI(SoundPtr seqPtr, uint16 size);
};

} // End of namespace Hugo

#endif //HUGO_SOUND_H
