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

#ifndef VIDEO_FLICDECODER_H
#define VIDEO_FLICDECODER_H

#include "video/video_decoder.h"
#include "common/list.h"
#include "common/rect.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
struct Surface;
}

namespace Video {

/**
 * Decoder for FLIC videos.
 *
 * Video decoder used in engines:
 *  - tucker
 */
class FlicDecoder : public VideoDecoder {
public:
	FlicDecoder();
	virtual ~FlicDecoder();

	bool loadStream(Common::SeekableReadStream *stream);

	const Common::List<Common::Rect> *getDirtyRects() const;
	void clearDirtyRects();
	void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);

private:
	class FlicVideoTrack : public VideoTrack {
	public:
		FlicVideoTrack(Common::SeekableReadStream *stream, uint16 frameCount, uint16 width, uint16 height);
		~FlicVideoTrack();

		bool endOfTrack() const;
		bool isRewindable() const { return true; }
		bool rewind();

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		uint32 getNextFrameStartTime() const { return _nextFrameStartTime; }
		const Graphics::Surface *decodeNextFrame();
		const byte *getPalette() const { _dirtyPalette = false; return _palette; }
		bool hasDirtyPalette() const { return _dirtyPalette; }

		const Common::List<Common::Rect> *getDirtyRects() const { return &_dirtyRects; }
		void clearDirtyRects() { _dirtyRects.clear(); }
		void copyDirtyRectsToBuffer(uint8 *dst, uint pitch);

	private:
		Common::SeekableReadStream *_fileStream;
		Graphics::Surface *_surface;

		int _curFrame;
		bool _atRingFrame;

		uint16 _offsetFrame1;
		uint16 _offsetFrame2;
		byte *_palette;
		mutable bool _dirtyPalette;

		uint32 _frameCount;
		uint32 _frameDelay, _startFrameDelay;
		uint32 _nextFrameStartTime;

		Common::List<Common::Rect> _dirtyRects;

		void decodeByteRun(uint8 *data);
		void decodeDeltaFLC(uint8 *data);
		void unpackPalette(uint8 *mem);
	};
};

} // End of namespace Video

#endif
