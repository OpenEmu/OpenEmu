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

/**
 * Internal interfaces to the ADPCM decoders.
 *
 * These can be used to make custom ADPCM decoder subclasses,
 * or to at least share some common data tables between various
 * ADPCM decoder implementations.
 */

#ifndef AUDIO_ADPCM_INTERN_H
#define AUDIO_ADPCM_INTERN_H

#include "audio/audiostream.h"
#include "common/endian.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Audio {

class ADPCMStream : public RewindableAudioStream {
protected:
	Common::DisposablePtr<Common::SeekableReadStream> _stream;
	int32 _startpos;
	const int32 _endpos;
	const int _channels;
	const uint32 _blockAlign;
	uint32 _blockPos[2];
	const int _rate;

	struct ADPCMStatus {
		// OKI/IMA
		struct {
			int32 last;
			int32 stepIndex;
		} ima_ch[2];
	} _status;

	virtual void reset();

public:
	ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign);

	virtual bool endOfData() const { return (_stream->eos() || _stream->pos() >= _endpos); }
	virtual bool isStereo() const { return _channels == 2; }
	virtual int getRate() const { return _rate; }

	virtual bool rewind();

	/**
	 * This table is used by some ADPCM variants (IMA and OKI) to adjust the
	 * step for use on the next sample.
	 * The first 8 entries are identical to the second 8 entries. Hence, we
	 * could half the table in size. But since the lookup index is always a
	 * 4-bit nibble, it is more efficient to just keep it as it is.
	 */
	static const int16 _stepAdjustTable[16];
};

class Oki_ADPCMStream : public ADPCMStream {
public:
	Oki_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) { _decodedSampleCount = 0; }

	virtual bool endOfData() const { return (_stream->eos() || _stream->pos() >= _endpos) && (_decodedSampleCount == 0); }

	virtual int readBuffer(int16 *buffer, const int numSamples);

protected:
	int16 decodeOKI(byte);

private:
	uint8 _decodedSampleCount;
	int16 _decodedSamples[2];
};

class Ima_ADPCMStream : public ADPCMStream {
protected:
	int16 decodeIMA(byte code, int channel = 0); // Default to using the left channel/using one channel

public:
	Ima_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {}

	/**
	 * This table is used by decodeIMA.
	 */
	static const int16 _imaTable[89];
};

class DVI_ADPCMStream : public Ima_ADPCMStream {
public:
	DVI_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Ima_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) { _decodedSampleCount = 0; }

	virtual bool endOfData() const { return (_stream->eos() || _stream->pos() >= _endpos) && (_decodedSampleCount == 0); }

	virtual int readBuffer(int16 *buffer, const int numSamples);

private:
	uint8 _decodedSampleCount;
	int16 _decodedSamples[2];
};

class Apple_ADPCMStream : public Ima_ADPCMStream {
protected:
	// Apple QuickTime IMA ADPCM
	int32 _streamPos[2];
	int16 _buffer[2][2];
	uint8 _chunkPos[2];

	void reset() {
		Ima_ADPCMStream::reset();
		_chunkPos[0] = 0;
		_chunkPos[1] = 0;
		_streamPos[0] = 0;
		_streamPos[1] = _blockAlign;
	}

public:
	Apple_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Ima_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {
		_chunkPos[0] = 0;
		_chunkPos[1] = 0;
		_streamPos[0] = 0;
		_streamPos[1] = _blockAlign;
	}

	virtual int readBuffer(int16 *buffer, const int numSamples);

};

class MSIma_ADPCMStream : public Ima_ADPCMStream {
public:
	MSIma_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Ima_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {

		if (blockAlign == 0)
			error("MSIma_ADPCMStream(): blockAlign isn't specified");

		if (blockAlign % (_channels * 4))
			error("MSIma_ADPCMStream(): invalid blockAlign");

		_samplesLeft[0] = 0;
		_samplesLeft[1] = 0;
	}

	virtual int readBuffer(int16 *buffer, const int numSamples);

	void reset() {
		Ima_ADPCMStream::reset();
		_samplesLeft[0] = 0;
		_samplesLeft[1] = 0;
	}

private:
	int16 _buffer[2][8];
	int _samplesLeft[2];
};

class MS_ADPCMStream : public ADPCMStream {
protected:
	struct ADPCMChannelStatus {
		byte predictor;
		int16 delta;
		int16 coeff1;
		int16 coeff2;
		int16 sample1;
		int16 sample2;
	};

	struct {
		// MS ADPCM
		ADPCMChannelStatus ch[2];
	} _status;

	void reset() {
		ADPCMStream::reset();
		memset(&_status, 0, sizeof(_status));
	}

public:
	MS_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {
		if (blockAlign == 0)
			error("MS_ADPCMStream(): blockAlign isn't specified for MS ADPCM");
		memset(&_status, 0, sizeof(_status));
	}

	virtual int readBuffer(int16 *buffer, const int numSamples);

protected:
	int16 decodeMS(ADPCMChannelStatus *c, byte);
};

// Duck DK3 IMA ADPCM Decoder
// Based on FFmpeg's decoder and http://wiki.multimedia.cx/index.php?title=Duck_DK3_IMA_ADPCM

class DK3_ADPCMStream : public Ima_ADPCMStream {
protected:

	void reset() {
		Ima_ADPCMStream::reset();
		_topNibble = false;
	}

public:
	DK3_ADPCMStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse, uint32 size, int rate, int channels, uint32 blockAlign)
		: Ima_ADPCMStream(stream, disposeAfterUse, size, rate, channels, blockAlign) {

		// DK3 only works as a stereo stream
		assert(channels == 2);
		_topNibble = false;
	}

	virtual int readBuffer(int16 *buffer, const int numSamples);

private:
	byte _nibble, _lastByte;
	bool _topNibble;
};

} // End of namespace Audio

#endif
