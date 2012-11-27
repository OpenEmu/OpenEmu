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

#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/scumm_v7.h"

namespace Scumm {

void ScummEngine::setCameraAtEx(int at) {
	if (_game.version < 7) {
		camera._mode = kNormalCameraMode;
		camera._cur.x = at;
		setCameraAt(at, 0);
		camera._movingToActor = false;
	}
}

void ScummEngine::setCameraAt(int pos_x, int pos_y) {
	if (camera._mode != kFollowActorCameraMode || ABS(pos_x - camera._cur.x) > (_screenWidth / 2)) {
		camera._cur.x = pos_x;
	}
	camera._dest.x = pos_x;

	if (VAR_CAMERA_MIN_X != 0xFF && camera._cur.x < VAR(VAR_CAMERA_MIN_X))
		camera._cur.x = (short) VAR(VAR_CAMERA_MIN_X);

	if (VAR_CAMERA_MAX_X != 0xFF && camera._cur.x > VAR(VAR_CAMERA_MAX_X))
		camera._cur.x = (short) VAR(VAR_CAMERA_MAX_X);

	if (VAR_SCROLL_SCRIPT != 0xFF && VAR(VAR_SCROLL_SCRIPT)) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
		runScript(VAR(VAR_SCROLL_SCRIPT), 0, 0, 0);
	}

	// If the camera moved and text is visible, remove it
	if (camera._cur.x != camera._last.x && _charset->_hasMask && _game.version > 3)
		stopTalk();
}

void ScummEngine::setCameraFollows(Actor *a, bool setCamera) {

	int t, i;

	camera._mode = kFollowActorCameraMode;
	camera._follows = a->_number;

	if (!a->isInCurrentRoom()) {
		startScene(a->getRoom(), 0, 0);
		camera._mode = kFollowActorCameraMode;
		camera._cur.x = a->getPos().x;
		setCameraAt(camera._cur.x, 0);
	}

	t = a->getPos().x / 8 - _screenStartStrip;

	if (t < camera._leftTrigger || t > camera._rightTrigger || setCamera == true)
		setCameraAt(a->getPos().x, 0);

	for (i = 1; i < _numActors; i++) {
		if (_actors[i]->isInCurrentRoom())
			_actors[i]->_needRedraw = true;
	}
	runInventoryScript(0);
}

void ScummEngine::clampCameraPos(Common::Point *pt) {
	if (pt->x < VAR(VAR_CAMERA_MIN_X))
		pt->x = (short) VAR(VAR_CAMERA_MIN_X);

	if (pt->x > VAR(VAR_CAMERA_MAX_X))
		pt->x = (short) VAR(VAR_CAMERA_MAX_X);

	if (pt->y < VAR(VAR_CAMERA_MIN_Y))
		pt->y = (short) VAR(VAR_CAMERA_MIN_Y);

	if (pt->y > VAR(VAR_CAMERA_MAX_Y))
		pt->y = (short) VAR(VAR_CAMERA_MAX_Y);
}

void ScummEngine::moveCamera() {
	int pos = camera._cur.x;
	int t;
	Actor *a = NULL;
	const bool snapToX = (_snapScroll || (VAR_CAMERA_FAST_X != 0xFF && VAR(VAR_CAMERA_FAST_X)));

	camera._cur.x &= 0xFFF8;

	if (VAR_CAMERA_MIN_X != 0xFF && camera._cur.x < VAR(VAR_CAMERA_MIN_X)) {
		if (snapToX)
			camera._cur.x = (short) VAR(VAR_CAMERA_MIN_X);
		else
			camera._cur.x += 8;
		cameraMoved();
		return;
	}

	if (VAR_CAMERA_MAX_X != 0xFF && camera._cur.x > VAR(VAR_CAMERA_MAX_X)) {
		if (snapToX)
			camera._cur.x = (short) VAR(VAR_CAMERA_MAX_X);
		else
			camera._cur.x -= 8;
		cameraMoved();
		return;
	}

	if (camera._mode == kFollowActorCameraMode) {
		a = derefActor(camera._follows, "moveCamera");

		int actorx = a->getPos().x;
		t = actorx / 8 - _screenStartStrip;

		if (t < camera._leftTrigger || t > camera._rightTrigger) {
			if (snapToX) {
				if (t > 40-5)
					camera._dest.x = actorx + 80;
				if (t < 5)
					camera._dest.x = actorx - 80;
			} else
				camera._movingToActor = true;
		}
	}

	if (camera._movingToActor) {
		a = derefActor(camera._follows, "moveCamera(2)");
		camera._dest.x = a->getPos().x;
	}

	if (VAR_CAMERA_MIN_X != 0xFF && camera._dest.x < VAR(VAR_CAMERA_MIN_X))
		camera._dest.x = (short) VAR(VAR_CAMERA_MIN_X);

	if (VAR_CAMERA_MAX_X != 0xFF && camera._dest.x > VAR(VAR_CAMERA_MAX_X))
		camera._dest.x = (short) VAR(VAR_CAMERA_MAX_X);

	if (snapToX) {
		camera._cur.x = camera._dest.x;
	} else {
		if (camera._cur.x < camera._dest.x)
			camera._cur.x += 8;
		if (camera._cur.x > camera._dest.x)
			camera._cur.x -= 8;
	}

	/* Actor 'a' is set a bit above */
	if (camera._movingToActor && (camera._cur.x / 8) == (a->getPos().x / 8)) {
		camera._movingToActor = false;
	}

	cameraMoved();

	if (VAR_SCROLL_SCRIPT != 0xFF && VAR(VAR_SCROLL_SCRIPT) && pos != camera._cur.x) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
		runScript(VAR(VAR_SCROLL_SCRIPT), 0, 0, 0);
	}
}

void ScummEngine::cameraMoved() {
	int screenLeft;
	if (_game.version >= 7) {
		assert(camera._cur.x >= (_screenWidth / 2) && camera._cur.y >= (_screenHeight / 2));
	} else {
		if (camera._cur.x < (_screenWidth / 2)) {
			camera._cur.x = (_screenWidth / 2);
		} else if (camera._cur.x > _roomWidth - (_screenWidth / 2)) {
			camera._cur.x = _roomWidth - (_screenWidth / 2);
		}
	}

	_screenStartStrip = camera._cur.x / 8 - _gdi->_numStrips / 2;
	_screenEndStrip = _screenStartStrip + _gdi->_numStrips - 1;

	_screenTop = camera._cur.y - (_screenHeight / 2);
	if (_game.version >= 7) {
		screenLeft = camera._cur.x - (_screenWidth / 2);
	} else {
		screenLeft = _screenStartStrip * 8;
	}

	_virtscr[kMainVirtScreen].xstart = screenLeft;
}

void ScummEngine::panCameraTo(int x, int y) {
	camera._dest.x = x;
	camera._mode = kPanningCameraMode;
	camera._movingToActor = false;
}

void ScummEngine::actorFollowCamera(int act) {
	if (_game.version < 7) {
		int old;

		old = camera._follows;
		setCameraFollows(derefActor(act, "actorFollowCamera"));
		if (camera._follows != old)
			runInventoryScript(0);

		camera._movingToActor = false;
	}
}

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::setCameraAt(int pos_x, int pos_y) {
	Common::Point old;

	old = camera._cur;

	camera._cur.x = pos_x;
	camera._cur.y = pos_y;

	clampCameraPos(&camera._cur);

	camera._dest = camera._cur;
	VAR(VAR_CAMERA_DEST_X) = camera._dest.x;
	VAR(VAR_CAMERA_DEST_Y) = camera._dest.y;

	assert(camera._cur.x >= (_screenWidth / 2) && camera._cur.y >= (_screenHeight / 2));

	if (camera._cur.x != old.x || camera._cur.y != old.y) {
		if (VAR(VAR_SCROLL_SCRIPT)) {
			VAR(VAR_CAMERA_POS_X) = camera._cur.x;
			VAR(VAR_CAMERA_POS_Y) = camera._cur.y;
			runScript(VAR(VAR_SCROLL_SCRIPT), 0, 0, 0);
		}

		// Even though cameraMoved() is called automatically, we may
		// need to know at once that the camera has moved, or text may
		// be printed at the wrong coordinates. See bugs #795938 and
		// #929242
		cameraMoved();
	}
}

void ScummEngine_v7::setCameraFollows(Actor *a, bool setCamera) {

	byte oldfollow = camera._follows;
	int ax, ay;

	camera._follows = a->_number;
	VAR(VAR_CAMERA_FOLLOWED_ACTOR) = a->_number;

	if (!a->isInCurrentRoom()) {
		startScene(a->getRoom(), 0, 0);
	}

	ax = ABS(a->getPos().x - camera._cur.x);
	ay = ABS(a->getPos().y - camera._cur.y);

	if (ax > VAR(VAR_CAMERA_THRESHOLD_X) || ay > VAR(VAR_CAMERA_THRESHOLD_Y) || ax > (_screenWidth / 2) || ay > (_screenHeight / 2)) {
		setCameraAt(a->getPos().x, a->getPos().y);
	}

	if (a->_number != oldfollow)
		runInventoryScript(0);
}

void ScummEngine_v7::moveCamera() {
	Common::Point old = camera._cur;
	Actor *a = NULL;

	if (camera._follows) {
		a = derefActor(camera._follows, "moveCamera");
		if (ABS(camera._cur.x - a->getPos().x) > VAR(VAR_CAMERA_THRESHOLD_X) ||
				ABS(camera._cur.y - a->getPos().y) > VAR(VAR_CAMERA_THRESHOLD_Y)) {
			camera._movingToActor = true;
			if (VAR(VAR_CAMERA_THRESHOLD_X) == 0)
				camera._cur.x = a->getPos().x;
			if (VAR(VAR_CAMERA_THRESHOLD_Y) == 0)
				camera._cur.y = a->getPos().y;
			clampCameraPos(&camera._cur);
		}
	} else {
		camera._movingToActor = false;
	}

	if (camera._movingToActor) {
		VAR(VAR_CAMERA_DEST_X) = camera._dest.x = a->getPos().x;
		VAR(VAR_CAMERA_DEST_Y) = camera._dest.y = a->getPos().y;
	}

	assert(camera._cur.x >= (_screenWidth / 2) && camera._cur.y >= (_screenHeight / 2));

	clampCameraPos(&camera._dest);

	if (camera._cur.x < camera._dest.x) {
		camera._cur.x += (short) VAR(VAR_CAMERA_SPEED_X);
		if (camera._cur.x > camera._dest.x)
			camera._cur.x = camera._dest.x;
	}

	if (camera._cur.x > camera._dest.x) {
		camera._cur.x -= (short) VAR(VAR_CAMERA_SPEED_X);
		if (camera._cur.x < camera._dest.x)
			camera._cur.x = camera._dest.x;
	}

	if (camera._cur.y < camera._dest.y) {
		camera._cur.y += (short) VAR(VAR_CAMERA_SPEED_Y);
		if (camera._cur.y > camera._dest.y)
			camera._cur.y = camera._dest.y;
	}

	if (camera._cur.y > camera._dest.y) {
		camera._cur.y -= (short) VAR(VAR_CAMERA_SPEED_Y);
		if (camera._cur.y < camera._dest.y)
			camera._cur.y = camera._dest.y;
	}

	if (camera._cur.x == camera._dest.x && camera._cur.y == camera._dest.y) {

		camera._movingToActor = false;
		camera._accel.x = camera._accel.y = 0;
		VAR(VAR_CAMERA_SPEED_X) = VAR(VAR_CAMERA_SPEED_Y) = 0;
	} else {

		camera._accel.x += (short) VAR(VAR_CAMERA_ACCEL_X);
		camera._accel.y += (short) VAR(VAR_CAMERA_ACCEL_Y);

		VAR(VAR_CAMERA_SPEED_X) += camera._accel.x / 100;
		VAR(VAR_CAMERA_SPEED_Y) += camera._accel.y / 100;

		if (VAR(VAR_CAMERA_SPEED_X) > 8)
			VAR(VAR_CAMERA_SPEED_X) = 8;

		if (VAR(VAR_CAMERA_SPEED_Y) > 8)
			VAR(VAR_CAMERA_SPEED_Y) = 8;

	}

	cameraMoved();

	if (camera._cur.x != old.x || camera._cur.y != old.y) {
		VAR(VAR_CAMERA_POS_X) = camera._cur.x;
		VAR(VAR_CAMERA_POS_Y) = camera._cur.y;

		if (VAR(VAR_SCROLL_SCRIPT))
			runScript(VAR(VAR_SCROLL_SCRIPT), 0, 0, 0);
	}
}

void ScummEngine_v7::panCameraTo(int x, int y) {
	VAR(VAR_CAMERA_FOLLOWED_ACTOR) = camera._follows = 0;
	VAR(VAR_CAMERA_DEST_X) = camera._dest.x = x;
	VAR(VAR_CAMERA_DEST_Y) = camera._dest.y = y;
}
#endif

} // End of namespace Scumm
