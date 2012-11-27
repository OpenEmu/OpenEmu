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

#ifndef PS2_FILESYSTEM_H
#define PS2_FILESYSTEM_H

#include "backends/fs/abstract-fs.h"

/**
 * Implementation of the ScummVM file system API based on the Ps2SDK.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class Ps2FilesystemNode : public AbstractFSNode {

friend class Ps2FilesystemFactory;

protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isRoot;
	bool _isHere;
	bool _verified;

private:
	const char *getDeviceDescription() const;
	void doverify();

public:
	/**
	 * Creates a PS2FilesystemNode with the root node as path.
	 */
	Ps2FilesystemNode();

	/**
	 * Creates a PS2FilesystemNode for a given path.
	 *
	 * @param path Common::String with the path the new node should point to.
	 */
	Ps2FilesystemNode(const Common::String &path);
	Ps2FilesystemNode(const Common::String &path, bool verify);

	/**
	 * Copy constructor.
	 */
	Ps2FilesystemNode(const Ps2FilesystemNode *node);

	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }

	virtual bool exists() const {
		// printf("%s : is %d\n", _path.c_str(), _isHere);
		return _isHere;
	}

	virtual bool isDirectory() const {
		// printf("%s : dir %d\n", _path.c_str(), _isDirectory);
		return _isDirectory;
	}

	virtual bool isReadable() const {
		return _isHere;
	}

	virtual bool isWritable() const {
		if (strncmp(_path.c_str(), "cdfs", 4)==0)
			return false;
		return true; // exists(); // creating ?
	}

	virtual AbstractFSNode *clone() const { return new Ps2FilesystemNode(this); }
	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();

	int getDev() { return 0; }
};

#endif
