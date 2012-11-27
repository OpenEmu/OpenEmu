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
#include "common/savefile.h"
#include "common/system.h"
#include "engines/advancedDetector.h"

#include "cruise/cruise.h"
#include "cruise/saveload.h"

namespace Cruise {

struct CRUISEGameDescription {
	ADGameDescription desc;

	int gameType;
	uint32 features;
};

const char *CruiseEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}

int CruiseEngine::getGameType() const {
	return _gameDescription->gameType;
}
uint32 CruiseEngine::getFeatures() const {
	return _gameDescription->features;
}
Common::Language CruiseEngine::getLanguage() const {
	return _gameDescription->desc.language;
}
Common::Platform CruiseEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

}

static const PlainGameDescriptor cruiseGames[] = {
	{"cruise", "Cinematique evo.2 engine game"},
	{"cruise", "Cruise for a Corpse"},
	{0, 0}
};

namespace Cruise {

static const CRUISEGameDescription gameDescriptions[] = {
	{
		{
			"cruise",
			"16 colors",
			AD_ENTRY1("D1", "cd29a4cd9162076e9a18495fe56a48f3"),
			Common::EN_GRB,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"16 colors",
			AD_ENTRY1("D1", "41a7a4d426dbd048eb369cfee4bb2717"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "a90d2b9ead6b4d812cd14268672cf178"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "e258865807ea31b2d523340e6f0a606b"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"16 colors",
			AD_ENTRY1("D1", "287d2ec1799e2f881dee23c70be96e81"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "f2a26522d49983c4ae32bcccbb801b02"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			0,
			AD_ENTRY1("D1", "70f42a21cc257b01d58667853335f4f1"),
			Common::DE_DEU,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{ // Amiga English US GOLD edition.
		{
			"cruise",
			0,
			AD_ENTRY1("D1", "de084e9d2c6e4b2cc14803bf849eda3e"),
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{ // Amiga Italian US GOLD edition.
		{
			"cruise",
			0,
			AD_ENTRY1("D1", "a0011075413b7335e003e8e3c9cf51b9"),
			Common::IT_ITA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{ // AtariST English KixxXL edition.
		{
			"cruise",
			0,
			AD_ENTRY1("D1", "be78614d5fa34bdb68bb03a2a6130280"),
			Common::EN_ANY,
			Common::kPlatformAtariST,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "e19a4ab2e24a69087e4ea994a5506231"),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{
		{
			"cruise",
			"256 colors",
			AD_ENTRY1("D1", "9a302ada55600d96061fda1d63a6ccda"),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GType_CRUISE,
		0,
	},
	{AD_TABLE_END_MARKER, 0, 0}
};

}

class CruiseMetaEngine : public AdvancedMetaEngine {
public:
	CruiseMetaEngine() : AdvancedMetaEngine(Cruise::gameDescriptions, sizeof(Cruise::CRUISEGameDescription), cruiseGames) {
		_singleid = "cruise";
		_guioptions = GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI);
	}

	virtual const char *getName() const {
		return "CruisE";
	}

	virtual const char *getOriginalCopyright() const {
		return "Cruise for a Corpse (C) Delphine Software";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual int getMaximumSaveSlot() const { return 99; }
	virtual SaveStateList listSaves(const char *target) const;
	virtual void removeSaveState(const char *target, int slot) const;
	virtual SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool CruiseMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSupportsLoadingDuringStartup);
}

SaveStateList CruiseMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern("cruise.s??");

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				Cruise::CruiseSavegameHeader header;
				Cruise::readSavegameHeader(in, header);
				saveList.push_back(SaveStateDescriptor(slotNum, header.saveName));
				delete header.thumbnail;
				delete in;
			}
		}
	}

	return saveList;
}

void CruiseMetaEngine::removeSaveState(const char *target, int slot) const {
	g_system->getSavefileManager()->removeSavefile(Cruise::CruiseEngine::getSavegameFile(slot));
}

SaveStateDescriptor CruiseMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::InSaveFile *f = g_system->getSavefileManager()->openForLoading(
		Cruise::CruiseEngine::getSavegameFile(slot));

	if (f) {
		Cruise::CruiseSavegameHeader header;
		Cruise::readSavegameHeader(f, header);
		delete f;

		// Create the return descriptor
		SaveStateDescriptor desc(slot, header.saveName);
		desc.setThumbnail(header.thumbnail);

		return desc;
	}

	return SaveStateDescriptor();
}

bool CruiseMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Cruise::CRUISEGameDescription *gd = (const Cruise::CRUISEGameDescription *)desc;
	if (gd) {
		*engine = new Cruise::CruiseEngine(syst, gd);
	}
	return gd != 0;
}


#if PLUGIN_ENABLED_DYNAMIC(CRUISE)
REGISTER_PLUGIN_DYNAMIC(CRUISE, PLUGIN_TYPE_ENGINE, CruiseMetaEngine);
#else
REGISTER_PLUGIN_STATIC(CRUISE, PLUGIN_TYPE_ENGINE, CruiseMetaEngine);
#endif
