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

#include "common/str.h"

#include "gob/gob.h"
#include "gob/surface.h"
#include "gob/anifile.h"
#include "gob/video.h"

#include "gob/pregob/onceupon/stork.h"

enum Animation {
	kAnimFlyNearWithBundle    =  9,
	kAnimFlyFarWithBundle     = 12,
	kAnimFlyNearWithoutBundle = 10,
	kAnimFlyFarWithoutBundle  = 13,
	kAnimBundleNear           = 11,
	kAnimBundleFar            = 14
};

namespace Gob {

namespace OnceUpon {

Stork::Stork(GobEngine *vm, const ANIFile &ani) : ANIObject(ani), _shouldDrop(false) {
	_frame = new Surface(320, 200, 1);
	vm->_video->drawPackedSprite("cadre.cmp", *_frame);

	_bundle = new ANIObject(ani);

	_bundle->setVisible(false);
	_bundle->setPause(true);

	setState(kStateFlyNearWithBundle, kAnimFlyNearWithBundle, -80);
}

Stork::~Stork() {
	delete _frame;

	delete _bundle;
}

bool Stork::hasBundleLanded() const {
	if (!_shouldDrop || !_bundle->isVisible() || _bundle->isPaused())
		return false;

	int16 x, y, width, height;
	_bundle->getFramePosition(x, y);
	_bundle->getFrameSize(width, height);

	return (y + height) >= _bundleDrop.landY;
}

void Stork::dropBundle(const BundleDrop &drop) {
	if (_shouldDrop)
		return;

	_shouldDrop = true;
	_bundleDrop = drop;
}

bool Stork::draw(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom) {
	left   = 0x7FFF;
	top    = 0x7FFF;
	right  = 0x0000;
	bottom = 0x0000;

	bool drawn = ANIObject::draw(dest, left, top, right, bottom);
	if (drawn) {
		// Left frame edge
		if (left <= 15)
			dest.blit(*_frame, left, top, MIN<int16>(15, right), bottom, left, top);

		// Right frame edge
		if (right >= 304)
			dest.blit(*_frame, MAX<int16>(304, left), top, right, bottom, MAX<int16>(304, left), top);
	}

	int16 bLeft, bTop, bRight, bBottom;
	if (_bundle->draw(dest, bLeft, bTop, bRight, bBottom)) {
		// Bottom frame edge
		if (bBottom >= 188)
			dest.blit(*_frame, bLeft, MAX<int16>(188, bTop), bRight, bBottom, bLeft, MAX<int16>(188, bTop));

		left   = MIN(left  , bLeft  );
		top    = MIN(top   , bTop   );
		right  = MAX(right , bRight );
		bottom = MAX(bottom, bBottom);

		drawn = true;
	}

	return drawn;
}

bool Stork::clear(Surface &dest, int16 &left, int16 &top, int16 &right, int16 &bottom) {
	left   = 0x7FFF;
	top    = 0x7FFF;
	right  = 0x0000;
	bottom = 0x0000;

	bool cleared = _bundle->clear(dest, left, top, right, bottom);

	int16 sLeft, sTop, sRight, sBottom;
	if (ANIObject::clear(dest, sLeft, sTop, sRight, sBottom)) {
		left   = MIN(left  , sLeft  );
		top    = MIN(top   , sTop   );
		right  = MAX(right , sRight );
		bottom = MAX(bottom, sBottom);

		cleared = true;
	}

	return cleared;
}

void Stork::advance() {
	_bundle->advance();

	ANIObject::advance();

	int16 curX, curY, curWidth, curHeight;
	getFramePosition(curX, curY, 0);
	getFrameSize(curWidth, curHeight, 0);

	const int16 curRight = curX + curWidth - 1;

	int16 nextX, nextY, nextWidth, nextHeight;
	getFramePosition(nextX, nextY, 1);
	getFrameSize(nextWidth, nextHeight, 1);

	const int16 nextRight = nextX + nextWidth - 1;

	switch (_state) {
	case kStateFlyNearWithBundle:
		if (curX >= 330)
			setState(kStateFlyFarWithBundle, kAnimFlyFarWithBundle, 330);

		if ((curRight  <= _bundleDrop.dropX) &&
		    (nextRight >= _bundleDrop.dropX) && _shouldDrop && !_bundleDrop.dropWhileFar)
			dropBundle(kStateFlyNearWithoutBundle, kAnimFlyNearWithoutBundle);

		break;

	case kStateFlyFarWithBundle:
		if (curX <= -80)
			setState(kStateFlyNearWithBundle, kAnimFlyNearWithBundle, -80);

		if ((curX  >= _bundleDrop.dropX) &&
		    (nextX <= _bundleDrop.dropX) && _shouldDrop && _bundleDrop.dropWhileFar)
			dropBundle(kStateFlyFarWithoutBundle, kAnimFlyFarWithoutBundle);

		break;

	case kStateFlyNearWithoutBundle:
		if (curX >= 330)
			setState(kStateFlyFarWithoutBundle, kAnimFlyFarWithoutBundle, 330);
		break;

	case kStateFlyFarWithoutBundle:
		if (curX <= -80)
			setState(kStateFlyNearWithoutBundle, kAnimFlyNearWithoutBundle, -80);
		break;

	default:
		break;
	}
}

void Stork::dropBundle(State state, uint16 anim) {
	setState(state, anim);

	int16 x, y, width, height;
	getFramePosition(x, y);
	getFrameSize(width, height);

	_bundle->setAnimation(_bundleDrop.anim);
	_bundle->setPause(false);
	_bundle->setVisible(true);

	int16 bWidth, bHeight;
	_bundle->getFrameSize(bWidth, bHeight);

	// Drop position
	x = _bundleDrop.dropX;
	y = y + height - bHeight;

	// If the stork is flying near (from left to right), drop the bundle at the right edge
	if (!_bundleDrop.dropWhileFar)
		x = x - bWidth;

	_bundle->setPosition(x, y);
}

void Stork::setState(State state, uint16 anim) {
	setAnimation(anim);
	setVisible(true);
	setPause(false);

	_state = state;
}

void Stork::setState(State state, uint16 anim, int16 x) {
	setState(state, anim);
	setPosition();

	int16 pX, pY;
	getPosition(pX, pY);
	setPosition( x, pY);
}

} // End of namespace OnceUpon

} // End of namespace Gob
