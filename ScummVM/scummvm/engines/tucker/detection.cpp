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
#include "common/fs.h"

#include "base/plugins.h"

#include "tucker/tucker.h"

static const PlainGameDescriptor tuckerGames[] = {
	{ "tucker", "Bud Tucker in Double Trouble" },
	{ 0, 0 }
};

static const ADGameDescription tuckerGameDescriptions[] = {
	{
		"tucker",
		"",
		AD_ENTRY1s("infobar.txt", "f1e42a95972643462b9c3c2ea79d6683", 543),
		Common::FR_FRA,
		Common::kPlatformPC,
		Tucker::kGameFlagNoSubtitles,
		GUIO0()
	},
	{
		"tucker",
		"",
		AD_ENTRY1s("infobar.txt", "9c1ddeafc5283b90d1a284bd0924831c", 462),
		Common::EN_ANY,
		Common::kPlatformPC,
		Tucker::kGameFlagEncodedData,
		GUIO0()
	},
	{
		"tucker",
		"",
		AD_ENTRY1s("infobar.txt", "1b3ea79d8528ea3c7df83dd0ed345e37", 525),
		Common::ES_ESP,
		Common::kPlatformPC,
		Tucker::kGameFlagEncodedData,
		GUIO0()
	},
	{
		"tucker",
		"",
		AD_ENTRY1s("infobrgr.txt", "4df9eb65722418d1a1723508115b146c", 552),
		Common::DE_DEU,
		Common::kPlatformPC,
		Tucker::kGameFlagEncodedData,
		GUIO0()
	},
	{
		"tucker",
		"",
		AD_ENTRY1s("infobar.txt", "5f85285bbc23ce57cbc164021ee1f23c", 525),
		Common::PL_POL,
		Common::kPlatformPC,
		0,
		GUIO0()
	},
	{
		"tucker",
		"",
		AD_ENTRY1s("infobar.txt", "e548994877ff31ca304f6352ce022a8e", 497),
		Common::CZ_CZE,
		Common::kPlatformPC,
		Tucker::kGameFlagEncodedData,
		GUIO0()
	},
	{
		"tucker",
		"Demo",
		AD_ENTRY1s("infobar.txt", "010b055de42097b140d5bcb6e95a5c7c", 203),
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_DEMO | Tucker::kGameFlagDemo,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

static const ADGameDescription tuckerDemoGameDescription = {
	"tucker",
	"Non-Interactive Demo",
	AD_ENTRY1(0, 0),
	Common::EN_ANY,
	Common::kPlatformPC,
	ADGF_DEMO | Tucker::kGameFlagDemo | Tucker::kGameFlagIntroOnly,
	GUIO0()
};

class TuckerMetaEngine : public AdvancedMetaEngine {
public:
	TuckerMetaEngine() : AdvancedMetaEngine(tuckerGameDescriptions, sizeof(ADGameDescription), tuckerGames) {
		_md5Bytes = 512;
		_singleid = "tucker";
	}

	virtual const char *getName() const {
		return "Tucker";
	}

	virtual const char *getOriginalCopyright() const {
		return "Bud Tucker in Double Trouble (C) Merit Studios";
	}

	virtual bool hasFeature(MetaEngineFeature f) const {
		switch (f) {
		case kSupportsListSaves:
		case kSupportsLoadingDuringStartup:
		case kSupportsDeleteSave:
			return true;
		default:
			return false;
		}
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
		if (desc) {
			*engine = new Tucker::TuckerEngine(syst, desc->language, desc->flags);
		}
		return desc != 0;
	}

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
		for (Common::FSList::const_iterator d = fslist.begin(); d != fslist.end(); ++d) {
			Common::FSList audiofslist;
			if (d->isDirectory() && d->getName().equalsIgnoreCase("audio") && d->getChildren(audiofslist, Common::FSNode::kListFilesOnly)) {
				for (Common::FSList::const_iterator f = audiofslist.begin(); f != audiofslist.end(); ++f) {
					if (!f->isDirectory() && f->getName().equalsIgnoreCase("demorolc.raw")) {
						return &tuckerDemoGameDescription;
					}
				}
			}
		}
		return 0;
	}

	virtual SaveStateList listSaves(const char *target) const {
		Common::String pattern = Tucker::generateGameStateFileName(target, 0, true);
		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
		bool slotsTable[Tucker::kLastSaveSlot + 1];
		memset(slotsTable, 0, sizeof(slotsTable));
		SaveStateList saveList;
		for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
			int slot;
			const char *ext = strrchr(file->c_str(), '.');
			if (ext && (slot = atoi(ext + 1)) >= 0 && slot <= Tucker::kLastSaveSlot) {
				Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(*file);
				if (in) {
					slotsTable[slot] = true;
					delete in;
				}
			}
		}
		for (int slot = 0; slot <= Tucker::kLastSaveSlot; ++slot) {
			if (slotsTable[slot]) {
				Common::String description = Common::String::format("savegm.%02d", slot);
				saveList.push_back(SaveStateDescriptor(slot, description));
			}
		}
		return saveList;
	}

	virtual int getMaximumSaveSlot() const {
		return Tucker::kLastSaveSlot;
	}

	virtual void removeSaveState(const char *target, int slot) const {
		Common::String filename = Tucker::generateGameStateFileName(target, slot);
		g_system->getSavefileManager()->removeSavefile(filename);
	}
};

#if PLUGIN_ENABLED_DYNAMIC(TUCKER)
	REGISTER_PLUGIN_DYNAMIC(TUCKER, PLUGIN_TYPE_ENGINE, TuckerMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TUCKER, PLUGIN_TYPE_ENGINE, TuckerMetaEngine);
#endif
