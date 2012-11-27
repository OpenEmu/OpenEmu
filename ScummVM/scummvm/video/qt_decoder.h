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

//
// Partially based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#ifndef VIDEO_QT_DECODER_H
#define VIDEO_QT_DECODER_H

#include "audio/decoders/quicktime_intern.h"
#include "common/scummsys.h"

#include "video/video_decoder.h"

namespace Common {
class Rational;
}

namespace Graphics {
struct PixelFormat;
}

namespace Video {

class Codec;

/**
 * Decoder for QuickTime videos.
 *
 * Video decoder used in engines:
 *  - mohawk
 *  - pegasus
 *  - sci
 */
class QuickTimeDecoder : public VideoDecoder, public Audio::QuickTimeAudioDecoder {
public:
	QuickTimeDecoder();
	virtual ~QuickTimeDecoder();

	bool loadFile(const Common::String &filename);
	bool loadStream(Common::SeekableReadStream *stream);
	void close();
	uint16 getWidth() const { return _width; }
	uint16 getHeight() const { return _height; }
	const Graphics::Surface *decodeNextFrame();
	Audio::Timestamp getDuration() const { return Audio::Timestamp(0, _duration, _timeScale); }

protected:
	Common::QuickTimeParser::SampleDesc *readSampleDesc(Common::QuickTimeParser::Track *track, uint32 format);

private:
	void init();

	void updateAudioBuffer();

	uint16 _width, _height;

	Graphics::Surface *_scaledSurface;
	void scaleSurface(const Graphics::Surface *src, Graphics::Surface *dst,
			const Common::Rational &scaleFactorX, const Common::Rational &scaleFactorY);

	class VideoSampleDesc : public Common::QuickTimeParser::SampleDesc {
	public:
		VideoSampleDesc(Common::QuickTimeParser::Track *parentTrack, uint32 codecTag);
		~VideoSampleDesc();

		void initCodec();

		// TODO: Make private in the long run
		uint16 _bitsPerSample;
		char _codecName[32];
		uint16 _colorTableId;
		byte *_palette;
		Codec *_videoCodec;
	};

	// The AudioTrackHandler is currently just a wrapper around some
	// QuickTimeDecoder functions.
	class AudioTrackHandler : public SeekableAudioTrack {
	public:
		AudioTrackHandler(QuickTimeDecoder *decoder, QuickTimeAudioTrack *audioTrack);

		void updateBuffer();

	protected:
		Audio::SeekableAudioStream *getSeekableAudioStream() const;

	private:
		QuickTimeDecoder *_decoder;
		QuickTimeAudioTrack *_audioTrack;
	};

	// The VideoTrackHandler is the bridge between the time of playback
	// and the media for the given track. It calculates when to start
	// tracks and at what rate to play the media using the edit list.
	class VideoTrackHandler : public VideoTrack {
	public:
		VideoTrackHandler(QuickTimeDecoder *decoder, Common::QuickTimeParser::Track *parent);
		~VideoTrackHandler();

		bool endOfTrack() const;
		bool isSeekable() const { return true; }
		bool seek(const Audio::Timestamp &time);
		Audio::Timestamp getDuration() const;

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const;
		uint32 getNextFrameStartTime() const;
		const Graphics::Surface *decodeNextFrame();
		const byte *getPalette() const { _dirtyPalette = false; return _curPalette; }
		bool hasDirtyPalette() const { return _curPalette; }

		Common::Rational getScaledWidth() const;
		Common::Rational getScaledHeight() const;

	private:
		QuickTimeDecoder *_decoder;
		Common::QuickTimeParser::Track *_parent;
		uint32 _curEdit;
		int32 _curFrame;
		uint32 _nextFrameStartTime;
		Graphics::Surface *_scaledSurface;
		bool _holdNextFrameStartTime;
		int32 _durationOverride;
		const byte *_curPalette;
		mutable bool _dirtyPalette;

		Common::SeekableReadStream *getNextFramePacket(uint32 &descId);
		uint32 getFrameDuration();
		uint32 findKeyFrame(uint32 frame) const;
		void enterNewEditList(bool bufferFrames);
		const Graphics::Surface *bufferNextFrame();
		uint32 getRateAdjustedFrameTime() const;
		uint32 getCurEditTimeOffset() const;
		uint32 getCurEditTrackDuration() const;
	};
};

} // End of namespace Video

#endif
