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
#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "base/plugins.h"
#include "graphics/thumbnail.h"
#include "toon/toon.h"

static const PlainGameDescriptor toonGames[] = {
	{ "toon", "Toonstruck" },
	{ 0, 0 }
};

namespace Toon {

static const ADGameDescription gameDescriptions[] = {
	{
		"toon", "",
		{
			{"local.pak", 0, "3290209ef9bc92692108dd2f45df0736", 3237611},
			{"arcaddbl.svl", 0, "c418478cd2833c7c983799f948af41ac", 7844688},
			{"study.svl", 0, "281efa3f33f6712c0f641a605f4d40fd", 2511090},
			AD_LISTEND
		},
		Common::EN_ANY, Common::kPlatformPC, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "517132c3575b38806d1e7b6f59848072", 3224044},
			{"arcaddbl.svl", 0, "ff74008827b62fbef1f46f104c438e44", 9699256},
			{"study.svl", 0, "df056b94ea83f1ed92a539cf636053ab", 2542668},
			AD_LISTEND
		},
		Common::FR_FRA, Common::kPlatformPC, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "bf5da4c03f78ffbd643f12122319366e", 3250841},
			{"arcaddbl.svl", 0, "7a0d74f4d66d1c722b946abbeb0834ef", 9122249},
			{"study.svl", 0, "72fe96a9e10967d3138e918295babc42", 2910283},
			AD_LISTEND
		},
		Common::DE_DEU, Common::kPlatformPC, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "e8645168a247e2abdbfc2f9fa9d1c0fa", 3232222},
			{"arcaddbl.svl", 0, "7893ac4cc78d51356baa058bbee7aa28", 8275016},
			{"study.svl", 0, "b6b1ee2d9d94d53d305856039ab7bde7", 2634620},
			AD_LISTEND
		},
		Common::ES_ESP, Common::kPlatformPC, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "48ec60709bebbdeff791d55ee18ec910", 3417846},
			{"arcaddbl.svl", 0, "1d1b96e317e03ffd3874a8ebe59556f3", 6246232},
			{"study.svl", 0, "d4aff126ee27be3c3d25e2996369d7cb", 2324368},
		},
		Common::RU_RUS, Common::kPlatformPC, ADGF_NO_FLAGS, GUIO0()
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "bf5da4c03f78ffbd643f12122319366e", 3250841},
			{"wacexdbl.emc", 0, "cfbc2156a31b294b038204888407ebc8", 6974},
			{"generic.svl", 0, "5eb99850ada22f0b8cf6392262d4dd07", 9404599},
			AD_LISTEND
		},
		Common::DE_DEU, Common::kPlatformPC, ADGF_DEMO, GUIO0()
	},
	{
		"toon", "",
		{
			{"local.pak", 0, "8ef3368078b9ea70b305c04db826feea", 2680573},
			{"generic.svl", 0, "5c42724bb93b360dca7044d6b7ef26e5", 7739319},
			AD_LISTEND
		},
		Common::EN_ANY, Common::kPlatformPC, ADGF_DEMO, GUIO0()
	},

	AD_TABLE_END_MARKER
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{ &gameDescriptions[0], { "local.pak", "arcaddbl.svl", "study.svl", 0 } }, // default to english version
	{ 0, { 0 } }
};

} // End of namespace Toon

static const char * const directoryGlobs[] = {
	"misc",
	"act1",
	"arcaddbl",
	"act2",
	"study",
	0
};

class ToonMetaEngine : public AdvancedMetaEngine {
public:
	ToonMetaEngine() : AdvancedMetaEngine(Toon::gameDescriptions, sizeof(ADGameDescription), toonGames) {
		_singleid = "toon";
		_maxScanDepth = 3;
		_directoryGlobs = directoryGlobs;
	}

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
		return detectGameFilebased(allFiles, fslist, Toon::fileBasedFallback);
	}

	virtual const char *getName() const {
		return "Toon";
	}

	virtual const char *getOriginalCopyright() const {
		return "Toonstruck (C) 1996 Virgin Interactive";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual int getMaximumSaveSlot() const;
	virtual SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool ToonMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate);
}

void ToonMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

int ToonMetaEngine::getMaximumSaveSlot() const { return 99; }

SaveStateList ToonMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	int slotNum = 0;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(filename->c_str() + filename->size() - 3);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				int32 version = file->readSint32BE();
				if (version != TOON_SAVEGAME_VERSION) {
					delete file;
					continue;
				}

				// read name
				uint16 nameSize = file->readUint16BE();
				if (nameSize >= 255) {
					delete file;
					continue;
				}
				char name[256];
				file->read(name, nameSize);
				name[nameSize] = 0;

				saveList.push_back(SaveStateDescriptor(slotNum, name));
				delete file;
			}
		}
	}

	return saveList;
}

SaveStateDescriptor ToonMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (file) {

		int32 version = file->readSint32BE();
		if (version != TOON_SAVEGAME_VERSION) {
			delete file;
			return SaveStateDescriptor();
		}

		uint32 saveNameLength = file->readUint16BE();
		char saveName[256];
		file->read(saveName, saveNameLength);
		saveName[saveNameLength] = 0;

		SaveStateDescriptor desc(slot, saveName);

		Graphics::Surface *const thumbnail = Graphics::loadThumbnail(*file);
		desc.setThumbnail(thumbnail);

		uint32 saveDate = file->readUint32BE();
		uint16 saveTime = file->readUint16BE();

		int day = (saveDate >> 24) & 0xFF;
		int month = (saveDate >> 16) & 0xFF;
		int year = saveDate & 0xFFFF;

		desc.setSaveDate(year, month, day);

		int hour = (saveTime >> 8) & 0xFF;
		int minutes = saveTime & 0xFF;

		desc.setSaveTime(hour, minutes);

		delete file;
		return desc;
	}

	return SaveStateDescriptor();
}

bool ToonMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Toon::ToonEngine(syst, desc);
	}
	return desc != 0;
}

#if PLUGIN_ENABLED_DYNAMIC(TOON)
	REGISTER_PLUGIN_DYNAMIC(TOON, PLUGIN_TYPE_ENGINE, ToonMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TOON, PLUGIN_TYPE_ENGINE, ToonMetaEngine);
#endif
