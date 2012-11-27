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

#ifndef TONY_FONT_H
#define TONY_FONT_H

#include "common/system.h"
#include "common/coroutines.h"
#include "tony/gfxcore.h"
#include "tony/resid.h"

namespace Tony {

class RMInput;
class RMInventory;
class RMItem;
class RMLoc;
class RMLocation;
class RMPointer;

/**
 * Manages a font, in which there is a different surface for each letter
 */
class RMFont : public RMGfxTaskSetPrior {
protected:
	int _nLetters;
	RMGfxSourceBuffer8RLEByte *_letter;
public:
	int _fontDimx, _fontDimy;

private:
	int _dimx, _dimy;

	class RMFontPrimitive : public RMGfxPrimitive {
	public:
		RMFontPrimitive() : RMGfxPrimitive() { _nChar = 0; }
		RMFontPrimitive(RMGfxTask *task) : RMGfxPrimitive(task) { _nChar = 0; }
		virtual ~RMFontPrimitive() { }
		virtual RMGfxPrimitive *duplicate() {
			return new RMFontPrimitive(*this);
		}

		int _nChar;
	};

protected:
	// Loads the font
	void load(uint32 resID, int nChars, int dimx, int dimy, uint32 palResID = RES_F_PAL);
	void load(const byte *buf, int nChars, int dimx, int dimy, uint32 palResID = RES_F_PAL);

	// Remove the font
	void unload();

protected:
	// Conversion form character to font index
	virtual int convertToLetter(byte nChar) = 0;

	// Character width
	virtual int letterLength(int nChar, int nNext = 0) = 0;

public:
	virtual int letterHeight() = 0;

public:
	RMFont();
	virtual ~RMFont();

	// Initialization and closing
	virtual void init() = 0;
	virtual void close();

	// Drawing
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBug, RMGfxPrimitive *prim);

	// Create a primitive for a letter
	RMGfxPrimitive *makeLetterPrimitive(byte bChar, int &nLength);

	// Length in pixels of a string with the current font
	int stringLen(const Common::String &text);
	int stringLen(char bChar, char bNext = 0);
};

class RMFontColor : public virtual RMFont {
private:
	byte _fontR, _fontG, _fontB;

public:
	RMFontColor();
	virtual ~RMFontColor();
	virtual void setBaseColor(byte r, byte g, byte b);
};

class RMFontWithTables : public virtual RMFont {
protected:
	int _cTable[256];
	int _lTable[256];
	int _lDefault;
	int _hDefault;
	signed char _l2Table[256][256];

protected:
	// Overloaded methods
	int convertToLetter(byte nChar);
	int letterLength(int nChar, int nNext = 0);

public:
	int letterHeight() {
		return _hDefault;
	}
	virtual ~RMFontWithTables() {}
};

class RMFontDialog : public RMFontColor, public RMFontWithTables {
public:
	void init();
	virtual ~RMFontDialog() {}
};

class RMFontObj : public RMFontColor, public RMFontWithTables {
private:
	void setBothCase(int nChar, int nNext, signed char spiazz);

public:
	void init();
	virtual ~RMFontObj() {}
};

class RMFontMacc : public RMFontColor, public RMFontWithTables {
public:
	void init();
	virtual ~RMFontMacc() {}
};

class RMFontCredits : public RMFontColor, public RMFontWithTables {
public:
	void init();
	virtual ~RMFontCredits() {}
	virtual void setBaseColor(byte r, byte g, byte b) {}
};

/**
 * Manages writing text onto9 the screen
 */
class RMText : public RMGfxWoodyBuffer {
private:
	static RMFontColor *_fonts[4];
	int _maxLineLength;

public:
	enum HorAlign {
		HLEFT,
		HLEFTPAR,
		HCENTER,
		HRIGHT
	};

	enum VerAlign {
		VTOP,
		VCENTER,
		VBOTTOM
	};

private:
	HorAlign _aHorType;
	VerAlign _aVerType;
	byte _textR, _textG, _textB;

protected:
	virtual void clipOnScreen(RMGfxPrimitive *prim);

public:
	RMText();
	virtual ~RMText();
	static void initStatics();
	static void unload();

	// Set the alignment type
	void setAlignType(HorAlign aHor, VerAlign aVer);

	// Sets the maximum length of a line in pixels (used to format the text)
	void setMaxLineLength(int max);

	// Write the text
	void writeText(const Common::String &text, int font, int *time = NULL);
	void writeText(Common::String text, RMFontColor *font, int *time = NULL);

	// Overloaded function to decide when you delete the object from the OT list
	virtual void removeThis(CORO_PARAM, bool &result);

	// Overloading of the Draw to center the text, if necessary
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Set the base color
	void setColor(byte r, byte g, byte b);
};

/**
 * Manages text in a dialog
 */
class RMTextDialog : public RMText {
protected:
	int _startTime;
	int _time;
	bool _bSkipStatus;
	RMPoint _dst;
	uint32 _hEndDisplay;
	bool _bShowed;
	bool _bForceTime;
	bool _bForceNoTime;
	uint32 _hCustomSkip;
	uint32 _hCustomSkip2;
	RMInput *_input;
	bool _bAlwaysDisplay;
	bool _bNoTab;

public:
	RMTextDialog();
	virtual ~RMTextDialog();

	// Write the text
	void writeText(const Common::String &text, int font, int *time = NULL);
	void writeText(const Common::String &text, RMFontColor *font, int *time = NULL);

	// Overloaded function to decide when you delete the object from the OT list
	virtual void removeThis(CORO_PARAM, bool &result);

	// Overloaded de-registration
	virtual void unregister();

	// Overloading of the Draw to center the text, if necessary
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	// Set the position
	void setPosition(const RMPoint &pt);

	// Waiting
	void waitForEndDisplay(CORO_PARAM);
	void setCustomSkipHandle(uint32 hCustomSkip);
	void setCustomSkipHandle2(uint32 hCustomSkip);
	void setSkipStatus(bool bEnabled);
	void setForcedTime(uint32 dwTime);
	void setNoTab();
	void forceTime();
	void forceNoTime();
	void setAlwaysDisplay();

	// Set the input device, to allow skip from mouse
	void setInput(RMInput *input);

	void show();
	void hide(CORO_PARAM);
};

class RMTextDialogScrolling : public RMTextDialog {
protected:
	RMLocation *_curLoc;
	RMPoint _startScroll;

	virtual void clipOnScreen(RMGfxPrimitive *prim);

public:
	RMTextDialogScrolling();
	RMTextDialogScrolling(RMLocation *loc);
	virtual ~RMTextDialogScrolling();

	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
};

/**
 * Manages the name of a selected item on the screen
 */
class RMTextItemName : protected RMText {
protected:
	RMPoint _mpos;
	RMPoint _curscroll;
	RMItem *_item;

public:
	RMTextItemName();
	virtual ~RMTextItemName();

	void setMouseCoord(const RMPoint &m);

	void doFrame(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMLocation &loc, RMPointer &ptr, RMInventory &inv);
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);

	RMPoint getHotspot();
	RMItem *getSelectedItem();
	bool isItemSelected();

	virtual void removeThis(CORO_PARAM, bool &result);
};

/**
 * Manages the selection of screen items in a box
 */
class RMDialogChoice : public RMGfxWoodyBuffer {
private:
	int _curSelection;
	int _numChoices;
	RMText *_drawedStrings;
	RMPoint *_ptDrawStrings;
	int _curAdded;
	bool _bShow;
	RMGfxSourceBuffer8 _dlgText;
	RMGfxSourceBuffer8 _dlgTextLine;
	RMPoint _ptDrawPos;
	uint32 _hUnreg;
	bool _bRemoveFromOT;

protected:
	void prepare(CORO_PARAM);
	void setSelected(CORO_PARAM, int pos);

public:
	virtual void removeThis(CORO_PARAM, bool &result);
	virtual void draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim);
	void unregister();

public:
	// Initialization
	RMDialogChoice();
	virtual ~RMDialogChoice();

	// Initialization and closure
	void init();
	void close();

	// Sets the number of possible sentences, which then be added with AddChoice()
	void setNumChoices(int num);

	// Adds a string with the choice
	void addChoice(const Common::String &string);

	// Show and hide the selection, with possible animations.
	// NOTE: If no parameter is passed to Show(), it is the obligation of
	// caller to ensure that the class is inserted into OT list
	void show(CORO_PARAM, RMGfxTargetBuffer *bigBuf = NULL);
	void hide(CORO_PARAM);

	// Polling Update
	void doFrame(CORO_PARAM, RMPoint ptMousePos);

	// Returns the currently selected item, or -1 if none is selected
	int getSelection();
};

} // End of namespace Tony

#endif
