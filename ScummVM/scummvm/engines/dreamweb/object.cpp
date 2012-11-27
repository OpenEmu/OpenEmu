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

#include "dreamweb/dreamweb.h"

namespace DreamWeb {

void DreamWebEngine::showRyanPage() {
	showFrame(_icons1, kInventx + 167, kInventy - 12, 12, 0);
	showFrame(_icons1, kInventx + 167 + 18 * _vars._ryanPage, kInventy - 12, 13 + _vars._ryanPage, 0);
}

void DreamWebEngine::findAllRyan() {
	memset(_ryanInvList, 0xff, sizeof(_ryanInvList));
	for (size_t i = 0; i < kNumexobjects; ++i) {
		const DynObject *extra = getExAd(i);
		if (extra->mapad[0] != kExObjectType)
			continue;
		if (extra->mapad[1] != 0xff)
			continue;
		uint8 slot = extra->mapad[2];
		assert(slot < 30);
		_ryanInvList[slot]._index = i;
		_ryanInvList[slot]._type = kExObjectType;
	}
}

void DreamWebEngine::fillRyan() {
	ObjectRef *inv = &_ryanInvList[_vars._ryanPage * 10];
	findAllRyan();
	for (size_t i = 0; i < 2; ++i) {
		for (size_t j = 0; j < 5; ++j) {
			obToInv(inv->_index, inv->_type, kInventx + j * kItempicsize, kInventy + i * kItempicsize);
			++inv;
		}
	}
	showRyanPage();
}

bool DreamWebEngine::isItWorn(const DynObject *object) {
	return (object->objId[0] == 'W'-'A') && (object->objId[1] == 'E'-'A');
}

void DreamWebEngine::wornError() {
	_commandType = 255;
	delPointer();
	printMessage(76, 21, 57, 240, false);
	workToScreenM();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	_commandType = 255;
	workToScreenM();
}

void DreamWebEngine::makeWorn(DynObject *object) {
	object->objId[0] = 'W'-'A';
	object->objId[1] = 'E'-'A';
}

void DreamWebEngine::obToInv(uint8 index, uint8 flag, uint16 x, uint16 y) {
	showFrame(_icons1, x - 2, y - 1, 10, 0);
	if (index == 0xff)
		return;

	if (flag == kExObjectType)
		showFrame(_exFrames, x + 18, y + 19, 3 * index + 1, 128);
	else
		showFrame(_freeFrames, x + 18, y + 19, 3 * index + 1, 128);

	const DynObject *object = (const DynObject *)getAnyAdDir(index, flag);
	bool worn = isItWorn(object);
	if (worn)
		showFrame(_icons1, x - 3, y - 2, 7, 0);
}

void DreamWebEngine::obPicture() {
	if (_objectType == kSetObjectType1)
		return;
	uint8 frame = 3 * _command + 1;
	if (_objectType == kExObjectType)
		showFrame(_exFrames, 160, 68, frame, 0x80);
	else
		showFrame(_freeFrames, 160, 68, frame, 0x80);
}

void DreamWebEngine::obIcons() {
	uint8 slotSize, slotCount;
	getAnyAd(&slotSize, &slotCount);
	if (slotSize != 0xff) {
		// can open it
		showFrame(_icons2, 210, 1, 4, 0);
	}

	showFrame(_icons2, 260, 1, 1, 0);
}

void DreamWebEngine::examineOb(bool examineAgain) {
	_pointerMode = 0;
	_timeCount = 0;

	while (true) {
		if (examineAgain) {
			_inMapArea = 0;
			_examAgain = 0;
			_openedOb = 255;
			_openedType = 255;
			_invOpen = 0;
			_objectType = _commandType;
			_itemFrame = 0;
			_pointerFrame = 0;
			createPanel();
			showPanel();
			showMan();
			showExit();
			obIcons();
			obPicture();
			describeOb();
			underTextLine();
			_commandType = 255;
			readMouse();
			showPointer();
			workToScreen();
			delPointer();
			examineAgain = false;
		}

		readMouse();
		showPointer();
		waitForVSync();
		dumpPointer();
		dumpTextLine();
		delPointer();
		_getBack = 0;

		switch (_invOpen) {
		case 0: {
			RectWithCallback examList[] = {
				{ 273,320,157,198,&DreamWebEngine::getBackFromOb },
				{ 260,300,0,44,&DreamWebEngine::useObject },
				{ 210,254,0,44,&DreamWebEngine::selectOpenOb },
				{ 144,176,64,96,&DreamWebEngine::setPickup },
				{ 0,50,50,200,&DreamWebEngine::examineInventory },
				{ 0,320,0,200,&DreamWebEngine::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkCoords(examList);
			break;
		}
		case 1: {
			// Note: This table contains the non-constant _openChangeSize!
			RectWithCallback invList1[] = {
				{ 273,320,157,198,&DreamWebEngine::getBackFromOb },
				{ 255,294,0,24,&DreamWebEngine::dropObject },
				{ kInventx+167,kInventx+167+(18*3),kInventy-18,kInventy-2,&DreamWebEngine::incRyanPage },
				{ kInventx,_openChangeSize,kInventy+100,kInventy+100+kItempicsize,&DreamWebEngine::useOpened },
				{ kInventx,kInventx+(5*kItempicsize),kInventy,kInventy+(2*kItempicsize),&DreamWebEngine::inToInv },
				{ 0,320,0,200,&DreamWebEngine::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkCoords(invList1);
			break;
		}
		default: {
			RectWithCallback withList1[] = {
				{ 273,320,157,198,&DreamWebEngine::getBackFromOb },
				{ kInventx+167,kInventx+167+(18*3),kInventy-18,kInventy-2,&DreamWebEngine::incRyanPage },
				{ kInventx,kInventx+(5*kItempicsize), kInventy,kInventy+(2*kItempicsize),&DreamWebEngine::selectOb },
				{ 0,320,0,200,&DreamWebEngine::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkCoords(withList1);
			break;
		}
		}

		if (_quitRequested)
			break;
		if (_examAgain != 0)
			examineAgain = true;
		else if (_getBack != 0)
			break;
	}

	_pickUp = 0;
	if (_vars._watchingTime != 0 || _newLocation == 255) {
		// isWatching
		makeMainScreen();
	}

	_invOpen = 0;
	_openedOb = 255;
}

void DreamWebEngine::inventory() {
	if (_vars._manDead == 1 || _vars._watchingTime != 0) {
		blank();
		return;
	}

	commandOnlyCond(32, 239);

	if (_mouseButton == _oldButton)
		return;
	if (!(_mouseButton & 1)) // only on left mouse button
		return;


	_timeCount = 0;
	_pointerMode = 0;
	_inMapArea = 0;
	animPointer();
	createPanel();
	showPanel();
	examIcon();
	showMan();
	showExit();
	underTextLine();
	_pickUp = 0;
	_invOpen = 2;
	openInv();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	_openedOb = 255;
	examineOb(false);
}

void DreamWebEngine::transferText(uint8 from, uint8 to) {
	_exText.setOffset(to, _vars._exTextPos);
	const char *src = _freeDesc.getString(from);
	char *dst = _exText._text + _vars._exTextPos;

	size_t len = strlen(src);
	memcpy(dst, src, len + 1);
	_vars._exTextPos += len + 1;
}

void DreamWebEngine::getBackFromOb() {
	if (_pickUp != 1)
		getBack1();
	else
		blank();
}

byte DreamWebEngine::getOpenedSlotCount() {
	byte obj = _openedOb;
	switch (_openedType) {
	case kExObjectType:
		return getExAd(obj)->slotCount;
	case kFreeObjectType:
		return getFreeAd(obj)->slotCount;
	default:
		return getSetAd(obj)->slotCount;
	}
}

byte DreamWebEngine::getOpenedSlotSize() {
	byte obj = _openedOb;
	switch (_openedType) {
	case kExObjectType:
		return getExAd(obj)->slotSize;
	case kFreeObjectType:
		return getFreeAd(obj)->slotSize;
	default:
		return getSetAd(obj)->slotSize;
	}
}

void DreamWebEngine::openOb() {
	uint8 commandLine[64] = "OBJECT NAME ONE                         ";

	copyName(_openedType, _openedOb, commandLine);

	printMessage(kInventx, kInventy+86, 62, 240, false);

	printDirect(commandLine, _lastXPos + 5, kInventy+86, 220, false);

	fillOpen();
	_openChangeSize = getOpenedSlotCount() * kItempicsize + kInventx;
}

void DreamWebEngine::identifyOb() {
	if (_vars._watchingTime != 0) {
		blank();
		return;
	}

	uint16 initialX = _mouseX - _mapAdX;
	uint16 initialY = _mouseY - _mapAdY;

	if (initialX >= 22 * 8 || initialY >= 20 * 8) {
		blank();
		return;
	}

	byte x = initialX & 0xFF;
	byte y = initialY & 0xFF;

	_inMapArea = 1;
	_pointersPath = findPathOfPoint(x, y);
	_pointerFirstPath = findFirstPath(x, y);

	if (checkIfEx(x, y) || checkIfFree(x, y) ||
		checkIfPerson(x, y) || checkIfSet(x, y))
		return; // finishidentify

	x = (_mouseX - _mapAdX) & 0xFF;
	y = (_mouseY - _mapAdY) & 0xFF;
	byte flag, flagEx, type, flagX, flagY;

	checkOne(x, y, &flag, &flagEx, &type, &flagX, &flagY);

	if (type != 0 && _vars._manDead != 1)
		obName(type, 3);
	else
		blank();
}

ObjectRef DreamWebEngine::findInvPos() {
	uint16 x = _mouseX - kInventx;
	uint16 y = _mouseY - kInventy;
	uint8 pos = (x / kItempicsize) + (y / kItempicsize) * 5;
	uint8 invPos = _vars._ryanPage * 10 + pos;
	_lastInvPos = invPos;
	return _ryanInvList[invPos];
}

void DreamWebEngine::selectOb() {
	ObjectRef objectId = findInvPos();
	if (objectId._index == 255) {
		blank();
		return;
	}

	_withObject = objectId._index;
	_withType   = objectId._type;

	if (objectId != _oldSubject || _commandType != 221) {
		if (objectId == _oldSubject)
			_commandType = 221;
		_oldSubject = objectId;
		commandWithOb(0, objectId._type, objectId._index);
	}

	if (_mouseButton == _oldButton || !(_mouseButton & 1))
		return;

	delPointer();
	_invOpen = 0;
	useRoutine();
}

void DreamWebEngine::setPickup() {
	if (_objectType != kSetObjectType1 && _objectType != kSetObjectType3) {
		// Object types 1 and 3 are excluded, so the resulting object is a DynObject
		uint8 dummy;
		DynObject *object = (DynObject *)getAnyAd(&dummy, &dummy);
		if (object->mapad[0] == 4) {
			blank();
			return;
		}
	} else {
		blank();
		return;
	}

	if (_commandType != 209) {
		_commandType = 209;
		commandWithOb(33, _objectType, _command);
	}

	if (_mouseButton != 1 || _mouseButton == _oldButton)
		return;

	createPanel();
	showPanel();
	showMan();
	showExit();
	examIcon();
	_pickUp = 1;
	_invOpen = 2;

	if (_objectType != kExObjectType) {
		assert(_objectType == kFreeObjectType);
		_openedOb = 255;
		_itemFrame = transferToEx(_command);
		_objectType = kExObjectType;
		DynObject *object = getExAd(_itemFrame);
		object->mapad[0] = 20;
		object->mapad[1] = 255;
	} else {
		_itemFrame = _command;
		_openedOb = 255;
	}

	openInv();
	workToScreenM();
}

void DreamWebEngine::deleteExFrame(uint8 frameNum) {
	Frame *frame = &_exFrames._frames[frameNum];

	uint16 frameSize = frame->width * frame->height;
	// Note: the original asm didn't subtract frameSize from remainder
	uint16 remainder = kExframeslen - frame->ptr() - frameSize;
	uint16 startOff = frame->ptr();
	uint16 endOff = startOff + frameSize;

	// Shift frame data after this one down
	memmove(&_exFrames._data[startOff], &_exFrames._data[endOff], remainder);

	// Combined frame data is now frameSize smaller
	_vars._exFramePos -= frameSize;

	// Adjust all frame pointers pointing into the shifted data
	for (unsigned int i = 0; i < 3*kNumexobjects; ++i) {
		frame = &_exFrames._frames[i];
		if (frame->ptr() >= startOff)
			frame->setPtr(frame->ptr() - frameSize);
	}
}

void DreamWebEngine::deleteExText(uint8 textNum) {
	uint16 offset = _exText.getOffset(textNum);

	uint16 startOff = offset;
	uint16 textSize = strlen(_exText.getString(textNum)) + 1;
	uint16 endOff = startOff + textSize;
	uint16 remainder = kExtextlen - offset - textSize;

	// Shift text data after this one down
	memmove(&_exText._text[startOff], &_exText._text[endOff], remainder);

	// Combined text data is now frameSize smaller
	_vars._exTextPos -= textSize;

	// Adjust all text pointers pointing into the shifted data
	for (unsigned int i = 0; i < kNumexobjects; ++i) {
		uint16 t = _exText.getOffset(i);
		if (t >= offset + textSize)
			_exText.setOffset(i, t - textSize);
	}
}

void DreamWebEngine::deleteExObject(uint8 index) {
	DynObject *obj = getExAd(index);

	memset(obj, 0xFF, sizeof(DynObject));

	deleteExFrame(3*index);
	deleteExFrame(3*index + 1);

	deleteExText(index);

	for (uint8 i = 0; i < kNumexobjects; ++i) {
		DynObject *t = getExAd(i);
		// Is this object contained in the one we've just deleted?
		if (t->mapad[0] == 4 && t->mapad[1] == index)
			deleteExObject(i);
	}
}

void DreamWebEngine::removeObFromInv() {
	if (_command == 100)
		return; // object doesn't exist

	assert(_objectType == kExObjectType);

	deleteExObject(_command);
}

void DreamWebEngine::inToInv() {
	if (!_pickUp) {
		outOfInv();
		return;
	}

	ObjectRef subject = findInvPos();

	if (subject._index != 255) {
		swapWithInv();
		return;
	}

	subject._type = _objectType;
	subject._index = _itemFrame;

	if (subject != _oldSubject || _commandType != 220) {
		if (subject == _oldSubject)
			_commandType = 220;
		_oldSubject = subject;
		commandWithOb(35, subject._type, subject._index);
	}

	if (_mouseButton == _oldButton || !(_mouseButton & 1))
		return; // notletgo2

	delPointer();
	DynObject *object = getExAd(_itemFrame);
	object->mapad[0] = 4;
	object->mapad[1] = 255;
	object->mapad[2] = _lastInvPos;
	_pickUp = 0;
	fillRyan();
	readMouse();
	showPointer();
	outOfInv();
	workToScreen();
	delPointer();
}

void DreamWebEngine::outOfInv() {
	ObjectRef subject = findInvPos();

	if (subject._index == 255) {
		blank();
		return;
	}

	if (_mouseButton == 2) {
		reExFromInv();
		return;
	}

	if (subject != _oldSubject || _commandType != 221) {
		if (subject == _oldSubject)
			_commandType = 221;
		_oldSubject = subject;
		commandWithOb(36, subject._type, subject._index);
	}

	if (_mouseButton == _oldButton || !(_mouseButton & 1))
		return;

	delPointer();
	_pickUp = 1;
	subject = findInvPos();
	_objectType = subject._type;
	_itemFrame = subject._index;
	assert(subject._type == kExObjectType);
	DynObject *object = getExAd(subject._index);
	object->mapad[0] = 20;
	object->mapad[1] = 255;
	fillRyan();
	readMouse();
	showPointer();
	inToInv();
	workToScreen();
	delPointer();
}

void DreamWebEngine::purgeALocation(uint8 index) {
	// index == al
	for (uint8 i = 0; i < kNumexobjects; ++i) {
		DynObject *t = getExAd(i);
		if (t->currentLocation == index && t->mapad[0] == 0) {
			deleteExObject(i);
		}
	}
}

const uint8 *DreamWebEngine::getObTextStart() {
	const uint8 *textBase = 0;
	const uint8 *text;
	uint16 textOff = 0;
	if (_objectType == kFreeObjectType) {
		text = (const uint8 *)_freeDesc.getString(_command);
	} else if (_objectType == kSetObjectType1) {
		textBase = (const uint8 *)_setDesc._text;
		textOff = kNumSetTexts * 2;
		text = (const uint8 *)_setDesc.getString(_command);
	} else {
		text = (const uint8 *)_exText.getString(_command);
	}

	if (_objectType != kSetObjectType1)
		return text;

	const uint8 *obname = text;
	while (true) {
		const uint8 *start = text;
		findNextColon(&text);

		// Not an empty description string?
		if (*text != 0 && *text != ':')
			return start;

		// If the description string (of a SetObjectType1 object) is empty,
		// look for an object with the same name.
		// Example: Eden's garage door outside has two parts. The right part
		// has no description of its own but uses that of the left part.

		bool found = false;
		do {
			text++;
			uint8 c = *obname;

			// scan for matching first character
			while (*text != c) {
				text++;

				// arbitrary give-up counter
				// FIXME: Make this more precise to avoid reading out of bounds
				if (text - (textBase - textOff) >= 8000) {
					warning("Object description for %d/%d not found", _objectType, _command);
					return obname;
				}
			}

			// found matching first character, so match the rest
			const uint8 *s1 = obname;
			const uint8 *s2 = text;
			do {
				s1++;
				s2++;
			} while (*s1 != ':' && *s1 != 0 && *s1 == *s2);

			if (*s1 == ':' || *s1 == 0)
				found = true; // (prefix) matched the entire object name
		} while (!found);

		// We found an object with the same name. The next loop iteration
		// will check if this one again has an empty description.
	}
}

void DreamWebEngine::dropObject() {
	if (_commandType != 223) {
		_commandType = 223;
		if (!_pickUp) {
			blank();
			return;
		}
		commandWithOb(37, _objectType, _itemFrame);
	}

	if (_mouseButton == _oldButton || !(_mouseButton & 1))
		return;

	if (isItWorn(getEitherAd())) {
		wornError();
		return;
	}

	if (_realLocation != 47) {
		byte flag, flagEx, type, flagX, flagY;
		checkOne(_ryanX + 12, _ryanY + 12, &flag, &flagEx, &type, &flagX, &flagY);

		if (flag >= 2) {
			dropError();
			return;
		}
	} else {
		dropError();
		return;
	}

	if (_mapXSize == 64 && _mapYSize == 64) {
		// Inside lift
		dropError();
		return;
	}

	if (compare(_itemFrame, kExObjectType, "GUNA") || compare(_itemFrame, kExObjectType, "SHLD")) {
		cantDrop();
		return;
	}

	_objectType = kExObjectType;
	DynObject *object = getExAd(_itemFrame);
	object->mapad[0] = 0;
	object->mapad[1] = ((_ryanX + 4) >> 4) + _mapX;
	object->mapad[2] = (_ryanX + 4) & 0xF;
	object->mapad[3] = ((_ryanY + 8) >> 4) + _mapY;
	object->mapad[4] = (_ryanY + 8) & 0xF;
	_pickUp = 0;
	object->currentLocation = _realLocation;
}

bool DreamWebEngine::checkObjectSize() {
	byte containerSize = getOpenedSlotSize();
	DynObject *object = getEitherAd();
	// If there is no size defined for the object in the editor, set its size
	// to 6. This could be a bad idea, according to the original source.
	byte objectSize = (object->objectSize != 255) ? object->objectSize : 6;

	if (containerSize >= 100) {
		// Special type of container: only objects of the same special type fit.
		if (containerSize == objectSize)
			return true;

		errorMessage3();
		return false;
	}

	if (objectSize >= 100) {
		// Special type of object, but a regular container.
		// Subtract 100 from the size to get its regular size.
		objectSize -= 100;
	}

	if (containerSize >= objectSize)
		return true;

	errorMessage2();
	return false;
}

void DreamWebEngine::selectOpenOb() {
	uint8 slotSize, slotCount;
	getAnyAd(&slotSize, &slotCount);
	if (slotCount == 255) {
		// Can't open the object
		blank();
		return;
	}

	if (_commandType != 224) {
		_commandType = 224;
		commandWithOb(38, _objectType, _command);
	}

	if (_mouseButton == _oldButton || !(_mouseButton & 1))
		return;

	_openedOb = _command;
	_openedType = _objectType;
	createPanel();
	showPanel();
	showMan();
	examIcon();
	showExit();
	openInv();
	openOb();
	underTextLine();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamWebEngine::reExFromInv() {
	ObjectRef objectId = findInvPos();
	_commandType = objectId._type;
	_command     = objectId._index;
	_examAgain = 1;
	_pointerMode = 0;
}

void DreamWebEngine::swapWithInv() {
	ObjectRef subject;
	subject._type = _objectType;
	subject._index = _itemFrame;
	if (subject != _oldSubject || _commandType != 243) {
		if (subject == _oldSubject)
			_commandType = 243;
		_oldSubject = subject;
		commandWithOb(34, subject._type, subject._index);
	}

	if (_mouseButton == _oldButton || !(_mouseButton & 1))
		return;

	byte prevType = _objectType;
	byte prevFrame = _itemFrame;
	ObjectRef objectId = findInvPos();
	_itemFrame = objectId._index;
	_objectType = objectId._type;
	DynObject *object = getEitherAd();
	object->mapad[0] = 20;
	object->mapad[1] = 255;
	byte prevType2 = _objectType;
	byte prevFrame2 = _itemFrame;
	_objectType = prevType;
	_itemFrame = prevFrame;
	delPointer();
	object = getEitherAd();
	object->mapad[0] = 4;
	object->mapad[1] = 255;
	object->mapad[2] = _lastInvPos;
	_objectType = prevType2;
	_itemFrame = prevFrame2;
	fillRyan();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamWebEngine::useOpened() {
	if (_openedOb == 255)
		return;	// cannot use opened object

	if (!_pickUp) {
		outOfOpen();
		return;
	}

	ObjectRef objectId = findOpenPos();

	if (objectId._index != 255) {
		swapWithOpen();
		return;
	}

	if (_pickUp != 1) {
		blank();
		return;
	}

	objectId._type = _objectType;
	objectId._index = _itemFrame;
	if (objectId != _oldSubject || _commandType != 227) {
		if (objectId == _oldSubject)
			_commandType = 227;
		_oldSubject = objectId;
		commandWithOb(35, objectId._type, objectId._index);
	}

	if (_mouseButton == _oldButton || !(_mouseButton & 1))
		return;

	if (isItWorn(getEitherAd())) {
		wornError();
		return;
	}

	delPointer();

	if (_itemFrame == _openedOb &&
		_objectType == _openedType) {
		errorMessage1();
		return;
	}

	if (!checkObjectSize())
		return;

	_pickUp = 0;
	DynObject *object = getEitherAd();
	object->mapad[0] = _openedType;
	object->mapad[1] = _openedOb;
	object->mapad[2] = _lastInvPos;
	object->mapad[3] = _realLocation;
	fillOpen();
	underTextLine();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamWebEngine::outOfOpen() {
	if (_openedOb == 255)
		return;	// cannot use opened object

	ObjectRef objectId = findOpenPos();

	if (objectId._index == 255) {
		blank();
		return;
	}

	if (objectId != _oldSubject || _commandType != 228) {
		if (objectId == _oldSubject)
			_commandType = 228;
		_oldSubject = objectId;
		commandWithOb(36, objectId._type, objectId._index);
	}

	if (_mouseButton == _oldButton)
		return;	// notletgo4

	if (_mouseButton != 1) {
		if (_mouseButton == 2)
			reExFromOpen();
		return;
	}

	delPointer();
	_pickUp = 1;
	objectId = findOpenPos();
	_objectType = objectId._type;
	_itemFrame = objectId._index;

	if (_objectType != kExObjectType) {
		assert(objectId._type == kFreeObjectType);
		_itemFrame = transferToEx(objectId._index);
		_objectType = kExObjectType;
	}

	DynObject *object = getEitherAd();
	object->mapad[0] = 20;
	object->mapad[1] = 255;

	fillOpen();
	underTextLine();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamWebEngine::swapWithOpen() {
	ObjectRef subject;
	subject._type = _objectType;
	subject._index = _itemFrame;
	if (subject != _oldSubject || _commandType != 242) {
		if (subject == _oldSubject)
			_commandType = 242;
		_oldSubject = subject;
		commandWithOb(34, subject._type, subject._index);
	}

	if (_mouseButton == _oldButton || !(_mouseButton & 1))
		return;

	if (isItWorn(getEitherAd())) {
		wornError();
		return;
	}

	delPointer();

	if (_itemFrame == _openedOb &&
		_objectType == _openedType) {
		errorMessage1();
		return;
	}

	if (!checkObjectSize())
		return;

	byte prevType = _objectType;
	byte prevFrame = _itemFrame;
	ObjectRef objectId = findOpenPos();
	_objectType = objectId._type;
	_itemFrame  = objectId._index;

	if (_objectType != kExObjectType) {
		assert(objectId._type == kFreeObjectType);
		_itemFrame = transferToEx(objectId._index);
		_objectType = kExObjectType;
	}

	DynObject *object = getEitherAd();
	object->mapad[0] = 20;
	object->mapad[1] = 255;

	byte prevType2 = _objectType;
	byte prevFrame2 = _itemFrame;
	_objectType = prevType;
	_itemFrame = prevFrame;
	object = getEitherAd();
	object->mapad[0] = _openedType;
	object->mapad[1] = _openedOb;
	object->mapad[2] = _lastInvPos;
	object->mapad[3] = _realLocation;
	_objectType = prevType2;
	_itemFrame = prevFrame2;
	fillOpen();
	fillRyan();
	underTextLine();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

ObjectRef DreamWebEngine::findOpenPos() {
	uint8 pos = (_mouseX - kInventx) / kItempicsize;
	_lastInvPos = pos;

	return _openInvList[pos];
}

byte DreamWebEngine::transferToEx(uint8 from) {
	emergencyPurge();

	byte pos = getExPos();
	DynObject *exObject = getExAd(pos);

	DynObject *freeObject = getFreeAd(from);

	memcpy(exObject, freeObject, sizeof(DynObject));

	exObject->currentLocation = _realLocation;
	exObject->initialLocation = _realLocation;
	exObject->index = from;
	exObject->mapad[0] = 4;
	exObject->mapad[1] = 255;
	exObject->mapad[2] = _lastInvPos;

	transferFrame(from, pos, 0);
	transferFrame(from, pos, 1);
	transferText(from, pos);

	freeObject->mapad[0] = 254;

	pickupConts(from, pos);

	return pos;
}

void DreamWebEngine::fillOpen() {
	delTextLine();
	uint8 size = getOpenedSlotCount();
	if (size > 4)
		size = 4;
	findAllOpen();
	for (uint8 i = 0; i < size; ++i) {
		uint8 index = _openInvList[i]._index;
		uint8 type = _openInvList[i]._type;
		obToInv(index, type, kInventx + i * kItempicsize, kInventy + 96);
	}
	underTextLine();
}

void DreamWebEngine::findAllOpen() {
	memset(_openInvList, 0xFF, 32);

	for (uint8 i = 0; i < kNumexobjects; ++i) {
		const DynObject *obj = getExAd(i);
		if (obj->mapad[1] != _openedOb)
			continue;
		if (obj->mapad[0] != _openedType)
			continue;
		if (_openedType != kExObjectType && obj->mapad[3] != _realLocation)
			continue;
		uint8 slot = obj->mapad[2];
		assert(slot < 16);
		_openInvList[slot]._index = i;
		_openInvList[slot]._type = kExObjectType;
	}

	for (uint8 i = 0; i < 80; ++i) {
		const DynObject *obj = getFreeAd(i);
		if (obj->mapad[1] != _openedOb)
			continue;
		if (obj->mapad[0] != _openedType)
			continue;
		uint8 slot = obj->mapad[2];
		_openInvList[slot]._index = i;
		_openInvList[slot]._type = kFreeObjectType;
	}
}

void DreamWebEngine::pickupConts(uint8 from, uint8 containerEx) {
	const DynObject *obj = getFreeAd(from);

	if (obj->slotCount == 255)
		return; // not openable

	for (uint8 index = 0; index < 80; ++index) {
		DynObject *freeObj = getFreeAd(index);

		if (freeObj->mapad[0] != kFreeObjectType)
			continue;
		if (freeObj->mapad[1] != from)
			continue;

		uint8 pos = getExPos();
		DynObject *exObj = getExAd(pos);

		memcpy(exObj, freeObj, sizeof(DynObject));
		exObj->currentLocation = _realLocation;
		exObj->initialLocation = _realLocation;
		exObj->index = index;
		exObj->mapad[0] = 4; // kExObjectType?
		exObj->mapad[1] = containerEx;

		transferFrame(index, pos, 0);
		transferFrame(index, pos, 1);
		transferText(index, pos);

		freeObj->mapad[0] = 0xFF;
	}
}

} // End of namespace DreamWeb
