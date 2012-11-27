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
#include "common/str.h"
#include "common/translation.h"

#include "gui/message.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/expression.h"
#include "gob/script.h"
#include "gob/hotspots.h"
#include "gob/palanim.h"
#include "gob/scenery.h"
#include "gob/video.h"
#include "gob/videoplayer.h"
#include "gob/save/saveload.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODEVER Inter_Playtoons
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_Playtoons::Inter_Playtoons(GobEngine *vm) : Inter_v6(vm) {
}

void Inter_Playtoons::setupOpcodesDraw() {
	Inter_v6::setupOpcodesDraw();

// In the code, the Draw codes 0x00 to 0x06 and 0x13 are replaced by an engrish
// error message. As it's useless, they are simply cleared.
	CLEAROPCODEDRAW(0x00);
	CLEAROPCODEDRAW(0x01);
	CLEAROPCODEDRAW(0x02);
	CLEAROPCODEDRAW(0x03);
	CLEAROPCODEDRAW(0x04);
	CLEAROPCODEDRAW(0x05);
	CLEAROPCODEDRAW(0x06);
	CLEAROPCODEDRAW(0x13);

	CLEAROPCODEDRAW(0x21);
	CLEAROPCODEDRAW(0x22);
	CLEAROPCODEDRAW(0x24);

	OPCODEDRAW(0x17, oPlaytoons_loadMultObject);
	OPCODEDRAW(0x19, oPlaytoons_getObjAnimSize);
	OPCODEDRAW(0x20, oPlaytoons_CD_20_23);
	OPCODEDRAW(0x23, oPlaytoons_CD_20_23);
	OPCODEDRAW(0x25, oPlaytoons_CD_25);
	OPCODEDRAW(0x60, oPlaytoons_copyFile);
	OPCODEDRAW(0x85, oPlaytoons_openItk);
}

void Inter_Playtoons::setupOpcodesFunc() {
	Inter_v6::setupOpcodesFunc();

	CLEAROPCODEFUNC(0x3D);
	OPCODEFUNC(0x0B, oPlaytoons_printText);
	OPCODEFUNC(0x1B, oPlaytoons_F_1B);
	OPCODEFUNC(0x24, oPlaytoons_putPixel);
	OPCODEFUNC(0x27, oPlaytoons_freeSprite);
	OPCODEFUNC(0x3F, oPlaytoons_checkData);
	OPCODEFUNC(0x4D, oPlaytoons_readData);
}

void Inter_Playtoons::setupOpcodesGob() {
}

void Inter_Playtoons::oPlaytoons_printText(OpFuncParams &params) {
	char buf[60];
	int i;
	int16 oldTransparency;

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();

	_vm->_draw->_backColor = _vm->_game->_script->readValExpr();
	_vm->_draw->_frontColor = _vm->_game->_script->readValExpr();
	_vm->_draw->_fontIndex = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSurface = Draw::kBackSurface;
	_vm->_draw->_textToPrint = buf;
	_vm->_draw->_transparency = 0;

	if (_vm->_draw->_backColor == 16) {
		_vm->_draw->_backColor = 0;
		_vm->_draw->_transparency = 1;
	}

// colMod is read from conf file (_off_=xxx).
// in Playtoons, it's not present in the conf file, thus always equal to the default value (0).
// Maybe used in ADIs...
//	if (!_vm->_draw->_transparency)
//		_vm->_draw->_backColor += colMod;
//	_vm->_draw->_frontColor += colMod;

	oldTransparency = _vm->_draw->_transparency;
	do {
		for (i = 0; (_vm->_game->_script->peekChar() != '.') &&
				(_vm->_game->_script->peekByte() != 200); i++) {
			buf[i] = _vm->_game->_script->readChar();
		}

		if (_vm->_game->_script->peekByte() != 200) {
			_vm->_game->_script->skip(1);
			switch (_vm->_game->_script->peekByte()) {
			case TYPE_VAR_INT8:
			case TYPE_ARRAY_INT8:
				sprintf(buf + i, "%d",
						(int8) READ_VARO_UINT8(_vm->_game->_script->readVarIndex()));
				break;

			case TYPE_VAR_INT16:
			case TYPE_VAR_INT32_AS_INT16:
			case TYPE_ARRAY_INT16:
				sprintf(buf + i, "%d",
						(int16) READ_VARO_UINT16(_vm->_game->_script->readVarIndex()));
				break;

			case TYPE_VAR_INT32:
			case TYPE_ARRAY_INT32:
				sprintf(buf + i, "%d",
						(int32)VAR_OFFSET(_vm->_game->_script->readVarIndex()));
				break;

			case TYPE_VAR_STR:
			case TYPE_ARRAY_STR:
				sprintf(buf + i, "%s",
						GET_VARO_STR(_vm->_game->_script->readVarIndex()));
				break;
			}
			_vm->_game->_script->skip(1);
		} else
			buf[i] = 0;

		if (_vm->_game->_script->peekByte() == 200) {
			_vm->_draw->_spriteBottom = _vm->_draw->_fonts[_vm->_draw->_fontIndex]->getCharHeight();
			_vm->_draw->_spriteRight = _vm->_draw->stringLength(_vm->_draw->_textToPrint, _vm->_draw->_fontIndex);
			_vm->_draw->adjustCoords(1, &_vm->_draw->_spriteBottom, &_vm->_draw->_spriteRight);
			if (_vm->_draw->_transparency == 0) {
				_vm->_draw->spriteOperation(DRAW_FILLRECT);
				_vm->_draw->_transparency = 1;
			}
			_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
			_vm->_draw->_transparency = oldTransparency;
			i = 0;
		} else
			i = strlen(buf);
	} while (_vm->_game->_script->peekByte() != 200);

	_vm->_game->_script->skip(1);
}

void Inter_Playtoons::oPlaytoons_F_1B(OpFuncParams &params) {
	_vm->_game->_hotspots->oPlaytoons_F_1B();
}

void Inter_Playtoons::oPlaytoons_putPixel(OpFuncParams &params) {
	_vm->_draw->_destSurface = _vm->_game->_script->readInt16();

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();

	_vm->_game->_script->readExpr(99, 0);

	//unk_var is always set to 0 in Playtoons
	_vm->_draw->_frontColor = _vm->_game->_script->getResultInt() & 0xFFFF; // + unk_var;

	_vm->_draw->_pattern = _vm->_game->_script->getResultInt()>>16;

	_vm->_draw->spriteOperation(DRAW_PUTPIXEL);
}

void Inter_Playtoons::oPlaytoons_freeSprite(OpFuncParams &params) {
	int16 index;
	if (_vm->_game->_script->peekByte(1) == 0)
		index = _vm->_game->_script->readInt16();
	else
		index = _vm->_game->_script->readValExpr();
	_vm->_draw->freeSprite(index);
}

void Inter_Playtoons::oPlaytoons_checkData(OpFuncParams &params) {
	Common::String file = getFile(_vm->_game->_script->evalString());

	uint16 varOff = _vm->_game->_script->readVarIndex();

	int32 size   = -1;
	int16 handle = 1;
	SaveLoad::SaveMode mode = _vm->_saveLoad->getSaveMode(file.c_str());
	if (mode == SaveLoad::kSaveModeNone) {
		size = _vm->_dataIO->fileSize(file);
		if (size == -1)
			warning("File \"%s\" not found", file.c_str());

	} else if (mode == SaveLoad::kSaveModeSave)
		size = _vm->_saveLoad->getSize(file.c_str());
	else if (mode == SaveLoad::kSaveModeExists)
		size = 23;

	if (size == -1)
		handle = -1;

	debugC(2, kDebugFileIO, "Requested size of file \"%s\": %d",
			file.c_str(), size);

	WRITE_VAR_OFFSET(varOff, handle);
	WRITE_VAR(16, (uint32) size);
}

void Inter_Playtoons::oPlaytoons_readData(OpFuncParams &params) {
	Common::String file = getFile(_vm->_game->_script->evalString());

	uint16 dataVar = _vm->_game->_script->readVarIndex();
	int32  size    = _vm->_game->_script->readValExpr();
	int32  offset  = _vm->_game->_script->evalInt();
	int32  retSize = 0;

	debugC(2, kDebugFileIO, "Read from file \"%s\" (%d, %d bytes at %d)",
			file.c_str(), dataVar, size, offset);

	SaveLoad::SaveMode mode = _vm->_saveLoad->getSaveMode(file.c_str());
	if (mode == SaveLoad::kSaveModeSave) {

		WRITE_VAR(1, 1);

		if (!_vm->_saveLoad->load(file.c_str(), dataVar, size, offset)) {
			GUI::MessageDialog dialog(_("Failed to load game state from file."));
			dialog.runModal();
		} else
			WRITE_VAR(1, 0);

		return;

	} else if (mode == SaveLoad::kSaveModeIgnore)
		return;

	if (size < 0) {
		if (readSprite(file, dataVar, size, offset))
			WRITE_VAR(1, 0);
		return;
	} else if (size == 0) {
		dataVar = 0;
		size = _vm->_game->_script->getVariablesCount() * 4;
	}

	byte *buf = _variables->getAddressOff8(dataVar);

	if (file[0] == 0) {
		WRITE_VAR(1, size);
		return;
	}

	WRITE_VAR(1, 1);
	Common::SeekableReadStream *stream = _vm->_dataIO->getFile(file);
	if (!stream)
		return;

	_vm->_draw->animateCursor(4);
	if (offset > stream->size()) {
		warning("oPlaytoons_readData: File \"%s\", Offset (%d) > file size (%d)",
				file.c_str(), offset, stream->size());
		delete stream;
		return;
	}

	if (offset < 0)
		stream->seek(offset + 1, SEEK_END);
	else
		stream->seek(offset);

	if (((dataVar >> 2) == 59) && (size == 4)) {
		WRITE_VAR(59, stream->readUint32LE());
		// The scripts in some versions divide through 256^3 then,
		// effectively doing a LE->BE conversion
		if ((_vm->getPlatform() != Common::kPlatformPC) && (VAR(59) < 256))
			WRITE_VAR(59, SWAP_BYTES_32(VAR(59)));
	} else
		retSize = stream->read(buf, size);

	if (retSize == size)
		WRITE_VAR(1, 0);

	delete stream;
}

void Inter_Playtoons::oPlaytoons_loadMultObject() {
	assert(_vm->_mult->_objects);

	uint16 objIndex = _vm->_game->_script->readValExpr();

	debugC(4, kDebugGameFlow, "Loading mult object %d", objIndex);

	Mult::Mult_Object &obj = _vm->_mult->_objects[objIndex];
	Mult::Mult_AnimData &objAnim = *(obj.pAnimData);

	*obj.pPosX = _vm->_game->_script->readValExpr();
	*obj.pPosY = _vm->_game->_script->readValExpr();

	byte *multData = (byte *) &objAnim;
	for (int i = 0; i < 11; i++) {
		if (_vm->_game->_script->peekByte() != 99)
			multData[i] = _vm->_game->_script->readValExpr();
		else
			_vm->_game->_script->skip(1);
	}

	if (((int32)*obj.pPosX != -1234) || ((int32)*obj.pPosY == -4321))
		return;

	warning("Stub: oPlaytoons_loadMultObject: pPosX == -1234, pPosY == -4321");
}

void Inter_Playtoons::oPlaytoons_getObjAnimSize() {
	int16 objIndex;
	uint16 readVar[4];
	uint16 types[4];
	Mult::Mult_AnimData animData;

	_vm->_game->_script->evalExpr(&objIndex);

	for (int i = 0; i < 4; i++)
		readVar[i] = _vm->_game->_script->readVarIndex(0, &types[0]);

	if (objIndex == -1) {
		warning("oPlaytoons_getObjAnimSize case -1 not implemented");
		return;
	}

	if (objIndex == -2) {
		bool doBreak = false;
		for (int i = 0; i < 3; i++)
			storeValue(readVar[i], types[i], (uint32) -1);

		for (int i = readValue(readVar[3], types[3]); i < _vm->_mult->_objCount; i++) {
			if (_vm->_mult->_objects[i].pAnimData->isStatic != 0)
				continue;

			byte *data = (byte *)_vm->_mult->_objects[i].pAnimData;

			for (int j = 1; j < 39; j += 2) {
				int32 value1 = READ_VARO_UINT32(readVar[3] +  j      * 4);
				int32 value2 = READ_VARO_UINT32(readVar[3] + (j + 1) * 4);
				if (value1 == -1) {
					doBreak = true;
					break;
				}

				if (value1 >= 0) {
					if ((int8)data[value1] != value2)
						break;
				} else {
					if ((int8)data[-value1] == value2)
						break;
				}

			}

			if (doBreak) {
				storeValue(readVar[0], types[0], i);
				break;
			}
		}

		return;
	}

	if ((objIndex < 0) || (objIndex >= _vm->_mult->_objCount)) {
		warning("oPlaytoons_getObjAnimSize(): objIndex = %d (%d)", objIndex, _vm->_mult->_objCount);
		_vm->_scenery->_toRedrawLeft   = 0;
		_vm->_scenery->_toRedrawTop    = 0;
		_vm->_scenery->_toRedrawRight  = 0;
		_vm->_scenery->_toRedrawBottom = 0;
	} else {
		animData = *(_vm->_mult->_objects[objIndex].pAnimData);
		if (animData.isStatic == 0)
			_vm->_scenery->updateAnim(animData.layer, animData.frame,
					animData.animation, 0, *(_vm->_mult->_objects[objIndex].pPosX),
					*(_vm->_mult->_objects[objIndex].pPosY), 0);

		_vm->_scenery->_toRedrawLeft = MAX<int16>(_vm->_scenery->_toRedrawLeft, 0);
		_vm->_scenery->_toRedrawTop  = MAX<int16>(_vm->_scenery->_toRedrawTop , 0);
	}

	WRITE_VAR_OFFSET(readVar[0], _vm->_scenery->_toRedrawLeft);
	WRITE_VAR_OFFSET(readVar[1], _vm->_scenery->_toRedrawTop);
	WRITE_VAR_OFFSET(readVar[2], _vm->_scenery->_toRedrawRight);
	WRITE_VAR_OFFSET(readVar[3], _vm->_scenery->_toRedrawBottom);
}

void Inter_Playtoons::oPlaytoons_CD_20_23() {
	_vm->_game->_script->evalExpr(0);
}

void Inter_Playtoons::oPlaytoons_CD_25() {
	_vm->_game->_script->readVarIndex();
	_vm->_game->_script->readVarIndex();
}

void Inter_Playtoons::oPlaytoons_copyFile() {
	Common::String path1 = _vm->_game->_script->evalString();
	Common::String path2 = _vm->_game->_script->evalString();

	Common::String file1 = getFile(path1.c_str());
	Common::String file2 = getFile(path2.c_str());

	if (file1.equalsIgnoreCase(file2)) {
		warning("oPlaytoons_copyFile(): \"%s\" == \"%s\"", path1.c_str(), path2.c_str());
		return;
	}

	warning("Playtoons Stub: copy file from \"%s\" to \"%s\"", file1.c_str(), file2.c_str());
}

void Inter_Playtoons::oPlaytoons_openItk() {
	Common::String file = getFile(_vm->_game->_script->evalString());
	if (!file.contains('.'))
		file += ".ITK";

	_vm->_dataIO->openArchive(file, false);
}

Common::String Inter_Playtoons::getFile(const char *path) {
	const char *orig = path;

	if      (!strncmp(path, "@:\\", 3))
		path += 3;
	else if (!strncmp(path, "<ME>", 4))
		path += 4;
	else if (!strncmp(path, "<CD>", 4))
		path += 4;
	else if (!strncmp(path, "<STK>", 5))
		path += 5;
	else if (!strncmp(path, "<ALLCD>", 7))
		path += 7;

	const char *backslash = strrchr(path, '\\');
	if (backslash)
		path = backslash + 1;

	if (orig != path)
		debugC(2, kDebugFileIO, "Inter_Playtoons::getFile(): Evaluating path"
				"\"%s\" to \"%s\"", orig, path);

	return path;
}

bool Inter_Playtoons::readSprite(Common::String file, int32 dataVar,
		int32 size, int32 offset) {

	// WORKAROUND: Adibou copies TEMP.CSA to TEMP01.CSA, which isn't yet implemented
	if (file.equalsIgnoreCase("TEMP01.CSA"))
		file = "TEMP.CSA";

	bool palette = false;
	if (size < -1000) {
		palette = true;
		size += 1000;
	}

	int index = -size - 1;
	if ((index < 0) || (index >= Draw::kSpriteCount) || !_vm->_draw->_spritesArray[index]) {
		warning("No such sprite");
		return false;
	}

	SurfacePtr sprite = _vm->_draw->_spritesArray[index];
	if (sprite->getBPP() != 1) {
		warning("bpp != 1");
		return false;
	}

	Common::SeekableReadStream *stream = _vm->_dataIO->getFile(file);
	if (!stream) {
		warning("No such file \"%s\"", file.c_str());
		return false;
	}

	int32 spriteSize = sprite->getWidth() * sprite->getHeight();
	int32 dataSize   = stream->size();

	if (palette)
		dataSize -= 768;

	int32 readSize = MIN(spriteSize, dataSize);
	if (readSize <= 0)
		return true;

	stream->read(sprite->getData(), readSize);

	if (palette)
		stream->read((byte *)_vm->_global->_pPaletteDesc->vgaPal, 768);

	delete stream;
	return true;
}

} // End of namespace Gob
