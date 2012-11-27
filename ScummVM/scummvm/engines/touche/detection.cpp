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
#include "common/translation.h"

#include "base/plugins.h"

#include "touche/touche.h"

static const PlainGameDescriptor toucheGames[] = {
	{ "touche", "Touche: The Adventures of the Fifth Musketeer" },
	{ 0, 0 }
};

namespace Touche {

static const ADGameDescription gameDescriptions[] = {
	{ // retail version
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "2af0177f8887e3430f345e6b4d8b1414", 26350211),
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{ // retail version - tracker item #1601818
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "95967f0b51d2e813e99ca00325098340", 26350190),
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{ // retail version
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "1caa20bb4d4fc2ce8eb867b6610082b3", 26558232),
		Common::FR_FRA,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{ // retail version - tracker item #1598643
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "be2ae6454b3325e410946f2322547cd4", 26625537),
		Common::DE_DEU,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{ // retail version - tracker item #1681643
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "64e95ba1decf5a5a60f8fa1840f40c62", 26529523),
		Common::ES_ESP,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{ // fan-made translation (http://www.iagtg.net/) - tracker item #1602360
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "1f442331d4b327c3488a9f6ffe9bdd25", 26367792),
		Common::IT_ITA,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{ // retail version - tracker item #1800500
		"touche",
		"",
		AD_ENTRY1s("touche.dat", "42d19a0bef65465109020440a9caa228", 26487370),
		Common::PL_POL,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	{ // demo version
		"touche",
		"Demo",
		AD_ENTRY1s("touche.dat", "ddaed436445b2e77294ed19e8ae4aa2c", 8720683),
		Common::EN_ANY,
		Common::kPlatformPC,
		ADGF_DEMO,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

static const ADFileBasedFallback fileBasedFallback[] = {
	{ &gameDescriptions[0], { "touche.dat", 0 } }, // default to english version
	{ 0, { 0 } }
};

} // End of namespace Touche

static const char *directoryGlobs[] = {
	"database",
	0
};

class ToucheMetaEngine : public AdvancedMetaEngine {
public:
	ToucheMetaEngine() : AdvancedMetaEngine(Touche::gameDescriptions, sizeof(ADGameDescription), toucheGames) {
		_md5Bytes = 4096;
		_singleid = "touche";
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
		ADFilePropertiesMap filesProps;

		const ADGameDescription *matchedDesc = detectGameFilebased(allFiles, fslist, Touche::fileBasedFallback, &filesProps);
		if (!matchedDesc)
			return 0;

		reportUnknown(fslist.begin()->getParent(), filesProps);
		return matchedDesc;
	}

	virtual const char *getName() const {
		return "Touche";
	}

	virtual const char *getOriginalCopyright() const {
		return "Touche: The Adventures of the 5th Musketeer (C) Clipper Software";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool ToucheMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

bool Touche::ToucheEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsSubtitleOptions);
}

bool ToucheMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (desc) {
		*engine = new Touche::ToucheEngine(syst, desc->language);
	}
	return desc != 0;
}

SaveStateList ToucheMetaEngine::listSaves(const char *target) const {
	Common::String pattern = Touche::generateGameStateFileName(target, 0, true);
	Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles(pattern);
	bool slotsTable[Touche::kMaxSaveStates];
	memset(slotsTable, 0, sizeof(slotsTable));
	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		int slot = Touche::getGameStateFileSlot(file->c_str());
		if (slot >= 0 && slot < Touche::kMaxSaveStates) {
			slotsTable[slot] = true;
		}
	}
	for (int slot = 0; slot < Touche::kMaxSaveStates; ++slot) {
		if (slotsTable[slot]) {
			Common::String file = Touche::generateGameStateFileName(target, slot);
			Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(file);
			if (in) {
				char description[64];
				Touche::readGameStateDescription(in, description, sizeof(description) - 1);
				if (description[0]) {
					saveList.push_back(SaveStateDescriptor(slot, description));
				}
				delete in;
			}
		}
	}
	return saveList;
}

int ToucheMetaEngine::getMaximumSaveSlot() const {
	return Touche::kMaxSaveStates - 1;
}

void ToucheMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = Touche::generateGameStateFileName(target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

#if PLUGIN_ENABLED_DYNAMIC(TOUCHE)
	REGISTER_PLUGIN_DYNAMIC(TOUCHE, PLUGIN_TYPE_ENGINE, ToucheMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(TOUCHE, PLUGIN_TYPE_ENGINE, ToucheMetaEngine);
#endif
