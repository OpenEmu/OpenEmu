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

#include "agi/agi.h"

namespace Agi {

int AgiEngine::checkPosition(VtEntry *v) {
	debugC(4, kDebugLevelSprites, "check position @ %d, %d", v->xPos, v->yPos);

	if (v->xPos < 0 ||
			v->xPos + v->xSize > _WIDTH ||
			v->yPos - v->ySize + 1 < 0 ||
			v->yPos >= _HEIGHT ||
			((~v->flags & fIgnoreHorizon) && v->yPos <= _game.horizon)) {
		debugC(4, kDebugLevelSprites, "check position failed: x=%d, y=%d, h=%d, w=%d",
				v->xPos, v->yPos, v->xSize, v->ySize);
		return 0;
	}

	// MH1 needs this, but it breaks LSL1
	if (getVersion() >= 0x3000) {
		if (v->yPos < v->ySize)
			return 0;
	}

	return 1;
}

/**
 * Check if there's another object on the way
 */
int AgiEngine::checkCollision(VtEntry *v) {
	VtEntry *u;

	if (v->flags & fIgnoreObjects)
		return 0;

	for (u = _game.viewTable; u < &_game.viewTable[MAX_VIEWTABLE]; u++) {
		if ((u->flags & (fAnimated | fDrawn)) != (fAnimated | fDrawn))
			continue;

		if (u->flags & fIgnoreObjects)
			continue;

		// Same object, check next
		if (v->entry == u->entry)
			continue;

		// No horizontal overlap, check next
		if (v->xPos + v->xSize < u->xPos || v->xPos > u->xPos + u->xSize)
			continue;

		// Same y, return error!
		if (v->yPos == u->yPos) {
			debugC(4, kDebugLevelSprites, "check returns 1 (object %d)", v->entry);
			return 1;
		}

		// Crossed the baseline, return error!
		if ((v->yPos > u->yPos && v->yPos2 < u->yPos2) ||
				(v->yPos < u->yPos && v->yPos2 > u->yPos2)) {
			debugC(4, kDebugLevelSprites, "check returns 1 (object %d)", v->entry);
			return 1;
		}
	}

	return 0;

}

int AgiEngine::checkPriority(VtEntry *v) {
	int i, trigger, water, pass, pri;
	uint8 *p0;

	if (~v->flags & fFixedPriority) {
		// Priority bands
		v->priority = _game.priTable[v->yPos];
	}

	trigger = 0;
	water = 0;
	pass = 1;

	if (v->priority == 0x0f) {
		// Check ego
		if (v->entry == 0) {
			setflag(fEgoTouchedP2, trigger ? true : false);
			setflag(fEgoWater, water ? true : false);
		}

		return pass;
	}

	water = 1;

	// Check if any picture is loaded before checking for priority below.
	// If no picture has been loaded, the priority buffer won't be initialized,
	// thus the check below will always fail. This case causes an infinite loop
	// in the fanmade game Nick's Quest (bug #3451122), as the game attempts to
	// draw a sprite (view 4, floating Nick) before it loads any picture. This
	// causes the checks below to always fail, and the engine keeps readjusting
	// the sprite's position in fixPosition() forever, as there is no valid
	// position to place it (the default visual and priority screen is set to
	// zero, i.e. unconditional black). To remedy this situation, we always
	// return true here if no picture has been loaded and no priority screen
	// has been set up.
	if (!_game._vm->_picture->isPictureLoaded()) {
		warning("checkPriority: no picture loaded");
		return pass;
	}

	p0 = &_game.sbuf16c[v->xPos + v->yPos * _WIDTH];

	for (i = 0; i < v->xSize; i++, p0++) {
		pri = *p0 >> 4;

		if (pri == 0) {	// unconditional black. no go at all!
			pass = 0;
			break;
		}

		if (pri == 3)	// water surface
			continue;

		water = 0;

		if (pri == 1) {	// conditional blue
			if (v->flags & fIgnoreBlocks)
				continue;

			debugC(4, kDebugLevelSprites, "Blocks observed!");
			pass = 0;
			break;
		}

		if (pri == 2) {	// trigger
			debugC(4, kDebugLevelSprites, "stepped on trigger");
			if (!_debug.ignoretriggers)
				trigger = 1;
		}
	}

	if (pass) {
		if (!water && v->flags & fOnWater)
			pass = 0;
		if (water && v->flags & fOnLand)
			pass = 0;
	}

	// Check ego
	if (v->entry == 0) {
		setflag(fEgoTouchedP2, trigger ? true : false);
		setflag(fEgoWater, water ? true : false);
	}

	return pass;
}

/*
 * Public functions
 */

/**
 * Update position of objects
 * This function updates the position of all animated, updating view
 * table entries according to its motion type, step size, etc. The
 * new position must be valid according to the sprite positioning
 * rules, otherwise the previous position will be kept.
 */
void AgiEngine::updatePosition() {
	VtEntry *v;
	int x, y, oldX, oldY, border;

	_game.vars[vBorderCode] = 0;
	_game.vars[vBorderTouchEgo] = 0;
	_game.vars[vBorderTouchObj] = 0;

	for (v = _game.viewTable; v < &_game.viewTable[MAX_VIEWTABLE]; v++) {
		if ((v->flags & (fAnimated | fUpdate | fDrawn)) != (fAnimated | fUpdate | fDrawn)) {
			continue;
		}

		if (v->stepTimeCount != 0) {
			if (--v->stepTimeCount != 0)
				continue;
		}

		v->stepTimeCount = v->stepTime;

		x = oldX = v->xPos;
		y = oldY = v->yPos;

		// If object has moved, update its position
		if (~v->flags & fUpdatePos) {
			int dx[9] = { 0, 0, 1, 1, 1, 0, -1, -1, -1 };
			int dy[9] = { 0, -1, -1, 0, 1, 1, 1, 0, -1 };
			x += v->stepSize * dx[v->direction];
			y += v->stepSize * dy[v->direction];
		}

		// Now check if it touched the borders
		border = 0;

		// Check left/right borders
		if (x < 0) {
			x = 0;
			border = 4;
		} else if (x <= 0 && getVersion() == 0x3086) {	// KQ4
			x = 0;	// See Sarien bug #590462
			border = 4;
		} else if (v->entry == 0 && x == 0 && v->flags & fAdjEgoXY) {
			// Extra test to walk west clicking the mouse
			x = 0;
			border = 4;
		} else if (x + v->xSize > _WIDTH) {
			x = _WIDTH - v->xSize;
			border = 2;
		}

		// Check top/bottom borders.
		if (y - v->ySize + 1 < 0) {
			y = v->ySize - 1;
			border = 1;
		} else if (y > _HEIGHT - 1) {
			y = _HEIGHT - 1;
			border = 3;
		} else if ((~v->flags & fIgnoreHorizon) && y <= _game.horizon) {
			debugC(4, kDebugLevelSprites, "y = %d, horizon = %d", y, _game.horizon);
			y = _game.horizon + 1;
			border = 1;
		}

		// Test new position. rollback if test fails
		v->xPos = x;
		v->yPos = y;
		if (checkCollision(v) || !checkPriority(v)) {
			v->xPos = oldX;
			v->yPos = oldY;
			border = 0;
			fixPosition(v->entry);
		}

		if (border != 0) {
			if (isEgoView(v)) {
				_game.vars[vBorderTouchEgo] = border;
			} else {
				_game.vars[vBorderCode] = v->entry;
				_game.vars[vBorderTouchObj] = border;
			}
			if (v->motion == kMotionMoveObj) {
				inDestination(v);
			}
		}

		v->flags &= ~fUpdatePos;
	}
}

/**
 * Adjust position of a sprite
 * This function adjusts the position of a sprite moving it until
 * certain criteria is matched. According to priority and control line
 * data, a sprite may not always appear at the location we specified.
 * This behavior is also known as the "Budin-Sonneveld effect".
 *
 * @param n view table entry number
 */
void AgiEngine::fixPosition(int n) {
	VtEntry *v = &_game.viewTable[n];
	int count, dir, size;

	debugC(4, kDebugLevelSprites, "adjusting view table entry #%d (%d,%d)", n, v->xPos, v->yPos);

	// test horizon
	if ((~v->flags & fIgnoreHorizon) && v->yPos <= _game.horizon)
		v->yPos = _game.horizon + 1;

	dir = 0;
	count = size = 1;

	while (!checkPosition(v) || checkCollision(v) || !checkPriority(v)) {
		switch (dir) {
		case 0:	// west
			v->xPos--;
			if (--count)
				continue;
			dir = 1;
			break;
		case 1:	// south
			v->yPos++;
			if (--count)
				continue;
			dir = 2;
			size++;
			break;
		case 2:	// east
			v->xPos++;
			if (--count)
				continue;
			dir = 3;
			break;
		case 3:	// north
			v->yPos--;
			if (--count)
				continue;
			dir = 0;
			size++;
			break;
		}

		count = size;
	}

	debugC(4, kDebugLevelSprites, "view table entry #%d position adjusted to (%d,%d)", n, v->xPos, v->yPos);
}

} // End of namespace Agi
