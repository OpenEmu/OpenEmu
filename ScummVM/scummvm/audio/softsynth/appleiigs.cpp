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

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "audio/null.h"

//	Plugin interface
//	(This can only create a null driver since apple II gs support seeems not to be implemented
//  and also is not part of the midi driver architecture. But we need the plugin for the options
//  menu in the launcher and for MidiDriver::detectDevice() which is more or less used by all engines.)

class AppleIIGSMusicPlugin : public NullMusicPlugin {
public:
	const char *getName() const {
		return _s("Apple II GS Emulator (NOT IMPLEMENTED)");
	}

	const char *getId() const {
		return "appleIIgs";
	}

	MusicDevices getDevices() const;
};

MusicDevices AppleIIGSMusicPlugin::getDevices() const {
	MusicDevices devices;
	devices.push_back(MusicDevice(this, "", MT_APPLEIIGS));
	return devices;
}

//#if PLUGIN_ENABLED_DYNAMIC(APPLEIIGS)
	//REGISTER_PLUGIN_DYNAMIC(APPLEIIGS, PLUGIN_TYPE_MUSIC, AppleIIGSMusicPlugin);
//#else
	REGISTER_PLUGIN_STATIC(APPLEIIGS, PLUGIN_TYPE_MUSIC, AppleIIGSMusicPlugin);
//#endif
