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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "kyra/gui_hof.h"
#include "kyra/kyra_hof.h"
#include "kyra/timer.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

#include "common/system.h"

#include "graphics/scaler.h"

namespace Kyra {

void KyraEngine_HoF::loadButtonShapes() {
	const uint8 *src = _screen->getCPagePtr(3);
	_screen->loadBitmap("_BUTTONS.CSH", 3, 3, 0);

	_gui->_scrollUpButton.data0ShapePtr = _buttonShapes[0] = _screen->makeShapeCopy(src, 0);
	_gui->_scrollUpButton.data2ShapePtr = _buttonShapes[1] = _screen->makeShapeCopy(src, 1);
	_gui->_scrollUpButton.data1ShapePtr = _buttonShapes[2] = _screen->makeShapeCopy(src, 2);
	_gui->_scrollDownButton.data0ShapePtr = _buttonShapes[3] = _screen->makeShapeCopy(src, 3);
	_gui->_scrollDownButton.data2ShapePtr = _buttonShapes[4] = _screen->makeShapeCopy(src, 4);
	_gui->_scrollDownButton.data1ShapePtr = _buttonShapes[5] = _screen->makeShapeCopy(src, 5);
	_buttonShapes[6] = _screen->makeShapeCopy(src, 6);
	_buttonShapes[7] = _screen->makeShapeCopy(src, 7);
	_buttonShapes[8] = _screen->makeShapeCopy(src, 6);
	_buttonShapes[9] = _screen->makeShapeCopy(src, 7);
	_buttonShapes[10] = _screen->makeShapeCopy(src, 10);
	_buttonShapes[11] = _screen->makeShapeCopy(src, 11);
	_buttonShapes[16] = _screen->makeShapeCopy(src, 16);
	_buttonShapes[17] = _screen->makeShapeCopy(src, 17);
	_buttonShapes[18] = _screen->makeShapeCopy(src, 18);
}

void KyraEngine_HoF::setupLangButtonShapes() {
	switch (_lang) {
	case 0:
		_inventoryButtons[0].data0ShapePtr = _buttonShapes[6];
		_inventoryButtons[0].data1ShapePtr = _inventoryButtons[0].data2ShapePtr = _buttonShapes[7];
		break;

	case 1:
		_inventoryButtons[0].data0ShapePtr = _buttonShapes[8];
		_inventoryButtons[0].data1ShapePtr = _inventoryButtons[0].data2ShapePtr = _buttonShapes[9];
		break;

	case 2:
		_inventoryButtons[0].data0ShapePtr = _buttonShapes[10];
		_inventoryButtons[0].data1ShapePtr = _inventoryButtons[0].data2ShapePtr = _buttonShapes[11];
		break;

	default:
		_inventoryButtons[0].data0ShapePtr = _buttonShapes[6];
		_inventoryButtons[0].data1ShapePtr = _inventoryButtons[0].data2ShapePtr = _buttonShapes[7];
	}
}

GUI_HoF::GUI_HoF(KyraEngine_HoF *vm) : GUI_v2(vm), _vm(vm), _screen(_vm->_screen) {
}

const char *GUI_HoF::getMenuTitle(const Menu &menu) {
	if (!menu.menuNameId)
		return 0;

	return _vm->getTableString(menu.menuNameId, _vm->_optionsBuffer, 1);
}

const char *GUI_HoF::getMenuItemTitle(const MenuItem &menuItem) {
	if (!menuItem.itemId)
		return 0;

	// Strings 41-45 are menu labels, those must be handled uncompressed!
	if (menuItem.itemId >= 41 && menuItem.itemId <= 45)
		return _vm->getTableString(menuItem.itemId, _vm->_optionsBuffer, 0);
	else
		return _vm->getTableString(menuItem.itemId, _vm->_optionsBuffer, 1);
}

const char *GUI_HoF::getMenuItemLabel(const MenuItem &menuItem) {
	if (!menuItem.labelId)
		return 0;

	return _vm->getTableString(menuItem.labelId, _vm->_optionsBuffer, 1);
}

char *GUI_HoF::getTableString(int id) {
	return _vm->getTableString(id, _vm->_optionsBuffer, 0);
}

#pragma mark -


int KyraEngine_HoF::buttonInventory(Button *button) {
	if (!_screen->isMouseVisible())
		return 0;

	int inventorySlot = button->index - 6;

	Item item = _mainCharacter.inventory[inventorySlot];
	if (_itemInHand == kItemNone) {
		if (item == kItemNone)
			return 0;
		clearInventorySlot(inventorySlot, 0);
		snd_playSoundEffect(0x0B);
		setMouseCursor(item);
		int string = (_lang == 1) ? getItemCommandStringPickUp(item) : 7;
		updateCommandLineEx(item+54, string, 0xD6);
		_itemInHand = (int16)item;
		_mainCharacter.inventory[inventorySlot] = kItemNone;
	} else {
		if (_mainCharacter.inventory[inventorySlot] != kItemNone) {
			if (checkInventoryItemExchange(_itemInHand, inventorySlot))
				return 0;

			item = _mainCharacter.inventory[inventorySlot];
			snd_playSoundEffect(0x0B);
			clearInventorySlot(inventorySlot, 0);
			drawInventoryShape(0, _itemInHand, inventorySlot);
			setMouseCursor(item);
			int string = (_lang == 1) ? getItemCommandStringPickUp(item) : 7;
			updateCommandLineEx(item+54, string, 0xD6);
			_mainCharacter.inventory[inventorySlot] = _itemInHand;
			setHandItem(item);
		} else {
			snd_playSoundEffect(0x0C);
			drawInventoryShape(0, _itemInHand, inventorySlot);
			_screen->setMouseCursor(0, 0, getShapePtr(0));
			int string = (_lang == 1) ? getItemCommandStringInv(_itemInHand) : 8;
			updateCommandLineEx(_itemInHand+54, string, 0xD6);
			_mainCharacter.inventory[inventorySlot] = _itemInHand;
			_itemInHand = kItemNone;
		}
	}

	return 0;
}

int KyraEngine_HoF::scrollInventory(Button *button) {
	Item *src = _mainCharacter.inventory;
	Item *dst = &_mainCharacter.inventory[10];
	Item temp[5];

	memcpy(temp, src, sizeof(Item)*5);
	memcpy(src, src+5, sizeof(Item)*5);
	memcpy(src+5, dst, sizeof(Item)*5);
	memcpy(dst, dst+5, sizeof(Item)*5);
	memcpy(dst+5, temp, sizeof(Item)*5);
	_screen->copyRegion(0x46, 0x90, 0x46, 0x90, 0x71, 0x2E, 0, 2);
	redrawInventory(2);
	scrollInventoryWheel();
	return 0;
}

int KyraEngine_HoF::getInventoryItemSlot(Item item) {
	for (int i = 0; i < 20; ++i) {
		if (_mainCharacter.inventory[i] == item)
			return i;
	}
	return -1;
}

int KyraEngine_HoF::findFreeVisibleInventorySlot() {
	for (int i = 0; i < 10; ++i) {
		if (_mainCharacter.inventory[i] == kItemNone)
			return i;
	}
	return -1;
}

void KyraEngine_HoF::removeSlotFromInventory(int slot) {
	_mainCharacter.inventory[slot] = kItemNone;
	if (slot < 10) {
		clearInventorySlot(slot, 0);
	}
}

bool KyraEngine_HoF::checkInventoryItemExchange(Item handItem, int slot) {
	bool removeItem = false;
	Item newItem = kItemNone;

	Item invItem = _mainCharacter.inventory[slot];

	for (const uint16 *table = _itemMagicTable; *table != 0xFFFF; table += 4) {
		if (table[0] != handItem || table[1] != (uint16)invItem)
			continue;

		if (table[3] == 0xFFFF)
			continue;

		removeItem = (table[3] == 1);
		newItem = (Item)table[2];

		snd_playSoundEffect(0x68);
		_mainCharacter.inventory[slot] = newItem;
		clearInventorySlot(slot, 0);
		drawInventoryShape(0, newItem, slot);

		if (removeItem)
			removeHandItem();

		if (_lang != 1)
			updateCommandLineEx(newItem+54, 0x2E, 0xD6);

		return true;
	}

	return false;
}

void KyraEngine_HoF::drawInventoryShape(int page, Item item, int slot) {
	_screen->drawShape(page, getShapePtr(item+64), _inventoryX[slot], _inventoryY[slot], 0, 0);
}

void KyraEngine_HoF::clearInventorySlot(int slot, int page) {
	_screen->drawShape(page, getShapePtr(240+slot), _inventoryX[slot], _inventoryY[slot], 0, 0);
}

void KyraEngine_HoF::redrawInventory(int page) {
	int pageBackUp = _screen->_curPage;
	_screen->_curPage = page;

	const Item *inventory = _mainCharacter.inventory;
	for (int i = 0; i < 10; ++i) {
		clearInventorySlot(i, page);
		if (inventory[i] != kItemNone) {
			_screen->drawShape(page, getShapePtr(inventory[i]+64), _inventoryX[i], _inventoryY[i], 0, 0);
			drawInventoryShape(page, inventory[i], i);
		}
	}
	_screen->updateScreen();

	_screen->_curPage = pageBackUp;
}

void KyraEngine_HoF::scrollInventoryWheel() {
	WSAMovie_v2 movie(this);
	movie.open("INVWHEEL.WSA", 0, 0);
	int frames = movie.opened() ? movie.frames() : 6;
	memcpy(_screenBuffer, _screen->getCPagePtr(2), 64000);
	uint8 overlay[0x100];
	_screen->generateOverlay(_screen->getPalette(0), overlay, 0, 50);
	_screen->copyRegion(0x46, 0x90, 0x46, 0x79, 0x71, 0x17, 0, 2, Screen::CR_NO_P_CHECK);
	snd_playSoundEffect(0x25);

	bool breakFlag = false;
	for (int i = 0; i <= 6 && !breakFlag; ++i) {
		if (movie.opened()) {
			movie.displayFrame(i % frames, 0, 0, 0, 0, 0, 0);
			_screen->updateScreen();
		}

		uint32 endTime = _system->getMillis() + _tickLength;

		int y = (i * 981) >> 8;
		if (y >= 23 || i == 6) {
			y = 23;
			breakFlag = true;
		}

		_screen->applyOverlay(0x46, 0x79, 0x71, 0x17, 2, overlay);
		_screen->copyRegion(0x46, y+0x79, 0x46, 0x90, 0x71, 0x2E, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		delayUntil(endTime);
	}

	_screen->copyBlockToPage(2, 0, 0, 320, 200, _screenBuffer);
	movie.close();
}

// spellbook specific code

int KyraEngine_HoF::bookButton(Button *button) {
	if (!queryGameFlag(1)) {
		objectChat(getTableString(0xEB, _cCodeBuffer, 1), 0, 0x83, 0xEB);
		return 0;
	}

	if (!_screen->isMouseVisible())
		return 0;

	if (queryGameFlag(0xE5)) {
		snd_playSoundEffect(0x0D);
		return 0;
	}

	if (_itemInHand == 72) {
		if (!queryGameFlag(0xE2)) {
			_bookMaxPage += 2;
			removeHandItem();
			snd_playSoundEffect(0x6C);
			setGameFlag(0xE2);
		}

		if (!queryGameFlag(0x18A) && queryGameFlag(0x170)) {
			_bookMaxPage += 2;
			removeHandItem();
			snd_playSoundEffect(0x6C);
			setGameFlag(0x18A);
		}

		return 0;
	}

	if (_mouseState != -1) {
		snd_playSoundEffect(0x0D);
		return 0;
	}

	_screen->hideMouse();
	showMessage(0, 0xCF);
	displayInvWsaLastFrame();
	_bookNewPage = _bookCurPage;

	if (_screenBuffer) {
		memcpy(_screenBuffer, _screen->getCPagePtr(0), 64000);
	}

	_screen->copyPalette(2, 0);
	_screen->fadeToBlack(7, &_updateFunctor);
	_screen->loadPalette("_BOOK.COL", _screen->getPalette(0));
	loadBookBkgd();
	showBookPage();
	_screen->copyRegion(0, 0, 0, 0, 0x140, 0xC8, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	int oldItemInHand = _itemInHand;
	removeHandItem();
	_screen->fadePalette(_screen->getPalette(0), 7);
	_screen->showMouse();

	bookLoop();

	_screen->fadeToBlack(7);
	_screen->hideMouse();
	setHandItem(oldItemInHand);
	updateMouse();
	restorePage3();

	if (_screenBuffer) {
		_screen->copyBlockToPage(0, 0, 0, 320, 200, _screenBuffer);
	}

	setHandItem(_itemInHand);
	_screen->copyPalette(0, 2);
	_screen->fadePalette(_screen->getPalette(0), 7, &_updateFunctor);
	_screen->showMouse();

	if (!queryGameFlag(4) && !queryGameFlag(0xB8)) {
		objectChat(getTableString(0xEC, _cCodeBuffer, 1), 0, 0x83, 0xEC);
		objectChat(getTableString(0xED, _cCodeBuffer, 1), 0, 0x83, 0xED);
		objectChat(getTableString(0xEE, _cCodeBuffer, 1), 0, 0x83, 0xEE);
		objectChat(getTableString(0xEF, _cCodeBuffer, 1), 0, 0x83, 0xEF);
		setGameFlag(4);
	}

	return 0;
}

void KyraEngine_HoF::loadBookBkgd() {
	char filename[16];

	if (_flags.isTalkie)
		strcpy(filename, (_bookBkgd == 0) ? "_XBOOKD.CPS" : "_XBOOKC.CPS");
	else
		strcpy(filename, (_bookBkgd == 0) ? "_BOOKD.CPS" : "_BOOKC.CPS");

	_bookBkgd ^= 1;

	if (_flags.isTalkie) {
		if (!_bookCurPage)
			strcpy(filename, "_XBOOKB.CPS");
		if (_bookCurPage == _bookMaxPage)
			strcpy(filename, "_XBOOKA.CPS");

		switch (_lang) {
		case 0:
			filename[1] = 'E';
			break;

		case 1:
			filename[1] = 'F';
			break;

		case 2:
			filename[1] = 'G';
			break;

		default:
			warning("loadBookBkgd unsupported language");
			filename[1] = 'E';
		}
	} else {
		if (!_bookCurPage)
			strcpy(filename, "_BOOKB.CPS");
		if (_bookCurPage == _bookMaxPage)
			strcpy(filename, "_BOOKA.CPS");
	}

	_screen->loadBitmap(filename, 3, 3, 0);
}

void KyraEngine_HoF::showBookPage() {
	char filename[16];

	sprintf(filename, "PAGE%.01X.%s", _bookCurPage, _languageExtension[_lang]);
	uint8 *leftPage = _res->fileData(filename, 0);
	if (!leftPage) {
		// some floppy version use a TXT extension
		sprintf(filename, "PAGE%.01X.TXT", _bookCurPage);
		leftPage = _res->fileData(filename, 0);
	}

	int leftPageY = _bookPageYOffset[_bookCurPage];

	sprintf(filename, "PAGE%.01X.%s", _bookCurPage+1, _languageExtension[_lang]);
	uint8 *rightPage = 0;
	if (_bookCurPage != _bookMaxPage) {
		rightPage = _res->fileData(filename, 0);
		if (!rightPage) {
			sprintf(filename, "PAGE%.01X.TXT", _bookCurPage);
			rightPage = _res->fileData(filename, 0);
		}
	}

	int rightPageY = _bookPageYOffset[_bookCurPage+1];

	if (leftPage) {
		bookDecodeText(leftPage);
		bookPrintText(2, leftPage, 20, leftPageY+20, 0x31);
		delete[] leftPage;
	}

	if (rightPage) {
		bookDecodeText(rightPage);
		bookPrintText(2, rightPage, 176, rightPageY+20, 0x31);
		delete[] rightPage;
	}
}

void KyraEngine_HoF::bookLoop() {
	Button bookButtons[5];

	GUI_V2_BUTTON(bookButtons[0], 0x24, 0, 0, 1, 1, 1, 0x4487, 0, 0x82, 0xBE, 0x0A, 0x0A, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	bookButtons[0].buttonCallback = BUTTON_FUNCTOR(KyraEngine_HoF, this, &KyraEngine_HoF::bookPrevPage);
	GUI_V2_BUTTON(bookButtons[1], 0x25, 0, 0, 1, 1, 1, 0x4487, 0, 0xB1, 0xBE, 0x0A, 0x0A, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	bookButtons[1].buttonCallback = BUTTON_FUNCTOR(KyraEngine_HoF, this, &KyraEngine_HoF::bookNextPage);
	GUI_V2_BUTTON(bookButtons[2], 0x26, 0, 0, 1, 1, 1, 0x4487, 0, 0x8F, 0xBE, 0x21, 0x0A, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	bookButtons[2].buttonCallback = BUTTON_FUNCTOR(KyraEngine_HoF, this, &KyraEngine_HoF::bookClose);
	GUI_V2_BUTTON(bookButtons[3], 0x27, 0, 0, 1, 1, 1, 0x4487, 0, 0x08, 0x08, 0x90, 0xB4, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	bookButtons[3].buttonCallback = BUTTON_FUNCTOR(KyraEngine_HoF, this, &KyraEngine_HoF::bookPrevPage);
	GUI_V2_BUTTON(bookButtons[4], 0x28, 0, 0, 1, 1, 1, 0x4487, 0, 0xAA, 0x08, 0x8E, 0xB4, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	bookButtons[4].buttonCallback = BUTTON_FUNCTOR(KyraEngine_HoF, this, &KyraEngine_HoF::bookNextPage);

	Button *buttonList = 0;

	for (uint i = 0; i < ARRAYSIZE(bookButtons); ++i)
		buttonList = _gui->addButtonToList(buttonList, &bookButtons[i]);

	showBookPage();
	_bookShown = true;
	while (_bookShown && !shouldQuit()) {
		checkInput(buttonList);
		removeInputTop();

		if (_bookCurPage != _bookNewPage) {
			_bookCurPage = _bookNewPage;
			_screen->clearPage(2);
			loadBookBkgd();
			showBookPage();
			snd_playSoundEffect(0x64);
			_screen->copyRegion(0, 0, 0, 0, 0x140, 0xC8, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
		}
		_system->delayMillis(10);
	}
	_screen->clearPage(2);
}

void KyraEngine_HoF::bookDecodeText(uint8 *str) {
	uint8 *dst = str, *op = str;
	while (*op != 0x1A) {
		while (*op != 0x1A && *op != 0x0D)
			*dst++ = *op++;

		if (*op == 0x1A)
			break;

		op += 2;
		*dst++ = 0x0D;
	}
	*dst = 0;
}

void KyraEngine_HoF::bookPrintText(int dstPage, const uint8 *str, int x, int y, uint8 color) {
	int curPageBackUp = _screen->_curPage;
	_screen->_curPage = dstPage;

	_screen->setTextColor(_bookTextColorMap, 0, 3);
	Screen::FontId oldFont = _screen->setFont(_flags.lang == Common::JA_JPN ? Screen::FID_SJIS_FNT : Screen::FID_BOOKFONT_FNT);
	_screen->_charWidth = -2;

	_screen->printText((const char *)str, x, y, color, (_flags.lang == Common::JA_JPN) ? 0xf6 : 0);

	_screen->_charWidth = 0;
	_screen->setFont(oldFont);
	_screen->_curPage = curPageBackUp;
}

int KyraEngine_HoF::bookPrevPage(Button *button) {
	_bookNewPage = MAX<int>(_bookCurPage-2, 0);
	return 0;
}

int KyraEngine_HoF::bookNextPage(Button *button) {
	_bookNewPage = MIN<int>(_bookCurPage+2, _bookMaxPage);
	return 0;
}

int KyraEngine_HoF::bookClose(Button *button) {
	_bookShown = false;
	return 0;
}

// cauldron specific code

int KyraEngine_HoF::cauldronClearButton(Button *button) {
	if (!queryGameFlag(2)) {
		updateCharFacing();
		objectChat(getTableString(0xF0, _cCodeBuffer, 1), 0, 0x83, 0xF0);
		return 0;
	}

	if (queryGameFlag(0xE4)) {
		snd_playSoundEffect(0x0D);
		return 0;
	}

	_screen->hideMouse();
	displayInvWsaLastFrame();
	snd_playSoundEffect(0x25);
	loadInvWsa("PULL.WSA", 1, 6, 0, -1, -1, 1);
	loadInvWsa("CAULD00.WSA", 1, 7, 0, 0xD4, 0x0F, 1);
	showMessage(0, 0xCF);
	setCauldronState(0, 0);
	clearCauldronTable();
	snd_playSoundEffect(0x57);
	loadInvWsa("CAULDFIL.WSA", 1, 7, 0, -1, -1, 1);
	_screen->showMouse();
	return 0;
}

int KyraEngine_HoF::cauldronButton(Button *button) {
	if (!queryGameFlag(2)) {
		objectChat(getTableString(0xF0, _cCodeBuffer, 1), 0, 0x83, 0xF0);
		return 0;
	}

	if (!_screen->isMouseVisible() || _mouseState < -1)
		return 0;

	if (queryGameFlag(0xE4)) {
		snd_playSoundEffect(0x0D);
		return 0;
	}

	updateCharFacing();

	for (int i = 0; _cauldronProtectedItems[i] != -1; ++i) {
		if (_itemInHand == _cauldronProtectedItems[i]) {
			objectChat(getTableString(0xF1, _cCodeBuffer, 1), 0, 0x83, 0xF1);
			return 0;
		}
	}

	if (_itemInHand == -1) {
		listItemsInCauldron();
		return 0;
	}

	for (int i = 0; _cauldronBowlTable[i] != -1; i += 2) {
		if (_itemInHand == _cauldronBowlTable[i]) {
			addFrontCauldronTable(_itemInHand);
			setHandItem(_cauldronBowlTable[i+1]);
			if (!updateCauldron()) {
				_cauldronState = 0;
				cauldronRndPaletteFade();
			}
			return 0;
		}
	}

	if (_itemInHand == 18) {
		const int16 *magicTable = (_mainCharacter.sceneId == 77) ? _cauldronMagicTableScene77 : _cauldronMagicTable;
		while (magicTable[0] != -1) {
			if (_cauldronState == magicTable[0]) {
				setHandItem(magicTable[1]);
				snd_playSoundEffect(0x6C);
				++_cauldronUseCount;
				if (_cauldronStateTable[_cauldronState] <= _cauldronUseCount && _cauldronUseCount) {
					showMessage(0, 0xCF);
					setCauldronState(0, true);
					clearCauldronTable();
				}
				return 0;
			}
			magicTable += 2;
		}
	} else if (_itemInHand >= 0) {
		int item = _itemInHand;
		cauldronItemAnim(item);
		addFrontCauldronTable(item);
		if (!updateCauldron()) {
			_cauldronState = 0;
			cauldronRndPaletteFade();
		}
	}

	return 0;
}

#pragma mark -

int GUI_HoF::optionsButton(Button *button) {
	PauseTimer pause(*_vm->_timer);

	_restartGame = false;
	_reloadTemporarySave = false;

	updateButton(&_vm->_inventoryButtons[0]);

	if (!_screen->isMouseVisible() && button)
		return 0;

	_vm->showMessage(0, 0xCF);

	if (_vm->_mouseState < -1) {
		_vm->_mouseState = -1;
		_screen->setMouseCursor(1, 1, _vm->getShapePtr(0));
		return 0;
	}

	int oldHandItem = _vm->_itemInHand;
	_screen->setMouseCursor(0, 0, _vm->getShapePtr(0));
	_vm->displayInvWsaLastFrame();
	_displayMenu = true;

	for (uint i = 0; i < ARRAYSIZE(_menuButtons); ++i) {
		_menuButtons[i].data0Val1 = _menuButtons[i].data1Val1 = _menuButtons[i].data2Val1 = 4;
		_menuButtons[i].data0Callback = _redrawShadedButtonFunctor;
		_menuButtons[i].data1Callback = _menuButtons[i].data2Callback = _redrawButtonFunctor;
	}

	initMenuLayout(_mainMenu);
	initMenuLayout(_gameOptions);
	initMenuLayout(_audioOptions);
	initMenuLayout(_choiceMenu);
	_loadMenu.numberOfItems = 6;
	initMenuLayout(_loadMenu);
	initMenuLayout(_saveMenu);
	initMenuLayout(_savenameMenu);
	initMenuLayout(_deathMenu);

	_currentMenu = &_mainMenu;

	if (_vm->_menuDirectlyToLoad) {
		backUpPage1(_vm->_screenBuffer);
		setupPalette();

		_loadedSave = false;

		loadMenu(0);

		if (_loadedSave) {
			if (_restartGame)
				_vm->_itemInHand = kItemNone;
		} else {
			restorePage1(_vm->_screenBuffer);
			restorePalette();
		}

		resetState(-1);
		_vm->_menuDirectlyToLoad = false;
		return 0;
	}

	if (!button) {
		_currentMenu = &_deathMenu;
		_isDeathMenu = true;
	} else {
		_isDeathMenu = false;
	}

	backUpPage1(_vm->_screenBuffer);
	setupPalette();
	initMenu(*_currentMenu);
	_madeSave = false;
	_loadedSave = false;
	updateAllMenuButtons();

	if (_isDeathMenu) {
		while (!_screen->isMouseVisible())
			_screen->showMouse();
	}

	while (_displayMenu) {
		processHighlights(*_currentMenu);
		getInput();
	}

	if (_vm->_runFlag && !_loadedSave && !_madeSave) {
		restorePalette();
		restorePage1(_vm->_screenBuffer);
	}

	if (_vm->_runFlag)
		updateMenuButton(&_vm->_inventoryButtons[0]);

	resetState(oldHandItem);

	if (!_loadedSave && _reloadTemporarySave) {
		_vm->_unkSceneScreenFlag1 = true;
		_vm->loadGameStateCheck(999);
		//_vm->_saveFileMan->removeSavefile(_vm->getSavegameFilename(999));
		_vm->_unkSceneScreenFlag1 = false;
	}

	return 0;
}

#pragma mark -

void GUI_HoF::createScreenThumbnail(Graphics::Surface &dst) {
	uint8 screenPal[768];
	_screen->getRealPalette(1, screenPal);
	::createThumbnail(&dst, _vm->_screenBuffer, Screen::SCREEN_W, Screen::SCREEN_H, screenPal);
}

void GUI_HoF::setupPalette() {
	_screen->copyPalette(1, 0);

	Palette &pal = _screen->getPalette(0);
	for (int i = 0; i < 741; ++i)
		pal[i] >>= 1;

	if (_isDeathMenu)
		_screen->fadePalette(_screen->getPalette(0), 0x64);
	else
		_screen->setScreenPalette(_screen->getPalette(0));
}

void GUI_HoF::restorePalette() {
	_screen->copyPalette(0, 1);
	_screen->setScreenPalette(_screen->getPalette(0));
}

void GUI_HoF::resetState(int item) {
	_vm->_timer->resetNextRun();
	_vm->setNextIdleAnimTimer();
	_isDeathMenu = false;
	if (!_loadedSave) {
		_vm->_itemInHand = kItemNone;
		_vm->setHandItem(item);
	} else {
		_vm->setHandItem(_vm->_itemInHand);
		_vm->setTimer1DelaySecs(7);
		_vm->_shownMessage = " ";
		_vm->_fadeMessagePalette = false;
	}
	_buttonListChanged = true;
}

void GUI_HoF::drawSliderBar(int slider, const uint8 *shape) {
	const int menuX = _audioOptions.x;
	const int menuY = _audioOptions.y;
	int x = menuX + _sliderBarsPosition[slider*2+0] + 10;
	int y = menuY + _sliderBarsPosition[slider*2+1];

	int position = 0;
	if (_vm->gameFlags().isTalkie) {
		position = _vm->getVolume(KyraEngine_v1::kVolumeEntry(slider));
	} else {
		if (slider < 2)
			position = _vm->getVolume(KyraEngine_v1::kVolumeEntry(slider));
		else if (slider == 2)
			position = (_vm->_configWalkspeed == 3) ? 97 : 2;
		else if (slider == 3)
			position = _vm->_configTextspeed;
	}

	position = CLIP(position, 2, 97);
	_screen->drawShape(0, shape, x+position, y, 0, 0);
}

#pragma mark -

int GUI_HoF::quitGame(Button *caller) {
	updateMenuButton(caller);
	if (choiceDialog(_vm->gameFlags().isTalkie ? 0xF : 0x17, 1)) {
		_displayMenu = false;
		_vm->_runFlag = false;
		_vm->_sound->beginFadeOut();
		_screen->fadeToBlack();
		_screen->clearCurPage();
	}

	if (_vm->_runFlag) {
		initMenu(*_currentMenu);
		updateAllMenuButtons();
	}

	return 0;
}

int GUI_HoF::audioOptions(Button *caller) {
	updateMenuButton(caller);
	restorePage1(_vm->_screenBuffer);
	backUpPage1(_vm->_screenBuffer);
	initMenu(_audioOptions);
	const int menuX = _audioOptions.x;
	const int menuY = _audioOptions.y;
	const int maxButton = 3;	// 2 if voc is disabled

	for (int i = 0; i < maxButton; ++i) {
		int x = menuX + _sliderBarsPosition[i*2+0];
		int y = menuY + _sliderBarsPosition[i*2+1];
		_screen->drawShape(0, _vm->_buttonShapes[16], x, y, 0, 0);
		drawSliderBar(i, _vm->_buttonShapes[17]);
		_sliderButtons[0][i].buttonCallback = _sliderHandlerFunctor;
		_sliderButtons[0][i].x = x;
		_sliderButtons[0][i].y = y;
		_menuButtonList = addButtonToList(_menuButtonList, &_sliderButtons[0][i]);
		_sliderButtons[2][i].buttonCallback = _sliderHandlerFunctor;
		_sliderButtons[2][i].x = x + 10;
		_sliderButtons[2][i].y = y;
		_menuButtonList = addButtonToList(_menuButtonList, &_sliderButtons[2][i]);
		_sliderButtons[1][i].buttonCallback = _sliderHandlerFunctor;
		_sliderButtons[1][i].x = x + 120;
		_sliderButtons[1][i].y = y;
		_menuButtonList = addButtonToList(_menuButtonList, &_sliderButtons[1][i]);
	}

	_isOptionsMenu = true;
	updateAllMenuButtons();
	bool speechEnabled = _vm->speechEnabled();
	while (_isOptionsMenu) {
		processHighlights(_audioOptions);
		getInput();
	}

	restorePage1(_vm->_screenBuffer);
	backUpPage1(_vm->_screenBuffer);
	if (speechEnabled && !_vm->textEnabled() && (!_vm->speechEnabled() || _vm->getVolume(KyraEngine_v1::kVolumeSpeech) == 2)) {
		_vm->_configVoice = 0;
		choiceDialog(0x1D, 0);
	}

	_vm->writeSettings();

	initMenu(*_currentMenu);
	updateAllMenuButtons();
	return 0;
}

int GUI_HoF::gameOptions(Button *caller) {
	updateMenuButton(caller);
	restorePage1(_vm->_screenBuffer);
	backUpPage1(_vm->_screenBuffer);
	initMenu(_gameOptions);
	_isOptionsMenu = true;

	const int menuX = _gameOptions.x;
	const int menuY = _gameOptions.y;

	for (int i = 0; i < 4; ++i) {
		int x = menuX + _sliderBarsPosition[i*2+0];
		int y = menuY + _sliderBarsPosition[i*2+1];
		_screen->drawShape(0, _vm->_buttonShapes[16], x, y, 0, 0);
		drawSliderBar(i, _vm->_buttonShapes[17]);
		_sliderButtons[0][i].buttonCallback = _sliderHandlerFunctor;
		_sliderButtons[0][i].x = x;
		_sliderButtons[0][i].y = y;
		_menuButtonList = addButtonToList(_menuButtonList, &_sliderButtons[0][i]);
		_sliderButtons[2][i].buttonCallback = _sliderHandlerFunctor;
		_sliderButtons[2][i].x = x + 10;
		_sliderButtons[2][i].y = y;
		_menuButtonList = addButtonToList(_menuButtonList, &_sliderButtons[2][i]);
		_sliderButtons[1][i].buttonCallback = _sliderHandlerFunctor;
		_sliderButtons[1][i].x = x + 120;
		_sliderButtons[1][i].y = y;
		_menuButtonList = addButtonToList(_menuButtonList, &_sliderButtons[1][i]);
	}

	while (_isOptionsMenu) {
		processHighlights(_gameOptions);
		getInput();
	}

	restorePage1(_vm->_screenBuffer);
	backUpPage1(_vm->_screenBuffer);

	_vm->writeSettings();

	initMenu(*_currentMenu);
	updateAllMenuButtons();

	return 0;
}

int GUI_HoF::gameOptionsTalkie(Button *caller) {
	updateMenuButton(caller);
	restorePage1(_vm->_screenBuffer);
	backUpPage1(_vm->_screenBuffer);
	bool textEnabled = _vm->textEnabled();
	int lang = _vm->_lang;

	setupOptionsButtons();
	initMenu(_gameOptions);
	_isOptionsMenu = true;

	while (_isOptionsMenu) {
		processHighlights(_gameOptions);
		getInput();
	}

	restorePage1(_vm->_screenBuffer);
	backUpPage1(_vm->_screenBuffer);

	if (textEnabled && !_vm->textEnabled() && !_vm->speechEnabled()) {
		_vm->_configVoice = 1;
		_vm->setVolume(KyraEngine_v1::kVolumeSpeech, 75);
		choiceDialog(0x1E, 0);
	}

	if (_vm->_lang != lang) {
		_reloadTemporarySave = true;

		Graphics::Surface thumb;
		createScreenThumbnail(thumb);
		_vm->saveGameStateIntern(999, "Autosave", &thumb);
		thumb.free();

		_vm->_lastAutosave = _vm->_system->getMillis();

		_vm->loadCCodeBuffer("C_CODE.XXX");
		if (_vm->_flags.isTalkie)
			_vm->loadOptionsBuffer("OPTIONS.XXX");
		else
			_vm->_optionsBuffer = _vm->_cCodeBuffer;
		_vm->loadChapterBuffer(_vm->_newChapterFile);
		_vm->loadNPCScript();
		_vm->setupLangButtonShapes();
	}

	_vm->writeSettings();

	initMenu(*_currentMenu);
	updateAllMenuButtons();
	return 0;
}

int GUI_HoF::changeLanguage(Button *caller) {
	updateMenuButton(caller);
	++_vm->_lang;
	_vm->_lang %= 3;
	setupOptionsButtons();
	renewHighlight(_gameOptions);
	return 0;
}

void GUI_HoF::setupOptionsButtons() {
	if (_vm->_configWalkspeed == 3)
		_gameOptions.item[0].itemId = 28;
	else
		_gameOptions.item[0].itemId = 27;

	if (_vm->textEnabled())
		_gameOptions.item[2].itemId = 18;
	else
		_gameOptions.item[2].itemId = 17;

	switch (_vm->_lang) {
	case 0:
		_gameOptions.item[1].itemId = 31;
		break;

	case 1:
		_gameOptions.item[1].itemId = 32;
		break;

	case 2:
		_gameOptions.item[1].itemId = 33;
		break;

	default:
		break;
	}
}

int GUI_HoF::sliderHandler(Button *caller) {
	int button = 0;
	if (caller->index >= 24 && caller->index <= 27)
		button = caller->index - 24;
	else if (caller->index >= 28 && caller->index <= 31)
		button = caller->index - 28;
	else
		button = caller->index - 32;

	assert(button >= 0 && button <= 3);

	int oldVolume = 0;

	if (_vm->gameFlags().isTalkie) {
		oldVolume = _vm->getVolume(KyraEngine_v1::kVolumeEntry(button));
	} else {
		if (button < 2)
			oldVolume = _vm->getVolume(KyraEngine_v1::kVolumeEntry(button));
		else if (button == 2)
			oldVolume = (_vm->_configWalkspeed == 3) ? 97 : 2;
		else if (button == 3)
			oldVolume = _vm->_configTextspeed;
	}

	int newVolume = oldVolume;

	if (caller->index >= 24 && caller->index <= 27)
		newVolume -= 10;
	else if (caller->index >= 28 && caller->index <= 31)
		newVolume += 10;
	else
		newVolume = _vm->_mouseX - caller->x - 7;

	newVolume = CLIP(newVolume, 2, 97);

	if (newVolume == oldVolume)
		return 0;

	int lastMusicCommand = -1;
	bool playSoundEffect = false;

	drawSliderBar(button, _vm->_buttonShapes[18]);

	if (_vm->gameFlags().isTalkie) {
		if (button == 2) {
			if (_vm->textEnabled())
				_vm->_configVoice = 2;
			else
				_vm->_configVoice = 1;
		}

		_vm->setVolume(KyraEngine_v1::kVolumeEntry(button), newVolume);

		switch (button) {
		case 0:
			lastMusicCommand = _vm->_lastMusicCommand;
			break;

		case 1:
			playSoundEffect = true;
			break;

		case 2:
			_vm->playVoice(90, 28);
			break;

		default:
			return 0;
		}
	} else {
		if (button < 2) {
			_vm->setVolume(KyraEngine_v1::kVolumeEntry(button), newVolume);
			if (button == 0)
				lastMusicCommand = _vm->_lastMusicCommand;
			else
				playSoundEffect = true;
		} else if (button == 2) {
			_vm->_configWalkspeed = (newVolume > 48) ? 3 : 5;
			_vm->setWalkspeed(_vm->_configWalkspeed);
		} else if (button == 3) {
			_vm->_configTextspeed = newVolume;
		}
	}

	drawSliderBar(button, _vm->_buttonShapes[17]);
	if (playSoundEffect)
		_vm->snd_playSoundEffect(0x18);
	else if (lastMusicCommand >= 0)
		_vm->snd_playWanderScoreViaMap(lastMusicCommand, 0);

	_screen->updateScreen();
	return 0;
}

int GUI_HoF::loadMenu(Button *caller) {
	updateSaveFileList(_vm->_targetName);

	if (!_vm->_menuDirectlyToLoad) {
		updateMenuButton(caller);
		restorePage1(_vm->_screenBuffer);
		backUpPage1(_vm->_screenBuffer);
	}

	_savegameOffset = 0;
	setupSavegameNames(_loadMenu, 5);
	initMenu(_loadMenu);
	_isLoadMenu = true;
	_noLoadProcess = false;
	_vm->_gameToLoad = -1;
	updateAllMenuButtons();

	_screen->updateScreen();
	while (_isLoadMenu) {
		processHighlights(_loadMenu);
		getInput();
	}

	if (_noLoadProcess) {
		if (!_vm->_menuDirectlyToLoad) {
			restorePage1(_vm->_screenBuffer);
			backUpPage1(_vm->_screenBuffer);
			initMenu(*_currentMenu);
			updateAllMenuButtons();
		}
	} else if (_vm->_gameToLoad >= 0) {
		restorePage1(_vm->_screenBuffer);
		restorePalette();
		_vm->loadGameStateCheck(_vm->_gameToLoad);
		if (_vm->_gameToLoad == 0) {
			_restartGame = true;
			for (int i = 0; i < 23; ++i)
				_vm->resetCauldronStateTable(i);
			_vm->runStartScript(1, 1);
		}
		_displayMenu = false;
		_loadedSave = true;
	}

	return 0;
}

} // End of namespace Kyra
