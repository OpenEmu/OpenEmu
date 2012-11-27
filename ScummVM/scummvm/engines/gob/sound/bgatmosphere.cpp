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

#include "common/array.h"

#include "gob/sound/bgatmosphere.h"
#include "gob/sound/sounddesc.h"

namespace Gob {

BackgroundAtmosphere::BackgroundAtmosphere(Audio::Mixer &mixer) :
	SoundMixer(mixer, Audio::Mixer::kMusicSoundType), _rnd("gobBA") {

	_playMode = kPlayModeLinear;
	_queuePos = -1;
	_shaded = false;
	_shadable = true;
}

BackgroundAtmosphere::~BackgroundAtmosphere() {
	queueClear();
}

void BackgroundAtmosphere::playBA() {
	Common::StackLock slock(_mutex);

	_queuePos = -1;
	getNextQueuePos();

	if (_queuePos == -1)
		return;

	SoundMixer::play(*_queue[_queuePos], 1, 0);
}

void BackgroundAtmosphere::stopBA() {
	SoundMixer::stop(0);
}

void BackgroundAtmosphere::setPlayMode(PlayMode mode) {
	_playMode = mode;
}

void BackgroundAtmosphere::queueSample(SoundDesc &sndDesc) {
	Common::StackLock slock(_mutex);

	_queue.push_back(&sndDesc);
}

void BackgroundAtmosphere::queueClear() {
	Common::StackLock slock(_mutex);

	SoundMixer::stop(0);
	for (uint i = 0; i < _queue.size(); i++)
		delete _queue[i];

	_queue.clear();
	_queuePos = -1;
}

void BackgroundAtmosphere::checkEndSample() {
	Common::StackLock slock(_mutex);

	getNextQueuePos();

	if (_queuePos == -1) {
		_end = true;
		_playingSound = 0;
	} else {
		SoundMixer::setSample(*_queue[_queuePos], 1, 0, 0);
		if (_shaded)
			_fadeVol = 20000;
	}
}

void BackgroundAtmosphere::getNextQueuePos() {
	if (_queue.size() == 0) {
		_queuePos = -1;
		return;
	}

	switch (_playMode) {

	case kPlayModeLinear:
		_queuePos = (_queuePos + 1) % _queue.size();
		break;

	case kPlayModeRandom:
		_queuePos = _rnd.getRandomNumber(_queue.size() - 1);
		break;

	}
}

void BackgroundAtmosphere::setShadable(bool shadable) {
	if (!shadable) {
		unshade();
		_shadable = false;
	} else
		_shadable = true;
}

void BackgroundAtmosphere::shade() {
	if (!_shadable)
		return;

	_shaded = true;
	_fadeVol = 32768;
}

void BackgroundAtmosphere::unshade() {
	if (!_shadable)
		return;

	_shaded = false;
	_fadeVol = 65536;
}

} // End of namespace Gob
