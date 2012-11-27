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

#if defined(WIN32) && !defined(_WIN32_WCE)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE

#include "audio/musicplugin.h"
#include "audio/mpu401.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/textconsole.h"
#include "common/error.h"

#include <mmsystem.h>

////////////////////////////////////////
//
// Windows MIDI driver
//
////////////////////////////////////////

class MidiDriver_WIN : public MidiDriver_MPU401 {
private:
	MIDIHDR _streamHeader;
	byte _streamBuffer[266];	// SysEx blocks should be no larger than 266 bytes
	HANDLE _streamEvent;
	HMIDIOUT _mo;
	bool _isOpen;
	int _device;

	void check_error(MMRESULT result);

public:
	MidiDriver_WIN(int deviceIndex) : _isOpen(false), _device(deviceIndex) { }
	int open();
	bool isOpen() const { return _isOpen; }
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);
};

int MidiDriver_WIN::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	_streamEvent = CreateEvent(NULL, true, true, NULL);
	MMRESULT res = midiOutOpen((HMIDIOUT *)&_mo, _device, (DWORD_PTR)_streamEvent, 0, CALLBACK_EVENT);
	if (res != MMSYSERR_NOERROR) {
		check_error(res);
		CloseHandle(_streamEvent);
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	_isOpen = true;
	return 0;
}

void MidiDriver_WIN::close() {
	if (!_isOpen)
		return;
	_isOpen = false;
	MidiDriver_MPU401::close();
	midiOutUnprepareHeader(_mo, &_streamHeader, sizeof(_streamHeader));
	check_error(midiOutClose(_mo));
	CloseHandle(_streamEvent);
}

void MidiDriver_WIN::send(uint32 b) {
	assert(_isOpen);

	union {
		DWORD dwData;
		BYTE bData[4];
	} u;

	u.bData[3] = (byte)((b & 0xFF000000) >> 24);
	u.bData[2] = (byte)((b & 0x00FF0000) >> 16);
	u.bData[1] = (byte)((b & 0x0000FF00) >> 8);
	u.bData[0] = (byte)(b & 0x000000FF);

	check_error(midiOutShortMsg(_mo, u.dwData));
}

void MidiDriver_WIN::sysEx(const byte *msg, uint16 length) {
	if (!_isOpen)
		return;

	if (WaitForSingleObject (_streamEvent, 2000) == WAIT_TIMEOUT) {
		warning ("Could not send SysEx - MMSYSTEM is still trying to send data");
		return;
	}

	assert(length+2 <= 266);

	midiOutUnprepareHeader(_mo, &_streamHeader, sizeof(_streamHeader));

	// Add SysEx frame
	_streamBuffer[0] = 0xF0;
	memcpy(&_streamBuffer[1], msg, length);
	_streamBuffer[length+1] = 0xF7;

	_streamHeader.lpData = (char *)_streamBuffer;
	_streamHeader.dwBufferLength = length + 2;
	_streamHeader.dwBytesRecorded = length + 2;
	_streamHeader.dwUser = 0;
	_streamHeader.dwFlags = 0;

	MMRESULT result = midiOutPrepareHeader(_mo, &_streamHeader, sizeof(_streamHeader));
	if (result != MMSYSERR_NOERROR) {
		check_error (result);
		return;
	}

	ResetEvent(_streamEvent);
	result = midiOutLongMsg(_mo, &_streamHeader, sizeof(_streamHeader));
	if (result != MMSYSERR_NOERROR) {
		check_error(result);
		SetEvent(_streamEvent);
		return;
	}
}

void MidiDriver_WIN::check_error(MMRESULT result) {
	char buf[200];
	if (result != MMSYSERR_NOERROR) {
		midiOutGetErrorText(result, buf, 200);
		warning("MM System Error '%s'", buf);
	}
}


// Plugin interface

class WindowsMusicPlugin : public MusicPluginObject {
public:
	const char *getName() const {
		return _s("Windows MIDI");
	}

	const char *getId() const {
		return "windows";
	}

	MusicDevices getDevices() const;
	Common::Error createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle = 0) const;
};

MusicDevices WindowsMusicPlugin::getDevices() const {
	MusicDevices devices;
	int numDevs = midiOutGetNumDevs();
	MIDIOUTCAPS tmp;

	Common::StringArray deviceNames;
	for (int i = 0; i < numDevs; i++) {
		if (midiOutGetDevCaps(i, &tmp, sizeof(MIDIOUTCAPS)) != MMSYSERR_NOERROR)
			break;
		deviceNames.push_back(tmp.szPname);
	}

	// Check for non-unique device names. This may happen if someone has devices with identical
	// names (e. g. more than one USB device of the exact same hardware type). It seems that this
	// does happen in reality sometimes. We generate index numbers for these devices.
	// This is not an ideal solution, since this index could change whenever another USB
	// device gets plugged in or removed, switched off or just plugged into a different port.
	// Unfortunately midiOutGetDevCaps() does not generate any other unique information
	// that could be used. Our index numbers which match the device order should at least be
	// a little more stable than just using the midiOutGetDevCaps() device ID, since a missing
	// device (e.g. switched off) should actually not be harmful to our indices (as it would be
	// when using the device IDs). The cases where users have devices with identical names should
	// be rare enough anyway.
	Common::Array<int> nonUniqueIndex;
	for (int i = 0; i < numDevs; i++) {
		int match = -1;
		for (int ii = 0; ii < i; ii++) {
			if (deviceNames[i] == deviceNames[ii]) {
				if (nonUniqueIndex[ii] == -1)
					nonUniqueIndex[ii] = 0;
				if (++match == 0)
					++match;
			}
		}
		nonUniqueIndex.push_back(match);
	}

	// We now add the index number to the non-unique device names to make them unique.
	for (int i = 0; i < numDevs; i++) {
		if (nonUniqueIndex[i] != -1)
			deviceNames[i] = Common::String::format("%s - #%.02d", deviceNames[i].c_str(), nonUniqueIndex[i]);
	}

	for (Common::StringArray::iterator i = deviceNames.begin(); i != deviceNames.end(); ++i)
		// There is no way to detect the "MusicType" so I just set it to MT_GM
		// The user will have to manually select his MT32 type device and his GM type device.
		devices.push_back(MusicDevice(this, *i, MT_GM));

	return devices;
}

Common::Error WindowsMusicPlugin::createInstance(MidiDriver **mididriver, MidiDriver::DeviceHandle dev) const {
	int devIndex = 0;
	bool found = false;

	if (dev) {
		MusicDevices i = getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); d++) {
			if (d->getCompleteId().equals(MidiDriver::getDeviceString(dev, MidiDriver::kDeviceId))) {
				found = true;
				break;
			}
			devIndex++;
		}
	}

	*mididriver = new MidiDriver_WIN(found ? devIndex : 0);
	return Common::kNoError;
}

//#if PLUGIN_ENABLED_DYNAMIC(WINDOWS)
	//REGISTER_PLUGIN_DYNAMIC(WINDOWS, PLUGIN_TYPE_MUSIC, WindowsMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(WINDOWS, PLUGIN_TYPE_MUSIC, WindowsMusicPlugin);
//#endif

#endif
