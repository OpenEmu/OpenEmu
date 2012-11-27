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

#ifndef COMMON_MEMSTREAM_H
#define COMMON_MEMSTREAM_H

#include "common/stream.h"
#include "common/types.h"

namespace Common {

/**
 * Simple memory based 'stream', which implements the ReadStream interface for
 * a plain memory block.
 */
class MemoryReadStream : public SeekableReadStream {
private:
	const byte * const _ptrOrig;
	const byte *_ptr;
	const uint32 _size;
	uint32 _pos;
	DisposeAfterUse::Flag _disposeMemory;
	bool _eos;

public:

	/**
	 * This constructor takes a pointer to a memory buffer and a length, and
	 * wraps it. If disposeMemory is true, the MemoryReadStream takes ownership
	 * of the buffer and hence free's it when destructed.
	 */
	MemoryReadStream(const byte *dataPtr, uint32 dataSize, DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO) :
		_ptrOrig(dataPtr),
		_ptr(dataPtr),
		_size(dataSize),
		_pos(0),
		_disposeMemory(disposeMemory),
		_eos(false) {}

	~MemoryReadStream() {
		if (_disposeMemory)
			free(const_cast<byte *>(_ptrOrig));
	}

	uint32 read(void *dataPtr, uint32 dataSize);

	bool eos() const { return _eos; }
	void clearErr() { _eos = false; }

	int32 pos() const { return _pos; }
	int32 size() const { return _size; }

	bool seek(int32 offs, int whence = SEEK_SET);
};


/**
 * This is a MemoryReadStream subclass which adds non-endian
 * read methods whose endianness is set on the stream creation.
 */
class MemoryReadStreamEndian : public MemoryReadStream, public ReadStreamEndian {
public:
	MemoryReadStreamEndian(const byte *buf, uint32 len, bool bigEndian)
		: MemoryReadStream(buf, len), ReadStreamEndian(bigEndian) {}
};

/**
 * Simple memory based 'stream', which implements the WriteStream interface for
 * a plain memory block.
 */
class MemoryWriteStream : public WriteStream {
private:
	byte *_ptr;
	const uint32 _bufSize;
	uint32 _pos;
	bool _err;
public:
	MemoryWriteStream(byte *buf, uint32 len) : _ptr(buf), _bufSize(len), _pos(0), _err(false) {}

	uint32 write(const void *dataPtr, uint32 dataSize) {
		// Write at most as many bytes as are still available...
		if (dataSize > _bufSize - _pos) {
			dataSize = _bufSize - _pos;
			// We couldn't write all the data => set error indicator
			_err = true;
		}
		memcpy(_ptr, dataPtr, dataSize);
		_ptr += dataSize;
		_pos += dataSize;
		return dataSize;
	}

	uint32 pos() const { return _pos; }
	uint32 size() const { return _bufSize; }

	virtual bool err() const { return _err; }
	virtual void clearErr() { _err = false; }
};

/**
 * A sort of hybrid between MemoryWriteStream and Array classes. A stream
 * that grows as it's written to.
 */
class MemoryWriteStreamDynamic : public WriteStream {
private:
	uint32 _capacity;
	uint32 _size;
	byte *_ptr;
	byte *_data;
	uint32 _pos;
	DisposeAfterUse::Flag _disposeMemory;

	void ensureCapacity(uint32 new_len) {
		if (new_len <= _capacity)
			return;

		byte *old_data = _data;

		_capacity = new_len + 32;
		_data = (byte *)malloc(_capacity);
		_ptr = _data + _pos;

		if (old_data) {
			// Copy old data
			memcpy(_data, old_data, _size);
			free(old_data);
		}

		_size = new_len;
	}
public:
	MemoryWriteStreamDynamic(DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO) : _capacity(0), _size(0), _ptr(0), _data(0), _pos(0), _disposeMemory(disposeMemory) {}

	~MemoryWriteStreamDynamic() {
		if (_disposeMemory)
			free(_data);
	}

	uint32 write(const void *dataPtr, uint32 dataSize) {
		ensureCapacity(_pos + dataSize);
		memcpy(_ptr, dataPtr, dataSize);
		_ptr += dataSize;
		_pos += dataSize;
		if (_pos > _size)
			_size = _pos;
		return dataSize;
	}

	uint32 pos() const { return _pos; }
	uint32 size() const { return _size; }

	byte *getData() { return _data; }

	bool seek(int32 offset, int whence = SEEK_SET);
};

}	// End of namespace Common

#endif
