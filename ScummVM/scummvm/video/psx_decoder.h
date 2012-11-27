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

#ifndef VIDEO_PSX_DECODER_H
#define VIDEO_PSX_DECODER_H

#include "common/endian.h"
#include "common/rational.h"
#include "common/rect.h"
#include "common/str.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Common {
class BitStream;
class Huffman;
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
}

namespace Video {

/**
 * Decoder for PSX stream videos.
 * This currently implements the most basic PSX stream format that is
 * used by most games on the system. Special variants are not supported
 * at this time.
 *
 * Video decoder used in engines:
 *  - sword1 (psx)
 *  - sword2 (psx)
 */
class PSXStreamDecoder : public VideoDecoder {
public:
	// CD speed in sectors/second
	// Calling code should use these enum values instead of the constants
	enum CDSpeed {
		kCD1x = 75,
		kCD2x = 150
	};

	PSXStreamDecoder(CDSpeed speed, uint32 frameCount = 0);
	virtual ~PSXStreamDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

protected:
	void readNextPacket();
	bool useAudioSync() const;

private:
	class PSXVideoTrack : public VideoTrack {
	public:
		PSXVideoTrack(Common::SeekableReadStream *firstSector, CDSpeed speed, int frameCount);
		~PSXVideoTrack();

		uint16 getWidth() const { return _surface->w; }
		uint16 getHeight() const { return _surface->h; }
		Graphics::PixelFormat getPixelFormat() const { return _surface->format; }
		bool endOfTrack() const { return _endOfTrack; }
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		uint32 getNextFrameStartTime() const;
		const Graphics::Surface *decodeNextFrame();

		void setEndOfTrack() { _endOfTrack = true; }
		void decodeFrame(Common::SeekableReadStream *frame, uint sectorCount);

	private:
		Graphics::Surface *_surface;
		uint32 _frameCount;
		Audio::Timestamp _nextFrameStartTime;
		bool _endOfTrack;
		int _curFrame;

		enum PlaneType {
			kPlaneY = 0,
			kPlaneU = 1,
			kPlaneV = 2
		};

		uint16 _macroBlocksW, _macroBlocksH;
		byte *_yBuffer, *_cbBuffer, *_crBuffer;
		void decodeMacroBlock(Common::BitStream *bits, int mbX, int mbY, uint16 scale, uint16 version);
		void decodeBlock(Common::BitStream *bits, byte *block, int pitch, uint16 scale, uint16 version, PlaneType plane);

		void readAC(Common::BitStream *bits, int *block);
		Common::Huffman *_acHuffman;

		int readDC(Common::BitStream *bits, uint16 version, PlaneType plane);
		Common::Huffman *_dcHuffmanLuma, *_dcHuffmanChroma;
		int _lastDC[3];

		void dequantizeBlock(int *coefficients, float *block, uint16 scale);
		void idct(float *dequantData, float *result);
		int readSignedCoefficient(Common::BitStream *bits);
	};

	class PSXAudioTrack : public AudioTrack {
	public:
		PSXAudioTrack(Common::SeekableReadStream *sector);
		~PSXAudioTrack();

		bool endOfTrack() const;

		void setEndOfTrack() { _endOfTrack = true; }
		void queueAudioFromSector(Common::SeekableReadStream *sector);

	private:
		Audio::AudioStream *getAudioStream() const;

		Audio::QueuingAudioStream *_audStream;

		struct ADPCMStatus {
			int16 sample[2];
		} _adpcmStatus[2];

		bool _endOfTrack;
	};

	CDSpeed _speed;
	uint32 _frameCount;
	Common::SeekableReadStream *_stream;
	PSXVideoTrack *_videoTrack;
	PSXAudioTrack *_audioTrack;

	Common::SeekableReadStream *readSector();
};

} // End of namespace Video

#endif
