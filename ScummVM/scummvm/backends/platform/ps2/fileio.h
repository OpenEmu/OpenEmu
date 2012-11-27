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

#ifndef __PS2FILE_IO__
#define __PS2FILE_IO__

#include <stdio.h>	// FIXME: Only for FILE -- get rid of this!

#include "common/scummsys.h"
#include "common/noncopyable.h"
#include "common/stream.h"

enum {
	CACHE_SIZE				= 2048 * 32,
	MAX_READ_STEP			= 2048 * 16,
	MAX_CACHED_FILES		= 6,
	CACHE_READ_THRESHOLD	= 16 * 2048,
	CACHE_FILL_MIN			= 2048 * 24,
	READ_ALIGN				= 64,   // align all reads to the size of an EE cache line
	READ_ALIGN_MASK			= READ_ALIGN - 1
};

// TODO: Make this a subclass of SeekableReadStream & WriteStream
// See also StdioStream.
class Ps2File {
public:
	Ps2File();
	virtual ~Ps2File();
	virtual bool open(const char *name, int mode);
	virtual uint32 read(void *dest, uint32 len);
	virtual uint32 write(const void *src, uint32 len);
	virtual int32 tell();
	virtual int32 size();
	virtual int seek(int32 offset, int origin);
	virtual bool eof();
	virtual bool getErr();
	virtual void setErr(bool);


private:
	void cacheReadAhead();
	void cacheReadSync();

	int _fd;
	uint32 _mode;
	uint32 _fileSize;
	uint32 _filePos;
	uint32 _cacheSize;
	uint32 _cachePos;

	uint8 *_cache;

	bool _eof;
	bool _err;
	int _sema;


	uint8 *_cacheBuf;
	bool _cacheOpRunning;
	uint32 _physFilePos;
	uint32 _bytesInCache, _cacheOfs;

	uint32 _readBytesBlock;
	bool _stream;
};

// TODO: Merge Ps2File into PS2FileStream
class PS2FileStream : public Common::SeekableReadStream, public Common::WriteStream, public Common::NonCopyable {
protected:
	/** File handle to the actual file. */
	Ps2File *_handle;

public:
	/**
	 * Given a path, invokes fopen on that path and wrap the result in a
	 * PS2FileStream instance.
	 */
	static PS2FileStream *makeFromPath(const Common::String &path, bool writeMode);

	PS2FileStream(Ps2File *handle);
	virtual ~PS2FileStream();

	virtual bool err() const;
	virtual void clearErr();
	virtual bool eos() const;

	virtual uint32 write(const void *dataPtr, uint32 dataSize);
	virtual bool flush();

	virtual int32 pos() const;
	virtual int32 size() const;
	virtual bool seek(int32 offs, int whence = SEEK_SET);
	virtual uint32 read(void *dataPtr, uint32 dataSize);
};

// TODO: Get rid of the following, instead use PS2FileStream directly.
FILE *ps2_fopen(const char *fname, const char *mode);
int ps2_fclose(FILE *stream);

size_t ps2_fread(void *buf, size_t r, size_t n, FILE *stream);
size_t ps2_fwrite(const void *buf, size_t r, size_t n, FILE *stream);

#endif // __PS2FILE_IO__
