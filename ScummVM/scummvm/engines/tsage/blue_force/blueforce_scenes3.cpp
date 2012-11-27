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

#include "common/config-manager.h"
#include "tsage/blue_force/blueforce_scenes3.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 300 - Outside Police Station
 *
 *--------------------------------------------------------------------------*/

bool Scene300::Object::startAction(CursorType action, Event &event) {
	if (action == CURSOR_TALK) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		scene->_stripManager.start(_stripNumber, scene);
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

bool Scene300::Object19::startAction(CursorType action, Event &event) {
	if ((action != CURSOR_USE) || !BF_GLOBALS.getFlag(onDuty)) {
		return NamedObject::startAction(action, event);
	} else if ((BF_GLOBALS._dayNumber != 2) || (BF_GLOBALS._bookmark >= bEndDayOne)) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		setAction(&scene->_action4);
	} else {
		SceneItem::display2(300, 33);
	}

	return true;
}
// entrance door
bool Scene300::Item1::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 305;
		scene->setAction(&scene->_sequenceManager1, scene, 305, &BF_GLOBALS._player,
			&scene->_object8, NULL);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene300::Item2::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_LOOK) || (action == CURSOR_USE)) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 0;
		scene->setAction(&scene->_sequenceManager1, scene, 304, &scene->_object11, NULL);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene300::Item14::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 151, 54);
	return true;
}

bool Scene300::Item15::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 316, 90);
	return true;
}

/*--------------------------------------------------------------------------*/

void Scene300::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(1);
		break;
	case 1:
		if (BF_GLOBALS.getFlag(fWithLyle))
			SceneItem::display2(666, 27);
		else
			SceneItem::display2(300, 0);
		setDelay(1);
		break;
	case 2: {
		ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x - 8,
			BF_GLOBALS._player._position.y);
		break;
	}
	case 3:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene300::Action2::signal() {
	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(1);
		break;
	case 1:
		SceneItem::display2(300, 28);
		setDelay(1);
		break;
	case 2: {
		ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x + 8,
			BF_GLOBALS._player._position.y);
		break;
	}
	case 3:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene300::Action3::signal() {
	Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(1);
		break;
	case 1:
		BF_GLOBALS._player.setAction(&scene->_sequenceManager1, this, 306, &BF_GLOBALS._player,
			&scene->_object8, NULL);
		break;
	case 2:
		SceneItem::display2(300, 35);
		setDelay(1);
		break;
	case 3:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene300::Action4::signal() {
	Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(1);
		break;
	case 1:
		setAction(&scene->_sequenceManager1, this, 316, &BF_GLOBALS._player, &scene->_object19, NULL);
		break;
	case 2:
		BF_GLOBALS._sceneManager.changeScene(60);
		setDelay(15);
		break;
	case 3:
		setAction(&scene->_sequenceManager1, this, 319, &scene->_object19, NULL);
		break;
	case 4:
		BF_GLOBALS.setFlag(onBike);
		BF_GLOBALS._sceneManager.changeScene(190);
		break;
	default:
		break;
	}
}

void Scene300::Action5::signal() {
	Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		scene->_field2760 = 1;
		setDelay(1);
		break;
	case 1:
		setAction(&scene->_sequenceManager1, this, 1306, &scene->_object1, &scene->_object8, NULL);
		break;
	case 2:
		scene->_stripManager.start(3004, this);
		break;
	case 3: {
		ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 186, 140);
		setDelay(3);
		break;
	}
	case 4:
		remove();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene300::Scene300(): SceneExt(), _object13(3000), _object14(3001), _object15(3002),
			_object16(3003) {
	_field2760 = _field2762 = 0;
}

void Scene300::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(300);

	// Add the speakers
	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_sutterSpeaker);
	_stripManager.addSpeaker(&_dougSpeaker);
	_stripManager.addSpeaker(&_jakeSpeaker);

	_field2762 = 0;
	_item14.setDetails(Rect(144, 27, 160, 60), 300, -1, -1, -1, 1, NULL);
	_item15.setDetails(Rect(310, 76, SCREEN_WIDTH, 105), 300, -1, -1, -1, 1, NULL);

	// Setup the player
	int playerVisage = BF_GLOBALS._player._visage;
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(playerVisage);
	BF_GLOBALS._player.setStrip(3);
	BF_GLOBALS._player.setPosition(Common::Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player._moveDiff = Common::Point(3, 1);
	BF_GLOBALS._player.disableControl();

	_object8.postInit();
	_object8.setVisage(301);
	_object8.setStrip(2);
	_object8.setPosition(Common::Point(300, 77));

	if ((BF_GLOBALS._dayNumber != 2) || (BF_GLOBALS._bookmark < bEndDayOne)) {
		_object17.postInit();
		_object17.setVisage(301);
		_object17.setStrip(1);
		_object17.setPosition(Common::Point(87, 88));
		_object17.setDetails(300, 11, 13, 2, 1, (SceneItem *)NULL);

		_object18.postInit();
		_object18.setVisage(301);
		_object18.setStrip(1);
		_object18.setPosition(Common::Point(137, 92));
		_object18.setDetails(300, 11, 13, 3, 1, (SceneItem *)NULL);
	}

	_object19.postInit();
	_object19.setVisage(301);
	_object19.setStrip(1);
	_object19.setPosition(Common::Point(175, 99));
	_object19.setDetails(300, 11, 13, 34, 1, (SceneItem *)NULL);

	_object11.postInit();
	_object11.setVisage(301);
	_object11.setStrip(8);
	_object11.setPosition(Common::Point(265, 91));
	_object11.hide();

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 50:
	case 60:
		BF_GLOBALS.clearFlag(onBike);
		if (BF_GLOBALS.getFlag(onDuty)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 318;
			setAction(&_sequenceManager1, this, 318, &BF_GLOBALS._player, &_object19, NULL);
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 300;
			setAction(&_sequenceManager1, this, 1300, &BF_GLOBALS._player, NULL);
		}
		break;
	case 190:
		_sceneMode = 0;
		if (!BF_GLOBALS.getFlag(onBike)) {
			_sceneMode = 7308;
			BF_GLOBALS._player.setPosition(Common::Point(175, 50));
			ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, 123, 71);

			if ((BF_GLOBALS._dayNumber == 2) && (BF_GLOBALS._bookmark < bEndDayOne))
				setupInspection();
		} else if (!BF_GLOBALS.getFlag(onDuty)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 300;
			setAction(&_sequenceManager1, this, 300, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 318;
			setAction(&_sequenceManager1, this, 318, &BF_GLOBALS._player, &_object19, NULL);
		}
		break;
	case 315:
		BF_GLOBALS._player.setPosition(Common::Point(305, 66));
		if ((BF_GLOBALS._dayNumber != 2) || (BF_GLOBALS._bookmark >= bEndDayOne)) {
			BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 1304 : 303);
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager1, this, 306, &BF_GLOBALS._player, &_object8, NULL);
		} else {
			BF_GLOBALS._player.setVisage(1304);
			setupInspection();
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager1, this, 306, &BF_GLOBALS._player, &_object8, NULL);
		}
		break;
	default:
		_sceneMode = 0;
		BF_GLOBALS._player.setVisage(1304);
		BF_GLOBALS._player.disableControl();
		setAction(&_sequenceManager1, this, 306, &BF_GLOBALS._player, &_object8, NULL);
		break;
	}

	if (BF_GLOBALS.getFlag(onBike) && !BF_GLOBALS.getFlag(onDuty)) {
		BF_GLOBALS._sound1.play(30);
	} else if ((BF_GLOBALS._dayNumber == 2) && (BF_GLOBALS._bookmark < bEndDayOne)) {
		BF_GLOBALS._sound1.changeSound(49);
	} else if (BF_GLOBALS._sceneManager._previousScene != 190) {
		BF_GLOBALS._sound1.changeSound(33);
	}

	_item10.setDetails(4, 300, 7, 13, 16, 1);
	_item11.setDetails(2, 300, 9, 13, 18, 1);
	_item12.setDetails(5, 300, 10, 13, 19, 1);
	_item13.setDetails(3, 300, 25, 26, 27, 1);
	_item2.setDetails(Rect(266, 54, 272, 59), 300, -1, -1, -1, 1, NULL);
	_item1.setDetails(Rect(262, 47, 299, 76), 300, 1, 13, -1, 1, NULL);
	_item4.setDetails(Rect(0, 85, SCREEN_WIDTH - 1, UI_INTERFACE_Y - 1), 300, 6, 13, 15, 1, NULL);
	_item7.setDetails(Rect(219, 46, 251, 74), 300, 22, 23, 24, 1, NULL);
	_item8.setDetails(Rect(301, 53, 319, 78), 300, 22, 23, 24, 1, NULL);
	_item5.setDetails(Rect(179, 44, 200, 55), 300, 8, 13, 17, 1, NULL);
	_item6.setDetails(Rect(210, 46, 231, 55), 300, 8, 13, 17, 1, NULL);
	_item3.setDetails(Rect(160, 0, SCREEN_WIDTH - 1, 75), 300, 4, 13, 14, 1, NULL);
	_item9.setDetails(Rect(0, 0, SCREEN_WIDTH, UI_INTERFACE_Y), 300, 29, 30, 31, 1, NULL);
}

void Scene300::signal() {
	switch (_sceneMode) {
	case 300:
		BF_GLOBALS._sound1.fadeSound(33);
		BF_GLOBALS.clearFlag(onBike);
		_sceneMode = 0;

		if ((BF_GLOBALS._dayNumber != 1) || (BF_GLOBALS._bookmark != bNone)) {
			signal();
		} else {
			_stripManager.start(3005, this);
		}
		break;
	case 301:
		if (_field2760) {
			_sceneMode = 1302;
			signal();
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 1302;
			setAction(&_sequenceManager1, this, 306, &_object1, &_object8, NULL);
		}

		_object12.show();
		_object5.dispatch();
		BF_GLOBALS._player.hide();
		break;
	case 303:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 2307;
		setAction(&_sequenceManager1, this, 303, &_object13, &_object1, NULL);
		break;
	case 305:
		if ((BF_GLOBALS._dayNumber == 4) || (BF_GLOBALS._dayNumber == 5)) {
			_sceneMode = 0;
			setAction(&_action3);
		} else {
			BF_GLOBALS._sound1.fadeOut2(NULL);
			BF_GLOBALS._sceneManager.changeScene(315);
		}
		break;
	case 309:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 3307;
		setAction(&_sequenceManager1, this, 309, &_object14, &_object1, NULL);
		break;
	case 310:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 4307;
		setAction(&_sequenceManager1, this, 310, &_object12, &_object1, NULL);
		break;
	case 311:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 5307;
		setAction(&_sequenceManager1, this, 311, &_object15, &_object1, NULL);
		break;
	case 312:
	case 5307:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 1305;
		setAction(&_sequenceManager1, this, 312, &_object1, &_object16, NULL);
		break;
	case 317:
		BF_GLOBALS.setFlag(onBike);
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 318:
		BF_GLOBALS.clearFlag(onBike);
		_sceneMode = 0;
		signal();
		break;
	case 1302:
		_field2762 = 0;
		BF_GLOBALS._player.disableControl();
		_sceneMode = 1308;
		setAction(&_sequenceManager1, this, 302, &_object1, NULL);
		break;
	case 1305:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 1313;
		setAction(&_sequenceManager1, this, 305, &_object1, &_object8, NULL);
		BF_GLOBALS._player.show();
		_object12.hide();
		break;
	case 1307:
	case 2308:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 303;
		setAction(&_sequenceManager1, this, 308, &_object14, NULL);
		break;
	case 1308:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 1307;
		setAction(&_sequenceManager1, this, 308, &_object13, NULL);
		break;
	case 1313:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		_object15.setAction(&_sequenceManager4, NULL, 315, &_object15, &_object16, NULL);
		_object13.setAction(&_sequenceManager2, NULL, 313, &_object13, &_object17, NULL);
		_object14.setAction(&_sequenceManager3, this, 314, &_object14, &_object18, NULL);

		BF_GLOBALS._bookmark = bEndDayOne;
		BF_GLOBALS._sound1.changeSound(33);
		break;
	case 2307:
	case 3308:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 309;
		setAction(&_sequenceManager1, this, 308, &_object12, NULL);
		break;
	case 3307:
		_object9.postInit();
		_object9.hide();
		_object10.postInit();
		_object10.hide();

		if (BF_GLOBALS.getFlag(gunClean)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 4308;
			setAction(&_sequenceManager1, this, 6307, &_object12, &_object1, &_object9, &_object10, NULL);
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 4308;
			setAction(&_sequenceManager1, this, 7307, &_object12, &_object1, &_object9, &_object10, NULL);
		}
		break;
	case 4307:
	case 5308:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 311;
		setAction(&_sequenceManager1, this, 308, &_object16, NULL);
		break;
	case 4308:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 310;
		setAction(&_sequenceManager1, this, 308, &_object15, NULL);
		break;
	case 6308:
		BF_GLOBALS._sceneManager.changeScene(190);
		break;
	case 7308:
		if (_field2762) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 301;
			setAction(&_sequenceManager1, this, 301, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS._player.enableControl();
		}
		break;
	case 0:
	default:
		if (_field2762) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 301;
			setAction(&_sequenceManager1, this, 301, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS._player.enableControl();
		}
		break;
	}
}

void Scene300::process(Event &event) {
	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1))) {
		// Check if the cursor is on an exit
		if (_item14.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_NE);
			BF_GLOBALS._events.setCursor(surface);
		} else if (_item15.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_E);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			// In case an exit cursor was being shown, restore the previously selected cursor
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene300::dispatch() {
	SceneExt::dispatch();

	if (!_action) {
		int regionIndex = BF_GLOBALS._player.getRegionIndex();
		if ((regionIndex == 1) && (_field2762 == 1)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 301;
			setAction(&_sequenceManager1, this, 301, &BF_GLOBALS._player, NULL);
		}

		if ((BF_GLOBALS._player._position.y < 59) && (BF_GLOBALS._player._position.x > 137) &&
				(_sceneMode != 6308) && (_sceneMode != 7308)) {
			// The original was setting a useless global variable (removed)
			_sceneMode = 6308;
			BF_GLOBALS._player.disableControl();
			ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x + 20,
				BF_GLOBALS._player._position.y - 5);
		}

		if (BF_GLOBALS._player._position.x <= 5)
			setAction(&_action2);

		if (BF_GLOBALS._player._position.x >= 315) {
			if (BF_GLOBALS.getFlag(onDuty) || (BF_GLOBALS._bookmark == bNone) || BF_GLOBALS.getFlag(fWithLyle)) {
				setAction(&_action1);
			} else {
				BF_GLOBALS._player.disableControl();
				_sceneMode = 317;
				setAction(&_sequenceManager1, this, 1301, &BF_GLOBALS._player, NULL);
			}
		}
	}
}

void Scene300::setupInspection() {
	_object13.postInit();
	_object13.setVisage(307);
	_object13.setStrip(6);
	_object13.setPosition(Common::Point(156, 134));
	_object13._moveDiff = Common::Point(3, 1);
	_object3.setup(&_object13, 306, 1, 29);

	_object14.postInit();
	_object14.setVisage(307);
	_object14.setStrip(6);
	_object14.setPosition(Common::Point(171, 137));
	_object14._moveDiff = Common::Point(3, 1);
	_object4.setup(&_object14, 306, 2, 29);

	_object12.postInit();
	_object12.setVisage(307);
	_object12.setStrip(6);
	_object12.setPosition(Common::Point(186, 140));
	_object12._moveDiff = Common::Point(3, 1);
	_object5.setup(&_object12, 306, 2, 29);
	_object12.hide();

	_object15.postInit();
	_object15.setVisage(307);
	_object15.setStrip(6);
	_object15.setPosition(Common::Point(201, 142));
	_object15._moveDiff = Common::Point(3, 1);
	_object6.setup(&_object15, 306, 3, 29);

	_object16.postInit();
	_object16.setVisage(307);
	_object16.setStrip(6);
	_object16.setPosition(Common::Point(216, 145));
	_object16._moveDiff = Common::Point(3, 1);
	_object7.setup(&_object16, 306, 1, 29);

	_object1.postInit();
	_object1.setVisage(307);
	_object1.setStrip(6);
	_object1.setPosition(Common::Point(305, 66));
	_object1._moveDiff = Common::Point(3, 1);
	_object1.setObjectWrapper(new SceneObjectWrapper());
	_object1.animate(ANIM_MODE_1, NULL);
	_object2.setup(&_object1, 306, 4, 29);

	BF_GLOBALS._sceneItems.addItems(&_object13, &_object14, &_object15, &_object16, NULL);
	_timer.set(3600, this, &_action5);

	_field2760 = 0;
	_field2762 = 1;
}

/*--------------------------------------------------------------------------
 * Scene 315 - Inside Police Station
 *
 *--------------------------------------------------------------------------*/

bool Scene315::Barry::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;
	scene->_currentCursor = action;

	switch (action) {
	case CURSOR_USE:
		if (scene->_invGreenCount || scene->_invGangCount)
			SceneItem::display2(320, 51);
		else
			NamedHotspot::startAction(action, event);
		break;
	case CURSOR_TALK:
		if ((BF_GLOBALS._dayNumber == 2) && (BF_GLOBALS._sceneManager._previousScene == 325))
			NamedHotspot::startAction(action, event);
		else {
			if (!BF_GLOBALS.getFlag(onDuty))
				scene->_stripNumber = 3172;
			else if (BF_GLOBALS.getFlag(fTalkedToBarry))
				scene->_stripNumber = 3166;
			else if (BF_GLOBALS.getFlag(fTalkedToLarry))
				scene->_stripNumber = 3164;
			else
				scene->_stripNumber = 3165;

			scene->setAction(&scene->_action1);
			BF_GLOBALS.setFlag(fTalkedToBarry);
		}
		break;
	case INV_GREENS_GUN:
	case INV_GREENS_KNIFE:
		BF_GLOBALS._player.disableControl();
		if (BF_INVENTORY._bookingGreen._sceneNumber == 390) {
			scene->_stripNumber = 3174;
			scene->setAction(&scene->_action1);
		} else {
			++scene->_bookGreenCount;
			scene->_stripNumber = (action == INV_GREENS_GUN) ? 3168 : 0;
			scene->_sceneMode = 3153;
			scene->setAction(&scene->_sequenceManager, scene, 3153, &BF_GLOBALS._player, NULL);
		}
		break;
	case INV_FOREST_RAP:
		BF_GLOBALS._player.disableControl();
		scene->_stripNumber = BF_GLOBALS.getFlag(onDuty) ? 3173 : 3178;
		scene->setAction(&scene->_action1);
		break;
	case INV_GREEN_ID:
	case INV_FRANKIE_ID:
	case INV_TYRONE_ID:
		BF_GLOBALS._player.disableControl();
		scene->_stripNumber = 3175;
		scene->setAction(&scene->_action1);
		break;
	case INV_BOOKING_GREEN:
	case INV_BOOKING_FRANKIE:
	case INV_BOOKING_GANG:
		BF_GLOBALS._player.disableControl();
		scene->_stripNumber = 3167;
		scene->setAction(&scene->_action1);
		break;
	case INV_COBB_RAP:
		if (BF_INVENTORY.getObjectScene(INV_MUG_SHOT) == 1)
			NamedHotspot::startAction(action, event);
		else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 3169;
			if (BF_GLOBALS._dayNumber > 2)
				scene->_stripNumber = 3176;
			else if (BF_GLOBALS.getFlag(onDuty))
				scene->_stripNumber = 3177;
			else
				scene->_stripNumber = 3170;
			scene->setAction(&scene->_action1);
		}
		break;
	case INV_22_BULLET:
	case INV_AUTO_RIFLE:
	case INV_WIG:
	case INV_22_SNUB:
		BF_GLOBALS._player.disableControl();
		if ((BF_GLOBALS.getFlag(fCuffedFrankie) && (BF_INVENTORY._bookingFrankie._sceneNumber == 0)) ||
				(!BF_GLOBALS.getFlag(fCuffedFrankie) && (BF_INVENTORY._bookingGang._sceneNumber == 0))) {
			scene->_stripNumber = 3174;
			scene->setAction(&scene->_action1);
		} else {
			++scene->_bookGangCount;
			if (!scene->_field1B6C && (scene->_bookGangCount == 1)) {
				scene->_field1B6C = 1;
				scene->_stripNumber = 3169;
			} else {
				scene->_stripNumber = 0;
			}

			scene->_sceneMode = 3153;
			scene->setAction(&scene->_sequenceManager, scene, 3153, &BF_GLOBALS._player, NULL);
		}
		break;
	default:
		return NamedHotspot::startAction(action, event);
	}

	return true;
}

bool Scene315::SutterSlot::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;
	scene->_currentCursor = action;

	switch (action) {
	case INV_GREENS_GUN:
	case INV_22_BULLET:
	case INV_AUTO_RIFLE:
	case INV_WIG:
	case INV_22_SNUB:
		SceneItem::display2(315, 30);
		break;
	case INV_GREEN_ID:
	case INV_FRANKIE_ID:
	case INV_TYRONE_ID:
		BF_GLOBALS._player.disableControl();
		scene->_stripNumber = 3175;
		scene->setAction(&scene->_action1);
		break;
	case INV_BOOKING_GREEN:
	case INV_BOOKING_FRANKIE:
	case INV_BOOKING_GANG:
		if (action == INV_BOOKING_GREEN)
			++scene->_bookGreenCount;
		else
			++scene->_bookGangCount;

		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 12;
		scene->setAction(&scene->_sequenceManager, scene, 3154, &BF_GLOBALS._player, NULL);
		break;
	default:
		return NamedHotspot::startAction(action, event);
	}

	return true;
}

bool Scene315::Sign::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_LOOK) {
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._player.addMover(NULL);
		scene->_object9.postInit();
		scene->_object9.hide();
		scene->_sceneMode = 3167;
		scene->setAction(&scene->_sequenceManager, scene, 3167, &scene->_object9, this, NULL);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene315::BulletinBoard::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_LOOK) {
		BF_GLOBALS._player.addMover(NULL);
		scene->_stripManager.start(3154, &BF_GLOBALS._stripProxy);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene315::CleaningKit::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	if ((action == INV_COLT45) && BF_GLOBALS.getFlag(onDuty)) {
		if (!BF_GLOBALS.getFlag(onDuty))
			SceneItem::display2(315, 27);
		else if (BF_GLOBALS.getHasBullets()) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 3162;
			scene->setAction(&scene->_sequenceManager, scene, 3162, &BF_GLOBALS._player, NULL);
		} else if (BF_GLOBALS.getFlag(fGunLoaded))
			SceneItem::display2(315, 46);
		else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 3159;
			scene->setAction(&scene->_sequenceManager, scene, 3159, &BF_GLOBALS._player, NULL);
		}
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene315::BriefingMaterial::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return NamedHotspot::startAction(action, event);
	else if (BF_INVENTORY._forestRap._sceneNumber == 1) {
		SceneItem::display2(315, 37);
		return true;
	} else {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 3158;
		scene->setAction(&scene->_sequenceManager, scene, 3158, &BF_GLOBALS._player, NULL);
		return true;
	}
}

bool Scene315::WestExit::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 190, 75);
	return true;
}

bool Scene315::SouthWestExit::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, event.mousePos.x, event.mousePos.y);
	return true;
}

/*--------------------------------------------------------------------------*/

bool Scene315::BulletinMemo::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		scene->_stripManager.start(3157, &BF_GLOBALS._stripProxy);
		return true;
	case CURSOR_USE:
		if (!BF_GLOBALS.getFlag(fGotPointsForCleaningGun)) {
			T2_GLOBALS._uiElements.addScore(10);
			BF_GLOBALS.setFlag(fGotPointsForCleaningGun);
		}
		BF_GLOBALS._player.addMover(NULL);
		scene->_stripManager.start(3159, &BF_GLOBALS._stripProxy);
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

// Own Mail Slot
bool Scene315::Object2::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		BF_GLOBALS._player.disableControl();
		scene->_object9.postInit();
		scene->_object9.hide();
		scene->_sceneMode = 3157;
		scene->setAction(&scene->_sequenceManager, scene, 3157, &BF_GLOBALS._player, &scene->_object9, NULL);
		return true;
	case CURSOR_USE:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 3156;
		scene->setAction(&scene->_sequenceManager, scene, BF_GLOBALS.getFlag(onDuty) ? 3156 : 3168,
			&BF_GLOBALS._player, this, NULL);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene315::ATFMemo::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		BF_GLOBALS._player.addMover(NULL);
		scene->_stripManager.start(3156, &BF_GLOBALS._stripProxy);
		return true;
	case CURSOR_USE:
		if (!BF_GLOBALS.getFlag(fGotPointsForMemo)) {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fGotPointsForMemo);
		}

		BF_GLOBALS._player.addMover(NULL);
		scene->_stripManager.start(3158, &BF_GLOBALS._stripProxy);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene315::Action1::signal() {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, 128, 128);
		break;
	case 1:
		BF_GLOBALS._player.changeAngle(315);
		setDelay(2);
		break;
	case 2:
		scene->_stripManager.start(scene->_stripNumber, this);
		break;
	case 3:
		if (scene->_sceneMode == 3169) {
			T2_GLOBALS._uiElements.addScore(30);
			BF_INVENTORY.setObjectScene(INV_MUG_SHOT, 1);
			//HACK: This has to be checked wether or not it occurs in the original.
			//When the _sceneMode is set to 3169, the value desn't change.
			//If you show the forest rapsheet, it gives points (and again... and again...)
			scene->_sceneMode = 3154;
		}

		remove();
		BF_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene315::Scene315() {
	BF_GLOBALS._v51C44 = 1;
	_field1B6C = _field139C = 0;
	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 1;

	BF_GLOBALS.clearFlag(fCanDrawGun);
	_field1B68 = true;
	_doorOpened = false;
	_invGreenCount = _bookGreenCount = 0;
	_invGangCount = _bookGangCount = 0;
}

void Scene315::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field1390);
	s.syncAsSint16LE(_stripNumber);
	s.syncAsSint16LE(_field1398);
	s.syncAsSint16LE(_invGreenCount);
	s.syncAsSint16LE(_bookGreenCount);
	s.syncAsSint16LE(_invGangCount);
	s.syncAsSint16LE(_bookGangCount);
	s.syncAsSint16LE(_field1B6C);
	s.syncAsSint16LE(_field139C);
	s.syncAsByte(_field1B68);
	s.syncAsByte(_doorOpened);
	s.syncAsSint16LE(_currentCursor);
}

void Scene315::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit(OwnerList);
	loadScene(315);

	if (BF_GLOBALS._sceneManager._previousScene != 325)
		BF_GLOBALS._sound1.fadeSound(11);

	setZoomPercents(67, 72, 124, 100);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_sutterSpeaker);
	_stripManager.addSpeaker(&_harrisonSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);
	_stripManager.addSpeaker(&_jailerSpeaker);

	_object8.postInit();
	_object8.setVisage(315);
	_object8.setPosition(Common::Point(272, 69));

	if (BF_GLOBALS._bookmark >= bLauraToParamedics) {
		_atfMemo.postInit();
		_atfMemo.setVisage(315);
		_atfMemo.setPosition(Common::Point(167, 53));
		_atfMemo.setStrip(4);
		_atfMemo.setFrame(4);
		_atfMemo.fixPriority(82);
		_atfMemo.setDetails(315, -1, -1, -1, 1, (SceneItem *)NULL);
	}

	if (BF_GLOBALS._dayNumber == 1) {
		if (BF_GLOBALS._bookmark >= bLauraToParamedics) {
			_bulletinMemo.postInit();
			_bulletinMemo.setVisage(315);
			_bulletinMemo.setPosition(Common::Point(156, 51));
			_bulletinMemo.setStrip(4);
			_bulletinMemo.setFrame(2);
			_bulletinMemo.fixPriority(82);
			_bulletinMemo.setDetails(315, -1, -1, -1, 1, (SceneItem *)NULL);
		}
	} else if ((BF_INVENTORY._daNote._sceneNumber != 1) && (BF_GLOBALS._dayNumber < 3)) {
		_object2.postInit();
		_object2.setVisage(315);
		_object2.setStrip(3);
		_object2.setFrame(2);
		_object2.setPosition(Common::Point(304, 31));
		_object2.fixPriority(70);
		_object2.setDetails(315, 3, 4, -1, 1, (SceneItem *)NULL);
	}

	_sutterSlot.setDetails(12, 315, 35, -1, 36, 1);
	_bulletinBoard.setDetails(3, 315, -1, -1, -1, 1);
	_barry.setDetails(4, 315, 10, 11, 12, 1);
	_item3.setDetails(2, 315, 0, 1, 2, 1);
	_sign.setDetails(Rect(190, 17, 208, 30), 315, -1, -1, -1, 1, NULL);
	_westExit.setDetails(Rect(184, 31, 211, 80), 315, -1, -1, -1, 1, NULL);
	_swExit.setDetails(Rect(0, 157, 190, 167), 315, -1, -1, -1, 1, NULL);

	if (!BF_GLOBALS.getFlag(onDuty) && ((BF_GLOBALS._bookmark == bNone) || (BF_GLOBALS._bookmark == bLyleStoppedBy))) {
		_field1398 = 1;
		BF_GLOBALS.setFlag(onDuty);
	} else {
		_field1398 = 0;
	}

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.changeZoom(-1);
	BF_GLOBALS._player.disableControl();

	if ((BF_GLOBALS._dayNumber != 2) || (BF_GLOBALS._sceneManager._previousScene != 325)) {
		_object4.postInit();
		_object4.setVisage(316);
		_object4.setPosition(Common::Point(99, 82));
		_object4.fixPriority(95);

		_object5.postInit();
		_object5.setVisage(395);
		_object5.setStrip(2);
		_object5.setPosition(Common::Point(96, 86));
	}

	// Set up evidence objects in inventory
	if (BF_INVENTORY._bookingGreen.inInventory())
		++_invGreenCount;
	if (BF_INVENTORY._greensGun.inInventory())
		++_invGreenCount;
	if (BF_INVENTORY._greensKnife.inInventory())
		++_invGreenCount;

	if (BF_INVENTORY._bullet22.inInventory())
		++_invGangCount;
	if (BF_INVENTORY._autoRifle.inInventory())
		++_invGangCount;
	if (BF_INVENTORY._wig.inInventory())
		++_invGangCount;
	if (BF_INVENTORY._bookingFrankie.inInventory())
		++_invGangCount;
	if (BF_INVENTORY._bookingGang.inInventory())
		++_invGangCount;
	if (BF_INVENTORY._snub22.inInventory())
		++_invGangCount;

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 190:
		if (_field1398)
			_doorOpened = true;
		_sceneMode = BF_GLOBALS.getFlag(onDuty) ? 3150 : 3165;
		setAction(&_sequenceManager, this, _sceneMode, &BF_GLOBALS._player, NULL);
		break;
	case 325:
		T2_GLOBALS._uiElements._active = false;
		_object6.postInit();
		_object7.postInit();
		_object8.setFrame(8);
		_sceneMode = (BF_GLOBALS._dayNumber == 1) ? 3152 : 3155;
		setAction(&_sequenceManager, this, _sceneMode, &BF_GLOBALS._player, &_object6,
			&_object7, &_object8, NULL);
		break;
	case 300:
	default:
		if (_field1398)
			_doorOpened = true;
		if (!BF_GLOBALS.getFlag(onDuty))
			_sceneMode = 3166;
		else if (!_field1398)
			_sceneMode = 3164;
		else
			_sceneMode = 3163;

		setAction(&_sequenceManager, this, _sceneMode, &BF_GLOBALS._player, NULL);
		break;
	}

	if (_doorOpened) {
		_object8.setFrame(8);
	} else {
		BF_GLOBALS._walkRegions.disableRegion(4);
	}

	_briefingMaterial.setDetails(24, 315, 38, 39, 40, 1);
	_cleaningKit.setDetails(14, 315, 24, 25, 26, 1);
	_item7.setDetails(5, 315, 8, 9, -1, 1);
	_item6.setDetails(6, 315, 5, 6, 7, 1);
	_item10.setDetails(8, 315, 13, -1, -1, 1);
	_item11.setDetails(9, 315, 14, -1, -1, 1);
	_item8.setDetails(7, 315, 15, 16, 17, 1);
	_item9.setDetails(10, 315, 18, 19, 20, 1);
}

void Scene315::signal() {
	int ctr = 0;

	switch (_sceneMode) {
	case 0:
		BF_GLOBALS._player.enableControl();
		break;
	case 10:
		if (_bookGreenCount) {
			if (_bookGreenCount >= _invGreenCount)
				BF_GLOBALS.setFlag(fLeftTraceIn910);
			else
				++ctr;
		}

		if (_bookGangCount) {
			if (_bookGangCount < _invGangCount)
				++ctr;
			else if (BF_GLOBALS._bookmark < bBookedFrankieEvidence)
				BF_GLOBALS._bookmark = bBookedFrankieEvidence;
		}

		if (ctr) {
			BF_GLOBALS._deathReason = 20;
			BF_GLOBALS._sceneManager.changeScene(666);
		} else {
			BF_GLOBALS._sceneManager.changeScene(300);
		}
		BF_GLOBALS._sound1.fadeOut2(NULL);
		break;
	case 11:
		if (_bookGreenCount) {
			if (_bookGreenCount >= _invGreenCount)
				BF_GLOBALS.setFlag(fLeftTraceIn910);
			else
				++ctr;
		}

		if (_bookGangCount) {
			if (_bookGangCount < _invGangCount)
				++ctr;
			else if (BF_GLOBALS._bookmark < bBookedFrankie)
				BF_GLOBALS._bookmark = bBookedFrankie;
			else if (BF_GLOBALS._bookmark < bBookedFrankieEvidence)
				BF_GLOBALS._bookmark = bBookedFrankie;
		}

		if (ctr == 1) {
			BF_GLOBALS._deathReason = 20;
			BF_GLOBALS._sound1.fadeOut2(NULL);
			BF_GLOBALS._sceneManager.changeScene(666);
		} else if ((BF_GLOBALS._bookmark != bBookedFrankie) || !BF_GLOBALS.getFlag(onDuty)) {
			BF_GLOBALS._sound1.fadeOut2(NULL);
			BF_GLOBALS._sceneManager.changeScene(190);
		} else {
			BF_GLOBALS._bookmark = bBookedFrankieEvidence;
			_field139C = 0;
			BF_GLOBALS.clearFlag(onDuty);
			BF_INVENTORY.setObjectScene(INV_TICKET_BOOK, 60);
			BF_INVENTORY.setObjectScene(INV_MIRANDA_CARD, 60);
			_sceneMode = 3165;
			setAction(&_sequenceManager, this, 3165, &BF_GLOBALS._player, NULL);
		}
		break;
	case 12:
		T2_GLOBALS._uiElements.addScore(30);
		BF_INVENTORY.setObjectScene((int)_currentCursor, 315);

		if (!_invGangCount || (_bookGangCount != _invGangCount))
			BF_GLOBALS._player.enableControl();
		else {
			_field139C = 1;
			_stripNumber = 3171;
			setAction(&_action1);
		}
		break;
	case 3150:
	case 3164:
	case 3165:
	case 3166:
		BF_GLOBALS._player.enableControl();
		_field1B68 = false;
		break;
	case 3151:
		BF_GLOBALS._sceneManager.changeScene(325);
		break;
	case 3152:
		BF_GLOBALS._walkRegions.disableRegion(4);
		_object7.remove();
		_object6.remove();
	// No break on purpose
	case 3155:
		BF_GLOBALS._player.enableControl();
		_field1B68 = false;
		BF_GLOBALS._walkRegions.disableRegion(4);
		T2_GLOBALS._uiElements._active = true;
		T2_GLOBALS._uiElements.show();
		break;
	case 3153:
		T2_GLOBALS._uiElements.addScore(30);
		BF_INVENTORY.setObjectScene((int)_currentCursor, 315);

		if (_stripNumber != 0)
			setAction(&_action1);
		else if (!_invGangCount || (_bookGangCount != _invGangCount))
			BF_GLOBALS._player.enableControl();
		else {
			_stripNumber = 3171;
			setAction(&_action1);
			_field139C = 1;
		}
		break;
	case 3156:
		T2_GLOBALS._uiElements.addScore(10);
		BF_INVENTORY.setObjectScene(INV_DA_NOTE, 1);
		_object2.remove();
		BF_GLOBALS._player.enableControl();
		break;
	case 3157:
		BF_GLOBALS._player.enableControl();
		_object9.remove();
		break;
	case 3158:
		BF_GLOBALS._player.enableControl();
		T2_GLOBALS._uiElements.addScore(10);
		BF_INVENTORY.setObjectScene(INV_FOREST_RAP, 1);
		break;
	case 3159:
		if (!BF_GLOBALS.getFlag(fBookedGreenEvidence)) {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fBookedGreenEvidence);
		}
		BF_GLOBALS.setFlag(gunClean);
		BF_GLOBALS._player.enableControl();
		break;
	case 3161:
		BF_GLOBALS._deathReason = 21;
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 3162:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 3161;
		setAction(&_sequenceManager, this, 3161, &BF_GLOBALS._player, NULL);
		BF_GLOBALS.setFlag(fShotSuttersDesk);
		break;
	case 3163:
		_sceneMode = 3150;
		setAction(&_sequenceManager, this, 3150, &BF_GLOBALS._player, NULL);
		break;
	case 3167:
		BF_GLOBALS._player.enableControl();
		_object9.remove();
		break;
	case 3169:
		T2_GLOBALS._uiElements.addScore(30);
		BF_INVENTORY.setObjectScene(INV_MUG_SHOT, 1);
		BF_GLOBALS._player.enableControl();
		break;
	case 3154:
	default:
		break;
	}
}

void Scene315::process(Event &event) {
	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1))) {
		// Check if the cursor is on an exit
		if (_swExit.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_SW);
			BF_GLOBALS._events.setCursor(surface);
		} else if ((BF_GLOBALS._bookmark != bBookedFrankie) && _westExit.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_W);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			// In case an exit cursor was being shown, restore the previously selected cursor
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene315::dispatch() {
	SceneExt::dispatch();

	if (_field1B68)
		return;

	if (_doorOpened) {
		if (BF_GLOBALS._player._position.y < 69) {
			BF_GLOBALS._player.disableControl();
			_field1B68 = true;
			_sceneMode = 3151;
			setAction(&_sequenceManager, this, 3151, &BF_GLOBALS._player, NULL);
		} else if (BF_GLOBALS._player.getRegionIndex() == 1) {
			BF_GLOBALS._player.disableControl();
			_field1B68 = true;
			SceneItem::display2(315, 28);
			_sceneMode = 3150;
			ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x + 30,
				BF_GLOBALS._player._position.y + 15);
		} else if (BF_GLOBALS._player._position.y > 156) {
			BF_GLOBALS._player.disableControl();
			_field1B68 = true;
			SceneItem::display2(315, 28);
			_sceneMode = 3150;
			ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x + 30,
				BF_GLOBALS._player._position.y - 24);
		}
	}  else if (BF_GLOBALS._player.getRegionIndex() == 1) {
		BF_GLOBALS._player.disableControl();
		_field1B68 = true;
		_sceneMode = 11;
		ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x - 30,
			BF_GLOBALS._player._position.y - 5);
	} else if (BF_GLOBALS._player._position.y > 156) {
		BF_GLOBALS._player.disableControl();
		_field1B68 = true;

		if (_field139C) {
			SceneItem::display2(315, 45);
			_sceneMode = 3150;
			ADD_MOVER(BF_GLOBALS._player, 112, 152);
		} else {
			_sceneMode = 10;
			ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x - 150,
				BF_GLOBALS._player._position.y + 120);
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 325 - Police Station Conference Room
 *
 *--------------------------------------------------------------------------*/

bool Scene325::Item1::startAction(CursorType action, Event &event) {
	if (action == CURSOR_EXIT) {
		BF_GLOBALS._events.setCursor(CURSOR_WALK);
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._sceneManager.changeScene(315);
		return true;
	} else {
		return false;
	}
}

/*--------------------------------------------------------------------------*/

void Scene325::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(325);
	BF_GLOBALS._interfaceY = 200;
	BF_GLOBALS.clearFlag(fCanDrawGun);

	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 1;

	// Add the speakers
	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_PSutterSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.hide();

	if (BF_GLOBALS._dayNumber == 1) {
		_object1.postInit();
		_object1.setVisage(325);
		_object1.setStrip(8);
		_object1.setPosition(Common::Point(128, 44));
	} else {
		_object1.postInit();
		_object1.setVisage(325);
		_object1.setStrip(8);
		_object1.setFrame(2);
		_object1.setPosition(Common::Point(132, 28));

		_object2.postInit();
		_object2.setVisage(325);
		_object2.setStrip(8);
		_object2.setFrame(3);
		_object2.setPosition(Common::Point(270, 24));
	}

	_object3.postInit();
	_object3.setVisage(335);
	_object3.setStrip(4);
	_object3.setPosition(Common::Point(202, 122));

	_object4.postInit();
	_object4.setVisage(335);
	_object4.setStrip(2);
	_object4.setPosition(Common::Point(283, 102));

	_object5.postInit();
	_object5.setVisage(335);
	_object5.setStrip(1);
	_object5.setPosition(Common::Point(135, 167));

	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 560, -1, -1, -1, 1, NULL);
	BF_GLOBALS._player.disableControl();

	_sceneMode = (BF_GLOBALS._dayNumber == 1) ? 3250 : 3251;
	setAction(&_sequenceManager, this, _sceneMode, &_object3, &_object4, &_object5, NULL);
}

void Scene325::signal() {
	BF_GLOBALS._player._uiEnabled = 0;
	BF_GLOBALS._player._canWalk = true;
	BF_GLOBALS._player._enabled = true;
	BF_GLOBALS._events.setCursor(CURSOR_EXIT);
}

/*--------------------------------------------------------------------------
 * Scene 330 - Approaching Marina
 *
 *--------------------------------------------------------------------------*/

void Scene330::Timer1::signal() {
	PaletteRotation *rotation = BF_GLOBALS._scenePalette.addRotation(240, 254, 1);
	rotation->setDelay(25);
	remove();
}

/*--------------------------------------------------------------------------*/

Scene330::Scene330() {
	_seqNumber = 0;
}

void Scene330::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_seqNumber);
}

void Scene330::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	BF_GLOBALS._sound1.changeSound(35);
	_sound1.fadeSound(35);

	loadScene(850);
	_timer.set(2, NULL);

	if (BF_GLOBALS._dayNumber >= 4) {
		_object2.postInit();
		_object2.setVisage(851);
		_object2.setPosition(Common::Point(120, 112));
	}

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 850 : 852);
	BF_GLOBALS._player.setStrip(2);
	BF_GLOBALS._player.setFrame(1);
	BF_GLOBALS._player.hide();
	if (BF_GLOBALS.getFlag(fWithLyle))
		BF_GLOBALS._player.setStrip(5);

	if ((BF_GLOBALS._dayNumber == 1) && BF_GLOBALS.getFlag(fBackupArrived340)) {
		_object1.postInit();
		_object1.setVisage(850);
		_object1.setStrip(6);
		_object1.setFrame(1);
		_object1.setPosition(Common::Point(47, 169));
		_object1.animate(ANIM_MODE_2);
	}

	if (BF_GLOBALS._sceneManager._previousScene == 50) {
		// Coming from map
		if ((BF_GLOBALS._driveFromScene == 340) || (BF_GLOBALS._driveFromScene == 342) ||
				(BF_GLOBALS._driveFromScene == 330)) {
			if (BF_GLOBALS.getFlag(fWithLyle)) {
				_seqNumber = 3304;
			} else {
				_seqNumber = 3302;
				_sound2.play(123);
				BF_GLOBALS.setFlag(onBike);
			}
		} else if (BF_GLOBALS.getFlag(fWithLyle)) {
			_seqNumber = 3303;
		} else {
			_sound2.play(123);
			_seqNumber = 3301;

			if ((BF_GLOBALS._dayNumber == 1) && (BF_GLOBALS._bookmark >= bStartOfGame) &&
					(BF_GLOBALS._bookmark < bCalledToDomesticViolence)) {
				BF_GLOBALS._player.animate(ANIM_MODE_2);
			}
		}
	} else if (BF_GLOBALS.getFlag(fWithLyle)) {
		_seqNumber = 3303;
	} else {
		_seqNumber = 3301;
		_sound2.play(123);

		if ((BF_GLOBALS._dayNumber == 1) && (BF_GLOBALS._bookmark >= bStartOfGame) &&
				(BF_GLOBALS._bookmark < bCalledToDomesticViolence)) {
			BF_GLOBALS._player.animate(ANIM_MODE_2);
		}
	}

	BF_GLOBALS._player.disableControl();
	_sceneMode = 0;
	setAction(&_sequenceManager, this, _seqNumber, &BF_GLOBALS._player, NULL);
}

void Scene330::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	SceneExt::remove();
}

void Scene330::signal() {
	if ((BF_GLOBALS._driveFromScene == 330) || (BF_GLOBALS._driveFromScene == 340) ||
			(BF_GLOBALS._driveFromScene == 342)) {
		// Leaving marina
		if ((BF_GLOBALS._dayNumber != 1) || (BF_GLOBALS._bookmark < bStartOfGame) ||
				(BF_GLOBALS._bookmark >= bCalledToDomesticViolence))
			// Leave scene normally
			BF_GLOBALS._sceneManager.changeScene(BF_GLOBALS._driveToScene);
		else {
			// Player leaves with domestic violence unresolved
			BF_GLOBALS._player.hide();
			BF_GLOBALS._deathReason = 4;
			BF_GLOBALS._sceneManager.changeScene(666);
		}
	} else {
		// Arriving at marina
		BF_GLOBALS.clearFlag(onBike);

		if ((BF_GLOBALS._dayNumber != 1) || (BF_GLOBALS._bookmark < bStartOfGame) ||
				(BF_GLOBALS._bookmark >= bCalledToDomesticViolence))
			BF_GLOBALS._sceneManager.changeScene(342);
		else
			BF_GLOBALS._sceneManager.changeScene(340);
	}
}

/*--------------------------------------------------------------------------
 * Scene 340 - Marina, Domestic Disturbance
 *
 *--------------------------------------------------------------------------*/

bool Scene340::Child::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(340, 5);
		return true;
	case CURSOR_USE:
		SceneItem::display2(340, 14);
		return true;
	case CURSOR_TALK:
		if (!BF_GLOBALS.getFlag(fBackupArrived340) || (BF_GLOBALS._marinaWomanCtr < 3) || !BF_GLOBALS.getFlag(fGotAllSkip340))
			scene->setAction(&scene->_action3);
		else
			scene->setAction(&scene->_action2);
		return true;
	case INV_COLT45:
		scene->gunDisplay();
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

bool Scene340::Woman::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(340, 4);
		return true;
	case CURSOR_USE:
		SceneItem::display2(340, 13);
		return true;
	case CURSOR_TALK:
		if (!BF_GLOBALS.getFlag(fBackupArrived340) || (BF_GLOBALS._marinaWomanCtr < 3) || !BF_GLOBALS.getFlag(fGotAllSkip340))
			scene->setAction(&scene->_action1);
		else
			scene->setAction(&scene->_action2);
		return true;
	case INV_COLT45:
		scene->gunDisplay();
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

bool Scene340::Harrison::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(340, 6);
		return true;
	case CURSOR_USE:
		SceneItem::display2(340, 15);
		return true;
	case CURSOR_TALK:
		scene->setAction(&scene->_action5);
		return true;
	case INV_COLT45:
		scene->gunDisplay();
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}


/*--------------------------------------------------------------------------*/

bool Scene340::Item1::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	if (action == INV_COLT45) {
		scene->gunDisplay();
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene340::WestExit::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	if (BF_GLOBALS.getFlag(fBackupArrived340)) {
		scene->setAction(&scene->_action6);
	} else {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 4;
		setAction(&scene->_sequenceManager1, scene, 1348, &BF_GLOBALS._player, NULL);
	}
	return true;
}

bool Scene340::SouthWestExit::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	BF_GLOBALS._player.disableControl();
	scene->_sceneMode = 3;
	setAction(&scene->_sequenceManager1, scene, 1340, &BF_GLOBALS._player, NULL);
	return true;
}

bool Scene340::NorthExit::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 254, 106);
	return true;
}

/*--------------------------------------------------------------------------*/

void Scene340::Action1::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER(64, 155);
		break;
	}
	case 1:
		BF_GLOBALS._player.changeAngle(45);
		setDelay(3);
		break;
	case 2:
		if (!BF_GLOBALS._marinaWomanCtr) {
			setAction(&scene->_action8, this);
		} else if (!_action) {
			BF_GLOBALS._player.setAction(&scene->_sequenceManager1, this,
					MIN(BF_GLOBALS._marinaWomanCtr, 3) + 2340, &scene->_woman, &scene->_child,
					&scene->_object4, NULL);
		}
		break;
	case 3:
		if ((BF_GLOBALS._marinaWomanCtr != 1) || BF_GLOBALS.getFlag(fCalledBackup)) {
			setDelay(3);
		} else {
			scene->_sound1.play(8);
			scene->_stripManager.start(3413, this);
		}
		break;
	case 4:
		if (BF_GLOBALS._marinaWomanCtr == 1)
			++BF_GLOBALS._marinaWomanCtr;

		if (BF_GLOBALS.getFlag(fBackupArrived340)) {
			scene->_backupPresent = 1;
			scene->_harrison.setPosition(Common::Point(46, 154));
			BF_GLOBALS._walkRegions.disableRegion(19);
		} else if (BF_GLOBALS.getFlag(fCalledBackup)) {
			scene->_timer1.set(40, &scene->_harrison, &scene->_action4);
		}

		++BF_GLOBALS._marinaWomanCtr;
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene340::Action2::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER(64, 155);
		break;
	}
	case 1:
		BF_GLOBALS._player.changeAngle(45);
		setDelay(3);
		break;
	case 2:
		BF_GLOBALS._player.setAction(&scene->_sequenceManager3, this, 1341, &scene->_woman, &scene->_child, NULL);
		break;
	case 3:
		scene->_woman.remove();
		scene->_child.remove();
		BF_GLOBALS.setFlag(fToldToLeave340);
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene340::Action3::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER(64, 155);
		break;
	}
	case 1:
		BF_GLOBALS._player.changeAngle(45);
		setDelay(3);
		break;
	case 2:
		scene->_stripManager.start(scene->_womanDialogCount + 3404, this);
		break;
	case 3:
		if (++scene->_womanDialogCount > 2) {
			if (!BF_GLOBALS.getFlag(fGotAllSkip340))
				BF_GLOBALS.setFlag(fGotAllSkip340);
			scene->_womanDialogCount = 0;
		}

		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene340::Action4::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		if (!_action) {
			BF_GLOBALS._player.disableControl();
			setDelay(3);
		} else {
			scene->_timer1.set(30, &scene->_harrison, &scene->_action4);
			remove();
		}
		break;
	case 1:
		BF_GLOBALS.setFlag(fBackupArrived340);
		scene->_backupPresent = 1;
		setDelay(3);
		break;
	case 2:
		BF_GLOBALS._player.setAction(&scene->_sequenceManager3, this, 1347, &scene->_harrison, NULL);
		break;
	case 3:
		BF_GLOBALS._walkRegions.disableRegion(19);
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene340::Action5::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		if (scene->_backupPresent) {
			ADD_PLAYER_MOVER(64, 155);
		} else {
			BF_GLOBALS._player.changeAngle(45);
			setDelay(3);
		}
		break;
	case 1:
		BF_GLOBALS._player.updateAngle(scene->_harrison._position);
		setDelay(3);
		break;
	case 2:
		setDelay(15);
		break;
	case 3:
		if (BF_GLOBALS.getFlag(fBriefedBackup))
			scene->_stripManager.start(3416, this);
		else {
			BF_GLOBALS.setFlag(fBriefedBackup);
			scene->_stripManager.start(3407, this);
		}
		break;
	case 4:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene340::Action6::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER(10, 110);
		break;
	case 1:
		BF_GLOBALS._player.updateAngle(scene->_harrison._position);
		scene->_harrison.updateAngle(BF_GLOBALS._player._position);
		scene->_stripManager.start(3415, this);
		break;
	case 2: {
		ADD_MOVER(BF_GLOBALS._player, -8, 110);
		break;
	}
	case 3:
		scene->_sceneMode = 4;
		scene->signal();
		remove();
		break;
	}
}

void Scene340::Action7::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER(254, 121);
		break;
	case 1:
		BF_GLOBALS._player.updateAngle(scene->_harrison._position);
		scene->_stripManager.start(BF_GLOBALS.getFlag(fBriefedBackup) ? 3414 : 3417, this);
		break;
	case 2:
		BF_GLOBALS.setFlag(fBackupIn350);
		BF_GLOBALS._sceneManager.changeScene(350);
		break;
	default:
		break;
	}
}

void Scene340::Action8::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_stripManager.start(3400, this);
		break;
	case 1:
		setDelay(2);
		break;
	case 2:
		scene->_object4.setPriority(250);
		scene->_object4.setPosition(Common::Point(138, 130));
		scene->_object4.setVisage(347);
		scene->_object4.setStrip(6);
		scene->_object4.setFrame(1);
		scene->_object4._numFrames = 2;
		scene->_object4.animate(ANIM_MODE_5, NULL);
		scene->_object4.show();
		break;
	case 3:
		scene->_object4.hide();

		scene->_woman.setPriority(123);
		scene->_woman.setPosition(Common::Point(88, 143));
		scene->_woman.setVisage(344);
		scene->_woman.setStrip(2);
		scene->_woman.setFrame(1);
		scene->_woman.changeZoom(100);
		scene->_woman._numFrames = 10;
		scene->_woman._moveRate = 10;
		scene->_woman._moveDiff = Common::Point(3, 2);
		scene->_woman.show();

		scene->_child.setPriority(120);
		scene->_child.setPosition(Common::Point(81, 143));
		scene->_child.setVisage(347);
		scene->_child.setStrip(3);
		scene->_child.setFrame(1);
		scene->_child.changeZoom(100);
		scene->_child._numFrames = 10;
		scene->_child._moveRate = 10;
		scene->_child.show();

		setDelay(6);
		break;
	case 4:
		remove();
		break;
	default:
		// This is present in the original game
		warning("Bugs");
		remove();
		break;
	}
}

void Scene340::Action8::process(Event &event) {
	if ((_actionIndex != 3) || (event.eventType == EVENT_NONE))
		Action::process(event);
	else if (event.eventType == EVENT_BUTTON_DOWN) {
		event.handled = true;
		setDelay(2);
	}
}

/*--------------------------------------------------------------------------*/

void Scene340::Timer2::signal() {
	PaletteRotation *item;

	item = BF_GLOBALS._scenePalette.addRotation(235, 239, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(247, 249, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(240, 246, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(252, 254, 1);
	item->setDelay(30);

	remove();
}

/*--------------------------------------------------------------------------*/

Scene340::Scene340(): PalettedScene() {
	_seqNumber1 = _womanDialogCount = _backupPresent = 0;
}

void Scene340::synchronize(Serializer &s) {
	PalettedScene::synchronize(s);

	s.syncAsSint16LE(_seqNumber1);
	s.syncAsSint16LE(_womanDialogCount);
	s.syncAsSint16LE(_backupPresent);
}

void Scene340::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit();
	loadScene(340);
	setZoomPercents(126, 70, 162, 100);

	BF_GLOBALS._walkRegions.disableRegion(13);
	BF_GLOBALS._walkRegions.disableRegion(15);
	_timer2.set(2, NULL);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);

	_womanDialogCount = 0;
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	_westExit.setDetails(Rect(0, 76, 19, 115), 340, -1, -1, -1, 1, NULL);
	_swExit.setDetails(15, 340, -1, -1, -1, 1);
	_northExit.setDetails(16, 340, -1, -1, -1, 1);

	BF_GLOBALS._player._regionBitList |= 0x10000;
	BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 1341 : 129);
	BF_GLOBALS._player._moveDiff = Common::Point(5, 2);

	if (BF_GLOBALS._dayNumber < 5) {
		_object5.postInit();
		_object5.setVisage(340);
		_object5.setStrip(4);
		_object5.setPosition(Common::Point(259, 61));
	}

	if (!BF_GLOBALS.getFlag(fToldToLeave340)) {
		_woman.postInit();
		_woman.setVisage(344);
		_woman.setStrip(3);
		_woman.setFrame(1);
		_woman.fixPriority(123);
		_woman.setPosition(Common::Point(88, 143));

		_object4.postInit();
		_object4.hide();

		_child.postInit();
		_child.setVisage(347);
		_child.setStrip(3);
		_child.setFrame(1);
		_child.fixPriority(120);
		_child.setPosition(Common::Point(81, 143));

		_woman.setAction(&_sequenceManager2, NULL, 348, &_woman, &_child, &_object4, NULL);
		BF_GLOBALS._sceneItems.addItems(&_child, &_woman, NULL);

		_stripManager.addSpeaker(&_jordanSpeaker);
		_stripManager.addSpeaker(&_skipBSpeaker);

		BF_GLOBALS.set2Flags(f1097Marina);
	}

	if (BF_GLOBALS.getFlag(fCalledBackup)) {
		_harrison.postInit();
		_harrison.setVisage(326);
		_harrison.setObjectWrapper(new SceneObjectWrapper());
		_harrison.animate(ANIM_MODE_1, NULL);
		_harrison.setPosition(Common::Point(-60, 219));
		BF_GLOBALS._sceneItems.push_back(&_harrison);

		_stripManager.addSpeaker(&_harrisonSpeaker);
		if (BF_GLOBALS.getFlag(fBackupIn350)) {
			_backupPresent = 0;
			_harrison.setVisage(1355);
			_harrison.setPosition(Common::Point(289, 112));
			_harrison.changeAngle(225);
			_harrison.setFrame(1);
			_harrison.fixPriority(75);

			BF_GLOBALS._walkRegions.disableRegion(23);
		} else if (BF_GLOBALS.getFlag(fBackupArrived340)) {
			_backupPresent = 1;
			_harrison.setPosition(Common::Point(46, 154));
			BF_GLOBALS._walkRegions.disableRegion(19);
		} else if (BF_GLOBALS.getFlag(fCalledBackup) && (BF_GLOBALS._marinaWomanCtr > 0)) {
			_timer1.set(900, &_harrison, &_action4);
		}
	}

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 350:
		_seqNumber1 = 1342;
		break;
	case 830:
		BF_GLOBALS._player._regionBitList |= 0x800;
		_seqNumber1 = 1343;
		break;
	case 60:
		_seqNumber1 = 342;
		break;
	default:
		_sound1.fadeSound(35);
		BF_GLOBALS._sound1.fadeSound((BF_GLOBALS._bookmark < bStartOfGame) ||
				(BF_GLOBALS._bookmark > bCalledToDomesticViolence) ? 10 : 19);
		_seqNumber1 = 342;
		break;
	}

	_item3.setDetails(7, 340, 3, 9, 12, 1);
	_item2.setDetails(Rect(0, 0, SCREEN_WIDTH - 1, 39), 340, 2, 8, 11, 1, NULL);
	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH - 1, UI_INTERFACE_Y), 340, 1, 7, 10, 1, NULL);

	BF_GLOBALS._player.disableControl();
	_sceneMode = 0;
	setAction(&_sequenceManager1, this, _seqNumber1, &BF_GLOBALS._player, NULL);
}

void Scene340::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	PalettedScene::remove();
}

void Scene340::signal() {
	switch (_sceneMode) {
	case 1:
	case 2:
		BF_GLOBALS._sceneManager.changeScene(350);
		break;
	case 3:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 4:
		BF_GLOBALS._sceneManager.changeScene(830);
		break;
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene340::process(Event &event) {
	// Check for gun being clicked on player
	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == INV_COLT45) &&
			BF_GLOBALS._player.contains(event.mousePos)) {
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		SceneItem::display2(350, 26);

		signal();
		event.handled = true;
	}

	if (!event.handled) {
		SceneExt::process(event);

		if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1))) {
			// Check if the cursor is on an exit
			if (_westExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_W);
				BF_GLOBALS._events.setCursor(surface);
			} else if (_swExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_SW);
				BF_GLOBALS._events.setCursor(surface);
			} else if (_northExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_N);
				BF_GLOBALS._events.setCursor(surface);
			} else {
				// In case an exit cursor was being shown, restore the previously selected cursor
				CursorType cursorId = BF_GLOBALS._events.getCursor();
				BF_GLOBALS._events.setCursor(cursorId);
			}
		}
	}
}

void Scene340::dispatch() {
	SceneExt::dispatch();
	int idx = BF_GLOBALS._player.getRegionIndex();

	if (idx == 20) {
		BF_GLOBALS._player.updateZoom();
		BF_GLOBALS._player.fixPriority(75);
	}
	if (idx == 26) {
		BF_GLOBALS._player.updateZoom();
	}
	if (idx == 31) {
		BF_GLOBALS._player.changeZoom(-1);
		BF_GLOBALS._player.fixPriority(-1);
	}

	if (BF_GLOBALS._player._regionIndex == 16) {
		BF_GLOBALS._player._regionBitList &= ~0x10000;

		if (!BF_GLOBALS.getFlag(fBackupArrived340)) {
			_sceneMode = 1;
			ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, 254, 110);
		} else if (!BF_GLOBALS.getFlag(fBackupIn350)) {
			setAction(&_action7);
		} else {
			_sceneMode = 1;
			ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, 254, 110);
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 342 - Marina, Normal
 *
 *--------------------------------------------------------------------------*/

bool Scene342::Lyle::startAction(CursorType action, Event &event) {
	Scene342 *scene = (Scene342 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(340, 16);
		return true;
	case CURSOR_USE:
		SceneItem::display2(340, 15);
		return true;
	case CURSOR_TALK:
		SceneItem::display2(340, 17);
		return true;
	case INV_COLT45:
		scene->gunDisplay();
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene342::Item1::startAction(CursorType action, Event &event) {
	Scene342 *scene = (Scene342 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case INV_COLT45:
		scene->gunDisplay();
		return true;
	default:
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene342::WestExit::startAction(CursorType action, Event &event) {
	Scene342 *scene = (Scene342 *)BF_GLOBALS._sceneManager._scene;

	BF_GLOBALS._player.disableControl();
	scene->_sceneMode = 4;
	setAction(&scene->_sequenceManager1, scene, 1348, &BF_GLOBALS._player, NULL);
	return true;
}

bool Scene342::SouthWestExit::startAction(CursorType action, Event &event) {
	Scene342 *scene = (Scene342 *)BF_GLOBALS._sceneManager._scene;

	BF_GLOBALS._player.disableControl();
	scene->_sceneMode = 3;
	setAction(&scene->_sequenceManager1, scene, 1340, &BF_GLOBALS._player, NULL);
	return true;
}

bool Scene342::NorthExit::startAction(CursorType action, Event &event) {
	Scene342 *scene = (Scene342 *)BF_GLOBALS._sceneManager._scene;

	scene->_sceneMode = 1;
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 254, 106);
	return true;
}

/*--------------------------------------------------------------------------*/

void Scene342::Timer1::signal() {
	PaletteRotation *item;

	item = BF_GLOBALS._scenePalette.addRotation(235, 239, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(247, 249, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(240, 246, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(252, 254, 1);
	item->setDelay(30);

	remove();
}

/*--------------------------------------------------------------------------*/

Scene342::Scene342(): PalettedScene() {
	_field1A1A = 0;
}

void Scene342::synchronize(Serializer &s) {
	PalettedScene::synchronize(s);

	s.syncAsSint16LE(_field1A1A);
}

void Scene342::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit();
	loadScene(340);
	setZoomPercents(126, 70, 162, 100);

	BF_GLOBALS._walkRegions.disableRegion(13);
	BF_GLOBALS._walkRegions.disableRegion(15);

	_field1A1A = 0;
	_timer1.set(2, NULL);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_lyleHatSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	_westExit.setDetails(Rect(0, 76, 19, 115), 340, -1, -1, -1, 1, NULL);
	_swExit.setDetails(15, 340, -1, -1, -1, 1);
	_northExit.setDetails(16, 340, -1, -1, -1, 1);

	if (BF_GLOBALS._dayNumber < 5) {
		_object3.postInit();
		_object3.setVisage(340);
		_object3.setStrip(4);
		_object3.setPosition(Common::Point(259, 61));
	}

	if (BF_GLOBALS.getFlag(onDuty)) {
		BF_GLOBALS._player.setVisage(1341);
		BF_GLOBALS._player._moveDiff = Common::Point(5, 2);
	} else {
		BF_GLOBALS._player.setVisage(129);
		BF_GLOBALS._player._moveDiff = Common::Point(5, 2);

		if (BF_GLOBALS.getFlag(fWithLyle)) {
			_lyle.postInit();
			_lyle.setVisage(469);
			_lyle.setObjectWrapper(new SceneObjectWrapper());
			_lyle.animate(ANIM_MODE_1, NULL);
			_lyle.hide();
		}
	}

	switch (BF_GLOBALS._randomSource.getRandomNumber(2)) {
	case 0:
		_object2.setPosition(Common::Point(46, 59));
		ADD_MOVER_NULL(_object2, 300, -10);
		break;
	case 1:
		_object2.setPosition(Common::Point(311, 57));
		ADD_MOVER_NULL(_object2, 140, -10);
		break;
	case 2:
		_object2.setPosition(Common::Point(-5, 53));
		ADD_MOVER_NULL(_object2, 170, -10);
		break;
	default:
		break;
	}

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 50:
	case 60:
	case 330:
		if (BF_GLOBALS._sceneManager._previousScene != 60) {
			_sound1.fadeSound(35);
			BF_GLOBALS._sound1.fadeSound((BF_GLOBALS._bookmark < bStartOfGame) ||
				(BF_GLOBALS._bookmark > bCalledToDomesticViolence) ? 10 : 19);
		}

		if (BF_GLOBALS.getFlag(fWithLyle)) {
			BF_GLOBALS._walkRegions.disableRegion(19);
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;

			setAction(&_sequenceManager1, this, 346, &_lyle, NULL);
			BF_GLOBALS._player.setAction(&_sequenceManager2, NULL, 342, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager1, this, 342, &BF_GLOBALS._player, NULL);
		}
		break;
	case 350:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 1342, &BF_GLOBALS._player, NULL);
		break;
	default:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 1343, &BF_GLOBALS._player, NULL);

		if (BF_GLOBALS.getFlag(fWithLyle)) {
			_lyle.setPosition(Common::Point(-21, 118));
			_lyle.setStrip(5);
			_lyle.setFrame(1);
			_lyle.setPriority(75);
			_lyle.setZoom(75);
			_lyle.setAction(&_sequenceManager2, NULL, 347, &_lyle, NULL);
			BF_GLOBALS._sceneItems.push_back(&_lyle);
		}
		break;
	}

	_item3.setDetails(7, 340, 3, 9, 12, 1);
	_item2.setDetails(Rect(0, 0, SCREEN_WIDTH - 1, 39), 340, 2, 8, 11, 1, NULL);
	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH - 1, UI_INTERFACE_Y), 340, 1, 7, 10, 1, NULL);
}

void Scene342::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	PalettedScene::remove();
}

void Scene342::signal() {
	switch (_sceneMode) {
	case 1:
		ADD_PLAYER_MOVER(254, 106);
		BF_GLOBALS._sceneManager.changeScene(350);
		break;
	case 3:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 4:
		BF_GLOBALS._sceneManager.changeScene(830);
		break;
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene342::process(Event &event) {
	// Check for gun being clicked on player
	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == INV_COLT45) &&
			BF_GLOBALS._player.contains(event.mousePos)) {
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		SceneItem::display2(350, 26);

		signal();
		event.handled = true;
	}

	if (!event.handled) {
		SceneExt::process(event);

		if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1))) {
			// Check if the cursor is on an exit
			if (_westExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_W);
				BF_GLOBALS._events.setCursor(surface);
			} else if (_swExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_SW);
				BF_GLOBALS._events.setCursor(surface);
			} else if (_northExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_N);
				BF_GLOBALS._events.setCursor(surface);
			} else {
				// In case an exit cursor was being shown, restore the previously selected cursor
				CursorType cursorId = BF_GLOBALS._events.getCursor();
				BF_GLOBALS._events.setCursor(cursorId);
			}
		}
	}
}

void Scene342::dispatch() {
	SceneExt::dispatch();
	int idx = BF_GLOBALS._player.getRegionIndex();

	if (idx == 20) {
		BF_GLOBALS._player.updateZoom();
		BF_GLOBALS._player.fixPriority(75);
	}
	if (idx == 26) {
		BF_GLOBALS._player.updateZoom();
	}
	if (idx == 31) {
		BF_GLOBALS._player.changeZoom(-1);
		BF_GLOBALS._player.fixPriority(-1);
	}

	if (idx == 16) {
		BF_GLOBALS._player.enableControl();
		ADD_PLAYER_MOVER(254, 110);
	}
}

/*--------------------------------------------------------------------------
 * Scene 350 - Marina, Outside Boat
 *
 *--------------------------------------------------------------------------*/

bool Scene350::FireBox::startAction(CursorType action, Event &event) {
	Scene350 *scene = (Scene350 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(350, 20);
		return true;
	case CURSOR_USE: {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 3;

		Common::Point pt(76, 154);
		PlayerMover *mover = new PlayerMover();
		BF_GLOBALS._player.addMover(mover, &pt, scene);
		return true;
	}
	default:
		return NamedHotspot::startAction(action, event);
	}
}

void Scene350::Yacht::synchronize(Serializer &s) {
	NamedHotspot::synchronize(s);
	s.syncAsSint16LE(_flag);
}

bool Scene350::Yacht::startAction(CursorType action, Event &event) {
	Scene350 *scene = (Scene350 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if ((BF_GLOBALS._dayNumber != 1)  || !BF_GLOBALS.getFlag(onDuty) || (BF_GLOBALS._bookmark < bStartOfGame)) {
			if ((BF_GLOBALS._dayNumber == 1) || (BF_GLOBALS._dayNumber == 4)) {
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 1;
				scene->setAction(&scene->_sequenceManager1, scene, 3512, &BF_GLOBALS._player, &scene->_yachtDoor, NULL);
				return true;
			}
		} else {
			_flag = true;
			scene->_sceneMode = 1;
			BF_GLOBALS._player.disableControl();
			scene->setAction(&scene->_sequenceManager1, scene, BF_GLOBALS.getFlag(gunDrawn) ? 3504 : 3505,
				&BF_GLOBALS._player, &scene->_yachtDoor, NULL);
			return true;
		}
		break;
	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

bool Scene350::SouthWestExit::startAction(CursorType action, Event &event) {
	BF_GLOBALS._player.disableControl();

	Scene350 *scene = (Scene350 *)BF_GLOBALS._sceneManager._scene;
	scene->_sceneMode = 2;

	if (BF_GLOBALS.getFlag(fBackupIn350))
		scene->setAction(&scene->_sequenceManager1, scene, 3507, &BF_GLOBALS._player, &scene->_harrison, NULL);
	else
		scene->setAction(&scene->_sequenceManager1, scene, 3510, &BF_GLOBALS._player, NULL);
	return true;
}

/*--------------------------------------------------------------------------*/

bool Scene350::Hook::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(350, 30);
		return true;
	case CURSOR_USE:
		BF_INVENTORY.setObjectScene(INV_HOOK, 1);
		if (!BF_GLOBALS.getFlag(hookPoints)) {
			BF_GLOBALS.setFlag(hookPoints);
			T2_GLOBALS._uiElements.addScore(30);
		}
		remove();
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene350::FireboxInset::startAction(CursorType action, Event &event) {
	Scene350 *scene = (Scene350 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(350, BF_INVENTORY.getObjectScene(INV_HOOK) ? 29 : 28);
		return true;
	case CURSOR_USE:
		scene->_fireBoxInset.remove();
		return true;
	case INV_HOOK:
		BF_INVENTORY.setObjectScene(INV_HOOK, 350);

		scene->_hook.postInit();
		scene->_hook.setVisage(350);
		scene->_hook.setStrip(5);
		scene->_hook.fixPriority(201);
		scene->_hook.setPosition(Common::Point(106, 146));
		BF_GLOBALS._sceneItems.push_front(&scene->_hook);
		return true;
	default:
		return FocusObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene350::Timer1::signal() {
	PaletteRotation *rot;
	rot = BF_GLOBALS._scenePalette.addRotation(233, 235, 1);
	rot->setDelay(40);
	rot = BF_GLOBALS._scenePalette.addRotation(236, 238, 1);
	rot->setDelay(20);
	rot = BF_GLOBALS._scenePalette.addRotation(239, 241, 1);
	rot->setDelay(20);
	rot = BF_GLOBALS._scenePalette.addRotation(242, 244, 1);
	rot->setDelay(12);

	remove();
}

/*--------------------------------------------------------------------------*/

Scene350::Scene350(): SceneExt() {
	_field1D44 = _field1D46 = 0;
}

void Scene350::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(350);
	setZoomPercents(90, 80, 143, 100);
	_sound1.fadeSound(35);
	_timer1.set(2, NULL);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 352 : 1358);
	BF_GLOBALS._player.setFrame(1);
	BF_GLOBALS._player.changeZoom(-1);
	BF_GLOBALS._player.setPosition(Common::Point(99, 152));
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player._moveDiff = Common::Point(3, 1);

	_yacht._flag = false;
	_swExit.setDetails(Rect(0, 160, SCREEN_WIDTH, UI_INTERFACE_Y - 1), 350, -1, -1, -1, 1, NULL);

	if ((BF_GLOBALS._dayNumber != 1) && (BF_GLOBALS._dayNumber != 4)) {
		_yacht.setDetails(28, 350, 15, 16, 17, 1);
	} else {
		_yachtDoor.postInit();
		_yachtDoor.setVisage(350);
		_yachtDoor.setStrip(3);
		_yachtDoor.setFrame(1);
		_yachtDoor.fixPriority(72);
		_yachtDoor.setPosition(Common::Point(40, 74));

		_yachtBody.setup(350, 1, 1, 129, 142, 255);

		if (BF_GLOBALS.getFlag(fBackupIn350)) {
			_harrison.postInit();
			_harrison.setVisage(1355);
			_harrison.setObjectWrapper(new SceneObjectWrapper());
			_harrison.animate(ANIM_MODE_1, NULL);
			_harrison.changeZoom(-1);
			_harrison.setDetails(350, 12, 13, 14, 1, (SceneItem *)NULL);
			_harrison._moveDiff = Common::Point(2, 1);

			_stripManager.addSpeaker(&_harrisonSpeaker);
		}

		if (BF_GLOBALS._bookmark < bStartOfGame) {
			// Setup scene in debug mode
			_yacht.setDetails(28, 350, 6, 18, 19, 1);
		} else {
			_yacht.setDetails(28, 350, 6, BF_GLOBALS.getFlag(bStartOfGame) ? 7 : 18, 8, 1);
		}
	}

	_fireBox._sceneRegionId = 5;
	BF_GLOBALS._sceneItems.push_back(&_fireBox);
	_item4.setDetails(15, 350, 0, 1, 2, 1);
	BF_GLOBALS._sceneItems.push_back(&_yacht);

	_item3.setDetails(7, 350, 23, 24, 25, 1);
	_item2.setDetails(Rect(0, 0, SCREEN_WIDTH - 1, 31), 350, 3, 4, 5, 1, NULL);
	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH - 1, UI_INTERFACE_Y), 350, 0, 1, 2, 1, NULL);

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 370:
		BF_GLOBALS._player.enableControl();
		T2_GLOBALS._uiElements._active = true;
		T2_GLOBALS._uiElements.show();
		// Deliberate fall-through
	case 355:
		if (BF_GLOBALS.getFlag(onDuty) && BF_GLOBALS.getFlag(gunDrawn))
			BF_GLOBALS._player.setVisage(351);

		BF_GLOBALS._player.setPosition(Common::Point(22, 91));
		BF_GLOBALS._player.changeAngle(225);
		break;
	default:
		BF_GLOBALS.clearFlag(gunDrawn);

		if ((BF_GLOBALS._dayNumber == 1) && (BF_GLOBALS._bookmark < bCalledToDomesticViolence) &&
				BF_GLOBALS.getFlag(fBackupIn350)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager1, this, 3509, &_harrison, NULL);
			BF_GLOBALS._player.setAction(&_sequenceManager2, NULL, 3508, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager1, this, 3508, &BF_GLOBALS._player, NULL);
		}
		break;
	}
}

void Scene350::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	SceneExt::remove();
}

void Scene350::signal() {
	switch (_sceneMode) {
	case 1:
		BF_GLOBALS._sceneManager.changeScene(355);
		break;
	case 2:
		BF_GLOBALS._sceneManager.changeScene((BF_GLOBALS._dayNumber != 1) ||
			(BF_GLOBALS._bookmark < bStartOfGame) || (BF_GLOBALS._bookmark >= bCalledToDomesticViolence) ? 342 : 340);
		break;
	case 3:
		BF_GLOBALS._player.setStrip(8);

		_fireBoxInset.postInit();
		_fireBoxInset.setVisage(350);
		_fireBoxInset.setStrip(4);
		_fireBoxInset.fixPriority(200);
		_fireBoxInset.setPosition(Common::Point(85, 166));
		BF_GLOBALS._sceneItems.push_front(&_fireBoxInset);

		if (BF_INVENTORY.getObjectScene(INV_HOOK) == 350) {
			_hook.postInit();
			_hook.setVisage(350);
			_hook.setStrip(5);
			_hook.fixPriority(201);
			_hook.setPosition(Common::Point(106, 146));
			BF_GLOBALS._sceneItems.push_front(&_hook);
		}

		BF_GLOBALS._player.enableControl();
		_sound2.play(97);
		break;
	default:
		if (BF_GLOBALS.getFlag(fBackupIn350)) {
			_harrison.updateAngle(BF_GLOBALS._player._position);
			BF_GLOBALS._walkRegions.disableRegion(19);
		}

		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene350::process(Event &event) {
	// Check for gun being clicked on player
	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == INV_COLT45) &&
			BF_GLOBALS._player.contains(event.mousePos)) {
		BF_GLOBALS._player.disableControl();

		if (!BF_GLOBALS.getFlag(onDuty)) {
			// Player not on duty
			SceneItem::display2(350, 26);
			signal();
		} else if (BF_GLOBALS.getFlag(gunDrawn)) {
			// Holster the gun
			BF_GLOBALS.clearFlag(gunDrawn);
			_sceneMode = 0;
			setAction(&_sequenceManager1, this, 3501, &BF_GLOBALS._player, NULL);

			if (BF_GLOBALS.getFlag(fBackupIn350))
				_harrison.setAction(&_sequenceManager3, NULL, 3503, &_harrison, NULL);
		} else {
			// Drawn the gun
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager1, this, 3500, &BF_GLOBALS._player, NULL);

			if (BF_GLOBALS.getFlag(fBackupIn350))
				_harrison.setAction(&_sequenceManager3, NULL, 3502, &_harrison, NULL);

			BF_GLOBALS.setFlag(gunDrawn);
		}

		event.handled = true;
	}

	if (!event.handled) {
		SceneExt::process(event);

		if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1))) {
			// Check if the cursor is on an exit
			if (_swExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_SW);
				BF_GLOBALS._events.setCursor(surface);
			} else {
				// In case an exit cursor was being shown, restore the previously selected cursor
				CursorType cursorId = BF_GLOBALS._events.getCursor();
				BF_GLOBALS._events.setCursor(cursorId);
			}
		}
	}
}

void Scene350::checkGun() {
	if ((BF_GLOBALS._dayNumber != 1) || (BF_GLOBALS._bookmark < bStartOfGame) ||
			(BF_GLOBALS._bookmark >= bCalledToDomesticViolence) || BF_GLOBALS.getFlag(fRandomShot350)) {
		SceneItem::display2(350, 27);
	} else {
		BF_GLOBALS.setFlag(fRandomShot350);
		if (BF_GLOBALS.getFlag(fBackupIn350)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager1, this, 3511, &BF_GLOBALS._player, NULL);
		} else {
			_stripManager.start(3502, this);
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 355 - Future Wave Exterior
 *
 *--------------------------------------------------------------------------*/

void Scene355::Doorway::synchronize(Serializer &s) {
	NamedObject::synchronize(s);
	s.syncAsSint16LE(_v1);
	s.syncAsSint16LE(_v2);
	s.syncAsSint16LE(_v3);
}

bool Scene355::Doorway::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(355, 9);
		return true;
	case CURSOR_USE:
		if (!scene->_modeFlag) {
			scene->_sceneMode = 9984;
			scene->signal();
		} else {
			scene->setMode(true, 9984);
		}
		return true;
	case CURSOR_TALK:
		if (BF_GLOBALS._dayNumber >= 5) {
			switch (_v2) {
			case 0:
				++_v2;
				BF_GLOBALS._sound1.play(109);
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 0;

				BF_GLOBALS.setFlag(fTookTrailerAmmo);
				scene->_stripManager.start(3575, scene);
				scene->_lyle._flag = 1;
				return true;
			case 1:
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 0;
				scene->_stripManager.start(3573, scene);
				return true;
			default:
				break;
			}
		} else if (!BF_GLOBALS.getFlag(greenTaken) && (BF_GLOBALS._dayNumber == 1)) {
			scene->_sceneMode = 1357;
			BF_GLOBALS._player.disableControl();
			scene->_stripManager.start(3550, scene);
			return true;
		}
		break;
	case INV_WAVE_KEYS:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 3562;
		scene->setAction(&scene->_sequenceManager, scene, 3562, &BF_GLOBALS._player, NULL);
		_v3 = !_v3 ? 1 : 0;
		return true;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}

bool Scene355::Locker::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(355, 51);
		return true;
	case CURSOR_USE:
		if (BF_GLOBALS._dayNumber < 5)
			SceneItem::display2(355, 46);
		else if (BF_INVENTORY.getObjectScene(INV_FLARE) != 355)
			SceneItem::display2(355, 45);
		else if (scene->_modeFlag)
			scene->setMode(true, 9996);
		else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9996;

			if (scene->_nextSceneMode) {
				scene->_nextSceneMode = 0;
				scene->setAction(&scene->_sequenceManager, scene, 3555, &BF_GLOBALS._player, NULL);
			} else {
				scene->signal();
			}
		}
		return true;
	case INV_SCREWDRIVER:
		if (scene->_modeFlag)
			scene->setMode(true, 9996);
		else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9996;
			if (!scene->_nextSceneMode)
				scene->signal();
			else {
				scene->_nextSceneMode = 0;
				scene->setAction(&scene->_sequenceManager, scene, 3555, &BF_GLOBALS._player, NULL);
			}
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene355::LockerInset::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (_frame == 1)
			SceneItem::display2(355, 46);
		else
			SceneItem::display2(355, BF_GLOBALS._sceneObjects->contains(&scene->_object5) ? 26 : 47);
		return true;
	case CURSOR_USE:
		if (_frame == 1) {
			SceneItem::display2(355, 23);
			return true;
		} else
			return NamedObject::startAction(action, event);
	case INV_SCREWDRIVER:
		scene->_sound2.play(104);
		BF_INVENTORY.setObjectScene(INV_SCREWDRIVER, 999);
		setFrame(2);

		scene->_object9.postInit();
		scene->_object9.setVisage(378);
		scene->_object9.setPosition(Common::Point(83, 100));
		scene->_object9.fixPriority(100);

		scene->_object5.postInit();
		scene->_object5.setVisage(2356);
		scene->_object5.setStrip(3);
		scene->_object5.setPosition(Common::Point(67, 85));
		scene->_object5.fixPriority(255);
		BF_GLOBALS._sceneItems.push_front(&scene->_object5);

		SceneItem::display2(355, 27);
		scene->_locker.setFrame(2);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene355::Object5::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(355, 30);
		return true;
	case CURSOR_USE:
		BF_INVENTORY.setObjectScene(INV_FLARE, 1);
		T2_GLOBALS._uiElements.addScore(30);

		scene->_object9.remove();
		remove();
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene355::Green::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(355, 34);
		return true;
	case CURSOR_USE:
		if (scene->_modeFlag)
			SceneItem::display2(355, 35);
		else if (!_flag)
			SceneItem::display2(355, 38);
		else if (BF_INVENTORY.getObjectScene(INV_GRENADES) == 1)
			SceneItem::display2(355, 49);
		else {
			BF_GLOBALS._player._regionBitList |= 0x10;
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9985;
			scene->setAction(&scene->_sequenceManager, scene, 3557, &BF_GLOBALS._player, this, NULL);
		}
		return true;
	case CURSOR_TALK:
		scene->_sceneMode = 0;
		if (BF_INVENTORY.getObjectScene(INV_GRENADES) != 355) {
			scene->_stripManager.start(3584, scene);
		} else if (BF_INVENTORY.getObjectScene(INV_HANDCUFFS) != 1) {
			return false;
		} else {
			switch (BF_GLOBALS._greenDay5TalkCtr++) {
			case 0:
				scene->_stripManager.start(3565, scene);
				break;
			case 1:
				scene->_stripManager.start(3567, scene);
				break;
			default:
				scene->_stripManager.start(3571, scene);
				break;
			}
		}
		return true;

	default:
		if ((action < BF_LAST_INVENT) && scene->_modeFlag) {
			SceneItem::display2(355, 35);
			return true;
		}

		switch (action) {
		case INV_COLT45:
			if (BF_INVENTORY.getObjectScene(INV_HANDCUFFS) == 355)
				SceneItem::display2(355, 39);
			else if (!BF_GLOBALS.getFlag(fGunLoaded))
				SceneItem::display2(1, 1);
			else if (!BF_GLOBALS.getFlag(gunDrawn))
				SceneItem::display2(1, 0);
			else {
				if (BF_GLOBALS._sceneObjects->contains(&scene->_lyle))
					scene->_lyle.setAction(NULL);

				BF_GLOBALS._player.disableControl();
				scene->_green.setStrip(1);
				scene->_green.setFrame(1);
				scene->_sceneMode = 9981;
				scene->signal();
			}
			return true;
		case INV_HANDCUFFS:
			if (BF_GLOBALS._greenDay5TalkCtr <= 1)
				SceneItem::display2(355, 38);
			else {
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 9979;
				scene->setAction(&scene->_sequenceManager, scene, 4551, &BF_GLOBALS._player, this, NULL);
				BF_INVENTORY.setObjectScene(INV_HANDCUFFS, 355);
				T2_GLOBALS._uiElements.addScore(50);
				_flag = 1;
				BF_GLOBALS._bookmark = bInvestigateBoat;
			}
			return true;
		default:
			break;
		}

		return NamedObject::startAction(action, event);
	}
}

bool Scene355::Lyle::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 0;

		if (BF_GLOBALS._sceneObjects->contains(&scene->_green)) {
			scene->_stripManager.start((BF_INVENTORY.getObjectScene(INV_HANDCUFFS) == 355) ? 3578 : 3577, scene);
		} else {
			switch (_flag) {
			case 0:
				scene->_stripManager.start(3574, scene);
				break;
			case 1:
				scene->_stripManager.start(3576, scene);
				break;
			case 2:
				scene->_stripManager.start(3563, scene);
				break;
			default:
				break;
			}
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene355::Object8::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(355, 0);
		return true;
	case CURSOR_USE:
		if (BF_GLOBALS.getFlag(fBackupIn350)) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9999;
			scene->_stripManager.start(3559, scene);
			return true;
		} else if (BF_GLOBALS._dayNumber < 5) {
			SceneItem::display2(355, 52);
			return true;
		}
		break;
	case INV_COLT45:
		if (BF_GLOBALS.getFlag(fBackupIn350)) {
			scene->_sceneMode = 9997;
			scene->_stripManager.start(3561, scene);
		} else {
			SceneItem::display2(1, 4);
		}
		return true;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

bool Scene355::Item1::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(355, 28);
		return true;
	case CURSOR_USE:
		SceneItem::display2(355, 37);
		return true;
	case INV_RENTAL_KEYS:
		if (scene->_modeFlag) {
			scene->_sceneMode = 9980;
			scene->signal();
		} else if (!scene->_nextSceneMode)
			SceneItem::display2(355, 36);
		else
			scene->setMode(false, 9980);
		return true;
	default:
		return SceneHotspot::startAction(action, event);
	}
}

bool Scene355::Item2::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	if (BF_GLOBALS.getFlag(fBackupIn350)) {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 9999;
		scene->_stripManager.start(3559, scene);
	} else {
		if (scene->_action) {
			scene->_sceneMode = 0;
			scene->_action->remove();
		}

		BF_GLOBALS._sceneManager.changeScene(350);
	}
	return true;
}

bool Scene355::Item3::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(355, 33);
		return true;
	case CURSOR_USE:
		if (BF_GLOBALS._dayNumber < 5)
			SceneItem::display2(355, 21);
		else if (scene->_modeFlag)
			scene->setMode(true, 9987);
		else {
			scene->_sceneMode = 9987;
			scene->signal();
		}
		return true;
	case INV_FLARE:
		scene->_sound2.play(105);
		_state = 3554;
		break;
	case INV_RAGS:
		_state = 3559;
		break;
	case INV_JAR:
		_state = 3558;
		break;
	default:
		return SceneHotspotExt::startAction(action, event);
	}

	// Handling for inventory objects
	BF_INVENTORY.setObjectScene(action, 0);
	if (scene->_modeFlag)
		scene->setMode(true, 9986);
	else {
		scene->_sceneMode = 9986;
		scene->signal();
	}

	scene->_nextSceneMode = 0;
	return true;
}

bool Scene355::Item4::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(355, 31);
		return true;
	case CURSOR_USE:
		if ((BF_GLOBALS._dayNumber < 5) || (BF_INVENTORY.getObjectScene(INV_GRENADES) != 355))
			SceneItem::display2(355, 21);
		else if (scene->_modeFlag)
			scene->setMode(true, 9987);
		else {
			scene->_sceneMode = 9987;
			scene->signal();
		}
		return true;
	default:
		return SceneHotspot::startAction(action, event);
	}
}


bool Scene355::Pouch::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (BF_INVENTORY.getObjectScene(INV_SCREWDRIVER) == 355) {
			SceneItem::display2(355, 29);
			return true;
		}
		break;
	case CURSOR_USE:
		if (BF_INVENTORY.getObjectScene(INV_SCREWDRIVER) == 355) {
			if (scene->_modeFlag) {
				scene->_sceneMode = 9992;
				scene->signal();
			} else if (!scene->_nextSceneMode) {
				scene->setMode(false, 9992);
			} else {
				scene->_sceneMode = 9977;
				scene->_nextSceneMode = 0;
				scene->setAction(&scene->_sequenceManager, scene, 3555, &BF_GLOBALS._player, NULL);
			}
		} else {
			SceneItem::display2(355, 45);
		}
		return true;
	case INV_RENTAL_KEYS:
		if (scene->_modeFlag) {
			scene->_sceneMode = 9980;
			scene->signal();
		} else if (!scene->_nextSceneMode) {
			scene->setMode(false, 9980);
		} else {
			SceneItem::display2(355, 36);
		}
		return true;
	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

bool Scene355::Item11::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_WALK:
		return (BF_GLOBALS._bookmark == bStartOfGame) || (BF_GLOBALS._bookmark == bCalledToDomesticViolence) ||
			(BF_GLOBALS._bookmark == bArrestedGreen);
	case CURSOR_LOOK:
		SceneItem::display2(355, 7);
		return true;
	case CURSOR_USE:
		SceneItem::display2(355, 8);
		return true;
	case CURSOR_TALK:
		if (BF_GLOBALS._dayNumber == 5) {
			switch (scene->_doorway._v2) {
			case 0:
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 0;
				BF_GLOBALS.setFlag(fTookTrailerAmmo);
				scene->_stripManager.start(3575, scene);
				scene->_lyle._flag = 1;
				scene->_doorway._v2 = 1;
				break;
			case 1:
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 0;
				scene->_stripManager.start(3573, scene);
				break;
			default:
				return false;
			}
		} else {
			if (!BF_GLOBALS.getFlag(onDuty))
				return false;
			scene->_sceneMode = 1357;
			BF_GLOBALS._player.disableControl();
			scene->_stripManager.start(3550, scene);
		}
		return true;
	case INV_COLT45:
		if (!BF_GLOBALS.getFlag(fBackupIn350) || !BF_GLOBALS.getFlag(gunDrawn) || !BF_GLOBALS.getFlag(fGunLoaded))
			SceneItem::display2(1, 4);
		else {
			scene->_sceneMode = 9997;
			BF_GLOBALS._player.disableControl();
			scene->_stripManager.start(3561, scene);
		}
		return true;
	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

bool Scene355::RentalExit::startAction(CursorType action, Event &event) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	if (!BF_GLOBALS.getFlag(gunDrawn)) {
		if (scene->_modeFlag)
			scene->setMode(true, 0);
		else if (!scene->_nextSceneMode)
			scene->setMode(false, 0);
		else {
			scene->_nextSceneMode = 0;
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9991;
			scene->setAction(&scene->_sequenceManager, scene, 3555, &BF_GLOBALS._player, NULL);
		}
	}

	return true;
}

/*--------------------------------------------------------------------------*/

void Scene355::Action1::signal() {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(3600 * (BF_GLOBALS._randomSource.getRandomNumber(1) + 1));
		break;
	case 1:
		_actionIndex = 0;
		scene->_lyle.animate(ANIM_MODE_8, 1, this);
		break;
	default:
		break;
	}
}

void Scene355::Action2::signal() {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_stripManager.start((BF_INVENTORY.getObjectScene(INV_HANDCUFFS) == 1) ? 3566 : 3568, this);
		break;
	case 2:
		scene->_sceneMode = 9979;
		scene->signal();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene355::Scene355(): PalettedScene() {
	_nextSceneMode = 0;
	_modeFlag = false;
}

void Scene355::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_nextSceneMode);
	s.syncAsSint16LE(_modeFlag);
}

void Scene355::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 5;

	if (BF_GLOBALS._dayNumber == 5) {
		loadScene(356);
		BF_GLOBALS._player._regionBitList &= ~0x10;
	} else {
		loadScene(355);
	}

	_sound1.fadeSound(35);
	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_lyleHatSpeaker);
	_stripManager.addSpeaker(&_harrisonSpeaker);
	_stripManager.addSpeaker(&_greenSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	if (!BF_GLOBALS.getFlag(onDuty)) {
		BF_GLOBALS._player.setVisage(368);
		BF_GLOBALS._player.setStrip(3);
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	} else if (BF_GLOBALS.getFlag(gunDrawn)) {
		BF_GLOBALS._player.setVisage(356);
		BF_GLOBALS._player.setFrame(BF_GLOBALS._player.getFrameCount());
	} else {
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player.setVisage(356);
		BF_GLOBALS._player.setStrip(7);
	}

	if (BF_GLOBALS.getFlag(greenTaken)) {
		BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 361 : 368);
		BF_GLOBALS._player.setStrip(3);
		BF_GLOBALS._player.setPosition(Common::Point(244, 140));
	} else {
		BF_GLOBALS._player.setPosition(Common::Point(244, 132));
	}

	BF_GLOBALS._player._moveDiff = Common::Point(6, 4);
	BF_GLOBALS._player.enableControl();
	BF_GLOBALS._player._canWalk = false;

	_item4._sceneRegionId = 17;
	BF_GLOBALS._sceneItems.push_back(&_item4);

	_locker.postInit();
	_locker.setVisage(2356);
	_locker.setPosition(Common::Point(88, 99));

	if (BF_INVENTORY.getObjectScene(INV_SCREWDRIVER) == 999) {
		_locker.setFrame(2);

		if (BF_INVENTORY.getObjectScene(INV_FLARE) == 355) {
			_object9.postInit();
			_object9.setVisage(378);
			_object9.setPosition(Common::Point(83, 100));
			_object9.fixPriority(100);
		}
	}
	BF_GLOBALS._sceneItems.push_back(&_locker);

	_doorway.postInit();
	_doorway.setVisage(355);
	_doorway.setPosition(Common::Point(193, 105));
	_doorway.fixPriority(18);
	_doorway._v1 = 0;
	_doorway._v3 = 0;
	BF_GLOBALS._sceneItems.push_back(&_doorway);

	switch (BF_GLOBALS._dayNumber) {
	case 1:
		if (!BF_GLOBALS.getFlag(onDuty))
			_doorway._v3 = 1;
		else if (BF_INVENTORY.getObjectScene(INV_GREENS_GUN)  == 320)
			_doorway._v3 = 1;
		break;
	case 2:
	case 3:
	case 4:
		_doorway._v3 = 1;
		break;
	default:
		break;
	}

	if (BF_GLOBALS._dayNumber == 5)
		_doorway._v2 = BF_GLOBALS.getFlag(fTookTrailerAmmo) ? 1 : 0;

	_object8.postInit();
	_object8.setVisage(355);
	if (BF_GLOBALS._dayNumber == 5) {
		_object8.setStrip(3);
		_object8.setPosition(Common::Point(103, 148));
		_object8.hide();
	} else {
		_object8.setStrip(2);
		_object8.setPosition(Common::Point(142, 151));
		_object8.fixPriority(247);
	}

	switch (BF_GLOBALS._dayNumber) {
	case 2:
	case 3:
	case 4:
		break;
	case 5:
		BF_GLOBALS._player.enableControl();
		_modeFlag = true;
		BF_GLOBALS._player.setPosition(Common::Point(133, 173));
		BF_GLOBALS._player.fixPriority(249);
		BF_GLOBALS._player.setStrip(7);

		if (BF_GLOBALS._bookmark == bDoneAtLyles)
			BF_GLOBALS._bookmark = bEndDayFour;

		_pouch.setDetails(Rect(22, 136, 46, 146), 355, 43, -1, -1, 1, NULL);
		_nextSceneMode = 0;

		if (!BF_GLOBALS.getFlag(fLyleOnIsland)) {
			_lyle.postInit();
			_lyle.setVisage(847);
			_lyle.setPosition(Common::Point(296, 97));
			_lyle.setStrip(1);
			_lyle.setAction(&_action1);
			_lyle._flag = BF_GLOBALS.getFlag(fTookTrailerAmmo) ? 1 : 0;
			_lyle.setDetails(355, 40, 42, 41, 1, (SceneItem *)NULL);
		}

		if ((BF_INVENTORY.getObjectScene(INV_RAGS) == 0) && (BF_INVENTORY.getObjectScene(INV_JAR) == 0) &&
				(BF_INVENTORY.getObjectScene(INV_FLARE) == 0)) {
			BF_GLOBALS._sound1.changeSound(103);

			_object9.postInit();
			_object9.setVisage(2357);
			_object9.setStrip(2);
			_object9.setPosition(Common::Point(231, 19));
			_object9.animate(ANIM_MODE_2);

			_object11.postInit();
			_object11.setVisage(2357);
			_object11.setStrip(6);
			_object11.setPosition(Common::Point(183, 39));
			_object11.animate(ANIM_MODE_2);

			_doorway.setPosition(Common::Point(146, 107));
			_doorway._v3 = 0;
			_doorway._v2 = 2;
			_lyle._flag = 2;

			_green.postInit();
			BF_GLOBALS._sceneItems.push_back(&_green);

			if (BF_INVENTORY.getObjectScene(INV_HANDCUFFS) == 1) {
				_green.setVisage(376);
				_green.setStrip(1);
				_green.setPosition(Common::Point(193, 88));
				_green._flag = 0;
			} else {
				_green._flag = 1;

				if (BF_INVENTORY.getObjectScene(INV_GRENADES) == 1) {
					_green.setVisage(373);
					_green.setStrip(5);
					_green.setPosition(Common::Point(238, 142));
				} else {
					_green.setVisage(375);
					_green.setStrip(1);
					_green.setFrame(_green.getFrameCount());
					_green.setPosition(Common::Point(193, 147));
				}
			}

			if ((BF_GLOBALS._bookmark == bFinishedWGreen) && BF_GLOBALS._sceneObjects->contains(&_lyle) &&
					!BF_GLOBALS.getFlag(iWasAmbushed)) {
				BF_GLOBALS.setFlag(iWasAmbushed);
				BF_GLOBALS._player.disableControl();

				_sceneMode = 0;
				_stripManager.start(3582, this);
			}
		}
		break;
	default:
		if (!BF_GLOBALS.getFlag(greenTaken)) {
			_harrison.postInit();
			_harrison.setPosition(Common::Point(152, 131));
			_harrison.animate(ANIM_MODE_1, NULL);
			_harrison.setObjectWrapper(new SceneObjectWrapper());

			if (BF_GLOBALS.getFlag(gunDrawn)) {
				_harrison.setVisage(357);
				_harrison.setStrip(2);
			} else {
				_harrison.setVisage(1363);
				_harrison.setStrip(3);
			}

			_harrison.hide();
			if (BF_GLOBALS.getFlag(fBackupIn350)) {
				_harrison.show();
				BF_GLOBALS._sceneItems.push_back(&_harrison);
			}

			_sceneMode = 1355;
			setAction(&_sequenceManager, this, 1355, NULL);
		}
		break;
	}

	_item3._sceneRegionId = 18;
	_harrison.setDetails(355, 18, 20, 19, 1, (SceneItem *)NULL);
	_item6.setDetails(10, 355, 2, -1, 14, 1);
	_item7.setDetails(11, 355, 3, -1, 15, 1);
	_item8.setDetails(12, 355, 4, -1, 8, 1);
	_item9.setDetails(13, 355, 5, -1, -1, 1);
	_item10.setDetails(15, 355, 10, -1, 11, 1);
	_item1._sceneRegionId = 22;
	BF_GLOBALS._sceneItems.addItems(&_item3, &_item6, &_item7, &_item8, &_item9, &_item10, &_item1, NULL);

	if (BF_GLOBALS._dayNumber == 5) {
		_rentalExit.setBounds(Rect(115, 101, 186, 154));
		BF_GLOBALS._sceneItems.push_front(&_rentalExit);
	} else {
		_item2.setDetails(Rect(273, 53, 320, 101), 355, -1, -1, -1, 2, NULL);
	}

	_item11.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	BF_GLOBALS._sceneItems.push_back(&_item11);
}

void Scene355::signal() {
	static uint32 black = 0;

	switch (_sceneMode) {
	case 12:
		SceneItem::display2(355, 22);
		BF_GLOBALS._player.enableControl();
		break;
	case 1355:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 9999;
		_stripManager.start(BF_GLOBALS.getFlag(fBackupIn350) ? 3559 : 3554, this);
		break;
	case 1356:
		switch (_doorway._v1) {
		case 0:
			++_doorway._v1;
			_sceneMode = 9999;
			_stripManager.start(3550, this);
			break;
		case 1:
			_sceneMode = 9999;
			_stripManager.start(3551, this);
			++_doorway._v1;
			break;
		default:
			break;
		}
		break;
	case 2357:
		_harrison.animate(ANIM_MODE_1, NULL);
		// Deliberate fall-through
	case 1357:
		BF_GLOBALS._player.enableControl();
		if (BF_GLOBALS.getFlag(gunDrawn))
			BF_GLOBALS._player._canWalk = false;
		break;
	case 1359:
		BF_GLOBALS._sceneManager.changeScene(360);
		break;
	case 2358:
		if (!BF_GLOBALS.getFlag(fBackupIn350))
			BF_GLOBALS._sceneManager.changeScene(360);
		else {
			_sceneMode = 1359;
			setAction(&_sequenceManager, this, BF_GLOBALS.getFlag(gunDrawn) ? 1359 : 3550, &_harrison, NULL);
		}
		break;
	case 3553:
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player._canWalk = false;
		break;
	case 3554:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 9988;
		setAction(&_sequenceManager, this, 3554, &BF_GLOBALS._player, NULL);
		break;
	case 3556:
		if (BF_GLOBALS._player._angle == 45) {
			_sceneMode = 4554;
			signal();
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 4554;
			setAction(&_sequenceManager, this, 3556, &BF_GLOBALS._player, NULL);
		}
		break;
	case 3558:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 9988;
		setAction(&_sequenceManager, this, 3558, &BF_GLOBALS._player, NULL);
		break;
	case 3559:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 9988;
		setAction(&_sequenceManager, this, 3559, &BF_GLOBALS._player, NULL);
		break;
	case 3561:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 1357;
		setAction(&_sequenceManager, this, 3561, &BF_GLOBALS._player, NULL);
		break;
	case 3562:
		if (!BF_GLOBALS.getFlag(unlockBoat)) {
			BF_GLOBALS.setFlag(unlockBoat);
			T2_GLOBALS._uiElements.addScore(10);
		}

		SceneItem::display2(355, !_doorway._v3 ? 24 : 25);
		BF_GLOBALS._player.enableControl();
		break;
	case 4550:
		T2_GLOBALS._uiElements.addScore(50);
		_object10.remove();
		BF_GLOBALS._sound1.play(90);
		BF_GLOBALS._player._regionBitList |= 0x10;

		_doorway._v3 = 0;
		_doorway._v2 = 2;
		_lyle._flag = 2;
		BF_GLOBALS._player.enableControl();
		break;
	case 4552:
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 4554:
		BF_GLOBALS._player.disableControl();
		switch (_nextSceneMode) {
		case 9980:
			_sceneMode = 9980;
			break;
		case 9992:
			_sceneMode = 9992;
			break;
		default:
			_sceneMode = 0;
			break;
		}
		setAction(&_sequenceManager, this, 4554, &BF_GLOBALS._player, NULL);
		_nextSceneMode = 0;
		_modeFlag = true;
		break;
	case 9977:
		setMode(false, 9992);
		break;
	case 9979:
		_sceneMode = 0;
		signal();
		break;
	case 9980:
		_sceneMode = 9982;
		ADD_MOVER(BF_GLOBALS._player, 64, 173);
		break;
	case 9981:
		_sceneMode = 9994;
		_green.animate(ANIM_MODE_5, NULL);
		addFader((const byte *)&black, 10, this);
		break;
	case 9982:
		_sceneMode = 9983;
		if (BF_INVENTORY.getObjectScene(INV_HANDCUFFS) == 355) {
			if (BF_GLOBALS.getFlag(fLyleOnIsland)) {
				BF_GLOBALS._player.updateAngle(_green._position);
				_stripManager.start(3581, this);
			} else {
				BF_GLOBALS._player.updateAngle(_lyle._position);
				_stripManager.start(3570, this);
			}
		} else {
			if (BF_GLOBALS.getFlag(fLyleOnIsland)) {
				if (BF_GLOBALS._sceneObjects->contains(&_green)) {
					BF_INVENTORY.setObjectScene(INV_GRENADES, 860);
					_stripManager.start(3583, this);
				} else {
					signal();
				}
			} else {
				BF_GLOBALS._player.updateAngle(_lyle._position);
				_stripManager.start(BF_GLOBALS.getFlag(fTookTrailerAmmo) ? 3579 : 3580, this);
			}
		}
		break;
	case 9983:
		BF_GLOBALS._sceneManager.changeScene(860);
		break;
	case 9984:
		if (BF_GLOBALS._dayNumber == 5) {
			_sceneMode = 0;
			switch (_doorway._v2) {
			case 0:
				BF_GLOBALS._sound1.play(109);
				BF_GLOBALS.setFlag(fTookTrailerAmmo);
				_stripManager.start(3575, this);
				_lyle._flag = 1;
				++_doorway._v2;
				break;
			case 1:
				_stripManager.start(3573, this);
				break;
			default:
				SceneItem::display2(355, 21);
				BF_GLOBALS._player.enableControl();
				break;
			}
		} else if (BF_GLOBALS.getFlag(greenTaken) || (BF_GLOBALS._dayNumber > 1)) {
			if (_doorway._v3) {
				SceneItem::display2(355, 23);
				_sceneMode = 0;
				signal();
			} else {
				BF_GLOBALS._player.disableControl();
				_sceneMode = 2358;
				setAction(&_sequenceManager, this, 3551, &BF_GLOBALS._player, &_doorway, NULL);
			}
		} else if (BF_GLOBALS.getFlag(gunDrawn)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 1356;
			setAction(&_sequenceManager, this, 1358, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 1356;
			setAction(&_sequenceManager, this, 1356, &BF_GLOBALS._player, NULL);
		}
		break;
	case 9985:
		_sceneMode = 0;
		_stripManager.start(3569, this);
		BF_INVENTORY.setObjectScene(INV_GRENADES, 1);
		T2_GLOBALS._uiElements.addScore(50);
		BF_GLOBALS._player._regionBitList |= 0x10;
		break;
	case 9986:
		if (_nextSceneMode) {
			_sceneMode = _item3._state;
			signal();
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = _item3._state;
			setAction(&_sequenceManager, this, 3553, &BF_GLOBALS._player, NULL);
		}
		break;
	case 9987:
		if (_nextSceneMode) {
			_nextSceneMode = 0;
			BF_GLOBALS._player.disableControl();
			_sceneMode = 3555;
			setAction(&_sequenceManager, this, 3555, &BF_GLOBALS._player, NULL);
		} else {
			_nextSceneMode = 1;
			BF_GLOBALS._player.disableControl();
			_sceneMode = 3553;
			setAction(&_sequenceManager, this, 3553, &BF_GLOBALS._player, NULL);
		}
		break;
	case 9988:
		T2_GLOBALS._uiElements.addScore(30);
		if ((BF_INVENTORY.getObjectScene(INV_RAGS) == 0) && (BF_INVENTORY.getObjectScene(INV_JAR) == 0) &&
				(BF_INVENTORY.getObjectScene(INV_FLARE) == 0)) {
			_green.postInit();
			_green.setVisage(373);
			_green.setPosition(Common::Point(-10, -10));
			_green._flag = 0;

			_object9.postInit();
			_object9.setPosition(Common::Point(-10, -10));
			_object10.postInit();
			_object10.setPosition(Common::Point(-10, -10));
			_object11.postInit();
			_object11.setPosition(Common::Point(-10, -10));

			BF_GLOBALS._sceneItems.push_front(&_green);
			BF_GLOBALS.setFlag(fTookTrailerAmmo);
			BF_GLOBALS._sound1.fade(0, 5, 60, true, NULL);
			_sceneMode = 4550;

			setAction(&_sequenceManager, this, 4550, &_doorway, &_green, &_object9, &_object10, &_object11, NULL);
		} else {
			BF_GLOBALS._player.enableControl();
		}
		break;
	case 9989:
		_lockerInset.postInit();
		_lockerInset.setVisage(2356);
		_lockerInset.setStrip(2);
		BF_GLOBALS._sceneItems.push_front(&_lockerInset);

		if (BF_INVENTORY.getObjectScene(INV_SCREWDRIVER) == 999) {
			_lockerInset.setFrame(2);
			if (BF_INVENTORY.getObjectScene(INV_FLARE) == 355) {
				_object5.postInit();
				_object5.setVisage(2356);
				_object5.setStrip(3);
				_object5.setPosition(Common::Point(67, 85));
				_object5.fixPriority(255);
				BF_GLOBALS._sceneItems.push_front(&_object5);
			}
		} else {
			_lockerInset.setFrame(1);
		}

		_lockerInset.setPosition(Common::Point(82, 115));
		_lockerInset.fixPriority(254);
		BF_GLOBALS._player.enableControl();
		break;
	case 9990:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 3556;
		ADD_PLAYER_MOVER(145, 145);
		break;
	case 9991:
		setMode(false, 0);
		break;
	case 9992:
		_sceneMode = 9993;
		ADD_PLAYER_MOVER(59, 174);
		break;
	case 9993:
		BF_INVENTORY.setObjectScene(INV_SCREWDRIVER, 1);
		SceneItem::display2(355, 29);
		T2_GLOBALS._uiElements.addScore(30);
		_sceneMode = 0;
		signal();
		break;
	case 9996: {
		_sceneMode = 9989;
		ADD_PLAYER_MOVER(116, 146);
		break;
	}
	case 9997:
		_sceneMode = 9999;
		_doorway._v1 = 2;
		_stripManager.start(3562, this);
		break;
	case 9998:
		error("Talkdoor state");
		break;
	case 9999:
		if (_doorway._v1 != 2) {
			BF_GLOBALS._player.enableControl();
			BF_GLOBALS._player._canWalk = false;
		} else if (BF_GLOBALS.getFlag(gunDrawn)) {
			_sceneMode = 2358;
			setAction(&_sequenceManager, this, 2359, &BF_GLOBALS._player, &_doorway, &_harrison, NULL);
		} else {
			_sceneMode = 2358;
			setAction(&_sequenceManager, this, 2358, &BF_GLOBALS._player, &_doorway, &_harrison, NULL);
		}
		break;
	case 0:
	case 3555:
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene355::process(Event &event) {
	if (BF_GLOBALS._dayNumber != 5) {
		// Handling for earlier days
		if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1))) {
			// Check if the cursor is on an exit
			if (_rentalExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_NW);
				BF_GLOBALS._events.setCursor(surface);
			} else {
				// In case an exit cursor was being shown, restore the previously selected cursor
				CursorType cursorId = BF_GLOBALS._events.getCursor();
				BF_GLOBALS._events.setCursor(cursorId);
			}
		}

		if ((_sceneMode != 2357) && (_sceneMode != 1357) && !BF_GLOBALS.getFlag(greenTaken) &&
				(event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == INV_COLT45)) {
			if (BF_GLOBALS._player.contains(event.mousePos)) {
				BF_GLOBALS._player.disableControl();
				if (BF_GLOBALS.getFlag(gunDrawn)) {
					BF_GLOBALS.clearFlag(gunDrawn);
					_sceneMode = 2357;
					setAction(&_sequenceManager, this, 2357, &BF_GLOBALS._player, &_harrison, NULL);
				} else {
					BF_GLOBALS._player.disableControl();
					_sceneMode = 1357;
					setAction(&_sequenceManager, this, 1357, &BF_GLOBALS._player, &_harrison, NULL);
					BF_GLOBALS.setFlag(gunDrawn);
				}
			} else {
				_item11.startAction(INV_COLT45, event);
			}
			event.handled = true;
		}
	} else {
		// Day 5 handling
		if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1))) {
			// Check if the cursor is on the exit to the rental boat
			if (_rentalExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(!_modeFlag ? EXITFRAME_SW : EXITFRAME_NE);
				BF_GLOBALS._events.setCursor(surface);
			} else {
				// In case an exit cursor was being shown, restore the previously selected cursor
				CursorType cursorId = BF_GLOBALS._events.getCursor();
				BF_GLOBALS._events.setCursor(cursorId);
			}
		}

		// Special handling of actions
		if (event.eventType == EVENT_BUTTON_DOWN) {
			switch (BF_GLOBALS._events.getCursor()) {
			case INV_COLT45:
				if (BF_GLOBALS._player.contains(event.mousePos)) {
					BF_GLOBALS._player.addMover(NULL);

					if (BF_GLOBALS.getFlag(gunDrawn)) {
						BF_GLOBALS.clearFlag(gunDrawn);
						BF_GLOBALS._player.disableControl();
						_sceneMode = 1357;
						setAction(&_sequenceManager, this, 3560, &BF_GLOBALS._player, NULL);
					} else {
						BF_GLOBALS.setFlag(gunDrawn);
						if (_modeFlag) {
							setMode(true, 3561);
						} else {
							_sceneMode = 3561;
							signal();
						}
					}
				}
				break;
			case CURSOR_WALK:
				if (BF_GLOBALS.getFlag(gunDrawn))
					event.handled = true;
				break;
			case CURSOR_LOOK:
			case CURSOR_TALK:
				break;
			case CURSOR_USE:
				if (BF_GLOBALS.getFlag(gunDrawn)) {
					SceneItem::display2(355, 50);
					event.handled = true;
				}
				break;
			default:
				if (BF_GLOBALS.getFlag(gunDrawn)) {
					SceneItem::display2(355, 50);
					event.handled = true;
				}
				break;
			}
		}
	}

	PalettedScene::process(event);
}

void Scene355::dispatch() {
	PalettedScene::dispatch();
	if (BF_GLOBALS._sceneObjects->contains(&_lyle)) {
		_lyle.updateAngle(BF_GLOBALS._player._position);
	}

	if (!_action && (BF_GLOBALS._player.getRegionIndex() == 20)) {
		ADD_MOVER(BF_GLOBALS._player, 238, 142);
	}
}

void Scene355::setMode(bool mode, int sceneMode) {
	Scene355 *scene = (Scene355 *)BF_GLOBALS._sceneManager._scene;

	if (mode) {
		BF_GLOBALS._player.disableControl();
		_sceneMode = sceneMode;
		setAction(&scene->_sequenceManager, this, 4553, &BF_GLOBALS._player, NULL);
		_modeFlag = false;
	} else {
		_sceneMode = 9990;
		_nextSceneMode = sceneMode;
		signal();
	}
}

/*--------------------------------------------------------------------------
 * Scene 360 - Future Wave Interior
 *
 *--------------------------------------------------------------------------*/

bool Scene360::Item1::startAction(CursorType action, Event &event) {
	Scene360 *scene = (Scene360 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(360, 9);
		return true;
	case CURSOR_TALK:
		scene->_sceneMode = 3607;
		BF_GLOBALS._player.disableControl();
		scene->_stripManager.start(3550, scene);
		return true;
	case INV_COLT45:
		SceneItem::display2(1, 4);
		return true;
	default:
		return SceneHotspot::startAction(action, event);
	}
}

bool Scene360::Item2::startAction(CursorType action, Event &event) {
	Scene360 *scene = (Scene360 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(360, 10);
		return true;
	case CURSOR_USE:
		if (BF_GLOBALS._dayNumber != 4)
			SceneItem::display2(360, 5);
		else
			scene->setAction(&scene->_action1);
		return true;
	default:
		return SceneHotspot::startAction(action, event);
	}
}

bool Scene360::Item3::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(360, 11);
		return true;
	case CURSOR_USE:
		SceneItem::display2(360, 12);
		return true;
	default:
		return SceneHotspot::startAction(action, event);
	}
}

bool Scene360::Barometer::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(360, 15);
		return true;
	case CURSOR_USE:
		SceneItem::display2(360, 16);
		return true;
	default:
		return SceneHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene360::SlidingDoor::startAction(CursorType action, Event &event) {
	Scene360 *scene = (Scene360 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(360, 0);
		return true;
	case CURSOR_USE:
		BF_GLOBALS._player.disableControl();
		if (BF_GLOBALS.getFlag(greenTaken)) {
			scene->_sceneMode = 3611;
			setAction(&scene->_sequenceManager1, scene, 3611, &BF_GLOBALS._player, this, NULL);
		} else {
			scene->_sceneMode = 3604;
			setAction(&scene->_sequenceManager1, scene, 3604, &BF_GLOBALS._player, NULL);
		}
		return true;
	case CURSOR_TALK:
		scene->_sceneMode = 3607;
		BF_GLOBALS._player.disableControl();
		scene->_stripManager.start(3550, scene);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene360::Window::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(360, 13);
		return true;
	case CURSOR_USE:
		SceneItem::display2(360, 14);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene360::Object4::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(360, 3);
		return true;
	case CURSOR_USE:
		SceneItem::display2(360, 2);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene360::BaseballCards::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		if (event.mousePos.x >= (_bounds.left + _bounds.width() / 2))
			SceneItem::display2(360, 4);
		else
			SceneItem::display2(360, 22);
		return true;
	case CURSOR_USE:
		SceneItem::display2(360, 2);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene360::Harrison::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(360, 6);
		return true;
	case CURSOR_USE:
		SceneItem::display2(360, 7);
		return true;
	case CURSOR_TALK:
		SceneItem::display2(360, 8);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene360::Object7::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(360, 1);
		return true;
	case CURSOR_USE:
		SceneItem::display2(360, 21);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene360::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER(153, 115);
		break;
	case 1:
		BF_GLOBALS._player.setStrip(7);
		if (BF_INVENTORY.getObjectScene(INV_WAREHOUSE_KEYS) == 360) {
			SceneItem::display2(360, 20);
			BF_INVENTORY.setObjectScene(INV_WAREHOUSE_KEYS, 1);
			T2_GLOBALS._uiElements.addScore(30);
		} else {
			SceneItem::display2(360, 5);
		}

		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene360::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	if (s.getVersion() < 9) {
		int tmpVar = 0;
		s.syncAsSint16LE(tmpVar);
	}
}

void Scene360::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(760);
	setZoomPercents(108, 90, 135, 100);
	_sound1.play(125);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);
	_stripManager.addSpeaker(&_harrisonSpeaker);
	_stripManager.addSpeaker(&_greenSpeaker);

	_item2._sceneRegionId = 11;
	BF_GLOBALS._sceneItems.push_back(&_item2);

	_slidingDoor.postInit();
	_slidingDoor.setVisage(760);
	_slidingDoor.setPosition(Common::Point(42, 120));
	_slidingDoor.setStrip(2);
	_slidingDoor.fixPriority(85);
	BF_GLOBALS._sceneItems.push_back(&_slidingDoor);

	_window.postInit();
	_window.setVisage(760);
	_window.setStrip(4);
	_window.setPosition(Common::Point(176, 43));
	_window.fixPriority(10);
	_window._numFrames = 2;
	_window.animate(ANIM_MODE_8, 0, NULL);
	BF_GLOBALS._sceneItems.push_back(&_window);

	_object4.postInit();
	_object4.setVisage(760);
	_object4.setStrip(5);
	_object4.setPosition(Common::Point(157, 75));
	_object4.fixPriority(50);
	BF_GLOBALS._sceneItems.push_back(&_object4);

	if (BF_GLOBALS._dayNumber <= 1) {
		_object7.postInit();
		_object7.setVisage(760);
		_object7.setStrip(1);
		_object7.setPosition(Common::Point(246, 105));
		_object7.fixPriority(50);
		BF_GLOBALS._sceneItems.push_back(&_object7);

		_baseballCards.postInit();
		_baseballCards.setVisage(760);
		_baseballCards.setStrip(6);
		_baseballCards.setPosition(Common::Point(159, 115));
		_baseballCards.fixPriority(50);
		BF_GLOBALS._sceneItems.push_back(&_baseballCards);
	}

	BF_GLOBALS._player.postInit();
	if (BF_GLOBALS.getFlag(onDuty)) {
		if (BF_GLOBALS.getFlag(gunDrawn)) {
			BF_GLOBALS._player.setVisage(1351);
			BF_GLOBALS._player._moveDiff.x = 5;
		} else {
			BF_GLOBALS._player.setVisage(361);
			BF_GLOBALS._player._moveDiff.x = 6;
		}
	} else {
		BF_GLOBALS._player.setVisage(368);
		BF_GLOBALS._player._moveDiff.x = 6;
	}

	BF_GLOBALS._player.changeZoom(-1);
	BF_GLOBALS._player.setStrip(3);
	BF_GLOBALS._player.setPosition(Common::Point(340, 160));
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player._moveDiff.y = 4;
	BF_GLOBALS._player.enableControl();

	if ((BF_GLOBALS._sceneManager._previousScene == 355) || (BF_GLOBALS._sceneManager._previousScene != 370)) {
		// The original was using there a useless variable (now removed)
		BF_GLOBALS._player.setPosition(Common::Point(253, 135));
		BF_GLOBALS._player.setStrip(2);

		if (BF_GLOBALS.getFlag(fBackupIn350)) {
			_harrison.postInit();
			_harrison.setVisage(BF_GLOBALS.getFlag(gunDrawn) ? 363 : 1363);
			_harrison.animate(ANIM_MODE_1, NULL);
			_harrison.setObjectWrapper(new SceneObjectWrapper());
			_harrison.setPosition(Common::Point(235, 150));
			_harrison.setStrip(2);
			BF_GLOBALS._sceneItems.push_back(&_harrison);
		}

		_sceneMode = 3607;
		if (BF_GLOBALS.getFlag(greenTaken)) {
			_slidingDoor.setPosition(Common::Point(42, 120));
		} else {
			BF_GLOBALS._player.disableControl();

			_object2.postInit();
			_object2.setPosition(Common::Point(-40, -40));

			_slidingDoor.setPosition(Common::Point(6, 130));
			_slidingDoor.setAction(&_sequenceManager1, this, 3606, &_slidingDoor, &_object7, NULL);
		}
	} else {
		BF_GLOBALS._player.setPosition(Common::Point(62, 122));
		BF_GLOBALS._player.enableControl();
	}

	_barometer._sceneRegionId = 9;
	BF_GLOBALS._sceneItems.push_back(&_barometer);
	_item3._sceneRegionId = 10;
	BF_GLOBALS._sceneItems.push_back(&_item3);
	_item1.setBounds(Rect(0, 0, SCREEN_WIDTH, UI_INTERFACE_Y));
	BF_GLOBALS._sceneItems.push_back(&_item1);
}

void Scene360::signal() {
	switch (_sceneMode) {
	case 3600:
	case 3611:
		BF_GLOBALS._sceneManager.changeScene(370);
		break;
	case 3602:
		BF_GLOBALS.setFlag(gunDrawn);
		BF_GLOBALS._deathReason = BF_GLOBALS.getFlag(fBackupIn350) ? 2 : 1;
		BF_GLOBALS._player.setPosition(Common::Point(BF_GLOBALS._player._position.x - 20,
			BF_GLOBALS._player._position.y + 1));
		_sceneMode = 3610;
		setAction(&_sequenceManager1, this, 3610, &_slidingDoor, &_object2, &BF_GLOBALS._player, NULL);
		break;
	case 3603:
		_sceneMode = 3605;
		setAction(&_sequenceManager1, this, 3605, &BF_GLOBALS._player, &_slidingDoor, NULL);
		break;
	case 3604:
		if (BF_GLOBALS.getFlag(fBackupIn350)) {
			_sceneMode = 3603;
			setAction(&_sequenceManager1, this, _sceneMode, &_harrison, NULL);
		} else {
			_sceneMode = 3605;
			setAction(&_sequenceManager1, this, _sceneMode, &BF_GLOBALS._player, &_slidingDoor, NULL);
		}
		break;
	case 3605:
		if (BF_GLOBALS.getFlag(fBackupIn350)) {
			_sceneMode = 3600;
			setAction(&_sequenceManager1, this, 3600, NULL);
		} else {
			BF_GLOBALS._deathReason = BF_GLOBALS.getFlag(fBackupIn350) ? 2 : 1;
			_sceneMode = 3610;
			setAction(&_sequenceManager1, this, 3601, &BF_GLOBALS._player, NULL);
		}
		break;
	case 3607:
	case 3609:
		// Original game was only using at this place visage 1363.
		// This workaround allow Harrison to keep his gun handy
		// when entering the romm (if required)
		if (! BF_GLOBALS.getFlag(gunDrawn))
			_harrison.setVisage(1363);
		else
			_harrison.setVisage(363);
		BF_GLOBALS._player.enableControl();
		break;
	case 3608:
 		BF_GLOBALS._sceneManager.changeScene(355);
		break;
	case 3610:
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 9998:
		BF_GLOBALS._player.setVisage(1351);
		BF_GLOBALS._player._moveDiff.x = 5;
		BF_GLOBALS._player.setFrame(1);
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player.enableControl();
		break;
	case 9999:
		BF_GLOBALS._player.setVisage(361);
		BF_GLOBALS._player._moveDiff.x = 6;
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player.enableControl();
		break;
	default:
		break;
	}
}

void Scene360::process(Event &event) {
	SceneExt::process(event);

	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == INV_COLT45) &&
			BF_GLOBALS._player.contains(event.mousePos) && !BF_GLOBALS.getFlag(greenTaken)) {
		// Player clicked gun on themselves
		if (BF_GLOBALS.getFlag(gunDrawn)) {
			// Handle holstering gun
			if (BF_GLOBALS._player._position.x <= 160)
				SceneItem::display2(360, 18);
			else {
				if (BF_GLOBALS.getFlag(fBackupIn350))
					SceneItem::display2(360, 19);

				BF_GLOBALS.clearFlag(gunDrawn);
				_sceneMode = 9999;
				BF_GLOBALS._player.setVisage(1361);
				BF_GLOBALS._player.addMover(NULL);
				BF_GLOBALS._player.setFrame(BF_GLOBALS._player.getFrameCount());
				BF_GLOBALS._player.animate(ANIM_MODE_6, this);

				_harrison.setVisage(1363);
			}
		} else {
			// Handle drawing gun
			if (BF_GLOBALS.getFlag(fBackupIn350))
				SceneItem::display2(360, 19);

			BF_GLOBALS._player.disableControl();
			BF_GLOBALS._player.addMover(NULL);
			BF_GLOBALS._player.setVisage(1361);
			BF_GLOBALS._player.setFrame(1);
			BF_GLOBALS._player.animate(ANIM_MODE_5, this);

			BF_GLOBALS.setFlag(gunDrawn);
			_sceneMode = 9998;
			_harrison.setVisage(363);
		}

		event.handled = true;
	}
}

void Scene360::dispatch() {
	SceneExt::dispatch();

	if (!_action) {
		if (BF_GLOBALS._player.getRegionIndex() == 8) {
			// Leaving the boat
			BF_GLOBALS._player.disableControl();
			if (BF_GLOBALS.getFlag(fBackupIn350)) {
				BF_GLOBALS._player.addMover(NULL);
				_sceneMode = 3609;
				setAction(&_sequenceManager1, this, 3609, &BF_GLOBALS._player, NULL);
			} else {
				BF_GLOBALS._sceneManager.changeScene(355);
			}
		}

		if ((BF_GLOBALS._player._position.x <= 168) && !BF_GLOBALS.getFlag(greenTaken) &&
				!BF_GLOBALS.getFlag(gunDrawn)) {
			// Moving to doorway without drawn gun before Green is captured
			BF_GLOBALS._player.disableControl();
			BF_GLOBALS._player.addMover(NULL);
			_sceneMode = 3602;
			setAction(&_sequenceManager1, this, 3602, &_slidingDoor, &_object2, &BF_GLOBALS._player, NULL);
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 370 - Future Wave Bedroom
 *
 *--------------------------------------------------------------------------*/

bool Scene370::GreensGun::startAction(CursorType action, Event &event) {
	Scene370 *scene = (Scene370 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(370, 4);
		return true;
	case CURSOR_USE:
		if ((BF_INVENTORY.getObjectScene(INV_HANDCUFFS) != 1) || BF_GLOBALS.getFlag(greenTaken)) {
			BF_GLOBALS._player.disableControl();
			BF_GLOBALS._walkRegions.enableRegion(3);
			scene->_sceneMode = 3711;
			scene->setAction(&scene->_sequenceManager, scene, 3711, &BF_GLOBALS._player, this, NULL);
		} else {
			SceneItem::display2(370, 5);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene370::Green::startAction(CursorType action, Event &event) {
	Scene370 *scene = (Scene370 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(370, (_v2 < 3) ? 10 : 0);
		return true;
	case CURSOR_USE:
		if (_v2 != 3)
			SceneItem::display2(370, 1);
		else if (BF_INVENTORY.getObjectScene(INV_HANDCUFFS) == 1)
			SceneItem::display2(370, 26);
		else if (BF_INVENTORY.getObjectScene(INV_GREENS_KNIFE) == 1) {
			scene->_sceneMode = 2;
			scene->_stripManager.start(3717, scene);
		} else {
			BF_GLOBALS._player.disableControl();
			BF_GLOBALS._walkRegions.enableRegion(3);
			scene->_sceneMode = 3715;
			scene->setAction(&scene->_sequenceManager, scene, 3715, &BF_GLOBALS._player, this, NULL);
		}
		return true;
	case CURSOR_TALK:
		BF_GLOBALS._player.disableControl();
		switch (_v2) {
		case 0:
			++_v2;
			scene->_sceneMode = 3706;
			scene->setAction(&scene->_sequenceManager, scene, 3706, NULL);
			break;
		case 1:
			++_v2;
			scene->_sceneMode = 3707;

			scene->_object5.postInit();
			scene->_object5.setVisage(362);
			scene->_object5.setStrip(3);
			scene->_object5.setPosition(scene->_harrison._position);
			scene->_object5.hide();

			scene->setAction(&scene->_sequenceManager, scene, 3707, &scene->_harrison, &scene->_object5, NULL);
			break;
		case 2:
			++_v2;
			scene->_sceneMode = 3708;
			scene->setAction(&scene->_sequenceManager, scene, 3708, this, &scene->_laura, &scene->_harrison,
				&scene->_object5, &scene->_greensGun, NULL);
			break;
		case 3:
			scene->_sceneMode = BF_INVENTORY.getObjectScene(INV_HANDCUFFS) == 1 ? 3713 : 2;
			scene->_stripManager.start(3717, scene);
			break;
		default:
			break;
		}
		return true;
	case INV_COLT45:
		if ((BF_INVENTORY.getObjectScene(INV_GREENS_GUN) == 370) || (BF_INVENTORY.getObjectScene(INV_GREENS_GUN) == 1))
			SceneItem::display2(370, 28);
		else if (!BF_GLOBALS.getHasBullets())
			SceneItem::display2(1, 1);
		else {
			BF_GLOBALS._player.disableControl();
			BF_GLOBALS._deathReason = 9;
			scene->_sceneMode = 3702;
			scene->setAction(&scene->_sequenceManager, scene, 3702, &BF_GLOBALS._player, this, &scene->_harrison, NULL);
		}
		return true;
	case INV_HANDCUFFS:
		if (_v2 != 3)
			SceneItem::display2(370, 2);
		else {
			T2_GLOBALS._uiElements.addScore(50);
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 3704;
			scene->setAction(&scene->_sequenceManager, scene, 3704, &BF_GLOBALS._player, this, &scene->_harrison, NULL);
		}
		return true;
	case INV_MIRANDA_CARD:
		if (BF_INVENTORY.getObjectScene(INV_HANDCUFFS) == 1)
			SceneItem::display2(370, 5);
		else if (BF_GLOBALS.getFlag(readGreenRights))
			SceneItem::display2(390, 15);
		else {
			BF_GLOBALS.setFlag(readGreenRights);
			BF_GLOBALS._player.disableControl();
			BF_GLOBALS._player.updateAngle(this->_position);
			scene->_sceneMode = 3717;
			scene->setAction(&scene->_sequenceManager, scene, 3717, &BF_GLOBALS._player, NULL);
		}
		return true;
	default:
		break;
	}

	return NamedObject2::startAction(action, event);
}

bool Scene370::Harrison::startAction(CursorType action, Event &event) {
	Scene370 *scene = (Scene370 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(370, 8);
		return true;
	case CURSOR_TALK:
		if (scene->_green._v2 != 3) {
			scene->_sceneMode = 3;
			scene->_stripManager.start(3714, scene);
		} else if ((BF_INVENTORY.getObjectScene(INV_GREENS_KNIFE) == 1) ||
				((BF_INVENTORY.getObjectScene(INV_GREENS_GUN) == 1) && BF_GLOBALS._sceneObjects->contains(&scene->_laura))) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 3716;
			scene->setAction(&scene->_sequenceManager, scene, 3716, &BF_GLOBALS._player, &scene->_green, this, NULL);
		} else {
			BF_GLOBALS._player.updateAngle(this->_position);
			scene->_stripManager.start(3715, scene);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene370::Laura::startAction(CursorType action, Event &event) {
	Scene370 *scene = (Scene370 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(370, 11);
		return true;
	case CURSOR_USE:
		SceneItem::display2(370, BF_GLOBALS._sceneObjects->contains(&scene->_green) ? 12 : 13);
		return true;
	case CURSOR_TALK:
		if (BF_INVENTORY.getObjectScene(INV_HANDCUFFS) != 1) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 3705;
			scene->setAction(&scene->_sequenceManager, scene, 3705, &BF_GLOBALS._player, this, NULL);
			return true;
		}
		// Deliberate fall-through
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene370::Item1::startAction(CursorType action, Event &event) {
	Scene370 *scene = (Scene370 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(370, 15);
		return true;
	} else if (action == CURSOR_USE) {
		SceneItem::display2(370, 16);
		return true;
	} else if (action == INV_COLT45) {
		if (BF_GLOBALS._sceneObjects->contains(&scene->_green) && (BF_INVENTORY.getObjectScene(INV_GREENS_GUN) != 370)) {
			scene->_green.setAction(NULL);
			scene->_sceneMode = 3703;
			scene->setAction(&scene->_sequenceManager, scene, 3703, &BF_GLOBALS._player, &scene->_green, &scene->_harrison, NULL);
			return true;
		}
		return false;
	} else if (action < CURSOR_WALK) // If any other inventory item used
		return false;
	else // If any other action is used
		return NamedHotspot::startAction(action, event);
}

bool Scene370::Item6::startAction(CursorType action, Event &event) {
	Scene370 *scene = (Scene370 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_LOOK) {
		SceneItem::display2(370, 14);
		return true;
	} else if (action == CURSOR_USE) {
		SceneItem::display2(370, 29);
		return true;
	} else if (action == INV_COLT45) {
		if (BF_GLOBALS._sceneObjects->contains(&scene->_green) && (BF_INVENTORY.getObjectScene(INV_GREENS_GUN) != 370) &&
				(BF_INVENTORY.getObjectScene(INV_HANDCUFFS) == 1)) {
			BF_GLOBALS._player.disableControl();
			scene->_green.setAction(NULL);
			scene->_sceneMode = 3703;
			scene->setAction(&scene->_sequenceManager, scene, 3703, &BF_GLOBALS._player, &scene->_green, &scene->_harrison, NULL);
			return true;
		}
		return SceneHotspot::startAction(action, event);
	} else if (action < CURSOR_WALK) // If any other inventory item used
		return false;
	else // If any other action
		return SceneHotspot::startAction(action, event);
}


bool Scene370::Exit::startAction(CursorType action, Event &event) {
	Scene370 *scene = (Scene370 *)BF_GLOBALS._sceneManager._scene;

	if (BF_GLOBALS._sceneObjects->contains(&scene->_green))
		return false;
	else {
		ADD_PLAYER_MOVER(event.mousePos.x, event.mousePos.y);
		return true;
	}
}

/*--------------------------------------------------------------------------*/

void Scene370::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(370);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);
	_stripManager.addSpeaker(&_lauraSpeaker);
	_stripManager.addSpeaker(&_lauraHeldSpeaker);
	_stripManager.addSpeaker(&_greenSpeaker);
	_stripManager.addSpeaker(&_harrisonSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(1351);
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.setStrip(8);
	BF_GLOBALS._player.setPosition(Common::Point(310, 153));
	BF_GLOBALS._player._moveDiff = Common::Point(7, 2);
	BF_GLOBALS._player.disableControl();

	if (BF_GLOBALS.getFlag(greenTaken)) {
		// Green has already been arrested
		BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 361 : 368);
		BF_GLOBALS._player._moveDiff.x = 6;
		BF_GLOBALS._player.enableControl();
	} else {
		// Hostage scene setup
		_green.postInit();
		_green.setVisage(373);
		_green.setStrip(2);
		_green._numFrames = 5;
		_green.setPosition(Common::Point(164, 137));
		_green.animate(ANIM_MODE_7, 0, NULL);
		_green.fixPriority(125);
		BF_GLOBALS._sceneItems.push_back(&_green);

		_laura.postInit();
		_laura.setVisage(374);
		_laura.setStrip(2);
		_laura.setPosition(_green._position);
		_laura.hide();
		_laura.animate(ANIM_MODE_1, NULL);

		_harrison.postInit();
		_harrison.setVisage(372);
		_harrison.setPosition(Common::Point(256, 166));
		BF_GLOBALS._sceneItems.push_back(&_harrison);

		_sceneMode = 3700;
		setAction(&_sequenceManager, this, 3700, NULL);
	}

	if ((BF_INVENTORY.getObjectScene(INV_GREENS_GUN) != 320) && (BF_INVENTORY.getObjectScene(INV_GREENS_GUN) != 1)) {
		_greensGun.postInit();
		_greensGun.setVisage(362);
		_greensGun.setStrip(6);
		_greensGun.setPosition(_green._position);
		_greensGun.fixPriority(149);
		_greensGun.hide();
	}

	if ((BF_INVENTORY.getObjectScene(INV_GREENS_GUN) == 370) && (BF_GLOBALS._dayNumber == 1)) {
		_greensGun.setPosition(Common::Point(172, 137));
		_greensGun.show();
		BF_GLOBALS._sceneItems.push_front(&_greensGun);
	}

	_item1._sceneRegionId = 8;
	BF_GLOBALS._sceneItems.push_back(&_item1);
	_item2.setDetails(9, 370, 17, -1, 18, 1);
	_item3.setDetails(10, 370, 19, -1, 20, 1);
	_item4.setDetails(11, 370, 21, -1, 22, 1);
	_item5.setDetails(12, 370, 23, -1, 24, 1);
	_exit.setDetails(Rect(316, 133, 320, 166), 360, -1, -1, -1, 1, NULL);
	_item6.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

	BF_GLOBALS._sceneItems.push_back(&_item6);
	BF_GLOBALS._sceneItems.remove(&BF_GLOBALS._player);
	BF_GLOBALS._sceneItems.push_back(&BF_GLOBALS._player);
}

void Scene370::signal() {
	switch (_sceneMode) {
	case 1:
		BF_GLOBALS._bookmark = bArrestedGreen;
		BF_GLOBALS._sceneManager.changeScene(350);
		break;
	case 2:
	case 3717:
		BF_GLOBALS._player.enableControl();
		break;
	case 3:
		break;
	case 3707:
		_object5.setDetails(370, 6, -1, 7, 1, (SceneItem *)NULL);
		BF_GLOBALS._sceneItems.push_back(&_object5);
		// Deliberate fall-through
	case 3700:
	case 3706:
		_green.setAction(&_sequenceManager, this, 3701, NULL);
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player._canWalk = false;
		break;
	case 3701:
		BF_GLOBALS._player.disableControl();
		setAction(&_sequenceManager, this, 3710, NULL);
		break;
	case 3702:
		// Player tried to shoot Green
		BF_GLOBALS._deathReason = 2;
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 3703:
		// Player dropped his gun
		BF_GLOBALS._deathReason = 9;
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 3704:
		BF_GLOBALS.clearFlag(gunDrawn);
		BF_INVENTORY.setObjectScene(INV_HANDCUFFS, 390);
		BF_GLOBALS._player.setVisage(361);
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player.setStrip(3);

		BF_GLOBALS._sceneItems.push_front(&_laura);
		BF_GLOBALS._walkRegions.disableRegion(3);
		_harrison.setAction(NULL);
		BF_GLOBALS._player.enableControl();

		BF_GLOBALS.set2Flags(f1015Marina);
		BF_GLOBALS.set2Flags(f1027Marina);
		BF_GLOBALS.set2Flags(f1098Marina);
		break;
	case 3705:
		_laura.remove();
		BF_GLOBALS._walkRegions.enableRegion(6);
		BF_GLOBALS._walkRegions.enableRegion(1);
		BF_GLOBALS._player.enableControl();
		break;
	case 3708:
		_sound1.play(6);
		BF_GLOBALS._sound1.changeSound(35);
		BF_INVENTORY.setObjectScene(INV_GREENS_GUN, 370);
		T2_GLOBALS._uiElements.addScore(50);
		BF_GLOBALS._sceneItems.push_front(&_greensGun);

		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player._canWalk = false;

		_sceneMode = 0;
		_object5.remove();
		_green.setAction(NULL);
		BF_GLOBALS._walkRegions.disableRegion(6);
		BF_GLOBALS._walkRegions.disableRegion(1);
		break;
	case 3709:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 3713;
		setAction(&_sequenceManager, this, 3713, NULL);
		break;
	case 3710:
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player._canWalk = false;
		_sceneMode = 3701;
		_green.setAction(&_sequenceManager, this, 3701, NULL);
		break;
	case 3711:
		BF_GLOBALS._walkRegions.disableRegion(3);
		BF_INVENTORY.setObjectScene(INV_GREENS_GUN, 1);
		T2_GLOBALS._uiElements.addScore(30);
		_greensGun.remove();
		BF_GLOBALS._player.enableControl();
		break;
	case 3712:
		T2_GLOBALS._uiElements._active = false;
		T2_GLOBALS._uiElements.hide();

		if (BF_GLOBALS._sceneObjects->contains(&_greensGun))
			_greensGun.remove();
		_sceneMode = 1;
		setAction(&_sequenceManager, this, 3714, NULL);
		break;
	case 3713:
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player._canWalk = false;
		break;
	case 3715:
		T2_GLOBALS._uiElements.addScore(50);
		BF_INVENTORY.setObjectScene(INV_GREEN_ID, 1);
		BF_INVENTORY.setObjectScene(INV_GREENS_KNIFE, 1);
		BF_GLOBALS._walkRegions.disableRegion(3);
		BF_GLOBALS._player.enableControl();
		break;
	case 3716:
		BF_GLOBALS.clearFlag(fCalledBackup);
		BF_GLOBALS.clearFlag(fBackupIn350);
		BF_GLOBALS.clearFlag(fBackupArrived340);
		BF_GLOBALS.setFlag(greenTaken);
		BF_GLOBALS.setFlag(fToldToLeave340);

		BF_GLOBALS._bookmark = bCalledToDomesticViolence;
		_harrison.remove();
		_green.remove();
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene370::process(Event &event) {
	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1))) {
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

void Scene370::dispatch() {
	SceneExt::dispatch();

	if ((BF_GLOBALS._player._position.x >= 316) && !BF_GLOBALS._sceneObjects->contains(&_laura) &&
				!BF_GLOBALS._sceneObjects->contains(&_green)) {
		if (BF_GLOBALS._bookmark < bArrestedGreen) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 3712;
			setAction(&_sequenceManager, this, 3712, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS._sound1.fadeSound(35);
			BF_GLOBALS._sceneManager.changeScene(360);
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 380 - Outside City Hall & Jail
 *
 *--------------------------------------------------------------------------*/

bool Scene380::Vechile::startAction(CursorType action, Event &event) {
	Scene380 *scene = (Scene380 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 1;
		scene->setAction(&scene->_sequenceManager, scene, 3802, &BF_GLOBALS._player, NULL);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene380::Door::startAction(CursorType action, Event &event) {
	Scene380 *scene = (Scene380 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 2;
		scene->setAction(&scene->_sequenceManager, scene, 3800, &BF_GLOBALS._player, &scene->_door, NULL);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene380::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(380);
	setZoomPercents(68, 80, 131, 100);

	BF_GLOBALS._sound1.fadeSound(33);
	BF_GLOBALS._walkRegions.disableRegion(9);

	_door.postInit();
	_door.setVisage(380);
	_door.setStrip(4);
	_door.setPosition(Common::Point(132, 66));
	_door.setDetails(380, 12, 13, -1, 1, (SceneItem *)NULL);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.changeZoom(-1);

	_vechile.postInit();
	_vechile.setVisage(380);
	_vechile.fixPriority(109);

	if (BF_GLOBALS.getFlag(fWithLyle)) {
		// Show vechile as car
		_vechile.setStrip(3);
		_vechile.setPosition(Common::Point(273, 125));
		_vechile.setDetails(580, 2, 3, -1, 1, (SceneItem *)NULL);

		BF_GLOBALS._player.setVisage(129);
		BF_GLOBALS._walkRegions.disableRegion(12);
		BF_GLOBALS._walkRegions.disableRegion(18);
		BF_GLOBALS._walkRegions.disableRegion(19);
		BF_GLOBALS._walkRegions.disableRegion(20);
		BF_GLOBALS._walkRegions.disableRegion(25);
		BF_GLOBALS._walkRegions.disableRegion(26);
		BF_GLOBALS._walkRegions.disableRegion(27);
	} else if (BF_GLOBALS.getFlag(onDuty)) {
		// Show on duty motorcycle
		_vechile.setStrip(2);
		_vechile.setDetails(300, 11, 13, -1, 1, (SceneItem *)NULL);
		_vechile.setPosition(Common::Point(252, 115));

		BF_GLOBALS._player.setVisage(1341);
	} else {
		// Show off duty motorcycle
		_vechile.setStrip(1);
		_vechile.setDetails(580, 0, 1, -1, 1, (SceneItem *)NULL);
		_vechile.setPosition(Common::Point(249, 110));

		BF_GLOBALS._player.setVisage(129);
	}

	BF_GLOBALS._player.updateAngle(_vechile._position);
	BF_GLOBALS._sceneItems.push_back(&_door);

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 50:
	case 60:
	case 330:
	case 370:
		BF_GLOBALS._player.setPosition(Common::Point(251, 100));
		BF_GLOBALS._player._strip = 3;
		BF_GLOBALS._player.enableControl();
		break;
	default:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager, this, 3801, &BF_GLOBALS._player, &_door, NULL);
		break;
	}

	_item1.setDetails(7, 380, 0, 1, 2, 1);
	_item2.setDetails(9, 380, 3, 4, 5, 1);
	_item3.setDetails(17, 380, 6, 7, 8, 1);
	_item4.setDetails(20, 380, 9, 10, 11, 1);
	_item5.setDetails(15, 380, 14, 15, 16, 1);
	_item6.setDetails(4, 380, 17, 18, 19, 1);
	_item7.setDetails(19, 380, 20, 4, 21, 1);
	_item8.setDetails(18, 380, 22, 23, 24, 1);
	_item9.setDetails(6, 380, 25, 26, 27, 1);
}

void Scene380::signal() {
	switch (_sceneMode) {
	case 1:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 2:
		BF_GLOBALS._sceneManager.changeScene(385);
		break;
	default:
		BF_GLOBALS._player.enableControl();
	}
}

/*--------------------------------------------------------------------------
 * Scene 385 - City Hall
 *
 *--------------------------------------------------------------------------*/

void Scene385::Action1::signal() {
	Scene385 *scene = (Scene385 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();

		switch (scene->_talkAction) {
		case 0:
			error("Bugs::talkscript385");
			break;
		case 3850:
		case 3851:
		case 3852:
		case 3853:
		case 3854:
		case 3855:
		case 3856:
		case 3857:
		case 3863:
		case 3866: {
			ADD_PLAYER_MOVER(187, 144);
			break;
		}
		default: {
			ADD_PLAYER_MOVER(231, 158);
			break;
		}
		}
		break;
	case 1:
		BF_GLOBALS._player.changeAngle(45);
		setDelay(3);
		break;
	case 2:
		scene->_stripManager.start(scene->_talkAction, this);
		break;
	case 3:
		if (scene->_talkAction)
			scene->_dezi.animate(ANIM_MODE_5, NULL);
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}
}

void Scene385::Action2::signal() {
	Scene385 *scene = (Scene385 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		ADD_PLAYER_MOVER(231, 158);
		break;
	}
	case 1:
		BF_GLOBALS._player.updateAngle(BF_GLOBALS._player._position);
		setDelay(3);
		break;
	case 2:
		scene->_stripManager.start(3864, this);
		break;
	case 3:
		scene->_jim.animate(ANIM_MODE_5, this);
		break;
	case 4:
		scene->_jim.setStrip(4);
		scene->_jim.animate(ANIM_MODE_5, this);
		break;
	case 5:
		scene->_stripManager.start(3865, this);
		break;
	case 6:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene385::Door::startAction(CursorType action, Event &event) {
	Scene385 *scene = (Scene385 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		BF_GLOBALS._walkRegions.enableRegion(6);
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 3850;
		scene->setAction(&scene->_sequenceManager, scene, 3850, &BF_GLOBALS._player, this, NULL);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene385::Jim::startAction(CursorType action, Event &event) {
	Scene385 *scene = (Scene385 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_TALK) {
		if (scene->_jimFlag) {
			scene->_talkAction = 3867;
			scene->setAction(&scene->_action1);
		} else {
			switch (BF_GLOBALS._dayNumber) {
			case 1:
				scene->_talkAction = 3858;
				break;
			case 2:
				scene->_talkAction = 3859;
				break;
			case 3:
				scene->_talkAction = 3860;
				break;
			case 4:
				scene->_talkAction = 3861;
				break;
			default:
				BF_GLOBALS._deziTopic = 3;
				scene->_talkAction = 3868;
				break;
			}

			scene->_jimFlag = true;
			scene->setAction(&scene->_action1);
		}
		return true;
	} else if (action == INV_PRINT_OUT) {
		if (!BF_GLOBALS.getFlag(fGotPointsForMCard)) {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fGotPointsForMCard);

			scene->setAction(&scene->_action2);
			return true;
		} else
			return false;
	} else if (action < CURSOR_WALK)
		// Any other inventory item
		return false;
	else
		return NamedObject::startAction(action, event);
}

bool Scene385::Dezi::startAction(CursorType action, Event &event) {
	Scene385 *scene = (Scene385 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if (BF_GLOBALS._deziTopic == 3) {
			scene->_talkAction = 3857;
		} else {
			BF_GLOBALS._deziTopic = 3;

			switch (BF_GLOBALS._dayNumber) {
			case 1:
				if (BF_GLOBALS._deziTopic++ == 0) {
					scene->_talkAction = 3850;
				} else {
					BF_GLOBALS._deziTopic = 3;
					scene->_talkAction = 3851;
				}
				break;
			case 2:
				if (BF_GLOBALS._deziTopic++ == 0) {
					scene->_talkAction = 3852;
				} else {
					BF_GLOBALS._deziTopic = 3;
					scene->_talkAction = 3853;
				}
				break;
			case 3:
				if (BF_GLOBALS._deziTopic++ == 0) {
					scene->_talkAction = 3854;
				} else {
					BF_GLOBALS._deziTopic = 3;
					scene->_talkAction = 3855;
				}
				break;
			case 4:
				BF_GLOBALS._deziTopic = 3;
				scene->_talkAction = 3856;
				break;
			default:
				BF_GLOBALS._deziTopic = 3;
				scene->_talkAction = 3868;
				break;
			}
		}

		scene->setAction(&scene->_action1);
		return true;
	case INV_PRINT_OUT:
		scene->_talkAction = 3863;
		scene->setAction(&scene->_action1);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene385::Exit::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER(BF_GLOBALS._player._position.x - 100, BF_GLOBALS._player._position.y + 100);
	return true;
}

/*--------------------------------------------------------------------------*/


Scene385::Scene385(): SceneExt() {
	_talkAction = 0;
	_jimFlag = false;
}

void Scene385::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_talkAction);
	s.syncAsSint16LE(_jimFlag);
}

void Scene385::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 1;

	_exit.setDetails(Rect(0, 162, 320, 167), 385, -1, -1, -1, 1, NULL);
	BF_GLOBALS._sound1.fadeSound(119);

	loadScene(385);
	setZoomPercents(115, 90, 145, 100);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jake385Speaker);
	_stripManager.addSpeaker(&_jimSpeaker);
	_stripManager.addSpeaker(&_deziSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 361 : 368);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.changeZoom(-1);
	BF_GLOBALS._player.enableControl();

	_item3.setDetails(1, 385, 5, -1, -1, 1);
	_item2.setDetails(2, 385, 7, -1, -1, 1);
	_item1.setDetails(3, 385, 6, -1, 6, 1);
	_item5.setDetails(4, 385, 14, -1, -1, 1);

	_jim.postInit();
	_jim.setVisage(385);
	_jim.setStrip(3);
	_jim.setPosition(Common::Point(304, 113));
	_jim.setDetails(385, 1, -1, 2, 1, (SceneItem *)NULL);

	_dezi.postInit();
	_dezi.setVisage(385);
	_dezi.setStrip(2);
	_dezi.setPosition(Common::Point(235, 93));
	_dezi.fixPriority(120);
	_dezi.setDetails(385, 3, -1, 2, 1, (SceneItem *)NULL);

	_door.postInit();
	_door.setVisage(385);
	_door.setPosition(Common::Point(107, 27));
	_door.setDetails(385, 0, -1, -1, 1, (SceneItem *)NULL);

	BF_GLOBALS._walkRegions.disableRegion(6);

	if (BF_GLOBALS._sceneManager._previousScene == 390) {
		BF_GLOBALS._player.setPosition(Common::Point(109, 119));
	} else {
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._player.setPosition(Common::Point(15, 250));
		_sceneMode = 3852;
		setAction(&_sequenceManager, this, 3852, &BF_GLOBALS._player, NULL);
	}

	_item4.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 385, 4, -1, -1, 1, NULL);
}

void Scene385::signal() {
	switch (_sceneMode) {
	case 3850:
		BF_GLOBALS._sceneManager.changeScene(390);
		break;
	case 3851:
		BF_GLOBALS._sceneManager.changeScene(380);
		break;
	case 3852:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene385::process(Event &event) {
	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1))) {
		// Check if the cursor is on an exit
		if (_exit.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_SW);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			// In case an exit cursor was being shown, restore the previously selected cursor
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene385::dispatch() {
	SceneExt::dispatch();

	if (!_action && (BF_GLOBALS._player._position.y > 162)) {
		// Leaving by exit
		BF_GLOBALS._player.disableControl();
		_sceneMode = 3851;
		setAction(&_sequenceManager, this, 3851, &BF_GLOBALS._player, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 390 - City Jail
 *
 *--------------------------------------------------------------------------*/

void Scene390::Action1::signal() {
	Scene390 *scene = (Scene390 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1:
		switch (scene->_sceneMode) {
		case 3900:
		case 3905:
		case 3906:
		case 3913:
		case 3914:
		case 3916:
		case 3917: {
			ADD_PLAYER_MOVER(75, 122);
			break;
		}
		default: {
			ADD_PLAYER_MOVER(147, 133);
			break;
		}
		}
		break;
	case 2:
		switch (scene->_sceneMode) {
		case 3900:
		case 3905:
		case 3906:
		case 3913:
		case 3914:
		case 3916:
		case 3917: {
			BF_GLOBALS._player.changeAngle(315);
			break;
		}
		default: {
			BF_GLOBALS._player.changeAngle(90);
			break;
		}
		}
		setDelay(1);
		break;
	case 3:
		scene->_stripManager.start(scene->_sceneMode, this);
		break;
	case 4:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene390::BookingForms::startAction(CursorType action, Event &event) {
	Scene390 *scene = (Scene390 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_flag) {
			SceneItem::display2(390, 16);
		} else if (!BF_GLOBALS.getFlag(onDuty)) {
			SceneItem::display2(390, 22);
		} else if (((BF_GLOBALS._bookmark < bLauraToParamedics) && BF_GLOBALS.getFlag(greenTaken)) ||
				((BF_GLOBALS._bookmark >= bStoppedFrankie) && !BF_GLOBALS.getFlag(frankInJail) && (BF_GLOBALS._dayNumber == 1))) {
			BF_GLOBALS._player.disableControl();
			++_flag;
			scene->_sceneMode = 3902;
			setAction(&scene->_sequenceManager, scene, 3902, &BF_GLOBALS._player, NULL);
		} else {
			SceneItem::display2(390, 23);
		}
		return true;
	default:
		return NamedHotspotExt::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene390::Green::startAction(CursorType action, Event &event) {
	Scene390 *scene = (Scene390 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if (!_flag) {
			++_flag;
			scene->_sceneMode = 3901;
		} else {
			scene->_sceneMode = 3902;
		}
		scene->setAction(&scene->_action1);
		return true;
	case INV_MIRANDA_CARD:
		if (BF_GLOBALS.getFlag(readGreenRights)) {
			SceneItem::display2(390, 15);
		} else {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(readGreenRights);
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 3904;

			scene->setAction(&scene->_sequenceManager, scene, 3904, &BF_GLOBALS._player, this, NULL);
		}
		return true;
	default:
		return NamedObjectExt::startAction(action, event);
	}
}

bool Scene390::Object2::startAction(CursorType action, Event &event) {
	Scene390 *scene = (Scene390 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if (!_flag)
			break;

		if (!BF_GLOBALS.getFlag(onDuty)) {
			scene->_sceneMode = 3917;
		} else if (BF_GLOBALS.getFlag(fTalkedToBarry) && !BF_GLOBALS.getFlag(fTalkedToLarry)) {
			scene->_sceneMode = 3913;
		} else if (!BF_GLOBALS.getFlag(fTalkedToLarry)) {
			scene->_sceneMode = 3914;
		} else {
			++_flag;

			if (BF_GLOBALS._sceneObjects->contains(&scene->_green)) {
				scene->_sceneMode = 3900;
			} else if (!BF_GLOBALS._sceneObjects->contains(&scene->_green)) {
				scene->_sceneMode = 3905;
			} else if (BF_GLOBALS.getFlag(fCuffedFrankie)) {
				scene->_sceneMode = 3906;
			} else {
				scene->_sceneMode = 3916;
			}
		}

		BF_GLOBALS.setFlag(fTalkedToLarry);
		scene->setAction(&scene->_action1);
		return true;
	case INV_BOOKING_GREEN:
		if (BF_GLOBALS._bookmark >= bLauraToParamedics)
			SceneItem::display2(390, 21);
		else {
			BF_GLOBALS._player.disableControl();
			BF_GLOBALS._bookmark = bLauraToParamedics;
			scene->_sceneMode = 3905;

			BF_INVENTORY.setObjectScene(INV_GREEN_ID, 390);
			if (BF_GLOBALS.getFlag(fLeftTraceIn910))
				BF_GLOBALS.clearFlag(fLeftTraceIn910);

			scene->setAction(&scene->_sequenceManager, scene, 3905, &BF_GLOBALS._player, this, NULL);
			BF_GLOBALS.clearFlag(f1015Marina);
			BF_GLOBALS.clearFlag(f1027Marina);
			BF_GLOBALS.clearFlag(f1035Marina);
		}
		return true;
	case INV_BOOKING_FRANKIE:
	case INV_BOOKING_GANG:
		if (BF_GLOBALS.getFlag(frankInJail)) {
			SceneItem::display2(390, 21);
		} else {
			scene->_sceneMode = 3905;
			BF_GLOBALS._player.disableControl();
			scene->setAction(&scene->_sequenceManager, scene, 3905, &BF_GLOBALS._player, this, NULL);

			BF_INVENTORY.setObjectScene(INV_FRANKIE_ID, 390);
			BF_INVENTORY.setObjectScene(INV_TYRONE_ID, 390);
			BF_GLOBALS.clearFlag(frankInJail);
			BF_GLOBALS.clearFlag(f1015Frankie);
			BF_GLOBALS.clearFlag(f1027Frankie);
			BF_GLOBALS.clearFlag(f1035Frankie);
		}
		return true;
	default:
		break;
	}

	return NamedObjectExt::startAction(action, event);
}

bool Scene390::Object3::startAction(CursorType action, Event &event) {
	Scene390 *scene = (Scene390 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case INV_GREEN_ID:
		scene->_sceneMode = 3903;
		setAction(&scene->_sequenceManager, scene, 3902, &BF_GLOBALS._player, NULL);
		remove();
		BF_INVENTORY.setObjectScene(INV_BOOKING_GREEN, 1);
		return true;
	case INV_FRANKIE_ID:
		BF_INVENTORY.setObjectScene(INV_BOOKING_FRANKIE, 1);
		scene->_sceneMode = 3903;
		setAction(&scene->_sequenceManager, scene, 3902, &BF_GLOBALS._player, NULL);
		remove();
		return true;
	case INV_TYRONE_ID:
		scene->_sceneMode = 3903;
		setAction(&scene->_sequenceManager, scene, 3902, &BF_GLOBALS._player, NULL);
		remove();
		BF_INVENTORY.setObjectScene(INV_BOOKING_GANG, 1);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene390::GangMember1::startAction(CursorType action, Event &event) {
	Scene390 *scene = (Scene390 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		scene->_sceneMode = 3908;
		scene->setAction(&scene->_action1);
		return true;
	case INV_MIRANDA_CARD:
		if (BF_GLOBALS.getFlag(readFrankRights)) {
			SceneItem::display2(390, 15);
		} else {
			BF_GLOBALS.setFlag(readFrankRights);
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS._player.disableControl();

			scene->_sceneMode = 3904;
			scene->setAction(&scene->_sequenceManager, scene, 3907, &BF_GLOBALS._player, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene390::GangMember2::startAction(CursorType action, Event &event) {
	Scene390 *scene = (Scene390 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		scene->_sceneMode = 3909;
		scene->setAction(&scene->_action1);
		return true;
	case INV_MIRANDA_CARD:
		if (BF_GLOBALS.getFlag(readFrankRights)) {
			SceneItem::display2(390, 15);
		} else {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(readFrankRights);
			BF_GLOBALS._player.disableControl();

			scene->_sceneMode = 3904;
			scene->setAction(&scene->_sequenceManager, scene, 3907, &BF_GLOBALS._player, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene390::Door::startAction(CursorType action, Event &event) {
	Scene390 *scene = (Scene390 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if ((BF_INVENTORY.getObjectScene(INV_BOOKING_GREEN) == 1) && (BF_INVENTORY.getObjectScene(INV_GREEN_ID) == 1)) {
			scene->_stripManager.start(3918, &BF_GLOBALS._stripProxy);
		} else if ((BF_INVENTORY.getObjectScene(INV_BOOKING_FRANKIE) == 1) && (BF_INVENTORY.getObjectScene(INV_FRANKIE_ID) == 1)) {
			scene->_stripManager.start(3918, &BF_GLOBALS._stripProxy);
		} else if ((BF_INVENTORY.getObjectScene(INV_BOOKING_GANG) == 1) && (BF_INVENTORY.getObjectScene(INV_TYRONE_ID) == 1)) {
			scene->_stripManager.start(3918, &BF_GLOBALS._stripProxy);
		} else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 3900;
			scene->setAction(&scene->_sequenceManager, scene, 3900, &BF_GLOBALS._player, this, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene390::postInit(TsAGE::SceneObjectList *OwnerList) {
	BF_GLOBALS._sound1.play(17);
	SceneExt::postInit();
	setZoomPercents(105, 85, 130, 100);
	loadScene(390);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_greenSpeaker);
	_stripManager.addSpeaker(&_jailerSpeaker);
	_stripManager.addSpeaker(&_shooterSpeaker);
	_stripManager.addSpeaker(&_driverSpeaker);

	if (BF_GLOBALS._dayNumber == 0) {
		BF_GLOBALS.setFlag(onDuty);
		BF_GLOBALS._bookmark = bStoppedFrankie;
		BF_GLOBALS.clearFlag(frankInJail);
		BF_GLOBALS.setFlag(fTalkedToBarry);
		BF_INVENTORY.setObjectScene(INV_22_SNUB, 1);
	}

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 361 : 368);
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.setPosition(Common::Point(147, 114));
	BF_GLOBALS._player.setStrip(6);
	BF_GLOBALS._player.changeZoom(-1);

	_object2.postInit();
	_object2.setVisage(395);
	_object2.setStrip(2);
	_object2.setPosition(Common::Point(38, 84));
	_object2.fixPriority(50);
	_object2._flag = 0;
	_object2.setDetails(390, 10, 17, 10, 1, (SceneItem *)NULL);

	_door.postInit();
	_door.setVisage(390);
	_door.setStrip(2);
	_door.setPosition(Common::Point(151, 18));
	_door.setDetails(390, 5, -1, -1, 1, (SceneItem *)NULL);

	BF_GLOBALS._player.disableControl();
	_sceneMode = 3901;
	setAction(&_sequenceManager, this, 3901, &BF_GLOBALS._player, NULL);

	if ((BF_GLOBALS._bookmark < bLauraToParamedics) && BF_GLOBALS.getFlag(greenTaken)) {
		_green.postInit();
		_green.setVisage(392);
		_green.setPosition(Common::Point(241, 164));
		_green.fixPriority(153);
		_green.setDetails(390, 12, -1, 13, 1, (SceneItem *)NULL);
		_green._flag = 0;
	}

	if ((BF_GLOBALS._bookmark >= bStoppedFrankie) && !BF_GLOBALS.getFlag(frankInJail) && (BF_GLOBALS._dayNumber == 1)) {
		_gangMember1.postInit();
		_gangMember1.setVisage(396);
		_gangMember1.setPosition(Common::Point(273, 169));
		_gangMember1.fixPriority(152);
		_gangMember1._flag = 0;
		_gangMember1.setDetails(390, 19, -1, 20, 1, (SceneItem *)NULL);

		_gangMember2.postInit();
		_gangMember2.setVisage(396);
		_gangMember2.setStrip(2);
		_gangMember2.setPosition(Common::Point(241, 153));
		_gangMember2.fixPriority(152);
		_gangMember2._flag = 0;
		_gangMember2.setDetails(390, 19, -1, 20, 1, (SceneItem *)NULL);
	}

	_item1.setDetails(Rect(22, 40, 77, 67), 390, 0, -1, 1, 1, NULL);
	_bookingForms.setDetails(Rect(89, 46, 110, 65), 390, 2, -1, -1, 1, NULL);
	_item3.setDetails(Rect(193, 0, 320, 165), 390, 4, -1, -1, 1, NULL);
	_item4.setDetails(Rect(0, 0, 320, 170), 390, 7, -1, -1, 1, NULL);
}

void Scene390::signal() {
	switch (_sceneMode) {
	case 3900:
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(385);
		break;
	case 3901:
	case 3904:
		if ((BF_INVENTORY.getObjectScene(INV_22_SNUB) == 410) && BF_GLOBALS._sceneObjects->contains(&_gangMember1)) {
			_sceneMode = 3906;
			setAction(&_sequenceManager, this, 3906, &BF_GLOBALS._player, &_object2, NULL);
		} else if ((BF_INVENTORY.getObjectScene(INV_GREENS_KNIFE) == 370) && BF_GLOBALS._sceneObjects->contains(&_green)) {
			_sceneMode = 3906;
			setAction(&_sequenceManager, this, 3906, &BF_GLOBALS._player, &_object2, NULL);
		} else {
			BF_GLOBALS._player.enableControl();
		}
		break;
	case 3902:
		SceneItem::display2(390, 3);

		_object3.postInit();
		_object3.setVisage(390);
		_object3.setPosition(Common::Point(250, 60));
		_object3.fixPriority(255);
		_object3.setDetails(390, 8, -1, 9, 2, (SceneItem *)NULL);

		BF_GLOBALS._player.enableControl();
		break;
	case 3903:
		SceneItem::display2(390, 18);
		BF_GLOBALS._player.enableControl();
		break;
	case 3905:
		T2_GLOBALS._uiElements.addScore(30);
		_object2.remove();
		BF_INVENTORY.setObjectScene(INV_HANDCUFFS, 1);
		BF_GLOBALS._player.enableControl();
		break;
	case 3906:
		BF_GLOBALS._deathReason = 18;
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	}
}

} // End of namespace BlueForce

} // End of namespace TsAGE
