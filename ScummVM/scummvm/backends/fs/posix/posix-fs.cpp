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

#if defined(POSIX) || defined(PLAYSTATION3) || defined(__LIBRETRO__)

// Re-enable some forbidden symbols to avoid clashes with stat.h and unistd.h.
// Also with clock() in sys/time.h in some Mac OS X SDKs.
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_getenv
#define FORBIDDEN_SYMBOL_EXCEPTION_exit		//Needed for IRIX's unistd.h

#include "backends/fs/posix/posix-fs.h"
#include "backends/fs/stdiostream.h"
#include "common/algorithm.h"

#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>

#ifdef __OS2__
#define INCL_DOS
#include <os2.h>
#endif


void POSIXFilesystemNode::setFlags() {
	struct stat st;

	_isValid = (0 == stat(_path.c_str(), &st));
	_isDirectory = _isValid ? S_ISDIR(st.st_mode) : false;
}

POSIXFilesystemNode::POSIXFilesystemNode(const Common::String &p) {
	assert(p.size() > 0);

	// Expand "~/" to the value of the HOME env variable
	if (p.hasPrefix("~/")) {
		const char *home = getenv("HOME");
		if (home != NULL && strlen(home) < MAXPATHLEN) {
			_path = home;
			// Skip over the tilda.  We know that p contains at least
			// two chars, so this is safe:
			_path += p.c_str() + 1;
		}
	} else {
		_path = p;
	}

#ifdef __OS2__
	// On OS/2, 'X:/' is a root of drive X, so we should not remove that last
	// slash.
	if (!(_path.size() == 3 && _path.hasSuffix(":/")))
#endif
	// Normalize the path (that is, remove unneeded slashes etc.)
	_path = Common::normalizePath(_path, '/');
	_displayName = Common::lastPathComponent(_path, '/');

	// TODO: should we turn relative paths into absolute ones?
	// Pro: Ensures the "getParent" works correctly even for relative dirs.
	// Contra: The user may wish to use (and keep!) relative paths in his
	//   config file, and converting relative to absolute paths may hurt him...
	//
	// An alternative approach would be to change getParent() to work correctly
	// if "_path" is the empty string.
#if 0
	if (!_path.hasPrefix("/")) {
		char buf[MAXPATHLEN+1];
		getcwd(buf, MAXPATHLEN);
		strcat(buf, "/");
		_path = buf + _path;
	}
#endif
	// TODO: Should we enforce that the path is absolute at this point?
	//assert(_path.hasPrefix("/"));

	setFlags();
}

AbstractFSNode *POSIXFilesystemNode::getChild(const Common::String &n) const {
	assert(!_path.empty());
	assert(_isDirectory);

	// Make sure the string contains no slashes
	assert(!n.contains('/'));

	// We assume here that _path is already normalized (hence don't bother to call
	//  Common::normalizePath on the final path).
	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	return makeNode(newPath);
}

bool POSIXFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	assert(_isDirectory);

#ifdef __OS2__
	if (_path == "/") {
		// Special case for the root dir: List all DOS drives
		ULONG ulDrvNum;
		ULONG ulDrvMap;

		DosQueryCurrentDisk(&ulDrvNum, &ulDrvMap);

		for (int i = 0; i < 26; i++) {
			if (ulDrvMap & 1) {
				char drive_root[] = "A:/";
				drive_root[0] += i;

                POSIXFilesystemNode *entry = new POSIXFilesystemNode();
				entry->_isDirectory = true;
				entry->_isValid = true;
				entry->_path = drive_root;
				entry->_displayName = "[" + Common::String(drive_root, 2) + "]";
				myList.push_back(entry);
			}

			ulDrvMap >>= 1;
		}

		return true;
	}
#endif

	DIR *dirp = opendir(_path.c_str());
	struct dirent *dp;

	if (dirp == NULL)
		return false;

	// loop over dir entries using readdir
	while ((dp = readdir(dirp)) != NULL) {
		// Skip 'invisible' files if necessary
		if (dp->d_name[0] == '.' && !hidden) {
			continue;
		}
		// Skip '.' and '..' to avoid cycles
		if ((dp->d_name[0] == '.' && dp->d_name[1] == 0) || (dp->d_name[0] == '.' && dp->d_name[1] == '.')) {
			continue;
		}

		// Start with a clone of this node, with the correct path set
		POSIXFilesystemNode entry(*this);
		entry._displayName = dp->d_name;
		if (_path.lastChar() != '/')
			entry._path += '/';
		entry._path += entry._displayName;

#if defined(SYSTEM_NOT_SUPPORTING_D_TYPE)
		/* TODO: d_type is not part of POSIX, so it might not be supported
		 * on some of our targets. For those systems where it isn't supported,
		 * add this #elif case, which tries to use stat() instead.
		 *
		 * The d_type method is used to avoid costly recurrent stat() calls in big
		 * directories.
		 */
		entry.setFlags();
#else
		if (dp->d_type == DT_UNKNOWN) {
			// Fall back to stat()
			entry.setFlags();
		} else {
			entry._isValid = (dp->d_type == DT_DIR) || (dp->d_type == DT_REG) || (dp->d_type == DT_LNK);
			if (dp->d_type == DT_LNK) {
				struct stat st;
				if (stat(entry._path.c_str(), &st) == 0)
					entry._isDirectory = S_ISDIR(st.st_mode);
				else
					entry._isDirectory = false;
			} else {
				entry._isDirectory = (dp->d_type == DT_DIR);
			}
		}
#endif

		// Skip files that are invalid for some reason (e.g. because we couldn't
		// properly stat them).
		if (!entry._isValid)
			continue;

		// Honor the chosen mode
		if ((mode == Common::FSNode::kListFilesOnly && entry._isDirectory) ||
			(mode == Common::FSNode::kListDirectoriesOnly && !entry._isDirectory))
			continue;

		myList.push_back(new POSIXFilesystemNode(entry));
	}
	closedir(dirp);

	return true;
}

AbstractFSNode *POSIXFilesystemNode::getParent() const {
	if (_path == "/")
		return 0;	// The filesystem root has no parent

#ifdef __OS2__
    if (_path.size() == 3 && _path.hasSuffix(":/"))
        // This is a root directory of a drive
        return makeNode("/");   // return a virtual root for a list of drives
#endif

	const char *start = _path.c_str();
	const char *end = start + _path.size();

	// Strip of the last component. We make use of the fact that at this
	// point, _path is guaranteed to be normalized
	while (end > start && *(end-1) != '/')
		end--;

	if (end == start) {
		// This only happens if we were called with a relative path, for which
		// there simply is no parent.
		// TODO: We could also resolve this by assuming that the parent is the
		//       current working directory, and returning a node referring to that.
		return 0;
	}

	return makeNode(Common::String(start, end));
}

Common::SeekableReadStream *POSIXFilesystemNode::createReadStream() {
	return StdioStream::makeFromPath(getPath(), false);
}

Common::WriteStream *POSIXFilesystemNode::createWriteStream() {
	return StdioStream::makeFromPath(getPath(), true);
}

#endif //#if defined(POSIX)
