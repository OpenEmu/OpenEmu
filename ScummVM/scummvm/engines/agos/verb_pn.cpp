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

// Verb and hitarea handling


#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

void AGOSEngine_PN::boxController(uint x, uint y, uint mode) {
	HitArea *best_ha;
	HitArea *ha = _hitAreaList;

	best_ha = NULL;

	do {
		if (!(ha->flags & kOBFBoxDisabled)) {
			if (x >= ha->x && y >= ha->y && x - ha->x < ha->width && y - ha->y < ha->height &&
				best_ha == NULL) {
				best_ha = ha;
			} else {
				if (ha->flags & kOBFBoxSelected) {
					hitarea_leave(ha , true);
					ha->flags &= ~kOBFBoxSelected;
				}
			}
		} else {
			ha->flags &= ~kOBFBoxSelected;
		}
	} while (ha++, ha->id != 0xFFFF);

	if (mode != 0) {
		_lastHitArea = best_ha;
	}

	if (best_ha == NULL) {
		return;
	}

	if (best_ha->flags & kOBFInvertTouch && !(best_ha->flags & kOBFBoxSelected)) {
		hitarea_leave(best_ha, false);
		best_ha->flags |= kOBFBoxSelected;
	}
}

void AGOSEngine_PN::mouseHit() {
	if (_hitCalled == 5) {
		execMouseHit(NULL);
	} else {
		boxController(_mouse.x, _mouse.y, 1);
		if (_hitCalled == 4 || _lastHitArea != 0) {
			execMouseHit(_lastHitArea);
		}
	}
	_hitCalled = 0;
	_oneClick = 0;
}

void AGOSEngine_PN::execMouseHit(HitArea *ha) {
	if (_hitCalled == 1) {
		if (ha->flags & kOBFUseMessageList)
			hitBox11(ha);
		else if (ha->flags & kOBFMoreBox)
			hitBox9(ha);
		else if (ha->flags & kOBFExit)
			hitBox7(ha);
		else if (ha->flags & kOBFUseEmptyLine)
			hitBox2(ha);
		else
			hitBox1(ha);
	} else if (_hitCalled == 2) {
		if (ha->flags & (kOBFObject | kOBFInventoryBox | kOBFRoomBox))
			hitBox3(ha);
		else if (ha->flags & kOBFUseMessageList)
			hitBox11(ha);
		else if (ha->flags & kOBFMoreBox)
			hitBox9(ha);
		else if (ha->flags & kOBFExit)
			hitBox7(ha);
		else if (ha->flags & kOBFUseEmptyLine)
			hitBox2(ha);
		else
			hitBox1(ha);
	} else if (_hitCalled == 3) {
		if ((ha->flags & kOBFDraggable) && !_dragFlag) {
			_dragFlag = true;
			_dragStore = ha;
			_needHitAreaRecalc++;
		}
	} else if (_hitCalled == 4) {
		_dragFlag = false;
		_hitCalled = 0;
		_oneClick = 0;
		_dragCount = 0;
		_needHitAreaRecalc++;
		if (ha != 0) {
			if (ha->flags & kOBFInventoryBox)
				hitBox5(ha);
			else if (ha->flags & kOBFRoomBox)
				hitBox6(ha);
			else if (_videoLockOut & 10)
				hitBox8(ha);
		}
	} else {
		_hitCalled = 0;
		if (_mouseString == 0) {
			_mouseString = (const char *)"\r";
		}
	}
}

void AGOSEngine_PN::hitBox1(HitArea *ha) {
	if (_mouseString)
		return;

	_mouseString = getMessage(_objectName1, ha->msg1);
	if (_intputCounter) {
		char *msgPtr = getMessage(_objectName1, ha->msg1);
		while (*msgPtr != 13)
			msgPtr++;
		*msgPtr = 0;
	} else if (!(ha->flags & kOBFNoShowName)) {
		_mousePrintFG++;
		_mouseString1 = _mouseString;
		_mouseString = (const char*)"showname \0";

	}
}

void AGOSEngine_PN::hitBox2(HitArea *ha) {
	if (!_intputCounter)
		hitBox1(ha);
}

void AGOSEngine_PN::hitBox3(HitArea *ha) {
	if (!_intputCounter)
		hitBox4(ha);
}

void AGOSEngine_PN::hitBox4(HitArea *ha) {
	if (_mouseString)
		return;

	uint16 num = ha->msg1 & ~0x8000;
	if ((_videoLockOut & 0x10) && !(ha->flags & (kOBFInventoryBox | kOBFRoomBox)) &&
		!testContainer(num)) {
		return;
	}

	_mouseString = getMessage(_objectName2, ha->msg2);
	_mouseString1 = getMessage(_objectName1, ha->msg1);
	_mousePrintFG++;
}

void AGOSEngine_PN::hitBox5(HitArea *ha) {
	if (_intputCounter || _mouseString)
		return;

	if (_dragStore && (_dragStore->flags & kOBFInventoryBox))
		return;

	_mousePrintFG++;
	_mouseString = (const char *)"take \0";
	_mouseString1 = _dragStore ? getMessage(_objectName1, _dragStore->msg1) : "";

	if (_dragStore->flags & kOBFRoomBox)
		_mouseString1 = (const char *)"all\r";
}

void AGOSEngine_PN::hitBox6(HitArea *ha) {
	if (_intputCounter || _mouseString)
		return;

	if (_dragStore->flags & kOBFRoomBox)
		return;

	_mousePrintFG++;
	_mouseString = (const char *)"drop \0";
	_mouseString1 = getMessage(_objectName1, _dragStore->msg1);

	if (_dragStore->flags & kOBFInventoryBox)
		_mouseString1 = (const char *)"all\r";
}

void AGOSEngine_PN::hitBox7(HitArea *ha) {
	if (_intputCounter) {
		if (!(ha->flags & kOBFUseEmptyLine)) {
			hitBox1(ha);
		}
		return;
	}

	if (_mouseString)
		return;

	_mousePrintFG++;
	_mouseString1 = getMessage(_objectName1, ha->msg1);

	uint16 num = ha->msg1 & ~0x8000;
	uint16 state = getptr(_quickptr[0] + num * _quickshort[0] + 2);
	if (state == 3) {
		_mouseString = (const char *)"unlock \0";
	} else if (state == 2) {
		_mouseString = (const char *)"open \0";
	} else {
		_mouseString = (const char *)"go through \0";
	}
}

void AGOSEngine_PN::hitBox8(HitArea *ha) {
	char *msgPtr, *tmpPtr;

	if (_intputCounter || _mouseString)
		return;

	if (_dragStore == ha)
		return;

	uint16 num = ha->msg1 & ~0x8000;
	if (!testSeen(num))
		return;

	msgPtr = getMessage(_objectName1, ha->msg1);
	sprintf(_inMessage, " in %s", msgPtr);
	_mouseString1 = _inMessage;

	msgPtr = getMessage(_objectName1, _dragStore->msg1);
	*(tmpPtr = strchr(msgPtr, 13)) = 0;
	sprintf(_placeMessage, "put %s", msgPtr);
	_mouseString = _placeMessage;
}

void AGOSEngine_PN::hitBox9(HitArea *ha) {
	if (_objectCountS == _objects) {
		_objectCountS = -1;
	}
	iconPage();
}

static const char *const messageList[9] = {
	"North\r",
	"East\r",
	"South\r",
	"West\r",
	"Up\r",
	"Down\r",
	"Push grey button\r",
	"Push red button\r",
	"Go under car\r"
};

void AGOSEngine_PN::hitBox11(HitArea *ha) {
	if (_intputCounter || _mouseString)
		return;

	_mouseString = messageList[ha->msg1];
	_mousePrintFG++;
}

} // End of namespace AGOS
