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
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/system.h"

#include "toltecs/toltecs.h"


namespace Toltecs {

struct ToltecsGameDescription {
	ADGameDescription desc;
};

uint32 ToltecsEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language ToltecsEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

}

static const PlainGameDescriptor toltecsGames[] = {
	{"toltecs", "3 Skulls of the Toltecs"},
	{0, 0}
};


namespace Toltecs {

static const ToltecsGameDescription gameDescriptions[] = {

	{
		// 3 Skulls of the Toltecs English version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "05472037e9cfde146e953c434e74f0f4", 337643527),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs Russian version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "ba1742d3193b68ceb9434e2ab7a09a9b", 391462783),
			Common::RU_RUS,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs German version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "1a3292bad8e0bb5701800c73531dd75e", 345176617),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs German Demo version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "1c85e82712d24f1d5c1ea2a66ddd75c2", 47730038),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs French version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "4fb845635cbdac732453fe23be350df9", 327269545),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs Spanish version
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "479f468beccc1b0ce5873ec523d1380e", 308391018),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{
		// 3 Skulls of the Toltecs Hungarian version
		// From bug #3440641
		{
			"toltecs",
			0,
			AD_ENTRY1s("WESTERN", "69a5572e75409d8c6230b787faa353af", 337647960),
			Common::HU_HUN,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NONE)
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Toltecs

class ToltecsMetaEngine : public AdvancedMetaEngine {
public:
	ToltecsMetaEngine() : AdvancedMetaEngine(Toltecs::gameDescriptions, sizeof(Toltecs::ToltecsGameDescription), toltecsGames) {
		_singleid = "toltecs";
	}

	virtual const char *getName() const {
		return "Toltecs Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Toltecs Engine Revistronic (C) 1996";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool ToltecsMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

bool Toltecs::ToltecsEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool ToltecsMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Toltecs::ToltecsGameDescription *gd = (const Toltecs::ToltecsGameDescription *)desc;
	if (gd) {
		*engine = new Toltecs::ToltecsEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList ToltecsMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Toltecs::ToltecsEngine::SaveHeader header;
	Common::String pattern = target;
	pattern += ".???";

	Common::StringArray filenames;
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); file++) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(file->c_str());
			if (in) {
				if (Toltecs::ToltecsEngine::readSaveHeader(in, false, header) == Toltecs::ToltecsEngine::kRSHENoError) {
					saveList.push_back(SaveStateDescriptor(slotNum, header.description));
				}
				delete in;
			}
		}
	}

	return saveList;
}

int ToltecsMetaEngine::getMaximumSaveSlot() const {
	return 999;
}

void ToltecsMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::String filename = Toltecs::ToltecsEngine::getSavegameFilename(target, slot);

	saveFileMan->removeSavefile(filename.c_str());

	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";
	filenames = saveFileMan->listSavefiles(pattern.c_str());
	Common::sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		// Rename every slot greater than the deleted slot,
		if (slotNum > slot) {
			saveFileMan->renameSavefile(file->c_str(), filename.c_str());
			filename = Toltecs::ToltecsEngine::getSavegameFilename(target, ++slot);
		}
	}
}

SaveStateDescriptor ToltecsMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String filename = Toltecs::ToltecsEngine::getSavegameFilename(target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(filename.c_str());

	if (in) {
		Toltecs::ToltecsEngine::SaveHeader header;
		Toltecs::ToltecsEngine::kReadSaveHeaderError error;

		error = Toltecs::ToltecsEngine::readSaveHeader(in, true, header);
		delete in;

		if (error == Toltecs::ToltecsEngine::kRSHENoError) {
			SaveStateDescriptor desc(slot, header.description);

			desc.setThumbnail(header.thumbnail);

			if (header.version > 0) {
				int day = (header.saveDate >> 24) & 0xFF;
				int month = (header.saveDate >> 16) & 0xFF;
				int year = header.saveDate & 0xFFFF;

				desc.setSaveDate(year, month, day);

				int hour = (header.saveTime >> 16) & 0xFF;
				int minutes = (header.saveTime >> 8) & 0xFF;

				desc.setSaveTime(hour, minutes);

				desc.setPlayTime(header.playTime * 1000);
			}

			return desc;
		}
	}

	return SaveStateDescriptor();
}	// End of namespace Toltecs

#if PLUGIN_ENABLED_DYNAMIC(TOLTECS)
	REGISTER_PLUGIN_DYNAMIC(TOLTECS, PLUGIN_TYPE_ENGINE, ToltecsMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TOLTECS, PLUGIN_TYPE_ENGINE, ToltecsMetaEngine);
#endif
