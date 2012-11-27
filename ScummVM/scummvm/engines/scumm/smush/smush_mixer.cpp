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


#include "common/util.h"

#include "scumm/smush/smush_mixer.h"
#include "scumm/smush/channel.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/imuse/imuse.h"

#include "audio/mixer.h"
#include "audio/decoders/raw.h"


namespace Scumm {

SmushMixer::SmushMixer(Audio::Mixer *m) :
	_mixer(m),
	_soundFrequency(22050) {
	for (int32 i = 0; i < NUM_CHANNELS; i++) {
		_channels[i].id = -1;
		_channels[i].chan = NULL;
		_channels[i].stream = NULL;
	}
}

SmushMixer::~SmushMixer() {
	for (int32 i = 0; i < NUM_CHANNELS; i++) {
		_mixer->stopHandle(_channels[i].handle);
	}
}

SmushChannel *SmushMixer::findChannel(int32 track) {
	debugC(DEBUG_SMUSH, "SmushMixer::findChannel(%d)", track);
	for (int32 i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i].id == track)
			return _channels[i].chan;
	}
	return NULL;
}

void SmushMixer::addChannel(SmushChannel *c) {
	int32 track = c->getTrackIdentifier();
	int i;

	debugC(DEBUG_SMUSH, "SmushMixer::addChannel(%d)", track);

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i].id == track)
			debugC(DEBUG_SMUSH, "SmushMixer::addChannel(%d): channel already exists", track);
	}

	for (i = 0; i < NUM_CHANNELS; i++) {
		if ((_channels[i].chan == NULL || _channels[i].id == -1) && !_mixer->isSoundHandleActive(_channels[i].handle)) {
			_channels[i].chan = c;
			_channels[i].id = track;
			return;
		}
	}

	for (i = 0; i < NUM_CHANNELS; i++) {
		debugC(DEBUG_SMUSH, "channel %d : %p(%d, %d)", i, (void *)_channels[i].chan,
			_channels[i].chan ? _channels[i].chan->getTrackIdentifier() : -1,
			_channels[i].chan ? _channels[i].chan->isTerminated() : 1);
	}

	error("SmushMixer::addChannel(%d): no channel available", track);
}

bool SmushMixer::handleFrame() {
	debugC(DEBUG_SMUSH, "SmushMixer::handleFrame()");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i].id != -1) {
			if (_channels[i].chan->isTerminated()) {
				delete _channels[i].chan;
				_channels[i].id = -1;
				_channels[i].chan = NULL;
				if (_channels[i].stream) {
					_channels[i].stream->finish();
					_channels[i].stream = 0;
				}
			} else {
				int32 vol, pan;
				bool stereo, is_16bit;

				_channels[i].chan->getParameters(stereo, is_16bit, vol, pan);

				// Grab the audio data from the channel
				int32 size = _channels[i].chan->getAvailableSoundDataSize();
				byte *data = _channels[i].chan->getSoundData();

				byte flags = stereo ? Audio::FLAG_STEREO : 0;
				if (is_16bit) {
					flags |= Audio::FLAG_16BITS;
				} else {
					flags |= Audio::FLAG_UNSIGNED;
				}

				if (_mixer->isReady()) {
					// Stream the data
					if (!_channels[i].stream) {
						_channels[i].stream = Audio::makeQueuingAudioStream(_channels[i].chan->getRate(), stereo);
						_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[i].handle, _channels[i].stream);
					}
					_mixer->setChannelVolume(_channels[i].handle, vol);
					_mixer->setChannelBalance(_channels[i].handle, pan);
					_channels[i].stream->queueBuffer(data, size, DisposeAfterUse::YES, flags);	// The stream will free the buffer for us
				} else
					delete[] data;
			}
		}
	}
	return true;
}

bool SmushMixer::stop() {
	debugC(DEBUG_SMUSH, "SmushMixer::stop()");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i].id != -1) {
			delete _channels[i].chan;
			_channels[i].id = -1;
			_channels[i].chan = NULL;
			if (_channels[i].stream) {
				_channels[i].stream->finish();
				_channels[i].stream = NULL;
			}
		}
	}

	return true;
}

bool SmushMixer::flush() {
	debugC(DEBUG_SMUSH, "SmushMixer::flush()");
	for (int i = 0; i < NUM_CHANNELS; i++) {
		if (_channels[i].id != -1) {
			if (_channels[i].stream->endOfStream()) {
				_mixer->stopHandle(_channels[i].handle);
				delete _channels[i].chan;
				_channels[i].id = -1;
				_channels[i].chan = NULL;
				_channels[i].stream = NULL;
			}
		}
	}

	return true;
}

} // End of namespace Scumm
