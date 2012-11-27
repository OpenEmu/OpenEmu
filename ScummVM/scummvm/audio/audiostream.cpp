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

#include "common/debug.h"
#include "common/file.h"
#include "common/mutex.h"
#include "common/textconsole.h"
#include "common/queue.h"
#include "common/util.h"

#include "audio/audiostream.h"
#include "audio/decoders/flac.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/quicktime.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/vorbis.h"


namespace Audio {

struct StreamFileFormat {
	/** Decodername */
	const char *decoderName;
	const char *fileExtension;
	/**
	 * Pointer to a function which tries to open a file of type StreamFormat.
	 * Return NULL in case of an error (invalid/nonexisting file).
	 */
	SeekableAudioStream *(*openStreamFile)(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);
};

static const StreamFileFormat STREAM_FILEFORMATS[] = {
	/* decoderName,  fileExt, openStreamFunction */
#ifdef USE_FLAC
	{ "FLAC",         ".flac", makeFLACStream },
	{ "FLAC",         ".fla",  makeFLACStream },
#endif
#ifdef USE_VORBIS
	{ "Ogg Vorbis",   ".ogg",  makeVorbisStream },
#endif
#ifdef USE_MAD
	{ "MPEG Layer 3", ".mp3",  makeMP3Stream },
#endif
	{ "MPEG-4 Audio",   ".m4a",  makeQuickTimeStream },
};

SeekableAudioStream *SeekableAudioStream::openStreamFile(const Common::String &basename) {
	SeekableAudioStream *stream = NULL;
	Common::File *fileHandle = new Common::File();

	for (int i = 0; i < ARRAYSIZE(STREAM_FILEFORMATS); ++i) {
		Common::String filename = basename + STREAM_FILEFORMATS[i].fileExtension;
		fileHandle->open(filename);
		if (fileHandle->isOpen()) {
			// Create the stream object
			stream = STREAM_FILEFORMATS[i].openStreamFile(fileHandle, DisposeAfterUse::YES);
			fileHandle = 0;
			break;
		}
	}

	delete fileHandle;

	if (stream == NULL)
		debug(1, "SeekableAudioStream::openStreamFile: Could not open compressed AudioFile %s", basename.c_str());

	return stream;
}

#pragma mark -
#pragma mark --- LoopingAudioStream ---
#pragma mark -

LoopingAudioStream::LoopingAudioStream(RewindableAudioStream *stream, uint loops, DisposeAfterUse::Flag disposeAfterUse)
    : _parent(stream, disposeAfterUse), _loops(loops), _completeIterations(0) {
	assert(stream);

	if (!stream->rewind()) {
		// TODO: Properly indicate error
		_loops = _completeIterations = 1;
	}
}

int LoopingAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	if ((_loops && _completeIterations == _loops) || !numSamples)
		return 0;

	int samplesRead = _parent->readBuffer(buffer, numSamples);

	if (_parent->endOfStream()) {
		++_completeIterations;
		if (_completeIterations == _loops)
			return samplesRead;

		const int remainingSamples = numSamples - samplesRead;

		if (!_parent->rewind()) {
			// TODO: Properly indicate error
			_loops = _completeIterations = 1;
			return samplesRead;
		}

		return samplesRead + readBuffer(buffer + samplesRead, remainingSamples);
	}

	return samplesRead;
}

bool LoopingAudioStream::endOfData() const {
	return (_loops != 0 && (_completeIterations == _loops));
}

AudioStream *makeLoopingAudioStream(RewindableAudioStream *stream, uint loops) {
	if (loops != 1)
		return new LoopingAudioStream(stream, loops);
	else
		return stream;
}

AudioStream *makeLoopingAudioStream(SeekableAudioStream *stream, Timestamp start, Timestamp end, uint loops) {
	if (!start.totalNumberOfFrames() && (!end.totalNumberOfFrames() || end == stream->getLength())) {
		return makeLoopingAudioStream(stream, loops);
	} else {
		if (!end.totalNumberOfFrames())
			end = stream->getLength();

		if (start >= end) {
			warning("makeLoopingAudioStream: start (%d) >= end (%d)", start.msecs(), end.msecs());
			delete stream;
			return 0;
		}

		return makeLoopingAudioStream(new SubSeekableAudioStream(stream, start, end), loops);
	}
}

#pragma mark -
#pragma mark --- SubLoopingAudioStream ---
#pragma mark -

SubLoopingAudioStream::SubLoopingAudioStream(SeekableAudioStream *stream,
                                             uint loops,
                                             const Timestamp loopStart,
                                             const Timestamp loopEnd,
                                             DisposeAfterUse::Flag disposeAfterUse)
    : _parent(stream, disposeAfterUse), _loops(loops),
      _pos(0, getRate() * (isStereo() ? 2 : 1)),
      _loopStart(convertTimeToStreamPos(loopStart, getRate(), isStereo())),
      _loopEnd(convertTimeToStreamPos(loopEnd, getRate(), isStereo())),
      _done(false) {
	assert(loopStart < loopEnd);

	if (!_parent->rewind())
		_done = true;
}

int SubLoopingAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	if (_done)
		return 0;

	int framesLeft = MIN(_loopEnd.frameDiff(_pos), numSamples);
	int framesRead = _parent->readBuffer(buffer, framesLeft);
	_pos = _pos.addFrames(framesRead);

	if (framesRead < framesLeft && _parent->endOfData()) {
		// TODO: Proper error indication.
		_done = true;
		return framesRead;
	} else if (_pos == _loopEnd) {
		if (_loops != 0) {
			--_loops;
			if (!_loops) {
				_done = true;
				return framesRead;
			}
		}

		if (!_parent->seek(_loopStart)) {
			// TODO: Proper error indication.
			_done = true;
			return framesRead;
		}

		_pos = _loopStart;
		framesLeft = numSamples - framesLeft;
		return framesRead + readBuffer(buffer + framesRead, framesLeft);
	} else {
		return framesRead;
	}
}

#pragma mark -
#pragma mark --- SubSeekableAudioStream ---
#pragma mark -

SubSeekableAudioStream::SubSeekableAudioStream(SeekableAudioStream *parent, const Timestamp start, const Timestamp end, DisposeAfterUse::Flag disposeAfterUse)
    : _parent(parent, disposeAfterUse),
      _start(convertTimeToStreamPos(start, getRate(), isStereo())),
      _pos(0, getRate() * (isStereo() ? 2 : 1)),
      _length(convertTimeToStreamPos(end, getRate(), isStereo()) - _start) {

	assert(_length.totalNumberOfFrames() % (isStereo() ? 2 : 1) == 0);
	_parent->seek(_start);
}

int SubSeekableAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int framesLeft = MIN(_length.frameDiff(_pos), numSamples);
	int framesRead = _parent->readBuffer(buffer, framesLeft);
	_pos = _pos.addFrames(framesRead);
	return framesRead;
}

bool SubSeekableAudioStream::seek(const Timestamp &where) {
	_pos = convertTimeToStreamPos(where, getRate(), isStereo());
	if (_pos > _length) {
		_pos = _length;
		return false;
	}

	if (_parent->seek(_pos + _start)) {
		return true;
	} else {
		_pos = _length;
		return false;
	}
}

#pragma mark -
#pragma mark --- Queueing audio stream ---
#pragma mark -


void QueuingAudioStream::queueBuffer(byte *data, uint32 size, DisposeAfterUse::Flag disposeAfterUse, byte flags) {
	AudioStream *stream = makeRawStream(data, size, getRate(), flags, disposeAfterUse);
	queueAudioStream(stream, DisposeAfterUse::YES);
}


class QueuingAudioStreamImpl : public QueuingAudioStream {
private:
	/**
	 * We queue a number of (pointers to) audio stream objects.
	 * In addition, we need to remember for each stream whether
	 * to dispose it after all data has been read from it.
	 * Hence, we don't store pointers to stream objects directly,
	 * but rather StreamHolder structs.
	 */
	struct StreamHolder {
		AudioStream *_stream;
		DisposeAfterUse::Flag _disposeAfterUse;
		StreamHolder(AudioStream *stream, DisposeAfterUse::Flag disposeAfterUse)
		    : _stream(stream),
		      _disposeAfterUse(disposeAfterUse) {}
	};

	/**
	 * The sampling rate of this audio stream.
	 */
	const int _rate;

	/**
	 * Whether this audio stream is mono (=false) or stereo (=true).
	 */
	const int _stereo;

	/**
	 * This flag is set by the finish() method only. See there for more details.
	 */
	bool _finished;

	/**
	 * A mutex to avoid access problems (causing e.g. corruption of
	 * the linked list) in thread aware environments.
	 */
	Common::Mutex _mutex;

	/**
	 * The queue of audio streams.
	 */
	Common::Queue<StreamHolder> _queue;

public:
	QueuingAudioStreamImpl(int rate, bool stereo)
	    : _rate(rate), _stereo(stereo), _finished(false) {}
	~QueuingAudioStreamImpl();

	// Implement the AudioStream API
	virtual int readBuffer(int16 *buffer, const int numSamples);
	virtual bool isStereo() const { return _stereo; }
	virtual int getRate() const { return _rate; }
	virtual bool endOfData() const {
		//Common::StackLock lock(_mutex);
		return _queue.empty();
	}
	virtual bool endOfStream() const { return _finished && _queue.empty(); }

	// Implement the QueuingAudioStream API
	virtual void queueAudioStream(AudioStream *stream, DisposeAfterUse::Flag disposeAfterUse);
	virtual void finish() { _finished = true; }

	uint32 numQueuedStreams() const {
		//Common::StackLock lock(_mutex);
		return _queue.size();
	}
};

QueuingAudioStreamImpl::~QueuingAudioStreamImpl() {
	while (!_queue.empty()) {
		StreamHolder tmp = _queue.pop();
		if (tmp._disposeAfterUse == DisposeAfterUse::YES)
			delete tmp._stream;
	}
}

void QueuingAudioStreamImpl::queueAudioStream(AudioStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	assert(!_finished);
	if ((stream->getRate() != getRate()) || (stream->isStereo() != isStereo()))
		error("QueuingAudioStreamImpl::queueAudioStream: stream has mismatched parameters");

	Common::StackLock lock(_mutex);
	_queue.push(StreamHolder(stream, disposeAfterUse));
}

int QueuingAudioStreamImpl::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);
	int samplesDecoded = 0;

	while (samplesDecoded < numSamples && !_queue.empty()) {
		AudioStream *stream = _queue.front()._stream;
		samplesDecoded += stream->readBuffer(buffer + samplesDecoded, numSamples - samplesDecoded);

		if (stream->endOfData()) {
			StreamHolder tmp = _queue.pop();
			if (tmp._disposeAfterUse == DisposeAfterUse::YES)
				delete stream;
		}
	}

	return samplesDecoded;
}

QueuingAudioStream *makeQueuingAudioStream(int rate, bool stereo) {
	return new QueuingAudioStreamImpl(rate, stereo);
}

Timestamp convertTimeToStreamPos(const Timestamp &where, int rate, bool isStereo) {
	Timestamp result(where.convertToFramerate(rate * (isStereo ? 2 : 1)));

	// When the Stream is a stereo stream, we have to assure
	// that the sample position is an even number.
	if (isStereo && (result.totalNumberOfFrames() & 1))
		result = result.addFrames(-1); // We cut off one sample here.

	// Since Timestamp allows sub-frame-precision it might lead to odd behaviors
	// when we would just return result.
	//
	// An example is when converting the timestamp 500ms to a 11025 Hz based
	// stream. It would have an internal frame counter of 5512.5. Now when
	// doing calculations at frame precision, this might lead to unexpected
	// results: The frame difference between a timestamp 1000ms and the above
	// mentioned timestamp (both with 11025 as framerate) would be 5512,
	// instead of 5513, which is what a frame-precision based code would expect.
	//
	// By creating a new Timestamp with the given parameters, we create a
	// Timestamp with frame-precision, which just drops a sub-frame-precision
	// information (i.e. rounds down).
	return Timestamp(result.secs(), result.numberOfFrames(), result.framerate());
}

/**
 * An AudioStream wrapper that cuts off the amount of samples read after a
 * given time length is reached.
 */
class LimitingAudioStream : public AudioStream {
public:
	LimitingAudioStream(AudioStream *parentStream, const Audio::Timestamp &length, DisposeAfterUse::Flag disposeAfterUse) :
			_parentStream(parentStream), _samplesRead(0), _disposeAfterUse(disposeAfterUse),
			_totalSamples(length.convertToFramerate(getRate()).totalNumberOfFrames() * getChannels()) {}

	~LimitingAudioStream() {
		if (_disposeAfterUse == DisposeAfterUse::YES)
			delete _parentStream;
	}

	int readBuffer(int16 *buffer, const int numSamples) {
		// Cap us off so we don't read past _totalSamples
		int samplesRead = _parentStream->readBuffer(buffer, MIN<int>(numSamples, _totalSamples - _samplesRead));
		_samplesRead += samplesRead;
		return samplesRead;
	}

	bool endOfData() const { return _parentStream->endOfData() || _samplesRead >= _totalSamples; }
	bool isStereo() const { return _parentStream->isStereo(); }
	int getRate() const { return _parentStream->getRate(); }

private:
	int getChannels() const { return isStereo() ? 2 : 1; }

	AudioStream *_parentStream;
	DisposeAfterUse::Flag _disposeAfterUse;
	uint32 _totalSamples, _samplesRead;
};

AudioStream *makeLimitingAudioStream(AudioStream *parentStream, const Timestamp &length, DisposeAfterUse::Flag disposeAfterUse) {
	return new LimitingAudioStream(parentStream, length, disposeAfterUse);
}

} // End of namespace Audio
