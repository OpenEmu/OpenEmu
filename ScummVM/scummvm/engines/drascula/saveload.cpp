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

#include "common/textconsole.h"

#include "drascula/drascula.h"

namespace Drascula {

/**
 * Loads the save names from the EPA index file.
 *
 * TODO: We should move the save names in their respective save files and get
 * rid of this completely. A good example is the sword1 engine, which also used
 * to have an index file for its saves, that has been removed.
 * sword1 contains code that converts the old index-based saves into the new
 * format without the index file, so we could apply this idea to drascula as
 * well.
 */
void DrasculaEngine::loadSaveNames() {
	Common::InSaveFile *sav;
	Common::String fileEpa = Common::String::format("%s.epa", _targetName.c_str());

	// Create and initialize the index file, if it doesn't exist
	if (!(sav = _saveFileMan->openForLoading(fileEpa))) {
		Common::OutSaveFile *epa;
		if (!(epa = _saveFileMan->openForSaving(fileEpa)))
			error("Can't open %s file", fileEpa.c_str());
		for (int n = 0; n < NUM_SAVES; n++)
			epa->writeString("*\n");
		epa->finalize();
		delete epa;
		if (!(sav = _saveFileMan->openForLoading(fileEpa))) {
			error("Can't open %s file", fileEpa.c_str());
		}
	}

	// Load the index file
	for (int n = 0; n < NUM_SAVES; n++) {
		strncpy(_saveNames[n], sav->readLine().c_str(), 23);
		_saveNames[n][22] = '\0';	// make sure the savegame name is 0-terminated
	}
	delete sav;
}

/**
 * Saves the save names into the EPA index file.
 *
 * TODO: We should move the save names in their respective save files and get
 * rid of this completely. A good example is the sword1 engine, which also used
 * to have an index file for its saves, that has been removed.
 * sword1 contains code that converts the old index-based saves into the new
 * format without the index file, so we could apply this idea to drascula as
 * well.
 */
void DrasculaEngine::saveSaveNames() {
	Common::OutSaveFile *tsav;
	Common::String fileEpa = Common::String::format("%s.epa", _targetName.c_str());

	if (!(tsav = _saveFileMan->openForSaving(fileEpa))) {
		error("Can't open %s file", fileEpa.c_str());
	}
	for (int n = 0; n < NUM_SAVES; n++) {
		tsav->writeString(_saveNames[n]);
		tsav->writeString("\n");
	}
	tsav->finalize();
	delete tsav;
}

bool DrasculaEngine::saveLoadScreen() {
	Common::String file;
	int n, n2, num_sav = 0, y = 27;

	clearRoom();

	loadSaveNames();

	loadPic("savescr.alg", bgSurface, HALF_PAL);

	color_abc(kColorLightGreen);

	select[0] = 0;

	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	setCursor(kCursorCrosshair);

	while (!shouldQuit()) {
		y = 27;
		copyBackground();
		for (n = 0; n < NUM_SAVES; n++) {
			print_abc(_saveNames[n], 116, y);
			y = y + 9;
		}
		print_abc(select, 117, 15);
		updateScreen();
		y = 27;

		updateEvents();

		if (leftMouseButton == 1) {
			delay(50);
			for (n = 0; n < NUM_SAVES; n++) {
				if (mouseX > 115 && mouseY > y + (9 * n) && mouseX < 115 + 175 && mouseY < y + 10 + (9 * n)) {
					strcpy(select, _saveNames[n]);

					if (strcmp(select, "*") != 0)
						selectionMade = 1;
					else {
						enterName();
						strcpy(_saveNames[n], select);
						if (selectionMade == 1) {
							file = Common::String::format("%s%02d", _targetName.c_str(), n + 1);
							saveGame(file.c_str());
							saveSaveNames();
						}
					}

					print_abc(select, 117, 15);
					y = 27;
					for (n2 = 0; n2 < NUM_SAVES; n2++) {
						print_abc(_saveNames[n2], 116, y);
						y = y + 9;
					}
					if (selectionMade == 1) {
						file = Common::String::format("%s%02d", _targetName.c_str(), n + 1);
					}
					num_sav = n;
				}
			}

			if (mouseX > 117 && mouseY > 15 && mouseX < 295 && mouseY < 24 && selectionMade == 1) {
				enterName();
				strcpy(_saveNames[num_sav], select);
				print_abc(select, 117, 15);
				y = 27;
				for (n2 = 0; n2 < NUM_SAVES; n2++) {
					print_abc(_saveNames[n2], 116, y);
					y = y + 9;
				}

				if (selectionMade == 1) {
					file = Common::String::format("%s%02d", _targetName.c_str(), n + 1);
					saveGame(file.c_str());
					saveSaveNames();
				}
			}

			if (mouseX > 125 && mouseY > 123 && mouseX < 199 && mouseY < 149 && selectionMade == 1) {
				if (!loadGame(file.c_str())) {
					_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
					return false;
				}
				break;
			} else if (mouseX > 208 && mouseY > 123 && mouseX < 282 && mouseY < 149 && selectionMade == 1) {
				saveGame(file.c_str());
				saveSaveNames();
			} else if (mouseX > 168 && mouseY > 154 && mouseX < 242 && mouseY < 180)
				break;
			else if (selectionMade == 0) {
				print_abc("Please select a slot", 117, 15);
			}
			updateScreen();
			delay(200);
		}
		y = 26;

		delay(5);
	}

	selectVerb(kVerbNone);

	clearRoom();
	loadPic(roomNumber, bgSurface, HALF_PAL);
	selectionMade = 0;

	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

	return true;
}

bool DrasculaEngine::loadGame(const char *gameName) {
	int l, savedChapter, roomNum = 0;
	Common::InSaveFile *sav;

	previousMusic = roomMusic;
	_menuScreen = false;
	if (currentChapter != 1)
		clearRoom();

	if (!(sav = _saveFileMan->openForLoading(gameName))) {
		error("missing savegame file");
	}

	savedChapter = sav->readSint32LE();
	if (savedChapter != currentChapter) {
		strcpy(saveName, gameName);
		currentChapter = savedChapter - 1;
		loadedDifferentChapter = 1;
		return false;
	}
	sav->read(currentData, 20);
	curX = sav->readSint32LE();
	curY = sav->readSint32LE();
	trackProtagonist = sav->readSint32LE();

	for (l = 1; l < ARRAYSIZE(inventoryObjects); l++) {
		inventoryObjects[l] = sav->readSint32LE();
	}

	for (l = 0; l < NUM_FLAGS; l++) {
		flags[l] = sav->readSint32LE();
	}

	takeObject = sav->readSint32LE();
	pickedObject = sav->readSint32LE();
	loadedDifferentChapter = 0;
	if (!sscanf(currentData, "%d.ald", &roomNum)) {
		error("Bad save format");
	}
	enterRoom(roomNum);
	selectVerb(kVerbNone);

	return true;
}

void DrasculaEngine::saveGame(const char *gameName) {
	Common::OutSaveFile *out;
	int l;

	if (!(out = _saveFileMan->openForSaving(gameName))) {
		error("Unable to open the file");
	}
	out->writeSint32LE(currentChapter);
	out->write(currentData, 20);
	out->writeSint32LE(curX);
	out->writeSint32LE(curY);
	out->writeSint32LE(trackProtagonist);

	for (l = 1; l < ARRAYSIZE(inventoryObjects); l++) {
		out->writeSint32LE(inventoryObjects[l]);
	}

	for (l = 0; l < NUM_FLAGS; l++) {
		out->writeSint32LE(flags[l]);
	}

	out->writeSint32LE(takeObject);
	out->writeSint32LE(pickedObject);

	out->finalize();
	if (out->err())
		warning("Can't write file '%s'. (Disk full?)", gameName);

	delete out;
}

} // End of namespace Drascula
