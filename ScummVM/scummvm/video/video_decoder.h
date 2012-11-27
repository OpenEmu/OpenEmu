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

#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include "audio/mixer.h"
#include "audio/timestamp.h"	// TODO: Move this to common/ ?
#include "common/array.h"
#include "common/rational.h"
#include "common/str.h"
#include "graphics/pixelformat.h"

namespace Audio {
class AudioStream;
class RewindableAudioStream;
class SeekableAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Video {

/**
 * Generic interface for video decoder classes.
 */
class VideoDecoder {
public:
	VideoDecoder();
	virtual ~VideoDecoder() {}

	/////////////////////////////////////////
	// Opening/Closing a Video
	/////////////////////////////////////////

	/**
	 * Load a video from a file with the given name.
	 *
	 * A default implementation using Common::File and loadStream is provided.
	 *
	 * @param filename	the filename to load
	 * @return whether loading the file succeeded
	 */
	virtual bool loadFile(const Common::String &filename);

	/**
	 * Load a video from a generic read stream. The ownership of the
	 * stream object transfers to this VideoDecoder instance, which is
	 * hence also responsible for eventually deleting it.
	 *
	 * Implementations of this function are required to call addTrack()
	 * for each track in the video upon success.
	 *
	 * @param stream  the stream to load
	 * @return whether loading the stream succeeded
	 */
	virtual bool loadStream(Common::SeekableReadStream *stream) = 0;

	/**
	 * Close the active video stream and free any associated resources.
	 *
	 * All subclasses that need to close their own resources should still
	 * call the base class' close() function at the start of their function.
	 */
	virtual void close();

	/**
	 * Returns if a video stream is currently loaded or not.
	 */
	bool isVideoLoaded() const;


	/////////////////////////////////////////
	// Playback Control
	/////////////////////////////////////////

	/**
	 * Begin playback of the video at normal speed.
	 *
	 * @note This has no effect if the video is already playing.
	 */
	void start();

	/**
	 * Stop playback of the video.
	 *
	 * @note This has no effect if the video is not playing.
	 */
	void stop();

	/**
	 * Set the rate of playback.
	 *
	 * For instance, a rate of 0 would stop the video, while a rate of 1
	 * would play the video normally. Passing 2 to this function would
	 * play the video at twice the normal speed.
	 *
	 * @note This function does not work for non-0/1 rates on videos that
	 * have audio tracks.
	 *
	 * @todo This currently does not implement backwards playback, but will
	 * be implemented soon.
	 */
	void setRate(const Common::Rational &rate);

	/**
	 * Returns the rate at which the video is being played.
	 */
	Common::Rational getRate() const { return _playbackRate; }

	/**
	 * Returns if the video is currently playing or not.
	 *
	 * This is not equivalent to the inverse of endOfVideo(). A video keeps
	 * its playing status even after reaching the end of the video. This will
	 * return true after calling start() and will continue to return true
	 * until stop() (or close()) is called.
	 */
	bool isPlaying() const;

	/**
	 * Returns if a video is rewindable or not. The default implementation
	 * polls each track for rewindability.
	 */
	virtual bool isRewindable() const;

	/**
	 * Rewind a video to its beginning.
	 *
	 * If the video is playing, it will continue to play. The default
	 * implementation will rewind each track.
	 *
	 * @return true on success, false otherwise
	 */
	virtual bool rewind();

	/**
	 * Returns if a video is seekable or not. The default implementation
	 * polls each track for seekability.
	 */
	virtual bool isSeekable() const;

	/**
	 * Seek to a given time in the video.
	 *
	 * If the video is playing, it will continue to play. The default
	 * implementation will seek each track and must still be called
	 * from any other implementation.
	 *
	 * @param time The time to seek to
	 * @return true on success, false otherwise
	 */
	virtual bool seek(const Audio::Timestamp &time);

	/**
	 * Pause or resume the video. This should stop/resume any audio playback
	 * and other stuff. The initial pause time is kept so that any timing
	 * variables can be updated appropriately.
	 *
	 * This is a convenience method which automatically keeps track on how
	 * often the video has been paused, ensuring that after pausing an video
	 * e.g. twice, it has to be unpaused twice before actuallying resuming.
	 *
	 * @param pause		true to pause the video, false to resume it
	 */
	void pauseVideo(bool pause);

	/**
	 * Return whether the video is currently paused or not.
	 */
	bool isPaused() const { return _pauseLevel != 0; }

	/**
	 * Set the time for this video to end at. At this time in the video,
	 * all audio will stop and endOfVideo() will return true.
	 *
	 * While the setting is stored even if a video is not playing,
	 * endOfVideo() is only affected when the video is playing.
	 */
	void setEndTime(const Audio::Timestamp &endTime);

	/**
	 * Get the stop time of the video (if not set, zero)
	 */
	Audio::Timestamp getEndTime() const { return _endTime; }


	/////////////////////////////////////////
	// Playback Status
	/////////////////////////////////////////

	/**
	 * Returns if the video has reached the end or not.
	 * @return true if the video has finished playing or if none is loaded, false otherwise
	 */
	bool endOfVideo() const;

	/**
	 * Returns the current frame number of the video.
	 * @return the last frame decoded by the video
	 */
	int getCurFrame() const;

	/**
	 * Returns the number of frames in the video.
	 * @return the number of frames in the video
	 */
	uint32 getFrameCount() const;

	/**
	 * Returns the time position (in ms) of the current video.
	 * This can be based on the "wall clock" time as determined by
	 * OSystem::getMillis() or the current time of any audio track
	 * running in the video, and takes pausing the video into account.
	 *
	 * As such, it will differ from what multiplying getCurFrame() by
	 * some constant would yield, e.g. for a video with non-constant
	 * frame rate.
	 *
	 * Due to the nature of the timing, this value may not always be
	 * completely accurate (since our mixer does not have precise
	 * timing).
	 */
	uint32 getTime() const;


	/////////////////////////////////////////
	// Video Info
	/////////////////////////////////////////

	/**
	 * Returns the width of the video's frames.
	 *
	 * By default, this finds the largest width between all of the loaded
	 * tracks. However, a subclass may override this if it does any kind
	 * of post-processing on it.
	 *
	 * @return the width of the video's frames
	 */
	virtual uint16 getWidth() const;

	/**
	 * Returns the height of the video's frames.
	 *
	 * By default, this finds the largest height between all of the loaded
	 * tracks. However, a subclass may override this if it does any kind
	 * of post-processing on it.
	 *
	 * @return the height of the video's frames
	 */
	virtual uint16 getHeight() const;

	/**
	 * Get the pixel format of the currently loaded video.
	 */
	Graphics::PixelFormat getPixelFormat() const;

	/**
	 * Get the duration of the video.
	 *
	 * If the duration is unknown, this will return 0. If this is not
	 * overriden, it will take the length of the longest track.
	 */
	virtual Audio::Timestamp getDuration() const;


	/////////////////////////////////////////
	// Frame Decoding
	/////////////////////////////////////////

	/**
	 * Get the palette for the video in RGB format (if 8bpp or less).
	 *
	 * The palette's format is the same as PaletteManager's palette
	 * (interleaved RGB values).
	 */
	const byte *getPalette();

	/**
	 * Returns if the palette is dirty or not.
	 */
	bool hasDirtyPalette() const { return _dirtyPalette; }

	/**
	 * Return the time (in ms) until the next frame should be displayed.
	 */
	uint32 getTimeToNextFrame() const;

	/**
	 * Check whether a new frame should be decoded, i.e. because enough
	 * time has elapsed since the last frame was decoded.
	 * @return whether a new frame should be decoded or not
	 */
	bool needsUpdate() const;

	/**
	 * Decode the next frame into a surface and return the latter.
	 *
	 * A subclass may override this, but must still call this function. As an
	 * example, a subclass may do this to apply some global video scale to
	 * individual track's frame.
	 *
	 * Note that this will call readNextPacket() internally first before calling
	 * the next video track's decodeNextFrame() function.
	 *
	 * @return a surface containing the decoded frame, or 0
	 * @note Ownership of the returned surface stays with the VideoDecoder,
	 *       hence the caller must *not* free it.
	 * @note this may return 0, in which case the last frame should be kept on screen
	 */
	virtual const Graphics::Surface *decodeNextFrame();

	/**
	 * Set the default high color format for videos that convert from YUV.
	 *
	 * By default, VideoDecoder will attempt to use the screen format
	 * if it's >8bpp and use a 32bpp format when not.
	 *
	 * This must be set before calling loadStream().
	 */
	void setDefaultHighColorFormat(const Graphics::PixelFormat &format) { _defaultHighColorFormat = format; }


	/////////////////////////////////////////
	// Audio Control
	/////////////////////////////////////////

	/**
	 * Get the current volume at which the audio in the video is being played
	 * @return the current volume at which the audio in the video is being played
	 */
	byte getVolume() const { return _audioVolume; }

	/**
	 * Set the volume at which the audio in the video should be played.
	 * This setting remains until close() is called (which may be called
	 * from loadStream()). The default volume is the maximum.
	 *
	 * @param volume The volume at which to play the audio in the video
	 */
	void setVolume(byte volume);

	/**
	 * Get the current balance at which the audio in the video is being played
	 * @return the current balance at which the audio in the video is being played
	 */
	int8 getBalance() const { return _audioBalance; }

	/**
	 * Set the balance at which the audio in the video should be played.
	 * This setting remains until close() is called (which may be called
	 * from loadStream()). The default balance is 0.
	 *
	 * @param balance The balance at which to play the audio in the video
	 */
	void setBalance(int8 balance);

	/**
	 * Add an audio track from a stream file.
	 *
	 * This calls SeekableAudioStream::openStreamFile() internally
	 */
	bool addStreamFileTrack(const Common::String &baseName);

protected:
	/**
	 * An abstract representation of a track in a movie. Since tracks here are designed
	 * to work independently, they should not reference any other track(s) in the video.
	 */
	class Track {
	public:
		Track();
		virtual ~Track() {}

		/**
		 * The types of tracks this class can be.
		 */
		enum TrackType {
			kTrackTypeNone,
			kTrackTypeVideo,
			kTrackTypeAudio
		};

		/**
		 * Get the type of track.
		 */
		virtual TrackType getTrackType() const = 0;

		/**
		 * Return if the track has finished.
		 */
		virtual bool endOfTrack() const = 0;

		/**
		 * Return if the track is rewindable.
		 *
		 * If a video is seekable, it does not need to implement this
		 * for it to also be rewindable.
		 */
		virtual bool isRewindable() const;

		/**
		 * Rewind the video to the beginning.
		 *
		 * If a video is seekable, it does not need to implement this
		 * for it to also be rewindable.
		 *
		 * @return true on success, false otherwise.
		 */
		virtual bool rewind();

		/**
		 * Return if the track is seekable.
		 */
		virtual bool isSeekable() const { return false; }

		/**
		 * Seek to the given time.
		 * @param time The time to seek to, from the beginning of the video.
		 * @return true on success, false otherwise.
		 */
		virtual bool seek(const Audio::Timestamp &time) { return false; }

		/**
		 * Set the pause status of the track.
		 */
		void pause(bool shouldPause);

		/**
		 * Return if the track is paused.
		 */
		bool isPaused() const { return _paused; }

		/**
		 * Get the duration of the track (starting from this track's start time).
		 *
		 * By default, this returns 0 for unknown.
		 */
		virtual Audio::Timestamp getDuration() const;

	protected:
		/**
		 * Function called by pause() for subclasses to implement.
		 */
		virtual void pauseIntern(bool shouldPause) {}

	private:
		bool _paused;
	};

	/**
	 * An abstract representation of a video track.
	 */
	class VideoTrack : public Track {
	public:
		VideoTrack() {}
		virtual ~VideoTrack() {}

		TrackType getTrackType() const  { return kTrackTypeVideo; }
		virtual bool endOfTrack() const;

		/**
		 * Get the width of this track
		 */
		virtual uint16 getWidth() const = 0;

		/**
		 * Get the height of this track
		 */
		virtual uint16 getHeight() const = 0;

		/**
		 * Get the pixel format of this track
		 */
		virtual Graphics::PixelFormat getPixelFormat() const = 0;

		/**
		 * Get the current frame of this track
		 *
		 * @see VideoDecoder::getCurFrame()
		 */
		virtual int getCurFrame() const = 0;

		/**
		 * Get the frame count of this track
		 *
		 * @note If the frame count is unknown, return 0 (which is also
		 * the default implementation of the function). However, one must
		 * also implement endOfTrack() in that case.
		 */
		virtual int getFrameCount() const { return 0; }

		/**
		 * Get the start time of the next frame in milliseconds since
		 * the start of the video
		 */
		virtual uint32 getNextFrameStartTime() const = 0;

		/**
		 * Decode the next frame
		 */
		virtual const Graphics::Surface *decodeNextFrame() = 0;

		/**
		 * Get the palette currently in use by this track
		 */
		virtual const byte *getPalette() const { return 0; }

		/**
		 * Does the palette currently in use by this track need to be updated?
		 */
		virtual bool hasDirtyPalette() const { return false; }
	};

	/**
	 * A VideoTrack that is played at a constant rate.
	 *
	 * If the frame count is unknown, you must override endOfTrack().
	 */
	class FixedRateVideoTrack : public VideoTrack {
	public:
		FixedRateVideoTrack() {}
		virtual ~FixedRateVideoTrack() {}

		uint32 getNextFrameStartTime() const;
		virtual Audio::Timestamp getDuration() const;

	protected:
		/**
		 * Get the rate at which this track is played.
		 */
		virtual Common::Rational getFrameRate() const = 0;
	};

	/**
	 * An abstract representation of an audio track.
	 */
	class AudioTrack : public Track {
	public:
		AudioTrack() {}
		virtual ~AudioTrack() {}

		TrackType getTrackType() const { return kTrackTypeAudio; }

		virtual bool endOfTrack() const;

		/**
		 * Start playing this track
		 */
		void start();

		/**
		 * Stop playing this track
		 */
		void stop();

		void start(const Audio::Timestamp &limit);

		/**
		 * Get the volume for this track
		 */
		byte getVolume() const { return _volume; }

		/**
		 * Set the volume for this track
		 */
		void setVolume(byte volume);

		/**
		 * Get the balance for this track
		 */
		int8 getBalance() const { return _balance; }

		/**
		 * Set the balance for this track
		 */
		void setBalance(int8 balance);

		/**
		 * Get the time the AudioStream behind this track has been
		 * running
		 */
		uint32 getRunningTime() const;

		/**
		 * Get the sound type to be used when playing this audio track
		 */
		virtual Audio::Mixer::SoundType getSoundType() const { return Audio::Mixer::kPlainSoundType; }

	protected:
		void pauseIntern(bool shouldPause);

		/**
		 * Get the AudioStream that is the representation of this AudioTrack
		 */
		virtual Audio::AudioStream *getAudioStream() const = 0;

	private:
		Audio::SoundHandle _handle;
		byte _volume;
		int8 _balance;
	};

	/**
	 * An AudioTrack that implements isRewindable() and rewind() using
	 * RewindableAudioStream.
	 */
	class RewindableAudioTrack : public AudioTrack {
	public:
		RewindableAudioTrack() {}
		virtual ~RewindableAudioTrack() {}

		bool isRewindable() const { return true; }
		bool rewind();

	protected:
		Audio::AudioStream *getAudioStream() const;

		/**
		 * Get the RewindableAudioStream pointer to be used by this class
		 * for rewind() and getAudioStream()
		 */
		virtual Audio::RewindableAudioStream *getRewindableAudioStream() const = 0;
	};

	/**
	 * An AudioTrack that implements isSeekable() and seek() using
	 * SeekableAudioStream.
	 */
	class SeekableAudioTrack : public AudioTrack {
	public:
		SeekableAudioTrack() {}
		virtual ~SeekableAudioTrack() {}

		bool isSeekable() const { return true; }
		bool seek(const Audio::Timestamp &time);

		Audio::Timestamp getDuration() const;

	protected:
		Audio::AudioStream *getAudioStream() const;

		/**
		 * Get the SeekableAudioStream pointer to be used by this class
		 * for seek(), getDuration(), and getAudioStream()
		 */
		virtual Audio::SeekableAudioStream *getSeekableAudioStream() const = 0;
	};

	/**
	 * A SeekableAudioTrack that constructs its SeekableAudioStream using
	 * SeekableAudioStream::openStreamFile()
	 */
	class StreamFileAudioTrack : public SeekableAudioTrack {
	public:
		StreamFileAudioTrack();
		~StreamFileAudioTrack();

		/**
		 * Load the track from a file with the given base name.
		 *
		 * @return true on success, false otherwise
		 */
		bool loadFromFile(const Common::String &baseName);

	protected:
		Audio::SeekableAudioStream *_stream;
		Audio::SeekableAudioStream *getSeekableAudioStream() const { return _stream; }
	};

	/**
	 * Reset the pause start time (which should be called when seeking)
	 */
	void resetPauseStartTime();

	/**
	 * Decode enough data for the next frame and enough audio to last that long.
	 *
	 * This function is used by this class' decodeNextFrame() function. A subclass
	 * of a Track may decide to just have its decodeNextFrame() function read
	 * and decode the frame, but only if it is the only track in the video.
	 */
	virtual void readNextPacket() {}

	/**
	 * Define a track to be used by this class.
	 *
	 * The pointer is then owned by this base class.
	 */
	void addTrack(Track *track);

	/**
	 * Whether or not getTime() will sync with a playing audio track.
	 *
	 * A subclass can override this to disable this feature.
	 */
	virtual bool useAudioSync() const { return true; }

	/**
	 * Get the given track based on its index.
	 *
	 * @return A valid track pointer on success, 0 otherwise
	 */
	Track *getTrack(uint track);

	/**
	 * Get the given track based on its index
	 *
	 * @return A valid track pointer on success, 0 otherwise
	 */
	const Track *getTrack(uint track) const;

	/**
	 * Find out if all video tracks have finished
	 *
	 * This is useful if one wants to figure out if they need to buffer all
	 * remaining audio in a file.
	 */
	bool endOfVideoTracks() const;

	/**
	 * Get the default high color format
	 */
	Graphics::PixelFormat getDefaultHighColorFormat() const { return _defaultHighColorFormat; }

	/**
	 * Find the video track with the lowest start time for the next frame
	 */
	VideoTrack *findNextVideoTrack();

	/**
	 * Find the video track with the lowest start time for the next frame
	 */
	const VideoTrack *findNextVideoTrack() const;

	/**
	 * Typedef helpers for accessing tracks
	 */
	typedef Common::Array<Track *> TrackList;
	typedef TrackList::iterator TrackListIterator;

	/**
	 * Get the begin iterator of the tracks
	 */
	TrackListIterator getTrackListBegin() { return _tracks.begin(); }

	/**
	 * Get the end iterator of the tracks
	 */
	TrackListIterator getTrackListEnd() { return _tracks.end(); }

private:
	// Tracks owned by this VideoDecoder
	TrackList _tracks;

	// Current playback status
	bool _needsUpdate;
	Audio::Timestamp _lastTimeChange, _endTime;
	bool _endTimeSet;
	Common::Rational _playbackRate;

	// Palette settings from individual tracks
	mutable bool _dirtyPalette;
	const byte *_palette;

	// Default PixelFormat settings
	Graphics::PixelFormat _defaultHighColorFormat;

	// Internal helper functions
	void stopAudio();
	void startAudio();
	void startAudioLimit(const Audio::Timestamp &limit);
	bool hasFramesLeft() const;
	bool hasAudio() const;

	int32 _startTime;
	uint32 _pauseLevel;
	uint32 _pauseStartTime;
	byte _audioVolume;
	int8 _audioBalance;
};

} // End of namespace Video

#endif
