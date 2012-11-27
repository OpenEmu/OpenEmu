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
#define FORBIDDEN_SYMBOL_EXCEPTION_exit		//Needed for IRIX's unistd.h

#include "backends/fs/posix/posix-fs-factory.h"
#include "backends/fs/posix/posix-fs.h"

AbstractFSNode *POSIXFilesystemFactory::makeRootFileNode() const {
	return new POSIXFilesystemNode("/");
}

AbstractFSNode *POSIXFilesystemFactory::makeCurrentDirectoryFileNode() const {
	char buf[MAXPATHLEN];
	return getcwd(buf, MAXPATHLEN) ? new POSIXFilesystemNode(buf) : NULL;
}

AbstractFSNode *POSIXFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	assert(!path.empty());
	return new POSIXFilesystemNode(path);
}
#endif
