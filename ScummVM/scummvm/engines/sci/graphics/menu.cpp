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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/font.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/menu.h"

namespace Sci {

GfxMenu::GfxMenu(EventManager *event, SegManager *segMan, GfxPorts *ports, GfxPaint16 *paint16, GfxText16 *text16, GfxScreen *screen, GfxCursor *cursor)
	: _event(event), _segMan(segMan), _ports(ports), _paint16(paint16), _text16(text16), _screen(screen), _cursor(cursor) {

	_menuSaveHandle = NULL_REG;
	_barSaveHandle = NULL_REG;
	_oldPort = NULL;

	reset();
}

GfxMenu::~GfxMenu() {
	for (GuiMenuItemList::iterator itemIter = _itemList.begin(); itemIter != _itemList.end(); ++itemIter)
		delete *itemIter;

	_itemList.clear();

	for (GuiMenuList::iterator menuIter = _list.begin(); menuIter != _list.end(); ++menuIter)
		delete *menuIter;

	_list.clear();
}

void GfxMenu::reset() {
	_list.clear();
	_itemList.clear();

	// We actually set active item in here and remember last selection of the
	// user. Sierra SCI always defaulted to first item every time menu was
	// called via ESC, we don't follow that logic.
	_curMenuId = 1;
	_curItemId = 1;
}

void GfxMenu::kernelAddEntry(Common::String title, Common::String content, reg_t contentVmPtr) {
	GuiMenuEntry *menuEntry;
	uint16 itemCount = 0;
	GuiMenuItemEntry *itemEntry;
	int contentSize = content.size();
	int separatorCount;
	int curPos, beginPos, endPos, tempPos;
	int tagPos, rightAlignedPos, functionPos, altPos, controlPos;
	const char *tempPtr;

	// Sierra SCI starts with id 1, so we do so as well
	menuEntry = new GuiMenuEntry(_list.size() + 1);
	menuEntry->text = title;
	_list.push_back(menuEntry);

	curPos = 0;
	uint16 listSize = _list.size();

	do {
		itemCount++;
		itemEntry = new GuiMenuItemEntry(listSize, itemCount);

		beginPos = curPos;

		// Now go through the content till we find end-marker and collect data about it.
		// ':' is an end-marker for each item.
		tagPos = 0; rightAlignedPos = 0;
		controlPos = 0; altPos = 0; functionPos = 0;
		while ((curPos < contentSize) && (content[curPos] != ':')) {
			switch (content[curPos]) {
			case '=': // Set tag
				// Special case for normal animation speed - they use right
				// aligned "=" for that one, so we ignore it as being recognized
				// as tag marker.
				if (rightAlignedPos == curPos - 1)
					break;
				if (tagPos)
					error("multiple tag markers within one menu-item");
				tagPos = curPos;
				break;
			case '`': // Right-aligned
				if (rightAlignedPos)
					error("multiple right-aligned markers within one menu-item");
				rightAlignedPos = curPos;
				break;
			case '^': // Ctrl-prefix
				if (controlPos)
					error("multiple control markers within one menu-item");
				controlPos = curPos;
				break;
			case '@': // Alt-prefix
				if (altPos)
					error("multiple alt markers within one menu-item");
				altPos = curPos;
				break;
			case '#': // Function-prefix
				if (functionPos)
					error("multiple function markers within one menu-item");
				functionPos = curPos;
				break;
			}
			curPos++;
		}
		endPos = curPos;

		// Control/Alt/Function key mapping...
		if (controlPos) {
			content.setChar(SCI_MENU_REPLACE_ONCONTROL, controlPos);
			itemEntry->keyModifier = SCI_KEYMOD_CTRL;
			tempPos = controlPos + 1;
			if (tempPos >= contentSize)
				error("control marker at end of item");
			itemEntry->keyPress = tolower(content[tempPos]);
			content.setChar(toupper(content[tempPos]), tempPos);
		}
		if (altPos) {
			content.setChar(SCI_MENU_REPLACE_ONALT, altPos);
			itemEntry->keyModifier = SCI_KEYMOD_ALT;
			tempPos = altPos + 1;
			if (tempPos >= contentSize)
				error("alt marker at end of item");
			itemEntry->keyPress = tolower(content[tempPos]);
			content.setChar(toupper(content[tempPos]), tempPos);
		}
		if (functionPos) {
			content.setChar(SCI_MENU_REPLACE_ONFUNCTION, functionPos);
			tempPos = functionPos + 1;
			if (tempPos >= contentSize)
				error("function marker at end of item");
			itemEntry->keyPress = content[tempPos];
			switch (content[functionPos + 1]) {
			case '1': itemEntry->keyPress = SCI_KEY_F1; break;
			case '2': itemEntry->keyPress = SCI_KEY_F2; break;
			case '3': itemEntry->keyPress = SCI_KEY_F3; break;
			case '4': itemEntry->keyPress = SCI_KEY_F4; break;
			case '5': itemEntry->keyPress = SCI_KEY_F5; break;
			case '6': itemEntry->keyPress = SCI_KEY_F6; break;
			case '7': itemEntry->keyPress = SCI_KEY_F7; break;
			case '8': itemEntry->keyPress = SCI_KEY_F8; break;
			case '9': itemEntry->keyPress = SCI_KEY_F9; break;
			case '0': itemEntry->keyPress = SCI_KEY_F10; break;
			default:
				error("illegal function key specified");
			}
		}

		// Now get all strings
		tempPos = endPos;
		if (rightAlignedPos) {
			tempPos = rightAlignedPos;
		} else if (tagPos) {
			tempPos = tagPos;
		}
		curPos = beginPos;
		separatorCount = 0;
		while (curPos < tempPos) {
			switch (content[curPos]) {
			case '!':
			case '-':
			case ' ':
				separatorCount++;
				break;
			case '%':
				// Some multilingual sci01 games use e.g. '--!%G--!' (which doesn't really make sense)
				separatorCount += 2;
				curPos++;
			}
			curPos++;
		}
		if (separatorCount == tempPos - beginPos) {
			itemEntry->separatorLine = true;
		} else {
			// We don't strSplit here, because multilingual SCI01 support
			// language switching on the fly, so we have to do this everytime
			// the menu is called.
			itemEntry->text = Common::String(content.c_str() + beginPos, tempPos - beginPos);

			// LSL6 uses "Ctrl-" prefix string instead of ^ like all the other games do
			tempPtr = itemEntry->text.c_str();
			tempPtr = strstr(tempPtr, "Ctrl-");
			if (tempPtr) {
				itemEntry->keyModifier = SCI_KEYMOD_CTRL;
				itemEntry->keyPress = tolower(tempPtr[5]);
			}
		}
		itemEntry->textVmPtr = contentVmPtr;
		itemEntry->textVmPtr.incOffset(beginPos);

		if (rightAlignedPos) {
			rightAlignedPos++;
			tempPos = endPos;
			// some games have tagPos in front of right rightAlignedPos
			// some have it the other way... (qfg1ega)
			if (tagPos && tagPos >= rightAlignedPos)
				tempPos = tagPos;
			itemEntry->textRightAligned = Common::String(content.c_str() + rightAlignedPos, tempPos - rightAlignedPos);
			// Remove ending space, if there is one. Strangely sometimes there
			// are lone spaces at the end in some games
			if (itemEntry->textRightAligned.hasSuffix(" "))
				itemEntry->textRightAligned.deleteLastChar();
			// - and + are used sometimes for volume control/animation speed,
			// = sometimes for animation speed
			if (itemEntry->textRightAligned == "-") {
				itemEntry->keyPress = '-';
			} else if (itemEntry->textRightAligned == "+") {
				itemEntry->keyPress = '+';
			} else if (itemEntry->textRightAligned == "=") {
				itemEntry->keyPress = '=';
			}
		}

		if (tagPos) {
			tempPos = functionPos + 1;
			if (tempPos >= contentSize)
				error("tag marker at end of item");
			itemEntry->tag = atoi(content.c_str() + tempPos);
		}

		curPos = endPos + 1;

		_itemList.push_back(itemEntry);
	} while (curPos < contentSize);
}

GuiMenuItemEntry *GfxMenu::findItem(uint16 menuId, uint16 itemId) {
	GuiMenuItemList::iterator listIterator;
	GuiMenuItemList::iterator listEnd = _itemList.end();
	GuiMenuItemEntry *listEntry;

	listIterator = _itemList.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		if ((listEntry->menuId == menuId) && (listEntry->id == itemId))
			return listEntry;

		listIterator++;
	}
	return NULL;
}

void GfxMenu::kernelSetAttribute(uint16 menuId, uint16 itemId, uint16 attributeId, reg_t value) {
	GuiMenuItemEntry *itemEntry = findItem(menuId, itemId);

	if (!itemEntry) {
		// PQ2 demo calls this, for example, but has no menus (bug report #3034507). Some SCI
		// fan games (Al Pond 2, Aquarius) call this too on non-existent menu items. The
		// original interpreter ignored these as well.
		debugC(kDebugLevelGraphics, "Tried to setAttribute() on non-existent menu-item %d:%d", menuId, itemId);
		return;
	}

	switch (attributeId) {
	case SCI_MENU_ATTRIBUTE_ENABLED:
		itemEntry->enabled = !value.isNull();
		break;
	case SCI_MENU_ATTRIBUTE_SAID:
		itemEntry->saidVmPtr = value;
		break;
	case SCI_MENU_ATTRIBUTE_TEXT:
		itemEntry->text = _segMan->getString(value);
		itemEntry->textVmPtr = value;
		// We assume here that no script ever creates a separatorLine dynamically
		break;
	case SCI_MENU_ATTRIBUTE_KEYPRESS:
		itemEntry->keyPress = tolower(value.getOffset());
		itemEntry->keyModifier = 0;
		// TODO: Find out how modifier is handled
		debug("setAttr keypress %X %X", value.getSegment(), value.getOffset());
		break;
	case SCI_MENU_ATTRIBUTE_TAG:
		itemEntry->tag = value.getOffset();
		break;
	default:
		// Happens when loading a game in LSL3 - attribute 1A
		warning("setAttribute() called with unsupported attributeId %X", attributeId);
	}
}

reg_t GfxMenu::kernelGetAttribute(uint16 menuId, uint16 itemId, uint16 attributeId) {
	GuiMenuItemEntry *itemEntry = findItem(menuId, itemId);
	if (!itemEntry)
		error("Tried to getAttribute() on non-existant menu-item %d:%d", menuId, itemId);
	switch (attributeId) {
	case SCI_MENU_ATTRIBUTE_ENABLED:
		if (itemEntry->enabled)
			return make_reg(0, 1);
		break;
	case SCI_MENU_ATTRIBUTE_SAID:
		return itemEntry->saidVmPtr;
	case SCI_MENU_ATTRIBUTE_TEXT:
		return itemEntry->textVmPtr;
	case SCI_MENU_ATTRIBUTE_KEYPRESS:
		// TODO: Find out how modifier is handled
		return make_reg(0, itemEntry->keyPress);
	case SCI_MENU_ATTRIBUTE_TAG:
		return make_reg(0, itemEntry->tag);
	default:
		error("getAttribute() called with unsupported attributeId %X", attributeId);
	}
	return NULL_REG;
}

void GfxMenu::drawBar() {
	GuiMenuEntry *listEntry;
	GuiMenuList::iterator listIterator;
	GuiMenuList::iterator listEnd = _list.end();

	// Hardcoded black on white and a black line afterwards
	_paint16->fillRect(_ports->_menuBarRect, 1, _screen->getColorWhite());
	_paint16->fillRect(_ports->_menuLine, 1, 0);
	_ports->penColor(0);
	_ports->moveTo(8, 1);

	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		_text16->DrawString(listEntry->textSplit.c_str());

		listIterator++;
	}
}

// This helper calculates all text widths for all menus (only)
void GfxMenu::calculateMenuWidth() {
	GuiMenuList::iterator menuIterator;
	GuiMenuList::iterator menuEnd = _list.end();
	GuiMenuEntry *menuEntry;
	int16 dummyHeight;

	menuIterator = _list.begin();
	while (menuIterator != menuEnd) {
		menuEntry = *menuIterator;
		menuEntry->textSplit = g_sci->strSplit(menuEntry->text.c_str(), NULL);
		_text16->StringWidth(menuEntry->textSplit.c_str(), 0, menuEntry->textWidth, dummyHeight);

		menuIterator++;
	}
}

// This helper calculates all text widths for all menus/items
void GfxMenu::calculateMenuAndItemWidth() {
	GuiMenuItemList::iterator itemIterator;
	GuiMenuItemList::iterator itemEnd = _itemList.end();
	GuiMenuItemEntry *itemEntry;
	int16 dummyHeight;

	calculateMenuWidth();

	itemIterator = _itemList.begin();
	while (itemIterator != itemEnd) {
		itemEntry = *itemIterator;
		// Split the text now for multilingual SCI01 games
		itemEntry->textSplit = g_sci->strSplit(itemEntry->text.c_str(), NULL);
		_text16->StringWidth(itemEntry->textSplit.c_str(), 0, itemEntry->textWidth, dummyHeight);
		_text16->StringWidth(itemEntry->textRightAligned.c_str(), 0, itemEntry->textRightAlignedWidth, dummyHeight);

		itemIterator++;
	}
}

reg_t GfxMenu::kernelSelect(reg_t eventObject, bool pauseSound) {
	int16 eventType = readSelectorValue(_segMan, eventObject, SELECTOR(type));
	int16 keyPress, keyModifier;
	GuiMenuItemList::iterator itemIterator = _itemList.begin();
	GuiMenuItemList::iterator itemEnd = _itemList.end();
	GuiMenuItemEntry *itemEntry = NULL;
	bool forceClaimed = false;

	switch (eventType) {
	case SCI_EVENT_KEYBOARD:
		keyPress = readSelectorValue(_segMan, eventObject, SELECTOR(message));
		keyModifier = readSelectorValue(_segMan, eventObject, SELECTOR(modifiers));
		// If tab got pressed, handle it here as if it was Ctrl-I - at least
		// sci0 also did it that way
		if (keyPress == SCI_KEY_TAB) {
			keyModifier = SCI_KEYMOD_CTRL;
			keyPress = 'i';
		}
		switch (keyPress) {
		case 0:
			break;
		case SCI_KEY_ESC:
			interactiveStart(pauseSound);
			itemEntry = interactiveWithKeyboard();
			interactiveEnd(pauseSound);
			forceClaimed = true;
			break;
		default:
			while (itemIterator != itemEnd) {
				itemEntry = *itemIterator;
				if (itemEntry->keyPress == keyPress &&
					itemEntry->keyModifier == keyModifier &&
					itemEntry->enabled)
					break;
				itemIterator++;
			}
			if (itemIterator == itemEnd)
				itemEntry = NULL;
		}
		break;

	case SCI_EVENT_SAID:
		while (itemIterator != itemEnd) {
			itemEntry = *itemIterator;

			if (!itemEntry->saidVmPtr.isNull()) {
				byte *saidSpec = _segMan->derefBulkPtr(itemEntry->saidVmPtr, 0);

				if (!saidSpec) {
					warning("Could not dereference saidSpec");
					continue;
				}

				if (said(saidSpec, 0) != SAID_NO_MATCH)
					break;
			}
			itemIterator++;
		}
		if (itemIterator == itemEnd)
			itemEntry = NULL;
		break;

	case SCI_EVENT_MOUSE_PRESS: {
		Common::Point mousePosition;
		mousePosition.x = readSelectorValue(_segMan, eventObject, SELECTOR(x));
		mousePosition.y = readSelectorValue(_segMan, eventObject, SELECTOR(y));
		if (mousePosition.y < 10) {
			interactiveStart(pauseSound);
			itemEntry = interactiveWithMouse();
			interactiveEnd(pauseSound);
			forceClaimed = true;
		}
		} break;
	}

	if (!_menuSaveHandle.isNull()) {
		_paint16->bitsRestore(_menuSaveHandle);
		// Display line inbetween menubar and actual menu
		Common::Rect menuLine = _menuRect;
		menuLine.bottom = menuLine.top + 1;
		_paint16->bitsShow(menuLine);
		_paint16->kernelGraphRedrawBox(_menuRect);
		_menuSaveHandle = NULL_REG;
	}
	if (!_barSaveHandle.isNull()) {
		_paint16->bitsRestore(_barSaveHandle);
		_paint16->bitsShow(_ports->_menuRect);
		_barSaveHandle = NULL_REG;
	}
	if (_oldPort) {
		_ports->setPort(_oldPort);
		_oldPort = NULL;
	}

	if ((itemEntry) || (forceClaimed))
		writeSelector(_segMan, eventObject, SELECTOR(claimed), make_reg(0, 1));
	if (itemEntry)
		return make_reg(0, (itemEntry->menuId << 8) | (itemEntry->id));
	return NULL_REG;
}

GuiMenuItemEntry *GfxMenu::interactiveGetItem(uint16 menuId, uint16 itemId, bool menuChanged) {
	GuiMenuItemList::iterator itemIterator = _itemList.begin();
	GuiMenuItemList::iterator itemEnd = _itemList.end();
	GuiMenuItemEntry *itemEntry;
	GuiMenuItemEntry *firstItemEntry = NULL;
	GuiMenuItemEntry *lastItemEntry = NULL;

	// Fixup menuId if needed
	if (menuId > _list.size())
		menuId = 1;
	if (menuId == 0)
		menuId = _list.size();
	while (itemIterator != itemEnd) {
		itemEntry = *itemIterator;
		if (itemEntry->menuId == menuId) {
			if (itemEntry->id == itemId)
				return itemEntry;
			if (!firstItemEntry)
				firstItemEntry = itemEntry;
			if ((!lastItemEntry) || (itemEntry->id > lastItemEntry->id))
				lastItemEntry = itemEntry;
		}
		itemIterator++;
	}
	if ((itemId == 0) || (menuChanged))
		return lastItemEntry;
	return firstItemEntry;
}

void GfxMenu::drawMenu(uint16 oldMenuId, uint16 newMenuId) {
	GuiMenuEntry *listEntry;
	GuiMenuList::iterator listIterator;
	GuiMenuList::iterator listEnd = _list.end();
	GuiMenuItemEntry *listItemEntry;
	GuiMenuItemList::iterator listItemIterator;
	GuiMenuItemList::iterator listItemEnd = _itemList.end();
	Common::Rect menuTextRect;
	uint16 listNr = 0;
	int16 maxTextWidth = 0, maxTextRightAlignedWidth = 0;
	int16 topPos;
	Common::Point pixelPos;

	// Remove menu, if one is displayed
	if (!_menuSaveHandle.isNull()) {
		_paint16->bitsRestore(_menuSaveHandle);
		// Display line inbetween menubar and actual menu
		Common::Rect menuLine = _menuRect;
		menuLine.bottom = menuLine.top + 1;
		_paint16->bitsShow(menuLine);
		_paint16->kernelGraphRedrawBox(_menuRect);
	}

	// First calculate rect of menu and also invert old and new menu text
	_menuRect.top = _ports->_menuBarRect.bottom;
	menuTextRect.top = _ports->_menuBarRect.top;
	menuTextRect.bottom = _ports->_menuBarRect.bottom;
	menuTextRect.left = menuTextRect.right = 7;
	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		listNr++;
		menuTextRect.left = menuTextRect.right;
		menuTextRect.right += listEntry->textWidth;
		if (listNr == newMenuId)
			_menuRect.left = menuTextRect.left;
		if ((listNr == newMenuId) || (listNr == oldMenuId)) {
			menuTextRect.translate(1, 0);
			_paint16->invertRect(menuTextRect);
			menuTextRect.translate(-1, 0);
		}

		listIterator++;
	}
	_paint16->bitsShow(_ports->_menuBarRect);

	_menuRect.bottom = _menuRect.top + 2;
	listItemIterator = _itemList.begin();
	while (listItemIterator != listItemEnd) {
		listItemEntry = *listItemIterator;
		if (listItemEntry->menuId == newMenuId) {
			_menuRect.bottom += _ports->_curPort->fontHeight;
			maxTextWidth = MAX<int16>(maxTextWidth, listItemEntry->textWidth);
			maxTextRightAlignedWidth = MAX<int16>(maxTextRightAlignedWidth, listItemEntry->textRightAlignedWidth);
		}
		listItemIterator++;
	}
	_menuRect.right = _menuRect.left + 16 + 4 + 2;
	_menuRect.right += maxTextWidth + maxTextRightAlignedWidth;
	if (!maxTextRightAlignedWidth)
		_menuRect.right -= 5;

	// If part of menu window is outside the screen, move it into the screen
	// (this happens in multilingual sq3 and lsl3).
	if (_menuRect.right > _screen->getWidth()) {
		_menuRect.translate(-(_menuRect.right - _screen->getWidth()), 0);
	}

	// Save background
	_menuSaveHandle = _paint16->bitsSave(_menuRect, GFX_SCREEN_MASK_VISUAL);

	// Do the drawing
	_paint16->fillRect(_menuRect, GFX_SCREEN_MASK_VISUAL, 0);
	_menuRect.left++; _menuRect.right--; _menuRect.bottom--;
	_paint16->fillRect(_menuRect, GFX_SCREEN_MASK_VISUAL, _screen->getColorWhite());

	_menuRect.left += 8;
	topPos = _menuRect.top + 1;
	listItemIterator = _itemList.begin();
	while (listItemIterator != listItemEnd) {
		listItemEntry = *listItemIterator;
		if (listItemEntry->menuId == newMenuId) {
			if (!listItemEntry->separatorLine) {
				_ports->textGreyedOutput(!listItemEntry->enabled);
				_ports->moveTo(_menuRect.left, topPos);
				_text16->DrawString(listItemEntry->textSplit.c_str());
				_ports->moveTo(_menuRect.right - listItemEntry->textRightAlignedWidth - 5, topPos);
				_text16->DrawString(listItemEntry->textRightAligned.c_str());
			} else {
				// We dont 100% follow sierra here, we draw the line from left to right. Looks better
				// BTW. SCI1.1 seems to put 2 pixels and then skip one, we don't do this at all (lsl6)
				pixelPos.y = topPos + (_ports->_curPort->fontHeight >> 1) - 1;
				pixelPos.x = _menuRect.left - 7;
				while (pixelPos.x < (_menuRect.right - 1)) {
					_screen->putPixel(pixelPos.x, pixelPos.y, GFX_SCREEN_MASK_VISUAL, 0, 0, 0);
					pixelPos.x += 2;
				}
			}
			topPos += _ports->_curPort->fontHeight;
		}
		listItemIterator++;
	}
	_ports->textGreyedOutput(false);

	// Draw the black line again
	_paint16->fillRect(_ports->_menuLine, 1, 0);

	_menuRect.left -= 8;
	_menuRect.left--; _menuRect.right++; _menuRect.bottom++;
	_paint16->bitsShow(_menuRect);
}

void GfxMenu::invertMenuSelection(uint16 itemId) {
	Common::Rect itemRect = _menuRect;

	if (itemId == 0)
		return;

	itemRect.top += (itemId - 1) * _ports->_curPort->fontHeight + 1;
	itemRect.bottom = itemRect.top + _ports->_curPort->fontHeight;
	itemRect.left++; itemRect.right--;

	_paint16->invertRect(itemRect);
	_paint16->bitsShow(itemRect);
}

void GfxMenu::interactiveStart(bool pauseSound) {
	_mouseOldState = _cursor->isVisible();
	_cursor->kernelShow();
	if (pauseSound)
		g_sci->_soundCmd->pauseAll(true);
}

void GfxMenu::interactiveEnd(bool pauseSound) {
	if (pauseSound)
		g_sci->_soundCmd->pauseAll(false);
	if (!_mouseOldState)
		_cursor->kernelHide();
}

uint16 GfxMenu::mouseFindMenuSelection(Common::Point mousePosition) {
	GuiMenuEntry *listEntry;
	GuiMenuList::iterator listIterator;
	GuiMenuList::iterator listEnd = _list.end();
	uint16 curXstart = 8;

	listIterator = _list.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		if (mousePosition.x >= curXstart && mousePosition.x < curXstart + listEntry->textWidth) {
			return listEntry->id;
		}
		curXstart += listEntry->textWidth;
		listIterator++;
	}
	return 0;
}

uint16 GfxMenu::mouseFindMenuItemSelection(Common::Point mousePosition, uint16 menuId) {
	GuiMenuItemEntry *listItemEntry;
	GuiMenuItemList::iterator listItemIterator;
	GuiMenuItemList::iterator listItemEnd = _itemList.end();
	uint16 curYstart = 10;
	uint16 itemId = 0;

	if (!menuId)
		return 0;

	if ((mousePosition.x < _menuRect.left) || (mousePosition.x >= _menuRect.right))
		return 0;

	listItemIterator = _itemList.begin();
	while (listItemIterator != listItemEnd) {
		listItemEntry = *listItemIterator;
		if (listItemEntry->menuId == menuId) {
			curYstart += _ports->_curPort->fontHeight;
			// Found it
			if ((!itemId) && (curYstart > mousePosition.y))
				itemId = listItemEntry->id;
		}

		listItemIterator++;
	}
	return itemId;
}

GuiMenuItemEntry *GfxMenu::interactiveWithKeyboard() {
	SciEvent curEvent;
	uint16 newMenuId = _curMenuId;
	uint16 newItemId = _curItemId;
	GuiMenuItemEntry *curItemEntry = findItem(_curMenuId, _curItemId);
	GuiMenuItemEntry *newItemEntry = curItemEntry;

	// We don't 100% follow Sierra here: we select last item instead of
	// selecting first item of first menu every time. Also sierra sci didn't
	// allow mouse interaction, when menu was activated via keyboard.

	_oldPort = _ports->setPort(_ports->_menuPort);
	calculateMenuAndItemWidth();
	_barSaveHandle = _paint16->bitsSave(_ports->_menuRect, GFX_SCREEN_MASK_VISUAL);

	_ports->penColor(0);
	_ports->backColor(_screen->getColorWhite());

	drawBar();
	drawMenu(0, curItemEntry->menuId);
	invertMenuSelection(curItemEntry->id);
	_paint16->bitsShow(_ports->_menuRect);
	_paint16->bitsShow(_menuRect);

	while (true) {
		curEvent = _event->getSciEvent(SCI_EVENT_ANY);

		switch (curEvent.type) {
		case SCI_EVENT_KEYBOARD:
			// We don't 100% follow sierra here:
			// - sierra didn't wrap around when changing item id
			// - sierra allowed item id to be 0, which didn't make any sense
			do {
				switch (curEvent.data) {
				case SCI_KEY_ESC:
					_curMenuId = curItemEntry->menuId; _curItemId = curItemEntry->id;
					return NULL;
				case SCI_KEY_ENTER:
					if (curItemEntry->enabled)  {
						_curMenuId = curItemEntry->menuId; _curItemId = curItemEntry->id;
						return curItemEntry;
					}
					break;
				case SCI_KEY_LEFT:
					newMenuId--; newItemId = 1;
					break;
				case SCI_KEY_RIGHT:
					newMenuId++; newItemId = 1;
					break;
				case SCI_KEY_UP:
					newItemId--;
					break;
				case SCI_KEY_DOWN:
					newItemId++;
					break;
				}
				if ((newMenuId != curItemEntry->menuId) || (newItemId != curItemEntry->id)) {
					// Selection changed, fix up new selection if required
					newItemEntry = interactiveGetItem(newMenuId, newItemId, newMenuId != curItemEntry->menuId);
					newMenuId = newItemEntry->menuId; newItemId = newItemEntry->id;

					// if we do this step again because of a separator line -> don't repeat left/right, but go down
					switch (curEvent.data) {
					case SCI_KEY_LEFT:
					case SCI_KEY_RIGHT:
						curEvent.data = SCI_KEY_DOWN;
					}
				}
			} while (newItemEntry->separatorLine);
			if ((newMenuId != curItemEntry->menuId) || (newItemId != curItemEntry->id)) {
				// paint old and new
				if (newMenuId != curItemEntry->menuId) {
					// Menu changed, remove cur menu and paint new menu
					drawMenu(curItemEntry->menuId, newMenuId);
				} else {
					invertMenuSelection(curItemEntry->id);
				}
				invertMenuSelection(newItemId);

				curItemEntry = newItemEntry;
			}
			break;

		case SCI_EVENT_MOUSE_PRESS: {
			Common::Point mousePosition = curEvent.mousePos;
			if (mousePosition.y < 10) {
				// Somewhere on the menubar
				newMenuId = mouseFindMenuSelection(mousePosition);
				if (newMenuId) {
					newItemId = 1;
					newItemEntry = interactiveGetItem(newMenuId, newItemId, newMenuId != curItemEntry->menuId);
					if (newMenuId != curItemEntry->menuId) {
						drawMenu(curItemEntry->menuId, newMenuId);
					} else {
						invertMenuSelection(curItemEntry->id);
					}
					invertMenuSelection(newItemId);
					curItemEntry = newItemEntry;
				} else {
					newMenuId = curItemEntry->menuId;
				}
			} else {
				// Somewhere below menubar
				newItemId = mouseFindMenuItemSelection(mousePosition, newMenuId);
				if (newItemId) {
					newItemEntry = interactiveGetItem(newMenuId, newItemId, false);
					if ((newItemEntry->enabled) && (!newItemEntry->separatorLine)) {
						_curMenuId = newItemEntry->menuId; _curItemId = newItemEntry->id;
						return newItemEntry;
					}
					newItemEntry = curItemEntry;
				}
				newItemId = curItemEntry->id;
			}
			} break;

		case SCI_EVENT_NONE:
			g_sci->sleep(2500 / 1000);
			break;
		}
	}
}

// Mouse button is currently pressed - we are now interpreting mouse coordinates
// till mouse button is released. The menu item that is selected at that time is
// chosen. If no menu item is selected we cancel. No keyboard interaction is
// allowed, cause that wouldnt make any sense at all.
GuiMenuItemEntry *GfxMenu::interactiveWithMouse() {
	SciEvent curEvent;
	uint16 newMenuId = 0, newItemId = 0;
	uint16 curMenuId = 0, curItemId = 0;
	bool firstMenuChange = true;
	GuiMenuItemEntry *curItemEntry = NULL;

	_oldPort = _ports->setPort(_ports->_menuPort);
	calculateMenuAndItemWidth();
	_barSaveHandle = _paint16->bitsSave(_ports->_menuRect, GFX_SCREEN_MASK_VISUAL);

	_ports->penColor(0);
	_ports->backColor(_screen->getColorWhite());

	drawBar();
	_paint16->bitsShow(_ports->_menuRect);

	while (true) {
		curEvent = _event->getSciEvent(SCI_EVENT_ANY);

		switch (curEvent.type) {
		case SCI_EVENT_MOUSE_RELEASE:
			if ((curMenuId == 0) || (curItemId == 0))
				return NULL;
			if ((!curItemEntry->enabled) || (curItemEntry->separatorLine))
				return NULL;
			return curItemEntry;

		case SCI_EVENT_NONE:
			g_sci->sleep(2500 / 1000);
			break;
		}

		// Find out where mouse is currently pointing to
		Common::Point mousePosition = curEvent.mousePos;
		if (mousePosition.y < 10) {
			// Somewhere on the menubar
			newMenuId = mouseFindMenuSelection(mousePosition);
			newItemId = 0;
		} else {
			// Somewhere below menubar
			newItemId = mouseFindMenuItemSelection(mousePosition, newMenuId);
			curItemEntry = interactiveGetItem(curMenuId, newItemId, false);
		}

		if (newMenuId != curMenuId) {
			// Menu changed, remove cur menu and paint new menu
			drawMenu(curMenuId, newMenuId);
			if (firstMenuChange) {
				_paint16->bitsShow(_ports->_menuBarRect);
				firstMenuChange = false;
			}
			curMenuId = newMenuId;
		} else {
			if (newItemId != curItemId) {
				// Item changed
				invertMenuSelection(curItemId);
				invertMenuSelection(newItemId);
				curItemId = newItemId;
			}
		}

	}
	return NULL;
}

void GfxMenu::kernelDrawStatus(const char *text, int16 colorPen, int16 colorBack) {
	Port *oldPort = _ports->setPort(_ports->_menuPort);

	_paint16->fillRect(_ports->_menuBarRect, 1, colorBack);
	_ports->penColor(colorPen);
	_ports->moveTo(0, 1);
	_text16->DrawStatus(text);
	_paint16->bitsShow(_ports->_menuBarRect);
	// Also draw the line under the status bar. Normally, this is never drawn,
	// but we need it to be drawn because Dr. Brain 1 Mac draws over it when
	// it displays the icon bar. SSCI used negative rectangles to erase the
	// area after drawing the icon bar, but this is a much cleaner way of
	// achieving the same effect.
	_paint16->fillRect(_ports->_menuLine, 1, 0);
	_paint16->bitsShow(_ports->_menuLine);
	_ports->setPort(oldPort);
}

void GfxMenu::kernelDrawMenuBar(bool clear) {
	if (!clear) {
		Port *oldPort = _ports->setPort(_ports->_menuPort);
		calculateMenuWidth();
		drawBar();
		_paint16->bitsShow(_ports->_menuBarRect);
		_ports->setPort(oldPort);
	} else {
		kernelDrawStatus("", 0, 0);
	}
}

} // End of namespace Sci
