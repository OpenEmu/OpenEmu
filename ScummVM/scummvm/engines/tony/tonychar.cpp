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

#include "tony/mpal/memory.h"
#include "tony/mpal/mpalutils.h"
#include "tony/game.h"
#include "tony/tonychar.h"
#include "tony/tony.h"

namespace Tony {

bool RMTony::_bAction = false;

void RMTony::initStatics() {
	_bAction = false;
}

RMTony::RMTony() {
	_bShow = false;
	_bShowShadow = false;
	_bBodyFront = false;
	_bActionPending = false;
	_actionItem = NULL;
	_action = 0;
	_actionParm = 0;
	_bShepherdess = false;
	_bIsStaticTalk = false;
	_bIsTalking = false;
	_nPatB4Talking = 0;
	_nTalkType = TALK_NORMAL;
	_talkDirection = UP;
	_nTimeLastStep = 0;
	_hActionThread = CORO_INVALID_PID_VALUE;
}

void RMTony::waitEndOfAction(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	uint32 pid = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, pid, CORO_INFINITE);

	_bAction = false;

	CORO_END_CODE;
}

RMGfxSourceBuffer *RMTony::newItemSpriteBuffer(int dimx, int dimy, bool bPreRLE) {
	RMGfxSourceBuffer8RLE *spr;

	assert(_cm == CM_256);
	spr = new RMGfxSourceBuffer8RLEByteAA;
	spr->setAlphaBlendColor(1);
	if (bPreRLE)
		spr->setAlreadyCompressed();
	return spr;
}

void RMTony::init() {
	RMRes tony(0);
	RMRes body(9999);

	// Tony is shown by default
	_bShow = _bShowShadow = true;

	// No action pending
	_bActionPending = false;
	_bAction = false;

	_bShepherdess = false;
	_bIsTalking = false;
	_bIsStaticTalk = false;

	// Opens the buffer
	Common::SeekableReadStream *ds = tony.getReadStream();

	// Reads his details from the stream
	readFromStream(*ds, true);

	// Closes the buffer
	delete ds;

	// Reads Tony's body
	ds = body.getReadStream();
	_body.readFromStream(*ds, true);
	delete ds;
	_body.setPattern(0);

	_nTimeLastStep = g_vm->getTime();
}


void RMTony::close() {
	// Deallocation of missing item
	//_shadow.destroy();
}

void RMTony::doFrame(CORO_PARAM, RMGfxTargetBuffer *bigBuf, int curLoc) {
	CORO_BEGIN_CONTEXT;
	int time;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (!_nInList && _bShow)
		bigBuf->addPrim(new RMGfxPrimitive(this));

	setSpeed(GLOBALS._nCfgTonySpeed);

	// Runs the normal character movement
	_ctx->time = g_vm->getTime();

	do {
		_nTimeLastStep += (1000 / 40);
		CORO_INVOKE_2(RMCharacter::doFrame, bigBuf, curLoc);
	} while (_ctx->time > _nTimeLastStep + (1000 / 40));

	// Check if we are at the end of a path
	if (endOfPath() && _bActionPending) {
		// Must perform the action on which we clicked
		_bActionPending = false;
	}

	if (_bIsTalking || _bIsStaticTalk)
		_body.doFrame(bigBuf, false);

	CORO_END_CODE;
}

void RMTony::show() {
	_bShow = true;
	_bShowShadow = true;
}

void RMTony::hide(bool bShowShadow) {
	_bShow = false;
	_bShowShadow = bShowShadow;
}

void RMTony::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Call the Draw() of the parent class if Tony is visible
	if (_bShow && _bDrawNow) {
		if (_bBodyFront) {
			prim->getDst().setEmpty();
			prim->getDst().offset(-44, -134);
			if (_bShepherdess)
				prim->getDst().offset(1, 4);
			CORO_INVOKE_2(RMCharacter::draw, bigBuf, prim);
		}

		if (_bIsTalking || _bIsStaticTalk) {
			// Offest direction from scrolling
			prim->getDst().setEmpty();
			prim->getDst().offset(-_curScroll);
			prim->getDst().offset(_pos);
			prim->getDst().offset(-44, -134);
			prim->getDst() += _nBodyOffset;
			CORO_INVOKE_2(_body.draw, bigBuf, prim);
		}

		if (!_bBodyFront) {
			prim->getDst().setEmpty();
			prim->getDst().offset(-44, -134);
			if (_bShepherdess)
				prim->getDst().offset(0, 3);
			CORO_INVOKE_2(RMCharacter::draw, bigBuf, prim);
		}
	}

	CORO_END_CODE;
}

void RMTony::moveAndDoAction(CORO_PARAM, RMPoint dst, RMItem *item, int nAction, int nActionParm) {
	CORO_BEGIN_CONTEXT;
	bool result;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Makes normal movement, but remember if you must then perform an action
	if (item == NULL) {
		_bActionPending = false;
		_actionItem = NULL;
	} else {
		_actionItem = item;
		_action = nAction;
		_actionParm = nActionParm;
		_bActionPending = true;
	}

	CORO_INVOKE_2(RMCharacter::move, dst, &_ctx->result);
	if (!_ctx->result) {
		_bActionPending = false;
		_actionItem = NULL;
	}

	CORO_END_CODE;
}

void RMTony::executeAction(int nAction, int nActionItem, int nParm) {
	uint32 pid;

	if (nAction == TA_COMBINE) {
		pid = mpalQueryDoAction(TA_COMBINE, nParm, nActionItem);

		// If you failed the combine, we have RECEIVECOMBINE as a fallback
		if (pid == CORO_INVALID_PID_VALUE) {
			pid = mpalQueryDoAction(TA_RECEIVECOMBINE, nActionItem, nParm);

			// If you failed with that, go with the generic
			// @@@ CombineGive!
			if (pid == CORO_INVALID_PID_VALUE) {
				pid = mpalQueryDoAction(TA_COMBINE, nParm, 0);

				if (pid == CORO_INVALID_PID_VALUE) {
					pid = mpalQueryDoAction(TA_RECEIVECOMBINE, nActionItem, 0);
				}
			}
		}
	} else {
		// Perform the action
		pid = mpalQueryDoAction(nAction, nActionItem, 0);
	}

	if (pid != CORO_INVALID_PID_VALUE) {
		_bAction = true;
		CoroScheduler.createProcess(waitEndOfAction, &pid, sizeof(uint32));
		_hActionThread = pid;
	} else if (nAction != TA_GOTO) {
		if (nAction == TA_TALK) {
			pid = mpalQueryDoAction(6, 1, 0);
			_bAction = true;
			CoroScheduler.createProcess(waitEndOfAction, &pid, sizeof(uint32));
			_hActionThread = pid;
		} else if (nAction == TA_PERORATE) {
			pid = mpalQueryDoAction(7, 1, 0);
			_bAction = true;
			CoroScheduler.createProcess(waitEndOfAction, &pid, sizeof(uint32));
			_hActionThread = pid;
		} else {
			pid = mpalQueryDoAction(5, 1, 0);
			_bAction = true;
			CoroScheduler.createProcess(waitEndOfAction, &pid, sizeof(uint32));
			_hActionThread = pid;
		}
	}
}

void RMTony::stopNoAction(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_bAction)
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _hActionThread, CORO_INFINITE);

	_bActionPending = false;
	_actionItem = NULL;
	CORO_INVOKE_0(stop);

	CORO_END_CODE;
}

void RMTony::stop(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	uint32 pid;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_actionItem != NULL) {
		// Call MPAL to choose the direction
		_ctx->pid = mpalQueryDoAction(21, _actionItem->mpalCode(), 0);

		if (_ctx->pid == CORO_INVALID_PID_VALUE)
			CORO_INVOKE_0(RMCharacter::stop);
		else {
			_bNeedToStop = false;    // If we make the OnWhichDirection, we don't need at least after the Stop().
			_bMoving = false;
			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->pid, CORO_INFINITE); // @@@ Put an assert after 10 seconds
		}
	} else {
		CORO_INVOKE_0(RMCharacter::stop);
	}

	if (!_bActionPending)
		return;

	_bActionPending = false;

	executeAction(_action, _actionItem->mpalCode(), _actionParm);

	_actionItem = NULL;

	CORO_END_CODE;
}

int RMTony::getCurPattern() {
	int nPatt = RMCharacter::getCurPattern();

	if (!_bShepherdess)
		return nPatt;

	switch (nPatt) {
	case PAT_PAST_STANDUP:
		return PAT_STANDUP;
	case PAT_PAST_STANDDOWN:
		return PAT_STANDDOWN;
	case PAT_PAST_STANDLEFT:
		return PAT_STANDLEFT;
	case PAT_PAST_STANDRIGHT:
		return PAT_STANDRIGHT;
	case PAT_PAST_WALKUP:
		return PAT_WALKUP;
	case PAT_PAST_WALKDOWN:
		return PAT_WALKDOWN;
	case PAT_PAST_WALKLEFT:
		return PAT_WALKLEFT;
	case PAT_PAST_WALKRIGHT:
		return PAT_WALKRIGHT;
	}

	return nPatt;
}

void RMTony::setPattern(int nPatt, bool bPlayP0) {
	if (_bShepherdess) {
		switch (nPatt) {
		case PAT_STANDUP:
			nPatt = PAT_PAST_STANDUP;
			break;
		case PAT_STANDDOWN:
			nPatt = PAT_PAST_STANDDOWN;
			break;
		case PAT_STANDLEFT:
			nPatt = PAT_PAST_STANDLEFT;
			break;
		case PAT_STANDRIGHT:
			nPatt = PAT_PAST_STANDRIGHT;
			break;
		case PAT_WALKUP:
			nPatt = PAT_PAST_WALKUP;
			break;
		case PAT_WALKDOWN:
			nPatt = PAT_PAST_WALKDOWN;
			break;
		case PAT_WALKLEFT:
			nPatt = PAT_PAST_WALKLEFT;
			break;
		case PAT_WALKRIGHT:
			nPatt = PAT_PAST_WALKRIGHT;
			break;
		}
	}

	RMCharacter::setPattern(nPatt, bPlayP0);
}

void RMTony::take(int nWhere, int nPart) {
	if (nPart == 0) {
		switch (getCurPattern()) {
		case PAT_STANDDOWN:
			assert(0);  // Not while you're doing a StandDown
			break;

		case PAT_STANDUP:
			switch (nWhere) {
			case 0:
				setPattern(PAT_TAKEUP_UP1);
				break;
			case 1:
				setPattern(PAT_TAKEUP_MID1);
				break;
			case 2:
				setPattern(PAT_TAKEUP_DOWN1);
				break;
			}
			break;

		case PAT_STANDRIGHT:
			switch (nWhere) {
			case 0:
				setPattern(PAT_TAKERIGHT_UP1);
				break;
			case 1:
				setPattern(PAT_TAKERIGHT_MID1);
				break;
			case 2:
				setPattern(PAT_TAKERIGHT_DOWN1);
				break;
			}
			break;

		case PAT_STANDLEFT:
			switch (nWhere) {
			case 0:
				setPattern(PAT_TAKELEFT_UP1);
				break;
			case 1:
				setPattern(PAT_TAKELEFT_MID1);
				break;
			case 2:
				setPattern(PAT_TAKELEFT_DOWN1);
				break;
			}
			break;
		}
	} else if (nPart == 1) {
		setPattern(getCurPattern() + 1);
	} else if (nPart == 2) {
		switch (getCurPattern()) {
		case PAT_TAKEUP_UP2:
		case PAT_TAKEUP_MID2:
		case PAT_TAKEUP_DOWN2:
			setPattern(PAT_STANDUP);
			break;

		case PAT_TAKELEFT_UP2:
		case PAT_TAKELEFT_MID2:
		case PAT_TAKELEFT_DOWN2:
			setPattern(PAT_STANDLEFT);
			break;

		case PAT_TAKERIGHT_UP2:
		case PAT_TAKERIGHT_MID2:
		case PAT_TAKERIGHT_DOWN2:
			setPattern(PAT_STANDRIGHT);
			break;
		}
	}
}

void RMTony::put(int nWhere, int nPart) {
	if (nPart == 0) {
		switch (getCurPattern()) {
		case PAT_STANDDOWN:
			break;

		case PAT_STANDUP:
			switch (nWhere) {
			case 0:
				setPattern(PAT_PUTUP_UP1);
				break;
			case 1:
				setPattern(PAT_PUTUP_MID1);
				break;
			case 2:
				setPattern(PAT_PUTUP_DOWN1);
				break;
			}
			break;

		case PAT_STANDRIGHT:
			switch (nWhere) {
			case 0:
				setPattern(PAT_PUTRIGHT_UP1);
				break;
			case 1:
				setPattern(PAT_PUTRIGHT_MID1);
				break;
			case 2:
				setPattern(PAT_PUTRIGHT_DOWN1);
				break;
			}
			break;

		case PAT_STANDLEFT:
			switch (nWhere) {
			case 0:
				setPattern(PAT_PUTLEFT_UP1);
				break;
			case 1:
				setPattern(PAT_PUTLEFT_MID1);
				break;
			case 2:
				setPattern(PAT_PUTLEFT_DOWN1);
				break;
			}
			break;
		}
	} else if (nPart == 1) {
		setPattern(getCurPattern() + 1);
	} else if (nPart == 2) {
		switch (getCurPattern()) {
		case PAT_PUTUP_UP2:
		case PAT_PUTUP_MID2:
		case PAT_PUTUP_DOWN2:
			setPattern(PAT_STANDUP);
			break;

		case PAT_PUTLEFT_UP2:
		case PAT_PUTLEFT_MID2:
		case PAT_PUTLEFT_DOWN2:
			setPattern(PAT_STANDLEFT);
			break;

		case PAT_PUTRIGHT_UP2:
		case PAT_PUTRIGHT_MID2:
		case PAT_PUTRIGHT_DOWN2:
			setPattern(PAT_STANDRIGHT);
			break;
		}
	}
}

bool RMTony::startTalkCalculate(CharacterTalkType nTalkType, int &headStartPat, int &bodyStartPat,
                                int &headLoopPat, int &bodyLoopPat) {
	assert(!_bIsTalking);

	_bIsTalking = true;
	_nPatB4Talking = getCurPattern();
	_nTalkType = nTalkType;

	// Set the direction of speech ONLY if we are not in a static animation (since it would have already been done)
	if (!_bIsStaticTalk) {
		switch (_nPatB4Talking) {
		case PAT_STANDDOWN:
			_talkDirection = DOWN;
			break;

		case PAT_TAKELEFT_UP2:
		case PAT_TAKELEFT_MID2:
		case PAT_TAKELEFT_DOWN2:
		case PAT_GETUPLEFT:
		case PAT_STANDLEFT:
			_talkDirection = LEFT;
			break;

		case PAT_TAKERIGHT_UP2:
		case PAT_TAKERIGHT_MID2:
		case PAT_TAKERIGHT_DOWN2:
		case PAT_GETUPRIGHT:
		case PAT_STANDRIGHT:
			_talkDirection = RIGHT;
			break;

		case PAT_TAKEUP_UP2:
		case PAT_TAKEUP_MID2:
		case PAT_TAKEUP_DOWN2:
		case PAT_STANDUP:
			_talkDirection = UP;
			break;
		}

		// Puts the body in front by default
		_bBodyFront = true;
	}

	if (_bShepherdess) {
		// Talking whilst a shepherdess
		switch (_talkDirection) {
		case UP:
			setPattern(PAT_PAST_TALKUP);
			break;

		case DOWN:
			setPattern(PAT_PAST_TALKDOWN);
			break;

		case LEFT:
			setPattern(PAT_PAST_TALKLEFT);
			break;

		case RIGHT:
			setPattern(PAT_PAST_TALKRIGHT);
			break;
		}
		return false;
	}

	headStartPat = bodyStartPat = 0;
	bodyLoopPat = 0;

	switch (nTalkType) {
	case TALK_NORMAL:
		_bBodyFront = false;
		headStartPat = 0;
		bodyStartPat = 0;

		switch (_talkDirection) {
		case DOWN:
			headLoopPat = PAT_TALK_DOWN;
			bodyLoopPat = BPAT_STANDDOWN;
			_nBodyOffset.set(4, 53);
			break;

		case LEFT:
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			_nBodyOffset.set(6, 56);
			break;

		case RIGHT:
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			_nBodyOffset.set(6, 56);
			break;

		case UP:
			headLoopPat = PAT_TALK_UP;
			bodyLoopPat = BPAT_STANDUP;
			_nBodyOffset.set(6, 53);
			break;
		}
		break;

	case TALK_HIPS:
		_bBodyFront = false;
		switch (_talkDirection) {
		case UP:
			_nBodyOffset.set(2, 42);
			headStartPat = PAT_HEAD_UP;
			bodyStartPat = BPAT_HIPSUP_START;
			headLoopPat = PAT_TALK_UP;
			bodyLoopPat = BPAT_HIPSUP_LOOP;
			break;

		case DOWN:
			_nBodyOffset.set(2, 48);
			headStartPat = PAT_HEAD_DOWN;
			bodyStartPat = BPAT_HIPSDOWN_START;
			headLoopPat = PAT_TALK_DOWN;
			bodyLoopPat = BPAT_HIPSDOWN_LOOP;
			break;

		case LEFT:
			_nBodyOffset.set(-3, 53);
			headStartPat = PAT_HEAD_LEFT;
			bodyStartPat = BPAT_HIPSLEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_HIPSLEFT_LOOP;
			break;

		case RIGHT:
			_nBodyOffset.set(2, 53);
			headStartPat = PAT_HEAD_RIGHT;
			bodyStartPat = BPAT_HIPSRIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_HIPSRIGHT_LOOP;
			break;
		}
		break;

	case TALK_SING:
		_nBodyOffset.set(-10, 25);
		headStartPat = PAT_HEAD_LEFT;
		bodyStartPat = BPAT_SINGLEFT_START;
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_SINGLEFT_LOOP;
		break;

	case TALK_LAUGH:
		_bBodyFront = false;
		switch (_talkDirection) {
		case UP:
		case DOWN:
		case LEFT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_LAUGHLEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_LAUGHLEFT_LOOP;
			bodyLoopPat = BPAT_LAUGHLEFT;
			break;

		case RIGHT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_LAUGHRIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_LAUGHRIGHT_LOOP;
			bodyLoopPat = BPAT_LAUGHRIGHT;
			break;
		}
		break;

	case TALK_LAUGH2:
		_bBodyFront = false;
		switch (_talkDirection) {
		case UP:
		case DOWN:
		case LEFT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_LAUGHLEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_LAUGHLEFT_LOOP;
			break;

		case RIGHT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_LAUGHRIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_LAUGHRIGHT_LOOP;
			bodyLoopPat = BPAT_LAUGHRIGHT;
			break;
		}
		break;

	case TALK_INDICATE:
		switch (_talkDirection) {
		case UP:
		case DOWN:
		case LEFT:
			_nBodyOffset.set(-4, 40);
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_INDICATELEFT;
			break;

		case RIGHT:
			_nBodyOffset.set(5, 40);
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_INDICATERIGHT;
			break;
		}
		break;

	case TALK_SCARED:
		switch (_talkDirection) {
		case UP:
			_nBodyOffset.set(-4, -11);
			headStartPat = PAT_HEAD_UP;
			bodyStartPat = BPAT_SCAREDUP_START;
			headLoopPat = PAT_TALK_UP;
			bodyLoopPat = BPAT_SCAREDUP_LOOP;
			break;

		case DOWN:
			_nBodyOffset.set(-5, 45);
			headStartPat = PAT_SCAREDDOWN_START;
			bodyStartPat = BPAT_SCAREDDOWN_START;
			headLoopPat = PAT_SCAREDDOWN_LOOP;
			bodyLoopPat = BPAT_SCAREDDOWN_LOOP;
			break;

		case RIGHT:
			_nBodyOffset.set(-4, 41);
			headStartPat = PAT_SCAREDRIGHT_START;
			bodyStartPat = BPAT_SCAREDRIGHT_START;
			headLoopPat = PAT_SCAREDRIGHT_LOOP;
			bodyLoopPat = BPAT_SCAREDRIGHT_LOOP;
			break;

		case LEFT:
			_nBodyOffset.set(-10, 41);
			headStartPat = PAT_SCAREDLEFT_START;
			bodyStartPat = BPAT_SCAREDLEFT_START;
			headLoopPat = PAT_SCAREDLEFT_LOOP;
			bodyLoopPat = BPAT_SCAREDLEFT_LOOP;
			break;
		}
		break;

	case TALK_SCARED2:
		_bBodyFront = false;
		switch (_talkDirection) {
		case UP:
			bodyStartPat = BPAT_STANDUP;
			bodyLoopPat = BPAT_STANDUP;
			_nBodyOffset.set(6, 53);

			headStartPat = PAT_HEAD_UP;
			headLoopPat = PAT_TALK_UP;
			break;

		case DOWN:
			bodyStartPat = BPAT_STANDDOWN;
			bodyLoopPat = BPAT_STANDDOWN;
			_nBodyOffset.set(4, 53);

			headStartPat = PAT_SCAREDDOWN_START;
			headLoopPat = PAT_SCAREDDOWN_LOOP;
			break;

		case RIGHT:
			bodyStartPat = BPAT_STANDRIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			_nBodyOffset.set(6, 56);

			headStartPat = PAT_SCAREDRIGHT_START;
			headLoopPat = PAT_SCAREDRIGHT_LOOP;
			break;

		case LEFT:
			bodyStartPat = BPAT_STANDLEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			_nBodyOffset.set(6, 56);

			headStartPat = PAT_SCAREDLEFT_START;
			headLoopPat = PAT_SCAREDLEFT_LOOP;
			break;
		}
		break;

	case TALK_WITHGLASSES:
		_nBodyOffset.set(4, 53);
		headLoopPat = PAT_TALK_DOWN;
		bodyLoopPat = BPAT_GLASS;
		break;
	case TALK_WITHWORM:
		_nBodyOffset.set(9, 56);
		headLoopPat = PAT_TALK_RIGHT;
		bodyLoopPat = BPAT_WORM;
		break;
	case TALK_WITHHAMMER:
		_nBodyOffset.set(6, 56);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_HAMMER;
		break;
	case TALK_WITHROPE:
		_nBodyOffset.set(-3, 38);
		headLoopPat = PAT_TALK_RIGHT;
		bodyLoopPat = BPAT_ROPE;
		break;
	case TALK_WITHSECRETARY:
		_nBodyOffset.set(-17, 12);
		headLoopPat = PAT_TALK_RIGHT;
		bodyLoopPat = BPAT_WITHSECRETARY;
		break;

	case TALK_WITHRABBIT:
		switch (_talkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(-21, -5);
			bodyStartPat = BPAT_WITHRABBITLEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_WITHRABBITLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-4, -5);
			bodyStartPat = BPAT_WITHRABBITRIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_WITHRABBITRIGHT_LOOP;
			break;
		}
		break;

	case TALK_WITHRECIPE:
		switch (_talkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(-61, -7);
			bodyStartPat = BPAT_WITHRECIPELEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_WITHRECIPELEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-5, -7);
			bodyStartPat = BPAT_WITHRECIPERIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_WITHRECIPERIGHT_LOOP;
			break;
		}
		break;

	case TALK_WITHCARDS:
		switch (_talkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(-34, -2);
			bodyStartPat = BPAT_WITHCARDSLEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_WITHCARDSLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-4, -2);
			bodyStartPat = BPAT_WITHCARDSRIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_WITHCARDSRIGHT_LOOP;
			break;
		}
		break;

	case TALK_WITHSNOWMAN:
		switch (_talkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(-35, 2);
			bodyStartPat = BPAT_WITHSNOWMANLEFT_START;
			headLoopPat = PAT_TALK_LEFT;
			bodyLoopPat = BPAT_WITHSNOWMANLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-14, 2);
			bodyStartPat = BPAT_WITHSNOWMANRIGHT_START;
			headLoopPat = PAT_TALK_RIGHT;
			bodyLoopPat = BPAT_WITHSNOWMANRIGHT_LOOP;
			break;
		}
		break;

	case TALK_WITHSNOWMANSTATIC:
	case TALK_WITHRECIPESTATIC:
	case TALK_WITHRABBITSTATIC:
	case TALK_WITHCARDSSTATIC:
	case TALK_WITH_NOTEBOOK:
	case TALK_WITHMEGAPHONESTATIC:
		switch (_talkDirection) {
		case LEFT:
		case UP:
			headLoopPat = PAT_TALK_LEFT;
			break;

		case DOWN:
		case RIGHT:
			headLoopPat = PAT_TALK_RIGHT;
			break;
		}
		break;

		// The beard is the only case in which the head is animated separately while the body is the standard
	case TALK_WITHBEARDSTATIC:
		switch (_talkDirection) {
		case LEFT:
		case UP:
			headLoopPat = PAT_TALKBEARD_LEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			_nBodyOffset.set(6, 56);
			break;

		case DOWN:
		case RIGHT:
			headLoopPat = PAT_TALKBEARD_RIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			_nBodyOffset.set(6, 56);
			break;
		}
		break;

	case TALK_DISGUSTED:
		switch (_talkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_DISGUSTEDLEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_DISGUSTEDLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_DISGUSTEDRIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_DISGUSTEDRIGHT_LOOP;
			break;
		}
		break;

	case TALK_SARCASTIC:
		switch (_talkDirection) {
		case LEFT:
		case UP:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_SARCASTICLEFT_START;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_SARCASTICLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(6, 56);
			headStartPat = PAT_SARCASTICRIGHT_START;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_SARCASTICRIGHT_LOOP;
			break;
		}
		break;

	case TALK_MACBETH1:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH1;
		break;
	case TALK_MACBETH2:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH2;
		break;
	case TALK_MACBETH3:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH3;
		break;
	case TALK_MACBETH4:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH4;
		break;
	case TALK_MACBETH5:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH5;
		break;
	case TALK_MACBETH6:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH6;
		break;
	case TALK_MACBETH7:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH7;
		break;
	case TALK_MACBETH8:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH8;
		break;
	case TALK_MACBETH9:
		_nBodyOffset.set(-33, -1);
		headLoopPat = PAT_TALK_LEFT;
		bodyLoopPat = BPAT_MACBETH9;
		break;

	case TALK_SCAREDSTATIC:
		_bBodyFront = false;
		switch (_talkDirection) {
		case DOWN:
			bodyStartPat = BPAT_STANDDOWN;
			bodyLoopPat = BPAT_STANDDOWN;
			_nBodyOffset.set(4, 53);

			headStartPat = PAT_SCAREDDOWN_STAND;
			headLoopPat = PAT_SCAREDDOWN_LOOP;
			break;

		case RIGHT:
			bodyStartPat = BPAT_STANDRIGHT;
			bodyLoopPat = BPAT_STANDRIGHT;
			_nBodyOffset.set(6, 56);

			headStartPat = PAT_SCAREDRIGHT_STAND;
			headLoopPat = PAT_SCAREDRIGHT_LOOP;
			break;

		case LEFT:
			bodyStartPat = BPAT_STANDLEFT;
			bodyLoopPat = BPAT_STANDLEFT;
			_nBodyOffset.set(6, 56);

			headStartPat = PAT_SCAREDLEFT_STAND;
			headLoopPat = PAT_SCAREDLEFT_LOOP;
			break;

		default:
			break;
		}
		break;
	}

	return true;
}

void RMTony::startTalk(CORO_PARAM, CharacterTalkType nTalkType) {
	CORO_BEGIN_CONTEXT;
	int headStartPat, bodyStartPat;
	int headLoopPat, bodyLoopPat;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->headStartPat = _ctx->bodyStartPat = 0;
	_ctx->headLoopPat = _ctx->bodyLoopPat = 0;

	if (!startTalkCalculate(nTalkType, _ctx->headStartPat, _ctx->bodyStartPat,
	                        _ctx->headLoopPat, _ctx->bodyLoopPat))
		return;

	// Perform the set pattern
	if (_ctx->headStartPat != 0 || _ctx->bodyStartPat != 0) {
		setPattern(_ctx->headStartPat);
		_body.setPattern(_ctx->bodyStartPat);

		if (_ctx->bodyStartPat != 0)
			CORO_INVOKE_0(_body.waitForEndPattern);
		if (_ctx->headStartPat != 0)
			CORO_INVOKE_0(waitForEndPattern);
	}

	setPattern(_ctx->headLoopPat);
	if (_ctx->bodyLoopPat)
		_body.setPattern(_ctx->bodyLoopPat);

	CORO_END_CODE;
}

bool RMTony::endTalkCalculate(int &headStandPat, int &headEndPat, int &bodyEndPat, int &finalPat, bool &bStatic) {
	bodyEndPat = 0;
	headEndPat = 0;

	switch (_talkDirection) {
	case UP:
		finalPat = PAT_STANDUP;
		headStandPat = PAT_HEAD_UP;
		break;

	case DOWN:
		finalPat = PAT_STANDDOWN;
		headStandPat = PAT_HEAD_DOWN;
		break;

	case LEFT:
		finalPat = PAT_STANDLEFT;
		headStandPat = PAT_HEAD_LEFT;
		break;

	case RIGHT:
		finalPat = PAT_STANDRIGHT;
		headStandPat = PAT_HEAD_RIGHT;
		break;
	}

	if (_bShepherdess) {
		setPattern(finalPat);
		_bIsTalking = false;
		return false;
	}

	bStatic = false;
	switch (_nTalkType) {
	case TALK_NORMAL:
		bodyEndPat = 0;
		break;

	case TALK_HIPS:
		switch (_talkDirection) {
		case UP:
			bodyEndPat = BPAT_HIPSUP_END;
			break;

		case DOWN:
			bodyEndPat = BPAT_HIPSDOWN_END;
			break;

		case LEFT:
			bodyEndPat = BPAT_HIPSLEFT_END;
			break;

		case RIGHT:
			bodyEndPat = BPAT_HIPSRIGHT_END;
			break;
		}
		break;

	case TALK_SING:
		bodyEndPat = BPAT_SINGLEFT_END;
		break;

	case TALK_LAUGH:
	case TALK_LAUGH2:
		if (_talkDirection == LEFT)
			headEndPat = PAT_LAUGHLEFT_END;
		else if (_talkDirection == RIGHT)
			headEndPat = PAT_LAUGHRIGHT_END;

		bodyEndPat = 0;
		break;

	case TALK_DISGUSTED:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			headEndPat = PAT_DISGUSTEDLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			headEndPat = PAT_DISGUSTEDRIGHT_END;
			break;
		}

		bodyEndPat = 0;
		break;

	case TALK_SARCASTIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			headEndPat = PAT_SARCASTICLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			headEndPat = PAT_SARCASTICRIGHT_END;
			break;
		}

		bodyEndPat = 0;
		break;

	case TALK_INDICATE:
		break;

	case TALK_SCARED:
		switch (_talkDirection) {
		case UP:
			bodyEndPat = BPAT_SCAREDUP_END;
			break;

		case DOWN:
			headEndPat = PAT_SCAREDDOWN_END;
			bodyEndPat = BPAT_SCAREDDOWN_END;
			break;

		case RIGHT:
			headEndPat = PAT_SCAREDRIGHT_END;
			bodyEndPat = BPAT_SCAREDRIGHT_END;
			break;

		case LEFT:
			headEndPat = PAT_SCAREDLEFT_END;
			bodyEndPat = BPAT_SCAREDLEFT_END;
			break;
		}
		break;

	case TALK_SCARED2:
		switch (_talkDirection) {
		case UP:
			bodyEndPat = 0;
			break;

		case DOWN:
			headEndPat = PAT_SCAREDDOWN_END;
			bodyEndPat = 0;
			break;

		case RIGHT:
			headEndPat = PAT_SCAREDRIGHT_END;
			bodyEndPat = 0;
			break;

		case LEFT:
			headEndPat = PAT_SCAREDLEFT_END;
			bodyEndPat = 0;
			break;
		}
		break;

	case TALK_WITHRABBIT:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_WITHRABBITLEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_WITHRABBITRIGHT_END;
			break;
		}
		break;

	case TALK_WITHRECIPE:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_WITHRECIPELEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_WITHRECIPERIGHT_END;
			break;
		}
		break;

	case TALK_WITHCARDS:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_WITHCARDSLEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_WITHCARDSRIGHT_END;
			break;
		}
		break;

	case TALK_WITHSNOWMAN:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			finalPat = PAT_STANDLEFT;
			bodyEndPat = BPAT_WITHSNOWMANLEFT_END;
			break;

		case RIGHT:
		case DOWN:
			finalPat = PAT_STANDRIGHT;
			bodyEndPat = BPAT_WITHSNOWMANRIGHT_END;
			break;
		}
		break;

	case TALK_WITHWORM:
		finalPat = PAT_WITHWORM;
		break;
	case TALK_WITHROPE:
		finalPat = PAT_WITHROPE;
		break;
	case TALK_WITHSECRETARY:
		finalPat = PAT_WITHSECRETARY;
		break;
	case TALK_WITHHAMMER:
		finalPat = PAT_WITHHAMMER;
		break;
	case TALK_WITHGLASSES:
		finalPat = PAT_WITHGLASSES;
		break;

	case TALK_MACBETH1:
	case TALK_MACBETH2:
	case TALK_MACBETH3:
	case TALK_MACBETH4:
	case TALK_MACBETH5:
	case TALK_MACBETH6:
	case TALK_MACBETH7:
	case TALK_MACBETH8:
		finalPat = 0;
		break;

	case TALK_SCAREDSTATIC:
		switch (_talkDirection) {
		case DOWN:
			headStandPat = PAT_SCAREDDOWN_STAND;
			bodyEndPat = 0;
			break;

		case RIGHT:
			headStandPat = PAT_SCAREDRIGHT_STAND;
			bodyEndPat = 0;
			break;

		case LEFT:
			headStandPat = PAT_SCAREDLEFT_STAND;
			bodyEndPat = 0;
			break;


		default:
			break;
		}
		break;

	default:
		break;
	}

	return true;
}

void RMTony::endTalk(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	int headStandPat, headEndPat;
	int bodyEndPat, finalPat;
	bool bStatic;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->headStandPat = _ctx->headEndPat = 0;
	_ctx->bodyEndPat = _ctx->finalPat = 0;
	_ctx->bStatic = false;

	_ctx->bodyEndPat = 0;
	_ctx->headEndPat = 0;

	if (!endTalkCalculate(_ctx->headStandPat, _ctx->headEndPat, _ctx->bodyEndPat, _ctx->finalPat, _ctx->bStatic))
		return;

	// Handles the end of an animated and static, leaving everything unchanged
	if (_bIsStaticTalk) {
		if (_nTalkType == TALK_WITHBEARDSTATIC) {
			setPattern(0);
			if (_talkDirection == UP || _talkDirection == LEFT) {
				_body.setPattern(BPAT_WITHBEARDLEFT_STATIC);
				_nBodyOffset.set(-41, -14);
			} else if (_talkDirection == DOWN || _talkDirection == RIGHT) {
				_body.setPattern(BPAT_WITHBEARDRIGHT_STATIC);
				_nBodyOffset.set(-26, -14);
			}
		} else {
			setPattern(_ctx->headStandPat);

			CORO_INVOKE_0(_body.waitForEndPattern);
		}

		_bIsTalking = false;
		return;
	}

	// Set the pattern
	if (_ctx->headEndPat != 0 && _ctx->bodyEndPat != 0) {
		setPattern(_ctx->headEndPat);

		CORO_INVOKE_0(_body.waitForEndPattern);

		_body.setPattern(_ctx->bodyEndPat);

		CORO_INVOKE_0(waitForEndPattern);
		CORO_INVOKE_0(_body.waitForEndPattern);
	} else if (_ctx->bodyEndPat != 0) {
		setPattern(_ctx->headStandPat);

		CORO_INVOKE_0(_body.waitForEndPattern);

		_body.setPattern(_ctx->bodyEndPat);

		CORO_INVOKE_0(_body.waitForEndPattern);
	} else if (_ctx->headEndPat != 0) {
		CORO_INVOKE_0(_body.waitForEndPattern);

		setPattern(_ctx->headEndPat);

		CORO_INVOKE_0(waitForEndPattern);
	} else {
		CORO_INVOKE_0(_body.waitForEndPattern);
	}

	if (_ctx->finalPat != 0) {
		_body.setPattern(0);
		setPattern(_ctx->finalPat);
	}

	_bIsTalking = false;

	CORO_END_CODE;
}

void RMTony::startStaticCalculate(CharacterTalkType nTalk, int &headPat, int &headLoopPat,
                                  int &bodyStartPat, int &bodyLoopPat) {
	int nPat = getCurPattern();

	headLoopPat = -1;

	switch (nPat) {
	case PAT_STANDDOWN:
		_talkDirection = DOWN;
		headPat = PAT_HEAD_RIGHT;
		break;

	case PAT_TAKELEFT_UP2:
	case PAT_TAKELEFT_MID2:
	case PAT_TAKELEFT_DOWN2:
	case PAT_GETUPLEFT:
	case PAT_STANDLEFT:
		_talkDirection = LEFT;
		headPat = PAT_HEAD_LEFT;
		break;

	case PAT_TAKERIGHT_UP2:
	case PAT_TAKERIGHT_MID2:
	case PAT_TAKERIGHT_DOWN2:
	case PAT_GETUPRIGHT:
	case PAT_STANDRIGHT:
		_talkDirection = RIGHT;
		headPat = PAT_HEAD_RIGHT;
		break;

	case PAT_TAKEUP_UP2:
	case PAT_TAKEUP_MID2:
	case PAT_TAKEUP_DOWN2:
	case PAT_STANDUP:
		_talkDirection = UP;
		headPat = PAT_HEAD_LEFT;
		break;
	}

	_bBodyFront = true;

	switch (nTalk) {
	case TALK_WITHRABBITSTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-21, -5);
			bodyStartPat = BPAT_WITHRABBITLEFT_START;
			bodyLoopPat = BPAT_WITHRABBITLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-4, -5);
			bodyStartPat = BPAT_WITHRABBITRIGHT_START;
			bodyLoopPat = BPAT_WITHRABBITRIGHT_LOOP;
			break;
		}
		break;

	case TALK_WITHCARDSSTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-34, -2);
			bodyStartPat = BPAT_WITHCARDSLEFT_START;
			bodyLoopPat = BPAT_WITHCARDSLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-4, -2);
			bodyStartPat = BPAT_WITHCARDSRIGHT_START;
			bodyLoopPat = BPAT_WITHCARDSRIGHT_LOOP;
			break;
		}
		break;

	case TALK_WITHRECIPESTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-61, -7);
			bodyStartPat = BPAT_WITHRECIPELEFT_START;
			bodyLoopPat = BPAT_WITHRECIPELEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-5, -7);
			bodyStartPat = BPAT_WITHRECIPERIGHT_START;
			bodyLoopPat = BPAT_WITHRECIPERIGHT_LOOP;
			break;
		}
		break;

	case TALK_WITHSNOWMANSTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-35, 2);
			bodyStartPat = BPAT_WITHSNOWMANLEFT_START;
			bodyLoopPat = BPAT_WITHSNOWMANLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-14, 2);
			bodyStartPat = BPAT_WITHSNOWMANRIGHT_START;
			bodyLoopPat = BPAT_WITHSNOWMANRIGHT_LOOP;
			break;
		}
		break;

	case TALK_WITH_NOTEBOOK:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-16, -9);
			bodyStartPat = BPAT_WITHNOTEBOOKLEFT_START;
			bodyLoopPat = BPAT_WITHNOTEBOOKLEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-6, -9);
			bodyStartPat = BPAT_WITHNOTEBOOKRIGHT_START;
			bodyLoopPat = BPAT_WITHNOTEBOOKRIGHT_LOOP;
			break;
		}
		break;

	case TALK_WITHMEGAPHONESTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-41, -8);
			bodyStartPat = BPAT_WITHMEGAPHONELEFT_START;
			bodyLoopPat = BPAT_WITHMEGAPHONELEFT_LOOP;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-14, -8);
			bodyStartPat = BPAT_WITHMEGAPHONERIGHT_START;
			bodyLoopPat = BPAT_WITHMEGAPHONERIGHT_LOOP;
			break;
		}
		break;

	case TALK_WITHBEARDSTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			_nBodyOffset.set(-41, -14);
			bodyStartPat = BPAT_WITHBEARDLEFT_START;
			bodyLoopPat = BPAT_STANDLEFT;
			headLoopPat = PAT_TALKBEARD_LEFT;
			headPat = 0;
			break;

		case DOWN:
		case RIGHT:
			_nBodyOffset.set(-26, -14);
			bodyStartPat = BPAT_WITHBEARDRIGHT_START;
			bodyLoopPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_TALKBEARD_RIGHT;
			headPat = 0;
			break;
		}
		break;

	case TALK_SCAREDSTATIC:
		switch (_talkDirection) {
		case DOWN:
			headPat = PAT_SCAREDDOWN_START;
			bodyLoopPat = BPAT_STANDDOWN;
			bodyStartPat = BPAT_STANDDOWN;
			headLoopPat = PAT_SCAREDDOWN_STAND;
			_nBodyOffset.set(4, 53);
			break;

		case LEFT:
			headPat = PAT_SCAREDLEFT_START;
			bodyLoopPat = BPAT_STANDLEFT;
			bodyStartPat = BPAT_STANDLEFT;
			headLoopPat = PAT_SCAREDLEFT_STAND;
			_nBodyOffset.set(6, 56);
			break;

		case RIGHT:
			headPat = PAT_SCAREDRIGHT_START;
			bodyLoopPat = BPAT_STANDRIGHT;
			bodyStartPat = BPAT_STANDRIGHT;
			headLoopPat = PAT_SCAREDRIGHT_STAND;
			_nBodyOffset.set(6, 56);
			break;

		default:
			break;
		}

	default:
		break;
	}
}

void RMTony::startStatic(CORO_PARAM, CharacterTalkType nTalk) {
	CORO_BEGIN_CONTEXT;
	int headPat, headLoopPat;
	int bodyStartPat, bodyLoopPat;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->headPat = _ctx->headLoopPat = 0;
	_ctx->bodyStartPat = _ctx->bodyLoopPat = 0;

	startStaticCalculate(nTalk, _ctx->headPat, _ctx->headLoopPat,
		_ctx->bodyStartPat, _ctx->bodyLoopPat);

	// e vai con i pattern
	_bIsStaticTalk = true;

	setPattern(_ctx->headPat);
	_body.setPattern(_ctx->bodyStartPat);

	CORO_INVOKE_0(_body.waitForEndPattern);
	CORO_INVOKE_0(waitForEndPattern);

	if (_ctx->headLoopPat != -1)
		setPattern(_ctx->headLoopPat);
	_body.setPattern(_ctx->bodyLoopPat);

	CORO_END_CODE;
}

void RMTony::endStaticCalculate(CharacterTalkType nTalk, int &bodyEndPat, int &finalPat, int &headEndPat) {
	switch (_talkDirection) {
	case UP:
	case LEFT:
		finalPat = PAT_STANDLEFT;
		break;

	case RIGHT:
	case DOWN:
		finalPat = PAT_STANDRIGHT;
		break;
	}

	switch (nTalk) {
	case TALK_WITHSNOWMANSTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_WITHSNOWMANLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_WITHSNOWMANRIGHT_END;
			break;
		}
		break;

	case TALK_WITHRECIPESTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_WITHRECIPELEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_WITHRECIPERIGHT_END;
			break;
		}
		break;

	case TALK_WITHRABBITSTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_WITHRABBITLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_WITHRABBITRIGHT_END;
			break;
		}
		break;

	case TALK_WITHCARDSSTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_WITHCARDSLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_WITHCARDSRIGHT_END;
			break;
		}
		break;

	case TALK_WITH_NOTEBOOK:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_WITHNOTEBOOKLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_WITHNOTEBOOKRIGHT_END;
			break;
		}
		break;

	case TALK_WITHMEGAPHONESTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_WITHMEGAPHONELEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_WITHMEGAPHONERIGHT_END;
			break;
		}
		break;

	case TALK_WITHBEARDSTATIC:
		switch (_talkDirection) {
		case UP:
		case LEFT:
			bodyEndPat = BPAT_WITHBEARDLEFT_END;
			break;

		case DOWN:
		case RIGHT:
			bodyEndPat = BPAT_WITHBEARDRIGHT_END;
			break;
		}
		break;

	case TALK_SCAREDSTATIC:
		switch (_talkDirection) {
		case LEFT:
			headEndPat = PAT_SCAREDLEFT_END;
			break;

		case DOWN:
			headEndPat = PAT_SCAREDDOWN_END;
			break;

		case RIGHT:
			headEndPat = PAT_SCAREDRIGHT_END;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

void RMTony::endStatic(CORO_PARAM, CharacterTalkType nTalk) {
	CORO_BEGIN_CONTEXT;
	int bodyEndPat;
	int finalPat;
	int headEndPat;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->bodyEndPat = 0;
	_ctx->finalPat = 0;
	_ctx->headEndPat = 0;

	endStaticCalculate(nTalk, _ctx->bodyEndPat, _ctx->finalPat, _ctx->headEndPat);

	if (_ctx->headEndPat != 0) {
		setPattern(_ctx->headEndPat);

		CORO_INVOKE_0(waitForEndPattern);
	} else {
		// Play please
		_body.setPattern(_ctx->bodyEndPat);

		CORO_INVOKE_0(_body.waitForEndPattern);
	}

	setPattern(_ctx->finalPat);
	_body.setPattern(0);

	_bIsStaticTalk = false;

	CORO_END_CODE;
}

/**
 * Waits until the end of a pattern
 */
void RMTony::waitForEndPattern(CORO_PARAM, uint32 hCustomSkip) {
	RMCharacter::waitForEndPattern(coroParam, hCustomSkip);
}

/**
 * Check if currently in an action
 */
bool RMTony::inAction() {
	return (_bActionPending && _action != 0) | _bAction;
}

/**
 * Check if there needs to be an update for scrolling movement
 */
bool RMTony::mustUpdateScrolling() {
	return ((!inAction()) || (isMoving()));
}

/**
 * Returns Tony's position
 */
RMPoint RMTony::position() {
	return _pos;
}

/**
 * Set the scrolling position
 */
void RMTony::setScrollPosition(const RMPoint &pt) {
	RMCharacter::setScrollPosition(pt);
}

/**
 * Tony disguises himself!
 */
void RMTony::setShepherdess(bool bIsPast) {
	_bShepherdess = bIsPast;
}

int RMTony::getShepherdess() {
	return _bShepherdess;
}

void RMTony::playSfx(int nSfx) {
	RMItem::playSfx(nSfx);
}

} // End of namespace Tony
