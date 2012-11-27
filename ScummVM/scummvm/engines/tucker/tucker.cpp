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
#include "common/events.h"
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

#include "tucker/tucker.h"
#include "tucker/graphics.h"

namespace Tucker {

TuckerEngine::TuckerEngine(OSystem *system, Common::Language language, uint32 flags)
	: Engine(system), _gameLang(language), _gameFlags(flags), _rnd("tucker") {
	_console = new TuckerConsole(this);
}

TuckerEngine::~TuckerEngine() {
	delete _console;
}

bool TuckerEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case kSupportsRTL:
	case kSupportsLoadingDuringRuntime:
	case kSupportsSavingDuringRuntime:
		return true;
	default:
		return false;
	}
}

Common::Error TuckerEngine::run() {
	initGraphics(kScreenWidth, kScreenHeight, false);
	syncSoundSettings();
	_compressedSound.openFile();
	handleIntroSequence();
	if ((_gameFlags & kGameFlagIntroOnly) == 0 && !shouldQuit()) {
		mainLoop();
	}
	_compressedSound.closeFile();
	return Common::kNoError;
}

int TuckerEngine::getRandomNumber() {
	return _rnd.getRandomNumber(0x7FFF);
}

void TuckerEngine::allocateBuffers() {
	_locationBackgroundGfxBuf = (uint8 *)calloc(1, 640 * 200);
	_loadTempBuf = (uint8 *)calloc(1, 64010);
	_panelGfxBuf = (uint8 *)calloc(1, 64010);
	_itemsGfxBuf = (uint8 *)calloc(1, 19200);
	_charsetGfxBuf = (uint8 *)calloc(1, 22400);
	_cursorGfxBuf = (uint8 *)calloc(1, 256 * 7);
	_infoBarBuf = (uint8 *)calloc(1, 1000);
	_charNameBuf = 0;
	_bgTextBuf = 0;
	_objTxtBuf = 0;
	_panelObjectsGfxBuf = (uint8 *)calloc(1, 20000);
	_data5Buf = 0;
	_data3GfxBuf = (uint8 *)calloc(1, 250000);
	_quadBackgroundGfxBuf = (uint8 *)calloc(1, 320 * 140 * 4);
	_locationBackgroundMaskBuf = (uint8 *)calloc(1, 640 * 140);
	_csDataBuf = 0;
	_spritesGfxBuf = (uint8 *)calloc(1, 160000);
	_ptTextBuf = 0;
	memset(_charWidthTable, 0, sizeof(_charWidthTable));
}

void TuckerEngine::freeBuffers() {
	free(_locationBackgroundGfxBuf);
	free(_loadTempBuf);
	free(_panelGfxBuf);
	free(_itemsGfxBuf);
	free(_charsetGfxBuf);
	free(_cursorGfxBuf);
	free(_infoBarBuf);
	free(_charNameBuf);
	free(_bgTextBuf);
	free(_objTxtBuf);
	free(_panelObjectsGfxBuf);
	free(_data5Buf);
	free(_data3GfxBuf);
	free(_quadBackgroundGfxBuf);
	free(_locationBackgroundMaskBuf);
	free(_csDataBuf);
	free(_spritesGfxBuf);
	free(_ptTextBuf);
}

void TuckerEngine::restart() {
	_quitGame = false;
	_fastMode = false;
	_syncCounter = 0;
	_lastFrameTime = _system->getMillis();
	_mainLoopCounter1 = _mainLoopCounter2 = 0;
	_timerCounter2 = 0;
	_partNum = _currentPartNum = 0;
	_locationNum = 0;
	_nextLocationNum = (_gameFlags & kGameFlagDemo) == 0 ? kStartupLocationGame : kStartupLocationDemo;
	_gamePaused = false;
	_gameDebug = false;
	_displayGameHints = false;
	_displaySpeechText = (_gameFlags & kGameFlagNoSubtitles) == 0 ? ConfMan.getBool("subtitles") : false;
	memset(_flagsTable, 0, sizeof(_flagsTable));

	_gameHintsIndex = 0;
	_gameHintsCounter = 0;
	_gameHintsDisplayText = 0;
	_gameHintsStringNum = 0;

	if ((_gameFlags & kGameFlagDemo) == 0) {
		_locationWidthTable = _locationWidthTableGame;
		_locationHeightTable = _locationHeightTableGame;
	} else {
		_locationWidthTable = _locationWidthTableDemo;
		_locationHeightTable = _locationHeightTableDemo;
	}

	memset(_sprA02Table, 0, sizeof(_sprA02Table));
	memset(_sprC02Table, 0, sizeof(_sprC02Table));
	memset(_actionsTable, 0, sizeof(_actionsTable));
	_actionsCount = 0;
	memset(_locationObjectsTable, 0, sizeof(_locationObjectsTable));
	_locationObjectsCount = 0;
	memset(_spritesTable, 0, sizeof(_spritesTable));
	_spritesCount = 0;
	memset(_locationAnimationsTable, 0, sizeof(_locationAnimationsTable));
	_locationAnimationsCount = 0;
	memset(_dataTable, 0, sizeof(_dataTable));
	_dataCount = 0;
	memset(_charPosTable, 0, sizeof(_charPosTable));
	_charPosCount = 0;
	memset(_locationSoundsTable, 0, sizeof(_locationSoundsTable));
	_locationSoundsCount = 0;
	memset(_locationMusicsTable, 0, sizeof(_locationMusicsTable));
	_locationMusicsCount = 0;

	_mousePosX = _mousePosY = 0;
	_prevMousePosX = _prevMousePosY = 0;
	_mouseButtonsMask = 0;
	_mouseClick = 0;
	_saveOrLoadGamePanel = 0;
	_mouseIdleCounter = 0;
	_leftMouseButtonPressed = _rightMouseButtonPressed = false;
	_lastKeyPressed = 0;
	memset(_inputKeys, 0, sizeof(_inputKeys));
	_cursorNum = 0;
	_cursorType = 0;
	_updateCursorFlag = false;

	_panelNum = 1;
	_panelState = 0;
	_forceRedrawPanelItems = true;
	_redrawPanelItemsCounter = 0;
	_switchPanelFlag = 0;
	memset(_panelObjectsOffsetTable, 0, sizeof(_panelObjectsOffsetTable));
	_switchPanelCounter = 0;
	_conversationOptionsCount = 0;
	_fadedPanel = false;
	_panelLockedFlag = 0;
	_conversationOptionLinesCount = 0;
	memset(_inventoryItemsState, 0, sizeof(_inventoryItemsState));
	memset(_inventoryObjectsList, 0, sizeof(_inventoryObjectsList));
	_inventoryObjectsOffset = 0;
	_inventoryObjectsCount = 0;
	_lastInventoryObjectIndex = 0;

	_currentFxSet = 0;
	_currentFxDist = 0;
	_currentFxScale = 0;
	_currentFxVolume = 0;
	_currentFxIndex = 0;
	_speechSoundNum = 0;
	_speechVolume = kMaxSoundVolume;
	memset(_miscSoundFxNum, 0, sizeof(_miscSoundFxNum));
	memset(_speechHistoryTable, 0, sizeof(_speechHistoryTable));
	_charSpeechSoundCounter = 0;
	memset(_miscSoundFxDelayCounter, 0, sizeof(_miscSoundFxDelayCounter));
	_characterSoundFxDelayCounter = 0;
	_characterSoundFxNum = 0;
	_speechSoundBaseNum = 0;

	_pendingActionIndex = 0;
	_pendingActionDelay = 0;
	_charPositionFlagNum = 0;
	_charPositionFlagValue = 0;
	_actionVerb = kVerbWalk;
	_nextAction = 0;
	_selectedObjectNum = 0;
	_selectedObjectType = 0;
	_selectedCharacterNum = 0;
	_actionObj1Type = _actionObj2Type = 0;
	_actionObj1Num = _actionObj2Num = 0;
	_actionRequiresTwoObjects = 0;
	_actionVerbLocked = 0;
	_actionPosX = 0;
	_actionPosY = 0;
	_selectedObjectLocationMask = 0;
	memset(&_selectedObject, 0, sizeof(_selectedObject));
	_selectedCharacterDirection = 0;
	_selectedCharacter2Num = 0;
	_currentActionObj1Num = _currentActionObj2Num = 0;
	_currentInfoString1SourceType = _currentInfoString2SourceType = 0;
	memset(_speechActionCounterTable, 0, sizeof(_speechActionCounterTable));
	_actionCharacterNum = 0;

	_csDataLoaded = false;
	_csDataHandled = false;
	_stopActionOnSoundFlag = false;
	_stopActionOnSpeechFlag = false;
	_stopActionOnPanelLock = false;
	_csDataTableCount = 0;
	_stopActionCounter = 0;
	_actionTextColor = 0;
	_nextTableToLoadIndex = 0;
	memset(_nextTableToLoadTable, 0, sizeof(_nextTableToLoadTable));
	_soundInstructionIndex = 0;
	_tableInstructionsPtr = 0;
	memset(_tableInstructionObj1Table, 0, sizeof(_tableInstructionObj1Table));
	memset(_tableInstructionObj2Table, 0, sizeof(_tableInstructionObj2Table));
	_tableInstructionFlag = 0;
	_tableInstructionItemNum1 = _tableInstructionItemNum2 = 0;
	memset(_instructionsActionsTable, 0, sizeof(_instructionsActionsTable));
	_validInstructionId = false;

	memset(_spriteFramesTable, 0, sizeof(_spriteFramesTable));
	memset(_spriteAnimationsTable, 0, sizeof(_spriteAnimationsTable));
	memset(_spriteAnimationFramesTable, 0, sizeof(_spriteAnimationFramesTable));
	_spriteAnimationFrameIndex = 0;
	_backgroundSpriteCurrentFrame = 0;
	_backgroundSpriteLastFrame = 0;
	_backgroundSpriteCurrentAnimation = -1;
	_disableCharactersPath = false;
	_skipCurrentCharacterDraw = false;
	_yPosCurrent = 131;
	_xPosCurrent = 160;
	_characterSpeechDataPtr = 0;
	_ptTextOffset = 0;
	memset(_characterAnimationsTable, 0, sizeof(_characterAnimationsTable));
	memset(_characterStateTable, 0, sizeof(_characterStateTable));
	_backgroundSprOffset = 0;
	_currentActionVerb = 0;
	_previousActionVerb = 0;
	_mainSpritesBaseOffset = 0;
	_currentSpriteAnimationLength = 0;
	_currentSpriteAnimationFrame = 0;
	_currentSpriteAnimationFrame2 = 0;
	_characterAnimationIndex = -1;
	_characterFacingDirection = _characterPrevFacingDirection = 0;
	_characterBackFrontFacing = _characterPrevBackFrontFacing = 0;
	_characterAnimationNum = 0;
	_noCharacterAnimationChange = 0;
	_changeBackgroundSprite = 0;
	_characterSpriteAnimationFrameCounter = 0;
	_locationMaskIgnore = 0;
	_locationMaskType = 0;
	_locationMaskCounter = 0;
	_updateSpriteFlag1 = 0;
	_updateSpriteFlag2 = 0;
	_handleMapCounter = 0;
	_noPositionChangeAfterMap = false;

	_mirroredDrawing = 0;
	_loadLocBufPtr = 0;
	_backgroundSpriteDataPtr = 0;
	_locationHeight = 0;
	_scrollOffset = 0;
	_currentGfxBackgroundCounter = 0;
	_currentGfxBackground = 0;
	_fadePaletteCounter = 0;
	memset(_currentPalette, 0, sizeof(_currentPalette));
	_fullRedraw = false;
	_dirtyRectsPrevCount = _dirtyRectsCount = 0;

	_updateLocationFadePaletteCounter = 0;
	_updateLocationCounter = 10;
	_updateLocationPos = 0;
	for (int i = 0; i < 5; ++i) {
		_updateLocationXPosTable[i] = 160;
		_updateLocationYPosTable[i] = 131;
	}
	memset(_updateLocationFlagsTable, 0, sizeof(_updateLocationFlagsTable));
	memset(_updateLocationXPosTable2, 0, sizeof(_updateLocationXPosTable2));
	memset(_updateLocationYPosTable2, 0, sizeof(_updateLocationYPosTable2));
	memset(_updateLocationYMaxTable, 0, sizeof(_updateLocationYMaxTable));
	memset(_updateLocation14Step, 0, sizeof(_updateLocation14Step));
	memset(_updateLocation14ObjNum, 0, sizeof(_updateLocation14ObjNum));
	memset(_updateLocation14Delay, 0, sizeof(_updateLocation14Delay));
	_updateLocationCounter2 = 0;
	_updateLocationFlag = 0;
	_updateLocation70StringLen = 0;
	memset(_updateLocation70String, 0, sizeof(_updateLocation70String));
}

void TuckerEngine::mainLoop() {
	allocateBuffers();
	restart();

	loadCharSizeDta();
	if ((_gameFlags & kGameFlagDemo) != 0) {
		addObjectToInventory(30);
		addObjectToInventory(12);
	}
	loadCharset();
	loadPanel();
	loadFile("infobar.txt", _infoBarBuf);
	_data5Buf = loadFile("data5.c", 0);
	_bgTextBuf = loadFile("bgtext.c", 0);
	_charNameBuf = loadFile("charname.c", 0);
	_csDataBuf = loadFile("csdata.c", 0);
	_csDataSize = _fileLoadSize;

	_currentSaveLoadGameState = 1;

	loadBudSpr(0);
	loadCursor();
	setCursorNum(_cursorNum);
	setCursorType(_cursorType);

	_flagsTable[219] = 1;
	_flagsTable[105] = 1;

	_spriteAnimationFrameIndex =  _spriteAnimationsTable[14].firstFrameIndex;

	if (ConfMan.hasKey("save_slot")) {
		const int slot = ConfMan.getInt("save_slot");
		if (slot >= 0 && slot <= kLastSaveSlot) {
			loadGameState(slot);
		}
	} else if (ConfMan.hasKey("boot_param")) {
		_nextLocationNum = ConfMan.getInt("boot_param");
	}

	do {
		++_syncCounter;
		if (_flagsTable[137] != _flagsTable[138]) {
			loadBudSpr(0);
			_flagsTable[138] = _flagsTable[137];
		}
		if (_syncCounter >= 2) {
			_syncCounter = 0;
			waitForTimer(2);
		}
		updateMouseState();
		if (_fadePaletteCounter < 16) {
			if (_fadePaletteCounter > 1) {
				fadeOutPalette();
			}
			++_fadePaletteCounter;
		}
		if (_fadePaletteCounter > 19 && _fadePaletteCounter < 34) {
			fadeInPalette();
			++_fadePaletteCounter;
		}
		if (_nextAction != 0) {
			loadActionsTable();
		}
		if (_nextLocationNum > 0) {
			setupNewLocation();
		}
		updateCharPosition();
		if (_cursorType == 0) {
			updateCursor();
		} else if (_panelState == 2) {
			handleMouseOnPanel();
		}
		if (_mainLoopCounter2 == 0) {
			updateFlagsForCharPosition();
		}
		if (_syncCounter == 0 || !_disableCharactersPath) {
			updateCharactersPath();
		}
		if (_mainLoopCounter2 == 0) {
			updateCharacterAnimation();
			if (_backgroundSpriteCurrentAnimation == -1) {
				_flagsTable[207] = 0;
				if (_flagsTable[220] > 0) {
					_flagsTable[_flagsTable[220]] = _flagsTable[221];
					_flagsTable[220] = 0;
				}
				if (_flagsTable[158] == 1) {
					_flagsTable[158] = 0;
					_skipCurrentCharacterDraw = 1;
				}
				_mainLoopCounter1 = 0;
			}
		}
		if (_mainLoopCounter1 == 0) {
			updateSprites();
			updateData3();
			updateSfxData3_1();
		}
		++_mainLoopCounter2;
		handleMap();
		updateScreenScrolling();
		if (_mainLoopCounter2 > 4) {
			_mainLoopCounter2 = 0;
			updateSfxData3_2();
		}
		++_mainLoopCounter1;
		if (_mainLoopCounter1 > 5) {
			_mainLoopCounter1 = 0;
		}
		if (_locationHeight == 140) {
			switchPanelType();
			redrawPanelItems();
			if (_displayGameHints && _gameHintsIndex < 6) {
				updateGameHints();
			}
			if (_panelState == 0) {
				if (_panelLockedFlag == 1 || _pendingActionDelay > 0) {
					if (!_fadedPanel) {
						updateItemsGfxColors(0x60, 0x80);
						_fadedPanel = true;
					}
				} else {
					_fadedPanel = false;
					clearItemsGfx();
					if (_gamePaused) {
						drawPausedInfoBar();
					} else if (_gameHintsDisplayText == 1 && _mouseIdleCounter > 1000) {
						drawGameHintString();
					} else {
						drawInfoString();
					}
				}
			}
		}
		_mainSpritesBaseOffset = 0;
		if (_locationWidthTable[_locationNum] > 3) {
			++_currentGfxBackgroundCounter;
			if (_currentGfxBackgroundCounter > 39) {
				_currentGfxBackgroundCounter = 0;
			}
			_currentGfxBackground = _quadBackgroundGfxBuf + (_currentGfxBackgroundCounter / 10) * 44800;
			if (_fadePaletteCounter < 34 && _locationNum == 22) {
				_spritesTable[0].gfxBackgroundOffset = (_currentGfxBackgroundCounter / 10) * 640;
				_mainSpritesBaseOffset = _currentGfxBackgroundCounter / 10;
				if (_locationNum == 22 && _currentGfxBackgroundCounter <= 29) {
					_spritesTable[0].gfxBackgroundOffset = 640;
					_mainSpritesBaseOffset = 1;
				}
			}
			_fullRedraw = true;
		} else {
			_currentGfxBackground = _quadBackgroundGfxBuf;
		}
		if (_syncCounter != 0) {
			continue;
		}
		if (_scrollOffset < 320) {
			Graphics::copyRect(_locationBackgroundGfxBuf + _scrollOffset, 640, _currentGfxBackground + _scrollOffset, 320, 320 - _scrollOffset, _locationHeight);
		}
		if (_scrollOffset > 0) {
			Graphics::copyRect(_locationBackgroundGfxBuf + 320, 640, _currentGfxBackground + 44800, 320, _scrollOffset, _locationHeight);
		}
		drawData3();
		execData3PreUpdate();
		for (int i = 0; i < _spritesCount; ++i) {
			if (!_spritesTable[i].disabled) {
				drawSprite(i);
			}
		}
		if (_skipCurrentCharacterDraw != 1) {
			if (_backgroundSpriteCurrentAnimation > -1 && _backgroundSpriteCurrentFrame > 0) {
				drawBackgroundSprites();
			} else {
				drawCurrentSprite();
			}
		}
		if (_locationHeight == 140) {
			redrawPanelOverBackground();
		}
		if (_panelState == 3) {
			saveOrLoad();
		}
		execData3PostUpdate();
		if (_timerCounter2 > 45) {
			_timerCounter2 = 0;
		}
		updateSoundsTypes3_4();
		if (_currentFxSet != 0) {
			setSoundVolumeDistance();
		}
		updateCharSpeechSound(_displaySpeechText);
		redrawScreen(_scrollOffset);
		startCharacterSounds();
		for (int num = 0; num < 2; ++num) {
			if (_miscSoundFxDelayCounter[num] > 0) {
				--_miscSoundFxDelayCounter[num];
				if (_miscSoundFxDelayCounter[num] == 0) {
					const int index = _miscSoundFxNum[num];
					startSound(_locationSoundsTable[index].offset, index, _locationSoundsTable[index].volume);
				}
			}
		}
		if (_gamePaused && _charSpeechSoundCounter == 0) {
			stopSounds();
			while (1) {
				waitForTimer(1);
				if (_inputKeys[kInputKeyPause]) {
					_inputKeys[kInputKeyPause] = false;
					if (_charSpeechSoundCounter <= 0) {
						break;
					}
				}
				if (_charSpeechSoundCounter == 0) {
					if (_lastKeyPressed >= Common::KEYCODE_1 && _lastKeyPressed <= Common::KEYCODE_5) {
						if (_speechHistoryTable[_lastKeyPressed - Common::KEYCODE_1] > 0) {
							startSpeechSound(_speechHistoryTable[_lastKeyPressed - Common::KEYCODE_1], 100);
							_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
						}
						_lastKeyPressed = 0;
					}
				}
				updateCharSpeechSound(false);
			}
			playSounds();
			_gamePaused = false;
		}
		if (_inputKeys[kInputKeyPause]) {
			_inputKeys[kInputKeyPause] = false;
			if (_locationNum != 70) {
				_gamePaused = true;
			}
		}
		if (_inputKeys[kInputKeyToggleTextSpeech]) {
			_inputKeys[kInputKeyToggleTextSpeech] = false;
			if ((_gameFlags & kGameFlagNoSubtitles) == 0) {
				_displaySpeechText = !_displaySpeechText;
				ConfMan.setBool("subtitles", _displaySpeechText);
			}
		}
		if (_inputKeys[kInputKeyHelp]) {
			_inputKeys[kInputKeyHelp] = false;
			if (_displayGameHints && _gameHintsDisplayText == 1) {
				_gameHintsStringNum = _gameHintsIndex + 1;
				_mouseIdleCounter = 1100;
			}
		}
		if (_inputKeys[kInputKeyEscape]) {
			_inputKeys[kInputKeyEscape] = false;
			if (_gameDebug) {
				_flagsTable[236] = 74;
			}
		}
		if (_flagsTable[236] > 70) {
			handleCreditsSequence();
			_quitGame = true;
		}
	} while (!_quitGame && _flagsTable[100] == 0);
	if (_flagsTable[100] == 1) {
		handleCongratulationsSequence();
	}
	unloadSprA02_01();
	unloadSprC02_01();
	freeBuffers();
}

void TuckerEngine::waitForTimer(int ticksCount) {
	uint32 end = _lastFrameTime + ticksCount * 1000 / 46;
	do {
		parseEvents();
		_system->delayMillis(10);
		_lastFrameTime = _system->getMillis();
	} while (!_fastMode && _lastFrameTime < end);
	_timerCounter2 += ticksCount;
}

void TuckerEngine::parseEvents() {
	Common::Event ev;
	while (_eventMan->pollEvent(ev)) {
		switch (ev.type) {
		case Common::EVENT_KEYDOWN:
			switch (ev.kbd.keycode) {
			case Common::KEYCODE_f:
				if (ev.kbd.hasFlags(Common::KBD_CTRL)) {
					_fastMode = !_fastMode;
				}
				break;
			case Common::KEYCODE_p:
				_inputKeys[kInputKeyPause] = true;
				break;
			case Common::KEYCODE_F1:
				_inputKeys[kInputKeyToggleInventory] = true;
				break;
			case Common::KEYCODE_F2:
				_inputKeys[kInputKeyToggleTextSpeech] = true;
				break;
			case Common::KEYCODE_F3:
				_inputKeys[kInputKeyHelp] = true;
				break;
			case Common::KEYCODE_ESCAPE:
				_inputKeys[kInputKeyEscape] = true;
				break;
			case Common::KEYCODE_d:
				if (ev.kbd.hasFlags(Common::KBD_CTRL)) {
					this->getDebugger()->attach();
					this->getDebugger()->onFrame();
				}
				break;
			default:
				break;
			}
			_lastKeyPressed = ev.kbd.keycode;
			break;
		case Common::EVENT_MOUSEMOVE:
			updateCursorPos(ev.mouse.x, ev.mouse.y);
			break;
		case Common::EVENT_LBUTTONDOWN:
			updateCursorPos(ev.mouse.x, ev.mouse.y);
			_mouseButtonsMask |= 1;
			break;
		case Common::EVENT_LBUTTONUP:
			updateCursorPos(ev.mouse.x, ev.mouse.y);
			break;
		case Common::EVENT_RBUTTONDOWN:
			updateCursorPos(ev.mouse.x, ev.mouse.y);
			_mouseButtonsMask |= 2;
			break;
		case Common::EVENT_RBUTTONUP:
			updateCursorPos(ev.mouse.x, ev.mouse.y);
			break;
		default:
			break;
		}
	}
	_quitGame = shouldQuit();
}

void TuckerEngine::updateCursorPos(int x, int y) {
	_prevMousePosX = _mousePosX;
	_prevMousePosY = _mousePosY;
	_mousePosX = x;
	_mousePosY = y;
}

void TuckerEngine::setCursorNum(int num) {
	_cursorNum = num;
	static const int cursorW = 16;
	static const int cursorH = 16;
	CursorMan.replaceCursor(_cursorGfxBuf + _cursorNum * 256, cursorW, cursorH, 1, 1, 0);
}

void TuckerEngine::setCursorType(int type) {
	_cursorType = type;
	CursorMan.showMouse(_cursorType < 2);
}

void TuckerEngine::setupNewLocation() {
	debug(2, "setupNewLocation() current %d next %d", _locationNum, _nextLocationNum);
	_locationNum = _nextLocationNum;
	loadObj();
	_switchPanelFlag = 0;
	_nextLocationNum = 0;
	_fadePaletteCounter = 0;
	_mainLoopCounter2 = 0;
	_mainLoopCounter1 = 0;
	_characterFacingDirection = 0;
	_actionVerbLocked = 0;
	_locationMaskIgnore = 0;
	_backgroundSprOffset = 0;
	if (_backgroundSpriteCurrentAnimation > 0 && _backgroundSpriteCurrentFrame > 0) {
		_backgroundSpriteCurrentAnimation = -1;
		_backgroundSpriteCurrentFrame = 0;
	}
	if (_panelLockedFlag == 0 || (_backgroundSpriteCurrentAnimation > 0 && _locationNum != 25)) {
		_locationMaskType = 0;
	} else {
		_locationMaskType = 3;
	}
	while (_spriteAnimationFramesTable[_spriteAnimationFrameIndex] != 999) {
		++_spriteAnimationFrameIndex;
	}
	_execData3Counter = 0;
	stopSounds();
	loadLoc();
	loadData4();
	loadData3();
	loadActionFile();
	loadCharPos();
	loadSprA02_01();
	loadSprC02_01();
	loadFx();
	playSounds();
	if (_flagsTable[215] > 0) {
		handleMeanwhileSequence();
		_flagsTable[215] = 0;
	}
	if (_flagsTable[231] > 0) {
		handleMeanwhileSequence();
		_flagsTable[231] = 0;
	}
}

void TuckerEngine::copyLocBitmap(const char *filename, int offset, bool isMask) {
	int type = !isMask ? 1 : 0;
	if (offset > 0 && _locationNum == 16) {
		type = 0;
	}
	loadImage(filename, _loadTempBuf, type);
	uint8 *dst = isMask ? _locationBackgroundMaskBuf : _locationBackgroundGfxBuf;
	dst += offset;
	const uint8 *src = _loadTempBuf;
	for (int y = 0; y < _locationHeight; ++y) {
		memcpy(dst, src, 320);
		src += 320;
		dst += 640;
	}
}

void TuckerEngine::updateMouseState() {
	if (_cursorType < 2) {
		_leftMouseButtonPressed = (_mouseButtonsMask & 1) != 0;
		if (_leftMouseButtonPressed) {
			_mouseIdleCounter = 0;
			_gameHintsStringNum = 0;
		}
		_rightMouseButtonPressed = (_mouseButtonsMask & 2) != 0;
		_mouseButtonsMask = 0;
		if (_prevMousePosX == _mousePosX && _prevMousePosY == _mousePosY) {
			++_mouseIdleCounter;
		} else {
			_mouseIdleCounter = 0;
			_gameHintsStringNum = 0;
		}
	}
	if (_cursorType == 1) {
		if (_panelState == 1) {
			setCursorNum(1);
		}
		if (_mousePosY < 140) {
			_mousePosY = 140;
		}
	}
}

void TuckerEngine::updateCharPositionHelper() {
	setCursorType(2);
	_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
	_currentActionVerb = 0;
	startSpeechSound(_speechSoundNum, _speechVolume);
	int pos = getPositionForLine(_speechSoundNum, _characterSpeechDataPtr);
	_characterSpeechDataPtr += pos;
	_speechSoundNum = 0;
}

void TuckerEngine::updateCharPosition() {
	if (_currentActionVerb == 0 || _locationMaskCounter == 0) {
		return;
	}
	if (_currentActionVerb == 1 && _locationNum != 18) {
		int pos;
		_actionPosX = _xPosCurrent;
		_actionPosY = _yPosCurrent - 64;
		_actionTextColor = 1;
		_actionCharacterNum = 99;
		switch (_currentInfoString1SourceType) {
		case 0:
			if (_currentActionObj1Num == 0) {
				return;
			}
			if (_currentActionObj1Num == 259) {
				handleSpecialObjectSelectionSequence();
				_currentActionVerb = 0;
				return;
			}
			_speechSoundNum = _currentActionObj1Num;
			_characterSpeechDataPtr = _ptTextBuf;
			pos = getPositionForLine(_speechSoundNum + 1865, _characterSpeechDataPtr);
			if (_characterSpeechDataPtr[pos] == '*') {
				switch (_characterSpeechDataPtr[pos + 1]) {
				case 'E':
					++_flagsTable[200];
					if (_flagsTable[200] > 6) {
						_flagsTable[200] = 1;
					}
					_speechSoundNum = 262 + _flagsTable[200];
					break;
				case 'M':
					++_flagsTable[200];
					if (_flagsTable[200] > 10) {
						_flagsTable[200] = 1;
					}
					_speechSoundNum = 268 + _flagsTable[200];
					break;
				case 'R':
					++_flagsTable[200];
					if (_flagsTable[200] > 10) {
						_flagsTable[200] = 1;
					}
					_speechSoundNum = 281 + _flagsTable[200];
					break;
				}
			}
			_speechSoundNum += 1865;
			updateCharPositionHelper();
			return;
		case 1:
			if (_locationAnimationsTable[_selectedCharacter2Num].getFlag == 1) {
				_speechSoundNum = _speechSoundBaseNum + _locationAnimationsTable[_selectedCharacter2Num].inventoryNum;
				_characterSpeechDataPtr = _ptTextBuf;
				updateCharPositionHelper();
				return;
			} else if (_currentActionObj1Num == 91) {
				handleSpecialObjectSelectionSequence();
				_currentActionVerb = 0;
				return;
			}
			break;
		case 2:
			_characterSpeechDataPtr = _ptTextBuf;
			_speechSoundNum = 2175 + _charPosTable[_selectedCharacterNum].description;
			if (_charPosTable[_selectedCharacterNum].description != 0) {
				updateCharPositionHelper();
				return;
			}
			break;
		}
	}
	int actionKey = _currentActionObj2Num * 1000000 + _currentInfoString2SourceType * 100000 + _currentActionVerb * 10000 + _currentInfoString1SourceType * 1000 + _currentActionObj1Num;
	debug(3, "updateCharPosition() actionKey %d",  actionKey);
	int skip = 0;
	Action *action = 0;
	for (int i = 0; i < _actionsCount && skip == 0; ++i) {
		action = &_actionsTable[i];
		if (action->key == actionKey) {
			skip = 1;
			if (action->testFlag1Num != 0) {
				if (action->testFlag1Num < 500) {
					if (_flagsTable[action->testFlag1Num] != action->testFlag1Value) {
						skip = 0;
					}
				} else if (_inventoryItemsState[action->testFlag1Num - 500] != action->testFlag1Value) {
					skip = 0;
				}
				debug(3, "updateCharPosition() flag1 %d value %d", action->testFlag1Num, action->testFlag1Value);
			}
			if (action->testFlag2Num != 0) {
				if (action->testFlag2Num < 500) {
					if (_flagsTable[action->testFlag2Num] != action->testFlag2Value) {
						skip = 0;
					}
				} else if (_inventoryItemsState[action->testFlag2Num - 500] != action->testFlag2Value) {
					skip = 0;
				}
				debug(3, "updateCharPosition() flag2 %d value %d", action->testFlag2Num, action->testFlag2Value);
			}
		}
	}
	if (skip == 0) {
		playSpeechForAction(_currentActionVerb);
		_currentActionVerb = 0;
		return;
	}
	assert(action);
	if (action->speech != 6) {
		if (action->speech < 100) {
			_spriteAnimationFrameIndex = _spriteAnimationsTable[action->speech].firstFrameIndex;
			_currentSpriteAnimationLength = _spriteAnimationsTable[action->speech].numParts;
			_mirroredDrawing = action->flipX;
			_characterFacingDirection = 5;
			_mainLoopCounter2 = 0;
		} else {
			_backgroundSpriteCurrentAnimation = action->speech - 100;
			_backgroundSpriteCurrentFrame = 0;
			_mirroredDrawing = 0;
		}
	}
	_pendingActionDelay = action->delay;
	_charPositionFlagNum = action->setFlagNum;
	_charPositionFlagValue = action->setFlagValue;
	_pendingActionIndex = action->index;
	_characterSoundFxDelayCounter = action->fxDelay;
	_characterSoundFxNum = action->fxNum;
	_previousActionVerb = _currentActionVerb;
	_currentActionVerb = 0;
}

void TuckerEngine::updateFlagsForCharPosition() {
	if (_pendingActionDelay != 0) {
		--_pendingActionDelay;
		if (_pendingActionDelay > 0) {
			return;
		}
		switch (_previousActionVerb) {
		case 3:
		case 4:
		case 8:
		case 2:
			debug(3, "updateFlagsForCharPosition() set flag %d value %d", _charPositionFlagNum, _charPositionFlagValue);
			_flagsTable[_charPositionFlagNum] = _charPositionFlagValue;
			break;
		case 6:
			if (_charPositionFlagValue == 1) {
				addObjectToInventory(_charPositionFlagNum);
				_forceRedrawPanelItems = true;
			}
			break;
		}
		if (_pendingActionIndex > 0) {
			_nextAction = _pendingActionIndex;
		}
	}
}

void TuckerEngine::fadeOutPalette(int colorsCount) {
	uint8 pal[256 * 3];
	_system->getPaletteManager()->grabPalette(pal, 0, colorsCount);
	for (int color = 0; color < colorsCount; ++color) {
		for (int i = 0; i < 3; ++i) {
			const int c = int(pal[color * 3 + i]) + kFadePaletteStep * 3;
			pal[color * 3 + i] = MIN<int>(c, _currentPalette[color * 3 + i]);
		}
	}
	_system->getPaletteManager()->setPalette(pal, 0, colorsCount);
	_system->updateScreen();
}

void TuckerEngine::fadeInPalette(int colorsCount) {
	uint8 pal[256 * 3];
	_system->getPaletteManager()->grabPalette(pal, 0, colorsCount);
	for (int color = 0; color < colorsCount; ++color) {
		for (int i = 0; i < 3; ++i) {
			const int c = int(pal[color * 3 + i]) - kFadePaletteStep * 3;
			pal[color * 3 + i] = MAX<int>(c, 0);
		}
	}
	_system->getPaletteManager()->setPalette(pal, 0, colorsCount);
	_system->updateScreen();
}

void TuckerEngine::fadePaletteColor(int color, int step) {
	uint8 rgb[4];
	_system->getPaletteManager()->grabPalette(rgb, color, 1);
	for (int i = 0; i < 3; ++i) {
		const int c = _currentPalette[color * 3 + i] + step * 4;
		rgb[i] = MIN(c, 255);
	}
	_system->getPaletteManager()->setPalette(rgb, color, 1);
}

void TuckerEngine::setBlackPalette() {
	uint8 pal[256 * 3];
	memset(pal, 0, sizeof(pal));
	_system->getPaletteManager()->setPalette(pal, 0, 256);
}

void TuckerEngine::updateCursor() {
	setCursorNum(0);
	if (_backgroundSpriteCurrentAnimation == -1 && _panelLockedFlag == 0 && _selectedObject.locationObject_locationNum > 0) {
		_selectedObject.locationObject_locationNum = 0;
	}
	if (_locationMaskType > 0 || _selectedObject.locationObject_locationNum > 0 || _pendingActionDelay > 0) {
		return;
	}
	if (_rightMouseButtonPressed) {
		if (!_updateCursorFlag) {
			++_actionVerb;
			if (_actionVerb > 8) {
				_actionVerb = 0;
			}
			_updateCursorFlag = true;
			_actionVerbLocked = 1;
			_actionRequiresTwoObjects = false;
		}
	} else {
		_updateCursorFlag = false;
	}
	if (_actionVerbLocked == 0) {
		setActionVerbUnderCursor();
		if (_actionVerb == 0 && _locationNum == 63) {
			_actionVerb = 8;
		}
	}
	_selectedObjectNum = 0;
	_selectedObjectType = 0;
	int num = setCharacterUnderCursor();
	if (_selectedObjectType == 0) {
		num = setLocationAnimationUnderCursor();
	}
	if (_selectedObjectType > 0) {
		_selectedObjectNum = num;
	} else {
		num = getObjectUnderCursor();
		if (num > -1) {
			_selectedObjectNum = _locationObjectsTable[num].textNum;
		}
	}
	handleMouseClickOnInventoryObject();
	if (_actionVerb == 2 && _selectedObjectType != 2) {
		_selectedObjectNum = 0;
		_selectedObjectType = 0;
	} else if (_actionVerb == 5 && _selectedObjectType != 3 && !_actionRequiresTwoObjects) {
		_selectedObjectNum = 0;
		_selectedObjectType = 0;
	}
	if (_actionVerbLocked == 0 && _selectedObjectType == 2 && _selectedObjectNum != 21) {
		_actionVerb = 2;
	}
	if (!_actionRequiresTwoObjects) {
		_actionObj1Num = _selectedObjectNum;
		_actionObj1Type = _selectedObjectType;
		_actionObj2Num = 0;
		_actionObj2Type = 0;
	} else if (_actionObj1Num == _selectedObjectNum && _actionObj1Type == _selectedObjectType) {
		_selectedObjectNum = 0;
		_selectedObjectType = 0;
		_actionObj2Num = 0;
		_actionObj2Type = 0;
	} else {
		_actionObj2Num = _selectedObjectNum;
		_actionObj2Type = _selectedObjectType;
	}
	if (!_leftMouseButtonPressed) {
		_mouseClick = 0;
	}
	if (_leftMouseButtonPressed && _mouseClick == 0) {
		_fadedPanel = 0;
		_mouseClick = 1;
		clearItemsGfx();
		drawInfoString();
		if (_mousePosY >= 150 && _mousePosX < 212) {
			if (_mousePosX < 200) {
				setActionVerbUnderCursor();
				_actionVerbLocked = 1;
				_actionRequiresTwoObjects = false;
			} else if (_mousePosY < 175) {
				moveDownInventoryObjects();
			} else {
				moveUpInventoryObjects();
			}
		} else {
			if (_selectedObjectType == 3) {
				setActionForInventoryObject();
			} else if (_actionVerb != 0) {
				_actionVerbLocked = 0;
				setActionState();
			} else if (_actionObj1Num == 261 || (_actionObj1Num == 205 && _flagsTable[143] == 0)) {
				_actionVerbLocked = 0;
				setActionState();
			} else {
				_actionVerbLocked = 0;
				_actionRequiresTwoObjects = false;
				_currentActionVerb = 0;
				setSelectedObjectKey();
			}
		}
	}
}

void TuckerEngine::stopSounds() {
	for (int i = 0; i < _locationSoundsCount; ++i) {
		stopSound(i);
	}
	for (int i = 0; i < _locationMusicsCount; ++i) {
		stopMusic(i);
	}
}

void TuckerEngine::playSounds() {
	for (int i = 0; i < _locationSoundsCount; ++i) {
		if (_locationSoundsTable[i].type == 1 || _locationSoundsTable[i].type == 2 || _locationSoundsTable[i].type == 5 ||
			(_locationSoundsTable[i].type == 7 && _flagsTable[_locationSoundsTable[i].flagNum] == _locationSoundsTable[i].flagValueStartFx)) {
			startSound(_locationSoundsTable[i].offset, i, _locationSoundsTable[i].volume);
		}
	}
	for (int i = 0; i < _locationMusicsCount; ++i) {
		if (_locationMusicsTable[i].flag > 0) {
			startMusic(_locationMusicsTable[i].offset, i, _locationMusicsTable[i].volume);
		}
	}
}

void TuckerEngine::updateCharactersPath() {
	if (_panelLockedFlag == 0) {
		return;
	}
	if (_backgroundSpriteCurrentAnimation != -1 && _locationNum != 25) {
		if (_xPosCurrent == _selectedObject.xPos && _yPosCurrent == _selectedObject.yPos) {
			_locationMaskCounter = 1;
			_panelLockedFlag = 0;
		}
		return;
	}
	int xPos = _xPosCurrent;
	int yPos = _yPosCurrent;
	if (_characterFacingDirection == 5) {
		_characterPrevFacingDirection = 5;
	}
	int flag = 0;
	if (_yPosCurrent > _selectedObject.yPos) {
		if (testLocationMask(_xPosCurrent, _yPosCurrent - 1) == 1) {
			--_yPosCurrent;
			_characterFacingDirection = 4;
			flag = 1;
		}
	} else if (_yPosCurrent < _selectedObject.yPos) {
		if (testLocationMask(_xPosCurrent, _yPosCurrent + 1) == 1) {
			++_yPosCurrent;
			_characterFacingDirection = 2;
			flag = 1;
		}
	}
	if (_xPosCurrent > _selectedObject.xPos) {
		if (testLocationMask(_xPosCurrent - 1, _yPosCurrent) == 1) {
			--_xPosCurrent;
			_characterFacingDirection = 3;
			_characterBackFrontFacing = 0;
			flag = 1;
		}
	} else if (_xPosCurrent < _selectedObject.xPos) {
		if (testLocationMask(_xPosCurrent + 1, _yPosCurrent) == 1) {
			++_xPosCurrent;
			_characterFacingDirection = 1;
			_characterBackFrontFacing = 1;
			flag = 1;
		}
	}
	if (flag == 0) {
		if (_selectedObjectLocationMask == 1) {
			_selectedObjectLocationMask = 0;
			_selectedObject.xPos = _selectedObject.xDefaultPos;
			_selectedObject.yPos = _selectedObject.yDefaultPos;
		} else {
			_panelLockedFlag = 0;
			_characterFacingDirection = 0;
			if (_xPosCurrent == _selectedObject.xPos && _yPosCurrent == _selectedObject.yPos) {
				_locationMaskCounter = 1;
			}
		}
	}
	if (_locationNum == 25) {
		if ((_backgroundSpriteCurrentAnimation != 3 || _characterBackFrontFacing != 0) && (_backgroundSpriteCurrentAnimation != 6 || _characterBackFrontFacing != 1)) {
			_xPosCurrent = xPos;
			_yPosCurrent = yPos;
			return;
		}
	}
	if (_xPosCurrent != _selectedObject.xPos || _yPosCurrent != _selectedObject.yPos) {
		return;
	}
	if (_selectedObjectLocationMask != 0) {
		return;
	}
	_locationMaskCounter = 1;
	_panelLockedFlag = 0;
	_locationMaskIgnore = 0;
	if (_characterPrevFacingDirection <= 0 || _characterPrevFacingDirection >= 5) {
		return;
	}
	if (_selectedObject.locationObject_locationNum == 0) {
		_characterFacingDirection = 5;
		while (_spriteAnimationFramesTable[_spriteAnimationFrameIndex] != 999) {
			++_spriteAnimationFrameIndex;
		}
		++_spriteAnimationFrameIndex;
	}
}

void TuckerEngine::setSoundVolumeDistance() {
	int w = ABS(_xPosCurrent - _currentFxDist);
	int d = w * _currentFxScale / 10;
	int volume = (d > _currentFxVolume) ? 0 : _currentFxVolume - d;
	setVolumeSound(_currentFxIndex, volume);
}

void TuckerEngine::updateData3DrawFlag() {
	for (int i = 0; i < _locationAnimationsCount; ++i) {
		LocationAnimation *a = &_locationAnimationsTable[i];
		if (a->flagNum > 0 && a->flagValue != _flagsTable[a->flagNum]) {
			a->drawFlag = 0;
		} else if (a->getFlag == 0) {
			a->drawFlag = 1;
		} else {
			a->drawFlag = (_inventoryItemsState[a->inventoryNum] == 0) ? 1 : 0;
		}
	}
}

void TuckerEngine::updateData3() {
	updateData3DrawFlag();
	for (int i = 0; i < _locationAnimationsCount; ++i) {
		LocationAnimation *a = &_locationAnimationsTable[i];
		if (a->animLastCounter != 0 && a->drawFlag != 0) {
			if (a->animLastCounter == a->animCurrentCounter) {
				a->animCurrentCounter = a->animInitCounter;
			} else {
				++a->animCurrentCounter;
			}
			const int index = a->animCurrentCounter;
			if (_staticData3Table[index] == 998) {
				_flagsTable[_staticData3Table[index + 1]] = _staticData3Table[index + 2];
				a->animCurrentCounter = a->animInitCounter;
				a->drawFlag = 0;
			}
			if (_locationNum == 24 && i == 0) {
				// workaround bug #2872385: update fish animation sequence for correct
				// position in aquarium.
				if (a->animInitCounter == 505 && a->animCurrentCounter == 513) {
					a->animCurrentCounter = 525;
				}
			}
			a->graphicNum = _staticData3Table[a->animCurrentCounter];
		}
	}
	updateData3DrawFlag();
}

void TuckerEngine::updateSfxData3_1() {
	for (int i = 0; i < _locationSoundsCount; ++i) {
		LocationSound *s = &_locationSoundsTable[i];
		if ((s->type == 6 || s->type == 7) && s->updateType == 1) {
			for (int j = 0; j < _spritesCount; ++j) {
				if (_spritesTable[j].animationFrame == s->startFxSpriteNum && _spritesTable[j].state == s->startFxSpriteState) {
					if (s->type == 7) {
						_flagsTable[s->flagNum] = s->flagValueStartFx;
					}
					startSound(s->offset, i, s->volume);
				} else if (s->type == 7) {
					if (_spritesTable[j].animationFrame == s->stopFxSpriteNum && _spritesTable[j].state == s->stopFxSpriteState) {
						_flagsTable[s->flagNum] = s->flagValueStopFx;
						stopSound(i);
					}
				}
			}
		}
	}
}

void TuckerEngine::updateSfxData3_2() {
	for (int i = 0; i < _locationSoundsCount; ++i) {
		LocationSound *s = &_locationSoundsTable[i];
		if ((s->type == 6 || s->type == 7) && s->updateType == 0) {
			if (s->startFxSpriteNum == _backgroundSpriteCurrentFrame && s->startFxSpriteState == _backgroundSpriteCurrentAnimation) {
				if (s->type == 7) {
					_flagsTable[s->flagNum] = s->flagValueStartFx;
				}
				startSound(s->offset, i, s->volume);
			} else if (s->type == 7) {
				if (s->stopFxSpriteNum == _backgroundSpriteCurrentFrame && s->stopFxSpriteState == _backgroundSpriteCurrentAnimation) {
					_flagsTable[s->flagNum] = s->flagValueStopFx;
					stopSound(i);
				}
			}
		}
	}
}

void TuckerEngine::saveOrLoad() {
	if (!_leftMouseButtonPressed) {
		_mouseClick = 0;
	}
	if (_currentSaveLoadGameState > 0) {
		drawSpeechText(_scrollOffset + 120, 170, _infoBarBuf, _saveOrLoadGamePanel + 19, 102);
		int len = getStringWidth(_saveOrLoadGamePanel + 19, _infoBarBuf);
		drawStringInteger(_currentSaveLoadGameState, len / 2 + 128, 160, 2);
	} else {
		drawSpeechText(_scrollOffset + 120, 170, _infoBarBuf, 21, 102);
	}
	if (_leftMouseButtonPressed && _mouseClick == 0) {
		_mouseClick = 1;
		if (_mousePosX > 228 && _mousePosX < 240 && _mousePosY > 154 && _mousePosY < 170) {
			if (_currentSaveLoadGameState < 99) {
				++_currentSaveLoadGameState;
				_forceRedrawPanelItems = true;
			}
			return;
		}
		if (_mousePosX > 228 && _mousePosX < 240 && _mousePosY > 170 && _mousePosY < 188) {
			if (_currentSaveLoadGameState > 1) {
				--_currentSaveLoadGameState;
				_forceRedrawPanelItems = true;
			}
			return;
		}
		if (_mousePosX > 244 && _mousePosX < 310 && _mousePosY > 170 && _mousePosY < 188) {
			_forceRedrawPanelItems = true;
			_panelState = 2;
			return;
		}
		if (_mousePosX > 260 && _mousePosX < 290 && _mousePosY > 152 && _mousePosY < 168) {
			if (_saveOrLoadGamePanel == 1) {
				saveGameState(_currentSaveLoadGameState, "");
			} else if (_currentSaveLoadGameState > 0) {
				loadGameState(_currentSaveLoadGameState);
			}
			_forceRedrawPanelItems = true;
			_panelState = 0;
			setCursorType(0);
			return;
		}
	}
}

void TuckerEngine::handleMouseOnPanel() {
	if (!_leftMouseButtonPressed) {
		_mouseClick = 0;
	}
	if (_leftMouseButtonPressed && _mouseClick == 0) {
		_mouseClick = 1;
		if (_mousePosY < 160 || _mousePosY > 176) {
			return;
		}
		if (_mousePosX < 45 || _mousePosX > 275) {
			return;
		}
		if (_mousePosX < 96) {
			_saveOrLoadGamePanel = 0;
			_forceRedrawPanelItems = true;
			_panelState = 3;
		} else if (_mousePosX < 158) {
			_saveOrLoadGamePanel = 1;
			_forceRedrawPanelItems = true;
			_panelState = 3;
		} else if (_mousePosX < 218) {
			_forceRedrawPanelItems = true;
			_panelState = 0;
			setCursorType(0);
		} else {
			_quitGame = true;
		}
	}
}

void TuckerEngine::switchPanelType() {
	if (_inputKeys[kInputKeyToggleInventory]) {
		_inputKeys[kInputKeyToggleInventory] = false;
		if (_panelState == 0 && _switchPanelFlag == 0) {
			_switchPanelFlag = 1;
			_switchPanelCounter = 1;
			return;
		}
	}
	if (_switchPanelFlag == 0) {
		return;
	}
	if (_switchPanelFlag == 1) {
		if (_switchPanelCounter == 25) {
			if (_panelNum == 0) {
				_panelNum = 1;
			} else {
				_panelNum = 0;
			}
			_switchPanelFlag = 2;
			loadPanel();
			_forceRedrawPanelItems = true;
		} else {
			++_switchPanelCounter;
		}
	} else {
		--_switchPanelCounter;
		if (_switchPanelCounter == 0) {
			_switchPanelFlag = 0;
		}
	}
}

void TuckerEngine::redrawPanelOverBackground() {
	const uint8 *src = _itemsGfxBuf;
	uint8 *dst = _locationBackgroundGfxBuf + 640 * 140 + _scrollOffset;
	for (int y = 0; y < 10; ++y) {
		memcpy(dst, src, 320);
		src += 320;
		dst += 640;
	}
	for (int y = 0; y < _switchPanelCounter; ++y) {
		for (int x = 0; x < 320; ++x) {
			dst[x] = 0;
		}
		dst += 640;
	}
	int y2 = 50 - _switchPanelCounter * 2;
	for (int y = 0; y < y2; ++y) {
		int i = y * 50 / y2;
		memcpy(dst, src + i * 320, 320);
		dst += 640;
	}
	for (int y = 0; y < _switchPanelCounter; ++y) {
		for (int x = 0; x < 320; ++x) {
			dst[x] = 0;
		}
		dst += 640;
	}
	if (_conversationOptionsCount > 0) {
		drawConversationTexts();
	}
	addDirtyRect(_scrollOffset, 140, 320, 60);
}

void TuckerEngine::drawConversationTexts() {
	int y = 141;
	int flag = 0;
	for (int i = 0; i <  _conversationOptionsCount; ++i) {
		int color = 108;
		if ((_mousePosY > y && _mousePosY < y + 11) || _nextTableToLoadIndex == i) {
			color = 106;
		}
		drawSpeechText(0, y, _characterSpeechDataPtr, _instructionsActionsTable[i], color);
		if (_mousePosY > y && _mousePosY < _conversationOptionLinesCount * 10 + y + 1) {
			_nextTableToLoadIndex = i;
			flag = 1;
		}
		y += _conversationOptionLinesCount * 10;
	}
	if (flag == 0) {
		_nextTableToLoadIndex = -1;
	}
}

void TuckerEngine::updateScreenScrolling() {
	int scrollPrevOffset = _scrollOffset;
	if (_locationWidthTable[_locationNum] != 2) {
		_scrollOffset = 0;
	} else if (_validInstructionId) {
		_scrollOffset = _xPosCurrent - 200;
	} else if (_locationNum == 16 && _backgroundSpriteCurrentAnimation == 6 && _scrollOffset + 200 < _xPosCurrent) {
		++_scrollOffset;
		if (_scrollOffset > 320) {
			_scrollOffset = 320;
		}
	} else if (_scrollOffset + 120 > _xPosCurrent) {
		_scrollOffset = _xPosCurrent - 120;
		if (_scrollOffset < 0) {
			_scrollOffset = 0;
		}
	} else if (_scrollOffset + 200 < _xPosCurrent) {
		_scrollOffset = _xPosCurrent - 200;
		if (_scrollOffset > 320) {
			_scrollOffset = 320;
		}
	}
	if (scrollPrevOffset != _scrollOffset) {
		_fullRedraw = true;
	}
}

void TuckerEngine::updateGameHints() {
	if (_gameHintsIndex == 0 && _flagsTable[3] > 0) {
		_gameHintsIndex = 1;
		_gameHintsCounter = 0;
		_gameHintsDisplayText = 0;
	} else if (_gameHintsIndex == 1 && _flagsTable[12] > 0) {
		_gameHintsIndex = 2;
		_gameHintsCounter = 0;
		_gameHintsDisplayText = 0;
	} else if (_gameHintsIndex == 2 && _flagsTable[20] > 0) {
		_gameHintsIndex = 3;
		_gameHintsCounter = 0;
		_gameHintsDisplayText = 0;
	} else if (_gameHintsIndex == 3 && _flagsTable[9] > 0) {
		_gameHintsIndex = 4;
		_gameHintsCounter = 0;
		_gameHintsDisplayText = 0;
	} else if (_gameHintsIndex == 4 && _flagsTable[23] > 0) {
		_gameHintsIndex = 5;
		_gameHintsCounter = 0;
		_gameHintsDisplayText = 0;
	} else if (_flagsTable[19] > 0) {
		_gameHintsIndex = 6;
		_gameHintsCounter = 0;
		_gameHintsDisplayText = 0;
	}
	++_gameHintsCounter;
	if (_gameHintsCounter > 1500) {
		_gameHintsDisplayText = 1;
	}
}

void TuckerEngine::startCharacterSounds() {
	if (_characterSoundFxDelayCounter != 0) {
		--_characterSoundFxDelayCounter;
		if (_characterSoundFxDelayCounter <= 0) {
			startSound(_locationSoundsTable[_characterSoundFxNum].offset, _characterSoundFxNum, _locationSoundsTable[_characterSoundFxNum].volume);
		}
	}
}

void TuckerEngine::updateSoundsTypes3_4() {
	if (isSoundPlaying(0)) {
		return;
	}
	for (int i = 0; i < _locationSoundsCount; ++i) {
		switch (_locationSoundsTable[i].type) {
		case 3:
			if (getRandomNumber() >= 32300) {
				startSound(_locationSoundsTable[i].offset, 0, _locationSoundsTable[i].volume);
				return;
			}
			break;
		case 4:
			if (getRandomNumber() >= 32763) {
				startSound(_locationSoundsTable[i].offset, 0, _locationSoundsTable[i].volume);
				return;
			}
			break;
		}
	}
}

void TuckerEngine::drawData3() {
	for (int i = 0; i < _locationAnimationsCount; ++i) {
		if (_locationAnimationsTable[i].drawFlag != 0) {
			int num = _locationAnimationsTable[i].graphicNum;
			const Data *d = &_dataTable[num];
			Graphics::decodeRLE(_locationBackgroundGfxBuf + d->yDest * 640 + d->xDest, _data3GfxBuf + d->sourceOffset, d->xSize, d->ySize);
			addDirtyRect(d->xDest, d->yDest, d->xSize, d->ySize);
		}
	}
}

void TuckerEngine::execData3PreUpdate() {
	switch (_locationNum) {
	case 1:
		execData3PreUpdate_locationNum1();
		break;
	case 2:
		execData3PreUpdate_locationNum2();
		break;
	case 3:
		execData3PreUpdate_locationNum3();
		break;
	case 4:
		execData3PreUpdate_locationNum4();
		break;
	case 6:
		execData3PreUpdate_locationNum6();
		break;
	case 9:
		execData3PreUpdate_locationNum9();
		break;
	case 10:
		execData3PreUpdate_locationNum10();
		break;
	case 12:
		execData3PreUpdate_locationNum12();
		break;
	case 13:
		execData3PreUpdate_locationNum13();
		break;
	case 14:
		execData3PreUpdate_locationNum14();
		break;
	case 15:
		execData3PreUpdate_locationNum15();
		break;
	case 16:
		execData3PreUpdate_locationNum16();
		break;
	case 19:
		execData3PreUpdate_locationNum19();
		break;
	case 21:
		execData3PreUpdate_locationNum21();
		break;
	case 22:
		execData3PreUpdate_locationNum22();
		break;
	case 24:
		execData3PreUpdate_locationNum24();
		break;
	case 25:
		execData3PreUpdate_locationNum25();
		break;
	case 26:
		execData3PreUpdate_locationNum26();
		break;
	case 27:
		execData3PreUpdate_locationNum27();
		break;
	case 28:
		execData3PreUpdate_locationNum28();
		break;
	case 29:
		execData3PreUpdate_locationNum29();
		break;
	case 30:
		execData3PreUpdate_locationNum30();
		break;
	case 31:
		execData3PreUpdate_locationNum31();
		break;
	case 32:
		execData3PreUpdate_locationNum32();
		break;
	case 33:
		execData3PreUpdate_locationNum33();
		break;
	case 34:
		execData3PreUpdate_locationNum34();
		break;
	case 35:
		execData3PreUpdate_locationNum35();
		break;
	case 36:
		execData3PreUpdate_locationNum36();
		break;
	case 38:
		execData3PreUpdate_locationNum38();
		break;
	case 41:
		execData3PreUpdate_locationNum41();
		break;
	case 42:
		execData3PreUpdate_locationNum42();
		break;
	case 43:
		execData3PreUpdate_locationNum43();
		break;
	case 44:
		execData3PreUpdate_locationNum44();
		break;
	case 49:
		execData3PreUpdate_locationNum49();
		break;
	case 52:
		execData3PreUpdate_locationNum52();
		break;
	case 53:
		execData3PreUpdate_locationNum53();
		break;
	case 57:
		execData3PreUpdate_locationNum57();
		break;
	case 58:
		execData3PreUpdate_locationNum58();
		break;
	case 61:
		execData3PreUpdate_locationNum61();
		break;
	case 63:
		execData3PreUpdate_locationNum63();
		break;
	case 64:
		execData3PreUpdate_locationNum64();
		break;
	case 65:
		execData3PreUpdate_locationNum65();
		break;
	case 66:
		execData3PreUpdate_locationNum66();
		break;
	case 70:
		execData3PreUpdate_locationNum70();
		break;
	}
}

void TuckerEngine::execData3PostUpdate() {
	switch (_locationNum) {
	case 1:
		execData3PostUpdate_locationNum1();
		break;
	case 6:
		execData3PostUpdate_locationNum6();
		break;
	case 8:
		execData3PostUpdate_locationNum8();
		break;
	case 9:
		execData3PostUpdate_locationNum9();
		break;
	case 14:
		execData3PostUpdate_locationNum14();
		break;
	case 21:
		execData3PostUpdate_locationNum21();
		break;
	case 24:
		execData3PostUpdate_locationNum24();
		break;
	case 27:
		execData3PostUpdate_locationNum27();
		break;
	case 28:
		execData3PostUpdate_locationNum28();
		break;
	case 32:
		execData3PostUpdate_locationNum32();
		break;
	case 60:
		execData3PostUpdate_locationNum60();
		break;
	case 66:
		execData3PostUpdate_locationNum66();
		break;
	}
}

void TuckerEngine::drawBackgroundSprites() {
	if (_backgroundSpriteDataPtr && _backgroundSpriteCurrentFrame != 0 && _backgroundSpriteCurrentFrame <= _backgroundSpriteLastFrame) {
		int frameOffset = READ_LE_UINT24(_backgroundSpriteDataPtr + _backgroundSpriteCurrentFrame * 4);
		int srcW = READ_LE_UINT16(_backgroundSpriteDataPtr + frameOffset);
		int srcH = READ_LE_UINT16(_backgroundSpriteDataPtr + frameOffset + 2);
		int srcX = READ_LE_UINT16(_backgroundSpriteDataPtr + frameOffset + 8);
		int srcY = READ_LE_UINT16(_backgroundSpriteDataPtr + frameOffset + 10);
		if (_locationNum == 22 && _backgroundSpriteCurrentAnimation > 1) {
			srcY += _mainSpritesBaseOffset;
		}
		if (_locationNum == 29 && _backgroundSpriteCurrentAnimation == 3) {
			srcX += 228;
		} else if (_locationNum == 58 && _backgroundSpriteCurrentAnimation == 1) {
			srcX += 100;
		} else if (_xPosCurrent > 320 && _xPosCurrent < 640) {
			srcX += 320;
		}
		srcX += _backgroundSprOffset;
		Graphics::decodeRLE_248(_locationBackgroundGfxBuf + srcY * 640 + srcX, _backgroundSpriteDataPtr + frameOffset + 12, srcW, srcH, 0, _locationHeightTable[_locationNum], false);
		addDirtyRect(srcX, srcY, srcW, srcH);
	}
}

void TuckerEngine::drawCurrentSprite() {
	// Workaround original game glitch: skip first bud frame drawing when entering location (tracker item #2597763)
	if ((_locationNum == 17 || _locationNum == 18) && _currentSpriteAnimationFrame == 16) {
		return;
	}
	// Workaround original game glitch: location 14 contains some colors from [0xE0-0xF8] in a walkable area (tracker item #3106542)
	const bool color248Only = (_locationNum == 14);
	SpriteFrame *chr = &_spriteFramesTable[_currentSpriteAnimationFrame];
	int yPos = _yPosCurrent + _mainSpritesBaseOffset - 54 + chr->yOffset;
	int xPos = _xPosCurrent;
	if (_mirroredDrawing == 0) {
		xPos += chr->xOffset - 14;
	} else {
		xPos -= chr->xSize + chr->xOffset - 14;
	}
	Graphics::decodeRLE_248(_locationBackgroundGfxBuf + yPos * 640 + xPos, _spritesGfxBuf + chr->sourceOffset, chr->xSize, chr->ySize,
		chr->yOffset, _locationHeightTable[_locationNum], _mirroredDrawing != 0, color248Only);
	addDirtyRect(xPos, yPos, chr->xSize, chr->ySize);
	if (_currentSpriteAnimationLength > 1) {
		SpriteFrame *chr2 = &_spriteFramesTable[_currentSpriteAnimationFrame2];
		yPos = _yPosCurrent + _mainSpritesBaseOffset - 54 + chr2->yOffset;
		xPos = _xPosCurrent;
		if (_mirroredDrawing == 0) {
			xPos += chr2->xOffset - 14;
		} else {
			xPos -= chr2->xSize + chr2->xOffset - 14;
		}
		Graphics::decodeRLE_248(_locationBackgroundGfxBuf + yPos * 640 + xPos, _spritesGfxBuf + chr2->sourceOffset, chr2->xSize, chr2->ySize,
			chr2->yOffset, _locationHeightTable[_locationNum], _mirroredDrawing != 0, color248Only);
		addDirtyRect(xPos, yPos, chr2->xSize, chr2->ySize);
	}
}

void TuckerEngine::setVolumeSound(int index, int volume) {
	if (volume < 0) {
		volume = 0;
	}
	_mixer->setChannelVolume(_sfxHandles[index], volume * Audio::Mixer::kMaxChannelVolume / kMaxSoundVolume);
}

void TuckerEngine::setVolumeMusic(int index, int volume) {
	if (volume < 0) {
		volume = 0;
	}
	_mixer->setChannelVolume(_musicHandles[index], volume * Audio::Mixer::kMaxChannelVolume / kMaxSoundVolume);
}

void TuckerEngine::startSound(int offset, int index, int volume) {
	bool loop = (_locationSoundsTable[index].type == 2 || _locationSoundsTable[index].type == 5 || _locationSoundsTable[index].type == 7);
	loadSound(Audio::Mixer::kSFXSoundType, _locationSoundsTable[index].num, volume, loop, &_sfxHandles[index]);
}

void TuckerEngine::stopSound(int index) {
	_mixer->stopHandle(_sfxHandles[index]);
}

bool TuckerEngine::isSoundPlaying(int index) {
	return _mixer->isSoundHandleActive(_sfxHandles[index]);
}

void TuckerEngine::startMusic(int offset, int index, int volume) {
	bool loop = (_locationMusicsTable[index].flag == 2);
	loadSound(Audio::Mixer::kMusicSoundType, _locationMusicsTable[index].num, volume, loop, &_musicHandles[index]);
}

void TuckerEngine::stopMusic(int index) {
	_mixer->stopHandle(_musicHandles[index]);
}

void TuckerEngine::startSpeechSound(int num, int volume) {
	loadSound(Audio::Mixer::kSpeechSoundType, num, volume, false, &_speechHandle);
}

void TuckerEngine::stopSpeechSound() {
	_mixer->stopHandle(_speechHandle);
}

bool TuckerEngine::isSpeechSoundPlaying() {
	return _mixer->isSoundHandleActive(_speechHandle);
}

void TuckerEngine::rememberSpeechSound() {
	for (int i = 4; i > 0; --i) {
		_speechHistoryTable[i] = _speechHistoryTable[i - 1];
	}
	_speechHistoryTable[0] = _partNum * 3000 + _ptTextOffset + _speechSoundNum - 3000;
}

void TuckerEngine::redrawPanelItems() {
	const uint8 *src = 0;
	uint8 *dst = 0;
	int sz = 0;
	if (_forceRedrawPanelItems || (_redrawPanelItemsCounter != 0 && _panelState == 0)) {
		_forceRedrawPanelItems = false;
		if (_redrawPanelItemsCounter > 0) {
			--_redrawPanelItemsCounter;
		}
		switch (_panelState) {
		case 0:
			src = _panelGfxBuf;
			dst = _itemsGfxBuf + 3200;
			sz = 16000;
			break;
		case 1:
			src = _panelGfxBuf + 16320;
			dst = _itemsGfxBuf;
			sz = 19200;
			break;
		case 2:
			src = _panelGfxBuf + 16320;
			dst = _itemsGfxBuf;
			sz = 19200;
			memcpy(dst, src, sz);
			src = _panelGfxBuf + 55040;
			dst = _itemsGfxBuf + 6400;
			sz = 5120;
			break;
		case 3:
			src = _panelGfxBuf + 35200;
			dst = _itemsGfxBuf;
			sz = 19200;
			break;
		}
		memcpy(dst, src, sz);
		if (_panelState == 0) {
			redrawPanelItemsHelper();
		}
	}
}

void TuckerEngine::redrawPanelItemsHelper() {
	const int k = (_redrawPanelItemsCounter / 4) - ((_redrawPanelItemsCounter / 8) * 2);
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (i * 3 + j + _inventoryObjectsOffset >= _inventoryObjectsCount) {
				continue;
			}
			if (i * 3 + j + _inventoryObjectsOffset == _lastInventoryObjectIndex && k != 0) {
				continue;
			}
			const int obj = _inventoryObjectsList[i * 3 + j + _inventoryObjectsOffset];
			const uint8 *src = _panelObjectsGfxBuf + _panelObjectsOffsetTable[obj];
			uint8 *dst = _itemsGfxBuf + 3412 + i * 8320 + j * 34;
			Graphics::decodeRLE_320(dst, src, 32, 24);
		}
	}
}

void TuckerEngine::drawSprite(int num) {
	Sprite *s = &_spritesTable[num];
	if (s->animationFrame <= s->firstFrame && s->animationFrame > 0 && s->state != -1) {
		const uint8 *p = s->animationData;
		if (!p) {
			return;
		}
		int frameOffset = READ_LE_UINT24(p + s->animationFrame * 4);
		int srcW = READ_LE_UINT16(p + frameOffset);
		int srcH = READ_LE_UINT16(p + frameOffset + 2);
		int srcX = READ_LE_UINT16(p + frameOffset + 8);
		int srcY = READ_LE_UINT16(p + frameOffset + 10);
		int xPos = s->gfxBackgroundOffset + srcX;
		if (xPos < 600 && (_scrollOffset + 320 < xPos || _scrollOffset - srcW > xPos)) {
			return;
		}
		s->xSource = srcX;
		s->gfxBackgroundOffset += s->backgroundOffset;
		uint8 *dstPtr = _locationBackgroundGfxBuf + srcY * 640 + xPos;
		const uint8 *srcPtr = p + frameOffset + 12;
		switch (s->colorType) {
		case 0:
			Graphics::decodeRLE(dstPtr, srcPtr, srcW, srcH);
			break;
		case 99:
			Graphics::decodeRLE_224(dstPtr, srcPtr, srcW, srcH);
			break;
		default:
			Graphics::decodeRLE_248(dstPtr, srcPtr, srcW, srcH, 0, s->yMaxBackground, s->flipX != 0);
			break;
		}
		const int xR = srcX + (s->gfxBackgroundOffset % 640);
		const int yR = srcY + (s->gfxBackgroundOffset / 640);
		addDirtyRect(xR, yR, srcW, srcH);
	}
}

void TuckerEngine::clearItemsGfx() {
	memset(_itemsGfxBuf, 0, 3200);
}

void TuckerEngine::drawPausedInfoBar() {
	int len = getStringWidth(36, _infoBarBuf);
	int x = 159 - len / 2;
	drawItemString(326 + x, 36, _infoBarBuf);
}

const uint8 *TuckerEngine::getStringBuf(int type) const {
	const uint8 *p = 0;
	switch (type) {
	case 0:
		p = _data5Buf;
		break;
	case 1:
		p = _bgTextBuf;
		break;
	case 2:
		p = _charNameBuf;
		break;
	case 3:
		p = _objTxtBuf;
		break;
	}
	return p;
}

void TuckerEngine::drawInfoString() {
	const uint8 *infoStrBuf = _infoBarBuf;
	const uint8 *obj1StrBuf = getStringBuf(_actionObj1Type);
	const uint8 *obj2StrBuf = getStringBuf(_actionObj2Type);
	int infoStringWidth = 0;
	int object1NameWidth = 0;
	int verbWidth = getStringWidth(_actionVerb + 1, infoStrBuf);
	if (_actionObj1Num > 0 || _actionObj1Type > 0) {
		object1NameWidth = getStringWidth(_actionObj1Num + 1, obj1StrBuf) + 4;
		infoStringWidth = verbWidth + object1NameWidth;
	} else {
		infoStringWidth = verbWidth;
	}
	int verbPreposition = 0;
	int verbPrepositionWidth = 0;
	if (_actionRequiresTwoObjects) {
		verbPreposition = (_actionVerb == 5) ? 12 : 11;
		verbPrepositionWidth = getStringWidth(verbPreposition, infoStrBuf) + 4;
		if (_gameLang != Common::EN_ANY && (_actionObj2Num > 0 || _actionObj2Type > 0) && verbPreposition > 0) {
			infoStringWidth = 0;
			verbWidth = 0;
			object1NameWidth = 0;
		}
		infoStringWidth += verbPrepositionWidth;
		if (_actionObj2Num > 0 || _actionObj2Type > 0) {
			infoStringWidth += getStringWidth(_actionObj2Num + 1, obj2StrBuf);
		}
	}
	const int xPos = 159 - infoStringWidth / 2;
	if (_gameLang == Common::EN_ANY || (_actionObj2Num == 0 && _actionObj2Type == 0) || verbPreposition == 0) {
		drawItemString(xPos, _actionVerb + 1, infoStrBuf);
		if (_actionObj1Num > 0 || _actionObj1Type > 0) {
			drawItemString(xPos + 4 + verbWidth, _actionObj1Num + 1, obj1StrBuf);
		}
	}
	if (verbPreposition > 0) {
		drawItemString(xPos + 4 + verbWidth + object1NameWidth, verbPreposition, infoStrBuf);
		if (_actionObj2Num > 0 || _actionObj2Type > 0) {
			drawItemString(xPos + 4 + verbWidth + object1NameWidth + verbPrepositionWidth, _actionObj2Num + 1, obj2StrBuf);
		}
	}
}

void TuckerEngine::drawGameHintString() {
	const int len = getStringWidth(_gameHintsStringNum + 29, _infoBarBuf);
	const int x = 159 - len / 2;
	drawItemString(326 + x, _gameHintsStringNum + 29, _infoBarBuf);
}

void TuckerEngine::updateCharacterAnimation() {
	if (_characterAnimationIndex > -1) {
		if (_backgroundSpriteCurrentFrame == 0) {
			_backgroundSpriteCurrentAnimation = _characterAnimationsTable[_characterAnimationIndex];
			++_characterAnimationIndex;
			_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
			_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
			_backgroundSpriteCurrentFrame = _characterAnimationsTable[_characterAnimationIndex];
			++_characterAnimationIndex;
		} else if (_characterAnimationsTable[_characterAnimationIndex] == 99) {
			_characterAnimationIndex = -1;
			_backgroundSpriteCurrentAnimation = -1;
			if (_nextAction == 0) {
				setCursorType(0);
			}
		} else {
			_backgroundSpriteCurrentFrame = _characterAnimationsTable[_characterAnimationIndex];
			if (_noCharacterAnimationChange == 0) {
				++_characterAnimationIndex;
			}
		}
	} else if (_backgroundSpriteCurrentAnimation > -1) {
		while (_spriteAnimationFramesTable[_spriteAnimationFrameIndex] != 999) {
			++_spriteAnimationFrameIndex;
		}
		_characterFacingDirection = 0;
		if (_changeBackgroundSprite == 1) {
			if (_backgroundSpriteCurrentFrame == 0) {
				_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
				_backgroundSpriteCurrentFrame = _backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
			} else {
				--_backgroundSpriteCurrentFrame;
				if (_backgroundSpriteCurrentFrame < 1) {
					_backgroundSpriteCurrentAnimation = -1;
					_backgroundSpriteCurrentFrame = 0;
					_changeBackgroundSprite = 0;
					if (_nextAction == 0) {
						setCursorType(0);
					}
				}
			}
		} else {
			if (_backgroundSpriteCurrentFrame == 0) {
				_backgroundSpriteCurrentFrame = 1;
				assert(_backgroundSpriteCurrentAnimation >= 0 && _backgroundSpriteCurrentAnimation < kSprA02TableSize);
				_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
				_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
			} else if (_locationNum == 25 && _panelLockedFlag != 1 && (_backgroundSpriteCurrentAnimation == 3 || _backgroundSpriteCurrentAnimation == 6)) {
				_backgroundSpriteCurrentFrame = 0;
				_backgroundSpriteCurrentAnimation = -1;
			} else {
				++_backgroundSpriteCurrentFrame;
				if (_backgroundSpriteCurrentFrame > _backgroundSpriteLastFrame) {
					_backgroundSpriteCurrentAnimation = -1;
					_backgroundSpriteCurrentFrame = 0;
					if (_nextAction == 0 && _panelState == 0) {
						setCursorType(0);
					}
				}
			}
		}
	}
	if (_locationNum == 24 && _flagsTable[103] == 0) {
		if (_panelLockedFlag == 1) {
			_panelLockedFlag = 0;
			_selectedObject.locationObject_locationNum = 0;
			if (_actionVerb != 2) {
				_speechSoundNum = 2236;
				startSpeechSound(_speechSoundNum, _speechVolume);
				_characterSpeechDataPtr = _ptTextBuf + getPositionForLine(_speechSoundNum, _ptTextBuf);
				_speechSoundNum = 0;
				_actionPosX = _xPosCurrent;
				_actionPosY = _yPosCurrent - 64;
				_actionTextColor = 1;
				_actionCharacterNum = 99;
				setCursorType(2);
				_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
			}
		}
		if (_charSpeechSoundCounter == 0 || _actionCharacterNum != 99) {
			if (_backgroundSpriteCurrentAnimation == 5) {
				_backgroundSpriteCurrentFrame = 0;
			}
		} else {
			if (_backgroundSpriteCurrentAnimation != 5) {
				_backgroundSpriteCurrentFrame = 0;
			}
		}
		if (_backgroundSpriteCurrentFrame == 0) {
			if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 99) {
				_backgroundSpriteCurrentAnimation = 5;
			} else {
				_backgroundSpriteCurrentAnimation = (getRandomNumber() < 33000) ? 2 : 3;
			}
			_backgroundSpriteCurrentFrame = 1;
			_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
			_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
		}
	} else if (_locationNum == 25) {
		if (_backgroundSpriteCurrentFrame == 0) {
			if (_characterBackFrontFacing == 0) {
				if (_characterBackFrontFacing != _characterPrevBackFrontFacing) {
					_backgroundSpriteCurrentAnimation = 10;
				} else if (_panelLockedFlag == 1) {
					_backgroundSpriteCurrentAnimation = 3;
				} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 99) {
					_backgroundSpriteCurrentAnimation = 8;
				} else {
					_backgroundSpriteCurrentAnimation = (getRandomNumber() < 32000) ? 11 : 5;
				}
			} else {
				if (_characterBackFrontFacing != _characterPrevBackFrontFacing) {
					_backgroundSpriteCurrentAnimation = 2;
				} else if (_panelLockedFlag == 1) {
					_backgroundSpriteCurrentAnimation = 6;
				} else if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 99) {
					_backgroundSpriteCurrentAnimation = 9;
				} else {
					_backgroundSpriteCurrentAnimation = (getRandomNumber() < 32000) ? 12 : 7;
				}
			}
			_characterPrevBackFrontFacing = _characterBackFrontFacing;
			_backgroundSpriteCurrentFrame = 1;
			_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
			_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
		}
		_backgroundSprOffset = _xPosCurrent - 160;
	} else if (_locationNum == 63 && _backgroundSpriteCurrentFrame == 0) {
		if (_charSpeechSoundCounter > 0 && _actionCharacterNum == 99) {
			_backgroundSpriteCurrentAnimation = 1;
		} else {
			_backgroundSpriteCurrentAnimation = (getRandomNumber() < 32000) ? 3 : 2;
		}
		_backgroundSpriteCurrentFrame = 1;
		_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
		_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
	}
	int frame = _spriteAnimationFramesTable[_spriteAnimationFrameIndex];
	if (_panelLockedFlag == 0 && _characterFacingDirection < 5 && _selectedObject.locationObject_locationNum == 0) {
		_characterFacingDirection = 0;
	}
	if (_charSpeechSoundCounter > 0 && _characterFacingDirection != 6 && _actionCharacterNum == 99) {
		_characterFacingDirection = 6;
		frame = 999;
	} else if (_characterFacingDirection == 6 && (_charSpeechSoundCounter == 0 || _actionCharacterNum != 99)) {
		_characterFacingDirection = 0;
		frame = 999;
	}
	int num = 0;
	if (frame == 999 || (_characterFacingDirection != _characterPrevFacingDirection && _characterFacingDirection < 5)) {
		_mirroredDrawing = 0;
		if (_characterFacingDirection == 6) {
			if (_csDataHandled) {
				switch (_selectedCharacterDirection) {
				case 1:
					num = 17;
					break;
				case 2:
					num = 16;
					break;
				case 4:
					num = 15;
					break;
				default:
					num = 16;
					_mirroredDrawing = 1;
					break;
				}
			} else {
				num = 15;
			}
		}
		if (_characterFacingDirection == 5) {
			_characterFacingDirection = 0;
		}
		if (_characterFacingDirection == 0) {
			if (_csDataHandled) {
				_mirroredDrawing = 0;
				switch (_selectedCharacterDirection) {
				case 1:
					num = 3;
					break;
				case 2:
					num = 1;
					break;
				case 3:
					num = 1;
					_mirroredDrawing = 1;
					break;
				default:
					num = 5;
					break;
				}
			} else if (getRandomNumber() < 2000) {
				num = 13;
			} else if (getRandomNumber() < 3000) {
				num = 14;
				if (_locationNum == 57) {
					num = 18;
				}
			} else {
				num = (getRandomNumber() < 20000) ? 18 : 6;
			}
		} else {
			switch (_characterFacingDirection) {
			case 1:
				num = 0;
				break;
			case 2:
				num = 4;
				break;
			case 3:
				num = 0;
				_mirroredDrawing = 1;
				break;
			case 4:
				num = 2;
				break;
			}
		}
		_currentSpriteAnimationLength = _spriteAnimationsTable[num].numParts;
		_spriteAnimationFrameIndex = _spriteAnimationsTable[num].firstFrameIndex;
		frame = _spriteAnimationFramesTable[_spriteAnimationFrameIndex];
	}
	if (_characterAnimationNum > 0) {
		num = _characterAnimationNum;
		_currentSpriteAnimationLength = _spriteAnimationsTable[num].numParts;
		_spriteAnimationFrameIndex = _spriteAnimationsTable[num].firstFrameIndex;
		frame = _spriteAnimationFramesTable[_spriteAnimationFrameIndex];
		_characterAnimationNum = 0;
	}
	_currentSpriteAnimationFrame = frame;
	++_spriteAnimationFrameIndex;
	if (_currentSpriteAnimationLength > 1) {
		_currentSpriteAnimationFrame2 = _spriteAnimationFramesTable[_spriteAnimationFrameIndex];
		++_spriteAnimationFrameIndex;
		if (_characterSpriteAnimationFrameCounter > 0) {
			++_characterSpriteAnimationFrameCounter;
			if (_characterSpriteAnimationFrameCounter > 121) {
				_characterSpriteAnimationFrameCounter = 0;
			}
			if (_selectedCharacterDirection == 1) {
				_currentSpriteAnimationFrame = (_characterSpriteAnimationFrameCounter > 2 && _characterSpriteAnimationFrameCounter < 120) ? 122 : 121;
			} else {
				_currentSpriteAnimationFrame = (_characterSpriteAnimationFrameCounter > 2 && _characterSpriteAnimationFrameCounter < 120) ? 120 : 119;
			}
		}
	}
	_characterPrevFacingDirection = _characterFacingDirection;
}

void TuckerEngine::addObjectToInventory(int num) {
	_inventoryObjectsList[_inventoryObjectsCount] = num;
	_lastInventoryObjectIndex = _inventoryObjectsCount;
	_redrawPanelItemsCounter = 50;
	++_inventoryObjectsCount;
	_inventoryItemsState[num] = 1;
	if (_inventoryObjectsOffset + 5 < _lastInventoryObjectIndex) {
		_inventoryObjectsOffset += 3;
	}
}

void TuckerEngine::removeObjectFromInventory(int num) {
	for (int i = 0; i < _inventoryObjectsCount; ++i) {
		if (_inventoryObjectsList[i] == num) {
			--_inventoryObjectsCount;
			_inventoryItemsState[num] = 2;
			const int count = _inventoryObjectsCount - i;
			if (count != 0) {
				memmove(_inventoryObjectsList + i, _inventoryObjectsList + i + 1, count * sizeof(int));
			}
			break;
		}
	}
}

void TuckerEngine::handleMap() {
	if (_handleMapCounter > 0) {
		++_handleMapCounter;
		if (_handleMapCounter > 19) {
			_handleMapCounter = 0;
			_locationMaskCounter = 1;
			_panelLockedFlag = 0;
		}
	}
	if (_panelLockedFlag == 0 && (_backgroundSpriteCurrentAnimation == -1 || _locationNum == 25) && _locationMaskType == 3) {
		setCursorType(0);
		if (_locationMaskCounter == 1) {
			_characterFacingDirection = 0;
			_locationMaskType = 0;
		}
		return;
	}
	if (_selectedObject.locationObject_locationNum != 0 && _locationMaskCounter != 0 && (_backgroundSpriteCurrentAnimation <= -1 || _locationNum == 25)) {
		if (_locationNum == 25 || _backgroundSpriteCurrentAnimation != 4) {
			if (_locationMaskType == 0) {
				_locationMaskType = 1;
				setCursorType(2);
				if (_selectedObject.locationObject_toWalkX2 > 800) {
					_backgroundSpriteCurrentAnimation = _selectedObject.locationObject_toWalkX2 - 900;
					if (_selectedObject.locationObject_toWalkY2 > 499) {
						_changeBackgroundSprite = 1;
						_backgroundSprOffset = _selectedObject.locationObject_toWalkY2 - 500;
					} else {
						_backgroundSprOffset = _selectedObject.locationObject_toWalkY2;
						_changeBackgroundSprite = 0;
					}
					_backgroundSpriteCurrentFrame = 0;
					_mirroredDrawing = 0;
					if (_locationNum == 25) {
						_backgroundSpriteDataPtr = _sprA02Table[_backgroundSpriteCurrentAnimation];
						_backgroundSpriteLastFrame = READ_LE_UINT16(_backgroundSpriteDataPtr);
						_backgroundSpriteCurrentFrame = 1;
					}
				} else {
					_locationMaskCounter = 0;
					_selectedObject.xPos = _selectedObject.locationObject_toWalkX2;
					_selectedObject.yPos = _selectedObject.locationObject_toWalkY2;
					_handleMapCounter = 1;
					_panelLockedFlag = 1;
				}
				return;
			}
			_locationMaskType = 2;
			_panelState = 0;
			setCursorType(0);
			if (_selectedObject.locationObject_locationNum == 99) {
				_noPositionChangeAfterMap = true;
				handleMapSequence();
				return;
			}
			for (int i = 0; i < 14; ++i) {
				fadeInPalette();
				redrawScreen(_scrollOffset);
				_fadePaletteCounter = 34;
			}
			_nextLocationNum = _selectedObject.locationObject_locationNum;
			_xPosCurrent = _selectedObject.locationObject_toX;
			_yPosCurrent = _selectedObject.locationObject_toY;
			if (_selectedObject.locationObject_toX2 > 800) {
				_backgroundSpriteCurrentAnimation = _selectedObject.locationObject_toX2 - 900;
				if (_selectedObject.locationObject_toY2 > 499) {
					_changeBackgroundSprite = 1;
					_backgroundSprOffset = _selectedObject.locationObject_toY2 - 500;
				} else {
					_changeBackgroundSprite = 0;
					_backgroundSprOffset = _selectedObject.locationObject_toY2;
				}
				_backgroundSpriteCurrentFrame = 0;
			} else {
				_selectedObject.xPos = _selectedObject.locationObject_toX2;
				_selectedObject.yPos = _selectedObject.locationObject_toY2;
				_panelLockedFlag = 1;
			}
			_scrollOffset = 0;
			_handleMapCounter = 0;
			_locationMaskCounter = 0;
			_selectedObject.locationObject_locationNum = 0;
		}
	}
}

void TuckerEngine::clearSprites() {
	memset(_spritesTable, 0, sizeof(_spritesTable));
	for (int i = 0; i < kMaxCharacters; ++i) {
		_spritesTable[i].state = -1;
		_spritesTable[i].stateIndex = -1;
	}
}

void TuckerEngine::updateSprites() {
	const int count = (_locationNum == 9) ? 3 : _spritesCount;
	for (int i = 0; i < count; ++i) {
		if (_spritesTable[i].stateIndex > -1) {
			++_spritesTable[i].stateIndex;
			if (_characterStateTable[_spritesTable[i].stateIndex] == 99) {
				_spritesTable[i].stateIndex = -1;
				_spritesTable[i].state = -1;
				updateSprite(i);
			} else {
				_spritesTable[i].animationFrame = _characterStateTable[_spritesTable[i].stateIndex];
			}
			continue;
		}
		if (_spritesTable[i].state == -1) {
			updateSprite(i);
			continue;
		}
		if (_charSpeechSoundCounter > 0 && i == _actionCharacterNum && _spritesTable[i].needUpdate == 0) {
			updateSprite(i);
			continue;
		}
		if (_charSpeechSoundCounter == 0 && _spritesTable[i].needUpdate > 0) {
			updateSprite(i);
			continue;
		}
		if (_spritesTable[i].updateDelay > 0) {
			--_spritesTable[i].updateDelay;
			if (_spritesTable[i].updateDelay == 0) {
				updateSprite(i);
			}
			continue;
		}
		if (_spritesTable[i].defaultUpdateDelay > 0) {
			_spritesTable[i].updateDelay = _spritesTable[i].defaultUpdateDelay - 1;
			++_spritesTable[i].animationFrame;
			if (_spritesTable[i].animationFrame == _spritesTable[i].firstFrame) {
				updateSprite(i);
			}
			continue;
		}
		if (_spritesTable[i].nextAnimationFrame == 0) {
			++_spritesTable[i].animationFrame;
			if (_spritesTable[i].firstFrame - 1 < _spritesTable[i].animationFrame) {
				if (_spritesTable[i].prevAnimationFrame == 1) {
					--_spritesTable[i].animationFrame;
					_spritesTable[i].nextAnimationFrame = 1;
				} else {
					updateSprite(i);
				}
			}
			continue;
		}
		--_spritesTable[i].animationFrame;
		if (_spritesTable[i].animationFrame == 0) {
			updateSprite(i);
		}
	}
}

void TuckerEngine::updateSprite(int i) {
	_spritesTable[i].prevState = _spritesTable[i].state;
	_spritesTable[i].prevAnimationFrame = 0;
	_spritesTable[i].nextAnimationFrame = 0;
	_updateSpriteFlag1 = 0;
	_updateSpriteFlag2 = 0;
	_spritesTable[i].defaultUpdateDelay = 0;
	_spritesTable[i].updateDelay = 0;
	switch (_locationNum) {
	case 2:
		updateSprite_locationNum2();
		break;
	case 3:
		if (i == 0) {
			updateSprite_locationNum3_0(i);
		} else if (i == 1) {
			updateSprite_locationNum3_1(i);
		} else {
			updateSprite_locationNum3_2(i);
		}
		break;
	case 4:
		updateSprite_locationNum4(0);
		break;
	case 5:
		if (i == 0) {
			updateSprite_locationNum5_0();
		} else {
			updateSprite_locationNum5_1(1);
		}
		break;
	case 6:
		if (i == 0) {
			updateSprite_locationNum6_0(0);
		} else if (i == 1) {
			updateSprite_locationNum6_1(1);
		} else {
			updateSprite_locationNum6_2(2);
		}
		break;
	case 7:
		if (i == 0) {
			updateSprite_locationNum7_0(0);
		} else {
			updateSprite_locationNum7_1(1);
		}
		break;
	case 8:
		if (i == 0) {
			updateSprite_locationNum8_0(0);
		} else {
			updateSprite_locationNum8_1(1);
		}
		break;
	case 9:
		if (i == 0) {
			updateSprite_locationNum9_0(i);
		} else if (i == 1) {
			updateSprite_locationNum9_1(i);
		} else {
			updateSprite_locationNum9_2(i);
		}
		break;
	case 10:
		updateSprite_locationNum10();
		break;
	case 11:
		if (i == 0) {
			updateSprite_locationNum11_0(0);
		} else if (i == 1) {
			updateSprite_locationNum11_1(1);
		} else if (i == 2) {
			updateSprite_locationNum11_2(2);
		} else if (i == 3) {
			updateSprite_locationNum11_3(3);
		} else if (i == 4) {
			updateSprite_locationNum11_4(4);
		}
		break;
	case 12:
		if (i == 0) {
			updateSprite_locationNum12_0(0);
		} else if (i == 1) {
			updateSprite_locationNum12_1(1);
		}
		break;
	case 13:
		updateSprite_locationNum13(0);
		break;
	case 14:
		updateSprite_locationNum14(0);
		break;
	case 15:
		if (i == 1) {
			updateSprite_locationNum15_1(1);
		} else if (i == 2) {
			updateSprite_locationNum15_2(2);
		} else if (i == 0) {
			updateSprite_locationNum15_0(0);
		}
		break;
	case 16:
		if (i == 0) {
			updateSprite_locationNum16_0(0);
		} else if (i == 1) {
			updateSprite_locationNum16_1(1);
		} else {
			updateSprite_locationNum16_2(2);
		}
		break;
	case 17:
		updateSprite_locationNum17();
		break;
	case 18:
		updateSprite_locationNum18();
		break;
	case 19:
		if (i == 0) {
			updateSprite_locationNum19_0(0);
		} else if (i == 1) {
			updateSprite_locationNum19_1(1);
		} else if (i == 2) {
			updateSprite_locationNum19_2(2);
		} else {
			updateSprite_locationNum19_3(3);
		}
		break;
	case 21:
		updateSprite_locationNum21();
		break;
	case 22:
		updateSprite_locationNum22();
		break;
	case 23:
		if (i == 0) {
			updateSprite_locationNum23_0(0);
		} else if (i == 1) {
			updateSprite_locationNum23_1(1);
		} else if (i == 2) {
			updateSprite_locationNum23_2(2);
		} else {
			updateSprite_locationNum23_3(3);
		}
		break;
	case 24:
		if (i == 0) {
			updateSprite_locationNum24_0(0);
		} else if (i == 1) {
			updateSprite_locationNum24_1(1);
		} else if (i == 2) {
			updateSprite_locationNum24_2(2);
		} else {
			updateSprite_locationNum24_3(3);
		}
		break;
	case 26:
		if (i == 0) {
			updateSprite_locationNum26_0(0);
		} else {
			updateSprite_locationNum26_1(1);
		}
		break;
	case 27:
		updateSprite_locationNum27(0);
		break;
	case 28:
		if (i == 0) {
			updateSprite_locationNum28_0(0);
		} else if (i == 1) {
			updateSprite_locationNum28_1(1);
		} else {
			updateSprite_locationNum28_2(2);
		}
		break;
	case 29:
		if (i == 0) {
			updateSprite_locationNum29_0(0);
		} else if (i == 1) {
			updateSprite_locationNum29_1(1);
		} else {
			updateSprite_locationNum29_2(2);
		}
		break;
	case 30:
		updateSprite_locationNum30_34(i);
		break;
	case 31:
		if (i == 0) {
			updateSprite_locationNum31_0(0);
		} else {
			updateSprite_locationNum31_1(1);
		}
		break;
	case 32:
		if (i == 0) {
			updateSprite_locationNum32_0(0);
		} else {
			_spritesTable[i].state = -1;
		}
		break;
	case 33:
		if (i == 1) {
			updateSprite_locationNum33_1(1);
		} else if (i == 0) {
			updateSprite_locationNum33_0(0);
		} else if (i == 2) {
			updateSprite_locationNum33_2(2);
		} else {
			_spritesTable[i].state = 12;
		}
		break;
	case 34:
		updateSprite_locationNum30_34(0);
		break;
	case 36:
		updateSprite_locationNum36(0);
		break;
	case 37:
		if (i == 0) {
			_spritesTable[0].state = -1;
		} else {
			updateSprite_locationNum37(i);
		}
		break;
	case 41:
		updateSprite_locationNum41(i);
		break;
	case 42:
		updateSprite_locationNum42(i);
		break;
	case 43:
		if (i == 2) {
			updateSprite_locationNum43_2(i);
		} else if (i < 2) {
			if (_flagsTable[236] < 4) {
				_spritesTable[i].state = i + 1;
			} else {
				_spritesTable[i].state = -1;
			}
		} else if (i == 3) {
			updateSprite_locationNum43_3(3);
		} else if (i == 4) {
			updateSprite_locationNum43_4(4);
		} else if (i == 5) {
			updateSprite_locationNum43_5(5);
		} else {
			updateSprite_locationNum43_6(6);
		}
		break;
	case 45:
		_spritesTable[0].state = 1;
		break;
	case 47:
		_spritesTable[i].state = i + 1;
		break;
	case 48:
		updateSprite_locationNum48(0);
		break;
	case 49:
		updateSprite_locationNum49(0);
		break;
	case 50:
		if (i < 6) {
			updateSprite_locationNum50(i);
		} else {
			_spritesTable[i].state = i + 1;
		}
		break;
	case 51:
		updateSprite_locationNum51(i);
		break;
	case 53:
		if (i == 0) {
			updateSprite_locationNum53_0(0);
		} else if (i == 1) {
			updateSprite_locationNum53_1(1);
		}
		break;
	case 54:
		updateSprite_locationNum54(0);
		break;
	case 55:
		updateSprite_locationNum55(0);
		break;
	case 56:
		updateSprite_locationNum56(0);
		break;
	case 57:
		if (i == 0) {
			updateSprite_locationNum57_0(0);
		} else if (i == 1) {
			updateSprite_locationNum57_1(1);
		}
		break;
	case 58:
		updateSprite_locationNum58(0);
		break;
	case 59:
		updateSprite_locationNum59(0);
		break;
	case 60:
		if (i == 0) {
			updateSprite_locationNum60_0(0);
		} else {
			updateSprite_locationNum60_1(1);
		}
		break;
	case 61:
		if (i == 0) {
			updateSprite_locationNum61_0(0);
		} else if (i == 2) {
			updateSprite_locationNum61_2(2);
		} else {
			updateSprite_locationNum61_1(1);
		}
		break;
	case 63:
		if (i == 0) {
			updateSprite_locationNum63_0(0);
		} else if (i == 1) {
			updateSprite_locationNum63_1(1);
		} else if (i == 2) {
			updateSprite_locationNum63_2(2);
		} else if (i == 3) {
			updateSprite_locationNum63_3(3);
		} else if (i == 4) {
			updateSprite_locationNum63_4(4);
		}
		break;
	case 65:
		updateSprite_locationNum65(0);
		break;
	case 66:
		if (i == 0) {
			updateSprite_locationNum66_0(0);
		} else if (i == 1) {
			updateSprite_locationNum66_1(1);
		} else if (i == 2) {
			updateSprite_locationNum66_2(2);
		} else if (i == 3) {
			updateSprite_locationNum66_3(3);
		} else {
			updateSprite_locationNum66_4(4);
		}
		break;
	case 69:
		if (i == 0) {
			_spritesTable[0].state = 1;
		} else if (i == 1) {
			updateSprite_locationNum69_1(1);
		} else if (i == 2) {
			updateSprite_locationNum69_2(2);
		} else if (i == 3) {
			updateSprite_locationNum69_3(3);
		}
		break;
	case 71:
		updateSprite_locationNum71(0);
		break;
	case 72:
		updateSprite_locationNum72(0);
		break;
	case 74:
		updateSprite_locationNum74(i);
		break;
	case 79:
		updateSprite_locationNum79(0);
		break;
	case 81:
		if (i == 0) {
			updateSprite_locationNum81_0(0);
		} else if (i == 1) {
			updateSprite_locationNum81_1(1);
		}
		break;
	case 82:
		updateSprite_locationNum82(0);
		break;
	case 98:
		_spritesTable[0].state = 1;
		break;
	}
	if (_spritesTable[i].stateIndex <= -1) {
		if (_updateSpriteFlag1 == 0) {
			_spritesTable[i].animationFrame = 1;
		}
		if (_spritesTable[i].state < 0 || !_sprC02Table[_spritesTable[i].state]) {
//			warning("Invalid state %d for sprite %d location %d", _spritesTable[i].state, i, _locationNum);
			return;
		}
		_spritesTable[i].animationData = _sprC02Table[_spritesTable[i].state];
		_spritesTable[i].firstFrame = READ_LE_UINT16(_spritesTable[i].animationData);
		if (_updateSpriteFlag2 == 1) {
			_spritesTable[i].state = _spritesTable[i].firstFrame;
			_spritesTable[i].nextAnimationFrame = 1;
			_spritesTable[i].prevAnimationFrame = 1;
		}
	}
}

void TuckerEngine::drawStringInteger(int num, int x, int y, int digits) {
	char numStr[4];
	assert(num < 1000);
	sprintf(numStr, "%03d", num);
	int i = (digits > 2) ? 0 : 1;
	for (; i < 3; ++i) {
		Graphics::drawStringChar(_locationBackgroundGfxBuf, _scrollOffset + x, y, 640, numStr[i], 102, _charsetGfxBuf);
		x += 8;
	}
	addDirtyRect(_scrollOffset + x, y, Graphics::_charset.charW * 3, Graphics::_charset.charH);
}

void TuckerEngine::drawStringAlt(int x, int y, int color, const uint8 *str, int strLen) {
	const int xStart = x;
	int pos = 0;
	while (pos != strLen && str[pos] != '\n') {
		const uint8 chr = str[pos];
		Graphics::drawStringChar(_locationBackgroundGfxBuf, x, y, 640, chr, color, _charsetGfxBuf);
		x += _charWidthTable[chr];
		++pos;
	}
	addDirtyRect(xStart, y, x - xStart, Graphics::_charset.charH);
}

void TuckerEngine::drawItemString(int x, int num, const uint8 *str) {
	int pos = getPositionForLine(num, str);
	while (str[pos] != '\n') {
		const uint8 chr = str[pos];
		Graphics::drawStringChar(_itemsGfxBuf, x, 0, 320, chr, 1, _charsetGfxBuf);
		x += _charWidthTable[chr];
		++pos;
	}
}

void TuckerEngine::drawCreditsString(int x, int y, int num) {
	int pos = getPositionForLine(num, _ptTextBuf);
	while (_ptTextBuf[pos] != '\n') {
		const uint8 chr = _ptTextBuf[pos];
		Graphics::drawStringChar(_locationBackgroundGfxBuf, x, y, 640, chr, 1, _charsetGfxBuf);
		x += _charWidthTable[chr];
		++pos;
	}
}

void TuckerEngine::updateCharSpeechSound(bool displayText) {
	if (_charSpeechSoundCounter == 0) {
		return;
	}
	if (_displaySpeechText) {
		_charSpeechSoundCounter = 0;
	} else {
		--_charSpeechSoundCounter;
	}
	if (_charSpeechSoundCounter == 0) {
		_charSpeechSoundCounter = isSpeechSoundPlaying() ? 1 : 0;
		if (_charSpeechSoundCounter == 0) {
			_characterSpriteAnimationFrameCounter = 0;
		}
	}
	if (_charSpeechSoundCounter == 0 && !_csDataHandled) {
		setCursorType(0);
	} else if (displayText) {
		drawSpeechText(_actionPosX, _actionPosY, _characterSpeechDataPtr, _speechSoundNum, _actionTextColor);
	}
}

void TuckerEngine::updateItemsGfxColors(int color1, int color128) {
	for (int i = 0; i < 3200; ++i) {
		if (_itemsGfxBuf[i] == 1) {
			_itemsGfxBuf[i] = color1;
		} else if (_itemsGfxBuf[i] == 128) {
			_itemsGfxBuf[i] = color128;
		}
	}
}

int TuckerEngine::testLocationMask(int x, int y) {
	if (_locationMaskType > 0 || _locationMaskIgnore > 0) {
		return 1;
	}
	if (_locationNum == 26 || _locationNum == 32) {
		y -= 3;
	}
	const int offset = y * 640 + x;
	return _locationBackgroundMaskBuf[offset] > 0 ? 1 : 0;
}

int TuckerEngine::getStringWidth(int num, const uint8 *ptr) {
	int w = 0;
	int pos = getPositionForLine(num, ptr);
	uint8 chr;
	while ((chr = ptr[pos]) != '\n') {
		w += _charWidthTable[chr];
		++pos;
	}
	return w;
}

int TuckerEngine::getPositionForLine(int num, const uint8 *ptr) {
	int linesCount = 0;
	int i = 0;
	while (linesCount < num) {
		if (ptr[i] == '\n') {
			++linesCount;
			if (ptr[i + 1] == '\r') {
				++i;
			}
		}
		++i;
	}
	while (1) {
		if (ptr[i] != '\n' && ptr[i] != '\r') {
			break;
		}
		++i;
	}
	return i;
}

void TuckerEngine::resetCharacterAnimationIndex(int count) {
	_backgroundSpriteCurrentFrame = 0;
	_characterAnimationIndex = 0;
	for (int i = 0; i < count; ++i) {
		while (_characterAnimationsTable[_characterAnimationIndex] != 99) {
			++_characterAnimationIndex;
		}
		++_characterAnimationIndex;
	}
}

enum TableInstructionCode {
	kCode_invalid,
	kCode_pan,
	kCode_bua,
	kCode_bub,
	kCode_buc,
	kCode_bsd,
	kCode_bof,
	kCode_buh,
	kCode_bon,
	kCode_bso,
	kCode_bus,
	kCode_buw,
	kCode_bux,
	kCode_c0a,
	kCode_c0c,
	kCode_c0s,
	kCode_end,
	kCode_fad,
	kCode_fw,
	kCode_flx,
	kCode_fxx,
	kCode_fx,
	kCode_gfg,
	kCode_gv,
	kCode_loc,
	kCode_opt,
	kCode_opf,
	kCode_ofg,
	kCode_snc,
	kCode_sse,
	kCode_ssp,
	kCode_s0p,
	kCode_sp,
	kCode_tpo,
	kCode_wa_,
	kCode_wsm,
	kCode_wat,
	kCode_was,
	kCode_wfx,
	kCode_xhr,
	kCode_xhm
};

static const struct {
	const char *name;
	int code;
} _instructions[] = {
	{ "pan", kCode_pan },
	{ "bua", kCode_bua },
	{ "bub", kCode_bub },
	{ "buc", kCode_buc },
	{ "bsd", kCode_bsd },
	{ "bcd", kCode_bsd }, // only ref 6.27
	{ "bud", kCode_bsd }, // only ref 13.3
	{ "bof", kCode_bof },
	{ "buh", kCode_buh },
	{ "bon", kCode_bon },
	{ "bso", kCode_bso },
	{ "bus", kCode_bus },
	{ "b0s", kCode_bus }, // only ref 65.25
	{ "buw", kCode_buw },
	{ "bdx", kCode_bux },
	{ "bux", kCode_bux },
	{ "c0a", kCode_c0a },
	{ "c0c", kCode_c0c },
	{ "c0s", kCode_c0s },
	{ "end", kCode_end },
	{ "fad", kCode_fad },
	{ "fw",  kCode_fw  },
	{ "flx", kCode_flx },
	{ "fxx", kCode_fxx },
	{ "fx",  kCode_fx  },
	{ "gfg", kCode_gfg },
	{ "gv",  kCode_gv  },
	{ "loc", kCode_loc },
	{ "opt", kCode_opt },
	{ "opf", kCode_opf },
	{ "ofg", kCode_ofg },
	{ "snc", kCode_snc },
	{ "sse", kCode_sse },
	{ "ssp", kCode_ssp },
	{ "s0p", kCode_s0p },
	{ "sp",  kCode_sp  },
	{ "tpo", kCode_tpo },
	{ "wa+", kCode_wa_ },
	{ "wsm", kCode_wsm },
	{ "wat", kCode_wat },
	{ "was", kCode_was },
	{ "wfx", kCode_wfx },
	{ "xhr", kCode_xhr },
	{ "xhm", kCode_xhm },
	{ 0, 0 }
};

int TuckerEngine::readTableInstructionCode(int *index) {
	bool match = false;
	for (int i = 0; _instructions[i].name; ++i) {
		const int nameLen = strlen(_instructions[i].name);
		if (_instructions[i].name[1] == '0') {
			if (_instructions[i].name[0] == _tableInstructionsPtr[0] && _instructions[i].name[2] == _tableInstructionsPtr[2]) {
				const char digit = _tableInstructionsPtr[1];
				assert(digit >= '0' && digit <= '9');
				*index = digit - '0';
				match = true;
			}
		} else {
			if (strncmp(_instructions[i].name, (const char *)_tableInstructionsPtr, nameLen) == 0) {
				*index = 0;
				match = true;
			}
		}
		if (match) {
			_tableInstructionsPtr += nameLen + 1;
			return _instructions[i].code;
		}
	}
	warning("Unhandled instruction '%c%c%c'", _tableInstructionsPtr[0], _tableInstructionsPtr[1], _tableInstructionsPtr[2]);
	return kCode_invalid;
}

int TuckerEngine::readTableInstructionParam(int len) {
	// skip duplicated minus signs (bua,--1, c0a,--1, ...)
	if (len >= 3 && memcmp(_tableInstructionsPtr, "--", 2) == 0) {
		++_tableInstructionsPtr;
		--len;
	}
	char *end = 0;
	const int param = strtol((const char *)_tableInstructionsPtr, &end, 10);
	if (end != (const char *)_tableInstructionsPtr + len) {
		warning("Unexpected instruction parameter length %d (%d)", (int)(end - (const char *)_tableInstructionsPtr), len);
	}
	_tableInstructionsPtr += len + 1;
	return param;
}

int TuckerEngine::executeTableInstruction() {
	int i, index = 0;

	debug(2, "executeTableInstruction() instruction %c%c%c", _tableInstructionsPtr[0], _tableInstructionsPtr[1], _tableInstructionsPtr[2]);
	const int code = readTableInstructionCode(&index);
	switch (code) {
	case kCode_pan:
		_panelState = readTableInstructionParam(2);
		_forceRedrawPanelItems = true;
		return 0;
	case kCode_bua:
		_backgroundSpriteCurrentAnimation = readTableInstructionParam(3);
		_backgroundSpriteCurrentFrame = 0;
		_backgroundSprOffset = 0;
		_mainLoopCounter2 = 0;
		return 0;
	case kCode_bub:
		i = readTableInstructionParam(3);
		_spriteAnimationFrameIndex = _spriteAnimationsTable[i].firstFrameIndex;
		_characterFacingDirection = 5;
		_mainLoopCounter2 = 0;
		return 0;
	case kCode_buc:
		i = readTableInstructionParam(3);
		resetCharacterAnimationIndex(i);
		_backgroundSpriteCurrentFrame = 0;
		_backgroundSprOffset = 0;
		return 0;
	case kCode_bsd:
		_selectedCharacterDirection = readTableInstructionParam(2);
		return 0;
	case kCode_bof:
		_skipCurrentCharacterDraw = 1;
		return 0;
	case kCode_buh:
		_noCharacterAnimationChange = readTableInstructionParam(2);
		return 0;
	case kCode_bon:
		_skipCurrentCharacterDraw = 0;
		return 0;
	case kCode_bso:
		_backgroundSprOffset = readTableInstructionParam(3);
		return 0;
	case kCode_bus:
		_speechSoundNum = readTableInstructionParam(3) - 1;
		rememberSpeechSound();
		startSpeechSound(_partNum * 3000 + _ptTextOffset + _speechSoundNum - 3000, _speechVolume);
		_actionPosX = _xPosCurrent;
		_actionPosY = _yPosCurrent - 64;
		_actionTextColor = 1;
		_actionCharacterNum = 99;
		_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
		return 0;
	case kCode_buw:
		_selectedObject.xPos = readTableInstructionParam(3);
		_selectedObject.yPos = readTableInstructionParam(3);
		_locationMaskIgnore = 1;
		_panelLockedFlag = 1;
		return 0;
	case kCode_bux:
		_xPosCurrent = readTableInstructionParam(3);
		_yPosCurrent = readTableInstructionParam(3);
		return 0;
	case kCode_c0a:
		_spritesTable[index].state = readTableInstructionParam(3);
		if (_spritesTable[index].state == 999) {
			_spritesTable[index].state = -1;
		}
		_mainLoopCounter1 = 0;
		_spritesTable[index].updateDelay = 0;
		_spritesTable[index].nextAnimationFrame = 0;
		_spritesTable[index].prevAnimationFrame = 0;
		return 0;
	case kCode_c0c:
		setCharacterAnimation(readTableInstructionParam(3), index);
		return 0;
	case kCode_c0s:
		_speechSoundNum = readTableInstructionParam(3) - 1;
		rememberSpeechSound();
		startSpeechSound(_partNum * 3000 + _ptTextOffset + _speechSoundNum - 3000, kMaxSoundVolume);
		_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
		_actionTextColor = 181 + index;
		if (_tableInstructionFlag == 0) {
			_actionPosX = _tableInstructionItemNum1;
			_actionPosY = _tableInstructionItemNum2;
		} else {
			_actionPosX = _tableInstructionObj1Table[index];
			_actionPosY = _tableInstructionObj2Table[index];
		}
		_actionCharacterNum = index;
		return 0;
	case kCode_end:
		return 2;
	case kCode_fad:
		_fadePaletteCounter = readTableInstructionParam(2);
		return 0;
	case kCode_fw:
		_selectedCharacterNum = readTableInstructionParam(2);
		_actionVerb = 0;
		_selectedObjectType = 0;
		_selectedObjectNum = 1;
		setSelectedObjectKey();
		return 0;
	case kCode_flx:
		i = readTableInstructionParam(2);
		_locationSoundsTable[i].type = 2;
		startSound(_locationSoundsTable[i].offset, i, _locationSoundsTable[i].volume);
		return 0;
	case kCode_fxx:
		i = readTableInstructionParam(2);
		if (isSoundPlaying(i)) {
			stopSound(i);
		}
		return 0;
	case kCode_fx:
		i = readTableInstructionParam(2);
		startSound(_locationSoundsTable[i].offset, i, _locationSoundsTable[i].volume);
		_soundInstructionIndex = i;
		return 0;
	case kCode_gfg:
		i = readTableInstructionParam(3);
		assert(i >= 0 && i < kFlagsTableSize);
		_flagsTable[i] = readTableInstructionParam(2);
		debug(2, "executeTableInstruction() set flag %d to %d", i, _flagsTable[i]);
		return 0;
	case kCode_gv:
		_characterAnimationNum = readTableInstructionParam(2);
		return 0;
	case kCode_loc:
		_nextLocationNum = readTableInstructionParam(2);
		return 1;
	case kCode_opt:
		_conversationOptionsCount = readTableInstructionParam(2);
		for (i = 0; i < _conversationOptionsCount; ++i) {
			_instructionsActionsTable[i] = readTableInstructionParam(3) - 1;
			_nextTableToLoadTable[i] = readTableInstructionParam(3);
		}
		_nextTableToLoadIndex = -1;
		setCursorType(1);
		return 1;
	case kCode_opf:
		_conversationOptionsCount = 0;
		for (i = readTableInstructionParam(2); i > 0; --i) {
			const int flag = readTableInstructionParam(3);
			const int value = readTableInstructionParam(2);
			debug(2, "executeTableInstruction() compare flag %d to %d (%d)", i, value, _flagsTable[i]);
			assert(flag >= 0 && flag < kFlagsTableSize);
			if (value == _flagsTable[flag]) {
				assert(_conversationOptionsCount < 6);
				_instructionsActionsTable[_conversationOptionsCount] = readTableInstructionParam(3) - 1;
				_nextTableToLoadTable[_conversationOptionsCount] = readTableInstructionParam(3);
				++_conversationOptionsCount;
			} else {
				readTableInstructionParam(3);
				readTableInstructionParam(3);
			}
		}
		_nextTableToLoadIndex = -1;
		setCursorType(1);
		return 1;
	case kCode_ofg:
		i = readTableInstructionParam(3);
		if (readTableInstructionParam(2) == 0) {
			removeObjectFromInventory(i);
		} else {
			addObjectToInventory(i);
		}
		return 0;
	case kCode_snc:
		_mainLoopCounter1 = 0;
		return 0;
	case kCode_sse:
		_nextAction = readTableInstructionParam(3);
		_csDataLoaded = false;
		return 3;
	case kCode_ssp:
		_tableInstructionFlag = 0;
		_tableInstructionItemNum1 = readTableInstructionParam(3);
		_tableInstructionItemNum2 = readTableInstructionParam(3);
		return 0;
	case kCode_s0p:
		_tableInstructionFlag = 1;
		_tableInstructionObj1Table[index] = readTableInstructionParam(3);
		_tableInstructionObj2Table[index] = readTableInstructionParam(3);
		return 0;
	case kCode_sp:
		_characterSpriteAnimationFrameCounter = 1;
		return 0;
	case kCode_tpo:
		_ptTextOffset = readTableInstructionParam(4);
		_characterSpeechDataPtr = _ptTextBuf + getPositionForLine(_ptTextOffset, _ptTextBuf);
		return 0;
	case kCode_wa_:
		_stopActionOnSpeechFlag = true;
		_stopActionCounter = 20;
		return 1;
	case kCode_wsm:
		_stopActionOnPanelLock = true;
		return 1;
	case kCode_wat:
		_stopActionCounter = readTableInstructionParam(3);
		return 1;
	case kCode_was:
		_stopActionOnSpeechFlag = true;
		return 1;
	case kCode_wfx:
		_stopActionOnSoundFlag = true;
		return 1;
	case kCode_xhr:
		_validInstructionId = true;
		return 0;
	case kCode_xhm:
		_validInstructionId = false;
		return 0;
	}
	return 2;
}

void TuckerEngine::moveUpInventoryObjects() {
	if (_inventoryObjectsOffset + 6 < _inventoryObjectsCount) {
		_inventoryObjectsOffset += 3;
		_forceRedrawPanelItems = true;
	}
}

void TuckerEngine::moveDownInventoryObjects() {
	if (_inventoryObjectsOffset > 2) {
		_inventoryObjectsOffset -= 3;
		_forceRedrawPanelItems = true;
	}
}

void TuckerEngine::setActionVerbUnderCursor() {
	if (_mousePosY < 150) {
		_actionVerb = 0;
	} else if (_mousePosX > 195) {
		_actionVerb = 1;
	} else if (_panelNum == 0) {
		_actionVerb = ((_mousePosY - 150) / 17) * 3 + (_mousePosX / 67);
	} else {
		_actionVerb = 0;
		if (_mousePosX < 30) {
			_actionVerb = 7;
		} else if (_mousePosX > 130 && _mousePosX < 165) {
			_actionVerb = 5;
		} else {
			if (_mousePosY < 175) {
				if (_mousePosX < 67) {
					_actionVerb = 3;
				} else if (_mousePosX > 164) {
					_actionVerb = 6;
				} else if (_mousePosX > 99) {
					_actionVerb = 4;
				}
			} else {
				if (_mousePosX < 85) {
					_actionVerb = 1;
				} else if (_mousePosX > 165) {
					_actionVerb = 2;
				} else {
					_actionVerb = 8;
				}
			}
		}
	}
}

int TuckerEngine::getObjectUnderCursor() {
	if (_mousePosY > 140) {
		return -1;
	}
	for (int i = 0; i < _locationObjectsCount; ++i) {
		if (_mousePosX + _scrollOffset + 1 <= _locationObjectsTable[i].xPos) {
			continue;
		}
		if (_mousePosX + _scrollOffset >= _locationObjectsTable[i].xPos + _locationObjectsTable[i].xSize) {
			continue;
		}
		if (_mousePosY <= _locationObjectsTable[i].yPos) {
			continue;
		}
		if (_mousePosY >= _locationObjectsTable[i].yPos + _locationObjectsTable[i].ySize) {
			continue;
		}
		_selectedObjectType = 0;
		_selectedCharacterNum = i;
		setCursorNum(_locationObjectsTable[i].cursorNum);
		return i;
	}
	return -1;
}

void TuckerEngine::setSelectedObjectKey() {
	const int x = _mousePosX + _scrollOffset;
	if (_mousePosY > 139 && _nextAction == 0) {
		return;
	}
	_panelLockedFlag = 1;
	_locationMaskCounter = 0;
	_actionRequiresTwoObjects = false;
	_selectedObject.yPos = 0;
	_selectedObject.locationObject_locationNum = 0;
	_pendingActionIndex = 0;
	if (_selectedObjectType == 0) {
		if (_selectedObjectNum == 0) {
			_selectedObject.xPos = x;
			_selectedObject.yPos = _mousePosY;
		} else {
			_selectedObject.xPos = _locationObjectsTable[_selectedCharacterNum].standX;
			_selectedObject.yPos = _locationObjectsTable[_selectedCharacterNum].standY;
			if (_actionVerb == 0 || _actionVerb == 8) {
				_selectedObject.locationObject_locationNum = _locationObjectsTable[_selectedCharacterNum].locationNum;
				_selectedObject.locationObject_toX = _locationObjectsTable[_selectedCharacterNum].toX;
				_selectedObject.locationObject_toY = _locationObjectsTable[_selectedCharacterNum].toY;
				_selectedObject.locationObject_toX2 = _locationObjectsTable[_selectedCharacterNum].toX2;
				_selectedObject.locationObject_toY2 = _locationObjectsTable[_selectedCharacterNum].toY2;
				_selectedObject.locationObject_toWalkX2 = _locationObjectsTable[_selectedCharacterNum].toWalkX2;
				_selectedObject.locationObject_toWalkY2 = _locationObjectsTable[_selectedCharacterNum].toWalkY2;
			}
		}
	} else {
		switch (_selectedObjectType) {
		case 1:
			_selectedObject.xPos = _locationAnimationsTable[_selectedCharacterNum].standX;
			_selectedObject.yPos = _locationAnimationsTable[_selectedCharacterNum].standY;
			break;
		case 2:
			_selectedObject.xPos = _charPosTable[_selectedCharacterNum].xWalkTo;
			_selectedObject.yPos = _charPosTable[_selectedCharacterNum].yWalkTo;
			break;
		case 3:
			_selectedObject.xPos = _xPosCurrent;
			_selectedObject.yPos = _yPosCurrent;
			break;
		}
	}
	if (_selectedObject.yPos == 0) {
		_selectedObject.xPos = x;
		_selectedObject.yPos = _mousePosY;
	}
	_selectedObjectLocationMask = testLocationMask(_selectedObject.xPos, _selectedObject.yPos);
	if (_selectedObjectLocationMask == 0 && _objectKeysLocationTable[_locationNum] == 1) {
		if (_selectedObject.yPos < _objectKeysPosYTable[_locationNum]) {
			while (_selectedObjectLocationMask == 0 && _selectedObject.yPos < _objectKeysPosYTable[_locationNum]) {
				++_selectedObject.yPos;
				_selectedObjectLocationMask = testLocationMask(_selectedObject.xPos, _selectedObject.yPos);
			}
		} else {
			while (_selectedObjectLocationMask == 0 && _selectedObject.yPos < _objectKeysPosYTable[_locationNum]) {
				--_selectedObject.yPos;
				_selectedObjectLocationMask = testLocationMask(_selectedObject.xPos, _selectedObject.yPos);
			}
		}
	}
	if (_selectedObjectLocationMask == 1) {
		_selectedObjectLocationMask = testLocationMaskArea(_xPosCurrent, _yPosCurrent, _selectedObject.xPos, _selectedObject.yPos);
		if (_selectedObjectLocationMask == 1 && _objectKeysPosXTable[_locationNum] > 0) {
			_selectedObject.xDefaultPos = _selectedObject.xPos;
			_selectedObject.yDefaultPos = _selectedObject.yPos;
			_selectedObject.xPos = _objectKeysPosXTable[_locationNum];
			_selectedObject.yPos = _objectKeysPosYTable[_locationNum];
			if (_objectKeysLocationTable[_locationNum] == 1) {
				_selectedObject.xPos = _selectedObject.xDefaultPos;
			}
		}
	}
}

void TuckerEngine::setCharacterAnimation(int count, int spr) {
	_spritesTable[spr].animationFrame = 0;
	_spritesTable[spr].stateIndex = 0;
	for (int i = 0; i < count; ++i) {
		while (_characterStateTable[_spritesTable[spr].stateIndex] != 99) {
			++_spritesTable[spr].stateIndex;
		}
		++_spritesTable[spr].stateIndex;
	}
	_spritesTable[spr].state = _characterStateTable[_spritesTable[spr].stateIndex];
	++_spritesTable[spr].stateIndex;
	_spritesTable[spr].animationFrame = _characterStateTable[_spritesTable[spr].stateIndex];
	++_spritesTable[spr].stateIndex;
	_spritesTable[spr].animationData = _sprC02Table[_spritesTable[spr].state];
	_spritesTable[spr].firstFrame = READ_LE_UINT16(_spritesTable[spr].animationData);
}

int TuckerEngine::testLocationMaskArea(int xBase, int yBase, int xPos, int yPos) {
	while (true) {
		bool loop = false;
		if (yBase > yPos) {
			if (testLocationMask(xBase, yBase - 1) == 1) {
				--yBase;
				loop = true;
			}
		} else if (yBase < yPos) {
			if (testLocationMask(xBase, yBase + 1) == 1) {
				++yBase;
				loop = true;
			}
		}
		if (xBase > xPos) {
			if (testLocationMask(xBase - 1, yBase) == 1) {
				--xBase;
				loop = true;
			}
		} else if (xBase < xPos) {
			if (testLocationMask(xBase + 1, yBase) == 1) {
				++xBase;
				loop = true;
			}
		}
		if (xBase == xPos && yBase == yPos) {
			return 0;
		}
		if (!loop) {
			break;
		}
	}
	return 1;
}

void TuckerEngine::handleMouseClickOnInventoryObject() {
	if (_panelState != 0) {
		return;
	}
	if (_mousePosY < 150 || _mousePosX < 212) {
		return;
	}
	int pos = ((_mousePosY - 150) / 25) * 3 + (_mousePosX - 212) / 36;
	int obj = _inventoryObjectsOffset + pos;
	if (_inventoryObjectsCount - 1 < obj) {
		return;
	}
	_selectedObjectNum = _inventoryObjectsList[obj];
	_selectedObjectType = 3;
	switch (_selectedObjectNum) {
	case 30:
		if (_leftMouseButtonPressed) {
			_selectedObjectType = 0;
			_selectedObjectNum = 0;
			_actionVerb = 0;
			_actionVerbLocked = 0;
			_forceRedrawPanelItems = true;
			_panelState = 2;
			setCursorType(1);
		}
		break;
	case 1:
		if (_actionVerb == 8 && _leftMouseButtonPressed) {
			if (_mapSequenceFlagsLocationTable[_locationNum - 1] == 1) {
				handleMapSequence();
			} else {
				_actionPosX = _xPosCurrent;
				_actionPosY = _yPosCurrent - 64;
				_actionTextColor = 1;
				_actionCharacterNum = 99;
				setCursorType(2);
				_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
				_currentActionVerb = 0;
				_speechSoundNum = 2235;
				startSpeechSound(_speechSoundNum, _speechVolume);
				_characterSpeechDataPtr = _ptTextBuf + getPositionForLine(_speechSoundNum, _ptTextBuf);
				_speechSoundNum = 0;
				_actionVerb = 0;
				_selectedObjectType = 0;
				_selectedObjectNum = 0;
				_actionVerbLocked = 0;
			}
		}
		break;
	}
}

int TuckerEngine::setCharacterUnderCursor() {
	if (_mousePosY > 140) {
		return -1;
	}
	for (int i = 0; i < _charPosCount; ++i) {
		if (_mousePosX + _scrollOffset <= _charPosTable[i].xPos) {
			continue;
		}
		if (_mousePosX + _scrollOffset >= _charPosTable[i].xPos + _charPosTable[i].xSize) {
			continue;
		}
		if (_mousePosY <= _charPosTable[i].yPos) {
			continue;
		}
		if (_mousePosY >= _charPosTable[i].yPos + _charPosTable[i].ySize) {
			continue;
		}
		if (_charPosTable[i].flagNum == 0 || _flagsTable[_charPosTable[i].flagNum] == _charPosTable[i].flagValue) {
			_selectedObjectType = 2;
			_selectedCharacterDirection = _charPosTable[i].direction;
			_selectedCharacterNum = i;
			return _charPosTable[i].name;
		}
	}
	return -1;
}

int TuckerEngine::setLocationAnimationUnderCursor() {
	if (_mousePosY > 140) {
		return -1;
	}
	for (int i = _locationAnimationsCount - 1; i >= 0; --i) {
		if (_locationAnimationsTable[i].drawFlag == 0) {
			continue;
		}
		int num = _locationAnimationsTable[i].graphicNum;
		if (_mousePosX + _scrollOffset + 1 <= _dataTable[num].xDest) {
			continue;
		}
		if (_mousePosX + _scrollOffset >= _dataTable[num].xDest + _dataTable[num].xSize) {
			continue;
		}
		if (_mousePosY <= _dataTable[num].yDest) {
			continue;
		}
		if (_mousePosY >= _dataTable[num].yDest + _dataTable[num].ySize) {
			continue;
		}
		if (_locationAnimationsTable[i].selectable == 0) {
			return -1;
		}
		_selectedObjectType = 1;
		_selectedCharacterNum = i;
		_selectedCharacter2Num = i;
		return _locationAnimationsTable[i].selectable;
	}
	return -1;
}

void TuckerEngine::setActionForInventoryObject() {
	if (_actionVerb == 0 || _actionVerb == 2 || _actionVerb == 6 || _actionVerb == 7) {
		playSpeechForAction(_actionVerb);
		_actionVerbLocked = 0;
		_actionRequiresTwoObjects = false;
		return;
	}
	if (_actionVerb == 3 || _actionVerb == 4) {
		if (!(_partNum == 2 && _selectedObjectNum == 19) && !(_partNum == 3 && _selectedObjectNum == 42)) {
			playSpeechForAction(_actionVerb);
			_actionVerbLocked = 0;
			_actionRequiresTwoObjects = false;
			return;
		}
	}
	_currentActionObj1Num = _actionObj1Num;
	_currentInfoString1SourceType = _actionObj1Type;
	_currentActionObj2Num = _actionObj2Num;
	_currentInfoString2SourceType = _actionObj2Type;
	if (_actionVerb == 1 && _selectedObjectType == 3) {
		if (_panelLockedFlag == 1) {
			if (_locationMaskType != 0) {
				return;
			}
			_panelLockedFlag = 0;
		}
		if (handleSpecialObjectSelectionSequence() == 1) {
			return;
		}
		_speechSoundNum = _actionObj1Num + _speechSoundBaseNum;
		startSpeechSound(_speechSoundNum, _speechVolume);
		_characterSpeechDataPtr = _ptTextBuf + getPositionForLine(_speechSoundNum, _ptTextBuf);
		_speechSoundNum = 0;
		_actionPosX = _xPosCurrent;
		_actionPosY = _yPosCurrent - 64;
		_actionTextColor = 1;
		_actionCharacterNum = 99;
		setCursorType(2);
		_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
		_actionVerbLocked = 0;
		_actionRequiresTwoObjects = false;
		return;
	}
	if ((_partNum == 3 && (_actionObj1Num == 6 || _actionObj1Num == 3 || _actionObj1Num == 17)) ||
		(_partNum == 2 && _actionObj1Num == 19) ||
		(_partNum == 3 && (_actionObj1Num == 42 && _selectedObjectNum == 18)) ) {
		_actionVerbLocked = 0;
		_actionRequiresTwoObjects = false;
		_locationMaskCounter = 1;
		setActionState();
		return;
	}
	if (!_actionRequiresTwoObjects) {
		_actionRequiresTwoObjects = true;
	} else {
		_actionVerbLocked = 0;
		_actionRequiresTwoObjects = false;
		_locationMaskCounter = 1;
		setActionState();
	}
}

void TuckerEngine::setActionState() {
	_currentActionVerb = (_actionVerb == 0) ? 8 : _actionVerb;
	_currentActionObj1Num = _actionObj1Num;
	_currentInfoString1SourceType = _actionObj1Type;
	_currentActionObj2Num = _actionObj2Num;
	_currentInfoString2SourceType = _actionObj2Type;
	_actionRequiresTwoObjects = false;
	if (_selectedObjectType < 3) {
		setSelectedObjectKey();
	}
}

void TuckerEngine::playSpeechForAction(int i) {
	static const int speechActionTable[] = { 0, 2235, 2235, 2251, 2261, 2276, 2294, 2312, 2235 };
	static const int maxCounterTable[] = { 0, 1, 13, 7, 12, 15, 15, 15, 14 };
	++_speechActionCounterTable[i];
	if (_speechActionCounterTable[i] > maxCounterTable[i]) {
		_speechActionCounterTable[i] = 0;
	}
	if (speechActionTable[i] >= 2000) {
		if (_currentActionVerb == 8 && _currentActionObj1Num == 6 && _currentInfoString1SourceType == 3) {
			_speechSoundNum = 2395;
		} else {
			_speechSoundNum = _speechActionCounterTable[i] + speechActionTable[i];
		}
		startSpeechSound(_speechSoundNum, _speechVolume);
		_characterSpeechDataPtr = _ptTextBuf + getPositionForLine(_speechSoundNum, _ptTextBuf);
		_speechSoundNum = 0;
		_actionPosX = _xPosCurrent;
		_actionPosY = _yPosCurrent - 64;
		_actionTextColor = 1;
		_actionCharacterNum = 99;
		setCursorType(2);
		_charSpeechSoundCounter = kDefaultCharSpeechSoundCounter;
	}
}

void TuckerEngine::drawSpeechText(int xStart, int y, const uint8 *dataPtr, int num, int color) {
	int x = (xStart - _scrollOffset) * 2;
	int offset = (_scrollOffset + 320 - xStart) * 2;
	if (_conversationOptionsCount > 0) {
		x = 304;
	} else {
		if (x > offset) {
			x = offset;
		}
		if (x > 180) {
			x = 180;
		} else if (x < 150) {
			x = 150;
		}
	}
	int count = 0;
	int flag = 0;
	struct {
		int w, count, offset;
	} lines[5];
	lines[0].offset = getPositionForLine(num, dataPtr);
	while (flag == 0 && count < 4) {
		int lineCharsCount, lineWidth;
		flag = splitSpeechTextLines(dataPtr, lines[count].offset, x, lineCharsCount, lineWidth);
		lines[count].w = lineWidth;
		lines[count].count = lineCharsCount;
		lines[count + 1].offset = lines[count].offset + lineCharsCount + 1;
		++count;
	}
	if (count * 10 > y) {
		y = count * 10;
	}
	for (int i = 0; i < count; ++i) {
		int yPos, xPos = xStart - lines[i].w / 2;
		if (xPos < _scrollOffset) {
			xPos = _scrollOffset;
		} else if (xPos > _scrollOffset + 320 - lines[i].w) {
			xPos = _scrollOffset + 320 - lines[i].w;
		}
		if (_conversationOptionsCount != 0) {
			xPos = xStart + _scrollOffset;
			yPos = i * 10 + y;
			_conversationOptionLinesCount = count;
		} else {
			yPos = y - (count - i) * 10;
		}
		drawSpeechTextLine(dataPtr, lines[i].offset, lines[i].count, xPos, yPos, color);
	}
}

int TuckerEngine::splitSpeechTextLines(const uint8 *dataPtr, int pos, int x, int &lineCharsCount, int &lineWidth) {
	int count = 0;
	int w = 0;
	lineCharsCount = 0;
	lineWidth = 0;
	while (x + 1 > w && dataPtr[pos] != '\n' && dataPtr[pos] != '\r') {
		if (dataPtr[pos] == ' ') {
			lineCharsCount = count;
			lineWidth = w;
		}
		w += _charWidthTable[dataPtr[pos]];
		++count;
		++pos;
	}
	int ret = 0;
	if (x + 1 > w) {
		lineCharsCount = count;
		lineWidth = w;
		ret = 1;
	}
	return ret;
}

void TuckerEngine::drawSpeechTextLine(const uint8 *dataPtr, int pos, int count, int x, int y, uint8 color) {
	const int xStart = x;
	for (int i = 0; i < count && dataPtr[pos] != '\n'; ++i) {
		Graphics::drawStringChar(_locationBackgroundGfxBuf, x, y, 640, dataPtr[pos], color, _charsetGfxBuf);
		x += _charWidthTable[dataPtr[pos]];
		++pos;
	}
	addDirtyRect(xStart, y, x - xStart, Graphics::_charset.charH);
}

void TuckerEngine::redrawScreen(int offset) {
	debug(9, "redrawScreen() _fullRedraw %d offset %d _dirtyRectsCount %d", _fullRedraw, offset, _dirtyRectsCount);
	assert(offset <= kScreenWidth);
	if (_fullRedraw) {
		_fullRedraw = false;
		_system->copyRectToScreen(_locationBackgroundGfxBuf + offset, kScreenPitch, 0, 0, kScreenWidth, kScreenHeight);
	} else {
		Common::Rect clipRect(offset, 0, offset + kScreenWidth, kScreenHeight);
		for (int i = 0; i < _dirtyRectsPrevCount + _dirtyRectsCount; ++i) {
			redrawScreenRect(clipRect, _dirtyRectsTable[i]);
		}
	}
	if (_dirtyRectsPrevCount + _dirtyRectsCount < kMaxDirtyRects) {
		for (int i = 0; i < _dirtyRectsCount; ++i) {
			_dirtyRectsTable[i] = _dirtyRectsTable[_dirtyRectsPrevCount + i];
		}
		_dirtyRectsPrevCount = _dirtyRectsCount;
	} else {
		_dirtyRectsPrevCount = 0;
		_fullRedraw = true;
	}
	_dirtyRectsCount = 0;
	_system->updateScreen();
}

void TuckerEngine::redrawScreenRect(const Common::Rect &clip, const Common::Rect &dirty) {
	if (dirty.intersects(clip)) {
		Common::Rect r(dirty);
		r.clip(clip);
		const uint8 *src = _locationBackgroundGfxBuf + r.top * 640 + r.left;
		r.translate(-clip.left, -clip.top);
		const int w = r.right - r.left;
		const int h = r.bottom - r.top;
		if (w <= 0 || h <= 0) {
			return;
		}
#if 0
		static const uint8 outlineColor = 0;
		memset(_locationBackgroundGfxBuf + r.top           * 640 + r.left, outlineColor, w);
		memset(_locationBackgroundGfxBuf + (r.top + h - 1) * 640 + r.left, outlineColor, w);
		for (int y = r.top; y < r.top + h; ++y) {
			_locationBackgroundGfxBuf[y * 640 + r.left] = outlineColor;
			_locationBackgroundGfxBuf[y * 640 + r.left + w - 1] = outlineColor;
		}
#endif
		_system->copyRectToScreen(src, 640, r.left, r.top, w, h);
	}
}

void TuckerEngine::addDirtyRect(int x, int y, int w, int h) {
	if (_dirtyRectsPrevCount + _dirtyRectsCount < kMaxDirtyRects) {
		Common::Rect r(x, y, x + w, y + h);
		for (int i = 0; i < _dirtyRectsCount; ++i) {
			if (_dirtyRectsTable[_dirtyRectsPrevCount + i].contains(r)) {
				return;
			}
		}
		_dirtyRectsTable[_dirtyRectsPrevCount + _dirtyRectsCount] = r;
		++_dirtyRectsCount;
	} else {
		_fullRedraw = true;
	}
}

} // namespace Tucker
