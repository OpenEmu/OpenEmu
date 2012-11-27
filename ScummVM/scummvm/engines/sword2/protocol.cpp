/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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

#include "common/file.h"
#include "common/endian.h"

#include "sword2/sword2.h"
#include "sword2/header.h"
#include "sword2/resman.h"
#include "sword2/logic.h"

namespace Sword2 {

/**
 * Returns a pointer to the first palette entry, given the pointer to the start
 * of the screen file.
 */

void Sword2Engine::fetchPalette(byte *screenFile, byte *palBuffer) {
	byte *palette;

	if (isPsx()) { // PSX version doesn't have a "MultiScreenHeader", instead there's a ScreenHeader and a tag
		palette = screenFile + ResHeader::size() + ScreenHeader::size() + 2;
	} else {
		MultiScreenHeader mscreenHeader;

		mscreenHeader.read(screenFile + ResHeader::size());
		palette = screenFile + ResHeader::size() + mscreenHeader.palette;
	}

	// Always set color 0 to black, because while most background screen
	// palettes have a bright color 0 it should come out as black in the
	// game.

	palBuffer[0] = 0;
	palBuffer[1] = 0;
	palBuffer[2] = 0;

	for (uint i = 4, j = 3; i < 4 * 256; i += 4, j += 3) {
		palBuffer[j + 0] = palette[i + 0];
		palBuffer[j + 1] = palette[i + 1];
		palBuffer[j + 2] = palette[i + 2];
	}
}

/**
 * Returns a pointer to the start of the palette match table, given the pointer
 * to the start of the screen file.
 * It returns NULL when used with PSX version, as there are no palette match tables in
 * the resource files.
 */

byte *Sword2Engine::fetchPaletteMatchTable(byte *screenFile) {

	if (isPsx()) return NULL;

	MultiScreenHeader mscreenHeader;

	mscreenHeader.read(screenFile + ResHeader::size());

	return screenFile + ResHeader::size() + mscreenHeader.paletteTable;
}

/**
 * Returns a pointer to the screen header, given the pointer to the start of
 * the screen file.
 */

byte *Sword2Engine::fetchScreenHeader(byte *screenFile) {
	if (isPsx()) { // In PSX version there's no MultiScreenHeader, so just skip resource header
		return screenFile + ResHeader::size();
	} else {
		MultiScreenHeader mscreenHeader;

		mscreenHeader.read(screenFile + ResHeader::size());
		return screenFile + ResHeader::size() + mscreenHeader.screen;
	}
}

/**
 * Returns a pointer to the requested layer header, given the pointer to the
 * start of the screen file. Drops out if the requested layer number exceeds
 * the number of layers on this screen.
 */

byte *Sword2Engine::fetchLayerHeader(byte *screenFile, uint16 layerNo) {
#ifdef SWORD2_DEBUG
	ScreenHeader screenHead;

	screenHead.read(fetchScreenHeader(screenFile));
	assert(layerNo < screenHead.noLayers);
#endif

	if (isPsx()) {
		return screenFile + ResHeader::size() + ScreenHeader::size() + 2 + 0x400 + layerNo * LayerHeader::size();
	} else {
		MultiScreenHeader mscreenHeader;

		mscreenHeader.read(screenFile + ResHeader::size());
		return screenFile + ResHeader::size() + mscreenHeader.layers + layerNo * LayerHeader::size();
	}
}

/**
 * Returns a pointer to the start of the shading mask, given the pointer to the
 * start of the screen file.
 * If we are non PSX, this will return NULL, as we don't have shading masks.
 */

byte *Sword2Engine::fetchShadingMask(byte *screenFile) {
	if (isPsx()) return NULL;

	MultiScreenHeader mscreenHeader;

	mscreenHeader.read(screenFile + ResHeader::size());

	return screenFile + ResHeader::size() + mscreenHeader.maskOffset;
}

/**
 * Returns a pointer to the anim header, given the pointer to the start of the
 * anim file.
 */

byte *Sword2Engine::fetchAnimHeader(byte *animFile) {
	return animFile + ResHeader::size();
}

/**
 * Returns a pointer to the requested frame number's cdtEntry, given the
 * pointer to the start of the anim file. Drops out if the requested frame
 * number exceeds the number of frames in this anim.
 */

byte *Sword2Engine::fetchCdtEntry(byte *animFile, uint16 frameNo) {
#ifdef SWORD2_DEBUG
	AnimHeader animHead;

	animHead.read(fetchAnimHeader(animFile));

	if (frameNo > animHead->noAnimFrames - 1)
		error("fetchCdtEntry(animFile,%d) - anim only %d frames", frameNo, animHead->noAnimFrames);
#endif

	return fetchAnimHeader(animFile) + AnimHeader::size() + frameNo * CdtEntry::size();
}

/**
 * Returns a pointer to the requested frame number's header, given the pointer
 * to the start of the anim file. Drops out if the requested frame number
 * exceeds the number of frames in this anim
 */

byte *Sword2Engine::fetchFrameHeader(byte *animFile, uint16 frameNo) {
	// required address = (address of the start of the anim header) + frameOffset

	CdtEntry cdt;

	cdt.read(fetchCdtEntry(animFile, frameNo));

	return animFile + ResHeader::size() + cdt.frameOffset;
}

/**
 * Returns a pointer to the requested parallax layer data.
 */

byte *Sword2Engine::fetchBackgroundParallaxLayer(byte *screenFile, int layer) {
	if (isPsx()) {
		byte *psxParallax = _screen->getPsxScrCache(0);

		// Manage cache for background psx parallaxes
		if (!_screen->getPsxScrCacheStatus(0)) { // This parallax layer is not present
			return NULL;
		} else if (psxParallax != NULL) { // Parallax layer present, and already in cache
			return psxParallax;
		} else { // Present, but not cached
			uint32 locNo = _logic->getLocationNum();

			// At game startup, we have a wrong location number stored
			// in game vars (0, instead of 3), work around this.
			locNo = (locNo == 0) ? 3 : locNo;

			psxParallax = fetchPsxParallax(locNo, 0);
			_screen->setPsxScrCache(psxParallax, 0);
			return psxParallax;
		}
	} else {
		MultiScreenHeader mscreenHeader;

		mscreenHeader.read(screenFile + ResHeader::size());
		assert(mscreenHeader.bg_parallax[layer]);
		return screenFile + ResHeader::size() + mscreenHeader.bg_parallax[layer];
	}
}

byte *Sword2Engine::fetchBackgroundLayer(byte *screenFile) {
	if (isPsx()) {
		byte *psxBackground = _screen->getPsxScrCache(1);

		// Manage cache for psx backgrounds
		if (psxBackground) { // Background is cached
			return psxBackground;
		} else { // Background not cached
			uint32 locNo = _logic->getLocationNum();

			// We have a wrong location number at start, fix that
			locNo = (locNo == 0) ? 3 : locNo;

			psxBackground = fetchPsxBackground(locNo);
			_screen->setPsxScrCache(psxBackground, 1);
			return psxBackground;
		}
	} else {
		MultiScreenHeader mscreenHeader;

		mscreenHeader.read(screenFile + ResHeader::size());
		assert(mscreenHeader.screen);
		return screenFile + ResHeader::size() + mscreenHeader.screen + ScreenHeader::size();
	}
}

byte *Sword2Engine::fetchForegroundParallaxLayer(byte *screenFile, int layer) {
	if (isPsx()) {
		byte *psxParallax = _screen->getPsxScrCache(2);

		// Manage cache for psx parallaxes
		if (!_screen->getPsxScrCacheStatus(2)) { // This parallax layer is not present
			return NULL;
		} else if (psxParallax) { // Parallax layer present and cached
			return psxParallax;
		} else { // Present, but still not cached
			uint32 locNo = _logic->getLocationNum();

			// We have a wrong location number at start, fix that
			locNo = (locNo == 0) ? 3 : locNo;

			psxParallax = fetchPsxParallax(locNo, 1);
			_screen->setPsxScrCache(psxParallax, 2);
			return psxParallax;
		}
	} else {
		MultiScreenHeader mscreenHeader;

		mscreenHeader.read(screenFile + ResHeader::size());
		assert(mscreenHeader.fg_parallax[layer]);
		return screenFile + ResHeader::size() + mscreenHeader.fg_parallax[layer];
	}
}

byte *Sword2Engine::fetchTextLine(byte *file, uint32 text_line) {
	TextHeader text_header;
	static byte errorLine[128];

	text_header.read(file + ResHeader::size());

	if (text_line >= text_header.noOfLines) {
		sprintf((char *)errorLine, "xxMissing line %d of %s (only 0..%d)", text_line, _resman->fetchName(file), text_header.noOfLines - 1);

		// first 2 chars are NULL so that actor-number comes out as '0'
		errorLine[0] = 0;
		errorLine[1] = 0;
		return errorLine;
	}

	// The "number of lines" field is followed by a lookup table

	return file + READ_LE_UINT32(file + ResHeader::size() + 4 + 4 * text_line);
}

/**
 * Returns a pointer to psx background data for passed location number
 * At the beginning of the passed data there's an artificial header composed by
 * uint16: background X resolution
 * uint16: background Y resolution
 * uint32: offset to subtract from offset table entries
 */

byte *Sword2Engine::fetchPsxBackground(uint32 location) {
	Common::File file;
	PSXScreensEntry header;
	uint32 screenOffset, dataOffset;
	uint32 totSize; // Total size of background, counting data, offset table and additional header
	byte *buffer;

	if (!file.open("screens.clu")) {
		GUIErrorMessage("Broken Sword II: Cannot open screens.clu");
		return NULL;
	}

	file.seek(location * 4, SEEK_SET);
	screenOffset = file.readUint32LE();

	if (screenOffset == 0) { // We don't have screen data for this location number.
		file.close();
		return NULL;
	}

	// Get to the beginning of PSXScreensEntry
	file.seek(screenOffset + ResHeader::size(), SEEK_SET);

	buffer = (byte *)malloc(PSXScreensEntry::size());
	file.read(buffer, PSXScreensEntry::size());

	// Prepare the header
	header.read(buffer);
	free(buffer);

	file.seek(screenOffset + header.bgOffset + 4, SEEK_SET);
	dataOffset = file.readUint32LE();

	file.seek(screenOffset + header.bgOffset, SEEK_SET);

	totSize = header.bgSize + (dataOffset - header.bgOffset) + 8;
	buffer = (byte *)malloc(totSize);

	// Write some informations before background data
	WRITE_LE_UINT16(buffer, header.bgXres);
	WRITE_LE_UINT16(buffer + 2, header.bgYres);
	WRITE_LE_UINT32(buffer + 4, header.bgOffset);

	file.read(buffer + 8, totSize - 8); // Do not write on the header
	file.close();

	return buffer;
}

/**
 * Returns a pointer to selected psx parallax data for passed location number
 * At the beginning of the passed data there's an artificial header composed by
 * uint16: parallax X resolution
 * uint16: parallax Y resolution
 * uint16: width in 64x16 tiles of parallax
 * uint16: height in 64x16 tiles of parallax
 */

byte *Sword2Engine::fetchPsxParallax(uint32 location, uint8 level) {
	Common::File file;
	PSXScreensEntry header;
	uint32 screenOffset;
	uint16 horTiles; // Number of horizontal tiles in the parallax grid
	uint16 verTiles; // Number of vertical tiles in parallax grid
	uint32 totSize; // Total size of parallax, counting data, grid, and additional header
	byte *buffer;

	uint16 plxXres;
	uint16 plxYres;
	uint32 plxOffset;
	uint32 plxSize;

	if (level > 1)
		return NULL;

	if (!file.open("screens.clu")) {
		GUIErrorMessage("Broken Sword II: Cannot open screens.clu");
		return NULL;
	}

	file.seek(location * 4, SEEK_SET);
	screenOffset = file.readUint32LE();

	if (screenOffset == 0) // There is no screen here
		return NULL;

	// Get to the beginning of PSXScreensEntry
	file.seek(screenOffset + ResHeader::size(), SEEK_SET);

	buffer = (byte *)malloc(PSXScreensEntry::size());
	file.read(buffer, PSXScreensEntry::size());

	// Initialize the header
	header.read(buffer);
	free(buffer);

	// We are fetching...
	if (level == 0) { // a background parallax
		plxXres = header.bgPlxXres;
		plxYres = header.bgPlxYres;
		plxOffset = header.bgPlxOffset;
		plxSize = header.bgPlxSize;
	} else {  // a foreground parallax
		plxXres = header.fgPlxXres;
		plxYres = header.fgPlxYres;
		plxOffset = header.fgPlxOffset;
		plxSize = header.fgPlxSize;
	}

	if (plxXres == 0 || plxYres == 0 || plxSize == 0) // This screen has no parallax data.
		return NULL;

	debug(2, "fetchPsxParallax() -> %s parallax, xRes: %u, yRes: %u", (level == 0) ? "Background" : "Foreground", plxXres, plxYres);

	// Calculate the number of tiles which compose the parallax grid.
	horTiles = (plxXres % 64) ? (plxXres / 64) + 1 : plxXres / 64;
	verTiles = (plxYres % 16) ? (plxYres / 16) + 1 : plxYres / 16;

	totSize = plxSize + horTiles * verTiles * 4 + 8;

	file.seek(screenOffset + plxOffset, SEEK_SET);
	buffer = (byte *)malloc(totSize);

	// Insert parallax resolution information in the buffer,
	// preceding parallax data.
	WRITE_LE_UINT16(buffer, plxXres);
	WRITE_LE_UINT16(buffer + 2, plxYres);
	WRITE_LE_UINT16(buffer + 4, horTiles);
	WRITE_LE_UINT16(buffer + 6, verTiles);

	// Read parallax data from file and store it inside the buffer,
	// skipping the generated header.
	file.read(buffer + 8, totSize - 8);
	file.close();

	return buffer;
}

// Used for testing text & speech (see fnISpeak in speech.cpp)

bool Sword2Engine::checkTextLine(byte *file, uint32 text_line) {
	TextHeader text_header;

	text_header.read(file + ResHeader::size());

	return text_line < text_header.noOfLines;
}

} // End of namespace Sword2
