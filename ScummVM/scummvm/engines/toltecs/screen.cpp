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
 *
 */

#include "graphics/cursorman.h"

#include "toltecs/toltecs.h"
#include "toltecs/palette.h"
#include "toltecs/render.h"
#include "toltecs/resource.h"
#include "toltecs/screen.h"
#include "toltecs/script.h"

namespace Toltecs {

Screen::Screen(ToltecsEngine *vm) : _vm(vm) {

	_frontScreen = new byte[268800];
	_backScreen = new byte[870400];

	memset(_fontResIndexArray, 0, sizeof(_fontResIndexArray));
	_fontColor1 = 0;
	_fontColor2 = 0;

	// Screen shaking
	_shakeActive = false;
	_shakeTime = 0;
	_shakeCounterInit = 0;
	_shakeCounter = 0;
	_shakePos = 0;
	_shakeTime = 0;

	// Verb line
	_verbLineNum = 0;
	memset(_verbLineItems, 0, sizeof(_verbLineItems));
	_verbLineX = 160;
	_verbLineY = 2;
	_verbLineWidth = 20;
	_verbLineCount = 0;

	// Talk text
	_talkTextItemNum = 0;
	memset(_talkTextItems, 0, sizeof(_talkTextItems));
	_talkTextX = 0;
	_talkTextY = 0;
	_talkTextFontColor = 0;
	_talkTextMaxWidth = 520;

	_renderQueue = new RenderQueue(_vm);
	_fullRefresh = false;
	_guiRefresh = false;

}

Screen::~Screen() {

	delete[] _frontScreen;
	delete[] _backScreen;

	delete _renderQueue;

}

void Screen::unpackRle(byte *source, byte *dest, uint16 width, uint16 height) {
	int32 size = width * height;
	while (size > 0) {
		byte a = *source++;
		byte b = *source++;
		if (a == 0) {
			dest += b;
			size -= b;
		} else {
			b = ((b << 4) & 0xF0) | ((b >> 4) & 0x0F);
			memset(dest, b, a);
			dest += a;
			size -= a;
		}
	}
}

void Screen::loadMouseCursor(uint resIndex) {
	byte mouseCursor[16 * 16], *mouseCursorP = mouseCursor;
	byte *cursorData = _vm->_res->load(resIndex)->data;
	for (int i = 0; i < 32; i++) {
		byte pixel;
		byte mask1 = *cursorData++;
		byte mask2 = *cursorData++;
		for (int j = 0; j < 8; j++) {
			pixel = 0xE5;
			if ((mask2 & 0x80) == 0)
				pixel = 0xE0;
			mask2 <<= 1;
			if ((mask1 & 0x80) == 0)
				pixel = 0;
			mask1 <<= 1;
			*mouseCursorP++ = pixel;
		}
	}
	// FIXME: Where's the cursor hotspot? Using 8, 8 seems good enough for now.
	CursorMan.replaceCursor(mouseCursor, 16, 16, 8, 8, 0);
}

void Screen::drawGuiImage(int16 x, int16 y, uint resIndex) {

	byte *imageData = _vm->_res->load(resIndex)->data;
	int16 headerSize = READ_LE_UINT16(imageData);
	int16 width = imageData[2];
	int16 height = imageData[3];
	int16 workWidth = width, workHeight = height;
	imageData += headerSize;

	byte *dest = _frontScreen + x + (y + _vm->_cameraHeight) * 640;

	//debug(0, "Screen::drawGuiImage() x = %d; y = %d; w = %d; h = %d; resIndex = %d", x, y, width, height, resIndex);

	while (workHeight > 0) {
		int count = 1;
		byte pixel = *imageData++;
		if (pixel & 0x80) {
			pixel &= 0x7F;
			count = *imageData++;
			count += 2;
		}
		pixel = pixel + 0xE0;
		while (count-- && workHeight > 0) {
			*dest++ = pixel;
			workWidth--;
			if (workWidth == 0) {
				workHeight--;
				dest += 640 - width;
				workWidth = width;
			}
		}
	}

	_guiRefresh = true;

}

void Screen::startShakeScreen(int16 shakeCounter) {
	_shakeActive = true;
	_shakeTime = 0;
	_shakeCounterInit = shakeCounter;
	_shakeCounter = shakeCounter;
	_shakePos = 0;
}

void Screen::stopShakeScreen() {
	_shakeActive = false;
	_vm->_system->setShakePos(0);
}

bool Screen::updateShakeScreen() {
	// Assume shaking happens no more often than 50 times per second
	if (_shakeActive && _vm->_system->getMillis() - _shakeTime >= 20) {
		_shakeTime = _vm->_system->getMillis();
		_shakeCounter--;
		if (_shakeCounter == 0) {
			_shakeCounter = _shakeCounterInit;
			_shakePos ^= 8;
			_vm->_system->setShakePos(_shakePos);
			return true;
		}
	}
	return false;
}

void Screen::addStaticSprite(byte *spriteItem) {

	DrawRequest drawRequest;
	memset(&drawRequest, 0, sizeof(drawRequest));

	drawRequest.y = READ_LE_UINT16(spriteItem + 0);
	drawRequest.x = READ_LE_UINT16(spriteItem + 2);
	int16 fragmentId = READ_LE_UINT16(spriteItem + 4);
	drawRequest.baseColor = _vm->_palette->findFragment(fragmentId) & 0xFF;
	drawRequest.resIndex = READ_LE_UINT16(spriteItem + 6);
	drawRequest.flags = READ_LE_UINT16(spriteItem + 8);
	drawRequest.scaling = 0;

	debug(0, "Screen::addStaticSprite() x = %d; y = %d; baseColor = %d; resIndex = %d; flags = %04X", drawRequest.x, drawRequest.y, drawRequest.baseColor, drawRequest.resIndex, drawRequest.flags);

	addDrawRequest(drawRequest);

}

void Screen::addAnimatedSprite(int16 x, int16 y, int16 fragmentId, byte *data, int16 *spriteArray, bool loop, int mode) {

	//debug(0, "Screen::addAnimatedSprite(%d, %d, %d)", x, y, fragmentId);

	DrawRequest drawRequest;
	memset(&drawRequest, 0, sizeof(drawRequest));

	drawRequest.x = x;
	drawRequest.y = y;
	drawRequest.baseColor = _vm->_palette->findFragment(fragmentId) & 0xFF;

	if (mode == 1) {
		drawRequest.scaling = _vm->_segmap->getScalingAtPoint(drawRequest.x, drawRequest.y);
	} else if (mode == 2) {
		drawRequest.scaling = 0;
	}

	int16 count = FROM_LE_16(spriteArray[0]);

	//debug(0, "count = %d", count);

	for (int16 index = 1; index <= count; index++) {

		byte *spriteItem = data + FROM_LE_16(spriteArray[index]);

		uint16 loopNum = READ_LE_UINT16(spriteItem + 0) & 0x7FFF;
		uint16 loopCount = READ_LE_UINT16(spriteItem + 2);
		uint16 frameNum = READ_LE_UINT16(spriteItem + 4);
		uint16 frameCount = READ_LE_UINT16(spriteItem + 6);
		drawRequest.resIndex = READ_LE_UINT16(spriteItem + 8);
		drawRequest.flags = READ_LE_UINT16(spriteItem + 10 + loopNum * 2);

		debug(0, "Screen::addAnimatedSprite(%d of %d) loopNum = %d; loopCount = %d; frameNum = %d; frameCount = %d; resIndex = %d; flags = %04X, mode = %d",
			index, count, loopNum, loopCount, frameNum, frameCount, drawRequest.resIndex, drawRequest.flags, mode);

		addDrawRequest(drawRequest);

		frameNum++;
		if (frameNum == frameCount) {
			frameNum = 0;
			loopNum++;
			if (loopNum == loopCount) {
				if (loop) {
					loopNum = 0;
				} else {
					loopNum--;
				}
			}
		} else {
			loopNum |= 0x8000;
		}

		WRITE_LE_UINT16(spriteItem + 0, loopNum);
		WRITE_LE_UINT16(spriteItem + 4, frameNum);

	}

}

void Screen::clearSprites() {

}

void Screen::blastSprite(int16 x, int16 y, int16 fragmentId, int16 resIndex, uint16 flags) {

	DrawRequest drawRequest;
	SpriteDrawItem sprite;

	drawRequest.x = x;
	drawRequest.y = y;
	drawRequest.baseColor = _vm->_palette->findFragment(fragmentId) & 0xFF;
	drawRequest.resIndex = resIndex;
	drawRequest.flags = flags;
	drawRequest.scaling = 0;

	if (createSpriteDrawItem(drawRequest, sprite)) {
		sprite.x -= _vm->_cameraX;
		sprite.y -= _vm->_cameraY;
		drawSprite(sprite);
	}

}

void Screen::updateVerbLine(int16 slotIndex, int16 slotOffset) {

	debug(0, "Screen::updateVerbLine() _verbLineNum = %d; _verbLineX = %d; _verbLineY = %d; _verbLineWidth = %d; _verbLineCount = %d",
		_verbLineNum, _verbLineX, _verbLineY, _verbLineWidth, _verbLineCount);

	Font font(_vm->_res->load(_fontResIndexArray[0])->data);

	_verbLineItems[_verbLineNum].slotIndex = slotIndex;
	_verbLineItems[_verbLineNum].slotOffset = slotOffset;

	// First clear the line
	int16 y = _verbLineY;
	for (int16 i = 0; i < _verbLineCount; i++) {
		byte *dest = _frontScreen + _verbLineX - _verbLineWidth / 2 + (y - 1 + _vm->_cameraHeight) * 640;
		for (int16 j = 0; j < 20; j++) {
			memset(dest, 0xE0, _verbLineWidth);
			dest += 640;
		}
		y += 18;
	}

	GuiTextWrapState wrapState;
	int16 len = 0;
	wrapState.width = 0;
	wrapState.destString = wrapState.textBuffer;
	wrapState.len1 = 0;
	wrapState.len2 = 0;

	y = _verbLineY;

	memset(wrapState.textBuffer, 0, sizeof(wrapState.textBuffer));

	for (int16 i = 0; i <= _verbLineNum; i++) {
		wrapState.sourceString = _vm->_script->getSlotData(_verbLineItems[i].slotIndex) + _verbLineItems[i].slotOffset;
		len = wrapGuiText(_fontResIndexArray[0], _verbLineWidth, wrapState);
		wrapState.len1 += len;
	}

	if (_verbLineCount != 1) {
		int16 charWidth = 0;
		if (*wrapState.sourceString < 0xF0) {
			while (*wrapState.sourceString > 0x20 && *wrapState.sourceString < 0xF0 && len > 0) {
				byte ch = *wrapState.sourceString--;
				wrapState.len1--;
				len--;
				charWidth = font.getCharWidth(ch) + font.getSpacing() - 1;
				wrapState.width -= charWidth;
			}
			wrapState.width += charWidth;
			wrapState.sourceString++;
			wrapState.len1 -= len;
			wrapState.len2 = len + 1;

			drawGuiText(_verbLineX - 1 - (wrapState.width / 2), y, 0xF9, 0xFF, _fontResIndexArray[0], wrapState);

			wrapState.destString = wrapState.textBuffer;
			wrapState.width = 0;
			len = wrapGuiText(_fontResIndexArray[0], _verbLineWidth, wrapState);
			wrapState.len1 += len;

			y += 9;
		}
		y += 9;
	}

	wrapState.len1 -= len;
	wrapState.len2 = len;

	drawGuiText(_verbLineX - 1 - (wrapState.width / 2), y, 0xF9, 0xFF, _fontResIndexArray[0], wrapState);

	_guiRefresh = true;

}

void Screen::updateTalkText(int16 slotIndex, int16 slotOffset) {

	int16 x, y, maxWidth, width, length;
	byte durationModifier = 1;
	byte *textData = _vm->_script->getSlotData(slotIndex) + slotOffset;

	TalkTextItem *item = &_talkTextItems[_talkTextItemNum];

	item->fontNum = 0;
	item->color = _talkTextFontColor;

	x = CLIP<int16>(_talkTextX - _vm->_cameraX, 120, _talkTextMaxWidth);
	y = CLIP<int16>(_talkTextY - _vm->_cameraY, 4, _vm->_cameraHeight - 16);

	maxWidth = 624 - ABS(x - 320) * 2;

	while (1) {
		if (*textData == 0x0A) {
			x = CLIP<int16>(READ_LE_UINT16(&textData[3]), 120, _talkTextMaxWidth);
			y = CLIP<int16>(READ_LE_UINT16(&textData[1]), 4, _vm->_cameraHeight - 16);
			maxWidth = 624 - ABS(x - 320) * 2;
			textData += 4;
		} else if (*textData == 0x14) {
			item->color = ((textData[1] << 4) & 0xF0) | ((textData[1] >> 4) & 0x0F);
			textData += 2;
		} else if (*textData == 0x19) {
			durationModifier = textData[1];
			textData += 2;
		} else if (*textData < 0x0A) {
			item->fontNum = textData[0];
			// FIXME: Some texts request a font which isn't registered so we change it to a font that is
			if (_fontResIndexArray[item->fontNum] == 0)
				item->fontNum = 0;
			textData += 1;
		} else
			break;
	}

	item->slotIndex = slotIndex;
	item->slotOffset = textData - _vm->_script->getSlotData(slotIndex);

	width = 0;
	length = 0;

	item->duration = 0;
	item->lineCount = 0;

	Font font(_vm->_res->load(_fontResIndexArray[item->fontNum])->data);
	int16 wordLength, wordWidth;

	while (*textData < 0xF0) {
		if (*textData == 0x1E) {
			textData++;
			addTalkTextRect(font, x, y, length, width, item);
			width = 0;
			length = 0;
		} else {
			wordLength = 0;
			wordWidth = 0;
			while (*textData >= 0x20 && *textData < 0xF0) {
				byte ch = *textData++;
				wordLength++;
				if (ch == 0x20) {
					wordWidth += font.getWidth();
					break;
				} else {
					wordWidth += font.getCharWidth(ch) + font.getSpacing() - 1;
				}
			}
			if (width + wordWidth > maxWidth + font.getWidth()) {
				addTalkTextRect(font, x, y, length, width, item);
				width = wordWidth;
				length = wordLength;
			} else {
				width += wordWidth;
				length += wordLength;
			}
		}
	}

	addTalkTextRect(font, x, y, length, width, item);

	if (item->lineCount > 0) {
		int16 ysub = (font.getHeight() - 1) * item->lineCount;
		if (item->lines[0].y - 4 < ysub)
			ysub = item->lines[0].y - 4;
		for (int16 l = 0; l < item->lineCount; l++)
			item->lines[l].y -= ysub;
	}

	int16 textDurationMultiplier = item->duration + 8;
	if (_vm->_doSpeech && *textData == 0xFE) {
		textDurationMultiplier += 100;
	}
	item->duration = 4 * textDurationMultiplier * durationModifier;

}

void Screen::addTalkTextRect(Font &font, int16 x, int16 &y, int16 length, int16 width, TalkTextItem *item) {

	if (width > 0) {
		TextRect *textRect = &item->lines[item->lineCount];
		width = width + 1 - font.getSpacing();
		textRect->width = width;
		item->duration += length;
		textRect->length = length;
		textRect->y = y;
		textRect->x = CLIP<int16>(x - width / 2, 0, 640);
		item->lineCount++;
	}

	y += font.getHeight() - 1;

}

void Screen::addTalkTextItemsToRenderQueue() {
	for (int16 i = 0; i <= _talkTextItemNum; i++) {
		TalkTextItem *item = &_talkTextItems[i];
		byte *text = _vm->_script->getSlotData(item->slotIndex) + item->slotOffset;

		if (item->fontNum == -1 || item->duration == 0)
			continue;

		//item->duration -= _vm->_counter01;
		item->duration--;
		if (item->duration < 0)
			item->duration = 0;

		if (!_vm->_cfgText)
			return;

		for (byte j = 0; j < item->lineCount; j++) {
			_renderQueue->addText(item->lines[j].x, item->lines[j].y, item->color,
					_fontResIndexArray[item->fontNum], text, item->lines[j].length);
			text += item->lines[j].length;
		}
	}
}

int16 Screen::getTalkTextDuration() {
	return _talkTextItems[_talkTextItemNum].duration;
}

void Screen::finishTalkTextItems() {
	for (int16 i = 0; i <= _talkTextItemNum; i++) {
		_talkTextItems[i].duration = 0;
	}
}

void Screen::keepTalkTextItemsAlive() {
	for (int16 i = 0; i <= _talkTextItemNum; i++) {
		TalkTextItem *item = &_talkTextItems[i];
		if (item->fontNum == -1)
			item->duration = 0;
		else if (item->duration > 0)
			item->duration = 2;
	}
}

void Screen::registerFont(uint fontIndex, uint resIndex) {
	_fontResIndexArray[fontIndex] = resIndex;
}

void Screen::drawGuiTextMulti(byte *textData) {

	int16 x = 0, y = 0;

	// Really strange stuff.
	for (int i = 30; i >= 0; i--) {
		if (textData[i] >= 0xF0)
			break;
		if (i == 0)
			return;
	}

	GuiTextWrapState wrapState;
	wrapState.sourceString = textData;

	do {
		if (*wrapState.sourceString == 0x0A) {
			// Set text position
			y = wrapState.sourceString[1];
			x = READ_LE_UINT32(wrapState.sourceString + 2);
			wrapState.sourceString += 4;
		} else if (*wrapState.sourceString == 0x0B) {
			// Inc text position
			y += wrapState.sourceString[1];
			x += wrapState.sourceString[2];
			wrapState.sourceString += 3;
		} else {
			wrapState.destString = wrapState.textBuffer;
			wrapState.width = 0;
			wrapState.len1 = 0;
			wrapState.len2 = wrapGuiText(_fontResIndexArray[1], 640, wrapState);
			drawGuiText(x - wrapState.width / 2, y, _fontColor1, _fontColor2, _fontResIndexArray[1], wrapState);
		}
	} while (*wrapState.sourceString != 0xFF);

	_guiRefresh = true;

}

int16 Screen::wrapGuiText(uint fontResIndex, int maxWidth, GuiTextWrapState &wrapState) {

	Font font(_vm->_res->load(fontResIndex)->data);
	int16 len = 0;

	while (*wrapState.sourceString >= 0x20 && *wrapState.sourceString < 0xF0) {
		byte ch = *wrapState.sourceString;
		byte charWidth;
		if (ch <= 0x20)
			charWidth = font.getWidth();
		else
			charWidth = font.getCharWidth(ch) + font.getSpacing() - 1;
		if (wrapState.width + charWidth >= maxWidth)
			break;
		len++;
		wrapState.width += charWidth;
		*wrapState.destString++ = *wrapState.sourceString++;
	}

	return len;

}

void Screen::drawGuiText(int16 x, int16 y, byte fontColor1, byte fontColor2, uint fontResIndex, GuiTextWrapState &wrapState) {

	debug(0, "Screen::drawGuiText(%d, %d, %d, %d, %d) wrapState.len1 = %d; wrapState.len2 = %d", x, y, fontColor1, fontColor2, fontResIndex, wrapState.len1, wrapState.len2);

	int16 ywobble = 1;

	x = drawString(x + 1, y + _vm->_cameraHeight, fontColor1, fontResIndex, wrapState.textBuffer, wrapState.len1, &ywobble, false);
	x = drawString(x, y + _vm->_cameraHeight, fontColor2, fontResIndex, wrapState.textBuffer + wrapState.len1, wrapState.len2, &ywobble, false);

}

int16 Screen::drawString(int16 x, int16 y, byte color, uint fontResIndex, const byte *text, int len, int16 *ywobble, bool outline) {

	//debug(0, "Screen::drawString(%d, %d, %d, %d)", x, y, color, fontResIndex);

	Font font(_vm->_res->load(fontResIndex)->data);

	if (len == -1)
		len = strlen((const char*)text);

	int16 yadd = 0;
	if (ywobble)
		yadd = *ywobble;

 	while (len--) {
		byte ch = *text++;
		if (ch <= 0x20) {
			x += font.getWidth();
		} else {
			drawChar(font, _frontScreen, x, y - yadd, ch, color, outline);
			x += font.getCharWidth(ch) + font.getSpacing() - 1;
			yadd = -yadd;
		}
	}

	if (ywobble)
		*ywobble = yadd;

	return x;

}

void Screen::drawChar(const Font &font, byte *dest, int16 x, int16 y, byte ch, byte color, bool outline) {

	int16 charWidth, charHeight;
	byte *charData;

	dest += x + y * 640;

	charWidth = font.getCharWidth(ch);
	//charHeight = font.getHeight() - 2;//Why was this here?!
	charHeight = font.getHeight();
	charData = font.getCharData(ch);

	while (charHeight--) {
		byte lineWidth = charWidth;
		while (lineWidth > 0) {
			byte count = charData[0] & 0x0F;
			byte flags = charData[0] & 0xF0;
			charData++;
			if ((flags & 0x80) == 0) {
 				if (flags & 0x10) {
					memset(dest, color, count);
				} else if (outline) {
					memset(dest, 0, count);
				}
			}
			dest += count;
			lineWidth -= count;
		}
		dest += 640 - charWidth;
	}

}

void Screen::drawSurface(int16 x, int16 y, Graphics::Surface *surface) {

	int16 skipX = 0;
	int16 width = surface->w;
	int16 height = surface->h;
	byte *surfacePixels = (byte *)surface->getBasePtr(0, 0);
	byte *frontScreen;

	// Not on screen, skip
	if (x + width < 0 || y + height < 0 || x >= 640 || y >= _vm->_cameraHeight)
		return;

	if (x < 0) {
		skipX = -x;
		x = 0;
		width -= skipX;
	}

	if (y < 0) {
		int16 skipY = -y;
		surfacePixels += surface->w * skipY;
		y = 0;
		height -= skipY;
	}

	if (x + width >= 640) {
		width -= x + width - 640;
	}

	if (y + height >= _vm->_cameraHeight) {
		height -= y + height - _vm->_cameraHeight;
	}

	frontScreen = _vm->_screen->_frontScreen + x + (y * 640);

	for (int16 h = 0; h < height; h++) {
		surfacePixels += skipX;
		for (int16 w = 0; w < width; w++) {
			if (*surfacePixels != 0xFF)
				*frontScreen = *surfacePixels;
			frontScreen++;
			surfacePixels++;
		}
		frontScreen += 640 - width;
		surfacePixels += surface->w - width - skipX;
	}

}

void Screen::saveState(Common::WriteStream *out) {

	// Save verb line
	out->writeUint16LE(_verbLineNum);
	out->writeUint16LE(_verbLineX);
	out->writeUint16LE(_verbLineY);
	out->writeUint16LE(_verbLineWidth);
	out->writeUint16LE(_verbLineCount);
	for (int i = 0; i < 8; i++) {
		out->writeUint16LE(_verbLineItems[i].slotIndex);
		out->writeUint16LE(_verbLineItems[i].slotOffset);
	}

	// Save talk text items
	out->writeUint16LE(_talkTextX);
	out->writeUint16LE(_talkTextY);
	out->writeUint16LE(_talkTextMaxWidth);
	out->writeByte(_talkTextFontColor);
	out->writeUint16LE(_talkTextItemNum);
	for (int i = 0; i < 5; i++) {
		out->writeUint16LE(_talkTextItems[i].duration);
		out->writeUint16LE(_talkTextItems[i].slotIndex);
		out->writeUint16LE(_talkTextItems[i].slotOffset);
		out->writeUint16LE(_talkTextItems[i].fontNum);
		out->writeByte(_talkTextItems[i].color);
		out->writeByte(_talkTextItems[i].lineCount);
		for (int j = 0; j < _talkTextItems[i].lineCount; j++) {
			out->writeUint16LE(_talkTextItems[i].lines[j].x);
			out->writeUint16LE(_talkTextItems[i].lines[j].y);
			out->writeUint16LE(_talkTextItems[i].lines[j].width);
			out->writeUint16LE(_talkTextItems[i].lines[j].length);
		}
	}

	// Save GUI bitmap
	{
		byte *gui = _frontScreen + _vm->_cameraHeight * 640;
		for (int i = 0; i < _vm->_guiHeight; i++) {
			out->write(gui, 640);
			gui += 640;
		}
	}

	// Save fonts
	for (int i = 0; i < 10; i++)
		out->writeUint32LE(_fontResIndexArray[i]);
	out->writeByte(_fontColor1);
	out->writeByte(_fontColor2);

}

void Screen::loadState(Common::ReadStream *in) {

	// Load verb line
	_verbLineNum = in->readUint16LE();
	_verbLineX = in->readUint16LE();
	_verbLineY = in->readUint16LE();
	_verbLineWidth = in->readUint16LE();
	_verbLineCount = in->readUint16LE();
	for (int i = 0; i < 8; i++) {
		_verbLineItems[i].slotIndex = in->readUint16LE();
		_verbLineItems[i].slotOffset = in->readUint16LE();
	}

	// Load talk text items
	_talkTextX = in->readUint16LE();
	_talkTextY = in->readUint16LE();
	_talkTextMaxWidth = in->readUint16LE();
	_talkTextFontColor = in->readByte();
	_talkTextItemNum = in->readUint16LE();
	for (int i = 0; i < 5; i++) {
		_talkTextItems[i].duration = in->readUint16LE();
		_talkTextItems[i].slotIndex = in->readUint16LE();
		_talkTextItems[i].slotOffset = in->readUint16LE();
		_talkTextItems[i].fontNum = in->readUint16LE();
		_talkTextItems[i].color = in->readByte();
		_talkTextItems[i].lineCount = in->readByte();
		for (int j = 0; j < _talkTextItems[i].lineCount; j++) {
			_talkTextItems[i].lines[j].x = in->readUint16LE();
			_talkTextItems[i].lines[j].y = in->readUint16LE();
			_talkTextItems[i].lines[j].width = in->readUint16LE();
			_talkTextItems[i].lines[j].length = in->readUint16LE();
		}
	}

	// Load GUI bitmap
	{
		byte *gui = _frontScreen + _vm->_cameraHeight * 640;
		for (int i = 0; i < _vm->_guiHeight; i++) {
			in->read(gui, 640);
			gui += 640;
		}
		_guiRefresh = true;
	}

	// Load fonts
	for (int i = 0; i < 10; i++)
		_fontResIndexArray[i] = in->readUint32LE();
	_fontColor1 = in->readByte();
	_fontColor2 = in->readByte();

}

} // End of namespace Toltecs
