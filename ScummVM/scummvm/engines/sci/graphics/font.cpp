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
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/font.h"

namespace Sci {

GfxFontFromResource::GfxFontFromResource(ResourceManager *resMan, GfxScreen *screen, GuiResourceId resourceId)
	: _resourceId(resourceId), _screen(screen), _resMan(resMan) {
	assert(resourceId != -1);

	// Workaround: lsl1sci mixes its own internal fonts with the global
	// SCI ones, so we translate them here, by removing their extra bits
	if (!resMan->testResource(ResourceId(kResourceTypeFont, resourceId)))
		resourceId = resourceId & 0x7ff;

	_resource = resMan->findResource(ResourceId(kResourceTypeFont, resourceId), true);
	if (!_resource) {
		error("font resource %d not found", resourceId);
	}
	_resourceData = _resource->data;

	_numChars = READ_SCI32ENDIAN_UINT16(_resourceData + 2);
	_fontHeight = READ_SCI32ENDIAN_UINT16(_resourceData + 4);
	_chars = new Charinfo[_numChars];
	// filling info for every char
	for (int16 i = 0; i < _numChars; i++) {
		_chars[i].offset = READ_SCI32ENDIAN_UINT16(_resourceData + 6 + i * 2);
		_chars[i].w = _resourceData[_chars[i].offset];
		_chars[i].h = _resourceData[_chars[i].offset + 1];
	}
}

GfxFontFromResource::~GfxFontFromResource() {
	delete[] _chars;
	_resMan->unlockResource(_resource);
}

GuiResourceId GfxFontFromResource::getResourceId() {
	return _resourceId;
}

byte GfxFontFromResource::getHeight() {
	return _fontHeight;
}
byte GfxFontFromResource::getCharWidth(uint16 chr) {
	return chr < _numChars ? _chars[chr].w : 0;
}
byte GfxFontFromResource::getCharHeight(uint16 chr) {
	return chr < _numChars ? _chars[chr].h : 0;
}
byte *GfxFontFromResource::getCharData(uint16 chr) {
	return chr < _numChars ? _resourceData + _chars[chr].offset + 2 : 0;
}

void GfxFontFromResource::draw(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput) {
	// Make sure we're comparing against the correct dimensions
	// If the font we're drawing is already upscaled, make sure we use the full screen width/height
	uint16 screenWidth = _screen->fontIsUpscaled() ? _screen->getDisplayWidth() : _screen->getWidth();
	uint16 screenHeight = _screen->fontIsUpscaled() ? _screen->getDisplayHeight() : _screen->getHeight();

	int charWidth = MIN<int>(getCharWidth(chr), screenWidth - left);
	int charHeight = MIN<int>(getCharHeight(chr), screenHeight - top);
	byte b = 0, mask = 0xFF;
	int y = 0;
	int16 greyedTop = top;

	byte *pIn = getCharData(chr);
	for (int i = 0; i < charHeight; i++, y++) {
		if (greyedOutput)
			mask = ((greyedTop++) % 2) ? 0xAA : 0x55;
		for (int done = 0; done < charWidth; done++) {
			if ((done & 7) == 0) // fetching next data byte
				b = *(pIn++) & mask;
			if (b & 0x80) // if MSB is set - paint it
				_screen->putFontPixel(top, left + done, y, color);
			b = b << 1;
		}
	}
}

#ifdef ENABLE_SCI32

void GfxFontFromResource::drawToBuffer(uint16 chr, int16 top, int16 left, byte color, bool greyedOutput, byte *buffer, int16 bufWidth, int16 bufHeight) {
	int charWidth = MIN<int>(getCharWidth(chr), bufWidth - left);
	int charHeight = MIN<int>(getCharHeight(chr), bufHeight - top);
	byte b = 0, mask = 0xFF;
	int y = 0;
	int16 greyedTop = top;

	byte *pIn = getCharData(chr);
	for (int i = 0; i < charHeight; i++, y++) {
		if (greyedOutput)
			mask = ((greyedTop++) % 2) ? 0xAA : 0x55;
		for (int done = 0; done < charWidth; done++) {
			if ((done & 7) == 0) // fetching next data byte
				b = *(pIn++) & mask;
			if (b & 0x80) {	// if MSB is set - paint it
				int offset = (top + y) * bufWidth + (left + done);
				buffer[offset] = color;
			}
			b = b << 1;
		}
	}
}

#endif

} // End of namespace Sci
