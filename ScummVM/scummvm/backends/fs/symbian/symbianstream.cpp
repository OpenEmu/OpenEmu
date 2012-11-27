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
#include "backends/fs/symbian/symbianstream.h"
#include "common/system.h"
#include "backends/platform/symbian/src/symbianos.h"

#include <f32file.h>

#define KInputBufferLength 128

// Symbian libc file functionality in order to provide shared file handles
class TSymbianFileEntry {
public:
	RFile _fileHandle;
	char _inputBuffer[KInputBufferLength];
	TInt _inputBufferLen;
	TInt _inputPos;
	TInt _lastError;
	TBool _eofReached;
};

TSymbianFileEntry*	CreateSymbianFileEntry(const char* name, const char* mode) {
	TSymbianFileEntry* fileEntry = new TSymbianFileEntry;
	fileEntry->_inputPos = KErrNotFound;
	fileEntry->_lastError = 0;
	fileEntry->_eofReached = EFalse;

	if (fileEntry != NULL) {
		TInt modeLen = strlen(mode);

		TPtrC8 namePtr((unsigned char*) name, strlen(name));
		TFileName tempFileName;
		tempFileName.Copy(namePtr);

		TInt fileMode = EFileRead;

		if (mode[0] == 'a')
			fileMode = EFileWrite;

		if (!((modeLen > 1 && mode[1] == 'b') || (modeLen > 2 && mode[2] == 'b'))) {
			fileMode |= EFileStreamText;
		}

		if ((modeLen > 1 && mode[1] == '+') || (modeLen > 2 && mode[2] == '+')) {
			fileMode = fileMode| EFileWrite;
		}

		fileMode = fileMode| EFileShareAny;

		switch (mode[0]) {
		case 'a':
			if (fileEntry->_fileHandle.Open(static_cast<OSystem_SDL_Symbian *>(g_system)->FsSession(), tempFileName, fileMode) != KErrNone) {
				if (fileEntry->_fileHandle.Create(static_cast<OSystem_SDL_Symbian *>(g_system)->FsSession(), tempFileName, fileMode) != KErrNone) {
					delete fileEntry;
					fileEntry = NULL;
				}
			}
			break;
		case 'r':
			if (fileEntry->_fileHandle.Open(static_cast<OSystem_SDL_Symbian *>(g_system)->FsSession(), tempFileName, fileMode) != KErrNone) {
				delete fileEntry;
				fileEntry = NULL;
			}
			break;

		case 'w':
			if (fileEntry->_fileHandle.Replace(static_cast<OSystem_SDL_Symbian *>(g_system)->FsSession(), tempFileName, fileMode) != KErrNone) {
				delete fileEntry;
				fileEntry = NULL;
			}
			break;
		}
	}
	return fileEntry;
}

size_t ReadData(const void* ptr, size_t size, size_t numItems, TSymbianFileEntry* handle) {
	TSymbianFileEntry* entry = ((TSymbianFileEntry *)(handle));
	TUint32 totsize = size*numItems;
	TPtr8 pointer ( (unsigned char*) ptr, totsize);

	// Nothing cached and we want to load at least KInputBufferLength bytes
	if (totsize >= KInputBufferLength) {
		TUint32 totLength = 0;
		if (entry->_inputPos != KErrNotFound) {
			TPtr8 cacheBuffer( (unsigned char*) entry->_inputBuffer+entry->_inputPos, entry->_inputBufferLen - entry->_inputPos, KInputBufferLength);
			pointer.Append(cacheBuffer);
			entry->_inputPos = KErrNotFound;
			totLength+=pointer.Length();
			pointer.Set(totLength+(unsigned char*) ptr, 0, totsize-totLength);
		}

		entry->_lastError = entry->_fileHandle.Read(pointer);

		totLength+=pointer.Length();

		pointer.Set((unsigned char*) ptr, totLength, totsize);

	} else {
		// Nothing in buffer
		if (entry->_inputPos == KErrNotFound) {
			TPtr8 cacheBuffer( (unsigned char*) entry->_inputBuffer, KInputBufferLength);
			entry->_lastError = entry->_fileHandle.Read(cacheBuffer);

			if (cacheBuffer.Length() >= totsize) {
				pointer.Copy(cacheBuffer.Left(totsize));
				entry->_inputPos = totsize;
				entry->_inputBufferLen = cacheBuffer.Length();
			} else {
				pointer.Copy(cacheBuffer);
				entry->_inputPos = KErrNotFound;
			}

		} else {
			TPtr8 cacheBuffer( (unsigned char*) entry->_inputBuffer, entry->_inputBufferLen, KInputBufferLength);

			if (entry->_inputPos+totsize < entry->_inputBufferLen) {
				pointer.Copy(cacheBuffer.Mid(entry->_inputPos, totsize));
				entry->_inputPos+=totsize;
			} else {

				pointer.Copy(cacheBuffer.Mid(entry->_inputPos, entry->_inputBufferLen-entry->_inputPos));
				cacheBuffer.SetLength(0);
				entry->_lastError = entry->_fileHandle.Read(cacheBuffer);

				if (cacheBuffer.Length() >= totsize-pointer.Length()) {
					TUint32 restSize = totsize-pointer.Length();
					pointer.Append(cacheBuffer.Left(restSize));
					entry->_inputPos = restSize;
					entry->_inputBufferLen = cacheBuffer.Length();
				} else {
					pointer.Append(cacheBuffer);
					entry->_inputPos = KErrNotFound;
				}
			}
		}
	}

	if ((numItems * size) != pointer.Length() && entry->_lastError == KErrNone) {
		entry->_eofReached = ETrue;
	}

	return pointer.Length() / size;
}

SymbianStdioStream::SymbianStdioStream(void *handle) : _handle(handle) {
	assert(handle);
}

SymbianStdioStream::~SymbianStdioStream() {
	((TSymbianFileEntry *)(_handle))->_fileHandle.Close();

	delete (TSymbianFileEntry *)(_handle);
}

bool SymbianStdioStream::err() const {
	return ((TSymbianFileEntry *)(_handle))->_lastError != 0;
}

void SymbianStdioStream::clearErr() {
	((TSymbianFileEntry *)(_handle))->_lastError = 0;
	((TSymbianFileEntry *)(_handle))->_eofReached = 0;
}

bool SymbianStdioStream::eos() const {
	TSymbianFileEntry* entry = ((TSymbianFileEntry *)(_handle));

	return entry->_eofReached != 0;
}

int32 SymbianStdioStream::pos() const {
	TInt pos = 0;
	TSymbianFileEntry* entry = ((TSymbianFileEntry *)(_handle));

	entry->_lastError = entry->_fileHandle.Seek(ESeekCurrent, pos);
	if (entry->_lastError == KErrNone && entry->_inputPos != KErrNotFound) {
		pos += (entry->_inputPos - entry->_inputBufferLen);
	}

	return pos;
}

int32 SymbianStdioStream::size() const {

	TInt length = 0;
	((TSymbianFileEntry *)(_handle))->_fileHandle.Size(length);

	return length;
}

bool SymbianStdioStream::seek(int32 offs, int whence) {
	assert(_handle);

	TSeek seekMode = ESeekStart;
	TInt pos = offs;
	TSymbianFileEntry* entry = ((TSymbianFileEntry *)(_handle));

	switch (whence) {
	case SEEK_SET:
		seekMode = ESeekStart;
		break;
	case SEEK_CUR:
		seekMode = ESeekCurrent;
		if (entry->_inputPos != KErrNotFound) {
			pos += (entry->_inputPos - entry->_inputBufferLen);
		}
		break;
	case SEEK_END:
		seekMode = ESeekEnd;
		break;

	}

	entry->_inputPos = KErrNotFound;
	entry->_eofReached = EFalse;
	entry->_fileHandle.Seek(seekMode, pos);

	return true;	// FIXME: Probably should return a value based on what _fileHandle.Seek returns
}

uint32 SymbianStdioStream::read(void *ptr, uint32 len) {
	return (uint32)ReadData((byte *)ptr, 1, len, (TSymbianFileEntry *)_handle);
}

uint32 SymbianStdioStream::write(const void *ptr, uint32 len) {
	TPtrC8 pointer( (unsigned char*) ptr, len);

	((TSymbianFileEntry *)(_handle))->_inputPos = KErrNotFound;
	((TSymbianFileEntry *)(_handle))->_lastError = ((TSymbianFileEntry *)(_handle))->_fileHandle.Write(pointer);
	((TSymbianFileEntry *)(_handle))->_eofReached = EFalse;

	if (((TSymbianFileEntry *)(_handle))->_lastError == KErrNone) {
		return len;
	}

	return 0;
}

bool SymbianStdioStream::flush() {
	((TSymbianFileEntry *)(_handle))->_fileHandle.Flush();
	return true;
}

SymbianStdioStream *SymbianStdioStream::makeFromPath(const Common::String &path, bool writeMode) {
	void *handle = CreateSymbianFileEntry(path.c_str(), writeMode ? "wb" : "rb");
	if (handle)
		return new SymbianStdioStream(handle);
	return 0;
}
