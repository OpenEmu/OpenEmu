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

#ifndef KYRA_PAK_H
#define KYRA_PAK_H

#include "util.h"
#include <string.h>

class PAKFile {
public:
	PAKFile() : _fileList(0), _isAmiga(false) {}
	~PAKFile() { delete _fileList; }

	bool loadFile(const char *file, const bool isAmiga);
	bool saveFile(const char *file);
	void clearFile() { delete _fileList; _fileList = 0; }

	uint32 getFileSize() const { return _fileList->getTableSize()+5+4+_fileList->getFileSize(); }

	void drawFileList();

	bool outputAllFiles();

	bool outputFile(const char *file) { return outputFileAs(file, file); }
	bool outputFileAs(const char *file, const char *outputName);
	const uint8 *getFileData(const char *file, uint32 *size);

	bool addFile(const char *name, const char *file);
	bool addFile(const char *name, uint8 *data, uint32 size);

	bool removeFile(const char *name);
public:
	struct FileList {
		FileList() : filename(0), size(0), data(0), next(0) {}
		~FileList() {
			delete[] filename;
			delete[] data;
			delete next;
		}

		FileList *findEntry(const char *f) {
			for (FileList *cur = this; cur; cur = cur->next) {
				if (scumm_stricmp(cur->filename, f) != 0)
					continue;
				return cur;
			}
			return 0;
		}

		const FileList *findEntry(const char *f) const {
			for (const FileList *cur = this; cur; cur = cur->next) {
				if (scumm_stricmp(cur->filename, f) != 0)
					continue;
				return cur;
			}
			return 0;
		}

		void addEntry(FileList *e) {
			if (next)
				next->addEntry(e);
			else
				next = e;
		}
		uint32 getTableSize() const {
			return strlen(filename)+1+4+((next != 0) ? next->getTableSize() : 0);
		}
		uint32 getFileSize() const {
			return size + (next != 0 ? next->getFileSize() : 0);
		}

		char *filename;
		uint32 size;
		uint8 *data;

		FileList *next;
	};

	typedef const FileList cFileList;

	cFileList *getFileList() const { return _fileList; }
private:
	FileList *_fileList;
	bool _isAmiga;
};

#endif
