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

#ifndef CINE_ANIM_H
#define CINE_ANIM_H

#include "common/endian.h"

#include "cine/saveload.h"

namespace Cine {

struct AnimHeaderStruct {
	byte field_0;
	byte field_1;
	byte field_2;
	byte field_3;
	uint16 frameWidth;
	uint16 frameHeight;
	byte field_8;
	byte field_9;
	byte field_A;
	byte field_B;
	byte field_C;
	byte field_D;
	uint16 numFrames;
	byte field_10;
	byte field_11;
	byte field_12;
	byte field_13;
	uint16 field_14;
};

struct AnimDataEntry {
	char name[9];
	byte color;
};

#define ANIM_RAW 0 // memcpy
#define ANIM_MASK 1 // convertMask
#define ANIM_SPRITE 2 // gfxConvertSpriteToRaw
#define ANIM_MASKSPRITE 3 // gfxConvertSpriteToRaw + generateMask
#define ANIM_PALSPRITE 5 // convert8BBP
#define ANIM_FULLSPRITE 8 // convert8BBP2

class AnimData {
private:
	byte *_data; ///< Image data
	byte *_mask; ///< Image mask (may be NULL)
	int16 _fileIdx; ///< Source file index in bundle
	int16 _frameIdx; ///< Frame number in animation
	char _name[10]; ///< Part filename
	int _size; ///< _data/_mask size, internal only

public:
	uint16 _width; ///< Image width (usually twice the real size)
	uint16 _height; ///< Image height
	uint16 _bpp; ///< Bit depth/type information
	uint16 _var1; ///< Something related to width
	int _realWidth; ///< Real image width in bytes

	AnimData();
	AnimData(const AnimData &src);
	~AnimData();

	AnimData &operator=(const AnimData &src);

	const byte *data() const { return _data; } ///< Image data
	const byte *mask() const { return _mask; } ///< Image mask (may be NULL)
	byte getColor(int x, int y);

	void load(byte *d, int type, uint16 w, uint16 h, int16 file, int16 frame, const char *n, byte transparent = 0);
	void clear();

	void save(Common::OutSaveFile &fHandle) const;
};

#define NUM_MAX_ANIMDATA 255

void freeAnimDataTable();
void freeAnimDataRange(byte startIdx, byte numIdx);
int loadResource(const char *resourceName, int16 idx = -1, int16 frameIndex = -1);
void loadResourcesFromSave(Common::SeekableReadStream &fHandle, enum CineSaveGameFormat saveGameFormat);
void generateMask(const byte *sprite, byte *mask, uint16 size, byte transparency);

} // End of namespace Cine

#endif
