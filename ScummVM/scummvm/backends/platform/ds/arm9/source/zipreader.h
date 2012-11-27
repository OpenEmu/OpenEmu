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

#ifndef _ZIPREADER_H_
#define _ZIPREADER_H_

#include <nds/ndstypes.h>

#define ZF_SEARCH_START 0x08000000
#define ZF_SEARCH_END 0x09000000
#define ZF_SEARCH_STRIDE 16

class ZipFile {

	struct FileHeader {
		char magic[4];		// Header (0x04034B50)			0
		u16 minVersion;		// Version needed to extract	4
		u16 flags;			// Flags						6
		u16 comp;			// Compression method			8
		u16 modTime;		// Last modified file time		A
		u16 modDate;		// Last modified file data		C
		u32 crc32;			// CRC32						F
		u32 compSize;		// Compressed size
		u32 fileSize;		// Uncompressed file size
		u16 nameLength;		// Length of the filename
		u16 extraLength;	// Length of any extra data
	} __attribute__ ((packed));

	char *_zipFile;
	char _directory[128];

	bool _allFilesVisible;

	FileHeader *_currentFile;

public:
	ZipFile();

	bool isReady();

	// These operations set the current file
	bool restartFile();
	bool skipFile();
	bool findFile(const char *search);

	// These return the file's data and information
	char *getFile();
	int getFileSize();
	void getFileName(char *name);
	bool isDirectory();

	// These set the current directory
	void changeDirectory(const char *name);
	void changeToRoot();
	void setAllFilesVisible(bool state) { _allFilesVisible = state; }

	bool currentFileInFolder();
};


#endif
