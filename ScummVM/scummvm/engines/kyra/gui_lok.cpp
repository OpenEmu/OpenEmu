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

#include "kyra/gui_lok.h"
#include "kyra/kyra_lok.h"
#include "kyra/animator_lok.h"
#include "kyra/text.h"
#include "kyra/timer.h"
#include "kyra/util.h"

#include "common/savefile.h"
#include "common/system.h"

#include "graphics/scaler.h"

namespace Kyra {

void KyraEngine_LoK::initMainButtonList() {
	_buttonList = &_buttonData[0];
	for (int i = 0; _buttonDataListPtr[i]; ++i)
		_buttonList = _gui->addButtonToList(_buttonList, _buttonDataListPtr[i]);
}

int KyraEngine_LoK::buttonInventoryCallback(Button *caller) {
	int itemOffset = caller->index - 2;
	Item inventoryItem = (int8)_currentCharacter->inventoryItems[itemOffset];
	if (_itemInHand == kItemNone) {
		if (inventoryItem == kItemNone) {
			snd_playSoundEffect(0x36);
			return 0;
		} else {
			_screen->fillRect(_itemPosX[itemOffset], _itemPosY[itemOffset], _itemPosX[itemOffset] + 15, _itemPosY[itemOffset] + 15, _flags.platform == Common::kPlatformAmiga ? 19 : 12);
			snd_playSoundEffect(0x35);
			setMouseItem(inventoryItem);
			updateSentenceCommand(_itemList[getItemListIndex(inventoryItem)], _takenList[0], 179);
			_itemInHand = inventoryItem;
			_currentCharacter->inventoryItems[itemOffset] = kItemNone;
		}
	} else {
		if (inventoryItem != kItemNone) {
			snd_playSoundEffect(0x35);
			_screen->fillRect(_itemPosX[itemOffset], _itemPosY[itemOffset], _itemPosX[itemOffset] + 15, _itemPosY[itemOffset] + 15, _flags.platform == Common::kPlatformAmiga ? 19 : 12);
			_screen->drawShape(0, _shapes[216 + _itemInHand], _itemPosX[itemOffset], _itemPosY[itemOffset], 0, 0);
			setMouseItem(inventoryItem);
			// TODO: Proper support for both taken strings in Amiga version
			if (_flags.platform == Common::kPlatformAmiga)
				updateSentenceCommand(_itemList[getItemListIndex(inventoryItem)], _takenList[0], 179);
			else
				updateSentenceCommand(_itemList[getItemListIndex(inventoryItem)], _takenList[1], 179);
			_currentCharacter->inventoryItems[itemOffset] = _itemInHand;
			_itemInHand = inventoryItem;
		} else {
			snd_playSoundEffect(0x32);
			_screen->drawShape(0, _shapes[216 + _itemInHand], _itemPosX[itemOffset], _itemPosY[itemOffset], 0, 0);
			_screen->setMouseCursor(1, 1, _shapes[0]);
			updateSentenceCommand(_itemList[getItemListIndex(_itemInHand)], _placedList[0], 179);
			_currentCharacter->inventoryItems[itemOffset] = _itemInHand;
			_itemInHand = kItemNone;
		}
	}
	_screen->updateScreen();
	return 0;
}

int KyraEngine_LoK::buttonAmuletCallback(Button *caller) {
	if (!(_deathHandler & 8))
		return 1;
	int jewel = caller->index - 0x14;
	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return 1;
	}
	if (!queryGameFlag(0x2D))
		return 1;
	if (_itemInHand != kItemNone) {
		assert(_putDownFirst);
		characterSays(2000, _putDownFirst[0], 0, -2);
		return 1;
	}
	if (queryGameFlag(0xF1)) {
		assert(_waitForAmulet);
		characterSays(2001, _waitForAmulet[0], 0, -2);
		return 1;
	}
	if (!queryGameFlag(0x55 + jewel)) {
		assert(_blackJewel);
		_animator->makeBrandonFaceMouse();
		drawJewelPress(jewel, 1);
		characterSays(2002, _blackJewel[0], 0, -2);
		return 1;
	}
	drawJewelPress(jewel, 0);
	drawJewelsFadeOutStart();
	drawJewelsFadeOutEnd(jewel);

	_emc->init(&_scriptClick, &_scriptClickData);
	_scriptClick.regs[3] = 0;
	_scriptClick.regs[6] = jewel;
	_emc->start(&_scriptClick, 4);

	while (_emc->isValid(&_scriptClick))
		_emc->run(&_scriptClick);

	if (_scriptClick.regs[3])
		return 1;

	_unkAmuletVar = 1;
	switch (jewel - 1) {
	case 0:
		if (_brandonStatusBit & 1) {
			seq_brandonHealing2();
		} else if (_brandonStatusBit == 0) {
			seq_brandonHealing();
			assert(_healingTip);
			characterSays(2003, _healingTip[0], 0, -2);
		}
		break;

	case 1:
		seq_makeBrandonInv();
		break;

	case 2:
		if (_brandonStatusBit & 1) {
			assert(_wispJewelStrings);
			characterSays(2004, _wispJewelStrings[0], 0, -2);
		} else {
			if (_brandonStatusBit & 2) {
				// XXX
				seq_makeBrandonNormal2();
				// XXX
			} else {
				// do not check for item in hand again as in the original since some strings are missing
				// in the cd version
				if (_currentCharacter->sceneId >= 109 && _currentCharacter->sceneId <= 198) {
					snd_playWanderScoreViaMap(1, 0);
					seq_makeBrandonWisp();
					snd_playWanderScoreViaMap(17, 0);
				} else {
					seq_makeBrandonWisp();
				}
				setGameFlag(0x9E);
			}
		}
		break;

	case 3:
		seq_dispelMagicAnimation();
		assert(_magicJewelString);
		characterSays(2007, _magicJewelString[0], 0, -2);
		break;

	default:
		break;
	}
	_unkAmuletVar = 0;
	return 1;
}

#pragma mark -

GUI_LoK::GUI_LoK(KyraEngine_LoK *vm, Screen_LoK *screen) : GUI_v1(vm), _vm(vm), _screen(screen) {
	_lastScreenUpdate = 0;
	_menu = 0;
	_pressFlag = false;
	initStaticResource();
	_scrollUpFunctor = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::scrollUp);
	_scrollDownFunctor = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::scrollDown);
}

GUI_LoK::~GUI_LoK() {
	delete[] _menu;
}

void GUI_LoK::createScreenThumbnail(Graphics::Surface &dst) {
	uint8 *screen = new uint8[Screen::SCREEN_W * Screen::SCREEN_H];
	if (screen) {
		_screen->queryPageFromDisk("SEENPAGE.TMP", 0, screen);
		uint8 screenPal[768];

		if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
			_screen->getRealPalette(0, &screenPal[ 0]);
			_screen->getRealPalette(1, &screenPal[96]);

			// Set the interface palette text color to white
			screenPal[96 + 16 * 3 + 0] = 0xFF;
			screenPal[96 + 16 * 3 + 1] = 0xFF;
			screenPal[96 + 16 * 3 + 2] = 0xFF;

			if (_screen->isInterfacePaletteEnabled()) {
				for (int y = 0; y < 64; ++y) {
					for (int x = 0; x < 320; ++x) {
						screen[(y + 136) * Screen::SCREEN_W + x] += 32;
					}
				}
			}

		} else {
			_screen->getRealPalette(2, screenPal);
		}

		::createThumbnail(&dst, screen, Screen::SCREEN_W, Screen::SCREEN_H, screenPal);
	}
	delete[] screen;
}

int GUI_LoK::processButtonList(Button *list, uint16 inputFlag, int8 mouseWheel) {
	if ((inputFlag & 0xFF) == 199)
		_pressFlag = true;
	else if ((inputFlag & 0xFF) == 200)
		_pressFlag = false;

	int returnValue = 0;
	while (list) {
		if (list->flags & 8) {
			list = list->nextButton;
			continue;
		}

		if (mouseWheel && list->mouseWheel == mouseWheel && list->buttonCallback) {
			if ((*list->buttonCallback)(list))
				break;
		}

		int x = list->x;
		int y = list->y;
		assert(_screen->getScreenDim(list->dimTableIndex) != 0);

		if (x < 0)
			x += _screen->getScreenDim(list->dimTableIndex)->w << 3;
		x += _screen->getScreenDim(list->dimTableIndex)->sx << 3;

		if (y < 0)
			y += _screen->getScreenDim(list->dimTableIndex)->h;
		y += _screen->getScreenDim(list->dimTableIndex)->sy;

		if (_vm->_mouseX >= x && _vm->_mouseY >= y && x + list->width >= _vm->_mouseX && y + list->height >= _vm->_mouseY) {
			int processMouseClick = 0;
			if (list->flags & 0x400) {
				if ((inputFlag & 0xFF) == 199 || _pressFlag) {
					if (!(list->flags2 & 1)) {
						list->flags2 |= 1;
						list->flags2 |= 4;
						processButton(list);
						_screen->updateScreen();
						inputFlag = 0;
					}
				} else if ((inputFlag & 0xFF) == 200) {
					if (list->flags2 & 1) {
						list->flags2 &= 0xFFFE;
						processButton(list);
						processMouseClick = 1;
						inputFlag = 0;
					}
				}
			}

			if (processMouseClick) {
				if (list->buttonCallback) {
					if ((*list->buttonCallback)(list))
						break;
				}
			}
		} else {
			if (list->flags2 & 1) {
				list->flags2 &= 0xFFFE;
				processButton(list);
			}

			if (list->flags2 & 4) {
				list->flags2 &= 0xFFFB;
				processButton(list);
				_screen->updateScreen();
			}
		}

		list = list->nextButton;
	}

	if (!returnValue)
		returnValue = inputFlag & 0xFF;

	return returnValue;
}

void GUI_LoK::processButton(Button *button) {
	if (!button)
		return;

	int processType = 0;
	const uint8 *shape = 0;
	Button::Callback callback;

	int flags = (button->flags2 & 5);
	if (flags == 1) {
		processType = button->data2Val1;
		if (processType == 1)
			shape = button->data2ShapePtr;
		else if (processType == 4)
			callback = button->data2Callback;
	} else if (flags == 4 || flags == 5) {
		processType = button->data1Val1;
		if (processType == 1)
			shape = button->data1ShapePtr;
		else if (processType == 4)
			callback = button->data1Callback;
	} else {
		processType = button->data0Val1;
		if (processType == 1)
			shape = button->data0ShapePtr;
		else if (processType == 4)
			callback = button->data0Callback;
	}

	int x = button->x;
	int y = button->y;
	assert(_screen->getScreenDim(button->dimTableIndex) != 0);
	if (x < 0)
		x += _screen->getScreenDim(button->dimTableIndex)->w << 3;

	if (y < 0)
		y += _screen->getScreenDim(button->dimTableIndex)->h;

	if (processType == 1 && shape)
		_screen->drawShape(_screen->_curPage, shape, x, y, button->dimTableIndex, 0x10);
	else if (processType == 4 && callback)
		(*callback)(button);
}

void GUI_LoK::setGUILabels() {
	int offset = 0;
	int offsetOptions = 0;
	int offsetMainMenu = 0;
	int offsetOn = 0;
	int offsetPC98 = 0;

	int walkspeedGarbageOffset = 36;
	int menuLabelGarbageOffset = 0;

	if (_vm->gameFlags().isTalkie) {
		if (_vm->gameFlags().lang == Common::EN_ANY)
			offset = 52;
		else if (_vm->gameFlags().lang == Common::DE_DEU)
			offset = 30;
		else if (_vm->gameFlags().lang == Common::FR_FRA || _vm->gameFlags().lang == Common::IT_ITA)
			offset = 6;
		offsetOn = offsetMainMenu = offsetOptions = offset;
		walkspeedGarbageOffset = 48;
	} else if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
		if (_vm->gameFlags().lang == Common::EN_ANY) {
			offset = offsetOn = 23;
			offsetOptions = 32;
			walkspeedGarbageOffset = 2;
			offsetMainMenu = 23;
		} else if (_vm->gameFlags().lang == Common::DE_DEU) {
			offset = offsetOn = 12;
			offsetOptions = 21;
			walkspeedGarbageOffset = 3;
			offsetMainMenu = 12;
		}
	} else if (_vm->gameFlags().lang == Common::ES_ESP) {
		offsetOn = offsetMainMenu = offsetOptions = offset = -4;
		menuLabelGarbageOffset = 72;
	} else if (_vm->gameFlags().lang == Common::IT_ITA) {
		offsetOn = offsetMainMenu = offsetOptions = offset = 32;
	} else if (_vm->gameFlags().lang == Common::DE_DEU) {
		offset = offsetMainMenu = offsetOn = offsetOptions = 24;
	} else if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
		offset = 1;
		offsetOptions = 10;
		offsetOn = 0;
		walkspeedGarbageOffset = 0;
	} else if (_vm->gameFlags().platform == Common::kPlatformPC98) {
		offsetMainMenu = offsetOptions = offsetOn = offset = 47;
		offsetPC98 = 1;
	}

	assert(offset + (_vm->gameFlags().isTalkie ? 28 : 23) < _vm->_guiStringsSize);
	assert(offsetOptions + 27 < _vm->_guiStringsSize);
	assert(offsetMainMenu + 19 < _vm->_guiStringsSize);

	// The Legend of Kyrandia
	_menu[0].menuNameString = _vm->_guiStrings[0];
	// Load a Game
	_menu[0].item[0].itemString = _vm->_guiStrings[1];
	// Save a Game
	_menu[0].item[1].itemString = _vm->_guiStrings[2];
	// Game controls
	_menu[0].item[2].itemString = _vm->_guiStrings[3];
	// Quit playing
	_menu[0].item[3].itemString = _vm->_guiStrings[4];
	// Resume game
	_menu[0].item[4].itemString = _vm->_guiStrings[5];

	// Cancel
	_menu[2].item[5].itemString = _vm->_guiStrings[10 + offsetPC98];

	// Enter a description of your saved game:
	_menu[3].menuNameString = _vm->_guiStrings[11 + offsetPC98];
	// Save
	_menu[3].item[0].itemString = _vm->_guiStrings[12 + offsetPC98];
	// Cancel
	_menu[3].item[1].itemString = _vm->_guiStrings[10 + offsetPC98];

	// Rest in peace, Brandon
	_menu[4].menuNameString = _vm->_guiStrings[13 + offsetPC98];
	// Load a game
	_menu[4].item[0].itemString = _vm->_guiStrings[1];
	// Quit playing
	_menu[4].item[1].itemString = _vm->_guiStrings[4];

	// Game Controls
	_menu[5].menuNameString = _vm->_guiStrings[6];
	// Yes
	_menu[1].item[0].itemString = _vm->_guiStrings[22 + offset];
	// No
	_menu[1].item[1].itemString = _vm->_guiStrings[23 + offset];

	// Music is
	_menu[5].item[0].labelString = _vm->_guiStrings[26 + offsetOptions];
	// Sounds are
	_menu[5].item[1].labelString = _vm->_guiStrings[27 + offsetOptions];
	// Walk speed
	_menu[5].item[2].labelString = &_vm->_guiStrings[24 + offsetOptions][walkspeedGarbageOffset];
	// Text speed
	_menu[5].item[4].labelString = _vm->_guiStrings[25 + offsetOptions];
	// Main Menu
	_menu[5].item[5].itemString = &_vm->_guiStrings[19 + offsetMainMenu][menuLabelGarbageOffset];

	if (_vm->gameFlags().isTalkie)
		// Text & Voice
		_voiceTextString = _vm->_guiStrings[28 + offset];

	_textSpeedString = _vm->_guiStrings[25 + offsetOptions];
	_onString =  _vm->_guiStrings[20 + offsetOn];
	_offString =  _vm->_guiStrings[21 + offset];
	_onCDString = _vm->_guiStrings[21];
}

int GUI_LoK::buttonMenuCallback(Button *caller) {
	PauseTimer pause(*_vm->_timer);

	_displayMenu = true;

	assert(_vm->_guiStrings);
	assert(_vm->_configStrings);

	/*
	for (int i = 0; i < _vm->_guiStringsSize; i++)
		debug("GUI string %i: %s", i, _vm->_guiStrings[i]);

	for (int i = 0; i < _vm->_configStringsSize; i++)
		debug("Config string %i: %s", i, _vm->_configStrings[i]);
	*/

	setGUILabels();
	if (_vm->_currentCharacter->sceneId == 210 && _vm->_deathHandler == -1) {
		_vm->snd_playSoundEffect(0x36);
		return 0;
	}

	if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
		_screen->setPaletteIndex(0x10, 0x3F, 0x3F, 0x3F);
		_screen->setInterfacePalette(_screen->getPalette(1), 0x3F, 0x3F, 0x3F);
	} else {
		_screen->setPaletteIndex(0xFE, 60, 60, 0);
	}

	for (int i = 0; i < 6; i++) {
		_menuButtonData[i].data0Val1 = _menuButtonData[i].data1Val1 = _menuButtonData[i].data2Val1 = 4;
		_menuButtonData[i].data0Callback = _redrawShadedButtonFunctor;
		_menuButtonData[i].data1Callback = _redrawButtonFunctor;
		_menuButtonData[i].data2Callback = _redrawButtonFunctor;
	}

	_screen->savePageToDisk("SEENPAGE.TMP", 0);
	fadePalette();

	for (int i = 0; i < 5; i++)
		initMenuLayout(_menu[i]);

	_menuRestoreScreen = true;
	_keyPressed.reset();

	_toplevelMenu = 0;
	if (_vm->_menuDirectlyToLoad) {
		loadGameMenu(0);
	} else {
		if (!caller)
			_toplevelMenu = 4;

		initMenu(_menu[_toplevelMenu]);
		updateAllMenuButtons();
	}

	while (_displayMenu && !_vm->shouldQuit()) {
		processHighlights(_menu[_toplevelMenu]);
		getInput();
	}

	if (_menuRestoreScreen) {
		restorePalette();
		_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	} else {
		_screen->deletePageFromDisk(0);
	}

	return 0;
}

void GUI_LoK::getInput() {
	uint32 now = _vm->_system->getMillis();

	_vm->checkInput(_menuButtonList);
	_vm->removeInputTop();

	if (now - _lastScreenUpdate > 50) {
		_vm->_system->updateScreen();
		_lastScreenUpdate = now;
	}

	_vm->_system->delayMillis(3);
}

int GUI_LoK::resumeGame(Button *button) {
	updateMenuButton(button);
	_displayMenu = false;

	return 0;
}

void GUI_LoK::setupSavegames(Menu &menu, int num) {
	Common::InSaveFile *in;
	uint8 startSlot;
	assert(num <= 5);

	if (_savegameOffset == 0) {
		menu.item[0].itemString = _specialSavegameString;
		menu.item[0].enabled = 1;
		menu.item[0].saveSlot = 0;
		startSlot = 1;
	} else {
		startSlot = 0;
	}

	for (int i = startSlot; i < num; ++i)
		menu.item[i].enabled = 0;

	KyraEngine_LoK::SaveHeader header;
	for (int i = startSlot; i < num && uint(_savegameOffset + i) < _saveSlots.size(); i++) {
		if ((in = _vm->openSaveForReading(_vm->getSavegameFilename(_saveSlots[i + _savegameOffset]), header))) {
			Common::strlcpy(_savegameNames[i], header.description.c_str(), ARRAYSIZE(_savegameNames[0]));

			// Trim long GMM save descriptions to fit our save slots
			_screen->_charWidth = -2;
			int fC = _screen->getTextWidth(_savegameNames[i]);
			while (_savegameNames[i][0] && (fC > 240)) {
				_savegameNames[i][strlen(_savegameNames[i]) - 1] = 0;
				fC = _screen->getTextWidth(_savegameNames[i]);
			}
			_screen->_charWidth = 0;

			Util::convertISOToDOS(_savegameNames[i]);

			menu.item[i].itemString = _savegameNames[i];
			menu.item[i].enabled = 1;
			menu.item[i].saveSlot = _saveSlots[i + _savegameOffset];
			delete in;
		}
	}
}

int GUI_LoK::saveGameMenu(Button *button) {
	updateSaveFileList(_vm->_targetName);

	updateMenuButton(button);
	_menu[2].item[5].enabled = true;

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	_menu[2].menuNameString = _vm->_guiStrings[8]; // Select a position to save to:
	_specialSavegameString = _vm->_guiStrings[_vm->gameFlags().platform == Common::kPlatformPC98 ? 10 : 9]; // [ EMPTY SLOT ]
	for (int i = 0; i < 5; i++)
		_menu[2].item[i].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::saveGame);

	_savegameOffset = 0;
	setupSavegames(_menu[2], 5);

	initMenu(_menu[2]);
	updateAllMenuButtons();

	_displaySubMenu = true;
	_cancelSubMenu = false;

	while (_displaySubMenu && !_vm->shouldQuit()) {
		processHighlights(_menu[2]);
		getInput();
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	if (_cancelSubMenu) {
		initMenu(_menu[0]);
		updateAllMenuButtons();
	} else {
		_displayMenu = false;
	}
	return 0;
}

int GUI_LoK::loadGameMenu(Button *button) {
	updateSaveFileList(_vm->_targetName);

	if (_vm->_menuDirectlyToLoad) {
		_menu[2].item[5].enabled = false;
	} else {
		updateMenuButton(button);
		_menu[2].item[5].enabled = true;
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	_specialSavegameString = _vm->_newGameString[0]; //[ START A NEW GAME ]
	_menu[2].menuNameString = _vm->_guiStrings[7]; // Which game would you like to reload?
	for (int i = 0; i < 5; i++)
		_menu[2].item[i].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::loadGame);

	_savegameOffset = 0;
	setupSavegames(_menu[2], 5);

	initMenu(_menu[2]);
	updateAllMenuButtons();

	_displaySubMenu = true;
	_cancelSubMenu = false;

	_vm->_gameToLoad = -1;

	while (_displaySubMenu && !_vm->shouldQuit()) {
		processHighlights(_menu[2]);
		getInput();
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	if (_cancelSubMenu) {
		initMenu(_menu[_toplevelMenu]);
		updateAllMenuButtons();
	} else {
		restorePalette();
		if (_vm->_gameToLoad != -1)
			_vm->loadGameStateCheck(_vm->_gameToLoad);
		_displayMenu = false;
		_menuRestoreScreen = false;
	}
	return 0;
}

void GUI_LoK::redrawTextfield() {
	_screen->fillRect(38, 91, 287, 102, _vm->gameFlags().platform == Common::kPlatformAmiga ? 18 : 250);
	_text->printText(_savegameName, 38, 92, 253, 0, 0);

	_screen->_charWidth = -2;
	int width = _screen->getTextWidth(_savegameName);
	_screen->fillRect(39 + width, 93, 45 + width, 100, _vm->gameFlags().platform == Common::kPlatformAmiga ? 31 : 254);
	_screen->_charWidth = 0;

	_screen->updateScreen();
}

void GUI_LoK::updateSavegameString() {
	int length;

	if (_keyPressed.keycode) {
		length = strlen(_savegameName);
		_screen->_charWidth = -2;
		int width = _screen->getTextWidth(_savegameName) + 7;
		_screen->_charWidth = 0;

		char inputKey = _keyPressed.ascii;
		Util::convertISOToDOS(inputKey);

		if ((uint8)inputKey > 31 && (uint8)inputKey < (_vm->gameFlags().lang == Common::JA_JPN ? 128 : 226)) {
			if ((length < ARRAYSIZE(_savegameName) - 1) && (width <= 240)) {
				_savegameName[length] = inputKey;
				_savegameName[length + 1] = 0;
				redrawTextfield();
			}
		} else if (_keyPressed.keycode == Common::KEYCODE_BACKSPACE ||
		           _keyPressed.keycode == Common::KEYCODE_DELETE) {
			if (length > 0) {
				_savegameName[length - 1] = 0;
				redrawTextfield();
			}
		} else if (_keyPressed.keycode == Common::KEYCODE_RETURN ||
		           _keyPressed.keycode == Common::KEYCODE_KP_ENTER) {
			_displaySubMenu = false;
		}
	}

	_keyPressed.reset();
}

int GUI_LoK::saveGame(Button *button) {
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	updateMenuButton(button);
	_vm->_gameToLoad = _menu[2].item[button->index - 0xC].saveSlot;

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	initMenu(_menu[3]);
	updateAllMenuButtons();

	_displaySubMenu = true;
	_cancelSubMenu = false;

	Screen::FontId cf = _screen->setFont(Screen::FID_8_FNT);

	if (_savegameOffset == 0 && _vm->_gameToLoad == 0) {
		_savegameName[0] = 0;
	} else {
		for (int i = 0; i < 5; i++) {
			if (_menu[2].item[i].saveSlot == _vm->_gameToLoad) {
				Common::strlcpy(_savegameName, _menu[2].item[i].itemString, 31);
				break;
			}
		}
	}
	redrawTextfield();

	_screen->setFont(cf);

	while (_displaySubMenu && !_vm->shouldQuit()) {
		checkTextfieldInput();
		cf = _screen->setFont(Screen::FID_8_FNT);
		updateSavegameString();
		_screen->setFont(cf);
		processHighlights(_menu[3]);
	}

	if (_cancelSubMenu) {
		_displaySubMenu = true;
		_cancelSubMenu = false;
		initMenu(_menu[2]);
		updateAllMenuButtons();
	} else {
		if (_savegameOffset == 0 && _vm->_gameToLoad == 0)
			_vm->_gameToLoad = getNextSavegameSlot();
		if (_vm->_gameToLoad > 0) {
			Util::convertDOSToISO(_savegameName);

			Graphics::Surface thumb;
			createScreenThumbnail(thumb);
			_vm->saveGameStateIntern(_vm->_gameToLoad, _savegameName, &thumb);
			thumb.free();
		}
	}

	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	return 0;
}

int GUI_LoK::savegameConfirm(Button *button) {
	updateMenuButton(button);
	_displaySubMenu = false;

	return 0;
}

int GUI_LoK::loadGame(Button *button) {
	updateMenuButton(button);
	_displaySubMenu = false;
	_vm->_gameToLoad = _menu[2].item[button->index - 0xC].saveSlot;

	return 0;
}

int GUI_LoK::cancelSubMenu(Button *button) {
	updateMenuButton(button);
	_displaySubMenu = false;
	_cancelSubMenu = true;

	return 0;
}

int GUI_LoK::quitPlaying(Button *button) {
	updateMenuButton(button);

	if (quitConfirm(_vm->_guiStrings[_vm->gameFlags().platform == Common::kPlatformPC98 ? 15 : 14])) { // Are you sure you want to quit playing?
		_vm->quitGame();
	} else {
		initMenu(_menu[_toplevelMenu]);
		updateAllMenuButtons();
	}

	return 0;
}

bool GUI_LoK::quitConfirm(const char *str) {
	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	_menu[1].menuNameString = str;
	initMenuLayout(_menu[1]);
	initMenu(_menu[1]);

	_displaySubMenu = true;
	_cancelSubMenu = true;

	while (_displaySubMenu && !_vm->shouldQuit()) {
		processHighlights(_menu[1]);
		getInput();
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	return !_cancelSubMenu;
}

int GUI_LoK::quitConfirmYes(Button *button) {
	updateMenuButton(button);
	_displaySubMenu = false;
	_cancelSubMenu = false;

	return 0;
}

int GUI_LoK::quitConfirmNo(Button *button) {
	updateMenuButton(button);
	_displaySubMenu = false;
	_cancelSubMenu = true;

	return 0;
}

int GUI_LoK::gameControlsMenu(Button *button) {
	_vm->readSettings();

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	if (_vm->gameFlags().isTalkie) {
		//_menu[5].width = 230;

		for (int i = 0; i < 5; i++) {
			//_menu[5].item[i].labelX = 24;
			//_menu[5].item[i].x = 115;
			//_menu[5].item[i].width = 94;
		}

		_menu[5].item[3].labelString = _voiceTextString; //"Voice / Text "
		_menu[5].item[3].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::controlsChangeVoice);

	} else {
		//_menu[5].height = 136;
		//_menu[5].item[5].y = 110;
		_menu[5].item[4].enabled = 0;
		_menu[5].item[3].labelString = _textSpeedString; // "Text speed "
		_menu[5].item[3].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::controlsChangeText);
	}

	setupControls(_menu[5]);

	updateAllMenuButtons();

	_displaySubMenu = true;
	_cancelSubMenu = false;

	while (_displaySubMenu && !_vm->shouldQuit()) {
		processHighlights(_menu[5]);
		getInput();
	}

	_screen->loadPageFromDisk("SEENPAGE.TMP", 0);
	_screen->savePageToDisk("SEENPAGE.TMP", 0);

	if (_cancelSubMenu) {
		initMenu(_menu[_toplevelMenu]);
		updateAllMenuButtons();
	}
	return 0;
}

void GUI_LoK::setupControls(Menu &menu) {
	switch (_vm->_configMusic) {
	case 0:
		menu.item[0].itemString = _offString; //"Off"
		break;
	case 1:
		menu.item[0].itemString = _onString; //"On"
		break;
	case 2:
		menu.item[0].itemString = _onCDString; //"On + CD"
		break;
	}

	if (_vm->_configSounds)
		menu.item[1].itemString = _onString; //"On"
	else
		menu.item[1].itemString = _offString; //"Off"


	switch (_vm->_configWalkspeed) {
	case 0:
		menu.item[2].itemString = _vm->_configStrings[0]; //"Slowest"
		break;
	case 1:
		menu.item[2].itemString = _vm->_configStrings[1]; //"Slow"
		break;
	case 2:
		menu.item[2].itemString = _vm->_configStrings[2]; //"Normal"
		break;
	case 3:
		menu.item[2].itemString = _vm->_configStrings[3]; //"Fast"
		break;
	case 4:
		menu.item[2].itemString = _vm->_configStrings[4]; //"Fastest"
		break;
	default:
		menu.item[2].itemString = "ERROR";
	}

	int textControl = 3;
	int clickableOffset = 8;
	if (_vm->gameFlags().isTalkie) {
		textControl = 4;
		clickableOffset = 11;

		if (_vm->_configVoice == 0) {
			menu.item[4].enabled = 1;
			menu.item[4].labelString = _textSpeedString;
		} else {
			menu.item[4].enabled = 0;
			menu.item[4].labelString = 0;
		}

		switch (_vm->_configVoice) {
		case 0:
			menu.item[3].itemString = _vm->_configStrings[5]; //"Text only"
			break;
		case 1:
			menu.item[3].itemString = _vm->_configStrings[6]; //"Voice only"
			break;
		case 2:
			menu.item[3].itemString = _vm->_configStrings[7]; //"Voice & Text"
			break;
		default:
			menu.item[3].itemString = "ERROR";
		}
	} else {
		if (_vm->gameFlags().platform == Common::kPlatformAmiga)
			clickableOffset = 5;

		menu.item[4].enabled = 0;
		menu.item[4].labelString = 0;
	}

	switch (_vm->_configTextspeed) {
	case 0:
		menu.item[textControl].itemString = _vm->_configStrings[1]; //"Slow"
		break;
	case 1:
		menu.item[textControl].itemString = _vm->_configStrings[2]; //"Normal"
		break;
	case 2:
		menu.item[textControl].itemString = _vm->_configStrings[3]; //"Fast"
		break;
	case 3:
		menu.item[textControl].itemString = _vm->_configStrings[clickableOffset]; //"Clickable"
		break;
	default:
		menu.item[textControl].itemString = "ERROR";
	}

	initMenuLayout(menu);
	initMenu(menu);
}

int GUI_LoK::controlsChangeMusic(Button *button) {
	updateMenuButton(button);

	_vm->_configMusic = (_vm->_configMusic + 1) % ((_vm->gameFlags().platform == Common::kPlatformFMTowns) ? 3 : 2);
	setupControls(_menu[5]);
	return 0;
}

int GUI_LoK::controlsChangeSounds(Button *button) {
	updateMenuButton(button);

	_vm->_configSounds = !_vm->_configSounds;
	setupControls(_menu[5]);
	return 0;
}

int GUI_LoK::controlsChangeWalk(Button *button) {
	updateMenuButton(button);

	_vm->_configWalkspeed = (_vm->_configWalkspeed + 1) % 5;
	_vm->setWalkspeed(_vm->_configWalkspeed);
	setupControls(_menu[5]);
	return 0;
}

int GUI_LoK::controlsChangeText(Button *button) {
	updateMenuButton(button);

	_vm->_configTextspeed = (_vm->_configTextspeed + 1) % 4;
	setupControls(_menu[5]);
	return 0;
}

int GUI_LoK::controlsChangeVoice(Button *button) {
	updateMenuButton(button);

	_vm->_configVoice = (_vm->_configVoice + 1) % 3;
	setupControls(_menu[5]);
	return 0;
}

int GUI_LoK::controlsApply(Button *button) {
	_vm->writeSettings();
	return cancelSubMenu(button);
}

int GUI_LoK::scrollUp(Button *button) {
	updateMenuButton(button);

	if (_savegameOffset > 0) {
		_savegameOffset--;
		setupSavegames(_menu[2], 5);
		initMenu(_menu[2]);
	}
	return 0;
}

int GUI_LoK::scrollDown(Button *button) {
	updateMenuButton(button);

	_savegameOffset++;
	if (uint(_savegameOffset + 5) >= _saveSlots.size())
		_savegameOffset = MAX<int>(_saveSlots.size() - 5, 0);
	setupSavegames(_menu[2], 5);
	initMenu(_menu[2]);

	return 0;
}

void GUI_LoK::fadePalette() {
	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		return;

	static const int16 menuPalIndexes[] = {248, 249, 250, 251, 252, 253, 254, -1};
	int index = 0;

	_screen->copyPalette(2, 0);

	for (int i = 0; i < 768; i++)
		_screen->getPalette(0)[i] >>= 1;

	while (menuPalIndexes[index] != -1) {
		_screen->getPalette(0).copy(_screen->getPalette(2), menuPalIndexes[index], 1);
		++index;
	}

	_screen->fadePalette(_screen->getPalette(0), 2);
}

void GUI_LoK::restorePalette() {
	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		return;

	_screen->copyPalette(0, 2);
	_screen->fadePalette(_screen->getPalette(0), 2);
}

#pragma mark -

void KyraEngine_LoK::drawAmulet() {
	static const int16 amuletTable1[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x150, 0x155, 0x15A, 0x15F, 0x164, 0x145, -1};
	static const int16 amuletTable3[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x14F, 0x154, 0x159, 0x15E, 0x163, 0x144, -1};
	static const int16 amuletTable2[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x152, 0x157, 0x15C, 0x161, 0x166, 0x147, -1};
	static const int16 amuletTable4[] = {0x167, 0x162, 0x15D, 0x158, 0x153, 0x151, 0x156, 0x15B, 0x160, 0x165, 0x146, -1};

	resetGameFlag(0xF1);
	_screen->hideMouse();

	int i = 0;
	while (amuletTable1[i] != -1) {
		if (queryGameFlag(87))
			_screen->drawShape(0, _shapes[amuletTable1[i]], _amuletX[0], _amuletY[0], 0, 0);

		if (queryGameFlag(89))
			_screen->drawShape(0, _shapes[amuletTable2[i]], _amuletX[1], _amuletY[1], 0, 0);

		if (queryGameFlag(86))
			_screen->drawShape(0, _shapes[amuletTable3[i]], _amuletX[2], _amuletY[2], 0, 0);

		if (queryGameFlag(88))
			_screen->drawShape(0, _shapes[amuletTable4[i]], _amuletX[3], _amuletY[3], 0, 0);

		_screen->updateScreen();
		delayWithTicks(3);
		i++;
	}
	_screen->showMouse();
}

} // End of namespace Kyra
