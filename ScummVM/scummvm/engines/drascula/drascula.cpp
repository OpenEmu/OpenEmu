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

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

#include "backends/audiocd/audiocd.h"

#include "base/plugins.h"
#include "base/version.h"

#include "engines/util.h"

#include "audio/mixer.h"

#include "drascula/drascula.h"
#include "drascula/console.h"

namespace Drascula {

struct GameSettings {
	const char *gameid;
	const char *description;
	byte id;
	uint32 features;
	const char *detectname;
};

static const GameSettings drasculaSettings[] = {
	{"drascula", "Drascula game", 0, 0, 0},

	{NULL, NULL, 0, 0, NULL}
};

DrasculaEngine::DrasculaEngine(OSystem *syst, const DrasculaGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_charMap = 0;
	_itemLocations = 0;
	_polX = 0;
	_polY = 0;
	_verbBarX = 0;
	_x1d_menu = 0;
	_y1d_menu = 0;
	_frameX = 0;
	_candleX = 0;
	_candleY = 0;
	_pianistX = 0;
	_drunkX = 0;
	_roomPreUpdates = 0;
	_roomUpdates = 0;
	_roomActions = 0;
	_text = 0;
	_textd = 0;
	_textb = 0;
	_textbj = 0;
	_texte = 0;
	_texti = 0;
	_textl = 0;
	_textp = 0;
	_textt = 0;
	_textvb = 0;
	_textsys = 0;
	_texthis = 0;
	_textverbs = 0;
	_textmisc = 0;
	_textd1 = 0;
	_talkSequences = 0;

	_color = 0;
	blinking = 0;
	mouseX = 0;
	mouseY = 0;
	leftMouseButton = 0;
	rightMouseButton = 0;
	*textName = 0;

	_rnd = new Common::RandomSource("drascula");

	_console = 0;

	int cd_num = ConfMan.getInt("cdrom");
	if (cd_num >= 0)
		_system->getAudioCDManager()->openCD(cd_num);

	_lang = kEnglish;

	_keyBufferHead = _keyBufferTail = 0;

	_roomHandlers = 0;
}

DrasculaEngine::~DrasculaEngine() {
	delete _rnd;
	stopSound();

	freeRoomsTable();

	delete _console;

	free(_charMap);
	free(_itemLocations);
	free(_polX);
	free(_polY);
	free(_verbBarX);
	free(_x1d_menu);
	free(_y1d_menu);
	free(_frameX);
	free(_candleX);
	free(_candleY);
	free(_pianistX);
	free(_drunkX);
	free(_roomPreUpdates);
	free(_roomUpdates);
	free(_roomActions);
	free(_talkSequences);
	freeTexts(_text);
	freeTexts(_textd);
	freeTexts(_textb);
	freeTexts(_textbj);
	freeTexts(_texte);
	freeTexts(_texti);
	freeTexts(_textl);
	freeTexts(_textp);
	freeTexts(_textt);
	freeTexts(_textvb);
	freeTexts(_textsys);
	freeTexts(_texthis);
	freeTexts(_textverbs);
	freeTexts(_textmisc);
	freeTexts(_textd1);
}

bool DrasculaEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

Common::Error DrasculaEngine::run() {
	// Initialize backend
	initGraphics(320, 200, false);

	switch (getLanguage()) {
	case Common::EN_ANY:
		_lang = kEnglish;
		break;
	case Common::ES_ESP:
		_lang = kSpanish;
		break;
	case Common::DE_DEU:
		_lang = kGerman;
		break;
	case Common::FR_FRA:
		_lang = kFrench;
		break;
	case Common::IT_ITA:
		_lang = kItalian;
		break;
	default:
		warning("Unknown game language. Falling back to English");
		_lang = kEnglish;
	}

	_console = new Console(this);

	if (!loadDrasculaDat())
		return Common::kUnknownError;

	setupRoomsTable();
	loadArchives();

	// Setup mixer
	syncSoundSettings();

	currentChapter = 1; // values from 1 to 6 will start each part of game
	loadedDifferentChapter = 0;

	checkCD();

	while (!shouldQuit()) {
		int i;
		takeObject = 0;
		_menuBar = false;
		_menuScreen = false;
		_hasName = false;
		frame_y = 0;
		curX = -1;
		characterMoved = 0;
		trackProtagonist = 3;
		num_frame = 0;
		hare_se_ve = 1;
		checkFlags = 1;
		doBreak = 0;
		walkToObject = 0;

		stepX = STEP_X;
		stepY = STEP_Y;

		curHeight = CHARACTER_HEIGHT;
		curWidth = CHARACTER_WIDTH;
		feetHeight = FEET_HEIGHT;

		talkHeight = TALK_HEIGHT;
		talkWidth = TALK_WIDTH;

		hasAnswer = 0;
		savedTime = 0;
		breakOut = 0;
		vonBraunX = 120;
		trackVonBraun = 1;
		vonBraunHasMoved = 0;
		framesWithoutAction = 0;
		term_int = 0;
		musicStopped = 0;
		selectionMade = 0;
		globalSpeed = 0;
		curExcuseLook = 0;
		curExcuseAction = 0;
		roomNumber = 0;

		for (i = 0; i < 8; i++)
			actorFrames[i] = 0;
		actorFrames[kFrameVonBraun] = 1;

		allocMemory();

		_subtitlesDisabled = !ConfMan.getBool("subtitles");
		selectionMade = 0;

		if (currentChapter != 3)
			loadPic(96, frontSurface, COMPLETE_PAL);

		loadPic(99, cursorSurface);

		if (currentChapter == 1) {
		} else if (currentChapter == 2) {
			loadPic("pts.alg", drawSurface2);
		} else if (currentChapter == 3) {
			loadPic("aux13.alg", bgSurface, COMPLETE_PAL);
			loadPic(96, frontSurface);
		} else if (currentChapter == 4) {
			if (loadedDifferentChapter == 0)
				animation_ray();
			loadPic(96, frontSurface);
			clearRoom();
		} else if (currentChapter == 5) {
		} else if (currentChapter == 6) {
			igorX = 105;
			igorY = 85;
			trackIgor = 1;
			drasculaX = 62;
			drasculaY = 99;
			trackDrascula = 1;
			actorFrames[kFramePendulum] = 0;
			flag_tv = 0;
		}

		loadPic(95, tableSurface);
		for (i = 0; i < 25; i++)
			memcpy(crosshairCursor + i * 40, tableSurface + 225 + (56 + i) * 320, 40);

		if (_lang == kSpanish)
			loadPic(974, tableSurface);

		if (currentChapter != 2) {
			loadPic(99, cursorSurface);
			loadPic(99, backSurface);
			loadPic(97, extraSurface);
		}

		memset(iconName, 0, sizeof(iconName));

		for (i = 0; i < 6; i++)
			strcpy(iconName[i + 1], _textverbs[i]);

		assignPalette(defaultPalette);
		if (!runCurrentChapter()) {
			endChapter();
			break;
		}
		endChapter();
		if (currentChapter == 6)
			break;

		currentChapter++;
	}

	return Common::kNoError;
}

void DrasculaEngine::endChapter() {
	stopSound();
	clearRoom();
	black();
	MusicFadeout();
	stopMusic();
	freeMemory();
}

bool DrasculaEngine::runCurrentChapter() {
	int n;

	rightMouseButton = 0;

	previousMusic = -1;

	if (currentChapter != 2) {
		int soc = 0;
		for (n = 0; n < 6; n++) {
			soc = soc + CHARACTER_WIDTH;
			_frameX[n] = soc;
		}
	}

	for (n = 1; n < ARRAYSIZE(inventoryObjects); n++)
		inventoryObjects[n] = 0;

	for (n = 0; n < NUM_FLAGS; n++)
		flags[n] = 0;

	if (currentChapter == 2) {
		flags[16] = 1;
		flags[17] = 1;
		flags[27] = 1;
	}

	for (n = 1; n < 7; n++)
		inventoryObjects[n] = n;

	if (currentChapter == 1) {
		pickObject(28);

		if (loadedDifferentChapter == 0)
			animation_1_1();

		selectVerb(kVerbNone);
		loadPic("2aux62.alg", drawSurface2);
		trackProtagonist = 1;
		objExit = 104;
		if (loadedDifferentChapter != 0) {
			if (!loadGame(saveName)) {
				return true;
			}
		} else {
			enterRoom(62);
			curX = -20;
			curY = 56;
			gotoObject(65, 145);
		}
	} else if (currentChapter == 2) {
		addObject(kItemPhone);
		trackProtagonist = 3;
		objExit = 162;
		if (loadedDifferentChapter == 0)
			enterRoom(14);
		else {
			if (!loadGame(saveName)) {
				return true;
			}
		}
	} else if (currentChapter == 3) {
		addObject(kItemPhone);
		addObject(kItemEarplugs);
		addObject(kItemSickle);
		addObject(kItemHandbag);
		addObject(kItemCross);
		addObject(kItemReefer);
		addObject(kItemOneCoin);
		flags[1] = 1;
		trackProtagonist = 1;
		objExit = 99;
		if (loadedDifferentChapter == 0)
			enterRoom(20);
		else {
			if (!loadGame(saveName)) {
				return true;
			}
		}
	// From here onwards the items have different IDs
	} else if (currentChapter == 4) {
		addObject(kItemPhone2);
		addObject(kItemCross2);
		addObject(kItemReefer2);
		addObject(kItemOneCoin2);
		objExit = 100;
		if (loadedDifferentChapter == 0) {
			enterRoom(21);
			trackProtagonist = 0;
			curX = 235;
			curY = 164;
		} else {
			if (!loadGame(saveName)) {
				return true;
			}
		}
	} else if (currentChapter == 5) {
		addObject(28);
		addObject(7);
		addObject(9);
		addObject(11);
		addObject(13);
		addObject(14);
		addObject(15);
		addObject(17);
		addObject(20);
		trackProtagonist = 1;
		objExit = 100;
		if (loadedDifferentChapter == 0) {
			enterRoom(45);
		} else {
			if (!loadGame(saveName)) {
				return true;
			}
		}
	} else if (currentChapter == 6) {
		addObject(28);
		addObject(9);

		trackProtagonist = 1;
		objExit = 104;
		if (loadedDifferentChapter == 0) {
			enterRoom(58);
			animation_1_6();
		} else {
			if (!loadGame(saveName)) {
				return true;
			}
			loadPic("auxdr.alg", drawSurface2);
		}
	}

	showCursor();

	while (!shouldQuit()) {
		if (characterMoved == 0) {
			stepX = STEP_X;
			stepY = STEP_Y;
		}
		if (characterMoved == 0 && walkToObject == 1) {
			trackProtagonist = trackFinal;
			walkToObject = 0;
		}

		if (currentChapter == 2) {
			// NOTE: the checks for room number 14 below are a hack used in the original
			// game, and move the character to a place where his feet are not drawn above
			// the pianist's head. Originally, walkToObject was not updated properly, which
			// lead to an incorrect setting of the protagonist's tracking flag (above). This
			// made the character start walking off screen, as his actual position was
			// different than the displayed one
			if (roomNumber == 3 && (curX == 279) && (curY + curHeight == 101)) {
				gotoObject(178, 121);
				gotoObject(169, 135);
			} else if (roomNumber == 14 && (curX == 214) && (curY + curHeight == 121)) {
				walkToObject = 1;
				gotoObject(190, 130);
			} else if (roomNumber == 14 && (curX == 246) && (curY + curHeight == 112)) {
				walkToObject = 1;
				gotoObject(190, 130);
			}
		}

		moveCursor();
		updateScreen();

		if (currentChapter == 2) {
			if (musicStatus() == 0 && roomMusic != 0)
				playMusic(roomMusic);
		} else {
			if (musicStatus() == 0)
				playMusic(roomMusic);
		}

		delay(25);
#ifndef _WIN32_WCE
		// FIXME
		// This and the following #ifndefs disable the excess updateEvents() calls *within* the game loop.
		// Events such as keypresses or mouse clicks are dropped on the ground with no processing
		// by these calls. They are properly handled by the implicit call through getScan() below.
		// It is not a good practice to not process events and indeed this created problems with synthesized
		// events in the wince port.
		updateEvents();
#endif

		if (!_menuScreen && takeObject == 1)
			checkObjects();

#ifdef _WIN32_WCE
		if (rightMouseButton) {
			if (_menuScreen) {
#else
		if (rightMouseButton == 1 && _menuScreen) {
#endif
			rightMouseButton = 0;
			delay(100);
			if (currentChapter == 2) {
				loadPic(menuBackground, cursorSurface);
				loadPic(menuBackground, backSurface);
			} else {
				loadPic(99, cursorSurface);
				loadPic(99, backSurface);
			}
			setPalette((byte *)&gamePalette);
			_menuScreen = false;
#ifndef _WIN32_WCE
			// FIXME: This call here is in hope that it will catch the rightmouseup event so the
			// next if block won't be executed. This too is not a good coding practice. I've recoded it
			// with a mutual exclusive if block for the menu. I would commit this properly but I cannot test
			// for other (see Desktop) ports right now.
			updateEvents();
#endif
#ifdef _WIN32_WCE
			} else {
#else
		}

		// Do not show the inventory screen in chapter 5, if the right mouse button is clicked
		// while the plug (object 16) is held
		// Fixes bug #2059621 - "DRASCULA: Plug bug"
		if (rightMouseButton == 1 && !_menuScreen &&
			!(currentChapter == 5 && pickedObject == 16)) {
#endif
			rightMouseButton = 0;
			delay(100);
			characterMoved = 0;
			if (trackProtagonist == 2)
				trackProtagonist = 1;
			if (currentChapter == 4) {
				loadPic("icons2.alg", backSurface);
				loadPic("icons2.alg", cursorSurface);
			} else if (currentChapter == 5) {
				loadPic("icons3.alg", backSurface);
				loadPic("icons3.alg", cursorSurface);
			} else if (currentChapter == 6) {
				loadPic("iconsp.alg", backSurface);
				loadPic("iconsp.alg", cursorSurface);
			} else {
				loadPic("icons.alg", backSurface);
				loadPic("icons.alg", cursorSurface);
			}
			_menuScreen = true;
#ifndef _WIN32_WCE
			updateEvents();
#endif
			selectVerb(kVerbNone);
		}
#ifdef _WIN32_WCE
		}
#endif

		if (leftMouseButton == 1 && _menuBar) {
			delay(100);
			selectVerbFromBar();
		} else if (leftMouseButton == 1 && takeObject == 0) {
			delay(100);
			if (verify1())
				return true;
		} else if (leftMouseButton == 1 && takeObject == 1) {
			if (verify2())
				return true;
		}

		_menuBar = (mouseY < 24 && !_menuScreen) ? true : false;

		Common::KeyCode key = getScan();
		if (key == Common::KEYCODE_F1 && !_menuScreen) {
			selectVerb(kVerbLook);
		} else if (key == Common::KEYCODE_F2 && !_menuScreen) {
			selectVerb(kVerbPick);
		} else if (key == Common::KEYCODE_F3 && !_menuScreen) {
			selectVerb(kVerbOpen);
		} else if (key == Common::KEYCODE_F4 && !_menuScreen) {
			selectVerb(kVerbClose);
		} else if (key == Common::KEYCODE_F5 && !_menuScreen) {
			selectVerb(kVerbTalk);
		} else if (key == Common::KEYCODE_F6 && !_menuScreen) {
			selectVerb(kVerbMove);
		} else if (key == Common::KEYCODE_F9) {
			volumeControls();
		} else if (key == Common::KEYCODE_F10) {
			if (!saveLoadScreen())
				return true;
		} else if (key == Common::KEYCODE_F8) {
			selectVerb(kVerbNone);
		} else if (key == Common::KEYCODE_v) {
			_subtitlesDisabled = true;
			ConfMan.setBool("subtitles", !_subtitlesDisabled);

			print_abc(_textsys[2], 96, 86);
			updateScreen();
			delay(1410);
		} else if (key == Common::KEYCODE_t) {
			_subtitlesDisabled = false;
			ConfMan.setBool("subtitles", !_subtitlesDisabled);

			print_abc(_textsys[3], 94, 86);
			updateScreen();
			delay(1460);
		} else if (key == Common::KEYCODE_ESCAPE) {
			if (!confirmExit())
				return false;
		} else if (key == Common::KEYCODE_TILDE || key == Common::KEYCODE_BACKQUOTE) {
			_console->attach();
			_console->onFrame();
		} else if (currentChapter == 6 && key == Common::KEYCODE_0 && roomNumber == 61) {
			loadPic("alcbar.alg", bgSurface, 255);
		}

		if (leftMouseButton != 0 || rightMouseButton != 0 || key != 0)
			if (currentChapter != 3)
				framesWithoutAction = 0;

		if (framesWithoutAction == 15000) {
			screenSaver();
			if (currentChapter != 3)
				framesWithoutAction = 0;
		}

		if (currentChapter != 3)
			framesWithoutAction++;
	}

	return false;
}


bool DrasculaEngine::verify1() {
	int l;

	if (_menuScreen)
		removeObject();
	else {
		for (l = 0; l < numRoomObjs; l++) {
			if (mouseX >= x1[l] && mouseY >= y1[l]
					&& mouseX <= x2[l] && mouseY <= y2[l] && doBreak == 0) {
				if (exitRoom(l))
					return true;
				if (doBreak == 1)
					break;
			}
		}

		if (mouseX > curX && mouseY > curY
				&& mouseX < curX + curWidth && mouseY < curY + curHeight)
			doBreak = 1;

		for (l = 0; l < numRoomObjs; l++) {
			if (mouseX > x1[l] && mouseY > y1[l]
					&& mouseX < x2[l] && mouseY < y2[l] && doBreak == 0) {
				roomX = roomObjX[l];
				roomY = roomObjY[l];
				trackFinal = trackObj[l];
				doBreak = 1;
				walkToObject = 1;
				startWalking();
			}
		}

		if (doBreak == 0) {
			roomX = CLIP(mouseX, floorX1, floorX2);
			roomY = CLIP(mouseY, floorY1 + feetHeight, floorY2);
			startWalking();
		}
		doBreak = 0;
	}

	return false;
}

bool DrasculaEngine::verify2() {
	int l;

	if (_menuScreen) {
		if (pickupObject())
			return true;
	} else {
		if (!strcmp(textName, "hacker") && _hasName) {
			if (checkAction(50))
				return true;
		} else {
			for (l = 0; l < numRoomObjs; l++) {
				if (mouseX > x1[l] && mouseY > y1[l]
						&& mouseX < x2[l] && mouseY < y2[l] && visible[l] == 1) {
					trackFinal = trackObj[l];
					walkToObject = 1;
					gotoObject(roomObjX[l], roomObjY[l]);
					if (checkAction(objectNum[l]))
						return true;
					if (currentChapter == 4)
						break;
				}
			}
		}
	}

	return false;
}

Common::KeyCode DrasculaEngine::getScan() {
	updateEvents();
	if (_keyBufferHead == _keyBufferTail)
		return Common::KEYCODE_INVALID;

	Common::KeyCode key = _keyBuffer[_keyBufferTail].keycode;
	_keyBufferTail = (_keyBufferTail + 1) % KEYBUFSIZE;

	return key;
}

void DrasculaEngine::addKeyToBuffer(Common::KeyState& key) {
	if ((_keyBufferHead + 1) % KEYBUFSIZE == _keyBufferTail) {
		warning("key buffer overflow");
		return;
	}

	_keyBuffer[_keyBufferHead] = key;
	_keyBufferHead = (_keyBufferHead + 1) % KEYBUFSIZE;
}

void DrasculaEngine::flushKeyBuffer() {
	updateEvents();
	_keyBufferHead = _keyBufferTail = 0;
}

void DrasculaEngine::updateEvents() {
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	updateMusic();

#ifdef _WIN32_WCE
	if (eventMan->pollEvent(event)) {
#else
	while (eventMan->pollEvent(event)) {
#endif
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			addKeyToBuffer(event.kbd);
			break;
		case Common::EVENT_KEYUP:
			break;
		case Common::EVENT_MOUSEMOVE:
			mouseX = event.mouse.x;
			mouseY = event.mouse.y;
			break;
		case Common::EVENT_LBUTTONDOWN:
			leftMouseButton = 1;
			break;
		case Common::EVENT_LBUTTONUP:
			leftMouseButton = 0;
			break;
		case Common::EVENT_RBUTTONDOWN:
			// We changed semantic and react only on button up event
			break;
		case Common::EVENT_RBUTTONUP:
			rightMouseButton = 1;
			break;
		default:
			break;
		}
	}
}

void DrasculaEngine::delay(int ms) {
	uint32 end = _system->getMillis() + ms * 2; // originally was 1

	do {
		_system->delayMillis(10);
		updateEvents();
		_system->updateScreen();
	} while (_system->getMillis() < end && !shouldQuit());
}

void DrasculaEngine::pause(int duration) {
	delay(duration * 15);
}

int DrasculaEngine::getTime() {
	return _system->getMillis() / 20; // originally was 1
}

void DrasculaEngine::reduce_hare_chico(int xx1, int yy1, int xx2, int yy2, int width, int height, int factor, byte *dir_inicio, byte *dir_fin) {
	float totalX, totalY;
	int n, m;
	float pixelX, pixelY;

	newWidth = (width * factor) / 100;
	newHeight = (height * factor) / 100;

	totalX = width / newWidth;
	totalY = height / newHeight;

	pixelX = xx1;
	pixelY = yy1;

	for (n = 0; n < newHeight; n++) {
		for (m = 0; m < newWidth; m++) {
			copyRect((int)pixelX, (int)pixelY, xx2 + m, yy2 + n,
					 1, 1, dir_inicio, dir_fin);

			pixelX += totalX;
		}
		pixelX = xx1;
		pixelY += totalY;
	}
}

void DrasculaEngine::hipo_sin_nadie(int counter){
	int y = 0, trackCharacter = 0;
	if (currentChapter == 3)
		y = -1;

	do {
		counter--;

		copyBackground();
		if (currentChapter == 3)
			updateScreen(0, 0, 0, y, 320, 200, screenSurface);
		else
			updateScreen(0, 1, 0, y, 320, 198, screenSurface);

		if (trackCharacter == 0)
			y++;
		else
			y--;

		if (currentChapter == 3) {
			if (y == 1)
				trackCharacter = 1;
			if (y == -1)
				trackCharacter = 0;
		} else {
			if (y == 2)
				trackCharacter = 1;
			if (y == 0)
				trackCharacter = 0;
		}
	} while (counter > 0);

	copyBackground();
	updateScreen();
}

bool DrasculaEngine::loadDrasculaDat() {
	Common::File in;
	int i;

	in.open("drascula.dat");

	if (!in.isOpen()) {
		Common::String errorMessage = "You're missing the 'drascula.dat' file. Get it from the ScummVM website";
		GUIErrorMessage(errorMessage);
		warning("%s", errorMessage.c_str());

		return false;
	}

	char buf[256];
	int ver;

	in.read(buf, 8);
	buf[8] = '\0';

	if (strcmp(buf, "DRASCULA") != 0) {
		Common::String errorMessage = "File 'drascula.dat' is corrupt. Get it from the ScummVM website";
		GUIErrorMessage(errorMessage);
		warning("%s", errorMessage.c_str());

		return false;
	}

	ver = in.readByte();

	if (ver != DRASCULA_DAT_VER) {
		Common::String errorMessage = Common::String::format("File 'drascula.dat' is wrong version. Expected %d but got %d. Get it from the ScummVM website", DRASCULA_DAT_VER, ver);
		GUIErrorMessage(errorMessage);
		warning("%s", errorMessage.c_str());

		return false;
	}

	_charMapSize = in.readUint16BE();
	_charMap = (CharInfo *)malloc(sizeof(CharInfo) * _charMapSize);
	for (i = 0; i < _charMapSize; i++) {
		_charMap[i].inChar = in.readByte();
		_charMap[i].mappedChar = in.readSint16BE();
		_charMap[i].charType = in.readByte();
	}

	_itemLocationsSize = in.readUint16BE();
	_itemLocations = (ItemLocation *)malloc(sizeof(ItemLocation) * _itemLocationsSize);
	for (i = 0; i < _itemLocationsSize; i++) {
		_itemLocations[i].x = in.readSint16BE();
		_itemLocations[i].y = in.readSint16BE();
	}

	_polXSize = in.readUint16BE();
	_polX = (int *)malloc(sizeof(int) * _polXSize);
	_polY = (int *)malloc(sizeof(int) * _polXSize);
	for (i = 0; i < _polXSize; i++) {
		_polX[i] = in.readSint16BE();
		_polY[i] = in.readSint16BE();
	}

	_verbBarXSize = in.readUint16BE();
	_verbBarX = (int *)malloc(sizeof(int) * _verbBarXSize);
	for (i = 0; i < _verbBarXSize; i++) {
		_verbBarX[i] = in.readSint16BE();
	}

	_x1dMenuSize = in.readUint16BE();
	_x1d_menu = (int *)malloc(sizeof(int) * _x1dMenuSize);
	_y1d_menu = (int *)malloc(sizeof(int) * _x1dMenuSize);
	for (i = 0; i < _x1dMenuSize; i++) {
		_x1d_menu[i] = in.readSint16BE();
		_y1d_menu[i] = in.readSint16BE();
	}

	_frameXSize = in.readUint16BE();
	_frameX = (int *)malloc(sizeof(int) * _frameXSize);
	for (i = 0; i < _frameXSize; i++) {
		_frameX[i] = in.readSint16BE();
	}

	_candleXSize = in.readUint16BE();
	_candleX = (int *)malloc(sizeof(int) * _candleXSize);
	_candleY = (int *)malloc(sizeof(int) * _candleXSize);
	for (i = 0; i < _candleXSize; i++) {
		_candleX[i] = in.readSint16BE();
		_candleY[i] = in.readSint16BE();
	}

	_pianistXSize = in.readUint16BE();
	_pianistX = (int *)malloc(sizeof(int) * _pianistXSize);
	for (i = 0; i < _pianistXSize; i++) {
		_pianistX[i] = in.readSint16BE();
	}

	_drunkXSize = in.readUint16BE();
	_drunkX = (int *)malloc(sizeof(int) * _drunkXSize);
	for (i = 0; i < _drunkXSize; i++) {
		_drunkX[i] = in.readSint16BE();
	}

	_roomPreUpdatesSize = in.readUint16BE();
	_roomPreUpdates = (RoomUpdate *)malloc(sizeof(RoomUpdate) * _roomPreUpdatesSize);
	for (i = 0; i < _roomPreUpdatesSize; i++) {
		_roomPreUpdates[i].roomNum = in.readSint16BE();
		_roomPreUpdates[i].flag = in.readSint16BE();
		_roomPreUpdates[i].flagValue = in.readSint16BE();
		_roomPreUpdates[i].sourceX = in.readSint16BE();
		_roomPreUpdates[i].sourceY = in.readSint16BE();
		_roomPreUpdates[i].destX = in.readSint16BE();
		_roomPreUpdates[i].destY = in.readSint16BE();
		_roomPreUpdates[i].width = in.readSint16BE();
		_roomPreUpdates[i].height = in.readSint16BE();
		_roomPreUpdates[i].type = in.readSint16BE();
	}

	_roomUpdatesSize = in.readUint16BE();
	_roomUpdates = (RoomUpdate *)malloc(sizeof(RoomUpdate) * _roomUpdatesSize);
	for (i = 0; i < _roomUpdatesSize; i++) {
		_roomUpdates[i].roomNum = in.readSint16BE();
		_roomUpdates[i].flag = in.readSint16BE();
		_roomUpdates[i].flagValue = in.readSint16BE();
		_roomUpdates[i].sourceX = in.readSint16BE();
		_roomUpdates[i].sourceY = in.readSint16BE();
		_roomUpdates[i].destX = in.readSint16BE();
		_roomUpdates[i].destY = in.readSint16BE();
		_roomUpdates[i].width = in.readSint16BE();
		_roomUpdates[i].height = in.readSint16BE();
		_roomUpdates[i].type = in.readSint16BE();
	}

	_roomActionsSize = in.readUint16BE();
	_roomActions = (RoomTalkAction *)malloc(sizeof(RoomTalkAction) * _roomActionsSize);
	for (i = 0; i < _roomActionsSize; i++) {
		_roomActions[i].room = in.readSint16BE();
		_roomActions[i].chapter = in.readSint16BE();
		_roomActions[i].action = in.readSint16BE();
		_roomActions[i].objectID = in.readSint16BE();
		_roomActions[i].speechID = in.readSint16BE();
	}

	_talkSequencesSize = in.readUint16BE();
	_talkSequences = (TalkSequenceCommand *)malloc(sizeof(TalkSequenceCommand) * _talkSequencesSize);
	for (i = 0; i < _talkSequencesSize; i++) {
		_talkSequences[i].chapter = in.readSint16BE();
		_talkSequences[i].sequence = in.readSint16BE();
		_talkSequences[i].commandType = in.readSint16BE();
		_talkSequences[i].action = in.readSint16BE();
	}

	_numLangs = in.readUint16BE();

	_text = loadTexts(in);
	_textd = loadTexts(in);
	_textb = loadTexts(in);
	_textbj = loadTexts(in);
	_texte = loadTexts(in);
	_texti = loadTexts(in);
	_textl = loadTexts(in);
	_textp = loadTexts(in);
	_textt = loadTexts(in);
	_textvb = loadTexts(in);
	_textsys = loadTexts(in);
	_texthis = loadTexts(in);
	_textverbs = loadTexts(in);
	_textmisc = loadTexts(in);
	_textd1 = loadTexts(in);

	return true;
}

char **DrasculaEngine::loadTexts(Common::File &in) {
	int numTexts = in.readUint16BE();
	char **res = (char **)malloc(sizeof(char *) * numTexts);
	int entryLen;
	char *pos = 0;
	int len;

	for (int lang = 0; lang < _numLangs; lang++) {
		entryLen = in.readUint16BE();
		if (lang == _lang) {
			pos = (char *)malloc(entryLen);
			in.read(pos, entryLen);
			res[0] = pos;
			pos += DATAALIGNMENT;

			for (int i = 1; i < numTexts; i++) {
				pos -= 2;

				len = READ_BE_UINT16(pos);
				pos += 2 + len;

				res[i] = pos;
			}
		} else
			in.seek(entryLen, SEEK_CUR);
	}

	return res;
}

void DrasculaEngine::freeTexts(char **ptr) {
	if (!ptr)
		return;

	free(*ptr);
	free(ptr);
}

} // End of namespace Drascula
