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


#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/system.h"
#include "common/archive.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/keyboard.h"
#include "common/textconsole.h"

#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "gui/debugger.h"

#include "touche/midi.h"
#include "touche/touche.h"
#include "touche/graphics.h"

namespace Touche {

ToucheEngine::ToucheEngine(OSystem *system, Common::Language language)
	: Engine(system), _midiPlayer(0), _language(language), _rnd("touche") {
	_saveLoadCurrentPage = 0;
	_saveLoadCurrentSlot = 0;
	_hideInventoryTexts = false;

	_screenRect = Common::Rect(kScreenWidth, kScreenHeight);
	_roomAreaRect = Common::Rect(kScreenWidth, kRoomHeight);

	memset(_flagsTable, 0, sizeof(_flagsTable));

	clearDirtyRects();

	_playSoundCounter = 0;

	_processRandomPaletteCounter = 0;

	_fastWalkMode = false;
	_fastMode = false;

	_currentObjectNum = -1;
	_objectDescriptionNum = 0;
	_speechPlaying = false;

	_roomNeedRedraw = false;
	_fullRedrawCounter = 0;
	_menuRedrawCounter = 0;
	memset(_paletteBuffer, 0, sizeof(_paletteBuffer));

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	SearchMan.addSubDirectoryMatching(gameDataDir, "database");

	DebugMan.addDebugChannel(kDebugEngine,   "Engine",   "Engine debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "Graphics", "Graphics debug level");
	DebugMan.addDebugChannel(kDebugResource, "Resource", "Resource debug level");
	DebugMan.addDebugChannel(kDebugOpcodes,  "Opcodes",  "Opcodes debug level");
	DebugMan.addDebugChannel(kDebugMenu,     "Menu",     "Menu debug level");
	DebugMan.addDebugChannel(kDebugCharset,  "Charset",   "Charset debug level");

	_console = new ToucheConsole(this);
}

ToucheEngine::~ToucheEngine() {
	DebugMan.clearAllDebugChannels();
	delete _console;

	delete _midiPlayer;
}

Common::Error ToucheEngine::run() {
	initGraphics(kScreenWidth, kScreenHeight, true);

	Graphics::setupFont(_language);

	setupOpcodes();

	_midiPlayer = new MidiPlayer;

	// Setup mixer
	syncSoundSettings();

	res_openDataFile();
	res_allocateTables();
	res_loadSpriteImage(18, _menuKitData);
	res_loadImageHelper(_menuKitData, _currentImageWidth, _currentImageHeight);
	res_loadSpriteImage(19, _convKitData);
	res_loadImageHelper(_convKitData, _currentImageWidth, _currentImageHeight);

	mainLoop();

	res_deallocateTables();
	res_closeDataFile();
	return Common::kNoError;
}

void ToucheEngine::restart() {
	_midiPlayer->stop();

	_gameState = kGameStateGameLoop;
	_displayQuitDialog = false;

	memset(_flagsTable, 0, sizeof(_flagsTable));

	clearDirtyRects();

	_currentKeyCharNum = 0;
	initKeyChars(-1);

	for (int i = 0; i < NUM_SEQUENCES; ++i) {
		_sequenceEntryTable[i].sprNum = -1;
		_sequenceEntryTable[i].seqNum = -1;
	}

	_disabledInputCounter = 0;
	_currentCursorObject = 0;
	setCursor(0);

	_waitingSetKeyCharNum1 = -1;
	_waitingSetKeyCharNum2 = -1;
	_waitingSetKeyCharNum3 = -1;

	_currentEpisodeNum = 0;
	_newEpisodeNum = kStartupEpisode;

	_newMusicNum = 0;
	_currentMusicNum = 0;

	_newSoundNum = 0;
	_newSoundDelay = 0;
	_newSoundPriority = 0;

	_flagsTable[176] = 0;
	_keyCharsTable[0].money = 25;
	_currentAmountOfMoney = 0;

	_giveItemToKeyCharNum = 0;
	_giveItemToObjectNum = 0;
	_giveItemToCounter = 0;

	clearAnimationTable();

	setupInventoryAreas();
	initInventoryObjectsTable();
	initInventoryLists();
	drawInventory(0, 1);

	_talkListEnd = 0;
	_talkListCurrent = 0;
	_talkTextRectDefined = false;
	_talkTextDisplayed = false;
	_talkTextInitialized = false;
	_skipTalkText = false;
	_talkTextSpeed = 0;
	_keyCharTalkCounter = 0;
	_talkTableLastTalkingKeyChar = -1;
	_talkTableLastOtherKeyChar = -1;
	_talkTableLastStringNum = -1;
	_objectDescriptionNum = 0;
	memset(_talkTable, 0, sizeof(_talkTable));

	_conversationChoicesUpdated = false;
	_conversationReplyNum = -1;
	_conversationEnded = false;
	_conversationNum = 0;
	_scrollConversationChoiceOffset = 0;
	_currentConversation = 0;
	_disableConversationScript = false;
	_conversationAreaCleared = false;
	memset(_conversationChoicesTable, 0, sizeof(_conversationChoicesTable));

	_currentRoomNum = 0;

	_flagsTable[901] = 1;
//	_flagsTable[902] = 1;
	if (_language == Common::FR_FRA) {
		_flagsTable[621] = 1;
	}
}

void ToucheEngine::readConfigurationSettings() {
	if (ConfMan.getBool("speech_mute")) {
		_talkTextMode = kTalkModeTextOnly;
		if (!ConfMan.getBool("subtitles")) {
			ConfMan.setBool("subtitles", true);
		}
	} else {
		if (ConfMan.getBool("subtitles")) {
			_talkTextMode = kTalkModeVoiceAndText;
		} else {
			_talkTextMode = kTalkModeVoiceOnly;
		}
	}
	_midiPlayer->setVolume(ConfMan.getInt("music_volume"));
}

void ToucheEngine::writeConfigurationSettings() {
	switch (_talkTextMode) {
	case kTalkModeTextOnly:
		ConfMan.setBool("speech_mute", true);
		ConfMan.setBool("subtitles", true);
		break;
	case kTalkModeVoiceOnly:
		ConfMan.setBool("speech_mute", false);
		ConfMan.setBool("subtitles", false);
		break;
	case kTalkModeVoiceAndText:
		ConfMan.setBool("speech_mute", false);
		ConfMan.setBool("subtitles", true);
		break;
	}
	ConfMan.setInt("music_volume", _midiPlayer->getVolume());
	ConfMan.flushToDisk();
}

Common::Point ToucheEngine::getMousePos() const {
	return _eventMan->getMousePos();
}

void ToucheEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	readConfigurationSettings();
}

void ToucheEngine::mainLoop() {
	restart();

	setPalette(0, 255, 0, 0, 0);

	readConfigurationSettings();

	_inp_leftMouseButtonPressed = false;
	_inp_rightMouseButtonPressed = false;

	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 99) {
			loadGameState(saveSlot);
			_newEpisodeNum = 0;
			resetSortedKeyCharsTable();
			showCursor(true);
		}
	} else {
		_newEpisodeNum = ConfMan.getInt("boot_param");
		if (_newEpisodeNum == 0) {
			_newEpisodeNum = kStartupEpisode;
		}
		showCursor(_newEpisodeNum != kStartupEpisode);
	}

	uint32 frameTimeStamp = _system->getMillis();
	for (uint32 cycleCounter = 0; !shouldQuit(); ++cycleCounter) {
		if ((cycleCounter % 3) == 0) {
			runCycle();
		}
		if ((cycleCounter % 2) == 0) {
			fadePaletteFromFlags();
		}

		uint32 nextFrame = frameTimeStamp + (_fastMode ? 10 : kCycleDelay);
		uint32 now = _system->getMillis();
		if (nextFrame < now) {
			nextFrame = now + 1;
		}
		do {
			processEvents();
			_system->updateScreen();
			_system->delayMillis(10);
			now = _system->getMillis();
		} while (now < nextFrame && !_fastMode);
		frameTimeStamp = nextFrame;
	}

	writeConfigurationSettings();
}

void ToucheEngine::processEvents(bool handleKeyEvents) {
	Common::Event event;
	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (!handleKeyEvents) {
				break;
			}
			_flagsTable[600] = event.kbd.keycode;
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				if (_displayQuitDialog) {
					if (displayQuitDialog()) {
						quitGame();
					}
				}
			} else if (event.kbd.keycode == Common::KEYCODE_F5) {
				if (_flagsTable[618] == 0 && !_hideInventoryTexts) {
					handleOptions(0);
				}
			} else if (event.kbd.keycode == Common::KEYCODE_F9) {
				_fastWalkMode = true;
			} else if (event.kbd.keycode == Common::KEYCODE_F10) {
				_fastWalkMode = false;
			}
			if (event.kbd.hasFlags(Common::KBD_CTRL)) {
				if (event.kbd.keycode == Common::KEYCODE_f) {
					_fastMode = !_fastMode;
				} else if (event.kbd.keycode == Common::KEYCODE_d) {
					this->getDebugger()->attach();
					this->getDebugger()->onFrame();
				}
			} else {
				if (event.kbd.ascii == 't') {
					++_talkTextMode;
					if (_talkTextMode == kTalkModeCount) {
						_talkTextMode = 0;
					}
					displayTextMode(-(92 + _talkTextMode));
				} else if (event.kbd.ascii == ' ') {
					updateKeyCharTalk(2);
				}
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			_inp_leftMouseButtonPressed = true;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_inp_rightMouseButtonPressed = true;
			break;
		case Common::EVENT_RBUTTONUP:
			_inp_rightMouseButtonPressed = false;
			break;
		default:
			break;
		}
	}
}

void ToucheEngine::runCycle() {
	debugC(9, kDebugEngine, "ToucheEngine::runCycle()");
	if (_flagsTable[290]) {
		changePaletteRange();
	}
	if (_flagsTable[270]) {
		playSoundInRange();
	}
	if (_conversationEnded) {
		_disabledInputCounter = 0;
		_fullRedrawCounter = 1;
		_roomAreaRect.setHeight(kRoomHeight);
		_hideInventoryTexts = false;
		_conversationEnded = false;
		drawInventory(_currentKeyCharNum, 1);
	}
	if (_giveItemToCounter == 1) {
		_fullRedrawCounter = 1;
		drawInventory(_giveItemToObjectNum, 1);
		++_giveItemToCounter;
	}
	if (_giveItemToCounter == -1) {
		_giveItemToCounter = 0;
		_roomAreaRect.setHeight(320);
		_keyCharsTable[_giveItemToKeyCharNum].flags &= ~kScriptPaused;
	}
	setupNewEpisode();
	startNewMusic();
	startNewSound();
	updateSpeech();
	handleConversation();
	if (scrollRoom(_currentKeyCharNum)) {
		_fullRedrawCounter |= 1;
	}
	redrawRoom();
	clearDirtyRects();
	updateRoomRegions();
	if (_flagsTable[612] != 0) {
		_flagsTable[613] = getRandomNumber(_flagsTable[612]);
	}
	sortKeyChars();
	for (int i = 0; i < NUM_KEYCHARS; ++i) {
		runKeyCharScript(&_keyCharsTable[i]);
	}
	if (_roomNeedRedraw) {
		scrollRoom(_currentKeyCharNum);
		redrawRoom();
		_roomNeedRedraw = false;
	}
	updateSpeech();
	for (int i = 0; i < NUM_KEYCHARS; ++i) {
		waitForKeyCharPosition(i);
	}
	redrawBackground();
	waitForKeyCharsSet();
	handleMouseInput(0);
	for (int i = 0; i < NUM_KEYCHARS; ++i) {
		drawKeyChar(&_keyCharsTable[i]);
	}
	processAnimationTable();
	updateKeyCharTalk(0);
	updateDirtyScreenAreas();
	++_flagsTable[295];
	++_flagsTable[296];
	++_flagsTable[297];
	if (_flagsTable[298]) {
		--_flagsTable[298];
	}
	if (_flagsTable[299]) {
		--_flagsTable[299];
	}
	processEvents();
}

int16 ToucheEngine::getRandomNumber(int max) {
	assert(max > 0);
	return _rnd.getRandomNumber(max - 1);
}

void ToucheEngine::changePaletteRange() {
	if (_processRandomPaletteCounter) {
		--_processRandomPaletteCounter;
	} else {
		int scale = _flagsTable[291] + getRandomNumber(_flagsTable[292]);
		setPalette(0, 240, scale, scale, scale);
		_processRandomPaletteCounter = _flagsTable[293] + getRandomNumber(_flagsTable[294]);
	}
}

void ToucheEngine::playSoundInRange() {
	if (_playSoundCounter != 0) {
		--_playSoundCounter;
	} else {
		int16 flag = getRandomNumber(_flagsTable[270]);
		int16 num = _flagsTable[273 + flag];
		res_loadSound(0, num);
		_playSoundCounter = _flagsTable[271] + getRandomNumber(_flagsTable[272]);
	}
}

void ToucheEngine::resetSortedKeyCharsTable() {
	for (int i = 0; i < NUM_KEYCHARS; ++i) {
		_sortedKeyCharsTable[i] = &_keyCharsTable[i];
	}
}

void ToucheEngine::setupEpisode(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::setupEpisode() num=%d", num);
	res_stopSpeech();
	resetTalkingVars();
	res_loadSpeech(-1);
	_currentObjectNum = -1;
	if (num != -1) {
		_updatedRoomAreasTable[0] = 1;
		initKeyChars(-1);
		for (int i = 200; i < 300; ++i) {
			_flagsTable[i] = 0;
		}
		_flagsTable[291] = 240;
		_flagsTable[292] = 16;
		_flagsTable[293] = 0;
		_flagsTable[294] = 1;
		_currentEpisodeNum = num;
		if (_flagsTable[911] != 0) {
			// load scripts from external data files
		}
		debug(0, "Setting up episode %d", num);
		res_loadProgram(num);
		_disabledInputCounter = 0;
	}
	res_decodeProgramData();
	_roomAreaRect.setHeight(kRoomHeight);
	_disableConversationScript = false;
	_hideInventoryTexts = false;
	_conversationEnded = false;
	clearRoomArea();
	drawInventory(_currentKeyCharNum, 1);
}

void ToucheEngine::setupNewEpisode() {
	debugC(9, kDebugEngine, "ToucheEngine::setupNewEpisode() _newEpisodeNum=%d", _newEpisodeNum);
	if (_newEpisodeNum) {
		if (_newEpisodeNum == 91) {
			_displayQuitDialog = true;
		}
		res_stopSound();
		res_stopSpeech();
		setupEpisode(_newEpisodeNum);
		runCurrentKeyCharScript(1);
		_newEpisodeNum = 0;
		resetSortedKeyCharsTable();
	}
}

void ToucheEngine::drawKeyChar(KeyChar *key) {
	debugC(9, kDebugEngine, "ToucheEngine::drawKeyChar()");
	if (key->num != 0) {
		Common::Rect r(key->prevBoundingRect);
		r.extend(key->boundingRect);
		addToDirtyRect(r);
	}
}

void ToucheEngine::sortKeyChars() {
	debugC(9, kDebugEngine, "ToucheEngine::sortKeyChars()");
	for (int i = 0; i < NUM_KEYCHARS; ++i) {
		bool hasSwapped = false;
		for (int j = 0; j < NUM_KEYCHARS - 1; ++j) {
			KeyChar *key1 = _sortedKeyCharsTable[j];
			KeyChar *key2 = _sortedKeyCharsTable[j + 1];
			if (key1->num != 0 && key2->num != 0) {
				if (key1->zPos > key2->zPos) {
					SWAP(_sortedKeyCharsTable[j], _sortedKeyCharsTable[j + 1]);
					hasSwapped = true;
				} else if (key1->zPos == key2->zPos && key1->yPos > key2->yPos) {
					SWAP(_sortedKeyCharsTable[j], _sortedKeyCharsTable[j + 1]);
					hasSwapped = true;
				}
			} else if (key2->num != 0) {
				SWAP(_sortedKeyCharsTable[j], _sortedKeyCharsTable[j + 1]);
				hasSwapped = true;
			}
		}
		if (!hasSwapped) {
			break;
		}
	}
}

void ToucheEngine::runKeyCharScript(KeyChar *key) {
	debugC(9, kDebugEngine, "ToucheEngine::runKeyCharScript() keyChar=%d", (int)(key - _keyCharsTable));
	if (key->scriptDataOffset != 0 && (key->flags & (kScriptStopped | kScriptPaused)) == 0) {
		int16 scriptParam = key->num - 1;
		int16 *prevStackDataPtr = _script.stackDataPtr;
		_script.stackDataPtr = key->scriptStackPtr;
		uint16 prevDataOffset = _script.dataOffset;
		_script.dataOffset = key->scriptDataOffset;
		_script.quitFlag = 0;
		while (_script.quitFlag == 0) {
			executeScriptOpcode(scriptParam);
		}
		switch (_script.quitFlag) {
		case 1: // restart
			key->scriptDataOffset = key->scriptDataStartOffset;
			key->scriptStackPtr = &key->scriptStackTable[39];
			break;
		case 3: // pause
			key->flags |= kScriptPaused;
			key->flags &= ~kScriptStopped;
			key->scriptDataOffset = _script.dataOffset;
			key->scriptStackPtr = _script.stackDataPtr;
			break;
		default: // stop
			key->flags |= kScriptStopped;
			key->flags &= ~kScriptPaused;
			key->scriptDataOffset = 0;
			break;
		}
		_script.dataOffset = prevDataOffset;
		_script.stackDataPtr = prevStackDataPtr;
	}
}

void ToucheEngine::runCurrentKeyCharScript(int mode) {
	debugC(9, kDebugEngine, "ToucheEngine::runCurrentKeyCharScript() _currentKeyCharNum=%d mode=%d", _currentKeyCharNum, mode);
	KeyChar *key = &_keyCharsTable[_currentKeyCharNum];
	if (mode == 1) {
		_script.dataOffset = 0;
		_script.stackDataPtr = key->scriptStackPtr;
	}
	if (mode != 0) {
		while (_script.quitFlag == 0) {
			executeScriptOpcode(0);
		}
		if (mode == 1) {
			centerScreenToKeyChar(_currentKeyCharNum);
		}
		if (_script.quitFlag == 3) {
			key->flags |= kScriptPaused;
			key->flags &= ~kScriptStopped;
			key->scriptDataOffset = _script.dataOffset;
			key->scriptStackPtr = _script.stackDataPtr;
		}
	}
	handleMouseInput(1);
}

void ToucheEngine::executeScriptOpcode(int16 param) {
	debugC(9, kDebugOpcodes, "ToucheEngine::executeScriptOpcode(%d) offset=%04X", param, _script.dataOffset);
	_script.keyCharNum = param;
	_script.opcodeNum = _script.readNextByte();
	if (_script.opcodeNum < _numOpcodes) {
		OpcodeProc op = _opcodesTable[_script.opcodeNum];
		if (op) {
			(this->*op)();
			return;
		}
	}
	error("Invalid opcode 0x%X", _script.opcodeNum);
}

void ToucheEngine::initKeyChars(int keyChar) {
	debugC(9, kDebugEngine, "ToucheEngine::initKeyChars() keyChar=%d", keyChar);
	int indexStart, indexEnd;
	if (keyChar == -1) {
		indexStart = 0;
		indexEnd = NUM_KEYCHARS;
	} else {
		indexStart = keyChar;
		indexEnd = keyChar + 1;
	}
	Common::Rect defaultKeyCharRect(10, 10, 11, 11);
	for (int i = indexStart; i < indexEnd; ++i) {
		KeyChar *key = &_keyCharsTable[i];
		if (keyChar != -1 && key->num != 0) {
			Common::Rect r(key->prevBoundingRect);
			r.extend(key->boundingRect);
			addToDirtyRect(r);
		}
		key->num = 0;
		key->strNum = 0;
		key->textColor = 253;
		key->currentAnimCounter = 0;
		key->currentAnimSpeed = 0;
		key->currentAnim = 0;
		key->framesListCount = 0;
		key->currentFrame = 0;
		key->anim1Start = 0;
		key->anim1Count = 1;
		key->anim2Start = 0;
		key->anim2Count = 1;
		key->anim3Start = 0;
		key->anim3Count = 1;
		key->facingDirection = 0;
		key->sequenceDataOffset = 0;
		key->walkDataNum = 0;
		key->walkPointsList[0] = -1;
		key->walkPointsListIndex = 0;
		key->delay = 0;
		key->waitingKeyChar = -1;
		key->flags = 0;
		key->scriptDataOffset = 0;
		key->scriptStackPtr = &key->scriptStackTable[39];
		key->xPos = 10;
		// like the original interpreter, don't reset yPos here. Doing so causes
		// glitches during the introduction for example (talk texts get displayed
		// at the wrong coordinates).
		key->boundingRect = defaultKeyCharRect;
		key->prevBoundingRect = defaultKeyCharRect;
	}
}

void ToucheEngine::setKeyCharTextColor(int keyChar, uint16 color) {
	debugC(9, kDebugEngine, "ToucheEngine::setKeyCharTextColor(%d) color=%d", keyChar, color);
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	_keyCharsTable[keyChar].textColor = color;
}

void ToucheEngine::waitForKeyCharPosition(int keyChar) {
	debugC(9, kDebugEngine, "ToucheEngine::waitForKeyCharPosition(%d)", keyChar);
	KeyChar *key = _sortedKeyCharsTable[keyChar];
	if (key->num != 0) {
		key->prevBoundingRect = key->boundingRect;
		moveKeyChar(_offscreenBuffer, kScreenWidth, key);
		key->boundingRect = _moveKeyCharRect;
		if (key->delay != 0) {
			--key->delay;
			if (key->delay == 0) {
				key->flags &= ~kScriptPaused;
			}
			return;
		}
		if (key->waitingKeyChar == -1) {
			return;
		}
		KeyChar *nextKey = &_keyCharsTable[key->waitingKeyChar];
		if (nextKey->currentAnim != key->waitingKeyCharPosTable[0] &&
		    nextKey->pointsDataNum != key->waitingKeyCharPosTable[1] &&
		    nextKey->walkDataNum != key->waitingKeyCharPosTable[2]) {
			return;
		}
		key->flags &= ~kScriptPaused;
		key->waitingKeyChar = -1;
	}
}

void ToucheEngine::setKeyCharBox(int keyChar, int value) {
	debugC(9, kDebugEngine, "ToucheEngine::setKeyCharBox(%d) value=%d", keyChar, value);
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	key->prevPointsDataNum = key->pointsDataNum = value;
	key->xPosPrev = key->xPos = _programPointsTable[value].x;
	key->yPosPrev = key->yPos = _programPointsTable[value].y;
	key->zPosPrev = key->zPos = _programPointsTable[value].z;
	key->prevWalkDataNum = key->walkDataNum = findWalkDataNum(value, 10000);
}

void ToucheEngine::setKeyCharFrame(int keyChar, int16 type, int16 value1, int16 value2) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	switch (type) {
	case 0:
		key->anim2Start = value1;
		key->anim2Count = value2;
		key->anim3Start = value1;
		key->anim3Count = value2;
		break;
	case 1:
		if (value2 != 0) {
			value2 = getRandomNumber(value2);
		}
		key->framesList[key->framesListCount] = value1 + value2;
		++key->framesListCount;
		key->framesListCount &= 15;
		break;
	case 2:
		key->anim1Start = value1;
		key->anim1Count = value2;
		break;
	case 3:
		key->currentAnim = value1;
		key->currentAnimSpeed = 0;
		key->currentAnimCounter = 0;
		break;
	case 4:
		key->anim3Start = value1;
		key->anim3Count = value2;
		break;
	}
}

void ToucheEngine::setKeyCharFacingDirection(int keyChar, int16 dir) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	_keyCharsTable[keyChar].facingDirection = dir;
}

void ToucheEngine::initKeyCharScript(int keyChar, int16 spriteNum, int16 seqDataIndex, int16 seqDataOffs) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	key->num = keyChar + 1;
	key->spriteNum = spriteNum;
	key->sequenceDataIndex = seqDataIndex;
	key->sequenceDataOffset = seqDataOffs;
	key->scriptDataStartOffset = findProgramKeyCharScriptOffset(keyChar);
	key->scriptDataOffset = key->scriptDataStartOffset;
}

uint16 ToucheEngine::findProgramKeyCharScriptOffset(int keyChar) const {
	for (uint i = 0; i < _programKeyCharScriptOffsetTable.size(); ++i) {
		if (_programKeyCharScriptOffsetTable[i].keyChar == keyChar) {
			return _programKeyCharScriptOffsetTable[i].offset;
		}
	}
	return 0;
}

bool ToucheEngine::scrollRoom(int keyChar) {
	if (_flagsTable[616] != 0) {
		return 0;
	}
	KeyChar *key = &_keyCharsTable[keyChar];
	bool needRedraw = false;

	// vertical scrolling
	int prevRoomDy = _flagsTable[615];
	_flagsTable[615] = key->yPos + 32 - kScreenHeight / 2;
	int roomHeight;
	if (_hideInventoryTexts) {
		roomHeight = kRoomHeight;
	} else {
		roomHeight = (_flagsTable[606] != 0) ? kScreenHeight : kRoomHeight;
		_roomAreaRect.setHeight(roomHeight);
	}
	_flagsTable[615] = CLIP<int16>(_flagsTable[615], 0, _currentBitmapHeight - roomHeight);
	if (_flagsTable[615] != prevRoomDy) {
		needRedraw = true;
	}

	// horizontal scrolling
	int prevRoomDx = _flagsTable[614];
	if (key->xPos > prevRoomDx + kScreenWidth - 160) {
		int dx = key->xPos - (prevRoomDx + kScreenWidth - 160);
		prevRoomDx += dx;
	} else if (key->xPos < prevRoomDx + 160) {
		int dx = prevRoomDx + 160 - key->xPos;
		prevRoomDx -= dx;
		if (prevRoomDx < 0) {
			prevRoomDx = 0;
		}
	}
	prevRoomDx = CLIP<int16>(prevRoomDx, 0, _roomWidth - kScreenWidth);
	if (_flagsTable[614] != prevRoomDx) {
		_flagsTable[614] = prevRoomDx;
		return true;
	}
	if (_screenOffset.x == 0) {
		return needRedraw;
	}
	int scrollDx = _screenOffset.x - _flagsTable[614];
	if (scrollDx < -4) {
		scrollDx = -4;
	} else if (scrollDx > 4) {
		scrollDx = 4;
	}
	_flagsTable[614] += scrollDx;

	if (_screenOffset.x == _flagsTable[614]) {
		_screenOffset.x = 0;
	}
	return true;
}

void ToucheEngine::drawIcon(int x, int y, int num) {
	res_loadImage(num, _iconData);
	Graphics::copyRect(_offscreenBuffer, kScreenWidth, x, y,
	  _iconData, kIconWidth, 0, 0,
	  kIconWidth, kIconHeight,
	  Graphics::kTransparent);
}

void ToucheEngine::centerScreenToKeyChar(int keyChar) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	_flagsTable[614] = key->xPos - kScreenWidth / 2;
	_flagsTable[615] = key->yPos - kScreenHeight / 2;
	_flagsTable[615] = CLIP<int16>(_flagsTable[615], 0, _currentBitmapHeight - kRoomHeight);
	scrollRoom(keyChar);
}

void ToucheEngine::waitForKeyCharsSet() {
	if (_waitingSetKeyCharNum2 != -1) {
		KeyChar *key = &_keyCharsTable[_waitingSetKeyCharNum2];
		if (key->framesListCount == key->currentFrame && key->currentAnim == key->anim2Start) {
			key = &_keyCharsTable[_waitingSetKeyCharNum1];
			if (key->framesListCount == key->currentFrame && key->currentAnim == key->anim2Start) {
				key = &_keyCharsTable[_waitingSetKeyCharNum3];
				_waitingSetKeyCharNum2 = -1;
				key->flags &= ~kScriptPaused;
			}
		}
	}
}

void ToucheEngine::redrawRoom() {
	if (_currentBitmapWidth == 0 || _currentBitmapHeight == 0) {
		return;
	}
	int w = kScreenWidth;
	if (_flagsTable[614] < 0 || _flagsTable[614] > _currentBitmapWidth - w) {
		error("Invalid room_x_offset = %d (w=%d, room_w=%d)", _flagsTable[614], w, _currentBitmapWidth);
	}
	int h = (_flagsTable[606] != 0) ? int(kScreenHeight) : _roomAreaRect.height();
	if (_flagsTable[615] < 0 || _flagsTable[615] > _currentBitmapHeight - h) {
		error("Invalid room_y_offset = %d (h=%d, room_h=%d)", _flagsTable[615], h, _currentBitmapHeight);
	}
	uint8 *dst = _offscreenBuffer;
	const uint8 *src = _backdropBuffer + _flagsTable[615] * _currentBitmapWidth + _flagsTable[614];
	while (h--) {
		memcpy(dst, src, w);
		dst += w;
		src += _currentBitmapWidth;
	}
}

void ToucheEngine::fadePalette(int firstColor, int colorCount, int scale, int scaleInc, int fadingStepsCount) {
	for (int i = 0; i < fadingStepsCount; ++i) {
		scale += scaleInc;
		scale = CLIP(scale, 0, 255);
		setPalette(firstColor, colorCount, scale, scale, scale);
		_system->updateScreen();
		_system->delayMillis(10);
	}
}

void ToucheEngine::fadePaletteFromFlags() {
	if (_flagsTable[603]) {
		setPalette(_flagsTable[607], _flagsTable[608], _flagsTable[605], _flagsTable[605], _flagsTable[605]);
		if (_flagsTable[603] > 0) {
			if (_flagsTable[605] >= _flagsTable[609]) {
				_flagsTable[603] = 0;
			}
		} else {
			if (_flagsTable[605] <= _flagsTable[610]) {
				_flagsTable[603] = 0;
			}
		}
		_flagsTable[605] += _flagsTable[603];
		if (_flagsTable[605] < 0) {
			_flagsTable[605] = 0;
		} else if (_flagsTable[605] > 255) {
			_flagsTable[605] = 255;
		}
	}
}

static uint8 *getKeyCharFrameData(uint8 *p, uint16 dir1, uint16 dir2, uint16 dir3, uint8 **dst, int16 sequence_num) {
	uint8 *src;
	uint16 offs, num1;

	// spriteData
	// LE16 offset to "sprite copy" data
	// LE16 offset to 4 * 2 * 10 offsets : "sprite info" offset
	// LE16 data offset
	// LE16 ?
	offs = READ_LE_UINT16(p + sequence_num * 8 + 2);
	offs = READ_LE_UINT16(p + offs + dir1 * 4); // facing
	offs = READ_LE_UINT16(p + offs + dir2 * 2); // top/bottom
	src = p + offs + dir3 * 10; // current frame anim ?
	*dst = src;
	// LE16 : if 0x8000 -> offset "sprite copy" data num
	// LE16 : dx
	// LE16 : dy
	// LE16 : dz

	num1 = READ_LE_UINT16(src) & 0x7FFF;
	offs = READ_LE_UINT16(p + sequence_num * 8 + 0);
	offs = READ_LE_UINT16(p + offs + num1 * 2);
	return p + offs;
	// LE16 : srcX
	// LE16 : srcY
	// LE16 : flags (vflip, hflip)
}

void ToucheEngine::moveKeyChar(uint8 *dst, int dstPitch, KeyChar *key) {
	int16 keyChar = key->num - 1;
	int16 walkDataNum = key->walkDataNum;
	int16 clippingRectNum = 0;
	if (walkDataNum != -1) {
		clippingRectNum = _programWalkTable[walkDataNum].clippingRect;
	}
	Common::Rect clippingRect(_programRectsTable[clippingRectNum]);
	clippingRect.translate(-_flagsTable[614], -_flagsTable[615]);
	if (key->flags & 0x8000) {
		clippingRect.moveTo(clippingRect.left, kRoomHeight);
	}
	clippingRect.clip(_roomAreaRect);
	SpriteData *spr = &_spritesTable[key->spriteNum];
	int x1 =  30000, y1 =  30000;
	int x2 = -30000, y2 = -30000;
	int16 keyCharDirection = _flagsTable[266];
	if (keyCharDirection == 0) {
		keyCharDirection = key->facingDirection;
	}
	int16 facingDirection = keyCharDirection;
	uint8 *sequenceDataBase = _sequenceDataTable[key->sequenceDataIndex];
	uint8 *sequenceData = sequenceDataBase;

	uint16 frameDirFlag = READ_LE_UINT16(sequenceData + key->sequenceDataOffset * 8 + 4);
	if (frameDirFlag) {
		sequenceData += frameDirFlag & ~1;
	}

	uint8 *frameData;
	uint8 *frameDataBase = getKeyCharFrameData(sequenceDataBase, key->currentAnim, facingDirection, key->currentAnimCounter, &frameData, key->sequenceDataOffset);
	uint16 frameFlag = READ_LE_UINT16(frameData); frameData += 2;
	uint16 walkDx = READ_LE_UINT16(frameData); frameData += 2;
	uint16 walkDy = READ_LE_UINT16(frameData); frameData += 2;
	uint16 walkDz = READ_LE_UINT16(frameData); frameData += 2;

	if (key->currentAnimSpeed <= 0) {
		key->currentAnimSpeed = READ_LE_UINT16(frameData);
	}
	--key->currentAnimSpeed;
	if (key->currentAnimSpeed <= 0) {
		++key->currentAnimCounter;
	}
	if (_fastWalkMode) {
		walkDx *= 2;
		walkDy *= 2;
		walkDz *= 2;
	}
	updateKeyCharWalkPath(key, walkDx, walkDy, walkDz);
	int posX = key->xPos;
	int posY = key->yPos;
	int posZ = key->zPos;
	if (frameFlag & 0x8000) {
		changeKeyCharFrame(key, keyChar);
	}
	posX -= _flagsTable[614];
	posY -= _flagsTable[615];
	if (posZ == 160) { // draw sprite frames without rescaling
		while (1) {
			int dstX = (int16)READ_LE_UINT16(frameDataBase); frameDataBase += 2;
			if (dstX == 10000) {
				_moveKeyCharRect = Common::Rect(x1, y1, x2 + spr->w, y2 + spr->h);
				break;
			}
			int dstY = (int16)READ_LE_UINT16(frameDataBase); frameDataBase += 2;

			if (facingDirection == 3) {
				dstX = -dstX - spr->w;
			}
			dstX += posX;
			dstY += posY;
			x1 = MIN(dstX, x1);
			x2 = MAX(dstX, x2);
			y1 = MIN(dstY, y1);
			y2 = MAX(dstY, y2);

			int frameDir = READ_LE_UINT16(frameDataBase); frameDataBase += 2;
//			assert((frameDir & 0x4000) == 0); // hflipped
			bool vflipped = (frameDir & 0x8000) != 0;

			frameDir &= 0xFFF;
			if (frameDirFlag) {
				frameDir = READ_LE_UINT16(sequenceData + frameDir * 2);
			}
			if (keyChar == 0) {
				assert(frameDir < NUM_DIRECTIONS);
				if (_directionsTable[frameDir] <= _flagsTable[176]) {
					continue;
				}
			}
			if (frameDir == 0x800) {
				continue;
			}

			assert(spr->w != 0);
			int framesPerLine = spr->bitmapWidth / spr->w;
			assert(framesPerLine != 0);
			const int srcOffsX = spr->w * (frameDir % framesPerLine);
			const int srcOffsY = spr->h * (frameDir / framesPerLine);

			Area copyRegion(dstX, dstY, spr->w, spr->h);
			copyRegion.srcX = 0;
			copyRegion.srcY = 0;
			if (!copyRegion.clip(clippingRect)) {
				continue;
			}

			if (facingDirection == 3) {
				vflipped = !vflipped;
			}

			uint8 *dstCur = dst + copyRegion.r.top * dstPitch + copyRegion.r.left;
			const uint8 *srcSpr = spr->ptr + (srcOffsY + copyRegion.srcY) * spr->bitmapWidth;
			srcSpr += vflipped ? srcOffsX + spr->w - 1 - copyRegion.srcX : srcOffsX + copyRegion.srcX;
			for (int h = 0; h < copyRegion.r.height(); ++h) {
				for (int w = 0; w < copyRegion.r.width(); ++w) {
					uint8 color = vflipped ? srcSpr[-w] : srcSpr[w];
					if (color != 0) {
						dstCur[w] = color;
					}
				}
				srcSpr += spr->bitmapWidth;
				dstCur += dstPitch;
			}
		}
	} else { // draw sprite frames with rescaling
		y2 = posY;
		int clippingRect_x1 = clippingRect.left;
		int clippingRect_y1 = clippingRect.top;
		int clippingRect_x2 = clippingRect.right;
		int clippingRect_y2 = clippingRect.bottom;
		buildSpriteScalingTable(160, posZ);
		while (1) {
			int dstX = (int16)READ_LE_UINT16(frameDataBase); frameDataBase += 2;
			if (dstX == 10000) {
				_moveKeyCharRect = Common::Rect(x1, y1, x2 + 1, y2 + 1);
				break;
			}
			int dstY = (int16)READ_LE_UINT16(frameDataBase); frameDataBase += 2;

			int frameDir = READ_LE_UINT16(frameDataBase); frameDataBase += 2;
//			assert((frameDir & 0x4000) == 0); // hflipped
			bool vflipped = (frameDir & 0x8000) != 0;

			frameDir &= 0xFFF;
			if (frameDirFlag) {
				frameDir = READ_LE_UINT16(sequenceData + frameDir * 2);
			}
			if (keyChar == 0) {
				assert(frameDir < NUM_DIRECTIONS);
				if (_directionsTable[frameDir] <= _flagsTable[176]) {
					continue;
				}
			}
			if (frameDir == 0x800) {
				continue;
			}

			assert(spr->w != 0);
			int framesPerLine = spr->bitmapWidth / spr->w;
			assert(framesPerLine != 0);
			const int srcOffsX = spr->w * (frameDir % framesPerLine);
			const int srcOffsY = spr->h * (frameDir / framesPerLine);
			const uint8 *srcSpr = spr->ptr + srcOffsY * spr->bitmapWidth + srcOffsX;

			assert(dstY >= -500 && dstY < 500);
			int scalingIndex = _spriteScalingIndex[500 + dstY];
			int16 *yScaledTable = &_spriteScalingTable[scalingIndex];
			int sprScaledY = posY + scalingIndex - 500;
			y1 = MIN(y1, sprScaledY);

			if (facingDirection == 3) {
				dstX = -dstX;
			}
			assert(dstX >= -500 && dstX < 500);
			scalingIndex = _spriteScalingIndex[500 + dstX];
			int sprScaledX = posX + scalingIndex - 500;
			int16 *xScaledTable = &_spriteScalingTable[scalingIndex];
			x1 = MIN(x1, sprScaledX);
			x2 = MAX(x2, sprScaledX);

			uint8 *dstCur = dst + sprScaledY * dstPitch + sprScaledX;

			uint8 *dstStart = dstCur;
			int sprStartY = 0;
			while (1) {
				int sprCurY = *yScaledTable - dstY; ++yScaledTable;
				if (sprCurY >= spr->h) {
					break;
				}
				sprStartY = sprCurY - sprStartY;
				while (sprStartY != 0) {
					srcSpr += spr->bitmapWidth;
					--sprStartY;
				}
				sprStartY = sprCurY;

				int16 *scalingTable = xScaledTable;
				int spr_x2 = sprScaledX;
				dstCur = dstStart;
				if (sprScaledY < clippingRect_y1 || sprScaledY >= clippingRect_y2) {
					continue;
				}
				if (facingDirection != 3) {
					while (1) {
						int spr_x1 = *scalingTable - dstX; ++scalingTable;
						if (spr_x1 >= spr->w || spr_x2 >= clippingRect_x2) {
							break;
						}
						if (spr_x2 >= clippingRect_x1) {
							uint8 color = vflipped ? srcSpr[spr->w - 1 - spr_x1] : srcSpr[spr_x1];
							if (color != 0) {
								*dstCur = color;
							}
						}
						++spr_x2;
						++dstCur;
					}
					x2 = MAX(x2, spr_x2);
				} else {
					while (1) {
						int spr_x1 = dstX - *scalingTable; --scalingTable;
						if (spr_x1 >= spr->w || spr_x2 < clippingRect_x1) {
							break;
						}
						if (spr_x2 < clippingRect_x2) {
							uint8 color = vflipped ? srcSpr[spr->w - 1 - spr_x1] : srcSpr[spr_x1];
							if (color != 0) {
								*dstCur = color;
							}
						}
						--spr_x2;
						--dstCur;
					}
					x1 = MIN(x1, spr_x2);
				}
				++sprScaledY;
				dstStart += dstPitch;
			}
		}
	}
	if (walkDataNum != -1) {
		if (_flagsTable[604] == 0) {
			int area1 = _programWalkTable[walkDataNum].area1;
			if (area1 != 0) {
				findAndRedrawRoomRegion(area1);
			}
			int area2 = _programWalkTable[walkDataNum].area2;
			if (area2 != 0) {
				findAndRedrawRoomRegion(area2);
			}
		}
	}
}

void ToucheEngine::changeKeyCharFrame(KeyChar *key, int keyChar) {
	key->currentAnimSpeed = 0;
	key->currentAnimCounter = 0;
	if (key->currentAnim != 1) {
		int16 animStart, animCount;
		if (_currentObjectNum == keyChar && _flagsTable[901] == 1) {
			animStart = key->anim1Start;
			animCount = key->anim1Count;
		} else if (key->framesListCount != key->currentFrame) {
			animStart = key->framesList[key->currentFrame];
			++key->currentFrame;
			key->currentFrame &= 15;
			animCount = 0;
		} else {
			animStart = key->anim2Start;
			animCount = key->anim2Count;
			if (key->currentAnim >= animStart && key->currentAnim < animStart + animCount) {
				int rnd = getRandomNumber(100);
				if (key->flags & 0x10) {
					if (rnd >= 50 && rnd <= 55) {
						KeyChar *followingKey = &_keyCharsTable[key->followingKeyCharNum];
						int16 num = followingKey->pointsDataNum;
						if (num != 0 && followingKey->currentWalkBox != -1 && num != key->followingKeyCharPos) {
							key->followingKeyCharPos = num;
							sortPointsData(-1, num);
							buildWalkPointsList(key->num - 1);
						}
					}
				} else {
					if (rnd >= 50 && rnd <= 51) {
						animStart = key->anim3Start;
						animCount = key->anim3Count;
					}
				}
			}
		}
		if (animCount != 0) {
			animCount = getRandomNumber(animCount);
		}
		key->currentAnim = animStart + animCount;
	}
}

void ToucheEngine::setKeyCharRandomFrame(KeyChar *key) {
	key->currentAnimSpeed = 0;
	key->currentAnim = key->anim2Start + getRandomNumber(key->anim2Count);
	key->currentAnimCounter = 0;
}

void ToucheEngine::setKeyCharMoney() {
	_keyCharsTable[_currentKeyCharNum].money += _currentAmountOfMoney;
	_currentAmountOfMoney = 0;
	drawAmountOfMoneyInInventory();
}

const char *ToucheEngine::getString(int num) const {
	if (num < 0) {
		return (const char *)_textData + READ_LE_UINT32(_textData - num * 4);
	} else {
		return (const char *)_programTextDataPtr + READ_LE_UINT32(_programTextDataPtr + num * 4);
	}
}

int ToucheEngine::getStringWidth(int num) const {
	const char *str = getString(num);
	if (DebugMan.isDebugChannelEnabled(kDebugCharset)) {
		debug("stringwidth: %s", str);
		debugN("raw:");
		const char *p = str;
		while (*p) debugN(" %02X", (unsigned char)*p++);
		debugN("\n");
	}
	return Graphics::getStringWidth16(str);
}

void ToucheEngine::drawString(uint16 color, int x, int y, int16 num, StringType strType) {
	const int xmax = (_language == Common::ES_ESP && strType == kStringTypeConversation) ? kScreenWidth - 20 : 0;
	if (num) {
		const char *str = getString(num);
		Graphics::drawString16(_offscreenBuffer, kScreenWidth, color, x, y, str, xmax);
	}
}

void ToucheEngine::drawGameString(uint16 color, int x1, int y, const char *str) {
	int w = Graphics::getStringWidth16(str);
	int x = x1 - w / 2;
	if (x + w >= kScreenWidth) {
		x = kScreenWidth - w - 1;
	}
	while (*str) {
		char chr = *str++;
		if (chr == '\\') {
			y += kTextHeight;
			w = Graphics::getStringWidth16(str);
			x = x1 - w / 2;
		} else {
			if (x < 0) {
				x = 0;
			}
			x += Graphics::drawChar16(_offscreenBuffer, kScreenWidth, chr, x, y, color);
		}
	}
}

int ToucheEngine::restartKeyCharScriptOnAction(int action, int obj1, int obj2) {
	debugC(9, kDebugEngine, "ToucheEngine::restartKeyCharScriptOnAction(%d, %d, %d)", action, obj1, obj2);
	for (uint i = 0; i < _programActionScriptOffsetTable.size(); ++i) {
		const ProgramActionScriptOffsetData *pasod = &_programActionScriptOffsetTable[i];
		if (pasod->object1 == obj1 && pasod->action == action && pasod->object2 == obj2) {
			debug(0, "Found matching action i=%d %d,%d,%d offset=0x%X", i, pasod->action, pasod->object1, pasod->object2, pasod->offset);
			KeyChar *key = &_keyCharsTable[_currentKeyCharNum];
			key->scriptDataOffset = pasod->offset;
			key->scriptStackPtr = &key->scriptStackTable[39];
			key->flags &= ~(kScriptStopped | kScriptPaused);
			return 1;
		}
	}
	return 0;
}

void ToucheEngine::buildSpriteScalingTable(int z1, int z2) {
	debugC(9, kDebugEngine, "ToucheEngine::buildSpriteScalingTable(%d, %d)", z1, z2);
	if (z2 > 500) {
		z2 = 500;
	}
	if (z2 == 0) {
		z2 = 1;
	}

	memset(_spriteScalingTable, 0, sizeof(_spriteScalingTable));
	const int scaleInc = z1 * 256 / z2;
	int scaleSum = 0;
	for (int i = 0; i < z2; ++i) {
		int value = scaleSum >> 8;
		assert(i < 500);
		_spriteScalingTable[500 + i] =  value;
		_spriteScalingTable[500 - i] = -value;
		scaleSum += scaleInc;
	}

	memset(_spriteScalingIndex, 0, sizeof(_spriteScalingIndex));
	const int16 *p = &_spriteScalingTable[500];
	int16 z1_s = *p++;
	int16 z2_s = *p++;
	for (int i = 0, j = 0; j < z1; ++i) {
		while (z2_s != z1_s) {
			++z1_s;
			assert(j < 500);
			_spriteScalingIndex[500 + j] = i + 500;
			_spriteScalingIndex[500 - j] = 500 - i;
			if (j++ >= z1) {
				break;
			}
		}
		z1_s = z2_s;
		z2_s = *p++;
	}
}

void ToucheEngine::drawSpriteOnBackdrop(int num, int x, int y) {
	assert(num >= 0 && num < NUM_SPRITES);
	SpriteData *spr = &_spritesTable[num];
	Graphics::copyRect(_backdropBuffer, _currentBitmapWidth, x, y,
	  spr->ptr, spr->bitmapWidth, 0, 0,
	  spr->bitmapWidth, spr->bitmapHeight);
}

void ToucheEngine::updateTalkFrames(int keyChar) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	if (key->currentAnim >= key->anim1Start && key->currentAnim < key->anim1Start + key->anim1Count) {
		key->currentAnim = key->anim2Start;
		key->currentAnimCounter = 0;
		key->currentAnimSpeed = 0;
	}
}

void ToucheEngine::setKeyCharTalkingFrame(int keyChar) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	if (key->currentAnim != 1) {
		key->currentAnim = key->anim1Start;
		key->currentAnimCounter = 0;
		key->currentAnimSpeed = 0;
	}
}

void ToucheEngine::lockUnlockHitBox(int num, int lock) {
	for (uint i = 0; i < _programHitBoxTable.size(); ++i) {
		if (_programHitBoxTable[i].item == num) {
			if (lock) {
				_programHitBoxTable[i].hitBoxes[0].top |= 0x4000;
			} else {
				_programHitBoxTable[i].hitBoxes[0].top &= ~0x4000;
			}
		}
	}
}

void ToucheEngine::drawHitBoxes() {
	for (uint i = 0; i < _programHitBoxTable.size(); ++i) {
		uint16 state = _programHitBoxTable[i].state;
		if (state & 0x8000) {
			_programHitBoxTable[i].state = state & 0x7FFF;
			addToDirtyRect(_programHitBoxTable[i].hitBoxes[1]);
		}
	}
}

void ToucheEngine::showCursor(bool show) {
	debugC(9, kDebugEngine, "ToucheEngine::showCursor()");
	CursorMan.showMouse(show);
}

void ToucheEngine::setCursor(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::setCursor(%d)", num);
	_currentCursorObject = num;
	res_loadImage(num, _mouseData);
	CursorMan.replaceCursor(_mouseData, kCursorWidth, kCursorHeight, kCursorWidth / 2, kCursorHeight / 2, 0);
}

void ToucheEngine::setDefaultCursor(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::setDefaultCursor(%d)", num);
	if (_currentCursorObject != 0) {
		if (_currentCursorObject != 1) {
			addItemToInventory(num, _currentCursorObject);
			drawInventory(num, 1);
		}
		setCursor(0);
	}
}

void ToucheEngine::handleLeftMouseButtonClickOnInventory() {
	Common::Point mousePos = getMousePos();
	for (int area = 0; area < ARRAYSIZE(_inventoryAreasTable); ++area) {
		if (_inventoryAreasTable[area].contains(mousePos)) {
			if (area >= kInventoryObject1 && area <= kInventoryObject6) {
				int item = _inventoryVar1[area - 6 + *_inventoryVar2];
				_flagsTable[119] = _currentCursorObject;
				if (_currentCursorObject == 1) {
					setKeyCharMoney();
					_flagsTable[118] = _currentAmountOfMoney;
					_currentAmountOfMoney = 0;
				}
				if (item != 0 && _currentCursorObject != 0) {
					if (restartKeyCharScriptOnAction(-53, item | 0x1000, 0)) {
						setDefaultCursor(_objectDescriptionNum);
						drawInventory(_objectDescriptionNum, 1);
					}
				} else {
					_inventoryVar1[area - 6 + *_inventoryVar2] = 0;
					if (_currentCursorObject != 0) {
						setDefaultCursor(_objectDescriptionNum);
					}
					if (item != 0) {
						setCursor(item);
						packInventoryItems(0);
						packInventoryItems(1);
					}
					drawInventory(_objectDescriptionNum, 1);
				}
			} else {
				switch (area) {
				case kInventoryCharacter:
					_keyCharsTable[_currentKeyCharNum].money += _currentAmountOfMoney;
					_currentAmountOfMoney = 0;
					handleOptions(0);
					break;
				case kInventoryMoneyDisplay:
					setKeyCharMoney();
					if (_currentCursorObject == 1) {
						setCursor(0);
					}
					break;
				case kInventoryGoldCoins:
					if (_keyCharsTable[_currentKeyCharNum].money >= 10) {
						_keyCharsTable[_currentKeyCharNum].money -= 10;
						_currentAmountOfMoney += 10;
						drawAmountOfMoneyInInventory();
					}
					break;
				case kInventorySilverCoins:
					if (_keyCharsTable[_currentKeyCharNum].money != 0) {
						--_keyCharsTable[_currentKeyCharNum].money;
						++_currentAmountOfMoney;
						drawAmountOfMoneyInInventory();
					}
					break;
				case kInventoryMoney:
					if (_currentAmountOfMoney != 0) {
						setDefaultCursor(_objectDescriptionNum);
						int money = _currentAmountOfMoney;
						_currentAmountOfMoney = 0;
						drawAmountOfMoneyInInventory();
						setCursor(1);
						_currentAmountOfMoney = money;
					}
					break;
				case kInventoryScroller1:
					if (*_inventoryVar2 != 0) {
						*_inventoryVar2 -= 6;
						drawInventory(_objectDescriptionNum, 1);
					}
					break;
				case kInventoryScroller2:
					if (_inventoryVar1[6 + *_inventoryVar2] != 0) {
						*_inventoryVar2 += 6;
						drawInventory(_objectDescriptionNum, 1);
					}
					break;
				}
			}
			break;
		}
	}
}

void ToucheEngine::handleRightMouseButtonClickOnInventory() {
	Common::Point mousePos = getMousePos();
	for (int area = kInventoryObject1; area <= kInventoryObject6; ++area) {
		const Common::Rect &r = _inventoryAreasTable[area];
		if (r.contains(mousePos)) {
			int item = _inventoryVar1[area - 6 + *_inventoryVar2] | 0x1000;
			for (uint i = 0; i < _programHitBoxTable.size(); ++i) {
				const ProgramHitBoxData *hitBox = &_programHitBoxTable[i];
				if (hitBox->item == item) {
					const int menuX = r.left + r.width() / 2;
					const int menuY = kRoomHeight;
					int act = handleActionMenuUnderCursor(hitBox->actions, menuX, menuY, hitBox->str);
					if (act != 0) {
						restartKeyCharScriptOnAction(act, hitBox->item, 0);
					}
				}
			}
			break;
		}
	}
}

void ToucheEngine::handleMouseInput(int flag) {
	if (_disabledInputCounter != 0 || _flagsTable[618] != 0) {
		_inp_rightMouseButtonPressed = false;
	}
	if (getMousePos().y < _roomAreaRect.height()) {
		handleMouseClickOnRoom(flag);
	} else {
		handleMouseClickOnInventory(flag);
	}
}

void ToucheEngine::handleMouseClickOnRoom(int flag) {
	if (_hideInventoryTexts && _conversationReplyNum != -1 && !_conversationAreaCleared) {
		drawConversationString(_conversationReplyNum, 0xD6);
	}
	if (_disabledInputCounter == 0 && !_hideInventoryTexts && _flagsTable[618] == 0) {
		bool itemSelected = false;
		bool stringDrawn = false;
		if (_conversationReplyNum != -1 && !_conversationAreaCleared && _giveItemToCounter == 0) {
			drawConversationString(_conversationReplyNum, 0xD6);
		}
		_conversationReplyNum = -1;
		Common::Point mousePos = getMousePos();
		int keyCharNewPosX = _flagsTable[614] + mousePos.x;
		int keyCharNewPosY = _flagsTable[615] + mousePos.y;
		for (uint i = 0; i < _programHitBoxTable.size(); ++i) {
			if (_programHitBoxTable[i].item & 0x1000) {
				break;
			}
			bool itemDisabled = false;
			Common::Rect *hitBox = &_programHitBoxTable[i].hitBoxes[0];
			int hitPosX = keyCharNewPosX;
			int hitPosY = keyCharNewPosY;
			int16 str = _programHitBoxTable[i].str;
			KeyChar *keyChar;
			switch (_programHitBoxTable[i].item & 0xF000) {
			case 0x1000:
				if (_inventoryItemsInfoTable[_programHitBoxTable[i].item & ~0x1000] != 0x20) {
					hitPosY = 10000;
				}
				break;
			case 0x2000:
				itemDisabled = true;
				break;
			case 0x4000:
				keyChar = &_keyCharsTable[_programHitBoxTable[i].item & ~0x4000];
				hitPosY = 10000;
				if (keyChar->num != 0) {
					if ((keyChar->flags & 0x4000) == 0) {
						if (keyChar->strNum != 0) {
							str = _programHitBoxTable[i].defaultStr;
						}
						hitBox = &keyChar->prevBoundingRect;
						hitPosX = mousePos.x;
						hitPosY = mousePos.y;
					}
				}
				break;
			}
			if (_giveItemToCounter == 0 && !_hideInventoryTexts) {
				if (hitBox->contains(hitPosX, hitPosY)) {
					if (!itemDisabled) {
						if (_inp_leftMouseButtonPressed && _currentCursorObject != 0) {
							_inp_leftMouseButtonPressed = false;
							itemSelected = true;
							_flagsTable[119] = _currentCursorObject;
							if (_currentCursorObject == 1) {
								_flagsTable[118] = _currentAmountOfMoney;
								_currentAmountOfMoney = 0;
							}
							_inventoryItemsInfoTable[_currentCursorObject] = 0x20;
							setCursor(0);
							if (_giveItemToCounter == 0) {
								if (!restartKeyCharScriptOnAction(-53, _programHitBoxTable[i].item, 0)) {
									if (_flagsTable[119] == 1) {
										_currentAmountOfMoney = _flagsTable[118];
									} else {
										addItemToInventory(_currentKeyCharNum, _flagsTable[119]);
										drawInventory(_currentKeyCharNum, 1);
									}
									drawAmountOfMoneyInInventory();
								}
							} else {
								_flagsTable[117] = _programHitBoxTable[i].item - 1;
								_giveItemToCounter = -1;
							}
						}
						const char *strData = getString(str);
						int strPosY = mousePos.y - 22;
						if (_currentCursorObject != 0) {
							strPosY -= 8;
						}
						if (strPosY <= 0) {
							strPosY = 1;
						}
						int strWidth = getStringWidth(str);
						int strPosX = mousePos.x - strWidth / 2;
						strPosX = CLIP<int>(strPosX, 0, kScreenWidth - strWidth - 1);
						if (_talkTextSpeed != 0) {
							--_talkTextSpeed;
						}
						if (!stringDrawn && _talkTextSpeed == 0) {
							drawGameString(0xFF, strPosX + strWidth / 2, strPosY, strData);
						}
						stringDrawn = true;
						Common::Rect redrawRect(strPosX, strPosY, strPosX + strWidth, strPosY + kTextHeight);
						if (_programHitBoxTable[i].state & 0x8000) {
							redrawRect.extend(_programHitBoxTable[i].hitBoxes[1]);
						}
						addToDirtyRect(redrawRect);
						_programHitBoxTable[i].hitBoxes[1] = Common::Rect(strPosX, strPosY, strPosX + strWidth, strPosY + kTextHeight);
						_programHitBoxTable[i].state |= 0x8000;
					}
					if (_inp_leftMouseButtonPressed) {
						_inp_leftMouseButtonPressed = false;
						if (_currentCursorObject != 0) {
							setDefaultCursor(_currentKeyCharNum);
						} else {
							drawInventory(_currentKeyCharNum, 0);
							if (restartKeyCharScriptOnAction(-49, _programHitBoxTable[i].item, 0) == 0) {
								buildWalkPath(keyCharNewPosX, keyCharNewPosY, _currentKeyCharNum);
							}
						}
					} else {
						if (_inp_rightMouseButtonPressed && !itemDisabled && !itemSelected) {
							int act = handleActionMenuUnderCursor(_programHitBoxTable[i].actions, mousePos.x, mousePos.y, str);
							_inp_rightMouseButtonPressed = false;
							int16 facing = (keyCharNewPosX <= _keyCharsTable[_currentKeyCharNum].xPos) ? 3 : 0;
							_keyCharsTable[_currentKeyCharNum].facingDirection = facing;
							if (act != 0) {
								restartKeyCharScriptOnAction(act, _programHitBoxTable[i].item, 0);
							} else {
								act = _programHitBoxTable[i].talk;
								if (act != 0) {
									addToTalkTable(0, act, _currentKeyCharNum);
								}
							}
						}
					}
				} else if (_programHitBoxTable[i].state & 0x8000) {
					_programHitBoxTable[i].state &= 0x7FFF;
					addToDirtyRect(_programHitBoxTable[i].hitBoxes[1]);
				}
			}
		}
		if (_inp_leftMouseButtonPressed) {
			_inp_leftMouseButtonPressed = false;
			if (_currentCursorObject != 0) {
				if (_currentCursorObject != 1) {
					addItemToInventory(_currentKeyCharNum, _currentCursorObject);
					drawInventory(_objectDescriptionNum, 1);
				}
				setCursor(0);
			} else {
				drawInventory(_currentKeyCharNum, 0);
				buildWalkPath(keyCharNewPosX, keyCharNewPosY, _currentKeyCharNum);
			}
		}
	} else {
		if (flag == 0) {
			drawHitBoxes();
		}
	}
}

void ToucheEngine::handleMouseClickOnInventory(int flag) {
	if (flag == 0) {
		drawHitBoxes();
	}
	if (_hideInventoryTexts && _giveItemToCounter == 0) {
		if (!_conversationAreaCleared) {
			Common::Point mousePos = getMousePos();
			if (mousePos.x >= 40) {
				if (mousePos.y >= 328) {
					int replyNum = (mousePos.y - 328) / kTextHeight;
					if (replyNum >= 4) {
						replyNum = 3;
					}
					if (replyNum != _conversationReplyNum) {
						if (_conversationReplyNum != -1) {
							drawConversationString(_conversationReplyNum, 0xD6);
						}
						drawConversationString(replyNum, 0xFF);
						_conversationReplyNum = replyNum;
					}
					if (_inp_leftMouseButtonPressed) {
						_inp_leftMouseButtonPressed = false;
						setupConversationScript(replyNum);
						_conversationReplyNum = -1;
					}
				}
			} else {
				if (_conversationReplyNum != -1 && !_conversationAreaCleared) {
					drawConversationString(_conversationReplyNum, 0xD6);
				}
				_conversationReplyNum = -1;
				if (_inp_leftMouseButtonPressed) {
					int replyNum = mousePos.y - _roomAreaRect.height();
					if (replyNum < 40) {
						scrollUpConversationChoice();
					} else {
						scrollDownConversationChoice();
					}
					_inp_leftMouseButtonPressed = false;
				}
			}
		}
	} else if (_disabledInputCounter == 0 && !_hideInventoryTexts) {
		if (_inp_leftMouseButtonPressed) {
			handleLeftMouseButtonClickOnInventory();
			_inp_leftMouseButtonPressed = false;
		}
		if (_inp_rightMouseButtonPressed) {
			handleRightMouseButtonClickOnInventory();
			_inp_rightMouseButtonPressed = false;
		}
	}
}

void ToucheEngine::scrollScreenToPos(int num) {
	_screenOffset.x = _programPointsTable[num].x - kScreenWidth / 2;
	_screenOffset.y = _programPointsTable[num].y - kScreenHeight / 2;
}

void ToucheEngine::clearRoomArea() {
	int h = (_flagsTable[606] != 0) ? int(kScreenHeight) : _roomAreaRect.height();
	Graphics::fillRect(_offscreenBuffer, kScreenWidth, 0, 0, kScreenWidth, h, 0);
	updateEntireScreen();
}

void ToucheEngine::startNewMusic() {
	if (_newMusicNum != 0 && _newMusicNum != _currentMusicNum) {
		res_loadMusic(_newMusicNum);
		_currentMusicNum = _newMusicNum;
		_newMusicNum = 0;
	}
}

void ToucheEngine::startNewSound() {
	if (_newSoundNum != 0) {
		if (_newSoundDelay == 0) {
			res_loadSound(_newSoundPriority, _newSoundNum);
			_newSoundNum = 0;
		} else {
			--_newSoundDelay;
		}
	}
}

void ToucheEngine::updateSpeech() {
	if (_speechPlaying) {
		if (!_mixer->isSoundHandleActive(_speechHandle)) {
			_speechPlaying = false;
		}
	}
}

int ToucheEngine::handleActionMenuUnderCursor(const int16 *actions, int offs, int y, int str) {
	if (*actions == 0 || _menuRedrawCounter != 0) {
		return -26;
	}
	int i;
	int16 actionsTable[10];
	int16 *currentAction = actionsTable;
	int drawY = 0;
	for (i = 0; i < 8; ++i) {
		int act = *actions++;
		if (act == 0) {
			break;
		}
		if (act != -49 && act != -53) {
			*currentAction++ = act;
			drawY = 1;
		}
	}
	if (drawY == 0) {
		return -26;
	}
	*currentAction = 0;
	int strW = getStringWidth(str);
	int h = 0;
	for (i = 0; i < 10; ++i) {
		if (actionsTable[i] == 0) {
			break;
		}
		++h;
		drawY = getStringWidth(actionsTable[i]);
		if (drawY > strW) {
			strW = drawY;
		}
	}
	int cursorW = strW + 28;
	int cursorPosX = CLIP<int16>(offs - cursorW / 2, 0, kScreenWidth - cursorW);
	offs = cursorPosX + 14;
	h *= kTextHeight;
	int cursorH = h + 28;
	int cursorPosY = CLIP<int16>(y - 24, 0, kRoomHeight - cursorH);
	y = cursorPosY + 24;
	_cursorObjectRect = Common::Rect(cursorPosX, cursorPosY, cursorPosX + cursorW, cursorPosY + cursorH);
	addToDirtyRect(_cursorObjectRect);

	Graphics::fillRect(_offscreenBuffer, kScreenWidth, cursorPosX + 14, cursorPosY + 24, cursorW - 28, cursorH - 40, 0xF8);
	drawActionsPanel(cursorPosX, cursorPosY, cursorW, cursorH);

	const char *strData = getString(str);
	drawGameString(0xF8FF, offs + strW / 2, cursorPosY + 4, strData);
	for (i = 0; i < 10; ++i) {
		if (actionsTable[i] == 0) {
			break;
		}
		drawString(0xF8F9, offs, y + i * kTextHeight, actionsTable[i]);
	}
	updateScreenArea(cursorPosX, cursorPosY, cursorW, cursorH);

	_menuRedrawCounter = 2;
	Common::Rect rect(0, y, kScreenWidth, y + h);
	i = -1;
	while (_inp_rightMouseButtonPressed && !shouldQuit()) {
		Common::Point mousePos = getMousePos();
		if (rect.contains(mousePos)) {
			int c = (mousePos.y - y) / kTextHeight;
			if (c != i) {
				if (i >= 0) {
					drawY = y + i * kTextHeight;
					drawString(0xF8F9, offs, drawY, actionsTable[i]);
					updateScreenArea(offs, drawY, strW, kTextHeight);
				}
				i = c;
				drawY = y + i * kTextHeight;
				drawString(0xF8FF, offs, drawY, actionsTable[i]);
				updateScreenArea(offs, drawY, strW, kTextHeight);
			}
		} else if (i >= 0) {
			drawY = y + i * kTextHeight;
			drawString(0xF8F9, offs, drawY, actionsTable[i]);
			updateScreenArea(offs, drawY, strW, kTextHeight);
			i = -1;
		}
		processEvents(false);
		_system->updateScreen();
		_system->delayMillis(10);
	}

	const int action = (i >= 0) ? actionsTable[i] : -26;
	return action;
}

void ToucheEngine::redrawBackground() {
	for (uint i = 0; i < _programBackgroundTable.size(); ++i) {
		Area area = _programBackgroundTable[i].area;
		if (area.r.top != 20000) {
			area.r.translate(-_flagsTable[614], -_flagsTable[615]);
			if (_programBackgroundTable[i].type == 4) {
				int16 dx = _programBackgroundTable[i].offset - kScreenWidth / 2 - _flagsTable[614];
				dx *= _programBackgroundTable[i].scaleMul;
				dx /= _programBackgroundTable[i].scaleDiv;
				area.r.translate(dx, 0);
			}
			if (area.clip(_roomAreaRect)) {
				Graphics::copyRect(_offscreenBuffer, kScreenWidth, area.r.left, area.r.top,
				  _backdropBuffer, _currentBitmapWidth, area.srcX, area.srcY,
				  area.r.width(), area.r.height(),
				  Graphics::kTransparent);
				addToDirtyRect(area.r);
			}
		}
	}
}

void ToucheEngine::addRoomArea(int num, int flag) {
	debugC(9, kDebugEngine, "ToucheEngine::addRoomArea(%d, %d)", num, flag);
	if (_flagsTable[flag] == 20000) {
		Area area = _programBackgroundTable[num].area;
		area.r.translate(-_flagsTable[614], -_flagsTable[615]);
		addToDirtyRect(area.r);
	}
	_programBackgroundTable[num].area.r.moveTo(_flagsTable[flag], _flagsTable[flag + 1]);
}

void ToucheEngine::updateRoomAreas(int num, int flags) {
	debugC(9, kDebugEngine, "ToucheEngine::updateRoomAreas(%d, %d)", num, flags);
	if (flags != -1) {
		int16 count = _updatedRoomAreasTable[0];
		++_updatedRoomAreasTable[0];
		if (count == 199) {
			_updatedRoomAreasTable[0] = 2;
			count = 1;
		}
		_updatedRoomAreasTable[count] = (uint8)num;
	}
	for (uint i = 0; i < _programAreaTable.size(); ++i) {
		if (_programAreaTable[i].id == num) {
			Area area = _programAreaTable[i].area;
			if (i == 14 && _currentRoomNum == 8 && area.r.left == 715) {
				// Workaround for bug #1751170. area[14].r.left (update rect) should
				// be equal to area[7].r.left (redraw rect) but it's one off, which
				// leads to a glitch when that room area needs to be redrawn.
				area.r.left = 714;
			}
			Graphics::copyRect(_backdropBuffer, _currentBitmapWidth, area.r.left, area.r.top,
			  _backdropBuffer, _currentBitmapWidth, area.srcX, area.srcY,
			  area.r.width(), area.r.height(),
			  Graphics::kTransparent);
			if (flags != 0) {
				debug(0, "updateRoomAreas(num=%d index=%d)", num, i);
				redrawRoomRegion(i, true);
//				area.r.translate(-_flagsTable[614], -_flagsTable[615]);
//				addToDirtyRect(area.r);
			}
		}
	}
}

void ToucheEngine::setRoomAreaState(int num, uint16 state) {
	debugC(9, kDebugEngine, "ToucheEngine::setRoomAreaState(%d, %d)", num, state);
	for (uint i = 0; i < _programAreaTable.size(); ++i) {
		if (_programAreaTable[i].id == num) {
			_programAreaTable[i].state = state;
		}
	}
}

void ToucheEngine::findAndRedrawRoomRegion(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::findAndRedrawRoomRegion(%d)", num);
	for (uint i = 0; i < _programAreaTable.size(); ++i) {
		if (_programAreaTable[i].id == num) {
			redrawRoomRegion(i, false);
			break;
		}
	}
}

void ToucheEngine::updateRoomRegions() {
	debugC(9, kDebugEngine, "ToucheEngine::updateRoomRegions()");
	if (_flagsTable[269] == 0) {
		uint i = 0;
		while (i < _programAreaTable.size() && _programAreaTable[i].id != 0) {
			switch (_programAreaTable[i].state) {
			case 0:
				++i;
				break;
			case 1:
				redrawRoomRegion(i + _programAreaTable[i].animNext, true);
				++_programAreaTable[i].animNext;
				if (_programAreaTable[i].animNext >= _programAreaTable[i].animCount) {
					_programAreaTable[i].animNext = 0;
				}
				i += _programAreaTable[i].animCount;
				break;
			case 3:
				redrawRoomRegion(i + _programAreaTable[i].animNext, true);
				++_programAreaTable[i].animNext;
				if (_programAreaTable[i].animNext >= _programAreaTable[i].animCount) {
					_programAreaTable[i].animNext = 0;
				}
				i += _programAreaTable[i].animCount + 1;
				break;
			}
		}
	}
}

void ToucheEngine::redrawRoomRegion(int num, bool markForRedraw) {
	debugC(9, kDebugEngine, "ToucheEngine::redrawRoomRegion(%d)", num);
	Area area = _programAreaTable[num].area;
	area.r.translate(-_flagsTable[614], -_flagsTable[615]);
	if (area.clip(_roomAreaRect)) {
		Graphics::copyRect(_offscreenBuffer, kScreenWidth, area.r.left, area.r.top,
		  _backdropBuffer, _currentBitmapWidth, area.srcX, area.srcY,
		  area.r.width(), area.r.height(),
		  Graphics::kTransparent);
		if (markForRedraw) {
			addToDirtyRect(area.r);
		}
	}
}

void ToucheEngine::initInventoryObjectsTable() {
	for (int i = 0; i < NUM_INVENTORY_ITEMS; ++i) {
		_inventoryItemsInfoTable[i] = 0x20;
	}
}

void ToucheEngine::initInventoryLists() {
	memset(_inventoryList1, 0, sizeof(_inventoryList1));
	_inventoryList1[100] = -1;
	_inventoryStateTable[0].displayOffset = 0;
	_inventoryStateTable[0].lastItem = 100;
	_inventoryStateTable[0].itemsPerLine = 6;
	_inventoryStateTable[0].itemsList = _inventoryList1;

	memset(_inventoryList2, 0, sizeof(_inventoryList2));
	_inventoryList2[100] = -1;
	_inventoryStateTable[1].displayOffset = 0;
	_inventoryStateTable[1].lastItem = 100;
	_inventoryStateTable[1].itemsPerLine = 6;
	_inventoryStateTable[1].itemsList = _inventoryList2;

	memset(_inventoryList3, 0, sizeof(_inventoryList3));
	_inventoryList3[6] = -1;
	_inventoryStateTable[2].displayOffset = 0;
	_inventoryStateTable[2].lastItem = 6;
	_inventoryStateTable[2].itemsPerLine = 6;
	_inventoryStateTable[2].itemsList = _inventoryList3;
}

void ToucheEngine::setupInventoryAreas() {
	_inventoryAreasTable[kInventoryCharacter]    = Common::Rect(  0, 354,  50, 400);
	_inventoryAreasTable[kInventoryMoneyDisplay] = Common::Rect( 66, 354, 124, 380);
	_inventoryAreasTable[kInventoryGoldCoins]    = Common::Rect( 74, 380, 116, 398);
	_inventoryAreasTable[kInventorySilverCoins]  = Common::Rect(116, 380, 158, 398);
	_inventoryAreasTable[kInventoryMoney]        = Common::Rect(144, 354, 198, 380);
	_inventoryAreasTable[kInventoryScroller1]    = Common::Rect(202, 354, 238, 396);
	_inventoryAreasTable[kInventoryObject1]      = Common::Rect(242, 354, 300, 396);
	_inventoryAreasTable[kInventoryObject2]      = Common::Rect(300, 354, 358, 396);
	_inventoryAreasTable[kInventoryObject3]      = Common::Rect(358, 354, 416, 396);
	_inventoryAreasTable[kInventoryObject4]      = Common::Rect(416, 354, 474, 396);
	_inventoryAreasTable[kInventoryObject5]      = Common::Rect(474, 354, 532, 396);
	_inventoryAreasTable[kInventoryObject6]      = Common::Rect(532, 354, 590, 396);
	_inventoryAreasTable[kInventoryScroller2]    = Common::Rect(594, 354, 640, 395);
}

void ToucheEngine::drawInventory(int index, int flag) {
	if (_flagsTable[606] == 0) {
		if (index > 1) {
			index = 1;
		}
		if (_objectDescriptionNum == index && flag == 0) {
			return;
		}
		_inventoryVar1 = _inventoryStateTable[index].itemsList;
		_inventoryVar2 = &_inventoryStateTable[index].displayOffset;
		_objectDescriptionNum = index;
		uint8 *dst = _offscreenBuffer + kScreenWidth * kRoomHeight;
		res_loadSpriteImage(index + 12, dst);
		res_loadImageHelper(dst, _currentImageWidth, _currentImageHeight);
		int firstObjNum = _inventoryVar2[0];
		for (int i = 0, x = 242; i < 6; ++i, x += 58) {
			int num = _inventoryVar1[firstObjNum + i];
			if (num == -1) {
				break;
			}
			if (num != 0) {
				drawIcon(x + 3, 353, num);
			}
		}
		drawAmountOfMoneyInInventory();
		updateScreenArea(0, kRoomHeight, kScreenWidth, kScreenHeight - kRoomHeight);
	}
}

void ToucheEngine::drawAmountOfMoneyInInventory() {
	if (_flagsTable[606] == 0 && !_hideInventoryTexts) {
		char text[10];
		sprintf(text, "%d", _keyCharsTable[0].money);
		Graphics::fillRect(_offscreenBuffer, kScreenWidth, 74, 354, 40, 16, 0xD2);
		drawGameString(217, 94, 355, text);
		updateScreenArea(74, 354, 40, 16);
		Graphics::fillRect(_offscreenBuffer, kScreenWidth, 150, 353, 40, 41, 0xD2);
		if (_currentAmountOfMoney != 0) {
			drawIcon(141, 348, 1);
			sprintf(text, "%d", _currentAmountOfMoney);
			drawGameString(217, 170, 378, text);
		}
		updateScreenArea(150, 353, 40, 41);
	}
}

void ToucheEngine::packInventoryItems(int index) {
	int16 *p = _inventoryStateTable[index].itemsList;
	for (int i = 0; *p != -1; ++i, ++p) {
		if (p[0] == 0 && p[1] != -1) {
			p[0] = p[1];
			p[1] = 0;
		}
	}
}

void ToucheEngine::appendItemToInventoryList(int index) {
	int last = _inventoryStateTable[index].lastItem - 1;
	int16 *p = _inventoryStateTable[index].itemsList;
	if (p[last] != 0) {
		warning("Inventory %d Full", index);
	} else {
		for (int i = last; i > 0; --i) {
			p[i] = p[i - 1];
		}
		*p = 0;
	}
}

void ToucheEngine::addItemToInventory(int inventory, int16 item) {
	if (item == 0) {
		packInventoryItems(inventory);
	} else if (item == 1)  {
		_currentAmountOfMoney += _flagsTable[118];
		drawAmountOfMoneyInInventory();
	} else {
		appendItemToInventoryList(inventory);
		assert(inventory >= 0 && inventory < 3);
		int16 *p = _inventoryStateTable[inventory].itemsList;
		for (int i = 0; *p != -1; ++i, ++p) {
			if (*p == 0) {
				*p = item;
				_inventoryItemsInfoTable[item] = inventory | 0x10;
				packInventoryItems(0);
				packInventoryItems(1);
				break;
			}
		}
	}
}

void ToucheEngine::removeItemFromInventory(int inventory, int16 item) {
	if (item == 1) {
		_currentAmountOfMoney = 0;
		drawAmountOfMoneyInInventory();
	} else {
		assert(inventory >= 0 && inventory < 3);
		int16 *p = _inventoryStateTable[inventory].itemsList;
		for (int i = 0; *p != -1; ++i, ++p) {
			if (*p == item) {
				*p = 0;
				packInventoryItems(0);
				packInventoryItems(1);
				break;
			}
		}
	}
}

void ToucheEngine::resetTalkingVars() {
	_talkListCurrent = 0;
	_talkListEnd = 0;
	_keyCharTalkCounter = 0;
	_talkTextRectDefined = false;
	_talkTextDisplayed = false;
	_skipTalkText = false;
	_talkTextInitialized = false;
	if (_speechPlaying) {
		res_stopSpeech();
	}
}

int ToucheEngine::updateKeyCharTalk(int skipFlag) {
	if (skipFlag != 0) {
		if (_speechPlaying) {
			res_stopSpeech();
		}
		if (_talkListEnd != _talkListCurrent) {
			_keyCharTalkCounter = 0;
			_talkTextInitialized = false;
			if (skipFlag == 2) {
				_skipTalkText = true;
			} else {
				_skipTalkText = false;
			}
		}
		return 0;
	}
	if (_talkListEnd == _talkListCurrent) {
		return 0;
	}
	int talkingKeyChar = _talkTable[_talkListCurrent].talkingKeyChar;
	int otherKeyChar = _talkTable[_talkListCurrent].otherKeyChar;
	KeyChar *key = &_keyCharsTable[talkingKeyChar];
	int x = key->xPos - _flagsTable[614];
	int y = key->yPos - _flagsTable[615] - (key->zPos / 2 + 16);
	int stringNum = _talkTable[_talkListCurrent].num;
	const char *stringData = getString(stringNum);
	int textWidth = getStringWidth(stringNum);
	if (!_talkTextInitialized && !_skipTalkText) {
		_keyCharTalkCounter = textWidth / 32 + 20;
		setKeyCharTalkingFrame(talkingKeyChar);
		res_loadSpeechSegment(stringNum);
		_talkTextInitialized = true;
	}
	if (_keyCharTalkCounter) {
		--_keyCharTalkCounter;
	}
	_currentObjectNum = talkingKeyChar;
	if (_speechPlaying) {
		_flagsTable[297] = 0;
		_keyCharTalkCounter = 1;
		if (_talkTextMode == kTalkModeVoiceOnly) {
			return 1;
		}
	}
	if (_keyCharTalkCounter != 0) {
		_talkTextDisplayed = true;
		int textHeight = kTextHeight;
		y -= kTextHeight;
		if (y < 0) {
			y = 1;
		} else if (y > kRoomHeight) {
			y = kRoomHeight - 16;
		}
		if (textWidth > 200) {
			textWidth = 200;
			stringData = formatTalkText(&y, &textHeight, stringData);
		}
		x -= textWidth / 2;
		if (x < 0) {
			x = 0;
		}
		if (x + textWidth >= kScreenWidth) {
			x = kScreenWidth - textWidth - 1;
		}
		drawGameString(key->textColor, x + textWidth / 2, y, stringData);
		_talkTextSpeed = 6;
		_talkTextRect = Common::Rect(x, y, x + textWidth, y + textHeight);
		if (_talkTextRectDefined) {
			_talkTextRect.extend(_talkTextRect2);
		}
		addToDirtyRect(_talkTextRect);
		_talkTextRect2 = Common::Rect(x, y, x + textWidth, y + textHeight);
		_talkTextRectDefined = true;
		_flagsTable[297] = 0;
	} else {
		updateTalkFrames(_currentObjectNum);
		_currentObjectNum = -1;
		if (_talkTextDisplayed) {
			addToDirtyRect(_talkTextRect2);
		}
		_talkTextInitialized = false;
		_skipTalkText = false;
		_talkTextRectDefined = false;
		++_talkListCurrent;
		if (_talkListCurrent == 16) {
			_talkListCurrent = 0;
		}
		if (otherKeyChar != -1) {
			_keyCharsTable[otherKeyChar].flags &= ~kScriptPaused;
		}
	}
	return 1;
}

const char *ToucheEngine::formatTalkText(int *y, int *h, const char *text) {
	static char talkTextBuffer[200];
	int newLineWidth = 0;
	int lineWidth = 0;
	char *textBuffer = talkTextBuffer;
	char *textLine = textBuffer;
	while (*text) {
		char chr = *text++;
		int chrWidth = Graphics::getCharWidth16(chr);
		lineWidth += chrWidth;
		if (chr == ' ') {
			if (lineWidth + newLineWidth >= 200) {
				*textLine = '\\';
				newLineWidth = lineWidth - chrWidth;
				*y -= kTextHeight;
				*h += kTextHeight;
				lineWidth = chrWidth;
			} else {
				newLineWidth += lineWidth;
				lineWidth = chrWidth;
			}
			*textBuffer = ' ';
			textLine = textBuffer;
			textBuffer++;
		} else {
			*textBuffer++ = chr;
		}
	}
	if (newLineWidth + lineWidth >= 200) {
		*textLine = '\\';
		*y -= kTextHeight;
		*h += kTextHeight;
	}
	*textBuffer = '\0';
	if (*y < 0) {
		*y = 1;
	}
	return talkTextBuffer;
}

void ToucheEngine::addToTalkTable(int talkingKeyChar, int num, int otherKeyChar) {
	if (_talkListEnd != _talkListCurrent) {
		if (_talkTableLastTalkingKeyChar == talkingKeyChar &&
			_talkTableLastOtherKeyChar == otherKeyChar &&
			_talkTableLastStringNum == num) {
			return;
		}
	}
	_talkTableLastTalkingKeyChar = talkingKeyChar;
	_talkTableLastOtherKeyChar = otherKeyChar;
	_talkTableLastStringNum = num;

	removeFromTalkTable(otherKeyChar);

	assert(_talkListEnd < NUM_TALK_ENTRIES);
	TalkEntry *talkEntry = &_talkTable[_talkListEnd];
	talkEntry->talkingKeyChar = talkingKeyChar;
	talkEntry->otherKeyChar = otherKeyChar;
	talkEntry->num = num;

	++_talkListEnd;
	if (_talkListEnd == NUM_TALK_ENTRIES) {
		_talkListEnd = 0;
	}
}

void ToucheEngine::removeFromTalkTable(int keyChar) {
	debugC(9, kDebugEngine, "ToucheEngine::removeFromTalkTable(%d)", keyChar);
	int i = _talkListCurrent;
	while (i != _talkListEnd) {
		if (_talkTable[i].otherKeyChar == keyChar) {
			_talkTable[i].otherKeyChar = -1;
		}
		++i;
		i %= NUM_TALK_ENTRIES;
	}
}

void ToucheEngine::addConversationChoice(int16 num) {
	debugC(9, kDebugEngine, "ToucheEngine::addConversationChoice(%d)", num);
	_conversationChoicesUpdated = true;
	int16 msg = _programConversationTable[_currentConversation + num].msg;
	for (int i = 0; i < NUM_CONVERSATION_CHOICES; ++i) {
		if (_conversationChoicesTable[i].msg == msg) {
			break;
		}
		if (_conversationChoicesTable[i].msg == 0) {
			_conversationChoicesTable[i].msg = msg;
			_conversationChoicesTable[i].num = num;
			break;
		}
	}
}

void ToucheEngine::removeConversationChoice(int16 num) {
	debugC(9, kDebugEngine, "ToucheEngine::removeConversationChoice(%d)", num);
	for (int i = 0; i < NUM_CONVERSATION_CHOICES; ++i) {
		if (_conversationChoicesTable[i].num == num) {
			_conversationChoicesUpdated = true;
			for (; i < NUM_CONVERSATION_CHOICES - 1; ++i) {
				_conversationChoicesTable[i].num = _conversationChoicesTable[i + 1].num;
				_conversationChoicesTable[i].msg = _conversationChoicesTable[i + 1].msg;
			}
			break;
		}
	}
}

void ToucheEngine::runConversationScript(uint16 offset) {
	debugC(9, kDebugEngine, "ToucheEngine::runConversationScript() offset=0x%X", offset);
	_script.dataOffset = offset;
	_script.quitFlag = 0;
	runCurrentKeyCharScript(2);
}

void ToucheEngine::findConversationByNum(int16 num) {
	debugC(9, kDebugEngine, "ToucheEngine::findConversationByNum(%d)", num);
	for (uint i = 0; i < _programConversationTable.size(); ++i) {
		if (_programConversationTable[i].num == num) {
			clearConversationChoices();
			_currentConversation = i;
			runConversationScript(_programConversationTable[i].offset);
			break;
		}
	}
}

void ToucheEngine::clearConversationChoices() {
	debugC(9, kDebugEngine, "ToucheEngine::clearConversationChoices()");
	_conversationChoicesUpdated = true;
	for (int i = 0; i < NUM_CONVERSATION_CHOICES; ++i) {
		_conversationChoicesTable[i].num = 0;
		_conversationChoicesTable[i].msg = 0;
	}
	_scrollConversationChoiceOffset = 0;
}

void ToucheEngine::scrollDownConversationChoice() {
	if (_conversationChoicesTable[4 + _scrollConversationChoiceOffset].msg != 0) {
		++_scrollConversationChoiceOffset;
		drawCharacterConversation();
	}
}

void ToucheEngine::scrollUpConversationChoice() {
	if (_scrollConversationChoiceOffset != 0) {
		--_scrollConversationChoiceOffset;
		drawCharacterConversation();
	}
}

void ToucheEngine::drawCharacterConversation() {
	_conversationChoicesUpdated = false;
	if (!_disableConversationScript) {
		if (_conversationChoicesTable[0].msg == 0) {
			_conversationEnded = true;
			return;
		}
		if (_conversationChoicesTable[1].msg == 0) {
			setupConversationScript(0);
			return;
		}
	}
	drawConversationPanel();
	for (int i = 0; i < 4; ++i) {
		drawString(214, 42, 328 + i * kTextHeight, _conversationChoicesTable[_scrollConversationChoiceOffset + i].msg, kStringTypeConversation);
	}
	updateScreenArea(0, 320, kScreenWidth, kScreenHeight - 320);
	_conversationAreaCleared = false;
}

void ToucheEngine::drawConversationString(int num, uint16 color) {
	const int y = 328 + num * kTextHeight;
	drawString(color, 42, y, _conversationChoicesTable[num + _scrollConversationChoiceOffset].msg, kStringTypeConversation);
	updateScreenArea(0, y, kScreenWidth, kTextHeight);
}

void ToucheEngine::clearConversationArea() {
	drawConversationPanel();
	updateScreenArea(0, 320, kScreenWidth, kScreenHeight - 320);
	_conversationAreaCleared = true;
}

void ToucheEngine::setupConversationScript(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::setupConversationScript(%d)", num);
	if (num < 5 && _conversationChoicesTable[num].msg != 0) {
		num = _conversationChoicesTable[_scrollConversationChoiceOffset + num].num;
		KeyChar *key = &_keyCharsTable[_currentKeyCharNum];
		key->scriptDataOffset = _programConversationTable[_currentConversation + num].offset;
		key->scriptStackPtr = &key->scriptStackTable[39];
		_scrollConversationChoiceOffset = 0;
		removeConversationChoice(num);
		clearConversationArea();
	}
}

void ToucheEngine::handleConversation() {
	if (_conversationNum != 0) {
		findConversationByNum(_conversationNum);
		_conversationAreaCleared = false;
		drawCharacterConversation();
		_roomAreaRect.setHeight(320);
		_hideInventoryTexts = true;
		_conversationEnded = false;
		_conversationNum = 0;
	} else if (_hideInventoryTexts && _conversationAreaCleared) {
		if (_keyCharsTable[_currentKeyCharNum].scriptDataOffset == 0) {
			drawCharacterConversation();
		}
	} else if (!_conversationAreaCleared && _conversationChoicesUpdated) {
		drawCharacterConversation();
	}
}

static int getDirection(int x1, int y1, int z1, int x2, int y2, int z2) {
	int ret = -1;
	x2 -= x1;
	y2 -= y1;
	z2 -= z1;
	if (x2 == 0 && y2 == 0 && z2 == 0) {
		ret = -2;
	} else {
		if (ABS(x2) >= ABS(z2)) {
			if (ABS(x2) > ABS(y2)) {
				if (x2 > 0) {
					ret = 0;
				} else {
					ret = 3;
				}
			} else {
				if (y2 > 0) {
					ret = 1;
				} else {
					ret = 2;
				}
			}
		} else {
			if (z2 != 0) {
				if (z2 > 0) {
					ret = 1;
				} else {
					ret = 2;
				}
			} else {
				if (y2 > 0) {
					ret = 1;
				} else {
					ret = 2;
				}
			}
		}
	}
	return ret;
}

void ToucheEngine::buildWalkPointsList(int keyChar) {
	debugC(9, kDebugEngine, "ToucheEngine::buildWalkPointsList(%d)", keyChar);
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	uint16 curPos, pos1, pos2;
	if (key->pointsDataNum & 0x8000) {
		const ProgramWalkData *pwd = &_programWalkTable[(key->pointsDataNum & 0x7FFF)];
		if (_programPointsTable[pwd->point1].order < _programPointsTable[pwd->point2].order) {
			curPos = pwd->point1;
		} else {
			curPos = pwd->point2;
		}
	} else {
		curPos = key->pointsDataNum;
	}

	int16 posNum = _programPointsTable[curPos].order;
	if (posNum == 32000) {
		return;
	}
	key->walkPointsList[0] = curPos;
	int16 walkPointsCount = 1;
	do {
		for (uint i = 0; i < _programWalkTable.size(); ++i) {
			if ((_programWalkTable[i].point1 & 0x4000) == 0) {
				pos1 = _programWalkTable[i].point1;
				pos2 = _programWalkTable[i].point2;
				if (pos1 == curPos && posNum > _programPointsTable[pos2].order) {
					curPos = pos2;
					assert(walkPointsCount < 40);
					key->walkPointsList[walkPointsCount] = curPos;
					++walkPointsCount;
					posNum = _programPointsTable[pos2].order;
					break;
				}
				if (pos2 == curPos && posNum > _programPointsTable[pos1].order) {
					curPos = pos1;
					assert(walkPointsCount < 40);
					key->walkPointsList[walkPointsCount] = curPos;
					++walkPointsCount;
					posNum = _programPointsTable[pos1].order;
					break;
				}
			}
		}
	} while (_programPointsTable[curPos].order != 0);
	assert(walkPointsCount < 40);
	key->walkPointsList[walkPointsCount] = -1;

	key->xPosPrev = _programPointsTable[curPos].x;
	key->yPosPrev = _programPointsTable[curPos].y;
	key->zPosPrev = _programPointsTable[curPos].z;
	key->prevWalkDataNum = findWalkDataNum(curPos, -1);
	key->walkPointsListIndex = 0;
	if (key->walkDataNum == -1) {
		return;
	}

	pos1 = _programWalkTable[key->walkDataNum].point1;
	pos2 = _programWalkTable[key->walkDataNum].point2;
	if (key->pointsDataNum == pos1) {
		if (key->walkPointsList[1] == pos2) {
			++key->walkPointsListIndex;
		}
		return;
	}
	if (key->pointsDataNum == pos2) {
		if (key->walkPointsList[1] == pos1) {
			++key->walkPointsListIndex;
		}
		return;
	}
}

int ToucheEngine::findWalkDataNum(int pointNum1, int pointNum2) {
	debugC(9, kDebugEngine, "ToucheEngine::findWalkDataNum(%d, %d)", pointNum1, pointNum2);
	if (pointNum1 != pointNum2) {
		for (uint i = 0; i < _programWalkTable.size(); ++i) {
			int p1 = _programWalkTable[i].point1 & 0xFFF;
			int p2 = _programWalkTable[i].point2 & 0xFFF;
			if (p1 == pointNum1) {
				if (p2 == pointNum2 || pointNum2 == 10000) {
					return i;
				}
			} else if (p2 == pointNum1) {
				if (p1 == pointNum2 || pointNum2 == 10000) {
					return i;
				}
			}
		}
	}
	return -1;
}

void ToucheEngine::changeWalkPath(int num1, int num2, int16 val) {
	debugC(9, kDebugEngine, "ToucheEngine::changeWalkPath(%d, %d)", num1, num2);
	int num = findWalkDataNum(num1, num2);
	if (num != -1) {
		_programWalkTable[num].area1 = val;
	}
}

void ToucheEngine::adjustKeyCharPosToWalkBox(KeyChar *key, int moveType) {
	const ProgramWalkData *pwd = &_programWalkTable[key->walkDataNum];

	const ProgramPointData *pts1 = &_programPointsTable[pwd->point1];
	int16 x1 = pts1->x;
	int16 y1 = pts1->y;
	int16 z1 = pts1->z;

	const ProgramPointData *pts2 = &_programPointsTable[pwd->point2];
	int16 x2 = pts2->x;
	int16 y2 = pts2->y;
	int16 z2 = pts2->z;

	int16 kx = key->xPos;
	int16 ky = key->yPos;
	int16 kz = key->zPos;

	int16 dx = x2 - x1;
	int16 dy = y2 - y1;
	int16 dz = z2 - z1;

	switch (moveType) {
	case 0:
		kx -= x1;
		if (dx != 0) {
			key->yPos = dy * kx / dx + y1;
			key->zPos = dz * kx / dx + z1;
		}
		break;
	case 1:
		ky -= y1;
		if (dy != 0) {
			key->xPos = dx * ky / dy + x1;
			key->zPos = dz * ky / dy + z1;
		}
		break;
	case 2:
		kz -= z1;
		if (dz != 0) {
			key->xPos = dx * kz / dz + x1;
			key->yPos = dy * kz / dz + y1;
		}
		break;
	}
}

void ToucheEngine::lockWalkPath(int num1, int num2) {
	debugC(9, kDebugEngine, "ToucheEngine::lockWalkPath(%d, %d)", num1, num2);
	const int num = findWalkDataNum(num1, num2);
	if (num != -1) {
		_programWalkTable[num].point1 |= 0x4000;
		_programWalkTable[num].point2 |= 0x4000;
	}
}

void ToucheEngine::unlockWalkPath(int num1, int num2) {
	debugC(9, kDebugEngine, "ToucheEngine::unlockWalkPath(%d, %d)", num1, num2);
	const int num = findWalkDataNum(num1, num2);
	if (num != -1) {
		_programWalkTable[num].point1 &= 0xFFF;
		_programWalkTable[num].point2 &= 0xFFF;
	}
}

void ToucheEngine::resetPointsData(int num) {
	debugC(9, kDebugEngine, "ToucheEngine::resetPointsData(%d)", num);
	for (uint i = 1; i < _programPointsTable.size(); ++i) {
		_programPointsTable[i].order = num;
	}
}

bool ToucheEngine::sortPointsData(int num1, int num2) {
	debugC(9, kDebugEngine, "ToucheEngine::sortPointsData(%d, %d)", num1, num2);
	resetPointsData(32000);
	if (num1 == -1) {
		if (num2 == -1) {
			return false;
		}
		_programPointsTable[num2].order = 0;
	} else {
		const int md1 = _programWalkTable[num1].point1;
		_programPointsTable[md1].order = 0;
		const int md2 = _programWalkTable[num1].point2;
		_programPointsTable[md2].order = 0;
	}
	bool quitLoop = false;
	int order = 1;
	while (!quitLoop) {
		quitLoop = true;
		for (uint i = 0; i < _programWalkTable.size(); ++i) {
			const int md1 = _programWalkTable[i].point1;
			const int md2 = _programWalkTable[i].point2;
			if ((md1 & 0x4000) == 0) {
				assert((md2 & 0x4000) == 0);
				if (_programPointsTable[md1].order == order - 1 && _programPointsTable[md2].order > order) {
					_programPointsTable[md2].order = order;
					quitLoop = false;
				}
				if (_programPointsTable[md2].order == order - 1 && _programPointsTable[md1].order > order) {
					_programPointsTable[md1].order = order;
					quitLoop = false;
				}
			}
		}
		++order;
	}
	return true;
}

void ToucheEngine::updateKeyCharWalkPath(KeyChar *key, int16 dx, int16 dy, int16 dz) {
	debugC(9, kDebugEngine, "ToucheEngine::updateKeyCharWalkPath(key=%d, dx=%d, dy=%d, dz=%d)", (int)(key - _keyCharsTable), dx, dy, dz);
	if (key->walkDataNum == -1) {
		return;
	}
	int16 kx = key->xPos;
	int16 ky = key->yPos;
	int16 kz = key->zPos;
	if (kz != 160) {
		if (dx != 0) {
			dx = dx * kz / 160;
			if (dx == 0) {
				dx = 1;
			}
		}
		if (dy != 0) {
			dy = dy * kz / 160;
			if (dy == 0) {
				dy = 1;
			}
		}
		if (dz != 0) {
			dz = dz * kz / 160;
			if (dz == 0) {
				dz = 1;
			}
		}
	}

	int16 curDirection = key->facingDirection;
	if (key->currentAnim > 1) {
		if (dx != 0 || dy != 0 || dz != 0) {
			if (curDirection == 3) {
				key->xPos -= dx;
			} else {
				key->xPos += dx;
			}
			key->xPosPrev = key->xPos;
		}
		return;
	}

	int16 xpos, ypos, zpos, walkPoint1, walkPoint2, newDirection, incDx, incDy, incDz;
	while (1) {
		walkPoint1 = key->walkPointsList[key->walkPointsListIndex];
		walkPoint2 = key->walkPointsList[key->walkPointsListIndex + 1];
		key->currentWalkBox = walkPoint1;
		if (walkPoint1 == -1) {
			xpos = key->xPosPrev;
			ypos = key->yPosPrev;
			zpos = key->zPosPrev;
			if (key->prevWalkDataNum != -1) {
				key->walkDataNum = key->prevWalkDataNum;
				key->prevWalkDataNum = -1;
			}
		} else {
			xpos = _programPointsTable[walkPoint1].x;
			ypos = _programPointsTable[walkPoint1].y;
			zpos = _programPointsTable[walkPoint1].z;
		}
		newDirection = getDirection(kx, ky, kz, xpos, ypos, zpos);
		if (newDirection < 0) {
			newDirection = curDirection;
		}
		if (newDirection != curDirection) {
			key->currentAnimCounter = 0;
			key->facingDirection = newDirection;
			return;
		}
		incDx = xpos - kx;
		incDy = ypos - ky;
		incDz = zpos - kz;
		if (incDz != 0 || incDy != 0 || incDx != 0) {
			break;
		}
		if (walkPoint1 == -1) {
			if (key->currentAnim == 1) {
				setKeyCharRandomFrame(key);
			}
			return;
		}
		key->prevPointsDataNum = key->pointsDataNum;
		key->pointsDataNum = walkPoint1;
		if (walkPoint2 == -1) {
			key->walkPointsList[0] = -1;
			key->walkPointsListIndex = 0;
		} else {
			++key->walkPointsListIndex;
			int16 walkDataNum = findWalkDataNum(walkPoint1, walkPoint2);
			if (walkDataNum != -1) {
				key->walkDataNum = walkDataNum;
			}
		}
	}

	if (key->currentAnim < 1) {
		key->currentAnimCounter = 0;
		key->currentAnim = 1;
		if (dx == 0 && dy == 0 && dz == 0) {
			return;
		}
	}

	switch (newDirection) {
	case 0:
	case 3:
		if (dx == 0) {
			return;
		}
		if (newDirection == 3) {
			dx = -dx;
		}
		if (ABS(dx) >= ABS(incDx)) {
			if (walkPoint1 != -1) {
				if (walkPoint2 == -1) {
					newDirection = getDirection(xpos, ypos, zpos, key->xPosPrev, key->yPosPrev, key->zPosPrev);
					if (key->prevWalkDataNum != -1) {
						key->walkDataNum = key->prevWalkDataNum;
						key->prevWalkDataNum = -1;
					}
				} else {
					newDirection = getDirection(xpos, ypos, zpos, _programPointsTable[walkPoint2].x, _programPointsTable[walkPoint2].y, _programPointsTable[walkPoint2].z);
					int16 walkDataNum = findWalkDataNum(walkPoint1, walkPoint2);
					if (walkDataNum != -1) {
						key->walkDataNum = walkDataNum;
					}
				}
				if (newDirection == -2) {
					key->xPos = xpos;
					key->yPos = ypos;
					key->zPos = zpos;
					setKeyCharRandomFrame(key);
					return;
				}
				if (newDirection < 0) {
					newDirection = curDirection;
				}
				key->prevPointsDataNum = key->pointsDataNum;
				key->pointsDataNum = walkPoint1;
				++key->walkPointsListIndex;
				if (newDirection != curDirection) {
					key->facingDirection = newDirection;
					key->currentAnimCounter = 0;
					key->xPos = xpos;
					key->yPos = ypos;
					key->zPos = zpos;
					return;
				}
			} else {
				key->xPos = xpos;
				key->yPos = ypos;
				key->zPos = zpos;
				return;
			}
		}
		key->xPos += dx;
		adjustKeyCharPosToWalkBox(key, 0);
		break;
	case 1:
	case 2:
		if (ABS(dz) >= ABS(incDz) && incDz != 0) {
			if (walkPoint1 != -1) {
				if (walkPoint2 == -1) {
					newDirection = getDirection(xpos, ypos, zpos, key->xPosPrev, key->yPosPrev, key->zPosPrev);
				} else {
					newDirection = getDirection(xpos, ypos, zpos, _programPointsTable[walkPoint2].x, _programPointsTable[walkPoint2].y, _programPointsTable[walkPoint2].z);
					int16 walkDataNum = findWalkDataNum(walkPoint1, walkPoint2);
					if (walkDataNum != -1) {
						key->walkDataNum = walkDataNum;
					}
				}
				if (newDirection == -2) {
					key->xPos = xpos;
					key->yPos = ypos;
					key->zPos = zpos;
					setKeyCharRandomFrame(key);
					return;
				}
				if (newDirection < 0) {
					newDirection = curDirection;
				}
				key->prevPointsDataNum = key->pointsDataNum;
				key->pointsDataNum = walkPoint1;
				++key->walkPointsListIndex;
				if (newDirection != curDirection) {
					key->facingDirection = newDirection;
					key->currentAnimCounter = 0;
					key->xPos = xpos;
					key->yPos = ypos;
					key->zPos = zpos;
					return;
				}
			} else {
				key->xPos = xpos;
				key->yPos = ypos;
				key->zPos = zpos;
				return;
			}
		}
		if (incDz != 0) {
			key->zPos += dz;
			adjustKeyCharPosToWalkBox(key, 2);
		} else {
			if (ABS(dz) < ABS(incDy)) {
				key->yPos += dz;
				adjustKeyCharPosToWalkBox(key, 1);
			} else {
				key->xPos = xpos;
				key->yPos = ypos;
				key->zPos = zpos;
			}
		}
		break;
	}
}

void ToucheEngine::markWalkPoints(int keyChar) {
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	int16 pointsDataNum = key->pointsDataNum;
	resetPointsData(0);
	if (pointsDataNum != -1) {
		_programPointsTable[pointsDataNum].order = 1;
		bool quitLoop = false;
		while (!quitLoop) {
			quitLoop = true;
			for (uint i = 0; i < _programWalkTable.size(); ++i) {
				int16 md1 = _programWalkTable[i].point1;
				int16 md2 = _programWalkTable[i].point2;
				if ((md1 & 0x4000) == 0) {
					assert((md2 & 0x4000) == 0);
					if (_programPointsTable[md1].order != 0 && _programPointsTable[md2].order == 0) {
						_programPointsTable[md2].order = 1;
						quitLoop = false;
					}
					if (_programPointsTable[md2].order != 0 && _programPointsTable[md1].order == 0) {
						_programPointsTable[md1].order = 1;
						quitLoop = false;
					}
				}
			}
		}
	}
}

void ToucheEngine::buildWalkPath(int dstPosX, int dstPosY, int keyChar) {
	debugC(9, kDebugEngine, "ToucheEngine::buildWalkPath(x=%d, y=%d, key=%d)", dstPosX, dstPosY, keyChar);
	if (_currentEpisodeNum == 130) {
		return;
	}
	markWalkPoints(keyChar);

	int minDistance = 0x7D000000;
	int minPointsDataNum = -1;
	for (uint i = 1; i < _programPointsTable.size(); ++i) {
		if (_programPointsTable[i].order != 0) {
			int dx = _programPointsTable[i].x - dstPosX;
			int dy = _programPointsTable[i].y - dstPosY;
			int distance = dx * dx + dy * dy;
			if (distance < minDistance) {
				minDistance = distance;
				minPointsDataNum = i;
			}
		}
	}

	minDistance = 32000;
	int minWalkDataNum = -1;
	for (uint i = 0; i < _programWalkTable.size(); ++i) {
		const ProgramWalkData *pwd = &_programWalkTable[i];
		if ((pwd->point1 & 0x4000) == 0) {
			int distance = 32000;
			ProgramPointData *pts1 = &_programPointsTable[pwd->point1];
			ProgramPointData *pts2 = &_programPointsTable[pwd->point2];
			if (pts1->order != 0) {
				int dx = pts2->x - pts1->x;
				int dy = pts2->y - pts1->y;
				if (dx == 0) {
					if (dstPosY > MIN(pts2->y, pts1->y) && dstPosY < MAX(pts2->y, pts1->y)) {
						int d = ABS(dstPosX - pts1->x);
						if (d <= 100) {
							distance = d * d;
						}
					}
				} else if (dy == 0) {
					if (dstPosX > MIN(pts2->x, pts1->x) && dstPosX < MAX(pts2->x, pts1->x)) {
						int d = ABS(dstPosY - pts1->y);
						if (d <= 100) {
							distance = d * d;
						}
					}
				} else {
					if (dstPosY > MIN(pts2->y, pts1->y) && dstPosY < MAX(pts2->y, pts1->y) &&
						dstPosX > MIN(pts2->x, pts1->x) && dstPosX < MAX(pts2->x, pts1->x) ) {
						distance = (dstPosX - pts1->x) * dy - (dstPosY - pts1->y) * dx;
						distance /= (dx * dx + dy * dy);
					}
				}
				if (distance < minDistance) {
					minDistance = distance;
					minWalkDataNum = i;
				}
			}
		}
	}
	if (!sortPointsData(minWalkDataNum, minPointsDataNum)) {
		return;
	}
	int dstPosZ;
	buildWalkPointsList(keyChar);
	KeyChar *key = &_keyCharsTable[keyChar];
	if (minWalkDataNum == -1) {
		dstPosX = _programPointsTable[minPointsDataNum].x;
		dstPosY = _programPointsTable[minPointsDataNum].y;
		dstPosZ = _programPointsTable[minPointsDataNum].z;
	} else {
		ProgramWalkData *pwd = &_programWalkTable[minWalkDataNum];
		ProgramPointData *pts1 = &_programPointsTable[pwd->point1];
		ProgramPointData *pts2 = &_programPointsTable[pwd->point2];
		int16 dx = pts2->x - pts1->x;
		int16 dy = pts2->y - pts1->y;
		int16 dz = pts2->z - pts1->z;
		if (ABS(dy) > ABS(dx)) {
			dstPosZ = pts2->z - (pts2->y - dstPosY) * dz / dy;
			dstPosX = pts2->x - (pts2->y - dstPosY) * dx / dy;
		} else {
			dstPosZ = pts2->z - (pts2->x - dstPosX) * dz / dx;
			dstPosY = pts2->y - (pts2->x - dstPosX) * dy / dx;
		}
	}
	key->prevWalkDataNum = minWalkDataNum;
	if (key->walkDataNum == key->prevWalkDataNum && key->walkPointsList[1] == -1 && minWalkDataNum != -1) {
		if (key->walkPointsList[0] == _programWalkTable[minWalkDataNum].point1 || key->walkPointsList[0] == _programWalkTable[minWalkDataNum].point2) {
			++key->walkPointsListIndex;
		}
	}
	key->xPosPrev = dstPosX;
	key->yPosPrev = dstPosY;
	key->zPosPrev = dstPosZ;
	if (_flagsTable[902] != 0) {
		Graphics::fillRect(_backdropBuffer, _currentBitmapWidth, dstPosX, dstPosY, 4, 4, 0xFC);
	}
}

void ToucheEngine::addToAnimationTable(int num, int posNum, int keyChar, int delayCounter) {
	for (int i = 0; i < NUM_ANIMATION_ENTRIES; ++i) {
		AnimationEntry *anim = &_animationTable[i];
		if (anim->num == 0) {
			anim->num = num;
			anim->delayCounter = delayCounter;
			anim->posNum = posNum;
			int16 xPos, yPos, x2Pos, y2Pos;
			if (posNum >= 0) {
				assert(posNum < NUM_KEYCHARS);
				xPos = _keyCharsTable[posNum].xPos;
				yPos = _keyCharsTable[posNum].yPos - 50;
			} else {
				posNum = -posNum;
				assert((uint)posNum < _programPointsTable.size());
				xPos = _programPointsTable[posNum].x;
				yPos = _programPointsTable[posNum].y;
			}
			xPos -= _flagsTable[614];
			yPos -= _flagsTable[615];
			assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
			x2Pos = _keyCharsTable[keyChar].xPos - _flagsTable[614];
			y2Pos = _keyCharsTable[keyChar].yPos - _flagsTable[615] - 50;
			xPos -= x2Pos;
			yPos -= y2Pos;
			xPos /= 8;
			yPos /= 8;
			anim->x = x2Pos;
			anim->y = y2Pos;
			anim->dx = xPos;
			anim->dy = yPos;
			anim->displayCounter = 8;
			anim->displayRect.left = -1;
			break;
		}
	}
}

void ToucheEngine::copyAnimationImage(int dstX, int dstY, int w, int h, const uint8 *src, int srcX, int srcY, int fillColor) {
	Area copyRegion(dstX, dstY, w, h);
	copyRegion.srcX = srcX;
	copyRegion.srcY = srcY;
	if (copyRegion.clip(_screenRect)) {
		if (fillColor != -1) {
			Graphics::copyMask(_offscreenBuffer, kScreenWidth, copyRegion.r.left, copyRegion.r.top,
			  src, kIconWidth, copyRegion.srcX, copyRegion.srcY,
			  copyRegion.r.width(), copyRegion.r.height(),
			  (uint8)fillColor);
		} else {
			Graphics::copyRect(_offscreenBuffer, kScreenWidth, copyRegion.r.left, copyRegion.r.top,
			  src, kIconWidth, copyRegion.srcX, copyRegion.srcY,
			  copyRegion.r.width(), copyRegion.r.height(),
			  Graphics::kTransparent);
		}
	}
}

void ToucheEngine::drawAnimationImage(AnimationEntry *anim) {
	if (anim->displayRect.left != -1) {
		addToDirtyRect(anim->displayRect);
	}
	int x = anim->x;
	int y = anim->y;
	int dx = -anim->dx;
	int dy = -anim->dy;

	int displayRectX1 = 30000;
	int displayRectY1 = 30000;
	int displayRectX2 = -30000;
	int displayRectY2 = -30000;

	dx /= 3;
	dy /= 3;

	res_loadImage(anim->num, _iconData);
	int color = 0xCF;

	x += dx * 5 - 29;
	y += dy * 5 - 21;
	dx = -dx;
	dy = -dy;
	for (int i = 0; i < 6; ++i) {
		if (i == 5) {
			color = -1;
		}
		copyAnimationImage(x, y, kIconWidth, kIconHeight, _iconData, 0, 0, color);
		--color;
		displayRectX1 = MIN(x, displayRectX1);
		displayRectX2 = MAX(x, displayRectX2);
		displayRectY1 = MIN(y, displayRectY1);
		displayRectY2 = MAX(y, displayRectY2);
		x += dx;
		y += dy;
	}
	anim->displayRect = Common::Rect(displayRectX1, displayRectY1, displayRectX2 + kIconWidth, displayRectY2 + kIconHeight);
	addToDirtyRect(anim->displayRect);
}

void ToucheEngine::processAnimationTable() {
	for (int i = 0; i < NUM_ANIMATION_ENTRIES; ++i) {
		AnimationEntry *anim = &_animationTable[i];
		if (anim->num != 0) {
			if (anim->displayCounter == 0) {
				anim->num = 0;
				if (anim->displayRect.left != -1) {
					addToDirtyRect(anim->displayRect);
				}
			} else {
				if (anim->delayCounter != 0) {
					--anim->delayCounter;
				} else {
					anim->x += anim->dx;
					anim->y += anim->dy;
					drawAnimationImage(anim);
					--anim->displayCounter;
				}
			}
		}
	}
}

void ToucheEngine::clearAnimationTable() {
	memset(_animationTable, 0, sizeof(_animationTable));
}

void ToucheEngine::addToDirtyRect(const Common::Rect &r) {
	if (_fullRedrawCounter == 0 && r.width() > 0 && r.height() > 0 && r.intersects(_roomAreaRect)) {
		Common::Rect dirtyRect(r);
		dirtyRect.clip(_roomAreaRect);
		if (_dirtyRectsTableCount == 0) {
			_dirtyRectsTable[_dirtyRectsTableCount] = dirtyRect;
			++_dirtyRectsTableCount;
		} else {
			int index = -1;
			int minRectSurface = kScreenWidth * kScreenHeight;
			for (int i = 0; i < _dirtyRectsTableCount; ++i) {
				if (r.intersects(_dirtyRectsTable[i])) {
					Common::Rect tmpRect(r);
					tmpRect.extend(_dirtyRectsTable[i]);
					int rectSurface = tmpRect.width() * tmpRect.height();
					if (rectSurface < minRectSurface) {
						minRectSurface = rectSurface;
						index = i;
					}
				}
			}
			if (index != -1) {
				_dirtyRectsTable[index].extend(dirtyRect);
			} else if (_dirtyRectsTableCount == NUM_DIRTY_RECTS) {
				debug(0, "Too many dirty rects, performing full screen update");
				_fullRedrawCounter = 1;
			} else {
				_dirtyRectsTable[_dirtyRectsTableCount] = dirtyRect;
				++_dirtyRectsTableCount;
			}
		}
	}
}

void ToucheEngine::clearDirtyRects() {
	_dirtyRectsTableCount = 0;
}

void ToucheEngine::setPalette(int firstColor, int colorCount, int rScale, int gScale, int bScale) {
	uint8 pal[256 * 3];
	for (int i = firstColor; i < firstColor + colorCount; ++i) {
		int r = _paletteBuffer[i * 3 + 0];
		r = (r * rScale) >> 8;
		pal[i * 3 + 0] = (uint8)r;

		int g = _paletteBuffer[i * 3 + 1];
		g = (g * gScale) >> 8;
		pal[i * 3 + 1] = (uint8)g;

		int b = _paletteBuffer[i * 3 + 2];
		b = (b * bScale) >> 8;
		pal[i * 3 + 2] = (uint8)b;
	}
	_system->getPaletteManager()->setPalette(&pal[firstColor * 3], firstColor, colorCount);
}

void ToucheEngine::updateScreenArea(int x, int y, int w, int h) {
	_system->copyRectToScreen(_offscreenBuffer + y * kScreenWidth + x, kScreenWidth, x, y, w, h);
}

void ToucheEngine::updateEntireScreen() {
	int h = (_flagsTable[606] != 0) ? kScreenHeight : kRoomHeight;
	_system->copyRectToScreen(_offscreenBuffer, kScreenWidth, 0, 0, kScreenWidth, h);
}

void ToucheEngine::updateDirtyScreenAreas() {
//	_fullRedrawCounter = 1;
	if (_fullRedrawCounter != 0) {
		updateEntireScreen();
		--_fullRedrawCounter;
	} else {
		debug(1, "dirtyRectsCount=%d", _dirtyRectsTableCount);
		for (int i = 0; i < _dirtyRectsTableCount; ++i) {
			const Common::Rect &r = _dirtyRectsTable[i];
#if 0
			Graphics::drawRect(_offscreenBuffer, kScreenWidth, r.left, r.top, r.width(), r.height(), 0xFF, 0xFF);
#endif
			_system->copyRectToScreen(_offscreenBuffer + r.top * kScreenWidth + r.left, kScreenWidth, r.left, r.top, r.width(), r.height());
		}
		if (_menuRedrawCounter) {
			const Common::Rect &r = _cursorObjectRect;
			_system->copyRectToScreen(_offscreenBuffer + r.top * kScreenWidth + r.left, kScreenWidth, r.left, r.top, r.width(), r.height());
			--_menuRedrawCounter;
		}
	}
}

void ToucheEngine::updatePalette() {
	_system->getPaletteManager()->setPalette(_paletteBuffer, 0, 256);
}

bool ToucheEngine::canLoadGameStateCurrently() {
	return _gameState == kGameStateGameLoop && _flagsTable[618] == 0 && !_hideInventoryTexts;
}

bool ToucheEngine::canSaveGameStateCurrently() {
	return _gameState == kGameStateGameLoop && _flagsTable[618] == 0 && !_hideInventoryTexts;
}

} // namespace Touche
