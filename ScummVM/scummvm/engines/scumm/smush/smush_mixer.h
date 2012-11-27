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

#ifndef SCUMM_SMUSH_MIXER_H
#define SCUMM_SMUSH_MIXER_H


#include "audio/mixer.h"
#include "common/mutex.h"
#include "scumm/sound.h"

namespace Scumm {

class SmushChannel;

class SmushMixer {
	enum {
		NUM_CHANNELS = 16
	};
private:

	Audio::Mixer *_mixer;
	struct channels {
		int id;
		SmushChannel *chan;
		Audio::SoundHandle handle;
		Audio::QueuingAudioStream *stream;
	} _channels[NUM_CHANNELS];

	int _soundFrequency;

	Common::Mutex _mutex;

public:

	SmushMixer(Audio::Mixer *);
	virtual ~SmushMixer();
	SmushChannel *findChannel(int32 track);
	void addChannel(SmushChannel *c);
	bool handleFrame();
	bool stop();
	bool flush();
	bool update();
};

} // End of namespace Scumm

#endif
