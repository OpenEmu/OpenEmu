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
#include "tsage/ringworld2/ringworld2_dialogs.h"
#include "tsage/ringworld2/ringworld2_scenes0.h"
#include "tsage/ringworld2/ringworld2_speakers.h"

namespace TsAGE {

namespace Ringworld2 {

void Scene50::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(2);
		break;
	case 1:
		setDelay(180);
		break;
	case 2:
		R2_GLOBALS._sceneManager.changeScene(100);
		break;
	default:
		break;
	}
}

void Scene50::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit(OwnerList);
	loadScene(110);

	R2_GLOBALS._uiElements._active = false;
	R2_GLOBALS._scenePalette.loadPalette(0);

	R2_GLOBALS._sound2.play(10);
	R2_GLOBALS._player.disableControl();

	setAction(&_action1);
}

void Scene50::process(Event &event) {
	if ((event.eventType != EVENT_BUTTON_DOWN) && (event.eventType != EVENT_KEYPRESS) && (event.eventType == EVENT_UNK27)) {
		event.handled = true;
		g_globals->_events.setCursor(CURSOR_ARROW);
		HelpDialog::show();
		R2_GLOBALS._sceneManager.changeScene(100);
	}
}

/*--------------------------------------------------------------------------
 * Scene 100 - Quinn's Room
 *
 *--------------------------------------------------------------------------*/

bool Scene100::Door::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_state) {
			SceneItem::display2(100, 6);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 101;
			scene->setAction(&scene->_sequenceManager1, scene, 101, &R2_GLOBALS._player, this, NULL);
		}
		return true;
	case CURSOR_TALK:
		if (_state) {
			SceneItem::display2(100, 26);
			_state = 0;
			scene->_doorDisplay.setFrame(1);
		} else {
			SceneItem::display2(100, 27);
			_state = 1;
			scene->_doorDisplay.setFrame(2);
		}
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::Table::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		if (_strip == 2) {
			scene->_sceneMode = 108;
			scene->_object3.postInit();
			scene->_stasisNegator.postInit();

			if (R2_INVENTORY.getObjectScene(R2_NEGATOR_GUN) == 1) {
				scene->_stasisNegator.setup(100, 7, 2);
			} else {
				scene->_stasisNegator.setup(100, 7, 1);
				scene->_stasisNegator.setDetails(100, 21, 22, 23, 2, (SceneItem *)NULL);
			}

			scene->setAction(&scene->_sequenceManager2, scene, 108, this, &scene->_object3,
				&scene->_stasisNegator, &R2_GLOBALS._player, NULL);
		} else {
			scene->_sceneMode = 109;
			scene->setAction(&scene->_sequenceManager2, scene, 109, this, &scene->_object3,
				&scene->_stasisNegator, &R2_GLOBALS._player, NULL);
		}
		return true;
	case CURSOR_TALK:
		R2_GLOBALS._player.disableControl();

		if (_strip == 2) {
			SceneItem::display2(100, 18);
			scene->_sceneMode = 102;
			scene->_object3.postInit();
			scene->_stasisNegator.postInit();

			if (R2_INVENTORY.getObjectScene(R2_NEGATOR_GUN) == 1) {
				scene->_stasisNegator.setup(100, 7, 2);
			} else {
				scene->_stasisNegator.setup(100, 7, 1);
				scene->_stasisNegator.setDetails(100, 21, 22, 23, 2, (SceneItem *)NULL);
			}

			scene->setAction(&scene->_sequenceManager2, scene, 102, this, &scene->_object3,
				&scene->_stasisNegator, NULL);
		} else {
			SceneItem::display2(100, 19);
			scene->_sceneMode = 103;
			scene->setAction(&scene->_sequenceManager2, scene, 103, this, &scene->_object3,
				&scene->_stasisNegator, NULL);
		}
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::StasisNegator::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 107;
		scene->setAction(&scene->_sequenceManager1, scene, 107, &R2_GLOBALS._player, &scene->_stasisNegator, NULL);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::DoorDisplay::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(100, _state ? 24 : 25);
		return true;
	case CURSOR_TALK:
		SceneItem::display2(100, _state ? 26 : 27);
		return true;
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 110;
		scene->setAction(&scene->_sequenceManager1, scene, 110, &R2_GLOBALS._player, NULL);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::SteppingDisks::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 111;
		scene->setAction(&scene->_sequenceManager1, scene, 111, &R2_GLOBALS._player, this, NULL);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene100::Terminal::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 105;
		scene->setAction(&scene->_sequenceManager1, scene, 105, &R2_GLOBALS._player, this, NULL);
		return true;
	default:
		return NamedHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene100::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(100);
	R2_GLOBALS._scenePalette.loadPalette(0);

	if (R2_GLOBALS._sceneManager._previousScene != 125)
		R2_GLOBALS._sound1.play(10);

	_door.postInit();
	_door._state = 0;
	_door.setVisage(100);
	_door.setPosition(Common::Point(160, 84));
	_door.setDetails(100, 3, 4, 5, 1, (SceneItem *)NULL);

	_doorDisplay.postInit();
	_doorDisplay.setup(100, 2, 1);
	_doorDisplay.setPosition(Common::Point(202, 53));
	_doorDisplay.setDetails(100, -1, -1, -1, 1, (SceneItem *)NULL);

	_table.postInit();
	_table.setup(100, 2, 3);
	_table.setPosition(Common::Point(175, 157));
	_table.setDetails(100, 17, 18, 20, 1, (SceneItem *)NULL);

	_bedLights1.postInit();
	_bedLights1.setup(100, 3, 1);
	_bedLights1.setPosition(Common::Point(89, 79));
	_bedLights1.fixPriority(250);
	_bedLights1.animate(ANIM_MODE_2, NULL);
	_bedLights1._numFrames = 3;

	_bedLights2.postInit();
	_bedLights2.setup(100, 3, 1);
	_bedLights2.setPosition(Common::Point(89, 147));
	_bedLights2.fixPriority(250);
	_bedLights2.animate(ANIM_MODE_7, 0, NULL);
	_bedLights2._numFrames = 3;

	_wardrobe.postInit();
	_wardrobe.setVisage(101);
	_wardrobe.setPosition(Common::Point(231, 126));
	_wardrobe.fixPriority(10);
	_wardrobe.setDetails(100, 37, -1, 39, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_STEPPING_DISKS) == 100) {
		_steppingDisks.postInit();
		_steppingDisks.setup(100, 8, 1);
		_steppingDisks.setPosition(Common::Point(274, 130));
		_steppingDisks.setDetails(100, 40, -1, 42, 1, (SceneItem *)NULL);
	}

	_terminal.setDetails(11, 100, 14, 15, 16);
	_desk.setDetails(12, 100, 11, -1, 13);
	_bed.setDetails(13, 100, 8, 9, 10);
	_duct.setDetails(14, 100, 34, -1, 36);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 100, 0, 1, -1, 1, NULL);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 50:
	case 180:
		_object5.postInit();
		_object4.postInit();
		_sceneMode = 104;
		setAction(&_sequenceManager1, this, 104, &R2_GLOBALS._player, &_wardrobe, &_object4, &_object5, NULL);
		break;
	case 125:
		_sceneMode = 100;
		setAction(&_sequenceManager1, this, 106, &R2_GLOBALS._player, NULL);
		break;
	case 200:
		_sceneMode = 100;
		setAction(&_sequenceManager1, this, 100, &R2_GLOBALS._player, &_door, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(180, 100));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene100::remove() {
	R2_GLOBALS._sound1.play(10);
	SceneExt::remove();
}

void Scene100::signal() {
	switch (_sceneMode) {
	case 101:
		R2_GLOBALS._sceneManager.changeScene(200);
		break;
	case 103:
	case 109:
		_table.setStrip(2);
		_table.setFrame(3);

		_object3.remove();
		_stasisNegator.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 104:
		_sceneMode = 0;
		_object5.remove();
		_object4.remove();

		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player.fixPriority(-1);
		R2_GLOBALS._player.enableControl();
		break;
	case 105:
		R2_GLOBALS._sceneManager.changeScene(125);
		break;
	case 107:
		R2_GLOBALS._sceneItems.remove(&_stasisNegator);

		_stasisNegator.setFrame(2);
		R2_INVENTORY.setObjectScene(R2_NEGATOR_GUN, 1);
		R2_GLOBALS._player.enableControl();
		break;
	case 110:
		if (_door._state) {
			_door._state = 0;
			_doorDisplay.setFrame(1);
		} else {
			_door._state = 1;
			_doorDisplay.setFrame(2);
		}
		R2_GLOBALS._player.enableControl();
		break;
	case 111:
		R2_INVENTORY.setObjectScene(R2_STEPPING_DISKS, 1);
		_steppingDisks.remove();
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene100::dispatch() {
	int regionIndex = R2_GLOBALS._player.getRegionIndex();
	if (regionIndex == 13)
		R2_GLOBALS._player._shade = 4;

	if ((R2_GLOBALS._player._visage == 13) || (R2_GLOBALS._player._visage == 101))
	R2_GLOBALS._player._shade = 0;

	SceneExt::dispatch();

	if ((_sceneMode == 101) && (_door._frame == 2) && (_table._strip == 5)) {
		_table.setAction(&_sequenceManager2, NULL, 103, &_table, &_object3, &_stasisNegator, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 125 - Computer Terminal
 *
 *--------------------------------------------------------------------------*/

bool Scene125::Object5::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE)
		return true;
	else
		return SceneActor::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

Scene125::Icon::Icon(): SceneActor()  {
	_lookLineNum = 0;
	_field98 = 0;
	_pressed = false;
}

void Scene125::Icon::postInit(SceneObjectList *OwnerList) {
	SceneObject::postInit();

	_object1.postInit();
	_object1.fixPriority(255);
	_object1.hide();

	_sceneText1._color1 = 92;
	_sceneText1._color2 = 0;
	_sceneText1._width = 200;
	_sceneText2._color1 = 0;
	_sceneText2._color2 = 0;
	_sceneText2._width = 200;
	setDetails(125, -1, -1, -1, 2, (SceneItem *)NULL);
}

void Scene125::Icon::synchronize(Serializer &s) {
	SceneActor::synchronize(s);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_field98);
	s.syncAsSint16LE(_pressed);
}

void Scene125::Icon::process(Event &event) {
	Scene125 *scene = (Scene125 *)R2_GLOBALS._sceneManager._scene;

	if (!event.handled && !(_flags & OBJFLAG_HIDING) && R2_GLOBALS._player._uiEnabled) {

		if (event.eventType == EVENT_BUTTON_DOWN) {
			int regionIndex = R2_GLOBALS._sceneRegions.indexOf(event.mousePos);

			switch (R2_GLOBALS._events.getCursor()) {
			case CURSOR_LOOK:
				if (regionIndex == _sceneRegionId) {
					event.handled = true;
					if (_lookLineNum == 26) {
						SceneItem::display2(130, 7);
					} else {
						SceneItem::display2(130, _lookLineNum);
					}
				}
				break;

			case CURSOR_USE:
				if ((regionIndex == _sceneRegionId) && !_pressed) {
					scene->_sound1.play(14);
					setFrame(2);

					switch (_object1._strip) {
					case 1:
						_object1.setStrip(2);
						break;
					case 3:
						_object1.setStrip(4);
						break;
					case 5:
						_object1.setStrip(6);
						break;
					default:
						break;
					}

					_pressed = true;
					event.handled = true;
				}
				break;

			default:
				break;
			}
		}

		if ((event.eventType == EVENT_BUTTON_UP) && _pressed) {
			setFrame(1);

			switch (_object1._strip) {
			case 2:
				_object1.setStrip(1);
				break;
			case 4:
				_object1.setStrip(3);
				break;
			case 6:
				_object1.setStrip(5);
				break;
			default:
				break;
			}

			_pressed = false;
			event.handled = true;
			scene->consoleAction(_lookLineNum);
		}
	}
}

void Scene125::Icon::setIcon(int id) {
	Scene125 *scene = (Scene125 *)R2_GLOBALS._sceneManager._scene;

	_lookLineNum = _field98 = id;
	SceneActor::_lookLineNum = id;

	_sceneText1.remove();
	_sceneText2.remove();

	if (_lookLineNum) {
		showIcon();
		_object1.setup(161, ((id - 1) / 10) * 2 + 1, ((id - 1) % 10) + 1);
		_object1.setPosition(_position);

		_sceneText1._fontNumber = scene->_iconFontNumber;
		_sceneText1.setup(CONSOLE125_MESSAGES[id]);
		_sceneText1.fixPriority(20);

		_sceneText2._fontNumber = scene->_iconFontNumber;
		_sceneText2.setup(CONSOLE125_MESSAGES[id]);
		_sceneText2.fixPriority(20);

		_sceneText2._fontNumber = scene->_iconFontNumber;
		_sceneText2.setup(CONSOLE125_MESSAGES[id]);
		_sceneText2.fixPriority(10);

		switch (_lookLineNum) {
		case 5:
			_sceneText1.setPosition(Common::Point(62, _position.y + 8));
			_sceneText2.setPosition(Common::Point(64, _position.y + 10));
			break;
		case 6:
		case 7:
		case 24:
		case 25:
			_sceneText1.setPosition(Common::Point(65, _position.y + 8));
			_sceneText2.setPosition(Common::Point(67, _position.y + 10));
			break;
		case 26:
			_sceneText1.setPosition(Common::Point(83, _position.y + 8));
			_sceneText2.setPosition(Common::Point(85, _position.y + 10));
			break;
		default:
			_sceneText1.setPosition(Common::Point(121, _position.y + 8));
			_sceneText2.setPosition(Common::Point(123, _position.y + 10));
			break;
		}
	} else {
		hideIcon();
	}
}

void Scene125::Icon::showIcon() {
	_sceneText1.show();
	_sceneText2.show();
	_object1.show();
	_object2.show();
	show();
}

void Scene125::Icon::hideIcon() {
	_sceneText1.hide();
	_sceneText2.hide();
	_object1.hide();
	_object2.hide();
	hide();
}

/*--------------------------------------------------------------------------*/

bool Scene125::Item4::startAction(CursorType action, Event &event) {
	Scene125 *scene = (Scene125 *)R2_GLOBALS._sceneManager._scene;
	switch (action) {
	case CURSOR_USE:
		if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == R2_GLOBALS._player._oldCharacterScene[1]) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 126;
			scene->setAction(&scene->_sequenceManager, scene, 126, &scene->_object7, NULL);
			return true;
		}
		break;
	case R2_OPTO_DISK:
		if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 1) {
			R2_GLOBALS._player.disableControl();
			scene->_object7.postInit();
			scene->_sceneMode = 125;
			scene->setAction(&scene->_sequenceManager, scene, 125, &scene->_object7, NULL);
			return true;
		}
		break;
	default:
		break;
	}

	return SceneHotspot::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

Scene125::Scene125(): SceneExt() {
	_iconFontNumber = 50;
	_consoleMode = 5;
	_logIndex = _databaseIndex = _infodiskIndex = 0;

	_soundCount = _soundIndex = 0;
	for (int i = 0; i < 10; ++i)
		_soundIndexes[i] = 0;
}

void Scene125::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(160);
	_palette.loadPalette(0);

	if (R2_GLOBALS._sceneManager._previousScene != 125)
		// Save the prior scene to return to when the console is turned off
		R2_GLOBALS._player._oldCharacterScene[1] = R2_GLOBALS._sceneManager._previousScene;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == R2_GLOBALS._player._oldCharacterScene[1]) {
		_object7.postInit();
		_object7.setup(160, 3, 5);
		_object7.setPosition(Common::Point(47, 167));
	}

	_object6.postInit();
	_object6.setup(162, 1, 1);
	_object6.setPosition(Common::Point(214, 168));

	_item4.setDetails(Rect(27, 145, 81, 159), 126, 9, -1, -1, 1, NULL);
	_item3.setDetails(Rect(144, 119, 286, 167), 126, 6, 7, 8, 1, NULL);
	_item2.setDetails(1, 126, 3, 4, 5);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 126, 0, 1, -1, 1, NULL);

	_sceneMode = 1;
	signal();
}

void Scene125::signal() {
	switch (_sceneMode) {
	case 1:
		_icon1.postInit();
		_icon1._sceneRegionId = 2;
		_icon2.postInit();
		_icon2._sceneRegionId = 3;
		_icon3.postInit();
		_icon3._sceneRegionId = 4;
		_icon4.postInit();
		_icon4._sceneRegionId = 5;

		_sceneMode = 2;
		setAction(&_sequenceManager, this, 127, &_icon1, &_icon2, &_icon3, &_icon4, &R2_GLOBALS._player, NULL);
		break;
	case 2:
		_icon1.setup(160, 1, 1);
		_icon1.setPosition(Common::Point(65, 17));
		_icon1._object2.postInit();
		_icon1._object2.setup(160, 7, 1);
		_icon1._object2.setPosition(Common::Point(106, 41));

		_icon2.setup(160, 1, 1);
		_icon2.setPosition(Common::Point(80, 32));
		_icon2._object2.postInit();
		_icon2._object2.setup(160, 7, 2);
		_icon2._object2.setPosition(Common::Point(106, 56));

		_icon3.setup(160, 1, 1);
		_icon3.setPosition(Common::Point(65, 47));
		_icon3._object2.postInit();
		_icon3._object2.setup(160, 7, 1);
		_icon3._object2.setPosition(Common::Point(106, 71));

		_icon4.setup(160, 1, 1);
		_icon4.setPosition(Common::Point(80, 62));
		_icon4._sceneRegionId = 5;
		_icon4._object2.postInit();
		_icon4._object2.setup(160, 7, 2);
		_icon4._object2.setPosition(Common::Point(106, 86));

		_icon5.postInit();
		_icon5.setup(160, 1, 1);
		_icon5.setPosition(Common::Point(37, 92));
		_icon5.setIcon(6);
		_icon5._sceneRegionId = 7;

		_icon6.postInit();
		_icon6.setup(160, 1, 1);
		_icon6.setPosition(Common::Point(106, 110));
		_icon6.setIcon(5);
		_icon6._sceneRegionId = 8;

		consoleAction(5);
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 10:
 		switch (_consoleMode) {
		case 12:
			_sceneMode = 129;

			_object1.postInit();
			_object2.postInit();
			_object3.postInit();

			if (R2_GLOBALS.getFlag(13)) {
				_object4.postInit();
				setAction(&_sequenceManager, this, 130, &R2_GLOBALS._player, &_object1, &_object2,
					&_object3, &_object4, NULL);
			} else {
				setAction(&_sequenceManager, this, 129, &R2_GLOBALS._player, &_object1, &_object2, &_object3, NULL);
			}
			break;
		case 13:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_infodiskIndex = 0;
			setDetails(129, 0);
			break;
		case 23:
			R2_GLOBALS._sceneManager.changeScene(1330);
			break;
		case 27:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_databaseIndex = 0;
			setDetails(128, 0);
			break;
		case 28:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_databaseIndex = 37;
			setDetails(128, 37);
			break;
		case 29:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_databaseIndex = 68;
			setDetails(128, 68);
			break;
		case 30:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_databaseIndex = 105;
			setDetails(128, 105);
			break;
		default:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_logIndex = 0;
			setDetails(127, 0);
			break;
		}
		break;
	case 11:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;

		if ((_consoleMode >= 27) && (_consoleMode <= 30)) {
			consoleAction(11);
		}
		consoleAction(2);
		_icon6.setIcon(5);
		break;
	case 12:
		if (_soundCount > 0)
			--_soundCount;
		if (!_soundCount || (R2_GLOBALS._speechSubtitles & SPEECH_VOICE)) {
			_soundIndex = 0;
			R2_GLOBALS._playStream.stop();
		} else {
			_sceneMode = 12;
			R2_GLOBALS._playStream.play(_soundIndexes[_soundIndex++], this);
		}
		break;
	case 125:
		R2_INVENTORY.setObjectScene(R2_OPTO_DISK, R2_GLOBALS._player._oldCharacterScene[1]);
		break;
	case 126:
		R2_INVENTORY.setObjectScene(R2_OPTO_DISK, 1);
		_object7.remove();
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 128:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		SceneItem::display2(126, 12);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene125::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_consoleMode);
	s.syncAsSint16LE(_iconFontNumber);
	s.syncAsSint16LE(_logIndex);
	s.syncAsSint16LE(_databaseIndex);
	s.syncAsSint16LE(_infodiskIndex);
	s.syncAsSint16LE(_soundCount);
	s.syncAsSint16LE(_soundIndex);

	for (int i = 0; i < 10; ++i)
		s.syncAsSint16LE(_soundIndexes[i]);
}

void Scene125::process(Event &event) {
	SceneExt::process(event);

	if (R2_GLOBALS._player._uiEnabled) {
		_icon1.process(event);
		_icon2.process(event);
		_icon3.process(event);
		_icon4.process(event);
		_icon5.process(event);
		_icon6.process(event);
	}
}

void Scene125::dispatch() {
	if (_soundCount)
		R2_GLOBALS._playStream.proc1();

	Scene::dispatch();
}

/**
 * Handles actions on the console screen.
 */
void Scene125::consoleAction(int id) {
	_icon3.setIcon(0);
	_icon4.setIcon(0);

	if (id == 5)
		_icon5.setIcon(6);
	else {
		switch (_consoleMode) {
		case 10:
		case 12:
		case 13:
		case 27:
		case 28:
		case 29:
		case 30:
			break;
		default:
			_icon5.setIcon(7);
			break;
		}
	}

	switch (id) {
	case 1:
		_icon1.setIcon(8);
		_icon2.setIcon(9);
		break;
	case 2:
		_icon1.setIcon(10);
		_icon2.setIcon(11);
		_icon3.setIcon(12);
		_icon4.setIcon(13);
		break;
	case 3:
		_icon1.setIcon(15);
		_icon2.setIcon(16);
		_icon3.setIcon(17);
		break;
	case 4:
		_icon1.setIcon(22);
		_icon2.setIcon(23);
		break;
	case 6:
		R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._player._oldCharacterScene[1]);
		break;
	case 7:
		if (_consoleMode == 11)
			consoleAction(2);
		else if (_consoleMode == 22)
			consoleAction(4);
		else
			consoleAction(5);
		break;
	case 8:
		_iconFontNumber = 50;
		stop();
		_icon6.setIcon(5);
		consoleAction(1);
		break;
	case 9:
		_iconFontNumber = 52;
		stop();
		_icon6.setIcon(5);
		consoleAction(1);
		break;
	case 10:
		R2_GLOBALS._player.disableControl();
		consoleAction(2);
		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();
		_icon5.setIcon(24);

		_icon4.setPosition(Common::Point(52, 107));
		_icon4._sceneRegionId = 9;
		_icon4.setIcon(25);
		_icon4._object2.hide();

		_icon6.setIcon(26);
		_sceneMode = 10;

		_palette.loadPalette(161);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		break;
	case 11:
		_icon1.setIcon(27);
		_icon2.setIcon(28);
		_icon3.setIcon(29);
		_icon4.setIcon(30);
		break;
	case 12:
		R2_GLOBALS._player.disableControl();
		consoleAction(2);
		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();
		_icon4.hideIcon();
		_icon5.hideIcon();

		_icon6.setIcon(26);
		_sceneMode = 10;
		_palette.loadPalette(161);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		break;
	case 13:
		consoleAction(2);
		if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) != R2_GLOBALS._player._oldCharacterScene[1]) {
			SceneItem::display2(126, 17);
		} else {
			R2_GLOBALS._player.disableControl();

			_icon1.hideIcon();
			_icon2.hideIcon();
			_icon3.hideIcon();
			_icon5.setIcon(24);

			_icon4.setPosition(Common::Point(52, 107));
			_icon4._sceneRegionId = 9;
			_icon4.setIcon(25);
			_icon4._object2.hide();

			_icon6.setIcon(26);
			_sceneMode = 10;

			_palette.loadPalette(161);
			R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		}
		break;
	case 15:
		consoleAction(3);

		if (R2_GLOBALS._v565F5 < 3) {
			R2_GLOBALS._player.disableControl();
			_object5.postInit();
			_object5.setup(162, 2, 2);
			_object5.setPosition(Common::Point(216, UI_INTERFACE_Y));

			R2_GLOBALS._v565F5 += 2;
		} else if (R2_GLOBALS._v565F5 == 3) {
			SceneItem::display2(126, 13);
		} else {
			SceneItem::display2(126, 14);
		}
		break;
	case 16:
		consoleAction(3);

		if (R2_GLOBALS._v565F5 < 4) {
			R2_GLOBALS._player.disableControl();
			_object5.postInit();
			_object5.setup(162, 2, 3);
			_object5.setPosition(Common::Point(218, UI_INTERFACE_Y));

			++R2_GLOBALS._v565F5;
		} else {
			SceneItem::display2(126, 15);
		}
		break;
	case 17:
		consoleAction(3);

		if (R2_GLOBALS._v565F5 < 4) {
			R2_GLOBALS._player.disableControl();
			_object5.postInit();
			_object5.setup(162, 2, 1);
			_object5.setPosition(Common::Point(215, UI_INTERFACE_Y));

			++R2_GLOBALS._v565F5;
		} else {
			SceneItem::display2(126, 16);
		}
		break;
	case 22:
		_icon1.setIcon(31);
		_icon2.setIcon(32);
		_icon3.setIcon(33);
		_icon4.setIcon(34);
		break;
	case 23:
		R2_GLOBALS._player.disableControl();
		consoleAction(4);
		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();
		_icon4.hideIcon();
		_icon5.hideIcon();
		_icon6.hideIcon();

		_sceneMode = 10;
		_palette.loadPalette(161);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		break;
	case 24:
		_icon4.setIcon(25);
		_icon4._object2.hide();

		if (_consoleMode == 10) {
			setDetails(127, --_logIndex);
		} else if (_consoleMode == 13) {
			setDetails(129, --_infodiskIndex);
		} else {
			setDetails(128, --_databaseIndex);
		}
		break;
	case 25:
		_icon4.setIcon(25);
		_icon4._object2.hide();

		if (_consoleMode == 10) {
			setDetails(127, ++_logIndex);
		} else if (_consoleMode == 13) {
			setDetails(129, ++_infodiskIndex);
		} else {
			setDetails(128, ++_databaseIndex);
		}
		break;
	case 26:
		R2_GLOBALS._player.disableControl();
		stop();
		_icon4.setPosition(Common::Point(80, 62));
		_icon4._sceneRegionId = 5;
		_icon4.hideIcon();

		R2_GLOBALS._player.hide();
		_object1.hide();
		_object2.hide();
		_object3.hide();
		_object4.hide();

		_sceneMode = 11;
		_palette.loadPalette(160);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		break;
	case 27:
	case 28:
	case 29:
	case 30:
		R2_GLOBALS._player.disableControl();
		consoleAction(11);
		_consoleMode = id;

		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();
		_icon4.hideIcon();
		_icon5.setIcon(24);

		_icon4.setPosition(Common::Point(52, 107));
		_icon4._sceneRegionId = 9;
		_icon4.setIcon(25);
		_icon4._object2.hide();

		_icon6.setIcon(26);
		_sceneMode = 10;

		_palette.loadPalette(161);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		break;
	case 31:
		consoleAction(22);
		R2_GLOBALS._sound1.play((R2_GLOBALS._sound1.getSoundNum() == 10) ? 63 : 10);
		break;
	case 32:
		consoleAction(22);
		R2_GLOBALS._sound1.play((R2_GLOBALS._sound1.getSoundNum() == 10) ? 64 : 10);
		break;
	case 33:
		consoleAction(22);
		R2_GLOBALS._sound1.play((R2_GLOBALS._sound1.getSoundNum() == 10) ? 65 : 10);
		break;
	case 34:
		consoleAction(22);
		R2_GLOBALS._sound1.play((R2_GLOBALS._sound1.getSoundNum() == 10) ? 66 : 10);
		break;
	default:
		_icon1.setIcon(1);
		_icon2.setIcon(2);
		_icon3.setIcon(3);
		_icon4.setIcon(4);
		break;
	}

	if ((id != 6) && (id != 7) && (id != 24) && (id != 25))
		_consoleMode = id;
}

/**
 * Sets the message to be displayed on the console screen.
 */
void Scene125::setDetails(int resNum, int lineNum) {
	stop();

	Common::String msg = g_resourceManager->getMessage(resNum, lineNum, true);

	if (!msg.empty()) {
		// Check for any specified sound numbers embedded in the message
		msg = parseMessage(msg);

		_sceneText._fontNumber = _iconFontNumber;
		_sceneText._color1 = 92;
		_sceneText._color2 = 0;
		_sceneText._width = 221;
		_sceneText.fixPriority(20);
		_sceneText.setup(msg);
		_sceneText.setPosition(Common::Point(49, 19));

		R2_GLOBALS._sceneObjects->draw();

		if ((_soundCount > 0) && (R2_GLOBALS._speechSubtitles & SPEECH_VOICE)) {
			_sceneMode = 12;
			R2_GLOBALS._playStream.play(_soundIndexes[_soundIndex], this);
		}
	} else {
		// Passed the start or end of the message set, so return to the menu
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.hide();

		_icon4.setPosition(Common::Point(80, 62));
		_icon4._sceneRegionId = 5;
		_icon4.hideIcon();

		_consoleMode = 0;
		_palette.loadPalette(160);
		_sceneMode = 11;
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
	}
}

/**
 * Stops any playing console sounds and hides any current console message.
 */
void Scene125::stop() {
	_sceneText.remove();
	_soundIndex = 0;
	_soundCount = 0;

	R2_GLOBALS._playStream.stop();
}

/**
 * Parses a message to be displayed on the console to see whether there are any sounds to be played.
 */
Common::String Scene125::parseMessage(const Common::String &msg) {
	_soundIndex = 0;
	_soundCount = 0;

	const char *msgP = msg.c_str();
	while (*msgP == '!') {
		// Get the sound number
		_soundIndexes[_soundCount++] = atoi(++msgP);

		while (!((*msgP == '\0') || (*msgP < '0') || (*msgP > '9')))
			++msgP;
	}

	return Common::String(msgP);
}

/*--------------------------------------------------------------------------
 * Scene 150 - Empty Bedroom
 *
 *--------------------------------------------------------------------------*/

void Scene150::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(100);

	_door.postInit();
	_door._state = 0;
	_door.setVisage(100);
	_door.setPosition(Common::Point(160, 84));
	_door.setDetails(100, 3, -1, -1, 1, (SceneItem *)NULL);

	_doorDisplay.postInit();
	_doorDisplay.setup(100, 2, 1);
	_doorDisplay.setPosition(Common::Point(202, 53));
	_doorDisplay.setDetails(100, -1, -1, -1, 1, (SceneItem *)NULL);

	_emptyRoomTable.postInit();
	_emptyRoomTable.setVisage(100);
	_emptyRoomTable.setStrip(4);
	_emptyRoomTable.setFrame(1);
	_emptyRoomTable.setPosition(Common::Point(175, 157));
	_emptyRoomTable.setDetails(150, 3, 4, 5, 1, (SceneItem *)NULL);

	_wardrobe.postInit();
	_wardrobe.setVisage(101);
	_wardrobe.setPosition(Common::Point(231, 126));
	_wardrobe.fixPriority(10);
	_wardrobe.setDetails(100, 37, -1, 39, 1, (SceneItem *)NULL);

	_terminal.setDetails(11, 100, 14, 15, 16);
	_desk.setDetails(12, 100, 11, -1, 13);
	_bed.setDetails(13, 100, 8, 9, 10);
	_duct.setDetails(14, 100, 34, -1, 36);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 150, 0, 1, -1, 1, NULL);
	_sceneMode = 100;

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 100:
		setAction(&_sequenceManager1, this, 106, &R2_GLOBALS._player, NULL);
		break;
	case 200:
		setAction(&_sequenceManager1, this, 100, &R2_GLOBALS._player, &_door, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(180, 100));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene150::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene150::signal() {
	switch (_sceneMode) {
	case 101:
		R2_GLOBALS._sceneManager.changeScene(200);
		break;
	case 105:
		R2_GLOBALS._sceneManager.changeScene(125);
		break;
	case 110:
		if (_door._state) {
			_door._state = 0;
			_doorDisplay.setFrame(1);
		} else {
			_door._state = 1;
			_doorDisplay.setFrame(2);
		}
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 160 - Credits
 *
 *--------------------------------------------------------------------------*/

void Scene160::Action1::signal() {
	Scene160 *scene = (Scene160 *)R2_GLOBALS._sceneManager._scene;
	bool breakFlag;
	SynchronizedList<SceneText *>::iterator i;
	SceneText *topItem;

	switch (_actionIndex) {
	case 0:
		scene->_yChange = 1;
		scene->_lineNum = 0;
		++_actionIndex;
		// Deliberate fall-through

	case 1:
		setDelay(5);
		breakFlag = true;
		do {
			if (!scene->_lineNum || ((scene->_lineNum != -1) &&
					(((*scene->_creditsList.reverse_begin())->_position.y < 164) || !breakFlag))) {
				breakFlag = true;
				Common::String msg = g_resourceManager->getMessage(160, scene->_lineNum++);

				if (*msg.c_str() == '^') {
					scene->_lineNum = -1;
				} else {
					if (msg.size() == 0)
						msg = " ";

					SceneText *sceneText = new SceneText();
					sceneText->_fontNumber = 50;

					switch (*msg.c_str()) {
					case '$': {
						// Centered text
						msg.deleteChar(0);
						int width = R2_GLOBALS.gfxManager()._font.getStringWidth(msg.c_str());

						sceneText->_textMode = ALIGN_CENTER;
						sceneText->setPosition(Common::Point(160 - (width / 2), 175));
						sceneText->_width = 320;
						break;
					}

					case '%': {
						// Text for position name
						msg.deleteChar(0);
						int width = R2_GLOBALS.gfxManager()._font.getStringWidth(msg.c_str());

						sceneText->_textMode = ALIGN_RIGHT;
						sceneText->setPosition(Common::Point(151 - width, 175));
						sceneText->_width = 320;
						breakFlag = false;
						break;
					}

					case '@':
						// Text for who was in the position
						msg.deleteChar(0);
						sceneText->_textMode = ALIGN_LEFT;
						sceneText->_position = Common::Point(167, 175);
						sceneText->_width = 153;
						break;

					default:
						sceneText->_width = 151;
						sceneText->setPosition(Common::Point(151, 175));
						sceneText->_textMode = ALIGN_RIGHT;
						break;
					}

					sceneText->_color1 = 191;
					sceneText->_color2 = 191;
					sceneText->_color3 = 191;
					sceneText->setup(msg);
					sceneText->_flags |= OBJFLAG_CLONED;
					sceneText->fixPriority(5);

					scene->_creditsList.push_back(sceneText);
				}
			}

		} while (!breakFlag);

		// Move all the active credits
		for (i = scene->_creditsList.begin(); i != scene->_creditsList.end(); ++i) {
			SceneObject *item = *i;
			item->setPosition(Common::Point(item->_position.x, item->_position.y - scene->_yChange));
		}

		topItem = *scene->_creditsList.begin();
		if (topItem->_position.y < 25) {
			// Credit has reached the top, so remove it
			topItem->remove();
			scene->_creditsList.remove(topItem);

			if (scene->_creditsList.size() == 0) {
				// No more items left
				setDelay(10);
				++_actionIndex;
			}
		}
		break;

	case 2:
		HelpDialog::show();
		setDelay(4);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene160::Scene160(): SceneExt() {
	_frameNumber = _yChange = 0;
	_lineNum = 0;
}

void Scene160::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(4001);

	R2_GLOBALS._player._uiEnabled = false;
	R2_GLOBALS._player.enableControl();
	R2_GLOBALS._player._canWalk = false;

	R2_GLOBALS._uiElements.hide();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;

	_lineNum = 0;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();

	_sound1.play(337);
	setAction(&_action1);
}

void Scene160::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_frameNumber);
	s.syncAsSint16LE(_yChange);
	s.syncAsSint16LE(_lineNum);
}

void Scene160::remove() {
	// Clear the credit list
	SynchronizedList<SceneText *>::iterator i;
	for (i = _creditsList.begin(); i != _creditsList.end(); ++i) {
		SceneText *item = *i;

		item->remove();
	}
	_creditsList.clear();

	_sound1.fadeOut(NULL);
	SceneExt::remove();
}

void Scene160::process(Event &event) {
	if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
		event.handled = true;
		HelpDialog::show();
	}

	if (!event.handled)
		SceneExt::process(event);
}

/*--------------------------------------------------------------------------
 * Scene 180 - Title Screen
 *
 *--------------------------------------------------------------------------*/

void Scene180::Action1::signal() {
	Scene180 *scene = (Scene180 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
	case 1:
	case 2:
		scene->_object5.setStrip((_actionIndex == 1) ? 1 : 2);
		scene->_object5.setFrame(1);
		scene->_object5.animate(ANIM_MODE_5, this);
		break;
	case 4:
		scene->_object5.setStrip(3);
		scene->_object5.setFrame(1);
		scene->_object5.animate(ANIM_MODE_5, this);
		_actionIndex = 0;
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene180::Scene180(): SceneExt(), _webbsterSpeaker(27) {
	_field412 = 0;
	_frameInc = 0;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
	_field480 = 1;
	_field482 = -1;
	_fontNumber = R2_GLOBALS.gfxManager()._font._fontNumber;

	GfxFont font;
	font.setFontNumber(7);
	_fontHeight = font.getHeight() + 1;

	_sceneMode = (R2_GLOBALS._sceneManager._previousScene == 205) ? 10 : 0;
	_gameTextSpeaker._displayMode = 9;
}

void Scene180::postInit(SceneObjectList *OwnerList) {
	loadScene(9999);
	SceneExt::postInit();

	R2_GLOBALS._uiElements._active = true;
	R2_GLOBALS._player.disableControl();

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_webbsterSpeaker);
	_stripManager.addSpeaker(&_tealSpeaker);
	_stripManager.addSpeaker(&_dutyOfficerSpeaker);

	signal();
}

void Scene180::remove() {
	_stripManager._field2E8 = -1;
//	_stripManager._field2EA = -1;
	SceneExt::remove();

	R2_GLOBALS._events.setCursor(CURSOR_WALK);
	// word_575F7 = 0;
	R2_GLOBALS._playStream.stop();
	R2_GLOBALS._sound2.fadeOut2(NULL);
	R2_GLOBALS._sound1.fadeOut2(NULL);
}

void Scene180::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_frameNumber);
	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_field480);
	s.syncAsSint16LE(_field482);
	s.syncAsSint16LE(_frameInc);
	s.syncAsSint16LE(_fontNumber);
	s.syncAsSint16LE(_fontHeight);
}

void Scene180::signal() {
	R2_GLOBALS._playStream.stop();

	switch (_sceneMode++) {
	case 0:
		setSceneDelay(6);
		break;

	case 1:
		_field412 = 1;
		R2_GLOBALS._sceneManager._hasPalette = true;
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._v = 1;
		_animationPlayer._objectMode = 1;
		R2_GLOBALS._scene180Mode = 1;

		_animationPlayer.load(1);
		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);

		R2_GLOBALS._sound1.play(1);
		break;

	case 2:
		R2_GLOBALS._scene180Mode = 1;
		R2_GLOBALS._paneRefreshFlag[0] = 3;

		if (R2_GLOBALS._sound1.isPlaying()) {
			setSceneDelay(1);
		} else {
			setSceneDelay(180);
		}
		break;

	case 3:
		R2_GLOBALS._scene180Mode = 1;

		if (R2_GLOBALS._sound1.isPlaying())
			_sceneMode = 3;

		setSceneDelay(1);
		break;

	case 4:
	case 8:
	case 30:
	case 43:
	case 47:
		_field412 = 0;
		_palette.loadPalette(0);
		_palette.loadPalette(9998);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 8, this);
		break;

	case 5:
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._v = 1;
		_animationPlayer._objectMode = 1;
		R2_GLOBALS._scene180Mode = 2;
		_animationPlayer.load(2);

		_field412 = 1;
		R2_GLOBALS._scenePalette.addFader(_animationPlayer._subData._palData, 256, 6, NULL);
		R2_GLOBALS._sound1.play(2);
		break;

	case 6:
		R2_GLOBALS._scene180Mode = 2;
		R2_GLOBALS._paneRefreshFlag[0] = 3;

		if (R2_GLOBALS._sound1.isPlaying()) {
			setSceneDelay(1);
		} else {
			setSceneDelay(180);
		}
		break;

	case 7:
		// Title screen. Wait until title music finishes playing
		R2_GLOBALS._scene180Mode = 2;
		if (R2_GLOBALS._sound1.isPlaying())
			_sceneMode = 7;
		setSceneDelay(1);
		break;

	case 9:
		R2_GLOBALS._sound1.play(3);
		clearScreen();
		setSceneDelay(2);
		break;

	case 10:
		loadScene(4002);
		R2_GLOBALS._scenePalette.loadPalette(0);
		setSceneDelay(6);
		break;

	case 11:
		_field412 = 1;
		_object4.postInit();
		_object5.postInit();
		setAction(&_sequenceManager, this, 4000, &_object4, &_object5, NULL);
		break;

	case 12:
	case 14:
	case 16:
	case 18:
	case 20:
	case 22:
	case 24:
	case 26:
	case 46:
		setSceneDelay((R2_GLOBALS._speechSubtitles & 1) ? 1 : 18);
		break;

	case 13:
		setAction(&_sequenceManager, this, 4001, &_object4, &_object5, NULL);
		break;

	case 15:
		setAction(&_sequenceManager, this, 4002, &_object4, &_object5, NULL);
		break;

	case 17:
		setAction(&_sequenceManager, this, 4003, &_object4, &_object5, NULL);
		break;

	case 19:
		setAction(&_sequenceManager, this, 4004, &_object4, &_object5, NULL);
		break;

	case 21:
		setAction(&_sequenceManager, this, 4005, &_object4, &_object5, NULL);
		break;

	case 23:
		setAction(&_sequenceManager, this, 4006, &_object4, &_object5, NULL);
		break;

	case 25:
		setAction(&_sequenceManager, this, 4007, &_object4, &_object5, NULL);
		break;

	case 27:
		_field412 = 0;
		_object4.remove();
		_object5.remove();
		setSceneDelay(2);
		break;

	case 28:
		_field412 = 0;
		_palette.loadPalette(0);
		_palette.loadPalette(9998);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 100, this);
		break;

	case 29:
		_field412 = 1;
		_animationPlayer._paletteMode = ANIMPALMODE_REPLACE_PALETTE;
		_animationPlayer._v = 1;
		_animationPlayer._objectMode = 42;
		R2_GLOBALS._scene180Mode = 3;
		_animationPlayer.load(3);
		break;

	case 31:
		R2_GLOBALS._sound2.play(7);

		_object4.postInit();
		_object4.setVisage(76);
		_object4.setStrip(1);
		_object4.setFrame(1);
		_object4.setPosition(Common::Point(288, 143));
		_object4.fixPriority(210);

		loadScene(75);

		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS._scenePalette.loadPalette(75);

		if (R2_GLOBALS._sceneManager._hasPalette)
			R2_GLOBALS._scenePalette.refresh();
		setSceneDelay(6);
		break;

	case 32:
		_field412 = 1;

		_object2.postInit();
		_object2.setPosition(Common::Point(161, 97));
		_object2.hide();

		_object3.postInit();
		_object3.setPosition(Common::Point(60, 96));
		_object3.hide();
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 11, this);
		break;

	case 33:
		_object2.hide();

		_object3.setup(76, 4, 1);
		_object3.setFrame(_object3.getFrameCount());

		_object5.postInit();
		_object5.setup(75, 1, 1);
		_object5.setPosition(Common::Point(221, 125));
		_object5.fixPriority(210);
		_object5.setAction(&_action1);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 12, this);
		break;

	case 34:
		_object2.hide();
		_object3.hide();

		_object1.postInit();
		_object1.setup(76, 2, 1);
		_object1.setPosition(Common::Point(287, 135));
		_object1.fixPriority(200);

		_sound1.play(19);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		break;

	case 35:
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 13, this);
		break;

	case 36:
		_object2.remove();
		_sound1.play(19);

		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 6, this);
		break;

	case 37:
		_field412 = 0;
		_object1.remove();
		_palette.loadPalette(9998);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 8, this);
		break;

	case 38:
		_object4.remove();
		_object5.setAction(NULL);
		_object5.remove();

		R2_GLOBALS._sound2.fadeOut2(NULL);
		R2_GLOBALS._sound1.fadeOut2(NULL);
		break;

	case 39:
		R2_GLOBALS._sound1.changeSound(8);
		setSceneDelay(1);
		break;

	case 40:
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = 1;
		R2_GLOBALS._scene180Mode = 4;
		if (_animationPlayer.load(4)) {
			_animationPlayer.dispatch();
			R2_GLOBALS._scenePalette.addFader(_animationPlayer._subData._palData, 256, 8, this);
		} else {
			_sceneMode = 43;
			setSceneDelay(1);
		}
		break;

	case 41:
		_field412 = 1;
		_animationPlayer._v = 1;
		break;

	case 42:
		R2_GLOBALS._scene180Mode = 4;
		R2_GLOBALS._paneRefreshFlag[0] = 3;
		setSceneDelay(1);
		break;

	case 44:
		loadScene(9997);
		R2_GLOBALS._scenePalette.loadPalette(9997);
		if (R2_GLOBALS._sceneManager._hasPalette)
			R2_GLOBALS._scenePalette.refresh();

		setSceneDelay(6);
		break;

	case 45:
		R2_GLOBALS._scenePalette.addFader(_animationPlayer._subData._palData, 256, 28, this);
		break;

	case 48:
		_field412 = 1;
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._v = 1;
		_animationPlayer._objectMode = 1;
		R2_GLOBALS._scene180Mode = 15;
		_animationPlayer.load(15, NULL);

		R2_GLOBALS._sound1.play(9);
		R2_GLOBALS._scenePalette.addFader(_animationPlayer._subData._palData, 256, 6, NULL);
		break;

	case 49:
		R2_GLOBALS._scene180Mode = 15;
		R2_GLOBALS._paneRefreshFlag[0] = 3;
		setSceneDelay(1);
		break;

	case 50:
		R2_GLOBALS._scene180Mode = 0;
		_field412 = 0;
		R2_GLOBALS._sceneManager.changeScene(100);
		break;
	}
}

void Scene180::setSceneDelay(int v) {
	_frameInc = v;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
}

void Scene180::process(Event &event) {
	if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
		event.handled = 1;
		if (!_field412) {
			if (R2_GLOBALS._scenePalette._listeners.size() == 0) {
				HelpDialog::show();
			}
		}
	}

	if (!event.handled)
		SceneExt::process(event);
}

void Scene180::dispatch() {
	if (_frameInc) {
		uint32 gameFrame = R2_GLOBALS._events.getFrameNumber();

		if (gameFrame >= (uint32)_frameNumber) {
			_frameInc -= gameFrame - _frameNumber;
			_frameNumber = gameFrame;

			if (_frameInc <= 0) {
				_frameInc = 0;
				signal();
			}
		}
	}

	if (_animationPlayer._v) {
		if (_animationPlayer.isCompleted()) {
			_animationPlayer._v = 0;
			_animationPlayer.close();
			_animationPlayer.remove();

			signal();
		} else {
			_animationPlayer.dispatch();
		}
	}

	Scene::dispatch();
}

void Scene180::restore() {
	R2_GLOBALS._gfxColors.background = 0;
	R2_GLOBALS._gfxColors.foreground = 0xff;
	R2_GLOBALS._fontColors.background = 0;
	R2_GLOBALS._fontColors.foreground = 0xff;

	switch (R2_GLOBALS._scene180Mode) {
	case 0:
		R2_GLOBALS._events.setCursor(SHADECURSOR_HAND);

		R2_GLOBALS._gfxColors.foreground = 4;
		R2_GLOBALS._gfxColors.background = 3;
		R2_GLOBALS._fontColors.background = 3;
		R2_GLOBALS._frameEdgeColour = 3;
		break;

	case 1:
		R2_GLOBALS._events.setCursor(R2_CURSOR_20);

		R2_GLOBALS._gfxColors.foreground = 25;
		R2_GLOBALS._gfxColors.background = 43;
		R2_GLOBALS._fontColors.background = 48;
		R2_GLOBALS._frameEdgeColour = 48;
		break;

	case 2:
		R2_GLOBALS._events.setCursor(R2_CURSOR_21);

		R2_GLOBALS._gfxColors.foreground = 106;
		R2_GLOBALS._gfxColors.background = 136;
		R2_GLOBALS._fontColors.background = 48;
		R2_GLOBALS._fontColors.foreground = 253;
		R2_GLOBALS._frameEdgeColour = 48;
		break;

	case 3:
		R2_GLOBALS._events.setCursor(R2_CURSOR_22);

		R2_GLOBALS._gfxColors.foreground = 84;
		R2_GLOBALS._gfxColors.background = 118;
		R2_GLOBALS._fontColors.background = 47;
		R2_GLOBALS._frameEdgeColour = 48;
		break;

	case 14:
		R2_GLOBALS._events.setCursor(R2_CURSOR_23);

		R2_GLOBALS._fontColors.background = 38;
		R2_GLOBALS._fontColors.foreground = 38;
		R2_GLOBALS._gfxColors.foreground = 192;
		R2_GLOBALS._gfxColors.background = 30;
		R2_GLOBALS._frameEdgeColour = 48;
		break;

	default:
		R2_GLOBALS._gfxColors.background = 0;
		R2_GLOBALS._gfxColors.foreground = 59;
		R2_GLOBALS._fontColors.background = 4;
		R2_GLOBALS._fontColors.foreground = 15;

		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 200 - Ship Corridor
 *
 *--------------------------------------------------------------------------*/

bool Scene200::NorthDoor::startAction(CursorType action, Event &event) {
	Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 202;
		scene->setAction(&scene->_sequenceManager, scene, 202, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene200::EastDoor::startAction(CursorType action, Event &event) {
	Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 200;
		scene->setAction(&scene->_sequenceManager, scene, 200, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene200::WestDoor::startAction(CursorType action, Event &event) {
	Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 204;
		scene->setAction(&scene->_sequenceManager, scene, 204, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene200::EastExit::changeScene() {
	Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 206;
	scene->setAction(&scene->_sequenceManager, scene, 206, &R2_GLOBALS._player, NULL);
}

void Scene200::WestExit::changeScene() {
	Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 208;
	scene->setAction(&scene->_sequenceManager, scene, 208, &R2_GLOBALS._player, NULL);
}

/*--------------------------------------------------------------------------*/

void Scene200::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(200);

	_westExit.setDetails(Rect(94, 0, 123, 58), EXITCURSOR_W, 175);
	_westExit.setDest(Common::Point(125, 52));
	_eastExit.setDetails(Rect(133, 0, 167, 58), EXITCURSOR_E, 150);
	_eastExit.setDest(Common::Point(135, 52));

	_northDoor.postInit();
	_northDoor.setVisage(200);
	_northDoor.setPosition(Common::Point(188, 79));
	_northDoor.setDetails(200, 3, -1, -1, 1, (SceneItem *)NULL);

	_eastDoor.postInit();
	_eastDoor.setVisage(200);
	_eastDoor.setStrip(2);
	_eastDoor.setPosition(Common::Point(305, 124));
	_eastDoor.setDetails(200, 6, -1, -1, 1, (SceneItem *)NULL);

	_westDoor.postInit();
	_westDoor.setVisage(200);
	_westDoor.setStrip(3);
	_westDoor.setPosition(Common::Point(62, 84));
	_westDoor.setDetails(200, 9, -1, -1, 1, (SceneItem *)NULL);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_compartment.setDetails(Rect(4, 88, 153, 167), 200, 12, -1, -1, 1, NULL);
	_westDoorDisplay.setDetails(Rect(41, 51, 48, 61), 200, 15, -1, -1, 1, NULL);
	_eastDoorDisplay.setDetails(Rect(279, 67, 286, 78), 200, 18, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 200, 0, -1, -1, 1, NULL);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 100:
		_sceneMode = 201;
		setAction(&_sequenceManager, this, 201, &R2_GLOBALS._player, &_eastDoor, NULL);
		break;
	case 150:
		_sceneMode = 207;
		setAction(&_sequenceManager, this, 207, &R2_GLOBALS._player, NULL);
		break;
	case 175:
		_sceneMode = 209;
		setAction(&_sequenceManager, this, 209, &R2_GLOBALS._player, NULL);
		break;
	case 250:
		_sceneMode = 203;
		setAction(&_sequenceManager, this, 203, &R2_GLOBALS._player, &_northDoor, NULL);
		break;
	case 400:
		_sceneMode = 205;
		setAction(&_sequenceManager, this, 205, &R2_GLOBALS._player, &_westDoor, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(215, 115));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene200::signal() {
	switch (_sceneMode) {
	case 200:
		R2_GLOBALS._sceneManager.changeScene(100);
		break;
	case 202:
		R2_GLOBALS._sceneManager.changeScene(250);
		break;
	case 204:
		R2_GLOBALS._sceneManager.changeScene(400);
		break;
	case 206:
		R2_GLOBALS._sceneManager.changeScene(150);
		break;
	case 208:
		R2_GLOBALS._sceneManager.changeScene(175);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 205 - Star-field Credits
 *
 *--------------------------------------------------------------------------*/

void Scene205::Action1::signal() {
	Scene205 *scene = (Scene205 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(2);
		break;

	case 1:
		scene->_yp = 100 - (scene->_fontHeight * 3 / 2);
		scene->handleText();

		for (int idx = 1; idx <= 2; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}
		setDelay(240);
		break;

	case 2:
	case 4:
	case 6:
	case 8:
	case 10:
	case 12:
	case 14:
	case 16:
	case 18:
	case 20:
		textLoop();
		setDelay(120);
		break;

	case 3:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 2) / 2);
		scene->handleText();

		++scene->_textIndex;
		scene->_yp += scene->_fontHeight;
		scene->handleText();
		setDelay(240);
		break;

	case 5:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 7) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 6; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(480);
		break;

	case 7:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 6) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 5; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(300);
		break;

	case 9:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 8) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 7; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(480);
		break;

	case 11:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 3) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 2; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(240);
		break;

	case 13:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 3) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 2; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(240);
		break;

	case 15:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 5) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 4; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(240);
		break;

	case 17:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 5) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 4; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(360);
		break;

	case 19:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 3) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 2; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(480);
		break;

	case 21:
		R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._sceneManager._previousScene);

	default:
		break;
	}
}

void Scene205::Action1::textLoop() {
	Scene205 *scene = (Scene205 *)R2_GLOBALS._sceneManager._scene;

	for (int idx = 1; idx <= 14; ++idx) {
		if (R2_GLOBALS._sceneObjects->contains(&scene->_textList[idx])) {
			scene->_textList[idx].remove();
		}
	}
}

/*--------------------------------------------------------------------------*/

Scene205::Object::Object(): SceneObject() {
	_x100 = _y100 = 0;
}

void Scene205::Object::synchronize(Serializer &s) {
	EventHandler::synchronize(s);

	s.syncAsSint32LE(_x100);
	s.syncAsSint32LE(_y100);
}

/*--------------------------------------------------------------------------*/

Scene205::Scene205(): SceneExt() {
	_yp = 0;
	_textIndex = 1;
	_lineNum = -1;

	GfxFont font;
	font.setFontNumber(4);
	_fontHeight = font.getHeight();
}

void Scene205::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(4000);
	R2_GLOBALS._player._uiEnabled = false;

	R2_GLOBALS._sound1.play(337);
	R2_GLOBALS._scenePalette.loadPalette(0);
	R2_GLOBALS._player.disableControl();

	setup();
	setAction(&_action1);
}

void Scene205::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	for (int idx = 0; idx < 3; ++idx)
		SYNC_POINTER(_objList1[idx]);
	for (int idx = 0; idx < 3; ++idx)
		SYNC_POINTER(_objList2[idx]);
	for (int idx = 0; idx < 4; ++idx)
		SYNC_POINTER(_objList3[idx]);

	s.syncAsSint16LE(_textIndex);
	s.syncAsSint16LE(_lineNum);
	s.syncAsSint16BE(_yp);
}

void Scene205::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene205::process(Event &event) {
	if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
		event.handled = true;
		R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._sceneManager._previousScene);
	} else {
		Scene::process(event);
	}
}

void Scene205::dispatch() {
	processList(_objList3, 4, Common::Rect(0, 0, 319, 200), 1, 1, 160, 100);
	processList(_objList2, 3, Common::Rect(0, 0, 319, 200), 2, 2, 160, 100);
	processList(_objList1, 3, Common::Rect(0, 0, 319, 200), 4, 3, 160, 100);

	Scene::dispatch();
}

void Scene205::setup() {
	const Common::Point pointList1[3] = { Common::Point(2, 50), Common::Point(100, 28), Common::Point(53, 15) };
	const Common::Point pointList2[3] = { Common::Point(289, 192), Common::Point(125, 60), Common::Point(130, 40) };
	const Common::Point pointList3[4] = {
		Common::Point(140, 149), Common::Point(91, 166), Common::Point(299, 46), Common::Point(314, 10)
	};

	// Set up the first object list
	for (int idx = 0; idx < 3; ++idx) {
		Object *obj = new Object();
		_objList1[idx] = obj;

		obj->postInit();
		obj->_flags |= OBJFLAG_CLONED;
		obj->setVisage(205);
		obj->_strip = 1;
		obj->_frame = 1;
		obj->setPosition(pointList1[idx]);
		obj->_x100 = obj->_position.x * 100;
		obj->_y100 = obj->_position.y * 100;
		obj->fixPriority(12);
	}

	// Setup the second object list
	for (int idx = 0; idx < 3; ++idx) {
		Object *obj = new Object();
		_objList2[idx] = obj;

		obj->postInit();
		obj->_flags |= OBJFLAG_CLONED;
		obj->setVisage(205);
		obj->_strip = 1;
		obj->_frame = 2;
		obj->setPosition(pointList2[idx]);
		obj->_x100 = obj->_position.x * 100;
		obj->_y100 = obj->_position.y * 100;
		obj->fixPriority(11);
	}

	// Setup the third object list
	for (int idx = 0; idx < 4; ++idx) {
		Object *obj = new Object();
		_objList3[idx] = obj;

		obj->postInit();
		obj->_flags |= OBJFLAG_CLONED;
		obj->setVisage(205);
		obj->_strip = 1;
		obj->_frame = 3;
		obj->setPosition(pointList3[idx]);
		obj->_x100 = obj->_position.x * 100;
		obj->_y100 = obj->_position.y * 100;
		obj->fixPriority(10);
	}
}

/**
 * Handles moving a group of stars in the scene background
 */
void Scene205::processList(Object **ObjList, int count, const Common::Rect &bounds,
						   int xMultiply, int yMultiply, int xCenter, int yCenter) {
	for (int idx = 0; idx < count; ++idx) {
		Object *obj = ObjList[idx];
		Common::Point pt(obj->_position.x - xCenter, obj->_position.y - yCenter);

		if ((obj->_position.x <= 319) && (obj->_position.x >= 0) &&
				(obj->_position.y <= 199) && (obj->_position.y >= 0)) {
			if (!pt.x && !pt.y) {
				pt.x = pt.y = 1;
			}

			pt.x *= xMultiply;
			pt.y *= yMultiply;
			obj->_x100 += pt.x;
			obj->_y100 += pt.y;
		} else {
			obj->_x100 = (bounds.left + R2_GLOBALS._randomSource.getRandomNumber(bounds.right)) * 100;
			obj->_y100 = (bounds.top + R2_GLOBALS._randomSource.getRandomNumber(bounds.bottom)) * 100;
		}

		obj->setPosition(Common::Point(obj->_x100 / 100, obj->_y100 / 100));
   }
}

void Scene205::handleText() {
	_message = g_resourceManager->getMessage(205, ++_lineNum);

	_textList[_textIndex]._fontNumber = 4;
	_textList[_textIndex]._color1 = 0;
	_textList[_textIndex]._color2 = 10;
	_textList[_textIndex]._color3 = 7;
	_textList[_textIndex]._width = 400;
	_textList[_textIndex].setup(_message);
	_textList[_textIndex].fixPriority(199);

	GfxFont font;
	font.setFontNumber(4);
	int width = font.getStringWidth(_message.c_str());

	_textList[_textIndex].setPosition(Common::Point(160 - (width / 2), _yp));
}

/*--------------------------------------------------------------------------
 * Scene 250 - Lift
 *
 *--------------------------------------------------------------------------*/

Scene250::Button::Button(): SceneActor() {
	_floorNumber = _v2 = 0;
}

void Scene250::Button::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_floorNumber);
	s.syncAsSint16LE(_v2);
}

bool Scene250::Button::startAction(CursorType action, Event &event) {
	Scene250 *scene = (Scene250 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (scene->_field414) {
			SceneItem::display2(250, 15);
		} else {
			switch (_floorNumber) {
			case 1:
			case 2:
			case 5:
			case 9:
				scene->_sound1.play(14);
				scene->changeFloor(_floorNumber);
				break;
			case 10:
				// Current Floor
				scene->_sound1.play(14);
				R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._sceneManager._previousScene);
				break;
			default:
				SceneItem::display2(250, 16);
				break;
			}
		}
		return true;

	case CURSOR_LOOK:
		switch (_floorNumber) {
		case 1:
		case 2:
		case 5:
		case 9:
			SceneItem::display2(250, 12);
			break;
		case 10:
			SceneItem::display2(250, 13);
			break;
		case 11:
			SceneItem::display2(250, 14);
			break;
		default:
			SceneItem::display2(250, 16);
			break;
		}
		return true;

	default:
		return SceneActor::startAction(action, event);
	}
}

void Scene250::Button::setFloor(int floorNumber) {
	SceneActor::postInit();
	_floorNumber = floorNumber;
	_v2 = 0;

	if (_floorNumber <= 9) {
		SceneObject::setup(250, 1, 4);

		switch (_floorNumber) {
		case 1:
		case 2:
		case 5:
		case 9:
			setFrame(6);
			break;
		default:
			break;
		}

		setPosition(Common::Point(111, (_floorNumber - 1) * 12 + 43));
		fixPriority(10);
		setDetails(250, -1, -1, -1, 1, (SceneItem *)NULL);
	}
}

/*--------------------------------------------------------------------------*/

Scene250::Scene250(): SceneExt() {
	_field412 = _field414 = _field416 = _field418 = _field41A = 0;
}

void Scene250::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_field414);
	s.syncAsSint16LE(_field416);
	s.syncAsSint16LE(_field418);
	s.syncAsSint16LE(_field41A);
}

void Scene250::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(250);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.enableControl();
	R2_GLOBALS._player._canWalk = false;

	_currentFloor.setFloor(10);
	_currentFloor.setup(250, 1, 5);
	_currentFloor.setDetails(250, 13, -1, -1, 1, (SceneItem *)NULL);

	_button1.setFloor(11);
	_button1.setup(250, 1, 3);
	_button1.setPosition(Common::Point(400, 100));
	_button1.setDetails(250, 14, -1, -1, 1, (SceneItem *)NULL);
	_button1.fixPriority(190);
	_button1.hide();

	_floor1.setFloor(1);
	_floor2.setFloor(2);
	_floor3.setFloor(3);
	_floor4.setFloor(4);
	_floor5.setFloor(5);
	_floor6.setFloor(6);
	_floor7.setFloor(7);
	_floor8.setFloor(8);
	_floor9.setFloor(9);

	_item2.setDetails(Rect(0, 0, 73, SCREEN_HEIGHT), 250, 9, -1, 9, 1, NULL);
	_item4.setDetails(Rect(239, 16, 283, 164), 250, 6, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 250, 0, 1, -1, 1, NULL);

	R2_GLOBALS._events.setCursor(CURSOR_USE);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 200:
		_field412 = 55;
		break;
	case 300:
		_field412 = 43;
		break;
	case 700:
		_field412 = 139;
		break;
	case 850:
		_field412 = 91;
		break;
	default:
		R2_GLOBALS._sceneManager._previousScene = 200;
		_field412 = 55;
		break;
	}

	_currentFloor.setPosition(Common::Point(111, _field412));
}

void Scene250::signal() {
	if (_field41A)
		_sceneMode = 20;

	switch (_sceneMode) {
	case 1:
		_sound1.play(22);
		R2_GLOBALS._player.show();
		R2_GLOBALS._player.setup(250, 1, 2);
		R2_GLOBALS._player.setPosition(Common::Point(261, 185));
		ADD_MOVER(R2_GLOBALS._player, 261, 15);

		_field416 = 0;
		_sceneMode = 2;
		break;
	case 2:
		_sceneMode = ((_field414 - 12) == _field412) ? 4 : 3;
		signal();
		break;
	case 3:
		_currentFloor.setPosition(Common::Point(111, _currentFloor._position.y + 12));
		_field412 += 12;
		R2_GLOBALS._player.setPosition(Common::Point(261, 185));
		ADD_MOVER(R2_GLOBALS._player, 261, 15);

		if ((_field414 - 12) == _field412)
			_sceneMode = 4;
		break;
	case 4:
		_sound1.play(21);

		_currentFloor.setPosition(Common::Point(111, _currentFloor._position.y + 12));
		R2_GLOBALS._player.setPosition(Common::Point(261, 185));
		ADD_MOVER(R2_GLOBALS._player, 261, 15);
		_sceneMode = 5;
		break;
	case 5:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 20;
		signal();
		break;
	case 6:
		_sound1.play(22);
		R2_GLOBALS._player.show();
		R2_GLOBALS._player.setup(250, 1, 2);
		R2_GLOBALS._player.setPosition(Common::Point(261, 15));
		ADD_MOVER(R2_GLOBALS._player, 261, 185);
		_field416 = 0;
		_sceneMode = 7;
		break;
	case 7:
		_field418 = 1;
		if ((_field414 + 12) == _field412)
			_sceneMode = 8;
		signal();
		break;
	case 8:
		_currentFloor.setPosition(Common::Point(111, _currentFloor._position.y - 12));
		_field412 -= 12;
		R2_GLOBALS._player.setPosition(Common::Point(261, 15));
		ADD_MOVER(R2_GLOBALS._player, 261, 185);

		if ((_field414 + 12) == _field412)
			_sceneMode = 9;
		break;
	case 9:
		_sound1.play(21);
		_currentFloor.setPosition(Common::Point(111, _currentFloor._position.y - 12));
		R2_GLOBALS._player.setPosition(Common::Point(261, 15));
		ADD_MOVER(R2_GLOBALS._player, 261, 185);
		_sceneMode = 10;
		break;
	case 10:
		_sceneMode = 20;
		signal();
		break;
	case 20:
		// Handle changing scene
		switch (_field414) {
		case 55:
			R2_GLOBALS._sceneManager.changeScene(200);
			break;
		case 43:
			R2_GLOBALS._sceneManager.changeScene(300);
			break;
		case 139:
			R2_GLOBALS._sceneManager.changeScene(139);
			break;
		case 91:
			R2_GLOBALS._sceneManager.changeScene(850);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void Scene250::changeFloor(int floorNumber) {
	_field414 = (floorNumber - 1) * 12 + 43;
	_button1.setPosition(Common::Point(111, _field414));
	_button1.show();

	_sceneMode = (_field412 >= _field414) ? 6 : 1;
	if (_field414 == _field412)
		_sceneMode = 20;

	signal();
}

void Scene250::process(Event &event) {
	if (!event.handled) {
		if (((event.eventType == EVENT_KEYPRESS) || (event.btnState != 0)) && _field418) {
			_field41A = 1;
			event.handled = true;
		}

		SceneExt::process(event);
	}
}

void Scene250::dispatch() {
	SceneExt::dispatch();

	if (((_sceneMode == 2) || (_sceneMode == 7)) && (_field416 < 100)) {
		++_field416;
		R2_GLOBALS._player._moveDiff.y = _field416 / 5;
	}

	if (((_sceneMode == 5) || (_sceneMode == 10)) && (R2_GLOBALS._player._moveDiff.y > 4)) {
		--_field416;
		R2_GLOBALS._player._moveDiff.y = _field416 / 7 + 3;
	}
}

/*--------------------------------------------------------------------------
 * Scene 300 - Bridge
 *
 *--------------------------------------------------------------------------*/

void Scene300::Action1::signal() {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 0:
		setAction(&scene->_sequenceManager2, this, 311, (R2_GLOBALS._player._characterIndex == 1) ?
			(SceneObject *)&R2_GLOBALS._player : (SceneObject *)&scene->_quinn);
		_actionIndex = 2;
		break;
	case 1:
		setAction(&scene->_sequenceManager2, this, 312, (R2_GLOBALS._player._characterIndex == 1) ?
			(SceneObject *)&R2_GLOBALS._player : (SceneObject *)&scene->_quinn);
		_actionIndex = 0;
		break;
	case 2:
		if (!R2_GLOBALS._playStream.isPlaying())
			_actionIndex = R2_GLOBALS._randomSource.getRandomNumber(1);
		break;
	default:
		break;
	}
}

void Scene300::Action2::signal() {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 0:
		setAction(&scene->_sequenceManager3, this, 302, &scene->_seeker, NULL);
		_actionIndex = 2;
		break;
	case 1:
		setAction(&scene->_sequenceManager3, this, 303, &scene->_seeker, NULL);
		_actionIndex = 2;
		break;
	case 2:
		if (!R2_GLOBALS._playStream.isPlaying())
			_actionIndex = R2_GLOBALS._randomSource.getRandomNumber(1);

		setDelay(60 + R2_GLOBALS._randomSource.getRandomNumber(119));
		break;
	default:
		break;
	}
}

void Scene300::Action3::signal() {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 0:
		setAction(&scene->_sequenceManager3, this, 304, &scene->_miranda, NULL);
		_actionIndex = 2;
		break;
	case 1:
		setAction(&scene->_sequenceManager3, this, 305, &scene->_miranda, NULL);
		_actionIndex = 2;
		break;
	case 2:
		if (!R2_GLOBALS._playStream.isPlaying())
			_actionIndex = R2_GLOBALS._randomSource.getRandomNumber(1);

		setDelay(60 + R2_GLOBALS._randomSource.getRandomNumber(119));
		break;
	default:
		break;
	}
}


void Scene300::Action4::signal() {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	if (!R2_GLOBALS._playStream.isPlaying()) {
		scene->_object7.setStrip2(R2_GLOBALS._randomSource.getRandomNumber(2));
		scene->_object7.setFrame(1);

		scene->_object9.setStrip2(3);
		scene->_object9.setFrame(1);
	}

	setDelay(60 + R2_GLOBALS._randomSource.getRandomNumber(479));
}

/*--------------------------------------------------------------------------*/

bool Scene300::QuinnWorkstation::startAction(CursorType action, Event &event) {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex != 1)
			SceneItem::display2(300, 46);
		else if (R2_GLOBALS.getFlag(44)) {
			R2_GLOBALS._player.setAction(NULL);
			R2_GLOBALS._sceneManager.changeScene(325);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 306;
			scene->setAction(&scene->_sequenceManager1, scene, 306, &R2_GLOBALS._player, NULL);
		}
		return true;

	case CURSOR_LOOK:
		if (R2_GLOBALS._player._characterIndex == 1) {
			SceneItem::display2(300, 47);
			return true;
		}
		break;

	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

bool Scene300::MirandaWorkstation::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex != 3)
			SceneItem::display2(300, 49);
		else
			R2_GLOBALS._sceneManager.changeScene(325);
		return true;

	case CURSOR_LOOK:
		if (R2_GLOBALS._player._characterIndex == 3) {
			SceneItem::display2(300, 47);
			return true;
		}
		break;

	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

bool Scene300::SeekerWorkstation::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		if (R2_GLOBALS._player._characterIndex == 2) {
			SceneItem::display2(300, 47);
			return true;
		}
		break;

	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex != 2)
			SceneItem::display2(300, 48);
		else
			R2_GLOBALS._sceneManager.changeScene(325);
		return true;

	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

bool Scene300::Miranda::startAction(CursorType action, Event &event) {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			// Quinn talking to Miranda
			R2_GLOBALS._player.disableControl();

			if (!R2_GLOBALS.getFlag(44)) {
				if (R2_GLOBALS.getFlag(40))
					scene->_stripId = 119;
				else if (R2_GLOBALS.getFlag(38))
					scene->_stripId = 101;
				else {
					R2_GLOBALS._sound1.play(69);
					scene->_stripId = 100;
				}

				scene->_sceneMode = 309;
				scene->setAction(&scene->_sequenceManager1, scene, 309, &R2_GLOBALS._player, NULL);
			} else if (!R2_GLOBALS.getFlag(55)) {
				R2_GLOBALS._events.setCursor(CURSOR_ARROW);
				scene->_sceneMode = 10;
				scene->_stripManager.start3(scene->_stripId, scene, R2_GLOBALS._stripManager_lookupList);
			} else {
				scene->_sceneMode = 16;

				if (!R2_GLOBALS.getFlag(57)) {
					R2_GLOBALS._events.setCursor(CURSOR_ARROW);
					scene->_stripManager.start3(434, scene, R2_GLOBALS._stripManager_lookupList);
				} else if (R2_GLOBALS._player._characterScene[R2_MIRANDA] != 500) {
					R2_GLOBALS._events.setCursor(CURSOR_ARROW);
					scene->_stripManager.start3(407, scene, R2_GLOBALS._stripManager_lookupList);
				} else {
					scene->_stripId = 433;
					scene->_sceneMode = 309;
					scene->setAction(&scene->_sequenceManager1, scene, 309, &R2_GLOBALS._player, NULL);
				}
			}
		} else {
			// Seeker talking to Miranda
			scene->_sceneMode = 10;
			R2_GLOBALS._events.setCursor(CURSOR_ARROW);

			if (!R2_GLOBALS.getFlag(44))
				scene->_stripId = 174 + R2_GLOBALS._randomSource.getRandomNumber(2);
			else if (!R2_GLOBALS.getFlag(55))
				scene->_stripId = 211;
			else
				scene->_stripId = 438;

			scene->_stripManager.start3(scene->_stripId, scene, R2_GLOBALS._stripManager_lookupList);
		}
		return true;

	case R2_OPTO_DISK:
		SceneItem::display2(300, 54);
		return true;

	case R2_READER:
		if (!R2_GLOBALS.getFlag(2) || !R2_GLOBALS.getFlag(3) || (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 1))
			SceneItem::display2(300, 55);
		else {
			R2_GLOBALS._player.disableControl();
			scene->_stripId = R2_GLOBALS.getFlag(4) ? 121 : 120;
			scene->_sceneMode = 309;
			scene->setAction(&scene->_sequenceManager1, scene, 309, &R2_GLOBALS._player, NULL);
		}
		return true;

	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

bool Scene300::Seeker::startAction(CursorType action, Event &event) {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			if (R2_GLOBALS.getFlag(44)) {
				if (!R2_GLOBALS.getFlag(38)) {
					R2_GLOBALS._sound1.play(69);
					scene->_stripId = 181;
					scene->_sceneMode = 310;
					scene->setAction(&scene->_sequenceManager1, scene, 309, &R2_GLOBALS._player, NULL);
				} else {
					scene->_stripId = R2_GLOBALS.getFlag(40) ? 170 : 150;
					scene->_sceneMode = 310;
					scene->setAction(&scene->_sequenceManager1, scene, 309, &R2_GLOBALS._player, NULL);
				}
			} else {
				R2_GLOBALS._events.setCursor(CURSOR_ARROW);
				if (!R2_GLOBALS.getFlag(55)) {
					scene->_sceneMode = 10;
					scene->_stripManager.start3(205, scene, R2_GLOBALS._stripManager_lookupList);
				} else {
					scene->_sceneMode = 16;
					scene->_stripManager.start3(R2_GLOBALS.getFlag(57) ? 407 : 401, scene, R2_GLOBALS._stripManager_lookupList);
				}
			}
		} else {
			scene->_sceneMode = 10;
			R2_GLOBALS._events.setCursor(CURSOR_ARROW);

			if (!R2_GLOBALS.getFlag(44))
				scene->_stripId = 122 + R2_GLOBALS._randomSource.getRandomNumber(2);
			else if (!R2_GLOBALS.getFlag(55))
				scene->_stripId = 209;
			else
				scene->_stripId = 440;

			scene->_stripManager.start3(scene->_stripId, scene, R2_GLOBALS._stripManager_lookupList);
		}
		return true;

	case R2_OPTO_DISK:
		if (R2_GLOBALS.getFlag(13)) {
			SceneItem::display2(300, 53);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_stripId = 171;
		}

		scene->_sceneMode = 310;
		scene->setAction(&scene->_sequenceManager1, scene, 310, &R2_GLOBALS._player, NULL);
		return true;

	case R2_READER:
		if (!R2_GLOBALS.getFlag(2) || !R2_GLOBALS.getFlag(3) || (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 1))
			break;

		R2_GLOBALS._player.disableControl();
		scene->_stripId = R2_GLOBALS.getFlag(4) ? 173 : 172;
		scene->_sceneMode = 310;
		scene->setAction(&scene->_sequenceManager1, scene, 310, &R2_GLOBALS._player, NULL);
		return true;

	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

bool Scene300::Quinn::startAction(CursorType action, Event &event) {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		scene->_sceneMode = 10;

		if (R2_GLOBALS._player._characterIndex == R2_MIRANDA) {
			if (R2_GLOBALS._player._characterScene[R2_MIRANDA] == 500)
				scene->_stripId = 442;
			else if (!R2_GLOBALS.getFlag(44))
				scene->_stripId = 177 + R2_GLOBALS._randomSource.getRandomNumber(2);
			else if (!R2_GLOBALS.getFlag(55))
				scene->_stripId = 208;
			else
				scene->_stripId = 441;
		} else if (R2_GLOBALS._player._characterScene[R2_MIRANDA] == 500) {
			scene->_stripId = 442;
		} else if (R2_GLOBALS.getFlag(44)) {
			scene->_stripId = R2_GLOBALS.getFlag(55) ? 441 : 208;
		} else {
			scene->_stripId = 125 + R2_GLOBALS._randomSource.getRandomNumber(2);
		}

		scene->_stripManager.start3(scene->_stripId, scene, R2_GLOBALS._stripManager_lookupList);
		return true;

	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene300::Doorway::startAction(CursorType action, Event &event) {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		if ((R2_GLOBALS._player._characterIndex == R2_QUINN) &&
				(!R2_GLOBALS.getFlag(44) || R2_GLOBALS._player._characterScene[R2_MIRANDA] == 500)) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 301;
			scene->setAction(&scene->_sequenceManager1, scene, 301, &R2_GLOBALS._player, this, NULL);
		} else {
			SceneItem::display2(300, 45);
		}

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

Scene300::Scene300(): SceneExt() {
	_stripId = 0;
	_rotation = NULL;
}

void Scene300::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_stripId);
	SYNC_POINTER(_rotation);
}

void Scene300::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(300);
	_sound1.play(23);
	setZoomPercents(75, 93, 120, 100);

	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._sceneManager._previousScene = 1000;
		R2_GLOBALS._player._characterIndex = R2_QUINN;
	}

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_mirandaSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_quinnLSpeaker);
	_stripManager.addSpeaker(&_seekerLSpeaker);
	_stripManager.addSpeaker(&_tealSpeaker);
	_stripManager.addSpeaker(&_soldierSpeaker);

	_rotation = R2_GLOBALS._scenePalette.addRotation(237, 254, -1);
	_rotation->setDelay(3);
	_rotation->_countdown = 1;

	if (R2_GLOBALS.getFlag(51) && !R2_GLOBALS.getFlag(25)) {
		_object1.postInit();
		_object1.setup(301, 7, 2);
		_object1.setPosition(Common::Point(65, 24));

		_object2.postInit();
		_object2.setup(301, 8, 2);
		_object2.setPosition(Common::Point(254, 24));
	}

	_doorway.postInit();
	_doorway.setVisage(300);
	_doorway.setPosition(Common::Point(159, 79));

	_object3.postInit();
	_object3.setup(300, 4, 1);
	_object3.setPosition(Common::Point(84, 48));
	_object3.animate(ANIM_MODE_2, NULL);
	_object3._numFrames = 5;

	_object4.postInit();
	_object4.setup(300, 5, 1);
	_object4.setPosition(Common::Point(236, 48));
	_object4.animate(ANIM_MODE_2, NULL);

	_protocolDisplay.postInit();
	_protocolDisplay.setup(300, 6, 1);
	_protocolDisplay.setPosition(Common::Point(287, 71));
	_protocolDisplay.animate(ANIM_MODE_7, NULL);
	_protocolDisplay._numFrames = 5;

	_object6.postInit();
	_object6.setup(300, 7, 1);
	_object6.setPosition(Common::Point(214, 37));
	_object6.animate(ANIM_MODE_2, NULL);
	_object6._numFrames = 3;

	_object7.postInit();
	_object7.setup(301, 1, 1);
	_object7.setPosition(Common::Point(39, 97));
	_object7.fixPriority(124);
	_object7.animate(ANIM_MODE_2, NULL);
	_object7._numFrames = 5;
	_object7.setAction(&_action4);

	_object8.postInit();
	_object8.setup(300, 8, 1);
	_object8.setPosition(Common::Point(105, 37));
	_object8.animate(ANIM_MODE_2, NULL);
	_object8._numFrames = 5;

	_object9.postInit();
	_object9.setup(301, 6, 1);
	_object9.setPosition(Common::Point(274, 116));
	_object9.fixPriority(143);
	_object9.animate(ANIM_MODE_2, NULL);
	_object9._numFrames = 5;

	_quinnWorkstation1.setDetails(Rect(243, 148, 315, 167), 300, 30, 31, 32, 1, NULL);
	_mirandaWorkstation1.setDetails(Rect(4, 128, 69, 167), 300, 33, 31, 35, 1, NULL);

	switch (R2_GLOBALS._player._characterIndex) {
	case 1:
		_miranda.postInit();
		_miranda.setup(302, 2, 1);
		_miranda.setPosition(Common::Point(47, 128));
		_miranda.setAction(&_action3);
		_miranda.setDetails(300, 39, 40, 41, 1, (SceneItem *)NULL);

		if ((R2_GLOBALS._player._characterScene[2] == 300) || (R2_GLOBALS._player._characterScene[2] == 325)) {
			_seeker.postInit();
			_seeker.setVisage(302);
			_seeker.setPosition(Common::Point(158, 108));
			_seeker.fixPriority(130);
			_seeker.setAction(&_action2);
			_seeker.setDetails(300, 42, 43, 44, 1, (SceneItem *)NULL);
		}

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.setVisage(10);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.disableControl();
		break;

	case 2:
		_miranda.postInit();
		_miranda.setup(302, 2, 1);
		_miranda.setPosition(Common::Point(47, 128));
		_miranda.setAction(&_action3);
		_miranda.setDetails(300, 39, 40, 41, 1, (SceneItem *)NULL);

		if ((R2_GLOBALS._player._characterScene[1] == 300) || (R2_GLOBALS._player._characterScene[1] == 325)) {
			_quinn.postInit();
			_quinn.setup(302, 3, 1);
			_quinn.setPosition(Common::Point(271, 150));
			_quinn.setAction(&_action1);
			_quinn.setDetails(300, 50, 51, 52, 1, (SceneItem *)NULL);
		}

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.setup(302, 1, 3);
		R2_GLOBALS._player.setPosition(Common::Point(158, 108));
		R2_GLOBALS._player.fixPriority(130);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;

	case 3:
		if ((R2_GLOBALS._player._characterScene[2] == 300) || (R2_GLOBALS._player._characterScene[2] == 325)) {
			_seeker.postInit();
			_seeker.setVisage(302);
			_seeker.setPosition(Common::Point(158, 108));
			_seeker.fixPriority(130);
			_seeker.setAction(&_action2);
			_seeker.setDetails(300, 42, 43, 44, 1, (SceneItem *)NULL);
		}

		if ((R2_GLOBALS._player._characterScene[1] == 300) || (R2_GLOBALS._player._characterScene[1] == 325)) {
			_quinn.postInit();
			_quinn.setup(302, 3, 1);
			_quinn.setPosition(Common::Point(271, 150));
			_quinn.setAction(&_action1);
			_quinn.setDetails(300, 50, 51, 52, 1, (SceneItem *)NULL);
		}

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.setup(302, 2, 1);
		R2_GLOBALS._player.setPosition(Common::Point(47, 128));
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;

	default:
		break;
	}

	_seekerWorkstation.setDetails(Rect(101, 95, 217, 143), 300, 36, 31, 35, 1, NULL);
	_quinnWorkstation2.setDetails(Rect(224, 102, 315, 143), 300, 30, 31, 32, 1, NULL);
	_mirandaWorkstation2.setDetails(Rect(4, 83, 84, 124), 300, 33, 31, 35, 1, NULL);
	_hull.setDetails(11, 300, 6, -1, -1);
	_statusDisplays.setDetails(12, 300, 9, 10, -1);
	_damageControl.setDetails(13, 300, 12, -1, -1);
	_manualOverrides.setDetails(14, 300, 15, -1, 17);
	_scanners1.setDetails(Rect(126, 15, 183, 25), 300, 18, -1, 20, 1, NULL);
	_scanners2.setDetails(Rect(126, 80, 183, 90), 300, 18, -1, 20, 1, NULL);
	_protocolDisplay.setDetails(300, 27, -1, 29, 1, (SceneItem *)NULL);
	_indirectLighting1.setDetails(Rect(74, 71, 122, 89), 300, 21, -1, -1, 1, NULL);
	_indirectLighting2.setDetails(Rect(197, 71, 245, 89), 300, 21, -1, -1, 1, NULL);
	_lighting.setDetails(Rect(129, 3, 190, 14), 300, 24, -1, -1, 1, NULL);
	_doorway.setDetails(300, 3, -1, 5, 1, (SceneItem *)NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 300, 0, -1, -1, 1, NULL);

	switch (R2_GLOBALS._player._characterIndex) {
	case 1:
		_sceneMode = 300;

		switch (R2_GLOBALS._sceneManager._previousScene) {
		case 250:
			setAction(&_sequenceManager1, this, 300, &R2_GLOBALS._player, &_doorway, NULL);
			break;
		case 325:
			if (!R2_GLOBALS.getFlag(44) || R2_GLOBALS.getFlag(25))
				setAction(&_sequenceManager1, this, 309, &R2_GLOBALS._player, NULL);
			else {
				R2_GLOBALS.setFlag(60);
				R2_GLOBALS._player.setup(302, 3, 1);
				R2_GLOBALS._player.setPosition(Common::Point(271, 150));
				R2_GLOBALS._player.setAction(&_action1);

				if (R2_GLOBALS.getFlag(55)) {
					if (R2_GLOBALS.getFlag(57)) {
						R2_GLOBALS.clearFlag(60);
						R2_GLOBALS._events.setCursor(CURSOR_ARROW);
						_sceneMode = 16;
						_stripManager.start(404, this);
					} else {
						R2_GLOBALS._player.enableControl();
						R2_GLOBALS._player._canWalk = false;
					}
				} else {
					if (R2_GLOBALS.getFlag(45)) {
						R2_GLOBALS.clearFlag(60);
						R2_GLOBALS._events.setCursor(CURSOR_ARROW);
						_sceneMode = 12;
						_stripManager.start3(204, this, R2_GLOBALS._stripManager_lookupList);
					} else {
						R2_GLOBALS._player.enableControl();
						R2_GLOBALS._player._canWalk = false;
					}
				}
			}
			break;
		case 1000:
			R2_GLOBALS.setFlag(60);
			R2_GLOBALS._player.setup(302, 3, 1);
			R2_GLOBALS._player.setPosition(Common::Point(271, 150));
			R2_GLOBALS._events.setCursor(CURSOR_ARROW);

			if (R2_GLOBALS.getFlag(51)) {
				_sceneMode = 13;
				_stripManager.start3(300, this, R2_GLOBALS._stripManager_lookupList);
			} else {
				_sceneMode = 11;
				_stripManager.start3(200, this, R2_GLOBALS._stripManager_lookupList);
			}
			break;

		case 1100:
			R2_GLOBALS._player.setVisage(10);
			R2_GLOBALS._player.setPosition(Common::Point(160, 95));
			_stripId = 400;
			_sceneMode = 309;
			setAction(&_sequenceManager1, this, 309, &R2_GLOBALS._player, NULL);
			break;

		case 1500:
			R2_GLOBALS.clearFlag(60);
			R2_GLOBALS._player.setup(302, 3, 1);
			R2_GLOBALS._player.setPosition(Common::Point(271, 150));
			_sceneMode = 17;
			R2_GLOBALS._events.setCursor(CURSOR_ARROW);
			_stripManager.start(413, this);
			break;

		default:
			if (R2_GLOBALS.getFlag(60)) {
				R2_GLOBALS._player.setup(302, 3, 1);
				R2_GLOBALS._player.setPosition(Common::Point(271, 150));
				R2_GLOBALS._player.setAction(&_action1);
				R2_GLOBALS._player.enableControl(CURSOR_USE);
				R2_GLOBALS._player._canWalk = false;
			} else {
				R2_GLOBALS._player.setStrip(3);
				R2_GLOBALS._player.setPosition(Common::Point(200, 150));
				R2_GLOBALS._player.enableControl();
			}
			break;
		}
		break;

	case 3:
		if (R2_GLOBALS._sceneManager._previousScene == 1500) {
			R2_GLOBALS._player._oldCharacterScene[3] = 3150;
			R2_GLOBALS._player._characterScene[3] = 3150;
			R2_GLOBALS._player._effect = 0;
			R2_GLOBALS._player.setAction(NULL);
			R2_GLOBALS._player.disableControl();

			_quinn.postInit();
			_quinn.setVisage(10);
			_quinn.setPosition(Common::Point(10, 10));
			_quinn.hide();

			_seeker.postInit();
			_seeker.setVisage(20);
			_seeker.setPosition(Common::Point(20, 20));
			_seeker.hide();

			_teal.postInit();
			_soldier.postInit();
			_object12.postInit();

			R2_GLOBALS._sound1.play(107);
			_sceneMode = 308;

			setAction(&_sequenceManager1, this, 308, &R2_GLOBALS._player, &_teal, &_soldier, &_object12, &_doorway, NULL);
		}
		break;
	default:
		break;
	}
}

void Scene300::remove() {
	R2_GLOBALS._player.setAction(NULL);
	SceneExt::remove();
}

void Scene300::signal() {
	switch (_sceneMode) {
	case 10:
		switch (_stripManager._field2E8) {
		case 0:
			R2_GLOBALS._sound1.changeSound(10);
			R2_GLOBALS.setFlag(38);
			break;
		case 1:
			R2_GLOBALS.setFlag(3);
			break;
		case 2:
			R2_GLOBALS.setFlag(4);
			break;
		case 3:
			R2_GLOBALS.setFlag(13);
			if (R2_GLOBALS._stripManager_lookupList[1] == 6)
				R2_GLOBALS.setFlag(40);
			break;
		case 4:
			if (R2_GLOBALS._stripManager_lookupList[1] == 6)
				R2_GLOBALS.setFlag(40);
			break;
		case 5:
			R2_GLOBALS._sceneManager.changeScene(1000);
			break;
		default:
			break;
		}

		_stripManager._field2E8 = 0;
		switch (_stripId) {
		case 400:
			R2_GLOBALS._player.disableControl();
			_sceneMode = 15;
			setAction(&_sequenceManager1, this, 306, &R2_GLOBALS._player, NULL);
			break;
		case 181:
			R2_GLOBALS._player.setStrip(6);
			// Deliberate fall-through
		default:
			R2_GLOBALS._player.enableControl(CURSOR_TALK);

			if ((R2_GLOBALS._player._characterIndex != 1) || R2_GLOBALS.getFlag(44))
				R2_GLOBALS._player._canWalk = false;
			break;
		}
		break;

	case 11:
		R2_GLOBALS.setFlag(44);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;

	case 12:
		R2_GLOBALS._player.setAction(NULL);
		R2_GLOBALS._sceneManager.changeScene(1010);
		break;

	case 13:
		R2_GLOBALS._player.disableControl();
		_seeker.changeZoom(-1);
		_sceneMode = 313;
		_seeker.setAction(NULL);
		setAction(&_sequenceManager1, this, 313, &R2_GLOBALS._player, &_seeker, NULL);
		break;

	case 14:
		if (_seeker._action)
			R2_GLOBALS._player.disableControl();
		_sceneMode = 314;
		break;

	case 15:
		R2_GLOBALS.setFlag(55);
		R2_GLOBALS.setFlag(38);
		R2_GLOBALS.setFlag(44);
		R2_GLOBALS.setFlag(51);
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_sceneMode = 16;
		_stripManager.start3(401, this, R2_GLOBALS._stripManager_lookupList);
		break;

	case 16:
		if (_stripManager._field2E8 == 1) {
			R2_GLOBALS._player.setAction(NULL);
			R2_GLOBALS._sceneManager.changeScene(1000);
		} else {
			R2_GLOBALS._player.setAction(&_action1);
			R2_GLOBALS._player.enableControl(CURSOR_TALK);
		}
		break;

	case 17:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 316;
		_seeker.changeZoom(-1);
		_seeker.setAction(&_sequenceManager3, this, 316, &_seeker, &_doorway, NULL);
		R2_GLOBALS._player.setAction(&_sequenceManager1, NULL, 307, &R2_GLOBALS._player, NULL);
		break;

	case 18:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 317;
		setAction(&_sequenceManager1, this, 317, &_teal, &_doorway, NULL);
		break;

	case 19:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 318;
		setAction(&_sequenceManager1, this, 318, &R2_GLOBALS._player, &_teal, &_soldier, &_object12, NULL);
		break;

	case 20:
		R2_GLOBALS._player._characterIndex = R2_QUINN;
		R2_GLOBALS._sceneManager.changeScene(1500);
		break;

	case 300:
	case 307:
		R2_GLOBALS._player.enableControl();
		break;

	case 301:
		R2_GLOBALS._sceneManager.changeScene(250);
		break;

	case 306:
		R2_GLOBALS._sceneManager.changeScene(325);
		break;

	case 308:
		_sceneMode = 18;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(418, this);
		break;

	case 310:
		R2_GLOBALS._player.setStrip(5);
		// Deliberate fall-through
	case 309:
		signal309();
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_sceneMode = 10;
		_stripManager.start3(_stripId, this, R2_GLOBALS._stripManager_lookupList);
		break;

	case 313:
		_sceneMode = 14;
		R2_GLOBALS._player._effect = 0;
		_seeker.setAction(&_sequenceManager3, this, 314, &_seeker, &_doorway, NULL);
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(301, this);
		break;

	case 314:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 315;
		R2_GLOBALS._player._effect = 1;
		setAction(&_sequenceManager1, this, 315, &R2_GLOBALS._player, &_doorway, NULL);
		break;

	case 315:
		R2_GLOBALS._sceneManager.changeScene(1100);
		break;

	case 316:
		R2_GLOBALS._player._characterScene[2] = 500;
		_seeker.remove();
		R2_GLOBALS._player.enableControl(CURSOR_CROSSHAIRS);
		break;

	case 317:
		_sceneMode = 19;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(419, this);
		break;

	case 318:
		_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(420, this);
		break;

	default:
		break;
	}
}

void Scene300::signal309() {
	if (R2_GLOBALS.getFlag(2))
		R2_GLOBALS._stripManager_lookupList[0] = (R2_INVENTORY.getObjectScene(R2_READER) == 1) ? 3 : 2;

	if (R2_GLOBALS.getFlag(4))
		R2_GLOBALS._stripManager_lookupList[0] = 4;

	if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 1)
		R2_GLOBALS._stripManager_lookupList[0] = 5;

	if (R2_GLOBALS.getFlag(13)) {
		R2_GLOBALS._stripManager_lookupList[0] = 6;
		R2_GLOBALS._stripManager_lookupList[2] = 2;
	}

	if (R2_GLOBALS.getFlag(39))
		R2_GLOBALS._stripManager_lookupList[1] = 2;

	if (R2_GLOBALS.getFlag(5))
		R2_GLOBALS._stripManager_lookupList[1] = 3;

	if (R2_GLOBALS.getFlag(6))
		R2_GLOBALS._stripManager_lookupList[1] = 4;

	if (R2_GLOBALS.getFlag(8))
		R2_GLOBALS._stripManager_lookupList[1] = 5;

	if (R2_GLOBALS.getFlag(9)) {
		R2_GLOBALS._stripManager_lookupList[1] = 6;
		R2_GLOBALS._stripManager_lookupList[3] = 2;
	}

	if (R2_GLOBALS.getFlag(48))
		R2_GLOBALS._stripManager_lookupList[4] = 2;

	if (R2_GLOBALS.getFlag(49))
		R2_GLOBALS._stripManager_lookupList[4] = 3;
}

/*--------------------------------------------------------------------------
 * Scene 325 - Bridge Console
 *
 *--------------------------------------------------------------------------*/

const double ADJUST_FACTOR = 0.06419999999999999;

/*--------------------------------------------------------------------------*/

Scene325::Icon::Icon(): SceneActor()  {
	_lookLineNum = 0;
	_field98 = 0;
	_pressed = false;
}

void Scene325::Icon::postInit(SceneObjectList *OwnerList) {
	SceneObject::postInit();

	_object1.postInit();
	_object1.fixPriority(21);
	_object1.hide();

	_sceneText1._color1 = 92;
	_sceneText1._color2 = 0;
	_sceneText1._width = 200;
	_sceneText2._color1 = 0;
	_sceneText2._color2 = 0;
	_sceneText2._width = 200;
	fixPriority(20);
}

void Scene325::Icon::synchronize(Serializer &s) {
	SceneActor::synchronize(s);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_field98);
	s.syncAsSint16LE(_pressed);
}

void Scene325::Icon::process(Event &event) {
	Scene325 *scene = (Scene325 *)R2_GLOBALS._sceneManager._scene;

	if (!event.handled && !(_flags & OBJFLAG_HIDING) && R2_GLOBALS._player._uiEnabled) {

		if (event.eventType == EVENT_BUTTON_DOWN) {
			int regionIndex = R2_GLOBALS._sceneRegions.indexOf(event.mousePos);

			switch (R2_GLOBALS._events.getCursor()) {
			case CURSOR_LOOK:
				if (regionIndex == _sceneRegionId) {
					event.handled = true;
					SceneItem::display2(326, _lookLineNum);
				}
				break;

			case CURSOR_USE:
				if ((regionIndex == _sceneRegionId) && !_pressed) {
					scene->_sound1.play(14);
					setFrame(2);

					switch (_object1._strip) {
					case 1:
						_object1.setStrip(2);
						break;
					case 3:
						_object1.setStrip(4);
						break;
					case 5:
						_object1.setStrip(6);
						break;
					case 7:
						_object1.setStrip(8);
						break;
					default:
						break;
					}

					_pressed = true;
					event.handled = true;
				}
				break;

			default:
				break;
			}
		}

		if ((event.eventType == EVENT_BUTTON_UP) && _pressed) {
			setFrame(1);

			switch (_object1._strip) {
			case 2:
				_object1.setStrip(1);
				break;
			case 4:
				_object1.setStrip(3);
				break;
			case 6:
				_object1.setStrip(5);
				break;
			default:
				break;
			}

			_pressed = false;
			event.handled = true;
			scene->consoleAction(_lookLineNum);
		}
	}
}

void Scene325::Icon::setIcon(int id) {
	Scene325 *scene = (Scene325 *)R2_GLOBALS._sceneManager._scene;

	_lookLineNum = _field98 = id;
	SceneActor::_lookLineNum = id;

	_sceneText1.remove();
	_sceneText2.remove();

	if (_lookLineNum) {
		showIcon();
		_object1.setup(325, ((id - 1) / 10) * 2 + 1, ((id - 1) % 10) + 1);
		_object1.setPosition(_position);

		_sceneText1._fontNumber = scene->_iconFontNumber;
		_sceneText1.setup(CONSOLE325_MESSAGES[id]);
		_sceneText1.fixPriority(20);

		_sceneText2._fontNumber = scene->_iconFontNumber;
		_sceneText2.setup(CONSOLE325_MESSAGES[id]);
		_sceneText2.fixPriority(20);

		_sceneText2._fontNumber = scene->_iconFontNumber;
		_sceneText2.setup(CONSOLE325_MESSAGES[id]);
		_sceneText2.fixPriority(10);

		switch (_lookLineNum) {
		case 7:
			_sceneText1.setPosition(Common::Point(62, _position.y + 8));
			_sceneText2.setPosition(Common::Point(64, _position.y + 10));
			break;
		case 8:
		case 9:
			_sceneText1.setPosition(Common::Point(65, _position.y + 8));
			_sceneText2.setPosition(Common::Point(67, _position.y + 10));
			break;
		case 12:
			_sceneText1.setPosition(Common::Point(83, _position.y + 8));
			_sceneText2.setPosition(Common::Point(85, _position.y + 10));
			break;
		default:
			_sceneText1.setPosition(Common::Point(121, _position.y + 8));
			_sceneText2.setPosition(Common::Point(123, _position.y + 10));
			break;
		}
	} else {
		hideIcon();
	}
}

void Scene325::Icon::showIcon() {
	_sceneText1.show();
	_sceneText2.show();
	_object1.show();
	_object2.show();
	show();
}

void Scene325::Icon::hideIcon() {
	_sceneText1.hide();
	_sceneText2.hide();
	_object1.hide();
	_object2.hide();
	hide();
}

/*--------------------------------------------------------------------------*/

Scene325::Scene325(): SceneExt() {
	_field412 = 7;
	_iconFontNumber = 50;
	_field416 = _field418 = 0;
	_field41A = _field41C = _field41E = _field420 = 0;
	_soundCount = _soundIndex = 0;

	for (int idx = 0; idx < 10; ++idx)
		_soundQueue[idx] = 0;
}

void Scene325::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(325);

	R2_GLOBALS.clearFlag(50);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_palette.loadPalette(0);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_item2.setDetails(1, 325, 3, 4, 5);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 325, 0, 1, 2, 1, (SceneItem *)NULL);
	_sceneMode = 1;
	signal();
}

void Scene325::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_iconFontNumber);
	s.syncAsSint16LE(_field416);
	s.syncAsSint16LE(_field418);
	s.syncAsSint16LE(_field41A);
	s.syncAsSint16LE(_field41C);
	s.syncAsSint16LE(_field41E);
	s.syncAsSint16LE(_field420);
	s.syncAsSint16LE(_soundCount);
	s.syncAsSint16LE(_soundIndex);

	for (int idx = 0; idx < 10; ++idx)
		s.syncAsSint16LE(_soundQueue[idx]);
}

void Scene325::remove() {
	removeText();
	SceneExt::remove();
}

void Scene325::signal() {
	switch (_sceneMode - 1) {
	case 0:
		_icon1.postInit();
		_icon1._sceneRegionId = 2;
		_icon2.postInit();
		_icon2._sceneRegionId = 3;
		_icon3.postInit();
		_icon3._sceneRegionId = 4;
		_icon4.postInit();
		_icon4._sceneRegionId = 5;

		setAction(&_sequenceManager1, this, 127, &_icon1, &_icon2, &_icon3, &_icon4,
			&R2_GLOBALS._player, NULL);
		_sceneMode = 2;
		break;
	case 1:
		_icon1.setup(160, 1, 1);
		_icon1.setPosition(Common::Point(65, 17));
		_icon1._object2.postInit();
		_icon1._object2.setup(160, 7, 1);
		_icon1._object2.setPosition(Common::Point(106, 41));

		_icon2.setup(160, 1, 1);
		_icon2.setPosition(Common::Point(80, 32));
		_icon2._object2.postInit();
		_icon2._object2.setup(160, 7, 2);
		_icon2._object2.setPosition(Common::Point(106, 56));

		_icon3.setup(160, 1, 1);
		_icon3.setPosition(Common::Point(65, 47));
		_icon3._object2.postInit();
		_icon3._object2.setup(160, 7, 1);
		_icon3._object2.setPosition(Common::Point(106, 71));

		_icon4.setup(160, 1, 1);
		_icon4.setPosition(Common::Point(80, 62));
		_icon4._sceneRegionId = 5;
		_icon4._object2.postInit();
		_icon4._object2.setup(160, 7, 2);
		_icon4._object2.setPosition(Common::Point(106, 86));

		_icon5.postInit();
		_icon5.setup(160, 1, 1);
		_icon5._sceneRegionId = 7;
		_icon5.setPosition(Common::Point(37, 92));
		_icon5.setIcon(8);

		_icon6.postInit();
		_icon6.setup(160, 1, 1);
		_icon6.setPosition(Common::Point(106, 110));
		_icon6.setIcon(7);
		_icon6._sceneRegionId = 8;

		consoleAction(7);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 9:
		switch (_field412) {
		case 3:
			_sceneMode = 129;
			_object1.postInit();
			_object2.postInit();
			_object3.postInit();
			if (R2_GLOBALS.getFlag(13)) {
				_object4.postInit();
				setAction(&_sequenceManager1, this, 130, &R2_GLOBALS._player, &_object1,
					&_object2, &_object3, &_object4, NULL);
			} else {
				setAction(&_sequenceManager1, this, 129, &R2_GLOBALS._player, &_object1,
					&_object2, &_object3, NULL);
			}
			break;
		case 17:
		case 18:
		case 19:
		case 20: {
			int v = 10 - ((21 - _field412) * 2);
			if (R2_GLOBALS.getFlag(50))
				--v;
			if (_field418 == 5)
				v += 8;
			if (R2_GLOBALS.getFlag(51) && (v == 2))
				R2_GLOBALS.setFlag(57);

			if (R2_GLOBALS.getFlag(44) && !R2_GLOBALS.getFlag(51)) {
				if (v != 13) {
					setMessage(328, 0);
				} else {
					_field420 = 864;

					_object12.postInit();
					_object2.setup(326, 4, 1);
					_object12.setPosition(Common::Point(149, 128));
					_object12.fixPriority(20);

					_object13.postInit();
					_object13.setup(326, 4, 2);
					_object13.setPosition(Common::Point(149, (int)(_field420 * ADJUST_FACTOR)));
					_object13.fixPriority(21);

					_object10.postInit();
					_object10.setup(326, 1, 1);
					_object10.setPosition(Common::Point(210, 20));
					_object10.fixPriority(10);

					_object1.postInit();
					_object1.setup(326, 1, 1);
					_object1.setPosition(Common::Point(210, 32));
					_object10.fixPriority(10);

					_object2.postInit();
					_object2.setup(326, 1, 1);
					_object2.setPosition(Common::Point(210, 44));
					_object2.fixPriority(10);

					_object3.postInit();
					_object3.setup(326, 1, 1);
					_object3.setPosition(Common::Point(210, 56));
					_object3.fixPriority(10);

					_object4.postInit();
					_object4.setup(326, 1, 1);
					_object4.setPosition(Common::Point(210, 68));
					_object4.fixPriority(10);

					_object5.postInit();
					_object5.setup(326, 1, 1);
					_object5.setPosition(Common::Point(210, 80));
					_object5.fixPriority(10);

					_object6.postInit();
					_object6.setup(326, 1, 1);
					_object6.setPosition(Common::Point(210, 92));
					_object6.fixPriority(10);

					_object7.postInit();
					_object7.setup(326, 1, 1);
					_object7.setPosition(Common::Point(210, 104));
					_object7.fixPriority(10);

					_object8.postInit();
					_object8.setup(326, 1, 1);
					_object8.setPosition(Common::Point(210, 116));
					_object8.fixPriority(10);

					_object9.postInit();
					_object9.setup(326, 1, 1);
					_object9.setPosition(Common::Point(210, 128));
					_object9.fixPriority(10);

					_object11.postInit();
					_object11.setup(326, 1, 1);
					_object11.setPosition(Common::Point(210, 150));
					_object11.fixPriority(10);
				}
			} else if (R2_GLOBALS.getFlag(51)) {
				setMessage(329, (v == 12) ? 10 : v);
			} else {
				setMessage(327, (v < 15) ? 1 : v);
			}
			break;
		}
		case 21:
			_sceneMode = 129;

			_object1.postInit();
			_object1.setup(327, 1, 1);
			_object1.setPosition(Common::Point(170, 80));
			_object1.fixPriority(10);
			_object1.animate(ANIM_MODE_5, NULL);
			break;
		case 22:
			_sceneMode = 129;

			_object1.postInit();
			_object1.setup(327, 2, 1);
			_object1.setPosition(Common::Point(160, 80));
			_object1.fixPriority(10);
			_object1.animate(ANIM_MODE_5, NULL);
			break;
		case 24:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_field416 = 37;
			setMessage(128, _field416);
			break;
		case 25:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_field416 = 68;
			setMessage(128, _field416);
			break;
		case 26:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_field416 = 105;
			setMessage(128, _field416);
			break;
		default:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_field416 = 105;
			setMessage(128, _field416);
			break;
		}
		break;
	case 10:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;

		if ((_field412 >= 17) && (_field412 <= 20)) {
			_icon5.setIcon(8);
			consoleAction(4);
		} else {
			consoleAction(7);
		}

		_icon6.setIcon(7);
		break;
	case 11:
		R2_GLOBALS.setFlag(45);
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	case 12:
		R2_GLOBALS.setFlag(57);
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	case 14:
		if (_soundCount)
			--_soundCount;

		if (!_soundCount || (R2_GLOBALS._speechSubtitles == 2)) {
			_soundIndex = 0;
			R2_GLOBALS._playStream.stop();
		} else {
			_sceneMode = 15;
			R2_GLOBALS._playStream.play(_soundQueue[_soundIndex], this);
		}
		break;
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene325::removeText() {
	_text1.remove();
	_soundCount = 0;
	_soundIndex = 0;
	R2_GLOBALS._playStream.stop();
}

void Scene325::consoleAction(int id) {
	_icon1.setIcon(0);
	_icon2.setIcon(0);
	_icon3.setIcon(0);
	_icon4.setIcon(0);

	if (id == 7)
		_icon5.setIcon(9);
	else if ((_field412 != 3) && ((_field412 < 17) || (_field412 > 26)))
		_icon5.setIcon(8);

	switch (id - 1) {
	case 0:
		_icon1.setIcon(10);
		_icon2.setIcon(11);
		break;
	case 1:
		_icon1.setIcon(23);
		_icon2.setIcon(24);
		_icon3.setIcon(25);
		_icon4.setIcon(26);
	case 2:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
		R2_GLOBALS._player.disableControl();
		consoleAction(7);
		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();
		// TODO: Finish
		break;
	case 3:
		_icon1.setIcon(5);
		_icon2.setIcon(6);
		_icon3.setIcon(R2_GLOBALS.getFlag(50) ? 16 : 15);
		break;
	case 4:
	case 5:
		_field418 = id;
		_icon1.setIcon(17);
		_icon2.setIcon(18);
		_icon3.setIcon(19);
		break;
	case 7:
		consoleAction(((_field412 == 5) || (_field412 == 6) || (_field412 == 15)) ? 4 : 7);
		break;
	case 8:
		R2_GLOBALS._sceneManager.changeScene(300);
	case 9:
	case 10:
		_iconFontNumber = (id - 1) == 9 ? 50 : 52;
		_text1.remove();
		_icon6.setIcon(7);
		break;
	case 11:
		if (R2_GLOBALS.getFlag(57) && (R2_GLOBALS._player._characterIndex == 1) && !R2_GLOBALS.getFlag(25)) {
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._events.setCursor(CURSOR_ARROW);
			_sceneMode = 13;
			_stripManager.start(403, this);
		} else {
			R2_GLOBALS._player.disableControl();
			_text1.remove();

			_icon4.setPosition(Common::Point(80, 62));
			_icon4._sceneRegionId = 5;
			_icon4.hideIcon();

			_object12.remove();
			_object13.remove();
			_object10.remove();
			_object1.remove();
			_object2.remove();
			_object3.remove();
			_object4.remove();
			_object5.remove();
			_object6.remove();
			_object7.remove();
			_object8.remove();
			_object9.remove();
			_object11.remove();

			_palette.loadPalette(160);
			_sceneMode = 11;

			BF_GLOBALS._scenePalette.addFader(&_palette._palette[0], 256, 5, this);
		}
		break;
	case 12:
		_icon4.setIcon(14);
		_icon4._object2.hide();

		switch (_field412) {
		case 17:
		case 18:
		case 19:
		case 20:
			if (_field420) {
				R2_GLOBALS._player.disableControl();
				_field41A = 1296;
				_field41E = 1;
			}
			break;
		default:
			setMessage(128, --_field416);
			break;
		}
		return;
	case 13:
		_icon4.setIcon(14);
		_icon4._object2.hide();

		switch (_field412) {
		case 17:
		case 18:
		case 19:
		case 20:
			if (_field420 < 1620) {
				R2_GLOBALS._player.disableControl();
				_field41A = 1296;
				_field41E = -1;
			}
			break;
		}
		return;
	case 14:
		if (R2_GLOBALS.getFlag(55)) {
			SceneItem::display2(329, 17);
		} else {
			R2_GLOBALS.setFlag(50);
			consoleAction(4);
		}
		id = 4;
		break;
	case 15:
		R2_GLOBALS.clearFlag(50);
		consoleAction(4);
		id = 4;
		break;
	case 22:
	case 23:
	case 24:
	case 25:
		R2_GLOBALS._player.disableControl();
		consoleAction(2);
		_field412 = id;

		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();
		_icon4.hideIcon();

		_icon5.setIcon(13);
		_icon4.setPosition(Common::Point(52, 107));
		_icon4._sceneRegionId = 9;
		_icon4.setIcon(14);
		_icon4._object2.hide();

		_icon6.setIcon(12);
		_sceneMode = 10;
		_palette.loadPalette(161);

		BF_GLOBALS._scenePalette.addFader(&_palette._palette[0], 256, 5, this);
		break;
	case 6:
	default:
		_icon1.setIcon(1);
		_icon2.setIcon(2);
		_icon3.setIcon(3);
		int idList[3] = { 4, 22, 21 };
		_icon4.setIcon(idList[R2_GLOBALS._player._characterIndex - 1]);
		break;
	}

	if (id != 8)
		_field412 = id;
}

void Scene325::process(Event &event) {
	SceneExt::process(event);

	if (R2_GLOBALS._player._uiEnabled) {
		_icon1.process(event);
		_icon2.process(event);
		_icon3.process(event);
		_icon4.process(event);
		_icon5.process(event);
		_icon6.process(event);
	}
}

void Scene325::dispatch() {
	if (_field41A) {
		switch (_field41A) {
		case 13:
			_field41C = 1;
			break;
		case 1296:
			R2_GLOBALS._sound3.play(87);
			_field41C = 1;
			break;
		case 33:
		case 1283:
			_field41C = 2;
			break;
		case 63:
		case 1263:
			_field41C = 3;
			break;
		case 103:
		case 1233:
			_field41C = 4;
			break;
		case 153:
		case 1193:
			_field41C = 5;
			break;
		case 213:
		case 1143:
			_field41C = 6;
			break;
		case 283:
		case 1083:
			_field41C = 7;
			break;
		case 1013:
			_field41C = 8;
			break;
		default:
			break;
		}

		_field41A -= _field41C;
		int yp = _field41E * _field41C + _object10._position.y;
		bool flag = false;

		if (yp >= 30) {
			yp -= 12;
			--_field420;
			flag = true;
		}
		if (yp <= 10) {
			yp += 12;
			++_field420;
			flag = true;
		}
		_object3.setPosition(Common::Point(149, (int)(_field420 * ADJUST_FACTOR) + 22));

		for (int idx = 0; idx < 4; ++idx)
			_objList[idx].remove();

		if (flag) {
			int v = _field420 - 758;
			_object10.setFrame((v++ <= 0) ? 1 : v);
			_object1.setFrame((v++ <= 0) ? 1 : v);
			_object2.setFrame((v++ <= 0) ? 1 : v);
			_object3.setFrame((v++ <= 0) ? 1 : v);
			_object4.setFrame((v++ <= 0) ? 1 : v);
			_object5.setFrame((v++ <= 0) ? 1 : v);
			_object6.setFrame((v++ <= 0) ? 1 : v);
			_object7.setFrame((v++ <= 0) ? 1 : v);
			_object8.setFrame((v++ <= 0) ? 1 : v);
			_object9.setFrame((v++ <= 0) ? 1 : v);
			_object11.setFrame((v++ <= 0) ? 1 : v);
		}

		_object10.setPosition(Common::Point(210, yp));
		yp += 12;
		_object1.setPosition(Common::Point(210, yp));
		yp += 12;
		_object2.setPosition(Common::Point(210, yp));
		yp += 12;
		_object3.setPosition(Common::Point(210, yp));
		yp += 12;
		_object4.setPosition(Common::Point(210, yp));
		yp += 12;
		_object5.setPosition(Common::Point(210, yp));
		yp += 12;
		_object6.setPosition(Common::Point(210, yp));
		yp += 12;
		_object7.setPosition(Common::Point(210, yp));
		yp += 12;
		_object8.setPosition(Common::Point(210, yp));
		yp += 12;
		_object9.setPosition(Common::Point(210, yp));
		yp += 12;
		_object11.setPosition(Common::Point(210, yp));

		if (!_field41A) {
			R2_GLOBALS._sound3.stop();
			_field41C = 0;

			if (_field420 == 756) {
				R2_GLOBALS._player.disableControl();
				R2_GLOBALS._events.setCursor(CURSOR_USE);
				_sceneMode = 12;
				_stripManager.start(212, this);
			} else {
				R2_GLOBALS._player.enableControl();
				R2_GLOBALS._player._canWalk = false;
			}
		}
	}

	SceneExt::dispatch();
}

void Scene325::setMessage(int resNum, int lineNum) {
	Common::String msg = g_resourceManager->getMessage(resNum, lineNum);

	if (!msg.empty()) {
		Common::String msgText = parseMessage(msg);

		_text1._fontNumber = _iconFontNumber;
		_text1._color1 = 92;
		_text1._color2 = 0;
		_text1._width = 221;
		_text1.fixPriority(20);
		_text1.setup(msgText);
		_text1.setPosition(Common::Point(49, 19));

		R2_GLOBALS._sceneObjects->draw();

		if ((_soundCount != 0) && (R2_GLOBALS._speechSubtitles != 2)) {
			_sceneMode = 15;
			R2_GLOBALS._playStream.play(_soundQueue[_soundIndex++], this);
		}
	} else {
		_field412 = 13;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.hide();

		_icon4.setPosition(Common::Point(80, 62));
		_icon4._sceneRegionId = 5;
		_icon4.hideIcon();

		_palette.loadPalette(160);
		_sceneMode = 11;
		BF_GLOBALS._scenePalette.addFader(&_palette._palette[0], 256, 5, this);
	}
}

/**
 * Parses a message to be displayed on the console to see whether there are any sounds to be played.
 */
Common::String Scene325::parseMessage(const Common::String &msg) {
	_soundIndex = 0;
	_soundCount = 0;

	const char *msgP = msg.c_str();
	while (*msgP == '!') {
		// Get the sound number
		_soundQueue[_soundCount++] = atoi(++msgP);

		while (!((*msgP == '\0') || (*msgP < '0') || (*msgP > '9')))
			++msgP;
	}

	return Common::String(msgP);
}
/*--------------------------------------------------------------------------
 * Scene 400 - Science Lab
 *
 *--------------------------------------------------------------------------*/

bool Scene400::Terminal::startAction(CursorType action, Event &event) {
	Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 402;
		scene->setAction(&scene->_sequenceManager1, scene, 402, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene400::Door::startAction(CursorType action, Event &event) {
	Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 401;
		scene->setAction(&scene->_sequenceManager1, scene, 401, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene400::Reader::startAction(CursorType action, Event &event) {
	Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 405;
		scene->setAction(&scene->_sequenceManager1, scene, 405, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene400::SensorProbe::startAction(CursorType action, Event &event) {
	Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 404;
		scene->setAction(&scene->_sequenceManager1, scene, 404, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene400::AttractorUnit::startAction(CursorType action, Event &event) {
	Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 406;
		scene->setAction(&scene->_sequenceManager1, scene, 406, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene400::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(400);
	_sound1.play(20);

	_door.postInit();
	_door.setVisage(100);
	_door.setPosition(Common::Point(123, 84));
	_door.setDetails(400, 24, -1, 26, 1, (SceneItem *)NULL);

	_consoleDisplay.postInit();
	_consoleDisplay.setup(400, 4, 1);
	_consoleDisplay.setPosition(Common::Point(236, 92));
	_consoleDisplay.fixPriority(120);
	_consoleDisplay.animate(ANIM_MODE_2, NULL);
	_consoleDisplay._numFrames = 5;

	_testerDisplay.postInit();
	_testerDisplay.setup(400, 2, 1);
	_testerDisplay.setPosition(Common::Point(198, 83));
	_testerDisplay.animate(ANIM_MODE_2, NULL);
	_testerDisplay._numFrames = 20;

	if (R2_INVENTORY.getObjectScene(R2_READER) == 400) {
		_reader.postInit();
		_reader.setup(400, 5, 2);
		_reader.setPosition(Common::Point(301, 95));
		_reader.setDetails(400, 54, -1, 56, 1, (SceneItem *)NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_SENSOR_PROBE) == 400) {
		_sensorProbe.postInit();
		_sensorProbe.setup(400, 5, 1);
		_sensorProbe.setPosition(Common::Point(251, 104));
		_sensorProbe.fixPriority(121);
		_sensorProbe.setDetails(400, 57, -1, 59, 1, (SceneItem *)NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_UNIT) == 400) {
		_attractorUnit.postInit();
		_attractorUnit.setup(400, 5, 3);
		_attractorUnit.setPosition(Common::Point(265, 129));
		_attractorUnit.setDetails(400, 60, -1, 62, 1, (SceneItem *)NULL);
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_equipment1.setDetails(11, 400, 3, -1, -1);
	_equipment2.setDetails(24, 400, 3, -1, -1);
	_equipment3.setDetails(25, 400, 3, -1, -1);
	_equipment4.setDetails(26, 400, 3, -1, -1);
	_equipment5.setDetails(28, 400, 3, -1, -1);
	_equipment6.setDetails(29, 400, 3, -1, -1);
	_desk.setDetails(12, 400, 6, -1, -1);
	_desk2.setDetails(27, 400, 6, -1, -1);
	_terminal.setDetails(13, 400, 6, -1, 11);
	_duct.setDetails(14, 400, 12, -1, -1);
	_console.setDetails(15, 400, 15, -1, 17);
	_equalizer.setDetails(Rect(284, 99, 308, 108), 400, 36, -1, 38, 1, NULL);
	_transducer.setDetails(Rect(295, 67, 314, 79), 400, 39, -1, 41, 1, NULL);
	_optimizer.setDetails(Rect(308, 106, 315, 113), 400, 42, -1, 44, 1, NULL);
	_soundModule.setDetails(Rect(291, 118, 315, 131), 400, 45, -1, 47, 1, NULL);
	_tester.setDetails(Rect(179, 62, 217, 92), 400, 30, -1, 32, 1, NULL);
	_helmet.setDetails(Rect(181, 53, 197, 65), 400, 48, -1, 50, 1, NULL);
	_nullifier.setDetails(Rect(201, 56, 212, 65), 400, 51, -1, 50, 1, NULL);
	_shelves.setDetails(16, 400, 18, -1, 20);
	_cabinet.setDetails(17, 400, 21, -1, -1);
	_doorDisplay.setDetails(Rect(161, 43, 166, 52), 400, 27, -1, -1, 1, NULL);
	_lights.setDetails(Rect(113, 3, 168, 14), 400, 33, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 400, 0, 1, -1, 1, NULL);

	_sceneMode = 400;
	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 125:
		setAction(&_sequenceManager1, this, 403, &R2_GLOBALS._player, NULL);
		break;
	case 200:
		setAction(&_sequenceManager1, this, 400, &R2_GLOBALS._player, &_door, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(180, 100));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene400::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene400::signal() {
	switch (_sceneMode) {
	case 400:
	case 403:
		R2_GLOBALS._player.enableControl();
		break;
	case 401:
		R2_GLOBALS._sceneManager.changeScene(200);
		break;
	case 402:
		R2_GLOBALS._sceneManager.changeScene(125);
		break;
	case 404:
		// Getting the sensor probe
		R2_INVENTORY.setObjectScene(R2_SENSOR_PROBE, 1);
		_sensorProbe.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 405:
		// Getting the reader
		R2_INVENTORY.setObjectScene(R2_READER, 1);
		_reader.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 406:
		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_UNIT, 1);
		_attractorUnit.remove();
		R2_GLOBALS._player.enableControl();
		break;
	default:
		break;
	}
}

void Scene400::dispatch() {
	SceneExt::dispatch();

	switch (R2_GLOBALS._player.getRegionIndex() - 15) {
	case 0:
	case 11:
	case 12:
		R2_GLOBALS._player._shade = 2;
		break;
	case 9:
		R2_GLOBALS._player._shade = 0;
		break;
	case 10:
		R2_GLOBALS._player._shade = 1;
		break;
	case 13:
		R2_GLOBALS._player._shade = 3;
		break;
	case 14:
		R2_GLOBALS._player._shade = 4;
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 500 - Lander Bay 2 Storage
 *
 *--------------------------------------------------------------------------*/

bool Scene500::ControlPanel::startAction(CursorType action, Event &event) {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_QUINN)) {
		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS.getFlag(26)) {
			scene->_stripNumber = 1104;
			scene->_sceneMode = 524;
			scene->setAction(&scene->_sequenceManager1, scene, 524, &R2_GLOBALS._player, NULL);
		} else {
			scene->_sceneMode = 510;
			scene->setAction(&scene->_sequenceManager1, scene, 510, &R2_GLOBALS._player, NULL);
		}
		return true;
	} else {
		return SceneHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene500::Object2::startAction(CursorType action, Event &event) {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_TALK) {
		R2_GLOBALS._player.disableControl();
		if (R2_GLOBALS._player._characterIndex == 1) {
			scene->_stripNumber = R2_GLOBALS.getFlag(26) ? 1101 : 1103;
		} else {
			scene->_stripNumber = R2_GLOBALS.getFlag(26) ? 1102 : 1105;
		}

		scene->setAction(&scene->_sequenceManager1, scene, 524, &R2_GLOBALS._player, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::Object3::startAction(CursorType action, Event &event) {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(500, R2_GLOBALS.getFlag(28) ? 28 : _strip + 25);
		return true;

	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			if ((_strip != 3) && (_strip != 7))
				SceneItem::display2(500, _strip);
			else if (R2_GLOBALS.getFlag(26)) {
				R2_GLOBALS._player.disableControl();
				scene->_stripNumber = 1103;
				scene->_sceneMode = 524;
				scene->setAction(&scene->_sequenceManager1, scene, 524, &R2_GLOBALS._player, NULL);
			} else if (!R2_GLOBALS.getFlag(28))
				SceneItem::display2(500, 41);
			else if (!R2_GLOBALS.getFlag(40))
				SceneItem::display2(500, 40);
			else {
				R2_GLOBALS._player.disableControl();
				scene->_sceneMode = 512;
				scene->setAction(&scene->_sequenceManager1, scene, 524, &R2_GLOBALS._player, &scene->_object3, NULL);
				R2_GLOBALS.setFlag(26);
			}
		} else {
			SceneItem::display2(500, 42);
		}
		return true;

	case R2_REBREATHER_TANK:
		if (!R2_GLOBALS.getFlag(25))
			SceneItem::display2(500, 10);
		else if (_strip != 3)
			SceneItem::display2(500, _strip + 25);
		else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 515;
			scene->setAction(&scene->_sequenceManager1, scene, 515, &R2_GLOBALS._player, &scene->_object3, NULL);
			R2_GLOBALS.setFlag(28);
		}
		return true;

	case R2_RESERVE_REBREATHER_TANK:
		SceneItem::display2(500, 53);
		return true;

	default:
		if (action < R2_LAST_INVENT) {
			SceneItem::display2(500, action);
			return true;
		} else {
			return SceneActor::startAction(action, event);
		}
	}
}

bool Scene500::Doorway::startAction(CursorType action, Event &event) {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_QUINN)) {
		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS.getFlag(26)) {
			scene->_stripNumber = 1104;
			scene->_sceneMode = 524;
			scene->setAction(&scene->_sequenceManager1, scene, 524, &R2_GLOBALS._player, NULL);
		} else {
			scene->_sceneMode = 500;
			scene->setAction(&scene->_sequenceManager1, scene, 500, &R2_GLOBALS._player, NULL);
		}

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::OxygenTanks::startAction(CursorType action, Event &event) {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(500, R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) ? 50 : 49);
		return true;

	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex != R2_QUINN) {
			SceneItem::display2(500, 52);
			return true;
		} else if ((R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) != 1) &&
				(R2_GLOBALS._player._characterIndex != R2_SEEKER) && !R2_GLOBALS.getFlag(28)) {
			R2_GLOBALS._player.disableControl();

			if (_position.y == 120) {
				scene->_sceneMode = 513;
				scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player,
					&scene->_tanks1, NULL);
			} else {
				scene->_sceneMode = 514;
				scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player,
					&scene->_tanks2, NULL);
			}
			return true;
		}
		break;

	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

bool Scene500::AirLock::startAction(CursorType action, Event &event) {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && R2_GLOBALS.getFlag(26)) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = (R2_GLOBALS._player._characterIndex == R2_QUINN) ? 521 : 522;
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player,
			&scene->_object2, &scene->_airLock, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::Aerosol::startAction(CursorType action, Event &event) {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 503;
		scene->setAction(&scene->_sequenceManager1, scene, 503, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::SonicStunner::startAction(CursorType action, Event &event) {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_QUINN)) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = R2_GLOBALS.getFlag(26) ? 520 : 502;
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::Locker1::startAction(CursorType action, Event &event) {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_QUINN)) {
		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS.getFlag(11))
			scene->_sceneMode = R2_GLOBALS.getFlag(26) ? 517 : 505;
		else
			scene->_sceneMode = R2_GLOBALS.getFlag(26) ? 516 : 504;

		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::Locker2::startAction(CursorType action, Event &event) {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_QUINN)) {
		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS.getFlag(12))
			scene->_sceneMode = R2_GLOBALS.getFlag(26) ? 519 : 507;
		else
			scene->_sceneMode = R2_GLOBALS.getFlag(26) ? 518 : 506;

		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::Object::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		return false;
	} else {
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene500::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(500);

	Common::fill(&_buffer[0], &_buffer[2710], 0);
	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(50);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_quinnSpeaker);

	if (R2_GLOBALS.getFlag(25)) {
		R2_GLOBALS._player._characterScene[R2_SEEKER] = 500;

		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_GLOBALS._walkRegions.disableRegion(1);

			_object2.postInit();
			_object2._effect = 1;
			_object2.setup(1505, 1, 1);
			_object2._moveDiff.x = 5;
			_object2.setPosition(Common::Point(42, 151));
			_object2.setDetails(500, 34, 35, 36, 1, (SceneItem *)NULL);
		} else if (R2_GLOBALS._player._characterScene[R2_QUINN] == 500) {
			_object2.postInit();
			_object2._effect = 1;
			_object2.setup(R2_GLOBALS.getFlag(26) ? 1500 : 10, 1, 1);
			_object2.setPosition(Common::Point(42, 151));

			R2_GLOBALS._walkRegions.disableRegion(1);
			R2_GLOBALS._walkRegions.disableRegion(2);
			R2_GLOBALS._walkRegions.disableRegion(3);

			_object2.setDetails(500, 37, 38, -1, 1, (SceneItem *)NULL);
		}
	}

	if ((R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) != 500) && R2_GLOBALS.getFlag(27)) {
		_tanks1.postInit();
		_tanks1.setup(502, 7, 1);
		_tanks1.setPosition(Common::Point(281, 120));
		_tanks1.setDetails(500, -1, -1, -1, 1, (SceneItem *)NULL);
	} else {
		if (R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) == 500) {
			_tanks1.postInit();
			_tanks1.setup(502, 7, 1);
			_tanks1.setPosition(Common::Point(281, 120));
			_tanks1.setDetails(500, -1, -1, -1, 1, (SceneItem *)NULL);
		}

		_tanks2.postInit();
		_tanks2.setup(502, 7, 1);
		_tanks2.setPosition(Common::Point(286, 121));
		_tanks2.setDetails(500, -1, -1, -1, 1, (SceneItem *)NULL);
	}

	_doorway.postInit();
	_doorway.setup(501, 1, 1);
	_doorway.setPosition(Common::Point(132, 85));
	_doorway.setDetails(500, 15, -1, 17, 1, (SceneItem *)NULL);

	_airLock.postInit();
	_airLock.setup(501, 2, 1);
	_airLock.setPosition(Common::Point(41, 121));

	if (!R2_GLOBALS.getFlag(25))
		_airLock.setDetails(500, 6, -1, 10, 1, (SceneItem *)NULL);
	else if ((R2_GLOBALS._player._characterScene[R2_QUINN] != 500) ||
			(R2_GLOBALS._player._characterScene[R2_SEEKER] != 500))
		_airLock.setDetails(500, 6, -1, 40, 1, (SceneItem *)NULL);
	else
		_airLock.setDetails(500, 6, -1, 9, 1, (SceneItem *)NULL);

	_locker1.postInit();
	_locker1.setup(500, 3, R2_GLOBALS.getFlag(11) ? 6 : 1);
	_locker1.setPosition(Common::Point(220, 82));
	_locker1.setDetails(500, 27, -1, -1, 1, (SceneItem *)NULL);

	_locker2.postInit();
	_locker2.setup(500, 4, R2_GLOBALS.getFlag(12) ? 6 : 1);
	_locker2.setPosition(Common::Point(291, 98));
	_locker2.fixPriority(121);
	_locker2.setDetails(500, 27, -1, -1, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_AEROSOL) == 500) {
		_aerosol.postInit();
		_aerosol.setup(500, 5, 2);
		_aerosol.setPosition(Common::Point(286, 91));
		_aerosol.fixPriority(120);
		_aerosol.setDetails(500, 24, 25, 26, 1, (SceneItem *)NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_SONIC_STUNNER) == 500) {
		_sonicStunner.postInit();
		_sonicStunner.setup(500, 5, 1);
		_sonicStunner.setPosition(Common::Point(214, 76));
		_sonicStunner.setDetails(500, 21, 22, 23, 1, (SceneItem *)NULL);
	}

	_object1.postInit();
	_object1._effect = 1;
	_object1.setup(502, 1, 1);
	_object1.setPosition(Common::Point(258, 99));
	_object1.fixPriority(50);

	_object8.postInit();
	_object8.setPosition(Common::Point(250, 111));

	if (!R2_GLOBALS.getFlag(35)) {
		_object8.setup(501, 3, 1);
	} else {
		_object8.setup(500, 8, 7);

		_object3.postInit();
		_object3._effect = 1;
		_object3.setPosition(Common::Point(247, 52));
		_object3.setDetails(500, -1, -1, -1, 2, (SceneItem *)NULL);

		if (!R2_GLOBALS.getFlag(26)) {
			if (R2_GLOBALS.getFlag(28))
				_object3.setup(502, 7, 2);
			else
				_object3.setup(502, R2_GLOBALS._v566A3 + 2, 7);
		}
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
		R2_GLOBALS._player._moveDiff.x = 5;

	_controlPanel.setDetails(Rect(175, 62, 191, 80), 500, 31, 32, 33, 1, (SceneItem *)NULL);
	_item2.setDetails(Rect(13, 58, 70, 118), 500, 12, -1, -1, 1, (SceneItem *)NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 500, 0, -1, -1, 1, (SceneItem *)NULL);

	if ((R2_GLOBALS._player._characterIndex == R2_QUINN) && (R2_GLOBALS._sceneManager._previousScene == 700)) {
		R2_GLOBALS._player.disableControl();
		_sceneMode = 501;
		setAction(&_sequenceManager1, this, 501, &R2_GLOBALS._player, &_doorway, NULL);
	} else {
		if (R2_GLOBALS._player._characterIndex != R2_QUINN) {
			R2_GLOBALS._player.setup(1505, 6, 1);
		} else {
			R2_GLOBALS._player.setup(R2_GLOBALS.getFlag(26) ? 1500 : 10, 6, 1);
		}

		R2_GLOBALS._player.setPosition(Common::Point(123, 135));
		R2_GLOBALS._player.enableControl();
	}
}

void Scene500::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_stripNumber);
}

void Scene500::signal() {
	switch (_sceneMode) {
	case 3:
		R2_GLOBALS._player.enableControl();
		break;
	case 5:
		_sceneMode = 12;
		_sound1.play(127);
		_object1.animate(ANIM_MODE_6, this);

		R2_GLOBALS.clearFlag(35);
		_object3.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 6:
		_sceneMode = 11;
		_sound1.play(127);
		_object1.animate(ANIM_MODE_5, this);

		R2_GLOBALS.clearFlag(35);
		_object3.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 7:
		_sound1.play(126);
		_object8.animate(ANIM_MODE_6, this);

		R2_GLOBALS.clearFlag(35);
		_object3.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 500:
		R2_GLOBALS._sceneManager.changeScene(700);
		break;
	case 501:
		if (R2_GLOBALS._player._characterScene[R2_QUINN] == 500) {
			_stripNumber = 1100;
			_sceneMode = 523;
			setAction(&_sequenceManager1, this, 523, &R2_GLOBALS._player, NULL);
		} else {
			R2_GLOBALS._player.enableControl();
		}
		break;
	case 502:
	case 520:
		R2_INVENTORY.setObjectScene(R2_SONIC_STUNNER, 1);
		_sonicStunner.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 503:
		R2_INVENTORY.setObjectScene(R2_AEROSOL, 1);
		_aerosol.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 504:
	case 516:
		R2_GLOBALS.setFlag(11);
		R2_GLOBALS._player.enableControl();
		break;
	case 505:
	case 517:
		R2_GLOBALS.clearFlag(11);
		R2_GLOBALS._player.enableControl();
		break;
	case 506:
	case 518:
		R2_GLOBALS.setFlag(11);
		R2_GLOBALS._player.enableControl();
		break;
	case 507:
	case 519:
		R2_GLOBALS.clearFlag(12);
		R2_GLOBALS._player.enableControl();
		break;
	case 509:
		R2_GLOBALS.clearFlag(35);
		_object3.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 510:
		R2_GLOBALS._player.enableControl();
		_area1.setDetails(500, 6, 1, Common::Point(160, 120));
		R2_GLOBALS._player.enableControl();
		break;
	case 513:
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 1);
		_tanks1.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 514:
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 1);
		R2_GLOBALS.setFlag(27);
		_tanks2.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 515:
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 0);
		R2_GLOBALS.setFlag(28);
		R2_GLOBALS._player.enableControl();
		break;
	case 521:
	case 522:
		R2_GLOBALS._sceneManager.changeScene(525);
		break;
	case 523:
	case 524:
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_sceneMode = 8;
		_stripManager.start(_stripNumber, this);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}


/*--------------------------------------------------------------------------
 * Scene 525 - Cutscene - Walking in hall
 *
 *--------------------------------------------------------------------------*/
void Scene525::postInit(SceneObjectList *OwnerList) {
	loadScene(525);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	R2_GLOBALS._sound1.play(105);

	_actor1.postInit();
	_actor1._effect = 1;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	setAction(&_sequenceManager, this, 525, &R2_GLOBALS._player, &_actor1, NULL);
}

void Scene525::signal() {
	R2_GLOBALS._sceneManager.changeScene(1525);
}

/*--------------------------------------------------------------------------
 * Scene 600 -
 *
 *--------------------------------------------------------------------------*/
Scene600::Scene600() {
	_field412 = 0;
	for (int i = 0; i < 256; i++)
		_fieldAD2[i] = 0;
}

void Scene600::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	for (int i = 0; i < 256; i++)
		s.syncAsByte(_fieldAD2[i]);
}

bool Scene600::Item1::startAction(CursorType action, Event &event) {
	if ((action != R2_NEGATOR_GUN) || (!R2_GLOBALS.getFlag(5)) || (R2_GLOBALS.getFlag(8)))
		return SceneHotspot::startAction(action, event);

	SceneItem::display(600, 32, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
	return true;
}

bool Scene600::Item4::startAction(CursorType action, Event &event) {
	if ((action != R2_NEGATOR_GUN) || (!R2_GLOBALS.getFlag(1)))
		return SceneHotspot::startAction(action, event);

	if ((R2_GLOBALS.getFlag(5)) && (!R2_GLOBALS.getFlag(8))) {
		SceneItem::display(600, 32, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		return true;
	}

	if (R2_GLOBALS.getFlag(5)) {
		SceneItem::display(600, 30, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		return true;
	}

	if ((!R2_GLOBALS.getFlag(8)) || (R2_GLOBALS.getFlag(9)))
		return SceneHotspot::startAction(action, event);

	R2_GLOBALS._player.disableControl();

	Scene600 *scene = (Scene600 *)R2_GLOBALS._sceneManager._scene;

	scene->_object1.setup2(603, 3, 1, 239, 54, 10, 0);
	scene->_actor3.postInit();
	scene->_actor2.postInit();

	scene->_sceneMode = 612;
	setAction(&scene->_sequenceManager1, this, 612, &scene->_actor3, &scene->_actor2, &R2_GLOBALS._player, NULL);
	return true;
}

void Scene600::Actor4::signal() {
	Common::Point pt(36, 177 + R2_GLOBALS._randomSource.getRandomNumber(5));
	NpcMover *mover = new NpcMover();
	addMover(mover, &pt, this);
}

bool Scene600::Actor4::startAction(CursorType action, Event &event) {
	if ((action >= CURSOR_WALK) && (action < R2CURSORS_START))
	// Only action cursors
		return SceneActor::startAction(action, event);

	return false;
}

void Scene600::Actor4::draw() {
	warning("TODO: Actor4::draw()");
	SceneActor::draw();
}

bool Scene600::Actor5::startAction(CursorType action, Event &event) {
	if ((action < CURSOR_WALK) && (action >= R2CURSORS_START))
		return false;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene600 *scene = (Scene600 *)R2_GLOBALS._sceneManager._scene;

	if ((R2_INVENTORY.getObjectScene(R2_CLAMP) == 600) && (!R2_GLOBALS.getFlag(6))) {
		R2_GLOBALS._player.disableControl();
		scene->_actor6.setDetails(600, 11, -1, -1, 3, (SceneItem *) NULL);
		R2_GLOBALS.setFlag(6);
		scene->_sceneMode = 609;
		scene->setAction(&scene->_sequenceManager1, scene, 609, &R2_GLOBALS._player, &scene->_actor5, &scene->_actor6, &scene->_actor1, NULL);
		return true;
	}

	if (_frame != 1)
		return false;

	if (!R2_GLOBALS.getFlag(6)) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 616;
		scene->setAction(&scene->_sequenceManager1, scene, 616, &R2_GLOBALS._player, &scene->_actor5, &scene->_actor6, NULL);
		return true;
	}

	if ((R2_GLOBALS.getFlag(9)) && (R2_INVENTORY.getObjectScene(R2_COM_SCANNER) == 600))
		SceneItem::display(600, 31, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 601;
		scene->setAction(&scene->_sequenceManager1, scene, 601, &R2_GLOBALS._player, &scene->_actor5, NULL);
	}
	return true;
}

bool Scene600::Actor6::startAction(CursorType action, Event &event) {
	Scene600 *scene = (Scene600 *)R2_GLOBALS._sceneManager._scene;

	if ((action < CURSOR_WALK) && (action >= R2CURSORS_START)) {
		switch (action) {
		case R2_COM_SCANNER:
			if (R2_GLOBALS.getFlag(6)) {
				if (R2_GLOBALS.getFlag(8)) {
					SceneItem::display(600, 29, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
					return true;
				} else {
					R2_GLOBALS._player.disableControl();
					scene->_actor8.postInit();
					scene->_actor8.setDetails(600, 20, -1, -1, 4, &scene->_actor6);
					scene->_sceneMode = 607;
					scene->setAction(&scene->_sequenceManager1, scene, 607, &R2_GLOBALS._player, &scene->_actor8, NULL);
					return true;
				}
			} else {
				return SceneActor::startAction(action, event);
			}
			break;
		case R2_AEROSOL:
			if (R2_GLOBALS.getFlag(5)) {
				SceneItem::display(600, 28, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
				return true;
			} else {
				R2_GLOBALS._player.disableControl();
				scene->_actor7.postInit();
				scene->_actor7.setDetails(600, 27, -1, -1, 5, &scene->_actor6);

				scene->_actor4.postInit();
				scene->_actor4.setup(601, 3, 1);
				scene->_actor4._effect = 3;
				scene->_actor4._moveDiff = Common::Point(1, 1);
				scene->_actor4._moveRate = 2;
				scene->_actor4._numFrames = 3;
				scene->_actor4.setDetails(600, 24, 25, 26, 5, &scene->_actor7);

				scene->_sceneMode = 605;

				scene->setAction(&scene->_sequenceManager1, scene, 605, &R2_GLOBALS._player, &scene->_actor7, &scene->_actor4, &scene->_actor5, NULL);
				return true;
			}
			break;
		case R2_CLAMP:
			if (R2_GLOBALS.getFlag(5)) {
				R2_GLOBALS._player.disableControl();
				scene->_sceneMode = 606;
				scene->setAction(&scene->_sequenceManager1, scene, 606, &R2_GLOBALS._player, &scene->_actor6, NULL);
				return true;
			} else {
				return SceneActor::startAction(action, event);
			}
			break;
		default:
			return false;
			break;
		}
	} else if (action != CURSOR_USE) {
		if (R2_GLOBALS.getFlag(5)) {
			return SceneActor::startAction(action, event);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 610;
			scene->setAction(&scene->_sequenceManager1, scene, 610, &scene->_actor1, &R2_GLOBALS._player, NULL);
			return true;
		}
	} else
		return SceneActor::startAction(action, event);
}

bool Scene600::Actor7::startAction(CursorType action, Event &event) {
	Scene600 *scene = (Scene600 *)R2_GLOBALS._sceneManager._scene;

	if ((action < CURSOR_WALK) && (action >= R2CURSORS_START)) {
		return false;
	} else if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 614;
		scene->setAction(&scene->_sequenceManager1, scene, 614, &R2_GLOBALS._player, &scene->_actor7, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene600::Actor8::startAction(CursorType action, Event &event) {
	Scene600 *scene = (Scene600 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (R2_INVENTORY.getObjectScene(9) == 600)) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 615;
		scene->setAction(&scene->_sequenceManager1, scene, 615, &R2_GLOBALS._player, &scene->_actor8, NULL);
	} else if ((action == R2_SONIC_STUNNER) && (R2_INVENTORY.getObjectScene(9) == 600) && (R2_GLOBALS._v565F1[1] == 2) && (!R2_GLOBALS.getFlag(8))){
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 608;
		scene->setAction(&scene->_sequenceManager1, scene, 608, &R2_GLOBALS._player, &scene->_actor4, NULL);
	} else {
		return SceneActor::startAction(action, event);
	}

	return true;
}

void Scene600::postInit(SceneObjectList *OwnerList) {
	loadScene(600);
	SceneExt::postInit();
	R2_GLOBALS.setFlag(39);
	R2_GLOBALS._walkRegions.enableRegion(3);
	_field412 = 0;

	warning("FIXME: loop to initialize _fieldAD2[]");

	_actor5.postInit();
	_actor5.setVisage(600);
	_actor5.setPosition(Common::Point(29, 147));
	_actor5.fixPriority(10);
	_actor5.setDetails(300, 3, -1, -1, 1, (SceneItem *) NULL);

	_actor6.postInit();
	_actor6.setPosition(Common::Point(246, 41));

	if (R2_INVENTORY.getObjectScene(9) == 600) {
		_actor8.postInit();
		_actor8.setup(602, 5, 1);
		_actor8.setPosition(Common::Point(246, 41));
		_actor8.setDetails(600, 20, -1, -1, 1, (SceneItem *) NULL);
		switch (R2_GLOBALS._v565F1[1] - 2) {
		case 0:
			R2_GLOBALS._sound4.play(45);
			break;
		case 1:
			R2_GLOBALS._sound4.play(4);
			break;
		case 2:
			R2_GLOBALS._sound4.play(5);
			break;
		case 3:
			R2_GLOBALS._sound4.play(6);
			break;
		default:
			break;
		}
	}

	if (R2_GLOBALS.getFlag(6)) {
		_actor6.setup(602, 7, 1);
		_actor6.setDetails(600, 11, -1, -1, 1, (SceneItem *) NULL);
	} else {
		_actor6.setup(600, 2, 1);
		_actor6.setDetails(600, 10, -1, -1, 1, (SceneItem *) NULL);

		_actor1.postInit();
		_actor1.setup(600, 3, 5);
		_actor1.setPosition(Common::Point(223, 51));
		_actor1.fixPriority(200);
	}

	if (! R2_GLOBALS.getFlag(9))
		_object1.setup2(603, 1, 1, 244, 50, 10, 0);

	if (R2_GLOBALS.getFlag(5)) {
		if (R2_INVENTORY.getObjectScene(12) == 600) {
			_actor7.postInit();
			_actor7.setup(602, 2, 2);
			_actor7.setPosition(Common::Point(189, 95));
			_actor7.setDetails(600, 27, -1, -1, 1, (SceneItem *) NULL);
		}

		if (R2_GLOBALS.getFlag(8)) {
			if (R2_GLOBALS.getFlag(9)) {
				_actor2.postInit();
				_actor2.setup(603, 2, 1);
				_actor2.setPosition(Common::Point(233, 45));
				_actor2.animate(ANIM_MODE_2, NULL);_actor2.fixPriority(11);
			}
		} else {
			_actor4.postInit();
			_actor4.setup(601, 1, 1);
			_actor4.setPosition(Common::Point(180, 110));
			_actor4._moveDiff = Common::Point(1, 1);
			_actor4._moveRate = 2;
			_actor4._numFrames = 3;
			_actor4.animate(ANIM_MODE_2, NULL);
			_actor4.fixPriority(130);
			_actor4._effect = 3;
			_actor4.setDetails(600, 24, 25, 26, 1, (SceneItem *) NULL);
			_actor4.signal();
		}
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_item2.setDetails(12, 600, 17, -1, 19);
	_item3.setDetails(11, 600, 14, -1, -1);

	if (R2_GLOBALS.getFlag(9)) {
		_item1.setDetails(Rect(159, 3, 315, 95), 600, 7, -1, -1, 1, NULL);
	} else {
		_item4.setDetails(Rect(173, 15, 315, 45), 600, 21, -1, 23, 1, NULL);
		_item1.setDetails(Rect(159, 3, 315, 95), 600, 6, -1, -1, 1, NULL);
	}
	_item5.setDetails(Rect(0, 0, 320, 200), 600, 0, -1, -1, 1, NULL);

	_sceneMode = 600;
	if (R2_GLOBALS._sceneManager._previousScene == 700) {
		if (R2_GLOBALS.getFlag(6)) {
			setAction(&_sequenceManager1, this, 600, &R2_GLOBALS._player, &_actor5, NULL);
		} else if (R2_GLOBALS.getFlag(5)) {
			setAction(&_sequenceManager1, this, 603, &R2_GLOBALS._player, &_actor5, &_actor6, &_actor1, NULL);
		} else {
			setAction(&_sequenceManager1, this, 602, &R2_GLOBALS._player, &_actor5, &_actor6, &_actor1, NULL);
		}
	} else if (R2_GLOBALS.getFlag(5)) {
		R2_GLOBALS._player.setPosition(Common::Point(50, 140));
		R2_GLOBALS._player.setStrip(3);
		_actor6.setFrame(_actor6.getFrameCount());
		signal();
	} else {
		_actor5.setFrame(7);
		_actor6.setFrame(7);
		R2_GLOBALS._player.setPosition(Common::Point(28, 140));
		R2_GLOBALS._player.setStrip(5);
		signal();
	}
}

void Scene600::remove() {
	if (R2_INVENTORY.getObjectScene(9) == 600)
		R2_GLOBALS._sound4.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene600::signal() {
	switch (_sceneMode) {
	case 601:
	// No break on purpose
	case 613:
	// No break on purpose
	case 616:
		R2_GLOBALS._sceneManager.changeScene(700);
		break;
	case 605:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._walkRegions.enableRegion(6);
		R2_GLOBALS._walkRegions.enableRegion(7);
		R2_GLOBALS._walkRegions.enableRegion(9);
		R2_GLOBALS._walkRegions.enableRegion(10);

		R2_INVENTORY.setObjectScene(12, 600);
		R2_GLOBALS.setFlag(5);

		_actor4._effect = 3;
		_actor4.signal();
		break;
	case 606:
		R2_INVENTORY.setObjectScene(15, 600);
		R2_GLOBALS._player.enableControl();
		break;
	case 607:
		R2_INVENTORY.setObjectScene(9, 600);
		R2_GLOBALS._player.enableControl();
		break;
	case 608:
		R2_GLOBALS.setFlag(8);
		_actor4.remove();
		R2_GLOBALS._walkRegions.disableRegion(6);
		R2_GLOBALS._walkRegions.disableRegion(9);
		R2_GLOBALS._walkRegions.disableRegion(10);
		R2_GLOBALS._player.enableControl();
		break;
	case 612:
		R2_GLOBALS.setFlag(9);
		_actor3.remove();
		R2_GLOBALS._sceneItems.remove(&_item4);
		_actor2.setDetails(600, 21, -1, 23, 4, &_item4);
		_item1.setDetails(600, 7, -1, -1, 3, (SceneItem *) NULL);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;
	case 614:
		R2_GLOBALS._player.enableControl();
		_actor7.remove();
		R2_INVENTORY.setObjectScene(12, 1);
		R2_GLOBALS._walkRegions.disableRegion(7);
		break;
	case 615:
		_actor8.remove();
		R2_INVENTORY.setObjectScene(9, 1);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		_field412 = 0;
		_sceneMode = 0;
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene600::process(Event &event) {
	if ((!R2_GLOBALS._player._canWalk) && (!R2_GLOBALS.getFlag(6)) && (event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._events.getCursor() == R2_NEGATOR_GUN)) {
		if (!_actor5.contains(event.mousePos) || (_actor5._frame <= 1)) {
			if (R2_GLOBALS.getFlag(5)) {
				_field412 += 10;
			} else {
				R2_GLOBALS._player.disableControl();
				_sceneMode = 604;
				setAction(&_sequenceManager1, this, 604, &_actor1, &R2_GLOBALS._player, NULL);
				event.handled = true;
			}
		} else {
			R2_GLOBALS._player.disableControl();
			_sceneMode = 613;
			setAction(&_sequenceManager1, this, 613, &R2_GLOBALS._player, &_actor6, NULL);
			event.handled = true;
		}
	} else if ((!R2_GLOBALS.getFlag(6)) && (R2_GLOBALS._player._mover) && (_field412 < 10)){
		_field412 += 10;
	}

	Scene::process(event);
}

void Scene600::dispatch() {
	if ((_field412 != 0) && (_sceneMode != 600) && (_sceneMode != 603) && (_sceneMode != 602)) {
		if ( ((_actor6._strip == 4) && (_actor6._frame > 1))
		  ||  (_sceneMode == 601)
		  || ((_sceneMode == 616) && (_actor5._frame > 1)) )
		  _field412 = 0;
		else {
			_field412--;
			if (_field412 % 10 == 0) {
				_actor1.setAction(&_sequenceManager2, NULL, 611, &_actor1, NULL);
			}
			if ((_field412 == 0) && (R2_GLOBALS._player._mover))
				_field412 = 10;
		}
	}

	if (_actor1._frame == 2)
		_aSound1.play(40);

	Scene::dispatch();
	if ((_actor4._strip == 3) && (_actor4._frame == 3)) {
		_actor1.setStrip(4);
		_actor1.setFrame(1);
	}
}

/*--------------------------------------------------------------------------
 * Scene 700 -
 *
 *--------------------------------------------------------------------------*/
Scene700::Scene700() {
	_rotation = NULL;
}

void Scene700::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	SYNC_POINTER(_rotation);
}

bool Scene700::Item11::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_GLOBALS._player._position.x < 100))
		return false;

	return NamedHotspot::startAction(action, event);
}

bool Scene700::Item12::startAction(CursorType action, Event &event) {
	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_CABLE_HARNESS:
		R2_GLOBALS._player.disableControl();
		scene->_actor5.postInit();
		scene->_actor5.setup(701, 3, 2);
		scene->_actor5.setPosition(Common::Point(243, 98));
		scene->_actor5.setDetails(700, 37, -1, -1, 2, (SceneItem *) NULL);
		scene->_actor5.hide();
		scene->_sceneMode = 20;
		break;
	case R2_ATTRACTOR_CABLE_HARNESS:
		R2_GLOBALS._player.disableControl();
		scene->_actor5.postInit();
		scene->_actor5.setup(701, 2, 8);
		scene->_actor5.setPosition(Common::Point(243, 98));
		scene->_actor5.setDetails(700, 38, -1, -1, 2, (SceneItem *) NULL);
		scene->_actor5.hide();
		scene->_sceneMode = 21;
		break;
	default:
		return NamedHotspot::startAction(action, event);
		break;
	}

	scene->setAction(&scene->_sequenceManager, this, 707, &R2_GLOBALS._player, &scene->_actor5, NULL);
	return true;
}

bool Scene700::Actor2::startAction(CursorType action, Event &event) {
	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._position.y <= 100)
		return false;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 701;
	scene->setAction(&scene->_sequenceManager, scene, 701, &R2_GLOBALS._player, this, NULL);

	return true;
}

bool Scene700::Actor3::startAction(CursorType action, Event &event) {
	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._position.y <= 100)
		return false;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 702;
	scene->setAction(&scene->_sequenceManager, scene, 702, &R2_GLOBALS._player, this, NULL);

	return true;
}

bool Scene700::Actor4::startAction(CursorType action, Event &event) {
	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._position.y <= 100)
		return false;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 704;
	scene->setAction(&scene->_sequenceManager, scene, 704, &R2_GLOBALS._player, this, NULL);

	return true;
}

bool Scene700::Actor5::startAction(CursorType action, Event &event) {
	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		switch (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS)) {
		case 0:
			if ((_strip == 2) && (_frame == 1)) {
				R2_GLOBALS._player.disableControl();
				if (R2_GLOBALS._player._position.x <= 100) {
					scene->_sceneMode = 710;
					scene->setAction(&scene->_sequenceManager, scene, 710, &R2_GLOBALS._player, this, NULL);
				} else {
					scene->_sceneMode = 709;
					scene->setAction(&scene->_sequenceManager, scene, 709, &R2_GLOBALS._player, this, NULL);
				}
			} else {
				return SceneActor::startAction(action, event);
			}
			break;
		case 700: {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 10;
			Common::Point pt(_position.x - 12, _position.y + 1);
			PlayerMover *mover = new PlayerMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			}
			break;
		default:
			break;
		}
		break;
	case R2_ATTRACTOR_UNIT:
		R2_GLOBALS._player.disableControl();
		if (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 700) {
			scene->_sceneMode = 706;
			scene->setAction(&scene->_sequenceManager, scene, 706, &R2_GLOBALS._player, &scene->_actor5, NULL);
		} else {
			scene->_sceneMode = 15;
			Common::Point pt(_position.x - 12, _position.y + 1);
			PlayerMover *mover = new PlayerMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
		}
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}

	return true;
}

bool Scene700::Actor6::startAction(CursorType action, Event &event) {
	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	if ((action != CURSOR_USE) || (R2_GLOBALS._player._position.y >= 100))
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 1;
	Common::Point pt(_position.x, 69);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);

	return true;
}

void Scene700::postInit(SceneObjectList *OwnerList) {
	if (R2_GLOBALS._sceneManager._previousScene == 900)
		g_globals->gfxManager()._bounds.moveTo(Common::Point(160, 0));

	loadScene(700);
	R2_GLOBALS._v558B6.set(60, 0, 260, 200);
	SceneExt::postInit();

	_rotation = R2_GLOBALS._scenePalette.addRotation(237, 246, -1);
	_rotation->setDelay(5);
	_rotation->_countdown = 1;

	_actor2.postInit();
	_actor2.setVisage(700);
	_actor2.setPosition(Common::Point(21, 128));
	_actor2.fixPriority(10);
	_actor2.setDetails(700, 3, -1, -1, 1, (SceneItem *) NULL);

	_actor3.postInit();
	_actor3.setup(700, 2, 1);
	_actor3.setPosition(Common::Point(217, 120));
	_actor3.fixPriority(10);
	_actor3.setDetails(700, 15, -1, -1, 1, (SceneItem *) NULL);

	_actor1.postInit();
	_actor1.setup(700, 4, 1);
	_actor1.setPosition(Common::Point(355 - ((R2_GLOBALS._v565E3 * 8) / 5), ((R2_GLOBALS._v565E1 + 20 ) / 5) - 12));
	_actor1.fixPriority(10);
	_actor1.setDetails(700, 12, -1, 14, 1, (SceneItem *) NULL);

	_actor6.postInit();
	_actor6.setup(700, 8, 1);
	_actor6.setPosition(Common::Point(85, 53));
	_actor6.setDetails(700, 33, -1, 35, 1, (SceneItem *) NULL);

	_actor7.postInit();
	_actor7.setup(700, 8, 1);
	_actor7.setPosition(Common::Point(164, 53));
	_actor7.setDetails(700, 33, -1, 35, 1, (SceneItem *) NULL);

	_actor8.postInit();
	_actor8.setup(700, 8, 1);
	_actor8.setPosition(Common::Point(243, 53));
	_actor8.setDetails(700, 33, -1, 35, 1, (SceneItem *) NULL);

	_actor9.postInit();
	_actor9.setup(700, 8, 1);
	_actor9.setPosition(Common::Point(324, 53));
	_actor9.setDetails(700, 33, -1, 35, 1, (SceneItem *) NULL);

	if ((R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) != 1) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) != 1)) {
		_actor5.postInit();
		_actor5.fixPriority(10);
		switch (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS)) {
		case 0:
			switch (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS)) {
			case 0:
				_actor5.setup(701, 3, 2);
				_actor5.setPosition(Common::Point(243, 98));
				_actor5.setDetails(700, 37, -1, -1, 1, (SceneItem *) NULL);
				break;
			case 700:
				_actor5.setup(701, 3, 1);
				_actor5.setPosition(Common::Point(356 - (R2_GLOBALS._v565EB * 8), 148 - (((R2_GLOBALS._v565E9 + 10) / 5) * 4)));
				_actor5.setDetails(700, 37, -1, -1, 1, (SceneItem *) NULL);
				break;
			default:
				break;
			}
			break;
		case 700:
			switch (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS)) {
			case 0:
				if ((R2_GLOBALS._v565E5 != 0) && (R2_GLOBALS._v565E1 == 20) && (R2_GLOBALS._v565E3 == 70))
					_actor5.setup(701, 2, 1);
				else
					_actor5.setup(701, 2, 8);
				_actor5.setPosition(Common::Point(243, 98));
				_actor5.fixPriority(77);
				_actor5.setDetails(700, 38, -1, -1, 1, (SceneItem *) NULL);
				break;
			case 700:
				_actor5.setup(701, 1, 8);
				if (R2_GLOBALS._v565E7 == 0) {
					_actor5.setPosition(Common::Point(356 - (R2_GLOBALS._v565EB * 8), 148 - (((R2_GLOBALS._v565E9 + 10) / 5) * 4)));
				} else {
					_actor5.setup(701, 1, 1);
					_actor5.setPosition(Common::Point(_actor1._position.x + 1, _actor1._position.y + 120));
				}
				_actor5.setDetails(700, 38, -1, -1, 1, (SceneItem *) NULL);
				break;
			default:
				break;
			}
		default:
			break;
		}
	}

	_actor4.postInit();
	_actor4.setup(700, 3, 1);
	_actor4.setPosition(Common::Point(454, 117));
	_actor4.setDetails(700, 27, -1, -1, 1, (SceneItem *) NULL);

	_item12.setDetails(Rect(234, 90, 252, 110), 700, 39, -1, -1, 1, NULL);
	_item6.setDetails(Rect(91, 158, 385, 167), 700, 6, -1, 8, 1, NULL);
	_item2.setDetails(Rect(47, 115, 149, 124), 700, 40, -1, 41, 1, NULL);
	_item3.setDetails(Rect(151, 108, 187, 124), 700, 40, -1, 41, 1, NULL);
	_item4.setDetails(Rect(247, 108, 275, 124), 700, 40, -1, 41, 1, NULL);
	_item5.setDetails(Rect(300, 105, 321, 124), 700, 40, -1, 41, 1, NULL);
	_item7.setDetails(Rect(255, 74, 368, 115), 700, 9, -1, 11, 1, NULL);
	_item8.setDetails(Rect(69, 74, 182, 115), 700, 9, -1, 11, 1, NULL);
	_item9.setDetails(Rect(370, 58, 475, 103), 700, 18, -1, -1, 1, NULL);
	_item10.setDetails(Rect(17, 11, 393, 31), 700, 24, -1, -1, 1, NULL);
	_item11.setDetails(Rect(42, 32, 368, 66), 700, 30, -1, 32, 1, NULL);
	_item1.setDetails(Rect(0, 0, 480, 200), 700, 0, -1, -1, 1, NULL);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(11);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player._moveDiff.x = 2;
	R2_GLOBALS._player.disableControl();

	R2_GLOBALS._sound1.play(34);

	_sceneMode = 700;

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 250:
		setAction(&_sequenceManager, this, 700, &R2_GLOBALS._player, &_actor2, NULL);
		break;
	case 500:
		setAction(&_sequenceManager, this, 703, &R2_GLOBALS._player, &_actor3, NULL);
		break;
	case 600: {
		_sceneMode = 4;
		_actor7.setFrame(5);
		R2_GLOBALS._player.setPosition(Common::Point(164, 74));
		R2_GLOBALS._player.setStrip2(3);
		Common::Point pt(164, 69);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 900:
		setAction(&_sequenceManager, this, 705, &R2_GLOBALS._player, &_actor4, NULL);
		break;
	default:
		if (R2_GLOBALS.getFlag(41))
			R2_GLOBALS._player.setPosition(Common::Point(107, 67));
		else
			R2_GLOBALS._player.setPosition(Common::Point(60, 140));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	}
}

void Scene700::remove() {
	R2_GLOBALS._sound1.play(10);
// CHECKME: Present in the original... But it crashes badly.
// The instruction was removed as it's not used in other scene coded the same way
// and reversed by dreammaster. A double check is required in order to verify it doesn't hide
// a memory leak
//	_rotation->remove();
	SceneExt::remove();
}

void Scene700::signal() {
	switch (_sceneMode) {
	case 1:
		_sceneMode = 2;
		R2_GLOBALS._player.setStrip(4);
		if (R2_GLOBALS._player._position.x != 164) {
			SceneItem::display(700, 36, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
			R2_GLOBALS._player.enableControl();
		} else {
			R2_GLOBALS._sound2.play(19);
			_actor7.animate(ANIM_MODE_5, this);
		}
		break;
	case 2: {
		_sceneMode = 3;
		R2_GLOBALS._player.setStrip2(4);
		Common::Point pt(R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y + 5);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 3:
		R2_INVENTORY.setObjectScene(5, 600);
		R2_INVENTORY.setObjectScene(16, 700);
		R2_GLOBALS._player.enableControl();
		break;
	case 4:
		_sceneMode = 5;
		R2_GLOBALS._player.setStrip2(-1);
		R2_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		R2_GLOBALS._sound2.play(19);
		_actor7.animate(ANIM_MODE_6, this);
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS.setFlag(41);
		break;
	case 10:
		_sceneMode = 11;
		R2_GLOBALS._player.setup(16, 7, 1);
		R2_GLOBALS._player.changeZoom(50);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 11:
		_sceneMode = 12;
		_actor5.remove();
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 12:
		R2_GLOBALS._player.setVisage(11);
		R2_GLOBALS._player.changeZoom(100);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		R2_GLOBALS._player._strip = 7;
		if (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_UNIT) == 0) {
			R2_INVENTORY.setObjectScene(R2_ATTRACTOR_CABLE_HARNESS, 1);
			R2_INVENTORY.setObjectScene(R2_CABLE_HARNESS, 0);
		} else {
			R2_INVENTORY.setObjectScene(R2_CABLE_HARNESS, 1);
		}
		R2_GLOBALS._player.enableControl();
		break;
	case 15:
		_sceneMode = 16;
		R2_GLOBALS._player.setup(16, 7, 1);
		R2_GLOBALS._player.changeZoom(50);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 16:
		_sceneMode = 17;
		_actor5.setup(701, 1, 8);
		_actor5.setDetails(700, 38, -1, -1, 3, (SceneItem *) NULL);
		if ((R2_GLOBALS._v565E5 != 0) && (_actor5._position.x == _actor1._position.x + 1) && (_actor5._position.x == 148 - (((R2_GLOBALS._v565E1 + 10) / 5) * 4))) {
			_actor5.animate(ANIM_MODE_6, NULL);
			Common::Point pt(_actor5._position.x, _actor1._position.y + 120);
			NpcMover *mover = new NpcMover();
			_actor5.addMover(mover, &pt, NULL);
			R2_GLOBALS._v565E7 = 1;
		}
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 17:
		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_UNIT, 0);
		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_CABLE_HARNESS, 700);
		R2_GLOBALS._player.setVisage(11);
		R2_GLOBALS._player.changeZoom(100);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		R2_GLOBALS._player._strip = 7;
		R2_GLOBALS._player.enableControl();
		break;
	case 20:
		R2_INVENTORY.setObjectScene(R2_CABLE_HARNESS, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 21:
		_actor5.fixPriority(77);
		if ((R2_GLOBALS._v565E5 != 0) && (R2_GLOBALS._v565E1 == 20) && (R2_GLOBALS._v565E3 == 70))
			_actor5.animate(ANIM_MODE_6, NULL);

		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_CABLE_HARNESS, 700);
		R2_GLOBALS._player.enableControl();
		break;
	case 701:
		R2_GLOBALS._sceneManager.changeScene(250);
		break;
	case 702:
		R2_GLOBALS._sceneManager.changeScene(500);
		break;
	case 704:
		R2_GLOBALS._sceneManager.changeScene(900);
		break;
	case 706:
		_actor5.setDetails(700, 38, -1, -1, 3, (SceneItem *) NULL);
		_actor5.fixPriority(77);
		if ((R2_GLOBALS._v565E5 != 0) && (R2_GLOBALS._v565E1 == 20) && (R2_GLOBALS._v565E3 == 70))
			_actor5.animate(ANIM_MODE_6, NULL);
		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_UNIT, 0);
		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_CABLE_HARNESS, 700);
		R2_GLOBALS._player.enableControl();
		break;
	case 709:
		R2_GLOBALS.setFlag(41);
		R2_GLOBALS._player.enableControl();
		break;
	case 710:
		R2_GLOBALS.clearFlag(41);
		R2_GLOBALS._player.enableControl();
		break;
	case 5:
	// No break on purpose
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 800 - Sick Bay
 *
 *--------------------------------------------------------------------------*/

bool Scene800::Button::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE) {
		return NamedHotspot::startAction(action, event);
	} else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 802;
		scene->setAction(&scene->_sequenceManager1, scene, 802, &R2_GLOBALS._player, &scene->_autodocCover, NULL);
		return true;
	}
}

bool Scene800::CableJunction::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	if (action != R2_OPTICAL_FIBRE) {
		return NamedHotspot::startAction(action, event);
	} else {
		R2_GLOBALS._player.disableControl();
		scene->_opticalFibre.postInit();
		scene->_sceneMode = 803;

		if (R2_INVENTORY.getObjectScene(R2_READER) == 800)
			scene->setAction(&scene->_sequenceManager1, scene, 813, &R2_GLOBALS._player, &scene->_opticalFibre, &scene->_reader, NULL);
		else
			scene->setAction(&scene->_sequenceManager1, scene, 803, &R2_GLOBALS._player, &scene->_opticalFibre, NULL);

		return true;
	}
}

bool Scene800::DeviceSlot::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_INVENTORY.getObjectScene(R2_READER) != 800)
			break;

		R2_GLOBALS._player.disableControl();
		scene->_reader.postInit();

		if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) == 800)
			scene->setAction(&scene->_sequenceManager1, scene, 814, &R2_GLOBALS._player, &scene->_reader, &scene->_opticalFibre, NULL);
		else
			scene->setAction(&scene->_sequenceManager1, scene, 804, &R2_GLOBALS._player, &scene->_reader, NULL);
		return true;
	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

bool Scene800::Door::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 801;
		scene->setAction(&scene->_sequenceManager1, scene, 801, &R2_GLOBALS._player, &scene->_door, NULL);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene800::Tray::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (!R2_GLOBALS.getFlag(10)) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 806;
			scene->setAction(&scene->_sequenceManager1, scene, 806, &R2_GLOBALS._player, &scene->_tray, NULL);
		} else if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 825) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 808;
			scene->setAction(&scene->_sequenceManager1, scene, 808, &R2_GLOBALS._player, &scene->_tray, NULL);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 807;
			scene->setAction(&scene->_sequenceManager1, scene, 807, &R2_GLOBALS._player, &scene->_tray, NULL);
		}
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene800::ComScanner::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (scene->_cabinet._frame == 1)
			return false;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 811;
		scene->setAction(&scene->_sequenceManager1, scene, 811, &R2_GLOBALS._player, &scene->_comScanner, NULL);
		return true;
	case CURSOR_TALK:
		SceneItem::display2(800, 35);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene800::Cabinet::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();

		if (scene->_cabinet._frame == 1) {
			scene->_sceneMode = 810;
			scene->setAction(&scene->_sequenceManager1, scene, 810, &R2_GLOBALS._player, &scene->_cabinet, NULL);
			R2_GLOBALS.setFlag(56);
		} else {
			scene->_sceneMode = 812;
			scene->setAction(&scene->_sequenceManager1, scene, 812, &R2_GLOBALS._player, &scene->_cabinet, NULL);
			R2_GLOBALS.clearFlag(56);
		}
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene800::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(800);

	_door.postInit();
	_door.setVisage(800);
	_door.setPosition(Common::Point(286, 108));
	_door.fixPriority(50);
	_door.setDetails(800, 3, -1, -1, 1, (SceneItem *)NULL);

	_autodocCover.postInit();
	_autodocCover.setup(800, 2, 1);
	_autodocCover.setPosition(Common::Point(119, 161));
	_autodocCover.setDetails(800, 6, 7, -1, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) == 800) {
		_opticalFibre.postInit();
		if (R2_INVENTORY.getObjectScene(R2_READER) == 800)
			_opticalFibre.setup(800, 4, 1);
		else
			_opticalFibre.setup(800, 7, 2);

		_opticalFibre.setPosition(Common::Point(220, 124));
		_opticalFibre.fixPriority(140);
	}

	if (R2_INVENTORY.getObjectScene(R2_READER) == 800) {
		_reader.postInit();

		if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) == 800) {
			_opticalFibre.setup(800, 4, 1);
			_reader.hide();
		} else {
			_reader.setup(800, 7, 1);
		}

		_reader.setPosition(Common::Point(230, 120));
		_reader.fixPriority(140);
	}

	_cabinet.postInit();
	_cabinet.setup(801, 1, R2_GLOBALS.getFlag(56) ? 6 : 1);
	_cabinet.setPosition(Common::Point(169, 79));
	_cabinet.setDetails(800, 41, -1, -1, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_COM_SCANNER) == 800) {
		_comScanner.postInit();
		_comScanner.setup(801, 2, 1);
		_comScanner.setPosition(Common::Point(174, 73));
		_comScanner.setDetails(800, 34, 35, -1, 1, (SceneItem *)NULL);
	}

	_tray.postInit();
	_tray.setup(800, R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 825 ? 6 : 5, 1);
	if (R2_GLOBALS.getFlag(10))
		_tray.setFrame(5);
	_tray.setPosition(Common::Point(203, 144));
	_tray.setDetails(800, 12, -1, 14, 1, (SceneItem *)NULL);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_dataConduits.setDetails(13, 800, 21, -1, -1);
	_cableJunction.setDetails(Rect(206, 111, 223, 125), 800, 24, -1, -1, 1, NULL);
	_deviceSlot.setDetails(Rect(220, 108, 239, 122), 800, 27, -1, -1, 1, NULL);
	_diskSlot.setDetails(Rect(209, 124, 226, 133), 800, 9, -1, 11, 1, NULL);

	if (R2_INVENTORY.getObjectScene(R2_READER) == 800)
		_deviceSlot._lookLineNum = 33;

	_button.setDetails(Rect(189, 112, 204, 124), 800, 30, -1, -1, 1, NULL);
	_couch.setDetails(11, 800, 15, -1, 17);
	_autoDoc.setDetails(Rect(152, 92, 247, 151), 800, 6, 7, -1, 1, NULL);
	_medicalDatabase.setDetails(12, 800, 18, -1, -1);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 800, 0, -1, -1, 1, NULL);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 825:
		_sceneMode = 800;
		setAction(&_sequenceManager1, this, 805, &R2_GLOBALS._player, &_autodocCover, NULL);
		break;
	case 850:
		_sceneMode = 800;
		setAction(&_sequenceManager1, this, 800, &R2_GLOBALS._player, &_door, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(277, 132));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene800::signal() {
	switch (_sceneMode) {
	case 801:
		R2_GLOBALS._sceneManager.changeScene(850);
		break;
	case 802:
		R2_GLOBALS._sceneManager.changeScene(825);
		break;
	case 803:
		R2_GLOBALS._player.enableControl();
		R2_INVENTORY.setObjectScene(R2_OPTICAL_FIBRE, 800);
		break;
	case 804:
		R2_GLOBALS._player.enableControl();
		_deviceSlot._lookLineNum = 33;
		R2_INVENTORY.setObjectScene(R2_READER, 800);
		break;
	case 806:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS.setFlag(10);
		break;
	case 807:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS.clearFlag(10);
		break;
	case 808:
		R2_GLOBALS._player.enableControl();
		R2_INVENTORY.setObjectScene(R2_OPTO_DISK, 1);
		break;
	case 809:
		R2_GLOBALS._player.enableControl();
		R2_INVENTORY.setObjectScene(R2_READER, 1);
		break;
	case 811:
		R2_GLOBALS._player.enableControl();
		_comScanner.remove();
		R2_INVENTORY.setObjectScene(R2_COM_SCANNER, 1);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 825 - Autodoc
 *
 *--------------------------------------------------------------------------*/

Scene825::Button::Button(): SceneObject() {
	_buttonId = 0;
	_v2 = 0;
	_buttonDown = false;
}

void Scene825::Button::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	s.syncAsSint16LE(_buttonId);
	s.syncAsSint16LE(_v2);
	s.syncAsSint16LE(_buttonDown);
}

void Scene825::Button::process(Event &event) {
	Scene825 *scene = (Scene825 *)R2_GLOBALS._sceneManager._scene;

	if (!event.handled) {
		if ((event.eventType == EVENT_BUTTON_DOWN) && _bounds.contains(event.mousePos) && !_buttonDown) {
			scene->_sound1.play(14);
			setFrame(2);
			_buttonDown = true;
			event.handled = true;
		}

		if ((event.eventType == EVENT_BUTTON_UP) && _buttonDown) {
			setFrame(1);
			_buttonDown = false;
			event.handled = true;

			scene->doButtonPress(_buttonId);
		}
	}
}

bool Scene825::Button::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE)
		return false;
	else
		return SceneObject::startAction(action, event);
}

void Scene825::Button::setButton(int buttonId) {
	SceneObject::postInit();
	_v2 = buttonId;
	_buttonDown = 0;
	_sceneText._color1 = 92;
	_sceneText._color2 = 0;
	_sceneText._width = 200;
	_sceneText.fixPriority(20);
	_sceneText._fontNumber = 50;

	switch (buttonId) {
	case 1:
		_sceneText.setPosition(Common::Point(95, 58));
		break;
	case 2:
		_sceneText.setPosition(Common::Point(98, 75));
		break;
	case 3:
		_sceneText.setPosition(Common::Point(102, 95));
		break;
	case 4:
		_sceneText.setPosition(Common::Point(180, 58));
		_sceneText._textMode = ALIGN_RIGHT;
		break;
	case 5:
		_sceneText.setPosition(Common::Point(177, 75));
		_sceneText._textMode = ALIGN_RIGHT;
		break;
	case 6:
		_sceneText.setPosition(Common::Point(175, 95));
		_sceneText._textMode = ALIGN_RIGHT;
		break;
	default:
		break;
	}

	setDetails(825, 6, 7, -1, 2, (SceneItem *)NULL);
}

void Scene825::Button::setText(int textId) {
	Scene825 *scene = (Scene825 *)R2_GLOBALS._sceneManager._scene;

	_buttonId = textId;
	_lookLineNum = textId;

	_sceneText.remove();
	if (_buttonId != 0)
		_sceneText.setup(scene->_autodocItems[textId - 1]);
}

/*--------------------------------------------------------------------------*/

Scene825::Scene825(): SceneExt() {
	_menuId = _frame1 = _frame2 = 0;

	// Setup Autodoc items list
	_autodocItems[0] = MAIN_MENU;
	_autodocItems[1] = DIAGNOSIS;
	_autodocItems[2] = ADVANCED_PROCEDURES;
	_autodocItems[3] = VITAL_SIGNS;
	_autodocItems[4] = OPEN_DOOR;
	_autodocItems[5] = TREATMENTS;
	_autodocItems[6] = NO_MALADY_DETECTED;
	_autodocItems[7] = NO_TREATMENT_REQUIRED;
	_autodocItems[8] = ACCESS_CODE_REQUIRED;
	_autodocItems[9] = INVALID_ACCESS_CODE;
	_autodocItems[10] = FOREIGN_OBJECT_EXTRACTED;
}

void Scene825::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(825);
	R2_GLOBALS._player._uiEnabled = false;
	BF_GLOBALS._interfaceY = 200;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player._effect = 0;
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_item2.setDetails(1, 825, 3, 4, 5);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 825, 0, -1, -1, 1, NULL);

	_sceneMode = 10;
	signal();
}

void Scene825::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_menuId);
	s.syncAsSint16LE(_frame1);
	s.syncAsSint16LE(_frame2);
}

void Scene825::remove() {
	SceneExt::remove();
	R2_GLOBALS._player._uiEnabled = true;
}

void Scene825::signal() {
	switch (_sceneMode) {
	case 10:
		_button1.setButton(1);
		_button1.setup(825, 1, 1);
		_button1.setPosition(Common::Point(71, 71));
		_button2.setButton(2);
		_button2.setup(825, 3, 1);
		_button2.setPosition(Common::Point(74, 90));
		_button3.setButton(3);
		_button3.setup(825, 5, 1);
		_button3.setPosition(Common::Point(78, 109));
		_button4.setButton(4);
		_button4.setup(825, 2, 1);
		_button4.setPosition(Common::Point(248, 71));
		_button5.setButton(5);
		_button5.setup(825, 4, 1);
		_button5.setPosition(Common::Point(245, 90));
		_button6.setButton(6);
		_button6.setup(825, 6, 1);
		_button6.setPosition(Common::Point(241, 109));

		doButtonPress(1);
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 825:
		_object5.remove();
		_sceneText._color1 = 92;
		_sceneText._color2 = 0;
		_sceneText._width = 200;
		_sceneText.fixPriority(20);
		_sceneText._fontNumber = 50;
		_sceneText.setPosition(Common::Point(120, 75));
		_sceneText.setup(NO_MALADY_DETECTED);
		_sceneMode = 826;
		setAction(&_sequenceManager1, this, 826, &R2_GLOBALS._player, NULL);
		break;
	case 826:
		_sceneText.remove();
		doButtonPress(1);
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 827:
		_object5.remove();
		R2_INVENTORY.setObjectScene(R2_OPTO_DISK, 825);
		_sceneText.setPosition(Common::Point(108, 75));
		_sceneText.setup(FOREIGN_OBJECT_EXTRACTED);
		_sceneMode = 826;
		setAction(&_sequenceManager1, this, 826, &R2_GLOBALS._player, NULL);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene825::process(Event &event) {
	SceneExt::process(event);

	if (R2_GLOBALS._player._uiEnabled) {
		_button1.process(event);
		_button2.process(event);
		_button3.process(event);
		_button4.process(event);
		_button5.process(event);
		_button6.process(event);
	}
}

void Scene825::dispatch() {
	if (R2_GLOBALS._sceneObjects->contains(&_object4) &&
			((_object4._frame == 1) || (_object4._frame == 3)) &&
			(_object4._frame != _frame1)) {
		_sound2.play(25);
	}

	if (R2_GLOBALS._sceneObjects->contains(&_object1) &&
			(_object1._frame == 3) && (_object1._frame != _frame2)) {
		_sound3.play(26);
	}

	_frame1 = _object4._frame;
	_frame2 = _object1._frame;

	Scene::dispatch();
}

void Scene825::doButtonPress(int buttonId) {
	if ((_menuId != 4) || (buttonId == 5)) {
		_button1.setText(0);
		_button2.setText(0);
		_button3.setText(0);
		_button4.setText(0);
		_button5.setText(0);
		_button6.setText(0);

		switch (buttonId) {
		case 2:
			R2_GLOBALS._player.disableControl();
			_object5.postInit();
			_sceneMode = 825;
			setAction(&_sequenceManager1, this, 825, &R2_GLOBALS._player, &_object5, NULL);
			break;
		case 3:
			R2_GLOBALS._player.disableControl();
			_sceneText._color1 = 92;
			_sceneText._color2 = 0;
			_sceneText._width = 200;
			_sceneText.fixPriority(20);
			_sceneText._fontNumber = 50;
			_sceneText.setPosition(Common::Point(115, 75));

			if (R2_GLOBALS.getFlag(4)) {
				if ((R2_INVENTORY.getObjectScene(R2_READER) != 800) ||
						(R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) != 800)) {
					_sceneText.setPosition(Common::Point(116, 75));
					_sceneText.setup(ACCESS_CODE_REQUIRED);
				} else if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) != 800) {
					_sceneText.setPosition(Common::Point(115, 75));
					_sceneText.setup(NO_TREATMENT_REQUIRED);
				} else {
					_button6._buttonId = 5;

					_object5.postInit();
					setAction(&_sequenceManager1, this, 827, &_object5, NULL);
				}
			} else {
				R2_GLOBALS.setFlag(2);

				if ((R2_INVENTORY.getObjectScene(R2_READER) != 800) ||
						(R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) != 800)) {
					_sceneText.setPosition(Common::Point(116, 75));
					_sceneText.setup(ACCESS_CODE_REQUIRED);
				} else {
					_sceneText.setPosition(Common::Point(119, 75));
					_sceneText.setup(INVALID_ACCESS_CODE);
				}
			}

			if (_sceneMode != 827) {
				_sceneMode = 826;
				setAction(&_sequenceManager1, this, 826, &R2_GLOBALS._player, NULL);
			}
			break;
		case 4:
			_sound4.play(27);
			_button6._buttonId = 5;

			_object1.postInit();
			_object1.setup(826, 7, 1);
			_object1.setPosition(Common::Point(112, 67));
			_object1._numFrames = 1;
			_object1.animate(ANIM_MODE_2);

			_object2.postInit();
			_object2.setup(826, 5, 1);
			_object2.setPosition(Common::Point(158, 67));
			_object2._numFrames = 5;
			_object2.animate(ANIM_MODE_2);

			_object3.postInit();
			_object3.setup(826, 6, 1);
			_object3.setPosition(Common::Point(206, 67));
			_object3._numFrames = 1;
			_object3.animate(ANIM_MODE_2);

			_object4.postInit();
			_object4.setup(826, 8, 1);
			_object4.setPosition(Common::Point(158, 84));
			_object4._numFrames = 1;
			_object4.animate(ANIM_MODE_2);

			_object5.postInit();
			_object5.setup(826, 4, 1);
			_object5.setPosition(Common::Point(161, 110));
			break;
		case 5:
			R2_GLOBALS._player.disableControl();
			if (_menuId == 4) {
				_menuId = 0;

				_object1.remove();
				_object2.remove();
				_object3.remove();
				_object4.remove();
				_object5.remove();

				_sound2.stop();
				_sound3.stop();
				_sound4.stop();

				doButtonPress(1);
				R2_GLOBALS._player.enableControl();
				R2_GLOBALS._player._canWalk = false;
			} else {
				R2_GLOBALS._sceneManager.changeScene(800);
			}
			break;
		case 6:
			R2_GLOBALS._player.disableControl();
			_sceneText._color1 = 92;
			_sceneText._color2 = 0;
			_sceneText._width = 200;
			_sceneText.fixPriority(20);
			_sceneText._fontNumber = 50;
			_sceneText.setPosition(Common::Point(115, 75));
			_sceneText.setup(NO_TREATMENT_REQUIRED);

			_sceneMode = 826;
			setAction(&_sequenceManager1, this, 826, &R2_GLOBALS._player, NULL);
			break;
		default:
			_button1.setText(2);
			_button2.setText(3);
			_button3.setText(4);
			_button4.setText(6);
			_button6.setText(5);
			break;
		}

		_menuId = buttonId;
	}
}

/*--------------------------------------------------------------------------
 * Scene 850 - Deck #5 - By Lift
 *
 *--------------------------------------------------------------------------*/

bool Scene850::Indicator::startAction(CursorType action, Event &event) {
	Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

	if ((action != CURSOR_USE) || (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) != 850))
		return NamedHotspot::startAction(action, event);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 851;
		scene->setAction(&scene->_sequenceManager1, scene, 851, &R2_GLOBALS._player, &scene->_fibre, NULL);
		return true;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene850::LiftDoor::startAction(CursorType action, Event &event) {
	Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 202;
		scene->setAction(&scene->_sequenceManager1, scene, 202, &R2_GLOBALS._player, this, NULL);
		return true;
	}
}

bool Scene850::SickBayDoor::startAction(CursorType action, Event &event) {
	Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 204;
		scene->setAction(&scene->_sequenceManager1, scene, 204, &R2_GLOBALS._player, this, NULL);
		return true;
	}
}

bool Scene850::Clamp::startAction(CursorType action, Event &event) {
	Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

	if (!R2_GLOBALS.getFlag(7))
		return false;
	else if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_object1.postInit();
		scene->_sceneMode = 850;
		scene->setAction(&scene->_sequenceManager1, scene, 850, &R2_GLOBALS._player, this, &scene->_object1, NULL);
		return true;
	}
}

bool Scene850::Panel::startAction(CursorType action, Event &event) {
	Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

	if ((action != CURSOR_USE) || R2_GLOBALS.getFlag(7))
		return SceneActor::startAction(action, event);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 852;
		scene->setAction(&scene->_sequenceManager1, scene, 852, &R2_GLOBALS._player, this, &scene->_object1, NULL);
		return true;
	}
}

/*--------------------------------------------------------------------------*/

void Scene850::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(850);

	_liftDoor.postInit();
	_liftDoor.setup(850, 2, 1);
	_liftDoor.setPosition(Common::Point(188, 79));
	_liftDoor.setDetails(850, 3, -1, -1, 1, (SceneItem *)NULL);

	_sickBayDoor.postInit();
	_sickBayDoor.setup(850, 3, 1);
	_sickBayDoor.setPosition(Common::Point(62, 84));
	_sickBayDoor.setDetails(850, 9, -1, -1, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_CLAMP) == 850) {
		_clamp.postInit();
		_clamp.setup(850, 5, 1);
		_clamp.setPosition(Common::Point(242, 93));
		_clamp.fixPriority(81);
		_clamp.animate(ANIM_MODE_2, NULL);
		_clamp.setDetails(850, 27, -1, -1, 1, (SceneItem *)NULL);
	}

	_panel.postInit();
	_panel.setVisage(850);

	if (R2_GLOBALS.getFlag(7))
		_panel.setFrame(7);

	_panel.setPosition(Common::Point(232, 119));
	_panel.fixPriority(82);
	_panel.setDetails(850, 24, -1, -1, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) == 850) {
		_fibre.postInit();
		_fibre.setup(850, 6, 1);
		_fibre.setPosition(Common::Point(280, 87));
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_eastDoor.setDetails(Rect(289, 53, 315, 125), 850, 6, -1, 8, 1, NULL);
	_indicator.setDetails(Rect(275, 67, 286, 79), 850, 18, -1, 20, 1, NULL);
	_sickBayIndicator.setDetails(Rect(41, 51, 48, 61), 850, 15, -1, -1, 1, NULL);
	_liftControls.setDetails(Rect(156, 32, 166, 44), 850, 21, -1, -1, 1, NULL);
	_compartment.setDetails(Rect(4, 88, 153, 167), 850, 12, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 850, 0, -1, -1, 1, NULL);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 250:
		_sceneMode = 203;
		setAction(&_sequenceManager1, this, 203, &R2_GLOBALS._player, &_liftDoor, NULL);
		break;
	case 800:
		_sceneMode = 205;
		setAction(&_sequenceManager1, this, 205, &R2_GLOBALS._player, &_sickBayDoor, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(215, 115));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene850::signal() {
	switch (_sceneMode) {
	case 202:
		R2_GLOBALS._sceneManager.changeScene(250);
		break;
	case 204:
		R2_GLOBALS._sceneManager.changeScene(800);
		break;
	case 850:
		R2_INVENTORY.setObjectScene(R2_CLAMP, 1);
		_clamp.remove();
		_object1.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 851:
		R2_INVENTORY.setObjectScene(R2_OPTICAL_FIBRE, 1);
		_fibre.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 852:
		R2_GLOBALS.setFlag(7);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 900 -
 *
 *--------------------------------------------------------------------------*/
Scene900::Actor4::Actor4() {
	_fieldA4 = 0;
}

void Scene900::Actor4::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_fieldA4);
}

void Scene900::Actor4::sub96135(int arg1) {
	_fieldA4 = arg1;
	setDetails(900, -1, -1, -1, 2, (SceneItem *) NULL);
}

Scene900::Scene900() {
	_field412 = 0;
	_field414 = 0;
	_field416 = 0;
}

void Scene900::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_field414);
	s.syncAsSint16LE(_field416);
}

bool Scene900::Actor4::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._sound2.play(14);
		switch (_fieldA4) {
		case 2:
			if (scene->_field412 == 1) {
				scene->_sceneMode = 2;
				scene->signal();
			} else if (scene->_field412 == 2) {
				if (R2_GLOBALS._v565E5 == 0) {
					scene->_aSound1.play(30);
					setup(900, 3, 11);
					R2_GLOBALS._v565E5 = 1;
					if ((R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 0) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS == 700)) && (R2_GLOBALS._v565E1 == 20) && (R2_GLOBALS._v565E3 == 70) && (scene->_actor2._animateMode != ANIM_MODE_6)) {
						scene->_actor2.animate(ANIM_MODE_6, NULL);
					} else {
						if (((scene->_actor3._percent * 49) / 100) + scene->_actor3._position.x == scene->_actor2._position.x) {
							if (scene->_actor2._position.x == 166 - (R2_GLOBALS._v565E3 / 15)) {
								R2_GLOBALS._player.disableControl();
								scene->_sceneMode = 4;
								scene->_actor2._moveDiff.y = (scene->_actor2._position.y - (scene->_actor3._position.y + ((scene->_actor3._percent * 3) / 10) - 2)) / 9;
								Common::Point pt(scene->_actor3._position.x + ((scene->_actor3._percent * 49) / 100), scene->_actor3._position.y + ((scene->_actor3._percent * 3) / 10) - 2);
								NpcMover *mover = new NpcMover();
								scene->_actor2.addMover(mover, &pt, this);
								scene->_actor2.animate(ANIM_MODE_6, NULL);
							}
						}
					}
				} else {
					scene->_aSound1.play(53);
					setup(900, 3, 9);
					R2_GLOBALS._v565E5 = 0;

					if ((R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 0) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) == 700) && (scene->_actor2._frame < 8) && (scene->_actor2._animateMode != ANIM_MODE_5)) {
							scene->_actor2.animate(ANIM_MODE_5, NULL);
					} else if ((R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 700) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) == 700) && (scene->_actor2._frame < 8)) {
						R2_GLOBALS._v565E7 = 0;
						if (scene->_actor2._animateMode != 5) {
							R2_GLOBALS._player.disableControl();
							scene->_sceneMode = 5;
							scene->_actor2.animate(ANIM_MODE_5, NULL);
							scene->_actor2._moveDiff.y = (166 - scene->_actor2._position.y) / 9;
							Common::Point pt(scene->_actor2._position.x, 166 - (R2_GLOBALS._v565E3 / 15));
							NpcMover *mover = new NpcMover();
							scene->_actor2.addMover(mover, &pt, this);
						}
					}
				}
			}
			return true;
			break;
		case 3:
			if (scene->_field412 == 1) {
				scene->_sceneMode = 3;
				scene->signal();
			}
			return true;
			break;
		case 4:
			if ((scene->_field416 == 0) && (scene->_field414 == 0) && (R2_GLOBALS._v565E3 != 0)) {
				scene->_aSound1.play(38);
				scene->_field416 = -5;
			}
			return true;
			break;
		case 5:
			if ((scene->_field416 == 0) && (scene->_field414 == 0) && (R2_GLOBALS._v565E3 < 135)) {
				scene->_aSound1.play(38);
				scene->_field416 = 5;
			}
			return true;
			break;
		case 6:
			if ((scene->_field416 == 0) && (scene->_field414 == 0) && (R2_GLOBALS._v565E1 > -10)) {
				scene->_aSound1.play(38);
				scene->_field414 = -5;
			}
			return true;
			break;
		case 7:
			if ((scene->_field416 == 0) && (scene->_field414 == 0) && (R2_GLOBALS._v565E1 < 20)) {
				scene->_aSound1.play(38);
				scene->_field414 = 5;
			}
			return true;
			break;
		case 8:
			SceneItem::display(5, 11, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
			return true;
			break;
		case 9:
			SceneItem::display(5, 12, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
			return true;
			break;
		default:
			if (scene->_field412 == 1) {
				R2_GLOBALS._player.disableControl();
				scene->_actor5.remove();
				scene->_actor6.remove();
				scene->_actor7.remove();
				scene->_actor8.remove();
				scene->_actor9.remove();
				scene->_actor10.remove();
				R2_GLOBALS._sound2.play(37);
				scene->_sceneMode = 901;
				scene->setAction(&scene->_sequenceManager1, scene, 901, &scene->_actor1, this ,NULL);
			} else if ((scene->_field412 == 2) || (scene->_field412 == 3)) {
				scene->_sceneMode = 1;
				scene->signal();
			}

			return true;
			break;
		}
	} else if (action == CURSOR_LOOK) {
		if ((_fieldA4 == 2) && (scene->_field412 == 2))
			SceneItem::display(900, 21, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		else
			SceneItem::display(900, _fieldA4, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

void Scene900::postInit(SceneObjectList *OwnerList) {
	g_globals->gfxManager()._bounds.moveTo(Common::Point(0, 0));
	loadScene(900);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(34);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor1.setDetails(900, 3, -1, -1, 1, (SceneItem *) NULL);

	_field414 = 0;
	_field416 = 0;

	_actor3.postInit();
	_actor3.fixPriority(1);
	// useless, the original use it for debugging purposes: strcpy(_actor3._actorName, "Crane");
	_actor3.setup(900, 1, 2);
	_actor3.setPosition(Common::Point(89, 0));
	_actor3._effect = 1;
	_actor3.setDetails(900, 6, -1, 8, 1, (SceneItem *) NULL);

	if ((R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) != 1) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) != 1)) {
		_actor2.postInit();
		_actor2.setPosition(Common::Point(0, 0));
		_actor2.fixPriority(1);

		if (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 0) {
			if (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) != 700) {
				_actor2.setup(901, 3, 2);
			} else if ((R2_GLOBALS._v565E5 != 0) && (R2_GLOBALS._v565E1 == 20) && (R2_GLOBALS._v565E3 == 70)) {
				_actor2.setup(901, 2, 1);
			} else {
				_actor2.setup(901, 2, 8);
			}
			_actor2.setPosition(Common::Point(171, 145));
			_actor2.setDetails(700, -1, -1, -1, 1, (SceneItem *) NULL);
		} else {
			_actor2.setDetails(700, -1, -1, -1, 1, (SceneItem *) NULL);
			if (R2_GLOBALS._v565E7 == 0) {
				_actor2.setup(901, 1, 8);
				// Original set two times the same values: skipped
				_actor2.setPosition(Common::Point((((100  - ((R2_GLOBALS._v565EB * 350) / 100)) * 49) / 100) + ((R2_GLOBALS._v565E9 * _actor3._percent * 6) / 100) + 89, 166 - (R2_GLOBALS._v565EB / 3)));
				_actor2.changeZoom(((100 - ((R2_GLOBALS._v565EB * 350) / 100) + 52) / 10) * 10);
			}
		}
	}
	_item1.setDetails(Rect(0, 0, 320, 200), 900, 0, -1, -1, 1, NULL);
	_sceneMode = 900;
	setAction(&_sequenceManager1, this, 900, &_actor1, NULL);
}

void Scene900::remove() {
	if (_sceneMode != 901)
		R2_GLOBALS._sound1.play(10);

	SceneExt::remove();
}

void Scene900::signal() {
	switch (_sceneMode) {
	case 1:
		_field412 = 1;
		R2_GLOBALS._sound2.play(37);

		_actor5.remove();
		_actor6.remove();
		_actor7.remove();
		_actor8.remove();
		_actor9.remove();
		_actor10.remove();

		_actor5.sub96135(2);
		_actor5.setup(900, 2, 1);
		_actor5.setPosition(Common::Point(36, 166));

		_actor6.sub96135(3);
		_actor6.setup(900, 2, 5);
		_actor6.setPosition(Common::Point(117, 166));
		break;
	case 2:
		_field412 = 2;

		_actor5.remove();
		_actor6.remove();

		_actor5.sub96135(2);
		if (R2_GLOBALS._v565E5 == 0)
			_actor5.setup(900, 3, 9);
		else
			_actor5.setup(900, 3, 11);
		_actor5.setPosition(Common::Point(36, 166));

		_actor7.sub96135(5);
		_actor7.setup(900, 3, 3);
		_actor7.setPosition(Common::Point(76, 134));

		_actor8.sub96135(4);
		_actor8.setup(900, 3, 7);
		_actor8.setPosition(Common::Point(76, 156));

		_actor9.sub96135(6);
		_actor9.setup(900, 3, 1);
		_actor9.setPosition(Common::Point(55, 144));

		_actor10.sub96135(7);
		_actor10.setup(900, 3, 5);
		_actor10.setPosition(Common::Point(99, 144));

		break;
	case 3:
		_field412 = 3;

		_actor5.remove();
		_actor6.remove();
		_actor7.remove();
		_actor8.remove();
		_actor9.remove();
		_actor10.remove();

		_actor5.sub96135(8);
		_actor5.setup(900, 4, 1);
		_actor5.setPosition(Common::Point(36, 166));

		_actor6.sub96135(9);
		_actor6.setup(900, 4, 5);
		_actor6.setPosition(Common::Point(117, 166));
		break;
	case 4:
		_sceneMode = 0;
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		R2_GLOBALS._v565E7 = 1;
		break;
	case 900:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;

		_actor1.setup(900, 1, 1);

		_actor4.sub96135(1);
		_actor4.setup(900, 1, 3);
		_actor4.setPosition(Common::Point(77, 168));

		_sceneMode = 1;
		signal();
		break;
	case 901:
		R2_GLOBALS._sceneManager.changeScene(700);
		break;
	case 5:
		_sceneMode = 0;
	// No break on purpose
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene900::dispatch() {
	if (_field416 != 0) {
		if (_field416 < 0) {
			R2_GLOBALS._v565E3--;
			++_field416;
		} else {
			++R2_GLOBALS._v565E3;
			_field416--;
		}
	}

	if (_field414 != 0) {
		R2_GLOBALS._v565E1--;
		++_field414;
	} else {
		++R2_GLOBALS._v565E1;
		_field414++;
	}

	if (R2_GLOBALS._sceneObjects->contains(&_actor2)) {
		if ((R2_GLOBALS._v565E5 != 0) && (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 0) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) == 700) && (R2_GLOBALS._v565E1 == 20) && (R2_GLOBALS._v565E3 == 70)) {
			if ((_actor2._frame > 1) && (_actor2._animateMode != ANIM_MODE_6))
				_actor2.animate(ANIM_MODE_6, NULL);
		} else {
			if ((_actor2._frame < 8) && (_actor2._animateMode != ANIM_MODE_5) && (R2_GLOBALS._v565E7 == 0) && (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 0) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) == 700) && (_sceneMode != 4))
				_actor2.animate(ANIM_MODE_5, NULL);
		}
	}

	_actor3.changeZoom(100 - ((R2_GLOBALS._v565E3 * 70) / 100));
	_actor3.setPosition(Common::Point(((_actor3._percent * R2_GLOBALS._v565E1 * 6) / 100) + 89, R2_GLOBALS._v565E3));

	if ((R2_GLOBALS._sceneObjects->contains(&_actor2)) && (R2_GLOBALS._v565E7 != 0) && (!_actor2._mover) && (_actor2._animateMode == ANIM_MODE_NONE)) {
		_actor2.setPosition(Common::Point(_actor3._position.x + ((_actor3._percent * 49) / 100), _actor3._position.y + ((_actor3._percent * 3) / 10)));
		if (R2_GLOBALS._v565E3 >= 75) {
			_actor2.setup(901, 1, 1);
			_actor2.changeZoom(((_actor3._percent + 52) / 10) * 10);
		} else {
			_actor2.setup(901, 5, 1);
			_actor2.changeZoom(((_actor3._percent / 10) * 10) + 30);
		}
	}
	Scene::dispatch();
}

} // End of namespace Ringworld2

} // End of namespace TsAGE
