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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/ps2/fileio.h"

#include <tamtypes.h>
#include <kernel.h>
#include <fileio.h>
#include <assert.h>
#include <string.h>

#include "common/config-manager.h"
#include "common/file.h"
#include "engines/engine.h"
#include "backends/platform/ps2/asyncfio.h"
#include "backends/platform/ps2/eecodyvdfs.h"
#include "backends/platform/ps2/ps2debug.h"
#include "backends/platform/ps2/systemps2.h"

#define __PS2_FILE_SEMA__ 1

AsyncFio fio;

Ps2File::Ps2File() {
	_fd = -1;
	_fileSize = 0;
	_filePos = 0;
	_cacheSize = 0;
	_cachePos = 0;
	_eof = false;
	_err = false;

	_cacheBuf = (uint8 *)memalign(64, CACHE_SIZE * 2);

	_cacheOpRunning = 0;
	_filePos = _physFilePos = _cachePos = 0;
	_fileSize = _bytesInCache = _cacheOfs = 0;
	_cacheOpRunning = false;
	_readBytesBlock = 0;
	_stream = true;

#ifdef __PS2_FILE_SEMA__
	ee_sema_t newSema;
	newSema.init_count = 1;
	newSema.max_count = 1;
	_sema = CreateSema(&newSema);
	assert(_sema >= 0);
#endif
}

Ps2File::~Ps2File() {
	uint32 w;
	if (_fd >= 0) {

		if (_mode != O_RDONLY) {
			fio.seek(_fd, 0, SEEK_SET);
			fio.write(_fd, _cacheBuf, _filePos);
			w = fio.sync(_fd);
			printf("flushed wbuf: %x of %x\n", w, _filePos);
		}

		fio.close(_fd);
		uint32 r = fio.sync(_fd);
		printf("close [%d] - sync'd = %d\n", _fd, r);
	}

	free(_cacheBuf);

#ifdef __PS2_FILE_SEMA__
	DeleteSema(_sema);
#endif
}

bool Ps2File::open(const char *name, int mode) {
#if 1
	_fd = fio.open(name, mode);

	printf("open %s [%d]\n", name, _fd);

	if (_fd >= 0) {
		_mode = mode;
		_filePos = 0;

		if (_mode == O_RDONLY) {
			_fileSize = fio.seek(_fd, 0, SEEK_END);
			fio.seek(_fd, 0, SEEK_SET);
		}
		else
			_fileSize = 0;

		printf("  _mode = %x\n", _mode);
		printf("  _fileSize = %d\n", _fileSize);
		// printf("  _filePos = %d\n", _filePos);

		return true;
	}

	return false;
#else
	uint32 r;

	// hack: FIO does not reports size for RW (?)
	_fd = fio.open(name, O_RDONLY);
	if (_fd >= 0) {
		_fileSize = fio.seek(_fd, 0, SEEK_END);
		fio.seek(_fd, 0, SEEK_SET); /* rewind ! */

		if (_fileSize && mode != O_RDONLY) {
			fio.read(_fd, _cacheBuf, _fileSize);
			r = fio.sync(_fd);
			printf(" sz=%d, read=%d\n", _fileSize, r);
			assert(r == _fileSize);
		}

		fio.close(_fd);
	}
	else
		_fileSize = 0; /* new file */

	_fd = fio.open(name, mode);

	printf("open %s [%d]\n", name, _fd);

	if (_fd >= 0) {
		_mode = mode;
		_filePos = 0;

		if (_fileSize) { /* existing data */
			if (mode == O_RDONLY) {
				/* DANGER: for w* modes it will truncate your fine files */
				fio.seek(_fd, 0, SEEK_SET);
			}
			else if (_mode & O_APPEND) {
				fio.seek(_fd, 0, _fileSize);
				_filePos = _fileSize;
			}
			#if 0 /* file already trunc'd when opened as w* -> moved up */
			if (mode != O_RDONLY) {
				fio.read(_fd, _cacheBuf, _fileSize);
				r = fio.sync(_fd);
				printf(" sz=%d, read=%d\n", _fileSize, r);
				assert(r == _fileSize);
				// _fileSize = fio.seek(_fd, 0, SEEK_END);
			}
			#endif
		}

		printf("  _mode = %x\n", _mode);
		printf("  _fileSize = %d\n", _fileSize);
		printf("  _filePos = %d\n", _filePos);

		return true;
	} else
		return false;
#endif
}

int32 Ps2File::tell() {
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif
	int32 res = _filePos;
#ifdef __PS2_FILE_SEMA__
	SignalSema(_sema);
#endif
	return res;
}

int32 Ps2File::size() {
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif
	int32 res = _fileSize;
#ifdef __PS2_FILE_SEMA__
	SignalSema(_sema);
#endif
	return res;
}

bool Ps2File::eof() {
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif
	bool res = _eof; // (_filePos == _fileSize);
	// bool res = (_filePos >= _fileSize);
#ifdef __PS2_FILE_SEMA__
	SignalSema(_sema);

	// printf(" EOF [%d] : %d of %d  -> %d\n", _fd, _filePos, _fileSize, res);
#endif
	return res;
}

bool Ps2File::getErr() {
	return _err;
}

void Ps2File::setErr(bool err) {
	_err = err;
	_eof = err;
}

int Ps2File::seek(int32 offset, int origin) {
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif
	int seekDest;
	int res = -1;
	switch (origin) {
		case SEEK_SET:
			seekDest = offset;
			break;
		case SEEK_CUR:
			seekDest = _filePos + offset;
			break;
		case SEEK_END:
			seekDest = _fileSize + offset;
			break;
		default:
			seekDest = -1;
			break;
	}
	if ((seekDest >= 0) && (seekDest <= (int)_fileSize)) {
		// uint32 _rseek = fio.sync(_fd);
		_filePos = seekDest;
		// fio.seek(_fd, _filePos, SEEK_SET);
		// fio.sync(_fd);
		// _cacheSize = 0;
		_eof = false;
		res = 0;
	}
	else {
		_eof = true;
	}

	// printf("seek [%d]  %d  %d\n", _fd, offset, origin);
	// printf("  res = %d\n", res);

#ifdef __PS2_FILE_SEMA__
	SignalSema(_sema);
#endif

	return res;
}

void Ps2File::cacheReadAhead() {
	if (_cacheOpRunning) {
		// there's already some cache read running
		if (fio.poll(_fd)) // did it finish?
			cacheReadSync(); // yes.
	}
	if ((!_cacheOpRunning) && ((_readBytesBlock >= CACHE_READ_THRESHOLD) || _stream) && fio.fioAvail()) {
		// the engine seems to do sequential reads and there are no other I/Os going on. read ahead.
		uint32 cachePosEnd = _cachePos + _bytesInCache;

		if (_cachePos > _filePos)
			return; // there was a seek in the meantime, don't cache.
		if (cachePosEnd - _filePos >= CACHE_FILL_MIN)
			return; // cache is full enough.
		if (cachePosEnd == _fileSize)
			return; // can't read beyond EOF.

		assert(cachePosEnd < _fileSize);

		if (_cachePos + _bytesInCache <= _filePos) {
			_cacheOfs = _bytesInCache = 0;
			_cachePos = cachePosEnd = _filePos & ~READ_ALIGN_MASK;
			assert(_filePos == _physFilePos);
		} else {
			uint32 cacheDiff = _filePos - _cachePos;
			assert(_bytesInCache >= cacheDiff);
			cacheDiff &= ~READ_ALIGN_MASK;
			_bytesInCache -= cacheDiff;
			_cachePos += cacheDiff;
			_cacheOfs = (_cacheOfs + cacheDiff) % CACHE_SIZE;
		}

		if (_physFilePos != cachePosEnd) {
			sioprintf("unexpected _physFilePos %d cache %d %d\n", _physFilePos, _cacheOfs, _bytesInCache);
			// assert(!(cachePosEnd & READ_ALIGN_MASK)); // romeo
			_physFilePos = fio.seek(_fd, cachePosEnd, SEEK_SET);
			if (_physFilePos != cachePosEnd) {
				sioprintf("cache seek error: seek to %d instead of %d, fs = %d\n", _physFilePos, cachePosEnd, _fileSize);
				return;
			}
		}

		uint32 cacheDest = (_cacheOfs + _bytesInCache) % CACHE_SIZE;
		uint32 cacheRead = CACHE_SIZE - _bytesInCache;
		if (cacheDest + cacheRead > CACHE_SIZE)
			cacheRead = CACHE_SIZE - cacheDest;
		if (cacheRead > MAX_READ_STEP)
			cacheRead = MAX_READ_STEP;

		assert((!(cacheRead & READ_ALIGN_MASK)) && cacheRead);

		_cacheOpRunning = true;
		fio.read(_fd, _cacheBuf + cacheDest, cacheRead);
	}
}

void Ps2File::cacheReadSync() {
	if (_cacheOpRunning) {
		int res = fio.sync(_fd);
		assert(res >= 0);
		_bytesInCache += res;
		_physFilePos += res;
		_cacheOpRunning = false;
	}
}

uint32 Ps2File::read(void *dest, uint32 len) {
	// uint32 r=0, d=0, ds=0, sz=0;
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif

#ifdef __PS2_FILE_DEBUG__
	printf("read (1) : _filePos = %d\n", _filePos);
	printf("read (1) : _cachePos = %d\n", _cachePos);
#endif

	if (len == 0) {
#ifdef __PS2_FILE_SEMA__
		SignalSema(_sema);
#endif
		return 0;
	}

	if (_filePos >= _fileSize) {
		_eof = true;
#ifdef __PS2_FILE_SEMA__
		SignalSema(_sema);
#endif
		return 0;
	}

	if ((_filePos+len) > _fileSize) {
		len = _fileSize-_filePos;
		_eof = true;
	}

	uint8 *destBuf = (uint8 *)dest;
	if ((_filePos < _cachePos) || (_filePos + len > _cachePos + _bytesInCache))
		cacheReadSync(); // we have to read from CD, sync cache.

	while (len && (_filePos != _fileSize)) {
		if ((_filePos >= _cachePos) && (_filePos < _cachePos + _bytesInCache)) { // read from cache
			uint32 staPos = (_cacheOfs + (_filePos - _cachePos)) % CACHE_SIZE;
			uint32 cpyLen = _bytesInCache - (_filePos - _cachePos);
			if (cpyLen > len)
				cpyLen = len;
			if (staPos + cpyLen > CACHE_SIZE)
				cpyLen = CACHE_SIZE - staPos;

			assert(cpyLen);
			memcpy(destBuf, _cacheBuf + staPos, cpyLen);
			_filePos += cpyLen;
			destBuf += cpyLen;
			_readBytesBlock += len;
			len -= cpyLen;
		} else { // cache miss
			assert(!_cacheOpRunning);
			if (_physFilePos != _filePos) {
				if ((_filePos < _physFilePos) || (_filePos > _physFilePos + (CACHE_SIZE / 2)))
					_readBytesBlock = 0; // reset cache hit count

				_physFilePos = _filePos & ~READ_ALIGN_MASK;
				if (fio.seek(_fd, _physFilePos, SEEK_SET) != (int)_physFilePos)
					break; // read beyond EOF
			}

			int doRead = len + (_filePos - _physFilePos);
			doRead = (doRead + READ_ALIGN_MASK) & ~READ_ALIGN_MASK;

			if (doRead > MAX_READ_STEP)
				doRead = MAX_READ_STEP;
			if (doRead < 2048)
				doRead = 2048;

			fio.read(_fd, _cacheBuf, doRead);
			_cachePos = _physFilePos;
			_cacheOfs = 0;
			_bytesInCache = fio.sync(_fd);
			_physFilePos += _bytesInCache;
			if (!_bytesInCache)
				break; // EOF
		}
	}
	cacheReadAhead();
#ifdef __PS2_FILE_SEMA__
	SignalSema(_sema);
#endif
	return destBuf - (uint8 *)dest;
}

uint32 Ps2File::write(const void *src, uint32 len) {
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif

	memcpy(&_cacheBuf[_filePos], src, len);
	_filePos += len;

#ifdef __PS2_FILE_SEMA__
	SignalSema(_sema);
#endif

	return len;
}


PS2FileStream *PS2FileStream::makeFromPath(const Common::String &path, bool writeMode) {
	Ps2File *file = new Ps2File();

	int mode = writeMode ? (O_WRONLY | O_CREAT) : O_RDONLY;

	if (file->open(path.c_str(), mode))
		return new PS2FileStream(file);

	delete file;
	return 0;
}

PS2FileStream::PS2FileStream(Ps2File *handle) : _handle(handle) {
	assert(handle);
}

PS2FileStream::~PS2FileStream() {
	delete _handle;
}

bool PS2FileStream::seek(int32 offs, int whence) {
	return _handle->seek(offs, whence) == 0;
}

int32 PS2FileStream::pos() const {
	return _handle->tell();
}

bool PS2FileStream::eos() const {
	return _handle->eof();
}

uint32 PS2FileStream::read(void *ptr, uint32 len) {
	return _handle->read(ptr, len);
}

uint32 PS2FileStream::write(const void *ptr, uint32 len) {
	return _handle->write(ptr, len);
}

bool PS2FileStream::flush() {
	// printf("flush not implemented\n");
	return true;
}

bool PS2FileStream::err() const {
	bool errVal = _handle->getErr();

	if (errVal) {
		printf("ferror -> %d\n", errVal);
	}

	return errVal;
}

void PS2FileStream::clearErr() {
	_handle->setErr(false);
}

int32 PS2FileStream::size() const {
	return _handle->size();
}



FILE *ps2_fopen(const char *fname, const char *mode) {
	Ps2File *file = new Ps2File();
	int _mode = O_RDONLY;

	printf("fopen(%s, %s)\n", fname, mode);

	if (mode[0] == 'r' && mode [1] == 'w')
		_mode = O_RDWR;
	else if (mode[0] == 'w')
		_mode = O_WRONLY | O_CREAT;
	else if (mode[0] == 'a')
		_mode = O_RDWR | O_CREAT | O_APPEND;

	if (file->open(fname, _mode))
		return (FILE *)file;

	delete file;
	return NULL;
}

int ps2_fclose(FILE *stream) {
	Ps2File *file = (Ps2File *)stream;

	delete file;

	return 0;
}


size_t ps2_fread(void *buf, size_t r, size_t n, FILE *stream) {
	assert(r != 0);
	return ((Ps2File *)stream)->read(buf, r * n) / r;
}

size_t ps2_fwrite(const void *buf, size_t r, size_t n, FILE *stream) {
	assert(r != 0);
	return ((Ps2File *)stream)->write(buf, r * n) / r;
}
