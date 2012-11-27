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
#include "agi/graphics.h"

namespace Agi {

/**
 * Sprite structure.
 * This structure holds information on visible and priority data of
 * a rectangular area of the AGI screen. Sprites are chained in two
 * circular lists, one for updating and other for non-updating sprites.
 */
struct Sprite {
	VtEntry *v;		/**< pointer to view table entry */
	int16 xPos;			/**< x coordinate of the sprite */
	int16 yPos;			/**< y coordinate of the sprite */
	int16 xSize;			/**< width of the sprite */
	int16 ySize;			/**< height of the sprite */
	uint8 *buffer;			/**< buffer to store background data */
};

/*
 * Sprite pool replaces dynamic allocation
 */
#undef ALLOC_DEBUG


#define POOL_SIZE 68000		// Gold Rush mine room needs > 50000
							// Speeder bike challenge needs > 67000

void *SpritesMgr::poolAlloc(int size) {
	uint8 *x;

	// Adjust size to sizeof(void *) boundary to prevent data misalignment
	// errors.
	const int alignPadding = sizeof(void *) - 1;
	size = (size + alignPadding) & ~alignPadding;

	x = _poolTop;
	_poolTop += size;

	if (_poolTop >= (uint8 *)_spritePool + POOL_SIZE) {
		debugC(1, kDebugLevelMain | kDebugLevelResources, "not enough memory");
		_poolTop = x;
		return NULL;
	}

	return x;
}

// Note: it's critical that pool_release() is called in the exact
//         reverse order of pool_alloc()
void SpritesMgr::poolRelease(void *s) {
	_poolTop = (uint8 *)s;
}

/*
 * Blitter functions
 */

// Blit one pixel considering the priorities
void SpritesMgr::blitPixel(uint8 *p, uint8 *end, uint8 col, int spr, int width, int *hidden) {
	int epr = 0, pr = 0;	// effective and real priorities

	// CM: priority 15 overrides control lines and is ignored when
	//     tracking effective priority. This tweak is needed to fix
	//     Sarien bug #451768, and should not affect Sierra games because
	//     sprites shouldn't have priority 15 (like the AGI Mouse
	//     demo "mouse pointer")
	//
	// Update: this solution breaks other games, and can't be used.

	if (p >= end)
		return;

	// Check if we're on a control line
	if ((pr = *p & 0xf0) < 0x30) {
		uint8 *p1;
		// Yes, get effective priority going down
		for (p1 = p; p1 < end && (epr = *p1 & 0xf0) < 0x30; p1 += width)
			;
		if (p1 >= end)
			epr = 0x40;
	} else {
		epr = pr;
	}

	if (spr >= epr) {
		// Keep control line information visible, but put our
		// priority over water (0x30) surface
		if (_vm->getFeatures() & (GF_AGI256 | GF_AGI256_2))
			*(p + FROM_SBUF16_TO_SBUF256_OFFSET) = col; // Write to 256 color buffer
		else
			*p = (pr < 0x30 ? pr : spr) | col; // Write to 16 color (+control line/priority info) buffer

		*hidden = false;

		// Except if our priority is 15, which should never happen
		// (fixes Sarien bug #451768)
		//
		// Update: breaks other games, can't be used
		//
		// if (spr == 0xf0)
		//      *p = spr | col;
	}
}


int SpritesMgr::blitCel(int x, int y, int spr, ViewCel *c, bool agi256_2) {
	uint8 *p0, *p, *q = NULL, *end;
	int i, j, t, m, col;
	int hidden = true;

	// Fixes Sarien bug #477841 (crash in PQ1 map C4 when y == -2)
	if (y < 0)
		y = 0;
	if (x < 0)
		x = 0;
	if (y >= _HEIGHT)
		y = _HEIGHT - 1;
	if (x >= _WIDTH)
		x = _WIDTH - 1;

	q = c->data;
	t = c->transparency;
	m = c->mirror;
	spr <<= 4;
	p0 = &_vm->_game.sbuf16c[x + y * _WIDTH + m * (c->width - 1)];

	end = _vm->_game.sbuf16c + _WIDTH * _HEIGHT;

	for (i = 0; i < c->height; i++) {
		p = p0;
		while (*q) {
			col = agi256_2 ? *q : (*q & 0xf0) >> 4; // Uses whole byte for color info with AGI256-2
			for (j = agi256_2 ? 1 : *q & 0x0f; j; j--, p += 1 - 2 * m) { // No RLE with AGI256-2
				if (col != t) {
					blitPixel(p, end, col, spr, _WIDTH, &hidden);
				}
			}
			q++;
		}
		p0 += _WIDTH;
		q++;
	}

	return hidden;
}

void SpritesMgr::objsSaveArea(Sprite *s) {
	int y;
	int16 xPos = s->xPos, yPos = s->yPos;
	int16 xSize = s->xSize, ySize = s->ySize;
	uint8 *p0, *q;

	if (xPos + xSize > _WIDTH)
		xSize = _WIDTH - xPos;

	if (xPos < 0) {
		xSize += xPos;
		xPos = 0;
	}

	if (yPos + ySize > _HEIGHT)
		ySize = _HEIGHT - yPos;

	if (yPos < 0) {
		ySize += yPos;
		yPos = 0;
	}

	if (xSize <= 0 || ySize <= 0)
		return;

	p0 = &_vm->_game.sbuf[xPos + yPos * _WIDTH];
	q = s->buffer;
	for (y = 0; y < ySize; y++) {
		memcpy(q, p0, xSize);
		q += xSize;
		p0 += _WIDTH;
	}
}

void SpritesMgr::objsRestoreArea(Sprite *s) {
	int y, offset;
	int16 xPos = s->xPos, yPos = s->yPos;
	int16 xSize = s->xSize, ySize = s->ySize;
	uint8 *q;
	uint32 pos0;

	if (xPos + xSize > _WIDTH)
		xSize = _WIDTH - xPos;

	if (xPos < 0) {
		xSize += xPos;
		xPos = 0;
	}

	if (yPos + ySize > _HEIGHT)
		ySize = _HEIGHT - yPos;

	if (yPos < 0) {
		ySize += yPos;
		yPos = 0;
	}

	if (xSize <= 0 || ySize <= 0)
		return;

	pos0 = xPos + yPos * _WIDTH;
	q = s->buffer;
	offset = _vm->_game.lineMinPrint * CHAR_LINES;
	for (y = 0; y < ySize; y++) {
		memcpy(&_vm->_game.sbuf[pos0], q, xSize);
		_gfx->putPixelsA(xPos, yPos + y + offset, xSize, &_vm->_game.sbuf16c[pos0]);
		q += xSize;
		pos0 += _WIDTH;
	}

	// WORKAROUND (see ScummVM bug #1945716)
	// When set.view command is called, current code cannot detect  this situation while updating
	// Thus we force removal of the old sprite
	if (s->v && s->v->viewReplaced) {
		commitBlock(xPos, yPos, xPos + xSize, yPos + ySize);
		s->v->viewReplaced = false;
	}
}


/**
 * Condition to determine whether a sprite will be in the 'updating' list.
 */
bool SpritesMgr::testUpdating(VtEntry *v, AgiEngine *agi) {
	// Sanity check (see Sarien bug #779302)
	if (~agi->_game.dirView[v->currentView].flags & RES_LOADED)
		return false;

	return (v->flags & (fAnimated | fUpdate | fDrawn)) == (fAnimated | fUpdate | fDrawn);
}

/**
 * Condition to determine whether a sprite will be in the 'non-updating' list.
 */
bool SpritesMgr::testNotUpdating(VtEntry *v, AgiEngine *vm) {
	// Sanity check (see Sarien bug #779302)
	if (~vm->_game.dirView[v->currentView].flags & RES_LOADED)
		return false;

	return (v->flags & (fAnimated | fUpdate | fDrawn)) == (fAnimated | fDrawn);
}

/**
 * Convert sprite priority to y value.
 */
int SpritesMgr::prioToY(int p) {
	int i;

	if (p == 0)
		return -1;

	for (i = 167; i >= 0; i--) {
		if (_vm->_game.priTable[i] < p)
			return i;
	}

	return -1;		// (p - 5) * 12 + 48;
}

/**
 * Create and initialize a new sprite structure.
 */
Sprite *SpritesMgr::newSprite(VtEntry *v) {
	Sprite *s;
	s = (Sprite *)poolAlloc(sizeof(Sprite));
	if (s == NULL)
		return NULL;

	s->v = v;		// link sprite to associated view table entry
	s->xPos = v->xPos;
	s->yPos = v->yPos - v->ySize + 1;
	s->xSize = v->xSize;
	s->ySize = v->ySize;
	s->buffer = (uint8 *)poolAlloc(s->xSize * s->ySize);
	v->s = s;		// link view table entry to this sprite

	return s;
}

/**
 * Insert sprite in the specified sprite list.
 */
void SpritesMgr::sprAddlist(SpriteList &l, VtEntry *v) {
	Sprite *s = newSprite(v);
	l.push_back(s);
}

/**
 * Sort sprites from lower y values to build a sprite list.
 */
void SpritesMgr::buildList(SpriteList &l, bool (*test)(VtEntry *, AgiEngine *)) {
	int i, j, k;
	VtEntry *v;
	VtEntry *entry[0x100];
	int yVal[0x100];
	int minY = 0xff, minIndex = 0;

	// fill the arrays with all sprites that satisfy the 'test'
	// condition and their y values
	i = 0;
	for (v = _vm->_game.viewTable; v < &_vm->_game.viewTable[MAX_VIEWTABLE]; v++) {
		if ((*test)(v, _vm)) {
			entry[i] = v;
			yVal[i] = v->flags & fFixedPriority ? prioToY(v->priority) : v->yPos;
			i++;
		}
	}

	debugC(5, kDebugLevelSprites, "buildList() --> entries %d", i);

	// now look for the smallest y value in the array and put that
	// sprite in the list
	for (j = 0; j < i; j++) {
		minY = 0xff;

		for (k = 0; k < i; k++) {
			if (yVal[k] < minY) {
				minIndex = k;
				minY = yVal[k];
			}
		}

		yVal[minIndex] = 0xff;
		sprAddlist(l, entry[minIndex]);
	}
}

/**
 * Build list of updating sprites.
 */
void SpritesMgr::buildUpdBlitlist() {
	buildList(_sprUpd, testUpdating);
}

/**
 * Build list of non-updating sprites.
 */
void SpritesMgr::buildNonupdBlitlist() {
	buildList(_sprNonupd, testNotUpdating);
}

/**
 * Clear the given sprite list.
 */
void SpritesMgr::freeList(SpriteList &l) {
	SpriteList::iterator iter;
	for (iter = l.reverse_begin(); iter != l.end(); ) {
		Sprite* s = *iter;

		poolRelease(s->buffer);
		poolRelease(s);
		iter = l.reverse_erase(iter);
	}
}

/**
 * Copy sprites from the pic buffer to the screen buffer, and check if
 * sprites of the given list have moved.
 */
void SpritesMgr::commitSprites(SpriteList &l, bool immediate) {
	SpriteList::iterator iter;
	for (iter = l.begin(); iter != l.end(); ++iter) {
		Sprite *s = *iter;
		int x1, y1, x2, y2;

		x1 = MIN((int)MIN(s->v->xPos, s->v->xPos2), MIN(s->v->xPos + s->v->celData->width, s->v->xPos2 + s->v->celData2->width));
		x2 = MAX((int)MAX(s->v->xPos, s->v->xPos2), MAX(s->v->xPos + s->v->celData->width, s->v->xPos2 + s->v->celData2->width));
		y1 = MIN((int)MIN(s->v->yPos, s->v->yPos2), MIN(s->v->yPos - s->v->celData->height, s->v->yPos2 - s->v->celData2->height));
		y2 = MAX((int)MAX(s->v->yPos, s->v->yPos2), MAX(s->v->yPos - s->v->celData->height, s->v->yPos2 - s->v->celData2->height));

		s->v->celData2 = s->v->celData;

		commitBlock(x1, y1, x2, y2, immediate);

		if (s->v->stepTimeCount != s->v->stepTime)
			continue;

		if (s->v->xPos == s->v->xPos2 && s->v->yPos == s->v->yPos2) {
			s->v->flags |= fDidntMove;
			continue;
		}

		s->v->xPos2 = s->v->xPos;
		s->v->yPos2 = s->v->yPos;
		s->v->flags &= ~fDidntMove;
	}
}

/**
 * Erase all sprites in the given list.
 */
void SpritesMgr::eraseSprites(SpriteList &l) {
	SpriteList::iterator iter;
	for (iter = l.reverse_begin(); iter != l.end(); --iter) {
		Sprite *s = *iter;
		objsRestoreArea(s);
	}

	freeList(l);
}

/**
 * Blit all sprites in the given list.
 */
void SpritesMgr::blitSprites(SpriteList& l) {
	int hidden;
	SpriteList::iterator iter;

	for (iter = l.begin(); iter != l.end(); ++iter) {
		Sprite *s = *iter;

		objsSaveArea(s);
		debugC(8, kDebugLevelSprites, "blitSprites(): s->v->entry = %d (prio %d)", s->v->entry, s->v->priority);
		hidden = blitCel(s->xPos, s->yPos, s->v->priority, s->v->celData, s->v->viewData->agi256_2);

		if (s->v->entry == 0) {	// if ego, update f1
			_vm->setflag(fEgoInvisible, hidden);
		}
	}
}

/*
 * Public functions
 */

void SpritesMgr::commitUpdSprites() {
	commitSprites(_sprUpd);
}

void SpritesMgr::commitNonupdSprites() {
	commitSprites(_sprNonupd);
}

// check moves in both lists
void SpritesMgr::commitBoth() {
	commitUpdSprites();
	commitNonupdSprites();
}

/**
 * Erase updating sprites.
 * This function follows the list of all updating sprites and restores
 * the visible and priority data of their background buffers back to
 * the AGI screen.
 *
 * @see erase_nonupd_sprites()
 * @see erase_both()
 */
void SpritesMgr::eraseUpdSprites() {
	eraseSprites(_sprUpd);
}

/**
 * Erase non-updating sprites.
 * This function follows the list of all non-updating sprites and restores
 * the visible and priority data of their background buffers back to
 * the AGI screen.
 *
 * @see erase_upd_sprites()
 * @see erase_both()
 */
void SpritesMgr::eraseNonupdSprites() {
	eraseSprites(_sprNonupd);
}

/**
 * Erase all sprites.
 * This function follows the lists of all updating and non-updating
 * sprites and restores the visible and priority data of their background
 * buffers back to the AGI screen.
 *
 * @see erase_upd_sprites()
 * @see erase_nonupd_sprites()
 */
void SpritesMgr::eraseBoth() {
	eraseUpdSprites();
	eraseNonupdSprites();
}

/**
 * Blit updating sprites.
 * This function follows the list of all updating sprites and blits
 * them on the AGI screen.
 *
 * @see blit_nonupd_sprites()
 * @see blit_both()
 */
void SpritesMgr::blitUpdSprites() {
	debugC(7, kDebugLevelSprites, "blitUpdSprites()");
	buildUpdBlitlist();
	blitSprites(_sprUpd);
}

/**
 * Blit non-updating sprites.
 * This function follows the list of all non-updating sprites and blits
 * them on the AGI screen.
 *
 * @see blit_upd_sprites()
 * @see blit_both()
 */
void SpritesMgr::blitNonupdSprites() {
	debugC(7, kDebugLevelSprites, "blitNonupdSprites()");
	buildNonupdBlitlist();
	blitSprites(_sprNonupd);
}

/**
 * Blit all sprites.
 * This function follows the lists of all updating and non-updating
 * sprites and blits them on the AGI screen.
 *
 * @see blit_upd_sprites()
 * @see blit_nonupd_sprites()
 */
void SpritesMgr::blitBoth() {
	blitNonupdSprites();
	blitUpdSprites();
}

/**
 * Add view to picture.
 * This function is used to implement the add.to.pic AGI command. It
 * copies the specified cel from a view resource on the current picture.
 * This cel is not a sprite, it can't be moved or removed.
 * @param view  number of view resource
 * @param loop  number of loop in the specified view resource
 * @param cel   number of cel in the specified loop
 * @param x     x coordinate to place the view
 * @param y     y coordinate to place the view
 * @param pri   priority to use
 * @param mar   if < 4, create a margin around the the base of the cel
 */
void SpritesMgr::addToPic(int view, int loop, int cel, int x, int y, int pri, int mar) {
	ViewCel *c = NULL;
	int x1, y1, x2, y2, y3;
	uint8 *p1, *p2;

	debugC(3, kDebugLevelSprites, "addToPic(view=%d, loop=%d, cel=%d, x=%d, y=%d, pri=%d, mar=%d)", view, loop, cel, x, y, pri, mar);

	_vm->recordImageStackCall(ADD_VIEW, view, loop, cel, x, y, pri, mar);

	// Was hardcoded to 8, changed to pri_table[y] to fix Gold
	// Rush (see Sarien bug #587558)
	if (pri == 0)
		pri = _vm->_game.priTable[y];

	c = &_vm->_game.views[view].loop[loop].cel[cel];

	x1 = x;
	y1 = y - c->height + 1;
	x2 = x + c->width - 1;
	y2 = y;

	if (x1 < 0) {
		x2 -= x1;
		x1 = 0;
	}
	if (y1 < 0) {
		y2 -= y1;
		y1 = 0;
	}
	if (x2 >= _WIDTH)
		x2 = _WIDTH - 1;
	if (y2 >= _HEIGHT)
		y2 = _HEIGHT - 1;

	eraseBoth();

	debugC(4, kDebugLevelSprites, "blitCel(%d, %d, %d, c)", x, y, pri);
	blitCel(x1, y1, pri, c, _vm->_game.views[view].agi256_2);

	// If margin is 0, 1, 2, or 3, the base of the cel is
	// surrounded with a rectangle of the corresponding priority.
	// If margin >= 4, this extra margin is not shown.
	//
	// -1 indicates ignore and is set for V1
	if (mar < 4 && mar != -1) {
		// add rectangle around object, don't clobber control
		// info in priority data. The box extends to the end of
		// its priority band!
		y3 = (y2 / 12) * 12;

		// SQ1 needs +1 (see Sarien bug #810331)
		if (_vm->getGameID() == GID_SQ1)
			y3++;

		// don't let box extend below y.
		if (y3 > y2) y3 = y2;

		p1 = &_vm->_game.sbuf16c[x1 + y3 * _WIDTH];
		p2 = &_vm->_game.sbuf16c[x2 + y3 * _WIDTH];

		for (y = y3; y <= y2; y++) {
			if ((*p1 >> 4) >= 4)
				*p1 = (mar << 4) | (*p1 & 0x0f);

			if ((*p2 >> 4) >= 4)
				*p2 = (mar << 4) | (*p2 & 0x0f);

			p1 += _WIDTH;
			p2 += _WIDTH;
		}

		debugC(4, kDebugLevelSprites, "pri box: %d %d %d %d (%d)", x1, y3, x2, y2, mar);
		p1 = &_vm->_game.sbuf16c[x1 + y3 * _WIDTH];
		p2 = &_vm->_game.sbuf16c[x1 + y2 * _WIDTH];
		for (x = x1; x <= x2; x++) {
			if ((*p1 >> 4) >= 4)
				*p1 = (mar << 4) | (*p1 & 0x0f);

			if ((*p2 >> 4) >= 4)
				*p2 = (mar << 4) | (*p2 & 0x0f);

			p1++;
			p2++;
		}
	}

	blitBoth();

	commitBlock(x1, y1, x2, y2, true);
}

/**
 * Show object and description
 * This function shows an object from the player's inventory, displaying
 * a message box with the object description.
 * @param n  Number of the object to show
 */
void SpritesMgr::showObj(int n) {
	ViewCel *c;
	Sprite s;
	int x1, y1, x2, y2;

	_vm->agiLoadResource(rVIEW, n);
	if (!(c = &_vm->_game.views[n].loop[0].cel[0]))
		return;

	x1 = (_WIDTH - c->width) / 2;
	y1 = 112;
	x2 = x1 + c->width - 1;
	y2 = y1 + c->height - 1;

	s.xPos = x1;
	s.yPos = y1;
	s.xSize = c->width;
	s.ySize = c->height;
	s.buffer = (uint8 *)malloc(s.xSize * s.ySize);
	s.v = 0;

	objsSaveArea(&s);
	blitCel(x1, y1, 15, c, _vm->_game.views[n].agi256_2);
	commitBlock(x1, y1, x2, y2, true);
	_vm->messageBox(_vm->_game.views[n].descr);
	objsRestoreArea(&s);
	commitBlock(x1, y1, x2, y2, true);

	free(s.buffer);
}

void SpritesMgr::commitBlock(int x1, int y1, int x2, int y2, bool immediate) {
	int i, w, offset;
	uint8 *q;

	if (!_vm->_game.pictureShown)
		return;

	x1 = CLIP(x1, 0, _WIDTH - 1);
	x2 = CLIP(x2, 0, _WIDTH - 1);
	y1 = CLIP(y1, 0, _HEIGHT - 1);
	y2 = CLIP(y2, 0, _HEIGHT - 1);

	// Check if a window is active, and clip the block commited to exclude the
	// window's contents. Fixes bug #3295652, and partially fixes bug #3080415.
	AgiBlock &window = _vm->_game.window;
	if (window.active) {
		if (y1 < window.y2 && y2 > window.y2 && (x1 < window.x2 || x2 > window.x1)) {
			// The top of the block covers the bottom of the window
			y1 = window.y2;
		}

		if (y1 < window.y1 && y2 > window.y1 && (x1 < window.x2 || x2 > window.x1)) {
			// The bottom of the block covers the top of the window
			y2 = window.y1;
		}
	}

	debugC(7, kDebugLevelSprites, "commitBlock(%d, %d, %d, %d)", x1, y1, x2, y2);

	w = x2 - x1 + 1;
	q = &_vm->_game.sbuf16c[x1 + _WIDTH * y1];
	offset = _vm->_game.lineMinPrint * CHAR_LINES;

	for (i = y1; i <= y2; i++) {
		_gfx->putPixelsA(x1, i + offset, w, q);
		q += _WIDTH;
	}

	_gfx->flushBlockA(x1, y1 + offset, x2, y2 + offset);

	if (immediate)
		_gfx->doUpdate();
}

SpritesMgr::SpritesMgr(AgiEngine *agi, GfxMgr *gfx) {
	_vm = agi;
	_gfx = gfx;

	_spritePool = (uint8 *)malloc(POOL_SIZE);
	_poolTop = _spritePool;
}

SpritesMgr::~SpritesMgr() {
	free(_spritePool);
}

} // End of namespace Agi
