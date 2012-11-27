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

// Disable symbol overrides for FILE as that is used in FLAC headers
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#include "common/str.h"
#include "common/util.h"
//#include <NDS/ARM9/console.h> //basic print funcionality
#include "backends/fs/ds/ds-fs.h"
#include "backends/fs/stdiostream.h"
#include "dsmain.h"
#include "fat/gba_nds_fat.h"
#include "common/bufferedstream.h"

namespace DS {

//////////////////////////////////////////////////////////////
// DSFileSystemNode - Flash ROM file system using Zip files //
//////////////////////////////////////////////////////////////

ZipFile*	DSFileSystemNode::_zipFile = NULL;	// FIXME: Avoid non-const global vars
char		currentDir[128];	// FIXME: Avoid non-const global vars
bool		readPastEndOfFile = false;	// FIXME: Avoid non-const global vars

enum {
	WRITE_BUFFER_SIZE = 512
};

DSFileSystemNode::DSFileSystemNode() {
	_displayName = "ds:/";
	_path = "ds:/";
	_isValid = true;
	_isDirectory = true;
	_path = "ds:/";

/*	if (!_archive) {
		_archive = (GBFS_FILE *) find_first_gbfs_file(scummdata);
		if (!_archive) consolePrintf("No GBFS archive found!\n");
	}*/

	if (!_zipFile) {
		_zipFile = new ZipFile();
	}
}

DSFileSystemNode::DSFileSystemNode(const Common::String& path) {
//	consolePrintf("--%s ",path.c_str());

	int lastSlash = 3;
	for (int r = 0; r < (int) path.size() - 1; r++) {
		if (path[r] == '\\') {
			lastSlash = r;
		}
	}

	_displayName = Common::String(path.c_str() + lastSlash + 1);
	_path = path;
//	_isValid = true;
//	_isDirectory = false;

	const char *pathStr = path.c_str();

	if (path.hasPrefix("ds:/")) {
		pathStr += 4;
	}

	if (*pathStr == '\0') {
		_isValid = true;
		_isDirectory = true;
		return;
	}

	_zipFile->setAllFilesVisible(true);
	if (_zipFile->findFile(pathStr)) {
		_isValid = true;
		_isDirectory = _zipFile->isDirectory();
	} else {
		_isValid = false;
		_isDirectory = false;
	}
	_zipFile->setAllFilesVisible(false);

//	consolePrintf("%s - Found: %d, Dir: %d\n", pathStr, _isValid, _isDirectory);
}

DSFileSystemNode::DSFileSystemNode(const Common::String& path, bool isDir) {
//	consolePrintf("--%s ",path.c_str());

	int lastSlash = 3;
	for (int r = 0; r < (int) path.size() - 1; r++) {
		if (path[r] == '\\') {
			lastSlash = r;
		}
	}

	_displayName = Common::String(path.c_str() + lastSlash + 1);
	_path = path;
	_isValid = true;
	_isDirectory = isDir;

//	consolePrintf("Found: %d, Dir: %d\n", _isValid, _isDirectory);
}

DSFileSystemNode::DSFileSystemNode(const DSFileSystemNode *node) {
	//TODO: not implemented?
}

AbstractFSNode *DSFileSystemNode::getChild(const Common::String& n) const {
	if (_path.lastChar() == '\\') {
		return new DSFileSystemNode(_path + n);
	} else {
		return new DSFileSystemNode(_path + "\\" + n);
	}

	return NULL;
}

bool DSFileSystemNode::getChildren(AbstractFSList &dirList, ListMode mode, bool hidden) const {
//	consolePrintf("Listdir\n");
//	consolePrintf("Directory\n");

	//TODO: honor the hidden flag

//	consolePrintf("This dir: %s\n", _path.c_str());

	if (_path.hasPrefix("ds:/")) {
		if (_path.size() > 4) {
			_zipFile->changeDirectory(_path.c_str() + 4);
		} else {
			_zipFile->changeToRoot();

/*			// This is the root dir, so add the RAM folder
			DSFileSystemNode *dsfsn = new DSFileSystemNode("ds:/ram");
			dsfsn->_isDirectory = true;
			dirList->push_back(wrap(dsfsn));
*/
		}
	} else {
		_zipFile->changeDirectory(_path.c_str());
	}

	if (_zipFile->restartFile()) {
		do {
			char n[128];
			_zipFile->getFileName(n);

//			consolePrintf("file: %s\n", n);
			if ( (_zipFile->isDirectory() && ((mode == Common::FSNode::kListDirectoriesOnly) || (mode == Common::FSNode::kListAll)) )
				|| (!_zipFile->isDirectory() && ((mode == Common::FSNode::kListFilesOnly) || (mode == Common::FSNode::kListAll)) ) )
			{
				DSFileSystemNode *dsfsn = new DSFileSystemNode("ds:/" + Common::String(n), _zipFile->isDirectory());
				dsfsn->_isDirectory = _zipFile->isDirectory();
				dirList.push_back((dsfsn));
			}

		} while (_zipFile->skipFile());
	}

	return true;
}

AbstractFSNode *DSFileSystemNode::getParent() const {
//	consolePrintf("parent\n");
	DSFileSystemNode *p;

	if (_path != "ds:/") {
		const char *path = (const char *)_path.c_str();
		int lastSlash = 4;

		for (uint r = 4; r < _path.size(); r++) {
			if (path[r] == '\\') {
				lastSlash = r;
			}
		}

		p = new DSFileSystemNode(Common::String(path, lastSlash));
		p->_isDirectory = true;
	} else {
		p = new DSFileSystemNode();
	}

	return p;
}

Common::SeekableReadStream *DSFileSystemNode::createReadStream() {
	return DSFileStream::makeFromPath(getPath(), false);
}

Common::WriteStream *DSFileSystemNode::createWriteStream() {
	Common::WriteStream *stream = DSFileStream::makeFromPath(getPath(), true);
	return Common::wrapBufferedWriteStream(stream, WRITE_BUFFER_SIZE);
}

//////////////////////////////////////////////////////////////////////////
// GBAMPFileSystemNode - File system using GBA Movie Player and CF card //
//////////////////////////////////////////////////////////////////////////

GBAMPFileSystemNode::GBAMPFileSystemNode() {
	_displayName = "mp:/";
	_path = "mp:/";
	_isValid = false;
	_isDirectory = true;
}

GBAMPFileSystemNode::GBAMPFileSystemNode(const Common::String& path) {
//	consolePrintf("'%s'",path.c_str());

	int lastSlash = 3;
	for (int r = 0; r < (int) path.size() - 1; r++) {
		if ((path[r] == '\\') || (path[r] == '/')) {
			lastSlash = r;
		}
	}

	if (path == "mp:/") {
		// This is the root directory
		_isDirectory = true;
		_isValid = false;		// Old code returned false here, but I'm not sure why
	} else {
		char check[128];
		memset(check, 0, 128);

		if (path.size() > 4 && path.hasPrefix("mp:/")) {
			// Files which start with mp:/
			// Clear the filename to 128 bytes, because a libfat bug occasionally tries to read in this area.
			strcpy(check, path.c_str() + 3);
		} else {
			// Clear the filename to 128 bytes, because a libfat bug occationally tries to read in this area.
			strcpy(check, path.c_str());
		}

		// Remove terminating slash - FileExists fails without this
		if (check[strlen(check) - 1] == '/') {
			check[strlen(check) - 1] = 0;
		}
		int fileOrDir = FAT_FileExists(check);

		_isDirectory = fileOrDir == FT_DIR;
		_isValid = fileOrDir == FT_FILE;
	}


//	consolePrintf("Path: %s \n", check);

	_displayName = Common::String(path.c_str() + lastSlash + 1);
	_path = path;
}

GBAMPFileSystemNode::GBAMPFileSystemNode(const Common::String& path, bool isDir) {
	//consolePrintf("'%s'",path.c_str());

	int lastSlash = 3;
	for (int r = 0; r < (int) path.size() - 1; r++) {
		if ((path[r] == '\\') || (path[r] == '/')) {
			lastSlash = r;
		}
	}

	_displayName = Common::String(path.c_str() + lastSlash + 1);
	_path = path;
	_isValid = true;
	_isDirectory = isDir;
}


GBAMPFileSystemNode::GBAMPFileSystemNode(const GBAMPFileSystemNode *node) {
	//TODO: not implemented?
}

AbstractFSNode *GBAMPFileSystemNode::getChild(const Common::String& n) const {
	if (_path.lastChar() == '\\') {
		return new GBAMPFileSystemNode(_path + n);
	} else {
		return new GBAMPFileSystemNode(_path + "\\" + n);
	}

	return NULL;
}

bool GBAMPFileSystemNode::getChildren(AbstractFSList& dirList, ListMode mode, bool hidden) const {
//	consolePrintf("Listdir\n");

	//TODO: honor the hidden flag

	enum { TYPE_NO_MORE = 0, TYPE_FILE = 1, TYPE_DIR = 2 };

	char temp[128], fname[256], *path, *pathTemp;
	strcpy(temp, _path.c_str());

	path = temp + 3;

	pathTemp = path;
	while (*pathTemp) {
		if (*pathTemp == '\\') {
			*pathTemp = '/';
		}
		pathTemp++;
	}

	// consolePrintf("This dir: %s\n", path);
	FAT_chdir(path);

	int entryType = FAT_FindFirstFileLFN(fname);

	while (entryType != TYPE_NO_MORE) {

		if ( ((entryType == TYPE_DIR) && ((mode == Common::FSNode::kListDirectoriesOnly) || (mode == Common::FSNode::kListAll)))
		||   ((entryType == TYPE_FILE) && ((mode == Common::FSNode::kListFilesOnly) || (mode == Common::FSNode::kListAll))) ) {
			GBAMPFileSystemNode *dsfsn;

			//consolePrintf("Fname: %s\n", fname);

			if (strcmp(fname, ".") && strcmp(fname, "..")) {

				if (!strcmp(path, "/")) {
					dsfsn = new GBAMPFileSystemNode("mp:" + Common::String(path) + Common::String(fname), entryType == TYPE_DIR);
				} else {
					dsfsn = new GBAMPFileSystemNode("mp:" + Common::String(path) + Common::String("/") + Common::String(fname), entryType == TYPE_DIR);
				}

//				dsfsn->_isDirectory = entryType == DIR;
				dirList.push_back((dsfsn));
			}
		} else {
//			consolePrintf("Skipping %s\n", fname);
		}

		entryType = FAT_FindNextFileLFN(fname);
	}

//	consolePrintf("No more");

	FAT_chdir("/");

	return true;
}

AbstractFSNode *GBAMPFileSystemNode::getParent() const {
//	consolePrintf("parent\n");
	GBAMPFileSystemNode *p;

	if (_path != "mp:/") {
		const char *path = (const char *)_path.c_str();
		int lastSlash = 4;

		for (uint r = 4; r < strlen(path); r++) {
			if (path[r] == '/') {
				lastSlash = r;
			}
		}

		p = new GBAMPFileSystemNode(Common::String(path, lastSlash));
		p->_isDirectory = true;
	} else {
		p = new GBAMPFileSystemNode();
	}

	return p;
}

Common::SeekableReadStream *GBAMPFileSystemNode::createReadStream() {
//	consolePrintf("Opening: %s\n", getPath().c_str());

	if (!strncmp(getPath().c_str(), "mp:/", 4)) {
		return DSFileStream::makeFromPath(getPath().c_str() + 3, false);
	} else {
		return DSFileStream::makeFromPath(getPath(), false);
	}
}

Common::WriteStream *GBAMPFileSystemNode::createWriteStream() {
	Common::WriteStream *stream = DSFileStream::makeFromPath(getPath(), true);
	return Common::wrapBufferedWriteStream(stream, WRITE_BUFFER_SIZE);
}



DSFileStream::DSFileStream(void *handle) : _handle(handle) {
	assert(handle);
}

DSFileStream::~DSFileStream() {
	std_fclose((FILE *)_handle);
}

bool DSFileStream::err() const {
	return std_ferror((FILE *)_handle) != 0;
}

void DSFileStream::clearErr() {
	std_clearerr((FILE *)_handle);
}

bool DSFileStream::eos() const {
	return std_feof((FILE *)_handle) != 0;
}

int32 DSFileStream::pos() const {
	return std_ftell((FILE *)_handle);
}

int32 DSFileStream::size() const {
	int32 oldPos = std_ftell((FILE *)_handle);
	std_fseek((FILE *)_handle, 0, SEEK_END);
	int32 length = std_ftell((FILE *)_handle);
	std_fseek((FILE *)_handle, oldPos, SEEK_SET);

	return length;
}

bool DSFileStream::seek(int32 offs, int whence) {
	return std_fseek((FILE *)_handle, offs, whence) == 0;
}

uint32 DSFileStream::read(void *ptr, uint32 len) {
	return std_fread(ptr, 1, len, (FILE *)_handle);
}

uint32 DSFileStream::write(const void *ptr, uint32 len) {
	return std_fwrite(ptr, 1, len, (FILE *)_handle);
}

bool DSFileStream::flush() {
	return std_fflush((FILE *)_handle) == 0;
}

DSFileStream *DSFileStream::makeFromPath(const Common::String &path, bool writeMode) {
	FILE *handle = std_fopen(path.c_str(), writeMode ? "wb" : "rb");

	if (handle)
		return new DSFileStream(handle);
	return 0;
}


// Stdio replacements
enum {
	MAX_FILE_HANDLES = 32
};

static bool inited = false;	// FIXME: Avoid non-const global vars
static DS::fileHandle s_handle[MAX_FILE_HANDLES];	// FIXME: Avoid non-const global vars

FILE *std_fopen(const char *name, const char *mode) {
	if (!inited) {
		for (int r = 0; r < MAX_FILE_HANDLES; r++) {
			s_handle[r].used = false;
		}
		inited = true;
		currentDir[0] = '\0';
	}

	char realName[MAXPATHLEN];

	// Remove file system prefix
	if ((name[0] == 'd') && (name[1] == 's') && (name[2] == ':') && (name[3] == '/')) {
		strlcpy(realName, name + 4, MAXPATHLEN);
	} else if ((name[0] == 'm') && (name[1] == 'p') && (name[2] == ':') && (name[3] == '/')) {
		strlcpy(realName, name + 4, MAXPATHLEN);
	} else {
		strlcpy(realName, name, MAXPATHLEN);
	}

//	consolePrintf("Open file:");
//	consolePrintf("'%s', [%s]", name, realName);

	if (DS::isGBAMPAvailable()) {
		FAT_chdir("/");

		// Turn all back slashes into forward slashes for gba_nds_fat
		char *p = realName;
		while (*p) {
			if (*p == '\\')
				*p = '/';
			p++;
		}

		FAT_FILE *result = FAT_fopen(realName, mode);

		if (result == 0) {
//			consolePrintf("Error code %d\n", result);
			//consolePrintf("Opening file %s\n", realName);
		} else {
//			consolePrintf("Opened file %d\n", result);
		}
//		MT_memoryReport();

		return (FILE *)result;
	}

	// Fail to open file for writing.  It's in ROM!

	// Allocate a file handle
	int r = 0;
	while (s_handle[r].used) {
		r++;
		assert(r < MAX_FILE_HANDLES);
	}

	char *data;

	ZipFile *zip = DSFileSystemNode::getZip();
	if (!zip) {
//		consolePrintf("No zip yet!");
		return NULL;
	}

	// Grab the data if it exists

	zip->setAllFilesVisible(true);

	if (currentDir[0] != 0) {
		char nameWithPath[128];
		sprintf(nameWithPath, "%s\\%s", currentDir, realName);
		strcpy(realName, nameWithPath);
	}

//	consolePrintf("fopen(%s, %s)\n", realName, name);

	if (zip->findFile(realName)) {
		data = zip->getFile();
		zip->setAllFilesVisible(false);

		// Allocate a file handle
		r = 0;
		while (s_handle[r].used)
			r++;


		s_handle[r].used = true;
		s_handle[r].pos = 0;
		s_handle[r].data = data;
		s_handle[r].size = zip->getFileSize();

//		consolePrintf("Opened file %d: %s (%s)   ", r, realName, name);
		return &s_handle[r];
	} else {
		zip->setAllFilesVisible(false);
//		consolePrintf("Not found: %s (%s)  ", realName, name);
		return NULL;
	}
}

void std_fclose(FILE *handle) {

	if (DS::isGBAMPAvailable()) {
		FAT_fclose((FAT_FILE *) handle);
		return;
	}

	handle->used = false;
}

size_t std_fread(void *ptr, size_t size, size_t numItems, FILE *handle) {
//	consolePrintf("fread %d,%d %d ", size, numItems, ptr);

	if (DS::isGBAMPAvailable()) {
		readPastEndOfFile = false;

		int bytes = FAT_fread(ptr, size, numItems, (FAT_FILE *) handle);
		if (!FAT_feof((FAT_FILE *) handle)) {
			return numItems;
		} else {
//			consolePrintf("Read past end of file: %d read out of %d\n", bytes / size, numItems);
			if ((size_t)bytes != size * numItems) readPastEndOfFile = true;
			return bytes / size;
		}
		return numItems;
	}

	if (handle->pos > handle->size)
		numItems = 0;
	else if ((int)(handle->pos + size * numItems) > handle->size)
		numItems = (handle->size - handle->pos) / size;

//	consolePrintf("read %d  ", size * numItems);

	memcpy(ptr, handle->data + handle->pos, size * numItems);
	handle->pos += size * numItems;

	return numItems;
}

size_t std_fwrite(const void *ptr, size_t size, size_t numItems, FILE *handle) {
	//consolePrintf("fwrite size=%d\n", size * numItems);

	if (DS::isGBAMPAvailable()) {
		FAT_fwrite(ptr, size, numItems, (FAT_FILE *) handle);
		return numItems;
	}

	return 0;
}

bool std_feof(FILE *handle) {
//	consolePrintf("feof ");

	if (DS::isGBAMPAvailable()) {
		return readPastEndOfFile && FAT_feof((FAT_FILE *) handle);
	}

//	consolePrintf("feof %s", handle->pos >= handle->size? "true": "false");
	return handle->pos >= handle->size;
}

int std_fflush(FILE *handle) {
	//FIXME: not implemented?
//	consolePrintf("fflush ");
	return 0;
}

long int std_ftell(FILE *handle) {
	if (DS::isGBAMPAvailable()) {
		return FAT_ftell((FAT_FILE *) handle);
	}

	return handle->pos;
}

int std_fseek(FILE *handle, long int offset, int whence) {
//	consolePrintf("fseek %d %d ", offset, whence);

	if (DS::isGBAMPAvailable()) {
		return FAT_fseek((FAT_FILE *) handle, offset, whence);
	}

	switch (whence) {
	case SEEK_CUR:
		handle->pos += offset;
		break;
	case SEEK_SET:
		handle->pos = offset;
		break;
	case SEEK_END:
		handle->pos = handle->size + offset;
		break;
	default:
		handle->pos = offset;
		break;
	}

	return 0;
}

int std_ferror(FILE *handle) {
	//FIXME: not implemented?
//	consolePrintf("ferror ");

	return readPastEndOfFile;
}

void std_clearerr(FILE *handle) {
	//FIXME: not implemented?
	readPastEndOfFile = false;
//	consolePrintf("clearerr ");
}

} // namespace DS
