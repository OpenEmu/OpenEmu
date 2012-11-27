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

#ifndef SCUMM_DETECTION_TABLES_H
#define SCUMM_DETECTION_TABLES_H

#include "engines/obsolete.h"
#include "common/gui_options.h"
#include "common/rect.h"
#include "common/util.h"

#include "scumm/detection.h"
#include "scumm/scumm.h"

#include "scumm/scumm-md5.h"


namespace Scumm {

#pragma mark -
#pragma mark --- Data types & constants ---
#pragma mark -

#define UNK Common::kPlatformUnknown

#pragma mark -
#pragma mark --- Tables ---
#pragma mark -

/**
 * This table contains list of directories which could contain game data
 * and which should be looked into during detection.
 */
static const char *const directoryGlobs[] = {
	"rooms *",  // Mac version of indy3/loom
	0
};


/**
 * This table contains all game IDs supported by the SCUMM engine, and maps
 * them to the full humand readable game name.
 */
static const PlainGameDescriptor gameDescriptions[] = {
	{ "atlantis", "Indiana Jones and the Fate of Atlantis" },
	{ "indy3", "Indiana Jones and the Last Crusade" },
	{ "loom", "Loom" },
	{ "maniac", "Maniac Mansion" },
	{ "monkey", "The Secret of Monkey Island" },
	{ "monkey2", "Monkey Island 2: LeChuck's Revenge" },
	{ "pass", "Passport to Adventure" },
	{ "samnmax", "Sam & Max Hit the Road" },
	{ "tentacle", "Day of the Tentacle" },
	{ "zak", "Zak McKracken and the Alien Mindbenders" },
	{ "indyloom", "Indiana Jones and the Last Crusade & Loom" },
	{ "indyzak", "Indiana Jones and the Last Crusade & Zak McKracken" },
	{ "zakloom", "Zak McKracken & Loom" },

#ifdef ENABLE_SCUMM_7_8
	{ "ft", "Full Throttle" },
	{ "dig", "The Dig" },
	{ "comi", "The Curse of Monkey Island" },
#endif

	{ "activity", "Putt-Putt & Fatty Bear's Activity Pack" },
	{ "brstorm", "Bear Stormin'" },
	{ "fbear", "Fatty Bear's Birthday Surprise" },
	{ "fbpack", "Fatty Bear's Fun Pack" },
	{ "funpack", "Putt-Putt's Fun Pack" },
	{ "puttmoon", "Putt-Putt Goes to the Moon" },
	{ "puttputt", "Putt-Putt Joins the Parade" },

#ifdef ENABLE_HE
#ifdef USE_RGB_COLOR
	{ "arttime", "Blue's Art Time Activities" },
	{ "baseball2001", "Backyard Baseball 2001" },
	{ "Baseball2003", "Backyard Baseball 2003" },
	{ "basketball", "Backyard Basketball" },
	{ "football2002", "Backyard Football 2002" },
	{ "freddicove", "Freddi Fish 5: The Case of the Creature of Coral Cove" },
	{ "moonbase", "Moonbase Commander" },
	{ "pjgames", "Pajama Sam: Games to Play on Any Day" },
	{ "readtime", "Blue's Reading Time Activities" },
	{ "Soccer2004", "Backyard Soccer 2004" },
	{ "SoccerMLS", "Backyard Soccer MLS Edition" },
	{ "spyozon", "SPY Fox 3: Operation Ozone" },
#endif
	{ "airport", "Let's Explore the Airport with Buzzy" },
	{ "balloon", "Putt-Putt and Pep's Balloon-O-Rama" },
	{ "baseball", "Backyard Baseball" },
	{ "Blues123Time", "Blue's 123 Time Activities" },
	{ "BluesABCTime", "Blue's ABC Time Activities" },
	{ "BluesBirthday", "Blue's Birthday Adventure" },
	{ "BluesTreasureHunt", "Blue's Treasure Hunt" },
	{ "catalog", "Humongous Interactive Catalog" },
	{ "chase", "SPY Fox in Cheese Chase" },
	{ "dog", "Putt-Putt and Pep's Dog on a Stick" },
	{ "farm", "Let's Explore the Farm with Buzzy" },
	{ "football", "Backyard Football" },
	{ "freddi", "Freddi Fish 1: The Case of the Missing Kelp Seeds" },
	{ "freddi2", "Freddi Fish 2: The Case of the Haunted Schoolhouse" },
	{ "freddi3", "Freddi Fish 3: The Case of the Stolen Conch Shell" },
	{ "freddi4", "Freddi Fish 4: The Case of the Hogfish Rustlers of Briny Gulch" },
	{ "FreddisFunShop", "Freddi Fish's One-Stop Fun Shop" },
	{ "jungle", "Let's Explore the Jungle with Buzzy" },
	{ "lost", "Pajama Sam's Lost & Found" },
	{ "maze", "Freddi Fish and Luther's Maze Madness" },
	{ "mustard", "SPY Fox in Hold the Mustard" },
	{ "pajama", "Pajama Sam 1: No Need to Hide When It's Dark Outside" },
	{ "pajama2", "Pajama Sam 2: Thunder and Lightning Aren't so Frightening" },
	{ "pajama3", "Pajama Sam 3: You Are What You Eat From Your Head to Your Feet" },
	{ "puttcircus", "Putt-Putt Joins the Circus" },
	{ "puttrace", "Putt-Putt Enters the Race" },
	{ "PuttsFunShop", "Putt-Putt's One-Stop Fun Shop" },
	{ "putttime", "Putt-Putt Travels Through Time" },
	{ "puttzoo", "Putt-Putt Saves the Zoo" },
	{ "SamsFunShop", "Pajama Sam's One-Stop Fun Shop" },
	{ "soccer", "Backyard Soccer" },
	{ "socks", "Pajama Sam's Sock Works" },
	{ "spyfox", "SPY Fox 1: Dry Cereal" },
	{ "spyfox2", "SPY Fox 2: Some Assembly Required" },
	{ "thinker1", "Big Thinkers First Grade" },
	{ "thinkerk", "Big Thinkers Kindergarten" },
	{ "water", "Freddi Fish and Luther's Water Worries" },
#endif
	{ 0, 0 }
};

/**
 * Conversion table mapping old obsolete game IDs to the
 * corresponding new game ID and platform combination.
 */
static const Engines::ObsoleteGameID obsoleteGameIDsTable[] = {
	{"bluesabctimedemo", "bluesabctime", UNK},
	{"BluesBirthdayDemo", "BluesBirthday", UNK},
	{"comidemo", "comi", UNK},
	{"digdemo", "dig", UNK},
	{"digdemoMac", "dig", Common::kPlatformMacintosh},
	{"dottdemo", "tentacle", UNK},
	{"fate", "atlantis", UNK},
	{"ftMac", "ft",  Common::kPlatformMacintosh},
	{"ftpcdemo", "ft", UNK},
	{"ftdemo", "ft",  Common::kPlatformMacintosh},
	{"game", "monkey", UNK},
	{"indy3ega", "indy3", UNK},
	{"indy3towns", "indy3", Common::kPlatformFMTowns},
	{"indy4", "atlantis", Common::kPlatformFMTowns},
	{"indydemo", "atlantis", Common::kPlatformFMTowns},
	{"loomcd", "loom", UNK},
	{"loomTowns", "loom", Common::kPlatformFMTowns},
	{"mi2demo", "monkey2", UNK},
	{"monkey1", "monkey", UNK},
	{"monkeyEGA", "monkey", UNK},
	{"monkeyVGA", "monkey", UNK},
	{"playfate", "atlantis", UNK},
	{"samnmax-alt", "samnmax", UNK},
	{"samnmaxMac", "samnmax", Common::kPlatformMacintosh},
	{"samdemo", "samnmax", UNK},
	{"samdemoMac", "samnmax", Common::kPlatformMacintosh},
	{"snmdemo", "samnmax", UNK},
	{"snmidemo", "samnmax", UNK},
	{"tentacleMac", "tentacle", Common::kPlatformMacintosh},
	{"zakTowns", "zak", Common::kPlatformFMTowns},
	{NULL, NULL, UNK}
};

// The following table contains information about variants of our various
// games. We index into it with help of md5table (from scumm-md5.h), to find
// the correct GameSettings for a given game variant.
//
// The first listed variant is assumed to be the 'default' variant -- i.e. the
// variant that gets used when no explicit variant code has been specified.
//
// Note #1: Only set 'platform' to a value different from UNK if that game
// variant really *only* exists for that given platform. In all other cases,
// the correct platform will be determined via the MD5 table or derived from
// the filename.
//
// Note #2: Make sure that all variants for a given gameid are in sequence with
// no gaps. Some code may rely on this and stop searching the table early as
// soon as the gameid changes.
//
// Note #3: Use 0 (zero) for the variant field *if and only if* the game has
// only a single unique variant. This is used to help the detector quickly
// decide whether it has to worry about distinguishing multiple variants or not.
static const GameSettings gameVariantsTable[] = {
	{"maniac", "Apple II",   0, GID_MANIAC, 0, 0, MDT_APPLEIIGS, 0, Common::kPlatformApple2GS, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"maniac", "C64",        0, GID_MANIAC, 0, 0, MDT_C64, 0, Common::kPlatformC64, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI) },
	{"maniac", "V1",      "v1", GID_MANIAC, 1, 0, MDT_PCSPK | MDT_PCJR, 0, Common::kPlatformPC, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"maniac", "V1 Demo", "v1", GID_MANIAC, 1, 0, MDT_PCSPK | MDT_PCJR, GF_DEMO, Common::kPlatformPC, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"maniac", "NES",        0, GID_MANIAC, 1, 0, MDT_NONE,  0, Common::kPlatformNES, GUIO3(GUIO_NOSPEECH, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"maniac", "V2",      "v2", GID_MANIAC, 2, 0, MDT_PCSPK | MDT_PCJR, 0, UNK, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"maniac", "V2 Demo", "v2", GID_MANIAC, 2, 0, MDT_PCSPK | MDT_PCJR, GF_DEMO, Common::kPlatformPC, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},

	{"zak", "V1",       "v1", GID_ZAK, 1, 0, MDT_PCSPK | MDT_PCJR, 0, UNK, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"zak", "V2",       "v2", GID_ZAK, 2, 0, MDT_PCSPK | MDT_PCJR, 0, UNK, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"zak", "FM-TOWNS",    0, GID_ZAK, 3, 0, MDT_TOWNS, GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns, GUIO4(GUIO_NOSPEECH, GUIO_NOMIDI, GUIO_MIDITOWNS, GUIO_NOASPECT)},
	{"zakloom", "FM-TOWNS",    0, GID_ZAK, 3, 0, MDT_TOWNS, GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns, GUIO4(GUIO_NOSPEECH, GUIO_NOMIDI, GUIO_MIDITOWNS, GUIO_NOASPECT)},
	{"indyloom", "FM-TOWNS",    0, GID_ZAK, 3, 0, MDT_TOWNS, GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns, GUIO4(GUIO_NOSPEECH, GUIO_NOMIDI, GUIO_MIDITOWNS, GUIO_NOASPECT)},
	{"indyzak", "FM-TOWNS",    0, GID_ZAK, 3, 0, MDT_TOWNS, GF_OLD256 | GF_AUDIOTRACKS, Common::kPlatformFMTowns, GUIO4(GUIO_NOSPEECH, GUIO_NOMIDI, GUIO_MIDITOWNS, GUIO_NOASPECT)},

	{"indy3", "EGA",      "ega", GID_INDY3, 3, 0, MDT_PCSPK | MDT_PCJR | MDT_CMS | MDT_ADLIB, 0, UNK, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"indy3", "No AdLib", "ega", GID_INDY3, 3, 0, MDT_PCSPK | MDT_PCJR,             0, UNK, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"indy3", "VGA",      "vga", GID_INDY3, 3, 0, MDT_PCSPK | MDT_PCJR | MDT_CMS | MDT_ADLIB, GF_OLD256 | GF_FEW_LOCALS,                  Common::kPlatformPC, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"indy3", "FM-TOWNS",     0, GID_INDY3, 3, 0, MDT_TOWNS,             GF_OLD256 | GF_FEW_LOCALS | GF_AUDIOTRACKS, Common::kPlatformFMTowns, GUIO4(GUIO_NOSPEECH, GUIO_NOMIDI, GUIO_MIDITOWNS, GUIO_NOASPECT)},

	{"loom", "EGA",      "ega", GID_LOOM, 3, 0, MDT_PCSPK | MDT_PCJR | MDT_CMS | MDT_ADLIB | MDT_MIDI | MDT_PREFER_MT32, 0, UNK, GUIO1(GUIO_NOSPEECH)},
	{"loom", "No AdLib", "ega", GID_LOOM, 3, 0, MDT_PCSPK | MDT_PCJR | MDT_CMS,                        0, UNK, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
#ifdef USE_RGB_COLOR
	{"loom", "PC-Engine",    0, GID_LOOM, 3, 0, MDT_NONE,                         GF_AUDIOTRACKS | GF_OLD256 | GF_16BIT_COLOR, Common::kPlatformPCEngine, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
#endif
	{"loom", "FM-TOWNS",     0, GID_LOOM, 3, 0, MDT_TOWNS,                        GF_AUDIOTRACKS | GF_OLD256, Common::kPlatformFMTowns, GUIO4(GUIO_NOSPEECH, GUIO_NOMIDI, GUIO_MIDITOWNS, GUIO_NOASPECT)},
	{"loom", "VGA",      "vga", GID_LOOM, 4, 0, MDT_NONE,                         GF_AUDIOTRACKS,             Common::kPlatformPC, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},

	{"pass", 0, 0, GID_PASS, 4, 0, MDT_PCSPK | MDT_PCJR | MDT_CMS | MDT_ADLIB, GF_16COLOR, Common::kPlatformPC, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},

	{"monkey", "VGA",      "vga", GID_MONKEY_VGA, 4, 0, MDT_PCSPK | MDT_PCJR | MDT_CMS | MDT_ADLIB | MDT_MIDI | MDT_PREFER_MT32, 0, UNK, GUIO1(GUIO_NOSPEECH)},
	{"monkey", "EGA",      "ega", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_PCJR | MDT_CMS | MDT_ADLIB | MDT_MIDI | MDT_PREFER_MT32, GF_16COLOR,     Common::kPlatformPC, GUIO1(GUIO_NOSPEECH)},
	{"monkey", "No AdLib", "ega", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_PCJR,                        GF_16COLOR,     Common::kPlatformAtariST, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"monkey", "Demo",     "ega", GID_MONKEY_EGA, 4, 0, MDT_PCSPK | MDT_PCJR | MDT_CMS | MDT_ADLIB,            GF_16COLOR,     Common::kPlatformPC, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"monkey", "CD",           0, GID_MONKEY,     5, 0, MDT_ADLIB,                        GF_AUDIOTRACKS, UNK, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},
	{"monkey", "FM-TOWNS",     0, GID_MONKEY,     5, 0, MDT_TOWNS,                        GF_AUDIOTRACKS, Common::kPlatformFMTowns, GUIO4(GUIO_NOSPEECH, GUIO_NOMIDI, GUIO_MIDITOWNS, GUIO_NOASPECT)},
	{"monkey", "SEGA",         0, GID_MONKEY,     5, 0, MDT_NONE,                         GF_AUDIOTRACKS, Common::kPlatformSegaCD, GUIO2(GUIO_NOSPEECH, GUIO_NOMIDI)},

	{"monkey2",  "", 0, GID_MONKEY2,  5, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI | MDT_PREFER_MT32, 0, UNK, GUIO1(GUIO_NOSPEECH)},
	{"monkey2", "FM-TOWNS", 0, GID_MONKEY2,  5, 0, MDT_PCSPK | MDT_TOWNS | MDT_ADLIB | MDT_MIDI | MDT_PREFER_MT32, 0, Common::kPlatformFMTowns, GUIO5(GUIO_NOSPEECH, GUIO_MIDITOWNS, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_NOASPECT)},

	{"atlantis", "", 0, GID_INDY4,    5, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI | MDT_PREFER_MT32, 0, UNK, GUIO0()},
	{"atlantis", "Floppy", 0, GID_INDY4,    5, 0, MDT_PCSPK | MDT_ADLIB | MDT_MIDI | MDT_PREFER_MT32, 0, UNK, GUIO1(GUIO_NOSPEECH)},
	{"atlantis", "FM-TOWNS", 0, GID_INDY4,    5, 0, MDT_TOWNS | MDT_ADLIB | MDT_MIDI | MDT_PREFER_MT32, 0, Common::kPlatformFMTowns, GUIO4(GUIO_MIDITOWNS, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_NOASPECT)},

	{"tentacle", "", 0, GID_TENTACLE, 6, 0, MDT_ADLIB | MDT_MIDI | MDT_PREFER_GM, GF_USE_KEY, UNK, GUIO0()},
	{"tentacle", "Floppy", 0, GID_TENTACLE, 6, 0, MDT_ADLIB | MDT_MIDI | MDT_PREFER_GM, GF_USE_KEY, UNK, GUIO1(GUIO_NOSPEECH)},

	{"samnmax",  "", 0, GID_SAMNMAX,  6, 0, MDT_ADLIB | MDT_MIDI | MDT_PREFER_GM, GF_USE_KEY, UNK, GUIO0()},
	{"samnmax",  "Floppy", 0, GID_SAMNMAX,  6, 0, MDT_ADLIB | MDT_MIDI | MDT_PREFER_GM, GF_USE_KEY, UNK, GUIO1(GUIO_NOSPEECH)},

#ifdef ENABLE_SCUMM_7_8
	{"ft",       0, 0, GID_FT,  7, 0, MDT_NONE, 0, UNK, GUIO1(GUIO_NOMIDI)},

	{"dig",      0, 0, GID_DIG, 7, 0, MDT_NONE, 0, UNK, GUIO1(GUIO_NOMIDI)},

	{"comi",     0, 0, GID_CMI, 8, 0, MDT_NONE, 0, Common::kPlatformWindows, GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)},
#endif

	// Humongous Entertainment Scumm Version 6
	{"activity", "", 0, GID_HEGAME, 6, 62, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK, GUIO1(GUIO_NOLAUNCHLOAD)},
	{"funpack",  0, 0, GID_FUNPACK, 6, 62, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK, GUIO1(GUIO_NOLAUNCHLOAD)},
	{"fbpack",   0, 0, GID_HEGAME,  6, 62, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK, GUIO1(GUIO_NOLAUNCHLOAD)},

	{"brstorm", 0, 0, GID_FBEAR, 6, 62, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK, GUIO1(GUIO_NOLAUNCHLOAD)},
	{"fbear", "HE 62", 0, GID_FBEAR, 6, 62, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK, GUIO1(GUIO_NOLAUNCHLOAD)},
	{"fbear", "HE 70", 0, GID_FBEAR, 6, 70, MDT_NONE,             GF_USE_KEY, Common::kPlatformWindows, GUIO2(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI)},

	{"puttmoon", "", 0, GID_PUTTMOON, 6, 62, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK, GUIO1(GUIO_NOLAUNCHLOAD)},
	{"puttmoon", "HE 70", 0, GID_PUTTMOON, 6, 70, MDT_NONE,             GF_USE_KEY, Common::kPlatformWindows, GUIO2(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI)},

	{"puttputt", "HE 60", 0, GID_HEGAME,   6, 60, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK, GUIO1(GUIO_NOLAUNCHLOAD)},
	{"puttputt", "HE 61", 0, GID_HEGAME,   6, 61, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK, GUIO1(GUIO_NOLAUNCHLOAD)},
	{"puttputt", "HE 62", 0, GID_HEGAME,   6, 62, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK, GUIO1(GUIO_NOLAUNCHLOAD)},
	{"puttputt", "Demo",  0, GID_PUTTDEMO, 6, 60, MDT_ADLIB | MDT_MIDI, GF_USE_KEY, UNK, GUIO1(GUIO_NOLAUNCHLOAD)},

	// The following are meant to be generic HE game variants and as such do
	// not specify a game ID. Make sure that these are last in the table, else
	// they'll override more specific entries that follow later on.
	{"", "HE 70",   0, GID_HEGAME, 6,  70, MDT_NONE, GF_USE_KEY, UNK, GUIO2(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI)},

#ifdef ENABLE_HE
	// HE CUP demos
	{"", "HE CUP",  0, GID_HECUP,  6, 200, MDT_NONE, 0, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOSPEECH)},

	// Humongous Entertainment Scumm Version 7.1
	// The first version to use 640x480 resolution and wizImages
	// There are also 7.1 versions of freddemo, airdemo and farmdemo
	{"catalog", "", 0, GID_HEGAME, 6, 71, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"freddi", "", 0, GID_HEGAME, 6, 71, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Humongous Entertainment Scumm Version 7.2
	{"airport", "", 0, GID_HEGAME, 6, 72, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Changed o_getResourceSize to cover all resource types
	{"farm", "", 0, GID_HEGAME, 6, 73, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"puttzoo", "", 0, GID_HEGAME, 6, 73, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Added VAR_PLATFORM variable
	{"jungle", "", 0, GID_HEGAME, 6, 74, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Humongous Entertainment Scumm Version 8.0 ?  Scummsrc.80
	{"freddi2", "", 0, GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"pajama", "", 0, GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	{"balloon", "", 0, GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"dog", "", 0, GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"maze", "", 0, GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	{"water", "",      0, GID_HEGAME, 6, 80, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// condMaskCode value changed in setUserCondition & setTalkCondition
	{"putttime", "", 0, GID_HEGAME, 6, 85, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"socks", "", 0, GID_HEGAME, 6, 85, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Humongous Entertainment Scumm Version 9.0 ?  Scummsys.90
	{"baseball", "", 0, GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"thinkerk", "", 0, GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"thinker1", "", 0, GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"spyfox", "", 0, GID_HEGAME, 6, 90, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	{"freddi3", "", 0, GID_FREDDI3, 6, 90, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"freddi3", "HE 99", 0, GID_FREDDI3, 6, 99, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Humongous Entertainment Scumm Version 9.5 ?  Scummsys.95
	{"pajama2", "", 0, GID_HEGAME, 6, 95, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"chase", "", 0, GID_HEGAME, 6, 95, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Humongous Entertainment Scumm Version 9.8 ?  Scummsys.98
	// these and later games can easily be identified by the .(a) file instead of a .he1
	// and INIB chunk in the .he0
	{"lost", "", 0, GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	{"puttrace", "HE 98",   0, GID_PUTTRACE, 6, 98, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"puttrace", "HE 98.5", 0, GID_PUTTRACE, 6, 98, MDT_NONE, GF_USE_KEY | GF_HE_985, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"puttrace", "HE 99",   0, GID_PUTTRACE, 6, 99, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	{"bluesabctime", "", 0, GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"BluesBirthday", "", 0, GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"BluesBirthday", "Red", 0, GID_BIRTHDAYRED, 6, 98, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"BluesBirthday", "Yellow", 0, GID_BIRTHDAYYELLOW, 6, 98, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"soccer", "", 0, GID_SOCCER, 6, 98, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Global scripts increased to 2048
	{"blues123time", "", 0, GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY | GF_HE_985, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"freddi4", "",       0, GID_HEGAME, 6, 98, MDT_NONE, GF_USE_KEY | GF_HE_985, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"freddi4", "unenc",  0, GID_HEGAME, 6, 98, MDT_NONE,              GF_HE_985, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Humongous Entertainment Scumm Version 9.9 ?  Scummsys.99
	{"football", 0, 0, GID_FOOTBALL, 6, 99, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"pajama3", 0, 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"puttcircus", 0, 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"spyfox2", 0, 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"mustard", 0, 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Added the use of fonts
	{"FreddisFunShop", 0, 0, GID_FUNSHOP, 6, 99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"SamsFunShop", 0, 0, GID_FUNSHOP, 6, 99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"PuttsFunShop", 0, 0, GID_FUNSHOP, 6, 99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Added the use of smacker videos
	{"BluesTreasureHunt", 0, 0, GID_TREASUREHUNT, 6, 99, MDT_NONE, GF_HE_LOCALIZED | GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

#ifdef USE_RGB_COLOR
	// Added 16bit color
	{"arttime", 0, 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"baseball2001", 0, 0, GID_BASEBALL2001, 6, 99, MDT_NONE, GF_USE_KEY | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"readtime", 0, 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"SoccerMLS", 0, 0, GID_SOCCERMLS, 6, 99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"spyozon", 0, 0, GID_HEGAME, 6, 99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	{"freddicove", "",       0, GID_HEGAME, 6,  99, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"freddicove", "unenc",  0, GID_HEGAME, 6,  99, MDT_NONE,              GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"freddicove", "HE 100", 0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Restructured the Scumm engine
	{"pjgames", 0, 0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY | GF_HE_LOCALIZED | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// Added the use of bink videos
	{"Soccer2004", 0, 0, GID_SOCCER2004, 6, 100, MDT_NONE, GF_USE_KEY | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// U32 code required, for testing only
	{"moonbase", 0, 0, GID_MOONBASE, 6, 100, MDT_NONE, GF_USE_KEY | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"moonbase", "Demo", 0, GID_MOONBASE, 6, 100, MDT_NONE, GF_USE_KEY | GF_16BIT_COLOR | GF_DEMO, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},

	// HE100 games, which use older o72_debugInput code
	{"Baseball2003", 0, 0, GID_BASEBALL2003, 6, 101, MDT_NONE, GF_USE_KEY | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"basketball", 0, 0, GID_BASKETBALL, 6, 101, MDT_NONE, GF_USE_KEY| GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"football2002", 0, 0, GID_FOOTBALL2002, 6, 101, MDT_NONE, GF_USE_KEY | GF_16BIT_COLOR, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
#endif

	// The following are meant to be generic HE game variants and as such do
	// not specify a game ID. Make sure that these are last in the table, else
	// they'll override more specific entries that follow later on.
	{"", "HE 71",   0, GID_HEGAME, 6,  71, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 72",   0, GID_HEGAME, 6,  72, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 73",   0, GID_HEGAME, 6,  73, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 74",   0, GID_HEGAME, 6,  74, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 80",   0, GID_HEGAME, 6,  80, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 85",   0, GID_HEGAME, 6,  85, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 90",   0, GID_HEGAME, 6,  90, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 95",   0, GID_HEGAME, 6,  95, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 98",   0, GID_HEGAME, 6,  98, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 98.5", 0, GID_HEGAME, 6,  98, MDT_NONE, GF_USE_KEY | GF_HE_985, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 99",   0, GID_HEGAME, 6,  99, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 100",  0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
	{"", "HE 101",  0, GID_HEGAME, 6, 100, MDT_NONE, GF_USE_KEY, UNK, GUIO3(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI, GUIO_NOASPECT)},
#endif
	{NULL, NULL, 0, 0, 0, MDT_NONE, 0, 0, UNK, 0}
};

using Common::UNK_LANG;

// The following describes how Fingolfin thinks this table might be used one day;
// this is work in progress, so read this with a salt of grain...
//
// The following table maps gameids to possible filename variants for that game.
// This information is used by the detector to determin possible "detect files".
// It is also later used by the engine creation code to verify the game to be
// launched is present. Finally, the correct GameFilenamePattern entry is passed on
// to the engine which uses it to locate the files for the game.
//
// The table is augmented by platform/language/variant information where applicable.
//
// Note: Setting variant to 0 means "don't care", while setting it to ""
// (i.e. an empty string) means "use the default variant".
static const GameFilenamePattern gameFilenamesTable[] = {
	{ "maniac", "%02d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "maniac", "%02d.MAN", kGenRoomNum, UNK_LANG, UNK, "V1 Demo" },
	{ "maniac", "maniac1.d64", kGenUnchanged, UNK_LANG, Common::kPlatformC64, "C64" },   // ... and maniac2.d64
	{ "maniac", "maniac1.dsk", kGenUnchanged, UNK_LANG, Common::kPlatformApple2GS, "Apple II" },   // ... and maniac2.dsk
	{ "maniac", "Maniac Mansion (E).prg", kGenUnchanged, Common::EN_GRB, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (F).prg", kGenUnchanged, Common::FR_FRA, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (SW).prg", kGenUnchanged, Common::SE_SWE, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (U).prg", kGenUnchanged, Common::EN_USA, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (G).prg", kGenUnchanged, Common::DE_DEU, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (I).prg", kGenUnchanged, Common::IT_ITA, Common::kPlatformNES, "NES" },
	{ "maniac", "Maniac Mansion (Sp).prg", kGenUnchanged, Common::ES_ESP, Common::kPlatformNES, "NES" },

	{ "zak", "%02d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "zak", "zak1.d64", kGenUnchanged, UNK_LANG, Common::kPlatformC64, "V1" },         // ... and zak2.d64

	{ "indy3", "%02d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },

	{ "indyloom", "%02d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "indyzak", "%02d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "zakloom", "%02d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },

	{ "loom", "%02d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },
	{ "loom", "%03d.LFL", kGenRoomNum, UNK_LANG, UNK, "VGA" },	// Loom CD

	{ "pass", "%03d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },

	{ "monkey", "%03d.LFL", kGenRoomNum, UNK_LANG, UNK, 0 },		// EGA & VGA versions
	{ "monkey", "monkey.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "monkey", "monkey1.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "monkey", "monkeyk.%03d", kGenDiskNum, Common::JA_JPN, Common::kPlatformFMTowns, "FM-TOWNS" },
	{ "monkey", "game.%03d", kGenDiskNum, UNK_LANG, Common::kPlatformSegaCD, "SEGA" }, // SegaCD

	{ "monkey2", "monkey2.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "monkey2", "mi2demo.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },

	{ "atlantis", "atlantis.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "fate.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "playfate.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "atlantis", "indy4.%03d", kGenDiskNum, Common::JA_JPN, Common::kPlatformFMTowns, "FM-TOWNS" },
	{ "atlantis", "indydemo.%03d", kGenDiskNum, Common::JA_JPN, Common::kPlatformFMTowns, "FM-TOWNS" },
	{ "atlantis", "Fate of Atlantis Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "tentacle", "tentacle.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "tentacle", "dottdemo.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "tentacle", "Day of the Tentacle Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "tentacle", "Day of the Tentacle Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "samnmax", "samnmax.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "samnmax.sm%d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "Sam & Max Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "samnmax", "Sam & Max Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "samnmax", "ramnmax.%03d", kGenDiskNum, Common::RU_RUS, UNK, 0 }, // Used in some releases of Russian Sam'n'Max
	{ "samnmax", "samdemo.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "snmdemo.sm%d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "snmidemo.%03d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "samnmax", "sdemo.sm%d", kGenDiskNum, Common::DE_DEU, UNK, 0 },

#ifdef ENABLE_SCUMM_7_8
	{ "dig", "dig.la%d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "dig", "thedig.la%d", kGenDiskNum, UNK_LANG, UNK, "Demo" }, // Used by an alternate version of the demo
	{ "dig", "The Dig Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "dig", "The Dig Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, "Demo" },

	{ "ft", "ft.la%d", kGenDiskNum, UNK_LANG, UNK, 0 },
	{ "ft", "ft.%03d", kGenDiskNum, UNK_LANG, UNK, "Demo" },    // Used by PC version of Full Throttle demo
	{ "ft", "ftdemo.la%d", kGenDiskNum, UNK_LANG, UNK, "Demo" },
	{ "ft", "Full Throttle Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "ft", "Full Throttle Demo Data", kGenUnchanged, UNK_LANG, Common::kPlatformMacintosh, "Demo" },
	{ "ft", "Vollgas Data", kGenUnchanged, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "ft", "Vollgas Demo Data", kGenUnchanged, Common::DE_DEU, Common::kPlatformMacintosh, "Demo" },

	{ "comi", "comi.la%d", kGenDiskNum, UNK_LANG, UNK, 0 },
#endif

	{ "activity", "activity", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "activity", "Putt & Fatty's Actpack", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "fbpack", "fbpack", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "funpack", "funpack", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "brstorm", "brstorm", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "fbear", "fbear", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "fbear", "fbdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "fbear", "Fatty Bear Demo", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "fbear", "Fatty Bear", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "fbear", "jfbear", kGenHEPC, Common::JA_JPN, Common::kPlatform3DO, 0 },

	{ "puttmoon", "puttmoon", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttmoon", "moondemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttmoon", "Putt-Putt Moon Demo", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttmoon", "Putt-Putt Moon", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "puttputt", "jputtputt", kGenHEPC, Common::JA_JPN, Common::kPlatform3DO, 0 },
	{ "puttputt", "puttputt", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttputt", "puttdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttputt", "Putt-Putt's Demo", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttputt", "Putt-Putt Parade", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttputt", "Putt-Putt", kGenHEMacNoParens, UNK_LANG, Common::kPlatformMacintosh, 0 },

#ifdef ENABLE_HE
#ifdef USE_RGB_COLOR
	{ "arttime", "arttime", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "arttime", "Blues-ArtTime", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "arttime", "artdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "arttime", "Blues-ArtTime Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "baseball2001", "baseball2001", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "baseball2001", "bb2demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "baseball2001", "Baseball 2001 Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "baseball2001", "Baseball 2001", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "baseball2001", "baseball 2001", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "Baseball2003", "Baseball2003", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "Baseball2003", "Baseball 2003", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "basketball", "basketball", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "basketball", "Basketball", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "football2002", "FootBall2002", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "football2002", "Football 2002", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "freddicove", "freddicove", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FreddiCCC", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FreddiCove", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddicove", "FreddiDZZ", kGenHEPC, Common::NL_NLD, Common::kPlatformWindows, 0 },
	{ "freddicove", "FreddiDZZ", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "freddicove", "FreddiMML", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "freddicove", "FreddiMML", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "freddicove", "FFCoveDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FreddiCoveDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddicove", "ff5demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddicove", "FF5Demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },

	{ "moonbase", "moonbase", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "moonbase", "moondemo", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "pjgames", "pjgames", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pjgames", "PJGames", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "readtime", "Blue's Reading Time", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "readtime", "Blues-ReadingTime", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "readtime", "readDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "readtime", "Blues-ReadingTime Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "Soccer2004", "Soccer2004", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "Soccer2004", "Soccer 2004", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "SoccerMLS", "SoccerMLS", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "SoccerMLS", "Backyard Soccer MLS", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "spyozon", "spyozon", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyozon", "sf3-demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyozon", "SF3Demo", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "spyozon", "Spy Ozone Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyozon", "SPYFoxAIW", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "spyozon", "SPYFoxOZU", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyozon", "SPYFoxSOS", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "spyozon", "SPYFoxSOS", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "spyozon", "SpyOzon", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyozon", "ozonepre.cup", kGenUnchanged, UNK_LANG, UNK, "HE CUP" },
#endif

	{ "airport", "airport", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "airport", "airdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "airport", "Airport Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "airport", "The AirPort", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "balloon", "balloon", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "balloon", "Balloon-O-Rama", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "balloon", "Ballon-O-Rama", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },

	{ "baseball", "baseball", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "baseball", "BaseBall", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "baseball", "basedemo.cup", kGenUnchanged, UNK_LANG, UNK, 0 },

	{ "blues123time", "Blues123time", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "blues123time", "Blue's 123 Time", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "bluesabctime", "bluesabctime", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "bluesabctime", "Blues ABC Time", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "bluesabctime", "BluesABCTimeDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "bluesabctime", "BluesABCTimeDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "bluesabctime", "abc-slideshow.cup", kGenUnchanged, UNK_LANG, UNK, 0 },
	{ "bluesabctime", "BluesABCTimeSlideshow.cup", kGenUnchanged, UNK_LANG, UNK, 0 },

	{ "BluesBirthday", "Blue'sBirthday-Red", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "BluesBirthday", "Blue'sBirthday-Red", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "BluesBirthday", "Blue'sBirthday-Yellow", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "BluesBirthday", "Blue'sBirthday-Yellow", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "BluesBirthday", "BluesBirthdayDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "BluesBirthday", "BluesBirthdayDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "BluesBirthday", "bda-slideshow.cup", kGenUnchanged, UNK_LANG, UNK, 0 },
	{ "BluesBirthday", "BluesBirthdaySlideshow.cup", kGenUnchanged, UNK_LANG, UNK, 0 },

	{ "BluesTreasureHunt", "Blue'sTreasureHunt-Disc1", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "BluesTreasureHunt", "Blue'sTreasureHunt-Disc1", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "BluesTreasureHunt", "Blue'sTreasureHunt-Disc2", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "BluesTreasureHunt", "Blue'sTreasureHunt-Disc2", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "catalog", "catalog", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "catalog", "catalog2", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "catalog", "Preview.cup", kGenUnchanged, UNK_LANG, UNK, 0 },

	{ "chase", "chase", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "chase", "Cheese Chase", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "dog", "dog", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "dog", "Dog on a Stick", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "dog", "Springparadijs", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },

	{ "farm", "farm", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "farm", "farmdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "farm", "Farm Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "football", "football", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "football", "FootBall", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "football", "FootBall Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "football", "footdemo", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "freddi", "freddi", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi", "Freddi", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi", "Freddi1", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi", "Freddemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi", "freddemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi", "Freddi Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi", "Freddi Fish", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi", "FreddiD", kGenHEPC, Common::NL_NLD, UNK, 0 },
	{ "freddi", "Freddi Fisk", kGenHEMac, Common::SE_SWE, Common::kPlatformMacintosh, 0 },
	{ "freddi", "FRITZI", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "freddi", "Marine Malice", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "freddi", "MM-DEMO", kGenHEPC, UNK_LANG, UNK, 0 },

	{ "freddi2", "freddi2", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi2", "FF2-demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi2", "ff2-demo", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "freddi2", "FFHSDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi2", "FFHSDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi2", "Freddi Fish 2 Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi2", "Freddi Fish 2", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi2", "FreddiCHSH", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi2", "FRITZI2", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "freddi2", "Fritzi Fisch 2", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "freddi2", "MALICE2", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },

	{ "freddi3", "freddi3", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "freddi3", "F3-Mdemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "f3-mdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "FF3 Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "FF3-demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "FF3-DEMO", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "freddi3", "FF3DEMO", kGenHEPC, Common::HE_ISR, UNK, 0 },
	{ "freddi3", "Freddi 3", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "Freddi Fish 3", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "FreddiFGT", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "freddi3", "FreddiFGT", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "FreddiSCS", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi3", "fritzi3", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "freddi3", "Fritzi3demo", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "Fritzi3demo", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "freddi3", "Marine Malice 3", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "MALICE3", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "freddi3", "MALICEMCV", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "freddi3", "MaliceMCV", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "freddi3", "MM3-DEMO", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "freddi3", "MM3-Demo", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },

	{ "freddi4", "freddi4", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "freddi4", "Freddi4", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "f4-demo", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "freddi4", "ff4demo", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "freddi4", "Ff4demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "FF4 demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "Freddi 4", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "Freddi 4 Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "FreddiGS", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "freddi4", "FreddiGS", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "FreddiHRBG", kGenHEPC, Common::EN_GRB, UNK, 0 },
	{ "freddi4", "FreddiHRBG", kGenHEMac, Common::EN_GRB, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "FreddiMini", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "freddi4", "Malice4", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "freddi4", "MaliceMRC", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "freddi4", "Mm4demo", kGenHEPC, Common::FR_FRA, UNK, 0 },

	{ "FreddisFunShop", "FreddisFunShop", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "FreddisFunShop", "FreddisFunShop", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "FreddisFunShop", "Freddi's FunShop", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "jungle", "jungle", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "jungle", "The Jungle", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "lost", "lost", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "lost", "Lost and Found", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "lost", "smaller", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "lost", "verloren", kGenHEPC, Common::NL_NLD, UNK, 0 },
	{ "lost", "Verloren", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },

	{ "maze", "maze", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "maze", "Doolhof", kGenHEPC, Common::NL_NLD, Common::kPlatformWindows, 0 },
	{ "maze", "Doolhof", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "maze", "Maze Madness", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "mustard", "mustard", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "mustard", "Mustard", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "pajama", "pajama", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "pajama", "Pyjama Pit", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "pajama", "Pajama Sam", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama", "PajamaNHD", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama", "PJS-DEMO", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "pajama", "PJS-DEMO", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama", "pjsam", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama", "PjSamDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama", "PYJAMA", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "pajama", "SAMDEMO", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "pajama", "SAMDEMO", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "pajama", "sampyjam", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "pajama", "SamPyjam", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },

	{ "pajama2", "pajama2", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "pajama2", "Pajama2", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "pyjam2", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "pajama2", "Pajama Sam 2", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "PajamaTAL", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama2", "PYJAMADB", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "pajama2", "PyjamaDBMN", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "pajama2", "PyjamaDBMN", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "PyjamaHM", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "PYJAMA2", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "pajama2", "Pyjama Pit 2", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "Pyjama Pit 2 Demo", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "PJP2DEMO", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "pajama2", "PJ2Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "pj2demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama2", "Pjs2demo", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "pajama2", "PJ2 Demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "pajama2", "PS2DEMO", kGenHEPC, Common::HE_ISR, UNK, 0 },

	{ "pajama3", "pajama3", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama3", "FPJ3Demo", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "pajama3", "GPJ3Demo", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "pajama3", "PajamaHTF", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "Pajama Sam 3", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "Pajama Sam 3-Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "pj3-demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama3", "pj3demo", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "pajama3", "PJ3Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "Pajama Sam Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "PJMini", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama3", "PjSamDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "PjSamDemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "pajama3", "PyjamaHG", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "pajama3", "PyjamaSKS", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "pajama3", "PyjamaSKS", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "pajama3", "UKPajamaEAT", kGenHEPC, Common::RU_RUS, UNK, 0 },

	{ "puttcircus", "puttcircus", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttcircus", "circdemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttcircus", "CircusDemo", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "puttcircus", "PouceDLC", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "puttcircus", "Putt Circus Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttcircus", "Putt Circus", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttcircus", "PuttIHC", kGenHEPC, Common::NL_NLD, Common::kPlatformWindows, 0 },
	{ "puttcircus", "PuttPuttIHC", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "puttcircus", "PuttPuttJTC", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttcircus", "ToffToff", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "puttcircus", "ToffToffGZZ", kGenHEPC, Common::DE_DEU, UNK, 0 },

	{ "puttrace", "puttrace", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttrace", "500demo", kGenHEPC, Common::NL_NLD, Common::kPlatformWindows, 0 },
	{ "puttrace", "course", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "puttrace", "CourseDemo", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "puttrace", "racedemo", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "puttrace", "RaceDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttrace", "Rennen", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "puttrace", "PouceCourse", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "puttrace", "Putt500", kGenHEPC, Common::NL_NLD, Common::kPlatformWindows, 0 },
	{ "puttrace", "Putt500", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "puttrace", "Putt500 demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "puttrace", "Putt Race", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttrace", "ToffRenn", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "puttrace", "ToffRennen", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "puttrace", "ToffRennen", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "puttrace", "UKPuttRace", kGenHEPC, Common::RU_RUS, UNK, 0 }, // Russian
	{ "puttrace", "PUTTDEMO.CUP", kGenUnchanged, UNK_LANG, UNK, 0 },
	{ "puttrace", "racedemo.cup", kGenUnchanged, UNK_LANG, UNK, 0 },

	{ "PuttsFunShop", "PuttsFunShop", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "PuttsFunShop", "Putt's FunShop", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "putttime", "putttime", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "putttime", "PuttTime", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "putttime", "pouce", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "putttime", "Pouce-Pouce", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "putttime", "PuttPuttTTT", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "putttime", "PuttPuttTTT", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "putttime", "PuttTijd", kGenHEPC, Common::NL_NLD, Common::kPlatformWindows, 0 },
	{ "putttime", "PuttTijd", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "putttime", "Putt Time", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "putttime", "PuttTTT", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "putttime", "PuttTTT", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "putttime", "TIJDDEMO", kGenHEPC, Common::NL_NLD, Common::kPlatformWindows, 0 },
	{ "putttime", "TijdDemo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "putttime", "timedemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "putttime", "TimeDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "putttime", "TEMPDEMO", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "putttime", "Tempdemo", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "putttime", "toffzeit", kGenHEPC, Common::DE_DEU, UNK, 0 }, // German Toeff-Toeff: Reist durch die Zeit
	{ "putttime", "toffzeit", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 }, // German Toeff-Toeff: Reist durch die Zeit
	{ "putttime", "ZeitDemo", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "putttime", "ZEITDEMO", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "putttime", "PuttMini", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "putttime", "PouceVDT", kGenHEPC, Common::FR_FRA, UNK, 0 },

	{ "puttzoo", "Putt-Putt Redt De Zoo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "puttzoo", "puttzoo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttzoo", "Puttzoo Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttzoo", "PuttZoo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttzoo", "T\xC3\x96""FFZOO", kGenHEPC, Common::DE_DEU, UNK, 0 },	// UTF-8 encoding
	{ "puttzoo", "T\xD6""FFZOO", kGenHEPC, Common::DE_DEU, UNK, 0 },	// Windows encoding
	{ "puttzoo", "T\xC3\xB6""ff-T\xC3\xB6""ff\xE2\x84\xA2 Zoo Demo", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },	// UTF-8 encoding
	{ "puttzoo", "T\xF6""ff-T""\xF6""ff\x99 Zoo Demo", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },	// Windows encoding
	{ "puttzoo", "Zoodemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttzoo", "zoodemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "puttzoo", "Zoo Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttzoo", "Putt-Putt Saves the Zoo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "puttzoo", "game", kGenHEPC, Common::EN_ANY, Common::kPlatformIOS, 0 },

	{ "SamsFunShop", "SamsFunShop", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "SamsFunShop", "Sam's FunShop", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "soccer", "soccer", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "soccer", "Soccer", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "socks", "socks", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "socks", "SockWorks", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "socks", "SokkenSoep", kGenHEPC, Common::NL_NLD, UNK, 0 },
	{ "socks", "SokkenSoep", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },

	{ "spyfox", "spyfox", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "spyfox", "Fuchsdem", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "FUCHSDEM", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "spyfox", "FoxDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "foxdemo", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "spyfox", "JAMESDEM", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "spyfox", "renard", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "spyfox", "Spydemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "Spydemo", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "spyfox", "SPYFox", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "SPYFoxDC", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "SPYFoxDC", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "SpyFoxDMK", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox", "SpyFoxDMK", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "SPYFoxOM", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "spyfox", "SPYFoxOM", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "SPYFoxOMW", kGenHEPC, Common::NL_NLD, UNK, 0 },
	{ "spyfox", "Spy Fox", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "Spy Fox Demo", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "JR-Demo", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "spyfox", "game", kGenHEIOS, Common::EN_ANY, Common::kPlatformIOS, 0 },

	{ "spyfox2", "spyfox2", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox2", "sf2-demo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox2", "sf2demo", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "spyfox2", "Sf2demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox2", "Spy Fox 2", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox2", "Spy Fox 2 - Demo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox2", "SpyFoxOR", kGenHEPC, Common::DE_DEU, UNK, 0 },
	{ "spyfox2", "SpyFoxOR", kGenHEMac, Common::DE_DEU, Common::kPlatformMacintosh, 0 },
	{ "spyfox2", "SPYFoxORE", kGenHEPC, Common::FR_FRA, UNK, 0 },
	{ "spyfox2", "SPYFoxORE", kGenHEMac, Common::FR_FRA, Common::kPlatformMacintosh, 0 },
	{ "spyfox2", "spyfoxsr", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox2", "SpyFoxSR", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "spyfox2", "SPYMini", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "spyfox2", "spy2preview.cup", kGenUnchanged, UNK_LANG, UNK, 0 },

	{ "thinker1", "1grademo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "thinker1", "thinker1", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "thinker1", "Thinker1", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "thinkerk", "kinddemo", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "thinkerk", "KindDemo", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
	{ "thinkerk", "thinkerk", kGenHEPC, UNK_LANG, UNK, 0 },
	{ "thinkerk", "ThinkerK", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },

	{ "water", "water", kGenHEPC, UNK_LANG, Common::kPlatformWindows, 0 },
	{ "water", "Water", kGenHEMac, Common::NL_NLD, Common::kPlatformMacintosh, 0 },
	{ "water", "Water Worries", kGenHEMac, UNK_LANG, Common::kPlatformMacintosh, 0 },
#endif
	{ NULL, NULL, kGenUnchanged, UNK_LANG, UNK, 0 }
};

} // End of namespace Scumm

#endif
