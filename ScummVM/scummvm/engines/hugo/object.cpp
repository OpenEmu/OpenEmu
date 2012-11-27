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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/debug.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/object.h"
#include "hugo/display.h"
#include "hugo/file.h"
#include "hugo/route.h"
#include "hugo/util.h"
#include "hugo/parser.h"
#include "hugo/schedule.h"
#include "hugo/text.h"
#include "hugo/inventory.h"
#include "hugo/mouse.h"

namespace Hugo {

ObjectHandler::ObjectHandler(HugoEngine *vm) : _vm(vm), _objects(0), _uses(0) {
	_numObj = 0;
	_objCount = 0;
	_usesSize = 0;
	memset(_objBound, '\0', sizeof(Overlay));
	memset(_boundary, '\0', sizeof(Overlay));
	memset(_overlay,  '\0', sizeof(Overlay));
	memset(_ovlBase,  '\0', sizeof(Overlay));
}

ObjectHandler::~ObjectHandler() {
}

byte ObjectHandler::getBoundaryOverlay(uint16 index) const {
	return _boundary[index];
}

byte ObjectHandler::getObjectBoundary(uint16 index) const {
	return _objBound[index];
}

byte ObjectHandler::getBaseBoundary(uint16 index) const {
	return _ovlBase[index];
}

byte ObjectHandler::getFirstOverlay(uint16 index) const {
	return _overlay[index];
}

bool ObjectHandler::isCarried(int objIndex) const {
	return _objects[objIndex]._carriedFl;
}

void ObjectHandler::setCarry(int objIndex, bool val) {
	_objects[objIndex]._carriedFl = val;
}

void ObjectHandler::setVelocity(int objIndex, int8 vx, int8 vy) {
	_objects[objIndex]._vx = vx;
	_objects[objIndex]._vy = vy;
}

void ObjectHandler::setPath(int objIndex, Path pathType, int16 vxPath, int16 vyPath) {
	_objects[objIndex]._pathType = pathType;
	_objects[objIndex]._vxPath = vxPath;
	_objects[objIndex]._vyPath = vyPath;
}

/**
 * Save sequence number and image number in given object
 */
void ObjectHandler::saveSeq(Object *obj) {
	debugC(1, kDebugObject, "saveSeq");

	bool found = false;
	for (int i = 0; !found && (i < obj->_seqNumb); i++) {
		Seq *q = obj->_seqList[i]._seqPtr;
		for (int j = 0; !found && (j < obj->_seqList[i]._imageNbr); j++) {
			if (obj->_currImagePtr == q) {
				found = true;
				obj->_curSeqNum = i;
				obj->_curImageNum = j;
			} else {
				q = q->_nextSeqPtr;
			}
		}
	}
}

/**
 * Set up cur_seqPtr from stored sequence and image number in object
 */
void ObjectHandler::restoreSeq(Object *obj) {
	debugC(1, kDebugObject, "restoreSeq");

	Seq *q = obj->_seqList[obj->_curSeqNum]._seqPtr;
	for (int j = 0; j < obj->_curImageNum; j++)
		q = q->_nextSeqPtr;
	obj->_currImagePtr = q;
}

/**
 * If status.objid = -1, pick up objid, else use status.objid on objid,
 * if objid can't be picked up, use it directly
 */
void ObjectHandler::useObject(int16 objId) {
	debugC(1, kDebugObject, "useObject(%d)", objId);

	const char *verb;                               // Background verb to use directly
	int16 inventObjId = _vm->_inventory->getInventoryObjId();
	Object *obj = &_objects[objId];               // Ptr to object
	if (inventObjId == -1) {
		// Get or use objid directly
		if ((obj->_genericCmd & TAKE) || obj->_objValue)  // Get collectible item
			sprintf(_vm->_line, "%s %s", _vm->_text->getVerb(_vm->_take, 0), _vm->_text->getNoun(obj->_nounIndex, 0));
		else if (obj->_cmdIndex != 0)                // Use non-collectible item if able
			sprintf(_vm->_line, "%s %s", _vm->_text->getVerb(_vm->_parser->getCmdDefaultVerbIdx(obj->_cmdIndex), 0), _vm->_text->getNoun(obj->_nounIndex, 0));
		else if ((verb = _vm->_parser->useBG(_vm->_text->getNoun(obj->_nounIndex, 0))) != 0)
			sprintf(_vm->_line, "%s %s", verb, _vm->_text->getNoun(obj->_nounIndex, 0));
		else
			return;                                 // Can't use object directly
	} else {
		// Use status.objid on objid
		// Default to first cmd verb
		sprintf(_vm->_line, "%s %s %s", _vm->_text->getVerb(_vm->_parser->getCmdDefaultVerbIdx(_objects[inventObjId]._cmdIndex), 0),
			                       _vm->_text->getNoun(_objects[inventObjId]._nounIndex, 0),
			                       _vm->_text->getNoun(obj->_nounIndex, 0));

		// Check valid use of objects and override verb if necessary
		for (Uses *use = _uses; use->_objId != _numObj; use++) {
			if (inventObjId == use->_objId) {
				// Look for secondary object, if found use matching verb
				bool foundFl = false;

				for (Target *target = use->_targets; target->_nounIndex != 0; target++)
					if (target->_nounIndex == obj->_nounIndex) {
						foundFl = true;
						sprintf(_vm->_line, "%s %s %s", _vm->_text->getVerb(target->_verbIndex, 0),
							                       _vm->_text->getNoun(_objects[inventObjId]._nounIndex, 0),
							                       _vm->_text->getNoun(obj->_nounIndex, 0));
					}

				// No valid use of objects found, print failure string
				if (!foundFl) {
					// Deselect dragged icon if inventory not active
					if (_vm->_inventory->getInventoryState() != kInventoryActive)
						_vm->_screen->resetInventoryObjId();
					Utils::notifyBox(_vm->_text->getTextData(use->_dataIndex));
					return;
				}
			}
		}
	}

	if (_vm->_inventory->getInventoryState() == kInventoryActive) // If inventory active, remove it
		_vm->_inventory->setInventoryState(kInventoryUp);

	_vm->_screen->resetInventoryObjId();

	_vm->_parser->lineHandler();                    // and process command
}

/**
 * Return object index of the topmost object under the cursor, or -1 if none
 * Objects are filtered if not "useful"
 */
int16 ObjectHandler::findObject(uint16 x, uint16 y) {
	debugC(3, kDebugObject, "findObject(%d, %d)", x, y);

	int16     objIndex = -1;                        // Index of found object
	uint16    y2Max = 0;                            // Greatest y2
	Object *obj = _objects;
	// Check objects on screen
	for (int i = 0; i < _numObj; i++, obj++) {
		// Object must be in current screen and "useful"
		if (obj->_screenIndex == *_vm->_screenPtr && (obj->_genericCmd || obj->_objValue || obj->_cmdIndex)) {
			Seq *curImage = obj->_currImagePtr;
			// Object must have a visible image...
			if (curImage != 0 && obj->_cycling != kCycleInvisible) {
				// If cursor inside object
				if (x >= (uint16)obj->_x && x <= obj->_x + curImage->_x2 && y >= (uint16)obj->_y && y <= obj->_y + curImage->_y2) {
					// If object is closest so far
					if (obj->_y + curImage->_y2 > y2Max) {
						y2Max = obj->_y + curImage->_y2;
						objIndex = i;               // Found an object!
					}
				}
			} else {
				// ...or a dummy object that has a hotspot rectangle
				if (curImage == 0 && obj->_vxPath != 0 && !obj->_carriedFl) {
					// If cursor inside special rectangle
					if ((int16)x >= obj->_oldx && (int16)x < obj->_oldx + obj->_vxPath && (int16)y >= obj->_oldy && (int16)y < obj->_oldy + obj->_vyPath) {
						// If object is closest so far
						if (obj->_oldy + obj->_vyPath - 1 > (int16)y2Max) {
							y2Max = obj->_oldy + obj->_vyPath - 1;
							objIndex = i;           // Found an object!
						}
					}
				}
			}
		}
	}
	return objIndex;
}

/**
 * Issue "Look at <object>" command
 * Note special case of swapped hero image
 */
void ObjectHandler::lookObject(Object *obj) {
	debugC(1, kDebugObject, "lookObject");

	if (obj == _vm->_hero)
		// Hero swapped - look at other
		obj = &_objects[_vm->_heroImage];

	_vm->_parser->command("%s %s", _vm->_text->getVerb(_vm->_look, 0), _vm->_text->getNoun(obj->_nounIndex, 0));
}

/**
 * Free all object images, uses and ObjArr (before exiting)
 */
void ObjectHandler::freeObjects() {
	debugC(1, kDebugObject, "freeObjects");

	if (_vm->_hero != 0 && _vm->_hero->_seqList[0]._seqPtr != 0) {
		// Free all sequence lists and image data
		for (int16 i = 0; i < _numObj; i++) {
			Object *obj = &_objects[i];
			for (int16 j = 0; j < obj->_seqNumb; j++) {
				Seq *seq = obj->_seqList[j]._seqPtr;
				Seq *next;
				if (seq == 0) // Failure during database load
					break;
				if (seq->_imagePtr != 0) {
					free(seq->_imagePtr);
					seq->_imagePtr = 0;
				}
				seq = seq->_nextSeqPtr;
				while (seq != obj->_seqList[j]._seqPtr) {
					if (seq->_imagePtr != 0) {
						free(seq->_imagePtr);
						seq->_imagePtr = 0;
					}
					next = seq->_nextSeqPtr;
					free(seq);
					seq = next;
				}
				free(seq);
			}
		}
	}

	if (_uses) {
		for (int16 i = 0; i < _usesSize; i++)
			free(_uses[i]._targets);
		free(_uses);
	}

	for (int16 i = 0; i < _objCount; i++) {
		free(_objects[i]._stateDataIndex);
		_objects[i]._stateDataIndex = 0;
	}

	free(_objects);
	_objects = 0;
}

/**
 * Compare function for the quicksort.  The sort is to order the objects in
 * increasing vertical position, using y+y2 as the baseline
 * Returns -1 if ay2 < by2 else 1 if ay2 > by2 else 0
 */
int ObjectHandler::y2comp(const void *a, const void *b) {
	debugC(6, kDebugObject, "y2comp");

	const Object *p1 = &HugoEngine::get()._object->_objects[*(const byte *)a];
	const Object *p2 = &HugoEngine::get()._object->_objects[*(const byte *)b];

	if (p1 == p2)
		// Why does qsort try the same indexes?
		return 0;

	if (p1->_priority == kPriorityBackground)
		return -1;

	if (p2->_priority == kPriorityBackground)
		return 1;

	if (p1->_priority == kPriorityForeground)
		return 1;

	if (p2->_priority == kPriorityForeground)
		return -1;

	int ay2 = p1->_y + p1->_currImagePtr->_y2;
	int by2 = p2->_y + p2->_currImagePtr->_y2;

	return ay2 - by2;
}

/**
 * Return TRUE if object being carried by hero
 */
bool ObjectHandler::isCarrying(uint16 wordIndex) {
	debugC(1, kDebugObject, "isCarrying(%d)", wordIndex);

	for (int i = 0; i < _numObj; i++) {
		if ((wordIndex == _objects[i]._nounIndex) && _objects[i]._carriedFl)
			return true;
	}
	return false;
}

/**
 * Describe any takeable objects visible in this screen
 */
void ObjectHandler::showTakeables() {
	debugC(1, kDebugObject, "showTakeables");

	for (int j = 0; j < _numObj; j++) {
		Object *obj = &_objects[j];
		if ((obj->_cycling != kCycleInvisible) &&
		    (obj->_screenIndex == *_vm->_screenPtr) &&
		    (((TAKE & obj->_genericCmd) == TAKE) || obj->_objValue)) {
			Utils::notifyBox(Common::String::format("You can also see:\n%s.", _vm->_text->getNoun(obj->_nounIndex, LOOK_NAME)));
		}
	}
}

/**
 * Find a clear space around supplied object that hero can walk to
 */
bool ObjectHandler::findObjectSpace(Object *obj, int16 *destx, int16 *desty) {
	debugC(1, kDebugObject, "findObjectSpace(obj, %d, %d)", *destx, *desty);

	Seq *curImage = obj->_currImagePtr;
	int16 y = obj->_y + curImage->_y2 - 1;

	bool foundFl = true;
	// Try left rear corner
	for (int16 x = *destx = obj->_x + curImage->_x1; x < *destx + kHeroMaxWidth; x++) {
		if (checkBoundary(x, y))
			foundFl = false;
	}

	if (!foundFl) {                                 // Try right rear corner
		foundFl = true;
		for (int16 x = *destx = obj->_x + curImage->_x2 - kHeroMaxWidth + 1; x <= obj->_x + (int16)curImage->_x2; x++) {
			if (checkBoundary(x, y))
				foundFl = false;
		}
	}

	if (!foundFl) {                                 // Try left front corner
		foundFl = true;
		y += 2;
		for (int16 x = *destx = obj->_x + curImage->_x1; x < *destx + kHeroMaxWidth; x++) {
			if (checkBoundary(x, y))
				foundFl = false;
		}
	}

	if (!foundFl) {                                 // Try right rear corner
		foundFl = true;
		for (int16 x = *destx = obj->_x + curImage->_x2 - kHeroMaxWidth + 1; x <= obj->_x + (int16)curImage->_x2; x++) {
			if (checkBoundary(x, y))
				foundFl = false;
		}
	}

	*desty = y;
	return foundFl;
}

void ObjectHandler::readUse(Common::ReadStream &in, Uses &curUse) {
	curUse._objId = in.readSint16BE();
	curUse._dataIndex = in.readUint16BE();
	uint16 numSubElem = in.readUint16BE();
	curUse._targets = (Target *)malloc(sizeof(Target) * numSubElem);
	for (int j = 0; j < numSubElem; j++) {
		curUse._targets[j]._nounIndex = in.readUint16BE();
		curUse._targets[j]._verbIndex = in.readUint16BE();
	}
}
/**
 * Load _uses from Hugo.dat
 */
void ObjectHandler::loadObjectUses(Common::ReadStream &in) {
	Uses tmpUse;
	tmpUse._targets = 0;

	//Read _uses
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant) {
			_usesSize = numElem;
			_uses = (Uses *)malloc(sizeof(Uses) * numElem);
		}

		for (int i = 0; i < numElem; i++) {
			if (varnt == _vm->_gameVariant)
				readUse(in, _uses[i]);
			else {
				readUse(in, tmpUse);
				free(tmpUse._targets);
				tmpUse._targets = 0;
			}
		}
	}
}

void ObjectHandler::readObject(Common::ReadStream &in, Object &curObject) {
	curObject._nounIndex = in.readUint16BE();
	curObject._dataIndex = in.readUint16BE();
	uint16 numSubElem = in.readUint16BE();

	if (numSubElem == 0)
		curObject._stateDataIndex = 0;
	else
		curObject._stateDataIndex = (uint16 *)malloc(sizeof(uint16) * numSubElem);
	for (int j = 0; j < numSubElem; j++)
		curObject._stateDataIndex[j] = in.readUint16BE();

	curObject._pathType = (Path) in.readSint16BE();
	curObject._vxPath = in.readSint16BE();
	curObject._vyPath = in.readSint16BE();
	curObject._actIndex = in.readUint16BE();
	curObject._seqNumb = in.readByte();
	curObject._currImagePtr = 0;

	if (curObject._seqNumb == 0) {
		curObject._seqList[0]._imageNbr = 0;
		curObject._seqList[0]._seqPtr = 0;
	}

	for (int j = 0; j < curObject._seqNumb; j++) {
		curObject._seqList[j]._imageNbr = in.readUint16BE();
		curObject._seqList[j]._seqPtr = 0;
	}

	curObject._cycling = (Cycle)in.readByte();
	curObject._cycleNumb = in.readByte();
	curObject._frameInterval = in.readByte();
	curObject._frameTimer = in.readByte();
	curObject._radius = in.readByte();
	curObject._screenIndex = in.readByte();
	curObject._x = in.readSint16BE();
	curObject._y = in.readSint16BE();
	curObject._oldx = in.readSint16BE();
	curObject._oldy = in.readSint16BE();
	curObject._vx = in.readByte();
	curObject._vy = in.readByte();
	curObject._objValue = in.readByte();
	curObject._genericCmd = in.readSint16BE();
	curObject._cmdIndex = in.readUint16BE();
	curObject._carriedFl = (in.readByte() != 0);
	curObject._state = in.readByte();
	curObject._verbOnlyFl = (in.readByte() != 0);
	curObject._priority = in.readByte();
	curObject._viewx = in.readSint16BE();
	curObject._viewy = in.readSint16BE();
	curObject._direction = in.readSint16BE();
	curObject._curSeqNum = in.readByte();
	curObject._curImageNum = in.readByte();
	curObject._oldvx = in.readByte();
	curObject._oldvy = in.readByte();
}
/**
 * Load ObjectArr from Hugo.dat
 */
void ObjectHandler::loadObjectArr(Common::ReadStream &in) {
	debugC(6, kDebugObject, "loadObject(&in)");
	Object tmpObject;
	tmpObject._stateDataIndex = 0;

	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		uint16 numElem = in.readUint16BE();

		if (varnt == _vm->_gameVariant) {
			_objCount = numElem;
			_objects = (Object *)malloc(sizeof(Object) * numElem);
		}

		for (int i = 0; i < numElem; i++) {
			if (varnt == _vm->_gameVariant)
				readObject(in, _objects[i]);
			else {
				// Skip over uneeded objects.
				readObject(in, tmpObject);
				free(tmpObject._stateDataIndex);
				tmpObject._stateDataIndex = 0;
			}
		}
	}
}

/**
 * Set the screenindex property of the carried objets to the given screen
 * number
 */
void ObjectHandler::setCarriedScreen(int screenNum) {
	for (int i = kHeroIndex + 1; i < _numObj; i++) {// Any others
		if (isCarried(i))                           // being carried
			_objects[i]._screenIndex = screenNum;
	}
}

/**
 * Load _numObj from Hugo.dat
 */
void ObjectHandler::loadNumObj(Common::ReadStream &in) {
	int numElem;

	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant)
			_numObj = numElem;
	}
}

/**
 * Restore all sequences
 */
void ObjectHandler::restoreAllSeq() {
	// Restore ptrs to currently loaded objects
	for (int i = 0; i < _numObj; i++)
		restoreSeq(&_objects[i]);
}

/**
 * Save objects
 */
void ObjectHandler::saveObjects(Common::WriteStream *out) {
	for (int i = 0; i < _numObj; i++) {
		// Save where curr_seqPtr is pointing to
		saveSeq(&_objects[i]);

		out->writeByte(_objects[i]._pathType);
		out->writeSint16BE(_objects[i]._vxPath);
		out->writeSint16BE(_objects[i]._vyPath);
		out->writeByte(_objects[i]._cycling);
		out->writeByte(_objects[i]._cycleNumb);
		out->writeByte(_objects[i]._frameTimer);
		out->writeByte(_objects[i]._screenIndex);
		out->writeSint16BE(_objects[i]._x);
		out->writeSint16BE(_objects[i]._y);
		out->writeSint16BE(_objects[i]._oldx);
		out->writeSint16BE(_objects[i]._oldy);
		out->writeSByte(_objects[i]._vx);
		out->writeSByte(_objects[i]._vy);
		out->writeByte(_objects[i]._objValue);
		out->writeByte((_objects[i]._carriedFl) ? 1 : 0);
		out->writeByte(_objects[i]._state);
		out->writeByte(_objects[i]._priority);
		out->writeSint16BE(_objects[i]._viewx);
		out->writeSint16BE(_objects[i]._viewy);
		out->writeSint16BE(_objects[i]._direction);
		out->writeByte(_objects[i]._curSeqNum);
		out->writeByte(_objects[i]._curImageNum);
		out->writeSByte(_objects[i]._oldvx);
		out->writeSByte(_objects[i]._oldvy);
	}
}

/**
 * Restore objects
 */
void ObjectHandler::restoreObjects(Common::SeekableReadStream *in) {
	for (int i = 0; i < _numObj; i++) {
		_objects[i]._pathType = (Path) in->readByte();
		_objects[i]._vxPath = in->readSint16BE();
		_objects[i]._vyPath = in->readSint16BE();
		_objects[i]._cycling = (Cycle) in->readByte();
		_objects[i]._cycleNumb = in->readByte();
		_objects[i]._frameTimer = in->readByte();
		_objects[i]._screenIndex = in->readByte();
		_objects[i]._x = in->readSint16BE();
		_objects[i]._y = in->readSint16BE();
		_objects[i]._oldx = in->readSint16BE();
		_objects[i]._oldy = in->readSint16BE();
		_objects[i]._vx = in->readSByte();
		_objects[i]._vy = in->readSByte();
		_objects[i]._objValue = in->readByte();
		_objects[i]._carriedFl = (in->readByte() == 1);
		_objects[i]._state = in->readByte();
		_objects[i]._priority = in->readByte();
		_objects[i]._viewx = in->readSint16BE();
		_objects[i]._viewy = in->readSint16BE();
		_objects[i]._direction = in->readSint16BE();
		_objects[i]._curSeqNum = in->readByte();
		_objects[i]._curImageNum = in->readByte();
		_objects[i]._oldvx = in->readSByte();
		_objects[i]._oldvy = in->readSByte();
	}
}

/**
 * Compute max object score
 */
int ObjectHandler::calcMaxScore() {
	int score = 0;
	for (int i = 0; i < _numObj; i++)
		score += _objects[i]._objValue;
	return score;
}

/**
 * Read Object images
 */
void ObjectHandler::readObjectImages() {
	debugC(1, kDebugObject, "readObjectImages");

	for (int i = 0; i < _numObj; i++)
		_vm->_file->readImage(i, &_objects[i]);
}

bool ObjectHandler::checkBoundary(int16 x, int16 y) {
	// Check if Boundary bit set
	return (_boundary[y * kCompLineSize + x / 8] & (0x80 >> x % 8)) != 0;
}

/**
 * Return maximum allowed movement (from zero to vx) such that object does
 * not cross a boundary (either background or another object)
 */
int ObjectHandler::deltaX(const int x1, const int x2, const int vx, int y) const {
// Explanation of algorithm:  The boundaries are drawn as contiguous
// lines 1 pixel wide.  Since DX,DY are not necessarily 1, we must
// detect boundary crossing.  If vx positive, examine each pixel from
// x1 old to x2 new, else x2 old to x1 new, both at the y2 line.
// If vx zero, no need to check.  If vy non-zero then examine each
// pixel on the line segment x1 to x2 from y old to y new.
// Fix from Hugo I v1.5:
// Note the diff is munged in the return statement to cater for a special
// cases arising from differences in image widths from one sequence to
// another.  The problem occurs reversing direction at a wall where the
// new image intersects before the object can move away.  This is cured
// by comparing the intersection with half the object width pos. If the
// intersection is in the other half wrt the intended direction, use the
// desired vx, else use the computed delta.  i.e. believe the desired vx

	debugC(3, kDebugEngine, "deltaX(%d, %d, %d, %d)", x1, x2, vx, y);

	if (vx == 0)
		return 0;                                  // Object stationary

	y *= kCompLineSize;                             // Offset into boundary file
	if (vx > 0) {
		// Moving to right
		for (int i = x1 >> 3; i <= (x2 + vx) >> 3; i++) {// Search by byte
			int b = Utils::firstBit((byte)(_boundary[y + i] | _objBound[y + i]));
			if (b < 8) {   // b is index or 8
				// Compute x of boundary and test if intersection
				b += i << 3;
				if ((b >= x1) && (b <= x2 + vx))
					return (b < x1 + ((x2 - x1) >> 1)) ? vx : b - x2 - 1; // return dx
			}
		}
	} else {
		// Moving to left
		for (int i = x2 >> 3; i >= (x1 + vx) >> 3; i--) {// Search by byte
			int b = Utils::lastBit((byte)(_boundary[y + i] | _objBound[y + i]));
			if (b < 8) {    // b is index or 8
				// Compute x of boundary and test if intersection
				b += i << 3;
				if ((b >= x1 + vx) && (b <= x2))
					return (b > x1 + ((x2 - x1) >> 1)) ? vx : b - x1 + 1; // return dx
			}
		}
	}
	return vx;
}

/**
 * Similar to Delta_x, but for movement in y direction.  Special case of
 * bytes at end of line segment; must only count boundary bits falling on
 * line segment.
 */
int ObjectHandler::deltaY(const int x1, const int x2, const int vy, const int y) const {
	debugC(3, kDebugEngine, "deltaY(%d, %d, %d, %d)", x1, x2, vy, y);

	if (vy == 0)
		return 0;                                   // Object stationary

	int inc = (vy > 0) ? 1 : -1;
	for (int j = y + inc; j != (y + vy + inc); j += inc) { //Search by byte
		for (int i = x1 >> 3; i <= x2 >> 3; i++) {
			int b = _boundary[j * kCompLineSize + i] | _objBound[j * kCompLineSize + i];
			if (b != 0) {                           // Any bit set
				// Make sure boundary bits fall on line segment
				if (i == (x2 >> 3))                 // Adjust right end
					b &= 0xff << ((i << 3) + 7 - x2);
				else if (i == (x1 >> 3))            // Adjust left end
					b &= 0xff >> (x1 - (i << 3));
				if (b)
					return j - y - inc;
			}
		}
	}
	return vy;
}

/**
 * Store a horizontal line segment in the object boundary file
 */
void ObjectHandler::storeBoundary(const int x1, const int x2, const int y) {
	debugC(5, kDebugEngine, "storeBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		byte *b = &_objBound[y * kCompLineSize + i];// get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b |= 0xff << ((i << 3) + 7 - x2);
		else if (i == x1 >> 3)                      // Adjust left end
			*b |= 0xff >> (x1 - (i << 3));
		else
			*b = 0xff;
	}
}

/**
 * Clear a horizontal line segment in the object boundary file
 */
void ObjectHandler::clearBoundary(const int x1, const int x2, const int y) {
	debugC(5, kDebugEngine, "clearBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		byte *b = &_objBound[y * kCompLineSize + i];// get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b &= ~(0xff << ((i << 3) + 7 - x2));
		else if (i == x1 >> 3)                      // Adjust left end
			*b &= ~(0xff >> (x1 - (i << 3)));
		else
			*b = 0;
	}
}

/**
 * Clear a horizontal line segment in the screen boundary file
 * Used to fix some data issues
 */
void ObjectHandler::clearScreenBoundary(const int x1, const int x2, const int y) {
	debugC(5, kDebugEngine, "clearScreenBoundary(%d, %d, %d)", x1, x2, y);

	for (int i = x1 >> 3; i <= x2 >> 3; i++) {      // For each byte in line
		byte *b = &_boundary[y * kCompLineSize + i];// get boundary byte
		if (i == x2 >> 3)                           // Adjust right end
			*b &= ~(0xff << ((i << 3) + 7 - x2));
		else if (i == x1 >> 3)                      // Adjust left end
			*b &= ~(0xff >> (x1 - (i << 3)));
		else
			*b = 0;
	}
}

/**
 * An object has collided with a boundary. See if any actions are required
 */
void ObjectHandler::boundaryCollision(Object *obj) {
	debugC(1, kDebugEngine, "boundaryCollision");

	if (obj == _vm->_hero) {
		// Hotspots only relevant to HERO
		int x;
		if (obj->_vx > 0)
			x = obj->_x + obj->_currImagePtr->_x2;
		else
			x = obj->_x + obj->_currImagePtr->_x1;
		int y = obj->_y + obj->_currImagePtr->_y2;

		int16 index = _vm->_mouse->findExit(x, y, obj->_screenIndex);
		if (index >= 0)
			_vm->_scheduler->insertActionList(_vm->_mouse->getHotspotActIndex(index));

	} else {
		// Check whether an object collided with HERO
		int dx = _vm->_hero->_x + _vm->_hero->_currImagePtr->_x1 - obj->_x - obj->_currImagePtr->_x1;
		int dy = _vm->_hero->_y + _vm->_hero->_currImagePtr->_y2 - obj->_y - obj->_currImagePtr->_y2;
		// If object's radius is infinity, use a closer value
		int8 radius = obj->_radius;
		if (radius < 0)
			radius = kStepDx * 2;
		if ((abs(dx) <= radius) && (abs(dy) <= radius))
			_vm->_scheduler->insertActionList(obj->_actIndex);
	}
}

} // End of namespace Hugo
