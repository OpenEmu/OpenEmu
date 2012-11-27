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

#include "audio/decoders/mp3.h"

#ifdef USE_MAD

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/audiostream.h"

#include <mad.h>

#if defined(__PSP__)
	#include "backends/platform/psp/mp3.h"
#endif
namespace Audio {


#pragma mark -
#pragma mark --- MP3 (MAD) stream ---
#pragma mark -


class MP3Stream : public SeekableAudioStream {
protected:
	enum State {
		MP3_STATE_INIT,	// Need to init the decoder
		MP3_STATE_READY,	// ready for processing data
		MP3_STATE_EOS		// end of data reached (may need to loop)
	};

	Common::DisposablePtr<Common::SeekableReadStream> _inStream;

	uint _posInFrame;
	State _state;

	Timestamp _length;
	mad_timer_t _totalTime;

	mad_stream _stream;
	mad_frame _frame;
	mad_synth _synth;

	enum {
		BUFFER_SIZE = 5 * 8192
	};

	// This buffer contains a slab of input data
	byte _buf[BUFFER_SIZE + MAD_BUFFER_GUARD];

public:
	MP3Stream(Common::SeekableReadStream *inStream,
	               DisposeAfterUse::Flag dispose);
	~MP3Stream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const		{ return _state == MP3_STATE_EOS; }
	bool isStereo() const		{ return MAD_NCHANNELS(&_frame.header) == 2; }
	int getRate() const			{ return _frame.header.samplerate; }

	bool seek(const Timestamp &where);
	Timestamp getLength() const { return _length; }
protected:
	void decodeMP3Data();
	void readMP3Data();

	void initStream();
	void readHeader();
	void deinitStream();
};

MP3Stream::MP3Stream(Common::SeekableReadStream *inStream, DisposeAfterUse::Flag dispose) :
	_inStream(inStream, dispose),
	_posInFrame(0),
	_state(MP3_STATE_INIT),
	_length(0, 1000),
	_totalTime(mad_timer_zero) {

	// The MAD_BUFFER_GUARD must always contain zeros (the reason
	// for this is that the Layer III Huffman decoder of libMAD
	// may read a few bytes beyond the end of the input buffer).
	memset(_buf + BUFFER_SIZE, 0, MAD_BUFFER_GUARD);

	// Calculate the length of the stream
	initStream();

	while (_state != MP3_STATE_EOS)
		readHeader();

	// To rule out any invalid sample rate to be encountered here, say in case the
	// MP3 stream is invalid, we just check the MAD error code here.
	// We need to assure this, since else we might trigger an assertion in Timestamp
	// (When getRate() returns 0 or a negative number to be precise).
	// Note that we allow "MAD_ERROR_BUFLEN" as error code here, since according
	// to mad.h it is also set on EOF.
	if ((_stream.error == MAD_ERROR_NONE || _stream.error == MAD_ERROR_BUFLEN) && getRate() > 0)
		_length = Timestamp(mad_timer_count(_totalTime, MAD_UNITS_MILLISECONDS), getRate());

	deinitStream();

	// Reinit stream
	_state = MP3_STATE_INIT;

	// Decode the first chunk of data. This is necessary so that _frame
	// is setup and isStereo() and getRate() return correct results.
	decodeMP3Data();
}

MP3Stream::~MP3Stream() {
	deinitStream();
}

void MP3Stream::decodeMP3Data() {
	do {
		if (_state == MP3_STATE_INIT)
			initStream();

		if (_state == MP3_STATE_EOS)
			return;

		// If necessary, load more data into the stream decoder
		if (_stream.error == MAD_ERROR_BUFLEN)
			readMP3Data();

		while (_state == MP3_STATE_READY) {
			_stream.error = MAD_ERROR_NONE;

			// Decode the next frame
			if (mad_frame_decode(&_frame, &_stream) == -1) {
				if (_stream.error == MAD_ERROR_BUFLEN) {
					break; // Read more data
				} else if (MAD_RECOVERABLE(_stream.error)) {
					// Note: we will occasionally see MAD_ERROR_BADDATAPTR errors here.
					// These are normal and expected (caused by our frame skipping (i.e. "seeking")
					// code above).
					debug(6, "MP3Stream: Recoverable error in mad_frame_decode (%s)", mad_stream_errorstr(&_stream));
					continue;
				} else {
					warning("MP3Stream: Unrecoverable error in mad_frame_decode (%s)", mad_stream_errorstr(&_stream));
					break;
				}
			}

			// Synthesize PCM data
			mad_synth_frame(&_synth, &_frame);
			_posInFrame = 0;
			break;
		}
	} while (_state != MP3_STATE_EOS && _stream.error == MAD_ERROR_BUFLEN);

	if (_stream.error != MAD_ERROR_NONE)
		_state = MP3_STATE_EOS;
}

void MP3Stream::readMP3Data() {
	uint32 remaining = 0;

	// Give up immediately if we already used up all data in the stream
	if (_inStream->eos()) {
		_state = MP3_STATE_EOS;
		return;
	}

	if (_stream.next_frame) {
		// If there is still data in the MAD stream, we need to preserve it.
		// Note that we use memmove, as we are reusing the same buffer,
		// and hence the data regions we copy from and to may overlap.
		remaining = _stream.bufend - _stream.next_frame;
		assert(remaining < BUFFER_SIZE);	// Paranoia check
		memmove(_buf, _stream.next_frame, remaining);
	}

	// Try to read the next block
	uint32 size = _inStream->read(_buf + remaining, BUFFER_SIZE - remaining);
	if (size <= 0) {
		_state = MP3_STATE_EOS;
		return;
	}

	// Feed the data we just read into the stream decoder
	_stream.error = MAD_ERROR_NONE;
	mad_stream_buffer(&_stream, _buf, size + remaining);
}

bool MP3Stream::seek(const Timestamp &where) {
	if (where == _length) {
		_state = MP3_STATE_EOS;
		return true;
	} else if (where > _length) {
		return false;
	}

	const uint32 time = where.msecs();

	mad_timer_t destination;
	mad_timer_set(&destination, time / 1000, time % 1000, 1000);

	if (_state != MP3_STATE_READY || mad_timer_compare(destination, _totalTime) < 0)
		initStream();

	while (mad_timer_compare(destination, _totalTime) > 0 && _state != MP3_STATE_EOS)
		readHeader();

	decodeMP3Data();

	return (_state != MP3_STATE_EOS);
}

void MP3Stream::initStream() {
	if (_state != MP3_STATE_INIT)
		deinitStream();

	// Init MAD
	mad_stream_init(&_stream);
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);

	// Reset the stream data
	_inStream->seek(0, SEEK_SET);
	_totalTime = mad_timer_zero;
	_posInFrame = 0;

	// Update state
	_state = MP3_STATE_READY;

	// Read the first few sample bytes
	readMP3Data();
}

void MP3Stream::readHeader() {
	if (_state != MP3_STATE_READY)
		return;

	// If necessary, load more data into the stream decoder
	if (_stream.error == MAD_ERROR_BUFLEN)
		readMP3Data();

	while (_state != MP3_STATE_EOS) {
		_stream.error = MAD_ERROR_NONE;

		// Decode the next header. Note: mad_frame_decode would do this for us, too.
		// However, for seeking we don't want to decode the full frame (else it would
		// be far too slow). Hence we perform this explicitly in a separate step.
		if (mad_header_decode(&_frame.header, &_stream) == -1) {
			if (_stream.error == MAD_ERROR_BUFLEN) {
				readMP3Data();  // Read more data
				continue;
			} else if (MAD_RECOVERABLE(_stream.error)) {
				debug(6, "MP3Stream: Recoverable error in mad_header_decode (%s)", mad_stream_errorstr(&_stream));
				continue;
			} else {
				warning("MP3Stream: Unrecoverable error in mad_header_decode (%s)", mad_stream_errorstr(&_stream));
				break;
			}
		}

		// Sum up the total playback time so far
		mad_timer_add(&_totalTime, _frame.header.duration);
		break;
	}

	if (_stream.error != MAD_ERROR_NONE)
		_state = MP3_STATE_EOS;
}

void MP3Stream::deinitStream() {
	if (_state == MP3_STATE_INIT)
		return;

	// Deinit MAD
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);

	_state = MP3_STATE_EOS;
}

static inline int scale_sample(mad_fixed_t sample) {
	// round
	sample += (1L << (MAD_F_FRACBITS - 16));

	// clip
	if (sample > MAD_F_ONE - 1)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	// quantize and scale to not saturate when mixing a lot of channels
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

int MP3Stream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	// Keep going as long as we have input available
	while (samples < numSamples && _state != MP3_STATE_EOS) {
		const int len = MIN(numSamples, samples + (int)(_synth.pcm.length - _posInFrame) * MAD_NCHANNELS(&_frame.header));
		while (samples < len) {
			*buffer++ = (int16)scale_sample(_synth.pcm.samples[0][_posInFrame]);
			samples++;
			if (MAD_NCHANNELS(&_frame.header) == 2) {
				*buffer++ = (int16)scale_sample(_synth.pcm.samples[1][_posInFrame]);
				samples++;
			}
			_posInFrame++;
		}
		if (_posInFrame >= _synth.pcm.length) {
			// We used up all PCM data in the current frame -- read & decode more
			decodeMP3Data();
		}
	}
	return samples;
}


#pragma mark -
#pragma mark --- MP3 factory functions ---
#pragma mark -

SeekableAudioStream *makeMP3Stream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse) {

#if defined(__PSP__)
	SeekableAudioStream *s = 0;

	if (Mp3PspStream::isOkToCreateStream())
		s = new Mp3PspStream(stream, disposeAfterUse);

	if (!s)	// go to regular MAD mp3 stream if ME fails
		s = new MP3Stream(stream, disposeAfterUse);
#else
	SeekableAudioStream *s = new MP3Stream(stream, disposeAfterUse);
#endif
	if (s && s->endOfData()) {
		delete s;
		return 0;
	} else {
		return s;
	}
}

} // End of namespace Audio

#endif // #ifdef USE_MAD
