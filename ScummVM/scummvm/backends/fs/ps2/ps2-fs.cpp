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

#if defined(__PLAYSTATION2__)

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL


#include "backends/fs/ps2/ps2-fs.h"

#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include "backends/platform/ps2/asyncfio.h"
#include "backends/platform/ps2/fileio.h"
#include "backends/platform/ps2/systemps2.h"
#include "backends/platform/ps2/ps2debug.h"

#include <fileXio_rpc.h>

#include "backends/platform/ps2/ps2temp.h"

#define DEFAULT_MODE (FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IROTH | FIO_S_IWOTH)

extern AsyncFio fio;
extern OSystem_PS2 *g_systemPs2;

const char *_lastPathComponent(const Common::String &str) {
	if (str.empty())
		return "";

	const char *start = str.c_str();
	const char *cur = start + str.size() - 2;

	while (cur >= start && *cur != '/' && *cur != ':') {
		--cur;
	}

	cur++;

	// printf("lastPathComponent path=%s token=%s\n", start, cur);

	return cur;
}

Ps2FilesystemNode::Ps2FilesystemNode() {
	printf("NEW FSNODE()\n");

	_isHere = true;
	_isDirectory = true;
	_isRoot = true;
	_verified = false;
	_displayName = Common::String("PlayStation 2");
	_path = "";
}

Ps2FilesystemNode::Ps2FilesystemNode(const Common::String &path) {
	printf("NEW FSNODE(%s)\n", path.c_str());

	_path = path;

	if (path.empty()) {
		_isHere = true;
		_isDirectory = true; /* root is always a dir */
		_isRoot = true;
		_displayName = Common::String("PlayStation 2");
		_verified = true;
	} else if (path.lastChar() == ':') {
		_isHere = true;
		_isDirectory = true; /* devs are always a dir */
		_isRoot = false;
		_displayName = getDeviceDescription();
		_verified = true;
	} else {
		_verified = false;
		doverify();
		if (!_isHere)
			return;

		_displayName = _lastPathComponent(_path);

		if (_isDirectory && _path.lastChar() != '/')
			_path+= '/';

		_isRoot = false;
	}
}

Ps2FilesystemNode::Ps2FilesystemNode(const Common::String &path, bool verify) {
	printf("NEW FSNODE(%s, %d)\n", path.c_str(), verify);

	_path = path;

	if (path.empty()) {
		_isHere = true;
		_isDirectory = true; /* root is always a dir */
		_isRoot = true;
		_displayName = Common::String("PlayStation 2");
		_verified = true;
	} else if (path.lastChar() == ':') {
		_isHere = true;
		_isDirectory = true; /* devs are always a dir */
		_isRoot = false;
		_displayName = getDeviceDescription();
		_verified = true;
	} else {
		_verified = false;
		if (verify) {
			doverify();

			if (!_isHere)
				return;

		} else {
			_verified = false;
			_isDirectory = false;
			_isHere = false; // true
		}

		_displayName = _lastPathComponent(_path);

		if (_isDirectory && _path.lastChar() != '/')
			_path+= '/';

		_isRoot = false;
	}
}

Ps2FilesystemNode::Ps2FilesystemNode(const Ps2FilesystemNode *node) {
	_displayName = node->_displayName;
	_isDirectory = node->_isDirectory;
	_path = node->_path;
	_isRoot = node->_isRoot;
	_isHere = node->_isHere;
	_verified = node->_verified;
}

void Ps2FilesystemNode::doverify(void) {
	PS2Device medium;
	int fd;

	if (_verified)
		return;

	_verified = true;

	printf(" verify: %s -> ", _path.c_str());

#if 0
	if (_path.empty()) {
		printf("PlayStation 2 Root !\n");
		_verified = true;
		return;
	}

	if (_path.lastChar() == ':') {
		printf("Dev: %s\n", _path.c_str());
		_verified = true;
		return;
	}
#endif

	if (_path[3] != ':' && _path[4] != ':') {
		printf("relative path !\n");
		_isHere = false;
		_isDirectory = false;
		return;
	}

	medium = _getDev(_path);
	if (medium == ERR_DEV) {
		_isHere = false;
		_isDirectory = false;
		return;
	}

	switch (medium) {
#if 0
	case HD_DEV: /*stat*/
	case USB_DEV:
		iox_stat_t stat;

		fileXioGetStat(_path.c_str(), &stat);
		fileXioWaitAsync(FXIO_WAIT, &fd);

		if (!fd) {
			printf("  yes [stat]\n");
			return true;
		}
	break;
#endif

	case CD_DEV: /*no stat*/
	case HD_DEV:
	case USB_DEV:
	case HOST_DEV:
	case MC_DEV:
#if 1
	fd = fio.open(_path.c_str(), O_RDONLY);

	printf("_path = %s -- fio.open -> %d\n", _path.c_str(), fd);

	if (fd >=0) {
		fio.close(fd);
		printf("  yes [open]\n");
		_isHere = true;
		if (medium==MC_DEV && _path.lastChar()=='/')
			_isDirectory = true;
		else
			_isDirectory = false;
		return;
	}

	fd = fio.dopen(_path.c_str());
	if (fd >=0) {
		fio.dclose(fd);
		printf("  yes [dopen]\n");
		_isHere = true;
		_isDirectory = true;
		return;
	}

#else
	fileXioOpen(_path.c_str(), O_RDONLY, DEFAULT_MODE);
	fileXioWaitAsync(FXIO_WAIT, &fd);
	if (fd>=0) {
		fileXioClose(fd);
		fileXioWaitAsync(FXIO_WAIT, &fd);
		return true;
	}

	fileXioDopen(_path.c_str());
	fileXioWaitAsync(FXIO_WAIT, &fd);
	if (fd>=0) {
		fileXioDclose(fd);
		fileXioWaitAsync(FXIO_WAIT, &fd);
		return true;
	}
#endif
	break;
	case ERR_DEV:
		_isHere = false;
		_isDirectory = false;
	break;
	}

	_isHere = false;
	_isDirectory = false;

	printf("  no\n");
	return;
}

AbstractFSNode *Ps2FilesystemNode::getChild(const Common::String &n) const {

	printf("getChild : %s\n", n.c_str());

	if (!_isDirectory)
		return NULL;

	if (_isRoot) {
		if (n.lastChar() == ':')
			return new Ps2FilesystemNode(n);
		else
			return NULL;
	}

	return new Ps2FilesystemNode(_path+n, 1);

/*
	int fd;

	if (_path == "pfs0:")
		fd = fio.dopen("pfs0:/");
	else
		fd = fio.dopen(_path.c_str());

	if (fd >= 0) {
		iox_dirent_t dirent;

		while (fio.dread(fd, &dirent) > 0) {
			if (strcmp(n.c_str(), dirent.name) == 0) {
				Ps2FilesystemNode *dirEntry = new Ps2FilesystemNode();

				dirEntry->_isHere = true;
				dirEntry->_isDirectory = (bool)(dirent.stat.mode & FIO_S_IFDIR);
				dirEntry->_isRoot = false;

				dirEntry->_path = _path;
				dirEntry->_path += dirent.name;
				if (dirEntry->_isDirectory && dirEntry->_path.lastChar() != '/')
					dirEntry->_path += '/';
				dirEntry->_displayName = dirent.name;

				dirEntry->_verified = true;

				fio.dclose(fd);
				return dirEntry;
			}
		}
		fio.dclose(fd);
	}

	return NULL;
*/
}

bool Ps2FilesystemNode::getChildren(AbstractFSList &list, ListMode mode, bool hidden) const {
	//TODO: honor the hidden flag

	// printf("getChildren\n");

	if (!_isDirectory)
		return false;

	if (_isRoot) {
		list.push_back(new Ps2FilesystemNode("cdfs:"));

		if (g_systemPs2->hddPresent())
			list.push_back(new Ps2FilesystemNode("pfs0:"));

		if (g_systemPs2->usbMassPresent())
			list.push_back(new Ps2FilesystemNode("mass:"));

		if (g_systemPs2->getBootDevice()==HOST_DEV || g_systemPs2->netPresent())
			list.push_back(new Ps2FilesystemNode("host:"));

		if (g_systemPs2->mcPresent())
			list.push_back(new Ps2FilesystemNode("mc0:"));

		return true;
	} else {
		int fd;

		if (_path == "pfs0:")
			fd = fio.dopen("pfs0:/");
		else
			fd = fio.dopen(_path.c_str());

		// printf("dopen = %d\n", fd);

		if (fd >= 0) {
			iox_dirent_t dirent;
			Ps2FilesystemNode dirEntry;
			int dreadRes;
			while ((dreadRes = fio.dread(fd, &dirent)) > 0) {

				if (dirent.name[0] == '.')
					continue; // ignore '.' and '..'

				if ( (mode == Common::FSNode::kListAll) ||

					((mode == Common::FSNode::kListDirectoriesOnly) &&
					 (dirent.stat.mode & FIO_S_IFDIR)) ||

				    ((mode == Common::FSNode::kListFilesOnly) &&
					 !(dirent.stat.mode & FIO_S_IFDIR)) ) {

					dirEntry._isHere = true;
					dirEntry._isDirectory = (bool)(dirent.stat.mode & FIO_S_IFDIR);
					dirEntry._isRoot = false;

					dirEntry._path = _path;
					dirEntry._path += dirent.name;
					if (dirEntry._isDirectory && dirEntry._path.lastChar() != '/')
						dirEntry._path += '/';
					dirEntry._displayName = dirent.name;

					dirEntry._verified = true;

					list.push_back(new Ps2FilesystemNode(&dirEntry));
				}
			}
			fio.dclose(fd);
			return true;
		}
	}
	return false;
}

AbstractFSNode *Ps2FilesystemNode::getParent() const {
	// printf("Ps2FilesystemNode::getParent : path = %s\n", _path.c_str());

	if (_isRoot)
		return new Ps2FilesystemNode(this); // FIXME : 0 ???

	if (_path.lastChar() == ':') // devs
		return new Ps2FilesystemNode(); // N: default is root

	const char *start = _path.c_str();
	const char *end = _lastPathComponent(_path);

	Common::String str(start, end - start);
	// printf("  parent = %s\n", str.c_str());

	return new Ps2FilesystemNode(str, true);
}

const char *Ps2FilesystemNode::getDeviceDescription() const {
	if (strncmp(_path.c_str(), "cdfs", 4) == 0)
		return "DVD Drive";
	else if (strncmp(_path.c_str(), "pfs0", 4) == 0)
		return "Harddisk";
	else if (strncmp(_path.c_str(), "mass", 4) == 0)
		return "USB Mass Storage";
	else if (strncmp(_path.c_str(), "host", 4) == 0)
		return "Host";
	else if (strncmp(_path.c_str(), "mc0", 3) == 0)
		return "Memory Card";
	else
		return "WTF ???";
}

Common::SeekableReadStream *Ps2FilesystemNode::createReadStream() {
	Common::SeekableReadStream *ss = PS2FileStream::makeFromPath(getPath(), false);
	return ss;
}

Common::WriteStream *Ps2FilesystemNode::createWriteStream() {
	return PS2FileStream::makeFromPath(getPath(), true);
}

#endif
