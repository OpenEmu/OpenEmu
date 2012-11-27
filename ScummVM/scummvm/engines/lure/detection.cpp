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
#include "engines/engine.h"
#include "common/savefile.h"

#include "lure/lure.h"

namespace Lure {

struct LureGameDescription {
	ADGameDescription desc;

	uint32 features;
};

uint32 LureEngine::getFeatures() const { return _gameDescription->features; }
Common::Language LureEngine::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform LureEngine::getPlatform() const { return _gameDescription->desc.platform; }

LureLanguage LureEngine::getLureLanguage() const {
	switch (_gameDescription->desc.language) {
	case Common::IT_ITA: return LANG_IT_ITA;
	case Common::FR_FRA: return LANG_FR_FRA;
	case Common::DE_DEU: return LANG_DE_DEU;
	case Common::ES_ESP: return LANG_ES_ESP;
	case Common::EN_ANY: return LANG_EN_ANY;
	case Common::UNK_LANG: return LANG_UNKNOWN;
	default:
		error("Unknown game language");
	}
}

} // End of namespace Lure

static const PlainGameDescriptor lureGames[] = {
	{"lure", "Lure of the Temptress"},
	{0, 0}
};


namespace Lure {

static const LureGameDescription gameDescriptions[] = {
	{
		{
			"lure",
			"VGA",
			AD_ENTRY1("disk1.vga", "b2a8aa6d7865813a17a3c636e063572e"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"EGA",
			AD_ENTRY1("disk1.ega", "e9c9fdd8a19f7910d68e53cb84651273"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY | GF_EGA,
	},

	{
		{
			"lure",
			"VGA",
			AD_ENTRY1("disk1.vga", "cf69d5ada228dd74f89046691c16aafb"),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"EGA",
			AD_ENTRY1("disk1.ega", "b80aced0321f64c58df2c7d3d74dfe79"),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY | GF_EGA,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "7aa19e444dab1ac7194d9f7a40ffe54a"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "894a2c2caeccbad2fc2f4a79a8ee47b0"),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "1c94475c1bb7e0e88c1757d3b5377e94"),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	{
		{
			"lure",
			"",
			AD_ENTRY1("disk1.vga", "1751145b653959f7a64fe1618d6b97ac"),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GF_FLOPPY,
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Lure

class LureMetaEngine : public AdvancedMetaEngine {
public:
	LureMetaEngine() : AdvancedMetaEngine(Lure::gameDescriptions, sizeof(Lure::LureGameDescription), lureGames) {
		_md5Bytes = 1024;
		_singleid = "lure";

		// Use kADFlagUseExtraAsHint to distinguish between EGA and VGA versions
		// of italian Lure when their datafiles sit in the same directory.
		_flags = kADFlagUseExtraAsHint;
		_guioptions = GUIO1(GUIO_NOSPEECH);
	}

	virtual const char *getName() const {
		return "Lure";
	}

	virtual const char *getOriginalCopyright() const {
		return "Lure of the Temptress (C) Revolution";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool LureMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave);
}

bool Lure::LureEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool LureMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Lure::LureGameDescription *gd = (const Lure::LureGameDescription *)desc;
	if (gd) {
		*engine = new Lure::LureEngine(syst, gd);
	}
	return gd != 0;
}

SaveStateList LureMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String saveDesc;
	Common::String pattern = "lure.???";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 999) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				saveDesc = Lure::getSaveName(in);
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	return saveList;
}

int LureMetaEngine::getMaximumSaveSlot() const { return 999; }

void LureMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = target;
	filename += Common::String::format(".%03d", slot);

	g_system->getSavefileManager()->removeSavefile(filename);
}

#if PLUGIN_ENABLED_DYNAMIC(LURE)
	REGISTER_PLUGIN_DYNAMIC(LURE, PLUGIN_TYPE_ENGINE, LureMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(LURE, PLUGIN_TYPE_ENGINE, LureMetaEngine);
#endif
