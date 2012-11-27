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

#include "graphics/iff.h"

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/textconsole.h"
#include "parallaction/parallaction.h"
#include "parallaction/parser.h"


namespace Parallaction {

extern byte braAmigaFramesDefaultPalette[];

struct Sprite {
	uint16	size;
	uint16	x;
	uint16	y;
	uint16	w;
	uint16	h;

	byte *packedData;

	Sprite() : size(0), x(0), y(0), w(0), h(0), packedData(0) {
	}

	~Sprite() {
		free(packedData);
	}
};

struct Sprites : public Frames {
	uint16		_num;
	Sprite*		_sprites;

	Sprites(uint num) : _num(0), _sprites(0) {
		_num = num;
		_sprites = new Sprite[_num];
	}

	~Sprites() {
		delete[] _sprites;
	}

	uint16 getNum() {
		return _num;
	}

	byte* getData(uint16 index) {
		assert(index < _num);
		return _sprites[index].packedData;
	}

	void getRect(uint16 index, Common::Rect &r) {
		assert(index < _num);
		r.setWidth(_sprites[index].w);
		r.setHeight(_sprites[index].h);
		r.moveTo(_sprites[index].x, _sprites[index].y);
	}
	uint	getRawSize(uint16 index) {
		assert(index < _num);
		return _sprites[index].size;
	}
	uint	getSize(uint16 index) {
		assert(index < _num);
		return _sprites[index].w * _sprites[index].h;
	}


};

Common::SeekableReadStream *Disk_br::openFile_internal(bool errorOnNotFound, const Common::String &name, const Common::String &ext) {
	assert(!name.empty());
	debugC(5, kDebugDisk, "Disk_br::openFile(%s, %s)", name.c_str(), ext.c_str());

	Common::String lookup(name);

	if (!ext.empty() && !name.hasSuffix(ext.c_str())) {
		// make sure we are using the specified extension
		debugC(9, kDebugDisk, "Disk_br::openFile: appending explicit extension (%s) to (%s)", ext.c_str(), name.c_str());
		lookup = name + ext;
	}

	Common::SeekableReadStream *stream = _sset.createReadStreamForMember(lookup);
	if (stream) {
		return stream;
	}

	// as a very last resort, try trimming the file name to 8 chars
	if (!ext.empty() && lookup.hasSuffix(ext.c_str())) {
		Common::String filename = Common::lastPathComponent(lookup, '/');
		int len = filename.size();
		if (len > 8) {
			debugC(9, kDebugDisk, "Disk_br::openFile: trimming filename (%s) to 8 characters", name.c_str());
			while (len-- > 8) {
				lookup.deleteLastChar();
			}
			lookup += ext;
			stream = _sset.createReadStreamForMember(lookup);
		}
	}

	if (!stream && errorOnNotFound) {
		errorFileNotFound(name);
	}
	return stream;
}

Common::SeekableReadStream *Disk_br::openFile(const Common::String &name, const Common::String &ext) {
	return openFile_internal(true, name, ext);
}

Common::SeekableReadStream *Disk_br::tryOpenFile(const Common::String &name, const Common::String &ext) {
	return openFile_internal(false, name, ext);
}


void Disk_br::errorFileNotFound(const Common::String &filename) {
	error("File '%s' not found", filename.c_str());
}

Common::String DosDisk_br::selectArchive(const Common::String& name) {
	debugC(5, kDebugDisk, "DosDisk_br::selectArchive");

	Common::String oldPath = _currentPart;
	_currentPart = name;

	debugC(5, kDebugDisk, "DosDisk_br::selectArchive: adding part directory to search set");
	_sset.remove("part");
	_sset.add("part", _baseDir->getSubDirectory(name, 3), 10);

	return oldPath;
}

void DosDisk_br::setLanguage(uint16 language) {
	debugC(5, kDebugDisk, "DosDisk_br::setLanguage");
	assert(language < 4);
	_language = language;
}

DosDisk_br::DosDisk_br(Parallaction* vm) : Disk_br(vm) {
}

void DosDisk_br::init() {
	// TODO: clarify whether the engine or OSystem should add the base game directory to the search manager.
	// Right now, I am keeping an internal search set to do the job.
	_baseDir = new Common::FSDirectory(ConfMan.get("path"));
	_sset.add("base", _baseDir, 5, true);
}


GfxObj* DosDisk_br::loadTalk(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadTalk(%s)", name);

	Common::SeekableReadStream *stream = openFile("tal/" + Common::String(name), ".tal");

	// talk position is set to (0,0), because talks are always displayed at
	// absolute coordinates, set in the dialogue manager. The original used
	// to null out coordinates every time they were needed. We do it better!
	Sprites *spr = createSprites(stream);
	for (int i = 0; i < spr->getNum(); i++) {
		spr->_sprites[i].x = 0;
		spr->_sprites[i].y = 0;
	}
	return new GfxObj(0, spr, name);
}

Script* DosDisk_br::loadLocation(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadLocation");

	static const char * const langs[4] = { "it/", "fr/", "en/", "ge/" };

	Common::String fullName(name);
	if (!fullName.hasSuffix(".slf")) {
		fullName += ".loc";
	}

	Common::SeekableReadStream *stream = openFile(langs[_language] + fullName);
	return new Script(stream, true);
}

Script* DosDisk_br::loadScript(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadScript");
	Common::SeekableReadStream *stream = openFile("scripts/" + Common::String(name), ".scr");
	return new Script(stream, true);
}

//	there are no Head resources in Big Red Adventure
GfxObj* DosDisk_br::loadHead(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadHead");
	return 0;
}

void DosDisk_br::loadBitmap(Common::SeekableReadStream &stream, Graphics::Surface &surf, byte *palette) {
	stream.skip(4);
	uint width = stream.readUint32BE();
	if (width & 1) width++;
	uint height = stream.readUint32BE();
	stream.skip(20);

	if (palette) {
		stream.read(palette, 768);
	} else {
		stream.skip(768);
	}

	surf.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	stream.read(surf.pixels, width * height);
}

Frames* DosDisk_br::loadPointer(const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadPointer");
	Common::SeekableReadStream *stream = openFile(Common::String(name), ".ras");
	Graphics::Surface *surf = new Graphics::Surface;
	loadBitmap(*stream, *surf, 0);
	delete stream;
	return new SurfaceToFrames(surf);
}


Font* DosDisk_br::loadFont(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadFont");
	Common::SeekableReadStream *stream = openFile(name, ".fnt");
	Font *font = createFont(name, *stream);
	delete stream;
	return font;
}


GfxObj* DosDisk_br::loadObjects(const char *name, uint8 part) {
	debugC(5, kDebugDisk, "DosDisk_br::loadObjects");
	Common::SeekableReadStream *stream = openFile(name);
	GfxObj *obj = createInventoryObjects(*stream);
	delete stream;
	return obj;
}

void genSlidePath(char *path, const char* name) {
	sprintf(path, "%s.bmp", name);
}

GfxObj* DosDisk_br::loadStatic(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadStatic");
	Common::SeekableReadStream *stream = openFile("ras/" + Common::String(name), ".ras");
	Graphics::Surface *surf = new Graphics::Surface;
	loadBitmap(*stream, *surf, 0);
	delete stream;
	return new GfxObj(0, new SurfaceToFrames(surf), name);
}

Sprites* DosDisk_br::createSprites(Common::ReadStream *stream) {

	uint16 num = stream->readUint16LE();

	Sprites *sprites = new Sprites(num);

	for (uint i = 0; i < num; i++) {
		Sprite *spr = &sprites->_sprites[i];
		spr->size = stream->readUint16LE();
		spr->x = stream->readUint16LE();
		spr->y = stream->readUint16LE();
		spr->w = stream->readUint16LE();
		spr->h = stream->readUint16LE();

		spr->packedData = (byte *)malloc(spr->size);
		stream->read(spr->packedData, spr->size);
	}
	delete stream;

	return sprites;
}

Frames* DosDisk_br::loadFrames(const char* name) {
	Common::SeekableReadStream *stream = 0;

	debugC(5, kDebugDisk, "DosDisk_br::loadFrames");

	Common::String path(name);
	if (path.hasSuffix(".win")) {
		stream = openFile(path);
	} else {
		stream = openFile("ani/" + Common::String(name), ".ani");
	}

	return createSprites(stream);
}

// Slides in Nippon Safes are basically screen-sized pictures with valid
// palette data used for menu and for location switches. Big Red Adventure
// doesn't need slides in that sense, but it still has some special
// graphics resources with palette data, so those will be named slides.
//
void DosDisk_br::loadSlide(BackgroundInfo& info, const char *name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadSlide");

	Common::SeekableReadStream *stream = openFile(name, ".bmp");

	byte rgb[768];

	loadBitmap(*stream, info.bg, rgb);
	info.width = info.bg.w;
	info.height = info.bg.h;

	delete stream;

	for (uint i = 0; i < 256; i++) {
		info.palette.setEntry(i, rgb[i] >> 2, rgb[i+256] >> 2, rgb[i+512] >> 2);
	}
}

MaskBuffer *DosDisk_br::loadMask(const char *name, uint32 w, uint32 h) {
	if (!name) {
		return 0;
	}

	Common::SeekableReadStream *stream = openFile("msk/" + Common::String(name), ".msk");

	MaskBuffer *buffer = new MaskBuffer;
	assert(buffer);
	buffer->create(w, h);
	buffer->bigEndian = false;

	stream->read(buffer->data, buffer->size);
	delete stream;

	return buffer;
}

PathBuffer *DosDisk_br::loadPath(const char *name, uint32 w, uint32 h) {
	if (!name) {
		return 0;
	}

	Common::SeekableReadStream *stream = openFile("pth/" + Common::String(name), ".pth");

	PathBuffer *buffer = new PathBuffer;
	assert(buffer);
	buffer->create(w, h);
	buffer->bigEndian = false;

	stream->read(buffer->data, buffer->size);
	delete stream;

	return buffer;
}

void DosDisk_br::loadScenery(BackgroundInfo& info, const char *name, const char *mask, const char* path) {
	debugC(5, kDebugDisk, "DosDisk_br::loadScenery");

	Common::SeekableReadStream *stream;

	if (name) {
		stream = openFile("bkg/" + Common::String(name), ".bkg");
		byte rgb[768];

		loadBitmap(*stream, info.bg, rgb);
		info.width = info.bg.w;
		info.height = info.bg.h;

		for (uint i = 0; i < 256; i++) {
			info.palette.setEntry(i, rgb[i] >> 2, rgb[i+256] >> 2, rgb[i+512] >> 2);
		}

		delete stream;
	}

	if (mask) {
		info._mask = loadMask(mask, info.width, info.height);
	}

	if (path) {
		info._path = loadPath(path, info.width, info.height);
	}
}

Table* DosDisk_br::loadTable(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadTable");
	return createTableFromStream(100, openFile(name, ".tab"));
}

Common::SeekableReadStream* DosDisk_br::loadMusic(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadMusic");
	return openFile("msc/" + Common::String(name), ".msc");
}


Common::SeekableReadStream* DosDisk_br::loadSound(const char* name) {
	debugC(5, kDebugDisk, "DosDisk_br::loadSound");
	return openFile("sfx/" + Common::String(name), ".sfx");
}




DosDemoDisk_br::DosDemoDisk_br(Parallaction *vm) : DosDisk_br(vm) {

}

void DosDemoDisk_br::init() {
	// TODO: clarify whether the engine or OSystem should add the base game directory to the search manager.
	// Right now, I am keeping an internal search set to do the job.
	_baseDir = new Common::FSDirectory(ConfMan.get("path"), 2);
	_sset.add("base", _baseDir, 5, false);
}


Common::String DosDemoDisk_br::selectArchive(const Common::String& name) {
	debugC(5, kDebugDisk, "DosDemoDisk_br::selectArchive");
	Common::String oldPath = _currentPart;
	_currentPart = name;
	return oldPath;
}


AmigaDisk_br::AmigaDisk_br(Parallaction *vm) : DosDisk_br(vm) {
}

void AmigaDisk_br::init() {
	_baseDir = new Common::FSDirectory(ConfMan.get("path"));
	_sset.add("base", _baseDir, 5, false);

	const Common::String subDirNames[3] = { "fonts", "backs", "common" };
	const Common::String subDirPrefixes[3] = { "fonts", "backs", "" };
	// The common sub directory, doesn't exist in the Amiga demo
	uint numDir = (_vm->getFeatures() & GF_DEMO) ? 2 : 3;
	for (uint i = 0; i < numDir; i++)
		_sset.add(subDirNames[i], _baseDir->getSubDirectory(subDirPrefixes[i], subDirNames[i], 2), 6);
}

void AmigaDisk_br::adjustForPalette(Graphics::Surface &surf, int transparentColor) {
	uint size = surf.w * surf.h;
	byte *data = (byte *)surf.pixels;
	for (uint i = 0; i < size; i++, data++) {
		if (transparentColor == -1 || transparentColor != *data)
			*data += 16;
	}
}

void AmigaDisk_br::loadBackground(BackgroundInfo& info, const char *filename) {
	byte r,g,b;
	byte *p;
	Common::SeekableReadStream *stream;
	uint i;

	stream = tryOpenFile("backs/" + Common::String(filename), ".ap");
	if (stream) {
		// NOTE: Additional palette (15-colors) is always loaded onto colors 1-15
		uint32 size = stream->size() / 3;
		for (i = 1; i < size; i++) {
			r = stream->readByte() >> 2;
			g = stream->readByte() >> 2;
			b = stream->readByte() >> 2;
			info.palette.setEntry(i, r, g, b);
		}
		delete stream;
	} else {
		p = braAmigaFramesDefaultPalette;
		for (i = 0; i < 16; i++) {
			r = *p >> 2;
			p++;
			g = *p >> 2;
			p++;
			b = *p >> 2;
			p++;
			info.palette.setEntry(i, r, g, b);
		}
	}

	stream = openFile("backs/" + Common::String(filename), ".bkg");

	byte pal[768];
	ILBMLoader loader(&info.bg, pal);
	loader.load(stream, true);

	info.width = info.bg.w;
	info.height = info.bg.h;

	p = pal;
	for (i = 16; i < 32; i++) {
		r = *p >> 2;
		p++;
		g = *p >> 2;
		p++;
		b = *p >> 2;
		p++;
		info.palette.setEntry(i, r, g, b);
	}

	// Overwrite the first color (transparent key) in the palette
	info.palette.setEntry(0, pal[0] >> 2, pal[1] >> 2, pal[2] >> 0);

	// background data is drawn used the upper portion of the palette
	adjustForPalette(info.bg);
}

void finalpass(byte *buffer, uint32 size) {
	byte b = 0xC0;
	byte r1 = 0x40;
	byte r2 = 0x80;
	for (uint32 i = 0; i < size*4; i++) {
		byte s = buffer[i/4];
		s &= b;

		if (s == r1) {
			buffer[i/4] |= b;
		} else
		if (s == b) {
			buffer[i/4] ^= r2;
		}

		b >>= 2; if (b == 0) { b = 0xC0; }
		r1 >>= 2; if (r1 == 0) { r1 = 0x40; }
		r2 >>= 2; if (r2 == 0) { r2 = 0x80; }
	}
}

MaskBuffer *AmigaDisk_br::loadMask(const char *name, uint32 w, uint32 h) {
	if (!name) {
		return 0;
	}
	debugC(1, kDebugDisk, "AmigaDisk_br::loadMask '%s'", name);

	Common::SeekableReadStream *stream = tryOpenFile("msk/" + Common::String(name), ".msk");
	if (!stream) {
		return 0;
	}

	ILBMLoader loader(ILBMLoader::BODYMODE_MASKBUFFER);
	loader.load(stream, true);

	MaskBuffer *buffer = loader._maskBuffer;
	buffer->bigEndian = true;
	finalpass(buffer->data, buffer->size);
	return buffer;
}

void AmigaDisk_br::loadScenery(BackgroundInfo& info, const char* name, const char* mask, const char* path) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadScenery '%s', '%s' '%s'", name, mask, path);

	if (name) {
		loadBackground(info, name);
	}
	if (mask) {
		info._mask = loadMask(mask, info.width, info.height);
	}

	if (path) {
		info._path = loadPath(path, info.width, info.height);
	}
}

void AmigaDisk_br::loadSlide(BackgroundInfo& info, const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadSlide '%s'", name);
	loadBackground(info, name);
}

GfxObj* AmigaDisk_br::loadStatic(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadStatic '%s'", name);

	Common::String sName = name;
	Common::SeekableReadStream *stream = openFile("ras/" + sName, ".ras");

	ILBMLoader loader(ILBMLoader::BODYMODE_SURFACE);
	loader.load(stream, true);

	Graphics::Surface* surf = loader._surf;
	assert(surf);

	// Static pictures are drawn used the upper half of the palette: this must be
	// done before shadow mask is applied. This way, only really transparent pixels
	// will have zero as a color.
	adjustForPalette(*surf);

	// NOTE: this assumes that the extension is always present in the file name
	sName.deleteLastChar();
	sName.deleteLastChar();
	sName.deleteLastChar();
	sName.deleteLastChar();
	stream = tryOpenFile("ras/" + sName + ".ras_shdw");

	if (!stream) {
		debugC(9, kDebugDisk, "Cannot find shadow file for '%s'\n", name);
	} else {
		uint32 shadowWidth = ((surf->w + 15)/8) & ~1;
		uint32 shadowSize = shadowWidth * surf->h;
		byte *shadow = new byte[shadowSize];
		assert(shadow);
		stream->read(shadow, shadowSize);
		for (int32 i = 0; i < surf->h; ++i) {
			byte *src = shadow + shadowWidth * i;
			byte *dst = (byte *)surf->pixels + surf->pitch * i;

			for (int32 j = 0; j < surf->w; ++j, ++dst) {
				byte bit = src[j/8] & (1 << (7 - (j & 7)));
				if (bit == 0) *dst = 0;
			}
		}

		delete[] shadow;
		delete stream;
	}

	return new GfxObj(0, new SurfaceToFrames(surf), name);
}

Sprites* AmigaDisk_br::createSprites(Common::ReadStream *stream) {
	uint16 num = stream->readUint16BE();

	Sprites *sprites = new Sprites(num);

	for (uint i = 0; i < num; i++) {
		Sprite *spr = &sprites->_sprites[i];
		spr->size = stream->readUint16BE();
		spr->x = stream->readUint16BE();
		spr->y = stream->readUint16BE();
		spr->w = stream->readUint16BE();
		spr->h = stream->readUint16BE() - 1;

		spr->packedData = (byte *)malloc(spr->size);
		stream->read(spr->packedData, spr->size);
	}

	delete stream;

	return sprites;
}

Frames* AmigaDisk_br::loadFrames(const char* name) {
	Common::SeekableReadStream *stream = 0;

	debugC(5, kDebugDisk, "AmigaDisk_br::loadFrames");

	Common::String path(name);
	if (path.hasSuffix(".win")) {
		stream = openFile(path);
	} else {
		stream = openFile("anims/" + Common::String(name), ".ani");
	}

	return createSprites(stream);
}

GfxObj* AmigaDisk_br::loadTalk(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_br::loadTalk '%s'", name);

	Common::SeekableReadStream *stream = openFile("talks/" + Common::String(name), ".tal");

	// talk position is set to (0,0), because talks are always displayed at
	// absolute coordinates, set in the dialogue manager. The original used
	// to null out coordinates every time they were needed. We do it better!
	Sprites *spr = createSprites(stream);
	for (int i = 0; i < spr->getNum(); i++) {
		spr->_sprites[i].x = 0;
		spr->_sprites[i].y = 0;
	}
	return new GfxObj(0, spr, name);
}

Font* AmigaDisk_br::loadFont(const char* name) {
	debugC(1, kDebugDisk, "AmigaFullDisk::loadFont '%s'", name);

	Common::SeekableReadStream *stream = openFile("fonts/" + Common::String(name), ".font");

	Common::String fontDir;
	Common::String fontFile;
	byte ch;

	stream->seek(4, SEEK_SET);
	while ((ch = stream->readByte()) != 0x2F) fontDir += ch;
	while ((ch = stream->readByte()) != 0) fontFile += ch;
	delete stream;

	stream = openFile("fonts/" + fontDir + "/" + fontFile);
	Font *font = createFont(name, *stream);

	delete stream;
	return font;
}

Common::SeekableReadStream* AmigaDisk_br::loadMusic(const char* name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::loadMusic");
	return tryOpenFile("msc/" + Common::String(name), ".msc");
}


Common::SeekableReadStream* AmigaDisk_br::loadSound(const char* name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::loadSound");
	return openFile("sfx/" + Common::String(name), ".sfx");
}

static const uint16 objectsMax[5] = {
	5, 73, 71, 19, 48
};

GfxObj* AmigaDisk_br::loadObjects(const char *name, uint8 part) {
	debugC(5, kDebugDisk, "AmigaDisk_br::loadObjects");

	Common::SeekableReadStream *stream = openFile(name);
	ILBMLoader loader(ILBMLoader::BODYMODE_SURFACE);
	loader.load(stream, true);

	uint16 max = objectsMax[part];
	if (_vm->getFeatures() & GF_DEMO)
		max = 72;

	byte *data = new byte[max * 2601];
	byte *srcPtr = (byte *)loader._surf->getBasePtr(0,0);
	int w = loader._surf->w;

	// Convert to the expected display format
	for (int i = 0; i < max; i++) {
		uint16 x = (i % 8) * 51;
		uint16 y = (i / 8) * 51;

		byte *src = srcPtr + y * w + x;
		byte *dst = data + i * 2601;
		for (int h = 0; h < 51; h++) {
			memcpy(dst, src, 51);
			src += w;
			dst += 51;
		}
	}
	delete loader._surf;

	return new GfxObj(0, new Cnv(max, 51, 51, data, true));
}

Common::String AmigaDisk_br::selectArchive(const Common::String& name) {
	debugC(5, kDebugDisk, "AmigaDisk_br::selectArchive");

	Common::String oldPath = _currentPart;
	_currentPart = name;

	debugC(5, kDebugDisk, "AmigaDisk_br::selectArchive: adding part directory to search set");
	_sset.remove("part");
	_sset.add("part", _baseDir->getSubDirectory(name, 3), 10);

	return oldPath;
}


Disk_br::Disk_br(Parallaction *vm) : _vm(vm), _baseDir(0) {

}

Disk_br::~Disk_br() {
	_sset.clear();
}



} // namespace Parallaction
