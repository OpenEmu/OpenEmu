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

#include "engines/wintermute/base/file/base_file.h"
#include "common/memstream.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
BaseFile::BaseFile() {
	_pos = 0;
	_size = 0;
}


//////////////////////////////////////////////////////////////////////////
BaseFile::~BaseFile() {

}


//////////////////////////////////////////////////////////////////////////
bool BaseFile::isEOF() {
	return _pos == _size;
}

Common::SeekableReadStream *BaseFile::getMemStream() {
	uint32 oldPos = getPos();
	seek(0);
	byte *data = new byte[getSize()];
	read(data, getSize());
	seek(oldPos);
	Common::MemoryReadStream *memStream = new Common::MemoryReadStream(data, getSize(), DisposeAfterUse::YES);
	return memStream;
}


} // end of namespace Wintermute
