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

#ifndef WINTERMUTE_BASE_FILE_H
#define WINTERMUTE_BASE_FILE_H


#include "engines/wintermute/base/base.h"
#include "common/str.h"
#include "common/stream.h"

namespace Common {
class SeekableReadStream;
}

namespace Wintermute {

class BaseFile {
protected:
	uint32 _pos;
	uint32 _size;
public:
	virtual uint32 getSize() {
		return _size;
	};
	virtual uint32 getPos() {
		return _pos;
	};
	virtual bool seek(uint32 pos, int whence = SEEK_SET) = 0;
	virtual bool read(void *buffer, uint32 size) = 0;
	virtual bool close() = 0;
	virtual bool open(const Common::String &filename) = 0;
	virtual bool isEOF();
	BaseFile();
	virtual ~BaseFile();
	// Temporary solution to allow usage in ScummVM-code:
	virtual Common::SeekableReadStream *getMemStream();
};

} // end of namespace Wintermute

#endif
