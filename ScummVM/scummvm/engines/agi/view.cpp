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
#include "agi/sprite.h"

namespace Agi {

void AgiEngine::lSetCel(VtEntry *v, int n) {
	ViewLoop *currentVl;
	ViewCel *currentVc;

	v->currentCel = n;

	currentVl = &_game.views[v->currentView].loop[v->currentLoop];

	// Added by Amit Vainsencher <amitv@subdimension.com> to prevent
	// crash in KQ1 -- not in the Sierra interpreter
	if (currentVl->numCels == 0)
		return;

	// WORKAROUND: This is a very nasty hack to fix a bug in the KQ4 introduction
	// In its original form, it caused a lot of regressions, including KQ4 bugs and crashes
	// Refer to Sarien bug #588899 for the original issue
	// Modifying this workaround to only work for a specific view in the KQ4 intro fixes several
	// ScummVM bugs. Refer to bugs #1660486, #1660169, #1660192, #1660162 and #1660354
	// FIXME: Remove this workaround and investigate the reason for the erroneous actor behavior
	// in the KQ4 introduction
	// It seems there's either a bug with KQ4's logic script 120 (the intro script)
	// or flag 64 is not set correctly, which causes the erroneous behavior from the actors
	if (getGameID() == GID_KQ4 && !(v->flags & fUpdate) && (v->currentView == 172))
		return;

	currentVc = &currentVl->cel[n];
	v->celData = currentVc;
	v->xSize = currentVc->width;
	v->ySize = currentVc->height;
}

void AgiEngine::lSetLoop(VtEntry *v, int n) {
	ViewLoop *currentVl;
	debugC(7, kDebugLevelResources, "vt entry #%d, loop = %d", v->entry, n);

	// Added to avoid crash when leaving the arcade machine in MH1
	// -- not in AGI 2.917
	if (n >= v->numLoops)
		n = 0;

	v->currentLoop = n;
	currentVl = &_game.views[v->currentView].loop[v->currentLoop];

	v->numCels = currentVl->numCels;
	if (v->currentCel >= v->numCels)
		v->currentCel = 0;

	v->loopData = &_game.views[v->currentView].loop[n];
}

void AgiEngine::updateView(VtEntry *v) {
	int cel, lastCel;

	if (v->flags & fDontupdate) {
		v->flags &= ~fDontupdate;
		return;
	}

	cel = v->currentCel;
	lastCel = v->numCels - 1;

	switch (v->cycle) {
	case kCycleNormal:
		if (++cel > lastCel)
			cel = 0;
		break;
	case kCycleEndOfLoop:
		if (cel < lastCel) {
			debugC(5, kDebugLevelResources, "cel %d (last = %d)", cel + 1, lastCel);
			if (++cel != lastCel)
				break;
		}
		setflag(v->parm1, true);
		v->flags &= ~fCycling;
		v->direction = 0;
		v->cycle = kCycleNormal;
		break;
	case kCycleRevLoop:
		if (cel) {
			if (--cel)
				break;
		}
		setflag(v->parm1, true);
		v->flags &= ~fCycling;
		v->direction = 0;
		v->cycle = kCycleNormal;
		break;
	case kCycleReverse:
		if (cel == 0) {
			cel = lastCel;
		} else {
			cel--;
		}
		break;
	}

	setCel(v, cel);
}

/*
 * Public functions
 */

/**
 * Decode an AGI view resource.
 * This function decodes the raw data of the specified AGI view resource
 * and fills the corresponding views array element.
 * @param n number of view resource to decode
 */
int AgiEngine::decodeView(int n) {
	int loop, cel;
	uint8 *v, *lptr;
	uint16 lofs, cofs;
	ViewLoop *vl;
	ViewCel *vc;

	debugC(5, kDebugLevelResources, "decode_view(%d)", n);
	v = _game.views[n].rdata;

	assert(v != NULL);

	_game.views[n].agi256_2 = (READ_LE_UINT16(v) == 0xf00f); // Detect AGI256-2 views by their header bytes
	_game.views[n].descr = READ_LE_UINT16(v + 3) ? (char *)(v + READ_LE_UINT16(v + 3)) : (char *)(v + 3);

	// if no loops exist, return!
	if ((_game.views[n].numLoops = *(v + 2)) == 0)
		return errNoLoopsInView;

	// allocate memory for all views
	_game.views[n].loop = (ViewLoop *)calloc(_game.views[n].numLoops, sizeof(ViewLoop));

	if (_game.views[n].loop == NULL)
		return errNotEnoughMemory;

	// decode all of the loops in this view
	lptr = v + 5;		// first loop address

	for (loop = 0; loop < _game.views[n].numLoops; loop++, lptr += 2) {
		lofs = READ_LE_UINT16(lptr);	// loop header offset
		vl = &_game.views[n].loop[loop];	// the loop struct

		vl->numCels = *(v + lofs);
		debugC(6, kDebugLevelResources, "view %d, num_cels = %d", n, vl->numCels);
		vl->cel = (ViewCel *)calloc(vl->numCels, sizeof(ViewCel));

		if (vl->cel == NULL) {
			free(_game.views[n].loop);
			_game.views[n].numLoops = 0;
			return errNotEnoughMemory;
		}

		// decode the cells
		for (cel = 0; cel < vl->numCels; cel++) {
			cofs = lofs + READ_LE_UINT16(v + lofs + 1 + (cel * 2));
			vc = &vl->cel[cel];

			vc->width = *(v + cofs);
			vc->height = *(v + cofs + 1);

			if (!_game.views[n].agi256_2) {
				vc->transparency = *(v + cofs + 2) & 0xf;
				vc->mirrorLoop = (*(v + cofs + 2) >> 4) & 0x7;
				vc->mirror = (*(v + cofs + 2) >> 7) & 0x1;
			} else {
				// Mirroring is disabled for AGI256-2 views because
				// AGI256-2 uses whole 8 bits for the transparency variable.
				vc->transparency = *(v + cofs + 2);
				vc->mirrorLoop = 0;
				vc->mirror = 0;
			}

			// skip over width/height/trans|mirror data
			cofs += 3;

			vc->data = v + cofs;

			// If mirror_loop is pointing to the current loop,
			// then this is the original.
			if (vc->mirrorLoop == loop)
				vc->mirror = 0;
		}		// cel
	}			// loop

	return errOK;
}

/**
 * Unloads all data in a view resource
 * @param n number of view resource
 */
void AgiEngine::unloadView(int n) {
	int x;

	debugC(5, kDebugLevelResources, "discard view %d", n);
	if (~_game.dirView[n].flags & RES_LOADED)
		return;

	// Rebuild sprite list, see Sarien bug #779302
	_sprites->eraseBoth();
	_sprites->blitBoth();
	_sprites->commitBoth();

	// free all the loops
	for (x = 0; x < _game.views[n].numLoops; x++)
		free(_game.views[n].loop[x].cel);

	free(_game.views[n].loop);
	free(_game.views[n].rdata);

	_game.dirView[n].flags &= ~RES_LOADED;
}

/**
 * Set a view table entry to use the specified cel of the current loop.
 * @param v pointer to view table entry
 * @param n number of cel
 */
void AgiEngine::setCel(VtEntry *v, int n) {
	assert(v->viewData != NULL);
	assert(v->numCels >= n);

	lSetCel(v, n);

	// If position isn't appropriate, update it accordingly
	clipViewCoordinates(v);
}

/**
 * Restrict view table entry's position so it stays wholly inside the screen.
 * Also take horizon into account when clipping if not set to ignore it.
 * @param v pointer to view table entry
 */
void AgiEngine::clipViewCoordinates(VtEntry *v) {
	if (v->xPos + v->xSize > _WIDTH) {
		v->flags |= fUpdatePos;
		v->xPos = _WIDTH - v->xSize;
	}
	if (v->yPos - v->ySize + 1 < 0) {
		v->flags |= fUpdatePos;
		v->yPos = v->ySize - 1;
	}
	if (v->yPos <= _game.horizon && (~v->flags & fIgnoreHorizon)) {
		v->flags |= fUpdatePos;
		v->yPos = _game.horizon + 1;
	}

	if (getVersion() < 0x2000) {
		v->flags |= fDontupdate;
	}

}

/**
 * Set a view table entry to use the specified loop of the current view.
 * @param v pointer to view table entry
 * @param n number of loop
 */
void AgiEngine::setLoop(VtEntry *v, int n) {
	assert(v->viewData != NULL);
	assert(v->numLoops >= n);
	lSetLoop(v, n);
	setCel(v, v->currentCel);
}

/**
 * Set a view table entry to use the specified view resource.
 * @param v pointer to view table entry
 * @param n number of AGI view resource
 */
void AgiEngine::setView(VtEntry *v, int n) {
	v->viewData = &_game.views[n];
	v->currentView = n;
	v->numLoops = v->viewData->numLoops;
	v->viewReplaced = true;

	if (getVersion() < 0x2000) {
		v->stepSize = v->viewData->rdata[0];
		v->cycleTime = v->viewData->rdata[1];
		v->cycleTimeCount = 0;
	}
	setLoop(v, v->currentLoop >= v->numLoops ? 0 : v->currentLoop);
}

/**
 * Set the view table entry as updating.
 * @param v pointer to view table entry
 */
void AgiEngine::startUpdate(VtEntry *v) {
	if (~v->flags & fUpdate) {
		_sprites->eraseBoth();

		v->flags |= fUpdate;
		_sprites->blitBoth();
		_sprites->commitBoth();
	}
}

/**
 * Set the view table entry as non-updating.
 * @param v pointer to view table entry
 */
void AgiEngine::stopUpdate(VtEntry *v) {
	if (v->flags & fUpdate) {
		_sprites->eraseBoth();

		v->flags &= ~fUpdate;
		_sprites->blitBoth();
		_sprites->commitBoth();
	}
}

// loops to use according to direction and number of loops in
// the view resource
static int loopTable2[] = {
	0x04, 0x04, 0x00, 0x00, 0x00, 0x04, 0x01, 0x01, 0x01
};

static int loopTable4[] = {
	0x04, 0x03, 0x00, 0x00, 0x00, 0x02, 0x01, 0x01, 0x01
};

/**
 * Update view table entries.
 * This function is called at the end of each interpreter cycle
 * to update the view table entries and blit the sprites.
 */
void AgiEngine::updateViewtable() {
	VtEntry *v;
	int i, loop;

	i = 0;
	for (v = _game.viewTable; v < &_game.viewTable[MAX_VIEWTABLE]; v++) {
		if ((v->flags & (fAnimated | fUpdate | fDrawn)) != (fAnimated | fUpdate | fDrawn)) {
			continue;
		}

		i++;

		loop = 4;
		if (~v->flags & fFixLoop) {
			switch (v->numLoops) {
			case 2:
			case 3:
				loop = loopTable2[v->direction];
				break;
			case 4:
				loop = loopTable4[v->direction];
				break;
			default:
				// for KQ4
				if (getVersion() == 0x3086 || getGameID() == GID_KQ4)
					loop = loopTable4[v->direction];
				break;
			}
		}

		// AGI 2.272 (ddp, xmas) doesn't test step_time_count!
		if (loop != 4 && loop != v->currentLoop) {
			if (getVersion() <= 0x2272 ||
			    v->stepTimeCount == 1) {
				setLoop(v, loop);
			}
		}

		if (~v->flags & fCycling)
			continue;

		if (v->cycleTimeCount == 0)
			continue;

		if (--v->cycleTimeCount == 0) {
			updateView(v);
			v->cycleTimeCount = v->cycleTime;
		}
	}

	if (i) {
		_sprites->eraseUpdSprites();
		updatePosition();
		_sprites->blitUpdSprites();
		_sprites->commitUpdSprites();
		_game.viewTable[0].flags &= ~(fOnWater | fOnLand);
	}
}

bool AgiEngine::isEgoView(const VtEntry* v) {
	return v == _game.viewTable;
}

} // End of namespace Agi
