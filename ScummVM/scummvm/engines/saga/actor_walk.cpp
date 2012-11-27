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

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/console.h"
#include "saga/events.h"
#include "saga/isomap.h"
#include "saga/objectmap.h"
#include "saga/script.h"
#include "saga/sound.h"
#include "saga/scene.h"

namespace Saga {

static const int angleLUT[16][2] = {
	{    0, -256 },
	{   98, -237 },
	{  181, -181 },
	{  237,  -98 },
	{  256,    0 },
	{  237,	  98 },
	{  181,  181 },
	{   98,  237 },
	{    0,  256 },
	{  -98,  237 },
	{ -181,  181 },
	{ -237,   98 },
	{ -256,    0 },
	{ -237,  -98 },
	{ -181, -181 },
	{  -98, -237 }
};

static const int directionLUT[8][2] = {
	{  0 * 2, -2 * 2 },
	{  2 * 2, -1 * 2 },
	{  3 * 2,  0 * 2 },
	{  2 * 2,  1 * 2 },
	{  0 * 2,  2 * 2 },
	{ -2 * 2,  1 * 2 },
	{ -4 * 2,  0 * 2 },
	{ -2 * 2, -1 * 2 }
};

static const int tileDirectionLUT[8][2] = {
	{  1,  1 },
	{  2,  0 },
	{  1, -1 },
	{  0, -2 },
	{ -1, -1 },
	{ -2,  0 },
	{ -1,  1 },
	{  0,  2 }
};

struct DragonMove {
	uint16 baseFrame;
	int16 offset[4][2];
};

static const DragonMove dragonMoveTable[12] = {
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{ 28, { { -0,  0 }, { -1,  6 }, {  -5,  11 }, { -10,  15 } } },
	{ 56, { {  0,  0 }, {  1,  6 }, {   5,  11 }, {  10,  15 } } },
	{ 40, { {  0,  0 }, {  6,  1 }, {  11,   5 }, {  15,  10 } } },
	{ 44, { {  0,  0 }, {  6, -1 }, {  11,  -5 }, {  15, -10 } } },
	{ 32, { { -0, -0 }, { -6, -1 }, { -11,  -5 }, { -15, -10 } } },
	{ 52, { { -0,  0 }, { -6,  1 }, { -11,   5 }, { -15,  10 } } },
	{ 36, { {  0, -0 }, {  1, -6 }, {   5, -11 }, {  10, -15 } } },
	{ 48, { { -0, -0 }, { -1, -6 }, {  -5, -11 }, { -10, -15 } } }
};

bool Actor::validFollowerLocation(const Location &location) {
	Point point;
	location.toScreenPointXY(point);

	if ((point.x < 5) || (point.x >= _vm->getDisplayInfo().width - 5) ||
		(point.y < 0) || (point.y > _vm->_scene->getHeight())) {
		return false;
	}

	return (_vm->_scene->canWalk(point));
}

void Actor::realLocation(Location &location, uint16 objectId, uint16 walkFlags) {
	int angle;
	int distance;
	ActorData *actor;
	ObjectData *obj;
	debug (8, "Actor::realLocation objectId=%i", objectId);
	if (walkFlags & kWalkUseAngle) {
		if (_vm->_scene->getFlags() & kSceneFlagISO) {
			angle = (location.x + 2) & 15;
			distance = location.y;

			location.u() = (angleLUT[angle][0] * distance) >> 8;
			location.v() = -(angleLUT[angle][1] * distance) >> 8;
		} else {
			angle = location.x & 15;
			distance = location.y;

			location.x = (angleLUT[angle][0] * distance) >> 6;
			location.y = (angleLUT[angle][1] * distance) >> 6;
		}
	}

	if (objectId != ID_NOTHING) {
		if (validActorId(objectId)) {
			actor = getActor(objectId);
			location.addXY(actor->_location);
		} else if (validObjId(objectId)) {
			obj = getObj(objectId);
			location.addXY(obj->_location);
		}
	}
}

void Actor::actorFaceTowardsObject(uint16 actorId, uint16 objectId) {
	ActorData *actor;
	ObjectData *obj;

	if (validActorId(objectId)) {
		actor = getActor(objectId);
		actorFaceTowardsPoint(actorId, actor->_location);
	} else if (validObjId(objectId)) {
		obj = getObj(objectId);
		actorFaceTowardsPoint(actorId, obj->_location);
	}
}

void Actor::actorFaceTowardsPoint(uint16 actorId, const Location &toLocation) {
	ActorData *actor;
	Location delta;
	//debug (8, "Actor::actorFaceTowardsPoint actorId=%i", actorId);
	actor = getActor(actorId);

	toLocation.delta(actor->_location, delta);

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		if (delta.u() > 0) {
			actor->_facingDirection = (delta.v() > 0) ? kDirUp : kDirRight;
		} else {
			actor->_facingDirection = (delta.v() > 0) ? kDirLeft : kDirDown;
		}
	} else {
		if (ABS(delta.y) > ABS(delta.x * 2)) {
			actor->_facingDirection = (delta.y > 0) ? kDirDown : kDirUp;
		} else {
			actor->_facingDirection = (delta.x > 0) ? kDirRight : kDirLeft;
		}
	}
}

void Actor::updateActorsScene(int actorsEntrance) {
	int j;
	int followerDirection;
	Location tempLocation;
	Location possibleLocation;
	Point delta;
	const SceneEntry *sceneEntry;

	if (_vm->_scene->currentSceneNumber() == 0) {
		error("Actor::updateActorsScene _vm->_scene->currentSceneNumber() == 0");
	}

	_vm->_sound->stopVoice();
	_activeSpeech.stringsCount = 0;
	_activeSpeech.playing = false;
	_protagonist = NULL;

	for (ActorDataArray::iterator actor = _actors.begin(); actor != _actors.end(); ++actor) {
		actor->_inScene = false;
		actor->_spriteList.clear();
		if ((actor->_flags & (kProtagonist | kFollower)) || (actor->_index == 0)) {
			if (actor->_flags & kProtagonist) {
				actor->_finalTarget = actor->_location;
				_centerActor = _protagonist = actor;
			} else if (_vm->getGameId() == GID_ITE &&
					   _vm->_scene->currentSceneResourceId() == ITE_SCENE_OVERMAP) {
				continue;
			}

			actor->_sceneNumber = _vm->_scene->currentSceneNumber();
		}
		if (actor->_sceneNumber == _vm->_scene->currentSceneNumber()) {
			actor->_inScene = true;
			actor->_actionCycle = (_vm->_rnd.getRandomNumber(7) & 0x7) * 4; // 1/8th chance
			if (actor->_currentAction != kActionFreeze) {
				actor->_currentAction = kActionWait;
			}
		}
	}

	// _protagonist can be null while loading a game from the command line
	if (_protagonist == NULL)
		return;

	if ((actorsEntrance >= 0) && (!_vm->_scene->_entryList.empty())) {
		if (_vm->_scene->_entryList.size() <= uint(actorsEntrance)) {
			actorsEntrance = 0; //OCEAN bug
		}

		sceneEntry = &_vm->_scene->_entryList[actorsEntrance];
		if (_vm->_scene->getFlags() & kSceneFlagISO) {
			_protagonist->_location = sceneEntry->location;
		} else {
			_protagonist->_location.x = sceneEntry->location.x * ACTOR_LMULT;
			_protagonist->_location.y = sceneEntry->location.y * ACTOR_LMULT;
			_protagonist->_location.z = sceneEntry->location.z * ACTOR_LMULT;
		}
		// Workaround for bug #1328045:
		// "When entering any of the houses at the start of the
		// game if you click on anything inside the building you
		// start walking through the door, turn around and leave."
		//
		// After stepping on an action zone, Rif is trying to exit.
		// Shift Rif's entry position to a non action zone area.
		if (_vm->getGameId() == GID_ITE) {
			if ((_vm->_scene->currentSceneNumber() >= 53) && (_vm->_scene->currentSceneNumber() <= 66))
				_protagonist->_location.y += 10;
		}

		_protagonist->_facingDirection = _protagonist->_actionDirection = sceneEntry->facing;
	}

	_protagonist->_currentAction = kActionWait;

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		//nothing?
	} else {
		_vm->_scene->initDoorsState(); //TODO: move to _scene
	}

	followerDirection = _protagonist->_facingDirection + 3;
	calcScreenPosition(_protagonist);

	for (ActorDataArray::iterator actor = _actors.begin(); actor != _actors.end(); ++actor) {
		if (actor->_flags & (kFollower)) {
			actor->_facingDirection = actor->_actionDirection = _protagonist->_facingDirection;
			actor->_currentAction = kActionWait;
			actor->_walkStepsCount = actor->_walkStepIndex = 0;
			actor->_location.z = _protagonist->_location.z;


			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				_vm->_isoMap->placeOnTileMap(_protagonist->_location, actor->_location, 3, followerDirection & 0x07);
			} else {
				followerDirection &= 0x07;

				possibleLocation = _protagonist->_location;

				delta.x = directionLUT[followerDirection][0];
				delta.y = directionLUT[followerDirection][1];

				for (j = 0; j < 30; j++) {
					tempLocation = possibleLocation;
					tempLocation.x += delta.x;
					tempLocation.y += delta.y;

					if (validFollowerLocation(tempLocation)) {
						possibleLocation = tempLocation;
					} else {
						tempLocation = possibleLocation;
						tempLocation.x += delta.x;
						if (validFollowerLocation(tempLocation)) {
							possibleLocation = tempLocation;
						} else {
							tempLocation = possibleLocation;
							tempLocation.y += delta.y;
							if (validFollowerLocation(tempLocation)) {
								possibleLocation = tempLocation;
							} else {
								break;
							}
						}
					}
				}

				actor->_location = possibleLocation;
			}
			followerDirection += 2;
		}

	}

	handleActions(0, true);
	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		_vm->_isoMap->adjustScroll(true);
	}
}

void Actor::handleActions(int msec, bool setup) {
	ActorFrameRange *frameRange;
	int state;
	int speed;
	int32 framesLeft;
	Location delta;
	Location addDelta;
	int hitZoneIndex;
	const HitZone *hitZone;
	Point hitPoint;
	Location pickLocation;

	for (ActorDataArray::iterator actor = _actors.begin(); actor != _actors.end(); ++actor) {
		if (!actor->_inScene)
			continue;

		if ((_vm->getGameId() == GID_ITE) && (actor->_index == ACTOR_DRAGON_INDEX)) {
			moveDragon(actor);
			continue;
		}

		switch (actor->_currentAction) {
		case kActionWait:
			if (!setup && (actor->_flags & kFollower)) {
				followProtagonist(actor);
				if (actor->_currentAction != kActionWait)
					break;
			}

			if (actor->_targetObject != ID_NOTHING) {
				actorFaceTowardsObject(actor->_id, actor->_targetObject);
			}

			if (actor->_flags & kCycle) {
				frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameStand));
				if (frameRange->frameCount > 0) {
					actor->_actionCycle++;
					actor->_actionCycle = (actor->_actionCycle) % frameRange->frameCount;
				} else {
					actor->_actionCycle = 0;
				}
				actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
				break;
			}

			if ((actor->_actionCycle & 3) == 0) {
				actor->cycleWrap(100);

				frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameWait));
				if ((frameRange->frameCount < 1 || actor->_actionCycle > 33))
					frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameStand));

				if (frameRange->frameCount) {
					actor->_frameNumber = frameRange->frameIndex + (uint16)_vm->_rnd.getRandomNumber(frameRange->frameCount - 1);
				} else {
					actor->_frameNumber = frameRange->frameIndex;
				}
			}
			actor->_actionCycle++;
			break;

		case kActionWalkToPoint:
		case kActionWalkToLink:
			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				actor->_partialTarget.delta(actor->_location, delta);

				while ((delta.u() == 0) && (delta.v() == 0)) {

					if ((actor == _protagonist) && (_vm->mouseButtonPressed())) {
						_vm->_isoMap->screenPointToTileCoords(_vm->mousePos(), pickLocation);

						if (!actorWalkTo(_protagonist->_id, pickLocation)) {
							break;
						}
					} else if (!_vm->_isoMap->nextTileTarget(actor) && !actorEndWalk(actor->_id, true)) {
						break;
					}

					actor->_partialTarget.delta(actor->_location, delta);
					actor->_partialTarget.z = 0;
				}

				if (actor->_flags & kFastest) {
					speed = 8;
				} else if (actor->_flags & kFaster) {
					speed = 6;
				} else {
					speed = 4;
				}

				if (_vm->_scene->currentSceneResourceId() == ITE_SCENE_OVERMAP) {
					speed = 2;
				}

				if ((actor->_actionDirection == 2) || (actor->_actionDirection == 6)) {
					speed = speed / 2;
				}

				if (ABS(delta.v()) > ABS(delta.u())) {
					addDelta.v() = CLIP<int>(delta.v(), -speed, speed);
					if (addDelta.v() == delta.v()) {
						addDelta.u() = delta.u();
					} else {
						addDelta.u() = delta.u() * addDelta.v();
						addDelta.u() += (addDelta.u() > 0) ? (delta.v() / 2) : (-delta.v() / 2);
						addDelta.u() /= delta.v();
					}
				} else {
					addDelta.u() = CLIP<int>(delta.u(), -speed, speed);
					if (addDelta.u() == delta.u()) {
						addDelta.v() = delta.v();
					} else {
						addDelta.v() = delta.v() * addDelta.u();
						addDelta.v() += (addDelta.v() > 0) ? (delta.u() / 2) : (-delta.u() / 2);
						addDelta.v() /= delta.u();
					}
				}

				actor->_location.add(addDelta);
			} else {
				actor->_partialTarget.delta(actor->_location, delta);

				while ((delta.x == 0) && (delta.y == 0)) {

					if (actor->_walkStepIndex >= actor->_walkStepsCount) {
						actorEndWalk(actor->_id, true);
						return;		// break out of select case
					}

					actor->_partialTarget.fromScreenPoint(actor->_walkStepsPoints[actor->_walkStepIndex++]);
					if (_vm->getGameId() == GID_ITE) {
						if (actor->_partialTarget.x > 224 * 2 * ACTOR_LMULT) {
							actor->_partialTarget.x -= 256 * 2 * ACTOR_LMULT;
						}
					} else {
						if (actor->_partialTarget.x > 224 * 4 * ACTOR_LMULT) {
							actor->_partialTarget.x -= 256 * 4 * ACTOR_LMULT;
						}
					}

					actor->_partialTarget.delta(actor->_location, delta);

					if (ABS(delta.y) > ABS(delta.x)) {
						actor->_actionDirection = delta.y > 0 ? kDirDown : kDirUp;
					} else {
						actor->_actionDirection = delta.x > 0 ? kDirRight : kDirLeft;
					}
				}

				if (_vm->getGameId() == GID_ITE)
					speed = (ACTOR_LMULT * 2 * actor->_screenScale + 63) / 256;
				else
					speed = (ACTOR_SPEED * actor->_screenScale + 128) >> 8;

				if (speed < 1)
					speed = 1;

				if (_vm->getGameId() == GID_IHNM)
					speed = speed / 2;

				if ((actor->_actionDirection == kDirUp) || (actor->_actionDirection == kDirDown)) {
					addDelta.y = CLIP<int>(delta.y, -speed, speed);
					if (addDelta.y == delta.y) {
						addDelta.x = delta.x;
					} else {
						addDelta.x = delta.x * addDelta.y;
						addDelta.x += (addDelta.x > 0) ? (delta.y / 2) : (-delta.y / 2);
						addDelta.x /= delta.y;
						actor->_facingDirection = actor->_actionDirection;
					}
				} else {
					addDelta.x = CLIP<int>(delta.x, -2 * speed, 2 * speed);
					if (addDelta.x == delta.x) {
						addDelta.y = delta.y;
					} else {
						addDelta.y = delta.y * addDelta.x;
						addDelta.y += (addDelta.y > 0) ? (delta.x / 2) : (-delta.x / 2);
						addDelta.y /= delta.x;
						actor->_facingDirection = actor->_actionDirection;
					}
				}

				actor->_location.add(addDelta);
			}

			if (actor->_actorFlags & kActorBackwards) {
				actor->_facingDirection = (actor->_actionDirection + 4) & 7;
				actor->_actionCycle--;
			} else {
				actor->_actionCycle++;
			}

			frameRange = getActorFrameRange(actor->_id, actor->_walkFrameSequence);

			if (actor->_actionCycle < 0) {
				actor->_actionCycle = frameRange->frameCount - 1;
			} else if (actor->_actionCycle >= frameRange->frameCount) {
				actor->_actionCycle = 0;
			}

			actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			break;

		case kActionWalkDir:
			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				actor->_location.u() += tileDirectionLUT[actor->_actionDirection][0];
				actor->_location.v() += tileDirectionLUT[actor->_actionDirection][1];

				frameRange = getActorFrameRange(actor->_id, actor->_walkFrameSequence);

				actor->_actionCycle++;
				actor->cycleWrap(frameRange->frameCount);
				actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			} else {
				if (_vm->getGameId() == GID_ITE) {
					actor->_location.x += directionLUT[actor->_actionDirection][0] * 2;
					actor->_location.y += directionLUT[actor->_actionDirection][1] * 2;
				} else {
					// FIXME: The original does not multiply by 8 here, but we do
					actor->_location.x += (directionLUT[actor->_actionDirection][0] * 8 * actor->_screenScale + 128) >> 8;
					actor->_location.y += (directionLUT[actor->_actionDirection][1] * 8 * actor->_screenScale + 128) >> 8;
				}

				frameRange = getActorFrameRange(actor->_id, actor->_walkFrameSequence);
				actor->_actionCycle++;
				actor->cycleWrap(frameRange->frameCount);
				actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			}
			break;

		case kActionSpeak:
			actor->_actionCycle++;
			actor->cycleWrap(64);

			frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameGesture));
			if (actor->_actionCycle >= frameRange->frameCount) {
				if (actor->_actionCycle & 1)
					break;
				frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameSpeak));

				state = (uint16)_vm->_rnd.getRandomNumber(frameRange->frameCount);

				if (state == 0) {
					frameRange = getActorFrameRange(actor->_id, getFrameType(kFrameStand));
				} else {
					state--;
				}
			} else {
				state = actor->_actionCycle;
			}

			actor->_frameNumber = frameRange->frameIndex + state;
			break;

		case kActionAccept:
		case kActionStoop:
			break;

		case kActionCycleFrames:
		case kActionPongFrames:
			if (actor->_cycleTimeCount > 0) {
				actor->_cycleTimeCount--;
				break;
			}

			actor->_cycleTimeCount = actor->_cycleDelay;
			actor->_actionCycle++;

			frameRange = getActorFrameRange(actor->_id, actor->_cycleFrameSequence);

			if (actor->_currentAction == kActionPongFrames) {
				if (actor->_actionCycle >= frameRange->frameCount * 2 - 2) {
					if (actor->_actorFlags & kActorContinuous) {
						actor->_actionCycle = 0;
					} else {
						actor->_currentAction = kActionFreeze;
						break;
					}
				}

				state = actor->_actionCycle;
				if (state >= frameRange->frameCount) {
					state = frameRange->frameCount * 2 - 2 - state;
				}
			} else {
				if (actor->_actionCycle >= frameRange->frameCount) {
					if (actor->_actorFlags & kActorContinuous) {
						actor->_actionCycle = 0;
					} else {
						actor->_currentAction = kActionFreeze;
						break;
					}
				}
				state = actor->_actionCycle;
			}

			if (frameRange->frameCount && (actor->_actorFlags & kActorRandom)) {
				state = _vm->_rnd.getRandomNumber(frameRange->frameCount - 1);
			}

			if (actor->_actorFlags & kActorBackwards) {
				actor->_frameNumber = frameRange->frameIndex + frameRange->frameCount - 1 - state;
			} else {
				actor->_frameNumber = frameRange->frameIndex + state;
			}
			break;

		case kActionFall:
			if (actor->_actionCycle > 0) {
				framesLeft = actor->_actionCycle--;
				actor->_finalTarget.delta(actor->_location, delta);
				delta.x /= framesLeft;
				delta.y /= framesLeft;
				actor->_location.addXY(delta);
				actor->_fallVelocity += actor->_fallAcceleration;
				actor->_fallPosition += actor->_fallVelocity;
				actor->_location.z = actor->_fallPosition >> 4;
			} else {
				actor->_location = actor->_finalTarget;
				actor->_currentAction = kActionFreeze;
				_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
			}
			break;

		case kActionClimb:
			actor->_cycleDelay++;
			if (actor->_cycleDelay & 3) {
				break;
			}

			if (actor->_location.z >= actor->_finalTarget.z + ACTOR_CLIMB_SPEED) {
				actor->_location.z -= ACTOR_CLIMB_SPEED;
				actor->_actionCycle--;
			} else if (actor->_location.z <= actor->_finalTarget.z - ACTOR_CLIMB_SPEED) {
				actor->_location.z += ACTOR_CLIMB_SPEED;
				actor->_actionCycle++;
			} else {
				actor->_location.z = actor->_finalTarget.z;
				actor->_currentAction = kActionFreeze;
				_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
			}

			frameRange = getActorFrameRange(actor->_id, actor->_cycleFrameSequence);

			if (actor->_actionCycle < 0) {
				actor->_actionCycle = frameRange->frameCount - 1;
			}
			actor->cycleWrap(frameRange->frameCount);
			actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			break;
		}

		if ((actor->_currentAction >= kActionWalkToPoint) && (actor->_currentAction <= kActionWalkDir)) {
			hitZone = NULL;

			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				actor->_location.toScreenPointUV(hitPoint);
			} else {
				actor->_location.toScreenPointXY(hitPoint);
			}
			hitZoneIndex = _vm->_scene->_actionMap->hitTest(hitPoint);
			if (hitZoneIndex != -1) {
				hitZone = _vm->_scene->_actionMap->getHitZone(hitZoneIndex);
			}

			// WORKAROUND for an incorrect hitzone which exists in IHNM
			// In Gorrister's chapter, in the toilet screen, the hitzone of the exit is
			// placed over the place where Gorrister sits to examine the graffiti on the wall
			// to the left, which makes him exit the screen when the graffiti is examined.
			// We effectively change the left side of the hitzone here so that it starts from
			// pixel 301 onwards. The same workaround is applied in Script::whichObject
			if (_vm->getGameId() == GID_IHNM) {
				if (_vm->_scene->currentChapterNumber() == 1 && _vm->_scene->currentSceneNumber() == 22)
					if (hitPoint.x <= 300)
						hitZone = NULL;
			}

			if (hitZone != actor->_lastZone) {
				if (actor->_lastZone)
					stepZoneAction(actor, actor->_lastZone, true, false);
				actor->_lastZone = hitZone;
				// WORKAROUND for graphics glitch in the rat caves. Don't do this step zone action in the rat caves
				// (room 51) for hitzone 24577 (the door with the copy protection) to avoid the glitch. This glitch
				// happens because the copy protection is supposed to kick in at this point, but it's bypassed
				// (with permission from Wyrmkeep Entertainment)
				if (hitZone && !(_vm->getGameId() == GID_ITE && _vm->_scene->currentSceneNumber() == 51 && hitZone->getHitZoneId() == 24577)) {
					stepZoneAction(actor, hitZone, false, false);
				}
			}
		}
	}
	// Update frameCount for sfWaitFrames in IHNM
	_vm->_frameCount++;
}

void Actor::direct(int msec) {

	if (_vm->_scene->_entryList.empty()) {
		return;
	}

	if (_vm->_interface->_statusTextInput) {
		return;
	}

	// FIXME: HACK. This should be turned into cycle event.
	_lastTickMsec += msec;

	if (_lastTickMsec > 1000 / _handleActionDiv) {
		_lastTickMsec = 0;
		//process actions
		handleActions(msec, false);
	}

//process speech
	handleSpeech(msec);
}

bool Actor::followProtagonist(ActorData *actor) {
	Location protagonistLocation;
	Location newLocation;
	Location delta;
	int protagonistBGMaskType;
	Point prefer1;
	Point prefer2;
	Point prefer3;
	int16 prefU;
	int16 prefV;
	int16 newU;
	int16 newV;

	assert(_protagonist);

	actor->_flags &= ~(kFaster | kFastest);
	protagonistLocation = _protagonist->_location;
	calcScreenPosition(_protagonist);

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		prefU = 60;
		prefV = 60;


		actor->_location.delta(protagonistLocation, delta);

		if (actor->_id == actorIndexToId(2)) {
			prefU = prefV = 48;
		}

		if ((delta.u() > prefU) || (delta.u() < -prefU) || (delta.v() > prefV) || (delta.v() < -prefV)) {

			if ((delta.u() > prefU * 2) || (delta.u() < -prefU * 2) || (delta.v() > prefV * 2) || (delta.v() < -prefV * 2)) {
				actor->_flags |= kFaster;

				if ((delta.u() > prefU * 3) || (delta.u() < -prefU*3) || (delta.v() > prefV * 3) || (delta.v() < -prefV * 3)) {
					actor->_flags |= kFastest;
				}
			}

			prefU /= 2;
			prefV /= 2;

			newU = CLIP<int32>(delta.u(), -prefU, prefU) + protagonistLocation.u();
			newV = CLIP<int32>(delta.v(), -prefV, prefV) + protagonistLocation.v();

			newLocation.u() = newU + _vm->_rnd.getRandomNumber(prefU - 1) - prefU / 2;
			newLocation.v() = newV + _vm->_rnd.getRandomNumber(prefV - 1) - prefV / 2;
			newLocation.z = 0;

			return actorWalkTo(actor->_id, newLocation);
		}

	} else {
		prefer1.x = (100 * _protagonist->_screenScale) >> 8;
		prefer1.y = (50 * _protagonist->_screenScale) >> 8;

		if (_protagonist->_currentAction == kActionWalkDir) {
			prefer1.x /= 2;
		}

		if (prefer1.x < 8) {
			prefer1.x = 8;
		}

		if (prefer1.y < 8) {
			prefer1.y = 8;
		}

		prefer2.x = prefer1.x * 2;
		prefer2.y = prefer1.y * 2;
		prefer3.x = prefer1.x + prefer1.x / 2;
		prefer3.y = prefer1.y + prefer1.y / 2;

		actor->_location.delta(protagonistLocation, delta);

		protagonistBGMaskType = 0;
		if (_vm->_scene->isBGMaskPresent() && _vm->_scene->validBGMaskPoint(_protagonist->_screenPosition)) {
			protagonistBGMaskType = _vm->_scene->getBGMaskType(_protagonist->_screenPosition);
		}

		if ((_vm->_rnd.getRandomNumber(7) & 0x7) == 0) // 1/8th chance
			actor->_actorFlags &= ~kActorNoFollow;

		if (actor->_actorFlags & kActorNoFollow) {
			return false;
		}

		if ((delta.x > prefer2.x) || (delta.x < -prefer2.x) ||
			(delta.y > prefer2.y) || (delta.y < -prefer2.y) ||
			((_protagonist->_currentAction == kActionWait) &&
			(delta.x * 2 < prefer1.x) && (delta.x * 2 > -prefer1.x) &&
			(delta.y < prefer1.y) && (delta.y > -prefer1.y))) {

				if (ABS(delta.x) > ABS(delta.y)) {

					delta.x = (delta.x > 0) ? prefer3.x : -prefer3.x;

					newLocation.x = delta.x + protagonistLocation.x;
					newLocation.y = CLIP<int32>(delta.y, -prefer2.y, prefer2.y) + protagonistLocation.y;
				} else {
					delta.y = (delta.y > 0) ? prefer3.y : -prefer3.y;

					newLocation.x = CLIP<int32>(delta.x, -prefer2.x, prefer2.x) + protagonistLocation.x;
					newLocation.y = delta.y + protagonistLocation.y;
				}
				newLocation.z = 0;

				if (protagonistBGMaskType != 3) {
					newLocation.x += _vm->_rnd.getRandomNumber(prefer1.x - 1) - prefer1.x / 2;
					newLocation.y += _vm->_rnd.getRandomNumber(prefer1.y - 1) - prefer1.y / 2;
				}

				newLocation.x = CLIP<int>(newLocation.x, -31 * 4, (_vm->getDisplayInfo().width + 31) * 4);

				return actorWalkTo(actor->_id, newLocation);
			}
	}
	return false;
}

bool Actor::actorWalkTo(uint16 actorId, const Location &toLocation) {
	ActorData *actor;

	Rect testBox;
	Rect testBox2;
	Point anotherActorScreenPosition;
	Point collision;
	Point pointFrom, pointTo, pointBest, pointAdd;
	Point delta, bestDelta;
	Point tempPoint;
	bool extraStartNode;
	bool extraEndNode;

	actor = getActor(actorId);

	if (actor == _protagonist) {
		_vm->_scene->setDoorState(2, 0xff);		// closed
		_vm->_scene->setDoorState(3, 0);		// open
	} else {
		_vm->_scene->setDoorState(2, 0);		// open
		_vm->_scene->setDoorState(3, 0xff);		// closed
	}

	if (_vm->_scene->getFlags() & kSceneFlagISO) {

		if ((_vm->getGameId() == GID_ITE) && (actor->_index == ACTOR_DRAGON_INDEX)) {
			return false;
		}

		actor->_finalTarget = toLocation;
		actor->_walkStepsCount = 0;
		_vm->_isoMap->findTilePath(actor, actor->_location, toLocation);


		if ((actor->_walkStepsCount == 0) && (actor->_flags & kProtagonist)) {
			actor->_actorFlags |= kActorNoCollide;
			_vm->_isoMap->findTilePath(actor, actor->_location, toLocation);
		}

		actor->_walkStepIndex = 0;
		if (_vm->_isoMap->nextTileTarget(actor)) {
			actor->_currentAction = kActionWalkToPoint;
			actor->_walkFrameSequence = getFrameType(kFrameWalk);
		} else {
			actorEndWalk(actorId, false);
			return false;
		}
	} else {

		actor->_location.toScreenPointXY(pointFrom);
		// FIXME: why is the following line needed?
		pointFrom.x &= ~1;	// set last bit to 0

		extraStartNode = _vm->_scene->offscreenPath(pointFrom);

		toLocation.toScreenPointXY(pointTo);
		// FIXME: why is the following line needed?
		pointTo.x &= ~1;	// set last bit to 0

		// Are we already where we want to go?
		if (pointFrom.x == pointTo.x && pointFrom.y == pointTo.y) {
			actor->_walkStepsCount = 0;
			actorEndWalk(actorId, false);
			return false;
		}

		extraEndNode = _vm->_scene->offscreenPath(pointTo);

		if (_vm->_scene->isBGMaskPresent()) {

			if (
				((actor->_currentAction >= kActionWalkToPoint && actor->_currentAction <= kActionWalkDir) ||
				(_vm->getGameId() == GID_ITE && actor == _protagonist)) &&
				!_vm->_scene->canWalk(pointFrom)) {

				int max = _vm->getGameId() == GID_ITE ? 8 : 4;

				for (int i = 1; i < max; i++) {
					pointAdd = pointFrom;
					pointAdd.y += i;
					if (_vm->_scene->canWalk(pointAdd)) {
						pointFrom = pointAdd;
						break;
					}
					pointAdd = pointFrom;
					pointAdd.y -= i;
					if (_vm->_scene->canWalk(pointAdd)) {
						pointFrom = pointAdd;
						break;
					}
					if (_vm->getGameId() == GID_ITE) {
						pointAdd = pointFrom;
						pointAdd.x += i;
						if (_vm->_scene->canWalk(pointAdd)) {
							pointFrom = pointAdd;
							break;
						}
						pointAdd = pointFrom;
						pointAdd.x -= i;
						if (_vm->_scene->canWalk(pointAdd)) {
							pointFrom = pointAdd;
							break;
						}
					}
				}
			}

			_barrierCount = 0;
			if (!(actor->_actorFlags & kActorNoCollide)) {
				collision.x = ACTOR_COLLISION_WIDTH * actor->_screenScale / (256 * 2);
				collision.y = ACTOR_COLLISION_HEIGHT * actor->_screenScale / (256 * 2);

				for (ActorDataArray::iterator anotherActor = _actors.begin(); (anotherActor != _actors.end()) && (_barrierCount < ACTOR_BARRIERS_MAX); ++anotherActor) {
					if (!anotherActor->_inScene)
						continue;
					if (anotherActor == actor)
						continue;

					anotherActorScreenPosition = anotherActor->_screenPosition;
					testBox.left = (anotherActorScreenPosition.x - collision.x) & ~1;
					testBox.right = (anotherActorScreenPosition.x + collision.x) & ~1;
					testBox.top = anotherActorScreenPosition.y - collision.y;
					testBox.bottom = anotherActorScreenPosition.y + collision.y;
					testBox2 = testBox;
					testBox2.left -= 2;
					testBox2.right += 2;
					testBox2.top -= 1;
					testBox2.bottom += 1;

					if (testBox2.contains(pointFrom)) {
						if (pointFrom.x > anotherActorScreenPosition.x + 4) {
							testBox.right = pointFrom.x - 2;
						} else if (pointFrom.x < anotherActorScreenPosition.x - 4) {
							testBox.left = pointFrom.x + 2;
						} else if (pointFrom.y > anotherActorScreenPosition.y) {
							testBox.bottom = pointFrom.y - 1;
						} else {
							testBox.top = pointFrom.y + 1;
						}
					}

					if ((testBox.width() > 0) && (testBox.height() > 0)) {
						_barrierList[_barrierCount++] = testBox;
					}
				}
			}


			pointBest = pointTo;
			actor->_walkStepsCount = 0;
			findActorPath(actor, pointFrom, pointTo);

			if (actor->_walkStepsCount == 0) {
				error("actor->_walkStepsCount == 0");
			}

			if (extraStartNode) {
				actor->_walkStepIndex = 0;
			} else {
				// FIXME: Why is this needed?
				actor->_walkStepIndex = 1;
			}

			if (extraEndNode) {
				toLocation.toScreenPointXY(tempPoint);
				actor->_walkStepsCount--;
				actor->addWalkStepPoint(tempPoint);
			}


			pointBest = actor->_walkStepsPoints[actor->_walkStepsCount - 1];

			pointBest.x &= ~1;
			delta.x = ABS(pointFrom.x - pointTo.x);
			delta.y = ABS(pointFrom.y - pointTo.y);

			bestDelta.x = ABS(pointBest.x - pointTo.x);
			bestDelta.y = ABS(pointBest.y - pointTo.y);

			if ((delta.x + delta.y <= bestDelta.x + bestDelta.y) && (actor->_flags & kFollower)) {
				actor->_actorFlags |= kActorNoFollow;
			}

			if (pointBest == pointFrom) {
				actor->_walkStepsCount = 0;
			}
		} else {
			actor->_walkStepsCount = 0;
			actor->addWalkStepPoint(pointTo);
			actor->_walkStepIndex = 0;
		}

		actor->_partialTarget = actor->_location;
		actor->_finalTarget = toLocation;
		if (actor->_walkStepsCount == 0) {
			actorEndWalk(actorId, false);
			return false;
		} else {
			if (actor->_flags & kProtagonist) {
				_actors[1]._actorFlags &= ~kActorNoFollow; // TODO: mark all actors with kFollower flag, not only 1 and 2
				_actors[2]._actorFlags &= ~kActorNoFollow;
			}
			actor->_currentAction = (actor->_walkStepsCount >= ACTOR_MAX_STEPS_COUNT) ? kActionWalkToLink : kActionWalkToPoint;
			actor->_walkFrameSequence = getFrameType(kFrameWalk);
		}
	}
	return true;
}

bool Actor::actorEndWalk(uint16 actorId, bool recurse) {
	bool walkMore = false;
	ActorData *actor;
	const HitZone *hitZone;
	int hitZoneIndex;
	Point testPoint;

	actor = getActor(actorId);
	actor->_actorFlags &= ~kActorBackwards;

	if (_vm->getGameId() == GID_ITE) {

		if (actor->_location.distance(actor->_finalTarget) > 8 && (actor->_flags & kProtagonist) && recurse && !(actor->_actorFlags & kActorNoCollide)) {
			actor->_actorFlags |= kActorNoCollide;
			return actorWalkTo(actorId, actor->_finalTarget);
		}
	}

	actor->_currentAction = kActionWait;
	actor->_actionCycle = 0;
	if (actor->_actorFlags & kActorFinalFace) {
		actor->_facingDirection = actor->_actionDirection = (actor->_actorFlags >> 6) & 0x07; //?
	}

	actor->_actorFlags &= ~(kActorNoCollide | kActorCollided | kActorFinalFace | kActorFacingMask);
	actor->_flags &= ~(kFaster | kFastest);

	if (actor == _protagonist) {
		_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
		if (_vm->_script->_pendingVerb == _vm->_script->getVerbType(kVerbWalkTo)) {
			if (_vm->getGameId() == GID_ITE)
				actor->_location.toScreenPointUV(testPoint); // it's wrong calculation, but it is used in ITE
			else
				actor->_location.toScreenPointXY(testPoint);

			hitZoneIndex = _vm->_scene->_actionMap->hitTest(testPoint);
			if (hitZoneIndex != -1) {
				hitZone = _vm->_scene->_actionMap->getHitZone(hitZoneIndex);
				stepZoneAction(actor, hitZone, false, true);
			} else {
				_vm->_script->setNoPendingVerb();
			}
		} else if (_vm->_script->_pendingVerb != _vm->_script->getVerbType(kVerbNone)) {
			_vm->_script->doVerb();
		}
	} else {
		if (recurse && (actor->_flags & kFollower))
			walkMore = followProtagonist(actor);

		_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
	}
	return walkMore;
}

void Actor::moveDragon(ActorData *actor) {
	int16 dir0, dir1, dir2, dir3;
	int16 moveType;
	Event event;
	const DragonMove *dragonMove;

	if ((actor->_actionCycle < 0) ||
		((actor->_actionCycle == 0) && (actor->_dragonMoveType >= ACTOR_DRAGON_TURN_MOVES))) {

		moveType = kDragonMoveInvalid;
		if (actor->_location.distance(_protagonist->_location) < 24) {
			if (_dragonHunt && (_protagonist->_currentAction != kActionFall)) {
				event.type = kEvTOneshot;
				event.code = kScriptEvent;
				event.op = kEventExecNonBlocking;
				event.time = 0;
				event.param = _vm->_scene->getScriptModuleNumber(); // module number
				event.param2 = ACTOR_EXP_KNOCK_RIF;			// script entry point number
				event.param3 = -1;		// Action
				event.param4 = -1;		// Object
				event.param5 = -1;		// With Object
				event.param6 = -1;		// Actor
				_vm->_events->queue(event);

				_dragonHunt = false;
			}
		} else {
			_dragonHunt = true;
		}

		if (actor->_walkStepIndex + 2 > actor->_walkStepsCount) {

			_vm->_isoMap->findDragonTilePath(actor, actor->_location, _protagonist->_location, actor->_actionDirection);

			if (actor->_walkStepsCount == 0) {
				_vm->_isoMap->findDragonTilePath(actor, actor->_location, _protagonist->_location, 0);
			}

			if (actor->_walkStepsCount < 2) {
				return;
			}

			actor->_partialTarget = actor->_location;
			actor->_finalTarget = _protagonist->_location;
			actor->_walkStepIndex = 0;
		}

		dir0 = actor->_actionDirection;
		dir1 = actor->_tileDirections[actor->_walkStepIndex++];
		dir2 = actor->_tileDirections[actor->_walkStepIndex];
		// Fix for Bug #3324850 ("ITE (SAGA): crash in dog sewers")
		// If there were more than two steps left, get the third (next) step.
		// Otherwise, store the second step again so the anim looks right.
		// (If you stop the move instead, Rif isn't automatically knocked into
		// the Sewer.)
		if (actor->_walkStepIndex + 1 < actor->_walkStepsCount)
			dir3 = actor->_tileDirections[actor->_walkStepIndex + 1];
		else
			dir3 = dir2;

		if (dir0 != dir1){
			actor->_actionDirection = dir0 = dir1;
		}

		actor->_location = actor->_partialTarget;

		if ((dir1 != dir2) && (dir1 == dir3)) {
			switch (dir1) {
			case kDirUpLeft:
				actor->_partialTarget.v() += 16;
				moveType = kDragonMoveUpLeft;
				break;
			case kDirDownLeft:
				actor->_partialTarget.u() -= 16;
				moveType = kDragonMoveDownLeft;
				break;
			case kDirDownRight:
				actor->_partialTarget.v() -= 16;
				moveType = kDragonMoveDownRight;
				break;
			case kDirUpRight:
				actor->_partialTarget.u() += 16;
				moveType = kDragonMoveUpRight;
				break;
			}

			switch (dir2) {
			case kDirUpLeft:
				actor->_partialTarget.v() += 16;
				break;
			case kDirDownLeft:
				actor->_partialTarget.u() -= 16;
				break;
			case kDirDownRight:
				actor->_partialTarget.v() -= 16;
				break;
			case kDirUpRight:
				actor->_partialTarget.u() += 16;
				break;
			}

			actor->_walkStepIndex++;
		} else {
			switch (dir1) {
			case kDirUpLeft:
				actor->_partialTarget.v() += 16;
				switch (dir2) {
				case kDirDownLeft:
					moveType = kDragonMoveUpLeft_Left;
					actor->_partialTarget.u() -= 16;
					break;
				case kDirUpLeft:
					moveType = kDragonMoveUpLeft;
					break;
				case kDirUpRight:
					actor->_partialTarget.u() += 16;
					moveType = kDragonMoveUpLeft_Right;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;
			case kDirDownLeft:
				actor->_partialTarget.u() -= 16;
				switch (dir2) {
				case kDirDownRight:
					moveType = kDragonMoveDownLeft_Left;
					actor->_partialTarget.v() -= 16;
					break;
				case kDirDownLeft:
					moveType = kDragonMoveDownLeft;
					break;
				case kDirUpLeft:
					moveType = kDragonMoveDownLeft_Right;
					actor->_partialTarget.v() += 16;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;
			case kDirDownRight:
				actor->_partialTarget.v() -= 16;
				switch (dir2) {
				case kDirUpRight:
					moveType = kDragonMoveDownRight_Left;
					actor->_partialTarget.u() += 16;
					break;
				case kDirDownRight:
					moveType = kDragonMoveDownRight;
					break;
				case kDirDownLeft:
					moveType = kDragonMoveDownRight_Right;
					actor->_partialTarget.u() -= 16;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;
			case kDirUpRight:
				actor->_partialTarget.u() += 16;
				switch (dir2) {
				case kDirUpLeft:
					moveType = kDragonMoveUpRight_Left;
					actor->_partialTarget.v() += 16;
					break;
				case kDirUpRight:
					moveType = kDragonMoveUpRight;
					break;
				case kDirDownRight:
					moveType = kDragonMoveUpRight_Right;
					actor->_partialTarget.v() -= 16;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;

			default:
				actor->_actionDirection = dir1;
				actor->_walkStepsCount = 0;
				break;
			}
		}

		actor->_dragonMoveType = moveType;

		if (moveType >= ACTOR_DRAGON_TURN_MOVES) {
			actor->_dragonStepCycle = 0;
			actor->_actionCycle = 4;
			actor->_walkStepIndex++;
		} else {
			actor->_actionCycle = 4;
		}
	}

	actor->_actionCycle--;

	if ((actor->_walkStepsCount < 1) || (actor->_actionCycle < 0)) {
		return;
	}

	if (actor->_dragonMoveType < ACTOR_DRAGON_TURN_MOVES) {

		actor->_dragonStepCycle++;
		if (actor->_dragonStepCycle >= 7) {
			actor->_dragonStepCycle = 0;
		}

		actor->_dragonBaseFrame = actor->_dragonMoveType * 7;

		if (actor->_location.u() > actor->_partialTarget.u() + 3) {
			actor->_location.u() -= 4;
		} else if (actor->_location.u() < actor->_partialTarget.u() - 3) {
			actor->_location.u() += 4;
		} else {
			actor->_location.u() = actor->_partialTarget.u();
		}

		if (actor->_location.v() > actor->_partialTarget.v() + 3) {
			actor->_location.v() -= 4;
		} else if (actor->_location.v() < actor->_partialTarget.v() - 3) {
			actor->_location.v() += 4;
		} else {
			actor->_location.v() = actor->_partialTarget.v();
		}
	} else {
		dragonMove = &dragonMoveTable[actor->_dragonMoveType];
		actor->_dragonBaseFrame = dragonMove->baseFrame;


		actor->_location.u() = actor->_partialTarget.u() - dragonMove->offset[actor->_actionCycle][0];
		actor->_location.v() = actor->_partialTarget.v() - dragonMove->offset[actor->_actionCycle][1];

		actor->_dragonStepCycle++;
		if (actor->_dragonStepCycle >= 3) {
			actor->_dragonStepCycle = 3;
		}
	}

	actor->_frameNumber = actor->_dragonBaseFrame + actor->_dragonStepCycle;
}

// Console wrappers - must be safe to run

void Actor::cmdActorWalkTo(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	Location location;
	Point movePoint;

	movePoint.x = atoi(argv[2]);
	movePoint.y = atoi(argv[3]);

	location.fromScreenPoint(movePoint);

	if (!validActorId(actorId)) {
		_vm->_console->DebugPrintf("Actor::cmActorWalkTo Invalid actorId 0x%X.\n", actorId);
		return;
	}

	actorWalkTo(actorId, location);
}

} // End of namespace Saga
