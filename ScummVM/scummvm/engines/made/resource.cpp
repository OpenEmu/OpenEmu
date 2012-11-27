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

#include "made/resource.h"
#include "made/graphics.h"

#include "common/file.h"
#include "common/memstream.h"
#include "common/debug.h"

#include "graphics/surface.h"

#include "audio/decoders/raw.h"
#include "audio/audiostream.h"

namespace Made {

/* Resource */

Resource::~Resource() {
}

/* PictureResource */

PictureResource::PictureResource() : _picture(NULL), _picturePalette(NULL) {
	_hasPalette = false;
}

PictureResource::~PictureResource() {
	if (_picture) {
		_picture->free();
		delete _picture;
		_picture = 0;
	}

	delete[] _picturePalette;
	_picturePalette = 0;
}

void PictureResource::load(byte *source, int size) {
	if (READ_BE_UINT32(source) == MKTAG('F','l','e','x')) {
		loadChunked(source, size);
	} else {
		loadRaw(source, size);
	}
}

void PictureResource::loadRaw(byte *source, int size) {
	// Loads a "raw" picture as used in RtZ, LGoP2, Manhole:N&E and Rodney's Funscreen

	Common::MemoryReadStream *sourceS = new Common::MemoryReadStream(source, size);

	_hasPalette = (sourceS->readByte() != 0);
	byte cmdFlags = sourceS->readByte();
	byte pixelFlags = sourceS->readByte();
	byte maskFlags = sourceS->readByte();
	uint16 cmdOffs = sourceS->readUint16LE();
	uint16 pixelOffs = sourceS->readUint16LE();
	uint16 maskOffs = sourceS->readUint16LE();
	uint16 lineSize = sourceS->readUint16LE();
	/*uint16 u = */sourceS->readUint16LE();
	uint16 width = sourceS->readUint16LE();
	uint16 height = sourceS->readUint16LE();

	if (cmdFlags || pixelFlags || maskFlags) {
		warning("PictureResource::loadRaw() Graphic has flags set (%d, %d, %d)", cmdFlags, pixelFlags, maskFlags);
	}

	_paletteColorCount = (cmdOffs - 18) / 3; // 18 = sizeof header

	debug(2, "width = %d; height = %d\n", width, height);

	if (_hasPalette) {
		_picturePalette = new byte[_paletteColorCount * 3];
		sourceS->read(_picturePalette, _paletteColorCount * 3);
	}

	_picture = new Graphics::Surface();
	_picture->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	decompressImage(source, *_picture, cmdOffs, pixelOffs, maskOffs, lineSize, cmdFlags, pixelFlags, maskFlags);

	delete sourceS;

}

void PictureResource::loadChunked(byte *source, int size) {
	// Loads a "chunked" picture as used in Manhole EGA

	Common::MemoryReadStream *sourceS = new Common::MemoryReadStream(source, size);

	byte cmdFlags = 0, pixelFlags = 0, maskFlags = 0;
	uint16 cmdOffs = 0, pixelOffs = 0, maskOffs = 0;
	uint16 lineSize = 0, width = 0, height = 0;

	sourceS->skip(36); // skip the "Flex" header

	_hasPalette = false;

	while (!sourceS->eos()) {

		uint32 chunkType = sourceS->readUint32BE();
		uint32 chunkSize = sourceS->readUint32BE();

		if (sourceS->eos())
			break;

		debug(0, "chunkType = %08X; chunkSize = %d", chunkType, chunkSize);

		if (chunkType == MKTAG('R','e','c','t')) {
			debug(0, "Rect");
			sourceS->skip(4);
			height = sourceS->readUint16BE();
			width = sourceS->readUint16BE();
			debug(0, "width = %d; height = %d", width, height);
		} else if (chunkType == MKTAG('f','M','a','p')) {
			debug(0, "fMap");
			lineSize = sourceS->readUint16BE();
			sourceS->skip(11);
			cmdFlags = sourceS->readByte();
			cmdOffs = sourceS->pos();
			sourceS->skip(chunkSize - 14);
			debug(0, "lineSize = %d; cmdFlags = %d; cmdOffs = %04X", lineSize, cmdFlags, cmdOffs);
		} else if (chunkType == MKTAG('f','L','C','o')) {
			debug(0, "fLCo");
			sourceS->skip(9);
			pixelFlags = sourceS->readByte();
			pixelOffs = sourceS->pos();
			sourceS->skip(chunkSize - 10);
			debug(0, "pixelFlags = %d; pixelOffs = %04X", pixelFlags, pixelOffs);
		} else if (chunkType == MKTAG('f','P','i','x')) {
			debug(0, "fPix");
			sourceS->skip(9);
			maskFlags = sourceS->readByte();
			maskOffs = sourceS->pos();
			sourceS->skip(chunkSize - 10);
			debug(0, "maskFlags = %d; maskOffs = %04X", maskFlags, maskOffs);
		} else if (chunkType == MKTAG('f','G','C','o')) {
			debug(0, "fGCo");
			_hasPalette = true;
			_paletteColorCount = chunkSize / 3;
			_picturePalette = new byte[_paletteColorCount * 3];
			sourceS->read(_picturePalette, _paletteColorCount * 3);
		} else {
			error("PictureResource::loadChunked() Invalid chunk %08X at %08X", chunkType, sourceS->pos());
		}

	}

	if (!cmdOffs || !pixelOffs /*|| !maskOffs*/ || !lineSize || !width || !height) {
		error("PictureResource::loadChunked() Error parsing the picture data, one or more chunks/parameters are missing");
	}

	_picture = new Graphics::Surface();
	_picture->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	decompressImage(source, *_picture, cmdOffs, pixelOffs, maskOffs, lineSize, cmdFlags, pixelFlags, maskFlags);

	delete sourceS;

}

/* AnimationResource */

AnimationResource::AnimationResource() {
}

AnimationResource::~AnimationResource() {
	for (uint i = 0; i < _frames.size(); i++) {
		_frames[i]->free();
		delete _frames[i];
	}
}

void AnimationResource::load(byte *source, int size) {
	Common::MemoryReadStream *sourceS = new Common::MemoryReadStream(source, size);

	sourceS->readUint32LE();
	sourceS->readUint32LE();
	sourceS->readUint16LE();

	_flags = sourceS->readUint16LE();
	_width = sourceS->readUint16LE();
	_height = sourceS->readUint16LE();
	sourceS->readUint32LE();
	uint16 frameCount = sourceS->readUint16LE();
	sourceS->readUint16LE();
	sourceS->readUint16LE();

	for (uint16 i = 0; i < frameCount; i++) {

		sourceS->seek(26 + i * 4);

		uint32 frameOffs = sourceS->readUint32LE();

		sourceS->seek(frameOffs);
		sourceS->readUint32LE();
		sourceS->readUint32LE();

		uint16 frameWidth = sourceS->readUint16LE();
		uint16 frameHeight = sourceS->readUint16LE();
		uint16 cmdOffs = sourceS->readUint16LE();
		sourceS->readUint16LE();
		uint16 pixelOffs = sourceS->readUint16LE();
		sourceS->readUint16LE();
		uint16 maskOffs = sourceS->readUint16LE();
		sourceS->readUint16LE();
		uint16 lineSize = sourceS->readUint16LE();

		Graphics::Surface *frame = new Graphics::Surface();
		frame->create(frameWidth, frameHeight, Graphics::PixelFormat::createFormatCLUT8());

		decompressImage(source + frameOffs, *frame, cmdOffs, pixelOffs, maskOffs, lineSize, 0, 0, 0, _flags & 1);

		_frames.push_back(frame);

	}

	delete sourceS;
}

/* SoundResource */

SoundResource::SoundResource() : _soundSize(0), _soundData(NULL) {
}

SoundResource::~SoundResource() {
	delete[] _soundData;
	delete _soundEnergyArray;
}

void SoundResource::load(byte *source, int size) {
	uint16 chunkCount = READ_LE_UINT16(source + 8);
	uint16 chunkSize = READ_LE_UINT16(source + 12);

	_soundSize = chunkCount * chunkSize;
	_soundData = new byte[_soundSize];

	_soundEnergyArray = new SoundEnergyArray;

	decompressSound(source + 14, _soundData, chunkSize, chunkCount, _soundEnergyArray);
}

Audio::AudioStream *SoundResource::getAudioStream(int soundRate, bool loop) {
	Audio::RewindableAudioStream *stream =
			Audio::makeRawStream(_soundData, _soundSize, soundRate, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);

	if (loop)
		return Audio::makeLoopingAudioStream(stream, 0);
	else
		return stream;
}

void SoundResourceV1::load(byte *source, int size) {
	_soundSize = size * 4;
	_soundData = new byte[_soundSize];
	ManholeEgaSoundDecompressor dec;
	dec.decompress(source, _soundData, size);
}

/* MenuResource */

MenuResource::MenuResource() {
}

MenuResource::~MenuResource() {
}

void MenuResource::load(byte *source, int size) {
	_strings.clear();
	Common::MemoryReadStream *sourceS = new Common::MemoryReadStream(source, size);
	sourceS->skip(4); // skip "MENU"
	uint16 count = sourceS->readUint16LE();
	for (uint16 i = 0; i < count; i++) {
		uint16 offs = sourceS->readUint16LE();
		const char *string = (const char*)(source + offs);
		_strings.push_back(string);
		debug(2, "%02d: %s\n", i, string);
	}
	delete sourceS;
}

const char *MenuResource::getString(uint index) const {
	if (index < _strings.size())
		return _strings[index].c_str();
	else
		return NULL;
}

/* FontResource */

FontResource::FontResource() : _data(NULL), _size(0) {
}

FontResource::~FontResource() {
	delete[] _data;
}

void FontResource::load(byte *source, int size) {
	_data = new byte[size];
	_size = size;
	memcpy(_data, source, size);
}

int FontResource::getHeight() const {
	return _data[0];
}

int FontResource::getCharWidth(uint c) const {
	byte *charData = getCharData(c);
	if (charData)
		return charData[0];
	else
		return 0;
}

byte *FontResource::getChar(uint c) const {
	byte *charData = getCharData(c);
	if (charData)
		return charData + 1;
	else
		return NULL;
}

int FontResource::getTextWidth(const char *text) {
	int width = 0;
	if (text) {
		int len = strlen(text);
		for (int pos = 0; pos < len; pos++)
			width += getCharWidth(text[pos]);
	}
	return width;
}

byte *FontResource::getCharData(uint c) const {
	if (c < 28 || c > 255)
		return NULL;
	return _data + 1 + (c - 28) * (getHeight() + 1);
}

/* GenericResource */

GenericResource::GenericResource() : _data(NULL), _size(0) {
}

GenericResource::~GenericResource() {
	delete[] _data;
}

void GenericResource::load(byte *source, int size) {
	_data = new byte[size];
	_size = size;
	memcpy(_data, source, size);
}

/* ResourceReader */

ResourceReader::ResourceReader() {
	_isV1 = false;
	_cacheDataSize = 0;
}

ResourceReader::~ResourceReader() {
	if (!_isV1) {
		delete _fd;
	} else {
		delete _fdPics;
		delete _fdSounds;
		delete _fdMusic;
	}
}

// V2
void ResourceReader::open(const char *filename) {
	_fd = new Common::File();
	_fd->open(filename);

	_fd->skip(0x18); // skip header for now

	uint16 indexCount = _fd->readUint16LE();

	for (uint16 i = 0; i < indexCount; i++) {

		uint32 resType = _fd->readUint32BE();
		uint32 indexOffs = _fd->readUint32LE();
		_fd->readUint32LE();
		_fd->readUint32LE();
		_fd->readUint32LE();
		_fd->readUint16LE();
		_fd->readUint16LE();

		// We don't need ARCH, FREE and OMNI resources
		if (resType == kResARCH || resType == kResFREE || resType == kResOMNI)
			continue;

		//debug(2, "resType = %08X; indexOffs = %d\n", resType, indexOffs);

		uint32 oldOffs = _fd->pos();

		ResourceSlots *resSlots = new ResourceSlots();
		_fd->seek(indexOffs);
		loadIndex(resSlots);
		_resSlots[resType] = resSlots;

		_fd->seek(oldOffs);

	}

	_cacheCount = 0;
}

// V1
void ResourceReader::openResourceBlocks() {
	_isV1 = true;
	_fdPics = new Common::File();
	_fdSounds = new Common::File();
	_fdMusic = new Common::File();

	openResourceBlock("pics.blk", _fdPics, kResFLEX);
	openResourceBlock("snds.blk", _fdSounds, kResSNDS);
	openResourceBlock("music.blk", _fdMusic, kResMIDI);
}

void ResourceReader::openResourceBlock(const char *filename, Common::File *blockFile, uint32 resType) {
	blockFile->open(filename);

	blockFile->readUint16LE(); // Skip unused
	uint16 count = blockFile->readUint16LE();
	blockFile->readUint16LE(); // Skip unused
	uint32 type = blockFile->readUint32BE();
	if (type != kResFLEX)
		warning("openResourceBlocks: resource header is not 'FLEX'");

	_resSlots[resType] = new ResourceSlots();

	// Add dummy entry since the resources are 1-based
	_resSlots[resType]->push_back(ResourceSlot(0, 0));

	for (uint16 i = 0; i < count; i++) {
		uint32 offset = blockFile->readUint32LE();
		blockFile->readUint32LE();
		uint32 size = blockFile->readUint32LE();
		_resSlots[resType]->push_back(ResourceSlot(offset, size));
	}
}

PictureResource *ResourceReader::getPicture(int index) {
	return createResource<PictureResource>(kResFLEX, index);
}

AnimationResource *ResourceReader::getAnimation(int index) {
	return createResource<AnimationResource>(kResANIM, index);
}

SoundResource *ResourceReader::getSound(int index) {
	if (!_isV1)
		return createResource<SoundResource>(kResSNDS, index);
	else
		return createResource<SoundResourceV1>(kResSNDS, index);
}

MenuResource *ResourceReader::getMenu(int index) {
	return createResource<MenuResource>(kResMENU, index);
}

FontResource *ResourceReader::getFont(int index) {
	return createResource<FontResource>(kResFONT, index);
}

GenericResource *ResourceReader::getXmidi(int index) {
	return createResource<GenericResource>(kResXMID, index);
}

GenericResource *ResourceReader::getMidi(int index) {
	return createResource<GenericResource>(kResMIDI, index);
}

void ResourceReader::loadIndex(ResourceSlots *slots) {
	_fd->readUint32LE(); // skip INDX
	_fd->readUint32LE(); // skip index size
	_fd->readUint32LE(); // skip unknown
	_fd->readUint32LE(); // skip res type
	uint16 count1 = _fd->readUint16LE();
	uint16 count2 = _fd->readUint16LE();
	uint16 count = MAX(count1, count2);
	_fd->readUint16LE(); // skip unknown count
	for (uint16 i = 0; i < count; i++) {
		uint32 offs = _fd->readUint32LE();
		uint32 size = _fd->readUint32LE();
		slots->push_back(ResourceSlot(offs, size));
	}
}

void ResourceReader::freeResource(Resource *resource) {
	tossResourceFromCache(resource->_slot);
}

bool ResourceReader::loadResource(ResourceSlot *slot, byte *&buffer, uint32 &size) {
	int offset = !_isV1 ? 62 : 0;
	if (slot && slot->size > 0) {
		size = slot->size - offset;
		buffer = new byte[size];
		debug(2, "ResourceReader::loadResource() %08X", slot->offs + offset);
		_fd->seek(slot->offs + offset);
		_fd->read(buffer, size);
		return true;
	} else {
		return false;
	}
}

ResourceSlot *ResourceReader::getResourceSlot(uint32 resType, uint index) {
	ResourceSlots *slots = _resSlots[resType];

	if (!slots)
		return NULL;

	if (index >= 1 && index < slots->size()) {
		return &(*slots)[index];
	} else {
		return NULL;
	}
}

Resource *ResourceReader::getResourceFromCache(ResourceSlot *slot) {
	if (slot->res)
		slot->refCount++;
	return slot->res;
}

void ResourceReader::addResourceToCache(ResourceSlot *slot, Resource *res) {
	_cacheDataSize += slot->size;

	if (_cacheDataSize >= kMaxResourceCacheSize) {
		purgeCache();
	}

	slot->res = res;
	slot->refCount = 1;
	_cacheCount++;
}

void ResourceReader::tossResourceFromCache(ResourceSlot *slot) {
	if (slot->res)
		slot->refCount--;
}

void ResourceReader::purgeCache() {
	debug(2, "ResourceReader::purgeCache()");
	for (ResMap::const_iterator resTypeIter = _resSlots.begin(); resTypeIter != _resSlots.end(); ++resTypeIter) {
		ResourceSlots *slots = (*resTypeIter)._value;
		for (ResourceSlots::iterator slotIter = slots->begin(); slotIter != slots->end(); ++slotIter) {
			ResourceSlot *slot = &(*slotIter);
			if (slot->refCount <= 0 && slot->res) {
				_cacheDataSize -= slot->size;
				delete slot->res;
				slot->res = NULL;
				slot->refCount = 0;
				_cacheCount--;
			}
		}
	}
}

} // End of namespace Made
