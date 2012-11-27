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

#include "tsage/blue_force/blueforce_scenes5.h"
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 550 - Outside Bikini Hut
 *
 *--------------------------------------------------------------------------*/

void Scene550::Action1::signal() {
	Scene550 *scene = (Scene550 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(5);
		break;
	case 1:
		scene->_stripManager.start(scene->_sceneMode, this);
		break;
	case 2:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene550::Lyle::startAction(CursorType action, Event &event) {
	Scene550 *scene = (Scene550 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if ((BF_INVENTORY.getObjectScene(INV_SCHEDULE) == 1) ||
				(BF_INVENTORY.getObjectScene(INV_9MM_BULLETS) == 1)) {
			if ((BF_INVENTORY.getObjectScene(INV_SCHEDULE) == 1) &&
					(BF_INVENTORY.getObjectScene(INV_9MM_BULLETS) == 1)) {
				BF_GLOBALS.setFlag(fToldLyleOfSchedule);
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 5501;
				scene->setAction(&scene->_sequenceManager, scene, 5514, &BF_GLOBALS._player, this, NULL);
			} else {
				scene->_sceneMode = 0;
				scene->_stripManager.start(5509, scene);
			}
		} else if (BF_GLOBALS._sceneManager._previousScene == 930) {
			scene->_sceneMode = 5512;
			scene->setAction(&scene->_action1);
		} else {
			scene->_sceneMode = BF_INVENTORY.getObjectScene(INV_CARAVAN_KEY) == 1 ? 5513 : 5512;
			scene->setAction(&scene->_action1);
		}
		return true;
	case INV_SCHEDULE:
		BF_GLOBALS.setFlag(fToldLyleOfSchedule);
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 5501;
		scene->setAction(&scene->_sequenceManager, scene, 5514, &BF_GLOBALS._player, this, NULL);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene550::CaravanDoor::startAction(CursorType action, Event &event) {
	Scene550 *scene = (Scene550 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(550, 0);
		return true;
	case CURSOR_USE:
		SceneItem::display2(550, 7);
		return true;
	case INV_CARAVAN_KEY:
		if ((BF_GLOBALS._dayNumber != 3) || !BF_GLOBALS.getFlag(fWithLyle))
			SceneItem::display2(550, 33);
		else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 5500;
			scene->setAction(&scene->_sequenceManager, scene, 5500, &BF_GLOBALS._player, this, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene550::Vechile::startAction(CursorType action, Event &event) {
	Scene550 *scene = (Scene550 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(550, 3);
		return true;
	case CURSOR_USE:
		if (!BF_GLOBALS.getFlag(fWithLyle)) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 5510;
			scene->setAction(&scene->_sequenceManager, scene, BF_GLOBALS.getFlag(fWithLyle) ? 5510 : 5515,
				&BF_GLOBALS._player, this, NULL);
		} else if (BF_GLOBALS.getFlag(fToldLyleOfSchedule)) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 2;
			scene->setAction(&scene->_sequenceManager, scene, 5501, &BF_GLOBALS._player, NULL);
		} else if ((BF_INVENTORY.getObjectScene(INV_SCHEDULE) == 1) ||
					(BF_INVENTORY.getObjectScene(INV_9MM_BULLETS) == 1)) {
			if (BF_INVENTORY.getObjectScene(INV_9MM_BULLETS) == 1) {
				scene->_sceneMode = 5501;
				scene->_stripManager.start(5511, scene);
			} else {
				scene->_sceneMode = 0;
				scene->_stripManager.start(5509, scene);
			}
		} else if (BF_GLOBALS._sceneManager._previousScene == 930) {
			scene->_sceneMode = 5512;
			scene->setAction(&scene->_action1);
		} else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 2;
			scene->setAction(&scene->_sequenceManager, scene, 5501, &BF_GLOBALS._player, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene550::postInit(SceneObjectList *OwnerList) {
	BF_GLOBALS._sound1.fadeSound(16);

	if ((BF_GLOBALS._bookmark == bInspectionDone) && !BF_GLOBALS.getFlag(fHasDrivenFromDrunk)) {
		_sceneMode = 1;
		signal();
		return;
	}

	SceneExt::postInit();
	loadScene(550);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_lyleHatSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.setPosition(Common::Point(160, 100));
	BF_GLOBALS._player._moveDiff = Common::Point(2, 1);
	BF_GLOBALS._player.enableControl();

	_caravanDoor.postInit();
	_caravanDoor.setVisage(550);
	_caravanDoor.setPosition(Common::Point(34, 66));
	BF_GLOBALS._sceneItems.push_back(&_caravanDoor);

	_vechile.postInit();
	_vechile.fixPriority(70);

	if (BF_GLOBALS.getFlag(fWithLyle)) {
		BF_GLOBALS._walkRegions.disableRegion(10);
		BF_GLOBALS._walkRegions.disableRegion(11);

		_vechile.setVisage(444);
		_vechile.setStrip(4);
		_vechile.setFrame2(2);
		_vechile.setPosition(Common::Point(110, 85));
		_vechile.fixPriority(76);

		_lyle.postInit();
		_lyle.setVisage(835);
		_lyle.setPosition(Common::Point(139, 83));
		_lyle.setDetails(550, 29, 30, 31, 1, (SceneItem *)NULL);
		_lyle.setStrip(8);

		BF_GLOBALS._player.setVisage(303);
		BF_GLOBALS._player.setPosition(Common::Point(89, 76));
		BF_GLOBALS._player.updateAngle(_lyle._position);
	} else {
		BF_GLOBALS._walkRegions.disableRegion(12);

		_vechile.setPosition(Common::Point(205, 77));
		_vechile.changeZoom(80);

		if (BF_GLOBALS.getFlag(onDuty)) {
			_vechile.setVisage(301);
			_vechile.setStrip(1);

			BF_GLOBALS._player.setVisage(304);
		} else {
			_vechile.setVisage(580);
			_vechile.setStrip(2);
			_vechile.setFrame(2);

			BF_GLOBALS._player.setVisage(303);
		}
	}

	BF_GLOBALS._sceneItems.push_back(&_vechile);

	if (BF_GLOBALS._sceneManager._previousScene == 930) {
		_caravanDoor.setFrame(_caravanDoor.getFrameCount());
		BF_GLOBALS._player.disableControl();

		_sceneMode = 0;
		setAction(&_sequenceManager, this, 5512, &BF_GLOBALS._player, &_caravanDoor, NULL);
	} else if (BF_GLOBALS.getFlag(onDuty)) {
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager, this, 5502, &BF_GLOBALS._player, &_vechile, NULL);
	} else if (!BF_GLOBALS.getFlag(fWithLyle)) {
		BF_GLOBALS._player.setPosition(Common::Point(185, 70));
	} else if (BF_GLOBALS._bookmark == bFlashBackOne) {
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager, this, 5513, &_lyle, NULL);
	} else {
		_sceneMode = 0;
	}

	_item2.setDetails(Rect(0, 26, 53, 67), 550, 1, -1, 2, 1, NULL);
	_item3.setDetails(Rect(53, 12, 173, 65), 550, 4, -1, 5, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 170), 550, 6, -1, -1, 1, NULL);
}

void Scene550::signal() {
	switch (_sceneMode) {
	case 0:
		BF_GLOBALS._player.enableControl();
		break;
	case 1:
		BF_GLOBALS._sceneManager.changeScene(551);
		break;
	case 2:
	case 5510:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 3:
		BF_GLOBALS._driveFromScene = 16;
		BF_GLOBALS._driveToScene = 128;
		BF_GLOBALS._mapLocationId = 128;
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(800);
		break;
	case 5500:
		BF_GLOBALS._sceneManager.changeScene(930);
		break;
	case 5501:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 3;
		setAction(&_sequenceManager, this, 5501, &BF_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 551 - Outside Bikini Hut (Drunk Stop)
 *
 *--------------------------------------------------------------------------*/

void Scene551::Action2::signal() {
	Scene551 *scene = (Scene551 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._walkRegions.enableRegion(18);
		BF_GLOBALS._walkRegions.enableRegion(4);
		scene->_field1CD2 = 1;

		scene->_harrison.setObjectWrapper(new SceneObjectWrapper());
		scene->_harrison.animate(ANIM_MODE_1, NULL);

		BF_GLOBALS._player.setVisage(304);
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);

		if (scene->_drunkStanding._flag != 1) {
			ADD_PLAYER_MOVER_THIS(scene->_harrison, 126, 85);
		} else {
			ADD_PLAYER_MOVER_THIS(scene->_harrison, 88, 91);
		}
		break;
	case 1:
		scene->_harrison.updateAngle(BF_GLOBALS._player._position);

		if (scene->_drunkStanding._flag == 1) {
			BF_GLOBALS._walkRegions.disableRegion(4);
			ADD_PLAYER_MOVER(71, 97);
		} else {
			ADD_PLAYER_MOVER(141, 87);
		}
		break;
	case 2:
		scene->_harrison.updateAngle(BF_GLOBALS._player._position);
		BF_GLOBALS._player.updateAngle(scene->_harrison._position);
		setDelay(10);
		break;
	case 3:
		scene->_stripManager.start(scene->_harrison._flag, this);
		break;
	case 4:
		scene->_field1CD2 = 0;
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene551::Vechile::startAction(CursorType action, Event &event) {
	Scene551 *scene = (Scene551 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(550, 3);
		return true;
	case CURSOR_USE:
		if (!BF_GLOBALS.getFlag(didDrunk)) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 1;
			scene->setAction(&scene->_sequenceManager, scene, 5510, &BF_GLOBALS._player, this, NULL);
		} else if (BF_INVENTORY.getObjectScene(INV_CENTER_PUNCH) == 1) {
			BF_INVENTORY.setObjectScene(INV_CENTER_PUNCH, 0);
			scene->_harrison._flag = 5505;
			scene->setAction(&scene->_action2, scene);
			scene->_sceneMode = 5509;
		} else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 1;
			scene->setAction(&scene->_sequenceManager, scene, 5510, &BF_GLOBALS._player, this, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene551::DrunkStanding::startAction(CursorType action, Event &event) {
	Scene551 *scene = (Scene551 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(550, 15);
		return true;
	case CURSOR_USE:
		SceneItem::display2(550, 16);
		return true;
	case CURSOR_TALK:
		SceneItem::display2(550, 17);
		return true;
	case INV_HANDCUFFS:
		scene->_harrison.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS.set2Flags(f1098Drunk);
		BF_GLOBALS.setFlag(didDrunk);
		_flag = 2;
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 5509;

		scene->setAction(&scene->_sequenceManager, scene, 5509, &BF_GLOBALS._player, this, &scene->_harrison, NULL);
		BF_GLOBALS.set2Flags(f1015Drunk);
		return true;
	default:
		return NamedObjectExt::startAction(action, event);
	}
}

bool Scene551::Drunk::startAction(CursorType action, Event &event) {
	Scene551 *scene = (Scene551 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		switch (_flag) {
		case 0:
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 0;
			scene->setAction(&scene->_sequenceManager, scene, 5505, &BF_GLOBALS._player, &scene->_object15, NULL);
			break;
		case 1:
			SceneItem::display2(550, 11);
			break;
		case 2:
			SceneItem::display2(550, 12);
			break;
		case 3:
			SceneItem::display2(550, 27);
			break;
		}
		return true;
	case CURSOR_USE:
		switch (_flag) {
		case 0:
			BF_GLOBALS._player.disableControl();
			if (BF_GLOBALS.getFlag(fTalkedToDrunkInCar)) {
				scene->_sceneMode = 5508;
				scene->setAction(&scene->_sequenceManager, scene, 5508, &BF_GLOBALS._player, NULL);
			} else {
				BF_GLOBALS.setFlag(fTalkedToDrunkInCar);
				scene->_sceneMode = 16;
				scene->setAction(&scene->_sequenceManager, scene, 5504, &BF_GLOBALS._player, &scene->_object15, NULL);
			}
			break;
		case 2:
			SceneItem::display2(550, 13);
			break;
		case 3:
			SceneItem::display2(550, 27);
			break;
		default:
			break;
		}
		return true;
	case CURSOR_TALK:
		if (_flag)
			break;
		BF_GLOBALS._player.disableControl();

		if (BF_GLOBALS.getFlag(fTalkedToDrunkInCar)) {
			scene->_sceneMode = 5508;
			scene->setAction(&scene->_sequenceManager, scene, 5508, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS.setFlag(fTalkedToDrunkInCar);
			scene->_sceneMode = 0;
			scene->setAction(&scene->_sequenceManager, scene, 5504, &BF_GLOBALS._player, &scene->_object15, NULL);
		}
		return true;
	case INV_CENTER_PUNCH:
		if (_flag) {
			SceneItem::display2(550, 14);
		} else {
			BF_GLOBALS.set2Flags(f1098Drunk);
			BF_GLOBALS.setFlag(didDrunk);
			_flag = 1;
			T2_GLOBALS._uiElements.addScore(30);

			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 5506;
			scene->setAction(&scene->_sequenceManager, scene, 5506, &BF_GLOBALS._player, this,
				&scene->_object12, NULL);
		}
		return true;
	default:
		break;
	}

	return NamedObjectExt::startAction(action, event);
}

bool Scene551::PatrolCarTrunk::startAction(CursorType action, Event &event) {
	Scene551 *scene = (Scene551 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (BF_INVENTORY.getObjectScene(INV_CENTER_PUNCH) != 1) {
			BF_GLOBALS._walkRegions.enableRegion(18);
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 2;
			scene->setAction(&scene->_sequenceManager, scene, 5503, &BF_GLOBALS._player, &scene->_harrison, this, NULL);
			return true;
		}
		break;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}

void Scene551::TrunkInset::remove() {
	Scene551 *scene = (Scene551 *)BF_GLOBALS._sceneManager._scene;
	scene->_trunkKits.remove();
	BF_GLOBALS._player.disableControl();

	scene->_sceneMode = 0;
	scene->setAction(&scene->_sequenceManager, scene, 5516, &scene->_harrison,
		&scene->_patrolCarTrunk, NULL);

	FocusObject::remove();
}

bool Scene551::TrunkInset::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(550, 18);
		return true;
	case CURSOR_USE:
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene551::TrunkKits::startAction(CursorType action, Event &event) {
	Scene551 *scene = (Scene551 *)BF_GLOBALS._sceneManager._scene;

	Rect tempRect = _bounds;
	tempRect.collapse(10, 6);
	if (!tempRect.contains(event.mousePos))
		return false;

	switch (action) {
	case CURSOR_LOOK:
		switch (_frame) {
		case 1:
			SceneItem::display2(550, 19);
			break;
		case 2:
			SceneItem::display2(550, 20);
			break;
		case 3:
			SceneItem::display2(550, 21);
			break;
		default:
			break;
		}
		return true;
	case CURSOR_USE:
		if (event.mousePos.y < _bounds.top) {
			setFrame(1);
		} else if ((event.mousePos.x < (_bounds.left - 25)) && (_frame > 1)) {
			SceneItem::display2(550, 28);
		} else {
			switch (_frame) {
			case 1:
				setFrame(BF_INVENTORY.getObjectScene(INV_CENTER_PUNCH) == 1 ? 3 : 2);
				break;
			case 2:
				if (!BF_GLOBALS.getFlag(fShowedBluePrint)) {
					T2_GLOBALS._uiElements.addScore(30);
					BF_GLOBALS.setFlag(fShowedBluePrint);
				}

				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 5520;
				scene->setAction(&scene->_sequenceManager, scene, 5520, &BF_GLOBALS._player, NULL);
				BF_INVENTORY.setObjectScene(INV_CENTER_PUNCH, 1);
				setFrame(3);
				break;
			case 3:
				setFrame(1);
				break;
			default:
				break;
			}
		}
		return true;
	case INV_CENTER_PUNCH:
		SceneItem::display2(550, 22);
		BF_INVENTORY.setObjectScene(INV_CENTER_PUNCH, 0);
		setFrame(2);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene551::Harrison::startAction(CursorType action, Event &event) {
	Scene551 *scene = (Scene551 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(550, 23);
		return true;
	case CURSOR_USE:
		SceneItem::display2(550, 24);
		return true;
	case CURSOR_TALK:
		BF_GLOBALS._player.disableControl();

		if (scene->_drunkStanding._flag == 1) {
			_flag = 5508;
		} else if (!scene->_field1CD0) {
			_flag = 5502;
		} else {
			scene->_field1CD0 = 1;
			_flag = 5500;
		}
		scene->setAction(&scene->_action2);
		return true;
	default:
		return NamedObjectExt::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene551::Item4::startAction(CursorType action, Event &event) {
	Scene551 *scene = (Scene551 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(550, 34);
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 1;
		scene->setAction(&scene->_sequenceManager, scene, 5510, &BF_GLOBALS._player, &scene->_vechile2, NULL);
		return true;
	default:
		return false;
	}
}

/*--------------------------------------------------------------------------*/

Scene551::Scene551(): Scene550() {
	_field1CD0 = _field1CD2 = 0;
}

void Scene551::synchronize(Serializer &s) {
	Scene550::synchronize(s);
	s.syncAsSint16LE(_field1CD0);
	s.syncAsSint16LE(_field1CD2);
}

void Scene551::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(550);
	BF_GLOBALS._walkRegions.load(551);
	BF_GLOBALS._sound1.fadeSound(16);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);
	_stripManager.addSpeaker(&_gigglesSpeaker);
	_stripManager.addSpeaker(&_drunkSpeaker);

	if (BF_GLOBALS.getFlag(fHasLeftDrunk)) {
		_item4.setBounds(Rect(0, 0, 320, 170));
		BF_GLOBALS._sceneItems.push_back(&_item4);
	}

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.setPosition(Common::Point(160, 100));
	BF_GLOBALS._player._moveDiff = Common::Point(2, 1);
	BF_GLOBALS._player.enableControl();

	_caravanDoor.postInit();
	_caravanDoor.setVisage(550);
	_caravanDoor.setPosition(Common::Point(34, 66));
	BF_GLOBALS._sceneItems.push_back(&_caravanDoor);

	_vechile2.postInit();
	_vechile2.setVisage(301);
	_vechile2.setStrip(1);
	_vechile2.setPosition(Common::Point(205, 77));
	_vechile2.fixPriority(70);
	_vechile2.changeZoom(80);
	BF_GLOBALS._sceneItems.push_back(&_vechile2);

	BF_GLOBALS._walkRegions.disableRegion(14);

	_drunk.postInit();
	_drunk.setVisage(550);
	_drunk.setStrip(3);
	_drunk.fixPriority(84);
	_drunk.setPosition(Common::Point(29, 92));
	_drunk._flag = 0;
	BF_GLOBALS._sceneItems.push_back(&_drunk);

	_object12.postInit();
	_object12.setVisage(550);
	_object12.setStrip(7);
	_object12.setPosition(Common::Point(29, 92));
	_object12.hide();

	_object13.postInit();
	_object13.setVisage(550);
	_object13.setStrip(2);
	_object13.setPosition(Common::Point(29, 92));
	_object13.fixPriority(82);
	_object13.setDetails(550, 8, -1, 9, 1, (SceneItem *)NULL);

	if (BF_GLOBALS.getFlag(didDrunk)) {
		_drunk._flag = 3;
		_drunk.setStrip(3);

		_object12.show();
		_object12.setDetails(550, 25, -1, 26, 1, (SceneItem *)NULL);
		BF_GLOBALS._sceneItems.push_front(&_object12);

		_harrison.postInit();
		_harrison.setVisage(304);
		_harrison.setPosition(Common::Point(67, 102));
		_harrison.setStrip(8);
		_harrison.setFrame(1);
		_harrison._flag = 1;
		BF_GLOBALS._sceneItems.push_back(&_harrison);

		_object14.postInit();
		_object14.setVisage(550);
		_object14.setStrip(5);
		_object14.animate(ANIM_MODE_2, NULL);
		_object14.fixPriority(80);
		_object14.setPosition(Common::Point(122, 57));

		_object11.postInit();
		_object11.setVisage(550);
		_object11.setStrip(2);
		_object11.setFrame(2);
		_object11.setPosition(Common::Point(116, 84));
		_object11.fixPriority(77);
		_object11.setDetails(550, 32, -1, 10, 1, (SceneItem *)NULL);

		_drunkStanding.postInit();
		_drunkStanding._flag = 0;
		_drunkStanding.setVisage(554);
		_drunkStanding.setStrip(7);
		_drunkStanding.setFrame(8);
		_drunkStanding.fixPriority(83);
		_drunkStanding.setPosition(Common::Point(57, 99));
	} else {
		_harrison.postInit();
		_harrison.setVisage(304);
		_harrison.setPosition(Common::Point(126, 83));
		_harrison.setObjectWrapper(new SceneObjectWrapper());
		_harrison._flag = 0;
		_harrison._moveDiff = Common::Point(2, 1);
		BF_GLOBALS._sceneItems.push_back(&_harrison);

		_object14.postInit();
		_object14.setVisage(550);
		_object14.setStrip(5);
		_object14.animate(ANIM_MODE_2, NULL);
		_object14.fixPriority(80);
		_object14.setPosition(Common::Point(122, 57));

		_patrolCarTrunk.postInit();
		_patrolCarTrunk.setVisage(550);
		_patrolCarTrunk.setStrip(4);
		_patrolCarTrunk.setFrame(1);
		_patrolCarTrunk.setPosition(Common::Point(149, 69));
		_patrolCarTrunk.fixPriority(79);
		_patrolCarTrunk.setDetails(550, 18, -1, 9, 1, (SceneItem *)NULL);

		_object11.postInit();
		_object11.setVisage(550);
		_object11.setStrip(2);
		_object11.setFrame(2);
		_object11.setPosition(Common::Point(116, 84));
		_object11.fixPriority(77);
		_object11.setDetails(550, 32, -1, 10, 1, (SceneItem *)NULL);

		_drunkStanding.postInit();
		_drunkStanding._flag = 0;
		_drunkStanding.setVisage(554);
		_drunkStanding.fixPriority(83);
		_drunkStanding.setPosition(Common::Point(47, 93));

		_object15.postInit();
		_object15.setVisage(552);
		_object15.setPosition(Common::Point(59, 94));
		_object15.fixPriority(200);
		_object15.hide();
	}

	BF_GLOBALS._player.disableControl();
	_sceneMode = 3;
	setAction(&_sequenceManager, this, 5502, &BF_GLOBALS._player, &_vechile2, NULL);

	_item2.setDetails(Rect(0, 26, 53, 67), 550, 1, -1, 2, 1, NULL);
	_item3.setDetails(Rect(53, 12, 173, 65), 550, 4, -1, 5, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 170), 550, 6, -1, -1, 1, NULL);
}

void Scene551::signal() {
	switch (_sceneMode) {
	case 0:
		BF_GLOBALS._player.enableControl();
		break;
	case 1:
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS.set2Flags(f1027Drunk);
		BF_GLOBALS.set2Flags(f1097Drunk);

		if (BF_GLOBALS.getFlag(didDrunk))
			BF_GLOBALS.setFlag(f1098Drunk);

		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 2:
		BF_GLOBALS._walkRegions.disableRegion(18);

		_trunkInset.postInit();
		_trunkInset.setVisage(553);
		_trunkInset.setPosition(Common::Point(59, 92));
		_trunkInset.fixPriority(252);
		BF_GLOBALS._sceneItems.push_front(&_trunkInset);

		_trunkKits.postInit();
		_trunkKits.setVisage(553);
		_trunkKits.setStrip(2);
		_trunkKits.setPosition(Common::Point(57, 73));
		_trunkKits.fixPriority(254);
		BF_GLOBALS._sceneItems.push_front(&_trunkKits);
		BF_GLOBALS._player.enableControl();
		break;
	case 3:
		if (BF_GLOBALS.getFlag(talkedToHarrisAboutDrunk)) {
			BF_GLOBALS._player.enableControl();
		} else {
			BF_GLOBALS.setFlag(talkedToHarrisAboutDrunk);
			_harrison._flag = 5500;
			setAction(&_action2);
		}
		break;
	case 5506:
		_harrison.setObjectWrapper(new SceneObjectWrapper());
		ADD_PLAYER_MOVER_NULL(_harrison, 88, 91);

		_object12.show();
		_object12.setDetails(550, 25, -1, 26, 1, (SceneItem *)NULL);
		BF_GLOBALS._sceneItems.push_front(&_object12);
		BF_GLOBALS._player.enableControl();

		_sceneMode = 5507;
		setAction(&_sequenceManager, this, 5507, &BF_GLOBALS._player, &_drunk, &_drunkStanding, NULL);
		break;
	case 5507:
		BF_GLOBALS._walkRegions.disableRegion(2);
		BF_GLOBALS._walkRegions.disableRegion(4);

		_drunkStanding._flag = 1;
		BF_GLOBALS._sceneItems.push_front(&_drunkStanding);
		BF_GLOBALS._player.enableControl();
		break;
	case 5508:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager, this, 5505, &BF_GLOBALS._player, &_object15, NULL);
		break;
	case 5509:
		BF_INVENTORY.setObjectScene(INV_CENTER_PUNCH, 0);
		BF_GLOBALS._player.disableControl();
		_sceneMode = 5510;
		setAction(&_sequenceManager, this, 5510, &BF_GLOBALS._player, &_vechile2, NULL);
		break;
	case 5510:
		BF_GLOBALS._bookmark = bCalledToDrunkStop;
		if (BF_GLOBALS.getFlag(didDrunk))
			BF_GLOBALS.setFlag(fHasLeftDrunk);

		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 5520:
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		_harrison.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player.enableControl();
		break;
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene551::dispatch() {
	SceneExt::dispatch();

	if ((_drunkStanding._flag != 2) && !_harrison._mover && !_field1CD2)
		_harrison.updateAngle(BF_GLOBALS._player._position);
}

/*--------------------------------------------------------------------------
 * Scene 550 - Study
 *
 *--------------------------------------------------------------------------*/

void Scene560::Action1::signal() {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		 ADD_MOVER(BF_GLOBALS._player, 105, 140);
		 break;
	case 2:
		scene->_deskChair.hide();

		BF_GLOBALS._player.changeZoom(81);
		BF_GLOBALS._player.setVisage(561);
		BF_GLOBALS._player.setStrip(2);
		BF_GLOBALS._player.setFrame(1);
		BF_GLOBALS._player.setPosition(Common::Point(96, 138));
		BF_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_deskChair.setVisage(561);
		scene->_deskChair.setFrame(BF_GLOBALS._player._frame);
		scene->_deskChair.setStrip(BF_GLOBALS._player._strip);
		scene->_deskChair.setPosition(BF_GLOBALS._player._position);

		scene->_field380 = true;
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene560::Action2::signal() {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(10);
		break;
	case 1:
		scene->_deskChair.hide();

		BF_GLOBALS._player.setVisage(561);
		BF_GLOBALS._player.setStrip(2);
		BF_GLOBALS._player.setFrame(BF_GLOBALS._player.getFrameCount());
		BF_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 2:
		scene->_field380 = false;
		scene->_deskChair.setPosition(Common::Point(81, 149));
		scene->_deskChair.setVisage(561);
		scene->_deskChair.setStrip(3);
		scene->_deskChair.setFrame(1);
		scene->_deskChair.fixPriority(151);
		scene->_deskChair.show();

		BF_GLOBALS._player.setVisage(563);
		BF_GLOBALS._player.setPosition(Common::Point(105, 140));
		BF_GLOBALS._player.fixPriority(-1);
		BF_GLOBALS._player.changeZoom(-1);
		BF_GLOBALS._player.animate(ANIM_MODE_1, this);
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player.enableControl();
		ADD_PLAYER_MOVER(scene->_destPosition.x, scene->_destPosition.y);
		remove();
		break;
	default:
		break;
	}
}

void Scene560::Action3::signal() {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(10);
		break;
	case 1:
		if (scene->_field380) {
			setDelay(10);
		} else {
			setAction(&scene->_action1, this);
		}
		break;
	case 2:
		BF_GLOBALS._sceneManager.changeScene(570);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene560::PicturePart::startAction(CursorType action, Event &event) {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (scene->_field380) {
			SceneItem::display2(560, 54);
		} else if ((scene->_sceneMode != 4) && (scene->_sceneMode != 3)) {
			scene->_sceneMode = _flag + 4;
			PlayerMover *mover = new PlayerMover();
			Common::Point destPos(139, 106);
			BF_GLOBALS._player.addMover(mover, &destPos, scene);
		}
		return true;
	default:
		return NamedHotspotExt::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene560::DeskChair::startAction(CursorType action, Event &event) {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && !scene->_field380) {
		scene->setAction(&scene->_action1);
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

bool Scene560::Box::startAction(CursorType action, Event &event) {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (scene->_field380) {
			SceneItem::display2(560, 54);
		} else {
			scene->_sceneMode = 9;
			Common::Point destPos(289, 108);
			PlayerMover *mover = new PlayerMover();
			BF_GLOBALS._player.addMover(mover, &destPos, scene);
		}
		return true;
	default:
		return NamedObjectExt::startAction(action, event);
	}
}

void Scene560::SafeInset::postInit(SceneObjectList *OwnerList) {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;
	FocusObject::postInit();

	_cursorVisage.setVisage(1, 5);

	if (BF_GLOBALS._safeCombination == 172) {
		setFrame(2);
		if (BF_INVENTORY.getObjectScene(INV_NICKEL) == 560) {
			scene->_nickel.postInit();
			scene->_nickel.setVisage(560);
			scene->_nickel.setStrip(2);
			scene->_nickel.setFrame(3);
			scene->_nickel.fixPriority(252);
			scene->_nickel.setPosition(Common::Point(181, 140));
			scene->_nickel.setDetails(560, 47, 48, -1, 1, (SceneItem *)NULL);
			BF_GLOBALS._sceneItems.remove(&scene->_nickel);
			BF_GLOBALS._sceneItems.push_front(&scene->_nickel);
		}
	} else {
		setFrame(1);
		_item1.setDetails(Rect(143, 68, 159, 85), 560, 49, 50, -1, 1, NULL);
		_item2.setDetails(Rect(159, 68, 175, 85), 560, 49, 50, -1, 1, NULL);
		_item3.setDetails(Rect(175, 68, 191, 85), 560, 49, 50, -1, 1, NULL);
		_item4.setDetails(Rect(143, 86, 159, 102), 560, 49, 50, -1, 1, NULL);
		_item5.setDetails(Rect(159, 86, 175, 102), 560, 49, 50, -1, 1, NULL);
		_item6.setDetails(Rect(175, 86, 191, 102), 560, 49, 50, -1, 1, NULL);

		BF_GLOBALS._sceneItems.remove(&_item1);
		BF_GLOBALS._sceneItems.remove(&_item2);
		BF_GLOBALS._sceneItems.remove(&_item3);
		BF_GLOBALS._sceneItems.remove(&_item4);
		BF_GLOBALS._sceneItems.remove(&_item5);
		BF_GLOBALS._sceneItems.remove(&_item6);
		BF_GLOBALS._sceneItems.push_front(&_item6);
		BF_GLOBALS._sceneItems.push_front(&_item5);
		BF_GLOBALS._sceneItems.push_front(&_item4);
		BF_GLOBALS._sceneItems.push_front(&_item3);
		BF_GLOBALS._sceneItems.push_front(&_item2);
		BF_GLOBALS._sceneItems.push_front(&_item1);

		_item1._flag = 1;
		_item2._flag = 2;
		_item3._flag = 3;
		_item4._flag = 4;
		_item5._flag = 5;
		_item6._flag = 6;

		_digit2.postInit();
		_digit2.setVisage(560);
		_digit2.setStrip(3);
		_digit2.setPosition(Common::Point(151, 94));
		_digit2.fixPriority(252);

		_digit1.postInit();
		_digit1.setVisage(560);
		_digit1.setStrip(3);
		_digit1.setPosition(Common::Point(167, 94));
		_digit1.fixPriority(252);

		_digit0.postInit();
		_digit0.setVisage(560);
		_digit0.setStrip(3);
		_digit0.setPosition(Common::Point(183, 94));
		_digit0.fixPriority(252);

		int amount = (BF_GLOBALS._safeCombination != 0) ? BF_GLOBALS._safeCombination : 1000;

		// Get digit 0 portion
		int remainder = amount % 10;
		amount /= 10;
		_digit0.setFrame(!remainder ? 10 : remainder);

		// Get digit 1 portion
		remainder = amount % 10;
		amount /= 10;
		_digit1.setFrame(!remainder ? 10 : remainder);

		// Get digit 2 portion
		remainder = amount % 10;
		_digit2.setFrame(!remainder ? 10 : remainder);
	}
}

void Scene560::SafeInset::remove() {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	_item1.remove();
	_item2.remove();
	_item3.remove();
	_item4.remove();
	_item5.remove();
	_item6.remove();
	_digit2.remove();
	_digit1.remove();
	_digit0.remove();

	scene->_nickel.remove();

	if (BF_GLOBALS._events.getCursor() == CURSOR_USE) {
		GfxSurface cursor = _cursorVisage.getFrame(2);
		BF_GLOBALS._events.setCursor(cursor);
	}

	FocusObject::remove();
}

void Scene560::SafeInset::signal() {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	// Recalculate the new total
	int total = 0;
	if (_digit2._frame < 10)
		total = _digit2._frame * 100;
	if (_digit1._frame < 10)
		total += _digit1._frame * 10;
	if (_digit0._frame < 10)
		total += _digit0._frame;

	BF_GLOBALS._safeCombination = !total ? 1000 : total;

	// Check if the combination is correct
	if (total == 172) {
		BF_GLOBALS._sceneObjects->draw();

		if (!BF_GLOBALS.getFlag(fGotPointsForBank)) {
			T2_GLOBALS._uiElements.addScore(50);
			BF_GLOBALS.setFlag(fGotPointsForBank);
		}

		setFrame(2);
		GfxSurface cursor = _cursorVisage.getFrame(2);
		BF_GLOBALS._events.setCursor(cursor);

		_item1.remove();
		_item2.remove();
		_item3.remove();
		_item4.remove();
		_item5.remove();
		_item6.remove();
		_digit2.remove();
		_digit1.remove();
		_digit0.remove();

		if (BF_INVENTORY.getObjectScene(INV_NICKEL) == 560) {
			// Nickel is still in the safe, show it
			scene->_nickel.postInit();
			scene->_nickel.setVisage(560);
			scene->_nickel.setStrip(2);
			scene->_nickel.setFrame(3);
			scene->_nickel.fixPriority(252);
			scene->_nickel.setPosition(Common::Point(181, 140));
			scene->_nickel.setDetails(560, 47, 48, -1, 1, (SceneItem *)NULL);
			BF_GLOBALS._sceneItems.remove(&scene->_nickel);
			BF_GLOBALS._sceneItems.push_front(&scene->_nickel);
		}
	}
}
void Scene560::SafeInset::process(Event &event) {
	if (_bounds.contains(event.mousePos)) {
		CursorType cursorId = BF_GLOBALS._events.getCursor();
		if (cursorId == CURSOR_USE) {
			// Instead of standard cursor, use special hand cursor
			GfxSurface cursor = _cursorVisage.getFrame(6);
			BF_GLOBALS._events.setCursor(cursor);
		} else {
			// Set cursor again just in case Exit cursor was showing
			BF_GLOBALS._events.setCursor(cursorId);
		}

		if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == CURSOR_WALK) &&
				(event.btnState == 3)) {
			BF_GLOBALS._events.setCursor(CURSOR_USE);
			event.handled = true;
		}
	} else {
		GfxSurface cursor = _cursorVisage.getFrame(7);
		BF_GLOBALS._events.setCursor(cursor);

		if ((event.mousePos.y < UI_INTERFACE_Y) && (event.eventType == EVENT_BUTTON_DOWN)) {
			// Leave the safe view
			event.handled = true;
			remove();
		}
	}
}

bool Scene560::SafeInset::Item::startAction(CursorType action, Event &event) {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		switch (_flag) {
		case 1: {
			int newFrame = scene->_safeInset._digit2._frame + 1;
			if (newFrame == 11)
				newFrame = 1;

			scene->_safeInset._digit2.setFrame(newFrame);
			break;
		}
		case 2: {
			int newFrame = scene->_safeInset._digit1._frame + 1;
			if (newFrame == 11)
				newFrame = 1;

			scene->_safeInset._digit1.setFrame(newFrame);
			break;
		}
		case 3: {
			int newFrame = scene->_safeInset._digit0._frame + 1;
			if (newFrame == 11)
				newFrame = 1;

			scene->_safeInset._digit0.setFrame(newFrame);
			break;
		}
		case 4: {
			int newFrame = scene->_safeInset._digit2._frame - 1;
			if (newFrame == 0)
				newFrame = 10;

			scene->_safeInset._digit2.setFrame(newFrame);
			break;
		}
		case 5: {
			int newFrame = scene->_safeInset._digit1._frame - 1;
			if (newFrame == 0)
				newFrame = 10;

			scene->_safeInset._digit1.setFrame(newFrame);
			break;
		}
		case 6: {
			int newFrame = scene->_safeInset._digit0._frame - 1;
			if (newFrame == 0)
				newFrame = 10;

			scene->_safeInset._digit0.setFrame(newFrame);
			break;
		}
		default:
			break;
		}

		scene->_safeInset.signal();
		scene->_sound1.play(75);
		return true;
	default:
		return NamedHotspotExt::startAction(action, event);
	}
}

bool Scene560::Nickel::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		BF_INVENTORY.setObjectScene(INV_NICKEL, 1);
		T2_GLOBALS._uiElements.addScore(10);
		remove();
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

void Scene560::BoxInset::postInit(SceneObjectList *OwnerList) {
	FocusObject::postInit();
	_item1.setDetails(Rect(110, 48, 189, 102), 560, 43, 44, -1, 1, NULL);
	BF_GLOBALS._sceneItems.remove(&_item1);
	BF_GLOBALS._sceneItems.push_front(&_item1);
}

void Scene560::BoxInset::remove() {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	if (scene->_sceneMode != 3)
		scene->_sceneMode = 0;

	_item1.remove();
	FocusObject::remove();
}

bool Scene560::BoxInset::Item1::startAction(CursorType action, Event &event) {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		if (!BF_GLOBALS.getFlag(fGotPointsForBox)) {
			T2_GLOBALS._uiElements.addScore(10);
			BF_GLOBALS.setFlag(fGotPointsForBox);
		}

		scene->_safeInset.postInit();
		scene->_safeInset.setVisage(560);
		scene->_safeInset.setStrip(2);
		scene->_safeInset.setPosition(Common::Point(160, 141));
		scene->_safeInset.fixPriority(251);
		scene->_safeInset.setDetails(560, 45, 46, -1);

		scene->_sceneMode = 3;
		scene->_boxInset.remove();

		GfxSurface cursor = surfaceFromRes(1, 5, 6);
		BF_GLOBALS._events.setCursor(cursor);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene560::Computer::startAction(CursorType action, Event &event) {
	Scene560 *scene = (Scene560 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (BF_GLOBALS._bookmark < bTalkedToGrannyAboutSkipsCard) {
			SceneItem::display2(666, 24);
		} else {
			scene->setAction(&scene->_action3);
		}
		return true;
	default:
		return NamedHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

Scene560::Scene560(): SceneExt() {
	_field380 = _field11EA = false;
}

void Scene560::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(560);
	setZoomPercents(100, 60, 168, 90);
	_stripManager.addSpeaker(&_gameTextSpeaker);

	_picture1.setDetails(Rect(121, 16, 139, 35), 560, -1, -1, -1, 1, NULL);
	_picture2.setDetails(Rect(139, 16, 157, 35), 560, -1, -1, -1, 1, NULL);
	_picture3.setDetails(Rect(121, 35, 139, 54), 560, -1, -1, -1, 1, NULL);
	_picture4.setDetails(Rect(139, 35, 157, 54), 560, -1, -1, -1, 1, NULL);
	_picture1._flag = 3;
	_picture2._flag = 4;
	_picture3._flag = 2;
	_picture4._flag = 1;

	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 3;

	if (BF_GLOBALS._bookmark >= bTalkedToGrannyAboutSkipsCard) {
		_box.postInit();
		_box.setVisage(560);
		_box.setStrip(4);
		_box.setFrame(1);
		_box.setPosition(Common::Point(295, 37));
		_box.setDetails(560, 41, 42, -1, 1, (SceneItem *)NULL);
	}

	_deskChair.postInit();
	_deskChair.setVisage(561);
	_deskChair.setStrip(3);
	_deskChair.setPosition(Common::Point(81, 149));
	_deskChair.fixPriority(151);
	_deskChair.changeZoom(81);

	if (BF_GLOBALS._sceneManager._previousScene == 570) {
		// Returning from using computer
		BF_GLOBALS._events.setCursor(CURSOR_USE);

		_deskChair.hide();

		BF_GLOBALS._player.postInit();
		BF_GLOBALS._player.setVisage(561);
		BF_GLOBALS._player.setStrip(2);
		BF_GLOBALS._player.setFrame(BF_GLOBALS._player.getFrameCount());
		BF_GLOBALS._player._moveDiff.x = 11;
		BF_GLOBALS._player.setPosition(Common::Point(96, 138));
		BF_GLOBALS._player.changeZoom(81);

		_deskChair.setFrame(BF_GLOBALS._player._frame);
		_deskChair.setStrip(BF_GLOBALS._player._strip);
		_deskChair.setPosition(BF_GLOBALS._player._position);

		_field11EA = false;
		_field380 = true;
	} else {
		// Entering study through doorway
		_field11EA = false;
		_field380 = false;

		BF_GLOBALS._player.postInit();
		BF_GLOBALS._player.setVisage(563);
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player.setPosition(Common::Point(79, 110));
		BF_GLOBALS._player._moveDiff.x = 11;
		BF_GLOBALS._player.changeZoom(-1);
		BF_GLOBALS._player.disableControl();

		_sceneMode = 10;
		ADD_MOVER(BF_GLOBALS._player, 85, 115);
	}

	_computer.setDetails(Rect(16, 77, 58, 107), 560, 2, -1, -1, 1, NULL);
	_deskChair.setDetails(560, 3, -1, -1, 1, (SceneItem *)NULL);
	_chair.setDetails(Rect(163, 64, 196, 102), 560, 13, 25, 36, 1, NULL);
	_lamp.setDetails(Rect(197, 43, 214, 56), 560, 7, 19, 30, 1, NULL);
	_item4.setDetails(Rect(121, 18, 156, 54), 560, 8, 20, 31, 1, NULL);
	_trophy.setDetails(Rect(259, 52, 275, 63), 560, 10, 22, 33, 1, NULL);
	_watercolours.setDetails(Rect(214, 48, 239, 64), 560, 12, 24, 35, 1, NULL);
	_fileCabinets.setDetails(Rect(0, 47, 49, 100), 560, 14, 26, 37, 1, NULL);
	_certificate.setDetails(Rect(280, 51, 292, 62), 560, 11, 23, 34, 1, NULL);
	_bookcase.setDetails(Rect(176, 0, 319, 103), 560, 9, 21, 32, 1, NULL);
	_desk.setDetails(1, 560, 6, 10, 29, 1);
	_item12.setDetails(Rect(62, 16, 96, 101), 560, 51, 52, 53, 1, NULL);
	_carpet.setDetails(Rect(0, 103, 319, 167), 560, 5, 16, 28, 1, NULL);
	_office.setDetails(Rect(0, 0, 320, 168), 560, 4, -1, -1, 1, NULL);

	BF_GLOBALS._player.enableControl();
}

void Scene560::signal() {
	switch (_sceneMode) {
	case 1:
		_object6.hide();
		_object6.remove();
		BF_GLOBALS._player.enableControl();
		break;
	case 5:
	case 6:
	case 7:
	case 8:
		BF_GLOBALS._player._strip = 4;
		BF_GLOBALS._player._frame = 1;
		BF_GLOBALS._player.disableControl();

		_object6.postInit();
		_object6.setVisage(560);
		_object6.setStrip(1);
		_object6.setFrame(_sceneMode - 4);
		_object6.setPosition(Common::Point(160, 141));
		_object6.fixPriority(250);
		_stripManager.start(_sceneMode + 5595, this);
		_sceneMode = 1;
		break;
	case 9:
		// Clicked on the Baseball Cards Box
		_object6._strip = 4;
		_object6._frame = 1;

		if (BF_GLOBALS._safeCombination == 172) {
			if (_sceneMode != 3) {
				_safeInset.postInit();
				_safeInset.setVisage(560);
				_safeInset.setStrip(2);
				_safeInset.setPosition(Common::Point(160, 141));
				_safeInset.fixPriority(251);
				_safeInset.setDetails(560, 45, 46, -1);

				_sceneMode = 3;
			}
		} else if ((_sceneMode != 3) && (_sceneMode != 4)) {
			if (!BF_GLOBALS.getFlag(fGotPointsForPunch)) {
				T2_GLOBALS._uiElements.addScore(10);
				BF_GLOBALS.setFlag(fGotPointsForPunch);
			}

			_boxInset.postInit();
			_boxInset.setVisage(560);
			_boxInset.setStrip(2);
			_boxInset.setFrame(4);
			_boxInset.setPosition(Common::Point(160, 141));
			_boxInset.fixPriority(251);
			_boxInset.setDetails(560, 43, 44, -1);

			_sceneMode = 4;
		}
		break;
	case 10:
		_field11EA = false;
		BF_GLOBALS._player.enableControl();
		break;
	case 11:
		BF_GLOBALS._sceneManager.changeScene(270);
		break;
	}
}

void Scene560::process(Event &event) {
	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == CURSOR_WALK) &&
			(_field380) && !_action) {
		_destPosition = event.mousePos;
		BF_GLOBALS._player.disableControl();
		setAction(&_action2);

		event.handled = true;
	}

	SceneExt::process(event);
}

void Scene560::dispatch() {
	if (!_field11EA && (BF_GLOBALS._player._position.y < 105)) {
		_field11EA = true;
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._sceneManager.changeScene(270);
	}

	SceneExt::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 570 - Computer
 *
 *--------------------------------------------------------------------------*/

Scene570::PasswordEntry::PasswordEntry(): EventHandler() {
	_passwordStr = SCENE570_PASSWORD;
}

void Scene570::PasswordEntry::synchronize(Serializer &s) {
	EventHandler::synchronize(s);
	s.syncString(_entryBuffer);
}

void Scene570::PasswordEntry::postInit(SceneObjectList *OwnerList) {
	Scene570 *scene = (Scene570 *)BF_GLOBALS._sceneManager._scene;

	scene->_sceneMode = 10;
	scene->_object3.setStrip(6);
	scene->_object3.setFrame(1);
	scene->_object3.fixPriority(3);

	_passwordText._color1 = 22;
	_passwordText._color2 = 9;
	_passwordText._color3 = 9;
	_passwordText._width = 128;
	_passwordText._fontNumber = 9000;
	_passwordText.setPosition(Common::Point(165, 40));
	_passwordText.fixPriority(255);
	_passwordText.setup(_passwordStr);

	_entryText._color1 = 22;
	_entryText._color2 = 9;
	_entryText._color3 = 9;
	_entryText._width = 128;
	_entryText._fontNumber = 9000;
	_entryText.setPosition(Common::Point(220, 40));
	_entryText.fixPriority(255);
	_entryText.setup(_entryBuffer);
}

void Scene570::PasswordEntry::process(Event &event) {
	Scene570 *scene = (Scene570 *)BF_GLOBALS._sceneManager._scene;
	bool entryChanged = false;

	switch (event.eventType) {
	case EVENT_KEYPRESS: {
		int key = toupper(event.kbd.ascii);
		scene->_sound1.play(72);

		if ((event.kbd.keycode == Common::KEYCODE_BACKSPACE) || (event.kbd.keycode == Common::KEYCODE_DELETE)) {
			// Delete a key from the entry
			if (_entryBuffer.size() > 0)
				_entryBuffer.deleteLastChar();
			entryChanged = true;
		} else if (event.kbd.keycode == Common::KEYCODE_RETURN) {
			// Finished entering password
			_passwordText.remove();
			_entryText.remove();

			checkPassword();
			remove();
		} else if ((key >= 32) || (key <= 126)) {
			// Valid character pressed
			if (_entryBuffer.size() < 10)
				_entryBuffer += (char)key;
			event.handled = true;
			entryChanged = true;
		}
		break;
	}
	case EVENT_BUTTON_DOWN:
		event.handled = true;
		break;
	default:
		break;
	}

	if (entryChanged) {
		_entryText._color1 = 22;
		_entryText._color2 = 9;
		_entryText._color3 = 9;
		_entryText._width = 128;
		_entryText._fontNumber = 9000;
		_entryText.setPosition(Common::Point(213, 40));
		_entryText.fixPriority(255);
		_entryText.setup(_entryBuffer);

		// Pad entered text with spaces to make up the allowed width and then display
		Common::String msg = _entryBuffer;
		while (msg.size() < 10)
			msg += " ";
		_entryText.setup(msg);
	}
}

void Scene570::PasswordEntry::checkPassword() {
	// Check if the password is correctly entered as 'JACKIE' or, as a nod to the
	// reimplementation in ScummVM, as the project name.
	Scene570 *scene = (Scene570 *)BF_GLOBALS._sceneManager._scene;

	if (!_entryBuffer.compareTo("JACKIE") || !_entryBuffer.compareTo("SCUMMVM")) {
		// Password was correct
		T2_GLOBALS._uiElements.addScore(30);
		BF_GLOBALS._player.disableControl();
		scene->_sound1.play(73);

		T2_GLOBALS._uiElements.hide();
		T2_GLOBALS._uiElements._active = false;
		scene->_sceneMode = 5701;
		scene->setAction(&scene->_sequenceManager, scene, 5701, &scene->_object3, NULL);
	} else {
		// Password was incorrect
		_entryBuffer = "";

		scene->_object3.fixPriority(1);
		scene->_iconManager.refreshList();
		BF_GLOBALS._events.setCursor(CURSOR_USE);
		scene->_sceneMode = 0;
	}
}

Scene570::IconManager::IconManager(): EventHandler() {
	_mode = _selectedFolder = _fieldAA = _fieldAC = 0;
}

void Scene570::IconManager::remove() {
	_object1.remove();
	EventHandler::remove();
}

void Scene570::IconManager::setup(int mode) {
	_mode = mode;
	_selectedFolder = 0;

	_object1.postInit();
	_object1.setVisage(572);
	_object1.fixPriority(2);
	_object1.setFrame((mode == 1) ? 4 : 5);
	EventHandler::postInit();
}

void Scene570::IconManager::hideList() {
	SynchronizedList<Icon *>::iterator i;
	for (i = _list.begin(); i != _list.end(); ++i) {
		(*i)->_sceneText.remove();
	}
}

void Scene570::IconManager::refreshList() {
	Scene570 *scene = (Scene570 *)BF_GLOBALS._sceneManager._scene;

	_object1.setPosition(Common::Point(163, 19));
	scene->_object3.setStrip(4);

	// Clear any current icons
	SynchronizedList<Icon *>::iterator i;
	for (i = _list.begin(); i != _list.end(); ++i) {
		Icon *item = *i;

		item->setVisage(572);
		item->setStrip(1);
		item->fixPriority(2);
		item->setPosition(Common::Point(330, 100));
		item->_sceneText.remove();
	}

	// Refresh the list
	int iconIndex = 0, folderIndex = 0;
	for (i = _list.begin(); i != _list.end(); ++i) {
		Icon *item = *i;

		if (item->_iconId == 1) {
			// Folder
			int parentId = item->_parentFolderId;
			item->setFrame((_selectedFolder == (item->_folderId - 1)) ? 1 : 8);
			item->setPosition(Common::Point(168 + parentId * 11, folderIndex * 8 + 27));
			item->_sceneText.setPosition(Common::Point(175 + parentId * 11, folderIndex * 8 + 21));
			item->_sceneText.setup(item->_text);
			++folderIndex;
		} else if (item->_parentFolderId == _selectedFolder) {
			item->setPosition(Common::Point(229, 27 + iconIndex * 12));

			switch (item->_iconId) {
			case 2:
				item->setFrame(9);
				break;
			case 3:
				item->setFrame(7);
				break;
			case 5:
				item->setFrame(10);
				break;
			case 6:
				item->setFrame(11);
				break;
			case 7:
				item->setFrame(12);
				break;
			default:
				break;
			}

			item->_sceneText.setPosition(Common::Point(236, iconIndex * 12 + 22));
			item->_sceneText.setup(item->_text);
			++iconIndex;
		}
	}
}

void Scene570::IconManager::addItem(Icon *item) {
	item->_mode = _mode;
	_list.push_back(item);
}

Scene570::Icon::Icon(): NamedObject() {
	_iconId = _folderId = 0;
}

void Scene570::Icon::synchronize(Serializer &s) {
	NamedObject::synchronize(s);
	s.syncAsSint16LE(_iconId);
	s.syncAsSint16LE(_folderId);
	s.syncAsSint16LE(_parentFolderId);
	s.syncAsSint16LE(_mode);
}

void Scene570::Icon::remove() {
	_sceneText.remove();
	NamedObject::remove();
}

bool Scene570::Icon::startAction(CursorType action, Event &event) {
	Scene570 *scene = (Scene570 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		switch (_iconId) {
		case 1:
			SceneItem::display2(570, 9);
			break;
		case 2:
			SceneItem::display2(570, 10);
			break;
		case 3:
			SceneItem::display2(570, 4);
			break;
		case 5:
			SceneItem::display2(570, 11);
			break;
		case 6:
			SceneItem::display2(570, 12);
			break;
		case 7:
			SceneItem::display2(570, 13);
			break;
		default:
			break;
		}
		return true;
	case CURSOR_USE:
		// Select the given icon
		scene->_sound1.play(73);
		switch (_iconId) {
		case 1:
			// Folder, so select it
			scene->_iconManager._selectedFolder = _folderId - 1;
			scene->_iconManager.refreshList();
			break;
		case 2:
			scene->_iconManager.hideList();
			scene->_sceneMode = 5702;
			scene->setAction(&scene->_sequenceManager, scene, 5702, &scene->_object3, NULL);
			break;
		case 3:
			scene->_iconManager.hideList();
			scene->_passwordEntry.postInit();
			break;
		case 5:
			SceneItem::display2(570, 5);
			break;
		case 6:
			scene->_iconManager.hideList();
			switch (_folderId) {
			case 8:
				T2_GLOBALS._uiElements.hide();
				T2_GLOBALS._uiElements._active = false;
				scene->_sceneMode = 5705;
				scene->setAction(&scene->_sequenceManager, scene, 5705, &scene->_object3, NULL);
				break;
			case 10:
				T2_GLOBALS._uiElements.hide();
				T2_GLOBALS._uiElements._active = false;
				scene->_sceneMode = 5706;
				scene->setAction(&scene->_sequenceManager, scene, 5706, &scene->_object3, NULL);
				break;
			case 12:
				T2_GLOBALS._uiElements.hide();
				T2_GLOBALS._uiElements._active = false;
				scene->_sceneMode = 5707;
				scene->setAction(&scene->_sequenceManager, scene, 5707, &scene->_object3, NULL);
				break;
			default:
				break;
			}
			break;
		case 7:
			scene->_iconManager.hideList();
			T2_GLOBALS._uiElements.hide();
			T2_GLOBALS._uiElements._active = false;
			scene->_sceneMode = 5704;
			scene->setAction(&scene->_sequenceManager, scene, 5704, &scene->_object3, NULL);
			break;
		}
		return true;
	case CURSOR_TALK:
		SceneItem::display2(570, 15);
		return true;
	case CURSOR_PRINTER:
		switch (_iconId) {
		case 1:
			// Folder - "You can't print that"
			SceneItem::display2(570, 8);
			break;
		case 7:
			scene->_sound1.play(74);
			if (BF_INVENTORY.getObjectScene(INV_PRINT_OUT) == 570) {
				SceneItem::display2(570, 6);
				T2_GLOBALS._uiElements.addScore(30);
				BF_INVENTORY.setObjectScene(INV_PRINT_OUT, 1);
			} else {
				SceneItem::display2(570, 7);
			}
			break;
		default:
			// You don't want to print that
			SceneItem::display2(570, 18);
			break;
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

void Scene570::Icon::setDetails(int iconId, int folderId, int parentFolderId, int unused, const Common::String &msg) {
	Scene570 *scene = (Scene570 *)BF_GLOBALS._sceneManager._scene;
	NamedObject::postInit();

	_iconId = iconId;
	_folderId = folderId;
	_parentFolderId = parentFolderId;
	_text = msg;

	_sceneText._color1 = 22;
	_sceneText._color2 = 9;
	_sceneText._color3 = 9;
	_sceneText._width = 128;
	_sceneText._fontNumber = 9000;
	_sceneText.fixPriority(2);

	BF_GLOBALS._sceneItems.push_front(this);
	scene->_iconManager.addItem(this);
}

/*--------------------------------------------------------------------------*/

bool Scene570::PowerSwitch::startAction(CursorType action, Event &event) {
	Scene570 *scene = (Scene570 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (scene->_object4._flag == 1) {
			setFrame(1);
			scene->_object3.remove();
		} else {
			if (!BF_GLOBALS.getFlag(fGotPointsForCoin)) {
				T2_GLOBALS._uiElements.addScore(10);
				BF_GLOBALS.setFlag(fGotPointsForCoin);
			}

			scene->_sound1.play(70);
			scene->_object4._flag = 1;
			setFrame(2);

			scene->_object3.postInit();
			scene->_object3.fixPriority(1);
			scene->_object3.setDetails(570, 16, 15, 17);
			BF_GLOBALS._sceneItems.remove(&scene->_object3);
			BF_GLOBALS._sceneItems.push_front(&scene->_object3);

			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 5700;
			setAction(&scene->_sequenceManager, scene, 5700, &scene->_object3, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene570::PrinterIcon::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		BF_GLOBALS._events.setCursor(CURSOR_PRINTER);
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

void Scene570::Object3::remove() {
	Scene570 *scene = (Scene570 *)BF_GLOBALS._sceneManager._scene;
	scene->_object4._flag = 0;

	scene->_printerIcon.remove();
	scene->_iconManager.remove();
	scene->_folder1.remove();
	scene->_folder2.remove();
	scene->_folder3.remove();
	scene->_folder4.remove();
	scene->_icon1.remove();
	scene->_icon2.remove();
	scene->_icon3.remove();
	scene->_icon4.remove();
	scene->_icon5.remove();
	scene->_icon6.remove();
	scene->_icon7.remove();
	scene->_icon8.remove();
	scene->_icon9.remove();

	FocusObject::remove();
	BF_GLOBALS._sceneManager.changeScene(560);
}

/*--------------------------------------------------------------------------*/

bool Scene570::FloppyDrive::startAction(CursorType action, Event &event) {
	Scene570 *scene = (Scene570 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (BF_INVENTORY.getObjectScene(INV_D_FLOPPY) == 571) {
			BF_INVENTORY.setObjectScene(INV_D_FLOPPY, 1);
			scene->_iconManager.refreshList();
			SceneItem::display2(570, 2);
		} else {
			SceneItem::display2(570, 3);
		}
		return true;
	case INV_D_FLOPPY:
		BF_INVENTORY.setObjectScene(INV_D_FLOPPY, 571);
		scene->_iconManager.refreshList();
		return true;
	default:
		return NamedHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene570::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(570);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 1;

	_object4._flag = 0;
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.enableControl();
	BF_GLOBALS._player.hide();

	_powerSwitch.postInit();
	_powerSwitch.setVisage(570);
	_powerSwitch.setStrip(4);
	_powerSwitch.setFrame(1);
	_powerSwitch.setPosition(Common::Point(163, 131));
	_powerSwitch.setDetails(570, 1, 15, -1, 1, (SceneItem *)NULL);

	_floppyDrive.setDetails(Rect(258, 111, 303, 120), 570, 0, 15, -1, 1, NULL);
	_item11.setDetails(0, 570, 15, 15, 15, 1);
	_monitor.setDetails(1, 570, 19, 20, 21, 1);
	_item3.setDetails(2, 570, 22, 23, 24, 1);
	_case.setDetails(3, 570, 25, 26, 27, 1);
	_keyboard.setDetails(4, 570, 28, 29, 30, 1);
	_desk.setDetails(5, 570, 31, 32, 33, 1);
	_printer.setDetails(7, 570, 37, 38, 39, 1);
	_window.setDetails(8, 570, 40, 41, 42, 1);
	_plant.setDetails(9, 570, 43, 44, 45, 1);

	if ((BF_GLOBALS._dayNumber == 1) && (BF_INVENTORY.getObjectScene(INV_CRATE1) == 1)) {
		_object4.postInit();
		_object4.setVisage(574);
		_object4.setPosition(Common::Point(90, 84));
	}
}

void Scene570::signal() {
	switch (_sceneMode) {
	case 5700:
		_object3.setStrip(4);
		_object3.setFrame(1);

		_printerIcon.postInit();
		_printerIcon.setVisage(572);
		_printerIcon.setFrame(3);
		_printerIcon.setPosition(Common::Point(172, 71));
		_printerIcon.fixPriority(2);
		_printerIcon.setDetails(570, 14, 15, -1, 2, (SceneItem *)NULL);

		_iconManager.setup(2);
		_folder1.setDetails(1, 1, 0, 2, SCENE570_C_DRIVE);
		_folder2.setDetails(1, 2, 1, 2, SCENE570_RING);
		_folder3.setDetails(1, 3, 1, 2, SCENE570_PROTO);
		_folder4.setDetails(1, 4, 1, 2, SCENE570_WACKY);

		if (!BF_GLOBALS.getFlag(fDecryptedBluePrints))
			_icon1.setDetails(3, 5, 0, 2, SCENE570_COBB);
		_icon2.setDetails(2, 7, 0, 2, SCENE570_LETTER);
		if (BF_GLOBALS.getFlag(fDecryptedBluePrints))
			_icon3.setDetails(7, 6, 0, 2, SCENE570_COBB);

		_icon4.setDetails(6, 8, 1, 2, SCENE570_RINGEXE);
		_icon5.setDetails(5, 9, 1, 2, SCENE570_RINGDATA);
		_icon6.setDetails(6, 10, 2, 2, SCENE570_PROTOEXE);
		_icon7.setDetails(5, 11, 2, 2, SCENE570_PROTODATA);
		_icon8.setDetails(6, 12, 3, 2, SCENE570_WACKYEXE);
		_icon9.setDetails(5, 13, 3, 2, SCENE570_WACKYDATA);

		_iconManager.refreshList();
		BF_GLOBALS._player.enableControl();
		break;
	case 5701:
		BF_GLOBALS.setFlag(fDecryptedBluePrints);
		_iconManager._list.remove(&_icon1);
		_icon1.remove();

		_object3.setVisage(572);
		_object3.setStrip(4);
		_object3.setFrame(1);
		_object3.fixPriority(1);

		_icon3.setDetails(7, 6, 0, 2, SCENE570_COBB);
		_iconManager.refreshList();
		T2_GLOBALS._uiElements._active = true;
		T2_GLOBALS._uiElements.show();
		BF_GLOBALS._player.enableControl();
		break;
	case 5704:
	case 5705:
	case 5706:
	case 5707:
		T2_GLOBALS._uiElements._active = true;
		T2_GLOBALS._uiElements.show();
		_object3.setPosition(Common::Point(220, 75));
		_object3.setVisage(572);
		_object3.setStrip(4);
		_object3.setFrame(1);
		// Deliberate fall-through
	case 5702:
	case 5703:
		_object3.fixPriority(1);
		_iconManager.refreshList();
		BF_GLOBALS._player.enableControl();
		break;
	default:
		break;
	}
}

void Scene570::process(Event &event) {
	SceneExt::process(event);

	if (!event.handled && (_sceneMode == 10))
		// Password entry active, so pass events to it
		_passwordEntry.process(event);
}

/*--------------------------------------------------------------------------
 * Scene 580 - Child Protective Services Parking Lot
 *
 *--------------------------------------------------------------------------*/

bool Scene580::Vechile::startAction(CursorType action, Event &event) {
	Scene580 *scene = (Scene580 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 1;
		scene->setAction(&scene->_sequenceManager, scene, 5800, &BF_GLOBALS._player, NULL);
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

bool Scene580::Door::startAction(CursorType action, Event &event) {
	Scene580 *scene = (Scene580 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 5802;
		scene->setAction(&scene->_sequenceManager, scene, 5802, &BF_GLOBALS._player, this, NULL);
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

bool Scene580::Lyle::startAction(CursorType action, Event &event) {
	if (action == CURSOR_LOOK) {
		SceneItem::display2(580, 7);
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene580::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(580);
	setZoomPercents(45, 95, 55, 100);
	BF_GLOBALS._sound1.changeSound(33);

	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 1;

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player._moveDiff = Common::Point(2, 1);

	_door.postInit();
	_door.setVisage(580);
	_door.setStrip(4);
	_door.setPosition(Common::Point(168, 41));
	_door.hide();
	_door.setDetails(580, 5, -1, -1, 1, (SceneItem *)NULL);

	_vechile.postInit();
	_vechile.setVisage(580);
	_vechile.fixPriority(65);

	if (BF_GLOBALS.getFlag(fWithLyle)) {
		_lyle.postInit();
		_lyle.setVisage(835);
		_lyle.setObjectWrapper(new SceneObjectWrapper());
		_lyle.animate(ANIM_MODE_1, NULL);
		_lyle._moveDiff = Common::Point(2, 1);
		_lyle.setPosition(Common::Point(149, 70));
		BF_GLOBALS._sceneItems.push_back(&_lyle);

		_vechile.changeZoom(90);
		_vechile.setStrip(3);
		_vechile.setPosition(Common::Point(165, 76));
		_vechile.setDetails(580, 2, 3, -1, 1, (SceneItem *)NULL);

		BF_GLOBALS._player.setVisage(303);

		BF_GLOBALS._walkRegions.disableRegion(8);
		BF_GLOBALS._walkRegions.disableRegion(9);
		BF_GLOBALS._walkRegions.disableRegion(10);
		BF_GLOBALS._walkRegions.disableRegion(11);
	} else {
		_vechile.setPosition(Common::Point(159, 72));

		if (BF_GLOBALS.getFlag(onDuty)) {
			_vechile.setStrip(1);
			_vechile.setFrame(2);
			_vechile.setDetails(300, 11, 13, -1, 1, (SceneItem *)NULL);

			BF_GLOBALS._player.setVisage(304);
		} else {
			_vechile.setStrip(2);
			_vechile.setFrame(3);
			_vechile.setDetails(580, 0, 1, -1, 1, (SceneItem *)NULL);

			BF_GLOBALS._player.setVisage(303);
		}
	}

	BF_GLOBALS._player.updateAngle(_vechile._position);

	if (BF_GLOBALS._sceneManager._previousScene == 590) {
		// Leaving Services
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager, this, 5801, &BF_GLOBALS._player, NULL);
	} else {
		// Arriving at parking lot
		BF_GLOBALS._player.setPosition(Common::Point(177, 58));
		signal();
	}

	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 580, 6, -1, -1, 1, NULL);
}

void Scene580::signal() {
	switch (_sceneMode) {
	case 1:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 5802:
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(590);
		break;
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene580::process(Event &event) {
	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == INV_COLT45) &&
			BF_GLOBALS._player.contains(event.mousePos)) {
		BF_GLOBALS._player.disableControl();
		SceneItem::display2(350, 26);

		_sceneMode = 0;
		signal();
		event.handled = true;
	}
}

/*--------------------------------------------------------------------------
 * Scene 590 - Child Protective Services
 *
 *--------------------------------------------------------------------------*/

bool Scene590::Laura::startAction(CursorType action, Event &event) {
	Scene590 *scene = (Scene590 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if (!scene->_field17DC)
			scene->_stripNumber = 5905;
		else if (!scene->_field17DE) {
			if (scene->_field17E2) {
				scene->_stripNumber = 5907;
			} else {
				scene->_field17E2 = 1;
				scene->_stripNumber = 5906;
			}
		}

		scene->setAction(&scene->_action1);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene590::Skip::startAction(CursorType action, Event &event) {
	Scene590 *scene = (Scene590 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if (BF_INVENTORY.getObjectScene(INV_BASEBALL_CARD) == 1) {
			scene->_stripNumber = !scene->_field17DC ? 5903 : 5902;
		} else {
			scene->_stripNumber = !scene->_field17DC ? 5901 : 5902;
		}

		scene->setAction(&scene->_action1);
		scene->_field17DC = 1;
		return true;
	case INV_BASEBALL_CARD:
		scene->_field17DE = 1;
		T2_GLOBALS._uiElements.addScore(10);
		BF_INVENTORY.setObjectScene(INV_BASEBALL_CARD, 2);
		BF_GLOBALS._player.disableControl();

		scene->_stripNumber = !scene->_field17DC ? 5900 : 5904;
		scene->_field17DC = 1;
		scene->setAction(&scene->_action2);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene590::Exit::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER(BF_GLOBALS._player._position.x - 100, BF_GLOBALS._player._position.y + 75);
	return true;
}

/*--------------------------------------------------------------------------*/

void Scene590::Action1::signal() {
	Scene590 *scene = (Scene590 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		ADD_PLAYER_MOVER(220, 132);
		break;
	case 1:
		scene->_stripManager.start(scene->_stripNumber, this);
		break;
	case 2:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene590::Action2::signal() {
	Scene590 *scene = (Scene590 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, 220, 132);
		break;
	}
	case 1:
		ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, 220, 132);
		break;
	case 2:
		BF_GLOBALS._player.setVisage(381);
		BF_GLOBALS._player.setStrip(2);
		BF_GLOBALS._player.setFrame(1);
		BF_GLOBALS._player.setPosition(Common::Point(257, 130));
		BF_GLOBALS._player.animate(ANIM_MODE_4, 3, 1, this);

		scene->_skip.setStrip(2);
		scene->_skip.animate(ANIM_MODE_5, NULL);
		break;
	case 3:
		scene->_skip.animate(ANIM_MODE_6, NULL);
		BF_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 4:
		scene->_skip.setStrip(1);
		scene->_skip.animate(ANIM_MODE_1, NULL);

		BF_GLOBALS._player.setVisage(368);
		BF_GLOBALS._player.setStrip(7);
		BF_GLOBALS._player.setPosition(Common::Point(238, 131));
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		scene->_stripManager.start(scene->_stripNumber, this);
		break;
	case 5:
		scene->_sceneMode = 3;
		scene->signal();
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene590::Scene590(): PalettedScene() {
	_field17DC = _field17DE = _stripNumber = _field17E2 = 0;
}

void Scene590::synchronize(Serializer &s) {
	PalettedScene::synchronize(s);
	s.syncAsSint16LE(_field17DC);
	s.syncAsSint16LE(_field17DE);
	s.syncAsSint16LE(_stripNumber);
	s.syncAsSint16LE(_field17E2);
}

void Scene590::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit();
	loadScene(590);

	BF_GLOBALS._sound1.fadeOut2(NULL);
	_sound1.play(18);
	_sound1.fade(100, 5, 10, false, NULL);

	if (BF_GLOBALS._dayNumber == 0) {
		BF_GLOBALS._dayNumber = 1;
		BF_GLOBALS._bookmark = bEndOfWorkDayOne;
		BF_INVENTORY.setObjectScene(INV_BASEBALL_CARD, 1);
	}

	_exit.setDetails(Rect(0, 157, 182, 167), 590, -1, -1, -1, 1, NULL);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setPosition(Common::Point(0, 190));

	_stripManager.addSpeaker(&_skipSpeaker);
	_stripManager.addSpeaker(&_lauraSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);

	if (BF_GLOBALS.getFlag(onDuty)) {
		BF_GLOBALS._player.setVisage(361);
		BF_GLOBALS._player._moveDiff = Common::Point(6, 2);
	} else {
		BF_GLOBALS._player.setVisage(368);
		BF_GLOBALS._player._moveDiff = Common::Point(6, 2);

		if ((BF_GLOBALS._bookmark >= bEndOfWorkDayOne) && (BF_GLOBALS._bookmark < bLyleStoppedBy)) {
			_skip.postInit();
			_skip.setVisage(693);
			_skip.setPosition(Common::Point(271, 117));
			_skip.setDetails(590, 26, -1, 27, 1, (SceneItem *)NULL);

			_laura.postInit();
			_laura.setVisage(692);
			_laura.setPosition(Common::Point(248, 115));
			_laura.setDetails(590, 24, -1, 25, 1, (SceneItem *)NULL);
		}
	}

	_item2.setDetails(1, 590, 0, -1, 12, 1);
	_item3.setDetails(3, 590, 2, -1, 14, 1);
	_item4.setDetails(20, 590, 3, -1, 15, 1);
	_item5.setDetails(22, 590, 4, -1, 16, 1);
	_item6.setDetails(7, 590, 6, -1, 17, 1);
	_item7.setDetails(7, 590, 6, -1, 18, 1);
	_item8.setDetails(9, 590, 7, -1, 19, 1);
	_item9.setDetails(11, 590, 8, -1, 20, 1);
	_item10.setDetails(13, 590, 9, -1, 21, 1);
	_item11.setDetails(15, 590, 10, -1, 22, 1);
	_item12.setDetails(17, 590, 11, -1, 23, 1);

	BF_GLOBALS._player.disableControl();
	_sceneMode = 0;
	setAction(&_sequenceManager, this, 5900, &BF_GLOBALS._player, NULL);
}

void Scene590::signal() {
	static uint32 black = 0;

	switch (_sceneMode) {
	case 1:
		_sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(271);
		break;
	case 2:
		_sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(580);
		break;
	case 3:
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 0, 170);

		_skip.animate(ANIM_MODE_1, NULL);
		ADD_MOVER_NULL(_skip, 0, 170);

		_laura.animate(ANIM_MODE_1, NULL);
		ADD_MOVER_NULL(_laura, 0, 170);

		_sceneMode = 1;
		addFader((byte *)&black, 2, this);
		break;
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene590::process(Event &event) {
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

void Scene590::dispatch() {
	if (!_action && (BF_GLOBALS._player._position.x < 182) && (BF_GLOBALS._player._position.y > 158)) {
		BF_GLOBALS._player.disableControl();
		_sceneMode = 2;
		setAction(&_sequenceManager, this, 5901, &BF_GLOBALS._player, NULL);
	}
}

} // End of namespace BlueForce

} // End of namespace TsAGE
