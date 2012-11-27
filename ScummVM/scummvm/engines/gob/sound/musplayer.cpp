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
#include "common/textconsole.h"

#include "gob/sound/musplayer.h"

namespace Gob {

MUSPlayer::MUSPlayer(Audio::Mixer &mixer) : AdLib(mixer),
	_songData(0), _songDataSize(0), _playPos(0), _songID(0) {

}

MUSPlayer::~MUSPlayer() {
	unload();
}

void MUSPlayer::unload() {
	stopPlay();

	unloadSND();
	unloadMUS();
}

uint32 MUSPlayer::getSampleDelay(uint16 delay) const {
	if (delay == 0)
		return 0;

	uint32 freq = (_ticksPerBeat * _tempo) / 60;

	return ((uint32)delay * getSamplesPerSecond()) / freq;
}

void MUSPlayer::skipToTiming() {
	while (*_playPos < 0x80)
		_playPos++;

	if (*_playPos != 0xF8)
		_playPos--;
}

uint32 MUSPlayer::pollMusic(bool first) {
	if (_timbres.empty() || !_songData || !_playPos || (_playPos >= (_songData + _songDataSize))) {
		end();
		return 0;
	}

	if (first)
		return getSampleDelay(*_playPos++);

	uint16 delay = 0;
	while (delay == 0) {
		byte cmd = *_playPos;

		// Delay overflow
		if (cmd == 0xF8) {
			_playPos++;
			delay = 0xF8;
			break;
		}

		// Song end marker
		if (cmd == 0xFC) {
			end();
			return 0;
		}

		// Global command
		if (cmd == 0xF0) {
			_playPos++;

			byte type1 = *_playPos++;
			byte type2 = *_playPos++;

			if ((type1 == 0x7F) && (type2 == 0)) {
				// Tempo change, as a fraction of the base tempo

				uint32 num   = *_playPos++;
				uint32 denom = *_playPos++;

				_tempo = _baseTempo * num + ((_baseTempo * denom) >> 7);

				_playPos++;
			} else {

				// Unsupported global command, skip it
				_playPos -= 2;
				while(*_playPos++ != 0xF7)
					;
			}

			delay = *_playPos++;
			break;
		}

		// Voice command

		if (cmd >= 0x80) {
			_playPos++;

			_lastCommand = cmd;
		} else
			cmd = _lastCommand;

		uint8 voice = cmd & 0x0F;
		uint8 note, volume;
		uint16 pitch;

		switch (cmd & 0xF0) {
		case 0x80: // Note off
			_playPos += 2;
			noteOff(voice);
			break;

		case 0x90: // Note on
			note   = *_playPos++;
			volume = *_playPos++;

			if (volume) {
				setVoiceVolume(voice, volume);
				noteOn(voice, note);
			} else
				noteOff(voice);
			break;

		case 0xA0: // Set volume
			setVoiceVolume(voice, *_playPos++);
			break;

		case 0xB0:
			_playPos += 2;
			break;

		case 0xC0: // Set instrument
			setInstrument(voice, *_playPos++);
			break;

		case 0xD0:
			_playPos++;
			break;

		case 0xE0: // Pitch bend
			pitch  = *_playPos++;
			pitch += *_playPos++ << 7;
			bendVoicePitch(voice, pitch);
			break;

		default:
			warning("MUSPlayer: Unsupported command: 0x%02X", cmd);
			skipToTiming();
			break;
		}

		delay = *_playPos++;
	}

	if (delay == 0xF8) {
		delay = 240;

		if (*_playPos != 0xF8)
			delay += *_playPos++;
	}

	return getSampleDelay(delay);
}

void MUSPlayer::rewind() {
	_playPos = _songData;
	_tempo   = _baseTempo;

	_lastCommand = 0;

	setPercussionMode(_soundMode != 0);
	setPitchRange(_pitchBendRange);
}

bool MUSPlayer::loadSND(Common::SeekableReadStream &snd) {
	unloadSND();

	int timbreCount, timbrePos;
	if (!readSNDHeader(snd, timbreCount, timbrePos))
		return false;

	if (!readSNDTimbres(snd, timbreCount, timbrePos) || snd.err()) {
		unloadSND();
		return false;
	}

	return true;
}

bool MUSPlayer::readString(Common::SeekableReadStream &stream, Common::String &string, byte *buffer, uint size) {
	if (stream.read(buffer, size) != size)
		return false;

	buffer[size] = '\0';

	string = (char *) buffer;

	return true;
}

bool MUSPlayer::readSNDHeader(Common::SeekableReadStream &snd, int &timbreCount, int &timbrePos) {
	// Sanity check
	if (snd.size() <= 6) {
		warning("MUSPlayer::readSNDHeader(): File too small (%d)", snd.size());
		return false;
	}

	// Version
	const uint8 versionMajor = snd.readByte();
	const uint8 versionMinor = snd.readByte();

	if ((versionMajor != 1) && (versionMinor != 0)) {
		warning("MUSPlayer::readSNDHeader(): Unsupported version %d.%d", versionMajor, versionMinor);
		return false;
	}

	// Number of timbres and where they start
	timbreCount = snd.readUint16LE();
	timbrePos   = snd.readUint16LE();

	const uint16 minTimbrePos = 6 + timbreCount * 9;

	// Sanity check
	if (timbrePos < minTimbrePos) {
		warning("MUSPlayer::readSNDHeader(): Timbre offset too small: %d < %d", timbrePos, minTimbrePos);
		return false;
	}

	const uint32 timbreParametersSize = snd.size() - timbrePos;
	const uint32 paramSize            = kOperatorsPerVoice * kParamCount * sizeof(uint16);

	// Sanity check
	if (timbreParametersSize != (timbreCount * paramSize)) {
		warning("MUSPlayer::loadSND(): Timbre parameters size mismatch: %d != %d",
		        timbreParametersSize, timbreCount * paramSize);
		return false;
	}

	return true;
}

bool MUSPlayer::readSNDTimbres(Common::SeekableReadStream &snd, int timbreCount, int timbrePos) {
	_timbres.resize(timbreCount);

	// Read names
	byte nameBuffer[10];
	for (Common::Array<Timbre>::iterator t = _timbres.begin(); t != _timbres.end(); ++t) {
		if (!readString(snd, t->name, nameBuffer, 9)) {
			warning("MUSPlayer::readMUSTimbres(): Failed to read timbre name");
			return false;
		}
	}

	if (!snd.seek(timbrePos)) {
		warning("MUSPlayer::readMUSTimbres(): Failed to seek to timbres");
		return false;
	}

	// Read parameters
	for (Common::Array<Timbre>::iterator t = _timbres.begin(); t != _timbres.end(); ++t) {
		for (int i = 0; i < (kOperatorsPerVoice * kParamCount); i++)
			t->params[i] = snd.readUint16LE();
	}

	return true;
}

bool MUSPlayer::loadMUS(Common::SeekableReadStream &mus) {
	unloadMUS();

	if (!readMUSHeader(mus) || !readMUSSong(mus) || mus.err()) {
		unloadMUS();
		return false;
	}

	rewind();

	return true;
}

bool MUSPlayer::readMUSHeader(Common::SeekableReadStream &mus) {
	// Sanity check
	if (mus.size() <= 6)
		return false;

	// Version
	const uint8 versionMajor = mus.readByte();
	const uint8 versionMinor = mus.readByte();

	if ((versionMajor != 1) && (versionMinor != 0)) {
		warning("MUSPlayer::readMUSHeader(): Unsupported version %d.%d", versionMajor, versionMinor);
		return false;
	}

	_songID = mus.readUint32LE();

	byte nameBuffer[31];
	if (!readString(mus, _songName, nameBuffer, 30)) {
		warning("MUSPlayer::readMUSHeader(): Failed to read the song name");
		return false;
	}

	_ticksPerBeat    = mus.readByte();
	_beatsPerMeasure = mus.readByte();

	mus.skip(4); // Length of song in ticks

	_songDataSize = mus.readUint32LE();

	mus.skip(4); // Number of commands
	mus.skip(8); // Unused

	_soundMode      = mus.readByte();
	_pitchBendRange = mus.readByte();
	_baseTempo      = mus.readUint16LE();

	mus.skip(8); // Unused

	return true;
}

bool MUSPlayer::readMUSSong(Common::SeekableReadStream &mus) {
	const uint32 realSongDataSize = mus.size() - mus.pos();

	if (realSongDataSize < _songDataSize) {
		warning("MUSPlayer::readMUSSong(): File too small for the song data: %d < %d", realSongDataSize, _songDataSize);
		return false;
	}

	_songData = new byte[_songDataSize];

	if (mus.read(_songData, _songDataSize) != _songDataSize) {
		warning("MUSPlayer::readMUSSong(): Read failed");
		return false;
	}

	return true;
}

void MUSPlayer::unloadSND() {
	_timbres.clear();
}

void MUSPlayer::unloadMUS() {
	delete[] _songData;

	_songData     = 0;
	_songDataSize = 0;

	_playPos = 0;
}

uint32 MUSPlayer::getSongID() const {
	return _songID;
}

const Common::String &MUSPlayer::getSongName() const {
	return _songName;
}

void MUSPlayer::setInstrument(uint8 voice, uint8 instrument) {
	if (instrument >= _timbres.size())
		return;

	setVoiceTimbre(voice, _timbres[instrument].params);
}

} // End of namespace Gob
