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
 *
 */

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "common/system.h"
#include "common/savefile.h"
#include "tony/mpal/mpal.h"
#include "tony/mpal/memory.h"
#include "tony/custom.h"
#include "tony/font.h"
#include "tony/game.h"
#include "tony/gfxcore.h"
#include "tony/tony.h"
#include "tony/tonychar.h"
#include "tony/utils.h"

namespace Tony {

static const char *const kAmbianceFile[] =  {
	"None",
	"1.ADP",    // Grilli.WAV
	"2.ADP",    // Grilli-Ovattati.WAV
	"3.ADP",    // Grilli-Vento.WAV
	"3.ADP",    // Grilli-Vento1.WAV
	"5.ADP",    // Vento1.WAV
	"4.ADP",    // Mare1.WAV
	"6.ADP"     // Mare1.WAV half volume
};

static const MusicFileEntry kMusicFiles[] =  {
	{"00.ADP", 0},    {"01.ADP", 0},    {"02.ADP", 0},    {"03.ADP", 0},
	{"04.ADP", 0},    {"05.ADP", 0},    {"06.ADP", 0},    {"07.ADP", 0},
	{"08.ADP", 2450}, {"09.ADP", 0},    {"10.ADP", 0},    {"11.ADP", 0},
	{"12.ADP", 0},    {"13.ADP", 0},    {"14.ADP", 0},    {"15.ADP", 0},
	{"16.ADP", 0},    {"17.ADP", 0},    {"18.ADP", 0},    {"19.ADP", 0},
	{"20.ADP", 0},    {"21.ADP", 0},    {"22.ADP", 0},    {"23.ADP", 0},
	{"24.ADP", 0},    {"25.ADP", 0},    {"26.ADP", 0},    {"27.ADP", 0},
	{"28.ADP", 1670}, {"29.ADP", 0},    {"30.ADP", 0},    {"31.ADP", 0},
	{"32.ADP", 2900}, {"33.ADP", 0},    {"34.ADP", 0},    {"35.ADP", 0},
	{"36.ADP", 0},    {"37.ADP", 0},    {"38.ADP", 0},    {"39.ADP", 0},
	{"40.ADP", 0},    {"41.ADP", 1920}, {"42.ADP", 1560}, {"43.ADP", 1920},
	{"44.ADP", 1920}, {"45.ADP", 1920}, {"46.ADP", 1920}, {"47.ADP", 1920},
	{"48.ADP", 1920}, {"49.ADP", 1920}, {"50.ADP", 1920}, {"51.ADP", 1920},
	{"52.ADP", 1920}, {"53.ADP", 0},    {"54.ADP", 0},    {"55.ADP", 0},
	{"56.ADP", 0},    {"57.ADP", 0},    {"58.ADP", 0},    {"59.ADP", 0}
};


static const char *const kJingleFileNames[] = {
	"S00.ADP", "S01.ADP", "S02.ADP", "S03.ADP", "S04.ADP",
	"S05.ADP", "S06.ADP", "S07.ADP", "S08.ADP", "S09.ADP",
	"S10.ADP", "S11.ADP", "S12.ADP", "S13.ADP", "S14.ADP",
	"S15.ADP", "S16.ADP", "S17.ADP", "S18.ADP"
};

void reapplyChangedHotspot() {
	for (int i = 0; i < GLOBALS._curChangedHotspot; i++)
		GLOBALS._loc->getItemFromCode(GLOBALS._changedHotspot[i]._dwCode)->changeHotspot(RMPoint(GLOBALS._changedHotspot[i]._nX, GLOBALS._changedHotspot[i]._nY));
}

void saveChangedHotspot(Common::OutSaveFile *f) {
	f->writeByte(GLOBALS._curChangedHotspot);
	if (GLOBALS._curChangedHotspot > 0) {
		for (int i = 0; i < GLOBALS._curChangedHotspot; ++i)
			GLOBALS._changedHotspot[i].save(f);
	}
}

void loadChangedHotspot(Common::InSaveFile *f) {
	GLOBALS._curChangedHotspot = f->readByte();

	if (GLOBALS._curChangedHotspot > 0) {
		for (int i = 0; i < GLOBALS._curChangedHotspot; ++i)
			GLOBALS._changedHotspot[i].load(f);
	}
}

/**
 * Classes required for custom functions
 *
 * Tony (To Move him)     -> You can do MPAL through the animation? I really think so
 *
 * SendMessage -> I'd say just theEngine.SendMessage()
 * ChangeLocation -> theEngine.ChangeLocation()
 * AddInventory -> theEngine.AddInventory()
*/

void mCharResetCodes() {
	for (int i = 0; i < 10; i++)
		GLOBALS._mCharacter[i]._item = GLOBALS._loc->getItemFromCode(GLOBALS._mCharacter[i]._code);
	for (int i = 0; i < 10; i++)
		GLOBALS._character[i]._item = GLOBALS._loc->getItemFromCode(GLOBALS._character[i]._code);
}

void charsSaveAll(Common::OutSaveFile *f) {
	for (int i = 0; i < 10; i++) {
		f->writeByte(GLOBALS._isMChar[i]);
		if (GLOBALS._isMChar[i]) {
			GLOBALS._mCharacter[i].save(f);
		} else {
			GLOBALS._character[i].save(f);
		}
	}
}

void charsLoadAll(Common::InSaveFile *f) {
	for (int i = 0; i < 10; i++) {
		GLOBALS._isMChar[i] = f->readByte();
		if (GLOBALS._isMChar[i])
			GLOBALS._mCharacter[i].load(f);
		else
			GLOBALS._character[i].load(f);
	}
}

void faceToMe(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_STANDDOWN);
}

void backToMe(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_STANDUP);
}

void leftToMe(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_STANDLEFT);
}

void rightToMe(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_STANDRIGHT);
}

void tonySetPerorate(CORO_PARAM, uint32 bStatus, uint32, uint32, uint32) {
	g_vm->getEngine()->setPerorate(bStatus);
}

void mySleep(CORO_PARAM, uint32 dwTime, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_1(CoroScheduler.sleep, dwTime);

	CORO_END_CODE;
}

void setAlwaysDisplay(CORO_PARAM, uint32 val, uint32, uint32, uint32) {
	GLOBALS._bAlwaysDisplay = (val != 0);
}

void setPointer(CORO_PARAM, uint32 dwPointer, uint32, uint32, uint32) {
	switch (dwPointer) {
	case 1:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_ARROWUP);
		break;
	case 2:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_ARROWDOWN);
		break;
	case 3:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_ARROWLEFT);
		break;
	case 4:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_ARROWRIGHT);
		break;
	case 5:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_ARROWMAP);
		break;

	default:
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_NONE);
		break;
	}
}

VoiceHeader *searchVoiceHeader(uint32 codehi, uint32 codelo) {
	int code = (codehi << 16) | codelo;

	if (g_vm->_voices.size() == 0)
		return NULL;

	for (uint i = 0; i < g_vm->_voices.size(); i++) {
		if (g_vm->_voices[i]._code == code)
			return &g_vm->_voices[i];
	}

	return NULL;
}

void sendTonyMessage(CORO_PARAM, uint32 dwMessage, uint32 nX, uint32 nY, uint32) {
	CORO_BEGIN_CONTEXT;
	RMMessage msg;
	int i;
	int curOffset;
	VoiceHeader *curVoc;
	FPSfx *voice;
	RMTextDialog text;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curOffset = 0;

	if (GLOBALS._bSkipIdle)
		return;

	_ctx->msg.load(dwMessage);
	if (!_ctx->msg.isValid())
		return;

	_ctx->curVoc = searchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Is positioned within the database of entries beginning at the first
		_ctx->curOffset = _ctx->curVoc->_offset;

		// First time allocation
		g_vm->_vdbFP.seek(_ctx->curOffset);
		g_vm->_theSound.createSfx(&_ctx->voice);

		_ctx->voice->loadVoiceFromVDB(g_vm->_vdbFP);
		_ctx->curOffset = g_vm->_vdbFP.pos();

		_ctx->voice->setLoop(false);
	}

	if (GLOBALS._nTonyNextTalkType != GLOBALS._tony->TALK_NORMAL) {
		CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._nTonyNextTalkType);

		if (!GLOBALS._bStaticTalk)
			GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;
	} else {
		if (_ctx->msg.numPeriods() > 1)
			CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._tony->TALK_HIPS);
		else
			CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._tony->TALK_NORMAL);
	}

	if (GLOBALS._curBackText)
		CORO_INVOKE_0(GLOBALS._curBackText->hide);

	GLOBALS._bTonyIsSpeaking = true;

	for (_ctx->i = 0; _ctx->i < _ctx->msg.numPeriods() && !GLOBALS._bSkipIdle; _ctx->i++) {
		_ctx->text.setInput(GLOBALS._input);

		// Alignment
		_ctx->text.setAlignType(RMText::HCENTER, RMText::VBOTTOM);

		// Color
		_ctx->text.setColor(0, 255, 0);

		// Writes the text
		_ctx->text.writeText(_ctx->msg[_ctx->i], 0);

		// Set the position
		if (nX == 0 && nY == 0)
			_ctx->text.setPosition(GLOBALS._tony->position() - RMPoint(0, 130) - GLOBALS._loc->scrollPosition());
		else
			_ctx->text.setPosition(RMPoint(nX, nY) - GLOBALS._loc->scrollPosition());

		// Handling for always display
		if (GLOBALS._bAlwaysDisplay) {
			_ctx->text.setAlwaysDisplay();
			_ctx->text.forceTime();
		}

		// Record the text
		g_vm->getEngine()->linkGraphicTask(&_ctx->text);

		if (_ctx->curVoc) {
			if (_ctx->i == 0) {
				_ctx->voice->play();
				_ctx->text.setCustomSkipHandle2(_ctx->voice->_hEndOfBuffer);
			} else {
				g_vm->_vdbFP.seek(_ctx->curOffset);
				g_vm->_theSound.createSfx(&_ctx->voice);
				_ctx->voice->loadVoiceFromVDB(g_vm->_vdbFP);

				_ctx->curOffset = g_vm->_vdbFP.pos();
				_ctx->voice->setLoop(false);
				_ctx->voice->play();
				_ctx->text.setCustomSkipHandle2(_ctx->voice->_hEndOfBuffer);
			}
		}

		// Wait for the end of the display
		_ctx->text.setCustomSkipHandle(GLOBALS._hSkipIdle);
		CORO_INVOKE_0(_ctx->text.waitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->stop();
			_ctx->voice->release();
			_ctx->voice = NULL;
		}
	}

	GLOBALS._bTonyIsSpeaking = false;
	if (GLOBALS._curBackText)
		GLOBALS._curBackText->show();

	CORO_INVOKE_0(GLOBALS._tony->endTalk);

	CORO_END_CODE;
}

void changeBoxStatus(CORO_PARAM, uint32 nLoc, uint32 nBox, uint32 nStatus, uint32) {
	GLOBALS._boxes->changeBoxStatus(nLoc, nBox, nStatus);
}

void custLoadLocation(CORO_PARAM, uint32 nLoc, uint32 tX, uint32 tY, uint32 bUseStartPos) {
	CORO_BEGIN_CONTEXT;
	uint32 h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._curChangedHotspot = 0;
	if (bUseStartPos != 0)
		g_vm->getEngine()->loadLocation(nLoc, RMPoint(tX, tY), GLOBALS._startLocPos[nLoc]);
	else
		g_vm->getEngine()->loadLocation(nLoc, RMPoint(tX, tY), RMPoint(-1, -1));

	_ctx->h = mpalQueryDoAction(0, nLoc, 0);

	// On Enter?
	if (_ctx->h != CORO_INVALID_PID_VALUE)
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);

	CORO_END_CODE;
}

void sendFullscreenMsgStart(CORO_PARAM, uint32 nMsg, uint32 nFont, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	RMMessage *msg;
	RMGfxClearTask clear;
	int i;
	RMTextDialog text;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(nMsg);

	GLOBALS._fullScreenMessageLoc = GLOBALS._loc->TEMPGetNumLoc();
	GLOBALS._fullScreenMessagePt = GLOBALS._tony->position();

	if (GLOBALS._bSkipIdle)
		return;

	CORO_INVOKE_2(g_vm->getEngine()->unloadLocation, false, NULL);
	GLOBALS._tony->hide();

	for (_ctx->i = 0; _ctx->i < _ctx->msg->numPeriods() && !GLOBALS._bSkipIdle; _ctx->i++) {
		_ctx->text.setInput(GLOBALS._input);

		// Alignment
		_ctx->text.setAlignType(RMText::HCENTER, RMText::VCENTER);

		// Forces the text to disappear in time
		_ctx->text.forceTime();

		// Color
		_ctx->text.setColor(255, 255, 255);

		// Write the text
		if (nFont == 0)
			_ctx->text.writeText((*_ctx->msg)[_ctx->i], 1);
		else if (nFont == 1)
			_ctx->text.writeText((*_ctx->msg)[_ctx->i], 0);

		// Set the position
		_ctx->text.setPosition(RMPoint(320, 240));

		_ctx->text.setAlwaysDisplay();
		_ctx->text.forceTime();

		// Record the text
		g_vm->getEngine()->linkGraphicTask(&_ctx->clear);
		g_vm->getEngine()->linkGraphicTask(&_ctx->text);

		// Wait for the end of display
		_ctx->text.setCustomSkipHandle(GLOBALS._hSkipIdle);
		CORO_INVOKE_0(_ctx->text.waitForEndDisplay);
	}

	delete _ctx->msg;

	CORO_END_CODE;
}

void clearScreen(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	char buf[256];
	RMGfxClearTask clear;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	g_vm->getEngine()->linkGraphicTask(&_ctx->clear);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

	// WORKAROUND: This fixes a bug in the original source where the linked clear task
	// didn't have time to be drawn and removed from the draw list before the method
	// ended, thus remaining in the draw list and causing a later crash
	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

	CORO_END_CODE;
}

void sendFullscreenMsgEnd(CORO_PARAM, uint32 bNotEnableTony, uint32, uint32, uint32) {
	g_vm->getEngine()->loadLocation(GLOBALS._fullScreenMessageLoc, RMPoint(GLOBALS._fullScreenMessagePt._x, GLOBALS._fullScreenMessagePt._y), RMPoint(-1, -1));
	if (!bNotEnableTony)
		GLOBALS._tony->show();

	mCharResetCodes();
	reapplyChangedHotspot();
}


void sendFullscreenMessage(CORO_PARAM, uint32 nMsg, uint32 nFont, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(sendFullscreenMsgStart, nMsg, nFont, 0, 0);
	CORO_INVOKE_4(sendFullscreenMsgEnd, 0, 0, 0, 0);

	CORO_END_CODE;
}

void noBullsEye(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bNoBullsEye = true;
}

void closeLocation(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!GLOBALS._bNoBullsEye) {
		g_vm->getEngine()->initWipe(1);
		CORO_INVOKE_0(g_vm->getEngine()->waitWipeEnd);
	}

	g_vm->stopMusic(4);

	// On exit, unload
	CORO_INVOKE_2(g_vm->getEngine()->unloadLocation, true, NULL);

	CORO_END_CODE;
}

void changeLocation(CORO_PARAM, uint32 nLoc, uint32 tX, uint32 tY, uint32 bUseStartPos) {
	CORO_BEGIN_CONTEXT;
	uint32 h;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!GLOBALS._bNoBullsEye) {
		g_vm->getEngine()->initWipe(1);
		CORO_INVOKE_0(g_vm->getEngine()->waitWipeEnd);
	}

	if (GLOBALS._lastTappeto != GLOBALS._ambiance[nLoc]) {
		g_vm->stopMusic(4);
	}

	// On exit, unfreeze
	CORO_INVOKE_2(g_vm->getEngine()->unloadLocation, true, NULL);

	GLOBALS._curChangedHotspot = 0;
	if (bUseStartPos != 0)
		g_vm->getEngine()->loadLocation(nLoc, RMPoint(tX, tY), GLOBALS._startLocPos[nLoc]);
	else
		g_vm->getEngine()->loadLocation(nLoc, RMPoint(tX, tY), RMPoint(-1, -1));

	if (GLOBALS._lastTappeto != GLOBALS._ambiance[nLoc]) {
		GLOBALS._lastTappeto = GLOBALS._ambiance[nLoc];
		if (GLOBALS._lastTappeto != 0)
			g_vm->playMusic(4, kAmbianceFile[GLOBALS._lastTappeto], 0, true, 2000);
	}

	if (!GLOBALS._bNoBullsEye) {
		g_vm->getEngine()->initWipe(2);
	}

	_ctx->h = mpalQueryDoAction(0, nLoc, 0);

	if (!GLOBALS._bNoBullsEye) {
		CORO_INVOKE_0(g_vm->getEngine()->waitWipeEnd);
		g_vm->getEngine()->closeWipe();
	}

	GLOBALS._bNoBullsEye = false;

	// On Enter?
	if (_ctx->h != CORO_INVALID_PID_VALUE)
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);

	CORO_END_CODE;
}

void setLocStartPosition(CORO_PARAM, uint32 nLoc, uint32 lX, uint32 lY, uint32) {
	GLOBALS._startLocPos[nLoc].set(lX, lY);
}

void saveTonyPosition(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._saveTonyPos = GLOBALS._tony->position();
	GLOBALS._saveTonyLoc = GLOBALS._loc->TEMPGetNumLoc();
}

void restoreTonyPosition(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(changeLocation, GLOBALS._saveTonyLoc, GLOBALS._saveTonyPos._x, GLOBALS._saveTonyPos._y, 0);

	mCharResetCodes();

	CORO_END_CODE;
}

void disableInput(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->getEngine()->disableInput();
}

void enableInput(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->getEngine()->enableInput();
}

void stopTony(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->stopNoAction(coroParam);
}

void custEnableGUI(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.EnableGUI();
}

void custDisableGUI(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS.DisableGUI();
}

void tonyGenericTake1(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->take(nDirection, 0);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	CORO_END_CODE;
}

void tonyGenericTake2(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->take(nDirection, 1);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	GLOBALS._tony->take(nDirection, 2);

	CORO_END_CODE;
}

void tonyGenericPut1(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->put(nDirection, 0);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	CORO_END_CODE;
}

void tonyGenericPut2(CORO_PARAM, uint32 nDirection) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->put(nDirection, 1);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	GLOBALS._tony->put(nDirection, 2);

	CORO_END_CODE;
}

void tonyTakeUp1(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericTake1(coroParam, 0);
}

void tonyTakeMid1(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericTake1(coroParam, 1);
}

void tonyTakeDown1(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericTake1(coroParam, 2);
}

void tonyTakeUp2(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericTake2(coroParam, 0);
}

void tonyTakeMid2(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericTake2(coroParam, 1);
}

void tonyTakeDown2(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericTake2(coroParam, 2);
}

void tonyPutUp1(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericPut1(coroParam, 0);
}

void tonyPutMid1(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericPut1(coroParam, 1);
}

void tonyPutDown1(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericPut1(coroParam, 2);
}

void tonyPutUp2(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericPut2(coroParam, 0);
}

void tonyPutMid2(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericPut2(coroParam, 1);
}

void tonyPutDown2(CORO_PARAM, uint32, uint32, uint32, uint32) {
	tonyGenericPut2(coroParam, 2);
}

void tonyOnTheFloor(CORO_PARAM, uint32 dwParte, uint32, uint32, uint32) {
	if (dwParte == 0)
		GLOBALS._tony->setPattern(GLOBALS._tony->PAT_ONTHEFLOORLEFT);
	else
		GLOBALS._tony->setPattern(GLOBALS._tony->PAT_ONTHEFLOORRIGHT);
}

void tonyGetUp(CORO_PARAM, uint32 dwParte, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (dwParte == 0)
		GLOBALS._tony->setPattern(GLOBALS._tony->PAT_GETUPLEFT);
	else
		GLOBALS._tony->setPattern(GLOBALS._tony->PAT_GETUPRIGHT);

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	CORO_END_CODE;
}

void tonyShepherdess(CORO_PARAM, uint32 bIsPast, uint32, uint32, uint32) {
	GLOBALS._tony->setShepherdess(bIsPast);
}

void tonyWhistle(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WHISTLERIGHT);
	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);

	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_STANDRIGHT);

	CORO_END_CODE;
}

void tonySetNumTexts(uint32 dwText) {
	GLOBALS._dwTonyNumTexts = dwText;
	GLOBALS._bTonyInTexts = false;
}

void tonyLaugh(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_LAUGH;
}

void tonyGiggle(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_LAUGH2;
}

void tonyHips(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_HIPS;
}

void tonySing(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_SING;
}

void tonyIndicate(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_INDICATE;
}

void tonyScaredWithHands(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_SCARED;
}

void tonyScaredWithoutHands(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_SCARED2;
}

void tonyWithHammer(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHHAMMER;
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WITHHAMMER);
}

void tonyWithGlasses(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHGLASSES;
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WITHGLASSES);
}

void tonyWithWorm(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHWORM;
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WITHWORM);
}

void tonyWithRope(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHROPE;
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WITHROPE);
}

void tonyWithSecretary(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHSECRETARY;
	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_WITHSECRETARY);
}

void tonyWithRabbitANIM(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHRABBIT;
}

void tonyWithRecipeANIM(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHRECIPE;
}

void tonyWithCardsANIM(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHCARDS;
}

void tonyWithSnowmanANIM(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHSNOWMAN;
}

void tonyWithSnowmanStart(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHSNOWMANSTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHSNOWMANSTATIC);

	CORO_END_CODE;
}

void tonyWithSnowmanEnd(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHSNOWMANSTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

void tonyWithRabbitStart(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHRABBITSTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHRABBITSTATIC);

	CORO_END_CODE;
}

void tonyWithRabbitEnd(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHRABBITSTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

void tonyWithRecipeStart(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHRECIPESTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHRECIPESTATIC);

	CORO_END_CODE;
}

void tonyWithRecipeEnd(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHRECIPESTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

void tonyWithCardsStart(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHCARDSSTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHCARDSSTATIC);

	CORO_END_CODE;
}

void tonyWithCardsEnd(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHCARDSSTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

void tonyWithNotebookStart(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITH_NOTEBOOK;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITH_NOTEBOOK);

	CORO_END_CODE;
}

void tonyWithNotebookEnd(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITH_NOTEBOOK);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

void tonyWithMegaphoneStart(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHMEGAPHONESTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHMEGAPHONESTATIC);

	CORO_END_CODE;
}

void tonyWithMegaphoneEnd(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHMEGAPHONESTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

void tonyWithBeardStart(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_WITHBEARDSTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_WITHBEARDSTATIC);

	CORO_END_CODE;
}

void tonyWithBeardEnd(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_WITHBEARDSTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

void tonyScaredStart(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_SCAREDSTATIC;
	GLOBALS._bStaticTalk = true;
	CORO_INVOKE_1(GLOBALS._tony->startStatic, GLOBALS._tony->TALK_SCAREDSTATIC);

	CORO_END_CODE;
}

void tonyScaredEnd(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(GLOBALS._tony->endStatic, GLOBALS._tony->TALK_SCAREDSTATIC);
	GLOBALS._bStaticTalk = false;
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;

	CORO_END_CODE;
}

void tonyDisgusted(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_DISGUSTED;
}

void tonySniffLeft(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_SNIFF_LEFT);
	CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);
	CORO_INVOKE_4(leftToMe, 0, 0, 0, 0);

	CORO_END_CODE;
}

void tonySniffRight(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._tony->setPattern(GLOBALS._tony->PAT_SNIFF_RIGHT);
	CORO_INVOKE_0(GLOBALS._tony->waitForEndPattern);
	CORO_INVOKE_4(rightToMe, 0, 0, 0, 0);

	CORO_END_CODE;
}

void tonySarcastic(CORO_PARAM, uint32 dwText, uint32, uint32, uint32) {
	tonySetNumTexts(dwText);
	GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_SARCASTIC;
}

void tonyMacbeth(CORO_PARAM, uint32 nPos, uint32, uint32, uint32) {
	switch (nPos) {
	case 1:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH1;
		break;
	case 2:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH2;
		break;
	case 3:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH3;
		break;
	case 4:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH4;
		break;
	case 5:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH5;
		break;
	case 6:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH6;
		break;
	case 7:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH7;
		break;
	case 8:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH8;
		break;
	case 9:
		GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_MACBETH9;
		break;
	}
}

void enableTony(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._tony->show();
}

void disableTony(CORO_PARAM, uint32 bShowShadow, uint32, uint32, uint32) {
	GLOBALS._tony->hide(bShowShadow);
}

void waitForPatternEnd(CORO_PARAM, uint32 nItem, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	RMItem *item;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->item = GLOBALS._loc->getItemFromCode(nItem);

	if (!GLOBALS._bSkipIdle && _ctx->item != NULL)
		CORO_INVOKE_1(_ctx->item->waitForEndPattern, GLOBALS._hSkipIdle);

	CORO_END_CODE;
}

void setTonyPosition(CORO_PARAM, uint32 nX, uint32 nY, uint32 nLoc, uint32) {
	GLOBALS._tony->setPosition(RMPoint(nX, nY), nLoc);
}

void moveTonyAndWait(CORO_PARAM, uint32 nX, uint32 nY, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// WORKAROUND: Delay for a frame before starting the move to give any previous move time to finish.
	// This fixes a bug in the first scene where if you immediately 'Use Door', Tony moves to the door,
	// and then floats to the right rather than properly walking.
	CORO_SLEEP(1);

	CORO_INVOKE_1(GLOBALS._tony->move, RMPoint(nX, nY));

	if (!GLOBALS._bSkipIdle)
		CORO_INVOKE_0(GLOBALS._tony->waitForEndMovement);

	CORO_END_CODE;
}

void moveTony(CORO_PARAM, uint32 nX, uint32 nY, uint32, uint32) {
	GLOBALS._tony->move(coroParam, RMPoint(nX, nY));
}

void scrollLocation(CORO_PARAM, uint32 nX, uint32 nY, uint32 sX, uint32 sY) {
	CORO_BEGIN_CONTEXT;
	int lx, ly;
	RMPoint pt;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Take the scroll coordinates
	_ctx->lx = (int32)nX;
	_ctx->ly = (int32)nY;

	_ctx->pt = GLOBALS._loc->scrollPosition();

	while ((_ctx->lx != 0 || _ctx->ly != 0) && !GLOBALS._bSkipIdle) {
		if (_ctx->lx > 0) {
			_ctx->lx -= (int32)sX;
			if (_ctx->lx < 0)
				_ctx->lx = 0;
			_ctx->pt.offset((int32)sX, 0);
		} else if (_ctx->lx < 0) {
			_ctx->lx += (int32)sX;
			if (_ctx->lx > 0)
				_ctx->lx = 0;
			_ctx->pt.offset(-(int32)sX, 0);
		}

		if (_ctx->ly > 0) {
			_ctx->ly -= sY;
			if (_ctx->ly < 0)
				_ctx->ly = 0;
			_ctx->pt.offset(0, sY);
		} else if (_ctx->ly < 0) {
			_ctx->ly += sY;
			if (_ctx->ly > 0)
				_ctx->ly = 0;
			_ctx->pt.offset(0, -(int32)sY);
		}

		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

		GLOBALS._loc->setScrollPosition(_ctx->pt);
		GLOBALS._tony->setScrollPosition(_ctx->pt);
	}

	CORO_END_CODE;
}

void syncScrollLocation(CORO_PARAM, uint32 nX, uint32 nY, uint32 sX, uint32 sY) {
	CORO_BEGIN_CONTEXT;
	int lx, ly;
	RMPoint pt, startpt;
	uint32 dwStartTime, dwCurTime, dwTotalTime;
	uint32 stepX, stepY;
	int dimx, dimy;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Take the scroll coordinates
	_ctx->lx = (int32)nX;
	_ctx->ly = (int32)nY;
	_ctx->dimx = _ctx->lx;
	_ctx->dimy = _ctx->ly;
	if (_ctx->lx < 0)
		_ctx->dimx = -_ctx->lx;

	if (_ctx->ly < 0)
		_ctx->dimy = -_ctx->ly;

	_ctx->stepX = sX;
	_ctx->stepY = sY;

	_ctx->startpt = GLOBALS._loc->scrollPosition();

	_ctx->dwStartTime = g_vm->getTime();

	if (sX)
		_ctx->dwTotalTime = _ctx->dimx * (1000 / 35) / sX;
	else
		_ctx->dwTotalTime = _ctx->dimy * (1000 / 35) / sY;

	while ((_ctx->lx != 0 || _ctx->ly != 0) && !GLOBALS._bSkipIdle) {
		_ctx->dwCurTime = g_vm->getTime() - _ctx->dwStartTime;
		if (_ctx->dwCurTime > _ctx->dwTotalTime)
			break;

		_ctx->pt = _ctx->startpt;

		if (sX) {
			if (_ctx->lx > 0)
				_ctx->pt._x += (_ctx->dimx * _ctx->dwCurTime) / _ctx->dwTotalTime;
			else
				_ctx->pt._x -= (_ctx->dimx * _ctx->dwCurTime) / _ctx->dwTotalTime;
		} else {
			if (_ctx->ly > 0)
				_ctx->pt._y += (_ctx->dimy * _ctx->dwCurTime) / _ctx->dwTotalTime;
			else
				_ctx->pt._y -= (_ctx->dimy * _ctx->dwCurTime) / _ctx->dwTotalTime;
		}

		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

		GLOBALS._loc->setScrollPosition(_ctx->pt);
		GLOBALS._tony->setScrollPosition(_ctx->pt);
	}

	// Set the position finale
	if (sX) {
		if (_ctx->lx > 0)
			_ctx->pt._x = _ctx->startpt._x + _ctx->dimx;
		else
			_ctx->pt._x = _ctx->startpt._x - _ctx->dimx;
	} else {
		if (_ctx->ly > 0)
			_ctx->pt._y = _ctx->startpt._y + _ctx->dimy;
		else
			_ctx->pt._y = _ctx->startpt._y - _ctx->dimy;
	}

	GLOBALS._loc->setScrollPosition(_ctx->pt);
	GLOBALS._tony->setScrollPosition(_ctx->pt);

	CORO_END_CODE;
}

void changeHotspot(CORO_PARAM, uint32 dwCode, uint32 nX, uint32 nY, uint32) {
	int i;

	for (i = 0; i < GLOBALS._curChangedHotspot; i++) {
		if (GLOBALS._changedHotspot[i]._dwCode == dwCode) {
			GLOBALS._changedHotspot[i]._nX = nX;
			GLOBALS._changedHotspot[i]._nY = nY;
			break;
		}
	}

	if (i == GLOBALS._curChangedHotspot) {
		GLOBALS._changedHotspot[i]._dwCode = dwCode;
		GLOBALS._changedHotspot[i]._nX = nX;
		GLOBALS._changedHotspot[i]._nY = nY;
		GLOBALS._curChangedHotspot++;
	}

	GLOBALS._loc->getItemFromCode(dwCode)->changeHotspot(RMPoint(nX, nY));
}

void autoSave(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->autoSave(coroParam);
}

void abortGame(CORO_PARAM, uint32, uint32, uint32, uint32) {
	debug(1, "script called abortGame");
	g_vm->quitGame();
}

void shakeScreen(CORO_PARAM, uint32 nScosse, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	uint32 i;
	uint32 curTime;
	int dirx, diry;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curTime = g_vm->getTime();

	_ctx->dirx = 1;
	_ctx->diry = 1;

	while (g_vm->getTime() < _ctx->curTime + nScosse) {
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

		GLOBALS._loc->setFixedScroll(RMPoint(1 * _ctx->dirx, 1 * _ctx->diry));
		GLOBALS._tony->setFixedScroll(RMPoint(1 * _ctx->dirx, 1 * _ctx->diry));

		_ctx->i = g_vm->_randomSource.getRandomNumber(2);

		if (_ctx->i == 0 || _ctx->i == 2)
			_ctx->dirx = -_ctx->dirx;
		else if (_ctx->i == 1 || _ctx->i == 2)
			_ctx->diry = -_ctx->diry;
	}

	GLOBALS._loc->setFixedScroll(RMPoint(0, 0));
	GLOBALS._tony->setFixedScroll(RMPoint(0, 0));

	CORO_END_CODE;
}

/*
 *  Characters
 */

void charSetCode(CORO_PARAM, uint32 nChar, uint32 nCode, uint32, uint32) {
	assert(nChar < 16);
	GLOBALS._character[nChar]._code = nCode;
	GLOBALS._character[nChar]._item = GLOBALS._loc->getItemFromCode(nCode);
	GLOBALS._character[nChar]._r = 255;
	GLOBALS._character[nChar]._g = 255;
	GLOBALS._character[nChar]._b = 255;
	GLOBALS._character[nChar]._talkPattern = 0;
	GLOBALS._character[nChar]._startTalkPattern = 0;
	GLOBALS._character[nChar]._endTalkPattern = 0;
	GLOBALS._character[nChar]._standPattern = 0;

	GLOBALS._isMChar[nChar] = false;
}

void charSetColor(CORO_PARAM, uint32 nChar, uint32 r, uint32 g, uint32 b) {
	assert(nChar < 16);
	GLOBALS._character[nChar]._r = r;
	GLOBALS._character[nChar]._g = g;
	GLOBALS._character[nChar]._b = b;
}

void charSetTalkPattern(CORO_PARAM, uint32 nChar, uint32 tp, uint32 sp, uint32) {
	assert(nChar < 16);
	GLOBALS._character[nChar]._talkPattern = tp;
	GLOBALS._character[nChar]._standPattern = sp;
}

void charSetStartEndTalkPattern(CORO_PARAM, uint32 nChar, uint32 sp, uint32 ep, uint32) {
	assert(nChar < 16);
	GLOBALS._character[nChar]._startTalkPattern = sp;
	GLOBALS._character[nChar]._endTalkPattern = ep;
}

void charSendMessage(CORO_PARAM, uint32 nChar, uint32 dwMessage, uint32 bIsBack, uint32) {
	CORO_BEGIN_CONTEXT;
	RMMessage *msg;
	int i;
	RMPoint pt;
	RMTextDialog *text;
	int curOffset;
	VoiceHeader *curVoc;
	FPSfx *voice;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(dwMessage);
	_ctx->curOffset = 0;

	assert(nChar < 16);
	_ctx->pt = GLOBALS._character[nChar]._item->calculatePos() - RMPoint(-60, 20) - GLOBALS._loc->scrollPosition();

	if (GLOBALS._character[nChar]._startTalkPattern != 0) {
		GLOBALS._character[nChar]._item->setPattern(GLOBALS._character[nChar]._startTalkPattern);

		CORO_INVOKE_0(GLOBALS._character[nChar]._item->waitForEndPattern);
	}

	GLOBALS._character[nChar]._item->setPattern(GLOBALS._character[nChar]._talkPattern);

	_ctx->curVoc = searchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Position within the database of entries, beginning at the first
		g_vm->_vdbFP.seek(_ctx->curVoc->_offset);
		_ctx->curOffset = _ctx->curVoc->_offset;
	}

	for (_ctx->i = 0; _ctx->i < _ctx->msg->numPeriods() && !GLOBALS._bSkipIdle; _ctx->i++) {
		if (bIsBack) {
			GLOBALS._curBackText = _ctx->text = new RMTextDialogScrolling(GLOBALS._loc);
			if (GLOBALS._bTonyIsSpeaking)
				CORO_INVOKE_0(GLOBALS._curBackText->hide);
		} else
			_ctx->text = new RMTextDialog;

		_ctx->text->setInput(GLOBALS._input);

		// Skipping
		_ctx->text->setSkipStatus(!bIsBack);

		// Alignment
		_ctx->text->setAlignType(RMText::HCENTER, RMText::VBOTTOM);

		// Color
		_ctx->text->setColor(GLOBALS._character[nChar]._r, GLOBALS._character[nChar]._g, GLOBALS._character[nChar]._b);

		// Write the text
		_ctx->text->writeText((*_ctx->msg)[_ctx->i], 0);

		// Set the position
		_ctx->text->setPosition(_ctx->pt);

		// Set the always display
		if (GLOBALS._bAlwaysDisplay) {
			_ctx->text->setAlwaysDisplay();
			_ctx->text->forceTime();
		}

		// Record the text
		g_vm->getEngine()->linkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			g_vm->_theSound.createSfx(&_ctx->voice);
			g_vm->_vdbFP.seek(_ctx->curOffset);
			_ctx->voice->loadVoiceFromVDB(g_vm->_vdbFP);
			_ctx->voice->setLoop(false);
			if (bIsBack)
				_ctx->voice->setVolume(55);
			_ctx->voice->play();
			_ctx->text->setCustomSkipHandle2(_ctx->voice->_hEndOfBuffer);
			_ctx->curOffset = g_vm->_vdbFP.pos();
		}

		// Wait for the end of display
		_ctx->text->setCustomSkipHandle(GLOBALS._hSkipIdle);
		CORO_INVOKE_0(_ctx->text->waitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->stop();
			_ctx->voice->release();
			_ctx->voice = NULL;
		}

		GLOBALS._curBackText = NULL;
		delete _ctx->text;
	}

	if (GLOBALS._character[nChar]._endTalkPattern != 0) {
		GLOBALS._character[nChar]._item->setPattern(GLOBALS._character[nChar]._endTalkPattern);
		CORO_INVOKE_0(GLOBALS._character[nChar]._item->waitForEndPattern);
	}

	GLOBALS._character[nChar]._item->setPattern(GLOBALS._character[nChar]._standPattern);
	delete _ctx->msg;

	CORO_END_CODE;
}

void addInventory(CORO_PARAM, uint32 dwCode, uint32, uint32, uint32) {
	GLOBALS._inventory->addItem(dwCode);
}

void removeInventory(CORO_PARAM, uint32 dwCode, uint32, uint32, uint32) {
	GLOBALS._inventory->removeItem(dwCode);
}

void changeInventoryStatus(CORO_PARAM, uint32 dwCode, uint32 dwStatus, uint32, uint32) {
	GLOBALS._inventory->changeItemStatus(dwCode, dwStatus);
}

/*
 * Master Characters
 */

void mCharSetCode(CORO_PARAM, uint32 nChar, uint32 nCode, uint32, uint32) {
	assert(nChar < 10);
	GLOBALS._mCharacter[nChar]._code = nCode;
	if (nCode == 0)
		GLOBALS._mCharacter[nChar]._item = NULL;
	else
		GLOBALS._mCharacter[nChar]._item = GLOBALS._loc->getItemFromCode(nCode);
	GLOBALS._mCharacter[nChar]._r = 255;
	GLOBALS._mCharacter[nChar]._g = 255;
	GLOBALS._mCharacter[nChar]._b = 255;
	GLOBALS._mCharacter[nChar]._x = -1;
	GLOBALS._mCharacter[nChar]._y = -1;
	GLOBALS._mCharacter[nChar]._bAlwaysBack = 0;

	for (int i = 0; i < 10; i++)
		GLOBALS._mCharacter[nChar]._numTalks[i] = 1;

	GLOBALS._mCharacter[nChar]._curGroup = 0;

	GLOBALS._isMChar[nChar] = true;
}

void mCharResetCode(CORO_PARAM, uint32 nChar, uint32, uint32, uint32) {
	GLOBALS._mCharacter[nChar]._item = GLOBALS._loc->getItemFromCode(GLOBALS._mCharacter[nChar]._code);
}

void mCharSetPosition(CORO_PARAM, uint32 nChar, uint32 nX, uint32 nY, uint32) {
	assert(nChar < 10);
	GLOBALS._mCharacter[nChar]._x = nX;
	GLOBALS._mCharacter[nChar]._y = nY;
}

void mCharSetColor(CORO_PARAM, uint32 nChar, uint32 r, uint32 g, uint32 b) {
	assert(nChar < 10);
	GLOBALS._mCharacter[nChar]._r = r;
	GLOBALS._mCharacter[nChar]._g = g;
	GLOBALS._mCharacter[nChar]._b = b;
}

void mCharSetNumTalksInGroup(CORO_PARAM, uint32 nChar, uint32 nGroup, uint32 nTalks, uint32) {
	assert(nChar < 10);
	assert(nGroup < 10);

	GLOBALS._mCharacter[nChar]._numTalks[nGroup] = nTalks;
}

void mCharSetCurrentGroup(CORO_PARAM, uint32 nChar, uint32 nGroup, uint32, uint32) {
	assert(nChar < 10);
	assert(nGroup < 10);

	GLOBALS._mCharacter[nChar]._curGroup = nGroup;
}

void mCharSetNumTexts(CORO_PARAM, uint32 nChar, uint32 nTexts, uint32, uint32) {
	assert(nChar < 10);

	GLOBALS._mCharacter[nChar]._numTexts = nTexts - 1;
	GLOBALS._mCharacter[nChar]._bInTexts = false;
}

void mCharSetAlwaysBack(CORO_PARAM, uint32 nChar, uint32 bAlwaysBack, uint32, uint32) {
	assert(nChar < 10);

	GLOBALS._mCharacter[nChar]._bAlwaysBack = bAlwaysBack;
}

void mCharSendMessage(CORO_PARAM, uint32 nChar, uint32 dwMessage, uint32 bIsBack, uint32 nFont) {
	CORO_BEGIN_CONTEXT;
	RMMessage *msg;
	int i;
	int parm;
	RMPoint pt;
	uint32 h;
	RMTextDialog *text;
	int curOffset;
	VoiceHeader *curVoc;
	FPSfx *voice;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(dwMessage);
	_ctx->curOffset = 0;

	assert(nChar < 10);

	bIsBack |= GLOBALS._mCharacter[nChar]._bAlwaysBack ? 1 : 0;

	// Calculates the position of the text according to the current frame
	if (GLOBALS._mCharacter[nChar]._x == -1)
		_ctx->pt = GLOBALS._mCharacter[nChar]._item->calculatePos() - RMPoint(-60, 20) - GLOBALS._loc->scrollPosition();
	else
		_ctx->pt = RMPoint(GLOBALS._mCharacter[nChar]._x, GLOBALS._mCharacter[nChar]._y);

	// Parameter for special actions: random between the spoken
	_ctx->parm = (GLOBALS._mCharacter[nChar]._curGroup * 10) + g_vm->_randomSource.getRandomNumber(
	                 GLOBALS._mCharacter[nChar]._numTalks[GLOBALS._mCharacter[nChar]._curGroup] - 1) + 1;

	// Try to run the custom function to initialize the speech
	if (GLOBALS._mCharacter[nChar]._item) {
		_ctx->h = mpalQueryDoAction(30, GLOBALS._mCharacter[nChar]._item->mpalCode(), _ctx->parm);
		if (_ctx->h != CORO_INVALID_PID_VALUE) {
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);
		}
	}

	_ctx->curVoc = searchVoiceHeader(0, dwMessage);
	_ctx->voice = NULL;
	if (_ctx->curVoc) {
		// Position within the database of entries, beginning at the first
		g_vm->_vdbFP.seek(_ctx->curVoc->_offset);
		_ctx->curOffset = _ctx->curVoc->_offset;
	}

	for (_ctx->i = 0; _ctx->i < _ctx->msg->numPeriods() && !GLOBALS._bSkipIdle; _ctx->i++) {
		// Create a different object depending on whether it's background or not
		if (bIsBack) {
			GLOBALS._curBackText = _ctx->text = new RMTextDialogScrolling(GLOBALS._loc);
			if (GLOBALS._bTonyIsSpeaking)
				CORO_INVOKE_0(GLOBALS._curBackText->hide);
		} else
			_ctx->text = new RMTextDialog;

		_ctx->text->setInput(GLOBALS._input);

		// Skipping
		_ctx->text->setSkipStatus(!bIsBack);

		// Alignment
		_ctx->text->setAlignType(RMText::HCENTER, RMText::VBOTTOM);

		// Color
		_ctx->text->setColor(GLOBALS._mCharacter[nChar]._r, GLOBALS._mCharacter[nChar]._g, GLOBALS._mCharacter[nChar]._b);

		// Write the text
		_ctx->text->writeText((*_ctx->msg)[_ctx->i], nFont);

		// Set the position
		_ctx->text->setPosition(_ctx->pt);

		// Set the always display
		if (GLOBALS._bAlwaysDisplay) {
			_ctx->text->setAlwaysDisplay();
			_ctx->text->forceTime();
		}

		// Record the text
		g_vm->getEngine()->linkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			g_vm->_theSound.createSfx(&_ctx->voice);
			g_vm->_vdbFP.seek(_ctx->curOffset);
			_ctx->voice->loadVoiceFromVDB(g_vm->_vdbFP);
			_ctx->voice->setLoop(false);
			if (bIsBack)
				_ctx->voice->setVolume(55);
			_ctx->voice->play();
			_ctx->text->setCustomSkipHandle2(_ctx->voice->_hEndOfBuffer);
			_ctx->curOffset = g_vm->_vdbFP.pos();
		}

		// Wait for the end of display
		_ctx->text->setCustomSkipHandle(GLOBALS._hSkipIdle);
		CORO_INVOKE_0(_ctx->text->waitForEndDisplay);

		if (_ctx->curVoc) {
			_ctx->voice->stop();
			_ctx->voice->release();
			_ctx->voice = NULL;
		}

		GLOBALS._curBackText = NULL;
		delete _ctx->text;
	}

	delete _ctx->msg;

	// Try to run the custom function to close the speech
	if (GLOBALS._mCharacter[nChar]._item) {
		_ctx->h = mpalQueryDoAction(31, GLOBALS._mCharacter[nChar]._item->mpalCode(), _ctx->parm);
		if (_ctx->h != CORO_INVALID_PID_VALUE)
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);
	}

	CORO_END_CODE;
}

/*
 *  Dialogs
 */

void sendDialogMessage(CORO_PARAM, uint32 nPers, uint32 nMsg, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	char *string;
	RMTextDialog *text;
	int parm;
	uint32 h;
	bool bIsBack;
	VoiceHeader *curVoc;
	FPSfx *voice;
	RMPoint pt;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->bIsBack = false;

	// The SendDialogMessage can go in the background if it is a character
	if (nPers != 0 && GLOBALS._isMChar[nPers] && GLOBALS._mCharacter[nPers]._bAlwaysBack)
		_ctx->bIsBack = true;

	_ctx->curVoc = searchVoiceHeader(GLOBALS._curDialog, nMsg);
	_ctx->voice = NULL;

	if (_ctx->curVoc) {
		// Position within the database of entries, beginning at the first
		g_vm->_vdbFP.seek(_ctx->curVoc->_offset);
		g_vm->_theSound.createSfx(&_ctx->voice);
		_ctx->voice->loadVoiceFromVDB(g_vm->_vdbFP);
		_ctx->voice->setLoop(false);
		if (_ctx->bIsBack)
			_ctx->voice->setVolume(55);
	}

	_ctx->string = mpalQueryDialogPeriod(nMsg);

	if (nPers == 0) {
		_ctx->text = new RMTextDialog;
		_ctx->text->setColor(0, 255, 0);
		_ctx->text->setPosition(GLOBALS._tony->position() - RMPoint(0, 130) - GLOBALS._loc->scrollPosition());
		_ctx->text->writeText(_ctx->string, 0);

		if (GLOBALS._dwTonyNumTexts > 0) {
			if (!GLOBALS._bTonyInTexts) {
				if (GLOBALS._nTonyNextTalkType != GLOBALS._tony->TALK_NORMAL) {
					CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._nTonyNextTalkType);
					if (!GLOBALS._bStaticTalk)
						GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;
				} else
					CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._tony->TALK_NORMAL);

				GLOBALS._bTonyInTexts = true;
			}
			GLOBALS._dwTonyNumTexts--;
		} else {
			CORO_INVOKE_1(GLOBALS._tony->startTalk, GLOBALS._nTonyNextTalkType);
			if (!GLOBALS._bStaticTalk)
				GLOBALS._nTonyNextTalkType = GLOBALS._tony->TALK_NORMAL;
		}
	} else if (!GLOBALS._isMChar[nPers]) {
		_ctx->text = new RMTextDialog;

		_ctx->pt = GLOBALS._character[nPers]._item->calculatePos() - RMPoint(-60, 20) - GLOBALS._loc->scrollPosition();

		if (GLOBALS._character[nPers]._startTalkPattern != 0) {
			GLOBALS._character[nPers]._item->setPattern(GLOBALS._character[nPers]._startTalkPattern);
			CORO_INVOKE_0(GLOBALS._character[nPers]._item->waitForEndPattern);
		}

		GLOBALS._character[nPers]._item->setPattern(GLOBALS._character[nPers]._talkPattern);

		_ctx->text->setColor(GLOBALS._character[nPers]._r, GLOBALS._character[nPers]._g, GLOBALS._character[nPers]._b);
		_ctx->text->writeText(_ctx->string, 0);
		_ctx->text->setPosition(_ctx->pt);
	} else {
		if (GLOBALS._mCharacter[nPers]._x == -1)
			_ctx->pt = GLOBALS._mCharacter[nPers]._item->calculatePos() - RMPoint(-60, 20) - GLOBALS._loc->scrollPosition();
		else
			_ctx->pt = RMPoint(GLOBALS._mCharacter[nPers]._x, GLOBALS._mCharacter[nPers]._y);

		// Parameter for special actions. Random between the spoken.
		_ctx->parm = (GLOBALS._mCharacter[nPers]._curGroup * 10) + g_vm->_randomSource.getRandomNumber(
		                 GLOBALS._mCharacter[nPers]._numTalks[GLOBALS._mCharacter[nPers]._curGroup] - 1) + 1;

		if (GLOBALS._mCharacter[nPers]._numTexts != 0 && GLOBALS._mCharacter[nPers]._bInTexts) {
			GLOBALS._mCharacter[nPers]._numTexts--;
		} else {
			// Try to run the custom function to initialize the speech
			_ctx->h = mpalQueryDoAction(30, GLOBALS._mCharacter[nPers]._item->mpalCode(), _ctx->parm);
			if (_ctx->h != CORO_INVALID_PID_VALUE)
				CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);

			GLOBALS._mCharacter[nPers]._curTalk = _ctx->parm;

			if (GLOBALS._mCharacter[nPers]._numTexts != 0) {
				GLOBALS._mCharacter[nPers]._bInTexts = true;
				GLOBALS._mCharacter[nPers]._numTexts--;
			}
		}

		if (GLOBALS._mCharacter[nPers]._bAlwaysBack) {
			_ctx->text = GLOBALS._curBackText = new RMTextDialogScrolling(GLOBALS._loc);
			if (GLOBALS._bTonyIsSpeaking)
				CORO_INVOKE_0(GLOBALS._curBackText->hide);

			_ctx->bIsBack = true;
		} else
			_ctx->text = new RMTextDialog;

		_ctx->text->setSkipStatus(!GLOBALS._mCharacter[nPers]._bAlwaysBack);
		_ctx->text->setColor(GLOBALS._mCharacter[nPers]._r, GLOBALS._mCharacter[nPers]._g, GLOBALS._mCharacter[nPers]._b);
		_ctx->text->writeText(_ctx->string, 0);
		_ctx->text->setPosition(_ctx->pt);
	}

	if (!GLOBALS._bSkipIdle) {
		_ctx->text->setInput(GLOBALS._input);
		if (GLOBALS._bAlwaysDisplay) {
			_ctx->text->setAlwaysDisplay();
			_ctx->text->forceTime();
		}
		_ctx->text->setAlignType(RMText::HCENTER, RMText::VBOTTOM);
		g_vm->getEngine()->linkGraphicTask(_ctx->text);

		if (_ctx->curVoc) {
			_ctx->voice->play();
			_ctx->text->setCustomSkipHandle2(_ctx->voice->_hEndOfBuffer);
		}

		// Wait for the end of display
		_ctx->text->setCustomSkipHandle(GLOBALS._hSkipIdle);
		CORO_INVOKE_0(_ctx->text->waitForEndDisplay);
	}

	if (_ctx->curVoc) {
		_ctx->voice->stop();
		_ctx->voice->release();
		_ctx->voice = NULL;
	}

	if (nPers != 0) {
		if (!GLOBALS._isMChar[nPers]) {
			if (GLOBALS._character[nPers]._endTalkPattern != 0) {
				GLOBALS._character[nPers]._item->setPattern(GLOBALS._character[nPers]._endTalkPattern);
				CORO_INVOKE_0(GLOBALS._character[nPers]._item->waitForEndPattern);
			}

			GLOBALS._character[nPers]._item->setPattern(GLOBALS._character[nPers]._standPattern);
			delete _ctx->text;
		} else {
			if ((GLOBALS._mCharacter[nPers]._bInTexts && GLOBALS._mCharacter[nPers]._numTexts == 0) || !GLOBALS._mCharacter[nPers]._bInTexts) {
				// Try to run the custom function to close the speech
				GLOBALS._mCharacter[nPers]._curTalk = (GLOBALS._mCharacter[nPers]._curTalk % 10) + GLOBALS._mCharacter[nPers]._curGroup * 10;
				_ctx->h = mpalQueryDoAction(31, GLOBALS._mCharacter[nPers]._item->mpalCode(), GLOBALS._mCharacter[nPers]._curTalk);
				if (_ctx->h != CORO_INVALID_PID_VALUE)
					CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->h, CORO_INFINITE);

				GLOBALS._mCharacter[nPers]._bInTexts = false;
				GLOBALS._mCharacter[nPers]._numTexts = 0;
			}

			GLOBALS._curBackText = NULL;
			delete _ctx->text;
		}
	} else {
		if ((GLOBALS._dwTonyNumTexts == 0 && GLOBALS._bTonyInTexts) || !GLOBALS._bTonyInTexts) {
			CORO_INVOKE_0(GLOBALS._tony->endTalk);
			GLOBALS._dwTonyNumTexts = 0;
			GLOBALS._bTonyInTexts = false;
		}

		delete _ctx->text;
	}

	globalDestroy(_ctx->string);

	CORO_END_CODE;
}

// @@@@ This cannot be skipped!!!!!!!!!!!!!!!!!!!

void startDialog(CORO_PARAM, uint32 nDialog, uint32 nStartGroup, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	uint32 nChoice;
	uint32 *sl;
	uint32 i, num;
	char *string;
	RMDialogChoice dc;
	int sel;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	GLOBALS._curDialog = nDialog;

	// Call MPAL to start the dialog
	mpalQueryDoDialog(nDialog, nStartGroup);

	// Wait until a choice is selected
	mpalQueryDialogWaitForChoice(&_ctx->nChoice);
	while (_ctx->nChoice != (uint32) - 1) {
		// Get the list of options
		_ctx->sl = mpalQueryDialogSelectList(_ctx->nChoice);
		for (_ctx->num = 0; _ctx->sl[_ctx->num] != 0; _ctx->num++)
			;

		// If there is only one option, do it automatically, and wait for the next choice
		if (_ctx->num == 1) {
			mpalQueryDialogSelectionDWORD(_ctx->nChoice, _ctx->sl[0]);
			globalDestroy(_ctx->sl);

			// Wait for the next choice to be made
			mpalQueryDialogWaitForChoice(&_ctx->nChoice);
			continue;
		}

		// Making a choice for dialog
		_ctx->dc.init();
		_ctx->dc.setNumChoices(_ctx->num);

		// Writeall the possible options
		for (_ctx->i = 0; _ctx->i < _ctx->num; _ctx->i++) {
			_ctx->string = mpalQueryDialogPeriod(_ctx->sl[_ctx->i]);
			assert(_ctx->string != NULL);
			_ctx->dc.addChoice(_ctx->string);
			globalDestroy(_ctx->string);
		}

		// Activate the object
		g_vm->getEngine()->linkGraphicTask(&_ctx->dc);
		CORO_INVOKE_0(_ctx->dc.show);

		// Draw the pointer
		GLOBALS._pointer->setSpecialPointer(GLOBALS._pointer->PTR_NONE);
		g_vm->getEngine()->enableMouse();

		while (!(GLOBALS._input->mouseLeftClicked() && ((_ctx->sel = _ctx->dc.getSelection()) != -1))) {
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);
			CORO_INVOKE_1(_ctx->dc.doFrame, GLOBALS._input->mousePos());
		}

		// Hide the pointer
		g_vm->getEngine()->disableMouse();

		CORO_INVOKE_0(_ctx->dc.hide);
		mpalQueryDialogSelectionDWORD(_ctx->nChoice, _ctx->sl[_ctx->sel]);

		// Closes the choice
		_ctx->dc.close();

		globalDestroy(_ctx->sl);

		// Wait for the next choice to be made
		mpalQueryDialogWaitForChoice(&_ctx->nChoice);
	}

	CORO_END_CODE;
}

/*
 *  Sync between idle and mpal
 */

void takeOwnership(CORO_PARAM, uint32 num, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (GLOBALS._mut[num]._ownerPid != (uint32)CoroScheduler.getCurrentPID()) {
		// The mutex is currently owned by a different process.
		// Wait for the event to be signalled, which means the mutex is free.
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, GLOBALS._mut[num]._eventId, CORO_INFINITE);
		GLOBALS._mut[num]._ownerPid = (uint32)CoroScheduler.getCurrentPID();
	}

	GLOBALS._mut[num]._lockCount++;

	CORO_END_CODE;
}

void releaseOwnership(CORO_PARAM, uint32 num, uint32, uint32, uint32) {
	if (!GLOBALS._mut[num]._lockCount) {
		warning("ReleaseOwnership tried to release mutex %d, which isn't held", num);
		return;
	}

	if (GLOBALS._mut[num]._ownerPid != (uint32)CoroScheduler.getCurrentPID()) {
		warning("ReleaseOwnership tried to release mutex %d, which is held by a different process", num);
		return;
	}

	GLOBALS._mut[num]._lockCount--;
	if (!GLOBALS._mut[num]._lockCount) {
		GLOBALS._mut[num]._ownerPid = 0;

		// Signal the event, to wake up processes waiting for the lock.
		CoroScheduler.setEvent(GLOBALS._mut[num]._eventId);
	}
}

/*
 *  Music
 *  -----
 *
 * Fadeout effects supposed:
 *
 *   nFX = 0 - The new music replaces the old one
 *   nFX=1   - The new music interfades with the old one
 *   nFX=2   - The new music takes over in time from the old
 *
 */

void threadFadeInMusic(CORO_PARAM, const void *nMusic) {
	CORO_BEGIN_CONTEXT;
	int i;
	CORO_END_CONTEXT(_ctx);

	int nChannel = *(const int *)nMusic;

	CORO_BEGIN_CODE(_ctx);

	debugC(DEBUG_INTERMEDIATE, kTonyDebugSound, "Start FadeIn Music");

	for (_ctx->i = 0; _ctx->i < 16; _ctx->i++) {
		g_vm->setMusicVolume(nChannel, _ctx->i * 4);

		CORO_INVOKE_1(CoroScheduler.sleep, 100);
	}
	g_vm->setMusicVolume(nChannel, 64);

	debugC(DEBUG_INTERMEDIATE, kTonyDebugSound, "End FadeIn Music");

	CORO_KILL_SELF();

	CORO_END_CODE;
}

void threadFadeOutMusic(CORO_PARAM, const void *nMusic) {
	CORO_BEGIN_CONTEXT;
	int i;
	int startVolume;
	CORO_END_CONTEXT(_ctx);

	int nChannel = *(const int *)nMusic;

	CORO_BEGIN_CODE(_ctx);

	_ctx->startVolume = g_vm->getMusicVolume(nChannel);

	for (_ctx->i = 16; _ctx->i > 0 && !GLOBALS._bFadeOutStop; _ctx->i--) {
		if (_ctx->i * 4 < _ctx->startVolume)
			g_vm->setMusicVolume(nChannel, _ctx->i * 4);

		CORO_INVOKE_1(CoroScheduler.sleep, 100);
	}

	if (!GLOBALS._bFadeOutStop)
		g_vm->setMusicVolume(nChannel, 0);

	// If a jingle is played, stop it
	if (nChannel == 2)
		g_vm->stopMusic(2);

	CORO_KILL_SELF();

	CORO_END_CODE;
}

void fadeInSoundEffect(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CoroScheduler.createProcess(threadFadeInMusic, &GLOBALS._curSoundEffect, sizeof(int));
}

void fadeOutSoundEffect(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bFadeOutStop = false;
	CoroScheduler.createProcess(threadFadeOutMusic, &GLOBALS._curSoundEffect, sizeof(int));
}

void fadeOutJingle(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bFadeOutStop = false;
	int channel = 2;
	CoroScheduler.createProcess(threadFadeOutMusic, &channel, sizeof(int));
}

void fadeInJingle(CORO_PARAM, uint32, uint32, uint32, uint32) {
	int channel = 2;
	CoroScheduler.createProcess(threadFadeInMusic, &channel, sizeof(int));
}

void stopSoundEffect(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->stopMusic(GLOBALS._curSoundEffect);
}

void stopJingle(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->stopMusic(2);
}

void muteSoundEffect(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->setMusicVolume(GLOBALS._curSoundEffect, 0);
}

void demuteSoundEffect(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bFadeOutStop = true;
	g_vm->setMusicVolume(GLOBALS._curSoundEffect, 64);
}

void muteJingle(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->setMusicVolume(2, 0);
}

void demuteJingle(CORO_PARAM, uint32, uint32, uint32, uint32) {
	g_vm->setMusicVolume(2, 64);
}

void custPlayMusic(uint32 nChannel, const char *mFN, uint32 nFX, bool bLoop, int nSync = 0) {
	if (nSync == 0)
		nSync = 2000;
	debugC(DEBUG_INTERMEDIATE, kTonyDebugMusic, "Start CustPlayMusic");
	g_vm->playMusic(nChannel, mFN, nFX, bLoop, nSync);
	debugC(DEBUG_INTERMEDIATE, kTonyDebugMusic, "End CustPlayMusic");
}

void playSoundEffect(CORO_PARAM, uint32 nMusic, uint32 nFX, uint32 bNoLoop, uint32) {
	if (nFX == 0 || nFX == 1 || nFX == 2) {
		debugC(DEBUG_INTERMEDIATE, kTonyDebugSound, "PlaySoundEffect stop fadeout");
		GLOBALS._bFadeOutStop = true;
	}

	GLOBALS._lastMusic = nMusic;
	custPlayMusic(GLOBALS._curSoundEffect, kMusicFiles[nMusic]._name, nFX, bNoLoop ? false : true, kMusicFiles[nMusic]._sync);
}

void playJingle(CORO_PARAM, uint32 nMusic, uint32 nFX, uint32 bLoop, uint32) {
	custPlayMusic(2, kJingleFileNames[nMusic], nFX, bLoop);
}

void playItemSfx(CORO_PARAM, uint32 nItem, uint32 nSFX, uint32, uint32) {
	if (nItem == 0) {
		GLOBALS._tony->playSfx(nSFX);
	} else {
		RMItem *item = GLOBALS._loc->getItemFromCode(nItem);
		if (item)
			item->playSfx(nSFX);
	}
}

void restoreMusic(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(playSoundEffect, GLOBALS._lastMusic, 0, 0, 0);

	if (GLOBALS._lastTappeto != 0)
		custPlayMusic(4, kAmbianceFile[GLOBALS._lastTappeto], 0, true);

	CORO_END_CODE;
}

void saveMusic(Common::OutSaveFile *f) {
	f->writeByte(GLOBALS._lastMusic);
	f->writeByte(GLOBALS._lastTappeto);
}

void loadMusic(Common::InSaveFile *f) {
	GLOBALS._lastMusic = f->readByte();
	GLOBALS._lastTappeto = f->readByte();
}

void jingleFadeStart(CORO_PARAM, uint32 nJingle, uint32 bLoop, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(fadeOutSoundEffect, 0, 0, 0, 0);
	CORO_INVOKE_4(muteJingle, 0, 0, 0, 0);
	CORO_INVOKE_4(playJingle, nJingle, 0, bLoop, 0);
	CORO_INVOKE_4(fadeInJingle, 0, 0, 0, 0);

	CORO_END_CODE;
}

void jingleFadeEnd(CORO_PARAM, uint32 nJingle, uint32 bLoop, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_4(fadeOutJingle, 0, 0, 0, 0);
	CORO_INVOKE_4(fadeInSoundEffect, 0, 0, 0, 0);

	CORO_END_CODE;
}

void mustSkipIdleStart(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bSkipIdle = true;
	CoroScheduler.setEvent(GLOBALS._hSkipIdle);
}

void mustSkipIdleEnd(CORO_PARAM, uint32, uint32, uint32, uint32) {
	GLOBALS._bSkipIdle = false;
	CoroScheduler.resetEvent(GLOBALS._hSkipIdle);
}

void patIrqFreeze(CORO_PARAM, uint32 bStatus, uint32, uint32, uint32) {
	// Unused in ScummVM.
}

void openInitLoadMenu(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_0(g_vm->openInitLoadMenu);

	CORO_END_CODE;
}

void openInitOptions(CORO_PARAM, uint32, uint32, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_0(g_vm->openInitOptions);

	CORO_END_CODE;
}

void doCredits(CORO_PARAM, uint32 nMsg, uint32 dwTime, uint32, uint32) {
	CORO_BEGIN_CONTEXT;
	RMMessage *msg;
	RMTextDialog *text;
	uint32 hDisable;
	int i;
	uint32 startTime;

	~CoroContextTag() {
		delete msg;
		delete[] text;
	}

	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->msg = new RMMessage(nMsg);
	_ctx->hDisable = CoroScheduler.createEvent(true, false);

	_ctx->text = new RMTextDialog[_ctx->msg->numPeriods()];

	for (_ctx->i = 0; _ctx->i < _ctx->msg->numPeriods(); _ctx->i++)     {
		_ctx->text[_ctx->i].setInput(GLOBALS._input);

		// Alignment
		if ((*_ctx->msg)[_ctx->i][0] == '@') {
			_ctx->text[_ctx->i].setAlignType(RMText::HCENTER, RMText::VTOP);
			_ctx->text[_ctx->i].writeText(&(*_ctx->msg)[_ctx->i][1], 3);
			_ctx->text[_ctx->i].setPosition(RMPoint(414, 70 + _ctx->i * 26));  // 70
		} else {
			_ctx->text[_ctx->i].setAlignType(RMText::HLEFT, RMText::VTOP);
			_ctx->text[_ctx->i].writeText((*_ctx->msg)[_ctx->i], 3);
			_ctx->text[_ctx->i].setPosition(RMPoint(260, 70 + _ctx->i * 26));
		}

		// Set the position
		_ctx->text[_ctx->i].setAlwaysDisplay();
		_ctx->text[_ctx->i].setForcedTime(dwTime * 1000);
		_ctx->text[_ctx->i].setNoTab();

		// Wait for the end of display
		_ctx->text[_ctx->i].setCustomSkipHandle(_ctx->hDisable);

		// Record the text
		g_vm->getEngine()->linkGraphicTask(&_ctx->text[_ctx->i]);
	}

	_ctx->startTime = g_vm->getTime();

	while (_ctx->startTime + dwTime * 1000 > g_vm->getTime()) {
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);
		if (GLOBALS._input->mouseLeftClicked() || GLOBALS._input->mouseRightClicked())
			break;
		if (g_vm->getEngine()->getInput().getAsyncKeyState(Common::KEYCODE_TAB))
			break;
	}

	CoroScheduler.setEvent(_ctx->hDisable);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);
	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);

	delete[] _ctx->text;
	delete _ctx->msg;
	_ctx->text = NULL;
	_ctx->msg = NULL;

	CORO_END_CODE;
}

BEGIN_CUSTOM_FUNCTION_MAP()

ASSIGN(1,   custLoadLocation)
ASSIGN(2,   mySleep)
ASSIGN(3,   setPointer)
ASSIGN(5,   moveTony)
ASSIGN(6,   faceToMe)
ASSIGN(7,   backToMe)
ASSIGN(8,   leftToMe)
ASSIGN(9,   rightToMe)
ASSIGN(10,  sendTonyMessage)
ASSIGN(11,  changeBoxStatus)
ASSIGN(12,  changeLocation)
ASSIGN(13,  disableTony)
ASSIGN(14,  enableTony)
ASSIGN(15,  waitForPatternEnd)
ASSIGN(16,  setLocStartPosition)
ASSIGN(17,  scrollLocation)
ASSIGN(18,  moveTonyAndWait)
ASSIGN(19,  changeHotspot)
ASSIGN(20,  addInventory)
ASSIGN(21,  removeInventory)
ASSIGN(22,  changeInventoryStatus)
ASSIGN(23,  setTonyPosition)
ASSIGN(24,  sendFullscreenMessage)
ASSIGN(25,  saveTonyPosition)
ASSIGN(26,  restoreTonyPosition)
ASSIGN(27,  disableInput)
ASSIGN(28,  enableInput)
ASSIGN(29,  stopTony)

ASSIGN(30,  tonyTakeUp1)
ASSIGN(31,  tonyTakeMid1)
ASSIGN(32,  tonyTakeDown1)
ASSIGN(33,  tonyTakeUp2)
ASSIGN(34,  tonyTakeMid2)
ASSIGN(35,  tonyTakeDown2)

ASSIGN(72,  tonyPutUp1)
ASSIGN(73,  tonyPutMid1)
ASSIGN(74,  tonyPutDown1)
ASSIGN(75,  tonyPutUp2)
ASSIGN(76,  tonyPutMid2)
ASSIGN(77,  tonyPutDown2)

ASSIGN(36,  tonyOnTheFloor)
ASSIGN(37,  tonyGetUp)
ASSIGN(38,  tonyShepherdess)
ASSIGN(39,  tonyWhistle)

ASSIGN(40,  tonyLaugh)
ASSIGN(41,  tonyHips)
ASSIGN(42,  tonySing)
ASSIGN(43,  tonyIndicate)
ASSIGN(44,  tonyScaredWithHands)
ASSIGN(49,  tonyScaredWithoutHands)
ASSIGN(45,  tonyWithGlasses)
ASSIGN(46,  tonyWithWorm)
ASSIGN(47,  tonyWithHammer)
ASSIGN(48,  tonyWithRope)
ASSIGN(90,  tonyWithRabbitANIM)
ASSIGN(91,  tonyWithRecipeANIM)
ASSIGN(92,  tonyWithCardsANIM)
ASSIGN(93,  tonyWithSnowmanANIM)
ASSIGN(94,  tonyWithSnowmanStart)
ASSIGN(95,  tonyWithSnowmanEnd)
ASSIGN(96,  tonyWithRabbitStart)
ASSIGN(97,  tonyWithRabbitEnd)
ASSIGN(98,  tonyWithRecipeStart)
ASSIGN(99,  tonyWithRecipeEnd)
ASSIGN(100, tonyWithCardsStart)
ASSIGN(101, tonyWithCardsEnd)
ASSIGN(102, tonyWithNotebookStart)
ASSIGN(103, tonyWithNotebookEnd)
ASSIGN(104, tonyWithMegaphoneStart)
ASSIGN(105, tonyWithMegaphoneEnd)
ASSIGN(106, tonyWithBeardStart)
ASSIGN(107, tonyWithBeardEnd)
ASSIGN(108, tonyGiggle)
ASSIGN(109, tonyDisgusted)
ASSIGN(110, tonySarcastic)
ASSIGN(111, tonyMacbeth)
ASSIGN(112, tonySniffLeft)
ASSIGN(113, tonySniffRight)
ASSIGN(114, tonyScaredStart)
ASSIGN(115, tonyScaredEnd)
ASSIGN(116, tonyWithSecretary)

ASSIGN(50,  charSetCode)
ASSIGN(51,  charSetColor)
ASSIGN(52,  charSetTalkPattern)
ASSIGN(53,  charSendMessage)
ASSIGN(54,  charSetStartEndTalkPattern)

ASSIGN(60,  mCharSetCode)
ASSIGN(61,  mCharSetColor)
ASSIGN(62,  mCharSetCurrentGroup)
ASSIGN(63,  mCharSetNumTalksInGroup)
ASSIGN(64,  mCharSetNumTexts)
ASSIGN(65,  mCharSendMessage)
ASSIGN(66,  mCharSetPosition)
ASSIGN(67,  mCharSetAlwaysBack)
ASSIGN(68,  mCharResetCode)

ASSIGN(70,  startDialog)
ASSIGN(71,  sendDialogMessage)

ASSIGN(80,  takeOwnership)
ASSIGN(81,  releaseOwnership)

ASSIGN(86,  playSoundEffect)
ASSIGN(87,  playJingle)
ASSIGN(88,  fadeInSoundEffect)
ASSIGN(89,  fadeOutSoundEffect)
ASSIGN(123, fadeInJingle)
ASSIGN(124, fadeOutJingle)
ASSIGN(125, muteSoundEffect)
ASSIGN(126, demuteSoundEffect)
ASSIGN(127, muteJingle)
ASSIGN(128, demuteJingle)
ASSIGN(84,  stopSoundEffect)
ASSIGN(85,  stopJingle)
ASSIGN(83,  playItemSfx)
ASSIGN(129, jingleFadeStart)
ASSIGN(130, jingleFadeEnd)

ASSIGN(120, shakeScreen)
ASSIGN(121, autoSave)
ASSIGN(122, abortGame)
ASSIGN(131, noBullsEye)
ASSIGN(132, sendFullscreenMsgStart)
ASSIGN(133, sendFullscreenMsgEnd)
ASSIGN(134, custEnableGUI)
ASSIGN(135, custDisableGUI)
ASSIGN(136, clearScreen)
ASSIGN(137, patIrqFreeze)
ASSIGN(138, tonySetPerorate)
ASSIGN(139, openInitLoadMenu)
ASSIGN(140, openInitOptions)
ASSIGN(141, syncScrollLocation)
ASSIGN(142, closeLocation)
ASSIGN(143, setAlwaysDisplay)
ASSIGN(144, doCredits)

ASSIGN(200, mustSkipIdleStart);
ASSIGN(201, mustSkipIdleEnd);

END_CUSTOM_FUNCTION_MAP()

void processKilledCallback(Common::PROCESS *p) {
	for (uint i = 0; i < 10; i++) {
		if (GLOBALS._mut[i]._ownerPid == p->pid) {
			// Handle scripts which don't call ReleaseOwnership, such as
			// the one in loc37's vEnter when Tony is chasing the mouse.
			debug(DEBUG_BASIC, "Force-releasing mutex %d after process died", i);

			GLOBALS._mut[i]._ownerPid = 0;
			GLOBALS._mut[i]._lockCount = 0;
			CoroScheduler.setEvent(GLOBALS._mut[i]._eventId);
		}
	}
}

void setupGlobalVars(RMTony *tony, RMPointer *ptr, RMGameBoxes *box, RMLocation *loc, RMInventory *inv, RMInput *input) {
	GLOBALS._tony = tony;
	GLOBALS._pointer = ptr;
	GLOBALS._boxes = box;
	GLOBALS._loc = loc;
	GLOBALS._inventory = inv;
	GLOBALS._input = input;

	GLOBALS.DisableGUI = mainDisableGUI;
	GLOBALS.EnableGUI = mainEnableGUI;

	GLOBALS._bAlwaysDisplay = false;

	CoroScheduler.setResourceCallback(processKilledCallback);
	for (int i = 0; i < 10; i++)
		GLOBALS._mut[i]._eventId = CoroScheduler.createEvent(false, true);

	for (int i = 0; i < 200; i++)
		GLOBALS._ambiance[i] = 0;

	GLOBALS._ambiance[6] =  AMBIANCE_CRICKETS;
	GLOBALS._ambiance[7] =  AMBIANCE_CRICKETS;
	GLOBALS._ambiance[8] =  AMBIANCE_CRICKETSMUFFLED;
	GLOBALS._ambiance[10] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[12] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[13] = AMBIANCE_CRICKETSMUFFLED;
	GLOBALS._ambiance[15] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[16] = AMBIANCE_CRICKETSWIND;
	GLOBALS._ambiance[18] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[19] = AMBIANCE_CRICKETSWIND;
	GLOBALS._ambiance[20] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[23] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[26] = AMBIANCE_SEAHALFVOLUME;
	GLOBALS._ambiance[27] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[28] = AMBIANCE_CRICKETSWIND;
	GLOBALS._ambiance[31] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[33] = AMBIANCE_SEA;
	GLOBALS._ambiance[35] = AMBIANCE_SEA;
	GLOBALS._ambiance[36] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[37] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[40] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[41] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[42] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[45] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[51] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[52] = AMBIANCE_CRICKETSWIND1;
	GLOBALS._ambiance[53] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[54] = AMBIANCE_CRICKETS;
	GLOBALS._ambiance[57] = AMBIANCE_WIND;
	GLOBALS._ambiance[58] = AMBIANCE_WIND;
	GLOBALS._ambiance[60] = AMBIANCE_WIND;

	// Create an event for the idle skipping
	GLOBALS._hSkipIdle = CoroScheduler.createEvent(true, false);
}

} // end of namespace Tony
