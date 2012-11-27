/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/quicktime.h"
#include "common/file.h"
#include "common/system.h"

#include "pegasus/fader.h"
#include "pegasus/sound.h"

namespace Pegasus {

Sound::Sound() {
	_stream = 0;
	_volume = 0xFF;
	_fader = 0;
}

Sound::~Sound() {
	disposeSound();
}

void Sound::disposeSound() {
	stopSound();
	delete _stream; _stream = 0;
}

void Sound::initFromAIFFFile(const Common::String &fileName) {
	disposeSound();

	Common::File *file = new Common::File();
	if (!file->open(fileName)) {
		warning("Failed to open AIFF file '%s'", fileName.c_str());
		delete file;
		return;
	}

	_stream = Audio::makeAIFFStream(file, DisposeAfterUse::YES);
}

void Sound::initFromQuickTime(const Common::String &fileName) {
	disposeSound();

	_stream = Audio::makeQuickTimeStream(fileName);

	if (!_stream)
		warning("Failed to open QuickTime file '%s'", fileName.c_str());
}

void Sound::attachFader(SoundFader *fader) {
	if (_fader)
		_fader->attachSound(0);

	_fader = fader;

	if (_fader)
		_fader->attachSound(this);
}

void Sound::playSound() {
	if (!isSoundLoaded())
		return;

	stopSound();

	if (_fader)
		setVolume(_fader->getFaderValue());

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, _stream, -1, _volume, 0, DisposeAfterUse::NO);
}

void Sound::loopSound() {
	if (!isSoundLoaded())
		return;

	stopSound();

	// Create a looping stream
	Audio::AudioStream *loopStream = new Audio::LoopingAudioStream(_stream, 0, DisposeAfterUse::NO);

	// Assume that if there is a fader, we're going to fade the sound in.
	if (_fader)
		setVolume(0);

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, loopStream, -1, _volume, 0, DisposeAfterUse::YES);
}

void Sound::playSoundSegment(uint32 start, uint32 end) {
	if (!isSoundLoaded())
		return;

	stopSound();

	Audio::AudioStream *subStream = new Audio::SubSeekableAudioStream(_stream, Audio::Timestamp(0, start, 600), Audio::Timestamp(0, end, 600), DisposeAfterUse::NO);

	g_system->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &_handle, subStream, -1, _volume, 0, DisposeAfterUse::YES);
}

void Sound::stopSound() {
	g_system->getMixer()->stopHandle(_handle);
}

void Sound::setVolume(const uint16 volume) {
	// Clipping the volume to [0x00, 0xFF] instead of Apple's [0, 0x100]
	// We store the volume in case SetVolume is called before the sound starts

	_volume = (volume == 0x100) ? 0xFF : volume;
	g_system->getMixer()->setChannelVolume(_handle, _volume);
}

bool Sound::isPlaying() {
	return isSoundLoaded() && g_system->getMixer()->isSoundHandleActive(_handle);
}

bool Sound::isSoundLoaded() const {
	return _stream != 0;
}

SoundTimeBase::SoundTimeBase() {
	setScale(600);
	_startScale = 600;
	_stopScale = 600;
	_setToStart = false;
}

void SoundTimeBase::playSoundSegment(uint32 startTime, uint32 endTime) {
	_startTime = startTime;
	_stopTime = endTime;
	_setToStart = true;
	_time = Common::Rational(startTime, getScale());
	setRate(1);
	Sound::playSoundSegment(startTime, endTime);
}

void SoundTimeBase::updateTime() {
	if (_setToStart) {
		if (isPlaying()) {
			// Not at the end, let's get the time
			uint numFrames = g_system->getMixer()->getSoundElapsedTime(_handle) * 600 / 1000;

			// WORKAROUND: Our mixer is woefully inaccurate and quite often returns
			// times that exceed the actual length of the clip. We'll just fake times
			// that are under the final time to ensure any trigger for the end time is
			// only sent when the sound has actually stopped.
			if (numFrames >= (_stopTime - _startTime))
				numFrames = _stopTime - _startTime - 1;

			_time = Common::Rational(_startTime + numFrames, getScale());
		} else {
			// Assume we reached the end
			_setToStart = false;
			_time = Common::Rational(_stopTime, getScale());
		}
	}
}

} // End of namespace Pegasus
