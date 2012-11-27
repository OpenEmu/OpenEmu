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

#ifndef __PS2PAD_H__
#define __PS2PAD_H__

enum PadStatus {
	STAT_NONE,
	STAT_OPEN,
	STAT_DETECT,
	STAT_INIT_DSHOCK,
	STAT_CHECK_ACT,
	STAT_INIT_ACT,
	STAT_WAIT_READY,
	STAT_OKAY
};

#include "common/system.h"
#include <libpad.h>

#define PAD_DIR_MASK  (PAD_LEFT | PAD_DOWN | PAD_RIGHT | PAD_UP)
#define PAD_BUTTON_MASK (PAD_START | PAD_SELECT | PAD_SQUARE | PAD_CROSS | PAD_CIRCLE | PAD_TRIANGLE)

class OSystem_PS2;

class Ps2Pad {
public:
	Ps2Pad(OSystem_PS2 *system);
	bool padAlive(void);
	bool isDualShock(void);
	void readPad(uint16 *pbuttons, int16 *joyh, int16 *joyv);
private:
	void initPad(void);
	bool checkPadReady(int port, int slot);

	OSystem_PS2 *_system;
	int _port, _slot;

	uint32 _padInitTime;
	PadStatus _padStatus;
	bool _isDualShock;
	uint8 *_padBuf;
	char _actuators;
};

#endif //__PS2PAD_H__
