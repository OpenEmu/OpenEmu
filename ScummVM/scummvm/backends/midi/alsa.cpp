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

#if defined(USE_ALSA)

#include "common/config-manager.h"
#include "common/error.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <alsa/asoundlib.h>

/*
 *     ALSA sequencer driver
 * Mostly cut'n'pasted from Virtual Tiny Keyboard (vkeybd) by Takashi Iwai
 *                                      (you really rox, you know?)
 */

#if SND_LIB_MAJOR >= 1 || SND_LIB_MINOR >= 6
#define snd_seq_flush_output(x) snd_seq_drain_output(x)
#define snd_seq_set_client_group(x,name)    /*nop */
#define my_snd_seq_open(seqp) snd_seq_open(seqp, "hw", SND_SEQ_OPEN_DUPLEX, 0)
#else
/* SND_SEQ_OPEN_OUT causes oops on early version of ALSA */
#define my_snd_seq_open(seqp) snd_seq_open(seqp, SND_SEQ_OPEN)
#endif

#define perm_ok(pinfo,bits) ((snd_seq_port_info_get_capability(pinfo) & (bits)) == (bits))

static int check_permission(snd_seq_port_info_t *pinfo) {
	if (perm_ok(pinfo, SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)) {
		if (!(snd_seq_port_info_get_capability(pinfo) & SND_SEQ_PORT_CAP_NO_EXPORT))
			return 1;
	}
	return 0;
}

/*
 * parse address string
 */

#define ADDR_DELIM      ".:"

class MidiDriver_ALSA : public MidiDriver_MPU401 {
public:
	MidiDriver_ALSA(int client, int port);
	int open();
	bool isOpen() const { return _isOpen; }
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	void send_event(int do_flush);
	bool _isOpen;
	snd_seq_event_t ev;
	snd_seq_t *seq_handle;
	int seq_client, seq_port;
	int my_client, my_port;
	// The volume controller value of the first MIDI channel
	int8 _channel0Volume;
};

MidiDriver_ALSA::MidiDriver_ALSA(int client, int port)
	: _isOpen(false), seq_handle(0), seq_client(client), seq_port(port), my_client(0), my_port(0), _channel0Volume(127) {
	memset(&ev, 0, sizeof(ev));
}

int MidiDriver_ALSA::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;
	_isOpen = true;

	if (my_snd_seq_open(&seq_handle) < 0) {
		error("Can't open sequencer");
		return -1;
	}

	my_client = snd_seq_client_id(seq_handle);
	if (snd_seq_set_client_name(seq_handle, "SCUMMVM") < 0) {
		error("Can't set sequencer client name");
	}
	snd_seq_set_client_group(seq_handle, "input");

	// According to http://www.alsa-project.org/~tiwai/alsa-subs.html
	// you can set read or write capabilities to allow other clients to
	// read or write the port. I don't think we need that, unless maybe
	// to be able to record the sound, but I can't get that to work even
	// with those capabilities.

	my_port = snd_seq_create_simple_port(seq_handle, "SCUMMVM port 0", 0,
	                                     SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);

	if (my_port < 0) {
		snd_seq_close(seq_handle);
		error("Can't create port");
		return -1;
	}

	if (seq_client != SND_SEQ_ADDRESS_SUBSCRIBERS) {
		// Subscribe to MIDI port. Prefer one that doesn't already have
		// any connections, unless we've forced a port number already.
		if (seq_port == -1) {
			snd_seq_client_info_t *cinfo;
			snd_seq_port_info_t *pinfo;

			snd_seq_client_info_alloca(&cinfo);
			snd_seq_port_info_alloca(&pinfo);

			snd_seq_get_any_client_info(seq_handle, seq_client, cinfo);

			int first_port = -1;
			int found_port = -1;

			snd_seq_port_info_set_client(pinfo, seq_client);
			snd_seq_port_info_set_port(pinfo, -1);
			while (found_port == -1 && snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
				if (check_permission(pinfo)) {
					if (first_port == -1)
						first_port = snd_seq_port_info_get_port(pinfo);
					if (found_port == -1 && snd_seq_port_info_get_write_use(pinfo) == 0)
						found_port = snd_seq_port_info_get_port(pinfo);
				}
			}

			if (found_port == -1) {
				// Should we abort here? For now, use the first
				// available port.
				seq_port = first_port;
				warning("MidiDriver_ALSA: All ports on client %d (%s) are already in use", seq_client, snd_seq_client_info_get_name(cinfo));
			} else {
				seq_port = found_port;
			}
		}

		if (snd_seq_connect_to(seq_handle, my_port, seq_client, seq_port) < 0) {
			error("Can't subscribe to MIDI port (%d:%d) see README for help", seq_client, seq_port);
		}
	}

	printf("Connected to Alsa sequencer client [%d:%d]\n", seq_client, seq_port);
	printf("ALSA client initialized [%d:0]\n", my_client);

	return 0;
}

void MidiDriver_ALSA::close() {
	if (_isOpen) {
		_isOpen = false;
		MidiDriver_MPU401::close();
		if (seq_handle)
			snd_seq_close(seq_handle);
	} else
		warning("MidiDriver_ALSA: Closing the driver before opening it");
}

void MidiDriver_ALSA::send(uint32 b) {
	if (!_isOpen) {
		warning("MidiDriver_ALSA: Got event while not open");
		return;
	}

	unsigned int midiCmd[4];
	ev.type = SND_SEQ_EVENT_OSS;

	midiCmd[3] = (b & 0xFF000000) >> 24;
	midiCmd[2] = (b & 0x00FF0000) >> 16;
	midiCmd[1] = (b & 0x0000FF00) >> 8;
	midiCmd[0] = (b & 0x000000FF);
	ev.data.raw32.d[0] = midiCmd[0];
	ev.data.raw32.d[1] = midiCmd[1];
	ev.data.raw32.d[2] = midiCmd[2];

	unsigned char chanID = midiCmd[0] & 0x0F;
	switch (midiCmd[0] & 0xF0) {
	case 0x80:
		snd_seq_ev_set_noteoff(&ev, chanID, midiCmd[1], midiCmd[2]);
		send_event(1);
		break;
	case 0x90:
		snd_seq_ev_set_noteon(&ev, chanID, midiCmd[1], midiCmd[2]);
		send_event(1);
		break;
	case 0xA0:
		snd_seq_ev_set_keypress(&ev, chanID, midiCmd[1], midiCmd[2]);
		send_event(1);
		break;
	case 0xB0:
		/* is it this simple ? Wow... */
		snd_seq_ev_set_controller(&ev, chanID, midiCmd[1], midiCmd[2]);

		// We save the volume of the first MIDI channel here to utilize it in
		// our workaround for broken USB-MIDI cables.
		if (chanID == 0 && midiCmd[1] == 0x07)
			_channel0Volume = midiCmd[2];

		send_event(1);
		break;
	case 0xC0:
		snd_seq_ev_set_pgmchange(&ev, chanID, midiCmd[1]);
		send_event(0);

		// Send a volume change command to work around a firmware bug in common
		// USB-MIDI cables. If the first MIDI command in a USB packet is a
		// Cx or Dx command, the second command in the packet is dropped
		// somewhere.
		send(0x07B0 | (_channel0Volume << 16));
		break;
	case 0xD0:
		snd_seq_ev_set_chanpress(&ev, chanID, midiCmd[1]);
		send_event(1);

		// Send a volume change command to work around a firmware bug in common
		// USB-MIDI cables. If the first MIDI command in a USB packet is a
		// Cx or Dx command, the second command in the packet is dropped
		// somewhere.
		send(0x07B0 | (_channel0Volume << 16));
		break;
	case 0xE0: {
		// long theBend = ((((long)midiCmd[1] + (long)(midiCmd[2] << 7))) - 0x2000) / 4;
		// snd_seq_ev_set_pitchbend(&ev, chanID, theBend);
		long theBend = ((long)midiCmd[1] + (long)(midiCmd[2] << 7)) - 0x2000;
		snd_seq_ev_set_pitchbend(&ev, chanID, theBend);
		send_event(1);
		} break;

	default:
		warning("Unknown MIDI Command: %08x", (int)b);
		/* I don't know if this works but, well... */
		send_event(1);
		break;
	}
}

void MidiDriver_ALSA::sysEx(const byte *msg, uint16 length) {
	if (!_isOpen) {
		warning("MidiDriver_ALSA: Got SysEx while not open");
		return;
	}

	unsigned char buf[266];

	assert(length + 2 <= ARRAYSIZE(buf));

	// Add SysEx frame
	buf[0] = 0xF0;
	memcpy(buf + 1, msg, length);
	buf[length + 1] = 0xF7;

	// Send it
	snd_seq_ev_set_sysex(&ev, length + 2, &buf);
	send_event(1);
}

void MidiDriver_ALSA::send_event(int do_flush) {
	snd_seq_ev_set_direct(&ev);
	snd_seq_ev_set_source(&ev, my_port);
	snd_seq_ev_set_dest(&ev, seq_client, seq_port);

	snd_seq_event_output(seq_handle, &ev);
	if (do_flush)
		snd_seq_flush_output(seq_handle);
}


// Plugin interface

class AlsaDevice {
public:
	AlsaDevice(Common::String name, MusicType mt, int client);
	Common::String getName();
	MusicType getType();
	int getClient();

private:
	Common::String _name;
	MusicType _type;
	int _client;
};

typedef Common::List<AlsaDevice> AlsaDevices;

AlsaDevice::AlsaDevice(Common::String name, MusicType mt, int client)
	: _name(name), _type(mt), _client(client) {
	// Make sure we do not get any trailing spaces to avoid problems when
	// storing the name in the configuration file.
	_name.trim();
}

Common::String AlsaDevice::getName() {
	return _name;
}

MusicType AlsaDevice::getType() {
	return _type;
}

int AlsaDevice::getClient() {
	return _client;
}

class AlsaMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "ALSA";
	}

	const char *getId() const {
		return "alsa";
	}

	AlsaDevices getAlsaDevices() const;
	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;

private:
	static int parse_addr(const char *arg, int *client, int *port);
};

AlsaDevices AlsaMusicPlugin::getAlsaDevices() const {
	AlsaDevices devices;
	snd_seq_t *seq_handle;
	if (my_snd_seq_open(&seq_handle) < 0)
		return devices; // can't open sequencer

	snd_seq_client_info_t *cinfo;
	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_t *pinfo;
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	while (snd_seq_query_next_client(seq_handle, cinfo) >= 0) {
		bool found_valid_port = false;

		/* reset query info */
		snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
		snd_seq_port_info_set_port(pinfo, -1);
		while (!found_valid_port && snd_seq_query_next_port(seq_handle, pinfo) >= 0) {
			if (check_permission(pinfo)) {
				found_valid_port = true;

				const char *name = snd_seq_client_info_get_name(cinfo);
				// TODO: Can we figure out the appropriate music type?
				MusicType type = MT_GM;
				int client = snd_seq_client_info_get_client(cinfo);
				devices.push_back(AlsaDevice(name, type, client));
			}
		}
	}
	snd_seq_close(seq_handle);

	return devices;
}

MusicDevices AlsaMusicPlugin::getDevices() const {
	MusicDevices devices;
	AlsaDevices::iterator d;

	AlsaDevices alsaDevices = getAlsaDevices();

	// Since the default behavior is to use the first device in the list,
	// try to put something sensible there. We used to have 17:0 and 65:0
	// as defaults.

	for (d = alsaDevices.begin(); d != alsaDevices.end();) {
		const int client = d->getClient();

		if (client == 17 || client == 65) {
			devices.push_back(MusicDevice(this, d->getName(), d->getType()));
			d = alsaDevices.erase(d);
		} else {
			++d;
		}
	}

	// 128:0 is probably TiMidity, or something like that, so that's
	// probably a good second choice.

	for (d = alsaDevices.begin(); d != alsaDevices.end();) {
		if (d->getClient() == 128) {
			devices.push_back(MusicDevice(this, d->getName(), d->getType()));
			d = alsaDevices.erase(d);
		} else {
			++d;
		}
	}

	// Add the remaining devices in the order they were found.

	for (d = alsaDevices.begin(); d != alsaDevices.end(); ++d)
		devices.push_back(MusicDevice(this, d->getName(), d->getType()));

	return devices;
}

Common::Error AlsaMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle dev) const {
	bool found = false;
	int seq_client, seq_port;

	const char *var = NULL;

	// TODO: Upgrade from old alsa_port setting. This probably isn't the
	// right place to do that, though.

	if (ConfMan.hasKey("alsa_port")) {
		warning("AlsaMusicPlugin: Found old 'alsa_port' setting, which will be ignored");
	}

	// The SCUMMVM_PORT environment variable can still be used to override
	// any config setting.

	var = getenv("SCUMMVM_PORT");
	if (var) {
		warning("AlsaMusicPlugin: SCUMMVM_PORT environment variable overrides config settings");
		if (parse_addr(var, &seq_client, &seq_port) >= 0) {
			found = true;
		} else {
			warning("AlsaMusicPlugin: Invalid port %s, using config settings instead", var);
		}
	}

	// Try to match the setting to an available ALSA device.

	if (!found && dev) {
		AlsaDevices alsaDevices = getAlsaDevices();

		for (AlsaDevices::iterator d = alsaDevices.begin(); d != alsaDevices.end(); ++d) {
			MusicDevice device(this, d->getName(), d->getType());

			if (device.getCompleteId().equals(MidiDriver::getDeviceString(dev, MidiDriver::kDeviceId))) {
				found = true;
				seq_client = d->getClient();
				seq_port = -1;
				break;
			}
		}
	}

	// Still nothing? Try a sensible default.

	if (!found) {
		// TODO: What's a sensible default anyway? And exactly when do
		// we get to this case?

		warning("AlsaMusicPlugin: Using 17:0 as default ALSA port");
		seq_client = 17;
		seq_port = 0;
	}

	*mididriver = new MidiDriver_ALSA(seq_client, seq_port);

	return Common::kNoError;
}

int AlsaMusicPlugin::parse_addr(const char *arg, int *client, int *port) {
	const char *p;

	if (isdigit(*arg)) {
		if ((p = strpbrk(arg, ADDR_DELIM)) == NULL)
			return -1;
		*client = atoi(arg);
		*port = atoi(p + 1);
	} else {
		if (*arg == 's' || *arg == 'S') {
			*client = SND_SEQ_ADDRESS_SUBSCRIBERS;
			*port = 0;
		} else
			return -1;
	}
	return 0;
}

//#if PLUGIN_ENABLED_DYNAMIC(ALSA)
	//REGISTER_PLUGIN_DYNAMIC(ALSA, PLUGIN_TYPE_MUSIC, AlsaMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(ALSA, PLUGIN_TYPE_MUSIC, AlsaMusicPlugin);
//#endif

#endif
