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

#include "common/textconsole.h"

#include "gob/sound/infogrames.h"

namespace Gob {

Infogrames::Infogrames(Audio::Mixer &mixer) : _mixer(&mixer) {
	_instruments = 0;
	_song = 0;
}

Infogrames::~Infogrames() {
	clearSong();
	clearInstruments();
}

bool Infogrames::loadInstruments(const char *fileName) {
	clearSong();
	clearInstruments();

	return loadInst(fileName);
}

bool Infogrames::loadSong(const char *fileName) {
	clearSong();

	if (!_instruments)
		if (!loadInst("i1.ins"))
			return false;

	_song = new Audio::Infogrames(*_instruments, true,
			_mixer->getOutputRate(), _mixer->getOutputRate() / 75);

	if (!_song->load(fileName)) {
		warning("Infogrames: Couldn't load music \"%s\"", fileName);
		clearSong();
		return false;
	}

	return true;
}

void Infogrames::play() {
	if (_song && !_mixer->isSoundHandleActive(_handle)) {
		_song->restart();
		_mixer->playStream(Audio::Mixer::kMusicSoundType,
				&_handle, _song, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
	}
}

void Infogrames::stop() {
	_mixer->stopHandle(_handle);
}

void Infogrames::clearInstruments() {
	delete _instruments;
	_instruments = 0;
}

void Infogrames::clearSong() {
	if (_song) {
		_mixer->stopHandle(_handle);

		delete _song;
		_song = 0;
	}
}

bool Infogrames::loadInst(const char *fileName) {
	_instruments = new Audio::Infogrames::Instruments;
	if (!_instruments->load(fileName)) {
		warning("Infogrames: Couldn't load instruments \"%s\"", fileName);
		clearInstruments();
		return false;
	}

	return true;
}

} // End of namespace Gob
