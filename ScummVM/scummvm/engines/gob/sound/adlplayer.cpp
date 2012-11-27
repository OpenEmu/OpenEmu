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

#include "common/stream.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "gob/sound/adlplayer.h"

namespace Gob {

ADLPlayer::ADLPlayer(Audio::Mixer &mixer) : AdLib(mixer),
	_songData(0), _songDataSize(0), _playPos(0) {

}

ADLPlayer::~ADLPlayer() {
	unload();
}

void ADLPlayer::unload() {
	stopPlay();

	_timbres.clear();

	delete[] _songData;

	_songData     = 0;
	_songDataSize = 0;

	_playPos = 0;
}

uint32 ADLPlayer::pollMusic(bool first) {
	if (_timbres.empty() || !_songData || !_playPos || (_playPos >= (_songData + _songDataSize))) {
		end();
		return 0;
	}

	// We'll ignore the first delay
	if (first)
		_playPos += (*_playPos & 0x80) ? 2 : 1;

	byte cmd = *_playPos++;

	// Song end marker
	if (cmd == 0xFF) {
		end();
		return 0;
	}

	// Set the instrument that should be modified
	if (cmd == 0xFE)
		_modifyInstrument = *_playPos++;

	if (cmd >= 0xD0) {
		// Modify an instrument

		if      (_modifyInstrument == 0xFF)
			warning("ADLPlayer: No instrument to modify");
		else if (_modifyInstrument >= _timbres.size())
			warning("ADLPlayer: Can't modify invalid instrument %d (%d)", _modifyInstrument, _timbres.size());
		else
			_timbres[_modifyInstrument].params[_playPos[0]] = _playPos[1];

		_playPos += 2;

		// If we currently have that instrument loaded, reload it
		for (int i = 0; i < kMaxVoiceCount; i++)
			if (_currentInstruments[i] == _modifyInstrument)
				setInstrument(i, _modifyInstrument);
	} else {
		// Voice command

		uint8 voice = cmd & 0x0F;
		uint8 note, volume;

		switch (cmd & 0xF0) {
		case 0x00: // Note on with volume
			note   = *_playPos++;
			volume = *_playPos++;

			setVoiceVolume(voice, volume);
			noteOn(voice, note);
			break;

		case 0xA0: // Pitch bend
			bendVoicePitch(voice, ((uint16)*_playPos++) << 7);
			break;

		case 0xB0: // Set volume
			setVoiceVolume(voice, *_playPos++);
			break;

		case 0xC0: // Set instrument
			setInstrument(voice, *_playPos++);
			break;

		case 0x90: // Note on
			noteOn(voice, *_playPos++);
			break;

		case 0x80: // Note off
			noteOff(voice);
			break;

		default:
			warning("ADLPlayer: Unsupported command: 0x%02X. Stopping playback.", cmd);
			end(true);
			return 0;
		}
	}

	uint16 delay = *_playPos++;

	if (delay & 0x80)
		delay = ((delay & 3) << 8) | *_playPos++;

	return getSampleDelay(delay);
}

uint32 ADLPlayer::getSampleDelay(uint16 delay) const {
	if (delay == 0)
		return 0;

	return ((uint32)delay * getSamplesPerSecond()) / 1000;
}

void ADLPlayer::rewind() {
	// Reset song data
	_playPos = _songData;

	// Set melody/percussion mode
	setPercussionMode(_soundMode != 0);

	// Reset instruments
	for (Common::Array<Timbre>::iterator t = _timbres.begin(); t != _timbres.end(); ++t)
		memcpy(t->params, t->startParams, kOperatorsPerVoice * kParamCount * sizeof(uint16));

	for (int i = 0; i < kMaxVoiceCount; i++)
		_currentInstruments[i] = 0;

	// Reset voices
	int numVoice = MIN<int>(_timbres.size(), _soundMode ? (int)kMaxVoiceCount : (int)kMelodyVoiceCount);
	for (int i = 0; i < numVoice; i++) {
		setInstrument(i, _currentInstruments[i]);
		setVoiceVolume(i, kMaxVolume);
	}

	_modifyInstrument = 0xFF;
}

bool ADLPlayer::load(Common::SeekableReadStream &adl) {
	unload();

	int timbreCount;
	if (!readHeader(adl, timbreCount)) {
		unload();
		return false;
	}

	if (!readTimbres(adl, timbreCount) || !readSongData(adl) || adl.err()) {
		unload();
		return false;
	}

	rewind();

	return true;
}

bool ADLPlayer::readHeader(Common::SeekableReadStream &adl, int &timbreCount) {
	// Sanity check
	if (adl.size() < 60) {
		warning("ADLPlayer::readHeader(): File too small (%d)", adl.size());
		return false;
	}

	_soundMode  = adl.readByte();
	timbreCount = adl.readByte() + 1;

	adl.skip(1);

	return true;
}

bool ADLPlayer::readTimbres(Common::SeekableReadStream &adl, int timbreCount) {
	_timbres.resize(timbreCount);
	for (Common::Array<Timbre>::iterator t = _timbres.begin(); t != _timbres.end(); ++t) {
		for (int i = 0; i < (kOperatorsPerVoice * kParamCount); i++)
			t->startParams[i] = adl.readUint16LE();
	}

	if (adl.err()) {
		warning("ADLPlayer::readTimbres(): Read failed");
		return false;
	}

	return true;
}

bool ADLPlayer::readSongData(Common::SeekableReadStream &adl) {
	_songDataSize = adl.size() - adl.pos();
	_songData     = new byte[_songDataSize];

	if (adl.read(_songData, _songDataSize) != _songDataSize) {
		warning("ADLPlayer::readSongData(): Read failed");
		return false;
	}

	return true;
}

bool ADLPlayer::load(const byte *data, uint32 dataSize, int index) {
	unload();

	Common::MemoryReadStream stream(data, dataSize);
	if (!load(stream))
		return false;

	_index = index;
	return true;
}

void ADLPlayer::setInstrument(int voice, int instrument) {
	if ((voice >= kMaxVoiceCount) || ((uint)instrument >= _timbres.size()))
		return;

	_currentInstruments[voice] = instrument;

	setVoiceTimbre(voice, _timbres[instrument].params);
}

int ADLPlayer::getIndex() const {
	return _index;
}

} // End of namespace Gob
