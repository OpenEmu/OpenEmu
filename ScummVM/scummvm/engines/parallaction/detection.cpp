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

#include "common/config-manager.h"
#include "engines/advancedDetector.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "parallaction/parallaction.h"

namespace Parallaction {

struct PARALLACTIONGameDescription {
	ADGameDescription desc;

	int gameType;
	uint32 features;
};

int Parallaction::getGameType() const { return _gameDescription->gameType; }
uint32 Parallaction::getFeatures() const { return _gameDescription->features; }
Common::Language Parallaction::getLanguage() const { return _gameDescription->desc.language; }
Common::Platform Parallaction::getPlatform() const { return _gameDescription->desc.platform; }
}

static const PlainGameDescriptor parallactionGames[] = {
	{"parallaction", "Parallaction engine game"},
	{"nippon", "Nippon Safes Inc."},
	{"bra", "The Big Red Adventure"},
	{0, 0}
};

namespace Parallaction {

static const PARALLACTIONGameDescription gameDescriptions[] = {
	{
		{
			"nippon",
			"Multi-lingual",
			{
				{ "disk1",	0, "610363308258e926dbabd5a9e7bb769f", 1060142},
				{ "disk2",	0, "bfdd7bcfbc226f4acf3f67fa9efa2826", 907205},
				{ "disk3",	0, "eec08180240888d76e3cfe3e183d5d5d", 1030721},
				{ "disk4",	0, "5bffddc7db226bdaa7dd3e10e5a15e68", 1151403},
				{ "en",		0, "65cbfa81eafe308621184796ed116700", 399360},
				{ "fr",		0, "ac20c743ea10f2cb4491f76c5644582c", 410624},
				{ "ge",		0, "50916bfa34aee1380e0e959b37eceb5a", 410624},
				{ "it",		0, "89964aef04d2c53a615ee8983caf2775", 410624},
				{ NULL, 0, NULL, 0}
			},
			Common::UNK_LANG,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_Nippon,
		GF_LANG_EN | GF_LANG_FR | GF_LANG_DE | GF_LANG_IT | GF_LANG_MULT,
	},

	{
		{
			"nippon",
			"Multi-lingual",
			{
				{ "disk0",	0, "16cca8724fdf4ec8234385497a0c728a", 208437},
				{ "disk1",	0, "6b29987cfe2298d3745b6d99a0080c44", 901120},
				{ "disk2",	0, "2db40bf8198a57d18e4471a6deaab970", 901120},
				{ "disk3",	0, "0486972962b2bfc230e789b9f88f9ec8", 901120},
				{ "disk4",	0, "6f625e7f05da4a2f57d6b62d57013614", 901120},
				{ "en",		0, "c9ec4f2267d736eef4877c5133e1c6e1", 174074},
				{ "ge",		0, "42d6f10a4ebdadb25a6161d53ea4f450", 182298},
				{ "fr",		0, "cf17defc24f143d1a9acb52eaa5c2406", 179958},
				{ NULL, 0, NULL, 0}
			},
			Common::UNK_LANG,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_Nippon,
		GF_LANG_EN | GF_LANG_FR | GF_LANG_DE | GF_LANG_MULT,
	},

	{
		{
			"nippon",
			"Demo",
			{
				{ "disk0",	0, "6fed2e18a6bfe5e8bb49144fcc95fd11", 624640},
				{ "fr",		0, "72f04be4320dfac719431419ec2b9a0d", 12778},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY,
			Common::kPlatformAmiga,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_Nippon,
		GF_LANG_EN | GF_DEMO,
	},


	{
		{
			"nippon",
			"",
			{
				{ "disk0",	0, "bfee75d8015f1fb97e75dbe08df4bef7", 354304},
				{ "disk1",	0, "f339dd108c1a1f5cd4853d9966e5d01f", 901120},
				{ "disk2",	0, "2db40bf8198a57d18e4471a6deaab970", 901120},
				{ "disk3",	0, "0486972962b2bfc230e789b9f88f9ec8", 901120},
				{ "disk4",	0, "6f625e7f05da4a2f57d6b62d57013614", 901120},
				{ "it",		0, "746088eb8de2b2713685d243a4e4678f", 185344},
				{ NULL, 0, NULL, 0}
			},
			Common::IT_ITA,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_Nippon,
		GF_LANG_IT,
	},

	{
		{
			"bra",
			"Multi-lingual",
			{
				{ "tbra.bmp",	0, "3174c095a0e1a4eaf05c403445711e9b", 80972 },
				{ "russia.fnt",	0, "57f85ff62aeca6334fdcaf718e313b49", 18344 },
				{ NULL, 0, NULL, 0}
			},
			Common::UNK_LANG,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_BRA,
		GF_LANG_EN | GF_LANG_FR | GF_LANG_DE | GF_LANG_IT | GF_LANG_MULT,
	},

	{
		{
			"bra",
			"Demo",
			{
				{ "russia.fnt",	0, "0dd55251d2886d6783718df2b184bf97", 10649 },
				{ NULL, 0, NULL, 0}
			},
			Common::UNK_LANG,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_BRA,
		GF_LANG_EN | GF_DEMO,
	},

	{
		{
			"bra",
			"Multi-lingual",
			{
				{ "request.win", 0, "7a844b9518310e4cc72eabb9c0340314", 6497 },
				{ NULL, 0, NULL, 0}
			},
			Common::UNK_LANG,
			Common::kPlatformAmiga,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_BRA,
		GF_LANG_EN | GF_LANG_FR | GF_LANG_DE | GF_LANG_IT | GF_LANG_MULT,
	},

	{
		{
			"bra",
			"Demo",
			{
				{ "request.win", 0, "3b6a99ffd626e324b663839bbad59cb3", 5326 },
				{ NULL, 0, NULL, 0}
			},
			Common::UNK_LANG,
			Common::kPlatformAmiga,
			ADGF_DEMO,
			GUIO1(GUIO_NOSPEECH)
		},
		GType_BRA,
		GF_LANG_EN | GF_DEMO,
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

}

class ParallactionMetaEngine : public AdvancedMetaEngine {
public:
	ParallactionMetaEngine() : AdvancedMetaEngine(Parallaction::gameDescriptions, sizeof(Parallaction::PARALLACTIONGameDescription), parallactionGames) {
		_guioptions = GUIO1(GUIO_NOLAUNCHLOAD);
	}

	virtual const char *getName() const {
		return "Parallaction";
	}

	virtual const char *getOriginalCopyright() const {
		return "Nippon Safes Inc. (C) Dynabyte";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
};

bool ParallactionMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave);
}

bool Parallaction::Parallaction::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

bool ParallactionMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Parallaction::PARALLACTIONGameDescription *gd = (const Parallaction::PARALLACTIONGameDescription *)desc;
	bool res = true;

	switch (gd->gameType) {
	case Parallaction::GType_Nippon:
		*engine = new Parallaction::Parallaction_ns(syst, gd);
		break;
	case Parallaction::GType_BRA:
		*engine = new Parallaction::Parallaction_br(syst, gd);
		break;
	default:
		res = false;
		error("Parallaction engine: unknown gameType");
	}

	return res;
}

SaveStateList ParallactionMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();

	Common::String pattern(ConfMan.getDomain(target)->getVal("gameid") + ".0??");
	Common::StringArray filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 2 digits of the filename, since they correspond to the save slot
		int slotNum = atoi(file->c_str() + file->size() - 2);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				Common::String saveDesc = in->readLine();
				saveList.push_back(SaveStateDescriptor(slotNum, saveDesc));
				delete in;
			}
		}
	}

	return saveList;
}

int ParallactionMetaEngine::getMaximumSaveSlot() const { return 99; }

void ParallactionMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String filename = ConfMan.getDomain(target)->getVal("gameid");
	filename += Common::String::format(".0%02d", slot);

	g_system->getSavefileManager()->removeSavefile(filename);
}

#if PLUGIN_ENABLED_DYNAMIC(PARALLACTION)
	REGISTER_PLUGIN_DYNAMIC(PARALLACTION, PLUGIN_TYPE_ENGINE, ParallactionMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(PARALLACTION, PLUGIN_TYPE_ENGINE, ParallactionMetaEngine);
#endif
