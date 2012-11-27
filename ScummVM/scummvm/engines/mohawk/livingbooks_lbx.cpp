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

#include "engines/mohawk/livingbooks.h"
#include "engines/mohawk/livingbooks_lbx.h"

namespace Mohawk {

class LBXDataFile : public LBXObject {
public:
	LBXDataFile(MohawkEngine_LivingBooks *vm);
	~LBXDataFile();

	bool call(uint callId, const Common::Array<LBValue> &params, LBValue &result);

protected:
	Common::ConfigFile _dataFile;
	Common::String _curSection;

	void open(const Common::String &filename);
	bool sectionExists(const Common::String &section);
};

LBXDataFile::LBXDataFile(MohawkEngine_LivingBooks *vm) : LBXObject(vm) {
}

LBXDataFile::~LBXDataFile() {
}

enum {
	kLBXDataFileOpen = 1,
	kLBXDataFileGetSectionList = 4,
	kLBXDataFileSetCurSection = 5,
	kLBXDataFileLoadCurSectionVars = 8,
	kLBXDataFileDeleteCurSection = 10,
	kLBXDataFileSectionExists = 14
};

bool LBXDataFile::call(uint callId, const Common::Array<LBValue> &params, LBValue &result) {
	switch (callId) {
	case kLBXDataFileOpen:
		if (params.size() != 1)
			error("incorrect number of parameters (%d) to LBXDataFile::open", params.size());

		open(params[0].toString());
		return false;

	case kLBXDataFileGetSectionList:
		{
		Common::SharedPtr<LBList> list = Common::SharedPtr<LBList>(new LBList);
		Common::ConfigFile::SectionList sections = _dataFile.getSections();
		for (Common::List<Common::ConfigFile::Section>::const_iterator i = sections.begin(); i != sections.end(); ++i)
			list->array.push_back(LBValue(i->name));
		result = LBValue(list);
		}
		return true;

	case kLBXDataFileSetCurSection:
		if (params.size() != 1)
			error("incorrect number of parameters (%d) to LBXDataFile::setCurSection", params.size());

		_curSection = params[0].toString();
		return false;

	case kLBXDataFileLoadCurSectionVars:
		if (params.size() != 0)
			error("incorrect number of parameters (%d) to LBXDataFile::loadCurSectionVars", params.size());

		{
		const Common::ConfigFile::SectionKeyList globals = _dataFile.getKeys(_curSection);
		for (Common::ConfigFile::SectionKeyList::const_iterator i = globals.begin(); i != globals.end(); i++) {
			Common::String command = Common::String::format("%s = %s", i->key.c_str(), i->value.c_str());
			LBCode tempCode(_vm, 0);
			uint offset = tempCode.parseCode(command);
			tempCode.runCode(NULL, offset);
		}
		}
		return false;

	case kLBXDataFileDeleteCurSection:
		if (params.size() != 0)
			error("incorrect number of parameters (%d) to LBXDataFile::deleteCurSection", params.size());

		_dataFile.removeSection(_curSection);
		return false;

	case kLBXDataFileSectionExists:
		if (params.size() != 1)
			error("incorrect number of parameters (%d) to LBXDataFile::sectionExists", params.size());
		if (_dataFile.hasSection(params[0].toString()))
			result = 1;
		else
			result = 0;
		return true;

	default:
		error("LBXDataFile call %d is unknown", callId);
	}
}

void LBXDataFile::open(const Common::String &filename) {
	_dataFile.clear();

	if (_dataFile.loadFromFile(filename))
		return;

	// FIXME: try savegames

	error("LBXDataFile::open: couldn't open '%s'", filename.c_str());
}

Common::SharedPtr<LBXObject> createLBXObject(MohawkEngine_LivingBooks *vm, uint16 type) {
	switch (type) {
	case 1001:
		return Common::SharedPtr<LBXObject>(new LBXDataFile(vm));

	default:
		error("unknown LBX object type %d", type);
	}
}

} // End of namespace Mohawk
