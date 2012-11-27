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


#include "kyra/gui.h"
#include "kyra/kyra_v1.h"
#include "kyra/util.h"

#include "common/savefile.h"
#include "common/system.h"


namespace Kyra {

GUI::GUI(KyraEngine_v1 *kyra) : _vm(kyra), _screen(kyra->screen()) {
	_saveSlotsListUpdateNeeded = true;
	_savegameListSize = 0;
	_savegameList = 0;
}

GUI::~GUI() {
	if (_savegameList) {
		for (int i = 0; i < _savegameListSize; i++)
			delete[] _savegameList[i];
		delete[] _savegameList;
		_savegameList = 0;
	}
}

void GUI::updateSaveFileList(Common::String targetName, bool excludeQuickSaves) {
	Common::String pattern = targetName + ".???";
	Common::StringArray saveFileList = _vm->_saveFileMan->listSavefiles(pattern);
	_saveSlots.clear();

	for (Common::StringArray::const_iterator i = saveFileList.begin(); i != saveFileList.end(); ++i) {
		char s1 = 0, s2 = 0, s3 = 0;
		s1 = (*i)[i->size() - 3];
		s2 = (*i)[i->size() - 2];
		s3 = (*i)[i->size() - 1];
		if (!Common::isDigit(s1) || !Common::isDigit(s2) || !Common::isDigit(s3))
			continue;
		s1 -= '0';
		s2 -= '0';
		s3 -= '0';
		if (excludeQuickSaves && s1 == 9 && s2 == 9)
			continue;
		_saveSlots.push_back(s1 * 100 + s2 * 10 + s3);
	}

	if (_saveSlots.begin() == _saveSlots.end())
		return;

	sortSaveSlots();
}

void GUI::sortSaveSlots() {
	Common::sort(_saveSlots.begin(), _saveSlots.end(), Common::Less<int>());
	if (_saveSlots.size() > 2)
		Common::sort(_saveSlots.begin() + 1, _saveSlots.end(), Common::Greater<int>());
}

int GUI::getNextSavegameSlot() {
	Common::InSaveFile *in;

	int start = _vm->game() == GI_LOL ? 0 : 1;

	for (int i = start; i < 990; i++) {
		if ((in = _vm->_saveFileMan->openForLoading(_vm->getSavegameFilename(i))))
			delete in;
		else
			return i;
	}
	warning("Didn't save: Ran out of saveGame filenames");
	return 0;
}

void GUI::updateSaveSlotsList(Common::String targetName, bool force) {
	if (!_saveSlotsListUpdateNeeded && !force)
		return;

	_saveSlotsListUpdateNeeded = false;

	if (_savegameList) {
		for (int i = 0; i < _savegameListSize; i++)
			delete[] _savegameList[i];
		delete[] _savegameList;
	}

	updateSaveFileList(targetName, true);
	int numSaves = _savegameListSize = _saveSlots.size();
	bool allowEmptySlots = (_vm->game() == GI_EOB1 || _vm->game() == GI_EOB2);

	if (_savegameListSize) {
		if (allowEmptySlots)
			_savegameListSize = 990;

		KyraEngine_v1::SaveHeader header;
		Common::InSaveFile *in;

		_savegameList = new char*[_savegameListSize];
		memset(_savegameList, 0, _savegameListSize * sizeof(char *));

		for (int i = 0; i < numSaves; i++) {
			in = _vm->openSaveForReading(_vm->getSavegameFilename(targetName, _saveSlots[i]).c_str(), header, targetName == _vm->_targetName);
			char **listEntry = &_savegameList[allowEmptySlots ? _saveSlots[i] : i];
			if (in) {
				*listEntry = new char[header.description.size() + 1];
				Common::strlcpy(*listEntry, header.description.c_str(), header.description.size() + 1);
				Util::convertISOToDOS(*listEntry);
				delete in;
			} else {
				*listEntry = 0;
				error("GUI::updateSavegameList(): Unexpected missing save file for slot: %d.", _saveSlots[i]);
			}
		}

	} else {
		_savegameList = 0;
	}
}

} // End of namespace Kyra
