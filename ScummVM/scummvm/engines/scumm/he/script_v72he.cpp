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

#ifdef ENABLE_HE

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/system.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/dialogs.h"
#include "scumm/file.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/he/sound_he.h"
#include "scumm/util.h"
#include "scumm/verbs.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v72he, x)

void ScummEngine_v72he::setupOpcodes() {
	ScummEngine_v71he::setupOpcodes();

	OPCODE(0x02, o72_pushDWord);
	OPCODE(0x04, o72_getScriptString);
	_opcodes[0x0a].setProc(0, 0);
	OPCODE(0x1b, o72_isAnyOf);
	_opcodes[0x42].setProc(0, 0);
	_opcodes[0x46].setProc(0, 0);
	_opcodes[0x4a].setProc(0, 0);
	_opcodes[0x4e].setProc(0, 0);
	OPCODE(0x50, o72_resetCutscene);
	OPCODE(0x52, o72_findObjectWithClassOf);
	OPCODE(0x54, o72_getObjectImageX);
	OPCODE(0x55, o72_getObjectImageY);
	OPCODE(0x56, o72_captureWizImage);
	OPCODE(0x58, o72_getTimer);
	OPCODE(0x59, o72_setTimer);
	OPCODE(0x5a, o72_getSoundPosition);
	OPCODE(0x5e, o72_startScript);
	OPCODE(0x60, o72_startObject);
	OPCODE(0x61, o72_drawObject);
	OPCODE(0x62, o72_printWizImage);
	OPCODE(0x63, o72_getArrayDimSize);
	OPCODE(0x64, o72_getNumFreeArrays);
	_opcodes[0x97].setProc(0, 0);	// was: o6_setObjectName
	OPCODE(0x9c, o72_roomOps);
	OPCODE(0x9d, o72_actorOps);
	OPCODE(0x9e, o72_verbOps);
	OPCODE(0xa0, o72_findObject);
	OPCODE(0xa4, o72_arrayOps);
	OPCODE(0xae, o72_systemOps);
	OPCODE(0xba, o72_talkActor);
	OPCODE(0xbb, o72_talkEgo);
	OPCODE(0xbc, o72_dimArray);
	OPCODE(0xc0, o72_dim2dimArray);
	OPCODE(0xc1, o72_traceStatus);
	OPCODE(0xc8, o72_kernelGetFunctions);
	OPCODE(0xce, o72_drawWizImage);
	OPCODE(0xcf, o72_debugInput);
	OPCODE(0xd5, o72_jumpToScript);
	OPCODE(0xda, o72_openFile);
	OPCODE(0xdb, o72_readFile);
	OPCODE(0xdc, o72_writeFile);
	OPCODE(0xdd, o72_findAllObjects);
	OPCODE(0xde, o72_deleteFile);
	OPCODE(0xdf, o72_rename);
	OPCODE(0xe1, o72_getPixel);
	OPCODE(0xe3, o72_pickVarRandom);
	OPCODE(0xea, o72_redimArray);
	OPCODE(0xf3, o72_readINI);
	OPCODE(0xf4, o72_writeINI);
	OPCODE(0xf8, o72_getResourceSize);
	OPCODE(0xf9, o72_createDirectory);
	OPCODE(0xfa, o72_setSystemMessage);
}

static const int arrayDataSizes[] = { 0, 1, 4, 8, 8, 16, 32 };

byte *ScummEngine_v72he::defineArray(int array, int type, int dim2start, int dim2end,
											int dim1start, int dim1end) {
	int id;
	int size;
	ArrayHeader *ah;

	assert(dim2start >= 0 && dim2start <= dim2end);
	assert(dim1start >= 0 && dim1start <= dim1end);
	assert(0 <= type && type <= 6);


	if (type == kBitArray || type == kNibbleArray)
		type = kByteArray;

	nukeArray(array);

	id = findFreeArrayId();

	debug(9,"defineArray (array %d, dim2start %d, dim2end %d dim1start %d dim1end %d", id, dim2start, dim2end, dim1start, dim1end);

	if (array & 0x80000000) {
		error("Can't define bit variable as array pointer");
	}

	size = arrayDataSizes[type];

	if (_game.heversion >= 80)
		id |= 0x33539000;

	writeVar(array, id);

	if (_game.heversion >= 80)
		id &= ~0x33539000;

	size *= dim2end - dim2start + 1;
	size *= dim1end - dim1start + 1;
	size >>= 3;

	ah = (ArrayHeader *)_res->createResource(rtString, id, size + sizeof(ArrayHeader));

	ah->type = TO_LE_32(type);
	ah->dim1start = TO_LE_32(dim1start);
	ah->dim1end = TO_LE_32(dim1end);
	ah->dim2start = TO_LE_32(dim2start);
	ah->dim2end = TO_LE_32(dim2end);

	return ah->data;
}

int ScummEngine_v72he::readArray(int array, int idx2, int idx1) {
	debug(9, "readArray (array %d, idx2 %d, idx1 %d)", readVar(array), idx2, idx1);

	if (readVar(array) == 0)
		error("readArray: Reference to zeroed array pointer");

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (ah == NULL || ah->data == NULL)
		error("readArray: invalid array %d (%d)", array, readVar(array));

	if (idx2 < (int)FROM_LE_32(ah->dim2start) || idx2 > (int)FROM_LE_32(ah->dim2end) ||
		idx1 < (int)FROM_LE_32(ah->dim1start) || idx1 > (int)FROM_LE_32(ah->dim1end)) {
		error("readArray: array %d out of bounds: [%d, %d] exceeds [%d..%d, %d..%d]",
			  array, idx1, idx2, FROM_LE_32(ah->dim1start), FROM_LE_32(ah->dim1end),
			  FROM_LE_32(ah->dim2start), FROM_LE_32(ah->dim2end));
	}

	const int offset = (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(idx2 - FROM_LE_32(ah->dim2start)) + (idx1 - FROM_LE_32(ah->dim1start));

	switch (FROM_LE_32(ah->type)) {
	case kByteArray:
	case kStringArray:
		return ah->data[offset];

	case kIntArray:
		return (int16)READ_LE_UINT16(ah->data + offset * 2);

	case kDwordArray:
		return (int32)READ_LE_UINT32(ah->data + offset * 4);
	}

	return 0;
}

void ScummEngine_v72he::writeArray(int array, int idx2, int idx1, int value) {
	debug(9, "writeArray (array %d, idx2 %d, idx1 %d, value %d)", readVar(array), idx2, idx1, value);

	if (readVar(array) == 0)
		error("writeArray: Reference to zeroed array pointer");

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));

	if (!ah)
		error("writeArray: Invalid array (%d) reference", readVar(array));

	if (idx2 < (int)FROM_LE_32(ah->dim2start) || idx2 > (int)FROM_LE_32(ah->dim2end) ||
		idx1 < (int)FROM_LE_32(ah->dim1start) || idx1 > (int)FROM_LE_32(ah->dim1end)) {
		error("writeArray: array %d out of bounds: [%d, %d] exceeds [%d..%d, %d..%d]",
			  array, idx1, idx2, FROM_LE_32(ah->dim1start), FROM_LE_32(ah->dim1end),
			  FROM_LE_32(ah->dim2start), FROM_LE_32(ah->dim2end));
	}

	const int offset = (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(idx2 - FROM_LE_32(ah->dim2start)) - FROM_LE_32(ah->dim1start) + idx1;

	switch (FROM_LE_32(ah->type)) {
	case kByteArray:
	case kStringArray:
		ah->data[offset] = value;
		break;

	case kIntArray:
		WRITE_LE_UINT16(ah->data + offset * 2, value);
		break;

	case kDwordArray:
		WRITE_LE_UINT32(ah->data + offset * 4, value);
		break;
	}
}

int ScummEngine_v72he::setupStringArray(int size) {
	writeVar(0, 0);
	defineArray(0, kStringArray, 0, 0, 0, size + 1);
	writeArray(0, 0, 0, 0);
	return readVar(0);
}

void ScummEngine_v72he::readArrayFromIndexFile() {
	int num;
	int a, b, c;

	while ((num = _fileHandle->readUint16LE()) != 0) {
		a = _fileHandle->readUint16LE();
		b = _fileHandle->readUint16LE();
		c = _fileHandle->readUint16LE();

		if (c == 1)
			defineArray(num, kBitArray, 0, a, 0, b);
		else
			defineArray(num, kDwordArray, 0, a, 0, b);
	}
}

void ScummEngine_v72he::copyScriptString(byte *dst, int dstSize) {
	byte string[1024];
	byte chr;
	int pos = 0;

	int array = pop();
	if (array == -1) {
		if (_stringLength == 1)
			error("String stack underflow");

		_stringLength -= 2;
		 while ((chr = _stringBuffer[_stringLength]) != 0) {
			string[pos] = chr;
			pos++;

			if (pos > dstSize)
				error("String too long to pop");

			_stringLength--;
		}

		string[pos] = 0;
		_stringLength++;

		// Reverse string
		int len = resStrLen(string);
		while (len--)
			*dst++ = string[len];
	} else {
		writeVar(0, array);
		while ((chr = readArray(0, 0, pos)) != 0) {
			*dst++ = chr;
			pos++;
		}
	}
	*dst = 0;
}

void ScummEngine_v72he::decodeScriptString(byte *dst, bool scriptString) {
	const byte *src;
	int args[31];
	int num, len, val;
	byte chr, string[1024];
	memset(args, 0, sizeof(args));
	memset(string, 0, sizeof(string));

	// Get stack list, plus one
	num = pop();
	for (int i = num; i >= 0; i--)
		args[i] = pop();

	// Get string
	if (scriptString) {
		len = resStrLen(_scriptPointer) + 1;
		memcpy(string, _scriptPointer, len);
		_scriptPointer += len;
	} else {
		copyScriptString(string, sizeof(string));
		len = resStrLen(string) + 1;
	}

	// Decode string
	num = 0;
	val = 0;
	while (len--) {
		chr = string[num++];
		if (chr == '%') {
			chr = string[num++];
			switch (chr) {
			case 'b':
				//dst += sprintf((char *)dst, "%b", args[val++]);
				break;
			case 'c':
				*dst++ = args[val++];
				break;
			case 'd':
				dst += sprintf((char *)dst, "%d", args[val++]);
				break;
			case 's':
				src = getStringAddress(args[val++]);
				if (src) {
					while (*src != 0)
						*dst++ = *src++;
				}
				break;
			case 'x':
				dst += sprintf((char *)dst, "%x", args[val++]);
				break;
			default:
				*dst++ = '%';
				num--;
				break;
			}
		} else {
			*dst++ = chr;
		}
	}
	*dst = 0;
}

int ScummEngine_v72he::findObject(int x, int y, int num, int *args) {
	int b, cls, i, result;

	for (i = 1; i < _numLocalObjects; i++) {
		result = 0;
		if ((_objs[i].obj_nr < 1) || getClass(_objs[i].obj_nr, kObjectClassUntouchable))
			continue;

		// Check polygon bounds
		if (_wiz->polygonDefined(_objs[i].obj_nr)) {
			if (_wiz->polygonHit(_objs[i].obj_nr, x, y))
				result = _objs[i].obj_nr;
			else if (VAR_POLYGONS_ONLY != 0xFF && VAR(VAR_POLYGONS_ONLY))
				continue;
		}

		if (!result) {
			// Check object bounds
			if (_objs[i].x_pos <= x && _objs[i].width + _objs[i].x_pos > x &&
			    _objs[i].y_pos <= y && _objs[i].height + _objs[i].y_pos > y)
					result = _objs[i].obj_nr;
		}

		if (result) {
			if (!num)
				return result;

			// Check object class
			cls = args[0];
			b = getClass(_objs[i].obj_nr, cls);
			if ((cls & 0x80 && b) || (!(cls & 0x80) && !b))
				return result;
		}
	}

	return 0;
}

void ScummEngine_v72he::o72_pushDWord() {
	push(fetchScriptDWordSigned());
}

void ScummEngine_v72he::o72_getScriptString() {
	byte chr;

	while ((chr = fetchScriptByte()) != 0) {
		_stringBuffer[_stringLength] = chr;
		_stringLength++;

		if (_stringLength >= 4096)
			error("String stack overflow");
	}

	_stringBuffer[_stringLength] = 0;
	_stringLength++;
}

void ScummEngine_v72he::o72_isAnyOf() {
	int args[128];
	int num, value;

	num = getStackList(args, ARRAYSIZE(args));
	value = pop();

	for (int i = 0; i < num; i++) {
		if (args[i] == value) {
			push(1);
			return;
		}
	}

	push(0);
}

void ScummEngine_v72he::o72_resetCutscene() {
	int idx;

	idx = vm.cutSceneStackPointer;
	vm.cutSceneStackPointer = 0;
	vm.cutScenePtr[idx] = 0;
	vm.cutSceneScript[idx] = 0;

	VAR(VAR_OVERRIDE) = 0;
}

void ScummEngine_v72he::o72_findObjectWithClassOf() {
	int args[16], num;

	num = getStackList(args, ARRAYSIZE(args));
	int y = pop();
	int x = pop();
	int r = findObject(x, y, num, args);
	push(r);
}

void ScummEngine_v72he::o72_getObjectImageX() {
	int object = pop();
	int objnum = getObjectIndex(object);

	if (objnum == -1) {
		push(0);
		return;
	}

	push(_objs[objnum].x_pos / 8);
}

void ScummEngine_v72he::o72_getObjectImageY() {
	int object = pop();
	int objnum = getObjectIndex(object);

	if (objnum == -1) {
		push(0);
		return;
	}

	push(_objs[objnum].y_pos / 8);
}

void ScummEngine_v72he::o72_captureWizImage() {
	Common::Rect grab;
	grab.bottom = pop() + 1;
	grab.right = pop() + 1;
	grab.top = pop();
	grab.left = pop();
	_wiz->captureWizImage(pop(), grab, false, true);
}

void ScummEngine_v72he::o72_getTimer() {
	int timer = pop();
	byte cmd = fetchScriptByte();

	if (cmd == 10 || cmd == 50) {
		push(getHETimer(timer));
	} else {
		push(0);
	}
}

void ScummEngine_v72he::o72_setTimer() {
	int timer = pop();
	byte cmd = fetchScriptByte();

	if (cmd == 158 || cmd == 61) {
		setHETimer(timer);
	} else {
		error("TIMER command %d?", cmd);
	}
}

void ScummEngine_v72he::o72_getSoundPosition() {
	int snd = pop();
	push(((SoundHE *)_sound)->getSoundPos(snd));
}

void ScummEngine_v72he::o72_startScript() {
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();

	// HACK: The credits script in Russian HE99 version of Freddi Fish 3
	// uses null strings, causing various errors, so skip it.
	if (_game.id == GID_FREDDI3 && _game.heversion == 99 && _language == Common::RU_RUS &&
		_currentRoom == 40 && script == 2057) {
			return;
	}

	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v72he::o72_startObject() {
	int args[25];
	int script, entryp;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	entryp = pop();
	script = pop();
	flags = fetchScriptByte();
	runObjectScript(script, entryp, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v72he::o72_drawObject() {
	byte subOp = fetchScriptByte();
	int state, y, x;

	switch (subOp) {
	case 62:
		state = pop();
		y = pop();
		x = pop();
		break;
	case 63:
		state = pop();
		if (state == 0)
			state = 1;
		y = x = -100;
		break;
	case 65:
		state = 1;
		y = pop();
		x = pop();
		break;
	default:
		error("o72_drawObject: default case %d", subOp);
	}

	int object = pop();
	int objnum = getObjectIndex(object);
	if (objnum == -1)
		return;

	if (y != -100 && x != -100) {
		_objs[objnum].x_pos = x * 8;
		_objs[objnum].y_pos = y * 8;
	}

	if (state != -1) {
		addObjectToDrawQue(objnum);
		putState(object, state);
	}
}

void ScummEngine_v72he::o72_printWizImage() {
	WizImage wi;
	wi.resNum = pop();
	wi.x1 = wi.y1 = 0;
	wi.state = 0;
	wi.flags = kWIFPrint;
	_wiz->displayWizImage(&wi);
}

void ScummEngine_v72he::o72_getArrayDimSize() {
	byte subOp = fetchScriptByte();
	int32 val1, val2;
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(fetchScriptWord()));
	if (!ah) {
		push(0);
		return;
	}

	switch (subOp) {
	case 1:
	case 3:
		val1 = FROM_LE_32(ah->dim1end);
		val2 = FROM_LE_32(ah->dim1start);
		push(val1 - val2 + 1);
		break;
	case 2:
		val1 = FROM_LE_32(ah->dim2end);
		val2 = FROM_LE_32(ah->dim2start);
		push(val1 - val2 + 1);
		break;
	case 4:
		push(FROM_LE_32(ah->dim1start));
		break;
	case 5:
		push(FROM_LE_32(ah->dim1end));
		break;
	case 6:
		push(FROM_LE_32(ah->dim2start));
		break;
	case 7:
		push(FROM_LE_32(ah->dim2end));
		break;
	default:
		error("o72_getArrayDimSize: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_getNumFreeArrays() {
	const ResourceManager::ResTypeData &rtd = _res->_types[rtString];
	int i, num = 0;

	for (i = 1; i < _numArray; i++) {
		if (!rtd[i]._address)
			num++;
	}

	push(num);
}

void ScummEngine_v72he::o72_roomOps() {
	int a, b, c, d, e;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 172:		// SO_ROOM_SCROLL
		b = pop();
		a = pop();
		if (a < (_screenWidth / 2))
			a = (_screenWidth / 2);
		if (b < (_screenWidth / 2))
			b = (_screenWidth / 2);
		if (a > _roomWidth - (_screenWidth / 2))
			a = _roomWidth - (_screenWidth / 2);
		if (b > _roomWidth - (_screenWidth / 2))
			b = _roomWidth - (_screenWidth / 2);
		VAR(VAR_CAMERA_MIN_X) = a;
		VAR(VAR_CAMERA_MAX_X) = b;
		break;

	case 174:		// SO_ROOM_SCREEN
		b = pop();
		a = pop();
		initScreens(a, _screenHeight);
		break;

	case 175:		// SO_ROOM_PALETTE
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		setPalColor(d, a, b, c);
		break;

	case 179:		// SO_ROOM_INTENSITY
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, a, a, b, c);
		break;

	case 180:		// SO_ROOM_SAVEGAME
		_saveTemporaryState = true;
		_saveLoadSlot = pop();
		_saveLoadFlag = pop();
		break;

	case 181:		// SO_ROOM_FADE
		// Defaults to 1 but doesn't use fade effects
		a = pop();
		break;

	case 182:		// SO_RGB_ROOM_INTENSITY
		e = pop();
		d = pop();
		c = pop();
		b = pop();
		a = pop();
		darkenPalette(a, b, c, d, e);
		break;

	case 213:		// SO_ROOM_NEW_PALETTE
		a = pop();
		setCurrentPalette(a);
		break;

	case 220:
		a = pop();
		b = pop();
		copyPalColor(a, b);
		break;

	case 221:
		byte buffer[256];

		copyScriptString((byte *)buffer, sizeof(buffer));

		_saveLoadFileName = (char *)buffer + convertFilePath(buffer, sizeof(buffer));
		debug(1, "o72_roomOps: case 221: filename %s", _saveLoadFileName.c_str());

		_saveLoadFlag = pop();
		_saveLoadSlot = 255;
		_saveTemporaryState = true;
		break;

	case 234:
		b = pop();
		a = pop();
		swapObjects(a, b);
		break;

	case 236:
		b = pop();
		a = pop();
		setRoomPalette(a, b);
		break;

	default:
		error("o72_roomOps: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_actorOps() {
	ActorHE *a;
	int i, j, k;
	int args[32];
	byte string[256];

	byte subOp = fetchScriptByte();
	if (subOp == 197) {
		_curActor = pop();
		return;
	}

	a = (ActorHE *)derefActorSafe(_curActor, "o72_actorOps");
	if (!a)
		return;

	switch (subOp) {
	case 21: // HE 80+
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; ++i) {
			a->setUserCondition(args[i] & 0x7F, args[i] & 0x80);
		}
		break;
	case 24: // HE 80+
		k = pop();
		if (k == 0)
			k = _rnd.getRandomNumberRng(1, 10);
		a->_heNoTalkAnimation = 1;
		a->setTalkCondition(k);
		break;
	case 43: // HE 90+
		a->_layer = pop();
		a->_needRedraw = true;
		break;
	case 64:
		_actorClipOverride.bottom = pop();
		_actorClipOverride.right = pop();
		_actorClipOverride.top = pop();
		_actorClipOverride.left = pop();
		adjustRect(_actorClipOverride);
		break;
	case 65: // HE 98+
		j = pop();
		i = pop();
		a->putActor(i, j);
		break;
	case 67: // HE 99+
		a->_clipOverride.bottom = pop();
		a->_clipOverride.right = pop();
		a->_clipOverride.top = pop();
		a->_clipOverride.left = pop();
		adjustRect(a->_clipOverride);
		break;
	case 68: // HE 90+
		k = pop();
		a->setHEFlag(1, k);
		break;
	case 76:		// SO_COSTUME
		a->setActorCostume(pop());
		break;
	case 77:		// SO_STEP_DIST
		j = pop();
		i = pop();
		a->setActorWalkSpeed(i, j);
		break;
	case 78:		// SO_SOUND
		k = getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < k; i++)
			a->_sound[i] = args[i];
		break;
	case 79:		// SO_WALK_ANIMATION
		a->_walkFrame = pop();
		break;
	case 80:		// SO_TALK_ANIMATION
		a->_talkStopFrame = pop();
		a->_talkStartFrame = pop();
		break;
	case 81:		// SO_STAND_ANIMATION
		a->_standFrame = pop();
		break;
	case 82:		// SO_ANIMATION
		// dummy case in scumm6
		pop();
		pop();
		pop();
		break;
	case 83:		// SO_DEFAULT
		a->initActor(0);
		break;
	case 84:		// SO_ELEVATION
		a->setElevation(pop());
		break;
	case 85:		// SO_ANIMATION_DEFAULT
		a->_initFrame = 1;
		a->_walkFrame = 2;
		a->_standFrame = 3;
		a->_talkStartFrame = 4;
		a->_talkStopFrame = 5;
		break;
	case 86:		// SO_PALETTE
		j = pop();
		i = pop();
		assertRange(0, i, 255, "palette slot");
		a->remapActorPaletteColor(i, j);
		a->_needRedraw = true;
		break;
	case 87:		// SO_TALK_COLOR
		a->_talkColor = pop();
		break;
	case 88:		// SO_ACTOR_NAME
		copyScriptString(string, sizeof(string));
		loadPtrToResource(rtActorName, a->_number, string);
		break;
	case 89:		// SO_INIT_ANIMATION
		a->_initFrame = pop();
		break;
	case 91:		// SO_ACTOR_WIDTH
		a->_width = pop();
		break;
	case 92:		// SO_SCALE
		i = pop();
		a->setScale(i, i);
		break;
	case 93:		// SO_NEVER_ZCLIP
		a->_forceClip = 0;
		break;
	case 94:		// SO_ALWAYS_ZCLIP
		a->_forceClip = pop();
		break;
	case 95:		// SO_IGNORE_BOXES
		a->_ignoreBoxes = 1;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case 96:		// SO_FOLLOW_BOXES
		a->_ignoreBoxes = 0;
		a->_forceClip = 0;
		if (a->isInCurrentRoom())
			a->putActor();
		break;
	case 97:		// SO_ANIMATION_SPEED
		a->setAnimSpeed(pop());
		break;
	case 98:		// SO_SHADOW
		a->_heXmapNum = pop();
		a->_needRedraw = true;
		break;
	case 99:		// SO_TEXT_OFFSET
		a->_talkPosY = pop();
		a->_talkPosX = pop();
		break;
	case 156:		// HE 72+
		a->_charset = pop();
		break;
	case 175:		// HE 99+
		a->_hePaletteNum = pop();
		a->_needRedraw = true;
		break;
	case 198:		// SO_ACTOR_VARIABLE
		i = pop();
		a->setAnimVar(pop(), i);
		break;
	case 215:		// SO_ACTOR_IGNORE_TURNS_ON
		a->_ignoreTurns = true;
		break;
	case 216:		// SO_ACTOR_IGNORE_TURNS_OFF
		a->_ignoreTurns = false;
		break;
	case 217:		// SO_ACTOR_NEW
		a->initActor(2);
		break;
	case 218:
		a->drawActorToBackBuf(a->getPos().x, a->getPos().y);
		break;
	case 219:
		a->_drawToBackBuf = false;
		a->_needRedraw = true;
		a->_needBgReset = true;
		break;
	case 225:
		{
		copyScriptString(string, sizeof(string));
		int slot = pop();

		int len = resStrLen(string) + 1;
		memcpy(a->_heTalkQueue[slot].sentence, string, len);

		a->_heTalkQueue[slot].posX = a->_talkPosX;
		a->_heTalkQueue[slot].posY = a->_talkPosY;
		a->_heTalkQueue[slot].color = a->_talkColor;
		break;
		}
	default:
		error("o72_actorOps: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_verbOps() {
	int slot, a, b;
	VerbSlot *vs;
	byte name[200];

	byte subOp = fetchScriptByte();
	if (subOp == 196) {
		_curVerb = pop();
		_curVerbSlot = getVerbSlot(_curVerb, 0);
		assertRange(0, _curVerbSlot, _numVerbs - 1, "new verb slot");
		return;
	}
	vs = &_verbs[_curVerbSlot];
	slot = _curVerbSlot;
	switch (subOp) {
	case 124:		// SO_VERB_IMAGE
		a = pop();
		if (_curVerbSlot) {
			setVerbObject(_roomResource, a, slot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case 125:		// SO_VERB_NAME
		copyScriptString(name, sizeof(name));
		loadPtrToResource(rtVerb, slot, name);
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case 126:		// SO_VERB_COLOR
		vs->color = pop();
		break;
	case 127:		// SO_VERB_HICOLOR
		vs->hicolor = pop();
		break;
	case 128:		// SO_VERB_AT
		vs->curRect.top = pop();
		vs->curRect.left = pop();
		break;
	case 129:		// SO_VERB_ON
		vs->curmode = 1;
		break;
	case 130:		// SO_VERB_OFF
		vs->curmode = 0;
		break;
	case 131:		// SO_VERB_DELETE
		slot = getVerbSlot(pop(), 0);
		killVerb(slot);
		break;
	case 132:		// SO_VERB_NEW
		slot = getVerbSlot(_curVerb, 0);
		if (slot == 0) {
			for (slot = 1; slot < _numVerbs; slot++) {
				if (_verbs[slot].verbid == 0)
					break;
			}
			if (slot == _numVerbs)
				error("Too many verbs");
			_curVerbSlot = slot;
		}
		vs = &_verbs[slot];
		vs->verbid = _curVerb;
		vs->color = 2;
		vs->hicolor = 0;
		vs->dimcolor = 8;
		vs->type = kTextVerbType;
		vs->charset_nr = _string[0]._default.charset;
		vs->curmode = 0;
		vs->saveid = 0;
		vs->key = 0;
		vs->center = 0;
		vs->imgindex = 0;
		break;
	case 133:		// SO_VERB_DIMCOLOR
		vs->dimcolor = pop();
		break;
	case 134:		// SO_VERB_DIM
		vs->curmode = 2;
		break;
	case 135:		// SO_VERB_KEY
		vs->key = pop();
		break;
	case 136:		// SO_VERB_CENTER
		vs->center = 1;
		break;
	case 137:		// SO_VERB_NAME_STR
		a = pop();
		if (a == 0) {
			loadPtrToResource(rtVerb, slot, (const byte *)"");
		} else {
			loadPtrToResource(rtVerb, slot, getStringAddress(a));
		}
		vs->type = kTextVerbType;
		vs->imgindex = 0;
		break;
	case 139:		// SO_VERB_IMAGE_IN_ROOM
		b = pop();
		a = pop();

		if (slot && a != vs->imgindex) {
			setVerbObject(b, a, slot);
			vs->type = kImageVerbType;
			vs->imgindex = a;
		}
		break;
	case 140:		// SO_VERB_BAKCOLOR
		vs->bkcolor = pop();
		break;
	case 255:
		drawVerb(slot, 0);
		verbMouseOver(0);
		break;
	default:
		error("o72_verbops: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_findObject() {
	int y = pop();
	int x = pop();
	int r = findObject(x, y, 0, 0);
	push(r);
}

void ScummEngine_v72he::o72_arrayOps() {
	byte *data;
	byte string[1024];
	int dim1end, dim1start, dim2end, dim2start;
	int id, len, b, c, list[128];
	int offs, tmp, tmp2;
	uint tmp3;

	byte subOp = fetchScriptByte();
	int array = fetchScriptWord();
	debug(9,"o72_arrayOps: array %d case %d", array, subOp);

	switch (subOp) {
	case 7:			// SO_ASSIGN_STRING
		copyScriptString(string, sizeof(string));
		len = resStrLen(string);
		data = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(data, string, len);
		break;

	case 126:
		len = getStackList(list, ARRAYSIZE(list));
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
		}
		checkArrayLimits(array, dim2start, dim2end, dim1start, dim1end);

		tmp2 = 0;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, list[tmp2++]);
				if (tmp2 == len)
					tmp2 = 0;
				tmp++;
			}
			dim2start++;
		}
		break;
	case 127:
		{
			int a2_dim1end = pop();
			int a2_dim1start = pop();
			int a2_dim2end = pop();
			int a2_dim2start = pop();
			int array2 = fetchScriptWord();
			int a1_dim1end = pop();
			int a1_dim1start = pop();
			int a1_dim2end = pop();
			int a1_dim2start = pop();
			if (a1_dim1end - a1_dim1start != a2_dim1end - a2_dim1start || a2_dim2end - a2_dim2start != a1_dim2end - a1_dim2start) {
				error("Source and dest ranges size are mismatched");
			}
			copyArray(array, a1_dim2start, a1_dim2end, a1_dim1start, a1_dim1end, array2, a2_dim2start, a2_dim2end, a2_dim1start, a2_dim1end);
		}
		break;
	case 128:
		b = pop();
		c = pop();
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, dim2start, dim2end, dim1start, dim1end);
		}
		checkArrayLimits(array, dim2start, dim2end, dim1start, dim1end);

		offs = (b >= c) ? 1 : -1;
		tmp2 = c;
		tmp3 = c - b + 1;
		while (dim2start <= dim2end) {
			tmp = dim1start;
			while (tmp <= dim1end) {
				writeArray(array, dim2start, tmp, tmp2);
				if (--tmp3 == 0) {
					tmp2 = c;
					tmp3 = c - b + 1;
				} else {
					tmp2 += offs;
				}
				tmp++;
			}
			dim2start++;
		}
		break;
	case 194:
		decodeScriptString(string);
		len = resStrLen(string);
		data = defineArray(array, kStringArray, 0, 0, 0, len);
		memcpy(data, string, len);
		break;
	case 208:		// SO_ASSIGN_INT_LIST
		b = pop();
		c = pop();
		id = readVar(array);
		if (id == 0) {
			defineArray(array, kDwordArray, 0, 0, 0, b + c - 1);
		}
		while (c--) {
			writeArray(array, 0, b + c, pop());
		}
		break;
	case 212:		// SO_ASSIGN_2DIM_LIST
		len = getStackList(list, ARRAYSIZE(list));
		id = readVar(array);
		if (id == 0)
			error("Must DIM a two dimensional array before assigning");
		c = pop();
		while (--len >= 0) {
			writeArray(array, c, len, list[len]);
		}
		break;
	default:
		error("o72_arrayOps: default case %d (array %d)", subOp, array);
	}
}

void ScummEngine_v72he::o72_systemOps() {
	byte string[1024];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 22: // HE80+
		clearDrawObjectQueue();
		break;
	case 26: // HE80+
		restoreBackgroundHE(Common::Rect(_screenWidth, _screenHeight));
		updatePalette();
		break;
	case 158:
		restart();
		break;
	case 160:
		// Confirm shutdown
		quitGame();
		break;
	case 244:
		quitGame();
		break;
	case 251:
		copyScriptString(string, sizeof(string));
		debug(0, "Start executable (%s)", string);
		break;
	case 252:
		copyScriptString(string, sizeof(string));
		debug(0, "Start game (%s)", string);
		break;
	default:
		error("o72_systemOps invalid case %d", subOp);
	}
}

void ScummEngine_v72he::o72_talkActor() {
	Actor *a;

	int act = pop();

	_string[0].loadDefault();

	// A value of 225 can occur when examining the gold in the mine of pajama, after mining the gold.
	// This is a script bug, the script should set the subtitle color, not actor number.
	// This script bug was fixed in the updated version of pajama.
	if (act == 225) {
		_string[0].color = act;
	} else {
		_actorToPrintStrFor = act;
		if (_actorToPrintStrFor != 0xFF) {
			a = derefActor(_actorToPrintStrFor, "o72_talkActor");
			_string[0].color = a->_talkColor;
		}
	}

	actorTalk(_scriptPointer);

	_scriptPointer += resStrLen(_scriptPointer) + 1;
}

void ScummEngine_v72he::o72_talkEgo() {
	push(VAR(VAR_EGO));
	o72_talkActor();
}

void ScummEngine_v72he::o72_dimArray() {
	int data;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 2:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 3:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 4:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 5:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 6:
		data = kDwordArray;
		break;
	case 7:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	case 204:		// SO_UNDIM_ARRAY
		nukeArray(fetchScriptWord());
		return;
	default:
		error("o72_dimArray: default case %d", subOp);
	}

	defineArray(fetchScriptWord(), data, 0, 0, 0, pop());
}


void ScummEngine_v72he::o72_dim2dimArray() {
	int data, dim1end, dim2end;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 2:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 3:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 4:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 5:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 6:
		data = kDwordArray;
		break;
	case 7:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	default:
		error("o72_dim2dimArray: default case %d", subOp);
	}

	dim1end = pop();
	dim2end = pop();
	defineArray(fetchScriptWord(), data, 0, dim2end, 0, dim1end);
}

void ScummEngine_v72he::o72_traceStatus() {
	byte string[80];

	copyScriptString(string, sizeof(string));
	pop();
}

void ScummEngine_v72he::o72_kernelGetFunctions() {
	int args[29];
	byte *data;
	getStackList(args, ARRAYSIZE(args));

	switch (args[0]) {
	case 1:
		writeVar(0, 0);
		data = defineArray(0, kByteArray, 0, 0, 0, virtScreenSave(0, args[1], args[2], args[3], args[4]));
		virtScreenSave(data, args[1], args[2], args[3], args[4]);
		push(readVar(0));
		break;
	default:
		error("o72_kernelGetFunctions: default case %d", args[0]);
	}
}

void ScummEngine_v72he::o72_drawWizImage() {
	WizImage wi;
	wi.flags = pop();
	wi.y1 = pop();
	wi.x1 = pop();
	wi.resNum = pop();
	wi.state = 0;
	_wiz->displayWizImage(&wi);
}

void ScummEngine_v72he::debugInput(byte* string) {
	byte *debugInputString;

	DebugInputDialog dialog(this, (char *)string);
	runDialog(dialog);
	while (!dialog.done) {
		parseEvents();
		dialog.handleKeyDown(_keyPressed);
	}

	writeVar(0, 0);
	debugInputString = defineArray(0, kStringArray, 0, 0, 0, dialog.buffer.size());
	memcpy(debugInputString, dialog.buffer.c_str(), dialog.buffer.size());
	push(readVar(0));
}

void ScummEngine_v72he::o72_debugInput() {
	byte string[255];

	copyScriptString(string, sizeof(string));
	debugInput(string);
}

void ScummEngine_v72he::o72_jumpToScript() {
	int args[25];
	int script;
	byte flags;

	getStackList(args, ARRAYSIZE(args));
	script = pop();
	flags = fetchScriptByte();
	stopObjectCode();
	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v72he::o72_openFile() {
	int mode, slot, i;
	byte buffer[256];

	mode = pop();
	copyScriptString(buffer, sizeof(buffer));
	debug(1, "Original filename %s", buffer);

	const char *filename = (char *)buffer + convertFilePath(buffer, sizeof(buffer));
	debug(1, "Final filename to %s", filename);

	slot = -1;
	for (i = 1; i < 17; i++) {
		if (_hInFileTable[i] == 0 && _hOutFileTable[i] == 0) {
			slot = i;
			break;
		}
	}

	if (slot != -1) {
		switch (mode) {
		case 1:   // Read mode
			if (!_saveFileMan->listSavefiles(filename).empty()) {
				_hInFileTable[slot] = _saveFileMan->openForLoading(filename);
			} else {
				_hInFileTable[slot] = SearchMan.createReadStreamForMember(filename);
			}
			break;
		case 2:   // Write mode
			if (!strchr(filename, '/')) {
				_hOutFileTable[slot] = _saveFileMan->openForSaving(filename);
			}
			break;
		case 6: { // Append mode
			if (strchr(filename, '/'))
				break;

			// First check if the file already exists
			Common::InSaveFile *initialState = 0;
			if (!_saveFileMan->listSavefiles(filename).empty())
				initialState = _saveFileMan->openForLoading(filename);
			else
				initialState = SearchMan.createReadStreamForMember(filename);

			// Read in the data from the initial file
			uint32 initialSize = 0;
			byte *initialData = 0;
			if (initialState) {
				initialSize = initialState->size();
				initialData = new byte[initialSize];
				initialState->read(initialData, initialSize);
				delete initialState;
			}

			// Attempt to open a save file
			_hOutFileTable[slot] = _saveFileMan->openForSaving(filename);

			// Begin us off with the data from the previous file
			if (_hOutFileTable[slot] && initialData) {
				_hOutFileTable[slot]->write(initialData, initialSize);
				delete[] initialData;
			}

			} break;
		default:
			error("o72_openFile(): wrong open file mode %d", mode);
		}

		if (_hInFileTable[slot] == 0 && _hOutFileTable[slot] == 0)
			slot = -1;

	}
	debug(1, "o72_openFile: slot %d, mode %d", slot, mode);
	push(slot);
}

int ScummEngine_v72he::readFileToArray(int slot, int32 size) {
	writeVar(0, 0);
	byte *data = defineArray(0, kByteArray, 0, 0, 0, size);

	if (slot != -1) {
		_hInFileTable[slot]->read(data, size + 1);
	}

	return readVar(0);
}

void ScummEngine_v72he::o72_readFile() {
	int slot, val;
	int32 size;
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 4:
		slot = pop();
		assert(_hInFileTable[slot]);
		val = _hInFileTable[slot]->readByte();
		push(val);
		break;
	case 5:
		slot = pop();
		assert(_hInFileTable[slot]);
		val = _hInFileTable[slot]->readUint16LE();
		push(val);
		break;
	case 6:
		slot = pop();
		assert(_hInFileTable[slot]);
		val = _hInFileTable[slot]->readUint32LE();
		push(val);
		break;
	case 8:
		fetchScriptByte();
		size = pop();
		slot = pop();
		assert(_hInFileTable[slot]);
		val = readFileToArray(slot, size);
		push(val);
		break;
	default:
		error("o72_readFile: default case %d", subOp);
	}
}

void ScummEngine_v72he::writeFileFromArray(int slot, int32 resID) {
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, resID);
	int32 size = (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(FROM_LE_32(ah->dim2end) - FROM_LE_32(ah->dim2start) + 1);

	if (slot != -1) {
		_hOutFileTable[slot]->write(ah->data, size);
	}
}

void ScummEngine_v72he::o72_writeFile() {
	int32 resID = pop();
	int slot = pop();
	byte subOp = fetchScriptByte();

	assert(_hOutFileTable[slot]);
	switch (subOp) {
	case 4:
		_hOutFileTable[slot]->writeByte(resID);
		break;
	case 5:
		_hOutFileTable[slot]->writeUint16LE(resID);
		break;
	case 6:
		_hOutFileTable[slot]->writeUint32LE(resID);
		break;
	case 8:
		fetchScriptByte();
		writeFileFromArray(slot, resID);
		break;
	default:
		error("o72_writeFile: default case %d", subOp);
	}
}

void ScummEngine_v72he::o72_findAllObjects() {
	int room = pop();
	int i;

	if (room != _currentRoom)
		error("o72_findAllObjects: current room is not %d", room);

	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, _numLocalObjects);
	writeArray(0, 0, 0, _numLocalObjects);

	for (i = 1; i < _numLocalObjects; i++) {
		writeArray(0, 0, i, _objs[i].obj_nr);
	}

	push(readVar(0));
}

void ScummEngine_v72he::o72_deleteFile() {
	byte buffer[256];

	copyScriptString(buffer, sizeof(buffer));
	const char *filename = (char *)buffer + convertFilePath(buffer, sizeof(buffer));

	debug(1, "o72_deleteFile(%s)", filename);

	if (!_saveFileMan->listSavefiles(filename).empty()) {
		_saveFileMan->removeSavefile(filename);
	}
}

void ScummEngine_v72he::o72_rename() {
	byte buffer1[100],buffer2[100];

	copyScriptString(buffer1, sizeof(buffer1));
	copyScriptString(buffer2, sizeof(buffer2));

	const char *newFilename = (char *)buffer1 + convertFilePath(buffer1, sizeof(buffer1));
	const char *oldFilename = (char *)buffer2 + convertFilePath(buffer2, sizeof(buffer2));

	_saveFileMan->renameSavefile(oldFilename, newFilename);

	debug(1, "o72_rename(%s to %s)", oldFilename, newFilename);
}

void ScummEngine_v72he::o72_getPixel() {
	uint16 area;

	int y = pop();
	int x = pop();
	byte subOp = fetchScriptByte();

	VirtScreen *vs = findVirtScreen(y);
	if (vs == NULL || x > _screenWidth - 1 || x < 0) {
		push(-1);
		return;
	}

	switch (subOp) {
	case 9: // HE 100
	case 218:
		if (_game.features & GF_16BIT_COLOR)
			area = READ_UINT16(vs->getBackPixels(x, y - vs->topline));
		else
			area = *vs->getBackPixels(x, y - vs->topline);
		break;
	case 8: // HE 100
	case 219:
		if (_game.features & GF_16BIT_COLOR)
			area = READ_UINT16(vs->getPixels(x, y - vs->topline));
		else
			area = *vs->getPixels(x, y - vs->topline);
		break;
	default:
		error("o72_getPixel: default case %d", subOp);
	}
	push(area);
}

void ScummEngine_v72he::o72_pickVarRandom() {
	int num;
	int args[100];
	int32 dim1end;

	num = getStackList(args, ARRAYSIZE(args));
	int value = fetchScriptWord();

	if (readVar(value) == 0) {
		defineArray(value, kDwordArray, 0, 0, 0, num);
		if (num > 0) {
			int16 counter = 0;
			do {
				writeArray(value, 0, counter + 1, args[counter]);
			} while (++counter < num);
		}

		shuffleArray(value, 1, num);
		writeArray(value, 0, 0, 2);
		push(readArray(value, 0, 1));
		return;
	}

	num = readArray(value, 0, 0);

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(value));
	dim1end = FROM_LE_32(ah->dim1end);

	if (dim1end < num) {
		int32 var_2 = readArray(value, 0, num - 1);
		shuffleArray(value, 1, dim1end);
		if (readArray(value, 0, 1) == var_2) {
			num = 2;
		} else {
			num = 1;
		}
	}

	writeArray(value, 0, 0, num + 1);
	push(readArray(value, 0, num));
}

void ScummEngine_v72he::o72_redimArray() {
	int newX, newY;
	newY = pop();
	newX = pop();

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 4:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kByteArray);
		break;
	case 5:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kIntArray);
		break;
	case 6:
		redimArray(fetchScriptWord(), 0, newX, 0, newY, kDwordArray);
		break;
	default:
		error("o72_redimArray: default type %d", subOp);
	}
}

void ScummEngine_v72he::redimArray(int arrayId, int newDim2start, int newDim2end,
								   int newDim1start, int newDim1end, int type) {
	int newSize, oldSize;

	if (readVar(arrayId) == 0)
		error("redimArray: Reference to zeroed array pointer");

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(arrayId));

	if (!ah)
		error("redimArray: Invalid array (%d) reference", readVar(arrayId));

	newSize = arrayDataSizes[type];
	oldSize = arrayDataSizes[FROM_LE_32(ah->type)];

	newSize *= (newDim1end - newDim1start + 1) * (newDim2end - newDim2start + 1);
	oldSize *= (FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1) *
		(FROM_LE_32(ah->dim2end) - FROM_LE_32(ah->dim2start) + 1);

	newSize >>= 3;
	oldSize >>= 3;

	if (newSize != oldSize)
		error("redimArray: array %d redim mismatch", readVar(arrayId));

	ah->type = TO_LE_32(type);
	ah->dim1start = TO_LE_32(newDim1start);
	ah->dim1end = TO_LE_32(newDim1end);
	ah->dim2start = TO_LE_32(newDim2start);
	ah->dim2end = TO_LE_32(newDim2end);
}

void ScummEngine_v72he::checkArrayLimits(int array, int dim2start, int dim2end, int dim1start, int dim1end) {
	if (dim1end < dim1start) {
		error("Across max %d smaller than min %d", dim1end, dim1start);
	}
	if (dim2end < dim2start) {
		error("Down max %d smaller than min %d", dim2end, dim2start);
	}
	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array));
	assert(ah);
	if ((int)FROM_LE_32(ah->dim2start) > dim2start || (int)FROM_LE_32(ah->dim2end) < dim2end || (int)FROM_LE_32(ah->dim1start) > dim1start || (int)FROM_LE_32(ah->dim1end) < dim1end) {
		error("Invalid array access (%d,%d,%d,%d) limit (%d,%d,%d,%d)", dim2start, dim2end, dim1start, dim1end, FROM_LE_32(ah->dim2start), FROM_LE_32(ah->dim2end), FROM_LE_32(ah->dim1start), FROM_LE_32(ah->dim1end));
	}
}

void ScummEngine_v72he::copyArray(int array1, int a1_dim2start, int a1_dim2end, int a1_dim1start, int a1_dim1end,
				int array2, int a2_dim2start, int a2_dim2end, int a2_dim1start, int a2_dim1end)
{
	byte *dst, *src;
	int dstPitch, srcPitch;
	int rowSize;
	checkArrayLimits(array1, a1_dim2start, a1_dim2end, a1_dim1start, a1_dim1end);
	checkArrayLimits(array2, a2_dim2start, a2_dim2end, a2_dim1start, a2_dim1end);
	int a12_num = a1_dim2end - a1_dim2start + 1;
	int a11_num = a1_dim1end - a1_dim1start + 1;
	int a22_num = a2_dim2end - a2_dim2start + 1;
	int a21_num = a2_dim1end - a2_dim1start + 1;
	if (a22_num != a12_num || a21_num != a11_num) {
		error("Operation size mismatch (%d vs %d)(%d vs %d)", a12_num, a22_num, a11_num, a21_num);
	}

	if (array1 != array2) {
		ArrayHeader *ah1 = (ArrayHeader *)getResourceAddress(rtString, readVar(array1));
		assert(ah1);
		ArrayHeader *ah2 = (ArrayHeader *)getResourceAddress(rtString, readVar(array2));
		assert(ah2);
		if (FROM_LE_32(ah1->type) == FROM_LE_32(ah2->type)) {
			copyArrayHelper(ah1, a1_dim2start, a1_dim1start, a1_dim1end, &dst, &dstPitch, &rowSize);
			copyArrayHelper(ah2, a2_dim2start, a2_dim1start, a2_dim1end, &src, &srcPitch, &rowSize);
			for (; a1_dim2start <= a1_dim2end; ++a1_dim2start) {
				memcpy(dst, src, rowSize);
				dst += dstPitch;
				src += srcPitch;
			}
		} else {
			for (; a1_dim2start <= a1_dim2end; ++a1_dim2start, ++a2_dim2start) {
				int a2dim1 = a2_dim1start;
				int a1dim1 = a1_dim1start;
				for (; a1dim1 <= a1_dim1end; ++a1dim1, ++a2dim1) {
					int val = readArray(array2, a2_dim2start, a2dim1);
					writeArray(array1, a1_dim2start, a1dim1, val);
				}
			}
		}
	} else {
		if (a2_dim2start != a1_dim2start || a2_dim1start != a1_dim1start) {
			ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(array1));
			assert(ah);
			if (a2_dim2start > a1_dim2start) {
				copyArrayHelper(ah, a1_dim2start, a1_dim1start, a1_dim1end, &dst, &dstPitch, &rowSize);
				copyArrayHelper(ah, a2_dim2start, a2_dim1start, a2_dim1end, &src, &srcPitch, &rowSize);
			} else {
				copyArrayHelper(ah, a1_dim2end, a1_dim1start, a1_dim1end, &dst, &dstPitch, &rowSize);
				copyArrayHelper(ah, a2_dim2end, a2_dim1start, a2_dim1end, &src, &srcPitch, &rowSize);
			}
			for (; a1_dim2start <= a1_dim2end; ++a1_dim2start) {
				memcpy(dst, src, rowSize);
				dst += dstPitch;
				src += srcPitch;
			}
		}
	}
}

void ScummEngine_v72he::copyArrayHelper(ArrayHeader *ah, int idx2, int idx1, int len1, byte **data, int *size, int *num) {
	const int pitch = FROM_LE_32(ah->dim1end) - FROM_LE_32(ah->dim1start) + 1;
	const int offset = pitch * (idx2 - FROM_LE_32(ah->dim2start)) + idx1 - FROM_LE_32(ah->dim1start);

	switch (FROM_LE_32(ah->type)) {
	case kByteArray:
	case kStringArray:
		*num = len1 - idx1 + 1;
		*size = pitch;
		*data = ah->data + offset;
		break;
	case kIntArray:
		*num = (len1 - idx1) * 2 + 2;
		*size = pitch * 2;
		*data = ah->data + offset * 2;
		break;
	case kDwordArray:
		*num = (len1 - idx1) * 4 + 4;
		*size = pitch * 4;
		*data = ah->data + offset * 4;
		break;
	default:
		error("Invalid array type %d", FROM_LE_32(ah->type));
	}
}

void ScummEngine_v72he::o72_readINI() {
	byte option[128];
	byte *data;

	copyScriptString(option, sizeof(option));
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 43: // HE 100
	case 6: // number
		if (!strcmp((char *)option, "DisablePrinting") || !strcmp((char *)option, "NoPrinting")) {
			push(1);
		} else if (!strcmp((char *)option, "TextOn")) {
			push(ConfMan.getBool("subtitles"));
		} else if (!strcmp((char *)option, "Disk") && (_game.id == GID_BIRTHDAYRED || _game.id == GID_BIRTHDAYYELLOW)) {
			// WORKAROUND: Override the disk detection
			// This removes the reliance on having the binary file around (which is
			// very bad for the Mac version) just for the scripts to figure out if
			// we're running Yellow or Red
			if (_game.id == GID_BIRTHDAYRED)
				push(4);
			else
				push(2);
		} else {
			push(ConfMan.getInt((char *)option));
		}
		break;
	case 77: // HE 100
	case 7: // string
		writeVar(0, 0);
		if (!strcmp((char *)option, "HE3File")) {
			Common::String fileName = generateFilename(-3);
			int len = resStrLen((const byte *)fileName.c_str());
			data = defineArray(0, kStringArray, 0, 0, 0, len);
			memcpy(data, fileName.c_str(), len);
		} else if (!strcmp((char *)option, "GameResourcePath") || !strcmp((char *)option, "SaveGamePath")) {
			// We set SaveGamePath in order to detect where it used
			// in convertFilePath and to avoid warning about invalid
			// path in Macintosh verisons.
			data = defineArray(0, kStringArray, 0, 0, 0, 2);
			if (_game.platform == Common::kPlatformMacintosh)
				memcpy(data, (const char *)"*:", 2);
			else
				memcpy(data, (const char *)"*\\", 2);
		} else {
			const char *entry = (ConfMan.get((char *)option).c_str());
			int len = resStrLen((const byte *)entry);
			data = defineArray(0, kStringArray, 0, 0, 0, len);
			memcpy(data, entry, len);
		}
		push(readVar(0));
		break;
	default:
		error("o72_readINI: default type %d", subOp);
	}

	debug(1, "o72_readINI: Option %s", option);
}

void ScummEngine_v72he::o72_writeINI() {
	int value;
	byte option[256], string[1024];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 43: // HE 100
	case 6: // number
		value = pop();
		copyScriptString(option, sizeof(option));
		debug(1, "o72_writeINI: Option %s Value %d", option, value);

		ConfMan.setInt((char *)option, value);
		break;
	case 77: // HE 100
	case 7: // string
		copyScriptString(string, sizeof(string));
		copyScriptString(option, sizeof(option));
		debug(1, "o72_writeINI: Option %s String %s", option, string);

		// Filter out useless setting
		if (!strcmp((char *)option, "HETest"))
			return;

		// Filter out confusing subtitle setting
		if (!strcmp((char *)option, "TextOn"))
			return;

		// Filter out confusing path settings
		if (!strcmp((char *)option, "DownLoadPath") || !strcmp((char *)option, "GameResourcePath") || !strcmp((char *)option, "SaveGamePath"))
			return;

		ConfMan.set((char *)option, (char *)string);
		break;
	default:
		error("o72_writeINI: default type %d", subOp);
	}

	ConfMan.flushToDisk();
}

void ScummEngine_v72he::o72_getResourceSize() {
	const byte *ptr;
	int size;
	ResType type;

	int resid = pop();
	if (_game.heversion == 72) {
		push(getSoundResourceSize(resid));
		return;
	}

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 13:
		push(getSoundResourceSize(resid));
		return;
	case 14:
		type = rtRoomImage;
		break;
	case 15:
		type = rtImage;
		break;
	case 16:
		type = rtCostume;
		break;
	case 17:
		type = rtScript;
		break;
	default:
		error("o72_getResourceSize: default type %d", subOp);
	}

	ptr = getResourceAddress(type, resid);
	assert(ptr);
	size = READ_BE_UINT32(ptr + 4) - 8;
	push(size);
}

void ScummEngine_v72he::o72_createDirectory() {
	byte directoryName[255];

	copyScriptString(directoryName, sizeof(directoryName));
	debug(1,"o72_createDirectory: %s", directoryName);
}

void ScummEngine_v72he::o72_setSystemMessage() {
	byte name[1024];

	copyScriptString(name, sizeof(name));
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 240:
		debug(1,"o72_setSystemMessage: (%d) %s", subOp, name);
		break;
	case 241: // Set Version
		debug(1,"o72_setSystemMessage: (%d) %s", subOp, name);
		break;
	case 242:
		debug(1,"o72_setSystemMessage: (%d) %s", subOp, name);
		break;
	case 243: // Set Window Caption
		// TODO: The 'name' string can contain non-ASCII data. This can lead to
		// problems, because (a) the encoding used for "name" is not clear,
		// (b) OSystem::setWindowCaption only supports ASCII. As a result, odd
		// behavior can occur, from strange wrong titles, up to crashes (happens
		// under Mac OS X).
		//
		// Possible fixes/workarounds:
		// - Simply stop using this. It's a rather unimportant "feature" anyway.
		// - Try to translate the text to ASCII.
		// - Refine OSystem to accept window captions that are non-ASCII, e.g.
		//   by enhancing all backends to deal with UTF-8 data. Of course, then
		//   one still would have to convert 'name' to the correct encoding.
		//_system->setWindowCaption((const char *)name);
		break;
	default:
		error("o72_setSystemMessage: default case %d", subOp);
	}
}

void ScummEngine_v72he::decodeParseString(int m, int n) {
	Actor *a;
	int i, colors, size;
	int args[31];
	byte name[1024];

	byte b = fetchScriptByte();

	switch (b) {
	case 65:		// SO_AT
		_string[m].ypos = pop();
		_string[m].xpos = pop();
		_string[m].overhead = false;
		break;
	case 66:		// SO_COLOR
		_string[m].color = pop();
		break;
	case 67:		// SO_CLIPPED
		_string[m].right = pop();
		break;
	case 69:		// SO_CENTER
		_string[m].center = true;
		_string[m].overhead = false;
		break;
	case 71:		// SO_LEFT
		_string[m].center = false;
		_string[m].overhead = false;
		break;
	case 72:		// SO_OVERHEAD
		_string[m].overhead = true;
		_string[m].no_talk_anim = false;
		break;
	case 74:		// SO_MUMBLE
		_string[m].no_talk_anim = true;
		break;
	case 75:		// SO_TEXTSTRING
		printString(m, _scriptPointer);
		_scriptPointer += resStrLen(_scriptPointer) + 1;
		break;
	case 194:
		decodeScriptString(name, true);
		printString(m, name);
		break;
	case 0xE1:
		{
		byte *dataPtr = getResourceAddress(rtTalkie, pop());
		byte *text = findWrappedBlock(MKTAG('T','E','X','T'), dataPtr, 0, 0);
		size = getResourceDataSize(text);
		memcpy(name, text, size);
		printString(m, name);
		}
		break;
	case 0xF9:
		colors = pop();
		if (colors == 1) {
			_string[m].color = pop();
		} else {
			push(colors);
			getStackList(args, ARRAYSIZE(args));
			for (i = 0; i < 16; i++)
				_charsetColorMap[i] = _charsetData[_string[m]._default.charset][i] = (unsigned char)args[i];
			_string[m].color = _charsetColorMap[0];
		}
		break;
	case 0xFE:
		_string[m].loadDefault();
		if (n) {
			_actorToPrintStrFor = pop();
			if (_actorToPrintStrFor != 0xFF) {
				a = derefActor(_actorToPrintStrFor, "decodeParseString");
				_string[m].color = a->_talkColor;
			}
		}
		break;
	case 0xFF:
		_string[m].saveDefault();
		break;
	default:
		error("decodeParseString: default case 0x%x", b);
	}
}

} // End of namespace Scumm

#endif // ENABLE_HE
