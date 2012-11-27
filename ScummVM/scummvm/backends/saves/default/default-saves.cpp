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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#if !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)

#include "backends/saves/default/default-saves.h"

#include "common/savefile.h"
#include "common/util.h"
#include "common/fs.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/zlib.h"

#ifndef _WIN32_WCE
#include <errno.h>	// for removeSavefile()
#endif

DefaultSaveFileManager::DefaultSaveFileManager() {
}

DefaultSaveFileManager::DefaultSaveFileManager(const Common::String &defaultSavepath) {
	ConfMan.registerDefault("savepath", defaultSavepath);
}


void DefaultSaveFileManager::checkPath(const Common::FSNode &dir) {
	clearError();
	if (!dir.exists()) {
		setError(Common::kPathDoesNotExist, "The savepath '"+dir.getPath()+"' does not exist");
	} else if (!dir.isDirectory()) {
		setError(Common::kPathNotDirectory, "The savepath '"+dir.getPath()+"' is not a directory");
	}
}

Common::StringArray DefaultSaveFileManager::listSavefiles(const Common::String &pattern) {
	Common::String savePathName = getSavePath();
	checkPath(Common::FSNode(savePathName));
	if (getError().getCode() != Common::kNoError)
		return Common::StringArray();

	// recreate FSNode since checkPath may have changed/created the directory
	Common::FSNode savePath(savePathName);

	Common::FSDirectory dir(savePath);
	Common::ArchiveMemberList savefiles;
	Common::StringArray results;
	Common::String search(pattern);

	if (dir.listMatchingMembers(savefiles, search) > 0) {
		for (Common::ArchiveMemberList::const_iterator file = savefiles.begin(); file != savefiles.end(); ++file) {
			results.push_back((*file)->getName());
		}
	}

	return results;
}

Common::InSaveFile *DefaultSaveFileManager::openForLoading(const Common::String &filename) {
	// Ensure that the savepath is valid. If not, generate an appropriate error.
	Common::String savePathName = getSavePath();
	checkPath(Common::FSNode(savePathName));
	if (getError().getCode() != Common::kNoError)
		return 0;

	// recreate FSNode since checkPath may have changed/created the directory
	Common::FSNode savePath(savePathName);

	Common::FSNode file = savePath.getChild(filename);
	if (!file.exists())
		return 0;

	// Open the file for reading
	Common::SeekableReadStream *sf = file.createReadStream();

	return Common::wrapCompressedReadStream(sf);
}

Common::OutSaveFile *DefaultSaveFileManager::openForSaving(const Common::String &filename, bool compress) {
	// Ensure that the savepath is valid. If not, generate an appropriate error.
	Common::String savePathName = getSavePath();
	checkPath(Common::FSNode(savePathName));
	if (getError().getCode() != Common::kNoError)
		return 0;

	// recreate FSNode since checkPath may have changed/created the directory
	Common::FSNode savePath(savePathName);

	Common::FSNode file = savePath.getChild(filename);

	// Open the file for saving
	Common::WriteStream *sf = file.createWriteStream();

	return compress ? Common::wrapCompressedWriteStream(sf) : sf;
}

bool DefaultSaveFileManager::removeSavefile(const Common::String &filename) {
	Common::String savePathName = getSavePath();
	checkPath(Common::FSNode(savePathName));
	if (getError().getCode() != Common::kNoError)
		return false;

	// recreate FSNode since checkPath may have changed/created the directory
	Common::FSNode savePath(savePathName);

	Common::FSNode file = savePath.getChild(filename);

	// FIXME: remove does not exist on all systems. If your port fails to
	// compile because of this, please let us know (scummvm-devel or Fingolfin).
	// There is a nicely portable workaround, too: Make this method overloadable.
	if (remove(file.getPath().c_str()) != 0) {
#ifndef _WIN32_WCE
		if (errno == EACCES)
			setError(Common::kWritePermissionDenied, "Search or write permission denied: "+file.getName());

		if (errno == ENOENT)
			setError(Common::kPathDoesNotExist, "removeSavefile: '"+file.getName()+"' does not exist or path is invalid");
#endif
		return false;
	} else {
		return true;
	}
}

Common::String DefaultSaveFileManager::getSavePath() const {

	Common::String dir;

	// Try to use game specific savepath from config
	dir = ConfMan.get("savepath");

	// Work around a bug (#999122) in the original 0.6.1 release of
	// ScummVM, which would insert a bad savepath value into config files.
	if (dir == "None") {
		ConfMan.removeKey("savepath", ConfMan.getActiveDomainName());
		ConfMan.flushToDisk();
		dir = ConfMan.get("savepath");
	}

#ifdef _WIN32_WCE
	if (dir.empty())
		dir = ConfMan.get("path");
#endif

	return dir;
}

#endif // !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)
