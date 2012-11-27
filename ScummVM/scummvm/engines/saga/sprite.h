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

// Sprite management module private header file

#ifndef SAGA_SPRITE_H
#define SAGA_SPRITE_H

namespace Saga {

#define SPRITE_ZMAX  16
#define SPRITE_ZMASK 0x0F

struct SpriteInfo {
	ByteArray decodedBuffer;
	int width;
	int height;
	int xAlign;
	int yAlign;

	SpriteInfo() : width(0), height(0), xAlign(0), yAlign(0) {
	}
};

typedef Common::Array<SpriteInfo> SpriteList;

class Sprite {
public:
	SpriteList _mainSprites;
	SpriteList _saveReminderSprites;
	SpriteList _arrowSprites;
	SpriteList _inventorySprites;

	Sprite(SagaEngine *vm);
	~Sprite();

	// draw scaled sprite using background scene mask
	void drawOccluded(SpriteList &spriteList, uint spriteNumber, const Point &screenCoord, int scale, int depth);

	// draw scaled sprite using background scene mask
	void draw(SpriteList &spriteList, uint spriteNumber, const Point &screenCoord, int scale, bool clipToScene = false);

	// main function
	void drawClip(const Point &spritePointer, int width, int height, const byte *spriteBuffer, bool clipToScene = false);

	void draw(SpriteList &spriteList, uint spriteNumber, const Rect &screenRect, int scale, bool clipToScene = false);

	void loadList(int resourceId, SpriteList &spriteList); // load or append spriteList
	bool hitTest(SpriteList &spriteList, uint spriteNumber, const Point &screenCoord, int scale, const Point &testPoint);
	void getScaledSpriteBuffer(SpriteList &spriteList, uint spriteNumber, int scale, int &width, int &height, int &xAlign, int &yAlign, const byte *&buffer);

private:
	void decodeRLEBuffer(const byte *inputBuffer, size_t inLength, size_t outLength);
	void scaleBuffer(const byte *src, int width, int height, int scale, size_t outLength);

	SagaEngine *_vm;
	ResourceContext *_spriteContext;
	ByteArray _decodeBuf;
};

} // End of namespace Saga

#endif
