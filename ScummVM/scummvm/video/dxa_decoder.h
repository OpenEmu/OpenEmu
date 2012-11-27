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

#ifndef VIDEO_DXA_DECODER_H
#define VIDEO_DXA_DECODER_H

#include "common/rational.h"
#include "graphics/pixelformat.h"
#include "video/video_decoder.h"

namespace Common {
class SeekableReadStream;
}

namespace Video {

/**
 * Decoder for DXA videos.
 *
 * Video decoder used in engines:
 *  - agos
 *  - sword1
 *  - sword2
 */
class DXADecoder : public VideoDecoder {
public:
	DXADecoder();
	virtual ~DXADecoder();

	bool loadStream(Common::SeekableReadStream *stream);

protected:
	/**
	 * Read the sound data out of the given DXA stream
	 */
	virtual void readSoundData(Common::SeekableReadStream *stream);

private:
	class DXAVideoTrack : public FixedRateVideoTrack {
	public:
		DXAVideoTrack(Common::SeekableReadStream *stream);
		~DXAVideoTrack();

		bool isRewindable() const { return true; }
		bool rewind();

		uint16 getWidth() const { return _width; }
		uint16 getHeight() const { return _height; }
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		const Graphics::Surface *decodeNextFrame();
		const byte *getPalette() const { _dirtyPalette = false; return _palette; }
		bool hasDirtyPalette() const { return _dirtyPalette; }

		void setFrameStartPos();

	protected:
		Common::Rational getFrameRate() const { return _frameRate; }

	private:
		void decodeZlib(byte *data, int size, int totalSize);
		void decode12(int size);
		void decode13(int size);

		enum ScaleMode {
			S_NONE,
			S_INTERLACED,
			S_DOUBLE
		};

		Common::SeekableReadStream *_fileStream;
		Graphics::Surface *_surface;

		byte *_frameBuffer1;
		byte *_frameBuffer2;
		byte *_scaledBuffer;
		byte *_inBuffer;
		uint32 _inBufferSize;
		byte *_decompBuffer;
		uint32 _decompBufferSize;
		uint16 _curHeight;
		uint32 _frameSize;
		ScaleMode _scaleMode;
		uint16 _width, _height;
		uint32 _frameRate;
		uint32 _frameCount;
		byte _palette[256 * 3];
		mutable bool _dirtyPalette;
		int _curFrame;
		uint32 _frameStartOffset;
	};
};

} // End of namespace Video

#endif
