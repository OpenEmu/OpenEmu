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

#include "common/textconsole.h"

#include "queen/cutaway.h"

#include "queen/bankman.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/grid.h"
#include "queen/input.h"
#include "queen/logic.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"
#include "queen/talk.h"
#include "queen/walk.h"

namespace Queen {

void Cutaway::run(
		const char *filename,
		char *nextFilename,
		QueenEngine *vm) {
	Cutaway *cutaway = new Cutaway(filename, vm);
	cutaway->run(nextFilename);
	delete cutaway;
}

Cutaway::Cutaway(
		const char *filename,
		QueenEngine *vm)
	: _vm(vm), _personDataCount(0), _personFaceCount(0), _lastSong(0), _songBeforeComic(0) {
	memset(&_bankNames, 0, sizeof(_bankNames));
	_vm->input()->cutawayQuitReset();
	load(filename);
}

Cutaway::~Cutaway() {
	delete[] _fileData;
}

void Cutaway::load(const char *filename) {
	byte *ptr;

	debug(6, "----- Cutaway::load(\"%s\") -----", filename);

	ptr = _fileData = _vm->resource()->loadFile(filename, 20);

	if (0 == scumm_stricmp(filename, "COMIC.CUT"))
		_songBeforeComic = _vm->sound()->lastOverride();

	strcpy(_basename, filename);
	_basename[strlen(_basename)-4] = '\0';

	_comPanel = READ_BE_UINT16(ptr);
	ptr += 2;
	debug(6, "_comPanel = %i", _comPanel);
	_cutawayObjectCount = (int16)READ_BE_INT16(ptr);
	ptr += 2;

	debug(6, "_cutawayObjectCount = %i", _cutawayObjectCount);

	if (_cutawayObjectCount < 0) {
		_cutawayObjectCount = -_cutawayObjectCount;
		_vm->input()->canQuit(false);
	} else
		_vm->input()->canQuit(true);

	int16 flags1 = (int16)READ_BE_INT16(ptr);
	ptr += 2;
	debug(6, "flags1 = %i", flags1);

	if (flags1 < 0) {
		_vm->logic()->entryObj(0);
		_finalRoom = -flags1;
	} else
		_finalRoom = PREVIOUS_ROOM;

	_anotherCutaway = (flags1 == 1);

	debug(6, "[Cutaway::load] _finalRoom      = %i", _finalRoom);
	debug(6, "[Cutaway::load] _anotherCutaway = %i", _anotherCutaway);

	/*
		 Pointers to other places in the cutaway data
	 */

	_gameStatePtr = _fileData + READ_BE_UINT16(ptr);
	ptr += 2;

	_nextSentenceOff = READ_BE_UINT16(ptr);
	ptr += 2;

	uint16 bankNamesOff = READ_BE_UINT16(ptr);
	ptr += 2;

	_objectData = ptr;

	loadStrings(bankNamesOff);

	if (_bankNames[0][0]) {
		debug(6, "Loading bank '%s'", _bankNames[0]);
		_vm->bankMan()->load(_bankNames[0], CUTAWAY_BANK);
	}

	char entryString[MAX_STRING_SIZE];
	Talk::getString(_fileData, _nextSentenceOff, entryString, MAX_STRING_LENGTH);
	debug(6, "Entry string = '%s'", entryString);

	_vm->logic()->joeCutFacing(_vm->logic()->joeFacing());
	_vm->logic()->joeFace();

	if (entryString[0] == '*' &&
			entryString[1] == 'F' &&
			entryString[3] == '\0') {
		switch (entryString[2]) {
		case 'L':
			_vm->logic()->joeCutFacing(DIR_LEFT);
			break;
		case 'R':
			_vm->logic()->joeCutFacing(DIR_RIGHT);
			break;
		case 'F':
			_vm->logic()->joeCutFacing(DIR_FRONT);
			break;
		case 'B':
			_vm->logic()->joeCutFacing(DIR_BACK);
			break;
		}
	}

}

void Cutaway::loadStrings(uint16 offset) {
	int bankNameCount = READ_BE_UINT16(_fileData + offset);
	offset += 2;

	debug(6, "Bank name count = %i", bankNameCount);

	/*
		 The _bankNames zero-based array is the one-based BANK_NAMEstr array in
		 the original source code.
	 */

	for (int i = 0, j = 0; i < bankNameCount; i++) {
		Talk::getString(_fileData, offset, _bankNames[j], MAX_FILENAME_LENGTH);
		if (_bankNames[j][0]) {
			debug(6, "Bank name %i = '%s'", j, _bankNames[j]);
			j++;
		}
	}

	debug(6, "Getting talk file");
	Talk::getString(_fileData, offset, _talkFile, MAX_FILENAME_LENGTH);
	debug(6, "Talk file = '%s'", _talkFile);

	_talkTo = (int16)READ_BE_INT16(_fileData + offset);
	debug(6, "_talkTo = %i", _talkTo);
}

const byte *Cutaway::getCutawayObject(const byte *ptr, CutawayObject &object) {
	const byte *oldPtr = ptr;

	object.objectNumber = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.moveToX      = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.moveToY      = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.bank         = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.animList     = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.execute      = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.limitBobX1   = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.limitBobY1   = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.limitBobX2   = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.limitBobY2   = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.specialMove  = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.animType     = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.fromObject   = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.bobStartX    = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.bobStartY    = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.room         = (int16)READ_BE_INT16(ptr); ptr += 2;
	object.scale        = (int16)READ_BE_INT16(ptr); ptr += 2;

	if ((ptr - oldPtr) != 17*sizeof(int16))
		error("Wrong number of values read");

	// Make ugly reuse of data less ugly
	if (object.limitBobX1 < 0) {
		object.song = -object.limitBobX1;
		object.limitBobX1 = 0;
	} else
		object.song = 0;

	return ptr;
}

void Cutaway::dumpCutawayObject(int index, CutawayObject &object) {
	debug(6, "----- CutawayObject[%i] -----", index);

	const char *objectNumberStr;

	switch (object.objectNumber) {
	case -1:
		objectNumberStr = "MESSAGE";
		break;
	case 0:
		objectNumberStr = "Joe";
		break;
	default:
		if (object.objectNumber > 0)
			objectNumberStr = _vm->logic()->objectName(ABS(_vm->logic()->objectData(object.objectNumber)->name));
		else
			objectNumberStr = "Unknown!";
		break;
	}

	debug(6, "objectNumber = %i (%s)", object.objectNumber, objectNumberStr);

	if (object.moveToX) debug(6, "moveToX = %i", object.moveToX);
	if (object.moveToY) debug(6, "moveToY = %i", object.moveToY);
	if (object.bank) debug(6, "bank = %i", object.bank);
	if (object.animList) debug(6, "animList = %i", object.animList);
	if (object.execute) debug(6, "execute = %i", object.execute);
	if (object.limitBobX1) debug(6, "limitBobX1 = %i", object.limitBobX1);
	if (object.limitBobY1) debug(6, "limitBobY1 = %i", object.limitBobY1);
	if (object.limitBobX2) debug(6, "limitBobX2 = %i", object.limitBobX2);
	if (object.limitBobY2) debug(6, "limitBobY2 = %i", object.limitBobY2);
	if (object.specialMove) debug(6, "specialMove = %i", object.specialMove);
	if (object.animType) debug(6, "animType = %i", object.animType);
	if (object.fromObject) debug(6, "fromObject = %i", object.fromObject);
	if (object.bobStartX) debug(6, "bobStartX = %i", object.bobStartX);
	if (object.bobStartY) debug(6, "bobStartY = %i", object.bobStartY);
	if (object.room) debug(6, "room = %i", object.room);
	if (object.scale) debug(6, "scale = %i", object.scale);

}


const byte *Cutaway::turnOnPeople(const byte *ptr, CutawayObject &object) {
	// Lines 1248-1259 in cutaway.c
	object.personCount = (int16)READ_BE_INT16(ptr);
	ptr += 2;

	if (object.personCount > MAX_PERSON_COUNT)
		error("[Cutaway::turnOnPeople] object.personCount > MAX_PERSON_COUNT");

	for (int i = 0; i < object.personCount; i++) {
		object.person[i] = (int16)READ_BE_INT16(ptr);
		ptr += 2;
		debug(7, "[%i] Turn on person %i", i, object.person[i]);
	}

	return ptr;
}

void Cutaway::limitBob(CutawayObject &object) {
	if (object.limitBobX1) {

		if (object.objectNumber < 0) {
			warning("QueenCutaway::limitBob called with objectNumber = %i", object.objectNumber);
			return;
		}

		BobSlot *bob =
			_vm->graphics()->bob(_vm->logic()->findBob(object.objectNumber));

		if (!bob) {
			warning("Failed to find bob");
			return;
		}

		bob->box.x1 = object.limitBobX1;
		bob->box.y1 = object.limitBobY1;
		bob->box.x2 = object.limitBobX2;
		bob->box.y2 = object.limitBobY2;
	}
}

void Cutaway::restorePersonData() {
	for (int i = 0; i < _personDataCount; i++) {
		int index           = _personData[i].index;
		ObjectData *objectData  = _vm->logic()->objectData(index);
		objectData->name        = _personData[i].name;
		objectData->image       = _personData[i].image;
	}
}

void Cutaway::changeRooms(CutawayObject &object) {
	// Lines 1291-1385 in cutaway.c

	debug(6, "Changing from room %i to room %i",
			_temporaryRoom,
			object.room);

	restorePersonData();
	_personDataCount = 0;

	if (_finalRoom != object.room) {
		int firstObjectInRoom = _vm->logic()->roomData(object.room) + 1;
		int lastObjectInRoom  = _vm->logic()->roomData(object.room) + _vm->grid()->objMax(object.room);

		for (int i = firstObjectInRoom; i <= lastObjectInRoom; i++) {
			ObjectData *objectData  = _vm->logic()->objectData(i);

			if (objectData->image == -3 || objectData->image == -4) {

				assert(_personDataCount < MAX_PERSON_COUNT);
				//  The object is a person! So record the details...
				_personData[_personDataCount].index = i;
				_personData[_personDataCount].name  = objectData->name;
				_personData[_personDataCount].image = objectData->image;
				_personDataCount++;

				// Now, check to see if we need to keep the person on
				bool on = false;
				for (int j = 0; j < object.personCount; j++) {
					if (object.person[j] == i) {
						on = true;
						break;
					}
				}

				if (on) {
					// It is needed, so ensure it's ON
					objectData->name = ABS(objectData->name);
				} else {
					// Not needed, so switch off!
					objectData->name = -ABS(objectData->name);
				}

			}
		} // for ()
	}

	// set coordinates for Joe if he is on screen

	_vm->logic()->joePos(0, 0);

	for (int i = 0; i < object.personCount; i++) {
		if (PERSON_JOE == object.person[i]) {
			_vm->logic()->joePos(object.bobStartX, object.bobStartY);
		}
	}

	_vm->logic()->oldRoom(_initialRoom);

	// FIXME: Cutaway c41f is played at the end of the command 0x178. This command
	// setups some persons and associates bob slots to them. They should be hidden as
	// their y coordinate is > 150, but they aren't ! As a workaround, we display the room
	// with the panel area enabled. We do the same problem for cutaway c62c.
	int16 comPanel = _comPanel;
	if ((strcmp(_basename, "c41f") == 0 && _temporaryRoom == 106 && object.room == 41) ||
		(strcmp(_basename, "c62c") == 0 && _temporaryRoom == 105 && object.room == 41)) {
		comPanel = 1;
	}

	// Hide panel before displaying the 'head room' (ie. before palette fading). This doesn't
	// match the original engine, but looks better to me.
	if (object.room == FAYE_HEAD || object.room == AZURA_HEAD || object.room == FRANK_HEAD) {
		comPanel = 2;
	}

	RoomDisplayMode mode;

	if (!_vm->logic()->joeX() && !_vm->logic()->joeY()) {
		mode = RDM_FADE_NOJOE;
	} else {
		// We need to display Joe on screen
		if (_roomFade)
			mode = RDM_NOFADE_JOE;
		else
			mode = RDM_FADE_JOE_XY;
	}

	_vm->logic()->displayRoom(_vm->logic()->currentRoom(), mode, object.scale, comPanel, true);

	_currentImage = _vm->graphics()->numFrames();

	_temporaryRoom = _vm->logic()->currentRoom();

	restorePersonData();
}

Cutaway::ObjectType Cutaway::getObjectType(CutawayObject &object) {
	// Lines 1387-1449 in cutaway.c

	ObjectType objectType = OBJECT_TYPE_ANIMATION;

	if (object.objectNumber > 0) {
		if (!object.animList) {
			// No anim frames, so treat as a PERSON, ie. allow to speak/walk
			ObjectData *objectData = _vm->logic()->objectData(object.objectNumber);
			if (objectData->image == -3 || objectData->image == -4)
				objectType = OBJECT_TYPE_PERSON;
		}
	} else if (object.objectNumber == OBJECT_JOE) {
		// It's Joe. See if he's to be treated as a person.
		if (!object.animList) {
			// There's no animation list, so Joe must be talking.
			objectType = OBJECT_TYPE_PERSON;
		}
	}

	if (object.fromObject > 0) {
		/* Copy FROM_OBJECT into OBJECT */

		if (object.objectNumber != object.fromObject) {
			_vm->logic()->objectCopy(object.fromObject, object.objectNumber);
		} else {
			// Same object, so just turn it on!
			ObjectData *objectData = _vm->logic()->objectData(object.objectNumber);
			objectData->name = ABS(objectData->name);
		}

		_vm->graphics()->refreshObject(object.objectNumber);

		// Skip doing any anim stuff
		objectType = OBJECT_TYPE_NO_ANIMATION;
	}

	switch (object.objectNumber) {
	case -2:
		// Text to be spoken
		objectType = OBJECT_TYPE_TEXT_SPEAK;
		break;
	case -3:
		// Text to be displayed AND spoken
		objectType = OBJECT_TYPE_TEXT_DISPLAY_AND_SPEAK;
		break;
	case -4:
		// Text to be displayed only (not spoken)
		objectType = OBJECT_TYPE_TEXT_DISPLAY;
		break;
	}

	if (OBJECT_TYPE_ANIMATION == objectType && !object.execute) {
		// Execute is not on, and it's an object, so ignore any Anims
		objectType = OBJECT_TYPE_NO_ANIMATION;
	}

	return objectType;
}

const byte *Cutaway::getCutawayAnim(const byte *ptr, int header, CutawayAnim &anim) {
	// lines 1531-1607 in cutaway.c
	debug(6, "[Cutaway::getCutawayAnim] header=%i", header);

	anim.currentFrame = 0;
	anim.originalFrame = 0;

	if (-1 == header)
		header = 0;

	if (0 == header) {
		anim.object = 0;
		anim.originalFrame = 31;
	} else {
		anim.object = _vm->logic()->findBob(header);
		anim.originalFrame = _vm->logic()->findFrame(header);
	}

	anim.unpackFrame = (int16)READ_BE_INT16(ptr);
	ptr += 2;

	anim.speed = ((int16)READ_BE_INT16(ptr)) / 3 + 1;
	ptr += 2;

	anim.bank = (int16)READ_BE_INT16(ptr);
	ptr += 2;

	if (anim.bank == 0) {
		anim.bank = 15;
	} else {
		if (anim.bank != 13) {
			assert(anim.bank - 1 < MAX_BANK_NAME_COUNT);
			_vm->bankMan()->load(_bankNames[anim.bank-1], CUTAWAY_BANK);
			anim.bank = 8;
		} else {
			// Make sure we ref correct JOE bank (7)
			anim.bank = 7;
		}
	}

	anim.mx = (int16)READ_BE_INT16(ptr);
	ptr += 2;

	anim.my = (int16)READ_BE_INT16(ptr);
	ptr += 2;

	anim.cx = (int16)READ_BE_INT16(ptr);
	ptr += 2;

	anim.cy = (int16)READ_BE_INT16(ptr);
	ptr += 2;

	anim.scale = (int16)READ_BE_INT16(ptr);
	ptr += 2;

	if ((_vm->resource()->isDemo() && _vm->resource()->getPlatform() == Common::kPlatformPC) ||
		(_vm->resource()->isInterview() && _vm->resource()->getPlatform() == Common::kPlatformAmiga)) {
		anim.song = 0;
	} else {
		anim.song = (int16)READ_BE_INT16(ptr);
		ptr += 2;
	}

	// Extract information that depend on the signedness of values
	if (anim.unpackFrame < 0) {
		anim.flip = true;
		anim.unpackFrame = -anim.unpackFrame;
	} else
		anim.flip = false;

	return ptr;
}

void Cutaway::dumpCutawayAnim(CutawayAnim &anim) {
	debug(6, "----- CutawayAnim -----");
	if (anim.object) debug(6, "object = %i", anim.object);
	if (anim.unpackFrame) debug(6, "unpackFrame = %i", anim.unpackFrame);
	if (anim.speed) debug(6, "speed = %i", anim.speed);
	if (anim.bank) debug(6, "bank = %i", anim.bank);
	if (anim.mx) debug(6, "mx = %i", anim.mx);
	if (anim.my) debug(6, "my = %i", anim.my);
	if (anim.cx) debug(6, "cx = %i", anim.cx);
	if (anim.cy) debug(6, "cy = %i", anim.cy);
	if (anim.scale) debug(6, "scale = %i", anim.scale);
	if (anim.currentFrame) debug(6, "currentFrame = %i", anim.currentFrame);
	if (anim.originalFrame) debug(6, "originalFrame = %i", anim.originalFrame);
	if (anim.song) debug(6, "song = %i", anim.song);
}

const byte *Cutaway::handleAnimation(const byte *ptr, CutawayObject &object) {
	// lines 1517-1770 in cutaway.c
	int frameCount = 0;
	int header = 0;
	int i;

	CutawayAnim objAnim[56];

	// Read animation frames
	for (;;) {

		header = (int16)READ_BE_INT16(ptr);
		ptr += 2;

		if (-2 == header)
			break;

		//debug(6, "Animation frame %i, header = %i", frameCount, header);

		if (header > 1000)
			error("Header too large");

		ptr = getCutawayAnim(ptr, header, objAnim[frameCount]);
		//dumpCutawayAnim(objAnim[frameCount]);

		frameCount++;

		if (_vm->input()->cutawayQuit())
			return NULL;
	}

	if (object.animType == 1) {
		// lines 1615-1636 in cutaway.c

		debug(6, "----- Complex cutaway animation (animType = %i) -----", object.animType);

		if ((_vm->logic()->currentRoom() == 47 || _vm->logic()->currentRoom() == 63) &&
			objAnim[0].object == 1) {
			//CR 2 - 3/3/95, Special harcoded section to make Oracle work...
			makeComplexAnimation(_vm->graphics()->personFrames(1) - 1,  objAnim, frameCount);
		} else {
			_currentImage = makeComplexAnimation(_currentImage, objAnim, frameCount);
		}

		if (object.bobStartX || object.bobStartY) {
			BobSlot *bob = _vm->graphics()->bob(objAnim[0].object);
			bob->x = object.bobStartX;
			bob->y = object.bobStartY;
		}
	}

	// Setup the SYNCHRO bob channels

	for (i = 0; i < frameCount; i++) {
		if (objAnim[i].mx || objAnim[i].my) {
			BobSlot *bob = _vm->graphics()->bob(objAnim[i].object);
			bob->frameNum = objAnim[i].originalFrame;
			bob->move(objAnim[i].mx, objAnim[i].my,	(object.specialMove > 0) ? object.specialMove : 4);
			// Boat room hard coded
			if (_vm->logic()->currentRoom() == ROOM_TEMPLE_OUTSIDE) {
				BobSlot *bobJoe = _vm->graphics()->bob(0);
				if (bobJoe->x < 320) {
					bobJoe->move(bobJoe->x + 346, bobJoe->y,	4);
				}
			}
		}
	}

	// Normal cutaway

	if (object.animType != 1) {
		// lines 1657-1761 in cutaway.c

		debug(6, "----- Normal cutaway animation (animType = %i) -----", object.animType);

		for (i = 0; i < frameCount; i++) {
			//debug(6, "===== Animating frame %i =====", i);
			//dumpCutawayAnim(objAnim[i]);

			BobSlot *bob = _vm->graphics()->bob(objAnim[i].object);
			bob->active = true;
			if (bob->animating) {
				bob->animating = false;
				bob->frameNum = objAnim[i].originalFrame;
			}

			if (objAnim[i].object < 4)
				bob->frameNum = 31 + objAnim[i].object;

			if (objAnim[i].unpackFrame == 0) {
				// Turn off the bob
				bob->active = false;
			} else {
				if (object.animType == 2 || object.animType == 0) {
					// Unpack animation, but do not unpack moving people

					if (!((objAnim[i].mx > 0 || objAnim[i].my > 0) && inRange(objAnim[i].object, 1, 3))) {
						_vm->bankMan()->unpack(
								objAnim[i].unpackFrame,
								objAnim[i].originalFrame,
								objAnim[i].bank);
					}

					if (0 == objAnim[i].object) {
						// Scale Joe
						bob->scale = scale(object);
					}
				}

				if (objAnim[i].cx || objAnim[i].cy) {
					bob->x = objAnim[i].cx;
					bob->y = objAnim[i].cy;
				}

				// Only flip if we are not moving or it is not a person object
				if (!(objAnim[i].object > 0 && objAnim[i].object < 4) ||
						!(objAnim[i].mx || objAnim[i].my))
					bob->xflip = objAnim[i].flip;

				// Add frame alteration
				if (!(objAnim[i].object > 0 && objAnim[i].object < 4)) {
					bob->frameNum = objAnim[i].originalFrame;
				}

				int j;
				for (j = 0; j < objAnim[i].speed; j++)
					_vm->update();
			}

			if (_vm->input()->cutawayQuit())
				return NULL;

			if (objAnim[i].song > 0)
				_vm->sound()->playSong(objAnim[i].song);

		} // for ()
	}

	bool moving = true;

	while (moving) {
		moving = false;
		_vm->update();

		for (i = 0; i < frameCount; i++) {
			BobSlot *bob = _vm->graphics()->bob(objAnim[i].object);
			if (bob->moving) {
				moving = true;
				break;
			}
		}

		if (_vm->input()->cutawayQuit())
			return NULL;
	}

	return ptr;
}

static void findCdCut(const char *basename, int index, char *result) {
	strcpy(result, basename);
	for (int i = strlen(basename); i < 5; i++)
		result[i] = '_';
	snprintf(result + 5, 3, "%02i", index);
}

void Cutaway::handlePersonRecord(
		int index,
		CutawayObject &object,
		const char *sentence) {
	// Lines 1455-1516 in cutaway.c

	Person p;

	if (object.objectNumber == OBJECT_JOE) {
		if (object.moveToX || object.moveToY) {
			_vm->walk()->moveJoe(0, object.moveToX, object.moveToY, true);
		}
	} else {
		_vm->logic()->initPerson(
				object.objectNumber - _vm->logic()->currentRoomData(),
				"", true, &p);

		if (object.bobStartX || object.bobStartY) {
			BobSlot *bob = _vm->graphics()->bob(p.actor->bobNum);
			bob->scale = scale(object);
			bob->x = object.bobStartX;
			bob->y = object.bobStartY;
		}

		if (object.moveToX || object.moveToY)
			_vm->walk()->movePerson(
					&p,
					object.moveToX, object.moveToY,
					_currentImage + 1,
					_vm->logic()->objectData(object.objectNumber)->image
					);
	}

	if (_vm->input()->cutawayQuit())
		return;

	if (0 != strcmp(sentence, "*")) {
		if (sentence[0] == '#') {
			debug(4, "Starting credits '%s'", sentence + 1);
			_vm->logic()->startCredits(sentence + 1);
		} else {
			if (object.objectNumber > 0) {
				bool foundPerson = false;

				for (int i = 1; i <= _personFaceCount; i++) {
					if (_personFace[i].index == object.objectNumber) {
						foundPerson = true;
						break;
					}
				}

				if (!foundPerson) {
					_personFaceCount++;
					assert(_personFaceCount < MAX_PERSON_FACE_COUNT);
					_personFace[_personFaceCount].index = object.objectNumber;
					_personFace[_personFaceCount].image = _vm->logic()->objectData(object.objectNumber)->image;
				}
			}

			char voiceFilePrefix[MAX_STRING_SIZE];
			findCdCut(_basename, index, voiceFilePrefix);
			_vm->logic()->makePersonSpeak(sentence, (object.objectNumber == OBJECT_JOE) ? NULL : &p, voiceFilePrefix);
		}

	}

	if (_vm->input()->cutawayQuit())
		return;
}

void Cutaway::run(char *nextFilename) {
	int i;
	nextFilename[0] = '\0';

	_currentImage = _vm->graphics()->numFrames();

	BobSlot *joeBob = _vm->graphics()->bob(0);
	int initialJoeX = joeBob->x;
	int initialJoeY = joeBob->y;
	debug(6, "[Cutaway::run] Joe started at (%i, %i)", initialJoeX, initialJoeY);

	_vm->input()->cutawayRunning(true);

	_initialRoom = _temporaryRoom = _vm->logic()->currentRoom();

	_vm->display()->screenMode(_comPanel, true);

	if (_comPanel == 0 || _comPanel == 2) {
		_vm->logic()->sceneStart();
	}

	memset(_personFace, 0, sizeof(_personFace));
	_personFaceCount = 0;

	const byte *ptr = _objectData;

	for (i = 0; i < _cutawayObjectCount; i++) {
		CutawayObject object;
		ptr = getCutawayObject(ptr, object);
		//dumpCutawayObject(i, object);

		if (!object.moveToX &&
				!object.moveToY &&
				object.specialMove > 0 &&
				object.objectNumber >= 0) {
			_vm->logic()->executeSpecialMove(object.specialMove);
			object.specialMove = 0;
		}

		if (CURRENT_ROOM == object.room) {
			// Get current room
			object.room = _vm->logic()->currentRoom();
		} else {
			// Change current room
			_vm->logic()->currentRoom(object.room);
		}

		ptr = turnOnPeople(ptr, object);

		limitBob(object);

		char sentence[MAX_STRING_SIZE];
		Talk::getString(_fileData, _nextSentenceOff, sentence, MAX_STRING_LENGTH);

		if (OBJECT_ROOMFADE == object.objectNumber) {
			_roomFade = true;
			object.objectNumber = OBJECT_JOE;
		} else {
			_roomFade = false;
		}

		if (object.room != _temporaryRoom)
			changeRooms(object);

		ObjectType objectType = getObjectType(object);

		if (object.song)
			_vm->sound()->playSong(object.song);

		switch (objectType) {
		case OBJECT_TYPE_ANIMATION:
			ptr = handleAnimation(ptr, object);
			break;
		case OBJECT_TYPE_PERSON:
			handlePersonRecord(i + 1, object, sentence);
			break;
		case OBJECT_TYPE_NO_ANIMATION:
			// Do nothing?
			break;
		case OBJECT_TYPE_TEXT_SPEAK:
		case OBJECT_TYPE_TEXT_DISPLAY_AND_SPEAK:
		case OBJECT_TYPE_TEXT_DISPLAY:
			handleText(i + 1, objectType, object, sentence);
			break;
		default:
			warning("Unhandled object type: %i", objectType);
			break;
		}

		if (_vm->input()->cutawayQuit())
			break;

		if (_roomFade) {
			_vm->update();
			BobSlot *j = _vm->graphics()->bob(0);
			_vm->display()->palFadeIn(_vm->logic()->currentRoom(), j->active, j->x, j->y);
			_roomFade = false;
		}

	} // for ()

	_vm->display()->clearTexts(0, 198);
	// XXX lines 1887-1895 in cutaway.c

	stop();

	updateGameState();

	_vm->bankMan()->close(CUTAWAY_BANK);

	talk(nextFilename);

	if (_comPanel == 0 || (_comPanel == 2 && !_anotherCutaway)) {
		_vm->logic()->sceneStop();
		_comPanel = 0;
	}

	if (nextFilename[0] == '\0' && !_anotherCutaway && _vm->logic()->currentRoom() != ROOM_ENDING_CREDITS) {
		_vm->display()->fullscreen(false);

		// Lines 2138-2182 in cutaway.c
		if (_finalRoom) {
			_vm->logic()->newRoom(0);
			_vm->logic()->entryObj(0);
		} else {
			/// No need to stay in current room, so return to previous room
			//  if one exists. Reset Joe's X,Y coords to those when first entered

			restorePersonData();

			debug(6, "_vm->logic()->entryObj() = %i", _vm->logic()->entryObj());
			if (_vm->logic()->entryObj() > 0) {
				_initialRoom = _vm->logic()->objectData(_vm->logic()->entryObj())->room;
			} else {
				// We're not returning to new room, so return to old Joe X,Y coords
				debug(6, "[Cutaway::run] Moving joe to (%i, %i)", initialJoeX, initialJoeY);
				_vm->logic()->joePos(initialJoeX, initialJoeY);
			}

			if (_vm->logic()->currentRoom() != _initialRoom) {
				_vm->logic()->currentRoom(_initialRoom);
				_vm->logic()->changeRoom();
				if (_vm->logic()->currentRoom() == _vm->logic()->newRoom()) {
					_vm->logic()->newRoom(0);
				}
			}
			_vm->logic()->joePos(0, 0);
		}

		_vm->logic()->joeCutFacing(0);
		_comPanel = 0;

		int k = 0;
		for (i = _vm->logic()->roomData(_vm->logic()->currentRoom());
				i <= _vm->logic()->roomData(_vm->logic()->currentRoom() + 1); i++) {

			ObjectData *object = _vm->logic()->objectData(i);
			if (object->image == -3 || object->image == -4) {
				k++;
				if (object->name > 0) {
					_vm->graphics()->resetPersonAnim(k);
				}
			}
		}

		_vm->logic()->removeHotelItemsFromInventory();
	}

	joeBob->animating = 0;
	joeBob->moving    = 0;

	// if the cutaway has been cancelled, we must stop the speech and the sfx as well
	if (_vm->input()->cutawayQuit()) {
		if (_vm->sound()->isSpeechActive())
			_vm->sound()->stopSpeech();
		_vm->sound()->stopSfx();
	}

	_vm->input()->cutawayRunning(false);
	_vm->input()->cutawayQuitReset();
	_vm->input()->quickSaveReset();
	_vm->input()->quickLoadReset();

	if (_songBeforeComic > 0)
		_vm->sound()->playSong(_songBeforeComic);
	else if (_lastSong > 0)
		_vm->sound()->playSong(_lastSong);
}

void Cutaway::stop() {
	// Lines 1901-2032 in cutaway.c
	byte *ptr = _gameStatePtr;

	// Skipping GAMESTATE data
	int gameStateCount = (int16)READ_BE_INT16(ptr); ptr += 2;
	if (gameStateCount > 0)
		ptr += (gameStateCount * 12);

	// Get the final room and Joe's final position

	int16 joeRoom = READ_BE_UINT16(ptr); ptr += 2;
	int16 joeX    = READ_BE_UINT16(ptr); ptr += 2;
	int16 joeY    = READ_BE_UINT16(ptr); ptr += 2;

	debug(6, "[Cutaway::stop] Final position is room %i and coordinates (%i, %i)",
			joeRoom, joeX, joeY);

	if ((!_vm->input()->cutawayQuit() || (!_anotherCutaway && joeRoom == _finalRoom)) &&
			joeRoom != _temporaryRoom &&
			joeRoom != 0) {

		debug(6, "[Cutaway::stop] Changing rooms and moving Joe");

		_vm->logic()->joePos(joeX, joeY);
		_vm->logic()->currentRoom(joeRoom);
		_vm->logic()->oldRoom(_initialRoom);
		_vm->logic()->displayRoom(_vm->logic()->currentRoom(), RDM_FADE_JOE_XY, 0, _comPanel, true);
	}

	if (_vm->input()->cutawayQuit()) {
		// Lines 1927-2032 in cutaway.c
		int i;

		// Stop the credits from running
		_vm->logic()->stopCredits();

		_vm->graphics()->stopBobs();

		for (i = 1; i <= _personFaceCount; i++) {
			int index =  _personFace[i].index;
			if (index > 0) {
				_vm->logic()->objectData(_personFace[i].index)->image = _personFace[i].image;

				_vm->graphics()->bob(_vm->logic()->findBob(index))->xflip =
					(_personFace[i].image != -4);
			}
		}

		int quitObjectCount = (int16)READ_BE_INT16(ptr); ptr += 2;

		for (i = 0; i < quitObjectCount; i++) {
			int16 objectIndex  = (int16)READ_BE_INT16(ptr); ptr += 2;
			int16 fromIndex    = (int16)READ_BE_INT16(ptr); ptr += 2;
			int16 x       = (int16)READ_BE_INT16(ptr); ptr += 2;
			int16 y       = (int16)READ_BE_INT16(ptr); ptr += 2;
			int16 room    = (int16)READ_BE_INT16(ptr); ptr += 2;
			int16 frame   = (int16)READ_BE_INT16(ptr); ptr += 2;
			int16 bank    = (int16)READ_BE_INT16(ptr); ptr += 2;

			int bobIndex = _vm->logic()->findBob(objectIndex);
			ObjectData *object = _vm->logic()->objectData(objectIndex);

			if (fromIndex > 0) {
				if (fromIndex == objectIndex) {
					// Enable object
					object->name = ABS(object->name);
				} else {
					_vm->logic()->objectCopy(fromIndex, objectIndex);

					ObjectData *from = _vm->logic()->objectData(fromIndex);
					if (object->image && !from->image && bobIndex && _vm->logic()->currentRoom() == object->room)
						_vm->graphics()->clearBob(bobIndex);
				}

				if (_vm->logic()->currentRoom() == room)
					_vm->graphics()->refreshObject(objectIndex);
			}

			if (_vm->logic()->currentRoom() == object->room) {
				BobSlot *pbs = _vm->graphics()->bob(bobIndex);

				if (x || y) {
					pbs->x = x;
					pbs->y = y;
					if (inRange(object->image, -4, -3))
						pbs->scale = _vm->grid()->findScale(x, y);
				}

				if (frame) {
					if (0 == bank)
						bank = 15;
					else if (bank != 13) {
						_vm->bankMan()->load(_bankNames[bank-1], CUTAWAY_BANK);
						bank = 8;
					}

					int objectFrame = _vm->logic()->findFrame(objectIndex);

					if (objectFrame == 1000) {
						_vm->graphics()->clearBob(bobIndex);
					} else if (objectFrame) {
						_vm->bankMan()->unpack(ABS(frame), objectFrame, bank);
						pbs->frameNum = objectFrame;
						if (frame < 0)
							pbs->xflip = true;

					}
				}
			}
		} // for ()

		int16 specialMove = (int16)READ_BE_INT16(ptr); ptr += 2;
		if (specialMove > 0)
			_vm->logic()->executeSpecialMove(specialMove);

		_lastSong = (int16)READ_BE_INT16(ptr); ptr += 2;
	}

	if (joeRoom == _temporaryRoom &&
			joeRoom != 37 && joeRoom != 105 && joeRoom != 106 &&
			(joeX || joeY)) {
		BobSlot *joeBob = _vm->graphics()->bob(0);

		debug(6, "[Cutaway::stop] Moving Joe");

		joeBob->x = joeX;
		joeBob->y = joeY;
		_vm->logic()->joeScale(_vm->grid()->findScale(joeX, joeY));
		_vm->logic()->joeFace();
	}
}

void Cutaway::updateGameState() {
	// Lines 2047-2115 in cutaway.c
	byte *ptr = _gameStatePtr;

	int gameStateCount = (int16)READ_BE_INT16(ptr); ptr += 2;

	for (int i = 0; i < gameStateCount; i++) {
		int16 stateIndex    = (int16)READ_BE_INT16(ptr); ptr += 2;
		int16 stateValue    = (int16)READ_BE_INT16(ptr); ptr += 2;
		int16 objectIndex   = (int16)READ_BE_INT16(ptr); ptr += 2;
		int16 areaIndex     = (int16)READ_BE_INT16(ptr); ptr += 2;
		int16 areaSubIndex  = (int16)READ_BE_INT16(ptr); ptr += 2;
		int16 fromObject    = (int16)READ_BE_INT16(ptr); ptr += 2;

		bool update = false;

		if (stateIndex > 0) {
			if (_vm->logic()->gameState(stateIndex) == stateValue)
				update = true;
		} else {
			_vm->logic()->gameState(ABS(stateIndex), stateValue);
			update = true;
		}

		if (update) {

			if (objectIndex > 0) {                    // Show the object
				ObjectData *objectData  = _vm->logic()->objectData(objectIndex);
				objectData->name        = ABS(objectData->name);
				if (fromObject > 0)
					_vm->logic()->objectCopy(fromObject, objectIndex);
				_vm->graphics()->refreshObject(objectIndex);
			} else if (objectIndex < 0) {               // Hide the object
				objectIndex             = -objectIndex;
				ObjectData *objectData  = _vm->logic()->objectData(objectIndex);
				objectData->name        = -ABS(objectData->name);
				_vm->graphics()->refreshObject(objectIndex);
			}

			if (areaIndex > 0) {

				// Turn area on or off

				if (areaSubIndex > 0) {
					Area *area = _vm->grid()->area(areaIndex, areaSubIndex);
					area->mapNeighbors = ABS(area->mapNeighbors);
				} else {
					Area *area = _vm->grid()->area(areaIndex, ABS(areaSubIndex));
					area->mapNeighbors = -ABS(area->mapNeighbors);
				}
			}

		}
	} // for ()
}

void Cutaway::talk(char *nextFilename) {
	const char *p = strrchr(_talkFile, '.');
	if (p && 0 == scumm_stricmp(p, ".DOG")) {
		nextFilename[0] = '\0';
		assert(_talkTo > 0);
		int personInRoom = _talkTo - _vm->logic()->roomData(_vm->logic()->currentRoom());
		_vm->logic()->startDialogue(_talkFile, personInRoom, nextFilename);
	}
}

int Cutaway::makeComplexAnimation(int16 currentImage, Cutaway::CutawayAnim *objAnim, int frameCount) {
	int frameIndex[256];
	int i;
	assert(frameCount < 30);
	AnimFrame cutAnim[30];

	memset(frameIndex, 0, sizeof(frameIndex));
	debug(6, "[Cutaway::makeComplexAnimation] currentImage = %i", currentImage);

	for (i = 0; i < frameCount; i++) {
		cutAnim[i].frame = objAnim[i].unpackFrame;
		cutAnim[i].speed = objAnim[i].speed;
		frameIndex[objAnim[i].unpackFrame] = 1;
	}

	cutAnim[frameCount].frame = 0;
	cutAnim[frameCount].speed = 0;

	int nextFrameIndex = 1;

	for (i = 1; i < 256; i++)
		if (frameIndex[i])
			frameIndex[i] = nextFrameIndex++;

	for (i = 0; i < frameCount; i++) {
		cutAnim[i].frame = currentImage + frameIndex[objAnim[i].unpackFrame];
	}

	for (i = 1; i < 256; i++) {
		if (frameIndex[i]) {
			currentImage++;
			_vm->bankMan()->unpack(i, currentImage, objAnim[0].bank);
		}
	}

	_vm->graphics()->setBobCutawayAnim(objAnim[0].object, objAnim[0].flip, cutAnim, frameCount + 1);
	return currentImage;
}

void Cutaway::handleText(
		int index,
		ObjectType type,
		CutawayObject &object,
		const char *sentence) {
	// lines 1776-1863 in cutaway.c

	int spaces = countSpaces(type, sentence);

	int x;
	int flags;

	if (OBJECT_TYPE_TEXT_DISPLAY == type) {
		x = _vm->display()->textCenterX(sentence);
		flags = 2;
	} else {
		x = object.bobStartX;
		flags = 1;
	}

	BobSlot *bob =
		_vm->graphics()->bob(_vm->logic()->findBob(ABS(object.objectNumber)));

	_vm->graphics()->setBobText(bob, sentence, x, object.bobStartY, object.specialMove, flags);

	if (OBJECT_TYPE_TEXT_SPEAK == type || OBJECT_TYPE_TEXT_DISPLAY_AND_SPEAK == type) {
		if (_vm->sound()->speechOn()) {
			char voiceFileName[MAX_STRING_SIZE];
			findCdCut(_basename, index, voiceFileName);
			strcat(voiceFileName, "1");
			_vm->sound()->playSpeech(voiceFileName);
		}

		if (OBJECT_TYPE_TEXT_SPEAK == type && _vm->sound()->speechOn() && !_vm->subtitles())
			_vm->display()->clearTexts(0, 150);
	}

	while (1) {
		_vm->update();

		if (_vm->input()->cutawayQuit())
			return;

		if (_vm->input()->keyVerb() == VERB_SKIP_TEXT) {
			_vm->input()->clearKeyVerb();
			break;
		}

		if ((OBJECT_TYPE_TEXT_SPEAK == type || OBJECT_TYPE_TEXT_DISPLAY_AND_SPEAK == type) && _vm->sound()->speechOn() && _vm->sound()->speechSfxExists()) {
			if (!_vm->sound()->isSpeechActive()) {
				break;
			}
		} else {
			--spaces;
			if (spaces <= 0) {
				break;
			}
		}
	}

	_vm->display()->clearTexts(0, 198);
	_vm->update();
}

int Cutaway::countSpaces(ObjectType type, const char *segment) {
	int tmp = 0;

	while (*segment++)
		tmp++;

	if (tmp < 50)
		tmp = 50;

	if (OBJECT_TYPE_TEXT_DISPLAY == type)
		tmp *= 3;

	return (tmp * 2) / (_vm->talkSpeed() / 3);

}

int Cutaway::scale(CutawayObject &object) {
	int scaling = 100;

	if (object.scale > 0)
		scaling = object.scale;
	else if (!object.objectNumber) {
		// Only scale Joe
		int x, y;

		if (object.bobStartX > 0 || object.bobStartY > 0) {
			x = object.bobStartX;
			y = object.bobStartY;
		} else {
			BobSlot *bob = _vm->graphics()->bob(0);
			x = bob->x;
			y = bob->y;
		}

		int zone = _vm->grid()->findAreaForPos(GS_ROOM, x, y);
		if (zone > 0) {
			Area *area = _vm->grid()->area(_vm->logic()->currentRoom(), zone);
			scaling = area->calcScale(y);
		}
	}

	return scaling;
}

} // End of namespace Queen
