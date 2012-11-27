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

#include "pegasus/neighborhood/norad/alpha/panoramascroll.h"

namespace Pegasus {

PanoramaScroll::PanoramaScroll(const DisplayElementID id) : IdlerAnimation(id) {
	_boundsWidth = 0;
	_totalWidth = 0;
}

void PanoramaScroll::initFromMovieFile(const Common::String &fileName) {
	_panorama.initFromMovieFile(fileName);

	Common::Rect r;
	_panorama.getPanoramaBounds(r);
	_totalWidth = r.width();
}

void PanoramaScroll::initMaskFromPICTFile(const Common::String &fileName) {
	if (!_panorama.isPanoramaOpen())
		return;

	_mask.getImageFromPICTFile(fileName);
	_panorama.setMask(&_mask);
}

void PanoramaScroll::releasePanorama() {
	if (_panorama.isPanoramaOpen())
		_panorama.releasePanorama();

	_mask.deallocateSurface();
}

void PanoramaScroll::setBounds(const Common::Rect &r) {
	Animation::setBounds(r);

	_boundsWidth = r.width();

	Common::Rect r2;
	_panorama.getViewBounds(r2);
	r2.right = r2.left + _boundsWidth;
	r2.bottom = r2.top + r.height();
	_panorama.setViewBounds(r2);
}

void PanoramaScroll::draw(const Common::Rect &) {
	_panorama.drawPanorama(_bounds);
}

void PanoramaScroll::timeChanged(const TimeValue newTime) {
	CoordType leftPixel = (_totalWidth - _boundsWidth) * newTime / getDuration();

	Common::Rect r;
	_panorama.getViewBounds(r);
	if (leftPixel != r.left) {
		_panorama.getViewBounds(r);
		r.moveTo(leftPixel, 0);
		_panorama.setViewBounds(r);
		triggerRedraw();
	}
}

void PanoramaScroll::getPanoramaBounds(Common::Rect &r) const {
	_panorama.getPanoramaBounds(r);
}

} // End of namespace Pegasus
