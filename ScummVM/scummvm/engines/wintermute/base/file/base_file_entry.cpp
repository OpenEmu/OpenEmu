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

#include "engines/wintermute/base/file/base_file_entry.h"
#include "engines/wintermute/base/file/base_package.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/zlib.h"

namespace Wintermute {

Common::SeekableReadStream *BaseFileEntry::createReadStream() const {
	Common::SeekableReadStream *file = _package->getFilePointer();
	if (!file) {
		return NULL;
	}

	bool compressed = (_compressedLength != 0);

	if (compressed) {
		file = Common::wrapCompressedReadStream(new Common::SeekableSubReadStream(file, _offset, _offset + _length, DisposeAfterUse::YES), _length); //
	} else {
		file = new Common::SeekableSubReadStream(file, _offset, _offset + _length, DisposeAfterUse::YES);
	}

	file->seek(0);

	return file;
}

//////////////////////////////////////////////////////////////////////////
BaseFileEntry::BaseFileEntry() {
	_package = NULL;
	_length = _compressedLength = _offset = _flags = 0;
	_filename = "";

	_timeDate1 = _timeDate2 = 0;

	_journalTime = 0;
}


//////////////////////////////////////////////////////////////////////////
BaseFileEntry::~BaseFileEntry() {
	_package = NULL; // ref only
}

} // end of namespace Wintermute
