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


#include "common/endian.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "cine/cine.h"
#include "cine/various.h"
#include "cine/bg.h"

namespace Cine {

uint16 bgVar0;
byte *additionalBgTable[9];
int16 currentAdditionalBgIdx = 0, currentAdditionalBgIdx2 = 0;

byte loadCtFW(const char *ctName) {
	debugC(1, kCineDebugCollision, "loadCtFW(\"%s\")", ctName);
	byte *ptr, *dataPtr;

	int16 foundFileIdx = findFileInBundle(ctName);
	if (foundFileIdx == -1) {
		warning("loadCtFW: Unable to find collision data file '%s'", ctName);
		// FIXME: Rework this function's return value policy and return an appropriate value here.
		// The return value isn't yet used for anything so currently it doesn't really matter.
		return 0;
	}

	if (currentCtName != ctName)
		strcpy(currentCtName, ctName);

	ptr = dataPtr = readBundleFile(foundFileIdx);

	loadRelatedPalette(ctName);

	assert(strstr(ctName, ".NEO"));

	gfxConvertSpriteToRaw(collisionPage, ptr + 0x80, 160, 200);

	free(dataPtr);
	return 0;
}

byte loadCtOS(const char *ctName) {
	debugC(1, kCineDebugCollision, "loadCtOS(\"%s\")", ctName);
	byte *ptr, *dataPtr;

	int16 foundFileIdx = findFileInBundle(ctName);
	if (foundFileIdx == -1) {
		warning("loadCtOS: Unable to find collision data file '%s'", ctName);
		// FIXME: Rework this function's return value policy and return an appropriate value here.
		// The return value isn't yet used for anything so currently it doesn't really matter.
		return 0;
	}

	if (currentCtName != ctName)
		strcpy(currentCtName, ctName);

	ptr = dataPtr = readBundleFile(foundFileIdx);

	uint16 bpp = READ_BE_UINT16(ptr);
	ptr += 2;

	if (bpp == 8) {
		renderer->loadCt256(ptr, ctName);
	} else {
		gfxConvertSpriteToRaw(collisionPage, ptr + 32, 160, 200);
		renderer->loadCt16(ptr, ctName);
	}

	free(dataPtr);
	return 0;
}

byte loadBg(const char *bgName) {
	byte *ptr, *dataPtr;

	byte fileIdx = findFileInBundle(bgName);
	ptr = dataPtr = readBundleFile(fileIdx);

	uint16 bpp = READ_BE_UINT16(ptr);
	ptr += 2;

	if (bpp == 8) {
		renderer->loadBg256(ptr, bgName);
	} else {
		if (g_cine->getGameType() == Cine::GType_FW) {
			loadRelatedPalette(bgName);
		}

		renderer->loadBg16(ptr, bgName);
	}
	free(dataPtr);
	return 0;
}

void addBackground(const char *bgName, uint16 bgIdx) {
	byte *ptr, *dataPtr;

	byte fileIdx = findFileInBundle(bgName);
	ptr = dataPtr = readBundleFile(fileIdx);

	uint16 bpp = READ_BE_UINT16(ptr); ptr += 2;

	if (bpp == 8) {
		renderer->loadBg256(ptr, bgName, bgIdx);
	} else {
		renderer->loadBg16(ptr, bgName, bgIdx);
	}
	free(dataPtr);
}

} // End of namespace Cine
