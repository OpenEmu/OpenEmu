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

#ifndef VIDEO_AVI_PLAYER_H
#define VIDEO_AVI_PLAYER_H

#include "common/endian.h"
#include "common/rational.h"
#include "common/rect.h"
#include "common/str.h"

#include "video/video_decoder.h"
#include "audio/mixer.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
}

namespace Video {

class Codec;

/**
 * Decoder for AVI videos.
 *
 * Video decoder used in engines:
 *  - sci
 */
class AVIDecoder : public VideoDecoder {
public:
	AVIDecoder(Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	virtual ~AVIDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();
	uint16 getWidth() const { return _header.width; }
	uint16 getHeight() const { return _header.height; }

protected:
	 void readNextPacket();

private:
	struct BitmapInfoHeader {
		uint32 size;
		uint32 width;
		uint32 height;
		uint16 planes;
		uint16 bitCount;
		uint32 compression;
		uint32 sizeImage;
		uint32 xPelsPerMeter;
		uint32 yPelsPerMeter;
		uint32 clrUsed;
		uint32 clrImportant;
	};

	struct WaveFormat {
		uint16 tag;
		uint16 channels;
		uint32 samplesPerSec;
		uint32 avgBytesPerSec;
		uint16 blockAlign;
	};

	struct PCMWaveFormat : public WaveFormat {
		uint16 size;
	};

	struct WaveFormatEX : public WaveFormat {
		uint16 bitsPerSample;
		uint16 size;
	};

	struct OldIndex {
		uint32 size;
		struct Index {
			uint32 id;
			uint32 flags;
			uint32 offset;
			uint32 size;
		} *indices;
	};

	// Index Flags
	enum IndexFlags {
		AVIIF_INDEX = 0x10
	};

	struct AVIHeader {
		uint32 size;
		uint32 microSecondsPerFrame;
		uint32 maxBytesPerSecond;
		uint32 padding;
		uint32 flags;
		uint32 totalFrames;
		uint32 initialFrames;
		uint32 streams;
		uint32 bufferSize;
		uint32 width;
		uint32 height;
	};

	// Flags from the AVIHeader
	enum AVIFlags {
		AVIF_HASINDEX = 0x00000010,
		AVIF_MUSTUSEINDEX = 0x00000020,
		AVIF_ISINTERLEAVED = 0x00000100,
		AVIF_TRUSTCKTYPE = 0x00000800,
		AVIF_WASCAPTUREFILE = 0x00010000,
		AVIF_WASCOPYRIGHTED = 0x00020000
	};

	struct AVIStreamHeader {
		uint32 size;
		uint32 streamType;
		uint32 streamHandler;
		uint32 flags;
		uint16 priority;
		uint16 language;
		uint32 initialFrames;
		uint32 scale;
		uint32 rate;
		uint32 start;
		uint32 length;
		uint32 bufferSize;
		uint32 quality;
		uint32 sampleSize;
		Common::Rect frame;
	};

	class AVIVideoTrack : public FixedRateVideoTrack {
	public:
		AVIVideoTrack(int frameCount, const AVIStreamHeader &streamHeader, const BitmapInfoHeader &bitmapInfoHeader);
		~AVIVideoTrack();

		void decodeFrame(Common::SeekableReadStream *stream);

		uint16 getWidth() const { return _bmInfo.width; }
		uint16 getHeight() const { return _bmInfo.height; }
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		const Graphics::Surface *decodeNextFrame() { return _lastFrame; }
		const byte *getPalette() const { _dirtyPalette = false; return _palette; }
		bool hasDirtyPalette() const { return _dirtyPalette; }
		void markPaletteDirty() { _dirtyPalette = true; }

	protected:
		Common::Rational getFrameRate() const { return Common::Rational(_vidsHeader.rate, _vidsHeader.scale); }

	private:
		AVIStreamHeader _vidsHeader;
		BitmapInfoHeader _bmInfo;
		byte _palette[3 * 256];
		mutable bool _dirtyPalette;
		int _frameCount, _curFrame;

		Codec *_videoCodec;
		const Graphics::Surface *_lastFrame;
		Codec *createCodec();
	};

	class AVIAudioTrack : public AudioTrack {
	public:
		AVIAudioTrack(const AVIStreamHeader &streamHeader, const PCMWaveFormat &waveFormat, Audio::Mixer::SoundType soundType);
		~AVIAudioTrack();

		void queueSound(Common::SeekableReadStream *stream);
		Audio::Mixer::SoundType getSoundType() const { return _soundType; }

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		// Audio Codecs
		enum {
			kWaveFormatNone = 0,
			kWaveFormatPCM = 1,
			kWaveFormatDK3 = 98
		};

		AVIStreamHeader _audsHeader;
		PCMWaveFormat _wvInfo;
		Audio::Mixer::SoundType _soundType;
		Audio::QueuingAudioStream *_audStream;
		Audio::QueuingAudioStream *createAudioStream();
	};

	OldIndex _ixInfo;
	AVIHeader _header;

	Common::SeekableReadStream *_fileStream;
	bool _decodedHeader;

	Audio::Mixer::SoundType _soundType;

	void runHandle(uint32 tag);
	void handleList();
	void handleStreamHeader();
};

} // End of namespace Video

#endif
