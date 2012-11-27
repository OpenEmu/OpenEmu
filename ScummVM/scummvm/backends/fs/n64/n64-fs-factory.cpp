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

#ifdef __N64__

#include <n64utils.h>
#include <romfs.h>

#include "backends/fs/n64/n64-fs-factory.h"
#include "backends/fs/n64/n64-fs.h"

AbstractFSNode *N64FilesystemFactory::makeRootFileNode() const {
	return new N64FilesystemNode();
}

AbstractFSNode *N64FilesystemFactory::makeCurrentDirectoryFileNode() const {
	char buf[MAXPATHLEN];
	return romfs_getcwd(buf, MAXPATHLEN) ? new N64FilesystemNode(Common::String(buf), false) : NULL;
}

AbstractFSNode *N64FilesystemFactory::makeFileNodePath(const Common::String &path) const {
	assert(!path.empty());
	return new N64FilesystemNode(path, false);
}

#endif
