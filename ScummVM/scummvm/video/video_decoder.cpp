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

#include "video/video_decoder.h"

#include "audio/audiostream.h"
#include "audio/mixer.h" // for kMaxChannelVolume

#include "common/rational.h"
#include "common/file.h"
#include "common/system.h"

#include "graphics/palette.h"

namespace Video {

VideoDecoder::VideoDecoder() {
	_startTime = 0;
	_dirtyPalette = false;
	_palette = 0;
	_playbackRate = 0;
	_audioVolume = Audio::Mixer::kMaxChannelVolume;
	_audioBalance = 0;
	_pauseLevel = 0;
	_needsUpdate = false;
	_lastTimeChange = 0;
	_endTime = 0;
	_endTimeSet = false;

	// Find the best format for output
	_defaultHighColorFormat = g_system->getScreenFormat();

	if (_defaultHighColorFormat.bytesPerPixel == 1)
		_defaultHighColorFormat = Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0);
}

void VideoDecoder::close() {
	if (isPlaying())
		stop();

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		delete *it;

	_tracks.clear();
	_dirtyPalette = false;
	_palette = 0;
	_startTime = 0;
	_audioVolume = Audio::Mixer::kMaxChannelVolume;
	_audioBalance = 0;
	_pauseLevel = 0;
	_needsUpdate = false;
	_lastTimeChange = 0;
	_endTime = 0;
	_endTimeSet = false;
}

bool VideoDecoder::loadFile(const Common::String &filename) {
	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		delete file;
		return false;
	}

	return loadStream(file);
}

bool VideoDecoder::needsUpdate() const {
	return hasFramesLeft() && getTimeToNextFrame() == 0;
}

void VideoDecoder::pauseVideo(bool pause) {
	if (pause) {
		_pauseLevel++;

	// We can't go negative
	} else if (_pauseLevel) {
		_pauseLevel--;

	// Do nothing
	} else {
		return;
	}

	if (_pauseLevel == 1 && pause) {
		_pauseStartTime = g_system->getMillis(); // Store the starting time from pausing to keep it for later

		for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
			(*it)->pause(true);
	} else if (_pauseLevel == 0) {
		for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
			(*it)->pause(false);

		_startTime += (g_system->getMillis() - _pauseStartTime);
	}
}

void VideoDecoder::resetPauseStartTime() {
	if (isPaused())
		_pauseStartTime = g_system->getMillis();
}

void VideoDecoder::setVolume(byte volume) {
	_audioVolume = volume;

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->setVolume(_audioVolume);
}

void VideoDecoder::setBalance(int8 balance) {
	_audioBalance = balance;

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->setBalance(_audioBalance);
}

bool VideoDecoder::isVideoLoaded() const {
	return !_tracks.empty();
}

uint16 VideoDecoder::getWidth() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			return ((VideoTrack *)*it)->getWidth();

	return 0;
}

uint16 VideoDecoder::getHeight() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			return ((VideoTrack *)*it)->getHeight();

	return 0;
}

Graphics::PixelFormat VideoDecoder::getPixelFormat() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			return ((VideoTrack *)*it)->getPixelFormat();

	return Graphics::PixelFormat();
}

const Graphics::Surface *VideoDecoder::decodeNextFrame() {
	_needsUpdate = false;

	readNextPacket();
	VideoTrack *track = findNextVideoTrack();

	if (!track)
		return 0;

	const Graphics::Surface *frame = track->decodeNextFrame();

	if (track->hasDirtyPalette()) {
		_palette = track->getPalette();
		_dirtyPalette = true;
	}

	return frame;
}

const byte *VideoDecoder::getPalette() {
	_dirtyPalette = false;
	return _palette;
}

int VideoDecoder::getCurFrame() const {
	int32 frame = -1;

	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			frame += ((VideoTrack *)*it)->getCurFrame() + 1;

	return frame;
}

uint32 VideoDecoder::getFrameCount() const {
	int count = 0;

	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo)
			count += ((VideoTrack *)*it)->getFrameCount();

	return count;
}

uint32 VideoDecoder::getTime() const {
	if (!isPlaying())
		return _lastTimeChange.msecs();

	if (isPaused())
		return (_playbackRate * (_pauseStartTime - _startTime)).toInt();

	if (useAudioSync()) {
		for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++) {
			if ((*it)->getTrackType() == Track::kTrackTypeAudio && !(*it)->endOfTrack()) {
				uint32 time = ((const AudioTrack *)*it)->getRunningTime();

				if (time != 0)
					return time + _lastTimeChange.msecs();
			}
		}
	}

	return (_playbackRate * (g_system->getMillis() - _startTime)).toInt();
}

uint32 VideoDecoder::getTimeToNextFrame() const {
	if (endOfVideo() || _needsUpdate)
		return 0;

	const VideoTrack *track = findNextVideoTrack();

	if (!track)
		return 0;

	uint32 elapsedTime = getTime();
	uint32 nextFrameStartTime = track->getNextFrameStartTime();

	if (nextFrameStartTime <= elapsedTime)
		return 0;

	return nextFrameStartTime - elapsedTime;
}

bool VideoDecoder::endOfVideo() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if (!(*it)->endOfTrack() && (!isPlaying() || (*it)->getTrackType() != Track::kTrackTypeVideo || !_endTimeSet || ((VideoTrack *)*it)->getNextFrameStartTime() < (uint)_endTime.msecs()))
			return false;

	return true;
}

bool VideoDecoder::isRewindable() const {
	if (!isVideoLoaded())
		return false;

	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if (!(*it)->isRewindable())
			return false;

	return true;
}

bool VideoDecoder::rewind() {
	if (!isRewindable())
		return false;

	// Stop all tracks so they can be rewound
	if (isPlaying())
		stopAudio();

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if (!(*it)->rewind())
			return false;

	// Now that we've rewound, start all tracks again
	if (isPlaying())
		startAudio();

	_lastTimeChange = 0;
	_startTime = g_system->getMillis();
	resetPauseStartTime();
	return true;
}

bool VideoDecoder::isSeekable() const {
	if (!isVideoLoaded())
		return false;

	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if (!(*it)->isSeekable())
			return false;

	return true;
}

bool VideoDecoder::seek(const Audio::Timestamp &time) {
	if (!isSeekable())
		return false;

	// Stop all tracks so they can be seeked
	if (isPlaying())
		stopAudio();

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if (!(*it)->seek(time))
			return false;

	_lastTimeChange = time;

	// Now that we've seeked, start all tracks again
	// Also reset our start time
	if (isPlaying()) {
		startAudio();
		_startTime = g_system->getMillis() - time.msecs();
	}

	resetPauseStartTime();
	_needsUpdate = true;
	return true;
}

void VideoDecoder::start() {
	if (!isPlaying())
		setRate(1);
}

void VideoDecoder::stop() {
	if (!isPlaying())
		return;

	// Stop audio here so we don't have it affect getTime()
	stopAudio();

	// Keep the time marked down in case we start up again
	// We do this before _playbackRate is set so we don't get
	// _lastTimeChange returned, but before _pauseLevel is
	// reset.
	_lastTimeChange = getTime();

	_playbackRate = 0;
	_startTime = 0;
	_palette = 0;
	_dirtyPalette = false;
	_needsUpdate = false;

	// Also reset the pause state.
	_pauseLevel = 0;

	// Reset the pause state of the tracks too
	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		(*it)->pause(false);
}

void VideoDecoder::setRate(const Common::Rational &rate) {
	if (!isVideoLoaded() || _playbackRate == rate)
		return;

	if (rate == 0) {
		stop();
		return;
	} else if (rate != 1 && hasAudio()) {
		warning("Cannot set custom rate in videos with audio");
		return;
	}

	Common::Rational targetRate = rate;

	if (rate < 0) {
		// TODO: Implement support for this
		warning("Cannot set custom rate to backwards");
		targetRate = 1;

		if (_playbackRate == targetRate)
			return;
	}

	if (_playbackRate != 0)
		_lastTimeChange = getTime();

	_playbackRate = targetRate;
	_startTime = g_system->getMillis();

	// Adjust start time if we've seeked to something besides zero time
	if (_lastTimeChange.totalNumberOfFrames() != 0)
		_startTime -= (_lastTimeChange.msecs() / _playbackRate).toInt();

	startAudio();
}

bool VideoDecoder::isPlaying() const {
	return _playbackRate != 0;
}

Audio::Timestamp VideoDecoder::getDuration() const {
	Audio::Timestamp maxDuration(0, 1000);

	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		Audio::Timestamp duration = (*it)->getDuration();

		if (duration > maxDuration)
			maxDuration = duration;
	}

	return maxDuration;
}

VideoDecoder::Track::Track() {
	_paused = false;
}

bool VideoDecoder::Track::isRewindable() const {
	return isSeekable();
}

bool VideoDecoder::Track::rewind() {
	return seek(Audio::Timestamp(0, 1000));
}

void VideoDecoder::Track::pause(bool shouldPause) {
	_paused = shouldPause;
	pauseIntern(shouldPause);
}

Audio::Timestamp VideoDecoder::Track::getDuration() const {
	return Audio::Timestamp(0, 1000);
}

bool VideoDecoder::VideoTrack::endOfTrack() const {
	return getCurFrame() >= (getFrameCount() - 1);
}

uint32 VideoDecoder::FixedRateVideoTrack::getNextFrameStartTime() const {
	if (endOfTrack() || getCurFrame() < 0)
		return 0;

	Common::Rational time = (getCurFrame() + 1) * 1000;
	time /= getFrameRate();
	return time.toInt();
}

Audio::Timestamp VideoDecoder::FixedRateVideoTrack::getDuration() const {
	// Since Audio::Timestamp doesn't support a fractional frame rate, we're currently
	// just converting to milliseconds.
	Common::Rational time = getFrameCount() * 1000;
	time /= getFrameRate();
	return time.toInt();
}

bool VideoDecoder::AudioTrack::endOfTrack() const {
	Audio::AudioStream *stream = getAudioStream();
	return !stream || !g_system->getMixer()->isSoundHandleActive(_handle) || stream->endOfData();
}

void VideoDecoder::AudioTrack::setVolume(byte volume) {
	_volume = volume;

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->setChannelVolume(_handle, _volume);
}

void VideoDecoder::AudioTrack::setBalance(int8 balance) {
	_balance = balance;

	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->setChannelBalance(_handle, _balance);
}

void VideoDecoder::AudioTrack::start() {
	stop();

	Audio::AudioStream *stream = getAudioStream();
	assert(stream);

	g_system->getMixer()->playStream(getSoundType(), &_handle, stream, -1, getVolume(), getBalance(), DisposeAfterUse::NO);

	// Pause the audio again if we're still paused
	if (isPaused())
		g_system->getMixer()->pauseHandle(_handle, true);
}

void VideoDecoder::AudioTrack::stop() {
	g_system->getMixer()->stopHandle(_handle);
}

void VideoDecoder::AudioTrack::start(const Audio::Timestamp &limit) {
	stop();

	Audio::AudioStream *stream = getAudioStream();
	assert(stream);

	stream = Audio::makeLimitingAudioStream(stream, limit, DisposeAfterUse::NO);

	g_system->getMixer()->playStream(getSoundType(), &_handle, stream, -1, getVolume(), getBalance(), DisposeAfterUse::YES);

	// Pause the audio again if we're still paused
	if (isPaused())
		g_system->getMixer()->pauseHandle(_handle, true);
}

uint32 VideoDecoder::AudioTrack::getRunningTime() const {
	if (g_system->getMixer()->isSoundHandleActive(_handle))
		return g_system->getMixer()->getSoundElapsedTime(_handle);

	return 0;
}

void VideoDecoder::AudioTrack::pauseIntern(bool shouldPause) {
	if (g_system->getMixer()->isSoundHandleActive(_handle))
		g_system->getMixer()->pauseHandle(_handle, shouldPause);
}

Audio::AudioStream *VideoDecoder::RewindableAudioTrack::getAudioStream() const {
	return getRewindableAudioStream();
}

bool VideoDecoder::RewindableAudioTrack::rewind() {
	Audio::RewindableAudioStream *stream = getRewindableAudioStream();
	assert(stream);
	return stream->rewind();
}

Audio::Timestamp VideoDecoder::SeekableAudioTrack::getDuration() const {
	Audio::SeekableAudioStream *stream = getSeekableAudioStream();
	assert(stream);
	return stream->getLength();
}

Audio::AudioStream *VideoDecoder::SeekableAudioTrack::getAudioStream() const {
	return getSeekableAudioStream();
}

bool VideoDecoder::SeekableAudioTrack::seek(const Audio::Timestamp &time) {
	Audio::SeekableAudioStream *stream = getSeekableAudioStream();
	assert(stream);
	return stream->seek(time);
}

VideoDecoder::StreamFileAudioTrack::StreamFileAudioTrack() {
	_stream = 0;
}

VideoDecoder::StreamFileAudioTrack::~StreamFileAudioTrack() {
	delete _stream;
}

bool VideoDecoder::StreamFileAudioTrack::loadFromFile(const Common::String &baseName) {
	// TODO: Make sure the stream isn't being played
	delete _stream;
	_stream = Audio::SeekableAudioStream::openStreamFile(baseName);
	return _stream != 0;
}

void VideoDecoder::addTrack(Track *track) {
	_tracks.push_back(track);

	// Update volume settings if it's an audio track
	if (track->getTrackType() == Track::kTrackTypeAudio) {
		((AudioTrack *)track)->setVolume(_audioVolume);
		((AudioTrack *)track)->setBalance(_audioBalance);
	}

	// Keep the track paused if we're paused
	if (isPaused())
		track->pause(true);

	// Start the track if we're playing
	if (isPlaying() && track->getTrackType() == Track::kTrackTypeAudio)
		((AudioTrack *)track)->start();
}

bool VideoDecoder::addStreamFileTrack(const Common::String &baseName) {
	// Only allow adding external tracks if a video is already loaded
	if (!isVideoLoaded())
		return false;

	StreamFileAudioTrack *track = new StreamFileAudioTrack();

	bool result = track->loadFromFile(baseName);

	if (result)
		addTrack(track);

	return result;
}

void VideoDecoder::setEndTime(const Audio::Timestamp &endTime) {
	Audio::Timestamp startTime = 0;

	if (isPlaying()) {
		startTime = getTime();
		stopAudio();
	}

	_endTime = endTime;
	_endTimeSet = true;

	if (startTime > endTime)
		return;

	if (isPlaying()) {
		// We'll assume the audio track is going to start up at the same time it just was
		// and therefore not do any seeking.
		// Might want to set it anyway if we're seekable.
		startAudioLimit(_endTime.msecs() - startTime.msecs());
		_lastTimeChange = startTime;
	}
}

VideoDecoder::Track *VideoDecoder::getTrack(uint track) {
	if (track > _tracks.size())
		return 0;

	return _tracks[track];
}

const VideoDecoder::Track *VideoDecoder::getTrack(uint track) const {
	if (track > _tracks.size())
		return 0;

	return _tracks[track];
}

bool VideoDecoder::endOfVideoTracks() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo && !(*it)->endOfTrack())
			return false;

	return true;
}

VideoDecoder::VideoTrack *VideoDecoder::findNextVideoTrack() {
	VideoTrack *bestTrack = 0;
	uint32 bestTime = 0xFFFFFFFF;

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		if ((*it)->getTrackType() == Track::kTrackTypeVideo && !(*it)->endOfTrack()) {
			VideoTrack *track = (VideoTrack *)*it;
			uint32 time = track->getNextFrameStartTime();

			if (time < bestTime) {
				bestTime = time;
				bestTrack = track;
			}
		}
	}

	return bestTrack;
}

const VideoDecoder::VideoTrack *VideoDecoder::findNextVideoTrack() const {
	const VideoTrack *bestTrack = 0;
	uint32 bestTime = 0xFFFFFFFF;

	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++) {
		if ((*it)->getTrackType() == Track::kTrackTypeVideo && !(*it)->endOfTrack()) {
			const VideoTrack *track = (const VideoTrack *)*it;
			uint32 time = track->getNextFrameStartTime();

			if (time < bestTime) {
				bestTime = time;
				bestTrack = track;
			}
		}
	}

	return bestTrack;
}

void VideoDecoder::startAudio() {
	if (_endTimeSet) {
		// HACK: Timestamp's subtraction asserts out when subtracting two times
		// with different rates.
		startAudioLimit(_endTime - _lastTimeChange.convertToFramerate(_endTime.framerate()));
		return;
	}

	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->start();
}

void VideoDecoder::stopAudio() {
	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->stop();
}

void VideoDecoder::startAudioLimit(const Audio::Timestamp &limit) {
	for (TrackList::iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			((AudioTrack *)*it)->start(limit);
}

bool VideoDecoder::hasFramesLeft() const {
	// This is similar to endOfVideo(), except it doesn't take Audio into account (and returns true if not the end of the video)
	// This is only used for needsUpdate() atm so that setEndTime() works properly
	// And unlike endOfVideoTracks(), this takes into account _endTime
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeVideo && !(*it)->endOfTrack() && (!isPlaying() || !_endTimeSet || ((VideoTrack *)*it)->getNextFrameStartTime() < (uint)_endTime.msecs()))
			return true;

	return false;
}

bool VideoDecoder::hasAudio() const {
	for (TrackList::const_iterator it = _tracks.begin(); it != _tracks.end(); it++)
		if ((*it)->getTrackType() == Track::kTrackTypeAudio)
			return true;

	return false;
}

} // End of namespace Video
