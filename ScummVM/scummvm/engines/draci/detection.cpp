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

#include "draci/draci.h"
#include "draci/saveload.h"

#include "base/plugins.h"
#include "common/system.h"
#include "engines/advancedDetector.h"
#include "engines/metaengine.h"

static const PlainGameDescriptor draciGames[] = {
	{ "draci", "Draci Historie" },
	{ 0, 0 }
};

namespace Draci {

const ADGameDescription gameDescriptions[] = {
	{
		"draci",
		0,
		AD_ENTRY1s("INIT.DFW", "b890a5aeebaf16af39219cba2416b0a3", 906),
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"draci",
		0,
		AD_ENTRY1s("INIT.DFW", "9921c8f0045679a8f37eca8d41c5ec02", 906),
		Common::CZ_CZE,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"draci",
		0,
		AD_ENTRY1s("INIT.DFW", "76b9b78a8a8809a240acc395df4d0715", 906),
		Common::PL_POL,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"draci",
		0,
		AD_ENTRY1s("INIT.DFW", "9a7115b91cdea361bcaff3e046ac7ded", 906),
		Common::DE_DEU,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Draci

class DraciMetaEngine : public AdvancedMetaEngine {
public:
	DraciMetaEngine() : AdvancedMetaEngine(Draci::gameDescriptions, sizeof(ADGameDescription), draciGames) {
		_singleid = "draci";
	}

	virtual const char *getName() const {
		return "Draci";
	}

	virtual const char *getOriginalCopyright() const {
		return "Copyright (C) 1995 NoSense";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual int getMaximumSaveSlot() const { return 99; }
	virtual SaveStateList listSaves(const char *target) const;
	virtual void removeSaveState(const char *target, int slot) const;
	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool DraciMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsLoadingDuringStartup);
}

SaveStateList DraciMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern("draci.s??");

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				Draci::DraciSavegameHeader header;
				if (Draci::readSavegameHeader(in, header)) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.saveName));
					if (header.thumbnail) {
						header.thumbnail->free();
						delete header.thumbnail;
					}
				}
				delete in;
			}
		}
	}

	return saveList;
}

void DraciMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(Draci::DraciEngine::getSavegameFile(slot));
}

SaveStateDescriptor DraciMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(
		Draci::DraciEngine::getSavegameFile(slot));

	if (f) {
		Draci::DraciSavegameHeader header;
		Draci::readSavegameHeader(f, header);
		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(slot, header.saveName);
		desc.setThumbnail(header.thumbnail);

		int day = (header.date >> 24) & 0xFF;
		int month = (header.date >> 16) & 0xFF;
		int year = header.date & 0xFFFF;
		desc.setSaveDate(year, month, day);

		int hour = (header.time >> 8) & 0xFF;
		int minutes = header.time & 0xFF;
		desc.setSaveTime(hour, minutes);

		desc.setPlayTime(header.playtime * 1000);

		return desc;
	}

	return SaveStateDescriptor();
}

bool DraciMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Draci::DraciEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(DRACI)
	REGISTER_PLUGIN_DYNAMIC(DRACI, PLUGIN_TYPE_ENGINE, DraciMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DRACI, PLUGIN_TYPE_ENGINE, DraciMetaEngine);
#endif
