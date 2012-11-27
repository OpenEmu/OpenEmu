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

#if defined(__amigaos4__)

#include "common/textconsole.h"
#include "common/error.h"
#include "common/endian.h"
#include "common/util.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <proto/camd.h>
#include <proto/exec.h>

/*
 * CAMD sequencer driver
 * Mostly cut'n'pasted from FreeSCI by Christoph Reichenbach
 */

class MidiDriver_CAMD : public MidiDriver_MPU401 {
public:
	MidiDriver_CAMD();
	int open();
	bool isOpen() const { return _isOpen; }
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	bool _isOpen;
	struct Library *_CamdBase;
	struct CamdIFace *_ICamd;
	struct MidiLink *_midi_link;

	char *getDevice();
	void closeAll();
};

MidiDriver_CAMD::MidiDriver_CAMD()
	: _isOpen(false), _CamdBase(NULL), _ICamd(NULL), _midi_link(NULL) {
}

int MidiDriver_CAMD::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	_CamdBase = IExec->OpenLibrary("camd.library", 36L);
	if (!_CamdBase) {
		error("Could not open 'camd.library'");
		return -1;
	}

	_ICamd = (struct CamdIFace *) IExec->GetInterface(_CamdBase, "main", 1, NULL);
	if (!_ICamd) {
		closeAll();
		error("Error while retrieving CAMD interface");
		return -1;
	}

	struct MidiNode *midi_node;
	midi_node = _ICamd->CreateMidi(MIDI_MsgQueue, 0L, MIDI_SysExSize, 4096L, MIDI_Name, "scummvm", TAG_END);
	if (!midi_node) {
		closeAll();
		error("Could not create CAMD MIDI node");
		return -1;
	}

	char *devicename = getDevice();
	if (!devicename) {
		closeAll();
		error("Could not find an output device");
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	_midi_link = _ICamd->AddMidiLink(midi_node, MLTYPE_Sender, MLINK_Location, devicename, TAG_END);
	if (!_midi_link) {
		closeAll();
		error("Could not create CAMD MIDI link to '%s'", devicename);
		return MERR_CANNOT_CONNECT;
	}

	_isOpen = true;
	return 0;
}

void MidiDriver_CAMD::close() {
	MidiDriver_MPU401::close();
	closeAll();
}

void MidiDriver_CAMD::send(uint32 b) {
	if (!_isOpen) {
		warning("MidiDriver_CAMD: Got event while not open");
		return;
	}

	ULONG data = READ_LE_UINT32(&b);
	_ICamd->PutMidi(_midi_link, data);
}

void MidiDriver_CAMD::sysEx(const byte *msg, uint16 length) {
	if (!_isOpen) {
		warning("MidiDriver_CAMD: Got SysEx while not open");
		return;
	}

	unsigned char buf[266];

	assert(length + 2 <= ARRAYSIZE(buf));

	// Add SysEx frame
	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	// Send it
	_ICamd->PutSysEx(_midi_link, buf);
}

char *MidiDriver_CAMD::getDevice() {
	char *retname = NULL;

	APTR key = _ICamd->LockCAMD(CD_Linkages);
	if (key != NULL) {
		struct MidiCluster *cluster = _ICamd->NextCluster(NULL);

		while (cluster && !retname) {
			// Get the current cluster name
			char *dev = cluster->mcl_Node.ln_Name;

			if (strstr(dev, "out") != NULL) {
				// This is an output device, return this
				retname = dev;
			} else {
				// Search the next one
				cluster = _ICamd->NextCluster(cluster);
			}
		}

		_ICamd->UnlockCAMD(key);
	}

	return retname;
}

void MidiDriver_CAMD::closeAll() {
	if (_ICamd)
		IExec->DropInterface((struct Interface *)_ICamd);
	if (_CamdBase)
		IExec->CloseLibrary(_CamdBase);

	_isOpen = false;
}


// Plugin interface

class CamdMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "CAMD";
	}

	const char *getId() const {
		return "camd";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices CamdMusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: Return a different music type depending on the configuration
	// TODO: List the available devices
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error CamdMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_CAMD();

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(CAMD)
	//REGISTER_PLUGIN_DYNAMIC(CAMD, PLUGIN_TYPE_MUSIC, CamdMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(CAMD, PLUGIN_TYPE_MUSIC, CamdMusicPlugin);
//#endif

#endif
