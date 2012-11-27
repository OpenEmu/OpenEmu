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

#if defined(__PSP__)

// Disable printf override in common/forbidden.h to avoid
// clashes with pspdebug.h from the PSP SDK.
// That header file uses
//   __attribute__((format(printf,1,2)));
// which gets messed up by our override mechanism; this could
// be avoided by either changing the PSP SDK to use the equally
// legal and valid
//   __attribute__((format(__printf__,1,2)));
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the PSP port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir

#include "backends/fs/psp/psp-fs.h"
#include "backends/fs/psp/psp-stream.h"
#include "common/bufferedstream.h"
#include "engines/engine.h"

#include <sys/stat.h>
#include <unistd.h>

#include <pspkernel.h>

#define	ROOT_PATH	"ms0:/"

//#define __PSP_PRINT_TO_FILE__
//#define __PSP_DEBUG_FUNCS__ /* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */
#include "backends/platform/psp/trace.h"

PSPFilesystemNode::PSPFilesystemNode() {
	_isDirectory = true;
	_displayName = "Root";
	_isValid = true;
	_path = ROOT_PATH;
}

PSPFilesystemNode::PSPFilesystemNode(const Common::String &p, bool verify) {
	DEBUG_ENTER_FUNC();
	assert(p.size() > 0);

	_path = p;
	_displayName = lastPathComponent(_path, '/');
	_isValid = true;
	_isDirectory = true;

	PSP_DEBUG_PRINT_FUNC("path [%s]\n", _path.c_str());

	if (verify) {
		struct stat st;
		if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
			PSP_DEBUG_PRINT_FUNC("Suspended\n");
		_isValid = (0 == stat(_path.c_str(), &st));
		PowerMan.endCriticalSection();
		_isDirectory = S_ISDIR(st.st_mode);
	}
}

bool PSPFilesystemNode::exists() const {
	DEBUG_ENTER_FUNC();
	int ret = 0;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");	// Make sure to block in case of suspend

	PSP_DEBUG_PRINT_FUNC("path [%s]\n", _path.c_str());

	ret = access(_path.c_str(), F_OK);
	PowerMan.endCriticalSection();

	return (ret == 0);
}

bool PSPFilesystemNode::isReadable() const {
	DEBUG_ENTER_FUNC();
	int ret = 0;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");	// Make sure to block in case of suspend

	PSP_DEBUG_PRINT_FUNC("path [%s]\n", _path.c_str());

	ret = access(_path.c_str(), R_OK);
	PowerMan.endCriticalSection();

	return (ret == 0);
}

bool PSPFilesystemNode::isWritable() const {
	DEBUG_ENTER_FUNC();
	int ret = 0;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");	// Make sure to block in case of suspend

	PSP_DEBUG_PRINT_FUNC("path [%s]\n", _path.c_str());

	ret = access(_path.c_str(), W_OK);
	PowerMan.endCriticalSection();

	return ret == 0;
}


AbstractFSNode *PSPFilesystemNode::getChild(const Common::String &n) const {
	DEBUG_ENTER_FUNC();
	// FIXME: Pretty lame implementation! We do no error checking to speak
	// of, do not check if this is a special node, etc.
	assert(_isDirectory);

	Common::String newPath(_path);
	if (_path.lastChar() != '/')
		newPath += '/';
	newPath += n;

	PSP_DEBUG_PRINT_FUNC("child [%s]\n", newPath.c_str());

	AbstractFSNode *node = new PSPFilesystemNode(newPath, true);

	return node;
}

bool PSPFilesystemNode::getChildren(AbstractFSList &myList, ListMode mode, bool hidden) const {
	DEBUG_ENTER_FUNC();
	assert(_isDirectory);

	//TODO: honor the hidden flag

	bool ret = true;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");	// Make sure to block in case of suspend

	PSP_DEBUG_PRINT_FUNC("Current path[%s]\n", _path.c_str());

	int dfd  = sceIoDopen(_path.c_str());
	if (dfd > 0) {
		SceIoDirent dir;
		memset(&dir, 0, sizeof(dir));

		while (sceIoDread(dfd, &dir) > 0) {
			// Skip 'invisible files
			if (dir.d_name[0] == '.')
				continue;

			PSPFilesystemNode entry;

			entry._isValid = true;
			entry._displayName = dir.d_name;

			Common::String newPath(_path);
			if (newPath.lastChar() != '/')
				newPath += '/';
			newPath += dir.d_name;

			entry._path = newPath;
			entry._isDirectory = dir.d_stat.st_attr & FIO_SO_IFDIR;

			PSP_DEBUG_PRINT_FUNC("Child[%s], %s\n", entry._path.c_str(), entry._isDirectory ? "dir" : "file");

			// Honor the chosen mode
			if ((mode == Common::FSNode::kListFilesOnly && entry._isDirectory) ||
			        (mode == Common::FSNode::kListDirectoriesOnly && !entry._isDirectory))
				continue;

			myList.push_back(new PSPFilesystemNode(entry));
		}

		sceIoDclose(dfd);
		ret = true;
	} else { // dfd <= 0
		ret = false;
	}

	PowerMan.endCriticalSection();

	return ret;
}

AbstractFSNode *PSPFilesystemNode::getParent() const {
	DEBUG_ENTER_FUNC();
	if (_path == ROOT_PATH)
		return 0;

	PSP_DEBUG_PRINT_FUNC("current[%s]\n", _path.c_str());

	const char *start = _path.c_str();
	const char *end = lastPathComponent(_path, '/');

	AbstractFSNode *node = new PSPFilesystemNode(Common::String(start, end - start), false);

	return node;
}

Common::SeekableReadStream *PSPFilesystemNode::createReadStream() {
	const uint32 READ_BUFFER_SIZE = 1024;

	Common::SeekableReadStream *stream = PspIoStream::makeFromPath(getPath(), false);

	return Common::wrapBufferedSeekableReadStream(stream, READ_BUFFER_SIZE, DisposeAfterUse::YES);
}

Common::WriteStream *PSPFilesystemNode::createWriteStream() {
	const uint32 WRITE_BUFFER_SIZE = 1024;

	Common::WriteStream *stream = PspIoStream::makeFromPath(getPath(), true);

	return Common::wrapBufferedWriteStream(stream, WRITE_BUFFER_SIZE);
}

#endif //#ifdef __PSP__
