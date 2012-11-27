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

#include "common/endian.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Audio {

// This used to be an inline template function, but
// buggy template function handling in MSVC6 forced
// us to go with the macro approach. So far this is
// the only template function that MSVC6 seemed to
// compile incorrectly. Knock on wood.
#define READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, ptr, isLE) \
	((is16Bit ? (isLE ? READ_LE_UINT16(ptr) : READ_BE_UINT16(ptr)) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))


#pragma mark -
#pragma mark --- RawStream ---
#pragma mark -

/**
 * This is a stream, which allows for playing raw PCM data from a stream.
 */
template<bool is16Bit, bool isUnsigned, bool isLE>
class RawStream : public SeekableAudioStream {
public:
	RawStream(int rate, bool stereo, DisposeAfterUse::Flag disposeStream, Common::SeekableReadStream *stream)
		: _rate(rate), _isStereo(stereo), _playtime(0, rate), _stream(stream, disposeStream), _endOfData(false), _buffer(0) {
		// Setup our buffer for readBuffer
		_buffer = new byte[kSampleBufferLength * (is16Bit ? 2 : 1)];
		assert(_buffer);

		// Calculate the total playtime of the stream
		_playtime = Timestamp(0, _stream->size() / (_isStereo ? 2 : 1) / (is16Bit ? 2 : 1), rate);
	}

	~RawStream() {
		delete[] _buffer;
	}

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const  { return _isStereo; }
	bool endOfData() const { return _endOfData; }

	int getRate() const         { return _rate; }
	Timestamp getLength() const { return _playtime; }

	bool seek(const Timestamp &where);
private:
	const int _rate;                                           ///< Sample rate of stream
	const bool _isStereo;                                      ///< Whether this is an stereo stream
	Timestamp _playtime;                                       ///< Calculated total play time
	Common::DisposablePtr<Common::SeekableReadStream> _stream; ///< Stream to read data from
	bool _endOfData;                                           ///< Whether the stream end has been reached

	byte *_buffer;                                             ///< Buffer used in readBuffer
	enum {
		/**
		 * How many samples we can buffer at once.
		 *
		 * TODO: Check whether this size suffices
		 * for systems with slow disk I/O.
		 */
		kSampleBufferLength = 2048
	};

	/**
	 * Fill the temporary sample buffer used in readBuffer.
	 *
	 * @param maxSamples Maximum samples to read.
	 * @return actual count of samples read.
	 */
	int fillBuffer(int maxSamples);
};

template<bool is16Bit, bool isUnsigned, bool isLE>
int RawStream<is16Bit, isUnsigned, isLE>::readBuffer(int16 *buffer, const int numSamples) {
	int samplesLeft = numSamples;

	while (samplesLeft > 0) {
		// Try to read up to "samplesLeft" samples.
		int len = fillBuffer(samplesLeft);

		// In case we were not able to read any samples
		// we will stop reading here.
		if (!len)
			break;

		// Adjust the samples left to read.
		samplesLeft -= len;

		// Copy the data to the caller's buffer.
		const byte *src = _buffer;
		while (len-- > 0) {
			*buffer++ = READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, src, isLE);
			src += (is16Bit ? 2 : 1);
		}
	}

	return numSamples - samplesLeft;
}

template<bool is16Bit, bool isUnsigned, bool isLE>
int RawStream<is16Bit, isUnsigned, isLE>::fillBuffer(int maxSamples) {
	int bufferedSamples = 0;
	byte *dst = _buffer;

	// We can only read up to "kSampleBufferLength" samples
	// so we take this into consideration, when trying to
	// read up to maxSamples.
	maxSamples = MIN<int>(kSampleBufferLength, maxSamples);

	// We will only read up to maxSamples
	while (maxSamples > 0 && !endOfData()) {
		// Try to read all the sample data and update the
		// destination pointer.
		const int bytesRead = _stream->read(dst, maxSamples * (is16Bit ? 2 : 1));
		dst += bytesRead;

		// Calculate how many samples we actually read.
		const int samplesRead = bytesRead / (is16Bit ? 2 : 1);

		// Update all status variables
		bufferedSamples += samplesRead;
		maxSamples -= samplesRead;

		// We stop stream playback, when we reached the end of the data stream.
		// We also stop playback when an error occures.
		if (_stream->pos() == _stream->size() || _stream->err() || _stream->eos())
			_endOfData = true;
	}

	return bufferedSamples;
}

template<bool is16Bit, bool isUnsigned, bool isLE>
bool RawStream<is16Bit, isUnsigned, isLE>::seek(const Timestamp &where) {
	_endOfData = true;

	if (where > _playtime)
		return false;

	const uint32 seekSample = convertTimeToStreamPos(where, getRate(), isStereo()).totalNumberOfFrames();
	_stream->seek(seekSample * (is16Bit ? 2 : 1), SEEK_SET);

	// In case of an error we will not continue stream playback.
	if (!_stream->err() && !_stream->eos() && _stream->pos() != _stream->size())
		_endOfData = false;

	return true;
}

#pragma mark -
#pragma mark --- Raw stream factories ---
#pragma mark -

/* In the following, we use preprocessor / macro tricks to simplify the code
 * which instantiates the input streams. We used to use template functions for
 * this, but MSVC6 / EVC 3-4 (used for WinCE builds) are extremely buggy when it
 * comes to this feature of C++... so as a compromise we use macros to cut down
 * on the (source) code duplication a bit.
 * So while normally macro tricks are said to make maintenance harder, in this
 * particular case it should actually help it :-)
 */

#define MAKE_RAW_STREAM(UNSIGNED) \
		if (is16Bit) { \
			if (isLE) \
				return new RawStream<true, UNSIGNED, true>(rate, isStereo, disposeAfterUse, stream); \
			else  \
				return new RawStream<true, UNSIGNED, false>(rate, isStereo, disposeAfterUse, stream); \
		} else \
			return new RawStream<false, UNSIGNED, false>(rate, isStereo, disposeAfterUse, stream)

SeekableAudioStream *makeRawStream(Common::SeekableReadStream *stream,
                                   int rate, byte flags,
                                   DisposeAfterUse::Flag disposeAfterUse) {
	const bool isStereo   = (flags & Audio::FLAG_STEREO) != 0;
	const bool is16Bit    = (flags & Audio::FLAG_16BITS) != 0;
	const bool isUnsigned = (flags & Audio::FLAG_UNSIGNED) != 0;
	const bool isLE       = (flags & Audio::FLAG_LITTLE_ENDIAN) != 0;

	assert(stream->size() % ((is16Bit ? 2 : 1) * (isStereo ? 2 : 1)) == 0);

	if (isUnsigned) {
		MAKE_RAW_STREAM(true);
	} else {
		MAKE_RAW_STREAM(false);
	}
}

SeekableAudioStream *makeRawStream(const byte *buffer, uint32 size,
                                   int rate, byte flags,
                                   DisposeAfterUse::Flag disposeAfterUse) {
	return makeRawStream(new Common::MemoryReadStream(buffer, size, disposeAfterUse), rate, flags, DisposeAfterUse::YES);
}

} // End of namespace Audio
