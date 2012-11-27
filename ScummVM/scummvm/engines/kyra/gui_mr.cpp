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

#include "kyra/gui_mr.h"
#include "kyra/kyra_mr.h"
#include "kyra/text_mr.h"
#include "kyra/resource.h"
#include "kyra/timer.h"
#include "kyra/sound_digital.h"

#include "common/system.h"

#include "graphics/scaler.h"

namespace Kyra {

void KyraEngine_MR::loadButtonShapes() {
	_res->exists("BUTTONS.SHP", true);
	uint8 *data = _res->fileData("BUTTONS.SHP", 0);
	assert(data);
	for (int i = 0; i <= 10; ++i)
		addShapeToPool(data, 0x1C7+i, i);
	delete[] data;

	Button::Callback callback1 = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::callbackButton1);
	Button::Callback callback2 = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::callbackButton2);
	Button::Callback callback3 = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::callbackButton3);

	_gui->getScrollUpButton()->data0Callback = callback1;
	_gui->getScrollUpButton()->data1Callback = callback2;
	_gui->getScrollUpButton()->data2Callback = callback3;
	_gui->getScrollDownButton()->data0Callback = callback1;
	_gui->getScrollDownButton()->data1Callback = callback2;
	_gui->getScrollDownButton()->data2Callback = callback3;

	_mainButtonData[0].data0Callback = callback1;
	_mainButtonData[0].data1Callback = callback2;
	_mainButtonData[0].data2Callback = callback3;
}

int KyraEngine_MR::callbackButton1(Button *button) {
	const uint8 *shapePtr = 0;
	if (button->index == 1)
		shapePtr = getShapePtr(0x1CD);
	else if (button->index == 22)
		shapePtr = getShapePtr(0x1C7);
	else if (button->index == 23)
		shapePtr = getShapePtr(0x1CA);

	if (shapePtr)
		_screen->drawShape(0, shapePtr, button->x, button->y, 0, 0, 0);

	return 0;
}

int KyraEngine_MR::callbackButton2(Button *button) {
	const uint8 *shapePtr = 0;
	if (button->index == 1)
		shapePtr = getShapePtr(0x1CE);
	else if (button->index == 22)
		shapePtr = getShapePtr(0x1C9);
	else if (button->index == 23)
		shapePtr = getShapePtr(0x1CC);

	if (shapePtr)
		_screen->drawShape(0, shapePtr, button->x, button->y, 0, 0, 0);

	return 0;
}

int KyraEngine_MR::callbackButton3(Button *button) {
	const uint8 *shapePtr = 0;
	if (button->index == 1)
		shapePtr = getShapePtr(0x1CE);
	else if (button->index == 22)
		shapePtr = getShapePtr(0x1C8);
	else if (button->index == 23)
		shapePtr = getShapePtr(0x1CB);

	if (shapePtr)
		_screen->drawShape(0, shapePtr, button->x, button->y, 0, 0, 0);

	return 0;
}

void KyraEngine_MR::showMessage(const char *string, uint8 c0, uint8 c1) {
	_shownMessage = string;

	restoreCommandLine();
	_restoreCommandLine = false;

	if (string) {
		int x = _text->getCenterStringX(string, 0, 320);
		int pageBackUp = _screen->_curPage;
		_screen->_curPage = 0;
		_text->printText(string, x, _commandLineY, c0, c1, 0);
		_screen->_curPage = pageBackUp;
		_screen->updateScreen();
		setCommandLineRestoreTimer(7);
	}
}

void KyraEngine_MR::showMessageFromCCode(int string, uint8 c0, int) {
	showMessage((const char *)getTableEntry(_cCodeFile, string), c0, 0xF0);
}

void KyraEngine_MR::updateItemCommand(Item item, int str, uint8 c0) {
	char buffer[100];
	char *src = (char *)getTableEntry(_itemFile, item);

	while (*src != ' ')
		++src;
	++src;

	*src = toupper(*src);

	strcpy(buffer, src);
	strcat(buffer, " ");
	strcat(buffer, (const char *)getTableEntry(_cCodeFile, str));

	showMessage(buffer, c0, 0xF0);
}

void KyraEngine_MR::updateCommandLine() {
	if (_restoreCommandLine) {
		restoreCommandLine();
		_restoreCommandLine = false;
	}
}

void KyraEngine_MR::restoreCommandLine() {
	int y = _inventoryState ? 144 : 188;
	_screen->copyBlockToPage(0, 0, y, 320, 12, _interfaceCommandLine);
}

void KyraEngine_MR::updateCLState() {
	if (_inventoryState)
		_commandLineY = 145;
	else
		_commandLineY = 189;
}

void KyraEngine_MR::showInventory() {
	if (!_screen->isMouseVisible())
		return;
	if (queryGameFlag(3))
		return;

	_screen->copyBlockToPage(3, 0, 0, 320, 56, _interface);
	drawMalcolmsMoodText();

	_inventoryState = true;
	updateCLState();

	redrawInventory(30);
	drawMalcolmsMoodPointer(-1, 30);
	drawScore(30, 215, 191);

	if (queryGameFlag(0x97))
		drawJestersStaff(1, 30);

	_screen->hideMouse();

	if (_itemInHand < 0) {
		_mouseState = -1;
		_screen->setMouseCursor(0, 0, getShapePtr(0));
	}

	_screen->copyRegion(0, 188, 0, 0, 320, 12, 0, 2, Screen::CR_NO_P_CHECK);

	if (_inventoryScrollSpeed == -1) {
		uint32 endTime = _system->getMillis() + _tickLength * 15;
		int times = 0;
		while (_system->getMillis() < endTime) {
			_screen->copyRegion(0, 188, 0, 0, 320, 12, 0, 2, Screen::CR_NO_P_CHECK);
			_screen->copyRegion(0, 188, 0, 0, 320, 12, 0, 2, Screen::CR_NO_P_CHECK);
			++times;
		}

		times = MAX(times, 1);

		int speed = 60 / times;
		if (speed <= 1)
			_inventoryScrollSpeed = 1;
		else if (speed >= 8)
			_inventoryScrollSpeed = 8;
		else
			_inventoryScrollSpeed = speed;
	}

	int height = 12;
	int y = 188;
	int times = 0;
	uint32 waitTill = _system->getMillis() + _tickLength;

	while (y > 144) {
		_screen->copyRegion(0, 0, 0, y, 320, height, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		++times;
		if (_inventoryScrollSpeed == 1 && times == 3) {
			while (waitTill > _system->getMillis())
				_system->delayMillis(10);
			times = 0;
			waitTill = _system->getMillis() + _tickLength;
		}

		height += _inventoryScrollSpeed;
		y -= _inventoryScrollSpeed;
	}

	_screen->copyRegion(0, 0, 0, 144, 320, 56, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	initMainButtonList(false);

	restorePage3();
	_screen->showMouse();
}

void KyraEngine_MR::hideInventory() {
	if (queryGameFlag(3))
		return;

	_inventoryState = false;
	updateCLState();
	initMainButtonList(true);

	_screen->copyBlockToPage(3, 0, 0, 320, 56, _interface);
	_screen->hideMouse();

	restorePage3();
	flagAnimObjsForRefresh();
	drawAnimObjects();
	_screen->copyRegion(0, 144, 0, 0, 320, 56, 0, 2, Screen::CR_NO_P_CHECK);

	if (_inventoryScrollSpeed == -1) {
		uint32 endTime = _system->getMillis() + _tickLength * 15;
		int times = 0;
		while (_system->getMillis() < endTime) {
			_screen->copyRegion(0, 144, 0, 0, 320, 12, 0, 2, Screen::CR_NO_P_CHECK);
			_screen->copyRegion(0, 144, 0, 0, 320, 12, 0, 2, Screen::CR_NO_P_CHECK);
			++times;
		}

		times = MAX(times, 1);

		int speed = 60 / times;
		if (speed <= 1)
			_inventoryScrollSpeed = 1;
		else if (speed >= 8)
			_inventoryScrollSpeed = 8;
		else
			_inventoryScrollSpeed = speed;
	}

	int y = 144;
	int y2 = 144 + _inventoryScrollSpeed;
	uint32 waitTill = _system->getMillis() + _tickLength;
	int times = 0;

	while (y2 < 188) {
		_screen->copyRegion(0, 0, 0, y2, 320, 56, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(0, y, 0, y, 320, _inventoryScrollSpeed, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		++times;
		if (_inventoryScrollSpeed == 1 && times == 3) {
			while (waitTill > _system->getMillis())
				_system->delayMillis(10);
			times = 0;
			waitTill = _system->getMillis() + _tickLength;
		}

		y += _inventoryScrollSpeed;
		y2 += _inventoryScrollSpeed;
	}

	_screen->copyRegion(0, 0, 0, 188, 320, 56, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(0, y, 0, y, 320, 188-y, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->showMouse();
}

void KyraEngine_MR::drawMalcolmsMoodText() {
	static const int stringId[] = { 0x32, 0x37, 0x3C };

	if (queryGameFlag(0x219))
		return;

	const char *string = (const char *)getTableEntry(_cCodeFile, stringId[_malcolmsMood]);

	Screen::FontId oldFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;

	int width = _screen->getTextWidth(string);

	_screen->_charWidth = 0;
	_screen->setFont(oldFont);

	int pageBackUp = _screen->_curPage;
	const int x = 280 - (width / 2);
	int y = 0;
	if (_inventoryState) {
		y = 189;
		_screen->_curPage = 0;
	} else {
		y = 45;
		_screen->_curPage = 2;
	}

	_screen->drawShape(_screen->_curPage, getShapePtr(432), 244, 189, 0, 0);
	_text->printText(string, x, y+1, 0xFF, 0xF0, 0x00);
	_screen->_curPage = pageBackUp;
}

void KyraEngine_MR::drawMalcolmsMoodPointer(int frame, int page) {
	static const uint8 stateTable[] = {
		1, 6, 11
	};

	if (frame == -1)
		frame = stateTable[_malcolmsMood];
	if (queryGameFlag(0x219))
		frame = 13;

	if (page == 0) {
		_invWsa->displayFrame(frame, 0, 0, 0, 0, 0, 0);
		_screen->updateScreen();
	} else if (page == 30) {
		_invWsa->displayFrame(frame, 2, 0, -144, 0, 0, 0);
	}

	_invWsaFrame = frame;
}

void KyraEngine_MR::drawJestersStaff(int type, int page) {
	int y = 155;
	if (page == 30) {
		page = 2;
		y -= 144;
	}

	int shape = (type != 0) ? 454 : 453;
	_screen->drawShape(page, getShapePtr(shape), 217, y, 0, 0);
}

void KyraEngine_MR::drawScore(int page, int x, int y) {
	if (page == 30) {
		page = 2;
		y -= 144;
	}

	int shape1 = _score / 100;
	int shape2 = (_score - shape1*100) / 10;
	int shape3 = _score % 10;

	_screen->drawShape(page, getShapePtr(shape1+433), x, y, 0, 0);
	x += 8;
	_screen->drawShape(page, getShapePtr(shape2+433), x, y, 0, 0);
	x += 8;
	_screen->drawShape(page, getShapePtr(shape3+433), x, y, 0, 0);
}

void KyraEngine_MR::drawScoreCounting(int oldScore, int newScore, int drawOld, const int x) {
	int y = 189;
	if (_inventoryState)
		y -= 44;

	int old100 = oldScore / 100;
	int old010 = (oldScore - old100*100) / 10;
	int old001 = oldScore % 10;

	int new100 = newScore / 100;
	int new010 = (newScore - new100*100) / 10;
	int new001 = newScore % 10;

	if (drawOld) {
		_screen->drawShape(0, getShapePtr(old100+433), x +  0, y, 0, 0);
		_screen->drawShape(0, getShapePtr(old010+433), x +  8, y, 0, 0);
		_screen->drawShape(0, getShapePtr(old001+433), x + 16, y, 0, 0);
	}

	if (old100 != new100)
		_screen->drawShape(0, getShapePtr(old100+443), x +  0, y, 0, 0);

	if (old010 != new010)
		_screen->drawShape(0, getShapePtr(old010+443), x +  8, y, 0, 0);

	_screen->drawShape(0, getShapePtr(old001+443), x + 16, y, 0, 0);

	_screen->updateScreen();

	_screen->drawShape(0, getShapePtr(new100+433), x +  0, y, 0, 0);
	_screen->drawShape(0, getShapePtr(new010+433), x +  8, y, 0, 0);
	_screen->drawShape(0, getShapePtr(new001+433), x + 16, y, 0, 0);
}

int KyraEngine_MR::getScoreX(const char *str) {
	Screen::FontId oldFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;

	int width = _screen->getTextWidth(str);
	int x = 160 + (width / 2) - 32;

	_screen->setFont(oldFont);
	_screen->_charWidth = 0;
	return x;
}

void KyraEngine_MR::redrawInventory(int page) {
	int yOffset = 0;

	if (page == 30) {
		page = 2;
		yOffset = -144;
	}

	int pageBackUp = _screen->_curPage;
	_screen->_curPage = page;

	for (int i = 0; i < 10; ++i) {
		clearInventorySlot(i, page);
		if (_mainCharacter.inventory[i] != kItemNone) {
			_screen->drawShape(page, getShapePtr(_mainCharacter.inventory[i]+248), _inventoryX[i], _inventoryY[i] + yOffset, 0, 0);
			drawInventorySlot(page, _mainCharacter.inventory[i], i);
		}
	}

	_screen->_curPage = pageBackUp;

	if (page == 0 || page == 1)
		_screen->updateScreen();
}

void KyraEngine_MR::clearInventorySlot(int slot, int page) {
	int yOffset = 0;
	if (page == 30) {
		page = 2;
		yOffset = -144;
	}

	_screen->drawShape(page, getShapePtr(slot+422), _inventoryX[slot], _inventoryY[slot] + yOffset, 0, 0);
}

void KyraEngine_MR::drawInventorySlot(int page, Item item, int slot) {
	int yOffset = 0;
	if (page == 30) {
		page = 2;
		yOffset = -144;
	}

	_screen->drawShape(page, getShapePtr(item+248), _inventoryX[slot], _inventoryY[slot] + yOffset, 0, 0);
}

int KyraEngine_MR::buttonInventory(Button *button) {
	setNextIdleAnimTimer();
	if (!_enableInventory || !_inventoryState || !_screen->isMouseVisible())
		return 0;

	const int slot = button->index - 5;
	const Item slotItem = _mainCharacter.inventory[slot];
	if (_itemInHand == kItemNone) {
		if (slotItem == kItemNone)
			return 0;

		clearInventorySlot(slot, 0);
		snd_playSoundEffect(0x0B, 0xC8);
		setMouseCursor(slotItem);
		updateItemCommand(slotItem, (_lang == 1) ? getItemCommandStringPickUp(slotItem) : 0, 0xFF);
		_itemInHand = slotItem;
		_mainCharacter.inventory[slot] = kItemNone;
	} else if (_itemInHand == 27) {
		if (_chatText)
			return 0;
		return buttonJesterStaff(&_mainButtonData[3]);
	} else {
		if (slotItem >= 0) {
			if (itemInventoryMagic(_itemInHand, slot))
				return 0;

			snd_playSoundEffect(0x0B, 0xC8);

			clearInventorySlot(slot, 0);
			drawInventorySlot(0, _itemInHand, slot);
			setMouseCursor(slotItem);
			updateItemCommand(slotItem, (_lang == 1) ? getItemCommandStringPickUp(slotItem) : 0, 0xFF);
			_mainCharacter.inventory[slot] = _itemInHand;
			_itemInHand = slotItem;
		} else {
			snd_playSoundEffect(0x0C, 0xC8);
			drawInventorySlot(0, _itemInHand, slot);
			_screen->setMouseCursor(0, 0, getShapePtr(0));
			updateItemCommand(_itemInHand, (_lang == 1) ? getItemCommandStringInv(_itemInHand) : 2, 0xFF);
			_mainCharacter.inventory[slot] = _itemInHand;
			_itemInHand = kItemNone;
		}
	}

	return 0;
}

int KyraEngine_MR::buttonMoodChange(Button *button) {
	if (queryGameFlag(0x219)) {
		snd_playSoundEffect(0x0D, 0xC8);
		return 0;
	}

	static const uint8 frameTable[] = { 1, 6, 11 };

	if (_mouseX >= 245 && _mouseX <= 267 && _mouseY >= 159 && _mouseY <= 198)
		_malcolmsMood = 0;
	else if (_mouseX >= 268 && _mouseX <= 289 && _mouseY >= 159 && _mouseY <= 198)
		_malcolmsMood = 1;
	else if (_mouseX >= 290 && _mouseX <= 312 && _mouseY >= 159 && _mouseY <= 198)
		_malcolmsMood = 2;

	int direction = (_invWsaFrame > frameTable[_malcolmsMood]) ? -1 : 1;

	if (_invWsaFrame != frameTable[_malcolmsMood]) {
		_screen->hideMouse();
		setGameFlag(3);

		snd_playSoundEffect(0x2E, 0xC8);

		while (_invWsaFrame != frameTable[_malcolmsMood]) {
			uint32 endTime = _system->getMillis() + 2 * _tickLength;
			_invWsaFrame += direction;

			drawMalcolmsMoodPointer(_invWsaFrame, 0);
			_screen->updateScreen();

			while (endTime > _system->getMillis()) {
				update();
				_system->delayMillis(10);
			}
		}

		resetGameFlag(3);
		_screen->showMouse();

		drawMalcolmsMoodText();
		updateDlgIndex();

		EMCData data;
		EMCState state;
		memset(&data, 0, sizeof(data));
		memset(&state, 0, sizeof(state));

		_res->exists("_ACTOR.EMC", true);
		_emc->load("_ACTOR.EMC", &data, &_opcodes);
		_emc->init(&state, &data);
		_emc->start(&state, 1);

		int vocHigh = _vocHigh;
		_vocHigh = 200;
		_useActorBuffer = true;

		while (_emc->isValid(&state))
			_emc->run(&state);

		_useActorBuffer = false;
		_vocHigh = vocHigh;
		_emc->unload(&data);
	}

	return 0;
}

int KyraEngine_MR::buttonShowScore(Button *button) {
	strcpy(_stringBuffer, (const char *)getTableEntry(_cCodeFile, 18));

	char *buffer = _stringBuffer;

	while (*buffer != '%')
		++buffer;

	buffer[0] = (_score / 100) + '0';
	buffer[1] = ((_score % 100) / 10) + '0';
	buffer[2] = (_score % 10) + '0';

	while (*buffer != '%')
		++buffer;

	buffer[0] = (_scoreMax / 100) + '0';
	buffer[1] = ((_scoreMax % 100) / 10) + '0';
	buffer[2] = (_scoreMax % 10) + '0';

	showMessage(_stringBuffer, 0xFF, 0xF0);
	return 0;
}

int KyraEngine_MR::buttonJesterStaff(Button *button) {
	makeCharFacingMouse();
	if (_itemInHand == 27) {
		removeHandItem();
		snd_playSoundEffect(0x0C, 0xC8);
		drawJestersStaff(1, 0);
		updateItemCommand(27, 2, 0xFF);
		setGameFlag(0x97);
	} else if (_itemInHand == kItemNone) {
		if (queryGameFlag(0x97)) {
			snd_playSoundEffect(0x0B, 0xC8);
			setHandItem(27);
			drawJestersStaff(0, 0);
			updateItemCommand(27, 0, 0xFF);
			resetGameFlag(0x97);
		} else {
			if (queryGameFlag(0x2F))
				objectChat((const char *)getTableEntry(_cCodeFile, 20), 0, 204, 20);
			else
				objectChat((const char *)getTableEntry(_cCodeFile, 25), 0, 204, 25);
		}
	} else {
		objectChat((const char *)getTableEntry(_cCodeFile, 30), 0, 204, 30);
	}
	return 0;
}

void KyraEngine_MR::showAlbum() {
	if (!_screen->isMouseVisible() || queryGameFlag(4) || _mouseState != -1)
		return;

	if (!loadLanguageFile("ALBUM.", _album.file))
		error("Couldn't load ALBUM");

	if (!queryGameFlag(0x8B))
		_album.wsa->open("ALBMGNTH.WSA", 1, 0);
	_album.backUpRect = new uint8[3100];
	assert(_album.backUpRect);
	_album.backUpPage = new uint8[64000];
	assert(_album.backUpPage);
	_album.nextPage = _album.curPage;

	_screen->copyRegionToBuffer(0, 0, 0, 320, 200, _screenBuffer);
	_screen->copyRegionToBuffer(4, 0, 0, 320, 200, _album.backUpPage);

	_screen->copyPalette(1, 0);
	_screen->fadeToBlack(9);

	int itemInHand = _itemInHand;
	removeHandItem();

	_screen->loadPalette("ALBUM.COL", _screen->getPalette(0));
	loadAlbumPage();
	loadAlbumPageWSA();

	if (_album.leftPage.wsa->opened())
		_album.leftPage.wsa->displayFrame(_album.leftPage.curFrame, 2, _albumWSAX[_album.nextPage+0], _albumWSAY[_album.nextPage+0], 0x4000, 0, 0);

	if (_album.rightPage.wsa->opened())
		_album.rightPage.wsa->displayFrame(_album.rightPage.curFrame, 2, _albumWSAX[_album.nextPage+1], _albumWSAY[_album.nextPage+1], 0x4000, 0, 0);

	printAlbumPageText();
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_screen->fadePalette(_screen->getPalette(0), 9);

	processAlbum();

	_screen->fadeToBlack(9);
	_album.wsa->close();

	setHandItem(itemInHand);
	updateMouse();

	restorePage3();
	_screen->copyBlockToPage(0, 0, 0, 320, 200, _screenBuffer);
	_screen->copyBlockToPage(4, 0, 0, 320, 200, _album.backUpPage);

	_screen->copyPalette(0, 1);
	_screen->fadePalette(_screen->getPalette(0), 9);

	delete[] _album.backUpRect;
	_album.backUpRect = 0;
	delete[] _album.backUpPage;
	_album.backUpPage = 0;
	delete[] _album.file;
	_album.file = 0;

	_eventList.clear();
}

void KyraEngine_MR::loadAlbumPage() {
	Common::String filename;
	int num = _album.curPage / 2;

	if (num == 0) {
		filename = "ALBUM0.CPS";
	} else if (num >= 1 && num <= 6) {
		--num;
		num %= 2;
		filename = Common::String::format("ALBUM%d.CPS", num+1);
	} else {
		filename = "ALBUM3.CPS";
	}

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 4, Screen::CR_NO_P_CHECK);
	_screen->loadBitmap(filename.c_str(), 3, 3, 0);
}

void KyraEngine_MR::loadAlbumPageWSA() {
	Common::String filename;

	_album.leftPage.curFrame = 0;
	_album.leftPage.maxFrame = 0;
	_album.leftPage.wsa->close();

	_album.rightPage.curFrame = 0;
	_album.rightPage.maxFrame = 0;
	_album.rightPage.wsa->close();

	if (_album.curPage) {
		filename = Common::String::format("PAGE%x.WSA", _album.curPage);
		_album.leftPage.wsa->open(filename.c_str(), 1, 0);
		_album.leftPage.maxFrame = _album.leftPage.wsa->frames()-1;
	}

	if (_album.curPage != 14) {
		filename = Common::String::format("PAGE%x.WSA", _album.curPage+1);
		_album.rightPage.wsa->open(filename.c_str(), 1, 0);
		_album.rightPage.maxFrame = _album.leftPage.wsa->frames()-1;
	}
}

void KyraEngine_MR::printAlbumPageText() {
	static const uint8 posY[] = {
		0x41, 0x55, 0x55, 0x55, 0x55, 0x55, 0x5A, 0x5A,
		0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x3C
	};

	const int leftY = posY[_album.curPage];
	const int rightY = posY[_album.curPage+1];

	for (int i = 0; i < 5; ++i) {
		const char *str = (const char *)getTableEntry(_album.file, _album.curPage*5+i);
		int y = i * 10 + leftY + 20;
		printAlbumText(2, str, 20, y, 10);
	}

	for (int i = 0; i < 5; ++i) {
		const char *str = (const char *)getTableEntry(_album.file, (_album.curPage+1)*5+i);
		int y = i * 10 + rightY + 20;
		printAlbumText(2, str, 176, y, 10);
	}

	albumBackUpRect();
}

void KyraEngine_MR::printAlbumText(int page, const char *str, int x, int y, uint8 c0) {
	int oldPage = _screen->_curPage;
	_screen->_curPage = page;

	static const uint8 colorMap[] = { 0, 0x87, 0xA3, 0 };
	_screen->setTextColor(colorMap, 0, 3);

	Screen::FontId oldFont = _screen->setFont(Screen::FID_BOOKFONT_FNT);
	_screen->_charWidth = -2;

	_screen->printText(str, x, y, c0, 0);

	_screen->_charWidth = 0;
	_screen->setFont(oldFont);
	_screen->_curPage = oldPage;
}

void KyraEngine_MR::processAlbum() {
	Button albumButtons[5];

	GUI_V2_BUTTON(albumButtons[0], 36, 0, 0, 1, 1, 1, 0x4487, 0, 130, 190,  10,  10, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
	albumButtons[0].buttonCallback = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::albumPrevPage);
	GUI_V2_BUTTON(albumButtons[1], 37, 0, 0, 1, 1, 1, 0x4487, 0, 177, 190,  10,  10, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
	albumButtons[1].buttonCallback = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::albumNextPage);
	GUI_V2_BUTTON(albumButtons[2], 38, 0, 0, 1, 1, 1, 0x4487, 0,   0,   0, 320,   8, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
	albumButtons[2].buttonCallback = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::albumClose);
	GUI_V2_BUTTON(albumButtons[3], 39, 0, 0, 1, 1, 1, 0x4487, 0,   8,   8, 144, 180, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
	albumButtons[3].buttonCallback = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::albumPrevPage);
	GUI_V2_BUTTON(albumButtons[4], 40, 0, 0, 1, 1, 1, 0x4487, 0, 170,   8, 142, 180, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
	albumButtons[4].buttonCallback = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::albumNextPage);

	Button *buttonList = 0;
	for (int i = 0; i < 5; ++i)
		buttonList = _gui->addButtonToList(buttonList, &albumButtons[i]);

	_album.leftPage.timer = _album.rightPage.timer = _system->getMillis();
	albumNewPage();
	_album.running = true;

	while (_album.running && !shouldQuit()) {
		updateInput();
		checkInput(buttonList);
		removeInputTop();

		if (_album.curPage != _album.nextPage) {
			int oldPage = _album.curPage;
			_album.curPage = _album.nextPage;

			_album.leftPage.wsa->close();
			_album.rightPage.wsa->close();

			loadAlbumPage();
			loadAlbumPageWSA();

			if (_album.leftPage.wsa->opened())
				_album.leftPage.wsa->displayFrame(_album.leftPage.curFrame, 2, _albumWSAX[_album.nextPage+0], _albumWSAY[_album.nextPage+0], 0x4000, 0, 0);

			if (_album.rightPage.wsa->opened())
				_album.rightPage.wsa->displayFrame(_album.rightPage.curFrame, 2, _albumWSAX[_album.nextPage+1], _albumWSAY[_album.nextPage+1], 0x4000, 0, 0);

			printAlbumPageText();

			snd_playSoundEffect(0x85, 0x80);
			albumSwitchPages(oldPage, _album.nextPage, 4);

			_album.leftPage.timer = _album.rightPage.timer = 0;
			albumNewPage();

			_eventList.clear();
		}

		albumUpdateAnims();
		_system->delayMillis(10);
	}

	_album.leftPage.wsa->close();
	_album.rightPage.wsa->close();
}

void KyraEngine_MR::albumNewPage() {
	int page = _album.nextPage / 2;
	if (!queryGameFlag(0x84+page)) {
		albumAnim1();
		delayWithTicks(8);

		int id = _album.curPage / 2 + 100;
		albumChat((const char *)getTableEntry(_album.file, id), 205, id);

		if (id == 107) {
			_screen->copyRegion(76, 100, 76, 100, 244, 100, 2, 0, Screen::CR_NO_P_CHECK);
			albumChat((const char *)getTableEntry(_album.file, 108), 205, 108);
			_screen->copyRegion(76, 100, 76, 100, 244, 100, 2, 0, Screen::CR_NO_P_CHECK);
			albumChat((const char *)getTableEntry(_album.file, 109), 205, 109);
		}

		delayWithTicks(5);
		albumAnim2();

		setGameFlag(0x84+page);
		_album.isPage14 = (_album.nextPage == 14);
	}
}

void KyraEngine_MR::albumUpdateAnims() {
	if (_album.nextPage == 14 && !_album.isPage14)
		return;

	uint32 nextRun = 0;

	nextRun = _album.leftPage.timer + 5 * _tickLength;
	if (nextRun < _system->getMillis() && _album.leftPage.wsa->opened()) {
		_album.leftPage.wsa->displayFrame(_album.leftPage.curFrame, 2, _albumWSAX[_album.nextPage+0], _albumWSAY[_album.nextPage+0], 0x4000, 0, 0);
		_screen->copyRegion(40, 17, 40, 17, 87, 73, 2, 0, Screen::CR_NO_P_CHECK);

		++_album.leftPage.curFrame;
		_album.leftPage.timer = _system->getMillis();

		if (_album.leftPage.curFrame > _album.leftPage.maxFrame) {
			_album.leftPage.curFrame = 0;
			if (_album.nextPage == 14) {
				_album.isPage14 = false;
				_album.leftPage.timer += 100000 * _tickLength;
			} else {
				_album.leftPage.timer += 180 * _tickLength;
			}
		}
	}

	nextRun = _album.rightPage.timer + 5 * _tickLength;
	if (nextRun < _system->getMillis() && _album.rightPage.wsa->opened()) {
		_album.rightPage.wsa->displayFrame(_album.rightPage.curFrame, 2, _albumWSAX[_album.nextPage+1], _albumWSAY[_album.nextPage+1], 0x4000, 0, 0);
		_screen->copyRegion(194, 20, 194, 20, 85, 69, 2, 0, Screen::CR_NO_P_CHECK);

		++_album.rightPage.curFrame;
		_album.rightPage.timer = _system->getMillis();

		if (_album.rightPage.curFrame > _album.rightPage.maxFrame) {
			_album.rightPage.curFrame = 0;
			_album.rightPage.timer += 180 * _tickLength;
		}
	}

	_screen->updateScreen();
}

void KyraEngine_MR::albumAnim1() {
	for (int i = 6; i >= 3; --i) {
		albumRestoreRect();
		_album.wsa->displayFrame(i, 2, -100, 90, 0x4000, 0, 0);
		albumUpdateRect();
		delayWithTicks(1);
	}

	albumRestoreRect();
	_album.wsa->displayFrame(14, 2, -100, 90, 0x4000, 0, 0);
	albumUpdateRect();
	delayWithTicks(1);
}

void KyraEngine_MR::albumAnim2() {
	for (int i = 3; i <= 6; ++i) {
		albumRestoreRect();
		_album.wsa->displayFrame(i, 2, -100, 90, 0x4000, 0, 0);
		albumUpdateRect();
		delayWithTicks(1);
	}

	albumRestoreRect();
	_screen->copyRegion(0, 100, 0, 100, 320, 100, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
}

void KyraEngine_MR::albumBackUpRect() {
	_screen->copyRegionToBuffer(2, 0, 146, 62, 50, _album.backUpRect);
}

void KyraEngine_MR::albumRestoreRect() {
	_screen->copyBlockToPage(2, 0, 146, 62, 50, _album.backUpRect);
}

void KyraEngine_MR::albumUpdateRect() {
	_screen->copyRegion(0, 146, 0, 146, 62, 50, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
}

void KyraEngine_MR::albumSwitchPages(int oldPage, int newPage, int srcPage) {
	if (newPage > oldPage) {
		_screen->wsaFrameAnimationStep(0xA0, 0x07, 0xA0, 0x07, 0x96, 0xBA, 0x64, 0xBA, srcPage, 0, 2);

		_screen->copyRegion(260, 7, 260, 7, 50, 186, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		delayWithTicks(2);

		_screen->wsaFrameAnimationStep(0xA0, 0x07, 0xA0, 0x07, 0x96, 0xBA, 0x32, 0xBA, srcPage, 0, 2);

		_screen->copyRegion(210, 7, 210, 7, 50, 186, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		delayWithTicks(2);

		_screen->copyRegion(160, 7, 160, 7, 50, 186, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		delayWithTicks(2);

		_screen->wsaFrameAnimationStep(0x10, 0x07, 0x6E, 0x07, 0x96, 0xBA, 0x32, 0xBA, 2, 0, 2);
		_screen->updateScreen();
		delayWithTicks(2);

		_screen->wsaFrameAnimationStep(0x10, 0x07, 0x3C, 0x07, 0x96, 0xBA, 0x64, 0xBA, 2, 0, 2);
		_screen->updateScreen();
		delayWithTicks(2);

		_screen->copyRegion(10, 7, 10, 7, 150, 186, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	} else {
		_screen->wsaFrameAnimationStep(0x0A, 0x07, 0x3C, 0x07, 0x96, 0xBA, 0x64, 0xBA, srcPage, 0, 2);

		_screen->copyRegion(10, 7, 10, 7, 50, 186, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		delayWithTicks(2);

		_screen->wsaFrameAnimationStep(0x0A, 0x07, 0x6E, 0x07, 0x96, 0xBA, 0x32, 0xBA, srcPage, 0, 2);

		_screen->copyRegion(60, 7, 60, 7, 50, 186, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		delayWithTicks(2);

		_screen->copyRegion(110, 7, 110, 7, 50, 186, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		delayWithTicks(2);

		_screen->wsaFrameAnimationStep(0xA0, 0x07, 0xA0, 0x07, 0x96, 0xBA, 0x32, 0xBA, 2, 0, 2);
		_screen->updateScreen();
		delayWithTicks(2);

		_screen->wsaFrameAnimationStep(0xA0, 0x07, 0xA0, 0x07, 0x96, 0xBA, 0x64, 0xBA, 2, 0, 2);
		_screen->updateScreen();
		delayWithTicks(2);

		_screen->copyRegion(160, 7, 160, 7, 150, 186, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}
}

int KyraEngine_MR::albumNextPage(Button *caller) {
	_album.nextPage = _album.curPage + 2;
	if (_album.nextPage >= 16) {
		_album.nextPage -= 2;
		_album.running = false;
	}
	return 0;
}

int KyraEngine_MR::albumPrevPage(Button *caller) {
	_album.nextPage = _album.curPage - 2;
	if (_album.nextPage < 0) {
		_album.nextPage = 0;
		_album.running = false;
	}
	return 0;
}

int KyraEngine_MR::albumClose(Button *caller) {
	_album.running = false;
	return 0;
}

#pragma mark -

GUI_MR::GUI_MR(KyraEngine_MR *vm) : GUI_v2(vm), _vm(vm), _screen(vm->_screen) {
}

void GUI_MR::createScreenThumbnail(Graphics::Surface &dst) {
	uint8 screenPal[768];
	_screen->getRealPalette(0, screenPal);
	::createThumbnail(&dst, _vm->_screenBuffer, Screen::SCREEN_W, Screen::SCREEN_H, screenPal);
}

void GUI_MR::flagButtonEnable(Button *button) {
	if (!button)
		return;

	if (button->flags & 8) {
		button->flags &= ~8;
		processButton(button);
	}
}

void GUI_MR::flagButtonDisable(Button *button) {
	if (!button)
		return;

	if (!(button->flags & 8)) {
		button->flags |= 8;
		processButton(button);
	}
}

const char *GUI_MR::getMenuTitle(const Menu &menu) {
	if (!menu.menuNameId)
		return 0;

	return (const char *)_vm->getTableEntry(_vm->_optionsFile, menu.menuNameId);
}

const char *GUI_MR::getMenuItemTitle(const MenuItem &menuItem) {
	if (!menuItem.itemId)
		return 0;

	return (const char *)_vm->getTableEntry(_vm->_optionsFile, menuItem.itemId);
}

const char *GUI_MR::getMenuItemLabel(const MenuItem &menuItem) {
	if (!menuItem.labelId)
		return 0;

	return (const char *)_vm->getTableEntry(_vm->_optionsFile, menuItem.labelId);
}

char *GUI_MR::getTableString(int id) {
	return (char *)_vm->getTableEntry(_vm->_optionsFile, id);
}

int GUI_MR::redrawButtonCallback(Button *button) {
	if (!_displayMenu)
		return 0;

	_screen->drawBox(button->x + 1, button->y + 1, button->x + button->width - 1, button->y + button->height - 1, 0xD0);

	return 0;
}

int GUI_MR::redrawShadedButtonCallback(Button *button) {
	if (!_displayMenu)
		return 0;

	_screen->drawShadedBox(button->x, button->y, button->x + button->width, button->y + button->height, 0xD1, 0xCF);

	return 0;
}
void GUI_MR::resetState(int item) {
	_vm->_timer->resetNextRun();
	_vm->setNextIdleAnimTimer();
	_isDeathMenu = false;
	if (!_loadedSave) {
		_vm->_itemInHand = kItemNone;
		_vm->setHandItem(item);
	} else {
		_vm->setHandItem(_vm->_itemInHand);
		_vm->setCommandLineRestoreTimer(7);
		_vm->_shownMessage = " ";
		_vm->_restoreCommandLine = false;
	}
	_buttonListChanged = true;
}

int GUI_MR::quitGame(Button *caller) {
	updateMenuButton(caller);
	if (choiceDialog(0x0F, 1)) {
		_displayMenu = false;
		_vm->_runFlag = false;
		_vm->fadeOutMusic(60);
		_screen->fadeToBlack(60);
		_screen->clearCurPage();
	}

	if (_vm->_runFlag) {
		initMenu(*_currentMenu);
		updateAllMenuButtons();
	}

	return 0;
}

int GUI_MR::optionsButton(Button *button) {
	PauseTimer pause(*_vm->_timer);

	updateButton(&_vm->_mainButtonData[0]);

	if (!_vm->_inventoryState && button && !_vm->_menuDirectlyToLoad)
		return 0;

	_restartGame = false;
	_reloadTemporarySave = false;

	if (!_screen->isMouseVisible() && button && !_vm->_menuDirectlyToLoad)
		return 0;

	_vm->showMessage(0, 0xF0, 0xF0);

	if (_vm->_mouseState < -1) {
		_vm->_mouseState = -1;
		_screen->setMouseCursor(1, 1, _vm->getShapePtr(0));
		return 0;
	}

	int oldHandItem = _vm->_itemInHand;
	_screen->setMouseCursor(0, 0, _vm->getShapePtr(0));

	_displayMenu = true;
	for (int i = 0; i < 4; ++i) {
		if (_vm->_musicSoundChannel != i)
			_vm->_soundDigital->stopSound(i);
	}

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

		_loadedSave = false;

		--_loadMenu.numberOfItems;
		loadMenu(0);
		++_loadMenu.numberOfItems;

		if (_loadedSave) {
			if (_restartGame)
				_vm->_itemInHand = kItemNone;
		} else {
			restorePage1(_vm->_screenBuffer);
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
		updateMenuButton(&_vm->_mainButtonData[0]);

	resetState(oldHandItem);

	if (!_loadedSave && _reloadTemporarySave) {
		_vm->_unkSceneScreenFlag1 = true;
		_vm->loadGameStateCheck(999);
		//_vm->_saveFileMan->removeSavefile(_vm->getSavegameFilename(999));
		_vm->_unkSceneScreenFlag1 = false;
	}

	return 0;
}

int GUI_MR::loadMenu(Button *caller) {
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
		_vm->_menuDirectlyToLoad = false;
		_vm->loadGameStateCheck(_vm->_gameToLoad);
		if (_vm->_gameToLoad == 0) {
			_restartGame = true;
			_vm->runStartupScript(1, 1);
		}
		_displayMenu = false;
		_loadedSave = true;
	}

	return 0;
}

int GUI_MR::loadSecondChance(Button *button) {
	updateMenuButton(button);

	_vm->_gameToLoad = 999;
	restorePage1(_vm->_screenBuffer);
	_vm->loadGameStateCheck(_vm->_gameToLoad);
	_displayMenu = false;
	_loadedSave = true;
	return 0;
}

int GUI_MR::gameOptions(Button *caller) {
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

		if (!_vm->loadLanguageFile("ITEMS.", _vm->_itemFile))
			error("Couldn't load ITEMS");
		if (!_vm->loadLanguageFile("SCORE.", _vm->_scoreFile))
			error("Couldn't load SCORE");
		if (!_vm->loadLanguageFile("C_CODE.", _vm->_cCodeFile))
			error("Couldn't load C_CODE");
		if (!_vm->loadLanguageFile("SCENES.", _vm->_scenesFile))
			error("Couldn't load SCENES");
		if (!_vm->loadLanguageFile("OPTIONS.", _vm->_optionsFile))
			error("Couldn't load OPTIONS");
		if (!_vm->loadLanguageFile("_ACTOR.", _vm->_actorFile))
			error("couldn't load _ACTOR");
	}

	_vm->writeSettings();

	initMenu(*_currentMenu);
	updateAllMenuButtons();
	return 0;
}

void GUI_MR::setupOptionsButtons() {
	if (_vm->_configWalkspeed == 3)
		_gameOptions.item[0].itemId = 28;
	else
		_gameOptions.item[0].itemId = 27;

	if (_vm->textEnabled())
		_gameOptions.item[4].itemId = 18;
	else
		_gameOptions.item[4].itemId = 17;

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

	if (_vm->_configStudio)
		_gameOptions.item[2].itemId = 18;
	else
		_gameOptions.item[2].itemId = 17;

	if (_vm->_configSkip)
		_gameOptions.item[3].itemId = 18;
	else
		_gameOptions.item[3].itemId = 17;
}

int GUI_MR::changeLanguage(Button *caller) {
	updateMenuButton(caller);
	if (!_vm->queryGameFlag(0x1B2)) {
		++_vm->_lang;
		_vm->_lang %= 3;
		setupOptionsButtons();
		renewHighlight(_gameOptions);
	}
	return 0;
}

int GUI_MR::toggleStudioSFX(Button *caller) {
	updateMenuButton(caller);
	_vm->_configStudio ^= 1;
	setupOptionsButtons();
	renewHighlight(_gameOptions);
	return 0;
}

int GUI_MR::toggleSkipSupport(Button *caller) {
	updateMenuButton(caller);
	_vm->_configSkip ^= 1;
	setupOptionsButtons();
	renewHighlight(_gameOptions);
	return 0;
}

int GUI_MR::toggleHeliumMode(Button *caller) {
	updateMenuButton(caller);
	_vm->_configHelium ^= 1;
	if (_vm->_configHelium)
		_audioOptions.item[3].itemId = 18;
	else
		_audioOptions.item[3].itemId = 17;
	renewHighlight(_audioOptions);
	return 0;
}

int GUI_MR::audioOptions(Button *caller) {
	updateMenuButton(caller);

	restorePage1(_vm->_screenBuffer);
	backUpPage1(_vm->_screenBuffer);

	if (_vm->_configHelium)
		_audioOptions.item[3].itemId = 18;
	else
		_audioOptions.item[3].itemId = 17;

	initMenu(_audioOptions);

	const int menuX = _audioOptions.x;
	const int menuY = _audioOptions.y;

	const int maxButton = 3;	// 2 if voc is disabled

	for (int i = 0; i < maxButton; ++i) {
		int x = menuX + _sliderBarsPosition[i*2+0];
		int y = menuY + _sliderBarsPosition[i*2+1];
		_screen->drawShape(0, _vm->getShapePtr(0x1CF), x, y, 0, 0);
		drawSliderBar(i, _vm->getShapePtr(0x1D0));
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

int GUI_MR::sliderHandler(Button *caller) {
	int button = 0;
	if (caller->index >= 24 && caller->index <= 27)
		button = caller->index - 24;
	else if (caller->index >= 28 && caller->index <= 31)
		button = caller->index - 28;
	else
		button = caller->index - 32;

	assert(button >= 0 && button <= 3);

	const int oldVolume = _vm->getVolume(KyraEngine_v1::kVolumeEntry(button));
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

	drawSliderBar(button, _vm->getShapePtr(0x1D1));

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
		if (_vm->_voiceSoundChannel != _vm->_musicSoundChannel)
			_vm->_soundDigital->stopSound(_vm->_voiceSoundChannel);
		_vm->playVoice(200, 943);
		break;

	default:
		return 0;
	}

	drawSliderBar(button, _vm->getShapePtr(0x1D0));
	if (playSoundEffect)
		_vm->snd_playSoundEffect(0x18, 0xC8);
	else if (lastMusicCommand >= 0)
		_vm->snd_playWanderScoreViaMap(lastMusicCommand, 0);

	_screen->updateScreen();
	return 0;
}

void GUI_MR::drawSliderBar(int slider, const uint8 *shape) {
	const int menuX = _audioOptions.x;
	const int menuY = _audioOptions.y;
	int x = menuX + _sliderBarsPosition[slider*2+0] + 10;
	int y = menuY + _sliderBarsPosition[slider*2+1];

	int position = _vm->getVolume(KyraEngine_v1::kVolumeEntry(slider));

	position = CLIP(position, 2, 97);
	_screen->drawShape(0, shape, x+position, y, 0, 0);
}

} // End of namespace Kyra
