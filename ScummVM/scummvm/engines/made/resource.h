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

#ifndef MADE_RESOURCE_H
#define MADE_RESOURCE_H

#include "made/sound.h"

#include "common/endian.h"
#include "common/array.h"
#include "common/hashmap.h"

namespace Common {
class File;
}

namespace Audio {
class AudioStream;
}

namespace Graphics {
struct Surface;
}

namespace Made {

/// This value specifies the size of the resource cache
/// which stores recently used resources.  On the DS,
/// 400Kb is all we can spare, while 1Mb seems like a
/// good value for larger systems.
#ifndef __DS__
const int kMaxResourceCacheSize = 1000 * 1024;
#else
const int kMaxResourceCacheSize = 400 * 1024;
#endif


enum ResourceType {
	kResARCH = MKTAG('A','R','C','H'),
	kResFREE = MKTAG('F','R','E','E'),
	kResOMNI = MKTAG('O','M','N','I'),
	kResFLEX = MKTAG('F','L','E','X'),
	kResSNDS = MKTAG('S','N','D','S'),
	kResANIM = MKTAG('A','N','I','M'),
	kResMENU = MKTAG('M','E','N','U'),
	kResFONT = MKTAG('F','O','N','T'),
	kResXMID = MKTAG('X','M','I','D'),
	kResMIDI = MKTAG('M','I','D','I')
};

struct ResourceSlot;

class Resource {
public:
	ResourceSlot *_slot;
	virtual ~Resource();
};

class PictureResource : public Resource {
public:
	PictureResource();
	virtual ~PictureResource();
	void load(byte *source, int size);
	Graphics::Surface *getPicture() const { return _picture; }
	byte *getPalette() const { return _picturePalette; }
	bool hasPalette() const { return _hasPalette; }
	int getPaletteColorCount() const { return _paletteColorCount; }
protected:
	Graphics::Surface *_picture;
	byte *_picturePalette;
	int _paletteColorCount;
	bool _hasPalette;
	void loadRaw(byte *source, int size);
	void loadChunked(byte *source, int size);
};

class AnimationResource : public Resource {
public:
	AnimationResource();
	virtual ~AnimationResource();
	void load(byte *source, int size);
	int getCount() const { return _frames.size(); }
	Graphics::Surface *getFrame(int index) const {
		if ((uint)index < _frames.size()) {
			return _frames[index];
		} else {
			warning("getFrame: Tried to obtain invalid frame %i, array has %i frames", index, _frames.size());
			return _frames[_frames.size() - 1];
		}
	}
	uint16 getFlags() const { return _flags; }
	int16 getWidth() const { return _width; }
	int16 getHeight() const { return _height; }
protected:
	Common::Array<Graphics::Surface *> _frames;
	uint16 _flags;
	int16 _width, _height;
};

class SoundResource : public Resource {
public:
	SoundResource();
	virtual ~SoundResource();
	virtual void load(byte *source, int size);
	Audio::AudioStream *getAudioStream(int soundRate, bool loop = false);
	SoundEnergyArray *getSoundEnergyArray() const { return _soundEnergyArray; }
protected:
	byte *_soundData;
	int _soundSize;
	SoundEnergyArray *_soundEnergyArray;
};

class SoundResourceV1 : public SoundResource {
public:
	SoundResourceV1() {}
	virtual ~SoundResourceV1() {}
	void load(byte *source, int size);
};

class MenuResource : public Resource {
public:
	MenuResource();
	virtual ~MenuResource();
	void load(byte *source, int size);
	int getCount() const { return _strings.size(); }
	const char *getString(uint index) const;
protected:
	Common::Array<Common::String> _strings;
};

class FontResource : public Resource {
public:
	FontResource();
	virtual ~FontResource();
	void load(byte *source, int size);
	int getHeight() const;
	int getCharWidth(uint c) const;
	int getTextWidth(const char *text);
	byte *getChar(uint c) const;
protected:
	byte *_data;
	int _size;
	byte *getCharData(uint c) const;
};

class GenericResource : public Resource {
public:
	GenericResource();
	virtual ~GenericResource();
	void load(byte *source, int size);
	byte *getData() const { return _data; }
	int getSize() const { return _size; }
protected:
	byte *_data;
	int _size;
};

struct ResourceSlot {
	uint32 offs;
	uint32 size;
	Resource *res;
	int refCount;
	ResourceSlot() : offs(0), size(0), res(NULL), refCount(0) {
	}
	ResourceSlot(uint32 roffs, uint32 rsize) : offs(roffs), size(rsize), res(NULL), refCount(0) {
	}
};

class ResourceReader {
public:
	ResourceReader();
	~ResourceReader();

	void open(const char *filename);
	void openResourceBlocks();

	PictureResource *getPicture(int index);
	AnimationResource *getAnimation(int index);
	SoundResource *getSound(int index);
	MenuResource *getMenu(int index);
	FontResource *getFont(int index);
	GenericResource *getXmidi(int index);
	GenericResource *getMidi(int index);

	void freeResource(Resource *resource);

protected:

	Common::File *_fd;
	Common::File *_fdPics, *_fdSounds, *_fdMusic;		// V1
	bool _isV1;

	typedef Common::Array<ResourceSlot> ResourceSlots;
	typedef Common::HashMap<uint32, ResourceSlots *> ResMap;
	void openResourceBlock(const char *filename, Common::File *blockFile, uint32 resType);

	ResMap _resSlots;
	int _cacheCount;
	int _cacheDataSize;

	void loadIndex(ResourceSlots *slots);

	template<class T>
	T *createResource(uint32 resType, int index) {
		ResourceSlot *slot = getResourceSlot(resType, index);
		if (!slot)
			return NULL;
		T *res = (T *)getResourceFromCache(slot);
		if (!res) {
			byte *buffer;
			uint32 size;

			// Read from the correct file for V1 games
			if (_isV1) {
				switch (resType) {
				case kResSNDS:
					_fd = _fdSounds;
					break;
				case kResMIDI:
					_fd = _fdMusic;
					break;
				default:
					_fd = _fdPics;
					break;
				}
			}

			if (loadResource(slot, buffer, size)) {
				res = new T();
				res->_slot = slot;
				res->load(buffer, size);
				addResourceToCache(slot, res);
				delete[] buffer;
			}
		}
		return res;
	}

	bool loadResource(ResourceSlot *slot, byte *&buffer, uint32 &size);
	ResourceSlot *getResourceSlot(uint32 resType, uint index);
	Resource *getResourceFromCache(ResourceSlot *slot);
	void addResourceToCache(ResourceSlot *slot, Resource *res);
	void tossResourceFromCache(ResourceSlot *slot);
	void purgeCache();

};

} // End of namespace Made

#endif /* MADE_H */
