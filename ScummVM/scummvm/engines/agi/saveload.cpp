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

//
// Savegame support by Vasyl Tsvirkunov <vasyl@pacbell.net>
// Multi-slots by Claudio Matsuoka <claudio@helllabs.org>
//

#include "common/file.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "gui/saveload.h"

#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/sprite.h"
#include "agi/keyboard.h"
#include "agi/menu.h"

#define SAVEGAME_VERSION 6

//
// Version 0 (Sarien): view table has 64 entries
// Version 1 (Sarien): view table has 256 entries (needed in KQ3)
// Version 2 (ScummVM): first ScummVM version
// Version 3 (ScummVM): added AGIPAL save/load support
// Version 4 (ScummVM): added thumbnails and save creation date/time
// Version 5 (ScummVM): Added game md5
// Version 6 (ScummVM): Added game played time
//

namespace Agi {

static const uint32 AGIflag = MKTAG('A','G','I',':');

int AgiEngine::saveGame(const Common::String &fileName, const Common::String &description) {
	char gameIDstring[8] = "gameIDX";
	int i;
	Common::OutSaveFile *out;
	int result = errOK;

	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "AgiEngine::saveGame(%s, %s)", fileName.c_str(), description.c_str());
	if (!(out = _saveFileMan->openForSaving(fileName))) {
		warning("Can't create file '%s', game not saved", fileName.c_str());
		return errBadFileOpen;
	} else {
		debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Successfully opened %s for writing", fileName.c_str());
	}

	out->writeUint32BE(AGIflag);
	out->write(description.c_str(), 31);

	out->writeByte(SAVEGAME_VERSION);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing save game version (%d)", SAVEGAME_VERSION);

	// Thumbnail
	Graphics::saveThumbnail(*out);

	// Creation date/time
	TimeDate curTime;
	_system->getTimeAndDate(curTime);

	uint32 saveDate = ((curTime.tm_mday & 0xFF) << 24) | (((curTime.tm_mon + 1) & 0xFF) << 16) | ((curTime.tm_year + 1900) & 0xFFFF);
	uint16 saveTime = ((curTime.tm_hour & 0xFF) << 8) | ((curTime.tm_min) & 0xFF);
	uint32 playTime = g_engine->getTotalPlayTime() / 1000;

	out->writeUint32BE(saveDate);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing save date (%d)", saveDate);
	out->writeUint16BE(saveTime);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing save time (%d)", saveTime);
	out->writeUint32BE(playTime);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing play time (%d)", playTime);

	out->writeByte(_game.state);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing game state (%d)", _game.state);

	strcpy(gameIDstring, _game.id);
	out->write(gameIDstring, 8);
	debugC(5, kDebugLevelMain | kDebugLevelSavegame, "Writing game id (%s, %s)", gameIDstring, _game.id);

	const char *tmp = getGameMD5();
	// As reported in bug report #2849084 "AGI: Crash when saving fallback-matched game"
	// getGameMD5 will return NULL for fallback matched games. Since there is also no
	// filename available we can not compute any MD5 here either. Thus we will just set
	// the MD5 sum in the savegame to all zero, when getGameMD5 returns NULL.
	if (!tmp) {
		for (i = 0; i < 32; ++i)
			out->writeByte(0);
	} else {
		for (i = 0; i < 32; ++i)
			out->writeByte(tmp[i]);
	}

	for (i = 0; i < MAX_FLAGS; i++)
		out->writeByte(_game.flags[i]);
	for (i = 0; i < MAX_VARS; i++)
		out->writeByte(_game.vars[i]);

	out->writeSint16BE((int8)_game.horizon);
	out->writeSint16BE((int16)_game.lineStatus);
	out->writeSint16BE((int16)_game.lineUserInput);
	out->writeSint16BE((int16)_game.lineMinPrint);

	out->writeSint16BE((int16)_game.inputMode);
	out->writeSint16BE((int16)_game.lognum);

	out->writeSint16BE((int16)_game.playerControl);
	out->writeSint16BE((int16)shouldQuit());
	out->writeSint16BE((int16)_game.statusLine);
	out->writeSint16BE((int16)_game.clockEnabled);
	out->writeSint16BE((int16)_game.exitAllLogics);
	out->writeSint16BE((int16)_game.pictureShown);
	out->writeSint16BE((int16)_game.hasPrompt);
	out->writeSint16BE((int16)_game.gameFlags);

	out->writeSint16BE(_game.inputEnabled);

	for (i = 0; i < _HEIGHT; i++)
		out->writeByte(_game.priTable[i]);

	out->writeSint16BE((int16)_game.gfxMode);
	out->writeByte(_game.cursorChar);
	out->writeSint16BE((int16)_game.colorFg);
	out->writeSint16BE((int16)_game.colorBg);

	// game.hires
	// game.sbuf
	// game.ego_words
	// game.num_ego_words

	out->writeSint16BE((int16)_game.numObjects);
	for (i = 0; i < (int16)_game.numObjects; i++)
		out->writeSint16BE((int16)objectGetLocation(i));

	// game.ev_keyp
	for (i = 0; i < MAX_STRINGS; i++)
		out->write(_game.strings[i], MAX_STRINGLEN);

	// record info about loaded resources
	for (i = 0; i < MAX_DIRS; i++) {
		out->writeByte(_game.dirLogic[i].flags);
		out->writeSint16BE((int16)_game.logics[i].sIP);
		out->writeSint16BE((int16)_game.logics[i].cIP);
	}
	for (i = 0; i < MAX_DIRS; i++)
		out->writeByte(_game.dirPic[i].flags);
	for (i = 0; i < MAX_DIRS; i++)
		out->writeByte(_game.dirView[i].flags);
	for (i = 0; i < MAX_DIRS; i++)
		out->writeByte(_game.dirSound[i].flags);

	// game.pictures
	// game.logics
	// game.views
	// game.sounds

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		VtEntry *v = &_game.viewTable[i];

		out->writeByte(v->stepTime);
		out->writeByte(v->stepTimeCount);
		out->writeByte(v->entry);
		out->writeSint16BE(v->xPos);
		out->writeSint16BE(v->yPos);
		out->writeByte(v->currentView);

		// v->view_data

		out->writeByte(v->currentLoop);
		out->writeByte(v->numLoops);

		// v->loop_data

		out->writeByte(v->currentCel);
		out->writeByte(v->numCels);

		// v->cel_data
		// v->cel_data_2

		out->writeSint16BE(v->xPos2);
		out->writeSint16BE(v->yPos2);

		// v->s

		out->writeSint16BE(v->xSize);
		out->writeSint16BE(v->ySize);
		out->writeByte(v->stepSize);
		out->writeByte(v->cycleTime);
		out->writeByte(v->cycleTimeCount);
		out->writeByte(v->direction);

		out->writeByte(v->motion);
		out->writeByte(v->cycle);
		out->writeByte(v->priority);

		out->writeUint16BE(v->flags);

		out->writeByte(v->parm1);
		out->writeByte(v->parm2);
		out->writeByte(v->parm3);
		out->writeByte(v->parm4);
	}

	// Save image stack

	for (Common::Stack<ImageStackElement>::size_type j = 0; j < _imageStack.size(); ++j) {
		const ImageStackElement &ise = _imageStack[j];
		out->writeByte(ise.type);
		out->writeSint16BE(ise.parm1);
		out->writeSint16BE(ise.parm2);
		out->writeSint16BE(ise.parm3);
		out->writeSint16BE(ise.parm4);
		out->writeSint16BE(ise.parm5);
		out->writeSint16BE(ise.parm6);
		out->writeSint16BE(ise.parm7);
	}
	out->writeByte(0);

	//Write which file number AGIPAL is using (0 if not being used)
	out->writeSint16BE(_gfx->getAGIPalFileNum());

	out->finalize();
	if (out->err()) {
		warning("Can't write file '%s'. (Disk full?)", fileName.c_str());
		result = errIOError;
	} else
		debugC(1, kDebugLevelMain | kDebugLevelSavegame, "Saved game %s in file %s", description.c_str(), fileName.c_str());

	delete out;
	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Closed %s", fileName.c_str());

	_lastSaveTime = _system->getMillis();

	return result;
}

int AgiEngine::loadGame(const Common::String &fileName, bool checkId) {
	char description[31], saveVersion, loadId[8];
	int i, vtEntries = MAX_VIEWTABLE;
	uint8 t;
	int16 parm[7];
	Common::InSaveFile *in;

	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "AgiEngine::loadGame(%s)", fileName.c_str());

	if (!(in = _saveFileMan->openForLoading(fileName))) {
		warning("Can't open file '%s', game not loaded", fileName.c_str());
		return errBadFileOpen;
	} else {
		debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Successfully opened %s for reading", fileName.c_str());
	}

	uint32 typea = in->readUint32BE();
	if (typea == AGIflag) {
		debugC(6, kDebugLevelMain | kDebugLevelSavegame, "Has AGI flag, good start");
	} else {
		warning("This doesn't appear to be an AGI savegame, game not restored");
		delete in;
		return errOK;
	}

	in->read(description, 31);

	debugC(6, kDebugLevelMain | kDebugLevelSavegame, "Description is: %s", description);

	saveVersion = in->readByte();
	if (saveVersion < 2)	// is the save game pre-ScummVM?
		warning("Old save game version (%d, current version is %d). Will try and read anyway, but don't be surprised if bad things happen", saveVersion, SAVEGAME_VERSION);

	if (saveVersion < 3)
		warning("This save game contains no AGIPAL data, if the game is using the AGIPAL hack, it won't work correctly");

	if (saveVersion >= 4) {
		// We don't need the thumbnail here, so just read it and discard it
		Graphics::skipThumbnail(*in);

		in->readUint32BE();	// save date
		in->readUint16BE(); // save time
		if (saveVersion >= 6) {
			uint32 playTime = in->readUint32BE();
			g_engine->setTotalPlayTime(playTime * 1000);
		}
	}

	_game.state = (State)in->readByte();

	in->read(loadId, 8);
	if (strcmp(loadId, _game.id) != 0 && checkId) {
		delete in;
		warning("This save seems to be from a different AGI game (save from %s, running %s), not loaded", loadId, _game.id);
		return errBadFileOpen;
	}

	strncpy(_game.id, loadId, 8);

	if (saveVersion >= 5) {
		char md5[32 + 1];

		for (i = 0; i < 32; i++) {
			md5[i] = in->readByte();

		}
		md5[i] = 0; // terminate

		// As noted above in AgiEngine::saveGame the MD5 sum field may be all zero
		// when the save was made via a fallback matched game. In this case we will
		// replace the MD5 sum with a nicer string, so that the user can easily see
		// this fact in the debug output. The string saved in "md5" will never match
		// any valid MD5 sum, thus it is safe to do that here.
		if (md5[0] == 0)
			strcpy(md5, "fallback matched");

		debug(0, "Saved game MD5: \"%s\"", md5);

		if (!getGameMD5()) {
			warning("Since your game was only detected via the fallback detector, there is no possibility to assure the save is compatible with your game version");

			debug(0, "The game used for saving is \"%s\".", md5);
		} else if (strcmp(md5, getGameMD5()) != 0) {
			warning("Game was saved with different gamedata - you may encounter problems");

			debug(0, "Your game is \"%s\" and save is \"%s\".", getGameMD5(), md5);
		}
	}

	for (i = 0; i < MAX_FLAGS; i++)
		_game.flags[i] = in->readByte();
	for (i = 0; i < MAX_VARS; i++)
		_game.vars[i] = in->readByte();

	setvar(vFreePages, 180); // Set amount of free memory to realistic value (Overwriting the just loaded value)

	_game.horizon = in->readSint16BE();
	_game.lineStatus = in->readSint16BE();
	_game.lineUserInput = in->readSint16BE();
	_game.lineMinPrint = in->readSint16BE();

	// These are never saved
	_game.cursorPos = 0;
	_game.inputBuffer[0] = 0;
	_game.echoBuffer[0] = 0;
	_game.keypress = 0;

	_game.inputMode = (InputMode)in->readSint16BE();
	_game.lognum = in->readSint16BE();

	_game.playerControl = in->readSint16BE();
	if (in->readSint16BE())
		quitGame();
	_game.statusLine = in->readSint16BE();
	_game.clockEnabled = in->readSint16BE();
	_game.exitAllLogics = in->readSint16BE();
	_game.pictureShown = in->readSint16BE();
	_game.hasPrompt = in->readSint16BE();
	_game.gameFlags = in->readSint16BE();
	_game.inputEnabled = in->readSint16BE();

	for (i = 0; i < _HEIGHT; i++)
		_game.priTable[i] = in->readByte();

	if (_game.hasWindow)
		closeWindow();

	_game.msgBoxTicks = 0;
	_game.block.active = false;
	// game.window - fixed by close_window()
	// game.has_window - fixed by close_window()

	_game.gfxMode = in->readSint16BE();
	_game.cursorChar = in->readByte();
	_game.colorFg = in->readSint16BE();
	_game.colorBg = in->readSint16BE();

	// game.hires - rebuilt from image stack
	// game.sbuf - rebuilt from image stack

	// game.ego_words - fixed by clean_input
	// game.num_ego_words - fixed by clean_input

	_game.numObjects = in->readSint16BE();
	for (i = 0; i < (int16)_game.numObjects; i++)
		objectSetLocation(i, in->readSint16BE());

	// Those are not serialized
	for (i = 0; i < MAX_DIRS; i++) {
		_game.controllerOccured[i] = false;
	}

	for (i = 0; i < MAX_STRINGS; i++)
		in->read(_game.strings[i], MAX_STRINGLEN);

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rLOGIC, i);
		else
			agiUnloadResource(rLOGIC, i);
		_game.logics[i].sIP = in->readSint16BE();
		_game.logics[i].cIP = in->readSint16BE();
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rPICTURE, i);
		else
			agiUnloadResource(rPICTURE, i);
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rVIEW, i);
		else
			agiUnloadResource(rVIEW, i);
	}

	for (i = 0; i < MAX_DIRS; i++) {
		if (in->readByte() & RES_LOADED)
			agiLoadResource(rSOUND, i);
		else
			agiUnloadResource(rSOUND, i);
	}

	// game.pictures - loaded above
	// game.logics - loaded above
	// game.views - loaded above
	// game.sounds - loaded above

	for (i = 0; i < vtEntries; i++) {
		VtEntry *v = &_game.viewTable[i];

		v->stepTime = in->readByte();
		v->stepTimeCount = in->readByte();
		v->entry = in->readByte();
		v->xPos = in->readSint16BE();
		v->yPos = in->readSint16BE();
		v->currentView = in->readByte();

		// v->view_data - fixed below

		v->currentLoop = in->readByte();
		v->numLoops = in->readByte();

		// v->loop_data - fixed below

		v->currentCel = in->readByte();
		v->numCels = in->readByte();

		// v->cel_data - fixed below
		// v->cel_data_2 - fixed below

		v->xPos2 = in->readSint16BE();
		v->yPos2 = in->readSint16BE();

		// v->s - fixed below

		v->xSize = in->readSint16BE();
		v->ySize = in->readSint16BE();
		v->stepSize = in->readByte();
		v->cycleTime = in->readByte();
		v->cycleTimeCount = in->readByte();
		v->direction = in->readByte();

		v->motion = (MotionType)in->readByte();
		v->cycle = (CycleType)in->readByte();
		v->priority = in->readByte();

		v->flags = in->readUint16BE();

		v->parm1 = in->readByte();
		v->parm2 = in->readByte();
		v->parm3 = in->readByte();
		v->parm4 = in->readByte();
	}
	for (i = vtEntries; i < MAX_VIEWTABLE; i++) {
		memset(&_game.viewTable[i], 0, sizeof(VtEntry));
	}

	// Fix some pointers in viewtable

	for (i = 0; i < MAX_VIEWTABLE; i++) {
		VtEntry *v = &_game.viewTable[i];

		if (_game.dirView[v->currentView].offset == _EMPTY)
			continue;

		if (!(_game.dirView[v->currentView].flags & RES_LOADED))
			agiLoadResource(rVIEW, v->currentView);

		setView(v, v->currentView);	// Fix v->view_data
		setLoop(v, v->currentLoop);	// Fix v->loop_data
		setCel(v, v->currentCel);	// Fix v->cel_data
		v->celData2 = v->celData;
		v->s = NULL;	// not sure if it is used...
	}

	_sprites->eraseBoth();

	// Clear input line
	_gfx->clearScreen(0);
	writeStatus();

	// Recreate background from saved image stack
	clearImageStack();
	while ((t = in->readByte()) != 0) {
		for (i = 0; i < 7; i++)
			parm[i] = in->readSint16BE();
		replayImageStackCall(t, parm[0], parm[1], parm[2],
				parm[3], parm[4], parm[5], parm[6]);
	}

	// Load AGIPAL Data
	if (saveVersion >= 3)
		_gfx->setAGIPal(in->readSint16BE());

	delete in;
	debugC(3, kDebugLevelMain | kDebugLevelSavegame, "Closed %s", fileName.c_str());

	setflag(fRestoreJustRan, true);

	_game.hasPrompt = 0;	// force input line repaint if necessary
	cleanInput();

	_sprites->eraseBoth();
	_sprites->blitBoth();
	_sprites->commitBoth();
	_picture->showPic();
	_gfx->doUpdate();

	return errOK;
}

#define NUM_SLOTS 100
#define NUM_VISIBLE_SLOTS 12

Common::String AgiEngine::getSavegameFilename(int num) const {
	Common::String saveLoadSlot = _targetName;
	saveLoadSlot += Common::String::format(".%.3d", num);
	return saveLoadSlot;
}

void AgiEngine::getSavegameDescription(int num, char *buf, bool showEmpty) {
	Common::InSaveFile *in;
	Common::String fileName = getSavegameFilename(num);

	debugC(4, kDebugLevelMain | kDebugLevelSavegame, "Current game id is %s", _targetName.c_str());

	if (!(in = _saveFileMan->openForLoading(fileName))) {
		debugC(4, kDebugLevelMain | kDebugLevelSavegame, "File %s does not exist", fileName.c_str());

		if (showEmpty)
			strcpy(buf, "        (empty slot)");
		else
			*buf = 0;
	} else {
		debugC(4, kDebugLevelMain | kDebugLevelSavegame, "Successfully opened %s for reading", fileName.c_str());

		uint32 type = in->readUint32BE();

		if (type == AGIflag) {
			debugC(6, kDebugLevelMain | kDebugLevelSavegame, "Has AGI flag, good start");
			in->read(buf, 31);
		} else {
			warning("This doesn't appear to be an AGI savegame");
			strcpy(buf, "(corrupt file)");
		}

		delete in;
	}
}

int AgiEngine::selectSlot() {
	int i, key, active = 0;
	int rc = -1;
	int hm = 1, vm = 3;	// box margins
	int xmin, xmax, slotClicked;
	char desc[NUM_VISIBLE_SLOTS][40];
	int textCenter, buttonLength, buttonX[2], buttonY;
	const char *buttonText[] = { "  OK  ", "Cancel", NULL };

	_noSaveLoadAllowed = true;

	for (i = 0; i < NUM_VISIBLE_SLOTS; i++) {
		getSavegameDescription(_firstSlot + i, desc[i]);
	}

	textCenter = GFX_WIDTH / CHAR_LINES / 2;
	buttonLength = 6;
	buttonX[0] = (textCenter - 3 * buttonLength / 2) * CHAR_COLS;
	buttonX[1] = (textCenter + buttonLength / 2) * CHAR_COLS;
	buttonY = (vm + 17) * CHAR_LINES;

	for (i = 0; i < 2; i++)
		_gfx->drawCurrentStyleButton(buttonX[i], buttonY, buttonText[i], false, false, i == 0);

	AllowSyntheticEvents on(this);
	int oldFirstSlot = _firstSlot + 1;
	int oldActive = active + 1;
	bool exitSelectSlot = false;
	while (!exitSelectSlot && !(shouldQuit() || _restartGame)) {
		int sbPos = 0;

		// Use the extreme scrollbar positions only if the extreme
		// slots are in sight. (We have to calculate this even if we
		// don't redraw the save slots, because it's also used for
		// clicking in the scrollbar.

		if (_firstSlot == 0)
			sbPos = 1;
		else if (_firstSlot == NUM_SLOTS - NUM_VISIBLE_SLOTS)
			sbPos = NUM_VISIBLE_SLOTS - 2;
		else {
			sbPos = 2 + (_firstSlot * (NUM_VISIBLE_SLOTS - 4)) / (NUM_SLOTS - NUM_VISIBLE_SLOTS - 1);
			if (sbPos >= NUM_VISIBLE_SLOTS - 3)
				sbPos = NUM_VISIBLE_SLOTS - 3;
		}

		if (oldFirstSlot != _firstSlot || oldActive != active) {
			char dstr[64];
			for (i = 0; i < NUM_VISIBLE_SLOTS; i++) {
				sprintf(dstr, "[%2d. %-28.28s]", i + _firstSlot, desc[i]);
				printText(dstr, 0, hm + 1, vm + 4 + i,
						(40 - 2 * hm) - 1, i == active ? MSG_BOX_COLOR : MSG_BOX_TEXT,
						i == active ? MSG_BOX_TEXT : MSG_BOX_COLOR);
			}

			char upArrow[] = "^";
			char downArrow[] = "v";
			char scrollBar[] = " ";

			for (i = 1; i < NUM_VISIBLE_SLOTS - 1; i++)
				printText(scrollBar, 35, hm + 1, vm + 4 + i, 1, MSG_BOX_COLOR, 7, true);

			printText(upArrow, 35, hm + 1, vm + 4, 1, 8, 7);
			printText(downArrow, 35, hm + 1, vm + 4 + NUM_VISIBLE_SLOTS - 1, 1, 8, 7);
			printText(scrollBar, 35, hm + 1, vm + 4 + sbPos, 1, MSG_BOX_COLOR, MSG_BOX_TEXT);

			oldActive = active;
			oldFirstSlot = _firstSlot;
		}

		pollTimer();
		key = doPollKeyboard();

		// It may happen that somebody will open GMM while
		// this dialog is open, and load a game
		// We are processing it here, effectively jumping
		// out of the dead loop
		if (getflag(fRestoreJustRan)) {
			rc = -2;
			exitSelectSlot = true;
		}

		if (!exitSelectSlot) {
			switch (key) {
			case KEY_ENTER:
				rc = active;
				strncpy(_game.strings[MAX_STRINGS], desc[i], MAX_STRINGLEN);
				debugC(8, kDebugLevelMain | kDebugLevelInput, "Button pressed: %d", rc);
				exitSelectSlot = true;
				break;
			case KEY_ESCAPE:
				rc = -1;
				exitSelectSlot = true;
				break;
			case BUTTON_LEFT:
				if (_gfx->testButton(buttonX[0], buttonY, buttonText[0])) {
					rc = active;
					strncpy(_game.strings[MAX_STRINGS], desc[i], MAX_STRINGLEN);
					debugC(8, kDebugLevelMain | kDebugLevelInput, "Button pressed: %d", rc);
					exitSelectSlot = true;
				} else if (_gfx->testButton(buttonX[1], buttonY, buttonText[1])) {
					rc = -1;
					exitSelectSlot = true;
				} else {
					slotClicked = ((int)_mouse.y - 1) / CHAR_COLS - (vm + 4);
					xmin = (hm + 1) * CHAR_COLS;
					xmax = xmin + CHAR_COLS * 34;
					if ((int)_mouse.x >= xmin && (int)_mouse.x <= xmax) {
						if (slotClicked >= 0 && slotClicked < NUM_VISIBLE_SLOTS)
							active = slotClicked;
					}
					xmin = (hm + 36) * CHAR_COLS;
					xmax = xmin + CHAR_COLS;
					if ((int)_mouse.x >= xmin && (int)_mouse.x <= xmax) {
						if (slotClicked >= 0 && slotClicked < NUM_VISIBLE_SLOTS) {
							if (slotClicked == 0)
								keyEnqueue(KEY_UP);
							else if (slotClicked == NUM_VISIBLE_SLOTS - 1)
								keyEnqueue(KEY_DOWN);
							else if (slotClicked < sbPos)
								keyEnqueue(KEY_UP_RIGHT);
							else if (slotClicked > sbPos)
								keyEnqueue(KEY_DOWN_RIGHT);
						}
					}
				}
				break;

			case KEY_DOWN:
				active++;
				if (active >= NUM_VISIBLE_SLOTS) {
					if (_firstSlot + NUM_VISIBLE_SLOTS < NUM_SLOTS) {
						_firstSlot++;
						for (i = 1; i < NUM_VISIBLE_SLOTS; i++)
							memcpy(desc[i - 1], desc[i], sizeof(desc[0]));
						getSavegameDescription(_firstSlot + NUM_VISIBLE_SLOTS - 1, desc[NUM_VISIBLE_SLOTS - 1]);
					}
					active = NUM_VISIBLE_SLOTS - 1;
				}
				break;
			case KEY_UP:
				active--;
				if (active < 0) {
					active = 0;
					if (_firstSlot > 0) {
						_firstSlot--;
						for (i = NUM_VISIBLE_SLOTS - 1; i > 0; i--)
							memcpy(desc[i], desc[i - 1], sizeof(desc[0]));
						getSavegameDescription(_firstSlot, desc[0]);
					}
				}
				break;

			// Page Up/Down and mouse wheel scrolling all leave 'active'
			// unchanged so that a visible slot will remain selected.

			case WHEEL_DOWN:
				if (_firstSlot < NUM_SLOTS - NUM_VISIBLE_SLOTS) {
					_firstSlot++;
					for (i = 1; i < NUM_VISIBLE_SLOTS; i++)
						memcpy(desc[i - 1], desc[i], sizeof(desc[0]));
					getSavegameDescription(_firstSlot + NUM_VISIBLE_SLOTS - 1, desc[NUM_VISIBLE_SLOTS - 1]);
				}
				break;
			case WHEEL_UP:
				if (_firstSlot > 0) {
					_firstSlot--;
					for (i = NUM_VISIBLE_SLOTS - 1; i > 0; i--)
						memcpy(desc[i], desc[i - 1], sizeof(desc[0]));
					getSavegameDescription(_firstSlot, desc[0]);
				}
				break;
			case KEY_DOWN_RIGHT:
				// This is probably triggered by Page Down.
				_firstSlot += NUM_VISIBLE_SLOTS;
				if (_firstSlot > NUM_SLOTS - NUM_VISIBLE_SLOTS) {
					_firstSlot = NUM_SLOTS - NUM_VISIBLE_SLOTS;
				}
				for (i = 0; i < NUM_VISIBLE_SLOTS; i++)
					getSavegameDescription(_firstSlot + i, desc[i]);
				break;
			case KEY_UP_RIGHT:
				// This is probably triggered by Page Up.
				_firstSlot -= NUM_VISIBLE_SLOTS;
				if (_firstSlot < 0) {
					_firstSlot = 0;
				}
				for (i = 0; i < NUM_VISIBLE_SLOTS; i++)
					getSavegameDescription(_firstSlot + i, desc[i]);
				break;
			}
		}
		_gfx->doUpdate();
	}

	closeWindow();

	_noSaveLoadAllowed = false;

	return rc;
}

int AgiEngine::scummVMSaveLoadDialog(bool isSave) {
	GUI::SaveLoadChooser *dialog;
	Common::String desc;
	int slot;

	if (isSave) {
		dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

		slot = dialog->runModalWithCurrentTarget();
		desc = dialog->getResultString();

		if (desc.empty()) {
			// create our own description for the saved game, the user didnt enter it
			desc = dialog->createDefaultSaveDescription(slot);
		}

		if (desc.size() > 28)
			desc = Common::String(desc.c_str(), 28);
	} else {
		dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
		slot = dialog->runModalWithCurrentTarget();
	}

	delete dialog;

	if (slot < 0)
		return true;

	if (isSave)
		return doSave(slot, desc);
	else
		return doLoad(slot, false);
}

int AgiEngine::doSave(int slot, const Common::String &desc) {
	Common::String fileName = getSavegameFilename(slot);
	debugC(8, kDebugLevelMain | kDebugLevelResources, "file is [%s]", fileName.c_str());

	// Make sure all graphics was blitted to screen. This fixes bug
	// #2960567: "AGI: Ego partly erased in Load/Save thumbnails"
	_gfx->doUpdate();

	return saveGame(fileName, desc);
}

int AgiEngine::doLoad(int slot, bool showMessages) {
	Common::String fileName = getSavegameFilename(slot);
	debugC(8, kDebugLevelMain | kDebugLevelResources, "file is [%s]", fileName.c_str());

	_sprites->eraseBoth();
	_sound->stopSound();
	closeWindow();

	int result = loadGame(fileName);

	if (result == errOK) {
		if (showMessages)
			messageBox("Game restored.");
		_game.exitAllLogics = 1;
		_menu->enableAll();
	} else {
		if (showMessages)
			messageBox("Error restoring game.");
	}

	return result;
}

int AgiEngine::saveGameDialog() {
	if (!ConfMan.getBool("originalsaveload"))
		return scummVMSaveLoadDialog(true);

	char *desc;
	const char *buttons[] = { "Do as I say!", "I regret", NULL };
	char dstr[200];
	int rc, slot = 0;
	int hm, vm, hp, vp;
	int w;

	hm = 1;
	vm = 3;
	hp = hm * CHAR_COLS;
	vp = vm * CHAR_LINES;
	w = (40 - 2 * hm) - 1;

	do {
		drawWindow(hp, vp, GFX_WIDTH - hp, GFX_HEIGHT - vp);
		printText("Select a slot in which you wish to\nsave the game:",
				0, hm + 1, vm + 1, w, MSG_BOX_TEXT, MSG_BOX_COLOR);
		slot = selectSlot();
		if (slot + _firstSlot == 0)
			messageBox("That slot is for Autosave only.");
		else if (slot < 0)
			return errOK;
	} while (slot + _firstSlot == 0);

	drawWindow(hp, vp + 5 * CHAR_LINES, GFX_WIDTH - hp,
			GFX_HEIGHT - vp - 9 * CHAR_LINES);
	printText("Enter a description for this game:",
			0, hm + 1, vm + 6, w, MSG_BOX_TEXT, MSG_BOX_COLOR);
	_gfx->drawRectangle(3 * CHAR_COLS, 11 * CHAR_LINES - 1,
			37 * CHAR_COLS, 12 * CHAR_LINES, MSG_BOX_TEXT);
	_gfx->flushBlock(3 * CHAR_COLS, 11 * CHAR_LINES - 1,
			37 * CHAR_COLS, 12 * CHAR_LINES);

	// The description field of the save/restore dialog holds 32 characters
	// but we use four of them for the slot number. The input field is a
	// bit wider than that, so we don't have to worry about leaving space
	// for the cursor.

	getString(2, 11, 28, MAX_STRINGS);

	// If we're saving over an old slot, show the old description. We can't
	// access that buffer directly, so we have to feed the characters to
	// the input handler one at a time.

	char name[40];
	int numChars;

	getSavegameDescription(_firstSlot + slot, name, false);

	for (numChars = 0; numChars < 28 && name[numChars]; numChars++)
		handleGetstring(name[numChars]);

	_gfx->printCharacter(numChars + 3, 11, _game.cursorChar, MSG_BOX_COLOR, MSG_BOX_TEXT);
	do {
		mainCycle();
	} while (_game.inputMode == INPUT_GETSTRING);
	closeWindow();

	desc = _game.strings[MAX_STRINGS];
	sprintf(dstr, "Are you sure you want to save the game "
			"described as:\n\n%s\n\nin slot %d?\n\n\n", desc, _firstSlot + slot);

	rc = selectionBox(dstr, buttons);

	if (rc != 0) {
		messageBox("Game NOT saved.");
		return errOK;
	}

	int result = doSave(_firstSlot + slot, desc);

	if (result == errOK)
		messageBox("Game saved.");
	else
		messageBox("Error saving game.");

	return result;
}

int AgiEngine::saveGameSimple() {
	if (!ConfMan.getBool("originalsaveload"))
		return scummVMSaveLoadDialog(true);

	Common::String fileName = getSavegameFilename(0);

	int result = saveGame(fileName, "Default savegame");
	if (result != errOK)
		messageBox("Error saving game.");
	return result;
}

int AgiEngine::loadGameDialog() {
	if (!ConfMan.getBool("originalsaveload"))
		return scummVMSaveLoadDialog(false);

	int slot = 0;
	int hm, vm, hp, vp;	// box margins
	int w;

	hm = 1;
	vm = 3;
	hp = hm * CHAR_COLS;
	vp = vm * CHAR_LINES;
	w = (40 - 2 * hm) - 1;

	_sprites->eraseBoth();
	_sound->stopSound();

	drawWindow(hp, vp, GFX_WIDTH - hp, GFX_HEIGHT - vp);
	printText("Select a game which you wish to\nrestore:",
			0, hm + 1, vm + 1, w, MSG_BOX_TEXT, MSG_BOX_COLOR);

	slot = selectSlot();

	if (slot < 0) {
		if (slot == -1) // slot = -2 when GMM was launched
			messageBox("Game NOT restored.");

		return errOK;
	}

	return doLoad(_firstSlot + slot, true);
}

int AgiEngine::loadGameSimple() {
	if (!ConfMan.getBool("originalsaveload"))
		return scummVMSaveLoadDialog(false);
	else
		return doLoad(0, true);
}

void AgiEngine::recordImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) {
	ImageStackElement pnew;

	pnew.type = type;
	pnew.pad = 0;
	pnew.parm1 = p1;
	pnew.parm2 = p2;
	pnew.parm3 = p3;
	pnew.parm4 = p4;
	pnew.parm5 = p5;
	pnew.parm6 = p6;
	pnew.parm7 = p7;

	_imageStack.push(pnew);
}

void AgiEngine::replayImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7) {
	switch (type) {
	case ADD_PIC:
		debugC(8, kDebugLevelMain, "--- decoding picture %d ---", p1);
		agiLoadResource(rPICTURE, p1);
		_picture->decodePicture(p1, p2, p3 != 0);
		break;
	case ADD_VIEW:
		agiLoadResource(rVIEW, p1);
		_sprites->addToPic(p1, p2, p3, p4, p5, p6, p7);
		break;
	}
}

void AgiEngine::clearImageStack() {
	_imageStack.clear();
}

void AgiEngine::releaseImageStack() {
	_imageStack.clear();
}

void AgiEngine::checkQuickLoad() {
	if (ConfMan.hasKey("save_slot")) {
		Common::String saveNameBuffer = getSavegameFilename(ConfMan.getInt("save_slot"));

		_sprites->eraseBoth();
		_sound->stopSound();

		if (loadGame(saveNameBuffer, false) == errOK) {	 // Do not check game id
			_game.exitAllLogics = 1;
			_menu->enableAll();
		}
	}
}

Common::Error AgiEngine::loadGameState(int slot) {
	Common::String saveLoadSlot = getSavegameFilename(slot);

	_sprites->eraseBoth();
	_sound->stopSound();

	if (loadGame(saveLoadSlot) == errOK) {
		_game.exitAllLogics = 1;
		_menu->enableAll();
		return Common::kNoError;
	} else {
		return Common::kUnknownError;
	}
}

Common::Error AgiEngine::saveGameState(int slot, const Common::String &desc) {
	Common::String saveLoadSlot = getSavegameFilename(slot);
	if (saveGame(saveLoadSlot, desc) == errOK)
		return Common::kNoError;
	else
		return Common::kUnknownError;
}

} // End of namespace Agi
