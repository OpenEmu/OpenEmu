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

#include "common/debug.h"

#include "groovie/player.h"
#include "groovie/groovie.h"

namespace Groovie {

VideoPlayer::VideoPlayer(GroovieEngine *vm) :
	_vm(vm), _syst(vm->_system), _file(NULL), _audioStream(NULL), _fps(0), _overrideSpeed(false) {
}

bool VideoPlayer::load(Common::SeekableReadStream *file, uint16 flags) {
	_file = file;
	_flags = flags;
	_overrideSpeed = false;
	_audioStream = NULL;

	_fps = loadInternal();

	if (_fps != 0) {
		setOverrideSpeed(_overrideSpeed);
		_begunPlaying = false;
		return true;
	} else {
		_file = NULL;
		return false;
	}
}

void VideoPlayer::setOverrideSpeed(bool isOverride) {
	_overrideSpeed = isOverride;
	if (_fps != 0) {
		if (isOverride)
			_millisBetweenFrames = 1000 / 26;
		else
			_millisBetweenFrames = 1000 / _fps;
	}
}

bool VideoPlayer::playFrame() {
	bool end = true;

	// Process the next frame while the file is open
	if (_file) {
		end = playFrameInternal();
	}

	// The file has been completely processed
	if (end) {
		_file = NULL;

		// Wait for pending audio
		if (_audioStream) {
			if (_audioStream->endOfData()) {
				// Mark the audio stream as finished (no more data will be appended)
				_audioStream->finish();
			} else {
				// Don't end if there's still audio playing
				end = false;
			}
		}
	}

	return end;
}

void VideoPlayer::waitFrame() {
	uint32 currTime = _syst->getMillis();
	if (!_begunPlaying) {
		_begunPlaying = true;
		_lastFrameTime = currTime;
	} else {
		uint32 millisDiff = currTime - _lastFrameTime;
		if (millisDiff < _millisBetweenFrames) {
			debugC(7, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::Player: Delaying %d (currTime=%d, _lastFrameTime=%d, millisDiff=%d, _millisBetweenFrame=%d)",
					_millisBetweenFrames - millisDiff, currTime, _lastFrameTime, millisDiff, _millisBetweenFrames);
			_syst->delayMillis(_millisBetweenFrames - millisDiff);
			currTime = _syst->getMillis();
			debugC(7, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::Player: Finished delay at %d", currTime);
		}
		debugC(6, kGroovieDebugVideo | kGroovieDebugAll, "Groovie::Player: Frame displayed at %d (%f FPS)", currTime, 1000.0 / (currTime - _lastFrameTime));
		_lastFrameTime = currTime;
	}
}

} // End of Groovie namespace
