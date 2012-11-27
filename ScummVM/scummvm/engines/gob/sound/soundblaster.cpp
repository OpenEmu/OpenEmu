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

#include "gob/sound/soundblaster.h"
#include "gob/sound/sounddesc.h"

namespace Gob {

SoundBlaster::SoundBlaster(Audio::Mixer &mixer) : SoundMixer(mixer, Audio::Mixer::kSFXSoundType) {
	_curSoundDesc = 0;

	_compositionSamples = 0;
	_compositionSampleCount = 0;
	_compositionPos = -1;

	_compositionRepCount = 0;
}

SoundBlaster::~SoundBlaster() {
}

void SoundBlaster::playSample(SoundDesc &sndDesc, int16 repCount,
		int16 frequency, int16 fadeLength) {
	SoundMixer::play(sndDesc, repCount, frequency, fadeLength);
}

void SoundBlaster::stopSound(int16 fadeLength, SoundDesc *sndDesc) {
	Common::StackLock slock(_mutex);

	if (sndDesc && (sndDesc != _curSoundDesc))
		return;

	_compositionRepCount = 0;

	if (fadeLength <= 0)
		_curSoundDesc = 0;

	SoundMixer::stop(fadeLength);
}

void SoundBlaster::stopComposition() {
	if (_compositionPos != -1) {
		stopSound(0);
		_compositionPos = -1;
	}
}

void SoundBlaster::endComposition() {
	_compositionPos = -1;
	_compositionRepCount = 0;
}

void SoundBlaster::nextCompositionPos() {
	int8 slot;

	while ((++_compositionPos < 50) &&
			((slot = _composition[_compositionPos]) != -1)) {
		if ((slot >= 0) && (slot < _compositionSampleCount)) {
			SoundDesc &sample = _compositionSamples[slot];
			if (!sample.empty() && (sample.getType() == SOUND_SND)) {
				setSample(sample, 1, 0, 0);
				return;
			}
		}
		if (_compositionPos == 49)
			_compositionPos = -1;
	}

	_compositionPos = -1;
}

void SoundBlaster::playComposition(const int16 *composition, int16 freqVal,
		SoundDesc *sndDescs, int8 sndCount) {

	_compositionSamples = sndDescs;
	_compositionSampleCount = sndCount;

	int i = -1;
	do {
		i++;
		_composition[i] = composition[i];
	} while ((i < 50) && (composition[i] != -1));

	_compositionPos = -1;
	nextCompositionPos();
}

void SoundBlaster::repeatComposition(int32 repCount) {
	_compositionRepCount = repCount;
}

void SoundBlaster::setSample(SoundDesc &sndDesc, int16 repCount, int16 frequency,
		int16 fadeLength) {

	_curSoundDesc = &sndDesc;
	SoundMixer::setSample(sndDesc, repCount, frequency, fadeLength);
}

void SoundBlaster::checkEndSample() {
	if (_compositionPos != -1) {
		nextCompositionPos();
		return;
	}

	if (_compositionRepCount != 0) {
		if (_compositionRepCount > 0)
			_compositionRepCount--;

		nextCompositionPos();
		if (_compositionPos != -1)
			return;
	}

	SoundMixer::checkEndSample();
}

void SoundBlaster::endFade() {
	if (_fadeVolStep > 0) {
		_compositionPos = -1;
		_curSoundDesc = 0;
	}

	SoundMixer::endFade();
}

} // End of namespace Gob
