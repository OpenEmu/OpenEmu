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

#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "backends/fs/psp/psp-fs-factory.h"
#include "backends/fs/psp/psp-fs.h"
#include "backends/platform/psp/powerman.h"

#include <unistd.h>

namespace Common {
DECLARE_SINGLETON(PSPFilesystemFactory);
}

AbstractFSNode *PSPFilesystemFactory::makeRootFileNode() const {
	return new PSPFilesystemNode();
}

AbstractFSNode *PSPFilesystemFactory::makeCurrentDirectoryFileNode() const {
	char buf[MAXPATHLEN];
	char *ret = 0;

	PowerMan.beginCriticalSection();
	ret = getcwd(buf, MAXPATHLEN);
	PowerMan.endCriticalSection();

	return (ret ? new PSPFilesystemNode(buf) : NULL);
}

AbstractFSNode *PSPFilesystemFactory::makeFileNodePath(const Common::String &path) const {
	return new PSPFilesystemNode(path, true);
}

#endif
