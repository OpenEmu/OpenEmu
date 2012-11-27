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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/textconsole.h"
#include "sword25/kernel/filesystemutil.h"
#include "sword25/kernel/persistenceservice.h"

namespace Sword25 {

Common::String FileSystemUtil::getUserdataDirectory() {
	// FIXME: This code is a hack which bypasses the savefile API,
	// and should eventually be removed.
	Common::String path = ConfMan.get("savepath");

	if (path.empty()) {
		error("No save path has been defined");
		return "";
	}

	// Return the path
	return path;
}

Common::String FileSystemUtil::getPathSeparator() {
	// FIXME: This code is a hack which bypasses the savefile API,
	// and should eventually be removed.
	return Common::String("/");
}

bool FileSystemUtil::fileExists(const Common::String &filename) {
	Common::File f;
	if (f.exists(filename))
		return true;

	// Check if the file exists in the save folder
	Common::FSNode folder(PersistenceService::getSavegameDirectory());
	Common::FSNode fileNode = folder.getChild(getPathFilename(filename));
	return fileNode.exists();
}

Common::String FileSystemUtil::getPathFilename(const Common::String &path) {
	for (int i = path.size() - 1; i >= 0; --i) {
		if ((path[i] == '/') || (path[i] == '\\')) {
			return Common::String(&path.c_str()[i + 1]);
		}
	}

	return path;
}

} // End of namespace Sword25
