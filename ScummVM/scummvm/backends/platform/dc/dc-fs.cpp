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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "dc.h"
#include "backends/fs/abstract-fs.h"
#include "backends/fs/stdiostream.h"

#include <ronin/cdfs.h>
#include <stdio.h>
#include <unistd.h>

/**
 * Implementation of the ScummVM file system API based on Ronin.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class RoninCDFileNode : public AbstractFSNode {
protected:
	Common::String _path;

public:
	RoninCDFileNode(const Common::String &path) : _path(path) {}

	virtual bool exists() const { return true; }
	virtual Common::String getName() const { return lastPathComponent(_path, '/'); }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return false; }
	virtual bool isReadable() const { return true; }
	virtual bool isWritable() const { return false; }

	virtual AbstractFSNode *getChild(const Common::String &n) const { return NULL; }
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const { return false; }
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream() { return 0; }

	static AbstractFSNode *makeFileNodePath(const Common::String &path);
};

/* A directory */
class RoninCDDirectoryNode : public RoninCDFileNode {
public:
	RoninCDDirectoryNode(const Common::String &path) : RoninCDFileNode(path) {}

	virtual bool isDirectory() const { return true; }
	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual Common::SeekableReadStream *createReadStream() { return 0; }
};

/* A file/directory which does not exist */
class RoninCDNonexistingNode : public RoninCDFileNode {
public:
	RoninCDNonexistingNode(const Common::String &path) : RoninCDFileNode(path) {}

	virtual bool exists() const { return false; }
	virtual bool isReadable() const { return false; }
	virtual Common::SeekableReadStream *createReadStream() { return 0; }
};

AbstractFSNode *RoninCDFileNode::makeFileNodePath(const Common::String &path) {
	assert(path.size() > 0);

	int fd;

	if ((fd = open(path.c_str(), O_RDONLY)) >= 0) {
		close(fd);
		return new RoninCDFileNode(path);
	} else if ((fd = open(path.c_str(), O_DIR|O_RDONLY)) >= 0) {
		close(fd);
		return new RoninCDDirectoryNode(path);
	} else {
		return NULL;
	}
}

AbstractFSNode *RoninCDDirectoryNode::getChild(const Common::String &n) const {
	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return makeFileNodePath(newPath);
}

bool RoninCDDirectoryNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {

	DIR *dirp = opendir(_path.c_str());
	struct dirent *dp;

	if (dirp == NULL)
		return false;

	// ... loop over dir entries using readdir
	while ((dp = readdir(dirp)) != NULL) {
		Common::String newPath(_path);
		if (newPath.lastChar() != '/')
			newPath += '/';
		newPath += dp->d_name;

		if (dp->d_size < 0) {
			// Honor the chosen mode
			if (mode == Common::FSNode::kListFilesOnly)
				continue;

			myList.push_back(new RoninCDDirectoryNode(newPath));
		} else {
			// Honor the chosen mode
			if (mode == Common::FSNode::kListDirectoriesOnly)
				continue;

			myList.push_back(new RoninCDFileNode(newPath));
		}
	}
	closedir(dirp);

	return true;
}

AbstractFSNode *RoninCDFileNode::getParent() const {
	if (_path == "/")
		return 0;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path, '/');

	return new RoninCDDirectoryNode(Common::String(start, end - start));
}


Common::SeekableReadStream *RoninCDFileNode::createReadStream() {
	return StdioStream::makeFromPath(getPath().c_str(), false);
}

AbstractFSNode *OSystem_Dreamcast::makeRootFileNode() const {
	return new RoninCDDirectoryNode("/");
}

AbstractFSNode *OSystem_Dreamcast::makeCurrentDirectoryFileNode() const {
	return makeRootFileNode();
}

AbstractFSNode *OSystem_Dreamcast::makeFileNodePath(const Common::String &path) const {
	AbstractFSNode *node = RoninCDFileNode::makeFileNodePath(path);
	return (node? node : new RoninCDNonexistingNode(path));
}
