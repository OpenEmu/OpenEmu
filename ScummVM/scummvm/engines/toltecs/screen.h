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

#ifndef TOLTECS_SCREEN_H
#define TOLTECS_SCREEN_H

#include "graphics/surface.h"
#include "toltecs/toltecs.h"

namespace Toltecs {

struct DrawRequest {
	int16 x, y;
	int16 resIndex;
	uint16 flags;
	int16 baseColor;
	int8 scaling;
};

struct SpriteDrawItem {
	int16 x, y;
	int16 width, height;
	int16 origWidth, origHeight;
	int16 resIndex, frameNum;
	uint32 offset;
	int16 xdelta, ydelta;
	uint16 flags;
	int16 skipX, yerror;
	int16 priority;
	int16 baseColor;
};

struct SpriteFrameEntry {
	int16 y, x, h, w;
	uint32 offset;
	SpriteFrameEntry() {
	}
	SpriteFrameEntry(byte *data) {
		y = READ_LE_UINT16(data + 0);
		x = READ_LE_UINT16(data + 2);
		h = READ_LE_UINT16(data + 4);
		w = READ_LE_UINT16(data + 6);
		offset = READ_LE_UINT32(data + 8);
	}
};

class Font {
public:
	Font(byte *fontData) : _fontData(fontData) {
	}
	~Font() {
	}
	int16 getSpacing() const {
		return _fontData[1];
	}
	int16 getHeight() const {
		return _fontData[2];
	}
	int16 getWidth() const {
		return _fontData[3];
	}
	int16 getCharWidth(byte ch) const {
		return _fontData[4 + (ch - 0x21)];
	}
	byte *getCharData(byte ch) const {
		return _fontData + 0x298 + READ_LE_UINT16(&_fontData[0xE0 + (ch - 0x21) * 2]);
	}
	int16 getTextWidth(const byte *text) {
		int16 width = 0;
		while (*text && *text < 0xF0) {
			byte ch = *text++;
			if (ch <= 0x20) {
				width += getWidth();
			} else {
				width += getCharWidth(ch) + getSpacing() - 1;
			}
		}
		return width;
	}

protected:
	byte *_fontData;
};

struct PixelPacket {
	byte count;
	byte pixel;
};

enum SpriteReaderStatus {
	kSrsPixelsLeft,
	kSrsEndOfLine,
	kSrsEndOfSprite
};

class SpriteFilter {
public:
	SpriteFilter(const SpriteDrawItem &sprite) : _sprite(&sprite) {
	}
	virtual ~SpriteFilter() {}
	virtual SpriteReaderStatus readPacket(PixelPacket &packet) = 0;
protected:
	const SpriteDrawItem *_sprite;
};

struct TextRect {
	int16 x, y;
	int16 width, length;
};

struct TalkTextItem {
	int16 duration;
	int16 slotIndex;
	int16 slotOffset;
	int16 fontNum;
	byte color;
	byte lineCount;
	TextRect lines[15];
};

struct GuiTextWrapState {
	int16 len1, len2;
	byte *sourceString;
	byte *destString;
	int16 width;
	byte textBuffer[100];
};

class RenderQueue;

class Screen {
public:
	Screen(ToltecsEngine *vm);
	~Screen();

	void unpackRle(byte *source, byte *dest, uint16 width, uint16 height);

	void loadMouseCursor(uint resIndex);

	void drawGuiImage(int16 x, int16 y, uint resIndex);

	void startShakeScreen(int16 shakeCounter);
	void stopShakeScreen();
	bool updateShakeScreen();

	// Sprite list
	void addStaticSprite(byte *spriteItem);
	void addAnimatedSprite(int16 x, int16 y, int16 fragmentId, byte *data, int16 *spriteArray, bool loop, int mode);
	void clearSprites();

	// Sprite drawing
	void drawSprite(const SpriteDrawItem &sprite);
	void drawSpriteCore(byte *dest, SpriteFilter &reader, const SpriteDrawItem &sprite);
	void blastSprite(int16 x, int16 y, int16 fragmentId, int16 resIndex, uint16 flags);

	// Verb line
	void updateVerbLine(int16 slotIndex, int16 slotOffset);

	// Talk text
	void updateTalkText(int16 slotIndex, int16 slotOffset);
	void addTalkTextRect(Font &font, int16 x, int16 &y, int16 length, int16 width, TalkTextItem *item);
	void addTalkTextItemsToRenderQueue();
	int16 getTalkTextDuration();
	void finishTalkTextItems();
	void keepTalkTextItemsAlive();

	// Font/text
	void registerFont(uint fontIndex, uint resIndex);
	void drawGuiTextMulti(byte *textData);
	int16 wrapGuiText(uint fontResIndex, int maxWidth, GuiTextWrapState &wrapState);
	void drawGuiText(int16 x, int16 y, byte fontColor1, byte fontColor2, uint fontResIndex, GuiTextWrapState &wrapState);

	int16 drawString(int16 x, int16 y, byte color, uint fontResIndex, const byte *text, int len = -1, int16 *ywobble = NULL, bool outline = false);
	void drawChar(const Font &font, byte *dest, int16 x, int16 y, byte ch, byte color, bool outline);

	void drawSurface(int16 x, int16 y, Graphics::Surface *surface);

	void saveState(Common::WriteStream *out);
	void loadState(Common::ReadStream *in);

	uint getFontResIndex(int fontNum) const { return _fontResIndexArray[fontNum]; }

//protected:
public:

	struct VerbLineItem {
		int16 slotIndex;
		int16 slotOffset;
	};

	struct Rect {
		int16 x, y, width, height;
	};

	ToltecsEngine *_vm;

	byte *_frontScreen, *_backScreen;

	uint _fontResIndexArray[10];
	byte _fontColor1, _fontColor2;

	// Screen shaking
	bool _shakeActive;
	uint32 _shakeTime;
	int16 _shakeCounterInit, _shakeCounter;
	int _shakePos;

	// Verb line
	int16 _verbLineNum;
	VerbLineItem _verbLineItems[8];
	int16 _verbLineX, _verbLineY, _verbLineWidth;
	int16 _verbLineCount;

	// Talk text
	int16 _talkTextX, _talkTextY;
	int16 _talkTextMaxWidth;
	byte _talkTextFontColor;
	int16 _talkTextItemNum;
	TalkTextItem _talkTextItems[5];

	RenderQueue *_renderQueue;
	bool _fullRefresh;
	bool _guiRefresh;

	bool createSpriteDrawItem(const DrawRequest &drawRequest, SpriteDrawItem &sprite);
	void addDrawRequest(const DrawRequest &drawRequest);

};

} // End of namespace Toltecs

#endif /* TOLTECS_SCREEN_H */
