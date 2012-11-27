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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/file/base_package.h"
#include "engines/wintermute/base/file/base_file_entry.h"
#include "engines/wintermute/base/file/dcpackage.h"
#include "engines/wintermute/wintermute.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/debug.h"

namespace Wintermute {

BasePackage::BasePackage() {
	_name = "";
	_cd = 0;
	_priority = 0;
	_boundToExe = false;
}

Common::SeekableReadStream *BasePackage::getFilePointer() {
	Common::SeekableReadStream *stream = _fsnode.createReadStream();

	return stream;
}

static bool findPackageSignature(Common::SeekableReadStream *f, uint32 *offset) {
	byte buf[32768];

	byte signature[8];
	((uint32 *)signature)[0] = PACKAGE_MAGIC_1;
	((uint32 *)signature)[1] = PACKAGE_MAGIC_2;

	uint32 fileSize = (uint32)f->size();
	uint32 startPos = 1024 * 1024;
	uint32 bytesRead = startPos;

	while (bytesRead < fileSize - 16) {
		uint32 toRead = MIN((unsigned int)32768, fileSize - bytesRead);
		f->seek((int32)startPos, SEEK_SET);
		uint32 actuallyRead = f->read(buf, toRead);
		if (actuallyRead != toRead) {
			return false;
		}

		for (uint32 i = 0; i < toRead - 8; i++)
			if (!memcmp(buf + i, signature, 8)) {
				*offset =  startPos + i;
				return true;
			}

		bytesRead = bytesRead + toRead - 16;
		startPos = startPos + toRead - 16;

	}
	return false;

}

void TPackageHeader::readFromStream(Common::ReadStream *stream) {
	_magic1 = stream->readUint32LE();
	_magic2 = stream->readUint32LE();
	_packageVersion = stream->readUint32LE();

	_gameVersion = stream->readUint32LE();

	_priority = stream->readByte();
	_cd = stream->readByte();
	_masterIndex = stream->readByte();
	stream->readByte(); // To align the next byte...

	_creationTime = stream->readUint32LE();

	stream->read(_desc, 100);
	_numDirs = stream->readUint32LE();
}

PackageSet::PackageSet(Common::FSNode file, const Common::String &filename, bool searchSignature) {
	uint32 absoluteOffset = 0;
	_priority = 0;
	bool boundToExe = false;
	Common::SeekableReadStream *stream = file.createReadStream();
	if (!stream) {
		return;
	}
	if (searchSignature) {
		uint32 offset;
		if (!findPackageSignature(stream, &offset)) {
			delete stream;
			return;
		} else {
			stream->seek(offset, SEEK_SET);
			absoluteOffset = offset;
			boundToExe = true;
		}
	}

	TPackageHeader hdr;
	hdr.readFromStream(stream);
	if (hdr._magic1 != PACKAGE_MAGIC_1 || hdr._magic2 != PACKAGE_MAGIC_2 || hdr._packageVersion > PACKAGE_VERSION) {
		debugC(kWintermuteDebugFileAccess | kWintermuteDebugLog, "  Invalid header in package file '%s'. Ignoring.", filename.c_str());
		delete stream;
		return;
	}

	if (hdr._packageVersion != PACKAGE_VERSION) {
		debugC(kWintermuteDebugFileAccess | kWintermuteDebugLog, "  Warning: package file '%s' is outdated.", filename.c_str());
	}
	_priority = hdr._priority;
	// new in v2
	if (hdr._packageVersion == PACKAGE_VERSION) {
		uint32 dirOffset;
		dirOffset = stream->readUint32LE();
		dirOffset += absoluteOffset;
		stream->seek(dirOffset, SEEK_SET);
	}
	assert(hdr._numDirs == 1);
	for (uint32 i = 0; i < hdr._numDirs; i++) {
		BasePackage *pkg = new BasePackage();
		if (!pkg) {
			return;
		}
		pkg->_fsnode = file;

		pkg->_boundToExe = boundToExe;

		// read package info
		byte nameLength = stream->readByte();
		char *pkgName = new char[nameLength];
		stream->read(pkgName, nameLength);
		pkg->_name = pkgName;
		pkg->_cd = stream->readByte();
		pkg->_priority = hdr._priority;
		delete[] pkgName;
		pkgName = NULL;

		if (!hdr._masterIndex) {
			pkg->_cd = 0;    // override CD to fixed disk
		}
		_packages.push_back(pkg);

		// read file entries
		uint32 numFiles = stream->readUint32LE();

		for (uint32 j = 0; j < numFiles; j++) {
			char *name;
			uint32 offset, length, compLength, flags;/*, timeDate1, timeDate2;*/

			nameLength = stream->readByte();
			name = new char[nameLength];
			stream->read(name, nameLength);

			// v2 - xor name
			if (hdr._packageVersion == PACKAGE_VERSION) {
				for (int k = 0; k < nameLength; k++) {
					((byte *)name)[k] ^= 'D';
				}
			}
			debugC(kWintermuteDebugFileAccess, "Package contains %s", name);

			Common::String upcName = name;
			upcName.toUppercase();
			delete[] name;
			name = NULL;

			offset = stream->readUint32LE();
			offset += absoluteOffset;
			length = stream->readUint32LE();
			compLength = stream->readUint32LE();
			flags = stream->readUint32LE();

			if (hdr._packageVersion == PACKAGE_VERSION) {
				/* timeDate1 = */ stream->readUint32LE();
				/* timeDate2 = */ stream->readUint32LE();
			}
			_filesIter = _files.find(upcName);
			if (_filesIter == _files.end()) {
				BaseFileEntry *fileEntry = new BaseFileEntry();
				fileEntry->_package = pkg;
				fileEntry->_offset = offset;
				fileEntry->_length = length;
				fileEntry->_compressedLength = compLength;
				fileEntry->_flags = flags;

				_files[upcName] = Common::ArchiveMemberPtr(fileEntry);
			} else {
				// current package has higher priority than the registered
				// TODO: This cast might be a bit ugly.
				BaseFileEntry *filePtr = (BaseFileEntry *) &*(_filesIter->_value);
				if (pkg->_priority > filePtr->_package->_priority) {
					filePtr->_package = pkg;
					filePtr->_offset = offset;
					filePtr->_length = length;
					filePtr->_compressedLength = compLength;
					filePtr->_flags = flags;
				}
			}
		}
	}
	debugC(kWintermuteDebugFileAccess, "  Registered %d files in %d package(s)", _files.size(), _packages.size());

	delete stream;
}

PackageSet::~PackageSet() {
	for (Common::Array<BasePackage *>::iterator it = _packages.begin(); it != _packages.end(); ++it) {
		delete *it;
	}
	_packages.clear();
}

bool PackageSet::hasFile(const Common::String &name) const {
	Common::String upcName = name;
	upcName.toUppercase();
	Common::HashMap<Common::String, Common::ArchiveMemberPtr>::const_iterator it;
	it = _files.find(upcName.c_str());
	return (it != _files.end());
}

int PackageSet::listMembers(Common::ArchiveMemberList &list) const {
	Common::HashMap<Common::String, Common::ArchiveMemberPtr>::const_iterator it = _files.begin();
	Common::HashMap<Common::String, Common::ArchiveMemberPtr>::const_iterator end = _files.end();
	int count = 0;
	for (; it != end; ++it) {
		const Common::ArchiveMemberPtr ptr(it->_value);
		list.push_back(ptr);
		count++;
	}
	return count;
}

const Common::ArchiveMemberPtr PackageSet::getMember(const Common::String &name) const {
	Common::String upcName = name;
	upcName.toUppercase();
	Common::HashMap<Common::String, Common::ArchiveMemberPtr>::const_iterator it;
	it = _files.find(upcName.c_str());
	return Common::ArchiveMemberPtr(it->_value);
}

Common::SeekableReadStream *PackageSet::createReadStreamForMember(const Common::String &name) const {
	Common::String upcName = name;
	upcName.toUppercase();
	Common::HashMap<Common::String, Common::ArchiveMemberPtr>::const_iterator it;
	it = _files.find(upcName.c_str());
	if (it != _files.end()) {
		return it->_value->createReadStream();
	}
	return NULL;
}

} // end of namespace Wintermute
