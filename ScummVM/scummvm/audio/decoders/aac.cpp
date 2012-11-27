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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "audio/decoders/aac.h"

#ifdef USE_FAAD

#include "common/debug.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "audio/audiostream.h"
#include "audio/decoders/codec.h"
#include "audio/decoders/raw.h"

#include <neaacdec.h>

namespace Audio {

class AACDecoder : public Codec {
public:
	AACDecoder(Common::SeekableReadStream *extraData,
	          DisposeAfterUse::Flag disposeExtraData);
	~AACDecoder();

	AudioStream *decodeFrame(Common::SeekableReadStream &stream);

private:
	NeAACDecHandle _handle;
	byte _channels;
	unsigned long _rate;
};

AACDecoder::AACDecoder(Common::SeekableReadStream *extraData, DisposeAfterUse::Flag disposeExtraData) {
	// Open the library
	_handle = NeAACDecOpen();

	// Configure the library to our needs
	NeAACDecConfigurationPtr conf = NeAACDecGetCurrentConfiguration(_handle);
	conf->outputFormat = FAAD_FMT_16BIT; // We only support 16bit audio
	conf->downMatrix = 1;                // Convert from 5.1 to stereo if required
	NeAACDecSetConfiguration(_handle, conf);

	// Copy the extra data to a buffer
	extraData->seek(0);
	byte *extraDataBuf = new byte[extraData->size()];
	extraData->read(extraDataBuf, extraData->size());

	// Initialize with our extra data
	// NOTE: This code assumes the extra data is coming from an MPEG-4 file!
	int err = NeAACDecInit2(_handle, extraDataBuf, extraData->size(), &_rate, &_channels);
	delete[] extraDataBuf;

	if (err < 0)
		error("Could not initialize AAC decoder: %s", NeAACDecGetErrorMessage(err));

	if (disposeExtraData == DisposeAfterUse::YES)
		delete extraData;
}

AACDecoder::~AACDecoder() {
	NeAACDecClose(_handle);
}

AudioStream *AACDecoder::decodeFrame(Common::SeekableReadStream &stream) {
	// read everything into a buffer
	uint32 inBufferPos = 0;
	uint32 inBufferSize = stream.size();
	byte *inBuffer = new byte[inBufferSize];
	stream.read(inBuffer, inBufferSize);

	QueuingAudioStream *audioStream = makeQueuingAudioStream(_rate, _channels == 2);

	// Decode until we have enough samples (or there's no more left)
	while (inBufferPos < inBufferSize) {
		NeAACDecFrameInfo frameInfo;
		void *decodedSamples = NeAACDecDecode(_handle, &frameInfo, inBuffer + inBufferPos, inBufferSize - inBufferPos);

		if (frameInfo.error != 0)
			error("Failed to decode AAC frame: %s", NeAACDecGetErrorMessage(frameInfo.error));

		byte *buffer = (byte *)malloc(frameInfo.samples * 2);
		memcpy(buffer, decodedSamples, frameInfo.samples * 2);

		byte flags = FLAG_16BITS;

		if (_channels == 2)
			flags |= FLAG_STEREO;

#ifdef SCUMM_LITTLE_ENDIAN
		flags |= FLAG_LITTLE_ENDIAN;
#endif

		audioStream->queueBuffer(buffer, frameInfo.samples * 2, DisposeAfterUse::YES, flags);

		inBufferPos += frameInfo.bytesconsumed;
	}

	return audioStream;
}

// Factory function
Codec *makeAACDecoder(Common::SeekableReadStream *extraData, DisposeAfterUse::Flag disposeExtraData) {
	return new AACDecoder(extraData, disposeExtraData);
}

} // End of namespace Audio

#endif // #ifdef USE_FAAD
