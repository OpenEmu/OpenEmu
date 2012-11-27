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

#include "sci/sci.h"

#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/fmopl.h"
#include "audio/softsynth/emumidi.h"

#include "sci/resource.h"
#include "sci/sound/drivers/mididriver.h"

namespace Sci {

#ifdef __DC__
#define STEREO false
#else
#define STEREO true
#endif

// FIXME: We don't seem to be sending the polyphony init data, so disable this for now
#define ADLIB_DISABLE_VOICE_MAPPING

class MidiDriver_AdLib : public MidiDriver_Emulated {
public:
	enum {
		kVoices = 9,
		kRhythmKeys = 62
	};

	MidiDriver_AdLib(Audio::Mixer *mixer) : MidiDriver_Emulated(mixer), _playSwitch(true), _masterVolume(15), _rhythmKeyMap(0), _opl(0) { }
	virtual ~MidiDriver_AdLib() { }

	// MidiDriver
	int openAdLib(bool isSCI0);
	void close();
	void send(uint32 b);
	MidiChannel *allocateChannel() { return NULL; }
	MidiChannel *getPercussionChannel() { return NULL; }

	// AudioStream
	bool isStereo() const { return _stereo; }
	int getRate() const { return _mixer->getOutputRate(); }

	// MidiDriver_Emulated
	void generateSamples(int16 *buf, int len);

	void setVolume(byte volume);
	void playSwitch(bool play);
	bool loadResource(const byte *data, uint size);
	virtual uint32 property(int prop, uint32 param);

	bool useRhythmChannel() const { return _rhythmKeyMap != NULL; }

private:
	enum ChannelID {
		kLeftChannel = 1,
		kRightChannel = 2
	};

	struct AdLibOperator {
		bool amplitudeMod;
		bool vibrato;
		bool envelopeType;
		bool kbScaleRate;
		byte frequencyMult;		// (0-15)
		byte kbScaleLevel;		// (0-3)
		byte totalLevel;		// (0-63, 0=max, 63=min)
		byte attackRate;		// (0-15)
		byte decayRate;			// (0-15)
		byte sustainLevel;		// (0-15)
		byte releaseRate;		// (0-15)
		byte waveForm;			// (0-3)
	};

	struct AdLibModulator {
		byte feedback;			// (0-7)
		bool algorithm;
	};

	struct AdLibPatch {
		AdLibOperator op[2];
		AdLibModulator mod;
	};

	struct Channel {
		uint8 patch;			// Patch setting
		uint8 volume;			// Channel volume (0-63)
		uint8 pan;				// Pan setting (0-127, 64 is center)
		uint8 holdPedal;		// Hold pedal setting (0 to 63 is off, 127 to 64 is on)
		uint8 extraVoices;		// The number of additional voices this channel optimally needs
		uint16 pitchWheel;		// Pitch wheel setting (0-16383, 8192 is center)
		uint8 lastVoice;		// Last voice used for this MIDI channel
		bool enableVelocity;	// Enable velocity control (SCI0)

		Channel() : patch(0), volume(63), pan(64), holdPedal(0), extraVoices(0),
					pitchWheel(8192), lastVoice(0), enableVelocity(false) { }
	};

	struct AdLibVoice {
		int8 channel;			// MIDI channel that this voice is assigned to or -1
		int8 note;				// Currently playing MIDI note or -1
		int patch;				// Currently playing patch or -1
		uint8 velocity;			// Note velocity
		bool isSustained;		// Flag indicating a note that is being sustained by the hold pedal
		uint16 age;				// Age of the current note

		AdLibVoice() : channel(-1), note(-1), patch(-1), velocity(0), isSustained(false), age(0) { }
	};

	bool _stereo;
	bool _isSCI0;
	OPL::OPL *_opl;
	bool _playSwitch;
	int _masterVolume;
	Channel _channels[MIDI_CHANNELS];
	AdLibVoice _voices[kVoices];
	byte *_rhythmKeyMap;
	Common::Array<AdLibPatch> _patches;

	void loadInstrument(const byte *ins);
	void voiceOn(int voice, int note, int velocity);
	void voiceOff(int voice);
	void setPatch(int voice, int patch);
	void setNote(int voice, int note, bool key);
	void setVelocity(int voice);
	void setOperator(int oper, AdLibOperator &op);
	void setRegister(int reg, int value, int channels = kLeftChannel | kRightChannel);
	void renewNotes(int channel, bool key);
	void noteOn(int channel, int note, int velocity);
	void noteOff(int channel, int note);
	int findVoice(int channel);
	void voiceMapping(int channel, int voices);
	void assignVoices(int channel, int voices);
	void releaseVoices(int channel, int voices);
	void donateVoices();
	int findVoiceBasic(int channel);
	void setVelocityReg(int regOffset, int velocity, int kbScaleLevel, int pan);
	int calcVelocity(int voice, int op);
};

class MidiPlayer_AdLib : public MidiPlayer {
public:
	MidiPlayer_AdLib(SciVersion soundVersion) : MidiPlayer(soundVersion) { _driver = new MidiDriver_AdLib(g_system->getMixer()); }
	~MidiPlayer_AdLib() {
		delete _driver;
		_driver = 0;
	}

	int open(ResourceManager *resMan);
	void close();

	byte getPlayId() const;
	int getPolyphony() const { return MidiDriver_AdLib::kVoices; }
	bool hasRhythmChannel() const { return false; }
	void setVolume(byte volume) { static_cast<MidiDriver_AdLib *>(_driver)->setVolume(volume); }
	void playSwitch(bool play) { static_cast<MidiDriver_AdLib *>(_driver)->playSwitch(play); }
	void loadInstrument(int idx, byte *data);

	int getLastChannel() const { return (static_cast<const MidiDriver_AdLib *>(_driver)->useRhythmChannel() ? 8 : 15); }
};

static const byte registerOffset[MidiDriver_AdLib::kVoices] = {
	0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12
};

static const byte velocityMap1[64] = {
	0x00, 0x0c, 0x0d, 0x0e, 0x0f, 0x11, 0x12, 0x13,
	0x14, 0x16, 0x17, 0x18, 0x1a, 0x1b, 0x1c, 0x1d,
	0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
	0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2d, 0x2d, 0x2e,
	0x2f, 0x30, 0x31, 0x32, 0x32, 0x33, 0x34, 0x34,
	0x35, 0x36, 0x36, 0x37, 0x38, 0x38, 0x39, 0x3a,
	0x3b, 0x3b, 0x3b, 0x3c, 0x3c, 0x3c, 0x3d, 0x3d,
	0x3d, 0x3e, 0x3e, 0x3e, 0x3e, 0x3f, 0x3f, 0x3f
};

static const byte velocityMap2[64] = {
	0x00, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
	0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x21,
	0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x2f, 0x30,
	0x31, 0x32, 0x32, 0x33, 0x34, 0x34, 0x35, 0x36,
	0x36, 0x37, 0x38, 0x38, 0x39, 0x39, 0x3a, 0x3a,
	0x3b, 0x3b, 0x3b, 0x3c, 0x3c, 0x3c, 0x3d, 0x3d,
	0x3d, 0x3e, 0x3e, 0x3e, 0x3e, 0x3f, 0x3f, 0x3f
};

static const int ym3812_note[13] = {
	0x157, 0x16b, 0x181, 0x198, 0x1b0, 0x1ca,
	0x1e5, 0x202, 0x220, 0x241, 0x263, 0x287,
	0x2ae
};

int MidiDriver_AdLib::openAdLib(bool isSCI0) {
	int rate = _mixer->getOutputRate();

	_stereo = STEREO;

	debug(3, "ADLIB: Starting driver in %s mode", (isSCI0 ? "SCI0" : "SCI1"));
	_isSCI0 = isSCI0;

	_opl = OPL::Config::create(isStereo() ? OPL::Config::kDualOpl2 : OPL::Config::kOpl2);

	// Try falling back to mono, thus plain OPL2 emualtor, when no Dual OPL2 is available.
	if (!_opl && _stereo) {
		_stereo = false;
		_opl = OPL::Config::create(OPL::Config::kOpl2);
	}

	if (!_opl)
		return -1;

	_opl->init(rate);

	setRegister(0xBD, 0);
	setRegister(0x08, 0);
	setRegister(0x01, 0x20);

	MidiDriver_Emulated::open();

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO);

	return 0;
}

void MidiDriver_AdLib::close() {
	_mixer->stopHandle(_mixerSoundHandle);

	delete _opl;
	delete[] _rhythmKeyMap;
}

void MidiDriver_AdLib::setVolume(byte volume) {
	_masterVolume = volume;
	renewNotes(-1, true);
}

// MIDI messages can be found at http://www.midi.org/techspecs/midimessages.php
void MidiDriver_AdLib::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	switch (command) {
	case 0x80:
		noteOff(channel, op1);
		break;
	case 0x90:
		noteOn(channel, op1, op2);
		break;
	case 0xb0:
		switch (op1) {
		case 0x07:
			_channels[channel].volume = op2 >> 1;
			renewNotes(channel, true);
			break;
		case 0x0a:
			_channels[channel].pan = op2;
			renewNotes(channel, true);
			break;
		case 0x40:
			_channels[channel].holdPedal = op2;
			if (op2 == 0) {
				for (int i = 0; i < kVoices; i++) {
					if ((_voices[i].channel == channel) && _voices[i].isSustained)
						voiceOff(i);
				}
			}
			break;
		case 0x4b:
#ifndef ADLIB_DISABLE_VOICE_MAPPING
			voiceMapping(channel, op2);
#endif
			break;
		case 0x4e:
			_channels[channel].enableVelocity = op2;
			break;
		case SCI_MIDI_CHANNEL_NOTES_OFF:
			for (int i = 0; i < kVoices; i++)
				if ((_voices[i].channel == channel) && (_voices[i].note != -1))
					voiceOff(i);
			break;
		default:
			//warning("ADLIB: ignoring MIDI command %02x %02x %02x", command | channel, op1, op2);
			break;
		}
		break;
	case 0xc0:
		_channels[channel].patch = op1;
		break;
	// The original AdLib driver from sierra ignores aftertouch completely, so should we
	case 0xa0: // Polyphonic key pressure (aftertouch)
	case 0xd0: // Channel pressure (aftertouch)
		break;
	case 0xe0:
		_channels[channel].pitchWheel = (op1 & 0x7f) | ((op2 & 0x7f) << 7);
		renewNotes(channel, true);
		break;
	default:
		warning("ADLIB: Unknown event %02x", command);
	}
}

void MidiDriver_AdLib::generateSamples(int16 *data, int len) {
	if (isStereo())
		len <<= 1;
	_opl->readBuffer(data, len);

	// Increase the age of the notes
	for (int i = 0; i < kVoices; i++) {
		if (_voices[i].note != -1)
			_voices[i].age++;
	}
}

void MidiDriver_AdLib::loadInstrument(const byte *ins) {
	AdLibPatch patch;

	// Set data for the operators
	for (int i = 0; i < 2; i++) {
		const byte *op = ins + i * 13;
		patch.op[i].kbScaleLevel = op[0] & 0x3;
		patch.op[i].frequencyMult = op[1] & 0xf;
		patch.op[i].attackRate = op[3] & 0xf;
		patch.op[i].sustainLevel = op[4] & 0xf;
		patch.op[i].envelopeType = op[5];
		patch.op[i].decayRate = op[6] & 0xf;
		patch.op[i].releaseRate = op[7] & 0xf;
		patch.op[i].totalLevel = op[8] & 0x3f;
		patch.op[i].amplitudeMod = op[9];
		patch.op[i].vibrato = op[10];
		patch.op[i].kbScaleRate = op[11];
	}
	patch.op[0].waveForm = ins[26] & 0x3;
	patch.op[1].waveForm = ins[27] & 0x3;

	// Set data for the modulator
	patch.mod.feedback = ins[2] & 0x7;
	patch.mod.algorithm = !ins[12]; // Flag is inverted

	_patches.push_back(patch);
}

void MidiDriver_AdLib::voiceMapping(int channel, int voices) {
	int curVoices = 0;

	for (int i = 0; i < kVoices; i++)
		if (_voices[i].channel == channel)
			curVoices++;

	curVoices += _channels[channel].extraVoices;

	if (curVoices < voices) {
		debug(3, "ADLIB: assigning %i additional voices to channel %i", voices - curVoices, channel);
		assignVoices(channel, voices - curVoices);
	} else if (curVoices > voices) {
		debug(3, "ADLIB: releasing %i voices from channel %i", curVoices - voices, channel);
		releaseVoices(channel, curVoices - voices);
		donateVoices();
	}
}

void MidiDriver_AdLib::assignVoices(int channel, int voices) {
	assert(voices > 0);

	for (int i = 0; i < kVoices; i++)
		if (_voices[i].channel == -1) {
			_voices[i].channel = channel;
			if (--voices == 0)
				return;
		}

	_channels[channel].extraVoices += voices;
}

void MidiDriver_AdLib::releaseVoices(int channel, int voices) {
	if (_channels[channel].extraVoices >= voices) {
		_channels[channel].extraVoices -= voices;
		return;
	}

	voices -= _channels[channel].extraVoices;
	_channels[channel].extraVoices = 0;

	for (int i = 0; i < kVoices; i++) {
		if ((_voices[i].channel == channel) && (_voices[i].note == -1)) {
			_voices[i].channel = -1;
			if (--voices == 0)
				return;
		}
	}

	for (int i = 0; i < kVoices; i++) {
		if (_voices[i].channel == channel) {
			voiceOff(i);
			_voices[i].channel = -1;
			if (--voices == 0)
				return;
		}
	}
}

void MidiDriver_AdLib::donateVoices() {
	int freeVoices = 0;

	for (int i = 0; i < kVoices; i++)
		if (_voices[i].channel == -1)
			freeVoices++;

	if (freeVoices == 0)
		return;

	for (int i = 0; i < MIDI_CHANNELS; i++) {
		if (_channels[i].extraVoices >= freeVoices) {
			assignVoices(i, freeVoices);
			_channels[i].extraVoices -= freeVoices;
			return;
		} else if (_channels[i].extraVoices > 0) {
			assignVoices(i, _channels[i].extraVoices);
			freeVoices -= _channels[i].extraVoices;
			_channels[i].extraVoices = 0;
		}
	}
}

void MidiDriver_AdLib::renewNotes(int channel, bool key) {
	for (int i = 0; i < kVoices; i++) {
		// Update all notes playing this channel
		if ((channel == -1) || (_voices[i].channel == channel)) {
			if (_voices[i].note != -1)
				setNote(i, _voices[i].note, key);
		}
	}
}

void MidiDriver_AdLib::noteOn(int channel, int note, int velocity) {
	if (velocity == 0)
		return noteOff(channel, note);

	velocity >>= 1;

	// Check for playable notes
	if ((note < 12) || (note > 107))
		return;

	for (int i = 0; i < kVoices; i++) {
		if ((_voices[i].channel == channel) && (_voices[i].note == note)) {
			voiceOff(i);
			voiceOn(i, note, velocity);
			return;
		}
	}

#ifdef ADLIB_DISABLE_VOICE_MAPPING
	int voice = findVoiceBasic(channel);
#else
	int voice = findVoice(channel);
#endif

	if (voice == -1) {
		debug(3, "ADLIB: failed to find free voice assigned to channel %i", channel);
		return;
	}

	voiceOn(voice, note, velocity);
}

// FIXME: Temporary, see comment at top of file regarding ADLIB_DISABLE_VOICE_MAPPING
int MidiDriver_AdLib::findVoiceBasic(int channel) {
	int voice = -1;
	int oldestVoice = -1;
	int oldestAge = -1;

	// Try to find a voice assigned to this channel that is free (round-robin)
	for (int i = 0; i < kVoices; i++) {
		int v = (_channels[channel].lastVoice + i + 1) % kVoices;

		if (_voices[v].note == -1) {
			voice = v;
			break;
		}

		// We also keep track of the oldest note in case the search fails
		if (_voices[v].age > oldestAge) {
			oldestAge = _voices[v].age;
			oldestVoice = v;
		}
	}

	if (voice == -1) {
		if (oldestVoice != -1) {
			voiceOff(oldestVoice);
			voice = oldestVoice;
		} else {
			return -1;
		}
	}

	_voices[voice].channel = channel;
	_channels[channel].lastVoice = voice;
	return voice;
}

int MidiDriver_AdLib::findVoice(int channel) {
	int voice = -1;
	int oldestVoice = -1;
	uint32 oldestAge = 0;

	// Try to find a voice assigned to this channel that is free (round-robin)
	for (int i = 0; i < kVoices; i++) {
		int v = (_channels[channel].lastVoice + i + 1) % kVoices;

		if (_voices[v].channel == channel) {
			if (_voices[v].note == -1) {
				voice = v;
				break;
			}

			// We also keep track of the oldest note in case the search fails
			// Notes started in the current time slice will not be selected
			if (_voices[v].age > oldestAge) {
				oldestAge = _voices[v].age;
				oldestVoice = v;
			}
		}
	}

	if (voice == -1) {
		if (oldestVoice != -1) {
			voiceOff(oldestVoice);
			voice = oldestVoice;
		} else {
			return -1;
		}
	}

	_channels[channel].lastVoice = voice;
	return voice;
}

void MidiDriver_AdLib::noteOff(int channel, int note) {
	for (int i = 0; i < kVoices; i++) {
		if ((_voices[i].channel == channel) && (_voices[i].note == note)) {
			if (_channels[channel].holdPedal)
				_voices[i].isSustained = true;
			else
				voiceOff(i);
			return;
		}
	}
}

void MidiDriver_AdLib::voiceOn(int voice, int note, int velocity) {
	int channel = _voices[voice].channel;
	int patch;

	_voices[voice].age = 0;

	if ((channel == 9) && _rhythmKeyMap) {
		patch = CLIP(note, 27, 88) + 101;
	} else {
		patch = _channels[channel].patch;
	}

	// Set patch if different from current patch
	if (patch != _voices[voice].patch)
		setPatch(voice, patch);

	_voices[voice].velocity = velocity;
	setNote(voice, note, true);
}

void MidiDriver_AdLib::voiceOff(int voice) {
	_voices[voice].isSustained = false;
	setNote(voice, _voices[voice].note, 0);
	_voices[voice].note = -1;
	_voices[voice].age = 0;
}

void MidiDriver_AdLib::setNote(int voice, int note, bool key) {
	int channel = _voices[voice].channel;
	int n, fre, oct;
	float delta;
	int bend = _channels[channel].pitchWheel;

	if ((channel == 9) && _rhythmKeyMap) {
		note = _rhythmKeyMap[CLIP(note, 27, 88) - 27];
	}

	_voices[voice].note = note;

	n = note % 12;

	if (bend < 8192)
		bend = 8192 - bend;
	delta = (float)pow(2.0, (bend % 8192) / 8192.0);

	if (bend > 8192)
		fre = (int)(ym3812_note[n] * delta);
	else
		fre = (int)(ym3812_note[n] / delta);

	oct = note / 12 - 1;

	if (oct < 0)
		oct = 0;

	if (oct > 7)
		oct = 7;

	setRegister(0xA0 + voice, fre & 0xff);
	setRegister(0xB0 + voice, (key << 5) | (oct << 2) | (fre >> 8));

	setVelocity(voice);
}

void MidiDriver_AdLib::setVelocity(int voice) {
	AdLibPatch &patch = _patches[_voices[voice].patch];
	int pan = _channels[_voices[voice].channel].pan;
	setVelocityReg(registerOffset[voice] + 3, calcVelocity(voice, 1), patch.op[1].kbScaleLevel, pan);

	// In AM mode we need to set the level for both operators
	if (_patches[_voices[voice].patch].mod.algorithm == 1)
		setVelocityReg(registerOffset[voice], calcVelocity(voice, 0), patch.op[0].kbScaleLevel, pan);
}

int MidiDriver_AdLib::calcVelocity(int voice, int op) {
	if (_isSCI0) {
		int velocity = _masterVolume;

		if (velocity > 0)
			velocity += 3;

		if (velocity > 15)
			velocity = 15;

		int insVelocity;
		if (_channels[_voices[voice].channel].enableVelocity)
			insVelocity = _voices[voice].velocity;
		else
			insVelocity = 63 - _patches[_voices[voice].patch].op[op].totalLevel;

		// Note: Later SCI0 has a static table that is close to this formula, but not exactly the same.
		// Early SCI0 does (velocity * (insVelocity / 15))
		return velocity * insVelocity / 15;
	} else {
		AdLibOperator &oper = _patches[_voices[voice].patch].op[op];
		int velocity = _channels[_voices[voice].channel].volume + 1;
		velocity = velocity * (velocityMap1[_voices[voice].velocity] + 1) / 64;
		velocity = velocity * (_masterVolume + 1) / 16;

		if (--velocity < 0)
			velocity = 0;

		return velocityMap2[velocity] * (63 - oper.totalLevel) / 63;
	}
}

void MidiDriver_AdLib::setVelocityReg(int regOffset, int velocity, int kbScaleLevel, int pan) {
	if (!_playSwitch)
		velocity = 0;

	if (isStereo()) {
		int velLeft = velocity;
		int velRight = velocity;

		if (pan > 0x40)
			velLeft = velLeft * (0x7f - pan) / 0x3f;
		else if (pan < 0x40)
			velRight = velRight * pan / 0x40;

		setRegister(0x40 + regOffset, (kbScaleLevel << 6) | (63 - velLeft), kLeftChannel);
		setRegister(0x40 + regOffset, (kbScaleLevel << 6) | (63 - velRight), kRightChannel);
	} else {
		setRegister(0x40 + regOffset, (kbScaleLevel << 6) | (63 - velocity));
	}
}

void MidiDriver_AdLib::setPatch(int voice, int patch) {
	if ((patch < 0) || ((uint)patch >= _patches.size())) {
		warning("ADLIB: Invalid patch %i requested", patch);
		// Substitute instrument 0
		patch = 0;
	}

	_voices[voice].patch = patch;
	AdLibModulator &mod = _patches[patch].mod;

	// Set the common settings for both operators
	setOperator(registerOffset[voice], _patches[patch].op[0]);
	setOperator(registerOffset[voice] + 3, _patches[patch].op[1]);

	// Set the additional settings for the modulator
	byte algorithm = mod.algorithm ? 1 : 0;
	setRegister(0xC0 + voice, (mod.feedback << 1) | algorithm);
}

void MidiDriver_AdLib::setOperator(int reg, AdLibOperator &op) {
	setRegister(0x40 + reg, (op.kbScaleLevel << 6) | op.totalLevel);
	setRegister(0x60 + reg, (op.attackRate << 4) | op.decayRate);
	setRegister(0x80 + reg, (op.sustainLevel << 4) | op.releaseRate);
	setRegister(0x20 + reg, (op.amplitudeMod << 7) | (op.vibrato << 6)
				| (op.envelopeType << 5) | (op.kbScaleRate << 4) | op.frequencyMult);
	setRegister(0xE0 + reg, op.waveForm);
}

void MidiDriver_AdLib::setRegister(int reg, int value, int channels) {
	if (channels & kLeftChannel) {
		_opl->write(0x220, reg);
		_opl->write(0x221, value);
	}

	if (isStereo()) {
		if (channels & kRightChannel) {
			_opl->write(0x222, reg);
			_opl->write(0x223, value);
		}
	}
}

void MidiDriver_AdLib::playSwitch(bool play) {
	_playSwitch = play;
	renewNotes(-1, play);
}

bool MidiDriver_AdLib::loadResource(const byte *data, uint size) {
	if ((size != 1344) && (size != 2690) && (size != 5382)) {
		error("ADLIB: Unsupported patch format (%i bytes)", size);
		return false;
	}

	for (int i = 0; i < 48; i++)
		loadInstrument(data + (28 * i));

	if (size == 1344) {
		byte dummy[28] = {0};

		// Only 48 instruments, add dummies
		for (int i = 0; i < 48; i++)
			loadInstrument(dummy);
	} else if (size == 2690) {
		for (int i = 48; i < 96; i++)
			loadInstrument(data + 2 + (28 * i));
	} else {
		// SCI1.1 and later
		for (int i = 48; i < 190; i++)
			loadInstrument(data + (28 * i));
		_rhythmKeyMap = new byte[kRhythmKeys];
		memcpy(_rhythmKeyMap, data + 5320, kRhythmKeys);
	}

	return true;
}

uint32 MidiDriver_AdLib::property(int prop, uint32 param) {
	switch(prop) {
	case MIDI_PROP_MASTER_VOLUME:
		if (param != 0xffff)
			_masterVolume = param;
		return _masterVolume;
	default:
		break;
	}
	return 0;
}


int MidiPlayer_AdLib::open(ResourceManager *resMan) {
	assert(resMan != NULL);

	// Load up the patch.003 file, parse out the instruments
	Resource *res = resMan->findResource(ResourceId(kResourceTypePatch, 3), 0);
	bool ok = false;

	if (res) {
		ok = static_cast<MidiDriver_AdLib *>(_driver)->loadResource(res->data, res->size);
	} else {
		// Early SCI0 games have the sound bank embedded in the AdLib driver

		Common::File f;

		if (f.open("ADL.DRV")) {
			int size = f.size();
			const uint patchSize = 1344;

			// Note: Funseeker's Guide also has another version of adl.drv, 8803 bytes.
			// This isn't supported, but it's not really used anywhere, as that demo
			// doesn't have sound anyway.
			if ((size == 5684) || (size == 5720) || (size == 5727)) {
				byte *buf = new byte[patchSize];

				if (f.seek(0x45a) && (f.read(buf, patchSize) == patchSize))
					ok = static_cast<MidiDriver_AdLib *>(_driver)->loadResource(buf, patchSize);

				delete[] buf;
			}
		}
	}

	if (!ok) {
		warning("ADLIB: Failed to load patch.003");
		return -1;
	}

	return static_cast<MidiDriver_AdLib *>(_driver)->openAdLib(_version <= SCI_VERSION_0_LATE);
}

void MidiPlayer_AdLib::close() {
	if (_driver) {
		_driver->close();
	}
}

byte MidiPlayer_AdLib::getPlayId() const {
	switch (_version) {
	case SCI_VERSION_0_EARLY:
		return 0x01;
	case SCI_VERSION_0_LATE:
		return 0x04;
	default:
		return 0x00;
	}
}

MidiPlayer *MidiPlayer_AdLib_create(SciVersion _soundVersion) {
	return new MidiPlayer_AdLib(_soundVersion);
}

} // End of namespace Sci
