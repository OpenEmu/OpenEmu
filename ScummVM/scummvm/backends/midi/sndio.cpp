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

#if defined(USE_SNDIO)

#include "common/error.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <sndio.h>

////////////////////////////////////////
//
// sndio MIDI driver
//
////////////////////////////////////////

class MidiDriver_Sndio : public MidiDriver_MPU401 {
public:
	MidiDriver_Sndio();
	int open();
	bool isOpen() const { return hdl != NULL; }
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	struct mio_hdl *hdl;
};

MidiDriver_Sndio::MidiDriver_Sndio() {
	hdl = NULL;
}

int MidiDriver_Sndio::open() {
	if (hdl != NULL)
		return MERR_ALREADY_OPEN;

	hdl = ::mio_open(NULL, MIO_OUT, 0);
	if (hdl == NULL)
		warning("Could open MIDI port (no music)");
	return 0;
}

void MidiDriver_Sndio::close() {
	MidiDriver_MPU401::close();
	if (!hdl)
		return;
	mio_close(hdl);
	hdl = NULL;
}

void MidiDriver_Sndio::send(uint32 b) {
	unsigned char buf[4];
	unsigned int len;

	if (!hdl)
		return;
	buf[0] = b & 0xff;
	buf[1] = (b >> 8) & 0xff;
	buf[2] = (b >> 16) & 0xff;
	buf[3] = (b >> 24) & 0xff;
	switch (buf[0] & 0xf0) {
	case 0xf0:
		return;
	case 0xc0:
	case 0xd0:
		len = 2;
		break;
	default:
		len = 3;
	}
	mio_write(hdl, buf, len);
}

void MidiDriver_Sndio::sysEx(const byte *msg, uint16 length) {
	if (!hdl)
		return;

	unsigned char buf[266];

	assert(length + 2 <= ARRAYSIZE(buf));

	// Add SysEx frame
	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	mio_write(hdl, buf, length + 2);
}


// Plugin interface

class SndioMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "Sndio";
	}

	const char *getId() const {
		return "sndio";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices SndioMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error SndioMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_Sndio();

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(Sndio)
	//REGISTER_PLUGIN_DYNAMIC(SNDIO, PLUGIN_TYPE_MUSIC, SndioMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(SNDIO, PLUGIN_TYPE_MUSIC, SndioMusicPlugin);
//#endif

#endif
