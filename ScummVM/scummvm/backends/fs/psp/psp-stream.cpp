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
#ifdef __PSP__

#include <pspiofilemgr.h>

#include "backends/platform/psp/powerman.h"
#include "backends/fs/psp/psp-stream.h"

#define MIN2(a,b) ((a < b) ? a : b)
#define MIN3(a,b,c) ( (a < b) ? (a < c ? a : c) : (b < c ? b : c) )

//#define __PSP_PRINT_TO_FILE__ /* For debugging suspend stuff, we have no screen output */
//#define __PSP_DEBUG_FUNCS__ 	/* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */

#include "backends/platform/psp/trace.h"

//#define DEBUG_BUFFERS					/* to see the contents of the buffers being read */

#ifdef DEBUG_BUFFERS
void printBuffer(byte *ptr, uint32 len) {
	uint32 printLen = len <= 10 ? len : 10;

	for (int i = 0; i < printLen; i++) {
		PSP_INFO_PRINT("%x ", ptr[i]);
	}

	if (len > 10) {
		PSP_INFO_PRINT("... ");
		for (int i = len - 10; i < len; i++)
			PSP_INFO_PRINT("%x ", ptr[i]);
	}

	PSP_INFO_PRINT("\n");
}
#endif

// Class PspIoStream ------------------------------------------------

PspIoStream::PspIoStream(const Common::String &path, bool writeMode)
		: _handle(0), _path(path), _fileSize(0), _writeMode(writeMode),
		  _physicalPos(0), _pos(0), _eos(false),	_error(false),
		  _errorSuspend(0), _errorSource(0), _errorPos(0), _errorHandle(0), _suspendCount(0) {
	DEBUG_ENTER_FUNC();

	//assert(!path.empty());	// do we need this?
}

PspIoStream::~PspIoStream() {
	DEBUG_ENTER_FUNC();

	if (PowerMan.beginCriticalSection())
		PSP_DEBUG_PRINT_FUNC("suspended\n");

	PowerMan.unregisterForSuspend(this); 			// Unregister with powermanager to be suspended
													// Must do this before fclose() or resume() will reopen.
	sceIoClose(_handle);

	PowerMan.endCriticalSection();
}

/* Function to open the file pointed to by the path.
 *
 */
void *PspIoStream::open() {
	DEBUG_ENTER_FUNC();

	if (PowerMan.beginCriticalSection()) {
		// No need to open? Just return the _handle resume() already opened
		PSP_DEBUG_PRINT_FUNC("suspended\n");
	}

	_handle = sceIoOpen(_path.c_str(), _writeMode ? PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC : PSP_O_RDONLY, 0777);
	if (!_handle) {
		_error = true;
		_handle = NULL;
	}

	// Get the file size. This way is much faster than going to the end of the file and back
	SceIoStat stat;
	sceIoGetstat(_path.c_str(), &stat);
	_fileSize = *((uint32 *)(void *)&stat.st_size);	// 4GB file (32 bits) is big enough for us

	PSP_DEBUG_PRINT("%s filesize[%d]\n", _path.c_str(), _fileSize);

	PowerMan.registerForSuspend(this);	 // Register with the powermanager to be suspended

	PowerMan.endCriticalSection();

	return (void *)_handle;
}

bool PspIoStream::err() const {
	DEBUG_ENTER_FUNC();

	if (_error)	// We dump since no printing to screen with suspend callback
		PSP_ERROR("mem_error[%d], source[%d], suspend error[%d], pos[%d],"
				  "_errorPos[%d], _errorHandle[%p], suspendCount[%d]\n",
		          _error, _errorSource, _errorSuspend, _pos,
				  _errorPos, _errorHandle, _suspendCount);

	return _error;
}

void PspIoStream::clearErr() {
	_error = false;
}

bool PspIoStream::eos() const {
	return _eos;
}

int32 PspIoStream::pos() const {
	return _pos;
}

int32 PspIoStream::size() const {
	return _fileSize;
}

bool PspIoStream::physicalSeekFromCur(int32 offset) {

	int ret = sceIoLseek32(_handle, offset, PSP_SEEK_CUR);

	if (ret < 0) {
		_error = true;
		PSP_ERROR("failed to seek in file[%s] to [%x]. Error[%x]\n", _path.c_str(), offset, ret);
		return false;
	}
	_physicalPos += offset;
	return true;
}

bool PspIoStream::seek(int32 offs, int whence) {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT_FUNC("offset[0x%x], whence[%d], _pos[0x%x], _physPos[0x%x]\n", offs, whence, _pos, _physicalPos);
	_eos = false;

	int32 posToSearchFor = 0;
	switch (whence) {
	case SEEK_CUR:
		posToSearchFor = _pos;
		break;
	case SEEK_END:
		posToSearchFor = _fileSize;
		break;
	}
	posToSearchFor += offs;

	// Check for bad values
	if (posToSearchFor < 0) {
		_error = true;
		return false;
	} else if (posToSearchFor > _fileSize) {
		_error = true;
		_eos = true;
		return false;
	}

	_pos = posToSearchFor;

	return true;
}

uint32 PspIoStream::read(void *ptr, uint32 len) {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT_FUNC("filename[%s], len[0x%x], ptr[%p], _pos[%x], _physPos[%x]\n", _path.c_str(), len, ptr, _pos, _physicalPos);

	if (_error || _eos || len <= 0)
		return 0;

	uint32 lenRemainingInFile = _fileSize - _pos;

	// check for getting EOS
	if (len > lenRemainingInFile) {
		len = lenRemainingInFile;
		_eos = true;
	}

	if (PowerMan.beginCriticalSection())
		PSP_DEBUG_PRINT_FUNC("suspended\n");

	// check if we need to seek
	if (_pos != _physicalPos)
		PSP_DEBUG_PRINT("seeking from %x to %x\n", _physicalPos, _pos);
		if (!physicalSeekFromCur(_pos - _physicalPos)) {
			_error = true;
			return 0;
		}

	int ret = sceIoRead(_handle, ptr, len);

	PowerMan.endCriticalSection();

	_physicalPos += ret;	// Update position
	_pos = _physicalPos;

	if (ret != (int)len) {	// error
		PSP_ERROR("sceIoRead returned [0x%x] instead of len[0x%x]\n", ret, len);
		_error = true;
		_errorSource = 4;
	}
	return ret;
}

uint32 PspIoStream::write(const void *ptr, uint32 len) {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT_FUNC("filename[%s], len[0x%x], ptr[%p], _pos[%x], _physPos[%x]\n", _path.c_str(), len, ptr, _pos, _physicalPos);

	if (!len || _error)		// we actually get some calls with len == 0!
		return 0;

	_eos = false;			// we can't have eos with write

	if (PowerMan.beginCriticalSection())
		PSP_DEBUG_PRINT_FUNC("suspended\n");

	// check if we need to seek
	if (_pos != _physicalPos)
		if (!physicalSeekFromCur(_pos - _physicalPos)) {
			_error = true;
			return 0;
		}

	int ret = sceIoWrite(_handle, ptr, len);

	PowerMan.endCriticalSection();

	if (ret != (int)len) {
		_error = true;
		_errorSource = 5;
		PSP_ERROR("sceIoWrite returned[0x%x] instead of len[0x%x]\n", ret, len);
	}

	_physicalPos += ret;
	_pos = _physicalPos;

	if (_pos > _fileSize)
		_fileSize = _pos;

	return ret;
}

bool PspIoStream::flush() {
	return true;
}

// For the PSP, since we're building in suspend support, we moved opening
// the actual file to an open function since we need an actual PspIoStream object to suspend.
//
PspIoStream *PspIoStream::makeFromPath(const Common::String &path, bool writeMode) {
	DEBUG_ENTER_FUNC();
	PspIoStream *stream = new PspIoStream(path, writeMode);

	if (stream->open() <= 0) {
		delete stream;
		stream = 0;
	}

	return stream;
}

/*
 *  Function to suspend the IO stream (called by PowerManager)
 *  we can have no output here
 */
int PspIoStream::suspend() {
	DEBUG_ENTER_FUNC();
	_suspendCount++;

	if (_handle > 0 && _pos < 0) {	/* check for error */
		_errorSuspend = SuspendError;
		_errorPos = _pos;
		_errorHandle = _handle;
	}

	if (_handle > 0) {
		sceIoClose(_handle);		// close our file descriptor
		_handle = 0xFFFFFFFF;		// Set handle to non-null invalid value so makeFromPath doesn't return error
	}

	return 0;
}

/*
 *  Function to resume the IO stream (called by Power Manager)
 */
int PspIoStream::resume() {
	DEBUG_ENTER_FUNC();
	int ret = 0;
	_suspendCount--;

	// We reopen our file descriptor
	_handle = sceIoOpen(_path.c_str(), _writeMode ? PSP_O_RDWR | PSP_O_CREAT : PSP_O_RDONLY, 0777); 	// open
	if (_handle <= 0) {
		_errorSuspend = ResumeError;
		_errorPos = _pos;
	}

	// Resume our previous position if needed
	if (_handle > 0 && _pos > 0) {
		ret = sceIoLseek32(_handle, _pos, PSP_SEEK_SET);

		_physicalPos = _pos;

		if (ret < 0) {		// Check for problem
			_errorSuspend = ResumeError;
			_errorPos = _pos;
			_errorHandle = _handle;
		}
	}
	return ret;
}

#endif /* __PSP__ */
