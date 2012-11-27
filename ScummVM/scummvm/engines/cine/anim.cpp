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

/** \file
 * @todo Make resource manager class and make load* functions its members
 */

#include "common/endian.h"
#include "common/memstream.h"
#include "common/textconsole.h"

#include "cine/cine.h"
#include "cine/anim.h"
#include "cine/gfx.h"
#include "cine/pal.h"
#include "cine/part.h"
#include "cine/various.h"

namespace Cine {

struct AnimHeader2Struct {
	uint32 field_0;
	uint16 width;
	uint16 height;
	uint16 type;
	uint16 field_A;
	uint16 field_C;
	uint16 field_E;
};

static const AnimDataEntry transparencyData[] = {
	{"ALPHA", 0xF},
	{"TITRE2", 0xF},
	{"ET", 0xC},
	{"L311", 0x3},
	{"L405", 0x1},
	{"L515", 0xC},
	{"L009", 0xE},
	{"L010", 0xE},
	{"FUTUR", 0x6},
	{"PAYSAN3", 0xB},
	{"L801", 0xC},
	{"L802", 0xC},
	{"L803", 0xC},
	{"L901", 0xD},
	{"L902", 0x8},
	{"L903", 0xD},
	{"L904", 0xD},
	{"L905", 0xD},
	{"L906", 0xD},
	{"L907", 0xD},
	{"LA03", 0x4},
	{"MOINE", 0xB},
	{"L908", 0x8},
	{"L909", 0x8},
	{"L807", 0xC},
	{"L808", 0xC},
	{"LA01", 0xB},
	{"L1201", 0xC},
	{"L1202", 0xC},
	{"L1203", 0xC},
	{"L1210", 0x5},
	{"L1211", 0xC},
	{"L1214", 0xC},
	{"L1215", 0xC},
	{"L1216", 0xC},
	{"L1217", 0xC},
	{"L1218", 0xC},
	{"L1219", 0xC},
	{"L1220", 0xC},
	{"SEIGNEUR", 0x6},
	{"PERE0", 0xD},
	{"L1302", 0x4},
	{"L1303", 0x4},
	{"L1304", 0x4},
	{"L1401", 0xF},
	{"L1402", 0xF},
	{"L1501", 0x8},
	{"L1503", 0x8},
	{"L1504", 0x4},
	{"L1505", 0x8},
	{"L1506", 0x8},
	{"L1601", 0xB},
	{"L1602", 0xB},
	{"L1603", 0xB},
	{"L1604", 0x4},
	{"L1605", 0x4},
	{"L1701", 0x4},
	{"L1702", 0x4},
	{"L1801", 0x6},
	{"L1904", 0x8},
	{"L2002", 0x8},
	{"L2003", 0x8},
	{"L2101", 0x4},
	{"L2102", 0x4},
	{"L2201", 0x7},
	{"L2202", 0x7},
	{"L2203", 0xE},
	{"L2305", 0x9},
	{"L2306", 0x9},
	{"GARDE1", 0x7},
	{"L2402", 0x7},
	{"L2407", 0x7},
	{"L2408", 0x7},
	{"GARDE2", 0x6},
	{"L2601", 0x6},
	{"L2602", 0x6},
	{"L2603", 0x6},
	{"L2604", 0x6},
	{"L2605", 0x8},
	{"L2606", 0x8},
	{"L2607", 0x8},
	{"L2610", 0x6},
	{"L2611", 0x6},
	{"L2612", 0x6},
	{"L2613", 0x8},
	{"L2614", 0x6},
	{"VOYAGEUR", 0x6},
	{"L2701", 0xD},
	{"L2702", 0xD},
	{"L2703", 0x6},
	{"L2801", 0xD},
	{"L2802", 0xD},
	{"L2803", 0xD},
	{"L2804", 0xD},
	{"L2807", 0xD},
	{"L2902", 0x8},
	{"L2903", 0x8},
	{"L3101", 0xA},
	{"L3102", 0xA},
	{"L3103", 0xA},
	{"L3203", 0xF},
	{"L3204", 0xF},
	{"L3001", 0x7},
	{"L3002", 0x7},
	{"L3416", 0xC},
	{"L3601", 0x5},
	{"L3602", 0x5},
	{"L3603", 0x5},
	{"L3607", 0x5},
	{"L3701", 0x8},
	{"L3702", 0x8},
	{"L3703", 0x8},
	{"L4001", 0xD},
	{"L4002", 0xD},
	{"L4103", 0xF},
	{"L4106", 0xF},
	{"CRUGHON1", 0xC},
	{"L4203", 0xC},
	{"L4301", 0xC},
	{"L4302", 0xC},
	{"L4303", 0xC},
	{"FUTUR2", 0x6},
	{"L4601", 0xE},
	{"L4603", 0x1},
	{"L4106", 0xF},
	{"L4801", 0xD},
	{"L4802", 0xD},
	{"FIN01", 0xB},
	{"FIN02", 0xB},
	{"FIN03", 0xB},
	{"FIN", 0x9},
};

void convertMask(byte *dest, const byte *source, int16 width, int16 height);
void convert8BBP(byte *dest, const byte *source, int16 width, int16 height);
void convert8BBP2(byte *dest, byte *source, int16 width, int16 height);

AnimData::AnimData() : _width(0), _height(0), _bpp(0), _var1(0), _data(NULL),
	_mask(NULL), _fileIdx(-1), _frameIdx(-1), _realWidth(0), _size(0) {

	memset(_name, 0, sizeof(_name));
}

/**
 * Copy constructor
 */
AnimData::AnimData(const AnimData &src) : _width(src._width),
	_height(src._height), _bpp(src._bpp), _var1(src._var1),
	_data(NULL), _mask(NULL), _fileIdx(src._fileIdx),
	_frameIdx(src._frameIdx), _realWidth(src._realWidth), _size(src._size) {

	if (src._data) {
		_data = new byte[_size];
		assert(_data);
		memcpy(_data, src._data, _size * sizeof(byte));
	}

	if (src._mask) {
		_mask = new byte[_size];
		assert(_mask);
		memcpy(_mask, src._mask, _size * sizeof(byte));
	}

	memset(_name, 0, sizeof(_name));
	strcpy(_name, src._name);
}

/**
 * Destructor
 */
AnimData::~AnimData() {
	clear();
}

/**
 * Assingment operator
 */
AnimData &AnimData::operator=(const AnimData &src) {
	AnimData tmp = src;
	byte *ptr;

	_width = tmp._width;
	_height = tmp._height;
	_bpp = tmp._bpp;
	_var1 = tmp._var1;

	ptr = _data;
	_data = tmp._data;
	tmp._data = ptr;

	ptr = _mask;
	_mask = tmp._mask;
	tmp._mask = ptr;

	_fileIdx = tmp._fileIdx;
	_frameIdx = tmp._frameIdx;
	memset(_name, 0, sizeof(_name));
	strcpy(_name, tmp._name);
	_realWidth = tmp._realWidth;
	_size = tmp._size;

	return *this;
}

byte AnimData::getColor(int x, int y) {
	assert(_data);
	assert(x >= 0 && x < _realWidth && y >= 0 && y <= _height);
	assert(x + y * _realWidth < _size);

	return _data[x + y * _realWidth];
}

/**
 * Load and decode image frame
 * @param d Encoded image data
 * @param type Encoding type
 * @param w Image width
 * @param h Image height
 * @param file Data file index in bundle
 * @param frame Image frame index
 * @param n Part name
 * @param transparent Transparent color (for ANIM_MASKSPRITE)
 */
void AnimData::load(byte *d, int type, uint16 w, uint16 h, int16 file,
                    int16 frame, const char *n, byte transparent) {
	assert(d);

	if (_data) {
		clear();
	}

	_width = w * 2;
	_height = h;
	_var1 = _width >> 3;
	_data = NULL;
	_mask = NULL;
	_fileIdx = file;
	_frameIdx = frame;
	memset(_name, 0, sizeof(_name));
	strcpy(_name, n);
	_realWidth = w;

	switch (type) {
	case ANIM_RAW:
		_width = w;
		_var1 = w >> 3;
		_bpp = 4;
		_size = w * h;
		_data = new byte[_size];
		assert(_data);
		memcpy(_data, d, _size * sizeof(byte));
		break;

	case ANIM_MASK:
		_bpp = 1;
		_size = w * h * 8;
		_data = new byte[_size];
		_realWidth = w * 8;
		assert(_data);
		convertMask(_data, d, w, h);
		break;

	case ANIM_SPRITE:
		_bpp = 4;
		_size = w * h * 2;
		_data = new byte[_size];
		_realWidth = w * 2;
		assert(_data);
		gfxConvertSpriteToRaw(_data, d, w, h);
		break;

	case ANIM_MASKSPRITE:
		_bpp = 4;
		_size = w * h * 2;
		_data = new byte[_size];
		_mask = new byte[_size];
		_realWidth = w * 2;
		assert(_data && _mask);
		gfxConvertSpriteToRaw(_data, d, w, h);
		generateMask(_data, _mask, _size, transparent);
		break;

	case ANIM_PALSPRITE:
		_bpp = 5;
		_size = w * h * 2;
		_data = new byte[_size];
		_realWidth = w * 2;
		assert(_data);
		convert8BBP(_data, d, w, h);
		break;

	case ANIM_FULLSPRITE:
		_bpp = 8;
		_var1 = _width >> 4;
		_size = w * h;
		_data = new byte[_size];
		assert(_data);
		convert8BBP2(_data, d, w, h);
		break;

	default:
		error("AnimData::load: unknown image type");
	}
}

/**
 * Reset image
 */
void AnimData::clear() {
	delete[] _data;
	delete[] _mask;

	_width = 0;
	_height = 0;
	_bpp = 0;
	_var1 = 0;
	_data = NULL;
	_mask = NULL;
	_fileIdx = -1;
	_frameIdx = -1;
	memset(_name, 0, sizeof(_name));
	_size = 0;
}

/**
 * Write image identifiers to savefile
 * @param fHandle Savefile open for writing
 */
void AnimData::save(Common::OutSaveFile &fHandle) const {
	fHandle.writeUint16BE(_width);
	fHandle.writeUint16BE(_var1);
	fHandle.writeUint16BE(_bpp);
	fHandle.writeUint16BE(_height);
	fHandle.writeUint32BE(_data != NULL); // _data
	fHandle.writeUint32BE(_mask != NULL); // _mask
	fHandle.writeUint16BE(_fileIdx);
	fHandle.writeUint16BE(_frameIdx);
	fHandle.write(_name, sizeof(_name));
}

/**
 * Clear part of animDataTable
 * @param startIdx First image frame to be cleared
 * @param numIdx Number of image frames to be cleared
 */
void freeAnimDataRange(byte startIdx, byte numIdx) {
	for (byte i = 0; i < numIdx; i++) {
		g_cine->_animDataTable[startIdx + i].clear();
	}
}

/**
 * Clear whole animDataTable
 */
void freeAnimDataTable() {
	freeAnimDataRange(0, NUM_MAX_ANIMDATA);
}

/**
 * Find transparent color index for image
 * @param animName Image file name
 */
static byte getAnimTransparentColor(const char *animName) {
	char name[15];

	removeExtention(name, animName);

	for (int i = 0; i < ARRAYSIZE(transparencyData); i++) {
		if (!strcmp(name, transparencyData[i].name)) {
			return transparencyData[i].color;
		}
	}
	return 0;
}

/**
 * Generate mask for image
 * @param[in] sprite Image data
 * @param[out] mask Image mask
 * @param size Image data length
 * @param transparency Transparent color index
 */
void generateMask(const byte *sprite, byte *mask, uint16 size, byte transparency) {
	for (uint16 i = 0; i < size; i++) {
		if (*(sprite++) != transparency) {
			*(mask++) = 0;
		} else {
			*(mask++) = 1;
		}
	}
}

/**
 * Decode 1bpp mask
 * @param[out] dest Decoded mask
 * @param[in] source Encoded mask
 * @param width Mask width
 * @param height Mask height
 */
void convertMask(byte *dest, const byte *source, int16 width, int16 height) {
	int16 i, j;
	byte maskEntry;

	for (i = 0; i < width * height; i++) {
		maskEntry = *(source++);
		for (j = 0; j < 8; j++) {
			*(dest++) = (maskEntry & 0x80) ? 0 : 1;
			maskEntry <<= 1;
		}
	}
}

/**
 * Decode 4bpp sprite
 * @param[out] dest Decoded image
 * @param[in] source Encoded image
 * @param width Image width
 * @param height Image height
 */
void convert4BBP(byte *dest, const byte *source, int16 width, int16 height) {
	byte maskEntry;

	for (int16 i = 0; i < width * height; i++) {
		maskEntry = *(source++);
		*(dest++) = (maskEntry & 0xF0) >> 4;
		*(dest++) = (maskEntry & 0xF);
	}
}

/**
 * Read image header
 * @param[out] animHeader Image header reference
 * @param readS Input stream open for reading
 */
void loadAnimHeader(AnimHeaderStruct &animHeader, Common::SeekableReadStream &readS) {
	animHeader.field_0 = readS.readByte();
	animHeader.field_1 = readS.readByte();
	animHeader.field_2 = readS.readByte();
	animHeader.field_3 = readS.readByte();
	animHeader.frameWidth = readS.readUint16BE();
	animHeader.frameHeight = readS.readUint16BE();
	animHeader.field_8 = readS.readByte();
	animHeader.field_9 = readS.readByte();
	animHeader.field_A = readS.readByte();
	animHeader.field_B = readS.readByte();
	animHeader.field_C = readS.readByte();
	animHeader.field_D = readS.readByte();
	animHeader.numFrames = readS.readUint16BE();
	animHeader.field_10 = readS.readByte();
	animHeader.field_11 = readS.readByte();
	animHeader.field_12 = readS.readByte();
	animHeader.field_13 = readS.readByte();
	animHeader.field_14 = readS.readUint16BE();
}

/**
 * Find next empty space animDataTable
 * @param start First index to check
 */
int emptyAnimSpace(int start = 0) {
	for (; start < NUM_MAX_ANIMDATA; start++) {
		if (!g_cine->_animDataTable[start].data()) {
			return start;
		}
	}

	return -1;
}

/**
 * Load SPL data into animDataTable
 * @param resourceName SPL filename
 * @param idx Target index in animDataTable (-1 if any empty space will do)
 * @return The number of the animDataTable entry after the loaded SPL data (-1 if error)
 */
int loadSpl(const char *resourceName, int16 idx) {
	int16 foundFileIdx = findFileInBundle(resourceName);
	int entry;

	if (foundFileIdx < 0) {
		return -1;
	}

	byte *dataPtr = readBundleFile(foundFileIdx);

	entry = idx < 0 ? emptyAnimSpace() : idx;
	assert(entry >= 0);
	g_cine->_animDataTable[entry].load(dataPtr + 0x16, ANIM_RAW, g_cine->_partBuffer[foundFileIdx].unpackedSize - 0x16, 1, foundFileIdx, 0, currentPartName);

	free(dataPtr);
	return entry + 1;
}

/**
 * Load 1bpp mask
 * @param resourceName Mask filename
 * @param idx Target index in animDataTable (-1 if any empty space will do)
 * @param frameIndex frame of animation to load (-1 for all frames)
 * @return The number of the animDataTable entry after the loaded mask (-1 if error)
 */
int loadMsk(const char *resourceName, int16 idx, int16 frameIndex) {
	int16 foundFileIdx = findFileInBundle(resourceName);
	if (foundFileIdx < 0) {
		return -1;
	}

	int entry = 0;
	byte *dataPtr = readBundleFile(foundFileIdx);
	byte *ptr;
	AnimHeaderStruct animHeader;

	Common::MemoryReadStream readS(dataPtr, 0x16);
	loadAnimHeader(animHeader, readS);
	ptr = dataPtr + 0x16;

	int16 startFrame = 0;
	int16 endFrame = animHeader.numFrames;

	if (frameIndex >= 0) {
		startFrame = frameIndex;
		endFrame = frameIndex + 1;
		ptr += frameIndex * animHeader.frameWidth * animHeader.frameHeight;
	}

	entry = idx < 0 ? emptyAnimSpace() : idx;
	assert(entry >= 0);
	for (int16 i = startFrame; i < endFrame; i++, entry++) {
		g_cine->_animDataTable[entry].load(ptr, ANIM_MASK, animHeader.frameWidth, animHeader.frameHeight, foundFileIdx, i, currentPartName);
		ptr += animHeader.frameWidth * animHeader.frameHeight;
	}

	free(dataPtr);
	return entry;
}

/**
 * Load animation
 * @param resourceName Animation filename
 * @param idx Target index in animDataTable (-1 if any empty space will do)
 * @param frameIndex frame of animation to load (-1 for all frames)
 * @return The number of the animDataTable entry after the loaded animation (-1 if error)
 */
int loadAni(const char *resourceName, int16 idx, int16 frameIndex) {
	int16 foundFileIdx = findFileInBundle(resourceName);
	if (foundFileIdx < 0) {
		return -1;
	}

	int entry = 0;
	byte *dataPtr = readBundleFile(foundFileIdx);
	byte *ptr;
	byte transparentColor;
	AnimHeaderStruct animHeader;

	Common::MemoryReadStream readS(dataPtr, 0x16);
	loadAnimHeader(animHeader, readS);
	ptr = dataPtr + 0x16;

	int16 startFrame = 0;
	int16 endFrame = animHeader.numFrames;

	if (frameIndex >= 0) {
		startFrame = frameIndex;
		endFrame = frameIndex + 1;
		ptr += frameIndex * animHeader.frameWidth * animHeader.frameHeight;
	}

	transparentColor = getAnimTransparentColor(resourceName);

	// TODO: Merge this special case hack into getAnimTransparentColor somehow.
	// HACK: Versions of TITRE.ANI with height 37 use color 0xF for transparency.
	//       Versions of TITRE.ANI with height 57 use color 0x0 for transparency.
	//       Fixes bug #2057619: FW: Glitches in title display of demo (regression).
	if (scumm_stricmp(resourceName, "TITRE.ANI") == 0 && animHeader.frameHeight == 37) {
		transparentColor = 0xF;
	}

	entry = idx < 0 ? emptyAnimSpace() : idx;
	assert(entry >= 0);

	for (int16 i = startFrame; i < endFrame; i++, entry++) {
		// special case transparency handling
		if (!strcmp(resourceName, "L2202.ANI")) {
			transparentColor = i < 2 ? 0 : 7;
		} else if (!strcmp(resourceName, "L4601.ANI")) {
			transparentColor = i < 1 ? 0xE : 0;
		}

		g_cine->_animDataTable[entry].load(ptr, ANIM_MASKSPRITE, animHeader.frameWidth, animHeader.frameHeight, foundFileIdx, i, currentPartName, transparentColor);
		ptr += animHeader.frameWidth * animHeader.frameHeight;
	}

	free(dataPtr);
	return entry;
}

/**
 * Decode 16 color image with palette
 * @param[out] dest Decoded image
 * @param[in] source Encoded image
 * @param width Image width
 * @param height Image height
 */
void convert8BBP(byte *dest, const byte *source, int16 width, int16 height) {
	const byte *table = source;
	byte color;

	source += 16;

	for (uint16 i = 0; i < width * height; i++) {
		color = *(source++);

		*(dest++) = table[color >> 4];
		*(dest++) = table[color & 0xF];
	}
}

/**
 * Decode 8bit image
 * @param[out] dest Decoded image
 * @param[in] source Encoded image
 * @param width Image width
 * @param height Image height
 * \attention Data in source are destroyed during decoding
 */
void convert8BBP2(byte *dest, byte *source, int16 width, int16 height) {
	uint16 i, j;
	int k, m;
	byte color;

	for (j = 0; j < (width * height) / 16; j++) {
		// m = 0: even bits, m = 1: odd bits
		for (m = 0; m <= 1; m++) {
			for (i = 0; i < 8; i++) {
				color = 0;
				for (k = 14 + m; k >= 0; k -= 2) {
					color |= ((*(source + k) & 0x080) >> 7);
					*(source + k) <<= 1;
					if (k > 0 + m)
						color <<= 1;
				} // end k
				*(dest++) = color;
			} // end i
		} // end m

		source += 0x10;
	} // end j
}

/**
 * Load image set
 * @param resourceName Image set filename
 * @param idx Target index in animDataTable (-1 if any empty space will do)
 * @param frameIndex frame of animation to load (-1 for all frames)
 * @return The number of the animDataTable entry after the loaded image set (-1 if error)
 */
int loadSet(const char *resourceName, int16 idx, int16 frameIndex = -1) {
	AnimHeader2Struct header2;
	uint16 numSpriteInAnim;
	int16 foundFileIdx = findFileInBundle(resourceName);
	int16 entry;
	byte *ptr, *startOfDataPtr, *dataPtr, *origDataPtr;
	int type;

	if (foundFileIdx < 0) {
		return -1;
	}

	origDataPtr = dataPtr = readBundleFile(foundFileIdx);
	assert(!memcmp(dataPtr, "SET", 3));
	ptr = dataPtr + 4;

	numSpriteInAnim = READ_BE_UINT16(ptr);
	ptr += 2;

	startOfDataPtr = ptr + numSpriteInAnim * 0x10;

	entry = idx < 0 ? emptyAnimSpace() : idx;
	assert(entry >= 0);

	int16 startFrame = 0;
	int16 endFrame = numSpriteInAnim;

	if (frameIndex >= 0) {
		startFrame = frameIndex;
		endFrame = frameIndex + 1;
		ptr += 0x10 * frameIndex;
	}

	for (int16 i = startFrame; i < endFrame; i++, entry++) {
		Common::MemoryReadStream readS(ptr, 0x10);

		header2.field_0 = readS.readUint32BE();
		header2.width = readS.readUint16BE();
		header2.height = readS.readUint16BE();
		header2.type = readS.readUint16BE();
		header2.field_A = readS.readUint16BE();
		header2.field_C = readS.readUint16BE();
		header2.field_E = readS.readUint16BE();

		ptr += 0x10;

		dataPtr = startOfDataPtr + header2.field_0;

		if (header2.type == 1) {
			type = ANIM_MASK;
		} else if (header2.type == 4) {
			type = ANIM_SPRITE;
		} else if (header2.type == 5) {
			type = ANIM_PALSPRITE;
		} else {
			type = ANIM_FULLSPRITE;
		}

		g_cine->_animDataTable[entry].load(dataPtr, type, header2.width, header2.height, foundFileIdx, i, currentPartName);
	}

	free(origDataPtr);
	return entry;
}

/**
 * Load SEQ data into animDataTable
 * @param resourceName SEQ data filename
 * @param idx Target index in animDataTable (-1 if any empty space will do)
 * @return The number of the animDataTable entry after the loaded SEQ data (-1 if error)
 */
int loadSeq(const char *resourceName, int16 idx) {
	int16 foundFileIdx = findFileInBundle(resourceName);
	if (foundFileIdx < 0) {
		return -1;
	}

	byte *dataPtr = readBundleFile(foundFileIdx);
	int entry = idx < 0 ? emptyAnimSpace() : idx;

	g_cine->_animDataTable[entry].load(dataPtr + 0x16, ANIM_RAW, g_cine->_partBuffer[foundFileIdx].unpackedSize - 0x16, 1, foundFileIdx, 0, currentPartName);
	free(dataPtr);
	return entry + 1;
}

/**
 * Load a resource into animDataTable
 * @param resourceName Resource's filename
 * @param idx Target index in animDataTable (-1 if any empty space will do)
 * @return The number of the animDataTable entry after the loaded resource (-1 if error)
 * @todo Implement loading of all resource types
 */
int loadResource(const char *resourceName, int16 idx, int16 frameIndex) {
	int result = -1; // Return an error by default
	if (strstr(resourceName, ".SPL")) {
		result = loadSpl(resourceName, idx);
	} else if (strstr(resourceName, ".MSK")) {
		result = loadMsk(resourceName, idx, frameIndex);
	} else if (strstr(resourceName, ".ANI")) {
		result = loadAni(resourceName, idx, frameIndex);
	} else if (strstr(resourceName, ".ANM")) {
		result = loadAni(resourceName, idx, frameIndex);
	} else if (strstr(resourceName, ".SET")) {
		result = loadSet(resourceName, idx, frameIndex);
	} else if (strstr(resourceName, ".SEQ")) {
		result = loadSeq(resourceName, idx);
	} else if (strstr(resourceName, ".H32")) {
		warning("loadResource: Ignoring file '%s' (Load at %d)", resourceName, idx);
	} else if (strstr(resourceName, ".AMI")) {
		warning("loadResource: Ignoring file '%s' (Load at %d)", resourceName, idx);
	} else if (strstr(resourceName, "ECHEC")) { // Echec (French) means failure
		g_cine->quitGame();
	} else {
		error("loadResource: Cannot determine type for '%s'", resourceName);
	}

	return result;
}

} // End of namespace Cine
