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

#ifndef FILESYSTEM_FACTORY_H
#define FILESYSTEM_FACTORY_H

#include "common/str.h"
#include "backends/fs/abstract-fs.h"

/**
 * Creates concrete FSNode objects depending on the current architecture.
 */
class FilesystemFactory {
public:
	/**
	 * Destructor.
	 */
	virtual ~FilesystemFactory() {}

	/**
	 * Returns a node representing the "current directory".
	 * If your system does not support this concept, you can either try to
	 * emulate it or simply return some "sensible" default directory node,
	 * e.g. the same value as getRoot() returns.
	 */
	virtual AbstractFSNode *makeCurrentDirectoryFileNode() const = 0;

	/**
	 * Construct a node based on a path; the path is in the same format as it
	 * would be for calls to fopen().
	 *
	 * Furthermore getNodeForPath(oldNode.path()) should create a new node
	 * identical to oldNode. Hence, we can use the "path" value for persistent
	 * storage e.g. in the config file.
	 *
	 * @param path The path string to create a FSNode for.
	 */
	virtual AbstractFSNode *makeFileNodePath(const Common::String &path) const = 0;

	/**
	 * Returns a special node representing the filesystem root.
	 * The starting point for any file system browsing.
	 *
	 * On Unix, this will be simply the node for / (the root directory).
	 * On Windows, it will be a special node which "contains" all drives (C:, D:, E:).
	 */
	virtual AbstractFSNode *makeRootFileNode() const = 0;
};

#endif /*FILESYSTEM_FACTORY_H*/
