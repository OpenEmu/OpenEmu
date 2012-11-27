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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "common/scummsys.h"

#ifdef USE_FLUIDSYNTH

#include "common/config-manager.h"
#include "common/error.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"
#include "audio/softsynth/emumidi.h"

#include <fluidsynth.h>

class MidiDriver_FluidSynth : public MidiDriver_Emulated {
private:
	MidiChannel_MPU401 _midiChannels[16];
	fluid_settings_t *_settings;
	fluid_synth_t *_synth;
	int _soundFont;
	int _outputRate;

protected:
	// Because GCC complains about casting from const to non-const...
	void setInt(const char *name, int val);
	void setNum(const char *name, double num);
	void setStr(const char *name, const char *str);

	void generateSamples(int16 *buf, int len);

public:
	MidiDriver_FluidSynth(Audio::Mixer *mixer);

	int open();
	void close();
	void send(uint32 b);

	MidiChannel *allocateChannel();
	MidiChannel *getPercussionChannel();

	// AudioStream API
	bool isStereo() const { return true; }
	int getRate() const { return _outputRate; }
};

// MidiDriver method implementations

MidiDriver_FluidSynth::MidiDriver_FluidSynth(Audio::Mixer *mixer)
	: MidiDriver_Emulated(mixer) {

	for (int i = 0; i < ARRAYSIZE(_midiChannels); i++) {
		_midiChannels[i].init(this, i);
	}

	// It ought to be possible to get FluidSynth to generate samples at
	// lower

	_outputRate = _mixer->getOutputRate();
	if (_outputRate < 22050)
		_outputRate = 22050;
	else if (_outputRate > 96000)
		_outputRate = 96000;
}

void MidiDriver_FluidSynth::setInt(const char *name, int val) {
	char *name2 = strdup(name);

	fluid_settings_setint(_settings, name2, val);
	free(name2);
}

void MidiDriver_FluidSynth::setNum(const char *name, double val) {
	char *name2 = strdup(name);

	fluid_settings_setnum(_settings, name2, val);
	free(name2);
}

void MidiDriver_FluidSynth::setStr(const char *name, const char *val) {
	char *name2 = strdup(name);
	char *val2 = strdup(val);

	fluid_settings_setstr(_settings, name2, val2);
	free(name2);
	free(val2);
}

int MidiDriver_FluidSynth::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	if (!ConfMan.hasKey("soundfont"))
		error("FluidSynth requires a 'soundfont' setting");

	_settings = new_fluid_settings();

	// The default gain setting is ridiculously low - at least for me. This
	// cannot be fixed by ScummVM's volume settings because they can only
	// soften the sound, not amplify it, so instead we add an option to
	// adjust the gain of FluidSynth itself.

	double gain = (double)ConfMan.getInt("midi_gain") / 100.0;

	setNum("synth.gain", gain);
	setNum("synth.sample-rate", _outputRate);

	_synth = new_fluid_synth(_settings);

	// In theory, this ought to reduce CPU load... but it doesn't make any
	// noticeable difference for me, so disable it for now.

	// fluid_synth_set_interp_method(_synth, -1, FLUID_INTERP_LINEAR);
	// fluid_synth_set_reverb_on(_synth, 0);
	// fluid_synth_set_chorus_on(_synth, 0);

	const char *soundfont = ConfMan.get("soundfont").c_str();

	_soundFont = fluid_synth_sfload(_synth, soundfont, 1);
	if (_soundFont == -1)
		error("Failed loading custom sound font '%s'", soundfont);

	MidiDriver_Emulated::open();

	// The MT-32 emulator uses kSFXSoundType here. I don't know why.
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_mixerSoundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
	return 0;
}

void MidiDriver_FluidSynth::close() {
	if (!_isOpen)
		return;
	_isOpen = false;

	_mixer->stopHandle(_mixerSoundHandle);

	if (_soundFont != -1)
		fluid_synth_sfunload(_synth, _soundFont, 1);

	delete_fluid_synth(_synth);
	delete_fluid_settings(_settings);
}

void MidiDriver_FluidSynth::send(uint32 b) {
	//byte param3 = (byte) ((b >> 24) & 0xFF);
	uint param2 = (byte) ((b >> 16) & 0xFF);
	uint param1 = (byte) ((b >>  8) & 0xFF);
	byte cmd    = (byte) (b & 0xF0);
	byte chan   = (byte) (b & 0x0F);

	switch (cmd) {
	case 0x80:	// Note Off
		fluid_synth_noteoff(_synth, chan, param1);
		break;
	case 0x90:	// Note On
		fluid_synth_noteon(_synth, chan, param1, param2);
		break;
	case 0xA0:	// Aftertouch
		break;
	case 0xB0:	// Control Change
		fluid_synth_cc(_synth, chan, param1, param2);
		break;
	case 0xC0:	// Program Change
		fluid_synth_program_change(_synth, chan, param1);
		break;
	case 0xD0:	// Channel Pressure
		break;
	case 0xE0:	// Pitch Bend
		fluid_synth_pitch_bend(_synth, chan, (param2 << 7) | param1);
		break;
	case 0xF0:	// SysEx
		// We should never get here! SysEx information has to be
		// sent via high-level semantic methods.
		warning("MidiDriver_FluidSynth: Receiving SysEx command on a send() call");
		break;
	default:
		warning("MidiDriver_FluidSynth: Unknown send() command 0x%02X", cmd);
		break;
	}
}

MidiChannel *MidiDriver_FluidSynth::allocateChannel() {
	for (int i = 0; i < ARRAYSIZE(_midiChannels); i++) {
		if (i != 9 && _midiChannels[i].allocate())
			return &_midiChannels[i];
	}
	return NULL;
}

MidiChannel *MidiDriver_FluidSynth::getPercussionChannel() {
	return &_midiChannels[9];
}

void MidiDriver_FluidSynth::generateSamples(int16 *data, int len) {
	fluid_synth_write_s16(_synth, len, data, 0, 2, data, 1, 2);
}


// Plugin interface

class FluidSynthMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "FluidSynth";
	}

	const char *getId() const {
		return "fluidsynth";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices FluidSynthMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error FluidSynthMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_FluidSynth(g_system->getMixer());

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(FLUIDSYNTH)
	//REGISTER_PLUGIN_DYNAMIC(FLUIDSYNTH, PLUGIN_TYPE_MUSIC, FluidSynthMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(FLUIDSYNTH, PLUGIN_TYPE_MUSIC, FluidSynthMusicPlugin);
//#endif

#endif
