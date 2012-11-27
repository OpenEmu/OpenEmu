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

#include "made/screen.h"
#include "made/made.h"
#include "made/screenfx.h"
#include "made/database.h"

#include "common/system.h"

#include "graphics/surface.h"
#include "graphics/palette.h"
#include "graphics/cursorman.h"

namespace Made {

Screen::Screen(MadeEngine *vm) : _vm(vm) {

	_palette = new byte[768];
	_newPalette = new byte[768];

	_backgroundScreen = new Graphics::Surface();
	_backgroundScreen->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	_workScreen = new Graphics::Surface();
	_workScreen->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	_backgroundScreenDrawCtx.clipRect = Common::Rect(320, 200);
	_workScreenDrawCtx.clipRect = Common::Rect(320, 200);

	_backgroundScreenDrawCtx.destSurface = _backgroundScreen;
	_workScreenDrawCtx.destSurface = _workScreen;
	_clipArea.destSurface = _workScreen;

	// Screen mask is only needed in v2 games
	if (_vm->getGameID() != GID_RTZ) {
		_screenMask = new Graphics::Surface();
		_screenMask->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
		_maskDrawCtx.clipRect = Common::Rect(320, 200);
		_maskDrawCtx.destSurface = _screenMask;
	}

	for (int i = 0; i <= 3; i++)
		_excludeClipAreaEnabled[i] = false;

	_screenLock = false;
	_paletteLock = false;

	_paletteInitialized = false;
	_needPalette = false;
	_oldPaletteColorCount = 256;
	_paletteColorCount = 256;
	memset(_newPalette, 0, 768);
	memset(_palette, 0, 768);

	_ground = 1;
	_clip = 0;
	_exclude = 0;
	_mask = 0;

	_visualEffectNum = 0;
	_fx = new ScreenEffects(this);

	_textX = 0;
	_textY = 0;
	_textColor = 0;
	_textRect.left = 0;
	_textRect.top = 0;
	_textRect.right = 320;
	_textRect.bottom = 200;
	_font = NULL;
	_currentFontNum = 0;
	_fontDrawCtx.clipRect = Common::Rect(320, 200);
	_fontDrawCtx.destSurface = _backgroundScreen;

	clearChannels();
}

Screen::~Screen() {

	delete[] _palette;
	delete[] _newPalette;

	delete _backgroundScreen;
	delete _workScreen;
	if (_vm->getGameID() != GID_RTZ)
		delete _screenMask;
	delete _fx;
}

void Screen::clearScreen() {
	_backgroundScreen->fillRect(Common::Rect(0, 0, 320, 200), 0);
	_workScreen->fillRect(Common::Rect(0, 0, 320, 200), 0);
	if (_vm->getGameID() != GID_RTZ)
		_screenMask->fillRect(Common::Rect(0, 0, 320, 200), 0);
	_mask = 0;
	_needPalette = true;
}

void Screen::setExcludeArea(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {

	_excludeClipAreaEnabled[0] = false;
	_excludeClipAreaEnabled[1] = false;
	_excludeClipAreaEnabled[2] = false;
	_excludeClipAreaEnabled[3] = false;

	if (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0) {
		_excludeClipArea[0].clipRect = Common::Rect(320, 200);
		_excludeClipAreaEnabled[0] = true;
		return;
	}

	if (y1 > 0 && y2 > 0) {
		_excludeClipArea[0].clipRect = Common::Rect(320, y1);
		_excludeClipAreaEnabled[0] = true;
	}

	if (y1 < 200 && y2 < 200) {
		_excludeClipArea[1].clipRect = Common::Rect(0, y2, 320, 200);
		_excludeClipAreaEnabled[1] = true;
	}

	if (x1 > 0 && x2 > 0) {
		_excludeClipArea[2].clipRect = Common::Rect(0, y1, x1, y2);
		_excludeClipAreaEnabled[2] = true;
	}

	if (x1 < 320 && x2 < 320) {
		_excludeClipArea[3].clipRect = Common::Rect(x2, y1, 320, y2);
		_excludeClipAreaEnabled[3] = true;
	}

}

void Screen::drawSurface(Graphics::Surface *sourceSurface, int x, int y, int16 flipX, int16 flipY, int16 mask, const ClipInfo &clipInfo) {

	byte *source, *dest, *maskp = 0;
	int startX = 0;
	int startY = 0;
	int clipWidth = sourceSurface->w;
	int clipHeight = sourceSurface->h;

	if (x < clipInfo.clipRect.left) {
		startX = clipInfo.clipRect.left - x;
		clipWidth -= startX;
		x = clipInfo.clipRect.left;
	}

	if (y < clipInfo.clipRect.top) {
		startY = clipInfo.clipRect.top - y;
		clipHeight -= startY;
		y = clipInfo.clipRect.top;
	}

	if (x + clipWidth > clipInfo.clipRect.right) {
		clipWidth = clipInfo.clipRect.right - x;
	}

	if (y + clipHeight > clipInfo.clipRect.bottom) {
		clipHeight = clipInfo.clipRect.bottom - y;
	}

	source = (byte *)sourceSurface->getBasePtr(0, startY);
	dest = (byte *)clipInfo.destSurface->getBasePtr(x, y);
	if (_vm->getGameID() != GID_RTZ)
		maskp = (byte *)_maskDrawCtx.destSurface->getBasePtr(x, y);

	int32 sourcePitch, linePtrAdd, sourceAdd;
	byte *linePtr;

	if (flipX) {
		linePtrAdd = -1;
		sourceAdd = sourceSurface->w - startX - 1;
	} else {
		linePtrAdd = 1;
		sourceAdd = startX;
	}

	if (flipY) {
		sourcePitch = -sourceSurface->pitch;
		source += (clipHeight - 1) * sourceSurface->pitch;
	} else {
		sourcePitch = sourceSurface->pitch;
	}

	for (int16 yc = 0; yc < clipHeight; yc++) {
		linePtr = source + sourceAdd;
		for (int16 xc = 0; xc < clipWidth; xc++) {
			if (*linePtr && (_vm->getGameID() == GID_RTZ || (mask == 0 || (maskp && maskp[xc] == 0)))) {
				if (*linePtr)
					dest[xc] = *linePtr;
			}
			linePtr += linePtrAdd;
		}

		source += sourcePitch;
		dest += clipInfo.destSurface->pitch;
		if (_vm->getGameID() != GID_RTZ)
			maskp += _maskDrawCtx.destSurface->pitch;
	}

}

void Screen::setRGBPalette(byte *palRGB, int start, int count) {
	_vm->_system->getPaletteManager()->setPalette(palRGB, start, count);
}

uint16 Screen::updateChannel(uint16 channelIndex) {
	return channelIndex;
}

void Screen::deleteChannel(uint16 channelIndex) {
	if (channelIndex < 1 || channelIndex >= 100)
		return;
	_channels[channelIndex - 1].type = 0;
	_channels[channelIndex - 1].state = 0;
	_channels[channelIndex - 1].index = 0;
}

int16 Screen::getChannelType(uint16 channelIndex) {
	if (channelIndex < 1 || channelIndex >= 100)
		return -1;
	return _channels[channelIndex - 1].type;
}

int16 Screen::getChannelState(uint16 channelIndex) {
	if (channelIndex < 1 || channelIndex >= 100)
		return -1;
	return _channels[channelIndex - 1].state;
}

void Screen::setChannelState(uint16 channelIndex, int16 state) {
	if (channelIndex < 1 || channelIndex >= 100 || _channels[channelIndex - 1].type == 0)
		return;
	_channels[channelIndex - 1].state = state;
}

uint16 Screen::setChannelLocation(uint16 channelIndex, int16 x, int16 y) {
	if (channelIndex < 1 || channelIndex >= 100 || _channels[channelIndex - 1].type == 0)
		return 0;
	_channels[channelIndex - 1].x = x;
	_channels[channelIndex - 1].y = y;
	return updateChannel(channelIndex - 1) + 1;
}

uint16 Screen::setChannelContent(uint16 channelIndex, uint16 index) {
	if (channelIndex < 1 || channelIndex >= 100 || _channels[channelIndex - 1].type == 0)
		return 0;
	_channels[channelIndex - 1].index = index;
	return updateChannel(channelIndex - 1) + 1;
}

void Screen::setChannelUseMask(uint16 channelIndex) {
	if (channelIndex < 1 || channelIndex >= 100)
		return;
	_channels[channelIndex - 1].mask = _mask;
}

void Screen::drawSpriteChannels(const ClipInfo &clipInfo, int16 includeStateMask, int16 excludeStateMask) {

	for (int i = 0; i <= 3; i++)
		_excludeClipArea[i].destSurface = clipInfo.destSurface;

	_clipArea.destSurface = clipInfo.destSurface;

	for (uint16 i = 0; i < _channelsUsedCount; i++) {

		debug(2, "drawSpriteChannels() i = %d\n", i);

		if (((_channels[i].state & includeStateMask) == includeStateMask) && (_channels[i].state & excludeStateMask) == 0) {
			int16 flipX = _channels[i].state & 0x10;
			int16 flipY = _channels[i].state & 0x20;

			debug(2, "drawSpriteChannels() type = %d; index = %04X\n", _channels[i].type, _channels[i].index);

			switch (_channels[i].type) {

			case 1: // drawFlex
				if (_channels[i].state & 4) {
					drawFlex(_channels[i].index, _channels[i].x, _channels[i].y, flipX, flipY, _channels[i].mask, _clipArea);
				} else if (_channels[i].state & 8) {
					for (int excludeIndex = 0; excludeIndex < 4; excludeIndex++) {
						if (_excludeClipAreaEnabled[excludeIndex]) {
							drawFlex(_channels[i].index, _channels[i].x, _channels[i].y, flipX, flipY, _channels[i].mask, _excludeClipArea[excludeIndex]);
						}
					}
				} else {
					drawFlex(_channels[i].index, _channels[i].x, _channels[i].y, flipX, flipY, _channels[i].mask, clipInfo);
				}
				break;

			case 2: // drawObjectText
				printObjectText(_channels[i].index, _channels[i].x, _channels[i].y, _channels[i].fontNum, _channels[i].textColor, _channels[i].outlineColor, clipInfo);
				break;

			case 3: // drawAnimFrame
				if (_channels[i].state & 4) {
					drawAnimFrame(_channels[i].index, _channels[i].x, _channels[i].y, _channels[i].frameNum, flipX, flipY, _clipArea);
				} else if (_channels[i].state & 8) {
					for (int excludeIndex = 0; excludeIndex < 4; excludeIndex++) {
						if (_excludeClipAreaEnabled[excludeIndex]) {
							drawAnimFrame(_channels[i].index, _channels[i].x, _channels[i].y, _channels[i].frameNum, flipX, flipY, _excludeClipArea[excludeIndex]);
						}
					}
				} else {
					drawAnimFrame(_channels[i].index, _channels[i].x, _channels[i].y, _channels[i].frameNum, flipX, flipY, clipInfo);
				}
				break;

			case 4: // drawMenuText
				// Never used in any game
				break;

			default:
				break;

			}

		}

	}

}

void Screen::updateSprites() {
	// TODO: This needs some more work, dirty rectangles are currently not used

	memcpy(_workScreen->pixels, _backgroundScreen->pixels, 64000);

	drawSpriteChannels(_backgroundScreenDrawCtx, 3, 0);
	drawSpriteChannels(_workScreenDrawCtx, 1, 2);

	_vm->_system->copyRectToScreen(_workScreen->pixels, _workScreen->pitch, 0, 0, _workScreen->w, _workScreen->h);
	_vm->_screen->updateScreenAndWait(10);
}

void Screen::clearChannels() {
	for (uint16 i = 0; i < ARRAYSIZE(_channels); i++) {
		_channels[i].type = 0;
		_channels[i].index = 0;
		_channels[i].mask = 0;
	}
	_channelsUsedCount = 0;
}

uint16 Screen::drawFlex(uint16 flexIndex, int16 x, int16 y, int16 flipX, int16 flipY, int16 mask, const ClipInfo &clipInfo) {

	if (flexIndex == 0)
		return 0;

	PictureResource *flex = _vm->_res->getPicture(flexIndex);
	Graphics::Surface *sourceSurface = flex->getPicture();

	drawSurface(sourceSurface, x, y, flipX, flipY, mask, clipInfo);

	// Palette is set in showPage
	if (flex->hasPalette() && !_paletteLock && _needPalette) {
		byte *flexPalette = flex->getPalette();
		_oldPaletteColorCount = _paletteColorCount;
		_paletteColorCount = flex->getPaletteColorCount();
		memcpy(_newPalette, _palette, _oldPaletteColorCount * 3);
		memcpy(_palette, flexPalette, _paletteColorCount * 3);
		_needPalette = false;
	}

	_vm->_res->freeResource(flex);

	return 0;
}

void Screen::drawAnimFrame(uint16 animIndex, int16 x, int16 y, int16 frameNum, int16 flipX, int16 flipY, const ClipInfo &clipInfo) {

	if (frameNum < 0)
		return;

	AnimationResource *anim = _vm->_res->getAnimation(animIndex);
	Graphics::Surface *sourceSurface = anim->getFrame(frameNum);

	drawSurface(sourceSurface, x, y, flipX, flipY, 0, clipInfo);

	_vm->_res->freeResource(anim);
}

uint16 Screen::drawPic(uint16 index, int16 x, int16 y, int16 flipX, int16 flipY) {
	drawFlex(index, x, y, flipX, flipY, 0, _backgroundScreenDrawCtx);
	return 0;
}

uint16 Screen::drawMask(uint16 index, int16 x, int16 y) {
	drawFlex(index, x, y, 0, 0, 0, _maskDrawCtx);
	return 0;
}

uint16 Screen::drawAnimPic(uint16 animIndex, int16 x, int16 y, int16 frameNum, int16 flipX, int16 flipY) {
	drawAnimFrame(animIndex, x, y, frameNum, flipX, flipY, _backgroundScreenDrawCtx);
	return 0;
}

void Screen::addSprite(uint16 spriteIndex) {
	bool oldScreenLock = _screenLock;
	drawFlex(spriteIndex, 0, 0, 0, 0, 0, _backgroundScreenDrawCtx);
	_screenLock = oldScreenLock;
}

uint16 Screen::drawSprite(uint16 flexIndex, int16 x, int16 y) {
	return placeSprite(_channelsUsedCount + 1, flexIndex, x, y);
}

uint16 Screen::placeSprite(uint16 channelIndex, uint16 flexIndex, int16 x, int16 y) {

	debug(2, "placeSprite(%d, %04X, %d, %d)\n", channelIndex, flexIndex, x, y);

	if (channelIndex < 1 || channelIndex >= 100)
		return 0;

	channelIndex--;

	PictureResource *flex = _vm->_res->getPicture(flexIndex);

	if (flex) {
		//Graphics::Surface *surf = flex->getPicture();

		int16 state = 1;
		/*int16 x1, y1, x2, y2;

		x1 = x;
		y1 = y;
		x2 = x + surf->w + 1;
		y2 = y + surf->h + 1;*/

		if (_ground == 0)
			state |= 2;
		if (_clip != 0)
			state |= 4;
		if (_exclude != 0)
			state |= 8;

		_channels[channelIndex].state = state;
		_channels[channelIndex].type = 1;
		_channels[channelIndex].index = flexIndex;
		_channels[channelIndex].x = x;
		_channels[channelIndex].y = y;

		if (_channelsUsedCount <= channelIndex)
			_channelsUsedCount = channelIndex + 1;

		_vm->_res->freeResource(flex);
	} else {
		_channels[channelIndex].type = 0;
		_channels[channelIndex].state = 0;
	}

	return channelIndex + 1;

}

uint16 Screen::placeAnim(uint16 channelIndex, uint16 animIndex, int16 x, int16 y, int16 frameNum) {

	if (channelIndex < 1 || channelIndex >= 100)
		return 0;

	channelIndex--;

	AnimationResource *anim = _vm->_res->getAnimation(animIndex);

	if (anim) {

		int16 state = 1;
		/*int16 x1, y1, x2, y2;

		x1 = x;
		y1 = y;
		x2 = x + anim->getWidth();
		y2 = y + anim->getHeight();*/

		if (anim->getFlags() == 1 || _ground == 0)
			state |= 2;
		if (_clip != 0)
			state |= 4;
		if (_exclude != 0)
			state |= 8;

		_channels[channelIndex].state = state;
		_channels[channelIndex].type = 3;
		_channels[channelIndex].index = animIndex;
		_channels[channelIndex].frameNum = frameNum;
		_channels[channelIndex].x = x;
		_channels[channelIndex].y = y;

		if (_channelsUsedCount <= channelIndex)
			_channelsUsedCount = channelIndex + 1;

		_vm->_res->freeResource(anim);
	} else {
		_channels[channelIndex].type = 0;
		_channels[channelIndex].state = 0;
	}

	return channelIndex + 1;

}

int16 Screen::setAnimFrame(uint16 channelIndex, int16 frameNum) {
	if (channelIndex < 1 || channelIndex >= 100 || _channels[channelIndex - 1].type == 0)
		return 0;
	 channelIndex--;
	_channels[channelIndex].frameNum = frameNum;
	return updateChannel(channelIndex) + 1;
}

int16 Screen::getAnimFrame(uint16 channelIndex) {
	if (channelIndex < 1 || channelIndex >= 100 || _channels[channelIndex - 1].type == 0)
		return -1;
	return _channels[channelIndex - 1].frameNum;
}

uint16 Screen::placeText(uint16 channelIndex, uint16 textObjectIndex, int16 x, int16 y, uint16 fontNum, int16 textColor, int16 outlineColor) {

	if (channelIndex < 1 || channelIndex >= 100 || textObjectIndex == 0 || fontNum == 0)
		return 0;

	channelIndex--;

	Object *obj = _vm->_dat->getObject(textObjectIndex);
	const char *text = obj->getString();

	//int16 x1, y1, x2, y2;

	setFont(fontNum);

	int textWidth = _font->getTextWidth(text);
	int textHeight = _font->getHeight();

	if (outlineColor != -1) {
		textWidth += 2;
		textHeight += 2;
		x--;
		y--;
	}

	/*x1 = x;
	y1 = y;
	x2 = x + textWidth;
	y2 = y + textHeight;*/

	if (textWidth > 0 && outlineColor != -1) {
		x++;
		y++;
	}

	int16 state = 1;

	if (_ground == 0)
		state |= 2;

	_channels[channelIndex].state = state;
	_channels[channelIndex].type = 2;
	_channels[channelIndex].index = textObjectIndex;
	_channels[channelIndex].x = x;
	_channels[channelIndex].y = y;
	_channels[channelIndex].textColor = textColor;
	_channels[channelIndex].fontNum = fontNum;
	_channels[channelIndex].outlineColor = outlineColor;

	if (_channelsUsedCount <= channelIndex)
		_channelsUsedCount = channelIndex + 1;

	return channelIndex + 1;
}

void Screen::show() {

	if (_screenLock)
		return;

	drawSpriteChannels(_backgroundScreenDrawCtx, 3, 0);
	memcpy(_workScreen->pixels, _backgroundScreen->pixels, 64000);
	drawSpriteChannels(_workScreenDrawCtx, 1, 2);

	_fx->run(_visualEffectNum, _workScreen, _palette, _newPalette, _paletteColorCount);
	_visualEffectNum = 0;

	if (!_paletteInitialized) {
		memcpy(_newPalette, _palette, _paletteColorCount * 3);
		_oldPaletteColorCount = _paletteColorCount;
		_paletteInitialized = true;
	}

	updateScreenAndWait(10);
}

void Screen::flash(int flashCount) {
	_fx->flash(flashCount, _palette, _paletteColorCount);
}

void Screen::setFont(int16 fontNum) {
	if (fontNum == _currentFontNum)
		return;
	if (_font)
		_vm->_res->freeResource(_font);
	_font = _vm->_res->getFont(fontNum);
	_currentFontNum = fontNum;
}

void Screen::printChar(uint c, int16 x, int16 y, byte color) {

	if (!_font)
		return;

	uint width = 8, height = _font->getHeight();
	byte *charData = _font->getChar(c);

	if (!charData)
		return;

	byte p;
	byte *dest = (byte *)_fontDrawCtx.destSurface->getBasePtr(x, y);

	for (uint yc = 0; yc < height; yc++) {
		p = charData[yc];
		for (uint xc = 0; xc < width; xc++) {
			if (p & 0x80)
				dest[xc] = color;
			p <<= 1;
		}
		dest += _fontDrawCtx.destSurface->pitch;
	}

}

void Screen::printText(const char *text) {

	const int tabWidth = 5;

	if (!_font)
		return;

	int textLen = strlen(text);
	int textHeight = _font->getHeight();
	int linePos = 1;
	int16 x = _textX;
	int16 y = _textY;

	for (int textPos = 0; textPos < textLen; textPos++) {

		uint c = ((const byte*)text)[textPos];
		int charWidth = _font->getCharWidth(c);

		if (c == 9) {
			linePos = ((linePos / tabWidth) + 1) * tabWidth;
			x = _textRect.left + linePos * _font->getCharWidth(32);
		} else if (c == 10) {
			linePos = 1;
			x = _textRect.left;
			y += textHeight;
		} else if (c == 13) {
			linePos = 1;
			x = _textRect.left;
		} else if (c == 32) {
			int wrapPos = textPos + 1;
			int wrapX = x + charWidth;
			while (wrapPos < textLen && text[wrapPos] != 0 && text[wrapPos] != 32 && text[wrapPos] >= 28) {
				wrapX += _font->getCharWidth(text[wrapPos]);
				wrapPos++;
			}
			if (wrapX >= _textRect.right) {
				linePos = 1;
				x = _textRect.left;
				y += textHeight;
				charWidth = 0;
				// TODO: text[textPos] = '\x01';
			}
		}

		if (x + charWidth > _textRect.right) {
			linePos = 1;
			x = _textRect.left;
			y += textHeight;
		}

		if (y + textHeight > _textRect.bottom) {
			// TODO
		}

		if (c >= 28 && c <= 255) {
			if (_dropShadowColor != -1) {
				printChar(c, x + 1, y + 1, _dropShadowColor);
			}
			if (_outlineColor != -1) {
				printChar(c, x, y - 1, _outlineColor);
				printChar(c, x, y + 1, _outlineColor);
				printChar(c, x - 1, y, _outlineColor);
				printChar(c, x + 1, y, _outlineColor);
				printChar(c, x - 1, y - 1, _outlineColor);
				printChar(c, x - 1, y + 1, _outlineColor);
				printChar(c, x + 1, y - 1, _outlineColor);
				printChar(c, x + 1, y + 1, _outlineColor);
			}
			printChar(c, x, y, _textColor);
			x += charWidth;
			linePos++;
		}

	}

	_textX = x;
	_textY = y;

}

void Screen::printTextEx(const char *text, int16 x, int16 y, int16 fontNum, int16 textColor, int16 outlineColor, const ClipInfo &clipInfo) {
	if (*text == 0 || x < 0 || y < 0)
		return;

	int16 oldFontNum = _currentFontNum;
	Common::Rect oldTextRect;
	ClipInfo oldFontDrawCtx = _fontDrawCtx;

	_fontDrawCtx = clipInfo;

	getTextRect(oldTextRect);
	setFont(fontNum);
	setTextColor(textColor);
	setOutlineColor(outlineColor);
	setTextXY(x, y);
	printText(text);
	setTextRect(oldTextRect);
	setFont(oldFontNum);
	_fontDrawCtx = oldFontDrawCtx;

}

void Screen::printObjectText(int16 objectIndex, int16 x, int16 y, int16 fontNum, int16 textColor, int16 outlineColor, const ClipInfo &clipInfo) {

	if (objectIndex == 0)
		return;

	Object *obj = _vm->_dat->getObject(objectIndex);
	const char *text = obj->getString();

	printTextEx(text, x, y, fontNum, textColor, outlineColor, clipInfo);

}

int16 Screen::getTextWidth(int16 fontNum, const char *text) {
	setFont(fontNum);
	return _font->getTextWidth(text);
}

Graphics::Surface *Screen::lockScreen() {
	return _vm->_system->lockScreen();
}

void Screen::unlockScreen() {
	_vm->_system->unlockScreen();
}

void Screen::showWorkScreen() {
	_vm->_system->copyRectToScreen(_workScreen->pixels, _workScreen->pitch, 0, 0, _workScreen->w, _workScreen->h);
}

void Screen::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	_vm->_system->copyRectToScreen(buf, pitch, x, y, w, h);
}

void Screen::updateScreenAndWait(int delay) {
	_vm->_system->updateScreen();
	uint32 startTime = _vm->_system->getMillis();
	while (_vm->_system->getMillis() < startTime + delay) {
		_vm->handleEvents();
		_vm->_system->delayMillis(5);
	}
}

int16 Screen::addToSpriteList(int16 index, int16 xofs, int16 yofs) {
	SpriteListItem item;
	item.index = index;
	item.xofs = xofs;
	item.yofs = yofs;
	_spriteList.push_back(item);
	return _spriteList.size();
}

SpriteListItem Screen::getFromSpriteList(int16 index) {
	if (((uint) index) > _spriteList.size()) {
		SpriteListItem emptyItem;
		emptyItem.index = 0;
		emptyItem.xofs = 0;
		emptyItem.yofs = 0;
		return emptyItem;
	} else {
		return _spriteList[index - 1];
	}
}

void Screen::clearSpriteList() {
	_spriteList.clear();
}

void Screen::setDefaultMouseCursor() {
	CursorMan.replaceCursor(defaultMouseCursor, 16, 16, 9, 2, 0);
}

} // End of namespace Made
