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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <kernel.h>
#include <malloc.h>
#include <assert.h>
#include "backends/platform/ps2/systemps2.h"
#include "backends/platform/ps2/ps2pad.h"

Ps2Pad::Ps2Pad(OSystem_PS2 *system) {
	_system = system;
	_padBuf = (uint8 *)memalign(64, 256);
	_padStatus = STAT_NONE;

	padInit(0); // initialize library
	_port = _slot = 0;	// first controller, no multitap
	initPad();
}

void Ps2Pad::initPad(void) {
	int modes = 0;
	if (_padStatus == STAT_NONE) {
		if (padPortOpen(_port, _slot, _padBuf) == 1) {
			_padStatus = STAT_OPEN;
			_padInitTime = _system->getMillis();
		} else {
			padPortClose(_port, _slot);
			printf("Unable to open port (%d/%d)!\n", _port, _slot);
		}
	} else {
		if (checkPadReady(_port, _slot)) {
			switch (_padStatus) {
				case STAT_OPEN:
					_padStatus = STAT_DETECT;
					break;
				case STAT_DETECT:
					_isDualShock = false;
					modes = padInfoMode(_port, _slot, PAD_MODETABLE, -1);

					// Verify that the controller has a DUAL SHOCK mode
					for (int cnt = 0; cnt < modes; cnt++)
						if (padInfoMode(_port, _slot, PAD_MODETABLE, cnt) == PAD_TYPE_DUALSHOCK)
							_isDualShock = true;

					// If ExId != 0x0 => This controller has actuator engines
					// This check should always pass if the Dual Shock test above passed
					if (_isDualShock)
						if (padInfoMode(_port, _slot, PAD_MODECUREXID, 0) == 0)
							_isDualShock = false;

					if (_isDualShock) {
						// When using MMODE_LOCK, user cant change mode with Select button
						padSetMainMode(_port, _slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);
						_padStatus = STAT_INIT_DSHOCK;
					} else
						_padStatus = STAT_WAIT_READY;
					break;
				case STAT_INIT_DSHOCK:
					padEnterPressMode(_port, _slot);
					_padStatus = STAT_CHECK_ACT;
					break;
				case STAT_CHECK_ACT:
					_actuators = padInfoAct(_port, _slot, -1, 0);
					if (_actuators != 0)
						_padStatus = STAT_INIT_ACT;
					else {
						_isDualShock = false;
						_padStatus = STAT_WAIT_READY;
					}
					break;
				case STAT_INIT_ACT:
					char actAlign[6];
					actAlign[0] = 0;
					actAlign[1] = 1;
					actAlign[2] = actAlign[3] = actAlign[4] = actAlign[5] = 0xff;
					padSetActAlign(_port, _slot, actAlign);
					_padStatus = STAT_WAIT_READY;
					break;
				case STAT_WAIT_READY:
					_padStatus = STAT_OKAY;
					break;
				case STAT_OKAY:
					// pad is already initialized
					break;
			}
		} else {
			// check for timeout...
			if (_system->getMillis() - _padInitTime > 5000) {
				// still no pad, give up.
				if (padPortClose(_port, _slot) != 1)
					printf("WARNING: can't close port: %d/%d\n", _port, _slot);
				printf("looking for pad, gave up and closed port\n");
				_padStatus = STAT_NONE;
			}
		}
	}
}

bool Ps2Pad::checkPadReady(int port, int slot) {
	int state = padGetState(port, slot);
	return (state == PAD_STATE_STABLE) || (state == PAD_STATE_FINDCTP1);
}

bool Ps2Pad::padAlive(void) {
	if ((_padStatus == STAT_OKAY) && checkPadReady(_port, _slot))
		return true;
	initPad();
	return false;
}

bool Ps2Pad::isDualShock(void) {
	return _isDualShock;
}

void Ps2Pad::readPad(uint16 *pbuttons, int16 *joyh, int16 *joyv) {
	if (padAlive()) {
		struct padButtonStatus buttons;
		padRead(_port, _slot, &buttons);
		*pbuttons = ~buttons.btns;
		if (_isDualShock) {
			*joyh = (int16)buttons.ljoy_h - 128;
			*joyv = (int16)buttons.ljoy_v - 128;
		} else
			*joyh = *joyv = 0;
	} else {
		*joyh = *joyv = 0;
		*pbuttons = 0;
	}
}
