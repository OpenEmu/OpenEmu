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

#include "tsage/blue_force/blueforce_scenes2.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 200 - Credits - Motorcycle Training
 *
 *--------------------------------------------------------------------------*/

void Scene200::Action1::signal() {
	Scene200 *scene = (Scene200 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	assert(owner);

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1:
		BF_GLOBALS._scenePalette.loadPalette(235);
		BF_GLOBALS._scenePalette.refresh();
		_state = 0;
		setDelay(30);
		break;
	case 2:
		assert(_owner);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 3:
		if (++_state < 2) {
			scene->_action2.signal();
			owner->setFrame(1);
			_actionIndex = 2;
		}
		setDelay(2);
		break;
	case 4: {
		PaletteRotation *rot;
		rot = BF_GLOBALS._scenePalette.addRotation(64, 79, 1);
		rot->setDelay(10);
		rot = BF_GLOBALS._scenePalette.addRotation(96, 111, 1);
		rot->setDelay(10);

		scene->setAction(&scene->_sequenceManager, this, 201, &scene->_object1, &scene->_object2,
			&scene->_object3, &scene->_object4, &scene->_object5, &scene->_object6, NULL);
		break;
	}
	case 5:
		BF_GLOBALS._sceneManager.changeScene(210);
		break;
	default:
		break;
	}
}

void Scene200::Action2::signal() {
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);
	assert(owner);

	switch (_actionIndex++) {
	case 1:
		owner->setPosition(owner->_position, 0);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setPosition(owner->_position, 1000);
		owner->setFrame(1);
		_actionIndex = 0;
		setDelay(1);
		break;
	default:
		break;
	}
}


/*--------------------------------------------------------------------------*/

void Scene200::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(200);
	setZoomPercents(0, 100, 200, 100);
	BF_GLOBALS._sound1.play(3);

	_object10.postInit();
	_object10.setVisage(200);
	_object10.setPosition(Common::Point(114, 102));
	_object10.setStrip(2);
	_object10.setFrame(1);
	_object10.changeZoom(100);

	_object1.postInit();
	_object1.hide();
	_object2.postInit();
	_object2.hide();
	_object3.postInit();
	_object3.hide();
	_object4.postInit();
	_object4.hide();
	_object5.postInit();
	_object5.hide();
	_object6.postInit();
	_object6.hide();

	_object11.postInit();
	_object11.setVisage(200);
	_object11.setPosition(Common::Point(96, 112), 1000);
	_object11.setStrip(3);
	_object11.setFrame(1);
	_object11.changeZoom(100);

	_object10.setAction(&_action1);
	_object11.setAction(&_action2);
}

void Scene200::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	SceneExt::remove();
}

/*--------------------------------------------------------------------------
 * Scene 210 - Credits - Car Training
 *
 *--------------------------------------------------------------------------*/

void Scene210::Action1::signal() {
	Scene210 *scene = (Scene210 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(_owner);
	assert(owner);

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2: {
		PaletteRotation *rot;
		rot = BF_GLOBALS._scenePalette.addRotation(64, 79, 1);
		rot->setDelay(10);
		rot = BF_GLOBALS._scenePalette.addRotation(96, 111, 1);
		rot->setDelay(10);

		scene->setAction(&scene->_sequenceManager, this, 210, &scene->_object10, &scene->_object11,
			&scene->_object12, &scene->_object13, &scene->_object14, &scene->_object15, NULL);
		break;
	}
	case 3:
		BF_GLOBALS._sceneManager.changeScene(220);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene210::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(210);

// FIXME: This fixes an obvious glitch during scene transition.
//        Shouldn't it be included in the 2 previous functions?
	clearScreen();
//

	BF_GLOBALS._scenePalette.loadPalette(235);
	BF_GLOBALS._scenePalette.refresh();

	_object9.postInit();
	_object9.setVisage(210);
	_object9.setPosition(Common::Point(146, 151));
	_object9.setStrip(1);
	_object9.setFrame(1);
	_object9.changeZoom(100);
	_object9.setAction(&_action1);

	_object10.postInit();
	_object10.hide();
	_object11.postInit();
	_object11.hide();
	_object12.postInit();
	_object12.hide();
	_object13.postInit();
	_object13.hide();
	_object14.postInit();
	_object14.hide();
	_object15.postInit();
	_object15.hide();
}

void Scene210::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	SceneExt::remove();
}

/*--------------------------------------------------------------------------
 * Scene 220 - Credits - Martial Arts
 *
 *--------------------------------------------------------------------------*/

void Scene220::Action1::signal() {
	Scene220 *scene = (Scene220 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1:
		BF_GLOBALS._scenePalette.loadPalette(235);
		BF_GLOBALS._scenePalette.refresh();
		setDelay(30);
		break;
	case 2:
		BF_GLOBALS._scenePalette.clearListeners();
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_object2.setVisage(221);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(178, 122));
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 4:
		BF_GLOBALS._scenePalette.loadPalette(2);
		BF_GLOBALS._scenePalette.refresh();
		BF_GLOBALS._scenePalette.loadPalette(235);
		BF_GLOBALS._scenePalette.refresh();
		setDelay(5);
		break;
	case 5:
		scene->_object1.remove();

		scene->_object2.setVisage(222);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(164, 138));
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 6:
		scene->_object2.setVisage(223);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(164, 139));
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 7:
		scene->_object2.setVisage(230);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(168, 136));
		scene->_object2.animate(ANIM_MODE_4, 13, 1, this);
		break;
	case 8:
		scene->_object2.animate(ANIM_MODE_5, this);

		scene->_object3.postInit();
		scene->_object3.setVisage(231);
		scene->_object3.setPosition(Common::Point(65, 179));
		scene->_object3.setStrip(1);
		scene->_object3.setFrame(1);
		scene->_object3.changeZoom(100);
		scene->_object3.setAction(&scene->_action2, this);
		break;
	case 9:
		break;
	case 10:
		scene->_object2.setVisage(224);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(148, 143));
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 11: {
		PaletteRotation *rot;
		rot = BF_GLOBALS._scenePalette.addRotation(64, 79, 1);
		rot->setDelay(10);
		rot = BF_GLOBALS._scenePalette.addRotation(96, 111, 1);
		rot->setDelay(10);

		scene->setAction(&scene->_sequenceManager, this, 220, &scene->_object4, &scene->_object5,
			&scene->_object6, &scene->_object7, &scene->_object8, &scene->_object9, NULL);
		break;
	}
	case 12:
		scene->_object2.setVisage(232);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(146, 143));
		scene->_object2._numFrames = 5;
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 13:
		scene->_object2.setVisage(228);
		scene->_object2.setFrame(1);
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 14:
		scene->_object2.setVisage(229);
		scene->_object2.setFrame(1);
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 15:
		BF_GLOBALS._sceneManager.changeScene(225);
		break;
	default:
		break;
	}
}

void Scene220::Action2::signal() {
	Scene220 *scene = (Scene220 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object3.setPosition(scene->_object3._position);
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 1:
		setDelay(50);
		break;
	case 2:
		scene->_object3.remove();
		remove();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene220::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(220);

// FIXME: This fixes an obvious glitch during scene transition.
//        Shouldn't it be included in the 2 previous functions?
	clearScreen();
//

	_object2.postInit();
	_object2.setVisage(220);
	_object2.setPosition(Common::Point(182, 122));
	_object2.setStrip(1);
	_object2.setFrame(1);
	_object2.changeZoom(100);

	_object1.postInit();
	_object1.setVisage(220);
	_object1.setPosition(Common::Point(164, 138));
	_object1.setStrip(2);
	_object1.setFrame(1);
	_object1.changeZoom(100);

	_object4.postInit();
	_object4.hide();
	_object5.postInit();
	_object5.hide();
	_object6.postInit();
	_object6.hide();
	_object7.postInit();
	_object7.hide();
	_object8.postInit();
	_object8.hide();
	_object9.postInit();
	_object9.hide();

	_object2.setAction(&_action1);
}

void Scene220::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	SceneExt::remove();
}

/*--------------------------------------------------------------------------
 * Scene 225 - Credits - Gun Training
 *
 *--------------------------------------------------------------------------*/

void Scene225::Action1::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		BF_GLOBALS._scenePalette.loadPalette(235);
		BF_GLOBALS._scenePalette.refresh();
		setDelay(5);
		break;
	case 2:
		owner->animate(ANIM_MODE_4, 7, 1, this);
		break;
	case 3:
		scene->_object8.animate(ANIM_MODE_5, this);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 4:
		scene->_object8.remove();
		break;
	case 5:
		scene->_action3.signal();
		break;
	case 6:
		owner->setPosition(Common::Point(owner->_position.x, owner->_position.y - 4));
		owner->setStrip(2);
		owner->setFrame(1);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 7:
		scene->_action2.signal();
		break;
	case 8:
		owner->animate(ANIM_MODE_6, this);
		break;
	case 9:
		owner->setPosition(Common::Point(owner->_position.x - 2, owner->_position.y - 1));
		owner->setStrip(3);
		owner->setFrame(1);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 10: {
		owner->setPosition(Common::Point(owner->_position.x + 10, owner->_position.y + 4));
		owner->setStrip(4);
		owner->setFrame(1);
		owner->fixPriority(116);
		owner->animate(ANIM_MODE_1, NULL);

		Common::Point destPos(138, 117);
		NpcMover *mover = new NpcMover();
		owner->addMover(mover, &destPos, this);
		break;
	}
	case 11: {
		owner->setPosition(Common::Point(owner->_position.x - 12, owner->_position.y - 1));
		owner->setStrip(5);
		owner->setFrame(1);
		owner->_moveDiff.x = 8;

		Common::Point destPos(402, 116);
		NpcMover *mover2 = new NpcMover();
		owner->addMover(mover2, &destPos, this);

		BF_GLOBALS._player.setPosition(Common::Point(owner->_position.x, 0));
		ADD_MOVER_NULL(BF_GLOBALS._player, 500, 0);
		break;
	}
	case 12:
		owner->setVisage(1227);
		owner->setStrip(1);
		owner->setFrame(1);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 13:
		owner->setStrip(2);
		owner->setFrame(4);
		scene->_action4.signal();
		break;
	case 14:
		owner->animate(ANIM_MODE_6, this);
		break;
	case 15:
		scene->_action6.signal();
		break;
	case 16:
		owner->animate(ANIM_MODE_4, 4, 1, this);
		break;
	case 17:
		owner->setFrame(6);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 18:
		scene->_action5.signal();
		break;
	case 19:
		owner->animate(ANIM_MODE_4, 4, -1, this);
		break;
	case 20:
		PaletteRotation *rot;
		rot = BF_GLOBALS._scenePalette.addRotation(64, 79, 1);
		rot->setDelay(10);
		rot = BF_GLOBALS._scenePalette.addRotation(96, 111, 1);
		rot->setDelay(10);

		scene->setAction(&scene->_sequenceManager, this, 225, &scene->_object15, &scene->_object16,
			&scene->_object17, &scene->_object18, &scene->_object19, &scene->_object20, NULL);
		break;
	case 21:
		scene->_object21.hide();
		BF_GLOBALS._player._moveDiff.x = 5;
		BF_GLOBALS._sceneManager.changeScene(265);
		break;
	default:
		break;
	}
}

void Scene225::Action2::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setStrip(3);
		owner->setFrame(2);
		scene->_action1.signal();
		break;
	default:
		break;
	}
}

void Scene225::Action3::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setStrip(3);
		owner->setFrame(1);
		scene->_action1.signal();
		break;
	default:
		break;
	}
}

void Scene225::Action4::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setStrip(5);
		owner->setFrame(4);
		scene->_action1.signal();
		break;
	default:
		break;
	}
}

void Scene225::Action5::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setStrip(3);
		owner->setFrame(6);
		scene->_action1.signal();
		break;
	default:
		break;
	}
}

void Scene225::Action6::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setStrip(3);
		owner->setFrame(3);
		scene->_action1.signal();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene225::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(1225);
	loadBackground(-320, 0);

	_object8.postInit();
	_object8.setVisage(1225);
	_object8.setPosition(Common::Point(114, 119));
	_object8._frame = 1;
	_object8.setStrip(2);
	_object8.changeZoom(100);

	_object9.postInit();
	_object9.setVisage(1226);
	_object9.setPosition(Common::Point(83, 128));
	_object9.setStrip(1);
	_object9.changeZoom(100);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setPosition(Common::Point(83, 0));
	BF_GLOBALS._player.hide();

	_object10.postInit();
	_object10.setVisage(1225);
	_object10.setPosition(Common::Point(237, 29));
	_object10.setStrip(1);
	_object10._frame = 1;
	_object10.changeZoom(100);
	_object10._numFrames = 2;

	_object11.postInit();
	_object11.setVisage(1225);
	_object11.setPosition(Common::Point(290, 47));
	_object11.setStrip(1);
	_object11._frame = 1;
	_object11.changeZoom(100);
	_object11._numFrames = 2;

	_object12.postInit();
	_object12.setVisage(1225);
	_object12.setPosition(Common::Point(368, 35));
	_object12.setStrip(4);
	_object12._frame = 1;
	_object12.changeZoom(100);
	_object12._numFrames = 2;

	_object13.postInit();
	_object13.setVisage(1225);
	_object13.setPosition(Common::Point(416, 33));
	_object13.setStrip(1);
	_object13._frame = 1;
	_object13.changeZoom(100);
	_object13._numFrames = 2;

	_object14.postInit();
	_object14.setVisage(1225);
	_object14.setPosition(Common::Point(476, 30));
	_object14.setStrip(1);
	_object14._frame = 1;
	_object14.changeZoom(100);
	_object14._numFrames = 2;

	_object21.postInit();
	_object21.setVisage(235);
	_object21.setStrip(1);
	_object21._frame = 1;
	_object21.setPosition(Common::Point(498, 41));
	_object21.changeZoom(100);
	_object21.hide();

	_object15.postInit();
	_object15.hide();
	_object16.postInit();
	_object16.hide();
	_object17.postInit();
	_object17.hide();
	_object18.postInit();
	_object18.hide();
	_object19.postInit();
	_object19.hide();
	_object20.postInit();
	_object20.hide();

	_object9.setAction(&_action1);
	_object10.setAction(&_action2);
	_object11.setAction(&_action3);
	_object12.setAction(&_action4);
	_object13.setAction(&_action5);
	_object14.setAction(&_action6);
}

void Scene225::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	SceneExt::remove();
}

/*--------------------------------------------------------------------------
 * Scene 265 - Intro - Graduation Article
 *
 *--------------------------------------------------------------------------*/

void Scene265::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._scenePalette.loadPalette(2);
		setDelay(30);
		break;
	case 1:
		BF_GLOBALS._scenePalette.refresh();
		setDelay(240);
		break;
	case 2:
		ADD_MOVER(BF_GLOBALS._player, 160, 280);
		break;
	case 3:
		// Wait until sound finishes playing
		if (BF_GLOBALS._sound1.isPlaying())
			_actionIndex = 3;
		setDelay(1);
		break;
	case 4:
		BF_GLOBALS._dayNumber = 1;
		BF_GLOBALS._sceneManager.changeScene(190);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene265::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(265);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setPosition(Common::Point(160, 100));
	BF_GLOBALS._player.hide();
	setAction(&_action1);
}

void Scene265::remove() {
	clearScreen();
	SceneExt::remove();
}

/*--------------------------------------------------------------------------
 * Scene 270 - Living Room & Kitchen
 *
 *--------------------------------------------------------------------------*/

void Scene270::Action1::signal() {
	Scene270 *scene = (Scene270 *)BF_GLOBALS._sceneManager._scene;

	scene->setAction(&scene->_sequenceManager2, this, 2703, &scene->_tv, NULL);
}

/*--------------------------------------------------------------------------*/

bool Scene270::Lyle::startAction(CursorType action, Event &event) {
	Scene270 *scene = (Scene270 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		scene->_field21A0 = 1;
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 2706;

		if (scene->_field380 == 1) {
			scene->setAction(&scene->_sequenceManager1, scene, 2706, &BF_GLOBALS._player, &scene->_object2, NULL);
		} else {
			scene->signal();
		}
		return true;
	case INV_CRATE1:
		scene->_field21A0 = 2;
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 2706;

		if (scene->_field380 == 1) {
			scene->setAction(&scene->_sequenceManager1, scene, 2706, &BF_GLOBALS._player, NULL);
		} else {
			scene->signal();
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene270::Grandma::startAction(CursorType action, Event &event) {
	Scene270 *scene = (Scene270 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		switch (BF_GLOBALS._dayNumber) {
		case 1:
			// Day 1
			if (scene->_field380 == 1) {
				scene->_stripManager.start((scene->_grandma._position.x == 157) ? 2712 : 2723, &BF_GLOBALS._stripProxy);
			} else if (BF_GLOBALS._bookmark == bBookedFrankieEvidence) {
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 2710;
				scene->setAction(&scene->_sequenceManager1, scene, 2710, &BF_GLOBALS._player, &scene->_grandma, NULL);
			} else if (BF_GLOBALS.getFlag(onDuty) || (BF_INVENTORY.getObjectScene(INV_BASEBALL_CARD) == 2) ||
						(scene->_field386 != 0)) {
				scene->_stripManager.start(2723, &BF_GLOBALS._stripProxy);
			} else {
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 2715;
				scene->setAction(&scene->_sequenceManager1, scene, 2715, &BF_GLOBALS._player, &scene->_grandma, NULL);
			}
			return true;
		case 3:
			// Day 3
			if (scene->_field380 == 1) {
				scene->_stripManager.start(2712, &BF_GLOBALS._stripProxy);
			} else if (BF_GLOBALS.getFlag(fGotGreen355fTalkedToGrannyDay3)) {
				scene->_stripManager.start(2714, &BF_GLOBALS._stripProxy);
			} else {
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 2713;
				scene->setAction(&scene->_sequenceManager1, scene, 2713, &BF_GLOBALS._player, &scene->_grandma, NULL);
			}
			return true;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

bool Scene270::Item::startAction(CursorType action, Event &event) {
	Scene270 *scene = (Scene270 *)BF_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (scene->_field380 == 0)) {
		BF_GLOBALS._player.disableControl();
		scene->_object2.postInit();
		scene->_object2.hide();
		scene->_sceneMode = 2705;
		scene->setAction(&scene->_sequenceManager1, scene, 2705, &BF_GLOBALS._player, &scene->_object2, NULL);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene270::Exit::startAction(CursorType action, Event &event) {
	Scene270 *scene = (Scene270 *)BF_GLOBALS._sceneManager._scene;

	if (!_action && !scene->_field384 && !scene->_field386) {
		if (scene->_field380 == 1) {
			scene->_tempPos = Common::Point(320, 140);
			BF_GLOBALS._player.disableControl();

			scene->_sceneMode = 2706;
			scene->setAction(&scene->_sequenceManager1, scene, 2706, &BF_GLOBALS._player, &scene->_object2, NULL);
		} else {
			ADD_PLAYER_MOVER(320, 140);
		}
		return true;
	} else {
		return false;
	}
}

/*--------------------------------------------------------------------------*/

Scene270::Scene270(): SceneExt() {
	_field380 = _field382 =_field384 = _field386 = 0;
	_field219A = _tempPos.x = _tempPos.y = _field21A0 = 0;
	_sceneMode = 0;
}

void Scene270::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_field380);
	s.syncAsSint16LE(_field382);
	s.syncAsSint16LE(_field384);
	s.syncAsSint16LE(_field386);
	s.syncAsSint16LE(_field219A);
	s.syncAsSint16LE(_tempPos.x);
	s.syncAsSint16LE(_tempPos.y);
	s.syncAsSint16LE(_field21A0);
}

void Scene270::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(270);
	setZoomPercents(120, 90, 130, 100);

	if (BF_GLOBALS._sceneManager._previousScene != 560)
		BF_GLOBALS._sound1.fadeSound(26);

	_exit.setDetails(Rect(310, 115, 320, 167), 270, -1, -1, -1, 1, NULL);

	if (BF_GLOBALS._dayNumber == 0) {
		BF_GLOBALS._dayNumber = 1;
		BF_INVENTORY.setObjectScene(INV_BASEBALL_CARD, 2);
		BF_INVENTORY.setObjectScene(INV_CRATE1, 1);
		BF_GLOBALS._sceneManager._previousScene = 710;
	}

	if (((BF_GLOBALS._bookmark >= bLauraToParamedics) && (BF_GLOBALS._dayNumber == 1) &&
				(BF_INVENTORY.getObjectScene(INV_BASEBALL_CARD) != 2) &&
				(BF_INVENTORY.getObjectScene(INV_BASEBALL_CARD) != 1)) ||
			((BF_GLOBALS._dayNumber == 3) && BF_GLOBALS.getFlag(fGotGreen355fTalkedToGrannyDay3))) {
		BF_GLOBALS._walkRegions.disableRegion(6);
		BF_GLOBALS._walkRegions.disableRegion(14);
		BF_GLOBALS._walkRegions.disableRegion(19);

		_grandma.postInit();
		_grandma.setVisage(274);
		_grandma.setPosition(Common::Point(157, 132));
		_grandma._numFrames = 5;
		_grandma.animate(ANIM_MODE_2, NULL);
		_grandma.fixPriority(129);
	}

	if (BF_GLOBALS._bookmark == bTalkedToGrannyAboutSkipsCard) {
		_grandma.postInit();
	}

	if (BF_GLOBALS._sceneManager._previousScene == 710) {
		_skip.postInit();
		_laura.postInit();
		_lyle.postInit();
		_grandma.postInit();
	}

	_stripManager.addSpeaker(&_grandmaSpeaker);
	_stripManager.addSpeaker(&_lyleSpeaker);
	_stripManager.addSpeaker(&_jakeSpeaker);
	_stripManager.addSpeaker(&_skipSpeaker);
	_stripManager.addSpeaker(&_lauraSpeaker);
	_stripManager.addSpeaker(&_gameTextSpeaker);

	_tv.postInit();
	_tv.setVisage(270);
	_tv.setPosition(Common::Point(264, 74));
	_tv.setStrip(5);
	_tv.fixPriority(132);
	_tv._numFrames = 3;
	_tv.setAction(&_action1);

	_fireplace.postInit();
	_fireplace.setVisage(270);
	_fireplace.setStrip(2);
	_fireplace.setPosition(Common::Point(302, 121));
	_fireplace.fixPriority(132);
	_fireplace.animate(ANIM_MODE_2, NULL);

	_fridge.setDetails(Rect(0, 56, 56, 130), 270, 9, 10, 11, 1, NULL);
	_object3.setDetails(270, 12, 13, 14, 1, (SceneItem *)NULL);
	_laura.setDetails(270, 15, -1, -1, 1, (SceneItem *)NULL);
	_skip.setDetails(270, 14, -1, -1, 1, (SceneItem *)NULL);
	_lyle.setDetails(270, 34, 35, 36, 1, (SceneItem *)NULL);
	_tv.setDetails(270, 3, 4, 5, 1, (SceneItem *)NULL);
	_fireplace.setDetails(270, 6, 7, 8, 1, (SceneItem *)NULL);

	if ((BF_GLOBALS._sceneManager._previousScene == 710) && (BF_GLOBALS._bookmark == bTalkedToGrannyAboutSkipsCard)) {
		_grandma.setDetails(270, 15, 16, 17, 1, (SceneItem *)NULL);
	} else {
		_grandma.setDetails(270, 40, 16, 17, 1, (SceneItem *)NULL);
	}

	_afgan.setDetails(4, 270, 27, 28, 29, 1);
	_couch.setDetails(1, 270, 18, 19, 20, 1);
	_photos.setDetails(Rect(278, 50, 318, 72), 270, 21, 22, 23, 1, NULL);
	_appliances.setDetails(3, 270, 24, 25, 26, 1);
	_ivy.setDetails(2, 270, 30, 31, 32, 1);
	_background.setDetails(Rect(0, 0, 320, 168), 270, 0, 1, 2, 1, NULL);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player._moveDiff.x = 8;
	BF_GLOBALS._player.changeZoom(-1);
	BF_GLOBALS._player.disableControl();

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 560:
		if (BF_GLOBALS._bookmark == bTalkedToGrannyAboutSkipsCard) {
			_field219A = 1;
			BF_GLOBALS._player._moveDiff.x = 5;
			_field386 = 0;

			_grandma.animate(ANIM_MODE_1, NULL);
			setAction(&_sequenceManager1, NULL, 2720, &BF_GLOBALS._player, &_grandma, NULL);
			BF_GLOBALS._bookmark = bLyleStoppedBy;
		} else {
			_sceneMode = 2700;
			setAction(&_sequenceManager1, this, 2700, &BF_GLOBALS._player, NULL);
		}
		break;
	case 690:
		BF_GLOBALS._player.setPosition(Common::Point(-13, 162));
		_sceneMode = 2702;
		setAction(&_sequenceManager1, this, 2702, &BF_GLOBALS._player, NULL);
		break;
	case 710:
		BF_GLOBALS._player._moveDiff.x = 6;
		_sceneMode = 2717;
		setAction(&_sequenceManager1, this, 2717, &BF_GLOBALS._player, &_laura, &_skip, &_lyle, &_grandma, NULL);
		break;
	default:
		_sceneMode = 2701;
		setAction(&_sequenceManager1, this, 2701, &BF_GLOBALS._player, NULL);
		break;
	}
}

void Scene270::signal() {
	switch (_sceneMode) {
	case 10:
		_sceneMode = 2702;
		setAction(&_sequenceManager1, this, 2702, &BF_GLOBALS._player, NULL);
		break;
	case 11:
		BF_GLOBALS._player._strip = 8;
		BF_GLOBALS._player._frame = 1;

		if (_field382) {
			_sceneMode = 2719;
			_stripManager.start(2720, this);
		} else {
			_field382 = 1;
			_sceneMode = 13;
			_stripManager.start(2718, this);
		}
		break;
	case 12:
		BF_GLOBALS._player._strip = 8;
		BF_GLOBALS._player._frame = 1;
		_sceneMode = 13;
		_stripManager.start(2719, this);
		break;
	case 13:
	case 2713:
	case 2715:
		BF_GLOBALS._player.enableControl();
		break;
	case 2700:
		_field219A = 1;
		BF_GLOBALS._player._strip = 6;
		BF_GLOBALS._player.enableControl();
		break;
	case 2701:
		BF_GLOBALS._player._strip = 2;
		BF_GLOBALS._player.enableControl();
		_field219A = 1;
		break;
	case 2702:
		BF_GLOBALS._player._strip = 1;
		BF_GLOBALS._player.enableControl();
		_field219A = 1;
		break;
	case 2705:
		_field380 = 1;
		BF_GLOBALS._player.enableControl();
		break;
	case 2706:
		BF_GLOBALS._player.changeZoom(-1);
		_object2.remove();
		_field380 = 0;

		switch (_field21A0) {
		case 1:
			_sceneMode = 11;
			ADD_PLAYER_MOVER(192, 135);
			break;
		case 2:
			if (BF_GLOBALS.getFlag(shownLyleCrate1Day1)) {
				SceneItem::display2(270, 37);
				BF_GLOBALS._player.enableControl();
			} else {
				T2_GLOBALS._uiElements.addScore(30);
				BF_GLOBALS.setFlag(shownLyleCrate1Day1);
				_sceneMode = 12;
				ADD_PLAYER_MOVER(192, 135);
			}
			break;
		default:
			BF_GLOBALS._player.enableControl();
			ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, _tempPos.x, _tempPos.y);
			break;
		}

		_field21A0 = 0;
		break;
	case 2710:
		BF_GLOBALS._bookmark = bEndOfWorkDayOne;
		BF_GLOBALS._player.enableControl();
		break;
	case 2711:
		BF_GLOBALS._player.setPosition(Common::Point(150, 300));
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(180);
		BF_GLOBALS._bookmark = bLyleStoppedBy;
		break;
	case 2712:
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(180);
		break;
	case 2714:
		BF_GLOBALS._sceneManager.changeScene(560);
		break;
	case 2717:
		_sceneMode = 2718;
		_lyle.setFrame2(-1);
		setAction(&_sequenceManager1, this, 2718, &BF_GLOBALS._player, &_laura, &_skip,
			&_lyle, &_grandma, NULL);
		break;
	case 2718:
		BF_GLOBALS._walkRegions.disableRegion(6);
		BF_GLOBALS._walkRegions.disableRegion(14);
		BF_GLOBALS._walkRegions.disableRegion(19);

		_field219A = 1;
		BF_GLOBALS._bookmark = bTalkedToGrannyAboutSkipsCard;

		_grandma.setStrip(8);
		_grandma._frame = 5;
		_field384 = 1;
		_field386 = 1;

		BF_GLOBALS._player._moveDiff.x = 8;
		BF_GLOBALS._player.enableControl();
		break;
	case 2719:
		_sceneMode = 13;
		_field384 = 0;
		BF_GLOBALS._player._moveDiff.x = 6;

		_lyle.setFrame2(-1);
		setAction(&_sequenceManager1, this, 2719, &BF_GLOBALS._player, &_lyle, &_grandma, NULL);
		break;
	default:
		break;
	}
}

void Scene270::process(Event &event) {
	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == CURSOR_WALK) &&
			(_field380 == 1) && !_action) {
		_tempPos = event.mousePos;
		BF_GLOBALS._player.disableControl();
		_sceneMode = 2706;
		setAction(&_sequenceManager1, this, 2706, &BF_GLOBALS._player, &_object2, NULL);
		event.handled = true;
	}

	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1)) &&
			!_field384 && !_field386) {
		// Check if the cursor is on an exit
		if (_exit.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_E);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			// In case an exit cursor was being shown, restore the previously selected cursor
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene270::dispatch() {
	if (_field384) {
		_lyle.updateAngle(BF_GLOBALS._player._position);

		if (_lyle._angle < 110)
			_lyle.setFrame2(4);
		else if (_lyle._angle < 180)
			_lyle.setFrame2(3);
		else if (_lyle._angle < 250)
			_lyle.setFrame2(2);
		else
			_lyle.setFrame2(1);
	}

	if (_field386) {
		if (BF_GLOBALS._player._position.x > 290) {
			_grandma.setFrame(6);
		} else if (BF_GLOBALS._player._position.x > 274) {
			_grandma.setFrame(5);
		} else if (BF_GLOBALS._player._position.x > 258) {
			_grandma.setFrame(4);
		} else if (BF_GLOBALS._player._position.x > 242) {
			_grandma.setFrame(3);
		} else if (BF_GLOBALS._player._position.x > 226) {
			_grandma.setFrame(2);
		} else if (BF_GLOBALS._player._position.x > 210) {
			if ((_grandma._animateMode == ANIM_MODE_NONE) && (_grandma._frame > 1))
				_grandma.animate(ANIM_MODE_6, NULL);
		} else {
			if ((_grandma._animateMode == ANIM_MODE_NONE) && (_grandma._frame < 3))
				_grandma.animate(ANIM_MODE_4, 3, 1, NULL);
		}
	}

	if (!_action && _field219A) {
		if ((BF_GLOBALS._player._position.x > 236) && (BF_GLOBALS._player._position.y < 125)) {
			_field219A = 0;
			BF_GLOBALS._player.disableControl();
			if (!_field384) {
				BF_GLOBALS._sceneManager.changeScene(560);
			} else {
				BF_GLOBALS._player.addMover(NULL);
				SceneItem::display2(270, 38);
				_sceneMode = 2700;
				ADD_PLAYER_MOVER(BF_GLOBALS._player._position.x - 10, BF_GLOBALS._player._position.y + 15);
			}
		}

		if (BF_GLOBALS._player._position.x <= 20) {
			_field219A = 0;
			BF_GLOBALS._player.disableControl();
			BF_GLOBALS._player.addMover(NULL);
			BF_GLOBALS._player._strip = 3;
			BF_GLOBALS._player._frame = 1;

			if (BF_GLOBALS._sceneObjects->contains(&_grandma)) {
				_sceneMode = 10;
				_stripManager.start(2711, this);
			} else {
				SceneItem::display2(270, 33);
				_sceneMode = 2702;
				setAction(&_sequenceManager1, this, 2702, &BF_GLOBALS._player, NULL);
			}
		}

		if (BF_GLOBALS._player._position.x > 310) {
			_field219A = 0;
			BF_GLOBALS._player.disableControl();
			if (!_field384 && !_field386) {
				_sceneMode = 2712;
				setAction(&_sequenceManager1, this, 2712, &BF_GLOBALS._player, NULL);
			} else {
				BF_GLOBALS._player.addMover(NULL);
				BF_GLOBALS._player._strip = 2;
				BF_GLOBALS._player._frame = 1;
				SceneItem::display2(270, !_field384 ? 39 : 38);
				_sceneMode = 2701;

				ADD_PLAYER_MOVER(BF_GLOBALS._player._position.x - 10, BF_GLOBALS._player._position.y);
			}
		}
	}

	SceneExt::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 271 - Living Room & Kitchen #2
 *
 *--------------------------------------------------------------------------*/

void Scene271::Action1::signal() {
	Scene271 *scene = (Scene271 *)BF_GLOBALS._sceneManager._scene;

	setAction(&scene->_sequenceManager2, this, 2703, &scene->_tv, NULL);
}

/*--------------------------------------------------------------------------*/

bool Scene271::Object12::startAction(CursorType action, Event &event) {
	Scene271 *scene = (Scene271 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if (BF_GLOBALS._dayNumber == 1) {
			if (!BF_GLOBALS.getFlag(onDuty) && (BF_INVENTORY.getObjectScene(INV_BASEBALL_CARD) != 2)) {
				scene->_sceneMode = 2715;
				scene->setAction(&scene->_sequenceManager1, scene, 2715, &BF_GLOBALS._player, NULL);
				return true;
			}
		} else if (BF_GLOBALS._dayNumber == 3) {
			if (scene->_field796 == 1) {
				scene->_stripManager.start(2712, &BF_GLOBALS._stripProxy);
				return true;
			} else if (BF_GLOBALS.getFlag(fGotGreen355fTalkedToGrannyDay3)) {
				scene->_stripManager.start(2714, &BF_GLOBALS._stripProxy);
				return true;
			} else {
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 2713;
				scene->setAction(&scene->_sequenceManager1, scene, 2713, &BF_GLOBALS._player, &scene->_object12, NULL);
				BF_GLOBALS.setFlag(fGotGreen355fTalkedToGrannyDay3);
				return true;
			}
		}
		break;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

bool Scene271::Item::startAction(CursorType action, Event &event) {
	Scene271 *scene = (Scene271 *)BF_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && !scene->_field796) {
		BF_GLOBALS._player.disableControl();
		scene->_object1.postInit();
		scene->_object1.hide();

		scene->_sceneMode = 2705;
		scene->setAction(&scene->_sequenceManager1, scene, 2705, &BF_GLOBALS._player, &scene->_object1, NULL);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene271::Exit::startAction(CursorType action, Event &event) {
	Scene271 *scene = (Scene271 *)BF_GLOBALS._sceneManager._scene;

	if (!scene->_action) {
		if (scene->_field796 == 1) {
			scene->_tempPos = Common::Point(320, 140);
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 2706;
			scene->setAction(&scene->_sequenceManager1, scene, 2706, &BF_GLOBALS._player, &scene->_object1, NULL);
		} else {
			ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 320, 140);
		}
	}

	return true;
}

/*--------------------------------------------------------------------------*/

Scene271::Scene271() {
	_field796 = _field2E16 = 0;
	_tempPos.x = _tempPos.y = 0;
	_rect1 = Rect(236, 120, 266, 130);
}

void Scene271::synchronize(Serializer &s) {
	PalettedScene::synchronize(s);

	s.syncAsSint16LE(_field796);
	s.syncAsSint16LE(_field2E16);
	s.syncAsSint16LE(_tempPos.x);
	s.syncAsSint16LE(_tempPos.y);
	_rect1.synchronize(s);
}

void Scene271::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit();
	loadScene(270);
	setZoomPercents(120, 80, 140, 100);
	BF_GLOBALS._sound1.fadeSound(26);

	_stripManager.addSpeaker(&_grandmaSpeaker);
	_stripManager.addSpeaker(&_lyleSpeaker);
	_stripManager.addSpeaker(&_jakeSpeaker);
	_stripManager.addSpeaker(&_skipSpeaker);
	_stripManager.addSpeaker(&_lauraSpeaker);
	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_granTextSpeaker);
	_stripManager.addSpeaker(&_lyleTextSpeaker);

	_exit.setDetails(Rect(310, 115, 320, 167), 270, -1, -1, -1, 1, NULL);

	_tv.postInit();
	_tv.setVisage(270);
	_tv.setPosition(Common::Point(264, 74));
	_tv.setStrip(5);
	_tv.fixPriority(132);
	_tv._numFrames = 3;
	_tv.setAction(&_action1);

	if ((BF_GLOBALS._sceneManager._previousScene != 280) && (BF_GLOBALS._sceneManager._previousScene != 620)) {
		_object10.postInit();
		_object10.setVisage(270);
		_object10.setStrip(2);
		_object10.setPosition(Common::Point(302, 121));
		_object10.fixPriority(132);
		_object10.animate(ANIM_MODE_2, NULL);
	}

	_object5.postInit();
	_object5.hide();

	_item5.setDetails(Rect(0, 56, 56, 130), 270, 9, 10, 11, 1, NULL);
	_object6.setDetails(270, 12, 13, 14, 1, (SceneItem *)NULL);
	_object7.setDetails(270, 15, -1, -1, 1, (SceneItem *)NULL);
	_object8.setDetails(270, 14, -1, -1, 1, (SceneItem *)NULL);
	_object11.setDetails(270, -1, -1, -1, 1, (SceneItem *)NULL);
	_tv.setDetails(270, 3, 4, 5, 1, (SceneItem *)NULL);
	_object10.setDetails(270, 6, 7, 8, 1, (SceneItem *)NULL);
	_object12.setDetails(270, 15, 16, 17, 1, (SceneItem *)NULL);
	_item3.setDetails(4, 270, 27, 28, 29, 1);
	_item1.setDetails(1, 270, 18, 19, 20, 1);
	_item6.setDetails(Rect(278, 50, 318, 72), 270, 21, 22, 23, 1, NULL);
	_item2.setDetails(3, 270, 24, 25, 26, 1);
	_item4.setDetails(2, 270, 30, 31, 32, 1);
	_item11.setDetails(Rect(0, 0, SCREEN_WIDTH, UI_INTERFACE_Y), 270, 0, 1, 2, 1, NULL);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player._moveDiff.x = 8;
	BF_GLOBALS._player.changeZoom(-1);
	BF_GLOBALS._player.disableControl();

	_field796 = 0;
	_sceneMode = 0;
	_field2E16 = 0;

	if (BF_GLOBALS._dayNumber == 0) {
		BF_GLOBALS._dayNumber = 1;
		BF_GLOBALS._sceneManager.changeScene(180);
	}

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 180:
		BF_GLOBALS._walkRegions.disableRegion(6);
		BF_GLOBALS._walkRegions.disableRegion(14);
		BF_GLOBALS._walkRegions.disableRegion(19);

		BF_GLOBALS._player.setVisage(275);
		BF_GLOBALS._player.setPosition(Common::Point(348, 151));

		_object12.postInit();
		_object12.setVisage(274);
		_object12.setPosition(Common::Point(157, 132));
		_object12._numFrames = 5;
		_object12.animate(ANIM_MODE_2, NULL);
		break;
	case 280:
		BF_GLOBALS._player.setVisage(271);
		BF_GLOBALS._player.setStrip(5);
		BF_GLOBALS._player._frame = 6;
		BF_GLOBALS._player.setPosition(Common::Point(228, 138));

		_object1.postInit();
		_object1.setPosition(Common::Point(340, 100));

		_object11.postInit();
		_object11.setVisage(272);
		_object11.setStrip(1);
		_object11._frame = 2;
		_object11.setPosition(Common::Point(35, 136));

		_object6.postInit();
		_object6.hide();

		BF_GLOBALS._walkRegions.disableRegion(6);
		BF_GLOBALS._walkRegions.disableRegion(14);
		BF_GLOBALS._walkRegions.disableRegion(19);

		_object12.postInit();
		_object12.setVisage(274);
		_object12.setPosition(Common::Point(157, 132));
		_object12.animate(ANIM_MODE_2, NULL);
		_object12._numFrames = 5;
		_object12.fixPriority(120);
		_field796 = 1;
		break;
	case 590:
		BF_GLOBALS._player.setVisage(275);
		BF_GLOBALS._player.setStrip(5);
		BF_GLOBALS._player.setPosition(Common::Point(58, 133));
		BF_GLOBALS._player.changeZoom(-1);

		_object8.postInit();
		_object8.setVisage(279);
		_object8.setPosition(Common::Point(87, 127));
		_object8.fixPriority(146);

		_object7.postInit();
		_object7.setVisage(277);
		_object7.setStrip(7);
		_object7.setPosition(Common::Point(48, 149));

		BF_GLOBALS._walkRegions.disableRegion(6);
		BF_GLOBALS._walkRegions.disableRegion(14);
		BF_GLOBALS._walkRegions.disableRegion(19);

		_object12.postInit();
		_object12.setVisage(276);
		_object12.setPosition(Common::Point(129, 130));

		_object2.postInit();
		_object2.setVisage(270);
		_object2.setStrip(3);
		_object2.setFrame(2);
		_object2.setPosition(Common::Point(62, 101));
		_object2.fixPriority(145);

		_object3.postInit();
		_object3.setVisage(270);
		_object3.setStrip(3);
		_object3.setFrame(3);
		_object3.setPosition(Common::Point(90, 104));
		_object3.fixPriority(132);

		_object4.postInit();
		_object4.setVisage(270);
		_object4.setStrip(3);
		_object4.setFrame(4);
		_object4.setPosition(Common::Point(132, 87));
		_object4.fixPriority(1);
		break;
	default:
		BF_GLOBALS._player.setVisage(271);
		BF_GLOBALS._player.setFrame(1);
		BF_GLOBALS._player.setPosition(Common::Point(239, 145));

		BF_GLOBALS._walkRegions.disableRegion(6);
		BF_GLOBALS._walkRegions.disableRegion(14);
		BF_GLOBALS._walkRegions.disableRegion(19);

		_object12.postInit();
		_object12.setVisage(274);
		_object12.setPosition(Common::Point(157, 132));
		_object12.animate(ANIM_MODE_2, NULL);
		_object12._numFrames = 5;

		_object1.postInit();
		_object1.setVisage(271);
		_object1.setStrip(4);
		_object1.setPosition(Common::Point(220, 117));
		_object1.fixPriority(145);
		break;
	}

	_sceneMode = 11;

	static uint32 black = 0;
	add2Faders((const byte *)&black, 2, 270, this);
}

void Scene271::signal() {
	static uint32 black = 0;

	switch (_sceneMode) {
	case 10:
		_sceneMode = 2702;
		setAction(&_sequenceManager1, this, 2702, &BF_GLOBALS._player, NULL);
		break;
	case 11:
		switch (BF_GLOBALS._sceneManager._previousScene) {
		case 180:
			_sceneMode = 2716;
			setAction(&_sequenceManager1, this, 2716, &BF_GLOBALS._player, &_object12, NULL);
			break;
		case 280:
			BF_GLOBALS._dayNumber = 3;
			BF_INVENTORY.alterInventory(3);

			_sceneMode = 2707;
			setAction(&_sequenceManager1, this, 2707, &BF_GLOBALS._player, &_object1, &_object11, &_object5, &_object6, NULL);
			break;
		case 590:
			_sceneMode = 2704;
			setAction(&_sequenceManager1, this, 2704, &BF_GLOBALS._player, &_object12, &_object7, &_object8, NULL);
			break;
		default:
			_object11.postInit();
			_object11.setPosition(Common::Point(340, 100));
			BF_GLOBALS._sound1.play(36);

			_sceneMode = 2709;
			setAction(&_sequenceManager1, this, 2709, &BF_GLOBALS._player, &_object1, &_object12, &_object11, NULL);
			break;
		}
		break;
	case 12:
		BF_GLOBALS._v51C44 = 0;
		BF_GLOBALS._sound1.changeSound(67);
		BF_GLOBALS._sceneManager.changeScene(280);
		break;
	case 13:
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 2702:
		BF_GLOBALS._player._strip = 1;
		BF_GLOBALS._player.enableControl();
		_field2E16 = 1;
		break;
	case 2704:
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(690);
		break;
	case 2705:
		_field796 = 1;
		BF_GLOBALS._player.enableControl();
		break;
	case 2706:
		BF_GLOBALS._player.changeZoom(-1);
		BF_GLOBALS._player.enableControl();

		_object1.remove();
		_field796 = 0;
		ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, _tempPos.x, _tempPos.y);
		break;
	case 2707:
		BF_GLOBALS._player.enableControl();
		_field796 = 1;
		_field2E16 = 1;

		_object1.remove();
		_object11.remove();

		BF_INVENTORY.setObjectScene(INV_LYLE_CARD, 1);
		break;
	case 2709:
		BF_GLOBALS._sound1.play(68);
		_sceneMode = 12;
		addFader((const byte *)&black, 2, this);
		break;
	case 2712:
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(180);
		break;
	case 2713:
		BF_GLOBALS._player.enableControl();
		break;
	case 2714:
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sceneManager.changeScene(560);
		break;
	case 2715:
		break;
	case 2716:
		BF_GLOBALS._deathReason = 24;
		_sceneMode = 13;
		addFader((const byte *)&black, 2, this);
		break;
	default:
		break;
	}
}

void Scene271::process(Event &event) {
	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == CURSOR_WALK) &&
			(_field796 == 1)  && (!_action)) {
		_tempPos = event.mousePos;
		BF_GLOBALS._player.disableControl();

		_sceneMode = 2706;
		setAction(&_sequenceManager1, this, 2706, &BF_GLOBALS._player, &_object1, NULL);
		event.handled = true;
	}

	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < UI_INTERFACE_Y)) {
		if (_exit.contains(event.mousePos)) {
			GfxSurface cursor = _cursorVisage.getFrame(EXITFRAME_E);
			BF_GLOBALS._events.setCursor(cursor);
		} else {
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene271::dispatch() {
	if (!_action && (_field2E16 == 1)) {
		if ((BF_GLOBALS._player._position.x > 236) && (BF_GLOBALS._player._position.y < 120)) {
			_field2E16 = 0;
			BF_GLOBALS._sceneManager.changeScene(560);
		}

		if (BF_GLOBALS._player._position.x <= 20) {
			_field2E16 = 0;
			BF_GLOBALS._player.disableControl();
			BF_GLOBALS._player._mover->remove();
			BF_GLOBALS._player._strip = 3;
			BF_GLOBALS._player._frame = 1;

			if (BF_GLOBALS._sceneObjects->contains(&_object12)) {
				_sceneMode = 10;
				_stripManager.start(2711, this);
			} else {
				SceneItem::display2(270, 33);
				_sceneMode = 2702;
				setAction(&_sequenceManager1, this, 2702, &BF_GLOBALS._player, NULL);
			}
		}

		if (BF_GLOBALS._player._position.x >= 300) {
			_field2E16 = 0;
			BF_GLOBALS._player.disableControl();
			_sceneMode = 2712;
			setAction(&_sequenceManager1, this, 2712, &BF_GLOBALS._player, NULL);
		}
	}

	SceneExt::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 280 - Bedroom Flashback cut-scene
 *
 *--------------------------------------------------------------------------*/

void Scene280::Action1::signal() {
	Scene280 *scene = (Scene280 *)BF_GLOBALS._sceneManager._scene;
	static uint32 black = 0;

	switch (_actionIndex++) {
	case 0:
		scene->_jake.postInit();
		scene->_jake.setVisage(283);
		scene->_jake.setPosition(Common::Point(331, 200));
		scene->_jake.animate(ANIM_MODE_1, NULL);
		scene->_jake.setStrip(1);
		ADD_MOVER(scene->_jake, 189, 131);
		break;
	case 1:
		scene->_jake.setStrip(2);
		scene->_jake.setFrame(1);
		scene->_jake.animate(ANIM_MODE_8, NULL);
		scene->_jake._numFrames = 5;

		scene->_stripManager.start(2800, this);
		break;
	case 2:
		scene->_jake.animate(ANIM_MODE_5, NULL);
		scene->_dad.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_jake.setStrip(4);
		scene->_jake.setFrame(1);
		scene->_dad.setStrip(2);
		scene->_jake.setFrame(1);
		scene->_dad.animate(ANIM_MODE_5, this);
		break;
	case 4:
		scene->_dad.setStrip(3);
		scene->_dad.setFrame(1);
		scene->_dad.animate(ANIM_MODE_5, this);
		break;
	case 5:
		scene->_object4.hide();
		scene->_dad.setVisage(282);
		scene->_dad.setStrip(1);
		scene->_dad.setFrame(1);
		scene->_dad._numFrames = 5;
		scene->_dad.animate(ANIM_MODE_5, this);
		break;
	case 6:
		scene->_stripManager.start(2801, this);
		break;
	case 7:
		scene->_mum.postInit();
		scene->_mum.setVisage(282);
		scene->_mum.setStrip(2);
		scene->_mum.setFrame(1);
		scene->_mum.fixPriority(1);
		scene->_mum.setPosition(Common::Point(160, 138));

		scene->_jake.setStrip(3);
		scene->_jake.setFrame(1);
		scene->_jake.animate(ANIM_MODE_5, this);

		scene->_dad._numFrames = 10;
		scene->_dad.setVisage(284);
		scene->_dad.setStrip(1);
		scene->_dad.fixPriority(-1);
		scene->_dad.setPosition(Common::Point(174, 136));
		scene->_dad.setFrame(1);
		scene->_dad.animate(ANIM_MODE_1, NULL);
		ADD_MOVER(scene->_dad, 438, 320);
		break;
	case 8:
		scene->_mum.animate(ANIM_MODE_4, 5, 1, this);
		break;
	case 9:
		scene->_sceneMode = 2;
		BF_GLOBALS._sound1.fadeOut2(NULL);
		scene->addFader((const byte *)&black, 2, scene);

		scene->_jake.remove();
		scene->_mum.animate(ANIM_MODE_5, NULL);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene280::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit();
	BF_GLOBALS._interfaceY = SCREEN_HEIGHT;
	loadScene(280);

	_stripManager.addSpeaker(&_gameTextSpeaker);

	_dad.postInit();
	_dad.setVisage(281);
	_dad.setPosition(Common::Point(160, 138));
	_dad.fixPriority(1);

	_object4.postInit();
	_object4.setVisage(280);
	_object4.setPosition(Common::Point(139, 141));

	const uint32 black = 0;
	add2Faders((const byte *)&black, 2, 280, this);
	_sceneMode = 1;
	setAction(&_action1);
}

void Scene280::signal() {
	if (_sceneMode == 2)
		BF_GLOBALS._sceneManager.changeScene(271);
}

} // End of namespace BlueForce

} // End of namespace TsAGE
