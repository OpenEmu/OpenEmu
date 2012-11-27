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

#include "groovie/groovie.h"
#include "groovie/detection.h"
#include "groovie/saveload.h"

#include "common/system.h"

namespace Groovie {

static const PlainGameDescriptor groovieGames[] = {
	// Games
	{"t7g", "The 7th Guest"},

#ifdef ENABLE_GROOVIE2
	{"11h", "The 11th Hour: The Sequel to The 7th Guest"},
	{"clandestiny", "Clandestiny"},
	{"unclehenry", "Uncle Henry's Playhouse"},
	{"tlc", "Tender Loving Care"},
#endif

	// Unknown
	{"groovie", "Groovie engine game"},
	{0, 0}
};

static const GroovieGameDescription gameDescriptions[] = {

	// The 7th Guest DOS English
	{
		{
			"t7g", "",
			AD_ENTRY1s("script.grv", "d1b8033b40aa67c076039881eccce90d", 16659),
			Common::EN_ANY, Common::kPlatformPC, ADGF_NO_FLAGS,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieT7G, 0
	},

	// The 7th Guest Mac English
	{
		{
			"t7g", "",
			AD_ENTRY1s("T7GMac", "acdc4a58dd3f007f65e99b99d78e0bce", 1814029),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_MACRESFORK,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieT7G, 0
	},

#if 0
	// These entries should now be identical to the first T7G Mac entry after
	// changing the app to only use the data part of the resource fork. They
	// are left disabled here as a reference.

	// The 7th Guest Mac English (Aztec single disc)
	{
		{
			"t7g", "",
			AD_ENTRY1s("T7GMac", "6bdee8d0f9eef6d58d02fcd7deec3fb2", 1830783),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_MACRESFORK,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieT7G, 0
	},

	// The 7th Guest Mac English (Aztec bundle, provided by Thefinaleofseem)
	{
		{
			"t7g", "",
			AD_ENTRY1s("T7GMac", "0d595d4b44ae1814082938d051e5174e", 1830783),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_MACRESFORK,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieT7G, 0
	},
#endif

	// The 7th Guest DOS Russian (Akella)
	{
		{
			"t7g", "",
			{
				{ "script.grv", 0, "d1b8033b40aa67c076039881eccce90d", 16659},
				{ "intro.gjd", 0, NULL, 31711554},
				{ NULL, 0, NULL, 0}
			},
			Common::RU_RUS, Common::kPlatformPC, ADGF_NO_FLAGS,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieT7G, 0
	},

	{
		{
			"t7g", "",
			{
				{ "script.grv", 0, "d1b8033b40aa67c076039881eccce90d", 16659},
				{ "SeventhGuest", 0, NULL, -1},
				{ NULL, 0, NULL, 0}
			},
			Common::EN_ANY, Common::kPlatformIOS, ADGF_NO_FLAGS,
			GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)
		},
		kGroovieT7G, 0
	},

#ifdef ENABLE_GROOVIE2
	// The 11th Hour DOS English
	{
		{
			"11h", "",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformPC, ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	// The 11th Hour DOS Demo English
	{
		{
			"11h", "Demo",
			AD_ENTRY1s("disk.1", "aacb32ce07e0df2894bd83a3dee40c12", 70),
			Common::EN_ANY, Common::kPlatformPC, ADGF_DEMO | ADGF_UNSTABLE,
			GUIO5(GUIO_NOLAUNCHLOAD, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	// The Making of The 11th Hour DOS English
	{
		{
			"11h", "Making Of",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformPC, ADGF_UNSTABLE,
			GUIO3(GUIO_NOMIDI, GUIO_NOLAUNCHLOAD, GUIO_NOASPECT)
		},
		kGroovieV2, 2
	},

	// Clandestiny Trailer DOS English
	{
		{
			"clandestiny", "Trailer",
			AD_ENTRY1s("disk.1", "5c0428cd3659fc7bbcd0aa16485ed5da", 227),
			Common::EN_ANY, Common::kPlatformPC, ADGF_UNSTABLE,
			GUIO3(GUIO_NOMIDI, GUIO_NOLAUNCHLOAD, GUIO_NOASPECT)
		},
		kGroovieV2, 3
	},

	// Clandestiny DOS English
	{
		{
			"clandestiny", "",
			AD_ENTRY1s("disk.1", "f79fc1515174540fef6a34132efc4c53", 76),
			Common::EN_ANY, Common::kPlatformPC, ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	// Uncle Henry's Playhouse PC English
	{
		{
			"unclehenry", "",
			AD_ENTRY1s("disk.1", "0e1b1d3cecc4fc7efa62a968844d1f7a", 72),
			Common::EN_ANY, Common::kPlatformPC, ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},

	// Tender Loving Care PC English
	{
		{
			"tlc", "",
			AD_ENTRY1s("disk.1", "32a1afa68478f1f9d2b25eeea427f2e3", 84),
			Common::EN_ANY, Common::kPlatformPC, ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)
		},
		kGroovieV2, 1
	},
#endif

	{AD_TABLE_END_MARKER, kGroovieT7G, 0}
};

class GroovieMetaEngine : public AdvancedMetaEngine {
public:
	GroovieMetaEngine() : AdvancedMetaEngine(gameDescriptions, sizeof(GroovieGameDescription), groovieGames) {
		_singleid = "groovie";

		// Use kADFlagUseExtraAsHint in order to distinguish the 11th hour from
		// its "Making of" as well as the Clandestiny Trailer; they all share
		// the same MD5.
		// TODO: Is this the only reason, or are there others (like the three
		// potentially sharing a single directory) ? In the former case, then
		// perhaps a better solution would be to add additional files
		// to the detection entries. In the latter case, this TODO should be
		// replaced with an according explanation.
		_flags = kADFlagUseExtraAsHint;
		_guioptions = GUIO3(GUIO_NOSUBTITLES, GUIO_NOSFX, GUIO_NOASPECT);
	}

	const char *getName() const {
		return "Groovie";
	}

	const char *getOriginalCopyright() const {
		return "Groovie Engine (C) 1990-1996 Trilobyte";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;

	bool hasFeature(MetaEngineFeature f) const;
	SaveStateList listSaves(const char *target) const;
	int getMaximumSaveSlot() const;
	void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

bool GroovieMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd) {
		*engine = new GroovieEngine(syst, (const GroovieGameDescription *)gd);
	}
	return gd != 0;
}

bool GroovieMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo);
}

SaveStateList GroovieMetaEngine::listSaves(const char *target) const {
	return SaveLoad::listValidSaves(target);
}

int GroovieMetaEngine::getMaximumSaveSlot() const {
	return SaveLoad::getMaximumSlot();
}

void GroovieMetaEngine::removeSaveState(const char *target, int slot) const {
	if (!SaveLoad::isSlotValid(slot)) {
		// Invalid slot, do nothing
		return;
	}

	Common::String filename = SaveLoad::getSlotSaveName(target, slot);
	g_system->getSavefileManager()->removeSavefile(filename);
}

SaveStateDescriptor GroovieMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor desc;

	Common::InSaveFile *savefile = SaveLoad::openForLoading(target, slot, &desc);
	delete savefile;

	return desc;
}

} // End of namespace Groovie

#if PLUGIN_ENABLED_DYNAMIC(GROOVIE)
	REGISTER_PLUGIN_DYNAMIC(GROOVIE, PLUGIN_TYPE_ENGINE, Groovie::GroovieMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GROOVIE, PLUGIN_TYPE_ENGINE, Groovie::GroovieMetaEngine);
#endif
