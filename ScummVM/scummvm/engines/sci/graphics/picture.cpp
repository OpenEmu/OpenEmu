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

#include "common/stack.h"
#include "common/system.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/picture.h"

namespace Sci {

//#define DEBUG_PICTURE_DRAW

GfxPicture::GfxPicture(ResourceManager *resMan, GfxCoordAdjuster *coordAdjuster, GfxPorts *ports, GfxScreen *screen, GfxPalette *palette, GuiResourceId resourceId, bool EGAdrawingVisualize)
	: _resMan(resMan), _coordAdjuster(coordAdjuster), _ports(ports), _screen(screen), _palette(palette), _resourceId(resourceId), _EGAdrawingVisualize(EGAdrawingVisualize) {
	assert(resourceId != -1);
	initData(resourceId);
}

GfxPicture::~GfxPicture() {
	_resMan->unlockResource(_resource);
}

void GfxPicture::initData(GuiResourceId resourceId) {
	_resource = _resMan->findResource(ResourceId(kResourceTypePic, resourceId), true);
	if (!_resource) {
		error("picture resource %d not found", resourceId);
	}
}

GuiResourceId GfxPicture::getResourceId() {
	return _resourceId;
}

// differentiation between various picture formats can NOT get done using sci-version checks.
//  Games like PQ1 use the "old" vector data picture format, but are actually SCI1.1
//  We should leave this that way to decide the format on-the-fly instead of hardcoding it in any way
void GfxPicture::draw(int16 animationNr, bool mirroredFlag, bool addToFlag, int16 EGApaletteNo) {
	uint16 headerSize;

	_animationNr = animationNr;
	_mirroredFlag = mirroredFlag;
	_addToFlag = addToFlag;
	_EGApaletteNo = EGApaletteNo;
	_priority = 0;

	headerSize = READ_LE_UINT16(_resource->data);
	switch (headerSize) {
	case 0x26: // SCI 1.1 VGA picture
		_resourceType = SCI_PICTURE_TYPE_SCI11;
		drawSci11Vga();
		break;
#ifdef ENABLE_SCI32
	case 0x0e: // SCI32 VGA picture
		_resourceType = SCI_PICTURE_TYPE_SCI32;
		drawSci32Vga(0, 0, 0, 0, 0, false);
		break;
#endif
	default:
		// VGA, EGA or Amiga vector data
		_resourceType = SCI_PICTURE_TYPE_REGULAR;
		drawVectorData(_resource->data, _resource->size);
	}
}

void GfxPicture::reset() {
	int16 x, y;
	for (y = _ports->getPort()->top; y < _screen->getHeight(); y++) {
		for (x = 0; x < _screen->getWidth(); x++) {
			_screen->putPixel(x, y, GFX_SCREEN_MASK_ALL, 255, 0, 0);
		}
	}
}

void GfxPicture::drawSci11Vga() {
	byte *inbuffer = _resource->data;
	int size = _resource->size;
	int priorityBandsCount = inbuffer[3];
	int has_cel = inbuffer[4];
	int vector_dataPos = READ_LE_UINT32(inbuffer + 16);
	int vector_size = size - vector_dataPos;
	int palette_data_ptr = READ_LE_UINT32(inbuffer + 28);
	int cel_headerPos = READ_LE_UINT32(inbuffer + 32);
	int cel_RlePos = READ_LE_UINT32(inbuffer + cel_headerPos + 24);
	int cel_LiteralPos = READ_LE_UINT32(inbuffer + cel_headerPos + 28);
	Palette palette;

	// Header
	// [headerSize:WORD] [unknown:BYTE] [priorityBandCount:BYTE] [hasCel:BYTE] [unknown:BYTE]
	// [unknown:WORD] [unknown:WORD] [unknown:WORD] [unknown:WORD] [unknown:WORD]
	// Offset 16
	// [vectorDataOffset:DWORD] [unknown:DWORD] [unknown:DWORD] [paletteDataOffset:DWORD]
	// Offset 32
	// [celHeaderOffset:DWORD] [unknown:DWORD]
	// [priorityBandData:WORD] * priorityBandCount
	// [priority:BYTE] [unknown:BYTE]

	// priority bands are supposed to be 14 for sci1.1 pictures
	assert(priorityBandsCount == 14);

	if (_addToFlag) {
		_priority = inbuffer[40 + priorityBandsCount * 2] & 0xF;
	}

	// display Cel-data
	if (has_cel) {
		// Create palette and set it
		_palette->createFromData(inbuffer + palette_data_ptr, size - palette_data_ptr, &palette);
		_palette->set(&palette, true);

		drawCelData(inbuffer, size, cel_headerPos, cel_RlePos, cel_LiteralPos, 0, 0, 0, 0);
	}

	// process vector data
	drawVectorData(inbuffer + vector_dataPos, vector_size);

	// Set priority band information
	_ports->priorityBandsInitSci11(inbuffer + 40);
}

#ifdef ENABLE_SCI32
int16 GfxPicture::getSci32celCount() {
	byte *inbuffer = _resource->data;
	return inbuffer[2];
}

int16 GfxPicture::getSci32celX(int16 celNo) {
	byte *inbuffer = _resource->data;
	int header_size = READ_SCI11ENDIAN_UINT16(inbuffer);
	int cel_headerPos = header_size + 42 * celNo;
	return READ_SCI11ENDIAN_UINT16(inbuffer + cel_headerPos + 38);
}

int16 GfxPicture::getSci32celY(int16 celNo) {
	byte *inbuffer = _resource->data;
	int header_size = READ_SCI11ENDIAN_UINT16(inbuffer);
	int cel_headerPos = header_size + 42 * celNo;
	return READ_SCI11ENDIAN_UINT16(inbuffer + cel_headerPos + 40);
}

int16 GfxPicture::getSci32celWidth(int16 celNo) {
	byte *inbuffer = _resource->data;
	int header_size = READ_SCI11ENDIAN_UINT16(inbuffer);
	int cel_headerPos = header_size + 42 * celNo;
	return READ_SCI11ENDIAN_UINT16(inbuffer + cel_headerPos + 0);
}

int16 GfxPicture::getSci32celHeight(int16 celNo) {
	byte *inbuffer = _resource->data;
	int header_size = READ_SCI11ENDIAN_UINT16(inbuffer);
	int cel_headerPos = header_size + 42 * celNo;
	return READ_SCI11ENDIAN_UINT16(inbuffer + cel_headerPos + 2);
}


int16 GfxPicture::getSci32celPriority(int16 celNo) {
	byte *inbuffer = _resource->data;
	int header_size = READ_SCI11ENDIAN_UINT16(inbuffer);
	int cel_headerPos = header_size + 42 * celNo;
	return READ_SCI11ENDIAN_UINT16(inbuffer + cel_headerPos + 36);
}

void GfxPicture::drawSci32Vga(int16 celNo, int16 drawX, int16 drawY, int16 pictureX, int16 pictureY, bool mirrored) {
	byte *inbuffer = _resource->data;
	int size = _resource->size;
	int header_size = READ_SCI11ENDIAN_UINT16(inbuffer);
	int palette_data_ptr = READ_SCI11ENDIAN_UINT32(inbuffer + 6);
//	int celCount = inbuffer[2];
	int cel_headerPos = header_size;
	int cel_RlePos, cel_LiteralPos;
	Palette palette;

	// HACK
	_mirroredFlag = mirrored;
	_addToFlag = false;
	_resourceType = SCI_PICTURE_TYPE_SCI32;

	if (celNo == 0) {
		// Create palette and set it
		_palette->createFromData(inbuffer + palette_data_ptr, size - palette_data_ptr, &palette);
		_palette->set(&palette, true);
	}

	// Header
	// [headerSize:WORD] [celCount:BYTE] [Unknown:BYTE] [Unknown:WORD] [paletteOffset:DWORD] [Unknown:DWORD]
	// cel-header follow afterwards, each is 42 bytes
	// Cel-Header
	// [width:WORD] [height:WORD] [displaceX:WORD] [displaceY:WORD] [clearColor:BYTE] [compressed:BYTE]
	//  offset 10-23 is unknown
	// [rleOffset:DWORD] [literalOffset:DWORD] [Unknown:WORD] [Unknown:WORD] [priority:WORD] [relativeXpos:WORD] [relativeYpos:WORD]

	cel_headerPos += 42 * celNo;

	if (mirrored) {
		// switch around relativeXpos
		Common::Rect displayArea = _coordAdjuster->pictureGetDisplayArea();
		drawX = displayArea.width() - drawX - READ_SCI11ENDIAN_UINT16(inbuffer + cel_headerPos + 0);
	}

	cel_RlePos = READ_SCI11ENDIAN_UINT32(inbuffer + cel_headerPos + 24);
	cel_LiteralPos = READ_SCI11ENDIAN_UINT32(inbuffer + cel_headerPos + 28);

	drawCelData(inbuffer, size, cel_headerPos, cel_RlePos, cel_LiteralPos, drawX, drawY, pictureX, pictureY);
	cel_headerPos += 42;
}
#endif

extern void unpackCelData(byte *inBuffer, byte *celBitmap, byte clearColor, int pixelCount, int rlePos, int literalPos, ViewType viewType, uint16 width, bool isMacSci11ViewData);

void GfxPicture::drawCelData(byte *inbuffer, int size, int headerPos, int rlePos, int literalPos, int16 drawX, int16 drawY, int16 pictureX, int16 pictureY) {
	byte *celBitmap = NULL;
	byte *ptr = NULL;
	byte *headerPtr = inbuffer + headerPos;
	byte *rlePtr = inbuffer + rlePos;
	int16 displaceX, displaceY;
	byte priority = _addToFlag ? _priority : 0;
	byte clearColor;
	bool compression = true;
	byte curByte;
	int16 y, lastY, x, leftX, rightX;
	int pixelCount;
	uint16 width, height;

#ifdef ENABLE_SCI32
	if (_resourceType != SCI_PICTURE_TYPE_SCI32) {
#endif
		// Width/height here are always LE, even in Mac versions
		width = READ_LE_UINT16(headerPtr + 0);
		height = READ_LE_UINT16(headerPtr + 2);
		displaceX = (signed char)headerPtr[4];
		displaceY = (unsigned char)headerPtr[5];
		if (_resourceType == SCI_PICTURE_TYPE_SCI11)
			// SCI1.1 uses hardcoded clearcolor for pictures, even if cel header specifies otherwise
			clearColor = _screen->getColorWhite();
		else
			clearColor = headerPtr[6];
#ifdef ENABLE_SCI32
	} else {
		width = READ_SCI11ENDIAN_UINT16(headerPtr + 0);
		height = READ_SCI11ENDIAN_UINT16(headerPtr + 2);
		displaceX = READ_SCI11ENDIAN_UINT16(headerPtr + 4); // probably signed?!?
		displaceY = READ_SCI11ENDIAN_UINT16(headerPtr + 6); // probably signed?!?
		clearColor = headerPtr[8];
		if (headerPtr[9] == 0)
			compression = false;
	}
#endif

	if (displaceX || displaceY)
		error("unsupported embedded cel-data in picture");

	// We will unpack cel-data into a temporary buffer and then plot it to screen
	//  That needs to be done cause a mirrored picture may be requested
	pixelCount = width * height;
	celBitmap = new byte[pixelCount];
	if (!celBitmap)
		error("Unable to allocate temporary memory for picture drawing");

	if (g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() >= SCI_VERSION_2) {
		// See GfxView::unpackCel() for why this black/white swap is done
		// This picture swap is only needed in SCI32, not SCI1.1
		if (clearColor == 0)
			clearColor = 0xff;
		else if (clearColor == 0xff)
			clearColor = 0;
	}

	if (compression)
		unpackCelData(inbuffer, celBitmap, clearColor, pixelCount, rlePos, literalPos, _resMan->getViewType(), width, false);
	else
		// No compression (some SCI32 pictures)
		memcpy(celBitmap, rlePtr, pixelCount);

	if (g_sci->getPlatform() == Common::kPlatformMacintosh && getSciVersion() >= SCI_VERSION_2) {
		// See GfxView::unpackCel() for why this black/white swap is done
		// This picture swap is only needed in SCI32, not SCI1.1
		for (int i = 0; i < pixelCount; i++) {
			if (celBitmap[i] == 0)
				celBitmap[i] = 0xff;
			else if (celBitmap[i] == 0xff)
				celBitmap[i] = 0;
		}
	}

	Common::Rect displayArea = _coordAdjuster->pictureGetDisplayArea();

	// Horizontal clipping
	uint16 skipCelBitmapPixels = 0;
	int16 displayWidth = width;
	if (pictureX) {
		// horizontal scroll position for picture active, we need to adjust drawX accordingly
		drawX -= pictureX;
		if (drawX < 0) {
			skipCelBitmapPixels = -drawX;
			displayWidth -= skipCelBitmapPixels;
			drawX = 0;
		}
	}

	// Vertical clipping
	uint16 skipCelBitmapLines = 0;
	int16 displayHeight = height;
	if (pictureY) {
		// vertical scroll position for picture active, we need to adjust drawY accordingly
		// TODO: Finish this
		/*drawY -= pictureY;
		if (drawY < 0) {
			skipCelBitmapLines = -drawY;
			displayHeight -= skipCelBitmapLines;
			drawY = 0;
		}*/
	}

	if (displayWidth > 0 && displayHeight > 0) {
		y = displayArea.top + drawY;
		lastY = MIN<int16>(height + y, displayArea.bottom);
		leftX = displayArea.left + drawX;
		rightX = MIN<int16>(displayWidth + leftX, displayArea.right);

		uint16 sourcePixelSkipPerRow = 0;
		if (width > rightX - leftX)
			sourcePixelSkipPerRow = width - (rightX - leftX);

		// Change clearcolor to white, if we dont add to an existing picture. That way we will paint everything on screen
		// but white and that won't matter because the screen is supposed to be already white. It seems that most (if not all)
		// SCI1.1 games use color 0 as transparency and SCI1 games use color 255 as transparency. Sierra SCI seems to paint
		// the whole data to screen and wont skip over transparent pixels. So this will actually make it work like Sierra.
		// SCI32 doesn't use _addToFlag at all.
		if (!_addToFlag && _resourceType != SCI_PICTURE_TYPE_SCI32)
			clearColor = _screen->getColorWhite();

		byte drawMask = priority > 15 ? GFX_SCREEN_MASK_VISUAL : GFX_SCREEN_MASK_VISUAL | GFX_SCREEN_MASK_PRIORITY;

		ptr = celBitmap;
		ptr += skipCelBitmapPixels;
		ptr += skipCelBitmapLines * width;
		if (!_mirroredFlag) {
			// Draw bitmap to screen
			x = leftX;
			while (y < lastY) {
				curByte = *ptr++;
				if ((curByte != clearColor) && (priority >= _screen->getPriority(x, y)))
					_screen->putPixel(x, y, drawMask, curByte, priority, 0);

				x++;

				if (x >= rightX) {
					ptr += sourcePixelSkipPerRow;
					x = leftX;
					y++;
				}
			}
		} else {
			// Draw bitmap to screen (mirrored)
			x = rightX - 1;
			while (y < lastY) {
				curByte = *ptr++;
				if ((curByte != clearColor) && (priority >= _screen->getPriority(x, y)))
					_screen->putPixel(x, y, drawMask, curByte, priority, 0);

				if (x == leftX) {
					ptr += sourcePixelSkipPerRow;
					x = rightX;
					y++;
				}

				x--;
			}
		}
	}

	delete[] celBitmap;
}

enum {
	PIC_OP_SET_COLOR = 0xf0,
	PIC_OP_DISABLE_VISUAL = 0xf1,
	PIC_OP_SET_PRIORITY = 0xf2,
	PIC_OP_DISABLE_PRIORITY = 0xf3,
	PIC_OP_SHORT_PATTERNS = 0xf4,
	PIC_OP_MEDIUM_LINES = 0xf5,
	PIC_OP_LONG_LINES = 0xf6,
	PIC_OP_SHORT_LINES = 0xf7,
	PIC_OP_FILL = 0xf8,
	PIC_OP_SET_PATTERN = 0xf9,
	PIC_OP_ABSOLUTE_PATTERN = 0xfa,
	PIC_OP_SET_CONTROL = 0xfb,
	PIC_OP_DISABLE_CONTROL = 0xfc,
	PIC_OP_MEDIUM_PATTERNS = 0xfd,
	PIC_OP_OPX = 0xfe,
	PIC_OP_TERMINATE = 0xff
};

#define PIC_OP_FIRST PIC_OP_SET_COLOR

enum {
	PIC_OPX_EGA_SET_PALETTE_ENTRIES = 0,
	PIC_OPX_EGA_SET_PALETTE = 1,
	PIC_OPX_EGA_MONO0 = 2,
	PIC_OPX_EGA_MONO1 = 3,
	PIC_OPX_EGA_MONO2 = 4,
	PIC_OPX_EGA_MONO3 = 5,
	PIC_OPX_EGA_MONO4 = 6,
	PIC_OPX_EGA_EMBEDDED_VIEW = 7,
	PIC_OPX_EGA_SET_PRIORITY_TABLE = 8
};

enum {
	PIC_OPX_VGA_SET_PALETTE_ENTRIES = 0,
	PIC_OPX_VGA_EMBEDDED_VIEW = 1,
	PIC_OPX_VGA_SET_PALETTE = 2,
	PIC_OPX_VGA_PRIORITY_TABLE_EQDIST = 3,
	PIC_OPX_VGA_PRIORITY_TABLE_EXPLICIT = 4
};

#ifdef DEBUG_PICTURE_DRAW
const char *picOpcodeNames[] = {
	"Set color",
	"Disable visual",
	"Set priority",
	"Disable priority",
	"Short patterns",
	"Medium lines",
	"Long lines",
	"Short lines",
	"Fill",
	"Set pattern",
	"Absolute pattern",
	"Set control",
	"Disable control",
	"Medium patterns",
	"Extended opcode",
	"Terminate"
};

const char *picExOpcodeNamesEGA[] = {
	"Set palette entries",
	"Set palette",
	"Mono0",
	"Mono1",
	"Mono2",
	"Mono3",
	"Mono4",
	"Embedded view",
	"Set priority table"
};

const char *picExOpcodeNamesVGA[] = {
	"Set palette entries",
	"Embedded view",
	"Set palette",
	"Set priority table (eqdist)",
	"Set priority table (explicit)"
};
#endif

#define PIC_EGAPALETTE_COUNT 4
#define PIC_EGAPALETTE_SIZE  40
#define PIC_EGAPALETTE_TOTALSIZE PIC_EGAPALETTE_COUNT*PIC_EGAPALETTE_SIZE
#define PIC_EGAPRIORITY_SIZE PIC_EGAPALETTE_SIZE

static const byte vector_defaultEGApalette[PIC_EGAPALETTE_SIZE] = {
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x88,
	0x88, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x88,
	0x88, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
	0x08, 0x91, 0x2a, 0x3b, 0x4c, 0x5d, 0x6e, 0x88
};

static const byte vector_defaultEGApriority[PIC_EGAPRIORITY_SIZE] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
};

void GfxPicture::drawVectorData(byte *data, int dataSize) {
	byte pic_op;
	byte pic_color = _screen->getColorDefaultVectorData();
	byte pic_priority = 255, pic_control = 255;
	int16 x = 0, y = 0, oldx, oldy;
	byte EGApalettes[PIC_EGAPALETTE_TOTALSIZE] = {0};
	byte *EGApalette = &EGApalettes[_EGApaletteNo * PIC_EGAPALETTE_SIZE];
	byte EGApriority[PIC_EGAPRIORITY_SIZE] = {0};
	bool isEGA = false;
	int curPos = 0;
	uint16 size;
	byte pixel;
	int i;
	Palette palette;
	int16 pattern_Code = 0, pattern_Texture = 0;
	bool icemanDrawFix = false;
	bool ignoreBrokenPriority = false;

	memset(&palette, 0, sizeof(palette));

	if (_EGApaletteNo >= PIC_EGAPALETTE_COUNT)
		_EGApaletteNo = 0;

	if (_resMan->getViewType() == kViewEga) {
		isEGA = true;
		// setup default mapping tables
		for (i = 0; i < PIC_EGAPALETTE_TOTALSIZE; i += PIC_EGAPALETTE_SIZE)
			memcpy(&EGApalettes[i], &vector_defaultEGApalette, sizeof(vector_defaultEGApalette));
		memcpy(&EGApriority, &vector_defaultEGApriority, sizeof(vector_defaultEGApriority));

		if (g_sci->getGameId() == GID_ICEMAN) {
			// WORKAROUND: we remove certain visual&priority lines in underwater
			// rooms of iceman, when not dithering the picture. Normally those
			// lines aren't shown, because they share the same color as the
			// dithered fill color combination. When not dithering, those lines
			// would appear and get distracting.
			if ((_screen->isUnditheringEnabled()) && ((_resourceId >= 53 && _resourceId <= 58) || (_resourceId == 61)))
				icemanDrawFix = true;
		}
		if (g_sci->getGameId() == GID_KQ5) {
			// WORKAROUND: ignore the seemingly broken priority of picture 48
			// (island overview). Fixes bug #3041044.
			if (_resourceId == 48)
				ignoreBrokenPriority = true;
		}
		if (g_sci->getGameId() == GID_SQ4) {
			// WORKAROUND: ignore the seemingly broken priority of pictures 546
			// and 547 (Vohaul's head and Roger Jr trapped). Fixes bug #3046543.
			if (_resourceId == 546 || _resourceId == 547)
				ignoreBrokenPriority = true;
			// WORKAROUND: ignore the seemingly broken priority of picture 631
			// (SQ1 view from the cockpit). Fixes bug #3046513.
			if (_resourceId == 631)
				ignoreBrokenPriority = true;
		}
	}

	// Drawing
	while (curPos < dataSize) {
#ifdef DEBUG_PICTURE_DRAW
		debug("Picture op: %X (%s) at %d", data[curPos], picOpcodeNames[data[curPos] - 0xF0], curPos);
#endif
		switch (pic_op = data[curPos++]) {
		case PIC_OP_SET_COLOR:
			pic_color = data[curPos++];
			if (isEGA) {
				pic_color = EGApalette[pic_color];
				pic_color ^= pic_color << 4;
			}
			break;
		case PIC_OP_DISABLE_VISUAL:
			pic_color = 0xFF;
			break;

		case PIC_OP_SET_PRIORITY:
			pic_priority = data[curPos++] & 0x0F;
			if (isEGA)
				pic_priority = EGApriority[pic_priority];
			if (ignoreBrokenPriority)
				pic_priority = 255;
			break;
		case PIC_OP_DISABLE_PRIORITY:
			pic_priority = 255;
			break;

		case PIC_OP_SET_CONTROL:
			pic_control = data[curPos++] & 0x0F;
			break;
		case PIC_OP_DISABLE_CONTROL:
			pic_control = 255;
			break;

		case PIC_OP_SHORT_LINES: // short line
			vectorGetAbsCoords(data, curPos, x, y);
			while (vectorIsNonOpcode(data[curPos])) {
				oldx = x; oldy = y;
				vectorGetRelCoords(data, curPos, x, y);
				Common::Point startPoint(oldx, oldy);
				Common::Point endPoint(x, y);
				_ports->offsetLine(startPoint, endPoint);
				_screen->drawLine(startPoint, endPoint, pic_color, pic_priority, pic_control);
			}
			break;
		case PIC_OP_MEDIUM_LINES: // medium line
			vectorGetAbsCoords(data, curPos, x, y);
			if (icemanDrawFix) {
				// WORKAROUND: remove certain lines in iceman ffs. see above
				if ((pic_color == 1) && (pic_priority == 14)) {
					if ((y < 100) || (!(y & 1))) {
						pic_color = 255;
						pic_priority = 255;
					}
				}
			}
			while (vectorIsNonOpcode(data[curPos])) {
				oldx = x; oldy = y;
				vectorGetRelCoordsMed(data, curPos, x, y);
				Common::Point startPoint(oldx, oldy);
				Common::Point endPoint(x, y);
				_ports->offsetLine(startPoint, endPoint);
				_screen->drawLine(startPoint, endPoint, pic_color, pic_priority, pic_control);
			}
			break;
		case PIC_OP_LONG_LINES: // long line
			vectorGetAbsCoords(data, curPos, x, y);
			while (vectorIsNonOpcode(data[curPos])) {
				oldx = x; oldy = y;
				vectorGetAbsCoords(data, curPos, x, y);
				Common::Point startPoint(oldx, oldy);
				Common::Point endPoint(x, y);
				_ports->offsetLine(startPoint, endPoint);
				_screen->drawLine(startPoint, endPoint, pic_color, pic_priority, pic_control);
			}
			break;

		case PIC_OP_FILL: //fill
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetAbsCoords(data, curPos, x, y);
				vectorFloodFill(x, y, pic_color, pic_priority, pic_control);
			}
			break;

		// Pattern opcodes are handled in sierra sci1.1+ as actual NOPs and
		// normally they definitely should not occur inside picture data for
		// such games.
		case PIC_OP_SET_PATTERN:
			if (_resourceType >= SCI_PICTURE_TYPE_SCI11) {
				if (g_sci->getGameId() == GID_SQ4) {
					// WORKAROUND: For SQ4 / for some pictures handle this like
					// a terminator. This picture includes garbage data, first a
					// set pattern w/o parameter and then short pattern. I guess
					// that garbage is a left over from the sq4-floppy (sci1) to
					// sq4-cd (sci1.1) conversion.
					switch (_resourceId) {
					case 35:
					case 381:
					case 376:
						return;
					default:
						break;
					}
				}
				error("pic-operation set pattern inside sci1.1+ vector data");
			}
			pattern_Code = data[curPos++];
			break;
		case PIC_OP_SHORT_PATTERNS:
			if (_resourceType >= SCI_PICTURE_TYPE_SCI11)
				error("pic-operation short pattern inside sci1.1+ vector data");
			vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
			vectorGetAbsCoords(data, curPos, x, y);
			vectorPattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
				vectorGetRelCoords(data, curPos, x, y);
				vectorPattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			}
			break;
		case PIC_OP_MEDIUM_PATTERNS:
			if (_resourceType >= SCI_PICTURE_TYPE_SCI11)
				error("pic-operation medium pattern inside sci1.1+ vector data");
			vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
			vectorGetAbsCoords(data, curPos, x, y);
			vectorPattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
				vectorGetRelCoordsMed(data, curPos, x, y);
				vectorPattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			}
			break;
		case PIC_OP_ABSOLUTE_PATTERN:
			if (_resourceType >= SCI_PICTURE_TYPE_SCI11)
				error("pic-operation absolute pattern inside sci1.1+ vector data");
			while (vectorIsNonOpcode(data[curPos])) {
				vectorGetPatternTexture(data, curPos, pattern_Code, pattern_Texture);
				vectorGetAbsCoords(data, curPos, x, y);
				vectorPattern(x, y, pic_color, pic_priority, pic_control, pattern_Code, pattern_Texture);
			}
			break;

		case PIC_OP_OPX: // Extended functions
			if (isEGA) {
#ifdef DEBUG_PICTURE_DRAW
				debug("* Picture ex op: %X (%s) at %d", data[curPos], picExOpcodeNamesEGA[data[curPos]], curPos);
#endif
				switch (pic_op = data[curPos++]) {
				case PIC_OPX_EGA_SET_PALETTE_ENTRIES:
					while (vectorIsNonOpcode(data[curPos])) {
						pixel = data[curPos++];
						if (pixel >= PIC_EGAPALETTE_TOTALSIZE) {
							error("picture trying to write to invalid EGA-palette");
						}
						EGApalettes[pixel] = data[curPos++];
					}
					break;
				case PIC_OPX_EGA_SET_PALETTE:
					pixel = data[curPos++];
					if (pixel >= PIC_EGAPALETTE_COUNT) {
						error("picture trying to write to invalid palette %d", (int)pixel);
					}
					pixel *= PIC_EGAPALETTE_SIZE;
					for (i = 0; i < PIC_EGAPALETTE_SIZE; i++) {
						EGApalettes[pixel + i] = data[curPos++];
					}
					break;
				case PIC_OPX_EGA_MONO0:
					curPos += 41;
					break;
				case PIC_OPX_EGA_MONO1:
				case PIC_OPX_EGA_MONO3:
					curPos++;
					break;
				case PIC_OPX_EGA_MONO2:
				case PIC_OPX_EGA_MONO4:
					break;
				case PIC_OPX_EGA_EMBEDDED_VIEW:
					vectorGetAbsCoordsNoMirror(data, curPos, x, y);
					size = READ_LE_UINT16(data + curPos); curPos += 2;
					_priority = pic_priority; // set global priority so the cel gets drawn using current priority as well
					drawCelData(data, _resource->size, curPos, curPos + 8, 0, x, y, 0, 0);
					curPos += size;
					break;
				case PIC_OPX_EGA_SET_PRIORITY_TABLE:
					_ports->priorityBandsInit(data + curPos);
					curPos += 14;
					break;
				default:
					error("Unsupported sci1 extended pic-operation %X", pic_op);
				}
			} else {
#ifdef DEBUG_PICTURE_DRAW
				debug("* Picture ex op: %X (%s) at %d", data[curPos], picExOpcodeNamesVGA[data[curPos]], curPos);
#endif
				switch (pic_op = data[curPos++]) {
				case PIC_OPX_VGA_SET_PALETTE_ENTRIES:
					while (vectorIsNonOpcode(data[curPos])) {
						curPos++; // skip commands
					}
					break;
				case PIC_OPX_VGA_SET_PALETTE:
					if (_resMan->getViewType() == kViewAmiga || _resMan->getViewType() == kViewAmiga64) {
						if ((data[curPos] == 0x00) && (data[curPos + 1] == 0x01) && ((data[curPos + 32] & 0xF0) != 0xF0)) {
							// Left-Over VGA palette, we simply ignore it
							curPos += 256 + 4 + 1024;
						} else {
							// Setting half of the Amiga palette
							_palette->modifyAmigaPalette(&data[curPos]);
							curPos += 32;
						}
					} else {
						curPos += 256 + 4; // Skip over mapping and timestamp
						for (i = 0; i < 256; i++) {
							palette.colors[i].used = data[curPos++];
							palette.colors[i].r = data[curPos++]; palette.colors[i].g = data[curPos++]; palette.colors[i].b = data[curPos++];
						}
						_palette->set(&palette, true);
					}
					break;
				case PIC_OPX_VGA_EMBEDDED_VIEW: // draw cel
					vectorGetAbsCoordsNoMirror(data, curPos, x, y);
					size = READ_LE_UINT16(data + curPos); curPos += 2;
					_priority = pic_priority; // set global priority so the cel gets drawn using current priority as well
					drawCelData(data, _resource->size, curPos, curPos + 8, 0, x, y, 0, 0);
					curPos += size;
					break;
				case PIC_OPX_VGA_PRIORITY_TABLE_EQDIST:
					_ports->priorityBandsInit(-1, READ_LE_UINT16(data + curPos), READ_LE_UINT16(data + curPos + 2));
					curPos += 4;
					break;
				case PIC_OPX_VGA_PRIORITY_TABLE_EXPLICIT:
					_ports->priorityBandsInit(data + curPos);
					curPos += 14;
					break;
				default:
					error("Unsupported sci1 extended pic-operation %X", pic_op);
				}
			}
			break;
		case PIC_OP_TERMINATE:
			_priority = pic_priority;
			// Dithering EGA pictures
			if (isEGA) {
				_screen->dither(_addToFlag);
				switch (g_sci->getGameId()) {
				case GID_SQ3:
					switch (_resourceId) {
					case 154: // SQ3: intro, ship gets sucked in
						_screen->ditherForceDitheredColor(0xD0);
						break;
					default:
						break;
					}
					break;
				default:
					break;
				}
			}
			return;
		default:
			error("Unsupported pic-operation %X", pic_op);
		}
		if ((_EGAdrawingVisualize) && (isEGA)) {
			_screen->copyToScreen();
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
	}
	error("picture vector data without terminator");
}

bool GfxPicture::vectorIsNonOpcode(byte pixel) {
	if (pixel >= PIC_OP_FIRST)
		return false;
	return true;
}

void GfxPicture::vectorGetAbsCoords(byte *data, int &curPos, int16 &x, int16 &y) {
	byte pixel = data[curPos++];
	x = data[curPos++] + ((pixel & 0xF0) << 4);
	y = data[curPos++] + ((pixel & 0x0F) << 8);
	if (_mirroredFlag) x = 319 - x;
}

void GfxPicture::vectorGetAbsCoordsNoMirror(byte *data, int &curPos, int16 &x, int16 &y) {
	byte pixel = data[curPos++];
	x = data[curPos++] + ((pixel & 0xF0) << 4);
	y = data[curPos++] + ((pixel & 0x0F) << 8);
}

void GfxPicture::vectorGetRelCoords(byte *data, int &curPos, int16 &x, int16 &y) {
	byte pixel = data[curPos++];
	if (pixel & 0x80) {
		x -= ((pixel >> 4) & 7) * (_mirroredFlag ? -1 : 1);
	} else {
		x += (pixel >> 4) * (_mirroredFlag ? -1 : 1);
	}
	if (pixel & 0x08) {
		y -= (pixel & 7);
	} else {
		y += (pixel & 7);
	}
}

void GfxPicture::vectorGetRelCoordsMed(byte *data, int &curPos, int16 &x, int16 &y) {
	byte pixel = data[curPos++];
	if (pixel & 0x80) {
		y -= (pixel & 0x7F);
	} else {
		y += pixel;
	}
	pixel = data[curPos++];
	if (pixel & 0x80) {
		x -= (128 - (pixel & 0x7F)) * (_mirroredFlag ? -1 : 1);
	} else {
		x += pixel * (_mirroredFlag ? -1 : 1);
	}
}

void GfxPicture::vectorGetPatternTexture(byte *data, int &curPos, int16 pattern_Code, int16 &pattern_Texture) {
	if (pattern_Code & SCI_PATTERN_CODE_USE_TEXTURE) {
		pattern_Texture = (data[curPos++] >> 1) & 0x7f;
	}
}

// WARNING: Do not replace the following code with something else, like generic
// code. This algo really needs to behave exactly as the one from sierra.
void GfxPicture::vectorFloodFill(int16 x, int16 y, byte color, byte priority, byte control) {
	Port *curPort = _ports->getPort();
	Common::Stack<Common::Point> stack;
	Common::Point p, p1;
	byte screenMask = _screen->getDrawingMask(color, priority, control);
	byte matchedMask, matchMask;
	int16 w, e, a_set, b_set;

	bool isEGA = (_resMan->getViewType() == kViewEga);

	p.x = x + curPort->left;
	p.y = y + curPort->top;
	stack.push(p);

	byte searchColor = _screen->getVisual(p.x, p.y);
	byte searchPriority = _screen->getPriority(p.x, p.y);
	byte searchControl = _screen->getControl(p.x, p.y);

	if (isEGA) {
		// In EGA games a pixel in the framebuffer is only 4 bits. We store
		// a full byte per pixel to allow undithering, but when comparing
		// pixels for flood-fill purposes, we should only compare the
		// visible color of a pixel.

		if ((x ^ y) & 1)
			searchColor = (searchColor ^ (searchColor >> 4)) & 0x0F;
		else
			searchColor = searchColor & 0x0F;
	}

	// This logic was taken directly from sierra sci, floodfill will get aborted on various occations
	if (screenMask & GFX_SCREEN_MASK_VISUAL) {
		if ((color == _screen->getColorWhite()) || (searchColor != _screen->getColorWhite()))
			return;
	} else if (screenMask & GFX_SCREEN_MASK_PRIORITY) {
		if ((priority == 0) || (searchPriority != 0))
			return;
	} else if (screenMask & GFX_SCREEN_MASK_CONTROL) {
		if ((control == 0) || (searchControl != 0))
			return;
	}

	// Now remove screens, that already got the right color/priority/control
	if ((screenMask & GFX_SCREEN_MASK_VISUAL) && (searchColor == color))
		screenMask &= ~GFX_SCREEN_MASK_VISUAL;
	if ((screenMask & GFX_SCREEN_MASK_PRIORITY) && (searchPriority == priority))
		screenMask &= ~GFX_SCREEN_MASK_PRIORITY;
	if ((screenMask & GFX_SCREEN_MASK_CONTROL) && (searchControl == control))
		screenMask &= ~GFX_SCREEN_MASK_CONTROL;

	// Exit, if no screens left
	if (!screenMask)
		return;

	if (screenMask & GFX_SCREEN_MASK_VISUAL) {
		matchMask = GFX_SCREEN_MASK_VISUAL;
	} else if (screenMask & GFX_SCREEN_MASK_PRIORITY) {
		matchMask = GFX_SCREEN_MASK_PRIORITY;
	} else {
		matchMask = GFX_SCREEN_MASK_CONTROL;
	}

	// hard borders for filling
	int l = curPort->rect.left + curPort->left;
	int t = curPort->rect.top + curPort->top;
	int r = curPort->rect.right + curPort->left - 1;
	int b = curPort->rect.bottom + curPort->top - 1;
	while (stack.size()) {
		p = stack.pop();
		if ((matchedMask = _screen->isFillMatch(p.x, p.y, matchMask, searchColor, searchPriority, searchControl, isEGA)) == 0) // already filled
			continue;
		_screen->putPixel(p.x, p.y, screenMask, color, priority, control);
		w = p.x;
		e = p.x;
		// moving west and east pointers as long as there is a matching color to fill
		while (w > l && (matchedMask = _screen->isFillMatch(w - 1, p.y, matchMask, searchColor, searchPriority, searchControl, isEGA)))
			_screen->putPixel(--w, p.y, screenMask, color, priority, control);
		while (e < r && (matchedMask = _screen->isFillMatch(e + 1, p.y, matchMask, searchColor, searchPriority, searchControl, isEGA)))
			_screen->putPixel(++e, p.y, screenMask, color, priority, control);
		// checking lines above and below for possible flood targets
		a_set = b_set = 0;
		while (w <= e) {
			if (p.y > t && (matchedMask = _screen->isFillMatch(w, p.y - 1, matchMask, searchColor, searchPriority, searchControl, isEGA))) { // one line above
				if (a_set == 0) {
					p1.x = w;
					p1.y = p.y - 1;
					stack.push(p1);
					a_set = 1;
				}
			} else
				a_set = 0;

			if (p.y < b && (matchedMask = _screen->isFillMatch(w, p.y + 1, matchMask, searchColor, searchPriority, searchControl, isEGA))) { // one line below
				if (b_set == 0) {
					p1.x = w;
					p1.y = p.y + 1;
					stack.push(p1);
					b_set = 1;
				}
			} else
				b_set = 0;
			w++;
		}
	}
}

// Bitmap for drawing sierra circles
static const byte vectorPatternCircles[8][30] = {
	{ 0x01 },
	{ 0x72, 0x02 },
	{ 0xCE, 0xF7, 0x7D, 0x0E },
	{ 0x1C, 0x3E, 0x7F, 0x7F, 0x7F, 0x3E, 0x1C, 0x00 },
	{ 0x38, 0xF8, 0xF3, 0xDF, 0x7F, 0xFF, 0xFD, 0xF7, 0x9F, 0x3F, 0x38 },
	{ 0x70, 0xC0, 0x1F, 0xFE, 0xE3, 0x3F, 0xFF, 0xF7, 0x7F, 0xFF, 0xE7, 0x3F, 0xFE, 0xC3, 0x1F, 0xF8, 0x00 },
	{ 0xF0, 0x01, 0xFF, 0xE1, 0xFF, 0xF8, 0x3F, 0xFF, 0xDF, 0xFF, 0xF7, 0xFF, 0xFD, 0x7F, 0xFF, 0x9F, 0xFF,
		0xE3, 0xFF, 0xF0, 0x1F, 0xF0, 0x01 },
	{ 0xE0, 0x03, 0xF8, 0x0F, 0xFC, 0x1F, 0xFE, 0x3F, 0xFE, 0x3F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF,
		0x7F, 0xFF, 0x7F, 0xFE, 0x3F, 0xFE, 0x3F, 0xFC, 0x1F, 0xF8, 0x0F, 0xE0, 0x03 }
//  { 0x01 };
//	{ 0x03, 0x03, 0x03 },
//	{ 0x02, 0x07, 0x07, 0x07, 0x02 },
//	{ 0x06, 0x06, 0x0F, 0x0F, 0x0F, 0x06, 0x06 },
//	{ 0x04, 0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x04 },
//	{ 0x0C, 0x1E, 0x1E, 0x1E, 0x3F, 0x3F, 0x3F, 0x1E, 0x1E, 0x1E, 0x0C },
//	{ 0x1C, 0x3E, 0x3E, 0x3E, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x3E, 0x3E, 0x3E, 0x1C },
//	{ 0x18, 0x3C, 0x7E, 0x7E, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0x7E, 0x3C, 0x18 }
};

// TODO: perhaps this is a better way to set the s_patternTextures array below?
//  in that case one would need to adjust bits of secondary table. Bit 256 is ignored by original interpreter
#if 0
static const byte patternTextures[32 * 2] = {
	0x04, 0x29, 0x40, 0x24, 0x09, 0x41, 0x25, 0x45,
	0x41, 0x90, 0x50, 0x44, 0x48, 0x08, 0x42, 0x28,
	0x89, 0x52, 0x89, 0x88, 0x10, 0x48, 0xA4, 0x08,
	0x44, 0x15, 0x28, 0x24, 0x00, 0x0A, 0x24, 0x20,
	// Now the table is actually duplicated, so we won't need to wrap around
	0x04, 0x29, 0x40, 0x24, 0x09, 0x41, 0x25, 0x45,
	0x41, 0x90, 0x50, 0x44, 0x48, 0x08, 0x42, 0x28,
	0x89, 0x52, 0x89, 0x88, 0x10, 0x48, 0xA4, 0x08,
	0x44, 0x15, 0x28, 0x24, 0x00, 0x0A, 0x24, 0x20,
};
#endif

// This table is bitwise upwards (from bit0 to bit7), sierras original table went down the bits (bit7 to bit0)
//  this was done to simplify things, so we can just run through the table w/o worrying too much about clipping
static const bool vectorPatternTextures[32 * 8 * 2] = {
	false, false,  true, false, false, false, false, false, // 0x04
	 true, false, false,  true, false,  true, false, false, // 0x29
	false, false, false, false, false, false,  true, false, // 0x40
	false, false,  true, false, false,  true, false, false, // 0x24
	 true, false, false,  true, false, false, false, false, // 0x09
	 true, false, false, false, false, false,  true, false, // 0x41
	 true, false,  true, false, false,  true, false, false, // 0x25
	 true, false,  true, false, false, false,  true, false, // 0x45
	 true, false, false, false, false, false,  true, false, // 0x41
	false, false, false, false,  true, false, false,  true, // 0x90
	false, false, false, false,  true, false,  true, false, // 0x50
	false, false,  true, false, false, false,  true, false, // 0x44
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false, false,  true, false, false, false, false, // 0x08
	false,  true, false, false, false, false,  true, false, // 0x42
	false, false, false,  true, false,  true, false, false, // 0x28
	 true, false, false,  true, false, false, false,  true, // 0x89
	false,  true, false, false,  true, false,  true, false, // 0x52
	 true, false, false,  true, false, false, false,  true, // 0x89
	false, false, false,  true, false, false, false,  true, // 0x88
	false, false, false, false,  true, false, false, false, // 0x10
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false,  true, false, false,  true, false,  true, // 0xA4
	false, false, false,  true, false, false, false, false, // 0x08
	false, false,  true, false, false, false,  true, false, // 0x44
	 true, false,  true, false,  true, false, false, false, // 0x15
	false, false, false,  true, false,  true, false, false, // 0x28
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false, false, false, false, // 0x00
	false,  true, false,  true, false, false, false, false, // 0x0A
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false,  true, false,        // 0x20 (last bit is not mentioned cause original interpreter also ignores that bit)
	// Now the table is actually duplicated, so we won't need to wrap around
	false, false,  true, false, false, false, false, false, // 0x04
	 true, false, false,  true, false,  true, false, false, // 0x29
	false, false, false, false, false, false,  true, false, // 0x40
	false, false,  true, false, false,  true, false, false, // 0x24
	 true, false, false,  true, false, false, false, false, // 0x09
	 true, false, false, false, false, false,  true, false, // 0x41
	 true, false,  true, false, false,  true, false, false, // 0x25
	 true, false,  true, false, false, false,  true, false, // 0x45
	 true, false, false, false, false, false,  true, false, // 0x41
	false, false, false, false,  true, false, false,  true, // 0x90
	false, false, false, false,  true, false,  true, false, // 0x50
	false, false,  true, false, false, false,  true, false, // 0x44
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false, false,  true, false, false, false, false, // 0x08
	false,  true, false, false, false, false,  true, false, // 0x42
	false, false, false,  true, false,  true, false, false, // 0x28
	 true, false, false,  true, false, false, false,  true, // 0x89
	false,  true, false, false,  true, false,  true, false, // 0x52
	 true, false, false,  true, false, false, false,  true, // 0x89
	false, false, false,  true, false, false, false,  true, // 0x88
	false, false, false, false,  true, false, false, false, // 0x10
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false,  true, false, false,  true, false,  true, // 0xA4
	false, false, false,  true, false, false, false, false, // 0x08
	false, false,  true, false, false, false,  true, false, // 0x44
	 true, false,  true, false,  true, false, false, false, // 0x15
	false, false, false,  true, false,  true, false, false, // 0x28
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false, false, false, false, // 0x00
	false,  true, false,  true, false, false, false, false, // 0x0A
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false,  true, false,        // 0x20 (last bit is not mentioned cause original interpreter also ignores that bit)
};

// Bit offsets into pattern_textures
static const byte vectorPatternTextureOffset[128] = {
	0x00, 0x18, 0x30, 0xc4, 0xdc, 0x65, 0xeb, 0x48,
	0x60, 0xbd, 0x89, 0x05, 0x0a, 0xf4, 0x7d, 0x7d,
	0x85, 0xb0, 0x8e, 0x95, 0x1f, 0x22, 0x0d, 0xdf,
	0x2a, 0x78, 0xd5, 0x73, 0x1c, 0xb4, 0x40, 0xa1,
	0xb9, 0x3c, 0xca, 0x58, 0x92, 0x34, 0xcc, 0xce,
	0xd7, 0x42, 0x90, 0x0f, 0x8b, 0x7f, 0x32, 0xed,
	0x5c, 0x9d, 0xc8, 0x99, 0xad, 0x4e, 0x56, 0xa6,
	0xf7, 0x68, 0xb7, 0x25, 0x82, 0x37, 0x3a, 0x51,
	0x69, 0x26, 0x38, 0x52, 0x9e, 0x9a, 0x4f, 0xa7,
	0x43, 0x10, 0x80, 0xee, 0x3d, 0x59, 0x35, 0xcf,
	0x79, 0x74, 0xb5, 0xa2, 0xb1, 0x96, 0x23, 0xe0,
	0xbe, 0x05, 0xf5, 0x6e, 0x19, 0xc5, 0x66, 0x49,
	0xf0, 0xd1, 0x54, 0xa9, 0x70, 0x4b, 0xa4, 0xe2,
	0xe6, 0xe5, 0xab, 0xe4, 0xd2, 0xaa, 0x4c, 0xe3,
	0x06, 0x6f, 0xc6, 0x4a, 0xa4, 0x75, 0x97, 0xe1
};

void GfxPicture::vectorPatternBox(Common::Rect box, byte color, byte prio, byte control) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			_screen->putPixel(x, y, flag, color, prio, control);
		}
	}
}

void GfxPicture::vectorPatternTexturedBox(Common::Rect box, byte color, byte prio, byte control, byte texture) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	const bool *textureData = &vectorPatternTextures[vectorPatternTextureOffset[texture]];
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			if (*textureData) {
				_screen->putPixel(x, y, flag, color, prio, control);
			}
			textureData++;
		}
	}
}

void GfxPicture::vectorPatternCircle(Common::Rect box, byte size, byte color, byte prio, byte control) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	const byte *circleData = vectorPatternCircles[size];
	byte bitmap = *circleData;
	byte bitNo = 0;
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			if (bitmap & 1) {
				_screen->putPixel(x, y, flag, color, prio, control);
			}
			bitNo++;
			if (bitNo == 8) {
				circleData++; bitmap = *circleData; bitNo = 0;
			} else {
				bitmap = bitmap >> 1;
			}
		}
	}
}

void GfxPicture::vectorPatternTexturedCircle(Common::Rect box, byte size, byte color, byte prio, byte control, byte texture) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	const byte *circleData = vectorPatternCircles[size];
	byte bitmap = *circleData;
	byte bitNo = 0;
	const bool *textureData = &vectorPatternTextures[vectorPatternTextureOffset[texture]];
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			if (bitmap & 1) {
				if (*textureData) {
					_screen->putPixel(x, y, flag, color, prio, control);
				}
				textureData++;
			}
			bitNo++;
			if (bitNo == 8) {
				circleData++; bitmap = *circleData; bitNo = 0;
			} else {
				bitmap = bitmap >> 1;
			}
		}
	}
}

void GfxPicture::vectorPattern(int16 x, int16 y, byte color, byte priority, byte control, byte code, byte texture) {
	byte size = code & SCI_PATTERN_CODE_PENSIZE;
	Common::Rect rect;

	// We need to adjust the given coordinates, because the ones given us do not define upper left but somewhat middle
	y -= size; if (y < 0) y = 0;
	x -= size; if (x < 0) x = 0;

	rect.top = y; rect.left = x;
	rect.setHeight((size*2)+1); rect.setWidth((size*2)+2);
	_ports->offsetRect(rect);
	rect.clip(_screen->getWidth(), _screen->getHeight());

	if (code & SCI_PATTERN_CODE_RECTANGLE) {
		// Rectangle
		if (code & SCI_PATTERN_CODE_USE_TEXTURE) {
			vectorPatternTexturedBox(rect, color, priority, control, texture);
		} else {
			vectorPatternBox(rect, color, priority, control);
		}

	} else {
		// Circle
		if (code & SCI_PATTERN_CODE_USE_TEXTURE) {
			vectorPatternTexturedCircle(rect, size, color, priority, control, texture);
		} else {
			vectorPatternCircle(rect, size, color, priority, control);
		}
	}
}

} // End of namespace Sci
