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

#include "common/macresman.h"
#include "common/stream.h"
#include "pegasus/neighborhood/norad/alpha/panorama.h"

namespace Pegasus {

Panorama::Panorama() : _panoramaMovie(kNoDisplayElement) {
	blankFields();
}

Panorama::~Panorama() {
	releasePanorama();
}

void Panorama::blankFields() {
	_viewBounds = Common::Rect();
	_drawBounds = Common::Rect();
	_mask = 0;
	_panoramaWidth = 0;
	_panoramaHeight = 0;
	_stripWidth = 0;
	_stripLeft = -1;
	_stripRight = -1;
}

void Panorama::releasePanorama() {
	if (_panoramaMovie.isMovieValid()) {
		_panoramaMovie.releaseMovie();
		_panoramaWorld.deallocateSurface();
		blankFields();
	}
}

static const uint32 kPanoramaResType = MKTAG('P', 'a', 'n', 'I'); // Panorama Information.
static const uint16 kPanoramaResID = 128;

void Panorama::initFromMovieFile(const Common::String &fileName) {
	// First, we need the resource fork for other reasons -- PanI resource
	Common::MacResManager *resFork = new Common::MacResManager();
	if (!resFork->open(fileName) || !resFork->hasResFork())
		error("Could not open the resource fork of '%s'", fileName.c_str());

	Common::SeekableReadStream *resource = resFork->getResource(kPanoramaResType, kPanoramaResID);
	if (!resource)
		error("No panorama information in the resource fork of '%s'", fileName.c_str());

	_panoramaWidth = resource->readUint16BE();
	_panoramaHeight = resource->readUint16BE();
	_stripWidth = resource->readUint16BE();

	delete resource;
	delete resFork;

	// Now we open the movie like normal
	_panoramaMovie.initFromMovieFile(fileName);
}

void Panorama::setMask(Surface *mask) {
	_mask = mask;
}

// If the panorama is not open, do nothing and return.
// Otherwise, set up the view bounds.
void Panorama::setViewBounds(const Common::Rect &newView) {
	if (!isPanoramaOpen())
		return;

	if (newView.isEmpty())
		return;

	Common::Rect r = newView;

	if (r.width() > _panoramaWidth) {
		r.left = 0;
		r.right = _panoramaWidth;
	} else {
		if (r.right > _panoramaWidth)
			r.translate(_panoramaWidth - r.right, 0);

		if (r.left < 0)
			r.translate(-r.left, 0);
	}

	if (r.height() > _panoramaHeight) {
		r.top = 0;
		r.bottom = _panoramaHeight;
	} else {
		if (r.bottom > _panoramaHeight)
			r.translate(0, _panoramaHeight - r.bottom);

		if (r.top < 0)
			r.translate(0, -r.top);
	}

	if (_viewBounds != r) {
		CoordType stripLeft = 0;

		if (r.width() != _viewBounds.width() || !_panoramaWorld.isSurfaceValid()) {
			_panoramaWorld.deallocateSurface();
			makeNewSurface(r);
		} else {
			CoordType stripRight;
			calcStripRange(r, stripLeft, stripRight);
			loadStrips(stripLeft, stripRight);
		}

		_viewBounds = r;
		_drawBounds = r;
		_drawBounds.translate(-stripLeft * _stripWidth, 0);
	}
}

void Panorama::getViewBounds(Common::Rect &r) const {
	r = _viewBounds;
}

void Panorama::getPanoramaBounds(Common::Rect &r) const {
	r = Common::Rect(0, 0, _panoramaWidth, _panoramaHeight);
}

void Panorama::drawPanorama(const Common::Rect &destRect) {
	if (_panoramaWorld.isSurfaceValid()) {
		if (_mask)
			_panoramaWorld.copyToCurrentPortMasked(_drawBounds, destRect, _mask);
		else
			_panoramaWorld.copyToCurrentPortTransparent(_drawBounds, destRect);
	}
}

// Make a new Surface big enough to show r, which is assumed to be a valid view bounds.
// Assumptions:
//      r is a valid view bounds.
//      _panoramaWorld is not allocated.
//      _panoramaHeight, _stripWidth is correct.
//      _panoramaMovie is allocated.
void Panorama::makeNewSurface(const Common::Rect& view) {
	CoordType stripLeft, stripRight;
	calcStripRange(view, stripLeft, stripRight);

	Common::Rect r(0, 0, (stripRight - stripLeft + 1) * _stripWidth, _panoramaHeight);
	_panoramaWorld.allocateSurface(r);
	_panoramaMovie.shareSurface(&_panoramaWorld);
	loadStrips(stripLeft, stripRight);
}

// Assumes view is not empty.
void Panorama::calcStripRange(const Common::Rect &view, CoordType &stripLeft, CoordType &stripRight) {
	stripLeft = view.left / _stripWidth;
	stripRight = (view.left - view.left % _stripWidth + _stripWidth - 1 + view.width()) / _stripWidth;
}

// Load in all needed strips to put range (stripLeft, stripRight) into the
// panorama's Surface. Try to optimize by saving any pixels already in the Surface.
// Assumptions:
//      Surface is allocated and is big enough for maximum range of
//          stripLeft and stripRight
void Panorama::loadStrips(CoordType stripLeft, CoordType stripRight) {
	if (_stripLeft == -1) {
		// Surface has just been allocated.
		// Load in all strips.
		for (CoordType i = stripLeft; i <= stripRight; i++)
			loadOneStrip(i, stripLeft);

		_stripLeft = stripLeft;
		_stripRight = stripRight;
	} else if (stripLeft != _stripLeft) {
		CoordType overlapLeft = MAX(stripLeft, _stripLeft);
		CoordType overlapRight = MIN(stripRight, _stripRight);

		if (overlapLeft <= overlapRight) {
			Common::Rect r1((overlapLeft - _stripLeft) * _stripWidth, 0,
					(overlapRight - _stripLeft + 1) * _stripWidth, _panoramaHeight);

			if (stripLeft < _stripLeft) {
				Common::Rect bounds;
				_panoramaWorld.getSurfaceBounds(bounds);
				_panoramaWorld.getSurface()->move(bounds.right - r1.right, 0, _panoramaHeight);

				for (CoordType i = stripLeft; i < _stripLeft; i++)
					loadOneStrip(i, stripLeft);
			} else {
				_panoramaWorld.getSurface()->move(-r1.left, 0, _panoramaHeight);

				for (CoordType i = _stripRight + 1; i <= stripRight; i++)
					loadOneStrip(i, stripLeft);
			}
		} else {
			// No overlap.
			// Load everything.
			for (CoordType i = stripLeft; i <= stripRight; i++)
				loadOneStrip(i, stripLeft);
		}

		_stripLeft = stripLeft;
		_stripRight = stripRight;
	} else if (stripRight > _stripRight) {
		// Need to add one or more strips.
		for (CoordType i = _stripRight + 1; i <= stripRight; i++)
			loadOneStrip(i, _stripLeft);

		_stripRight = stripRight;
	} else if (stripRight < _stripRight) {
		// Need to chop off one strip.
		_stripRight = stripRight;
	}
}

void Panorama::loadOneStrip(CoordType stripToLoad, CoordType leftStrip) {
	_panoramaMovie.moveMovieBoxTo((stripToLoad - leftStrip) * _stripWidth, 0);
	_panoramaMovie.setTime(stripToLoad, 1);
	_panoramaMovie.redrawMovieWorld();
}

} // End of namespace Pegasus
