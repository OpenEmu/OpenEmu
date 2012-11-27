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

#ifndef SCI_VIDEO_ROBOT_DECODER_H
#define SCI_VIDEO_ROBOT_DECODER_H

#include "common/rational.h"
#include "common/rect.h"
#include "video/video_decoder.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Common {
class SeekableSubReadStreamEndian;
}

namespace Sci {

class RobotDecoder : public Video::VideoDecoder {
public:
	RobotDecoder(bool isBigEndian);
	virtual ~RobotDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	bool load(GuiResourceId id);
	void close();

	void setPos(uint16 x, uint16 y) { _pos = Common::Point(x, y); }
	Common::Point getPos() const { return _pos; }

protected:
	void readNextPacket();

private:
	class RobotVideoTrack : public FixedRateVideoTrack {
	public:
		RobotVideoTrack(int frameCount);
		~RobotVideoTrack();

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		const Graphics::Surface *decodeNextFrame() { return _surface; }
		const byte *getPalette() const { _dirtyPalette = false; return _palette; }
		bool hasDirtyPalette() const { return _dirtyPalette; }

		void readPaletteChunk(Common::SeekableSubReadStreamEndian *stream, uint16 chunkSize);
		void calculateVideoDimensions(Common::SeekableSubReadStreamEndian *stream, uint32 *frameSizes);
		Graphics::Surface *getSurface() { return _surface; }
		void increaseCurFrame() { _curFrame++; }

	protected:
		Common::Rational getFrameRate() const { return Common::Rational(60, 10); }

	private:
		int _frameCount;
		int _curFrame;
		byte _palette[256 * 3];
		mutable bool _dirtyPalette;
		Graphics::Surface *_surface;
	};

	class RobotAudioTrack : public AudioTrack {
	public:
		RobotAudioTrack();
		~RobotAudioTrack();

		Audio::Mixer::SoundType getSoundType() const { return Audio::Mixer::kMusicSoundType; }

		void queueBuffer(byte *buffer, int size);

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::QueuingAudioStream *_audioStream;
	};

	struct RobotHeader {
		// 6 bytes, identifier bytes
		uint16 version;
		uint16 audioChunkSize;
		uint16 audioSilenceSize;
		// 2 bytes, unknown
		uint16 frameCount;
		uint16 paletteDataSize;
		uint16 unkChunkDataSize;
		// 5 bytes, unknown
		byte hasSound;
		// 34 bytes, unknown
	} _header;

	void readHeaderChunk();
	void readFrameSizesChunk();

	Common::Point _pos;
	bool _isBigEndian;
	uint32 *_frameTotalSize;

	Common::SeekableSubReadStreamEndian *_fileStream;
};

} // End of namespace Sci

#endif
