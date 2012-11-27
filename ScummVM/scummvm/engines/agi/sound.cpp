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

#include "agi/agi.h"

#include "agi/sound_2gs.h"
#include "agi/sound_coco3.h"
#include "agi/sound_midi.h"
#include "agi/sound_sarien.h"
#include "agi/sound_pcjr.h"

#include "common/textconsole.h"

namespace Agi {

//
// TODO: add support for variable sampling rate in the output device
//

AgiSound *AgiSound::createFromRawResource(uint8 *data, uint32 len, int resnum, SoundMgr &manager, int soundemu) {
	if (data == NULL || len < 2) // Check for too small resource or no resource at all
		return NULL;
	uint16 type = READ_LE_UINT16(data);

	// For V1 sound resources
	if (type != AGI_SOUND_SAMPLE && (type & 0xFF) == 0x01)
		return new PCjrSound(data, len, resnum, manager);

	switch (type) { // Create a sound object based on the type
	case AGI_SOUND_SAMPLE:
		return new IIgsSample(data, len, resnum, manager);
	case AGI_SOUND_MIDI:
		return new IIgsMidi(data, len, resnum, manager);
	case AGI_SOUND_4CHN:
		if (soundemu == SOUND_EMU_MIDI) {
			return new MIDISound(data, len, resnum, manager);
		} else {
			return new PCjrSound(data, len, resnum, manager);
		}
	}

	warning("Sound resource (%d) has unknown type (0x%04x). Not using the sound", resnum, type);
	return NULL;
}

PCjrSound::PCjrSound(uint8 *data, uint32 len, int resnum, SoundMgr &manager) : AgiSound(manager) {
	_data = data; // Save the resource pointer
	_len  = len;  // Save the resource's length
	_type = READ_LE_UINT16(data); // Read sound resource's type

	// Detect V1 sound resources
	if ((_type & 0xFF) == 0x01)
		_type = AGI_SOUND_4CHN;

	_isValid = (_type == AGI_SOUND_4CHN) && (_data != NULL) && (_len >= 2);

	if (!_isValid) // Check for errors
		warning("Error creating PCjr 4-channel sound from resource %d (Type %d, length %d)", resnum, _type, len);
}

const uint8 *PCjrSound::getVoicePointer(uint voiceNum) {
	assert(voiceNum < 4);
	uint16 voiceStartOffset = READ_LE_UINT16(_data + voiceNum * 2);

	return _data + voiceStartOffset;
}

#if 0
static const uint16 period[] = {
	1024, 1085, 1149, 1218, 1290, 1367,
	1448, 1534, 1625, 1722, 1825, 1933
};

static int noteToPeriod(int note) {
	return 10 * (period[note % 12] >> (note / 12 - 3));
}
#endif

void SoundMgr::unloadSound(int resnum) {
	if (_vm->_game.dirSound[resnum].flags & RES_LOADED) {
		if (_vm->_game.sounds[resnum]->isPlaying()) {
			_vm->_game.sounds[resnum]->stop();
		}

		// Release the sound resource's data
		delete _vm->_game.sounds[resnum];
		_vm->_game.sounds[resnum] = NULL;
		_vm->_game.dirSound[resnum].flags &= ~RES_LOADED;
	}
}

/**
 * Start playing a sound resource. The logic here is that when the sound is
 * finished we set the given flag to be true. This way the condition can be
 * detected by the game. On the other hand, if the game wishes to start
 * playing a new sound before the current one is finished, we also let it
 * do that.
 * @param resnum  the sound resource number
 * @param flag    the flag that is wished to be set true when finished
 */
void SoundMgr::startSound(int resnum, int flag) {
	debugC(3, kDebugLevelSound, "startSound(resnum = %d, flag = %d)", resnum, flag);

	if (_vm->_game.sounds[resnum] == NULL) // Is this needed at all?
		return;

	stopSound();

	AgiSoundEmuType type = (AgiSoundEmuType)_vm->_game.sounds[resnum]->type();
	if (type != AGI_SOUND_SAMPLE && type != AGI_SOUND_MIDI && type != AGI_SOUND_4CHN)
		return;
	debugC(3, kDebugLevelSound, "    type = %d", type);

	_vm->_game.sounds[resnum]->play();
	_playingSound = resnum;
	_soundGen->play(resnum);

	// Reset the flag
	_endflag = flag;

	if (_vm->getVersion() < 0x2000) {
		_vm->_game.vars[_endflag] = 0;
	} else {
		_vm->setflag(_endflag, false);
	}
}

void SoundMgr::stopSound() {
	debugC(3, kDebugLevelSound, "stopSound() --> %d", _playingSound);

	if (_playingSound != -1) {
		if (_vm->_game.sounds[_playingSound]) // sanity checking
			_vm->_game.sounds[_playingSound]->stop();
		_soundGen->stop();
		_playingSound = -1;
	}

	// This is probably not needed most of the time, but there also should
	// not be any harm doing it, so do it anyway.
	if (_endflag != -1) {
		if (_vm->getVersion() < 0x2000) {
			_vm->_game.vars[_endflag] = 1;
		} else {
			_vm->setflag(_endflag, true);
		}
	}

	_endflag = -1;
}

int SoundMgr::initSound() {
	return -1;
}

void SoundMgr::deinitSound() {
	stopSound();

	delete _soundGen;
}

void SoundMgr::soundIsFinished() {
	if (_endflag != -1)
		_vm->setflag(_endflag, true);

	if (_playingSound != -1)
		_vm->_game.sounds[_playingSound]->stop();
	_playingSound = -1;
	_endflag = -1;
}

SoundMgr::SoundMgr(AgiBase *agi, Audio::Mixer *pMixer) {
	_vm = agi;
	_endflag = -1;
	_playingSound = -1;

	switch (_vm->_soundemu) {
	case SOUND_EMU_NONE:
	case SOUND_EMU_AMIGA:
	case SOUND_EMU_MAC:
	case SOUND_EMU_PC:
		_soundGen = new SoundGenSarien(_vm, pMixer);
		break;
	case SOUND_EMU_PCJR:
		_soundGen = new SoundGenPCJr(_vm, pMixer);
		break;
	case SOUND_EMU_APPLE2GS:
		_soundGen = new SoundGen2GS(_vm, pMixer);
		break;
	case SOUND_EMU_COCO3:
		_soundGen = new SoundGenCoCo3(_vm, pMixer);
		break;
	case SOUND_EMU_MIDI:
		_soundGen = new SoundGenMIDI(_vm, pMixer);
		break;
	}
}

void SoundMgr::setVolume(uint8 volume) {
	// TODO
}

SoundMgr::~SoundMgr() {
}

} // End of namespace Agi
