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

#include "common/translation.h"

#include "gui/message.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/draw.h"
#include "gob/save/saveload.h"

namespace Gob {

#define OPCODEVER Inter_v5
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_v5::Inter_v5(GobEngine *vm) : Inter_v4(vm) {
}

void Inter_v5::setupOpcodesDraw() {
	Inter_v4::setupOpcodesDraw();

	OPCODEDRAW(0x61, o5_deleteFile);
	OPCODEDRAW(0x80, o5_initScreen);
}

void Inter_v5::setupOpcodesFunc() {
	Inter_v4::setupOpcodesFunc();

	OPCODEFUNC(0x45, o5_istrlen);
}

void Inter_v5::setupOpcodesGob() {
	OPCODEGOB(  0, o5_spaceShooter);
	OPCODEGOB(  1, o5_spaceShooter);
	OPCODEGOB(  2, o5_spaceShooter);
	OPCODEGOB(  3, o5_spaceShooter);

	OPCODEGOB( 33, o5_spaceShooter);

	OPCODEGOB( 34, o5_spaceShooter);
	OPCODEGOB( 37, o5_spaceShooter);

	OPCODEGOB( 80, o5_getSystemCDSpeed);
	OPCODEGOB( 81, o5_getSystemRAM);
	OPCODEGOB( 82, o5_getSystemCPUSpeed);
	OPCODEGOB( 83, o5_getSystemDrawSpeed);
	OPCODEGOB( 84, o5_totalSystemSpecs);

	OPCODEGOB( 85, o5_saveSystemSpecs);
	OPCODEGOB( 86, o5_loadSystemSpecs);

	OPCODEGOB( 87, o5_spaceShooter);
	OPCODEGOB( 88, o5_spaceShooter);
	OPCODEGOB( 89, o5_spaceShooter);
	OPCODEGOB( 90, o5_spaceShooter);

	OPCODEGOB( 91, o5_spaceShooter);
	OPCODEGOB( 92, o5_gob92);
	OPCODEGOB( 93, o5_spaceShooter);
	OPCODEGOB( 94, o5_spaceShooter);

	OPCODEGOB( 95, o5_gob95);
	OPCODEGOB( 96, o5_gob96);
	OPCODEGOB( 97, o5_gob97);
	OPCODEGOB( 98, o5_gob98);

	OPCODEGOB( 99, o5_spaceShooter);
	OPCODEGOB(100, o5_gob100);
	OPCODEGOB(200, o5_gob200);
}

void Inter_v5::o5_deleteFile() {
	const char *file =_vm->_game->_script->evalString();

	debugC(2, kDebugFileIO, "Delete file \"%s\"", file);

	SaveLoad::SaveMode mode = _vm->_saveLoad->getSaveMode(file);
	if (mode == SaveLoad::kSaveModeSave) {

		if (!_vm->_saveLoad->deleteFile(file)) {
			GUI::MessageDialog dialog(_("Failed to delete file."));
			dialog.runModal();
		}

	} else if (mode == SaveLoad::kSaveModeNone)
		warning("Attempted to delete file \"%s\"", file);
}

void Inter_v5::o5_initScreen() {
	int16 offY;
	int16 videoMode;
	int16 width, height;

	offY = _vm->_game->_script->readInt16();

	videoMode = offY & 0xFF;
	offY = (offY >> 8) & 0xFF;

	width = _vm->_game->_script->readValExpr();
	height = _vm->_game->_script->readValExpr();

	if (videoMode == 0)
		videoMode = 0x14;

	_vm->_video->clearScreen();

	if (videoMode == 0x13) {

		if (width == -1)
			width = 320;
		if (height == -1)
			height = 200;

		_vm->_width = 320;
		_vm->_height = 200;

		_vm->_video->setSize(false);

	} else if (_vm->_global->_videoMode == 0x13) {
		width = _vm->_width = 640;
		height = _vm->_height = 480;

		_vm->_video->setSize(true);
	}

	_vm->_global->_fakeVideoMode = videoMode;

	// Some versions require this
	if (videoMode == 0x18)
		_vm->_global->_fakeVideoMode = 0x37;

	if ((videoMode == _vm->_global->_videoMode) && (width == -1))
		return;

	if (width > 0)
		_vm->_video->_surfWidth = width;
	if (height > 0)
		_vm->_video->_surfHeight = height;

	_vm->_video->_screenDeltaX = 0;
	if (_vm->_video->_surfWidth < _vm->_width)
		_vm->_video->_screenDeltaX = (_vm->_width - _vm->_video->_surfWidth) / 2;

	_vm->_global->_mouseMinX = _vm->_video->_screenDeltaX;
	_vm->_global->_mouseMaxX = _vm->_video->_screenDeltaX + _vm->_video->_surfWidth - 1;

	_vm->_video->_splitStart = _vm->_video->_surfHeight - offY;

	_vm->_video->_splitHeight1 = MIN<int16>(_vm->_height, _vm->_video->_surfHeight);
	_vm->_video->_splitHeight2 = offY;

	if ((_vm->_video->_surfHeight + offY) < _vm->_height)
		_vm->_video->_screenDeltaY = (_vm->_height - (_vm->_video->_surfHeight + offY)) / 2;
	else
		_vm->_video->_screenDeltaY = 0;

	_vm->_global->_mouseMaxY = (_vm->_video->_surfHeight + _vm->_video->_screenDeltaY) - offY - 1;
	_vm->_global->_mouseMinY = _vm->_video->_screenDeltaY;

	_vm->_draw->closeScreen();
	_vm->_util->clearPalette();
	memset(_vm->_global->_redPalette, 0, 256);
	memset(_vm->_global->_greenPalette, 0, 256);
	memset(_vm->_global->_bluePalette, 0, 256);

	_vm->_video->_splitSurf.reset();
	_vm->_draw->_spritesArray[24].reset();
	_vm->_draw->_spritesArray[25].reset();

	_vm->_global->_videoMode = videoMode;
	_vm->_video->initPrimary(videoMode);
	WRITE_VAR(15, _vm->_global->_fakeVideoMode);

	_vm->_global->_setAllPalette = true;

	_vm->_util->setMousePos(_vm->_global->_inter_mouseX,
			_vm->_global->_inter_mouseY);
	_vm->_util->clearPalette();

	_vm->_draw->initScreen();

	_vm->_util->setScrollOffset();

	if (offY > 0) {
		_vm->_draw->_spritesArray[24] = SurfacePtr(new Surface(_vm->_width, offY, _vm->getPixelFormat().bytesPerPixel));
		_vm->_draw->_spritesArray[25] = SurfacePtr(new Surface(_vm->_width, offY, _vm->getPixelFormat().bytesPerPixel));
		_vm->_video->_splitSurf = _vm->_draw->_spritesArray[25];
	}
}

void Inter_v5::o5_istrlen(OpFuncParams &params) {
	int16 strVar1, strVar2;
	int16 len;
	uint16 type;

	if (_vm->_game->_script->peekByte() == 0x80) {
		_vm->_game->_script->skip(1);

		strVar1 = _vm->_game->_script->readVarIndex();
		strVar2 = _vm->_game->_script->readVarIndex(0, &type);

		len = _vm->_draw->stringLength(GET_VARO_STR(strVar1), READ_VARO_UINT16(strVar2));

	} else {

		strVar1 = _vm->_game->_script->readVarIndex();
		strVar2 = _vm->_game->_script->readVarIndex(0, &type);

		if (_vm->_global->_language == 10) {
			// Extra handling for Japanese strings

			for (len = 0; READ_VARO_UINT8(strVar1) != 0; strVar1++, len++)
				if (READ_VARO_UINT8(strVar1) >= 128)
					strVar1++;

		} else
			len = strlen(GET_VARO_STR(strVar1));
	}

	writeVar(strVar2, type, (int32) len);
}

void Inter_v5::o5_spaceShooter(OpGobParams &params) {
	warning("Dynasty Stub: Space shooter: %d, %d, %s",
			params.extraData, params.paramCount, _vm->_game->_curTotFile.c_str());

	if (params.paramCount < 4) {
		warning("Space shooter variable counter < 4");
		_vm->_game->_script->skip(params.paramCount * 2);
		return;
	}

	uint32 var1 = _vm->_game->_script->readInt16() * 4;
	uint32 var2 = _vm->_game->_script->readInt16() * 4;

	_vm->_game->_script->readInt16();
	_vm->_game->_script->readInt16();

	if (params.extraData != 0) {
		// we need to return 1 for the shooter mission 34. There is only one planet to choose from in the map.
		WRITE_VARO_UINT32(var1,(params.extraData == 34) ? 1 : 2);
		WRITE_VARO_UINT32(var2,0);
	} else {
		if (params.paramCount < 5) {
			warning("Space shooter variable counter < 5");
			return;
		}

		_vm->_game->_script->skip((params.paramCount - 4) * 2);
	}
}

void Inter_v5::o5_getSystemCDSpeed(OpGobParams &params) {
	WRITE_VAR_UINT32(_vm->_game->_script->readInt16(), 100); // Fudging 100%

	Font *font;
	if ((font = _vm->_draw->loadFont("SPEED.LET"))) {
		font->drawString("100 %", 402, 89, 112, 144, 0, *_vm->_draw->_backSurface);
		_vm->_draw->forceBlit();

		delete font;
	}
}

void Inter_v5::o5_getSystemRAM(OpGobParams &params) {
	WRITE_VAR_UINT32(_vm->_game->_script->readInt16(), 100); // Fudging 100%

	Font *font;
	if ((font = _vm->_draw->loadFont("SPEED.LET"))) {
		font->drawString("100 %", 402, 168, 112, 144, 0, *_vm->_draw->_backSurface);
		_vm->_draw->forceBlit();

		delete font;
	}
}

void Inter_v5::o5_getSystemCPUSpeed(OpGobParams &params) {
	WRITE_VAR_UINT32(_vm->_game->_script->readInt16(), 100); // Fudging 100%

	Font *font;
	if ((font = _vm->_draw->loadFont("SPEED.LET"))) {
		font->drawString("100 %", 402, 248, 112, 144, 0, *_vm->_draw->_backSurface);
		_vm->_draw->forceBlit();

		delete font;
	}
}

void Inter_v5::o5_getSystemDrawSpeed(OpGobParams &params) {
	WRITE_VAR_UINT32(_vm->_game->_script->readInt16(), 100); // Fudging 100%

	Font *font;
	if ((font = _vm->_draw->loadFont("SPEED.LET"))) {
		font->drawString("100 %", 402, 326, 112, 144, 0, *_vm->_draw->_backSurface);
		_vm->_draw->forceBlit();

		delete font;
	}
}

void Inter_v5::o5_totalSystemSpecs(OpGobParams &params) {
	WRITE_VAR_UINT32(_vm->_game->_script->readInt16(), 100); // Fudging 100%

	Font *font;
	if ((font = _vm->_draw->loadFont("SPEED.LET"))) {
		font->drawString("100 %", 402, 405, 112, 144, 0, *_vm->_draw->_backSurface);
		_vm->_draw->forceBlit();

		delete font;
	}
}

void Inter_v5::o5_saveSystemSpecs(OpGobParams &params) {
	warning("Dynasty Stub: Saving system specifications");

/*
	FILE *f = fopen("SAVE\\SPEED.INF", w);
	fwrite(&_cdSpeed,   sizeof(_cdSpeed),   1, f);
	fwrite(&_ram,       sizeof(_ram),       1, f);
	fwrite(&_cpuSpeed,  sizeof(_cpuSpeed),  1, f);
	fwrite(&_drawSpeed, sizeof(_drawSpeed), 1, f);
	fwrite(&_total,     sizeof(_total),     1, f);
	fclose(f);
*/
}

void Inter_v5::o5_loadSystemSpecs(OpGobParams &params) {
	warning("Dynasty Stub: Loading system specifications");

/*
	FILE *f = fopen("SAVE\\SPEED.INF", r);
	fread(&_cdSpeed,   sizeof(_cdSpeed),   1, f);
	fread(&_ram,       sizeof(_ram),       1, f);
	fread(&_cpuSpeed,  sizeof(_cpuSpeed),  1, f);
	fread(&_drawSpeed, sizeof(_drawSpeed), 1, f);
	fread(&_total,     sizeof(_total),     1, f);
	fclose(f);
*/

/*
	speedSum = MAX(_cdSpeed, 150);
	speedSum += (_ram << 3);
	speedSum += (_cpuSpeed << 3);
	speedSum /= 17;

	needThrottle2 = (speedSum > 81) ? 1 : 0;
	needThrottle1 = (_total >= 95) ? 1 : 0;

	if (needThrottle1 == 1) {
		speedThrottle1 = 100;
		speedThrottle2 = 1;
		speedThrottle3 = 1;
		speedThrottle4 = 0;
	} else {
		speedThrottle1 = 0;
		speedThrottle2 = 0;
		speedThrottle3 = 0;
		speedThrottle4 = 40;
	}
*/
}

void Inter_v5::o5_gob92(OpGobParams &params) {
	warning("Dynasty Stub: GobFunc 92");

	WRITE_VAR_UINT32(_vm->_game->_script->readInt16(), 0 /* (uint32) ((int32) ((int8) _gob92_1)) */);
}

void Inter_v5::o5_gob95(OpGobParams &params) {
	warning("Dynasty Stub: GobFunc 95");

	WRITE_VAR_UINT32(_vm->_game->_script->readInt16(), 0 /* (uint32) ((int32) ((int16) speedThrottle4)) */);
	WRITE_VAR_UINT32(_vm->_game->_script->readInt16(), 0 /* (uint32) ((int32) ((int8)  speedThrottle3)) */);
	WRITE_VAR_UINT32(_vm->_game->_script->readInt16(), 0 /* (uint32) ((int32) ((int8)  speedThrottle2)) */);
	WRITE_VAR_UINT32(_vm->_game->_script->readInt16(), 0 /* (uint32) ((int32) ((int16) speedThrottle1)) */);
}

void Inter_v5::o5_gob96(OpGobParams &params) {
	int16 speedThrottle4, speedThrottle1;
	byte speedThrottle3, speedThrottle2;

	speedThrottle4 = READ_VAR_UINT16(_vm->_game->_script->readInt16());
	speedThrottle3 = READ_VAR_UINT8(_vm->_game->_script->readInt16());
	speedThrottle2 = READ_VAR_UINT8(_vm->_game->_script->readInt16());
	speedThrottle1 = READ_VAR_UINT16(_vm->_game->_script->readInt16());

	warning("Dynasty Stub: GobFunc 96: %d, %d, %d, %d",
			speedThrottle4, speedThrottle3, speedThrottle2, speedThrottle1);

	// TODO
}

void Inter_v5::o5_gob97(OpGobParams &params) {
	_gob_97_98_val = 1;
}

void Inter_v5::o5_gob98(OpGobParams &params) {
	_gob_97_98_val = 0;
}

void Inter_v5::o5_gob100(OpGobParams &params) {
	uint16 var1 = READ_VAR_UINT16(_vm->_game->_script->readInt16());
	uint16 var2 = READ_VAR_UINT16(_vm->_game->_script->readInt16());
	uint16 var3 = READ_VAR_UINT16(_vm->_game->_script->readInt16());
	uint16 var4 = READ_VAR_UINT16(_vm->_game->_script->readInt16());

	warning("Dynasty Stub: GobFunc 100: %d, %d, %d, %d", var1, var2, var3, var4);

	var3 = (var3 + var1) - 1;
	var4 = (var4 + var2) - 1;
}

void Inter_v5::o5_gob200(OpGobParams &params) {
	uint16 var1 = _vm->_game->_script->readUint16(); // index into the spritesArray
	uint16 var2 = _vm->_game->_script->readUint16();
	uint16 var3 = _vm->_game->_script->readUint16();

	warning("Dynasty Stub: GobFunc 200: %d, %d, %d", var1, var2, var3);
}

} // End of namespace Gob
