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

#if defined(__ANDROID__)

#include <dlfcn.h>

#include "common/debug.h"
#include "common/endian.h"
#include "common/textconsole.h"
#include "common/error.h"
#include "common/file.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "audio/audiostream.h"
#include "audio/mpu401.h"
#include "audio/musicplugin.h"
#include "audio/mixer.h"

//#define EAS_DUMPSTREAM

// NOTE:
// EAS's render function *only* accepts one mix buffer size. it's defined at
// compile time of the library and can be retrieved via EASLibConfig.bufSize
// (seen: 128 bytes).
// to avoid local intermediate buffers, this implementation insists on a fixed
// buffer size of the calling rate converter, which in return must be a
// multiple of EAS's. that may change if there're hickups because slower
// devices can't render fast enough

// from rate_arm.cpp
#define INTERMEDIATE_BUFFER_SIZE 512

// so far all android versions have the very same library version
#define EAS_LIBRARY "libsonivox.so"
#define EAS_KNOWNVERSION 0x03060a0e

#define EAS_REVERB 2
#define EAS_REVERB_BYPASS 0
#define EAS_REVERB_PRESET 1
#define EAS_REVERB_CHAMBER 2

class MidiDriver_EAS : public MidiDriver_MPU401, Audio::AudioStream {
public:
	MidiDriver_EAS();
	virtual ~MidiDriver_EAS();

	// MidiDriver
	virtual int open();
	virtual bool isOpen() const;
	virtual void close();
	virtual void send(uint32 b);
	virtual void sysEx(const byte *msg, uint16 length);
	virtual void setTimerCallback(void *timerParam,
								Common::TimerManager::TimerProc timerProc);
	virtual uint32 getBaseTempo();

	// AudioStream
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const;
	virtual int getRate() const;
	virtual bool endOfData() const;

private:
	struct EASLibConfig {
		uint32 version;
		uint32 debug;
		int32 voices;
		int32 channels;
		int32 rate;
		int32 bufSize;
		uint32 filter;
		uint32 timeStamp;
		char *GUID;
	};

	struct EASFile {
		const char *path;
		int fd;
		long long offset;
		long long length;
	};

	typedef void * EASDataHandle;
	typedef void * EASHandle;

	typedef EASLibConfig *(*ConfigFunc)();
	typedef int32 (*InitFunc)(EASDataHandle *);
	typedef int32 (*ShutdownFunc)(EASDataHandle);
	typedef int32 (*LoadDLSFunc)(EASDataHandle, EASHandle, EASFile *);
	typedef int32 (*SetParameterFunc)(EASDataHandle, int32, int32, int32);
	typedef int32 (*SetVolumeFunc)(EASDataHandle, EASHandle, int32);
	typedef int32 (*OpenStreamFunc)(EASDataHandle, EASHandle *, EASHandle);
	typedef int32 (*WriteStreamFunc)(EASDataHandle, EASHandle, byte *, int32);
	typedef int32 (*CloseStreamFunc)(EASDataHandle, EASHandle);
	typedef int32 (*RenderFunc)(EASDataHandle, int16 *, int32, int32 *);

	template<typename T>
	void sym(T &t, const char *symbol) {
		union {
			void *v;
			T t;
		} u;

		assert(sizeof(u.v) == sizeof(u.t));

		u.v = dlsym(_dlHandle, symbol);

		if (!u.v)
			warning("couldn't resolve %s from " EAS_LIBRARY, symbol);

		t = u.t;
	}

	void *_dlHandle;

	ConfigFunc _configFunc;
	InitFunc _initFunc;
	ShutdownFunc _shutdownFunc;
	LoadDLSFunc _loadDLSFunc;
	SetParameterFunc _setParameterFunc;
	SetVolumeFunc _setVolumeFunc;
	OpenStreamFunc _openStreamFunc;
	WriteStreamFunc _writeStreamFunc;
	CloseStreamFunc _closeStreamFunc;
	RenderFunc _renderFunc;

	const EASLibConfig *_config;
	EASDataHandle _EASHandle;
	EASHandle _midiStream;

	Common::TimerManager::TimerProc _timerProc;
	void *_timerParam;
	uint32 _baseTempo;
	uint _rounds;
	Audio::SoundHandle _soundHandle;

	Common::DumpFile _dump;
};

MidiDriver_EAS::MidiDriver_EAS() :
	MidiDriver_MPU401(),
	_dlHandle(0),
	_configFunc(0),
	_initFunc(0),
	_shutdownFunc(0),
	_loadDLSFunc(0),
	_setParameterFunc(0),
	_setVolumeFunc(0),
	_openStreamFunc(0),
	_writeStreamFunc(0),
	_closeStreamFunc(0),
	_renderFunc(0),
	_config(0),
	_EASHandle(0),
	_midiStream(0),
	_timerProc(0),
	_timerParam(0),
	_baseTempo(0),
	_rounds(0),
	_soundHandle(),
	_dump() {
}

MidiDriver_EAS::~MidiDriver_EAS() {
}

int MidiDriver_EAS::open() {
	if (isOpen())
		return MERR_ALREADY_OPEN;

	_dlHandle = dlopen(EAS_LIBRARY, RTLD_LAZY);
	if (!_dlHandle) {
		warning("error opening " EAS_LIBRARY ": %s", dlerror());
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	sym(_configFunc, "EAS_Config");
	if (!_configFunc) {
		close();
		return -1;
	}

	_config = _configFunc();
	if (!_config) {
		close();
		warning("error retrieving EAS library configuration");
		return -1;
	}

	if (_config->version != EAS_KNOWNVERSION) {
		close();
		warning("unknown EAS library version: 0x%08x", _config->version);
		return -1;
	}

	if (_config->channels > 2) {
		close();
		warning("unsupported number of EAS channels: %d", _config->channels);
		return -1;
	}

	// see note at top of this file
	if (INTERMEDIATE_BUFFER_SIZE % (_config->bufSize * _config->channels)) {
		close();
		warning("unsupported EAS buffer size: %d", _config->bufSize);
		return -1;
	}

	sym(_initFunc, "EAS_Init");
	sym(_shutdownFunc, "EAS_Shutdown");
	sym(_loadDLSFunc, "EAS_LoadDLSCollection");
	sym(_setParameterFunc, "EAS_SetParameter");
	sym(_setVolumeFunc, "EAS_SetVolume");
	sym(_openStreamFunc, "EAS_OpenMIDIStream");
	sym(_writeStreamFunc, "EAS_WriteMIDIStream");
	sym(_closeStreamFunc, "EAS_CloseMIDIStream");
	sym(_renderFunc, "EAS_Render");

	if (!_initFunc || !_shutdownFunc || !_loadDLSFunc || !_setParameterFunc ||
			!_openStreamFunc || !_writeStreamFunc || !_closeStreamFunc ||
			!_renderFunc) {
		close();
		return -1;
	}

	int32 res = _initFunc(&_EASHandle);
	if (res) {
		close();
		warning("error initializing the EAS library: %d", res);
		return -1;
	}

	res = _setParameterFunc(_EASHandle, EAS_REVERB, EAS_REVERB_PRESET,
							EAS_REVERB_CHAMBER);
	if (res)
		warning("error setting reverb preset: %d", res);

	res = _setParameterFunc(_EASHandle, EAS_REVERB, EAS_REVERB_BYPASS, 0);
	if (res)
		warning("error disabling reverb bypass: %d", res);

	// 90 is EAS's default, max is 100
	// so the option slider will only work from 0.1 to 1.1
	res = _setVolumeFunc(_EASHandle, 0, ConfMan.getInt("midi_gain") - 10);
	if (res)
		warning("error setting EAS master volume: %d", res);

	res = _openStreamFunc(_EASHandle, &_midiStream, 0);
	if (res) {
		close();
		warning("error opening EAS MIDI stream: %d", res);
		return -1;
	}

	// set the timer frequency to match a single buffer size
	_baseTempo = (1000000 * _config->bufSize) / _config->rate;

	// number of buffer fills per readBuffer()
	_rounds = INTERMEDIATE_BUFFER_SIZE / (_config->bufSize * _config->channels);

	debug("EAS initialized (voices:%d channels:%d rate:%d buffer:%d) "
			"tempo:%u rounds:%u", _config->voices, _config->channels,
			_config->rate, _config->bufSize, _baseTempo, _rounds);

	// TODO doesn't seem to work with midi streams?
	if (ConfMan.hasKey("soundfont")) {
		const Common::String dls = ConfMan.get("soundfont");

		debug("loading DLS file '%s'", dls.c_str());

		EASFile f;
		memset(&f, 0, sizeof(EASFile));
		f.path = dls.c_str();

		res = _loadDLSFunc(_EASHandle, 0, &f);
		if (res)
			warning("error loading DLS file '%s': %d", dls.c_str(), res);
		else
			debug("DLS file loaded");
	}

#ifdef EAS_DUMPSTREAM
	if (!_dump.open("/sdcard/eas.dump"))
		warning("error opening EAS dump file");
#endif

	g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType,
										&_soundHandle, this, -1,
										Audio::Mixer::kMaxChannelVolume, 0,
										DisposeAfterUse::NO, true);

	return 0;
}

bool MidiDriver_EAS::isOpen() const {
	return _dlHandle != 0;
}

void MidiDriver_EAS::close() {
	MidiDriver_MPU401::close();

	if (!isOpen())
		return;

	g_system->getMixer()->stopHandle(_soundHandle);

#ifdef EAS_DUMPSTREAM
	if (_dump.isOpen())
		_dump.close();
#endif

	// not pretty, but better than a mutex
	g_system->delayMillis((_baseTempo * _rounds) / 1000);

	if (_midiStream) {
		int32 res = _closeStreamFunc(_EASHandle, _midiStream);
		if (res)
			warning("error closing EAS MIDI stream: %d", res);

		_midiStream = 0;
	}

	if (_EASHandle) {
		int32 res = _shutdownFunc(_EASHandle);
		if (res)
			warning("error shutting down the EAS library: %d", res);

		_EASHandle = 0;
	}

	if (dlclose(_dlHandle))
		warning("error closing " EAS_LIBRARY ": %s", dlerror());

	_dlHandle = 0;
}

void MidiDriver_EAS::send(uint32 b) {
	byte buf[4];

	WRITE_LE_UINT32(buf, b);

	int32 len = 3;
	if ((buf[0] >> 4) == 0xC || (buf[0] >> 4) == 0xD)
		len = 2;

	int32 res = _writeStreamFunc(_EASHandle, _midiStream, buf, len);
	if (res)
		warning("error writing to EAS MIDI stream: %d", res);
}

void MidiDriver_EAS::sysEx(const byte *msg, uint16 length) {
	byte buf[266];

	assert(length + 2 <= ARRAYSIZE(buf));

	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	int32 res = _writeStreamFunc(_EASHandle, _midiStream, buf, length + 2);
	if (res)
		warning("error writing to EAS MIDI stream: %d", res);
}

void MidiDriver_EAS::setTimerCallback(void *timerParam,
								Common::TimerManager::TimerProc timerProc) {
	_timerParam = timerParam;
	_timerProc = timerProc;
}

uint32 MidiDriver_EAS::getBaseTempo() {
	return _baseTempo;
}

int MidiDriver_EAS::readBuffer(int16 *buffer, const int numSamples) {
	// see note at top of this file
	assert(numSamples == INTERMEDIATE_BUFFER_SIZE);

	int32 res, c;

	for (uint i = 0; i < _rounds; ++i) {
		// pull in MIDI events for exactly one buffer size
		if (_timerProc)
			(*_timerProc)(_timerParam);

		// if there are no MIDI events, this just renders silence
		res = _renderFunc(_EASHandle, buffer, _config->bufSize, &c);
		if (res) {
			warning("error rendering EAS samples: %d", res);
			return -1;
		}

#ifdef EAS_DUMPSTREAM
		if (_dump.isOpen())
			_dump.write(buffer, c * _config->channels * 2);
#endif

		buffer += c * _config->channels;
	}

	return numSamples;
}

bool MidiDriver_EAS::isStereo() const {
	return _config->channels == 2;
}

int MidiDriver_EAS::getRate() const {
	return _config->rate;
}

bool MidiDriver_EAS::endOfData() const {
	return false;
}

class EASMusicPlugin : public MusicPluginObject {
public:
	EASMusicPlugin();
	virtual ~EASMusicPlugin();

	const char *getName() const;
	const char *getId() const;
	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver,
									MidiDriver::DeviceHandle = 0) const;
};

EASMusicPlugin::EASMusicPlugin() {
}

EASMusicPlugin::~EASMusicPlugin() {
}

const char *EASMusicPlugin::getName() const {
	return "Embedded Audio Synthesis";
}

const char *EASMusicPlugin::getId() const {
	return "eas";
}

MusicDevices EASMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_GM));

	return devices;
}

Common::Error EASMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_EAS();

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(EAS)
	//REGISTER_PLUGIN_DYNAMIC(EAS, PLUGIN_TYPE_MUSIC, EASMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(EAS, PLUGIN_TYPE_MUSIC, EASMusicPlugin);
//#endif

#endif
