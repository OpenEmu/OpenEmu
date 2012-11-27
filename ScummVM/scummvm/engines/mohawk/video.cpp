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

#include "mohawk/mohawk.h"
#include "mohawk/resource.h"
#include "mohawk/video.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/textconsole.h"
#include "common/system.h"

#include "graphics/palette.h"
#include "graphics/surface.h"

#include "video/qt_decoder.h"


namespace Mohawk {

void VideoEntry::clear() {
	video = 0;
	x = 0;
	y = 0;
	loop = false;
	enabled = false;
	start = Audio::Timestamp(0, 1);
	filename.clear();
	id = -1;
}

bool VideoEntry::endOfVideo() {
	return !video || video->endOfVideo();
}

VideoManager::VideoManager(MohawkEngine* vm) : _vm(vm) {
}

VideoManager::~VideoManager() {
	stopVideos();
}

void VideoManager::pauseVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++)
		if (_videoStreams[i].video)
			_videoStreams[i]->pauseVideo(true);
}

void VideoManager::resumeVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++)
		if (_videoStreams[i].video)
			_videoStreams[i]->pauseVideo(false);
}

void VideoManager::stopVideos() {
	for (uint16 i = 0; i < _videoStreams.size(); i++)
		delete _videoStreams[i].video;

	_videoStreams.clear();
}

void VideoManager::playMovieBlocking(const Common::String &filename, uint16 x, uint16 y, bool clearScreen) {
	VideoHandle videoHandle = createVideoHandle(filename, x, y, false);
	if (videoHandle == NULL_VID_HANDLE)
		return;

	// Clear screen if requested
	if (clearScreen) {
		_vm->_system->fillScreen(_vm->_system->getScreenFormat().RGBToColor(0, 0, 0));
		_vm->_system->updateScreen();
	}

	waitUntilMovieEnds(videoHandle);
}

void VideoManager::playMovieBlockingCentered(const Common::String &filename, bool clearScreen) {
	VideoHandle videoHandle = createVideoHandle(filename, 0, 0, false);
	if (videoHandle == NULL_VID_HANDLE)
		return;

	// Clear screen if requested
	if (clearScreen) {
		_vm->_system->fillScreen(_vm->_system->getScreenFormat().RGBToColor(0, 0, 0));
		_vm->_system->updateScreen();
	}

	_videoStreams[videoHandle].x = (_vm->_system->getWidth() - _videoStreams[videoHandle]->getWidth()) / 2;
	_videoStreams[videoHandle].y = (_vm->_system->getHeight() - _videoStreams[videoHandle]->getHeight()) / 2;

	waitUntilMovieEnds(videoHandle);
}

void VideoManager::waitUntilMovieEnds(VideoHandle videoHandle) {
	if (videoHandle == NULL_VID_HANDLE)
		return;

	bool continuePlaying = true;

	while (!_videoStreams[videoHandle].endOfVideo() && !_vm->shouldQuit() && continuePlaying) {
		if (updateMovies())
			_vm->_system->updateScreen();

		Common::Event event;
		while (_vm->_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
				continuePlaying = false;
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_SPACE:
					_vm->pauseGame();
					break;
				case Common::KEYCODE_ESCAPE:
					continuePlaying = false;
					_vm->doVideoTimer(videoHandle, true);
					break;
				default:
					break;
			}
			default:
				break;
			}
		}

		// Cut down on CPU usage
		_vm->_system->delayMillis(10);
	}

	delete _videoStreams[videoHandle].video;
	_videoStreams[videoHandle].clear();
}

void VideoManager::delayUntilMovieEnds(VideoHandle videoHandle) {
	while (!_videoStreams[videoHandle].endOfVideo() && !_vm->shouldQuit()) {
		if (updateMovies())
			_vm->_system->updateScreen();

		// Cut down on CPU usage
		_vm->_system->delayMillis(10);
	}

	delete _videoStreams[videoHandle].video;
	_videoStreams[videoHandle].clear();
}

VideoHandle VideoManager::playMovie(const Common::String &filename, int16 x, int16 y, bool loop) {
	VideoHandle videoHandle = createVideoHandle(filename, x, y, loop);
	if (videoHandle == NULL_VID_HANDLE)
		return NULL_VID_HANDLE;

	// Center x if requested
	if (x < 0)
		_videoStreams[videoHandle].x = (_vm->_system->getWidth() - _videoStreams[videoHandle]->getWidth()) / 2;

	// Center y if requested
	if (y < 0)
		_videoStreams[videoHandle].y = (_vm->_system->getHeight() - _videoStreams[videoHandle]->getHeight()) / 2;

	return videoHandle;
}

VideoHandle VideoManager::playMovie(uint16 id, int16 x, int16 y, bool loop) {
	VideoHandle videoHandle = createVideoHandle(id, x, y, loop);
	if (videoHandle == NULL_VID_HANDLE)
		return NULL_VID_HANDLE;

	// Center x if requested
	if (x < 0)
		_videoStreams[videoHandle].x = (_vm->_system->getWidth() - _videoStreams[videoHandle]->getWidth()) / 2;

	// Center y if requested
	if (y < 0)
		_videoStreams[videoHandle].y = (_vm->_system->getHeight() - _videoStreams[videoHandle]->getHeight()) / 2;

	return videoHandle;
}

bool VideoManager::updateMovies() {
	bool updateScreen = false;

	for (uint32 i = 0; i < _videoStreams.size() && !_vm->shouldQuit(); i++) {
		// Skip deleted videos
		if (!_videoStreams[i].video)
			continue;

		// Remove any videos that are over
		if (_videoStreams[i].endOfVideo()) {
			if (_videoStreams[i].loop) {
				_videoStreams[i]->seek(_videoStreams[i].start);
			} else {
				// Check the video time one last time before deleting it
				_vm->doVideoTimer(i, true);
				delete _videoStreams[i].video;
				_videoStreams[i].clear();
				continue;
			}
		}

		// Nothing more to do if we're paused
		if (_videoStreams[i]->isPaused())
			continue;

		// Check if we need to draw a frame
		if (_videoStreams[i]->needsUpdate()) {
			const Graphics::Surface *frame = _videoStreams[i]->decodeNextFrame();
			Graphics::Surface *convertedFrame = 0;

			if (frame && _videoStreams[i].enabled) {
				Graphics::PixelFormat pixelFormat = _vm->_system->getScreenFormat();

				if (frame->format != pixelFormat) {
					// We don't support downconverting to 8bpp
					if (pixelFormat.bytesPerPixel == 1)
						error("Cannot convert high color video frame to 8bpp");

					// Convert to the current screen format
					convertedFrame = frame->convertTo(pixelFormat, _videoStreams[i]->getPalette());
					frame = convertedFrame;
				} else if (pixelFormat.bytesPerPixel == 1 && _videoStreams[i]->hasDirtyPalette()) {
					// Set the palette when running in 8bpp mode only
					_vm->_system->getPaletteManager()->setPalette(_videoStreams[i]->getPalette(), 0, 256);
				}

				// Clip the width/height to make sure we stay on the screen (Myst does this a few times)
				uint16 width = MIN<int32>(_videoStreams[i]->getWidth(), _vm->_system->getWidth() - _videoStreams[i].x);
				uint16 height = MIN<int32>(_videoStreams[i]->getHeight(), _vm->_system->getHeight() - _videoStreams[i].y);
				_vm->_system->copyRectToScreen(frame->pixels, frame->pitch, _videoStreams[i].x, _videoStreams[i].y, width, height);

				// We've drawn something to the screen, make sure we update it
				updateScreen = true;

				// Delete 8bpp conversion surface
				if (convertedFrame) {
					convertedFrame->free();
					delete convertedFrame;
				}
			}
		}

		// Check the video time
		_vm->doVideoTimer(i, false);
	}

	// Return true if we need to update the screen
	return updateScreen;
}

void VideoManager::activateMLST(uint16 mlstId, uint16 card) {
	Common::SeekableReadStream *mlstStream = _vm->getResource(ID_MLST, card);
	uint16 recordCount = mlstStream->readUint16BE();

	for (uint16 i = 0; i < recordCount; i++) {
		MLSTRecord mlstRecord;
		mlstRecord.index = mlstStream->readUint16BE();
		mlstRecord.movieID = mlstStream->readUint16BE();
		mlstRecord.code = mlstStream->readUint16BE();
		mlstRecord.left = mlstStream->readUint16BE();
		mlstRecord.top = mlstStream->readUint16BE();

		for (byte j = 0; j < 2; j++)
			if (mlstStream->readUint16BE() != 0)
				warning("u0[%d] in MLST non-zero", j);

		if (mlstStream->readUint16BE() != 0xFFFF)
			warning("u0[2] in MLST not 0xFFFF");

		mlstRecord.loop = mlstStream->readUint16BE();
		mlstRecord.volume = mlstStream->readUint16BE();
		mlstRecord.u1 = mlstStream->readUint16BE();

		if (mlstRecord.u1 != 1)
			warning("mlstRecord.u1 not 1");

		// We've found a match, add it
		if (mlstRecord.index == mlstId) {
			// Make sure we don't have any duplicates
			for (uint32 j = 0; j < _mlstRecords.size(); j++)
				if (_mlstRecords[j].index == mlstRecord.index || _mlstRecords[j].code == mlstRecord.code) {
					_mlstRecords.remove_at(j);
					j--;
				}

			_mlstRecords.push_back(mlstRecord);
			break;
		}
	}

	delete mlstStream;
}

void VideoManager::clearMLST() {
	_mlstRecords.clear();
}

VideoHandle VideoManager::playMovieRiven(uint16 id) {
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id) {
			debug(1, "Play tMOV %d (non-blocking) at (%d, %d) %s", _mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top, _mlstRecords[i].loop != 0 ? "looping" : "non-looping");
			return createVideoHandle(_mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top, _mlstRecords[i].loop != 0, _mlstRecords[i].volume);
		}

	return NULL_VID_HANDLE;
}

void VideoManager::playMovieBlockingRiven(uint16 id) {
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id) {
			debug(1, "Play tMOV %d (blocking) at (%d, %d)", _mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top);
			VideoHandle videoHandle = createVideoHandle(_mlstRecords[i].movieID, _mlstRecords[i].left, _mlstRecords[i].top, false);
			waitUntilMovieEnds(videoHandle);
			return;
		}
}

void VideoManager::stopMovieRiven(uint16 id) {
	debug(2, "Stopping movie %d", id);
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id)
			for (uint16 j = 0; j < _videoStreams.size(); j++)
				if (_mlstRecords[i].movieID == _videoStreams[j].id) {
					delete _videoStreams[j].video;
					_videoStreams[j].clear();
					return;
				}
}

void VideoManager::enableMovieRiven(uint16 id) {
	debug(2, "Enabling movie %d", id);
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id)
			for (uint16 j = 0; j < _videoStreams.size(); j++)
				if (_mlstRecords[i].movieID == _videoStreams[j].id) {
					_videoStreams[j].enabled = true;
					return;
				}
}

void VideoManager::disableMovieRiven(uint16 id) {
	debug(2, "Disabling movie %d", id);
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id)
			for (uint16 j = 0; j < _videoStreams.size(); j++)
				if (_mlstRecords[i].movieID == _videoStreams[j].id) {
					_videoStreams[j].enabled = false;
					return;
				}
}

void VideoManager::disableAllMovies() {
	debug(2, "Disabling all movies");
	for (uint16 i = 0; i < _videoStreams.size(); i++)
		_videoStreams[i].enabled = false;
}

VideoHandle VideoManager::createVideoHandle(uint16 id, uint16 x, uint16 y, bool loop, byte volume) {
	// First, check to see if that video is already playing
	for (uint32 i = 0; i < _videoStreams.size(); i++)
		if (_videoStreams[i].id == id)
			return i;

	// Otherwise, create a new entry
	Video::QuickTimeDecoder *decoder = new Video::QuickTimeDecoder();
	decoder->setChunkBeginOffset(_vm->getResourceOffset(ID_TMOV, id));
	decoder->loadStream(_vm->getResource(ID_TMOV, id));
	decoder->setVolume(volume);

	VideoEntry entry;
	entry.clear();
	entry.video = decoder;
	entry.x = x;
	entry.y = y;
	entry.id = id;
	entry.loop = loop;
	entry.enabled = true;

	entry->start();

	// Search for any deleted videos so we can take a formerly used slot
	for (uint32 i = 0; i < _videoStreams.size(); i++)
		if (!_videoStreams[i].video) {
			_videoStreams[i] = entry;
			return i;
		}

	// Otherwise, just add it to the list
	_videoStreams.push_back(entry);
	return _videoStreams.size() - 1;
}

VideoHandle VideoManager::createVideoHandle(const Common::String &filename, uint16 x, uint16 y, bool loop, byte volume) {
	// First, check to see if that video is already playing
	for (uint32 i = 0; i < _videoStreams.size(); i++)
		if (_videoStreams[i].filename == filename)
			return i;

	// Otherwise, create a new entry
	VideoEntry entry;
	entry.clear();
	entry.video = new Video::QuickTimeDecoder();
	entry.x = x;
	entry.y = y;
	entry.filename = filename;
	entry.loop = loop;
	entry.enabled = true;

	Common::File *file = new Common::File();
	if (!file->open(filename)) {
		delete file;
		return NULL_VID_HANDLE;
	}

	entry->loadStream(file);
	entry->setVolume(volume);
	entry->start();

	// Search for any deleted videos so we can take a formerly used slot
	for (uint32 i = 0; i < _videoStreams.size(); i++)
		if (!_videoStreams[i].video) {
			_videoStreams[i] = entry;
			return i;
		}

	// Otherwise, just add it to the list
	_videoStreams.push_back(entry);
	return _videoStreams.size() - 1;
}

VideoHandle VideoManager::findVideoHandleRiven(uint16 id) {
	for (uint16 i = 0; i < _mlstRecords.size(); i++)
		if (_mlstRecords[i].code == id)
			for (uint32 j = 0; j < _videoStreams.size(); j++)
				if (_videoStreams[j].video && _mlstRecords[i].movieID == _videoStreams[j].id)
					return j;

	return NULL_VID_HANDLE;
}

VideoHandle VideoManager::findVideoHandle(uint16 id) {
	if (!id)
		return NULL_VID_HANDLE;

	for (uint32 i = 0; i < _videoStreams.size(); i++)
		if (_videoStreams[i].video && _videoStreams[i].id == id)
			return i;

	return NULL_VID_HANDLE;
}

VideoHandle VideoManager::findVideoHandle(const Common::String &filename) {
	if (filename.empty())
		return NULL_VID_HANDLE;

	for (uint32 i = 0; i < _videoStreams.size(); i++)
		if (_videoStreams[i].video && _videoStreams[i].filename.equalsIgnoreCase(filename))
			return i;

	return NULL_VID_HANDLE;
}

int VideoManager::getCurFrame(VideoHandle handle) {
	assert(handle != NULL_VID_HANDLE);
	return _videoStreams[handle]->getCurFrame();
}

uint32 VideoManager::getFrameCount(VideoHandle handle) {
	assert(handle != NULL_VID_HANDLE);
	return _videoStreams[handle]->getFrameCount();
}

uint32 VideoManager::getTime(VideoHandle handle) {
	assert(handle != NULL_VID_HANDLE);
	return _videoStreams[handle]->getTime();
}

uint32 VideoManager::getDuration(VideoHandle handle) {
	assert(handle != NULL_VID_HANDLE);
	return _videoStreams[handle]->getDuration().msecs();
}

bool VideoManager::endOfVideo(VideoHandle handle) {
	assert(handle != NULL_VID_HANDLE);
	return _videoStreams[handle].endOfVideo();
}

bool VideoManager::isVideoPlaying() {
	for (uint32 i = 0; i < _videoStreams.size(); i++)
		if (!_videoStreams[i].endOfVideo())
			return true;

	return false;
}

void VideoManager::setVideoBounds(VideoHandle handle, Audio::Timestamp start, Audio::Timestamp end) {
	assert(handle != NULL_VID_HANDLE);
	_videoStreams[handle].start = start;
	_videoStreams[handle]->setEndTime(end);
	_videoStreams[handle]->seek(start);
}

void VideoManager::drawVideoFrame(VideoHandle handle, Audio::Timestamp time) {
	assert(handle != NULL_VID_HANDLE);
	_videoStreams[handle]->seek(time);
	updateMovies();
	delete _videoStreams[handle].video;
	_videoStreams[handle].clear();
}

void VideoManager::seekToTime(VideoHandle handle, Audio::Timestamp time) {
	assert(handle != NULL_VID_HANDLE);
	_videoStreams[handle]->seek(time);
}

void VideoManager::setVideoLooping(VideoHandle handle, bool loop) {
	assert(handle != NULL_VID_HANDLE);
	_videoStreams[handle].loop = loop;
}

void VideoManager::pauseMovie(VideoHandle handle, bool pause) {
	assert(handle != NULL_VID_HANDLE);
	_videoStreams[handle]->pauseVideo(pause);
}

} // End of namespace Mohawk
