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

#ifndef WINDOWS_FILESYSTEM_H
#define WINDOWS_FILESYSTEM_H

#include "backends/fs/abstract-fs.h"

#if defined(ARRAYSIZE)
#undef ARRAYSIZE
#endif
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#ifdef _WIN32_WCE
#undef GetCurrentDirectory
#endif
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

/**
 * Implementation of the ScummVM file system API based on Windows API.
 *
 * Parts of this class are documented in the base interface class, AbstractFSNode.
 */
class WindowsFilesystemNode : public AbstractFSNode {
protected:
	Common::String _displayName;
	Common::String _path;
	bool _isDirectory;
	bool _isPseudoRoot;
	bool _isValid;

public:
	/**
	 * Creates a WindowsFilesystemNode with the root node as path.
	 *
	 * In regular windows systems, a virtual root path is used "".
	 * In windows CE, the "\" root is used instead.
	 */
	WindowsFilesystemNode();

	/**
	 * Creates a WindowsFilesystemNode for a given path.
	 *
	 * Examples:
	 *			path=c:\foo\bar.txt, currentDir=false -> c:\foo\bar.txt
	 *			path=c:\foo\bar.txt, currentDir=true -> current directory
	 *			path=NULL, currentDir=true -> current directory
	 *
	 * @param path Common::String with the path the new node should point to.
	 * @param currentDir if true, the path parameter will be ignored and the resulting node will point to the current directory.
	 */
	WindowsFilesystemNode(const Common::String &path, const bool currentDir);

	virtual bool exists() const;
	virtual Common::String getDisplayName() const { return _displayName; }
	virtual Common::String getName() const { return _displayName; }
	virtual Common::String getPath() const { return _path; }
	virtual bool isDirectory() const { return _isDirectory; }
	virtual bool isReadable() const;
	virtual bool isWritable() const;

	virtual AbstractFSNode *getChild(const Common::String &n) const;
	virtual bool getChildren(AbstractFSList &list, ListMode mode, bool hidden) const;
	virtual AbstractFSNode *getParent() const;

	virtual Common::SeekableReadStream *createReadStream();
	virtual Common::WriteStream *createWriteStream();

private:
	/**
	 * Adds a single WindowsFilesystemNode to a given list.
	 * This method is used by getChildren() to populate the directory entries list.
	 *
	 * @param list         List to put the file entry node in.
	 * @param mode         Mode to use while adding the file entry to the list.
	 * @param base         Common::String with the directory being listed.
	 * @param hidden       true if hidden files should be added, false otherwise
	 * @param find_data    Describes a file that the FindFirstFile, FindFirstFileEx, or FindNextFile functions find.
	 */
	static void addFile(AbstractFSList &list, ListMode mode, const char *base, bool hidden, WIN32_FIND_DATA* find_data);

	/**
	 * Converts a Unicode string to Ascii format.
	 *
	 * @param str Common::String to convert from Unicode to Ascii.
	 * @return str in Ascii format.
	 */
	static char *toAscii(TCHAR *str);

	/**
	 * Converts an Ascii string to Unicode format.
	 *
	 * @param str Common::String to convert from Ascii to Unicode.
	 * @return str in Unicode format.
	 */
	static const TCHAR* toUnicode(const char *str);
};

#endif
