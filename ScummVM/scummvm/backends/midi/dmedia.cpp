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

/*
 * IRIX dmedia support by Rainer Canavan <scumm@canavan.de>
 *    some code liberated from seq.cpp and coremidi.cpp
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(IRIX)

#include "common/config-manager.h"
#include "common/error.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <dmedia/midi.h>
#include <sys/types.h>
#include <bstring.h>
#include <unistd.h>

////////////////////////////////////////
//
// IRIX dmedia midi driver
//
////////////////////////////////////////

#define SEQ_MIDIPUTC 5

class MidiDriver_DMEDIA : public MidiDriver_MPU401 {
public:
	MidiDriver_DMEDIA();
	int open();
	bool isOpen() const { return _isOpen; }
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	bool _isOpen;
	int _deviceNum;
	char *_midiportName;
	MDport _midiPort;
	int _fd;
};

MidiDriver_DMEDIA::MidiDriver_DMEDIA() {
	_isOpen = false;
	_deviceNum = 0;
	_midiportName = NULL;
}

int MidiDriver_DMEDIA::open() {
	int numinterfaces;
	int i;
	const char *var;
	char *portName;

	if (_isOpen)
		return MERR_ALREADY_OPEN;
	_isOpen = true;

	numinterfaces = mdInit();
	if (numinterfaces <= 0) {
		fprintf(stderr, "No MIDI interfaces configured.\n");
		perror("Cannot initialize libmd for sound output");
		return -1;
	}

	if (getenv("SCUMMVM_MIDIPORT")) {
		_deviceNum = atoi(getenv("SCUMMVM_MIDIPORT"));
		_midiportName = mdGetName(_deviceNum);
	} else {
		var = ConfMan.get("dmedia_port").c_str();
		if (strlen(var) > 0) {
			for (i = 0; i < numinterfaces; i++) {
				portName = mdGetName(i);
				if (strcmp(var, portName) == 0) {
					_deviceNum = i;
					_midiportName = portName;
				}
			}

		}
	}

	_midiPort = mdOpenOutPort(_midiportName);
	if (!_midiPort) {
		warning("Failed to open MIDI interface %s", _midiportName);
		return -1;
	}

	_fd = mdGetFd(_midiPort);
	if (!_fd) {
		warning("Failed to aquire filehandle for MIDI port %s", _midiportName);
		mdClosePort(_midiPort);
		return -1;
	}

	mdSetStampMode(_midiPort, MD_NOSTAMP);  /* don't use Timestamps */

	return 0;
}

void MidiDriver_DMEDIA::close() {
	mdClosePort(_midiPort);
	_isOpen = false;
	_deviceNum = 0;
	_midiportName = NULL;
}

void MidiDriver_DMEDIA::send(uint32 b) {
	MDevent event;
	byte status_byte = (b & 0x000000FF);
	byte first_byte = (b & 0x0000FF00) >> 8;
	byte second_byte = (b & 0x00FF0000) >> 16;


	event.sysexmsg = NULL;
	event.msg[0] = status_byte;
	event.msg[1] = first_byte;
	event.msg[2] = second_byte;

	switch (status_byte & 0xF0) {
	case 0x80:      // Note Off
	case 0x90:      // Note On
	case 0xA0:      // Polyphonic Aftertouch
	case 0xB0:      // Controller Change
	case 0xE0:      // Pitch Bending
		event.msglen = 3;
		break;
	case 0xC0:      // Programm Change
	case 0xD0:      // Monophonic Aftertouch
		event.msglen = 2;
		break;
	default:
		warning("DMediaMIDI driver encountered unsupported status byte: 0x%02x", status_byte);
		event.msglen = 3;
		break;
	}
	if (mdSend(_midiPort, &event, 1) != 1) {
		warning("failed sending MIDI event (dump follows...)");
		warning("MIDI Event (len=%u):", event.msglen);
		for (int i = 0; i < event.msglen; i++) warning("%02x ", (int)event.msg[i]);
	}
}

void MidiDriver_DMEDIA::sysEx (const byte *msg, uint16 length) {
	MDevent event;
	char buf [1024];

	assert(length + 2 <= 256);

	memcpy(buf, msg, length);
	buf[length] = MD_EOX;
	event.sysexmsg = buf;
	event.msglen = length;
	event.msg[0] = MD_SYSEX;
	event.msg[1] = 0;
	event.msg[2] = 0;

	if (mdSend(_midiPort, &event, 1) != 1) {
		fprintf(stderr, "failed sending MIDI SYSEX event (dump follows...)\n");
		for (int i = 0; i < event.msglen; i++) warning("%02x ", (int)event.msg[i]);
	}
}


// Plugin interface

class DMediaMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "DMedia";
	}

	const char *getId() const {
		return "dmedia";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices DMediaMusicPlugin::getDevices() const {
	int numinterfaces;
	int i;
	char *portName;
	MusicDevices devices;

	// TODO: Return a different music type depending on the configuration

	numinterfaces = mdInit();
	if (numinterfaces <= 0) {
		fprintf(stderr, "No MIDI interfaces configured.\n");
	}

	for (i=0; i<numinterfaces; i++) {
		portName = mdGetName(0);
		fprintf(stderr, "device %i %s\n", i, portName);
		devices.push_back(MusicDevice(this, portName, MT_GM));
	}

	return devices;
}

Common::Error DMediaMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_DMEDIA();

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(DMEDIA)
	//REGISTER_PLUGIN_DYNAMIC(DMEDIA, PLUGIN_TYPE_MUSIC, DMediaMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(DMEDIA, PLUGIN_TYPE_MUSIC, DMediaMusicPlugin);
//#endif

#endif
