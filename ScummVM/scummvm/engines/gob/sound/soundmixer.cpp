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

#include "gob/sound/soundmixer.h"
#include "gob/sound/sounddesc.h"

#include "audio/decoders/raw.h"

namespace Gob {

SoundMixer::SoundMixer(Audio::Mixer &mixer, Audio::Mixer::SoundType type) : _mixer(&mixer) {
	_playingSound = 0;

	_rate = _mixer->getOutputRate();
	_end = true;
	_data = 0;
	_length = 0;
	_freq = 0;
	_repCount = 0;

	_offset = 0;
	_offsetFrac = 0;
	_offsetInc = 0;

	_cur = 0;
	_last = 0;

	_fade = false;
	_fadeVol = 65536;
	_fadeVolStep = 0;
	_fadeSamples = 0;
	_curFadeSamples = 0;

	_mixer->playStream(type, &_handle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

SoundMixer::~SoundMixer() {
	_mixer->stopHandle(_handle);
}

inline int16 SoundMixer::getData(int offset) {
	if (!_16bit)
		return (int16) ((int8) _data[offset]);
	else
		return (int16) READ_LE_UINT16(_data + (offset * 2));
}

bool SoundMixer::isPlaying() const {
	return !_end;
}

char SoundMixer::getPlayingSound() const {
	return _playingSound;
}

void SoundMixer::stop(int16 fadeLength) {
	Common::StackLock slock(_mutex);

	if (fadeLength <= 0) {
		_data = 0;
		_end = true;
		_playingSound = 0;
		return;
	}

	_fade = true;
	_fadeVol = 65536;
	_fadeSamples = (int) (fadeLength * (((double) _rate) / 10.0));
	_fadeVolStep = MAX((int32) 1, (int32) (65536 / _fadeSamples));
	_curFadeSamples = 0;
}

void SoundMixer::setRepeating(int32 repCount) {
	Common::StackLock slock(_mutex);

	_repCount = repCount;
}

void SoundMixer::setSample(SoundDesc &sndDesc, int16 repCount, int16 frequency,
		int16 fadeLength) {

	if (frequency <= 0)
		frequency = sndDesc._frequency;

	sndDesc._repCount = repCount - 1;
	sndDesc._frequency = frequency;

	_16bit = (sndDesc._mixerFlags & Audio::FLAG_16BITS) != 0;

	_data = sndDesc.getData();

	_length = sndDesc.size();
	_freq = frequency;

	_repCount = repCount;
	_end = false;
	_playingSound = 1;

	_offset = 0;
	_offsetFrac = 0;
	_offsetInc = (_freq << FRAC_BITS) / _rate;

	_last = _cur;
	_cur = getData(0);

	_curFadeSamples = 0;
	if (fadeLength == 0) {
		_fade = false;
		_fadeVol = 65536;
		_fadeSamples = 0;
		_fadeVolStep = 0;
	} else {
		_fade = true;
		_fadeVol = 0;
		_fadeSamples = (int) (fadeLength * (((double) _rate) / 10.0));
		_fadeVolStep = - MAX((int32) 1, (int32) (65536 / _fadeSamples));
	}
}

void SoundMixer::play(SoundDesc &sndDesc, int16 repCount, int16 frequency,
		int16 fadeLength) {
	Common::StackLock slock(_mutex);

	if (!_end)
		return;

	setSample(sndDesc, repCount, frequency, fadeLength);
}

void SoundMixer::checkEndSample() {
	if ((_repCount == -1) || (--_repCount > 0)) {
		_offset = 0;
		_offsetFrac = 0;
		_end = false;
		_playingSound = 1;
	} else {
		_end = true;
		_playingSound = 0;
	}
}

int SoundMixer::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock slock(_mutex);

	for (int i = 0; i < numSamples; i++) {
		if (!_data)
			return i;
		if (_end || (_offset >= _length))
			checkEndSample();
		if (_end)
			return i;

		// Linear interpolation. See sound/rate.cpp

		int16 val = (_last + (((_cur - _last) * _offsetFrac +
					FRAC_HALF) >> FRAC_BITS)) << (_16bit ? 0 : 8);
		*buffer++ = (val * _fadeVol) >> 16;

		_offsetFrac += _offsetInc;

		// Was there an integral change?
		if (fracToInt(_offsetFrac) > 0) {
			_last = _cur;
			_cur = getData(_offset);
			_offset += fracToInt(_offsetFrac);
			_offsetFrac &= FRAC_LO_MASK;
		}

		if (_fade) {

			if (++_curFadeSamples >= _fadeSamples)
				endFade();
			else
				_fadeVol -= _fadeVolStep;

			if (_fadeVol < 0)
				_fadeVol = 0;

		}
	}
	return numSamples;
}

void SoundMixer::endFade() {
	if (_fadeVolStep > 0) {
		_data = 0;
		_end = true;
		_playingSound = 0;
	} else {
		_fadeVol = 65536;
		_fade = false;
	}
}

} // End of namespace Gob
