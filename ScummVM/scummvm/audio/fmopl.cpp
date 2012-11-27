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

#include "audio/fmopl.h"

#include "audio/softsynth/opl/dosbox.h"
#include "audio/softsynth/opl/mame.h"

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/translation.h"

namespace OPL {

// Config implementation

enum OplEmulator {
	kAuto = 0,
	kMame = 1,
	kDOSBox = 2
};

OPL::OPL() {
	if (_hasInstance)
		error("There are multiple OPL output instances running");
	_hasInstance = true;
}

const Config::EmulatorDescription Config::_drivers[] = {
	{ "auto", "<default>", kAuto, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3 },
	{ "mame", _s("MAME OPL emulator"), kMame, kFlagOpl2 },
#ifndef DISABLE_DOSBOX_OPL
	{ "db", _s("DOSBox OPL emulator"), kDOSBox, kFlagOpl2 | kFlagDualOpl2 | kFlagOpl3 },
#endif
	{ 0, 0, 0, 0 }
};

Config::DriverId Config::parse(const Common::String &name) {
	for (int i = 0; _drivers[i].name; ++i) {
		if (name.equalsIgnoreCase(_drivers[i].name))
			return _drivers[i].id;
	}

	return -1;
}

Config::DriverId Config::detect(OplType type) {
	uint32 flags = 0;
	switch (type) {
	case kOpl2:
		flags = kFlagOpl2;
		break;

	case kDualOpl2:
		flags = kFlagDualOpl2;
		break;

	case kOpl3:
		flags = kFlagOpl3;
		break;
	}

	DriverId drv = parse(ConfMan.get("opl_driver"));

	// When a valid driver is selected, check whether it supports
	// the requested OPL chip.
	if (drv != -1 && drv != kAuto) {
		// If the chip is supported, just use the driver.
		if ((flags & _drivers[drv].flags)) {
			return drv;
		} else {
			// Else we will output a warning and just
			// return that no valid driver is found.
			warning("Your selected OPL driver \"%s\" does not support type %d emulation, which is requested by your game", _drivers[drv].description, type);
			return -1;
		}
	}

	// Detect the first matching emulator
	drv = -1;

	for (int i = 1; _drivers[i].name; ++i) {
		if (_drivers[i].flags & flags) {
			drv = _drivers[i].id;
			break;
		}
	}

	return drv;
}

OPL *Config::create(OplType type) {
	return create(kAuto, type);
}

OPL *Config::create(DriverId driver, OplType type) {
	// On invalid driver selection, we try to do some fallback detection
	if (driver == -1) {
		warning("Invalid OPL driver selected, trying to detect a fallback emulator");
		driver = kAuto;
	}

	// If autodetection is selected, we search for a matching
	// driver.
	if (driver == kAuto) {
		driver = detect(type);

		// No emulator for the specified OPL chip could
		// be found, thus stop here.
		if (driver == -1) {
			warning("No OPL emulator available for type %d", type);
			return 0;
		}
	}

	switch (driver) {
	case kMame:
		if (type == kOpl2)
			return new MAME::OPL();
		else
			warning("MAME OPL emulator only supports OPL2 emulation");
			return 0;

#ifndef DISABLE_DOSBOX_OPL
	case kDOSBox:
		return new DOSBox::OPL(type);
#endif

	default:
		warning("Unsupported OPL emulator %d", driver);
		// TODO: Maybe we should add some dummy emulator too, which just outputs
		// silence as sound?
		return 0;
	}
}

bool OPL::_hasInstance = false;

} // End of namespace OPL

void OPLDestroy(FM_OPL *OPL) {
	delete OPL;
}

void OPLResetChip(FM_OPL *OPL) {
	OPL->reset();
}

void OPLWrite(FM_OPL *OPL, int a, int v) {
	OPL->write(a, v);
}

unsigned char OPLRead(FM_OPL *OPL, int a) {
	return OPL->read(a);
}

void OPLWriteReg(FM_OPL *OPL, int r, int v) {
	OPL->writeReg(r, v);
}

void YM3812UpdateOne(FM_OPL *OPL, int16 *buffer, int length) {
	OPL->readBuffer(buffer, length);
}

FM_OPL *makeAdLibOPL(int rate) {
	FM_OPL *opl = OPL::Config::create();

	if (opl) {
		if (!opl->init(rate)) {
			delete opl;
			opl = 0;
		}
	}

	return opl;
}
