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

#include "kyra/kyra_hof.h"
#include "kyra/resource.h"
#include "kyra/text_hof.h"
#include "kyra/timer.h"
#include "kyra/debugger.h"
#include "kyra/util.h"
#include "kyra/sound.h"

#include "common/system.h"
#include "common/config-manager.h"

namespace Kyra {

const KyraEngine_v2::EngineDesc KyraEngine_HoF::_hofEngineDesc = {
	// Generic shape related
	64,
	KyraEngine_HoF::_characterFrameTable,

	// Scene script
	8,

	// Animation script specific
	33,

	// Item specific
	175
};

KyraEngine_HoF::KyraEngine_HoF(OSystem *system, const GameFlags &flags) : KyraEngine_v2(system, flags, _hofEngineDesc), _updateFunctor(this, &KyraEngine_HoF::update) {
	_screen = 0;
	_text = 0;

	_gamePlayBuffer = 0;
	_cCodeBuffer = _optionsBuffer = _chapterBuffer = 0;

	_overwriteSceneFacing = false;
	_mainCharX = _mainCharY = -1;
	_drawNoShapeFlag = false;
	_charPalEntry = 0;
	_itemInHand = kItemNone;
	_unkSceneScreenFlag1 = false;
	_noScriptEnter = true;
	_currentChapter = 0;
	_newChapterFile = 1;
	_oldTalkFile = -1;
	_currentTalkFile = 0;
	_lastSfxTrack = -1;
	_mouseState = -1;
	_unkHandleSceneChangeFlag = false;
	_pathfinderFlag = 0;
	_mouseX = _mouseY = 0;

	_nextIdleAnim = 0;
	_lastIdleScript = -1;
	_useSceneIdleAnim = false;

	_currentTalkSections.STATim = 0;
	_currentTalkSections.TLKTim = 0;
	_currentTalkSections.ENDTim = 0;

	memset(&_invWsa, 0, sizeof(_invWsa));
	_itemAnimDefinition = 0;
	_nextAnimItem = 0;

	for (int i = 0; i < 15; i++)
		memset(&_activeItemAnim[i], 0, sizeof(ActiveItemAnim));

	_colorCodeFlag1 = 0;
	_colorCodeFlag2 = -1;
	_scriptCountDown = 0;
	_dbgPass = 0;

	_gamePlayBuffer = 0;
	_unkBuf500Bytes = 0;
	_inventorySaved = false;
	_unkBuf200kByte = 0;
	memset(&_sceneShapeTable, 0, sizeof(_sceneShapeTable));

	_talkObjectList = 0;
	_shapeDescTable = 0;
	_gfxBackUpRect = 0;
	_sceneList = 0;
	memset(&_sceneAnimMovie, 0, sizeof(_sceneAnimMovie));
	memset(&_wsaSlots, 0, sizeof(_wsaSlots));
	memset(&_buttonShapes, 0, sizeof(_buttonShapes));

	_configTextspeed = 50;

	_inventoryButtons = _buttonList = 0;

	_dlgBuffer = 0;
	_conversationState = new int8 *[19];
	for (int i = 0; i < 19; i++)
		_conversationState[i] = new int8[14];
	_npcTalkChpIndex = _npcTalkDlgIndex = -1;
	_mainCharacter.dlgIndex = 0;
	setDlgIndex(-1);

	_bookMaxPage = 6;
	_bookCurPage = 0;
	_bookNewPage = 0;
	_bookBkgd = 0;

	_cauldronState = 0;
	_cauldronUseCount = 0;
	memset(_cauldronStateTables, 0, sizeof(_cauldronStateTables));

	_menuDirectlyToLoad = false;
	_chatIsNote = false;
	memset(&_npcScriptData, 0, sizeof(_npcScriptData));

	_setCharPalFinal = false;
	_useCharPal = false;

	memset(_characterFacingCountTable, 0, sizeof(_characterFacingCountTable));
}

KyraEngine_HoF::~KyraEngine_HoF() {
	cleanup();

	delete _screen;
	delete _text;
	delete _gui;
	delete _tim;
	_text = 0;
	delete _invWsa.wsa;

	delete[] _dlgBuffer;
	for (int i = 0; i < 19; i++)
		delete[] _conversationState[i];
	delete[] _conversationState;

	for (Common::Array<const TIMOpcode *>::iterator i = _timOpcodes.begin(); i != _timOpcodes.end(); ++i)
		delete *i;
	_timOpcodes.clear();
}

void KyraEngine_HoF::pauseEngineIntern(bool pause) {
	KyraEngine_v2::pauseEngineIntern(pause);

	seq_pausePlayer(pause);

	if (!pause) {
		uint32 pausedTime = _system->getMillis() - _pauseStart;
		_pauseStart = 0;

		_nextIdleAnim += pausedTime;
		_tim->refreshTimersAfterPause(pausedTime);
	}
}

Common::Error KyraEngine_HoF::init() {
	_screen = new Screen_HoF(this, _system);
	assert(_screen);
	_screen->setResolution();

	_debugger = new Debugger_HoF(this);
	assert(_debugger);

	KyraEngine_v1::init();
	initStaticResource();

	_text = new TextDisplayer_HoF(this, _screen);
	assert(_text);
	_gui = new GUI_HoF(this);
	assert(_gui);
	_gui->initStaticData();
	_tim = new TIMInterpreter(this, _screen, _system);
	assert(_tim);

	if (_flags.isDemo && !_flags.isTalkie) {
		_screen->loadFont(_screen->FID_8_FNT, "FONT9P.FNT");
	} else {
		_screen->loadFont(_screen->FID_6_FNT, "6.FNT");
		_screen->loadFont(_screen->FID_8_FNT, "8FAT.FNT");
		_screen->loadFont(_screen->FID_BOOKFONT_FNT, "BOOKFONT.FNT");
	}
	_screen->setFont(_flags.lang == Common::JA_JPN ? Screen::FID_SJIS_FNT : _screen->FID_8_FNT);

	_screen->setAnimBlockPtr(3504);
	_screen->setScreenDim(0);

	if (!_sound->init())
		error("Couldn't init sound");

	// No mouse display in demo
	if (_flags.isDemo && !_flags.isTalkie)
		return Common::kNoError;

	_res->exists("PWGMOUSE.SHP", true);
	uint8 *shapes = _res->fileData("PWGMOUSE.SHP", 0);
	assert(shapes);

	for (int i = 0; i < 2; i++)
		addShapeToPool(shapes, i, i);

	delete[] shapes;

	_screen->setMouseCursor(0, 0, getShapePtr(0));
	return Common::kNoError;
}

Common::Error KyraEngine_HoF::go() {
	int menuChoice = 0;

	if (_gameToLoad == -1) {
		if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98)
			seq_showStarcraftLogo();

		if (_flags.isDemo && !_flags.isTalkie) {
			menuChoice = seq_playDemo();
		} else {
			menuChoice = seq_playIntro();
		}
	} else {
		menuChoice = 1;
	}

	_res->unloadAllPakFiles();

	if (menuChoice != 4) {
		// load just the pak files needed for ingame
		_staticres->loadStaticResourceFile();

		if (_flags.platform == Common::kPlatformPC && _flags.isTalkie) {
			if (!_res->loadFileList("FILEDATA.FDT"))
				error("couldn't load 'FILEDATA.FDT'");
		} else {
			_res->loadFileList(_ingamePakList, _ingamePakListSize);
		}

		if (_flags.platform == Common::kPlatformPC98) {
			_res->loadPakFile("AUDIO.PAK");
			_sound->loadSoundFile("SOUND.DAT");
		}
	}

	_menuDirectlyToLoad = (menuChoice == 3) ? true : false;
	_menuDirectlyToLoad &= saveFileLoadable(0);

	if (menuChoice & 1) {
		startup();
		if (!shouldQuit())
			runLoop();
		cleanup();

		if (_showOutro)
			seq_playOutro();
	}

	return Common::kNoError;
}

void KyraEngine_HoF::startup() {
	_sound->selectAudioResourceSet(kMusicIngame);
	// The track map is exactly the same
	// for FM-TOWNS and DOS
	_trackMap = _dosTrackMap;
	_trackMapSize = _dosTrackMapSize;

	allocAnimObjects(1, 10, 30);

	_screen->_curPage = 0;

	memset(_sceneShapeTable, 0, sizeof(_sceneShapeTable));
	_gamePlayBuffer = new uint8[46080];
	_unkBuf500Bytes = new uint8[500];

	loadMouseShapes();
	loadItemShapes();

	_screen->setMouseCursor(0, 0, getShapePtr(0));

	_screenBuffer = new uint8[64000];
	_unkBuf200kByte = new uint8[200000];

	loadChapterBuffer(_newChapterFile);

	loadCCodeBuffer("C_CODE.XXX");

	if (_flags.isTalkie) {
		loadOptionsBuffer("OPTIONS.XXX");

		showMessageFromCCode(265, 150, 0);
		_screen->updateScreen();
		openTalkFile(0);
		_currentTalkFile = 1;
		openTalkFile(1);
	} else {
		_optionsBuffer = _cCodeBuffer;
	}

	showMessage(0, 207);

	_screen->setShapePages(5, 3);

	_mainCharacter.height = 0x30;
	_mainCharacter.facing = 4;
	_mainCharacter.animFrame = 0x12;

	memset(_sceneAnims, 0, sizeof(_sceneAnims));
	for (int i = 0; i < ARRAYSIZE(_sceneAnimMovie); ++i)
		_sceneAnimMovie[i] = new WSAMovie_v2(this);
	memset(_wsaSlots, 0, sizeof(_wsaSlots));
	for (int i = 0; i < ARRAYSIZE(_wsaSlots); ++i)
		_wsaSlots[i] = new WSAMovie_v2(this);

	_screen->_curPage = 0;

	_talkObjectList = new TalkObject[72];
	memset(_talkObjectList, 0, sizeof(TalkObject)*72);
	_shapeDescTable = new ShapeDesc[55];
	memset(_shapeDescTable, 0, sizeof(ShapeDesc)*55);

	for (int i = 9; i <= 32; ++i) {
		_shapeDescTable[i-9].width = 30;
		_shapeDescTable[i-9].height = 55;
		_shapeDescTable[i-9].xAdd = -15;
		_shapeDescTable[i-9].yAdd = -50;
	}

	for (int i = 19; i <= 24; ++i) {
		_shapeDescTable[i-9].width = 53;
		_shapeDescTable[i-9].yAdd = -51;
	}

	_gfxBackUpRect = new uint8[_screen->getRectSize(32, 32)];
	initItemList(30);
	loadButtonShapes();
	resetItemList();
	_characterShapeFile = 1;
	loadCharacterShapes(_characterShapeFile);
	initInventoryButtonList();
	setupLangButtonShapes();
	loadInventoryShapes();

	_screen->loadPalette("PALETTE.COL", _screen->getPalette(0));
	_screen->loadBitmap("_PLAYFLD.CPS", 3, 3, 0);
	_screen->copyPage(3, 0);

	clearAnimObjects();

	for (int i = 0; i < 19; ++i)
		memset(_conversationState[i], -1, sizeof(int8)*14);
	clearCauldronTable();
	memset(_inputColorCode, -1, sizeof(_inputColorCode));
	memset(_newSceneDlgState, 0, sizeof(_newSceneDlgState));
	for (int i = 0; i < 23; ++i)
		resetCauldronStateTable(i);

	_sceneList = new SceneDesc[86];
	memset(_sceneList, 0, sizeof(SceneDesc)*86);
	_sceneListSize = 86;
	runStartScript(1, 0);
	loadNPCScript();

	if (_gameToLoad == -1) {
		snd_playWanderScoreViaMap(52, 1);
		enterNewScene(_mainCharacter.sceneId, _mainCharacter.facing, 0, 0, 1);
		saveGameStateIntern(0, "New Game", 0);
	} else {
		loadGameStateCheck(_gameToLoad);
	}

	_screen->showMouse();

	if (_menuDirectlyToLoad)
		(*_inventoryButtons[0].buttonCallback)(&_inventoryButtons[0]);

	setNextIdleAnimTimer();
	setWalkspeed(_configWalkspeed);
}

void KyraEngine_HoF::runLoop() {
	// Initialize debugger since how it should be fully usable
	_debugger->initialize();

	_screen->updateScreen();

	_runFlag = true;
	while (!shouldQuit() && _runFlag) {
		if (_deathHandler >= 0) {
			removeHandItem();
			delay(5);
			_drawNoShapeFlag = 0;
			_gui->optionsButton(0);
			_deathHandler = -1;

			if (!_runFlag || shouldQuit())
				break;
		}

		if (_system->getMillis() > _nextIdleAnim)
			showIdleAnim();

		if (queryGameFlag(0x159)) {
			dinoRide();
			resetGameFlag(0x159);
		}

		if (queryGameFlag(0x124) && !queryGameFlag(0x125)) {
			_mainCharacter.animFrame = 32;
			enterNewScene(39, -1, 0, 0, 0);
		}

		if (queryGameFlag(0xD8)) {
			resetGameFlag(0xD8);
			if (_mainCharacter.sceneId == 34) {
				if (queryGameFlag(0xD1)) {
					initTalkObject(28);
					npcChatSequence(getTableString(0xFA, _cCodeBuffer, 1), 28, 0x83, 0xFA);
					deinitTalkObject(28);
					enterNewScene(35, 4, 0, 0, 0);
				} else if (queryGameFlag(0xD0)) {
					initTalkObject(29);
					npcChatSequence(getTableString(0xFB, _cCodeBuffer, 1), 29, 0x83, 0xFB);
					deinitTalkObject(29);
					enterNewScene(33, 6, 0, 0, 0);
				}
			}
		}

		int inputFlag = checkInput(_buttonList, true);
		removeInputTop();

		update();

		if (inputFlag == 198 || inputFlag == 199) {
			_savedMouseState = _mouseState;
			handleInput(_mouseX, _mouseY);
		}

		//if (queryGameFlag(0x1EE) && inputFlag)
		//	sub_13B19(inputFlag);

		_system->delayMillis(10);
	}
}

void KyraEngine_HoF::handleInput(int x, int y) {
	setNextIdleAnimTimer();
	if (_unk5) {
		_unk5 = 0;
		return;
	}

	if (!_screen->isMouseVisible())
		return;

	if (_savedMouseState == -2) {
		snd_playSoundEffect(13);
		return;
	}

	setNextIdleAnimTimer();

	if (x <= 6 || x >= 312 || y <= 6 || y >= 135) {
		bool exitOk = false;
		assert(_savedMouseState + 6 >= 0);
		switch (_savedMouseState + 6) {
		case 0:
			if (_sceneExit1 != 0xFFFF)
				exitOk = true;
			break;

		case 1:
			if (_sceneExit2 != 0xFFFF)
				exitOk = true;
			break;

		case 2:
			if (_sceneExit3 != 0xFFFF)
				exitOk = true;
			break;

		case 3:
			if (_sceneExit4 != 0xFFFF)
				exitOk = true;
			break;

		default:
			break;
		}

		if (exitOk) {
			inputSceneChange(x, y, 1, 1);
			return;
		}
	}

	if (checkCharCollision(x, y) && _savedMouseState >= -1) {
		runSceneScript2();
		return;
	} else if (pickUpItem(x, y)) {
		return;
	} else {
		int skipHandling = 0;

		if (checkItemCollision(x, y) == -1) {
			resetGameFlag(0x1EF);
			skipHandling = handleInputUnkSub(x, y) ? 1 : 0;

			if (queryGameFlag(0x1EF)) {
				resetGameFlag(0x1EF);
				return;
			}

			if (_unk5) {
				_unk5 = 0;
				return;
			}
		}

		if (_deathHandler > -1)
			skipHandling = 1;

		if (skipHandling)
			return;

		if (checkCharCollision(x, y)) {
			runSceneScript2();
			return;
		}

		if (_itemInHand >= 0) {
			if (y > 136)
				return;

			dropItem(0, _itemInHand, x, y, 1);
		} else {
			if (_savedMouseState == -2 || y > 135)
				return;

			if (!_unk5) {
				inputSceneChange(x, y, 1, 1);
				return;
			}

			_unk5 = 0;
		}
	}
}

bool KyraEngine_HoF::handleInputUnkSub(int x, int y) {
	if (y > 143 || _deathHandler > -1 || queryGameFlag(0x164))
		return false;

	if (_mouseState <= -3 && findItem(_mainCharacter.sceneId, 13) >= 0) {
		updateCharFacing();
		objectChat(getTableString(0xFC, _cCodeBuffer, 1), 0, 0x83, 0xFC);
		return true;
	} else {
		_emc->init(&_sceneScriptState, &_sceneScriptData);

		_sceneScriptState.regs[1] = x;
		_sceneScriptState.regs[2] = y;
		_sceneScriptState.regs[3] = 0;
		_sceneScriptState.regs[4] = _itemInHand;

		_emc->start(&_sceneScriptState, 1);

		while (_emc->isValid(&_sceneScriptState))
			_emc->run(&_sceneScriptState);

		if (queryGameFlag(0x1ED)) {
			_sound->beginFadeOut();
			_screen->fadeToBlack();
			_showOutro = true;
			_runFlag = false;
		}

		return _sceneScriptState.regs[3] != 0;
	}
}

void KyraEngine_HoF::update() {
	updateInput();

	refreshAnimObjectsIfNeed();
	updateMouse();
	updateSpecialSceneScripts();
	_timer->update();
	updateItemAnimations();
	updateInvWsa();
	fadeMessagePalette();
	_screen->updateScreen();
}

void KyraEngine_HoF::updateWithText() {
	updateInput();

	updateMouse();
	fadeMessagePalette();
	updateSpecialSceneScripts();
	_timer->update();
	updateItemAnimations();
	updateInvWsa();
	restorePage3();
	drawAnimObjects();

	if (_chatTextEnabled && _chatText) {
		int pageBackUp = _screen->_curPage;
		_screen->_curPage = 2;
		objectChatPrintText(_chatText, _chatObject);
		_screen->_curPage = pageBackUp;
	}

	refreshAnimObjects(0);
	_screen->updateScreen();
}

void KyraEngine_HoF::updateMouse() {
	int shapeIndex = 0;
	int type = 0;
	int xOffset = 0, yOffset = 0;
	Common::Point mouse = getMousePos();

	if (mouse.y <= 145) {
		if (mouse.x <= 6) {
			if (_sceneExit4 != 0xFFFF) {
				type = -3;
				shapeIndex = 4;
				xOffset = 1;
				yOffset = 5;
			} else {
				type = -2;
			}
		} else if (mouse.x >= 312) {
			if (_sceneExit2 != 0xFFFF) {
				type = -5;
				shapeIndex = 2;
				xOffset = 7;
				yOffset = 5;
			} else {
				type = -2;
			}
		} else if (mouse.y >= 135) {
			if (_sceneExit3 != 0xFFFF) {
				type = -4;
				shapeIndex = 3;
				xOffset = 5;
				yOffset = 10;
			} else {
				type = -2;
			}
		} else if (mouse.y <= 6) {
			if (_sceneExit1 != 0xFFFF) {
				type = -6;
				shapeIndex = 1;
				xOffset = 5;
				yOffset = 1;
			} else {
				type = -2;
			}
		}
	}

	for (int i = 0; i < _specialExitCount; ++i) {
		if (checkSpecialSceneExit(i, mouse.x, mouse.y)) {
			switch (_specialExitTable[20+i]) {
			case 0:
				type = -6;
				shapeIndex = 1;
				xOffset = 5;
				yOffset = 1;
				break;

			case 2:
				type = -5;
				shapeIndex = 2;
				xOffset = 7;
				yOffset = 5;
				break;

			case 4:
				type = -4;
				shapeIndex = 3;
				xOffset = 5;
				yOffset = 7;
				break;

			case 6:
				type = -3;
				shapeIndex = 4;
				xOffset = 1;
				yOffset = 5;
				break;

			default:
				break;
			}
		}
	}

	if (type == -2) {
		shapeIndex = 5;
		xOffset = 5;
		yOffset = 9;
	}

	if (type != 0 && _mouseState != type && _screen->isMouseVisible()) {
		_mouseState = type;
		_screen->setMouseCursor(xOffset, yOffset, getShapePtr(shapeIndex));
	}

	if (type == 0 && _mouseState != _itemInHand && _screen->isMouseVisible()) {
		if ((mouse.y > 145) || (mouse.x > 6 && mouse.x < 312 && mouse.y > 6 && mouse.y < 135)) {
			_mouseState = _itemInHand;
			if (_itemInHand == kItemNone)
				_screen->setMouseCursor(0, 0, getShapePtr(0));
			else
				_screen->setMouseCursor(8, 15, getShapePtr(_itemInHand+64));
		}
	}
}

void KyraEngine_HoF::cleanup() {
	delete[] _inventoryButtons; _inventoryButtons = 0;

	delete[] _gamePlayBuffer; _gamePlayBuffer = 0;
	delete[] _unkBuf500Bytes; _unkBuf500Bytes = 0;
	delete[] _unkBuf200kByte; _unkBuf200kByte = 0;

	freeSceneShapePtrs();

	if (_optionsBuffer != _cCodeBuffer)
		delete[] _optionsBuffer;
	_optionsBuffer = 0;
	delete[] _cCodeBuffer; _cCodeBuffer = 0;
	delete[] _chapterBuffer; _chapterBuffer = 0;

	delete[] _talkObjectList; _talkObjectList = 0;
	delete[] _shapeDescTable; _shapeDescTable = 0;

	delete[] _gfxBackUpRect; _gfxBackUpRect = 0;

	for (int i = 0; i < ARRAYSIZE(_sceneAnimMovie); ++i) {
		delete _sceneAnimMovie[i];
		_sceneAnimMovie[i] = 0;
	}
	for (int i = 0; i < ARRAYSIZE(_wsaSlots); ++i) {
		delete _wsaSlots[i];
		_wsaSlots[i] = 0;
	}
	for (int i = 0; i < ARRAYSIZE(_buttonShapes); ++i) {
		delete[] _buttonShapes[i];
		_buttonShapes[i] = 0;
	}

	_emc->unload(&_npcScriptData);
}

#pragma mark - Localization

void KyraEngine_HoF::loadCCodeBuffer(const char *file) {
	char tempString[13];
	strcpy(tempString, file);
	changeFileExtension(tempString);

	delete[] _cCodeBuffer;
	_cCodeBuffer = _res->fileData(tempString, 0);
}

void KyraEngine_HoF::loadOptionsBuffer(const char *file) {
	char tempString[13];
	strcpy(tempString, file);
	changeFileExtension(tempString);

	delete[] _optionsBuffer;
	_optionsBuffer = _res->fileData(tempString, 0);
}

void KyraEngine_HoF::loadChapterBuffer(int chapter) {
	char tempString[14];

	static const char *const chapterFilenames[] = {
		"CH1.XXX", "CH2.XXX", "CH3.XXX", "CH4.XXX", "CH5.XXX"
	};

	assert(chapter >= 1 && chapter <= ARRAYSIZE(chapterFilenames));
	strcpy(tempString, chapterFilenames[chapter-1]);
	changeFileExtension(tempString);

	delete[] _chapterBuffer;
	_chapterBuffer = _res->fileData(tempString, 0);
	_currentChapter = chapter;
}

void KyraEngine_HoF::changeFileExtension(char *buffer) {
	while (*buffer != '.')
		++buffer;

	++buffer;
	strcpy(buffer, _languageExtension[_lang]);
}

uint8 *KyraEngine_HoF::getTableEntry(uint8 *buffer, int id) {
	return buffer + READ_LE_UINT16(buffer + (id<<1));
}

char *KyraEngine_HoF::getTableString(int id, uint8 *buffer, int decode) {
	char *string = (char *)getTableEntry(buffer, id);

	if (decode && _flags.lang != Common::JA_JPN) {
		Util::decodeString1(string, _internStringBuf);
		Util::decodeString2(_internStringBuf, _internStringBuf);
		string = _internStringBuf;
	}

	return string;
}

const char *KyraEngine_HoF::getChapterString(int id) {
	if (_currentChapter != _newChapterFile)
		loadChapterBuffer(_newChapterFile);

	return getTableString(id, _chapterBuffer, 1);
}

#pragma mark -

void KyraEngine_HoF::showMessageFromCCode(int id, int16 palIndex, int) {
	const char *string = getTableString(id, _cCodeBuffer, 1);
	showMessage(string, palIndex);
}

void KyraEngine_HoF::showMessage(const char *string, int16 palIndex) {
	_shownMessage = string;
	_screen->fillRect(0, 190, 319, 199, 0xCF);

	if (string) {
		if (palIndex != -1 || _fadeMessagePalette) {
			palIndex *= 3;
			memcpy(_messagePal, _screen->getPalette(0).getData() + palIndex, 3);
			_screen->getPalette(0).copy(_screen->getPalette(0), palIndex / 3, 1, 255);
			_screen->setScreenPalette(_screen->getPalette(0));
		}

		int x = _text->getCenterStringX(string, 0, 320);
		_text->printText(string, x, 190, 255, 207, 0);

		setTimer1DelaySecs(7);
	}

	_fadeMessagePalette = false;
}

void KyraEngine_HoF::showChapterMessage(int id, int16 palIndex) {
	showMessage(getChapterString(id), palIndex);
}

void KyraEngine_HoF::updateCommandLineEx(int str1, int str2, int16 palIndex) {
	char buffer[0x51];
	char *src = buffer;

	strcpy(src, getTableString(str1, _cCodeBuffer, 1));

	if (_flags.lang != Common::JA_JPN) {
		while (*src != 0x20)
			++src;
		++src;
		*src = toupper(*src);
	}

	strcpy((char *)_unkBuf500Bytes, src);

	if (str2 > 0) {
		if (_flags.lang != Common::JA_JPN)
			strcat((char *)_unkBuf500Bytes, " ");
		strcat((char *)_unkBuf500Bytes, getTableString(str2, _cCodeBuffer, 1));
	}

	showMessage((char *)_unkBuf500Bytes, palIndex);
}

void KyraEngine_HoF::fadeMessagePalette() {
	if (!_fadeMessagePalette)
		return;

	bool updatePalette = false;
	for (int i = 0; i < 3; ++i) {
		if (_messagePal[i] >= 4) {
			_messagePal[i] -= 4;
			updatePalette = true;
		} else if (_messagePal[i] != 0) {
			_messagePal[i] = 0;
			updatePalette = true;
		}
	}

	if (updatePalette) {
		_screen->getPalette(0).copy(_messagePal, 0, 1, 255);
		_screen->setScreenPalette(_screen->getPalette(0));
	} else {
		_fadeMessagePalette = false;
	}
}

#pragma mark -

void KyraEngine_HoF::loadMouseShapes() {
	_screen->loadBitmap("_MOUSE.CSH", 3, 3, 0);

	for (int i = 0; i <= 8; ++i)
		addShapeToPool(_screen->getCPagePtr(3), i, i);
}

void KyraEngine_HoF::loadItemShapes() {
	_screen->loadBitmap("_ITEMS.CSH", 3, 3, 0);

	for (int i = 64; i <= 239; ++i)
		addShapeToPool(_screen->getCPagePtr(3), i, i-64);

	_res->loadFileToBuf("_ITEMHT.DAT", _itemHtDat, sizeof(_itemHtDat));
	assert(_res->getFileSize("_ITEMHT.DAT") == sizeof(_itemHtDat));

	_screen->_curPage = 0;
}

void KyraEngine_HoF::loadCharacterShapes(int shapes) {
	char file[10];
	strcpy(file, "_ZX.SHP");

	_characterShapeFile = shapes;
	file[2] = '0' + shapes;

	uint8 *data = _res->fileData(file, 0);
	for (int i = 9; i <= 32; ++i)
		addShapeToPool(data, i, i-9);
	delete[] data;

	_characterShapeFile = shapes;
}

void KyraEngine_HoF::loadInventoryShapes() {
	int curPageBackUp = _screen->_curPage;
	_screen->_curPage = 2;

	_screen->loadBitmap("_PLAYALL.CPS", 3, 3, 0);

	for (int i = 0; i < 10; ++i)
		addShapeToPool(_screen->encodeShape(_inventoryX[i], _inventoryY[i], 16, 16, 0), 240+i);

	_screen->_curPage = curPageBackUp;
}

void KyraEngine_HoF::runStartScript(int script, int unk1) {
	char filename[14];
	strcpy(filename, "_START0X.EMC");
	filename[7] = script + '0';

	EMCData scriptData;
	EMCState scriptState;
	memset(&scriptData, 0, sizeof(EMCData));
	memset(&scriptState, 0, sizeof(EMCState));

	_emc->load(filename, &scriptData, &_opcodes);
	_emc->init(&scriptState, &scriptData);
	scriptState.regs[6] = unk1;
	_emc->start(&scriptState, 0);
	while (_emc->isValid(&scriptState))
		_emc->run(&scriptState);
	_emc->unload(&scriptData);
}

void KyraEngine_HoF::loadNPCScript() {
	_emc->unload(&_npcScriptData);

	char filename[] = "_NPC.EMC";

	if (_flags.platform != Common::kPlatformPC || _flags.isTalkie) {
		switch (_lang) {
		case 0:
			filename[5] = 'E';
			break;

		case 1:
			filename[5] = 'F';
			break;

		case 2:
			filename[5] = 'G';
			break;

		case 3:
			filename[5] = 'J';
			break;

		default:
			break;
		};
	}

	_emc->load(filename, &_npcScriptData, &_opcodes);
}

#pragma mark -

void KyraEngine_HoF::resetScaleTable() {
	Common::fill(_scaleTable, ARRAYEND(_scaleTable), 0x100);
}

void KyraEngine_HoF::setScaleTableItem(int item, int data) {
	if (item >= 1 && item <= 15)
		_scaleTable[item-1] = (data << 8) / 100;
}

int KyraEngine_HoF::getScale(int x, int y) {
	return _scaleTable[_screen->getLayer(x, y) - 1];
}

void KyraEngine_HoF::setDrawLayerTableEntry(int entry, int data) {
	if (entry >= 1 && entry <= 15)
		_drawLayerTable[entry-1] = data;
}

int KyraEngine_HoF::getDrawLayer(int x, int y) {
	int layer = _screen->getLayer(x, y);
	layer = _drawLayerTable[layer-1];
	if (layer < 0)
		layer = 0;
	else if (layer >= 7)
		layer = 6;
	return layer;
}

void KyraEngine_HoF::backUpPage0() {
	if (_screenBuffer) {
		memcpy(_screenBuffer, _screen->getCPagePtr(0), 64000);
	}
}

void KyraEngine_HoF::restorePage0() {
	restorePage3();
	if (_screenBuffer)
		_screen->copyBlockToPage(0, 0, 0, 320, 200, _screenBuffer);
}

void KyraEngine_HoF::updateCharPal(int unk1) {
	if (!_useCharPal)
		return;

	int layer = _screen->getLayer(_mainCharacter.x1, _mainCharacter.y1);
	int palEntry = _charPalTable[layer];

	if (palEntry != _charPalEntry && unk1) {
		const uint8 *src = &_scenePal[(palEntry << 4) * 3];
		uint8 *ptr = _screen->getPalette(0).getData() + 336;
		for (int i = 0; i < 48; ++i) {
			*ptr -= (*ptr - *src) >> 1;
			++ptr;
			++src;
		}
		_screen->setScreenPalette(_screen->getPalette(0));
		_setCharPalFinal = true;
		_charPalEntry = palEntry;
	} else if (_setCharPalFinal || !unk1) {
		_screen->getPalette(0).copy(_scenePal, palEntry << 4, 16, 112);
		_screen->setScreenPalette(_screen->getPalette(0));
		_setCharPalFinal = false;
	}
}

void KyraEngine_HoF::setCharPalEntry(int entry, int value) {
	if (entry > 15 || entry < 1)
		entry = 1;
	if (value > 8 || value < 0)
		value = 0;
	_charPalTable[entry] = value;
	_useCharPal = 1;
	_charPalEntry = 0;
}

int KyraEngine_HoF::inputSceneChange(int x, int y, int unk1, int unk2) {
	bool refreshNPC = false;
	uint16 curScene = _mainCharacter.sceneId;
	_pathfinderFlag = 15;

	if (!_unkHandleSceneChangeFlag) {
		if (_savedMouseState == -3) {
			if (_sceneList[curScene].exit4 != 0xFFFF) {
				x = 4;
				y = _sceneEnterY4;
				_pathfinderFlag = 7;
			}
		} else if (_savedMouseState == -5) {
			if (_sceneList[curScene].exit2 != 0xFFFF) {
				x = 316;
				y = _sceneEnterY2;
				_pathfinderFlag = 7;
			}
		} else if (_savedMouseState == -6) {
			if (_sceneList[curScene].exit1 != 0xFFFF) {
				x = _sceneEnterX1;
				y = _sceneEnterY1 - 2;
				_pathfinderFlag = 14;
			}
		} else if (_savedMouseState == -4) {
			if (_sceneList[curScene].exit3 != 0xFFFF) {
				x = _sceneEnterX3;
				y = 147;
				_pathfinderFlag = 11;
			}
		}
	}

	int strId = 0;
	int vocH = _flags.isTalkie ? 131 : -1;

	if (_pathfinderFlag) {
		if (findItem(curScene, 13) >= 0 && _savedMouseState <= -3) {
			strId = 252;
		} else if (_itemInHand == 72) {
			strId = 257;
		} else if (findItem(curScene, 72) >= 0 && _savedMouseState <= -3) {
			strId = 256;
		} else if (getInventoryItemSlot(72) != -1 && _savedMouseState <= -3) {
			strId = 257;
		}
	}

	if (strId) {
		updateCharFacing();
		objectChat(getTableString(strId, _cCodeBuffer, 1), 0, vocH, strId);
		_pathfinderFlag = 0;
		return 0;
	}

	if (ABS(_mainCharacter.x1 - x) < 4 && ABS(_mainCharacter.y1 - y) < 2) {
		_pathfinderFlag = 0;
		return 0;
	}

	int curX = _mainCharacter.x1 & ~3;
	int curY = _mainCharacter.y1 & ~1;
	int dstX = x & ~3;
	int dstY = y & ~1;

	int wayLength = findWay(curX, curY, dstX, dstY, _movFacingTable, 600);
	_pathfinderFlag = 0;
	_timer->disable(5);

	if (wayLength != 0 && wayLength != 0x7D00)
		refreshNPC = (trySceneChange(_movFacingTable, unk1, unk2) != 0);

	int charLayer = _screen->getLayer(_mainCharacter.x1, _mainCharacter.y1);
	if (_layerFlagTable[charLayer] != 0 && !queryGameFlag(0x163)) {
		if (queryGameFlag(0x164)) {
			_screen->hideMouse();
			_timer->disable(5);
			runAnimationScript("_ZANBURN.EMC", 0, 1, 1, 0);
			_deathHandler = 7;
			snd_playWanderScoreViaMap(0x53, 1);
		} else {
			objectChat(getTableString(0xFD, _cCodeBuffer, 1), 0, 0x83, 0xFD);
			setGameFlag(0x164);
			_timer->enable(5);
			_timer->setCountdown(5, 120);
		}
	} else if (queryGameFlag(0x164)) {
		objectChat(getTableString(0xFE, _cCodeBuffer, 1), 0, 0x83, 0xFE);
		resetGameFlag(0x164);
		_timer->disable(5);
	}

	if (refreshNPC)
		enterNewSceneUnk2(0);

	_pathfinderFlag = 0;
	return refreshNPC;
}

int KyraEngine_HoF::getCharacterWalkspeed() const {
	return _timer->getDelay(0);
}

void KyraEngine_HoF::updateCharAnimFrame(int *table) {
	static const int unkFrame1 = 17;
	static const int unkFrame2 = 10;
	static const int unkFrame3 = 24;
	static const int unkFrame4 = 19;
	static const int unkFrame5 = 21;
	static const int unkFrame6 = 31;
	static const int unkFrame7 = 26;

	Character *character = &_mainCharacter;
	++character->animFrame;

	int facing = character->facing;

	if (table) {
		if (table[0] != table[-1] && table[-1] == table[1]) {
			facing = getOppositeFacingDirection(table[-1]);
			table[0] = table[-1];
		}
	}

	if (!facing) {
		++_characterFacingCountTable[0];
	} else if (facing == 4) {
		++_characterFacingCountTable[1];
	} else if (facing == 7 || facing == 1 || facing == 5 || facing == 3) {
		if (facing == 7 || facing == 1) {
			if (_characterFacingCountTable[0] > 2)
				facing = 0;
		} else {
			if (_characterFacingCountTable[1] > 2)
				facing = 4;
		}

		_characterFacingCountTable[0] = 0;
		_characterFacingCountTable[1] = 0;
	}

	if (facing == 0) {
		if (character->animFrame < unkFrame7)
			character->animFrame = unkFrame7;

		if (character->animFrame > unkFrame6)
			character->animFrame = unkFrame7;
	} else if (facing == 4) {
		if (character->animFrame < unkFrame4)
			character->animFrame = unkFrame4;

		if (character->animFrame > unkFrame3)
			character->animFrame = unkFrame4;
	} else {
		if (character->animFrame > unkFrame4)
			character->animFrame = unkFrame5;

		if (character->animFrame == unkFrame1)
			character->animFrame = unkFrame2;

		if (character->animFrame > unkFrame1)
			character->animFrame = unkFrame2 + 2;
	}

	updateCharacterAnim(0);
}

bool KyraEngine_HoF::checkCharCollision(int x, int y) {
	int scale1 = 0, scale2 = 0, scale3 = 0;
	int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
	scale1 = getScale(_mainCharacter.x1, _mainCharacter.y1);
	scale2 = (scale1 * 24) >> 8;
	scale3 = (scale1 * 48) >> 8;

	x1 = _mainCharacter.x1 - (scale2 >> 1);
	x2 = _mainCharacter.x1 + (scale2 >> 1);
	y1 = _mainCharacter.y1 - scale3;
	y2 = _mainCharacter.y1;

	if (x >= x1 && x <= x2 && y >= y1 && y <= y2)
		return true;

	return false;
}

int KyraEngine_HoF::initAnimationShapes(uint8 *filedata) {
	const int lastEntry = MIN(_animShapeLastEntry, 31);
	for (int i = 0; i < lastEntry; ++i) {
		addShapeToPool(filedata, i+33, i);
		ShapeDesc *desc = &_shapeDescTable[24+i];
		desc->xAdd = _animShapeXAdd;
		desc->yAdd = _animShapeYAdd;
		desc->width = _animShapeWidth;
		desc->height = _animShapeHeight;
	}
	return lastEntry;
}

void KyraEngine_HoF::uninitAnimationShapes(int count, uint8 *filedata) {
	for (int i = 0; i < count; ++i)
		remShapeFromPool(i+33);
	delete[] filedata;
	setNextIdleAnimTimer();
}

void KyraEngine_HoF::setNextIdleAnimTimer() {
	_nextIdleAnim = _system->getMillis() + _rnd.getRandomNumberRng(10, 15) * 60 * _tickLength;
}

void KyraEngine_HoF::showIdleAnim() {
	static const uint8 scriptMinTable[] = {
		0x00, 0x05, 0x07, 0x08, 0x00, 0x09, 0x0A, 0x0B, 0xFF, 0x00
	};

	static const uint8 scriptMaxTable[] = {
		0x04, 0x06, 0x07, 0x08, 0x04, 0x09, 0x0A, 0x0B, 0xFF, 0x00
	};

	if (queryGameFlag(0x159) && _flags.isTalkie)
		return;

	if (!_useSceneIdleAnim && _flags.isTalkie) {
		_useSceneIdleAnim = true;
		randomSceneChat();
	} else {
		_useSceneIdleAnim = false;
		if (_characterShapeFile > 8)
			return;

		int scriptMin = scriptMinTable[_characterShapeFile-1];
		int scriptMax = scriptMaxTable[_characterShapeFile-1];
		int script = 0;

		if (scriptMin < scriptMax) {
			do {
				script = _rnd.getRandomNumberRng(scriptMin, scriptMax);
			} while (script == _lastIdleScript);
		} else {
			script = scriptMin;
		}

		runIdleScript(script);
		_lastIdleScript = script;
	}
}

void KyraEngine_HoF::runIdleScript(int script) {
	if (script < 0 || script >= 12)
		script = 0;

	if (_mainCharacter.animFrame != 18) {
		setNextIdleAnimTimer();
	} else {
		// FIXME: move this to staticres.cpp?
		static const char *const idleScriptFiles[] = {
			"_IDLHAIR.EMC", "_IDLDUST.EMC", "_IDLLEAN.EMC", "_IDLDIRT.EMC", "_IDLTOSS.EMC", "_IDLNOSE.EMC",
			"_IDLBRSH.EMC", "_Z3IDLE.EMC", "_Z4IDLE.EMC", "_Z6IDLE.EMC", "_Z7IDLE.EMC", "_Z8IDLE.EMC"
		};

		runAnimationScript(idleScriptFiles[script], 1, 1, 1, 1);
	}
}

#pragma mark -

void KyraEngine_HoF::backUpGfxRect24x24(int x, int y) {
	_screen->copyRegionToBuffer(_screen->_curPage, x, y, 24, 24, _gfxBackUpRect);
}

void KyraEngine_HoF::restoreGfxRect24x24(int x, int y) {
	_screen->copyBlockToPage(_screen->_curPage, x, y, 24, 24, _gfxBackUpRect);
}

void KyraEngine_HoF::backUpGfxRect32x32(int x, int y) {
	_screen->copyRegionToBuffer(_screen->_curPage, x, y, 32, 32, _gfxBackUpRect);
}

void KyraEngine_HoF::restoreGfxRect32x32(int x, int y) {
	_screen->copyBlockToPage(_screen->_curPage, x, y, 32, 32, _gfxBackUpRect);
}

#pragma mark -

void KyraEngine_HoF::openTalkFile(int newFile) {
	char talkFilename[16];

	if (_oldTalkFile > 0) {
		sprintf(talkFilename, "CH%dVOC.TLK", _oldTalkFile);
		_res->unloadPakFile(talkFilename);
		_oldTalkFile = -1;
	}

	if (newFile == 0)
		strcpy(talkFilename, "ANYTALK.TLK");
	else
		sprintf(talkFilename, "CH%dVOC.TLK", newFile);

	_oldTalkFile = newFile;

	if (!_res->loadPakFile(talkFilename)) {
		if (speechEnabled()) {
			warning("Couldn't load voice file '%s', falling back to text only mode", talkFilename);
			_configVoice = 0;

			// Sync the config manager with the new settings
			writeSettings();
		}
	}
}

void KyraEngine_HoF::snd_playVoiceFile(int id) {
	char vocFile[9];
	assert(id >= 0 && id <= 9999999);
	sprintf(vocFile, "%07d", id);
	if (_sound->isVoicePresent(vocFile)) {
		snd_stopVoice();

		while (!_sound->voicePlay(vocFile, &_speechHandle)) {
			updateWithText();
			_system->delayMillis(10);
		}
	}
}

void KyraEngine_HoF::snd_loadSoundFile(int id) {
	if (id < 0 || !_trackMap)
		return;

	assert(id < _trackMapSize);
	int file = _trackMap[id*2];
	_curSfxFile = _curMusicTheme = file;
	_sound->loadSoundFile(file);
}

void KyraEngine_HoF::playVoice(int high, int low) {
	if (!_flags.isTalkie)
		return;
	int vocFile = high * 10000 + low * 10;
	if (speechEnabled())
		snd_playVoiceFile(vocFile);
}

void KyraEngine_HoF::snd_playSoundEffect(int track, int volume) {
	if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98) {
		if (track == 10)
			track = _lastSfxTrack;

		if (track == 10 || track == -1)
			return;

		_lastSfxTrack = track;
	}

	int16 vocIndex = (int16)READ_LE_UINT16(&_ingameSoundIndex[track * 2]);
	if (vocIndex != -1) {
		_sound->voicePlay(_ingameSoundList[vocIndex], 0, 255, 255, true);
	} else if (_flags.platform == Common::kPlatformPC) {
		if (_sound->getSfxType() == Sound::kMidiMT32)
			track = track < _mt32SfxMapSize ? _mt32SfxMap[track] - 1 : -1;
		else if (_sound->getSfxType() == Sound::kMidiGM)
			track = track < _gmSfxMapSize ? _gmSfxMap[track] - 1 : -1;
		else if (_sound->getSfxType() == Sound::kPCSpkr)
			track = track < _pcSpkSfxMapSize ? _pcSpkSfxMap[track] - 1 : -1;

		if (track != -1)
			KyraEngine_v1::snd_playSoundEffect(track);

		// TODO ?? Maybe there is a way to let users select whether they want
		// voc, midi or adl sfx (even though it makes no sense to choose anything but voc).
		// The PC-98 version has support for non-pcm sound effects, but only for tracks
		// which also have voc files. The syntax would be:
		// KyraEngine_v1::snd_playSoundEffect(vocIndex);
	}
}

#pragma mark -

void KyraEngine_HoF::loadInvWsa(const char *filename, int run_, int delayTime, int page, int sfx, int sFrame, int flags) {
	int wsaFlags = 1;
	if (flags)
		wsaFlags |= 2;

	if (!_invWsa.wsa)
		_invWsa.wsa = new WSAMovie_v2(this);

	if (!_invWsa.wsa->open(filename, wsaFlags, 0))
		error("Couldn't open inventory WSA file '%s'", filename);

	_invWsa.curFrame = 0;
	_invWsa.lastFrame = _invWsa.wsa->frames();

	_invWsa.x = _invWsa.wsa->xAdd();
	_invWsa.y = _invWsa.wsa->yAdd();
	_invWsa.w = _invWsa.wsa->width();
	_invWsa.h = _invWsa.wsa->height();
	_invWsa.x2 = _invWsa.x + _invWsa.w - 1;
	_invWsa.y2 = _invWsa.y + _invWsa.h - 1;

	_invWsa.delay = delayTime;
	_invWsa.page = page;
	_invWsa.sfx = sfx;

	_invWsa.specialFrame = sFrame;

	if (_invWsa.page)
		_screen->copyRegion(_invWsa.x, _invWsa.y, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, 0, _invWsa.page, Screen::CR_NO_P_CHECK);

	_invWsa.running = true;
	_invWsa.timer = _system->getMillis();

	if (run_) {
		while (_invWsa.running && !skipFlag() && !shouldQuit()) {
			update();
			_system->delayMillis(10);
		}

		if (skipFlag()) {
			resetSkipFlag();
			displayInvWsaLastFrame();
		}
	}
}

void KyraEngine_HoF::closeInvWsa() {
	_invWsa.wsa->close();
	delete _invWsa.wsa;
	_invWsa.wsa = 0;
	_invWsa.running = false;
}

void KyraEngine_HoF::updateInvWsa() {
	if (!_invWsa.running || !_invWsa.wsa)
		return;

	if (_invWsa.timer > _system->getMillis())
		return;

	_invWsa.wsa->displayFrame(_invWsa.curFrame, _invWsa.page, 0, 0, 0, 0, 0);

	if (_invWsa.page)
		_screen->copyRegion(_invWsa.x, _invWsa.y, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, _invWsa.page, 0, Screen::CR_NO_P_CHECK);

	_invWsa.timer = _system->getMillis() + _invWsa.delay * _tickLength;

	++_invWsa.curFrame;
	if (_invWsa.curFrame >= _invWsa.lastFrame)
		displayInvWsaLastFrame();

	if (_invWsa.curFrame == _invWsa.specialFrame)
		snd_playSoundEffect(_invWsa.sfx);

	if (_invWsa.sfx == -2) {
		switch (_invWsa.curFrame) {
		case 9: case 27: case 40:
			snd_playSoundEffect(0x39);
			break;

		case 18: case 34: case 44:
			snd_playSoundEffect(0x33);
			break;

		case 48:
			snd_playSoundEffect(0x38);
			break;

		default:
			break;
		}
	}
}

void KyraEngine_HoF::displayInvWsaLastFrame() {
	if (!_invWsa.wsa)
		return;

	_invWsa.wsa->displayFrame(_invWsa.lastFrame-1, _invWsa.page, 0, 0, 0, 0, 0);

	if (_invWsa.page)
		_screen->copyRegion(_invWsa.x, _invWsa.y, _invWsa.x, _invWsa.y, _invWsa.w, _invWsa.h, _invWsa.page, 0, Screen::CR_NO_P_CHECK);

	closeInvWsa();

	int32 countdown = _rnd.getRandomNumberRng(45, 80);
	_timer->setCountdown(2, countdown * 60);
}

#pragma mark -

void KyraEngine_HoF::setCauldronState(uint8 state, bool paletteFade) {
	_screen->copyPalette(2, 0);
	Common::SeekableReadStream *file = _res->createReadStream("_POTIONS.PAL");
	if (!file)
		error("Couldn't load cauldron palette");
	file->seek(state*18, SEEK_SET);
	_screen->getPalette(2).loadVGAPalette(*file, 241, 6);
	delete file;
	file = 0;

	if (paletteFade) {
		snd_playSoundEffect((state == 0) ? 0x6B : 0x66);
		_screen->fadePalette(_screen->getPalette(2), 0x4B, &_updateFunctor);
	} else {
		_screen->setScreenPalette(_screen->getPalette(2));
		_screen->updateScreen();
	}

	_screen->getPalette(0).copy(_screen->getPalette(2), 241, 6);
	_cauldronState = state;
	_cauldronUseCount = 0;
	if (state == 5)
		setDlgIndex(5);
}

void KyraEngine_HoF::clearCauldronTable() {
	Common::fill(_cauldronTable, ARRAYEND(_cauldronTable), -1);
}

void KyraEngine_HoF::addFrontCauldronTable(int item) {
	for (int i = 23; i >= 0; --i)
		_cauldronTable[i+1] = _cauldronTable[i];
	_cauldronTable[0] = item;
}

void KyraEngine_HoF::cauldronItemAnim(int item) {
	const int x = 282;
	const int y = 135;
	const int mouseDstX = (x + 7) & (~1);
	const int mouseDstY = (y + 15) & (~1);
	int mouseX = _mouseX & (~1);
	int mouseY = _mouseY & (~1);

	while (mouseY != mouseDstY) {
		if (mouseY < mouseDstY)
			mouseY += 2;
		else if (mouseY > mouseDstY)
			mouseY -= 2;
		uint32 waitEnd = _system->getMillis() + _tickLength;
		setMousePos(mouseX, mouseY);
		_system->updateScreen();
		delayUntil(waitEnd);
	}

	while (mouseX != mouseDstX) {
		if (mouseX < mouseDstX)
			mouseX += 2;
		else if (mouseX > mouseDstX)
			mouseX -= 2;
		uint32 waitEnd = _system->getMillis() + _tickLength;
		setMousePos(mouseX, mouseY);
		_system->updateScreen();
		delayUntil(waitEnd);
	}

	if (itemIsFlask(item)) {
		setHandItem(19);
		delayUntil(_system->getMillis()+_tickLength*30);
		setHandItem(18);
	} else {
		_screen->hideMouse();
		backUpGfxRect32x32(x, y);
		uint8 *shape = getShapePtr(item+64);

		int curY = y;
		for (int i = 0; i < 12; i += 2, curY += 2) {
			restoreGfxRect32x32(x, y);
			uint32 waitEnd = _system->getMillis() + _tickLength;
			_screen->drawShape(0, shape, x, curY, 0, 0);
			_screen->updateScreen();
			delayUntil(waitEnd);
		}

		snd_playSoundEffect(0x17);

		for (int i = 16; i > 0; i -= 2, curY += 2) {
			_screen->setNewShapeHeight(shape, i);
			restoreGfxRect32x32(x, y);
			uint32 waitEnd = _system->getMillis() + _tickLength;
			_screen->drawShape(0, shape, x, curY, 0, 0);
			_screen->updateScreen();
			delayUntil(waitEnd);
		}

		restoreGfxRect32x32(x, y);
		_screen->resetShapeHeight(shape);
		removeHandItem();
		_screen->showMouse();
	}
}

bool KyraEngine_HoF::updateCauldron() {
	for (int i = 0; i < 23; ++i) {
		const int16 *curStateTable = _cauldronStateTables[i];
		if (*curStateTable == -2)
			continue;

		int cauldronState = i;
		int16 cauldronTable[25];
		memcpy(cauldronTable, _cauldronTable, sizeof(cauldronTable));

		while (*curStateTable != -2) {
			int stateValue = *curStateTable++;
			int j = 0;
			for (; j < 25; ++j) {
				int val = cauldronTable[j];

				switch (val) {
				case 68:
					val = 70;
					break;

				case 133:
				case 167:
					val = 119;
					break;

				case 130:
				case 143:
				case 100:
					val = 12;
					break;

				case 132:
				case 65:
				case 69:
				case 74:
					val = 137;
					break;

				case 157:
					val = 134;
					break;

				default:
					break;
				}

				if (val == stateValue) {
					cauldronTable[j] = -1;
					j = 26;
				}
			}

			if (j == 25)
				cauldronState = -1;
		}

		if (cauldronState >= 0) {
			showMessage(0, 0xCF);
			setCauldronState(cauldronState, true);
			if (cauldronState == 7)
				objectChat(getTableString(0xF2, _cCodeBuffer, 1), 0, 0x83, 0xF2);
			clearCauldronTable();
			return true;
		}
	}

	return false;
}

void KyraEngine_HoF::cauldronRndPaletteFade() {
	showMessage(0, 0xCF);
	int index = _rnd.getRandomNumberRng(0x0F, 0x16);
	Common::SeekableReadStream *file = _res->createReadStream("_POTIONS.PAL");
	if (!file)
		error("Couldn't load cauldron palette");
	file->seek(index*18, SEEK_SET);
	_screen->getPalette(0).loadVGAPalette(*file, 241, 6);
	snd_playSoundEffect(0x6A);
	_screen->fadePalette(_screen->getPalette(0), 0x1E, &_updateFunctor);
	file->seek(0, SEEK_SET);
	_screen->getPalette(0).loadVGAPalette(*file, 241, 6);
	delete file;
	_screen->fadePalette(_screen->getPalette(0), 0x1E, &_updateFunctor);
}

void KyraEngine_HoF::resetCauldronStateTable(int idx) {
	for (int i = 0; i < 7; ++i)
		_cauldronStateTables[idx][i] = -2;
}

bool KyraEngine_HoF::addToCauldronStateTable(int data, int idx) {
	for (int i = 0; i < 7; ++i) {
		if (_cauldronStateTables[idx][i] == -2) {
			_cauldronStateTables[idx][i] = data;
			return true;
		}
	}
	return false;
}

void KyraEngine_HoF::listItemsInCauldron() {
	int itemsInCauldron = 0;
	for (int i = 0; i < 25; ++i) {
		if (_cauldronTable[i] != -1)
			++itemsInCauldron;
		else
			break;
	}

	if (!itemsInCauldron) {
		if (!_cauldronState)
			objectChat(getTableString(0xF4, _cCodeBuffer, 1), 0, 0x83, 0xF4);
		else
			objectChat(getTableString(0xF3, _cCodeBuffer, 1), 0, 0x83, 0xF3);
	} else {
		objectChat(getTableString(0xF7, _cCodeBuffer, 1), 0, 0x83, 0xF7);

		char buffer[80];
		for (int i = 0; i < itemsInCauldron-1; ++i) {
			char *str = buffer;
			strcpy(str, getTableString(_cauldronTable[i]+54, _cCodeBuffer, 1));
			if (_lang == 1) {
				if (*str == 37)
					str += 2;
			}
			strcpy((char *)_unkBuf500Bytes, "...");
			strcat((char *)_unkBuf500Bytes, str);
			strcat((char *)_unkBuf500Bytes, "...");
			objectChat((const char *)_unkBuf500Bytes, 0, 0x83, _cauldronTable[i]+54);
		}

		char *str = buffer;
		strcpy(str, getTableString(_cauldronTable[itemsInCauldron-1]+54, _cCodeBuffer, 1));
		if (_lang == 1) {
			if (*str == 37)
				str += 2;
		}
		strcpy((char *)_unkBuf500Bytes, "...");
		strcat((char *)_unkBuf500Bytes, str);
		strcat((char *)_unkBuf500Bytes, ".");
		objectChat((const char *)_unkBuf500Bytes, 0, 0x83, _cauldronTable[itemsInCauldron-1]+54);
	}
}

#pragma mark -

void KyraEngine_HoF::dinoRide() {
	_mainCharX = _mainCharY = -1;

	setGameFlag(0x15A);
	enterNewScene(41, -1, 0, 0, 0);
	resetGameFlag(0x15A);

	setGameFlag(0x15B);
	enterNewScene(39, -1, 0, 0, 0);
	resetGameFlag(0x15B);

	setGameFlag(0x16F);

	setGameFlag(0x15C);
	enterNewScene(42, -1, 0, 0, 0);
	resetGameFlag(0x15C);

	setGameFlag(0x15D);
	enterNewScene(39, -1, 0, 0, 0);
	resetGameFlag(0x15D);

	setGameFlag(0x15E);
	enterNewScene(40, -1, 0, 0, 0);
	resetGameFlag(0x15E);

	_mainCharX = 262;
	_mainCharY = 28;
	_mainCharacter.facing = 5;
	_mainCharacter.animFrame = _characterFrameTable[5];
	enterNewScene(39, 4, 0, 0, 0);
	setHandItem(0x61);
	_screen->showMouse();
	resetGameFlag(0x159);
}

#pragma mark -

void KyraEngine_HoF::playTim(const char *filename) {
	TIM *tim = _tim->load(filename, &_timOpcodes);
	if (!tim)
		return;

	_tim->resetFinishedFlag();
	while (!shouldQuit() && !_tim->finished()) {
		_tim->exec(tim, 0);
		if (_chatText)
			updateWithText();
		else
			update();
		delay(10);
	}

	_tim->unload(tim);
}

#pragma mark -

void KyraEngine_HoF::registerDefaultSettings() {
	KyraEngine_v1::registerDefaultSettings();

	// Most settings already have sensible defaults. This one, however, is
	// specific to the Kyra engine.
	ConfMan.registerDefault("walkspeed", 5);
}

void KyraEngine_HoF::writeSettings() {
	ConfMan.setInt("talkspeed", ((_configTextspeed-2) * 255) / 95);

	switch (_lang) {
	case 1:
		_flags.lang = Common::FR_FRA;
		break;

	case 2:
		_flags.lang = Common::DE_DEU;
		break;

	case 3:
		_flags.lang = Common::JA_JPN;
		break;

	case 0:
	default:
		_flags.lang = Common::EN_ANY;
	}

	if (_flags.lang == _flags.replacedLang && _flags.fanLang != Common::UNK_LANG)
		_flags.lang = _flags.fanLang;

	ConfMan.set("language", Common::getLanguageCode(_flags.lang));

	KyraEngine_v1::writeSettings();
}

void KyraEngine_HoF::readSettings() {
	KyraEngine_v2::readSettings();

	int talkspeed = ConfMan.getInt("talkspeed");
	_configTextspeed = (talkspeed*95)/255 + 2;
}

} // End of namespace Kyra
