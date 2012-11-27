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

#include "common/endian.h"

#include "gob/gob.h"
#include "gob/goblin.h"
#include "gob/util.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/scenery.h"
#include "gob/sound/sound.h"

namespace Gob {

Goblin_v1::Goblin_v1(GobEngine *vm) : Goblin(vm) {
	_rotStates[0][0] = 0; _rotStates[0][1] = 22; _rotStates[0][2] = 23; _rotStates[0][3] = 24;
	_rotStates[1][0] = 13; _rotStates[1][1] = 2; _rotStates[1][2] = 12; _rotStates[1][3] = 14;
	_rotStates[2][0] = 16; _rotStates[2][1] = 15; _rotStates[2][2] = 4; _rotStates[2][3] = 17;
	_rotStates[3][0] = 27; _rotStates[3][1] = 25; _rotStates[3][2] = 26; _rotStates[3][3] = 6;
}

void Goblin_v1::freeObjects() {
	int16 state;
	int16 col;

	for (int i = 0; i < 16; i++)
		_vm->_sound->sampleFree(&_soundData[i]);

	for (int i = 0; i < 4; i++) {
		if (_goblins[i] == 0)
			continue;

		_goblins[i]->stateMach = _goblins[i]->realStateMach;

		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				delete _goblins[i]->stateMach[state][col];
				_goblins[i]->stateMach[state][col] = 0;
			}
		}

		if (i == 3) {
			for (state = 40; state < 70; state++) {
				delete _goblins[3]->stateMach[state][0];
				_goblins[3]->stateMach[state][0] = 0;
			}
		}

		delete[] _goblins[i]->stateMach;
		delete _goblins[i];
		_goblins[i] = 0;
	}

	for (int i = 0; i < 20; i++) {
		if (_objects[i] == 0)
			continue;

		_objects[i]->stateMach = _objects[i]->realStateMach;

		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				delete _objects[i]->stateMach[state][col];
				_objects[i]->stateMach[state][col] = 0;
			}
		}

		delete[] _objects[i]->stateMach;
		delete _objects[i];
		_objects[i] = 0;
	}
}

void Goblin_v1::placeObject(Gob_Object *objDesc, char animated,
		int16 index, int16 x, int16 y, int16 state) {
	int16 layer;

	if (objDesc->stateMach[objDesc->state][0] != 0) {
		objDesc->animation = objDesc->stateMach[objDesc->state][0]->animation;

		objDesc->noTick = 0;
		objDesc->toRedraw = 1;
		objDesc->doAnim = animated;

		objDesc->maxTick = 1;
		objDesc->tick = 1;
		objDesc->curFrame = 0;
		objDesc->type = 0;
		objDesc->actionStartState = 0;
		objDesc->nextState = -1;
		objDesc->multState = -1;
		objDesc->stateColumn = 0;
		objDesc->curLookDir = 0;
		objDesc->visible = 1;
		objDesc->pickable = 0;
		objDesc->unk14 = 0;

		objDesc->relaxTime = _vm->_util->getRandom(30);

		layer = objDesc->stateMach[objDesc->state][0]->layer;
		_vm->_scenery->updateAnim(layer, 0, objDesc->animation, 0,
		    objDesc->xPos, objDesc->yPos, 0);

		objDesc->order = _vm->_scenery->_toRedrawBottom / 24 + 3;

		objDesc->left = objDesc->xPos;
		objDesc->right = objDesc->xPos;
		objDesc->dirtyLeft = objDesc->xPos;
		objDesc->dirtyRight = objDesc->xPos;

		objDesc->top = objDesc->yPos;
		objDesc->bottom = objDesc->yPos;
		objDesc->dirtyTop = objDesc->yPos;
		objDesc->dirtyBottom = objDesc->yPos;

		_vm->_util->listInsertBack(_objList, objDesc);
	}
}

void Goblin_v1::initiateMove(Mult::Mult_Object *obj) {
	_vm->_map->findNearestToDest(0);
	_vm->_map->findNearestToGob(0);
	_vm->_map->optimizePoints(0, 0, 0);

	_pathExistence = _vm->_map->checkDirectPath(0,
			_vm->_map->_curGoblinX, _vm->_map->_curGoblinY,
	    _pressedMapX, _pressedMapY);

	if (_pathExistence == 3) {
		if (_vm->_map->checkLongPath(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY,
			_pressedMapX, _pressedMapY,
			_vm->_map->_nearestWayPoint, _vm->_map->_nearestDest) == 0) {
			_pathExistence = 0;
		} else {
			const WayPoint &wayPoint = _vm->_map->getWayPoint(_vm->_map->_nearestWayPoint);

			_vm->_map->_destX = wayPoint.x;
			_vm->_map->_destY = wayPoint.y;
		}
	}
}

void Goblin_v1::movePathFind(Mult::Mult_Object *obj,
		Gob_Object *gobDesc, int16 nextAct) {

	if (_pathExistence == 1) {
		_vm->_map->_curGoblinX = _gobPositions[_currentGoblin].x;
		_vm->_map->_curGoblinY = _gobPositions[_currentGoblin].y;

		if ((_vm->_map->_curGoblinX == _pressedMapX) &&
		    (_vm->_map->_curGoblinY == _pressedMapY) && (_gobAction != 0)) {
			_readyToAct = 1;
			_pathExistence = 0;
		}

		nextAct = (int16) _vm->_map->getDirection(_vm->_map->_curGoblinX,
				_vm->_map->_curGoblinY, _vm->_map->_destX, _vm->_map->_destY);

		if (nextAct == kDirNone)
			_pathExistence = 0;
	} else if (_pathExistence == 3) {
		_vm->_map->_curGoblinX = _gobPositions[_currentGoblin].x;
		_vm->_map->_curGoblinY = _gobPositions[_currentGoblin].y;

		if ((_vm->_map->_curGoblinX == _gobDestX) &&
				(_vm->_map->_curGoblinY == _gobDestY)) {
			_pathExistence = 1;
			_vm->_map->_destX = _pressedMapX;
			_vm->_map->_destY = _pressedMapY;
		} else {

			if (_vm->_map->checkDirectPath(0, _vm->_map->_curGoblinX,
						_vm->_map->_curGoblinY, _gobDestX, _gobDestY) == 1) {
				_vm->_map->_destX = _gobDestX;
				_vm->_map->_destY = _gobDestY;
			} else if ((_vm->_map->_curGoblinX == _vm->_map->_destX) &&
					(_vm->_map->_curGoblinY == _vm->_map->_destY)) {

				if (_vm->_map->_nearestWayPoint > _vm->_map->_nearestDest) {
					_vm->_map->optimizePoints(0, 0, 0);

					const WayPoint &wayPoint = _vm->_map->getWayPoint(_vm->_map->_nearestWayPoint);

					_vm->_map->_destX = wayPoint.x;
					_vm->_map->_destY = wayPoint.y;

					if (_vm->_map->_nearestWayPoint > _vm->_map->_nearestDest)
						_vm->_map->_nearestWayPoint--;
				} else if (_vm->_map->_nearestWayPoint < _vm->_map->_nearestDest) {
					_vm->_map->optimizePoints(0, 0, 0);

					const WayPoint &wayPoint = _vm->_map->getWayPoint(_vm->_map->_nearestWayPoint);

					_vm->_map->_destX = wayPoint.x;
					_vm->_map->_destY = wayPoint.y;

					if (_vm->_map->_nearestWayPoint < _vm->_map->_nearestDest)
						_vm->_map->_nearestWayPoint++;
				} else {
					if ((_vm->_map->checkDirectPath(0, _vm->_map->_curGoblinX,
						_vm->_map->_curGoblinY, _gobDestX, _gobDestY) == 3) &&
							(_vm->_map->getPass(_pressedMapX, _pressedMapY) != 0)) {

						const WayPoint &wayPoint = _vm->_map->getWayPoint(_vm->_map->_nearestWayPoint);

						_vm->_map->_destX = wayPoint.x;
						_vm->_map->_destY = wayPoint.y;

					} else {
						_pathExistence = 1;
						_vm->_map->_destX = _pressedMapX;
						_vm->_map->_destY = _pressedMapY;
					}
				}
			}
			nextAct = (int16) _vm->_map->getDirection(_vm->_map->_curGoblinX,
					_vm->_map->_curGoblinY, _vm->_map->_destX, _vm->_map->_destY);
		}
	}

	if ((_readyToAct != 0) && ((_gobAction == 3) || (_gobAction == 4)))
		nextAct = 0x4DC8;

	switch (nextAct) {
	case kDirW:
		gobDesc->nextState = rotateState(gobDesc->curLookDir, 0);
		break;

	case kDirE:
		gobDesc->nextState = rotateState(gobDesc->curLookDir, 4);
		break;

	case 16:
		gobDesc->nextState = 16;
		break;

	case 23:
		gobDesc->nextState = 23;
		break;

	case kDirN:
		if ((_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY - 1) == 6) &&
		    (_currentGoblin != 1)) {
			_pathExistence = 0;
			break;
		}

		if (_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 3) {
			gobDesc->nextState = 8;
			break;
		}

		if ((_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 6) &&
		    (_currentGoblin == 1)) {
			gobDesc->nextState = 28;
			break;
		}

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 2);
		break;

	case kDirS:
		if ((_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY + 1) == 6) &&
		    (_currentGoblin != 1)) {
			_pathExistence = 0;
			break;
		}

		if (_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 3) {
			gobDesc->nextState = 9;
			break;
		}

		if ((_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 6) &&
		    (_currentGoblin == 1)) {
			gobDesc->nextState = 29;
			break;
		}

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 6);
		break;

	case kDirSE:
		if ((_vm->_map->getPass(_vm->_map->_curGoblinX + 1, _vm->_map->_curGoblinY + 1) == 6) &&
		    (_currentGoblin != 1)) {
			_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 5;
		if (gobDesc->curLookDir == 4)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 4);
		break;

	case kDirSW:
		if ((_vm->_map->getPass(_vm->_map->_curGoblinX - 1, _vm->_map->_curGoblinY + 1) == 6) &&
		    (_currentGoblin != 1)) {
			_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 7;
		if (gobDesc->curLookDir == 0)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 0);
		break;

	case kDirNW:
		if ((_vm->_map->getPass(_vm->_map->_curGoblinX - 1, _vm->_map->_curGoblinY - 1) == 6) &&
		    (_currentGoblin != 1)) {
			_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 1;
		if (gobDesc->curLookDir == 0)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 0);
		break;

	case kDirNE:
		if ((_vm->_map->getPass(_vm->_map->_curGoblinX + 1, _vm->_map->_curGoblinY - 1) == 6) &&
		    (_currentGoblin != 1)) {
			_pathExistence = 0;
			break;
		}

		gobDesc->nextState = 3;
		if (gobDesc->curLookDir == 4)
			break;

		gobDesc->nextState = rotateState(gobDesc->curLookDir, 4);
		break;

	case 0x4DC8:

		if ((_currentGoblin == 0) && (_gobAction == 3) &&
				(_itemIndInPocket == -1)) {
			_destItemId = -1;
			_readyToAct = 0;
			break;
		}

		if ((_currentGoblin == 0) && (_gobAction == 4) &&
				(_itemIndInPocket == -1) && (_destActionItem == 0)) {
			gobDesc->multState = 104;
			_destItemId = -1;
			_readyToAct = 0;
			break;
		}

		if ((_currentGoblin == 0) && (_gobAction == 4) &&
		    (_itemIndInPocket == -1 && _destActionItem != 0) &&
		    (_itemToObject[_destActionItem] != -1) &&
		    (_objects[_itemToObject[_destActionItem]]->pickable == 0)) {
			gobDesc->multState = 104;
			_destItemId = -1;
			_readyToAct = 0;
			break;
		}

		switch (_vm->_map->_itemPoses[_destActionItem].orient) {
		case 0:
		case -4:
			gobDesc->nextState = 10;
			gobDesc->curLookDir = 0;
			_destItemId = -1;
			break;

		case -1:
		case 4:
			gobDesc->nextState = 11;
			gobDesc->curLookDir = 4;
			_destItemId = -1;
			break;
		}
		break;

	default:
		if ((_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 3) ||
		    ((_vm->_map->getPass(_vm->_map->_curGoblinX, _vm->_map->_curGoblinY) == 6)
			&& (_currentGoblin == 1))) {
			gobDesc->nextState = 20;
			break;
		}

		switch (gobDesc->curLookDir) {
		case 2:
		case 4:
			gobDesc->nextState = 18;
			break;

		case 6:
		case 0:
			gobDesc->nextState = 19;
			break;
		}
		break;
	}
	return;
}

void Goblin_v1::moveAdvance(Mult::Mult_Object *obj, Gob_Object *gobDesc,
		int16 nextAct, int16 framesCount) {
	int16 i;
	int16 newX;
	int16 newY;
	int16 flag;

	movePathFind(0, gobDesc, nextAct);

	gobDesc->curFrame++;
	if (gobDesc->curFrame == 1)
		gobDesc->actionStartState = gobDesc->state;

	if ((_goesAtTarget == 0) && (gobDesc->stateMach == gobDesc->realStateMach)) {
		switch (gobDesc->state) {
		case 0:
		case 1:
		case 7:
		case 13:
		case 16:
		case 27:
			gobDesc->curLookDir = 0;
			break;

		case 3:
		case 4:
		case 5:
		case 12:
		case 23:
		case 26:
			gobDesc->curLookDir = 4;
			break;

		case 28:
			if (_currentGoblin != 1)
				break;
			gobDesc->curLookDir = 2;
			break;

		case 2:
		case 8:
		case 15:
		case 22:
		case 25:
			gobDesc->curLookDir = 2;
			break;

		case 29:
			if (_currentGoblin != 1)
				break;

			gobDesc->curLookDir = 6;
			break;

		case 6:
		case 9:
		case 14:
		case 17:
		case 24:
			gobDesc->curLookDir = 6;
			break;
		}
	}

	if ((gobDesc->state >= 0) && (gobDesc->state < 10) &&
	    (gobDesc->stateMach == gobDesc->realStateMach) &&
	    ((gobDesc->curFrame == 3) || (gobDesc->curFrame == 6))) {
		_vm->_sound->speakerOn(10 * _vm->_util->getRandom(3) + 50, 5);
	}

	if ((_currentGoblin == 0) &&
			(gobDesc->stateMach == gobDesc->realStateMach) &&
			((gobDesc->state == 10) || (gobDesc->state == 11)) &&
			(gobDesc->curFrame == 9)) {
		_vm->_sound->blasterStop(0);

		if (_itemIndInPocket != -1)
			_vm->_sound->blasterPlay(&_soundData[14], 1, 9000);
		else
			_vm->_sound->blasterPlay(&_soundData[14], 1, 5000);
	}

	if (_boreCounter++ == 120) {
		_boreCounter = 0;
		for (i = 0; i < 3; i++)
			showBoredom(i);
	}

	if ((gobDesc->multState != -1) && (gobDesc->curFrame == framesCount) &&
	    (gobDesc->state != gobDesc->multState)) {
		gobDesc->nextState = gobDesc->multState;
		gobDesc->multState = -1;

		newX = _vm->_scenery->getAnimLayer(gobDesc->animation,
				_gobStateLayer)->animDeltaX + gobDesc->xPos;

		newY = _vm->_scenery->getAnimLayer(gobDesc->animation,
				_gobStateLayer)->animDeltaY + gobDesc->yPos;

		_gobStateLayer = nextLayer(gobDesc);

		gobDesc->xPos = newX;
		gobDesc->yPos = newY;
	} else {
		if ((gobDesc->curFrame == 3) &&
		    (gobDesc->stateMach == gobDesc->realStateMach) &&
		    ((gobDesc->state < 10) ||
			((_currentGoblin == 1) && ((gobDesc->state == 28) ||
				(gobDesc->state == 29))))) {
			flag = 0;
			if (_forceNextState[0] != -1) {
				gobDesc->nextState = _forceNextState[0];
				for (i = 0; i < 9; i++)
					_forceNextState[i] = _forceNextState[i + 1];
			}

			_vm->_map->_curGoblinX = _gobPositions[_currentGoblin].x;
			_vm->_map->_curGoblinY = _gobPositions[_currentGoblin].y;

			if (gobDesc->nextState != gobDesc->state) {
				_gobStateLayer = nextLayer(gobDesc);
				flag = 1;
			}

			switch (gobDesc->state) {
			case 0:
				_gobPositions[_currentGoblin].x--;
				break;

			case 2:
			case 8:
				_gobPositions[_currentGoblin].y--;
				break;

			case 4:
				_gobPositions[_currentGoblin].x++;
				break;

			case 6:
			case 9:
				_gobPositions[_currentGoblin].y++;
				break;

			case 1:
				_gobPositions[_currentGoblin].x--;
				_gobPositions[_currentGoblin].y--;
				break;

			case 3:
				_gobPositions[_currentGoblin].x++;
				_gobPositions[_currentGoblin].y--;
				break;

			case 5:
				_gobPositions[_currentGoblin].x++;
				_gobPositions[_currentGoblin].y++;
				break;

			case 7:
				_gobPositions[_currentGoblin].x--;
				_gobPositions[_currentGoblin].y++;
				break;

			case 38:
				_gobPositions[_currentGoblin].y++;
				break;
			}

			if (_currentGoblin == 1) {
				if (gobDesc->state == 28)
					_gobPositions[1].y--;

				if (gobDesc->state == 29)
					_gobPositions[1].y++;
			}

			if (flag != 0) {
				_vm->_scenery->updateAnim(_gobStateLayer, 0,
						gobDesc->animation, 0, gobDesc->xPos, gobDesc->yPos, 0);

				gobDesc->yPos =
				    (_vm->_map->_curGoblinY + 1) * 6 -
				    (_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
				gobDesc->xPos =
				    _vm->_map->_curGoblinX * 12 - (_vm->_scenery->_toRedrawLeft -
				    _vm->_scenery->_animLeft);
			}

			if (((gobDesc->state == 10) || (gobDesc->state == 11)) &&
					(_currentGoblin != 0))
				_goesAtTarget = 1;
		}

		if (gobDesc->curFrame != framesCount)
			return;

		if (_forceNextState[0] != -1) {
			gobDesc->nextState = _forceNextState[0];
			for (i = 0; i < 9; i++)
				_forceNextState[i] = _forceNextState[i + 1];
		}

		_vm->_map->_curGoblinX = _gobPositions[_currentGoblin].x;
		_vm->_map->_curGoblinY = _gobPositions[_currentGoblin].y;

		_gobStateLayer = nextLayer(gobDesc);
		if (gobDesc->stateMach == gobDesc->realStateMach) {

			switch (gobDesc->nextState) {
			case 0:
				_gobPositions[_currentGoblin].x--;
				break;

			case 2:
			case 8:
				_gobPositions[_currentGoblin].y--;
				break;

			case 4:
				_gobPositions[_currentGoblin].x++;
				break;

			case 6:
			case 9:
				_gobPositions[_currentGoblin].y++;
				break;

			case 1:
				_gobPositions[_currentGoblin].x--;
				_gobPositions[_currentGoblin].y--;
				break;

			case 3:
				_gobPositions[_currentGoblin].x++;
				_gobPositions[_currentGoblin].y--;
				break;

			case 5:
				_gobPositions[_currentGoblin].x++;
				_gobPositions[_currentGoblin].y++;
				break;

			case 7:
				_gobPositions[_currentGoblin].x--;
				_gobPositions[_currentGoblin].y++;
				break;

			case 38:
				_gobPositions[_currentGoblin].y++;
				break;
			}

			if (_currentGoblin == 1) {
				if (gobDesc->nextState == 28)
					_gobPositions[1].y--;

				if (gobDesc->nextState == 29)
					_gobPositions[1].y++;
			}
		}

		_vm->_scenery->updateAnim(_gobStateLayer, 0, gobDesc->animation, 0,
		    gobDesc->xPos, gobDesc->yPos, 0);

		gobDesc->yPos =
		    (_vm->_map->_curGoblinY + 1) * 6 - (_vm->_scenery->_toRedrawBottom -
		    _vm->_scenery->_animTop);
		gobDesc->xPos =
		    _vm->_map->_curGoblinX * 12 -
				(_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);

		if (((gobDesc->state == 10) || (gobDesc->state == 11)) &&
				(_currentGoblin != 0))
			_goesAtTarget = 1;
	}
	return;
}

} // End of namespace Gob
