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
#include "tsage/ringworld2/ringworld2_scenes2.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 2000 - Ice Maze
 *
 *--------------------------------------------------------------------------*/
void Scene2000::initPlayer() {
	R2_GLOBALS._player.disableControl();

	switch (_mazePlayerMode) {
	case 0:
		R2_GLOBALS._player.setStrip(5);
		if (_exit1._enabled) {
			if (_exit2._enabled)
				R2_GLOBALS._player.setPosition(Common::Point(140, 129));
			else
				R2_GLOBALS._player.setPosition(Common::Point(20, 129));
		} else
			R2_GLOBALS._player.setPosition(Common::Point(245, 129));
		R2_GLOBALS._player.enableControl();
		break;
	case 1:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2001;
		else
			_sceneMode = 2021;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 2:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2002;
		else
			_sceneMode = 2022;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 3:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2000;
		else
			_sceneMode = 2020;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 4:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2005;
		else
			_sceneMode = 2025;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 5:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2004;
		else
			_sceneMode = 2024;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 6:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2009;
		else
			_sceneMode = 2029;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 7:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2008;
		else
			_sceneMode = 2028;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 8:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2013;
		else
			_sceneMode = 2033;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 9:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2012;
		else
			_sceneMode = 2032;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 10:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2016;
		else
			_sceneMode = 2036;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 11:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2038;
		else
			_sceneMode = 2040;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
	for (int i = 0; i < 11; i++) {
		if (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] == R2_GLOBALS._v56605[3 + i])
			_objList1[i].show();
	}

	if ((R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) && (R2_GLOBALS._v56605[1] == R2_GLOBALS._v56605[2])) {
		_object1.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_object1.setup(20, 5, 1);
			_object1.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_object1.setup(2008, 5, 1);
			_object1.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		if (_exit1._enabled) {
			if (_exit2._enabled)
				_object1.setPosition(Common::Point(180, 128));
			else
				_object1.setPosition(Common::Point(75, 128));
		} else
			_object1.setPosition(Common::Point(300, 128));
	}
}

void Scene2000::initExits() {
	_exit1._enabled = true;
	_exit2._enabled = true;
	_exit3._enabled = false;
	_exit4._enabled = false;
	_exit5._enabled = false;

	_exit1._insideArea = false;
	_exit2._insideArea = false;
	_exit3._insideArea = false;
	_exit4._insideArea = false;
	_exit5._insideArea = false;

	_exit1._moving = false;
	_exit2._moving = false;
	_exit3._moving = false;
	_exit4._moving = false;
	_exit5._moving = false;

	for (int i = 0; i < 11; i++)
		_objList1[i].hide();

	_object1.remove();

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
	case 3:
	case 10:
	case 16:
	case 21:
		_exit5._enabled = true;
		_exit5._bounds.set(61, 68, 90, 125);
		_exit5.setDest(Common::Point(92, 129));
		_exit5._cursorNum = EXITCURSOR_W;
		break;
	case 4:
	case 12:
	case 25:
	case 34:
		_exit5._enabled = true;
		_exit5._bounds.set(230, 68, 259, 125);
		_exit5.setDest(Common::Point(244, 129));
		_exit5._cursorNum = EXITCURSOR_E;
		break;
	default:
		break;
	}

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] - 1) {
	case 0:
	case 6:
	case 13:
	case 18:
	case 22:
	case 27:
	case 30:
		_exit1._enabled = false;
		loadScene(2225);
		R2_GLOBALS._walkRegions.load(2225);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 1:
	case 19:
		_exit3._enabled = true;
		_exit3._bounds.set(71, 130, 154, 168);
		_exit3.setDest(Common::Point(94, 129));
		_exit3._cursorNum = EXITCURSOR_SE;
		loadScene(2300);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._walkRegions.load(2000);
		break;
	case 2:
	case 9:
	case 15:
	case 20:
		loadScene(2150);
		R2_GLOBALS._walkRegions.load(2000);
		switch(R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex]) {
		case 2400:
			_mazePlayerMode = 1;
			break;
		case 2425:
		case 2430:
		case 2435:
		case 2450:
			_mazePlayerMode = 3;
			break;
		default:
			if (!_exitingFlag)
				_mazePlayerMode = 0;
			break;
		}
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 3:
	case 11:
	case 24:
	case 33:
		loadScene(2175);
		R2_GLOBALS._walkRegions.load(2000);
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
			if (!_exitingFlag)
				_mazePlayerMode = 0;
		} else if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2350)
			_mazePlayerMode = 1;
		else
			_mazePlayerMode = 10;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 4:
	case 8:
		loadScene(2000);
		R2_GLOBALS._walkRegions.load(2000);
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1900)
			_mazePlayerMode = 1;
		else if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 5:
	case 12:
	case 17:
	case 21:
	case 26:
		loadScene(2200);
		R2_GLOBALS._walkRegions.load(2000);
		_exit2._enabled = false;
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1900)
			_mazePlayerMode = 2;
		else if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 7:
	case 29:
		_exit4._enabled = true;
		_exit4._bounds.set(138, 83, 211, 125);
		_exit4.setDest(Common::Point(129, 188));
		_exit4._cursorNum = EXITCURSOR_NW;
		loadScene(2250);
		R2_GLOBALS._walkRegions.load(2000);
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2500)
			_mazePlayerMode = 1;
		else if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 10:
	case 25:
		_exit3._enabled = true;
		_exit3._bounds.set(78, 130, 148, 168);
		_exit3.setDest(Common::Point(100, 129));
		_exit3._cursorNum = EXITCURSOR_SE;
		loadScene(2075);
		R2_GLOBALS._walkRegions.load(2000);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 14:
		_exit3._enabled = true;
		_exit3._bounds.set(160, 130, 248, 168);
		_exit3.setDest(Common::Point(225, 129));
		_exit3._cursorNum = EXITCURSOR_SW;
		loadScene(2325);
		R2_GLOBALS._walkRegions.load(2000);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 16:
	case 31:
		_exit4._enabled = true;
		_exit4._bounds.set(122, 83, 207, 125);
		_exit4.setDest(Common::Point(210, 129));
		_exit4._cursorNum = EXITCURSOR_NW;
		loadScene(2125);
		R2_GLOBALS._walkRegions.load(2000);
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2400)
			_mazePlayerMode = 2;
		else if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 23:
		_exit4._enabled = true;
		_exit4._bounds.set(108, 83, 128, 184);
		_exit4.setDest(Common::Point(135, 129));
		_exit4._cursorNum = CURSOR_INVALID;
		loadScene(2275);
		R2_GLOBALS._walkRegions.load(2000);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 28:
		_exit3._enabled = true;
		_exit3._bounds.set(171, 130, 241, 168);
		_exit3.setDest(Common::Point(218, 129));
		_exit3._cursorNum = EXITCURSOR_SW;
		loadScene(2050);
		R2_GLOBALS._walkRegions.load(2000);
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2350)
			_mazePlayerMode = 11;
		else if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 32:
		loadScene(2025);
		R2_GLOBALS._walkRegions.load(2000);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	default:
		break;
	}
	_exitingFlag = false;
	R2_GLOBALS._uiElements.show();
}

void Scene2000::Action1::signal() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 0: {
		_actionIndex = 1;
		Common::Point pt(-20, 127);
		NpcMover *mover = new NpcMover();
		scene->_objList1[_state].addMover(mover, &pt, scene);
		break;
		}
	case 1:
		scene->_objList1[_state].setPosition(Common::Point(340, 127));
		--R2_GLOBALS._v56605[4 + _state];
		_actionIndex = 0;
		switch (_state - 1) {
		case 0:
			if (R2_GLOBALS._v56605[4] == 1)
				_actionIndex = 10;
			break;
		case 2:
			if (R2_GLOBALS._v56605[6] == 7)
				_actionIndex = 10;
			break;
		case 4:
			if (R2_GLOBALS._v56605[8] == 14)
				_actionIndex = 10;
			break;
		case 6:
			if (R2_GLOBALS._v56605[10] == 19)
				_actionIndex = 10;
			break;
		case 7:
			if (R2_GLOBALS._v56605[11] == 23)
				_actionIndex = 10;
			break;
		default:
			break;
		}

		if (R2_GLOBALS._v56605[3 + _state] == R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex])
			scene->_objList1[_state].show();
		else
			scene->_objList1[_state].hide();

		signal();
		break;
	case 5: {
		_actionIndex = 6;
		Common::Point pt(340, 127);
		NpcMover *mover = new NpcMover();
		scene->_objList1[_state].addMover(mover, &pt, this);
		break;
		}
	case 6:
		scene->_objList1[_state].setPosition(Common::Point(-20, 127));
		++R2_GLOBALS._v56605[3 + _state];
		_actionIndex = 5;
		switch (_state - 1) {
		case 0:
			if (R2_GLOBALS._v56605[4] == 5)
				_actionIndex = 15;
			break;
		case 2:
			if (R2_GLOBALS._v56605[6] == 13)
				_actionIndex = 15;
			break;
		case 4:
			if (R2_GLOBALS._v56605[8] == 16)
				_actionIndex = 15;
			break;
		case 6:
			if (R2_GLOBALS._v56605[10] == 22)
				_actionIndex = 15;
			break;
		case 7:
			if (R2_GLOBALS._v56605[11] == 27)
				_actionIndex = 15;
			break;
		default:
			break;
		}

		if (R2_GLOBALS._v56605[3 + _state] == R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex])
			scene->_objList1[_state].show();
		else
			scene->_objList1[_state].hide();

		signal();
		break;
	case 10: {
		Common::Point pt(290, 127);
		NpcMover *mover = new NpcMover();
		scene->_objList1[_state].addMover(mover, &pt, this);
		_actionIndex = 11;
		break;
		}
	case 11:
		if (_state == 1)
			scene->_objList1[0].setStrip(1);
		else if (_state == 5)
			scene->_objList1[4].setStrip(1);
		setDelay(600);
		_actionIndex = 12;
		break;
	case 12:
		if (_state == 1)
			scene->_objList1[0].setStrip(2);
		else if (_state == 5)
			scene->_objList1[4].setStrip(2);
		scene->_objList1[_state].setStrip(1);
		_actionIndex = 5;
		signal();
		break;
	case 15:
		if ((R2_GLOBALS._v56605[3 + _state] == 13) || (R2_GLOBALS._v56605[3 + _state] == 22) || (R2_GLOBALS._v56605[3 + _state] == 27)) {
			Common::Point pt(30, 127);
			NpcMover *mover = new NpcMover();
			scene->_objList1[_state].addMover(mover, &pt, this);
			_actionIndex = 16;
		} else {
			Common::Point pt(120, 127);
			NpcMover *mover = new NpcMover();
			scene->_objList1[_state].addMover(mover, &pt, this);
			_actionIndex = 16;
		}
		break;
	case 16:
		if (_state == 1)
			scene->_objList1[2].setStrip(2);
		else if (_state == 8)
			scene->_objList1[9].setStrip(2);
		setDelay(600);
		_actionIndex = 17;
		break;
	case 17:
		if (_state == 1)
			scene->_objList1[2].setStrip(1);
		else if (_state == 8)
			scene->_objList1[9].setStrip(1);
		scene->_objList1[_state].setStrip(2);
		_actionIndex = 0;
		break;
	case 99:
		error("99");
		break;
	default:
		break;
	}
}

void Scene2000::Exit1::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	scene->_exitingFlag = true;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 10;

	Common::Point pt(-10, 129);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);

	scene->setAction(&scene->_sequenceManager, scene, 206, &R2_GLOBALS._player, NULL);
}

void Scene2000::Exit2::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	scene->_exitingFlag = true;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 11;

	Common::Point pt(330, 129);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2000::Exit3::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	scene->_exitingFlag = true;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 12;

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
	case 2:
		scene->_mazePlayerMode = 4;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 8;
		break;
	case 11:
		scene->_mazePlayerMode = 6;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 17;
		break;
	case 15:
		scene->_mazePlayerMode = 8;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 24;
		break;
	case 20:
		scene->_mazePlayerMode = 4;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 30;
		break;
	case 26:
		scene->_mazePlayerMode = 6;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 32;
		break;
	case 29:
		scene->_mazePlayerMode = 11;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 29;
		break;
	default:
		break;
	}

	switch (scene->_mazePlayerMode) {
	case 4:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2003, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2023, &R2_GLOBALS._player, NULL);
		break;
	case 6:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2007, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2027, &R2_GLOBALS._player, NULL);
		break;
	case 8:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2011, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2031, &R2_GLOBALS._player, NULL);
		break;
	case 11:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->_sceneMode = 2039;
		else
			scene->_sceneMode = 2041;
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
		break;

	default:
		break;
	}
}
void Scene2000::Exit4::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	scene->_exitingFlag = true;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 13;

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
	case 8:
		scene->_mazePlayerMode = 5;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 2;
		break;
	case 17:
		scene->_mazePlayerMode = 7;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 11;
		break;
	case 24:
		scene->_mazePlayerMode = 9;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 15;
		break;
	case 30:
		scene->_mazePlayerMode = 5;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 20;
		break;
	case 32:
		scene->_mazePlayerMode = 7;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 26;
		break;
	default:
		break;
	}

	switch (scene->_mazePlayerMode) {
	case 5:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2006, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2026, &R2_GLOBALS._player, NULL);
		break;
	case 7:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2010, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2030, &R2_GLOBALS._player, NULL);
		break;
	case 9:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2014, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2034, &R2_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
}

void Scene2000::Exit5::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 14;

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
	case 3:
		scene->_mazePlayerMode = 1;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 4:
		scene->_mazePlayerMode = 7;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 10:
		scene->_mazePlayerMode = 8;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 12:
		scene->_mazePlayerMode = 3;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 16:
		scene->_mazePlayerMode = 4;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 21:
		scene->_mazePlayerMode = 5;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 25:
		scene->_mazePlayerMode = 2;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 34:
		scene->_mazePlayerMode = 6;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
}

void Scene2000::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(2000);

	if (R2_GLOBALS._sceneManager._previousScene != -1) {
		R2_GLOBALS._v56605[1] = 21;
		R2_GLOBALS._v56605[2] = 21;
	}
	if ((R2_GLOBALS._player._characterScene[R2_GLOBALS._player._characterIndex] != R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex]) && (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] != 2350)) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 0;
	}
	_exitingFlag = false;

	_exit1.setDetails(Rect(0, 100, 14, 140), EXITCURSOR_W, 2000);
	_exit1.setDest(Common::Point(14, 129));
	_exit2.setDetails(Rect(305, 100, 320, 140), EXITCURSOR_E, 2000);
	_exit2.setDest(Common::Point(315, 129));
	_exit3.setDetails(Rect(71, 130, 154, 168), EXITCURSOR_S, 2000);
	_exit3.setDest(Common::Point(94, 129));
	_exit4.setDetails(Rect(138, 83, 211, 125), EXITCURSOR_N, 2000);
	_exit4.setDest(Common::Point(188, 128));
	_exit5.setDetails(Rect(61, 68, 90, 125), EXITCURSOR_W, 2000);
	_exit5.setDest(Common::Point(92, 129));

	R2_GLOBALS._sound1.play(200);
	initExits();
	g_globals->_sceneManager._fadeMode = FADEMODE_IMMEDIATE;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.setup(2008, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setup(20, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}

	_action1._state = 8;
	_action2._state = 1;
	_action3._state = 5;
	_action4._state = 7;
	_action5._state = 3;

	for (int i = 0; i < 11; i++)
		_objList1[i].postInit();

	_objList1[0].setVisage(2000);
	_objList1[0].setStrip(2);
	_objList1[0].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_objList1[1].setVisage(2001);
	_objList1[1].setStrip(2);
	_objList1[1].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_objList1[2].setVisage(2003);
	_objList1[2].setStrip(1);
	_objList1[2].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_objList1[3].setVisage(2007);
	_objList1[3].setStrip(2);
	_objList1[3].setDetails(2001, 12, -1, -1, 1, (SceneItem *)NULL);

	_objList1[4].setVisage(2004);
	_objList1[4].setStrip(2);
	_objList1[4].setDetails(2001, 19, -1, -1, 1, (SceneItem *)NULL);

	_objList1[5].setVisage(2003);
	_objList1[5].setStrip(2);
	_objList1[5].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_objList1[6].setVisage(2000);
	_objList1[6].setStrip(1);
	_objList1[6].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_objList1[7].setVisage(2000);
	_objList1[7].setStrip(2);
	_objList1[7].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_objList1[8].setVisage(2000);
	_objList1[8].setStrip(2);
	_objList1[8].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_objList1[9].setVisage(2006);
	_objList1[9].setStrip(1);
	_objList1[9].setDetails(2001, 6, -1, -1, 1, (SceneItem *)NULL);

	_objList1[10].setVisage(2007);
	_objList1[10].setStrip(1);
	_objList1[10].setDetails(2001, 12, -1, -1, 1, (SceneItem *)NULL);

	for (int i = 0; i < 11; i++) {
		_objList1[i].animate(ANIM_MODE_1, NULL);
		_objList1[i]._moveDiff.x = 3;
		_objList1[i]._moveRate = 8;
		_objList1[i].hide();
		switch (i - 1) {
		case 0:
			if (R2_GLOBALS._v56605[3 + i]  == 1)
				++R2_GLOBALS._v56605[3 + i];
			else if (R2_GLOBALS._v56605[3 + i]  == 5)
				--R2_GLOBALS._v56605[3 + i];
			break;
		case 2:
			if (R2_GLOBALS._v56605[3 + i]  == 7)
				++R2_GLOBALS._v56605[3 + i];
			else if (R2_GLOBALS._v56605[3 + i]  == 13)
				--R2_GLOBALS._v56605[3 + i];
			break;
		case 4:
			if (R2_GLOBALS._v56605[3 + i]  == 14)
				++R2_GLOBALS._v56605[3 + i];
			else if (R2_GLOBALS._v56605[3 + i]  == 16)
				--R2_GLOBALS._v56605[3 + i];
			break;
		case 6:
			if (R2_GLOBALS._v56605[3 + i]  == 19)
				++R2_GLOBALS._v56605[3 + i];
			else if (R2_GLOBALS._v56605[3 + i]  == 22)
				--R2_GLOBALS._v56605[3 + i];
			break;
		case 8:
			if (R2_GLOBALS._v56605[3 + i]  == 23)
				++R2_GLOBALS._v56605[3 + i];
			else if (R2_GLOBALS._v56605[3 + i]  == 27)
				--R2_GLOBALS._v56605[3 + i];
			break;
		default:
			break;
		}
		switch (R2_GLOBALS._v56605[3 + i] - 1) {
		case 0:
		case 6:
		case 13:
		case 18:
		case 22:
		case 27:
		case 30:
			_objList1[i].setPosition(Common::Point(265, 127));
			break;
		case 5:
		case 12:
		case 17:
		case 21:
		case 26:
			_objList1[i].setPosition(Common::Point(55, 127));
			break;
		default:
			_objList1[i].setPosition(Common::Point(160, 127));
			break;
		}
	}
	_objList1[1].setAction(&_action2);
	_objList1[3].setAction(&_action5);
	_objList1[5].setAction(&_action4);
	_objList1[8].setAction(&_action1);

	initPlayer();

	_item1.setDetails(Rect(0, 0, 320, 200), 2000, 0, -1, 23, 1, NULL);
}

void Scene2000::remove() {
	R2_GLOBALS._sound1.fadeOut(NULL);
	SceneExt::remove();
}

void Scene2000::signal() {
	switch (_sceneMode) {
	case 10:
		if (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] == 6)
			g_globals->_sceneManager.changeScene(1900);
		else {
			_mazePlayerMode = 1;
			--R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex];
			initExits();
			initPlayer();
		}
	break;
	case 11:
		switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
		case 5:
			g_globals->_sceneManager.changeScene(1900);
			break;
		case 30:
			g_globals->_sceneManager.changeScene(2500);
			break;
		case 34:
			g_globals->_sceneManager.changeScene(2350);
			break;
		default:
			_mazePlayerMode = 2;
			++R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex];
			initExits();
			initPlayer();
			break;
		}
		break;
	case 12:
	case 13:
		initExits();
		initPlayer();
		break;
	case 14:
		switch (_mazePlayerMode - 1) {
		case 0:
			g_globals->_sceneManager.changeScene(2450);
			break;
		case 1:
			g_globals->_sceneManager.changeScene(2440);
			break;
		case 2:
			g_globals->_sceneManager.changeScene(2435);
			break;
		case 3:
			g_globals->_sceneManager.changeScene(2430);
			break;
		case 4:
			g_globals->_sceneManager.changeScene(2425);
			break;
		case 5:
			g_globals->_sceneManager.changeScene(2525);
			break;
		case 6:
			g_globals->_sceneManager.changeScene(2530);
			break;
		case 7:
			g_globals->_sceneManager.changeScene(2535);
			break;
		default:
			if (R2_GLOBALS._v56AAB != 0)
				R2_GLOBALS._v56AAB = 0;
			R2_GLOBALS._player.enableControl(CURSOR_ARROW);
			break;
		}
		break;
	case 2039:
	case 2041:
		g_globals->_sceneManager.changeScene(2350);
		break;
	default:
		break;
	}
}

void Scene2000::process(Event &event) {
	if ((R2_GLOBALS._player._canWalk) && (event.eventType == EVENT_BUTTON_DOWN) &&
			(R2_GLOBALS._events.getCursor() == CURSOR_CROSSHAIRS)) {

		Common::Point pt(event.mousePos.x, 129);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);

		event.handled = true;
	}
	Scene::process(event);
}

void Scene2000::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsByte(_exitingFlag);
	s.syncAsSint16LE(_mazePlayerMode);
}

/*--------------------------------------------------------------------------
 * Scene 2350 - Balloon Launch Platform
 *
 *--------------------------------------------------------------------------*/
bool Scene2350::Actor2::startAction(CursorType action, Event &event) {
	if (action != R2_SENSOR_PROBE)
		return(SceneActor::startAction(action, event));
	return true;
}

bool Scene2350::Actor3::startAction(CursorType action, Event &event) {
	Scene2350 *scene = (Scene2350 *)R2_GLOBALS._sceneManager._scene;

	if ((action == R2_REBREATHER_TANK) && (R2_GLOBALS.getFlag(74))) {
		R2_GLOBALS._player.disableControl();
		scene->_actor1.postInit();
		scene->_sceneMode = 2355;
		scene->setAction(&scene->_sequenceManager, scene, 2355, &R2_GLOBALS._player, &scene->_actor1, NULL);
		return true;
	}

	return(SceneActor::startAction(action, event));
}

void Scene2350::ExitUp::changeScene() {
	Scene2350 *scene = (Scene2350 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl(CURSOR_CROSSHAIRS);
	scene->_sceneMode = 12;
	if (R2_GLOBALS._player._characterIndex == 1)
		scene->setAction(&scene->_sequenceManager, scene, 2350, &R2_GLOBALS._player, NULL);
	else
		scene->setAction(&scene->_sequenceManager, scene, 2352, &R2_GLOBALS._player, NULL);
}

void Scene2350::ExitWest::changeScene() {
	Scene2350 *scene = (Scene2350 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl(CURSOR_CROSSHAIRS);
	scene->_sceneMode = 11;

	Common::Point pt(-10, 129);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);

}

void Scene2350::postInit(SceneObjectList *OwnerList) {
	loadScene(2350);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(200);
	_stripManager.addSpeaker(&_pharishaSpeaker);
	_stripManager.addSpeaker(&_quinnSpeaker);

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._player._characterScene[2] = 2350;

	_exitUp.setDetails(Rect(25, 83, 93, 125), EXITCURSOR_NW, 2350);
	_exitUp.setDest(Common::Point(80, 129));
	_exitWest.setDetails(Rect(0, 100, 14, 140), EXITCURSOR_W, 2350);
	_exitWest.setDest(Common::Point(14, 129));

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.setup(2008, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setup(20, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}

	if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
		_actor2.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_actor2.setup(20, 5, 1);
			_actor2.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_actor2.setup(2008, 5, 1);
			_actor2.setDetails(9001, 0, 5, 3, 1,  (SceneItem *)NULL);
		}
		_actor2.setPosition(Common::Point(135, 128));
	}
	_actor3.postInit();
	_actor4.postInit();

	if (R2_INVENTORY.getObjectScene(20) == 2350) {
		_actor3.hide();
		_actor4.hide();
	} else {
		_actor3.setup(2350, 0, 1);
		_actor3.setPosition(Common::Point(197, 101));
		_actor3.setDetails(2000, 12, -1, -1, 1, (SceneItem *)NULL);
		_actor3.fixPriority(10);
		_actor4.setup(2350, 1, 2);
		_actor4.setPosition(Common::Point(199, 129));
		_actor4.setDetails(2000, 12, -1, -1, 1, (SceneItem *)NULL);
		_actor4.fixPriority(10);
	}
	_item1.setDetails(Rect(0, 0, 320, 200), 2000, 9, -1, -1, 1, NULL);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		if (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] == 34) {
			if (R2_GLOBALS._player._characterIndex == 1)
				_sceneMode = 2351;
			else
				_sceneMode = 2353;
			setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		} else {
			_sceneMode = 10;
			R2_GLOBALS._player.setPosition(Common::Point(-20, 129));
			Common::Point pt(20, 129);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);

		}
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(100, 129));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
	R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2350;
}

void Scene2350::remove() {
	R2_GLOBALS._sound1.fadeOut(NULL);
	SceneExt::remove();
}

void Scene2350::signal() {
	switch (_sceneMode) {
	case 11:
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 34;
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 12:
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 29;
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 20:
		_sceneMode = 21;
		_stripManager.start(712, this);
		break;
	case 21:
		R2_GLOBALS._player.disableControl();
		R2_INVENTORY.setObjectScene(36, 1);
		_sceneMode = 2354;
		setAction(&_sequenceManager, this, 2354, &R2_GLOBALS._player, NULL);
		break;
	case 2354:
		R2_INVENTORY.setObjectScene(20, 2350);
		g_globals->_sceneManager.changeScene(2900);
		break;
	case 2355:
		_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(711, this);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene2350::process(Event &event) {
	if ((R2_GLOBALS._player._canWalk) && (event.eventType != EVENT_BUTTON_DOWN) &&
			(R2_GLOBALS._events.getCursor() == CURSOR_CROSSHAIRS)){
		Common::Point pt(event.mousePos.x, 129);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt);
		event.handled = true;
	}
	Scene::process(event);
}

/*--------------------------------------------------------------------------
 * Scene 2400 - Ice Maze: Large empty room
 *
 *--------------------------------------------------------------------------*/
void Scene2400::Exit1::changeScene() {
	Scene2400 *scene = (Scene2400 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 10;

	Common::Point pt(-10, 150);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);

}

void Scene2400::Exit2::changeScene() {
	Scene2400 *scene = (Scene2400 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(330, 150);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2400::postInit(SceneObjectList *OwnerList) {
	loadScene(2400);
	SceneExt::postInit();
	_exit1.setDetails(Rect(0, 125, 14, 165), EXITCURSOR_W, 2000);
	_exit1.setDest(Common::Point(14, 150));
	_exit2.setDetails(Rect(305, 125, 320, 165), EXITCURSOR_E, 2000);
	_exit2.setDest(Common::Point(315, 150));
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._v56605[1] == 16) {
		_sceneMode = 2400;
		setAction(&_sequenceManager, this, 2400, &R2_GLOBALS._player, NULL);
	} else {
		_sceneMode = 2401;
		setAction(&_sequenceManager, this, 2401, &R2_GLOBALS._player, NULL);
	}
}

void Scene2400::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._v56605[1] = 16;
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 11:
		R2_GLOBALS._v56605[1] = 17;
		g_globals->_sceneManager.changeScene(2000);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2425 - Ice Maze:
 *
 *--------------------------------------------------------------------------*/

bool Scene2425::Item1::startAction(CursorType action, Event &event) {
	Scene2425 *scene = (Scene2425 *)R2_GLOBALS._sceneManager._scene;

	if ((action == R2_GUNPOWDER) && (!R2_GLOBALS.getFlag(84))) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2426;
		scene->setAction(&scene->_sequenceManager, scene, 2426, &R2_GLOBALS._player, &scene->_actor1, NULL);
		R2_GLOBALS.setFlag(84);
		return true;
	} else if (action == R2_GUNPOWDER) {
		R2_GLOBALS._events.setCursor(R2_STEPPING_DISKS);
		R2_GLOBALS._player.enableControl(R2_STEPPING_DISKS);
		return NamedHotspot::startAction(R2_STEPPING_DISKS, event);
	} else
		return NamedHotspot::startAction(action, event);
}

bool Scene2425::Item2::startAction(CursorType action, Event &event) {
	Scene2425 *scene = (Scene2425 *)R2_GLOBALS._sceneManager._scene;

	if ((action == R2_GUNPOWDER) && (R2_GLOBALS.getFlag(84))) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2427;
		scene->setAction(&scene->_sequenceManager, scene, 2427, &R2_GLOBALS._player, &scene->_actor1, NULL);
		R2_GLOBALS.clearFlag(84);
		return true;
	} else if (action == R2_GUNPOWDER) {
		R2_GLOBALS._events.setCursor(R2_STEPPING_DISKS);
		R2_GLOBALS._player.enableControl(R2_STEPPING_DISKS);
		return NamedHotspot::startAction(R2_STEPPING_DISKS, event);
	} else
		return NamedHotspot::startAction(action, event);
}

bool Scene2425::Item3::startAction(CursorType action, Event &event) {
	Scene2425 *scene = (Scene2425 *)R2_GLOBALS._sceneManager._scene;

	if (action != R2_GUNPOWDER)
		return NamedHotspot::startAction(action, event);
	else {
		R2_GLOBALS._player.disableControl();
		if (R2_GLOBALS.getFlag(84)) {
			scene->_sceneMode = 20;
			scene->setAction(&scene->_sequenceManager, scene, 2427, &R2_GLOBALS._player, &scene->_actor1, NULL);
			R2_GLOBALS.clearFlag(84);
		} else {
			scene->_sceneMode = 2425;
			scene->setAction(&scene->_sequenceManager, scene, 2425, &R2_GLOBALS._player, &scene->_actor1, NULL);
		}
		return true;
	}
}

bool Scene2425::Item4::startAction(CursorType action, Event &event) {
	if (action != R2_GUNPOWDER)
		return NamedHotspot::startAction(action, event);
	else {
		R2_GLOBALS._events.setCursor(R2_STEPPING_DISKS);
		R2_GLOBALS._player.enableControl(R2_STEPPING_DISKS);
		return NamedHotspot::startAction(R2_STEPPING_DISKS, event);
	}
}

bool Scene2425::Actor1::startAction(CursorType action, Event &event) {
	if (action == R2_STEPPING_DISKS) {
		if (R2_GLOBALS._player._characterIndex == 2) {
			R2_GLOBALS._events.setCursor(R2_GUNPOWDER);
			return true;
		} else {
			return SceneActor::startAction(action, event);
		}
	} else if (R2_GLOBALS._events.getCursor() == R2_GUNPOWDER)
		return false;
	else
		return SceneActor::startAction(action, event);
}

bool Scene2425::Actor2::startAction(CursorType action, Event &event) {
	if (action != R2_GUNPOWDER)
		return SceneActor::startAction(action, event);
	else {
		R2_GLOBALS._events.setCursor(R2_STEPPING_DISKS);
		R2_GLOBALS._player.enableControl(R2_STEPPING_DISKS);
		return SceneActor::startAction(R2_STEPPING_DISKS, event);
	}
}

void Scene2425::Exit1::changeScene() {
	Scene2425 *scene = (Scene2425 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(R2_NEGATOR_GUN);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(340, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}


void Scene2425::postInit(SceneObjectList *OwnerList) {
	loadScene(2425);
	SceneExt::postInit();
	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._player._characterIndex = R2_SEEKER;
		R2_GLOBALS._sceneManager._previousScene = 2000;
	}

	R2_GLOBALS._sound1.play(200);
	_exit1.setDetails(Rect(270, 136, 319, 168), EXITCURSOR_SE, 2000);
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}

	if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
		_actor2.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_actor2.setup(20, 5, 1);
			_actor2.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_actor2.setup(2008, 5, 1);
			_actor2.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_actor2.setPosition(Common::Point(250, 185));
	}

	_actor1.postInit();
	if (R2_GLOBALS._sceneManager._previousScene == 2455)
		_actor1.setup(2426, 1, 1);
	else
		_actor1.setup(2426, 1, 2);

	_actor1.setPosition(Common::Point(290, 9));
	_actor1.fixPriority(20);
	_actor1.setDetails(2455, 12, -1, -1, 1, (SceneItem *)NULL);
	_item1.setDetails(Rect(225, 52, 248, 65), 2425, -1, -1, -1, 1, NULL);
	_item2.setDetails(Rect(292, 81, 316, 94), 2425, -1, -1, -1, 1, NULL);

//  CHECKME: SceneActor using a SceneItem function??
//	_actor3.setDetails(11, 2425, 3, -1, 6);
	_actor3._sceneRegionId = 11;
	_actor3._resNum = 2425;
	_actor3._lookLineNum = 3;
	_actor3._talkLineNum = -1;
	_actor3._useLineNum = 6;
	g_globals->_sceneItems.push_back(&_actor3);

	_item3.setDetails(12, 2425, 7, -1, 9);
	_item4.setDetails(Rect(0, 0, 320, 200), 2425, 0, -1, -1, 1, NULL);

	R2_GLOBALS._player.disableControl();
	switch (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex]) {
	case 2000: {
		_sceneMode = 10;
		R2_GLOBALS._player.setPosition(Common::Point(340, 200));

		Common::Point pt(280, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 2425:
		_sceneMode = 10;
		R2_GLOBALS._player.setPosition(Common::Point(280, 150));
		_action->signal();
		break;
	case 2455:
		_sceneMode = 2428;
		setAction(&_sequenceManager, this, 2428, &R2_GLOBALS._player, &_actor1, NULL);
		break;
	default:
		R2_GLOBALS._player.setPosition(Common::Point(280, 150));
		R2_GLOBALS._player.setStrip(8);
		R2_GLOBALS._player.enableControl();
		break;
	}
	R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2425;
}

void Scene2425::remove() {
	R2_GLOBALS._sound1.fadeOut(NULL);
	SceneExt::remove();
}

void Scene2425::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 20:
		_sceneMode = 2425;
		setAction(&_sequenceManager, this, 2425, &R2_GLOBALS._player, &_actor1, NULL);
		break;
	case 2425:
		g_globals->_sceneManager.changeScene(2455);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2430 - Ice Maze: Bedroom
 *
 *--------------------------------------------------------------------------*/

bool Scene2430::Actor1::startAction(CursorType action, Event &event) {
	return SceneActor::startAction(action, event);
}

bool Scene2430::Actor2::startAction(CursorType action, Event &event) {
	Scene2430 *scene = (Scene2430 *)R2_GLOBALS._sceneManager._scene;

	if ((action != R2_STEPPING_DISKS) || (R2_GLOBALS._player._characterIndex != 2))
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 2430;
	scene->setAction(&scene->_sequenceManager, scene, 2430, &R2_GLOBALS._player, &scene->_actor2, NULL);
	return true;
}

bool Scene2430::Actor3::startAction(CursorType action, Event &event) {
	Scene2430 *scene = (Scene2430 *)R2_GLOBALS._sceneManager._scene;

	if ((action != R2_STEPPING_DISKS) || (R2_GLOBALS._player._characterIndex != 2))
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 2435;
	scene->setAction(&scene->_sequenceManager, scene, 2435, &R2_GLOBALS._player, &scene->_actor3, NULL);
	return true;
}

void Scene2430::Exit1::changeScene() {
	Scene2430 *scene = (Scene2430 *)R2_GLOBALS._sceneManager._scene;

	scene->_sceneMode = 0;
	R2_GLOBALS._events.setCursor(R2_NEGATOR_GUN);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;
	Common::Point pt(108, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2430::postInit(SceneObjectList *OwnerList) {
	loadScene(2430);
	SceneExt::postInit();
	_exit1.setDetails(Rect(68, 155, 147, 168), EXITCURSOR_S, 2000);
	_exit1.setDest(Common::Point(108, 160));

	if (R2_INVENTORY.getObjectScene(37) == 2430) {
		_actor2.postInit();
		_actor2.setup(2435, 1, 5);
		_actor2.setPosition(Common::Point(205, 119));
		_actor2.fixPriority(152);
		_actor2.setDetails(2430, 51, -1, 53, 1, (SceneItem *)NULL);
	}

	if (R2_INVENTORY.getObjectScene(50) == 2435) {
		_actor3.postInit();
		_actor3.setup(2435, 1, 1);
		_actor3.setPosition(Common::Point(31, 65));
		_actor3.setDetails(2430, 48, -1, -1, 1, (SceneItem *)NULL);
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}
	R2_GLOBALS._player.setPosition(Common::Point(100, 200));

	if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
		_actor1.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_actor1.setup(20, 5, 1);
			_actor1.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_actor1.setup(2008, 5, 1);
			_actor1.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_actor1.setPosition(Common::Point(189, 137));
		R2_GLOBALS._walkRegions.enableRegion(4);
	}

	_item2.setDetails(Rect(11, 30, 37, 45), 2430, 3, -1, 5, 1, NULL);
	_item3.setDetails(Rect(9, 58, 63, 92), 2430, 6, -1, -1, 1, NULL);
	_item4.setDetails(Rect(20, 89, 127, 107), 2430,  9, -1, 11, 1, NULL);
	_item5.setDetails(Rect(49, 7, 60, 27), 2430, 12, 13, 14, 1, NULL);
	_item6.setDetails(Rect(69, 10, 95, 72), 2430, 15, -1, 14, 1, NULL);
	_item10.setDetails(Rect(198, 4, 222, 146), 2430, 30, 31, 32, 1, NULL);
	_item7.setDetails(Rect(155, 40, 304, 120), 2430, 21, -1, 23, 1, NULL);
	_item8.setDetails(Rect(249, 3, 261, 39), 2430, 24, 25, -1, 1, NULL);
	_item9.setDetails(Rect(279, 13, 305, 34), 2430, 33, -1, 18, 1, NULL);
	// CHECKME: initialized for the 2nd time??
	_item2.setDetails(Rect(11, 30, 37, 45), 2430, 33, -1, 18, 1, NULL);
	_item11.setDetails(Rect(116, 104, 148, 111), 2430, 39, -1, -1, 1, NULL);
	_item12.setDetails(Rect(66, 77, 84, 83), 2430, 39, -1, -1, 1, NULL);
	_item13.setDetails(Rect(117, 118, 201, 141), 2430, 9, -1, 11, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 2430, 0, -1, -1, 1, NULL);

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2430;
		Common::Point pt(108, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(105, 145));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}

void Scene2430::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 2430:
		_actor2.remove();
		R2_INVENTORY.setObjectScene(R2_GUNPOWDER, 2);
		R2_GLOBALS._player.enableControl();
		break;
	case 2435:
		_actor3.remove();
		R2_INVENTORY.setObjectScene(R2_ALCOHOL_LAMP_3, 2);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2435 - Ice Maze: Throne room
 *
 *--------------------------------------------------------------------------*/
bool Scene2435::Actor1::startAction(CursorType action, Event &event) {
	return SceneActor::startAction(action, event);
}

bool Scene2435::Actor2::startAction(CursorType action, Event &event) {
	Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_SAPPHIRE_BLUE:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		R2_GLOBALS.setFlag(82);
		scene->_stripManager.start(603, scene);
		return true;
	case R2_ANCIENT_SCROLLS:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		R2_GLOBALS.setFlag(82);
		scene->_stripManager.start(602, scene);
		R2_INVENTORY.setObjectScene(R2_ANCIENT_SCROLLS, 2000);
		return true;
	case CURSOR_TALK:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		if ((R2_GLOBALS._player._characterIndex == 1) || (R2_GLOBALS.getFlag(82))) {
			scene->_stripManager.start(605, scene);
			return true;
		} else if (R2_INVENTORY.getObjectScene(R2_ANCIENT_SCROLLS) == 2) {
			scene->_stripManager.start(601, scene);
			return true;
		} else {
			R2_GLOBALS.setFlag(82);
			scene->_stripManager.start(600, scene);
			return true;
		}
	default:
		return SceneActor::startAction(action, event);
	}
}

void Scene2435::Exit1::changeScene() {
	Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(R2_NEGATOR_GUN);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;
	Common::Point pt(175, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);

}

void Scene2435::postInit(SceneObjectList *OwnerList) {
	loadScene(2435);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(201);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_pharishaSpeaker);
	_exit1.setDetails(Rect(142, 155, 207, 167), EXITCURSOR_S, 2000);
	_exit1.setDest(Common::Point(175, 160));
	_actor2.postInit();
	_actor2.setup(2005, 3, 1);
	_actor2.setPosition(Common::Point(219, 106));
	_actor2.setDetails(2001, 25, 26, -1, 1, (SceneItem *)NULL);
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}
	R2_GLOBALS._player.setPosition(Common::Point(715, 200));
	if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
		_actor1.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_actor1.setup(20, 5, 1);
			_actor1.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_actor1.setup(2008, 5, 1);
			_actor1.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_actor1.setPosition(Common::Point(107, 145));
		R2_GLOBALS._walkRegions.enableRegion(2);
	}

	_item2.setDetails(Rect(52, 44, 96, 82), 2430, 3, -1, 5, 1, NULL);
	_item3.setDetails(Rect(117, 36, 161, 74), 2430, 3, -1, 5, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 2430, 0, -1, -1, 1, NULL);
	R2_GLOBALS._player.disableControl();
	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		_sceneMode = 10;
		Common::Point pt(175, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2450) {
		_sceneMode = 30;
		Common::Point pt(175, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(210, 150));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
	R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2435;
	R2_GLOBALS._v56605[1 + R2_GLOBALS._player._characterIndex] = 12;
}

void Scene2435::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene2435::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 20:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	case 30:
		R2_GLOBALS._player._characterScene[1] = 2435;
		R2_GLOBALS._player._characterScene[2] = 2435;
		R2_GLOBALS._player._oldCharacterScene[1] = 2435;
		R2_GLOBALS._player._oldCharacterScene[2] = 2435;
		R2_GLOBALS._v56605[1] = 12;
		R2_GLOBALS._v56605[2] = 12;
		R2_GLOBALS.setFlag(81);
		_sceneMode = 2436;
		R2_GLOBALS._player.setStrip(7);
		_actor1.postInit();
		if (R2_GLOBALS._player._characterIndex == 1)
			_actor1.setVisage(20);
		else
			_actor1.setVisage(2008);
		setAction(&_sequenceManager, this, 2436, &_actor1, NULL);
		break;
	case 2436:
		R2_GLOBALS._walkRegions.enableRegion(2);
		_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(709, this);
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2440 - Ice Maze: Another bedroom
 *
 *--------------------------------------------------------------------------*/

bool Scene2440::Actor1::startAction(CursorType action, Event &event) {
	return SceneActor::startAction(action, event);
}

bool Scene2440::Actor2::startAction(CursorType action, Event &event) {
	Scene2440 *scene = (Scene2440 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == 2)){
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2440;
		scene->setAction(&scene->_sequenceManager, scene, 2440, &R2_GLOBALS._player, &scene->_actor2, NULL);
		return true;
	}

	return SceneActor::startAction(action, event);
}

void Scene2440::Exit1::changeScene() {
	Scene2440 *scene = (Scene2440 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;
	Common::Point pt(210, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2440::postInit(SceneObjectList *OwnerList) {
	loadScene(2440);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(200);
	// Fix exit cursor, the original was using NW
	_exit1.setDetails(Rect(172, 155, 250, 167), EXITCURSOR_SE, 2000);
	_exit1.setDest(Common::Point(210, 160));
	if (R2_INVENTORY.getObjectScene(49) == 2440) {
		_actor2.postInit();
		_actor2.setup(2435, 1, 1);
		_actor2.setPosition(Common::Point(94, 80));
		_actor2.fixPriority(106);
		_actor2.setDetails(2430, 48, -1, -1, 1, (SceneItem *)NULL);
	}
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.enableControl();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}
	R2_GLOBALS._player.setPosition(Common::Point(210, 200));
	if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
		_actor1.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_actor1.setup(20, 5, 1);
			_actor1.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_actor1.setup(2008, 5, 1);
			_actor1.setDetails(9002, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_actor1.setPosition(Common::Point(38, 119));
	}

	_item2.setDetails(Rect(125, 25, 142, 73), 2430, 15, -1, 14, 1, NULL);
	_item3.setDetails(Rect(124, 78, 237, 120), 2430, 36, -1, 38, 1, NULL);
	_item4.setDetails(Rect(250, 3, 265, 133), 2430, 30, 31, 32, 1, NULL);
	_item5.setDetails(Rect(91, 117, 203, 140), 2430, 9, -1, 11, 1, NULL);
	_item6.setDetails(Rect(48, 78, 103, 112), 2430, 6, -1, -1, 1, NULL);
	_item7.setDetails(Rect(48, 31, 73, 52), 2430, 33, -1, 18, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 2430, 0, -1, -1, 1, NULL);

	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2440;
		Common::Point pt(210, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(210, 150));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}

void Scene2440::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene2440::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 2440:
		_actor2.remove();
		R2_INVENTORY.setObjectScene(49, 2);
	// No break on purpose
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2445 - Ice Maze:
 *
 *--------------------------------------------------------------------------*/
void Scene2445::postInit(SceneObjectList *OwnerList) {
	loadScene(2445);
	SceneExt::postInit();
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.setPosition(Common::Point(160, 140));
	R2_GLOBALS._player.disableControl();
}

void Scene2445::signal() {
	R2_GLOBALS._player.enableControl();
}

/*--------------------------------------------------------------------------
 * Scene 2450 - Ice Maze: Another bedroom
 *
 *--------------------------------------------------------------------------*/

bool Scene2450::Actor2::startAction(CursorType action, Event &event) {
	Scene2450 *scene = (Scene2450 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == 1)) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2452;
		scene->setAction(&scene->_sequenceManager, scene, 2452, &R2_GLOBALS._player, &scene->_actor2, NULL);
		return true;
	}
	return SceneActor::startAction(action, event);
}

bool Scene2450::Actor3::startAction(CursorType action, Event &event) {
	Scene2450 *scene = (Scene2450 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_TALK) {
		R2_GLOBALS._player.disableControl();
		if (R2_GLOBALS._v565AE < 3) {
			++R2_GLOBALS._v565AE;
			scene->_sceneMode = 20;
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			if (R2_GLOBALS._player._characterIndex == 1)
				scene->_stripManager.start(699 + (R2_GLOBALS._v565AE * 2), scene);
			else
				scene->_stripManager.start(700 + (R2_GLOBALS._v565AE * 2), scene);
		}
		return true;} else {
		return SceneActor::startAction(action, event);
	}
}

void Scene2450::Exit1::changeScene() {
	Scene2450 *scene = (Scene2450 *)R2_GLOBALS._sceneManager._scene;

	if ((R2_GLOBALS._player._characterIndex == 2) || (R2_GLOBALS.getFlag(61))) {
		_enabled = false;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 10;
		Common::Point pt(-10, 180);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, scene);
	} else {
		_moving = false;
		SceneItem::display(2450, 3, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		Common::Point pt(60, 140);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, NULL);
	}
}

void Scene2450::postInit(SceneObjectList *OwnerList) {
	loadScene(2450);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(200);
	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._sceneManager._previousScene = 1900;
		R2_GLOBALS._player._oldCharacterScene[1] = 1900;
		R2_GLOBALS._player._oldCharacterScene[2] = 1900;
	}
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_caretakerSpeaker);

	if (R2_GLOBALS.getFlag(72)) {
		_exit1.setDetails(Rect(0, 143, 47, 168), EXITCURSOR_SW, 2000);
		_exit1.setDest(Common::Point(10, 160));
	}

	if (!R2_GLOBALS.getFlag(61)) {
		_actor2.postInit();
		_actor2.setVisage(2009);
		_actor2.setPosition(Common::Point(190, 119));
		_actor2.fixPriority(50);
		_actor2.setDetails(2450, 0, -1, -1, 1, (SceneItem *)NULL);
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();
	switch (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex]) {
	case 1900:
		R2_GLOBALS._v565AE = 0;
		R2_GLOBALS._player._characterScene[1] = 2450;
		R2_GLOBALS._player._characterScene[2] = 2450;
		R2_GLOBALS._player._oldCharacterScene[1] = 2450;
		R2_GLOBALS._player._oldCharacterScene[2] = 2450;
		R2_GLOBALS._player.setup(2450, 1, 1);
		R2_GLOBALS._player.setPosition(Common::Point(126, 101));
		setAction(&_sequenceManager, this, 2450, &R2_GLOBALS._player, NULL);
		break;
	case 2000:
		_sceneMode = 2451;
		if (R2_GLOBALS._player._characterIndex == 1) {
			if (R2_GLOBALS._player._characterScene[2] == 2450) {
				_actor1.postInit();
				_actor1.setup(20, 6, 1);
				_actor1.setPosition(Common::Point(240, 120));
				_actor1.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
			}
			setAction(&_sequenceManager, this, 2451, &R2_GLOBALS._player, NULL);
		} else {
			R2_GLOBALS._player._oldCharacterScene[2] = 2450;
			R2_GLOBALS._player._characterScene[2] = 2450;
			if (R2_GLOBALS._player._characterScene[1] == 2450) {
				_actor1.postInit();
				if (R2_GLOBALS.getFlag(61))
					_actor1.setup(2008, 6, 1);
				else
					_actor1.setup(10, 6, 1);
				_actor1.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
				_actor1.setPosition(Common::Point(106, 111));
			}
			setAction(&_sequenceManager, this, 2456, &R2_GLOBALS._player, NULL);
		}
		break;
	case 2450:
		if (R2_GLOBALS._player._characterIndex == 1) {
			R2_GLOBALS._player.postInit();
			if (R2_GLOBALS.getFlag(61)) {
				R2_GLOBALS._player.setup(2008, 6, 1);
			} else {
				R2_GLOBALS._player.setup(10, 6, 1);
			}
			R2_GLOBALS._player.setPosition(Common::Point(106, 111));
			R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
			if (R2_GLOBALS.getFlag(72)) {
				if (R2_GLOBALS._player._characterScene[2] == 2450) {
					_actor1.postInit();
					_actor1.setup(20, 6, 1);
					_actor1.setPosition(Common::Point(240, 120));
					_actor1.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
				}
			} else {
				_actor1.postInit();
				_actor1.setup(20, 8, 1);
				_actor1.setPosition(Common::Point(93, 158));
				_actor1.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);

				_actor3.postInit();
				_actor3.setup(2001, 7, 1);
				_actor3.setPosition(Common::Point(34, 153));
				_actor3.setDetails(2001, 40, -1, -1, 1, (SceneItem *)NULL);

				_exit1._enabled = false;
			}
		} else {
			R2_GLOBALS._player.postInit();
			R2_GLOBALS._player.setup(20, 8, 1);
			R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
			R2_GLOBALS._player.setPosition(Common::Point(93, 158));
			if (R2_GLOBALS.getFlag(72)) {
				if (R2_GLOBALS._player._characterScene[1] == 2450) {
					_actor1.postInit();
					if (R2_GLOBALS.getFlag(61)) {
						_actor1.setup(2008, 6, 1);
					} else {
						_actor1.setup(10, 6, 1);
					}
					_actor1.setPosition(Common::Point(106, 111));
					_actor1.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
				}
			} else {
				_actor1.postInit();
				if (R2_GLOBALS.getFlag(61)) {
					_actor1.setup(2008, 6, 1);
				} else {
					_actor1.setup(10, 6, 1);
				}
				_actor1.setPosition(Common::Point(106, 111));
				_actor1.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);

				_actor3.postInit();
				_actor3.setup(2001, 7, 1);
				_actor3.setPosition(Common::Point(34, 153));
				_actor3.setDetails(2001, 40, -1, -1, 1, (SceneItem *)NULL);

				_exit1._enabled = false;
			}
		}
		R2_GLOBALS._player.enableControl();
		if (!R2_GLOBALS.getFlag(72)) {
			R2_GLOBALS._player._canWalk = false;
		}
		break;
	default:
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		if (R2_GLOBALS._player._characterIndex == 1) {
			if (R2_GLOBALS.getFlag(61)) {
				R2_GLOBALS._player.setup(2008, 3, 1);
			} else {
				R2_GLOBALS._player.setup(10, 3, 1);
			}
			R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		} else {
			R2_GLOBALS._player.setVisage(20);
			R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
		}
		R2_GLOBALS._player.setPosition(Common::Point(100, 130));
		R2_GLOBALS._player.enableControl();
		break;
	}
	_item2.setDetails(Rect(174, 4, 199, 123), 2430, 30, 31, 32, 1, NULL);
	_item3.setDetails(Rect(67, 73, 207, 121), 2430, 36, -1, 38, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 2430, 0, -1, -1, 1, NULL);
}

void Scene2450::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene2450::signal() {
	switch (_sceneMode) {
	case 10:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 20:
		if (R2_GLOBALS._v565AE == 3) {
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._v565AE = 4;
			_sceneMode = 2454;
			setAction(&_sequenceManager, this, 2454, &_actor3, NULL);
		} else {
			R2_GLOBALS._player.enableControl(CURSOR_TALK);
			if (R2_GLOBALS._v565AE < 4)
				R2_GLOBALS._player._canWalk = false;
		}
		break;
	case 30:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 2455;
		setAction(&_sequenceManager, this, 2455, &_actor1, NULL);
		break;
	case 31:
		R2_GLOBALS.setFlag(61);
		g_globals->_sceneManager.changeScene(2435);
		break;
	case 2451:
		R2_GLOBALS._player.enableControl();
		break;
	case 2452:
		R2_GLOBALS.setFlag(61);
		_actor2.remove();
		R2_GLOBALS._player.enableControl();
		if (!R2_GLOBALS.getFlag(72)) {
			R2_GLOBALS._player.setStrip(6);
			R2_GLOBALS._player._canWalk = false;
		}
		break;
	case 2453:
		_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(700, this);
		break;
	case 2454:
		_exit1._enabled = true;
		R2_GLOBALS.setFlag(72);
		_actor3.remove();
		if (R2_GLOBALS.getFlag(61)) {
			g_globals->_sceneManager.changeScene(2435);
		} else {
			_sceneMode = 31;
			if (R2_GLOBALS._player._characterIndex == 1) {
				setAction(&_sequenceManager, this, 2452, &R2_GLOBALS._player, NULL);
			} else {
				setAction(&_sequenceManager, this, 2452, &_actor1, &_actor2, NULL);
			}
		}
		break;
	case 2455:
		R2_GLOBALS._player._oldCharacterScene[2] = 2450;
		R2_GLOBALS._player._characterScene[2] = 2000;
		R2_GLOBALS._v56605[2] = 3;
		_actor1.remove();
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	default:
		_actor1.postInit();
		_actor1.setDetails(9002, 0, 4, 3, 2, (SceneItem *)NULL);
		_actor3.postInit();
		_actor3.setDetails(2001, 40, -1, -1, 2, (SceneItem *)NULL);
		_sceneMode = 2453;
		setAction(&_sequenceManager, this, 2453, &_actor3, &_actor1, NULL);
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2455 - Ice Maze: Inside crevasse
 *
 *--------------------------------------------------------------------------*/

bool Scene2455::Actor1::startAction(CursorType action, Event &event) {
	Scene2455 *scene = (Scene2455 *)R2_GLOBALS._sceneManager._scene;

	if (action == R2_GLASS_DOME) {
		if ((R2_INVENTORY.getObjectScene(49) == 2455) || (R2_INVENTORY.getObjectScene(50) == 2455)) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 2458;
			scene->_actor2._lookLineNum = 9;
			scene->_actor1.remove();
			scene->_actor3.postInit();
			scene->_actor3.setDetails(2455, 16, 1, -1, 2, (SceneItem *)NULL);
			scene->setAction(&scene->_sequenceManager, scene, 2458, &R2_GLOBALS._player, &scene->_actor2, &scene->_actor3, NULL);
			return true;
		}
	}

	return SceneActor::startAction(action, event);
}

bool Scene2455::Actor2::startAction(CursorType action, Event &event) {
	Scene2455 *scene = (Scene2455 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_ALCOHOL_LAMP_2:
		if (R2_INVENTORY.getObjectScene(50) != 2455) {
			R2_GLOBALS._player.disableControl();
			scene->_actor1.postInit();
			scene->_actor1.setup(2456, 3, 3);
			scene->_actor1.setPosition(Common::Point(162, 165));
			scene->_actor1.setDetails(2455, 15, 1, -1, 2, (SceneItem *)NULL);
			scene->_sceneMode = 11;
			scene->setAction(&scene->_sequenceManager, scene, 2457, &R2_GLOBALS._player, &scene->_actor2, NULL);
			return true;
		}
		break;
	case R2_ALCOHOL_LAMP_3:
		if (R2_INVENTORY.getObjectScene(49) != 2455) {
			R2_GLOBALS._player.disableControl();
			scene->_actor1.postInit();
			scene->_actor1.setup(2456, 3, 3);
			scene->_actor1.setPosition(Common::Point(162, 165));
			scene->_actor1.setDetails(2455, 15, 1, -1, 2, (SceneItem *)NULL);
			scene->_sceneMode = 12;
			scene->setAction(&scene->_sequenceManager, scene, 2457, &R2_GLOBALS._player, &scene->_actor2, NULL);
			return true;
		}
		break;
	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

bool Scene2455::Actor3::startAction(CursorType action, Event &event) {
	Scene2455 *scene = (Scene2455 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2459;
		scene->setAction(&scene->_sequenceManager, scene, 2459, &R2_GLOBALS._player, &scene->_actor3, NULL);
		return true;
	}

	return SceneActor::startAction(action, event);
}

void Scene2455::Exit1::changeScene() {
	Scene2455 *scene = (Scene2455 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 2461;
	scene->setAction(&scene->_sequenceManager, scene, 2461, &R2_GLOBALS._player, NULL);
}

void Scene2455::postInit(SceneObjectList *OwnerList) {
	loadScene(2455);
	SceneExt::postInit();

	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_INVENTORY.setObjectScene(29, 2);
		R2_INVENTORY.setObjectScene(50, 2);
	}

	R2_GLOBALS._sound1.play(200);
	_exit1.setDetails(Rect(0, 0, 320, 15), EXITCURSOR_N, 2425);

	if (R2_INVENTORY.getObjectScene(29) == 2455) {
		if ((R2_INVENTORY.getObjectScene(50) == 2455) || (R2_INVENTORY.getObjectScene(49) == 2455)) {
			_actor1.postInit();
			_actor1.setup(2456, 3, 3);
			_actor1.setPosition(Common::Point(162, 165));
			_actor1.setDetails(2455, 15, 1, -1, 1, (SceneItem *)NULL);
		}
	} else {
		_actor3.postInit();
		_actor3.setup(2456, 3, 1);
		_actor3.setPosition(Common::Point(176, 165));
		_actor3.setDetails(2455, 16, 1, -1, 1, (SceneItem *)NULL);
	}

	_actor2.postInit();
	if (R2_INVENTORY.getObjectScene(29) == 2455) {
		_actor2.setup(2456, 3, 2);
		_actor2.setDetails(2455, 9, 1, -1, 1, (SceneItem *)NULL);
	} else {
		if ((R2_INVENTORY.getObjectScene(50) != 2455) && (R2_INVENTORY.getObjectScene(49) != 2455))
			_actor2.setup(2455, 1, 1);
		else
			_actor2.setup(2456, 1, 1);
		_actor2.setDetails(2455, 3, 1, -1, 1, (SceneItem *)NULL);
	}
	_actor2.setPosition(Common::Point(162, 165));
	_actor2.fixPriority(20);
	if (R2_INVENTORY.getObjectScene(29) != 2455)
		_actor2.animate(ANIM_MODE_2, NULL);

	R2_GLOBALS._player.postInit();
	_item1.setDetails(Rect(0, 0, 320, 200), 2455, 0, 1, -1, 1, NULL);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2425) {
		_sceneMode = 2460;
		setAction(&_sequenceManager, this, 2460, &R2_GLOBALS._player, NULL);
	} else {
		R2_GLOBALS._player.setup(2455, 2, 9);
		R2_GLOBALS._player.setPosition(Common::Point(118, 165));
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
	}
	R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2455;
}

void Scene2455::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene2455::signal() {
	switch (_sceneMode) {
	case 10:
	// No break on purpose
	case 2461:
		g_globals->_sceneManager.changeScene(2425);
		break;
	case 11:
		R2_INVENTORY.setObjectScene(49, 2455);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 12:
		R2_INVENTORY.setObjectScene(50, 2455);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 2458:
		R2_INVENTORY.setObjectScene(29, 2455);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 2459:
		_actor3.remove();
		R2_INVENTORY.setObjectScene(31, 2);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2500 - Ice Maze: Large Cave
 *
 *--------------------------------------------------------------------------*/

void Scene2500::Exit1::changeScene() {
	Scene2500 *scene = (Scene2500 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(20, 105);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2500::postInit(SceneObjectList *OwnerList) {
	loadScene(2500);
	SceneExt::postInit();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 3100)
		R2_GLOBALS._uiElements._active = false;

	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);
	_stripManager.addSpeaker(&_webbsterSpeaker);

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._sceneManager._previousScene = 2000;

	_exit1.setDetails(Rect(30, 50, 85, 105), EXITCURSOR_W, 2000);
	_exit1.setDest(Common::Point(84, 104));
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.setVisage(11);
		R2_GLOBALS._player._moveDiff = Common::Point(2, 1);
	} else {
		R2_GLOBALS._player.setVisage(21);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	}

	if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
		_actor1.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_actor1.setup(21, 3, 1);
			_actor1.setDetails(9002, 1, -1, -1, 1, (SceneItem *)NULL);
		} else {
			_actor1.setup(2008, 3, 1);
			_actor1.changeZoom(50);
			_actor1.setDetails(9001, 0, -1, -1, 1, (SceneItem *)NULL);
		}
		_actor1.setPosition(Common::Point(141, 94));
	}

	_item1.setDetails(Rect(0, 0, 320, 200), 2430, 0, -1, -1, 1, NULL);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		_sceneMode = 10;
		R2_GLOBALS._player.setPosition(Common::Point(20, 105));
		Common::Point pt(95, 105);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 3100) {
		_sceneMode = 2500;
		_actor2.postInit();
		_actor3.postInit();
		setAction(&_sequenceManager, this, 2500, &R2_GLOBALS._player, &_actor2, &_actor3, NULL);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(160, 150));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
	R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2500;
}

void Scene2500::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 20:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 2501;
		setAction(&_sequenceManager, this, 2501, &R2_GLOBALS._player, &_actor2, &_actor3, NULL);
		break;
	case 2500:
		_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(900, this);
		break;
	case 2501:
		g_globals->_sceneManager.changeScene(1000);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2525 - Furnace room
 *
 *--------------------------------------------------------------------------*/
bool Scene2525::Item5::startAction(CursorType action, Event &event) {
	Scene2525 *scene = (Scene2525 *)R2_GLOBALS._sceneManager._scene;

	if ((action == R2_REBREATHER_TANK) && (!R2_GLOBALS.getFlag(74))) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2526;
		scene->setAction(&scene->_sequenceManager, scene, 2526, &R2_GLOBALS._player, NULL);
		return true;
	}

	return SceneItem::startAction(action, event);
}

bool Scene2525::Actor3::startAction(CursorType action, Event &event) {
	Scene2525 *scene = (Scene2525 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._characterIndex == 2) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2525;
		scene->setAction(&scene->_sequenceManager, scene, 2525, &R2_GLOBALS._player, &scene->_actor3, NULL);
	} else {
		SceneItem::display(2530, 33, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
	}
	return true;
}

void Scene2525::Exit1::changeScene() {
	Scene2525 *scene = (Scene2525 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(R2_GLOBALS._player._position.x, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2525::postInit(SceneObjectList *OwnerList) {
	loadScene(2525);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(200);
	R2_GLOBALS._sound2.play(207);

	_exit1.setDetails(Rect(86, 155, 228, 168), EXITCURSOR_S, 2000);

	if (R2_INVENTORY.getObjectScene(29) == 2525) {
		_actor3.postInit();
		_actor3.setup(2435, 1, 2);
		_actor3.setPosition(Common::Point(78, 155));
		_actor3.fixPriority(155);
		_actor3.setDetails(2525, 27, -1, -1, 1, (SceneItem *)NULL);
	}

	_actor2.postInit();
	_actor2.setup(2525, 1, 1);
	_actor2.setPosition(Common::Point(183, 114));
	_actor2.setDetails(2525, 15, -1, -1, 1, (SceneItem *)NULL);
	_actor2.animate(ANIM_MODE_2, NULL);
	_actor2._numFrames = 3;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.setup(2008, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setup(20, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}

	if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
		_actor1.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_actor1.setup(20, 5, 1);
			_actor1.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_actor1.setup(2008, 5, 1);
			_actor1.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_actor1.setPosition(Common::Point(209, 162));

		R2_GLOBALS._walkRegions.enableRegion(4);
	}

	_item5.setDetails(Rect(125, 73, 140, 86), 2525, 6, -1, -1, 1, NULL);
	_item3.setDetails(Rect(137, 11, 163, 72), 2525, 12, -1, -1, 1, NULL);
	_item4.setDetails(Rect(204, 20, 234, 78), 2525, 12, -1, -1, 1, NULL);
	_item2.setDetails(Rect(102, 62, 230, 134), 2525, 0, -1, -1, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 2525, 24, -1, -1, 1, NULL);

	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2525;
		R2_GLOBALS._player.setPosition(Common::Point(160, 200));
		Common::Point pt(160, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(160, 150));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}

void Scene2525::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	R2_GLOBALS._sound2.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene2525::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 2525:
		_actor3.remove();
		R2_INVENTORY.setObjectScene(29, 2);
		R2_GLOBALS._player.enableControl();
		break;
	case 2526:
		R2_GLOBALS.setFlag(74);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2530 - Ice Maze: Well
 *
 *--------------------------------------------------------------------------*/
bool Scene2530::Actor2::startAction(CursorType action, Event &event) {
	Scene2530 *scene = (Scene2530 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._characterIndex == 2) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2530;
		scene->setAction(&scene->_sequenceManager, scene, 2530, &R2_GLOBALS._player, &scene->_actor2, NULL);
	} else {
		SceneItem::display(2530, 33, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
	}

	return true;
}

bool Scene2530::Actor3::startAction(CursorType action, Event &event) {
	Scene2530 *scene = (Scene2530 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._characterIndex == 1) {
		if (R2_GLOBALS.getFlag(73))
			SceneItem::display(2530, 35, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 2532;
			scene->setAction(&scene->_sequenceManager, scene, 2532, &R2_GLOBALS._player, &scene->_actor3, NULL);
		}
	} else {
		if (R2_GLOBALS.getFlag(73)) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 2533;
			scene->setAction(&scene->_sequenceManager, scene, 2533, &R2_GLOBALS._player, &scene->_actor3, NULL);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 2531;
			scene->setAction(&scene->_sequenceManager, scene, 2531, &R2_GLOBALS._player, &scene->_actor3, NULL);
		}
	}

	return true;
}

void Scene2530::Exit1::changeScene() {
	Scene2530 *scene = (Scene2530 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(108, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2530::postInit(SceneObjectList *OwnerList) {
	loadScene(2530);
	SceneExt::postInit();

	_exit1.setDetails(Rect(68, 155, 147, 168), EXITCURSOR_S, 2000);
	_exit1.setDest(Common::Point(108, 160));

	if (R2_INVENTORY.getObjectScene(33) == 2530) {
		_actor2.postInit();
		_actor2.setup(2435, 1, 3);
		_actor2.setPosition(Common::Point(299, 80));
		_actor2.fixPriority(80);
		_actor2.setDetails(2530, 28, -1, -1, 1, (SceneItem *)NULL);
	}

	_actor3.postInit();
	if (R2_GLOBALS.getFlag(73)) {
		_actor3.setup(2531, 4, 2);
		_actor3.setPosition(Common::Point(154, 130));
	} else {
		_actor3.setup(2531, 4, 1);
		_actor3.setPosition(Common::Point(173, 131));
	}
	_actor3.setDetails(2530, 22, -1, -1, 1, (SceneItem *)NULL);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}
	R2_GLOBALS._player.setPosition(Common::Point(100, 200));

	if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
		_actor1.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_actor1.setup(20, 5, 1);
			_actor1.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_actor1.setup(2008, 5, 1);
			_actor1.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_actor1.setPosition(Common::Point(20, 130));
		R2_GLOBALS._walkRegions.enableRegion(1);
	}

	_item2.setDetails(Rect(108, 90, 135, 205), 2530, 22, -1, -1, 1, NULL);
	_item5.setDetails(Rect(115, 112, 206, 130), 2530, 25, -1, 27, 1, NULL);
	_item3.setDetails(Rect(256, 64, 311, 85), 2530, 31, -1, 33, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 2530, 0, 1, -1, 1, NULL);

	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2530;
		Common::Point pt(108, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(105, 145));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}

void Scene2530::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 2530:
		R2_INVENTORY.setObjectScene(33, 2);
		_actor2.remove();
		break;
	case 2531:
	// No break on purpose
	case 2532:
		R2_GLOBALS.setFlag(73);
		R2_GLOBALS._player.enableControl();
		break;
	case 2533:
		R2_GLOBALS.clearFlag(73);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2535 - Ice Maze: Tannery
 *
 *--------------------------------------------------------------------------*/

bool Scene2535::Actor3::startAction(CursorType action, Event &event) {
	Scene2535 *scene = (Scene2535 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.disableControl();
		if (R2_INVENTORY.getObjectScene(20) == 2535) {
			scene->_sceneMode = 2536;
			scene->setAction(&scene->_sequenceManager, scene, 2536, &R2_GLOBALS._player, &scene->_actor3, NULL);
		} else {
			scene->_sceneMode = 2537;
			scene->setAction(&scene->_sequenceManager, scene, 2537, &R2_GLOBALS._player, &scene->_actor3, NULL);
		}
	} else {
		SceneItem::display(2530, 33, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
	}

	return true;
}

bool Scene2535::Actor4::startAction(CursorType action, Event &event) {
	Scene2535 *scene = (Scene2535 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._characterIndex == 2) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2535;
		scene->setAction(&scene->_sequenceManager, scene, 2535, &R2_GLOBALS._player, &scene->_actor4, NULL);
	} else {
		SceneItem::display(2530, 33, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
	}

	return true;
}

void Scene2535::Exit1::changeScene() {
	Scene2535 *scene = (Scene2535 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;
	Common::Point pt(210, 200);
	PlayerMover *mover = new PlayerMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2535::postInit(SceneObjectList *OwnerList) {
	loadScene(2535);
	SceneExt::postInit();
	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS.setFlag(73);
		R2_INVENTORY.setObjectScene(20, 2535);
	}
	_exit1.setDetails(Rect(172, 155, 250, 167), EXITCURSOR_S, 2000);
	_exit1.setDest(Common::Point(210, 160));

	if (R2_INVENTORY.getObjectScene(32) == 2535) {
		_actor4.postInit();
		_actor4.setup(2435, 1, 4);
		_actor4.setPosition(Common::Point(47, 74));
		_actor4.fixPriority(74);
		_actor4.setDetails(2535, 21, -1, -1, 1, (SceneItem *)NULL);
	}

	if (R2_INVENTORY.getObjectScene(20) == 2535) {
		_actor3.postInit();
		_actor3.setup(2535, 3, 1);
		_actor3.setPosition(Common::Point(203, 131));
		_actor3.setDetails(3, 20, -1, -1, 1, (SceneItem *)NULL);
		R2_GLOBALS._walkRegions.enableRegion(6);
	}

	if ((R2_INVENTORY.getObjectScene(20) == 0) && (R2_GLOBALS.getFlag(73))) {
		_actor3.postInit();
		_actor3.setup(2536, 1, 2);
		_actor3.setPosition(Common::Point(164, 133));
		_actor3.setDetails(3, 20, -1, -1, 1, (SceneItem *)NULL);
	}

	if (R2_GLOBALS.getFlag(73)) {
		_actor2.postInit();
		_actor2.setup(2536, 1, 1);
		_actor2.setPosition(Common::Point(160, 130));
		_actor2.fixPriority(122);
		_actor2.setDetails(2535, 37, -1, -1, 1, (SceneItem *)NULL);
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}
	R2_GLOBALS._player.setPosition(Common::Point(210, 200));

	if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
		_actor1.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_actor1.setup(20, 5, 1);
			_actor1.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_actor1.setup(2008, 5, 1);
			_actor1.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_actor1.setPosition(Common::Point(245, 115));
		R2_GLOBALS._walkRegions.enableRegion(2);
	}

	_item2.setDetails(Rect(96, 3, 215, 33), 2535, 3, 6, 5, 1, NULL);
	_item3.setDetails(Rect(4, 43, 40, 101), 2535, 6, 7, 8, 1, NULL);
	_item4.setDetails(Rect(55, 13, 140, 89), 2535, 6, 7, 8, 1, NULL);
	_item5.setDetails(Rect(144, 23, 216, 76), 2535, 6, 7, 8, 1, NULL);
	_item6.setDetails(Rect(227, 8, 307, 99), 2535, 6, 7, 8, 1, NULL);
	_item7.setDetails(Rect(116, 111, 201, 132), 2535, 18, 19, 20, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 2535, 0, 1, -1, 1, NULL);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2535;
		Common::Point pt(210, 150);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(210, 150));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}

void Scene2535::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 2535:
		R2_INVENTORY.setObjectScene(32, 2);
		_actor4.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 2536:
		R2_INVENTORY.setObjectScene(20, 0);
		R2_GLOBALS._walkRegions.disableRegion(6);
		if (!R2_GLOBALS.getFlag(73)) {
			_actor3.remove();
			R2_GLOBALS._player.enableControl();
		} else {
			_sceneMode = 20;
			_actor3.show();
			_actor3.setup(2536, 1, 2);
			_actor3.setDetails(3, 20, -1, -1, 3, (SceneItem *)NULL);
			_actor3.setPosition(Common::Point(164, 150));
			_actor3.fixPriority(130);
			_actor3._moveDiff.y = 1;
			Common::Point pt(164, 133);
			PlayerMover *mover = new PlayerMover();
			_actor3.addMover(mover, &pt, this);
		}
		break;
	case 2537:
		_actor3.remove();
		R2_INVENTORY.setObjectScene(20, 1);
		R2_GLOBALS._player.enableControl();
		break;
	case 20:
	// No break on purpose
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2600 - Ice Maze: Exit
 *
 *--------------------------------------------------------------------------*/
Scene2600::Scene2600(): SceneExt() {
	_rotation = NULL;
}

void Scene2600::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	SYNC_POINTER(_rotation);
}

void Scene2600::postInit(SceneObjectList *OwnerList) {
	loadScene(2600);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();
	R2_GLOBALS._sound1.fadeSound(214);
	R2_GLOBALS._sound2.play(215);
	_rotation = R2_GLOBALS._scenePalette.addRotation(176, 191, 1);
	_rotation->setDelay(3);
	_rotation->_countdown = 1;
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();
	_sceneMode = 2600;
	R2_GLOBALS._player.setAction(&_sequenceManager, this, 2600, &R2_GLOBALS._player, NULL);
}

void Scene2600::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	R2_GLOBALS._sound2.fadeOut2(NULL);
//	_rotation->remove();
	SceneExt::remove();
}

void Scene2600::signal() {
	if (_sceneMode == 2600)
		g_globals->_sceneManager.changeScene(3800);
}

/*--------------------------------------------------------------------------
 * Scene 2700 - Forest Maze
 *
 *--------------------------------------------------------------------------*/
Scene2700::Scene2700(): SceneExt() {
	_field412 = _field414 = _field416 = 0;
}

void Scene2700::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_field414);
	s.syncAsSint16LE(_field416);
}

void Scene2700::Action1::signal() {
	Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_actor2.animate(ANIM_MODE_5, NULL);
}

void Scene2700::Action2::signal() {
	Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;

	setDelay(300 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_actor3.animate(ANIM_MODE_5, NULL);
}

void Scene2700::Action3::signal() {
	Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;

	setDelay(450 + R2_GLOBALS._randomSource.getRandomNumber(450));
	scene->_actor4.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2700::Action4::signal() {
	Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;

	setDelay(300 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_actor5.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2700::Area1::process(Event &event) {
	SceneArea::process(event);
	if ((event.eventType == 1) && (R2_GLOBALS._player._canWalk) && (_bounds.contains(event.mousePos))) {
		Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 10;
		scene->_field414 = 2703;
		switch (scene->_field412) {
		case 0:
		// No break on purpose
		case 6:
			scene->_sceneMode = 2703;
			scene->setAction(&scene->_sequenceManager, scene, 2703, &R2_GLOBALS._player, NULL);
			break;
		case 1:	{
			Common::Point pt(80, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 2:	{
			Common::Point pt(155, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 3:	{
			Common::Point pt(140, 162);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 4:	{
			Common::Point pt(155, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 5:	{
			Common::Point pt(235, 132);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		default:
			break;
		}
	}
}

void Scene2700::Area2::process(Event &event) {
	SceneArea::process(event);
	if ((event.eventType == 1) && (R2_GLOBALS._player._canWalk) && (_bounds.contains(event.mousePos))) {
		Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 10;
		scene->_field414 = 2704;
		switch (scene->_field412) {
		case 0:	{
			Common::Point pt(140, 162);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 1:	{
			Common::Point pt(80, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 2:	{
			Common::Point pt(155, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 3:	{
			Common::Point pt(155, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 4:	{
			Common::Point pt(235, 132);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 5:
			scene->_sceneMode = 2704;
			scene->setAction(&scene->_sequenceManager, scene, 2704, &R2_GLOBALS._player, NULL);
			break;
		case 6:	{
			Common::Point pt(140, 162);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		default:
			break;
		}
	}
}

void Scene2700::postInit(SceneObjectList *OwnerList) {
	loadScene(2700);
	SceneExt::postInit();
	R2_GLOBALS._sound1.stop();
	R2_GLOBALS._sound2.stop();

	_area1.setDetails(Rect(135, 160, 185, 168), SHADECURSOR_DOWN);
	_area2.setDetails(Rect(300, 90, 320, 135), EXITCURSOR_E);
	_rect1.set(70, 122, 90, 132);
	_rect2.set(150, 122, 160, 132);
	_rect3.set(90, 142, 130, 157);
	_rect4.set(175, 137, 200, 147);
	_rect5.set(280, 127, 300, 137);
	_rect6.set(240, 157, 265, 167);

	_actor2.postInit();
	_actor2.setup(2700, 1, 1);
	_actor2.setPosition(Common::Point(140, 29));
	_actor2.setAction(&_action1);

	_actor3.postInit();
	_actor3.setup(2700, 2, 1);
	_actor3.setPosition(Common::Point(213, 32));
	_actor3.setAction(&_action2);

	_actor4.postInit();
	_actor4.setup(2700, 3, 1);
	_actor4.setPosition(Common::Point(17, 39));
	_actor4.setAction(&_action3);

	_actor5.postInit();
	_actor5.setup(2700, 5, 1);
	_actor5.setPosition(Common::Point(17, 71));
	_actor5.setAction(&_action4);

	_item2.setDetails(Rect(52, 38, 68, 60), 2700, 4, -1, 6, 1, NULL);
	_item3.setDetails(Rect(113, 22, 127, 33), 2700, 4, -1, 6, 1, NULL);
	_item4.setDetails(Rect(161, 44, 170, 52), 2700, 4, -1, 6, 1, NULL);
	_item5.setDetails(Rect(221, 19, 233, 31), 2700, 4, -1, 6, 1, NULL);
	_item6.setDetails(Rect(235, 59, 250, 75), 2700, 4, -1, 6, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 2700, 4, -1, 6, 1, NULL);

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_nejSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(19);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player._moveDiff = Common::Point(2, 2);
	R2_GLOBALS._player.disableControl();

	if (R2_INVENTORY.getObjectScene(36) == 0)
		R2_GLOBALS._sound1.changeSound(234);

	if (R2_GLOBALS._sceneManager._previousScene == 2750) {
		_sceneMode = 2702;
		_field412 = 5;
		setAction(&_sequenceManager, this, 2702, &R2_GLOBALS._player, NULL);
	} else {
		_field412 = 0;
		if (R2_GLOBALS._sceneManager._previousScene == 3900) {
			_sceneMode = 2701;
			setAction(&_sequenceManager, this, 2701, &R2_GLOBALS._player, NULL);
		} else {
			R2_GLOBALS._player.setStrip(3);
			R2_GLOBALS._player.setPosition(Common::Point(164, 160));
			R2_GLOBALS._player.enableControl();
		}
	}
}

void Scene2700::signal() {
	switch (_sceneMode) {
	case 10:
		switch (_field414) {
		case 1:
			switch (_field412) {
			case 0:
			case 2:
			case 4:
			case 6:
				_field412 = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2705, &R2_GLOBALS._player, NULL);
				break;
			case 3: {
				_sceneMode = _field414;
				_field412 = 1;
				Common::Point pt(80, 127);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			case 5:
				_field412 = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default: // includes case 1
				break;
			}
			break;
		case 2:
			switch (_field412) {
			case 0:
			case 1:
			case 6:
				_field412 = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2706, &R2_GLOBALS._player, NULL);
				break;
			case 3:
			case 4: {
				_sceneMode = _field414;
				_field412 = 2;
				Common::Point pt(155, 127);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			case 5:
				_field412 = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default: // includes case 2
				break;
			}
			break;
		case 3:
			switch (_field412) {
			case 0:
			case 1:
			case 2:
			case 4:
			case 6: {
				_sceneMode = _field414;
				_field412 = 3;
				Common::Point pt(115, 152);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			case 5:
				_field412 = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default: // includes case 3
				break;
			}
			break;
		case 4:
			switch (_field412) {
			case 0:
			case 1:
			case 6:
				_field412 = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2706, &R2_GLOBALS._player, NULL);
				break;
			case 2:
			case 3:
				_field412 = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2709, &R2_GLOBALS._player, NULL);
				break;
			case 4:
			case 5:
				_sceneMode = _field414;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2704, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		case 5:
			switch (_field412) {
			case 0:
			case 1:
			case 6:
				_field412 = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2706, &R2_GLOBALS._player, NULL);
				break;
			case 2:
			case 3:
				_field412 = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2709, &R2_GLOBALS._player, NULL);
				break;
			case 4: {
				_sceneMode = _field414;
				_field412 = 5;
				Common::Point pt(285, 132);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			default: // includes case 5
				break;
			}
			break;
		case 6:
			switch (_field412) {
			case 0:
			case 3: {
				_sceneMode = _field414;
				_field412 = 6;
				Common::Point pt(250, 162);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			case 1:
			case 2:
			case 4:
				_field412 = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2707, &R2_GLOBALS._player, NULL);
				break;
			case 5:
				_field412 = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		case 2703:
			switch (_field412) {
			case 0:
			case 3:
			case 6:
				_sceneMode = _field414;
				setAction(&_sequenceManager, this, 2703, &R2_GLOBALS._player, NULL);
				break;
			case 1:
			case 2:
			case 4:
				_field412 = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2707, &R2_GLOBALS._player, NULL);
				break;
			case 5:
				_field412 = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		case 2704:
			switch (_field412) {
			case 0:
			case 1:
			case 6:
				_field412 = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2706, &R2_GLOBALS._player, NULL);
				break;
			case 2:
			case 3:
				_field412 = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2709, &R2_GLOBALS._player, NULL);
				break;
			case 4:
			case 5:
				_sceneMode = _field414;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2704, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		case 2710:
			switch (_field412) {
			case 0:
			case 1:
			case 3:
				_field412 = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2707, &R2_GLOBALS._player, NULL);
				break;
			case 2:
			case 5: {
				_sceneMode = _field414;
				Common::Point pt(164, 160);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			case 4:
				_field412 = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	case 11:
		R2_INVENTORY.setObjectScene(36, 0);
		R2_GLOBALS._player.disableControl();
		_field412 = 0;
		_sceneMode = 2700;
		setAction(&_sequenceManager, this, 2700, &_actor1, NULL);
		break;
	case 12:
		R2_GLOBALS._sound1.play(234);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_sceneMode = 2711;
		_stripManager.start(_field416, this);
		break;
	case 13:
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_sceneMode = 2712;
		_stripManager.start(_field416, this);
		break;
	case 14:
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_sceneMode = 2713;
		_stripManager.start(_field416, this);
		break;
	case 15:
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_sceneMode = 11;
		_stripManager.start(_field416, this);
		break;
	case 2700:
		_actor1.remove();
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	case 2703:
		g_globals->_sceneManager.changeScene(3900);
		break;
	case 2704:
		g_globals->_sceneManager.changeScene(2750);
		break;
	case 2710:
		_field416 = 1200;
		_sceneMode = 12;
		_actor1.postInit();
		setAction(&_sequenceManager, this, 2710, &R2_GLOBALS._player, &_actor1, NULL);
		break;
	case 2711:
		R2_GLOBALS._player.disableControl();
		_field416 = 1201;
		_sceneMode = 13;
		setAction(&_sequenceManager, this, 2711, &R2_GLOBALS._player, &_actor1, NULL);
		break;
	case 2712:
		R2_GLOBALS._player.disableControl();
		_field416 = 1202;
		_sceneMode = 14;
		setAction(&_sequenceManager, this, 2712, &R2_GLOBALS._player, &_actor1, NULL);
		break;
	case 2713:
		R2_GLOBALS._player.disableControl();
		_field416 = 1203;
		_sceneMode = 14;
		setAction(&_sequenceManager, this, 2713, &R2_GLOBALS._player, &_actor1, NULL);
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	}
}
void Scene2700::process(Event &event) {
	if ((R2_GLOBALS._player._canWalk) && (event.eventType == EVENT_BUTTON_DOWN)) {
		if (R2_GLOBALS._events.getCursor() == R2_FLUTE) {
			if (R2_GLOBALS._player._bounds.contains(event.mousePos)) {
				_sceneMode = 10;
				_field414 = 2710;
				R2_GLOBALS._player.disableControl();
				R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
				switch (_field412) {
				case 0: {
					_sceneMode = 2710;
					Common::Point pt(164, 160);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 1: {
					Common::Point pt(80, 137);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 2: {
					Common::Point pt(155, 137);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 3: {
					Common::Point pt(140, 162);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 4: {
					Common::Point pt(155, 137);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 5: {
					Common::Point pt(235, 132);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 6: {
					Common::Point pt(205, 162);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				default:
					break;
				}
				event.handled = true;
			} else {
				SceneItem::display(2700, 3, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
			}
		} else if (R2_GLOBALS._events.getCursor() == R2_NEGATOR_GUN) {
			if (_rect1.contains(event.mousePos)) {
				if (!_rect1.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_field414 = 1;
				}
			} else if (_rect2.contains(event.mousePos)) {
				if (!_rect2.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_field414 = 2;
				}
			} else if (_rect3.contains(event.mousePos)) {
				if (!_rect3.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_field414 = 3;
				}
			} else if (_rect4.contains(event.mousePos)) {
				if (!_rect4.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_field414 = 4;
				}
			} else if (_rect5.contains(event.mousePos)) {
				if (!_rect5.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_field414 = 5;
				}
			} else if (_rect6.contains(event.mousePos)) {
				if (!_rect6.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_field414 = 6;
				}
			} else {
				event.handled = true;
				R2_GLOBALS._player.updateAngle(Common::Point(event.mousePos.x, event.mousePos.y));
			}
			if (_sceneMode == 10) {
				R2_GLOBALS._player.disableControl();
				switch (_field412) {
				case 0:
					if (_field414 >= 6) {
						Common::Point pt(205, 162);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					} else {
						Common::Point pt(140, 162);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					}
					break;
				case 1: {
					Common::Point pt(80, 137);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 2: {
					Common::Point pt(155, 137);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 3:
					if (_field414 == 1) {
						Common::Point pt(80, 137);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					} else if (_field414 == 6) {
						Common::Point pt(140, 162);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					} else {
						Common::Point pt(155, 137);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					}
					break;
				case 4:
					if (_field414 == 5) {
						Common::Point pt(235, 132);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					} else {
						Common::Point pt(155, 137);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					}
					break;
				case 5: {
					Common::Point pt(235, 132);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 6: {
					Common::Point pt(140, 162);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				default:
					break;
				}
			}
		}
	}
	Scene::process(event);
}

/*--------------------------------------------------------------------------
 * Scene 2750 - Forest Maze
 *
 *--------------------------------------------------------------------------*/
Scene2750::Scene2750(): SceneExt() {
	_field412 = _field414 = _field416 = 0;
}

void Scene2750::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_field414);
	s.syncAsSint16LE(_field416);
}

void Scene2750::Action1::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 1:
		setDelay(60 + R2_GLOBALS._randomSource.getRandomNumber(240));
		_actionIndex = 2;
		scene->_actor5.show();
		scene->_actor5.animate(ANIM_MODE_8, 1, NULL);
		break;
	case 2:
		setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(600));
		_actionIndex = 0;
		scene->_actor5.show();
		scene->_actor3.animate(ANIM_MODE_2, NULL);
		break;
	default:
		setDelay(30);
		_actionIndex = 1;
		scene->_actor3.animate(ANIM_MODE_6, NULL);
		scene->_actor4.animate(ANIM_MODE_8, 1, NULL);
		break;
	}
}

void Scene2750::Action2::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_actor6.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2750::Action3::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	if (scene->_actor7._position.x <= 320) {
		setDelay(1800 + R2_GLOBALS._randomSource.getRandomNumber(600));
	} else {
		setDelay(60);
		scene->_actor7.setPosition(Common::Point(-10, 25));
		Common::Point pt(330, 45);
		NpcMover *mover = new NpcMover();
		scene->_actor7.addMover(mover, &pt, NULL);
	}
}

void Scene2750::Action4::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_actor8.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2750::Action5::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_actor9.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2750::Action6::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_actor10.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2750::Action7::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_actor11.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2750::Area1::process(Event &event) {
	SceneArea::process(event);
	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._player._canWalk) && (_bounds.contains(event.mousePos))) {
		Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 10;
		scene->_field414 = 2752;
		switch (scene->_field412) {
		case 1:	{
			scene->_sceneMode = 2752;
			scene->setAction(&scene->_sequenceManager, scene, 2752, &R2_GLOBALS._player, NULL);
			break;
			}
		case 2:	{
			Common::Point pt(140, 142);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 3:	{
			Common::Point pt(210, 142);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		default:
			break;
		}
	}
}

void Scene2750::Area2::process(Event &event) {
	SceneArea::process(event);
	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._player._canWalk) && (_bounds.contains(event.mousePos))) {
		Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 10;
		scene->_field414 = 2753;
		switch (scene->_field412) {
		case 1:	{
			Common::Point pt(140, 142);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 2:	{
			Common::Point pt(210, 142);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 3:	{
			scene->_sceneMode = 2753;
			scene->setAction(&scene->_sequenceManager, scene, 2753, &R2_GLOBALS._player, NULL);
			break;
			}
		default:
			break;
		}
	}
}

void Scene2750::postInit(SceneObjectList *OwnerList) {
	loadScene(2750);
	R2_GLOBALS._sound2.stop();
	SceneExt::postInit();
	_area1.setDetails(Rect(0, 90, 20, 135), EXITCURSOR_W);
	_area2.setDetails(Rect(300, 90, 320, 135), EXITCURSOR_E);

	_rect1.set(30, 127, 155, 147);
	_rect2.set(130, 142, 210, 167);
	_rect3.set(-1, 137, 290, 147);

	if (R2_INVENTORY.getObjectScene(36) == 0) {
		R2_GLOBALS._sound1.changeSound(235);
		_actor2.postInit();
		_actor2.setup(2751, 1, 1);
		_actor2.setPosition(Common::Point(104, 158));
		_actor2.animate(ANIM_MODE_2, NULL);
	}

	_actor3.postInit();
	_actor3.setup(2750, 1, 1);
	_actor3.setPosition(Common::Point(188, 34));
	_actor3.animate(ANIM_MODE_2, NULL);
	_actor3._numFrames = 16;

	_actor4.postInit();
	_actor4.setup(2700, 4, 1);
	_actor4.setPosition(Common::Point(188, 37));
	_actor4.fixPriority(26);

	_actor5.postInit();
	_actor5.setup(2750, 2, 1);
	_actor5.setPosition(Common::Point(188, 34));
	_actor5.hide();

	_actor3.setAction(&_action1);

	_actor6.postInit();
	_actor6.setup(2750, 3, 1);
	_actor6.setPosition(Common::Point(9, 167));
	_actor6.fixPriority(252);
	_actor6.setAction(&_action2);

	_actor7.postInit();
	_actor7.setup(2750, 4, 1);
	_actor7.setPosition(Common::Point(-10, 25));
	_actor7.animate(ANIM_MODE_1, NULL);
	_actor7.setStrip2(4);
	_actor7._moveRate = 20;
	_actor7.setAction(&_action3);

	_actor8.postInit();
	_actor8.fixPriority(26);
	_actor8.setup(2750, 5, 1);
	_actor8.setPosition(Common::Point(258, 33));
	_actor8.setAction(&_action4);

	_actor9.postInit();
	_actor9.fixPriority(26);
	_actor9.setup(2750, 6, 1);
	_actor9.setPosition(Common::Point(61, 38));
	_actor9.setAction(&_action5);

	_actor10.postInit();
	_actor10.fixPriority(26);
	_actor10.setup(2750, 7, 1);
	_actor10.setPosition(Common::Point(69, 37));
	_actor10.setAction(&_action6);

	_actor11.postInit();
	_actor11.fixPriority(26);
	_actor11.setup(2750, 8, 1);
	_actor11.setPosition(Common::Point(80, 35));
	_actor11.setAction(&_action7);

	_item2.setDetails(Rect(29, 50, 35, 56), 2750, 3, -1, 5, 1, NULL);
	_item3.setDetails(Rect(47, 36, 54, 42), 2750, 3, -1, 5, 1, NULL);
	_item4.setDetails(Rect(193, 21, 206, 34), 2750, 3, -1, 5, 1, NULL);
	_item5.setDetails(Rect(301, 18, 315, 32), 2750, 3, -1, 5, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 2700, 0, -1, 2, 1, NULL);

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_nejSpeaker);

	if (R2_INVENTORY.getObjectScene(36) == 0) {
		_actor1.postInit();
		_actor1.setup(2752, 5, 1);
		_actor1.animate(ANIM_MODE_NONE, NULL);
		_actor1.setPosition(Common::Point(101, 148));
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(19);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player._moveDiff = Common::Point(2, 2);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._sceneManager._previousScene == 2700) {
		if (R2_INVENTORY.getObjectScene(36) == 0) {
			R2_GLOBALS._player.setVisage(2752);
			R2_GLOBALS._player.setStrip(6);
			R2_GLOBALS._player.animate(ANIM_MODE_NONE, NULL);
			R2_GLOBALS._player.setPosition(Common::Point(81, 165));
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			_field416 = 1204;
			_sceneMode = 11;
			_stripManager.start(_field416, this);
		} else {
			_sceneMode = 2750;
			_field412 = 1;
			R2_GLOBALS._player.setAction(&_sequenceManager, this, 2750, &R2_GLOBALS._player, NULL);
		}
	} else if (R2_GLOBALS._sceneManager._previousScene == 2800) {
		_sceneMode = 2751;
		_field412 = 3;
		R2_GLOBALS._player.setAction(&_sequenceManager, this, 2751, &R2_GLOBALS._player, NULL);
	} else {
		_field412 = 1;
		R2_GLOBALS._player.setPosition(Common::Point(90, 137));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}
void Scene2750::signal() {
	switch (_sceneMode) {
	case 10:
		switch (_field414) {
		case 1:
			switch (_field412) {
			case 2: {
				_sceneMode = _field414;
				_field412 = 1;
				Common::Point pt(90, 137);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 3: {
				_field412 = 2;
				Common::Point pt(140, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			default:
				break;
			}
			break;
		case 2: {
			_sceneMode = _field414;
			_field412 = 2;
			Common::Point pt(170, 162);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
			}
			break;
		case 3:
			switch (_field412) {
			case 1: {
				_field412 = 2;
				Common::Point pt(210, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 2: {
				_sceneMode = _field414;
				_field412 = 3;
				Common::Point pt(270, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			default:
				break;
			}
			break;
		case 2752:
			switch (_field412) {
			case 1:
				_sceneMode = _field414;
				setAction(&_sequenceManager, this, 2752, &R2_GLOBALS._player, NULL);
				break;
			case 2: {
				_field412 = 1;
				Common::Point pt(20, 132);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 3: {
				_field412 = 2;
				Common::Point pt(140, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			default:
				break;
			}
			break;
		case 2753:
			switch (_field412) {
			case 1: {
				_field412 = 2;
				Common::Point pt(210, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 2: {
				_field412 = 3;
				Common::Point pt(300, 132);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 3:
				_sceneMode = _field414;
				setAction(&_sequenceManager, this, 2753, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	case 11:
	// No break on purpose
	case 2753:
		g_globals->_sceneManager.changeScene(2800);
		break;
	case 2752:
		g_globals->_sceneManager.changeScene(2700);
		break;
	default:
		R2_GLOBALS._player.enableControl(R2_NEGATOR_GUN);
		break;
	}
}

void Scene2750::process(Event &event) {
	if ((R2_GLOBALS._player._canWalk) && (event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._events.getCursor() == R2_NEGATOR_GUN)) {
		if (_rect1.contains(event.mousePos)) {
			if (!_rect1.contains(R2_GLOBALS._player._position)) {
				event.handled = true;
				_sceneMode = 10;
				_field414 = 1;
			}
		} else if (_rect2.contains(event.mousePos)) {
			if (!_rect2.contains(R2_GLOBALS._player._position)) {
				event.handled = true;
				_sceneMode = 10;
				_field414 = 2;
			}
		} else if (_rect3.contains(event.mousePos)) {
			if (!_rect3.contains(R2_GLOBALS._player._position)) {
				event.handled = true;
				_sceneMode = 10;
				_field414 = 3;
			}
		} else {
			event.handled = true;
			R2_GLOBALS._player.updateAngle(Common::Point(event.mousePos.x, event.mousePos.y));
		}

		if (_sceneMode == 10) {
			R2_GLOBALS._player.disableControl();
			switch (_field412) {
			case 1: {
				Common::Point pt(140, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 2:
				if (_field414 == 1) {
					Common::Point pt(140, 142);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
				} else {
					Common::Point pt(210, 142);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 3: {
				Common::Point pt(210, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			default:
				break;
			}
		}
	}
	Scene::process(event);
}

/*--------------------------------------------------------------------------
 * Scene 2800 - Exiting forest
 *
 *--------------------------------------------------------------------------*/
Scene2800::Scene2800(): SceneExt() {
	_field412 = 0;
}

void Scene2800::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
}

bool Scene2800::Item2::startAction(CursorType action, Event &event) {
	Scene2800 *scene = (Scene2800 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (R2_GLOBALS.getFlag(47))) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2805;
		scene->setAction(&scene->_sequenceManager, scene, 2805, &R2_GLOBALS._player, NULL);
		return true;
	} else
		return SceneHotspot::startAction(action, event);
}

bool Scene2800::Actor1::startAction(CursorType action, Event &event) {
	Scene2800 *scene = (Scene2800 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_TALK) {
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		R2_GLOBALS.setFlag(47);
		scene->_field412 = 1205;
		scene->_sceneMode = 2803;
		scene->_stripManager.start(scene->_field412, scene);
		return true;
	} else if (action == R2_SONIC_STUNNER) {
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS.setFlag(47);
		scene->_sceneMode = 10;
		scene->setAction(&scene->_sequenceManager, scene, 2802, &R2_GLOBALS._player, &scene->_actor2, &scene->_actor1, NULL);
		return true;
	} else
		return SceneActor::startAction(action, event);
}

void Scene2800::Action1::signal() {
	Scene2800 *scene = (Scene2800 *)R2_GLOBALS._sceneManager._scene;

	if (R2_GLOBALS._player._position.x <= 320) {
		setDelay(120);
		Common::Point pt(330, 25);
		NpcMover *mover = new NpcMover();
		scene->_object1.addMover(mover, &pt, NULL);
	} else {
		setDelay(1800 + R2_GLOBALS._randomSource.getRandomNumber(600));
		scene->_object1.setPosition(Common::Point(-10, 45));
	}
}

void Scene2800::Action2::signal() {
	Scene2800 *scene = (Scene2800 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(240);
		R2_GLOBALS._sound1.changeSound(240);
		R2_GLOBALS._sound2.stop();
		break;
	case 1:
		_object2.postInit();
		_object2.setVisage(2800);
		_object2.setStrip(1);
		_object2._numFrames = 8;
		_object2._moveRate = 8;
		_object2.changeZoom(100);
		_object2.setPosition(Common::Point(1, 1));
		_object2.show();
		_object2.animate(ANIM_MODE_5, this);
		break;
	case 2:
		R2_GLOBALS._sound2.play(130);
		_object2.setVisage(2800);
		_object2.setStrip(7);

		_object3.postInit();
		_object3.setVisage(2800);
		_object3.setStrip(3);
		_object3._numFrames = 8;
		_object3._moveRate = 8;
		_object3.changeZoom(100);
		_object3.setPosition(Common::Point(300, 104));
		_object3.show();
		_object3.animate(ANIM_MODE_5, this);
		break;
	case 3:
		R2_GLOBALS._sound1.play(241);
		_object4.postInit();
		_object4.setVisage(2800);
		_object4.setStrip(2);
		_object4._numFrames = 4;
		_object4._moveRate = 4;
		_object4.changeZoom(100);
		_object4.setPosition(Common::Point(300, 104));
		_object4.fixPriority(105);
		_object4.show();
		_object4.animate(ANIM_MODE_5, this);
		break;
	case 4:
		setDelay(18);
		_object4.setStrip(4);
		scene->_actor1.setVisage(2800);
		scene->_actor1.setStrip(5);
		scene->_actor1.setFrame(1);
		scene->_actor1._numFrames = 5;
		scene->_actor1._moveRate = 5;
		scene->_actor1.setPosition(Common::Point(300, 104));
		scene->_actor1.fixPriority(110);
		scene->_actor1.changeZoom(100);
		scene->_actor1.show();
		break;
	case 5:
		scene->_actor1.animate(ANIM_MODE_5, this);
		break;
	case 6: {
		scene->_actor1.changeZoom(-1);
		scene->_actor1.setVisage(3107);
		scene->_actor1.animate(ANIM_MODE_1, NULL);
		scene->_actor1.setStrip(3);
		scene->_actor1.setPosition(Common::Point(297, 140));
		scene->_actor1._numFrames = 10;
		scene->_actor1._moveRate = 10;
		scene->_actor1._moveDiff = Common::Point(3, 2);

		Common::Point pt(297, 160);
		NpcMover *mover = new NpcMover();
		scene->_actor1.addMover(mover, &pt, this);
		break;
		}
	case 7: {
		scene->_actor1.changeZoom(75);
		scene->_actor1.updateAngle(R2_GLOBALS._player._position);

		Common::Point pt(105, 82);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 8: {
		R2_GLOBALS._player._numFrames = 8;
		R2_GLOBALS._player._moveRate = 8;
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		R2_GLOBALS._player.setObjectWrapper(NULL);
		R2_GLOBALS._player.setStrip(2);
		R2_GLOBALS._player.changeZoom(-1);

		Common::Point pt(79, 100);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 9: {
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player._moveRate = 10;
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());

		Common::Point pt(100, 64);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 10: {
		R2_GLOBALS._player.fixPriority(124);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);

		Common::Point pt(160, 124);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 11: {
		R2_GLOBALS._player.fixPriority(-1);

		Common::Point pt(160, 160);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 12: {
		Common::Point pt(270, 160);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 13:
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		scene->_field412 = 1207;
		scene->_stripManager.start(scene->_field412, this);
		break;
	case 14: {
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.fixPriority(110);

		Common::Point pt(288, 140);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 15:
		setDelay(18);
		scene->_actor1.updateAngle(R2_GLOBALS._player._position);
		R2_GLOBALS._player.setVisage(2800);
		R2_GLOBALS._player.setStrip(6);
		R2_GLOBALS._player.setFrame(1);
		R2_GLOBALS._player.changeZoom(100);
		R2_GLOBALS._player.setPosition(Common::Point(300, 104));
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player._moveRate = 5;
		break;
	case 16:
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 17:
		setDelay(6);
		_object4.setStrip(2);
		_object4.setFrame(11);
		R2_GLOBALS._player.hide();
	// No break on purpose
	case 18:
		R2_GLOBALS._sound1.play(241);
		_object4.animate(ANIM_MODE_6, this);
		break;
	case 19:
		_object4.remove();
		_object3.animate(ANIM_MODE_6, this);
		break;
	case 20:
		setDelay(6);
		_object3.remove();
		_object2.setStrip(1);
		_object2.setFrame(19);
		break;
	case 21:
		setDelay(150);
		R2_GLOBALS._sound1.play(269);
		R2_GLOBALS._sound2.stop();
		break;
	case 22:
		scene->_sceneMode = 12;
		_object2.animate(ANIM_MODE_6, scene);
		break;
	default:
		break;
	}
}

void Scene2800::postInit(SceneObjectList *OwnerList) {
	loadScene(2800);
	setZoomPercents(100, 50, 124, 75);
	R2_GLOBALS._sound1.stop();
	R2_GLOBALS._sound2.stop();
	SceneExt::postInit();

	_object1.postInit();
	_object1.setup(2750, 4, 1);
	_object1.setPosition(Common::Point(-10, 25));
	_object1.animate(ANIM_MODE_1, NULL);
	_object1.setStrip2(4);
	_object1._moveRate = 20;
	_object1.setAction(&_action1);

	_actor3.postInit();
	_actor3.setup(2802, 1, 1);
	_actor3.setPosition(Common::Point(116, 80));
	_actor3.fixPriority(111);
	_actor3.animate(ANIM_MODE_2, NULL);
	_actor3._numFrames = 6;

	if (!R2_GLOBALS.getFlag(47)) {
		_actor1.postInit();
		_actor1.setVisage(3105);
		_actor1.setStrip(3);
		_actor1.setFrame(1);
		_actor1.setZoom(50);
		_actor1._moveDiff = Common::Point(2, 1);
		_actor1.setPosition(Common::Point(122, 82));
		_actor1.animate(ANIM_MODE_NONE, NULL);
		_actor1.setDetails(2800, -1, -1, -1, 1, (SceneItem *)NULL);
	}

	_item1.setDetails(Rect(0, 0, 320, 200), 2800, -1, -1, -1, 1, NULL);

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_nejSpeaker);
	_stripManager.addSpeaker(&_guardSpeaker);

	if (R2_INVENTORY.getObjectScene(36) == 0) {
		R2_GLOBALS._sound1.fadeSound(237);
		if (R2_GLOBALS.getFlag(47)) {
			_item2.setDetails(Rect(76, 45, 155, 90), 2800, 3, -1, -1, 2, NULL);
		} else {
			_actor2.postInit();
			_actor2.setup(2752, 5, 1);
			_actor2.animate(ANIM_MODE_NONE, NULL);
			_actor2.changeZoom(100);
			_actor2._moveDiff = Common::Point(2, 1);
			_actor2.setPosition(Common::Point(101, 148));
		}
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(19);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.changeZoom(100);
	R2_GLOBALS._player._moveDiff = Common::Point(2, 2);
	R2_GLOBALS._player.disableControl();

	if (R2_INVENTORY.getObjectScene(36) == 0) {
		R2_GLOBALS._player.setAction(&_sequenceManager, this, 2800, &R2_GLOBALS._player, NULL);
	} else if (R2_GLOBALS.getFlag(47)) {
		R2_GLOBALS._player.setVisage(3110);
		R2_GLOBALS._player.changeZoom(-1);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		R2_GLOBALS._player.setPosition(Common::Point(160, 124));
		R2_GLOBALS._player.enableControl();
	} else {
		_sceneMode = 2801;
		R2_GLOBALS._player.setAction(&_sequenceManager, this, 2801, &R2_GLOBALS._player, &_actor2, &_actor1, NULL);
	}
}

void Scene2800::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._sound1.play(238);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_field412 = 1206;
		_sceneMode = 2804;
		_stripManager.start(_field412, this);
		break;
	case 11:
		_actor2.remove();
		_object1.setAction(NULL);
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		_item2.setDetails(Rect(76, 45, 155, 90), 2800, 3, -1, -1, 2, NULL);
		break;
	case 12:
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_GLOBALS._sound2.fadeOut2(NULL);
		g_globals->_sceneManager.changeScene(1000);
		break;
	case 2800:
		g_globals->_sceneManager.changeScene(2750);
		break;
	case 2801:
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 2803:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 10;
		setAction(&_sequenceManager, this, 2803, &R2_GLOBALS._player, &_actor2, &_actor1, NULL);
		break;
	case 2804:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 11;
		setAction(&_sequenceManager, this, 2804, &R2_GLOBALS._player, &_actor2, NULL);
		break;
	case 2805:
		_object1.remove();
		setAction(&_action2);
		break;
	default:
		break;
	}
}

} // End of namespace Ringworld2
} // End of namespace TsAGE
