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
#include "gob/global.h"
#include "gob/util.h"
#include "gob/game.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/scenery.h"
#include "gob/inter.h"

namespace Gob {

Goblin_v2::Goblin_v2(GobEngine *vm) : Goblin_v1(vm) {
	_gobsCount = -1;
	_rotStates[0][0] =  0; _rotStates[0][1] = 18; _rotStates[0][2] = 19; _rotStates[0][3] = 20;
	_rotStates[1][0] = 13; _rotStates[1][1] =  2; _rotStates[1][2] = 12; _rotStates[1][3] = 14;
	_rotStates[2][0] = 16; _rotStates[2][1] = 15; _rotStates[2][2] =  4; _rotStates[2][3] = 17;
	_rotStates[3][0] = 23; _rotStates[3][1] = 21; _rotStates[3][2] = 22; _rotStates[3][3] =  6;
}

void Goblin_v2::freeObjects() {
	_vm->_map->_mapUnknownBool = false;

	if (_gobsCount < 0)
		return;

	for (int i = 0; i < _gobsCount; i++) {
		delete[] _vm->_mult->_objects[i].goblinStates[0];
		delete[] _vm->_mult->_objects[i].goblinStates;
	}
	for (int i = 0; i < _soundSlotsCount; i++)
		if ((_soundSlots[i] & 0x8000) == 0)
			_vm->_game->freeSoundSlot(_soundSlots[i]);
	_gobsCount = -1;
}

void Goblin_v2::placeObject(Gob_Object *objDesc, char animated,
		int16 index, int16 x, int16 y, int16 state) {
	Mult::Mult_Object *obj;
	Mult::Mult_AnimData *objAnim;
	int16 layer;
	int16 animation;

	obj = &_vm->_mult->_objects[index];
	objAnim = obj->pAnimData;

	obj->goblinX = x;
	obj->goblinY = y;
	objAnim->order = y;

	if (state == -1) {
		objAnim->frame = 0;
		objAnim->isPaused = 0;
		objAnim->isStatic = 0;
		objAnim->newCycle = 0;
		_vm->_scenery->updateAnim(objAnim->layer, 0, objAnim->animation, 0,
				*obj->pPosX, *obj->pPosY, 0);
		if (!_vm->_map->hasBigTiles())
			*obj->pPosY = (y + 1) * _vm->_map->getTilesHeight()
				- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
		else
			*obj->pPosY = ((y + 1) * _vm->_map->getTilesHeight()) -
				(_vm->_scenery->_animBottom - _vm->_scenery->_animTop) - (y + 1) / 2;
		*obj->pPosX = x * _vm->_map->getTilesWidth();
	} else {
		if ((obj->goblinStates != 0) && (obj->goblinStates[state] != 0)) {
			layer = obj->goblinStates[state][0].layer;
			animation = obj->goblinStates[state][0].animation;
			objAnim->state = state;
			objAnim->layer = layer;
			objAnim->animation = animation;
			objAnim->frame = 0;
			objAnim->isPaused = 0;
			objAnim->isStatic = 0;
			objAnim->newCycle = _vm->_scenery->getAnimLayer(animation, layer)->framesCount;
			_vm->_scenery->updateAnim(layer, 0, animation, 0, *obj->pPosX, *obj->pPosY, 0);
			if (!_vm->_map->hasBigTiles())
				*obj->pPosY = (y + 1) * _vm->_map->getTilesHeight()
					- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
			else
				*obj->pPosY = ((y + 1) * _vm->_map->getTilesHeight()) -
					(_vm->_scenery->_animBottom - _vm->_scenery->_animTop) - (y + 1) / 2;
			*obj->pPosX = x * _vm->_map->getTilesWidth();
			initiateMove(obj);
		} else
			initiateMove(obj);
	}
}

void Goblin_v2::initiateMove(Mult::Mult_Object *obj) {
	obj->destX = obj->gobDestX;
	obj->destY = obj->gobDestY;
	_vm->_map->findNearestToDest(obj);
	_vm->_map->findNearestToGob(obj);
	_vm->_map->optimizePoints(obj, obj->goblinX, obj->goblinY);
	obj->pAnimData->pathExistence = _vm->_map->checkDirectPath(obj,
			obj->goblinX, obj->goblinY, obj->gobDestX, obj->gobDestY);
	if (obj->pAnimData->pathExistence == 3) {
		const WayPoint &wayPoint = _vm->_map->getWayPoint(obj->nearestWayPoint);

		obj->destX = wayPoint.x;
		obj->destY = wayPoint.y;
	}
}

void Goblin_v2::movePathFind(Mult::Mult_Object *obj, Gob_Object *gobDesc, int16 nextAct) {
	Mult::Mult_AnimData *animData = obj->pAnimData;

	animData->newCycle = _vm->_scenery->getAnimLayer(animData->animation, animData->layer)->framesCount;

	int16 gobX     = obj->goblinX;
	int16 gobY     = obj->goblinY;
	int16 destX    = obj->destX;
	int16 destY    = obj->destY;
	int16 gobDestX = obj->gobDestX;
	int16 gobDestY = obj->gobDestY;

	animData->destX = gobDestX;
	animData->destY = gobDestY;
	animData->order = gobY;

	Direction dir = kDirNone;

	if (animData->pathExistence == 1) {

		dir = _vm->_map->getDirection(gobX, gobY, destX, destY);
		if (dir == kDirNone)
			animData->pathExistence = 0;
		if ((gobX == gobDestX) && (gobY == gobDestY))
			animData->pathExistence = 4;

	} else if (animData->pathExistence == 3) {

		if ((gobX != gobDestX) || (gobY != gobDestY)) {

			if (_vm->_map->checkDirectPath(obj, gobX, gobY, gobDestX, gobDestY) != 1) {

				if ((gobX == destX) && (gobY == destY)) {

					if (obj->nearestWayPoint > obj->nearestDest) {
						_vm->_map->optimizePoints(obj, gobX, gobY);

						const WayPoint &wayPoint = _vm->_map->getWayPoint(obj->nearestWayPoint);

						destX = wayPoint.x;
						destY = wayPoint.y;

						if (_vm->_map->checkDirectPath(obj, gobX, gobY, destX, destY) == 3) {
							WRITE_VAR(56, 1);
							animData->pathExistence = 0;
						}
						if (obj->nearestWayPoint > obj->nearestDest)
							obj->nearestWayPoint--;
					} else if (obj->nearestWayPoint < obj->nearestDest) {
						_vm->_map->optimizePoints(obj, gobX, gobY);

						const WayPoint &wayPoint = _vm->_map->getWayPoint(obj->nearestWayPoint);

						destX = wayPoint.x;
						destY = wayPoint.y;

						if (_vm->_map->checkDirectPath(obj, gobX, gobY, destX, destY) == 3) {
							WRITE_VAR(56, 1);
							animData->pathExistence = 0;
						}
						if (obj->nearestWayPoint < obj->nearestDest)
							obj->nearestWayPoint++;
					} else {
						if ((_vm->_map->checkDirectPath(obj, gobX, gobY, gobDestX, gobDestY) == 3) &&
								(_vm->_map->getPass(gobDestX, gobDestY) != 0)) {

							const WayPoint &wayPoint = _vm->_map->getWayPoint(obj->nearestWayPoint);

							destX = wayPoint.x;
							destY = wayPoint.y;

							WRITE_VAR(56, 1);
						} else {
							animData->pathExistence = 1;
							destX = gobDestX;
							destY = gobDestY;
						}
					}

				}

			} else {
				destX = gobDestX;
				destY = gobDestY;
			}

			dir = _vm->_map->getDirection(gobX, gobY, destX, destY);

		} else {
			animData->pathExistence = 4;
			destX = gobDestX;
			destY = gobDestY;
		}

	}

	obj->goblinX   = gobX;
	obj->goblinY   = gobY;
	obj->destX     = destX;
	obj->destY     = destY;
	obj->gobDestX  = gobDestX;
	obj->gobDestY  = gobDestY;

	switch (dir) {
	case kDirNW:
		animData->nextState = 1;
		if (_vm->_map->getScreenWidth() == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
				animData->nextState = 40;
			if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY - 2) != 10)
				animData->nextState = 1;
		}
		break;

	case kDirN:
		animData->nextState =
			(animData->curLookDir == 2) ? 2 : rotateState(animData->curLookDir, 2);
		if (_vm->_map->getScreenWidth() == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10) {
				if      (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY - 2) == 10)
					animData->nextState = 40;
				else if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY - 2) == 10)
					animData->nextState = 42;
				else
					animData->nextState = 2;
			}

			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 20)
				animData->nextState = 38;
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 19)
				animData->nextState = 26;
		}
		break;

	case kDirNE:
		animData->nextState = 3;
		if (_vm->_map->getScreenWidth() == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
				animData->nextState = 42;
			if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY - 2) != 10)
				animData->nextState = 3;
		}
		break;

	case kDirW:
		animData->nextState = rotateState(animData->curLookDir, 0);
		break;

	case kDirE:
		animData->nextState = rotateState(animData->curLookDir, 4);
		break;

	case kDirSW:
		animData->nextState = 7;
		if (_vm->_map->getScreenWidth() == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
				animData->nextState = 41;
			if (_vm->_map->getPass(obj->goblinX - 1, obj->goblinY + 2) != 10)
				animData->nextState = 7;
		}
		break;

	case kDirS:
		animData->nextState =
			(animData->curLookDir == 6) ? 6 : rotateState(animData->curLookDir, 6);
		if (_vm->_map->getScreenWidth() == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 20)
				animData->nextState = 39;
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 19)
				animData->nextState = 27;
		}
		break;

	case kDirSE:
		animData->nextState = 5;
		if (_vm->_map->getScreenWidth() == 640) {
			if (_vm->_map->getPass(obj->goblinX, obj->goblinY) == 10)
				animData->nextState = 43;
			if (_vm->_map->getPass(obj->goblinX + 1, obj->goblinY + 2) != 10)
				animData->nextState = 5;
		}
		break;

	default:
		if      (animData->curLookDir == 0)
			animData->nextState = 8;
		else if (animData->curLookDir == 2)
			animData->nextState = 29;
		else if (animData->curLookDir == 4)
			animData->nextState = 9;
		else if (animData->curLookDir == 6)
			animData->nextState = 28;
		break;
	}
}

void Goblin_v2::moveAdvance(Mult::Mult_Object *obj, Gob_Object *gobDesc,
		int16 nextAct, int16 framesCount) {

	if (!obj->goblinStates)
		return;

	movePathFind(obj, 0, 0);
	playSounds(obj);

	Mult::Mult_AnimData *animData = obj->pAnimData;

	framesCount = _vm->_scenery->getAnimLayer(animData->animation, animData->layer)->framesCount;

	if (animData->isPaused == 0)
		animData->frame++;

	switch (animData->stateType) {
	case 0:
	case 1:
		animData->isPaused = 0;
		break;

	case 4:
		if (animData->frame == 0)
			animData->isPaused = 1;
		break;

	case 6:
		if (animData->frame >= framesCount)
			animData->isPaused = 1;
		break;
	}

	switch (animData->state) {
	case 0:
	case 1:
	case 7:
	case 13:
	case 16:
	case 23:
	case 40:
	case 41:
		animData->curLookDir = 0;
		break;

	case 2:
	case 15:
	case 18:
	case 21:
	case 26:
	case 38:
		animData->curLookDir = 2;
		break;

	case 3:
	case 4:
	case 5:
	case 12:
	case 19:
	case 22:
	case 42:
	case 43:
		animData->curLookDir = 4;
		break;

	case 6:
	case 14:
	case 17:
	case 20:
	case 27:
	case 39:
		animData->curLookDir = 6;
		break;

	case 8:
	case 9:
	case 28:
	case 29:
		if (animData->pathExistence == 4)
			animData->pathExistence = 5;
		break;
	}

	if ((animData->newState != -1) && (animData->frame == framesCount) &&
	    (animData->newState != animData->state)) {

		animData->nextState = animData->newState;
		animData->newState  = -1;
		animData->state     = animData->nextState;

		Scenery::AnimLayer *animLayer =
			_vm->_scenery->getAnimLayer(animData->animation, animData->layer);

		*obj->pPosX += animLayer->animDeltaX;
		*obj->pPosY += animLayer->animDeltaY;

		int16 animation = obj->goblinStates[animData->nextState][0].animation;
		int16 layer     = obj->goblinStates[animData->nextState][0].layer;

		animData->layer     = layer;
		animData->animation = animation;
		animData->frame     = 0;

		return;
	}

	if (isMovement(animData->state)) {
		int16 state = animData->nextState;

		if (animData->frame == ((framesCount + 1) / 2)) {
			int16 gobX = obj->goblinX;
			int16 gobY = obj->goblinY + 1;

			advMovement(obj, state);

			if (animData->state != state) {
				int16 animation = obj->goblinStates[state][0].animation;
				int16 layer     = obj->goblinStates[state][0].layer;

				animData->layer     = layer;
				animData->animation = animation;
				animData->frame     = 0;
				animData->state     = state;

				_vm->_scenery->updateAnim(layer, 0, animation, 0, *obj->pPosX, *obj->pPosY, 0);
				uint32 gobPosX =  gobX * _vm->_map->getTilesWidth();
				uint32 gobPosY = (gobY * _vm->_map->getTilesHeight()) -
				                 (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);

				if (_vm->_map->hasBigTiles())
					gobPosY -= gobY / 2;

				*obj->pPosX = gobPosX;
				*obj->pPosY = gobPosY;
			}
		}
	}

	if (animData->frame < framesCount)
		return;

	int16 state     = animData->nextState;
	int16 animation = obj->goblinStates[state][0].animation;
	int16 layer     = obj->goblinStates[state][0].layer;

	animData->layer     = layer;
	animData->animation = animation;
	animData->frame     = 0;
	animData->state     = state;

	int16 gobX = obj->goblinX;
	int16 gobY = obj->goblinY + 1;

	advMovement(obj, state);

	_vm->_scenery->updateAnim(layer, 0, animation, 0, *obj->pPosX, *obj->pPosY, 0);
	uint32 gobPosX =  gobX * _vm->_map->getTilesWidth();
	uint32 gobPosY = (gobY * _vm->_map->getTilesHeight()) -
	                 (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);

	if (_vm->_map->hasBigTiles())
		gobPosY -= gobY / 2;

	*obj->pPosX = gobPosX;
	*obj->pPosY = gobPosY;
}

void Goblin_v2::handleGoblins() {
	Mult::Mult_Object *obj0, *obj1;
	Mult::Mult_AnimData *anim0, *anim1;
	int16 pass;
	int16 gob1State, gob2State;
	int16 gob1X, gob2X;
	int16 gob1Y, gob2Y;
	int16 gob1DestX, gob2DestX;
	int16 gob1DestY, gob2DestY;

	obj0 = &_vm->_mult->_objects[0];
	obj1 = &_vm->_mult->_objects[1];
	anim0 = obj0->pAnimData;
	anim1 = obj1->pAnimData;

	gob1State = anim0->state;
	gob2State = anim1->state;

	if (!anim0->isBusy) {
		if (!_gob1Busy && (anim0->isStatic == 0)) {
			if ((VAR(_gob1RelaxTimeVar) == 0) && (gob1State == 28)) {
				// Goblin 1 showing boredom
				gob1State = _vm->_util->getRandom(3) + 24;
				setState(0, gob1State);
				WRITE_VAR(_gob1RelaxTimeVar, 100);
			} else
				WRITE_VAR(_gob1RelaxTimeVar, VAR(_gob1RelaxTimeVar) - 1);
		}
		if ((gob1State == 8) || (gob1State == 9) || (gob1State == 29))
			anim0->curLookDir = 6;
	}
	if (!anim1->isBusy) {
		if (!_gob2Busy && (anim1->isStatic == 0)) {
			if ((VAR(_gob2RelaxTimeVar) == 0) && (gob2State == 28)) {
				// Goblin 2 showing boredom
				gob2State = _vm->_util->getRandom(3) + 24;
				setState(1, gob2State);
				WRITE_VAR(_gob2RelaxTimeVar, 100);
			} else
				WRITE_VAR(_gob2RelaxTimeVar, VAR(_gob2RelaxTimeVar) - 1);
		}
		if ((gob2State == 8) || (gob2State == 9) || (gob2State == 29))
			anim1->curLookDir = 6;
	}

	if ((anim0->isBusy == 1) && (anim0->isStatic == 0) &&
			((anim0->state == 28) || (anim0->state == 29)))
		anim0->curLookDir = 0;
	if ((anim1->isBusy == 1) && (anim1->isStatic == 0) &&
			((anim1->state == 28) || (anim1->state == 29)))
		anim1->curLookDir = 0;

	if (VAR(18) != ((uint32) -1)) {
		if (anim0->layer == 44)
			anim0->curLookDir = 4;
		else if (anim0->layer == 45)
			anim0->curLookDir = 0;
		if (anim0->isBusy == 0)
			anim0->curLookDir = 6;
	}
	if (VAR(19) != ((uint32) -1)) {
		if (anim1->layer == 48)
			anim1->curLookDir = 4;
		else if (anim1->layer == 49)
			anim1->curLookDir = 0;
		if (anim1->isBusy == 0)
			anim1->curLookDir = 6;
	}

	if ((anim0->layer == 45) && (anim0->curLookDir == 4) && (anim0->pathExistence == 5) &&
			(VAR(18) == ((uint32) -1)) && !_gob1NoTurn) {
		setState(0, 19); // Turning right->left
	}
	if ((anim0->layer == 44) && (anim0->curLookDir == 0) && (anim0->pathExistence == 5) &&
			(VAR(18) == ((uint32) -1)) && !_gob1NoTurn) {
		setState(0, 16); // Turning left->right
	}
	if ((anim1->layer == 49) && (anim1->curLookDir == 4) && (anim1->pathExistence == 5) &&
			(VAR(19) == ((uint32) -1)) && !_gob2NoTurn) {
		setState(1, 19); // Turning right->left
	}
	if ((anim1->layer == 48) && (anim1->curLookDir == 0) && (anim1->pathExistence == 5) &&
			(VAR(19) == ((uint32) -1)) && !_gob2NoTurn) {
		setState(1, 16); // Turning left->right
	}

	gob1X = obj0->goblinX;
	gob2X = obj1->goblinX;
	gob1Y = obj0->goblinY;
	gob2Y = obj1->goblinY;
	gob1DestX = anim0->destX;
	gob2DestX = anim1->destX;
	gob1DestY = anim0->destY;
	gob2DestY = anim1->destY;

	pass = _vm->_map->getPass(gob1X, gob1Y);
	if ((pass > 17) && (pass < 21)) // Ladders, ropes, stairs
		updateLayer1(anim0);
	pass = _vm->_map->getPass(gob2X, gob2Y);
	if ((pass > 17) && (pass < 21)) // Ladders, ropes, stairs
		updateLayer2(anim1);

	if ((gob1DestX < 0) || (gob1DestX > 39) || (gob1DestY < 0) || (gob1DestY > 39))
		return;

	if (gob1Y > gob1DestY) {
		if (_vm->_map->getPass(gob1DestX, gob1DestY) > 17) {
			do {
				gob1DestY--;
			} while (_vm->_map->getPass(gob1DestX, gob1DestY) > 17);
			gob1DestY++;
			if (_vm->_map->getPass(gob1DestX - 1, gob1DestY) == 0) {
				if (_vm->_map->getPass(gob1DestX + 1, gob1DestY) != 0)
					gob1DestX++;
			} else
				gob1DestX--;
			move(gob1DestX, gob1DestY, 0);
		}
	} else {
		if (_vm->_map->getPass(gob1DestX, gob1DestY) > 17) {
			do {
				gob1DestY++;
			} while (_vm->_map->getPass(gob1DestX, gob1DestY) > 17);
			gob1DestY--;
			if (_vm->_map->getPass(gob1DestX - 1, gob1DestY) == 0) {
				if (_vm->_map->getPass(gob1DestX + 1, gob1DestY) != 0)
					gob1DestX++;
			} else
				gob1DestX--;
			move(gob1DestX, gob1DestY, 0);
		}
	}
	if (gob2Y > gob2DestY) {
		if (_vm->_map->getPass(gob2DestX, gob2DestY) > 17) {
			do {
				gob2DestY--;
			} while (_vm->_map->getPass(gob2DestX, gob2DestY) > 17);
			gob2DestY++;
			if (_vm->_map->getPass(gob2DestX - 1, gob2DestY) == 0) {
				if (_vm->_map->getPass(gob2DestX + 1, gob2DestY) != 0)
					gob2DestX++;
			} else
				gob2DestX--;
			move(gob2DestX, gob2DestY, 1);
		}
	} else {
		if (_vm->_map->getPass(gob2DestX, gob2DestY) > 17) {
			do {
				gob2DestY++;
			} while (_vm->_map->getPass(gob2DestX, gob2DestY) > 17);
			gob2DestY--;
			if (_vm->_map->getPass(gob2DestX - 1, gob2DestY) == 0) {
				if (_vm->_map->getPass(gob2DestX + 1, gob2DestY) != 0)
					gob2DestX++;
			} else
				gob2DestX--;
			move(gob2DestX, gob2DestY, 1);
		}
	}
}

bool Goblin_v2::isMovement(int8 state) {
	if ((state >= 0) && (state < 8))
		return true;
	if ((state == 38) || (state == 39))
		return true;

	return false;
}

void Goblin_v2::advMovement(Mult::Mult_Object *obj, int8 state) {
	switch (state) {
	case 0:
		obj->goblinX--;
		break;

	case 1:
		obj->goblinX--;
		obj->goblinY--;
		break;

	case 2:
	case 38:
		obj->goblinY--;
		break;

	case 3:
		obj->goblinX++;
		obj->goblinY--;
		break;

	case 4:
		obj->goblinX++;
		break;

	case 5:
		obj->goblinX++;
		obj->goblinY++;
		break;

	case 6:
	case 39:
		obj->goblinY++;
		break;

	case 7:
		obj->goblinX--;
		obj->goblinY++;
		break;
	}
}

} // End of namespace Gob
