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

#ifndef BACKENDS_AUDIOCD_DEFAULT_H
#define BACKENDS_AUDIOCD_DEFAULT_H

#include "backends/audiocd/audiocd.h"
#include "audio/mixer.h"

/**
 * The default audio cd manager. Implements emulation of audio cd playback.
 */
class DefaultAudioCDManager : public AudioCDManager {
public:
	DefaultAudioCDManager();
	virtual ~DefaultAudioCDManager() {}

	void play(int track, int numLoops, int startFrame, int duration, bool only_emulate = false);
	void stop();
	bool isPlaying() const;
	void setVolume(byte volume);
	void setBalance(int8 balance);
	void update();
	virtual Status getStatus() const; // Subclasses should override for better status results

	virtual bool openCD(int drive) { return false; }
	virtual void updateCD() {}
	virtual bool pollCD() const { return false; }
	virtual void playCD(int track, int num_loops, int start_frame, int duration) {}
	virtual void stopCD() {}

protected:
	Audio::SoundHandle _handle;
	bool _emulating;

	Status _cd;
	Audio::Mixer *_mixer;
};

#endif
