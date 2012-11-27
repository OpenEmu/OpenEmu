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

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/memstream.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "parallaction/parser.h"
#include "parallaction/parallaction.h"


namespace Parallaction {


//  HACK: Several archives ('de', 'en', 'fr' and 'disk0') in the multi-lingual
//  Amiga version of Nippon Safes, and one archive ('fr') in the Amiga Demo of
//  Nippon Safes used different internal offsets than all the other archives.
//
//  When an archive is opened in the Amiga demo, its size is checked against
//  SIZEOF_SMALL_ARCHIVE to detect when the smaller archive is used.
//
//  When an archive is opened in Amiga multi-lingual version, the header is
//  checked again NDOS to detect when a smaller archive is used.
//
#define SIZEOF_SMALL_ARCHIVE		12778

#define ARCHIVE_FILENAMES_OFS		0x16

#define NORMAL_ARCHIVE_FILES_NUM	384
#define SMALL_ARCHIVE_FILES_NUM		180

#define NORMAL_ARCHIVE_SIZES_OFS	0x3016
#define SMALL_ARCHIVE_SIZES_OFS		0x1696

#define NORMAL_ARCHIVE_DATA_OFS		0x4000
#define SMALL_ARCHIVE_DATA_OFS		0x1966

#define MAX_ARCHIVE_ENTRIES			384

class NSArchive : public Common::Archive {

	Common::SeekableReadStream	*_stream;

	char			_archiveDir[MAX_ARCHIVE_ENTRIES][32];
	uint32			_archiveLengths[MAX_ARCHIVE_ENTRIES];
	uint32			_archiveOffsets[MAX_ARCHIVE_ENTRIES];
	uint32			_numFiles;

	uint32			lookup(const char *name) const;

public:
	NSArchive(Common::SeekableReadStream *stream, Common::Platform platform, uint32 features);
	~NSArchive();

	Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;
	bool hasFile(const Common::String &name) const;
	int listMembers(Common::ArchiveMemberList &list) const;
	const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
};


NSArchive::NSArchive(Common::SeekableReadStream *stream, Common::Platform platform, uint32 features) : _stream(stream) {
	if (!_stream) {
		error("NSArchive: invalid stream passed to constructor");
	}

	bool isSmallArchive = false;
	if (platform == Common::kPlatformAmiga) {
		if (features & GF_DEMO) {
			isSmallArchive = stream->size() == SIZEOF_SMALL_ARCHIVE;
		} else if (features & GF_LANG_MULT) {
			isSmallArchive = (stream->readUint32BE() != MKTAG('N','D','O','S'));
		}
	}

	_numFiles = (isSmallArchive) ? SMALL_ARCHIVE_FILES_NUM : NORMAL_ARCHIVE_FILES_NUM;

	_stream->seek(ARCHIVE_FILENAMES_OFS);
	_stream->read(_archiveDir, _numFiles*32);

	_stream->seek((isSmallArchive) ? SMALL_ARCHIVE_SIZES_OFS : NORMAL_ARCHIVE_SIZES_OFS);

	uint32 dataOffset = (isSmallArchive) ? SMALL_ARCHIVE_DATA_OFS : NORMAL_ARCHIVE_DATA_OFS;
	for (uint16 i = 0; i < _numFiles; i++) {
		_archiveOffsets[i] = dataOffset;
		_archiveLengths[i] = _stream->readUint32BE();
		dataOffset += _archiveLengths[i];
	}

}

NSArchive::~NSArchive() {
	delete _stream;
}

uint32 NSArchive::lookup(const char *name) const {
	uint32 i = 0;
	for ( ; i < _numFiles; i++) {
		if (!scumm_stricmp(_archiveDir[i], name)) break;
	}
	return i;
}

Common::SeekableReadStream *NSArchive::createReadStreamForMember(const Common::String &name) const {
	debugC(3, kDebugDisk, "NSArchive::createReadStreamForMember(%s)", name.c_str());

	if (name.empty())
		return 0;

	uint32 index = lookup(name.c_str());
	if (index == _numFiles) return 0;

	debugC(9, kDebugDisk, "NSArchive::createReadStreamForMember: '%s' found in slot %i", name.c_str(), index);

	int offset = _archiveOffsets[index];
	int endOffset = _archiveOffsets[index] + _archiveLengths[index];
	return new Common::SeekableSubReadStream(_stream, offset, endOffset, DisposeAfterUse::NO);
}

bool NSArchive::hasFile(const Common::String &name) const {
	if (name.empty())
		return false;
	return lookup(name.c_str()) != _numFiles;
}

int NSArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (uint32 i = 0; i < _numFiles; i++) {
		list.push_back(Common::SharedPtr<Common::GenericArchiveMember>(new Common::GenericArchiveMember(_archiveDir[i], this)));
	}
	return _numFiles;
}

const Common::ArchiveMemberPtr NSArchive::getMember(const Common::String &name) const {
	uint32 index = lookup(name.c_str());

	const char *item = 0;
	if (index < _numFiles) {
		item = _archiveDir[index];
	}

	return Common::SharedPtr<Common::GenericArchiveMember>(new Common::GenericArchiveMember(item, this));
}


#define HIGHEST_PRIORITY		9
#define NORMAL_ARCHIVE_PRIORITY		5
#define LOW_ARCHIVE_PRIORITY		2
#define LOWEST_ARCHIVE_PRIORITY		1

Disk_ns::Disk_ns(Parallaction *vm) : _vm(vm) {
	Common::FSDirectory *baseDir = new Common::FSDirectory(ConfMan.get("path"));
	_sset.add("basedir", baseDir, HIGHEST_PRIORITY);
}

Disk_ns::~Disk_ns() {
	_sset.clear();
}

void Disk_ns::errorFileNotFound(const char *s) {
	error("File '%s' not found", s);
}

Common::SeekableReadStream *Disk_ns::openFile(const char *filename) {
	Common::SeekableReadStream *stream = tryOpenFile(filename);
	if (!stream)
		errorFileNotFound(filename);
	return stream;
}


void Disk_ns::addArchive(const Common::String& name, int priority) {
	Common::SeekableReadStream *stream = _sset.createReadStreamForMember(name);
	if (!stream)
		error("Disk_ns::addArchive() couldn't find archive '%s'", name.c_str());

	debugC(1, kDebugDisk, "Disk_ns::addArchive(name = %s, priority = %i)", name.c_str(), priority);

	NSArchive *arc = new NSArchive(stream, _vm->getPlatform(), _vm->getFeatures());
	_sset.add(name, arc, priority);
}

Common::String Disk_ns::selectArchive(const Common::String& name) {
	Common::String oldName = _resArchiveName;

	if (_sset.hasArchive(name)) {
		return oldName;
	}

	if (!_resArchiveName.empty()) {
		_sset.remove(_resArchiveName);
	}
	_resArchiveName = name;
	addArchive(name, NORMAL_ARCHIVE_PRIORITY);

	return oldName;
}

void Disk_ns::setLanguage(uint16 language) {
	debugC(1, kDebugDisk, "setLanguage(%i)", language);
	assert(language < 4);

	if (!_language.empty()) {
		_sset.remove(_language);
	}

	static const char *languages[] = { "it", "fr", "en", "ge" };
	_language = languages[language];

	if (_sset.hasArchive(_language)) {
		return;
	}

	addArchive(_language, LOWEST_ARCHIVE_PRIORITY);
}

#pragma mark -


DosDisk_ns::DosDisk_ns(Parallaction* vm) : Disk_ns(vm) {

}

DosDisk_ns::~DosDisk_ns() {
}

void DosDisk_ns::init() {
	// setup permament archives
	addArchive("disk1", LOW_ARCHIVE_PRIORITY);
}

Common::SeekableReadStream *DosDisk_ns::tryOpenFile(const char* name) {
	debugC(3, kDebugDisk, "DosDisk_ns::tryOpenFile(%s)", name);

	Common::SeekableReadStream *stream = _sset.createReadStreamForMember(name);
	if (stream)
		return stream;

	char path[PATH_LEN];
	sprintf(path, "%s.pp", name);
	return _sset.createReadStreamForMember(path);
}

Script* Disk_ns::loadLocation(const char *name) {
	char path[PATH_LEN];
	const char *charName = _vm->_char.getBaseName();

	// WORKAROUND: Special case for the Multilingual DOS version: during the ending
	// sequence, it tries to load a non-existing file using "Dinor" as a character
	// name. In this case, the character name should be just "dino".
	if (!strcmp(charName, "Dinor"))
		charName = "dino";

	sprintf(path, "%s%s/%s.loc", charName, _language.c_str(), name);
	debugC(3, kDebugDisk, "Disk_ns::loadLocation(%s): trying '%s'", name, path);
	Common::SeekableReadStream *stream = tryOpenFile(path);

	if (!stream) {
		sprintf(path, "%s/%s.loc", _language.c_str(), name);
		debugC(3, kDebugDisk, "DosDisk_ns::loadLocation(%s): trying '%s'", name, path);
		stream = openFile(path);
	}
	return new Script(stream, true);
}

Script* Disk_ns::loadScript(const char* name) {
	debugC(1, kDebugDisk, "Disk_ns::loadScript '%s'", name);
	char path[PATH_LEN];
	sprintf(path, "%s.script", name);
	Common::SeekableReadStream *stream = openFile(path);
	return new Script(stream, true);
}

Cnv *Disk_ns::makeCnv(Common::SeekableReadStream *stream) {
	assert(stream);

	uint16 numFrames = stream->readByte();
	uint16 width = stream->readByte();
	assert((width & 7) == 0);
	uint16 height = stream->readByte();
	uint32 decsize = numFrames * width * height;
	byte *data = new byte[decsize];
	assert(data);
	memset(data, 0, decsize);

	decodeCnv(data, numFrames, width, height, stream);

	delete stream;
	return new Cnv(numFrames, width, height, data, true);
}

void DosDisk_ns::decodeCnv(byte *data, uint16 numFrames, uint16 width, uint16 height, Common::SeekableReadStream *stream) {
	int32 decsize = numFrames * width * height;
	bool packed = (stream->size() - stream->pos()) != decsize;
	if (packed) {
		Graphics::PackBitsReadStream decoder(*stream);
		decoder.read(data, decsize);
	} else {
		stream->read(data, decsize);
	}
}

Cnv* DosDisk_ns::loadCnv(const char *filename) {
	Common::SeekableReadStream *stream = openFile(filename);
	assert(stream);
	return makeCnv(stream);
}


GfxObj* DosDisk_ns::loadTalk(const char *name) {

	const char *ext = strstr(name, ".talk");
	if (ext) {
		// npc talk
		return new GfxObj(0, loadCnv(name), name);
	}

	char v20[30];
	if (g_engineFlags & kEngineTransformedDonna) {
		sprintf(v20, "%stta.cnv", name);
	} else {
		sprintf(v20, "%stal.cnv", name);
	}

	return new GfxObj(0, loadCnv(v20), name);
}


GfxObj* DosDisk_ns::loadHead(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%shead", name);
	path[8] = '\0';
	strcat(path, ".cnv");
	return new GfxObj(0, loadCnv(path));
}


Frames* DosDisk_ns::loadPointer(const char *name) {
	char path[PATH_LEN];
	sprintf(path, "%s.cnv", name);
	return loadCnv(path);
}


Font* DosDisk_ns::loadFont(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%scnv.cnv", name);
	return createFont(name, loadCnv(path));
}


GfxObj* DosDisk_ns::loadObjects(const char *name, uint8 part) {
	char path[PATH_LEN];
	sprintf(path, "%sobj.cnv", name);
	return new GfxObj(0, loadCnv(path), name);
}


GfxObj* DosDisk_ns::loadStatic(const char* name) {
	return new GfxObj(0, loadCnv(name), name);
}

Frames* DosDisk_ns::loadFrames(const char* name) {
	return loadCnv(name);
}

/*
	Background images are compressed using a RLE algorithm that resembles PackBits.

	The uncompressed data is then unpacked as following:
	- color data [bits 0-5]
	- mask data [bits 6-7] (z buffer)
	- path data [bit 8] (walkable areas)
*/
void DosDisk_ns::unpackBackground(Common::ReadStream *stream, byte *screen, byte *mask, byte *path) {
	byte storage[127];
	uint32 storageLen = 0, len = 0;
	uint32 j = 0;

	while (1) {
		// first extracts packbits variant data
		do {
			len = stream->readByte();
			if (stream->eos())
				return;

			if (len == 128) {
				storageLen = 0;
			} else if (len <= 127) {
				len++;
				for (uint32 i = 0; i < len; i++) {
					storage[i] = stream->readByte();
				}
				storageLen = len;
			} else {
				len = (256 - len) + 1;
				byte v = stream->readByte();
				memset(storage, v, len);
				storageLen = len;
			}
		} while (storageLen == 0);

		// then unpacks the bits to the destination buffers
		for (uint32 i = 0; i < storageLen; i++, j++) {
			byte b = storage[i];
			path[j/8] |= ((b & 0x80) >> 7) << (j & 7);
			mask[j/4] |= ((b & 0x60) >> 5) << ((j & 3) << 1);
			screen[j] = b & 0x1F;
		}
	}
}

void DosDisk_ns::parseDepths(BackgroundInfo &info, Common::SeekableReadStream &stream) {
	info.layers[0] = stream.readByte();
	info.layers[1] = stream.readByte();
	info.layers[2] = stream.readByte();
	info.layers[3] = stream.readByte();
}

void DosDisk_ns::createMaskAndPathBuffers(BackgroundInfo &info) {
	info._mask = new MaskBuffer;
	assert(info._mask);
	info._mask->create(info.width, info.height);
	info._mask->bigEndian = true;

	info._path = new PathBuffer;
	assert(info._path);
	info._path->create(info.width, info.height);
	info._path->bigEndian = true;
}

void DosDisk_ns::loadBackground(BackgroundInfo& info, const char *filename) {
	Common::SeekableReadStream *stream = openFile(filename);

	info.width = _vm->_screenWidth;	// 320
	info.height = _vm->_screenHeight;	// 200

	// read palette
	byte tmp[3];
	for (uint i = 0; i < 32; i++) {
		tmp[0] = stream->readByte();
		tmp[1] = stream->readByte();
		tmp[2] = stream->readByte();
		info.palette.setEntry(i, tmp[0], tmp[1], tmp[2]);
	}

	// read z coordinates
	parseDepths(info, *stream);

	// read palette rotation parameters
	PaletteFxRange range;
	for (uint32 _si = 0; _si < 6; _si++) {
		range._timer = stream->readUint16BE();
		range._step = stream->readUint16BE();
		range._flags = stream->readUint16BE();
		range._first = stream->readByte();
		range._last = stream->readByte();
		info.setPaletteRange(_si, range);
	}

	// read bitmap, mask and path data and extract them into the 3 buffers
	info.bg.create(info.width, info.height, Graphics::PixelFormat::createFormatCLUT8());
	createMaskAndPathBuffers(info);
	unpackBackground(stream, (byte *)info.bg.pixels, info._mask->data, info._path->data);

	delete stream;
}


void DosDisk_ns::loadSlide(BackgroundInfo& info, const char *filename) {
	char path[PATH_LEN];
	sprintf(path, "%s.slide", filename);
	loadBackground(info, path);
}

void DosDisk_ns::loadScenery(BackgroundInfo& info, const char *name, const char *mask, const char* path) {
	char filename[PATH_LEN];
	sprintf(filename, "%s.dyn", name);

	// load bitmap
	loadBackground(info, filename);

	if (mask == 0) {
		return;
	}

	// load external mask and path if present (overwriting the ones loaded by loadBackground)
	char maskPath[PATH_LEN];
	sprintf(maskPath, "%s.msk", mask);

	Common::SeekableReadStream *stream = openFile(maskPath);
	assert(stream);

	parseDepths(info, *stream);

	createMaskAndPathBuffers(info);
	stream->read(info._path->data, info._path->size);
	stream->read(info._mask->data, info._mask->size);

	delete stream;
}

Table* DosDisk_ns::loadTable(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%s.tab", name);
	return createTableFromStream(100, openFile(path));
}

Common::SeekableReadStream* DosDisk_ns::loadMusic(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%s.mid", name);
	return openFile(path);
}


Common::SeekableReadStream* DosDisk_ns::loadSound(const char* name) {
	return 0;
}






#pragma mark -


/* the decoder presented here is taken from pplib by Stuart Caie. The
 * following statement comes from the original source.
 *
 * pplib 1.0: a simple PowerPacker decompression and decryption library
 * placed in the Public Domain on 2003-09-18 by Stuart Caie.
 */

#define PP_READ_BITS(nbits, var) do {				\
	bit_cnt = (nbits); (var) = 0;				\
	while (bits_left < bit_cnt) {				\
		if (buf < src) return 0;			\
		bit_buffer |= *--buf << bits_left;		\
		bits_left += 8;					\
	}							\
	bits_left -= bit_cnt;					\
	while (bit_cnt--) {					\
		(var) = ((var) << 1) | (bit_buffer & 1);	\
		bit_buffer >>= 1;				\
	}							\
} while (0)

#define PP_BYTE_OUT(byte) do {					\
	if (out <= dest) return 0;				\
	*--out = (byte); written++;				\
} while (0)


class PowerPackerStream : public Common::SeekableReadStream {

	SeekableReadStream *_stream;
	bool				_dispose;

private:
	int ppDecrunchBuffer(byte *src, byte *dest, uint32 src_len, uint32 dest_len) {

		byte *buf, *out, *dest_end, *off_lens, bits_left = 0, bit_cnt;
		uint32 bit_buffer = 0, x, todo, offbits, offset, written = 0;

		if (!src || !dest) return 0;

		/* set up input and output pointers */
		off_lens = src; src = &src[4];
		buf = &src[src_len];

		out = dest_end = &dest[dest_len];

		/* skip the first few bits */
		PP_READ_BITS(src[src_len + 3], x);

		/* while there are input bits left */
		while (written < dest_len) {
			PP_READ_BITS(1, x);
			if (x == 0) {
				  /* bit==0: literal, then match. bit==1: just match */
				  todo = 1; do { PP_READ_BITS(2, x); todo += x; } while (x == 3);
				  while (todo--) { PP_READ_BITS(8, x); PP_BYTE_OUT(x); }

				  /* should we end decoding on a literal, break out of the main loop */
				  if (written == dest_len) break;
			}

			/* match: read 2 bits for initial offset bitlength / match length */
			PP_READ_BITS(2, x);
			offbits = off_lens[x];
			todo = x+2;
			if (x == 3) {
				PP_READ_BITS(1, x);
				if (x == 0) offbits = 7;
				PP_READ_BITS(offbits, offset);
				do { PP_READ_BITS(3, x); todo += x; } while (x == 7);
			}
			else {
				PP_READ_BITS(offbits, offset);
			}
			if (&out[offset] >= dest_end) return 0; /* match_overflow */
			while (todo--) { x = out[offset]; PP_BYTE_OUT(x); }
		}

		/* all output bytes written without error */
		return 1;
	}

	uint16 getCrunchType(uint32 signature) {

		byte eff = 0;

		switch (signature) {
		case 0x50503230: /* PP20 */
			eff = 4;
			break;
		case 0x50504C53: /* PPLS */
			error("PPLS crunched files are not supported");
#if 0
			eff = 8;
			break;
#endif
		case 0x50583230: /* PX20 */
			error("PX20 crunched files are not supported");
#if 0
			eff = 6;
			break;
#endif
		default:
			eff = 0;

		}

		return eff;
	}

public:
	PowerPackerStream(Common::SeekableReadStream &stream) {

		_dispose = false;

		uint32 signature = stream.readUint32BE();
		if (getCrunchType(signature) == 0) {
			stream.seek(0, SEEK_SET);
			_stream = &stream;
			return;
		}

		stream.seek(-4, SEEK_END);
		uint32 decrlen = stream.readUint32BE() >> 8;
		byte *dest = (byte *)malloc(decrlen);

		uint32 crlen = stream.size() - 4;
		byte *src = (byte *)malloc(crlen);
		stream.seek(4, SEEK_SET);
		stream.read(src, crlen);

		ppDecrunchBuffer(src, dest, crlen-8, decrlen);

		free(src);
		_stream = new Common::MemoryReadStream(dest, decrlen, DisposeAfterUse::YES);
		_dispose = true;
	}

	~PowerPackerStream() {
		if (_dispose) delete _stream;
	}

	int32 size() const {
		return _stream->size();
	}

	int32 pos() const {
		return _stream->pos();
	}

	bool eos() const {
		return _stream->eos();
	}

	bool seek(int32 offs, int whence = SEEK_SET) {
		return _stream->seek(offs, whence);
	}

	uint32 read(void *dataPtr, uint32 dataSize) {
		return _stream->read(dataPtr, dataSize);
	}
};




AmigaDisk_ns::AmigaDisk_ns(Parallaction *vm) : Disk_ns(vm) {
}


AmigaDisk_ns::~AmigaDisk_ns() {

}

void AmigaDisk_ns::init() {
	// setup permament archives
	if (_vm->getFeatures() & GF_DEMO) {
		addArchive("disk0", LOW_ARCHIVE_PRIORITY);
	} else {
		addArchive("disk0", LOW_ARCHIVE_PRIORITY);
		addArchive("disk1", LOW_ARCHIVE_PRIORITY);
	}
}

#define NUM_PLANES		5

/*
	unpackFrame transforms images from 5-bitplanes format to
	8-bit color-index mode
*/
void AmigaDisk_ns::unpackFrame(byte *dst, byte *src, uint16 planeSize) {

	byte s0, s1, s2, s3, s4, mask, t0, t1, t2, t3, t4;

	for (uint32 j = 0; j < planeSize; j++) {
		s0 = src[j];
		s1 = src[j+planeSize];
		s2 = src[j+planeSize*2];
		s3 = src[j+planeSize*3];
		s4 = src[j+planeSize*4];

		for (uint32 k = 0; k < 8; k++) {
			mask = 1 << (7 - k);
			t0 = (s0 & mask ? 1 << 0 : 0);
			t1 = (s1 & mask ? 1 << 1 : 0);
			t2 = (s2 & mask ? 1 << 2 : 0);
			t3 = (s3 & mask ? 1 << 3 : 0);
			t4 = (s4 & mask ? 1 << 4 : 0);
			*dst++ = t0 | t1 | t2 | t3 | t4;
		}

	}

}

/*
	patchFrame applies DLTA data (dlta) to specified buffer (dst)
*/
void AmigaDisk_ns::patchFrame(byte *dst, byte *dlta, uint16 bytesPerPlane, uint16 height) {

	uint32 *dataIndex = (uint32 *)dlta;
	uint32 *ofslenIndex = (uint32 *)dlta + 8;

	uint16 *base = (uint16 *)dlta;
	uint16 wordsPerLine = bytesPerPlane >> 1;

	for (uint j = 0; j < NUM_PLANES; j++) {
		uint16 *dst16 = (uint16 *)(dst + j * bytesPerPlane * height);

		uint16 *data = base + READ_BE_UINT32(dataIndex);
		dataIndex++;
		uint16 *ofslen = base + READ_BE_UINT32(ofslenIndex);
		ofslenIndex++;

		while (*ofslen != 0xFFFF) {

			uint16 ofs = READ_BE_UINT16(ofslen);
			ofslen++;
			uint16 size = READ_BE_UINT16(ofslen);
			ofslen++;

			while (size > 0) {
				dst16[ofs] ^= *data++;
				ofs += wordsPerLine;
				size--;
			}

		}

	}

}

// FIXME: no mask is loaded
void AmigaDisk_ns::unpackBitmap(byte *dst, byte *src, uint16 numFrames, uint16 bytesPerPlane, uint16 height) {

	byte *baseFrame = src;
	byte *tempBuffer = 0;

	uint16 planeSize = bytesPerPlane * height;

	for (uint32 i = 0; i < numFrames; i++) {
		if (READ_BE_UINT32(src) == MKTAG('D','L','T','A')) {

			uint size = READ_BE_UINT32(src + 4);

			if (tempBuffer == 0)
				tempBuffer = (byte *)malloc(planeSize * NUM_PLANES);

			memcpy(tempBuffer, baseFrame, planeSize * NUM_PLANES);

			patchFrame(tempBuffer, src + 8, bytesPerPlane, height);
			unpackFrame(dst, tempBuffer, planeSize);

			src += (size + 8);
			dst += planeSize * 8;
		} else {
			unpackFrame(dst, src, planeSize);
			src += planeSize * NUM_PLANES;
			dst += planeSize * 8;
		}
	}

	free(tempBuffer);

}


void AmigaDisk_ns::decodeCnv(byte *data, uint16 numFrames, uint16 width, uint16 height, Common::SeekableReadStream *stream) {
	byte bytesPerPlane = width / 8;
	uint32 rawsize = numFrames * bytesPerPlane * NUM_PLANES * height;
	byte *buf = new byte[rawsize];
	assert(buf);
	stream->read(buf, rawsize);
	unpackBitmap(data, buf, numFrames, bytesPerPlane, height);
	delete[] buf;
}

#undef NUM_PLANES

Frames* AmigaDisk_ns::loadPointer(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadPointer");
	Common::SeekableReadStream *stream = openFile(name);
	return makeCnv(stream);
}

GfxObj* AmigaDisk_ns::loadStatic(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadStatic '%s'", name);
	Common::SeekableReadStream *s = openFile(name);
	return new GfxObj(0, makeCnv(s), name);
}

Common::SeekableReadStream *AmigaDisk_ns::tryOpenFile(const char* name) {
	debugC(3, kDebugDisk, "AmigaDisk_ns::tryOpenFile(%s)", name);

	PowerPackerStream *ret;
	Common::SeekableReadStream *stream = _sset.createReadStreamForMember(name);
	if (stream)
		return stream;

	char path[PATH_LEN];
	sprintf(path, "%s.pp", name);
	stream = _sset.createReadStreamForMember(path);
	if (stream) {
		ret = new PowerPackerStream(*stream);
		delete stream;
		return ret;
	}

	sprintf(path, "%s.dd", name);
	stream = _sset.createReadStreamForMember(path);
	if (stream) {
		ret = new PowerPackerStream(*stream);
		delete stream;
		return ret;
	}

	return 0;
}


/*
	FIXME: mask values are not computed correctly for level 1 and 2

	NOTE: this routine is only able to build masks for Nippon Safes, since mask widths are hardcoded
	into the main loop.
*/
void AmigaDisk_ns::buildMask(byte* buf) {

	byte mask1[16] = { 0, 0x80, 0x20, 0xA0, 8, 0x88, 0x28, 0xA8, 2, 0x82, 0x22, 0xA2, 0xA, 0x8A, 0x2A, 0xAA };
	byte mask0[16] = { 0, 0x40, 0x10, 0x50, 4, 0x44, 0x14, 0x54, 1, 0x41, 0x11, 0x51, 0x5, 0x45, 0x15, 0x55 };

	byte plane0[40];
	byte plane1[40];

	for (int32 i = 0; i < _vm->_screenHeight; i++) {

		memcpy(plane0, buf, 40);
		memcpy(plane1, buf+40, 40);

		for (uint32 j = 0; j < 40; j++) {
			*buf++ = mask0[(plane0[j] & 0xF0) >> 4] | mask1[(plane1[j] & 0xF0) >> 4];
			*buf++ = mask0[plane0[j] & 0xF] | mask1[plane1[j] & 0xF];
		}

	}
}


void AmigaDisk_ns::loadBackground(BackgroundInfo& info, const char *name) {
	PaletteFxRange ranges[6];
	byte pal[768];

	Common::SeekableReadStream *s = openFile(name);
	ILBMLoader loader(&info.bg, pal, ranges);
	loader.load(s, true);

	info.width = info.bg.w;
	info.height = info.bg.h;

	byte *p = pal;
	for (uint i = 0; i < 32; i++) {
		byte r = *p >> 2;
		p++;
		byte g = *p >> 2;
		p++;
		byte b = *p >> 2;
		p++;
		info.palette.setEntry(i, r, g, b);
	}

	for (uint j = 0; j < 6; j++) {
		info.setPaletteRange(j, ranges[j]);
	}
}

void AmigaDisk_ns::loadMask_internal(BackgroundInfo& info, const char *name) {
	debugC(5, kDebugDisk, "AmigaDisk_ns::loadMask_internal(%s)", name);

	char path[PATH_LEN];
	sprintf(path, "%s.mask", name);

	Common::SeekableReadStream *s = tryOpenFile(path);
	if (!s) {
		debugC(5, kDebugDisk, "Mask file not found");
		return;	// no errors if missing mask files: not every location has one
	}

	byte pal[768];
	ILBMLoader loader(ILBMLoader::BODYMODE_MASKBUFFER, pal);
	loader.load(s, true);

	byte r, g, b;
	for (uint i = 0; i < 4; i++) {
		r = pal[i*3];
		g = pal[i*3+1];
		b = pal[i*3+2];
		info.layers[i] = (((r << 4) & 0xF00) | (g & 0xF0) | (b >> 4)) & 0xFF;
	}

	info._mask = loader._maskBuffer;
}

void AmigaDisk_ns::loadPath_internal(BackgroundInfo& info, const char *name) {

	char path[PATH_LEN];
	sprintf(path, "%s.path", name);

	Common::SeekableReadStream *s = tryOpenFile(path);
	if (!s) {
		return;	// no errors if missing path files: not every location has one
	}

	ILBMLoader loader(ILBMLoader::BODYMODE_PATHBUFFER);
	loader.load(s, true);
	info._path = loader._pathBuffer;
	info._path->bigEndian = true;
}

void AmigaDisk_ns::loadScenery(BackgroundInfo& info, const char* background, const char* mask, const char* path) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadScenery '%s', '%s'", background, mask);

	char filename[PATH_LEN];
	sprintf(filename, "%s.bkgnd", background);

	loadBackground(info, filename);

	if (mask == 0) {
		loadMask_internal(info, background);
		loadPath_internal(info, background);
	} else {
		loadMask_internal(info, mask);
		loadPath_internal(info, mask);
	}

	return;
}

void AmigaDisk_ns::loadSlide(BackgroundInfo& info, const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadSlide '%s'", name);
	loadBackground(info, name);
	return;
}

Frames* AmigaDisk_ns::loadFrames(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadFrames '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "anims/%s", name);

	Common::SeekableReadStream *s = tryOpenFile(path);
	if (!s)
		s = openFile(name);

	return makeCnv(s);
}

GfxObj* AmigaDisk_ns::loadHead(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadHead '%s'", name);
	char path[PATH_LEN];
	sprintf(path, "%s.head", name);
	Common::SeekableReadStream *s = openFile(path);
	return new GfxObj(0, makeCnv(s), name);
}


GfxObj* AmigaDisk_ns::loadObjects(const char *name, uint8 part) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadObjects");

	char path[PATH_LEN];
	if (_vm->getFeatures() & GF_DEMO)
		sprintf(path, "%s.objs", name);
	else
		sprintf(path, "objs/%s.objs", name);

	Common::SeekableReadStream *s = openFile(path);
	return new GfxObj(0, makeCnv(s), name);
}


GfxObj* AmigaDisk_ns::loadTalk(const char *name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadTalk '%s'", name);

	char path[PATH_LEN];
	if (_vm->getFeatures() & GF_DEMO)
		sprintf(path, "%s.talk", name);
	else
		sprintf(path, "talk/%s.talk", name);

	Common::SeekableReadStream *s = tryOpenFile(path);
	if (!s) {
		s = openFile(name);
	}
	return new GfxObj(0, makeCnv(s), name);
}

Table* AmigaDisk_ns::loadTable(const char* name) {
	debugC(1, kDebugDisk, "AmigaDisk_ns::loadTable '%s'", name);

	char path[PATH_LEN];
	if (!scumm_stricmp(name, "global")) {
		sprintf(path, "%s.table", name);
	} else {
		if (!(_vm->getFeatures() & GF_DEMO))
			sprintf(path, "objs/%s.table", name);
		else
			sprintf(path, "%s.table", name);
	}

	return createTableFromStream(100, openFile(path));
}

Font* AmigaDisk_ns::loadFont(const char* name) {
	debugC(1, kDebugDisk, "AmigaFullDisk::loadFont '%s'", name);

	char path[PATH_LEN];
	sprintf(path, "%sfont", name);

	Common::SeekableReadStream *stream = openFile(path);
	Font *font = createFont(name, *stream);
	delete stream;

	return font;
}


Common::SeekableReadStream* AmigaDisk_ns::loadMusic(const char* name) {
	return tryOpenFile(name);
}

Common::SeekableReadStream* AmigaDisk_ns::loadSound(const char* name) {
	char path[PATH_LEN];
	sprintf(path, "%s.snd", name);

	return tryOpenFile(path);
}

} // End of namespace Parallaction
