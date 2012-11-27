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

#ifndef BACKENDS_ABSTRACT_FS_H
#define BACKENDS_ABSTRACT_FS_H

#include "common/array.h"
#include "common/str.h"
#include "common/fs.h"

class AbstractFSNode;

typedef Common::Array<AbstractFSNode *>	AbstractFSList;

/**
 * Abstract file system node. Private subclasses implement the actual
 * functionality.
 *
 * Most of the methods correspond directly to methods in class FSNode,
 * so if they are not documented here, look there for more information about
 * the semantics.
 */
class AbstractFSNode {
protected:
	friend class Common::FSNode;
	typedef Common::FSNode::ListMode ListMode;

	/**
	 * Returns the child node with the given name. When called on a non-directory
	 * node, it should handle this gracefully by returning 0.
	 * When called with a name not matching any of the files/dirs contained in this
	 * directory, a valid node should be returned, which returns 'false' upon calling
	 * the exists() method. The idea is that this node can then still can be used to
	 * create a new file via the createWriteStream() method.
	 *
	 * Example:
	 *			Calling getChild() for a node with path "/foo/bar" using name="file.txt",
	 *			would produce a new node with "/foo/bar/file.txt" as path.
	 *
	 * @note Handling calls on non-dir nodes gracefully makes it possible to
	 * switch to a lazy type detection scheme in the future.
	 *
	 * @param name String containing the name of the child to create a new node.
	 */
	virtual AbstractFSNode *getChild(const Common::String &name) const = 0;

	/**
	 * The parent node of this directory.
	 * The parent of the root is the root itself.
	 */
	virtual AbstractFSNode *getParent() const = 0;

	/**
	 * Returns the last component of a given path.
	 *
	 * Examples:
	 *			/foo/bar.txt would return /bar.txt
	 *			/foo/bar/    would return /bar/
	 *
	 * @param str String containing the path.
	 * @param sep character used to separate path components
	 * @return Pointer to the first char of the last component inside str.
	 */
	static const char *lastPathComponent(const Common::String &str, const char sep);

public:
	/**
	 * Destructor.
	 */
	virtual ~AbstractFSNode() {}

	/*
	 * Indicates whether the object referred by this path exists in the filesystem or not.
	 */
	virtual bool exists() const = 0;

	/**
	 * Return a list of child nodes of this directory node. If called on a node
	 * that does not represent a directory, false is returned.
	 *
	 * @param list List to put the contents of the directory in.
	 * @param mode Mode to use while listing the directory.
	 * @param hidden Whether to include hidden files or not in the results.
	 *
	 * @return true if succesful, false otherwise (e.g. when the directory does not exist).
	 */
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const = 0;

	/**
	 * Returns a human readable path string.
	 *
	 * @note By default, this method returns the value of getName().
	 */
	virtual Common::String getDisplayName() const { return getName(); }

	/**
	 * Returns the last component of the path pointed by this FSNode.
	 *
	 * Examples (POSIX):
	 *			/foo/bar.txt would return /bar.txt
	 *			/foo/bar/    would return /bar/
	 *
	 * @note This method is very architecture dependent, please check the concrete implementation for more information.
	 */
	virtual Common::String getName() const = 0;

	/**
	 * Returns the 'path' of the current node, usable in fopen().
	 */
	virtual Common::String getPath() const = 0;

	/**
	 * Indicates whether this path refers to a directory or not.
	 */
	virtual bool isDirectory() const = 0;

	/**
	 * Indicates whether the object referred by this path can be read from or not.
	 *
	 * If the path refers to a directory, readability implies being able to read
	 * and list the directory entries.
	 *
	 * If the path refers to a file, readability implies being able to read the
	 * contents of the file.
	 *
	 * @return bool true if the object can be read, false otherwise.
	 */
	virtual bool isReadable() const = 0;

	/**
	 * Indicates whether the object referred by this path can be written to or not.
	 *
	 * If the path refers to a directory, writability implies being able to modify
	 * the directory entry (i.e. rename the directory, remove it or write files inside of it).
	 *
	 * If the path refers to a file, writability implies being able to write data
	 * to the file.
	 *
	 * @return bool true if the object can be written to, false otherwise.
	 */
	virtual bool isWritable() const = 0;


	/**
	 * Creates a SeekableReadStream instance corresponding to the file
	 * referred by this node. This assumes that the node actually refers
	 * to a readable file. If this is not the case, 0 is returned.
	 *
	 * @return pointer to the stream object, 0 in case of a failure
	 */
	virtual Common::SeekableReadStream *createReadStream() = 0;

	/**
	 * Creates a WriteStream instance corresponding to the file
	 * referred by this node. This assumes that the node actually refers
	 * to a readable file. If this is not the case, 0 is returned.
	 *
	 * @return pointer to the stream object, 0 in case of a failure
	 */
	virtual Common::WriteStream *createWriteStream() = 0;
};



#endif //BACKENDS_ABSTRACT_FS_H
