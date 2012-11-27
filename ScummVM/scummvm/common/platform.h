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

#ifndef COMMON_PLATFORM_H
#define COMMON_PLATFORM_H

#include "common/scummsys.h"

namespace Common {

class String;

/**
 * List of game platforms. Specifying a platform for a target can be used to
 * give the game engines a hint for which platform the game data file are.
 * This may be optional or required, depending on the game engine and the
 * game in question.
 */
enum Platform {
	kPlatformPC,
	kPlatformAmiga,
	kPlatformAtariST,
	kPlatformMacintosh,
	kPlatformFMTowns,
	kPlatformWindows,
	kPlatformNES,
	kPlatformC64,
	kPlatformCoCo3,
	kPlatformLinux,
	kPlatformAcorn,
	kPlatformSegaCD,
	kPlatform3DO,
	kPlatformPCEngine,
	kPlatformApple2GS,
	kPlatformPC98,
	kPlatformWii,
	kPlatformPSX,
	kPlatformCDi,
	kPlatformIOS,

	kPlatformUnknown = -1
};

struct PlatformDescription {
	const char *code;
	const char *code2;
	const char *abbrev;
	const char *description;
	Platform id;
};

extern const PlatformDescription g_platforms[];

/** Convert a string containing a platform name into a Platform enum value. */
extern Platform parsePlatform(const String &str);
extern const char *getPlatformCode(Platform id);
extern const char *getPlatformAbbrev(Platform id);
extern const char *getPlatformDescription(Platform id);

}	// End of namespace Common

#endif
