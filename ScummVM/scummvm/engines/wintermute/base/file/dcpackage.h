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

#ifndef _DCPACKAGE_H_
#define _DCPACKAGE_H_


#define PACKAGE_MAGIC_1   0xDEC0ADDE
#define PACKAGE_MAGIC_2   0x4B4E554A    // "JUNK"
#define PACKAGE_VERSION   0x00000200
#define PACKAGE_EXTENSION "dcp"

#include "common/stream.h"

namespace Wintermute {

struct TPackageHeader {
	uint32 _magic1;
	uint32 _magic2;
	uint32 _packageVersion;
	uint32 _gameVersion;
	byte _priority;
	byte _cd;
	bool _masterIndex;
	uint32 _creationTime;
	char _desc[100];
	uint32 _numDirs;
	// base_package.cpp:
	void readFromStream(Common::ReadStream *stream);
};

/*
v2:  uint32 DirOffset


Dir: byte NameLength
     char Name [NameLength]
     byte CD;
     uint32 NumEntries


Entry: byte NameLength
       char Name [NameLength]
       uint32 Offset
       uint32 Length
       uint32 CompLength
       uint32 Flags
v2:    uint32 TimeDate1
       uint32 TimeDate2  // not used

*/

} // end of namespace Wintermute

#endif
