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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "common/platform.h"
#include "common/str.h"

namespace Common {

const PlatformDescription g_platforms[] = {
	{ "2gs", "2gs", "2gs", "Apple IIgs", kPlatformApple2GS },
	{ "3do", "3do", "3do", "3DO", kPlatform3DO },
	{ "acorn", "acorn", "acorn", "Acorn", kPlatformAcorn },
	{ "amiga", "ami", "amiga", "Amiga", kPlatformAmiga },
	{ "atari", "atari-st", "st", "Atari ST", kPlatformAtariST },
	{ "c64", "c64", "c64", "Commodore 64", kPlatformC64 },
	{ "pc", "dos", "ibm", "DOS", kPlatformPC },
	{ "pc98", "pc98", "pc98", "PC-98", kPlatformPC98 },
	{ "wii", "wii", "wii", "Nintendo Wii", kPlatformWii },
	{ "coco3", "coco3", "coco3", "CoCo3", kPlatformCoCo3 },

	// The 'official' spelling seems to be "FM-TOWNS" (e.g. in the Indy4 demo).
	// However, on the net many variations can be seen, like "FMTOWNS",
	// "FM TOWNS", "FmTowns", etc.
	{ "fmtowns", "towns", "fm", "FM-TOWNS", kPlatformFMTowns },

	{ "linux", "linux", "linux", "Linux", kPlatformLinux },
	{ "macintosh", "mac", "mac", "Macintosh", kPlatformMacintosh },
	{ "pce", "pce", "pce", "PC-Engine", kPlatformPCEngine },
	{ "nes", "nes", "nes", "NES", kPlatformNES },
	{ "segacd", "segacd", "sega", "SegaCD", kPlatformSegaCD },
	{ "windows", "win", "win", "Windows", kPlatformWindows },
	{ "playstation", "psx", "psx", "Sony PlayStation", kPlatformPSX },
	{ "cdi", "cdi", "cdi", "Philips CD-i", kPlatformCDi },
	{ "ios", "ios", "ios", "Apple iOS", kPlatformIOS },

	{ 0, 0, 0, "Default", kPlatformUnknown }
};

Platform parsePlatform(const String &str) {
	if (str.empty())
		return kPlatformUnknown;

	// Handle some special case separately, for compatibility with old config
	// files.
	if (str == "1")
		return kPlatformAmiga;
	else if (str == "2")
		return kPlatformAtariST;
	else if (str == "3")
		return kPlatformMacintosh;

	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (str.equalsIgnoreCase(l->code) || str.equalsIgnoreCase(l->code2) || str.equalsIgnoreCase(l->abbrev))
			return l->id;
	}

	return kPlatformUnknown;
}


const char *getPlatformCode(Platform id) {
	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->code;
	}
	return 0;
}

const char *getPlatformAbbrev(Platform id) {
	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->abbrev;
	}
	return 0;
}

const char *getPlatformDescription(Platform id) {
	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->description;
	}
	return l->description;
}

} // End of namespace Common
