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

#include "dreamweb/sound.h"
#include "dreamweb/dreamweb.h"
#include "common/config-manager.h"

namespace DreamWeb {

// Keyboard buffer. _bufferIn and _bufferOut are indexes
// into this, making it a ring buffer
uint8 g_keyBuffer[16];

const Room g_roomData[] = {
	// location 0
	{ "DREAMWEB.R00", // Ryan's apartment
	  5,255,33,10,
	  255,255,255,0,
	  1,6,2,255,3,255,255,255,255,255,0 },

	// location 1
	{ "DREAMWEB.R01",
	  1,255,44,10,
	  255,255,255,0,
	  7,2,255,255,255,255,6,255,255,255,1 },

	// location 2: Louis' (?)
	{ "DREAMWEB.R02",
	  2,255,33,0,
	  255,255,255,0,
	  1,0,255,255,1,255,3,255,255,255,2 },

	// location 3
	{ "DREAMWEB.R03",
	  5,255,33,10,
	  255,255,255,0,
	  2,2,0,2,4,255,0,255,255,255,3 },

	// location 4
	{ "DREAMWEB.R04",
	  23,255,11,30,
	  255,255,255,0,
	  1,4,0,5,255,255,3,255,255,255,4 },

	// location 5: In hotel, lift noise audible (?)
	{ "DREAMWEB.R05",
	  5,255,22,30, // if demo: 22,255,22,30,
	  255,255,255,0,
	  1,2,0,4,255,255,3,255,255,255,5 },

	// location 6: sarters (?)
	{ "DREAMWEB.R06",
	  5,255,11,30,
	  255,255,255,0,
	  1,0,0,1,2,255,0,255,255,255,6 },

	// location 7
	{ "DREAMWEB.R07",
	  255,255,0,20,
	  255,255,255,0,
	  2,2,255,255,255,255,0,255,255,255,7 },

	// location 8: TV studio (?)
	{ "DREAMWEB.R08",
	  8,255,0,10,
	  255,255,255,0,
	  1,2,255,255,255,255,0,11,40,0,8 },

	// location 9
	{ "DREAMWEB.R09",
	  9,255,22,10,
	  255,255,255,0,
	  4,6,255,255,255,255,0,255,255,255,9 },

	// location 10
	{ "DREAMWEB.R10",
	  10,255,33,30,
	  255,255,255,0,
	  2,0,255,255,2,2,4,22,30,255,10 }, // 22,30,0 switches
	                                    // off path 0 in skip
	// location 11
	{ "DREAMWEB.R11",
	  11,255,11,20,
	  255,255,255,0,
	  0,4,255,255,255,255,255,255,255,255,11 },

	// location 12
	{ "DREAMWEB.R12",
	  12,255,22,20,
	  255,255,255,0,
	  1,4,255,255,255,255,255,255,255,255,12 },

	// location 13: boathouse (?)
	{ "DREAMWEB.R13",
	  12,255,22,20,
	  255,255,255,0,
	  1,4,255,255,255,255,255,255,255,255,13 },

	// location 14
	{ "DREAMWEB.R14",
	  14,255,44,20,
	  255,255,255,0,
	  0,6,255,255,255,255,255,255,255,255,14 },

	{ "", 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ "", 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ "", 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ "", 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },

	// location 19
	{ "DREAMWEB.R19",
	  19,255,0,0,
	  255,255,255,0,
	  0,4,255,255,255,255,255,255,255,255,19 },

	// location 20: Sart room (?)
	{ "DREAMWEB.R20",
	  22,255,0,20,
	  255,255,255,0,
	  1,4,2,15,255,255,255,255,255,255,20 },

	// location 21: lift noise audible (?)
	{ "DREAMWEB.R21",
	  5,255,11,10, // if demo: 22,255,11,10,
	  255,255,255,0,
	  1,4,2,15,1,255,255,255,255,255,21 },

	// location 22: pool room (?)
	{ "DREAMWEB.R22",
	  22,255,22,10,
	  255,255,255,0,
	  0,4,255,255,1,255,255,255,255,255,22 },

	// location 23
	{ "DREAMWEB.R23",
	  23,255,22,30,
	  255,255,255,0,
	  1,4,2,15,3,255,255,255,255,255,23 },

	// location 24: only room in which initialInv() is active, i.e. we get initial inventory here
	{ "DREAMWEB.R24",
	  5,255,44,0,
	  255,255,255,0,
	  1,6,2,15,255,255,255,255,255,255,24 },

	// location 25: helicopter (?)
	{ "DREAMWEB.R25",
	  22,255,11,40,
	  255,255,255,0,
	  1,0,255,255,255,255,255,255,255,255,25 },

	// location 26: reached via trap door (?)
	{ "DREAMWEB.R26",
	  9,255,22,20,
	  255,255,255,0,
	  4,2,255,255,255,255,255,255,255,255,26 },

	// location 27: rock room (?)
	{ "DREAMWEB.R27",
	  22,255,11,20,
	  255,255,255,0,
	  0,6,255,255,255,255,255,255,255,255,27 },

	// location 28: related to TV studiou (?), see resetLocation()
	{ "DREAMWEB.R28",
	  5,255,11,30,
	  255,255,255,0,
	  0,0,255,255,2,255,255,255,255,255,28 },

	// location 29: aide (?)
	{ "DREAMWEB.R29",
	  22,255,11,10,
	  255,255,255,0,
	  0,2,255,255,255,255,255,255,255,255,29 },


	// location 30
	{ "DREAMWEB.R05", // Duplicate of hotel lobby, but emerging from the lift.
	  5,255,22,10,    // if demo: 22,255,22,10
	  255,255,255,0,
	  1,4,1,15,255,255,255,255,255,255,5 },

	// location 31
	{ "DREAMWEB.R04",  // Duplicate of pool hall lobby,
	  23,255,22,20,    // but emerging from the lift.
	  255,255,255,0,
	  1,4,2,15,255,255,255,255,255,255,4 },

	// location 32
	{ "DREAMWEB.R10",  // entering alley via skip
	  10,255,22,30,
	  255,255,255,0,
	  3,6,255,255,255,255,255,255,255,255,10 },

	// location 33
	{ "DREAMWEB.R12",  // on the beach, getting up.
	  12,255,22,20,
	  255,255,255,0,
	  0,2,255,255,255,255,255,255,255,255,12 },

	// location 34
	{ "DREAMWEB.R03",  // Duplicate of Eden's lobby
	  5,255,44,0,      // but emerging from the lift
	  255,255,255,0,
	  1,6,2,255,4,255,255,255,255,255,3 },

	// location 35: Location when starting the game, after dream (?)
	{ "DREAMWEB.R24",  // Duplicate of Eden's flat
	  5,255,22,0,      // but starting on the bed
	  255,255,255,0,
	  3,6,0,255,255,255,255,33,0,3,24 }, //  33,0,3 turns off path for lift

	// location 36
	{ "DREAMWEB.R22",  // Duplicate
	  22,255,22,20,    // of hotel but in pool room
	  255,255,255,0,
	  1,4,255,255,255,255,255,255,255,255,22 },

	// location 37
	{ "DREAMWEB.R22",  // Duplicate
	  22,255,22,20,    // of hotel but in pool room
	  255,255,255,0,   // coming out of bedroom
	  0,2,255,255,255,255,255,255,255,255,22 },

	// location 38
	{ "DREAMWEB.R11",  // Duplicate
	  11,255,22,30,    // of carpark but getting
	  255,255,255,0,   // up off the floor
	  0,0,255,255,255,255,255,255,255,255,11 },

	// location 39
	{ "DREAMWEB.R28",
	  5,255,11,20,
	  255,255,255,0,
	  0,6,255,255,2,255,255,255,255,255,28 },

	// location 40
	{ "DREAMWEB.R21",
	  5,255,11,10, // if demo: 22,255,11,10
	  255,255,255,0,
	  1,4,2,15,1,255,255,255,255,255,21 },

	// location 41
	{ "DREAMWEB.R26",
	  9,255,0,40,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,26 },

	// location 42
	{ "DREAMWEB.R19",
	  19,255,0,0,
	  255,255,255,0,
	  2,2,255,255,255,255,255,255,255,255,19 },

	// location 43
	{ "DREAMWEB.R08",  // leaving tvstudio into street
	  8,255,11,40,
	  255,255,255,0,
	  0,4,255,255,255,255,255,255,255,255,8 },

	// location 44
	{ "DREAMWEB.R01",
	  1,255,44,10,
	  255,255,255,0,
	  3,6,255,255,255,255,255,255,255,255,1 },


	// location 45
	{ "DREAMWEB.R45",  // Dream room
	  35,255,22,30,
	  255,255,255,0,
	  0,6,255,255,255,255,255,255,255,255,45 },

	// location 46
	{ "DREAMWEB.R46",  // Dream room
	  35,255,22,40,
	  255,255,255,0,
	  0,4,255,255,255,255,255,255,255,255,46 },

	// location 47
	{ "DREAMWEB.R47",  // Dream room
	  35,255,0,0,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,47 },

	// location 48
	{ "DREAMWEB.R45",  // Dream room
	  35,255,22,30,
	  255,255,255,0,
	  4,0,255,255,255,255,255,255,255,255,45 },

	// location 49
	{ "DREAMWEB.R46",  // Dream room
	  35,255,22,50,
	  255,255,255,0,
	  0,4,255,255,255,255,255,255,255,255,46 },


	// location 50
	{ "DREAMWEB.R50",  //  Intro sequence one
	  35,255,22,30,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,50 },

	{ "DREAMWEB.R51",  //  Intro sequence two
	  35,255,11,30,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,51 },

	{ "DREAMWEB.R52",  //  Intro sequence three
	  35,255,22,30,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,52 },

	{ "DREAMWEB.R53",  //  Intro sequence four
	  35,255,33,0,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,53 },

	{ "DREAMWEB.R54",  //  Intro sequence five - wasteland
	  35,255,0,0,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,54 },

	{ "DREAMWEB.R55",  //  End sequence
	  14,255,44,0,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,55 }
};

static const Atmosphere g_atmosphereList[] = {
	// location,map x,y,sound,repeat
	{ 0,33,10,15,255 },
	{ 0,22,10,15,255 },
	{ 0,22,0,15,255 },
	{ 0,11,0,15,255 },
	{ 0,11,10,15,255 },
	{ 0,0,10,15,255 },

	{ 1,44,10,6,255	},
	{ 1,44,0,13,255 },

	{ 2,33,0,6,255 },
	{ 2,22,0,5,255 },
	{ 2,22,10,16,255 },
	{ 2,11,10,16,255 },

	{ 3,44,0,15,255 },
	{ 3,33,10,6,255 },
	{ 3,33,0,5,255 },

	{ 4,11,30,6,255 },
	{ 4,22,30,5,255 },
	{ 4,22,20,13,255 },

	{ 10,33,30,6,255 },
	{ 10,22,30,6,255 },

	{ 9,22,10,6,255 },
	{ 9,22,20,16,255 },
	{ 9,22,30,16,255 },
	{ 9,22,40,16,255 },
	{ 9,22,50,16,255 },

	{ 6,11,30,6,255 },
	{ 6,0,10,15,255 },
	{ 6,0,20,15,255 },
	{ 6,11,20,15,255 },
	{ 6,22,20,15,255 },

	{ 7,11,20,6,255 },
	{ 7,0,20,6,255 },
	{ 7,0,30,6,255 },

	{ 55,44,0,5,255 },
	{ 55,44,10,5,255 },

	{ 5,22,30,6,255 },
	{ 5,22,20,15,255 }, // if demo: { 5,22,20,16,255 },
	{ 5,22,10,15,255 }, // if demo: { 5,22,10,16,255 },

	{ 24,22,0,15,255 },
	{ 24,33,0,15,255 },
	{ 24,44,0,15,255 },
	{ 24,33,10,15,255 },

	{ 8,0,10,6,255 },
	{ 8,11,10,6,255 },
	{ 8,22,10,6,255 },
	{ 8,33,10,6,255 },
	{ 8,33,20,6,255 },
	{ 8,33,30,6,255 },
	{ 8,33,40,6,255 },
	{ 8,22,40,6,255 },
	{ 8,11,40,6,255 },

	{ 11,11,20,12,255 },
	{ 11,11,30,12,255 },
	{ 11,22,20,12,255 },
	{ 11,22,30,12,255 },

	{ 12,22,20,12,255 },
	{ 13,22,20,12,255 },
	{ 13,33,20,12,255 },

	{ 14,44,20,12,255 },
	{ 14,33,0,12,255 },
	{ 14,33,10,12,255 },
	{ 14,33,20,12,255 },
	{ 14,33,30,12,255 },
	{ 14,33,40,12,255 },
	{ 14,22,0,16,255 },

	{ 19,0,0,12,255 },

	{ 20,0,20,16,255 },
	{ 20,0,30,16,255 },
	{ 20,11,30,16,255 },
	{ 20,0,40,16,255 },
	{ 20,11,40,16,255 },

	{ 21,11,10,15,255 }, // if demo: { 21,11,10,16,255 },
	{ 21,11,20,15,255 }, // if demo: { 21,11,20,16,255 },
	{ 21, 0,20,15,255 }, // if demo: { 21,0,20,16,255 },
	{ 21,22,20,15,255 }, // if demo: { 21,22,20,16,255 },
	{ 21,33,20,15,255 }, // if demo: { 21,33,20,16,255 },
	{ 21,44,20,15,255 }, // if demo: { 21,44,20,16,255 },
	{ 21,44,10,15,255 }, // if demo: { 21,44,10,16,255 },

	{ 22,22,10,16,255 },
	{ 22,22,20,16,255 },

	{ 23,22,30,13,255 },
	{ 23,22,40,13,255 },
	{ 23,33,40,13,255 },
	{ 23,11,40,13,255 },
	{ 23,0,40,13,255 },
	{ 23,0,50,13,255 },

	{ 25,11,40,16,255 },
	{ 25,11,50,16,255 },
	{ 25,0,50,16,255 },

	{ 27,11,20,16,255 },
	{ 27,11,30,16,255 },

	{ 29,11,10,16,255 },

	{ 45,22,30,12,255 },
	{ 45,22,40,12,255 },
	{ 45,22,50,12,255 },

	{ 46,22,40,12,255 },
	{ 46,11,50,12,255 },
	{ 46,22,50,12,255 },
	{ 46,33,50,12,255 },

	{ 47,0,0,12,255 },

	{ 26,22,20,16,255 },
	{ 26,33,10,16,255 },
	{ 26,33,20,16,255 },
	{ 26,33,30,16,255 },
	{ 26,44,30,16,255 },
	{ 26,22,30,16,255 },
	{ 26,11,30,16,255 },
	{ 26,11,20,16,255 },
	{ 26,0,20,16,255 },
	{ 26,11,40,16,255 },
	{ 26,0,40,16,255 },
	{ 26,22,40,16,255 },
	{ 26,11,50,16,255 },

	{ 28,0,30,15,255 },
	{ 28,0,20,15,255 },
	{ 28,0,40,15,255 },
	{ 28,11,30,15,255 },
	{ 28,11,20,15,255 },
	{ 28,22,30,15,255 },
	{ 28,22,20,15,255 },

	{ 255,255,255,255,255 }

};

void DreamWebEngine::dreamwebFinalize() {
	// The engine will need some cleaner finalization (destructor?), let's put
	// it here for now

	getRidOfAll();

	_icons1.clear();
	_icons2.clear();
	_charset1.clear();
	_mainSprites.clear();

	// clear local graphics, just in case
	_keypadGraphics.clear();
	_menuGraphics.clear();
	_menuGraphics2.clear();
	_folderGraphics.clear();
	_folderGraphics2.clear();
	_folderGraphics3.clear();
	_folderCharset.clear();
	_symbolGraphics.clear();
	_diaryGraphics.clear();
	_diaryCharset.clear();
	_monitorGraphics.clear();
	_monitorCharset.clear();
	_newplaceGraphics.clear();
	_newplaceGraphics2.clear();
	_newplaceGraphics3.clear();
	_cityGraphics.clear();
	_saveGraphics.clear();

	_exFrames.clear();
	_exText.clear();

	_setFrames.clear();
	_freeFrames.clear();
	_reel1.clear();
	_reel2.clear();
	_reel3.clear();
	_setDesc.clear();
	_blockDesc.clear();
	_roomDesc.clear();
	_freeDesc.clear();
	_personText.clear();

	_textFile1.clear();
	_textFile2.clear();
	_textFile3.clear();
	_travelText.clear();
	_puzzleText.clear();
	_commandText.clear();
}

void DreamWebEngine::dreamweb() {
	switch(getLanguage()) {
	case Common::EN_ANY:
	case Common::EN_GRB:
	case Common::EN_USA:
		_foreignRelease = false;
		break;
	default:
		_foreignRelease = true;
		break;
	}

	allocateBuffers();

	// setMouse
	_oldPointerX = 0xffff;

	fadeDOS();
	getTime();
	clearBuffers();
	clearPalette();
	set16ColPalette();
	readSetData();
	_wonGame = false;

	_sound->loadSounds(0, "V99"); // basic sample

	bool firstLoop = true;

	int savegameId = Common::ConfigManager::instance().getInt("save_slot");

	while (true) {
		uint count = scanForNames();

		bool startNewGame = true;

		if (firstLoop && savegameId >= 0) {
			// loading a savegame requested from launcher/command line

			cls();
			setMode();
			loadPalFromIFF();
			clearPalette();

			doLoad(savegameId);
			workToScreen();
			fadeScreenUp();
			startNewGame = false;

		} else if (count == 0 && firstLoop) {

			// no savegames found, and we're not restarting.

			setMode();
			loadPalFromIFF();

		} else {
			// "doDecisions"

			// Savegames found, so ask if we should load one.
			// (If we're restarting after game over, we also always show these
			// options.)

			cls();
			setMode();
			decide();
			if (_quitRequested)
				return;

			if (_getBack == 4)
				startNewGame = false; // savegame has been loaded
		}

		firstLoop = false;

		if (startNewGame) {
			// "playGame"

			// "titles"
			// TODO: In the demo version, titles() did nothing
			clearPalette();
			bibleQuote();
			if (!_quitRequested) // "titlesearly"
				intro();

			if (_quitRequested)
				return;

			// "credits"
			clearPalette();
			realCredits();

			if (_quitRequested)
				return;

			clearChanges();
			setMode();
			loadPalFromIFF();
			_vars._location = 255;
			_vars._roomAfterDream = 1;
			_newLocation = 35;
			_sound->volumeSet(7);
			loadRoom();
			clearSprites();
			initMan();
			entryTexts();
			entryAnims();
			_destPos = 3;
			initialInv();
			_lastFlag = 32;
			startup1();
			_sound->volumeChange(0, -1);
			_commandType = 255;
		}

		// main loop
		while (true) {
			if (_quitRequested)
				return;

			screenUpdate();

			if (_quitRequested)
				return;

			if (_wonGame) {
				// "endofgame"
				clearBeforeLoad();
				fadeScreenDowns();
				hangOn(200);
				endGame();
				return;
			}

			if (_vars._manDead == 1 || _vars._manDead == 2)
				break;

			if (_vars._watchingTime > 0) {
				if (_finalDest == _mansPath)
					_vars._watchingTime--;
			}

			if (_vars._watchingTime == 0) {
				// "notWatching"

				if (_vars._manDead == 4)
					break;

				if (_newLocation != 255) {
					// "loadNew"
					clearBeforeLoad();
					loadRoom();
					clearSprites();
					initMan();
					entryTexts();
					entryAnims();
					_newLocation = 255;
					startup();
					_commandType = 255;
					workToScreenM();
				}
			}
		}

		// "gameOver"
		clearBeforeLoad();
		showGun();
		fadeScreenDown();
		hangOn(100);

	}
}

void DreamWebEngine::loadTextFile(TextFile &file, const char *suffix) {
	Common::String fileName = getDatafilePrefix() + suffix;
	FileHeader header;

	Common::File f;
	f.open(fileName);
	f.read((uint8 *)&header, sizeof(FileHeader));
	uint16 sizeInBytes = header.len(0);
	assert(sizeInBytes >= 2*66);

	delete[] file._text;
	file._text = new char[sizeInBytes - 2*66];

	f.read(file._offsetsLE, 2*66);
	f.read(file._text, sizeInBytes - 2*66);
}

void DreamWebEngine::screenUpdate() {
	newPlace();
	mainScreen();
	if (_quitRequested)
		return;
	animPointer();

	showPointer();
	if ((_vars._watchingTime == 0) && (_newLocation != 0xff))
		return;
	waitForVSync();
	uint16 mouseState = 0;
	mouseState |= readMouseState();
	dumpPointer();

	dumpTextLine();
	delPointer();
	autoLook();
	spriteUpdate();
	watchCount();
	zoom();

	showPointer();
	if (_wonGame)
		return;
	waitForVSync();
	mouseState |= readMouseState();
	dumpPointer();

	dumpZoom();
	delPointer();
	delEverything();
	printSprites();
	reelsOnScreen();
	afterNewRoom();

	showPointer();
	waitForVSync();
	mouseState |= readMouseState();
	dumpPointer();

	dumpMap();
	dumpTimedText();
	delPointer();

	showPointer();
	waitForVSync();
	_oldButton = _mouseButton;
	mouseState |= readMouseState();
	_mouseButton = mouseState;
	dumpPointer();

	dumpWatch();
	delPointer();
}

void DreamWebEngine::startup() {
	_currentKey = 0;
	_mainMode = 0;
	createPanel();
	_vars._newObs = 1;
	drawFloor();
	showIcon();
	getUnderZoom();
	spriteUpdate();
	printSprites();
	underTextLine();
	reelsOnScreen();
	atmospheres();
}

void DreamWebEngine::startup1() {
	clearPalette();
	_vars._throughDoor = 0;

	startup();

	workToScreen();
	fadeScreenUp();
}

void DreamWebEngine::switchRyanOn() {
	_vars._ryanOn = 255;
}

void DreamWebEngine::switchRyanOff() {
	_vars._ryanOn = 1;
}

void DreamWebEngine::loadGraphicsFile(GraphicsFile &file, const char *suffix) {
	Common::String fileName = getDatafilePrefix() + suffix;
	FileHeader header;

	Common::File f;
	f.open(fileName);
	f.read((uint8 *)&header, sizeof(FileHeader));
	uint16 sizeInBytes = header.len(0);

	assert(sizeInBytes >= kFrameBlocksize);
	file.clear();
	file._data = new uint8[sizeInBytes - kFrameBlocksize];
	file._frames = new Frame[kGraphicsFileFrameSize];
	f.read((uint8 *)file._frames, kFrameBlocksize);
	f.read(file._data, sizeInBytes - kFrameBlocksize);
}

void DreamWebEngine::loadGraphicsSegment(GraphicsFile &file, Common::File &inFile, unsigned int len) {
	assert(len >= kFrameBlocksize);
	file.clear();
	file._data = new uint8[len - kFrameBlocksize];
	file._frames = new Frame[kGraphicsFileFrameSize];
	inFile.read((uint8 *)file._frames, kFrameBlocksize);
	inFile.read(file._data, len - kFrameBlocksize);
}

void DreamWebEngine::loadTextSegment(TextFile &file, Common::File &inFile, unsigned int len) {
	const uint headerSize = 2 * file._size;
	assert(len >= headerSize);
	delete[] file._text;
	file._text = new char[len - headerSize];
	inFile.read((uint8 *)file._offsetsLE, headerSize);
	inFile.read((uint8 *)file._text, len - headerSize);
}

void DreamWebEngine::hangOnCurs(uint16 frameCount) {
	for (uint16 i = 0; i < frameCount; ++i) {
		printCurs();
		waitForVSync();
		delCurs();
	}
}

void DreamWebEngine::dumpTextLine() {
	if (_newTextLine != 1)
		return;
	_newTextLine = 0;
	uint16 x = _textAddressX;
	uint16 y = _textAddressY;
	if (_foreignRelease)
		y -= 3;
	multiDump(x, y, 228, 13);
}

void DreamWebEngine::getUnderTimed() {
	if (_foreignRelease)
		multiGet(_underTimedText, _timedTemp._x, _timedTemp._y - 3, 240, kUnderTimedTextSizeY_f);
	else
		multiGet(_underTimedText, _timedTemp._x, _timedTemp._y, 240, kUnderTimedTextSizeY);
}

void DreamWebEngine::putUnderTimed() {
	if (_foreignRelease)
		multiPut(_underTimedText, _timedTemp._x, _timedTemp._y - 3, 240, kUnderTimedTextSizeY_f);
	else
		multiPut(_underTimedText, _timedTemp._x, _timedTemp._y, 240, kUnderTimedTextSizeY);
}

void DreamWebEngine::triggerMessage(uint16 index) {
	multiGet(_mapStore, 174, 153, 200, 63);
	const uint8 *string = (const uint8 *)_puzzleText.getString(index);
	uint16 y = 156;
	printDirect(&string, 174, &y, 141, true);
	hangOn(140);
	workToScreen();
	hangOn(340);
	multiPut(_mapStore, 174, 153, 200, 63);
	workToScreen();
	_vars._lastTrigger = 0;
}

void DreamWebEngine::processTrigger() {
	if (_vars._lastTrigger == '1') {
		setLocation(8);
		triggerMessage(45);
	} else if (_vars._lastTrigger == '2') {
		setLocation(9);
		triggerMessage(55);
	} else if (_vars._lastTrigger == '3') {
		setLocation(2);
		triggerMessage(59);
	}
}

void DreamWebEngine::useTimedText() {
	if (_previousTimedTemp._string) {
		// TODO: It might be nice to make subtitles wait for the speech
		// to finish (_sound->isChannel1Playing()) when we're in speech+subtitles mode,
		// instead of waiting the pre-specified amount of time.


		// Ugly... (Maybe make this an argument to putUnderTimed()?)
		TimedTemp t = _timedTemp;
		_timedTemp = _previousTimedTemp;

		// Force-reset the previous string to make room for the next one
		putUnderTimed();

		_timedTemp = t;
		return;
	}

	if (_timeCount == 0)
		return;
	--_timeCount;
	if (_timeCount == 0) {
		putUnderTimed();
		_needToDumpTimed = 1;
		return;
	}

	if (_timeCount == _timedTemp._countToTimed)
		getUnderTimed();
	else if (_timeCount > _timedTemp._countToTimed)
		return;

	const uint8 *string = (const uint8 *)_timedTemp._string;
	printDirect(string, _timedTemp._x, _timedTemp._y, 237, true);
	_needToDumpTimed = 1;
}

void DreamWebEngine::setupTimedTemp(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount) {

	if (hasSpeech() && voiceIndex != 0) {
		_speechLoaded = _sound->loadSpeech('T', voiceIndex, 'T', textIndex);
		if (_speechLoaded)
			_sound->playChannel1(62);

		if (_speechLoaded && !_subtitles)
			return;

		if (_timeCount != 0) {
			// store previous TimedTemp for deletion
			_previousTimedTemp = _timedTemp;
			_timeCount = 0;
		}
	}

	if (_timeCount != 0)
		return;

	_timedTemp._y = y;
	_timedTemp._x = x;
	_timedTemp._countToTimed = countToTimed;
	_timeCount = _timedTemp._timeCount = timeCount + countToTimed;
	_timedTemp._string = _textFile1.getString(textIndex);
	debug(1, "setupTimedTemp: (%d, %d) => '%s'", textIndex, voiceIndex, _timedTemp._string);
}

void DreamWebEngine::dumpTimedText() {
	const TimedTemp *tt;
	if (_previousTimedTemp._string) {
		assert(!_needToDumpTimed);

		tt = &_previousTimedTemp;
		_previousTimedTemp._string = 0;
		_previousTimedTemp._timeCount = 0;
	} else if (_needToDumpTimed != 1) {
		return;
	} else {
		tt = &_timedTemp;
		_needToDumpTimed = 0;
	}

	const uint16 kUndertimedysize = 30;
	uint8 y = tt->_y;
	if (_foreignRelease)
		y -= 3;

	multiDump(tt->_x, y, 240, kUndertimedysize);
}

void DreamWebEngine::getTime() {
	TimeDate t;
	g_system->getTimeAndDate(t);
	debug(1, "\tgettime: %02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
	_vars._secondCount = t.tm_sec;
	_vars._minuteCount = t.tm_min;
	_vars._hourCount = t.tm_hour;
}

void DreamWebEngine::DOSReturn() {
	commandOnlyCond(46, 250);

	if (_mouseButton & 1) {
		_mouseButton = 0;
		quit();
	}
}

void DreamWebEngine::eraseOldObs() {
	if (_vars._newObs == 0)
		return;

	// Note: the original didn't delete sprites here, but marked the
	// entries as unused, to be filled again by makeSprite. This can
	// change the order of entries, but since they are drawn based on the
	// priority field, this shouldn't matter.
	Common::List<Sprite>::iterator i;
	for (i = _spriteTable.begin(); i != _spriteTable.end(); ) {
		Sprite &sprite = *i;
		if (sprite._objData)
			i = _spriteTable.erase(i);
		else
			++i;
	}
}

void DreamWebEngine::lockMon() {
	// Pressing space pauses text output in the monitor. We use the "hard"
	// key because calling readkey() drains characters from the input
	// buffer, we we want the user to be able to type ahead while the text
	// is being printed.
	if (_lastHardKey == 57) {
		// Clear the keyboard buffer. Otherwise the space that caused
		// the pause will be read immediately unpause the game.
		do {
			readKey();
		} while (_currentKey != 0);

		lockLightOn();
		while (!shouldQuit()) {
			waitForVSync();
			readKey();
			if (_currentKey == ' ')
				break;
		}
		// Forget the last "hard" key, otherwise the space that caused
		// the unpausing will immediately re-pause the game.
		_lastHardKey = 0;
		lockLightOff();
	}
}

void DreamWebEngine::startLoading(const Room &room) {
	_vars._combatCount = 0;
	_roomsSample = room.roomsSample;
	_mapX = room.mapX;
	_mapY = room.mapY;
	_vars._liftFlag = room.liftFlag;
	_mansPath = room.b21;
	_destination = room.b21;
	_finalDest = room.b21;
	_facing = room.facing;
	_turnToFace = room.facing;
	_vars._countToOpen = room.countToOpen;
	_vars._liftPath = room.liftPath;
	_vars._doorPath = room.doorPath;
	_vars._lastWeapon = (uint8)-1;
	_realLocation = room.realLocation;

	loadRoomData(room, false);

	findRoomInLoc();
	deleteTaken();
	setAllChanges();
	autoAppear();
//	const Room &newRoom = g_roomData[_newLocation];
	_vars._lastWeapon = (uint8)-1;
	_vars._manDead = 0;
	_lookCounter = 160;
	_newLocation = 255;
	_linePointer = 254;
	if (room.b27 != 255) {
		_mansPath = room.b27;
		autoSetWalk();
	}
	findXYFromPath();
}

void DreamWebEngine::dealWithSpecial(uint8 firstParam, uint8 secondParam) {
	uint8 type = firstParam - 220;
	if (type == 0) {
		placeSetObject(secondParam);
		_haveDoneObs = 1;
	} else if (type == 1) {
		removeSetObject(secondParam);
		_haveDoneObs = 1;
	} else if (type == 2) {
		placeFreeObject(secondParam);
		_haveDoneObs = 1;
	} else if (type == 3) {
		removeFreeObject(secondParam);
		_haveDoneObs = 1;
	} else if (type == 4) {
		switchRyanOff();
	} else if (type == 5) {
		_turnToFace = secondParam;
		_facing = secondParam;
		switchRyanOn();
	} else if (type == 6) {
		_newLocation = secondParam;
	} else {
		moveMap(secondParam);
	}
}

void DreamWebEngine::plotReel(uint16 &reelPointer) {
	Reel *reel = getReelStart(reelPointer);
	while (reel->x >= 220 && reel->x != 255) {
		dealWithSpecial(reel->x, reel->y);
		++reelPointer;
		reel += 8;
	}

	for (size_t i = 0; i < 8; ++i) {
		if (reel->frame() != 0xffff)
			showReelFrame(reel);
		++reel;
	}
	soundOnReels(reelPointer);
}

void DreamWebEngine::delTextLine() {
	if (_foreignRelease)
		multiPut(_textUnder, _textAddressX, _textAddressY - 3, kUnderTextSizeX_f, kUnderTextSizeY_f);
	else
		multiPut(_textUnder, _textAddressX, _textAddressY, kUnderTextSizeX, kUnderTextSizeY);
}

void DreamWebEngine::commandOnlyCond(uint8 command, uint8 commandType) {
	if (_commandType != commandType) {
		_commandType = commandType;
		commandOnly(command);
	}
}

void DreamWebEngine::commandOnly(uint8 command) {
	delTextLine();
	const uint8 *string = (const uint8 *)_commandText.getString(command);
	printDirect(string, _textAddressX, _textAddressY, _textLen, (bool)(_textLen & 1));
	_newTextLine = 1;
}

bool DreamWebEngine::checkIfPerson(uint8 x, uint8 y) {
	Common::List<People>::iterator i;
	for (i = _peopleList.begin(); i != _peopleList.end(); ++i) {
		People &people = *i;
		Reel *reel = getReelStart(people._reelPointer);
		if (reel->frame() == 0xffff)
			++reel;
		const Frame *frame = getReelFrameAX(reel->frame());
		uint8 xmin = reel->x + frame->x;
		uint8 ymin = reel->y + frame->y;
		uint8 xmax = xmin + frame->width;
		uint8 ymax = ymin + frame->height;
		if (x < xmin)
			continue;
		if (y < ymin)
			continue;
		if (x >= xmax)
			continue;
		if (y >= ymax)
			continue;
		_personData = people._routinePointer;
		obName(people.b4, 5);
		return true;
	}
	return false;
}

bool DreamWebEngine::checkIfFree(uint8 x, uint8 y) {
	Common::List<ObjPos>::const_iterator i;
	for (i = _freeList.reverse_begin(); i != _freeList.end(); --i) {
		const ObjPos &pos = *i;
		assert(pos.index != 0xff);
		if (!pos.contains(x,y))
			continue;
		obName(pos.index, 2);
		return true;
	}
	return false;
}

bool DreamWebEngine::checkIfEx(uint8 x, uint8 y) {
	Common::List<ObjPos>::const_iterator i;
	for (i = _exList.reverse_begin(); i != _exList.end(); --i) {
		const ObjPos &pos = *i;
		assert(pos.index != 0xff);
		if (!pos.contains(x,y))
			continue;
		obName(pos.index, 4);
		return true;
	}
	return false;
}

const uint8 *DreamWebEngine::findObName(uint8 type, uint8 index) {
	switch (type) {
	case 5:
		return (const uint8 *)_personText.getString(64 * (index & 127));
	case kExObjectType:
		return (const uint8 *)_exText.getString(index);
	case kFreeObjectType:
		return (const uint8 *)_freeDesc.getString(index);
	case kSetObjectType1:
		return (const uint8 *)_setDesc.getString(index);
	default:
		return (const uint8 *)_blockDesc.getString(index);
	}
}

void DreamWebEngine::copyName(uint8 type, uint8 index, uint8 *dst) {
	const uint8 *src = findObName(type, index);
	size_t i;
	for (i = 0; i < 28; ++i) {
		char c = src[i];
		if (c == ':')
			break;
		if (c == 0)
			break;
		dst[i] = c;
	}
	dst[i] = 0;
}

void DreamWebEngine::commandWithOb(uint8 command, uint8 type, uint8 index) {
	uint8 commandLine[64] = "OBJECT NAME ONE                         ";
	delTextLine();
	uint8 textLen = _textLen;

	const uint8 *string = (const uint8 *)_commandText.getString(command);
	printDirect(string, _textAddressX, _textAddressY, textLen, (bool)(textLen & 1));

	copyName(type, index, commandLine);
	uint16 x = _lastXPos;
	if (command != 0)
		x += 5;
	printDirect(commandLine, x, _textAddressY, textLen, (bool)(textLen & 1));
	_newTextLine = 1;
}

void DreamWebEngine::examineObText() {
	commandWithOb(1, _commandType, _command);
}

void DreamWebEngine::blockNameText() {
	commandWithOb(0, _commandType, _command);
}

void DreamWebEngine::personNameText() {
	commandWithOb(2, _commandType, _command & 127);
}

void DreamWebEngine::walkToText() {
	commandWithOb(3, _commandType, _command);
}

void DreamWebEngine::findOrMake(uint8 index, uint8 value, uint8 type) {
	Change *change = _listOfChanges;
	for (; change->index != 0xff; ++change) {
		if (index == change->index && _realLocation == change->location && type == change->type) {
			change->value = value;
			return;
		}
	}

	change->index = index;
	change->location = _realLocation;
	change->value = value;
	change->type = type;
}

void DreamWebEngine::setAllChanges() {
	Change *change = _listOfChanges;
	for (; change->index != 0xff; ++change)
		if (change->location == _realLocation)
			doChange(change->index, change->value, change->type);
}

DynObject *DreamWebEngine::getFreeAd(uint8 index) {
	return &_freeDat[index];
}

DynObject *DreamWebEngine::getExAd(uint8 index) {
	return &_exData[index];
}

DynObject *DreamWebEngine::getEitherAd() {
	if (_objectType == kExObjectType)
		return getExAd(_itemFrame);
	else
		return getFreeAd(_itemFrame);
}

void *DreamWebEngine::getAnyAd(uint8 *slotSize, uint8 *slotCount) {
	if (_objectType == kExObjectType) {
		DynObject *exObject = getExAd(_command);
		*slotSize = exObject->slotSize;
		*slotCount = exObject->slotCount;
		return exObject;
	} else if (_objectType == kFreeObjectType) {
		DynObject *freeObject = getFreeAd(_command);
		*slotSize = freeObject->slotSize;
		*slotCount = freeObject->slotCount;
		return freeObject;
	} else {	// 1 or 3. 0 should never happen
		SetObject *setObject = getSetAd(_command);
		// Note: the original returned slotCount/priority (bytes 4 and 5)
		// instead of slotSize/slotCount (bytes 3 and 4).
		// Changed this for consistency with the Ex/Free cases, and also
		// with getOpenedSize()
		*slotSize = setObject->slotSize;
		*slotCount = setObject->slotCount;
		return setObject;
	}
}

void *DreamWebEngine::getAnyAdDir(uint8 index, uint8 flag) {
	if (flag == kExObjectType)
		return getExAd(index);
	else if (flag == kFreeObjectType)
		return getFreeAd(index);
	else
		return getSetAd(index);
}

SetObject *DreamWebEngine::getSetAd(uint8 index) {
	return &_setDat[index];
}

void DreamWebEngine::doChange(uint8 index, uint8 value, uint8 type) {
	if (type == 0) { //object
		getSetAd(index)->mapad[0] = value;
	} else if (type == 1) { //freeObject
		DynObject *freeObject = getFreeAd(index);
		if (freeObject->mapad[0] == 0xff)
			freeObject->mapad[0] = value;
	} else { //path
		_pathData[type - 100].nodes[index].on = value;
	}
}

void DreamWebEngine::deleteTaken() {
	for (size_t i = 0; i < kNumexobjects; ++i) {
		uint8 location = _exData[i].initialLocation;
		if (location == _realLocation) {
			uint8 index = _exData[i].index;
			_freeDat[index].mapad[0] = 0xfe;
		}
	}
}

uint8 DreamWebEngine::getExPos() {
	DynObject *objects = _exData;
	for (size_t i = 0; i < kNumexobjects; ++i) {
		if (objects[i].mapad[0] == 0xff)
			return i;
	}
	error("Out of Ex object positions");
}

void DreamWebEngine::placeSetObject(uint8 index) {
	findOrMake(index, 0, 0);
	getSetAd(index)->mapad[0] = 0;
}

void DreamWebEngine::removeSetObject(uint8 index) {
	findOrMake(index, 0xff, 0);
	getSetAd(index)->mapad[0] = 0xff;
}

bool DreamWebEngine::finishedWalking() {
	return (_linePointer == 254) && (_facing == _turnToFace);
}

void DreamWebEngine::getFlagUnderP(uint8 *flag, uint8 *flagEx) {
	uint8 type, flagX, flagY;
	checkOne(_mouseX - _mapAdX, _mouseY - _mapAdY, flag, flagEx, &type, &flagX, &flagY);
	_lastFlag = *flag;
}

void DreamWebEngine::walkAndExamine() {
	if (!finishedWalking())
		return;
	_commandType = _walkExamType;
	_command = _walkExamNum;
	_walkAndExam = 0;
	if (_commandType != 5)
		examineOb();
}

void DreamWebEngine::obName(uint8 command, uint8 commandType) {
	if (_reAssesChanges == 0) {
		if ((commandType == _commandType) && (command == _command)) {
			if (_walkAndExam == 1) {
				walkAndExamine();
				return;
			} else if (_mouseButton == 0)
				return;
			else if ((_commandType == 3) && (_lastFlag < 2))
				return;
			else if ((_mansPath != _pointersPath) || (_commandType == 3)) {
				setWalk();
				_reAssesChanges = 1;
				return;
			} else if (!finishedWalking())
				return;
			else if (_commandType == 5) {
				if (_vars._watchingTime == 0)
					talk();
				return;
			} else {
				if (_vars._watchingTime == 0)
					examineOb();
				return;
			}
		}
	} else
		_reAssesChanges = 0;

	_command = command;
	_commandType = commandType;
	if ((_linePointer != 254) || (_vars._watchingTime != 0) || (_facing != _turnToFace)) {
		blockNameText();
		return;
	} else if (_commandType != 3) {
		if (_mansPath != _pointersPath) {
			walkToText();
			return;
		} else if (_commandType == 3) {
			blockNameText();
			return;
		} else if (_commandType == 5) {
			personNameText();
			return;
		} else {
			examineObText();
			return;
		}
	}
	if (_mansPath == _pointersPath) {
		uint8 flag, flagEx, type, flagX, flagY;
		checkOne(_ryanX + 12, _ryanY + 12, &flag, &flagEx, &type, &flagX, &flagY);
		if (flag < 2) {
			blockNameText();
			return;
		}
	}

	uint8 flag, flagEx;
	getFlagUnderP(&flag, &flagEx);
	if (_lastFlag < 2) {
		blockNameText();
		return;
	} else if (_lastFlag >= 128) {
		blockNameText();
		return;
	} else {
		walkToText();
		return;
	}
}

void DreamWebEngine::showBlink() {
	if (_manIsOffScreen == 1)
		return;
	++_blinkCount;
	if (_vars._shadesOn != 0)
		return;
	if (_realLocation >= 50) // eyesshut
		return;
	if (_blinkCount != 3)
		return;
	_blinkCount = 0;
	uint8 blinkFrame = _blinkFrame;
	++blinkFrame; // Implicit %256
	_blinkFrame = blinkFrame;
	if (blinkFrame > 6)
		blinkFrame = 6;
	static const uint8 blinkTab[] = { 16,18,18,17,16,16,16 };
	uint8 width, height;
	showFrame(_icons1, 44, 32, blinkTab[blinkFrame], 0, &width, &height);
}

void DreamWebEngine::dumpBlink() {
	if (_vars._shadesOn != 0)
		return;
	if (_blinkCount != 0)
		return;
	if (_blinkFrame >= 6)
		return;
	multiDump(44, 32, 16, 12);
}

void DreamWebEngine::printMessage(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered) {
	const uint8 *string = (const uint8 *)_commandText.getString(index);
	printDirect(string, x, y, maxWidth, centered);
}

void DreamWebEngine::printMessage2(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered, uint8 count) {
	const uint8 *string = (const uint8 *)_commandText.getString(index);
	while (count--) {
		findNextColon(&string);
	}
	printDirect(string, x, y, maxWidth, centered);
}

bool DreamWebEngine::objectMatches(void *object, const char *id) {
	const char *objId = (const char *)object + 12; // whether it is a DynObject or a SetObject
	for (size_t i = 0; i < 4; ++i) {
		if (id[i] != objId[i] + 'A')
			return false;
	}
	return true;
}

bool DreamWebEngine::compare(uint8 index, uint8 flag, const char id[4]) {
	return objectMatches(getAnyAdDir(index, flag), id);
}

uint16 DreamWebEngine::findSetObject(const char *id) {
	for (uint16 index = 0; index < 128; index++) {
		if (objectMatches(getSetAd(index), id))
			return index;
	}

	return 128;
}

uint16 DreamWebEngine::findExObject(const char *id) {
	for (uint16 index = 0; index < kNumexobjects; index++) {
		if (objectMatches(getExAd(index), id))
			return index;
	}

	return kNumexobjects;
}

bool DreamWebEngine::isRyanHolding(const char *id) {
	for (uint16 index = 0; index < kNumexobjects; index++) {
		DynObject *object = getExAd(index);
		if (object->mapad[0] == 4 && objectMatches(object, id))
			return true;
	}

	return false;
}

bool DreamWebEngine::isItDescribed(const ObjPos *pos) {
	const char *string = _setDesc.getString(pos->index);
	return string[0] != 0;
}

void DreamWebEngine::showIcon() {
	if (_realLocation < 50) {
		showPanel();
		showMan();
		roomName();
		panelIcons1();
		zoomIcon();
	} else {
		error("Unimplemented tempsprites code called");
		// the tempsprites segment is never initialized, but used here.
/*
		Frame *tempSprites = (Frame *)getSegment(data.word(kTempsprites)).ptr(0, 0);
		showFrame(tempSprites, 72, 2, 45, 0);
		showFrame(tempSprites, 72+47, 2, 46, 0);
		showFrame(tempSprites, 69-10, 21, 49, 0);
		showFrame(tempSprites, 160+88, 2, 45, 4 & 0xfe);
		showFrame(tempSprites, 160+43, 2, 46, 4 & 0xfe);
		showFrame(tempSprites, 160+101, 21, 49, 4 & 0xfe);

		// middle panel
		showFrame(tempSprites, 72 + 47 + 20, 0, 48, 0);
		showFrame(tempSprites, 72 + 19, 21, 47, 0);
		showFrame(tempSprites, 160 + 23, 0, 48, 4);
		showFrame(tempSprites, 160 + 71, 21, 47, 4);
*/
	}
}

bool DreamWebEngine::checkIfSet(uint8 x, uint8 y) {
	Common::List<ObjPos>::const_iterator i;
	for (i = _setList.reverse_begin(); i != _setList.end(); --i) {
		const ObjPos &pos = *i;
		assert(pos.index != 0xff);
		if (!pos.contains(x,y))
			continue;
		if (!pixelCheckSet(&pos, x, y))
			continue;
		if (!isItDescribed(&pos))
			continue;
		obName(pos.index, 1);
		return true;
	}
	return false;
}

void DreamWebEngine::hangOn(uint16 frameCount) {
	while (frameCount) {
		waitForVSync();
		--frameCount;
		if (_quitRequested)
			break;
	}
}

void DreamWebEngine::hangOnW(uint16 frameCount) {
	while (frameCount) {
		delPointer();
		readMouse();
		animPointer();
		showPointer();
		waitForVSync();
		dumpPointer();
		--frameCount;
		if (_quitRequested)
			break;
	}
}

void DreamWebEngine::hangOnP(uint16 count) {
	_mainTimer = 0;
	uint8 pointerFrame = _pointerFrame;
	uint8 pickup = _pickUp;
	_pointerMode = 3;
	_pickUp = 0;
	_commandType = 255;
	readMouse();
	animPointer();
	showPointer();
	waitForVSync();
	dumpPointer();

	count *= 3;
	for (uint16 i = 0; i < count; ++i) {
		delPointer();
		readMouse();
		animPointer();
		showPointer();
		waitForVSync();
		dumpPointer();
		if (_quitRequested)
			break;
		if (_mouseButton != 0 && _mouseButton != _oldButton)
			break;
	}

	delPointer();
	_pointerFrame = pointerFrame;
	_pickUp = pickup;
	_pointerMode = 0;
}

uint8 DreamWebEngine::findNextColon(const uint8 **string) {
	uint8 c;
	do {
		c = **string;
		++(*string);
	} while ((c != 0) && (c != ':'));
	return c;
}

void DreamWebEngine::zoomOnOff() {
	if (_vars._watchingTime != 0 || _pointerMode == 2) {
		blank();
		return;
	}

	commandOnlyCond(39, 222);

	if (!(_mouseButton & 1) || (_mouseButton == _oldButton))
		return;

	_vars._zoomOn ^= 1;
	createPanel();
	_vars._newObs = 0;
	drawFloor();
	printSprites();
	reelsOnScreen();
	showIcon();
	getUnderZoom();
	underTextLine();
	commandOnly(39);
	readMouse();
	workToScreenM();
}

void DreamWebEngine::sortOutMap() {
	const uint8 *src = workspace();
	uint8 *dst = _mapData;
	for (uint16 y = 0; y < kMapHeight; ++y) {
		memcpy(dst, src, kMapWidth);
		dst += kMapWidth;
		src += 132;
	}
}

void DreamWebEngine::mainScreen() {
	_inMapArea = 0;
	if (_vars._watchOn == 1) {
		RectWithCallback mainList[] = {
			{ 44,70,32,46,&DreamWebEngine::look },
			{ 0,50,0,180,&DreamWebEngine::inventory },
			{ 226,244,10,26,&DreamWebEngine::zoomOnOff },
			{ 226,244,26,40,&DreamWebEngine::saveLoad },
			{ 240,260,100,124,&DreamWebEngine::madmanRun },
			{ 0,320,0,200,&DreamWebEngine::identifyOb },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(mainList);
	} else {
		RectWithCallback mainList2[] = {
			{ 44,70,32,46,&DreamWebEngine::look },
			{ 0,50,0,180,&DreamWebEngine::inventory },
			{ 226+48,244+48,10,26,&DreamWebEngine::zoomOnOff },
			{ 226+48,244+48,26,40,&DreamWebEngine::saveLoad },
			{ 240,260,100,124,&DreamWebEngine::madmanRun },
			{ 0,320,0,200,&DreamWebEngine::identifyOb },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(mainList2);
	}

	if (_walkAndExam != 0)
		walkAndExamine();
}

void DreamWebEngine::showWatch() {
	if (_vars._watchOn) {
		showFrame(_icons1, 250, 1, 6, 0);
		showTime();
	}
}

void DreamWebEngine::dumpWatch() {
	if (_watchDump != 1)
		return;
	multiDump(256, 21, 40, 12);
	_watchDump = 0;
}

void DreamWebEngine::showTime() {
	if (_vars._watchOn == 0)
		return;

	int seconds = _vars._secondCount;
	int minutes = _vars._minuteCount;
	int hours = _vars._hourCount;

	showFrame(_charset1, 282+5, 21, 91*3+10 + seconds / 10, 0);
	showFrame(_charset1, 282+9, 21, 91*3+10 + seconds % 10, 0);

	showFrame(_charset1, 270+5, 21, 91*3 + minutes / 10, 0);
	showFrame(_charset1, 270+11, 21, 91*3 + minutes % 10, 0);

	showFrame(_charset1, 256+5, 21, 91*3 + hours / 10, 0);
	showFrame(_charset1, 256+11, 21, 91*3 + hours % 10, 0);

	showFrame(_charset1, 267+5, 21, 91*3+20, 0);
}

void DreamWebEngine::watchCount() {
	if (_vars._watchOn == 0)
		return;
	++_timerCount;
	if (_timerCount == 9) {
		showFrame(_charset1, 268+4, 21, 91*3+21, 0);
		_watchDump = 1;
	} else if (_timerCount == 18) {
		_timerCount = 0;
		++_vars._secondCount;
		if (_vars._secondCount == 60) {
			_vars._secondCount = 0;
			++_vars._minuteCount;
			if (_vars._minuteCount == 60) {
				_vars._minuteCount = 0;
				++_vars._hourCount;
				if (_vars._hourCount == 24)
					_vars._hourCount = 0;
			}
		}
		showTime();
		_watchDump = 1;
	}
}

void DreamWebEngine::roomName() {
	printMessage(88, 18, 53, 240, false);
	uint16 textIndex = _roomNum;
	if (textIndex >= 32)
		textIndex -= 32;
	_lineSpacing = 7;
	uint8 maxWidth = (_vars._watchOn == 1) ? 120 : 160;
	const uint8 *string = (const uint8 *)_roomDesc.getString(textIndex);
	printDirect(string, 88, 25, maxWidth, false);
	_lineSpacing = 10;
	useCharset1();
}

void DreamWebEngine::loadRoom() {
	_roomLoaded = 1;
	_timeCount = 0;
	_mainTimer = 0;
	_mapOffsetX = 104;
	_mapOffsetY = 38;
	_textAddressX = 13;
	_textAddressY = 182;
	_textLen = 240;
	_vars._location = _newLocation;
	const Room &room = g_roomData[_newLocation];
	startLoading(room);
	_sound->loadRoomsSample(_roomsSample);
	switchRyanOn();
	drawFlags();

	uint8 mapXstart, mapYstart;
	uint8 mapXsize, mapYsize;
	getDimension(&mapXstart, &mapYstart, &mapXsize, &mapYsize);
}

void DreamWebEngine::readSetData() {
	loadGraphicsFile(_charset1, "C00");
	loadGraphicsFile(_icons1, "G00");
	loadGraphicsFile(_icons2, "G01");
	loadGraphicsFile(_mainSprites, "S00");
	loadTextFile(_puzzleText, "T80");
	loadTextFile(_commandText, "T84");
	useCharset1();

	// FIXME: Why is this commented out?
	//openFile(getDatafilePrefix() + "VOL");
	//uint8 *volumeTab = getSegment(data.word(kSoundbuffer)).ptr(16384, 0);
	//readFromFile(volumeTab, 2048-256);
	//closeFile();
}

void DreamWebEngine::findRoomInLoc() {
	uint8 x = _mapX / 11;
	uint8 y = _mapY / 10;
	uint8 roomNum = y * 6 + x;
	_roomNum = roomNum;
}

void DreamWebEngine::autoLook() {
	if ((_mouseX != _oldX) || (_mouseY != _oldY)) {
		_lookCounter = 1000;
		return;
	}

	--_lookCounter;
	if (_lookCounter)
		return;
	if (_vars._watchingTime)
		return;
	doLook();
}

void DreamWebEngine::look() {
	if (_vars._watchingTime || (_pointerMode == 2)) {
		blank();
		return;
	}
	commandOnlyCond(25, 241);
	if ((_mouseButton == 1) && (_mouseButton != _oldButton))
		doLook();
}

void DreamWebEngine::doLook() {
	createPanel();
	showIcon();
	underTextLine();
	workToScreenM();
	_commandType = 255;
	dumpTextLine();
	uint8 index = _roomNum & 31;
	const uint8 *string = (const uint8 *)_roomDesc.getString(index);
	findNextColon(&string);
	uint16 x;
	if (_realLocation < 50)
		x = 66;
	else
		x = 40;
	if (printSlow(string, x, 80, 241, true) != 1)
		hangOnP(400);

	_pointerMode = 0;
	_commandType = 0;
	redrawMainScrn();
	workToScreenM();
}

void DreamWebEngine::useCharset1() {
	_currentCharset = &_charset1;
}

void DreamWebEngine::useTempCharset(GraphicsFile *charset) {
	_currentCharset = charset;
}

void DreamWebEngine::getRidOfTempText() {
	_textFile1.clear();
}

void DreamWebEngine::getRidOfAll() {
	delete[] _backdropBlocks;
	_backdropBlocks = 0;

	_setFrames.clear();
	_reel1.clear();
	_reel2.clear();
	_reel3.clear();
	delete[] _reelList;
	_reelList = 0;
	_personText.clear();
	_setDesc.clear();
	_blockDesc.clear();
	_roomDesc.clear();
	_freeFrames.clear();
	_freeDesc.clear();
}

// if skipDat, skip clearing and loading Setdat and Freedat
void DreamWebEngine::loadRoomData(const Room &room, bool skipDat) {
	processEvents();
	Common::File file;
	if (!file.open(modifyFileName(room.name)))
		error("cannot open file %s", modifyFileName(room.name).c_str());

	FileHeader header;
	file.read((uint8 *)&header, sizeof(FileHeader));

	// read segment lengths from room file header
	unsigned int len[15];
	for (int i = 0; i < 15; ++i)
		len[i] = header.len(i);

	assert(len[0] >= 192);
	_backdropBlocks = new uint8[len[0] - 192];
	file.read((uint8 *)_backdropFlags, 192);
	file.read(_backdropBlocks, len[0] - 192);

	assert(len[1] <= 132*66); // 132*66 = maplen
	memset(workspace(), 0, 132*66);
	file.read(workspace(), len[1]);

	sortOutMap();

	loadGraphicsSegment(_setFrames, file, len[2]);
	if (!skipDat) {
		const uint16 kSetdatlen = 64*128;
		assert(len[3] <= kSetdatlen);
		memset(_setDat, 255, kSetdatlen);
		file.read(_setDat, len[3]);
	} else {
		file.skip(len[3]);
	}
	// NB: The skipDat version of this function as called by restoreall
	// had a 'call bloc' instead of 'call loadseg' for reel1,
	// but 'bloc' was not defined.
	loadGraphicsSegment(_reel1, file, len[4]);
	loadGraphicsSegment(_reel2, file, len[5]);
	loadGraphicsSegment(_reel3, file, len[6]);

	// segment 7 consists of 36 RoomPaths followed by 'reelList'
	delete[] _reelList;
	if (len[7] <= 36*sizeof(RoomPaths)) {
		file.read((uint8 *)_pathData, len[7]);
		_reelList = 0;
	} else {
		file.read((uint8 *)_pathData, 36*sizeof(RoomPaths));
		unsigned int reelLen = len[7] - 36*sizeof(RoomPaths);
		unsigned int reelCount = (reelLen + sizeof(Reel) - 1) / sizeof(Reel);
		_reelList = new Reel[reelCount];
		file.read((uint8 *)_reelList, reelLen);
	}

	// segment 8 consists of 12 personFrames followed by a TextFile
	file.read((uint8 *)_personFramesLE, 24);
	loadTextSegment(_personText, file, len[8] - 24);

	loadTextSegment(_setDesc, file, len[9]);
	loadTextSegment(_blockDesc, file, len[10]);
	loadTextSegment(_roomDesc, file, len[11]);
	loadGraphicsSegment(_freeFrames, file, len[12]);
	if (!skipDat) {
		const uint16 kFreedatlen = 16*80;
		assert(len[13] <= kFreedatlen);
		memset(_freeDat, 255, kFreedatlen);
		file.read(_freeDat, len[13]);
	} else {
		file.skip(len[13]);
	}
	loadTextSegment(_freeDesc, file, len[14]);

	processEvents();
}

void DreamWebEngine::restoreAll() {
	const Room &room = g_roomData[_vars._location];
	loadRoomData(room, true);
	setAllChanges();
}

void DreamWebEngine::restoreReels() {
	if (_roomLoaded == 0)
		return;

	const Room &room = g_roomData[_realLocation];

	processEvents();
	Common::File file;
	if (!file.open(modifyFileName(room.name)))
		error("cannot open file %s", modifyFileName(room.name).c_str());

	FileHeader header;
	file.read((uint8 *)&header, sizeof(FileHeader));

	// read segment lengths from room file header
	int len[15];
	for (int i = 0; i < 15; ++i)
		len[i] = header.len(i);

	file.skip(len[0]);
	file.skip(len[1]);
	file.skip(len[2]);
	file.skip(len[3]);
	loadGraphicsSegment(_reel1, file, len[4]);
	loadGraphicsSegment(_reel2, file, len[5]);
	loadGraphicsSegment(_reel3, file, len[6]);

	processEvents();
}

void DreamWebEngine::showExit() {
	showFrame(_icons1, 274, 154, 11, 0);
}

void DreamWebEngine::showMan() {
	showFrame(_icons1, 0, 0, 0, 0);
	showFrame(_icons1, 0, 114, 1, 0);
	if (_vars._shadesOn)
		showFrame(_icons1, 28, 25, 2, 0);
}

void DreamWebEngine::panelIcons1() {
	uint16 x;
	if (_vars._watchOn != 1)
		x = 48;
	else
		x = 0;
	showFrame(_icons2, 204 + x, 4, 2, 0);
	if (_vars._zoomOn != 1)
		showFrame(_icons1, 228 + x, 8, 5, 0);
	showWatch();
}

void DreamWebEngine::examIcon() {
	showFrame(_icons2, 254, 5, 3, 0);
}

const uint8 *DreamWebEngine::getTextInFile1(uint16 index) {
	return (const uint8 *)_textFile1.getString(index);
}

void DreamWebEngine::loadTravelText() {
	loadTextFile(_travelText, "T81"); // location descs
}

void DreamWebEngine::loadTempText(const char *suffix) {
	loadTextFile(_textFile1, suffix);
}

void DreamWebEngine::drawFloor() {
	eraseOldObs();
	drawFlags();
	calcMapAd();
	doBlocks();
	showAllObs();
	showAllFree();
	showAllEx();
	panelToMap();
	initRain();
	_vars._newObs = 0;
}

void DreamWebEngine::allocateBuffers() {
	_exFrames.clear();
	_exFrames._data = new uint8[kExframeslen];
	_exFrames._frames = new Frame[kGraphicsFileFrameSize];
	_exText.clear();
	_exText._text = new char[kExtextlen];
}

void DreamWebEngine::workToScreenM() {
	animPointer();
	readMouse();
	showPointer();
	waitForVSync();
	workToScreen();
	delPointer();
}

void DreamWebEngine::atmospheres() {

	const Atmosphere *a = &g_atmosphereList[0];

	for (; a->_location != 255; ++a) {
		if (a->_location != _realLocation)
			continue;
		if (a->_mapX != _mapX || a->_mapY != _mapY)
			continue;
		if (a->_sound != _sound->getChannel0Playing()) {

			if (_vars._location == 45 && _vars._reelToWatch == 45)
				continue; // "web"

			_sound->playChannel0(a->_sound, a->_repeat);

			// NB: The asm here reads
			//	cmp reallocation,2
			//  cmp mapy,0
			//  jz fullvol
			//  jnz notlouisvol
			//  I'm interpreting this as if the cmp reallocation is below the jz

			if (_mapY == 0) {
				_sound->volumeSet(0); // "fullvol"
				return;
			}

			if (_realLocation == 2 && _mapX == 22 && _mapY == 10)
				_sound->volumeSet(5); // "louisvol"

			if (hasSpeech() && _realLocation == 14) {
				if (_mapX == 33) {
					_sound->volumeSet(0); // "ismad2"
					return;
				}

				if (_mapX == 22) {
					_sound->volumeSet(5);
					return;
				}

			}
		}

		if (_realLocation == 2) {
			if (_mapX == 22) {
				_sound->volumeSet(5); // "louisvol"
				return;
			}

			if (_mapX == 11) {
				_sound->volumeSet(0); // "fullvol"
				return;
			}
		}
		return;
	}

	_sound->cancelCh0();
}

void DreamWebEngine::readKey() {
	uint16 bufOut = _bufferOut;

	if (bufOut == _bufferIn) {
		// empty buffer
		_currentKey = 0;
		return;
	}

	bufOut = (bufOut + 1) & 15; // The buffer has size 16
	_currentKey = g_keyBuffer[bufOut];
	_bufferOut = bufOut;
}

void DreamWebEngine::newGame() {
	commandOnlyCond(47, 251);

	if (_mouseButton == 1)
		_getBack = 3;
}

void DreamWebEngine::pickupOb(uint8 command, uint8 pos) {
	_lastInvPos = pos;
	_objectType = kFreeObjectType;
	_itemFrame = command;
	_command = command;
	//uint8 dummy;
	//getAnyAd(&dummy, &dummy);	// was in the original source, seems useless here
	transferToEx(command);
}

void DreamWebEngine::initialInv() {
	if (_realLocation != 24)
		return;

	pickupOb(11, 5);
	pickupOb(12, 6);
	pickupOb(13, 7);
	pickupOb(14, 8);
	pickupOb(18, 0);
	pickupOb(19, 1);
	pickupOb(20, 9);
	pickupOb(16, 2);
	_vars._watchMode = 1;
	_vars._reelToHold = 0;
	_vars._endOfHoldReel = 6;
	_vars._watchSpeed = 1;
	_vars._speedCount = 1;
	switchRyanOff();
}

void DreamWebEngine::walkIntoRoom() {
	if (_vars._location == 14 && _mapX == 22) {
		_destination = 1;
		_finalDest = 1;
		autoSetWalk();
	}
}

void DreamWebEngine::afterIntroRoom() {
	if (_nowInNewRoom == 0)
		return; // notnewintro

	clearWork();
	findRoomInLoc();
	_vars._newObs = 1;
	drawFloor();
	reelsOnScreen();
	spriteUpdate();
	printSprites();
	workToScreen();
	_nowInNewRoom = 0;
}

void DreamWebEngine::redrawMainScrn() {
	_timeCount = 0;
	createPanel();
	_vars._newObs = 0;
	drawFloor();
	printSprites();
	reelsOnScreen();
	showIcon();
	getUnderZoom();
	underTextLine();
	readMouse();
	_commandType = 255;
}

void DreamWebEngine::blank() {
	commandOnlyCond(0, 199);
}

void DreamWebEngine::allPointer() {
	readMouse();
	showPointer();
	dumpPointer();
}

void DreamWebEngine::makeMainScreen() {
	createPanel();
	_vars._newObs = 1;
	drawFloor();
	spriteUpdate();
	printSprites();
	reelsOnScreen();
	showIcon();
	getUnderZoom();
	underTextLine();
	_commandType = 255;
	animPointer();
	workToScreenM();
	_commandType = 200;
	_manIsOffScreen = 0;
}

void DreamWebEngine::openInv() {
	_invOpen = 1;
	printMessage(80, 58 - 10, 61, 240, (240 & 1));
	fillRyan();
	_commandType = 255;
}

void DreamWebEngine::obsThatDoThings() {
	if (!compare(_command, _objectType, "MEMB"))
		return; // notlouiscard

	if (getLocation(4) != 1) {
		setLocation(4);
		lookAtCard();
	}
}

void DreamWebEngine::describeOb() {
	const uint8 *obText = getObTextStart();
	uint16 y = 92;
	if (_foreignRelease && _objectType == kSetObjectType1)
		y = 82;
	_charShift = 91 + 91;
	printDirect(&obText, 33, &y, 241, 241 & 1);
	_charShift = 0;
	y = 104;
	if (_foreignRelease && _objectType == kSetObjectType1)
		y = 94;
	printDirect(&obText, 36, &y, 241, 241 & 1);
	obsThatDoThings();

	// Additional text
	if (compare(_command, _objectType, "CUPE")) {
		// Empty cup
		const uint8 *string = (const uint8 *)_puzzleText.getString(40);
		printDirect(string, 36, y + 10, 241, 241 & 1);
	} else if (compare(_command, _objectType, "CUPF")) {
		// Full cup
		const uint8 *string = (const uint8 *)_puzzleText.getString(39);
		printDirect(string, 36, y + 10, 241, 241 & 1);
	}
}

void DreamWebEngine::delEverything() {
	if (_mapYSize + _mapOffsetY < 182) {
		mapToPanel();
	} else {
		// Big room
		_mapYSize -= 8;
		mapToPanel();
		_mapYSize += 8;
	}
}

void DreamWebEngine::errorMessage1() {
	delPointer();
	printMessage(76, 21, 58, 240, (240 & 1));
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamWebEngine::errorMessage2() {
	_commandType = 255;
	delPointer();
	printMessage(76, 21, 59, 240, (240 & 1));
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamWebEngine::errorMessage3() {
	delPointer();
	printMessage(76, 21, 60, 240, (240 & 1));
	workToScreenM();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamWebEngine::reExFromOpen() {

}

void DreamWebEngine::putBackObStuff() {
	createPanel();
	showPanel();
	showMan();
	obIcons();
	showExit();
	obPicture();
	describeOb();
	underTextLine();
	_commandType = 255;
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

bool DreamWebEngine::isSetObOnMap(uint8 index) {
	return (getSetAd(index)->mapad[0] == 0);
}

void DreamWebEngine::examineInventory() {
	commandOnlyCond(32, 249);

	if (!(_mouseButton & 1))
		return;

	createPanel();
	showPanel();
	showMan();
	showExit();
	examIcon();
	_pickUp = 0;
	_invOpen = 2;
	openInv();
	workToScreenM();
}

void DreamWebEngine::middlePanel() {
}

void DreamWebEngine::underTextLine() {
	if (_foreignRelease)
		multiGet(_textUnder, _textAddressX, _textAddressY - 3, kUnderTextSizeX_f, kUnderTextSizeY_f);
	else
		multiGet(_textUnder, _textAddressX, _textAddressY, kUnderTextSizeX, kUnderTextSizeY);
}

void DreamWebEngine::showWatchReel() {
	uint16 reelPointer = _vars._reelToWatch;
	plotReel(reelPointer);
	_vars._reelToWatch = reelPointer;

	// check for shake
	if (_realLocation == 26 && reelPointer == 104)
		_vars._shakeCounter = 0xFF;
}

void DreamWebEngine::watchReel() {
	if (_vars._reelToWatch != 0xFFFF) {
		if (_mansPath != _finalDest)
			return; // Wait until stopped walking
		if (_turnToFace != _facing)
			return;

		if (--_vars._speedCount != 0xFF) {
			showWatchReel();
			return;
		}
		_vars._speedCount = _vars._watchSpeed;
		if (_vars._reelToWatch != _vars._endWatchReel) {
			++_vars._reelToWatch;
			showWatchReel();
			return;
		}
		if (_vars._watchingTime) {
			showWatchReel();
			return;
		}
		_vars._reelToWatch = 0xFFFF;
		_vars._watchMode = 0xFF;
		if (_vars._reelToHold == 0xFFFF)
			return; // No more reel
		_vars._watchMode = 1;
	} else if (_vars._watchMode != 1) {
		if (_vars._watchMode != 2)
			return; // "notreleasehold"
		if (--_vars._speedCount == 0xFF) {
			_vars._speedCount = _vars._watchSpeed;
			++_vars._reelToHold;
		}
		if (_vars._reelToHold == _vars._endOfHoldReel) {
			_vars._reelToHold = 0xFFFF;
			_vars._watchMode = 0xFF;
			_destination = _vars._destAfterHold;
			_finalDest = _vars._destAfterHold;
			autoSetWalk();
			return;
		}
	}

	uint16 reelPointer = _vars._reelToHold;
	plotReel(reelPointer);
}

void DreamWebEngine::afterNewRoom() {
	if (_nowInNewRoom == 0)
		return; // notnew

	_timeCount = 0;
	createPanel();
	_commandType = 0;
	findRoomInLoc();
	if (_vars._ryanOn != 1) {
		_mansPath = findPathOfPoint(_ryanX + 12, _ryanY + 12);
		findXYFromPath();
		_resetManXY = 1;
	}
	_vars._newObs = 1;
	drawFloor();
	_lookCounter = 160;
	_nowInNewRoom = 0;
	showIcon();
	spriteUpdate();
	printSprites();
	underTextLine();
	reelsOnScreen();
	mainScreen();
	getUnderZoom();
	zoom();
	workToScreenM();
	walkIntoRoom();
	edensFlatReminders();
	atmospheres();
}

void DreamWebEngine::madmanRun() {
	if (_vars._location    != 14 ||
		_mapX        != 22 ||
		_pointerMode !=  2 ||
		_vars._madmanFlag  !=  0) {
		identifyOb();
		return;
	}

	commandOnlyCond(52, 211);

	if (_mouseButton == 1 &&
		_mouseButton != _oldButton)
		_vars._lastWeapon = 8;
}


void DreamWebEngine::decide() {
	setMode();
	loadPalFromIFF();
	clearPalette();
	_pointerMode = 0;
	_vars._watchingTime = 0;
	_pointerFrame = 0;
	_textAddressX = 70;
	_textAddressY = 182 - 8;
	_textLen = 181;
	_manIsOffScreen = 1;
	loadSaveBox();
	showDecisions();
	workToScreen();
	fadeScreenUp();
	_getBack = 0;

	RectWithCallback decideList[] = {
		{ kOpsx+69,kOpsx+124,kOpsy+30,kOpsy+76,&DreamWebEngine::newGame },
		{ kOpsx+20,kOpsx+87,kOpsy+10,kOpsy+59,&DreamWebEngine::DOSReturn },
		{ kOpsx+123,kOpsx+190,kOpsy+10,kOpsy+59,&DreamWebEngine::loadOld },
		{ 0,320,0,200,&DreamWebEngine::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	do {
		if (_quitRequested)
			return;

		readMouse();
		showPointer();
		waitForVSync();
		dumpPointer();
		dumpTextLine();
		delPointer();
		checkCoords(decideList);
	} while (!_getBack);

	if (_getBack != 4)
		_saveGraphics.clear();	// room not loaded

	_textAddressX = 13;
	_textAddressY = 182;
	_textLen = 240;
}

void DreamWebEngine::showGun() {
	_addToRed = 0;
	_addToGreen = 0;
	_addToBlue = 0;
	palToStartPal();
	palToEndPal();
	greyscaleSum();
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 128;
	hangOn(130);
	endPalToStart();
	clearEndPal();
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 128;
	hangOn(200);
	_roomsSample = 34;
	_sound->loadRoomsSample(_roomsSample);
	_sound->volumeSet(0);
	GraphicsFile graphics;
	loadGraphicsFile(graphics, "G13");
	createPanel2();
	showFrame(graphics, 100, 4, 0, 0);
	showFrame(graphics, 158, 106, 1, 0);
	workToScreen();
	graphics.clear();
	fadeScreenUp();
	hangOn(160);
	_sound->playChannel0(12, 0);
	loadTempText("T83");
	rollEndCreditsGameLost();
	getRidOfTempText();
}

void DreamWebEngine::dropError() {
	_commandType = 255;
	delPointer();
	printMessage(76, 21, 56, 240, 240 & 1);
	workToScreenM();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	_commandType = 255;
	workToScreenM();
}

void DreamWebEngine::cantDrop() {
	_commandType = 255;
	delPointer();
	printMessage(76, 21, 24, 240, 240 & 1);
	workToScreenM();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	_commandType = 255;
	workToScreenM();
}

void DreamWebEngine::getBack1() {
	if (_pickUp != 0) {
		blank();
		return;
	}


	commandOnlyCond(26, 202);

	if (_mouseButton == _oldButton)
		return;

	if (_mouseButton & 1) {
		// Get back
		_getBack = 1;
		_pickUp = 0;
	}
}

void DreamWebEngine::autoAppear() {
	if (_vars._location == 32) {
		// In alley
		resetLocation(5);
		setLocation(10);
		_destPos = 10;
		return;
	}

	if (_realLocation == 24) {
		// In Eden's apartment
		if (_vars._generalDead == 1) {
			_vars._generalDead++;
			placeSetObject(44);
			placeSetObject(18);
			placeSetObject(93);
			removeSetObject(92);
			removeSetObject(55);
			removeSetObject(75);
			removeSetObject(84);
			removeSetObject(85);
		} else if (_vars._sartainDead == 1) {
			// Eden's part 2
			removeSetObject(44);
			removeSetObject(93);
			placeSetObject(55);
			_vars._sartainDead++;
		}
	} else {
		// Not in Eden's
		if (_realLocation == 25) {
			// Sart roof
			_vars._newsItem = 3;
			resetLocation(6);
			setLocation(11);
			_destPos = 11;
		} else {
			if (_realLocation == 2 && _vars._rockstarDead != 0)
				placeSetObject(23);
		}
	}
}

void DreamWebEngine::setupTimedUse(uint16 textIndex, uint16 countToTimed, uint16 timeCount, byte x, byte y) {
	if (_timeCount != 0)
		return; // can't setup

	_timedTemp._y = y;
	_timedTemp._x = x;
	_timedTemp._countToTimed = countToTimed;
	_timeCount = _timedTemp._timeCount = timeCount + countToTimed;
	_timedTemp._string = _puzzleText.getString(textIndex);
	debug(1, "setupTimedUse: %d => '%s'", textIndex, _timedTemp._string);
}

void DreamWebEngine::entryTexts() {
	switch (_vars._location) {
	case 21:
		setupTimedUse(28, 60, 11, 68, 64);
		break;
	case 30:
		setupTimedUse(27, 60, 11, 68, 64);
		break;
	case 23:
		setupTimedUse(29, 60, 11, 68, 64);
		break;
	case 31:
		setupTimedUse(30, 60, 11, 68, 64);
		break;
	case 20:	// Sarter's 2
		setupTimedUse(31, 60, 11, 68, 64);
		break;
	case 24:	// Eden's lobby
		setupTimedUse(32, 60, 3, 68, 64);
		break;
	case 34:	// Eden 2
		setupTimedUse(33, 60, 3, 68, 64);
		break;
	default:
		break;
	}
}

void DreamWebEngine::entryAnims() {
	_vars._reelToWatch = 0xFFFF;
	_vars._watchMode = (byte)-1;

	switch (_vars._location) {
	case 33:	// beach
		switchRyanOff();
		_vars._watchingTime = 76 * 2;
		_vars._reelToWatch = 0;
		_vars._endWatchReel = 76;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		break;
	case 44:	// Sparky's
		resetLocation(8);
		_vars._watchingTime = 50*2;
		_vars._reelToWatch = 247;
		_vars._endWatchReel = 297;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		switchRyanOff();
		break;
	case 22:	// lift
		_vars._watchingTime = 31 * 2;
		_vars._reelToWatch = 0;
		_vars._endWatchReel = 30;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		switchRyanOff();
		break;
	case 26:	// under church
		_symbolTopNum = 2;
		_symbolBotNum = 1;
		break;
	case 45:	// entered Dreamweb
		_vars._keeperFlag = 0;
		_vars._watchingTime = 296;
		_vars._reelToWatch = 45;
		_vars._endWatchReel = 198;
		_vars._watchSpeed = 1;
		_vars._speedCount = 1;
		switchRyanOff();
		break;
	default:
		if (_realLocation == 46 && _vars._sartainDead == 1) {	// Crystal
			removeFreeObject(0);
		} else if (_vars._location == 9 && !checkIfPathIsOn(2) && _vars._aideDead != 0) {
			// Top of church
			if (checkIfPathIsOn(3))
				turnPathOn(2);

			// Make doors open
			removeSetObject(4);
			placeSetObject(5);
		} else if (_vars._location == 47) {	// Dream centre
			placeSetObject(4);
			placeSetObject(5);
		} else if (_vars._location == 38) {	// Car park
			_vars._watchingTime = 57 * 2;
			_vars._reelToWatch = 4;
			_vars._endWatchReel = 57;
			_vars._watchSpeed = 1;
			_vars._speedCount = 1;
			switchRyanOff();
		} else if (_vars._location == 32) {	// Alley
			_vars._watchingTime = 66 * 2;
			_vars._reelToWatch = 0;
			_vars._endWatchReel = 66;
			_vars._watchSpeed = 1;
			_vars._speedCount = 1;
			switchRyanOff();
		} else if (_vars._location == 24) {	// Eden's again
			turnAnyPathOn(2, _roomNum - 1);
		}
	}
}

void DreamWebEngine::lookAtCard() {
	_manIsOffScreen = 1;
	getRidOfReels();
	loadKeypad();
	createPanel2();
	showFrame(_keypadGraphics, 160, 80, 42, 128);
	const uint8 *obText = getObTextStart();
	findNextColon(&obText);
	findNextColon(&obText);
	findNextColon(&obText);
	uint16 y = 124;
	printDirect(&obText, 36, &y, 241, 241 & 1);
	workToScreenM();
	hangOnW(280);
	createPanel2();
	showFrame(_keypadGraphics, 160, 80, 42, 128);
	printDirect(obText, 36, 130, 241, 241 & 1);
	workToScreenM();
	hangOnW(200);
	_manIsOffScreen = 0;
	_keypadGraphics.clear();
	restoreReels();
	putBackObStuff();
}

void DreamWebEngine::clearBuffers() {
	clearChanges();
}

void DreamWebEngine::clearChanges() {
	memset(_listOfChanges, 0xFF, sizeof(_listOfChanges));

	setupInitialReelRoutines();

	setupInitialVars();

	_vars._exFramePos = 0;
	_vars._exTextPos = 0;

	memset(_exFrames._frames, 0xFF, kFrameBlocksize);
	memset(_exFrames._data, 0xFF, kExframeslen);
	memset(_exData, 0xFF, sizeof(_exData));
	memset(_exText._offsetsLE, 0xFF, 2*(kNumexobjects+2));
	memset(_exText._text, 0xFF, kExtextlen);

	const uint8 initialRoomsCanGo[16] = { 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	assert(sizeof(_roomsCanGo) == sizeof(initialRoomsCanGo));
	memcpy(_roomsCanGo, initialRoomsCanGo, sizeof(initialRoomsCanGo));
}

void DreamWebEngine::setupInitialVars() {
	_vars._startVars = 0;
	_vars._progressPoints = 0;
	_vars._watchOn = 0;
	_vars._shadesOn = 0;
	getTime();
	_vars._zoomOn = 1;
	_vars._location = 0;
	_vars._exPos = 0;
	_vars._exFramePos = 0;
	_vars._exTextPos = 0;
	_vars._card1Money = 0;
	_vars._listPos = 0;
	_vars._ryanPage = 0;
	_vars._watchingTime = 0;
	_vars._reelToWatch = (uint16)-1;
	_vars._endWatchReel = 0;
	_vars._speedCount = 0;
	_vars._watchSpeed = 0;
	_vars._reelToHold = (uint16)-1;
	_vars._endOfHoldReel = (uint16)-1;
	_vars._watchMode = (uint8)-1;
	_vars._destAfterHold = 0;
	_vars._newsItem = 0;
	_vars._liftFlag = 0;
	_vars._liftPath = 0;
	_vars._lockStatus = 1;
	_vars._doorPath = 0;
	_vars._countToOpen = 0;
	_vars._countToClose = 0;
	_vars._rockstarDead = 0;
	_vars._generalDead = 0;
	_vars._sartainDead = 0;
	_vars._aideDead = 0;
	_vars._beenMugged = 0;
	_vars._gunPassFlag = 0;
	_vars._canMoveAltar = 0;
	_vars._talkedToAttendant = 0;
	_vars._talkedToSparky = 0;
	_vars._talkedToBoss = 0;
	_vars._talkedToRecep = 0;
	_vars._cardPassFlag = 0;
	_vars._madmanFlag = 0;
	_vars._keeperFlag = 0;
	_vars._lastTrigger = 0;
	_vars._manDead = 0;
	_vars._seed1 = 1;
	_vars._seed2 = 2;
	_vars._seed3 = 3;
	_vars._needToTravel = 0;
	_vars._throughDoor = 0;
	_vars._newObs = 0;
	_vars._ryanOn = 255;
	_vars._combatCount = 0;
	_vars._lastWeapon = (uint8)-1;
	_vars._dreamNumber = 0;
	_vars._roomAfterDream = 0;
	_vars._shakeCounter = 48;
}

void DreamWebEngine::edensFlatReminders() {
	if (_realLocation != 24 || _mapX != 44)
		return; // not in Eden's lift

	if (_vars._progressPoints)
		return; // not the first time in Eden's apartment

	uint16 exObjextIndex = findExObject("CSHR");
	if (!isRyanHolding("DKEY") || exObjextIndex == kNumexobjects) {
		setupTimedUse(50, 48, 8, 54, 70);	// forgot something
		return;
	}

	DynObject *object = getExAd(exObjextIndex);

	if (object->mapad[0] != 4) {
		setupTimedUse(50, 48, 8, 54, 70);	// forgot something
		return;
	} else if (object->mapad[1] != 255) {
		if (!compare(object->mapad[1], object->mapad[0], "PURS")) {
			setupTimedUse(50, 48, 8, 54, 70);	// forgot something
			return;
		}
	}

	_vars._progressPoints++;	// got card
}

void DreamWebEngine::incRyanPage() {
	commandOnlyCond(31, 222);

	if (_mouseButton == _oldButton || !(_mouseButton & 1))
		return;

	_vars._ryanPage = (_mouseX - (kInventx + 167)) / 18;

	delPointer();
	fillRyan();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();

}

void DreamWebEngine::emergencyPurge() {
	while (true) {
		if (_vars._exFramePos + 4000 < kExframeslen) {
			// Not near frame end
			if (_vars._exTextPos + 400 < kExtextlen)
				return;	// notneartextend
		}

		purgeAnItem();
	}
}

void DreamWebEngine::purgeAnItem() {
	const DynObject *extraObjects = _exData;

	for (size_t i = 0; i < kNumexobjects; ++i) {
		if (extraObjects[i].mapad[0] && extraObjects[i].objId[0] == 255 &&
			extraObjects[i].initialLocation != _realLocation) {
			deleteExObject(i);
			return;
		}
	}

	for (size_t i = 0; i < kNumexobjects; ++i) {
		if (extraObjects[i].mapad[0] && extraObjects[i].objId[0] == 255) {
			deleteExObject(i);
			return;
		}
	}
}

} // End of namespace DreamWeb
