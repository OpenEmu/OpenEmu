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

#ifndef GOB_DETECTION_TABLES_H
#define GOB_DETECTION_TABLES_H

namespace Gob {

struct GOBGameDescription {
	ADGameDescription desc;

	GameType gameType;
	int32 features;
	const char *startStkBase;
	const char *startTotBase;
	uint32 demoIndex;
};

}

using namespace Common;

// Game IDs and proper names
static const PlainGameDescriptor gobGames[] = {
	{"gob", "Gob engine game"},
	{"gob1", "Gobliiins"},
	{"gob1cd", "Gobliiins CD"},
	{"gob2", "Gobliins 2"},
	{"gob2cd", "Gobliins 2 CD"},
	{"ween", "Ween: The Prophecy"},
	{"bargon", "Bargon Attack"},
	{"babayaga", "Once Upon A Time: Baba Yaga"},
	{"abracadabra", "Once Upon A Time: Abracadabra"},
	{"littlered", "Once Upon A Time: Little Red Riding Hood"},
	{"onceupon", "Once Upon A Time"},
	{"ajworld", "A.J.'s World of Discovery"},
	{"gob3", "Goblins Quest 3"},
	{"gob3cd", "Goblins Quest 3 CD"},
	{"lit1", "Lost in Time Part 1"},
	{"lit2", "Lost in Time Part 2"},
	{"lit", "Lost in Time"},
	{"inca2", "Inca II: Wiracocha"},
	{"woodruff", "The Bizarre Adventures of Woodruff and the Schnibble"},
	{"dynasty", "The Last Dynasty"},
	{"urban", "Urban Runner"},
	{"playtoons1", "Playtoons 1 - Uncle Archibald"},
	{"playtoons2", "Playtoons 2 - The Case of the Counterfeit Collaborator"},
	{"playtoons3", "Playtoons 3 - The Secret of the Castle"},
	{"playtoons4", "Playtoons 4 - The Mandarine Prince"},
	{"playtoons5", "Playtoons 5 - The Stone of Wakan"},
	{"playtnck1", "Playtoons Construction Kit 1 - Monsters"},
	{"playtnck2", "Playtoons Construction Kit 2 - Knights"},
	{"playtnck3", "Playtoons Construction Kit 3 - Far West"},
	{"bambou", "Playtoons Limited Edition - Bambou le sauveur de la jungle"},
	{"fascination", "Fascination"},
	{"geisha", "Geisha"},
	{"adi2", "ADI 2"},
	{"adi4", "ADI 4"},
	{"adibou2", "Adibou 2"},
	{"adibou1", "Adibou 1"},
	{0, 0}
};

// Obsolete IDs we don't want anymore
static const Engines::ObsoleteGameID obsoleteGameIDsTable[] = {
	{"gob1", "gob", kPlatformUnknown},
	{"gob2", "gob", kPlatformUnknown},
	{0, 0, kPlatformUnknown}
};

namespace Gob {

// Detection tables
static const GOBGameDescription gameDescriptions[] = {
	#include "gob/detection/tables_gob1.h"      // Gobliiins
	#include "gob/detection/tables_gob2.h"      // Gobliins 2: The Prince Buffoon
	#include "gob/detection/tables_gob3.h"      // Goblins 3 / Goblins Quest 3
	#include "gob/detection/tables_ween.h"      // Ween: The Prophecy
	#include "gob/detection/tables_bargon.h"    // Bargon Attack
	#include "gob/detection/tables_littlered.h" // Once Upon A Time: Little Red Riding Hood
	#include "gob/detection/tables_onceupon.h"  // Once Upon A Time: Baba Yaga and Abracadabra
	#include "gob/detection/tables_lit.h"       // Lost in Time
	#include "gob/detection/tables_fascin.h"    // Fascination
	#include "gob/detection/tables_geisha.h"    // Geisha
	#include "gob/detection/tables_inca2.h"     // Inca II: Wiracocha
	#include "gob/detection/tables_woodruff.h"  // (The Bizarre Adventures of) Woodruff and the Schnibble (of Azimuth)
	#include "gob/detection/tables_dynasty.h"   // The Last Dynasty
	#include "gob/detection/tables_urban.h"     // Urban Runner
	#include "gob/detection/tables_playtoons.h" // The Playtoons series
	#include "gob/detection/tables_adi2.h"      // The ADI / Addy 2 series
	#include "gob/detection/tables_adi4.h"      // The ADI / Addy 4 series
	#include "gob/detection/tables_adibou.h"    // The Adibou / Addy Junior series
	#include "gob/detection/tables_ajworld.h"   // A.J.'s World of Discovery / ADI Jr.

	{ AD_TABLE_END_MARKER, kGameTypeNone, kFeaturesNone, 0, 0, 0}
};

// File-based fallback tables
#include "gob/detection/tables_fallback.h"
}

#endif // GOB_DETECTION_TABLES_H
