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

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "mohawk/livingbooks.h"

#ifdef ENABLE_CSTIME
#include "mohawk/cstime.h"
#endif

#ifdef ENABLE_MYST
#include "mohawk/myst.h"
#endif

#ifdef ENABLE_RIVEN
#include "mohawk/riven.h"
#endif

namespace Mohawk {

struct MohawkGameDescription {
	ADGameDescription desc;

	uint8 gameType;
	uint32 features;
	const char *appName;
};

const char* MohawkEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}

uint32 MohawkEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform MohawkEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

const char *MohawkEngine::getAppName() const {
	return _gameDescription->appName;
}

uint8 MohawkEngine::getGameType() const {
	return _gameDescription->gameType;
}

Common::String MohawkEngine_LivingBooks::getBookInfoFileName() const {
	return _gameDescription->desc.filesDescriptions[0].fileName;
}

Common::Language MohawkEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool MohawkEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

#ifdef ENABLE_MYST

bool MohawkEngine_Myst::hasFeature(EngineFeature f) const {
	return
		MohawkEngine::hasFeature(f)
		|| (f == kSupportsLoadingDuringRuntime)
		|| (f == kSupportsSavingDuringRuntime);
}

#endif

#ifdef ENABLE_RIVEN

bool MohawkEngine_Riven::hasFeature(EngineFeature f) const {
	return
		MohawkEngine::hasFeature(f)
		|| (f == kSupportsLoadingDuringRuntime)
		|| (f == kSupportsSavingDuringRuntime);
}

#endif

} // End of Namespace Mohawk

static const PlainGameDescriptor mohawkGames[] = {
	{"mohawk", "Mohawk Game"},
	{"myst", "Myst"},
	{"MakingOfMyst", "The Making of Myst"},
	{"riven", "Riven: The Sequel to Myst"},
	{"zoombini", "Logical Journey of the Zoombinis"},
	{"cstime", "Where in Time is Carmen Sandiego?"},
	{"csworld", "Where in the World is Carmen Sandiego?"},
	{"csamtrak", "Where in America is Carmen Sandiego? (The Great Amtrak Train Adventure)"},
	{"carmentq", "Carmen Sandiego's ThinkQuick Challenge"},
	{"carmentqc", "Carmen Sandiego's ThinkQuick Challenge Custom Question Creator"},
	{"maggiesfa", "Maggie's Farmyard Adventure"},
	{"jamesmath", "James Discovers/Explores Math"},
	{"treehouse", "The Treehouse"},
	{"greeneggs", "Green Eggs and Ham"},
	{"seussabc", "Dr Seuss's ABC"},
	{"1stdegree", "In the 1st Degree"},
	{"csusa", "Where in the USA is Carmen Sandiego?"},
	{"tortoise", "Aesop's Fables: The Tortoise and the Hare"},
	{"arthur", "Arthur's Teacher Trouble"},
	{"grandma", "Just Grandma and Me"},
	{"ruff", "Ruff's Bone"},
	{"newkid", "The New Kid on the Block"},
	{"arthurrace", "Arthur's Reading Race"},
	{"arthurbday", "Arthur's Birthday"},
	{"lilmonster", "Little Monster at School"},
	{"catinthehat", "The Cat in the Hat"},
	{"rugrats", "Rugrats Adventure Game"},
	{"lbsampler", "Living Books Sampler"},
	{"bearfight", "The Berenstain Bears Get in a Fight"},
	{"beardark", "The Berenstain Bears In The Dark"},
	{"arthurcomp", "Arthur's Computer Adventure"},
	{"harryhh","Harry and the Haunted House"},
	{"stellaluna", "Stellaluna"},
	{"sheila", "Sheila Rae, the Brave"},
	{"rugratsps", "Rugrats Print Shop" },
	{0, 0}
};

#include "mohawk/detection_tables.h"

static const char *directoryGlobs[] = {
	"all",
	"assets1",
	"data",
	"program",
	"95instal",
	"Rugrats Adventure Game",
	0
};

class MohawkMetaEngine : public AdvancedMetaEngine {
public:
	MohawkMetaEngine() : AdvancedMetaEngine(Mohawk::gameDescriptions, sizeof(Mohawk::MohawkGameDescription), mohawkGames) {
		_singleid = "mohawk";
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	virtual const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
		return detectGameFilebased(allFiles, fslist, Mohawk::fileBased);
	}

	virtual const char *getName() const {
		return "Mohawk";
	}

	virtual const char *getOriginalCopyright() const {
		return "Myst and Riven (C) Cyan Worlds\nMohawk OS (C) Ubisoft";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const { return 999; }
	virtual void removeSaveState(const char *target, int slot) const;
};

bool MohawkMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves)
		|| (f == kSupportsLoadingDuringStartup)
		|| (f == kSupportsDeleteSave);
}

SaveStateList MohawkMetaEngine::listSaves(const char *target) const {
	Common::StringArray filenames;
	SaveStateList saveList;

	// Loading games is only supported in Myst/Riven currently.
	if (strstr(target, "myst")) {
		filenames = g_system->getSavefileManager()->listSavefiles("*.mys");

		for (uint32 i = 0; i < filenames.size(); i++)
			saveList.push_back(SaveStateDescriptor(i, filenames[i]));
	} else if (strstr(target, "riven")) {
		filenames = g_system->getSavefileManager()->listSavefiles("*.rvn");

		for (uint32 i = 0; i < filenames.size(); i++)
			saveList.push_back(SaveStateDescriptor(i, filenames[i]));
	}

	return saveList;
}

void MohawkMetaEngine::removeSaveState(const char *target, int slot) const {
	// Removing saved games is only supported in Myst/Riven currently.
	if (strstr(target, "myst")) {
		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles("*.mys");
		g_system->getSavefileManager()->removeSavefile(filenames[slot].c_str());
	} else if (strstr(target, "riven")) {
		Common::StringArray filenames = g_system->getSavefileManager()->listSavefiles("*.rvn");
		g_system->getSavefileManager()->removeSavefile(filenames[slot].c_str());
	}
}

bool MohawkMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Mohawk::MohawkGameDescription *gd = (const Mohawk::MohawkGameDescription *)desc;

	if (gd) {
		switch (gd->gameType) {
		case Mohawk::GType_MYST:
		case Mohawk::GType_MAKINGOF:
#ifdef ENABLE_MYST
			*engine = new Mohawk::MohawkEngine_Myst(syst, gd);
			break;
#else
			warning("Myst support not compiled in");
			return false;
#endif
		case Mohawk::GType_RIVEN:
#ifdef ENABLE_RIVEN
			*engine = new Mohawk::MohawkEngine_Riven(syst, gd);
			break;
#else
			warning("Riven support not compiled in");
			return false;
#endif
		case Mohawk::GType_LIVINGBOOKSV1:
		case Mohawk::GType_LIVINGBOOKSV2:
		case Mohawk::GType_LIVINGBOOKSV3:
		case Mohawk::GType_LIVINGBOOKSV4:
		case Mohawk::GType_LIVINGBOOKSV5:
			*engine = new Mohawk::MohawkEngine_LivingBooks(syst, gd);
			break;
		case Mohawk::GType_CSTIME:
#ifdef ENABLE_CSTIME
			*engine = new Mohawk::MohawkEngine_CSTime(syst, gd);
			break;
#else
			warning("CSTime support not compiled in");
			return false;
#endif
		case Mohawk::GType_ZOOMBINI:
		case Mohawk::GType_CSWORLD:
		case Mohawk::GType_CSAMTRAK:
		case Mohawk::GType_JAMESMATH:
		case Mohawk::GType_TREEHOUSE:
		case Mohawk::GType_1STDEGREE:
		case Mohawk::GType_CSUSA:
			warning("Unsupported Mohawk Engine");
			return false;
		default:
			error("Unknown Mohawk Engine");
		}
	}

	return (gd != 0);
}

#if PLUGIN_ENABLED_DYNAMIC(MOHAWK)
	REGISTER_PLUGIN_DYNAMIC(MOHAWK, PLUGIN_TYPE_ENGINE, MohawkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MOHAWK, PLUGIN_TYPE_ENGINE, MohawkMetaEngine);
#endif
