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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "tsage/blue_force/blueforce_scenes7.h"
#include "tsage/globals.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 710 - Beach
 *
 *--------------------------------------------------------------------------*/

void Scene710::Timer1::signal() {
	PaletteRotation *rotation = BF_GLOBALS._scenePalette.addRotation(136, 138, -1);
	rotation->setDelay(20);
	rotation = BF_GLOBALS._scenePalette.addRotation(146, 148, -1);
	rotation->setDelay(30);
	rotation = BF_GLOBALS._scenePalette.addRotation(187, 191, -1);
	rotation->setDelay(35);
	rotation = BF_GLOBALS._scenePalette.addRotation(245, 246, -1);
	rotation->setDelay(20);
	remove();
}

void Scene710::Action1::signal() {
	Scene710 *scene = (Scene710 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		_state = 7104;
		if (BF_INVENTORY.getObjectScene(INV_CRATE1) == 1)
			_state = 7105;
		setDelay(3);
		break;
	case 1: {
		ADD_MOVER(BF_GLOBALS._player, scene->_laura._position.x + 8, scene->_laura._position.y + 8);
		break;
		}
	case 2:
		BF_GLOBALS._player._strip = 2;
		scene->_stripManager.start(_state, this);
		break;
	case 3:
		if (_state != 7105)
			BF_GLOBALS._player.enableControl();
		else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 7106;
			scene->setAction(&scene->_sequenceManager1, scene, 7106, &BF_GLOBALS._player, &scene->_laura, &scene->_kid, &scene->_dog, NULL);
		}
		if (_state < 7104) {
			_state++;
			if ((_state == 7104) && (BF_INVENTORY.getObjectScene(INV_CRATE1) == 1))
					_state = 7105;
		}
		remove();
		break;
	default:
		break;
	}
}

// Laura
bool Scene710::Object3::startAction(CursorType action, Event &event) {
	Scene710 *scene = (Scene710 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_TALK) {
		BF_GLOBALS._player.setAction(&scene->_action1);
		return true;
	} else
		return NamedObject::startAction(action, event);
}

bool Scene710::Object4::startAction(CursorType action, Event &event) {
	Scene710 *scene = (Scene710 *)BF_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_LOOK) && (scene->_kid._position.x < 0)) {
		SceneItem::display2(710, 13);
		return true;
	} else
		return NamedObject::startAction(action, event);
}

//Stick
bool Scene710::Object5::startAction(CursorType action, Event &event) {
	Scene710 *scene = (Scene710 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (scene->_stickThrowCount <= 2)
			return NamedObject::startAction(action, event);
		else {
			SceneItem::display2(710, 3);
			scene->_watchCrate = true;
			return true;
		}
	case CURSOR_USE:
		if ((scene->_kid._position.x < 0) && (scene->_dogLying)) {
			scene->_stickThrowCount++;
			if (!scene->_watchCrate) {
				BF_GLOBALS._player.disableControl();
				scene->_dogLying = false;
				scene->_sceneMode = 7105;
				scene->setAction(&scene->_sequenceManager1, scene, 7105, &BF_GLOBALS._player, &scene->_stick, &scene->_dog, NULL);
			} else {
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 7101;
				scene->setAction(&scene->_sequenceManager1, scene, 7101, &BF_GLOBALS._player, &scene->_dog, &scene->_stick, NULL);
			}
			return true;
		}
		// No break on purpose
	default:
		return NamedObject::startAction(action, event);
	}
}

void Scene710::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(710);
	_sceneBounds.moveTo(320, 0);

	BF_GLOBALS._sound1.fadeSound(14);
	_soundExt1.fadeSound(48);
	_v51C34.set(40, 0, 280, 240);
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.hide();
	BF_GLOBALS._player._moveDiff = Common::Point(4, 2);
	BF_GLOBALS._player.disableControl();
	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 1;
	_stripManager.addSpeaker(&_jakeSpeaker);
	_stripManager.addSpeaker(&_skipSpeaker);
	_stripManager.addSpeaker(&_lauraSpeaker);
	_stripManager.addSpeaker(&_gameTextSpeaker);

	_kid.postInit();
	_kid._moveDiff = Common::Point(4, 2);
	_laura.postInit();
	_laura._moveDiff = Common::Point(4, 2);
	_dog.postInit();
	_dog._moveDiff = Common::Point(4, 2);
	_stick.postInit();
	_stick.setVisage(710);
	_stick.setStrip(2);
	_stick.animate(ANIM_MODE_2, NULL);
	_stick.setPosition(Common::Point(650, 160));
	_stick._moveDiff.x = 16;
	_stick.setDetails(710, 4, -1, -1, 1, (SceneItem *)NULL);
	_laura.setDetails(710, 2, -1, -1, 1, (SceneItem *)NULL);
	_kid.setDetails(710, 6, -1, -1, 1, (SceneItem *)NULL);
	_dog.setDetails(710, 0, -1, -1, 1, (SceneItem *)NULL);

	_item1.setDetails(Rect(555, 68, 583, 101), 710, 7,  23, -1, 1, NULL);
	_item2.setDetails(Rect(583, 46, 611,  78), 710, 7,  23, -1, 1, NULL);
	_item3.setDetails(Rect(611, 24, 639,  56), 710, 7,  23, -1, 1, NULL);
	_item6.setDetails(1, 710, 9,  1,  -1, 1);
	_item4.setDetails(2, 710, 8,  14, -1, 1);
	_item5.setDetails(3, 710, 10, 16, -1, 1);
	_item8.setDetails(Rect(222, 18, 249,  42), 710, 12, 18, -1, 1, NULL);
	_item7.setDetails(Rect(0,    0, 640,  52), 710, 11, 17, -1, 1, NULL);
	_item9.setDetails(Rect(0,    0, 640, 128), 710,  5, 15, -1, 1, NULL);

	_stickThrowCount = 0;
	_dogLying = _watchCrate = _throwStick = false;
	_action1._state = 7100;
	_timer1.set(2, NULL);
	_sceneMode = 7100;
	setAction(&_sequenceManager1, this, 7100, &BF_GLOBALS._player, &_laura, &_kid, &_dog, NULL);
}

void Scene710::signal() {
	switch (_sceneMode) {
	case 0:
		BF_GLOBALS._player.enableControl();
		break;
	case 7100:
		BF_GLOBALS._player.enableControl();
		_sceneMode = 7102;
		setAction(&_sequenceManager1, this, 7102, &_dog, NULL);
		break;
	case 7101:
		// Pick up crate part
		BF_GLOBALS._player.enableControl();
		BF_INVENTORY.setObjectScene(INV_CRATE1, 1);
		_stick.remove();
		BF_GLOBALS._walkRegions.enableRegion(2);
		break;
	case 7102:
		_stick.setPosition(Common::Point(100, 122));
		_stick.animate(ANIM_MODE_NONE, NULL);
		_stick._strip = 2;
		if (_stickThrowCount <= 2)
			_stick._frame = 2;
		else {
			if (_stickThrowCount == 3) {
				BF_GLOBALS._player.disableControl();
				_sceneMode = 0;
				_stripManager.start(7108, this);
			}
			_stick._frame = 1;
		}
		_dogLying = true;
		BF_GLOBALS._walkRegions.disableRegion(2);
		if ((_throwStick) && (_sceneMode != 0))
			BF_GLOBALS._player.enableControl();
		break;
	case 7103:
		if (BF_GLOBALS._player._position.x > 179) {
			_sceneMode = 7102;
			setAction(&_sequenceManager1, this, 7102, &_dog, NULL);
		} else {
			_sceneMode = 7104;
			setAction(&_sequenceManager3, this, 7104, &_kid, NULL);
		}
		break;
	case 7105:
		_throwStick = true;
		// No break on purpose
	case 7104:
		_sceneMode = 7102;
		setAction(&_sequenceManager1, this, 7102, &_dog, NULL);
		BF_GLOBALS._walkRegions.enableRegion(2);
		break;
	case 7106:
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(270);
		break;
	default:
		break;
	}
}

void Scene710::dispatch() {
	if ((_kid._position.x > 0) && (_dogLying) && (_sceneMode != 7106)) {
		_dogLying = false;
		_sceneMode = 7103;
		setAction(&_sequenceManager1, this, 7103, &_kid, &_stick, &_dog, NULL);
	}
	SceneExt::dispatch();
}

void Scene710::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_dogLying);
	s.syncAsSint16LE(_stickThrowCount);
	s.syncAsSint16LE(_watchCrate);
	s.syncAsSint16LE(_throwStick);
}


} // End of namespace BlueForce
} // End of namespace TsAGE
