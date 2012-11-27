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

#include "gob/hotspots.h"
#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/expression.h"
#include "gob/script.h"
#include "gob/palanim.h"
#include "gob/video.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODEVER Inter_Fascination
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_Fascination::Inter_Fascination(GobEngine *vm) : Inter_v2(vm) {
}

void Inter_Fascination::setupOpcodesDraw() {
	Inter_v2::setupOpcodesDraw();

	OPCODEDRAW(0x03, oFascin_setWinSize);
	OPCODEDRAW(0x04, oFascin_closeWin);
	OPCODEDRAW(0x05, oFascin_activeWin);
	OPCODEDRAW(0x06, oFascin_openWin);

	OPCODEDRAW(0x08, oFascin_initCursorAnim);

	OPCODEDRAW(0x0A, oFascin_setRenderFlags);
	OPCODEDRAW(0x0B, oFascin_setWinFlags);

	CLEAROPCODEDRAW(0x50);
	CLEAROPCODEDRAW(0x51);
	CLEAROPCODEDRAW(0x52);
	CLEAROPCODEDRAW(0x53);

	CLEAROPCODEDRAW(0x54);
	CLEAROPCODEDRAW(0x55);
	CLEAROPCODEDRAW(0x56);

	CLEAROPCODEDRAW(0x80);
	CLEAROPCODEDRAW(0x81);
	CLEAROPCODEDRAW(0x82);
	CLEAROPCODEDRAW(0x83);

	CLEAROPCODEDRAW(0x84);
	CLEAROPCODEDRAW(0x85);
	CLEAROPCODEDRAW(0x86);
	CLEAROPCODEDRAW(0x87);

	CLEAROPCODEDRAW(0x88);
}

void Inter_Fascination::setupOpcodesFunc() {
	Inter_v2::setupOpcodesFunc();

	OPCODEFUNC(0x06, oFascin_repeatUntil);
	OPCODEFUNC(0x09, oFascin_assign);
	OPCODEFUNC(0x32, oFascin_copySprite);
}

void Inter_Fascination::setupOpcodesGob() {
	OPCODEGOB(   1, oFascin_playTirb);
	OPCODEGOB(   2, oFascin_playTira);
	OPCODEGOB(   3, oFascin_loadExtasy);
	OPCODEGOB(   4, oFascin_adlibPlay);

	OPCODEGOB(   5, oFascin_adlibStop);
	OPCODEGOB(   6, oFascin_adlibUnload);
	OPCODEGOB(   7, oFascin_loadMus1);
	OPCODEGOB(   8, oFascin_loadMus2);

	OPCODEGOB(   9, oFascin_loadMus3);
	OPCODEGOB(  10, oFascin_loadBatt1);
	OPCODEGOB(  11, oFascin_loadBatt2);
	OPCODEGOB(  12, oFascin_loadBatt3);

	OPCODEGOB(1000, oFascin_loadMod);
	OPCODEGOB(1001, oFascin_playProtracker);
	OPCODEGOB(1002, o2_stopProtracker);
}

void Inter_Fascination::oFascin_repeatUntil(OpFuncParams &params) {
	int16 size;
	bool flag;

	_nestLevel[0]++;

	uint32 blockPos = _vm->_game->_script->pos();

	do {
		_vm->_game->_script->seek(blockPos);
		size = _vm->_game->_script->peekUint16(2) + 2;

		funcBlock(1);

		_vm->_game->_script->seek(blockPos + size + 1);

		flag = _vm->_game->_script->evalBool();

		// WORKAROUND: The script of the PC version of Fascination, when the protection check
		// fails, writes on purpose everywhere in the memory in order to hang the computer.
		// This results in a crash in Scummvm. This workaround avoids that crash.
		if (_vm->getPlatform() == Common::kPlatformPC) {
			if (((blockPos == 3533) && _vm->isCurrentTot("INTRO1.TOT")) ||
			    ((blockPos == 3519) && _vm->isCurrentTot("INTRO2.TOT")) ||
			    ((blockPos == 3265) && _vm->isCurrentTot("INTRO2.TOT")))  //PC Hebrew
				_terminate = 1;
		}
	} while (!flag && !_break && !_terminate && !_vm->shouldQuit());

	_nestLevel[0]--;

	if (*_breakFromLevel > -1) {
		_break = false;
		*_breakFromLevel = -1;
	}
}

void Inter_Fascination::oFascin_assign(OpFuncParams &params) {
	byte destType = _vm->_game->_script->peekByte();
	int16 dest = _vm->_game->_script->readVarIndex();

	byte loopCount;
	if (_vm->_game->_script->peekByte() == 99) {
		_vm->_game->_script->skip(1);
		loopCount = _vm->_game->_script->readByte();
	} else
		loopCount = 1;

	for (int i = 0; i < loopCount; i++) {
		int16 result;
		int16 srcType = _vm->_game->_script->evalExpr(&result);

		switch (destType) {
		case TYPE_VAR_INT8:
			if (srcType != TYPE_IMM_INT16) {
				char* str = _vm->_game->_script->getResultStr();
				WRITE_VARO_STR(dest, str);
			} else
				WRITE_VARO_UINT8(dest + i, _vm->_game->_script->getResultInt());
			break;

		case TYPE_VAR_INT32_AS_INT16:
		case TYPE_ARRAY_INT16:
			WRITE_VARO_UINT16(dest + i * 2, _vm->_game->_script->getResultInt());
			break;

		case TYPE_VAR_INT32:
		case TYPE_ARRAY_INT32:
			WRITE_VAR_OFFSET(dest + i * 4, _vm->_game->_script->getResultInt());
			break;

		case TYPE_VAR_STR:
		case TYPE_ARRAY_STR:
			if (srcType == TYPE_IMM_INT16)
				WRITE_VARO_UINT8(dest, result);
			else
				WRITE_VARO_STR(dest, _vm->_game->_script->getResultStr());
			break;
		}
	}
}

void Inter_Fascination::oFascin_copySprite(OpFuncParams &params) {
	_vm->_draw->_sourceSurface = _vm->_game->_script->readInt16();
	_vm->_draw->_destSurface = _vm->_game->_script->readInt16();
	_vm->_draw->_spriteLeft = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteTop = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteRight = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteBottom = _vm->_game->_script->readValExpr();

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();

	_vm->_draw->_transparency = _vm->_game->_script->readInt16();

	_vm->_draw->spriteOperation(DRAW_BLITSURF);
}

void Inter_Fascination::oFascin_playTirb(OpGobParams &params) {
	VideoPlayer::Properties vidProps;

	vidProps.type   = VideoPlayer::kVideoTypePreIMD;
	vidProps.sprite = Draw::kFrontSurface;
	vidProps.x      = 150;
	vidProps.y      =  88;
	vidProps.width  = 128;
	vidProps.height =  80;

	int vidSlot = _vm->_vidPlayer->openVideo(true, "tirb", vidProps);
	if (vidSlot < 0)
		return;

	_vm->_vidPlayer->play(vidSlot, vidProps);
	_vm->_vidPlayer->closeVideo(vidSlot);
}

void Inter_Fascination::oFascin_playTira(OpGobParams &params) {
	VideoPlayer::Properties vidProps;

	vidProps.type   = VideoPlayer::kVideoTypePreIMD;
	vidProps.sprite = Draw::kFrontSurface;
	vidProps.x      =  88;
	vidProps.y      =  66;
	vidProps.width  = 128;
	vidProps.height =  80;

	int vidSlot = _vm->_vidPlayer->openVideo(true, "tira", vidProps);
	if (vidSlot < 0)
		return;

	_vm->_vidPlayer->play(vidSlot, vidProps);
	_vm->_vidPlayer->closeVideo(vidSlot);
}

void Inter_Fascination::oFascin_loadExtasy(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("extasy.tbr");
	_vm->_sound->adlibLoadMDY("extasy.mdy");
	_vm->_sound->adlibSetRepeating(-1);
}

void Inter_Fascination::oFascin_adlibPlay(OpGobParams &params) {
	_vm->_sound->adlibPlay();
}

void Inter_Fascination::oFascin_adlibStop(OpGobParams &params) {
	_vm->_sound->adlibStop();
}

void Inter_Fascination::oFascin_adlibUnload(OpGobParams &params) {
	_vm->_sound->adlibUnload();
}

void Inter_Fascination::oFascin_loadMus1(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("music1.tbr");
	_vm->_sound->adlibLoadMDY("music1.mdy");
}

void Inter_Fascination::oFascin_loadMus2(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("music2.tbr");
	_vm->_sound->adlibLoadMDY("music2.mdy");
}

void Inter_Fascination::oFascin_loadMus3(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("music3.tbr");
	_vm->_sound->adlibLoadMDY("music3.mdy");
}

void Inter_Fascination::oFascin_loadBatt1(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("batt1.tbr");
	_vm->_sound->adlibLoadMDY("batt1.mdy");
}

void Inter_Fascination::oFascin_loadBatt2(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("batt2.tbr");
	_vm->_sound->adlibLoadMDY("batt2.mdy");
}

void Inter_Fascination::oFascin_loadBatt3(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("batt3.tbr");
	_vm->_sound->adlibLoadMDY("batt3.mdy");
}

void Inter_Fascination::oFascin_loadMod(OpGobParams &params) {
	// Fascination GE Function 1000 - Load MOD music.
	// Only used by Amiga and Atari versions.
	// Useless as it's included in Paula's playProTracker
}

void Inter_Fascination::oFascin_setWinSize() {
	_vm->_draw->_winMaxWidth  = _vm->_game->_script->readUint16();
	_vm->_draw->_winMaxHeight = _vm->_game->_script->readUint16();
	_vm->_draw->_winVarArrayLeft   = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayTop    = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayWidth  = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayHeight = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayStatus = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayLimitsX = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayLimitsY = _vm->_game->_script->readVarIndex();
}

void Inter_Fascination::oFascin_closeWin() {
	int16 id;
	_vm->_game->_script->evalExpr(&id);
	_vm->_draw->activeWin(id);
	_vm->_draw->closeWin(id);
}

void Inter_Fascination::oFascin_activeWin() {
	int16 id;
	_vm->_game->_script->evalExpr(&id);
	_vm->_draw->activeWin(id);
}

void Inter_Fascination::oFascin_openWin() {
	int16 retVal, id;
	_vm->_game->_script->evalExpr(&id);
	retVal = _vm->_game->_script->readVarIndex();
	WRITE_VAR((retVal / 4), (int32) _vm->_draw->openWin(id));
}

void Inter_Fascination::oFascin_initCursorAnim() {
	int16 ind = _vm->_game->_script->readValExpr();
	_vm->_draw->_cursorAnimLow[ind] = _vm->_game->_script->readInt16();
	_vm->_draw->_cursorAnimHigh[ind] = _vm->_game->_script->readInt16();
	_vm->_draw->_cursorAnimDelays[ind] = _vm->_game->_script->readInt16();
}

void Inter_Fascination::oFascin_setRenderFlags() {
	int16 expr;
	_vm->_game->_script->evalExpr(&expr);
	_vm->_draw->_renderFlags = expr;
}

void Inter_Fascination::oFascin_setWinFlags() {
	int16 expr;
	_vm->_game->_script->evalExpr(&expr);
	_vm->_global->_curWinId = expr;
}

void Inter_Fascination::oFascin_playProtracker(OpGobParams &params) {
	_vm->_sound->protrackerPlay("mod.extasy");
}

} // End of namespace Gob
