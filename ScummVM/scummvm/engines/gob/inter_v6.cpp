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
#include "gob/dataio.h"
#include "gob/game.h"
#include "gob/expression.h"
#include "gob/script.h"
#include "gob/resources.h"
#include "gob/hotspots.h"
#include "gob/draw.h"
#include "gob/sound/sound.h"
#include "gob/videoplayer.h"

namespace Gob {

#define OPCODEVER Inter_v6
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_v6::Inter_v6(GobEngine *vm) : Inter_v5(vm) {
	_gotFirstPalette = false;
}

void Inter_v6::setupOpcodesDraw() {
	Inter_v5::setupOpcodesDraw();

	OPCODEDRAW(0x40, o6_totSub);
	OPCODEDRAW(0x83, o6_playVmdOrMusic);
}

void Inter_v6::setupOpcodesFunc() {
	Inter_v5::setupOpcodesFunc();

	OPCODEFUNC(0x03, o6_loadCursor);
	OPCODEFUNC(0x09, o6_assign);
	OPCODEFUNC(0x19, o6_removeHotspot);
	OPCODEFUNC(0x32, o1_copySprite);
	OPCODEFUNC(0x33, o6_fillRect);
}

void Inter_v6::setupOpcodesGob() {
}

void Inter_v6::o6_totSub() {
	uint8 length = _vm->_game->_script->readByte();
	if ((length & 0x7F) > 13)
		error("Length in o6_totSub is greater than 13 (%d)", length);

	Common::String totFile;
	if (length & 0x80)
		totFile = _vm->_game->_script->evalString();
	else
		for (uint8 i = 0; i < length; i++)
			totFile += _vm->_game->_script->readChar();

	uint8 flags = _vm->_game->_script->readByte();
	if (flags & 0x40)
		warning("Urban Stub: o6_totSub(), flags & 0x40");

	_vm->_game->totSub(flags, totFile);
}

void Inter_v6::o6_playVmdOrMusic() {
	Common::String file = _vm->_game->_script->evalString();

	VideoPlayer::Properties props;

	props.x          = _vm->_game->_script->readValExpr();
	props.y          = _vm->_game->_script->readValExpr();
	props.startFrame = _vm->_game->_script->readValExpr();
	props.lastFrame  = _vm->_game->_script->readValExpr();
	props.breakKey   = _vm->_game->_script->readValExpr();
	props.flags      = _vm->_game->_script->readValExpr();
	props.palStart   = _vm->_game->_script->readValExpr();
	props.palEnd     = _vm->_game->_script->readValExpr();
	props.palCmd     = 1 << (props.flags & 0x3F);
	props.forceSeek  = true;

	debugC(1, kDebugVideo, "Playing video \"%s\" @ %d+%d, frames %d - %d, "
			"paletteCmd %d (%d - %d), flags %X", file.c_str(),
			props.x, props.y, props.startFrame, props.lastFrame,
			props.palCmd, props.palStart, props.palEnd, props.flags);

	// WORKAROUND: When taking the music sheet from Dr. Dramish's car,
	//             the video that lets the sheet vanish is missing. We'll
	//             play the one where the sheet is already gone instead.
	if (_vm->isCurrentTot("avt005.tot") && file.equalsIgnoreCase("MXRAMPART"))
		file = "PLCOFDR2";

	if (file == "RIEN") {
		_vm->_vidPlayer->closeAll();
		return;
	}

	bool close = false;
	if (props.lastFrame == -1) {
		close = true;
	} else if (props.lastFrame == -5) {
//		warning("Urban/Playtoons Stub: Stop without delay");
		_vm->_sound->bgStop();
		return;
	} else if (props.lastFrame == -6) {
//		warning("Urban/Playtoons Stub: Video/Music command -6 (cache video)");
		return;
	} else if (props.lastFrame == -7) {
//		warning("Urban/Playtoons Stub: Video/Music command -6 (flush cache)");
		return;
	} else if ((props.lastFrame == -8) || (props.lastFrame == -9)) {
		if (!file.contains('.'))
			file += ".WA8";

		probe16bitMusic(file);

		if (props.lastFrame == -9)
			debugC(0, kDebugVideo, "Urban/Playtoons Stub: Delayed music stop?");

		_vm->_sound->bgStop();
		_vm->_sound->bgPlay(file.c_str(), SOUND_WAV);
		return;
	} else if (props.lastFrame <= -10) {
		_vm->_vidPlayer->closeVideo();

		if (!(props.flags & VideoPlayer::kFlagNoVideo))
			props.loop = true;

	} else if (props.lastFrame < 0) {
		warning("Urban/Playtoons Stub: Unknown Video/Music command: %d, %s", props.lastFrame, file.c_str());
		return;
	}

	if (props.startFrame == -2) {
		props.startFrame = 0;
		props.lastFrame  = -1;
		props.noBlock    = true;
	}

	_vm->_vidPlayer->evaluateFlags(props);

	bool primary = true;
	if (props.noBlock && (props.flags & VideoPlayer::kFlagNoVideo))
		primary = false;

	int slot = 0;
	if (!file.empty() && ((slot = _vm->_vidPlayer->openVideo(primary, file, props)) < 0)) {
		WRITE_VAR(11, (uint32) -1);
		return;
	}

	if (props.hasSound)
		_vm->_vidPlayer->closeLiveSound();

	if (props.startFrame >= 0)
		_vm->_vidPlayer->play(slot, props);

	if (close && !props.noBlock) {
		if (!props.canceled)
			_vm->_vidPlayer->waitSoundEnd(slot);
		_vm->_vidPlayer->closeVideo(slot);
	}

}

void Inter_v6::o6_loadCursor(OpFuncParams &params) {
	int16 id = _vm->_game->_script->readInt16();

	if ((id == -1) || (id == -2)) {
		char file[10];

		if (id == -1) {
			for (int i = 0; i < 9; i++)
				file[i] = _vm->_game->_script->readChar();
		} else
			strncpy(file, GET_VAR_STR(_vm->_game->_script->readInt16()), 10);

		file[9] = '\0';

		uint16 start = _vm->_game->_script->readUint16();
		int8 index = _vm->_game->_script->readInt8();

		VideoPlayer::Properties props;

		props.sprite = -1;

		int vmdSlot = _vm->_vidPlayer->openVideo(false, file, props);
		if (vmdSlot == -1) {
			warning("Can't open video \"%s\" as cursor", file);
			return;
		}

		int16 framesCount = _vm->_vidPlayer->getFrameCount(vmdSlot);

		for (int i = 0; i < framesCount; i++) {
			props.startFrame   = i;
			props.lastFrame    = i;
			props.waitEndFrame = false;

			_vm->_vidPlayer->play(vmdSlot, props);
			_vm->_vidPlayer->copyFrame(vmdSlot, *_vm->_draw->_cursorSprites,
					0, 0, _vm->_draw->_cursorWidth, _vm->_draw->_cursorWidth,
					(start + i) * _vm->_draw->_cursorWidth, 0);
		}

		_vm->_vidPlayer->closeVideo(vmdSlot);

		_vm->_draw->_cursorAnimLow[index] = start;
		_vm->_draw->_cursorAnimHigh[index] = framesCount + start - 1;
		_vm->_draw->_cursorAnimDelays[index] = 10;

		return;
	}

	int8 index = _vm->_game->_script->readInt8();

	if ((index * _vm->_draw->_cursorWidth) >= _vm->_draw->_cursorSprites->getWidth())
		return;

	Resource *resource = _vm->_game->_resources->getResource((uint16) id);
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

void Inter_v6::o6_assign(OpFuncParams &params) {
	uint16 size, destType;
	uint16 dest = _vm->_game->_script->readVarIndex(&size, &destType);

	if (size != 0) {
		int16 src;

		_vm->_game->_script->push();

		src = _vm->_game->_script->readVarIndex(&size, 0);

		memcpy(_vm->_inter->_variables->getAddressOff8(dest),
				_vm->_inter->_variables->getAddressOff8((uint16) src), size * 4);

		_vm->_game->_script->pop();

		_vm->_game->_script->evalExpr(&src);

		return;
	}

	byte loopCount;
	if (_vm->_game->_script->peekByte() == 98) {
		_vm->_game->_script->skip(1);
		loopCount = _vm->_game->_script->readByte();

		for (int i = 0; i < loopCount; i++) {
			uint8 c = _vm->_game->_script->readByte();
			uint16 n = _vm->_game->_script->readUint16();

			memset(_vm->_inter->_variables->getAddressOff8(dest), c, n);

			dest += n;
		}

		return;

	} else if (_vm->_game->_script->peekByte() == 99) {
		_vm->_game->_script->skip(1);
		loopCount = _vm->_game->_script->readByte();
	} else
		loopCount = 1;

	for (int i = 0; i < loopCount; i++) {
		int16 result;
		int16 srcType = _vm->_game->_script->evalExpr(&result);

		switch (destType) {
		case TYPE_VAR_INT8:
		case TYPE_ARRAY_INT8:
			WRITE_VARO_UINT8(dest + i, _vm->_game->_script->getResultInt());
			break;

		case TYPE_VAR_INT16:
		case TYPE_ARRAY_INT16:
			WRITE_VARO_UINT16(dest + i * 2, _vm->_game->_script->getResultInt());
			break;

		case TYPE_VAR_INT32:
		case TYPE_ARRAY_INT32:
			WRITE_VAR_OFFSET(dest + i * 4, _vm->_game->_script->getResultInt());
			break;

		case TYPE_VAR_INT32_AS_INT16:
			WRITE_VARO_UINT16(dest + i * 4, _vm->_game->_script->getResultInt());
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

void Inter_v6::o6_removeHotspot(OpFuncParams &params) {
	int16 id;
	uint8 stateType1    = Hotspots::kStateFilledDisabled | Hotspots::kStateType1;
	uint8 stateType2    = Hotspots::kStateFilledDisabled | Hotspots::kStateType2;
	uint8 stateDisabled = Hotspots::kStateDisabled;

	id = _vm->_game->_script->readValExpr();

	switch (id + 5) {
	case 0:
		_vm->_game->_hotspots->push(1);
		break;
	case 1:
		_vm->_game->_hotspots->pop();
		break;
	case 2:
		_vm->_game->_hotspots->push(2);
		break;
	case 3:
		_vm->_game->_hotspots->removeState(stateType1);
		_vm->_game->_hotspots->removeState(stateDisabled);
		break;
	case 4:
		_vm->_game->_hotspots->removeState(stateType2);
		break;
	default:
		_vm->_game->_hotspots->remove((stateType2 << 12) + id);
		break;
	}
}

void Inter_v6::o6_fillRect(OpFuncParams &params) {
	int16 destSurf;

	_vm->_draw->_destSurface = destSurf = _vm->_game->_script->readInt16();

	_vm->_draw->_destSpriteX = _vm->_game->_script->readValExpr();
	_vm->_draw->_destSpriteY = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteRight = _vm->_game->_script->readValExpr();
	_vm->_draw->_spriteBottom = _vm->_game->_script->readValExpr();

	uint32 patternColor = _vm->_game->_script->evalInt();

	_vm->_draw->_backColor = patternColor & 0xFFFF;
	_vm->_draw->_pattern   = patternColor >> 16;

	if (_vm->_draw->_pattern != 0)
		warning("Urban Stub: o6_fillRect(), _pattern = %d", _vm->_draw->_pattern);

	if (_vm->_draw->_spriteRight < 0) {
		_vm->_draw->_destSpriteX += _vm->_draw->_spriteRight - 1;
		_vm->_draw->_spriteRight = -_vm->_draw->_spriteRight + 2;
	}
	if (_vm->_draw->_spriteBottom < 0) {
		_vm->_draw->_destSpriteY += _vm->_draw->_spriteBottom - 1;
		_vm->_draw->_spriteBottom = -_vm->_draw->_spriteBottom + 2;
	}

	if (destSurf & 0x80) {
		warning("Urban Stub: o6_fillRect(), destSurf & 0x80");
		return;
	}

	if (!_vm->_draw->_spritesArray[(destSurf > 100) ? (destSurf - 80) : destSurf])
		return;

	_vm->_draw->spriteOperation(DRAW_FILLRECT);
}

void Inter_v6::probe16bitMusic(Common::String &fileName) {
	if (fileName[fileName.size() - 1] != '8')
		return;

	fileName.setChar('V', fileName.size() - 1);

	if (_vm->_dataIO->hasFile(fileName))
		return;

	fileName.setChar('8', fileName.size() - 1);
}

} // End of namespace Gob
