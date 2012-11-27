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

#include "gob/sound/pcspeaker.h"

namespace Gob {

PCSpeaker::PCSpeaker(Audio::Mixer &mixer) : _mixer(&mixer) {

	_stream = new Audio::PCSpeaker(_mixer->getOutputRate());
	_mixer->playStream(Audio::Mixer::kSFXSoundType,
			&_handle, _stream, -1, 50, 0, DisposeAfterUse::NO, true);
}

PCSpeaker::~PCSpeaker() {
	_mixer->stopHandle(_handle);
	delete _stream;
}

void PCSpeaker::speakerOn(int16 frequency, int32 length) {
	_stream->play(Audio::PCSpeaker::kWaveFormSquare, frequency, length);
}

void PCSpeaker::speakerOff() {
	_stream->stop();
}

void PCSpeaker::onUpdate(uint32 millis) {
	if (_stream->isPlaying())
		_stream->stop(millis);
}

} // End of namespace Gob
