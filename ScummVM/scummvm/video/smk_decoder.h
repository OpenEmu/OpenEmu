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

#ifndef VIDEO_SMK_PLAYER_H
#define VIDEO_SMK_PLAYER_H

#include "common/rational.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"
#include "audio/mixer.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Common {
class BitStream;
class SeekableReadStream;
}

namespace Video {

class BigHuffmanTree;

/**
 * Decoder for Smacker v2/v4 videos.
 *
 * Based on http://wiki.multimedia.cx/index.php?title=Smacker
 * and the FFmpeg Smacker decoder (libavcodec/smacker.c), revision 16143
 * http://svn.ffmpeg.org/ffmpeg/trunk/libavcodec/smacker.c?revision=16143&view=markup
 *
 * Video decoder used in engines:
 *  - agos
 *  - saga
 *  - scumm (he)
 *  - sword1
 *  - sword2
 *  - toon
 */
class SmackerDecoder : public VideoDecoder {
public:
	SmackerDecoder(Audio::Mixer::SoundType soundType = Audio::Mixer::kSFXSoundType);
	virtual ~SmackerDecoder();

	virtual bool loadStream(Common::SeekableReadStream *stream);
	void close();

	bool rewind();

protected:
	void readNextPacket();

	virtual void handleAudioTrack(byte track, uint32 chunkSize, uint32 unpackedSize);

	class SmackerVideoTrack : public FixedRateVideoTrack {
	public:
		SmackerVideoTrack(uint32 width, uint32 height, uint32 frameCount, const Common::Rational &frameRate, uint32 flags, uint32 signature);
		~SmackerVideoTrack();

		bool isRewindable() const { return true; }
		bool rewind() { _curFrame = -1; return true; }

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		const Graphics::Surface *decodeNextFrame() { return _surface; }
		const byte *getPalette() const { _dirtyPalette = false; return _palette; }
		bool hasDirtyPalette() const { return _dirtyPalette; }

		void readTrees(Common::BitStream &bs, uint32 mMapSize, uint32 mClrSize, uint32 fullSize, uint32 typeSize);
		void increaseCurFrame() { _curFrame++; }
		void decodeFrame(Common::BitStream &bs);
		void unpackPalette(Common::SeekableReadStream *stream);

	protected:
		Common::Rational getFrameRate() const { return _frameRate; }

		Graphics::Surface *_surface;

	private:
		Common::Rational _frameRate;
		uint32 _flags, _signature;

		byte _palette[3 * 256];
		mutable bool _dirtyPalette;

		int _curFrame;
		uint32 _frameCount;

		BigHuffmanTree *_MMapTree;
		BigHuffmanTree *_MClrTree;
		BigHuffmanTree *_FullTree;
		BigHuffmanTree *_TypeTree;

		// Possible runs of blocks
		static uint getBlockRun(int index) { return (index <= 58) ? index + 1 : 128 << (index - 59); }
	};

	virtual SmackerVideoTrack *createVideoTrack(uint32 width, uint32 height, uint32 frameCount, const Common::Rational &frameRate, uint32 flags, uint32 signature) const;

	Common::SeekableReadStream *_fileStream;

private:
	enum AudioCompression {
		kCompressionNone,
		kCompressionDPCM,
		kCompressionRDFT,
		kCompressionDCT
	};

	struct AudioInfo {
		AudioCompression compression;
		bool hasAudio;
		bool is16Bits;
		bool isStereo;
		uint32 sampleRate;
	};

	struct {
		uint32 signature;
		uint32 flags;
		uint32 audioSize[7];
		uint32 treesSize;
		uint32 mMapSize;
		uint32 mClrSize;
		uint32 fullSize;
		uint32 typeSize;
		AudioInfo audioInfo[7];
		uint32 dummy;
	} _header;

	class SmackerAudioTrack : public AudioTrack {
	public:
		SmackerAudioTrack(const AudioInfo &audioInfo, Audio::Mixer::SoundType soundType);
		~SmackerAudioTrack();

		bool isRewindable() const { return true; }
		bool rewind();

		Audio::Mixer::SoundType getSoundType() const { return _soundType; }

		void queueCompressedBuffer(byte *buffer, uint32 bufferSize, uint32 unpackedSize);
		void queuePCM(byte *buffer, uint32 bufferSize);

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::Mixer::SoundType _soundType;
		Audio::QueuingAudioStream *_audioStream;
		AudioInfo _audioInfo;
	};

	uint32 *_frameSizes;
	// The FrameTypes section of a Smacker file contains an array of bytes, where
	// the 8 bits of each byte describe the contents of the corresponding frame.
	// The highest 7 bits correspond to audio frames (bit 7 is track 6, bit 6 track 5
	// and so on), so there can be up to 7 different audio tracks. When the lowest bit
	// (bit 0) is set, it denotes a frame that contains a palette record
	byte *_frameTypes;

	uint32 _firstFrameStart;

	Audio::Mixer::SoundType _soundType;
};

} // End of namespace Video

#endif
