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

#include "common/file.h"

#include "audio/mods/protracker.h"

#include "gob/sound/protracker.h"

namespace Gob {

Protracker::Protracker(Audio::Mixer &mixer) : _mixer(&mixer) {
	_protrackerStream = 0;
}

Protracker::~Protracker() {
	stop();
}

bool Protracker::play(const char *fileName) {
	stop();

	Common::File f;

	if (!f.open(fileName))
		return false;

	_protrackerStream = Audio::makeProtrackerStream(&f);

	if (!_protrackerStream)
		return false;

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_handle,
			_protrackerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);

	return true;
}

void Protracker::stop() {
	if (_protrackerStream) {
		_mixer->stopHandle(_handle);

		delete _protrackerStream;
		_protrackerStream = 0;
	}
}

} // End of namespace Gob
