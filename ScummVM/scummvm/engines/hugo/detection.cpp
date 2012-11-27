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

#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/savefile.h"
#include "common/textconsole.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "hugo/hugo.h"

namespace Hugo {

struct HugoGameDescription {
	ADGameDescription desc;
	GameType gameType;
};

uint32 HugoEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

const char *HugoEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}


static const PlainGameDescriptor hugoGames[] = {
	// Games
	{"hugo1", "Hugo 1: Hugo's House of Horrors"},
	{"hugo2", "Hugo 2: Whodunit?"},
	{"hugo3", "Hugo 3: Jungle of Doom"},
	{0, 0}
};

static const HugoGameDescription gameDescriptions[] = {

	// Hugo1 DOS
	{
		{
			"hugo1", 0,
			AD_ENTRY1s("house.art", "c9403b2fe539185c9fd569b6cc4ff5ca", 14811),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		kGameTypeHugo1
	},
	// Hugo1 Windows
	{
		{
			"hugo1", 0,
			AD_ENTRY1s("objects.dat", "3ba0f108f7690a05a34c56a02fbe644a", 126488),
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_PACKED,
			GUIO0()
		},
		kGameTypeHugo1
	},
	// Hugo2 DOS
	{
		{
			"hugo2", 0,
			AD_ENTRY1s("objects.dat", "88a718cc0ff2b3b25d49aaaa69d6d52c", 155240),
			Common::EN_ANY,
			Common::kPlatformPC,
			GF_PACKED,
			GUIO0()
		},
		kGameTypeHugo2
	},
	// Hugo2 Windows
	{
		{
			"hugo2", 0,
			AD_ENTRY1s("objects.dat", "5df4ffc851e66a544c0e95e4e084a806", 158480),
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_PACKED,
			GUIO0()
		},
		kGameTypeHugo2
	},
	// Hugo3 DOS
	{
		{
			"hugo3", 0,
			AD_ENTRY1s("objects.dat", "bb1b061538a445f2eb99b682c0f506cc", 136419),
			Common::EN_ANY,
			Common::kPlatformPC,
			GF_PACKED,
			GUIO0()
		},
		kGameTypeHugo3
	},
	// Hugo3 Windows
	{
		{
			"hugo3", 0,
			AD_ENTRY1s("objects.dat", "c9a8af7aa14cc907434eecee3ddd06d3", 136638),
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_PACKED,
			GUIO0()
		},
		kGameTypeHugo3
	},

	{AD_TABLE_END_MARKER, kGameTypeNone}
};

class HugoMetaEngine : public AdvancedMetaEngine {
public:
	HugoMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(HugoGameDescription), hugoGames) {
	}

	const char *getName() const {
		return "Hugo";
	}

	const char *getOriginalCopyright() const {
		return "Hugo Engine (C) 1989-1997 David P. Gray";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
	bool hasFeature(MetaEngineFeature f) const;

	int getMaximumSaveSlot() const;
	SaveStateList listSaves(const char *target) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	void removeSaveState(const char *target, int slot) const;
};

bool HugoMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd) {
		*engine = new HugoEngine(syst, (const HugoGameDescription *)gd);
		((HugoEngine *)*engine)->initGame((const HugoGameDescription *)gd);
	}
	return gd != 0;
}

bool HugoMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
	    (f == kSupportsListSaves) ||
	    (f == kSupportsLoadingDuringStartup) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSavesSupportCreationDate);
}

int HugoMetaEngine::getMaximumSaveSlot() const { return 99; }

SaveStateList HugoMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += "-??.SAV";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());   // Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	char slot[3];
	int slotNum = 0;
	for (Common::StringArray::const_iterator filename = filenames.begin(); filename != filenames.end(); ++filename) {
		slot[0] = filename->c_str()[filename->size() - 6];
		slot[1] = filename->c_str()[filename->size() - 5];
		slot[2] = '\0';
		// Obtain the last 2 digits of the filename (without extension), since they correspond to the save slot
		slotNum = atoi(slot);
		if (slotNum >= 0 && slotNum <= getMaximumSaveSlot()) {
			Common::InSaveFile *file = saveFileMan->openForLoading(*filename);
			if (file) {
				int saveVersion = file->readByte();

				if (saveVersion != kSavegameVersion) {
					warning("Savegame of incompatible version");
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

SaveStateDescriptor HugoMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s-%02d.SAV", target, slot);
	Common::InSaveFile *file = g_system->getSavefileManager()->openForLoading(fileName);

	if (file) {
		int saveVersion = file->readByte();

		if (saveVersion != kSavegameVersion) {
			warning("Savegame of incompatible version");
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

		// Slot 0 is used for the 'restart game' save in all Hugo games, thus
		// we prevent it from being deleted.
		desc.setDeletableFlag(slot != 0);
		desc.setWriteProtectedFlag(slot == 0);

		delete file;
		return desc;
	}
	return SaveStateDescriptor();
}

void HugoMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s-%02d.SAV", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}
} // End of namespace Hugo

#if PLUGIN_ENABLED_DYNAMIC(HUGO)
REGISTER_PLUGIN_DYNAMIC(HUGO, PLUGIN_TYPE_ENGINE, Hugo::HugoMetaEngine);
#else
REGISTER_PLUGIN_STATIC(HUGO, PLUGIN_TYPE_ENGINE, Hugo::HugoMetaEngine);
#endif

namespace Hugo {

void HugoEngine::initGame(const HugoGameDescription *gd) {
	_gameType = gd->gameType;
	_platform = gd->desc.platform;
	_packedFl = (getFeatures() & GF_PACKED);
	_gameVariant = _gameType - 1 + ((_platform == Common::kPlatformWindows) ? 0 : 3);
}

} // End of namespace Hugo
