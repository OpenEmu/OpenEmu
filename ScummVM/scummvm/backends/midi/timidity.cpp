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
 * Output to TiMidity++ MIDI server support
 *                            by Dmitry Marakasov <amdmi3@amdmi3.ru>
 * based on:
 * - Raw output support (seq.cpp) by Michael Pearce
 * - Pseudo /dev/sequencer of TiMidity (timidity-io.c)
 *                                 by Masanao Izumo <mo@goice.co.jp>
 * - sys/soundcard.h by Hannu Savolainen (got from my FreeBSD
 *   distribution, for which it was modified by Luigi Rizzo)
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"

#if defined(USE_TIMIDITY)

#include "common/endian.h"
#include "common/error.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "audio/musicplugin.h"
#include "audio/mpu401.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netdb.h>		/* for gethostbyname */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

// WORKAROUND bug #1870304: Solaris does not provide INADDR_NONE.
#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif

// BeOS BONE uses snooze (x/1000) in place of usleep(x)
#ifdef __BEOS__
#define usleep(v) snooze(v/1000)
#endif


#define SEQ_MIDIPUTC 5

#define TIMIDITY_LOW_DELAY

#ifdef TIMIDITY_LOW_DELAY
#define BUF_LOW_SYNC	0.1
#define BUF_HIGH_SYNC	0.15
#else
#define BUF_LOW_SYNC	0.4
#define BUF_HIGH_SYNC	0.8
#endif

/* default host & port */
#define DEFAULT_TIMIDITY_HOST "127.0.0.1"
#define DEFAULT_TIMIDITY_PORT 7777

class MidiDriver_TIMIDITY : public MidiDriver_MPU401 {
public:
	MidiDriver_TIMIDITY();

	int open();
	bool isOpen() const { return _isOpen; }
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);

private:
	/* standart routine to extract ip address from a string */
	in_addr_t host_to_addr(const char* address);

	/* creates a tcp connection to TiMidity server, returns filedesc (like open()) */
	int connect_to_server(const char* hostname, unsigned short tcp_port);

	/* send command to the server; printf-like; returns reply string */
	char *timidity_ctl_command(const char *fmt, ...) GCC_PRINTF(2, 3);

	/* timidity data socket-related stuff */
	void timidity_meta_seq(int p1, int p2, int p3);
	int timidity_sync(int centsec);
	int timidity_eot();

	/* write() analogue for any midi data */
	void timidity_write_data(const void *buf, size_t nbytes);

	/* get single line of server reply on control connection */
	int fdgets(char *buff, size_t buff_size);

	/* teardown connection to server */
	void teardown();

	/* close (if needed) and nullify both control and data filedescs */
	void close_all();

private:
	bool _isOpen;
	int _device_num;

	int _control_fd;
	int _data_fd;

	/* buffer for partial data read from _control_fd - from timidity-io.c, see fdgets() */
	char _controlbuffer[BUFSIZ];
	int _controlbuffer_count;	/* beginning of read pointer */
	int _controlbuffer_size;	/* end of read pointer */
};

MidiDriver_TIMIDITY::MidiDriver_TIMIDITY() {
	_isOpen = false;
	_device_num = 0;

	/* init fd's */
	_control_fd = _data_fd = -1;

	/* init buffer for control connection */
	_controlbuffer_count = _controlbuffer_size = 0;
}

int MidiDriver_TIMIDITY::open() {
	char *res;
	char timidity_host[MAXHOSTNAMELEN];
	int timidity_port, data_port, i;

	/* count ourselves open */
	if (_isOpen)
		return MERR_ALREADY_OPEN;
	_isOpen = true;

	/* get server hostname; if not specified in env, use default */
	if ((res = getenv("TIMIDITY_HOST")) == NULL)
		Common::strlcpy(timidity_host, DEFAULT_TIMIDITY_HOST, sizeof(timidity_host));
	else
		Common::strlcpy(timidity_host, res, sizeof(timidity_host));

	/* extract control port */
	if ((res = strrchr(timidity_host, ':')) != NULL) {
		*res++ = '\0';
		timidity_port = atoi(res);
	} else {
		timidity_port = DEFAULT_TIMIDITY_PORT;
	}

	/*
	 * create control connection to the server
	 */
	if ((_control_fd = connect_to_server(timidity_host, timidity_port)) < 0) {
		warning("TiMidity: can't open control connection (host=%s, port=%d)", timidity_host, timidity_port);
		return -1;
	}

	/* should read greeting issued by server upon connect:
	 * "220 TiMidity++ v2.13.2 ready)" */
	res = timidity_ctl_command(NULL);
	if (atoi(res) != 220) {
		warning("TiMidity: bad response from server (host=%s, port=%d): %s", timidity_host, timidity_port, res);
		close_all();
		return -1;
	}

	/*
	 * setup buf and prepare data connection
	 */
	/* should read: "200 OK" */
	res = timidity_ctl_command("SETBUF %f %f", BUF_LOW_SYNC, BUF_HIGH_SYNC);
	if (atoi(res) != 200)
		warning("TiMidity: bad reply for SETBUF command: %s", res);

	/* should read something like "200 63017 is ready acceptable",
	 * where 63017 is port for data connection */
	// FIXME: The following looks like a cheap endian test. If this is true, then
	// it should be replaced by suitable #ifdef SCUMM_LITTLE_ENDIAN.
	i = 1;
	if (*(char *)&i == 1)
		res = timidity_ctl_command("OPEN lsb");
	else
		res = timidity_ctl_command("OPEN msb");

	if (atoi(res) != 200) {
		warning("TiMidity: bad reply for OPEN command: %s", res);
		close_all();
		return -1;
	}

	/*
	 * open data connection
	 */
	data_port = atoi(res + 4);
	if ((_data_fd = connect_to_server(timidity_host, data_port)) < 0) {
		warning("TiMidity: can't open data connection (host=%s, port=%d)", timidity_host, data_port);
		close_all();
		return -1;
	}

	/* should read message issued after connecting to data port:
	 * "200 Ready data connection" */
	res = timidity_ctl_command(NULL);
	if (atoi(res) != 200) {
		warning("Can't connect timidity: %s\t(host=%s, port=%d)", res, timidity_host, data_port);
		close_all();
		return -1;
	}

	/*
	 * From seq.cpp
	 */
	if (getenv("SCUMMVM_MIDIPORT"))
		_device_num = atoi(getenv("SCUMMVM_MIDIPORT"));

	return 0;
}

void MidiDriver_TIMIDITY::close() {
	teardown();

	MidiDriver_MPU401::close();
	_isOpen = false;
}

void MidiDriver_TIMIDITY::close_all() {
	if (_control_fd >= 0)
		::close(_control_fd);

	if (_data_fd >= 0)
		::close(_data_fd);

	_control_fd = _data_fd = -1;
}

void MidiDriver_TIMIDITY::teardown() {
	char *res;

	/* teardown connection to server (see timidity-io.c) if it
	 * is initialized */
	if (_data_fd >= 0 && _control_fd >= 0) {
		timidity_eot();
		timidity_sync(0);

		/* scroll through all "302 Data connection is (already) closed"
		 * messages till we reach something like "200 Bye" */
		do {
			res = timidity_ctl_command("QUIT");
		} while (*res && atoi(res) && atoi(res) != 302);
	}

	/* now close and nullify both filedescs */
	close_all();
}

in_addr_t MidiDriver_TIMIDITY::host_to_addr(const char* address) {
	in_addr_t addr;
	struct hostent *hp;

	/* first check if IP address is given (like 127.0.0.1)*/
	if ((addr = inet_addr(address)) != INADDR_NONE)
		return addr;

	/* if not, try to resolve a hostname */
	if ((hp = gethostbyname(address)) == NULL) {
		warning("TiMidity: unknown hostname: %s", address);
		return INADDR_NONE;
	}

	memcpy(&addr, hp->h_addr, (int)sizeof(in_addr_t) <= hp->h_length ? sizeof(in_addr_t) : hp->h_length);

	return addr;
}

int MidiDriver_TIMIDITY::connect_to_server(const char* hostname, unsigned short tcp_port) {
	int fd;
	struct sockaddr_in in;
	unsigned int addr;

	/* create socket */
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		warning("TiMidity: socket(): %s", strerror(errno));
		return -1;
	}

	/* connect */
	memset(&in, 0, sizeof(in));
	in.sin_family = AF_INET;
	in.sin_port   = htons(tcp_port);
	addr = host_to_addr(hostname);
	memcpy(&in.sin_addr, &addr, 4);

	if (connect(fd, (struct sockaddr *)&in, sizeof(in)) < 0) {
		warning("TiMidity: connect(): %s", strerror(errno));
		return -1;
	}

	return fd;
}

char *MidiDriver_TIMIDITY::timidity_ctl_command(const char *fmt, ...) {
	/* XXX: I don't like this static buffer!!! */
	static char buff[BUFSIZ];
	va_list ap;

	if (fmt != NULL) {
		/* if argumends are present, write them to control connection */
		va_start(ap, fmt);
		int len = vsnprintf(buff, BUFSIZ-1, fmt, ap); /* leave one byte for \n */
		va_end(ap);

		/* add newline if needed */
		if (len > 0 && buff[len - 1] != '\n')
			buff[len++] = '\n';

		/* write command to control socket */
		if (write(_control_fd, buff, len) == -1) {
			warning("TiMidity: CONTROL WRITE FAILED (%s)", strerror(errno));
			// TODO: Disable output?
			//close_all();
		}
	}

	while (1) {
		/* read reply */
		if (fdgets(buff, sizeof(buff)) <= 0) {
			strcpy(buff, "Read error\n");
			break;
		}

		/* report errors from server */
		int status = atoi(buff);
		if (400 <= status && status <= 499) { /* Error of data stream */
			warning("TiMidity: error from server: %s", buff);
			continue;
		}
		break;
	}

	return buff;
}

void MidiDriver_TIMIDITY::timidity_meta_seq(int p1, int p2, int p3) {
	/* see _CHN_COMMON from soundcard.h; this is simplified
	 * to just send seq to the server without any buffers,
	 * delays and extra functions/macros */
	u_char seqbuf[8];

	seqbuf[0] = 0x92;
	seqbuf[1] = 0;
	seqbuf[2] = 0xff;
	seqbuf[3] = 0x7f;
	seqbuf[4] = p1;
	seqbuf[5] = p2;
	WRITE_UINT16(&seqbuf[6], p3);

	timidity_write_data(seqbuf, sizeof(seqbuf));
}

int MidiDriver_TIMIDITY::timidity_sync(int centsec) {
	char *res;
	int status;
	unsigned long sleep_usec;

	timidity_meta_seq(0x02, 0x00, centsec); /* Wait playout */

	/* Wait "301 Sync OK" */
	do {
		res = timidity_ctl_command(NULL);
		status = atoi(res);

		if (status != 301)
			warning("TiMidity: error: SYNC: %s", res);

	} while (status && status != 301);

	if (status != 301)
		return -1; /* error */

	sleep_usec = (unsigned long)(atof(res + 4) * 1000000);

	if (sleep_usec > 0)
		usleep(sleep_usec);

	return 0;
}

int MidiDriver_TIMIDITY::timidity_eot(void) {
	timidity_meta_seq(0x00, 0x00, 0); /* End of playing */
	return timidity_sync(0);
}

void MidiDriver_TIMIDITY::timidity_write_data(const void *buf, size_t nbytes) {
	/* nowhere to write... */
	if (_data_fd < 0)
		return;

	/* write, and disable everything if write failed */
	/* TODO: add reconnect? */
	if (write(_data_fd, buf, nbytes) == -1) {
		warning("TiMidity: DATA WRITE FAILED (%s), DISABLING MUSIC OUTPUT", strerror(errno));
		close_all();
	}
}

int MidiDriver_TIMIDITY::fdgets(char *buff, size_t buff_size) {
	int n, count, size;
	char *buff_endp = buff + buff_size - 1, *pbuff, *beg;

	count = _controlbuffer_count;
	size = _controlbuffer_size;
	pbuff = _controlbuffer;
	beg = buff;
	do {
		if (count == size) {
			if ((n = read(_control_fd, pbuff, BUFSIZ)) <= 0) {
				*buff = '\0';
				if (n == 0) {
					_controlbuffer_count = _controlbuffer_size = 0;
					return buff - beg;
				}
				return -1; /* < 0 error */
			}
			count = _controlbuffer_count = 0;
			size = _controlbuffer_size = n;
		}
		*buff++ = pbuff[count++];
	} while (*(buff - 1) != '\n' && buff != buff_endp);

	*buff = '\0';
	_controlbuffer_count = count;

	return buff - beg;
}

void MidiDriver_TIMIDITY::send(uint32 b) {
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
		warning("MidiDriver_TIMIDITY::send: unknown : %08x", (int)b);
		break;
	}

	timidity_write_data(buf, position);
}

void MidiDriver_TIMIDITY::sysEx(const byte *msg, uint16 length) {
	fprintf(stderr, "Timidity::sysEx\n");
	unsigned char buf[266*4];
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

	timidity_write_data(buf, position);
}


// Plugin interface

class TimidityMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return "TiMidity";
	}

	const char *getId() const {
		return "timidity";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices TimidityMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_GM));
	return devices;
}

Common::Error TimidityMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle) const {
	*mididriver = new MidiDriver_TIMIDITY();

	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(TIMIDITY)
	//REGISTER_PLUGIN_DYNAMIC(TIMIDITY, PLUGIN_TYPE_MUSIC, TimidityMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(TIMIDITY, PLUGIN_TYPE_MUSIC, TimidityMusicPlugin);
//#endif

#endif // defined(USE_TIMIDITY)
