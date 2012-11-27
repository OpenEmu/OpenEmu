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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "scumm/file.h"

#include "scumm/scumm.h"

#include "common/memstream.h"
#include "common/substream.h"

namespace Scumm {

#pragma mark -
#pragma mark --- ScummFile ---
#pragma mark -

ScummFile::ScummFile() : _subFileStart(0), _subFileLen(0) {
}

void ScummFile::setSubfileRange(int32 start, int32 len) {
	// TODO: Add sanity checks
	const int32 fileSize = File::size();
	assert(start <= fileSize);
	assert(start + len <= fileSize);
	_subFileStart = start;
	_subFileLen = len;
	seek(0, SEEK_SET);
}

void ScummFile::resetSubfile() {
	_subFileStart = 0;
	_subFileLen = 0;
	seek(0, SEEK_SET);
}

bool ScummFile::open(const Common::String &filename) {
	if (File::open(filename)) {
		resetSubfile();
		return true;
	} else {
		return false;
	}
}

bool ScummFile::openSubFile(const Common::String &filename) {
	assert(isOpen());

	// Disable the XOR encryption and reset any current subfile range
	setEnc(0);
	resetSubfile();

	// Read in the filename table and look for the specified file

	unsigned long file_off, file_len;
	char file_name[0x20+1];
	unsigned long i;

	// Get the length of the data file to use for consistency checks
	const uint32 data_file_len = size();

	// Read offset and length to the file records */
	const uint32 file_record_off = readUint32BE();
	const uint32 file_record_len = readUint32BE();

	// Do a quick check to make sure the offset and length are good
	if (file_record_off + file_record_len > data_file_len) {
		return false;
	}

	// Do a little consistancy check on file_record_length
	if (file_record_len % 0x28) {
		return false;
	}

	// Scan through the files
	for (i = 0; i < file_record_len; i += 0x28) {
		// read a file record
		seek(file_record_off + i, SEEK_SET);
		file_off = readUint32BE();
		file_len = readUint32BE();
		read(file_name, 0x20);
		file_name[0x20] = 0;

		assert(file_name[0]);
		//debug(7, "  extracting \'%s\'", file_name);

		// Consistency check. make sure the file data is in the file
		if (file_off + file_len > data_file_len) {
			return false;
		}

		if (scumm_stricmp(file_name, filename.c_str()) == 0) {
			// We got a match!
			setSubfileRange(file_off, file_len);
			return true;
		}
	}

	return false;
}


bool ScummFile::eos() const {
	return _subFileLen ? _myEos : File::eos();
}

int32 ScummFile::pos() const {
	return File::pos() - _subFileStart;
}

int32 ScummFile::size() const {
	return _subFileLen ? _subFileLen : File::size();
}

bool ScummFile::seek(int32 offs, int whence) {
	if (_subFileLen) {
		// Constrain the seek to the subfile
		switch (whence) {
		case SEEK_END:
			offs = _subFileStart + _subFileLen + offs;
			break;
		case SEEK_SET:
			offs += _subFileStart;
			break;
		case SEEK_CUR:
			offs += File::pos();
			break;
		}
		assert((int32)_subFileStart <= offs && offs <= (int32)(_subFileStart + _subFileLen));
		whence = SEEK_SET;
	}
	bool ret = File::seek(offs, whence);
	if (ret)
		_myEos = false;
	return ret;
}

uint32 ScummFile::read(void *dataPtr, uint32 dataSize) {
	uint32 realLen;

	if (_subFileLen) {
		// Limit the amount we read by the subfile boundaries.
		const int32 curPos = pos();
		assert(_subFileLen >= curPos);
		int32 newPos = curPos + dataSize;
		if (newPos > _subFileLen) {
			dataSize = _subFileLen - curPos;
			_myEos = true;
		}
	}

	realLen = File::read(dataPtr, dataSize);


	// If an encryption byte was specified, XOR the data we just read by it.
	// This simple kind of "encryption" was used by some of the older SCUMM
	// games.
	if (_encbyte) {
		byte *p = (byte *)dataPtr;
		byte *end = p + realLen;
		while (p < end)
			*p++ ^= _encbyte;
	}

	return realLen;
}

#pragma mark -
#pragma mark --- ScummDiskImage ---
#pragma mark -

static const int maniacResourcesPerFile[55] = {
	 0, 11,  1,  3,  9, 12,  1, 13, 10,  6,
	 4,  1,  7,  1,  1,  2,  7,  8, 19,  9,
	 6,  9,  2,  6,  8,  4, 16,  8,  3,  3,
	12, 12,  2,  8,  1,  1,  2,  1,  9,  1,
	 3,  7,  3,  3, 13,  5,  4,  3,  1,  1,
	 3, 10,  1,  0,  0
};

static const int zakResourcesPerFile[59] = {
	 0, 29, 12, 14, 13,  4,  4, 10,  7,  4,
	14, 19,  5,  4,  7,  6, 11,  9,  4,  4,
	 1,  3,  3,  5,  1,  9,  4, 10, 13,  6,
	 7, 10,  2,  6,  1, 11,  2,  5,  7,  1,
	 7,  1,  4,  2,  8,  6,  6,  6,  4, 13,
	 3,  1,  2,  1,  2,  1, 10,  1,  1
};


static uint16 write_byte(Common::WriteStream *out, byte val) {
	val ^= 0xFF;
	if (out != 0)
		out->writeByte(val);
	return 1;
}

static uint16 write_word(Common::WriteStream *out, uint16 val) {
	val ^= 0xFFFF;
	if (out != 0)
		out->writeUint16LE(val);
	return 2;
}

ScummDiskImage::ScummDiskImage(const char *disk1, const char *disk2, GameSettings game)
	: _stream(0), _buf(0), _game(game),
	_disk1(disk1), _disk2(disk2), _openedDisk(0) {

	if (_game.id == GID_MANIAC) {
		_numGlobalObjects = 256;
		_numRooms = 55;
		_numCostumes = 25;
		_numScripts = 160;
		_numSounds = 70;
		_resourcesPerFile = maniacResourcesPerFile;
	} else {
		_numGlobalObjects = 775;
		_numRooms = 59;
		_numCostumes = 38;
		_numScripts = 155;
		_numSounds = 127;
		_resourcesPerFile = zakResourcesPerFile;
	}
}

byte ScummDiskImage::fileReadByte() {
	byte b = 0;
	File::read(&b, 1);
	return b;
}

uint16 ScummDiskImage::fileReadUint16LE() {
	uint16 a = fileReadByte();
	uint16 b = fileReadByte();
	return a | (b << 8);
}

bool ScummDiskImage::openDisk(char num) {
	if (num == '1')
		num = 1;
	if (num == '2')
		num = 2;

	if (_openedDisk != num || !File::isOpen()) {
		if (File::isOpen())
			File::close();

		if (num == 1)
			File::open(_disk1);
		else if (num == 2)
			File::open(_disk2);
		else {
			error("ScummDiskImage::open(): wrong disk (%c)", num);
			return false;
		}

		_openedDisk = num;

		if (!File::isOpen()) {
			error("ScummDiskImage::open(): cannot open disk (%d)", num);
			return false;
		}
	}
	return true;
}

bool ScummDiskImage::open(const Common::String &filename) {
	uint16 signature;

	// check signature
	openDisk(1);

	if (_game.platform == Common::kPlatformApple2GS) {
		File::seek(142080);
	} else {
		File::seek(0);
	}

	signature = fileReadUint16LE();
	if (signature != 0x0A31) {
		error("ScummDiskImage::open(): signature not found in disk 1");
		return false;
	}

	extractIndex(0); // Fill in resource arrays

	openDisk(2);

	if (_game.platform == Common::kPlatformApple2GS) {
		File::seek(143104);
		signature = fileReadUint16LE();
		if (signature != 0x0032)
			error("Error: signature not found in disk 2");
	} else {
		File::seek(0);
		signature = fileReadUint16LE();
		if (signature != 0x0132)
			error("Error: signature not found in disk 2");
	}


	return true;
}


uint16 ScummDiskImage::extractIndex(Common::WriteStream *out) {
	int i;
	uint16 reslen = 0;

	openDisk(1);

	if (_game.platform == Common::kPlatformApple2GS) {
		File::seek(142080);
	} else {
		File::seek(0);
	}

	// skip signature
	fileReadUint16LE();

	// write expected signature
	if (_game.platform == Common::kPlatformApple2GS) {
		reslen += write_word(out, 0x0032);
	} else {
		reslen += write_word(out, 0x0132);
	}

	// copy object flags
	for (i = 0; i < _numGlobalObjects; i++)
		reslen += write_byte(out, fileReadByte());

	// copy room offsets
	for (i = 0; i < _numRooms; i++) {
		_roomDisks[i] = fileReadByte();
		reslen += write_byte(out, _roomDisks[i]);
	}
	for (i = 0; i < _numRooms; i++) {
		_roomSectors[i] = fileReadByte();
		reslen += write_byte(out, _roomSectors[i]);
		_roomTracks[i] = fileReadByte();
		reslen += write_byte(out, _roomTracks[i]);
	}
	for (i = 0; i < _numCostumes; i++)
		reslen += write_byte(out, fileReadByte());
	for (i = 0; i < _numCostumes; i++)
		reslen += write_word(out, fileReadUint16LE());

	for (i = 0; i < _numScripts; i++)
		reslen += write_byte(out, fileReadByte());
	for (i = 0; i < _numScripts; i++)
		reslen += write_word(out, fileReadUint16LE());

	for (i = 0; i < _numSounds; i++)
		reslen += write_byte(out, fileReadByte());
	for (i = 0; i < _numSounds; i++)
		reslen += write_word(out, fileReadUint16LE());

	return reslen;
}

bool ScummDiskImage::generateIndex() {
	int bufsize;

	bufsize = extractIndex(0);

	free(_buf);
	_buf = (byte *)calloc(1, bufsize);

	Common::MemoryWriteStream out(_buf, bufsize);

	extractIndex(&out);

	delete _stream;
	_stream = new Common::MemoryReadStream(_buf, bufsize);

	return true;
}

uint16 ScummDiskImage::extractResource(Common::WriteStream *out, int res) {
	const int AppleSectorOffset[36] = {
		0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 256,
		272, 288, 304, 320, 336, 352, 368,
		384, 400, 416, 432, 448, 464,
		480, 496, 512, 528, 544, 560
	};
	const int C64SectorOffset[36] = {
		0,
		0, 21, 42, 63, 84, 105, 126, 147, 168, 189, 210, 231, 252, 273, 294, 315, 336,
		357, 376, 395, 414, 433, 452, 471,
		490, 508, 526, 544, 562, 580,
		598, 615, 632, 649, 666
	};
	int i;
	uint16 reslen = 0;

	openDisk(_roomDisks[res]);

	if (_game.platform == Common::kPlatformApple2GS) {
		File::seek((AppleSectorOffset[_roomTracks[res]] + _roomSectors[res]) * 256);
	} else {
		File::seek((C64SectorOffset[_roomTracks[res]] + _roomSectors[res]) * 256);
	}

	for (i = 0; i < _resourcesPerFile[res]; i++) {
		uint16 len;
		do {
			// Note: len might be 0xFFFF for padding in zak-c64-german
			len = fileReadUint16LE();
			reslen += write_word(out, len);
		} while (len == 0xFFFF);

		for (len -= 2; len > 0; len--)
			reslen += write_byte(out, fileReadByte());
	}

	return reslen;
}

bool ScummDiskImage::generateResource(int res) {
	int bufsize;

	if (res >= _numRooms)
		return false;

	bufsize = extractResource(0, res);

	free(_buf);
	_buf = (byte *)calloc(1, bufsize);

	Common::MemoryWriteStream out(_buf, bufsize);

	extractResource(&out, res);

	delete _stream;
	_stream = new Common::MemoryReadStream(_buf, bufsize);

	return true;
}

void ScummDiskImage::close() {
	delete _stream;
	_stream = 0;

	free(_buf);
	_buf = 0;

	File::close();
}

bool ScummDiskImage::openSubFile(const Common::String &filename) {
	assert(isOpen());

	const char *ext = strrchr(filename.c_str(), '.');
	char resNum[3];
	int res;

	// We always have file name in form of XX.lfl
	resNum[0] = ext[-2];
	resNum[1] = ext[-1];
	resNum[2] = 0;

	res = atoi(resNum);

	if (res == 0) {
		return generateIndex();
	} else {
		return generateResource(res);
	}

	return true;
}

uint32 ScummDiskImage::read(void *dataPtr, uint32 dataSize) {
	uint32 realLen = _stream->read(dataPtr, dataSize);

	if (_encbyte) {
		byte *p = (byte *)dataPtr;
		byte *end = p + realLen;
		while (p < end)
			*p++ ^= _encbyte;
	}

	return realLen;
}

} // End of namespace Scumm
