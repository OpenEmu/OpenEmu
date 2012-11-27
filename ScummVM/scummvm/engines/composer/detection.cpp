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

#include "composer/composer.h"

namespace Composer {

struct ComposerGameDescription {
	ADGameDescription desc;

	int gameType;
};

int ComposerEngine::getGameType() const {
	return _gameDescription->gameType;
}

const char *ComposerEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}

uint32 ComposerEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language ComposerEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

}

static const PlainGameDescriptor composerGames[] = {
	{"composer", "Composer Game"},
	{"babayaga", "Magic Tales: Baba Yaga and the Magic Geese"},
	{"darby", "Darby the Dragon"},
	{"gregory", "Gregory and the Hot Air Balloon"},
	{"imoking", "Magic Tales: Imo and the King"},
	{"liam", "Magic Tales: Liam Finds a Story"},
	{"littlesamurai", "Magic Tales: The Little Samurai"},
	{"princess", "The Princess and the Crab"},
	{"sleepingcub", "Sleeping Cub's Test of Courage"},
	{0, 0}
};

namespace Composer {

static const ComposerGameDescription gameDescriptions[] = {
	// Magic Tales: Baba Yaga and the Magic Geese - from bug #3485018
	{
		{
			"babayaga",
			"",
			AD_ENTRY1s("book.ini", "412b7f4b0ef07f442009d28e3a819974", 3852),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV1
	},

	// Magic Tales: Baba Yaga and the Magic Geese Mac - from bug #3466402
	{
		{
			"babayaga",
			"",
			AD_ENTRY1("Baba Yaga", "ae3a4445f42fe10253da7ee4ea0d37"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV1
	},

	// Magic Tales: Imo and the King - from bug #3485018
	{
		{
			"imoking",
			"",
			AD_ENTRY1s("book.ini", "62b52a1763cce7d7d6ccde9f9d32fd4b", 3299),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV1
	},

	// Magic Tales: Imo and the King Mac - from bug #3466402
	{
		{
			"imoking",
			"",
			AD_ENTRY1("imo and the king", "b0277885fec943b5f19409f35b33964c"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV1
	},

	// Magic Tales: The Little Samurai - from bug #3485018
	{
		{
			"littlesamurai",
			"",
			AD_ENTRY1s("book.ini", "7a851869d022a9041e0dd11e5bace09b", 3747),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV1
	},

	// Magic Tales: The Little Samurai Mac - from bug #3466402
	{
		{
			"littlesamurai",
			"",
			AD_ENTRY1("The Little Samurai", "38121dd649c24e8676aa108cf35d44b5"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV1
	},

	// from Liam Finds a Story CD
	{
		{
			"magictales",
			"Magic Tales Demo: Baby Yaga, Samurai, Imo",
			AD_ENTRY1("book.ini", "dbc98c566f4ac61b544443524585dccb"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV1
	},

	{
		{
			"liam",
			0,
			AD_ENTRY1s("book.ini", "fc9d9b9e72e7301d011b808606eaa15b", 834),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	// Liam Finds a Story Mac - from bug #3463201
	{
		{
			"liam",
			0,
			{
				{"liam finds a story.ini", 0, "85a1ca6002ded8572920bbdb73d35b0a", -1},
				{"page99.rsc", 0, "11b0a19c6b6d73c39e2bd289a457c1dc", -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	// from Liam Finds a Story CD
	{
		{
			"magictales",
			"Magic Tales Demo: Sleeping Cub, Princess & Crab",
			AD_ENTRY1("book.ini", "3dede2522bb0886c95667b082987a87f"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	{
		{
			"darby",
			0,
			{
				{"book.ini", 0, "7e3404c559d058521fff2aebe5c427a8", 2545},
				{"page99.rsc", 0, "49cc6b16caa1c5ec7d94a3c47eed9a02", 1286480},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	{
		{
			"darby",
			0,
			AD_ENTRY1("Darby the Dragon.ini", "d81f9214936fa70d42fc578908d4bb3d"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	{ // Provided by Strangerke, "CD-Rom 100% Malin" Pack
		{
			"darby",
			0,
			AD_ENTRY1("book.ini", "285308372f7dddff2ca5a25c9192cf5c"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	{
		{
			"gregory",
			0,
			{
				{"book.ini", 0, "14a562dcf361773445255af9f3e94790", 2234},
				{"page99.rsc", 0, "01f9381162467e052dfd4c704169ef3e", 388644},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	{
		{
			"gregory",
			0,
			AD_ENTRY1("Gregory.ini", "fa82f14731f28c7379c5a106df07a0d6"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	{ // Provided by Strangerke, "CD-Rom 100% Malin" Pack
		{
			"gregory",
			0,
			AD_ENTRY1("book.ini", "e54fc5c00de5f94e908a969e445af5d0"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	{ // Provided by sev
		{
			"princess",
			0,
			{
				{"book.ini", 0, "fb32572577b9a41ba299825ef1e3181e", 966},
				{"page99.rsc", 0, "fd5ebd3b5e36c4651c50241619525355", 45418},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	// The Princess and the Crab Mac - From Bug #3461984
	{
		{
			"princess",
			0,
			{
				{"the princess and the crab.ini", 0, "f6b551a7304643004bd5e4df7ac1e76e", -1},
				{"page99.rsc", 0, "fd5ebd3b5e36c4651c50241619525355", -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	{ // Provided by sev
		{
			"sleepingcub",
			0,
			{
				{"book.ini", 0, "0d329e592387009c6387a733a3ea2235", 964},
				{"page99.rsc", 0, "219fbd9bd2ff87c7023814405d753145", 46916},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	// Sleeping Cub Mac - From Bug #3461369
	{
		{
			"sleepingcub",
			0,
			{
				{"sleeping cub.ini", 0, "39642a4036cb51443f5e90052c3ad0b2", -1},
				{"page99.rsc", 0, "219fbd9bd2ff87c7023814405d753145", -1},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_ComposerV2
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Composer

using namespace Composer;

// we match from data too, to stop detection from a non-top-level directory
static const char *directoryGlobs[] = {
	"data",
	"liam",
	"programs",
	"princess",
	"sleepcub",
	0
};

class ComposerMetaEngine : public AdvancedMetaEngine {
public:
	ComposerMetaEngine() : AdvancedMetaEngine(Composer::gameDescriptions, sizeof(Composer::ComposerGameDescription), composerGames) {
		_singleid = "composer";
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	virtual const char *getName() const {
		return "Magic Composer Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Copyright (C) 1995-1999 Animation Magic";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
};

bool ComposerMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Composer::ComposerGameDescription *gd = (const Composer::ComposerGameDescription *)desc;
	if (gd) {
		*engine = new Composer::ComposerEngine(syst, gd);
	}
	return gd != 0;
}

bool ComposerMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

bool Composer::ComposerEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsRTL);
}

#if PLUGIN_ENABLED_DYNAMIC(COMPOSER)
REGISTER_PLUGIN_DYNAMIC(COMPOSER, PLUGIN_TYPE_ENGINE, ComposerMetaEngine);
#else
REGISTER_PLUGIN_STATIC(COMPOSER, PLUGIN_TYPE_ENGINE, ComposerMetaEngine);
#endif
