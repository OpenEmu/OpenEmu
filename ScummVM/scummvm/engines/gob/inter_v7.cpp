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
#include "common/archive.h"
#include "common/winexe.h"
#include "common/winexe_pe.h"

#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/dataio.h"
#include "gob/inter.h"
#include "gob/game.h"
#include "gob/script.h"
#include "gob/expression.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODEVER Inter_v7
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_v7::Inter_v7(GobEngine *vm) : Inter_Playtoons(vm), _cursors(0) {
}

Inter_v7::~Inter_v7() {
	delete _cursors;
}

void Inter_v7::setupOpcodesDraw() {
	Inter_Playtoons::setupOpcodesDraw();

	OPCODEDRAW(0x0C, o7_draw0x0C);
	OPCODEDRAW(0x0D, o7_loadCursor);
	OPCODEDRAW(0x44, o7_displayWarning);
	OPCODEDRAW(0x45, o7_logString);
	OPCODEDRAW(0x57, o7_intToString);
	OPCODEDRAW(0x59, o7_callFunction);
	OPCODEDRAW(0x5A, o7_loadFunctions);
	OPCODEDRAW(0x83, o7_playVmdOrMusic);
	OPCODEDRAW(0x89, o7_draw0x89);
	OPCODEDRAW(0x8A, o7_findFile);
	OPCODEDRAW(0x8B, o7_findCDFile);
	OPCODEDRAW(0x8C, o7_getSystemProperty);
	OPCODEDRAW(0x90, o7_loadImage);
	OPCODEDRAW(0x93, o7_setVolume);
	OPCODEDRAW(0x95, o7_zeroVar);
	OPCODEDRAW(0xA1, o7_getINIValue);
	OPCODEDRAW(0xA2, o7_setINIValue);
	OPCODEDRAW(0xA4, o7_loadLBMPalette);
	OPCODEDRAW(0xC4, o7_opendBase);
	OPCODEDRAW(0xC5, o7_closedBase);
	OPCODEDRAW(0xC6, o7_getDBString);
}

void Inter_v7::setupOpcodesFunc() {
	Inter_Playtoons::setupOpcodesFunc();
}

void Inter_v7::setupOpcodesGob() {
	Inter_Playtoons::setupOpcodesGob();

	OPCODEGOB(420, o7_oemToANSI);
	OPCODEGOB(513, o7_gob0x201);
}

void Inter_v7::o7_draw0x0C() {
	WRITE_VAR(17, 0);
}

void Inter_v7::resizeCursors(int16 width, int16 height, int16 count, bool transparency) {
	if (width <= 0)
		width = _vm->_draw->_cursorWidth;
	if (height <= 0)
		height = _vm->_draw->_cursorHeight;

	width  = MAX<uint16>(width , _vm->_draw->_cursorWidth);
	height = MAX<uint16>(height, _vm->_draw->_cursorHeight);

	_vm->_draw->_transparentCursor = transparency;

	// Cursors sprite already big enough
	if ((_vm->_draw->_cursorWidth >= width) && (_vm->_draw->_cursorHeight >= height) &&
	    (_vm->_draw->_cursorCount >= count))
		return;

	_vm->_draw->_cursorCount  = count;
	_vm->_draw->_cursorWidth  = width;
	_vm->_draw->_cursorHeight = height;

	_vm->_draw->freeSprite(Draw::kCursorSurface);
	_vm->_draw->_cursorSprites.reset();
	_vm->_draw->_cursorSpritesBack.reset();
	_vm->_draw->_scummvmCursor.reset();

	_vm->_draw->initSpriteSurf(Draw::kCursorSurface, width * count, height, 2);

	_vm->_draw->_cursorSpritesBack = _vm->_draw->_spritesArray[Draw::kCursorSurface];
	_vm->_draw->_cursorSprites     = _vm->_draw->_cursorSpritesBack;

	_vm->_draw->_scummvmCursor = _vm->_video->initSurfDesc(width, height, SCUMMVM_CURSOR);

	for (int i = 0; i < 40; i++) {
		_vm->_draw->_cursorAnimLow[i] = -1;
		_vm->_draw->_cursorAnimDelays[i] = 0;
		_vm->_draw->_cursorAnimHigh[i] = 0;
	}
	_vm->_draw->_cursorAnimLow[1] = 0;

	delete[] _vm->_draw->_doCursorPalettes;
	delete[] _vm->_draw->_cursorPalettes;
	delete[] _vm->_draw->_cursorKeyColors;
	delete[] _vm->_draw->_cursorPaletteStarts;
	delete[] _vm->_draw->_cursorPaletteCounts;
	delete[] _vm->_draw->_cursorHotspotsX;
	delete[] _vm->_draw->_cursorHotspotsY;

	_vm->_draw->_cursorPalettes      = new byte[256 * 3 * count];
	_vm->_draw->_doCursorPalettes    = new bool[count];
	_vm->_draw->_cursorKeyColors     = new byte[count];
	_vm->_draw->_cursorPaletteStarts = new uint16[count];
	_vm->_draw->_cursorPaletteCounts = new uint16[count];
	_vm->_draw->_cursorHotspotsX     = new int32[count];
	_vm->_draw->_cursorHotspotsY     = new int32[count];

	memset(_vm->_draw->_cursorPalettes     , 0, count * 256 * 3);
	memset(_vm->_draw->_doCursorPalettes   , 0, count * sizeof(bool));
	memset(_vm->_draw->_cursorKeyColors    , 0, count * sizeof(byte));
	memset(_vm->_draw->_cursorPaletteStarts, 0, count * sizeof(uint16));
	memset(_vm->_draw->_cursorPaletteCounts, 0, count * sizeof(uint16));
	memset(_vm->_draw->_cursorHotspotsX    , 0, count * sizeof(int32));
	memset(_vm->_draw->_cursorHotspotsY    , 0, count * sizeof(int32));
}

void Inter_v7::o7_loadCursor() {
	int16          cursorIndex = _vm->_game->_script->readValExpr();
	Common::String cursorName  = _vm->_game->_script->evalString();

	// Clear the cursor sprite at that index
	_vm->_draw->_cursorSprites->fillRect(cursorIndex * _vm->_draw->_cursorWidth, 0,
			cursorIndex * _vm->_draw->_cursorWidth + _vm->_draw->_cursorWidth - 1,
			_vm->_draw->_cursorHeight - 1, 0);

	// If the cursor name is empty, that cursor will be drawn by the scripts
	if (cursorName.empty()) {
		// Make sure the cursors sprite is big enough and set to non-extern palette
		resizeCursors(-1, -1, cursorIndex + 1, true);
		_vm->_draw->_doCursorPalettes[cursorIndex] = false;
		return;
	}

	Graphics::WinCursorGroup *cursorGroup = 0;
	Graphics::Cursor *defaultCursor = 0;

	// Load the cursor file and cursor group
	if (loadCursorFile())
		cursorGroup = Graphics::WinCursorGroup::createCursorGroup(*_cursors, Common::WinResourceID(cursorName));

	// If the requested cursor does not exist, create a default one
	const Graphics::Cursor *cursor = 0;
	if (!cursorGroup || cursorGroup->cursors.empty() || !cursorGroup->cursors[0].cursor) {
		defaultCursor = Graphics::makeDefaultWinCursor();

		cursor = defaultCursor;
	} else
		cursor = cursorGroup->cursors[0].cursor;

	// Make sure the cursors sprite it big enough
	resizeCursors(cursor->getWidth(), cursor->getHeight(), cursorIndex + 1, true);

	Surface cursorSurf(cursor->getWidth(), cursor->getHeight(), 1, cursor->getSurface());

	_vm->_draw->_cursorSprites->blit(cursorSurf, cursorIndex * _vm->_draw->_cursorWidth, 0);

	memcpy(_vm->_draw->_cursorPalettes + cursorIndex * 256 * 3, cursor->getPalette(), cursor->getPaletteCount() * 3);

	_vm->_draw->_doCursorPalettes   [cursorIndex] = true;
	_vm->_draw->_cursorKeyColors    [cursorIndex] = cursor->getKeyColor();
	_vm->_draw->_cursorPaletteStarts[cursorIndex] = cursor->getPaletteStartIndex();
	_vm->_draw->_cursorPaletteCounts[cursorIndex] = cursor->getPaletteCount();
	_vm->_draw->_cursorHotspotsX    [cursorIndex] = cursor->getHotspotX();
	_vm->_draw->_cursorHotspotsY    [cursorIndex] = cursor->getHotspotY();

	delete cursorGroup;
	delete defaultCursor;
}

void Inter_v7::o7_displayWarning() {
	Common::String caption = _vm->_game->_script->evalString();
	Common::String text    = _vm->_game->_script->evalString();
	Common::String source  = _vm->_game->_script->evalString();
	Common::String msg     = _vm->_game->_script->evalString();
	Common::String param   = _vm->_game->_script->evalString();

	warning("%s: %s (%s)", source.c_str(), msg.c_str(), param.c_str());
}

void Inter_v7::o7_logString() {
	Common::String str0 = _vm->_game->_script->evalString();
	Common::String str1 = _vm->_game->_script->evalString();

	TimeDate t;
	_vm->_system->getTimeAndDate(t);

	debug(1, "%-9s%04d-%02d-%02dT%02d:%02d:%02d --> %s", str0.c_str(),
			t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
			t.tm_hour, t.tm_min, t.tm_sec, str1.c_str());
}

void Inter_v7::o7_intToString() {
	uint16 valueIndex = _vm->_game->_script->readVarIndex();
	uint16 destIndex  = _vm->_game->_script->readVarIndex();

	sprintf(GET_VARO_STR(destIndex), "%d", (int32)READ_VARO_UINT32(valueIndex));
}

void Inter_v7::o7_callFunction() {
	Common::String tot      = _vm->_game->_script->evalString();
	Common::String function = _vm->_game->_script->evalString();

	int16 param = _vm->_game->_script->readValExpr();

	if (!tot.contains('.'))
		tot += ".TOT";

	_vm->_game->callFunction(tot, function, param);
}

void Inter_v7::o7_loadFunctions() {
	Common::String tot = _vm->_game->_script->evalString();

	int16 flags = _vm->_game->_script->readValExpr();

	if (!tot.contains('.'))
		tot += ".TOT";

	_vm->_game->loadFunctions(tot, flags);
}

void Inter_v7::o7_playVmdOrMusic() {
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

	if (file == "RIEN") {
		_vm->_vidPlayer->closeAll();
		return;
	}

	bool close = false;
	if (props.lastFrame == -1) {
		close = true;
	} else if (props.lastFrame == -3) {

		if (file.empty()) {
			_vm->_vidPlayer->closeVideo(_vm->_mult->_objects[props.startFrame].videoSlot - 1);
			_vm->_mult->_objects[props.startFrame].videoSlot = 0;
			return;
		}

		props.flags  = VideoPlayer::kFlagOtherSurface;
		props.sprite = -1;

		_vm->_mult->_objects[props.startFrame].pAnimData->animation = -props.startFrame - 1;

		if (_vm->_mult->_objects[props.startFrame].videoSlot > 0)
			_vm->_vidPlayer->closeVideo(_vm->_mult->_objects[props.startFrame].videoSlot - 1);

		uint32 x = props.x;
		uint32 y = props.y;

		int slot = _vm->_vidPlayer->openVideo(false, file, props);

		_vm->_mult->_objects[props.startFrame].videoSlot = slot + 1;

		if (x == 0xFFFFFFFF) {
			*_vm->_mult->_objects[props.startFrame].pPosX = _vm->_vidPlayer->getDefaultX(slot);
			*_vm->_mult->_objects[props.startFrame].pPosY = _vm->_vidPlayer->getDefaultY(slot);
		} else {
			*_vm->_mult->_objects[props.startFrame].pPosX = x;
			*_vm->_mult->_objects[props.startFrame].pPosY = y;
		}

		return;
	} else if (props.lastFrame == -4) {
		warning("Woodruff Stub: Video/Music command -4: Play background video %s", file.c_str());
		return;
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
void Inter_v7::o7_draw0x89() {
	Common::String str0 = _vm->_game->_script->evalString();
	Common::String str1 = _vm->_game->_script->evalString();

	warning("Addy Stub Draw 0x89: \"%s\", \"%s\"", str0.c_str(), str1.c_str());

	if (findFile(str0).empty()) {
		storeValue(0);
		return;
	}

	storeValue(1);
}

void Inter_v7::o7_findFile() {
	Common::String file = findFile(getFile(_vm->_game->_script->evalString()));

	storeString(file.c_str());
	storeValue(file.empty() ? 0 : 1);
}

void Inter_v7::o7_findCDFile() {
	Common::String mask = getFile(GET_VARO_STR(_vm->_game->_script->readVarIndex()));
	Common::String file = findFile(mask);

	warning("Addy Stub: Find CD file \"%s\"", mask.c_str());
	storeValue(0);
}

void Inter_v7::o7_getSystemProperty() {
	const char *property = _vm->_game->_script->evalString();
	if (!scumm_stricmp(property, "TotalPhys")) {
		// HACK
		storeValue(1000000);
		return;
	}

	if (!scumm_stricmp(property, "AvailPhys")) {
		// HACK
		storeValue(1000000);
		return;
	}

	if (!scumm_stricmp(property, "TimeGMT")) {
		renewTimeInVars();
		storeValue(0);
		return;
	}

	warning("Inter_v7::o7_getSystemProperty(): Unknown property \"%s\"", property);
	storeValue(0);
}

void Inter_v7::o7_loadImage() {
	Common::String file = _vm->_game->_script->evalString();
	if (!file.contains('.'))
		file += ".TGA";

	int16 spriteIndex = _vm->_game->_script->readValExpr();
	int16 left        = _vm->_game->_script->readValExpr();
	int16 top         = _vm->_game->_script->readValExpr();
	int16 width       = _vm->_game->_script->readValExpr();
	int16 height      = _vm->_game->_script->readValExpr();
	int16 x           = _vm->_game->_script->readValExpr();
	int16 y           = _vm->_game->_script->readValExpr();
	int16 transp      = _vm->_game->_script->readValExpr();

	if (spriteIndex > 100)
		spriteIndex -= 80;

	if ((spriteIndex < 0) || (spriteIndex >= Draw::kSpriteCount)) {
		warning("o7_loadImage(): Sprite %d out of range", spriteIndex);
		return;
	}

	SurfacePtr destSprite = _vm->_draw->_spritesArray[spriteIndex];
	if (!destSprite) {
		warning("o7_loadImage(): Sprite %d does not exist", spriteIndex);
		return;
	}

	Common::SeekableReadStream *imageFile = _vm->_dataIO->getFile(file);
	if (!imageFile) {
		warning("o7_loadImage(): No such file \"%s\"", file.c_str());
		return;
	}

	SurfacePtr image = _vm->_video->initSurfDesc(1, 1);
	if (!image->loadImage(*imageFile)) {
		warning("o7_loadImage(): Failed to load image \"%s\"", file.c_str());
		return;
	}

	int16 right  = left + width  - 1;
	int16 bottom = top  + height - 1;
	destSprite->blit(*image, left, top, right, bottom, x, y, transp);
}

void Inter_v7::o7_setVolume() {
	uint32 volume = _vm->_game->_script->readValExpr();

	warning("Addy Stub: Set volume %d (0 - 100)", volume);
}

void Inter_v7::o7_zeroVar() {
	uint16 index = _vm->_game->_script->readVarIndex();

	WRITE_VARO_UINT32(index, 0);
}

void Inter_v7::o7_getINIValue() {
	Common::String file = getFile(_vm->_game->_script->evalString());

	Common::String section = _vm->_game->_script->evalString();
	Common::String key     = _vm->_game->_script->evalString();
	Common::String def     = _vm->_game->_script->evalString();

	Common::String value;
	_inis.getValue(value, file, section, key, def);

	storeString(value.c_str());
}

void Inter_v7::o7_setINIValue() {
	Common::String file = getFile(_vm->_game->_script->evalString());

	Common::String section = _vm->_game->_script->evalString();
	Common::String key     = _vm->_game->_script->evalString();
	Common::String value   = _vm->_game->_script->evalString();

	_inis.setValue(file, section, key, value);
}

void Inter_v7::o7_loadLBMPalette() {
	Common::String file = _vm->_game->_script->evalString();
	if (!file.contains('.'))
		file += ".LBM";

	int16 startIndex = CLIP<int16>(_vm->_game->_script->readValExpr(), 0, 255);
	int16 stopIndex  = CLIP<int16>(_vm->_game->_script->readValExpr(), 0, 255);

	if (startIndex > stopIndex)
		SWAP(startIndex, stopIndex);

	Common::SeekableReadStream *lbmFile = _vm->_dataIO->getFile(file);
	if (!lbmFile) {
		warning("o7_loadLBMPalette(): No such file \"%s\"", file.c_str());
		return;
	}

	ImageType type = Surface::identifyImage(*lbmFile);
	if (type != kImageTypeLBM) {
		warning("o7_loadLBMPalette(): \"%s\" is no LBM", file.c_str());
		return;
	}

	byte palette[768];

	LBMLoader lbm(*lbmFile);
	if (!lbm.loadPalette(palette)) {
		warning("o7_loadLBMPalette(): Failed reading palette from LBM \"%s\"", file.c_str());
		return;
	}

	memset(palette      , 0x00, 3);
	memset(palette + 765, 0xFF, 3);
	for (int i = 0; i < 768; i++)
		palette[i] >>= 2;

	int16 count = stopIndex - startIndex + 1;

	startIndex *= 3;
	count      *= 3;

	memcpy((char *)_vm->_draw->_vgaPalette + startIndex, palette + startIndex, count);
	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
}

void Inter_v7::o7_opendBase() {
	Common::String dbFile = _vm->_game->_script->evalString();
	Common::String id     = _vm->_game->_script->evalString();

	dbFile += ".DBF";

	_databases.setLanguage(_vm->_language);
	if (!_databases.open(id, dbFile)) {
		WRITE_VAR(27, 0); // Failure
		return;
	}

	WRITE_VAR(27, 1); // Success
}

void Inter_v7::o7_closedBase() {
	Common::String id = _vm->_game->_script->evalString();

	if (_databases.close(id))
		WRITE_VAR(27, 1); // Success
	else
		WRITE_VAR(27, 0); // Failure
}

void Inter_v7::o7_getDBString() {
	Common::String id      = _vm->_game->_script->evalString();
	Common::String group   = _vm->_game->_script->evalString();
	Common::String section = _vm->_game->_script->evalString();
	Common::String keyword = _vm->_game->_script->evalString();

	Common::String result;
	if (!_databases.getString(id, group, section, keyword, result)) {
		WRITE_VAR(27, 0); // Failure
		storeString("");
		return;
	}

	storeString(result.c_str());
	WRITE_VAR(27, 1); // Success
}

void Inter_v7::o7_oemToANSI(OpGobParams &params) {
	_vm->_game->_script->skip(2);
}

void Inter_v7::o7_gob0x201(OpGobParams &params) {
	uint16 varIndex = _vm->_game->_script->readUint16();

	WRITE_VAR(varIndex, 1);
}

Common::String Inter_v7::findFile(const Common::String &mask) {
	Common::ArchiveMemberList files;

	SearchMan.listMatchingMembers(files, mask);

	if (files.empty())
		return "";

	return files.front()->getName();
}

bool Inter_v7::loadCursorFile() {
	if (_cursors)
		return true;

	_cursors = new Common::PEResources();

	if (_cursors->loadFromEXE("cursor32.dll"))
		return true;

	delete _cursors;
	_cursors = 0;

	return false;
}

} // End of namespace Gob
