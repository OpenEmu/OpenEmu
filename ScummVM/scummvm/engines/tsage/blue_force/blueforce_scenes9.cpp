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

#include "tsage/blue_force/blueforce_scenes9.h"
#include "tsage/globals.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 900 - Outside Warehouse
 *
 *--------------------------------------------------------------------------*/

bool Scene900::Exterior::startAction(CursorType action, Event &event) {
	if (action == CURSOR_LOOK) {
		SceneItem::display2(900, 6);
		return true;
	} else {
		return SceneHotspot::startAction(action, event);
	}
}

bool Scene900::WestExit::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	BF_GLOBALS._player.disableControl();
	scene->_sceneMode = 9001;
	Common::Point pt(0, 117);
	PlayerMover *mover = new PlayerMover();
	BF_GLOBALS._player.addMover(mover, &pt, scene);

	return true;
}

/*--------------------------------------------------------------------------*/
bool Scene900::Gate::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (BF_GLOBALS._gateStatus == 0) {
			return NamedObject::startAction(action, event);
		} else {
			BF_GLOBALS._player.disableControl();
			if (BF_GLOBALS._gateStatus == 2) {
				scene->_sceneMode = 9006;
				BF_GLOBALS._gateStatus = 1;
				scene->setAction(&scene->_sequenceManager1, scene, 9006, &BF_GLOBALS._player, this, NULL);
			} else {
				BF_GLOBALS._gateStatus = 2;
				if (scene->_dog._flag == false) {
					BF_GLOBALS._player.setAction(&scene->_action4);
				} else {
					scene->_sceneMode = 9005;
					scene->setAction(&scene->_sequenceManager1, scene, 9005, &BF_GLOBALS._player, this, NULL);
					BF_GLOBALS._walkRegions.enableRegion(24);
				}
			}
			return true;
		}
		break;
	case INV_WAREHOUSE_KEYS:
		if (BF_GLOBALS._gateStatus == 2) {
			SceneItem::display2(900, 14);
		} else {
			if (BF_GLOBALS._gateStatus == 0) {
				if (!BF_GLOBALS.getFlag(fGotPointsForUnlockGate)) {
					BF_GLOBALS.setFlag(fGotPointsForUnlockGate);
					T2_GLOBALS._uiElements.addScore(30);
				}
				BF_GLOBALS._gateStatus = 1;
			} else {
				if (!BF_GLOBALS.getFlag(fGotPointsForLockGate)) {
					if (BF_GLOBALS._bookmark == bEndDayThree) {
						BF_GLOBALS.setFlag(fGotPointsForLockGate);
						T2_GLOBALS._uiElements.addScore(30);
					}
				}
				BF_GLOBALS._gateStatus = 0;
			}
			scene->_sceneMode = 9004;
			BF_GLOBALS._player.disableControl();
			scene->setAction(&scene->_sequenceManager1, scene, 9004, &BF_GLOBALS._player, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene900::Door::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (BF_GLOBALS._gateStatus == 2) {
			if (_flag) {
				SceneItem::display2(900, 1);
			} else {
				BF_GLOBALS._player.disableControl();
				BF_GLOBALS._walkRegions.enableRegion(26);
				scene->_sceneMode = 9007;
				scene->setAction(&scene->_sequenceManager1, scene, 9007, &BF_GLOBALS._player, &scene->_door, this, NULL);
			}
			return true;
		} else
			return NamedObject::startAction(action, event);
		break;
	case INV_WAREHOUSE_KEYS:
		if (BF_GLOBALS._gateStatus == 2) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9012;
			scene->setAction(&scene->_sequenceManager1, scene, 9012, &BF_GLOBALS._player, &scene->_door, NULL);
		} else
			SceneItem::display2(900, 5);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene900::Dog::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(900, 8);
		return true;
	case CURSOR_USE:
		SceneItem::display2(900, 9);
		return true;
	case INV_HOOK:
		if (_flag) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9010;
			scene->setAction(&scene->_sequenceManager1, scene, 9010, &BF_GLOBALS._player, &scene->_object5, this, NULL);
		} else
			SceneItem::display2(900, 23);
		return true;
	case INV_FISHING_NET:
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._player.setAction(&scene->_action2);
		return true;
	case INV_DOG_WHISTLE:
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._player.setAction(&scene->_action3);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene900::Lyle::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_TALK) {
		if (!_action) {
			if (scene->_dog._flag) {
				// Dog is no longer there
				if (BF_GLOBALS._gateStatus == 0)
					scene->_stripManager.start(9004, &BF_GLOBALS._stripProxy);
				else {
					if (scene->_door._flag == 1) {
						if (BF_GLOBALS._gateStatus == 2)
							scene->_stripManager.start(9005, &BF_GLOBALS._stripProxy);
						else
							scene->_stripManager.start(9001, &BF_GLOBALS._stripProxy);
					} else
						scene->_stripManager.start(9001, &BF_GLOBALS._stripProxy);
				}
			} else {
				// Dog is there
				if (scene->_lyleDialogCtr)
					scene->_stripManager.start(9003, &BF_GLOBALS._stripProxy);
				else {
					++scene->_lyleDialogCtr;
					scene->_stripManager.start(9002, &BF_GLOBALS._stripProxy);
				}
			}
		}
		return true;
	} else
		return NamedObject::startAction(action, event);
}

bool Scene900::Body::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 9016;
		scene->setAction(&scene->_sequenceManager1, scene, 9016, &BF_GLOBALS._player, NULL);
		return true;
	} else
		return NamedObject::startAction(action, event);
}

/*--------------------------------------------------------------------------*/
void Scene900::Action1::signal() {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		if (scene->_dog._flag == 0) {
			scene->_dog.setStrip(3);
			if ((BF_GLOBALS._randomSource.getRandomNumber(3) == 1) || (BF_GLOBALS._player._position.x > 790) || (scene->_field1976 != 0)) {
				Common::Point pt(864, 130);
				NpcMover *mover = new NpcMover();
				scene->_dog.addMover(mover, &pt, this);
			} else {
				_actionIndex = 4;
				Common::Point pt(775, 107);
				NpcMover *mover = new NpcMover();
				scene->_dog.addMover(mover, &pt, this);
			}
		}
		break;
	case 1:
		scene->_dog.setPosition(Common::Point(864, 117));
		scene->_dog.setStrip(7);
		scene->_dog.setFrame(1);
		scene->_dog.animate(ANIM_MODE_5, this);
		if (BF_GLOBALS._randomSource.getRandomNumber(3) == 1)
			scene->_sound1.play(92);
		else
			scene->_sound1.play(44);
		if ((BF_GLOBALS._player._position.x > 750) || (scene->_field1976 != 0))
			_actionIndex = 7;
		break;
	case 2:
		scene->_dog.animate(ANIM_MODE_6, this);
		break;
	case 3: {
		scene->_dog.setStrip(3);
		scene->_dog.setPosition(Common::Point(864, 130));
		scene->_dog.fixPriority(122);
		scene->_dog.animate(ANIM_MODE_1, NULL);
		Common::Point pt(775, 107);
		NpcMover *mover = new NpcMover();
		scene->_dog.addMover(mover, &pt, this);
		break;
		}
	case 6:
		_actionIndex = 0;
		// No break on purpose
	case 4:
		setDelay(30);
		break;
	case 5: {
		scene->_dog.setStrip(4);
		Common::Point pt(940, 145);
		NpcMover *mover = new NpcMover();
		scene->_dog.addMover(mover, &pt, this);
		break;
		}
	case 7:
		_actionIndex = 7;
		setDelay(5);
		if (BF_GLOBALS._player._position.x < 790)
			_actionIndex = 2;
		if (scene->_field1976 != 0)
			_actionIndex = 8;
		break;
	case 8:
		scene->_dog.setStrip(1);
		scene->_dog.setFrame(7);
		scene->_dog.animate(ANIM_MODE_6, NULL);
		break;
	case 9:
		scene->_field1976 = 0;
		scene->_dog._flag = 0;
		_actionIndex = 7;
		scene->_dog.setStrip(1);
		scene->_dog.setFrame(1);
		scene->_dog.animate(ANIM_MODE_5, this);
		break;
	default:
		break;
	}
}

void Scene900::Action2::signal() {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_field1976 = 1;
		if (scene->_dog._action->getActionIndex() != 8)
			_actionIndex = 0;
		setDelay(5);
		break;
	case 1:
		scene->_object5.postInit();
		scene->_object5.setVisage(902);
		scene->_object5.setStrip(2);
		scene->_object5.setPosition(Common::Point(-20, -20));
		scene->_object5._moveDiff.y = 10;
		setAction(&scene->_sequenceManager1, this, 9009, &BF_GLOBALS._player, &scene->_object5, &scene->_dog, NULL);
		BF_INVENTORY.setObjectScene(INV_FISHING_NET, 900);
		break;
	case 2:
		BF_GLOBALS._player._strip = 7;
		if (!BF_GLOBALS.getFlag(fGotPointsForTrapDog)) {
			BF_GLOBALS.setFlag(fGotPointsForTrapDog);
			T2_GLOBALS._uiElements.addScore(50);
		}
		SceneItem::display2(900, 10);
		scene->_dog._flag = 1;
		scene->_dog.fixPriority(130);
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene900::Action3::signal() {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		if ((BF_GLOBALS._player._position.x < 790) && (!BF_GLOBALS._player._mover)) {
			Common::Point pt(821, 136);
			PlayerMover *mover = new PlayerMover();
			BF_GLOBALS._player.addMover(mover, &pt, 0);
		}
		if (scene->_dog._action->getActionIndex() != 7) {
			_actionIndex = 0;
		}
		setDelay(5);
		break;
	case 1:
		if (scene->_dog._strip == 3) {
			_actionIndex = 3;
			Common::Point pt(775, 107);
			NpcMover *mover = new NpcMover();
			scene->_dog.addMover(mover, &pt, this);
		} else
			scene->_dog.animate(ANIM_MODE_6, this);
		break;
	case 2: {
		scene->_dog.setStrip(3);
		scene->_dog.setPosition(Common::Point(864, 130));
		scene->_dog.fixPriority(122);
		scene->_dog.animate(ANIM_MODE_1, NULL);
		Common::Point pt(775, 107);
		NpcMover *mover = new NpcMover();
		scene->_dog.addMover(mover, &pt, this);
		break;
		}
	case 3:
		scene->_dog.remove();
		scene->_dog._flag = 1;
		SceneItem::display2(900, 24);
		if (!BF_GLOBALS.getFlag(fGotPointsForLockWarehouse)) {
			BF_GLOBALS.setFlag(fGotPointsForLockWarehouse);
			T2_GLOBALS._uiElements.addScore(10);
		}
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene900::Action4::signal() {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_field1976 = 1;
		if (scene->_dog._action->getActionIndex() != 8)
			_actionIndex = 0;
		setDelay(5);
		break;
	case 1:
		scene->setAction(&scene->_sequenceManager1, this, 9005, &BF_GLOBALS._player, &scene->_gate, NULL);
		break;
	case 2:
		scene->setAction(&scene->_sequenceManager1, this, 9008, &BF_GLOBALS._player, &scene->_dog, NULL);
		break;
	case 3:
		BF_GLOBALS._deathReason = 5;
		BF_GLOBALS._sceneManager.changeScene(666);
		remove();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene900::Scene900(): PalettedScene() {
	_lyleDialogCtr = _field1976 = 0;
}

void Scene900::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit(OwnerList);
	loadScene(900);

	if (BF_GLOBALS._sceneManager._previousScene == 910)
		BF_GLOBALS._sound1.changeSound(91);
	_lyleDialogCtr = 0;
	_field1976 = 0;
	T2_GLOBALS._uiElements._active = true;
	BF_GLOBALS.clearFlag(fCanDrawGun);
	if (BF_GLOBALS._dayNumber == 0) {
		BF_GLOBALS._dayNumber = 4;
		BF_INVENTORY.setObjectScene(INV_FISHING_NET, 1);
		BF_INVENTORY.setObjectScene(INV_HOOK, 1);
	}
	_dog._flag = 0;
	if (BF_GLOBALS._bookmark >= bFinishedWGreen) {
		_body.postInit();
		_body.fixPriority(120);
		_body.setVisage(901);
		_body.setPosition(Common::Point(159,128));
		_body.setDetails(900, 15, 16, 17, ANIM_MODE_1, (SceneItem *)NULL);
	}
	if (BF_GLOBALS._sceneManager._previousScene == 910) {
		_sceneBounds.moveTo(639, 0);

		BF_GLOBALS._gateStatus = 2;
		BF_INVENTORY.setObjectScene(INV_FISHING_NET, 900);
		_dog._flag = 1;
	}
	if (BF_INVENTORY.getObjectScene(INV_FISHING_NET) == 900)
		_dog._flag = 1;

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_lyleHatSpeaker);

	_westExit.setDetails(Rect(0, 85, 20, 130), 900, -1, -1, -1, 1, 0);
	BF_GLOBALS._player.postInit();

	_dog.postInit();
	_dog.setVisage(902);
	_dog.setPosition(Common::Point(845, 135));
	_dog.fixPriority(122);
	_dog.setDetails(900, 8, -1, 9, 1, (SceneItem *)NULL);

	if (_dog._flag == 0) {
		_dog.animate(ANIM_MODE_1, NULL);
		_dog.setAction(&_action1);
	} else {
		_dog.setAction(&_action1);
		_dog.fixPriority(130);
		if (BF_GLOBALS._dayNumber == 4) {
			_dog.setPosition(Common::Point(879, 120));
			_dog.setStrip(2);
		} else {
			_dog.setPosition(Common::Point(864, 117));
			_dog.setStrip(6);
			_dog.setFrame(6);
		}
	}

	_gate.postInit();
	_gate.setVisage(900);
	_gate.setStrip(2);

	if (BF_GLOBALS._gateStatus == 2)
		_gate.setPosition(Common::Point(758, 127));
	else {
		BF_GLOBALS._walkRegions.disableRegion(24);
		_gate.setPosition(Common::Point(804, 132));
	}

	if (BF_GLOBALS._dayNumber == 5)
		BF_GLOBALS._v4CEC8 = 0;

	if ((BF_GLOBALS._v4CEC8 == 0) && (BF_GLOBALS.getFlag(fWithLyle))) {
		_object4.postInit();
		_object4.setVisage(900);
		_object4.setStrip(3);
		_object4.fixPriority(1);
		_object4.setPosition(Common::Point(866, 117));
	}

	_door.postInit();
	_door.setVisage(900);

	if (BF_GLOBALS._v4CEC8 == 0)
		_door.setStrip(4);
	else
		_door.setStrip(1);

	_door.setPosition(Common::Point(847, 45));
	_door._flag = 1;

	if ((BF_GLOBALS._sceneManager._previousScene == 880) || (BF_GLOBALS._sceneManager._previousScene != 910)) {
		BF_GLOBALS._walkRegions.disableRegion(26);
		BF_GLOBALS._player.disableControl();
		if (BF_GLOBALS._bookmark == bFinishedWGreen) {
			_sceneMode = 9013;
			_lyle.postInit();
			_lyle._moveDiff.y = 7;
			_lyle._flags |= OBJFLAG_CHECK_REGION;
			_lyle.setDetails(900, 19, 20, 21, ANIM_MODE_1, (SceneItem *)NULL);
			_lyleHatSpeaker._xp = 210;
			_jakeJacketSpeaker._xp = 75;
			setAction(&_sequenceManager1, this, 9013, &BF_GLOBALS._player, &_lyle, NULL);
			BF_GLOBALS._bookmark = bAmbushed;
			BF_GLOBALS.setFlag(fWithLyle);
		} else {
			if (BF_GLOBALS._bookmark > bFinishedWGreen) {
				_lyle.postInit();
				_lyle.setVisage(811);
				_lyle.setPosition(Common::Point(780, 153));
				_lyle._moveDiff.y = 7;
				_lyle._flags |= 0x1000;
				_lyle.setDetails(900, 19, 20, 21, ANIM_MODE_1, (SceneItem *)NULL);
				_lyle.animate(ANIM_MODE_1, NULL);
				_lyle.setObjectWrapper(new SceneObjectWrapper());
			}

			_sceneMode = 9000;
			setAction(&_sequenceManager1, this, 9000, &BF_GLOBALS._player, NULL);
		}
	} else {
		_door._flag = 0;
		_door.setFrame(_door.getFrameCount());
		BF_GLOBALS._player.disableControl();
		_sceneMode = 9002;
		if (BF_GLOBALS.getFlag(fWithLyle)) {
			_lyle.postInit();
			_lyle._flags |= 0x1000;
			_lyle.setDetails(900, 19, 20, 21, ANIM_MODE_1, (SceneItem *)NULL);
			setAction(&_sequenceManager1, this, 9014, &BF_GLOBALS._player, &_door, &_lyle, NULL);
		} else
			setAction(&_sequenceManager1, this, 9002, &BF_GLOBALS._player, &_door, NULL);
	}

	_gate.setDetails(900, 0, -1, 1, 1, (SceneItem *)NULL);
	_door.setDetails(900, 2, -1, 5, 1, (SceneItem *)NULL);
	_item2.setDetails(Rect(0, 0, 225, 21), 666, 25, -1, -1, 1, NULL);
	_item3.setDetails(Rect(37, 21, 324, 50), 666, 26, -1, -1, 1, NULL);
	_exterior.setDetails(Rect(0, 0, 960, 200), 900, 7, -1, -1, 1, NULL);
}

void Scene900::signal() {
	static uint32 v50E8B = 0;

	switch (_sceneMode++) {
	case 1:
		BF_GLOBALS._sceneManager.changeScene(910);
		break;
	case 3:
		BF_GLOBALS._walkRegions.disableRegion(24);
		_sceneMode = 9004;
		setAction(&_sequenceManager1, this, 9006, &BF_GLOBALS._player, &_gate, NULL);
		break;
	case 9000:
		BF_GLOBALS._player.enableControl();
		break;
	case 9001:
		if ((BF_INVENTORY.getObjectScene(INV_FISHING_NET) == 900) || (BF_GLOBALS._gateStatus != 0) ||
				(_door._flag == 0))
			BF_GLOBALS.setFlag(fLeftTraceIn900);
		else
			BF_GLOBALS.clearFlag(fLeftTraceIn900);

		BF_GLOBALS._sceneManager.changeScene(880);
		break;
	case 9002:
		BF_GLOBALS._walkRegions.disableRegion(26);
		BF_GLOBALS._player.enableControl();
		break;
	case 9004:
		if (BF_GLOBALS._gateStatus == 0)
			SceneItem::display2(900, 3);
		else
			SceneItem::display2(900, 4);
		BF_GLOBALS._player.enableControl();
		break;
	case 9005:
		if (_dog._flag == 0)
			BF_GLOBALS._player.setAction(&_action4);
		else
			BF_GLOBALS._player.enableControl();
		break;
	case 9006:
		BF_GLOBALS._walkRegions.disableRegion(24);
		BF_GLOBALS._player.enableControl();
		break;
	case 9007:
		if (BF_GLOBALS.getFlag(fWithLyle)) {
			Common::Point pt(862, 119);
			PlayerMover *mover = new PlayerMover();
			_lyle.addMover(mover, &pt, NULL);
			_sceneMode = 1;
			addFader((const byte *)&v50E8B, 5, this);
		} else
			BF_GLOBALS._sceneManager.changeScene(910);
		break;
	case 9008:
		BF_GLOBALS._deathReason = 5;
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 9010:
		_sound1.play(92);
		if (BF_GLOBALS._gateStatus == 2) {
			_sceneMode = 9008;
			setAction(&_sequenceManager1, this, 9008, &BF_GLOBALS._player, &_dog, NULL);
		} else {
			BF_GLOBALS._player._strip = 7;
			_action1.setActionIndex(9);
			_dog.signal();
			if ((!BF_GLOBALS.getFlag(fGotPointsForFreeDog)) && (BF_GLOBALS._bookmark == bEndDayThree)) {
				BF_GLOBALS.setFlag(fGotPointsForFreeDog);
				T2_GLOBALS._uiElements.addScore(50);
			}
			BF_INVENTORY.setObjectScene(INV_FISHING_NET, 1);
			SceneItem::display2(900, 11);
			BF_GLOBALS._player.enableControl();
		}
		break;
	case 9012:
		if (_door._flag == 0) {
			SceneItem::display2(900, 12);
			_door._flag = 1;
			if ((!BF_GLOBALS.getFlag(fGotPointsForLockWarehouse)) && (BF_GLOBALS._bookmark == bEndDayThree)) {
				BF_GLOBALS.setFlag(fGotPointsForLockWarehouse);
				T2_GLOBALS._uiElements.addScore(30);
			}
		} else {
			SceneItem::display2(900, 13);
			_door._flag = 0;
			if (!BF_GLOBALS.getFlag(fGotPointsForUnlockWarehouse)) {
				BF_GLOBALS.setFlag(fGotPointsForUnlockWarehouse);
				T2_GLOBALS._uiElements.addScore(30);
			}
		}
		BF_GLOBALS._player.enableControl();
		break;
	case 9013:
		_lyleHatSpeaker._xp = 75;
		_jakeJacketSpeaker._xp = 210;
		_lyle.setAction(&_sequenceManager2, &_lyle, 9015, &_lyle, NULL);
		BF_GLOBALS._player.enableControl();
		break;
	case 9016:
		if ((BF_GLOBALS._clip1Bullets == 0) && (BF_GLOBALS._clip2Bullets == 0)) {
			BF_GLOBALS._clip1Bullets = 8;
			BF_GLOBALS._clip2Bullets = 8;
			SceneItem::display2(900, 25);
		} else if (BF_GLOBALS._clip1Bullets == 0) {
			BF_GLOBALS._clip1Bullets = 8;
			SceneItem::display2(900, 26);
		} else if (BF_GLOBALS._clip2Bullets == 0) {
			BF_GLOBALS._clip2Bullets = 8;
			SceneItem::display2(900, 26);
		} else
			SceneItem::display2(900, 27);

		BF_GLOBALS._player.enableControl();
		break;
	default:
		break;
	}
}

void Scene900::process(Event &event) {
	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1))) {
		if (_westExit.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_W);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene900::dispatch() {
	SceneExt::dispatch();

	if (BF_GLOBALS.getFlag(fWithLyle) && _lyle.isNoMover()) {
		_lyle.updateAngle(BF_GLOBALS._player._position);
	}

	if (!_action) {
		if ((BF_GLOBALS._player._position.x <= 20) && (BF_GLOBALS._player._position.y < 130)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 9001;
			setAction(&_sequenceManager1, this, 9001, &BF_GLOBALS._player, NULL);
		}
	}
}

void Scene900::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_lyleDialogCtr);
	s.syncAsSint16LE(_field1976);
}

/*--------------------------------------------------------------------------
 * Scene 910 - Inside the warehouse
 *
 *--------------------------------------------------------------------------*/
/* Actions */
void Scene910::Action1::signal() {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex - 1) {
	case 0:
		scene->_vent.setStrip(1);
		scene->_vent.animate(ANIM_MODE_5, this);
		break;
	case 1:
		if (scene->_vent._strip == 2)
			scene->_vent.setFrame(4);
		scene->_vent.setStrip(1);
		scene->_vent.animate(ANIM_MODE_6, NULL);
		break;
	case 2:
		scene->_vent.setStrip(2);
		scene->_vent.animate(ANIM_MODE_2, NULL);
		break;
	default:
		break;
	}
}

void Scene910::Action2::signal() {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_shadow.postInit();
		scene->_shadow.setVisage(919);
		scene->_shadow.setPosition(Common::Point(267, 51));
		scene->_shadow.fixPriority(40);
		signal();
		break;
	case 1:
		scene->_shadow.hide();
		setDelay(600);
		break;
	case 2:
		scene->_shadow.setStrip(BF_GLOBALS._randomSource.getRandomNumber(2) + 2);
		scene->_shadow.setFrame(1);
		scene->_shadow.show();
		setDelay(6);
		break;
	case 3:
		_actionIndex = 1;
		scene->_shadow.setStrip(BF_GLOBALS._randomSource.getRandomNumber(2) + 2);
		scene->_shadow.animate(ANIM_MODE_5, this);
		break;
	default:
		break;
	}
}

/* Objects */

void Scene910::Lyle::synchronize(Serializer &s) {
	NamedObject::synchronize(s);
	s.syncAsSint16LE(_field90);
}

bool Scene910::Lyle::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		if (BF_GLOBALS._nico910State == 0)
			return NamedObject::startAction(action, event);
		else
			return false;
	} else if (action == CURSOR_TALK) {
		if ((BF_GLOBALS._hiddenDoorStatus != 0) || (BF_GLOBALS._nico910State != 0)) {
			scene->_stripManager.start(9100 + _field90, &BF_GLOBALS._stripProxy);
			if (_field90 < 1)
				_field90++;
			return true;
		} else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9130;
			if (BF_GLOBALS.getFlag(gunDrawn))
				scene->setAction(&scene->_sequenceManager1, scene, 9138, &BF_GLOBALS._player, &scene->_lyle, NULL);
			else
				scene->setAction(&scene->_sequenceManager1, scene, 9130, &BF_GLOBALS._player, &scene->_lyle, NULL);
			return true;
		}
	} else {
			return NamedObject::startAction(action, event);
	}
}

bool Scene910::Nico::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		BF_GLOBALS._player.disableControl();
		scene->_sceneSubMode = 6;
		scene->_sceneMode = 9123;
		if (BF_GLOBALS._player._visage == 1911)
			scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
		else
			scene->signal();
		return true;
		break;
	case CURSOR_TALK:
		if (BF_GLOBALS._nico910State >= 4)
			return NamedObject::startAction(action, event);

		if (BF_GLOBALS._v4CEE6 < 4)
			BF_GLOBALS._v4CEE6++;

		if (BF_GLOBALS._v4CEE6 == 2) {
			scene->_sceneMode = 13;
			scene->_stripManager.start(9105, scene);
		} else
			scene->_stripManager.start(9103 + BF_GLOBALS._v4CEE6, &BF_GLOBALS._stripProxy);
		return true;
		break;
	case INV_COLT45:
		if (BF_GLOBALS._nico910State > 1) {
			if (BF_GLOBALS._nico910State != 4) {
				if ((BF_GLOBALS.getFlag(gunDrawn)) && (BF_GLOBALS.getFlag(fGunLoaded)) && (BF_GLOBALS.getHasBullets())) {
					if (scene->_field2DE0 == 0) {
						BF_GLOBALS._player.disableControl();
						scene->_sceneMode = 9126;
						scene->setAction(&scene->_sequenceManager1, scene, 9126, &scene->_nico, NULL);
						return NamedObject::startAction(action, event);
					} else {
						scene->_stripManager.start(9117, &BF_GLOBALS._stripProxy);
						return NamedObject::startAction(action, event);
					}
				} else
					return NamedObject::startAction(action, event);
			} else {
				SceneItem::display(910, 90, SET_WIDTH, 312,
						SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
						SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
						SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
						SET_EXT_FGCOLOR, 13, LIST_END);
				return true;
			}
		} else {
			if ((BF_GLOBALS.getFlag(gunDrawn)) && (BF_GLOBALS.getFlag(fGunLoaded)) && (BF_GLOBALS.getHasBullets())) {
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 9125;
				scene->setAction(&scene->_sequenceManager1, scene, 9125, &scene->_nico, NULL);
			}
			return NamedObject::startAction(action, event);
		}
		break;
	case INV_BADGE:
	case INV_ID:
		if (BF_GLOBALS._nico910State >= 4)
			return NamedObject::startAction(action, event);

		if (BF_GLOBALS._v4CEE6 < 4)
			BF_GLOBALS._v4CEE6++;

		if (BF_GLOBALS._v4CEE6 == 2) {
			scene->_sceneMode = 13;
			scene->_stripManager.start(9105, scene);
		} else
			scene->_stripManager.start(9103 + BF_GLOBALS._v4CEE6, &BF_GLOBALS._stripProxy);

		return true;
		break;
	case INV_YELLOW_CORD:
		if (BF_GLOBALS._nico910State < 4) {
			BF_GLOBALS._player.disableControl();
			scene->_yellowCord.fixPriority(121);
			scene->_sceneSubMode = 10;
			scene->_sceneMode = 9123;
			BF_GLOBALS._nico910State = 3;
			if (BF_GLOBALS._player._visage == 1911)
				scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
			else
				scene->signal();

			return true;
		} else {
			SceneItem::display(910, 95, SET_WIDTH, 312,
					SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
					SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
					SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
					SET_EXT_FGCOLOR, 13, LIST_END);

			return true;
		}
		break;
	case INV_HALF_YELLOW_CORD:
		if (BF_GLOBALS._v4CECC == 1)
			SceneItem::display(910, 84, SET_WIDTH, 312,
					SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
					SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
					SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
					SET_EXT_FGCOLOR, 13, LIST_END);
		else
			SceneItem::display(910, 82, SET_WIDTH, 312,
					SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
					SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
					SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
					SET_EXT_FGCOLOR, 13, LIST_END);
		return true;
		break;
	case INV_BLACK_CORD:
	case INV_HALF_BLACK_CORD:
		SceneItem::display(910, 83, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
		return true;
		break;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

bool Scene910::Stuart::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		BF_GLOBALS._player.disableControl();
		scene->_sceneSubMode = 7;
		scene->_sceneMode = 9123;
		if (BF_GLOBALS._player._visage == 1911)
			scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
		else
			scene->signal();
		return true;
		break;
	case CURSOR_TALK:
		if (BF_GLOBALS._v4CEE8 < 3)
			BF_GLOBALS._v4CEE8++;

		scene->_stripManager.start(9107 + BF_GLOBALS._v4CEE8, &BF_GLOBALS._stripProxy);
		return true;
		break;
	case INV_COLT45:
		if ((BF_GLOBALS.getFlag(gunDrawn)) && (BF_GLOBALS.getFlag(fGunLoaded)) && (BF_GLOBALS.getHasBullets())){
			BF_GLOBALS._player.disableControl();
			if (BF_GLOBALS._v4CEE4 == 2) {
				scene->_sceneMode = 9132;
				scene->setAction(&scene->_sequenceManager1, scene, 9132, &scene->_stuart, NULL);
				return NamedObject::startAction(action, event);
			} else {
				if (scene->_field2DE0 == 0) {
					scene->_sceneMode = 9132;
					scene->setAction(&scene->_sequenceManager1, scene, 9132, &scene->_stuart, NULL);
				} else
					scene->_stripManager.start(9117, &BF_GLOBALS._stripProxy);
				return NamedObject::startAction(action, event);
			}
		} else
			return NamedObject::startAction(action, event);
		break;
	case INV_YELLOW_CORD:
		if (BF_GLOBALS._v4CECC == 1) {
			SceneItem::display(910, 84, SET_WIDTH, 312,
					SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
					SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
					SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
					SET_EXT_FGCOLOR, 13, LIST_END);
			return true;
		} else {
			BF_GLOBALS._player.disableControl();
			if (BF_GLOBALS._nico910State == 4) {
				scene->_sceneSubMode = 11;
				scene->_sceneMode = 9123;
				if (BF_GLOBALS._player._visage == 1911)
					scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
				else
					scene->signal();
				return true;
			} else {
				scene->_sceneSubMode = 12;
				scene->_sceneMode = 9123;
				if (BF_GLOBALS._player._visage == 1911)
					scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
				else
					scene->signal();
				return true;
			}
		}
		break;
	case INV_HALF_YELLOW_CORD:
		if (BF_GLOBALS._v4CECC == 1) {
			SceneItem::display(910, 84, SET_WIDTH, 312,
					SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
					SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
					SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
					SET_EXT_FGCOLOR, 13, LIST_END);
			return true;
		} else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneSubMode = 11;
			scene->_sceneMode = 9123;
			if (BF_GLOBALS._player._visage == 1911)
				scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
			else
				scene->signal();
			return true;
		}
		break;
	case INV_BLACK_CORD:
	case INV_HALF_BLACK_CORD:
		SceneItem::display(910, 83, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
		return true;
		break;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

bool Scene910::Forbes::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_TALK) {
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._walkRegions.enableRegion(1);
		BF_GLOBALS._walkRegions.enableRegion(16);
		scene->_sceneMode = 9140;
		scene->setAction(&scene->_sequenceManager1, scene, 9140, &scene->_forbes, &BF_GLOBALS._player, &scene->_lyle, NULL);
		return true;
	} else
		return NamedObject::startAction(action, event);
}

void Scene910::PowerCord::synchronize(Serializer &s) {
	NamedObject::synchronize(s);
	s.syncAsSint16LE(_field90);
	s.syncAsSint16LE(_field92);
}

bool Scene910::PowerCord::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	// the last check is used in order to replace a check on CURSOR_1000, which is replaced
	// directly by its sub-check. All items have an id inferior to 100h.
	if ((action == CURSOR_LOOK) || (action == CURSOR_TALK) || (action < CURSOR_WALK)) {
		if (_field90 == 1)
			return false;
		if ((_field92 != 1) || (BF_GLOBALS._hiddenDoorStatus == 0 ))
			return NamedObject::startAction(action, event);
		return false;
	} else if (action == CURSOR_USE) {
		if (_field90 == 0) {
			if ((BF_GLOBALS._hiddenDoorStatus == 0) || (_field92 != 1)) {
				BF_GLOBALS._player.disableControl();
				if (_field92 == 1) {
					scene->_sceneSubMode = 8;
					scene->_sceneMode = 9123;
					if (BF_GLOBALS._player._visage == 1911)
						scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
					else
						scene->signal();
					_field90 = 1;
					return true;
				} else {
					scene->_destPos = Common::Point(151, 186);
					scene->_sceneSubMode = 4;
					scene->_sceneMode = 9123;
					if (BF_GLOBALS._player._visage == 1911)
						scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
					else
						scene->signal();
					_field90 = 1;
					return true;
				}
			} else
				return false;
		} else if (_field90 == 1)
			return false;
		else
			return true;
	} else
		return NamedObject::startAction(action, event);
}

void Scene910::PowerCord::init(int val) {
	NamedObject::postInit();

	_field92 = val;
	_field90 = 0;
}

bool Scene910::BreakerBox::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (BF_GLOBALS._v4CEC8 == 0)
			SceneItem::display2(910, 7);
		else
			SceneItem::display2(910, 6);
		return true;
		break;
	case CURSOR_USE:
		if (scene->_lyle._position.x == 115) {
			SceneItem::display2(910, 62);
			return true;
		} else if (scene->_sceneMode != 9120) {
			if (BF_GLOBALS._nico910State == 1) {
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 9118;
				scene->setAction(&scene->_sequenceManager1, scene, 9118, &BF_GLOBALS._player, &scene->_nico, NULL);
				return true;
			} else {
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 9102;
				if (BF_GLOBALS.getFlag(gunDrawn)) {
					scene->_sceneSubMode = 1;
					scene->_sceneMode = 9123;
					scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
					return true;
				} else {
					scene->setAction(&scene->_sequenceManager1, scene, 9102, &BF_GLOBALS._player, NULL);
					return true;
				}
			}
		} else {
			SceneItem::display(910, 62, SET_WIDTH, 312,
					SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
					SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
					SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
					SET_EXT_FGCOLOR, 13, LIST_END);
			return true;
		}
		break;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

bool Scene910::FakeWall::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	if (action == INV_YELLOW_CORD) {
		BF_GLOBALS._player.disableControl();
		scene->_destPos = Common::Point(285, 114);
		scene->_sceneSubMode = 9;
		scene->_sceneMode = 9123;
		if (BF_GLOBALS._player._visage == 1911)
			scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
		else
			scene->signal();
		return true;
	} else
		return NamedObject::startAction(action, event);
}

void Scene910::BreakerBoxInset::postInit(SceneObjectList *OwnerList) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	FocusObject::postInit();
	_resNum = 910;
	_lookLineNum = 7;
	_useLineNum = 8;
	BF_GLOBALS._sceneItems.push_back(this);
	scene->_breakerButtonCtr = 0;

	_object13.setupBreaker(115,  44, 1, BF_GLOBALS._breakerBoxStatusArr[0]);
	_object14.setupBreaker(116,  63, 2, BF_GLOBALS._breakerBoxStatusArr[1]);
	_object15.setupBreaker(116,  69, 2, BF_GLOBALS._breakerBoxStatusArr[2]);
	_object16.setupBreaker(115,  76, 1, BF_GLOBALS._breakerBoxStatusArr[3]);
	_object17.setupBreaker(115,  95, 1, BF_GLOBALS._breakerBoxStatusArr[4]);
	_object18.setupBreaker(116, 114, 2, BF_GLOBALS._breakerBoxStatusArr[5]);
	_object19.setupBreaker(116, 120, 2, BF_GLOBALS._breakerBoxStatusArr[6]);
	_object20.setupBreaker(188,  45, 2, BF_GLOBALS._breakerBoxStatusArr[7]);
	_object21.setupBreaker(188,  51, 2, BF_GLOBALS._breakerBoxStatusArr[8]);
	_object22.setupBreaker(179,  59, 1, BF_GLOBALS._breakerBoxStatusArr[9]);
	_object23.setupBreaker(187,  78, 2, BF_GLOBALS._breakerBoxStatusArr[10]);
	_object24.setupBreaker(187,  84, 2, BF_GLOBALS._breakerBoxStatusArr[11]);

	_object25.setupHiddenSwitch(178,  90, 1, BF_GLOBALS._breakerBoxStatusArr[12]);
	_object26.setupHiddenSwitch(178, 108, 2, BF_GLOBALS._breakerBoxStatusArr[13]);
}

void Scene910::BreakerBoxInset::remove() {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	_object13.remove();
	_object14.remove();
	_object15.remove();
	_object16.remove();
	_object17.remove();
	_object18.remove();
	_object19.remove();
	_object20.remove();
	_object21.remove();
	_object22.remove();
	_object23.remove();
	_object24.remove();
	_object25.remove();
	_object26.remove();
	_object27.remove();
	_object28.remove();

	if ((BF_GLOBALS._breakerBoxStatusArr[13] < 4) && (scene->_breakerBox._frame > 1))
		scene->_breakerBox.animate(ANIM_MODE_6, NULL);

	FocusObject::remove();
}

void Scene910::Object13::synchronize(Serializer &s) {
	NamedObject::synchronize(s);
	s.syncAsSint16LE(_state);
	s.syncAsSint16LE(_mode);
}

bool Scene910::Object13::startAction(CursorType action, Event &event) {
	static uint32 black = 0;
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	int8 xDiff;

	if (_mode == 1)
		xDiff = 12;
	else
		xDiff = 7;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display(910, 9, SET_WIDTH, 312,
			SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
			SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
			SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
			SET_EXT_FGCOLOR, 13, LIST_END);
		return true;
	case CURSOR_USE:
		scene->_sound2.play(101);
		if (event.mousePos.x <= _position.x + xDiff) {
			if (_mode != 1) {
				if (_frame > 6)
					setFrame(_frame - 1);
			} else {
				if (_frame > 1)
					setFrame(_frame - 1);
			}
		} else {
			if (_mode == 1) {
				if (_frame < 3)
					setFrame(_frame + 1);
			} else {
				if (_frame < 8)
					setFrame(_frame + 1);
			}
		}

		if (_mode != 1)
			BF_GLOBALS._breakerBoxStatusArr[_state - 1] = (_frame + 251) % 256;
		else
			BF_GLOBALS._breakerBoxStatusArr[_state - 1] = _frame;

		switch (_state) {
		case 1:
			if (BF_GLOBALS._nico910State < 1) {
				if (_frame == 2) {
					if (!BF_GLOBALS.getFlag(fGotPointsForClosingDoor)) {
						T2_GLOBALS._uiElements.addScore(30);
						BF_GLOBALS.setFlag(fGotPointsForClosingDoor);
					}
					scene->_sceneMode = 0;
					if (BF_GLOBALS._dayNumber == 5) {
						if (BF_GLOBALS._nico910State == 0) {
							scene->_breakerBoxInset.remove();
							// _objectList.draw();
							BF_GLOBALS._player.disableControl();
							scene->_lyle.setVisage(912);
							scene->_shadow.remove();
							scene->_action2.remove();
							scene->_nico.postInit();
							scene->_sceneMode = 9129;
							scene->setAction(&scene->_sequenceManager1, scene, 9129, &BF_GLOBALS._player, &scene->_nico, NULL);
						} else if (BF_GLOBALS._nico910State == 2) {
							scene->_breakerBoxInset.remove();
							// _objectList.draw();
							BF_GLOBALS._player.disableControl();
							scene->_lyle.setVisage(911);
							scene->_lyle.setStrip(4);
							scene->_nico.setVisage(923);
							scene->_nico.setStrip(2);
							scene->_stuart.setVisage(923);
							scene->_stuart.setStrip(3);
							scene->_sceneMode = 9134;
							scene->setAction(&scene->_sequenceManager1, scene, 9134, &BF_GLOBALS._player, &scene->_nico, &scene->_lyle, NULL);
						}
					}
					BF_GLOBALS._v4CEC8 = 1;
					scene->_object5.show();
					if (scene->_sceneMode == 0) {
						BF_GLOBALS._player.setVisage(911);
						scene->_lyle.setVisage(912);
					}
					scene->transition((const byte *)&black, 25, 910, NULL, 0, 111, 112, 255, 0);
					BF_GLOBALS._scenePalette.signalListeners();
					// _objectList.draw();
				} else {
					if (BF_GLOBALS._v4CEC8 == 1) {
						if (!BF_GLOBALS.getFlag(fGotPointsForStartGenerator)) {
							T2_GLOBALS._uiElements.addScore(30);
							BF_GLOBALS.setFlag(fGotPointsForStartGenerator);
						}
						BF_GLOBALS._player.disableControl();
						BF_GLOBALS._v4CEC8 = 0;
						scene->_sceneMode = 2;
						scene->transition((const byte *)&black, 30, 910, scene, 0, 111, 112, 255, 0);
					}
				}
			} else
				SceneItem::display(910, 97, SET_WIDTH, 312,
					SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
					SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
					SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
					SET_EXT_FGCOLOR, 13, LIST_END);
			return true;
		case 4:
			if (_frame == 2) {
				if (BF_GLOBALS._breakerBoxStatusArr[4] == 2) {
					scene->_action1.setActionIndex(2);
					scene->_action1.signal();
				}
			} else if (scene->_action1.getActionIndex() == 2) {
				scene->_action1.setActionIndex(1);
				scene->_action1.signal();
			}
			return true;
		case 5:
			if (_frame == 2) {
				if (BF_GLOBALS._breakerBoxStatusArr[3] == 2) {
					scene->_action1.setActionIndex(2);
					scene->_action1.signal();
				}
			} else if (scene->_action1.getActionIndex() == 2) {
				scene->_action1.setActionIndex(1);
				scene->_action1.signal();
			}
			return true;
		case 15:
			if ((BF_GLOBALS._v4CECA == 2) && (BF_GLOBALS._breakerBoxStatusArr[17] == 1)) {
				if (_frame == 7)
					scene->closeHiddenDoor();
				else
					scene->openHiddenDoor();
			}
			return true;
		default:
			return true;
		}
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

void Scene910::Object13::remove() {
	BF_GLOBALS._sceneItems.remove(this);
	SceneObject::remove();
}

void Scene910::Object13::setupBreaker(int x, int y, int mode, int8 frameNumber) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	NamedObject::postInit();
	_mode = mode;
	scene->_breakerButtonCtr++;
	_state = scene->_breakerButtonCtr;
	setVisage(910);

	if (mode == 1) {
		setStrip(8);
		setFrame(frameNumber);
	} else if (mode == 2) {
		setStrip(7);
		setFrame(frameNumber + 5);
	}

	fixPriority(252);
	setPosition(Common::Point(x, y));
	BF_GLOBALS._sceneItems.push_front(this);
}

void Scene910::Object25::synchronize(Serializer &s) {
	NamedObject::synchronize(s);
	s.syncAsSint16LE(_field90);
	s.syncAsSint16LE(_field92);
}

bool Scene910::Object25::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (_field92 == 1)
			SceneItem::display2(910, 10);
		else
			SceneItem::display2(910, 11);
		return true;
	case CURSOR_USE:
		_field92 = BF_GLOBALS._breakerBoxStatusArr[_field90 + 11];
		switch (_field92 - 1) {
		case 0:
			_field92 = 2;
			setStrip(7);
			setFrame(1);
			if (_field90 == 1) {
				scene->_breakerButtonCtr = 14;
				scene->_breakerBoxInset._object27.setupBreaker(182, 92, 2, BF_GLOBALS._breakerBoxStatusArr[14]);
			} else {
				scene->_breakerButtonCtr = 15;
				scene->_breakerBoxInset._object28.init(178, 108, 0, BF_GLOBALS._breakerBoxStatusArr[15]);
			}
			SceneItem::display2(910, 12);
			break;
		case 1:
			_field92 = 3;
			setStrip(6);
			setFrame(3);
			if (_field90 == 1)
				scene->_breakerBoxInset._object27.remove();
			else
				scene->_breakerBoxInset._object28.remove();
			break;
		case 2:
			_field92 = 2;
			setStrip(7);
			setFrame(1);
			if (_field90 == 1) {
				scene->_breakerButtonCtr = 14;
				scene->_breakerBoxInset._object27.setupBreaker(182, 96, 2, BF_GLOBALS._breakerBoxStatusArr[14]);
			} else {
				scene->_breakerButtonCtr = 15;
				scene->_breakerBoxInset._object28.init(178, 108, 0, BF_GLOBALS._breakerBoxStatusArr[15]);
			}
			break;
		case 3:
			SceneItem::display2(910, 13);
			break;
		default:
			break;
		}

		BF_GLOBALS._breakerBoxStatusArr[_field90 + 11] = _field92;
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

void Scene910::Object25::remove() {
	BF_GLOBALS._sceneItems.remove(this);
	SceneObject::remove();
}

void Scene910::Object25::setupHiddenSwitch(int x, int y, int arg8, int argA) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	NamedObject::postInit();
	scene->_breakerButtonCtr++;
	_field90 = arg8;
	_field92 = argA;
	setVisage(910);

	if ((_field92 != 2) && (_field92 != 4)) {
		setStrip(6);
		setFrame(3);
	} else {
		setStrip(7);
		setFrame(1);
		if (_field90 == 1) {
			scene->_breakerButtonCtr = 14;
			scene->_breakerBoxInset._object27.setupBreaker(182, 96, 2, BF_GLOBALS._breakerBoxStatusArr[14]);
		} else {
			scene->_breakerButtonCtr = 15;
			scene->_breakerBoxInset._object28.init(178, 108, 0, BF_GLOBALS._breakerBoxStatusArr[15]);
		}
	}

	fixPriority(251);
	setPosition(Common::Point(x, y));
	BF_GLOBALS._sceneItems.push_front(this);
}

bool Scene910::BlackPlug::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display(910, 14, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
		return true;
	case CURSOR_USE:
		switch (_frame - _state - 2) {
		case 0:
			SceneItem::display(910, 15, SET_WIDTH, 312,
					SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
					SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
					SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
					SET_EXT_FGCOLOR, 13, LIST_END);
			break;
		case 1:
			if (BF_GLOBALS._v4CECA == 1) {
				BF_GLOBALS._v4CECA = 0;
				BF_INVENTORY.setObjectScene(INV_BLACK_CORD, 1);
				BF_INVENTORY.setObjectScene(INV_HALF_BLACK_CORD, 910);
			} else if (BF_GLOBALS._v4CECA == 2) {
				BF_GLOBALS._v4CECA = 1;
				BF_INVENTORY.setObjectScene(INV_BLACK_CORD, 3 - _state);
				BF_INVENTORY.setObjectScene(INV_HALF_BLACK_CORD, 1);
				scene->_blackCord.setPosition(Common::Point(540, 100));
			}
			setFrame(_state + 2);
			break;
		case 2:
			if (BF_GLOBALS._v4CECC == 1) {
				BF_GLOBALS._v4CECC = 0;
				BF_INVENTORY.setObjectScene(INV_YELLOW_CORD, 1);
				BF_INVENTORY.setObjectScene(INV_HALF_YELLOW_CORD, 910);
			} else if (BF_GLOBALS._v4CECC == 2) {
				BF_GLOBALS._v4CECC = 1;
				BF_INVENTORY.setObjectScene(INV_YELLOW_CORD, 3 - _state);
				BF_INVENTORY.setObjectScene(INV_HALF_YELLOW_CORD, 1);
				scene->_yellowCord.setPosition(Common::Point(540, 100));
			}
			setFrame(_state + 2);
			break;
		case 3:
			if ((_position.x - 12) - (5 * _state) < event.mousePos.x) {
				if (BF_GLOBALS._v4CECA == 1) {
					BF_GLOBALS._v4CECA = 0;
					BF_INVENTORY.setObjectScene(INV_BLACK_CORD, 1);
					BF_INVENTORY.setObjectScene(INV_HALF_BLACK_CORD, 910);
					scene->_blackCord.setPosition(Common::Point(540, 100));
				} else if (BF_GLOBALS._v4CECA == 2) {
					BF_GLOBALS._v4CECA = 1;
					BF_INVENTORY.setObjectScene(INV_BLACK_CORD, 3 - _state);
					BF_INVENTORY.setObjectScene(INV_HALF_BLACK_CORD, 1);
					scene->_blackCord.setPosition(Common::Point(540, 100));
				}
				setFrame(_state + 4);
			} else {
				if (BF_GLOBALS._v4CECC == 1) {
					BF_GLOBALS._v4CECC = 0;
					BF_INVENTORY.setObjectScene(INV_YELLOW_CORD, 1);
					BF_INVENTORY.setObjectScene(INV_HALF_YELLOW_CORD, 910);
				} else if (BF_GLOBALS._v4CECC == 2) {
					BF_GLOBALS._v4CECC = 1;
					BF_INVENTORY.setObjectScene(INV_YELLOW_CORD, 3 - _state);
					BF_INVENTORY.setObjectScene(INV_HALF_YELLOW_CORD, 1);
					scene->_yellowCord.setPosition(Common::Point(540, 100));
				}
				setFrame(_state + 3);
			}
		default:
			break;
		}
		BF_GLOBALS._breakerBoxStatusArr[_state + 15] = _frame;
		if (_state == 0) {
			if (_frame == 2)
				BF_GLOBALS._breakerBoxStatusArr[13] = 2;
			else
				BF_GLOBALS._breakerBoxStatusArr[13] = 4;
		}
		return true;
	case INV_HALF_YELLOW_CORD:
		if (BF_INVENTORY.getObjectScene(INV_YELLOW_CORD) == _state + 2) {
			SceneItem::display(910, 85, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
			return true;
		}
		if (BF_GLOBALS._nico910State == 3) {
			SceneItem::display(910, 84, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
			return true;
		}
	// no break on purpose
	case INV_YELLOW_CORD:
		if (BF_GLOBALS._v4CECC == 0) {
			BF_GLOBALS._v4CECC = 1;
			BF_INVENTORY.setObjectScene(INV_YELLOW_CORD, _state + 2);
			BF_INVENTORY.setObjectScene(INV_HALF_YELLOW_CORD, 1);
		} else if (BF_GLOBALS._v4CECC == 1) {
			BF_GLOBALS._v4CECC = 2;
			BF_INVENTORY.setObjectScene(INV_HALF_YELLOW_CORD, _state + 2);
			scene->_yellowCord.setStrip(4);
			scene->_yellowCord.setFrame(2);
			scene->_yellowCord.setPosition(Common::Point(135, 93));
			scene->_yellowCord.fixPriority(50);
		}
		if (_frame - _state == 2)
			setFrame(_state + 4);
		else if (_frame - _state == 3)
			setFrame(_state + 5);
		BF_GLOBALS._breakerBoxStatusArr[_state + 15] = _frame;
		if (_state == 0) {
			if (_frame == 2)
				BF_GLOBALS._breakerBoxStatusArr[13] = 2;
			else
				BF_GLOBALS._breakerBoxStatusArr[13] = 4;
		}
		return true;
	case INV_HALF_BLACK_CORD:
		if (BF_INVENTORY.getObjectScene(INV_BLACK_CORD) == _state + 2) {
			SceneItem::display(910, 85, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
			return true;
		}
	//No break on purpose
	case INV_BLACK_CORD:
		if (BF_GLOBALS._v4CECA == 0) {
			if (_state == 1) {
				if (!BF_GLOBALS.getFlag(fGotPointsForBlackCord)) {
					T2_GLOBALS._uiElements.addScore(30);
					BF_GLOBALS.setFlag(fGotPointsForBlackCord);
				}
			} else {
				if (!BF_GLOBALS.getFlag(fGotPointsForGeneratorPlug)) {
					T2_GLOBALS._uiElements.addScore(30);
					BF_GLOBALS.setFlag(fGotPointsForGeneratorPlug);
				}
			}
			BF_GLOBALS._v4CECA = 1;
			BF_INVENTORY.setObjectScene(INV_BLACK_CORD, _state + 2);
			BF_INVENTORY.setObjectScene(INV_HALF_BLACK_CORD, 1);
		} else if (BF_GLOBALS._v4CECA == 1) {
			if (_state == 1) {
				if (!BF_GLOBALS.getFlag(fGotPointsForBlackCord)) {
					T2_GLOBALS._uiElements.addScore(30);
					BF_GLOBALS.setFlag(fGotPointsForBlackCord);
				}
			} else {
				if (!BF_GLOBALS.getFlag(fGotPointsForGeneratorPlug)) {
					T2_GLOBALS._uiElements.addScore(30);
					BF_GLOBALS.setFlag(fGotPointsForGeneratorPlug);
				}
			}
			BF_GLOBALS._v4CECA = 2;
			BF_INVENTORY.setObjectScene(INV_HALF_BLACK_CORD, _state + 2);
			scene->_blackCord.setStrip(4);
			scene->_blackCord.setFrame(1);
			scene->_blackCord.setPosition(Common::Point(135, 93));
			scene->_blackCord.fixPriority(50);
			scene->_blackCord.show();
			scene->_blackCord._field90 = 1;
			if (BF_GLOBALS._breakerBoxStatusArr[17] == 1) {
				if (BF_GLOBALS._breakerBoxStatusArr[14] == 2)
					scene->closeHiddenDoor();
				else
					scene->openHiddenDoor();
			}
		}
		if (_frame - _state == 2)
			setFrame(_state + 3);
		else if (_frame - _state == 4)
			setFrame(_state + 5);
		BF_GLOBALS._breakerBoxStatusArr[_state + 15] = _frame;
		if (_state == 0) {
			if (_frame == 2)
				BF_GLOBALS._breakerBoxStatusArr[13] = 2;
			else
				BF_GLOBALS._breakerBoxStatusArr[13] = 4;
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

void Scene910::BlackPlug::init(int x, int y, int arg8, int8 argA) {
	NamedObject::postInit();
	_state = arg8;
	_mode = argA;
	setVisage(910);
	if (_state == 0)
		setStrip(7);
	else
		setStrip(3);
	setFrame(argA);
	fixPriority(251);
	setPosition(Common::Point(x, y));
	BF_GLOBALS._sceneItems.push_front(this);
}

void Scene910::BlackPlug::remove() {
	BF_GLOBALS._sceneItems.remove(this);
	SceneObject::remove();
}

void Scene910::GeneratorInset::postInit(SceneObjectList *OwnerList) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	FocusObject::postInit();
	_resNum = 910;
	_lookLineNum = 99;
	_useLineNum = 87;
	BF_GLOBALS._sceneItems.push_front(this);

	scene->_breakerButtonCtr = 16;
	_blackPlug.init(142, 86, 1, BF_GLOBALS._breakerBoxStatusArr[16]);

	scene->_breakerButtonCtr = 17;
	_powerButton.init(BF_GLOBALS._breakerBoxStatusArr[17]);
}

void Scene910::GeneratorInset::remove() {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	BF_GLOBALS._player.disableControl();
	scene->_sceneMode = 12;
	BF_GLOBALS._player.animate(ANIM_MODE_6, scene);
	_blackPlug.remove();
	_powerButton.remove();
	FocusObject::remove();
}

bool Scene910::PowerButton::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		if (_frame == 4) {
			scene->_sound1.play(100);
			scene->_sound1.holdAt(1);
			if (!BF_GLOBALS.getFlag(fGotPointsForFuseBoxPlug)) {
				T2_GLOBALS._uiElements.addScore(30);
				BF_GLOBALS.setFlag(fGotPointsForFuseBoxPlug);
			}
			setFrame(5);
			_object32.setFrame(7);
			if (BF_GLOBALS._v4CECA == 2) {
				if (BF_GLOBALS._breakerBoxStatusArr[14] == 2)
					scene->closeHiddenDoor();
				else
					scene->openHiddenDoor();
			}
		} else {
			scene->_sound1.release();
			if (BF_GLOBALS._bookmark == bEndDayThree) {
				if (!BF_GLOBALS.getFlag(fGotPointsForLightsOff)) {
					T2_GLOBALS._uiElements.addScore(30);
					BF_GLOBALS.setFlag(fGotPointsForLightsOff);
				}
			}
			setFrame(4);
			_object32.setFrame(6);
		}
		BF_GLOBALS._breakerBoxStatusArr[17] = (_frame + 252) % 256;
		return true;
	} else
		return NamedObject::startAction(action, event);
}

void Scene910::PowerButton::remove() {
	_object32.remove();
	SceneObject::remove();
}

void Scene910::PowerButton::init(int frame) {
	NamedObject::postInit();
	setVisage(910);
	setStrip(6);
	setFrame(frame + 4);
	setPosition(Common::Point(159, 83));
	fixPriority(251);
	_object32.postInit();
	_object32.setVisage(910);
	_object32.setStrip(6);
	_object32.setFrame(6 + frame);
	_object32.setPosition(Common::Point(166, 84));
	_object32.fixPriority(251);
	BF_GLOBALS._sceneItems.push_front(this);
}

bool Scene910::Generator::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		if (scene->_lyle._position.x == 115)
			SceneItem::display(910, 62, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
		else if (BF_GLOBALS._nico910State == 1) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9118;
			scene->setAction(&scene->_sequenceManager1, scene, 9118, &BF_GLOBALS._player, &scene->_nico, NULL);
		} else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9103;
			if (BF_GLOBALS.getFlag(gunDrawn)) {
				scene->_sceneMode = 9123;
				scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
			} else
				scene->setAction(&scene->_sequenceManager1, scene, 9103, &BF_GLOBALS._player, NULL);
		}
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene910::Item2::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	if (action == 59) {
		BF_GLOBALS._player.disableControl();
		scene->_destPos = Common::Point(151, 186);
		scene->_sceneSubMode = 5;
		scene->_sceneMode = 9123;
		if (BF_GLOBALS._player._visage == 1911)
			scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
		else
			scene->signal();
		return true;
	} else
		return NamedHotspot::startAction(action, event);
}

bool Scene910::Item3::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_TALK) && (BF_GLOBALS._nico910State == 4) && (BF_GLOBALS._v4CEE4 == 0)) {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 15;
		scene->_stripManager.start(9102, scene);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene910::Item9::startAction(CursorType action, Event &event) {
	if (BF_GLOBALS._hiddenDoorStatus == 0)
		return NamedHotspot::startAction(action, event);
	else
		return false;
}

bool Scene910::Item15::startAction(CursorType action, Event &event) {
	if (BF_GLOBALS._v4CEC8 == 0)
		return false;

	if ((action == CURSOR_LOOK) || (action == CURSOR_USE) || (action == CURSOR_TALK))
		return NamedHotspot::startAction(action, event);

	if (action >= CURSOR_WALK)
		return false;

	SceneItem::display(910, 2, SET_WIDTH, 312,
		SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
		SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
		SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
		SET_EXT_FGCOLOR, 13, LIST_END);
	return true;
}

bool Scene910::Item16::startAction(CursorType action, Event &event) {
	Scene910 *scene = (Scene910 *)BF_GLOBALS._sceneManager._scene;

	if ((BF_GLOBALS._hiddenDoorStatus == 0) || (BF_GLOBALS._nico910State != 0))
		return false;

	if (BF_GLOBALS._player._visage == 1911) {
		BF_GLOBALS._player.disableControl();
		scene->_destPos = Common::Point(292, 100);
		scene->_sceneSubMode = 0;
		scene->_sceneMode = 9123;
		scene->setAction(&scene->_sequenceManager1, scene, 9123, &BF_GLOBALS._player, NULL);
	} else {
		Common::Point pt(292, 100);
		PlayerMover *mover = new PlayerMover();
		BF_GLOBALS._player.addMover(mover, &pt, NULL);
	}

	return true;
}

bool Scene910::Item17::startAction(CursorType action, Event &event) {
	Common::Point pt(15, 159);
	PlayerMover *mover = new PlayerMover();
	BF_GLOBALS._player.addMover(mover, &pt, NULL);

	return true;
}

void Scene910::remove() {
	PalettedScene::remove();
}

void Scene910::synchronize(Serializer &s) {
	PalettedScene::synchronize(s);

	s.syncAsSint16LE(_sceneSubMode);
	s.syncAsSint16LE(_breakerButtonCtr);
	s.syncAsSint16LE(_field2DE0);
	s.syncAsSint16LE(_field2DE2);
	s.syncAsSint16LE(_field2DE4);
	s.syncAsSint16LE(_destPos.x);
	s.syncAsSint16LE(_destPos.y);
}

void Scene910::postInit(SceneObjectList *OwnerList) {
	uint32 unk_50E94 = 0, unk_50E90 = 0;
	uint32 unk_50E98 = 0, unk_50E9C = 0;

	PalettedScene::postInit(OwnerList);
	loadScene(910);

	BF_GLOBALS._sound1.changeSound(99);
	BF_GLOBALS._v51C44 = 0;

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_lyleHatSpeaker);
	_stripManager.addSpeaker(&_fbiSpeaker);
	_stripManager.addSpeaker(&_nicoSpeaker);
	_stripManager.addSpeaker(&_daSpeaker);
	BF_GLOBALS._player.postInit();

	if (BF_GLOBALS._v4CEC8 == 0)
		BF_GLOBALS._player.setVisage(129);
	else
		BF_GLOBALS._player.setVisage(911);

	BF_GLOBALS._player.setPosition(Common::Point(330, 190));
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.disableControl();

	_vent.postInit();
	_vent.setVisage(910);
	if ((BF_GLOBALS._breakerBoxStatusArr[3] == 2) && (BF_GLOBALS._breakerBoxStatusArr[4] == 2)) {
		_action1.setActionIndex(4);
	} else {
		_vent.animate(ANIM_MODE_2, NULL);
		_vent.setStrip(2);
	}
	_vent.setPosition(Common::Point(98, 34));
	_vent.setAction(&_action1);

	_breakerBox.postInit();
	_breakerBox.setVisage(910);
	_breakerBox.setStrip(5);
	_breakerBox.setFrame(1);
	_breakerBox.setPosition(Common::Point(91, 66));

	_fakeWall.postInit();
	_fakeWall.setVisage(910);
	_fakeWall.setStrip(8);
	_fakeWall.setFrame(4);
	_fakeWall.fixPriority(48);

	_yellowCord.init(1);
	_yellowCord.setVisage(910);
	_yellowCord.setStrip(6);
	_yellowCord.setFrame(2);
	_yellowCord.fixPriority(50);

	_blackCord.init(2);
	_blackCord.setVisage(910);
	_blackCord.setStrip(6);
	_blackCord.setFrame(1);
	_blackCord.setPosition(Common::Point(114, 149));
	_blackCord.fixPriority(246);

	if (BF_GLOBALS._dayNumber < 5)
		_item17.setDetails(Rect(0, 149, 29, 167), 910, -1, -1, -1, 1, NULL);

	if (BF_GLOBALS._nico910State == 0)
		_item16.setDetails(Rect(265, 18, 319, 102), 910, -1, -1, -1, 1, NULL);

	_breakerBox.setDetails(910, 6, -1, -1, 1, (SceneItem *)NULL);
	_item15.setDetails(Rect(0, 0, 320, 170), 910, 0, 1, 2, 1, NULL);
	_yellowCord.setDetails(910, 52, 53, -1, 1, (SceneItem *)NULL);
	_blackCord.setDetails(910, 54, 55, -1, 1, (SceneItem *)NULL);
	_item2.setDetails(3, 910, 22, -1, 24, 1);
	_item4.setDetails(1, 910, 16, 17, 18, 1);
	_item8.setDetails(4, 910, 25, 26, 27, 1);
	_item6.setDetails(Rect(37, 58, 42, 67), 910, 34, 35, 36, 1, NULL);
	_item7.setDetails(Rect(114, 53, 135, 87), 910, 37, 38, 39, 1, NULL);
	_generator.setDetails(Rect(146, 67, 184, 93), 910, 40, 41, 42, 1, NULL);
	_item9.setDetails(Rect(266, 39, 274, 70), 910, 43, 44, 45, 1, NULL);
	_item10.setDetails(Rect(276, 27, 288, 83), 910, 46, 47, 48, 1, NULL);
	_item11.setDetails(Rect(295, 42, 312, 87), 910, 49, 50, 51, 1, NULL);
	_fakeWall.setDetails(910, 28, -1, 30, 1, (SceneItem *)NULL);
	_item3.setDetails(7, 910, 59, 60, 61, 1);
	_item5.setDetails(2, 910, 19, 20, 21, 1);
	_backWall.setDetails(6, 910, 28, 29, 30, 1);
	_item13.setDetails(5, 910, 31, 32, 33, 1);
	_item14.setDetails(Rect(0, 0, 320, 170), 910, 3, 4, 5, 1, NULL);
	if (BF_GLOBALS._dayNumber == 0) {
		BF_GLOBALS._dayNumber = 5;
		BF_GLOBALS._sceneManager._previousScene = 900;
		BF_GLOBALS.setFlag(fWithLyle);
	}

	if (   (BF_GLOBALS._sceneManager._previousScene == 910)
		|| (BF_GLOBALS._sceneManager._previousScene == 190)
		|| (BF_GLOBALS._sceneManager._previousScene == 300)) {
		BF_GLOBALS._sceneManager._previousScene = 900;
		BF_GLOBALS._nico910State = 0;
		BF_GLOBALS._v4CEE4 = 0;
	}

	_field2DE0 = 0;
	_field2DE2 = 0;
	_field2DE4 = 0;
	BF_GLOBALS.clearFlag(fCanDrawGun);
	_lyle._position.x = 0;

	if ((BF_GLOBALS._dayNumber == 5) && (BF_GLOBALS._sceneManager._previousScene == 900)){
		BF_GLOBALS.setFlag(fCanDrawGun);
		BF_GLOBALS._v4CEC8 = 0;
		BF_GLOBALS._player.setVisage(129);

		_lyle.postInit();
		_lyle.setVisage(811);
		_lyle.setPosition(Common::Point(-52, 217));
		_lyle.animate(ANIM_MODE_1, NULL);
		_lyle.setObjectWrapper(new SceneObjectWrapper());
		_lyle._field90 = 0;
		_lyle.setDetails(910, 69, 70, 71, 5, &_item4);

		BF_GLOBALS._breakerBoxStatusArr[0] = 3;
		BF_GLOBALS._breakerBoxStatusArr[12] = 2;
		BF_GLOBALS._breakerBoxStatusArr[13] = 4;
		BF_GLOBALS._breakerBoxStatusArr[14] = 3;
		BF_GLOBALS._breakerBoxStatusArr[15] = 3;
		BF_GLOBALS._breakerBoxStatusArr[16] = 4;
		BF_GLOBALS._breakerBoxStatusArr[17] = 1;
		BF_GLOBALS._v4CECA = 2;
		BF_GLOBALS._hiddenDoorStatus = 1;
		_yellowCord.setPosition(Common::Point(291, -30));
		BF_GLOBALS._v4CECC = 0;
	}

	if (BF_GLOBALS._sceneManager._previousScene == 920) {
		BF_GLOBALS.setFlag(fCanDrawGun);
		BF_GLOBALS._player.setPosition(Common::Point(276, 119));
		BF_GLOBALS._player.setStrip(6);
		if (BF_GLOBALS._v4CECC == 0)
			_yellowCord.setPosition(Common::Point(291, -30));

		if (BF_GLOBALS._dayNumber == 5) {
			_nico.postInit();
			_nico.setVisage(914);
			_nico.setStrip(5);
			_nico.setFrame(8);
			_nico.setPosition(Common::Point(263, 120));
			_nico.setDetails(910, 63, 64, 65, 5, &_item4);

			_stuart.postInit();
			_stuart.setVisage(918);
			_stuart.setStrip(4);
			_stuart.setFrame(1);
			_stuart.setPosition(Common::Point(291, 127));
			_stuart.setDetails(910, 66, 67, 68, 5, &_nico);

			_lyle.postInit();
			_lyle.setVisage(916);
			_lyle.setPosition(Common::Point(155, 113));
			_lyle.setStrip(3);
			_lyle.setFrame(3);
			_lyle._field90 = 1;
			_lyle.setDetails(910, 69, 70, 71, 5, &_item4);
		}
		BF_GLOBALS._player.enableControl();
	} else if (BF_GLOBALS._sceneManager._previousScene == 935) {
		BF_GLOBALS.setFlag(fCanDrawGun);
		BF_GLOBALS._v4CEC8 = 0;
		_lyle.postInit();
		_lyle.setVisage(916);
		_lyle.setPosition(Common::Point(155, 113));
		_lyle.setStrip(3);
		_lyle.setFrame(3);
		_lyle._field90 = 1;
		_lyle.setDetails(910, 69, 70 ,71 , 5, &_item4);

		BF_GLOBALS._walkRegions.disableRegion(15);
		BF_GLOBALS._walkRegions.disableRegion(16);
		BF_GLOBALS._walkRegions.disableRegion(14);
		BF_GLOBALS._walkRegions.disableRegion(10);
		if (BF_GLOBALS.getFlag(gunDrawn)) {
			BF_GLOBALS._player.setVisage(1911);
			BF_GLOBALS._player.animate(ANIM_MODE_NONE, 0, NULL);
			BF_GLOBALS._player.setFrame(7);
		}
		BF_GLOBALS._player.setPosition(Common::Point(174, 157));
		BF_GLOBALS._player._strip = 7;
		_nico.postInit();
		_nico.setVisage(913);
		_nico.setPosition(Common::Point(262, 124));
		_nico.setStrip(6);
		BF_GLOBALS._v4CEE6 = 0;
		BF_GLOBALS._nico910State = 1;
		_nico.setDetails(910, 63, 64, 67, 5, &_item4);
		BF_GLOBALS._v4CECA = 2;
		if (BF_GLOBALS._v4CECC == 0)
			_yellowCord.setPosition(Common::Point(291, -30));
		_sceneMode = 11;
		if (BF_GLOBALS._v4CEC8 == 0)
			add2Faders((const byte *)&unk_50E94, 2, 913, this);
		else
			add2Faders((const byte *)&unk_50E90, 2, 911, this);
	} else {
		BF_GLOBALS.clearFlag(gunDrawn);
		BF_GLOBALS._player.disableControl();
	}

	if ((BF_GLOBALS._dayNumber == 5) && (BF_GLOBALS._nico910State == 0)){
		_shadow.postInit();
		_shadow.setAction(&_action2);
	}

	if (BF_INVENTORY.getObjectScene(INV_YELLOW_CORD) == 1)
		_yellowCord.setPosition(Common::Point(540, 100));

	if (BF_INVENTORY.getObjectScene(INV_BLACK_CORD) == 1)
		_blackCord.setPosition(Common::Point(540, 100));

	if (BF_GLOBALS._v4CECC == 2) {
		_yellowCord.setStrip(4);
		_yellowCord.setFrame(2);
		_yellowCord.setPosition(Common::Point(135, 93));
		_yellowCord.setPriority(50);
		_yellowCord._field90 = 1;

		_breakerBox.setFrame(3);
	}

	if (BF_GLOBALS._v4CECA == 2) {
		_blackCord.setStrip(4);
		_blackCord.setFrame(1);
		_blackCord.setPosition(Common::Point(135, 93));
		_blackCord.fixPriority(50);
		_blackCord._field90 = 1;

		_breakerBox.setFrame(3);
	}

	_object5.postInit();
	_object5.setVisage(919);
	_object5.setStrip(5);
	_object5.setPosition(Common::Point(286, 129));
	_object5.fixPriority(1);

	if (BF_GLOBALS._v4CEC8 == 0)
		_object5.hide();

	if (BF_GLOBALS._hiddenDoorStatus == 0) {
		_object5.setFrame(1);
		_fakeWall.setPosition(Common::Point(292, 107));
		if (BF_GLOBALS._v4CECC != 2)
			_yellowCord.setPosition(Common::Point(288, 57));
		BF_GLOBALS._walkRegions.disableRegion(10);
	} else {
		_object5.setFrame(6);
		_fakeWall.setPosition(Common::Point(295, 20));
		_fakeWall.hide();
		if (BF_GLOBALS._v4CECC != 2)
			_yellowCord.setPosition(Common::Point(291, -30));
		BF_GLOBALS._walkRegions.enableRegion(10);
	}

	if (BF_GLOBALS._breakerBoxStatusArr[17] != 0) {
		_sound1.play(100);
		_sound1.holdAt(1);
	}

	if (BF_GLOBALS._sceneManager._previousScene != 935) {
		_sceneMode = 11;
		if (BF_GLOBALS._v4CEC8 == 0)
			add2Faders((const byte *)&unk_50E9C, 10, 910, this);
		else
			add2Faders((const byte *)&unk_50E98, 10, 911, this);
	}
}

void Scene910::signal() {
	static uint32 black = 0;

	switch (_sceneMode) {
	case 2:
		_sceneMode = 3;
		_timer1.set(60, this);
		break;
	case 3:
		_sceneMode = 4;
		transition((const byte *)&black, 35, 910, this, 0, 111, 112, 255, false);
		break;
	case 4:
		_sceneMode = 5;
		_timer1.set(30, this);
		break;
	case 5:
		_sceneMode = 6;
		transition((const byte *)&black, 40, 910, this, 0, 111, 112, 255, false);
		break;
	case 6:
		_sceneMode = 7;
		_timer1.set(60, this);
		break;
	case 7:
		BF_GLOBALS._player.setVisage(129);
		_lyle.setVisage(811);
		_object5.hide();
		_sceneMode = 8;
		transition((const byte *)&black, 95, 910, this, 0, 111, 112, 255, false);
		break;
	case 8:
		_sceneMode = 9;
		_timer1.set(30, this);
		break;
	case 9:
		_sceneMode = 0;
		transition((const byte *)&black, 100, 910, this, 0, 111, 112, 255, false);
		BF_GLOBALS._player.enableControl();
		break;
	case 10:
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._v51C44 = 0;
		BF_GLOBALS._sceneManager.changeScene(935);
		break;
	case 11:
		if (BF_GLOBALS._sceneManager._previousScene == 900) {
			if (BF_GLOBALS._v4CEC8 != 0)
				transition((const byte *)&black, 25, 910, NULL, 0, 111, 112, 255, false);
			if (BF_GLOBALS.getFlag(fWithLyle)) {
				NpcMover *mover = new NpcMover();
				Common::Point destPos(22, 157);
				_lyle.addMover(mover, &destPos, NULL);
			}

			_sceneMode = 9100;
			setAction(&_sequenceManager1, this, 9100, &BF_GLOBALS._player, NULL);
		} else if (BF_GLOBALS._sceneManager._previousScene == 935) {
			_sceneMode = 9146;
			_stripManager.start(9103, this);
		}
		break;
	case 12:
		BF_GLOBALS._player.setVisage(129);
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player.setStrip(7);
		BF_GLOBALS._player.enableControl();
		break;
	case 13:
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._player.setAction(&_sequenceManager2, NULL, 9117, &_nico, NULL);
		BF_GLOBALS._nico910State = 2;
	// No break on purpose
	case 15:
		_stuart.postInit();
		_stuart.setDetails(910, 66, 67, 68, 5, &_nico);
		BF_GLOBALS._v4CEE8 = 0;
		_sceneMode = 9121;
		setAction(&_sequenceManager1, this, 9121, &_stuart, NULL);
		break;
	case 14:
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sceneManager.changeScene(940);
		break;
	case 16:
		_lyle._field90 = 1;
		_sceneMode = 10;
		addFader((const byte *)&black, 2, this);
		BF_GLOBALS._nico910State = 1;
		BF_GLOBALS._walkRegions.disableRegion(16);
		BF_GLOBALS._walkRegions.disableRegion(14);
		BF_GLOBALS._sceneItems.remove(&_item16);
		break;
	case 17:
		BF_GLOBALS._player._strip = 7;
		BF_GLOBALS._player._frame = 1;
		if (_field2DE2 == 0) {
			_field2DE2 = 1;
			if (BF_GLOBALS._nico910State == 4) {
				_sceneMode = 9149;
				setAction(&_sequenceManager1, this, 9149, &BF_GLOBALS._player, NULL);
			} else {
				_sceneMode = 9141;
				setAction(&_sequenceManager1, this, 9141, &BF_GLOBALS._player, &_nico, NULL);
			}
		} else {
			SceneItem::display(910, 91, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
			BF_GLOBALS._player.enableControl();
		}
		break;
	case 18:
		BF_GLOBALS._player._strip = 7;
		BF_GLOBALS._player._frame = 1;
		if (BF_GLOBALS._v4CEE4 == 3) {
			if (_field2DE4 == 0) {
				_field2DE4 = 1;
				_sceneMode = 9142;
				setAction(&_sequenceManager1, this, 9142, &BF_GLOBALS._player, &_stuart, NULL);
			} else {
				SceneItem::display(910, 92, SET_WIDTH, 312,
					SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
					SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
					SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
					SET_EXT_FGCOLOR, 13, LIST_END);
				BF_GLOBALS._player.enableControl();
			}
		} else {
			SceneItem::display(910, 94, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
			BF_GLOBALS._player.enableControl();
		}
		break;
	case 19:
		BF_GLOBALS._deathReason = 14;
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 20:
		BF_GLOBALS._player.enableControl();
		break;
	case 9100:
		if (BF_GLOBALS._dayNumber == 5)
			BF_GLOBALS._walkRegions.disableRegion(1);
		BF_GLOBALS._player.enableControl();
		break;
	case 9101:
		if ((BF_GLOBALS._hiddenDoorStatus == 0) && (BF_GLOBALS._v4CEC8 != 0) && (BF_GLOBALS._breakerBoxStatusArr[17] == 0) && (BF_INVENTORY.getObjectScene(INV_YELLOW_CORD) == 910) && (BF_INVENTORY.getObjectScene(INV_BLACK_CORD) == 910))
			BF_GLOBALS.clearFlag(fGotPointsForSearchingDA);
		else
			BF_GLOBALS.setFlag(fGotPointsForSearchingDA);
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sceneManager.changeScene(900);
		break;
	case 9102:
		if ((BF_GLOBALS._breakerBoxStatusArr[13] < 4) && (_breakerBox._frame == 1))
			_breakerBox.animate(ANIM_MODE_5, NULL);

		BF_GLOBALS._player.enableControl();
		_breakerBoxInset.postInit();
		_breakerBoxInset.setVisage(910);
		_breakerBoxInset.setStrip(3);
		_breakerBoxInset.setFrame(1);
		_breakerBoxInset.setPosition(Common::Point(160, 130));
		_breakerBoxInset.fixPriority(250);
		break;
	case 9103:
		BF_GLOBALS._player.enableControl();
		_generatorInset.postInit();
		_generatorInset.setVisage(910);
		_generatorInset.setStrip(3);
		_generatorInset.setFrame(2);
		_generatorInset.setPosition(Common::Point(160, 130));
		_generatorInset.fixPriority(250);
		break;
	case 9110:
		if (!BF_GLOBALS.getFlag(fGotPointsForCrate)) {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fGotPointsForCrate);
		}
		BF_INVENTORY.setObjectScene(INV_BLACK_CORD, 1);
		_blackCord.setPosition(Common::Point(540, 100));
		BF_GLOBALS._player.enableControl();
		break;
	case 9111:
		if ((BF_GLOBALS._bookmark == bEndDayThree) && (!BF_GLOBALS.getFlag(fGotPointsForGeneratorOff))) {
				T2_GLOBALS._uiElements.addScore(30);
				BF_GLOBALS.setFlag(fGotPointsForGeneratorOff);
		}
		BF_INVENTORY.setObjectScene(INV_BLACK_CORD, 910);
		_blackCord.setStrip(6);
		_blackCord.setFrame(1);
		_blackCord.setPosition(Common::Point(114, 149));
		_blackCord.fixPriority(249);
		BF_GLOBALS._v4CECA = 0;
		_blackCord._field90 = 0;
		BF_GLOBALS._player.enableControl();
		break;
	case 9112:
		BF_GLOBALS._player._strip = 4;
		BF_GLOBALS._player._frame = 1;
		BF_INVENTORY.setObjectScene(INV_YELLOW_CORD, 1);
		_yellowCord.setPosition(Common::Point(540, 100));
		BF_GLOBALS._player.enableControl();
		break;
	case 9113:
		BF_GLOBALS._player._strip = 4;
		BF_GLOBALS._player._frame = 1;
		BF_INVENTORY.setObjectScene(INV_YELLOW_CORD, 910);
		_yellowCord.setStrip(6);
		_yellowCord.setFrame(2);
		_yellowCord.setPosition(Common::Point(_fakeWall._position.x - 4, _fakeWall._position.y - 50));
		_yellowCord.fixPriority(50);
		BF_GLOBALS._v4CECC = 0;
		_yellowCord._field90 = 0;
		_yellowCord.show();
		BF_GLOBALS._player.enableControl();
		break;
	case 9114:
		_fakeWall.hide();
		if ((BF_GLOBALS._dayNumber == 5) && (BF_GLOBALS._nico910State == 0)) {
			BF_GLOBALS._player.disableControl();
			_nico.postInit();
			_nico.setDetails(910, 63, 64, 65, 5, &_item4);
			BF_GLOBALS._v4CEE6 = 0;
			_shadow.postInit();
			_action2.remove();
			_sceneMode = 9116;
			setAction(&_sequenceManager1, this, 9116, &_nico, NULL);
		} else {
			BF_GLOBALS._player.enableControl();
		}
		break;
	case 9115:
		BF_GLOBALS._player.enableControl();
		break;
	case 9116:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 16;
		_stripManager.start(9119, this);
		break;
	case 9118:
	// No break on purpose
	case 9137:
		BF_GLOBALS._deathReason = 16;
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 9119:
		if (!BF_GLOBALS.getFlag(fGotPointsForCordOnForklift)) {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fGotPointsForCordOnForklift);
		}
		BF_INVENTORY.setObjectScene(INV_YELLOW_CORD, 0);
		BF_INVENTORY.setObjectScene(INV_HALF_YELLOW_CORD, 1);
		BF_GLOBALS._player.enableControl();
		break;
	case 9120:
		BF_GLOBALS._walkRegions.disableRegion(7);
		BF_GLOBALS._player.enableControl();
		break;
	case 9121:
		_item3.setDetails(7, 910, 96, 60, 61, 3);
		BF_GLOBALS._v4CEE4 = 2;
		if (BF_GLOBALS._nico910State == 4) {
			_sceneMode = 20;
			_stripManager.start(9115, this);
		} else {
			BF_GLOBALS._player.enableControl();
		}
		break;
	case 9123:
		BF_GLOBALS.clearFlag(gunDrawn);
		switch (_sceneSubMode - 1) {
		case 0:
			_sceneMode = 9102;
			setAction(&_sequenceManager1, this, 9102, &BF_GLOBALS._player, NULL);
			break;
		case 1:
			_sceneMode = 9103;
			setAction(&_sequenceManager1, this, 9103, &BF_GLOBALS._player, NULL);
			break;
		case 2:
			_sceneMode = 9143;
			setAction(&_sequenceManager1, this, 9143, &BF_GLOBALS._player, NULL);
			break;
		case 3:
			_sceneMode = 9110;
			setAction(&_sequenceManager1, this, 9110, &BF_GLOBALS._player, &_blackCord, NULL);
			break;
		case 4:
			_sceneMode = 9111;
			setAction(&_sequenceManager1, this, 9111, &BF_GLOBALS._player, &_blackCord, NULL);
			break;
		case 5:
			switch (BF_GLOBALS._nico910State - 1) {
			case 0:
				_sceneMode = 9118;
				setAction(&_sequenceManager1, this, 9118, &BF_GLOBALS._player, &_nico, NULL);
				break;
			case 1:
				_sceneMode = 9148;
				setAction(&_sequenceManager1, this, 9148, &BF_GLOBALS._player, &_nico, NULL);
				break;
			case 2:
			// No break on purpose
			case 3: {
				_sceneMode = 17;
				PlayerMover *mover = new PlayerMover();
				Common::Point destPos(232, 138);
				BF_GLOBALS._player.addMover(mover, &destPos, this);
				}
			default:
				break;
			}
			break;
		case 6: {
			_sceneMode = 18;
			PlayerMover *mover = new PlayerMover();
			Common::Point destPos(248, 137);
			BF_GLOBALS._player.addMover(mover, &destPos, this);
			break;
			}
		case 7: {
			_sceneMode = 9112;
			PlayerMover *mover = new PlayerMover();
			Common::Point destPos(285, 114);
			BF_GLOBALS._player.addMover(mover, &destPos, this);
			break;
			}
		case 8: {
			_sceneMode = 9113;
			PlayerMover *mover = new PlayerMover();
			Common::Point destPos(285, 114);
			BF_GLOBALS._player.addMover(mover, &destPos, this);
			break;
			}
		case 9:
			_sceneMode = 9119;
			setAction(&_sequenceManager1, this, 9119, &BF_GLOBALS._player, &_nico, &_yellowCord, NULL);
			break;
		case 10:
			_yellowCord.fixPriority(127);
			if (_yellowCord._position.x != 267)
				_yellowCord.hide();
			_sceneMode = 9136;
			setAction(&_sequenceManager1, this, 9136, &BF_GLOBALS._player, &_stuart, &_yellowCord, NULL);
			BF_GLOBALS._v4CEE4 = 3;
			break;
		case 11:
			_sceneMode = 9137;
			setAction(&_sequenceManager1, this, 9137, &BF_GLOBALS._player, &_nico, NULL);
			break;
		default:
			BF_GLOBALS._player.enableControl();
			PlayerMover *mover = new PlayerMover();
			BF_GLOBALS._player.addMover(mover, &_destPos, NULL);
			break;
		}
		break;
	case 9124:
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS.setFlag(gunDrawn);
		break;
	case 9125:
		BF_GLOBALS.setFlag(fBackupAt340);
		BF_GLOBALS._nico910State = 4;
		_stuart.postInit();
		_nico.setDetails(910, 72, 73, 74, 3, (SceneItem *)NULL);
		_stuart.setDetails(910, 66, 67, 68, 5, &_nico);
		BF_GLOBALS._v4CEE8 = 0;
		_sceneMode = 9121;
		setAction(&_sequenceManager1, this, 9121, &_stuart, NULL);
		break;
	case 9126:
		_sceneMode = 19;
		if (BF_GLOBALS._v4CEE4 == 0)
			signal();
		else
			_stripManager.start(9115, this);
		break;
	case 9129:
	// No break on purpose
	case 9134:
		BF_GLOBALS._deathReason = 17;
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 9130:
		_lyle.setAction(&_sequenceManager2, NULL, 9133, &_lyle, NULL);
		BF_GLOBALS._breakerBoxStatusArr[14] = 3;
		openHiddenDoor();
		BF_GLOBALS._walkRegions.disableRegion(15);
		break;
	case 9132:
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._v4CEE4 = 4;
		BF_GLOBALS._deathReason = 13;
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 9135:
		BF_GLOBALS._deathReason = 15;
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 9136:
		if (!BF_GLOBALS.getFlag(fGotPointsForCuffingNico)) {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fGotPointsForCuffingNico);
		}
		_lyle.setAction(&_sequenceManager2, NULL, 9131, &_lyle, NULL);
		BF_GLOBALS._walkRegions.enableRegion(16);
		if (BF_GLOBALS._nico910State == 4)
			BF_INVENTORY.setObjectScene(INV_YELLOW_CORD, 0);
		else
			BF_INVENTORY.setObjectScene(INV_HALF_YELLOW_CORD, 910);
		_forbes.postInit();
		_forbes.setDetails(910, 86, 87, 88, 4, &_item8);
		_sceneMode = 9139;
		setAction(&_sequenceManager1, this, 9139, &_forbes, &BF_GLOBALS._player, NULL);
		break;
	case 9139:
		BF_GLOBALS._walkRegions.disableRegion(4);
		_field2DE0 = 1;
		BF_GLOBALS._player.enableControl();
		break;
	case 9140:
		_sceneMode = 14;
		addFader((const byte *)&black, 2, this);
		break;
	case 9141:
		BF_INVENTORY.setObjectScene(INV_22_SNUB, 1);
		if (!BF_GLOBALS.getFlag(fGotPointsForCuffingDA)) {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fGotPointsForCuffingDA);
		}
		BF_GLOBALS._player.enableControl();
		break;
	case 9142:
		BF_GLOBALS._player.enableControl();
		if (!BF_GLOBALS.getFlag(fGotPointsForSearchingNico)) {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fGotPointsForSearchingNico);
		}
		break;
	case 9143:
		if (BF_GLOBALS._nico910State == 0) {
			BF_GLOBALS._v51C44 = 1;
			BF_GLOBALS._sceneManager.changeScene(920);
		} else {
			SceneItem::display(910, 89, SET_WIDTH, 312,
					SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
					SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
					SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
					SET_EXT_FGCOLOR, 13, LIST_END);
			_sceneMode = 9144;
			setAction(&_sequenceManager1, this, 9144, &BF_GLOBALS._player, NULL);
		}
		break;
	case 9144:
	// No break on purpose
	case 9146:
		BF_GLOBALS._player.enableControl();
		break;
	case 9148:
		BF_GLOBALS._deathReason = 23;
		BF_GLOBALS._v51C44 = 1;
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 9149:
		SceneItem::display(910, 74, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
		BF_INVENTORY.setObjectScene(INV_22_SNUB, 1);
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player.setVisage(129);
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player._strip = 7;
		BF_GLOBALS._player._frame = 1;
		break;
	default:
		break;
	}
}

void Scene910::process(Event &event) {
	SceneExt::process(event);
	if ((!BF_GLOBALS._player._enabled) || (event.mousePos.y >= 167))
		return;

	if (!_focusObject) {
		if (_item17._bounds.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_SW);
				BF_GLOBALS._events.setCursor(surface);
		} else if ((BF_GLOBALS._hiddenDoorStatus == 0) || (BF_GLOBALS._nico910State != 0)) {
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		} else if (!_item16._bounds.contains(event.mousePos)) {
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		} else {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_NW);
			BF_GLOBALS._events.setCursor(surface);
		}
	}

	if (event.eventType != EVENT_BUTTON_DOWN)
		return;

	switch (BF_GLOBALS._events.getCursor()) {
	case INV_COLT45:
		if (!BF_GLOBALS._player._bounds.contains(event.mousePos))
			break;
		if (BF_GLOBALS.getFlag(gunDrawn)) {
			BF_GLOBALS._player.disableControl();
			_destPos = BF_GLOBALS._player._position;
			_sceneMode = 9123;
			setAction(&_sequenceManager1, this, 9123, &BF_GLOBALS._player, NULL);
			event.handled = true;
		} else if (BF_GLOBALS._nico910State <= 1) {
			if (BF_GLOBALS.getFlag(fCanDrawGun)) {
				BF_GLOBALS._player.addMover(NULL);
				BF_GLOBALS._player.disableControl();
				_sceneMode = 9124;
				setAction(&_sequenceManager1, this, 9124, &BF_GLOBALS._player, NULL);
			} else
				SceneItem::display(1, 4, SET_WIDTH, 312,
					SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
					SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
					SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
					SET_EXT_FGCOLOR, 13, LIST_END);
		} else
			SceneItem::display(910, 81, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
		event.handled = true;
		break;
	case CURSOR_WALK:
		if (BF_GLOBALS._nico910State == 1) {
			BF_GLOBALS._player.disableControl();
			if (BF_GLOBALS._player._visage == 1911) {
				BF_GLOBALS._player.disableControl();
				_destPos = event.mousePos;
				_sceneSubMode = 0;
				_sceneMode = 9123;
				setAction(&_sequenceManager1, this, 9123, &BF_GLOBALS._player, NULL);
			} else {
				_sceneMode = 9118;
				setAction(&_sequenceManager1, this, 9118, &BF_GLOBALS._player, &_nico, NULL);
				event.handled = true;
			}
		} else if (BF_GLOBALS._player._visage == 1911) {
			BF_GLOBALS._player.disableControl();
			_destPos = event.mousePos;
			_sceneSubMode = 0;
			_sceneMode = 9123;
			setAction(&_sequenceManager1, this, 9123, &BF_GLOBALS._player, NULL);
		}
	default:
		break;
	}
}

void Scene910::dispatch() {
	SceneExt::dispatch();
	if (_action)
		return;

	if ((_sceneMode != 14) && (BF_GLOBALS._player._position.x < 30) && (BF_GLOBALS._player._position.y > 148)) {
		BF_GLOBALS._player.disableControl();
		if (BF_GLOBALS._dayNumber != 5) {
			_sceneMode = 9101;
			setAction(&_sequenceManager1, this, 9101, &BF_GLOBALS._player, NULL);
		} else {
			_sceneMode = 9146;
			if (BF_GLOBALS._hiddenDoorStatus == 0)
				setAction(&_sequenceManager1, this, 9146, &BF_GLOBALS._player, NULL);
			else
				setAction(&_sequenceManager1, this, 9145, &BF_GLOBALS._player, NULL);
		}
	}

	if ((BF_GLOBALS._player._position.x > 265) && (BF_GLOBALS._player._position.y < 102) && (BF_GLOBALS._hiddenDoorStatus != 0) && (_sceneMode != 9143)) {
		BF_GLOBALS._player.disableControl();
		if (BF_GLOBALS.getFlag(gunDrawn)) {
			_sceneSubMode = 3;
			_sceneMode = 9123;
			setAction(&_sequenceManager1, this, 9123, &BF_GLOBALS._player, NULL);
		} else if (BF_GLOBALS._nico910State == 0) {
			_sceneMode = 9143;
			setAction(&_sequenceManager1, this, 9143, &BF_GLOBALS._player, NULL);
		} else {
			SceneItem::display(910, 98, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
			_sceneMode = 9144;
			setAction(&_sequenceManager1, this, 9144, &BF_GLOBALS._player, NULL);
		}
	}

	if ((BF_GLOBALS._dayNumber == 5) && (BF_GLOBALS._player._position.x > 250) && (_sceneMode != 9135) && (_sceneMode != 11) && (BF_GLOBALS._hiddenDoorStatus != 0) && (BF_GLOBALS._nico910State == 0)) {
		BF_GLOBALS._player.disableControl();
		_shadow.remove();
		_nico.remove();
		_nico.postInit();
		_nico.setDetails(910, 63, 64, 65, 5, &_item4);
		_sceneMode = 9135;
		setAction(&_sequenceManager1, this, 9135, &BF_GLOBALS._player, &_nico, NULL);
	}

}

void Scene910::checkGun() {
	if ((BF_GLOBALS._dayNumber == 5) && (BF_GLOBALS._nico910State == 0) && (BF_GLOBALS._hiddenDoorStatus != 0))
		SceneItem::display(910, 70, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
	else
		SceneExt::checkGun();
}

void Scene910::openHiddenDoor() {
	if (BF_GLOBALS._hiddenDoorStatus != 0)
		return;

	if (! BF_GLOBALS.getFlag(fGotPointsForLightsOn)) {
		T2_GLOBALS._uiElements.addScore(50);
		BF_GLOBALS.setFlag(fGotPointsForLightsOn);
	}
	BF_GLOBALS._hiddenDoorStatus = 1;
	BF_GLOBALS._player.disableControl();
	BF_GLOBALS._walkRegions.enableRegion(10);
	_sceneMode = 9114;
	_sound2.play(42);
	if ((BF_GLOBALS._v4CECC == 0) && (BF_INVENTORY.getObjectScene(INV_YELLOW_CORD) == 910))
		setAction(&_sequenceManager1, this, 9127, &_fakeWall, &_yellowCord, &_object5, NULL);
	else
		setAction(&_sequenceManager1, this, 9114, &_fakeWall, &_object5, NULL);
}

void Scene910::closeHiddenDoor() {
	if (BF_GLOBALS._hiddenDoorStatus != 0) {
		_fakeWall.show();
		if ((BF_GLOBALS._bookmark == bEndDayThree) && (!BF_GLOBALS.getFlag(fGotPointsForOpeningDoor))) {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fGotPointsForOpeningDoor);
		}
		BF_GLOBALS._hiddenDoorStatus = 0;
		BF_GLOBALS._walkRegions.disableRegion(10);
		BF_GLOBALS._player.disableControl();
		_sceneMode = 9115;
		_sound2.play(42);
		if ((BF_GLOBALS._v4CECC == 0) && (BF_INVENTORY.getObjectScene(INV_YELLOW_CORD) == 910))
			setAction(&_sequenceManager1, this, 9128, &_fakeWall, &_yellowCord, &_object5, NULL);
		else
			setAction(&_sequenceManager1, this, 9115, &_fakeWall, &_object5, NULL);
	}

	if ((BF_GLOBALS._dayNumber == 5) && (BF_GLOBALS._nico910State == 0)) {
		// _objectList.draw();
		if (BF_GLOBALS._sceneObjects->contains(&_breakerBoxInset))
			_breakerBoxInset.remove();
		if (BF_GLOBALS._sceneObjects->contains(&_generatorInset))
			_generatorInset.remove();

		BF_GLOBALS._player.disableControl();
		_sceneMode = 9120;
		BF_GLOBALS._player.setAction(&_sequenceManager2, NULL, 9120, &BF_GLOBALS._player, &_lyle, NULL);
		BF_GLOBALS._walkRegions.enableRegion(1);
	}
}

/*--------------------------------------------------------------------------
 * Scene 920 - Inside Warehouse: Secret Room
 *
 *--------------------------------------------------------------------------*/
// Crate
bool Scene920::Item1::startAction(CursorType action, Event &event) {
	Scene920 *scene = (Scene920 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if ((BF_GLOBALS.getFlag(fCrateOpen)) && (BF_GLOBALS._player._visage == 921)) {
			BF_GLOBALS._player.disableControl();
			scene->_crateWindow.postInit();
			scene->_sceneMode = 9204;
			if (!BF_GLOBALS.getFlag(fGotPointsForBoots)) {
				T2_GLOBALS._uiElements.addScore(30);
				BF_GLOBALS.setFlag(fGotPointsForBoots);
			}
			scene->setAction(&scene->_sequenceManager1, scene, 9204, &BF_GLOBALS._player, &scene->_crateWindow, NULL);
			return true;
		} else
			return NamedHotspot::startAction(action, event);
		break;
	case CURSOR_USE:
		BF_GLOBALS._player.disableControl();
		if (BF_GLOBALS.getFlag(fCrateOpen)) {
			if (BF_GLOBALS._player._visage == 921) {
				if ((BF_INVENTORY.getObjectScene(INV_AUTO_RIFLE) != 1) && (BF_GLOBALS.getFlag(fSawGuns))) {
					scene->_sceneMode = 9207;
					scene->setAction(&scene->_sequenceManager1, scene, 9207, &BF_GLOBALS._player, NULL);
				} else {
					scene->_sceneMode = 9203;
					scene->setAction(&scene->_sequenceManager1, scene, 9203, &BF_GLOBALS._player, &scene->_crateTop, NULL);
					BF_GLOBALS.clearFlag(fCrateOpen);
				}
			} else {
				scene->_sceneMode = 9205;
				scene->setAction(&scene->_sequenceManager1, scene, 9205, &BF_GLOBALS._player, NULL);
			}
		} else {
			scene->_sceneMode = 9202;
			scene->setAction(&scene->_sequenceManager1, scene, 9202, &BF_GLOBALS._player, &scene->_crateTop, NULL);
			BF_GLOBALS.setFlag(fCrateOpen);
		}
		return true;
		break;
	default:
		return NamedHotspot::startAction(action, event);
		break;
	}
}

// North Exit
bool Scene920::Item8::startAction(CursorType action, Event &event) {
	Scene920 *scene = (Scene920 *)BF_GLOBALS._sceneManager._scene;

	BF_GLOBALS._player.disableControl();
	if (BF_GLOBALS._player._visage == 921) {
		scene->_sceneMode = 10;
		scene->setAction(&scene->_sequenceManager1, scene, 9206, &BF_GLOBALS._player, NULL);
		// TO BE CHECKED: Original code uses a variable to store the address of scene instance.
		// As it's used later a coordinates to create a playermover, I don't understand.
		// On the other hand, it's not really important as just after the hero leaves the scene
		// so the variable is no longer used.
		// scene->_oldCoord = &scene;
		_field10 = 1;
	} else {
		scene->_sceneMode = 9201;
		scene->setAction(&scene->_sequenceManager1, scene, 9201, &BF_GLOBALS._player, NULL);
	}
	return true;
}

void Scene920::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(920);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 4;
	BF_GLOBALS._player.postInit();
	if (BF_GLOBALS._v4CEC8 != 0) {
		_doorway.postInit();
		_doorway.setVisage(922);
		_doorway.setStrip(2);
		_doorway.fixPriority(1);
		_doorway.setPosition(Common::Point(145, 82));
	}

	_crateTop.postInit();
	_crateTop.setVisage(922);
	if (BF_GLOBALS.getFlag(fCrateOpen)) {
		_crateTop.setStrip(3);
		_crateTop.setFrame(5);
	}

	_crateTop.setPosition(Common::Point(158, 107));
	_crateTop.fixPriority(130);

	_exitN.setDetails(Rect(116, 12, 165, 81), 920, -1, -1, -1, 1, NULL);
	_item6.setDetails(6, 920, 15, 16, 17, 1);
	_item4.setDetails(5, 920, 12, 13, 14, 1);
	_item7.setDetails(4, 920, 9, 10, 11, 1);
	_item5.setDetails(3, 920, 6, 7, 8, 1);
	_crate.setDetails(2, 920, 3, 4, 5, 1);
	_item3.setDetails(1, 920, 3, 4, 5, 1);
	_item2.setDetails(Rect(0, 0, 320, 200), 920, 0, 1, 2, 1, NULL);

	BF_GLOBALS._player.disableControl();
	_sceneMode = 9200;
	setAction(&_sequenceManager1, this, 9200, &BF_GLOBALS._player, NULL);
}

void Scene920::signal() {
	switch (_sceneMode) {
	case 10:
		_sceneMode = 9201;
		setAction(&_sequenceManager1, this, 9201, &BF_GLOBALS._player, NULL);
		break;
	case 9201:
		if (BF_GLOBALS.getFlag(fCrateOpen))
			BF_GLOBALS.setFlag(fLeftTraceIn920);
		else
			BF_GLOBALS.clearFlag(fLeftTraceIn920);
		BF_GLOBALS._sceneManager.changeScene(910);
		break;
	case 9204:
		_crateWindow.remove();
		BF_GLOBALS.setFlag(fSawGuns);
		BF_GLOBALS._player.enableControl();
		break;
	case 9206: {
		BF_GLOBALS._player.enableControl();
		PlayerMover *mover = new PlayerMover();
		BF_GLOBALS._player.addMover(mover, &_oldCoord, NULL);
		break;
		}
	case 9207:
		BF_GLOBALS._player.enableControl();
		T2_GLOBALS._uiElements.addScore(30);
		BF_INVENTORY.setObjectScene(INV_AUTO_RIFLE, 1);
		BF_GLOBALS._bookmark = bEndDayThree;
		break;
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}
void Scene920::process(Event &event) {
	SceneExt::process(event);
	if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (UI_INTERFACE_Y - 1))) {
		if (_exitN.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_N);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == CURSOR_WALK) && (BF_GLOBALS._player._visage == 921)) {
		BF_GLOBALS._player.disableControl();
		_sceneMode = 9206;
		setAction(&_sequenceManager1, this, 9206, &BF_GLOBALS._player, NULL);
		_oldCoord = event.mousePos;
		event.handled = true;
	}
}

void Scene920::dispatch() {
	SceneExt::dispatch();
	if ((_action == 0) && (BF_GLOBALS._player._position.y < 75)) {
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._sceneManager.changeScene(910);
	}
}

void Scene920::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_oldCoord.x);
	s.syncAsSint16LE(_oldCoord.y);
}

/*--------------------------------------------------------------------------
 * Scene 930 - Inside the caravan
 *
 *--------------------------------------------------------------------------*/
/* Objects */
bool Scene930::Object1::startAction(CursorType action, Event &event) {
// Small box
	Scene930 *scene = (Scene930 *)BF_GLOBALS._sceneManager._scene;
	bool result;

	if ((action == CURSOR_USE) && (!BF_GLOBALS.getFlag(fGotPointsForFBI))) {
		scene->setAction(&scene->_action2);
		result = true;
	} else
		result = NamedObject::startAction(action, event);

	return result;
}

bool Scene930::Object2::startAction(CursorType action, Event &event) {
// Small box Inset
	Scene930 *scene = (Scene930 *)BF_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return NamedObject::startAction(action, event);

	NamedObject::startAction(action, event);
	T2_GLOBALS._uiElements.addScore(30);
	BF_INVENTORY.setObjectScene(INV_9MM_BULLETS, 1);
	BF_GLOBALS.setFlag(fGotPointsForFBI);
	remove();
	scene->_box.remove();
	return true;
}

bool Scene930::Object3::startAction(CursorType action, Event &event) {
// Boots
	Scene930 *scene = (Scene930 *)BF_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return NamedObject::startAction(action, event);

	if (!scene->_bootInsetDisplayed)
		scene->setAction(&scene->_action1);
	return true;
}

bool Scene930::Object4::startAction(CursorType action, Event &event) {
// Boot inset
	Scene930 *scene = (Scene930 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_WALK:
		return true;
	case CURSOR_LOOK:
		if (BF_GLOBALS._bookmark >= bFlashBackTwo) {
			_lookLineNum = 94;
			return NamedObject::startAction(action, event);
		} else {
			BF_GLOBALS._player.disableControl();
			NamedObject::startAction(action, event);
			BF_GLOBALS._bookmark = bFlashBackTwo;
			scene->_sceneMode = 2;
			scene->signal();
			return true;
		}
		break;
	case CURSOR_USE:
		if (BF_GLOBALS._bookmark >= bFlashBackTwo) {
			_lookLineNum = 71;
			NamedObject::startAction(action, event);
			scene->ShowSoleInset();
			remove();
		} else
			NamedObject::startAction(action, event);
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

void Scene930::Object4::remove() {
// Boots inset
	Scene930 *scene = (Scene930 *)BF_GLOBALS._sceneManager._scene;

	if (scene->_bootInsetDisplayed && !BF_GLOBALS._sceneObjects->contains(&scene->_soleInset)) {
		scene->_boots.setAction(&scene->_action3);
	}

	FocusObject::remove();
}

bool Scene930::Object5::startAction(CursorType action, Event &event) {
// Boots sole inset
	Scene930 *scene = (Scene930 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_WALK:
		return true;
	case CURSOR_USE:
		if (BF_INVENTORY.getObjectScene(INV_SCHEDULE) == 1)
			return NamedObject::startAction(action, event);
		if (!scene->_soleOpened) {
			animate(ANIM_MODE_4, getFrameCount() - 1, 1, NULL);
			scene->_soleOpened = 1;
			_lookLineNum = 76;
			_useLineNum = 78;
		} else {
			T2_GLOBALS._uiElements.addScore(50);
			BF_INVENTORY.setObjectScene(INV_SCHEDULE, 1);
			setFrame2(getFrameCount());
			_lookLineNum = 92;
			_useLineNum = -1;
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

void Scene930::Object5::remove() {
	Scene930 *scene = (Scene930 *)BF_GLOBALS._sceneManager._scene;
	scene->_boots.setAction(&scene->_action3);

	FocusObject::remove();
}

/* Items */
bool Scene930::Item1::startAction(CursorType action, Event &event) {
	Scene930 *scene = (Scene930 *)BF_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_WALK) || (action == CURSOR_USE)) {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 1;
		scene->setAction(&scene->_sequenceManager1, scene, 9301, &BF_GLOBALS._player, NULL);
		return true;
	} else
		return NamedHotspot::startAction(action, event);
}

/* Actions */
void Scene930::Action1::signal() {
	Scene930 *scene = (Scene930 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		BF_GLOBALS._player.disableControl();
		scene->_bootInsetDisplayed = 1;
		break;
	case 1: {
		Common::Point pt(50, 142);
		PlayerMover *mover = new PlayerMover();
		BF_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 2:
		BF_GLOBALS._player.changeAngle(270);
		setDelay(10);
		break;
	case 3:
		BF_GLOBALS._player.changeZoom(110);
		BF_GLOBALS._player.setVisage(931);
		BF_GLOBALS._player.setStrip(1);
		BF_GLOBALS._player.setFrame(1);
		BF_GLOBALS._player.animate(ANIM_MODE_4, 4, 1, this);
		break;
	case 4:
		scene->_boots.setFrame(2);
		BF_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 5:
		scene->showBootInset();
		if (!BF_GLOBALS.getFlag(fGotPointsForCPU)) {
			T2_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fGotPointsForCPU);
		}
		SceneItem::display(0, 312);
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}
void Scene930::Action2::signal() {
	Scene930 *scene = (Scene930 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		BF_GLOBALS._player.disableControl();
		break;
	case 1: {
		Common::Point pt(176, 137);
		PlayerMover *mover = new PlayerMover();
		BF_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 2:
		setDelay(10);
		break;
	case 3:
		SceneItem::display(930, scene->_box._useLineNum, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
		scene->ShowBoxInset();
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene930::Action3::signal() {
	Scene930 *scene = (Scene930 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._player.animate(ANIM_MODE_4, 4, -1, this);
		break;
	case 1:
		scene->_boots.setFrame(1);
		BF_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 2:
		BF_GLOBALS._player.changeZoom(-1);
		BF_GLOBALS._player.setVisage(368);
		BF_GLOBALS._player.setStrip(6);
		BF_GLOBALS._player.setFrame(1);
		scene->_bootInsetDisplayed = 0;
		remove();
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player.enableControl();
		break;
	default:
		break;
	}
}

void Scene930::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit();
	loadScene(930);

	BF_GLOBALS._sound1.changeSound(85);
	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 1;
	setZoomPercents(83, 75, 140, 100);
	_soleOpened = 0;
	_bootInsetDisplayed = 0;
	if (BF_INVENTORY.getObjectScene(INV_9MM_BULLETS) != 1) {
		_box.postInit();
		_box.setVisage(930);
		_box.setStrip(1);
		_box.setPosition(Common::Point(223, 21));
		_box.setDetails(930, 66, 67, 68, 1, (SceneItem *)NULL);
	}
	_boots.postInit();
	_boots.setVisage(930);
	_boots.setStrip(2);
	_boots.setPosition(Common::Point(9, 161));
	_boots.fixPriority(120);
	_boots.setDetails(930, 62, 63, 64, 1, (SceneItem *)NULL);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(368);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setPosition(Common::Point(170, 92));
	BF_GLOBALS._player.fixPriority(80);
	BF_GLOBALS._player.changeZoom(-1);
	BF_GLOBALS._player.enableControl();
	BF_GLOBALS._events.setCursor(CURSOR_WALK);

	_item1.setDetails(  1, 930,  0,  1,  2, 1);
	_item2.setDetails(  2, 930,  4,  5,  6, 1);
	_item3.setDetails(  3, 930,  8,  9, 10, 1);
	_item4.setDetails(  4, 930, 12, 13, 14, 1);
	_item5.setDetails(  5, 930, 16, 17, 18, 1);
	_item6.setDetails( 20, 930, 20, 21, 22, 1);
	_item7.setDetails(  6, 930, 23, 24, 25, 1);
	_item8.setDetails(  7, 930, 26, 27, 28, 1);
	_item21.setDetails( 8, 930, 89, 90, 91, 1);
	_item9.setDetails(  9, 930, 29, 30, 31, 1);
	_item20.setDetails(10, 930, 86, 87, 88, 1);
	_item10.setDetails(11, 930, 33, 34, 35, 1);
	_item11.setDetails(12, 930, 37, 38, 39, 1);
	_item13.setDetails(13, 930, 40, 41, 42, 1);
	_item14.setDetails(14, 930, 44, 45, 46, 1);
	_item15.setDetails(15, 930, 48, 49, 50, 1);
	_item16.setDetails(16, 930, 52, 53, 54, 1);
	_item17.setDetails(17, 930, 56, 57, 58, 1);
	_item12.setDetails(18, 930, 59, 60, 61, 1);
	_item18.setDetails(19, 930, 80, 81, 82, 1);
	_item19.setDetails(21, 930, 83, 84, 85, 1);

	if (BF_GLOBALS._sceneManager._previousScene != 935) {
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 9300, &BF_GLOBALS._player, NULL);
	} else {
		// After flashback
		_bootInsetDisplayed = 1;
		BF_GLOBALS._player.animate(ANIM_MODE_NONE);
		BF_GLOBALS._player.setPosition(Common::Point(50, 142));
		BF_GLOBALS._player.setVisage(931);
		BF_GLOBALS._player.setStrip(1);
		BF_GLOBALS._player.setFrame(9);
		BF_GLOBALS._player.fixPriority(-1);
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player.changeZoom(110);
		_boots.setFrame(2);
		showBootInset();
	}

}

void Scene930::signal() {
	static uint32 v50EC4 = 0;

	switch (_sceneMode++) {
	case 1:
		BF_GLOBALS._sceneManager.changeScene(550);
		break;
	case 2:
		_sceneMode = 3;
		SceneItem::display(930, 95, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + UI_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
		signal();
		break;
	case 3:
		_sceneMode = 4;
		addFader((const byte *)&v50EC4, 5, this);
		break;
	case 4:
		BF_GLOBALS._sceneManager.changeScene(935);
		break;
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene930::dispatch() {
	SceneExt::dispatch();
}

void Scene930::showBootInset() {
	_bootsInset.postInit();
	_bootsInset.setVisage(930);
	_bootsInset.setStrip(3);
	_bootsInset.setFrame2(1);
	_bootsInset.fixPriority(260);
	_bootsInset.setPosition(Common::Point(147, 128));
	_bootsInset.setDetails(930, 69, 70, 93);
}

void Scene930::ShowBoxInset() {
	_boxInset.postInit();
	_boxInset.setVisage(930);
	_boxInset.setStrip(1);
	_boxInset.setFrame2(2);
	_boxInset.fixPriority(260);
	_boxInset.setPosition(Common::Point(147, 128));
	_boxInset.setDetails(930, 73, 74, 75);
}

void Scene930::ShowSoleInset() {
	_soleInset.postInit();
	_soleInset.setVisage(930);
	_soleInset.setStrip(3);
	if (BF_INVENTORY.getObjectScene(INV_SCHEDULE) == 1) {
		_soleInset.setFrame(_soleInset.getFrameCount());
		_soleInset.setDetails(930, 92, 77, -1);
	} else if (!_soleOpened) {
		_soleInset.setFrame(2);
		_soleInset.setDetails(930, 93, 77, -1);
	} else {
		_soleInset.setFrame(_soleInset.getFrameCount());
		_soleInset.setDetails(930, 76, 77, 78);
	}

	_soleInset.fixPriority(260);
	_soleInset.setPosition(Common::Point(147, 128));
}

void Scene930::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_soleOpened);
	s.syncAsSint16LE(_bootInsetDisplayed);
}

/*--------------------------------------------------------------------------
 * Scene 935 - Hidden in the wardrobe
 *
 *--------------------------------------------------------------------------*/

void Scene935::Action1::signal() {
	Scene935 *scene = (Scene935 *)BF_GLOBALS._sceneManager._scene;
	static uint32 v50ECC = 0, v50EEA = 0, v50EEE = 0, v50F26 = 0, v50F2A = 0, v50F62 = 0, v50F66 = 0, v50F6A = 0;

	switch (_actionIndex++) {
	case 0:
		scene->addFader((const byte *)&v50ECC, 100, this);
		break;
	case 1:
		scene->_visualSpeaker.setText("Jake! Hide in the closet!");
		for (int i = 1; i < 21; i++)
			scene->transition((const byte *)&v50EEA, 5 * i, 935, NULL, 0, 255, 249, 255, 1);
		setDelay(3);
		break;
	case 2:
		scene->addFader((const byte *)&v50EEE, 5, this);
		break;
	case 3:
		scene->_visualSpeaker.removeText();
		scene->_visualSpeaker._textPos.y = scene->_sceneBounds.top + 80;
		scene->_visualSpeaker._color1 = 252;
		scene->_visualSpeaker._color1 = 251;
		scene->_visualSpeaker.setText("Jake! Hide in the closet!");
		setDelay(3);
		break;
	case 4:
		scene->_visualSpeaker.setText("Jake! Hide in the closet!");
		for (int i = 1; i < 21; i++)
			scene->transition((const byte *)&v50F26, 5 * i, 935, NULL, 0, 255, 249, 255, 1);
		setDelay(3);
		break;
	case 5:
		scene->addFader((const byte *)&v50F2A, 5, this);
		break;
	case 6:
		scene->_visualSpeaker.removeText();
		scene->_visualSpeaker._textPos.y = scene->_sceneBounds.top + 150;
		scene->_visualSpeaker._color1 = 250;
		scene->_visualSpeaker._color1 = 249;
		scene->_visualSpeaker.setText("Jake! Hide in the closet!");
		setDelay(3);
		break;
	case 7:
		scene->_visualSpeaker.setText("Jake! Hide in the closet!");
		for (int i = 1; i < 21; i++)
			scene->transition((const byte *)&v50F62, 5 * i, 935, NULL, 0, 255, 249, 255, 1);
		setDelay(3);
		break;
	case 8:
		scene->addFader((const byte *)&v50F66, 5, this);
		break;
	case 9:
		scene->_visualSpeaker.removeText();
		setDelay(3);
		break;
	case 10:
		scene->_sceneMode = 1;
		scene->add2Faders((const byte *)&v50F6A, 5, 935, scene);
		remove();
		break;
	default:
		break;
	}
}

void Scene935::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit();
	loadScene(935);

	BF_GLOBALS._interfaceY = 200;
	BF_GLOBALS._player.disableControl();
	_visualSpeaker._textMode = ALIGN_CENTER;
	_visualSpeaker._hideObjects = false;
	_visualSpeaker._color1 = 254;
	_visualSpeaker._color2 = 253;
	_visualSpeaker._textPos.y = _sceneBounds.top + 18;
	_visualSpeaker._textWidth = 300;
	_visualSpeaker._textPos.x = _sceneBounds.left + 10;
	setAction(&_action1);
	BF_GLOBALS._sound1.fadeSound(67);
}

void Scene935::remove() {
	BF_GLOBALS._sound1.fadeOut2(NULL);
	BF_GLOBALS._scrollFollower = &BF_GLOBALS._player;
	PalettedScene::remove();
}

void Scene935::signal() {
	static uint32 v50EC8 = 0;

	switch (_sceneMode) {
	case 1:
		_object1.postInit();
		if (BF_GLOBALS._sceneManager._previousScene == 810) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 9352;
			setAction(&_sequenceManager, this, 9350, &_object1, NULL);
		} else if (BF_GLOBALS._sceneManager._previousScene == 930) {
			_object3.postInit();
			_object3.setVisage(938);
			_object3.fixPriority(255);
			_object3.setPosition(Common::Point(260, -4));
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager, this, 9354, &_object1, &_object3, NULL);
		} else {
			_sceneMode = 9351;
			setAction(&_sequenceManager, this, 9350, &_object1, NULL);
		}
		break;
	case 2:
		BF_GLOBALS._sound1.play(68);
		_sceneMode = 0;
		addFader((const byte *)&v50EC8, 5, this);
		break;
	case 3:
		_sceneMode = 2;
		_object1.animate(ANIM_MODE_6, NULL);
		signal();
		break;
	case 9351:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 2;
		setAction(&_sequenceManager, this, 9351, &_object1, NULL);
		if (BF_GLOBALS._sceneManager._previousScene == 910)
			_sceneMode = 9353;
		break;
	case 9352:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 2;
		setAction(&_sequenceManager, this, 9352, &_object1, NULL);
		break;
	case 9353:
		_object2.postInit();
		BF_GLOBALS._player.disableControl();
		_sceneMode = 3;
		setAction(&_sequenceManager, this, 9353, &_object1, &_object2, NULL);
		break;
	default:
		BF_GLOBALS._sceneManager.changeScene(BF_GLOBALS._sceneManager._previousScene);
		break;
	}
}

void Scene935::dispatch() {
	SceneExt::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 940 - Jail ending animation
 *
 *--------------------------------------------------------------------------*/

bool Scene940::Item1::startAction(CursorType action, Event &event) {
	return true;
}

void Scene940::Action1::signal() {
	Scene940 *scene = (Scene940 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 0:
		_actionIndex = 2;
		setDelay(60);
		break;
	case 2:
		_actionIndex = 941;
		if (BF_GLOBALS.getFlag(fBackupAt340)) {
			// WORKAROUND: If the player shot Nico, the alternate text was previously going off-screen
			scene->_gameTextSpeaker1._offsetPos.y -= 10;
			scene->_stripManager.start(9408, this);
		} else
			scene->_stripManager.start(9400, this);
		break;
	case 5:
		setActionIndex(948);
		setDelay(90);
		scene->_object2.remove();
		scene->_object3.remove();
		scene->_object4.remove();
		scene->_object5.remove();
		scene->_object6.remove();
		scene->_object7.remove();
		scene->_object8.remove();
		scene->_object9.remove();
		scene->_object10.remove();
		break;
	case 99:
		BF_GLOBALS._sound1.play(117);
		BF_GLOBALS._sceneManager.changeScene(100);
		remove();
		break;
	case 941:
		scene->_gameTextSpeaker1._textWidth = 312;
		_actionIndex = 944;
		setAction(&scene->_sequenceManager1, this, 941, &BF_GLOBALS._player, NULL);
		break;
	case 942:
		_actionIndex = 955;
		setAction(&scene->_sequenceManager1, this, 942, &scene->_object2, NULL);
		break;
	case 943:
		_actionIndex = 946;
		setAction(&scene->_sequenceManager1, this, 943, &scene->_object3, NULL);
		break;
	case 944:
		scene->_object4.setAction(&scene->_sequenceManager2, NULL, 944, &scene->_object4, &scene->_object5);
		_actionIndex = 945;
		setDelay(3);
		break;
	case 945:
		scene->_object6.setAction(&scene->_sequenceManager3, NULL, 945, &scene->_object6, &scene->_object10, NULL);
		_actionIndex = 943;
		setDelay(3);
		break;
	case 946:
		_actionIndex = 942;
		setAction(&scene->_sequenceManager1, this, 946, &scene->_object7, &scene->_object8, NULL);
		break;
	case 947:
		_actionIndex = 5;
		setAction(&scene->_sequenceManager1, this, 947, &scene->_object1, &scene->_object11, &scene->_object12, &scene->_object13, NULL);
		break;
	case 948:
		scene->_gameTextSpeaker1._textPos.x = scene->_sceneBounds.left + 10;
		scene->_object17.postInit();
		scene->_object17.hide();
		scene->_object18.postInit();
		scene->_object18.hide();
		_actionIndex = 960;
		setAction(&scene->_sequenceManager1, this, 948, &scene->_object11, &scene->_object12, &scene->_object13, &scene->_object1, &scene->_object14, NULL);
		break;
	case 949:
		_actionIndex = 950;
		setAction(&scene->_sequenceManager1, this, 949, &scene->_object17, &scene->_object18, &scene->_object1, &scene->_object12, &scene->_object13, &scene->_object15, NULL);
		scene->_object11.setAction(&scene->_sequenceManager2, NULL, 952, &scene->_object11, NULL);
		break;
	case 950:
		_actionIndex = 951;
		setAction(&scene->_sequenceManager1, this, 950, &scene->_object17, &scene->_object18, &scene->_object1, &scene->_object13, &scene->_object16, NULL);
		scene->_object12.setAction(&scene->_sequenceManager3, NULL, 953, &scene->_object12, NULL);
		break;
	case 951:
		scene->_object13.setAction(&scene->_sequenceManager4, NULL, 954, &scene->_object13, NULL);
		scene->_object11.setAction(&scene->_sequenceManager2, NULL, 952, &scene->_object11, NULL);
		scene->_stripManager.start(9407, this);
		setActionIndex(99);
		break;
	case 955:
		scene->_object2.remove();
		_actionIndex = 956;
		setAction(&scene->_sequenceManager1, this, 955, &BF_GLOBALS._player, NULL);
		break;
	case 956:
		_actionIndex = 947;
		setAction(&scene->_sequenceManager1, this, 956, &scene->_object3, NULL);
		scene->_object11.postInit();
		scene->_object11.setVisage(943);
		scene->_object11.setStrip(1);
		scene->_object11.setPosition(Common::Point(-8, 178));
		scene->_object12.postInit();
		scene->_object12.setVisage(942);
		scene->_object12.setStrip(1);
		scene->_object12.setPosition(Common::Point(-41, 181));
		scene->_object13.postInit();
		scene->_object13.setVisage(944);
		scene->_object13.setStrip(2);
		scene->_object13.setPosition(Common::Point(-74, 179));
		scene->_object1.postInit();
		scene->_object1.setVisage(948);
		scene->_object1.setStrip(2);
		scene->_object1.setPosition(Common::Point(-107, 180));
		scene->_object14.postInit();
		scene->_object14.setVisage(949);
		scene->_object14.setStrip(1);
		scene->_object14.setFrame(3);
		scene->_object14.setPosition(Common::Point(234, 75));
		scene->_object15.postInit();
		scene->_object15.setVisage(949);
		scene->_object15.setStrip(1);
		scene->_object15.setFrame(2);
		scene->_object15.setPosition(Common::Point(144, 76));
		scene->_object16.postInit();
		scene->_object16.setVisage(949);
		scene->_object16.setStrip(1);
		scene->_object16.setFrame(1);
		scene->_object16.setPosition(Common::Point(45, 77));
		break;
	case 960:
		_actionIndex = 949;
		setAction(&scene->_sequenceManager1, this, 960, &scene->_object11, &scene->_object17, &scene->_object18, &scene->_object1, &scene->_object14, NULL);
		break;
	default:
		break;
	}
}

void Scene940::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(940);

	BF_GLOBALS._sound1.play(115);
	BF_GLOBALS._dayNumber = 6;
	BF_GLOBALS._interfaceY = 200;
	T2_GLOBALS._uiElements._active = false;

	_gameTextSpeaker2._speakerName = "SENTTEXT";
	_gameTextSpeaker2._color1 = 104;
	_gameTextSpeaker2._textMode = ALIGN_CENTER;
	_stripManager.addSpeaker(&_gameTextSpeaker1);
	_stripManager.addSpeaker(&_gameTextSpeaker2);
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(941);
	BF_GLOBALS._player.setStrip(1);
	BF_GLOBALS._player.setFrame(7);
	BF_GLOBALS._player.setPosition(Common::Point(563, 80));
	BF_GLOBALS._player.disableControl();
	_object3.postInit();
	_object3.setVisage(944);
	_object3.setStrip(3);
	_object3.setFrame(3);
	_object3.setPosition(Common::Point(626, 78));
	_object3.fixPriority(0);
	_object2.postInit();
	_object2.setVisage(944);
	_object2.setStrip(3);
	_object2.setFrame(6);
	_object2.setPosition(Common::Point(378, 136));
	_object2.setZoom(80);
	_object2.fixPriority(0);
	_object4.postInit();
	_object4.setVisage(945);
	_object4.setStrip(6);
	_object4.setPosition(Common::Point(361, 70));
	_object4.setZoom(80);
	_object4.fixPriority(0);
	_object5.postInit();
	_object5.setVisage(945);
	_object5.setStrip(7);
	_object5.setPosition(Common::Point(366, 39));
	_object5.setZoom(80);
	_object5.fixPriority(3);
	_object6.postInit();
	_object6.setVisage(945);
	_object6.setStrip(4);
	_object6.setPosition(Common::Point(432, 70));
	_object6.setZoom(65);
	_object6.fixPriority(0);
	_object7.postInit();
	_object7.setVisage(945);
	_object7.setStrip(1);
	_object7.setPosition(Common::Point(423, 131));
	_object7.setZoom(65);
	_object7.fixPriority(0);
	_object8.postInit();
	_object8.setVisage(945);
	_object8.setStrip(2);
	_object8.setPosition(Common::Point(420, 99));
	_object8.setZoom(65);
	_object8.fixPriority(3);
	_object9.postInit();
	_object9.setVisage(945);
	_object9.setStrip(3);
	_object9.setPosition(Common::Point(458, 65));
	_object9.setZoom(55);
	_object9.fixPriority(0);
	_object10.postInit();
	_object10.setVisage(945);
	_object10.setStrip(3);
	_object10.setFrame(2);
	_object10.setPosition(Common::Point(465, 118));
	_object10.setZoom(55);
	_object10.fixPriority(0);
	_item1.setBounds(Rect(0, 0, 320, 200));
	BF_GLOBALS._sceneItems.push_back(&_item1);
	_sceneBounds.moveTo(320, 0);
	_gameTextSpeaker1._textPos.x = _sceneBounds.left + 10;
	_gameTextSpeaker1._color1 = 16;
	_gameTextSpeaker1._textWidth = 265;
	setAction(&_action1);
}

void Scene940::remove() {
	// clearScren();
	BF_GLOBALS._scrollFollower = &BF_GLOBALS._player;
	SceneExt::remove();
	T2_GLOBALS._uiElements._active = true;
}

} // End of namespace BlueForce
} // End of namespace TsAGE
