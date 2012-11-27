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
 * Raw MIDI output for the Atari ST line of computers.
 * Based on the ScummVM SEQ & CoreMIDI drivers.
 * Atari code by Keith Scroggins
 * We, unfortunately, could not use the SEQ driver because the /dev/midi under
 * FreeMiNT (and hence in libc) is considered to be a serial port for machine
 * access.  So, we just use OS calls then to send the data to the MIDI ports
 * directly.  The current implementation is sending 1 byte at a time because
 * in most cases we are only sending up to 3 bytes, I believe this saves a few
 * cycles.  I might change so sysex messages are sent the other way later.
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(__MINT__)

#include <osbind.h>
#include "audio/mpu401.h"
#include "common/util.h"
#include "audio/musicplugin.h"

class MidiDriver_STMIDI : public MidiDriver_MPU401 {
public:
	MidiDriver_STMIDI() : _isOpen (false) { }
	int open();
	bool isOpen() const { return _isOpen; }
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	bool _isOpen;
};

int MidiDriver_STMIDI::open() {
	if (_isOpen && (!Bcostat(4)))
		return MERR_ALREADY_OPEN;
	warning("ST Midi Port Open");
	_isOpen = true;
	return 0;
}

void MidiDriver_STMIDI::close() {
	MidiDriver_MPU401::close();
	_isOpen = false;
}

void MidiDriver_STMIDI::send(uint32 b) {

	byte status_byte = (b & 0x000000FF);
	byte first_byte = (b & 0x0000FF00) >> 8;
	byte second_byte = (b & 0x00FF0000) >> 16;

//	warning("ST MIDI Packet sent");

	switch (b & 0xF0) {
	case 0x80:	// Note Off
	case 0x90:	// Note On
	case 0xA0:	// Polyphonic Key Pressure
	case 0xB0:	// Controller
	case 0xE0:	// Pitch Bend
		Bconout(3, status_byte);
		Bconout(3, first_byte);
		Bconout(3, second_byte);
		break;
	case 0xC0:	// Program Change
	case 0xD0:	// Aftertouch
		Bconout(3, status_byte);
		Bconout(3, first_byte);
		break;
	default:
		fprintf(stderr, "Unknown : %08x\n", (int)b);
		break;
	}
}

void MidiDriver_STMIDI::sysEx (const byte *msg, uint16 length) {
	// FIXME: LordHoto doesn't know if this will still work
	// when sending 264 byte sysEx data, as needed by KYRA,
	// feel free to revert it to 254 again if needed.
	if (length > 264) {
		warning ("Cannot send SysEx block - data too large");
		return;
	}

	const byte *chr = msg;
	warning("Sending SysEx Message");

	Bconout(3, '0xF0');
	for (; length; --length, ++chr) {
		Bconout(3,((unsigned char) *chr & 0x7F));
	}
	Bconout(3, '0xF7');
}

// Plugin interface

class StMidiMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "STMIDI";
	}

	const char *getId() const {
		return "stmidi";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices StMidiMusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: Return a different music type depending on the configuration
	// TODO: List the available devices
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error StMidiMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_STMIDI();

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(STMIDI)
	//REGISTER_PLUGIN_DYNAMIC(STMIDI, PLUGIN_TYPE_MUSIC, StMidiMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(STMIDI, PLUGIN_TYPE_MUSIC, StMidiMusicPlugin);
//#endif

#endif
