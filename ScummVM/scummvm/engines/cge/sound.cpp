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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/sound.h"
#include "cge/text.h"
#include "cge/cge_main.h"
#include "common/config-manager.h"
#include "common/memstream.h"
#include "audio/decoders/raw.h"
#include "audio/audiostream.h"

namespace CGE {

DataCk::DataCk(byte *buf, int bufSize) {
	_buf = buf;
	_ckSize = bufSize;
}

DataCk::~DataCk() {
	free(_buf);
}

Sound::Sound(CGEEngine *vm) : _vm(vm) {
	_audioStream = NULL;
	_soundRepeatCount = 1;
	open();
}

Sound::~Sound() {
	close();
}

void Sound::close() {
	_vm->_midiPlayer->killMidi();
}

void Sound::open() {
	setRepeat(1);
	play((*_vm->_fx)[30000], 8);
}

void Sound::setRepeat(int16 count) {
	_soundRepeatCount = count;
}

int16 Sound::getRepeat() {
	return _soundRepeatCount;
}

void Sound::play(DataCk *wav, int pan) {
	if (wav) {
		stop();
		_smpinf._saddr = &*(wav->addr());
		_smpinf._slen = (uint16)wav->size();
		_smpinf._span = pan;
		_smpinf._counter = getRepeat();
		sndDigiStart(&_smpinf);
	}
}

void Sound::sndDigiStart(SmpInfo *PSmpInfo) {
	// Create an audio stream wrapper for sound
	Common::MemoryReadStream *stream = new Common::MemoryReadStream(PSmpInfo->_saddr,
		PSmpInfo->_slen, DisposeAfterUse::NO);
	_audioStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	// Start the new sound
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle,
		Audio::makeLoopingAudioStream(_audioStream, (uint)PSmpInfo->_counter));

	// CGE pan:
	// 8 = Center
	// Less = Left
	// More = Right
	_vm->_mixer->setChannelBalance(_soundHandle, (int8)CLIP(((PSmpInfo->_span - 8) * 16), -127, 127));
}

void Sound::stop() {
	sndDigiStop(&_smpinf);
}

void Sound::sndDigiStop(SmpInfo *PSmpInfo) {
	if (_vm->_mixer->isSoundHandleActive(_soundHandle))
		_vm->_mixer->stopHandle(_soundHandle);
	_audioStream = NULL;
}

Fx::Fx(CGEEngine *vm, int size) : _current(NULL), _vm(vm) {
	_cache = new Handler[size];
	for (_size = 0; _size < size; _size++) {
		_cache[_size]._ref = 0;
		_cache[_size]._wav = NULL;
	}
}

Fx::~Fx() {
	clear();
	delete[] _cache;
}

void Fx::clear() {
	for (Handler *p = _cache, *q = p + _size; p < q; p++) {
		if (p->_ref) {
			p->_ref = 0;
			delete p->_wav;
			p->_wav = NULL;
		}
	}
	_current = NULL;
}

int Fx::find(int ref) {
	int i = 0;
	for (Handler *p = _cache, *q = p + _size; p < q; p++) {
		if (p->_ref == ref)
			break;
		else
			++i;
	}
	return i;
}

void Fx::preload(int ref0) {
	Handler *cacheLim = _cache + _size;
	char filename[12];

	for (int ref = ref0; ref < ref0 + 10; ref++) {
		sprintf(filename, "FX%05d.WAV", ref);
		EncryptedStream file(_vm, filename);
		DataCk *wav = loadWave(&file);
		if (wav) {
			Handler *p = &_cache[find(0)];
			if (p >= cacheLim) {
				delete wav;
				break;
			}
			delete p->_wav;
			p->_wav = wav;
			p->_ref = ref;
		} else {
			warning("Unable to load %s", filename);
		}
	}
}

DataCk *Fx::load(int idx, int ref) {
	char filename[12];
	sprintf(filename, "FX%05d.WAV", ref);

	EncryptedStream file(_vm, filename);
	DataCk *wav = loadWave(&file);
	if (wav) {
		Handler *p = &_cache[idx];
		delete p->_wav;
		p->_wav = wav;
		p->_ref = ref;
	} else {
		warning("Unable to load %s", filename);
	}
	return wav;
}

DataCk *Fx::loadWave(EncryptedStream *file) {
	byte *data = (byte *)malloc(file->size());
	file->read(data, file->size());

	return new DataCk(data, file->size());
}

DataCk *Fx::operator[](int ref) {
	int i;
	if ((i = find(ref)) < _size)
		_current = _cache[i]._wav;
	else {
		if ((i = find(0)) >= _size) {
			clear();
			i = 0;
		}
		_current = load(i, ref);
	}
	return _current;
}

MusicPlayer::MusicPlayer(CGEEngine *vm) : _vm(vm) {
	_data = NULL;
	_isGM = false;

	MidiPlayer::createDriver();

	int ret = _driver->open();
	if (ret == 0) {
		if (_nativeMT32)
			_driver->sendMT32Reset();
		else
			_driver->sendGMReset();

		// TODO: Load cmf.ins with the instrument table.  It seems that an
		// interface for such an operation is supported for AdLib.  Maybe for
		// this card, setting instruments is necessary.

		_driver->setTimerCallback(this, &timerCallback);
	}
}

MusicPlayer::~MusicPlayer() {
	killMidi();
}

void MusicPlayer::killMidi() {
	Audio::MidiPlayer::stop();

	free(_data);
	_data = NULL;
}

void MusicPlayer::loadMidi(int ref) {
	// Work out the filename and check the given MIDI file exists
	Common::String filename = Common::String::format("%.2d.MID", ref);
	if (!_vm->_resman->exist(filename.c_str()))
		return;

	// Stop any currently playing MIDI file
	killMidi();

	// Read in the data for the file
	EncryptedStream mid(_vm, filename.c_str());
	_dataSize = mid.size();
	_data = (byte *)malloc(_dataSize);
	mid.read(_data, _dataSize);

	// Start playing the music
	sndMidiStart();
}

void MusicPlayer::sndMidiStart() {
	_isGM = true;

	MidiParser *parser = MidiParser::createParser_SMF();
	if (parser->loadMusic(_data, _dataSize)) {
		parser->setTrack(0);
		parser->setMidiDriver(this);
		parser->setTimerRate(_driver->getBaseTempo());
		parser->property(MidiParser::mpCenterPitchWheelOnUnload, 1);

		_parser = parser;

		syncVolume();

		// Al the tracks are supposed to loop
		_isLooping = true;
		_isPlaying = true;
	}
}

void MusicPlayer::send(uint32 b) {
	if ((b & 0xF0) == 0xC0 && !_isGM && !_nativeMT32) {
		b = (b & 0xFFFF00FF) | MidiDriver::_mt32ToGm[(b >> 8) & 0xFF] << 8;
	}

	Audio::MidiPlayer::send(b);
}

void MusicPlayer::sendToChannel(byte channel, uint32 b) {
	if (!_channelsTable[channel]) {
		_channelsTable[channel] = (channel == 9) ? _driver->getPercussionChannel() : _driver->allocateChannel();
		// If a new channel is allocated during the playback, make sure
		// its volume is correctly initialized.
		if (_channelsTable[channel])
			_channelsTable[channel]->volume(_channelsVolume[channel] * _masterVolume / 255);
	}

	if (_channelsTable[channel])
		_channelsTable[channel]->send(b);
}

} // End of namespace CGE
