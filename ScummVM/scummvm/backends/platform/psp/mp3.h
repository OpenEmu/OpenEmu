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

#ifndef SOUND_MP3_PSP_H
#define SOUND_MP3_PSP_H

#include "common/ptr.h"
#include "common/types.h"
#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {

class AudioStream;
class SeekableAudioStream;

class Mp3PspStream : public SeekableAudioStream {
protected:
	enum State {
		MP3_STATE_INIT,		// Need to init the decoder
		MP3_STATE_READY,	// ready for processing data
		MP3_STATE_EOS		// end of data reached (may need to loop)
	};

	#define MAX_SAMPLES_PER_FRAME	1152 * 2	/* x2 for stereo */
	int16 _pcmSamples[MAX_SAMPLES_PER_FRAME] __attribute__((aligned(64)));	// samples to output PCM data into
	byte _codecInBuffer[3072] __attribute__((aligned(64))); // the codec always needs alignment
	unsigned long _codecParams[65]__attribute__((aligned(64)));		// TODO: change to struct

	Common::DisposablePtr<Common::SeekableReadStream> _inStream;

	uint32 _pcmLength;		// how many pcm samples we have for this type of file (x2 this for stereo)

	uint _posInFrame;		// position in frame
	State _state;			// what state the stream is in

	Timestamp _length;
	uint32 _sampleRate;
	bool _stereo;

	mad_timer_t _totalTime;
	mad_stream _stream;		//
	mad_header _header;		// This is all we need from libmad

	static bool _decoderInit;	// has the decoder been initialized
	static bool _decoderFail;	// has the decoder failed to load

	enum {
		BUFFER_SIZE = 5 * 8192
	};

	// This buffer contains a slab of input data
	byte _buf[BUFFER_SIZE + MAD_BUFFER_GUARD];

	void decodeMP3Data();
	void readMP3DataIntoBuffer();

	static bool loadStartAudioModule(const char *modname, int partition);
	int initStream();
	void findValidHeader();
	void deinitStream();
	void updatePcmLength();

	// to init and uninit ME decoder
	static bool initDecoder();
	static bool stopDecoder();

	// ME functions for stream
	bool initStreamME();
	void releaseStreamME();

public:
	Mp3PspStream(Common::SeekableReadStream *inStream, DisposeAfterUse::Flag dispose);
	~Mp3PspStream();

	// This function avoids having to create streams when it's not possible
	static inline bool isOkToCreateStream() {
		if (_decoderFail)			// fatal failure
			return false;
		if (!_decoderInit)			// if we're not initialized
			if (!initDecoder())		// check if we failed init
				return false;
		return true;
	}

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const		{ return _state == MP3_STATE_EOS; }
	bool isStereo() const		{ return _stereo; }
	int getRate() const			{ return _sampleRate; }

	bool seek(const Timestamp &where);
	Timestamp getLength() const { return _length; }
};

} // End of namespace Audio

#endif // #ifndef SOUND_MP3_PSP_H
