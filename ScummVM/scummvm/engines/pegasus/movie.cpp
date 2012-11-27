/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#include "common/system.h"
#include "graphics/surface.h"
#include "video/qt_decoder.h"
#include "video/video_decoder.h"

#include "pegasus/movie.h"

namespace Pegasus {

Movie::Movie(const DisplayElementID id) : Animation(id) {
	_video = 0;
	setScale(600);
}

Movie::~Movie() {
	releaseMovie();
}

// *** Make sure this will stop displaying the movie.

void Movie::releaseMovie() {
	if (_video) {
		delete _video;
		_video = 0;
		disposeAllCallBacks();
		deallocateSurface();
	}

	setBounds(Common::Rect(0, 0, 0, 0));
}

void Movie::initFromMovieFile(const Common::String &fileName, bool transparent) {
	_transparent = transparent;

	releaseMovie();
	_video = new Video::QuickTimeDecoder();
	if (!_video->loadFile(fileName)) {
		// Replace any colon with an underscore, since only Mac OS X
		// supports that. See PegasusEngine::detectOpeningClosingDirectory()
		// for more info.
		Common::String newName(fileName);
		if (newName.contains(':'))
			for (uint i = 0; i < newName.size(); i++)
				if (newName[i] == ':')
					newName.setChar('_', i);

		if (!_video->loadFile(newName))
			error("Could not load video '%s'", fileName.c_str());
	}

	Common::Rect bounds(0, 0, _video->getWidth(), _video->getHeight());
	sizeElement(_video->getWidth(), _video->getHeight());
	_movieBox = bounds;

	if (!isSurfaceValid())
		allocateSurface(bounds);

	setStart(0, getScale());
	TimeBase::setStop(_video->getDuration().convertToFramerate(getScale()).totalNumberOfFrames(), getScale());
}

void Movie::redrawMovieWorld() {
	if (_video && _video->needsUpdate()) {
		const Graphics::Surface *frame = _video->decodeNextFrame();

		if (!frame)
			return;

		// Make sure we have a surface in the current pixel format
		Graphics::Surface *convertedFrame = 0;

		if (frame->format != g_system->getScreenFormat()) {
			convertedFrame = frame->convertTo(g_system->getScreenFormat());
			frame = convertedFrame;
		}

		// Copy to the surface using _movieBox
		uint16 width = MIN<int>(frame->w, _movieBox.width());
		uint16 height = MIN<int>(frame->h, _movieBox.height());

		for (uint16 y = 0; y < height; y++)
			memcpy((byte *)_surface->getBasePtr(_movieBox.left, _movieBox.top + y), (const byte *)frame->getBasePtr(0, y), width * frame->format.bytesPerPixel);

		if (convertedFrame) {
			convertedFrame->free();
			delete convertedFrame;
		}

		triggerRedraw();
	}
}

void Movie::draw(const Common::Rect &r) {
	Common::Rect worldBounds = _movieBox;
	Common::Rect elementBounds;
	getBounds(elementBounds);

	worldBounds.moveTo(elementBounds.left, elementBounds.top);
	Common::Rect r1 = r.findIntersectingRect(worldBounds);

	Common::Rect r2 = r1;
	r2.translate(_movieBox.left - elementBounds.left, _movieBox.top - elementBounds.top);
	drawImage(r2, r1);
}

void Movie::moveMovieBoxTo(const CoordType h, const CoordType v) {
	_movieBox.moveTo(h, v);
}

void Movie::setStop(const TimeValue stopTime, const TimeScale scale) {
	TimeBase::setStop(stopTime, scale);

	if (_video)
		_video->setEndTime(Audio::Timestamp(0, _stopTime, _stopScale));
}

void Movie::setVolume(uint16 volume) {
	if (_video)
		_video->setVolume(MIN<uint>(volume, 0xFF));
}

void Movie::setTime(const TimeValue time, const TimeScale scale) {
	if (_video) {
		// Don't go past the ends of the movie
		Common::Rational timeFrac = Common::Rational(time, ((scale == 0) ? getScale() : scale));

		if (timeFrac < Common::Rational(_startTime, _startScale))
			timeFrac = Common::Rational(_startTime, _startScale);
		else if (timeFrac >= Common::Rational(_stopTime, _stopScale))
			return;

		_video->seek(Audio::Timestamp(0, timeFrac.getNumerator(), timeFrac.getDenominator()));
		_time = timeFrac;
		_lastMillis = 0;
	}
}

void Movie::setRate(const Common::Rational rate) {
	if (_video) {
		_video->setRate(rate);

		TimeBase::setRate(_video->getRate());
		return;
	}

	TimeBase::setRate(rate);
}

void Movie::start() {
	if (_video)
		_video->start();

	TimeBase::start();
}

void Movie::stop() {
	if (_video)
		_video->stop();

	TimeBase::stop();
}

void Movie::resume() {
	if (_paused) {
		if (_video)
			_video->pauseVideo(false);

		_paused = false;
	}
}

void Movie::pause() {
	if (isRunning() && !_paused) {
		if (_video)
			_video->pauseVideo(true);

		_paused = true;
		_pauseStart = g_system->getMillis();
	}
}

TimeValue Movie::getDuration(const TimeScale scale) const {
	// Unlike TimeBase::getDuration(), this returns the whole duration of the movie
	// The original source has a TODO to make this behave like TimeBase::getDuration(),
	// but the problem is that too much code requires this function to behave this way...

	if (_video)
		return _video->getDuration().convertToFramerate(((scale == 0) ? getScale() : scale)).totalNumberOfFrames();

	return 0;
}

void Movie::updateTime() {
	// The reason why we overrode TimeBase's updateTime():
	// Again, avoiding timers and handling it here
	if (_video && _video->isPlaying() && !_video->isPaused()) {
		redrawMovieWorld();

		uint32 startTime = _startTime * getScale() / _startScale;
		uint32 stopTime = _stopTime * getScale() / _stopScale;
		uint32 actualTime = CLIP<int>(_video->getTime() * getScale() / 1000, startTime, stopTime);

		// HACK: Due to the inaccuracy of the time, we won't actually allow us to hit
		// the stop time unless we've actually reached the end of the segment.
		if (actualTime == stopTime && !_video->endOfVideo())
			actualTime--;

		_time = Common::Rational(actualTime, getScale());
	}
}

GlowingMovie::GlowingMovie(const DisplayElementID id) : Movie(id) {
	_glowing = false;
}

void GlowingMovie::draw(const Common::Rect &r) {
	// Make sure the rectangles are clipped properly, OR guarantee that _bounds will
	// never fall off the screen.
	if (_glowing) {
		Common::Rect bounds;
		getBounds(bounds);

		copyToCurrentPortTransparentGlow(_movieBox, bounds);
	} else {
		Movie::draw(r);
	}
}

void GlowingMovie::setBounds(const Common::Rect &r) {
	Common::Rect bounds;
	getBounds(bounds);

	if (r != bounds) {
		// Avoid Movie::setBounds.
		// clone2727 asks why, but goes along with it
		Animation::setBounds(r);
	}
}

ScalingMovie::ScalingMovie(const DisplayElementID id) : GlowingMovie(id) {
}

void ScalingMovie::draw(const Common::Rect &) {
	// Make sure the rectangles are clipped properly, OR guarantee that _bounds will
	// never fall off the screen.

	Common::Rect bounds;
	getBounds(bounds);

	if (_glowing)
		scaleTransparentCopyGlow(_movieBox, bounds);
	else
		scaleTransparentCopy(_movieBox, bounds);
}

} // End of namespace Pegasus
