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
 */

// Disable symbol overrides for FILE as that is used in FLAC headers
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#if defined(__DS__)
#include "backends/fs/ds/ds-fs-factory.h"
#include "backends/fs/ds/ds-fs.h"
#include "dsmain.h" //for the isGBAMPAvailable() function

namespace Common {
DECLARE_SINGLETON(DSFilesystemFactory);
}

AbstractFSNode *DSFilesystemFactory::makeRootFileNode() const {
	if (DS::isGBAMPAvailable()) {
		return new DS::GBAMPFileSystemNode();
	} else {
		return new DS::DSFileSystemNode();
	}
}

AbstractFSNode *DSFilesystemFactory::makeCurrentDirectoryFileNode() const {
	if (DS::isGBAMPAvailable()) {
		return new DS::GBAMPFileSystemNode();
	} else {
		return new DS::DSFileSystemNode();
	}
}

AbstractFSNode *DSFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	if (DS::isGBAMPAvailable()) {
		return new DS::GBAMPFileSystemNode(path);
	} else {
		return new DS::DSFileSystemNode(path);
	}
}
#endif
