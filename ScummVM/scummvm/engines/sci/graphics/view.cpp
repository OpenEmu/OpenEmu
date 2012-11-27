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

#include "sci/sci.h"
#include "sci/util.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/view.h"

namespace Sci {

GfxView::GfxView(ResourceManager *resMan, GfxScreen *screen, GfxPalette *palette, GuiResourceId resourceId)
	: _resMan(resMan), _screen(screen), _palette(palette), _resourceId(resourceId) {
	assert(resourceId != -1);
	_coordAdjuster = g_sci->_gfxCoordAdjuster;
	initData(resourceId);
}

GfxView::~GfxView() {
	// Iterate through the loops
	for (uint16 loopNum = 0; loopNum < _loopCount; loopNum++) {
		// and through the cells of each loop
		for (uint16 celNum = 0; celNum < _loop[loopNum].celCount; celNum++) {
			delete[] _loop[loopNum].cel[celNum].rawBitmap;
		}
		delete[] _loop[loopNum].cel;
	}
	delete[] _loop;

	_resMan->unlockResource(_resource);
}

static const byte EGAmappingStraight[SCI_VIEW_EGAMAPPING_SIZE] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

void GfxView::initData(GuiResourceId resourceId) {
	_resource = _resMan->findResource(ResourceId(kResourceTypeView, resourceId), true);
	if (!_resource) {
		error("view resource %d not found", resourceId);
	}
	_resourceData = _resource->data;
	_resourceSize = _resource->size;

	byte *celData, *loopData;
	uint16 celOffset;
	CelInfo *cel;
	uint16 celCount = 0;
	uint16 mirrorBits = 0;
	uint32 palOffset = 0;
	uint16 headerSize = 0;
	uint16 loopSize = 0, celSize = 0;
	int loopNo, celNo, EGAmapNr;
	byte seekEntry;
	bool isEGA = false;
	bool isCompressed = true;
	ViewType curViewType = _resMan->getViewType();

	_loopCount = 0;
	_embeddedPal = false;
	_EGAmapping = NULL;
	_sci2ScaleRes = SCI_VIEW_NATIVERES_NONE;
	_isScaleable = true;

	// we adjust inside getCelRect for SCI0EARLY (that version didn't have the +1 when calculating bottom)
	_adjustForSci0Early = getSciVersion() == SCI_VERSION_0_EARLY ? -1 : 0;

	// If we find an SCI1/SCI1.1 view (not amiga), we switch to that type for
	// EGA. This could get used to make view patches for EGA games, where the
	// new views include more colors. Users could manually adjust old views to
	// make them look better (like removing dithered colors that aren't caught
	// by our undithering or even improve the graphics overall).
	if (curViewType == kViewEga) {
		if (_resourceData[1] == 0x80) {
			curViewType = kViewVga;
		} else {
			if (READ_LE_UINT16(_resourceData + 4) == 1)
				curViewType = kViewVga11;
		}
	}

	switch (curViewType) {
	case kViewEga: // SCI0 (and Amiga 16 colors)
		isEGA = true;
	case kViewAmiga: // Amiga ECS (32 colors)
	case kViewAmiga64: // Amiga AGA (64 colors)
	case kViewVga: // View-format SCI1
		// LoopCount:WORD MirrorMask:WORD Version:WORD PaletteOffset:WORD LoopOffset0:WORD LoopOffset1:WORD...

		_loopCount = _resourceData[0];
		// bit 0x8000 of _resourceData[1] means palette is set
		if (_resourceData[1] & 0x40)
			isCompressed = false;
		mirrorBits = READ_LE_UINT16(_resourceData + 2);
		palOffset = READ_LE_UINT16(_resourceData + 6);

		if (palOffset && palOffset != 0x100) {
			// Some SCI0/SCI01 games also have an offset set. It seems that it
			// points to a 16-byte mapping table but on those games using that
			// mapping will actually screw things up. On the other side: VGA
			// SCI1 games have this pointing to a VGA palette and EGA SCI1 games
			// have this pointing to a 8x16 byte mapping table that needs to get
			// applied then.
			if (!isEGA) {
				_palette->createFromData(&_resourceData[palOffset], _resourceSize - palOffset, &_viewPalette);
				_embeddedPal = true;
			} else {
				// Only use the EGA-mapping, when being SCI1 EGA
				//  SCI1 VGA conversion games (which will get detected as SCI1EARLY/MIDDLE/LATE) have some views
				//  with broken mapping tables. I guess those games won't use the mapping, so I rather disable it
				//  for them
				if (getSciVersion() == SCI_VERSION_1_EGA_ONLY) {
					_EGAmapping = &_resourceData[palOffset];
					for (EGAmapNr = 0; EGAmapNr < SCI_VIEW_EGAMAPPING_COUNT; EGAmapNr++) {
						if (memcmp(_EGAmapping, EGAmappingStraight, SCI_VIEW_EGAMAPPING_SIZE) != 0)
							break;
						_EGAmapping += SCI_VIEW_EGAMAPPING_SIZE;
					}
					// If all mappings are "straight", then we actually ignore the mapping
					if (EGAmapNr == SCI_VIEW_EGAMAPPING_COUNT)
						_EGAmapping = NULL;
					else
						_EGAmapping = &_resourceData[palOffset];
				}
			}
		}

		_loop = new LoopInfo[_loopCount];
		for (loopNo = 0; loopNo < _loopCount; loopNo++) {
			loopData = _resourceData + READ_LE_UINT16(_resourceData + 8 + loopNo * 2);
			// CelCount:WORD Unknown:WORD CelOffset0:WORD CelOffset1:WORD...

			celCount = READ_LE_UINT16(loopData);
			_loop[loopNo].celCount = celCount;
			_loop[loopNo].mirrorFlag = mirrorBits & 1 ? true : false;
			mirrorBits >>= 1;

			// read cel info
			_loop[loopNo].cel = new CelInfo[celCount];
			for (celNo = 0; celNo < celCount; celNo++) {
				celOffset = READ_LE_UINT16(loopData + 4 + celNo * 2);
				celData = _resourceData + celOffset;

				// For VGA
				// Width:WORD Height:WORD DisplaceX:BYTE DisplaceY:BYTE ClearKey:BYTE Unknown:BYTE RLEData starts now directly
				// For EGA
				// Width:WORD Height:WORD DisplaceX:BYTE DisplaceY:BYTE ClearKey:BYTE EGAData starts now directly
				cel = &_loop[loopNo].cel[celNo];
				cel->scriptWidth = cel->width = READ_LE_UINT16(celData);
				cel->scriptHeight = cel->height = READ_LE_UINT16(celData + 2);
				cel->displaceX = (signed char)celData[4];
				cel->displaceY = celData[5];
				cel->clearKey = celData[6];

				// HACK: Fix Ego's odd displacement in the QFG3 demo, scene 740.
				// For some reason, ego jumps above the rope, so we fix his rope
				// hanging view by displacing it down by 40 pixels. Fixes bug
				// #3035693.
				// FIXME: Remove this once we figure out why Ego jumps so high.
				// Likely culprits include kInitBresen, kDoBresen and kCantBeHere.
				// The scripts have the y offset that hero reaches (11) hardcoded,
				// so it might be collision detection. However, since this requires
				// extensive work to fix properly for very little gain, this hack
				// here will suffice until the actual issue is found.
				if (g_sci->getGameId() == GID_QFG3 && g_sci->isDemo() && resourceId == 39)
					cel->displaceY = 98;

				if (isEGA) {
					cel->offsetEGA = celOffset + 7;
					cel->offsetRLE = 0;
					cel->offsetLiteral = 0;
				} else {
					cel->offsetEGA = 0;
					if (isCompressed) {
						cel->offsetRLE = celOffset + 8;
						cel->offsetLiteral = 0;
					} else {
						cel->offsetRLE = 0;
						cel->offsetLiteral = celOffset + 8;
					}
				}
				cel->rawBitmap = 0;
				if (_loop[loopNo].mirrorFlag)
					cel->displaceX = -cel->displaceX;
			}
		}
		break;

	case kViewVga11: // View-format SCI1.1+
		// HeaderSize:WORD LoopCount:BYTE Flags:BYTE Version:WORD Unknown:WORD PaletteOffset:WORD
		headerSize = READ_SCI11ENDIAN_UINT16(_resourceData + 0) + 2; // headerSize is not part of the header, so it's added
		assert(headerSize >= 16);
		_loopCount = _resourceData[2];
		assert(_loopCount);
		palOffset = READ_SCI11ENDIAN_UINT32(_resourceData + 8);

		// For SCI32, this is a scale flag
		if (getSciVersion() >= SCI_VERSION_2) {
			_sci2ScaleRes = (Sci32ViewNativeResolution)_resourceData[5];
			if (_screen->getUpscaledHires() == GFX_SCREEN_UPSCALED_DISABLED)
				_sci2ScaleRes = SCI_VIEW_NATIVERES_NONE;
		}

		// flags is actually a bit-mask
		//  it seems it was only used for some early sci1.1 games (or even just laura bow 2)
		//  later interpreters dont support it at all anymore
		// we assume that if flags is 0h the view does not support flags and default to scaleable
		// if it's 1h then we assume that the view is not to be scaled
		// if it's 40h then we assume that the view is scaleable
		switch (_resourceData[3]) {
		case 1:
			_isScaleable = false;
			break;
		case 0x40:
		case 0:
			break; // don't do anything, we already have _isScaleable set
		default:
			error("unsupported flags byte (%d) inside sci1.1 view", _resourceData[3]);
			break;
		}

		loopData = _resourceData + headerSize;
		loopSize = _resourceData[12];
		assert(loopSize >= 16);
		celSize = _resourceData[13];
		assert(celSize >= 32);

		if (palOffset) {
			_palette->createFromData(&_resourceData[palOffset], _resourceSize - palOffset, &_viewPalette);
			_embeddedPal = true;
		}

		_loop = new LoopInfo[_loopCount];
		for (loopNo = 0; loopNo < _loopCount; loopNo++) {
			loopData = _resourceData + headerSize + (loopNo * loopSize);

			seekEntry = loopData[0];
			if (seekEntry != 255) {
				if (seekEntry >= _loopCount)
					error("Bad loop-pointer in sci 1.1 view");
				_loop[loopNo].mirrorFlag = true;
				loopData = _resourceData + headerSize + (seekEntry * loopSize);
			} else {
				_loop[loopNo].mirrorFlag = false;
			}

			celCount = loopData[2];
			_loop[loopNo].celCount = celCount;

			celData = _resourceData + READ_SCI11ENDIAN_UINT32(loopData + 12);

			// read cel info
			_loop[loopNo].cel = new CelInfo[celCount];
			for (celNo = 0; celNo < celCount; celNo++) {
				cel = &_loop[loopNo].cel[celNo];
				cel->scriptWidth = cel->width = READ_SCI11ENDIAN_UINT16(celData);
				cel->scriptHeight = cel->height = READ_SCI11ENDIAN_UINT16(celData + 2);
				cel->displaceX = READ_SCI11ENDIAN_UINT16(celData + 4);
				cel->displaceY = READ_SCI11ENDIAN_UINT16(celData + 6);
				if (cel->displaceY < 0)
					cel->displaceY += 255; // sierra did this adjust in their sci1.1 getCelRect() - not sure about sci32

				assert(cel->width && cel->height);

				cel->clearKey = celData[8];
				cel->offsetEGA = 0;
				cel->offsetRLE = READ_SCI11ENDIAN_UINT32(celData + 24);
				cel->offsetLiteral = READ_SCI11ENDIAN_UINT32(celData + 28);

				// GK1-hires content is actually uncompressed, we need to swap both so that we process it as such
				if ((cel->offsetRLE) && (!cel->offsetLiteral))
					SWAP(cel->offsetRLE, cel->offsetLiteral);

				cel->rawBitmap = 0;
				if (_loop[loopNo].mirrorFlag)
					cel->displaceX = -cel->displaceX;

				celData += celSize;
			}
		}
#ifdef ENABLE_SCI32
		// adjust width/height returned to scripts
		if (_sci2ScaleRes != SCI_VIEW_NATIVERES_NONE) {
			for (loopNo = 0; loopNo < _loopCount; loopNo++)
				for (celNo = 0; celNo < _loop[loopNo].celCount; celNo++)
					_screen->adjustBackUpscaledCoordinates(_loop[loopNo].cel[celNo].scriptWidth, _loop[loopNo].cel[celNo].scriptHeight, _sci2ScaleRes);
		} else if (getSciVersion() == SCI_VERSION_2_1) {
			for (loopNo = 0; loopNo < _loopCount; loopNo++)
				for (celNo = 0; celNo < _loop[loopNo].celCount; celNo++)
					_coordAdjuster->fromDisplayToScript(_loop[loopNo].cel[celNo].scriptHeight, _loop[loopNo].cel[celNo].scriptWidth);
		}
#endif
		break;

	default:
		error("ViewType was not detected, can't continue");
	}
}

GuiResourceId GfxView::getResourceId() const {
	return _resourceId;
}

int16 GfxView::getWidth(int16 loopNo, int16 celNo) const {
	return _loopCount ? getCelInfo(loopNo, celNo)->width : 0;
}

int16 GfxView::getHeight(int16 loopNo, int16 celNo) const {
	return _loopCount ? getCelInfo(loopNo, celNo)->height : 0;
}

const CelInfo *GfxView::getCelInfo(int16 loopNo, int16 celNo) const {
	assert(_loopCount);
	loopNo = CLIP<int16>(loopNo, 0, _loopCount - 1);
	celNo = CLIP<int16>(celNo, 0, _loop[loopNo].celCount - 1);
	return &_loop[loopNo].cel[celNo];
}

uint16 GfxView::getCelCount(int16 loopNo) const {
	assert(_loopCount);
	loopNo = CLIP<int16>(loopNo, 0, _loopCount - 1);
	return _loop[loopNo].celCount;
}

Palette *GfxView::getPalette() {
	return _embeddedPal ? &_viewPalette : NULL;
}

bool GfxView::isSci2Hires() {
	return _sci2ScaleRes > SCI_VIEW_NATIVERES_320x200;
}

bool GfxView::isScaleable() {
	return _isScaleable;
}

void GfxView::getCelRect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, Common::Rect &outRect) const {
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	outRect.left = x + celInfo->displaceX - (celInfo->width >> 1);
	outRect.right = outRect.left + celInfo->width;
	outRect.bottom = y + celInfo->displaceY - z + 1 + _adjustForSci0Early;
	outRect.top = outRect.bottom - celInfo->height;
}

void GfxView::getCelSpecialHoyle4Rect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, Common::Rect &outRect) const {
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	int16 adjustY = y + celInfo->displaceY - celInfo->height + 1;
	int16 adjustX = x + celInfo->displaceX - ((celInfo->width - 1) >> 1);
	outRect.translate(adjustX, adjustY);
}

void GfxView::getCelScaledRect(int16 loopNo, int16 celNo, int16 x, int16 y, int16 z, int16 scaleX, int16 scaleY, Common::Rect &outRect) const {
	int16 scaledDisplaceX, scaledDisplaceY;
	int16 scaledWidth, scaledHeight;
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);

	// Scaling displaceX/Y, Width/Height
	scaledDisplaceX = (celInfo->displaceX * scaleX) >> 7;
	scaledDisplaceY = (celInfo->displaceY * scaleY) >> 7;
	scaledWidth = (celInfo->width * scaleX) >> 7;
	scaledHeight = (celInfo->height * scaleY) >> 7;
	scaledWidth = CLIP<int16>(scaledWidth, 0, _screen->getWidth());
	scaledHeight = CLIP<int16>(scaledHeight, 0, _screen->getHeight());

	outRect.left = x + scaledDisplaceX - (scaledWidth >> 1);
	outRect.right = outRect.left + scaledWidth;
	outRect.bottom = y + scaledDisplaceY - z + 1;
	outRect.top = outRect.bottom - scaledHeight;
}

void unpackCelData(byte *inBuffer, byte *celBitmap, byte clearColor, int pixelCount, int rlePos, int literalPos, ViewType viewType, uint16 width, bool isMacSci11ViewData) {
	byte *outPtr = celBitmap;
	byte curByte, runLength;
	byte *rlePtr = inBuffer + rlePos;
	// The existence of a literal position pointer signifies data with two
	// separate streams, most likely a SCI1.1 view
	byte *literalPtr = inBuffer + literalPos;
	int pixelNr = 0;

	memset(celBitmap, clearColor, pixelCount);

	// View unpacking:
	//
	// EGA:
	// Each byte is like XXXXYYYY (XXXX: 0 - 15, YYYY: 0 - 15)
	// Set the next XXXX pixels to YYYY
	//
	// Amiga:
	// Each byte is like XXXXXYYY (XXXXX: 0 - 31, YYY: 0 - 7)
	// - Case A: YYY != 0
	//   Set the next YYY pixels to XXXXX
	// - Case B: YYY == 0
	//   Skip the next XXXXX pixels (i.e. transparency)
	//
	// Amiga 64:
	// Each byte is like XXYYYYYY (XX: 0 - 3, YYYYYY: 0 - 63)
	// - Case A: XX != 0
	//   Set the next XX pixels to YYYYYY
	// - Case B: XX == 0
	//   Skip the next YYYYYY pixels (i.e. transparency)
	//
	// VGA:
	// Each byte is like XXYYYYYY (YYYYY: 0 - 63)
	// - Case A: XX == 00 (binary)
	//   Copy next YYYYYY bytes as-is
	// - Case B: XX == 01 (binary)
	//   Same as above, copy YYYYYY + 64 bytes as-is
	// - Case C: XX == 10 (binary)
	//   Set the next YYYYY pixels to the next byte value
	// - Case D: XX == 11 (binary)
	//   Skip the next YYYYY pixels (i.e. transparency)

	if (literalPos && isMacSci11ViewData) {
		// KQ6/Freddy Pharkas/Slater use byte lengths, all others use uint16
		// The SCI devs must have realized that a max of 255 pixels wide
		// was not very good for 320 or 640 width games.
		bool hasByteLengths = (g_sci->getGameId() == GID_KQ6 || g_sci->getGameId() == GID_FREDDYPHARKAS
				|| g_sci->getGameId() == GID_SLATER);

		// compression for SCI1.1+ Mac
		while (pixelNr < pixelCount) {
			uint32 pixelLine = pixelNr;

			if (hasByteLengths) {
				pixelNr += *rlePtr++;
				runLength = *rlePtr++;
			} else {
				pixelNr += READ_BE_UINT16(rlePtr);
				runLength = READ_BE_UINT16(rlePtr + 2);
				rlePtr += 4;
			}

			while (runLength-- && pixelNr < pixelCount)
				outPtr[pixelNr++] = *literalPtr++;

			pixelNr = pixelLine + width;
		}
		return;
	}

	switch (viewType) {
	case kViewEga:
		while (pixelNr < pixelCount) {
			curByte = *rlePtr++;
			runLength = curByte >> 4;
			memset(outPtr + pixelNr,        curByte & 0x0F, MIN<uint16>(runLength, pixelCount - pixelNr));
			pixelNr += runLength;
		}
		break;
	case kViewAmiga:
		while (pixelNr < pixelCount) {
			curByte = *rlePtr++;
			if (curByte & 0x07) { // fill with color
				runLength = curByte & 0x07;
				curByte = curByte >> 3;
				memset(outPtr + pixelNr,           curByte, MIN<uint16>(runLength, pixelCount - pixelNr));
			} else { // skip the next pixels (transparency)
				runLength = curByte >> 3;
			}
			pixelNr += runLength;
		}
		break;
	case kViewAmiga64:
		while (pixelNr < pixelCount) {
			curByte = *rlePtr++;
			if (curByte & 0xC0) { // fill with color
				runLength = curByte >> 6;
				curByte = curByte & 0x3F;
				memset(outPtr + pixelNr,           curByte, MIN<uint16>(runLength, pixelCount - pixelNr));
			} else { // skip the next pixels (transparency)
				runLength = curByte & 0x3F;
			}
			pixelNr += runLength;
		}
		break;
	case kViewVga:
	case kViewVga11:
		// If we have no RLE data, the image is just uncompressed
		if (rlePos == 0) {
			memcpy(outPtr, literalPtr, pixelCount);
			break;
		}

		while (pixelNr < pixelCount) {
			curByte = *rlePtr++;
			runLength = curByte & 0x3F;

			switch (curByte & 0xC0) {
			case 0x40: // copy bytes as is (In copy case, runLength can go up to 127 i.e. pixel & 0x40). Fixes bug #3135872.
				runLength += 64;
			case 0x00: // copy bytes as-is
				if (!literalPos) {
					memcpy(outPtr + pixelNr,        rlePtr, MIN<uint16>(runLength, pixelCount - pixelNr));
					rlePtr += runLength;
				} else {
					memcpy(outPtr + pixelNr,    literalPtr, MIN<uint16>(runLength, pixelCount - pixelNr));
					literalPtr += runLength;
				}
				break;
			case 0x80: // fill with color
				if (!literalPos)
					memset(outPtr + pixelNr,     *rlePtr++, MIN<uint16>(runLength, pixelCount - pixelNr));
				else
					memset(outPtr + pixelNr, *literalPtr++, MIN<uint16>(runLength, pixelCount - pixelNr));
				break;
			case 0xC0: // skip the next pixels (transparency)
				break;
			}

			pixelNr += runLength;
		}
		break;
	default:
		error("Unsupported picture viewtype");
	}
}

void GfxView::unpackCel(int16 loopNo, int16 celNo, byte *outPtr, uint32 pixelCount) {
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);

	if (celInfo->offsetEGA) {
		// decompression for EGA views
		unpackCelData(_resourceData, outPtr, 0, pixelCount, celInfo->offsetEGA, 0, _resMan->getViewType(), celInfo->width, false);
	} else {
		// We fill the buffer with transparent pixels, so that we can later skip
		//  over pixels to automatically have them transparent
		// Also some RLE compressed cels are possibly ending with the last
		// non-transparent pixel (is this even possible with the current code?)
		byte clearColor = _loop[loopNo].cel[celNo].clearKey;

		// Since Mac OS required palette index 0 to be white and 0xff to be black, the
		// Mac SCI devs decided that rather than change scripts and various pieces of
		// code, that they would just put a little snippet of code to swap these colors
		// in various places around the SCI codebase. We figured that it would be less
		// hacky to swap pixels instead and run the Mac games with a PC palette.
		if (g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() >= SCI_VERSION_1_1) {
			// clearColor is based on PC palette, but the literal data is not.
			// We flip clearColor here to make it match the literal data. All
			// these pixels will be flipped back again below.
			if (clearColor == 0)
				clearColor = 0xff;
			else if (clearColor == 0xff)
				clearColor = 0;
		}

		bool isMacSci11ViewData = g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() == SCI_VERSION_1_1;
		unpackCelData(_resourceData, outPtr, clearColor, pixelCount, celInfo->offsetRLE, celInfo->offsetLiteral, _resMan->getViewType(), celInfo->width, isMacSci11ViewData);

		// Swap 0 and 0xff pixels for Mac SCI1.1+ games (see above)
		if (g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() >= SCI_VERSION_1_1) {
			for (uint32 i = 0; i < pixelCount; i++) {
				if (outPtr[i] == 0)
					outPtr[i] = 0xff;
				else if (outPtr[i] == 0xff)
					outPtr[i] = 0;
			}
		}
	}
}

const byte *GfxView::getBitmap(int16 loopNo, int16 celNo) {
	loopNo = CLIP<int16>(loopNo, 0, _loopCount -1);
	celNo = CLIP<int16>(celNo, 0, _loop[loopNo].celCount - 1);
	if (_loop[loopNo].cel[celNo].rawBitmap)
		return _loop[loopNo].cel[celNo].rawBitmap;

	uint16 width = _loop[loopNo].cel[celNo].width;
	uint16 height = _loop[loopNo].cel[celNo].height;
	// allocating memory to store cel's bitmap
	int pixelCount = width * height;
	_loop[loopNo].cel[celNo].rawBitmap = new byte[pixelCount];
	byte *pBitmap = _loop[loopNo].cel[celNo].rawBitmap;

	// unpack the actual cel bitmap data
	unpackCel(loopNo, celNo, pBitmap, pixelCount);

	if (_resMan->getViewType() == kViewEga)
		unditherBitmap(pBitmap, width, height, _loop[loopNo].cel[celNo].clearKey);

	// mirroring the cel if needed
	if (_loop[loopNo].mirrorFlag) {
		for (int i = 0; i < height; i++, pBitmap += width)
			for (int j = 0; j < width / 2; j++)
				SWAP(pBitmap[j], pBitmap[width - j - 1]);
	}
	return _loop[loopNo].cel[celNo].rawBitmap;
}

/**
 * Called after unpacking an EGA cel, this will try to undither (parts) of the
 * cel if the dithering in here matches dithering used by the current picture.
 */
void GfxView::unditherBitmap(byte *bitmapPtr, int16 width, int16 height, byte clearKey) {
	int16 *ditheredPicColors = _screen->unditherGetDitheredBgColors();

	// It makes no sense to go further, if there isn't any dithered color data
	// available for the current picture
	if (!ditheredPicColors)
		return;

	// We need at least a 4x2 bitmap for this algorithm to work
	if (width < 4 || height < 2)
		return;

	// If EGA mapping is used for this view, dont do undithering as well
	if (_EGAmapping)
		return;

	// Walk through the bitmap and remember all combinations of colors
	int16 ditheredBitmapColors[DITHERED_BG_COLORS_SIZE];
	byte *curPtr;
	byte color1, color2;
	byte nextColor1, nextColor2;
	int16 y, x;

	memset(&ditheredBitmapColors, 0, sizeof(ditheredBitmapColors));

	// Count all seemingly dithered pixel-combinations as soon as at least 4
	// pixels are adjacent and check pixels in the following line as well to
	// be the reverse pixel combination
	int16 checkHeight = height - 1;
	curPtr = bitmapPtr;
	byte *nextPtr = curPtr + width;
	for (y = 0; y < checkHeight; y++) {
		color1 = curPtr[0]; color2 = (curPtr[1] << 4) | curPtr[2];
		nextColor1 = nextPtr[0] << 4; nextColor2 = (nextPtr[2] << 4) | nextPtr[1];
		curPtr += 3;
		nextPtr += 3;
		for (x = 3; x < width; x++) {
			color1 = (color1 << 4) | (color2 >> 4);
			color2 = (color2 << 4) | *curPtr++;
			nextColor1 = (nextColor1 >> 4) | (nextColor2 << 4);
			nextColor2 = (nextColor2 >> 4) | *nextPtr++ << 4;
			if ((color1 == color2) && (color1 == nextColor1) && (color1 == nextColor2))
				ditheredBitmapColors[color1]++;
		}
	}

	// Now compare both dither color tables to find out matching dithered color
	// combinations
	bool unditherTable[DITHERED_BG_COLORS_SIZE];
	byte color, unditherCount = 0;
	memset(&unditherTable, false, sizeof(unditherTable));
	for (color = 0; color < 255; color++) {
		if ((ditheredBitmapColors[color] > 5) && (ditheredPicColors[color] > 200)) {
			// match found, check if colorKey is contained -> if so, we ignore
			// of course
			color1 = color & 0x0F; color2 = color >> 4;
			if ((color1 != clearKey) && (color2 != clearKey) && (color1 != color2)) {
				// so set this and the reversed color-combination for undithering
				unditherTable[color] = true;
				unditherTable[(color1 << 4) | color2] = true;
				unditherCount++;
			}
		}
	}

	// Nothing found to undither -> exit straight away
	if (!unditherCount)
		return;

	// We now need to replace color-combinations
	curPtr = bitmapPtr;
	for (y = 0; y < height; y++) {
		color = *curPtr;
		for (x = 1; x < width; x++) {
			color = (color << 4) | curPtr[1];
			if (unditherTable[color]) {
				// Some color with black? Turn colors around, otherwise it won't
				// be the right color at all.
				byte unditheredColor = color;
				if ((color & 0xF0) == 0)
					unditheredColor = (color << 4) | (color >> 4);
				curPtr[0] = unditheredColor; curPtr[1] = unditheredColor;
			}
			curPtr++;
		}
		curPtr++;
	}
}

void GfxView::draw(const Common::Rect &rect, const Common::Rect &clipRect, const Common::Rect &clipRectTranslated,
			int16 loopNo, int16 celNo, byte priority, uint16 EGAmappingNr, bool upscaledHires) {
	const Palette *palette = _embeddedPal ? &_viewPalette : &_palette->_sysPalette;
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	const byte *bitmap = getBitmap(loopNo, celNo);
	const int16 celHeight = celInfo->height;
	const int16 celWidth = celInfo->width;
	const byte clearKey = celInfo->clearKey;
	const byte drawMask = priority > 15 ? GFX_SCREEN_MASK_VISUAL : GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY;
	int x, y;

	if (_embeddedPal)
		// Merge view palette in...
		_palette->set(&_viewPalette, false);

	const int16 width = MIN(clipRect.width(), celWidth);
	const int16 height = MIN(clipRect.height(), celHeight);

	bitmap += (clipRect.top - rect.top) * celWidth + (clipRect.left - rect.left);

	// WORKAROUND: EcoQuest French and German draw the fish and anemone sprites
	// with priority 15 in scene 440. Afterwards, a dialog is shown on top of
	// these sprites with priority 15 as well. This is undefined behavior
	// actually, as the sprites and dialog share the same priority, so in our
	// implementation the sprites get drawn incorrectly on top of the dialog.
	// Perhaps this worked by mistake in SSCI because of subtle differences in
	// how sprites are drawn. We compensate for this by resetting the priority
	// of all sprites that have a priority of 15 in scene 440 to priority 14,
	// so that the speech bubble can be drawn correctly on top of them. Fixes
	// bug #3040625.
	if (g_sci->getGameId() == GID_ECOQUEST && g_sci->getEngineState()->currentRoomNumber() == 440 && priority == 15)
		priority = 14;

	if (!_EGAmapping) {
		for (y = 0; y < height; y++, bitmap += celWidth) {
			for (x = 0; x < width; x++) {
				const byte color = bitmap[x];
				if (color != clearKey) {
					const int x2 = clipRectTranslated.left + x;
					const int y2 = clipRectTranslated.top + y;
					if (!upscaledHires) {
						if (priority >= _screen->getPriority(x2, y2)) {
							if (!_palette->isRemapped(palette->mapping[color])) {
								_screen->putPixel(x2, y2, drawMask, palette->mapping[color], priority, 0);
							} else {
								byte remappedColor = _palette->remapColor(palette->mapping[color], _screen->getVisual(x2, y2));
								_screen->putPixel(x2, y2, drawMask, remappedColor, priority, 0);
							}
						}
					} else {
						// UpscaledHires means view is hires and is supposed to
						// get drawn onto lowres screen.
						// FIXME(?): we can't read priority directly with the
						// hires coordinates. May not be needed at all in kq6
						// FIXME: Handle proper aspect ratio. Some GK1 hires images
						// are in 640x400 instead of 640x480
						_screen->putPixelOnDisplay(x2, y2, palette->mapping[color]);
					}
				}
			}
		}
	} else {
		byte *EGAmapping = _EGAmapping + (EGAmappingNr * SCI_VIEW_EGAMAPPING_SIZE);
		for (y = 0; y < height; y++, bitmap += celWidth) {
			for (x = 0; x < width; x++) {
				const byte color = EGAmapping[bitmap[x]];
				const int x2 = clipRectTranslated.left + x;
				const int y2 = clipRectTranslated.top + y;
				if (color != clearKey && priority >= _screen->getPriority(x2, y2))
					_screen->putPixel(x2, y2, drawMask, color, priority, 0);
			}
		}
	}
}

/**
 * We don't fully follow sierra sci here, I did the scaling algo myself and it
 * is definitely not pixel-perfect with the one sierra is using. It shouldn't
 * matter because the scaled cel rect is definitely the same as in sierra sci.
 */
void GfxView::drawScaled(const Common::Rect &rect, const Common::Rect &clipRect, const Common::Rect &clipRectTranslated,
			int16 loopNo, int16 celNo, byte priority, int16 scaleX, int16 scaleY) {
	const Palette *palette = _embeddedPal ? &_viewPalette : &_palette->_sysPalette;
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	const byte *bitmap = getBitmap(loopNo, celNo);
	const int16 celHeight = celInfo->height;
	const int16 celWidth = celInfo->width;
	const byte clearKey = celInfo->clearKey;
	const byte drawMask = priority > 15 ? GFX_SCREEN_MASK_VISUAL : GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY;
	uint16 scalingX[640];
	uint16 scalingY[480];
	int16 scaledWidth, scaledHeight;
	int pixelNo, scaledPixel, scaledPixelNo, prevScaledPixelNo;

	if (_embeddedPal)
		// Merge view palette in...
		_palette->set(&_viewPalette, false);

	scaledWidth = (celInfo->width * scaleX) >> 7;
	scaledHeight = (celInfo->height * scaleY) >> 7;
	scaledWidth = CLIP<int16>(scaledWidth, 0, _screen->getWidth());
	scaledHeight = CLIP<int16>(scaledHeight, 0, _screen->getHeight());

	// Do we really need to do this?!
	//memset(scalingX, 0, sizeof(scalingX));
	//memset(scalingY, 0, sizeof(scalingY));

	// Create height scaling table
	pixelNo = 0;
	scaledPixel = scaledPixelNo = prevScaledPixelNo = 0;
	while (pixelNo < celHeight) {
		scaledPixelNo = scaledPixel >> 7;
		assert(scaledPixelNo < ARRAYSIZE(scalingY));
		for (; prevScaledPixelNo <= scaledPixelNo; prevScaledPixelNo++)
			scalingY[prevScaledPixelNo] = pixelNo;
		pixelNo++;
		scaledPixel += scaleY;
	}
	pixelNo--;
	scaledPixelNo++;
	for (; scaledPixelNo < scaledHeight; scaledPixelNo++)
		scalingY[scaledPixelNo] = pixelNo;

	// Create width scaling table
	pixelNo = 0;
	scaledPixel = scaledPixelNo = prevScaledPixelNo = 0;
	while (pixelNo < celWidth) {
		scaledPixelNo = scaledPixel >> 7;
		assert(scaledPixelNo < ARRAYSIZE(scalingX));
		for (; prevScaledPixelNo <= scaledPixelNo; prevScaledPixelNo++)
			scalingX[prevScaledPixelNo] = pixelNo;
		pixelNo++;
		scaledPixel += scaleX;
	}
	pixelNo--;
	scaledPixelNo++;
	for (; scaledPixelNo < scaledWidth; scaledPixelNo++)
		scalingX[scaledPixelNo] = pixelNo;

	scaledWidth = MIN(clipRect.width(), scaledWidth);
	scaledHeight = MIN(clipRect.height(), scaledHeight);

	const int16 offsetY = clipRect.top - rect.top;
	const int16 offsetX = clipRect.left - rect.left;

	// Happens in SQ6, first room
	if (offsetX < 0 || offsetY < 0)
		return;

	assert(scaledHeight + offsetY <= ARRAYSIZE(scalingY));
	assert(scaledWidth + offsetX <= ARRAYSIZE(scalingX));
	for (int y = 0; y < scaledHeight; y++) {
		for (int x = 0; x < scaledWidth; x++) {
			const byte color = bitmap[scalingY[y + offsetY] * celWidth + scalingX[x + offsetX]];
			const int x2 = clipRectTranslated.left + x;
			const int y2 = clipRectTranslated.top + y;
			if (color != clearKey && priority >= _screen->getPriority(x2, y2)) {
				if (!_palette->isRemapped(palette->mapping[color])) {
					_screen->putPixel(x2, y2, drawMask, palette->mapping[color], priority, 0);
				} else {
					byte remappedColor = _palette->remapColor(palette->mapping[color], _screen->getVisual(x2, y2));
					_screen->putPixel(x2, y2, drawMask, remappedColor, priority, 0);
				}
			}
		}
	}
}

void GfxView::adjustToUpscaledCoordinates(int16 &y, int16 &x) {
	_screen->adjustToUpscaledCoordinates(y, x, _sci2ScaleRes);
}

void GfxView::adjustBackUpscaledCoordinates(int16 &y, int16 &x) {
	_screen->adjustBackUpscaledCoordinates(y, x, _sci2ScaleRes);
}

byte GfxView::getColorAtCoordinate(int16 loopNo, int16 celNo, int16 x, int16 y) {
	const CelInfo *celInfo = getCelInfo(loopNo, celNo);
	const byte *bitmap = getBitmap(loopNo, celNo);
	const int16 celWidth = celInfo->width;

	bitmap += (celWidth * y);
	return bitmap[x];
}

} // End of namespace Sci
