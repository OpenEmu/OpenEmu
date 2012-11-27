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

#include "kyra/resource_intern.h"
#include "kyra/resource.h"

#include "common/endian.h"
#include "common/memstream.h"
#include "common/substream.h"

namespace Kyra {

// Implementation of various Archive subclasses

// -> PlainArchive implementation

PlainArchive::PlainArchive(Common::ArchiveMemberPtr file)
	: _file(file), _files() {
}

bool PlainArchive::hasFile(const Common::String &name) const {
	return (_files.find(name) != _files.end());
}

int PlainArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (FileMap::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, this)));
		++count;
	}

	return count;
}

const Common::ArchiveMemberPtr PlainArchive::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *PlainArchive::createReadStreamForMember(const Common::String &name) const {
	FileMap::const_iterator fDesc = _files.find(name);
	if (fDesc == _files.end())
		return 0;

	Common::SeekableReadStream *parent = _file->createReadStream();
	if (!parent)
		return 0;

	return new Common::SeekableSubReadStream(parent, fDesc->_value.offset, fDesc->_value.offset + fDesc->_value.size, DisposeAfterUse::YES);
}

void PlainArchive::addFileEntry(const Common::String &name, const Entry entry) {
	_files[name] = entry;
}

PlainArchive::Entry PlainArchive::getFileEntry(const Common::String &name) const {
	FileMap::const_iterator fDesc = _files.find(name);
	if (fDesc == _files.end())
		return Entry();
	return fDesc->_value;
}

// -> TlkArchive implementation

TlkArchive::TlkArchive(Common::ArchiveMemberPtr file, uint16 entryCount, const uint32 *fileEntries)
	: _file(file), _entryCount(entryCount), _fileEntries(fileEntries) {
}

TlkArchive::~TlkArchive() {
	delete[] _fileEntries;
}

bool TlkArchive::hasFile(const Common::String &name) const {
	return (findFile(name) != 0);
}

int TlkArchive::listMembers(Common::ArchiveMemberList &list) const {
	uint count = 0;

	for (; count < _entryCount; ++count) {
		const Common::String name = Common::String::format("%08u.AUD", _fileEntries[count * 2 + 0]);
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(name, this)));
	}

	return count;
}

const Common::ArchiveMemberPtr TlkArchive::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *TlkArchive::createReadStreamForMember(const Common::String &name) const {
	const uint32 *fileDesc = findFile(name);
	if (!fileDesc)
		return 0;

	Common::SeekableReadStream *parent = _file->createReadStream();
	if (!parent)
		return 0;

	parent->seek(fileDesc[1], SEEK_SET);
	const uint32 size = parent->readUint32LE();
	const uint32 fileStart = fileDesc[1] + 4;

	return new Common::SeekableSubReadStream(parent, fileStart, fileStart + size, DisposeAfterUse::YES);
}

const uint32 *TlkArchive::findFile(const Common::String &name) const {
	Common::String uppercaseName = name;
	uppercaseName.toUppercase();

	if (!uppercaseName.hasSuffix(".AUD"))
		return 0;

	uint32 id;
	if (sscanf(uppercaseName.c_str(), "%08u.AUD", &id) != 1)
		return 0;

	// Binary search for the file entry
	int leftIndex = 0;
	int rightIndex = _entryCount - 1;

	while (leftIndex <= rightIndex) {
		int mid = (leftIndex + rightIndex) / 2;

		const uint32 key = _fileEntries[mid * 2];
		if (key == id) {
			// Found
			return &_fileEntries[mid * 2];
		} else if (key > id) {
			// Take the left sub-tree
			rightIndex = mid - 1;
		} else {
			// Take the right sub-tree
			leftIndex = mid + 1;
		}
	}

	return 0;
}

// -> CachedArchive implementation

CachedArchive::CachedArchive(const FileInputList &files)
	: _files() {
	for (FileInputList::const_iterator i = files.begin(); i != files.end(); ++i) {
		Entry entry;

		entry.data = i->data;
		entry.size = i->size;

		Common::String name = i->name;
		name.toLowercase();
		_files[name] = entry;
	}
}

CachedArchive::~CachedArchive() {
	for (FileMap::iterator i = _files.begin(); i != _files.end(); ++i)
		delete[] i->_value.data;
	_files.clear();
}

bool CachedArchive::hasFile(const Common::String &name) const {
	return (_files.find(name) != _files.end());
}

int CachedArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (FileMap::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		list.push_back(Common::ArchiveMemberList::value_type(new Common::GenericArchiveMember(i->_key, this)));
		++count;
	}

	return count;
}

const Common::ArchiveMemberPtr CachedArchive::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *CachedArchive::createReadStreamForMember(const Common::String &name) const {
	FileMap::const_iterator fDesc = _files.find(name);
	if (fDesc == _files.end())
		return 0;

	return new Common::MemoryReadStream(fDesc->_value.data, fDesc->_value.size, DisposeAfterUse::NO);
}

// ResFileLoader implementations

// -> ResLoaderPak implementation

bool ResLoaderPak::checkFilename(Common::String filename) const {
	filename.toUppercase();
	return (filename.hasSuffix(".PAK") || filename.hasSuffix(".APK") || filename.hasSuffix(".VRM") || filename.hasSuffix(".CMP") || filename.hasSuffix(".TLK") || filename.equalsIgnoreCase(StaticResource::staticDataFilename()));
}

namespace {

Common::String readString(Common::SeekableReadStream &stream) {
	Common::String result;
	char c = 0;

	while ((c = stream.readByte()) != 0)
		result += c;

	return result;
}

} // end of anonymous namespace

bool ResLoaderPak::isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const {
	int32 filesize = stream.size();
	if (filesize < 0)
		return false;

	int32 offset = 0;
	bool switchEndian = false;
	bool firstFile = true;

	offset = stream.readUint32LE();
	if (offset > filesize || offset < 0) {
		switchEndian = true;
		offset = SWAP_BYTES_32(offset);
	}

	int32 firstOffset = offset;

	Common::String file;
	while (!stream.eos()) {
		// The start offset of a file should never be in the filelist
		if (offset < stream.pos() || offset > filesize || offset < 0)
			return false;

		file = readString(stream);

		if (stream.eos())
			return false;

		// Quit now if we encounter an empty string
		if (file.empty()) {
			if (firstFile)
				return false;
			else
				break;
		}

		firstFile = false;
		offset = switchEndian ? stream.readUint32BE() : stream.readUint32LE();

		if (!offset || offset == filesize || firstOffset == stream.pos())
			break;
	}

	return true;
}

Common::Archive *ResLoaderPak::load(Common::ArchiveMemberPtr memberFile, Common::SeekableReadStream &stream) const {
	int32 filesize = stream.size();
	if (filesize < 0)
		return 0;

	Common::ScopedPtr<PlainArchive> result(new PlainArchive(memberFile));
	if (!result)
		return 0;

	int32 startoffset = 0, endoffset = 0;
	bool switchEndian = false;
	bool firstFile = true;

	startoffset = stream.readUint32LE();
	int32 firstOffset = startoffset;
	if (startoffset > filesize || startoffset < 0) {
		switchEndian = true;
		startoffset = SWAP_BYTES_32(startoffset);
	}

	Common::String file;
	while (!stream.eos()) {
		// The start offset of a file should never be in the filelist
		if (startoffset < stream.pos() || startoffset > filesize || startoffset < 0) {
			warning("PAK file '%s' is corrupted", memberFile->getDisplayName().c_str());
			return 0;
		}

		file = readString(stream);

		if (stream.eos()) {
			warning("PAK file '%s' is corrupted", memberFile->getDisplayName().c_str());
			return 0;
		}

		// Quit now if we encounter an empty string
		if (file.empty()) {
			if (firstFile) {
				warning("PAK file '%s' is corrupted", memberFile->getDisplayName().c_str());
				return 0;
			} else {
				break;
			}
		}

		firstFile = false;
		endoffset = switchEndian ? stream.readUint32BE() : stream.readUint32LE();

		if (endoffset < 0 && stream.pos() != firstOffset) {
			warning("PAK file '%s' is corrupted", memberFile->getDisplayName().c_str());
			return 0;
		}

		if (!endoffset || stream.pos() == firstOffset)
			endoffset = filesize;

		if (startoffset != endoffset)
			result->addFileEntry(file, PlainArchive::Entry(startoffset, endoffset - startoffset));

		if (endoffset == filesize)
			break;

		startoffset = endoffset;
	}

	PlainArchive::Entry linklistFile = result->getFileEntry("LINKLIST");
	if (linklistFile.size != 0) {
		stream.seek(linklistFile.offset, SEEK_SET);

		const uint32 magic = stream.readUint32BE();

		if (magic != MKTAG('S', 'C', 'V', 'M'))
			error("LINKLIST file does not contain 'SCVM' header");

		const uint32 links = stream.readUint32BE();
		for (uint32 i = 0; i < links; ++i) {
			const Common::String linksTo = readString(stream);
			const uint32 sources = stream.readUint32BE();

			PlainArchive::Entry destination = result->getFileEntry(linksTo);
			if (destination.size == 0)
				error("PAK file link destination '%s' not found", linksTo.c_str());

			for (uint32 j = 0; j < sources; ++j) {
				const Common::String dest = readString(stream);
				result->addFileEntry(dest, destination);
			}
		}
	}

	return result.release();
}

// -> ResLoaderInsMalcolm implementation

bool ResLoaderInsMalcolm::checkFilename(Common::String filename) const {
	filename.toUppercase();
	if (!filename.hasSuffix(".001"))
		return false;
	return true;
}

bool ResLoaderInsMalcolm::isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const {
	stream.seek(3, SEEK_SET);
	int32 size = stream.readUint32LE();

	if (size + 7 > stream.size())
		return false;

	stream.seek(size + 5, SEEK_SET);
	uint8 buffer[2];
	stream.read(&buffer, 2);

	return (buffer[0] == 0x0D && buffer[1] == 0x0A);
}

Common::Archive *ResLoaderInsMalcolm::load(Common::ArchiveMemberPtr memberFile, Common::SeekableReadStream &stream) const {
	Common::List<Common::String> filenames;
	Common::ScopedPtr<PlainArchive> result(new PlainArchive(memberFile));
	if (!result)
		return 0;

	// thanks to eriktorbjorn for this code (a bit modified though)
	stream.seek(3, SEEK_SET);

	// first file is the index table
	uint32 size = stream.readUint32LE();
	Common::String temp;

	for (uint32 i = 0; i < size; ++i) {
		byte c = stream.readByte();

		if (c == '\\') {
			temp.clear();
		} else if (c == 0x0D) {
			// line endings are CRLF
			c = stream.readByte();
			assert(c == 0x0A);
			++i;

			filenames.push_back(temp);
		} else {
			temp += (char)c;
		}
	}

	stream.seek(3, SEEK_SET);

	for (Common::List<Common::String>::iterator file = filenames.begin(); file != filenames.end(); ++file) {
		const uint32 fileSize = stream.readUint32LE();
		const uint32 fileOffset = stream.pos();

		result->addFileEntry(*file, PlainArchive::Entry(fileOffset, fileSize));
		stream.seek(fileSize, SEEK_CUR);
	}

	return result.release();
}

bool ResLoaderTlk::checkFilename(Common::String filename) const {
	filename.toUppercase();
	return (filename.hasSuffix(".TLK"));
}

bool ResLoaderTlk::isLoadable(const Common::String &filename, Common::SeekableReadStream &stream) const {
	uint16 entries = stream.readUint16LE();
	int32 entryTableSize = (entries * 8);

	if (entryTableSize + 2 > stream.size())
		return false;

	int32 offset = 0;

	for (uint i = 0; i < entries; ++i) {
		stream.readUint32LE();
		offset = stream.readUint32LE();

		if (offset > stream.size())
			return false;
	}

	return true;
}

Common::Archive *ResLoaderTlk::load(Common::ArchiveMemberPtr file, Common::SeekableReadStream &stream) const {
	const uint16 entryCount = stream.readUint16LE();

	uint32 *fileEntries = new uint32[entryCount * 2];
	assert(fileEntries);
	stream.read(fileEntries, sizeof(uint32) * entryCount * 2);

	for (uint i = 0; i < entryCount; ++i) {
		fileEntries[i * 2 + 0] = READ_LE_UINT32(&fileEntries[i * 2 + 0]);
		fileEntries[i * 2 + 1] = READ_LE_UINT32(&fileEntries[i * 2 + 1]);
	}


	return new TlkArchive(file, entryCount, fileEntries);
}

// InstallerLoader implementation

class FileExpanderSource {
public:
	FileExpanderSource(const uint8 *data, int dataSize) : _dataPtr(data), _endofBuffer(data + dataSize), _bitsLeft(8), _key(0), _index(0) {}
	~FileExpanderSource() {}

	void advSrcRefresh();
	void advSrcBitsBy1();
	void advSrcBitsByIndex(uint8 newIndex);

	uint8 getKeyLower() const { return _key & 0xff; }
	void setIndex(uint8 index) { _index = index; }
	uint16 getKeyMasked(uint8 newIndex);
	uint16 keyMaskedAlign(uint16 val);

	void copyBytes(uint8 *& dst);

private:
	const uint8 *_dataPtr;
	const uint8 *_endofBuffer;
	uint16 _key;
	int8 _bitsLeft;
	uint8 _index;
};

void FileExpanderSource::advSrcBitsBy1() {
	_key >>= 1;
	if (!--_bitsLeft) {
		if (_dataPtr < _endofBuffer)
			_key = ((*_dataPtr++) << 8) | (_key & 0xff);
		_bitsLeft = 8;
	}
}

void FileExpanderSource::advSrcBitsByIndex(uint8 newIndex) {
	_index = newIndex;
	_bitsLeft -= _index;
	if (_bitsLeft <= 0) {
		_key >>= (_index + _bitsLeft);
		_index = -_bitsLeft;
		_bitsLeft = 8 - _index;
		if (_dataPtr < _endofBuffer)
			_key = (*_dataPtr++ << 8) | (_key & 0xff);
	}
	_key >>= _index;
}

uint16 FileExpanderSource::getKeyMasked(uint8 newIndex) {
	static const uint8 mskTable[] = { 0x0F, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };
	_index = newIndex;
	uint16 res = 0;

	if (_index > 8) {
		newIndex = _index - 8;
		res = (_key & 0xff) & mskTable[8];
		advSrcBitsByIndex(8);
		_index = newIndex;
		res |= (((_key & 0xff) & mskTable[_index]) << 8);
		advSrcBitsByIndex(_index);
	} else {
		res = (_key & 0xff) & mskTable[_index];
		advSrcBitsByIndex(_index);
	}

	return res;
}

void FileExpanderSource::copyBytes(uint8 *& dst) {
	advSrcBitsByIndex(_bitsLeft);
	uint16 r = (READ_LE_UINT16(_dataPtr) ^ _key) + 1;
	_dataPtr += 2;

	if (r)
		error("decompression failure");

	memcpy(dst, _dataPtr, _key);
	_dataPtr += _key;
	dst += _key;
}

uint16 FileExpanderSource::keyMaskedAlign(uint16 val) {
	val -= 0x101;
	_index = (val & 0xff) >> 2;
	int16 b = ((_bitsLeft << 8) | _index) - 1;
	_bitsLeft = b >> 8;
	_index = b & 0xff;
	uint16 res = (((val & 3) + 4) << _index) + 0x101;
	return res + getKeyMasked(_index);
}

void FileExpanderSource::advSrcRefresh() {
	_key = READ_LE_UINT16(_dataPtr);
	if (_dataPtr < _endofBuffer - 1)
		_dataPtr += 2;
	_bitsLeft = 8;
}

class FileExpander {
public:
	FileExpander();
	~FileExpander();

	bool process(uint8 *dst, const uint8 *src, uint32 outsize, uint32 insize);

private:
	void generateTables(uint8 srcIndex, uint8 dstIndex, uint8 dstIndex2, int cnt);
	uint8 calcCmdAndIndex(const uint8 *tbl, int16 &para);

	FileExpanderSource *_src;
	uint8 *_tables[9];
	uint16 *_tables16[3];
};

FileExpander::FileExpander() : _src(0) {
	_tables[0] = new uint8[3914];
	assert(_tables[0]);

	_tables[1] = _tables[0] + 320;
	_tables[2] = _tables[0] + 352;
	_tables[3] = _tables[0] + 864;
	_tables[4] = _tables[0] + 2016;
	_tables[5] = _tables[0] + 2528;
	_tables[6] = _tables[0] + 2656;
	_tables[7] = _tables[0] + 2736;
	_tables[8] = _tables[0] + 2756;

	_tables16[0] = (uint16 *)(_tables[0] + 3268);
	_tables16[1] = (uint16 *)(_tables[0] + 3302);
	_tables16[2] = (uint16 *)(_tables[0] + 3338);
}

FileExpander::~FileExpander() {
	delete _src;
	delete[] _tables[0];
}

bool FileExpander::process(uint8 *dst, const uint8 *src, uint32 outsize, uint32 compressedSize) {
	static const uint8 indexTable[] = {
		0x10, 0x11, 0x12, 0x00, 0x08, 0x07, 0x09, 0x06, 0x0A,
		0x05, 0x0B, 0x04, 0x0C, 0x03, 0x0D, 0x02, 0x0E, 0x01, 0x0F
	};

	memset(_tables[0], 0, 3914);

	uint8 *d = dst;
	uint16 tableSize0 = 0;
	uint16 tableSize1 = 0;
	bool needrefresh = true;
	bool postprocess = false;

	_src = new FileExpanderSource(src, compressedSize);

	while (d < dst + outsize) {

		if (needrefresh) {
			needrefresh = false;
			_src->advSrcRefresh();
		}

		_src->advSrcBitsBy1();

		int mode = _src->getKeyMasked(2) - 1;
		if (mode == 1) {
			tableSize0 = _src->getKeyMasked(5) + 257;
			tableSize1 = _src->getKeyMasked(5) + 1;
			memset(_tables[7], 0, 19);

			const uint8 *itbl = indexTable;
			int numbytes = _src->getKeyMasked(4) + 4;

			while (numbytes--)
				_tables[7][*itbl++] = _src->getKeyMasked(3);

			generateTables(7, 8, 255, 19);

			int cnt = tableSize0 + tableSize1;
			uint8 *tmp = _tables[0];

			while (cnt) {
				uint16 cmd = _src->getKeyLower();
				cmd = READ_LE_UINT16(&_tables[8][cmd << 1]);
				_src->advSrcBitsByIndex(_tables[7][cmd]);

				if (cmd < 16) {
					*tmp++ = cmd;
					cnt--;
				} else {
					uint8 tmpI = 0;
					if (cmd == 16) {
						cmd = _src->getKeyMasked(2) + 3;
						tmpI = *(tmp - 1);
					} else if (cmd == 17) {
						cmd = _src->getKeyMasked(3) + 3;
					} else {
						cmd = _src->getKeyMasked(7) + 11;
					}
					_src->setIndex(tmpI);
					memset(tmp, tmpI, cmd);
					tmp += cmd;

					cnt -= cmd;
					if (cnt < 0)
						error("decompression failure");
				}
			}

			memcpy(_tables[1], _tables[0] + tableSize0, tableSize1);
			generateTables(0, 2, 3, tableSize0);
			generateTables(1, 4, 5, tableSize1);
			postprocess = true;
		} else if (mode < 0) {
			_src->copyBytes(d);
			postprocess = false;
			needrefresh = true;
		} else if (mode == 0) {
			uint8 *d2 = _tables[0];
			memset(d2, 8, 144);
			memset(d2 + 144, 9, 112);
			memset(d2 + 256, 7, 24);
			memset(d2 + 280, 8, 8);
			d2 = _tables[1];
			memset(d2, 5, 32);
			tableSize0 = 288;
			tableSize1 = 32;

			generateTables(0, 2, 3, tableSize0);
			generateTables(1, 4, 5, tableSize1);
			postprocess = true;
		} else {
			error("decompression failure");
		}

		if (!postprocess)
			continue;

		int16 cmd = 0;

		do  {
			cmd = ((int16 *)_tables[2])[_src->getKeyLower()];
			_src->advSrcBitsByIndex(cmd < 0 ? calcCmdAndIndex(_tables[3], cmd) : _tables[0][cmd]);

			if (cmd == 0x11d) {
				cmd = 0x200;
			} else if (cmd > 0x108) {
				cmd = _src->keyMaskedAlign(cmd);
			}

			if (!(cmd >> 8)) {
				*d++ = cmd & 0xff;
			} else if (cmd != 0x100) {
				cmd -= 0xfe;
				int16 offset = ((int16 *)_tables[4])[_src->getKeyLower()];
				_src->advSrcBitsByIndex(offset < 0 ? calcCmdAndIndex(_tables[5], offset) : _tables[1][offset]);
				if ((offset & 0xff) >= 4) {
					uint8 newIndex = ((offset & 0xff) >> 1) - 1;
					offset = (((offset & 1) + 2) << newIndex);
					offset += _src->getKeyMasked(newIndex);
				}

				uint8 *s2 = d - 1 - offset;
				if (s2 >= dst) {
					while (cmd--)
						*d++ = *s2++;
				} else {
					uint32 pos = dst - s2;
					s2 += (d - dst);

					if (pos < (uint32) cmd) {
						cmd -= pos;
						while (pos--)
							*d++ = *s2++;
						s2 = dst;
					}
					while (cmd--)
						*d++ = *s2++;
				}
			}
		} while (cmd != 0x100);
	}

	delete _src;
	_src = 0;

	return true;
}

void FileExpander::generateTables(uint8 srcIndex, uint8 dstIndex, uint8 dstIndex2, int cnt) {
	uint8 *tbl1 = _tables[srcIndex];
	uint8 *tbl2 = _tables[dstIndex];
	uint8 *tbl3 = dstIndex2 == 0xff ? 0 : _tables[dstIndex2];

	if (!cnt)
		return;

	const uint8 *s = tbl1;
	memset(_tables16[0], 0, 32);

	for (int i = 0; i < cnt; i++)
		_tables16[0][(*s++)]++;

	_tables16[1][1] = 0;

	for (uint16 i = 1, r = 0; i < 16; i++) {
		r = (r + _tables16[0][i]) << 1;
		_tables16[1][i + 1] = r;
	}

	if (_tables16[1][16]) {
		uint16 r = 0;
		for (uint16 i = 1; i < 16; i++)
			r += _tables16[0][i];
		if (r > 1)
			error("decompression failure");
	}

	s = tbl1;
	uint16 *d = _tables16[2];
	for (int i = 0; i < cnt; i++) {
		uint16 t = *s++;
		if (t) {
			_tables16[1][t]++;
			t = _tables16[1][t] - 1;
		}
		*d++ = t;
	}

	s = tbl1;
	d = _tables16[2];
	for (int i = 0; i < cnt; i++) {
		int8 t = ((int8)(*s++)) - 1;
		if (t > 0) {
			uint16 v1 = *d;
			uint16 v2 = 0;

			do {
				v2 = (v2 << 1) | (v1 & 1);
				v1 >>= 1;
			} while (--t && v1);

			t++;
			uint8 c1 = (v1 & 1);
			while (t--) {
				uint8 c2 = v2 >> 15;
				v2 = (v2 << 1) | c1;
				c1 = c2;
			};

			*d++ = v2;
		} else {
			d++;
		}
	}

	memset(tbl2, 0, 512);

	cnt--;
	s = tbl1 + cnt;
	d = &_tables16[2][cnt];
	uint16 *bt = (uint16 *)tbl3;
	uint16 inc = 0;
	uint16 cnt2 = 0;

	do {
		uint8 t = *s--;
		uint16 *s2 = (uint16 *)tbl2;

		if (t && t < 9) {
			inc = 1 << t;
			uint16 o = *d;

			do {
				s2[o] = cnt;
				o += inc;
			} while (!(o & 0xf00));

		} else if (t > 8) {
			if (!bt)
				error("decompression failure");

			t -= 8;
			uint8 shiftCnt = 1;
			uint8 v = (*d) >> 8;
			s2 = &((uint16 *)tbl2)[*d & 0xff];

			do {
				if (!*s2) {
					*s2 = (uint16)(~cnt2);
					*(uint32 *)&bt[cnt2] = 0;
					cnt2 += 2;
				}

				s2 = &bt[(uint16)(~*s2)];
				if (v & shiftCnt)
					s2++;

				shiftCnt <<= 1;
			} while (--t);
			*s2 = cnt;
		}
		d--;
	} while (--cnt >= 0);
}

uint8 FileExpander::calcCmdAndIndex(const uint8 *tbl, int16 &para) {
	const uint16 *t = (const uint16 *)tbl;
	_src->advSrcBitsByIndex(8);
	uint8 newIndex = 0;
	uint16 v = _src->getKeyLower();

	do {
		newIndex++;
		para = t[((~para) & 0xfffe) | (v & 1)];
		v >>= 1;
	} while (para < 0);

	return newIndex;
}

namespace {

struct InsArchive {
	Common::String filename;
	uint32 firstFile;
	uint32 startOffset;
	uint32 lastFile;
	uint32 endOffset;
	uint32 totalSize;
};

} // end of anonymouse namespace

class CmpVocDecoder {
public:
	CmpVocDecoder();
	~CmpVocDecoder();
	uint8 *process(uint8 *src, uint32 insize, uint32 *outsize, bool disposeInput = true);

private:
	void decodeHelper(int p);

	int32 *_vtbl;
	int32 *_tbl1, *_p1, *_tbl2, *_p2, *_tbl3, *_p3, *_tbl4, *_p4, *_floatArray, *_stTbl;
	uint8 *_sndArray;
};

Common::Archive *InstallerLoader::load(Resource *owner, const Common::String &filename, const Common::String &extension, const uint8 containerOffset) {
	uint32 pos = 0;
	uint32 bytesleft = 0;
	bool startFile = true;

	Common::String filenameBase = filename;
	Common::String filenameTemp;
	char filenameExt[4];

	if (filenameBase.lastChar() != '.')
		filenameBase += '.';

	InsArchive newArchive;
	Common::List<InsArchive> archives;

	Common::SeekableReadStream *tmpFile = 0;

	for (int8 currentFile = 1; currentFile; currentFile++) {
		sprintf(filenameExt, extension.c_str(), currentFile);
		filenameTemp = filenameBase + Common::String(filenameExt);

		if (!(tmpFile = owner->createReadStream(filenameTemp))) {
			debug(3, "couldn't open file '%s'\n", filenameTemp.c_str());
			break;
		}

		tmpFile->seek(pos, SEEK_SET);
		uint8 fileId = tmpFile->readByte();
		pos++;

		uint32 size = tmpFile->size() - 1;
		if (startFile) {
			size -= 4;
			if (fileId == currentFile) {
				size -= containerOffset;
				pos += containerOffset;
				tmpFile->seek(containerOffset, SEEK_CUR);
			} else {
				size = size + 1 - pos;
			}
			newArchive.filename = filenameBase;
			bytesleft = newArchive.totalSize = tmpFile->readUint32LE();
			pos += 4;
			newArchive.firstFile = currentFile;
			newArchive.startOffset = pos;
			startFile = false;
		}

		uint32 cs = MIN(size, bytesleft);
		bytesleft -= cs;

		delete tmpFile;
		tmpFile = 0;

		pos += cs;
		if (cs == size) {
			if (!bytesleft) {
				newArchive.lastFile = currentFile;
				newArchive.endOffset = --pos;
				archives.push_back(newArchive);
				currentFile = -1;
			} else {
				pos = 0;
			}
		} else {
			startFile = true;
			bytesleft = size - cs;
			newArchive.lastFile = currentFile--;
			newArchive.endOffset = --pos;
			archives.push_back(newArchive);
		}
	}

	FileExpander exp;
	CmpVocDecoder cvdec;
	CachedArchive::InputEntry newEntry;
	uint32 insize = 0;
	uint32 outsize = 0;
	uint8 *inbuffer = 0;
	uint8 *outbuffer = 0;
	uint32 inPart1 = 0;
	uint32 inPart2 = 0;
	uint8 compressionType = 0;
	Common::String entryStr;

	CachedArchive::FileInputList fileList;

	pos = 0;

	const uint32 kExecSize = 0x0bba;
	const uint32 kHeaderSize = 30;
	const uint32 kHeaderSize2 = 46;

	for (Common::List<InsArchive>::iterator a = archives.begin(); a != archives.end(); ++a) {
		startFile = true;
		for (uint32 i = a->firstFile; i != (a->lastFile + 1); i++) {
			sprintf(filenameExt, extension.c_str(), i);
			filenameTemp = a->filename + Common::String(filenameExt);

			if (!(tmpFile = owner->createReadStream(filenameTemp))) {
				debug(3, "couldn't open file '%s'\n", filenameTemp.c_str());
				break;
			}

			uint32 size = (i == a->lastFile) ? a->endOffset : tmpFile->size();

			if (startFile) {
				startFile = false;
				pos = a->startOffset + kExecSize;
				if (pos > size) {
					pos -= size;
					delete tmpFile;
					tmpFile = 0;
					continue;
				}
			} else {
				if (inPart2) {
					tmpFile->seek(1, SEEK_SET);
					tmpFile->read(inbuffer + inPart1, inPart2);
					inPart2 = 0;

					if (compressionType > 0)
						exp.process(outbuffer, inbuffer, outsize, insize);
					else
						memcpy(outbuffer, inbuffer, outsize);

					delete[] inbuffer;
					inbuffer = 0;

					newEntry.data = outbuffer;
					newEntry.size = outsize;
					newEntry.name = entryStr;

					entryStr.toUppercase();
					if (entryStr.hasSuffix(".CMP")) {
						entryStr.deleteLastChar();
						entryStr.deleteLastChar();
						entryStr.deleteLastChar();
						entryStr += "PAK";
						newEntry.data = cvdec.process(outbuffer, outsize, &outsize);
						newEntry.size = outsize;
						newEntry.name = entryStr;
					}

					fileList.push_back(newEntry);
				}
				pos++;
			}

			while (pos < size) {
				uint8 hdr[43];
				uint32 m = 0;
				tmpFile->seek(pos, SEEK_SET);

				if (pos + 42 > size) {
					m = size - pos;
					uint32 b = 42 - m;

					if (m >= 4) {
						uint32 id = tmpFile->readUint32LE();
						if (id == 0x06054B50) {
							startFile = true;
							break;
						} else {
							tmpFile->seek(pos, SEEK_SET);
						}
					}

					sprintf(filenameExt, extension.c_str(), i + 1);
					filenameTemp = a->filename + Common::String(filenameExt);

					Common::SeekableReadStream *tmpFile2 = owner->createReadStream(filenameTemp);
					tmpFile->read(hdr, m);
					tmpFile2->read(hdr + m, b);
					delete tmpFile2;
				} else {
					tmpFile->read(hdr, 42);
				}

				uint32 id = READ_LE_UINT32(hdr);

				if (id == 0x04034B50) {
					compressionType = hdr[8];
					insize = READ_LE_UINT32(hdr + 18);
					outsize = READ_LE_UINT32(hdr + 22);

					uint16 filestrlen = READ_LE_UINT16(hdr + 26);
					*(hdr + 30 + filestrlen) = 0;
					entryStr = Common::String((const char *)(hdr + 30));
					pos += (kHeaderSize + filestrlen - m);
					tmpFile->seek(pos, SEEK_SET);

					outbuffer = new uint8[outsize];
					if (!outbuffer)
						error("Out of memory: Can't uncompress installer files");

					if (!inbuffer) {
						inbuffer = new uint8[insize];
						if (!inbuffer)
							error("Out of memory: Can't uncompress installer files");
					}

					if ((pos + insize) > size) {
						// this is for files that are split between two archive files
						inPart1 = size - pos;
						inPart2 = insize - inPart1;
						tmpFile->read(inbuffer, inPart1);
					} else {
						tmpFile->read(inbuffer, insize);
						inPart2 = 0;

						if (compressionType > 0)
							exp.process(outbuffer, inbuffer, outsize, insize);
						else
							memcpy(outbuffer, inbuffer, outsize);

						delete[] inbuffer;
						inbuffer = 0;
						newEntry.data = outbuffer;
						newEntry.size = outsize;
						newEntry.name = entryStr;

						entryStr.toUppercase();
						if (entryStr.hasSuffix(".CMP")) {
							entryStr.deleteLastChar();
							entryStr.deleteLastChar();
							entryStr.deleteLastChar();
							entryStr += "PAK";
							newEntry.data = cvdec.process(outbuffer, outsize, &outsize);
							newEntry.size = outsize;
							newEntry.name = entryStr;
						}

						fileList.push_back(newEntry);
					}

					pos += insize;
					if (pos > size) {
						pos -= size;
						break;
					}
				} else {
					uint32 filestrlen = READ_LE_UINT32(hdr + 28);
					pos += (kHeaderSize2 + filestrlen - m);
				}
			}
			delete tmpFile;
			tmpFile = 0;
		}
	}

	archives.clear();
	return new CachedArchive(fileList);
}

CmpVocDecoder::CmpVocDecoder() {
	_tbl1 = new int32[4000];
	_p1 = _tbl1 + 2000;
	_tbl2 = new int32[4000];
	_p2 = _tbl2 + 2000;
	_tbl3 = new int32[4000];
	_p3 = _tbl3 + 2000;
	_tbl4 = new int32[4000];
	_p4 = _tbl4 + 2000;

	_vtbl = new int32[8193];
	_floatArray = new int32[8193];
	_sndArray = new uint8[8192];
	_stTbl = new int32[256];

	assert(_tbl1);
	assert(_tbl2);
	assert(_tbl3);
	assert(_tbl4);
	assert(_vtbl);
	assert(_floatArray);
	assert(_sndArray);
	assert(_stTbl);

	for (int32 i = -2000; i < 2000; i++) {
		int32 x = i + 2000;
		_tbl1[x] = (int32)(0.4829629131445341 * (double)i * 256.0);
		_tbl2[x] = (int32)(0.8365163037378079 * (double)i * 256.0);
		_tbl3[x] = (int32)(0.2241438680420134 * (double)i * 256.0);
		_tbl4[x] = (int32)(-0.1294095225512604 * (double)i * 256.0);
	}
}

CmpVocDecoder::~CmpVocDecoder() {
	delete[] _stTbl;
	delete[] _sndArray;
	delete[] _floatArray;
	delete[] _vtbl;
	delete[] _tbl1;
	delete[] _tbl2;
	delete[] _tbl3;
	delete[] _tbl4;
}

uint8 *CmpVocDecoder::process(uint8 *src, uint32 insize, uint32 *outsize, bool disposeInput) {
	*outsize = 0;
	uint8 *outTemp = new uint8[insize];

	uint8 *inPosH = src;
	uint8 *outPosH = outTemp;
	uint8 *outPosD = outTemp + READ_LE_UINT32(src);
	uint8 *end = outPosD;

	while (outPosH < end) {
		uint8 *spos = inPosH;
		uint32 offset = READ_LE_UINT32(inPosH);
		inPosH += 4;
		char *name = (char *)inPosH;
		inPosH += strlen(name) + 1;

		if (!name[0]) {
			*outsize = outPosD - outTemp;
			WRITE_LE_UINT32(outPosH, *outsize);
			memset(outPosH + 4, 0, 5);
			break;
		}

		uint32 fileSize = READ_LE_UINT32(inPosH) - offset;
		int headerEntryLen = inPosH - spos;

		if (scumm_stricmp(name + strlen(name) - 4, ".voc")) {
			memcpy(outPosH, spos, headerEntryLen);
			WRITE_LE_UINT32(outPosH, outPosD - outTemp);
			outPosH += headerEntryLen;
			memcpy(outPosD, src + offset, fileSize);
			outPosD += fileSize;
			continue;
		}

		uint8 *vocPtr = src + offset;
		uint32 vocLen = (vocPtr[27] | (vocPtr[28] << 8) | (vocPtr[29] << 16)) - 2;

		uint8 *vocOutEnd = outPosD + vocLen + 32;
		uint8 *vocInEnd = src + offset + fileSize;
		memcpy(outPosD, vocPtr, 32);
		uint8 *dst = outPosD + 32;
		vocPtr += 32;
		float t = 0.0f;

		while (dst < vocOutEnd) {
			memcpy(&t, vocPtr, 4);
			vocPtr += 4;
			uint32 readSize = MIN<uint32>(8192, vocInEnd - vocPtr);
			memcpy(_sndArray, vocPtr, readSize);
			vocPtr += readSize;

			for (int i = -128; i < 128; i++)
				_stTbl[i + 128] = (int32)((float)i / t + 0.5f);

			int8 *ps = (int8 *)_sndArray;
			for (int i = 0; i < 8192; i++)
				_floatArray[i + 1] = _stTbl[128 + *ps++];

			for (int i = 4; i <= 8192; i <<= 1)
				decodeHelper(i);

			for (int i = 1; i <= 8192; i++) {
				int32 v = CLIP<int32>(_floatArray[i] + 128, 0, 255);
				_sndArray[i - 1] = v;
			}

			uint32 numBytesOut = MIN<uint32>(vocOutEnd - dst, 8192);
			memcpy(dst, _sndArray, numBytesOut);
			dst += numBytesOut;
		}

		*dst++ = 0;
		memcpy(outPosH, spos, headerEntryLen);
		WRITE_LE_UINT32(outPosH, outPosD - outTemp);
		outPosH += headerEntryLen;
		outPosD += (vocLen + 33);
	}

	if (disposeInput)
		delete[] src;

	uint8 *outFinal = new uint8[*outsize];
	memcpy(outFinal, outTemp, *outsize);
	delete[] outTemp;

	return outFinal;
}

void CmpVocDecoder::decodeHelper(int p1) {
	int p2 = p1 >> 1;
	int p3 = p2 + 1;

	int16 fi1 = _floatArray[1];
	int16 fi2 = _floatArray[p2];
	int16 fi3 = _floatArray[p3];
	int16 fi4 = _floatArray[p1];

	_vtbl[1] = (*(_p3 + fi2) + *(_p2 + fi4) + *(_p1 + fi1) + *(_p4 + fi3)) >> 8;
	_vtbl[2] = (*(_p4 + fi2) - *(_p1 + fi4) + *(_p2 + fi1) - *(_p3 + fi3)) >> 8;

	int d = 3;
	int s = 1;

	while (s < p2) {
		fi2 = _floatArray[s];
		fi1 = _floatArray[s + 1];
		fi4 = _floatArray[s + p2];
		fi3 = _floatArray[s + p3];

		_vtbl[d++] = (*(_p3 + fi2) + *(_p2 + fi4) + *(_p1 + fi1) + *(_p4 + fi3)) >> 8;
		_vtbl[d++] = (*(_p4 + fi2) - *(_p1 + fi4) + *(_p2 + fi1) - *(_p3 + fi3)) >> 8;
		s++;
	}

	memcpy(&_floatArray[1], &_vtbl[1], p1 * sizeof(int32));
}

} // End of namespace Kyra
