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

#include "common/savefile.h"
#include "tony/mpal/lzo.h"
#include "tony/mpal/mpalutils.h"
#include "tony/custom.h"
#include "tony/gfxengine.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*       RMGfxEngine Methods
\****************************************************************************/

void exitAllIdles(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	int nCurLoc = *(const int *)param;

	CORO_BEGIN_CODE(_ctx);

	// Closes idle
	GLOBALS._bSkipSfxNoLoop = true;

	CORO_INVOKE_2(mpalEndIdlePoll, nCurLoc, NULL);

	GLOBALS._bIdleExited = true;
	GLOBALS._bSkipSfxNoLoop = false;

	CORO_END_CODE;
}

RMGfxEngine::RMGfxEngine() {
	// Create big buffer where the frame will be rendered
	_bigBuf.create(RM_BBX, RM_BBY, 16);
	_bigBuf.offsetY(RM_SKIPY);
	_bigBuf.setTrackDirtyRects(true);

	_nCurLoc = 0;
	_curAction = TA_GOTO;
	_curActionObj = 0;
	_nWipeType = 0;
	_hWipeEvent = 0;
	_nWipeStep = 0;
	_bMustEnterMenu = false;
	_bWiping = false;
	_bGUIOption = false;
	_bGUIInterface = false;
	_bGUIInventory = false;
	_bAlwaysDrawMouse = false;
	_bOption = false;
	_bLocationLoaded = false;
	_bInput = false;
}

RMGfxEngine::~RMGfxEngine() {
	// Close the buffer
	_bigBuf.destroy();
}

void RMGfxEngine::openOptionScreen(CORO_PARAM, int type) {
	CORO_BEGIN_CONTEXT;
	bool bRes;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->bRes = false;

	if (type == 0)
		CORO_INVOKE_2(_opt.init, _bigBuf, _ctx->bRes);
	else if (type == 1)
		CORO_INVOKE_3(_opt.initLoadMenuOnly, _bigBuf, true, _ctx->bRes);
	else if (type == 2)
		CORO_INVOKE_2(_opt.initNoLoadSave, _bigBuf, _ctx->bRes);
	else if (type == 3)
		CORO_INVOKE_3(_opt.initLoadMenuOnly, _bigBuf, false, _ctx->bRes);
	else if (type == 4)
		CORO_INVOKE_3(_opt.initSaveMenuOnly, _bigBuf, false, _ctx->bRes);

	if (_ctx->bRes) {
		g_vm->pauseSound(true);

		disableInput();
		_inv.endCombine();
		_curActionObj = 0;
		_curAction = TA_GOTO;
		_point.setAction(_curAction);
		_point.setSpecialPointer(RMPointer::PTR_NONE);
		_point.setCustomPointer(NULL);
		enableMouse();
		g_vm->grabThumbnail();

		// Exists the IDLE to avoid premature death in loading
		_bMustEnterMenu = true;
		if (type == 1 || type == 2) {
			GLOBALS._bIdleExited = true;
		} else {
			CORO_INVOKE_0(_tony.stopNoAction);

			GLOBALS._bIdleExited = false;

			CoroScheduler.createProcess(exitAllIdles, &_nCurLoc, sizeof(int));
		}
	}

	CORO_END_CODE;
}

void RMGfxEngine::doFrame(CORO_PARAM, bool bDrawLocation) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Poll of input devices
	_input.poll();

	if (_bMustEnterMenu && GLOBALS._bIdleExited) {
		_bOption = true;
		_bMustEnterMenu = false;
		GLOBALS._bIdleExited = false;
	}

	if (_bOption) {
		CORO_INVOKE_1(_opt.doFrame, &_input);
		_bOption = !_opt.isClosing();
		if (!_bOption) {
			disableMouse();
			enableInput();
			mpalStartIdlePoll(_nCurLoc);
			g_vm->pauseSound(false);
		}
	}

	if (bDrawLocation && _bLocationLoaded) {
		// Location and objects
		_loc.doFrame(&_bigBuf);

		// Check the mouse input
		if (_bInput && !_tony.inAction()) {
			// If we are on the inventory, it is it who controls all input
			if (_inv.haveFocus(_input.mousePos()) && !_inter.active()) {
				// Left Click
				// **********
				if (_input.mouseLeftClicked()/* && m_itemName.IsItemSelected()*/) {
					// Left click activates the combine, if we are on an object
					if (_inv.leftClick(_input.mousePos(), _curActionObj)) {
						_curAction = TA_COMBINE;
						_point.setAction(_curAction);
					}
				} else

					// Right Click
					// ***********
					if (_input.mouseRightClicked()) {
						if (_itemName.isItemSelected()) {
							_curActionObj = 0;
							_inv.rightClick(_input.mousePos());
						} else
							_inv.rightClick(_input.mousePos());
					} else

						// Right Release
						// *************
						if (_input.mouseRightReleased()) {
							if (_inv.rightRelease(_input.mousePos(), _curAction)) {
								CORO_INVOKE_3(_tony.moveAndDoAction, _itemName.getHotspot(), _itemName.getSelectedItem(), _curAction);

								_curAction = TA_GOTO;
								_point.setAction(_curAction);
							}
						}
			} else {
				// Options Menu
				// ************
				if (_bGUIOption) {
					if (!_tony.inAction() && _bInput) {
						if ((_input.mouseLeftClicked() && _input.mousePos()._x < 3 && _input.mousePos()._y < 3)) {
							CORO_INVOKE_1(openOptionScreen, 0);
							goto SKIPCLICKSINISTRO;
						} else if (_input.getAsyncKeyState(Common::KEYCODE_ESCAPE))
							CORO_INVOKE_1(openOptionScreen, 0);
						else if (!g_vm->getIsDemo()) {
							if (_input.getAsyncKeyState(Common::KEYCODE_F3) || _input.getAsyncKeyState(Common::KEYCODE_F5))
								// Save game screen
								CORO_INVOKE_1(openOptionScreen, 4);
							else if (_input.getAsyncKeyState(Common::KEYCODE_F2) || _input.getAsyncKeyState(Common::KEYCODE_F7))
								// Load game screen
								CORO_INVOKE_1(openOptionScreen, 3);
						}
					}
				}

				// Left Click
				// **************
				if (_input.mouseLeftClicked() && !_inter.active()) {

					if (_curAction != TA_COMBINE)
						CORO_INVOKE_3(_tony.moveAndDoAction, _itemName.getHotspot(), _itemName.getSelectedItem(), _point.curAction());
					else if (_itemName.getSelectedItem() != NULL)
						CORO_INVOKE_4(_tony.moveAndDoAction, _itemName.getHotspot(), _itemName.getSelectedItem(), TA_COMBINE, _curActionObj);

					if (_curAction == TA_COMBINE) {
						_inv.endCombine();
						_point.setSpecialPointer(RMPointer::PTR_NONE);
					}

					_curAction = TA_GOTO;
					_point.setAction(_curAction);
				}

SKIPCLICKSINISTRO:
				// Right Click
				// ************
				if (_curAction == TA_COMBINE) {
					// During a combine, it cancels it
					if (_input.mouseRightClicked()) {
						_inv.endCombine();
						_curActionObj = 0;
						_curAction = TA_GOTO;
						_point.setAction(_curAction);
						_point.setSpecialPointer(RMPointer::PTR_NONE);
					}
				} else if (_input.mouseRightClicked() && _itemName.isItemSelected() && _point.getSpecialPointer() == RMPointer::PTR_NONE) {
					if (_bGUIInterface) {
						// Before opening the interface, replaces GOTO
						_curAction = TA_GOTO;
						_curActionObj = 0;
						_point.setAction(_curAction);
						_inter.clicked(_input.mousePos());
					}
				}

				// Right Release
				// *************
				if (_input.mouseRightReleased()) {
					if (_bGUIInterface) {
						if (_inter.released(_input.mousePos(), _curAction)) {
							_point.setAction(_curAction);
							CORO_INVOKE_3(_tony.moveAndDoAction, _itemName.getHotspot(), _itemName.getSelectedItem(), _curAction);

							_curAction = TA_GOTO;
							_point.setAction(_curAction);
						}
					}
				}
			}

			// Update the name under the mouse pointer
			_itemName.setMouseCoord(_input.mousePos());
			if (!_inter.active() && !_inv.miniActive())
				CORO_INVOKE_4(_itemName.doFrame, _bigBuf, _loc, _point, _inv);
		}

		// Interface & Inventory
		_inter.doFrame(_bigBuf, _input.mousePos());
		_inv.doFrame(_bigBuf, _point, _input.mousePos(), (!_tony.inAction() && !_inter.active() && _bGUIInventory));
	}

	// Animate Tony
	CORO_INVOKE_2(_tony.doFrame, &_bigBuf, _nCurLoc);

	// Update screen scrolling to keep Tony in focus
	if (_tony.mustUpdateScrolling() && _bLocationLoaded) {
		RMPoint showThis = _tony.position();
		showThis._y -= 60;
		_loc.updateScrolling(showThis);
	}

	if (_bLocationLoaded)
		_tony.setScrollPosition(_loc.scrollPosition());

	if ((!_tony.inAction() && _bInput) || _bAlwaysDrawMouse) {
		_point.showCursor();
	} else {
		_point.hideCursor();
	}
	_point.doFrame();

	// **********************
	// Draw the list in the OT
	// **********************
	CORO_INVOKE_0(_bigBuf.drawOT);

#define FSTEP (480/32)

	// Wipe
	if (_bWiping) {
		switch (_nWipeType) {
		case 1:
			if (!(_rcWipeEllipse.bottom - _rcWipeEllipse.top >= FSTEP * 2)) {
				CoroScheduler.setEvent(_hWipeEvent);
				_nWipeType = 3;
				break;
			}

			_rcWipeEllipse.top += FSTEP;
			_rcWipeEllipse.left += FSTEP;
			_rcWipeEllipse.right -= FSTEP;
			_rcWipeEllipse.bottom -= FSTEP;
			break;

		case 2:
			if (!(_rcWipeEllipse.bottom - _rcWipeEllipse.top < 480 - FSTEP)) {
				CoroScheduler.setEvent(_hWipeEvent);
				_nWipeType = 3;
				break;
			}

			_rcWipeEllipse.top -= FSTEP;
			_rcWipeEllipse.left -= FSTEP;
			_rcWipeEllipse.right += FSTEP;
			_rcWipeEllipse.bottom += FSTEP;
			break;
		}
	}

	CORO_END_CODE;
}

void RMGfxEngine::initCustomDll() {
	setupGlobalVars(&_tony, &_point, &g_vm->_theBoxes, &_loc, &_inv, &_input);
}

void RMGfxEngine::itemIrq(uint32 dwItem, int nPattern, int nStatus) {
	RMItem *item;
	assert(GLOBALS._gfxEngine);

	if (GLOBALS._gfxEngine->_bLocationLoaded) {
		item = GLOBALS._gfxEngine->_loc.getItemFromCode(dwItem);
		if (item != NULL) {
			if (nPattern != -1) {
				item->setPattern(nPattern, true);
			}
			if (nStatus != -1)
				item->setStatus(nStatus);
		}
	}
}

void RMGfxEngine::initForNewLocation(int nLoc, RMPoint ptTonyStart, RMPoint start) {
	if (start._x == -1 || start._y == -1) {
		start._x = ptTonyStart._x - RM_SX / 2;
		start._y = ptTonyStart._y - RM_SY / 2;
	}

	_loc.setScrollPosition(start);

	if (ptTonyStart._x == 0 && ptTonyStart._y == 0) {
	} else {
		_tony.setPosition(ptTonyStart, nLoc);
		_tony.setScrollPosition(start);
	}

	_curAction = TA_GOTO;
	_point.setCustomPointer(NULL);
	_point.setSpecialPointer(RMPointer::PTR_NONE);
	_point.setAction(_curAction);
	_inter.reset();
	_inv.reset();

	mpalStartIdlePoll(_nCurLoc);
}

uint32 RMGfxEngine::loadLocation(int nLoc, RMPoint ptTonyStart, RMPoint start) {
	_nCurLoc = nLoc;

	bool bLoaded = false;
	for (int i = 0; i < 5; i++) {
		// Try the loading of the location
		RMRes res(_nCurLoc);
		if (!res.isValid())
			continue;

		Common::SeekableReadStream *ds = res.getReadStream();
		_loc.load(*ds);
		delete ds;

		initForNewLocation(nLoc, ptTonyStart, start);
		bLoaded = true;
		break;
	}

	if (!bLoaded)
		error("Location was not loaded");

	if (_bOption)
		_opt.reInit(_bigBuf);

	_bLocationLoaded = true;

	// On entering the location
	return CORO_INVALID_PID_VALUE; //mpalQueryDoAction(0, m_nCurLoc, 0);
}

void RMGfxEngine::unloadLocation(CORO_PARAM, bool bDoOnExit, uint32 *result) {
	CORO_BEGIN_CONTEXT;
	uint32 h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Release the location
	CORO_INVOKE_2(mpalEndIdlePoll, _nCurLoc, NULL);

	// On Exit?
	if (bDoOnExit) {
		_ctx->h = mpalQueryDoAction(1, _nCurLoc, 0);
		if (_ctx->h != CORO_INVALID_PID_VALUE)
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);
	}

	_bLocationLoaded = false;

	_bigBuf.clearOT();
	_loc.unload();

	if (result != NULL)
		*result = CORO_INVALID_PID_VALUE;

	CORO_END_CODE;
}

void RMGfxEngine::init() {
	// Screen loading
	RMResRaw *raw;
	RMGfxSourceBuffer16 *load = NULL;
	INIT_GFX16_FROMRAW(20038, load);
	_bigBuf.addPrim(new RMGfxPrimitive(load));
	_bigBuf.drawOT(Common::nullContext);
	_bigBuf.clearOT();
	delete load;

	// Display 'Loading' screen
	_bigBuf.addDirtyRect(Common::Rect(0, 0, RM_SX, RM_SY));
	g_vm->_window.getNewFrame(*this, NULL);
	g_vm->_window.repaint();

	// Activate GUI
	_bGUIOption = true;
	_bGUIInterface = true;
	_bGUIInventory = true;

	GLOBALS._bSkipSfxNoLoop = false;
	_bMustEnterMenu = false;
	GLOBALS._bIdleExited = false;
	_bOption = false;
	_bWiping = false;
	_hWipeEvent = CoroScheduler.createEvent(false, false);

	// Initialize the IRQ function for items for MPAL
	GLOBALS._gfxEngine = this;
	mpalInstallItemIrq(itemIrq);

	// Initialize the mouse pointer
	_point.init();

	// Initialize Tony
	_tony.init();
	_tony.linkToBoxes(&g_vm->_theBoxes);

	// Initialize the inventory and the interface
	_inv.init();
	_inter.init();

	// Download the location and set priorities   @@@@@
	_bLocationLoaded = false;

	enableInput();

	// Starting the game
	_tony.executeAction(20, 1, 0);
}

void RMGfxEngine::close() {
	_bigBuf.clearOT();

	_inter.close();
	_inv.close();
	_tony.close();
	_point.close();
}

void RMGfxEngine::enableInput() {
	_bInput = true;
}

void RMGfxEngine::disableInput() {
	_bInput = false;
	_inter.reset();
}

void RMGfxEngine::enableMouse() {
	_bAlwaysDrawMouse = true;
}

void RMGfxEngine::disableMouse() {
	_bAlwaysDrawMouse = false;
}

#define TONY_SAVEGAME_VERSION 8

void RMGfxEngine::saveState(const Common::String &fn, byte *curThumb, const Common::String &name) {
	Common::OutSaveFile *f;
	byte *state;
	char buf[4];
	RMPoint tp = _tony.position();

	// Saving: MPAL variables, current location, and Tony inventory position

	// For now, we only save the MPAL state
	uint size = mpalGetSaveStateSize();
	state = new byte[size];
	mpalSaveState(state);

	uint thumbsize = 160 * 120 * 2;

	buf[0] = 'R';
	buf[1] = 'M';
	buf[2] = 'S';
	buf[3] = TONY_SAVEGAME_VERSION;

	f = g_system->getSavefileManager()->openForSaving(fn);
	if (f == NULL)
		return;

	f->write(buf, 4);
	f->writeUint32LE(thumbsize);
	f->write(curThumb, thumbsize);

	// Difficulty level
	int i = mpalQueryGlobalVar("VERSIONEFACILE");
	f->writeByte(i);

	i = strlen(name.c_str());
	f->writeByte(i);
	f->write(name.c_str(), i);
	f->writeUint32LE(_nCurLoc);
	f->writeUint32LE(tp._x);
	f->writeUint32LE(tp._y);

	f->writeUint32LE(size);
	f->write(state, size);
	delete[] state;

	// Inventory
	size = _inv.getSaveStateSize();
	state = new byte[size];
	_inv.saveState(state);
	f->writeUint32LE(size);
	f->write(state, size);
	delete[] state;

	// boxes
	size = g_vm->_theBoxes.getSaveStateSize();
	state = new byte[size];
	g_vm->_theBoxes.saveState(state);
	f->writeUint32LE(size);
	f->write(state, size);
	delete[] state;

	// New Ver5
	// Saves the state of the shepherdess and show yourself
	bool bStat = _tony.getShepherdess();
	f->writeByte(bStat);
	bStat = _inter.getPerorate();
	f->writeByte(bStat);

	// Save the chars
	charsSaveAll(f);

	// Save the options
	f->writeByte(GLOBALS._bCfgInvLocked);
	f->writeByte(GLOBALS._bCfgInvNoScroll);
	f->writeByte(GLOBALS._bCfgTimerizedText);
	f->writeByte(GLOBALS._bCfgInvUp);
	f->writeByte(GLOBALS._bCfgAnni30);
	f->writeByte(GLOBALS._bCfgAntiAlias);
	f->writeByte(GLOBALS._bShowSubtitles);
	f->writeByte(GLOBALS._bCfgTransparence);
	f->writeByte(GLOBALS._bCfgInterTips);
	f->writeByte(GLOBALS._bCfgDubbing);
	f->writeByte(GLOBALS._bCfgMusic);
	f->writeByte(GLOBALS._bCfgSFX);
	f->writeByte(GLOBALS._nCfgTonySpeed);
	f->writeByte(GLOBALS._nCfgTextSpeed);
	f->writeByte(GLOBALS._nCfgDubbingVolume);
	f->writeByte(GLOBALS._nCfgMusicVolume);
	f->writeByte(GLOBALS._nCfgSFXVolume);

	// Save the hotspots
	saveChangedHotspot(f);

	// Save the music
	saveMusic(f);

	f->finalize();
	delete f;
}

void RMGfxEngine::loadState(CORO_PARAM, const Common::String &fn) {
	// PROBLEM: You should change the location in a separate process to do the OnEnter
	CORO_BEGIN_CONTEXT;
	Common::InSaveFile *f;
	byte *state, *statecmp;
	uint32 size, sizecmp;
	char buf[4];
	RMPoint tp;
	int loc;
	int ver;
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->f = g_system->getSavefileManager()->openForLoading(fn);
	if (_ctx->f == NULL)
		return;
	_ctx->f->read(_ctx->buf, 4);

	if (_ctx->buf[0] != 'R' || _ctx->buf[1] != 'M' || _ctx->buf[2] != 'S') {
		delete _ctx->f;
		return;
	}

	_ctx->ver = _ctx->buf[3];

	if (_ctx->ver == 0 || _ctx->ver > TONY_SAVEGAME_VERSION) {
		delete _ctx->f;
		return;
	}

	if (_ctx->ver >= 0x3) {
		// There is a thumbnail. If the version is between 5 and 7, it's compressed
		if ((_ctx->ver >= 0x5) && (_ctx->ver <= 0x7)) {
			_ctx->i = 0;
			_ctx->i = _ctx->f->readUint32LE();
			_ctx->f->seek(_ctx->i);
		} else {
			if (_ctx->ver >= 8)
				// Skip thumbnail size
				_ctx->f->skip(4);

			_ctx->f->seek(160 * 120 * 2, SEEK_CUR);
		}
	}

	if (_ctx->ver >= 0x5) {
		// Skip the difficulty level
		_ctx->f->seek(1, SEEK_CUR);
	}

	if (_ctx->ver >= 0x4) { // Skip the savegame name, which serves no purpose
		_ctx->i = _ctx->f->readByte();
		_ctx->f->seek(_ctx->i, SEEK_CUR);
	}

	_ctx->loc = _ctx->f->readUint32LE();
	_ctx->tp._x = _ctx->f->readUint32LE();
	_ctx->tp._y = _ctx->f->readUint32LE();
	_ctx->size = _ctx->f->readUint32LE();

	if ((_ctx->ver >= 0x5) && (_ctx->ver <= 7)) {
		// MPAL was packed!
		_ctx->sizecmp = _ctx->f->readUint32LE();
		_ctx->state = new byte[_ctx->size];
		_ctx->statecmp = new byte[_ctx->sizecmp];
		_ctx->f->read(_ctx->statecmp, _ctx->sizecmp);
		lzo1x_decompress(_ctx->statecmp, _ctx->sizecmp, _ctx->state, &_ctx->size);
		delete[] _ctx->statecmp;
	} else {
		// Read uncompressed MPAL data
		_ctx->state = new byte[_ctx->size];
		_ctx->f->read(_ctx->state, _ctx->size);
	}

	mpalLoadState(_ctx->state);
	delete[] _ctx->state;

	// Inventory
	_ctx->size = _ctx->f->readUint32LE();
	_ctx->state = new byte[_ctx->size];
	_ctx->f->read(_ctx->state, _ctx->size);
	_inv.loadState(_ctx->state);
	delete[] _ctx->state;

	if (_ctx->ver >= 0x2) {   // Version 2: box please
		_ctx->size = _ctx->f->readUint32LE();
		_ctx->state = new byte[_ctx->size];
		_ctx->f->read(_ctx->state, _ctx->size);
		g_vm->_theBoxes.loadState(_ctx->state);
		delete[] _ctx->state;
	}

	if (_ctx->ver >= 5) {
		// Version 5
		bool bStat = false;

		bStat = _ctx->f->readByte();
		_tony.setShepherdess(bStat);
		bStat = _ctx->f->readByte();
		_inter.setPerorate(bStat);

		charsLoadAll(_ctx->f);
	}

	if (_ctx->ver >= 6) {
		// Load options
		GLOBALS._bCfgInvLocked = _ctx->f->readByte();
		GLOBALS._bCfgInvNoScroll = _ctx->f->readByte();
		GLOBALS._bCfgTimerizedText = _ctx->f->readByte();
		GLOBALS._bCfgInvUp = _ctx->f->readByte();
		GLOBALS._bCfgAnni30 = _ctx->f->readByte();
		GLOBALS._bCfgAntiAlias = _ctx->f->readByte();
		GLOBALS._bShowSubtitles = _ctx->f->readByte();
		GLOBALS._bCfgTransparence = _ctx->f->readByte();
		GLOBALS._bCfgInterTips = _ctx->f->readByte();
		GLOBALS._bCfgDubbing = _ctx->f->readByte();
		GLOBALS._bCfgMusic = _ctx->f->readByte();
		GLOBALS._bCfgSFX = _ctx->f->readByte();
		GLOBALS._nCfgTonySpeed = _ctx->f->readByte();
		GLOBALS._nCfgTextSpeed = _ctx->f->readByte();
		GLOBALS._nCfgDubbingVolume = _ctx->f->readByte();
		GLOBALS._nCfgMusicVolume = _ctx->f->readByte();
		GLOBALS._nCfgSFXVolume = _ctx->f->readByte();

		// Load hotspots
		loadChangedHotspot(_ctx->f);
	}

	if (_ctx->ver >= 7) {
		loadMusic(_ctx->f);
	}

	delete _ctx->f;

	CORO_INVOKE_2(unloadLocation, false, NULL);
	loadLocation(_ctx->loc, _ctx->tp, RMPoint(-1, -1));
	_tony.setPattern(RMTony::PAT_STANDRIGHT);

	// On older versions, need to an enter action
	if (_ctx->ver < 5)
		mpalQueryDoAction(0, _ctx->loc, 0);
	else {
		// In the new ones, we just reset the mcode
		mCharResetCodes();
	}

	if (_ctx->ver >= 6)
		reapplyChangedHotspot();

	CORO_INVOKE_0(restoreMusic);

	_bGUIInterface = true;
	_bGUIInventory = true;
	_bGUIOption = true;

	CORO_END_CODE;
}

void RMGfxEngine::pauseSound(bool bPause) {
	if (_bLocationLoaded)
		_loc.pauseSound(bPause);
}

void RMGfxEngine::initWipe(int type) {
	_bWiping = true;
	_nWipeType = type;
	_nWipeStep = 0;

	if (_nWipeType == 1)
		_rcWipeEllipse = Common::Rect(80, 0, 640 - 80, 480);
	else if (_nWipeType == 2)
		_rcWipeEllipse = Common::Rect(320 - FSTEP, 240 - FSTEP, 320 + FSTEP, 240 + FSTEP);
}

void RMGfxEngine::closeWipe() {
	_bWiping = false;
}

void RMGfxEngine::waitWipeEnd(CORO_PARAM) {
	CoroScheduler.waitForSingleObject(coroParam, _hWipeEvent, CORO_INFINITE);
}

bool RMGfxEngine::canLoadSave() {
	return _bInput && !_tony.inAction() && !g_vm->getIsDemo();
}

RMGfxEngine::operator RMGfxTargetBuffer &() {
	return _bigBuf;
}

RMInput &RMGfxEngine::getInput() {
	return _input;
}

RMPointer &RMGfxEngine::getPointer() {
	return _point;
}

/**
 * Link to graphic task
 */
void RMGfxEngine::linkGraphicTask(RMGfxTask *task) {
	_bigBuf.addPrim(new RMGfxPrimitive(task));
}

void RMGfxEngine::setPerorate(bool bpal) {
	_inter.setPerorate(bpal);
}

} // End of namespace Tony
