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

#include "common/file.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"
#include "tony/mpal/lzo.h"
#include "tony/mpal/memory.h"
#include "tony/mpal/mpal.h"
#include "tony/mpal/mpalutils.h"
#include "tony/game.h"
#include "tony/gfxengine.h"
#include "tony/tony.h"

namespace Tony {

using namespace MPAL;

//  Global functions
void mainEnableGUI() {
	g_vm->getEngine()->_bGUIInterface = true;
	g_vm->getEngine()->_bGUIInventory = true;
	g_vm->getEngine()->_bGUIOption = true;
}

void mainDisableGUI() {
	g_vm->getEngine()->_bGUIInterface = false;
	g_vm->getEngine()->_bGUIInventory = false;
	g_vm->getEngine()->_bGUIOption = false;
}

/****************************************************************************\
*       RMOptionButton Methods
\****************************************************************************/

RMOptionButton::RMOptionButton(uint32 dwRes, RMPoint pt, bool bDoubleState) {
	RMResRaw raw(dwRes);
	assert(raw.isValid());
	_buf = new RMGfxSourceBuffer16(false);
	_buf->init(raw, raw.width(), raw.height());

	_rect.setRect(pt._x, pt._y, pt._x + raw.width() - 1, pt._y + raw.height() - 1);
	_bActive = false;
	_bHasGfx = true;
	_bDoubleState = bDoubleState;
}

RMOptionButton::RMOptionButton(const RMRect &pt) {
	_rect = pt;
	_bActive = false;
	_bHasGfx = false;
	_bDoubleState = false;
	_buf = NULL;
}

RMOptionButton::~RMOptionButton() {
	if (_bHasGfx)
		delete _buf;
}

bool RMOptionButton::doFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick) {
	if (!_bDoubleState) {
		if (_rect.ptInRect(mousePos)) {
			if (!_bActive) {
				_bActive = true;
				return true;
			}
		} else {
			if (_bActive) {
				_bActive = false;
				return true;
			}
		}
	} else {
		if (bLeftClick && _rect.ptInRect(mousePos)) {
			_bActive = !_bActive;
			return true;
		}
	}

	return false;
}

void RMOptionButton::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!_bActive)
		return;

	if (_bHasGfx)
		CORO_INVOKE_2(_buf->draw, bigBuf, prim);

	CORO_END_CODE;
}

void RMOptionButton::addToList(RMGfxTargetBuffer &bigBuf) {
	if (_bHasGfx)
		bigBuf.addPrim(new RMGfxPrimitive(this, _rect));
}

bool RMOptionButton::isActive() {
	return _bActive;
}

void RMOptionButton::setActiveState(bool bState) {
	_bActive = bState;
}

/****************************************************************************\
*       RMOptionSlide Methods
\****************************************************************************/

RMOptionSlide::RMOptionSlide(const RMPoint &pt, int nRange, int nStartValue, int slideSize) {
	RMResRaw *raw;

	_pos = pt;
	_nSlideSize = slideSize;
	_nMax = nRange;
	_nStep = 100 / _nMax;
	_nValue = nStartValue;

	_sliderCenter = NULL;
	_sliderLeft = NULL;
	_sliderRight = NULL;
	_sliderSingle = NULL;

	// Sliders
	INIT_GFX16_FROMRAW(20029, _sliderCenter);
	INIT_GFX16_FROMRAW(20030, _sliderLeft);
	INIT_GFX16_FROMRAW(20031, _sliderRight);
	INIT_GFX16_FROMRAW(20032, _sliderSingle);

	// Buttons
	_pushLeft = new RMOptionButton(RMRect(pt._x - 23, pt._y, pt._x - 23 + 22, pt._y + 26));
	_pushRight = new RMOptionButton(RMRect(pt._x + _nSlideSize, pt._y, pt._x + _nSlideSize + 5 + 22, pt._y + 26));
}

RMOptionSlide::~RMOptionSlide() {
	delete _sliderCenter;
	_sliderCenter = NULL;
	delete _sliderLeft;
	_sliderLeft = NULL;
	delete _sliderRight;
	_sliderRight = NULL;
	delete _sliderSingle;
	_sliderSingle = NULL;

	delete _pushLeft;
	_pushLeft = NULL;
	delete _pushRight;
	_pushRight = NULL;
}

bool RMOptionSlide::doFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick) {
	bool bRefresh = false;

	// Do the button DoFrame's
	_pushLeft->doFrame(mousePos, bLeftClick, bRightClick);
	_pushRight->doFrame(mousePos, bLeftClick, bRightClick);

	if (_pushLeft->isActive()) {
		if (bLeftClick) {
			bRefresh = true;
			_nValue--;
		} else if (bRightClick) {
			bRefresh = true;
			_nValue -= 3;
		}
		if (_nValue < 1)
			_nValue = 1;
	} else if (_pushRight->isActive()) {
		bRefresh = true;

		if (bLeftClick) {
			bRefresh = true;
			_nValue++;
		} else if (bRightClick) {
			bRefresh = true;
			_nValue += 3;
		}
		if (_nValue > _nMax)
			_nValue = _nMax;
	}

	return bRefresh;
}

void RMOptionSlide::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	int i;
	int val;
	RMPoint pos;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->pos = _pos;
	_ctx->pos._x += 4;
	_ctx->pos._y += 4;

	_ctx->val = _nValue * _nStep;
	if (_ctx->val < 1)
		_ctx->val = 1;
	else if (_ctx->val > 100)
		_ctx->val = 100;

	if (_ctx->val == 1) {
		prim->setDst(_ctx->pos);
		CORO_INVOKE_2(_sliderSingle->draw, bigBuf, prim);
	} else {
		prim->setDst(_ctx->pos);
		CORO_INVOKE_2(_sliderLeft->draw, bigBuf, prim);
		_ctx->pos._x += 3;

		for (_ctx->i = 1; _ctx->i < _ctx->val - 1; _ctx->i++) {
			prim->setDst(_ctx->pos);
			CORO_INVOKE_2(_sliderCenter->draw, bigBuf, prim);
			_ctx->pos._x += 3;
		}

		prim->setDst(_ctx->pos);
		CORO_INVOKE_2(_sliderRight->draw, bigBuf, prim);
		_ctx->pos._x += 3;
	}

	CORO_END_CODE;
}

void RMOptionSlide::addToList(RMGfxTargetBuffer &bigBuf) {
	bigBuf.addPrim(new RMGfxPrimitive(this));
}

int RMOptionSlide::getValue() {
	return _nValue;
}

/****************************************************************************\
*       RMOptionScreen Methods
\****************************************************************************/

RMOptionScreen::RMOptionScreen() {
	_nState = MENUNONE;
	_menu = NULL;
	_hideLoadSave = NULL;
	_quitConfirm = NULL;
	_bQuitConfirm = false;

	create(RM_SX, RM_SY);

	_buttonExit = NULL;
	_buttonLoad = NULL;
	_buttonSave = NULL;
	_buttonGameMenu = NULL;
	_buttonGfxMenu = NULL;
	_buttonSoundMenu = NULL;
	_buttonSave_ArrowLeft = NULL;
	_buttonSave_ArrowRight = NULL;
	_bEditSaveName = false;

	for (int i = 0; i < 6; i++) {
		_curThumb[i] = NULL;
		_buttonSave_States[i] = NULL;
	}

	_statePos = 0;
	_buttonQuitYes = NULL;
	_buttonQuitNo = NULL;
	_buttonQuit = NULL;
	_saveEasy = NULL;
	_saveHard = NULL;
	_buttonGfx_Tips = NULL;
	_buttonSound_DubbingOn = NULL;
	_buttonSound_MusicOn = NULL;
	_buttonSound_SFXOn = NULL;
	_slideTonySpeed = NULL;
	_slideTextSpeed = NULL;
	_buttonGame_Lock = NULL;
	_buttonGfx_Anni30 = NULL;
	_sliderSound_Music = NULL;
	_buttonGame_TimerizedText = NULL;
	_buttonGfx_AntiAlias = NULL;
	_sliderSound_SFX = NULL;
	_buttonGame_Scrolling = NULL;
	_buttonGfx_Sottotitoli = NULL;
	_sliderSound_Dubbing = NULL;
	_buttonGame_InterUp = NULL;
	_buttonGfx_Trans = NULL;

	_fadeStep = 0;
	_fadeY = 0;
	_fadeTime = 0;
	_nEditPos = 0;
	_nLastState = MENUGAME;
}

RMOptionScreen::~RMOptionScreen() {
	closeState();
}

void RMOptionScreen::refreshAll(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	RMGfxSourceBuffer16 *thumb;
	RMText *title;
	RMText *num[6];
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	clearOT();

	addPrim(new RMGfxPrimitive(_menu));

	if (_bNoLoadSave)
		addPrim(new RMGfxPrimitive(_hideLoadSave, RMPoint(0, 401)));

	if (_bQuitConfirm) {
		addPrim(new RMGfxPrimitive(_quitConfirm, RMPoint(270, 200)));
		_buttonQuitYes->addToList(*this);
		_buttonQuitNo->addToList(*this);
	}

	_buttonExit->addToList(*this);

	if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND) {
		_buttonQuit->addToList(*this);
		_buttonLoad->addToList(*this);
		_buttonSave->addToList(*this);
	}

	if (_nState == MENUGAME) {
		_buttonGame_Lock->addToList(*this);
		_buttonGame_TimerizedText->addToList(*this);
		_buttonGame_Scrolling->addToList(*this);
		_buttonGame_InterUp->addToList(*this);
		_slideTextSpeed->addToList(*this);
		_slideTonySpeed->addToList(*this);
	} else if (_nState == MENUGFX) {
		_buttonGfx_Anni30->addToList(*this);
		_buttonGfx_AntiAlias->addToList(*this);
		_buttonGfx_Sottotitoli->addToList(*this);
		_buttonGfx_Trans->addToList(*this);
		_buttonGfx_Tips->addToList(*this);
	} else if (_nState == MENUSOUND) {
		_sliderSound_Dubbing->addToList(*this);
		_sliderSound_Music->addToList(*this);
		_sliderSound_SFX->addToList(*this);
		_buttonSound_DubbingOn->addToList(*this);
		_buttonSound_MusicOn->addToList(*this);
		_buttonSound_SFXOn->addToList(*this);
	}

	_ctx->thumb = NULL;
	_ctx->title = NULL;
	Common::fill(&_ctx->num[0], &_ctx->num[6], (RMText *)NULL);

	if (_nState == MENULOAD || _nState == MENUSAVE) {
		_ctx->title = new RMText;
		if (_nState == MENULOAD) {
			RMMessage msg(10);
			_ctx->title->writeText(msg[0], 1);
		} else {
			RMMessage msg(11);
			_ctx->title->writeText(msg[0], 1);
		}

		addPrim(new RMGfxPrimitive(_ctx->title, RMPoint(320, 10)));

		if (_curThumbDiff[0] == 0)
			addPrim(new RMGfxPrimitive(_saveHard, RMPoint(48, 57)));
		else if (_curThumbDiff[0] == 1)
			addPrim(new RMGfxPrimitive(_saveEasy, RMPoint(48, 57)));
		if (_curThumbDiff[1] == 0)
			addPrim(new RMGfxPrimitive(_saveHard, RMPoint(240, 57)));
		else if (_curThumbDiff[1] == 1)
			addPrim(new RMGfxPrimitive(_saveEasy, RMPoint(240, 57)));
		if (_curThumbDiff[2] == 0)
			addPrim(new RMGfxPrimitive(_saveHard, RMPoint(432, 57)));
		else if (_curThumbDiff[2] == 1)
			addPrim(new RMGfxPrimitive(_saveEasy, RMPoint(432, 57)));
		if (_curThumbDiff[3] == 0)
			addPrim(new RMGfxPrimitive(_saveHard, RMPoint(48, 239)));
		else if (_curThumbDiff[3] == 1)
			addPrim(new RMGfxPrimitive(_saveEasy, RMPoint(48, 239)));
		if (_curThumbDiff[4] == 0)
			addPrim(new RMGfxPrimitive(_saveHard, RMPoint(240, 239)));
		else if (_curThumbDiff[4] == 1)
			addPrim(new RMGfxPrimitive(_saveEasy, RMPoint(240, 239)));
		if (_curThumbDiff[5] == 0)
			addPrim(new RMGfxPrimitive(_saveHard, RMPoint(432, 239)));
		else if (_curThumbDiff[5] == 1)
			addPrim(new RMGfxPrimitive(_saveEasy, RMPoint(432, 239)));

		if (_curThumb[0] && !(_bEditSaveName && _nEditPos == 0))
			addPrim(new RMGfxPrimitive(_curThumb[0], RMPoint(48, 57)));
		if (_curThumb[1] && !(_bEditSaveName && _nEditPos == 1))
			addPrim(new RMGfxPrimitive(_curThumb[1], RMPoint(240, 57)));
		if (_curThumb[2] && !(_bEditSaveName && _nEditPos == 2))
			addPrim(new RMGfxPrimitive(_curThumb[2], RMPoint(432, 57)));
		if (_curThumb[3] && !(_bEditSaveName && _nEditPos == 3))
			addPrim(new RMGfxPrimitive(_curThumb[3], RMPoint(48, 239)));
		if (_curThumb[4] && !(_bEditSaveName && _nEditPos == 4))
			addPrim(new RMGfxPrimitive(_curThumb[4], RMPoint(240, 239)));
		if (_curThumb[5] && !(_bEditSaveName && _nEditPos == 5))
			addPrim(new RMGfxPrimitive(_curThumb[5], RMPoint(432, 239)));

		if (_bEditSaveName) {
			_ctx->thumb = new RMGfxSourceBuffer16;
			_ctx->thumb->init((byte *)g_vm->getThumbnail(), 640 / 4, 480 / 4);

			if (_nEditPos == 0)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(48, 57)));
			else if (_nEditPos == 1)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(240, 57)));
			else if (_nEditPos == 2)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(432, 57)));
			else if (_nEditPos == 3)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(48, 239)));
			else if (_nEditPos == 4)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(240, 239)));
			else if (_nEditPos == 5)
				addPrim(new RMGfxPrimitive(_ctx->thumb, RMPoint(432, 239)));
		}

		for (_ctx->i = 0; _ctx->i < 6; _ctx->i++) {
			Common::String s;

			if (_bEditSaveName && _nEditPos == _ctx->i)
				s = Common::String::format("%02d)%s*", _statePos + _ctx->i, _editName);
			else {
				if (_statePos == 0 && _ctx->i == 0)
					s = "Autosave";
				else
					s = Common::String::format("%02d)%s", _statePos + _ctx->i, _curThumbName[_ctx->i].c_str());
			}

			_ctx->num[_ctx->i] = new RMText;
			_ctx->num[_ctx->i]->setAlignType(RMText::HLEFT, RMText::VTOP);
			_ctx->num[_ctx->i]->writeText(s, 2);
		}

		addPrim(new RMGfxPrimitive(_ctx->num[0], RMPoint(55 - 3, 180 + 14)));
		addPrim(new RMGfxPrimitive(_ctx->num[1], RMPoint(247 - 3, 180 + 14)));
		addPrim(new RMGfxPrimitive(_ctx->num[2], RMPoint(439 - 3, 180 + 14)));
		addPrim(new RMGfxPrimitive(_ctx->num[3], RMPoint(55 - 3, 362 + 14)));
		addPrim(new RMGfxPrimitive(_ctx->num[4], RMPoint(247 - 3, 362 + 14)));
		addPrim(new RMGfxPrimitive(_ctx->num[5], RMPoint(439 - 3, 362 + 14)));

		_buttonSave_ArrowLeft->addToList(*this);
		_buttonSave_ArrowRight->addToList(*this);
	}

	CORO_INVOKE_0(drawOT);

	if (_nState == MENULOAD || _nState == MENUSAVE) {
		if (_ctx->thumb)
			delete _ctx->thumb;

		if (_ctx->title)
			delete _ctx->title;

		for (_ctx->i = 0; _ctx->i < 6; _ctx->i++) {
			if (_ctx->num[_ctx->i])
				delete _ctx->num[_ctx->i];
		}
	}

	CORO_END_CODE;
}

void RMOptionScreen::refreshThumbnails() {
	for (int i = 0; i < 6; i++) {
		if (_curThumb[i])
			delete _curThumb[i];

		_curThumb[i] = new RMGfxSourceBuffer16;
		_curThumb[i]->create(640 / 4, 480 / 4);
		if (!loadThumbnailFromSaveState(_statePos + i, *_curThumb[i], _curThumbName[i], _curThumbDiff[i])) {
			delete _curThumb[i];
			_curThumb[i] = NULL;
			_curThumbName[i].clear();
			_curThumbDiff[i] = 11;
		}
	}
}

void RMOptionScreen::initState(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	RMResRaw *raw;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND)
		_ctx->raw = new RMResRaw(20000 + _nState);
	else if (_nState == MENULOAD || _nState == MENUSAVE) {
		if (_bAlterGfx)
			_ctx->raw = new RMResRaw(20024);
		else
			_ctx->raw = new RMResRaw(20003);
	} else {
		error("Invalid state");
	}

	assert(_ctx->raw->isValid());
	assert(_menu == NULL);
	_menu = new RMGfxSourceBuffer16(false);
	_menu->init(*_ctx->raw, _ctx->raw->width(), _ctx->raw->height());
	delete _ctx->raw;

	if (_nState == MENULOAD || _nState == MENUSAVE) {
		if (_bAlterGfx) {
			assert(_buttonExit == NULL);
			_buttonExit = new RMOptionButton(20025, RMPoint(561, 406));
		} else {
			assert(_buttonExit == NULL);
			_buttonExit = new RMOptionButton(20012, RMPoint(560, 404));
		}

		INIT_GFX8_FROMRAW(_ctx->raw, 20036, _saveEasy);
		INIT_GFX8_FROMRAW(_ctx->raw, 20037, _saveHard);

		refreshThumbnails();

		assert(_buttonSave_States[0] == NULL);
		_buttonSave_States[0] = new RMOptionButton(RMRect(48, 57, 48 + 160, 57 + 120));
		assert(_buttonSave_States[1] == NULL);
		_buttonSave_States[1] = new RMOptionButton(RMRect(240, 57, 240 + 160, 57 + 120));
		assert(_buttonSave_States[2] == NULL);
		_buttonSave_States[2] = new RMOptionButton(RMRect(432, 57, 432 + 160, 57 + 120));
		assert(_buttonSave_States[3] == NULL);
		_buttonSave_States[3] = new RMOptionButton(RMRect(48, 239, 48 + 160, 239 + 120));
		assert(_buttonSave_States[4] == NULL);
		_buttonSave_States[4] = new RMOptionButton(RMRect(240, 239, 240 + 160, 239 + 120));
		assert(_buttonSave_States[5] == NULL);
		_buttonSave_States[5] = new RMOptionButton(RMRect(432, 239, 432 + 160, 239 + 120));

		if (_bAlterGfx) {
			assert(_buttonSave_ArrowLeft == NULL);
			_buttonSave_ArrowLeft = new RMOptionButton(20026, RMPoint(3, 196));
			assert(_buttonSave_ArrowRight == NULL);
			_buttonSave_ArrowRight = new RMOptionButton(20027, RMPoint(601, 197));
		} else {
			assert(_buttonSave_ArrowLeft == NULL);
			_buttonSave_ArrowLeft = new RMOptionButton(20013, RMPoint(0, 197));
			assert(_buttonSave_ArrowRight == NULL);
			_buttonSave_ArrowRight = new RMOptionButton(20014, RMPoint(601, 197));
		}
	} else if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND) {
		assert(_buttonExit == NULL);
		_buttonExit = new RMOptionButton(20005, RMPoint(560, 405));
		assert(_buttonQuit == NULL);
		_buttonQuit = new RMOptionButton(20020, RMPoint(7, 408));
		assert(_buttonLoad == NULL);
		_buttonLoad = new RMOptionButton(20006, RMPoint(231, 401));
		assert(_buttonSave == NULL);
		_buttonSave = new RMOptionButton(20007, RMPoint(325, 401));

		assert(_buttonGameMenu == NULL);
		_buttonGameMenu = new RMOptionButton(RMRect(24, 32, 118, 64));
		assert(_buttonGfxMenu == NULL);
		_buttonGfxMenu = new RMOptionButton(RMRect(118, 32, 212, 64));
		assert(_buttonSoundMenu == NULL);
		_buttonSoundMenu = new RMOptionButton(RMRect(212, 32, 306, 64));

		_ctx->raw = new RMResRaw(20021);
		assert(_ctx->raw->isValid());
		assert(_quitConfirm == NULL);
		_quitConfirm = new RMGfxSourceBuffer16(false);
		_quitConfirm->init(*_ctx->raw, _ctx->raw->width(), _ctx->raw->height());
		delete _ctx->raw;

		assert(_buttonQuitYes == NULL);
		_buttonQuitYes = new RMOptionButton(20022, RMPoint(281, 265));
		_buttonQuitYes->setPriority(30);
		assert(_buttonQuitNo == NULL);
		_buttonQuitNo = new RMOptionButton(20023, RMPoint(337, 264));
		_buttonQuitNo->setPriority(30);

		if (_bNoLoadSave) {
			_ctx->raw = new RMResRaw(20028);
			assert(_ctx->raw->isValid());
			assert(_hideLoadSave == NULL);
			_hideLoadSave = new RMGfxSourceBuffer16(false);
			_hideLoadSave->init(*_ctx->raw, _ctx->raw->width(), _ctx->raw->height());
			delete _ctx->raw;
		}

		// Menu GAME
		if (_nState == MENUGAME) {
			assert(_buttonGame_Lock == NULL);
			_buttonGame_Lock = new RMOptionButton(20008, RMPoint(176, 262), true);
			_buttonGame_Lock->setActiveState(GLOBALS._bCfgInvLocked);
			assert(_buttonGame_TimerizedText == NULL);
			_buttonGame_TimerizedText = new RMOptionButton(20009, RMPoint(463, 273), true);
			_buttonGame_TimerizedText->setActiveState(!GLOBALS._bCfgTimerizedText);
			assert(_buttonGame_Scrolling == NULL);
			_buttonGame_Scrolling = new RMOptionButton(20010, RMPoint(315, 263), true);
			_buttonGame_Scrolling->setActiveState(GLOBALS._bCfgInvNoScroll);
			assert(_buttonGame_InterUp == NULL);
			_buttonGame_InterUp = new RMOptionButton(20011, RMPoint(36, 258), true);
			_buttonGame_InterUp->setActiveState(GLOBALS._bCfgInvUp);

			assert(_slideTextSpeed == NULL);
			_slideTextSpeed = new RMOptionSlide(RMPoint(165, 122), 10, GLOBALS._nCfgTextSpeed);
			assert(_slideTonySpeed == NULL);
			_slideTonySpeed = new RMOptionSlide(RMPoint(165, 226), 5, GLOBALS._nCfgTonySpeed);
		}
		// Menu Graphics
		else if (_nState == MENUGFX) {
			assert(_buttonGfx_Anni30 == NULL);
			_buttonGfx_Anni30 = new RMOptionButton(20015, RMPoint(247, 178), true);
			_buttonGfx_Anni30->setActiveState(GLOBALS._bCfgAnni30);
			assert(_buttonGfx_AntiAlias == NULL);
			_buttonGfx_AntiAlias = new RMOptionButton(20016, RMPoint(430, 83), true);
			_buttonGfx_AntiAlias->setActiveState(!GLOBALS._bCfgAntiAlias);
			assert(_buttonGfx_Sottotitoli == NULL);
			_buttonGfx_Sottotitoli = new RMOptionButton(20017, RMPoint(98, 82), true);
			_buttonGfx_Sottotitoli->setActiveState(!GLOBALS._bShowSubtitles);
			assert(_buttonGfx_Tips == NULL);
			_buttonGfx_Tips = new RMOptionButton(20018, RMPoint(431, 246), true);
			_buttonGfx_Tips->setActiveState(GLOBALS._bCfgInterTips);
			assert(_buttonGfx_Trans == NULL);
			_buttonGfx_Trans = new RMOptionButton(20019, RMPoint(126, 271), true);
			_buttonGfx_Trans->setActiveState(!GLOBALS._bCfgTransparence);

		} else if (_nState == MENUSOUND) {
			assert(_sliderSound_Dubbing == NULL);
			_sliderSound_Dubbing = new RMOptionSlide(RMPoint(165, 122), 10, GLOBALS._nCfgDubbingVolume);
			assert(_sliderSound_Music == NULL);
			_sliderSound_Music = new RMOptionSlide(RMPoint(165, 226), 10, GLOBALS._nCfgMusicVolume);
			assert(_sliderSound_SFX == NULL);
			_sliderSound_SFX = new RMOptionSlide(RMPoint(165, 330), 10, GLOBALS._nCfgSFXVolume);

			assert(_buttonSound_DubbingOn == NULL);
			_buttonSound_DubbingOn = new RMOptionButton(20033, RMPoint(339, 75), true);
			_buttonSound_DubbingOn->setActiveState(GLOBALS._bCfgDubbing);
			assert(_buttonSound_MusicOn == NULL);
			_buttonSound_MusicOn = new RMOptionButton(20034, RMPoint(338, 179), true);
			_buttonSound_MusicOn->setActiveState(GLOBALS._bCfgMusic);
			assert(_buttonSound_SFXOn == NULL);
			_buttonSound_SFXOn = new RMOptionButton(20035, RMPoint(338, 283), true);
			_buttonSound_SFXOn->setActiveState(GLOBALS._bCfgSFX);
		}
	}

	CORO_INVOKE_0(refreshAll);

	CORO_END_CODE;
}

void RMOptionScreen::closeState() {
	delete _menu;
	_menu = NULL;

	delete _buttonExit;
	_buttonExit = NULL;

	if (_nState == MENULOAD || _nState == MENUSAVE) {
		for (int i = 0; i < 6; i++) {
			if (_curThumb[i] != NULL) {
				delete _curThumb[i];
				_curThumb[i] = NULL;
			}

			delete _buttonSave_States[i];
			_buttonSave_States[i] = NULL;
		}

		delete _buttonSave_ArrowLeft;
		_buttonSave_ArrowLeft = NULL;
		delete _buttonSave_ArrowRight;
		_buttonSave_ArrowRight = NULL;

		delete _saveEasy;
		_saveEasy = NULL;
		delete _saveHard;
		_saveHard = NULL;
	}

	if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND) {
		delete _buttonQuit;
		_buttonQuit = NULL;
		delete _buttonLoad;
		_buttonLoad = NULL;
		delete _buttonSave;
		_buttonSave = NULL;
		delete _buttonGameMenu;
		_buttonGameMenu = NULL;
		delete _buttonGfxMenu;
		_buttonGfxMenu = NULL;
		delete _buttonSoundMenu;
		_buttonSoundMenu = NULL;
		delete _quitConfirm;
		_quitConfirm = NULL;
		delete _buttonQuitYes;
		_buttonQuitYes = NULL;
		delete _buttonQuitNo;
		_buttonQuitNo = NULL;

		if (_bNoLoadSave) {
			delete _hideLoadSave;
			_hideLoadSave = NULL;
		}

		if (_nState == MENUGAME) {
			GLOBALS._bCfgInvLocked = _buttonGame_Lock->isActive();
			delete _buttonGame_Lock;
			_buttonGame_Lock = NULL;

			GLOBALS._bCfgTimerizedText = !_buttonGame_TimerizedText->isActive();
			delete _buttonGame_TimerizedText;
			_buttonGame_TimerizedText = NULL;

			GLOBALS._bCfgInvNoScroll = _buttonGame_Scrolling->isActive();
			delete _buttonGame_Scrolling;
			_buttonGame_Scrolling = NULL;

			GLOBALS._bCfgInvUp = _buttonGame_InterUp->isActive();
			delete _buttonGame_InterUp;
			_buttonGame_InterUp = NULL;

			GLOBALS._nCfgTextSpeed = _slideTextSpeed->getValue();
			delete _slideTextSpeed;
			_slideTextSpeed = NULL;

			GLOBALS._nCfgTonySpeed = _slideTonySpeed->getValue();
			delete _slideTonySpeed;
			_slideTonySpeed = NULL;
		} else if (_nState == MENUGFX) {
			GLOBALS._bCfgAnni30 = _buttonGfx_Anni30->isActive();
			delete _buttonGfx_Anni30;
			_buttonGfx_Anni30 = NULL;

			GLOBALS._bCfgAntiAlias = !_buttonGfx_AntiAlias->isActive();
			delete _buttonGfx_AntiAlias;
			_buttonGfx_AntiAlias = NULL;

			GLOBALS._bShowSubtitles = !_buttonGfx_Sottotitoli->isActive();
			delete _buttonGfx_Sottotitoli;
			_buttonGfx_Sottotitoli = NULL;

			GLOBALS._bCfgInterTips = _buttonGfx_Tips->isActive();
			delete _buttonGfx_Tips;
			_buttonGfx_Tips = NULL;

			GLOBALS._bCfgTransparence = !_buttonGfx_Trans->isActive();
			delete _buttonGfx_Trans;
			_buttonGfx_Trans = NULL;
		} else if (_nState == MENUSOUND) {
			GLOBALS._nCfgDubbingVolume = _sliderSound_Dubbing->getValue();
			delete _sliderSound_Dubbing;
			_sliderSound_Dubbing = NULL;

			GLOBALS._nCfgMusicVolume = _sliderSound_Music->getValue();
			delete _sliderSound_Music;
			_sliderSound_Music = NULL;

			GLOBALS._nCfgSFXVolume = _sliderSound_SFX->getValue();
			delete _sliderSound_SFX;
			_sliderSound_SFX = NULL;

			GLOBALS._bCfgDubbing = _buttonSound_DubbingOn->isActive();
			delete _buttonSound_DubbingOn;
			_buttonSound_DubbingOn = NULL;

			GLOBALS._bCfgMusic = _buttonSound_MusicOn->isActive();
			delete _buttonSound_MusicOn;
			_buttonSound_MusicOn = NULL;

			GLOBALS._bCfgSFX = _buttonSound_SFXOn->isActive();
			delete _buttonSound_SFXOn;
			_buttonSound_SFXOn = NULL;
		}

		// Save the new settings to ScummVM
		g_vm->saveSoundSettings();
	}

	_nState = MENUNONE;
}

void RMOptionScreen::reInit(RMGfxTargetBuffer &bigBuf) {
	bigBuf.addPrim(new RMGfxPrimitive(this));
}

void RMOptionScreen::init(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_fadeStep != 0) {
		result = false;
		return;
	}

	_fadeStep = 1;
	_fadeY = -20;
	_fadeTime = -1;
	_bExit = false;
	_bLoadMenuOnly = false;
	_bNoLoadSave = false;
	_bAlterGfx = false;

	bigBuf.addPrim(new RMGfxPrimitive(this));

	if (_nState == MENULOAD || _nState == MENUSAVE || _nState == MENUNONE)
		_nState = MENUGAME;

	CORO_INVOKE_0(initState);

	result = true;

	CORO_END_CODE;
}

void RMOptionScreen::initLoadMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_fadeStep != 0) {
		result = false;
		return;
	}

	_fadeStep = 1;
	_fadeY = -20;
	_fadeTime = -1;
	_bExit = false;
	_bLoadMenuOnly = true;
	_bNoLoadSave = false;
	_bAlterGfx = bAlternateGfx;

	bigBuf.addPrim(new RMGfxPrimitive(this));

	_nState = MENULOAD;
	CORO_INVOKE_0(initState);

	result = true;

	CORO_END_CODE;
}

void RMOptionScreen::initSaveMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_fadeStep != 0) {
		result = false;
		return;
	}

	_fadeStep = 1;
	_fadeY = -20;
	_fadeTime = -1;
	_bExit = false;
	_bLoadMenuOnly = true;
	_bNoLoadSave = false;
	_bAlterGfx = bAlternateGfx;

	bigBuf.addPrim(new RMGfxPrimitive(this));

	_nState = MENUSAVE;
	CORO_INVOKE_0(initState);

	result = true;

	CORO_END_CODE;
}

void RMOptionScreen::initNoLoadSave(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_fadeStep != 0) {
		result = false;
		return;
	}

	_fadeStep = 1;
	_fadeY = -20;
	_fadeTime = -1;
	_bExit = false;
	_bLoadMenuOnly = false;
	_bNoLoadSave = true;

	bigBuf.addPrim(new RMGfxPrimitive(this));

	_nState = MENUGAME;
	CORO_INVOKE_0(initState);

	result = true;

	CORO_END_CODE;
}

bool RMOptionScreen::close() {
	if (_fadeStep != 6)
		return false;

	// Start fade out
	_fadeStep++;
	_fadeTime = g_vm->getTime();
	return true;
}

bool RMOptionScreen::isClosing() {
	return _bExit;
}

int RMOptionScreen::priority() {
	// Just below the mouse
	return 190;
}

void RMOptionScreen::changeState(CORO_PARAM, OptionScreenState newState) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_nLastState = _nState;
	closeState();
	_nState = newState;
	CORO_INVOKE_0(initState);

	CORO_END_CODE;
}

void RMOptionScreen::doFrame(CORO_PARAM, RMInput *input) {
	CORO_BEGIN_CONTEXT;
	bool bLeftClick, bRightClick;
	RMPoint mousePos;
	bool bRefresh;
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// If it is not fully open, do nothing
	if (_fadeStep != 6)
		return;

	// Reads input
	_ctx->mousePos = input->mousePos();
	_ctx->bLeftClick = input->mouseLeftClicked();
	_ctx->bRightClick = input->mouseRightClicked();

	_ctx->bRefresh = false;

	if (_bQuitConfirm) {
		_ctx->bRefresh |= _buttonQuitYes->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
		_ctx->bRefresh |= _buttonQuitNo->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
	} else {
		_ctx->bRefresh |= _buttonExit->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		// Check if you have clicked on the output
		if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND) {
			// Buttons without graphics...
			_buttonGameMenu->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_buttonGfxMenu->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_buttonSoundMenu->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

			// Buttons with graphics
			if (!_bNoLoadSave) {
				if (!g_vm->getIsDemo()) {
					_ctx->bRefresh |= _buttonLoad->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
					_ctx->bRefresh |= _buttonSave->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
				}

				_ctx->bRefresh |= _buttonQuit->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			}
		}

		if (_nState == MENUGAME) {
			_ctx->bRefresh |= _buttonGame_Lock->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _buttonGame_TimerizedText->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _buttonGame_Scrolling->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _buttonGame_InterUp->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _slideTextSpeed->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _slideTonySpeed->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		} else if (_nState == MENUGFX) {
			_ctx->bRefresh |= _buttonGfx_Anni30->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _buttonGfx_AntiAlias->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _buttonGfx_Sottotitoli->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _buttonGfx_Tips->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _buttonGfx_Trans->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		} else if (_nState == MENUSOUND) {
			_ctx->bRefresh |= _sliderSound_Dubbing->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _sliderSound_Music->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _sliderSound_SFX->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _buttonSound_DubbingOn->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _buttonSound_MusicOn->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			_ctx->bRefresh |= _buttonSound_SFXOn->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

		} else if (_nState == MENULOAD || _nState == MENUSAVE) {
			for (_ctx->i = 0; _ctx->i < 6; _ctx->i++)
				_buttonSave_States[_ctx->i]->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);

			if (_statePos > 0)
				_ctx->bRefresh |= _buttonSave_ArrowLeft->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
			if (_statePos < 90)
				_ctx->bRefresh |= _buttonSave_ArrowRight->doFrame(_ctx->mousePos, _ctx->bLeftClick, _ctx->bRightClick);
		}
	}

#define KEYPRESS(c) (g_vm->getEngine()->getInput().getAsyncKeyState(c))
#define PROCESS_CHAR(cod, c) if (KEYPRESS(cod)) { \
		_editName[strlen(_editName) + 1] = '\0'; _editName[strlen(_editName)] = c; _ctx->bRefresh = true; }

	// State Buttons
	if (_bEditSaveName) {
		if (KEYPRESS(Common::KEYCODE_BACKSPACE)) {
			if (_editName[0] != '\0') {
				_editName[strlen(_editName) - 1] = '\0';
				_ctx->bRefresh = true;
			}
		}

		for (_ctx->i = 0; _ctx->i < 26 && strlen(_editName) < 12; _ctx->i++) {
			if (KEYPRESS(Common::KEYCODE_LSHIFT) ||
			    KEYPRESS(Common::KEYCODE_RSHIFT)) {
				PROCESS_CHAR((Common::KeyCode)((int)'a' + _ctx->i), _ctx->i + 'A');
			} else {
				PROCESS_CHAR((Common::KeyCode)((int)'a' + _ctx->i), _ctx->i + 'a');
			}
		}

		for (_ctx->i = 0; _ctx->i < 10 && strlen(_editName) < 12; _ctx->i++)
			PROCESS_CHAR((Common::KeyCode)((int)'0' + _ctx->i), _ctx->i + '0');

		if (strlen(_editName) < 12)
			PROCESS_CHAR(Common::KEYCODE_SPACE, ' ');

		if (strlen(_editName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP0, '0');
		if (strlen(_editName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP1, '1');
		if (strlen(_editName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP2, '2');
		if (strlen(_editName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP3, '3');
		if (strlen(_editName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP4, '4');
		if (strlen(_editName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP5, '5');
		if (strlen(_editName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP6, '6');
		if (strlen(_editName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP7, '7');
		if (strlen(_editName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP8, '8');
		if (strlen(_editName) < 12)
			PROCESS_CHAR(Common::KEYCODE_KP9, '9');

		// Cancel
		if (KEYPRESS(Common::KEYCODE_ESCAPE)) {
			_bEditSaveName = false;
			_ctx->bRefresh = true;
		}

		// OK
		if (KEYPRESS(Common::KEYCODE_RETURN)) {
			_bEditSaveName = false;
			g_vm->saveState(_statePos + _nEditPos, _editName);
			close();
		}

	} else if (_ctx->bLeftClick) {
		if (_nState == MENULOAD || _nState == MENUSAVE) {
			if (_buttonExit->isActive()) {
				if (_bLoadMenuOnly) {
					// If only the loading menu, close
					close();
				} else {
					CORO_INVOKE_1(changeState, _nLastState);
					_ctx->bRefresh = true;
				}
			} else if (_buttonSave_ArrowLeft->isActive()) {
				if (_statePos > 0) {
					_statePos -= 6;
					if (_statePos < 0)
						_statePos = 0;
					_buttonSave_ArrowLeft->setActiveState(false);
					_ctx->bRefresh = true;
					refreshThumbnails();
				}
			} else if (_buttonSave_ArrowRight->isActive()) {
				if (_statePos < 90) {
					_statePos += 6;
					if (_statePos > 90)
						_statePos = 90;
					_buttonSave_ArrowRight->setActiveState(false);
					_ctx->bRefresh = true;
					refreshThumbnails();
				}
			} else {
				for (_ctx->i = 0; _ctx->i < 6; _ctx->i++)
					if (_buttonSave_States[_ctx->i]->isActive()) {
						// There by saving or loading!!!
						if (_nState == MENULOAD && _curThumb[_ctx->i] != NULL) {
							// Loading
							CORO_INVOKE_1(g_vm->loadState, _statePos + _ctx->i);
							close();
						} else if (_nState == MENUSAVE && (_statePos != 0 || _ctx->i != 0)) {
							// Turn on edit mode
							_bEditSaveName = true;
							_nEditPos = _ctx->i;
							strcpy(_editName, _curThumbName[_ctx->i].c_str());
							_ctx->bRefresh = true;
						}

						break;
					}
			}
		}

		if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND) {
			if (_bQuitConfirm) {
				if (_buttonQuitNo->isActive()) {
					_bQuitConfirm = false;
					_ctx->bRefresh = true;
				} else if (_buttonQuitYes->isActive()) {
					_bQuitConfirm = false;
					_ctx->bRefresh = true;

					g_vm->quitGame();
				}
			} else {
				if (_buttonQuit->isActive()) {
					_bQuitConfirm = true;
					_buttonQuitNo->setActiveState(false);
					_buttonQuitYes->setActiveState(false);
					_ctx->bRefresh = true;
				} else if (_buttonExit->isActive())
					close();
				else if (_buttonLoad->isActive()) {
					CORO_INVOKE_1(changeState, MENULOAD);
					_ctx->bRefresh = true;
				} else if (_buttonSave->isActive()) {
					CORO_INVOKE_1(changeState, MENUSAVE);
					_ctx->bRefresh = true;
				} else if (_buttonGameMenu->isActive() && _nState != MENUGAME) {
					CORO_INVOKE_1(changeState, MENUGAME);
					_ctx->bRefresh = true;
				} else if (_buttonGfxMenu->isActive() && _nState != MENUGFX) {
					CORO_INVOKE_1(changeState, MENUGFX);
					_ctx->bRefresh = true;
				} else if (_buttonSoundMenu->isActive() && _nState != MENUSOUND) {
					CORO_INVOKE_1(changeState, MENUSOUND);
					_ctx->bRefresh = true;
				}

				if (_nState == MENUGFX) {
					// These options take effect immediately
					if (_buttonGfx_Anni30->isActive())
						GLOBALS._bCfgAnni30 = true;
					else
						GLOBALS._bCfgAnni30 = false;

					if (_buttonGfx_AntiAlias->isActive())
						GLOBALS._bCfgAntiAlias = false;
					else
						GLOBALS._bCfgAntiAlias = true;

					if (_buttonGfx_Trans->isActive())
						GLOBALS._bCfgTransparence = false;
					else
						GLOBALS._bCfgTransparence = true;
				}
			}
		}
	}

	if (_nState == MENUGAME || _nState == MENUGFX || _nState == MENUSOUND) {
		if (!_bQuitConfirm && KEYPRESS(Common::KEYCODE_ESCAPE))
			close();
	}

	if (_ctx->bRefresh)
		CORO_INVOKE_0(refreshAll);

	CORO_END_CODE;
}

void RMOptionScreen::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	int curTime;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curTime = g_vm->getTime();

#define FADE_SPEED 20
#define SYNC (_ctx->curTime - _fadeTime) / 25

	if (_bExit)
		return;

	if (_fadeStep == 1) {
		// Downhill fast
		if (_fadeTime == -1)
			_fadeY += FADE_SPEED;
		else
			_fadeY += FADE_SPEED * SYNC;
		if (_fadeY > 480) {
			_fadeY = 480;
			_fadeStep++;
		}

		// Set the part to draw the scrolling
		prim->setSrc(RMRect(0, 480 - _fadeY, 640, 480));

	} else if (_fadeStep == 2) {
		// Bounce 1
		_fadeY -= FADE_SPEED / 2 * SYNC;
		if (_fadeY < 400) {
			_fadeY = 400;
			_fadeStep++;
		}

		prim->setSrc(RMRect(0, 480 - _fadeY, 640, 480));

	} else if (_fadeStep == 3) {
		_fadeY -= FADE_SPEED / 4 * SYNC;
		if (_fadeY < 380) {
			_fadeY = 380;
			_fadeStep++;
		}

		prim->setSrc(RMRect(0, 480 - _fadeY, 640, 480));

	} else if (_fadeStep == 4) {
		// Bounce 1 - 2
		_fadeY += FADE_SPEED / 3 * SYNC;
		if (_fadeY > 420) {
			_fadeY = 420;
			_fadeStep++;
		}

		prim->setSrc(RMRect(0, 480 - _fadeY, 640, 480));

	} else if (_fadeStep == 5) {
		_fadeY += FADE_SPEED / 2 * SYNC;
		if (_fadeY > 480) {
			_fadeY = 480;
			_fadeStep++;
			g_vm->hideLocation();
		}

		prim->setSrc(RMRect(0, 480 - _fadeY, 640, 480));

	} else if (_fadeStep == 6) {
		// Menu ON

	} else if (_fadeStep == 7) {
		// Menu OFF
		g_vm->showLocation();
		_fadeStep++;

	} else if (_fadeStep == 8) {
		_fadeY -= FADE_SPEED * SYNC;
		if (_fadeY < 0) {
			_fadeY = 0;
			_fadeStep++;
		}
		prim->setSrc(RMRect(0, 480 - _fadeY, 640, 480));

	} else if (_fadeStep == 9) {
		// Hello hello!
		_bExit = true;
		_fadeStep = 0;

		// Free memory
		closeState();
		return;

	} else {
		_fadeStep = 0;
	}

	_fadeTime = _ctx->curTime;

	CORO_INVOKE_2(RMGfxWoodyBuffer::draw, bigBuf, prim);

	CORO_END_CODE;
}

void RMOptionScreen::removeThis(CORO_PARAM, bool &result) {
	if (_bExit)
		result = true;
	else
		result = false;
}

bool RMOptionScreen::loadThumbnailFromSaveState(int nState, byte *lpDestBuf, Common::String &name, byte &diff) {
	char namebuf[256];
	Common::InSaveFile *f;
	char id[4];

	// Cleans the destination
	Common::fill(lpDestBuf, lpDestBuf + 160 * 120 * 2, 0);
	name = "No name";
	diff = 10;

	// Get the savegame filename for the given slot
	Common::String buf = g_vm->getSaveStateFileName(nState);

	// Try and open the savegame
	f = g_system->getSavefileManager()->openForLoading(buf);
	if (f == NULL)
		return false;

	// Check to see if the file has a valid header
	f->read(id, 4);
	if (id[0] != 'R' || id[1] != 'M' || id[2] != 'S') {
		delete f;
		return false;
	}

	if (id[3] < 0x3) {
		// Very old version that doesn't have screenshots
		delete f;
		return true;
	}

	// Load the screenshot
	if ((id[3] >= 0x5) && (id[3] < 0x8)) {
		// Read it as an LZO compressed data block
		byte *cmpbuf;
		uint32 cmpsize, size;

		cmpbuf = new byte[160 * 120 * 4];

		// Read in the compressed data
		cmpsize = f->readUint32LE();
		f->read(cmpbuf, cmpsize);

		lzo1x_decompress(cmpbuf, cmpsize, lpDestBuf, &size);

		delete[] cmpbuf;
	} else {
		// Read in the screenshot as an uncompressed data block
		if (id[3] >= 8)
			// Recent versions use hardcoded 160x120 uncomrpessed data, so size can be skipped
			f->skip(4);

		f->read(lpDestBuf, 160 * 120 * 2);
	}

	if (id[3] >= 0x5) {
		// Read in the difficulty level
		diff = f->readByte();
	}

	if (id[3] < 0x4) {
		// Savegame version doesn't have a stored name
		delete f;
		return true;
	}

	int bufSize = f->readByte();
	f->read(namebuf, bufSize);
	namebuf[bufSize] = '\0';
	name = namebuf;

	delete f;
	return true;
}

/****************************************************************************\
*       RMPointer Methods
\****************************************************************************/

RMPointer::RMPointer() {
	Common::fill(_pointer, _pointer + 16, (RMGfxSourceBuffer8 *)NULL);
	Common::fill(_specialPointer, _specialPointer + 16, (RMItem *)NULL);

	_nCurPointer = _nCurSpecialPointer = 0;
	_nCurCustomPointer = NULL;
}

RMPointer::~RMPointer() {
	close();
}

void RMPointer::init() {
	for (int i = 0; i < 5; i++) {
		RMResRaw res(RES_P_GO + i);

		_pointer[i] = new RMGfxSourceBuffer8RLEByteAA;
		_pointer[i]->init(res, res.width(), res.height(), false);
		_pointer[i]->loadPaletteWA(RES_P_PAL);
	}

	for (int i = 0; i < 5; i++) {
		RMRes res(RES_P_PAP1 + i);
		Common::SeekableReadStream *ds = res.getReadStream();
		_specialPointer[i] = new RMItem;
		_specialPointer[i]->readFromStream(*ds);
		delete ds;
	}

	//m_hotspot[0].set(19,5);
	_hotspot[0].set(5, 1);
	_hotspot[1].set(32, 28);
	_hotspot[2].set(45, 23);
	_hotspot[3].set(35, 25);
	_hotspot[4].set(32, 28);

	// Default=GO
	_nCurPointer = 0;
	_nCurSpecialPointer = 0;
}

void RMPointer::close() {
	for (int i = 0; i < 5; i++) {
		if (_pointer[i] != NULL) {
			delete _pointer[i];
			_pointer[i] = NULL;
		}

		if (_specialPointer[i] != NULL) {
			delete _specialPointer[i];
			_specialPointer[i] = NULL;
		}
	}
}

void RMPointer::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	int n;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Check the pointer
	_ctx->n = _nCurPointer;
	if (_ctx->n == TA_COMBINE)
		_ctx->n = TA_USE;

	_cursorHotspot = _hotspot[_ctx->n];

	// Call the Draw method of the pointer
	if (_nCurSpecialPointer == 0) {
		// WORKAROUND: updateCursor gets called too early sometimes (for example, when
		// the cursor is released over the TA_PERORATE option), via setAction.
		if (_ctx->n > 4)
			_ctx->n = 0;

		CORO_INVOKE_2(_pointer[_ctx->n]->draw, bigBuf, prim);
	} else {
		if (_nCurSpecialPointer == PTR_CUSTOM)
			CORO_INVOKE_2(_nCurCustomPointer->draw, bigBuf, prim);
		else
			// Call the draw on the special pointer
			CORO_INVOKE_2(_specialPointer[_nCurSpecialPointer - 1]->draw, bigBuf, prim);
	}

	CORO_END_CODE;
}

int RMPointer::curAction() {
	if (_nCurSpecialPointer != 0)
		return 0;

	return _nCurPointer;
}

/**
 * Show the cursor
 */
void RMPointer::showCursor() {
	if (!CursorMan.isVisible()) {
		CursorMan.showMouse(true);

		updateCursor();
	}
}

/**
 * Hide the cursor
 */
void RMPointer::hideCursor() {
	if (CursorMan.isVisible()) {
		CursorMan.showMouse(false);
	}
}

void RMPointer::doFrame() {
	// Update the cursor animation if needed.
	if (_nCurSpecialPointer == 0 || _nCurSpecialPointer == PTR_CUSTOM)
		return;

	RMGfxTargetBuffer buf;
	if (_specialPointer[_nCurSpecialPointer - 1]->doFrame(&buf, false))
		updateCursor();
}

void RMPointer::updateCursor() {
	// Create an intermediate buffer and draw the cursor onto it
	RMGfxTargetBuffer buf;
	buf.create(64, 64, 16);
	RMGfxPrimitive prim;

	draw(Common::nullContext, buf, &prim);

	// Get a pointer to the cursor data
	byte *cursorData = buf;

	// If in black & white mode, convert the cursor
	if (GLOBALS._bCfgAnni30) {
		if (!RMGfxTargetBuffer::_precalcTable) {
			RMGfxTargetBuffer::createBWPrecalcTable();
		}
		uint16 *src = (uint16 *)cursorData;
		for (int i = 0; i < 64; i++) {
			uint16 *lineP = src;
			for (int j = 0; j < 64; j++) {
				lineP[j] = RMGfxTargetBuffer::_precalcTable[lineP[j] & 0x7FFF];
			}
			src += 64;
		}
	}

	// Get the raw pixel data and set the cursor to it
	Graphics::PixelFormat pixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
	CursorMan.replaceCursor(cursorData, 64, 64, _cursorHotspot._x, _cursorHotspot._y, 0, 1, &pixelFormat);
}

/**
 * Sets a new action as current
 */
void RMPointer::setAction(RMTonyAction action) {
	_nCurPointer = action;
	updateCursor();
}

/**
 * Sets a new pointer
 */
void RMPointer::setSpecialPointer(PointerType ptr) {
	_nCurSpecialPointer = ptr;
	if (_nCurSpecialPointer && _nCurSpecialPointer != PTR_CUSTOM)
		_specialPointer[ptr - 1]->setPattern(1);

	updateCursor();
}

RMPointer::PointerType RMPointer::getSpecialPointer() {
	return (PointerType)_nCurSpecialPointer;
}

/**
 * Set the new custom pointer
 */
void RMPointer::setCustomPointer(RMGfxSourceBuffer8 *ptr) {
	_nCurCustomPointer = ptr;
	updateCursor();
}

} // End of namespace Tony
