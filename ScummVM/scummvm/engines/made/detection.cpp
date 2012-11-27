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

#include "made/made.h"

#include "engines/advancedDetector.h"

namespace Made {

struct MadeGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

uint32 MadeEngine::getGameID() const {
	return _gameDescription->gameID;
}

uint32 MadeEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform MadeEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 MadeEngine::getVersion() const {
	return _gameDescription->version;
}

}

static const PlainGameDescriptor madeGames[] = {
	{"made", "MADE engine game"},
	{"manhole", "The Manhole"},
	{"rtz", "Return to Zork"},
	{"lgop2", "Leather Goddesses of Phobos 2"},
	{"rodney", "Rodney's Funscreen"},
	{0, 0}
};


namespace Made {

static const MadeGameDescription gameDescriptions[] = {
	{
		// NOTE: Return to Zork entries with *.dat are used to detect the game via rtzcd.dat,
		// which is packed inside rtzcd.red. Entries with *.red refer to the packed file
		// directly, which is the "official" way.

		// Return to Zork - English CD version 1.0 9/15/93 (installed)
		// Patch #1953654 submitted by spookypeanut
		{
			"rtz",
			"V1.0, 9/15/93, installed, CD",
			AD_ENTRY1("rtzcd.dat", "e95c38ded389e39cfbf87a8cb250b12e"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - English CD version 1.0 9/15/93
		// Patch #1953654 submitted by spookypeanut
		{
			"rtz",
			"V1.0, 9/15/93, CD",
			AD_ENTRY1("rtzcd.red", "cd8b62ece4677c438688c1de3f5379b9"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - English CD version 1.1 12/7/93 (installed)
		{
			"rtz",
			"V1.1, 12/7/93, installed, CD",
			AD_ENTRY1s("rtzcd.dat", "a1db8c97a78dae10f91d356f16ad07b8", 536064),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - English CD version 1.1 12/7/93
		{
			"rtz",
			"V1.1, 12/7/93, CD",
			AD_ENTRY1s("rtzcd.red", "c4e2430e6b6c6ff1562a80fb4a9df24c", 276177),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - English CD version 1.2 9/29/94 (installed)
		// Supplied by Dark-Star in the ScummVM forums
		{
			"rtz",
			"V1.2, 9/29/94, installed, CD",
			AD_ENTRY1("rtzcd.dat", "9d740378da2d16e83d0d0efff01bf83a"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - English CD version 1.2 9/29/94
		{
			"rtz",
			"V1.2, 9/29/94, CD",
			AD_ENTRY1s("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2", 276584),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - German CD version 1.2 9/29/94 (installed)
		// Supplied by Dark-Star in the ScummVM forums
		{
			"rtz",
			"V1.2, 9/29/94, installed, CD",
			AD_ENTRY1s("rtzcd.dat", "9d740378da2d16e83d0d0efff01bf83a", 525824),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - German CD version 1.2 4/18/95
		// Supplied by Dark-Star in the ScummVM forums
		{
			"rtz",
			"V1.2, 4/18/95, CD",
			AD_ENTRY1s("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2", 355442),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

 	{
 		// Return to Zork - Italian CD version 1.2 3/31/95 (installed)
		// Patch #2685032 submitted by goodoldgeorg
 		{
 			"rtz",
 			"V1.2, 3/31/95, installed, CD",
 			AD_ENTRY1s("rtzcd.dat", "5b86035aed0277f96e3d173542b5364a", 523776),
 			Common::IT_ITA,
 			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
 		},
 		GID_RTZ,
 		0,
 		GF_CD,
 		3,
 	},

	{
 		// Return to Zork - Italian CD version 1.2 3/31/95
		// Patch #2685032 submitted by goodoldgeorg
 		{
 			"rtz",
 			"V1.2, 3/31/95, CD",
 			AD_ENTRY1s("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2", 354971),
 			Common::IT_ITA,
 			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
 		},
 		GID_RTZ,
 		0,
 		GF_CD_COMPRESSED,
 		3,
 	},

 	{
 		// Return to Zork - French CD version 1.2 5/13/95 (installed)
		// Patch #2685032 submitted by goodoldgeorg
 		{
 			"rtz",
 			"V1.2, 5/13/95, installed, CD",
 			AD_ENTRY1s("rtzcd.dat", "bde8251a8e34e87c54e3f93147d56c9e", 523776),
 			Common::FR_FRA,
 			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
 		},
 		GID_RTZ,
 		0,
 		GF_CD,
 		3,
 	},

 	{
 		// Return to Zork - French CD version 1.2 5/13/95
		// Patch #2685032 submitted by goodoldgeorg
 		{
 			"rtz",
 			"V1.2, 3/31/95, CD",
 			AD_ENTRY1s("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2", 354614),
 			Common::FR_FRA,
 			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
 		},
 		GID_RTZ,
 		0,
 		GF_CD_COMPRESSED,
 		3,
 	},

	{
		// Return to Zork - English floppy version
		{
			"rtz",
			"Floppy",
			AD_ENTRY1("rtz.prj", "764d02f52ce1c219f2c0066677fba4ce"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_RTZ,
		0,
		GF_FLOPPY,
		3,
	},

	{
		// Return to Zork - Demo
		{
			"rtz",
			"Demo",
			AD_ENTRY1("demo.dat", "2a6a1354bd5346fad4aee08e5b56caaa"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_DEMO,
		3,
	},

	{
		// Return to Zork - Japanese DOS
		// This is the RTZCD.DAT in the base directory of the FM-Towns CD
		{
			"rtz",
			"",
			AD_ENTRY1("rtzcd.dat", "c4fccf67ad247f09b94c3c808b138576"),
			Common::JA_JPN,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - Japanese FM-Towns
		// This is in the RTZFM folder of the FM-Towns CD
		{
			"rtz",
			"",
			AD_ENTRY1("rtzcd.dat", "e949a6a42d82daabfa7d4dc0a87a9843"),
			Common::JA_JPN,
			Common::kPlatformFMTowns,
			ADGF_CD,
			GUIO1(GUIO_NOASPECT)
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - Japanese PC-98
		// This is in the RTZ9821 folder of the FM-Towns CD
		{
			"rtz",
			"",
			AD_ENTRY1("rtzcd.dat", "0c0117e98530c736a141c2aad6834dc5"),
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// The Manhole: New and Enhanced
		{
			"manhole",
			"",
			AD_ENTRY1("manhole.dat", "cb21e31ed35c963208343bc995225b73"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_CD,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_MANHOLE,
		0,
		GF_CD,
		2,
	},

	{
		// The Manhole (EGA, 5.25")
		{
			"manhole",
			"EGA",
			AD_ENTRY1("manhole.dat", "2b1658292599a861c4cd3cf6cdb3c581"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_MANHOLE,
		0,
		GF_FLOPPY,
		1,
	},

	{
		// Leather Goddesses of Phobos 2 (English)
		{
			"lgop2",
			"",
			AD_ENTRY1("lgop2.dat", "8137996db200ff67e8f172ff106f2e48"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_LGOP2,
		0,
		GF_FLOPPY,
		2,
	},

	{
		// Leather Goddesses of Phobos 2 (German)
		// Supplied by windlepoons (bug tracker #2675695)
		{
			"lgop2",
			"",
			AD_ENTRY1s("lgop2.dat", "a0ffea6a3b7e39bd861edd00c397641c", 299466),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_LGOP2,
		0,
		GF_FLOPPY,
		2,
	},

	{
		// Leather Goddesses of Phobos 2 (French)
		// Supplied by goodoldgeorg (bug tracker #2675759)
		{
			"lgop2",
			"",
			AD_ENTRY1s("lgop2.dat", "f9e974087af7cf4b7ec2d8dc45d01e0c", 295366),
			Common::FR_FRA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_LGOP2,
		0,
		GF_FLOPPY,
		2,
	},

	{
		// Leather Goddesses of Phobos 2 (Spanish)
		// Supplied by goodoldgeorg (bug tracker #2675759)
		{
			"lgop2",
			"",
			AD_ENTRY1s("lgop2.dat", "96eb95b4d75b9a3da0b0d67e3b4a787d", 288984),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_LGOP2,
		0,
		GF_FLOPPY,
		2,
	},

	{
		// Rodney's Funscreen
		{
			"rodney",
			"",
			AD_ENTRY1("rodneys.dat", "a79887dbaa47689facd7c6f09258ba5a"),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_RODNEY,
		0,
		GF_FLOPPY,
		2,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

/**
 * The fallback game descriptor used by the Made engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static MadeGameDescription g_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformPC,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	0,
	0,
	0,
	0,
};

} // End of namespace Made

class MadeMetaEngine : public AdvancedMetaEngine {
public:
	MadeMetaEngine() : AdvancedMetaEngine(Made::gameDescriptions, sizeof(Made::MadeGameDescription), madeGames) {
		_singleid = "made";
	}

	virtual const char *getName() const {
		return "MADE";
	}

	virtual const char *getOriginalCopyright() const {
		return "MADE Engine (C) Activision";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;

	const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const;

};

bool MadeMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		false;
}

bool Made::MadeEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

bool MadeMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Made::MadeGameDescription *gd = (const Made::MadeGameDescription *)desc;
	if (gd) {
		*engine = new Made::MadeEngine(syst, gd);
	}
	return gd != 0;
}

const ADGameDescription *MadeMetaEngine::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	// Set the default values for the fallback descriptor's ADGameDescription part.
	Made::g_fallbackDesc.desc.language = Common::UNK_LANG;
	Made::g_fallbackDesc.desc.platform = Common::kPlatformPC;
	Made::g_fallbackDesc.desc.flags = ADGF_NO_FLAGS;

	// Set default values for the fallback descriptor's MadeGameDescription part.
	Made::g_fallbackDesc.gameID = 0;
	Made::g_fallbackDesc.features = 0;
	Made::g_fallbackDesc.version = 3;

	//return (const ADGameDescription *)&Made::g_fallbackDesc;
	return NULL;
}

#if PLUGIN_ENABLED_DYNAMIC(MADE)
	REGISTER_PLUGIN_DYNAMIC(MADE, PLUGIN_TYPE_ENGINE, MadeMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MADE, PLUGIN_TYPE_ENGINE, MadeMetaEngine);
#endif
