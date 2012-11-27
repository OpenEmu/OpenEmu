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

#ifndef PARALLACTION_SAVELOAD_H
#define PARALLACTION_SAVELOAD_H

namespace Parallaction {

struct Character;

class SaveLoad {
protected:
	Common::SaveFileManager	*_saveFileMan;
	Common::String _saveFilePrefix;

	Common::String genSaveFileName(uint slot);
	Common::InSaveFile *getInSaveFile(uint slot);
	Common::OutSaveFile *getOutSaveFile(uint slot);
	int selectSaveFile(Common::String &selectedName, bool saveMode, const Common::String &caption, const Common::String &button);
	int buildSaveFileList(Common::StringArray& l);
	virtual void doLoadGame(uint16 slot) = 0;
	virtual void doSaveGame(uint16 slot, const char* name) = 0;

public:
	SaveLoad(Common::SaveFileManager* saveFileMan, const char *prefix) : _saveFileMan(saveFileMan), _saveFilePrefix(prefix) { }
	virtual ~SaveLoad() { }

	virtual bool loadGame();
	virtual bool saveGame();

	virtual void getGamePartProgress(bool *complete, int size) = 0;
	virtual void setPartComplete(const char *part) = 0;

	virtual void renameOldSavefiles() { }
};

class SaveLoad_ns : public SaveLoad {
	Parallaction_ns *_vm;

protected:
	void renameOldSavefiles();
	virtual void doLoadGame(uint16 slot);
	virtual void doSaveGame(uint16 slot, const char* name);

public:
	SaveLoad_ns(Parallaction_ns *vm, Common::SaveFileManager *saveFileMan) : SaveLoad(saveFileMan, "nippon"), _vm(vm) { }

	virtual bool saveGame();

	virtual void getGamePartProgress(bool *complete, int size);
	virtual void setPartComplete(const char *part);
};

class SaveLoad_br : public SaveLoad {
	Parallaction_br *_vm;
	virtual void doLoadGame(uint16 slot);
	virtual void doSaveGame(uint16 slot, const char* name);

public:
	SaveLoad_br(Parallaction_br *vm, Common::SaveFileManager *saveFileMan) : SaveLoad(saveFileMan, "bra"), _vm(vm) { }

	virtual void getGamePartProgress(bool *complete, int size);
	virtual void setPartComplete(const char *part);
};

} // namespace Parallaction

#endif
