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

#include "kyra/sound_intern.h"
#include "kyra/resource.h"

#include "common/system.h"
#include "common/config-manager.h"
#include "common/translation.h"

#include "gui/message.h"

namespace Kyra {

class MidiOutput : public MidiDriver_BASE {
public:
	MidiOutput(OSystem *system, MidiDriver *output, bool isMT32, bool defaultMT32);
	~MidiOutput();

	void setSourceVolume(int source, int volume, bool apply=false);

	void initSource(int source);
	void deinitSource(int source);
	void stopNotesOnChannel(int channel);

	void setSoundSource(int source) { _curSource = source; }

	// MidiDriver_BASE interface
	virtual void send(uint32 b);
	virtual void sysEx(const byte *msg, uint16 length);
	virtual void metaEvent(byte type, byte *data, uint16 length);

	// TODO: Get rid of the following two methods
	void setTimerCallback(void *timerParam, void (*timerProc)(void *)) { _output->setTimerCallback(timerParam, timerProc); }
	uint32 getBaseTempo() { return _output->getBaseTempo(); }


private:
	void sendIntern(const byte event, const byte channel, byte param1, const byte param2);
	void sendSysEx(const byte p1, const byte p2, const byte p3, const byte *buffer, const int size);

	OSystem *_system;
	MidiDriver *_output;

	bool _isMT32;
	bool _defaultMT32;

	struct Controller {
		byte controller;
		byte value;
	};

	enum {
		kChannelLocked = 0x80,
		kChannelProtected = 0x40
	};

	struct Channel {
		byte flags;

		byte program;
		int16 pitchWheel;

		byte noteCount;

		Controller controllers[9];
	} _channels[16];

	int lockChannel();
	void unlockChannel(int channel);

	int _curSource;

	struct SoundSource {
		int volume;

		int8 channelMap[16];
		byte channelProgram[16];
		int16 channelPW[16];
		Controller controllers[16][9];

		struct Note {
			byte channel;
			byte note;
		};

		Note notes[32];
	} _sources[4];
};

MidiOutput::MidiOutput(OSystem *system, MidiDriver *output, bool isMT32, bool defaultMT32) : _system(system), _output(output) {
	_isMT32 = isMT32;
	_defaultMT32 = defaultMT32;

	int ret = _output->open();
	if (ret != MidiDriver::MERR_ALREADY_OPEN && ret != 0)
		error("Couldn't open midi driver");

	static const Controller defaultControllers[] = {
		{ 0x07, 0x7F }, { 0x01, 0x00 }, { 0x0A, 0x40 },
		{ 0x0B, 0x7F }, { 0x40, 0x00 }, { 0x72, 0x00 },
		{ 0x6E, 0x00 }, { 0x6F, 0x00 }, { 0x70, 0x00 }
	};

	static const byte defaultPrograms[] = {
		0x44, 0x30, 0x5F, 0x4E, 0x29, 0x03, 0x6E, 0x7A, 0xFF
	};

	static const byte sysEx1[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	static const byte sysEx2[] = { 3, 4, 3, 4, 3, 4, 3, 4, 4 };
	static const byte sysEx3[] = { 0, 3, 2 };

	if (_isMT32) {
		sendSysEx(0x7F, 0x00, 0x00, sysEx1, 1);
		sendSysEx(0x10, 0x00, 0x0D, sysEx1, 9);
		sendSysEx(0x10, 0x00, 0x04, sysEx2, 9);
		sendSysEx(0x10, 0x00, 0x01, sysEx3, 3);
	} else {
		_output->sendGMReset();
	}

	memset(_channels, 0, sizeof(_channels));
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 9; ++j)
			_channels[i].controllers[j] = defaultControllers[j];
		_channels[i].pitchWheel = -1;
		_channels[i].program = 0xFF;
	}

	for (int i = 0; i < 9; ++i) {
		for (int j = 1; j <= 9; ++j)
			sendIntern(0xB0, j, defaultControllers[i].controller, defaultControllers[i].value);
	}

	for (int i = 1; i <= 9; ++i) {
		sendIntern(0xE0, i, 0x00, 0x40);
		if (defaultPrograms[i - 1] != 0xFF)
			sendIntern(0xC0, i, defaultPrograms[i - 1], 0x00);
	}

	for (int i = 0; i < 4; ++i) {
		_sources[i].volume = 256;
		initSource(i);
	}
}


MidiOutput::~MidiOutput() {
	_output->close();
	delete _output;
}

void MidiOutput::send(uint32 b) {
	const byte event = b & 0xF0;
	const byte channel = b & 0x0F;
	byte param1 = (b >>  8) & 0xFF;
	byte param2 = (b >> 16) & 0xFF;

	if (event == 0xE0) {							// Pitch-Wheel
		_channels[channel].pitchWheel =
		_sources[_curSource].channelPW[channel] = (param2 << 8) | param1;
	} else if (event == 0xC0) {						// Program change
		_channels[channel].program =
		_sources[_curSource].channelProgram[channel] = param1;
	} else if (event == 0xB0) {						// Controller change
		for (int i = 0; i < 9; ++i) {
			Controller &cont = _sources[_curSource].controllers[channel][i];
			if (cont.controller == param1) {
				cont.value = param2;
				break;
			}
		}

		if (param1 == 0x07) {
			param2 = (param2 * _sources[_curSource].volume) >> 8;
		} else if (param1 == 0x6E) {	// Lock Channel
			if (param2 >= 0x40) {	// Lock Channel
				int chan = lockChannel();
				if (chan < 0)
					chan = channel;
				_sources[_curSource].channelMap[channel] = chan;
			} else {				// Unlock Channel
				stopNotesOnChannel(channel);
				unlockChannel(_sources[_curSource].channelMap[channel]);
				_sources[_curSource].channelMap[channel] = channel;
			}
		} else if (param1 == 0x6F) {	// Protect Channel
			if (param2 >= 0x40) {	// Protect Channel
				_channels[channel].flags |= kChannelProtected;
			} else {				// Unprotect Channel
				_channels[channel].flags &= ~kChannelProtected;
			}
		} else if (param1 == 0x7B) {	// All notes off
			// FIXME: Since the XMIDI parsers sends this
			// on track change, we simply ignore it.
			return;
		}
	} else if (event == 0x90 || event == 0x80) {	// Note On/Off
		if (!(_channels[channel].flags & kChannelLocked)) {
			const bool remove = (event == 0x80) || (param2 == 0x00);
			int note = -1;

			for (int i = 0; i < 32; ++i) {
				if (remove) {
					if (_sources[_curSource].notes[i].channel == channel &&
						_sources[_curSource].notes[i].note == param1) {
						note = i;
						break;
					}
				} else {
					if (_sources[_curSource].notes[i].channel == 0xFF) {
						note = i;
						break;
					}
				}
			}

			if (note != -1) {
				if (remove) {
					_sources[_curSource].notes[note].channel = 0xFF;

					--_channels[_sources[_curSource].channelMap[channel]].noteCount;
				} else {
					_sources[_curSource].notes[note].channel = channel;
					_sources[_curSource].notes[note].note = param1;

					++_channels[_sources[_curSource].channelMap[channel]].noteCount;
				}

				sendIntern(event, _sources[_curSource].channelMap[channel], param1, param2);
			}
		}
		return;
	}

	if (!(_channels[channel].flags & kChannelLocked))
		sendIntern(event, _sources[_curSource].channelMap[channel], param1, param2);
}

void MidiOutput::sendIntern(const byte event, const byte channel, byte param1, const byte param2) {
	if (event == 0xC0) {
		// MT32 -> GM conversion
		if (!_isMT32 && _defaultMT32)
			param1 = MidiDriver::_mt32ToGm[param1];
	}

	_output->send(event | channel, param1, param2);
}

void MidiOutput::sysEx(const byte *msg, uint16 length) {
	// Wait the time it takes to send the SysEx data
	uint32 delay = (length + 2) * 1000 / 3125;

	// Plus an additional delay for the MT-32 rev00
	if (_isMT32)
		delay += 40;

	_output->sysEx(msg, length);
	_system->delayMillis(delay);
}

void MidiOutput::sendSysEx(const byte p1, const byte p2, const byte p3, const byte *buffer, const int size) {
	int bufferSize = 8 + size;
	byte *outBuffer = new byte[bufferSize];
	assert(outBuffer);

	outBuffer[0] = 0x41;
	outBuffer[1] = 0x10;
	outBuffer[2] = 0x16;
	outBuffer[3] = 0x12;

	outBuffer[4] = p1;
	outBuffer[5] = p2;
	outBuffer[6] = p3;

	memcpy(outBuffer + 7, buffer, size);

	uint16 checkSum = p1 + p2 + p3;
	for (int i = 0; i < size; ++i)
		checkSum += buffer[i];
	checkSum &= 0x7F;
	checkSum -= 0x80;
	checkSum = -checkSum;
	checkSum &= 0x7F;

	outBuffer[7+size] = checkSum;

	sysEx(outBuffer, bufferSize);

	delete[] outBuffer;
}

void MidiOutput::metaEvent(byte type, byte *data, uint16 length) {
	if (type == 0x2F) // End of Track
		deinitSource(_curSource);

	_output->metaEvent(type, data, length);
}

void MidiOutput::setSourceVolume(int source, int volume, bool apply) {
	_sources[source].volume = volume;

	if (apply) {
		for (int i = 0; i < 16; ++i) {
			// Controller 0 in the state table should always be '7' aka
			// volume control
			byte realVol = (_channels[i].controllers[0].value * volume) >> 8;
			sendIntern(0xB0, i, 0x07, realVol);
		}
	}
}

void MidiOutput::initSource(int source) {
	memset(_sources[source].notes, -1, sizeof(_sources[source].notes));

	for (int i = 0; i < 16; ++i) {
		_sources[source].channelMap[i] = i;
		_sources[source].channelProgram[i] = 0xFF;
		_sources[source].channelPW[i] = -1;

		for (int j = 0; j < 9; ++j)
			_sources[source].controllers[i][j] = _channels[i].controllers[j];
	}
}

void MidiOutput::deinitSource(int source) {
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 9; ++j) {
			const Controller &cont = _sources[source].controllers[i][j];

			if (cont.controller == 0x40) {
				if (cont.value >= 0x40)
					sendIntern(0xB0, i, 0x40, 0);
			} else if (cont.controller == 0x6E) {
				if (cont.value >= 0x40) {
					stopNotesOnChannel(i);
					unlockChannel(_sources[source].channelMap[i]);
					_sources[source].channelMap[i] = i;
				}
			} else if (cont.controller == 0x6F) {
				if (cont.value >= 0x40)
					_channels[i].flags &= ~kChannelProtected;
			} else if (cont.controller == 0x70) {
				if (cont.value >= 0x40)
					sendIntern(0xB0, i, 0x70, 0);
			}
		}
	}
}

int MidiOutput::lockChannel() {
	int channel = -1;
	int notes = 0xFF;
	byte flags = kChannelLocked | kChannelProtected;

	while (channel == -1) {
		for (int i = _isMT32 ? 8 : 15; i >= 1; --i) {
			if (_channels[i].flags & flags)
				continue;
			if (_channels[i].noteCount < notes) {
				channel = i;
				notes = _channels[i].noteCount;
			}
		}

		if (channel == -1) {
			if (flags & kChannelProtected)
				flags &= ~kChannelProtected;
			else
				break;
		}
	}

	if (channel == -1)
		return -1;

	sendIntern(0xB0, channel, 0x40, 0);
	stopNotesOnChannel(channel);
	_channels[channel].noteCount = 0;
	_channels[channel].flags |= kChannelLocked;

	return channel;
}

void MidiOutput::unlockChannel(int channel) {
	if (!(_channels[channel].flags & kChannelLocked))
		return;

	_channels[channel].flags &= ~kChannelLocked;
	_channels[channel].noteCount = 0;
	sendIntern(0xB0, channel, 0x40, 0);
	sendIntern(0xB0, channel, 0x7B, 0);

	for (int i = 0; i < 9; ++i) {
		if (_channels[channel].controllers[i].value != 0xFF)
			sendIntern(0xB0, channel, _channels[channel].controllers[i].controller, _channels[channel].controllers[i].value);
	}

	if (_channels[channel].program != 0xFF)
		sendIntern(0xC0, channel, _channels[channel].program, 0);

	if (_channels[channel].pitchWheel != -1)
		sendIntern(0xE0, channel, _channels[channel].pitchWheel & 0xFF, (_channels[channel].pitchWheel >> 8) & 0xFF);
}

void MidiOutput::stopNotesOnChannel(int channel) {
	for (int i = 0; i < 4; ++i) {
		SoundSource &sound = _sources[i];
		for (int j = 0; j < 32; ++j) {
			if (sound.notes[j].channel == channel) {
				sound.notes[j].channel = 0xFF;
				sendIntern(0x80, sound.channelMap[channel], sound.notes[j].note, 0);
				--_channels[sound.channelMap[channel]].noteCount;
			}
		}
	}
}

#pragma mark -

SoundMidiPC::SoundMidiPC(KyraEngine_v1 *vm, Audio::Mixer *mixer, MidiDriver *driver, kType type) : Sound(vm, mixer) {
	_driver = driver;
	_output = 0;

	_musicFile = _sfxFile = 0;
	_currentResourceSet = 0;
	memset(&_resInfo, 0, sizeof(_resInfo));

	_music = MidiParser::createParser_XMIDI();
	assert(_music);
	for (int i = 0; i < 3; ++i) {
		_sfx[i] = MidiParser::createParser_XMIDI();
		assert(_sfx[i]);
	}

	_musicVolume = _sfxVolume = 0;
	_fadeMusicOut = false;

	_type = type;
	assert(_type == kMidiMT32 || _type == kMidiGM || _type == kPCSpkr);

	// Only General MIDI isn't a Roland MT-32 MIDI implemenation,
	// even the PC Speaker driver is a Roland MT-32 based MIDI implementation.
	// Thus we set "_nativeMT32" for all types except Gerneral MIDI to true.
	_nativeMT32 = (_type != kMidiGM);

	// KYRA1 does not include any General MIDI tracks, thus we have
	// to overwrite the internal type with MT32 to get the correct
	// file extension.
	if (_vm->game() == GI_KYRA1 && _type == kMidiGM)
		_type = kMidiMT32;

	// Display a warning about possibly wrong sound when the user only has
	// a General MIDI device, but the game is setup to use Roland MT32 MIDI.
	// (This will only happen in The Legend of Kyrandia 1 though, all other
	// supported games include special General MIDI tracks).
	if (_type == kMidiMT32 && !_nativeMT32) {
		::GUI::MessageDialog dialog(_("You appear to be using a General MIDI device,\n"
									"but your game only supports Roland MT32 MIDI.\n"
									"We try to map the Roland MT32 instruments to\n"
									"General MIDI ones. It is still possible that\n"
									"some tracks sound incorrect."));
		dialog.runModal();
	}
}

SoundMidiPC::~SoundMidiPC() {
	Common::StackLock lock(_mutex);
	_output->setTimerCallback(0, 0);

	delete _music;
	for (int i = 0; i < 3; ++i)
		delete _sfx[i];

	delete _output; // This automatically frees _driver (!)

	if (_musicFile != _sfxFile)
		delete[] _sfxFile;

	delete[] _musicFile;

	for (int i = 0; i < 3; i++)
		initAudioResourceInfo(i, 0);
}

bool SoundMidiPC::init() {
	_output = new MidiOutput(_vm->_system, _driver, _nativeMT32, (_type != kMidiGM));
	assert(_output);

	updateVolumeSettings();

	_music->setMidiDriver(_output);
	_music->setTempo(_output->getBaseTempo());
	_music->setTimerRate(_output->getBaseTempo());

	for (int i = 0; i < 3; ++i) {
		_sfx[i]->setMidiDriver(_output);
		_sfx[i]->setTempo(_output->getBaseTempo());
		_sfx[i]->setTimerRate(_output->getBaseTempo());
	}

	_output->setTimerCallback(this, SoundMidiPC::onTimer);

	if (_nativeMT32 && _type == kMidiMT32) {
		const char *midiFile = 0;
		const char *pakFile = 0;
		if (_vm->game() == GI_KYRA1) {
			midiFile = "INTRO";
		} else if (_vm->game() == GI_KYRA2) {
			midiFile = "HOF_SYX";
			pakFile = "AUDIO.PAK";
		} else if (_vm->game() == GI_LOL) {
			midiFile = "LOREINTR";

			if (_vm->gameFlags().isDemo) {
				if (_vm->gameFlags().useAltShapeHeader) {
					// Intro demo
					pakFile = "INTROVOC.PAK";
				} else {
					// Kyra2 SEQ player based demo
					pakFile = "GENERAL.PAK";
					midiFile = "LOLSYSEX";
				}
			} else {
				if (_vm->gameFlags().isTalkie)
					pakFile = "ENG/STARTUP.PAK";
				else
					pakFile = "INTROVOC.PAK";
			}
		}

		if (!midiFile)
			return true;

		if (pakFile)
			_vm->resource()->loadPakFile(pakFile);

		loadSoundFile(midiFile);
		playTrack(0);

		Common::Event event;
		while (isPlaying() && !_vm->shouldQuit()) {
			_vm->_system->updateScreen();
			_vm->_eventMan->pollEvent(event);
			_vm->_system->delayMillis(10);
		}

		if (pakFile)
			_vm->resource()->unloadPakFile(pakFile);
	}

	return true;
}

void SoundMidiPC::updateVolumeSettings() {
	Common::StackLock lock(_mutex);

	if (!_output)
		return;

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	const int newMusVol = (mute ? 0 : ConfMan.getInt("music_volume"));
	_sfxVolume = (mute ? 0 : ConfMan.getInt("sfx_volume"));

	_output->setSourceVolume(0, newMusVol, newMusVol != _musicVolume);
	_musicVolume = newMusVol;

	for (int i = 1; i < 4; ++i)
		_output->setSourceVolume(i, _sfxVolume, false);
}

void SoundMidiPC::initAudioResourceInfo(int set, void *info) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		delete _resInfo[set];
		_resInfo[set] = info ? new SoundResourceInfo_PC(*(SoundResourceInfo_PC*)info) : 0;
	}
}

void SoundMidiPC::selectAudioResourceSet(int set) {
	if (set >= kMusicIntro && set <= kMusicFinale) {
		if (_resInfo[set])
			_currentResourceSet = set;
	}
}

bool SoundMidiPC::hasSoundFile(uint file) const {
	if (file < res()->fileListSize)
		return (res()->fileList[file] != 0);
	return false;
}

void SoundMidiPC::loadSoundFile(uint file) {
	if (file < res()->fileListSize)
		loadSoundFile(res()->fileList[file]);
}

void SoundMidiPC::loadSoundFile(Common::String file) {
	Common::StackLock lock(_mutex);
	file = getFileName(file);

	if (_mFileName == file)
		return;

	if (!_vm->resource()->exists(file.c_str()))
		return;

	// When loading a new file we stop all notes
	// still running on our own, just to prevent
	// glitches
	for (int i = 0; i < 16; ++i)
		_output->stopNotesOnChannel(i);

	delete[] _musicFile;
	uint32 fileSize = 0;
	_musicFile = _vm->resource()->fileData(file.c_str(), &fileSize);
	_mFileName = file;

	_output->setSoundSource(0);
	_music->loadMusic(_musicFile, fileSize);
	_music->stopPlaying();

	// Since KYRA1 uses the same file for SFX and Music
	// we setup sfx to play from music file as well
	if (_vm->game() == GI_KYRA1) {
		for (int i = 0; i < 3; ++i) {
			_output->setSoundSource(i+1);
			_sfx[i]->loadMusic(_musicFile, fileSize);
			_sfx[i]->stopPlaying();
		}
	}
}

void SoundMidiPC::loadSfxFile(Common::String file) {
	Common::StackLock lock(_mutex);

	// Kyrandia 1 doesn't use a special sfx file
	if (_vm->game() == GI_KYRA1)
		return;

	file = getFileName(file);

	if (_sFileName == file)
		return;

	if (!_vm->resource()->exists(file.c_str()))
		return;

	delete[] _sfxFile;

	uint32 fileSize = 0;
	_sfxFile = _vm->resource()->fileData(file.c_str(), &fileSize);
	_sFileName = file;

	for (int i = 0; i < 3; ++i) {
		_output->setSoundSource(i+1);
		_sfx[i]->loadMusic(_sfxFile, fileSize);
		_sfx[i]->stopPlaying();
	}
}

void SoundMidiPC::playTrack(uint8 track) {
	if (!_musicEnabled)
		return;

	haltTrack();

	Common::StackLock lock(_mutex);

	_fadeMusicOut = false;
	_output->setSourceVolume(0, _musicVolume, true);

	_output->initSource(0);
	_output->setSourceVolume(0, _musicVolume, true);
	_music->setTrack(track);
}

void SoundMidiPC::haltTrack() {
	Common::StackLock lock(_mutex);

	_output->setSoundSource(0);
	_music->stopPlaying();
	_output->deinitSource(0);
}

bool SoundMidiPC::isPlaying() const {
	Common::StackLock lock(_mutex);

	return _music->isPlaying();
}

void SoundMidiPC::playSoundEffect(uint8 track, uint8) {
	if (!_sfxEnabled)
		return;

	Common::StackLock lock(_mutex);
	for (int i = 0; i < 3; ++i) {
		if (!_sfx[i]->isPlaying()) {
			_output->initSource(i+1);
			_sfx[i]->setTrack(track);
			return;
		}
	}
}

void SoundMidiPC::stopAllSoundEffects() {
	Common::StackLock lock(_mutex);

	for (int i = 0; i < 3; ++i) {
		_output->setSoundSource(i+1);
		_sfx[i]->stopPlaying();
		_output->deinitSource(i+1);
	}
}

void SoundMidiPC::beginFadeOut() {
	Common::StackLock lock(_mutex);

	_fadeMusicOut = true;
	_fadeStartTime = _vm->_system->getMillis();
}

void SoundMidiPC::pause(bool paused) {
	Common::StackLock lock(_mutex);

	if (paused) {
		_music->setMidiDriver(0);
		for (int i = 0; i < 3; i++)
			_sfx[i]->setMidiDriver(0);
		for (int i = 0; i < 16; i++)
			_output->stopNotesOnChannel(i);
	} else {
		_music->setMidiDriver(_output);
		for (int i = 0; i < 3; ++i)
			_sfx[i]->setMidiDriver(_output);
		// Possible TODO (IMHO unnecessary): restore notes and/or update _fadeStartTime
	}
}

void SoundMidiPC::onTimer(void *data) {
	SoundMidiPC *midi = (SoundMidiPC *)data;

	Common::StackLock lock(midi->_mutex);

	if (midi->_fadeMusicOut) {
		static const uint32 musicFadeTime = 1 * 1000;

		if (midi->_fadeStartTime + musicFadeTime > midi->_vm->_system->getMillis()) {
			int volume = (byte)((musicFadeTime - (midi->_vm->_system->getMillis() - midi->_fadeStartTime)) * midi->_musicVolume / musicFadeTime);
			midi->_output->setSourceVolume(0, volume, true);
		} else {
			for (int i = 0; i < 16; ++i)
				midi->_output->stopNotesOnChannel(i);
			for (int i = 0; i < 4; ++i)
				midi->_output->deinitSource(i);

			midi->_output->setSoundSource(0);
			midi->_music->stopPlaying();

			for (int i = 0; i < 3; ++i) {
				midi->_output->setSoundSource(i+1);
				midi->_sfx[i]->stopPlaying();
			}

			midi->_output->setSourceVolume(0, midi->_musicVolume, true);
			midi->_fadeMusicOut = false;
		}
	}

	midi->_output->setSoundSource(0);
	midi->_music->onTimer();

	for (int i = 0; i < 3; ++i) {
		midi->_output->setSoundSource(i+1);
		midi->_sfx[i]->onTimer();
	}
}

Common::String SoundMidiPC::getFileName(const Common::String &str) {
	Common::String file = str;
	if (_type == kMidiMT32)
		file += ".XMI";
	else if (_type == kMidiGM)
		file += ".C55";
	else if (_type == kPCSpkr)
		file += ".PCS";

	if (_vm->resource()->exists(file.c_str()))
		return file;

	return str + ".XMI";
}

} // End of namespace Kyra
