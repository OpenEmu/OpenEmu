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

#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "common/file.h"
#include "common/ptr.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "graphics/thumbnail.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"
#include "sci/engine/script.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"

namespace Sci {

// Titles of the games
static const PlainGameDescriptor s_sciGameTitles[] = {
	{"sci",             "Sierra SCI Game"},
	{"sci-fanmade",     "Fanmade SCI Game"},
	// === SCI0 games =========================================================
	{"astrochicken",    "Astro Chicken"},
	{"christmas1988",   "Christmas Card 1988"},
	{"iceman",          "Codename: Iceman"},
	{"camelot",         "Conquests of Camelot: King Arthur, Quest for the Grail"},
	{"funseeker",       "Fun Seeker's Guide"},
	{"hoyle1",          "Hoyle Official Book of Games: Volume 1"},
	{"hoyle2",          "Hoyle Official Book of Games: Volume 2"},
	{"kq4sci",          "King's Quest IV: The Perils of Rosella"},	// Note: There was also an AGI version of this
	{"laurabow",        "Laura Bow: The Colonel's Bequest"},
	{"lsl2",            "Leisure Suit Larry 2: Goes Looking for Love (in Several Wrong Places)"},
	{"lsl3",            "Leisure Suit Larry 3: Passionate Patti in Pursuit of the Pulsating Pectorals"},
	{"mothergoose",     "Mixed-Up Mother Goose"},
	{"pq2",             "Police Quest II: The Vengeance"},
	{"qfg1",            "Quest for Glory I: So You Want to Be a Hero"},	// Note: There was also a SCI11 VGA remake of this (further down)
	{"sq3",             "Space Quest III: The Pirates of Pestulon"},
	// === SCI01 games ========================================================
	{"qfg2",            "Quest for Glory II: Trial by Fire"},
	{"kq1sci",          "King's Quest I: Quest for the Crown"},	// Note: There was also an AGI version of this
	// === SCI1 games =========================================================
	{"castlebrain",     "Castle of Dr. Brain"},
	{"christmas1990",   "Christmas Card 1990: The Seasoned Professional"},
	{"cnick-lsl",       "Crazy Nick's Software Picks: Leisure Suit Larry's Casino"},
	{"cnick-kq",        "Crazy Nick's Software Picks: King Graham's Board Game Challenge"},
	{"cnick-laurabow",  "Crazy Nick's Software Picks: Parlor Games with Laura Bow"},
	{"cnick-longbow",   "Crazy Nick's Software Picks: Robin Hood's Game of Skill and Chance"},
	{"cnick-sq",        "Crazy Nick's Software Picks: Roger Wilco's Spaced Out Game Pack"},
	{"ecoquest",        "EcoQuest: The Search for Cetus"},	// floppy is SCI1, CD SCI1.1
	{"fairytales",      "Mixed-up Fairy Tales"},
	{"hoyle3",          "Hoyle Official Book of Games: Volume 3"},
	{"jones",           "Jones in the Fast Lane"},
	{"kq5",             "King's Quest V: Absence Makes the Heart Go Yonder"},
	{"longbow",         "Conquests of the Longbow: The Adventures of Robin Hood"},
	{"lsl1sci",         "Leisure Suit Larry in the Land of the Lounge Lizards"},	// Note: There was also an AGI version of this
	{"lsl5",            "Leisure Suit Larry 5: Passionate Patti Does a Little Undercover Work"},
	{"mothergoose256",  "Mixed-Up Mother Goose"},
	{"msastrochicken",  "Ms. Astro Chicken"},
	{"pq1sci",          "Police Quest: In Pursuit of the Death Angel"},	// Note: There was also an AGI version of this
	{"pq3",             "Police Quest III: The Kindred"},
	{"sq1sci",          "Space Quest I: The Sarien Encounter"},	// Note: There was also an AGI version of this
	{"sq4",             "Space Quest IV: Roger Wilco and the Time Rippers"},	// floppy is SCI1, CD SCI1.1
	// === SCI1.1 games =======================================================
	{"christmas1992",   "Christmas Card 1992"},
	{"ecoquest2",       "EcoQuest II: Lost Secret of the Rainforest"},
	{"freddypharkas",   "Freddy Pharkas: Frontier Pharmacist"},
	{"hoyle4",          "Hoyle Classic Card Games"},
	{"kq6",             "King's Quest VI: Heir Today, Gone Tomorrow"},
	{"laurabow2",       "Laura Bow 2: The Dagger of Amon Ra"},
	{"qfg1vga",         "Quest for Glory I: So You Want to Be a Hero"},	// Note: There was also a SCI0 version of this (further up)
	{"qfg3",            "Quest for Glory III: Wages of War"},
	{"sq5",             "Space Quest V: The Next Mutation"},
	{"islandbrain",     "The Island of Dr. Brain"},
	{"lsl6",            "Leisure Suit Larry 6: Shape Up or Slip Out!"},
	{"pepper",          "Pepper's Adventure in Time"},
	{"slater",          "Slater & Charlie Go Camping"},
	// === SCI2 games =========================================================
	{"gk1",             "Gabriel Knight: Sins of the Fathers"},	// demo is SCI11, full version SCI32
	{"pq4",             "Police Quest IV: Open Season"}, // floppy is SCI2, CD SCI2.1
	{"qfg4",            "Quest for Glory IV: Shadows of Darkness"},	// floppy is SCI2, CD SCI2.1
	// === SCI2.1 games ========================================================
	{"chest",           "Inside the Chest"},	// aka Behind the Developer's Shield
	{"gk2",             "The Beast Within: A Gabriel Knight Mystery"},
	// TODO: Inside The Chest/Behind the Developer's Shield
	{"kq7",             "King's Quest VII: The Princeless Bride"},
	// TODO: King's Questions
	{"lsl6hires",       "Leisure Suit Larry 6: Shape Up or Slip Out!"},
	{"mothergoosehires","Mixed-Up Mother Goose Deluxe"},
	{"phantasmagoria",  "Phantasmagoria"},
	{"pqswat",          "Police Quest: SWAT"},
	{"shivers",         "Shivers"},
	{"sq6",             "Space Quest 6: The Spinal Frontier"},
	{"torin",           "Torin's Passage"},
	// === SCI3 games =========================================================
	{"lsl7",            "Leisure Suit Larry 7: Love for Sail!"},
	{"lighthouse",      "Lighthouse: The Dark Being"},
	{"phantasmagoria2", "Phantasmagoria II: A Puzzle of Flesh"},
	//{"shivers2",        "Shivers II: Harvest of Souls"},	// Not SCI
	{"rama",            "RAMA"},
	{0, 0}
};

struct GameIdStrToEnum {
	const char *gameidStr;
	SciGameId gameidEnum;
};

static const GameIdStrToEnum s_gameIdStrToEnum[] = {
	{ "astrochicken",    GID_ASTROCHICKEN },
	{ "camelot",         GID_CAMELOT },
	{ "castlebrain",     GID_CASTLEBRAIN },
	{ "chest",           GID_CHEST },
	{ "christmas1988",   GID_CHRISTMAS1988 },
	{ "christmas1990",   GID_CHRISTMAS1990 },
	{ "christmas1992",   GID_CHRISTMAS1992 },
	{ "cnick-kq",        GID_CNICK_KQ },
	{ "cnick-laurabow",  GID_CNICK_LAURABOW },
	{ "cnick-longbow",   GID_CNICK_LONGBOW },
	{ "cnick-lsl",       GID_CNICK_LSL },
	{ "cnick-sq",        GID_CNICK_SQ },
	{ "ecoquest",        GID_ECOQUEST },
	{ "ecoquest2",       GID_ECOQUEST2 },
	{ "fairytales",      GID_FAIRYTALES },
	{ "freddypharkas",   GID_FREDDYPHARKAS },
	{ "funseeker",       GID_FUNSEEKER },
	{ "gk1",             GID_GK1 },
	{ "gk2",             GID_GK2 },
	{ "hoyle1",          GID_HOYLE1 },
	{ "hoyle2",          GID_HOYLE2 },
	{ "hoyle3",          GID_HOYLE3 },
	{ "hoyle4",          GID_HOYLE4 },
	{ "iceman",          GID_ICEMAN },
	{ "islandbrain",     GID_ISLANDBRAIN },
	{ "jones",           GID_JONES },
	{ "kq1sci",          GID_KQ1 },
	{ "kq4sci",          GID_KQ4 },
	{ "kq5",             GID_KQ5 },
	{ "kq6",             GID_KQ6 },
	{ "kq7",             GID_KQ7 },
	{ "laurabow",        GID_LAURABOW },
	{ "laurabow2",       GID_LAURABOW2 },
	{ "lighthouse",      GID_LIGHTHOUSE },
	{ "longbow",         GID_LONGBOW },
	{ "lsl1sci",         GID_LSL1 },
	{ "lsl2",            GID_LSL2 },
	{ "lsl3",            GID_LSL3 },
	{ "lsl5",            GID_LSL5 },
	{ "lsl6",            GID_LSL6 },
	{ "lsl6hires",       GID_LSL6HIRES },
	{ "lsl7",            GID_LSL7 },
	{ "mothergoose",     GID_MOTHERGOOSE },
	{ "mothergoose256",  GID_MOTHERGOOSE256 },
	{ "mothergoosehires",GID_MOTHERGOOSEHIRES },
	{ "msastrochicken",  GID_MSASTROCHICKEN },
	{ "pepper",          GID_PEPPER },
	{ "phantasmagoria",  GID_PHANTASMAGORIA },
	{ "phantasmagoria2", GID_PHANTASMAGORIA2 },
	{ "pq1sci",          GID_PQ1 },
	{ "pq2",             GID_PQ2 },
	{ "pq3",             GID_PQ3 },
	{ "pq4",             GID_PQ4 },
	{ "pqswat",          GID_PQSWAT },
	{ "qfg1",            GID_QFG1 },
	{ "qfg1vga",         GID_QFG1VGA },
	{ "qfg2",            GID_QFG2 },
	{ "qfg3",            GID_QFG3 },
	{ "qfg4",            GID_QFG4 },
	{ "rama",            GID_RAMA },
	{ "sci-fanmade",     GID_FANMADE },	// FIXME: Do we really need/want this?
	{ "shivers",         GID_SHIVERS },
	//{ "shivers2",        GID_SHIVERS2 },	// Not SCI
	{ "slater",          GID_SLATER },
	{ "sq1sci",          GID_SQ1 },
	{ "sq3",             GID_SQ3 },
	{ "sq4",             GID_SQ4 },
	{ "sq5",             GID_SQ5 },
	{ "sq6",             GID_SQ6 },
	{ "torin",           GID_TORIN },
	{ NULL, (SciGameId)-1 }
};

struct OldNewIdTableEntry {
	const char *oldId;
	const char *newId;
	SciVersion version;
};

static const OldNewIdTableEntry s_oldNewTable[] = {
	{ "archive",    "chest",            SCI_VERSION_NONE     },
	{ "arthur",		"camelot",			SCI_VERSION_NONE     },
	{ "brain",      "castlebrain",      SCI_VERSION_1_MIDDLE },	// Amiga
	{ "brain",      "castlebrain",      SCI_VERSION_1_LATE   },
	{ "demo",		"christmas1988",	SCI_VERSION_NONE     },
	{ "card",       "christmas1990",    SCI_VERSION_1_EARLY, },
	{ "card",       "christmas1992",    SCI_VERSION_1_1      },
	{ "RH Budget",	"cnick-longbow",	SCI_VERSION_NONE     },
	// iceman is the same
	{ "icedemo",	"iceman",			SCI_VERSION_NONE     },
	// longbow is the same
	{ "eco",		"ecoquest",			SCI_VERSION_NONE     },
	{ "eco2",		"ecoquest2",		SCI_VERSION_NONE     },	// EcoQuest 2 demo
	{ "rain",		"ecoquest2",		SCI_VERSION_NONE     },	// EcoQuest 2 full
	{ "tales",		"fairytales",		SCI_VERSION_NONE     },
	{ "fp",			"freddypharkas",	SCI_VERSION_NONE     },
	{ "emc",		"funseeker",		SCI_VERSION_NONE     },
	{ "gk",			"gk1",				SCI_VERSION_NONE     },
	// gk2 is the same
	{ "gk2demo",	"gk2",				SCI_VERSION_NONE     },
	{ "hoyledemo",	"hoyle1",			SCI_VERSION_NONE     },
	{ "cardgames",	"hoyle1",			SCI_VERSION_NONE     },
	{ "solitare",	"hoyle2",			SCI_VERSION_NONE     },
	// hoyle3 is the same
	// hoyle4 is the same
	{ "brain",      "islandbrain",      SCI_VERSION_1_1      },
	{ "demo000",	"kq1sci",			SCI_VERSION_NONE     },
	{ "kq1",		"kq1sci",			SCI_VERSION_NONE     },
	{ "kq4",		"kq4sci",			SCI_VERSION_NONE     },
	// kq5 is the same
	// kq6 is the same
	{ "kq7cd",		"kq7",				SCI_VERSION_NONE     },
	{ "mm1",		"laurabow",			SCI_VERSION_NONE     },
	{ "cb1",		"laurabow",			SCI_VERSION_NONE     },
	{ "lb2",		"laurabow2",		SCI_VERSION_NONE     },
	{ "rh",			"longbow",			SCI_VERSION_NONE     },
	{ "ll1",		"lsl1sci",			SCI_VERSION_NONE     },
	{ "lsl1",		"lsl1sci",			SCI_VERSION_NONE     },
	// lsl2 is the same
	{ "lsl3",		"lsl3",				SCI_VERSION_NONE     },
	{ "ll5",		"lsl5",				SCI_VERSION_NONE     },
	// lsl5 is the same
	// lsl6 is the same
	{ "mg",			"mothergoose",		SCI_VERSION_NONE     },
	{ "twisty",		"pepper",			SCI_VERSION_NONE     },
	{ "scary",      "phantasmagoria",   SCI_VERSION_NONE     },
	// TODO: distinguish the full version of Phantasmagoria from the demo
	{ "pq1",		"pq1sci",			SCI_VERSION_NONE     },
	{ "pq",			"pq2",				SCI_VERSION_NONE     },
	// pq3 is the same
	// pq4 is the same
	{ "hq",			"qfg1",				SCI_VERSION_NONE     },	// QFG1 SCI0/EGA
	{ "glory",      "qfg1",             SCI_VERSION_0_LATE   },	// QFG1 SCI0/EGA
	{ "trial",		"qfg2",				SCI_VERSION_NONE     },
	{ "hq2demo",	"qfg2",				SCI_VERSION_NONE     },
	// rama is the same
	// TODO: distinguish the full version of rama from the demo
	{ "thegame",	"slater",			SCI_VERSION_NONE     },
	{ "sq1demo",	"sq1sci",			SCI_VERSION_NONE     },
	{ "sq1",		"sq1sci",			SCI_VERSION_NONE     },
	// sq3 is the same
	// sq4 is the same
	// sq5 is the same
	// sq6 is the same
	// TODO: distinguish the full version of SQ6 from the demo
	// torin is the same


	// TODO: SCI3 IDs

	{ "", "", SCI_VERSION_NONE }
};

/**
 * Converts the builtin Sierra game IDs to the ones we use in ScummVM
 * @param[in] gameId		The internal game ID
 * @param[in] gameFlags     The game's flags, which are adjusted accordingly for demos
 * @return					The equivalent ScummVM game id
 */
Common::String convertSierraGameId(Common::String sierraId, uint32 *gameFlags, ResourceManager &resMan) {
	// Convert the id to lower case, so that we match all upper/lower case variants.
	sierraId.toLowercase();

	// If the game has less than the expected scripts, it's a demo
	uint32 demoThreshold = 100;
	// ...but there are some exceptions
	if (sierraId == "brain" || sierraId == "lsl1" ||
		sierraId == "mg" || sierraId == "pq" ||
		sierraId == "jones" ||
		sierraId == "cardgames" || sierraId == "solitare" ||
		sierraId == "hoyle4")
		demoThreshold = 40;
	if (sierraId == "hoyle3")
		demoThreshold = 45;	// cnick-kq has 42 scripts. The actual hoyle 3 demo has 27.
	if (sierraId == "fp" || sierraId == "gk" || sierraId == "pq4")
		demoThreshold = 150;

	Common::List<ResourceId> resources = resMan.listResources(kResourceTypeScript, -1);
	if (resources.size() < demoThreshold) {
		*gameFlags |= ADGF_DEMO;

		// Crazy Nick's Picks
		if (sierraId == "lsl1" && resources.size() == 34)
			return "cnick-lsl";
		if (sierraId == "sq4" && resources.size() == 34)
			return "cnick-sq";
		if (sierraId == "hoyle3" && resources.size() == 42)
			return "cnick-kq";
		if (sierraId == "rh budget" && resources.size() == 39)
			return "cnick-longbow";
		// TODO: cnick-laurabow (the name of the game object contains junk)

		// Handle Astrochicken 1 (SQ3) and 2 (SQ4)
		if (sierraId == "sq3" && resources.size() == 20)
			return "astrochicken";
		if (sierraId == "sq4")
			return "msastrochicken";
	}

	if (sierraId == "torin" && resources.size() == 226)	// Torin's Passage demo
		*gameFlags |= ADGF_DEMO;

	for (const OldNewIdTableEntry *cur = s_oldNewTable; cur->oldId[0]; ++cur) {
		if (sierraId == cur->oldId) {
			// Distinguish same IDs via the SCI version
			if (cur->version != SCI_VERSION_NONE && cur->version != getSciVersion())
				continue;

			return cur->newId;
		}
	}

	if (sierraId == "glory") {
		// This could either be qfg1 VGA, qfg3 or qfg4 demo (all SCI1.1),
		// or qfg4 full (SCI2)
		// qfg1 VGA doesn't have view 1
		if (!resMan.testResource(ResourceId(kResourceTypeView, 1)))
			return "qfg1vga";

		// qfg4 full is SCI2
		if (getSciVersion() == SCI_VERSION_2)
			return "qfg4";

		// qfg4 demo has less than 50 scripts
		if (resources.size() < 50)
			return "qfg4";

		// Otherwise it's qfg3
		return "qfg3";
	}

	return sierraId;
}

#include "sci/detection_tables.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_EGA_UNDITHER,
		{
			_s("EGA undithering"),
			_s("Enable undithering in EGA games"),
			"disable_dithering",
			false
		}
	},

	{
		GAMEOPTION_PREFER_DIGITAL_SFX,
		{
			_s("Prefer digital sound effects"),
			_s("Prefer digital sound effects instead of synthesized ones"),
			"prefer_digitalsfx",
			true
		}
	},

	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens, instead of the ScummVM ones"),
			"originalsaveload",
			false
		}
	},

	{
		GAMEOPTION_FB01_MIDI,
		{
			_s("Use IMF/Yamaha FB-01 for MIDI output"),
			_s("Use an IBM Music Feature card or a Yamaha FB-01 FM synth module for MIDI output"),
			"native_fb01",
			false
		}
	},

	// Jones in the Fast Lane - CD audio tracks or resource.snd
	{
		GAMEOPTION_JONES_CDAUDIO,
		{
			_s("Use CD audio"),
			_s("Use CD audio instead of in-game audio, if available"),
			"use_cdaudio",
			true
		}
	},

	// KQ6 Windows - windows cursors
	{
		GAMEOPTION_KQ6_WINDOWS_CURSORS,
		{
			_s("Use Windows cursors"),
			_s("Use the Windows cursors (smaller and monochrome) instead of the DOS ones"),
			"windows_cursors",
			false
		}
	},

	// SQ4 CD - silver cursors
	{
		GAMEOPTION_SQ4_SILVER_CURSORS,
		{
			_s("Use silver cursors"),
			_s("Use the alternate set of silver cursors, instead of the normal golden ones"),
			"silver_cursors",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

/**
 * The fallback game descriptor used by the SCI engine's fallbackDetector.
 * Contents of this struct are overwritten by the fallbackDetector.
 */
static ADGameDescription s_fallbackDesc = {
	"",
	"",
	AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
	Common::UNK_LANG,
	Common::kPlatformPC,
	ADGF_NO_FLAGS,
	GUIO3(GAMEOPTION_PREFER_DIGITAL_SFX, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_FB01_MIDI)
};

static char s_fallbackGameIdBuf[256];

class SciMetaEngine : public AdvancedMetaEngine {
public:
	SciMetaEngine() : AdvancedMetaEngine(Sci::SciGameDescriptions, sizeof(ADGameDescription), s_sciGameTitles, optionsList) {
		_singleid = "sci";
	}

	virtual const char *getName() const {
		return "SCI [SCI0, SCI01, SCI10, SCI11"
#ifdef ENABLE_SCI32
			", SCI32"
#endif
			"]";
	}

	virtual const char *getOriginalCopyright() const {
		return "Sierra's Creative Interpreter (C) Sierra Online";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
	const ADGameDescription *fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const;
	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const;
	virtual void removeSaveState(const char *target, int slot) const;
	SaveStateDescriptor querySaveMetaInfos(const char *target, int slot) const;
};

Common::Language charToScummVMLanguage(const char c) {
	switch (c) {
	case 'F':
		return Common::FR_FRA;
	case 'S':
		return Common::ES_ESP;
	case 'I':
		return Common::IT_ITA;
	case 'G':
		return Common::DE_DEU;
	case 'J':
	case 'j':
		return Common::JA_JPN;
	case 'P':
		return Common::PT_BRA;
	default:
		return Common::UNK_LANG;
	}
}

const ADGameDescription *SciMetaEngine::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
	bool foundResMap = false;
	bool foundRes000 = false;

	// Set some defaults
	s_fallbackDesc.extra = "";
	s_fallbackDesc.language = Common::EN_ANY;
	s_fallbackDesc.flags = ADGF_NO_FLAGS;
	s_fallbackDesc.platform = Common::kPlatformPC;	// default to PC platform
	s_fallbackDesc.gameid = "sci";
	s_fallbackDesc.guioptions = GUIO3(GAMEOPTION_PREFER_DIGITAL_SFX, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_FB01_MIDI);

	if (allFiles.contains("resource.map") || allFiles.contains("Data1")
	    || allFiles.contains("resmap.001") || allFiles.contains("resmap.001")) {
		foundResMap = true;
	}

	// Determine if we got a CD version and set the CD flag accordingly, by checking for
	// resource.aud for SCI1.1 CD games, or audio001.002 for SCI1 CD games. We assume that
	// the file should be over 10MB, as it contains all the game speech and is usually
	// around 450MB+. The size check is for some floppy game versions like KQ6 floppy, which
	// also have a small resource.aud file
	if (allFiles.contains("resource.aud") || allFiles.contains("audio001.002")) {
		Common::FSNode file = allFiles.contains("resource.aud") ? allFiles["resource.aud"] :  allFiles["audio001.002"];
		Common::SeekableReadStream *tmpStream = file.createReadStream();
		if (tmpStream->size() > 10 * 1024 * 1024) {
			// We got a CD version, so set the CD flag accordingly
			s_fallbackDesc.flags |= ADGF_CD;
		}
		delete tmpStream;
	}

	if (allFiles.contains("resource.000") || allFiles.contains("resource.001")
		|| allFiles.contains("ressci.000") || allFiles.contains("ressci.001"))
		foundRes000 = true;

	// Data1 contains both map and volume for SCI1.1+ Mac games
	if (allFiles.contains("Data1")) {
		foundResMap = foundRes000 = true;
		 s_fallbackDesc.platform = Common::kPlatformMacintosh;
	}

	// Determine the game platform
	// The existence of any of these files indicates an Amiga game
	if (allFiles.contains("9.pat") || allFiles.contains("spal") ||
		allFiles.contains("patch.005") || allFiles.contains("bank.001"))
			s_fallbackDesc.platform = Common::kPlatformAmiga;

	// The existence of 7.pat or patch.200 indicates a Mac game
	if (allFiles.contains("7.pat") || allFiles.contains("patch.200"))
		s_fallbackDesc.platform = Common::kPlatformMacintosh;

	// The data files for Atari ST versions are the same as their DOS counterparts


	// If these files aren't found, it can't be SCI
	if (!foundResMap && !foundRes000) {
		return 0;
	}

	ResourceManager resMan;
	resMan.addAppropriateSources(fslist);
	resMan.init(true);
	// TODO: Add error handling.

#ifndef ENABLE_SCI32
	// Is SCI32 compiled in? If not, and this is a SCI32 game,
	// stop here
	if (getSciVersion() >= SCI_VERSION_2) {
		return (const ADGameDescription *)&s_fallbackDesc;
	}
#endif

	ViewType gameViews = resMan.getViewType();

	// Have we identified the game views? If not, stop here
	// Can't be SCI (or unsupported SCI views). Pinball Creep by sierra also uses resource.map/resource.000 files
	//  but doesnt share sci format at all, if we dont return 0 here we will detect this game as SCI
	if (gameViews == kViewUnknown) {
		return 0;
	}

	// Set the platform to Amiga if the game is using Amiga views
	if (gameViews == kViewAmiga)
		s_fallbackDesc.platform = Common::kPlatformAmiga;

	// Determine the game id
	Common::String sierraGameId = resMan.findSierraGameId();

	// If we don't have a game id, the game is not SCI
	if (sierraGameId.empty()) {
		return 0;
	}

	Common::String gameId = convertSierraGameId(sierraGameId, &s_fallbackDesc.flags, resMan);
	strncpy(s_fallbackGameIdBuf, gameId.c_str(), sizeof(s_fallbackGameIdBuf) - 1);
	s_fallbackGameIdBuf[sizeof(s_fallbackGameIdBuf) - 1] = 0;	// Make sure string is NULL terminated
	s_fallbackDesc.gameid = s_fallbackGameIdBuf;

	// Try to determine the game language
	// Load up text 0 and start looking for "#" characters
	// Non-English versions contain strings like XXXX#YZZZZ
	// Where XXXX is the English string, #Y a separator indicating the language
	// (e.g. #G for German) and ZZZZ is the translated text
	// NOTE: This doesn't work for games which use message instead of text resources
	// (like, for example, Eco Quest 1 and all SCI1.1 games and newer, e.g. Freddy Pharkas).
	// As far as we know, these games store the messages of each language in separate
	// resources, and it's not possible to detect that easily
	// Also look for "%J" which is used in japanese games
	Resource *text = resMan.findResource(ResourceId(kResourceTypeText, 0), 0);
	uint seeker = 0;
	if (text) {
		while (seeker < text->size) {
			if (text->data[seeker] == '#')  {
				if (seeker + 1 < text->size)
					s_fallbackDesc.language = charToScummVMLanguage(text->data[seeker + 1]);
				break;
			}
			if (text->data[seeker] == '%') {
				if ((seeker + 1 < text->size) && (text->data[seeker + 1] == 'J')) {
					s_fallbackDesc.language = charToScummVMLanguage(text->data[seeker + 1]);
					break;
				}
			}
			seeker++;
		}
	}


	// Fill in "extra" field

	// Is this an EGA version that might have a VGA pendant? Then we want
	// to mark it as such in the "extra" field.
	const bool markAsEGA = (gameViews == kViewEga && s_fallbackDesc.platform != Common::kPlatformAmiga
			&& getSciVersion() > SCI_VERSION_1_EGA_ONLY);

	const bool isDemo = (s_fallbackDesc.flags & ADGF_DEMO);
	const bool isCD = (s_fallbackDesc.flags & ADGF_CD);

	if (!isCD)
		s_fallbackDesc.guioptions = GUIO4(GUIO_NOSPEECH, GAMEOPTION_PREFER_DIGITAL_SFX, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_FB01_MIDI);

	if (gameId.hasSuffix("sci")) {
		s_fallbackDesc.extra = "SCI";

		// Differentiate EGA versions from the VGA ones, where needed
		if (markAsEGA)
			s_fallbackDesc.extra = "SCI/EGA";

		// Mark as demo.
		// Note: This overwrites the 'EGA' info, if it was previously set.
		if (isDemo)
			s_fallbackDesc.extra = "SCI/Demo";
	} else {
		if (markAsEGA)
			s_fallbackDesc.extra = "EGA";

		// Set "CD" and "Demo" as appropriate.
		// Note: This overwrites the 'EGA' info, if it was previously set.
		if (isDemo && isCD)
			s_fallbackDesc.extra = "CD Demo";
		else if (isDemo)
			s_fallbackDesc.extra = "Demo";
		else if (isCD)
			s_fallbackDesc.extra = "CD";
	}

	return &s_fallbackDesc;
}

bool SciMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const GameIdStrToEnum *g = s_gameIdStrToEnum;
	for (; g->gameidStr; ++g) {
		if (0 == strcmp(desc->gameid, g->gameidStr)) {
			*engine = new SciEngine(syst, desc, g->gameidEnum);
			return true;
		}
	}

	return false;
}

bool SciMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime);
}

bool SciEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime); // ||
		//(f == kSupportsSavingDuringRuntime);
		// We can't allow saving through ScummVM menu, because
		//  a) lots of games don't like saving everywhere (e.g. castle of dr. brain)
		//  b) some games even dont allow saving in certain rooms (e.g. lsl6)
		//  c) somehow some games even get mad when doing this (execstackbase was 1 all of a sudden in lsl3)
		//  d) for sci0/sci01 games we should at least wait till status bar got drawn, although this may not be enough
		// we can't make sure that the scripts are fine with us saving at a specific location, doing so may work sometimes
		//  and some other times it won't work.
}

SaveStateList SciMetaEngine::listSaves(const char *target) const {
	Common::SaveFileManager *saveFileMan = g_system->getSavefileManager();
	Common::StringArray filenames;
	Common::String pattern = target;
	pattern += ".???";

	filenames = saveFileMan->listSavefiles(pattern);
	sort(filenames.begin(), filenames.end());	// Sort (hopefully ensuring we are sorted numerically..)

	SaveStateList saveList;
	int slotNum = 0;
	for (Common::StringArray::const_iterator file = filenames.begin(); file != filenames.end(); ++file) {
		// Obtain the last 3 digits of the filename, since they correspond to the save slot
		slotNum = atoi(file->c_str() + file->size() - 3);

		if (slotNum >= 0 && slotNum <= 99) {
			Common::InSaveFile *in = saveFileMan->openForLoading(*file);
			if (in) {
				SavegameMetadata meta;
				if (!get_savegame_metadata(in, &meta)) {
					// invalid
					delete in;
					continue;
				}
				saveList.push_back(SaveStateDescriptor(slotNum, meta.name));
				delete in;
			}
		}
	}

	return saveList;
}

SaveStateDescriptor SciMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading(fileName);

	if (in) {
		SavegameMetadata meta;
		if (!get_savegame_metadata(in, &meta)) {
			// invalid
			delete in;

			SaveStateDescriptor desc(slot, "Invalid");
			return desc;
		}

		SaveStateDescriptor desc(slot, meta.name);

		Graphics::Surface *const thumbnail = Graphics::loadThumbnail(*in);
		desc.setThumbnail(thumbnail);

		int day = (meta.saveDate >> 24) & 0xFF;
		int month = (meta.saveDate >> 16) & 0xFF;
		int year = meta.saveDate & 0xFFFF;

		desc.setSaveDate(year, month, day);

		int hour = (meta.saveTime >> 16) & 0xFF;
		int minutes = (meta.saveTime >> 8) & 0xFF;

		desc.setSaveTime(hour, minutes);

		desc.setPlayTime(meta.playTime * 1000);

		delete in;

		return desc;
	}

	return SaveStateDescriptor();
}

int SciMetaEngine::getMaximumSaveSlot() const { return 99; }

void SciMetaEngine::removeSaveState(const char *target, int slot) const {
	Common::String fileName = Common::String::format("%s.%03d", target, slot);
	g_system->getSavefileManager()->removeSavefile(fileName);
}

Common::Error SciEngine::loadGameState(int slot) {
	Common::String fileName = Common::String::format("%s.%03d", _targetName.c_str(), slot);
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::SeekableReadStream *in = saveFileMan->openForLoading(fileName);

	if (in) {
		// found a savegame file
		gamestate_restore(_gamestate, in);
		delete in;
	}

	if (_gamestate->r_acc != make_reg(0, 1)) {
		return Common::kNoError;
	} else {
		warning("Restoring gamestate '%s' failed", fileName.c_str());
		return Common::kUnknownError;
	}
}

Common::Error SciEngine::saveGameState(int slot, const Common::String &desc) {
	Common::String fileName = Common::String::format("%s.%03d", _targetName.c_str(), slot);
	Common::SaveFileManager *saveFileMan = g_engine->getSaveFileManager();
	Common::OutSaveFile *out = saveFileMan->openForSaving(fileName);
	const char *version = "";
	if (!out) {
		warning("Opening savegame \"%s\" for writing failed", fileName.c_str());
		return Common::kWritingFailed;
	}

	if (!gamestate_save(_gamestate, out, desc, version)) {
		warning("Saving the game state to '%s' failed", fileName.c_str());
		return Common::kWritingFailed;
	} else {
		out->finalize();
		if (out->err()) {
			warning("Writing the savegame failed");
			return Common::kWritingFailed;
		}
		delete out;
	}

	return Common::kNoError;
}

// Before enabling the load option in the ScummVM menu, the main game loop must
// have run at least once. When the game loop runs, kGameIsRestarting is invoked,
// thus the speed throttler is initialized. Hopefully fixes bug #3565505.

bool SciEngine::canLoadGameStateCurrently() {
	return !_gamestate->executionStackBase && (_gamestate->_throttleLastTime > 0 || _gamestate->_throttleTrigger);
}

bool SciEngine::canSaveGameStateCurrently() {
	return !_gamestate->executionStackBase && (_gamestate->_throttleLastTime > 0 || _gamestate->_throttleTrigger);
}

} // End of namespace Sci

#if PLUGIN_ENABLED_DYNAMIC(SCI)
	REGISTER_PLUGIN_DYNAMIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(SCI, PLUGIN_TYPE_ENGINE, Sci::SciMetaEngine);
#endif
