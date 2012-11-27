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

#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld2/ringworld2_scenes3.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 3100 -
 *
 *--------------------------------------------------------------------------*/
Scene3100::Scene3100() {
	_field412 = 0;
}

void Scene3100::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
}

bool Scene3100::Guard::startAction(CursorType action, Event &event) {
	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	Scene3100 *scene = (Scene3100 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 10;
	R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
	scene->_stripManager.start(606, scene);
	return true;
}

void Scene3100::postInit(SceneObjectList *OwnerList) {
	if (R2_GLOBALS._sceneManager._previousScene == 1000) {
		if (R2_GLOBALS._player._oldCharacterScene[1] == 3100) {
			loadScene(3101);
			R2_GLOBALS._uiElements._active = false;
		} else {
			loadScene(3100);
			g_globals->gfxManager()._bounds.moveTo(Common::Point(160, 0));
		}
	} else {
		loadScene(3100);
	}
	// Original was doing it twice in a row. Skipped.

	if (R2_GLOBALS._sceneManager._previousScene == 3255)
		R2_GLOBALS._uiElements._active = false;

	SceneExt::postInit();
	_stripManager.addSpeaker(&_guardSpeaker);

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._sceneManager._previousScene = 1000;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player._characterIndex = R2_QUINN;
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_item2.setDetails(Rect(212, 97, 320, 114), 3100, 3, -1, -1, 1, NULL);
	_item1.setDetails(Rect(0, 0, 480, 200), 3100, 0, -1, -1, 1, NULL);
	_field412 = 0;

	if (R2_GLOBALS._sceneManager._previousScene == 1000) {
		if (R2_GLOBALS._player._oldCharacterScene[1] == 3100) {
			_sceneMode = 3102;
			_actor3.postInit();
			_actor4.postInit();
			_actor5.postInit();
			R2_GLOBALS._sound1.play(274);
			_sound1.fadeSound(130);
			setAction(&_sequenceManager, this, 3102, &_actor1, &R2_GLOBALS._player, &_actor3, &_actor4, &_actor5, NULL);
		} else {
			_guard.postInit();
			_guard.setup(3110, 5, 1);
			_guard.changeZoom(50);
			_guard.setPosition(Common::Point(10, 149));
			_guard.setDetails(3100, 6, -1, -1, 2, (SceneItem *)NULL);

			_actor4.postInit();
			_actor4.setup(3103, 1, 1);
			_actor4.setPosition(Common::Point(278, 113));
			_actor4.setDetails(3100, 9, -1, -1, 2, (SceneItem *)NULL);
			_actor4.animate(ANIM_MODE_2, NULL);

			_field412 = 1;
			_actor1.setDetails(3100, 3, -1, -1, 2, (SceneItem *)NULL);
			R2_GLOBALS._sound1.play(243);
			R2_GLOBALS._sound2.play(130);
			_sceneMode = 3100;

			setAction(&_sequenceManager, this, 3100, &R2_GLOBALS._player, &_actor1, NULL);
		}
	} else if (R2_GLOBALS._sceneManager._previousScene == 3255) {
		_sceneMode = 3101;
		_actor2.postInit();
		_actor3.postInit();
		_field412 = 1;

		setAction(&_sequenceManager, this, 3101, &R2_GLOBALS._player, &_actor1, &_actor2, &_actor3, NULL);
	} else {
		_guard.postInit();
		_guard.setup(3110, 5, 1);
		_guard.changeZoom(50);
		_guard.setPosition(Common::Point(10, 149));
		_guard.setDetails(3100, 6, -1, -1, 2, (SceneItem *)NULL);

		_actor4.postInit();
		_actor4.setup(3103, 1, 1);
		_actor4.setPosition(Common::Point(278, 113));
		_actor4.setDetails(3100, 9, -1, -1, 2, (SceneItem *)NULL);
		_actor4.animate(ANIM_MODE_2, NULL);

		_actor1.postInit();
		_actor1.setup(3104, 4, 1);
		_actor1.setPosition(Common::Point(143, 104));
		_actor1.setDetails(3100, 3, -1, -1, 2, (SceneItem *)NULL);

		R2_GLOBALS._player.setup(3110, 3, 1);
		R2_GLOBALS._player.changeZoom(50);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(160, 150));
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);

		R2_GLOBALS._sound1.play(243);
	}

	R2_GLOBALS._player._oldCharacterScene[1] = 3100;
}

void Scene3100::remove() {
	R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
	R2_GLOBALS._sound1.fadeOut2(NULL);
	R2_GLOBALS._sound2.fadeOut2(NULL);
	_sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene3100::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	case 3100:
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	case 3101:
		R2_GLOBALS._sceneManager.changeScene(1000);
		break;
	case 3102:
		R2_GLOBALS._player._oldCharacterScene[1] = 1000;
		R2_GLOBALS._sceneManager.changeScene(1000);
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	}
}

void Scene3100::dispatch() {
	if ((_sceneMode == 3100) && (_field412 != 0) && (R2_GLOBALS._player._position.y == 104)) {
		_field412 = 0;
		R2_GLOBALS._sound2.fadeOut2(NULL);
	}

	if ((_sceneMode == 3101) && (_field412 != 0) && (R2_GLOBALS._player._position.y < 104)) {
		_field412 = 0;
		_sound1.fadeSound(130);
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3125 - Ghouls dormitory
 *
 *--------------------------------------------------------------------------*/
Scene3125::Scene3125() {
	_field412 = 0;
}

void Scene3125::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
}

bool Scene3125::Item1::startAction(CursorType action, Event &event) {
	Scene3125 *scene = (Scene3125 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_useLineNum != -1)
			SceneItem::display(_resNum, _useLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	case CURSOR_LOOK:
		if (_lookLineNum != -1)
			SceneItem::display(_resNum, _lookLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	case CURSOR_TALK:
		if (_talkLineNum != -1)
			SceneItem::display(_resNum, _talkLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	default:
		return scene->display(action, event);
		break;
	}

	return true;
}

bool Scene3125::Item2::startAction(CursorType action, Event &event) {
	Scene3125 *scene = (Scene3125 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3125;
		scene->setAction(&scene->_sequenceManager1, scene, 3125, &R2_GLOBALS._player, NULL);
		break;
	case CURSOR_LOOK:
		SceneItem::display(3125, 15, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	case CURSOR_TALK:
		SceneItem::display(3125, 13, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	default:
		return SceneHotspot::startAction(action, event);
		break;
	}

	return true;
}

bool Scene3125::Item3::startAction(CursorType action, Event &event) {
	Scene3125 *scene = (Scene3125 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_actor5.postInit();
		scene->_sceneMode = 3126;
		scene->setAction(&scene->_sequenceManager1, scene, 3126, &R2_GLOBALS._player, &scene->_actor2, &scene->_actor3, &scene->_actor4, &scene->_actor1, &scene->_actor5, NULL);
		break;
	case CURSOR_LOOK:
		SceneItem::display(3125, 9, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	case CURSOR_TALK:
		SceneItem::display(3125, 13, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	default:
		return SceneHotspot::startAction(action, event);
		break;
	}

	return true;
}

bool Scene3125::Actor1::startAction(CursorType action, Event &event) {
	Scene3125 *scene = (Scene3125 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 3176;
	scene->setAction(&scene->_sequenceManager1, scene, 3176, &R2_GLOBALS._player, &scene->_actor1, NULL);
	return true;
}

void Scene3125::postInit(SceneObjectList *OwnerList) {
	loadScene(3125);
	SceneExt::postInit();
	_field412 = 0;

	_actor1.postInit();
	_actor1.setup(3175, 1, 1);
	_actor1.setPosition(Common::Point(35, 72));
	_actor1.setDetails(3125, 12, 13, -1, 1, (SceneItem *)NULL);

	_actor2.postInit();
	_actor2.setup(3126, 4, 1);
	_actor2.setPosition(Common::Point(71, 110));
	_actor2._numFrames = 20;

	_actor3.postInit();
	_actor3.setup(3126, 1, 1);
	_actor3.setPosition(Common::Point(215, 62));
	_actor3.fixPriority(71);

	_actor4.postInit();
	_actor4.setup(3126, 1, 1);
	_actor4.setPosition(Common::Point(171, 160));
	_actor4.fixPriority(201);

	_item3.setDetails(12, 3125, 9, 13, -1);
	_item2.setDetails(11, 3125, 15, 13, -1);
	_item1.setDetails(Rect(0, 0, 320, 200), 3125, 0, 1, 2, 1, NULL);

	R2_GLOBALS._sound1.play(262);
	R2_GLOBALS._player.postInit();

	if (R2_GLOBALS._player._oldCharacterScene[3] == 3250) {
		_sceneMode = 3175;
		setAction(&_sequenceManager1, this, 3175, &R2_GLOBALS._player, &_actor1, NULL);
	} else {
		R2_GLOBALS._player.setup(30, 5, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(89, 76));
		R2_GLOBALS._player.enableControl();
	}
	R2_GLOBALS._player._oldCharacterScene[3] = 3125;
}

void Scene3125::signal() {
	switch (_sceneMode) {
	case 3125:
		SceneItem::display(3125, 3, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		_sceneMode = 3127;
		setAction(&_sequenceManager1, this, 3127, &R2_GLOBALS._player, NULL);
		break;
	case 3126:
		R2_GLOBALS.setFlag(79);
	// No break on purpose
	case 3176:
		R2_GLOBALS._sceneManager.changeScene(3250);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene3125::dispatch() {
	if ((_sceneMode == 3126) && (_actor2._frame == 2) && (_field412 == 0)) {
		_field412 = 1;
		R2_GLOBALS._sound1.play(265);
	}
	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3150 - Jail
 *
 *--------------------------------------------------------------------------*/
bool Scene3150::Item5::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_INVENTORY.getObjectScene(47) != 3150)
			return SceneHotspot::startAction(action, event);

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3154;
		scene->setAction(&scene->_sequenceManager, scene, 3154, &R2_GLOBALS._player, &scene->_actor3, NULL);
		return true;
	case R2_SUPERCONDUCTOR_WIRE:
		if ((R2_INVENTORY.getObjectScene(47) != 3150) && (R2_GLOBALS.getFlag(75))) {
			R2_GLOBALS._player.disableControl();
			scene->_actor3.postInit();
			scene->_actor3._effect = 3;
			scene->_actor3._shade = 5;
			scene->_sceneMode = 3155;
			scene->setAction(&scene->_sequenceManager, scene, 3155, &R2_GLOBALS._player, &scene->_actor3, NULL);
		} else {
			SceneItem::display(3150, 42, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		}
		return true;
	default:
		return SceneHotspot::startAction(action, event);
		break;
	}
}

bool Scene3150::Item6::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_PILLOW:
		R2_GLOBALS._player.disableControl();
		scene->_actor4.postInit();
		scene->_actor4._effect = 6;
		scene->_actor4._shade = 3;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3158;
		scene->setAction(&scene->_sequenceManager, scene, 3158, &R2_GLOBALS._player, &scene->_actor4, NULL);
		return true;
	case R2_FOOD_TRAY:
		if ((R2_INVENTORY.getObjectScene(47) != 3150) && (R2_INVENTORY.getObjectScene(40) == 3150) && (R2_GLOBALS.getFlag(75))) {
			scene->_actor5.postInit();
			scene->_actor5._effect = 6;
			scene->_actor5._shade = 3;
			scene->_actor5.setDetails(3150, 30, -1, -1, 2, (SceneItem *)NULL);

			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 3159;
			scene->setAction(&scene->_sequenceManager, scene, 3159, &R2_GLOBALS._player, &scene->_actor5, NULL);
		} else {
			SceneItem::display(3150, 42, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		}
	default:
		return SceneHotspot::startAction(action, event);
		break;
	}
}

bool Scene3150::Actor4::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_GLOBALS.getFlag(75))
			return SceneActor::startAction(action, event);

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3151;
		scene->setAction(&scene->_sequenceManager, scene, 3151, &R2_GLOBALS._player, &scene->_actor4, NULL);
		return true;
	case R2_FOOD_TRAY:
		return false;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

bool Scene3150::Actor5::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	if ((action != CURSOR_USE) || (R2_GLOBALS.getFlag(77)))
		return SceneActor::startAction(action ,event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 3157;
	scene->setAction(&scene->_sequenceManager, scene, 3157, &R2_GLOBALS._player, &scene->_actor5, NULL);
	return true;
}

bool Scene3150::Actor6::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		if (R2_GLOBALS.getFlag(75)) {
			if (R2_GLOBALS.getFlag(77)) {
				R2_GLOBALS._player.disableControl();
				if (R2_GLOBALS.getFlag(76)) {
					scene->_sceneMode = 3152;
					scene->setAction(&scene->_sequenceManager, scene, 3152, &R2_GLOBALS._player, NULL);
				} else {
					scene->_sceneMode = 3153;
					scene->setAction(&scene->_sequenceManager, scene, 3152, &R2_GLOBALS._player, &scene->_actor4, NULL);
				}
			} else {
				SceneItem::display(3150, 42, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
			}
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 3152;
			scene->setAction(&scene->_sequenceManager, scene, 3152, &R2_GLOBALS._player, NULL);
		}
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene3150::Actor7::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	if ((action == R2_LASER_HACKSAW) && (!R2_GLOBALS.getFlag(80))) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3160;
		scene->setAction(&scene->_sequenceManager, scene, 3160, &R2_GLOBALS._player, &scene->_actor7, NULL);
		return true;
	}

	return SceneActor::startAction(action, event);
}

void Scene3150::Exit1::changeScene() {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	g_globals->_events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(-20, 180);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene3150::Exit2::changeScene() {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	g_globals->_events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 12;

	scene->setAction(&scene->_sequenceManager, scene, 3163, &R2_GLOBALS._player, NULL);
}

void Scene3150::postInit(SceneObjectList *OwnerList) {
	loadScene(3150);
	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_INVENTORY.setObjectScene(35, 2000);
		R2_GLOBALS._player._oldCharacterScene[1] = 3100;
		R2_GLOBALS._player._oldCharacterScene[3] = 0;
		R2_GLOBALS._player._characterIndex = R2_MIRANDA;
	}
	SceneExt::postInit();

	if (R2_GLOBALS.getFlag(78)) {
		_exit1.setDetails(Rect(0, 135, 60, 168), EXITCURSOR_SW, 3275);
		_exit1.setDest(Common::Point(70, 125));
	}

	if (R2_GLOBALS.getFlag(80)) {
		_exit2.setDetails(Rect(249, 36, 279, 60), EXITCURSOR_NE, 3150);
		_exit2.setDest(Common::Point(241, 106));
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();

	_actor2.postInit();
	_actor2.setPosition(Common::Point(64, 139));
	if (R2_GLOBALS.getFlag(78)) {
		_actor2.setup(3151, 1, 5);
		_actor2.fixPriority(125);
	} else {
		_actor2.setup(3151, 1, 1);
		_actor2.setDetails(3150, 8, -1, 9, 1, (SceneItem *)NULL);
	}

	if (R2_GLOBALS.getFlag(78)) {
		_actor1.postInit();
		_actor1.setup(3154, 1, 16);
		_actor1.setPosition(Common::Point(104, 129));
		_actor1._effect = 6;
		_actor1._shade = 3;
		_actor1.setDetails(3150, 24, -1, -1, -1, (SceneItem *)NULL);
	}

	_actor7.postInit();
	_actor7.setup(3154, 5, 1);
	if (R2_GLOBALS.getFlag(80))
		_actor7.setPosition(Common::Point(264, 108));
	else
		_actor7.setPosition(Common::Point(264, 58));
	_actor7.fixPriority(50);
	_actor7.setDetails(3150, 17, -1, 19, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(41) == 3150) {
		_actor4.postInit();
		if (R2_GLOBALS.getFlag(75)) {
			if (R2_GLOBALS.getFlag(76)) {
				R2_GLOBALS._walkRegions.enableRegion(1);
				R2_GLOBALS._walkRegions.enableRegion(4);
				R2_GLOBALS._walkRegions.enableRegion(5);
				R2_GLOBALS._walkRegions.enableRegion(6);
				_actor4.setup(3152, 4, 10);
				_actor4.setDetails(3150, 14, -1, -1, 1, (SceneItem *)NULL);
			} else {
				_actor4.setup(3152, 7, 4);
				_actor4.setDetails(3150, 13, -1, -1, 1, (SceneItem *)NULL);
			}
			_actor4.fixPriority(110);
			_actor4.setPosition(Common::Point(83, 88));
			_actor4._effect = 6;
			_actor4._shade = 3;
		} else {
			_actor4.setup(3152, 7, 3);
			_actor4.setPosition(Common::Point(143, 70));
			_actor4.setDetails(3150, 15, -1, -1, 1, (SceneItem *)NULL);
		}
	}

	if (R2_INVENTORY.getObjectScene(47) == 3150) {
		_actor3.postInit();
		_actor3.setup(3152, 7, 1);
		_actor3.setPosition(Common::Point(73, 83));
	}

	if (R2_INVENTORY.getObjectScene(40) == 3150) {
		_actor3.postInit();
		_actor3.setup(3152, 7, 3);
		_actor3.setPosition(Common::Point(70, 55));
		_actor3.fixPriority(111);
		_actor3._effect = 6;
		_actor3._shade = 5;
	}

	if (R2_INVENTORY.getObjectScene(42) == 3150) {
		_actor5.postInit();
		if (R2_GLOBALS.getFlag(77)) {
			_actor5.setup(3152, 7, 8);
			_actor5.setPosition(Common::Point(82, 92));
			_actor5.fixPriority(111);
			_actor5._effect = 6;
			_actor5._shade = 3;
		} else {
			_actor5.setup(3152, 7, 7);
			_actor5.setPosition(Common::Point(155, 79));
		}
		_actor5.setDetails(3150, 30, -1, -1, 2, (SceneItem *)NULL);
	}

	_actor6.postInit();
	_actor6.setup(3152, 7, 6);
	_actor6.setPosition(Common::Point(98, 73));
	_actor6.setDetails(3150, 43, -1, -1, 1, (SceneItem *)NULL);

	_item2.setDetails(12, 3150, 10, -1, 12);
	_item3.setDetails(Rect(186, 17, 210, 36), 3150, 6, -1, -1, 1, NULL);
	_item4.setDetails(Rect(61, 21, 92, 41), 3150, 7, -1, -1, 1, NULL);
	_item5.setDetails(Rect(63, 48, 78, 58), 3150, 6, -1, -1, 1, NULL);
	_item6.setDetails(Rect(63, 81, 100, 95), 3150, 3, 4, -1, 1, NULL);
	_item1.setDetails(Rect(0, 0, 200, 320), 3150, 0, 1, 2, 1, NULL);

	switch (R2_GLOBALS._player._oldCharacterScene[3]) {
	case 0:
		_sceneMode = 3150;
		_actor1.postInit();
		_actor1._effect = 6;
		_actor1._shade = 5;
		setAction(&_sequenceManager, this, 3150, &R2_GLOBALS._player, &_actor1, &_actor2, NULL);
		break;
	case 1200:
		_sceneMode = 3162;
		setAction(&_sequenceManager, this, 3162, &R2_GLOBALS._player, NULL);
		break;
	case 3275: {
		_sceneMode = 10;
		R2_GLOBALS._player.setup(30, 3, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(-20, 180));
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);

		Common::Point pt(80, 125);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	default:
		if ((R2_GLOBALS._v56AA0 == 1) && (R2_INVENTORY.getObjectScene(35) == 2000) && (R2_GLOBALS._player._oldCharacterScene[1] == 3100)) {
			++R2_GLOBALS._v56AA0;
			_sceneMode = 3156;
			_actor1.postInit();
			_actor1._effect = 6;
			_actor1._shade = 3;

			_actor2.postInit();
			_actor5.postInit();
			_actor5._effect = 6;
			_actor5._shade = 3;

			setAction(&_sequenceManager, this, 3156, &R2_GLOBALS._player, &_actor1, &_actor2, &_actor5, NULL);
		} else {
			if (R2_GLOBALS._v56AA0 != 2)
				++R2_GLOBALS._v56AA0;

			R2_GLOBALS._player.setup(30, 3, 1);
			R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
			R2_GLOBALS._player.setPosition(Common::Point(155, 120));
			R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
			R2_GLOBALS._player.enableControl();
		}
	}

	R2_GLOBALS._player._oldCharacterScene[3] = 3150;
}

void Scene3150::signal() {
	switch (_sceneMode) {
	case 11:
		R2_GLOBALS._sceneManager.changeScene(3275);
		break;
	case 12:
		R2_GLOBALS._sceneManager.changeScene(1200);
		break;
	case 3151:
		_actor1.remove();
		R2_INVENTORY.setObjectScene(41, 3);
		R2_GLOBALS._player.enableControl();
		break;
	case 3153:
		R2_GLOBALS.setFlag(76);
		_actor4.setDetails(3150, 14, -1, -1, 3, (SceneItem *)NULL);
		_actor1.postInit();
		_actor1.setDetails(3150, 24, -1, -1, 2, (SceneItem *)NULL);
		_sceneMode = 3161;
		setAction(&_sequenceManager, this, 3161, &_actor1, &_actor2, NULL);
		break;
	case 3154:
		_actor3.remove();
		R2_INVENTORY.setObjectScene(47, 3);
		R2_GLOBALS._player.enableControl();
		break;
	case 3155:
		R2_INVENTORY.setObjectScene(40, 3150);
		R2_GLOBALS._player.enableControl();
		break;
	case 3156:
		_actor5.setDetails(3150, 30, -1, -1, 2, (SceneItem *)NULL);
		R2_INVENTORY.setObjectScene(42, 3150);
		R2_GLOBALS._player.enableControl();
		break;
	case 3157:
		_actor5.remove();
		R2_INVENTORY.setObjectScene(42, 3);
		R2_GLOBALS._player.enableControl();
		break;
	case 3158:
		R2_GLOBALS.setFlag(75);
		R2_INVENTORY.setObjectScene(41, 3150);
		_actor4.fixPriority(110);
		_actor4.setDetails(3150, 13, -1, -1, 2, (SceneItem *)NULL);
		R2_GLOBALS._player.enableControl();
		break;
	case 3159:
		R2_GLOBALS.setFlag(77);
		R2_INVENTORY.setObjectScene(42, 3150);
		R2_GLOBALS._player.enableControl();
		break;
	case 3160:
		R2_INVENTORY.setObjectScene(52, 3150);
		R2_GLOBALS.setFlag(80);
		R2_GLOBALS._sceneManager.changeScene(1200);
		break;
	case 3161:
		R2_GLOBALS._sceneItems.remove(&_actor2);
		_exit1.setDetails(Rect(0, 135, 60, 168), EXITCURSOR_SW, 3275);
		_exit1.setDest(Common::Point(70, 125));
		R2_GLOBALS._walkRegions.enableRegion(1);
		R2_GLOBALS._walkRegions.enableRegion(4);
		R2_GLOBALS._walkRegions.enableRegion(5);
		R2_GLOBALS._walkRegions.enableRegion(6);
		R2_GLOBALS.setFlag(78);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene3150::dispatch() {
	if (_actor5._position.x == 155) {
		_actor5._effect = 0;
		_actor5._shade = 0;
	}

	if (_actor1._visage == 3154) {
		_actor1._effect = 0;
		_actor1._shade = 0;
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3175 - Autopsy room
 *
 *--------------------------------------------------------------------------*/
bool Scene3175::Item1::startAction(CursorType action, Event &event) {
	Scene3175 *scene = (Scene3175 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_useLineNum != -1) {
			SceneItem::display(_resNum, _useLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_LOOK:
		if (_lookLineNum != -1) {
			SceneItem::display(_resNum, _lookLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_TALK:
		if (_talkLineNum != -1) {
			SceneItem::display(_resNum, _talkLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	default:
		break;
	}

	return scene->display(action, event);
}

bool Scene3175::Actor3::startAction(CursorType action, Event &event) {
	Scene3175 *scene = (Scene3175 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_useLineNum != -1) {
			SceneItem::display(_resNum, _useLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_LOOK:
		if (_lookLineNum != -1) {
			SceneItem::display(_resNum, _lookLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_TALK:
		if (_talkLineNum != -1) {
			SceneItem::display(_resNum, _talkLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	default:
		break;
	}

	return scene->display(action, event);
}

bool Scene3175::Actor1::startAction(CursorType action, Event &event) {
	Scene3175 *scene = (Scene3175 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3176;
		scene->setAction(&scene->_sequenceManager, scene, 3176, &R2_GLOBALS._player, &scene->_actor1, NULL);
		return true;
		break;
	case CURSOR_LOOK:
		SceneItem::display(3175, 9, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		return true;
		break;
	case CURSOR_TALK:
		SceneItem::display(3175, 10, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		return true;
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

void Scene3175::postInit(SceneObjectList *OwnerList) {
	loadScene(3175);
	SceneExt::postInit();

	_actor1.postInit();
	_actor1.setup(3175, 1, 1);
	_actor1.setPosition(Common::Point(35, 72));
	_actor1.setDetails(3175, 9, 10, -1, 1, (SceneItem *)NULL);

	_actor2.postInit();
	_actor2.setup(3175, 2, 1);
	_actor2.setPosition(Common::Point(87, 148));

	_actor3.postInit();
	_actor3.setup(3175, 3, 1);
	_actor3.setPosition(Common::Point(199, 117));
	_actor3.setDetails(3175, 15, 16, 17, 1, (SceneItem *)NULL);

	_item2.setDetails(12, 3175, 3, 1, 5);
	_item3.setDetails(11, 3175, 6, 7, 8);
	_item1.setDetails(Rect(0, 0, 320, 200), 3175, 0, 1, 2, 1, NULL);

	R2_GLOBALS._player.postInit();

	if (R2_GLOBALS._player._oldCharacterScene[3] == 3250) {
		R2_GLOBALS._player.setup(30, 5, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(126, 77));
		R2_GLOBALS._player.enableControl();
	} else {
		_sceneMode = 3175;
		setAction(&_sequenceManager, this, 3175, &R2_GLOBALS._player, &_actor1, NULL);
	}

	R2_GLOBALS._player._oldCharacterScene[3] = 3175;
}

void Scene3175::signal() {
	if (_sceneMode == 3176)
		R2_GLOBALS._sceneManager.changeScene(3250);
	else
		R2_GLOBALS._player.enableControl();
}

/*--------------------------------------------------------------------------
 * Scene 3200 - Cutscene : Guards - Discussion
 *
 *--------------------------------------------------------------------------*/
void Scene3200::postInit(SceneObjectList *OwnerList) {
	loadScene(3200);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_rockoSpeaker);
	_stripManager.addSpeaker(&_jockoSpeaker);
	_stripManager.addSpeaker(&_sockoSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor3.postInit();
	_actor2.postInit();

	setAction(&_sequenceManager, this, 3200 + R2_GLOBALS._randomSource.getRandomNumber(1), &_actor1, &_actor2, &_actor3, NULL);
}

void Scene3200::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3210 - Cutscene : Captain and Private - Discussion
 *
 *--------------------------------------------------------------------------*/
void Scene3210::postInit(SceneObjectList *OwnerList) {
	loadScene(3210);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_privateSpeaker);
	_stripManager.addSpeaker(&_captainSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor2.postInit();

	setAction(&_sequenceManager, this, 3210 + R2_GLOBALS._randomSource.getRandomNumber(1), &_actor1, &_actor2, NULL);
}

void Scene3210::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3220 - Cutscene : Guards in cargo zone
 *
 *--------------------------------------------------------------------------*/
void Scene3220::postInit(SceneObjectList *OwnerList) {
	loadScene(3220);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_rockoSpeaker);
	_stripManager.addSpeaker(&_jockoSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor2.postInit();

	setAction(&_sequenceManager, this, 3220 + R2_GLOBALS._randomSource.getRandomNumber(1), &_actor1, &_actor2, NULL);
}

void Scene3220::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3230 - Cutscene : Guards on duty
 *
 *--------------------------------------------------------------------------*/
void Scene3230::postInit(SceneObjectList *OwnerList) {
	loadScene(3230);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_rockoSpeaker);
	_stripManager.addSpeaker(&_jockoSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor2.postInit();
	_actor3.postInit();

	setAction(&_sequenceManager, this, 3230 + R2_GLOBALS._randomSource.getRandomNumber(1), &_actor1, &_actor2, &_actor3, NULL);
}

void Scene3230::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3240 - Cutscene : Teal monolog
 *
 *--------------------------------------------------------------------------*/
void Scene3240::postInit(SceneObjectList *OwnerList) {
	loadScene(3240);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_tealSpeaker);
	_stripManager.addSpeaker(&_webbsterSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor2.postInit();

	setAction(&_sequenceManager, this, 3240 + R2_GLOBALS._randomSource.getRandomNumber(1), &_actor1, &_actor2, NULL);
}

void Scene3240::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3245 - Cutscene : Discussions with Dr. Tomko
 *
 *--------------------------------------------------------------------------*/
void Scene3245::postInit(SceneObjectList *OwnerList) {
	loadScene(3245);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_ralfSpeaker);
	_stripManager.addSpeaker(&_tomkoSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor2.postInit();

	if (R2_GLOBALS._v56AA1 < 4)
		++R2_GLOBALS._v56AA1;

	if (R2_GLOBALS._v56AA1 >= 4) {
		SceneItem::display(1200, 7, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		signal();
	} else {
		setAction(&_sequenceManager, this, 3244 + R2_GLOBALS._v56AA1, &_actor1, &_actor2, NULL);
	}
}

void Scene3245::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3250 - Room with large stasis field negator
 *
 *--------------------------------------------------------------------------*/
bool Scene3250::Item::startAction(CursorType action, Event &event) {
	Scene3250 *scene = (Scene3250 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_useLineNum != -1) {
			SceneItem::display(_resNum, _useLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_LOOK:
		if (_lookLineNum != -1) {
			SceneItem::display(_resNum, _lookLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_TALK:
		if (_talkLineNum != -1) {
			SceneItem::display(_resNum, _talkLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	default:
		break;
	}

	return scene->display(action, event);
}

bool Scene3250::Actor::startAction(CursorType action, Event &event) {
	Scene3250 *scene = (Scene3250 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();

	switch(_position.x) {
	case 25:
		scene->_sceneMode = 3262;
		scene->setAction(&scene->_sequenceManager, scene, 3262, &R2_GLOBALS._player, &scene->_actor1, NULL);
		break;
	case 259:
		scene->_sceneMode = 3260;
		scene->setAction(&scene->_sequenceManager, scene, 3260, &R2_GLOBALS._player, &scene->_actor2, NULL);
		break;
	case 302:
		scene->_sceneMode = 3261;
		scene->setAction(&scene->_sequenceManager, scene, 3261, &R2_GLOBALS._player, &scene->_actor3, NULL);
		break;
	default:
		break;
	}
	return true;
}

void Scene3250::postInit(SceneObjectList *OwnerList) {
	loadScene(3250);

	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._player._oldCharacterScene[3] = 1200;
		R2_GLOBALS._player._characterIndex = R2_MIRANDA;
	}

	SceneExt::postInit();
	_actor1.postInit();
	_actor1.setup(3250, 6, 1);
	_actor1.setPosition(Common::Point(25, 148));
	_actor1.fixPriority(10);
	_actor1.setDetails(3250, 9, 10, -1, 1, (SceneItem *)NULL);

	_actor2.postInit();
	_actor2.setup(3250, 4, 1);
	_actor2.setPosition(Common::Point(259, 126));
	_actor2.fixPriority(10);
	_actor2.setDetails(3250, 9, 10, -1, 1, (SceneItem *)NULL);

	_actor3.postInit();
	_actor3.setup(3250, 5, 1);
	_actor3.setPosition(Common::Point(302, 138));
	_actor3.fixPriority(10);
	_actor3.setDetails(3250, 9, 10, -1, 1, (SceneItem *)NULL);

	_item3.setDetails(Rect(119, 111, 149, 168), 3250, 6, 7, 2, 1, NULL);
	_item2.setDetails(Rect(58, 85, 231, 138), 3250, 12, 7, 2, 1, NULL);
	_item4.setDetails(12, 3250, 3, 1, 2);
	_item1.setDetails(Rect(0, 0, 320, 200), 3250, 0, 1, 2, 1, NULL);

	R2_GLOBALS._player.postInit();

	switch (R2_GLOBALS._player._oldCharacterScene[3]) {
	case 1200:
		_sceneMode = 3250;
		_actor4.postInit();
		R2_GLOBALS._player._effect = 0;
		setAction(&_sequenceManager, this, 3250, &R2_GLOBALS._player, &_actor4, NULL);
		break;
	case 3125:
		if (R2_GLOBALS.getFlag(79)) {
			_sceneMode = 3254;
			_actor5.postInit();
			_actor5._effect = 1;
			_actor6.postInit();
			_actor6._effect = 1;
			_actor7.postInit();
			_actor7._effect = 1;
			setAction(&_sequenceManager, this, 3254, &R2_GLOBALS._player, &_actor3, &_actor5, &_actor6, &_actor7, &_actor1, NULL);
		} else {
			_sceneMode = 3252;
			setAction(&_sequenceManager, this, 3252, &R2_GLOBALS._player, &_actor3, NULL);
		}
		break;
	case 3175:
		_sceneMode = 3251;
		setAction(&_sequenceManager, this, 3251, &R2_GLOBALS._player, &_actor2, NULL);
		break;
	case 3255:
		_sceneMode = 3253;
		setAction(&_sequenceManager, this, 3253, &R2_GLOBALS._player, &_actor1, NULL);
		break;
	default:
		R2_GLOBALS._player.setup(31, 3, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(185, 150));
		R2_GLOBALS._player.enableControl();
		break;
	}

	R2_GLOBALS._player._oldCharacterScene[3] = 3250;
}

void Scene3250::signal() {
	switch(_sceneMode) {
	case 3250:
		R2_GLOBALS._player._effect = 1;
		R2_GLOBALS._player.enableControl();
		break;
	case 3254:
	//No break on purpose
	case 3262:
		R2_GLOBALS._sceneManager.changeScene(3255);
		break;
	case 3260:
		R2_GLOBALS._sceneManager.changeScene(3175);
		break;
	case 3261:
		R2_GLOBALS._sceneManager.changeScene(3125);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene3250::dispatch() {
	if ((R2_GLOBALS._player._visage == 3250) && (R2_GLOBALS._player._strip == 3) && (R2_GLOBALS._player._effect == 0)) {
		R2_GLOBALS._player._effect = 6;
		R2_GLOBALS._player._shade = 6;
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3255 -
 *
 *--------------------------------------------------------------------------*/
void Scene3255::postInit(SceneObjectList *OwnerList) {
	loadScene(3255);
	SceneExt::postInit();

	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS.setFlag(79);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS.getFlag(79)) {
		R2_GLOBALS._sound1.play(267);
		R2_GLOBALS._sound2.play(268);
		_sceneMode = 3257;
		_actor3.postInit();
		_actor4.postInit();
		_actor4._effect = 1;
		setAction(&_sequenceManager, this, 3257, &R2_GLOBALS._player, &_actor4, &_actor3, NULL);
	} else {
		_actor1.postInit();
		_actor1.setup(303, 1, 1);
		_actor1.setPosition(Common::Point(208, 128));
		_actor2.postInit();
		_actor2.setup(3107, 3, 1);
		_actor2.setPosition(Common::Point(230, 127));
		_sceneMode = 3255;
		setAction(&_sequenceManager, this, 3255, &R2_GLOBALS._player, NULL);
	}
	R2_GLOBALS._player._oldCharacterScene[3] = 3255;
}

void Scene3255::signal() {
	switch (_sceneMode) {
	case 10:
		_sceneMode = 3258;
		_actor5.postInit();
		_actor6.postInit();
		_actor7.postInit();
		setAction(&_sequenceManager, this, 3258, &R2_GLOBALS._player, &_actor4, &_actor3, &_actor5, &_actor6, &_actor7, NULL);
		break;
	case 3256:
		R2_GLOBALS._sceneManager.changeScene(3250);
		break;
	case 3257:
		_sceneMode = 10;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(607, this);
		break;
	case 3258:
		R2_GLOBALS._sceneManager.changeScene(3100);
		break;
	default:
		SceneItem::display(3255, 0, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		_sceneMode = 3256;
		setAction(&_sequenceManager, this, 3256, &R2_GLOBALS._player, NULL);
	}
}

void Scene3255::dispatch() {
	if (R2_GLOBALS.getFlag(79)) {
		if (_actor5._position.y >= 95) {
			if (_actor5._position.y <= 110)
				_actor5._shade = 6 - (_actor5._position.y - 95) / 3;
			else
				_actor5._effect = 1;
		} else {
			_actor5._effect = 6;
			_actor5._shade = 6;
		}

		if (_actor6._position.y >= 95) {
			if (_actor6._position.y <= 110)
				_actor6._shade = 6 - (_actor6._position.y - 95) / 3;
			else
				_actor6._effect = 1;
		} else {
			_actor6._effect = 6;
			_actor6._shade = 6;
		}

		if (_actor7._position.y >= 95) {
			if (_actor7._position.y <= 110)
				_actor7._shade = 6 - (_actor7._position.y - 95) / 3;
			else
				_actor7._effect = 1;
		} else {
			_actor7._effect = 6;
			_actor7._shade = 6;
		}
	}

	if ((R2_GLOBALS._player._position.x > 250) && (R2_GLOBALS._player._shade == 1)) {
		R2_GLOBALS._player._effect = 6;
		_actor4._effect = 6;
	}
	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3260 - Computer room
 *
 *--------------------------------------------------------------------------*/
bool Scene3260::Actor13::startAction(CursorType action, Event &event) {
	Scene3260 *scene = (Scene3260 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 3271;
	scene->setAction(&scene->_sequenceManager, scene, 3271, &R2_GLOBALS._player, &scene->_actor13, NULL);
	return true;
}

bool Scene3260::Actor14::startAction(CursorType action, Event &event) {
	Scene3260 *scene = (Scene3260 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 3272;
	scene->setAction(&scene->_sequenceManager, scene, 3272, &R2_GLOBALS._player, &scene->_actor14, NULL);
	return true;
}

void Scene3260::Action1::signal() {
	SceneObjectExt *fmtObj = (SceneObjectExt *) _endHandler;

	fmtObj->setFrame(R2_GLOBALS._randomSource.getRandomNumber(6));
	setDelay(120 + R2_GLOBALS._randomSource.getRandomNumber(179));
}

void Scene3260::postInit(SceneObjectList *OwnerList) {
	loadScene(3260);
	R2_GLOBALS._player._characterIndex = R2_MIRANDA;
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(285);

	_actor13.postInit();
	_actor13.setup(3260, 6, 1);
	_actor13.setPosition(Common::Point(40, 106));
	_actor13.setDetails(3260, 18, 1, -1, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(52) == 3260) {
		_actor14.postInit();
		_actor14.setup(3260, 7, 1);
		_actor14.setPosition(Common::Point(202, 66));
		_actor14.setDetails(3260, 12, 1, -1, 1, (SceneItem *)NULL);
	}

	_actor1.postInit();
	_actor1.setup(3260, 1, 1);
	_actor1.setPosition(Common::Point(93, 73));
	_actor1.setDetails(3260, 3, 1, 5, 1, (SceneItem *)NULL);
	_actor1.setAction(&_action1, &_actor1);

	_actor2.postInit();
	_actor2.setup(3260, 2, 1);
	_actor2.setPosition(Common::Point(142, 63));
	_actor2.setDetails(3260, 3, 1, 5, 1, (SceneItem *)NULL);
	_actor2.setAction(&_action2, &_actor2);

	_actor3.postInit();
	_actor3.setup(3260, 2, 1);
	_actor3.setPosition(Common::Point(166, 54));
	_actor3.setDetails(3260, 3, 1, 5, 1, (SceneItem *)NULL);
	_actor3.setAction(&_action3, &_actor3);

	_actor4.postInit();
	_actor4.setup(3260, 2, 1);
	_actor4.setPosition(Common::Point(190, 46));
	_actor4.setDetails(3260, 3, 1, 5, 1, (SceneItem *)NULL);
	_actor4.setAction(&_action4, &_actor4);

	_actor5.postInit();
	_actor5.setup(3260, 2, 1);
	_actor5.setPosition(Common::Point(142, 39));
	_actor5.setDetails(3260, 3, 1, 5, 1, (SceneItem *)NULL);
	_actor5.setAction(&_action5, &_actor5);

	_actor6.postInit();
	_actor6.setup(3260, 2, 1);
	_actor6.setPosition(Common::Point(166, 30));
	_actor6.setDetails(3260, 3, 1, 5, 1, (SceneItem *)NULL);
	_actor6.setAction(&_action6, &_actor6);

	_actor7.postInit();
	_actor7.setup(3260, 2, 1);
	_actor7.setPosition(Common::Point(190, 22));
	_actor7.setDetails(3260, 3, 1, 5, 1, (SceneItem *)NULL);
	_actor7.setAction(&_action7, &_actor7);

	_actor8.postInit();
	_actor8.setup(3260, 2, 1);
	_actor8.setPosition(Common::Point(142, 14));
	_actor8.setDetails(3260, 3, 1, 5, 1, (SceneItem *)NULL);
	_actor8.setAction(&_action8, &_actor8);

	_actor9.postInit();
	_actor9.setup(3260, 2, 1);
	_actor9.setPosition(Common::Point(166, 6));
	_actor9.setDetails(3260, 3, 1, 5, 1, (SceneItem *)NULL);
	_actor9.setAction(&_action9, &_actor9);

	_actor10.postInit();
	_actor10.setup(3260, 3, 1);
	_actor10.setPosition(Common::Point(265, 163));
	_actor10.fixPriority(180);
	_actor10._numFrames = 10;
	_actor10.setDetails(3260, 6, 1, 8, 1, (SceneItem *)NULL);
	_actor10.animate(ANIM_MODE_2, NULL);

	_actor11.postInit();
	_actor11.setup(3260, 4, 1);
	_actor11.setPosition(Common::Point(127, 108));
	_actor11.fixPriority(120);
	_actor11.setAction(&_action11, &_actor11);
	_actor11._numFrames = 15;
	_actor11.setDetails(3260, 6, 1, 8, 1, (SceneItem *)NULL);
	_actor11.animate(ANIM_MODE_2, NULL);

	_actor12.postInit();
	_actor12.setup(3260, 5, 1);
	_actor12.setPosition(Common::Point(274, 65));
	_actor12.setAction(&_action12, &_actor12);
	_actor12._numFrames = 5;
	_actor12.setDetails(3260, 9, 1, 11, 1, (SceneItem *)NULL);
	_actor12.animate(ANIM_MODE_2, NULL);

	_item1.setDetails(Rect(0, 0, 320, 200), 3260, 0, 1, 2, 1, NULL);
	R2_GLOBALS._player.postInit();

	if (R2_GLOBALS._player._oldCharacterScene[3] == 3275) {
		_sceneMode = 3270;
		setAction(&_sequenceManager, this, 3270, &R2_GLOBALS._player, &_actor13, NULL);
	} else {
		R2_GLOBALS._player.setup(30, 5, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(53, 113));
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		R2_GLOBALS._player.enableControl();
	}
	R2_GLOBALS._player._oldCharacterScene[3] = 3260;
}

void Scene3260::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene3260::signal() {
	switch (_sceneMode) {
	case 3271:
		R2_GLOBALS._sceneManager.changeScene(3275);
		break;
	case 3272:
		_sceneMode = 3273;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		SceneItem::display(3260, 15, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		R2_GLOBALS._player.disableControl();
		R2_INVENTORY.setObjectScene(52, 3);
		R2_INVENTORY.setObjectScene(43, 3);
		setAction(&_sequenceManager, this, 3273, &R2_GLOBALS._player, &_actor14, NULL);
		break;
	case 3273:
		_actor4.remove();
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 3275 - Hall
 *
 *--------------------------------------------------------------------------*/
bool Scene3275::Actor2::startAction(CursorType action, Event &event) {
	Scene3275 *scene = (Scene3275 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 3275;
	scene->setAction(&scene->_sequenceManager, scene, 3275, &R2_GLOBALS._player, &scene->_actor2, NULL);
	return true;
}

void Scene3275::Exit1::changeScene() {
	Scene3275 *scene = (Scene3275 *)R2_GLOBALS._sceneManager._scene;

	scene->_sceneMode = 0;
	g_globals->_events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 10;
	Common::Point pt(418, 118);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene3275::postInit(SceneObjectList *OwnerList) {
	loadScene(3275);

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._sceneManager._previousScene = 3260;

	if (R2_GLOBALS._sceneManager._previousScene == 3150)
		g_globals->gfxManager()._bounds.moveTo(Common::Point(160, 0));
	else
		g_globals->gfxManager()._bounds.moveTo(Common::Point(0, 0));

	SceneExt::postInit();
	_exit1.setDetails(Rect(398, 60, 439, 118), SHADECURSOR_UP, 3150);
	_exit1.setDest(Common::Point(418, 128));

	_actor1.postInit();
	_actor1.setup(3275, 1, 7);
	_actor1.setPosition(Common::Point(419, 119));

	_actor2.postInit();
	_actor2.setup(3275, 2, 1);
	_actor2.setPosition(Common::Point(56, 118));
	_actor2.setDetails(3275, 3, 4, -1, 1, (SceneItem *)NULL);

	_item2.setDetails(Rect(153, 58, 200, 120), 3275, 6, 7, 8, 1, NULL);
	_item3.setDetails(Rect(275, 58, 331, 120), 3275, 6, 7, 8, 1, NULL);
	_item4.setDetails(Rect(0, 66, 22, 127), 3275, 9, 10, 11, 1, NULL);
	_item5.setDetails(Rect(457, 66, 480, 127), 3275, 9, 10, 11, 1, NULL);
	_item1.setDetails(Rect(0, 0, 480, 200), 3275, 0, 1, 2, 1, NULL);

	R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();
	if (R2_GLOBALS._player._oldCharacterScene[3] == 3150) {
		_sceneMode = 11;
		R2_GLOBALS._player.setup(30, 3, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(418, 118));
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		Common::Point pt(418, 128);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else if (R2_GLOBALS._player._oldCharacterScene[3] == 3260) {
		_sceneMode = 3276;
		setAction(&_sequenceManager, this, 3276, &R2_GLOBALS._player, &_actor2, NULL);
	} else {
		R2_GLOBALS._player.setup(30, 3, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(245, 135));
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		R2_GLOBALS._player.enableControl();
	}
	R2_GLOBALS._player._oldCharacterScene[3] = 3275;
}

void Scene3275::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._sceneManager.changeScene(3150);
		break;
	case 3275:
		R2_GLOBALS._sceneManager.changeScene(3260);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 3350 - Cutscene - Ship landing
 *
 *--------------------------------------------------------------------------*/
void Scene3350::postInit(SceneObjectList *OwnerList) {
	loadScene(3350);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();
	R2_GLOBALS._sound2.play(310);

	_rotation = R2_GLOBALS._scenePalette.addRotation(176, 203, 1);
	_rotation->setDelay(3);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor1.hide();
	_actor2.postInit();
	_actor2.hide();
	_actor3.postInit();
	_actor3.hide();
	_actor4.postInit();
	_actor4.hide();
	_actor9.postInit();
	_actor9.hide();
	_actor8.postInit();
	_actor8.hide();
	_actor5.postInit();
	_actor5.hide();
	_actor6.postInit();
	_actor6.hide();
	_actor7.postInit();
	_actor7.hide();

	_sceneMode = 3350;
	setAction(&_sequenceManager, this, _sceneMode, &_actor5, &_actor6, &_actor7, NULL);
}

void Scene3350::remove() {
	R2_GLOBALS._sound2.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene3350::signal() {
	switch (_sceneMode) {
	case 3350:
		_sceneMode = 3351;
		setAction(&_sequenceManager, this, 3351, &_actor4, &_actor9, &_actor8, NULL);
		break;
	case 3351:
		_sceneMode = 3352;
		setAction(&_sequenceManager, this, 3352, &_actor4, &R2_GLOBALS._player, &_actor1, &_actor2, &_actor3, NULL);
	case 3352:
		R2_GLOBALS._sceneManager.changeScene(3395);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 3375 -
 *
 *--------------------------------------------------------------------------*/
Scene3375::Scene3375() {
	_field1488 = _field1492 = 0;
	for (int i = 0; i < 4; ++i)
		_field148A[i] = 0;
}

void Scene3375::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field1488);
	s.syncAsSint16LE(_field1492);
	for (int i = 0; i < 4; ++i)
		s.syncAsSint16LE(_field148A[i]);
}

void Scene3375::subFC696(int sceneMode) {
	switch (sceneMode) {
	case 3379:
		R2_GLOBALS._player.setPosition(Common::Point(0, 155));
		_actor1.setPosition(Common::Point(-20, 163));
		_actor2.setPosition(Common::Point(-5, 150));
		_actor3.setPosition(Common::Point(-20, 152));
		break;
	case 3380:
		++R2_GLOBALS._v56A9E;
		if (R2_GLOBALS._v56A9E >= 4)
			R2_GLOBALS._v56A9E = 0;

		loadScene(_field148A[R2_GLOBALS._v56A9E]);

		R2_GLOBALS._uiElements.show();
		R2_GLOBALS._player.setStrip(4);
		R2_GLOBALS._player.setPosition(Common::Point(148, 230));

		_actor1.setPosition(Common::Point(191, 274));
		_actor1._effect = 1;
		_actor2.setPosition(Common::Point(124, 255));
		_actor2._effect = 1;
		_actor3.setPosition(Common::Point(155, 245));
		_actor3._effect = 1;
		break;
	case 3381:
		--R2_GLOBALS._v56A9E;
		if (R2_GLOBALS._v56A9E < 0)
			R2_GLOBALS._v56A9E = 3;

		loadScene(_field148A[R2_GLOBALS._v56A9E]);

		R2_GLOBALS._uiElements.show();
		R2_GLOBALS._player.setStrip(6);
		R2_GLOBALS._player.setPosition(Common::Point(201, 131));

		_actor1.setPosition(Common::Point(231, 127));
		_actor1._effect = 1;
		_actor2.setPosition(Common::Point(231, 127));
		_actor2._effect = 1;
		_actor3.setPosition(Common::Point(231, 127));
		_actor3._effect = 1;
		break;
	default:
		R2_GLOBALS._player.setPosition(Common::Point(192, 155));

		_actor1.setPosition(Common::Point(138, 134));
		_actor2.setPosition(Common::Point(110, 139));
		_actor3.setPosition(Common::Point(125, 142));
		break;
	}

	if (R2_GLOBALS._v56A9E == 2) {
		R2_GLOBALS._sceneItems.remove(&_actor4);
		for (int i = 0; i <= 12; i++)
			R2_GLOBALS._sceneItems.remove(&_itemArray[i]);
		R2_GLOBALS._sceneItems.remove(&_item1);

		_actor4.show();
		_actor4.setDetails(3375, 9, 10, -1, 1, (SceneItem *)NULL);

		for (int i = 0; i <= 12; i++)
			_itemArray[i].setDetails(3375, 3, -1, -1);

		_item1.setDetails(Rect(0, 0, 320, 200), 3375, 0, -1, -1, 1, NULL);
	} else {
		_actor4.hide();
		R2_GLOBALS._sceneItems.remove(&_actor4);
	}

	if (_sceneMode == 0)
		signal();
	else
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, &_actor1, &_actor2, &_actor3, NULL);
}

bool Scene3375::Actor1::startAction(CursorType action, Event &event) {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	scene->_sceneMode = 9999;
	if (R2_GLOBALS._player._characterIndex == 2)
		scene->_stripManager.start(3302, scene);
	else
		scene->_stripManager.start(3304, scene);

	return true;
}

bool Scene3375::Actor2::startAction(CursorType action, Event &event) {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	scene->_sceneMode = 9999;
	if (R2_GLOBALS._player._characterIndex == 3)
		scene->_stripManager.start(3302, scene);
	else
		scene->_stripManager.start(3301, scene);

	return true;
}

bool Scene3375::Actor3::startAction(CursorType action, Event &event) {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	scene->_sceneMode = 9999;
	scene->_stripManager.start(3303, scene);

	return true;
}

bool Scene3375::Actor4::startAction(CursorType action, Event &event) {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._v56A9E != 0) {
		R2_GLOBALS._walkRegions.disableRegion(2);
		R2_GLOBALS._walkRegions.disableRegion(3);
	} else {
		R2_GLOBALS._walkRegions.disableRegion(1);
		R2_GLOBALS._walkRegions.disableRegion(3);
		R2_GLOBALS._walkRegions.disableRegion(4);
	}
	R2_GLOBALS._walkRegions.disableRegion(6);
	R2_GLOBALS._walkRegions.disableRegion(7);
	R2_GLOBALS._walkRegions.disableRegion(8);

	R2_GLOBALS._player.disableControl(CURSOR_ARROW);

	scene->_sceneMode = 3375;
	scene->setAction(&scene->_sequenceManager, scene, 3375, &R2_GLOBALS._player, &scene->_actor1, &scene->_actor2, &scene->_actor3, &scene->_actor4, NULL);

	return true;
}

void Scene3375::Exit1::changeScene() {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 3376;
	if (R2_GLOBALS._v56A9E != 0) {
		R2_GLOBALS._walkRegions.disableRegion(2);
		R2_GLOBALS._walkRegions.disableRegion(3);
	} else {
		R2_GLOBALS._walkRegions.disableRegion(1);
		R2_GLOBALS._walkRegions.disableRegion(3);
		R2_GLOBALS._walkRegions.disableRegion(4);
	}
	if (scene->_actor1._position.y != 163) {
		R2_GLOBALS._player.setStrip(-1);
		scene->_actor1.setStrip2(-1);
		scene->_actor2.setStrip2(-1);
		scene->_actor3.setStrip2(-1);
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_actor1, &scene->_actor2, &scene->_actor3, NULL);
	} else {
		R2_GLOBALS._player.setStrip2(2);
		scene->_actor1.setStrip2(2);
		scene->_actor2.setStrip2(2);
		scene->_actor3.setStrip2(2);
		R2_GLOBALS._sound2.play(314);

		Common::Point pt(50, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, scene);
	}
}

void Scene3375::Exit2::changeScene() {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player._effect = 6;
	R2_GLOBALS._player._shade = 4;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);

	scene->_sceneMode = 3377;
	scene->_field1488 = 3381;

	if (R2_GLOBALS._v56A9E != 0) {
		R2_GLOBALS._walkRegions.disableRegion(2);
		R2_GLOBALS._walkRegions.disableRegion(3);
	} else {
		R2_GLOBALS._walkRegions.disableRegion(1);
		R2_GLOBALS._walkRegions.disableRegion(3);
		R2_GLOBALS._walkRegions.disableRegion(4);
	}
	scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_actor1, &scene->_actor2, &scene->_actor3, NULL);
}

void Scene3375::Exit3::changeScene() {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player._effect = 6;
	R2_GLOBALS._player._shade = 4;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);

	scene->_sceneMode = 3378;
	scene->_field1488 = 3380;

	if (R2_GLOBALS._v56A9E != 0) {
		R2_GLOBALS._walkRegions.disableRegion(2);
		R2_GLOBALS._walkRegions.disableRegion(3);
	} else {
		R2_GLOBALS._walkRegions.disableRegion(1);
		R2_GLOBALS._walkRegions.disableRegion(3);
		R2_GLOBALS._walkRegions.disableRegion(4);
	}
	scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_actor1, &scene->_actor2, &scene->_actor3, NULL);
}

void Scene3375::postInit(SceneObjectList *OwnerList) {
	_field148A[0] = 3376;
	_field148A[1] = 3377;
	_field148A[2] = 3375;
	_field148A[3] = 3378;

	loadScene(_field148A[R2_GLOBALS._v56A9E]);
	SceneExt::postInit();

	R2_GLOBALS._sound1.play(313);

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);
	_stripManager.addSpeaker(&_webbsterSpeaker);

	R2_GLOBALS._player._characterScene[1] = 3375;
	R2_GLOBALS._player._characterScene[2] = 3375;
	R2_GLOBALS._player._characterScene[3] = 3375;

	setZoomPercents(126, 55, 200, 167);
	R2_GLOBALS._player.postInit();

	if (R2_GLOBALS._player._characterIndex == 2) {
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	} else {
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	}
	R2_GLOBALS._player.changeZoom(-1);

	switch (R2_GLOBALS._player._characterIndex) {
	case 2:
		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setup(20, 1, 1);
		else
			R2_GLOBALS._player.setup(20, 3, 1);
		break;
	case 3:
		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setup(30, 1, 1);
		else
			R2_GLOBALS._player.setup(30, 3, 1);
		break;
	default:
		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setup(10, 1, 1);
		else
			R2_GLOBALS._player.setup(10, 3, 1);
		break;
	}

	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	if (R2_GLOBALS._player._characterIndex == 2) {
		_actor1._moveRate = 10;
		_actor1._moveDiff = Common::Point(3, 2);
	} else {
		_actor1._moveRate = 7;
		_actor1._moveDiff = Common::Point(5, 3);
	}
	_actor1.changeZoom(-1);
	_actor1._effect = 1;

	int tmpStrip, tmpVisage;
	if (R2_GLOBALS._sceneManager._previousScene == 3385)
		tmpStrip = 1;
	else
		tmpStrip = 4;

	if (R2_GLOBALS._player._characterIndex == 2)
		tmpVisage = 10;
	else
		tmpVisage = 20;

	_actor1.setup(tmpVisage, tmpStrip, 1);
	_actor1.animate(ANIM_MODE_1, NULL);

	_actor2.postInit();
	_actor2._moveDiff = Common::Point(3, 2);
	_actor2.changeZoom(-1);
	_actor2._effect = 1;
	if (R2_GLOBALS._sceneManager._previousScene == 3385)
		tmpStrip = 1;
	else
		tmpStrip = 8;

	if (R2_GLOBALS._player._characterIndex == 3)
		tmpVisage = 10;
	else
		tmpVisage = 30;

	_actor2.setup(tmpVisage, tmpStrip, 1);
	_actor2.animate(ANIM_MODE_1, NULL);

	_actor3.postInit();
	_actor3._moveRate = 7;
	_actor3._moveDiff = Common::Point(5, 3);
	_actor3.changeZoom(-1);
	_actor3._effect = 1;
	if (R2_GLOBALS._sceneManager._previousScene == 3385)
		tmpStrip = 1;
	else
		tmpStrip = 4;

	_actor3.setup(40, tmpStrip, 1);
	_actor3.animate(ANIM_MODE_1, NULL);

	_actor2.setDetails(3375, -1, -1, -1, 1, (SceneItem *)NULL);
	_actor3.setDetails(3375, 21, -1, -1, 1, (SceneItem *)NULL);
	_actor1.setDetails(3375, -1, -1, -1, 1, (SceneItem *)NULL);

	_actor4.postInit();
	_actor4.setup(3375, 1, 1);
	_actor4.setPosition(Common::Point(254, 166));
	_actor4.fixPriority(140);
	_actor4.hide();

	_exit1.setDetails(Rect(0, 84, 24, 167), EXITCURSOR_W, 3375);
	_exit1.setDest(Common::Point(65, 155));
	_exit2.setDetails(Rect(103, 152, 183, 170), SHADECURSOR_DOWN, 3375);
	_exit2.setDest(Common::Point(158, 151));
	_exit3.setDetails(Rect(180, 75, 213, 132), EXITCURSOR_E, 3375);
	_exit3.setDest(Common::Point(201, 131));

	for (int i = 0; i <= 12; ++i)
		_itemArray[i].setDetails(i, 3375, 3, -1, -1);

	_item1.setDetails(Rect(0, 0, 320, 200), 3375, 0, -1, 1, 1, NULL);

	if (R2_GLOBALS._sceneManager._previousScene == 3385)
		_sceneMode = 3379;
	else
		_sceneMode = 0;

	subFC696(_sceneMode);
}

void Scene3375::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene3375::signalCase3379() {
	switch (R2_GLOBALS._v56A9E) {
	case 0:
		_exit1._enabled = true;
		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._walkRegions.enableRegion(1);
		else {
			R2_GLOBALS._walkRegions.enableRegion(3);
			R2_GLOBALS._walkRegions.enableRegion(4);
		}
		R2_GLOBALS._walkRegions.enableRegion(6);
		R2_GLOBALS._walkRegions.enableRegion(7);
	case 2:
		_exit1._enabled = false;
		R2_GLOBALS._walkRegions.enableRegion(2);
		R2_GLOBALS._walkRegions.enableRegion(3);
		R2_GLOBALS._walkRegions.enableRegion(5);
		R2_GLOBALS._walkRegions.enableRegion(6);
		R2_GLOBALS._walkRegions.enableRegion(7);
		R2_GLOBALS._walkRegions.enableRegion(8);
		R2_GLOBALS._walkRegions.enableRegion(9);
	default:
		_exit1._enabled = false;
		R2_GLOBALS._walkRegions.enableRegion(2);
		R2_GLOBALS._walkRegions.enableRegion(3);
		R2_GLOBALS._walkRegions.enableRegion(5);
		R2_GLOBALS._walkRegions.enableRegion(6);
		break;
	}
	R2_GLOBALS._sceneManager._previousScene = 3375;
	R2_GLOBALS._player._effect = 1;
	_actor1._effect = 1;
	_actor2._effect = 1;
	_actor3._effect = 1;
	R2_GLOBALS._player.enableControl(CURSOR_ARROW);
}

void Scene3375::signal() {
	switch (_sceneMode) {
	case 3375:
		R2_GLOBALS._sceneManager.changeScene(3400);
		break;
	case 3376:
		R2_GLOBALS._sceneManager.changeScene(3385);
		break;
	case 3377:
		// No break on purpose
	case 3378:
		_sceneMode = _field1488;
		_field1488 = 0;
		_actor1._effect = 6;
		_actor1._shade = 4;
		_actor2._effect = 6;
		_actor2._shade = 4;
		_actor3._effect = 6;
		_actor3._shade = 4;
		subFC696(_sceneMode);
		break;
	case 3379:
		signalCase3379();
		break;
	case 9999:
		if (_actor1._position.y == 163)
			R2_GLOBALS._player.setStrip(1);
		else
			R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
	default:
		_actor1.setPriority(130);
		_actor2.setPriority(132);
		_actor3.setPriority(134);
		signalCase3379();
		break;
	}
}

void Scene3375::dispatch() {
	if ((R2_GLOBALS._player._position.y >= 168) && (R2_GLOBALS._player._effect == 1))
		R2_GLOBALS._player._effect = 6;
	else if ((R2_GLOBALS._player._position.y < 168) && (R2_GLOBALS._player._effect == 6))
		R2_GLOBALS._player._effect = 1;

	if ((_actor1._position.y >= 168) && (_actor1._effect == 1))
		_actor1._effect = 6;
	else if ((_actor1._position.y < 168) && (_actor1._effect == 6))
		_actor1._effect = 1;

	if ((_actor2._position.y >= 168) && (_actor2._effect == 1))
		_actor2._effect = 6;
	else if ((_actor2._position.y < 168) && (_actor2._effect == 6))
		_actor2._effect = 1;

	if ((_actor3._position.y >= 168) && (_actor3._effect == 1))
		_actor3._effect = 6;
	else if ((_actor3._position.y < 168) && (_actor3._effect == 6))
		_actor3._effect = 1;

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3385 -
 *
 *--------------------------------------------------------------------------*/
Scene3385::Scene3385() {
	_field11B2 = 0;
}

void Scene3385::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field11B2);
}

bool Scene3385::Actor1::startAction(CursorType action, Event &event) {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	scene->_sceneMode = 9999;
	if (R2_GLOBALS._player._characterIndex == 2)
		scene->_stripManager.start(3302, scene);
	else
		scene->_stripManager.start(3304, scene);

	return true;
}

bool Scene3385::Actor2::startAction(CursorType action, Event &event) {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	scene->_sceneMode = 9999;
	if (R2_GLOBALS._player._characterIndex == 3)
		scene->_stripManager.start(3302, scene);
	else
		scene->_stripManager.start(3301, scene);

	return true;
}

bool Scene3385::Actor3::startAction(CursorType action, Event &event) {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	scene->_sceneMode = 9999;
	scene->_stripManager.start(3303, scene);

	return true;
}

bool Scene3385::Actor4::startAction(CursorType action, Event &event) {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	if (R2_GLOBALS._sceneManager._previousScene == 3375)
		R2_GLOBALS._sound2.play(314);

	scene->_sceneMode = 3386;
	scene->setAction(&scene->_sequenceManager, scene, 3386, &R2_GLOBALS._player, &scene->_actor1, &scene->_actor2, &scene->_actor3, &scene->_actor4, NULL);

	return true;
}

void Scene3385::Exit1::changeScene() {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 3387;

	if (R2_GLOBALS._sceneManager._previousScene == 3375)
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_actor1, &scene->_actor2, &scene->_actor3, NULL);
	else
		scene->signal();
}

void Scene3385::Action1::signal() {
	int v = _actionIndex;
	++_actionIndex;

	if (v == 0)
		setDelay(1);
	else if (v == 1)
		R2_GLOBALS._sound2.play(314);
}

void Scene3385::postInit(SceneObjectList *OwnerList) {
	loadScene(3385);
	SceneExt::postInit();

	R2_GLOBALS._sound1.play(313);

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);
	_stripManager.addSpeaker(&_webbsterSpeaker);

	R2_GLOBALS._player._characterScene[1] = 3385;
	R2_GLOBALS._player._characterScene[2] = 3385;
	R2_GLOBALS._player._characterScene[3] = 3385;

	if (R2_GLOBALS._sceneManager._previousScene == 3375)
		_field11B2 = 3;
	else
		_field11B2 = 4;

	setZoomPercents(102, 40, 200, 160);
	R2_GLOBALS._player.postInit();

	if (R2_GLOBALS._player._characterIndex == 2)
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	else
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);

	R2_GLOBALS._player.changeZoom(-1);

	if (R2_GLOBALS._player._characterIndex == 2)
		R2_GLOBALS._player.setup(20, _field11B2, 1);
	else if (R2_GLOBALS._player._characterIndex == 3)
		R2_GLOBALS._player.setup(30, _field11B2, 1);
	else
		R2_GLOBALS._player.setup(10, _field11B2, 1);

	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	if (R2_GLOBALS._player._characterIndex == 2) {
		_actor1._moveRate = 10;
		_actor1._moveDiff = Common::Point(3, 2);
	} else {
		_actor1._moveRate = 7;
		_actor1._moveDiff = Common::Point(5, 3);
	}
	_actor1.changeZoom(-1);
	_actor1._effect = 1;
	if (R2_GLOBALS._player._characterIndex == 2)
		_actor1.setup(10, _field11B2, 1);
	else
		_actor1.setup(20, _field11B2, 1);
	_actor1.animate(ANIM_MODE_1, NULL);
	_actor1.setDetails(3385, -1, -1, -1, 1, (SceneItem *) NULL);

	_actor2.postInit();
	_actor2._moveDiff = Common::Point(3, 2);
	_actor2.changeZoom(-1);
	_actor2._effect = 1;
	if (R2_GLOBALS._player._characterIndex == 3)
		_actor2.setup(10, _field11B2, 1);
	else
		_actor2.setup(30, _field11B2, 1);
	_actor2.animate(ANIM_MODE_1, NULL);
	_actor2.setDetails(3385, -1, -1, -1, 1, (SceneItem *) NULL);

	_actor3.postInit();
	_actor3._moveDiff = Common::Point(3, 2);
	_actor3.changeZoom(-1);
	_actor3._effect = 1;
	_actor3.setup(40, _field11B2, 1);
	_actor3.animate(ANIM_MODE_1, NULL);
	_actor3.setDetails(3385, 15, -1, -1, 1, (SceneItem *) NULL);

	_exit1.setDetails(Rect(103, 152, 217, 170), SHADECURSOR_DOWN, 3395);
	_exit1.setDest(Common::Point(158, 151));

	_actor4.postInit();
	_actor4.setPosition(Common::Point(160, 100));
	_actor4.fixPriority(90);
	_actor4.setDetails(3385, 3, 4, -1, 1, (SceneItem *) NULL);

	if (R2_GLOBALS._sceneManager._previousScene == 3375) {
		R2_GLOBALS._player.setPosition(Common::Point(158, 102));
		_actor1.setPosition(Common::Point(164, 100));
		_actor1.fixPriority(98);
		_actor2.setPosition(Common::Point(150, 100));
		_actor2.fixPriority(97);
		_actor3.setPosition(Common::Point(158, 100));
		_actor3.fixPriority(96);
		_sceneMode = 3384;
		_actor4.setup(3385, 1, 6);
		_actor4.animate(ANIM_MODE_6, this);
		setAction(&_action1, &_actor4);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(158, 230));
		_actor1.setPosition(Common::Point(191, 270));
		_actor2.setPosition(Common::Point(124, 255));
		_actor3.setPosition(Common::Point(155, 245));
		_actor4.setup(3385, 1, 1);
		_sceneMode = 3385;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, &_actor1, &_actor2, &_actor3, NULL);
	}

	_item1.setDetails(Rect(0, 0, 320, 200), 3385, 0, -1, -1, 1, NULL);
	R2_GLOBALS._v56A9E = 0;
}

void Scene3385::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene3385::signal() {
	switch (_sceneMode) {
	case 3386:
		R2_GLOBALS._sceneManager.changeScene(3375);
		break;
	case 3387:
		R2_GLOBALS._sceneManager.changeScene(3395);
		break;
	case 9999:
		if (R2_GLOBALS._sceneManager._previousScene == 3375)
			R2_GLOBALS._player.setStrip(3);
		else
			R2_GLOBALS._player.setStrip(4);
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 3395 -
 *
 *--------------------------------------------------------------------------*/
Scene3395::Scene3395() {
	_field142E = 0;
}

void Scene3395::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field142E);
}

bool Scene3395::Actor1::startAction(CursorType action, Event &event) {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	scene->_sceneMode = 9999;
	if (R2_GLOBALS._player._characterIndex == 2)
		scene->_stripManager.start(3302, scene);
	else
		scene->_stripManager.start(3304, scene);

	return true;
}

bool Scene3395::Actor2::startAction(CursorType action, Event &event) {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	scene->_sceneMode = 9999;
	if (R2_GLOBALS._player._characterIndex == 3)
		scene->_stripManager.start(3302, scene);
	else
		scene->_stripManager.start(3301, scene);

	return true;
}

bool Scene3395::Actor3::startAction(CursorType action, Event &event) {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	scene->_sceneMode = 9999;
	scene->_stripManager.start(3303, scene);

	return true;
}

bool Scene3395::Actor4::startAction(CursorType action, Event &event) {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	if (R2_GLOBALS._sceneManager._previousScene == 3385)
		R2_GLOBALS._sound2.play(314);

	scene->_sceneMode = 3396;
	scene->setAction(&scene->_sequenceManager, scene, 3396, &R2_GLOBALS._player, &scene->_actor1, &scene->_actor2, &scene->_actor3, &scene->_actor4, NULL);

	return true;
}

void Scene3395::Action1::signal() {
	int v = _actionIndex;
	++_actionIndex;

	if (v == 0)
		setDelay(2);
	else if (v == 1)
		R2_GLOBALS._sound2.play(314);
}

void Scene3395::postInit(SceneObjectList *OwnerList) {
	loadScene(3395);
	SceneExt::postInit();

	R2_GLOBALS._sound1.play(313);

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);
	_stripManager.addSpeaker(&_webbsterSpeaker);

	R2_GLOBALS._player._characterScene[1] = 3395;
	R2_GLOBALS._player._characterScene[2] = 3395;
	R2_GLOBALS._player._characterScene[3] = 3395;

	if (R2_GLOBALS._sceneManager._previousScene == 3385)
		_field142E = 3;
	else
		_field142E = 4;

	setZoomPercents(51, 40, 200, 137);
	R2_GLOBALS._player.postInit();

	if (R2_GLOBALS._player._characterIndex == 2)
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	else
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);

	R2_GLOBALS._player.changeZoom(-1);

	if (R2_GLOBALS._player._characterIndex == 2)
		R2_GLOBALS._player.setup(20, _field142E, 1);
	else if (R2_GLOBALS._player._characterIndex == 3)
		R2_GLOBALS._player.setup(30, _field142E, 1);
	else
		R2_GLOBALS._player.setup(10, _field142E, 1);

	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	if (R2_GLOBALS._player._characterIndex == 2) {
		_actor1._moveRate = 10;
		_actor1._moveDiff = Common::Point(3, 2);
	} else {
		_actor1._moveRate = 7;
		_actor1._moveDiff = Common::Point(5, 3);
	}
	_actor1.changeZoom(-1);
	_actor1._effect = 1;
	if (R2_GLOBALS._player._characterIndex == 2)
		_actor1.setup(10, _field142E, 1);
	else
		_actor1.setup(20, _field142E, 1);
	_actor1.animate(ANIM_MODE_1, NULL);
	_actor1.setDetails(3395, -1, -1, -1, 1, (SceneItem *) NULL);

	_actor2.postInit();
	_actor2._moveDiff = Common::Point(3, 2);
	_actor2.changeZoom(-1);
	_actor2._effect = 1;
	if (R2_GLOBALS._player._characterIndex == 3)
		_actor2.setup(10, _field142E, 1);
	else
		_actor2.setup(30, _field142E, 1);
	_actor2.animate(ANIM_MODE_1, NULL);
	_actor2.setDetails(3395, -1, -1, -1, 1, (SceneItem *) NULL);

	_actor3.postInit();
	_actor3._moveDiff = Common::Point(3, 2);
	_actor3.changeZoom(-1);
	_actor3._effect = 1;
	_actor3.setup(40, _field142E, 1);
	_actor3.animate(ANIM_MODE_1, NULL);
	_actor3.setDetails(3385, 18, -1, -1, 1, (SceneItem *) NULL);

	_actor4.postInit();
	_actor4.setPosition(Common::Point(159, 50));
	_actor4.fixPriority(40);
	_actor4.setDetails(3395, 6, 7, -1, 1, (SceneItem *) NULL);

	if (R2_GLOBALS._sceneManager._previousScene == 3385) {
		R2_GLOBALS._player.setPosition(Common::Point(158, 53));
		_actor1.setPosition(Common::Point(164, 51));
		_actor1.fixPriority(48);
		_actor2.setPosition(Common::Point(150, 51));
		_actor2.fixPriority(47);
		_actor3.setPosition(Common::Point(158, 51));
		_actor3.fixPriority(46);
		_sceneMode = 3394;
		_actor4.setup(3395, 1, 7);
		_actor4.animate(ANIM_MODE_6, this);
		setAction(&_action1, &_actor4);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(158, 200));
		_actor1.setPosition(Common::Point(191, 255));
		_actor2.setPosition(Common::Point(124, 240));
		_actor3.setPosition(Common::Point(155, 242));
		_actor4.setup(3395, 1, 1);

		R2_GLOBALS._walkRegions.enableRegion(1);

		_sceneMode = 3395;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, &_actor1, &_actor2, &_actor3, NULL);
	}

	for (int i = 0; i <= 12; i++) {
		_itemArray[i].setDetails(i, 3995, 0, -1, -1);
	}

	_item1.setDetails(Rect(0, 0, 320, 200), 3395, 3, -1, -1, 1, NULL);
}

void Scene3395::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene3395::signal() {
	switch (_sceneMode) {
	case 3396:
		R2_GLOBALS._sceneManager.changeScene(3385);
		break;
	case 9999:
		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setStrip(3);
		else
			R2_GLOBALS._player.setStrip(4);
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 3400 -
 *
 *--------------------------------------------------------------------------*/
Scene3400::Scene3400() {
	_field157C = 0;
}

void Scene3400::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field157C);
}

void Scene3400::postInit(SceneObjectList *OwnerList) {
	R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
	g_globals->gfxManager()._bounds.moveTo(Common::Point(160, 0));
	loadScene(3400);
	_field157C = 0;
	R2_GLOBALS._v558B6.set(60, 0, 260, 200);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(317);

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);
	_stripManager.addSpeaker(&_webbsterSpeaker);
	_stripManager.addSpeaker(&_tealSpeaker);

	setZoomPercents(51, 46, 180, 200);
	R2_GLOBALS._player._characterScene[1] = 3400;
	R2_GLOBALS._player._characterScene[2] = 3400;
	R2_GLOBALS._player._characterScene[3] = 3400;

	_actor7.postInit();
	_actor7.setup(3403, 1, 1);
	_actor7.setPosition(Common::Point(190, 103));
	_actor7.fixPriority(89);

	R2_GLOBALS._player.postInit();
	if (R2_GLOBALS._player._characterIndex == 2)
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	else
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	R2_GLOBALS._player.changeZoom(-1);
	R2_GLOBALS._player.setPosition(Common::Point(239, 64));

	if (R2_GLOBALS._player._characterIndex == 2)
		R2_GLOBALS._player.setup(20, 5, 1);
	else if (R2_GLOBALS._player._characterIndex == 3)
		R2_GLOBALS._player.setup(30, 5, 1);
	else
		R2_GLOBALS._player.setup(10, 5, 1);

	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	if (R2_GLOBALS._player._characterIndex == 2) {
		_actor1._numFrames = 10;
		_actor1._moveDiff = Common::Point(3, 2);
	} else {
		_actor1._numFrames = 7;
		_actor1._moveDiff = Common::Point(5, 3);
	}
	_actor1.changeZoom(-1);
	_actor1._effect = 1;
	_actor1.setPosition(Common::Point(247, 63));
	if (R2_GLOBALS._player._characterIndex == 2)
		_actor1.setup(10, 5, 1);
	else
		_actor1.setup(20, 5, 1);
	_actor1.animate(ANIM_MODE_1, NULL);

	_actor2.postInit();
	_actor2._moveDiff = Common::Point(3, 2);
	_actor2.changeZoom(-1);
	_actor2._effect = 1;
	_actor2.setPosition(Common::Point(225, 63));
	if (R2_GLOBALS._player._characterIndex == 3)
		_actor2.setup(10, 5, 1);
	else
		_actor2.setup(30, 5, 1);
	_actor2.animate(ANIM_MODE_1, NULL);

	_actor3.postInit();
	_actor3._numFrames = 7;
	_actor3._moveDiff = Common::Point(5, 3);
	_actor3.changeZoom(-1);
	_actor3._effect = 1;
	_actor3.setPosition(Common::Point(235, 61));
	_actor3.setup(40, 3, 1);
	_actor3.animate(ANIM_MODE_1, NULL);

	_actor6.postInit();
	_actor6.setup(3400, 1, 6);
	_actor6.setPosition(Common::Point(236, 51));
	_actor6.fixPriority(51);
	_actor6.animate(ANIM_MODE_6, NULL);

	R2_GLOBALS.clearFlag(71);
	_sceneMode = 3400;
	setAction(&_sequenceManager, this, 3400, &R2_GLOBALS._player, &_actor1, &_actor2, &_actor3, NULL);
}

void Scene3400::remove() {
	R2_GLOBALS._sound2.fadeOut2(NULL);
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene3400::signal() {
	switch (_sceneMode) {
	case 3305: {
		warning("STUB: sub_1D227()");
		_tealSpeaker._object1.hide();
		_actor4.show();
		_actor4.setStrip(1);
		Common::Point pt(158, 190);
		NpcMover *mover = new NpcMover();
		_actor4.addMover(mover, &pt, this);
		_sceneMode = 3402;
		setAction(&_sequenceManager, this, 3402, &R2_GLOBALS._player, &_actor1, &_actor2, &_actor3, NULL);
		}
		break;
	case 3306:
		R2_GLOBALS._sound2.play(318);
		_actor1.setStrip(2);
		R2_GLOBALS._player.setStrip(6);
		_actor2.setStrip(6);
		_actor3.setStrip(3);
		_actor4.setStrip(1);
		R2_INVENTORY.setObjectScene(34, 0);
		_stripManager.start(3307, this);
		if (R2_GLOBALS._player._characterIndex == 2) {
			_sceneMode = 3400;
			R2_GLOBALS._player.setAction(&_sequenceManager, this, 3400, &R2_GLOBALS._player, &_actor4, &_actor8, NULL);
		} else {
			_sceneMode = 3408;
			_actor1.setAction(&_sequenceManager, this, 3408, &_actor1, &_actor4, &_actor8, NULL);
		}
		break;
	case 3307:
	case 3404:
	case 3408:
		if (_field157C == 0) {
			R2_GLOBALS._sound2.fadeOut2(NULL);
			_field157C = 1;
		} else {
			_sceneMode = 3308;
			_stripManager.start(3308, this);
		}
		break;
	case 3308:
		warning("STUB: sub_1D227()");
		_actor1.setStrip(2);
		R2_GLOBALS._player.setStrip(6);
		_actor2.setStrip(6);
		_actor3.setStrip(3);
		_actor4.setStrip(1);
		_sceneMode = 3403;
		if (R2_GLOBALS._player._characterIndex == 2)
			setAction(&_sequenceManager, this, 3403, &R2_GLOBALS._player, &_actor3, &_actor7, NULL);
		else
			setAction(&_sequenceManager, this, 3403, &_actor1, &_actor3, &_actor7, NULL);
		break;
	case 3309:
		warning("STUB: sub_1D227()");
		_actor4.setStrip(1);
		_sceneMode = 3405;
		if (R2_GLOBALS._player._characterIndex == 3)
			setAction(&_sequenceManager, this, 3405, &R2_GLOBALS._player, &_actor7, NULL);
		else
			setAction(&_sequenceManager, this, 3405, &_actor2, &_actor7, NULL);
		break;
	case 3310:
		warning("STUB: sub_1D227()");
		_actor4.setStrip(1);
		_sceneMode = 3406;
		if (R2_GLOBALS._player._characterIndex == 1)
			setAction(&_sequenceManager, this, 3406, &R2_GLOBALS._player, &_actor7, NULL);
		else if (R2_GLOBALS._player._characterIndex == 2)
			setAction(&_sequenceManager, this, 3406, &_actor1, &_actor7, NULL);
		else if (R2_GLOBALS._player._characterIndex == 3)
			setAction(&_sequenceManager, this, 3406, &_actor2, &_actor7, NULL);
		break;
	case 3311:
		warning("STUB: sub_1D227()");
		_tealSpeaker._object1.hide();
		_actor4.show();
		_actor4.setStrip(1);
		_sceneMode = 3407;
		setAction(&_sequenceManager, this, 3407, &_actor4, &_actor7, NULL);
		break;
	case 3400: {
		_actor8.postInit();
		_actor8.hide();
		_actor4.postInit();
		_actor4._numFrames = 7;
		_actor4._moveDiff = Common::Point(3, 2);
		_actor4.changeZoom(-1);
		_actor4._effect = 1;
		_actor4.setPosition(Common::Point(-15, 90));
		_actor4.setup(3402, 1, 1);
		_actor4.animate(ANIM_MODE_1, NULL);
		Common::Point pt1(115, 90);
		NpcMover *mover1 = new NpcMover();
		_actor4.addMover(mover1, &pt1, this);
		R2_GLOBALS._scrollFollower = &_actor4;
		Common::Point pt2(203, 76);
		NpcMover *mover2 = new NpcMover();
		_actor3.addMover(mover2, &pt2, NULL);
		_sceneMode = 3401;
		}
		break;
	case 3401:
		_sceneMode = 3305;
		_stripManager.start(3305, this);
		break;
	case 3402:
		_sceneMode = 3306;
		_stripManager.start(3306, this);
		break;
	case 3403:
		R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
		_sceneMode = 3309;
		_stripManager.start(3309, this);
		break;
	case 3405:
		_sceneMode = 3310;
		_stripManager.start(3310, this);
		break;
	case 3406:
		_sceneMode = 3311;
		_stripManager.start(3311, this);
		break;
	case 3407:
		R2_GLOBALS._sceneManager.changeScene(3600);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 3500 -
 *
 *--------------------------------------------------------------------------*/
Scene3500::Action1::Action1() {
	_field1E = 0;
	_field20 = 0;
	_field22 = 0;
	_field24 = 0;
}

void Scene3500::Action1::synchronize(Serializer &s) {
	Action::synchronize(s);

	s.syncAsSint16LE(_field1E);
	s.syncAsSint16LE(_field20);
	s.syncAsSint16LE(_field22);
	s.syncAsSint16LE(_field24);
}

void Scene3500::Action1::sub108670(int arg1) {
	Scene3500 *scene = (Scene3500 *)R2_GLOBALS._sceneManager._scene;

	_field1E = arg1;
	_field20 = 1;
	_field24 = 1;

	scene->_actor9.setStrip(2);
	scene->_actor9.show();

	if (_field1E == 1)
		scene->_actor6.show();
	else
		scene->_actor5.show();

	if (scene->_actor1._frame % 2 == 0)
	scene->_actor1._frameChange = _field1E;
	scene->_actor1.setFrame(scene->_actor1.changeFrame());

	setActionIndex(0);
}

void Scene3500::Action1::sub108732(int arg1) {
	Scene3500 *scene = (Scene3500 *)R2_GLOBALS._sceneManager._scene;

	_field20 = arg1;
	_field1E = -_field1E;

	if (_field1E == 1) {
		scene->_actor6.show();
		scene->_actor5.hide();
	} else {
		scene->_actor5.show();
		scene->_actor6.hide();
	}

	switch (_actionIndex) {
	case 4:
		scene->_actor1._frameChange = _field1E;
		scene->_actor1.setFrame(scene->_actor1.changeFrame());
	// No break on purpose
	case 3:
		_actionIndex = 10;
		setDelay(0);
		break;
	case 5: {
		scene->_fieldAF8 = 160;
		Common::Point pt(160, 73);
		NpcMover *mover = new NpcMover();
		scene->_actor8.addMover(mover, &pt, NULL);

		scene->_fieldB9E = 160 - (_field1E * 2 * 160);
		Common::Point pt2(scene->_fieldB9E, 73);
		NpcMover *mover2 = new NpcMover();
		scene->_actor9.addMover(mover2, &pt2, this);

		_actionIndex = 11;
		}
		break;
	case 6:
		scene->_actor1._frameChange = _field1E;
		scene->_actor1.setFrame(scene->_actor1.changeFrame());
		setDelay(1);
	// No break on purpose
	case 8:
		scene->_actor9.setStrip(2);
		_actionIndex = 1;
		break;
	default:
		break;
	}
}

Scene3500::Action2::Action2() {
	_field1E = 0;
}

void Scene3500::Action2::synchronize(Serializer &s) {
	Action::synchronize(s);

	s.syncAsSint16LE(_field1E);
}

Scene3500::Item4::Item4() {
	_field34 = 0;
}

void Scene3500::Item4::synchronize(Serializer &s) {
	NamedHotspot::synchronize(s);

	s.syncAsSint16LE(_field34);
}

Scene3500::Actor7::Actor7() {
	_fieldA4 = 0;
	_fieldA6 = 0;
	_fieldA8 = 0;
	_fieldAA = 0;
	_fieldAC = 0;
	_fieldAE = 0;
}

void Scene3500::Actor7::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_fieldA4);
	s.syncAsSint16LE(_fieldA6);
	s.syncAsSint16LE(_fieldA8);
	s.syncAsSint16LE(_fieldAA);
	s.syncAsSint16LE(_fieldAC);
	s.syncAsSint16LE(_fieldAE);
}

void Scene3500::Actor7::sub109466(int arg1, int arg2, int arg3, int arg4, int arg5) {
	_fieldAE = 0;
	_fieldA4 = arg1;
	_fieldA6 = arg2;
	_fieldA8 = arg3;
	_fieldAA = arg4;
	_fieldAC = _fieldAA / _fieldA8;

	postInit();
	setup(10501, 3, 1);
	fixPriority(255);
	sub109663(arg5);
}

void Scene3500::Actor7::sub1094ED() {
	Scene3500 *scene = (Scene3500 *)R2_GLOBALS._sceneManager._scene;

	scene->_field1270 = _position.x - _fieldA4;
}

void Scene3500::Actor7::sub109663(int arg1){
	sub109693(Common::Point(_fieldA4 + arg1, _fieldA6 - (_fieldAC * arg1)));
}

void Scene3500::Actor7::sub109693(Common::Point Pt) {
	setPosition(Pt);
}

int Scene3500::UnkObject3500::sub1097C9(int arg1) {
	return (_field2A / 2) + arg1 - (arg1 % _field2A);
}

int Scene3500::UnkObject3500::sub1097EF(int arg1) {
	return (_field2C / 2) + arg1 - (arg1 % _field2C);
}

int Scene3500::UnkObject3500::sub109C09(Common::Point pt) {
	int vx = pt.x / _field2A;
	int vy = pt.y / _field2C;

	if ((vx >= 0) && (_field26 > vx) && (_field28 > vy)) {
		return _field16[((_field26 * vy) + vx) * 2];
	} else
		return -1;
}

int Scene3500::UnkObject3500::sub109C5E(int &x, int &y) {
	int retVal = sub51AFD(Common::Point(x, y));
	x = _field2E;
	y = _field30;

	return retVal;
}

Scene3500::Scene3500() {
	_fieldAF8 = 0;
	_fieldB9E = 0;
	_rotation = NULL;
	_field126E = 0;
	_field1270 = 0;
	_field1272 = 0;
	_field1274 = 0;
	_field1276 = 0;
	_field1278 = 0;
	_field127A = 0;
	_field127C = 0;
	_field127E = 0;
	_field1280 = 0;
	_field1282 = 0;
	_field1284 = 0;
	_field1286 = 0;
}

void Scene3500::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	SYNC_POINTER(_rotation);

	s.syncAsSint16LE(_fieldAF8);
	s.syncAsSint16LE(_fieldB9E);
	s.syncAsSint16LE(_field126E);
	s.syncAsSint16LE(_field1270);
	s.syncAsSint16LE(_field1272);
	s.syncAsSint16LE(_field1274);
	s.syncAsSint16LE(_field1276);
	s.syncAsSint16LE(_field1278);
	s.syncAsSint16LE(_field127A);
	s.syncAsSint16LE(_field127C);
	s.syncAsSint16LE(_field127E);
	s.syncAsSint16LE(_field1280);
	s.syncAsSint16LE(_field1282);
	s.syncAsSint16LE(_field1284);
	s.syncAsSint16LE(_field1286);
}

void Scene3500::sub107F71(int arg1) {
	switch (arg1) {
	case -1:
		_actor7.sub1094ED();
		if (_field1270 != 0) {
			_field1270--;
			_actor7.sub109663(_field1270);
		}
		if (_action1._field24 != 0)
			_field1270 = 0;
		break;
	case 1:
		_actor7.sub1094ED();
		if (_field1270 < 16) {
			++_field1270;
			_actor7.sub109663(_field1270);
		}
		if (_action1._field24 != 0)
			_field1270 = 0;
		break;
	case 88:
		if ((_action == 0) || (_action1._field24 == 0)) {
		// The original makes a second useless check on action, skipped
			_action2.sub10831F(2);
			if ((_action) && ((_action2.getActionIndex() != 0) || (_action2._field1E != 2))) {
				_action2.signal();
			} else {
				_actor9.setAction(&_action2, &_actor9, NULL);
			}
		}
		break;
	case 96:
		if ((_action) && (_action1._field24 != 0) && (_action2._field1E != 1)) {
			_field1278 = 0;
			_action1.sub108732(0);
		} else if ((_action) && (_field1278 == 0) && (_action1._field24 != 0)) {
			_field1278 = arg1;
		} else if ((_action) && (_action1._field24 == 0)) {
			_action1.sub108670(1);
			_action1.signal();
		} else if (_action == 0) {
			_action1.sub108670(1);
			setAction(&_action1, &_actor1, NULL);
		}
		break;
	case 104:
		if ((_action == 0) || (_action1._field24 == 0)) {
			_action2.sub10831F(-1);
			if ((_action) && ((_action2.getActionIndex() != 0) || (_action2._field1E != -1))) {
				_action2.signal();
			} else {
				_actor9.setAction(&_action2, &_actor9, NULL);
			}
		}
		break;
	case 112:
		if ((_action) && (_action1._field24 != 0) && (_action2._field1E != -1)) {
			_field1278 = 0;
			_action1.sub108732(0);
		} else if ((_action) && (_field1278 == 0) && (_action1._field24 != 0)) {
			_field1278 = arg1;
		} else if ((_action) && (_action1._field24 == 0)) {
			_action1.sub108670(-1);
			_action1.signal();
		} else if (_action == 0) {
			_action1.sub108670(-1);
			setAction(&_action1, &_actor1, NULL);
		}
		break;
	default:
		_field1270 = arg1;
		_actor7.sub109663(arg1);
		if (_action1._field24 != 0) {
			_field1270 = 0;
		}
		break;
	}
}

void Scene3500::Action1::signal() {
	Scene3500 *scene = (Scene3500 *)R2_GLOBALS._sceneManager._scene;

	switch(_actionIndex++) {
	case 0:
		R2_GLOBALS._player.disableControl();
		scene->_field1286 = 0;
		if (scene->_field1270 != 0) {
			scene->_field1270 = 0;
			scene->_field126E = 0;
			scene->_field1272 = 0;
			scene->_rotation->_idxChange = 0;
		}
		break;
	case 1:
		if ((scene->_actor1._frame % 2) == 0) {
			setDelay(1);
			return;
		}
	// No break on purpose
	case 3:
		scene->_actor1._frameChange = _field1E;
		scene->_actor1.setFrame(scene->_actor1.changeFrame());
		setDelay(1);
		break;
	case 4: {
		int si = scene->_unkObj1.sub109C09(Common::Point(scene->_field127A + 70, scene->_field127C + 46));
		int var2 = scene->_unkObj1.sub1097C9(scene->_field127A + 70) - 70;
		int var4 = scene->_unkObj1.sub1097EF(scene->_field127C + 46) - 46;
		int di = abs(var2 - scene->_field127A);
		int var6 = abs(var4 - scene->_field127C);

		if ((scene->_actor1._frame % 2) != 0) {
			scene->_actor1._frameChange = _field1E;
			scene->_actor1.setFrame(scene->_actor1.changeFrame());
		}

		int var8 = (scene->_action1._field1E * 2 + scene->_field1276);
		if (var8 > 7)
			var8 = 1;
		else if (var8 < 1)
			var8 = 7;

		switch (var8) {
		case 0:
			if ( ((si != 2)  && (si != 3)  && (si != 6) && (si != 1) && (si != 23) && (si != 24) && (si != 4) && (si != 11))
				|| (var6 != 0)) {
				if ((si != 25) && (si != 26) && (si != 5) && (si != 14) && (si != 15))
					_field20 = 0;
				else if ((var6 != 0) || (di <= 3)) // useless, skipped: "|| (di == 0)"
					_field20 = 0;
				else
					_field20 = 1;
			} else
				_field20 = 1;
			break;
		case 2:
			if ( ((si != 12)  && (si != 13)  && (si != 11) && (si != 16) && (si != 26) && (si != 24) && (si != 15) && (si != 6) && (si != 31))
				|| (di != 0)) {
				if ((si != 25) && (si != 23) && (si != 14) && (si != 5) && (si != 4))
					_field20 = 0;
				else if ((di != 0) || (var6 <= 3)) // useless, skipped: "|| (var6 == 0)"
					_field20 = 0;
				else
					_field20 = 1;
			} else
				_field20 = 1;
			break;
		case 4:
			if ( ((si != 2)  && (si != 3)  && (si != 6) && (si != 1) && (si != 25) && (si != 26) && (si != 5) && (si != 16) && (si != 31))
				|| (var6 != 0)) {
					if ((si != 23) && (si != 24) && (si != 4) && (si != 14) && (si != 15))
						_field20 = 0;
					else if ((var6 != 0) || (di <= 3)) // useless, skipped: "|| (di == 0)"
						_field20 = 0;
					else
						_field20 = 1;
			} else
				_field20 = 1;
			break;
		case 6:
			if ( ((si != 12)  && (si != 13)  && (si != 11) && (si != 16) && (si != 25) && (si != 23) && (si != 14) && (si != 1) && (si != 31))
				|| (var6 != 0)) {
					if ((si != 26) && (si != 24) && (si != 15) && (si != 5) && (si != 4))
						_field20 = 0;
					else if ((var6 <= 0) || (di != 0)) // useless, skipped: "|| (var6 == 0)"
						_field20 = 0;
					else
						_field20 = 1;
			} else
				_field20 = 1;
		default:
			break;
		}
		}
	// No break on purpose
	case 2: {
		scene->_actor8.setPosition(Common::Point(160, 73));
		scene->_actor8._moveDiff.x = 160 - scene->_field126E;
		scene->_fieldAF8 = 160 - ((_field1E * 2) * 160);
		Common::Point pt(scene->_fieldAF8, 73);
		NpcMover *mover = new NpcMover();
		scene->_actor8.addMover(mover, &pt, this);

		scene->_actor9.setPosition(Common::Point(160 + ((_field1E * 2) * 160), 73));
		scene->_actor9._moveDiff.x = 160 - scene->_field126E;
		scene->_fieldB9E = 160;
		Common::Point pt2(scene->_fieldB9E, 73);
		NpcMover *mover2 = new NpcMover();
		scene->_actor9.addMover(mover2, &pt2, NULL);
		}
		break;
	case 5:
		scene->_actor1._frameChange = _field1E;
		scene->_field1276 = scene->_actor1.changeFrame();
		scene->_actor1.setFrame(scene->_field1276);
		setDelay(1);
		break;
	case 6:
		scene->_actor8.setPosition(Common::Point(160, 73));
		if (_field20 == 0)
			scene->_actor8.setStrip(1);
		else
			scene->_actor8.setStrip(2);
		scene->_actor8.fixPriority(1);

		scene->_actor9.setPosition(Common::Point(-160, 73));
		scene->_actor9.setStrip(9);
		scene->_actor9.fixPriority(11);
		scene->_actor9.hide();
		setDelay(1);
		break;
	case 7:
		if ((scene->_actor1._frame % 2) == 0) {
			scene->_actor1._frameChange = _field1E;
			scene->_field1276 = scene->_actor1.changeFrame();
			scene->_actor1.setFrame(scene->_field1276);
		}
		setDelay(1);
		break;
	case 8: {
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		scene->_field1286 = 1;
		if ((scene->_actor1._frame % 2) == 0) {
			scene->_actor1._frameChange = _field1E;
			scene->_actor1.setFrame(scene->_actor1.changeFrame());
		}
		// All the var_8 initialization was missing in the original
		// but it's clearly a cut and paste error from case 4.
		// The following code allows the switch to work properly.
		warning("Checkme: fix for dead code");
		int var_8 = (_field1E * 2 + scene->_field1276);
		if (var_8 > 7)
			var_8 = 1;
		else if (var_8 < 1)
			var_8 = 7;
		//

		switch (var_8 - 1) {
		case 0:
		// No break on purpose
		case 4:
			scene->_field127A = scene->_unkObj1.sub1097C9(scene->_field127A + 70) - 70;
			break;
		case 2:
		// No break on purpose
		case 6:
			scene->_field127C = scene->_unkObj1.sub1097EF(scene->_field127C + 46) - 46;
			break;
		default:
			break;
		}
		scene->_actor5.hide();
		scene->_actor6.hide();
		_field24 = 0;
		if (_field20 == 0) {
			scene->_actor7.sub1094ED();
			if (scene->_field126E == scene->_field1270)
				scene->_aSound1.play(276);
		}
		break;
		}
	case 10: {
		scene->_fieldAF8 = 160;
		Common::Point pt(160, 73);
		NpcMover *mover = new NpcMover();
		scene->_actor8.addMover(mover, &pt, NULL);

		scene->_fieldB9E = 160 - (_field1E * 2 * 160);
		Common::Point pt2(scene->_fieldB9E, 73);
		NpcMover *mover2 = new NpcMover();
		scene->_actor9.addMover(mover2, &pt2, this);
		_actionIndex = 6;
		}
		break;
	case 11: {
		scene->_actor8.setStrip(2);
		scene->_actor8.setPosition(Common::Point(160, 73));
		scene->_fieldAF8 = 160 - (_field1E * 2 * 160);
		Common::Point pt(scene->_fieldAF8, 73);
		NpcMover *mover = new NpcMover();
		scene->_actor8.addMover(mover, &pt, NULL);
		scene->_actor8.fixPriority(11);
		if (_field20 == 0)
			scene->_actor9.setStrip(1);
		else
			scene->_actor9.setStrip(2);
		scene->_actor9.setPosition(Common::Point(160 - (_field1E * 2 * 160), 73));
		scene->_fieldB9E = 160;
		Common::Point pt2(scene->_fieldB9E, 73);
		NpcMover *mover2 = new NpcMover();
		scene->_actor9.addMover(mover2, &pt2, this);
		scene->_actor9.fixPriority(1);
		_actionIndex = 5;
		}
		break;
	default:
		break;
	}
}

void Scene3500::Action1::dispatch() {
	Scene3500 *scene = (Scene3500 *)R2_GLOBALS._sceneManager._scene;

	Action::dispatch();
	if ((_actionIndex == 1) && (scene->_field126E <= 4)) {
		scene->_rotation->_idxChange = 0;
		signal();
	}
}

void Scene3500::Action2::sub10831F(int arg1) {
	Scene3500 *scene = (Scene3500 *)R2_GLOBALS._sceneManager._scene;

	_field1E = arg1;
	if (_field1E == -1)
		scene->_actor3.setFrame2(3);
	else
		scene->_actor3.setFrame2(1);

	setActionIndex(0);
}

void Scene3500::Action2::signal() {
	Scene3500 *scene = (Scene3500 *)R2_GLOBALS._sceneManager._scene;

	int si;
	int di;

	switch (_actionIndex++) {
	case 0: {
		if (scene->_actor8._mover) {
			si = scene->_fieldAF8;
			di = scene->_fieldB9E;
		} else {
			scene->_fieldAF8 = scene->_actor8._position.x;
			si = scene->_fieldAF8;
			scene->_fieldB9E = scene->_actor9._position.y;
			di = scene->_fieldB9E;
		}

		scene->_actor8._moveDiff.y = 9 - (scene->_field126E / 2);
		Common::Point pt(si, 73 - (_field1E * 12));
		NpcMover *mover = new NpcMover();
		scene->_actor8.addMover(mover, &pt, NULL);

		scene->_actor9._moveDiff.y = 9 - (scene->_field126E / 2);
		Common::Point pt2(di, 73 - (_field1E * 12));
		NpcMover *mover2 = new NpcMover();
		scene->_actor9.addMover(mover2, &pt2, NULL);
		scene->_field126E = (scene->_field126E / 2) + (scene->_field126E % 2);
		setDelay(17 - scene->_field126E);
		}
		break;
	case 1: {
		R2_GLOBALS._sound2.play(339);
		if (scene->_actor8._mover) {
			si = scene->_fieldAF8;
			di = scene->_fieldB9E;
		} else {
			si = scene->_actor8._position.x;
			di = scene->_actor9._position.x;
		}

		scene->_actor7.sub1094ED();

		scene->_actor8._moveDiff.y = 9 - (scene->_field126E / 2);
		Common::Point pt(si, 73);
		NpcMover *mover = new NpcMover();
		scene->_actor8.addMover(mover, &pt, NULL);

		scene->_actor9._moveDiff.y = 9 - (scene->_field126E / 2);
		Common::Point pt2(di, 73);
		NpcMover *mover2 = new NpcMover();
		scene->_actor9.addMover(mover2, &pt2, NULL);

		scene->_actor3.setFrame2(2);
		}
		break;
	default:
		break;
	}
}

bool Scene3500::Item4::startAction(CursorType action, Event &event) {
	Scene3500 *scene = (Scene3500 *)R2_GLOBALS._sceneManager._scene;

	if (scene->_field1286 == 0)
		return true;

	if (scene->_field1286 != 4)
		return SceneHotspot::startAction(action, event);

	R2_GLOBALS._sound2.play(14);
	scene->sub107F71(_field34);

	return true;
}

void Scene3500::Actor7::process(Event &event) {
	Scene3500 *scene = (Scene3500 *)R2_GLOBALS._sceneManager._scene;

	if (scene->_field1286 == 0)
		return;

	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._events.getCursor() == CURSOR_USE) && (_bounds.contains(event.mousePos))) {
		_fieldAE = 1 + event.mousePos.y - _position.y;
		event.eventType = EVENT_NONE;
	}

	if ((event.eventType == EVENT_BUTTON_UP) && (_fieldAE != 0)) {
		_fieldAE = 0;
		event.handled = true;
		if (scene->_action1._field24 == 0)
			sub1094ED();
	}

	if (_fieldAE == 0)
		return;

	R2_GLOBALS._sound2.play(338);
	event.handled = true;

	int cx = event.mousePos.y - _fieldAE + 1;
	if (_fieldA6 >= cx) {
		if (_fieldA6 - _fieldAA <= cx)
			sub109693(Common::Point(((_fieldA6 - cx) / 2) + _fieldA4 + ((_fieldA6 - cx) % 2), cx));
		else
			sub109693(Common::Point(_fieldA4 + _fieldA8, _fieldA6 - _fieldAA));
	} else {
		sub109693(Common::Point(_fieldA4, _fieldA6));
	}
}

bool Scene3500::Actor7::startAction(CursorType action, Event &event) {
	Scene3500 *scene = (Scene3500 *)R2_GLOBALS._sceneManager._scene;

	if (scene->_field1286 == 0)
		return true;

	if (scene->_field1286 == 4)
		return false;

	return SceneActor::startAction(action, event);
}

void Scene3500::postInit(SceneObjectList *OwnerList) {
	byte tmpPal[768];
	Rect tmpRect;

	loadScene(1050);
	R2_GLOBALS._uiElements._active = false;
	R2_GLOBALS._v5589E.set(0, 0, 320, 200);
	R2_GLOBALS._sound1.play(305);
	R2_GLOBALS._player._characterIndex = R2_QUINN;
	R2_GLOBALS._player._characterScene[1] = 3500;
	R2_GLOBALS._player._characterScene[2] = 3500;
	R2_GLOBALS._player._characterScene[3] = 3500;
	_field1284 = 0;
	_field1282 = 0;
	_field1278 = 0;
	_field1272 = 1;
	_field1270 = 4;
	_field126E = 4;
	_field127A = 860;
	_field127C = 891;
	_rotation = R2_GLOBALS._scenePalette.addRotation(240, 254, -1);
	_rotation->setDelay(0);
	_rotation->_idxChange = 1;

	for (int i = 240; i <= 254; i++) {
		int tmpIndex = _rotation->_currIndex - 240;

		if (tmpIndex > 254)
			tmpIndex--;

		tmpPal[3 * i] = R2_GLOBALS._scenePalette._palette[3 * tmpIndex];
		tmpPal[(3 * i) + 1] = R2_GLOBALS._scenePalette._palette[(3 * tmpIndex) + 1];
		tmpPal[(3 * i) + 2] = R2_GLOBALS._scenePalette._palette[(3 * tmpIndex) + 2];
	}

	for (int i = 240; i <= 254; i++) {
		R2_GLOBALS._scenePalette._palette[3 * i] = tmpPal[3 * i];
		R2_GLOBALS._scenePalette._palette[(3 * i) + 1] = tmpPal[(3 * i) + 1];
		R2_GLOBALS._scenePalette._palette[(3 * i) + 2] = tmpPal[(3 * i) + 2];
	}

	_actor7.sub109466(38, 165, 16, 32, _field1270);
	_actor7.setDetails(3500, 6, 7, -1, 1, (SceneItem *)NULL);
	R2_GLOBALS._sound1.play(276);

	_item4._field34 = 88;
	_item4.setDetails(88, 3500, 18, 10, -1);

	_item5._field34 = 112;
	_item5.setDetails(112, 3500, 9, 10, -1);

	_item6._field34 = 104;
	_item6.setDetails(104, 3500, 15, 10, -1);

	_item7._field34 = 96;
	_item7.setDetails(96, 3500, 12, 10, -1);

	_actor8.postInit();
	_actor8.setup(10501, 1, 1);
	_actor8.setPosition(Common::Point(160, 73));
	_actor8.fixPriority(1);

	_actor9.postInit();
	_actor9.setup(1050, 2, 1);
	_actor9.setPosition(Common::Point(-160, 73));
	_actor9.fixPriority(11);
	_actor9.hide();

	_item2.setDetails(27, 3500, 21, -1, -1);
	_item3.setDetails(Rect(160, 89, 299, 182), 3500, 3, -1, -1, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 3500, 0, -1, 2, 1, NULL);

	_actor1.postInit();
	_field1276 = 1;
	_actor1.setup(1004, 1, _field1276);
	_actor1.setPosition(Common::Point(230, 135));
	_actor1.fixPriority(200);
	_actor1._frameChange = 1;

	_actor5.postInit();
	_actor5.setup(1004, 3, 1);
	_actor5.setPosition(Common::Point(117, 163));
	_actor5.fixPriority(200);
	_actor5.hide();

	_actor4.postInit();
	_actor4.setup(1004, 3, 2);
	_actor4.setPosition(Common::Point(126, 163));
	_actor4.fixPriority(200);

	_actor6.postInit();
	_actor6.setup(1004, 3, 3);
	_actor6.setPosition(Common::Point(135, 163));
	_actor6.fixPriority(200);
	_actor6.hide();

	_actor2.postInit();
	_actor2.setup(1004, 4, _field126E + 1);
	_actor2.setPosition(Common::Point(126, 137));
	_actor2.fixPriority(200);

	_actor3.postInit();
	_actor3.setup(1004, 5, 2);
	_actor3.setPosition(Common::Point(126, 108));
	_actor3.fixPriority(200);

	tmpRect.set(160, 89, 299, 182);
	_unkObj1.sub9EDE8(tmpRect);
	_unkObj1.sub51AE9(2);
	_unkObj1.sub51AFD(Common::Point(_field127A, _field127C));

	_action1._field24 = 0;
	warning("gfx_set_pane_p()");
	_unkObj1.sub51B02();
	warning("gfx_set_pane_p()");
	_field1286 = 1;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.enableControl(CURSOR_USE);
	R2_GLOBALS._player._uiEnabled = false;
	R2_GLOBALS._player._canWalk = false;
}

void Scene3500::remove() {
	_rotation->remove();
	R2_GLOBALS._sound2.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene3500::signal() {
	R2_GLOBALS._player.enableControl(CURSOR_USE);
	R2_GLOBALS._player._canWalk = false;
	_field1286 = 1;
}

void Scene3500::process(Event &event) {
	if (_field1286 == 0)
		return;

	if (event.eventType == EVENT_KEYPRESS) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_1:
			warning("FIXME: keycode = 0x4700");
			R2_GLOBALS._sound2.play(338);
			sub107F71(16);
			event.handled = true;
			break;
		case Common::KEYCODE_2:
			warning("FIXME: keycode = 0x4800");
			R2_GLOBALS._sound2.play(14, NULL, 63);
			sub107F71(88);
			event.handled = true;
			break;
		case Common::KEYCODE_3:
			warning("FIXME: keycode = 0x4900");
			if (_field1270 < 16)
				R2_GLOBALS._sound2.play(338);
			sub107F71(1);
			event.handled = true;
			break;
		case Common::KEYCODE_4:
			warning("FIXME: keycode = 0x4B00");
			R2_GLOBALS._sound2.play(14, NULL, 63);
			sub107F71(112);
			event.handled = true;
			break;
		case Common::KEYCODE_5:
			warning("FIXME: keycode = 0x4D00");
			R2_GLOBALS._sound2.play(14, NULL, 63);
			sub107F71(96);
			event.handled = true;
			break;
		case Common::KEYCODE_6:
			warning("FIXME: keycode = 0x4F00");
			R2_GLOBALS._sound2.play(338);
			sub107F71(0);
			event.handled = true;
			break;
		case Common::KEYCODE_7:
			warning("FIXME: keycode = 0x5000");
			R2_GLOBALS._sound2.play(14, NULL, 63);
			sub107F71(104);
			event.handled = true;
			break;
		case Common::KEYCODE_8:
			warning("FIXME: keycode = 0x5100");
			if (_field1270 != 0)
				R2_GLOBALS._sound2.play(338);
			sub107F71(-1);
			event.handled = true;
			break;
		case Common::KEYCODE_9:
			warning("FIXME: keycode = 0x5200");
			R2_GLOBALS._sound2.play(338);
			sub107F71(8);
			event.handled = true;
			break;
		case Common::KEYCODE_0:
			warning("FIXME: keycode = 0x5300");
			R2_GLOBALS._sound2.play(338);
			sub107F71(4);
			event.handled = true;
			break;
		default:
			break;
		}
	}

	if (!event.handled)
		_actor7.process(event);

	if (!event.handled)
		_item4.process(event);

	if (!event.handled)
		_item5.process(event);

	if (!event.handled)
		_item6.process(event);

	if (!event.handled)
		_item7.process(event);

	Scene::process(event);
}

void Scene3500::dispatch() {
	Rect tmpRect;
	Scene::dispatch();
	if (((_actor1._frame % 2) == 0) && (_action1._field24 == 0)) {
		_actor1.setFrame(_actor1.changeFrame());
		_field1276 = _actor1._frame;
	}
	int oldField1278;
	if ((_field1278 != 0) && (_action1._field24 == 0)) {
		oldField1278 = _field1278;
		_field1278 = 0;
		sub107F71(oldField1278);
	}

	if (!_rotation)
		return;

	int var_field127A = 0;
	int di = 0;
	int var_4 = 0;
	int var_6 = 0;
	int var_8 = 0;
	int var_a = 0;
	int dx = 0;
	int tmpVar = 0;

	if ((_field126E == 0) && (_field1282 == 0)) {
		if (_field1284 == 2)
			R2_GLOBALS._sceneManager.changeScene(1000);
	} else {
		_field1282 = 0;
		tmpRect.set(160, 89, 299, 182);

		var_field127A = _field127A;
		di = _field127C;
		var_4 = _unkObj1.sub1097C9(70) - 70;
		var_6 = _unkObj1.sub1097EF(_field127C + 46) - 46;
		var_8 = abs(var_4 - var_field127A);
		var_a = abs(var_6 - di);
		dx = 0;

		switch (_field1276) {
		case 0:
			tmpVar = _unkObj1.sub109C09(Common::Point(var_field127A + 70, 46));
			if (    ((tmpVar == 2) || (tmpVar == 3) || (tmpVar == 6) || (tmpVar == 1))
				|| (((tmpVar == 25) || (tmpVar == 26) || (tmpVar == 5) || (tmpVar == 14) || (tmpVar == 15)) && (var_8 > 3)) ) {
				R2_GLOBALS._sound2.play(339);
				_rotation->_idxChange = 0;
				_field1270 = 0;
				_field126E = 0;
				_field1272 = 0;
				if (_action1._field24 == 0)
					_actor8.hide();
			} else {
				var_6 = _unkObj1.sub1097EF(di + 46) - 46;
				di = _field127C - _field126E;
				dx = _unkObj1.sub109C09(Common::Point(var_field127A + 70, di + 46));
				if (((tmpVar == 23) || (tmpVar == 24) || (tmpVar == 4)) && (tmpVar != dx)) {
					di = var_6;
					R2_GLOBALS._sound2.play(339);
					_rotation->_idxChange = 0;
					_field1270 = 0;
					_field126E = 0;
					_field1272 = 0;
					if (_action1._field24 == 0)
						_actor8.hide();
				} else if ((tmpVar == 11) && (tmpVar != dx)) {
					di = var_6 + 3;
					R2_GLOBALS._sound2.play(339);
					_rotation->_idxChange = 0;
					_field1270 = 0;
					_field126E = 0;
					_field1272 = 0;
					if (_action1._field24 == 0)
						_actor8.hide();
				} else {
					var_6 = _unkObj1.sub1097EF(di + 46) - 46;
					var_a = abs(var_6 - di);
					tmpVar = _unkObj1.sub109C09(Common::Point(var_field127A + 70, di + 46));

					if ( (((tmpVar == 23) || (tmpVar == 24) || (tmpVar == 4)) && (di <= var_6) && (_field127C>= var_6))
						|| (((tmpVar == 25) || (tmpVar == 26) || (tmpVar == 5) || (tmpVar == 14) || (tmpVar == 15)) && (_field126E >= var_a) && (_field126E > 3) && (_action1._field24 != 0)) ) {
						di = var_6;
						if ((tmpVar != 25) && (tmpVar != 26) && (tmpVar != 5) && (tmpVar != 14) && (tmpVar == 15))
							R2_GLOBALS._sound2.play(339);
						_rotation->_idxChange = 0;
						_field1270 = 0;
						_field126E = 0;
						_field1272 = 0;
						if (_action1._field24 == 0)
							_actor8.hide();
					} else if ((tmpVar == 11) && (var_6 + 3 >= di) && (_field127C >= var_6 + 3)) {
						R2_GLOBALS._sound2.play(339);
						_rotation->_idxChange = 0;
						_field1270 = 0;
						_field126E = 0;
						_field1272 = 0;
						if (_action1._field24 == 0)
							_actor8.hide();
					} else if (((tmpVar == 25) || (tmpVar == 26) || (tmpVar == 5) || (tmpVar == 14) || (tmpVar == 15)) && (var_8 != 0) && (var_8 <= 3)) {
						var_field127A = var_4;
						R2_GLOBALS._sound2.play(339);
					} else {
						// Nothing
					}
				}
			}
			break;
		case 2:
			tmpVar = _unkObj1.sub109C09(Common::Point(var_field127A + 70, di + 46));
			if (  ((tmpVar == 12) || (tmpVar == 13) || (tmpVar == 11) || (tmpVar == 16) || (tmpVar == 31))
			  || (((tmpVar == 25) || (tmpVar == 23) || (tmpVar == 14) || (tmpVar == 5) || (tmpVar == 4)) && (var_a > 3)) ) {
				R2_GLOBALS._sound2.play(339);
				_rotation->_idxChange = 0;
				_field1270 = 0;
				_field126E = 0;
				_field1272 = 0;
				if (_action1._field24 == 0)
					_actor8.hide();
			} else {
				var_4 = _unkObj1.sub1097C9(var_field127A + 70) - 70;
				var_field127A = _field127A + _field126E;
				dx = _unkObj1.sub109C09(Common::Point(var_field127A + 70, di + 46));
				if (((tmpVar == 26) || (tmpVar == 24) || (tmpVar == 15)) && (tmpVar != dx)) {
					var_field127A = var_4;
					R2_GLOBALS._sound2.play(339);
					_rotation->_idxChange = 0;
					_field1270 = 0;
					_field126E = 0;
					_field1272 = 0;
					if (_action1._field24 == 0)
						_actor8.hide();
				} else if ((tmpVar == 6) && (tmpVar != dx)) {
					var_field127A = var_4 - 5;
					R2_GLOBALS._sound2.play(339);
					_rotation->_idxChange = 0;
					_field1270 = 0;
					_field126E = 0;
					_field1272 = 0;
					if (_action1._field24 == 0)
						_actor8.hide();
				} else {
					var_4 = _unkObj1.sub1097C9(var_field127A + 70) - 70;
					var_8 = abs(var_field127A - var_4);
					tmpVar = _unkObj1.sub109C09(Common::Point(var_field127A + 70, tmpVar + 46));
					if ( (((tmpVar == 26) || (tmpVar == 24) || (tmpVar == 15)) && (var_field127A >= var_4) && (_field127A <= var_4))
						|| (((tmpVar == 25) || (tmpVar == 23) || (tmpVar == 14) || (tmpVar == 5) || (tmpVar == 4)) && (_field126E >= var_8) && (_field126E <= 3) && (_action1._field24 != 0)) ) {
						var_field127A = var_4;
						if ((tmpVar == 25) || (tmpVar == 23) || (tmpVar == 14) || (tmpVar == 5) || (tmpVar == 4))
							R2_GLOBALS._sound2.play(339);
						_rotation->_idxChange = 0;
						_field1270 = 0;
						_field126E = 0;
						_field1272 = 0;
						if (_action1._field24 == 0)
							_actor8.hide();
					} else if ((tmpVar == 6) && (var_4 - 5 <= var_field127A) && (_field127A <= var_4 - 5)) {
						var_field127A = var_4 - 5;
						R2_GLOBALS._sound2.play(339);
						_rotation->_idxChange = 0;
						_field1270 = 0;
						_field126E = 0;
						_field1272 = 0;
						if (_action1._field24 == 0)
							_actor8.hide();
					} else if (((tmpVar == 25) || (tmpVar == 23) || (tmpVar == 14) || (tmpVar == 5) || (tmpVar == 4)) && (var_a != 0) && (var_a <= 3)) {
						di = var_6;
						R2_GLOBALS._sound2.play(339);
					} else {
						// Nothing
					}
				}
			}
			break;
		case 4:
			tmpVar = _unkObj1.sub109C09(Common::Point(var_field127A + 70, di + 46));
			if (  ((tmpVar == 2) || (tmpVar == 3) || (tmpVar == 6) || (tmpVar == 1))
			  || (((tmpVar == 23) || (tmpVar == 24) || (tmpVar == 4) || (tmpVar == 14) || (tmpVar == 15)) && (var_8 > 3)) ) {
				R2_GLOBALS._sound2.play(339);
				_rotation->_idxChange = 0;
				_field1270 = 0;
				_field126E = 0;
				_field1272 = 0;
				if (_action1._field24 == 0)
					_actor8.hide();
			} else {
				var_6 = _unkObj1.sub1097EF(di + 46) - 46;
				di = _field127C + _field126E;
				dx = _unkObj1.sub109C09(Common::Point(var_field127A + 70, di + 46));
				if (((tmpVar == 25) || (tmpVar == 26) || (tmpVar == 5)) && (tmpVar == dx)) {
					R2_GLOBALS._sound2.play(339);
					_rotation->_idxChange = 0;
					_field1270 = 0;
					_field126E = 0;
					_field1272 = 0;
					if (_action1._field24 == 0)
						_actor8.hide();
				} else if ((tmpVar == 16) && (tmpVar == dx)) {
					di = var_6 - 3;
					R2_GLOBALS._sound2.play(339);
					_rotation->_idxChange = 0;
					_field1270 = 0;
					_field126E = 0;
					_field1272 = 0;
					if (_action1._field24 == 0)
						_actor8.hide();
				} else if ((tmpVar == 31) && (tmpVar == dx)) {
					di = var_6 + 4;
					R2_GLOBALS._sound2.play(339);
					_rotation->_idxChange = 0;
					_field1270 = 0;
					_field126E = 0;
					_field1272 = 0;
					if (_action1._field24 == 0)
						_actor8.hide();
				} else {
					var_6 = _unkObj1.sub1097EF(di + 46) - 46;
					var_a = abs(di - var_6);
					tmpVar = _unkObj1.sub109C09(Common::Point(var_field127A + 70, di + 46));
					if ( (((tmpVar == 25) || (tmpVar == 26) || (tmpVar == 5)) && (di >= var_6) && (_field127C <= var_6))
					  || (((tmpVar == 23) || (tmpVar == 24) || (tmpVar == 4) || (tmpVar == 14) || (tmpVar == 15)) && (_field126E >= var_a) && (_field126E <= 3) && (_action1._field24 != 0)) ){
						if ((tmpVar != 23) && (tmpVar != 24) && (tmpVar != 4) && (tmpVar != 14) && (tmpVar != 15))
							R2_GLOBALS._sound2.play(339);
						_rotation->_idxChange = 0;
						_field1270 = 0;
						_field126E = 0;
						_field1272 = 0;
						if (_action1._field24 == 0)
							_actor8.hide();
					} else if ((tmpVar == 16) && (var_6 - 3 <= di) && (_field127C <= var_6 - 3)) {
						di = var_6 - 3;
						R2_GLOBALS._sound2.play(339);
						_rotation->_idxChange = 0;
						_field1270 = 0;
						_field126E = 0;
						_field1272 = 0;
						if (_action1._field24 == 0)
							_actor8.hide();
					} else if ((tmpVar == 31) && (var_6 + 4 <= di) && (_field127C <= var_6 + 4)) {
						di = var_6 + 4;
						_rotation->_idxChange = 0;
						_field1270 = 0;
						_field126E = 0;
						_field1272 = 0;
						if (_action1._field24 == 0)
							_actor8.hide();
						if ((var_field127A == 660) && (_field126E + 306 <= di) && (di <= 307))
							 ++_field1284;
						else
							R2_GLOBALS._sound2.play(339);
					} else if (((tmpVar == 23) || (tmpVar == 24) || (tmpVar == 4) || (tmpVar == 14) || (tmpVar == 15)) && (var_8 != 0) && (var_8 <= 3)) {
						var_field127A = var_4;
						R2_GLOBALS._sound2.play(339);
					} else {
						// Nothing
					}
				}
			}
			break;
		case 6:
			tmpVar = _unkObj1.sub109C09(Common::Point(var_field127A + 70, di + 46));
			if ( ((tmpVar == 12) || (tmpVar == 13) || (tmpVar == 11) || (tmpVar == 16) || (tmpVar == 31))
			 || (((tmpVar == 26) || (tmpVar == 24) || (tmpVar == 15) || (tmpVar == 5) || (tmpVar == 4)) && (var_a > 3)) ) {
				R2_GLOBALS._sound2.play(339);
				_rotation->_idxChange = 0;
				_field1270 = 0;
				_field126E = 0;
				_field1272 = 0;
				if (_action1._field24 == 0)
					_actor8.hide();
			} else {
				var_4 = _unkObj1.sub1097C9(var_field127A + 70) - 70;
				var_field127A = _field127A - _field126E;
				dx = _unkObj1.sub109C09(Common::Point(var_field127A + 70, di + 46));
				if (((tmpVar == 25) || (tmpVar == 23) || (tmpVar == 14)) && (tmpVar != dx)) {
					var_field127A = var_4;
					R2_GLOBALS._sound2.play(339);
					_rotation->_idxChange = 0;
					_field1270 = 0;
					_field126E = 0;
					_field1272 = 0;
					if (_action1._field24 == 0)
						_actor8.hide();
				} else if ((tmpVar == 1) && (tmpVar != dx)) {
					var_field127A = var_4 + 5;
					R2_GLOBALS._sound2.play(339);
					_rotation->_idxChange = 0;
					_field1270 = 0;
					_field126E = 0;
					_field1272 = 0;
					if (_action1._field24 == 0)
						_actor8.hide();
				} else {
					var_4 = _unkObj1.sub1097C9(var_field127A + 70) - 70;
					var_8 = abs(var_4 - var_field127A);
					tmpVar = _unkObj1.sub109C09(Common::Point(var_field127A + 70, di + 46));
					if ( (((tmpVar == 25) || (tmpVar == 23) || (tmpVar == 14)) && (var_field127A <= var_4) && (_field127A >= var_4))
					  || (((tmpVar == 26) || (tmpVar == 24) || (tmpVar == 15) || (tmpVar == 5) || (tmpVar == 4)) && (_field126E >= var_8) && (_field126E <= 3) && (_action1._field24 != 0)) ) {
						var_field127A = var_4;
						if ((tmpVar == 26) || (tmpVar == 24) || (tmpVar == 15) || (tmpVar == 5) || (tmpVar == 4))
							R2_GLOBALS._sound2.play(339);
						_rotation->_idxChange = 0;
						_field1270 = 0;
						_field126E = 0;
						_field1272 = 0;
						if (_action1._field24 == 0)
							_actor8.hide();
					} else if ((tmpVar == 1) && (var_field127A >= var_4 + 5) && (_field127A >= var_4 + 5)) {
						var_field127A = var_4 + 5;
						R2_GLOBALS._sound2.play(339);
						_rotation->_idxChange = 0;
						_field1270 = 0;
						_field126E = 0;
						_field1272 = 0;
						if (_action1._field24 == 0)
							_actor8.hide();
					} else if (((tmpVar == 26) || (tmpVar == 24) || (tmpVar == 15) || (tmpVar == 5) || (tmpVar == 4)) && (var_a != 0) && (var_a <= 3)) {
						di = var_6;
						R2_GLOBALS._sound2.play(339);
					} else {
						// Nothing
					}
				}
			}
			break;
		default:
			break;
		}

		if (_field1284 < 2) {
			_field127A = var_field127A;
			_field127C = di;
			if (_unkObj1.sub109C5E(_field127A, _field127C) != 0) {
				_field1272 = 0;
				_field126E = 0;
				_field1270 = 0;
				_rotation->setDelay(0);
				_rotation->_idxChange = 0;
			}
			warning("gfx_set_pane_p");
			_unkObj1.sub51B02();
			if (_field1284 != 0)
				++_field1284;
		}
	}

	if (_field1272 == 0) {
		if (_field126E != _field1270) {
			if (_field126E >= _field1270) {
				if (_field126E == 1) {
					if (_action1._field24 != 0) {
						if ( ((_field1276 == 1) && (var_8 == 0) && (var_a != 0) && (var_a <= 3) && ((tmpVar == 25) || (tmpVar == 26) || (tmpVar == 5) || (tmpVar == 14) || (tmpVar == 15)))
						  || ((_field1276 == 3) && (var_a == 0) && (var_8 != 0) && (var_8 <= 3) && ((tmpVar == 25) || (tmpVar == 23) || (tmpVar == 14) || (tmpVar == 5) || (tmpVar == 4)))
						  || ((_field1276 == 5) && (var_8 == 0) && (var_a != 0) && (var_a <= 3) && ((tmpVar == 23) || (tmpVar == 24) || (tmpVar == 4) || (tmpVar == 14) || (tmpVar == 15)))
						  || ((_field1276 == 7) && (var_a == 0) && (var_8 != 0) && (var_8 <= 3) && ((tmpVar == 26) || (tmpVar == 24) || (tmpVar == 15) || (tmpVar == 5) || (tmpVar == 4))) ){
							_field126E = 1;
						} else
							_field126E--;
					} else
						_field126E--;
				} else
					_field126E--;
			} else
				++_field126E;
			_field1272 = 1;
		}
		_actor2.setFrame2(_field126E);
	}

	if (_field1272 == 1) {
		if (_field126E == 0)
			_rotation->_idxChange = 0;
		else if (_field126E > 8)
			_rotation->_idxChange = 2;
		else
			_rotation->_idxChange = 1;
	}

	if (_field1272 != 0)
		_field1272--;

	if (_field126E != 0) {
		R2_GLOBALS._player._uiEnabled = false;
		if (_field126E != _field1270)
			_aSound1.play(276);
	} else {
		R2_GLOBALS._player._uiEnabled = true;
		_aSound1.fadeOut2(NULL);
	}

	if (_rotation->_currIndex != _field1274)
		_field1274 = _rotation->_currIndex;
}

/*--------------------------------------------------------------------------
 * Scene 3600 -
 *
 *--------------------------------------------------------------------------*/
Scene3600::Scene3600() {
	_field2548 = 0;
	_field254A = 0;
	_field254C = 0;
	_field254E = 0;
	_field2550 = false;
}
void Scene3600::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field2548);
	s.syncAsSint16LE(_field254A);
	s.syncAsSint16LE(_field254C);
	s.syncAsSint16LE(_field254E);
	s.syncAsSint16LE(_field2550);
}

Scene3600::Action3600::Action3600() {
	_field1E = 0;
	_field20 = 0;
}

void Scene3600::Action3600::synchronize(Serializer &s) {
	Action::synchronize(s);

	s.syncAsSint16LE(_field1E);
	s.syncAsSint16LE(_field20);
}

void Scene3600::Action3600::signal() {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 0:
		++_actionIndex;
		setDelay(60);
		break;
	case 1:
		if (_field1E == 0) {
			_field1E = 1;
			scene->_actor2.setAction(NULL);
			R2_GLOBALS._sound2.play(330, NULL, 0);
			R2_GLOBALS._sound2.fade(127, 5, 10, false, NULL);
		}
		setDelay(1);
		warning("TODO: Palette fader using parameter 2 = 256");
		R2_GLOBALS._scenePalette.fade((const byte *)&scene->_palette1._palette, true, _field20);
		if (_field20 > 0)
			_field20 -= 2;
		break;
	case 2:
		R2_GLOBALS._sound2.stop();
		++_actionIndex;
		setDelay(3);
		break;
	case 3:
		R2_GLOBALS._sound2.play(330, this, 0);
		R2_GLOBALS._sound2.fade(127, 5, 10, false, NULL);
		_actionIndex = 1;
		break;
	default:
		break;
	}
}

void Scene3600::Action2::signal() {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 3621:
		R2_GLOBALS._events.proc1();
		R2_GLOBALS._player.enableControl();
		_actionIndex = 3619;
		scene->_actor13._state = 0;
	// No break on purpose
	case 3619: {
		++_actionIndex;
		scene->_actor13.setup(3127, 2, 1);
		scene->_actor13.animate(ANIM_MODE_1, NULL);
		NpcMover *mover = new NpcMover();
		scene->_actor13.addMover(mover, &scene->_actor13._field8A, scene);
		}
		break;
	default:
		_actionIndex = 3619;
		setDelay(360);
		break;
	}
}

bool Scene3600::Item5::startAction(CursorType action, Event &event) {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	if ((action != CURSOR_USE) || (scene->_action1._field1E == 0))
		return SceneItem::startAction(action, event);

	R2_GLOBALS._walkRegions.disableRegion(2);
	R2_GLOBALS._walkRegions.disableRegion(7);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 3624;
	scene->_actor10.setStrip2(-1);
	scene->_actor11.setStrip2(-1);
	scene->_actor12.setStrip2(-1);
	scene->_actor4.setStrip2(-1);

	if (R2_GLOBALS._player._characterIndex == 2)
		R2_GLOBALS._player.setAction(&scene->_sequenceManager3, scene, 3611, &R2_GLOBALS._player, NULL);
	else if (R2_GLOBALS._player._characterIndex == 3)
		R2_GLOBALS._player.setAction(&scene->_sequenceManager4, scene, 3612, &R2_GLOBALS._player, NULL);
	else
		R2_GLOBALS._player.setAction(&scene->_sequenceManager2, scene, 3610, &R2_GLOBALS._player, NULL);

	return true;
}

bool Scene3600::Actor13::startAction(CursorType action, Event &event) {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	switch(action) {
	case CURSOR_TALK:
		if (!_action)
			return SceneActor::startAction(action, event);

		scene->_protectorSpeaker._displayMode = 1;
		if (!R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);
		if (!scene->_actor10._mover)
			scene->_actor10.addMover(NULL);
		if (!scene->_actor11._mover)
			scene->_actor11.addMover(NULL);
		if (!scene->_actor12._mover)
			scene->_actor12.addMover(NULL);
		if (!scene->_actor4._mover)
			scene->_actor4.addMover(NULL);

		setup(3127, 2, 1);
		scene->_sceneMode = 3327;
		scene->_stripManager.start(3327, scene);

		return true;
	case R2_SONIC_STUNNER:
	// No break on purpose
	case R2_PHOTON_STUNNER:
		if (action == R2_SONIC_STUNNER)
			R2_GLOBALS._sound3.play(43);
		else
			R2_GLOBALS._sound3.play(99);
		if (_state != 0) {
			_state = 1;
			setup(3128, 1, 1);
			addMover(NULL);
		}
		scene->_action2.setActionIndex(3621);

		if (!_action)
			setAction(&scene->_action2, scene, NULL);

		animate(ANIM_MODE_5, &scene->_action2);
		R2_GLOBALS._player.disableControl();
		return true;
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

void Scene3600::postInit(SceneObjectList *OwnerList) {
	if (R2_GLOBALS._sceneManager._previousScene == 3600) {
		R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
		R2_GLOBALS._v558B6.set(60, 0, 260, 200);
	} else {
		R2_GLOBALS._scrollFollower = &_actor2;
		g_globals->gfxManager()._bounds.moveTo(Common::Point(160, 0));
		R2_GLOBALS._v558B6.set(25, 0, 260, 200);
	}

	loadScene(3600);
	SceneExt::postInit();
	_field254C = 0;

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);
	_stripManager.addSpeaker(&_tealSpeaker);
	_stripManager.addSpeaker(&_protectorSpeaker);

	setZoomPercents(142, 80, 167, 105);
	R2_GLOBALS._player._characterScene[1] = 3600;
	R2_GLOBALS._player._characterScene[2] = 3600;
	R2_GLOBALS._player._characterScene[3] = 3600;

	_item2.setDetails(33, 3600, 6, -1, -1);
	_item3.setDetails(Rect(3, 3, 22, 45), 3600, 9, -1, -1, 1, NULL);
	_item4.setDetails(Rect(449, 3, 475, 45), 3600, 9, -1, -1, 1, NULL);

	_actor10.postInit();
	_actor10._moveDiff = Common::Point(3, 2);
	_actor10.changeZoom(-1);
	_actor10._effect = 1;

	if (R2_GLOBALS._player._characterIndex != 1)
		_actor10.setDetails(9001, 0, -1, -1, 1, (SceneItem *) NULL);

	_actor11.postInit();
	_actor11._numFrames = 7;
	_actor11._moveDiff = Common::Point(5, 3);
	_actor11.changeZoom(-1);
	_actor11._effect = 1;

	if (R2_GLOBALS._player._characterIndex != 2)
		_actor11.setDetails(9002, 1, -1, -1, 1, (SceneItem *) NULL);

	_actor12.postInit();
	_actor12._moveDiff = Common::Point(3, 2);
	_actor12.changeZoom(-1);
	_actor12._effect = 1;

	if (R2_GLOBALS._player._characterIndex != 3)
		_actor12.setDetails(9003, 1, -1, -1, 1, (SceneItem *) NULL);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.changeZoom(-1);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_actor4.postInit();
	_actor4._numFrames = 7;
	_actor4._moveDiff = Common::Point(5, 3);
	_actor4.changeZoom(-1);
	_actor4._effect = 1;
	_actor4.setDetails(3600, 27, -1, -1, 1, (SceneItem *) NULL);

	_actor5.postInit();
	_actor5._numFrames = 7;
	_actor5._moveDiff = Common::Point(3, 2);
	_actor5.changeZoom(-1);
	_actor5._effect = 1;
	_actor5.setDetails(3600, 12, -1, -1, 1, (SceneItem *) NULL);

	_palette1.loadPalette(0);
	_palette1.loadPalette(3601);

	if (R2_GLOBALS._sceneManager._previousScene == 3600) {
		_item5._sceneRegionId = 200;
		_item5.setDetails(3600, 30, -1, -1, 5, &_actor4);
		_field254A = 1;
		_field2548 = 1;

		R2_GLOBALS._walkRegions.enableRegion(2);
		R2_GLOBALS._walkRegions.enableRegion(7);
		R2_GLOBALS._walkRegions.enableRegion(14);
		R2_GLOBALS._walkRegions.enableRegion(15);
		R2_GLOBALS._walkRegions.enableRegion(16);

		_actor10.setup(10, 5, 11);
		_actor10.animate(ANIM_MODE_1, NULL);

		_actor11.setup(20, 5, 11);
		_actor11.animate(ANIM_MODE_1, NULL);

		_actor12.setup(30, 5, 11);
		_actor12.animate(ANIM_MODE_1, NULL);

		if (R2_GLOBALS._player._characterIndex == 2) {
			_actor10.setPosition(Common::Point(76, 148));
			_actor11.setPosition(Common::Point(134, 148));
			_actor12.setPosition(Common::Point(100, 148));
			R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
			R2_GLOBALS._player.setup(20, _actor11._strip, 1);
			R2_GLOBALS._player.setPosition(_actor11._position);
			_actor11.hide();
		} else if (R2_GLOBALS._player._characterIndex == 3) {
			_actor10.setPosition(Common::Point(110, 148));
			_actor11.setPosition(Common::Point(76, 148));
			_actor12.setPosition(Common::Point(134, 148));
			R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
			R2_GLOBALS._player.setup(30, _actor12._strip, 1);
			R2_GLOBALS._player.setPosition(_actor12._position);
			_actor12.hide();
		} else {
			_actor10.setPosition(Common::Point(134, 148));
			_actor11.setPosition(Common::Point(76, 148));
			_actor12.setPosition(Common::Point(110, 148));
			R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
			R2_GLOBALS._player.setup(10, _actor10._strip, 1);
			R2_GLOBALS._player.setPosition(_actor10._position);
			_actor10.hide();
		}
		_actor4.setPosition(Common::Point(47, 149));
		_actor4.setup(40, 1, 11);
		_actor4.animate(ANIM_MODE_1, NULL);

		_actor5.setPosition(Common::Point(367, 148));
		_actor5.setup(3601, 7, 5);

		if (!R2_GLOBALS.getFlag(71)) {
			_actor13.postInit();
			_actor13._state = 0;
			_actor13._field8A = Common::Point(226, 152);
			_actor13._moveDiff = Common::Point(3, 2);
			_actor13.setPosition(Common::Point(284, 152));
			_actor13.setup(3127, 2, 1);
			_actor13.changeZoom(-1);
			_actor13.setDetails(3600, 15, -1, 17, 1, (SceneItem *) NULL);
		}

		R2_GLOBALS._sound2.play(330);
		_actor3.postInit();
		_actor3.setPosition(Common::Point(84, 156));
		_actor3.fixPriority(158);
		_actor3.setup(3601, 5, 1);
		_actor3.animate(ANIM_MODE_2, NULL);

		_action1._field1E = 1;
		_action1._field20 = 0;
		_action1.setActionIndex(1);

		_actor3.setAction(&_action1);
		_sceneMode = 3623;

		g_globals->_events.setCursor(CURSOR_ARROW);
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
	} else {
		_field254A = 0;
		_field2548 = 0;

		R2_GLOBALS._walkRegions.enableRegion(17);
		R2_GLOBALS._walkRegions.enableRegion(18);

		_actor10.setPosition(Common::Point(393, 148));
		_actor11.setPosition(Common::Point(364, 153));
		_actor12.setPosition(Common::Point(413, 164));

		R2_GLOBALS._player.hide();

		_actor4.setPosition(Common::Point(373, 164));

		_actor5.setup(3403, 8, 11);
		_actor5.setPosition(Common::Point(403, 155));

		_actor12.setup(3403, 7, 1);

		_actor13.setPosition(Common::Point(405, 155));

		_actor2.postInit();
		_actor2.setup(3600, 2, 1);
		_actor2.setPosition(Common::Point(403, 161));
		_actor2.fixPriority(149);
		_actor2.changeZoom(-1);

		_action1._field1E = 0;
		_action1._field20 = 90;

		_sceneMode = 3600;
		setAction(&_sequenceManager1, this, 3600, &_actor11, &_actor10, &_actor12, &_actor4, &_actor5, &_actor2, NULL);
		_field254E = 0;
	}
	_field254E = 0;
	_field2550 = R2_GLOBALS.getFlag(71);

	R2_GLOBALS._sound1.play(326);
	_item1.setDetails(Rect(0, 0, 480, 200), 3600, 0, -1, -1, 1, NULL);
}

void Scene3600::remove() {
	_actor3.animate(ANIM_MODE_NONE, NULL);
	_actor3.setAction(NULL);
	R2_GLOBALS._sound2.fadeOut2(NULL);
	R2_GLOBALS._sound1.fadeOut2(NULL);
	R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
	SceneExt::remove();
}

void Scene3600::signal() {
	switch (_sceneMode) {
	case 3320:
		warning("STUB: sub_1D227()");
		R2_GLOBALS._walkRegions.enableRegion(14);
		R2_GLOBALS._scrollFollower = &_actor11;
		_tealSpeaker._object1.hide();
		_actor5.show();
		_actor5.setStrip(2);
		if (R2_GLOBALS._player._characterIndex == 2)
			_sceneMode = 3602;
		else if (R2_GLOBALS._player._characterIndex == 3)
			_sceneMode = 3603;
		else
			_sceneMode = 3601;
		setAction(&_sequenceManager1, this, _sceneMode, &_actor11, &_actor10, &_actor12, &_actor4, &_actor5, NULL);
		break;
	case 3321:
		warning("STUB: sub_1D227()");
		R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
		_tealSpeaker.proc16();
		_actor5.show();
		_actor5.setStrip(1);
		_actor3.postInit();
		_sceneMode = 3604;
		setAction(&_sequenceManager1, this, _sceneMode, &_actor5, &_actor3, &_actor10, &_actor11, &_actor12, &_actor4, NULL);
		break;
	case 3322:
		warning("STUB: sub_1D227()");
		_quinnSpeaker.proc16();
		_quinnSpeaker._displayMode = 1;
		_tealSpeaker.proc16();
		_tealSpeaker._displayMode = 7;
		R2_GLOBALS._scrollFollower = &_actor5;
		_sceneMode = 3605;
		setAction(&_sequenceManager1, this, _sceneMode, &_actor5, &_actor13, &_actor2, NULL);
		break;
	case 3323:
		if (_field254A == 0)
			_field254A = 1;
		else {
			warning("STUB: sub_1D227()");
			_protectorSpeaker.proc16();
			_actor13.show();
			_actor13.setup(3258, 6, 1);
			_sceneMode = 3607;
			_actor13.setAction(&_sequenceManager1, this, _sceneMode, &_actor13, NULL);
			R2_GLOBALS._v558C2 = 1;
			_protectorSpeaker.proc16();
			_protectorSpeaker._displayMode = 1;
			_quinnSpeaker._displayMode = 1;
			_actor13.show();
			R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
			R2_GLOBALS._walkRegions.disableRegion(17);
			R2_GLOBALS._walkRegions.disableRegion(18);
			R2_GLOBALS._walkRegions.enableRegion(2);
			R2_GLOBALS._walkRegions.enableRegion(7);
			R2_GLOBALS._walkRegions.enableRegion(14);
			R2_GLOBALS._walkRegions.enableRegion(15);
			R2_GLOBALS._walkRegions.enableRegion(16);
			_actor13.setAction(&_action1);
		}
		break;
	case 3324:
	// No break on purpose
	case 3607:
		g_globals->_events.setCursor(CURSOR_ARROW);
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		_actor13.fixPriority(-1);
		_sceneMode = 3623;
		_field2548 = 1;
		break;
	case 3327:
		g_globals->_events.setCursor(CURSOR_ARROW);
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		_sceneMode = 3623;
		break;
	case 3450:
		R2_GLOBALS._sound1.stop();
		_actor1.hide();
		_actor6.hide();
		g_globals->gfxManager()._bounds.moveTo(Common::Point(40, 0));
		setZoomPercents(142, 80, 167, 105);
		loadScene(3600);
		R2_GLOBALS._uiElements.show();
		_item5._sceneRegionId = 200;
		_item5.setDetails(3600, 30, -1, -1, 5, &_actor4);

		_actor3.show();
		_actor10.show();
		_actor11.show();
		_actor12.show();
		_actor4.show();
		_actor5.show();

		_actor5.setPosition(Common::Point(298, 151));

		_actor13.postInit();
		_actor13._state = 0;
		_actor13._field8A = Common::Point(226, 152);
		_actor13._moveDiff = Common::Point(5, 3);
		_actor13.setup(3403, 7, 1);
		_actor13.setPosition(Common::Point(405, 155));
		_actor13.changeZoom(-1);
		_actor13.addMover(NULL);
		_actor13.animate(ANIM_MODE_NONE);
		_actor13.hide();
		_actor13.setDetails(3600, 15, -1, 17, 5, &_item5);

		_actor2.setup(3600, 2, 1);
		_actor2.setPosition(Common::Point(403, 161));
		_actor2.fixPriority(149);
		_actor2.changeZoom(-1);
		_actor2.show();

		_quinnSpeaker._displayMode = 2;
		_tealSpeaker._displayMode = 2;

		if (R2_GLOBALS._player._characterIndex == 2) {
			R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
			R2_GLOBALS._player.setup(20, _actor11._strip, 1);
			R2_GLOBALS._player.setPosition(_actor11._position);
			_actor11.hide();
		} else if (R2_GLOBALS._player._characterIndex == 3) {
			R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
			R2_GLOBALS._player.setup(30, _actor12._strip, 1);
			R2_GLOBALS._player.setPosition(_actor12._position);
			_actor12.hide();
		} else {
			R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
			R2_GLOBALS._player.setup(10, _actor10._strip, 1);
			R2_GLOBALS._player.setPosition(_actor10._position);
			_actor10.hide();
		}
		R2_GLOBALS._player.show();
		R2_GLOBALS._sound1.play(326);
		_sceneMode = 3322;
		_stripManager.start(3322, this);
		R2_GLOBALS._sound2.play(329);
		break;
	case 3600:
		_sceneMode = 3320;
		_stripManager.start(3320, this);
		break;
	case 3601:
	// No break on purpose
	case 3602:
	// No break on purpose
	case 3603:
		R2_GLOBALS._walkRegions.enableRegion(2);
		R2_GLOBALS._walkRegions.enableRegion(7);
		R2_GLOBALS._v558B6.set(60, 0, 260, 200);
		_tealSpeaker._displayMode = 1;
		_sceneMode = 3321;
		_stripManager.start(3321, this);
		break;
	case 3604:
		R2_GLOBALS._sound2.fadeOut2(NULL);
		R2_GLOBALS._sound1.stop();
		R2_GLOBALS._walkRegions.disableRegion(2);
		R2_GLOBALS._walkRegions.disableRegion(7);

		_actor2.hide();
		_actor3.hide();
		R2_GLOBALS._player.hide();
		_actor10.hide();
		_actor11.hide();
		_actor12.hide();
		_actor4.hide();
		_actor5.hide();

		g_globals->gfxManager()._bounds.moveTo(Common::Point(60, 0));
		setZoomPercents(51, 46, 180, 200);

		loadScene(3400);
		R2_GLOBALS._uiElements.show();
		_actor1.postInit();

		_actor2.setup(3403, 1, 1);
		_actor2.setPosition(Common::Point(190, 103));
		_actor2.fixPriority(89);
		_actor2.show();

		_actor6.postInit();
		_actor6.setup(3400, 1, 6);
		_actor6.setPosition(Common::Point(236, 51));
		_actor6.fixPriority(51);
		R2_GLOBALS._scrollFollower = &_actor6;

		R2_GLOBALS._sound1.play(323);
		_sceneMode = 3450;
		setAction(&_sequenceManager1, this, 3450, &_actor1, &_actor6, NULL);
		break;
	case 3605:
		_actor13.setup(3258, 4, 1);
		_actor13.setAction(&_sequenceManager1, this, 3606, &_actor5, &_actor13, &_actor2, NULL);
		_sceneMode = 3323;
		_stripManager.start(3323, this);

		break;
	case 3620:
	// No break on purpose
	case 3623:
		if ((_actor13._position.x == 226) && (_actor13._position.y == 152) && (_action1._field1E != 0) && (_actor13._visage == 3127) && (!R2_GLOBALS.getFlag(71))) {
			R2_GLOBALS._sound2.stop();
			R2_GLOBALS._sound2.play(331);
			R2_GLOBALS.setFlag(71);
			_sceneMode = 3626;
			setAction(&_sequenceManager1, this, 3626, &_actor13, NULL);
		}
		break;
	case 3624:
		R2_GLOBALS._player.disableControl();
		if ((_field254E != 0) && (_actor10._position.x == 229) && (_actor10._position.y == 154) && (_actor11._position.x == 181) && (_actor11._position.y == 154) && (_actor12._position.x == 207) && (_actor12._position.y == 154) && (_actor4._position.x == 155) && (_actor4._position.y == 154)) {
			R2_GLOBALS._sound2.stop();
			R2_GLOBALS._sound2.play(331);
			_sceneMode = 3625;
			setAction(&_sequenceManager1, this, 3625, &_actor10, &_actor11, &_actor12, &_actor4, NULL);
		}
		break;
	case 3625:
		R2_GLOBALS._sound2.stop();
		R2_GLOBALS._sceneManager.changeScene(3700);
		break;
	case 3626:
		_actor13.setPosition(Common::Point(0, 0));
		_action1.setActionIndex(2);
		if (R2_GLOBALS._events.getCursor() > R2_LAST_INVENT) {
			R2_GLOBALS._events.setCursor(CURSOR_USE);
			R2_GLOBALS._player.enableControl(CURSOR_USE);
		} else {
			R2_GLOBALS._player.enableControl();
		}
		R2_GLOBALS._sound2.stop();
		_sceneMode = 3623;
		break;
	default:
		break;
	}
}

void Scene3600::process(Event &event) {
	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._events.getCursor() == CURSOR_ARROW) && (event.mousePos.x > 237) && (!R2_GLOBALS.getFlag(71))) {
		SceneItem::display(3600, 17, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7 -999);
		event.handled = true;
	}
	Scene::process(event);
}

void Scene3600::dispatch() {
	if ((R2_GLOBALS._player.getRegionIndex() == 200) && (_action1._field1E != 0) && (_field254E == 0)){
		R2_GLOBALS._sound2.fadeOut2(NULL);
		if (_actor13._mover)
			_actor13.addMover(NULL);
		if (R2_GLOBALS._player._action)
			R2_GLOBALS._player.setAction(NULL);
		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		_field254C = 0;
		_field254E = 1;

		R2_GLOBALS._walkRegions.disableRegion(2);
		R2_GLOBALS._walkRegions.disableRegion(7);
		R2_GLOBALS._player.disableControl();

		_sceneMode = 3624;

		_actor10.setStrip(-1);
		_actor11.setStrip(-1);
		_actor12.setStrip(-1);
		_actor4.setStrip(-1);

		R2_GLOBALS._player.hide();

		if (R2_GLOBALS._player._characterIndex == 2) {
			_actor11.setPosition(R2_GLOBALS._player._position);
			_actor11.show();
		} else if (R2_GLOBALS._player._characterIndex == 3) {
			_actor12.setPosition(R2_GLOBALS._player._position);
			_actor12.show();
		} else {
			_actor10.setPosition(R2_GLOBALS._player._position);
			_actor10.show();
		}
		_actor10.setAction(&_sequenceManager2, this, 3610, &_actor10, NULL);
		_actor11.setAction(&_sequenceManager3, this, 3611, &_actor11, NULL);
		_actor12.setAction(&_sequenceManager4, this, 3612, &_actor12, NULL);
		_actor4.setAction(&_sequenceManager1, this, 3613, &_actor4, NULL);
	}

	if ((_actor13.getRegionIndex() == 200) && (_action1._field1E != 0) && (_field254E == 0)){
		R2_GLOBALS._sound2.fadeOut2(NULL);
		_sceneMode = 3620;
		_field2550 = 1;
		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);
		if (_actor10._mover)
			_actor10.addMover(NULL);
		if (_actor11._mover)
			_actor11.addMover(NULL);
		if (_actor12._mover)
			_actor12.addMover(NULL);
		if (_actor4._mover)
			_actor4.addMover(NULL);
	}
	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3700 - Cutscene - Teleport outside
 *
 *--------------------------------------------------------------------------*/
void Scene3700::postInit(SceneObjectList *OwnerList) {
	loadScene(3700);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);

	_actor1.postInit();
	_actor1._moveDiff = Common::Point(3, 2);

	_actor2.postInit();
	_actor2._numFrames = 7;
	_actor2._moveDiff = Common::Point(5, 3);
	_actor2.hide();

	_actor3.postInit();
	_actor3._moveDiff = Common::Point(3, 2);
	_actor3.hide();

	_actor4.postInit();
	_actor4._numFrames = 7;
	_actor4._moveDiff = Common::Point(5, 3);
	_actor4.hide();

	_actor5.postInit();

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._sound1.play(332);

	_sceneMode = 3700;
	setAction(&_sequenceManager, this, 3700, &_actor1, &_actor2, &_actor3, &_actor4, &_actor5, NULL);
}

void Scene3700::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene3700::signal() {
	switch (_sceneMode) {
	case 3328:
	// No break on purpose
	case 3329:
		warning("STUB: sub_1D227()");
		_sceneMode = 3701;
		setAction(&_sequenceManager, this, 3701, &_actor2, &_actor3, &_actor4, NULL);
		break;
	case 3700:
		_actor1.setup(10, 6, 1);
		_actor2.setup(20, 5, 1);
		if (R2_GLOBALS.getFlag(71)) {
			_sceneMode = 3329;
			_stripManager.start(3329, this);
		} else {
			_sceneMode = 3328;
			_stripManager.start(3328, this);
		}
		break;
	case 3701:
		R2_GLOBALS._sceneManager.changeScene(1000);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 3800 -
 *
 *--------------------------------------------------------------------------*/
Scene3800::Scene3800() {
	_field412 = 0;
}
void Scene3800::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
}

void Scene3800::Exit1::changeScene() {
	Scene3800 *scene = (Scene3800 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_field412 = 1;

	if (R2_GLOBALS.getFlag(46)) {
		if (scene->_field412 == R2_GLOBALS._v566A9) {
			R2_GLOBALS._v566AA = 3;
			if (R2_GLOBALS._v56A93 + 1 == 0) {
				R2_GLOBALS._v566A8--;
				R2_GLOBALS._v566A9 = 0;
			} else {
				R2_GLOBALS._v566A9 = R2_GLOBALS._v566AB[R2_GLOBALS._v56A93];
				R2_GLOBALS._v56A93--;
			}
		} else {
			++R2_GLOBALS._v56A93;
			if (R2_GLOBALS._v56A93 > 999)
				R2_GLOBALS._v56A93 = 999;
			R2_GLOBALS._v566AB[R2_GLOBALS._v56A93] = R2_GLOBALS._v566A9;
			R2_GLOBALS._v566A9 = 3;
		}
	}

	if (R2_GLOBALS._v566A8 == 0)
		scene->_sceneMode = 16;
	else
		scene->_sceneMode = 11;

	Common::Point pt(160, 115);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene3800::Exit2::changeScene() {
	Scene3800 *scene = (Scene3800 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_field412 = 2;

	if (R2_GLOBALS.getFlag(46)) {
		if (scene->_field412 == R2_GLOBALS._v566A9) {
			R2_GLOBALS._v566AA = 4;
			if (R2_GLOBALS._v56A93 + 1 == 0) {
				R2_GLOBALS._v566A8--;
				R2_GLOBALS._v566A9 = 0;
			} else {
				R2_GLOBALS._v566A9 = R2_GLOBALS._v566AB[R2_GLOBALS._v56A93];
				R2_GLOBALS._v56A93--;
			}
		} else {
			++R2_GLOBALS._v56A93;
			if (R2_GLOBALS._v56A93 > 999)
				R2_GLOBALS._v56A93 = 999;
			R2_GLOBALS._v566AB[R2_GLOBALS._v56A93] = R2_GLOBALS._v566A9;
			R2_GLOBALS._v566A9 = 4;
		}
	}

	if (R2_GLOBALS._v566A8 == 0)
		scene->_sceneMode = 16;
	else
		scene->_sceneMode = 12;

	Common::Point pt(330, 145);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene3800::Exit3::changeScene() {
	Scene3800 *scene = (Scene3800 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_field412 = 3;

	if (R2_GLOBALS.getFlag(46)) {
		if (scene->_field412 == R2_GLOBALS._v566A9) {
			R2_GLOBALS._v566AA = 1;
			if (R2_GLOBALS._v56A93 + 1 == 0) {
				R2_GLOBALS._v566A8--;
				R2_GLOBALS._v566A9 = 0;
			} else {
				R2_GLOBALS._v566A9 = R2_GLOBALS._v566AB[R2_GLOBALS._v56A93];
				R2_GLOBALS._v56A93--;
			}
		} else {
			++R2_GLOBALS._v56A93;
			if (R2_GLOBALS._v56A93 > 999)
				R2_GLOBALS._v56A93 = 999;
			R2_GLOBALS._v566AB[R2_GLOBALS._v56A93] = R2_GLOBALS._v566A9;
			R2_GLOBALS._v566A9 = 1;
		}
	}

	if (R2_GLOBALS._v566A8 == 0)
		scene->_sceneMode = 16;
	else
		scene->_sceneMode = 13;

	Common::Point pt(160, 220);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene3800::Exit4::changeScene() {
	Scene3800 *scene = (Scene3800 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_field412 = 4;

	if (R2_GLOBALS.getFlag(46)) {
		if (scene->_field412 == R2_GLOBALS._v566A9) {
			R2_GLOBALS._v566AA = 2;
			if (R2_GLOBALS._v56A93 + 1 == 0) {
				R2_GLOBALS._v566A8--;
				R2_GLOBALS._v566A9 = 0;
			} else {
				R2_GLOBALS._v566A9 = R2_GLOBALS._v566AB[R2_GLOBALS._v56A93];
				R2_GLOBALS._v56A93--;
			}
		} else {
			++R2_GLOBALS._v56A93;
			if (R2_GLOBALS._v56A93 > 999)
				R2_GLOBALS._v56A93 = 999;
			R2_GLOBALS._v566AB[R2_GLOBALS._v56A93] = R2_GLOBALS._v566A9;
			R2_GLOBALS._v566A9 = 2;
		}
	}

	if (R2_GLOBALS._v566A8 == 0)
		scene->_sceneMode = 16;
	else
		scene->_sceneMode = 14;

	Common::Point pt(-10, 145);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene3800::initScene3800() {
	_exit1._enabled = true;
	_exit2._enabled = true;
	_exit3._enabled = true;
	_exit4._enabled = true;
	_exit1._insideArea = false;
	_exit2._insideArea = false;
	_exit3._insideArea = false;
	_exit4._insideArea = false;
	_exit1._moving = false;
	_exit2._moving = false;
	_exit3._moving = false;
	_exit4._moving = false;

	loadScene(R2_GLOBALS._v566A6);

	R2_GLOBALS._uiElements.draw();
}

void Scene3800::sub110BBD() {
	R2_GLOBALS._player.disableControl();
	switch (_field412) {
	case 0:
		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.setVisage(10);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(160, 145));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.changeZoom(-1);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		_actor1.postInit();
		_actor1.fixPriority(10);
		_actor1.changeZoom(-1);
		_actor1.setVisage(1110);
		_actor1._effect = 5;
		_actor1._field9C = this->_field312;
		R2_GLOBALS._player._linkedActor = &_actor1;
		switch (R2_GLOBALS._sceneManager._previousScene) {
		case 2600:
			_object1.postInit();
			_object2.postInit();
			_actor1.hide();
			_sceneMode = 3800;
			setAction(&_sequenceManager1, this, 3800, &R2_GLOBALS._player, &_object1, &_object2, NULL);
			break;
		case 3900:
			_sceneMode = 15;
			switch (R2_GLOBALS._v566AA - 1) {
			case 0: {
				R2_GLOBALS._player.setPosition(Common::Point(160, 220));
				Common::Point pt(160, 160);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 1: {
				R2_GLOBALS._player.setPosition(Common::Point(-10, 145));
				Common::Point pt(19, 145);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 2: {
				R2_GLOBALS._player.setPosition(Common::Point(160, 115));
				Common::Point pt(160, 120);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 3: {
				R2_GLOBALS._player.setPosition(Common::Point(330, 145));
				Common::Point pt(300, 145);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			default:
				break;
			}
		default:
			R2_GLOBALS._player.enableControl(CURSOR_ARROW);
			break;
		}
		break;
	case 1: {
		_sceneMode = 15;
		R2_GLOBALS._player.setPosition(Common::Point(160, 220));
		Common::Point pt(160, 160);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 2: {
		_sceneMode = 15;
		R2_GLOBALS._player.setPosition(Common::Point(-10, 145));
		Common::Point pt(19, 145);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 3: {
		_sceneMode = 15;
		R2_GLOBALS._player.setPosition(Common::Point(160, 115));
		Common::Point pt(160, 120);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 4: {
		_sceneMode = 15;
		R2_GLOBALS._player.setPosition(Common::Point(330, 145));
		Common::Point pt(300, 145);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	default:
		break;
	}
}

void Scene3800::postInit(SceneObjectList *OwnerList) {
	_field412 = 0;

	initScene3800();

	SceneExt::postInit();
	R2_GLOBALS._sound1.play(231);

	scalePalette(65, 65, 65);

	setZoomPercents(87, 40, 144, 100);

	_exit1.setDetails(Rect(14, 87, 305, 125), SHADECURSOR_UP, 3800);
	_exit1.setDest(Common::Point(160, 126));
	_exit2.setDetails(Rect(305, 87, 320, 128), EXITCURSOR_E, 3800);
	_exit2.setDest(Common::Point(312, 145));
	_exit3.setDetails(Rect(14, 160, 305, 168), SHADECURSOR_DOWN, 3800);
	_exit3.setDest(Common::Point(160, 165));
	_exit4.setDetails(Rect(0, 87, 14, 168), EXITCURSOR_W, 3800);
	_exit4.setDest(Common::Point(7, 145));

	_rect1.set(0, 0, 320, 87);
	_item1.setDetails(Rect(0, 0, 320, 200), 3800, 0, 1, 2, 1, (SceneItem *) NULL);

	sub110BBD();
}

void Scene3800::signal() {
	switch (_sceneMode) {
	case 11:
		R2_GLOBALS._v566A6 += 15;
		if (R2_GLOBALS._v566A6 > 3815)
			R2_GLOBALS._v566A6 -= 20;
		initScene3800();
		sub110BBD();
		break;
	case 12:
		R2_GLOBALS._v566A6 += 5;
		if (R2_GLOBALS._v566A6 > 3815)
			R2_GLOBALS._v566A6 = 3800;
		initScene3800();
		sub110BBD();
		break;
	case 13:
		R2_GLOBALS._v566A6 -= 15;
		if (R2_GLOBALS._v566A6 < 3800)
			R2_GLOBALS._v566A6 += 20;
		initScene3800();
		sub110BBD();
		break;
	case 14:
		R2_GLOBALS._v566A6 -= 5;
		if (R2_GLOBALS._v566A6 < 3800)
			R2_GLOBALS._v566A6 = 3815;
		initScene3800();
		sub110BBD();
		break;
	case 15:
		R2_GLOBALS._v56AAB = 0;
		R2_GLOBALS._player.disableControl(CURSOR_ARROW);
		break;
	case 16:
		g_globals->_sceneManager.changeScene(3900);
		break;
	case 3800:
		_actor1.show();
		_object1.remove();
		_object2.remove();
		R2_GLOBALS._v56AAB = 0;
		R2_GLOBALS._player.disableControl(CURSOR_ARROW);
		break;
	case 3805:
		_exit1._enabled = false;
		_exit2._enabled = false;
		_exit3._enabled = false;
		_exit4._enabled = false;
		R2_GLOBALS._player._canWalk = false;
		R2_GLOBALS._events.setCursor(CURSOR_USE);
		break;
	case 3806:
		_exit1._enabled = true;
		_exit2._enabled = true;
		_exit3._enabled = true;
		_exit4._enabled = true;
		R2_GLOBALS._player.disableControl(CURSOR_ARROW);
		break;
	default:
		break;
	}
}

void Scene3800::process(Event &event) {
	if ((R2_GLOBALS._player._uiEnabled) && (event.eventType == 1) && (_rect1.contains(event.mousePos))) {
		event.handled = true;
		switch (R2_GLOBALS._events.getCursor()) {
		case R2_NEGATOR_GUN:
			R2_GLOBALS._player.addMover(NULL);
			R2_GLOBALS._player.updateAngle(event.mousePos);
			break;
		case R2_STEPPING_DISKS:
			SceneItem::display(3800, 5, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
			break;
		case R2_ATTRACTOR_UNIT:
			SceneItem::display(3800, 3, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
			break;
		default:
			event.handled = false;
			break;
		}
	}

	Scene::process(event);
}

/*--------------------------------------------------------------------------
 * Scene 3900 -
 *
 *--------------------------------------------------------------------------*/
void Scene3900::Exit1::changeScene() {
	Scene3900 *scene = (Scene3900 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	R2_GLOBALS._v566A9 = 3;
	R2_GLOBALS._v566AA = 1;
	R2_GLOBALS._v566A8 = 1;
	scene->_sceneMode = 14;

	Common::Point pt(160, 115);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene3900::Exit2::changeScene() {
	Scene3900 *scene = (Scene3900 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	R2_GLOBALS._v566A9 = 4;
	R2_GLOBALS._v566AA = 2;
	R2_GLOBALS._v566A8 = 1;
	scene->_sceneMode = 14;

	Common::Point pt(330, 145);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene3900::Exit3::changeScene() {
	Scene3900 *scene = (Scene3900 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	R2_GLOBALS._v566A9 = 1;
	R2_GLOBALS._v566AA = 3;
	R2_GLOBALS._v566A8 = 1;
	scene->_sceneMode = 14;

	Common::Point pt(160, 220);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene3900::Exit4::changeScene() {
	Scene3900 *scene = (Scene3900 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	R2_GLOBALS._v566A9 = 2;
	R2_GLOBALS._v566AA = 4;
	R2_GLOBALS._v566A8 = 1;
	scene->_sceneMode = 14;

	Common::Point pt(-10, 145);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene3900::Exit5::changeScene() {
	Scene3900 *scene = (Scene3900 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 13;

	if (R2_GLOBALS._v566A9 == 4) {
		Common::Point pt(-10, 135);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, scene);
	} else {
		Common::Point pt(330, 135);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, scene);
	}

	R2_GLOBALS._v566A9 = 0;
}

void Scene3900::postInit(SceneObjectList *OwnerList) {
	if ((R2_GLOBALS._v566AA == 2) && (R2_GLOBALS._sceneManager._previousScene != 2700))
		loadScene(3825);
	else
		loadScene(3820);
	SceneExt::postInit();
	R2_GLOBALS._sound1.changeSound(231);
	setZoomPercents(87, 40, 144, 100);
	R2_GLOBALS._player.disableControl();
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.setStrip(3);
	R2_GLOBALS._player.changeZoom(-1);
	R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	_actor1.postInit();
	_actor1.fixPriority(10);
	_actor1.changeZoom(-1);
	_actor1.setVisage(1110);
	_actor1._effect = 5;
	_actor1._field9C = _field312;
	R2_GLOBALS._player._linkedActor = &_actor1;
	if ((R2_GLOBALS._v566AA == 2) && (R2_GLOBALS._sceneManager._previousScene != 2700)) {
//		loadScene(3825);
		R2_GLOBALS._v566AA = 4;
		_exit1.setDetails(Rect(29, 87, 305, 125), SHADECURSOR_UP, 3900);
		_exit3.setDetails(Rect(29, 160, 305, 168), SHADECURSOR_DOWN, 3900);

		_exit2.setDetails(Rect(305, 87, 320, 168), EXITCURSOR_E, 3900);
		_exit2.setDest(Common::Point(312, 145));
		_exit2._enabled = true;
		_exit2._insideArea = false;
		_exit2._moving = false;

		_exit4._enabled = false;

		_exit5.setDetails(Rect(0, 87, 29, 168), EXITCURSOR_W, 3900);
		_exit5.setDest(Common::Point(24, 135));
	} else {
//		loadScene(3820);
		R2_GLOBALS._v566AA = 2;
		_exit1.setDetails(Rect(14, 87, 290, 125), SHADECURSOR_UP, 3900);
		_exit3.setDetails(Rect(14, 160, 290, 168), SHADECURSOR_DOWN, 3900);


		_exit2._enabled = false;

		_exit4.setDetails(Rect(0, 87, 14, 168), EXITCURSOR_W, 3900);
		_exit4.setDest(Common::Point(7, 145));
		_exit4._enabled = true;
		_exit4._insideArea = false;
		_exit4._moving = false;

		_exit5.setDetails(Rect(290, 87, 320, 168), EXITCURSOR_E, 3900);
		_exit5.setDest(Common::Point(295, 135));
	}
	_exit5._enabled = true;
	_exit5._insideArea = false;
	_exit5._moving = false;

	scalePalette(65, 65, 65);

	_exit1.setDest(Common::Point(160, 126));
	_exit1._enabled = true;
	_exit1._insideArea = false;
	_exit1._moving = false;

	_exit3.setDest(Common::Point(160, 165));
	_exit3._enabled = true;
	_exit3._insideArea = false;
	_exit3._moving = false;

	R2_GLOBALS._uiElements.draw();

	_rect1.set(0, 0, 320, 87);
	_item1.setDetails(Rect(0, 0, 320, 200), 3800, 0, 1, 2, 1, (SceneItem *)NULL);
	if (R2_GLOBALS._sceneManager._previousScene == 3800) {
		_sceneMode = 11;
		switch (R2_GLOBALS._v566AA - 1) {
		case 0: {
			R2_GLOBALS._player.setPosition(Common::Point(160, 115));
			Common::Point pt(160, 120);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
			}
			break;
		case 1: {
			R2_GLOBALS._player.setPosition(Common::Point(330, 145));
			Common::Point pt(300, 145);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
			}
			break;
		case 2: {
			R2_GLOBALS._player.setPosition(Common::Point(160, 220));
			Common::Point pt(160, 160);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
			}
			break;
		case 3: {
			R2_GLOBALS._player.setPosition(Common::Point(-10, 145));
			Common::Point pt(19, 145);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
			}
			break;
		default:
			break;
		}
	} else if (R2_GLOBALS._sceneManager._previousScene == 2700) {
		_sceneMode = 12;
		R2_GLOBALS._player.setPosition(Common::Point(330, 135));
		Common::Point pt(265, 135);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(160, 145));
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
	}
}

void Scene3900::signal() {
	switch (_sceneMode) {
	case 11:
	// No break on purpose
	case 12:
		R2_GLOBALS._v56AAB = 0;
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	case 13:
		R2_GLOBALS._sceneManager.changeScene(2700);
		break;
	case 14:
		R2_GLOBALS._sceneManager.changeScene(3800);
		break;
	case 3805:
		_exit1._enabled = false;
		_exit2._enabled = false;
		_exit3._enabled = false;
		_exit4._enabled = false;
		R2_GLOBALS._player._canWalk = false;
		R2_GLOBALS._events.setCursor(R2_STEPPING_DISKS);
		break;
	case 3806:
		_exit1._enabled = true;
		_exit2._enabled = true;
		_exit3._enabled = true;
		_exit4._enabled = true;
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	default:
		break;
	}
}

void Scene3900::process(Event &event) {
	if ((R2_GLOBALS._player._uiEnabled) && (event.eventType == 1) && (_rect1.contains(event.mousePos))) {
		event.handled = true;
		switch (R2_GLOBALS._events.getCursor()) {
		case R2_NEGATOR_GUN:
			R2_GLOBALS._player.addMover(NULL);
			R2_GLOBALS._player.updateAngle(event.mousePos);
			break;
		case R2_STEPPING_DISKS:
			SceneItem::display(3800, 5, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
			break;
		case R2_ATTRACTOR_UNIT:
			SceneItem::display(3800, 3, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
			break;
		default:
			event.handled = false;
			break;
		}
	}
	Scene::process(event);
}

} // End of namespace Ringworld2
} // End of namespace TsAGE
