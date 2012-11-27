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

#include "common/archive.h"
#include "common/config-file.h"
#include "common/config-manager.h"
#include "common/macresman.h"
#include "common/savefile.h"
#include "common/str.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/he/sound_he.h"

namespace Scumm {

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v80he, x)

void ScummEngine_v80he::setupOpcodes() {
	ScummEngine_v72he::setupOpcodes();

	OPCODE(0x45, o80_createSound);
	OPCODE(0x46, o80_getFileSize);
	OPCODE(0x48, o80_stringToInt);
	OPCODE(0x49, o80_getSoundVar);
	OPCODE(0x4a, o80_localizeArrayToRoom);
	OPCODE(0x4c, o80_sourceDebug);
	OPCODE(0x4d, o80_readConfigFile);
	OPCODE(0x4e, o80_writeConfigFile);
	_opcodes[0x69].setProc(0, 0);
	OPCODE(0x6b, o80_cursorCommand);
	OPCODE(0x70, o80_setState);
	_opcodes[0x76].setProc(0, 0);
	_opcodes[0x94].setProc(0, 0);
	_opcodes[0x9e].setProc(0, 0);
	_opcodes[0xa5].setProc(0, 0);
	OPCODE(0xac, o80_drawWizPolygon);
	OPCODE(0xe0, o80_drawLine);
	OPCODE(0xe3, o80_pickVarRandom);
}

void ScummEngine_v80he::o80_createSound() {
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 27:
		createSound(_heSndResId, pop());
		break;
	case 217:
		createSound(_heSndResId, -1);
		break;
	case 232:
		_heSndResId = pop();
		break;
	case 255:
		// dummy case
		break;
	default:
		error("o80_createSound: default case %d", subOp);
	}
}

void ScummEngine_v80he::o80_getFileSize() {
	byte buffer[256];

	copyScriptString(buffer, sizeof(buffer));
	const char *filename = (char *)buffer + convertFilePath(buffer, sizeof(buffer));

	Common::SeekableReadStream *f = 0;
	if (!_saveFileMan->listSavefiles(filename).empty()) {
		f = _saveFileMan->openForLoading(filename);
	} else {
		f = SearchMan.createReadStreamForMember(filename);
	}

	if (!f) {
		push(-1);
	} else {
		push(f->size());
		delete f;
	}
}

void ScummEngine_v80he::o80_stringToInt() {
	int id, len, val;
	byte *addr;
	char string[100];

	id = pop();

	addr = getStringAddress(id);
	if (!addr)
		error("o80_stringToInt: Reference to zeroed array pointer (%d)", id);

	len = resStrLen(getStringAddress(id)) + 1;
	memcpy(string, addr, len);
	val = atoi(string);
	push(val);
}

void ScummEngine_v80he::o80_getSoundVar() {
	int var = pop();
	int snd = pop();
	push(((SoundHE *)_sound)->getSoundVar(snd, var));
}

void ScummEngine_v80he::o80_localizeArrayToRoom() {
	int slot = pop();
	localizeArray(slot, 0xFF);
}

void ScummEngine_v80he::o80_sourceDebug() {
	fetchScriptDWord();
	fetchScriptDWord();
}

void ScummEngine_v80he::o80_readConfigFile() {
	byte option[128], section[128], filename[256];
	byte *data;
	Common::String entry;
	int len, r;

	copyScriptString(option, sizeof(option));
	copyScriptString(section, sizeof(section));
	copyScriptString(filename, sizeof(filename));

	r = convertFilePath(filename, sizeof(filename));

	if (_game.id == GID_TREASUREHUNT) {
		// WORKAROUND: Remove invalid characters
		if (!strcmp((char *)section, "Blue'sTreasureHunt-Disc1"))
			memcpy(section, "BluesTreasureHunt-Disc1\0", 24);
		else if (!strcmp((char *)section, "Blue'sTreasureHunt-Disc2"))
			memcpy(section, "BluesTreasureHunt-Disc2\0", 24);
	}

	if (!strcmp((const char *)filename, "map (i)")) {
		// Mac resource fork config file
		// (as used by only mustard mac for map data?)
		Common::MacResManager resFork;

		if (!resFork.open((const char *)filename) || !resFork.hasResFork())
			error("Could not open '%s'", filename);

		Common::String prefResName = Common::String::format("Pref:%s.%s", (const char *)section, (const char *)option);
		Common::SeekableReadStream *res = resFork.getResource(prefResName);

		if (res) {
			// The string is inside the resource as a pascal string
			byte length = res->readByte();
			for (byte i = 0; i < length; i++)
				entry += (char)res->readByte();

			delete res;
		}
	} else {
		// Normal Windows INI files
		Common::ConfigFile confFile;
		if (!strcmp((char *)filename + r, "map.ini"))
			confFile.loadFromFile((const char *)filename + r);
		else
			confFile.loadFromSaveFile((const char *)filename + r);

		confFile.getKey((const char *)option, (const char *)section, entry);
	}

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 43: // HE 100
	case 6: // number
		if (!strcmp((char *)option, "Benchmark"))
			push(2);
		else
			push(atoi(entry.c_str()));
		break;
	case 77: // HE 100
	case 7: // string
		writeVar(0, 0);
		len = resStrLen((const byte *)entry.c_str());
		data = defineArray(0, kStringArray, 0, 0, 0, len);
		memcpy(data, entry.c_str(), len);
		push(readVar(0));
		break;
	default:
		error("o80_readConfigFile: default type %d", subOp);
	}

	debug(1, "o80_readConfigFile: Filename %s Section %s Option %s Value %s", filename, section, option, entry.c_str());
}

void ScummEngine_v80he::o80_writeConfigFile() {
	byte filename[256], section[256], option[256], string[1024];
	int r, value;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 43: // HE 100
	case 6: // number
		value = pop();
		sprintf((char *)string, "%d", value);
		copyScriptString(option, sizeof(option));
		copyScriptString(section, sizeof(section));
		copyScriptString(filename, sizeof(filename));
		break;
	case 77: // HE 100
	case 7: // string
		copyScriptString(string, sizeof(string));
		copyScriptString(option, sizeof(option));
		copyScriptString(section, sizeof(section));
		copyScriptString(filename, sizeof(filename));
		break;
	default:
		error("o80_writeConfigFile: default type %d", subOp);
	}

	r = convertFilePath(filename, sizeof(filename));

	if (_game.id == GID_TREASUREHUNT) {
		// WORKAROUND: Remove invalid characters
		if (!strcmp((char *)section, "Blue'sTreasureHunt-Disc1"))
			memcpy(section, "BluesTreasureHunt-Disc1\0", 24);
		else if (!strcmp((char *)section, "Blue'sTreasureHunt-Disc2"))
			memcpy(section, "BluesTreasureHunt-Disc2\0", 24);
	}

	Common::ConfigFile ConfFile;
	ConfFile.loadFromSaveFile((const char *)filename + r);
	ConfFile.setKey((char *)option, (char *)section, (char *)string);
	ConfFile.saveToSaveFile((const char *)filename + r);

	debug(1,"o80_writeConfigFile: Filename %s Section %s Option %s String %s", filename, section, option, string);
}

void ScummEngine_v80he::o80_cursorCommand() {
	int a, b, i;
	int args[16];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0x13:
	case 0x14:
		a = pop();
		_wiz->loadWizCursor(a, 0);
		break;
	case 0x3C:
		b = pop();
		a = pop();
		_wiz->loadWizCursor(a, b);
		break;
	case 0x90:		// SO_CURSOR_ON Turn cursor on
		_cursor.state = 1;
		break;
	case 0x91:		// SO_CURSOR_OFF Turn cursor off
		_cursor.state = 0;
		break;
	case 0x92:		// SO_USERPUT_ON
		_userPut = 1;
		break;
	case 0x93:		// SO_USERPUT_OFF
		_userPut = 0;
		break;
	case 0x94:		// SO_CURSOR_SOFT_ON Turn soft cursor on
		_cursor.state++;
		if (_cursor.state > 1)
			error("Cursor state greater than 1 in script");
		break;
	case 0x95:		// SO_CURSOR_SOFT_OFF Turn soft cursor off
		_cursor.state--;
		break;
	case 0x96:		// SO_USERPUT_SOFT_ON
		_userPut++;
		break;
	case 0x97:		// SO_USERPUT_SOFT_OFF
		_userPut--;
		break;
	case 0x9C:		// SO_CHARSET_SET
		initCharset(pop());
		break;
	case 0x9D:		// SO_CHARSET_COLOR
		getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < 16; i++)
			_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)args[i];
		break;
	default:
		error("o80_cursorCommand: default case %x", subOp);
	}

	VAR(VAR_CURSORSTATE) = _cursor.state;
	VAR(VAR_USERPUT) = _userPut;
}

void ScummEngine_v80he::o80_setState() {
	int state = pop();
	int obj = pop();

	state &= 0x7FFF;
	putState(obj, state);
	removeObjectFromDrawQue(obj);
}

void ScummEngine_v80he::o80_drawWizPolygon() {
	WizImage wi;
	wi.x1 = wi.y1 = pop();
	wi.resNum = pop();
	wi.state = 0;
	wi.flags = kWIFIsPolygon;
	_wiz->displayWizImage(&wi);
}

/**
 * Draw a 'line' between two points.
 *
 * @param x1	the starting x coordinate
 * @param y1	the starting y coordinate
 * @param x		the ending x coordinate
 * @param y		the ending y coordinate
 * @param step	the step size used to render the line, only ever 'step'th point is drawn
 * @param type	the line type -- points are rendered by drawing actors (type == 2),
 *              wiz images (type == 3), or pixels (any other type)
 * @param id	the id of an actor, wizimage or color (low bit) & flag (high bit)
 */
void ScummEngine_v80he::drawLine(int x1, int y1, int x, int y, int step, int type, int id) {
	if (step < 0) {
		step = -step;
	}
	if (step == 0) {
		step = 1;
	}

	const int dx = x - x1;
	const int dy = y - y1;

	const int absDX = ABS(dx);
	const int absDY = ABS(dy);

	const int maxDist = MAX(absDX, absDY);

	y = y1;
	x = x1;


	if (type == 2) {
		ActorHE *a = (ActorHE *)derefActor(id, "drawLine");
		a->drawActorToBackBuf(x, y);
	} else if (type == 3) {
		WizImage wi;
		wi.flags = 0;
		wi.y1 = y;
		wi.x1 = x;
		wi.resNum = id;
		wi.state = 0;
		_wiz->displayWizImage(&wi);
	} else {
		drawPixel(x, y, id);
	}

	int stepCount = 0;
	int tmpX = 0;
	int tmpY = 0;
	for (int i = 0; i <= maxDist; i++) {
		tmpX += absDX;
		tmpY += absDY;

		int drawFlag = 0;

		if (tmpX > maxDist) {
			drawFlag = 1;
			tmpX -= maxDist;

			if (dx >= 0) {
				x++;
			} else {
				x--;
			}
		}
		if (tmpY > maxDist) {
			drawFlag = dy;
			tmpY -= maxDist;

			if (dy >= 0) {
				y++;
			} else {
				y--;
			}
		}

		if (drawFlag == 0)
			continue;

		if ((stepCount++ % step) != 0 && maxDist != i)
			continue;

		if (type == 2) {
			ActorHE *a = (ActorHE *)derefActor(id, "drawLine");
			a->drawActorToBackBuf(x, y);
		} else if (type == 3) {
			WizImage wi;
			wi.flags = 0;
			wi.y1 = y;
			wi.x1 = x;
			wi.resNum = id;
			wi.state = 0;
			_wiz->displayWizImage(&wi);
		} else {
			drawPixel(x, y, id);
		}
	}
}

void ScummEngine_v80he::drawPixel(int x, int y, int flags) {
	byte *src, *dst;
	VirtScreen *vs;

	if (x < 0 || x > 639)
		return;

	if (y < 0)
		return;

	if ((vs = findVirtScreen(y)) == NULL)
		return;

	markRectAsDirty(vs->number, x, y, x, y + 1);

	if ((flags & 0x4000) || (flags & 0x2000000)) {
		src = vs->getPixels(x, y);
		dst = vs->getBackPixels(x, y);
		*dst = *src;
	} else if ((flags & 0x2000) || (flags & 4000000)) {
		src = vs->getBackPixels(x, y);
		dst = vs->getPixels(x, y);
		*dst = *src;
	} else if (flags & 0x8000000) {
		error("drawPixel: unsupported flag 0x%x", flags);
	} else {
		dst = vs->getPixels(x, y);
		*dst = flags;
		if ((flags & 0x8000) || (flags & 0x1000000)) {
			dst = vs->getBackPixels(x, y);
			*dst = flags;
		}
	}
}

void ScummEngine_v80he::o80_drawLine() {
	int id, step, x, y, x1, y1;

	step = pop();
	id = pop();
	y = pop();
	x = pop();
	y1 = pop();
	x1 = pop();

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 55:
		drawLine(x1, y1, x, y, step, 2, id);
		break;
	case 63:
		drawLine(x1, y1, x, y, step, 3, id);
		break;
	case 66:
		drawLine(x1, y1, x, y, step, 1, id);
		break;
	default:
		error("o80_drawLine: default case %d", subOp);
	}

}

void ScummEngine_v80he::o80_pickVarRandom() {
	int num;
	int args[100];
	int32 dim1end;

	num = getStackList(args, ARRAYSIZE(args));
	int value = fetchScriptWord();

	if (readVar(value) == 0) {
		defineArray(value, kDwordArray, 0, 0, 0, num);
		if (value & 0x8000)
			localizeArray(readVar(value), 0xFF);
		else if (value & 0x4000)
			localizeArray(readVar(value), _currentScript);

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
		num = 1;
		if (readArray(value, 0, 1) == var_2 && dim1end >= 3) {
			int32 tmp = readArray(value, 0, 2);
			writeArray(value, 0, num, tmp);
			writeArray(value, 0, 2, var_2);
		}
	}

	writeArray(value, 0, 0, num + 1);
	push(readArray(value, 0, num));
}

} // End of namespace Scumm

#endif // ENABLE_HE
