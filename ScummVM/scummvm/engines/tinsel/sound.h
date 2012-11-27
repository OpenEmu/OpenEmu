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
 * This file contains the Sound Driver data structures etc.
 */

#ifndef TINSEL_SOUND_H
#define TINSEL_SOUND_H

#include "common/file.h"
#include "common/file.h"

#include "audio/mixer.h"

#include "tinsel/dw.h"
#include "tinsel/tinsel.h"
#include "tinsel/drives.h"

namespace Tinsel {

enum STYPE {FX, VOICE};

enum PlayPriority { PRIORITY_SCRIPT, PRIORITY_SPLAY1, PRIORITY_SPLAY2, PRIORITY_TALK };

enum SampleFlags {PS_COMPLETE = 0x01, PS_SUSTAIN = 0x02};

/*----------------------------------------------------------------------*\
|*				Function Prototypes			*|
\*----------------------------------------------------------------------*/

class SoundManager {
protected:
	static const int kNumSFX = 3; // Number of SFX channels
	enum {
		kChannelTalk = 0,
		kChannelTinsel1 = 0, // Always using this channel for DW1
		kChannelSFX = 1
	};
	static const int kNumChannels = kChannelSFX + kNumSFX;

	enum SoundMode {
		kVOCMode,
		kMP3Mode,
		kVorbisMode,
		kFLACMode
	};

	struct Channel {
		// Sample handle
		Audio::SoundHandle handle;

		// Sample id
		int sampleNum;
		int subSample;

		// Playing properties
		bool looped;
		int x, y;
		int priority;

		// Time properties
		uint32 timeStarted;
		uint32 timeDuration;
		uint32 lastStart;
	};

	Channel _channels[kNumChannels];

	//TinselEngine *_vm;	// TODO: Enable this once global _vm var is gone

	/** Sample index buffer and number of entries */
	uint32 *_sampleIndex;

	/** Number of entries in the sample index */
	int _sampleIndexLen;

	/** Specifies if the sample-data is compressed and if yes, how */
	SoundMode _soundMode;

	/** file stream for sample file */
	TinselFile _sampleStream;

	bool offscreenChecks(int x, int &y);
	int8 getPan(int x);

public:

	SoundManager(TinselEngine *vm);
	~SoundManager();

	bool playSample(int id, Audio::Mixer::SoundType type, Audio::SoundHandle *handle = 0);
	bool playSample(int id, int sub, bool bLooped, int x, int y, int priority,
			Audio::Mixer::SoundType type, Audio::SoundHandle *handle = 0);

	void stopAllSamples();                // Stops any currently playing sample
	void stopSpecSample(int id, int sub = 0); // Stops a specific sample

	void setSFXVolumes(uint8 volume);

	bool sampleExists(int id);
	bool sampleIsPlaying(int id = -1);

	// TODO: Internal method, make this protected?
	void openSampleFiles();
	void closeSampleStream();
};

} // End of namespace Tinsel

#endif	// TINSEL_SOUND_H
