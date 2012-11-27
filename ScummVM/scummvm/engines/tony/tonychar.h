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

#ifndef TONY_TONYCHAR_H
#define TONY_TONYCHAR_H

#include "common/coroutines.h"
#include "tony/loc.h"

namespace Tony {

class RMTony : public RMCharacter {
private:
	enum CharacterDirection {
		UP, DOWN, LEFT, RIGHT
	};

public:
	enum CharacterTalkType {
		TALK_NORMAL,
		TALK_HIPS,
		TALK_SING,
		TALK_LAUGH,
		TALK_INDICATE,
		TALK_SCARED,
		TALK_SCARED2,
		TALK_WITHGLASSES,
		TALK_WITHHAMMER,
		TALK_WITHWORM,
		TALK_WITHROPE,
		TALK_WITHRABBIT,
		TALK_WITHRECIPE,
		TALK_WITHCARDS,
		TALK_WITHSNOWMAN,
		TALK_WITHSNOWMANSTATIC,
		TALK_WITHRABBITSTATIC,
		TALK_WITHRECIPESTATIC,
		TALK_WITHCARDSSTATIC,
		TALK_WITH_NOTEBOOK,
		TALK_WITHMEGAPHONESTATIC,
		TALK_WITHBEARDSTATIC,
		TALK_LAUGH2,
		TALK_DISGUSTED,
		TALK_SARCASTIC,
		TALK_MACBETH1,
		TALK_MACBETH2,
		TALK_MACBETH3,
		TALK_MACBETH4,
		TALK_MACBETH5,
		TALK_MACBETH6,
		TALK_MACBETH7,
		TALK_MACBETH8,
		TALK_MACBETH9,
		TALK_SCAREDSTATIC,
		TALK_WITHSECRETARY
	};

private:
	bool _bShow;
	bool _bShowShadow;
	bool _bBodyFront;
	// Useless variable?
	// RMGfxSourceBuffer8AB _shadow;
	bool _bActionPending;
	RMItem *_actionItem;
	int _action;
	int _actionParm;
	static bool _bAction;

	bool _bShepherdess;

	bool _bIsStaticTalk;
	bool _bIsTalking;
	int _nPatB4Talking;
	CharacterTalkType _nTalkType;
	CharacterDirection _talkDirection;
	RMPoint _nBodyOffset;

	int _nTimeLastStep;

	RMItem _body;
	uint32 _hActionThread;

protected:
	/**
	 * Overload of the allocation allocation of sprites
	 */
	virtual RMGfxSourceBuffer *newItemSpriteBuffer(int dimx, int dimy, bool bPreRLE);

	/**
	 * Watch thread which waits for the end of an action
	 */
	static void waitEndOfAction(CORO_PARAM, const void *param);

public:
	enum CharacterPatterns {
		PAT_TAKEUP_UP1 = 9,
		PAT_TAKEUP_UP2,
		PAT_TAKEUP_MID1,
		PAT_TAKEUP_MID2,
		PAT_TAKEUP_DOWN1,
		PAT_TAKEUP_DOWN2,

		PAT_TAKELEFT_UP1,
		PAT_TAKELEFT_UP2,
		PAT_TAKELEFT_MID1,
		PAT_TAKELEFT_MID2,
		PAT_TAKELEFT_DOWN1,
		PAT_TAKELEFT_DOWN2,

		PAT_TAKERIGHT_UP1,
		PAT_TAKERIGHT_UP2,
		PAT_TAKERIGHT_MID1,
		PAT_TAKERIGHT_MID2,
		PAT_TAKERIGHT_DOWN1,
		PAT_TAKERIGHT_DOWN2,

		PAT_GETUPLEFT,
		PAT_ONTHEFLOORLEFT,
		PAT_GETUPRIGHT,
		PAT_ONTHEFLOORRIGHT,

		// Sheperdess!
		PAT_PAST_WALKUP,
		PAT_PAST_WALKDOWN,
		PAT_PAST_WALKLEFT,
		PAT_PAST_WALKRIGHT,

		PAT_PAST_STANDUP,
		PAT_PAST_STANDDOWN,
		PAT_PAST_STANDLEFT,
		PAT_PAST_STANDRIGHT,

		// Speech
		PAT_TALK_UP,
		PAT_TALK_DOWN,
		PAT_TALK_LEFT,
		PAT_TALK_RIGHT,

		// Static head
		PAT_HEAD_UP,
		PAT_HEAD_DOWN,
		PAT_HEAD_LEFT,
		PAT_HEAD_RIGHT,

		// Laugh
		PAT_LAUGHLEFT_START,
		PAT_LAUGHLEFT_LOOP,
		PAT_LAUGHLEFT_END,
		PAT_LAUGHRIGHT_START,
		PAT_LAUGHRIGHT_LOOP,
		PAT_LAUGHRIGHT_END,

		// Speaking as a shepherdess
		PAT_PAST_TALKUP,
		PAT_PAST_TALKDOWN,
		PAT_PAST_TALKLEFT,
		PAT_PAST_TALKRIGHT,

		// Fear
		PAT_SCAREDLEFT_START,
		PAT_SCAREDLEFT_LOOP,
		PAT_SCAREDLEFT_END,
		PAT_SCAREDRIGHT_START,
		PAT_SCAREDRIGHT_LOOP,
		PAT_SCAREDRIGHT_END,
		PAT_SCAREDDOWN_START,
		PAT_SCAREDDOWN_LOOP,
		PAT_SCAREDDOWN_END,

		// With objects: full body
		PAT_WITHGLASSES,
		PAT_WITHROPE,
		PAT_WITHWORM,
		PAT_WITHHAMMER,

		// Sound the whistle
		PAT_WHISTLERIGHT,

		// Head with beard
		PAT_TALKBEARD_LEFT,
		PAT_TALKBEARD_RIGHT,

		// Sniff
		PAT_SNIFF_LEFT,
		PAT_SNIFF_RIGHT,

		// Disgusted
		PAT_DISGUSTEDLEFT_START,
		PAT_DISGUSTEDLEFT_LOOP,
		PAT_DISGUSTEDLEFT_END,
		PAT_DISGUSTEDRIGHT_START,
		PAT_DISGUSTEDRIGHT_LOOP,
		PAT_DISGUSTEDRIGHT_END,
		PAT_SARCASTICLEFT_START,
		PAT_SARCASTICLEFT_LOOP,
		PAT_SARCASTICLEFT_END,
		PAT_SARCASTICRIGHT_START,
		PAT_SARCASTICRIGHT_LOOP,
		PAT_SARCASTICRIGHT_END,

		// Stand scared
		PAT_SCAREDLEFT_STAND,
		PAT_SCAREDRIGHT_STAND,
		PAT_SCAREDDOWN_STAND,

		PAT_PUTLEFT_UP1,
		PAT_PUTLEFT_UP2,
		PAT_PUTRIGHT_UP1,
		PAT_PUTRIGHT_UP2,
		PAT_PUTLEFT_MID1,
		PAT_PUTLEFT_MID2,
		PAT_PUTRIGHT_MID1,
		PAT_PUTRIGHT_MID2,
		PAT_PUTLEFT_DOWN1,
		PAT_PUTLEFT_DOWN2,
		PAT_PUTRIGHT_DOWN1,
		PAT_PUTRIGHT_DOWN2,
		PAT_PUTUP_UP1,
		PAT_PUTUP_UP2,
		PAT_PUTUP_MID1,
		PAT_PUTUP_MID2,
		PAT_PUTUP_DOWN1,
		PAT_PUTUP_DOWN2,

		PAT_WITHSECRETARY
	};

	enum CharacterBodyPatterns {
		BPAT_STANDUP = 1,
		BPAT_STANDDOWN,
		BPAT_STANDLEFT,
		BPAT_STANDRIGHT,

		BPAT_HAMMER,
		BPAT_SNOWMAN,
		BPAT_WORM,
		BPAT_GLASS,

		BPAT_SINGLEFT_START,
		BPAT_SINGLEFT_LOOP,
		BPAT_SINGLEFT_END,

		BPAT_HIPSLEFT_START,
		BPAT_HIPSLEFT_LOOP,
		BPAT_HIPSLEFT_END,
		BPAT_HIPSRIGHT_START,
		BPAT_HIPSRIGHT_LOOP,
		BPAT_HIPSRIGHT_END,
		BPAT_HIPSUP_START,
		BPAT_HIPSUP_LOOP,
		BPAT_HIPSUP_END,
		BPAT_HIPSDOWN_START,
		BPAT_HIPSDOWN_LOOP,
		BPAT_HIPSDOWN_END,

		BPAT_LAUGHLEFT,
		BPAT_LAUGHRIGHT,

		BPAT_INDICATELEFT,
		BPAT_INDICATERIGHT,

		BPAT_SCAREDDOWN_START,
		BPAT_SCAREDDOWN_LOOP,
		BPAT_SCAREDDOWN_END,
		BPAT_SCAREDLEFT_START,
		BPAT_SCAREDLEFT_LOOP,
		BPAT_SCAREDLEFT_END,
		BPAT_SCAREDRIGHT_START,
		BPAT_SCAREDRIGHT_LOOP,
		BPAT_SCAREDRIGHT_END,
		BPAT_SCAREDUP_START,
		BPAT_SCAREDUP_LOOP,
		BPAT_SCAREDUP_END,

		BPAT_ROPE,

		BPAT_WITHRABBITLEFT_START,
		BPAT_WITHRABBITLEFT_LOOP,
		BPAT_WITHRABBITLEFT_END,
		BPAT_WITHRABBITRIGHT_START,
		BPAT_WITHRABBITRIGHT_LOOP,
		BPAT_WITHRABBITRIGHT_END,

		BPAT_WITHRECIPELEFT_START,
		BPAT_WITHRECIPELEFT_LOOP,
		BPAT_WITHRECIPELEFT_END,
		BPAT_WITHRECIPERIGHT_START,
		BPAT_WITHRECIPERIGHT_LOOP,
		BPAT_WITHRECIPERIGHT_END,

		BPAT_WITHCARDSLEFT_START,
		BPAT_WITHCARDSLEFT_LOOP,
		BPAT_WITHCARDSLEFT_END,
		BPAT_WITHCARDSRIGHT_START,
		BPAT_WITHCARDSRIGHT_LOOP,
		BPAT_WITHCARDSRIGHT_END,

		BPAT_WITHSNOWMANLEFT_START,
		BPAT_WITHSNOWMANLEFT_LOOP,
		BPAT_WITHSNOWMANLEFT_END,
		BPAT_WITHSNOWMANRIGHT_START,
		BPAT_WITHSNOWMANRIGHT_LOOP,
		BPAT_WITHSNOWMANRIGHT_END,

		BPAT_WITHNOTEBOOKLEFT_START,
		BPAT_WITHNOTEBOOKLEFT_LOOP,
		BPAT_WITHNOTEBOOKLEFT_END,
		BPAT_WITHNOTEBOOKRIGHT_START,
		BPAT_WITHNOTEBOOKRIGHT_LOOP,
		BPAT_WITHNOTEBOOKRIGHT_END,

		BPAT_WITHMEGAPHONELEFT_START,
		BPAT_WITHMEGAPHONELEFT_LOOP,
		BPAT_WITHMEGAPHONELEFT_END,
		BPAT_WITHMEGAPHONERIGHT_START,
		BPAT_WITHMEGAPHONERIGHT_LOOP,
		BPAT_WITHMEGAPHONERIGHT_END,

		BPAT_WITHBEARDLEFT_START,
		BPAT_WITHBEARDLEFT_END,
		BPAT_WITHBEARDRIGHT_START,
		BPAT_WITHBEARDRIGHT_END,
		BPAT_WITHBEARDLEFT_STATIC,
		BPAT_WITHBEARDRIGHT_STATIC,

		BPAT_MACBETH1,
		BPAT_MACBETH2,
		BPAT_MACBETH3,
		BPAT_MACBETH4,
		BPAT_MACBETH5,
		BPAT_MACBETH6,
		BPAT_MACBETH7,
		BPAT_MACBETH8,
		BPAT_MACBETH9,

		BPAT_WITHSECRETARY
	};

public:
	static void initStatics();
	RMTony();

	/**
	 * Initialize Tony
	 */
	void init();

	/**
	 * Free all memory
	 */
	void close();

	/**
	 * Tony makes a frame, updating the movement, etc.
	 */
	void doFrame(CORO_PARAM, RMGfxTargetBuffer *bigBuf, int curLoc);

	/**
	 * Draw method, which controls chararacter display
	 */
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	/**
	 * Show or hide
	 */
	void show();
	void hide(bool bShowShadow = false);

	/**
	 * Move and make an action, if necessary
	 */
	void moveAndDoAction(CORO_PARAM, RMPoint dst, RMItem *item, int nAction, int nActionParm = 0);

	/**
	 * Tony stops (on the right side with respect to any subject)
	 */
	virtual void stop(CORO_PARAM);
	void stopNoAction(CORO_PARAM);

	/**
	 * Set a pattern
	 */
	void setPattern(int npatt, bool bPlayP0 = false);

	/**
	 * Reads the current pattern
	 */
	int getCurPattern();

	/**
	 * Waits until the end of a pattern
	 */
	void waitForEndPattern(CORO_PARAM, uint32 hCustomSkip = CORO_INVALID_PID_VALUE);

	/**
	 * Check if currently in an action
	 */
	bool inAction();

	/**
	 * Check if there needs to be an update for scrolling movement
	 */
	bool mustUpdateScrolling();

	/**
	 * Returns Tony's position
	 */
	RMPoint position();

	/**
	 * Set the scrolling position
	 */
	void setScrollPosition(const RMPoint &pt);

	/**
	 * Set the take animation
	 */
	void take(int nWhere, int nPart);
	void put(int nWhere, int nPart);

	/**
	 * Start or End Talk
	 */
	bool startTalkCalculate(CharacterTalkType nTalkType, int &headStartPat, int &bodyStartPat,
	                        int &headLoopPat, int &bodyLoopPat);
	void startTalk(CORO_PARAM, CharacterTalkType nTalkType);
	bool endTalkCalculate(int &headStandPat, int &headEndPat, int &bodyEndPat, int &finalPat, bool &bStatic);
	void endTalk(CORO_PARAM);

	/**
	 * Start or End Static
	 */
	void startStaticCalculate(CharacterTalkType nTalk, int &headPat, int &headLoopPat,
	                          int &bodyStartPat, int &bodyLoopPat);
	void startStatic(CORO_PARAM, CharacterTalkType nTalkType);
	void endStaticCalculate(CharacterTalkType nTalk, int &bodyEndPat, int &finalPat, int &headEndPat);
	void endStatic(CORO_PARAM, CharacterTalkType nTalkType);

	/**
	 * Tony disguises himself!
	 */
	void setShepherdess(bool bIsPast);

	int getShepherdess();

	/**
	 * Perform an action
	 */
	void executeAction(int nAction, int nActionItem, int nParm);

	void playSfx(int nSfx);
};

} // End of namespace Tony

#endif
