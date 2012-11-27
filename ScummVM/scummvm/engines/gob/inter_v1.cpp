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

#include "common/str.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/expression.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/hotspots.h"
#include "gob/goblin.h"
#include "gob/inter.h"
#include "gob/map.h"
#include "gob/mult.h"
#include "gob/palanim.h"
#include "gob/scenery.h"
#include "gob/video.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODEVER Inter_v1
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_v1::Inter_v1(GobEngine *vm) : Inter(vm) {
}

void Inter_v1::setupOpcodesDraw() {
	OPCODEDRAW(0x00, o1_loadMult);
	OPCODEDRAW(0x01, o1_playMult);
	OPCODEDRAW(0x02, o1_freeMultKeys);

	OPCODEDRAW(0x07, o1_initCursor);

	OPCODEDRAW(0x08, o1_initCursorAnim);
	OPCODEDRAW(0x09, o1_clearCursorAnim);
	OPCODEDRAW(0x0A, o1_setRenderFlags);

	OPCODEDRAW(0x10, o1_loadAnim);
	OPCODEDRAW(0x11, o1_freeAnim);
	OPCODEDRAW(0x12, o1_updateAnim);

	OPCODEDRAW(0x14, o1_initMult);
	OPCODEDRAW(0x15, o1_freeMult);
	OPCODEDRAW(0x16, o1_animate);
	OPCODEDRAW(0x17, o1_loadMultObject);

	OPCODEDRAW(0x18, o1_getAnimLayerInfo);
	OPCODEDRAW(0x19, o1_getObjAnimSize);
	OPCODEDRAW(0x1A, o1_loadStatic);
	OPCODEDRAW(0x1B, o1_freeStatic);

	OPCODEDRAW(0x1C, o1_renderStatic);
	OPCODEDRAW(0x1D, o1_loadCurLayer);

	OPCODEDRAW(0x20, o1_playCDTrack);
	OPCODEDRAW(0x21, o1_getCDTrackPos);
	OPCODEDRAW(0x22, o1_stopCD);

	OPCODEDRAW(0x30, o1_loadFontToSprite);
	OPCODEDRAW(0x31, o1_freeFontToSprite);
}

void Inter_v1::setupOpcodesFunc() {
	OPCODEFUNC(0x00, o1_callSub);
	OPCODEFUNC(0x01, o1_callSub);
	OPCODEFUNC(0x02, o1_printTotText);
	OPCODEFUNC(0x03, o1_loadCursor);

	OPCODEFUNC(0x05, o1_switch);
	OPCODEFUNC(0x06, o1_repeatUntil);
	OPCODEFUNC(0x07, o1_whileDo);

	OPCODEFUNC(0x08, o1_if);
	OPCODEFUNC(0x09, o1_assign);
	OPCODEFUNC(0x0A, o1_loadSpriteToPos);
	OPCODEFUNC(0x11, o1_printText);
	OPCODEFUNC(0x12, o1_loadTot);
	OPCODEFUNC(0x13, o1_palLoad);

	OPCODEFUNC(0x14, o1_keyFunc);
	OPCODEFUNC(0x15, o1_capturePush);
	OPCODEFUNC(0x16, o1_capturePop);
	OPCODEFUNC(0x17, o1_animPalInit);

	OPCODEFUNC(0x1E, o1_drawOperations);
	OPCODEFUNC(0x1F, o1_setcmdCount);

	OPCODEFUNC(0x20, o1_return);
	OPCODEFUNC(0x21, o1_renewTimeInVars);
	OPCODEFUNC(0x22, o1_speakerOn);
	OPCODEFUNC(0x23, o1_speakerOff);

	OPCODEFUNC(0x24, o1_putPixel);
	OPCODEFUNC(0x25, o1_goblinFunc);
	OPCODEFUNC(0x26, o1_createSprite);
	OPCODEFUNC(0x27, o1_freeSprite);

	OPCODEFUNC(0x30, o1_returnTo);
	OPCODEFUNC(0x31, o1_loadSpriteContent);
	OPCODEFUNC(0x32, o1_copySprite);
	OPCODEFUNC(0x33, o1_fillRect);

	OPCODEFUNC(0x34, o1_drawLine);
	OPCODEFUNC(0x35, o1_strToLong);
	OPCODEFUNC(0x36, o1_invalidate);
	OPCODEFUNC(0x37, o1_setBackDelta);

	OPCODEFUNC(0x38, o1_playSound);
	OPCODEFUNC(0x39, o1_stopSound);
	OPCODEFUNC(0x3A, o1_loadSound);
	OPCODEFUNC(0x3B, o1_freeSoundSlot);

	OPCODEFUNC(0x3C, o1_waitEndPlay);
	OPCODEFUNC(0x3D, o1_playComposition);
	OPCODEFUNC(0x3E, o1_getFreeMem);
	OPCODEFUNC(0x3F, o1_checkData);

	OPCODEFUNC(0x41, o1_cleanupStr);
	OPCODEFUNC(0x42, o1_insertStr);
	OPCODEFUNC(0x43, o1_cutStr);

	OPCODEFUNC(0x44, o1_strstr);
	OPCODEFUNC(0x45, o1_istrlen);
	OPCODEFUNC(0x46, o1_setMousePos);
	OPCODEFUNC(0x47, o1_setFrameRate);

	OPCODEFUNC(0x48, o1_animatePalette);
	OPCODEFUNC(0x49, o1_animateCursor);
	OPCODEFUNC(0x4A, o1_blitCursor);
	OPCODEFUNC(0x4B, o1_loadFont);

	OPCODEFUNC(0x4C, o1_freeFont);
	OPCODEFUNC(0x4D, o1_readData);
	OPCODEFUNC(0x4E, o1_writeData);
	OPCODEFUNC(0x4F, o1_manageDataFile);
}

void Inter_v1::setupOpcodesGob() {
	OPCODEGOB(   1, o1_setState);
	OPCODEGOB(   2, o1_setCurFrame);
	OPCODEGOB(   3, o1_setNextState);
	OPCODEGOB(   4, o1_setMultState);
	OPCODEGOB(   5, o1_setOrder);
	OPCODEGOB(   6, o1_setActionStartState);
	OPCODEGOB(   7, o1_setCurLookDir);
	OPCODEGOB(   8, o1_setType);
	OPCODEGOB(   9, o1_setNoTick);
	OPCODEGOB(  10, o1_setPickable);
	OPCODEGOB(  12, o1_setXPos);
	OPCODEGOB(  13, o1_setYPos);
	OPCODEGOB(  14, o1_setDoAnim);
	OPCODEGOB(  15, o1_setRelaxTime);
	OPCODEGOB(  16, o1_setMaxTick);
	OPCODEGOB(  21, o1_getState);
	OPCODEGOB(  22, o1_getCurFrame);
	OPCODEGOB(  23, o1_getNextState);
	OPCODEGOB(  24, o1_getMultState);
	OPCODEGOB(  25, o1_getOrder);
	OPCODEGOB(  26, o1_getActionStartState);
	OPCODEGOB(  27, o1_getCurLookDir);
	OPCODEGOB(  28, o1_getType);
	OPCODEGOB(  29, o1_getNoTick);
	OPCODEGOB(  30, o1_getPickable);
	OPCODEGOB(  32, o1_getObjMaxFrame);
	OPCODEGOB(  33, o1_getXPos);
	OPCODEGOB(  34, o1_getYPos);
	OPCODEGOB(  35, o1_getDoAnim);
	OPCODEGOB(  36, o1_getRelaxTime);
	OPCODEGOB(  37, o1_getMaxTick);
	OPCODEGOB(  40, o1_manipulateMap);
	OPCODEGOB(  41, o1_getItem);
	OPCODEGOB(  42, o1_manipulateMapIndirect);
	OPCODEGOB(  43, o1_getItemIndirect);
	OPCODEGOB(  44, o1_setPassMap);
	OPCODEGOB(  50, o1_setGoblinPosH);
	OPCODEGOB(  52, o1_getGoblinPosXH);
	OPCODEGOB(  53, o1_getGoblinPosYH);
	OPCODEGOB( 150, o1_setGoblinMultState);
	OPCODEGOB( 152, o1_setGoblinUnk14);
	OPCODEGOB( 200, o1_setItemIdInPocket);
	OPCODEGOB( 201, o1_setItemIndInPocket);
	OPCODEGOB( 202, o1_getItemIdInPocket);
	OPCODEGOB( 203, o1_getItemIndInPocket);
	OPCODEGOB( 204, o1_setItemPos);
	OPCODEGOB( 250, o1_setGoblinPos);
	OPCODEGOB( 251, o1_setGoblinState);
	OPCODEGOB( 252, o1_setGoblinStateRedraw);
	OPCODEGOB( 500, o1_decRelaxTime);
	OPCODEGOB( 502, o1_getGoblinPosX);
	OPCODEGOB( 503, o1_getGoblinPosY);
	OPCODEGOB( 600, o1_clearPathExistence);
	OPCODEGOB( 601, o1_setGoblinVisible);
	OPCODEGOB( 602, o1_setGoblinInvisible);
	OPCODEGOB( 603, o1_getObjectIntersect);
	OPCODEGOB( 604, o1_getGoblinIntersect);
	OPCODEGOB( 605, o1_setItemPos);
	OPCODEGOB(1000, o1_loadObjects);
	OPCODEGOB(1001, o1_freeObjects);
	OPCODEGOB(1002, o1_animateObjects);
	OPCODEGOB(1003, o1_drawObjects);
	OPCODEGOB(1004, o1_loadMap);
	OPCODEGOB(1005, o1_moveGoblin);
	OPCODEGOB(1006, o1_switchGoblin);
	OPCODEGOB(1008, o1_loadGoblin);
	OPCODEGOB(1009, o1_writeTreatItem);
	OPCODEGOB(1010, o1_moveGoblin0);
	OPCODEGOB(1011, o1_setGoblinTarget);
	OPCODEGOB(1015, o1_setGoblinObjectsPos);
	OPCODEGOB(2005, o1_initGoblin);
}

void Inter_v1::checkSwitchTable(uint32 &offset) {
	int16 len;
	int32 value;
	bool found;
	bool notFound;

	found = false;
	notFound = true;
	offset = 0;
	value = VAR_OFFSET(_vm->_game->_script->readVarIndex());

	len = _vm->_game->_script->readInt8();
	while (len != -5) {
		for (int i = 0; i < len; i++) {
			_vm->_game->_script->evalExpr(0);

			if (_terminate)
				return;

			if (_vm->_game->_script->getResultInt() == value) {
				found = true;
				notFound = false;
			}
		}

		if (found)
			offset = _vm->_game->_script->pos();

		_vm->_game->_script->skip(_vm->_game->_script->peekUint16(2) + 2);
		found = false;
		len = _vm->_game->_script->readInt8();
	}

	if ((_vm->_game->_script->peekByte() >> 4) != 4)
		return;

	_vm->_game->_script->skip(1);
	if (notFound)
		offset = _vm->_game->_script->pos();

	_vm->_game->_script->skip(_vm->_game->_script->peekUint16(2) + 2);
}

void Inter_v1::o1_loadMult() {
	_vm->_mult->loadMult(_vm->_game->_script->readInt16());
}

void Inter_v1::o1_playMult() {
	// NOTE: The EGA version of Gobliiins has an MDY tune.
	//       While the original doesn't play it, we do.
	bool isGob1EGAIntro = _vm->getGameType() == kGameTypeGob1  &&
	                      _vm->isEGA()                         &&
	                      _vm->_game->_script->pos() == 1010   &&
	                      _vm->isCurrentTot("intro.tot")       &&
	                      VAR(57) != 0xFFFFFFFF                &&
	                      _vm->_dataIO->hasFile("goblins.mdy") &&
	                      _vm->_dataIO->hasFile("goblins.tbr");

	int16 checkEscape = _vm->_game->_script->readInt16();

	if (isGob1EGAIntro) {
		_vm->_sound->adlibLoadTBR("goblins.tbr");
		_vm->_sound->adlibLoadMDY("goblins.mdy");
		_vm->_sound->adlibSetRepeating(-1);

		_vm->_sound->adlibPlay();
	}

	_vm->_mult->playMult(VAR(57), -1, checkEscape, 0);

	if (isGob1EGAIntro) {

		// User didn't escape the intro mult, wait for an escape here
		if (VAR(57) != 0xFFFFFFFF) {
			while (_vm->_util->getKey() != kKeyEscape) {
				_vm->_util->processInput();
				_vm->_util->longDelay(1);
			}
		}

		_vm->_sound->adlibUnload();
	}
}

void Inter_v1::o1_freeMultKeys() {
	_vm->_game->_script->readInt16();
	_vm->_mult->freeMultKeys();
}

void Inter_v1::o1_initCursor() {
	int16 width;
	int16 height;
	int16 count;

	_vm->_draw->_cursorHotspotXVar = ((uint16) _vm->_game->_script->readVarIndex()) / 4;
	_vm->_draw->_cursorHotspotYVar = ((uint16) _vm->_game->_script->readVarIndex()) / 4;

	width = _vm->_game->_script->readInt16();
	if (width < 16)
		width = 16;

	height = _vm->_game->_script->readInt16();
	if (height < 16)
		height = 16;

	_vm->_draw->adjustCoords(0, &width, &height);

	count = _vm->_game->_script->readInt16();

	if (count < 2)
		count = 2;

	if ((width != _vm->_draw->_cursorWidth) ||
			(height != _vm->_draw->_cursorHeight) ||
	    (_vm->_draw->_cursorSprites->getWidth() != (width * count))) {

		_vm->_draw->freeSprite(Draw::kCursorSurface);
		_vm->_draw->_cursorSprites.reset();
		_vm->_draw->_cursorSpritesBack.reset();
		_vm->_draw->_scummvmCursor.reset();

		_vm->_draw->_cursorWidth = width;
		_vm->_draw->_cursorHeight = height;

		if (count < 0x80)
			_vm->_draw->_transparentCursor = 1;
		else
			_vm->_draw->_transparentCursor = 0;

		if (count > 0x80)
			count -= 0x80;

		_vm->_draw->initSpriteSurf(Draw::kCursorSurface, _vm->_draw->_cursorWidth * count,
				_vm->_draw->_cursorHeight, 2);
		_vm->_draw->_cursorSpritesBack = _vm->_draw->_spritesArray[Draw::kCursorSurface];
		_vm->_draw->_cursorSprites = _vm->_draw->_cursorSpritesBack;

		_vm->_draw->_scummvmCursor =
			_vm->_video->initSurfDesc(_vm->_draw->_cursorWidth,
					_vm->_draw->_cursorHeight, SCUMMVM_CURSOR);

		for (int i = 0; i < 40; i++) {
			_vm->_draw->_cursorAnimLow[i] = -1;
			_vm->_draw->_cursorAnimDelays[i] = 0;
			_vm->_draw->_cursorAnimHigh[i] = 0;
		}
		_vm->_draw->_cursorAnimLow[1] = 0;
	}
}

void Inter_v1::o1_initCursorAnim() {
	int16 ind;

	_vm->_draw->_showCursor = 3;
	ind = _vm->_game->_script->readValExpr();
	_vm->_draw->_cursorAnimLow[ind] = _vm->_game->_script->readInt16();
	_vm->_draw->_cursorAnimHigh[ind] = _vm->_game->_script->readInt16();
	_vm->_draw->_cursorAnimDelays[ind] = _vm->_game->_script->readInt16();
}

void Inter_v1::o1_clearCursorAnim() {
	int16 ind;

	_vm->_draw->_showCursor = 0;
	ind = _vm->_game->_script->readValExpr();
	_vm->_draw->_cursorAnimLow[ind] = -1;
	_vm->_draw->_cursorAnimHigh[ind] = 0;
	_vm->_draw->_cursorAnimDelays[ind] = 0;
}

void Inter_v1::o1_setRenderFlags() {
	_vm->_draw->_renderFlags = _vm->_game->_script->readValExpr();
}

void Inter_v1::o1_loadAnim() {
	_vm->_scenery->loadAnim(0);
}

void Inter_v1::o1_freeAnim() {
	_vm->_scenery->freeAnim(-1);
}

void Inter_v1::o1_updateAnim() {
	int16 deltaX;
	int16 deltaY;
	int16 flags;
	int16 frame;
	int16 layer;
	int16 animation;

	_vm->_game->_script->evalExpr(&deltaX);
	_vm->_game->_script->evalExpr(&deltaY);
	_vm->_game->_script->evalExpr(&animation);
	_vm->_game->_script->evalExpr(&layer);
	_vm->_game->_script->evalExpr(&frame);
	flags = _vm->_game->_script->readInt16();
	_vm->_scenery->updateAnim(layer, frame, animation, flags,
			deltaX, deltaY, 1);
}

void Inter_v1::o1_initMult() {
	int16 oldAnimHeight;
	int16 oldAnimWidth;
	int16 oldObjCount;
	int16 posXVar;
	int16 posYVar;
	int16 animDataVar;

	oldAnimWidth = _vm->_mult->_animWidth;
	oldAnimHeight = _vm->_mult->_animHeight;
	oldObjCount = _vm->_mult->_objCount;

	_vm->_mult->_animLeft = _vm->_game->_script->readInt16();
	_vm->_mult->_animTop = _vm->_game->_script->readInt16();
	_vm->_mult->_animWidth = _vm->_game->_script->readInt16();
	_vm->_mult->_animHeight = _vm->_game->_script->readInt16();
	_vm->_mult->_objCount = _vm->_game->_script->readInt16();
	posXVar = _vm->_game->_script->readVarIndex();
	posYVar = _vm->_game->_script->readVarIndex();
	animDataVar = _vm->_game->_script->readVarIndex();

	if (_vm->_mult->_objects && (oldObjCount != _vm->_mult->_objCount)) {
		warning("Initializing new objects without having "
				"cleaned up the old ones at first");

		for (int i = 0; i < _vm->_mult->_objCount; i++) {
			delete _vm->_mult->_objects[i].pPosX;
			delete _vm->_mult->_objects[i].pPosY;
		}

		delete[] _vm->_mult->_objects;
		delete[] _vm->_mult->_renderData;

		_vm->_mult->_objects = 0;
		_vm->_mult->_renderObjs = 0;

	}

	if (_vm->_mult->_objects == 0) {
		_vm->_mult->_renderData = new int16[_vm->_mult->_objCount * 9];
		memset(_vm->_mult->_renderData, 0,
				_vm->_mult->_objCount * 9 * sizeof(int16));
		_vm->_mult->_objects = new Mult::Mult_Object[_vm->_mult->_objCount];
		memset(_vm->_mult->_objects, 0,
				_vm->_mult->_objCount * sizeof(Mult::Mult_Object));

		for (int i = 0; i < _vm->_mult->_objCount; i++) {
			uint32 offPosX = i * 4 + (posXVar / 4) * 4;
			uint32 offPosY = i * 4 + (posYVar / 4) * 4;
			uint32 offAnim = animDataVar + i * 4 * _vm->_global->_inter_animDataSize;

			_vm->_mult->_objects[i].pPosX = new VariableReference(*_variables, offPosX);
			_vm->_mult->_objects[i].pPosY = new VariableReference(*_variables, offPosY);

			_vm->_mult->_objects[i].pAnimData =
				(Mult::Mult_AnimData *)_variables->getAddressOff8(offAnim);

			_vm->_mult->_objects[i].pAnimData->isStatic = 1;
			_vm->_mult->_objects[i].tick = 0;
			_vm->_mult->_objects[i].lastLeft = -1;
			_vm->_mult->_objects[i].lastRight = -1;
			_vm->_mult->_objects[i].lastTop = -1;
			_vm->_mult->_objects[i].lastBottom = -1;
		}
	}

	if (_vm->_mult->_animSurf &&
	    ((oldAnimWidth != _vm->_mult->_animWidth) ||
			 (oldAnimHeight != _vm->_mult->_animHeight))) {
		_vm->_draw->freeSprite(Draw::kAnimSurface);
		_vm->_mult->_animSurf.reset();
	}

	if (!_vm->_mult->_animSurf) {
		_vm->_draw->initSpriteSurf(Draw::kAnimSurface, _vm->_mult->_animWidth,
				_vm->_mult->_animHeight, 0);
		_vm->_mult->_animSurf = _vm->_draw->_spritesArray[Draw::kAnimSurface];
	}

	_vm->_mult->_animSurf->blit(*_vm->_draw->_backSurface,
	    _vm->_mult->_animLeft, _vm->_mult->_animTop,
	    _vm->_mult->_animLeft + _vm->_mult->_animWidth - 1,
	    _vm->_mult->_animTop + _vm->_mult->_animHeight - 1, 0, 0);

	debugC(4, kDebugGraphics, "o1_initMult: x = %d, y = %d, w = %d, h = %d",
		  _vm->_mult->_animLeft, _vm->_mult->_animTop,
			_vm->_mult->_animWidth, _vm->_mult->_animHeight);
	debugC(4, kDebugGraphics, "    _vm->_mult->_objCount = %d, "
			"animation data size = %d", _vm->_mult->_objCount,
			_vm->_global->_inter_animDataSize);
}

void Inter_v1::o1_freeMult() {
	_vm->_mult->freeMult();
}

void Inter_v1::o1_animate() {
	_vm->_mult->animate();
}

void Inter_v1::o1_loadMultObject() {
	int16 val;
	int16 objIndex;
	byte *multData;

	_vm->_game->_script->evalExpr(&objIndex);
	_vm->_game->_script->evalExpr(&val);
	*_vm->_mult->_objects[objIndex].pPosX = val;
	_vm->_game->_script->evalExpr(&val);
	*_vm->_mult->_objects[objIndex].pPosY = val;

	debugC(4, kDebugGameFlow, "Loading mult object %d", objIndex);

	multData = (byte *)_vm->_mult->_objects[objIndex].pAnimData;
	for (int i = 0; i < 11; i++) {
		if (_vm->_game->_script->peekUint16() != 99) {
			_vm->_game->_script->evalExpr(&val);
			multData[i] = val;
		} else
			_vm->_game->_script->skip(1);
	}
}

void Inter_v1::o1_getAnimLayerInfo() {
	int16 anim;
	int16 layer;
	int16 varDX, varDY;
	int16 varUnk0;
	int16 varFrames;

	_vm->_game->_script->evalExpr(&anim);
	_vm->_game->_script->evalExpr(&layer);

	varDX = _vm->_game->_script->readVarIndex();
	varDY = _vm->_game->_script->readVarIndex();
	varUnk0 = _vm->_game->_script->readVarIndex();
	varFrames = _vm->_game->_script->readVarIndex();

	_vm->_scenery->writeAnimLayerInfo(anim, layer,
			varDX, varDY, varUnk0, varFrames);
}

void Inter_v1::o1_getObjAnimSize() {
	int16 objIndex;

	_vm->_game->_script->evalExpr(&objIndex);

	uint16 varLeft   = _vm->_game->_script->readVarIndex();
	uint16 varTop    = _vm->_game->_script->readVarIndex();
	uint16 varRight  = _vm->_game->_script->readVarIndex();
	uint16 varBottom = _vm->_game->_script->readVarIndex();

	if ((objIndex < 0) || (objIndex >= _vm->_mult->_objCount)) {
		warning("o1_getObjAnimSize(): objIndex = %d (%d)", objIndex, _vm->_mult->_objCount);
		_vm->_scenery->_toRedrawLeft   = 0;
		_vm->_scenery->_toRedrawTop    = 0;
		_vm->_scenery->_toRedrawRight  = 0;
		_vm->_scenery->_toRedrawBottom = 0;
	} else {
		Mult::Mult_AnimData &animData = *(_vm->_mult->_objects[objIndex].pAnimData);
		if (animData.isStatic == 0)
			_vm->_scenery->updateAnim(animData.layer, animData.frame,
					animData.animation, 0, *(_vm->_mult->_objects[objIndex].pPosX),
					*(_vm->_mult->_objects[objIndex].pPosY), 0);

		_vm->_scenery->_toRedrawLeft = MAX<int16>(_vm->_scenery->_toRedrawLeft, 0);
		_vm->_scenery->_toRedrawTop  = MAX<int16>(_vm->_scenery->_toRedrawTop , 0);
	}

	WRITE_VAR_OFFSET(varLeft  , _vm->_scenery->_toRedrawLeft);
	WRITE_VAR_OFFSET(varTop   , _vm->_scenery->_toRedrawTop);
	WRITE_VAR_OFFSET(varRight , _vm->_scenery->_toRedrawRight);
	WRITE_VAR_OFFSET(varBottom, _vm->_scenery->_toRedrawBottom);
}

void Inter_v1::o1_loadStatic() {
	_vm->_scenery->loadStatic(0);
}

void Inter_v1::o1_freeStatic() {
	_vm->_scenery->freeStatic(-1);
}

void Inter_v1::o1_renderStatic() {
	int16 layer;
	int16 index;

	_vm->_game->_script->evalExpr(&index);
	_vm->_game->_script->evalExpr(&layer);
	_vm->_scenery->renderStatic(index, layer);
}

void Inter_v1::o1_loadCurLayer() {
	_vm->_game->_script->evalExpr(&_vm->_scenery->_curStatic);
	_vm->_game->_script->evalExpr(&_vm->_scenery->_curStaticLayer);
}

void Inter_v1::o1_playCDTrack() {
	_vm->_sound->adlibPlayBgMusic(); // Mac version
	_vm->_sound->cdPlay(_vm->_game->_script->evalString()); // PC CD version
}

void Inter_v1::o1_getCDTrackPos() {
	// Used in gob1 CD

	// Some scripts busy-wait while calling this opcode.
	// This is a very nasty thing to do, so let's add a
	// short delay here. It's probably a safe thing to do.

	_vm->_util->longDelay(1);

	int pos = _vm->_sound->cdGetTrackPos();
	if (pos == -1)
		pos = 32767;
	WRITE_VAR(5, pos);
}

void Inter_v1::o1_stopCD() {
	_vm->_sound->adlibStop(); // Mac version
	_vm->_sound->cdStop(); // PC CD version
}

void Inter_v1::o1_loadFontToSprite() {
	int16 i = _vm->_game->_script->readInt16();
	_vm->_draw->_fontToSprite[i].sprite = _vm->_game->_script->readInt16();
	_vm->_draw->_fontToSprite[i].base = _vm->_game->_script->readInt16();
	_vm->_draw->_fontToSprite[i].width = _vm->_game->_script->readInt16();
	_vm->_draw->_fontToSprite[i].height = _vm->_game->_script->readInt16();
}

void Inter_v1::o1_freeFontToSprite() {
	int16 i = _vm->_game->_script->readInt16();
	_vm->_draw->_fontToSprite[i].sprite = -1;
	_vm->_draw->_fontToSprite[i].base = -1;
	_vm->_draw->_fontToSprite[i].width = -1;
	_vm->_draw->_fontToSprite[i].height = -1;
}

void Inter_v1::o1_callSub(OpFuncParams &params) {
	uint16 offset = _vm->_game->_script->readUint16();

	debugC(5, kDebugGameFlow, "tot = \"%s\", offset = %d",
			_vm->_game->_curTotFile.c_str(), offset);

	if (offset < 128) {
		warning("Inter_v1::o1_callSub(): Offset %d points into the header. "
				"Skipping call", offset);
		return;
	}

	// Skipping the copy protection screen in Gobliiins
	if (!_vm->_copyProtection && (_vm->getGameType() == kGameTypeGob1) && (offset == 3905) &&
	    _vm->isCurrentTot(_vm->_startTot)) {
		debugC(2, kDebugGameFlow, "Skipping copy protection screen");
		return;
	}
	// Skipping the copy protection screen in Gobliins 2
	if (!_vm->_copyProtection && (_vm->getGameType() == kGameTypeGob2) && (offset == 1746) &&
	    _vm->isCurrentTot("intro0.tot")) {
		debugC(2, kDebugGameFlow, "Skipping copy protection screen");
		return;
	}

	_vm->_game->_script->call(offset);

	if ((params.counter == params.cmdCount) && (params.retFlag == 2)) {
		_vm->_game->_script->pop(false);
		params.doReturn = true;
		return;
	}

	callSub(2);

	_vm->_game->_script->pop();
}

void Inter_v1::o1_printTotText(OpFuncParams &params) {
	_vm->_draw->printTotText(_vm->_game->_script->readInt16());
}

void Inter_v1::o1_loadCursor(OpFuncParams &params) {
	int16 id    = _vm->_game->_script->readInt16();
	int8  index = _vm->_game->_script->readInt8();

	if ((index * _vm->_draw->_cursorWidth) >= _vm->_draw->_cursorSprites->getWidth())
		return;

	Resource *resource = _vm->_game->_resources->getResource(id);
	if (!resource)
		return;

	_vm->_draw->_cursorSprites->fillRect(index * _vm->_draw->_cursorWidth, 0,
			index * _vm->_draw->_cursorWidth + _vm->_draw->_cursorWidth - 1,
			_vm->_draw->_cursorHeight - 1, 0);

	_vm->_video->drawPackedSprite(resource->getData(),
			resource->getWidth(), resource->getHeight(),
			index * _vm->_draw->_cursorWidth, 0, 0, *_vm->_draw->_cursorSprites);
	_vm->_draw->_cursorAnimLow[index] = 0;

	delete resource;
}

void Inter_v1::o1_switch(OpFuncParams &params) {
	uint32 offset;

	checkSwitchTable(offset);

	_vm->_game->_script->call(offset);

	if (offset == 0)
		_vm->_game->_script->setFinished(true);

	if ((params.counter == params.cmdCount) && (params.retFlag == 2)) {
		_vm->_game->_script->pop(false);
		params.doReturn = true;
		return;
	}

	funcBlock(0);

	_vm->_game->_script->pop();
}

void Inter_v1::o1_repeatUntil(OpFuncParams &params) {
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
	} while (!flag && !_break && !_terminate && !_vm->shouldQuit());

	_nestLevel[0]--;

	if (*_breakFromLevel > -1) {
		_break = false;
		*_breakFromLevel = -1;
	}
}

void Inter_v1::o1_whileDo(OpFuncParams &params) {
	bool flag;
	int16 size;

	_nestLevel[0]++;
	do {
		uint32 startPos = _vm->_game->_script->pos();

		flag = _vm->_game->_script->evalBool();

		if (_terminate)
			return;

		uint32 blockPos = _vm->_game->_script->pos();

		size = _vm->_game->_script->peekUint16(2) + 2;

		if (flag) {
			funcBlock(1);
			_vm->_game->_script->seek(startPos);
		} else
			_vm->_game->_script->skip(size);

		if (_break || _terminate || _vm->shouldQuit()) {
			_vm->_game->_script->seek(blockPos);
			_vm->_game->_script->skip(size);
			break;
		}
	} while (flag);

	_nestLevel[0]--;
	if (*_breakFromLevel > -1) {
		_break = false;
		*_breakFromLevel = -1;
	}
}

void Inter_v1::o1_if(OpFuncParams &params) {
	byte cmd;
	bool boolRes;

	// WORKAROUND: Gob1 goblin stuck on reload bugs present in original - bugs #3018918 and 3065914
	if ((_vm->getGameType() == kGameTypeGob1) && (_vm->_game->_script->pos() == 2933) &&
			_vm->isCurrentTot("inter.tot") && VAR(285) != 0) {
		warning("Workaround for Gob1 Goblin Stuck On Reload Bug applied...");
		// VAR(59) actually locks goblin movement, but these variables trigger this in the script.
		WRITE_VAR(285, 0);
	}

	boolRes = _vm->_game->_script->evalBool();
	if (boolRes) {
		if ((params.counter == params.cmdCount) && (params.retFlag == 2)) {
			params.doReturn = true;
			return;
		}

		_vm->_game->_script->push();
		funcBlock(0);
		_vm->_game->_script->pop();

		_vm->_game->_script->skip(_vm->_game->_script->peekUint16(2) + 2);

		debugC(6, kDebugGameFlow, "cmd = %d", (int16) _vm->_game->_script->peekByte());

		cmd = _vm->_game->_script->readByte() >> 4;
		if (cmd != 12)
			return;

		_vm->_game->_script->skip(_vm->_game->_script->peekUint16(2) + 2);
	} else {
		_vm->_game->_script->skip(_vm->_game->_script->peekUint16(2) + 2);

		debugC(6, kDebugGameFlow, "cmd = %d", (int16) _vm->_game->_script->peekByte());

		cmd = _vm->_game->_script->readByte() >> 4;
		if (cmd != 12)
			return;

		if ((params.counter == params.cmdCount) && (params.retFlag == 2)) {
			params.doReturn = true;
			return;
		}

		_vm->_game->_script->push();
		funcBlock(0);
		_vm->_game->_script->pop();

		_vm->_game->_script->skip(_vm->_game->_script->peekUint16(2) + 2);
	}
}

void Inter_v1::o1_assign(OpFuncParams &params) {
	byte destType = _vm->_game->_script->peekByte();
	int16 dest = _vm->_game->_script->readVarIndex();

	int16 result;
	int16 srcType = _vm->_game->_script->evalExpr(&result);

	switch (destType) {
	case TYPE_VAR_INT32:
	case TYPE_ARRAY_INT32:
		WRITE_VAR_OFFSET(dest, _vm->_game->_script->getResultInt());
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

void Inter_v1::o1_loadSpriteToPos(OpFuncParams &params) {
	_vm->_draw->_spriteLeft = _vm->_game->_script->readInt16();

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();

	// WORKAROUND: The EGA version of Gobliiins 1 has an invalid expression there
	if (_vm->isEGA() && (_vm->_game->_script->pos() == 1398) &&
			_vm->isCurrentTot("intro.tot")) {

		_vm->_draw->_destSpriteY = 0;
		_vm->_game->_script->skip(1);

	} else
		_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();

	_vm->_draw->_transparency = _vm->_game->_script->peekByte() & 1;
	_vm->_draw->_destSurface = ((int16) (_vm->_game->_script->peekByte() >> 1)) - 1;
	if (_vm->_draw->_destSurface < 0)
		_vm->_draw->_destSurface = 101;

	_vm->_game->_script->skip(2);

	_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
}

void Inter_v1::o1_printText(OpFuncParams &params) {
	char buf[60];
	int i;

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();

	_vm->_draw->_backColor = _vm->_game->_script->readValExpr();
	_vm->_draw->_frontColor = _vm->_game->_script->readValExpr();
	_vm->_draw->_fontIndex = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSurface = Draw::kBackSurface;
	_vm->_draw->_textToPrint = buf;
	_vm->_draw->_transparency = 0;

	if (_vm->_draw->_backColor >= 16) {
		_vm->_draw->_backColor = 0;
		_vm->_draw->_transparency = 1;
	}

	do {
		for (i = 0; ((_vm->_game->_script->peekChar()) != '.') &&
				(_vm->_game->_script->peekByte() != 200);
				i++, _vm->_game->_script->skip(1)) {
			buf[i] = _vm->_game->_script->peekChar();
		}

		if (_vm->_game->_script->peekByte() != 200) {
			_vm->_game->_script->skip(1);
			switch (_vm->_game->_script->peekByte()) {
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

		_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
	} while (_vm->_game->_script->peekByte() != 200);

	_vm->_game->_script->skip(1);
}

void Inter_v1::o1_loadTot(OpFuncParams &params) {
	if ((_vm->_game->_script->peekByte() & 0x80) != 0) {
		_vm->_game->_script->skip(1);
		_vm->_game->_totToLoad = _vm->_game->_script->evalString();
	} else {
		uint8 size = _vm->_game->_script->readInt8();
		_vm->_game->_totToLoad = Common::String(_vm->_game->_script->readString(size), size);
	}

	_vm->_game->_totToLoad += ".tot";

	if (_terminate != 2)
		_terminate = 1;
}

void Inter_v1::o1_palLoad(OpFuncParams &params) {
	int index1, index2;
	int16 id;
	byte cmd;
	Resource *resource;

	cmd = _vm->_game->_script->readByte();
	switch (cmd & 0x7F) {
	case 48:
		if ((_vm->_global->_fakeVideoMode < 0x32) ||
				(_vm->_global->_fakeVideoMode > 0x63)) {
			_vm->_game->_script->skip(48);
			return;
		}
		break;

	case 49:
		if ((_vm->_global->_fakeVideoMode != 5) &&
				(_vm->_global->_fakeVideoMode != 7)) {
			_vm->_game->_script->skip(18);
			return;
		}
		break;

	case 50:
		if (_vm->_global->_colorCount == 256) {
			_vm->_game->_script->skip(16);
			return;
		}
		break;

	case 51:
		if (_vm->_global->_fakeVideoMode < 0x64) {
			_vm->_game->_script->skip(2);
			return;
		}
		break;

	case 52:
		if (_vm->_global->_colorCount == 256) {
			_vm->_game->_script->skip(48);
			return;
		}
		break;

	case 53:
		if (_vm->_global->_colorCount != 256) {
			_vm->_game->_script->skip(2);
			return;
		}
		break;

	case 54:
		if (_vm->_global->_fakeVideoMode < 0x13) {
			return;
		}
		break;

	case 61:
		if (_vm->_global->_fakeVideoMode < 0x13) {
			_vm->_game->_script->skip(4);
			return;
		}
		break;
	}

	if ((cmd & 0x7F) == 0x30) {
		_vm->_game->_script->skip(48);
		return;
	}

	_vm->_draw->_applyPal = !(cmd & 0x80);
	cmd &= 0x7F;

	if (cmd == 49) {
		bool allZero = true;

		for (int i = 2; i < 18; i++) {
			if (_vm->_game->_script->peekByte(i) != 0) {
				allZero = false;
				break;
			}
		}
		if (!allZero) {
			_vm->_draw->_frontSurface->clear();
			_vm->_draw->_noInvalidated57 = true;
			_vm->_game->_script->skip(48);
			return;
		}
		_vm->_draw->_noInvalidated57 = false;

		for (int i = 0; i < 18; i++) {
			if (i < 2) {
				if (!_vm->_draw->_applyPal)
					continue;

				_vm->_draw->_unusedPalette1[i] = _vm->_game->_script->peekByte();
				continue;
			}

			index1 = _vm->_game->_script->peekByte() >> 4;
			index2 = _vm->_game->_script->peekByte() & 0xF;

			_vm->_draw->_unusedPalette1[i] =
				((_vm->_draw->_palLoadData1[index1] +
					_vm->_draw->_palLoadData2[index2]) << 8) +
				(_vm->_draw->_palLoadData2[index1] +
					_vm->_draw->_palLoadData1[index2]);

			_vm->_game->_script->skip(1);
		}

		_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
		return;
	}

	switch (cmd) {
	case 50:
		for (int i = 0; i < 16; i++)
			_vm->_draw->_unusedPalette2[i] = _vm->_game->_script->readByte();
		break;

	case 52:
		for (int i = 0; i < 16; i++) {
			_vm->_draw->_vgaPalette[i].red   = _vm->_game->_script->readByte();
			_vm->_draw->_vgaPalette[i].green = _vm->_game->_script->readByte();
			_vm->_draw->_vgaPalette[i].blue  = _vm->_game->_script->readByte();
		}

		memcpy(_vm->_draw->_vgaSmallPalette, _vm->_draw->_vgaPalette, 16 * 3);
		break;

	case 53:
		resource = _vm->_game->_resources->getResource(_vm->_game->_script->readInt16());
		if (!resource)
			break;

		memcpy((char *)_vm->_draw->_vgaPalette, resource->getData(), MIN<int>(768, resource->getSize()));
		delete resource;
		break;

	case 54:
		memset((char *)_vm->_draw->_vgaPalette, 0, 768);
		break;

	case 61:
		index1 =  _vm->_game->_script->readByte();
		index2 = (_vm->_game->_script->readByte() - index1 + 1) * 3;
		id     = _vm->_game->_script->readInt16();
		resource = _vm->_game->_resources->getResource(id);
		if (!resource)
			break;

		memcpy((char *)_vm->_draw->_vgaPalette + index1 * 3,
		       resource->getData() + index1 * 3, index2);
		delete resource;

		// WORKAROUND: The Last Dynasty overwrites the 0. palette entry but depends on it staying black.
		if ((_vm->getGameType() == kGameTypeDynasty) && (index1 == 0)) {
			_vm->_draw->_vgaPalette[0].red   = 0;
			_vm->_draw->_vgaPalette[0].green = 0;
			_vm->_draw->_vgaPalette[0].blue  = 0;
		}

		if (_vm->_draw->_applyPal) {
			_vm->_draw->_applyPal = false;
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			return;
		}
		break;
	}

	if (!_vm->_draw->_applyPal) {
		_vm->_global->_pPaletteDesc->unused2 = _vm->_draw->_unusedPalette2;
		_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;

		if (_vm->_global->_videoMode < 0x13) {
			_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, 0);
			return;
		}
		if ((_vm->_global->_videoMode < 0x32) ||
				(_vm->_global->_videoMode >= 0x64)) {
			_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;
			_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, 0);
			return;
		}
		_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaSmallPalette;
		_vm->_palAnim->fade(_vm->_global->_pPaletteDesc, 0, 0);
	}
}

void Inter_v1::o1_keyFunc(OpFuncParams &params) {
	if (!_vm->_vidPlayer->isPlayingLive()) {
		_vm->_draw->forceBlit();
		_vm->_video->retrace();
	}

	animPalette();
	_vm->_draw->blitInvalidated();

	handleBusyWait();

	// WORKAROUND for bug #1726130: Ween busy-waits in the intro for a counter
	// to become 5000. We deliberately slow down busy-waiting, so we shorten
	// the counting, too.
	if ((_vm->getGameType() == kGameTypeWeen) && (VAR(59) < 4000) &&
	    (_vm->_game->_script->pos() == 729) && _vm->isCurrentTot("intro5.tot"))
		WRITE_VAR(59, 4000);

	int16 cmd = _vm->_game->_script->readInt16();
	int16 key;

	switch (cmd) {
	case -1:
		break;

	case 0:
		_vm->_draw->_showCursor &= ~2;
		_vm->_util->longDelay(1);
		key = _vm->_game->_hotspots->check(0, 0);
		storeKey(key);

		_vm->_util->clearKeyBuf();
		break;

	case 1:
		if (_vm->getGameType() != kGameTypeFascination)
			_vm->_util->forceMouseUp(true);
		key = _vm->_game->checkKeys(&_vm->_global->_inter_mouseX,
				&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons, 0);
		storeKey(key);
		break;

	case 2:
		_vm->_util->processInput(true);
		key = _vm->_util->checkKey();

		WRITE_VAR(0, key);
		_vm->_util->clearKeyBuf();
		break;

	default:
		_vm->_sound->speakerOnUpdate(cmd);
		if (cmd < 20) {
			_vm->_util->delay(cmd);
			_noBusyWait = true;
		} else
			_vm->_util->longDelay(cmd);
		break;
	}
}

void Inter_v1::o1_capturePush(OpFuncParams &params) {
	int16 left, top;
	int16 width, height;

	left = _vm->_game->_script->readValExpr();
	top = _vm->_game->_script->readValExpr();
	width = _vm->_game->_script->readValExpr();
	height = _vm->_game->_script->readValExpr();

	if ((width < 0) || (height < 0))
		return;

	_vm->_game->capturePush(left, top, width, height);
	(*_vm->_scenery->_pCaptureCounter)++;
}

void Inter_v1::o1_capturePop(OpFuncParams &params) {
	if (*_vm->_scenery->_pCaptureCounter != 0) {
		(*_vm->_scenery->_pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}
}

void Inter_v1::o1_animPalInit(OpFuncParams &params) {
	_animPalDir[0] = _vm->_game->_script->readInt16();
	_animPalLowIndex[0] = _vm->_game->_script->readValExpr();
	_animPalHighIndex[0] = _vm->_game->_script->readValExpr();
}

void Inter_v1::o1_drawOperations(OpFuncParams &params) {
	byte cmd;

	cmd = _vm->_game->_script->readByte();

	executeOpcodeDraw(cmd);
}

void Inter_v1::o1_setcmdCount(OpFuncParams &params) {
	params.cmdCount = _vm->_game->_script->readByte();
	params.counter = 0;
}

void Inter_v1::o1_return(OpFuncParams &params) {
	if (params.retFlag != 2)
		_break = true;

	_vm->_game->_script->setFinished(true);
	params.doReturn = true;
}

void Inter_v1::o1_renewTimeInVars(OpFuncParams &params) {
	renewTimeInVars();
}

void Inter_v1::o1_speakerOn(OpFuncParams &params) {
	_vm->_sound->speakerOn(_vm->_game->_script->readValExpr(), -1);
}

void Inter_v1::o1_speakerOff(OpFuncParams &params) {
	_vm->_sound->speakerOff();
}

void Inter_v1::o1_putPixel(OpFuncParams &params) {
	_vm->_draw->_destSurface = _vm->_game->_script->readInt16();

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();
	_vm->_draw->_frontColor = _vm->_game->_script->readValExpr();
	_vm->_draw->spriteOperation(DRAW_PUTPIXEL);
}

void Inter_v1::o1_goblinFunc(OpFuncParams &params) {
	OpGobParams gobParams;
	bool objDescSet = false;
	int16 cmd;

	gobParams.extraData = 0;
	gobParams.objDesc = 0;
	gobParams.retVarPtr.set(*_variables, 236);

	cmd = _vm->_game->_script->readInt16();
	gobParams.paramCount = _vm->_game->_script->readInt16();

	if ((cmd > 0) && (cmd < 17)) {
		objDescSet = true;
		gobParams.extraData = _vm->_game->_script->readInt16();
		gobParams.objDesc = _vm->_goblin->_objects[gobParams.extraData];
		gobParams.extraData = _vm->_game->_script->readInt16();
	}

	if ((cmd > 90) && (cmd < 107)) {
		objDescSet = true;
		gobParams.extraData = _vm->_game->_script->readInt16();
		gobParams.objDesc = _vm->_goblin->_goblins[gobParams.extraData];
		gobParams.extraData = _vm->_game->_script->readInt16();
		cmd -= 90;
	}

	if ((cmd > 110) && (cmd < 128)) {
		objDescSet = true;
		gobParams.extraData = _vm->_game->_script->readInt16();
		gobParams.objDesc = _vm->_goblin->_goblins[gobParams.extraData];
		cmd -= 90;
	} else if ((cmd > 20) && (cmd < 38)) {
		objDescSet = true;
		gobParams.extraData = _vm->_game->_script->readInt16();
		gobParams.objDesc = _vm->_goblin->_objects[gobParams.extraData];
	}

/*
	NB: The original gobliiins engine did not initialize the gobParams.objDesc
	variable, so we manually check if gobParams.objDesc is properly set before
	checking if it is zero. If it was not set, we do not return. This
	fixes a crash in the EGA version if the life bar is depleted, because
	interFunc is called multiple times with cmd == 39.
	Bug #1324814
*/

	if ((cmd < 40) && objDescSet && !gobParams.objDesc)
		return;

	executeOpcodeGob(cmd, gobParams);
}

void Inter_v1::o1_createSprite(OpFuncParams &params) {
	int16 index;
	int16 width, height;
	int16 flag;

	if (_vm->_game->_script->peekByte(1) == 0) {
		index = _vm->_game->_script->readInt16();
		width = _vm->_game->_script->readInt16();
		height = _vm->_game->_script->readInt16();
	} else {
		index = _vm->_game->_script->readValExpr();
		width = _vm->_game->_script->readValExpr();
		height = _vm->_game->_script->readValExpr();
	}

	flag = _vm->_game->_script->readInt16();
	_vm->_draw->initSpriteSurf(index, width, height, flag ? 2 : 0);
}

void Inter_v1::o1_freeSprite(OpFuncParams &params) {
	_vm->_draw->freeSprite(_vm->_game->_script->readInt16());
}

void Inter_v1::o1_returnTo(OpFuncParams &params) {
	if (params.retFlag == 1) {
		_break = true;
		_vm->_game->_script->setFinished(true);
		params.doReturn = true;
		return;
	}

	if (*_nestLevel == 0)
		return;

	*_breakFromLevel = *_nestLevel;
	_break = true;
	_vm->_game->_script->setFinished(true);

	params.doReturn = true;
}

void Inter_v1::o1_loadSpriteContent(OpFuncParams &params) {
	_vm->_draw->_spriteLeft = _vm->_game->_script->readInt16();
	_vm->_draw->_destSurface = _vm->_game->_script->readInt16();
	_vm->_draw->_transparency = _vm->_game->_script->readInt16();
	_vm->_draw->_destSpriteX = 0;
	_vm->_draw->_destSpriteY = 0;

	_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
}

void Inter_v1::o1_copySprite(OpFuncParams &params) {
	if (_vm->_game->_script->peekByte(1) == 0)
		_vm->_draw->_sourceSurface = _vm->_game->_script->readInt16();
	else
		_vm->_draw->_sourceSurface = _vm->_game->_script->readValExpr();

	if (_vm->_game->_script->peekByte(1) == 0)
		_vm->_draw->_destSurface = _vm->_game->_script->readInt16();
	else
		_vm->_draw->_destSurface = _vm->_game->_script->readValExpr();

	_vm->_draw->_spriteLeft = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteTop = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteRight = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteBottom = _vm->_game->_script->readValExpr();

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();

	_vm->_draw->_transparency = _vm->_game->_script->readInt16();

	_vm->_draw->spriteOperation(DRAW_BLITSURF);
}

void Inter_v1::o1_fillRect(OpFuncParams &params) {
	int16 destSurf;

	_vm->_draw->_destSurface = destSurf = _vm->_game->_script->readInt16();

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteRight = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteBottom = _vm->_game->_script->readValExpr();

	_vm->_draw->_backColor = _vm->_game->_script->readValExpr();

	if (!_vm->_draw->_spritesArray[(destSurf >= 100) ? (destSurf - 80) : destSurf])
		return;

	if (_vm->_draw->_spriteRight < 0) {
		_vm->_draw->_destSpriteX += _vm->_draw->_spriteRight - 1;
		_vm->_draw->_spriteRight = -_vm->_draw->_spriteRight + 2;
	}
	if (_vm->_draw->_spriteBottom < 0) {
		_vm->_draw->_destSpriteY += _vm->_draw->_spriteBottom - 1;
		_vm->_draw->_spriteBottom = -_vm->_draw->_spriteBottom + 2;
	}

	_vm->_draw->spriteOperation(DRAW_FILLRECT);
}

void Inter_v1::o1_drawLine(OpFuncParams &params) {
	_vm->_draw->_destSurface = _vm->_game->_script->readInt16();

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteRight = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteBottom = _vm->_game->_script->readValExpr();

	_vm->_draw->_frontColor = _vm->_game->_script->readValExpr();
	_vm->_draw->spriteOperation(DRAW_DRAWLINE);
}

void Inter_v1::o1_strToLong(OpFuncParams &params) {
	char str[20];
	int16 strVar;
	int16 destVar;
	int32 res;

	strVar = _vm->_game->_script->readVarIndex();
	Common::strlcpy(str, GET_VARO_STR(strVar), 20);
	res = atoi(str);

	destVar = _vm->_game->_script->readVarIndex();
	WRITE_VAR_OFFSET(destVar, res);
}

void Inter_v1::o1_invalidate(OpFuncParams &params) {
	_vm->_draw->_destSurface = _vm->_game->_script->readInt16();
	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteRight = _vm->_game->_script->readValExpr();
	_vm->_draw->_frontColor = _vm->_game->_script->readValExpr();
	_vm->_draw->spriteOperation(DRAW_INVALIDATE);
}

void Inter_v1::o1_setBackDelta(OpFuncParams &params) {
	_vm->_draw->_backDeltaX = _vm->_game->_script->readValExpr();
	_vm->_draw->_backDeltaY = _vm->_game->_script->readValExpr();
}

void Inter_v1::o1_playSound(OpFuncParams &params) {
	int16 frequency;
	int16 freq2;
	int16 repCount;
	int16 index;
	int16 endRep;

	index = _vm->_game->_script->readValExpr();
	repCount = _vm->_game->_script->readValExpr();
	frequency = _vm->_game->_script->readValExpr();

	SoundDesc *sample = _vm->_sound->sampleGetBySlot(index);

	_soundEndTimeKey = 0;
	if (!sample || sample->empty())
		return;

	if (repCount < 0) {
		if (_vm->_global->_soundFlags < 2)
			return;

		repCount = -repCount;
		_soundEndTimeKey = _vm->_util->getTimeKey();

		freq2 = frequency ? frequency : sample->_frequency;
		endRep = MAX(repCount - 1, 1);

		_soundStopVal = sample->calcFadeOutLength(freq2);
		_soundEndTimeKey += sample->calcLength(endRep, freq2, true);
	}

	if (sample->getType() == SOUND_ADL) {
		_vm->_sound->adlibLoadADL(sample->getData(), sample->size(), index);
		_vm->_sound->adlibSetRepeating(repCount - 1);
		_vm->_sound->adlibPlay();
	} else {
		_vm->_sound->blasterStop(0);
		_vm->_sound->blasterPlay(sample, repCount - 1, frequency);
	}
}

void Inter_v1::o1_stopSound(OpFuncParams &params) {
	_vm->_sound->adlibStop();
	_vm->_sound->blasterStop(_vm->_game->_script->readValExpr());

	_soundEndTimeKey = 0;
}

void Inter_v1::o1_loadSound(OpFuncParams &params) {
	loadSound(-1);
}

void Inter_v1::o1_freeSoundSlot(OpFuncParams &params) {
	_vm->_game->freeSoundSlot(-1);
}

void Inter_v1::o1_waitEndPlay(OpFuncParams &params) {
	_vm->_sound->blasterWaitEndPlay();
}

void Inter_v1::o1_playComposition(OpFuncParams &params) {
	int16 dataVar = _vm->_game->_script->readVarIndex();
	int16 freqVal = _vm->_game->_script->readValExpr();

	int16 composition[50];
	int maxEntries = MIN<int>(50, (_variables->getSize() - dataVar) / 4);
	for (int i = 0; i < 50; i++)
		composition[i] = (i < maxEntries) ? ((int16) VAR_OFFSET(dataVar + i * 4)) : -1;

	_vm->_sound->blasterPlayComposition(composition, freqVal);
}

void Inter_v1::o1_getFreeMem(OpFuncParams &params) {
	int16 freeVar;
	int16 maxFreeVar;

	freeVar = _vm->_game->_script->readVarIndex();
	maxFreeVar = _vm->_game->_script->readVarIndex();

	// HACK
	WRITE_VAR_OFFSET(freeVar, 1000000);
	WRITE_VAR_OFFSET(maxFreeVar, 1000000);
}

void Inter_v1::o1_checkData(OpFuncParams &params) {
	const char *file   = _vm->_game->_script->evalString();
	      int16 varOff = _vm->_game->_script->readVarIndex();

	if (!_vm->_dataIO->hasFile(file)) {
		warning("File \"%s\" not found", file);
		WRITE_VAR_OFFSET(varOff, (uint32) -1);
	} else
		WRITE_VAR_OFFSET(varOff, 50); // "handle" between 50 and 128 = in archive
}

void Inter_v1::o1_cleanupStr(OpFuncParams &params) {
	int16 strVar;

	strVar = _vm->_game->_script->readVarIndex();
	_vm->_util->cleanupStr(GET_VARO_FSTR(strVar));
}

void Inter_v1::o1_insertStr(OpFuncParams &params) {
	int16 pos;
	int16 strVar;

	strVar = _vm->_game->_script->readVarIndex();
	_vm->_game->_script->evalExpr(0);
	pos = _vm->_game->_script->readValExpr();

	char *str = GET_VARO_FSTR(strVar);
	_vm->_util->insertStr(_vm->_game->_script->getResultStr(), str, pos);
}

void Inter_v1::o1_cutStr(OpFuncParams &params) {
	int16 strVar;
	int16 pos;
	int16 size;

	strVar = _vm->_game->_script->readVarIndex();
	pos = _vm->_game->_script->readValExpr();
	size = _vm->_game->_script->readValExpr();
	_vm->_util->cutFromStr(GET_VARO_STR(strVar), pos, size);
}

void Inter_v1::o1_strstr(OpFuncParams &params) {
	int16 strVar;
	int16 resVar;
	int16 pos;

	strVar = _vm->_game->_script->readVarIndex();
	_vm->_game->_script->evalExpr(0);
	resVar = _vm->_game->_script->readVarIndex();

	char *res = strstr(GET_VARO_STR(strVar), _vm->_game->_script->getResultStr());
	pos = res ? (res - (GET_VARO_STR(strVar))) : -1;
	WRITE_VAR_OFFSET(resVar, pos);
}

void Inter_v1::o1_istrlen(OpFuncParams &params) {
	int16 len;
	int16 strVar;

	strVar = _vm->_game->_script->readVarIndex();
	len = strlen(GET_VARO_STR(strVar));
	strVar = _vm->_game->_script->readVarIndex();

	WRITE_VAR_OFFSET(strVar, len);
}

void Inter_v1::o1_setMousePos(OpFuncParams &params) {
	_vm->_global->_inter_mouseX = _vm->_game->_script->readValExpr();
	_vm->_global->_inter_mouseY = _vm->_game->_script->readValExpr();
	_vm->_global->_inter_mouseX -= _vm->_video->_scrollOffsetX;
	_vm->_global->_inter_mouseY -= _vm->_video->_scrollOffsetY;
	if (_vm->_global->_useMouse != 0)
		_vm->_util->setMousePos(_vm->_global->_inter_mouseX,
				_vm->_global->_inter_mouseY);
}

void Inter_v1::o1_setFrameRate(OpFuncParams &params) {
	_vm->_util->setFrameRate(_vm->_game->_script->readValExpr());
}

void Inter_v1::o1_animatePalette(OpFuncParams &params) {
	_vm->_draw->blitInvalidated();
	_vm->_util->waitEndFrame();
	animPalette();
	storeKey(_vm->_game->checkKeys(&_vm->_global->_inter_mouseX,
		&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons, 0));
}

void Inter_v1::o1_animateCursor(OpFuncParams &params) {
	_vm->_draw->animateCursor(1);
}

void Inter_v1::o1_blitCursor(OpFuncParams &params) {
	_vm->_draw->blitCursor();
}

void Inter_v1::o1_loadFont(OpFuncParams &params) {
	const char  *font  = _vm->_game->_script->evalString();
	      uint16 index = _vm->_game->_script->readInt16();

	_vm->_draw->animateCursor(4);
	_vm->_draw->loadFont(index, font);
}

void Inter_v1::o1_freeFont(OpFuncParams &params) {
	int16 index;

	index = _vm->_game->_script->readInt16();

	if (index >= Draw::kFontCount) {
		warning("o1_freeFont(): Index %d > count %d", index, Draw::kFontCount);
		return;
	}

	delete _vm->_draw->_fonts[index];
	_vm->_draw->_fonts[index] = 0;
}

void Inter_v1::o1_readData(OpFuncParams &params) {
	const char *file    = _vm->_game->_script->evalString();
	      int16 dataVar = _vm->_game->_script->readVarIndex();
	      int16 size    = _vm->_game->_script->readValExpr();
	      int16 offset  = _vm->_game->_script->readValExpr();
	      int16 retSize = 0;

	WRITE_VAR(1, 1);

	Common::SeekableReadStream *stream = _vm->_dataIO->getFile(file);
	if (!stream)
		return;

	_vm->_draw->animateCursor(4);
	if (offset < 0)
		stream->seek(offset + 1, SEEK_END);
	else
		stream->seek(offset);

	if (((dataVar >> 2) == 59) && (size == 4))
		WRITE_VAR(59, stream->readUint32LE());
	else
		retSize = stream->read((byte *)_variables->getAddressOff8(dataVar), size);

	if (retSize == size)
		WRITE_VAR(1, 0);

	delete stream;
}

void Inter_v1::o1_writeData(OpFuncParams &params) {
	// This writes into a file. It's not portable and isn't needed anyway
	// (Gobliiins 1 doesn't use save file), so we just warn should it be
	// called regardless.

	const char *file = _vm->_game->_script->evalString();

	int16 dataVar = _vm->_game->_script->readVarIndex();
	int16 size    = _vm->_game->_script->readValExpr();
	int16 offset  = _vm->_game->_script->readValExpr();

	warning("Attempted to write to file \"%s\" (%d, %d, %d)", file, dataVar, size, offset);
	WRITE_VAR(1, 0);
}

void Inter_v1::o1_manageDataFile(OpFuncParams &params) {
	Common::String file = _vm->_game->_script->evalString();

	if (!file.empty()) {
		_vm->_dataIO->openArchive(file, true);
	} else {
		_vm->_dataIO->closeArchive(true);

		// NOTE: Lost in Time might close a data file without explicitely closing a video in it.
		//       So we make sure that all open videos are still available.
		_vm->_vidPlayer->reopenAll();
	}
}

void Inter_v1::o1_setState(OpGobParams &params) {
	params.objDesc->state = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemStateVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setCurFrame(OpGobParams &params) {
	params.objDesc->curFrame = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemFrameVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setNextState(OpGobParams &params) {
	params.objDesc->nextState = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemNextStateVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setMultState(OpGobParams &params) {
	params.objDesc->multState = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemMultStateVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setOrder(OpGobParams &params) {
	params.objDesc->order = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemOrderVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setActionStartState(OpGobParams &params) {
	params.objDesc->actionStartState = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemActStartStVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setCurLookDir(OpGobParams &params) {
	params.objDesc->curLookDir = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemLookDirVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setType(OpGobParams &params) {
	params.objDesc->type = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemTypeVarPtr = (uint32) params.extraData;

	if (params.extraData == 0)
		params.objDesc->toRedraw = 1;
}

void Inter_v1::o1_setNoTick(OpGobParams &params) {
	params.objDesc->noTick = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemNoTickVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setPickable(OpGobParams &params) {
	params.objDesc->pickable = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemPickableVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setXPos(OpGobParams &params) {
	params.objDesc->xPos = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemScrXVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setYPos(OpGobParams &params) {
	params.objDesc->yPos = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemScrYVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setDoAnim(OpGobParams &params) {
	params.objDesc->doAnim = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemDoAnimVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setRelaxTime(OpGobParams &params) {
	params.objDesc->relaxTime = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemRelaxVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_setMaxTick(OpGobParams &params) {
	params.objDesc->maxTick = params.extraData;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc)
		_vm->_goblin->_destItemMaxTickVarPtr = (uint32) params.extraData;
}

void Inter_v1::o1_getState(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->state;
}

void Inter_v1::o1_getCurFrame(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->curFrame;
}

void Inter_v1::o1_getNextState(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->nextState;
}

void Inter_v1::o1_getMultState(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->multState;
}

void Inter_v1::o1_getOrder(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->order;
}

void Inter_v1::o1_getActionStartState(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->actionStartState;
}

void Inter_v1::o1_getCurLookDir(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->curLookDir;
}

void Inter_v1::o1_getType(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->type;
}

void Inter_v1::o1_getNoTick(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->noTick;
}

void Inter_v1::o1_getPickable(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->pickable;
}

void Inter_v1::o1_getObjMaxFrame(OpGobParams &params) {
	params.retVarPtr = (uint32) _vm->_goblin->getObjMaxFrame(params.objDesc);
}

void Inter_v1::o1_getXPos(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->xPos;
}

void Inter_v1::o1_getYPos(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->yPos;
}

void Inter_v1::o1_getDoAnim(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->doAnim;
}

void Inter_v1::o1_getRelaxTime(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->relaxTime;
}

void Inter_v1::o1_getMaxTick(OpGobParams &params) {
	params.retVarPtr = (uint32) params.objDesc->maxTick;
}

void Inter_v1::o1_manipulateMap(OpGobParams &params) {
	int16 xPos = _vm->_game->_script->readInt16();
	int16 yPos = _vm->_game->_script->readInt16();
	int16 item = _vm->_game->_script->readInt16();

	manipulateMap(xPos, yPos, item);
}

void Inter_v1::o1_getItem(OpGobParams &params) {
	int16 xPos = _vm->_game->_script->readInt16();
	int16 yPos = _vm->_game->_script->readInt16();

	if ((_vm->_map->getItem(xPos, yPos) & 0xFF00) != 0)
		params.retVarPtr = (uint32) ((_vm->_map->getItem(xPos, yPos) & 0xFF00) >> 8);
	else
		params.retVarPtr = (uint32) _vm->_map->getItem(xPos, yPos);
}

void Inter_v1::o1_manipulateMapIndirect(OpGobParams &params) {
	int16 xPos = _vm->_game->_script->readInt16();
	int16 yPos = _vm->_game->_script->readInt16();
	int16 item = _vm->_game->_script->readInt16();

	xPos = VAR(xPos);
	yPos = VAR(yPos);
	item = VAR(item);

	manipulateMap(xPos, yPos, item);
}

void Inter_v1::o1_getItemIndirect(OpGobParams &params) {
	int16 xPos = _vm->_game->_script->readInt16();
	int16 yPos = _vm->_game->_script->readInt16();

	xPos = VAR(xPos);
	yPos = VAR(yPos);

	if ((_vm->_map->getItem(xPos, yPos) & 0xFF00) != 0)
		params.retVarPtr = (uint32) ((_vm->_map->getItem(xPos, yPos) & 0xFF00) >> 8);
	else
		params.retVarPtr = (uint32) _vm->_map->getItem(xPos, yPos);
}

void Inter_v1::o1_setPassMap(OpGobParams &params) {
	int16 xPos = _vm->_game->_script->readInt16();
	int16 yPos = _vm->_game->_script->readInt16();
	int16 val = _vm->_game->_script->readInt16();
	_vm->_map->setPass(xPos, yPos, val);
}

void Inter_v1::o1_setGoblinPosH(OpGobParams &params) {
	int16 layer;
	int16 item = _vm->_game->_script->readInt16();
	int16 xPos = _vm->_game->_script->readInt16();
	int16 yPos = _vm->_game->_script->readInt16();

	_vm->_goblin->_gobPositions[item].x = xPos * 2;
	_vm->_goblin->_gobPositions[item].y = yPos * 2;

	params.objDesc = _vm->_goblin->_goblins[item];
	params.objDesc->nextState = 21;

	_vm->_goblin->nextLayer(params.objDesc);

	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, params.objDesc->animation, 0,
			params.objDesc->xPos, params.objDesc->yPos, 0);

	params.objDesc->yPos = (_vm->_goblin->_gobPositions[item].y * 6 + 6) -
		(_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
	params.objDesc->xPos =
			_vm->_goblin->_gobPositions[item].x * 12 -
			(_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);

	params.objDesc->curFrame = 0;
	params.objDesc->state = 21;
	if (_vm->_goblin->_currentGoblin == item) {
		_vm->_goblin->_curGobScrXVarPtr = (uint32) params.objDesc->xPos;
		_vm->_goblin->_curGobScrYVarPtr = (uint32) params.objDesc->yPos;

		_vm->_goblin->_curGobFrameVarPtr = 0;
		_vm->_goblin->_curGobStateVarPtr = 18;
		_vm->_goblin->_pressedMapX = _vm->_goblin->_gobPositions[item].x;
		_vm->_goblin->_pressedMapY = _vm->_goblin->_gobPositions[item].y;
	}
}

void Inter_v1::o1_getGoblinPosXH(OpGobParams &params) {
	int16 item = _vm->_game->_script->readInt16();
	params.retVarPtr = (uint32) (_vm->_goblin->_gobPositions[item].x >> 1);
}

void Inter_v1::o1_getGoblinPosYH(OpGobParams &params) {
	int16 item = _vm->_game->_script->readInt16();
	params.retVarPtr = (uint32) (_vm->_goblin->_gobPositions[item].y >> 1);
}

void Inter_v1::o1_setGoblinMultState(OpGobParams &params) {
	int16 layer;
	int16 item = _vm->_game->_script->readInt16();
	int16 xPos = _vm->_game->_script->readInt16();
	int16 yPos = _vm->_game->_script->readInt16();

	params.objDesc = _vm->_goblin->_goblins[item];
	if (yPos == 0) {
		params.objDesc->multState = xPos;
		params.objDesc->nextState = xPos;
		_vm->_goblin->nextLayer(params.objDesc);

		layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

		Scenery::AnimLayer *animLayer =
			_vm->_scenery->getAnimLayer(params.objDesc->animation, layer);
		params.objDesc->xPos = animLayer->posX;
		params.objDesc->yPos = animLayer->posY;

		_vm->_goblin->_curGobScrXVarPtr = (uint32) params.objDesc->xPos;
		_vm->_goblin->_curGobScrYVarPtr = (uint32) params.objDesc->yPos;
		_vm->_goblin->_curGobFrameVarPtr = 0;
		_vm->_goblin->_curGobStateVarPtr = (uint32) params.objDesc->state;
		_vm->_goblin->_curGobNextStateVarPtr = (uint32) params.objDesc->nextState;
		_vm->_goblin->_curGobMultStateVarPtr = (uint32) params.objDesc->multState;
		_vm->_goblin->_curGobMaxFrameVarPtr =
				(uint32) _vm->_goblin->getObjMaxFrame(params.objDesc);
		_vm->_goblin->_noPick = 1;
		return;
	}

	params.objDesc->multState = 21;
	params.objDesc->nextState = 21;
	params.objDesc->state = 21;
	_vm->_goblin->nextLayer(params.objDesc);
	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, params.objDesc->animation, 0,
			params.objDesc->xPos, params.objDesc->yPos, 0);

	params.objDesc->yPos = (yPos * 6 + 6) -
		(_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
	params.objDesc->xPos = xPos * 12 -
		(_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);

	_vm->_goblin->_gobPositions[item].x = xPos;
	_vm->_goblin->_pressedMapX = xPos;
	_vm->_map->_curGoblinX = xPos;

	_vm->_goblin->_gobPositions[item].y = yPos;
	_vm->_goblin->_pressedMapY = yPos;
	_vm->_map->_curGoblinY = yPos;

	_vm->_goblin->_curGobScrXVarPtr = (uint32) params.objDesc->xPos;
	_vm->_goblin->_curGobScrYVarPtr = (uint32) params.objDesc->yPos;
	_vm->_goblin->_curGobFrameVarPtr = 0;
	_vm->_goblin->_curGobStateVarPtr = 21;
	_vm->_goblin->_curGobNextStateVarPtr = 21;
	_vm->_goblin->_curGobMultStateVarPtr = (uint32) -1;
	_vm->_goblin->_noPick = 0;
}

void Inter_v1::o1_setGoblinUnk14(OpGobParams &params) {
	int16 item = _vm->_game->_script->readInt16();
	int16 val = _vm->_game->_script->readInt16();
	params.objDesc = _vm->_goblin->_objects[item];
	params.objDesc->unk14 = val;
}

void Inter_v1::o1_setItemIdInPocket(OpGobParams &params) {
	_vm->_goblin->_itemIdInPocket = _vm->_game->_script->readInt16();
}

void Inter_v1::o1_setItemIndInPocket(OpGobParams &params) {
	_vm->_goblin->_itemIndInPocket = _vm->_game->_script->readInt16();
}

void Inter_v1::o1_getItemIdInPocket(OpGobParams &params) {
	params.retVarPtr = (uint32) _vm->_goblin->_itemIdInPocket;
}

void Inter_v1::o1_getItemIndInPocket(OpGobParams &params) {
	params.retVarPtr = (uint32) _vm->_goblin->_itemIndInPocket;
}

void Inter_v1::o1_setGoblinPos(OpGobParams &params) {
	int16 layer;
	int16 item = _vm->_game->_script->readInt16();
	int16 xPos = _vm->_game->_script->readInt16();
	int16 yPos = _vm->_game->_script->readInt16();

	_vm->_goblin->_gobPositions[item].x = xPos;
	_vm->_goblin->_gobPositions[item].y = yPos;

	params.objDesc = _vm->_goblin->_goblins[item];
	params.objDesc->nextState = 21;
	_vm->_goblin->nextLayer(params.objDesc);

	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, params.objDesc->animation, 0,
			params.objDesc->xPos, params.objDesc->yPos, 0);

	params.objDesc->yPos = (yPos * 6 + 6) -
		(_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
	params.objDesc->xPos = xPos * 12 -
		(_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);

	params.objDesc->curFrame = 0;
	params.objDesc->state = 21;

	if (_vm->_goblin->_currentGoblin == item) {
		_vm->_goblin->_curGobScrXVarPtr = (uint32) params.objDesc->xPos;
		_vm->_goblin->_curGobScrYVarPtr = (uint32) params.objDesc->yPos;
		_vm->_goblin->_curGobFrameVarPtr = 0;
		_vm->_goblin->_curGobStateVarPtr = 18;

		_vm->_goblin->_pressedMapX = _vm->_goblin->_gobPositions[item].x;
		_vm->_goblin->_pressedMapY = _vm->_goblin->_gobPositions[item].y;
	}
}

void Inter_v1::o1_setGoblinState(OpGobParams &params) {
	int16 layer;
	int16 item = _vm->_game->_script->readInt16();
	int16 state = _vm->_game->_script->readInt16();

	params.objDesc = _vm->_goblin->_goblins[item];
	params.objDesc->nextState = state;

	_vm->_goblin->nextLayer(params.objDesc);
	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	Scenery::AnimLayer *animLayer =
		_vm->_scenery->getAnimLayer(params.objDesc->animation, layer);
	params.objDesc->xPos = animLayer->posX;
	params.objDesc->yPos = animLayer->posY;

	if (item == _vm->_goblin->_currentGoblin) {
		_vm->_goblin->_curGobScrXVarPtr = (uint32) params.objDesc->xPos;
		_vm->_goblin->_curGobScrYVarPtr = (uint32) params.objDesc->yPos;
		_vm->_goblin->_curGobFrameVarPtr = 0;
		_vm->_goblin->_curGobStateVarPtr = (uint32) params.objDesc->state;
		_vm->_goblin->_curGobMultStateVarPtr = (uint32) params.objDesc->multState;
	}
}

void Inter_v1::o1_setGoblinStateRedraw(OpGobParams &params) {
	int16 layer;
	int16 item = _vm->_game->_script->readInt16();
	int16 state = _vm->_game->_script->readInt16();
	params.objDesc = _vm->_goblin->_objects[item];

	params.objDesc->nextState = state;

	_vm->_goblin->nextLayer(params.objDesc);
	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	Scenery::AnimLayer *animLayer =
		_vm->_scenery->getAnimLayer(params.objDesc->animation, layer);
	params.objDesc->xPos = animLayer->posX;
	params.objDesc->yPos = animLayer->posY;

	params.objDesc->toRedraw = 1;
	params.objDesc->type = 0;
	if (params.objDesc == _vm->_goblin->_actDestItemDesc) {
		_vm->_goblin->_destItemScrXVarPtr = (uint32) params.objDesc->xPos;
		_vm->_goblin->_destItemScrYVarPtr = (uint32) params.objDesc->yPos;

		_vm->_goblin->_destItemStateVarPtr = (uint32) params.objDesc->state;
		_vm->_goblin->_destItemNextStateVarPtr = (uint32) -1;
		_vm->_goblin->_destItemMultStateVarPtr = (uint32) -1;
		_vm->_goblin->_destItemFrameVarPtr = 0;
	}
}

void Inter_v1::o1_decRelaxTime(OpGobParams &params) {
	params.extraData = _vm->_game->_script->readInt16();
	params.objDesc = _vm->_goblin->_objects[params.extraData];

	params.objDesc->relaxTime--;
	if ((params.objDesc->relaxTime < 0) &&
	    (_vm->_goblin->getObjMaxFrame(params.objDesc) ==
	     params.objDesc->curFrame)) {
		params.objDesc->relaxTime = _vm->_util->getRandom(100) + 50;
		params.objDesc->curFrame = 0;
		params.objDesc->toRedraw = 1;
	}
}

void Inter_v1::o1_getGoblinPosX(OpGobParams &params) {
	int16 item = _vm->_game->_script->readInt16();
	params.retVarPtr = (uint32) _vm->_goblin->_gobPositions[item].x;
}

void Inter_v1::o1_getGoblinPosY(OpGobParams &params) {
	int16 item = _vm->_game->_script->readInt16();
	params.retVarPtr = (uint32) _vm->_goblin->_gobPositions[item].y;
}

void Inter_v1::o1_clearPathExistence(OpGobParams &params) {
	_vm->_goblin->_pathExistence = 0;
}

void Inter_v1::o1_setGoblinVisible(OpGobParams &params) {
	params.extraData = _vm->_game->_script->readInt16();
	_vm->_goblin->_goblins[params.extraData]->visible = 1;
}

void Inter_v1::o1_setGoblinInvisible(OpGobParams &params) {
	params.extraData = _vm->_game->_script->readInt16();
	_vm->_goblin->_goblins[params.extraData]->visible = 0;
}

void Inter_v1::o1_getObjectIntersect(OpGobParams &params) {
	params.extraData = _vm->_game->_script->readInt16();
	int16 item = _vm->_game->_script->readInt16();

	params.objDesc = _vm->_goblin->_objects[params.extraData];
	if (_vm->_goblin->objIntersected(params.objDesc,
				_vm->_goblin->_goblins[item]))
		params.retVarPtr = 1;
	else
		params.retVarPtr = 0;
}

void Inter_v1::o1_getGoblinIntersect(OpGobParams &params) {
	params.extraData = _vm->_game->_script->readInt16();
	int16 item = _vm->_game->_script->readInt16();

	params.objDesc = _vm->_goblin->_goblins[params.extraData];
	if (_vm->_goblin->objIntersected(params.objDesc,
				_vm->_goblin->_goblins[item]))
		params.retVarPtr = 1;
	else
		params.retVarPtr = 0;
}

void Inter_v1::o1_setItemPos(OpGobParams &params) {
	int16 item = _vm->_game->_script->readInt16();
	int16 xPos = _vm->_game->_script->readInt16();
	int16 yPos = _vm->_game->_script->readInt16();
	int16 val = _vm->_game->_script->readInt16();

	_vm->_map->_itemPoses[item].x = xPos;
	_vm->_map->_itemPoses[item].y = yPos;
	_vm->_map->_itemPoses[item].orient = val;
}

void Inter_v1::o1_loadObjects(OpGobParams &params) {
	params.extraData = _vm->_game->_script->readInt16();

	_vm->_goblin->loadObjects(_variables->getAddressVarString(params.extraData));
}

void Inter_v1::o1_freeObjects(OpGobParams &params) {
	_vm->_goblin->freeAllObjects();
}

void Inter_v1::o1_animateObjects(OpGobParams &params) {
	_vm->_goblin->animateObjects();
}

void Inter_v1::o1_drawObjects(OpGobParams &params) {
	_vm->_goblin->drawObjects();

	_vm->_sound->adlibPlayBgMusic(); // Mac version
	if (_vm->_sound->cdGetTrackPos() == -1)
		_vm->_sound->cdPlayBgMusic(); // PC CD version
}

void Inter_v1::o1_loadMap(OpGobParams &params) {
	_vm->_map->loadMapsInitGobs();
}

void Inter_v1::o1_moveGoblin(OpGobParams &params) {
	int16 item;
	params.extraData = _vm->_game->_script->readInt16();
	int16 xPos = _vm->_game->_script->readInt16();

	if ((uint16) VAR(xPos) == 0) {
		item =
				_vm->_goblin->doMove(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin],
					1, (uint16) VAR(params.extraData));
	} else {
		item =
				_vm->_goblin->doMove(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin],
					1, 3);
	}

	if (item != 0)
		_vm->_goblin->switchGoblin(item);
}

void Inter_v1::o1_switchGoblin(OpGobParams &params) {
	_vm->_goblin->switchGoblin(0);
}

void Inter_v1::o1_loadGoblin(OpGobParams &params) {
	_vm->_goblin->loadGobDataFromVars();
}

void Inter_v1::o1_writeTreatItem(OpGobParams &params) {
	params.extraData = _vm->_game->_script->readInt16();
	int16 cmd = _vm->_game->_script->readInt16();
	int16 xPos = _vm->_game->_script->readInt16();

	if ((uint16) VAR(xPos) == 0) {
		WRITE_VAR(cmd, _vm->_goblin->treatItem((uint16) VAR(params.extraData)));
		return;
	}

	WRITE_VAR(cmd, _vm->_goblin->treatItem(3));
}

void Inter_v1::o1_moveGoblin0(OpGobParams &params) {
	_vm->_goblin->doMove(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin],
			0, 0);
}

void Inter_v1::o1_setGoblinTarget(OpGobParams &params) {
	params.extraData = _vm->_game->_script->readInt16();
	if (VAR(params.extraData) != 0)
		_vm->_goblin->_goesAtTarget = 1;
	else
		_vm->_goblin->_goesAtTarget = 0;
}

void Inter_v1::o1_setGoblinObjectsPos(OpGobParams &params) {
	params.extraData = _vm->_game->_script->readInt16();
	params.extraData = VAR(params.extraData);
	_vm->_goblin->_objects[10]->xPos = params.extraData;

	params.extraData = _vm->_game->_script->readInt16();
	params.extraData = VAR(params.extraData);
	_vm->_goblin->_objects[10]->yPos = params.extraData;
}

void Inter_v1::o1_initGoblin(OpGobParams &params) {
	Goblin::Gob_Object *gobDesc = _vm->_goblin->_goblins[0];
	int16 xPos;
	int16 yPos;
	int16 layer;

	if (_vm->_goblin->_currentGoblin != 0) {
		_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin]->doAnim = 1;
		_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin]->nextState = 21;

		_vm->_goblin->nextLayer(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin]);
		_vm->_goblin->_currentGoblin = 0;

		gobDesc->doAnim = 0;
		gobDesc->type = 0;
		gobDesc->toRedraw = 1;

		_vm->_goblin->_pressedMapX = _vm->_goblin->_gobPositions[0].x;
		_vm->_map->_destX = _vm->_goblin->_gobPositions[0].x;
		_vm->_goblin->_gobDestX = _vm->_goblin->_gobPositions[0].x;

		_vm->_goblin->_pressedMapY = _vm->_goblin->_gobPositions[0].y;
		_vm->_map->_destY = _vm->_goblin->_gobPositions[0].y;
		_vm->_goblin->_gobDestY = _vm->_goblin->_gobPositions[0].y;

		_vm->_goblin->_curGobVarPtr = 0;
		_vm->_goblin->_pathExistence = 0;
		_vm->_goblin->_readyToAct = 0;
	}

	if ((gobDesc->state != 10) && (_vm->_goblin->_itemIndInPocket != -1) &&
			(_vm->_goblin->getObjMaxFrame(gobDesc) == gobDesc->curFrame)) {

		gobDesc->stateMach = gobDesc->realStateMach;
		xPos = _vm->_goblin->_gobPositions[0].x;
		yPos = _vm->_goblin->_gobPositions[0].y;

		gobDesc->nextState = 10;
		layer = _vm->_goblin->nextLayer(gobDesc);

		_vm->_scenery->updateAnim(layer, 0, gobDesc->animation, 0,
				gobDesc->xPos, gobDesc->yPos, 0);

		gobDesc->yPos = (yPos * 6 + 6) -
			(_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
		gobDesc->xPos = xPos * 12 -
			(_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);
	}

	if (gobDesc->state != 10)
		return;

	if (_vm->_goblin->_itemIndInPocket == -1)
		return;

	if (gobDesc->curFrame != 10)
		return;

	params.objDesc = _vm->_goblin->_objects[_vm->_goblin->_itemIndInPocket];
	params.objDesc->type = 0;
	params.objDesc->toRedraw = 1;
	params.objDesc->curFrame = 0;

	params.objDesc->order = gobDesc->order;
	params.objDesc->animation =
			params.objDesc->stateMach[params.objDesc->state][0]->animation;

	layer = params.objDesc->stateMach[params.objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, params.objDesc->animation, 0,
			params.objDesc->xPos, params.objDesc->yPos, 0);

	params.objDesc->yPos += (_vm->_goblin->_gobPositions[0].y * 6 + 5) -
		_vm->_scenery->_toRedrawBottom;

	if (gobDesc->curLookDir == 4) {
		params.objDesc->xPos += _vm->_goblin->_gobPositions[0].x * 12 + 14
				- (_vm->_scenery->_toRedrawLeft + _vm->_scenery->_toRedrawRight) / 2;
	} else {
		params.objDesc->xPos += _vm->_goblin->_gobPositions[0].x * 12
				- (_vm->_scenery->_toRedrawLeft + _vm->_scenery->_toRedrawRight) / 2;
	}

	_vm->_goblin->_itemIndInPocket = -1;
	_vm->_goblin->_itemIdInPocket = -1;
	_vm->_util->beep(50);
}

int16 Inter_v1::loadSound(int16 slot) {
	if (slot == -1)
		slot = _vm->_game->_script->readValExpr();

	uint16 id = _vm->_game->_script->readUint16();
	if (id == 0xFFFF) {
		_vm->_game->_script->skip(9);
		return 0;
	}

	Resource *resource = _vm->_game->_resources->getResource(id);
	if (!resource)
		return 0;

	SoundDesc *sample = _vm->_sound->sampleGetBySlot(slot);
	if (!sample)
		return 0;

	sample->load(SOUND_SND, resource);
	return 0;
}

void Inter_v1::animPalette() {
	int16 i;
	Video::Color col;

	if (_animPalDir[0] == 0)
		return;

	_vm->_video->waitRetrace();

	if (_animPalDir[0] == -1) {
		col = _vm->_draw->_vgaSmallPalette[_animPalLowIndex[0]];

		for (i = _animPalLowIndex[0]; i < _animPalHighIndex[0]; i++)
			_vm->_draw->_vgaSmallPalette[i] = _vm->_draw->_vgaSmallPalette[i + 1];

		_vm->_draw->_vgaSmallPalette[_animPalHighIndex[0]] = col;
	} else {
		col = _vm->_draw->_vgaSmallPalette[_animPalHighIndex[0]];
		for (i = _animPalHighIndex[0]; i > _animPalLowIndex[0]; i--)
			_vm->_draw->_vgaSmallPalette[i] = _vm->_draw->_vgaSmallPalette[i - 1];

		_vm->_draw->_vgaSmallPalette[_animPalLowIndex[0]] = col;
	}

	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaSmallPalette;
	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
}

void Inter_v1::manipulateMap(int16 xPos, int16 yPos, int16 item) {
	for (int y = 0; y < _vm->_map->getMapHeight(); y++) {
		for (int x = 0; x < _vm->_map->getMapWidth(); x++) {
			if ((_vm->_map->getItem(x, y) & 0xFF) == item)
				_vm->_map->setItem(x, y, _vm->_map->getItem(x, y) & 0xFF00);
			else if (((_vm->_map->getItem(x, y) & 0xFF00) >> 8) == item)
				_vm->_map->setItem(x, y, _vm->_map->getItem(x, y) & 0xFF);
		}
	}

	if (xPos < _vm->_map->getMapWidth() - 1) {
		if (yPos > 0) {
			if (((_vm->_map->getItem(xPos, yPos) & 0xFF00) != 0) ||
					((_vm->_map->getItem(xPos, yPos - 1) & 0xFF00) != 0) ||
					((_vm->_map->getItem(xPos + 1, yPos) & 0xFF00) != 0) ||
					((_vm->_map->getItem(xPos + 1, yPos - 1) & 0xFF00) != 0)) {

				_vm->_map->setItem(xPos, yPos,
						(_vm->_map->getItem(xPos, yPos) & 0xFF00) + item);

				_vm->_map->setItem(xPos, yPos - 1,
						(_vm->_map->getItem(xPos, yPos - 1) & 0xFF00) + item);

				_vm->_map->setItem(xPos + 1, yPos,
						(_vm->_map->getItem(xPos + 1, yPos) & 0xFF00) + item);

				_vm->_map->setItem(xPos + 1, yPos - 1,
						(_vm->_map->getItem(xPos + 1, yPos - 1) & 0xFF00) + item);

			} else {
				_vm->_map->setItem(xPos, yPos,
						(_vm->_map->getItem(xPos, yPos) & 0xFF) + (item << 8));

				_vm->_map->setItem(xPos, yPos - 1,
						(_vm->_map->getItem(xPos, yPos - 1) & 0xFF) + (item << 8));

				_vm->_map->setItem(xPos + 1, yPos,
						(_vm->_map->getItem(xPos + 1, yPos) & 0xFF) + (item << 8));

				_vm->_map->setItem(xPos + 1, yPos - 1,
						(_vm->_map->getItem(xPos + 1, yPos - 1) & 0xFF) + (item << 8));
			}
		} else {
			if (((_vm->_map->getItem(xPos, yPos) & 0xFF00) != 0) ||
					((_vm->_map->getItem(xPos + 1, yPos) & 0xFF00) != 0)) {

				_vm->_map->setItem(xPos, yPos,
						(_vm->_map->getItem(xPos, yPos) & 0xFF00) + item);

				_vm->_map->setItem(xPos + 1, yPos,
						(_vm->_map->getItem(xPos + 1, yPos) & 0xFF00) + item);

			} else {
				_vm->_map->setItem(xPos, yPos,
						(_vm->_map->getItem(xPos, yPos) & 0xFF) + (item << 8));

				_vm->_map->setItem(xPos + 1, yPos,
						(_vm->_map->getItem(xPos + 1, yPos) & 0xFF) + (item << 8));
			}
		}
	} else {
		if (yPos > 0) {
			if (((_vm->_map->getItem(xPos, yPos) & 0xFF00) != 0) ||
					((_vm->_map->getItem(xPos, yPos - 1) & 0xFF00) != 0)) {

				_vm->_map->setItem(xPos, yPos,
						(_vm->_map->getItem(xPos, yPos) & 0xFF00) + item);

				_vm->_map->setItem(xPos, yPos - 1,
						(_vm->_map->getItem(xPos, yPos - 1) & 0xFF00) + item);

			} else {
				_vm->_map->setItem(xPos, yPos,
						(_vm->_map->getItem(xPos, yPos) & 0xFF) + (item << 8));

				_vm->_map->setItem(xPos, yPos - 1,
						(_vm->_map->getItem(xPos, yPos - 1) & 0xFF) + (item << 8));
			}
		} else {
			if ((_vm->_map->getItem(xPos, yPos) & 0xFF00) != 0) {
				_vm->_map->setItem(xPos, yPos,
						(_vm->_map->getItem(xPos, yPos) & 0xFF00) + item);
			} else {
				_vm->_map->setItem(xPos, yPos,
						(_vm->_map->getItem(xPos, yPos) & 0xFF) + (item << 8));
			}
		}
	}

	if ((item < 0) || (item >= 20))
		return;

	if ((xPos > 1) && (_vm->_map->getPass(xPos - 2, yPos) == 1)) {
		_vm->_map->_itemPoses[item].x = xPos - 2;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 4;
		return;
	}

	if ((xPos < _vm->_map->getMapWidth() - 2) &&
			(_vm->_map->getPass(xPos + 2, yPos) == 1)) {
		_vm->_map->_itemPoses[item].x = xPos + 2;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 0;
		return;
	}

	if ((xPos < _vm->_map->getMapWidth() - 1) &&
			(_vm->_map->getPass(xPos + 1, yPos) == 1)) {
		_vm->_map->_itemPoses[item].x = xPos + 1;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 0;
		return;
	}

	if ((xPos > 0) && (_vm->_map->getPass(xPos - 1, yPos) == 1)) {
		_vm->_map->_itemPoses[item].x = xPos - 1;
		_vm->_map->_itemPoses[item].y = yPos;
		_vm->_map->_itemPoses[item].orient = 4;
		return;
	}
}

} // End of namespace Gob
