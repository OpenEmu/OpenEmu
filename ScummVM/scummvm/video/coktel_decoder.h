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

// Currently, only GOB and SCI32 games play IMDs and VMDs, so skip compiling if GOB and SCI32 is disabled.
#if !(defined(ENABLE_GOB) || defined(ENABLE_SCI32) || defined(DYNAMIC_MODULES))

// Do not compile the CoktelDecoder code

#else

#ifndef VIDEO_COKTELDECODER_H
#define VIDEO_COKTELDECODER_H

#include "common/list.h"
#include "common/array.h"
#include "common/rational.h"
#include "common/str.h"

#include "graphics/surface.h"

#include "video/video_decoder.h"

#include "audio/mixer.h"

namespace Common {
struct Rect;
class SeekableReadStream;
}
namespace Audio {
class QueuingAudioStream;
}

namespace Graphics {
struct PixelFormat;
}

namespace Video {

class Codec;

/**
 * Decoder for Coktel videos.
 *
 * Video decoder used in engines:
 *  - gob
 *  - sci
 */
class CoktelDecoder {
public:
	struct State {
		/** Set accordingly to what was done. */
		uint32 flags;
		/** The id of the spoken words. */
		uint16 speechId;

		State();
	};

	CoktelDecoder(Audio::Mixer *mixer,
			Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	virtual ~CoktelDecoder();

	/** Replace the current video stream with this identical one. */
	virtual bool reloadStream(Common::SeekableReadStream *stream) = 0;

	virtual bool seek(int32 frame, int whence = SEEK_SET, bool restart = false) = 0;

	/** Draw directly onto the specified video memory. */
	void setSurfaceMemory(void *mem, uint16 width, uint16 height, uint8 bpp);
	/** Reset the video memory. */
	void setSurfaceMemory();

	const Graphics::Surface *getSurface() const;

	/** Draw the video starting at this position within the video memory. */
	virtual void setXY(uint16 x, uint16 y);
	/** Draw the video at the default position. */
	void setXY();

	/** Override the video's frame rate. */
	void setFrameRate(Common::Rational frameRate);
	/** Get the video's frame rate. */
	Common::Rational getFrameRate() const;

	/** Get the video's default X position. */
	uint16 getDefaultX() const;
	/** Get the video's default Y position. */
	uint16 getDefaultY() const;

	/** Return a list of rectangles that changed in the last frame. */
	const Common::List<Common::Rect> &getDirtyRects() const;

	bool hasPalette() const;
	virtual bool hasVideo() const;

	bool hasSound()       const;
	bool isSoundEnabled() const;
	bool isSoundPlaying() const;

	void enableSound();
	void disableSound();
	void finishSound();

	virtual void colorModeChanged();

	/** Return the coordinates of the specified frame. */
	virtual bool getFrameCoords(int16 frame, int16 &x, int16 &y, int16 &width, int16 &height);

	/** Return whether that video has any embedded files. */
	virtual bool hasEmbeddedFiles() const;

	/** Return whether that embedded file exists. */
	virtual bool hasEmbeddedFile(const Common::String &fileName) const;

	/** Return that embedded file. */
	virtual Common::SeekableReadStream *getEmbeddedFile(const Common::String &fileName) const;

	/** Return the current subtitle index. */
	virtual int32 getSubtitleIndex() const;

	/** Is the video paletted or true color? */
	virtual bool isPaletted() const;

	/**
	 * Get the current frame
	 * @see VideoDecoder::getCurFrame()
	 */
	int getCurFrame() const;

	/**
	 * Decode the next frame
	 * @see VideoDecoder::decodeNextFrame()
	 */
	virtual const Graphics::Surface *decodeNextFrame() = 0;

	/**
	 * Load a video from a stream
	 * @see VideoDecoder::loadStream()
	 */
	virtual bool loadStream(Common::SeekableReadStream *stream) = 0;

	/** Has a video been loaded? */
	virtual bool isVideoLoaded() const = 0;

	/** Has the end of the video been reached? */
	bool endOfVideo() const;

	/** Close the video. */
	void close();

	/** Get the Mixer SoundType audio is being played with. */
	Audio::Mixer::SoundType getSoundType() const;
	/** Get the AudioStream for the audio. */
	Audio::AudioStream *getAudioStream() const;

	uint16 getWidth()  const;
	uint16 getHeight() const;
	virtual Graphics::PixelFormat getPixelFormat() const = 0;

	uint32 getFrameCount() const;

	const byte *getPalette();
	bool  hasDirtyPalette() const;

	uint32 getTimeToNextFrame() const;
	uint32 getStaticTimeToNextFrame() const;

	void pauseVideo(bool pause);

protected:
	enum SoundStage {
		kSoundNone     = 0, ///< No sound.
		kSoundLoaded   = 1, ///< Sound loaded.
		kSoundPlaying  = 2, ///< Sound is playing.
		kSoundFinished = 3  ///< No more new sound data.
	};

	enum Features {
		kFeaturesNone        = 0x0000,
		kFeaturesPalette     = 0x0008, ///< Has an own palette.
		kFeaturesDataSize    = 0x0020, ///< Suggests a data size.
		kFeaturesSound       = 0x0040, ///< Has sound.
		kFeaturesFrameCoords = 0x0080, ///< Has specific frame coordinates.
		kFeaturesStdCoords   = 0x0100, ///< Has general standard coordinates.
		kFeaturesFramePos    = 0x0200, ///< Has a frame positions table.
		kFeaturesVideo       = 0x0400  ///< Has video.
	};

	Audio::Mixer *_mixer;
	Audio::Mixer::SoundType _soundType;

	uint16 _width;
	uint16 _height;

	uint16 _x;
	uint16 _y;

	uint16 _defaultX;
	uint16 _defaultY;

	uint32 _features;

	 int32 _curFrame;
	uint32 _frameCount;

	uint32 _startTime;

	byte _palette[768];
	bool _paletteDirty;

	bool    _ownSurface;
	Graphics::Surface _surface;

	Common::List<Common::Rect> _dirtyRects;

	Common::Rational _frameRate;

	// Current sound state
	bool       _hasSound;
	bool       _soundEnabled;
	SoundStage _soundStage;

	Audio::QueuingAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;

	bool evaluateSeekFrame(int32 &frame, int whence) const;

	// Surface management
	bool hasSurface();
	void createSurface();
	void freeSurface();

	// Decompression
	uint32 deLZ77(byte *dest, const byte *src, uint32 srcSize, uint32 destSize);
	void deRLE(byte *&destPtr, const byte *&srcPtr, int16 destLen, int16 srcLen);

	// Block rendering
	void renderBlockWhole   (Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect);
	void renderBlockWhole4X (Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect);
	void renderBlockWhole2Y (Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect);
	void renderBlockSparse  (Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect);
	void renderBlockSparse2Y(Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect);
	void renderBlockRLE     (Graphics::Surface &dstSurf, const byte *src, Common::Rect &rect);

	// Sound helper functions
	inline void unsignedToSigned(byte *buffer, int length);

private:
	uint32 _pauseStartTime;
	bool   _isPaused;
};

class PreIMDDecoder : public CoktelDecoder {
public:
	PreIMDDecoder(uint16 width, uint16 height, Audio::Mixer *mixer,
			Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	~PreIMDDecoder();

	bool reloadStream(Common::SeekableReadStream *stream);

	bool seek(int32 frame, int whence = SEEK_SET, bool restart = false);

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

	bool isVideoLoaded() const;

	const Graphics::Surface *decodeNextFrame();

	Graphics::PixelFormat getPixelFormat() const;

private:
	Common::SeekableReadStream *_stream;

	// Buffer for processed frame data
	byte  *_videoBuffer;
	uint32 _videoBufferSize;

	// Frame decoding
	void processFrame();
	void renderFrame();
};

class IMDDecoder : public CoktelDecoder {
public:
	IMDDecoder(Audio::Mixer *mixer, Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	~IMDDecoder();

	bool reloadStream(Common::SeekableReadStream *stream);

	bool seek(int32 frame, int whence = SEEK_SET, bool restart = false);

	void setXY(uint16 x, uint16 y);

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

	bool isVideoLoaded() const;

	const Graphics::Surface *decodeNextFrame();

	Graphics::PixelFormat getPixelFormat() const;

private:
	enum Command {
		kCommandNextSound   = 0xFF00,
		kCommandStartSound  = 0xFF01,

		kCommandBreak       = 0xFFF0,
		kCommandBreakSkip0  = 0xFFF1,
		kCommandBreakSkip16 = 0xFFF2,
		kCommandBreakSkip32 = 0xFFF3,
		kCommandBreakMask   = 0xFFF8,

		kCommandPalette     = 0xFFF4,
		kCommandVideoData   = 0xFFFC,

		kCommandJump        = 0xFFFD
	};

	struct Coord {
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
	};

	Common::SeekableReadStream *_stream;

	byte _version;

	// Standard coordinates gives by the header
	int16 _stdX;
	int16 _stdY;
	int16 _stdWidth;
	int16 _stdHeight;

	uint32 _flags;

	uint32  _firstFramePos; ///< Position of the first frame's data within the stream.
	uint32 *_framePos;      ///< Positions of all frames.
	Coord  *_frameCoords;   ///< Coordinates of all frames.

	uint32 _videoBufferSize;   ///< Size of the video buffers.
	byte  *_videoBuffer[2];    ///< Video buffers.
	uint32 _videoBufferLen[2]; ///< Size of the video buffers filled.

	// Sound properties
	uint16 _soundFlags;
	 int16 _soundFreq;
	 int16 _soundSliceSize;
	 int16 _soundSlicesCount;

	// Loading helper functions
	bool loadCoordinates();
	bool loadFrameTableOffsets(uint32 &framePosPos, uint32 &frameCoordsPos);
	bool assessVideoProperties();
	bool assessAudioProperties();
	bool loadFrameTables(uint32 framePosPos, uint32 frameCoordsPos);

	// Frame decoding
	void processFrame();
	Common::Rect calcFrameCoords(uint32 frame);

	// Video
	bool renderFrame(Common::Rect &rect);

	// Sound
	void nextSoundSlice(bool hasNextCmd);
	bool initialSoundSlice(bool hasNextCmd);
	void emptySoundSlice(bool hasNextCmd);
};

class VMDDecoder : public CoktelDecoder {
friend class AdvancedVMDDecoder;

public:
	VMDDecoder(Audio::Mixer *mixer, Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	~VMDDecoder();

	bool reloadStream(Common::SeekableReadStream *stream);

	bool seek(int32 frame, int whence = SEEK_SET, bool restart = false);

	void setXY(uint16 x, uint16 y);

	void colorModeChanged();

	bool getFrameCoords(int16 frame, int16 &x, int16 &y, int16 &width, int16 &height);

	bool hasEmbeddedFiles() const;
	bool hasEmbeddedFile(const Common::String &fileName) const;
	Common::SeekableReadStream *getEmbeddedFile(const Common::String &fileName) const;

	int32 getSubtitleIndex() const;

	bool hasVideo() const;
	bool isPaletted() const;

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

	bool isVideoLoaded() const;

	const Graphics::Surface *decodeNextFrame();

	Graphics::PixelFormat getPixelFormat() const;

protected:
	void setAutoStartSound(bool autoStartSound);

private:
	enum PartType {
		kPartTypeSeparator = 0,
		kPartTypeAudio     = 1,
		kPartTypeVideo     = 2,
		kPartTypeFile      = 3,
		kPartType4         = 4,
		kPartTypeSubtitle  = 5
	};

	enum AudioFormat {
		kAudioFormat8bitRaw    = 0,
		kAudioFormat16bitDPCM  = 1,
		kAudioFormat16bitADPCM = 2
	};

	struct File {
		Common::String name;

		uint32 offset;
		uint32 size;
		uint32 realSize;

		File();
	};

	struct Part {
		PartType type;
		byte     field_1;
		byte     field_E;
		uint32   size;
		int16    left;
		int16    top;
		int16    right;
		int16    bottom;
		uint16   id;
		byte     flags;

		Part();
	};

	struct Frame {
		uint32 offset;
		Part  *parts;

		Frame();
		~Frame();
	};

	Common::SeekableReadStream *_stream;

	byte   _version;
	uint32 _flags;

	uint32 _frameInfoOffset;
	uint16 _partsPerFrame;
	Frame *_frames;

	Common::Array<File> _files;

	// Sound properties
	uint16 _soundFlags;
	int16  _soundFreq;
	int16  _soundSliceSize;
	int16  _soundSlicesCount;
	byte   _soundBytesPerSample;
	byte   _soundStereo; // (0: mono, 1: old-style stereo, 2: new-style stereo)
	uint32 _soundHeaderSize;
	uint32 _soundDataSize;
	uint32 _soundLastFilledFrame;
	AudioFormat _audioFormat;
	bool   _autoStartSound;

	// Video properties
	bool   _hasVideo;
	uint32 _videoCodec;
	byte   _blitMode;
	byte   _bytesPerPixel;

	uint32  _firstFramePos; ///< Position of the first frame's data within the stream.

	uint32 _videoBufferSize;   ///< Size of the video buffers.
	byte  *_videoBuffer[3];    ///< Video buffers.
	uint32 _videoBufferLen[3]; ///< Size of the video buffers filled.

	Graphics::Surface _8bppSurface[3]; ///< Fake 8bpp surfaces over the video buffers.

	bool _externalCodec;
	Codec *_codec;

	int32 _subtitle;

	bool _isPaletted;

	// Loading helper functions
	bool assessVideoProperties();
	bool assessAudioProperties();
	bool openExternalCodec();
	bool readFrameTable(int &numFiles);
	bool readFiles();

	// Frame decoding
	void processFrame();

	// Video
	bool renderFrame(Common::Rect &rect);
	bool getRenderRects(const Common::Rect &rect,
			Common::Rect &realRect, Common::Rect &fakeRect);
	void blit16(const Graphics::Surface &srcSurf, Common::Rect &rect);
	void blit24(const Graphics::Surface &srcSurf, Common::Rect &rect);

	// Sound
	void emptySoundSlice  (uint32 size);
	void filledSoundSlice (uint32 size);
	void filledSoundSlices(uint32 size, uint32 mask);

	uint8 evaluateMask(uint32 mask, bool *fillInfo, uint8 &max);

	// Generating audio streams
	Audio::AudioStream *create8bitRaw   (Common::SeekableReadStream *stream);
	Audio::AudioStream *create16bitDPCM (Common::SeekableReadStream *stream);
	Audio::AudioStream *create16bitADPCM(Common::SeekableReadStream *stream);

	bool getPartCoords(int16 frame, PartType type, int16 &x, int16 &y, int16 &width, int16 &height);
};

/**
 * A wrapper around the VMD code that implements the VideoDecoder
 * API.
 */
class AdvancedVMDDecoder : public VideoDecoder {
public:
	AdvancedVMDDecoder(Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	~AdvancedVMDDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();

private:
	class VMDVideoTrack : public FixedRateVideoTrack {
	public:
		VMDVideoTrack(VMDDecoder *decoder);

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const;
		int getFrameCount() const;
		const Graphics::Surface *decodeNextFrame();
		const byte *getPalette() const;
		bool hasDirtyPalette() const;

	protected:
		Common::Rational getFrameRate() const;

	private:
		VMDDecoder *_decoder;
	};

	class VMDAudioTrack : public AudioTrack {
	public:
		VMDAudioTrack(VMDDecoder *decoder);

		Audio::Mixer::SoundType getSoundType() const;

	protected:
		virtual Audio::AudioStream *getAudioStream() const;

	private:
		VMDDecoder *_decoder;
	};

	VMDDecoder    *_decoder;
	VMDVideoTrack *_videoTrack;
	VMDAudioTrack *_audioTrack;
};

} // End of namespace Video

#endif // VIDEO_COKTELDECODER_H

#endif // Engine and dynamic plugins guard
