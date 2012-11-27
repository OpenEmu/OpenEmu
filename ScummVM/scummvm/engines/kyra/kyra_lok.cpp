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

#include "kyra/kyra_lok.h"
#include "kyra/resource.h"
#include "kyra/seqplayer.h"
#include "kyra/sprites.h"
#include "kyra/animator_lok.h"
#include "kyra/debugger.h"
#include "kyra/timer.h"
#include "kyra/sound.h"

#include "common/system.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"

#include "gui/message.h"

namespace Kyra {

KyraEngine_LoK::KyraEngine_LoK(OSystem *system, const GameFlags &flags)
	: KyraEngine_v1(system, flags) {

	_seq_Forest = _seq_KallakWriting = _seq_KyrandiaLogo = _seq_KallakMalcolm = 0;
	_seq_MalcolmTree = _seq_WestwoodLogo = _seq_Demo1 = _seq_Demo2 = _seq_Demo3 = 0;
	_seq_Demo4 = 0;

	_seq_WSATable = _seq_CPSTable = _seq_COLTable = _seq_textsTable = 0;
	_seq_WSATable_Size = _seq_CPSTable_Size = _seq_COLTable_Size = _seq_textsTable_Size = 0;

	_roomFilenameTable = _characterImageTable = 0;
	_roomFilenameTableSize = _characterImageTableSize = 0;
	_itemList = _takenList = _placedList = _droppedList = _noDropList = 0;
	_itemList_Size = _takenList_Size = _placedList_Size = _droppedList_Size = _noDropList_Size = 0;
	_putDownFirst = _waitForAmulet = _blackJewel = _poisonGone = _healingTip = 0;
	_putDownFirst_Size = _waitForAmulet_Size = _blackJewel_Size = _poisonGone_Size = _healingTip_Size = 0;
	_thePoison = _fluteString = _wispJewelStrings = _magicJewelString = _flaskFull = _fullFlask = 0;
	_thePoison_Size = _fluteString_Size = _wispJewelStrings_Size = 0;
	_magicJewelString_Size = _flaskFull_Size = _fullFlask_Size = 0;

	_defaultShapeTable = 0;
	_healingShapeTable = _healingShape2Table = 0;
	_defaultShapeTableSize = _healingShapeTableSize = _healingShape2TableSize = 0;
	_posionDeathShapeTable = _fluteAnimShapeTable = 0;
	_posionDeathShapeTableSize = _fluteAnimShapeTableSize = 0;
	_winterScrollTable = _winterScroll1Table = _winterScroll2Table = 0;
	_winterScrollTableSize = _winterScroll1TableSize = _winterScroll2TableSize = 0;
	_drinkAnimationTable = _brandonToWispTable = _magicAnimationTable = _brandonStoneTable = 0;
	_drinkAnimationTableSize = _brandonToWispTableSize = _magicAnimationTableSize = _brandonStoneTableSize = 0;
	_specialPalettes = 0;
	_sprites = 0;
	_animator = 0;
	_seq = 0;
	_characterList = 0;
	_roomTable = 0;
	_movFacingTable = 0;
	_buttonData = 0;
	_buttonDataListPtr = 0;
	memset(_shapes, 0, sizeof(_shapes));
	memset(_movieObjects, 0, sizeof(_movieObjects));
	_finalA = _finalB = _finalC = 0;
	_endSequenceBackUpRect = 0;
	memset(_panPagesTable, 0, sizeof(_panPagesTable));
	memset(_sceneAnimTable, 0, sizeof(_sceneAnimTable));
	_currHeadShape = 0;
	_currentHeadFrameTableIndex = 0;
	_speechPlayTime = 0;
	_seqPlayerFlag = false;

	memset(&_characterFacingZeroCount, 0, sizeof(_characterFacingZeroCount));
	memset(&_characterFacingFourCount, 0, sizeof(_characterFacingFourCount));

	memset(&_itemBkgBackUp, 0, sizeof(_itemBkgBackUp));

	_beadStateTimer1 = _beadStateTimer2 = 0;
	memset(&_beadState1, 0, sizeof(_beadState1));
	_beadState1.x = -1;
	memset(&_beadState2, 0, sizeof(_beadState2));

	_malcolmFrame = 0;
	_malcolmTimer1 = _malcolmTimer2 = 0;
}

KyraEngine_LoK::~KyraEngine_LoK() {
	for (int i = 0; i < ARRAYSIZE(_movieObjects); ++i) {
		if (_movieObjects[i])
			_movieObjects[i]->close();
		delete _movieObjects[i];
		_movieObjects[i] = 0;
	}

	closeFinalWsa();
	if (_emc) {
		_emc->unload(&_npcScriptData);
		_emc->unload(&_scriptClickData);
	}

	DebugMan.clearAllDebugChannels();

	delete _screen;
	delete _sprites;
	delete _animator;
	delete _seq;

	delete[] _characterList;

	delete[] _roomTable;

	delete[] _movFacingTable;

	delete[] _defaultShapeTable;

	delete[] _specialPalettes;

	delete[] _gui->_scrollUpButton.data0ShapePtr;
	delete[] _gui->_scrollUpButton.data1ShapePtr;
	delete[] _gui->_scrollUpButton.data2ShapePtr;
	delete[] _gui->_scrollDownButton.data0ShapePtr;
	delete[] _gui->_scrollDownButton.data1ShapePtr;
	delete[] _gui->_scrollDownButton.data2ShapePtr;

	delete[] _buttonData;
	delete[] _buttonDataListPtr;

	delete _gui;

	delete[] _itemBkgBackUp[0];
	delete[] _itemBkgBackUp[1];

	for (int i = 0; i < ARRAYSIZE(_shapes); ++i) {
		if (_shapes[i] != 0) {
			delete[] _shapes[i];
			for (int i2 = 0; i2 < ARRAYSIZE(_shapes); i2++) {
				if (_shapes[i2] == _shapes[i] && i2 != i) {
					_shapes[i2] = 0;
				}
			}
			_shapes[i] = 0;
		}
	}

	for (int i = 0; i < ARRAYSIZE(_sceneAnimTable); ++i)
		delete[] _sceneAnimTable[i];
}

Common::Error KyraEngine_LoK::init() {
	if (Common::parseRenderMode(ConfMan.get("render_mode")) == Common::kRenderPC9801)
		_screen = new Screen_LoK_16(this, _system);
	else
		_screen = new Screen_LoK(this, _system);
	assert(_screen);
	_screen->setResolution();

	_debugger = new Debugger_LoK(this);
	assert(_debugger);

	KyraEngine_v1::init();

	_sprites = new Sprites(this, _system);
	assert(_sprites);
	_seq = new SeqPlayer(this, _system);
	assert(_seq);
	_animator = new Animator_LoK(this, _system);
	assert(_animator);
	_animator->init(5, 11, 12);
	assert(*_animator);
	_text = new TextDisplayer(this, screen());
	assert(_text);
	_gui = new GUI_LoK(this, _screen);
	assert(_gui);

	initStaticResource();

	_sound->selectAudioResourceSet(kMusicIntro);

	if (_flags.platform == Common::kPlatformAmiga) {
		_trackMap = _amigaTrackMap;
		_trackMapSize = _amigaTrackMapSize;
	} else {
		_trackMap = _dosTrackMap;
		_trackMapSize = _dosTrackMapSize;
	}

	if (!_sound->init())
		error("Couldn't init sound");

	_sound->loadSoundFile(0);

	setupButtonData();

	_paletteChanged = 1;
	_currentCharacter = 0;
	_characterList = new Character[11];
	assert(_characterList);
	memset(_characterList, 0, sizeof(Character) * 11);

	for (int i = 0; i < 11; ++i)
		memset(_characterList[i].inventoryItems, 0xFF, sizeof(_characterList[i].inventoryItems));

	_characterList[0].sceneId = 5;
	_characterList[0].height = 48;
	_characterList[0].facing = 3;
	_characterList[0].currentAnimFrame = 7;

	memset(&_npcScriptData, 0, sizeof(EMCData));
	memset(&_scriptClickData, 0, sizeof(EMCData));

	memset(&_npcScript, 0, sizeof(EMCState));
	memset(&_scriptMain, 0, sizeof(EMCState));
	memset(&_scriptClick, 0, sizeof(EMCState));

	memset(_shapes, 0, sizeof(_shapes));

	for (int i = 0; i < ARRAYSIZE(_movieObjects); ++i)
		_movieObjects[i] = createWSAMovie();

	memset(_flagsTable, 0, sizeof(_flagsTable));

	_talkingCharNum = -1;
	_charSayUnk3 = -1;
	_disabledTalkAnimObject = _enabledTalkAnimObject = 0;
	memset(_currSentenceColor, 0, 3);
	_startSentencePalIndex = -1;
	_fadeText = false;

	_cauldronState = 0;
	_crystalState[0] = _crystalState[1] = -1;

	_brandonStatusBit = 0;
	_brandonStatusBit0x02Flag = _brandonStatusBit0x20Flag = 10;
	_brandonPosX = _brandonPosY = -1;
	_poisonDeathCounter = 0;

	memset(_itemHtDat, 0, sizeof(_itemHtDat));
	memset(_exitList, 0xFF, sizeof(_exitList));
	_exitListPtr = 0;
	_pathfinderFlag = _pathfinderFlag2 = 0;
	_lastFindWayRet = 0;
	_sceneChangeState = _loopFlag2 = 0;

	_movFacingTable = new int[150];
	assert(_movFacingTable);
	_movFacingTable[0] = 8;

	_marbleVaseItem = -1;
	memset(_foyerItemTable, -1, sizeof(_foyerItemTable));
	_itemInHand = kItemNone;

	_currentRoom = 0xFFFF;
	_scenePhasingFlag = 0;
	_lastProcessedItem = 0;
	_lastProcessedItemHeight = 16;

	_unkScreenVar1 = 1;
	_unkScreenVar2 = 0;
	_unkScreenVar3 = 0;
	_unkAmuletVar = 0;

	_endSequenceNeedLoading = 1;
	_malcolmFlag = 0;
	_beadStateVar = 0;
	_endSequenceSkipFlag = 0;
	_unkEndSeqVar2 = 0;
	_endSequenceBackUpRect = 0;
	_unkEndSeqVar4 = 0;
	_unkEndSeqVar5 = 0;
	_lastDisplayedPanPage = 0;
	memset(_panPagesTable, 0, sizeof(_panPagesTable));
	_finalA = _finalB = _finalC = 0;
	memset(&_kyragemFadingState, 0, sizeof(_kyragemFadingState));
	_kyragemFadingState.gOffset = 0x13;
	_kyragemFadingState.bOffset = 0x13;

	_menuDirectlyToLoad = false;

	_lastMusicCommand = 0;

	return Common::kNoError;
}

Common::Error KyraEngine_LoK::go() {
	if (_res->getFileSize("6.FNT"))
		_screen->loadFont(Screen::FID_6_FNT, "6.FNT");
	_screen->loadFont(Screen::FID_8_FNT, "8FAT.FNT");

	_screen->setFont(_flags.lang == Common::JA_JPN ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);

	_screen->setScreenDim(0);

	_abortIntroFlag = false;

	if (_flags.isDemo && !_flags.isTalkie) {
		_seqPlayerFlag = true;
		seq_demo();
		_seqPlayerFlag = false;
	} else {
		setGameFlag(0xF3);
		setGameFlag(0xFD);
		if (_gameToLoad == -1) {
			setGameFlag(0xEF);
			_seqPlayerFlag = true;
			seq_intro();
			_seqPlayerFlag = false;

			if (_flags.isDemo) {
				_screen->fadeToBlack();
				return Common::kNoError;
			}

			if (shouldQuit())
				return Common::kNoError;

			if (_skipIntroFlag && _abortIntroFlag && saveFileLoadable(0))
				resetGameFlag(0xEF);
		}
		_eventList.clear();
		startup();
		resetGameFlag(0xEF);
		mainLoop();
	}
	return Common::kNoError;
}


void KyraEngine_LoK::startup() {
	static const uint8 colorMap[] = { 0, 0, 0, 0, 12, 12, 12, 0, 0, 0, 0, 0 };
	_screen->setTextColorMap(colorMap);

	_sound->selectAudioResourceSet(kMusicIngame);
	if (_flags.platform == Common::kPlatformPC98)
		_sound->loadSoundFile("SE.DAT");
	else
		_sound->loadSoundFile(0);

//	_screen->setFont(Screen::FID_6_FNT);
	_screen->setAnimBlockPtr(3750);
	memset(_sceneAnimTable, 0, sizeof(_sceneAnimTable));
	loadMouseShapes();
	_currentCharacter = &_characterList[0];
	for (int i = 1; i < 5; ++i)
		_animator->setCharacterDefaultFrame(i);
	for (int i = 5; i <= 10; ++i)
		setCharactersPositions(i);
	_animator->setCharactersHeight();
	resetBrandonPoisonFlags();
	_screen->_curPage = 0;
	// XXX
	for (int i = 0; i < 12; ++i) {
		int size = _screen->getRectSize(3, 24);
		_shapes[361 + i] = new byte[size];
	}

	_itemBkgBackUp[0] = new uint8[_screen->getRectSize(3, 24)];
	memset(_itemBkgBackUp[0], 0, _screen->getRectSize(3, 24));
	_itemBkgBackUp[1] = new uint8[_screen->getRectSize(4, 32)];
	memset(_itemBkgBackUp[1], 0, _screen->getRectSize(4, 32));

	for (int i = 0; i < _roomTableSize; ++i) {
		for (int item = 0; item < 12; ++item) {
			_roomTable[i].itemsTable[item] = kItemNone;
			_roomTable[i].itemsXPos[item] = 0xFFFF;
			_roomTable[i].itemsYPos[item] = 0xFF;
			_roomTable[i].needInit[item] = 0;
		}
	}

	loadCharacterShapes();
	loadSpecialEffectShapes();
	loadItems();
	loadButtonShapes();
	initMainButtonList();
	loadMainScreen();
	_screen->loadPalette("PALETTE.COL", _screen->getPalette(0));

	if (_flags.platform == Common::kPlatformAmiga)
		_screen->loadPaletteTable("PALETTE.DAT", 6);

	// XXX
	_animator->initAnimStateList();
	setCharactersInDefaultScene();

	if (!_emc->load("_STARTUP.EMC", &_npcScriptData, &_opcodes))
		error("Could not load \"_STARTUP.EMC\" script");
	_emc->init(&_scriptMain, &_npcScriptData);

	if (!_emc->start(&_scriptMain, 0))
		error("Could not start script function 0 of script \"_STARTUP.EMC\"");

	while (_emc->isValid(&_scriptMain))
		_emc->run(&_scriptMain);

	_emc->unload(&_npcScriptData);

	if (!_emc->load("_NPC.EMC", &_npcScriptData, &_opcodes))
		error("Could not load \"_NPC.EMC\" script");

	snd_playTheme(1, -1);
	if (_gameToLoad == -1) {
		enterNewScene(_currentCharacter->sceneId, _currentCharacter->facing, 0, 0, 1);
		if (_abortIntroFlag && _skipIntroFlag && saveFileLoadable(0)) {
			_menuDirectlyToLoad = true;
			_screen->setMouseCursor(1, 1, _shapes[0]);
			_screen->showMouse();
			_gui->buttonMenuCallback(0);
			_menuDirectlyToLoad = false;
		} else if (!shouldQuit()) {
			saveGameStateIntern(0, "New game", 0);
		}
	} else {
		_screen->setFont(_flags.lang == Common::JA_JPN ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);
		loadGameStateCheck(_gameToLoad);
		_gameToLoad = -1;
	}
}

void KyraEngine_LoK::mainLoop() {
	// Initialize debugger since how it should be fully usable
	_debugger->initialize();

	_eventList.clear();

	while (!shouldQuit()) {
		int32 frameTime = (int32)_system->getMillis();

		if (_currentCharacter->sceneId == 210) {
			updateKyragemFading();
			if (seq_playEnd() && _deathHandler != 8)
				break;
		}

		if (_deathHandler != -1) {
			snd_playWanderScoreViaMap(0, 1);
			snd_playSoundEffect(49);
			_screen->setMouseCursor(1, 1, _shapes[0]);
			removeHandItem();
			_gui->buttonMenuCallback(0);
			_deathHandler = -1;
		}

		if ((_brandonStatusBit & 2) && _brandonStatusBit0x02Flag)
			_animator->animRefreshNPC(0);

		if ((_brandonStatusBit & 0x20) && _brandonStatusBit0x20Flag) {
			_animator->animRefreshNPC(0);
			_brandonStatusBit0x20Flag = 0;
		}

		// FIXME: Why is this here?
		_screen->showMouse();

		int inputFlag = checkInput(_buttonList, _currentCharacter->sceneId != 210);
		removeInputTop();

		updateMousePointer();
		_timer->update();
		_sound->process();
		updateTextFade();

		if (inputFlag == 198 || inputFlag == 199)
			processInput(_mouseX, _mouseY);

		if (skipFlag())
			resetSkipFlag();

		delay((frameTime + _gameSpeed) - _system->getMillis(), true, true);
	}
}

void KyraEngine_LoK::delayUntil(uint32 timestamp, bool updateTimers, bool update, bool isMainLoop) {
	while (_system->getMillis() < timestamp && !shouldQuit() && !skipFlag()) {
		if (updateTimers)
			_timer->update();

		if (timestamp - _system->getMillis() >= 10)
			delay(10, update, isMainLoop);
	}
}

void KyraEngine_LoK::delay(uint32 amount, bool update, bool isMainLoop) {
	uint32 start = _system->getMillis();
	do {
		if (update) {
			_sprites->updateSceneAnims();
			_animator->updateAllObjectShapes();
			updateTextFade();
			updateMousePointer();
		} else {
			// We need to do Screen::updateScreen here, since client code
			// relies on this method to copy screen changes to the actual
			// screen since at least 0af418e7ea3a41f93fcc551a45ee5bae822d812a.
			_screen->updateScreen();
		}

		_isSaveAllowed = isMainLoop;
		updateInput();
		_isSaveAllowed = false;

		if (_currentCharacter && _currentCharacter->sceneId == 210 && update)
			updateKyragemFading();

		if (amount > 0 && !skipFlag() && !shouldQuit())
			_system->delayMillis(10);

		// FIXME: Major hackery to allow skipping the intro
		if (_seqPlayerFlag) {
			for (Common::List<Event>::iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
				if (i->causedSkip) {
					if (i->event.type == Common::EVENT_KEYDOWN && i->event.kbd.keycode == Common::KEYCODE_ESCAPE)
						_abortIntroFlag = true;
					else
						i->causedSkip = false;
				}
			}
		}

		if (skipFlag())
			snd_stopVoice();
	} while (!skipFlag() && _system->getMillis() < start + amount && !shouldQuit());
}

bool KyraEngine_LoK::skipFlag() const {
	return KyraEngine_v1::skipFlag() || shouldQuit();
}

void KyraEngine_LoK::resetSkipFlag(bool removeEvent) {
	if (removeEvent) {
		_eventList.clear();
	} else {
		KyraEngine_v1::resetSkipFlag(false);
	}
}

void KyraEngine_LoK::delayWithTicks(int ticks) {
	uint32 nextTime = _system->getMillis() + ticks * _tickLength;

	while (_system->getMillis() < nextTime) {
		_sprites->updateSceneAnims();
		_animator->updateAllObjectShapes();

		if (_currentCharacter->sceneId == 210) {
			updateKyragemFading();
			seq_playEnd();
		}

		if (skipFlag())
			break;

		if (nextTime - _system->getMillis() >= 10)
			delay(10);
	}
}

#pragma mark -
#pragma mark - Animation/shape specific code
#pragma mark -

void KyraEngine_LoK::setupShapes123(const Shape *shapeTable, int endShape, int flags) {
	for (int i = 123; i <= 172; ++i)
		_shapes[i] = 0;

	uint8 curImage = 0xFF;
	int curPageBackUp = _screen->_curPage;
	_screen->_curPage = 8;  // we are using page 8 here in the original page 2 was backuped and then used for this stuff
	int shapeFlags = 2;
	if (flags)
		shapeFlags = 3;
	for (int i = 123; i < 123 + endShape; ++i) {
		uint8 newImage = shapeTable[i - 123].imageIndex;
		if (newImage != curImage && newImage != 0xFF) {
			assert(_characterImageTable);
			_screen->loadBitmap(_characterImageTable[newImage], 8, 8, 0);
			curImage = newImage;
		}
		_shapes[i] = _screen->encodeShape(shapeTable[i - 123].x << 3, shapeTable[i - 123].y, shapeTable[i - 123].w << 3, shapeTable[i - 123].h, shapeFlags);
		assert(i - 7 < _defaultShapeTableSize);
		_defaultShapeTable[i - 7].xOffset = shapeTable[i - 123].xOffset;
		_defaultShapeTable[i - 7].yOffset = shapeTable[i - 123].yOffset;
		_defaultShapeTable[i - 7].w = shapeTable[i - 123].w;
		_defaultShapeTable[i - 7].h = shapeTable[i - 123].h;
	}
	_screen->_curPage = curPageBackUp;
}

void KyraEngine_LoK::freeShapes123() {
	for (int i = 123; i <= 172; ++i) {
		delete[] _shapes[i];
		_shapes[i] = 0;
	}
}

#pragma mark -
#pragma mark - Misc stuff
#pragma mark -

Movie *KyraEngine_LoK::createWSAMovie() {
	if (_flags.platform == Common::kPlatformAmiga)
		return new WSAMovieAmiga(this);

	return new WSAMovie_v1(this);
}

void KyraEngine_LoK::setBrandonPoisonFlags(int reset) {
	_brandonStatusBit |= 1;

	if (reset)
		_poisonDeathCounter = 0;

	for (int i = 0; i < 0x100; ++i)
		_brandonPoisonFlagsGFX[i] = i;

	_brandonPoisonFlagsGFX[0x99] = 0x34;
	_brandonPoisonFlagsGFX[0x9A] = 0x35;
	_brandonPoisonFlagsGFX[0x9B] = 0x37;
	_brandonPoisonFlagsGFX[0x9C] = 0x38;
	_brandonPoisonFlagsGFX[0x9D] = 0x2B;
}

void KyraEngine_LoK::resetBrandonPoisonFlags() {
	_brandonStatusBit = 0;

	for (int i = 0; i < 0x100; ++i)
		_brandonPoisonFlagsGFX[i] = i;
}

#pragma mark -
#pragma mark - Input
#pragma mark -

void KyraEngine_LoK::processInput(int xpos, int ypos) {
	if (processInputHelper(xpos, ypos))
		return;

	uint8 item = findItemAtPos(xpos, ypos);
	if (item == 0xFF) {
		_changedScene = false;
		int handled = clickEventHandler(xpos, ypos);
		if (_changedScene || handled)
			return;
	}

	// XXX _deathHandler specific
	if (ypos <= 158) {
		uint16 exit = 0xFFFF;

		if (xpos < 12)
			exit = _walkBlockWest;
		else if (xpos >= 308)
			exit = _walkBlockEast;
		else if (ypos >= 136)
			exit = _walkBlockSouth;
		else if (ypos < 12)
			exit = _walkBlockNorth;

		if (exit != 0xFFFF) {
			handleSceneChange(xpos, ypos, 1, 1);
			return;
		} else {
			int script = checkForNPCScriptRun(xpos, ypos);
			if (script >= 0) {
				runNpcScript(script);
				return;
			}
			if (_itemInHand != kItemNone) {
				if (ypos < 155) {
					if (hasClickedOnExit(xpos, ypos)) {
						handleSceneChange(xpos, ypos, 1, 1);
						return;
					}

					dropItem(0, _itemInHand, xpos, ypos, 1);
				}
			} else {
				if (ypos <= 155)
					handleSceneChange(xpos, ypos, 1, 1);
			}
		}
	}
}

int KyraEngine_LoK::processInputHelper(int xpos, int ypos) {
	uint8 item = findItemAtPos(xpos, ypos);
	if (item != 0xFF) {
		if (_itemInHand == kItemNone) {
			_animator->animRemoveGameItem(item);
			snd_playSoundEffect(53);
			assert(_currentCharacter->sceneId < _roomTableSize);
			Room *currentRoom = &_roomTable[_currentCharacter->sceneId];
			int item2 = currentRoom->itemsTable[item];
			currentRoom->itemsTable[item] = kItemNone;
			setMouseItem(item2);
			assert(_itemList && _takenList);
			updateSentenceCommand(_itemList[getItemListIndex(item2)], _takenList[0], 179);
			_itemInHand = item2;
			clickEventHandler2();
			return 1;
		} else {
			exchangeItemWithMouseItem(_currentCharacter->sceneId, item);
			return 1;
		}
	}
	return 0;
}

int KyraEngine_LoK::clickEventHandler(int xpos, int ypos) {
	_emc->init(&_scriptClick, &_scriptClickData);
	_scriptClick.regs[1] = xpos;
	_scriptClick.regs[2] = ypos;
	_scriptClick.regs[3] = 0;
	_scriptClick.regs[4] = _itemInHand;
	_emc->start(&_scriptClick, 1);

	while (_emc->isValid(&_scriptClick))
		_emc->run(&_scriptClick);

	return _scriptClick.regs[3];
}

void KyraEngine_LoK::updateMousePointer(bool forceUpdate) {
	int shape = 0;

	int newMouseState = 0;
	int newX = 0;
	int newY = 0;
	Common::Point mouse = getMousePos();
	if (mouse.y <= 158) {
		if (mouse.x >= 12) {
			if (mouse.x >= 308) {
				if (_walkBlockEast == 0xFFFF) {
					newMouseState = -2;
				} else {
					newMouseState = -5;
					shape = 3;
					newX = 7;
					newY = 5;
				}
			} else if (mouse.y >= 136) {
				if (_walkBlockSouth == 0xFFFF) {
					newMouseState = -2;
				} else {
					newMouseState = -4;
					shape = 4;
					newX = 5;
					newY = 7;
				}
			} else if (mouse.y < 12) {
				if (_walkBlockNorth == 0xFFFF) {
					newMouseState = -2;
				} else {
					newMouseState = -6;
					shape = 2;
					newX = 5;
					newY = 1;
				}
			}
		} else {
			if (_walkBlockWest == 0xFFFF) {
				newMouseState = -2;
			} else {
				newMouseState = -3;
				newX = 1;
				newY = shape = 5;
			}
		}
	}

	if (mouse.x >= _entranceMouseCursorTracks[0] && mouse.y >= _entranceMouseCursorTracks[1]
	        && mouse.x <= _entranceMouseCursorTracks[2] && mouse.y <= _entranceMouseCursorTracks[3]) {
		switch (_entranceMouseCursorTracks[4]) {
		case 0:
			newMouseState = -6;
			shape = 2;
			newX = 5;
			newY = 1;
			break;

		case 2:
			newMouseState = -5;
			shape = 3;
			newX = 7;
			newY = 5;
			break;

		case 4:
			newMouseState = -4;
			shape = 4;
			newX = 5;
			newY = 7;
			break;

		case 6:
			newMouseState = -3;
			shape = 5;
			newX = 1;
			newY = 5;
			break;

		default:
			break;
		}
	}

	if (newMouseState == -2) {
		shape = 6;
		newX = 4;
		newY = 4;
	}

	if ((newMouseState && _mouseState != newMouseState) || (newMouseState && forceUpdate)) {
		_mouseState = newMouseState;
		_screen->setMouseCursor(newX, newY, _shapes[shape]);
	}

	if (!newMouseState) {
		if (_mouseState != _itemInHand || forceUpdate) {
			if (mouse.y > 158 || (mouse.x >= 12 && mouse.x < 308 && mouse.y < 136 && mouse.y >= 12) || forceUpdate) {
				_mouseState = _itemInHand;
				if (_itemInHand == kItemNone)
					_screen->setMouseCursor(1, 1, _shapes[0]);
				else
					_screen->setMouseCursor(8, 15, _shapes[216 + _itemInHand]);
			}
		}
	}
}

bool KyraEngine_LoK::hasClickedOnExit(int xpos, int ypos) {
	if (xpos < 16 || xpos >= 304)
		return true;

	if (ypos < 8)
		return true;

	if (ypos < 136 || ypos > 155)
		return false;

	return true;
}

void KyraEngine_LoK::clickEventHandler2() {
	Common::Point mouse = getMousePos();

	_emc->init(&_scriptClick, &_scriptClickData);
	_scriptClick.regs[0] = _currentCharacter->sceneId;
	_scriptClick.regs[1] = mouse.x;
	_scriptClick.regs[2] = mouse.y;
	_scriptClick.regs[4] = _itemInHand;
	_emc->start(&_scriptClick, 6);

	while (_emc->isValid(&_scriptClick))
		_emc->run(&_scriptClick);
}

int KyraEngine_LoK::checkForNPCScriptRun(int xpos, int ypos) {
	int returnValue = -1;
	const Character *currentChar = _currentCharacter;
	int charLeft = 0, charRight = 0, charTop = 0, charBottom = 0;

	int scaleFactor = _scaleTable[currentChar->y1];
	int addX = (((scaleFactor * 8) * 3) >> 8) >> 1;
	int addY = ((scaleFactor * 3) << 4) >> 8;

	charLeft = currentChar->x1 - addX;
	charRight = currentChar->x1 + addX;
	charTop = currentChar->y1 - addY;
	charBottom = currentChar->y1;

	if (xpos >= charLeft && charRight >= xpos && charTop <= ypos && charBottom >= ypos)
		return 0;

	if (xpos > 304 || xpos < 16)
		return -1;

	for (int i = 1; i < 5; ++i) {
		currentChar = &_characterList[i];

		if (currentChar->sceneId != _currentCharacter->sceneId)
			continue;

		charLeft = currentChar->x1 - 12;
		charRight = currentChar->x1 + 11;
		charTop = currentChar->y1 - 48;
		// if (!i)
		//  charBottom = currentChar->y2 - 16;
		// else
		charBottom = currentChar->y1;

		if (xpos < charLeft || xpos > charRight || ypos < charTop || charBottom < ypos)
			continue;

		if (returnValue != -1) {
			if (currentChar->y1 >= _characterList[returnValue].y1)
				returnValue = i;
		} else {
			returnValue = i;
		}
	}

	return returnValue;
}

void KyraEngine_LoK::runNpcScript(int func) {
	_emc->init(&_npcScript, &_npcScriptData);
	_emc->start(&_npcScript, func);
	_npcScript.regs[0] = _currentCharacter->sceneId;
	_npcScript.regs[4] = _itemInHand;
	_npcScript.regs[5] = func;

	while (_emc->isValid(&_npcScript))
		_emc->run(&_npcScript);
}

void KyraEngine_LoK::checkAmuletAnimFlags() {
	if (_brandonStatusBit & 2) {
		seq_makeBrandonNormal2();
		_timer->setCountdown(19, 300);
	}

	if (_brandonStatusBit & 0x20) {
		seq_makeBrandonNormal();
		_timer->setCountdown(19, 300);
	}
}

#pragma mark -

void KyraEngine_LoK::registerDefaultSettings() {
	KyraEngine_v1::registerDefaultSettings();

	// Most settings already have sensible defaults. This one, however, is
	// specific to the Kyra engine.
	ConfMan.registerDefault("walkspeed", 2);
}

void KyraEngine_LoK::readSettings() {
	int talkspeed = ConfMan.getInt("talkspeed");

	// The default talk speed is 60. This should be mapped to "Normal".

	if (talkspeed == 0)
		_configTextspeed = 3;   // Clickable
	if (talkspeed <= 50)
		_configTextspeed = 0;   // Slow
	else if (talkspeed <= 150)
		_configTextspeed = 1;   // Normal
	else
		_configTextspeed = 2;   // Fast

	KyraEngine_v1::readSettings();
}

void KyraEngine_LoK::writeSettings() {
	int talkspeed;

	switch (_configTextspeed) {
	case 0:     // Slow
		talkspeed = 1;
		break;
	case 1:     // Normal
		talkspeed = 60;
		break;
	case 2:     // Fast
		talkspeed = 255;
		break;
	default:    // Clickable
		talkspeed = 0;
	}

	ConfMan.setInt("talkspeed", talkspeed);

	KyraEngine_v1::writeSettings();
}

} // End of namespace Kyra
