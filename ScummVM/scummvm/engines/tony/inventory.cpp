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

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "common/textconsole.h"
#include "tony/mpal/mpalutils.h"
#include "tony/inventory.h"
#include "tony/game.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*           RMInventory Methods
\****************************************************************************/

RMInventory::RMInventory() {
	_items = NULL;
	_state = CLOSED;
	_bCombining = false;
	_csModifyInterface = g_system->createMutex();
	_nItems = 0;

	Common::fill(_inv, _inv + 256, 0);
	_nInv = 0;
	_curPutY = 0;
	_curPutTime = 0;
	_curPos = 0;
	_bHasFocus = false;
	_nSelectObj = 0;
	_nCombine = 0;
	_bBlinkingRight = false;
	_bBlinkingLeft = false;
	_miniAction = 0;
}

RMInventory::~RMInventory() {
	close();
	g_system->deleteMutex(_csModifyInterface);
}

bool RMInventory::checkPointInside(const RMPoint &pt) {
	if (!GLOBALS._bCfgInvUp)
		return pt._y > RM_SY - 70;
	else
		return pt._y < 70;
}

void RMInventory::init() {
	// Create the main buffer
	create(RM_SX, 68);
	setPriority(185);

	// Setup the inventory
	_nInv = 0;
	_curPos = 0;
	_bCombining = false;

	// New items
	_nItems = 78;  // @@@ Number of takeable items
	_items = new RMInventoryItem[_nItems + 1];

	int curres = 10500;

	// Loop through the items
	for (int i = 0; i <= _nItems; i++) {
		// Load the items from the resource
		RMRes res(curres);
		assert(res.isValid());
		Common::SeekableReadStream *ds = res.getReadStream();

		// Initialize the MPAL inventory item by reading it in.
		_items[i]._icon.setInitCurPattern(false);
		_items[i]._icon.readFromStream(*ds);
		delete ds;

		// Puts in the default pattern 1
		_items[i]._pointer = NULL;
		_items[i]._status = 1;
		_items[i]._icon.setPattern(1);
		_items[i]._icon.doFrame(this, false);

		curres++;
		if (i == 0 || i == 28 || i == 29)
			continue;

		_items[i]._pointer = new RMGfxSourceBuffer8RLEByteAA[_items[i]._icon.numPattern()];

		for (int j = 0; j < _items[i]._icon.numPattern(); j++) {
			RMResRaw raw(curres);

			assert(raw.isValid());

			_items[i]._pointer[j].init((const byte *)raw, raw.width(), raw.height(), true);
			curres++;
		}
	}

	_items[28]._icon.setPattern(1);
	_items[29]._icon.setPattern(1);

	// Download interface
	RMRes res(RES_I_MINIINTER);
	assert(res.isValid());
	Common::SeekableReadStream *ds = res.getReadStream();
	_miniInterface.readFromStream(*ds);
	_miniInterface.setPattern(1);
	delete ds;

	// Create the text for hints on the mini interface
	_hints[0].setAlignType(RMText::HCENTER, RMText::VTOP);
	_hints[1].setAlignType(RMText::HCENTER, RMText::VTOP);
	_hints[2].setAlignType(RMText::HCENTER, RMText::VTOP);

	// The text is taken from MPAL for translation
	RMMessage msg1(15);
	RMMessage msg2(13);
	RMMessage msg3(14);

	_hints[0].writeText(msg1[0], 1); // Examine
	_hints[1].writeText(msg2[0], 1); // Take
	_hints[2].writeText(msg3[0], 1); // Use

	// Prepare initial inventory
	prepare();
	drawOT(Common::nullContext);
	clearOT();
}

void RMInventory::close() {
	// Has memory
	if (_items != NULL) {
		// Delete the item pointers
		for (int i = 0; i <= _nItems; i++)
			delete[] _items[i]._pointer;

		// Delete the items array
		delete[] _items;
		_items = NULL;
	}

	destroy();
}

void RMInventory::reset() {
	_state = CLOSED;
	endCombine();
}

void RMInventory::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	RMPoint pos;
	RMPoint pos2;
	RMGfxPrimitive *p;
	RMGfxPrimitive *p2;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	prim->setDst(RMPoint(0, _curPutY));
	g_system->lockMutex(_csModifyInterface);
	CORO_INVOKE_2(RMGfxWoodyBuffer::draw, bigBuf, prim);
	g_system->unlockMutex(_csModifyInterface);

	if (_state == SELECTING) {

		if (!GLOBALS._bCfgInvUp) {
			_ctx->pos.set((_nSelectObj + 1) * 64 - 20, RM_SY - 113);
			_ctx->pos2.set((_nSelectObj + 1) * 64 + 34, RM_SY - 150);
		} else {
			_ctx->pos.set((_nSelectObj + 1) * 64 - 20, 72 - 4); // The brown part is at the top :(
			_ctx->pos2.set((_nSelectObj + 1) * 64 + 34, 119 - 4);
		}

		_ctx->p = new RMGfxPrimitive(prim->_task, _ctx->pos);
		_ctx->p2 = new RMGfxPrimitive(prim->_task, _ctx->pos2);

		// Draw the mini interface
		CORO_INVOKE_2(_miniInterface.draw, bigBuf, _ctx->p);

		if (GLOBALS._bCfgInterTips) {
			if (_miniAction == 1) // Examine
				CORO_INVOKE_2(_hints[0].draw, bigBuf, _ctx->p2);
			else if (_miniAction == 2) // Talk
				CORO_INVOKE_2(_hints[1].draw, bigBuf, _ctx->p2);
			else if (_miniAction == 3) // Use
				CORO_INVOKE_2(_hints[2].draw, bigBuf, _ctx->p2);
		}

		delete _ctx->p;
		delete _ctx->p2;
	}

	CORO_END_CODE;
}

void RMInventory::removeThis(CORO_PARAM, bool &result) {
	if (_state == CLOSED)
		result = true;
	else
		result = false;
}

void RMInventory::removeItem(int code) {
	for (int i = 0; i < _nInv; i++) {
		if (_inv[i] == code - 10000) {
			g_system->lockMutex(_csModifyInterface);

			Common::copy(&_inv[i + 1], &_inv[i + 1] + (_nInv - i), &_inv[i]);
			_nInv--;

			prepare();
			drawOT(Common::nullContext);
			clearOT();
			g_system->unlockMutex(_csModifyInterface);
			return;
		}
	}
}

void RMInventory::addItem(int code) {
	if (code <= 10000 || code >= 10101) {
		// If we are here, it means that we are adding an item that should not be in the inventory
		warning("RMInventory::addItem(%d) - Cannot find a valid icon for this item, and then it will not be added to the inventory", code);
	} else {
		g_system->lockMutex(_csModifyInterface);
		if (_curPos + 8 == _nInv) {
			// Break through the inventory! On the flashing pattern
			_items[28]._icon.setPattern(2);
		}

		_inv[_nInv++] = code - 10000;

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	}
}

void RMInventory::changeItemStatus(uint32 code, uint32 dwStatus) {
	if (code <= 10000 || code >= 10101) {
		error("RMInventory::changeItemStatus(%d) - Specified object code is not valid", code);
	} else {
		g_system->lockMutex(_csModifyInterface);
		_items[code - 10000]._icon.setPattern(dwStatus);
		_items[code - 10000]._status = dwStatus;

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	}
}

void RMInventory::prepare() {
	for (int i = 1; i < RM_SX / 64 - 1; i++) {
		if (i - 1 + _curPos < _nInv)
			addPrim(new RMGfxPrimitive(&_items[_inv[i - 1 + _curPos]]._icon, RMPoint(i * 64, 0)));
		else
			addPrim(new RMGfxPrimitive(&_items[0]._icon, RMPoint(i * 64, 0)));
	}

	// Frecce
	addPrim(new RMGfxPrimitive(&_items[29]._icon, RMPoint(0, 0)));
	addPrim(new RMGfxPrimitive(&_items[28]._icon, RMPoint(640 - 64, 0)));
}

bool RMInventory::miniActive() {
	return _state == SELECTING;
}

bool RMInventory::haveFocus(const RMPoint &mpos) {
	// When we combine, have the focus only if we are on an arrow (to scroll)
	if (_state == OPENED && _bCombining && checkPointInside(mpos) && (mpos._x < 64 || mpos._x > RM_SX - 64))
		return true;

	// If the inventory is open, focus we we go over it
	if (_state == OPENED && !_bCombining && checkPointInside(mpos))
		return true;

	// If we are selecting a verb (and then right down), we always focus
	if (_state == SELECTING)
		return true;

	return false;
}

void RMInventory::endCombine() {
	_bCombining = false;
}

bool RMInventory::leftClick(const RMPoint &mpos, int &nCombineObj) {
	// The left click picks an item from your inventory to use it with the background
	int n = mpos._x / 64;

	if (_state == OPENED) {
		if (n > 0 && n < RM_SX / 64 - 1 && _inv[n - 1 + _curPos] != 0) {
			_bCombining = true; //m_state = COMBINING;
			_nCombine = _inv[n - 1 + _curPos];
			nCombineObj = _nCombine + 10000;

			g_vm->playUtilSFX(1);
			return true;
		}
	}

	// Click the right arrow
	if ((_state == OPENED) && _bBlinkingRight) {
		g_system->lockMutex(_csModifyInterface);
		_curPos++;

		if (_curPos + 8 >= _nInv) {
			_bBlinkingRight = false;
			_items[28]._icon.setPattern(1);
		}

		if (_curPos > 0) {
			_bBlinkingLeft = true;
			_items[29]._icon.setPattern(2);
		}

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	}

	// Click the left arrow
	else if ((_state == OPENED) && _bBlinkingLeft) {
		assert(_curPos > 0);
		g_system->lockMutex(_csModifyInterface);
		_curPos--;

		if (_curPos == 0) {
			_bBlinkingLeft = false;
			_items[29]._icon.setPattern(1);
		}

		if (_curPos + 8 < _nInv) {
			_bBlinkingRight = true;
			_items[28]._icon.setPattern(2);
		}

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	}

	return false;
}

void RMInventory::rightClick(const RMPoint &mpos) {
	assert(checkPointInside(mpos));

	if (_state == OPENED && !_bCombining) {
		// Open the context interface
		int n = mpos._x / 64;

		if (n > 0 && n < RM_SX / 64 - 1 && _inv[n - 1 + _curPos] != 0) {
			_state = SELECTING;
			_miniAction = 0;
			_nSelectObj = n - 1;

			g_vm->playUtilSFX(0);
		}
	}

	if ((_state == OPENED) && _bBlinkingRight) {
		g_system->lockMutex(_csModifyInterface);
		_curPos += 7;
		if (_curPos + 8 > _nInv)
			_curPos = _nInv - 8;

		if (_curPos + 8 <= _nInv) {
			_bBlinkingRight = false;
			_items[28]._icon.setPattern(1);
		}

		if (_curPos > 0) {
			_bBlinkingLeft = true;
			_items[29]._icon.setPattern(2);
		}

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	} else if ((_state == OPENED) && _bBlinkingLeft) {
		assert(_curPos > 0);
		g_system->lockMutex(_csModifyInterface);
		_curPos -= 7;
		if (_curPos < 0)
			_curPos = 0;

		if (_curPos == 0) {
			_bBlinkingLeft = false;
			_items[29]._icon.setPattern(1);
		}

		if (_curPos + 8 < _nInv) {
			_bBlinkingRight = true;
			_items[28]._icon.setPattern(2);
		}

		prepare();
		drawOT(Common::nullContext);
		clearOT();
		g_system->unlockMutex(_csModifyInterface);
	}
}

bool RMInventory::rightRelease(const RMPoint &mpos, RMTonyAction &curAction) {
	if (_state == SELECTING) {
		_state = OPENED;

		if (_miniAction == 1) { // Examine
			curAction = TA_EXAMINE;
			return true;
		} else if (_miniAction == 2) { // Talk
			curAction = TA_TALK;
			return true;
		} else if (_miniAction == 3) { // Use
			curAction = TA_USE;
			return true;
		}
	}

	return false;
}

#define INVSPEED 20

void RMInventory::doFrame(RMGfxTargetBuffer &bigBuf, RMPointer &ptr, RMPoint mpos, bool bCanOpen) {
	bool bNeedRedraw = false;

	if (_state != CLOSED) {
		// Clean up the OT list
		g_system->lockMutex(_csModifyInterface);
		clearOT();

		// DoFrame makes all the objects currently in the inventory be displayed
		// @@@ Maybe we should do all takeable objects? Please does not help
		for (int i = 0; i < _nInv; i++) {
			if (_items[_inv[i]]._icon.doFrame(this, false) && (i >= _curPos && i <= _curPos + 7))
				bNeedRedraw = true;
		}

		if ((_state == CLOSING || _state == OPENING || _state == OPENED) && checkPointInside(mpos)) {
			if (mpos._x > RM_SX - 64) {
				if (_curPos + 8 < _nInv && !_bBlinkingRight) {
					_items[28]._icon.setPattern(3);
					_bBlinkingRight = true;
					bNeedRedraw = true;
				}
			} else if (_bBlinkingRight) {
				_items[28]._icon.setPattern(2);
				_bBlinkingRight = false;
				bNeedRedraw = true;
			}

			if (mpos._x < 64) {
				if (_curPos > 0 && !_bBlinkingLeft) {
					_items[29]._icon.setPattern(3);
					_bBlinkingLeft = true;
					bNeedRedraw = true;
				}
			} else if (_bBlinkingLeft) {
				_items[29]._icon.setPattern(2);
				_bBlinkingLeft = false;
				bNeedRedraw = true;
			}
		}

		if (_items[28]._icon.doFrame(this, false))
			bNeedRedraw = true;

		if (_items[29]._icon.doFrame(this, false))
			bNeedRedraw = true;

		if (bNeedRedraw)
			prepare();

		g_system->unlockMutex(_csModifyInterface);
	}

	if (g_vm->getEngine()->getInput().getAsyncKeyState(Common::KEYCODE_i)) {
		GLOBALS._bCfgInvLocked = !GLOBALS._bCfgInvLocked;
	}

	if (_bCombining) { // m_state == COMBINING)
		ptr.setCustomPointer(&_items[_nCombine]._pointer[_items[_nCombine]._status - 1]);
		ptr.setSpecialPointer(RMPointer::PTR_CUSTOM);
	}

	if (!GLOBALS._bCfgInvUp) {
		if ((_state == CLOSED) && (mpos._y > RM_SY - 10 || GLOBALS._bCfgInvLocked) && bCanOpen) {
			if (!GLOBALS._bCfgInvNoScroll) {
				_state = OPENING;
				_curPutY = RM_SY - 1;
				_curPutTime = g_vm->getTime();
			} else {
				_state = OPENED;
				_curPutY = RM_SY - 68;
			}
		} else if (_state == OPENED) {
			if ((mpos._y < RM_SY - 70 && !GLOBALS._bCfgInvLocked) || !bCanOpen) {
				if (!GLOBALS._bCfgInvNoScroll) {
					_state = CLOSING;
					_curPutY = RM_SY - 68;
					_curPutTime = g_vm->getTime();
				} else {
					_state = CLOSED;
				}
			}
		} else if (_state == OPENING) {
			while (_curPutTime + INVSPEED < g_vm->getTime()) {
				_curPutY -= 3;
				_curPutTime += INVSPEED;
			}

			if (_curPutY <= RM_SY - 68) {
				_state = OPENED;
				_curPutY = RM_SY - 68;
			}

		} else if (_state == CLOSING) {
			while (_curPutTime + INVSPEED < g_vm->getTime()) {
				_curPutY += 3;
				_curPutTime += INVSPEED;
			}

			if (_curPutY > 480)
				_state = CLOSED;
		}
	} else {
		if ((_state == CLOSED) && (mpos._y < 10 || GLOBALS._bCfgInvLocked) && bCanOpen) {
			if (!GLOBALS._bCfgInvNoScroll) {
				_state = OPENING;
				_curPutY = - 68;
				_curPutTime = g_vm->getTime();
			} else {
				_state = OPENED;
				_curPutY = 0;
			}
		} else if (_state == OPENED) {
			if ((mpos._y > 70 && !GLOBALS._bCfgInvLocked) || !bCanOpen) {
				if (!GLOBALS._bCfgInvNoScroll) {
					_state = CLOSING;
					_curPutY = -2;
					_curPutTime = g_vm->getTime();
				} else {
					_state = CLOSED;
				}
			}
		} else if (_state == OPENING) {
			while (_curPutTime + INVSPEED < g_vm->getTime()) {
				_curPutY += 3;
				_curPutTime += INVSPEED;
			}

			if (_curPutY >= 0) {
				_state = OPENED;
				_curPutY = 0;
			}
		} else if (_state == CLOSING) {
			while (_curPutTime + INVSPEED < g_vm->getTime()) {
				_curPutY -= 3;
				_curPutTime += INVSPEED;
			}

			if (_curPutY < -68)
				_state = CLOSED;
		}
	}

	if (_state == SELECTING) {
		int startx = (_nSelectObj + 1) * 64 - 20;
		int starty;

		if (!GLOBALS._bCfgInvUp)
			starty = RM_SY - 109;
		else
			starty = 70;

		// Make sure it is on one of the verbs
		if (mpos._y > starty && mpos._y < starty + 45) {
			if (mpos._x > startx && mpos._x < startx + 40) {
				if (_miniAction != 1) {
					_miniInterface.setPattern(2);
					_miniAction = 1;
					g_vm->playUtilSFX(1);
				}
			} else if (mpos._x >= startx + 40 && mpos._x < startx + 80) {
				if (_miniAction != 2) {
					_miniInterface.setPattern(3);
					_miniAction = 2;
					g_vm->playUtilSFX(1);
				}
			} else if (mpos._x >= startx + 80 && mpos._x < startx + 108) {
				if (_miniAction != 3) {
					_miniInterface.setPattern(4);
					_miniAction = 3;
					g_vm->playUtilSFX(1);
				}
			} else {
				_miniInterface.setPattern(1);
				_miniAction = 0;
			}
		} else  {
			_miniInterface.setPattern(1);
			_miniAction = 0;
		}

		// Update the mini-interface
		_miniInterface.doFrame(&bigBuf, false);
	}

	if ((_state != CLOSED) && !_nInList) {
		bigBuf.addPrim(new RMGfxPrimitive(this));
	}
}

bool RMInventory::itemInFocus(const RMPoint &mpt) {
	if ((_state == OPENED || _state == OPENING) && checkPointInside(mpt))
		return true;
	else
		return false;
}

RMItem *RMInventory::whichItemIsIn(const RMPoint &mpt) {
	if (_state == OPENED) {
		if (checkPointInside(mpt)) {
			int n = mpt._x / 64;
			if (n > 0 && n < RM_SX / 64 - 1 && _inv[n - 1 + _curPos] != 0 && (!_bCombining || _inv[n - 1 + _curPos] != _nCombine))
				return &_items[_inv[n - 1 + _curPos]]._icon;
		}
	}

	return NULL;
}

int RMInventory::getSaveStateSize() {
	//     m_inv   pattern   m_nInv
	return 256 * 4 + 256 * 4   +  4;
}

void RMInventory::saveState(byte *state) {
	WRITE_LE_UINT32(state, _nInv);
	state += 4;
	for (int i = 0; i < 256; ++i) {
		WRITE_LE_UINT32(state, _inv[i]);
		state += 4;
	}

	int x;
	for (int i = 0; i < 256; i++) {
		if (i < _nItems)
			x = _items[i]._status;
		else
			x = 0;

		WRITE_LE_UINT32(state, x);
		state += 4;
	}
}

int RMInventory::loadState(byte *state) {
	_nInv = READ_LE_UINT32(state);
	state += 4;
	for (int i = 0; i < 256; ++i) {
		_inv[i] = READ_LE_UINT32(state);
		state += 4;
	}

	int x;
	for (int i = 0; i < 256; i++) {
		x = READ_LE_UINT32(state);
		state += 4;

		if (i < _nItems) {
			_items[i]._status = x;
			_items[i]._icon.setPattern(x);
		}
	}

	_curPos = 0;
	_bCombining = false;

	_items[29]._icon.setPattern(1);

	if (_nInv > 8)
		_items[28]._icon.setPattern(2);
	else
		_items[28]._icon.setPattern(1);

	prepare();
	drawOT(Common::nullContext);
	clearOT();

	return getSaveStateSize();
}

RMInventory &RMInventory::operator+=(RMItem *item) {
	addItem(item->mpalCode());
	return *this;
}

RMInventory &RMInventory::operator+=(RMItem &item) {
	addItem(item.mpalCode());
	return *this;
}

RMInventory &RMInventory::operator+=(int code) {
	addItem(code);
	return *this;
}

/****************************************************************************\
*           RMInterface methods
\****************************************************************************/

RMInterface::RMInterface() : RMGfxSourceBuffer8RLEByte() {
	_bActive = _bPerorate = false;
	_lastHotZone = -1;
}

RMInterface::~RMInterface() {
}

bool RMInterface::active() {
	return _bActive;
}

int RMInterface::onWhichBox(RMPoint pt) {
	pt -= _openStart;

	// Check how many verbs you have to consider
	int max = 4;
	if (_bPerorate)
		max = 5;

	// Find the verb
	for (int i = 0; i < max; i++) {
		if (_hotbbox[i].ptInRect(pt))
			return i;
	}

	// Found no verb
	return -1;
}

void RMInterface::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	int h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	prim->getDst().topLeft() = _openStart;
	CORO_INVOKE_2(RMGfxSourceBuffer8RLEByte::draw, bigBuf, prim);

	// Check if there is a draw hot zone
	_ctx->h = onWhichBox(_mpos);
	if (_ctx->h != -1) {
		prim->getDst().topLeft() = _openStart;
		CORO_INVOKE_2(_hotzone[_ctx->h].draw, bigBuf, prim);

		if (_lastHotZone != _ctx->h) {
			_lastHotZone = _ctx->h;
			g_vm->playUtilSFX(1);
		}

		if (GLOBALS._bCfgInterTips) {
			prim->getDst().topLeft() = _openStart + RMPoint(70, 177);
			CORO_INVOKE_2(_hints[_ctx->h].draw, bigBuf, prim);
		}
	} else
		_lastHotZone = -1;

	CORO_END_CODE;
}

void RMInterface::doFrame(RMGfxTargetBuffer &bigBuf, RMPoint mousepos) {
	// If needed, add to the OT schedule list
	if (!_nInList && _bActive)
		bigBuf.addPrim(new RMGfxPrimitive(this));

	_mpos = mousepos;
}

void RMInterface::clicked(const RMPoint &mousepos) {
	_bActive = true;
	_openPos = mousepos;

	// Calculate the top left corner of the interface
	_openStart = _openPos - RMPoint(_dimx / 2, _dimy / 2);
	_lastHotZone = -1;

	// Keep it inside the screen
	if (_openStart._x < 0)
		_openStart._x = 0;
	if (_openStart._y < 0)
		_openStart._y = 0;
	if (_openStart._x + _dimx > RM_SX)
		_openStart._x = RM_SX - _dimx;
	if (_openStart._y + _dimy > RM_SY)
		_openStart._y = RM_SY - _dimy;

	// Play the sound effect
	g_vm->playUtilSFX(0);
}

bool RMInterface::released(const RMPoint &mousepos, RMTonyAction &action) {
	if (!_bActive)
		return false;

	_bActive = false;

	switch (onWhichBox(mousepos)) {
	case 0:
		action = TA_TAKE;
		break;

	case 1:
		action = TA_TALK;
		break;

	case 2:
		action = TA_USE;
		break;

	case 3:
		action = TA_EXAMINE;
		break;

	case 4:
		action = TA_PERORATE;
		break;

	default: // No verb
		return false;
	}

	return true;
}

void RMInterface::reset() {
	_bActive = false;
}

void RMInterface::setPerorate(bool bOn) {
	_bPerorate = bOn;
}

bool RMInterface::getPerorate() {
	return _bPerorate;
}

void RMInterface::init() {
	RMResRaw inter(RES_I_INTERFACE);
	RMRes pal(RES_I_INTERPPAL);

	setPriority(191);

	RMGfxSourceBuffer::init(inter, inter.width(), inter.height());
	loadPaletteWA(RES_I_INTERPAL);

	for (int i = 0; i < 5; i++) {
		RMResRaw part(RES_I_INTERP1 + i);

		_hotzone[i].init(part, part.width(), part.height());
		_hotzone[i].loadPaletteWA(pal);
	}

	_hotbbox[0].setRect(126, 123, 159, 208); // Take
	_hotbbox[1].setRect(90, 130, 125, 186); // About
	_hotbbox[2].setRect(110, 60, 152, 125);
	_hotbbox[3].setRect(56, 51, 93, 99);
	_hotbbox[4].setRect(51, 105, 82, 172);

	_hints[0].setAlignType(RMText::HRIGHT, RMText::VTOP);
	_hints[1].setAlignType(RMText::HRIGHT, RMText::VTOP);
	_hints[2].setAlignType(RMText::HRIGHT, RMText::VTOP);
	_hints[3].setAlignType(RMText::HRIGHT, RMText::VTOP);
	_hints[4].setAlignType(RMText::HRIGHT, RMText::VTOP);

	// The text is taken from MPAL for translation
	RMMessage msg0(12);
	RMMessage msg1(13);
	RMMessage msg2(14);
	RMMessage msg3(15);
	RMMessage msg4(16);

	_hints[0].writeText(msg0[0], 1); // Take
	_hints[1].writeText(msg1[0], 1); // Talk
	_hints[2].writeText(msg2[0], 1); // Use
	_hints[3].writeText(msg3[0], 1); // Examine
	_hints[4].writeText(msg4[0], 1); // Show Yourself

	_bActive = false;
	_bPerorate = false;
	_lastHotZone = 0;
}

void RMInterface::close() {
	destroy();

	for (int i = 0; i < 5; i++)
		_hotzone[i].destroy();
}

} // End of namespace Tony
