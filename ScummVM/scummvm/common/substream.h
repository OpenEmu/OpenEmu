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

#ifndef COMMON_SUBSTREAM_H
#define COMMON_SUBSTREAM_H

#include "common/ptr.h"
#include "common/stream.h"
#include "common/types.h"

namespace Common {

/**
 * SubReadStream provides access to a ReadStream restricted to the range
 * [currentPosition, currentPosition+end).
 *
 * Manipulating the parent stream directly /will/ mess up a substream.
 * Likewise, manipulating two substreams of a parent stream will cause them to
 * step on each others toes.
 */
class SubReadStream : virtual public ReadStream {
protected:
	DisposablePtr<ReadStream> _parentStream;
	uint32 _pos;
	uint32 _end;
	bool _eos;
public:
	SubReadStream(ReadStream *parentStream, uint32 end, DisposeAfterUse::Flag disposeParentStream = DisposeAfterUse::NO)
		: _parentStream(parentStream, disposeParentStream),
		  _pos(0),
		  _end(end),
		  _eos(false) {
		assert(parentStream);
	}

	virtual bool eos() const { return _eos | _parentStream->eos(); }
	virtual bool err() const { return _parentStream->err(); }
	virtual void clearErr() { _eos = false; _parentStream->clearErr(); }
	virtual uint32 read(void *dataPtr, uint32 dataSize);
};

/*
 * SeekableSubReadStream provides access to a SeekableReadStream restricted to
 * the range [begin, end).
 * The same caveats apply to SeekableSubReadStream as do to SeekableReadStream.
 *
 * Manipulating the parent stream directly /will/ mess up a substream.
 * @see SubReadStream
 */
class SeekableSubReadStream : public SubReadStream, public SeekableReadStream {
protected:
	SeekableReadStream *_parentStream;
	uint32 _begin;
public:
	SeekableSubReadStream(SeekableReadStream *parentStream, uint32 begin, uint32 end, DisposeAfterUse::Flag disposeParentStream = DisposeAfterUse::NO);

	virtual int32 pos() const { return _pos - _begin; }
	virtual int32 size() const { return _end - _begin; }

	virtual bool seek(int32 offset, int whence = SEEK_SET);
};

/**
 * This is a SeekableSubReadStream subclass which adds non-endian
 * read methods whose endianness is set on the stream creation.
 *
 * Manipulating the parent stream directly /will/ mess up a substream.
 * @see SubReadStream
 */
class SeekableSubReadStreamEndian : public SeekableSubReadStream, public ReadStreamEndian {
public:
	SeekableSubReadStreamEndian(SeekableReadStream *parentStream, uint32 begin, uint32 end, bool bigEndian, DisposeAfterUse::Flag disposeParentStream = DisposeAfterUse::NO)
		: SeekableSubReadStream(parentStream, begin, end, disposeParentStream),
		  ReadStreamEndian(bigEndian) {
	}
};

/**
 * A seekable substream that removes the exclusivity demand required by the
 * normal SeekableSubReadStream, at the cost of seek()ing the parent stream
 * before each read().
 *
 * More than one SafeSeekableSubReadStream to the same parent stream can be used
 * at the same time; they won't mess up each other. They will, however,
 * reposition the parent stream, so don't depend on its position to be
 * the same after a read() or seek() on one of its SafeSeekableSubReadStream.
 *
 * Note that this stream is *not* threading safe. Calling read from the audio
 * thread and from the main thread might mess up the data retrieved.
 */
class SafeSeekableSubReadStream : public SeekableSubReadStream {
public:
	SafeSeekableSubReadStream(SeekableReadStream *parentStream, uint32 begin, uint32 end, DisposeAfterUse::Flag disposeParentStream = DisposeAfterUse::NO)
		: SeekableSubReadStream(parentStream, begin, end, disposeParentStream) {
	}

	virtual uint32 read(void *dataPtr, uint32 dataSize);
};


} // End of namespace Common

#endif
