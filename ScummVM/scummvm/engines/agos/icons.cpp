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

#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/surface.h"

#include "agos/agos.h"

namespace AGOS {

void AGOSEngine::loadIconFile() {
	Common::File in;
	uint32 srcSize;

	in.open(getFileName(GAME_ICONFILE));
	if (in.isOpen() == false)
		error("Can't open icons file '%s'", getFileName(GAME_ICONFILE));

	srcSize = in.size();

	if (getGameType() == GType_WW && getPlatform() == Common::kPlatformAmiga) {
		byte *srcBuf = (byte *)malloc(srcSize);
		in.read(srcBuf, srcSize);

		uint32 dstSize = READ_BE_UINT32(srcBuf + srcSize - 4);
		_iconFilePtr = (byte *)malloc(dstSize);
		if (_iconFilePtr == NULL)
			error("Out of icon memory");

		decrunchFile(srcBuf, _iconFilePtr, srcSize);
		free(srcBuf);
	} else if (getGameType() == GType_PN && getPlatform() == Common::kPlatformAtariST) {
		// The icon data is hard coded in the program file.
		_iconFilePtr = (byte *)malloc(15038);
		if (_iconFilePtr == NULL)
			error("Out of icon memory");

		in.seek(48414);
		in.read(_iconFilePtr, 15038);
	} else {
		_iconFilePtr = (byte *)malloc(srcSize);
		if (_iconFilePtr == NULL)
			error("Out of icon memory");

		in.read(_iconFilePtr, srcSize);
	}
	in.close();
}

void AGOSEngine::loadIconData() {
	loadZone(8);
	VgaPointersEntry *vpe = &_vgaBufferPointers[8];

	byte *src = vpe->vgaFile2 + READ_LE_UINT32(vpe->vgaFile2 + 8);

	_iconFilePtr = (byte *)malloc(43 * 336);
	if (_iconFilePtr == NULL)
		error("Out of icon memory");

	memcpy(_iconFilePtr, src, 43 * 336);
	unfreezeBottom();
}

// Thanks to Stuart Caie for providing the original
// C conversion upon which this function is based.
static void decompressIconPlanar(byte *dst, byte *src, uint width, uint height, byte base, uint pitch, bool decompress = true) {
	byte *i, *icon_pln, *o, *srcPtr;
	byte x, y;

	icon_pln = 0;
	srcPtr = src;

	if (decompress) {
		icon_pln = (byte *)calloc(width * height, 1);

		// Decode RLE planar icon data
		i = src;
		o = icon_pln;
		while (o < &icon_pln[width * height]) {
			x = *i++;
			if (x < 128) {
				do {
					*o++ = *i++;
					*o++ = *i++;
					*o++ = *i++;
				} while (x-- > 0);
			} else {
				x = 256 - x;
				do {
					*o++ = i[0];
					*o++ = i[1];
					*o++ = i[2];
				} while (x-- > 0);
				i += 3;
			}
		}
		srcPtr = icon_pln;
	}

	// Translate planar data to chunky (very slow method)
	for (y = 0; y < height * 2; y++) {
		for (x = 0; x < width; x++) {
			byte pixel =
				  (srcPtr[((height * 0 + y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 1 : 0)
				| (srcPtr[((height * 2 + y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 2 : 0)
				| (srcPtr[((height * 4 + y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 4 : 0)
				| (srcPtr[((height * 6 + y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 8 : 0);
			if (pixel)
				dst[x] = pixel | base;
		}
		dst += pitch;
	}

	free(icon_pln);
}

static void decompressIcon(byte *dst, byte *src, uint width, uint height, byte base, uint pitch) {
	int8 reps;
	byte color_1, color_2;
	byte *dst_org = dst;
	uint h = height;

	for (;;) {
		reps = *src++;
		if (reps < 0) {
			reps--;
			color_1 = *src >> 4;
			if (color_1 != 0)
				color_1 |= base;
			color_2 = *src++ & 0xF;
			if (color_2 != 0)
				color_2 |= base;

			do {
				if (color_1 != 0)
					*dst = color_1;
				dst += pitch;
				if (color_2 != 0)
					*dst = color_2;
				dst += pitch;

				// reached bottom?
				if (--h == 0) {
					// reached right edge?
					if (--width == 0)
						return;
					dst = ++dst_org;
					h = height;
				}
			} while (++reps != 0);
		} else {
			do {
				color_1 = *src >> 4;
				if (color_1 != 0)
					*dst = color_1 | base;
				dst += pitch;

				color_2 = *src++ & 0xF;
				if (color_2 != 0)
					*dst = color_2 | base;
				dst += pitch;

				// reached bottom?
				if (--h == 0) {
					// reached right edge?
					if (--width == 0)
						return;
					dst = ++dst_org;
					h = height;
				}
			} while (--reps >= 0);
		}
	}
}

void AGOSEngine_Simon2::drawIcon(WindowBlock *window, uint icon, uint x, uint y) {
	byte *dst;
	byte *src;

	_videoLockOut |= 0x8000;

	Graphics::Surface *screen = _system->lockScreen();
	dst = (byte *)screen->pixels;

	dst += 110;
	dst += x;
	dst += (y + window->y) * screen->pitch;

	src = _iconFilePtr;
	src += READ_LE_UINT16(src + icon * 4 + 0);
	decompressIcon(dst, src, 20, 10, 224, screen->pitch);

	src = _iconFilePtr;
	src += READ_LE_UINT16(src + icon * 4 + 2);
	decompressIcon(dst, src, 20, 10, 208, screen->pitch);

	_system->unlockScreen();

	_videoLockOut &= ~0x8000;
}

void AGOSEngine_Simon1::drawIcon(WindowBlock *window, uint icon, uint x, uint y) {
	byte *dst;
	byte *src;

	_videoLockOut |= 0x8000;

	Graphics::Surface *screen = _system->lockScreen();
	dst = (byte *)screen->pixels;

	dst += (x + window->x) * 8;
	dst += (y * 25 + window->y) * screen->pitch;

	if (getPlatform() == Common::kPlatformAmiga) {
		src = _iconFilePtr;
		src += READ_BE_UINT32(src + icon * 4);
		uint8 color = (getFeatures() & GF_32COLOR) ? 224 : 240;
		decompressIconPlanar(dst, src, 24, 12, color, screen->pitch);
	} else {
		src = _iconFilePtr;
		src += READ_LE_UINT16(src + icon * 2);
		decompressIcon(dst, src, 24, 12, 224, screen->pitch);
	}

	_system->unlockScreen();

	_videoLockOut &= ~0x8000;
}

void AGOSEngine_Waxworks::drawIcon(WindowBlock *window, uint icon, uint x, uint y) {
	byte *dst;
	byte *src;

	_videoLockOut |= 0x8000;

	Graphics::Surface *screen = _system->lockScreen();
	dst = (byte *)screen->pixels;

	dst += (x + window->x) * 8;
	dst += (y * 20 + window->y) * screen->pitch;

	uint8 color = dst[0] & 0xF0;
	if (getPlatform() == Common::kPlatformAmiga) {
		src = _iconFilePtr;
		src += READ_BE_UINT32(src + icon * 4);
		decompressIconPlanar(dst, src, 24, 10, color, screen->pitch);
	} else {
		src = _iconFilePtr;
		src += READ_LE_UINT16(src + icon * 2);
		decompressIcon(dst, src, 24, 10, color, screen->pitch);
	}

	_system->unlockScreen();

	_videoLockOut &= ~0x8000;
}

void AGOSEngine_Elvira2::drawIcon(WindowBlock *window, uint icon, uint x, uint y) {
	byte *dst;
	byte *src;

	_videoLockOut |= 0x8000;

	Graphics::Surface *screen = _system->lockScreen();
	dst = (byte *)screen->pixels;

	dst += (x + window->x) * 8;
	dst += (y * 8 + window->y) * screen->pitch;

	uint color = dst[0] & 0xF0;
	if (getFeatures() & GF_PLANAR) {
		src = _iconFilePtr;
		src += READ_BE_UINT32(src + icon * 4);
		decompressIconPlanar(dst, src, 24, 12, color, screen->pitch);
	} else {
		src = _iconFilePtr;
		src += READ_LE_UINT16(src + icon * 2);
		decompressIcon(dst, src, 24, 12, color, screen->pitch);
	}

	_system->unlockScreen();

	_videoLockOut &= ~0x8000;
}

void AGOSEngine_Elvira1::drawIcon(WindowBlock *window, uint icon, uint x, uint y) {
	byte *dst;
	byte *src;

	_videoLockOut |= 0x8000;

	Graphics::Surface *screen = _system->lockScreen();
	dst = (byte *)screen->pixels;

	dst += (x + window->x) * 8;
	dst += (y * 8 + window->y) * screen->pitch;

	if (getFeatures() & GF_PLANAR) {
		src = _iconFilePtr;
		src += READ_BE_UINT16(src + icon * 2);
		decompressIconPlanar(dst, src, 24, 12, 16, screen->pitch);
	} else {
		src = _iconFilePtr;
		src += icon * 288;
		decompressIconPlanar(dst, src, 24, 12, 16, screen->pitch, false);
	}

	_system->unlockScreen();

	_videoLockOut &= ~0x8000;
}

void AGOSEngine::drawIcon(WindowBlock *window, uint icon, uint x, uint y) {
	byte *dst;
	byte *src;

	_videoLockOut |= 0x8000;

	Graphics::Surface *screen = _system->lockScreen();
	dst = (byte *)screen->pixels + y * screen->pitch + x * 8;
	src = _iconFilePtr + icon * 146;

	if (icon == 0xFF) {
		// Draw Blank Icon
		for (int yp = 0; yp < 24; yp++) {
			memset(dst, 0, 24);
			dst += screen->pitch;
		}
	} else {
		uint8 palette[4];
		palette[0] = *src >> 4;
		palette[1] = *src++ & 0xf;
		palette[2] = *src >> 4;
		palette[3] = *src++ & 0xf;
		for (int yp = 0; yp < 24; ++yp, src += 6) {
			// Get bit-set representing the 24 pixels for the line
			uint32 v1 = (READ_BE_UINT16(src) << 8) | *(src + 4);
			uint32 v2 = (READ_BE_UINT16(src + 2) << 8) | *(src + 5);
			for (int xp = 0; xp < 24; ++xp, v1 >>= 1, v2 >>= 1) {
				dst[yp * screen->pitch + (23 - xp)] = palette[((v1 & 1) << 1) | (v2 & 1)];
			}
		}
	}

	_system->unlockScreen();

	_videoLockOut &= ~0x8000;
}

#ifdef ENABLE_AGOS2
void AGOSEngine_Feeble::drawIconArray(uint num, Item *itemRef, int line, int classMask) {
	Item *item_ptr_org = itemRef;
	WindowBlock *window;
	uint16 flagnumber = 201;
	uint16 iconperline = 458;
	uint16 iconsdown = 384;
	uint16 idone = 0;
	uint16 icount = 0;
	uint16 xp = 188, yp = 306;
	int k;
	_iOverflow = 0;

	line = _variableArray[30];
	if (line == 0)
		_variableArray[31] = 0;

	window = _windowArray[num & 7];
	if (window == NULL)
		return;

	for (k = flagnumber; k <= flagnumber + 18; k++)
		_variableArray[k] = 0;

	if (window->iconPtr)
		removeIconArray(num);

	window->iconPtr=(IconBlock *)malloc(sizeof(IconBlock));
	window->iconPtr->itemRef = itemRef;
	window->iconPtr->upArrow = -1;
	window->iconPtr->downArrow = -1;
	window->iconPtr->line = line;
	window->iconPtr->classMask = classMask;

	itemRef = derefItem(itemRef->child);
	k = flagnumber;

	while (itemRef && (line > 65)) {
		uint16 ct = xp;
		while (itemRef && ct < iconperline) {
			if ((classMask == 0) || ((itemRef->classFlags & classMask) != 0)) {
				if (hasIcon(itemRef)) {
					ct += 45;
					k++;
				}
			}
			itemRef = derefItem(itemRef->next);
		}
		line -= 52;
		if (k == (flagnumber + 18))
			k = flagnumber;
	}
	yp -= line;	// Adjust starting y

	if (itemRef == NULL) {
		window->iconPtr->line = 0;
		itemRef = derefItem(item_ptr_org->child);
	}

	while (itemRef) {
		if ((classMask != 0) && ((itemRef->classFlags & classMask) == 0))
			goto l1;
		if (hasIcon(itemRef) == 0)
			goto l1;
		if (!idone) {
/*
 *	Create thee icon and graphics rendering
 */
			window->iconPtr->iconArray[icount].item = itemRef;
			_variableArray[k] = itemGetIconNumber(itemRef);
			window->iconPtr->iconArray[icount++].boxCode =
				setupIconHitArea(window, k++, xp, yp, itemRef);
		} else {
/*
 *	Just remember the overflow has occurred
 */
			window->iconPtr->iconArray[icount].item = NULL;	/* END MARKINGS */
			_iOverflow = 1;
		}
		xp += 45;
		if (xp >= iconperline) {	/* End of line ? */
			if (k == (flagnumber + 18))
				k = flagnumber;
			xp = 188;
			yp += 52;		/* Move down */
			if (yp >= iconsdown) {	/* Full ? */
				idone = 1;	/* Note completed screen */
			}
		}
l1:;		itemRef = derefItem(itemRef->next);
	}
	window->iconPtr->iconArray[icount].item = NULL;	/* END MARKINGS */
	if (_variableArray[30] == 0) {
		if (yp != 306)
			_variableArray[31] = 52;
		if ((xp == 188) && (yp == 358))
			_variableArray[31] = 0;
	}

	/* Plot arrows and add their boxes */
	addArrows(window, num);
	window->iconPtr->upArrow = _scrollUpHitArea;
	window->iconPtr->downArrow = _scrollDownHitArea;
}
#endif

void AGOSEngine::drawIconArray(uint num, Item *itemRef, int line, int classMask) {
	Item *item_ptr_org = itemRef;
	WindowBlock *window;
	uint width, height;
	uint k, curWidth;
	bool item_again, showArrows;
	uint x_pos, y_pos;
	const int iconSize = (getGameType() == GType_SIMON2) ? 20 : 1;

	window = _windowArray[num & 7];

	if (getGameType() == GType_SIMON2) {
		width = 100;
		height = 40;
	} else if (getGameType() == GType_WW) {
		width = window->width / 3;
		height = window->height / 2;
	} else {
		width = window->width / 3;
		height = window->height / 3;
	}

	if (window == NULL)
		return;

	if (window->iconPtr)
		removeIconArray(num);

	window->iconPtr = (IconBlock *) malloc(sizeof(IconBlock));
	window->iconPtr->itemRef = itemRef;
	window->iconPtr->upArrow = -1;
	window->iconPtr->downArrow = -1;
	window->iconPtr->line = line;
	window->iconPtr->classMask = classMask;

	itemRef = derefItem(itemRef->child);

	while (itemRef && line-- != 0) {
		curWidth = 0;
		while (itemRef && width > curWidth) {
			if ((classMask == 0 || itemRef->classFlags & classMask) && hasIcon(itemRef))
				curWidth += iconSize;
			itemRef = derefItem(itemRef->next);
		}
	}

	if (itemRef == NULL) {
		window->iconPtr->line = 0;
		itemRef = derefItem(item_ptr_org->child);
	}

	x_pos = 0;
	y_pos = 0;
	k = 0;
	item_again = false;
	showArrows = false;

	while (itemRef) {
		if ((classMask == 0 || itemRef->classFlags & classMask) && hasIcon(itemRef)) {
			if (item_again == false) {
				window->iconPtr->iconArray[k].item = itemRef;
				if (getGameType() == GType_SIMON2) {
					drawIcon(window, itemGetIconNumber(itemRef), x_pos, y_pos);
					window->iconPtr->iconArray[k].boxCode =
						setupIconHitArea(window, 0, x_pos, y_pos, itemRef);
				} else if (getGameType() == GType_SIMON1 || getGameType() == GType_WW) {
					drawIcon(window, itemGetIconNumber(itemRef), x_pos * 3, y_pos);
					window->iconPtr->iconArray[k].boxCode =
						setupIconHitArea(window, 0, x_pos * 3, y_pos, itemRef);
				} else {
					drawIcon(window, itemGetIconNumber(itemRef), x_pos * 3, y_pos * 3);
					window->iconPtr->iconArray[k].boxCode =
						setupIconHitArea(window, 0, x_pos * 3, y_pos * 3, itemRef);
				}
				k++;
			} else {
				window->iconPtr->iconArray[k].item = NULL;
				showArrows = 1;
			}

			x_pos += iconSize;
			if (x_pos >= width) {
				x_pos = 0;
				y_pos += iconSize;
				if (y_pos >= height)
					item_again = true;
			}
		}
		itemRef = derefItem(itemRef->next);
	}

	window->iconPtr->iconArray[k].item = NULL;

	if (showArrows != 0 || window->iconPtr->line != 0) {
		/* Plot arrows and add their boxes */
		addArrows(window, num);
		window->iconPtr->upArrow = _scrollUpHitArea;
		window->iconPtr->downArrow = _scrollDownHitArea;
	}
}

#ifdef ENABLE_AGOS2
uint AGOSEngine_Feeble::setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr) {
	HitArea *ha = findEmptyHitArea();

	ha->x = x;
	ha->y = y;
	ha->itemPtr = itemPtr;
	ha->width = 45;
	ha->height = 44;
	ha->flags = kBFBoxInUse | kBFBoxItem;
	ha->id = num;
	ha->priority = 100;
	ha->verb = 208;

	return ha - _hitAreas;
}
#endif

uint AGOSEngine_Simon2::setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr) {
	HitArea *ha = findEmptyHitArea();

	ha->x = x + 110;
	ha->y = window->y + y;
	ha->itemPtr = itemPtr;
	ha->width = 20;
	ha->height = 20;
	ha->flags = kBFDragBox | kBFBoxInUse | kBFBoxItem;
	ha->id = 0x7FFD;
	ha->priority = 100;
	ha->verb = 208;

	return ha - _hitAreas;
}

uint AGOSEngine_Simon1::setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr) {
	HitArea *ha = findEmptyHitArea();

	ha->x = (x + window->x) * 8;
	ha->y = y * 25 + window->y;
	ha->itemPtr = itemPtr;
	ha->width = 24;
	ha->height = 24;
	ha->flags = kBFDragBox | kBFBoxInUse | kBFBoxItem;
	ha->id = 0x7FFD;
	ha->priority = 100;
	ha->verb = 208;

	return ha - _hitAreas;
}

uint AGOSEngine_Waxworks::setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr) {
	HitArea *ha = findEmptyHitArea();

	ha->x = (x + window->x) * 8;
	ha->y = y * 20 + window->y;
	ha->itemPtr = itemPtr;
	ha->width = 24;
	ha->height = 20;
	ha->flags = kBFDragBox | kBFBoxInUse | kBFBoxItem;
	ha->id = 0x7FFD;
	ha->priority = 100;
	ha->verb = 208;

	return ha - _hitAreas;
}

uint AGOSEngine_Elvira2::setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr) {
	HitArea *ha = findEmptyHitArea();

	ha->x = (x + window->x) * 8;
	ha->y = y * 8 + window->y;
	ha->itemPtr = itemPtr;
	ha->width = 24;
	ha->height = 24;
	ha->id = 0x7FFD;
	ha->priority = 100;

	if (window->iconPtr->classMask == 2) {
		ha->flags = kBFDragBox | kBFBoxInUse;
		ha->verb = 248 + 0x4000;
	} else {
		ha->flags = kBFDragBox | kBFBoxInUse | kBFBoxItem;
		ha->verb = 208;
	}

	return ha - _hitAreas;
}

uint AGOSEngine::setupIconHitArea(WindowBlock *window, uint num, uint x, uint y, Item *itemPtr) {
	HitArea *ha = findEmptyHitArea();

	ha->x = (x + window->x) * 8;
	ha->y = y * 8 + window->y;
	ha->itemPtr = itemPtr;
	ha->width = 24;
	ha->height = 24;
	ha->flags = kBFDragBox | kBFBoxInUse | kBFBoxItem;
	ha->id = 0x7FFD;
	ha->priority = 100;
	ha->verb = 253;

	return ha - _hitAreas;
}

#ifdef ENABLE_AGOS2
void AGOSEngine_Feeble::addArrows(WindowBlock *window, uint8 num) {
	HitArea *ha;

	ha = findEmptyHitArea();
	_scrollUpHitArea = ha - _hitAreas;

	ha->x = 496;
	ha->y = 279;
	ha->width = 30;
	ha->height = 45;
	ha->flags = kBFBoxInUse | kBFNoTouchName;
	ha->id = 0x7FFB;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;

	ha = findEmptyHitArea();
	_scrollDownHitArea = ha - _hitAreas;

	ha->x = 496;
	ha->y = 324;
	ha->width = 30;
	ha->height = 44;
	ha->flags = kBFBoxInUse | kBFNoTouchName;
	ha->id = 0x7FFC;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;
}
#endif

void AGOSEngine_Simon2::addArrows(WindowBlock *window, uint8 num) {
	HitArea *ha;

	ha = findEmptyHitArea();
	_scrollUpHitArea = ha - _hitAreas;

	ha->x = 81;
	ha->y = 158;
	ha->width = 12;
	ha->height = 26;
	ha->flags = kBFBoxInUse | kBFNoTouchName;
	ha->id = 0x7FFB;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;

	ha = findEmptyHitArea();
	_scrollDownHitArea = ha - _hitAreas;

	ha->x = 227;
	ha->y = 162;
	ha->width = 12;
	ha->height = 26;
	ha->flags = kBFBoxInUse | kBFNoTouchName;
	ha->id = 0x7FFC;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;
}

void AGOSEngine_Simon1::addArrows(WindowBlock *window, uint8 num) {
	HitArea *ha;

	ha = findEmptyHitArea();
	_scrollUpHitArea = ha - _hitAreas;

	ha->x = 308;
	ha->y = 149;
	ha->width = 12;
	ha->height = 17;
	ha->flags = kBFBoxInUse | kBFNoTouchName;
	ha->id = 0x7FFB;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;

	ha = findEmptyHitArea();
	_scrollDownHitArea = ha - _hitAreas;

	ha->x = 308;
	ha->y = 176;
	ha->width = 12;
	ha->height = 17;
	ha->flags = kBFBoxInUse | kBFNoTouchName;
	ha->id = 0x7FFC;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;

	_videoLockOut |= 0x8;

	VgaPointersEntry *vpe = &_vgaBufferPointers[1];
	byte *curVgaFile2Orig = _curVgaFile2;
	uint16 windowNumOrig = _windowNum;
	uint8 palette = (getPlatform() == Common::kPlatformAmiga) ? 15 : 14;

	_windowNum = 0;
	_curVgaFile2 = vpe->vgaFile2;
	drawImage_init(1, palette, 38, 150, 4);

	_curVgaFile2 = curVgaFile2Orig;
	_windowNum = windowNumOrig;

	_videoLockOut &= ~0x8;
}

void AGOSEngine_Waxworks::addArrows(WindowBlock *window, uint8 num) {
	HitArea *ha;

	ha = findEmptyHitArea();
	_scrollUpHitArea = ha - _hitAreas;

	setBitFlag(22, true);
	ha->x = 255;
	ha->y = 153;
	ha->width = 9;
	ha->height = 11;
	ha->flags = kBFBoxInUse | kBFNoTouchName;
	ha->id = 0x7FFB;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;

	ha = findEmptyHitArea();
	_scrollDownHitArea = ha - _hitAreas;

	ha->x = 255;
	ha->y = 170;
	ha->width = 9;
	ha->height = 11;
	ha->flags = kBFBoxInUse | kBFNoTouchName;
	ha->id = 0x7FFC;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;
	setWindowImageEx(6, 103);
}

void AGOSEngine_Elvira2::addArrows(WindowBlock *window, uint8 num) {
	HitArea *ha;

	ha = findEmptyHitArea();
	_scrollUpHitArea = ha - _hitAreas;

	setBitFlag(21, true);
	ha->x = 54;
	ha->y = 154;
	ha->width = 12;
	ha->height = 10;
	ha->flags = kBFBoxInUse;
	ha->id = 0x7FFB;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;

	ha = findEmptyHitArea();
	_scrollDownHitArea = ha - _hitAreas;

	ha->x = 54;
	ha->y = 178;
	ha->width = 12;
	ha->height = 10;
	ha->flags = kBFBoxInUse;
	ha->id = 0x7FFC;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;
	setWindowImageEx(6, 106);
}

void AGOSEngine::addArrows(WindowBlock *window, uint8 num) {
	HitArea *ha;
	uint16 x, y;

	x = 30;
	y = 151;
	if (num != 2) {
		y = window->y + window->height * 4 - 19;
		x = window->x + window->width;
	}
	drawArrow(x, y, 16);

	ha = findEmptyHitArea();
	_scrollUpHitArea = ha - _hitAreas;

	ha->x = x * 8;
	ha->y = y;
	ha->width = 16;
	ha->height = 19;
	ha->flags = kBFBoxInUse;
	ha->id = 0x7FFB;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;

	x = 30;
	y = 170;
	if (num != 2) {
		y = window->y + window->height * 4;
		x = window->x + window->width;
	}
	drawArrow(x, y, -16);

	ha = findEmptyHitArea();
	_scrollDownHitArea = ha - _hitAreas;

	ha->x = x * 8;
	ha->y = y;
	ha->width = 16;
	ha->height = 19;
	ha->flags = kBFBoxInUse;
	ha->id = 0x7FFC;
	ha->priority = 100;
	ha->window = window;
	ha->verb = 1;
}

static const byte _arrowImage[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a,
	0x0b, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x0b,
	0x0a, 0x0b, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x0b, 0x0a,
	0x0d, 0x0a, 0x0b, 0x0a, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0a, 0x0b, 0x0a, 0x0d,
	0x03, 0x0d, 0x0a, 0x0b, 0x0a, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x0a, 0x0b, 0x0a, 0x0d, 0x03,
	0x04, 0x03, 0x0d, 0x0a, 0x0b, 0x0a, 0x00, 0x00,
	0x00, 0x00, 0x0a, 0x0b, 0x0a, 0x0d, 0x03, 0x04,
	0x0f, 0x04, 0x03, 0x0d, 0x0a, 0x0b, 0x0a, 0x00,
	0x00, 0x0a, 0x0b, 0x0a, 0x0d, 0x0d, 0x0d, 0x03,
	0x04, 0x03, 0x0d, 0x0d, 0x0d, 0x0a, 0x0b, 0x0a,
	0x00, 0x0b, 0x0a, 0x0a, 0x0a, 0x0a, 0x09, 0x0d,
	0x03, 0x0d, 0x09, 0x0a, 0x0a, 0x0a, 0x0a, 0x0b,
	0x00, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0a, 0x0d,
	0x0d, 0x0d, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
	0x00, 0x0a, 0x0a, 0x0a, 0x0e, 0x0b, 0x0b, 0x0c,
	0x0e, 0x0c, 0x0b, 0x0b, 0x0e, 0x0a, 0x0a, 0x0a,
	0x00, 0x00, 0x02, 0x02, 0x0a, 0x0b, 0x0a, 0x0d,
	0x0d, 0x0d, 0x0a, 0x0b, 0x0a, 0x02, 0x02, 0x00,
	0x00, 0x00, 0x00, 0x02, 0x0a, 0x0b, 0x0b, 0x0c,
	0x0e, 0x0c, 0x0b, 0x0b, 0x0a, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0a, 0x0b, 0x0a, 0x0d,
	0x0d, 0x0d, 0x0a, 0x0b, 0x0a, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0a, 0x0b, 0x0b, 0x0c,
	0x0e, 0x0c, 0x0b, 0x0b, 0x0a, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0a, 0x0b, 0x0b, 0x0b,
	0x0b, 0x0b, 0x0b, 0x0b, 0x0a, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x02, 0x0e, 0x0a, 0x0a,
	0x0e, 0x0a, 0x0a, 0x0e, 0x02, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00,
	0x0a, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
};

void AGOSEngine::drawArrow(uint16 x, uint16 y, int8 dir) {
	const byte *src;
	uint8 w, h;

	if (dir < 0) {
		src = _arrowImage + 288;
	} else {
		src = _arrowImage;
	}

	Graphics::Surface *screen = _system->lockScreen();
	byte *dst = (byte *)screen->pixels + y * screen->pitch + x * 8;

	for (h = 0; h < 19; h++) {
		for (w = 0; w < 16; w++) {
			if (src[w])
				dst[w] = src[w] + 16;
		}

		src += dir;
		dst+= screen->pitch;
	}

	_system->unlockScreen();
}

void AGOSEngine_Simon1::removeArrows(WindowBlock *window, uint num) {
	if (getGameType() == GType_SIMON1) {
		restoreBlock(304, 146, 320, 200);
	}
}

void AGOSEngine_Waxworks::removeArrows(WindowBlock *window, uint num) {
	setBitFlag(22, false);
	setWindowImageEx(6, 103);
}

void AGOSEngine_Elvira2::removeArrows(WindowBlock *window, uint num) {
	setBitFlag(21, false);
	setWindowImageEx(6, 106);
}

void AGOSEngine::removeArrows(WindowBlock *window, uint num) {
	if (num != 2) {
		uint y = window->y + window->height * 4 - 19;
		uint x = (window->x + window->width) * 8;
		restoreBlock(x, y, x + 16, y + 38);
	} else {
		colorBlock(window, 240, 151, 16, 38);
	}
}

void AGOSEngine::removeIconArray(uint num) {
	WindowBlock *window;
	uint16 curWindow;
	uint16 i;

	window = _windowArray[num & 7];
	curWindow = _curWindow;

	if (window == NULL || window->iconPtr == NULL)
		return;

	if (getGameType() != GType_FF && getGameType() != GType_PP) {
		changeWindow(num);
		sendWindow(12);
		changeWindow(curWindow);
	}

	for (i = 0; window->iconPtr->iconArray[i].item != NULL; i++) {
		freeBox(window->iconPtr->iconArray[i].boxCode);
	}

	if (window->iconPtr->upArrow != -1) {
		freeBox(window->iconPtr->upArrow);
	}

	if (window->iconPtr->downArrow != -1) {
		freeBox(window->iconPtr->downArrow);
		removeArrows(window, num);
	}

	free(window->iconPtr);
	window->iconPtr = NULL;

	_fcsData1[num] = 0;
	_fcsData2[num] = 0;
}

static const byte hitBarData[12 * 7] = {
	0x3C, 0x00, 0x80, 0x00, 0x88, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00,
	0x20, 0x00, 0x04, 0x00, 0xD8, 0x00, 0x00, 0x04, 0x48, 0x00, 0x00, 0x00,
	0x20, 0x89, 0x8E, 0x00, 0xA8, 0x86, 0x10, 0x04, 0x08, 0x21, 0x88, 0x00,
	0x38, 0x50, 0x84, 0x00, 0x89, 0x49, 0x28, 0x04, 0x08, 0x52, 0x14, 0x00,
	0x20, 0x20, 0x84, 0x00, 0x89, 0x48, 0x38, 0x04, 0x08, 0x53, 0x9C, 0x00,
	0x20, 0x50, 0x84, 0x00, 0x89, 0x48, 0x20, 0x04, 0x48, 0x50, 0x90, 0x00,
	0x3C, 0x89, 0xC3, 0x00, 0x88, 0x88, 0x18, 0x03, 0x86, 0x23, 0x0C, 0x00
};

// Personal Nightmare specific
void AGOSEngine_PN::drawIconHitBar() {
	Graphics::Surface *screen = _system->lockScreen();
	byte *dst = (byte *)screen->pixels + 3 * screen->pitch + 6 * 8;
	const byte *src = hitBarData;
	uint8 color = (getPlatform() == Common::kPlatformPC) ? 7 : 15;

	for (int h = 0; h < 7; h++) {
		for (int w = 0; w < 12; w++) {
			int8 b = *src++;
			for (int i = 0; i < 8; i++) {
				if (b < 0) {
					dst[w * 8 + i] = color;
				}

				b <<= 1;
			}
		}
		dst += screen->pitch;
	}

	_system->unlockScreen();
}

void AGOSEngine_PN::iconPage() {
	_objectCountS = -1;

	mouseOff();

	uint8 objRoom = getptr(_quickptr[12] + _variableArray[210] * _quickshort[5] + 20);
	uint8 iconNum = getptr(_quickptr[0] + objRoom * _quickshort[0] + 4);

	drawIcon(NULL, iconNum, 6, 12);

	HitArea *ha = _invHitAreas + 5;
	for (uint8 r = 0; r < 5; r++) {
		for (uint8 i = 0; i < 7; i++) {
			printIcon(ha, i, r);
			ha++;
		}
	}

	mouseOn();
}

bool AGOSEngine_PN::ifObjectInInv(uint16 a) {
	return _variableArray[210] == getptr(_quickptr[11] + a * _quickshort[4] + 2);
}

bool AGOSEngine_PN::testContainer(uint16 a) {
	return 	bitextract(_quickptr[1] + a * _quickshort[1], 0) != 0;
}

bool AGOSEngine_PN::testObvious(uint16 a) {
	return 	bitextract(_quickptr[1] + a * _quickshort[1], 4) != 0;
}

bool AGOSEngine_PN::testSeen(uint16 a) {
	return 	bitextract(_quickptr[1] + a * _quickshort[1], 3) != 0;
}

void AGOSEngine_PN::printIcon(HitArea *ha, uint8 i, uint8 r) {
	 if (_objects == _objectCountS) {
		ha->flags |= kOBFBoxDisabled;
		drawIcon(NULL, 0xFF, 12 + i * 3, 12 + 24 * r);
	} else {
		_objectCountS++;
		if (!ifObjectInInv(_objectCountS) || !testObvious(_objectCountS)) {
			printIcon(ha, i, r);
		} else {

			uint8 iconNum = getptr(_quickptr[0] + _objectCountS * _quickshort[0] + 4);
			drawIcon(NULL, iconNum, 12 + i * 3, 12 + 24 * r);

			ha->msg1 = _objectCountS | 0x8000;
			ha->flags &= ~kOBFBoxDisabled;
		}
	}
}

} // End of namespace AGOS
