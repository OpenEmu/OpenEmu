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

#include "pegasus/compass.h"

namespace Pegasus {

Compass *g_compass = 0;

Compass::Compass() : FaderAnimation(kCompassID) {
	// Initialize it to east...
	setFaderValue(90);
	g_compass = this;
}

Compass::~Compass() {
	g_compass = 0;
}

void Compass::initCompass() {
	if (!isCompassValid()) {
		Common::Rect r;
		_compassImage.initFromPICTFile("Images/Compass/Compass");
		_compassImage.getSurfaceBounds(r);
		r.right = kCompassWidth;
		setBounds(r);
	}
}

void Compass::deallocateCompass() {
	_compassImage.deallocateSurface();
}

void Compass::setFaderValue(const int32 angle) {
	int16 a = angle % 360;

	if (a < 0)
		a += 360;

	FaderAnimation::setFaderValue(a);
}

void Compass::draw(const Common::Rect &r1) {
	if (_compassImage.isSurfaceValid()) {
		Common::Rect bounds;
		getBounds(bounds);

		Common::Rect r2;
		_compassImage.getSurfaceBounds(r2);

		CoordType width = r2.width();
		CoordType offsetH = width / 10 - bounds.width() / 2 + (getFaderValue() * width) / 450 - bounds.left;
		CoordType offsetV = -bounds.top;
		r2 = r1;
		r2.translate(offsetH, offsetV);
		_compassImage.drawImage(r2, r1);
	}
}

} // End of namespace Pegasus
