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
 * Raw output support by Michael Pearce
 * Alsa support by Nicolas Noble <nicolas@nobis-crew.org> copied from
 *    both the QuickTime support and (vkeybd http://www.alsa-project.org/~iwai/alsa.html)
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(USE_SEQ_MIDI)

#include "common/error.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

////////////////////////////////////////
//
// Unix dev/sequencer driver
//
////////////////////////////////////////

#define SEQ_MIDIPUTC 5

class MidiDriver_SEQ : public MidiDriver_MPU401 {
public:
	MidiDriver_SEQ();
	int open();
	bool isOpen() const { return _isOpen; }
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	bool _isOpen;
	int device, _device_num;
};

MidiDriver_SEQ::MidiDriver_SEQ() {
	_isOpen = false;
	device = 0;
	_device_num = 0;
}

int MidiDriver_SEQ::open() {
	char *device_name;
	char dev_seq[] = "/dev/sequencer";

	if (_isOpen)
		return MERR_ALREADY_OPEN;
	_isOpen = true;
	device = 0;

	device_name = getenv("SCUMMVM_MIDI");

	if (device_name == NULL) {
		warning("SCUMMVM_MIDI environment variable not set, using /dev/sequencer");
		device_name = dev_seq;
	}

	device = ::open((device_name), O_RDWR, 0);

	if ((device_name == NULL) || (device < 0)) {
		if (device_name == NULL)
			warning("Opening /dev/null (no music will be heard)");
		else
			warning("Cannot open rawmidi device %s - using /dev/null (no music will be heard)",
							device_name);
		device = (::open(("/dev/null"), O_RDWR, 0));
		if (device < 0)
			error("Cannot open /dev/null to dump midi output");
	}

	if (getenv("SCUMMVM_MIDIPORT"))
		_device_num = atoi(getenv("SCUMMVM_MIDIPORT"));
	return 0;
}

void MidiDriver_SEQ::close() {
	MidiDriver_MPU401::close();
	::close(device);
	_isOpen = false;
}

void MidiDriver_SEQ::send(uint32 b) {
	unsigned char buf[256];
	int position = 0;

	switch (b & 0xF0) {
	case 0x80:
	case 0x90:
	case 0xA0:
	case 0xB0:
	case 0xE0:
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)b;
		buf[position++] = _device_num;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 8) & 0x7F);
		buf[position++] = _device_num;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 16) & 0x7F);
		buf[position++] = _device_num;
		buf[position++] = 0;
		break;
	case 0xC0:
	case 0xD0:
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)b;
		buf[position++] = _device_num;
		buf[position++] = 0;
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char)((b >> 8) & 0x7F);
		buf[position++] = _device_num;
		buf[position++] = 0;
		break;
	default:
		warning("MidiDriver_SEQ::send: unknown: %08x", (int)b);
		break;
	}
	if (write(device, buf, position) == -1)
		warning("MidiDriver_SEQ::send: write failed (%s)", strerror(errno));
}

void MidiDriver_SEQ::sysEx(const byte *msg, uint16 length) {
	unsigned char buf [266*4];
	int position = 0;
	const byte *chr = msg;

	assert(length + 2 <= 266);

	buf[position++] = SEQ_MIDIPUTC;
	buf[position++] = 0xF0;
	buf[position++] = _device_num;
	buf[position++] = 0;
	for (; length; --length, ++chr) {
		buf[position++] = SEQ_MIDIPUTC;
		buf[position++] = (unsigned char) *chr & 0x7F;
		buf[position++] = _device_num;
		buf[position++] = 0;
	}
	buf[position++] = SEQ_MIDIPUTC;
	buf[position++] = 0xF7;
	buf[position++] = _device_num;
	buf[position++] = 0;

	if (write(device, buf, position) == -1)
		warning("MidiDriver_SEQ::send: write failed (%s)", strerror(errno));
}


// Plugin interface

class SeqMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "SEQ";
	}

	const char *getId() const {
		return "seq";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices SeqMusicPlugin::getDevices() const {
	MusicDevices devices;
	// TODO: Return a different music type depending on the configuration
	// TODO: List the available devices
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error SeqMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_SEQ();

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(SEQ)
	//REGISTER_PLUGIN_DYNAMIC(SEQ, PLUGIN_TYPE_MUSIC, SeqMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(SEQ, PLUGIN_TYPE_MUSIC, SeqMusicPlugin);
//#endif

#endif
