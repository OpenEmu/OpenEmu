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
 * Based on AdLib emulation code of DOSBox
 * Copyright (C) 2002-2009  The DOSBox Team
 * Licensed under GPLv2+
 * http://www.dosbox.com
 */

#ifndef DISABLE_DOSBOX_OPL

#include "dosbox.h"
#include "dbopl.h"

#include "common/system.h"
#include "common/scummsys.h"
#include "common/util.h"

#include <math.h>
#include <string.h>

namespace OPL {
namespace DOSBox {

Timer::Timer() {
	masked = false;
	overflow = false;
	enabled = false;
	counter = 0;
	delay = 0;
}

void Timer::update(double time) {
	if (!enabled || !delay)
		return;
	double deltaStart = time - startTime;
	// Only set the overflow flag when not masked
	if (deltaStart >= 0 && !masked)
		overflow = 1;
}

void Timer::reset(double time) {
	overflow = false;
	if (!delay || !enabled)
		return;
	double delta = (time - startTime);
	double rem = fmod(delta, delay);
	double next = delay - rem;
	startTime = time + next;
}

void Timer::stop() {
	enabled = false;
}

void Timer::start(double time, int scale) {
	//Don't enable again
	if (enabled)
		return;
	enabled = true;
	delay = 0.001 * (256 - counter) * scale;
	startTime = time + delay;
}

bool Chip::write(uint32 reg, uint8 val) {
	switch (reg) {
	case 0x02:
		timer[0].counter = val;
		return true;
	case 0x03:
		timer[1].counter = val;
		return true;
	case 0x04:
		double time = g_system->getMillis() / 1000.0;

		if (val & 0x80) {
			timer[0].reset(time);
			timer[1].reset(time);
		} else {
			timer[0].update(time);
			timer[1].update(time);

			if (val & 0x1)
				timer[0].start(time, 80);
			else
				timer[0].stop();

			timer[0].masked = (val & 0x40) > 0;

			if (timer[0].masked)
				timer[0].overflow = false;

			if (val & 0x2)
				timer[1].start(time, 320);
			else
				timer[1].stop();

			timer[1].masked = (val & 0x20) > 0;

			if (timer[1].masked)
				timer[1].overflow = false;
		}
		return true;
	}
	return false;
}

uint8 Chip::read() {
	double time = g_system->getMillis() / 1000.0;

	timer[0].update(time);
	timer[1].update(time);

	uint8 ret = 0;
	// Overflow won't be set if a channel is masked
	if (timer[0].overflow) {
		ret |= 0x40;
		ret |= 0x80;
	}
	if (timer[1].overflow) {
		ret |= 0x20;
		ret |= 0x80;
	}
	return ret;
}

OPL::OPL(Config::OplType type) : _type(type), _rate(0), _emulator(0) {
}

OPL::~OPL() {
	free();
}

void OPL::free() {
	delete _emulator;
	_emulator = 0;
}

bool OPL::init(int rate) {
	free();

	memset(&_reg, 0, sizeof(_reg));
	memset(_chip, 0, sizeof(_chip));

	_emulator = new DBOPL::Chip();
	if (!_emulator)
		return false;

	DBOPL::InitTables();
	_emulator->Setup(rate);

	if (_type == Config::kDualOpl2) {
		// Setup opl3 mode in the hander
		_emulator->WriteReg(0x105, 1);
	}

	_rate = rate;
	return true;
}

void OPL::reset() {
	init(_rate);
}

void OPL::write(int port, int val) {
	if (port&1) {
		switch (_type) {
		case Config::kOpl2:
		case Config::kOpl3:
			if (!_chip[0].write(_reg.normal, val))
				_emulator->WriteReg(_reg.normal, val);
			break;
		case Config::kDualOpl2:
			// Not a 0x??8 port, then write to a specific port
			if (!(port & 0x8)) {
				byte index = (port & 2) >> 1;
				dualWrite(index, _reg.dual[index], val);
			} else {
				//Write to both ports
				dualWrite(0, _reg.dual[0], val);
				dualWrite(1, _reg.dual[1], val);
			}
			break;
		}
	} else {
		// Ask the handler to write the address
		// Make sure to clip them in the right range
		switch (_type) {
		case Config::kOpl2:
			_reg.normal = _emulator->WriteAddr(port, val) & 0xff;
			break;
		case Config::kOpl3:
			_reg.normal = _emulator->WriteAddr(port, val) & 0x1ff;
			break;
		case Config::kDualOpl2:
			// Not a 0x?88 port, when write to a specific side
			if (!(port & 0x8)) {
				byte index = (port & 2) >> 1;
				_reg.dual[index] = val & 0xff;
			} else {
				_reg.dual[0] = val & 0xff;
				_reg.dual[1] = val & 0xff;
			}
			break;
		}
	}
}

byte OPL::read(int port) {
	switch (_type) {
	case Config::kOpl2:
		if (!(port & 1))
			//Make sure the low bits are 6 on opl2
			return _chip[0].read() | 0x6;
		break;
	case Config::kOpl3:
		if (!(port & 1))
			return _chip[0].read();
		break;
	case Config::kDualOpl2:
		// Only return for the lower ports
		if (port & 1)
			return 0xff;
		// Make sure the low bits are 6 on opl2
		return _chip[(port >> 1) & 1].read() | 0x6;
	}
	return 0;
}

void OPL::writeReg(int r, int v) {
	int tempReg = 0;
	switch (_type) {
	case Config::kOpl2:
	case Config::kDualOpl2:
	case Config::kOpl3:
		// We can't use _handler->writeReg here directly, since it would miss timer changes.

		// Backup old setup register
		tempReg = _reg.normal;

		// We directly allow writing to secondary OPL3 registers by using
		// register values >= 0x100.
		if (_type == Config::kOpl3 && r >= 0x100) {
			// We need to set the register we want to write to via port 0x222,
			// since we want to write to the secondary register set.
			write(0x222, r);
			// Do the real writing to the register
			write(0x223, v);
		} else {
			// We need to set the register we want to write to via port 0x388
			write(0x388, r);
			// Do the real writing to the register
			write(0x389, v);
		}

		// Restore the old register
		if (_type == Config::kOpl3 && tempReg >= 0x100) {
			write(0x222, tempReg & ~0x100);
		} else {
			write(0x388, tempReg);
		}
		break;
	};
}

void OPL::dualWrite(uint8 index, uint8 reg, uint8 val) {
	// Make sure you don't use opl3 features
	// Don't allow write to disable opl3
	if (reg == 5)
		return;

	// Only allow 4 waveforms
	if (reg >= 0xE0 && reg <= 0xE8)
		val &= 3;

	// Write to the timer?
	if (_chip[index].write(reg, val))
		return;

	// Enabling panning
	if (reg >= 0xC0 && reg <= 0xC8) {
		val &= 15;
		val |= index ? 0xA0 : 0x50;
	}

	uint32 fullReg = reg + (index ? 0x100 : 0);
	_emulator->WriteReg(fullReg, val);
}

void OPL::readBuffer(int16 *buffer, int length) {
	// For stereo OPL cards, we divide the sample count by 2,
	// to match stereo AudioStream behavior.
	if (_type != Config::kOpl2)
		length >>= 1;

	const uint bufferLength = 512;
	int32 tempBuffer[bufferLength * 2];

	if (_emulator->opl3Active) {
		while (length > 0) {
			const uint readSamples = MIN<uint>(length, bufferLength);

			_emulator->GenerateBlock3(readSamples, tempBuffer);

			for (uint i = 0; i < (readSamples << 1); ++i)
				buffer[i] = tempBuffer[i];

			buffer += (readSamples << 1);
			length -= readSamples;
		}
	} else {
		while (length > 0) {
			const uint readSamples = MIN<uint>(length, bufferLength << 1);

			_emulator->GenerateBlock2(readSamples, tempBuffer);

			for (uint i = 0; i < readSamples; ++i)
				buffer[i] = tempBuffer[i];

			buffer += readSamples;
			length -= readSamples;
		}
	}
}

} // End of namespace DOSBox
} // End of namespace OPL

#endif // !DISABLE_DOSBOX_ADLIB
