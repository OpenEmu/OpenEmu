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

// Sprite management module

#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/scene.h"
#include "saga/resource.h"
#include "saga/font.h"

#include "saga/sprite.h"
#include "saga/render.h"

namespace Saga {

#define RID_IHNM_ARROW_SPRITES 13
#define RID_IHNM_SAVEREMINDER_SPRITES 14
#define RID_IHNMDEMO_ARROW_SPRITES 8
#define RID_IHNMDEMO_SAVEREMINDER_SPRITES 9

Sprite::Sprite(SagaEngine *vm) : _vm(vm) {
	debug(8, "Initializing sprite subsystem...");

	// Load sprite module resource context
	_spriteContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (_spriteContext == NULL) {
		error("Sprite::Sprite resource context not found");
	}

	if (_vm->getGameId() == GID_ITE) {
		loadList(_vm->getResourceDescription()->mainSpritesResourceId, _mainSprites);
		_arrowSprites = _saveReminderSprites = _inventorySprites = _mainSprites;
#ifdef ENABLE_IHNM
	} else if (_vm->getGameId() == GID_IHNM) {
		if (_vm->isIHNMDemo()) {
			loadList(RID_IHNMDEMO_ARROW_SPRITES, _arrowSprites);
			loadList(RID_IHNMDEMO_SAVEREMINDER_SPRITES, _saveReminderSprites);
		} else {
			loadList(RID_IHNM_ARROW_SPRITES, _arrowSprites);
			loadList(RID_IHNM_SAVEREMINDER_SPRITES, _saveReminderSprites);
		}
#endif
	} else {
		error("Sprite: unknown game type");
	}
}

Sprite::~Sprite() {
	debug(8, "Shutting down sprite subsystem...");
}

void Sprite::loadList(int resourceId, SpriteList &spriteList) {
	SpriteInfo *spriteInfo;
	ByteArray spriteListData;
	uint16 oldSpriteCount;
	uint16 newSpriteCount;
	uint16 spriteCount;
	uint i;
	int outputLength, inputLength;
	uint32 offset;
	const byte *spritePointer;
	const byte *spriteDataPointer;

	_vm->_resource->loadResource(_spriteContext, resourceId, spriteListData);

	if (spriteListData.empty()) {
		return;
	}

	ByteArrayReadStreamEndian readS(spriteListData, _spriteContext->isBigEndian());

	spriteCount = readS.readUint16();

	debug(9, "Sprites: %d", spriteCount);

	oldSpriteCount = spriteList.size();
	newSpriteCount = oldSpriteCount + spriteCount;

	spriteList.resize(newSpriteCount);

	bool bigHeader = _vm->getGameId() == GID_IHNM || _vm->isMacResources();

	for (i = oldSpriteCount; i < spriteList.size(); i++) {
		spriteInfo = &spriteList[i];
		if (bigHeader)
			offset = readS.readUint32();
		else
			offset = readS.readUint16();

		if (offset >= spriteListData.size()) {
			// ITE Mac demos throw this warning
			warning("Sprite::loadList offset exceeded");
			spriteList.resize(i);
			return;
		}

		spritePointer = spriteListData.getBuffer();
		spritePointer += offset;

		if (bigHeader) {
			Common::MemoryReadStreamEndian readS2(spritePointer, 8, _spriteContext->isBigEndian());

			spriteInfo->xAlign = readS2.readSint16();
			spriteInfo->yAlign = readS2.readSint16();

			spriteInfo->width = readS2.readUint16();
			spriteInfo->height = readS2.readUint16();

			spriteDataPointer = spritePointer + readS2.pos();
		} else {
			Common::MemoryReadStreamEndian readS2(spritePointer, 4, false);

			spriteInfo->xAlign = readS2.readSByte();
			spriteInfo->yAlign = readS2.readSByte();

			spriteInfo->width = readS2.readByte();
			spriteInfo->height = readS2.readByte();
			spriteDataPointer = spritePointer + readS2.pos();
		}

		outputLength = spriteInfo->width * spriteInfo->height;
		inputLength = spriteListData.size() - (spriteDataPointer - spriteListData.getBuffer());
		spriteInfo->decodedBuffer.resize(outputLength);
		if (outputLength > 0) {
			decodeRLEBuffer(spriteDataPointer, inputLength, outputLength);
			byte *dst = &spriteInfo->decodedBuffer.front();
#ifdef ENABLE_IHNM
			// IHNM sprites are upside-down, for reasons which i can only
			// assume are perverse. To simplify things, flip them now. Not
			// at drawing time.

			if (_vm->getGameId() == GID_IHNM) {
				byte *src = &_decodeBuf[spriteInfo->width * (spriteInfo->height - 1)];

				for (int j = 0; j < spriteInfo->height; j++) {
					memcpy(dst, src, spriteInfo->width);
					src -= spriteInfo->width;
					dst += spriteInfo->width;
				}
			} else
#endif
				memcpy(dst, &_decodeBuf.front(), outputLength);
		}
	}
}

void Sprite::getScaledSpriteBuffer(SpriteList &spriteList, uint spriteNumber, int scale, int &width, int &height, int &xAlign, int &yAlign, const byte *&buffer) {
	SpriteInfo *spriteInfo;

	if (spriteList.size() <= spriteNumber) {
		// this can occur in IHNM while loading a saved game from chapter 1-5 when being in the end chapter
		warning("spriteList.size() <= spriteNumber");
		return;
	}

	spriteInfo = &spriteList[spriteNumber];

	if (scale < 256) {
		xAlign = (spriteInfo->xAlign * scale) >> 8; //TODO: do we need to take in account sprite x&y aligns ?
		yAlign = (spriteInfo->yAlign * scale) >> 8; // ????
		height = (spriteInfo->height * scale + 0x7f) >> 8;
		width = (spriteInfo->width * scale + 0x7f) >> 8;
		size_t outLength = width * height;
		if (outLength > 0) {
			scaleBuffer(&spriteInfo->decodedBuffer.front(), spriteInfo->width, spriteInfo->height, scale, outLength);
			buffer = &_decodeBuf.front();
		} else {
			buffer = NULL;
		}
	} else {
		xAlign = spriteInfo->xAlign;
		yAlign = spriteInfo->yAlign;
		height = spriteInfo->height;
		width = spriteInfo->width;
		buffer = spriteInfo->decodedBuffer.getBuffer();
	}
}

void Sprite::drawClip(const Point &spritePointer, int width, int height, const byte *spriteBuffer, bool clipToScene) {
	Common::Rect clipRect = clipToScene ? _vm->_scene->getSceneClip() : _vm->getDisplayClip();

	int xDstOffset, yDstOffset, xSrcOffset, ySrcOffset, xDiff, yDiff, cWidth, cHeight;
	byte *bufRowPointer;
	byte *bufPointer;
	const byte *srcRowPointer;
	const byte *srcPointer;

	int backBufferPitch = _vm->_gfx->getBackBufferPitch();

	//find Rects intersection
	yDiff = clipRect.top - spritePointer.y;
	if (yDiff > 0) {
		ySrcOffset = yDiff;
		yDstOffset = clipRect.top;
		cHeight = height - yDiff;
	} else {
		ySrcOffset = 0;
		yDstOffset = spritePointer.y;
		cHeight = height;
	}

	xDiff = clipRect.left - spritePointer.x;
	if (xDiff > 0) {
		xSrcOffset = xDiff;
		xDstOffset = clipRect.left;
		cWidth = width - xDiff;
	} else {
		xSrcOffset = 0;
		xDstOffset = spritePointer.x;
		cWidth = width;
	}

	yDiff = yDstOffset + cHeight - clipRect.bottom;
	if (yDiff > 0) {
		cHeight -= yDiff;
	}

	xDiff = xDstOffset + cWidth - clipRect.right;
	if (xDiff > 0) {
		cWidth -= xDiff;
	}

	if ((cHeight <= 0) || (cWidth <= 0)) {
		//no intersection
		return;
	}
	bufRowPointer = _vm->_gfx->getBackBufferPixels() + backBufferPitch * yDstOffset + xDstOffset;
	srcRowPointer = spriteBuffer + width * ySrcOffset + xSrcOffset;

	// validate src, dst buffers
	assert(_vm->_gfx->getBackBufferPixels() <= bufRowPointer);
	assert((_vm->_gfx->getBackBufferPixels() + (_vm->getDisplayInfo().width * _vm->getDisplayInfo().height)) >=
		(byte *)(bufRowPointer + backBufferPitch * (cHeight - 1) + cWidth));
	assert((const byte *)spriteBuffer <= srcRowPointer);
	assert(((const byte *)spriteBuffer + (width * height)) >= (const byte *)(srcRowPointer + width * (cHeight - 1) + cWidth));

	const byte *srcPointerFinish2 = srcRowPointer + width * cHeight;
	for (;;) {
		srcPointer = srcRowPointer;
		bufPointer = bufRowPointer;
		const byte *srcPointerFinish = srcRowPointer + cWidth;
		for (;;) {
			if (*srcPointer != 0) {
				*bufPointer = *srcPointer;
			}
			srcPointer++;
			bufPointer++;
			if (srcPointer == srcPointerFinish) {
				break;
			}
		}
		srcRowPointer += width;
		if (srcRowPointer == srcPointerFinish2) {
			break;
		}
		bufRowPointer += backBufferPitch;
	}

	_vm->_render->addDirtyRect(Common::Rect(xDstOffset, yDstOffset, xDstOffset + cWidth, yDstOffset + cHeight));
}

void Sprite::draw(SpriteList &spriteList, uint spriteNumber, const Point &screenCoord, int scale, bool clipToScene) {
	const byte *spriteBuffer = NULL;
	int width  = 0;
	int height = 0;
	int xAlign = 0;
	int yAlign = 0;
	Point spritePointer;

	getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);

	spritePointer.x = screenCoord.x + xAlign;
	spritePointer.y = screenCoord.y + yAlign;

	drawClip(spritePointer, width, height, spriteBuffer, clipToScene);
}

void Sprite::draw(SpriteList &spriteList, uint spriteNumber, const Rect &screenRect, int scale, bool clipToScene) {
	const byte *spriteBuffer = NULL;
	int width  = 0;
	int height = 0;
	int xAlign = 0;
	int spw;
	int yAlign = 0;
	int sph;
	Point spritePointer;

	getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);
	spw = (screenRect.width() - width) / 2;
	sph = (screenRect.height() - height) / 2;
	if (spw < 0) {
		spw = 0;
	}
	if (sph < 0) {
		sph = 0;
	}
	spritePointer.x = screenRect.left + xAlign + spw;
	spritePointer.y = screenRect.top + yAlign + sph;
	drawClip(spritePointer, width, height, spriteBuffer, clipToScene);
}

bool Sprite::hitTest(SpriteList &spriteList, uint spriteNumber, const Point &screenCoord, int scale, const Point &testPoint) {
	const byte *spriteBuffer = NULL;
	int i, j;
	const byte *srcRowPointer;
	int width  = 0;
	int height = 0;
	int xAlign = 0;
	int yAlign = 0;
	Point spritePointer;

	getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);

	spritePointer.x = screenCoord.x + xAlign;
	spritePointer.y = screenCoord.y + yAlign;

	if ((testPoint.y < spritePointer.y) || (testPoint.y >= spritePointer.y + height)) {
		return false;
	}
	if ((testPoint.x < spritePointer.x) || (testPoint.x >= spritePointer.x + width)) {
		return false;
	}
	i = testPoint.y - spritePointer.y;
	j = testPoint.x - spritePointer.x;
	srcRowPointer = spriteBuffer + j + i * width;
	return *srcRowPointer != 0;
}

void Sprite::drawOccluded(SpriteList &spriteList, uint spriteNumber, const Point &screenCoord, int scale, int depth) {
	const byte *spriteBuffer = NULL;
	int x, y;
	byte *destRowPointer;
	const byte *sourceRowPointer;
	const byte *sourcePointer;
	byte *destPointer;
	byte *maskPointer;
	int width  = 0;
	int height = 0;
	int xAlign = 0;
	int yAlign = 0;

	ClipData clipData;

	// BG mask variables
	int maskWidth;
	int maskHeight;
	byte *maskBuffer;
	byte *maskRowPointer;
	int maskZ;

	if (!_vm->_scene->isBGMaskPresent()) {
		draw(spriteList, spriteNumber, screenCoord, scale);
		return;
	}

	_vm->_scene->getBGMaskInfo(maskWidth, maskHeight, maskBuffer);

	getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);

	clipData.destPoint.x = screenCoord.x + xAlign;
	clipData.destPoint.y = screenCoord.y + yAlign;

	clipData.sourceRect.left = 0;
	clipData.sourceRect.top = 0;
	clipData.sourceRect.right = width;
	clipData.sourceRect.bottom = height;

	clipData.destRect = _vm->_scene->getSceneClip();

	if (!clipData.calcClip()) {
		return;
	}

	// Finally, draw the occluded sprite

	sourceRowPointer = spriteBuffer + clipData.drawSource.x + (clipData.drawSource.y * width);
	destRowPointer = _vm->_gfx->getBackBufferPixels() + clipData.drawDest.x + (clipData.drawDest.y * _vm->_gfx->getBackBufferPitch());
	maskRowPointer = maskBuffer + clipData.drawDest.x + (clipData.drawDest.y * maskWidth);

	for (y = 0; y < clipData.drawHeight; y++) {
		sourcePointer = sourceRowPointer;
		destPointer = destRowPointer;
		maskPointer = maskRowPointer;
		for (x = 0; x < clipData.drawWidth; x++) {
			if (*sourcePointer != 0) {
				maskZ = *maskPointer & SPRITE_ZMASK;
				if (maskZ > depth) {
					*destPointer = *sourcePointer;
				}
			}
			sourcePointer++;
			destPointer++;
			maskPointer++;
		}
		destRowPointer += _vm->_gfx->getBackBufferPitch();
		maskRowPointer += maskWidth;
		sourceRowPointer += width;
	}

	_vm->_render->addDirtyRect(Common::Rect(clipData.drawSource.x, clipData.drawSource.y,
								clipData.drawSource.x + width, clipData.drawSource.y + height));
}

void Sprite::decodeRLEBuffer(const byte *inputBuffer, size_t inLength, size_t outLength) {
	int bg_runcount;
	int fg_runcount;
	byte *outPointer;
	byte *outPointerEnd;
	int c;

	_decodeBuf.resize(outLength);
	outPointer = &_decodeBuf.front();
	outPointerEnd = &_decodeBuf.back();

	memset(outPointer, 0, _decodeBuf.size());

	Common::MemoryReadStream readS(inputBuffer, inLength);

	while (!readS.eos() && (outPointer < outPointerEnd)) {
		bg_runcount = readS.readByte();
		if (readS.eos())
			break;
		fg_runcount = readS.readByte();

		for (c = 0; c < bg_runcount && !readS.eos(); c++) {
			*outPointer = (byte) 0;
			if (outPointer < outPointerEnd)
				outPointer++;
			else
				return;
		}

		for (c = 0; c < fg_runcount && !readS.eos(); c++) {
			*outPointer = readS.readByte();
			if (readS.eos())
				break;
			if (outPointer < outPointerEnd)
				outPointer++;
			else
				return;
		}
	}
}

void Sprite::scaleBuffer(const byte *src, int width, int height, int scale, size_t outLength) {
	byte skip = 256 - scale; // skip factor
	byte vskip = 0x80, hskip;

	_decodeBuf.resize(outLength);
	byte *dst = &_decodeBuf.front();

	memset(dst, 0, _decodeBuf.size());

	for (int i = 0; i < height; i++) {
		vskip += skip;

		if (vskip < skip) { // We had an overflow
			src += width;
		} else {
			hskip = 0x80;

			for (int j = 0; j < width; j++) {
				*dst++ = *src++;

				hskip += skip;
				if (hskip < skip) // overflow
					dst--;
			}
		}
	}
}


} // End of namespace Saga
