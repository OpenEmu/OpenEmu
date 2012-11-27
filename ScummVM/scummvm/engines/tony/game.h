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

#ifndef TONY_GAME_H
#define TONY_GAME_H

#include "tony/gfxcore.h"
#include "tony/input.h"
#include "tony/loc.h"
#include "tony/utils.h"

namespace Tony {

#define INIT_GFX16_FROMRAW(dwRes, buf16)                 \
	raw = new RMResRaw(dwRes);                             \
	assert(raw->isValid());                                \
	assert((buf16) == NULL);                               \
	(buf16) = new RMGfxSourceBuffer16(false);              \
	(buf16)->init(*raw, raw->width(), raw->height());      \
	delete raw;

#define INIT_GFX8_FROMRAW(raw, dwRes, buf8)              \
	raw = new RMResRaw(dwRes);                             \
	assert(raw->isValid());                                \
	assert((buf8) == NULL);                                \
	(buf8) = new RMGfxSourceBuffer8RLEByte();              \
	(buf8)->init(*raw, raw->width(), raw->height(), true); \
	delete raw;

// X & Y dimensions of the adventure
#define RM_SX       640
#define RM_SY       480

// X & Y dimensions of bigbuf
#define RM_BBX      (RM_SX)
#define RM_BBY      (RM_SY)

// Skipping X & Y
#define RM_SKIPY    ((RM_BBY - RM_SY) / 2)
#define RM_SKIPX    0

// Tony's actions
enum RMTonyAction {
	TA_GOTO = 0,
	TA_TAKE,
	TA_USE,
	TA_EXAMINE,
	TA_TALK,
	TA_PERORATE,

	TA_COMBINE = 10,
	TA_RECEIVECOMBINE,
	TA_COMBINEGIVE,
	TA_RECEIVECOMBINEGIVE
};

// Global Functions
void mainEnableGUI();
void mainDisableGUI();

// Classes
class RMPointer {
public:
	enum PointerType {
		PTR_NONE = 0,
		PTR_ARROWUP,
		PTR_ARROWDOWN,
		PTR_ARROWLEFT,
		PTR_ARROWRIGHT,
		PTR_ARROWMAP,
		PTR_CUSTOM
	};

private:
	RMGfxSourceBuffer8 *_pointer[16];
	RMPoint _hotspot[16];
	RMPoint _cursorHotspot;

	RMItem *_specialPointer[16];

	int _nCurPointer;
	int _nCurSpecialPointer;

	RMGfxSourceBuffer8 *_nCurCustomPointer;

public:
	/**
	 * Constructor & destructor
	 */
	RMPointer();
	virtual ~RMPointer();

	/**
	 * Initialization
	 */
	void init();

	/**
	 * Deinitialization
	 */
	void close();

	/**
	 * Process a frame
	 */
	void doFrame();

	/**
	 * draw method
	 */
	void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	/**
	 * Sets a new action as current
	 */
	void setAction(RMTonyAction action);

	/**
	 * Sets a new pointer
	 */
	void setSpecialPointer(PointerType ptr);

	PointerType getSpecialPointer();

	/**
	 * Set the new custom pointer
	 */
	void setCustomPointer(RMGfxSourceBuffer8 *ptr);

	/**
	 * Return the current action to be applied according to the pointer
	 */
	int curAction();

	/**
	 * Update the cursor
	 */
	void updateCursor();

	/**
	 * Show the cursor
	 */
	void showCursor();

	/**
	 * Hide the cursor
	 */
	void hideCursor();
};

class RMOptionButton: public RMGfxTaskSetPrior {
public:
	RMRect _rect;
	RMGfxSourceBuffer16 *_buf;
	bool _bActive;
	bool _bHasGfx;
	bool _bDoubleState;

public:
	RMOptionButton(uint32 dwRes, RMPoint pt, bool bDoubleState = false);
	RMOptionButton(const RMRect &pt);
	virtual ~RMOptionButton();

	bool doFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick);
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void addToList(RMGfxTargetBuffer &bigBuf);
	bool isActive();
	void setActiveState(bool bState);
};

class RMOptionSlide : public RMGfxTaskSetPrior {
private:
	RMOptionButton *_pushLeft;
	RMOptionButton *_pushRight;
	RMGfxSourceBuffer16 *_sliderCenter;
	RMGfxSourceBuffer16 *_sliderLeft;
	RMGfxSourceBuffer16 *_sliderRight;
	RMGfxSourceBuffer16 *_sliderSingle;
	int _nSlideSize;
	RMPoint _pos;
	int _nValue;
	int _nMax;
	int _nStep;

public:
	RMOptionSlide(const RMPoint &pt, int m_nRange = 100, int m_nStartValue = 0, int slideSize = 300);
	virtual ~RMOptionSlide();

	bool doFrame(const RMPoint &mousePos, bool bLeftClick, bool bRightClick);
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void addToList(RMGfxTargetBuffer &bigBuf);

	int getValue();
};

class RMOptionScreen : public RMGfxWoodyBuffer {
private:
	RMGfxSourceBuffer16 *_menu;
	RMGfxSourceBuffer16 *_quitConfirm;
	RMGfxSourceBuffer16 *_hideLoadSave;
	RMOptionButton *_buttonQuitYes;
	RMOptionButton *_buttonQuitNo;
	RMOptionButton *_buttonExit;
	RMOptionButton *_buttonQuit;
	RMOptionButton *_buttonLoad;
	RMOptionButton *_buttonSave;
	RMOptionButton *_buttonGameMenu;
	RMOptionButton *_buttonGfxMenu;
	RMOptionButton *_buttonSoundMenu;
	RMGfxSourceBuffer8 *_saveEasy;
	RMGfxSourceBuffer8 *_saveHard;
	RMGfxSourceBuffer16 *_curThumb[6];
	Common::String _curThumbName[6];
	byte _curThumbDiff[6];
	RMOptionButton *_buttonSave_States[6];
	RMOptionButton *_buttonSave_ArrowLeft;
	RMOptionButton *_buttonSave_ArrowRight;
	RMOptionButton *_buttonGfx_Tips;

	RMOptionButton *_buttonSound_DubbingOn;
	RMOptionButton *_buttonSound_MusicOn;
	RMOptionButton *_buttonSound_SFXOn;

	RMOptionSlide *_slideTonySpeed;
	RMOptionSlide *_slideTextSpeed;


	int _statePos;
	bool _bEditSaveName;
	int _nEditPos;
	char _editName[256];

	union {
		RMOptionButton *_buttonGame_Lock;
		RMOptionButton *_buttonGfx_Anni30;
		RMOptionSlide *_sliderSound_Music;
	};
	union {
		RMOptionButton *_buttonGame_TimerizedText;
		RMOptionButton *_buttonGfx_AntiAlias;
		RMOptionSlide *_sliderSound_SFX;
	};
	union {
		RMOptionButton *_buttonGame_Scrolling;
		RMOptionButton *_buttonGfx_Sottotitoli;
		RMOptionSlide *_sliderSound_Dubbing;
	};
	union {
		RMOptionButton *_buttonGame_InterUp;
		RMOptionButton *_buttonGfx_Trans;
	};

	int _fadeStep;
	bool _bExit;
	bool _bQuitConfirm;
	int _fadeY;
	int _fadeTime;
	bool _bLoadMenuOnly;
	bool _bNoLoadSave;
	bool _bAlterGfx;

	enum OptionScreenState {
		MENUGAME,
		MENUGFX,
		MENUSOUND,
		MENULOAD,
		MENUSAVE,
		MENUNONE
	};

	OptionScreenState _nState;
	OptionScreenState _nLastState;

public:
	RMOptionScreen();
	virtual ~RMOptionScreen();

	void init(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result);
	void initLoadMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result);
	void initSaveMenuOnly(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool bAlternateGfx, bool &result);
	void initNoLoadSave(CORO_PARAM, RMGfxTargetBuffer &bigBuf, bool &result);
	void reInit(RMGfxTargetBuffer &bigBuf);
	bool close();
	bool isClosing();

	// Overloaded methods
	virtual int priority();
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	virtual void removeThis(CORO_PARAM, bool &result);

	/**
	 * Polling for the option screen
	 */
	void doFrame(CORO_PARAM, RMInput *m_input);

	/**
	 * Retrieves a savegame's thumbnail, description, and difficulty level
	 */
	static bool loadThumbnailFromSaveState(int numState, byte *lpDestBuf, Common::String &name, byte &diff);

protected:
	// Initialization and state change
	void initState(CORO_PARAM);
	void closeState();
	void changeState(CORO_PARAM, OptionScreenState newState);

	// Repaint the options menu
	void refreshAll(CORO_PARAM);
	void refreshThumbnails();
};

} // End of namespace Tony

#endif
