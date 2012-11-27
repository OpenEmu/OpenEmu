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

#ifndef MOHAWK_CSTIME_UI_H
#define MOHAWK_CSTIME_UI_H

#include "mohawk/cstime.h"
#include "graphics/fonts/winfont.h"

namespace Mohawk {

struct CSTimeHelpQaR {
	uint16 text, speech;
};

class CSTimeHelp {
public:
	CSTimeHelp(MohawkEngine_CSTime *vm);
	~CSTimeHelp();

	void addQaR(uint16 text, uint16 speech);
	void start();
	void end(bool runEvents = true);
	void cleanupAfterFlapping();

	void mouseDown(Common::Point &pos);
	void mouseMove(Common::Point &pos);
	void mouseUp(Common::Point &pos);

	void reset();

	uint getState() { return _state; }

protected:
	MohawkEngine_CSTime *_vm;

	uint _state;
	uint16 _currHover, _currEntry, _nextToProcess;
	Common::Array<CSTimeHelpQaR> _qars;
	Common::Array<uint16> _askedAlready;

	void display();
	void highlightLine(uint line);
	void unhighlightLine(uint line);
	void selectStrings();

	bool noHelperChanges();
};

class CSTimeOptions {
public:
	CSTimeOptions(MohawkEngine_CSTime *vm);
	~CSTimeOptions();

	uint getState() { return _state; }

protected:
	MohawkEngine_CSTime *_vm;

	uint _state;
};

#define MAX_DISPLAYED_ITEMS 4
class CSTimeInventoryDisplay {
public:
	CSTimeInventoryDisplay(MohawkEngine_CSTime *vm, Common::Rect baseRect);
	~CSTimeInventoryDisplay();

	void install();
	void draw();
	void show();
	void hide();
	void idle();
	void clearDisplay();
	void insertItemInDisplay(uint16 id);
	void removeItem(uint16 id);

	void mouseDown(Common::Point &pos);
	void mouseMove(Common::Point &pos);
	void mouseUp(Common::Point &pos);

	void activateCuffs(bool active);
	void setCuffsFlashing();
	bool getCuffsState() { return _cuffsState; }
	uint16 getCuffsShape() { return _cuffsShape; }

	bool isItemDisplayed(uint16 id);
	uint16 getDisplayedNum(uint id) { return _displayedItems[id]; }
	uint16 getLastDisplayedClicked() { return getDisplayedNum(_draggedItem); }

	void setState(uint state) { _state = state; }
	uint getState() { return _state; }

	Common::Rect _invRect;
	Common::Rect _itemRect[MAX_DISPLAYED_ITEMS];

protected:
	MohawkEngine_CSTime *_vm;

	uint _state;
	bool _cuffsState;
	uint16 _cuffsShape;
	uint16 _draggedItem;
	uint16 _displayedItems[MAX_DISPLAYED_ITEMS];
};

class CSTimeBook {
public:
	CSTimeBook(MohawkEngine_CSTime *vm);
	~CSTimeBook();

	uint getState() { return _state; }
	void setState(uint state) { _state = state; }

	void drawSmallBook();

protected:
	MohawkEngine_CSTime *_vm;

	uint _state;
	Feature *_smallBookFeature;
};

#define NUM_NOTE_PIECES 3
class CSTimeCarmenNote {
public:
	CSTimeCarmenNote(MohawkEngine_CSTime *vm);
	~CSTimeCarmenNote();

	uint getState() { return _state; }
	void setState(uint state) { _state = state; }

	void clearPieces();
	bool havePiece(uint16 piece);
	void addPiece(uint16 piece, uint16 speech);
	void drawSmallNote();
	void drawBigNote();
	void closeNote();

protected:
	MohawkEngine_CSTime *_vm;

	uint _state;
	uint16 _pieces[NUM_NOTE_PIECES];
	Feature *_feature;
};

enum CSTimeInterfaceState {
	kCSTimeInterfaceStateNormal = 1,
	kCSTimeInterfaceStateDragStart = 2,
	kCSTimeInterfaceStateDragging = 3
};

class CSTimeInterface {
public:
	CSTimeInterface(MohawkEngine_CSTime *vm);
	~CSTimeInterface();

	void cursorInstall();
	void cursorActivate(bool state);
	bool cursorGetState() { return _cursorActive; }
	void cursorIdle();
	void cursorChangeShape(uint16 id);
	uint16 cursorGetShape();
	void cursorSetShape(uint16 id, bool reset = true);
	void cursorSetWaitCursor();

	void openResFile();
	void install();
	void draw();
	void idle();
	void mouseDown(Common::Point pos);
	void mouseMove(Common::Point pos);
	void mouseUp(Common::Point pos);

	void cursorOverHotspot();
	void setCursorForCurrentPoint();

	void clearTextLine();
	void displayTextLine(Common::String text);

	void clearDialogArea();
	void clearDialogLine(uint line);
	void displayDialogLine(uint16 id, uint line, byte color = 32);

	void drawTextIdToBubble(uint16 id);
	void drawTextToBubble(Common::String *text);
	void closeBubble();

	void startDragging(uint16 id);
	void stopDragging();
	void setGrabPoint();
	uint16 getDraggedNum() { return _draggedItem; }
	Common::Point getGrabPoint() { return _grabPoint; }
	bool grabbedFromInventory();

	void dropItemInInventory(uint16 id);

	CSTimeInterfaceState getState() { return _state; }
	void setState(CSTimeInterfaceState state) { _state = state; }

	CSTimeHelp *getHelp() { return _help; }
	CSTimeInventoryDisplay *getInventoryDisplay() { return _inventoryDisplay; }
	CSTimeBook *getBook() { return _book; }
	CSTimeCarmenNote *getCarmenNote() { return _note; }
	CSTimeOptions *getOptions() { return _options; }

	const Common::String &getRolloverText() { return _rolloverText; }
	const Common::String &getDialogText() { return _dialogText; }
	const Common::String &getCurrBubbleText() { return _currentBubbleText; }
	const Common::Array<Common::String> &getDialogLines() { return _dialogLines; }
	const Common::Array<byte> &getDialogLineColors() { return _dialogLineColors; }

	const Graphics::Font &getNormalFont() const;
	const Graphics::Font &getDialogFont() const;
	const Graphics::Font &getRolloverFont() const;

	Common::Rect _sceneRect, _uiRect;
	Common::Rect _dialogTextRect, _bookRect, _noteRect;

protected:
	MohawkEngine_CSTime *_vm;

	Common::String _bubbleText;
	bool _mouseWasInScene;
	CSTimeInterfaceState _state;

	CSTimeHelp *_help;
	CSTimeInventoryDisplay *_inventoryDisplay;
	CSTimeBook *_book;
	CSTimeCarmenNote *_note;
	CSTimeOptions *_options;

	Feature *_uiFeature;
	Feature *_dialogTextFeature;
	Feature *_rolloverTextFeature;
	Feature *_bubbleTextFeature;

	Common::String _rolloverText;
	Common::String _dialogText;
	Common::String _currentBubbleText;
	Common::Array<Common::String> _dialogLines;
	Common::Array<byte> _dialogLineColors;

	uint16 _draggedItem;
	Common::Point _grabPoint;

	Graphics::WinFont _normalFont, _dialogFont, _rolloverFont;

	bool _cursorActive;
	uint16 _cursorShapes[3];
	uint32 _cursorNextTime;
};

} // End of namespace Mohawk

#endif
