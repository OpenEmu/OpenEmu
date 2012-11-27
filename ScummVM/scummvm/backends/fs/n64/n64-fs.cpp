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
 *
 */

#ifdef __N64__

#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "backends/fs/n64/n64-fs.h"
#include "backends/fs/n64/romfsstream.h"

#include <romfs.h>
#include <sys/param.h>
#include <unistd.h>
#include <n64utils.h>

#define	ROOT_PATH	"/"

N64FilesystemNode::N64FilesystemNode() {
	_isDirectory = true;
	_displayName = "Root";
	_isValid = true;
	_path = ROOT_PATH;
}

N64FilesystemNode::N64FilesystemNode(const Common::String &p, bool verify) {
	assert(p.size() > 0);

	_path = p;
	_displayName = lastPathComponent(_path, '/');
	_isValid = true;
	_isDirectory = true;

	// Check if it's a dir
	ROMFILE *tmpfd = romfs_open(p.c_str(), "r");
	if (tmpfd) {
		_isDirectory = (tmpfd->type == 0 || tmpfd->type == 1);
		romfs_close(tmpfd);
	}
}

bool N64FilesystemNode::exists() const {
	int ret = -1;

	ret = romfs_access(_path.c_str(), F_OK);

	return ret == 0;
}

bool N64FilesystemNode::isReadable() const {
	int ret = -1;

	ret = romfs_access(_path.c_str(), R_OK);

	return ret == 0;
}

// We can't write on ROMFS!
bool N64FilesystemNode::isWritable() const {
	return false;
}


AbstractFSNode *N64FilesystemNode::getChild(const Common::String &n) const {
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);

	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return new N64FilesystemNode(newPath, true);
}

bool N64FilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);
	ROMDIR *dirp = romfs_opendir(_path.c_str());
	romfs_dirent *dp;

	if (dirp == NULL)
		return false;

	// loop over dir entries using readdir
	while ((dp = romfs_readdir(dirp)) != NULL) {
		// Skip 'invisible' files if necessary
		if (dp->entryname[0] == '.' && !hidden) {
			free(dp);
			continue;
		}
		// Skip '.' and '..' to avoid cycles
		if ((dp->entryname[0] == '.' && dp->entryname[1] == 0) || (dp->entryname[0] == '.' && dp->entryname[1] == '.')) {
			free(dp);
			continue;
		}

		// Start with a clone of this node, with the correct path set
		N64FilesystemNode entry(*this);
		entry._displayName = dp->entryname;
		if (_path.lastChar() != '/')
			entry._path += '/';
		entry._path += entry._displayName;

		// Force validity for now...
		entry._isValid = 1;

		entry._isDirectory = (dp->type == 0 || dp->type == 1);

		// Honor the chosen mode
		if ((mode == Common::FSNode::kListFilesOnly && entry._isDirectory) ||
		        (mode == Common::FSNode::kListDirectoriesOnly && !entry._isDirectory)) {

			free(dp);
			continue;
		}

		myList.push_back(new N64FilesystemNode(entry));

		free(dp);
	}
	romfs_closedir(dirp);

	return true;
}

AbstractFSNode *N64FilesystemNode::getParent() const {
	if (_path == ROOT_PATH)
		return 0;

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path, '/');

	return new N64FilesystemNode(Common::String(start, end - start), false);
}

Common::SeekableReadStream *N64FilesystemNode::createReadStream() {
	return RomfsStream::makeFromPath(getPath(), false);
}

Common::WriteStream *N64FilesystemNode::createWriteStream() {
	return RomfsStream::makeFromPath(getPath(), true);
}

#endif //#ifdef __N64__
