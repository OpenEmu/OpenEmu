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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#ifdef MACOSX

#include <AvailabilityMacros.h>

// With the release of Mac OS X 10.5 in October 2007, Apple deprecated the
// AUGraphNewNode & AUGraphGetNodeInfo APIs in favor of the new AUGraphAddNode &
// AUGraphNodeInfo APIs. While it is easy to switch to those, it breaks
// compatibility with all pre-10.5 systems.
//
// Since 10.5 was the last system to support PowerPC, we use the old, deprecated
// APIs on PowerPC based systems by default. On all other systems (such as Mac
// OS X running on Intel hardware, or iOS running on ARM), we use the new API by
// default.
//
// This leaves Mac OS X 10.4 running on x86 processors as the only system
// combination that this code will not support by default. It seems quite
// reasonable to assume that anybody with an Intel system has since then moved
// on to a newer Mac OS X release. But if for some reason you absolutely need to
// build an x86 version of this code using the old, deprecated API, you can
// simply do so by manually enable the USE_DEPRECATED_COREAUDIO_API switch (e.g.
// by adding setting it suitably in CPPFLAGS).
#if !defined(USE_DEPRECATED_COREAUDIO_API)
	#if TARGET_CPU_PPC || TARGET_CPU_PPC64 || !defined(MAC_OS_X_VERSION_10_6)
		#define USE_DEPRECATED_COREAUDIO_API 1
	#else
		#define USE_DEPRECATED_COREAUDIO_API 0
	#endif
#endif

#if USE_DEPRECATED_COREAUDIO_API
	// Try to silence warnings about use of deprecated APIs
	#undef DEPRECATED_ATTRIBUTE
	#define DEPRECATED_ATTRIBUTE
#endif


#include "common/config-manager.h"
#include "common/error.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <CoreServices/CoreServices.h>
#include <AudioToolbox/AUGraph.h>


// Activating the following switch disables reverb support in the CoreAudio
// midi backend. Reverb will suck away a *lot* of CPU time, so on slower
// systems, you may want to turn it off completely.
// TODO: Maybe make this a config option?
//#define COREAUDIO_DISABLE_REVERB


// A macro to simplify error handling a bit.
#define RequireNoErr(error)                                         \
do {                                                                \
	err = error;                                                    \
	if (err != noErr)                                               \
		goto bail;                                                  \
} while (false)


/* CoreAudio MIDI driver
 * By Max Horn / Fingolfin
 * Based on code by Benjamin W. Zale
 */
class MidiDriver_CORE : public MidiDriver_MPU401 {
public:
	MidiDriver_CORE();
	~MidiDriver_CORE();
	int open();
	bool isOpen() const { return _auGraph != 0; }
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	AUGraph _auGraph;
	AudioUnit _synth;
};

MidiDriver_CORE::MidiDriver_CORE()
	: _auGraph(0) {
}

MidiDriver_CORE::~MidiDriver_CORE() {
	if (_auGraph) {
		AUGraphStop(_auGraph);
		DisposeAUGraph(_auGraph);
		_auGraph = 0;
	}
}

int MidiDriver_CORE::open() {
	OSStatus err = 0;

	if (isOpen())
		return MERR_ALREADY_OPEN;

	// Open the Music Device.
	RequireNoErr(NewAUGraph(&_auGraph));

	AUNode outputNode, synthNode;
#if USE_DEPRECATED_COREAUDIO_API
	ComponentDescription desc;
#else
	AudioComponentDescription desc;
#endif

	// The default output device
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;
#if USE_DEPRECATED_COREAUDIO_API
	RequireNoErr(AUGraphNewNode(_auGraph, &desc, 0, NULL, &outputNode));
#else
	RequireNoErr(AUGraphAddNode(_auGraph, &desc, &outputNode));
#endif

	// The built-in default (softsynth) music device
	desc.componentType = kAudioUnitType_MusicDevice;
	desc.componentSubType = kAudioUnitSubType_DLSSynth;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
#if USE_DEPRECATED_COREAUDIO_API
	RequireNoErr(AUGraphNewNode(_auGraph, &desc, 0, NULL, &synthNode));
#else
	RequireNoErr(AUGraphAddNode(_auGraph, &desc, &synthNode));
#endif

	// Connect the softsynth to the default output
	RequireNoErr(AUGraphConnectNodeInput(_auGraph, synthNode, 0, outputNode, 0));

	// Open and initialize the whole graph
	RequireNoErr(AUGraphOpen(_auGraph));
	RequireNoErr(AUGraphInitialize(_auGraph));

	// Get the music device from the graph.
#if USE_DEPRECATED_COREAUDIO_API
	RequireNoErr(AUGraphGetNodeInfo(_auGraph, synthNode, NULL, NULL, NULL, &_synth));
#else
	RequireNoErr(AUGraphNodeInfo(_auGraph, synthNode, NULL, &_synth));
#endif

	// Load custom soundfont, if specified
	if (ConfMan.hasKey("soundfont")) {
		const char *soundfont = ConfMan.get("soundfont").c_str();

		// TODO: We should really check whether the file contains an
		// actual soundfont...

#if USE_DEPRECATED_COREAUDIO_API
		// Before 10.5, we need to use kMusicDeviceProperty_SoundBankFSSpec
		FSRef	fsref;
		FSSpec	fsSpec;
		err = FSPathMakeRef ((const byte *)soundfont, &fsref, NULL);

		if (err == noErr) {
			err = FSGetCatalogInfo (&fsref, kFSCatInfoNone, NULL, NULL, &fsSpec, NULL);
		}

		if (err == noErr) {
			err = AudioUnitSetProperty (
				_synth,
				kMusicDeviceProperty_SoundBankFSSpec, kAudioUnitScope_Global,
				0,
				&fsSpec, sizeof(fsSpec)
			);
		}
#else
		// kMusicDeviceProperty_SoundBankFSSpec is present on 10.6+, but broken
		// kMusicDeviceProperty_SoundBankURL was added in 10.5 as a replacement
		CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8 *)soundfont, strlen(soundfont), false);

		if (url) {
			err = AudioUnitSetProperty (
				_synth,
				kMusicDeviceProperty_SoundBankURL, kAudioUnitScope_Global,
				0,
				&url, sizeof(url)
			);

			CFRelease(url);
		} else {
			warning("Failed to allocate CFURLRef from '%s'", soundfont);
		}
#endif

		if (err != noErr)
			error("Failed loading custom sound font '%s' (error %ld)", soundfont, (long)err);
	}

#ifdef COREAUDIO_DISABLE_REVERB
	// Disable reverb mode, as that sucks up a lot of CPU power, which can
	// be painful on low end machines.
	// TODO: Make this customizable via a config key?
	UInt32 usesReverb = 0;
	AudioUnitSetProperty (_synth, kMusicDeviceProperty_UsesInternalReverb,
		kAudioUnitScope_Global, 0, &usesReverb, sizeof (usesReverb));
#endif


	// Finally: Start the graph!
	RequireNoErr(AUGraphStart(_auGraph));

	return 0;

bail:
	if (_auGraph) {
		AUGraphStop(_auGraph);
		DisposeAUGraph(_auGraph);
		_auGraph = 0;
	}
	return MERR_CANNOT_CONNECT;
}

void MidiDriver_CORE::close() {
	MidiDriver_MPU401::close();
	if (_auGraph) {
		AUGraphStop(_auGraph);
		DisposeAUGraph(_auGraph);
		_auGraph = 0;
	}
}

void MidiDriver_CORE::send(uint32 b) {
	assert(isOpen());

	byte status_byte = (b & 0x000000FF);
	byte first_byte = (b & 0x0000FF00) >> 8;
	byte second_byte = (b & 0x00FF0000) >> 16;

	MusicDeviceMIDIEvent(_synth, status_byte, first_byte, second_byte, 0);
}

void MidiDriver_CORE::sysEx(const byte *msg, uint16 length) {
	unsigned char buf[266];

	assert(length + 2 <= ARRAYSIZE(buf));
	assert(isOpen());

	// Add SysEx frame
	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	// Send it
	MusicDeviceSysEx(_synth, buf, length+2);
}


// Plugin interface

class CoreAudioMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "CoreAudio";
	}

	const char *getId() const {
		return "core";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices CoreAudioMusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: Return a different music type depending on the configuration
	// TODO: List the available devices
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error CoreAudioMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_CORE();

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(COREAUDIO)
	//REGISTER_PLUGIN_DYNAMIC(COREAUDIO, PLUGIN_TYPE_MUSIC, CoreAudioMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(COREAUDIO, PLUGIN_TYPE_MUSIC, CoreAudioMusicPlugin);
//#endif

#endif // MACOSX
