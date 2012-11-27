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

#include "common/random.h"

#include "audio/mididrv.h"

#include "agi/agi.h"

#include "agi/sound_sarien.h"

namespace Agi {

#define USE_INTERPOLATION

static const int16 waveformRamp[WAVEFORM_SIZE] = {
	0, 8, 16, 24, 32, 40, 48, 56,
	64, 72, 80, 88, 96, 104, 112, 120,
	128, 136, 144, 152, 160, 168, 176, 184,
	192, 200, 208, 216, 224, 232, 240, 255,
	0, -248, -240, -232, -224, -216, -208, -200,
	-192, -184, -176, -168, -160, -152, -144, -136,
	-128, -120, -112, -104, -96, -88, -80, -72,
	-64, -56, -48, -40, -32, -24, -16, -8	// Ramp up
};

static const int16 waveformSquare[WAVEFORM_SIZE] = {
	255, 230, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 220,
	220, 220, 220, 220, 220, 220, 220, 110,
	-255, -230, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -220, -220, -220, -220, -220,
	-220, -220, -220, -110, 0, 0, 0, 0	// Square
};

static const int16 waveformMac[WAVEFORM_SIZE] = {
	45, 110, 135, 161, 167, 173, 175, 176,
	156, 137, 123, 110, 91, 72, 35, -2,
	-60, -118, -142, -165, -170, -176, -177, -179,
	-177, -176, -164, -152, -117, -82, -17, 47,
	92, 137, 151, 166, 170, 173, 171, 169,
	151, 133, 116, 100, 72, 43, -7, -57,
	-99, -141, -156, -170, -174, -177, -178, -179,
	-175, -172, -165, -159, -137, -114, -67, -19
};

SoundGenSarien::SoundGenSarien(AgiBase *vm, Audio::Mixer *pMixer) : SoundGen(vm, pMixer), _chn() {
	_sndBuffer = (int16 *)calloc(2, BUFFER_SIZE);

	memset(_sndBuffer, 0, BUFFER_SIZE << 1);
	_env = false;
	_playingSound = -1;
	_playing = false;
	_useChorus = true;	// FIXME: Currently always true?

	switch (_vm->_soundemu) {
	case SOUND_EMU_NONE:
		_waveform = waveformRamp;
		_env = true;
		break;
	case SOUND_EMU_AMIGA:
	case SOUND_EMU_PC:
		_waveform = waveformSquare;
		break;
	case SOUND_EMU_MAC:
		_waveform = waveformMac;
		break;
	}

	if (_env) {
		debug(0, "Initializing sound: envelopes enabled (decay=%d, sustain=%d)", ENV_DECAY, ENV_SUSTAIN);
	} else {
		debug(0, "Initializing sound: envelopes disabled");
	}

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
}

SoundGenSarien::~SoundGenSarien() {
	_mixer->stopHandle(_soundHandle);

	free(_sndBuffer);
}

int SoundGenSarien::readBuffer(int16 *buffer, const int numSamples) {
	fillAudio(buffer, numSamples / 2);

	return numSamples;
}

void SoundGenSarien::play(int resnum) {
	AgiSoundEmuType type;

	type = (AgiSoundEmuType)_vm->_game.sounds[resnum]->type();

	assert(type == AGI_SOUND_4CHN);

	_playingSound = resnum;

	PCjrSound *pcjrSound = (PCjrSound *) _vm->_game.sounds[resnum];

	// Initialize channel info
	for (int i = 0; i < NUM_CHANNELS; i++) {
		_chn[i].type = type;
		_chn[i].flags = AGI_SOUND_LOOP;

		if (_env) {
			_chn[i].flags |= AGI_SOUND_ENVELOPE;
			_chn[i].adsr = AGI_SOUND_ENV_ATTACK;
		}

		_chn[i].ins = _waveform;
		_chn[i].size = WAVEFORM_SIZE;
		_chn[i].ptr = pcjrSound->getVoicePointer(i % 4);
		_chn[i].timer = 0;
		_chn[i].vol = 0;
		_chn[i].end = 0;
	}

	memset(_sndBuffer, 0, BUFFER_SIZE << 1);
}

void SoundGenSarien::stop() {
	_playingSound = -1;

	for (int i = 0; i < NUM_CHANNELS; i++)
		stopNote(i);
}

void SoundGenSarien::stopNote(int i) {
	_chn[i].adsr = AGI_SOUND_ENV_RELEASE;

	if (_useChorus) {
		// Stop chorus ;)
		if (_chn[i].type == AGI_SOUND_4CHN &&
			_vm->_soundemu == SOUND_EMU_NONE && i < 3) {
			stopNote(i + 4);
		}
	}
}

void SoundGenSarien::playNote(int i, int freq, int vol) {
	if (!_vm->getflag(fSoundOn))
		vol = 0;
	else if (vol && _vm->_soundemu == SOUND_EMU_PC)
		vol = 160;

	_chn[i].phase = 0;
	_chn[i].freq = freq;
	_chn[i].vol = vol;
	_chn[i].env = 0x10000;
	_chn[i].adsr = AGI_SOUND_ENV_ATTACK;

	if (_useChorus) {
		// Add chorus ;)
		if (_chn[i].type == AGI_SOUND_4CHN &&
			_vm->_soundemu == SOUND_EMU_NONE && i < 3) {

			int newfreq = freq * 1007 / 1000;

			if (freq == newfreq)
				newfreq++;

			playNote(i + 4, newfreq, vol * 2 / 3);
		}
	}
}

void SoundGenSarien::playSound() {
	int i;
	AgiNote note;

	if (_playingSound == -1)
		return;

	_playing = false;
	for (i = 0; i < (_vm->_soundemu == SOUND_EMU_PC ? 1 : 4); i++) {
		_playing |= !_chn[i].end;
		note.read(_chn[i].ptr); // Read a single note (Doesn't advance the pointer)

		if (_chn[i].end)
			continue;

		if ((--_chn[i].timer) <= 0) {
			stopNote(i);

			if (note.freqDiv != 0) {
				int volume = (note.attenuation == 0x0F) ? 0 : (0xFF - note.attenuation * 2);
				playNote(i, note.freqDiv * 10, volume);
			}

			_chn[i].timer = note.duration;

			if (_chn[i].timer == 0xffff) {
				_chn[i].end = 1;
				_chn[i].vol = 0;
				_chn[i].env = 0;

				if (_useChorus) {
					// chorus
					if (_chn[i].type == AGI_SOUND_4CHN && _vm->_soundemu == SOUND_EMU_NONE && i < 3) {
						_chn[i + 4].vol = 0;
						_chn[i + 4].env = 0;
					}
				}
			}
			_chn[i].ptr += 5; // Advance the pointer to the next note data (5 bytes per note)
		}
	}

	if (!_playing) {
		_vm->_sound->soundIsFinished();

		_playingSound = -1;
	}
}

uint32 SoundGenSarien::mixSound() {
	register int i, p;
	const int16 *src;
	int c, b, m;

	memset(_sndBuffer, 0, BUFFER_SIZE << 1);

	if (!_playing || _playingSound == -1)
		return BUFFER_SIZE;

	// Handle PCjr 4-channel sound mixing here
	for (c = 0; c < NUM_CHANNELS; c++) {
		if (!_chn[c].vol)
			continue;

		m = _chn[c].flags & AGI_SOUND_ENVELOPE ?
		    _chn[c].vol * _chn[c].env >> 16 : _chn[c].vol;

		if (_chn[c].type != AGI_SOUND_4CHN || c != 3) {
			src = _chn[c].ins;

			p = _chn[c].phase;
			for (i = 0; i < BUFFER_SIZE; i++) {
				b = src[p >> 8];
#ifdef USE_INTERPOLATION
				b += ((src[((p >> 8) + 1) % _chn[c].size] - src[p >> 8]) * (p & 0xff)) >> 8;
#endif
				_sndBuffer[i] += (b * m) >> 4;

				p += (uint32) 118600 *4 / _chn[c].freq;

				// FIXME: Fingolfin asks: why is there a FIXME here? Please either clarify what
				// needs fixing, or remove it!
				// FIXME
				if (_chn[c].flags & AGI_SOUND_LOOP) {
					p %= _chn[c].size << 8;
				} else {
					if (p >= _chn[c].size << 8) {
						p = _chn[c].vol = 0;
						_chn[c].end = 1;
						break;
					}
				}

			}
			_chn[c].phase = p;
		} else {
			// Add white noise
			for (i = 0; i < BUFFER_SIZE; i++) {
				b = _vm->_rnd->getRandomNumber(255) - 128;
				_sndBuffer[i] += (b * m) >> 4;
			}
		}

		switch (_chn[c].adsr) {
		case AGI_SOUND_ENV_ATTACK:
			// not implemented
			_chn[c].adsr = AGI_SOUND_ENV_DECAY;
			break;
		case AGI_SOUND_ENV_DECAY:
			if (_chn[c].env > _chn[c].vol * ENV_SUSTAIN + ENV_DECAY) {
				_chn[c].env -= ENV_DECAY;
			} else {
				_chn[c].env = _chn[c].vol * ENV_SUSTAIN;
				_chn[c].adsr = AGI_SOUND_ENV_SUSTAIN;
			}
			break;
		case AGI_SOUND_ENV_SUSTAIN:
			break;
		case AGI_SOUND_ENV_RELEASE:
			if (_chn[c].env >= ENV_RELEASE) {
				_chn[c].env -= ENV_RELEASE;
			} else {
				_chn[c].env = 0;
			}
		}
	}

	return BUFFER_SIZE;
}

void SoundGenSarien::fillAudio(int16 *stream, uint len) {
	uint32 p = 0;

	// current number of audio bytes in _sndBuffer
	static uint32 data_available = 0;
	// offset of start of audio bytes in _sndBuffer
	static uint32 data_offset = 0;

	len <<= 2;

	debugC(5, kDebugLevelSound, "(%p, %d)", (void *)stream, len);

	while (len > data_available) {
		memcpy((uint8 *)stream + p, (uint8 *)_sndBuffer + data_offset, data_available);
		p += data_available;
		len -= data_available;

		playSound();
		data_available = mixSound() << 1;
		data_offset = 0;
	}

	memcpy((uint8 *)stream + p, (uint8 *)_sndBuffer + data_offset, len);
	data_offset += len;
	data_available -= len;
}

} // End of namespace Agi
