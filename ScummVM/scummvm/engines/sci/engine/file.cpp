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

#include "common/savefile.h"
#include "common/stream.h"

#include "sci/sci.h"
#include "sci/engine/file.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"

namespace Sci {

/*
 * Note on how file I/O is implemented: In ScummVM, one can not create/write
 * arbitrary data files, simply because many of our target platforms do not
 * support this. The only files one can create are savestates. But SCI has an
 * opcode to create and write to seemingly 'arbitrary' files. This is mainly
 * used in LSL3 for LARRY3.DRV (which is a game data file, not a driver, used
 * for persisting the results of the "age quiz" across restarts) and in LSL5
 * for MEMORY.DRV (which is again a game data file and contains the game's
 * password, XOR encrypted).
 * To implement that opcode, we combine the SaveFileManager with regular file
 * code, similarly to how the SCUMM HE engine does it.
 *
 * To handle opening a file called "foobar", what we do is this: First, we
 * create an 'augmented file name', by prepending the game target and a dash,
 * so if we running game target sq1sci, the name becomes "sq1sci-foobar".
 * Next, we check if such a file is known to the SaveFileManager. If so, we
 * we use that for reading/writing, delete it, whatever.
 *
 * If no such file is present but we were only asked to *read* the file,
 * we fallback to looking for a regular file called "foobar", and open that
 * for reading only.
 */

reg_t file_open(EngineState *s, const Common::String &filename, int mode, bool unwrapFilename) {
	Common::String englishName = g_sci->getSciLanguageString(filename, K_LANG_ENGLISH);
	englishName.toLowercase();

	Common::String wrappedName = unwrapFilename ? g_sci->wrapFilename(englishName) : englishName;
	Common::SeekableReadStream *inFile = 0;
	Common::WriteStream *outFile = 0;
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();

	bool isCompressed = true;
	const SciGameId gameId = g_sci->getGameId();
	if ((gameId == GID_QFG1 || gameId == GID_QFG1VGA || gameId == GID_QFG2 || gameId == GID_QFG3)
		&& englishName.hasSuffix(".sav")) {
		// QFG Characters are saved via the CharSave object.
		// We leave them uncompressed so that they can be imported in later QFG
		// games.
		// Rooms/Scripts: QFG1: 601, QFG2: 840, QFG3/4: 52
		isCompressed = false;
	}

	if (mode == _K_FILE_MODE_OPEN_OR_FAIL) {
		// Try to open file, abort if not possible
		inFile = saveFileMan->openForLoading(wrappedName);
		// If no matching savestate exists: fall back to reading from a regular
		// file
		if (!inFile)
			inFile = SearchMan.createReadStreamForMember(englishName);

		if (!inFile)
			debugC(kDebugLevelFile, "  -> file_open(_K_FILE_MODE_OPEN_OR_FAIL): failed to open file '%s'", englishName.c_str());
	} else if (mode == _K_FILE_MODE_CREATE) {
		// Create the file, destroying any content it might have had
		outFile = saveFileMan->openForSaving(wrappedName, isCompressed);
		if (!outFile)
			debugC(kDebugLevelFile, "  -> file_open(_K_FILE_MODE_CREATE): failed to create file '%s'", englishName.c_str());
	} else if (mode == _K_FILE_MODE_OPEN_OR_CREATE) {
		// Try to open file, create it if it doesn't exist
		outFile = saveFileMan->openForSaving(wrappedName, isCompressed);
		if (!outFile)
			debugC(kDebugLevelFile, "  -> file_open(_K_FILE_MODE_CREATE): failed to create file '%s'", englishName.c_str());

		// QfG1 opens the character export file with _K_FILE_MODE_CREATE first,
		// closes it immediately and opens it again with this here. Perhaps
		// other games use this for read access as well. I guess changing this
		// whole code into using virtual files and writing them after close
		// would be more appropriate.
	} else {
		error("file_open: unsupported mode %d (filename '%s')", mode, englishName.c_str());
	}

	if (!inFile && !outFile) { // Failed
		debugC(kDebugLevelFile, "  -> file_open() failed");
		return SIGNAL_REG;
	}

	// Find a free file handle
	uint handle = 1; // Ignore _fileHandles[0]
	while ((handle < s->_fileHandles.size()) && s->_fileHandles[handle].isOpen())
		handle++;

	if (handle == s->_fileHandles.size()) {
		// Hit size limit => Allocate more space
		s->_fileHandles.resize(s->_fileHandles.size() + 1);
	}

	s->_fileHandles[handle]._in = inFile;
	s->_fileHandles[handle]._out = outFile;
	s->_fileHandles[handle]._name = englishName;

	debugC(kDebugLevelFile, "  -> opened file '%s' with handle %d", englishName.c_str(), handle);
	return make_reg(0, handle);
}

FileHandle *getFileFromHandle(EngineState *s, uint handle) {
	if (handle == 0 || handle == VIRTUALFILE_HANDLE) {
		error("Attempt to use invalid file handle (%d)", handle);
		return 0;
	}

	if ((handle >= s->_fileHandles.size()) || !s->_fileHandles[handle].isOpen()) {
		warning("Attempt to use invalid/unused file handle %d", handle);
		return 0;
	}

	return &s->_fileHandles[handle];
}

int fgets_wrapper(EngineState *s, char *dest, int maxsize, int handle) {
	FileHandle *f = getFileFromHandle(s, handle);
	if (!f)
		return 0;

	if (!f->_in) {
		error("fgets_wrapper: Trying to read from file '%s' opened for writing", f->_name.c_str());
		return 0;
	}
	int readBytes = 0;
	if (maxsize > 1) {
		memset(dest, 0, maxsize);
		f->_in->readLine(dest, maxsize);
		readBytes = strlen(dest); // FIXME: sierra sci returned byte count and didn't react on NUL characters
		// The returned string must not have an ending LF
		if (readBytes > 0) {
			if (dest[readBytes - 1] == 0x0A)
				dest[readBytes - 1] = 0;
		}
	} else {
		*dest = 0;
	}

	debugC(kDebugLevelFile, "  -> FGets'ed \"%s\"", dest);
	return readBytes;
}

static bool _savegame_sort_byDate(const SavegameDesc &l, const SavegameDesc &r) {
	if (l.date != r.date)
		return (l.date > r.date);
	return (l.time > r.time);
}

// Create a sorted array containing all found savedgames
void listSavegames(Common::Array<SavegameDesc> &saves) {
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();

	// Load all saves
	Common::StringArray saveNames = saveFileMan->listSavefiles(g_sci->getSavegamePattern());

	for (Common::StringArray::const_iterator iter = saveNames.begin(); iter != saveNames.end(); ++iter) {
		Common::String filename = *iter;
		Common::SeekableReadStream *in;
		if ((in = saveFileMan->openForLoading(filename))) {
			SavegameMetadata meta;
			if (!get_savegame_metadata(in, &meta) || meta.name.empty()) {
				// invalid
				delete in;
				continue;
			}
			delete in;

			SavegameDesc desc;
			desc.id = strtol(filename.end() - 3, NULL, 10);
			desc.date = meta.saveDate;
			// We need to fix date in here, because we save DDMMYYYY instead of
			// YYYYMMDD, so sorting wouldn't work
			desc.date = ((desc.date & 0xFFFF) << 16) | ((desc.date & 0xFF0000) >> 8) | ((desc.date & 0xFF000000) >> 24);
			desc.time = meta.saveTime;
			desc.version = meta.version;

			if (meta.name.lastChar() == '\n')
				meta.name.deleteLastChar();

			Common::strlcpy(desc.name, meta.name.c_str(), SCI_MAX_SAVENAME_LENGTH);

			debug(3, "Savegame in file %s ok, id %d", filename.c_str(), desc.id);

			saves.push_back(desc);
		}
	}

	// Sort the list by creation date of the saves
	Common::sort(saves.begin(), saves.end(), _savegame_sort_byDate);
}

// Find a savedgame according to virtualId and return the position within our array
int findSavegame(Common::Array<SavegameDesc> &saves, int16 savegameId) {
	for (uint saveNr = 0; saveNr < saves.size(); saveNr++) {
		if (saves[saveNr].id == savegameId)
			return saveNr;
	}
	return -1;
}


FileHandle::FileHandle() : _in(0), _out(0) {
}

FileHandle::~FileHandle() {
	close();
}

void FileHandle::close() {
	delete _in;
	delete _out;
	_in = 0;
	_out = 0;
	_name.clear();
}

bool FileHandle::isOpen() const {
	return _in || _out;
}


void DirSeeker::addAsVirtualFiles(Common::String title, Common::String fileMask) {
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	Common::StringArray foundFiles = saveFileMan->listSavefiles(fileMask);
	if (!foundFiles.empty()) {
		_files.push_back(title);
		_virtualFiles.push_back("");
		Common::StringArray::iterator it;
		Common::StringArray::iterator it_end = foundFiles.end();

		for (it = foundFiles.begin(); it != it_end; it++) {
			Common::String regularFilename = *it;
			Common::String wrappedFilename = Common::String(regularFilename.c_str() + fileMask.size() - 1);

			Common::SeekableReadStream *testfile = saveFileMan->openForLoading(regularFilename);
			int32 testfileSize = testfile->size();
			delete testfile;
			if (testfileSize > 1024) // check, if larger than 1k. in that case its a saved game.
				continue; // and we dont want to have those in the list
			// We need to remove the prefix for display purposes
			_files.push_back(wrappedFilename);
			// but remember the actual name as well
			_virtualFiles.push_back(regularFilename);
		}
	}
}

Common::String DirSeeker::getVirtualFilename(uint fileNumber) {
	if (fileNumber >= _virtualFiles.size())
		error("invalid virtual filename access");
	return _virtualFiles[fileNumber];
}

reg_t DirSeeker::firstFile(const Common::String &mask, reg_t buffer, SegManager *segMan) {
	// Verify that we are given a valid buffer
	if (!buffer.getSegment()) {
		error("DirSeeker::firstFile('%s') invoked with invalid buffer", mask.c_str());
		return NULL_REG;
	}
	_outbuffer = buffer;
	_files.clear();
	_virtualFiles.clear();

	int QfGImport = g_sci->inQfGImportRoom();
	if (QfGImport) {
		_files.clear();
		addAsVirtualFiles("-QfG1-", "qfg1-*");
		addAsVirtualFiles("-QfG1VGA-", "qfg1vga-*");
		if (QfGImport > 2)
			addAsVirtualFiles("-QfG2-", "qfg2-*");
		if (QfGImport > 3)
			addAsVirtualFiles("-QfG3-", "qfg3-*");

		if (QfGImport == 3) {
			// QfG3 sorts the filelisting itself, we can't let that happen otherwise our
			//  virtual list would go out-of-sync
			reg_t savedHeros = segMan->findObjectByName("savedHeros");
			if (!savedHeros.isNull())
				writeSelectorValue(segMan, savedHeros, SELECTOR(sort), 0);
		}

	} else {
		// Prefix the mask
		const Common::String wrappedMask = g_sci->wrapFilename(mask);

		// Obtain a list of all files matching the given mask
		Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
		_files = saveFileMan->listSavefiles(wrappedMask);
	}

	// Reset the list iterator and write the first match to the output buffer,
	// if any.
	_iter = _files.begin();
	return nextFile(segMan);
}

reg_t DirSeeker::nextFile(SegManager *segMan) {
	if (_iter == _files.end()) {
		return NULL_REG;
	}

	Common::String string;

	if (_virtualFiles.empty()) {
		// Strip the prefix, if we don't got a virtual filelisting
		const Common::String wrappedString = *_iter;
		string = g_sci->unwrapFilename(wrappedString);
	} else {
		string = *_iter;
	}
	if (string.size() > 12)
		string = Common::String(string.c_str(), 12);
	segMan->strcpy(_outbuffer, string.c_str());

	// Return the result and advance the list iterator :)
	++_iter;
	return _outbuffer;
}


#ifdef ENABLE_SCI32

VirtualIndexFile::VirtualIndexFile(Common::String fileName) : _fileName(fileName), _changed(false) {
	Common::SeekableReadStream *inFile = g_sci->getSaveFileManager()->openForLoading(fileName);

	_bufferSize = inFile->size();
	_buffer = new char[_bufferSize];
	inFile->read(_buffer, _bufferSize);
	_ptr = _buffer;
	delete inFile;
}

VirtualIndexFile::VirtualIndexFile(uint32 initialSize) : _changed(false) {
	_bufferSize = initialSize;
	_buffer = new char[_bufferSize];
	_ptr = _buffer;
}

VirtualIndexFile::~VirtualIndexFile() {
	close();

	_bufferSize = 0;
	delete[] _buffer;
	_buffer = 0;
}

uint32 VirtualIndexFile::read(char *buffer, uint32 size) {
	uint32 curPos = _ptr - _buffer;
	uint32 finalSize = MIN<uint32>(size, _bufferSize - curPos);
	char *localPtr = buffer;

	for (uint32 i = 0; i < finalSize; i++)
		*localPtr++ = *_ptr++;

	return finalSize;
}

uint32 VirtualIndexFile::write(const char *buffer, uint32 size) {
	_changed = true;
	uint32 curPos = _ptr - _buffer;

	// Check if the buffer needs to be resized
	if (curPos + size >= _bufferSize) {
		_bufferSize = curPos + size + 1;
		char *tmp = _buffer;
		_buffer = new char[_bufferSize];
		_ptr = _buffer + curPos;
		memcpy(_buffer, tmp, _bufferSize);
		delete[] tmp;
	}

	for (uint32 i = 0; i < size; i++)
		*_ptr++ = *buffer++;

	return size;
}

uint32 VirtualIndexFile::readLine(char *buffer, uint32 size) {
	uint32 startPos = _ptr - _buffer;
	uint32 bytesRead = 0;
	char *localPtr = buffer;

	// This is not a full-blown implementation of readLine, but it
	// suffices for Phantasmagoria
	while (startPos + bytesRead < size) {
		bytesRead++;

		if (*_ptr == 0 || *_ptr == 0x0A) {
			_ptr++;
			*localPtr = 0;
			return bytesRead;
		} else {
			*localPtr++ = *_ptr++;
		}
	}

	return bytesRead;
}

bool VirtualIndexFile::seek(int32 offset, int whence) {
	uint32 startPos = _ptr - _buffer;
	assert(offset >= 0);

	switch (whence) {
	case SEEK_CUR:
		assert(startPos + offset < _bufferSize);
		_ptr += offset;
		break;
	case SEEK_SET:
		assert(offset < (int32)_bufferSize);
		_ptr = _buffer + offset;
		break;
	case SEEK_END:
		assert((int32)_bufferSize - offset >= 0);
		_ptr = _buffer + (_bufferSize - offset);
		break;
	}

	return true;
}

void VirtualIndexFile::close() {
	if (_changed && !_fileName.empty()) {
		Common::WriteStream *outFile = g_sci->getSaveFileManager()->openForSaving(_fileName);
		outFile->write(_buffer, _bufferSize);
		delete outFile;
	}

	// Maintain the buffer, and seek to the beginning of it
	_ptr = _buffer;
}

#endif

} // End of namespace Sci
