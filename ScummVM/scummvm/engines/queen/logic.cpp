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


#include "common/config-manager.h"
#include "common/textconsole.h"

#include "queen/logic.h"

#include "queen/bankman.h"
#include "queen/command.h"
#include "queen/credits.h"
#include "queen/cutaway.h"
#include "queen/debug.h"
#include "queen/defs.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/grid.h"
#include "queen/input.h"
#include "queen/journal.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"
#include "queen/state.h"
#include "queen/talk.h"
#include "queen/walk.h"

namespace Queen {

Logic::Logic(QueenEngine *vm)
	: _credits(NULL), _objectData(NULL), _roomData(NULL), _sfxName(NULL),
	_itemData(NULL), _graphicData(NULL), _walkOffData(NULL), _objectDescription(NULL),
	_furnitureData(NULL), _actorData(NULL), _graphicAnim(NULL), _vm(vm) {
	_joe.x = _joe.y = 0;
	_joe.scale = 100;
	_joe.walk = JWM_NORMAL;
	memset(_gameState, 0, sizeof(_gameState));
	memset(_talkSelected, 0, sizeof(_talkSelected));
	_puzzleAttemptCount = 0;
	_journal = new Journal(vm);
	_scene = 0;
	memset(_specialMoves, 0, sizeof(_specialMoves));
	readQueenJas();
}

Logic::~Logic() {
	delete _journal;
	delete _credits;
	delete[] _objectData;
	delete[] _roomData;
	delete[] _sfxName;
	delete[] _itemData;
	delete[] _graphicData;
	delete[] _walkOffData;
	delete[] _objectDescription;
	delete[] _furnitureData;
	delete[] _actorData;
	delete[] _graphicAnim;
}

void Logic::readQueenJas() {
	int16 i;

	uint8 *jas = _vm->resource()->loadFile("QUEEN.JAS", 20);
	uint8 *ptr = jas;

	_numRooms = READ_BE_UINT16(ptr); ptr += 2;
	_numNames = READ_BE_UINT16(ptr); ptr += 2;
	_numObjects = READ_BE_UINT16(ptr); ptr += 2;
	_numDescriptions = READ_BE_UINT16(ptr); ptr += 2;

	_objectData = new ObjectData[_numObjects + 1];
	memset(&_objectData[0], 0, sizeof(ObjectData));
	for (i = 1; i <= _numObjects; i++) {
		_objectData[i].readFromBE(ptr);
	}

	_roomData = new uint16[_numRooms + 2];
	_roomData[0] = 0;
	for (i = 1; i <= (_numRooms + 1); i++) {
		_roomData[i] = READ_BE_UINT16(ptr);	ptr += 2;
	}
	_roomData[_numRooms + 1] = _numObjects;

	if ((_vm->resource()->isDemo() && _vm->resource()->getPlatform() == Common::kPlatformPC) ||
		(_vm->resource()->isInterview() && _vm->resource()->getPlatform() == Common::kPlatformAmiga)) {
		_sfxName = NULL;
	} else {
		_sfxName = new uint16[_numRooms + 1];
		_sfxName[0] = 0;
		for (i = 1; i <= _numRooms; i++) {
			_sfxName[i] = READ_BE_UINT16(ptr); ptr += 2;
		}
	}

	_numItems = READ_BE_UINT16(ptr); ptr += 2;
	_itemData = new ItemData[_numItems + 1];
	memset(&_itemData[0], 0, sizeof(ItemData));
	for (i = 1; i <= _numItems; i++) {
		_itemData[i].readFromBE(ptr);
	}

	_numGraphics = READ_BE_UINT16(ptr); ptr += 2;
	_graphicData = new GraphicData[_numGraphics + 1];
	memset(&_graphicData[0], 0, sizeof(GraphicData));
	for (i = 1; i <= _numGraphics; i++) {
		_graphicData[i].readFromBE(ptr);
	}

	_vm->grid()->readDataFrom(_numObjects, _numRooms, ptr);

	_numWalkOffs = READ_BE_UINT16(ptr);	ptr += 2;
	_walkOffData = new WalkOffData[_numWalkOffs + 1];
	memset(&_walkOffData[0], 0, sizeof(WalkOffData));
	for (i = 1; i <= _numWalkOffs; i++) {
		_walkOffData[i].readFromBE(ptr);
	}

	_numObjDesc = READ_BE_UINT16(ptr); ptr += 2;
	_objectDescription = new ObjectDescription[_numObjDesc + 1];
	memset(&_objectDescription[0], 0, sizeof(ObjectDescription));
	for (i = 1; i <= _numObjDesc; i++) {
		_objectDescription[i].readFromBE(ptr);
	}

	_vm->command()->readCommandsFrom(ptr);

	_entryObj = READ_BE_UINT16(ptr); ptr += 2;

	_numFurniture = READ_BE_UINT16(ptr); ptr += 2;
	_furnitureData = new FurnitureData[_numFurniture + 1];
	memset(&_furnitureData[0], 0, sizeof(FurnitureData));
	for (i = 1; i <= _numFurniture; i++) {
		_furnitureData[i].readFromBE(ptr);
	}

	// Actors
	_numActors = READ_BE_UINT16(ptr); ptr += 2;
	_numAAnim = READ_BE_UINT16(ptr); ptr += 2;
	_numAName = READ_BE_UINT16(ptr); ptr += 2;
	_numAFile = READ_BE_UINT16(ptr); ptr += 2;

	_actorData = new ActorData[_numActors + 1];
	memset(&_actorData[0], 0, sizeof(ActorData));
	for (i = 1; i <= _numActors; i++) {
		_actorData[i].readFromBE(ptr);
	}

	_numGraphicAnim = READ_BE_UINT16(ptr); ptr += 2;

	_graphicAnim = new GraphicAnim[_numGraphicAnim + 1];
	if (_numGraphicAnim == 0) {
		_graphicAnim[0].readFromBE(ptr);
	} else {
		memset(&_graphicAnim[0], 0, sizeof(GraphicAnim));
		for (i = 1; i <= _numGraphicAnim; i++) {
			_graphicAnim[i].readFromBE(ptr);
		}
	}

	_currentRoom = _objectData[_entryObj].room;
	_entryObj = 0;

	if (memcmp(ptr, _vm->resource()->getJASVersion(), 5) != 0) {
		warning("Unexpected queen.jas file format");
	}
	delete[] jas;

	_vm->resource()->loadTextFile("QUEEN2.JAS", _jasStringList);
	_jasStringOffset[0] = 0;
	_jasStringOffset[1] = _jasStringOffset[0] + _numDescriptions;
	_jasStringOffset[2] = _jasStringOffset[1] + _numNames;
	_jasStringOffset[3] = _jasStringOffset[2] + _numRooms;
	_jasStringOffset[4] = _jasStringOffset[3] + 12;
	_jasStringOffset[5] = _jasStringOffset[4] + JOE_RESPONSE_MAX;
	_jasStringOffset[6] = _jasStringOffset[5] + _numAAnim;
	_jasStringOffset[7] = _jasStringOffset[6] + _numAName;

	// Patch for German text bug
	if (_vm->resource()->getLanguage() == Common::DE_DEU) {
		_jasStringList[_jasStringOffset[JSO_OBJECT_DESCRIPTION] + 296 - 1] = "Es bringt nicht viel, das festzubinden.";
	}
}

void Logic::start() {
	setupSpecialMoveTable();
	_vm->command()->clear(false);
	_vm->display()->setupPanel();
	_vm->graphics()->unpackControlBank();
	_vm->graphics()->setupMouseCursor();
	setupJoe();
	_vm->grid()->setupPanel();
	inventorySetup();

	_oldRoom = 0;
	_newRoom = _currentRoom;
}

uint16 Logic::findBob(uint16 obj) const {
	assert(obj <= _numObjects);

	uint16 room = _objectData[obj].room;
	assert(room <= _numRooms);

	uint16 bobnum = 0;
	int16 img = _objectData[obj].image;
	if (img != 0) {
		if (img == -3 || img == -4) {
			// a person object
			bobnum = findPersonNumber(obj, room);
		} else {
			uint16 bobtype = 0; // 1 for animated, 0 for static

			if (img <= -10) {
				// object has been turned off, but the image order hasn't been updated
				if (_graphicData[-(img + 10)].lastFrame != 0) {
					bobtype = 1;
				}
			} else if (img == -2) {
				// -1 static, -2 animated
				bobtype = 1;
			} else if (img > 0) {
				if (_graphicData[img].lastFrame != 0) {
					bobtype = 1;
				}
			}

			uint16 idxAnimated = 0;
			uint16 idxStatic = 0;
			for (uint16 i = _roomData[room] + 1; i <= obj; ++i) {
				img = _objectData[i].image;
				if (img <= -10) {
					if (_graphicData[-(img + 10)].lastFrame != 0) {
						++idxAnimated;
					} else {
						++idxStatic;
					}
				} else if (img > 0) {
					if (img > 5000) {
						img -= 5000;
					}

					assert (img <= _numGraphics);

					if (_graphicData[img].lastFrame != 0) {
						++idxAnimated;
					} else {
						++idxStatic;
					}
				} else if (img == -1) {
					++idxStatic;
				} else if (img == -2) {
					++idxAnimated;
				}
			}
			if (bobtype == 0) {
				// static bob
				if (idxStatic > 0) {
					bobnum = 19 + _vm->graphics()->numStaticFurniture() + idxStatic;
				}
			} else {
				// animated bob
				if (idxAnimated > 0) {
					bobnum = 4 + _vm->graphics()->numAnimatedFurniture() + idxAnimated;
				}
			}
		}
	}
	return bobnum;
}

uint16 Logic::findFrame(uint16 obj) const {
	uint16 framenum = 0;
	uint16 room = _objectData[obj].room;
	int16 img = _objectData[obj].image;
	if (img == -3 || img == -4) {
		uint16 bobnum = findPersonNumber(obj, room);
		if (bobnum <= 3) {
			framenum = 31 + bobnum;
		}
	} else {
		uint16 idx = 0;
		for (uint16 i = _roomData[room] + 1; i < obj; ++i) {
			img = _objectData[i].image;
			if (img <= -10) {
				const GraphicData* pgd = &_graphicData[-(img + 10)];
				if (pgd->lastFrame != 0) {
					// skip all the frames of the animation
					idx += ABS(pgd->lastFrame) - pgd->firstFrame + 1;
				} else {
					// static bob, skip one frame
					++idx;
				}
			} else if (img == -1) {
				++idx;
			} else if (img > 0) {
				if (img > 5000) {
					img -= 5000;
				}
				const GraphicData* pgd = &_graphicData[img];
				uint16 lastFrame = ABS(pgd->lastFrame);
				if (pgd->firstFrame < 0) {
					idx += lastFrame;
				} else if (lastFrame != 0) {
					idx += (lastFrame - pgd->firstFrame) + 1;
				} else {
					++idx;
				}
			}
		}

		img = _objectData[obj].image;
		if (img <= -10) {
			const GraphicData* pgd = &_graphicData[-(img + 10)];
			if (pgd->lastFrame != 0) {
				idx += ABS(pgd->lastFrame) - pgd->firstFrame + 1;
			} else {
				++idx;
			}
		} else if (img == -1 || img > 0) {
			++idx;
		}

		// calculate only if there are person frames
		if (idx > 0) {
			framenum = FRAMES_JOE + _vm->graphics()->numFurnitureFrames() + idx;
		}
	}
	return framenum;
}

uint16 Logic::objectForPerson(uint16 bobNum) const {
	uint16 bobcur = 0;
	// first object number in the room
	uint16 cur = currentRoomData() + 1;
	// last object number in the room
	uint16 last = _roomData[_currentRoom + 1];
	for (; cur <= last; ++cur) {
		int16 image = _objectData[cur].image;
		if (image == -3 || image == -4) {
			// the object is a bob
			++bobcur;
		}
		if (bobcur == bobNum) {
			return cur;
		}
	}
	return 0;
}

WalkOffData *Logic::walkOffPointForObject(int16 obj) const {
	for (uint16 i = 1; i <= _numWalkOffs; ++i) {
		if (_walkOffData[i].entryObj == obj) {
			return &_walkOffData[i];
		}
	}
	return NULL;
}

void Logic::joeWalk(JoeWalkMode walking) {
	_joe.walk = walking;
	// Do this so that Input doesn't need to know the walk value
	_vm->input()->dialogueRunning(JWM_SPEAK == walking);
}

int16 Logic::gameState(int index) const {
	assert(index >= 0 && index < GAME_STATE_COUNT);
	return _gameState[index];
}

void Logic::gameState(int index, int16 newValue) {
	assert(index >= 0 && index < GAME_STATE_COUNT);
	debug(8, "Logic::gameState() [%d] = %d", index, newValue);
	_gameState[index] = newValue;
}

const char *Logic::roomName(uint16 roomNum) const {
	assert(roomNum >= 1 && roomNum <= _numRooms);
	return _jasStringList[_jasStringOffset[JSO_ROOM_NAME] + roomNum - 1].c_str();
}

const char *Logic::objectName(uint16 objNum) const {
	assert(objNum >= 1 && objNum <= _numNames);
	return _jasStringList[_jasStringOffset[JSO_OBJECT_NAME] + objNum - 1].c_str();
}

const char *Logic::objectTextualDescription(uint16 objNum) const {
	assert(objNum >= 1 && objNum <= _numDescriptions);
	return _jasStringList[_jasStringOffset[JSO_OBJECT_DESCRIPTION] + objNum - 1].c_str();
}

const char *Logic::joeResponse(int i) const {
	assert(i >= 1 && i <= JOE_RESPONSE_MAX);
	return _jasStringList[_jasStringOffset[JSO_JOE_RESPONSE] + i - 1].c_str();
}

const char *Logic::verbName(Verb v) const {
	assert(v >= 0 && v <= 12);
	if (v == 0) {
		return "";
	}
	return _jasStringList[_jasStringOffset[JSO_VERB_NAME] + v - 1].c_str();
}

const char *Logic::actorAnim(int num) const {
	assert(num >= 1 && num <= _numAAnim);
	return _jasStringList[_jasStringOffset[JSO_ACTOR_ANIM] + num - 1].c_str();
}

const char *Logic::actorName(int num) const {
	assert(num >= 1 && num <= _numAName);
	return _jasStringList[_jasStringOffset[JSO_ACTOR_NAME] + num - 1].c_str();
}

const char *Logic::actorFile(int num) const {
	assert(num >= 1 && num <= _numAFile);
	return _jasStringList[_jasStringOffset[JSO_ACTOR_FILE] + num - 1].c_str();
}

void Logic::eraseRoom() {
	_vm->bankMan()->eraseFrames(false);
	_vm->bankMan()->close(15);
	_vm->bankMan()->close(11);
	_vm->bankMan()->close(10);
	_vm->bankMan()->close(12);

	_vm->display()->palFadeOut(_currentRoom);

	// invalidates all persons animations
	_vm->graphics()->clearPersonFrames();
	_vm->graphics()->eraseAllAnims();

	uint16 cur = _roomData[_oldRoom] + 1;
	uint16 last = _roomData[_oldRoom + 1];
	for (; cur <= last; ++cur) {
		ObjectData *pod = &_objectData[cur];
		if (pod->name == 0) {
			// object has been deleted, invalidate image
			pod->image = 0;
		} else if (pod->image > -4000 && pod->image <= -10) {
			if (_graphicData[ABS(pod->image + 10)].lastFrame == 0) {
				// static Bob
				pod->image = -1;
			} else {
				// animated Bob
				pod->image = -2;
			}
		}
	}
}

void Logic::setupRoom(const char *room, int comPanel, bool inCutaway) {
	// load backdrop image, init dynalum, setup colors
	_vm->display()->setupNewRoom(room, _currentRoom);

	// setup graphics to enter fullscreen/panel mode
	_vm->display()->screenMode(comPanel, inCutaway);

	_vm->grid()->setupNewRoom(_currentRoom, _roomData[_currentRoom]);

	int16 furn[9];
	uint16 furnTot = 0;
	for (uint16 i = 1; i <= _numFurniture; ++i) {
		if (_furnitureData[i].room == _currentRoom) {
			++furnTot;
			furn[furnTot] = _furnitureData[i].objNum;
		}
	}
	_vm->graphics()->setupNewRoom(room, _currentRoom, furn, furnTot);

	_vm->display()->forceFullRefresh();
}

void Logic::displayRoom(uint16 room, RoomDisplayMode mode, uint16 scale, int comPanel, bool inCutaway) {
	debug(6, "Logic::displayRoom(%d, %d, %d, %d, %d)", room, mode, scale, comPanel, inCutaway);

	eraseRoom();

	if (_credits)
		_credits->nextRoom();

	setupRoom(roomName(room), comPanel, inCutaway);
	if (mode != RDM_FADE_NOJOE) {
		setupJoeInRoom(mode != RDM_FADE_JOE_XY, scale);
	}
	if (mode != RDM_NOFADE_JOE) {
		_vm->update();
		BobSlot *joe = _vm->graphics()->bob(0);
		_vm->display()->palFadeIn(_currentRoom, joe->active, joe->x, joe->y);
	}
	if (mode != RDM_FADE_NOJOE && joeX() != 0 && joeY() != 0) {
		int16 jx = joeX();
		int16 jy = joeY();
		joePos(0, 0);
		_vm->walk()->moveJoe(0, jx, jy, inCutaway);
	}
}

ActorData *Logic::findActor(uint16 noun, const char *name) const {
	uint16 obj = currentRoomData() + noun;
	int16 img = objectData(obj)->image;
	if (img != -3 && img != -4) {
		warning("Logic::findActor() - Object %d is not a person", obj);
		return NULL;
	}

	// search Bob number for the person
	uint16 bobNum = findPersonNumber(obj, _currentRoom);

	// search for a matching actor
	if (bobNum > 0) {
		for (uint16 i = 1; i <= _numActors; ++i) {
			ActorData *pad = &_actorData[i];
			if (pad->room == _currentRoom && gameState(pad->gsSlot) == pad->gsValue) {
				if (bobNum == pad->bobNum || (name && strcmp(actorName(pad->name), name) == 0)) {
					return pad;
				}
			}
		}
	}
	return NULL;
}

bool Logic::initPerson(uint16 noun, const char *name, bool loadBank, Person *pp) {
	const ActorData *pad = findActor(noun, name);
	if (pad != NULL) {
		pp->actor = pad;
		pp->name = actorName(pad->name);
		if (pad->anim != 0) {
			pp->anim = actorAnim(pad->anim);
		} else {
			pp->anim = NULL;
		}
		if (loadBank && pad->file != 0) {
			_vm->bankMan()->load(actorFile(pad->file), pad->bankNum);
			// if there is no valid actor file (ie pad->file is 0), the person
			// data is already loaded as it is included in objects room bank (.bbk)
		}
		pp->bobFrame = 31 + pp->actor->bobNum;
	}
	return pad != NULL;
}

uint16 Logic::findPersonNumber(uint16 obj, uint16 room) const {
	uint16 num = 0;
	for (uint16 i = _roomData[room] + 1; i <= obj; ++i) {
		int16 img = _objectData[i].image;
		if (img == -3 || img == -4) {
			++num;
		}
	}
	return num;
}

void Logic::loadJoeBanks(const char *animBank, const char *standBank) {
	_vm->bankMan()->load(animBank, 13);
	for (int i = 11; i < 31; ++i) {
		_vm->bankMan()->unpack(i - 10, i, 13);
	}
	_vm->bankMan()->close(13);

	_vm->bankMan()->load(standBank, 7);
	_vm->bankMan()->unpack(1, 35, 7);
	_vm->bankMan()->unpack(3, 36, 7);
	_vm->bankMan()->unpack(5, 37, 7);
}

void Logic::setupJoe() {
	loadJoeBanks("JOE_A.BBK", "JOE_B.BBK");
	joePrevFacing(DIR_FRONT);
	joeFacing(DIR_FRONT);
}

void Logic::setupJoeInRoom(bool autoPosition, uint16 scale) {
	debug(9, "Logic::setupJoeInRoom(%d, %d) joe.x=%d joe.y=%d", autoPosition, scale, _joe.x, _joe.y);

	int16 oldx, oldy;
	if (!autoPosition || joeX() != 0 || joeY() != 0) {
		oldx = joeX();
		oldy = joeY();
		joePos(0, 0);
	} else {
		const ObjectData *pod = objectData(_entryObj);
		// find the walk off point for the entry object and make
		// Joe walking to that point
		const WalkOffData *pwo = walkOffPointForObject(_entryObj);
		if (pwo != NULL) {
			oldx = pwo->x;
			oldy = pwo->y;
			// entryObj has a walk off point, then walk from there to object x,y
			joePos(pod->x, pod->y);
		} else {
			// no walk off point, use object position
			oldx = pod->x;
			oldy = pod->y;
			joePos(0, 0);
		}
	}

	debug(6, "Logic::setupJoeInRoom() - oldx=%d, oldy=%d scale=%d", oldx, oldy, scale);

	if (scale > 0 && scale < 100) {
		joeScale(scale);
	} else {
		uint16 a = _vm->grid()->findAreaForPos(GS_ROOM, oldx, oldy);
		if (a > 0) {
			joeScale(_vm->grid()->area(_currentRoom, a)->calcScale(oldy));
		} else {
			joeScale(100);
		}
	}

	if (joeCutFacing() > 0) {
		joeFacing(joeCutFacing());
		joeCutFacing(0);
	} else {
		// check to see which way Joe entered room
		const ObjectData *pod = objectData(_entryObj);
		switch (State::findDirection(pod->state)) {
		case DIR_BACK:
			joeFacing(DIR_FRONT);
			break;
		case DIR_FRONT:
			joeFacing(DIR_BACK);
			break;
		case DIR_LEFT:
			joeFacing(DIR_RIGHT);
			break;
		case DIR_RIGHT:
			joeFacing(DIR_LEFT);
			break;
		}
	}
	joePrevFacing(joeFacing());

	BobSlot *pbs = _vm->graphics()->bob(0);
	pbs->scale = joeScale();

	if (_currentRoom == 108) {
		_vm->graphics()->putCameraOnBob(-1);
		_vm->bankMan()->load("JOE_E.ACT", 7);
		_vm->bankMan()->unpack(2, 31, 7);

		_vm->display()->horizontalScroll(320);

		joeFacing(DIR_RIGHT);
		joeCutFacing(DIR_RIGHT);
		joePrevFacing(DIR_RIGHT);
	}

	joeFace();
	pbs->curPos(oldx, oldy);
	pbs->frameNum = 31;
}

uint16 Logic::joeFace() {
	debug(9, "Logic::joeFace() - curFace = %d, prevFace = %d", _joe.facing, _joe.prevFacing);
	BobSlot *pbs = _vm->graphics()->bob(0);
	uint16 frame;
	if (_currentRoom == 108) {
		frame = 1;
	} else {
		frame = 35;
		if (joeFacing() == DIR_FRONT) {
			if (joePrevFacing() == DIR_BACK) {
				pbs->frameNum = 35;
				_vm->update();
			}
			frame = 36;
		} else if (joeFacing() == DIR_BACK) {
			if (joePrevFacing() == DIR_FRONT) {
				pbs->frameNum = 35;
				_vm->update();
			}
			frame = 37;
		} else if ((joeFacing() == DIR_LEFT && joePrevFacing() == DIR_RIGHT)
			||  (joeFacing() == DIR_RIGHT && joePrevFacing() == DIR_LEFT)) {
			pbs->frameNum = 36;
			_vm->update();
		}
		pbs->frameNum = frame;
		pbs->scale = joeScale();
		pbs->xflip = (joeFacing() == DIR_LEFT);
		_vm->update();
		joePrevFacing(joeFacing());
		switch (frame) {
		case 35:
			frame = 1;
			break;
		case 36:
			frame = 3;
			break;
		case 37:
			frame = 5;
			break;
		}
	}
	pbs->frameNum = 31;
	_vm->bankMan()->unpack(frame, pbs->frameNum, 7);
	return frame;
}

void Logic::joeGrab(int16 grabState) {
	uint16 frame = 0;
	BobSlot *bobJoe = _vm->graphics()->bob(0);

	switch (grabState) {
	case STATE_GRAB_NONE:
		break;
	case STATE_GRAB_MID:
		if (joeFacing() == DIR_BACK) {
			frame = 6;
		} else if (joeFacing() == DIR_FRONT) {
			frame = 4;
		} else {
			frame = 2;
		}
		break;
	case STATE_GRAB_DOWN:
		if (joeFacing() == DIR_BACK) {
			frame = 9;
		} else {
			frame = 8;
		}
		break;
	case STATE_GRAB_UP:
		// turn back
		_vm->bankMan()->unpack(5, 31, 7);
		bobJoe->xflip = (joeFacing() == DIR_LEFT);
		bobJoe->scale = joeScale();
		_vm->update();
		// grab up
		_vm->bankMan()->unpack(7, 31, 7);
		bobJoe->xflip = (joeFacing() == DIR_LEFT);
		bobJoe->scale = joeScale();
		_vm->update();
		// turn back
		frame = 7;
		break;
	}

	if (frame != 0) {
		_vm->bankMan()->unpack(frame, 31, 7);
		bobJoe->xflip = (joeFacing() == DIR_LEFT);
		bobJoe->scale = joeScale();
		_vm->update();

		// extra delay for grab down
		if (grabState == STATE_GRAB_DOWN) {
			_vm->update();
			_vm->update();
		}
	}
}

void Logic::joeUseDress(bool showCut) {
	if (showCut) {
		joeFacing(DIR_FRONT);
		joeFace();
		if (gameState(VAR_JOE_DRESSING_MODE) == 0) {
			playCutaway("CDRES.CUT");
			inventoryInsertItem(ITEM_CLOTHES);
		} else {
			playCutaway("CUDRS.CUT");
		}
	}
	_vm->display()->palSetJoeDress();
	loadJoeBanks("JOED_A.BBK", "JOED_B.BBK");
	inventoryDeleteItem(ITEM_DRESS);
	gameState(VAR_JOE_DRESSING_MODE, 2);
}

void Logic::joeUseClothes(bool showCut) {
	if (showCut) {
		joeFacing(DIR_FRONT);
		joeFace();
		playCutaway("CDCLO.CUT");
		inventoryInsertItem(ITEM_DRESS);
	}
	_vm->display()->palSetJoeNormal();
	loadJoeBanks("JOE_A.BBK", "JOE_B.BBK");
	inventoryDeleteItem(ITEM_CLOTHES);
	gameState(VAR_JOE_DRESSING_MODE, 0);
}

void Logic::joeUseUnderwear() {
	_vm->display()->palSetJoeNormal();
	loadJoeBanks("JOEU_A.BBK", "JOEU_B.BBK");
	gameState(VAR_JOE_DRESSING_MODE, 1);
}

void Logic::makePersonSpeak(const char *sentence, Person *person, const char *voiceFilePrefix) {
	_vm->command()->clear(false);
	Talk::speak(sentence, person, voiceFilePrefix, _vm);
}

void Logic::startDialogue(const char *dlgFile, int personInRoom, char *cutaway) {
	ObjectData *data = objectData(_roomData[_currentRoom] + personInRoom);
	if (data->name > 0 && data->entryObj <= 0) {
		if (State::findTalk(data->state) == STATE_TALK_MUTE) {
			// 'I can't talk to that'
			makeJoeSpeak(24 + _vm->randomizer.getRandomNumber(2));
		} else {
			char cutawayFile[20];
			if (cutaway == NULL) {
				cutaway = cutawayFile;
			}
			_vm->display()->fullscreen(true);
			Talk::talk(dlgFile, personInRoom, cutaway, _vm);
			if (!cutaway[0]) {
				_vm->display()->fullscreen(false);
			}
		}
	}
}

void Logic::playCutaway(const char *cutFile, char *next) {
	char nextFile[20];
	if (next == NULL) {
		next = nextFile;
	}
	_vm->display()->clearTexts(CmdText::COMMAND_Y_POS, CmdText::COMMAND_Y_POS);
	Cutaway::run(cutFile, next, _vm);
}

void Logic::makeJoeSpeak(uint16 descNum, bool objectType) {
	const char *text = objectType ? objectTextualDescription(descNum) : joeResponse(descNum);
	if (objectType) {
		descNum += JOE_RESPONSE_MAX;
	}
	char descFilePrefix[10];
	sprintf(descFilePrefix, "JOE%04i", descNum);
	makePersonSpeak(text, NULL, descFilePrefix);
}

uint16 Logic::findInventoryItem(int invSlot) const {
	// queen.c l.3894-3898
	if (invSlot >= 0 && invSlot < 4) {
		return _inventoryItem[invSlot];
	}
	return 0;
}

void Logic::inventorySetup() {
	_vm->bankMan()->load("OBJECTS.BBK", 14);
	if (_vm->resource()->isInterview()) {
		_inventoryItem[0] = 1;
		_inventoryItem[1] = 2;
		_inventoryItem[2] = 3;
		_inventoryItem[3] = 4;
	} else {
		_inventoryItem[0] = ITEM_BAT;
		_inventoryItem[1] = ITEM_JOURNAL;
		_inventoryItem[2] = ITEM_NONE;
		_inventoryItem[3] = ITEM_NONE;
	}
}

void Logic::inventoryRefresh() {
	uint16 x = 182;
	for (int i = 0; i < 4; ++i) {
		uint16 itemNum = _inventoryItem[i];
		if (itemNum != 0) {
			uint16 dstFrame = (i == 0) ? 8 : 9;
			// unpack frame for object and draw it
			_vm->bankMan()->unpack(_itemData[itemNum].frame, dstFrame, 14);
			_vm->graphics()->drawInventoryItem(dstFrame, x, 14);
		} else {
			// no object, clear the panel
			_vm->graphics()->drawInventoryItem(0, x, 14);
		}
		x += 35;
	}
}

int16 Logic::previousInventoryItem(int16 first) const {
	int i;
	for (i = first - 1; i >= 1; i--)
		if (_itemData[i].name > 0)
			return i;
	for (i = _numItems; i > first; i--)
		if (_itemData[i].name > 0)
			return i;

	return 0;	//nothing found
}

int16 Logic::nextInventoryItem(int16 first) const {
	int i;
	for (i = first + 1; i < _numItems; i++)
		if (_itemData[i].name > 0)
			return i;
	for (i = 1; i < first; i++)
		if (_itemData[i].name > 0)
			return i;

	return 0;	//nothing found
}

void Logic::removeDuplicateItems() {
	for (int i = 0; i < 4; i++)
		for (int j = i + 1; j < 4; j++)
			if (_inventoryItem[i] == _inventoryItem[j])
				_inventoryItem[j] = ITEM_NONE;
}

uint16 Logic::numItemsInventory() const {
	uint16 count = 0;
	for (int i = 1; i < _numItems; i++)
		if (_itemData[i].name > 0)
			count++;

	return count;
}

void Logic::inventoryInsertItem(uint16 itemNum, bool refresh) {
	int16 item = _inventoryItem[0] = (int16)itemNum;
	_itemData[itemNum].name = ABS(_itemData[itemNum].name);	//set visible
	for (int i = 1; i < 4; i++) {
		item = nextInventoryItem(item);
		_inventoryItem[i] = item;
		removeDuplicateItems();
	}

	if (refresh)
		inventoryRefresh();
}

void Logic::inventoryDeleteItem(uint16 itemNum, bool refresh) {
	int16 item = (int16)itemNum;
	_itemData[itemNum].name = -ABS(_itemData[itemNum].name);	//set invisible
	for (int i = 0; i < 4; i++) {
		item = nextInventoryItem(item);
		_inventoryItem[i] = item;
		removeDuplicateItems();
	}

	if (refresh)
		inventoryRefresh();
}

void Logic::inventoryScroll(uint16 count, bool up) {
	if (!(numItemsInventory() > 4))
		return;
	while (count--) {
		if (up) {
			for (int i = 3; i > 0; i--)
				_inventoryItem[i] = _inventoryItem[i - 1];
			_inventoryItem[0] = previousInventoryItem(_inventoryItem[0]);
		} else {
			for (int i = 0; i < 3; i++)
				_inventoryItem[i] = _inventoryItem[i + 1];
			_inventoryItem[3] = nextInventoryItem(_inventoryItem[3]);
		}
	}

	inventoryRefresh();
}

void Logic::removeHotelItemsFromInventory() {
	if (currentRoom() == 1 && gameState(VAR_HOTEL_ITEMS_REMOVED) == 0) {
		inventoryDeleteItem(ITEM_CROWBAR, false);
		inventoryDeleteItem(ITEM_DRESS, false);
		inventoryDeleteItem(ITEM_CLOTHES, false);
		inventoryDeleteItem(ITEM_HAY, false);
		inventoryDeleteItem(ITEM_OIL, false);
		inventoryDeleteItem(ITEM_CHICKEN, false);
		gameState(VAR_HOTEL_ITEMS_REMOVED, 1);
		inventoryRefresh();
	}
}

void Logic::objectCopy(int dummyObjectIndex, int realObjectIndex) {
	// copy data from dummy object to real object, if COPY_FROM object
	// images are greater than COPY_TO Object images then swap the objects around.

	ObjectData *dummyObject = objectData(dummyObjectIndex);
	ObjectData *realObject  = objectData(realObjectIndex);

	int fromState = (dummyObject->name < 0) ? -1 : 0;

	int frameCountReal  = 1;
	int frameCountDummy = 1;

	int graphic = realObject->image;
	if (graphic > 0) {
		if (graphic > 5000)
			graphic -= 5000;

		GraphicData *data = graphicData(graphic);

		if (data->lastFrame > 0)
			frameCountReal = data->lastFrame - data->firstFrame + 1;

		graphic = dummyObject->image;
		if (graphic > 0) {
			if (graphic > 5000)
				graphic -= 5000;

			data = graphicData(graphic);

			if (data->lastFrame > 0)
				frameCountDummy = data->lastFrame - data->firstFrame + 1;
		}
	}

	ObjectData temp = *realObject;
	*realObject = *dummyObject;

	if (frameCountDummy > frameCountReal)
		*dummyObject = temp;

	realObject->name = ABS(realObject->name);

	if (fromState == -1)
		dummyObject->name = -ABS(dummyObject->name);

	for (int i = 1; i <= _numWalkOffs; i++) {
		WalkOffData *walkOff = &_walkOffData[i];
		if (walkOff->entryObj == (int16)dummyObjectIndex) {
			walkOff->entryObj = (int16)realObjectIndex;
			break;
		}
	}
}

void Logic::handleSpecialArea(Direction facing, uint16 areaNum, uint16 walkDataNum) {
	// queen.c l.2838-2911
	debug(9, "handleSpecialArea(%d, %d, %d)\n", facing, areaNum, walkDataNum);

	// Stop animating Joe
	_vm->graphics()->bob(0)->animating = false;

	// Make Joe face the right direction
	joeFacing(facing);
	joeFace();

	_newRoom = 0;
	_entryObj = 0;

	char nextCut[20];
	memset(nextCut, 0, sizeof(nextCut));

	switch (_currentRoom) {
	case ROOM_JUNGLE_BRIDGE:
		makeJoeSpeak(16);
		break;
	case ROOM_JUNGLE_GORILLA_1:
		playCutaway("C6C.CUT", nextCut);
		break;
	case ROOM_JUNGLE_GORILLA_2:
		playCutaway("C14B.CUT", nextCut);
		break;
	case ROOM_AMAZON_ENTRANCE:
		if (areaNum == 3) {
			playCutaway("C16A.CUT", nextCut);
		}
		break;
	case ROOM_AMAZON_HIDEOUT:
		if (walkDataNum == 4) {
			playCutaway("C17A.CUT", nextCut);
		} else if (walkDataNum == 2) {
			playCutaway("C17B.CUT", nextCut);
		}
		break;
	case ROOM_FLODA_OUTSIDE:
		playCutaway("C22A.CUT", nextCut);
		break;
	case ROOM_FLODA_KITCHEN:
		playCutaway("C26B.CUT", nextCut);
		break;
	case ROOM_FLODA_KLUNK:
		playCutaway("C30A.CUT", nextCut);
		break;
	case ROOM_FLODA_HENRY:
		playCutaway("C32C.CUT", nextCut);
		break;
	case ROOM_TEMPLE_ZOMBIES:
		if (areaNum == 6) {
			switch (gameState(VAR_BYPASS_ZOMBIES)) {
			case 0:
				playCutaway("C50D.CUT", nextCut);
				while (nextCut[0] != '\0') {
					playCutaway(nextCut, nextCut);
				}
				gameState(VAR_BYPASS_ZOMBIES, 1);
				break;
			case 1:
				playCutaway("C50H.CUT", nextCut);
				break;
			}
		}
		break;
	case ROOM_TEMPLE_SNAKE:
		playCutaway("C53B.CUT", nextCut);
		break;
	case ROOM_TEMPLE_LIZARD_LASER:
		makeJoeSpeak(19);
		break;
	case ROOM_HOTEL_DOWNSTAIRS:
		makeJoeSpeak(21);
		break;
	case ROOM_HOTEL_LOBBY:
		switch (gameState(VAR_HOTEL_ESCAPE_STATE)) {
		case 0:
			playCutaway("C73A.CUT");
			joeUseUnderwear();
			joeFace();
			gameState(VAR_HOTEL_ESCAPE_STATE, 1);
			break;
		case 1:
			playCutaway("C73B.CUT");
			gameState(VAR_HOTEL_ESCAPE_STATE, 2);
			break;
		case 2:
			playCutaway("C73C.CUT");
			break;
		}
		break;
	case ROOM_TEMPLE_MAZE_5:
		if (areaNum == 7) {
			makeJoeSpeak(17);
		}
		break;
	case ROOM_TEMPLE_MAZE_6:
		if (areaNum == 5 && gameState(187) == 0) {
			playCutaway("C101B.CUT", nextCut);
		}
		break;
	case ROOM_FLODA_FRONTDESK:
		if (areaNum == 3) {
			switch (gameState(VAR_BYPASS_FLODA_RECEPTIONIST)) {
			case 0:
				playCutaway("C103B.CUT", nextCut);
				gameState(VAR_BYPASS_FLODA_RECEPTIONIST, 1);
				break;
			case 1:
				playCutaway("C103E.CUT", nextCut);
				break;
			}
		}
		break;
	}

	while (strlen(nextCut) > 4 &&
		scumm_stricmp(nextCut + strlen(nextCut) - 4, ".CUT") == 0) {
		playCutaway(nextCut, nextCut);
	}
}

void Logic::handlePinnacleRoom() {
	// camera does not follow Joe anymore
	_vm->graphics()->putCameraOnBob(-1);
	displayRoom(ROOM_JUNGLE_PINNACLE, RDM_NOFADE_JOE, 100, 2, true);

	BobSlot *joe   = _vm->graphics()->bob(6);
	BobSlot *piton = _vm->graphics()->bob(7);

	// set scrolling value to mouse position to avoid glitch
	Common::Point mouse = _vm->input()->getMousePos();
	_vm->display()->horizontalScroll(mouse.x);

	joe->x = piton->x = 3 * mouse.x / 4 + 200;
	joe->frameNum = mouse.x / 36 + 45;

	// bobs have been unpacked from animating objects, we don't need them
	// to animate anymore ; so turn animation off
	joe->animating = piton->animating = false;

	_vm->update();
	_vm->display()->palFadeIn(ROOM_JUNGLE_PINNACLE, joe->active, joe->x, joe->y);

	_entryObj = 0;
	uint16 prevObj = 0;
	CmdText *cmdText = CmdText::makeCmdTextInstance(5, _vm);
	cmdText->setVerb(VERB_WALK_TO);
	while (!_vm->shouldQuit() && (_vm->input()->mouseButton() == 0 || _entryObj == 0)) {

		_vm->update();
		mouse = _vm->input()->getMousePos();

		// update bobs position / frame
		joe->x = piton->x = 3 * mouse.x / 4 + 200;
		joe->frameNum = mouse.x / 36 + 45;

		_vm->display()->clearTexts(5, 5);

		uint16 curObj = _vm->grid()->findObjectUnderCursor(mouse.x, mouse.y);
		if (curObj != 0 && curObj != prevObj) {
			_entryObj = 0;
			curObj += currentRoomData(); // global object number
			ObjectData *objData = objectData(curObj);
			if (objData->name > 0) {
				_entryObj = objData->entryObj;
				cmdText->displayTemp(INK_PINNACLE_ROOM, objectName(objData->name), true);
			}
			prevObj = curObj;
		}

		// update screen scrolling
		_vm->display()->horizontalScroll(mouse.x);
	}
	delete cmdText;
	_vm->input()->clearMouseButton();

	_newRoom = objectData(_entryObj)->room;

	// Only a few commands can be triggered from this room :
	// piton -> crash  : 0x216 (obj1=0x2a, song=3)
	// piton -> floda  : 0x217 (obj1=0x29, song=16)
	// piton -> bob    : 0x219 (obj1=0x2f, song=6)
	// piton -> embark : 0x218 (obj1=0x2c, song=7)
	// piton -> jungle : 0x20B (obj1=0x2b, song=3)
	// piton -> amazon : 0x21A (obj1=0x30, song=3)
	//
	// Because none of these update objects/areas/gamestate, the EXECUTE_ACTION()
	// call, as the original does, is useless. All we have to do is the playsong
	// call (all songs have the PLAY_BEFORE type). This way we could get rid of
	// the hack described in execute.c l.334-339.
	struct {
		uint16 obj;
		int16 song;
	} cmds[] = {
		{ 0x2A,  3 },
		{ 0x29, 16 },
		{ 0x2F,  6 },
		{ 0x2C,  7 },
		{ 0x2B,  3 },
		{ 0x30,  3 }
	};
	for (int i = 0; i < ARRAYSIZE(cmds); ++i) {
		if (cmds[i].obj == prevObj) {
			_vm->sound()->playSong(cmds[i].song);
			break;
		}
	}

	joe->active = piton->active = false;
	_vm->display()->clearTexts(5, 5);

	// camera follows Joe again
	_vm->graphics()->putCameraOnBob(0);

	_vm->display()->palFadeOut(ROOM_JUNGLE_PINNACLE);
}

void Logic::update() {
	if (_credits)
		_credits->update();

	if (_vm->debugger()->flags() & Debugger::DF_DRAW_AREAS) {
		_vm->grid()->drawZones();
	}
}

void Logic::saveState(byte *&ptr) {
	uint16 i;
	for (i = 0; i < 4; i++) {
		WRITE_BE_UINT16(ptr, _inventoryItem[i]); ptr += 2;
	}

	WRITE_BE_UINT16(ptr, _vm->graphics()->bob(0)->x); ptr += 2;
	WRITE_BE_UINT16(ptr, _vm->graphics()->bob(0)->y); ptr += 2;

	WRITE_BE_UINT16(ptr, _currentRoom); ptr += 2;

	for (i = 1; i <= _numObjects; i++)
		_objectData[i].writeToBE(ptr);

	for (i = 1; i <= _numItems; i++)
		_itemData[i].writeToBE(ptr);

	for (i = 0; i < GAME_STATE_COUNT; i++) {
		WRITE_BE_UINT16(ptr, _gameState[i]); ptr += 2;
	}

	for (i = 0; i < TALK_SELECTED_COUNT; i++)
		_talkSelected[i].writeToBE(ptr);

	for (i = 1; i <= _numWalkOffs; i++)
		_walkOffData[i].writeToBE(ptr);

	WRITE_BE_UINT16(ptr, _joe.facing); ptr += 2;

	// V1
	WRITE_BE_UINT16(ptr, _puzzleAttemptCount); ptr += 2;
	for (i = 1; i <= _numObjDesc; i++)
		_objectDescription[i].writeToBE(ptr);
}

void Logic::loadState(uint32 ver, byte *&ptr) {
	uint16 i;
	for (i = 0; i < 4; i++) {
		_inventoryItem[i] = (int16)READ_BE_INT16(ptr); ptr += 2;
	}

	_joe.x = (int16)READ_BE_INT16(ptr); ptr += 2;
	_joe.y = (int16)READ_BE_INT16(ptr); ptr += 2;

	_currentRoom = READ_BE_UINT16(ptr); ptr += 2;

	for (i = 1; i <= _numObjects; i++)
		_objectData[i].readFromBE(ptr);

	for (i = 1; i <= _numItems; i++)
		_itemData[i].readFromBE(ptr);

	for (i = 0; i < GAME_STATE_COUNT; i++) {
		_gameState[i] = (int16)READ_BE_INT16(ptr); ptr += 2;
	}

	for (i = 0; i < TALK_SELECTED_COUNT; i++)
		_talkSelected[i].readFromBE(ptr);

	for (i = 1; i <= _numWalkOffs; i++)
		_walkOffData[i].readFromBE(ptr);

	_joe.facing = READ_BE_UINT16(ptr); ptr += 2;

	if (ver >= 1) {
		_puzzleAttemptCount = READ_BE_UINT16(ptr); ptr += 2;

		for (i = 1; i <= _numObjDesc; i++)
			_objectDescription[i].readFromBE(ptr);
	}
}

void Logic::setupRestoredGame() {
	_vm->sound()->playLastSong();

	switch (gameState(VAR_JOE_DRESSING_MODE)) {
	case 0:
		_vm->display()->palSetJoeNormal();
		loadJoeBanks("JOE_A.BBK", "JOE_B.BBK");
		break;
	case 1:
		_vm->display()->palSetJoeNormal();
		loadJoeBanks("JOEU_A.BBK", "JOEU_B.BBK");
		break;
	case 2:
		_vm->display()->palSetJoeDress();
		loadJoeBanks("JOED_A.BBK", "JOED_B.BBK");
		break;
	}

	BobSlot *pbs = _vm->graphics()->bob(0);
	pbs->xflip = (joeFacing() == DIR_LEFT);
	joePrevFacing(joeFacing());
	joeCutFacing(joeFacing());
	switch (joeFacing()) {
	case DIR_FRONT:
		pbs->frameNum = 36;
		_vm->bankMan()->unpack(3, 31, 7);
		break;
	case DIR_BACK:
		pbs->frameNum = 37;
		_vm->bankMan()->unpack(5, 31, 7);
		break;
	default:
		pbs->frameNum = 35;
		_vm->bankMan()->unpack(1, 31, 7);
		break;
	}

	_oldRoom = 0;
	_newRoom = _currentRoom;
	_entryObj = 0;

	if (_vm->bam()->_flag != BamScene::F_STOP) {
		_vm->bam()->prepareAnimation();
	}

	inventoryRefresh();
}

void Logic::sceneStart() {
	debug(6, "[Logic::sceneStart] _scene = %i", _scene);
	_scene++;

	_vm->display()->showMouseCursor(false);

	if (1 == _scene) {
		_vm->display()->palGreyPanel();
	}

	_vm->update();
}

void Logic::sceneStop() {
	debug(6, "[Logic::sceneStop] _scene = %i", _scene);
	_scene--;

	if (_scene > 0)
		return;

	_vm->display()->palSetAllDirty();
	_vm->display()->showMouseCursor(true);
	_vm->grid()->setupPanel();
}

void Logic::changeRoom() {
	if (!changeToSpecialRoom())
		displayRoom(currentRoom(), RDM_FADE_JOE, 100, 1, false);
	_vm->display()->showMouseCursor(true);
}

void Logic::executeSpecialMove(uint16 sm) {
	debug(6, "Special move: %d", sm);
	if (sm < ARRAYSIZE(_specialMoves) && _specialMoves[sm] != 0) {
		(this->*_specialMoves[sm])();
	}
}

void Logic::asmMakeJoeUseDress() {
	joeUseDress(false);
}

void Logic::asmMakeJoeUseNormalClothes() {
	joeUseClothes(false);
}

void Logic::asmMakeJoeUseUnderwear() {
	joeUseUnderwear();
}

void Logic::asmSwitchToDressPalette() {
	_vm->display()->palSetJoeDress();
}

void Logic::asmSwitchToNormalPalette() {
	_vm->display()->palSetJoeNormal();
}

void Logic::asmStartCarAnimation() {
	_vm->bam()->_flag = BamScene::F_PLAY;
	_vm->bam()->prepareAnimation();
}

void Logic::asmStopCarAnimation() {
	_vm->bam()->_flag = BamScene::F_STOP;
	_vm->graphics()->bob(findBob(594))->active = false; // oil object
	_vm->graphics()->bob(7)->active = false; // gun shots
}

void Logic::asmStartFightAnimation() {
	_vm->bam()->_flag = BamScene::F_PLAY;
	_vm->bam()->prepareAnimation();
	gameState(148, 1);
}

void Logic::asmWaitForFrankPosition() {
	_vm->bam()->_flag = BamScene::F_REQ_STOP;
	while (_vm->bam()->_flag != BamScene::F_STOP) {
		_vm->update();
	}
}

void Logic::asmMakeFrankGrowing() {
	_vm->bankMan()->unpack(1, 38, 15);
	BobSlot *bobFrank = _vm->graphics()->bob(5);
	bobFrank->frameNum = 38;
	if (_vm->resource()->getPlatform() == Common::kPlatformAmiga) {
		bobFrank->active = true;
		bobFrank->x = 160;
		bobFrank->scale = 100;
		for (int i = 350; i >= 200; i -= 5) {
			bobFrank->y = i;
			_vm->update();
		}
	} else {
		bobFrank->curPos(160, 200);
		for (int i = 10; i <= 100; i += 4) {
			bobFrank->scale = i;
			_vm->update();
		}
	}
	for (int i = 0; i <= 20; ++i) {
		_vm->update();
	}

	objectData(521)->name =  ABS(objectData(521)->name); // Dinoray
	objectData(526)->name =  ABS(objectData(526)->name); // Frank obj
	objectData(522)->name = -ABS(objectData(522)->name); // TMPD object off
	objectData(525)->name = -ABS(objectData(525)->name); // Floda guards off
	objectData(523)->name = -ABS(objectData(523)->name); // Sparky object off
	gameState(157, 1); // No more Ironstein
}

void Logic::asmMakeRobotGrowing() {
	_vm->bankMan()->unpack(1, 38, 15);
	BobSlot *bobRobot = _vm->graphics()->bob(5);
	bobRobot->frameNum = 38;
	if (_vm->resource()->getPlatform() == Common::kPlatformAmiga) {
		bobRobot->active = true;
		bobRobot->x = 160;
		bobRobot->scale = 100;
		for (int i = 350; i >= 200; i -= 5) {
			bobRobot->y = i;
			_vm->update();
		}
	} else {
		bobRobot->curPos(160, 200);
		for (int i = 10; i <= 100; i += 4) {
			bobRobot->scale = i;
			_vm->update();
		}
	}
	for (int i = 0; i <= 20; ++i) {
		_vm->update();
	}

	objectData(524)->name = -ABS(objectData(524)->name); // Azura object off
	objectData(526)->name = -ABS(objectData(526)->name); // Frank object off
}

void Logic::asmShrinkRobot() {
	int i;
	BobSlot *robot = _vm->graphics()->bob(6);
	for (i = 100; i >= 35; i -= 5) {
		robot->scale = i;
		_vm->update();
	}
}

void Logic::asmEndGame() {
	int n = 40;
	while (n--) {
		_vm->update();
	}
//	debug("Game completed.");
	_vm->quitGame();
}

void Logic::asmPutCameraOnDino() {
	_vm->graphics()->putCameraOnBob(-1);
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx < 320) {
		scrollx += 16;
		if (scrollx > 320) {
			scrollx = 320;
		}
		_vm->display()->horizontalScroll(scrollx);
		_vm->update();
	}
	_vm->graphics()->putCameraOnBob(1);
}

void Logic::asmPutCameraOnJoe() {
	_vm->graphics()->putCameraOnBob(0);
}

void Logic::asmAltIntroPanRight() {
	_vm->graphics()->putCameraOnBob(-1);
	_vm->input()->fastMode(true);
	_vm->update();
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx < 285 && !_vm->input()->cutawayQuit()) {
		++scrollx;
		if (scrollx > 285) {
			scrollx = 285;
		}
		_vm->display()->horizontalScroll(scrollx);
		_vm->update();
	}
	_vm->input()->fastMode(false);
}

void Logic::asmAltIntroPanLeft() {
	_vm->graphics()->putCameraOnBob(-1);
	_vm->input()->fastMode(true);
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx > 0 && !_vm->input()->cutawayQuit()) {
		scrollx -= 4;
		if (scrollx < 0) {
			scrollx = 0;
		}
		_vm->display()->horizontalScroll(scrollx);
		_vm->update();
	}
	_vm->input()->fastMode(false);
}

void Logic::asmSetAzuraInLove() {
	gameState(VAR_AZURA_IN_LOVE, 1);
}

void Logic::asmPanRightFromJoe() {
	_vm->graphics()->putCameraOnBob(-1);
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx < 320) {
		scrollx += 16;
		if (scrollx > 320) {
			scrollx = 320;
		}
		_vm->display()->horizontalScroll(scrollx);
		_vm->update();
	}
}

void Logic::asmSetLightsOff() {
	_vm->display()->palCustomLightsOff(currentRoom());
}

void Logic::asmSetLightsOn() {
	_vm->display()->palCustomLightsOn(currentRoom());
}

void Logic::asmSetManequinAreaOn() {
	Area *a = _vm->grid()->area(ROOM_FLODA_FRONTDESK, 7);
	a->mapNeighbors = ABS(a->mapNeighbors);
}

void Logic::asmPanToJoe() {
	int i = _vm->graphics()->bob(0)->x - 160;
	if (i < 0) {
		i = 0;
	} else if (i > 320) {
		i = 320;
	}
	_vm->graphics()->putCameraOnBob(-1);
	int16 scrollx = _vm->display()->horizontalScroll();
	if (i < scrollx) {
		while (scrollx > i) {
			scrollx -= 16;
			if (scrollx < i) {
				scrollx = i;
			}
			_vm->display()->horizontalScroll(scrollx);
			_vm->update();
		}
	} else {
		while (scrollx < i) {
			scrollx += 16;
			if (scrollx > i) {
				scrollx = i;
			}
			_vm->display()->horizontalScroll(scrollx);
			_vm->update();
		}
		_vm->update();
	}
	_vm->graphics()->putCameraOnBob(0);
}

void Logic::asmTurnGuardOn() {
	gameState(VAR_GUARDS_TURNED_ON, 1);
}

void Logic::asmPanLeft320To144() {
	_vm->graphics()->putCameraOnBob(-1);
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx > 144) {
		scrollx -= 8;
		if (scrollx < 144) {
			scrollx = 144;
		}
		_vm->display()->horizontalScroll(scrollx);
		_vm->update();
	}
}

void Logic::asmSmooch() {
	_vm->graphics()->putCameraOnBob(-1);
	BobSlot *bobAzura = _vm->graphics()->bob(5);
	BobSlot *bobJoe = _vm->graphics()->bob(6);
	int16 scrollx = _vm->display()->horizontalScroll();
	while (scrollx < 320) {
		scrollx += 8;
		_vm->display()->horizontalScroll(scrollx);
		if (bobJoe->x - bobAzura->x > 128) {
			bobAzura->x += 10;
			bobJoe->x += 6;
		} else {
			bobAzura->x += 8;
			bobJoe->x += 8;
		}
		_vm->update();
	}
}

void Logic::asmSmoochNoScroll() {
	_vm->graphics()->putCameraOnBob(-1);
	BobSlot *bobAzura = _vm->graphics()->bob(5);
	BobSlot *bobJoe = _vm->graphics()->bob(6);
	for (int i = 0; i < 320; i += 8) {
		if (bobJoe->x - bobAzura->x > 128) {
			bobAzura->x += 2;
			bobJoe->x -= 2;
		}
		_vm->update();
	}
}

void Logic::asmMakeLightningHitPlane() {
	_vm->graphics()->putCameraOnBob(-1);
	short iy = 0, x, ydir = -1, j, k;

	BobSlot *planeBob     = _vm->graphics()->bob(5);
	BobSlot *lightningBob = _vm->graphics()->bob(20);

	planeBob->y = 135;

	if (_vm->resource()->getPlatform() == Common::kPlatformAmiga) {
		planeBob->scale = 100;
	} else {
		planeBob->scale = 20;
	}

	for (x = 660; x > 163; x -= 6) {
		planeBob->x = x;
		planeBob->y = 135 + iy;

		iy -= ydir;
		if (iy < -9 || iy > 9)
			ydir = -ydir;

		planeBob->scale++;
		if (planeBob->scale > 100)
			planeBob->scale = 100;

		int scrollX = x - 163;
		if (scrollX > 320)
			scrollX = 320;
		_vm->display()->horizontalScroll(scrollX);
		_vm->update();
	}

	planeBob->scale = 100;
	_vm->display()->horizontalScroll(0);

	planeBob->x += 8;
	planeBob->y += 6;

	lightningBob->x = 160;
	lightningBob->y = 0;

	_vm->sound()->playSfx(currentRoomSfx());

	_vm->bankMan()->unpack(18, lightningBob->frameNum, 15);
	_vm->bankMan()->unpack(4,  planeBob    ->frameNum, 15);

	// Plane plunges into the jungle!
	BobSlot *fireBob = _vm->graphics()->bob(6);

	fireBob->animating = true;
	fireBob->x = planeBob->x;
	fireBob->y = planeBob->y + 10;

	_vm->bankMan()->unpack(19, fireBob->frameNum, 15);
	_vm->update();

	k = 20;
	j = 1;

	for (x = 163; x > -30; x -= 10) {
		planeBob->y += 4;
		fireBob->y += 4;
		planeBob->x = fireBob->x = x;

		if (k < 40) {
			_vm->bankMan()->unpack(j, planeBob->frameNum, 15);
			_vm->bankMan()->unpack(k, fireBob ->frameNum, 15);
			k++;
			j++;

			if (j == 4)
				j = 1;
		}

		_vm->update();
	}

	_vm->graphics()->putCameraOnBob(0);
}

void Logic::asmScaleBlimp() {
	int16 z = 256;
	BobSlot *bob = _vm->graphics()->bob(7);
	int16 x = bob->x;
	int16 y = bob->y;
	bob->scale = 100;
	while (bob->x > 150) {
		bob->x = x * 256 / z + 150;
		bob->y = y * 256 / z + 112;
		if (_vm->resource()->getPlatform() != Common::kPlatformAmiga) {
			bob->scale = 100 * 256 / z;
		}
		++z;
		if (z % 6 == 0) {
			--x;
		}

		_vm->update();
	}
}

void Logic::asmScaleEnding() {
	_vm->graphics()->bob(7)->active = false; // Turn off blimp
	BobSlot *b = _vm->graphics()->bob(20);
	b->curPos(160, 100);
	if (_vm->resource()->getPlatform() != Common::kPlatformAmiga) {
		for (int i = 5; i <= 100; i += 5) {
			b->scale = i;
			_vm->update();
		}
	}
	for (int i = 0; i < 50; ++i) {
		_vm->update();
	}
	_vm->display()->palFadeOut(_currentRoom);
}

void Logic::asmWaitForCarPosition() {
	// Wait for car to reach correct position before pouring oil
	while (_vm->bam()->_index != 60) {
		_vm->update();
	}
}

void Logic::asmShakeScreen() {
	_vm->display()->shake(false);
	_vm->update();
	_vm->display()->shake(true);
	_vm->update();
}

void Logic::asmAttemptPuzzle() {
	++_puzzleAttemptCount;
	if (_puzzleAttemptCount == 4) {
		makeJoeSpeak(226, true);
		_puzzleAttemptCount = 0;
	}
}

void Logic::asmScaleTitle() {
	BobSlot *bob = _vm->graphics()->bob(5);
	bob->animating = false;
	bob->x = 161;
	bob->y = 200;
	bob->scale = 100;

	int i;
	for (i = 5; i <= 100; i +=5) {
		bob->scale = i;
		bob->y -= 4;
		_vm->update();
	}
}

void Logic::asmScrollTitle() {
	BobSlot *bob = _vm->graphics()->bob(5);
	bob->animating = false;
	bob->x = 161;
	bob->y = 300;
	bob->scale = 100;
	while (bob->y >= 120) {
		_vm->update();
		bob->y -= 4;
	}
}

void Logic::asmPanRightToHugh() {
	BobSlot *bob_thugA1 = _vm->graphics()->bob(20);
	BobSlot *bob_thugA2 = _vm->graphics()->bob(21);
	BobSlot *bob_thugA3 = _vm->graphics()->bob(22);
	BobSlot *bob_hugh1  = _vm->graphics()->bob(1);
	BobSlot *bob_hugh2  = _vm->graphics()->bob(23);
	BobSlot *bob_hugh3  = _vm->graphics()->bob(24);
	BobSlot *bob_thugB1 = _vm->graphics()->bob(25);
	BobSlot *bob_thugB2 = _vm->graphics()->bob(26);

	_vm->graphics()->putCameraOnBob(-1);
	_vm->input()->fastMode(true);
	_vm->update();

	// Adjust thug1 gun so it matches rest of body
	bob_thugA1->x += 160 - 45;
	bob_thugA2->x += 160;
	bob_thugA3->x += 160;

	bob_hugh1->x += 160 * 2;
	bob_hugh2->x += 160 * 2;
	bob_hugh3->x += 160 * 2;

	bob_thugB1->x += 160 * 3;
	bob_thugB2->x += 160 * 3;

	int horizontalScroll = 0;
	while (horizontalScroll < 160 && !_vm->input()->cutawayQuit()) {

		horizontalScroll += 8;
		if (horizontalScroll > 160)
			horizontalScroll = 160;

		_vm->display()->horizontalScroll(horizontalScroll);

		bob_thugA1->x -= 16;
		bob_thugA2->x -= 16;
		bob_thugA3->x -= 16;

		bob_hugh1->x -= 24;
		bob_hugh2->x -= 24;
		bob_hugh3->x -= 24;

		bob_thugB1->x -= 32;
		bob_thugB2->x -= 32;

		_vm->update();
	}

	_vm->input()->fastMode(false);
}

void Logic::asmMakeWhiteFlash() {
	_vm->display()->palCustomFlash();
}

void Logic::asmPanRightToJoeAndRita() { // cdint.cut
	BobSlot *bob_box   = _vm->graphics()->bob(20);
	BobSlot *bob_beam  = _vm->graphics()->bob(21);
	BobSlot *bob_crate = _vm->graphics()->bob(22);
	BobSlot *bob_clock = _vm->graphics()->bob(23);
	BobSlot *bob_hands = _vm->graphics()->bob(24);

	_vm->graphics()->putCameraOnBob(-1);
	_vm->input()->fastMode(true);

	_vm->update();

	bob_box  ->x += 280 * 2;
	bob_beam ->x += 30;
	bob_crate->x += 180 * 3;

	int horizontalScroll = _vm->display()->horizontalScroll();

	while (horizontalScroll < 290 && !_vm->input()->cutawayQuit()) {

		++horizontalScroll;
		if (horizontalScroll > 290)
			horizontalScroll = 290;

		_vm->display()->horizontalScroll(horizontalScroll);

		bob_box  ->x -= 2;
		bob_beam ->x -= 1;
		bob_crate->x -= 3;
		bob_clock->x -= 2;
		bob_hands->x -= 2;

		_vm->update();
	}
	_vm->input()->fastMode(false);
}

void Logic::asmPanLeftToBomb() {
	BobSlot *bob21 = _vm->graphics()->bob(21);
	BobSlot *bob22 = _vm->graphics()->bob(22);

	_vm->graphics()->putCameraOnBob(-1);
	_vm->input()->fastMode(true);

	int horizontalScroll = _vm->display()->horizontalScroll();

	while ((horizontalScroll > 0 || bob21->x < 136) && !_vm->input()->cutawayQuit()) {

		horizontalScroll -= 5;
		if (horizontalScroll < 0)
			horizontalScroll = 0;

		_vm->display()->horizontalScroll(horizontalScroll);

		if (horizontalScroll < 272 && bob21->x < 136)
			bob21->x += 2;

		bob22->x += 5;

		_vm->update();
	}

	_vm->input()->fastMode(false);
}

void Logic::asmEndDemo() {
//	debug("Flight of the Amazon Queen, released January 95.");
	_vm->quitGame();
}

void Logic::asmInterviewIntro() {
	// put camera on airship
	_vm->graphics()->putCameraOnBob(5);
	BobSlot *bas = _vm->graphics()->bob(5);

	bas->curPos(-30, 40);

	bas->move(700, 10, 3);
	int scale = 450;
	while (bas->moving && !_vm->input()->cutawayQuit()) {
		bas->scale = 256 * 100 / scale;
		--scale;
		if (scale < 256) {
			scale = 256;
		}
		_vm->update();
	}

	bas->scale = 90;
	bas->xflip = true;

	bas->move(560, 25, 4);
	while (bas->moving && !_vm->input()->cutawayQuit()) {
		_vm->update();
	}

	bas->move(545, 65, 2);
	while (bas->moving && !_vm->input()->cutawayQuit()) {
		_vm->update();
	}

	bas->move(540, 75, 2);
	while (bas->moving && !_vm->input()->cutawayQuit()) {
		_vm->update();
	}

	// put camera on Joe
	_vm->graphics()->putCameraOnBob(0);
}

void Logic::asmEndInterview() {
//	debug("Interactive Interview copyright (c) 1995, IBI.");
	_vm->quitGame();
}

void Logic::startCredits(const char *filename) {
	stopCredits();
	_credits = new Credits(_vm, filename);
}

void Logic::stopCredits() {
	if (_credits) {
		_vm->display()->clearTexts(0, 199);
		delete _credits;
		_credits = NULL;
	}
}

void LogicDemo::useJournal() {
	makePersonSpeak("This is a demo, so I can't load or save games*14", NULL, "");
}

bool LogicDemo::changeToSpecialRoom() {
	if (currentRoom() == FOTAQ_LOGO && gameState(VAR_INTRO_PLAYED) == 0) {
		currentRoom(79);
		displayRoom(currentRoom(), RDM_FADE_NOJOE, 100, 2, true);
		playCutaway("CLOGO.CUT");
		sceneReset();
		if (_vm->shouldQuit())
			return true;
		currentRoom(ROOM_HOTEL_LOBBY);
		entryObj(584);
		displayRoom(currentRoom(), RDM_FADE_JOE, 100, 2, true);
		playCutaway("C70D.CUT");
		gameState(VAR_INTRO_PLAYED, 1);
		inventoryRefresh();
		return true;
	}
	return false;
}

void LogicDemo::setupSpecialMoveTable() {
	_specialMoves[4] = &LogicDemo::asmMakeJoeUseUnderwear;
	_specialMoves[14] = &LogicDemo::asmEndDemo;
	if (_vm->resource()->getPlatform() == Common::kPlatformPC) {
		_specialMoves[5]  = &LogicDemo::asmSwitchToDressPalette;
	}
}

void LogicInterview::useJournal() {
	// no-op
}

bool LogicInterview::changeToSpecialRoom() {
	if (currentRoom() == 2 && gameState(2) == 0) {
		currentRoom(6);
		displayRoom(currentRoom(), RDM_FADE_NOJOE, 100, 2, true);
		playCutaway("START.CUT");
		gameState(2, 1);
		inventoryRefresh();
		return true;
	}
	return false;
}

void LogicInterview::setupSpecialMoveTable() {
	_specialMoves[1] = &LogicInterview::asmInterviewIntro;
	_specialMoves[2] = &LogicInterview::asmEndInterview;
}

void LogicGame::useJournal() {
	_vm->command()->clear(false);
	_journal->use();
	_vm->walk()->stopJoe();
}

bool LogicGame::changeToSpecialRoom() {
	if (currentRoom() == ROOM_JUNGLE_PINNACLE) {
		handlePinnacleRoom();
		return true;
	} else if (currentRoom() == FOTAQ_LOGO && gameState(VAR_INTRO_PLAYED) == 0) {
		displayRoom(currentRoom(), RDM_FADE_NOJOE, 100, 2, true);
		playCutaway("COPY.CUT");
		if (_vm->shouldQuit())
			return true;
		playCutaway("CLOGO.CUT");
		if (_vm->shouldQuit())
			return true;
		if (_vm->resource()->getPlatform() != Common::kPlatformAmiga) {
			if (ConfMan.getBool("alt_intro") && _vm->resource()->isCD()) {
				playCutaway("CINTR.CUT");
			} else {
				playCutaway("CDINT.CUT");
			}
		}
		if (_vm->shouldQuit())
			return true;
		playCutaway("CRED.CUT");
		if (_vm->shouldQuit())
			return true;
		_vm->display()->palSetPanel();
		sceneReset();
		currentRoom(ROOM_HOTEL_LOBBY);
		entryObj(584);
		displayRoom(currentRoom(), RDM_FADE_JOE, 100, 2, true);
		playCutaway("C70D.CUT");
		gameState(VAR_INTRO_PLAYED, 1);
		inventoryRefresh();
		return true;
	}
	return false;
}

void LogicGame::setupSpecialMoveTable() {
	_specialMoves[2] = &LogicGame::asmMakeJoeUseDress;
	_specialMoves[3] = &LogicGame::asmMakeJoeUseNormalClothes;
	_specialMoves[4] = &LogicGame::asmMakeJoeUseUnderwear;
	_specialMoves[7] = &LogicGame::asmStartCarAnimation;       // room 74
	_specialMoves[8] = &LogicGame::asmStopCarAnimation;        // room 74
	_specialMoves[9] = &LogicGame::asmStartFightAnimation;     // room 69
	_specialMoves[10] = &LogicGame::asmWaitForFrankPosition;   // c69e.cut
	_specialMoves[11] = &LogicGame::asmMakeFrankGrowing;       // c69z.cut
	_specialMoves[12] = &LogicGame::asmMakeRobotGrowing;       // c69z.cut
	_specialMoves[14] = &LogicGame::asmEndGame;
	_specialMoves[15] = &LogicGame::asmPutCameraOnDino;
	_specialMoves[16] = &LogicGame::asmPutCameraOnJoe;
	_specialMoves[19] = &LogicGame::asmSetAzuraInLove;
	_specialMoves[20] = &LogicGame::asmPanRightFromJoe;
	_specialMoves[21] = &LogicGame::asmSetLightsOff;
	_specialMoves[22] = &LogicGame::asmSetLightsOn;
	_specialMoves[23] = &LogicGame::asmSetManequinAreaOn;
	_specialMoves[24] = &LogicGame::asmPanToJoe;
	_specialMoves[25] = &LogicGame::asmTurnGuardOn;
	_specialMoves[26] = &LogicGame::asmPanLeft320To144;
	_specialMoves[27] = &LogicGame::asmSmoochNoScroll;
	_specialMoves[28] = &LogicGame::asmMakeLightningHitPlane;
	_specialMoves[29] = &LogicGame::asmScaleBlimp;
	_specialMoves[30] = &LogicGame::asmScaleEnding;
	_specialMoves[31] = &LogicGame::asmWaitForCarPosition;
	_specialMoves[33] = &LogicGame::asmAttemptPuzzle;
	_specialMoves[34] = &LogicGame::asmScrollTitle;
	if (_vm->resource()->getPlatform() == Common::kPlatformPC) {
		_specialMoves[5]  = &LogicGame::asmSwitchToDressPalette;
		_specialMoves[6]  = &LogicGame::asmSwitchToNormalPalette;
		_specialMoves[13] = &LogicGame::asmShrinkRobot;
		_specialMoves[17] = &LogicGame::asmAltIntroPanRight;      // cintr.cut
		_specialMoves[18] = &LogicGame::asmAltIntroPanLeft;       // cintr.cut
		_specialMoves[27] = &LogicGame::asmSmooch;
		_specialMoves[32] = &LogicGame::asmShakeScreen;
		_specialMoves[34] = &LogicGame::asmScaleTitle;
		_specialMoves[36] = &LogicGame::asmPanRightToHugh;
		_specialMoves[37] = &LogicGame::asmMakeWhiteFlash;
		_specialMoves[38] = &LogicGame::asmPanRightToJoeAndRita;
		_specialMoves[39] = &LogicGame::asmPanLeftToBomb;         // cdint.cut
	}
}

} // End of namespace Queen
