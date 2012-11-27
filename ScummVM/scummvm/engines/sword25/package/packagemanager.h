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

/*
 * BS_PackageManager
 * -----------------
 * This is the package manager interface, that contains all the methods that a package manager
 * must implement.
 * In the package manager, note the following:
 * 1. It creates a completely new (virtual) directory tree in the packages and directories
 *    can be mounted.
 * 2. To seperate elements of a directory path '/' must be used rather than '\'
 * 3. LoadDirectoryAsPackage should only be used for testing. The final release will be
 *    have all files in packages.
 *
 * Autor: Malte Thiesen, $author$
 */

#ifndef SWORD25_PACKAGE_MANAGER_H
#define SWORD25_PACKAGE_MANAGER_H

#include "common/archive.h"
#include "common/array.h"
#include "common/fs.h"
#include "common/str.h"

#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/service.h"

namespace Sword25 {

// Class definitions

/**
 * The Package Manager interface
 *
 * 1. It creates a completely new (virtual) directory tree in the packages and directories
 *    can be mounted.
 * 2. To seperate elements of a directory path '/' must be used rather than '\'
 * 3. LoadDirectoryAsPackage should only be used for testing. The final release will be
 *    have all files in packages.
 */
class PackageManager : public Service {
private:
	class ArchiveEntry {
	public:
		Common::Archive *archive;
		Common::String _mountPath;

		ArchiveEntry(Common::Archive *archive_, const Common::String &mountPath_):
			archive(archive_), _mountPath(mountPath_) {
		}
		~ArchiveEntry() {
			delete archive;
		}
	};

	Common::String _currentDirectory;
	Common::FSNode _rootFolder;
	Common::List<ArchiveEntry *> _archiveList;

	Common::ArchiveMemberPtr getArchiveMember(const Common::String &fileName);

public:
	PackageManager(Kernel *pKernel);
	~PackageManager();

	enum FILE_TYPES {
		FT_DIRECTORY    = (1 << 0),
		FT_FILE         = (1 << 1)
	};

	/**
	 * Mounts the contents of a package in the directory specified in the directory tree.
	 * @param FileName      The filename of the package to mount
	 * @param MountPosition The directory name under which the package should be mounted
	 * @return              Returns true if the mount was successful, otherwise false.
	 */
	bool loadPackage(const Common::String &fileName, const Common::String &mountPosition);
	/**
	 * Mounts the contents of a directory in the specified directory in the directory tree.
	 * @param               The name of the directory to mount
	 * @param MountPosition The directory name under which the package should be mounted
	 * @return              Returns true if the mount was successful, otherwise false.
	 */
	bool loadDirectoryAsPackage(const Common::String &directoryName, const Common::String &mountPosition);
	/**
	 * Downloads a file from the directory tree
	 * @param FileName      The filename of the file to load
	 * @param pFileSize     Pointer to the variable that will contain the size of the loaded file. The deafult is NULL.
	 * @return              Specifies a pointer to the loaded data of the file
	 * @remark              The client must not forget to release the data of the file using BE_DELETE_A.
	 */
	byte *getFile(const Common::String &fileName, uint *pFileSize = NULL);

	/**
	 * Returns a stream from file file from the directory tree
	 * @param FileName      The filename of the file to load
	 * @return              Pointer to the stream object
	 */
	Common::SeekableReadStream *getStream(const Common::String &fileName);
	/**
	 * Downloads an XML file and prefixes it with an XML Version key, since the XML files don't contain it,
	 * and it is required for ScummVM to correctly parse the XML.
	 * @param FileName      The filename of the file to load
	 * @param pFileSize     Pointer to the variable that will contain the size of the loaded file. The deafult is NULL.
	 * @return              Specifies a pointer to the loaded data of the file
	 * @remark              The client must not forget to release the data of the file using BE_DELETE_A.
	 */
	char *getXmlFile(const Common::String &fileName, uint *pFileSize = NULL) {
		const char *versionStr = "<?xml version=\"1.0\"?>";
		uint fileSize;
		char *data = (char *)getFile(fileName, &fileSize);
		char *result = (char *)malloc(fileSize + strlen(versionStr) + 1);
		if (!result)
			error("[PackageManager::getXmlFile] Cannot allocate memory");

		strcpy(result, versionStr);
		Common::copy(data, data + fileSize, result + strlen(versionStr));
		result[fileSize + strlen(versionStr)] = '\0';

		delete[] data;
		if (pFileSize)
			*pFileSize = fileSize + strlen(versionStr);

		return result;
	}

	/**
	 * Returns the path to the current directory.
	 * @return              Returns a string containing the path to the current directory.
	 * If the path could not be determined, an empty string is returned.
	 * @remark              For cutting path elements '\' is used rather than '/' elements.
	 */
	Common::String getCurrentDirectory() { return _currentDirectory; }
	/**
	 * Changes the current directory.
	 * @param Directory     The path to the new directory. The path can be relative.
	 * @return              Returns true if the operation was successful, otherwise false.
	 * @remark              For cutting path elements '\' is used rather than '/' elements.
	 */
	bool changeDirectory(const Common::String &directory);
	/**
	 * Returns the absolute path to a file in the directory tree.
	 * @param FileName      The filename of the file whose absolute path is to be determined.
	 * These parameters may include both relative and absolute paths.
	 * @return              Returns an absolute path to the given file.
	 * @remark              For cutting path elements '\' is used rather than '/' elements.
	 */
	Common::String getAbsolutePath(const Common::String &fileName);
	/**
	 * Creates a BS_PackageManager::FileSearch object to search for files
	 * @param Filter        Specifies the search string. Wildcards of '*' and '?' are allowed
	 * @param Path          Specifies the directory that should be searched.
	 * @param TypeFilter    A combination of flags BS_PackageManager::FT_DIRECTORY and BS_PackageManager::FT_FILE.
	 * These flags indicate whether to search for files, directories, or both.
	 * The default is BS_PackageManager::FT_DIRECTORY | BS_PackageManager::FT_FILE
	 * @return              Specifies a pointer to a BS_PackageManager::FileSearch object, or NULL if no file was found.
	 * @remark              Do not forget to delete the object after use.
	*/
	int doSearch(Common::ArchiveMemberList &list, const Common::String &filter, const Common::String &path, uint typeFilter = FT_DIRECTORY | FT_FILE);

	/**
	 * Determines whether a file exists
	 * @param FileName      The filename
	 * @return              Returns true if the file exists, otherwise false.
	 */
	bool fileExists(const Common::String &FileName);

private:
	bool registerScriptBindings();
};

} // End of namespace Sword25

#endif
