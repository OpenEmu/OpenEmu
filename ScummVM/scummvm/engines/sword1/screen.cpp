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


#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "graphics/palette.h"

#include "sword1/screen.h"
#include "sword1/logic.h"
#include "sword1/sworddefs.h"
#include "sword1/text.h"
#include "sword1/resman.h"
#include "sword1/objectman.h"
#include "sword1/menu.h"
#include "sword1/swordres.h"
#include "sword1/sword1.h"

namespace Sword1 {

#define SCROLL_FRACTION 16
#define MAX_SCROLL_DISTANCE 8
#define FADE_UP 1
#define FADE_DOWN -1

Screen::Screen(OSystem *system, ResMan *pResMan, ObjectMan *pObjMan) {
	_system = system;
	_resMan = pResMan;
	_objMan = pObjMan;
	_screenBuf = _screenGrid = NULL;
	_backLength = _foreLength = _sortLength = 0;
	_fadingStep = 0;
	_currentScreen = 0xFFFF;
	_updatePalette = false;
	_psxCache.decodedBackground = NULL;
	_psxCache.extPlxCache = NULL;
	_oldScrollX = 0;
	_oldScrollY = 0;
}

Screen::~Screen() {
	free(_screenBuf);
	free(_screenGrid);
	if (_currentScreen != 0xFFFF)
		quitScreen();
}

void Screen::clearScreen() {
	if (_screenBuf) {
		_fullRefresh = true;
		memset(_screenBuf, 0, _scrnSizeX * _scrnSizeY);
		_system->fillScreen(0);
	}
}

void Screen::useTextManager(Text *pTextMan) {
	_textMan = pTextMan;
}

void Screen::setScrolling(int16 offsetX, int16 offsetY) {
	offsetX = CLIP<int32>(offsetX, 0, Logic::_scriptVars[MAX_SCROLL_OFFSET_X]);
	offsetY = CLIP<int32>(offsetY, 0, Logic::_scriptVars[MAX_SCROLL_OFFSET_Y]);

	if (Logic::_scriptVars[SCROLL_FLAG] == 2) { // first time on this screen - need absolute scroll immediately!
		_oldScrollX = Logic::_scriptVars[SCROLL_OFFSET_X] = (uint32)offsetX;
		_oldScrollY = Logic::_scriptVars[SCROLL_OFFSET_Y] = (uint32)offsetY;
		Logic::_scriptVars[SCROLL_FLAG] = 1;
		_fullRefresh = true;
	} else if (Logic::_scriptVars[SCROLL_FLAG] == 1) {
		// Because parallax layers may be drawn on the old scroll offset, we
		// want a full refresh not only when the scroll offset changes, but
		// also on the frame where they become the same.
		if (_oldScrollX != Logic::_scriptVars[SCROLL_OFFSET_X] || _oldScrollY != Logic::_scriptVars[SCROLL_OFFSET_Y])
			_fullRefresh = true;
		_oldScrollX = Logic::_scriptVars[SCROLL_OFFSET_X];
		_oldScrollY = Logic::_scriptVars[SCROLL_OFFSET_Y];
		int dx = offsetX - Logic::_scriptVars[SCROLL_OFFSET_X];
		int dy = offsetY - Logic::_scriptVars[SCROLL_OFFSET_Y];
		int scrlDistX = CLIP<int32>((((SCROLL_FRACTION - 1) + ABS(dx)) / SCROLL_FRACTION) * ((dx > 0) ? 1 : -1), -MAX_SCROLL_DISTANCE, MAX_SCROLL_DISTANCE);
		int scrlDistY = CLIP<int32>((((SCROLL_FRACTION - 1) + ABS(dy)) / SCROLL_FRACTION) * ((dy > 0) ? 1 : -1), -MAX_SCROLL_DISTANCE, MAX_SCROLL_DISTANCE);
		if ((scrlDistX != 0) || (scrlDistY != 0))
			_fullRefresh = true;
		Logic::_scriptVars[SCROLL_OFFSET_X] = CLIP<int32>(Logic::_scriptVars[SCROLL_OFFSET_X] + scrlDistX, 0, Logic::_scriptVars[MAX_SCROLL_OFFSET_X]);
		Logic::_scriptVars[SCROLL_OFFSET_Y] = CLIP<int32>(Logic::_scriptVars[SCROLL_OFFSET_Y] + scrlDistY, 0, Logic::_scriptVars[MAX_SCROLL_OFFSET_Y]);
	} else {
		// SCROLL_FLAG == 0, this usually means that the screen is smaller than 640x400 and doesn't need scrolling at all
		// however, it can also mean that the gamescript overwrote the scrolling flag to take care of scrolling directly,
		// (see bug report #1345130) so we ignore the offset arguments in this case
		Logic::_scriptVars[SCROLL_OFFSET_X] = CLIP<int32>(Logic::_scriptVars[SCROLL_OFFSET_X], 0, Logic::_scriptVars[MAX_SCROLL_OFFSET_X]);
		Logic::_scriptVars[SCROLL_OFFSET_Y] = CLIP<int32>(Logic::_scriptVars[SCROLL_OFFSET_Y], 0, Logic::_scriptVars[MAX_SCROLL_OFFSET_Y]);
		if ((Logic::_scriptVars[SCROLL_OFFSET_X] != _oldScrollX) || (Logic::_scriptVars[SCROLL_OFFSET_Y] != _oldScrollY)) {
			_fullRefresh = true;
			_oldScrollX = Logic::_scriptVars[SCROLL_OFFSET_X];
			_oldScrollY = Logic::_scriptVars[SCROLL_OFFSET_Y];
		}
	}
}

void Screen::fadeDownPalette() {
	if (!_isBlack) { // don't fade down twice
		_fadingStep = 15;
		_fadingDirection = FADE_DOWN;
	}
}

void Screen::fadeUpPalette() {
	_fadingStep = 1;
	_fadingDirection = FADE_UP;
}

void Screen::fnSetPalette(uint8 start, uint16 length, uint32 id, bool fadeUp) {
	uint8 *palData = (uint8 *)_resMan->openFetchRes(id);
	if (start == 0) // force color 0 to black
		palData[0] = palData[1] = palData[2] = 0;

	if (SwordEngine::isMac()) {  // see bug #1701058
		if (start != 0 && start + length == 256) // and force color 255 to black as well
			palData[(length - 1) * 3 + 0] = palData[(length - 1) * 3 + 1] = palData[(length - 1) * 3 + 2] = 0;
	}

	for (uint32 cnt = 0; cnt < length; cnt++) {
		_targetPalette[(start + cnt) * 3 + 0] = palData[cnt * 3 + 0] << 2;
		_targetPalette[(start + cnt) * 3 + 1] = palData[cnt * 3 + 1] << 2;
		_targetPalette[(start + cnt) * 3 + 2] = palData[cnt * 3 + 2] << 2;
	}
	_resMan->resClose(id);
	_isBlack = false;
	if (fadeUp) {
		_fadingStep = 1;
		_fadingDirection = FADE_UP;
		memset(_currentPalette, 0, 256 * 3);
		_system->getPaletteManager()->setPalette(_currentPalette, 0, 256);
	} else
		_system->getPaletteManager()->setPalette(_targetPalette + 3 * start, start, length);
}

void Screen::fullRefresh() {
	_fullRefresh = true;
	_system->getPaletteManager()->setPalette(_targetPalette, 0, 256);
}

bool Screen::stillFading() {
	return (_fadingStep != 0);
}

bool Screen::showScrollFrame() {
	if ((!_fullRefresh) || Logic::_scriptVars[NEW_PALETTE] || _updatePalette)
		return false; // don't draw an additional frame if we aren't scrolling or have to change the palette
	if ((_oldScrollX == Logic::_scriptVars[SCROLL_OFFSET_X]) &&
	        (_oldScrollY == Logic::_scriptVars[SCROLL_OFFSET_Y]))
		return false; // check again if we *really* are scrolling.

	uint16 avgScrlX = (uint16)(_oldScrollX + Logic::_scriptVars[SCROLL_OFFSET_X]) / 2;
	uint16 avgScrlY = (uint16)(_oldScrollY + Logic::_scriptVars[SCROLL_OFFSET_Y]) / 2;

	_system->copyRectToScreen(_screenBuf + avgScrlY * _scrnSizeX + avgScrlX, _scrnSizeX, 0, 40, SCREEN_WIDTH, SCREEN_DEPTH);
	_system->updateScreen();
	return true;
}

void Screen::updateScreen() {
	if (Logic::_scriptVars[NEW_PALETTE]) {
		_fadingStep = 1;
		_fadingDirection = FADE_UP;
		_updatePalette = true;
		Logic::_scriptVars[NEW_PALETTE] = 0;
	}
	if (_updatePalette) {
		fnSetPalette(0, 184, _roomDefTable[_currentScreen].palettes[0], false);
		fnSetPalette(184, 72, _roomDefTable[_currentScreen].palettes[1], false);
		_updatePalette = false;
	}
	if (_fadingStep) {
		fadePalette();
		_system->getPaletteManager()->setPalette(_currentPalette, 0, 256);
	}

	uint16 scrlX = (uint16)Logic::_scriptVars[SCROLL_OFFSET_X];
	uint16 scrlY = (uint16)Logic::_scriptVars[SCROLL_OFFSET_Y];
	if (_fullRefresh) {
		_fullRefresh = false;
		uint16 copyWidth = SCREEN_WIDTH;
		uint16 copyHeight = SCREEN_DEPTH;
		if (scrlX + copyWidth > _scrnSizeX)
			copyWidth = _scrnSizeX - scrlX;
		if (scrlY + copyHeight > _scrnSizeY)
			copyHeight = _scrnSizeY - scrlY;
		_system->copyRectToScreen(_screenBuf + scrlY * _scrnSizeX + scrlX, _scrnSizeX, 0, 40, copyWidth, copyHeight);
	} else {
		// partial screen update only. The screen coordinates probably won't fit to the
		// grid holding the informations on which blocks have to be updated.
		// as the grid will be X pixel higher and Y pixel more to the left, this can be cured
		// by first checking the top border, then the left column and then the remaining (aligned) part.
		uint8 *gridPos = _screenGrid + (scrlX / SCRNGRID_X) + (scrlY / SCRNGRID_Y) * _gridSizeX;
		uint8 *scrnBuf = _screenBuf + scrlY * _scrnSizeX + scrlX;
		uint8 diffX = (uint8)(scrlX % SCRNGRID_X);
		uint8 diffY = (uint8)(scrlY % SCRNGRID_Y);
		uint16 gridW = SCREEN_WIDTH / SCRNGRID_X;
		uint16 gridH = SCREEN_DEPTH / SCRNGRID_Y;
		if (diffY) {
			diffY = SCRNGRID_Y - diffY;
			uint16 cpWidth = 0;
			for (uint16 cntx = 0; cntx < gridW; cntx++)
				if (gridPos[cntx]) {
					gridPos[cntx] >>= 1;
					cpWidth++;
				} else if (cpWidth) {
					int16 xPos = (cntx - cpWidth) * SCRNGRID_X - diffX;
					if (xPos < 0)
						xPos = 0;
					_system->copyRectToScreen(scrnBuf + xPos, _scrnSizeX, xPos, 40, cpWidth * SCRNGRID_X, diffY);
					cpWidth = 0;
				}
			if (cpWidth) {
				int16 xPos = (gridW - cpWidth) * SCRNGRID_X - diffX;
				if (xPos < 0)
					xPos = 0;
				_system->copyRectToScreen(scrnBuf + xPos, _scrnSizeX, xPos, 40, SCREEN_WIDTH - xPos, diffY);
			}
			scrlY += diffY;
		}
		// okay, y scrolling is compensated. check x now.
		gridPos = _screenGrid + (scrlX / SCRNGRID_X) + (scrlY / SCRNGRID_Y) * _gridSizeX;
		scrnBuf = _screenBuf + scrlY * _scrnSizeX + scrlX;
		if (diffX) {
			diffX = SCRNGRID_X - diffX;
			uint16 cpHeight = 0;
			for (uint16 cnty = 0; cnty < gridH; cnty++) {
				if (*gridPos) {
					*gridPos >>= 1;
					cpHeight++;
				} else if (cpHeight) {
					uint16 yPos = (cnty - cpHeight) * SCRNGRID_Y;
					_system->copyRectToScreen(scrnBuf + yPos * _scrnSizeX, _scrnSizeX, 0, yPos + diffY + 40, diffX, cpHeight * SCRNGRID_Y);
					cpHeight = 0;
				}
				gridPos += _gridSizeX;
			}
			if (cpHeight) {
				uint16 yPos = (gridH - cpHeight) * SCRNGRID_Y;
				_system->copyRectToScreen(scrnBuf + yPos * _scrnSizeX, _scrnSizeX, 0, yPos + diffY + 40, diffX, SCREEN_DEPTH - (yPos + diffY));
			}
			scrlX += diffX;
		}
		// x scroll is compensated, too. check the rest of the screen, now.
		scrnBuf = _screenBuf + scrlY * _scrnSizeX + scrlX;
		gridPos = _screenGrid + (scrlX / SCRNGRID_X) + (scrlY / SCRNGRID_Y) * _gridSizeX;
		for (uint16 cnty = 0; cnty < gridH; cnty++) {
			uint16 cpWidth = 0;
			uint16 cpHeight = SCRNGRID_Y;
			if (cnty == gridH - 1)
				cpHeight = SCRNGRID_Y - diffY;
			for (uint16 cntx = 0; cntx < gridW; cntx++)
				if (gridPos[cntx]) {
					gridPos[cntx] >>= 1;
					cpWidth++;
				} else if (cpWidth) {
					_system->copyRectToScreen(scrnBuf + (cntx - cpWidth) * SCRNGRID_X, _scrnSizeX, (cntx - cpWidth) * SCRNGRID_X + diffX, cnty * SCRNGRID_Y + diffY + 40, cpWidth * SCRNGRID_X, cpHeight);
					cpWidth = 0;
				}
			if (cpWidth) {
				uint16 xPos = (gridW - cpWidth) * SCRNGRID_X;
				_system->copyRectToScreen(scrnBuf + xPos, _scrnSizeX, xPos + diffX, cnty * SCRNGRID_Y + diffY + 40, SCREEN_WIDTH - (xPos + diffX), cpHeight);
			}
			gridPos += _gridSizeX;
			scrnBuf += _scrnSizeX * SCRNGRID_Y;
		}
	}
	_system->updateScreen();
}

void Screen::newScreen(uint32 screen) {
	uint8 cnt;
	// set sizes and scrolling, initialize/load screengrid, force screen refresh
	_currentScreen = screen;
	_scrnSizeX = _roomDefTable[screen].sizeX;
	_scrnSizeY = _roomDefTable[screen].sizeY;
	_gridSizeX = _scrnSizeX / SCRNGRID_X;
	_gridSizeY = _scrnSizeY / SCRNGRID_Y;
	if ((_scrnSizeX % SCRNGRID_X) || (_scrnSizeY % SCRNGRID_Y))
		error("Illegal screensize: %d: %d/%d", screen, _scrnSizeX, _scrnSizeY);
	if ((_scrnSizeX > SCREEN_WIDTH) || (_scrnSizeY > SCREEN_DEPTH)) {
		Logic::_scriptVars[SCROLL_FLAG] = 2;
		Logic::_scriptVars[MAX_SCROLL_OFFSET_X] = _scrnSizeX - SCREEN_WIDTH;
		Logic::_scriptVars[MAX_SCROLL_OFFSET_Y] = _scrnSizeY - SCREEN_DEPTH;
	} else {
		Logic::_scriptVars[SCROLL_FLAG] = 0;
		Logic::_scriptVars[MAX_SCROLL_OFFSET_X] = 0;
		Logic::_scriptVars[MAX_SCROLL_OFFSET_Y] = 0;
	}
	Logic::_scriptVars[SCROLL_OFFSET_X] = 0;
	Logic::_scriptVars[SCROLL_OFFSET_Y] = 0;

	free(_screenBuf);
	free(_screenGrid);

	if (SwordEngine::isPsx())
		flushPsxCache();

	_screenBuf = (uint8 *)malloc(_scrnSizeX * _scrnSizeY);
	_screenGrid = (uint8 *)malloc(_gridSizeX * _gridSizeY);
	memset(_screenGrid, 0, _gridSizeX * _gridSizeY);
	for (cnt = 0; cnt < _roomDefTable[_currentScreen].totalLayers; cnt++) {
		// open and lock all resources, will be closed in quitScreen()
		_layerBlocks[cnt] = (uint8 *)_resMan->openFetchRes(_roomDefTable[_currentScreen].layers[cnt]);
		if (cnt > 0)
			_layerBlocks[cnt] += sizeof(Header);
	}
	for (cnt = 0; cnt < _roomDefTable[_currentScreen].totalLayers - 1; cnt++) {
		// there's no grid for the background layer, so it's totalLayers - 1
		_layerGrid[cnt] = (uint16 *)_resMan->openFetchRes(_roomDefTable[_currentScreen].grids[cnt]);
		_layerGrid[cnt] += 14;
	}
	_parallax[0] = _parallax[1] = NULL;
	if (_roomDefTable[_currentScreen].parallax[0])
		_parallax[0] = (uint8 *)_resMan->openFetchRes(_roomDefTable[_currentScreen].parallax[0]);
	if (_roomDefTable[_currentScreen].parallax[1])
		_parallax[1] = (uint8 *)_resMan->openFetchRes(_roomDefTable[_currentScreen].parallax[1]);

	_updatePalette = true;
	_fullRefresh = true;
}

void Screen::quitScreen() {
	uint8 cnt;
	if (SwordEngine::isPsx())
		flushPsxCache();
	for (cnt = 0; cnt < _roomDefTable[_currentScreen].totalLayers; cnt++)
		_resMan->resClose(_roomDefTable[_currentScreen].layers[cnt]);
	for (cnt = 0; cnt < _roomDefTable[_currentScreen].totalLayers - 1; cnt++)
		_resMan->resClose(_roomDefTable[_currentScreen].grids[cnt]);
	if (_roomDefTable[_currentScreen].parallax[0])
		_resMan->resClose(_roomDefTable[_currentScreen].parallax[0]);
	if (_roomDefTable[_currentScreen].parallax[1])
		_resMan->resClose(_roomDefTable[_currentScreen].parallax[1]);
	_currentScreen = 0xFFFF;
}

void Screen::draw() {
	uint8 cnt;

	debug(8, "Screen::draw() -> _currentScreen %u", _currentScreen);

	if (_currentScreen == 54) {
		// rm54 has a BACKGROUND parallax layer in parallax[0]
		if (_parallax[0] && !SwordEngine::isPsx()) //Avoid drawing this parallax on PSX edition, it gets occluded by background
			renderParallax(_parallax[0]);
		uint8 *src = _layerBlocks[0];
		uint8 *dest = _screenBuf;

		if (SwordEngine::isPsx()) {
			if (!_psxCache.decodedBackground)
				_psxCache.decodedBackground = psxShrinkedBackgroundToIndexed(_layerBlocks[0], _scrnSizeX, _scrnSizeY);
			memcpy(_screenBuf, _psxCache.decodedBackground, _scrnSizeX * _scrnSizeY);
		} else {
			uint16 scrnScrlY = MIN((uint32)_oldScrollY, Logic::_scriptVars[SCROLL_OFFSET_Y]);
			uint16 scrnHeight = SCREEN_DEPTH + ABS((int32)_oldScrollY - (int32)Logic::_scriptVars[SCROLL_OFFSET_Y]);

			src += scrnScrlY * _scrnSizeX;
			dest += scrnScrlY * _scrnSizeX;

			// In this background to create transparency we have to iterate through all pixels, avoid checking those out of screen
			for (uint16 cnty = scrnScrlY; (cnty < _scrnSizeY) && (cnty < scrnHeight + scrnScrlY); cnty++)
				for (uint16 cntx = 0; cntx < _scrnSizeX; cntx++) {
					if (*src)
						if (!(SwordEngine::isMac()) || *src != 255) // see bug #1701058
							*dest = *src;
					src++;
					dest++;
				}
		}

	} else if (!(SwordEngine::isPsx())) {
		memcpy(_screenBuf, _layerBlocks[0], _scrnSizeX * _scrnSizeY);
	} else { //We are using PSX version
		if (_currentScreen == 45 || _currentScreen == 55 ||
		        _currentScreen == 57 || _currentScreen == 63 || _currentScreen == 71) { // Width shrinked backgrounds
			if (!_psxCache.decodedBackground)
				_psxCache.decodedBackground = psxShrinkedBackgroundToIndexed(_layerBlocks[0], _scrnSizeX, _scrnSizeY);
		} else {
			if (!_psxCache.decodedBackground)
				_psxCache.decodedBackground = psxBackgroundToIndexed(_layerBlocks[0], _scrnSizeX, _scrnSizeY);
		}
		memcpy(_screenBuf, _psxCache.decodedBackground, _scrnSizeX * _scrnSizeY);
	}

	for (cnt = 0; cnt < _backLength; cnt++)
		processImage(_backList[cnt]);

	for (cnt = 0; cnt < _sortLength - 1; cnt++)
		for (uint8 sCnt = 0; sCnt < _sortLength - 1; sCnt++)
			if (_sortList[sCnt].y > _sortList[sCnt + 1].y) {
				SWAP(_sortList[sCnt], _sortList[sCnt + 1]);
			}
	for (cnt = 0; cnt < _sortLength; cnt++)
		processImage(_sortList[cnt].id);

	if ((_currentScreen != 54) && _parallax[0])
		renderParallax(_parallax[0]); // screens other than 54 have FOREGROUND parallax layer in parallax[0]
	if (_parallax[1])
		renderParallax(_parallax[1]);

	// PSX version has parallax layer for this room in an external file (TRAIN.PLX)
	if (SwordEngine::isPsx() && _currentScreen == 63) {
		// FIXME: this should be handled in a cleaner way...
		if (!_psxCache.extPlxCache) {
			Common::File parallax;
			parallax.open("TRAIN.PLX");
			_psxCache.extPlxCache = (uint8 *)malloc(parallax.size());
			parallax.read(_psxCache.extPlxCache, parallax.size());
			parallax.close();
		}
		renderParallax(_psxCache.extPlxCache);
	}

	for (cnt = 0; cnt < _foreLength; cnt++)
		processImage(_foreList[cnt]);

	_backLength = _sortLength = _foreLength = 0;
}

void Screen::processImage(uint32 id) {
	Object *compact;
	FrameHeader *frameHead;
	int scale;

	compact = _objMan->fetchObject(id);

	if (compact->o_type == TYPE_TEXT)
		frameHead = _textMan->giveSpriteData((uint8)compact->o_target);
	else
		frameHead = _resMan->fetchFrame(_resMan->openFetchRes(compact->o_resource), compact->o_frame);

	uint8 *sprData = ((uint8 *)frameHead) + sizeof(FrameHeader);

	uint16 spriteX = compact->o_anim_x;
	uint16 spriteY = compact->o_anim_y;

	if (compact->o_status & STAT_SHRINK) {
		scale = (compact->o_scale_a * compact->o_ycoord + compact->o_scale_b) / 256;
		spriteX += ((int16)_resMan->readUint16(&frameHead->offsetX) * scale) / 256;
		spriteY += ((int16)_resMan->readUint16(&frameHead->offsetY) * scale) / 256;
	} else {
		scale = 256;
		spriteX += (int16)_resMan->readUint16(&frameHead->offsetX);
		spriteY += (int16)_resMan->readUint16(&frameHead->offsetY);
	}

	uint8 *tonyBuf = NULL;
	uint8 *hifBuf = NULL;
	if (SwordEngine::isPsx() && compact->o_type != TYPE_TEXT) { // PSX sprites are compressed with HIF
		hifBuf = (uint8 *)malloc(_resMan->readUint16(&frameHead->width) * _resMan->readUint16(&frameHead->height) / 2);
		memset(hifBuf, 0x00, (_resMan->readUint16(&frameHead->width) * _resMan->readUint16(&frameHead->height) / 2));
		decompressHIF(sprData, hifBuf);
		sprData = hifBuf;
	} else if (frameHead->runTimeComp[3] == '7') { // RLE7 encoded?
		decompressRLE7(sprData, _resMan->readUint32(&frameHead->compSize), _rleBuffer);
		sprData = _rleBuffer;
	} else if (frameHead->runTimeComp[3] == '0') { // RLE0 encoded?
		decompressRLE0(sprData, _resMan->readUint32(&frameHead->compSize), _rleBuffer);
		sprData = _rleBuffer;
	} else if (frameHead->runTimeComp[1] == 'I') { // new type
		tonyBuf = (uint8 *)malloc(_resMan->readUint16(&frameHead->width) * _resMan->readUint16(&frameHead->height));
		decompressTony(sprData, _resMan->readUint32(&frameHead->compSize), tonyBuf);
		sprData = tonyBuf;
	}

	uint16 sprSizeX, sprSizeY;
	if (compact->o_status & STAT_SHRINK) {
		memset(_shrinkBuffer, 0, SHRINK_BUFFER_SIZE); //Clean shrink buffer to avoid corruption
		if (SwordEngine::isPsx() && (compact->o_resource != GEORGE_MEGA)) { //PSX Height shrinked sprites
			sprSizeX = (scale * _resMan->readUint16(&frameHead->width)) / 256;
			sprSizeY = (scale * (_resMan->readUint16(&frameHead->height))) / 256 / 2;
			fastShrink(sprData, _resMan->readUint16(&frameHead->width), (_resMan->readUint16(&frameHead->height)) / 2, scale, _shrinkBuffer);
		} else if (SwordEngine::isPsx()) { //PSX width/height shrinked sprites
			sprSizeX = (scale * _resMan->readUint16(&frameHead->width)) / 256 / 2;
			sprSizeY = (scale * _resMan->readUint16(&frameHead->height)) / 256 / 2;
			fastShrink(sprData, _resMan->readUint16(&frameHead->width) / 2, _resMan->readUint16(&frameHead->height) / 2, scale, _shrinkBuffer);
		} else {
			sprSizeX = (scale * _resMan->readUint16(&frameHead->width)) / 256;
			sprSizeY = (scale * _resMan->readUint16(&frameHead->height)) / 256;
			fastShrink(sprData, _resMan->readUint16(&frameHead->width), _resMan->readUint16(&frameHead->height), scale, _shrinkBuffer);
		}
		sprData = _shrinkBuffer;
	} else {
		sprSizeX = _resMan->readUint16(&frameHead->width);
		if (SwordEngine::isPsx()) { //PSX sprites are half height
			sprSizeY = _resMan->readUint16(&frameHead->height) / 2;
		} else
			sprSizeY = (_resMan->readUint16(&frameHead->height));
	}

	if (!(compact->o_status & STAT_OVERRIDE)) {
		//mouse size linked to exact size & coordinates of sprite box - shrink friendly
		if (_resMan->readUint16(&frameHead->offsetX) || _resMan->readUint16(&frameHead->offsetY)) {
			//for megas the mouse area is reduced to account for sprite not
			//filling the box size is reduced to 1/2 width, 4/5 height
			compact->o_mouse_x1 = spriteX + sprSizeX / 4;
			compact->o_mouse_x2 = spriteX + (3 * sprSizeX) / 4;
			compact->o_mouse_y1 = spriteY + sprSizeY / 10;
			compact->o_mouse_y2 = spriteY + (9 * sprSizeY) / 10;
		} else {
			compact->o_mouse_x1 = spriteX;
			compact->o_mouse_x2 = spriteX + sprSizeX;
			compact->o_mouse_y1 = spriteY;
			compact->o_mouse_y2 = spriteY + sprSizeY;
		}
	}

	uint16 sprPitch = sprSizeX;
	uint16 incr;
	spriteClipAndSet(&spriteX, &spriteY, &sprSizeX, &sprSizeY, &incr);

	if ((sprSizeX > 0) && (sprSizeY > 0)) {
		if ((!(SwordEngine::isPsx()) || (compact->o_type == TYPE_TEXT)
		        || (compact->o_resource == LVSFLY) || (!(compact->o_resource == GEORGE_MEGA) && (sprSizeX < 260))))
			drawSprite(sprData + incr, spriteX, spriteY, sprSizeX, sprSizeY, sprPitch);
		else if (((sprSizeX >= 260) && (sprSizeX < 450)) || ((compact->o_resource == GMWRITH) && (sprSizeX < 515))  // a psx shrinked sprite (1/2 width)
		         || ((compact->o_resource == GMPOWER) && (sprSizeX < 515)))                                         // some needs to be hardcoded, headers don't give useful infos
			drawPsxHalfShrinkedSprite(sprData + incr, spriteX, spriteY, sprSizeX / 2, sprSizeY, sprPitch / 2);
		else if (sprSizeX >= 450) // A PSX double shrinked sprite (1/3 width)
			drawPsxFullShrinkedSprite(sprData + incr, spriteX, spriteY, sprSizeX / 3, sprSizeY, sprPitch / 3);
		else // This is for psx half shrinked, walking george and remaining sprites
			drawPsxHalfShrinkedSprite(sprData + incr, spriteX, spriteY, sprSizeX, sprSizeY, sprPitch);
		if (!(compact->o_status & STAT_FORE) && !(SwordEngine::isPsx() && (compact->o_resource == MOUBUSY))) // Check fixes moue sprite being masked by layer, happens only on psx
			verticalMask(spriteX, spriteY, sprSizeX, sprSizeY);
	}

	if (compact->o_type != TYPE_TEXT)
		_resMan->resClose(compact->o_resource);

	free(tonyBuf);
	free(hifBuf);
}

void Screen::verticalMask(uint16 x, uint16 y, uint16 bWidth, uint16 bHeight) {
	if (_roomDefTable[_currentScreen].totalLayers <= 1)
		return;

	if (SwordEngine::isPsx()) { // PSX sprites are vertical shrinked, and some width shrinked
		bHeight *= 2;
		bWidth *= 2;
	}

	bWidth = (bWidth + (x & (SCRNGRID_X - 1)) + (SCRNGRID_X - 1)) / SCRNGRID_X;
	bHeight = (bHeight + (y & (SCRNGRID_Y - 1)) + (SCRNGRID_Y - 1)) / SCRNGRID_Y;

	x /= SCRNGRID_X;
	y /= SCRNGRID_Y;
	if (x + bWidth > _gridSizeX)
		bWidth = _gridSizeX - x;
	if (y + bHeight > _gridSizeY)
		bHeight = _gridSizeY - y;

	uint16 gridY = y + SCREEN_TOP_EDGE / SCRNGRID_Y; // imaginary screen on top
	gridY += bHeight - 1; // we start from the bottom edge
	uint16 gridX = x + SCREEN_LEFT_EDGE / SCRNGRID_X; // imaginary screen left
	uint16 lGridSizeX = _gridSizeX + 2 * (SCREEN_LEFT_EDGE / SCRNGRID_X); // width of the grid for the imaginary screen

	for (uint16 blkx = 0; blkx < bWidth; blkx++) {
		// A sprite can be masked by several layers at the same time,
		// so we have to check them all. See bug #917427.
		for (int16 level = _roomDefTable[_currentScreen].totalLayers - 2; level >= 0; level--) {
			if (_layerGrid[level][gridX + blkx + gridY * lGridSizeX]) {
				uint16 *grid = _layerGrid[level] + gridX + blkx + gridY * lGridSizeX;
				for (int16 blky = bHeight - 1; blky >= 0; blky--) {
					if (*grid) {
						uint8 *blkData;
						if (SwordEngine::isPsx())
							blkData = _layerBlocks[level + 1] + (_resMan->readUint16(grid) - 1) * 64; //PSX layers are half height too...
						else
							blkData = _layerBlocks[level + 1] + (_resMan->readUint16(grid) - 1) * 128;
						blitBlockClear(x + blkx, y + blky, blkData);
					} else
						break;
					grid -= lGridSizeX;
				}
			}
		}
	}
}

void Screen::blitBlockClear(uint16 x, uint16 y, uint8 *data) {
	uint8 *dest = _screenBuf + (y * SCRNGRID_Y) * _scrnSizeX + (x * SCRNGRID_X);

	for (uint8 cnty = 0; cnty < (SwordEngine::isPsx() ? SCRNGRID_Y / 2 : SCRNGRID_Y); cnty++) {
		for (uint8 cntx = 0; cntx < SCRNGRID_X; cntx++)
			if (data[cntx])
				dest[cntx] = data[cntx];

		if (SwordEngine::isPsx()) {
			dest += _scrnSizeX;
			for (uint8 cntx = 0; cntx < SCRNGRID_X; cntx++)
				if (data[cntx])
					dest[cntx] = data[cntx];
		}

		data += SCRNGRID_X;
		dest += _scrnSizeX;
	}
}

void Screen::renderParallax(uint8 *data) {
	uint16 paraScrlX, paraScrlY;
	uint16 scrnScrlX, scrnScrlY;
	uint16 scrnWidth, scrnHeight;
	uint16 paraSizeX, paraSizeY;
	ParallaxHeader *header = NULL;
	uint32 *lineIndexes = NULL;

	if (SwordEngine::isPsx()) //Parallax headers are different in PSX version
		fetchPsxParallaxSize(data, &paraSizeX, &paraSizeY);
	else {
		header = (ParallaxHeader *)data;
		lineIndexes = (uint32 *)(data + sizeof(ParallaxHeader));
		paraSizeX = _resMan->getUint16(header->sizeX);
		paraSizeY = _resMan->getUint16(header->sizeY);
	}

	assert((paraSizeX >= SCREEN_WIDTH) && (paraSizeY >= SCREEN_DEPTH));

	// we have to render more than the visible screen part for displaying scroll frames
	scrnScrlX = MIN((uint32)_oldScrollX, Logic::_scriptVars[SCROLL_OFFSET_X]);
	scrnWidth = SCREEN_WIDTH + ABS((int32)_oldScrollX - (int32)Logic::_scriptVars[SCROLL_OFFSET_X]);
	scrnScrlY = MIN((uint32)_oldScrollY, Logic::_scriptVars[SCROLL_OFFSET_Y]);
	scrnHeight = SCREEN_DEPTH + ABS((int32)_oldScrollY - (int32)Logic::_scriptVars[SCROLL_OFFSET_Y]);


	if (_scrnSizeX != SCREEN_WIDTH) {
		double scrlfx = (paraSizeX - SCREEN_WIDTH) / ((double)(_scrnSizeX - SCREEN_WIDTH));
		paraScrlX = (uint16)(scrnScrlX * scrlfx);
	} else
		paraScrlX = 0;

	if (_scrnSizeY != SCREEN_DEPTH) {
		double scrlfy = (paraSizeY - SCREEN_DEPTH) / ((double)(_scrnSizeY - SCREEN_DEPTH));
		paraScrlY = (uint16)(scrnScrlY * scrlfy);
	} else
		paraScrlY = 0;

	if (SwordEngine::isPsx())
		drawPsxParallax(data, paraScrlX, scrnScrlX, scrnWidth);
	else
		for (uint16 cnty = 0; cnty < scrnHeight; cnty++) {
			uint8 *src = data + _resMan->readUint32(lineIndexes + cnty + paraScrlY);
			uint8 *dest = _screenBuf + scrnScrlX + (cnty + scrnScrlY) * _scrnSizeX;
			uint16 remain = paraScrlX;
			uint16 xPos = 0;
			while (remain) { // skip past the first part of the parallax to get to the right scrolling position
				uint8 doSkip = *src++;
				if (doSkip <= remain)
					remain -= doSkip;
				else {
					xPos = doSkip - remain;
					dest += xPos;
					remain = 0;
				}
				uint8 doCopy = *src++;
				if (doCopy <= remain) {
					remain -= doCopy;
					src += doCopy;
				} else {
					uint16 remCopy = doCopy - remain;
					memcpy(dest, src + remain, remCopy);
					dest += remCopy;
					src += doCopy;
					xPos = remCopy;
					remain = 0;
				}
			}
			while (xPos < scrnWidth) {
				if (uint8 skip = *src++) {
					dest += skip;
					xPos += skip;
				}
				if (xPos < scrnWidth) {
					if (uint8 doCopy = *src++) {
						if (xPos + doCopy > scrnWidth)
							doCopy = scrnWidth - xPos;
						memcpy(dest, src, doCopy);
						dest += doCopy;
						xPos += doCopy;
						src += doCopy;
					}
				}
			}
		}
}

void Screen::drawSprite(uint8 *sprData, uint16 sprX, uint16 sprY, uint16 sprWidth, uint16 sprHeight, uint16 sprPitch) {
	uint8 *dest = _screenBuf + (sprY * _scrnSizeX) + sprX;

	for (uint16 cnty = 0; cnty < sprHeight; cnty++) {
		for (uint16 cntx = 0; cntx < sprWidth; cntx++)
			if (sprData[cntx])
				dest[cntx] = sprData[cntx];

		if (SwordEngine::isPsx()) { //On PSX version we need to double horizontal lines
			dest += _scrnSizeX;
			for (uint16 cntx = 0; cntx < sprWidth; cntx++)
				if (sprData[cntx])
					dest[cntx] = sprData[cntx];
		}

		sprData += sprPitch;
		dest += _scrnSizeX;
	}
}

// Used to draw psx sprites which are 1/2 of original width
void Screen::drawPsxHalfShrinkedSprite(uint8 *sprData, uint16 sprX, uint16 sprY, uint16 sprWidth, uint16 sprHeight, uint16 sprPitch) {
	uint8 *dest = _screenBuf + (sprY * _scrnSizeX) + sprX;

	for (uint16 cnty = 0; cnty < sprHeight; cnty++) {
		for (uint16 cntx = 0; cntx < sprWidth; cntx++)
			if (sprData[cntx]) {
				dest[cntx * 2] = sprData[cntx]; //In these sprites we need to double vetical lines too...
				dest[cntx * 2 + 1] = sprData[cntx];
			}

		dest += _scrnSizeX;
		for (uint16 cntx = 0; cntx < sprWidth; cntx++)
			if (sprData[cntx]) {
				dest[cntx * 2] = sprData[cntx];
				dest[cntx * 2 + 1] = sprData[cntx];
			}

		sprData += sprPitch;
		dest += _scrnSizeX;
	}
}

// Used to draw psx sprites which are 1/3 of original width
void Screen::drawPsxFullShrinkedSprite(uint8 *sprData, uint16 sprX, uint16 sprY, uint16 sprWidth, uint16 sprHeight, uint16 sprPitch) {
	uint8 *dest = _screenBuf + (sprY * _scrnSizeX) + sprX;

	for (uint16 cnty = 0; cnty < sprHeight; cnty++) {
		for (uint16 cntx = 0; cntx < sprWidth; cntx++)
			if (sprData[cntx]) {
				dest[cntx * 3] = sprData[cntx]; //In these sprites we need to double vertical lines too...
				dest[cntx * 3 + 1] = sprData[cntx];
				dest[cntx * 3 + 2] = sprData[cntx];
			}

		dest += _scrnSizeX;
		for (uint16 cntx = 0; cntx < sprWidth; cntx++)
			if (sprData[cntx]) {
				dest[cntx * 3] = sprData[cntx];
				dest[cntx * 3 + 1] = sprData[cntx];
				dest[cntx * 3 + 2] = sprData[cntx];
			}

		sprData += sprPitch;
		dest += _scrnSizeX;
	}
}

// nearest neighbor filter:
void Screen::fastShrink(uint8 *src, uint32 width, uint32 height, uint32 scale, uint8 *dest) {
	uint32 resHeight = (height * scale) >> 8;
	uint32 resWidth = (width * scale) >> 8;
	uint32 step = 0x10000 / scale;
	uint8 columnTab[160];
	uint32 res = step >> 1;

	for (uint16 cnt = 0; cnt < resWidth; cnt++) {
		columnTab[cnt] = (uint8)(res >> 8);
		res += step;
	}

	uint32 newRow = step >> 1;
	uint32 oldRow = 0;

	uint8 *destPos = dest;
	uint16 lnCnt;
	for (lnCnt = 0; lnCnt < resHeight; lnCnt++) {
		while (oldRow < (newRow >> 8)) {
			oldRow++;
			src += width;
		}
		for (uint16 colCnt = 0; colCnt < resWidth; colCnt++) {
			*destPos++ = src[columnTab[colCnt]];
		}
		newRow += step;
	}
	// scaled, now stipple shadows if there are any
	for (lnCnt = 0; lnCnt < resHeight; lnCnt++) {
		uint16 xCnt = lnCnt & 1;
		destPos = dest + lnCnt * resWidth + (lnCnt & 1);
		while (xCnt < resWidth) {
			if (*destPos == 200)
				*destPos = 0;
			destPos += 2;
			xCnt += 2;
		}
	}
}

void Screen::addToGraphicList(uint8 listId, uint32 objId) {
	if (listId == 0) {
		assert(_foreLength < MAX_FORE);
		_foreList[_foreLength++] = objId;
	}
	if (listId == 1) {
		assert(_sortLength < MAX_SORT);
		Object *cpt = _objMan->fetchObject(objId);
		_sortList[_sortLength].id = objId;
		_sortList[_sortLength].y = cpt->o_anim_y; // gives feet coords if boxed mega, otherwise top of sprite box
		if (!(cpt->o_status & STAT_SHRINK)) {     // not a boxed mega using shrinking
			Header *frameRaw = (Header *)_resMan->openFetchRes(cpt->o_resource);
			FrameHeader *frameHead = _resMan->fetchFrame(frameRaw, cpt->o_frame);
			_sortList[_sortLength].y += _resMan->readUint16(&frameHead->height) - 1; // now pointing to base of sprite
			_resMan->resClose(cpt->o_resource);
		}
		_sortLength++;
	}
	if (listId == 2) {
		assert(_backLength < MAX_BACK);
		_backList[_backLength++] = objId;
	}
}

uint8 *Screen::psxBackgroundToIndexed(uint8 *psxBackground, uint32 bakXres, uint32 bakYres) {
	uint32 xresInTiles = bakXres / 16;
	uint32 yresInTiles = ((bakYres / 2) % 16) ? (bakYres / 32) + 1 : (bakYres / 32);
	uint32 totTiles = xresInTiles * yresInTiles;
	uint32 tileYpos = 0; //tile position in a virtual xresInTiles * yresInTiles grid
	uint32 tileXpos = 0;
	uint32 tag = READ_LE_UINT32(psxBackground);

	uint8 *decomp_tile = (uint8 *)malloc(16 * 16); //Tiles are always 16 * 16
	uint8 *fullres_buffer = (uint8 *)malloc(bakXres * yresInTiles * 32);
	memset(fullres_buffer, 0, bakXres * yresInTiles * 32);

	bool isCompressed = (tag == 0x434F4D50);

	psxBackground += 4; //We skip the id tag

	for (uint32 currentTile = 0; currentTile < totTiles; currentTile++) {
		uint32 tileOffset = READ_LE_UINT32(psxBackground + 4 * currentTile);

		if (isCompressed)
			decompressHIF(psxBackground + tileOffset - 4, decomp_tile); //Decompress the tile into decomp_tile
		else
			memcpy(decomp_tile, psxBackground + tileOffset - 4, 16 * 16);

		if (currentTile > 0 && !(currentTile % xresInTiles)) { //Finished a line of tiles, going down
			tileYpos++;
			tileXpos = 0;
		}

		for (byte tileLine = 0; tileLine < 16; tileLine++) { // Copy data to destination buffer
			memcpy(fullres_buffer + tileLine * bakXres * 2 + tileXpos * 16 + tileYpos * bakXres * 16 * 2, decomp_tile + tileLine * 16, 16);
			memcpy(fullres_buffer + tileLine * bakXres * 2 + bakXres + tileXpos * 16 + tileYpos * bakXres * 16 * 2, decomp_tile + tileLine * 16, 16);
		}
		tileXpos++;
	}

	free(decomp_tile);

	return fullres_buffer;
}

// needed because some psx backgrounds are half width and half height
uint8 *Screen::psxShrinkedBackgroundToIndexed(uint8 *psxBackground, uint32 bakXres, uint32 bakYres) {
	uint32 xresInTiles = ((bakXres / 2) % 16) ? (bakXres / 32) + 1 : (bakXres / 32);
	uint32 yresInTiles = ((bakYres / 2) % 16) ? (bakYres / 32) + 1 : (bakYres / 32);
	uint32 totTiles = xresInTiles * yresInTiles;
	uint32 tileYpos = 0; //tile position in a virtual xresInTiles * yresInTiles grid
	uint32 tileXpos = 0;
	uint32 dataBegin = READ_LE_UINT32(psxBackground + 4);

	uint8 *decomp_tile = (uint8 *)malloc(16 * 16); //Tiles are always 16 * 16
	uint8 *fullres_buffer = (uint8 *)malloc(bakXres * (yresInTiles + 1) * 32);
	memset(fullres_buffer, 0, bakXres * (yresInTiles + 1) * 32);

	bool isCompressed = (READ_LE_UINT32(psxBackground) == MKTAG('C', 'O', 'M', 'P'));

	totTiles -= xresInTiles;
	psxBackground += 4; //We skip the id tag

	uint32 currentTile;
	for (currentTile = 0; currentTile < totTiles; currentTile++) {
		uint32 tileOffset = READ_LE_UINT32(psxBackground + 4 * currentTile);

		if (isCompressed)
			decompressHIF(psxBackground + tileOffset - 4, decomp_tile); //Decompress the tile into decomp_tile
		else
			memcpy(decomp_tile, psxBackground + tileOffset - 4, 16 * 16);

		if (currentTile > 0 && !(currentTile % xresInTiles)) { //Finished a line of tiles, going down
			tileYpos++;
			tileXpos = 0;
		}

		for (byte tileLine = 0; tileLine < 16; tileLine++) {
			uint8 *dest = fullres_buffer + tileLine * bakXres * 2 + tileXpos * 32 + tileYpos * bakXres * 16 * 2;
			for (byte tileColumn = 0; tileColumn < 16; tileColumn++) {
				uint8 pixData = *(decomp_tile + tileColumn + tileLine * 16);
				*(dest + tileColumn * 2) = pixData;
				*(dest + tileColumn * 2 + 1) = pixData;
			}
			dest += bakXres;
			for (byte tileColumn = 0; tileColumn < 16; tileColumn++) {
				uint8 pixData = *(decomp_tile + tileColumn + tileLine * 16);
				*(dest + tileColumn * 2) = pixData;
				*(dest + tileColumn * 2 + 1) = pixData;
			}

		}
		tileXpos++;
	}

	//Calculate number of remaining tiles
	uint32 remainingTiles = (dataBegin - (currentTile * 4 + 4)) / 4;

	// Last line of tiles is full width!
	uint32 tileHeight = (remainingTiles == xresInTiles * 2) ? 16 : 8;

	tileXpos = 0;
	for (; currentTile < totTiles + remainingTiles; currentTile++) {
		uint32 tileOffset = READ_LE_UINT32(psxBackground + 4 * currentTile);

		if (isCompressed)
			decompressHIF(psxBackground + tileOffset - 4, decomp_tile); //Decompress the tile into decomp_tile
		else
			memcpy(decomp_tile, psxBackground + tileOffset - 4, 256);

		for (byte tileLine = 0; tileLine < tileHeight; tileLine++) { // Write the decoded tiles into last lines of background
			memcpy(fullres_buffer + tileXpos * 16 + (tileLine + (yresInTiles - 1) * 16) * bakXres * 2, decomp_tile + tileLine * 16, 16);
			memcpy(fullres_buffer + tileXpos * 16 + (tileLine + (yresInTiles - 1) * 16) * bakXres * 2 + bakXres, decomp_tile + tileLine * 16, 16);
		}
		tileXpos++;
	}

	free(decomp_tile);

	return fullres_buffer;
}

void Screen::fetchPsxParallaxSize(uint8 *psxParallax, uint16 *paraSizeX, uint16 *paraSizeY) {
	uint16 xresInTiles = READ_LE_UINT16(psxParallax + 10);
	uint16 yresInTiles = READ_LE_UINT16(psxParallax + 12);

	*paraSizeX = xresInTiles * 16;
	*paraSizeY = yresInTiles * 32; // Vertical resolution needs to be doubled
}

void Screen::drawPsxParallax(uint8 *psxParallax, uint16 paraScrlX, uint16 scrnScrlX, uint16 scrnWidth) {
	uint16 totTiles = READ_LE_UINT16(psxParallax + 14); // Total tiles

	uint16 skipRow = paraScrlX / 16; // Rows of tiles we have to skip
	uint8  leftPixelSkip = paraScrlX % 16; // Pixel columns we have to skip while drawing the first row

	uint8 *plxPos = psxParallax + 16; // Pointer to tile position header section
	uint8 *plxOff = psxParallax + 16 + totTiles * 2; // Pointer to tile relative offsets section
	uint8 *plxData = psxParallax + 16 + totTiles * 2 + totTiles * 4; //Pointer to beginning of tiles data section

	uint8 *tile_buffer = (uint8 *)malloc(16 * 16); // Buffer for 16x16 pix tile

	/* For parallax rendering we should check both horizontal and vertical scrolling,
	 * but in PSX edition of the game, the only vertical scrolling parallax is disabled.
	 * So, in this function i'll only check for horizontal scrolling.
	 */

	for (uint16 currentTile = 0; currentTile < totTiles - 1; currentTile++) {
		uint8 tileXpos = *(plxPos + 2 * currentTile); // Fetch tile X and Y position in the grid
		uint8 tileYpos = *(plxPos + 2 * currentTile + 1) * 2;
		int32 tileBegin = (tileXpos * 16) - paraScrlX;
		tileBegin = (tileBegin < 0) ? 0 : tileBegin;
		uint16 currentLine = (tileYpos * 16); //Current line of the image we are drawing upon, used to avoid going out of screen

		if (tileXpos >= skipRow) { // Tiles not needed in the screen buffer are not uncompressed
			uint32 tileOffset = READ_LE_UINT32(plxOff + 4 * currentTile);
			uint16 rightScreenLimit = _scrnSizeX - scrnScrlX; // Do not write over and beyond this limit, lest we get memory corruption
			uint8 *dest = _screenBuf + (tileYpos * 16 * _scrnSizeX) + tileBegin + scrnScrlX;
			uint8 *src = tile_buffer;

			decompressHIF(plxData + tileOffset, tile_buffer); // Decompress the tile

			if (tileXpos != skipRow) { // This tile will surely be drawn fully in the buffer
				for (byte tileLine = 0; (tileLine < 16) && (currentLine < SCREEN_DEPTH); tileLine++) { // Check that we are not going outside the bottom screen part
					for (byte tileColumn = 0; (tileColumn < 16) && (tileBegin + tileColumn) < rightScreenLimit; tileColumn++)
						if (*(src + tileColumn)) *(dest + tileColumn) = *(src + tileColumn);
					dest += _scrnSizeX;
					currentLine++;

					if (currentLine < SCREEN_DEPTH) {
						for (byte tileColumn = 0; (tileColumn < 16) && (tileBegin + tileColumn) < rightScreenLimit; tileColumn++)
							if (*(src + tileColumn)) *(dest + tileColumn) = *(src + tileColumn);
						dest += _scrnSizeX;
						currentLine++;
					}
					src += 16; // get to next line of decoded tile
				}
			} else { // This tile may be drawn only partially
				src += leftPixelSkip; //Skip hidden pixels
				for (byte tileLine = 0; (tileLine < 16) && (currentLine < SCREEN_DEPTH); tileLine++) {
					for (byte tileColumn = 0; tileColumn < (16 - leftPixelSkip); tileColumn++)
						if (*(src + tileColumn)) *(dest + tileColumn) = *(src + tileColumn);
					dest += _scrnSizeX;
					currentLine++;

					if (currentLine < SCREEN_DEPTH) {
						for (byte tileColumn = 0; tileColumn < (16 - leftPixelSkip); tileColumn++)
							if (*(src + tileColumn)) *(dest + tileColumn) = *(src + tileColumn);
						dest += _scrnSizeX;
						currentLine++;
					}
					src += 16;
				}
			}
		}
	}

	free(tile_buffer);
}

void Screen::decompressTony(uint8 *src, uint32 compSize, uint8 *dest) {
	uint8 *endOfData = src + compSize;
	while (src < endOfData) {
		uint8 numFlat = *src++;
		if (numFlat) {
			memset(dest, *src, numFlat);
			src++;
			dest += numFlat;
		}
		if (src < endOfData) {
			uint8 numNoFlat = *src++;
			memcpy(dest, src, numNoFlat);
			src += numNoFlat;
			dest += numNoFlat;
		}
	}
}

void Screen::decompressRLE7(uint8 *src, uint32 compSize, uint8 *dest) {
	uint8 *compBufEnd = src + compSize;
	while (src < compBufEnd) {
		uint8 code = *src++;
		if ((code > 127) || (code == 0))
			*dest++ = code;
		else {
			code++;
			memset(dest, *src++, code);
			dest += code;
		}
	}
}

void Screen::decompressRLE0(uint8 *src, uint32 compSize, uint8 *dest) {
	uint8 *srcBufEnd = src + compSize;
	while (src < srcBufEnd) {
		uint8 color = *src++;
		if (color) {
			*dest++ = color;
		} else {
			uint8 skip = *src++;
			memset(dest, 0, skip);
			dest += skip;
		}
	}
}

void Screen::decompressHIF(uint8 *src, uint8 *dest) {
	for (;;) { //Main loop
		byte control_byte = *src++;
		uint32 byte_count = 0;
		while (byte_count < 8) {
			if (control_byte & 0x80) {
				uint16 info_word = READ_BE_UINT16(src); //Read the info word
				src += 2;
				if (info_word == 0xFFFF) return; //Got 0xFFFF code, finished.

				int32 repeat_count = (info_word >> 12) + 2; //How many time data needs to be refetched
				while (repeat_count >= 0) {
					uint8 *old_data_src = dest - ((info_word & 0xFFF) + 1);
					*dest++ = *old_data_src;
					repeat_count--;
				}
			} else
				*dest++ = *src++;
			byte_count++;
			control_byte <<= 1; //Shifting left the control code one bit
		}
	}
}

void Screen::flushPsxCache() {
	if (_psxCache.decodedBackground) {
		free(_psxCache.decodedBackground);
		_psxCache.decodedBackground = NULL;
	}

	if (_psxCache.extPlxCache) {
		free(_psxCache.extPlxCache);
		_psxCache.extPlxCache = NULL;
	}
}

void Screen::fadePalette() {
	if (_fadingStep == 16)
		memcpy(_currentPalette, _targetPalette, 256 * 3);
	else if ((_fadingStep == 1) && (_fadingDirection == FADE_DOWN)) {
		memset(_currentPalette, 0, 3 * 256);
	} else
		for (uint16 cnt = 0; cnt < 256 * 3; cnt++)
			_currentPalette[cnt] = (_targetPalette[cnt] * _fadingStep) >> 4;

	_fadingStep += _fadingDirection;
	if (_fadingStep == 17) {
		_fadingStep = 0;
		_isBlack = false;
	} else if (_fadingStep == 0)
		_isBlack = true;
}

void Screen::fnSetParallax(uint32 screen, uint32 resId) {
	_roomDefTable[screen].parallax[0] = resId;
}

void Screen::spriteClipAndSet(uint16 *pSprX, uint16 *pSprY, uint16 *pSprWidth, uint16 *pSprHeight, uint16 *incr) {
	int16 sprX = *pSprX - SCREEN_LEFT_EDGE;
	int16 sprY = *pSprY - SCREEN_TOP_EDGE;
	int16 sprW = *pSprWidth;
	int16 sprH = *pSprHeight;

	if (sprY < 0) {
		*incr = (uint16)((-sprY) * sprW);
		sprH += sprY;
		sprY = 0;
	} else
		*incr = 0;
	if (sprX < 0) {
		*incr -= sprX;
		sprW += sprX;
		sprX = 0;
	}

	if (sprY + sprH > _scrnSizeY)
		sprH = _scrnSizeY - sprY;
	if (sprX + sprW > _scrnSizeX)
		sprW = _scrnSizeX - sprX;

	if (sprH < 0)
		*pSprHeight = 0;
	else
		*pSprHeight = (uint16)sprH;
	if (sprW < 0)
		*pSprWidth = 0;
	else
		*pSprWidth = (uint16)sprW;

	*pSprX = (uint16)sprX;
	*pSprY = (uint16)sprY;

	if (*pSprWidth && *pSprHeight) {
		// sprite will be drawn, so mark it in the grid buffer
		uint16 gridH = (*pSprHeight + (sprY & (SCRNGRID_Y - 1)) + (SCRNGRID_Y - 1)) / SCRNGRID_Y;
		uint16 gridW = (*pSprWidth + (sprX & (SCRNGRID_X - 1)) + (SCRNGRID_X - 1)) / SCRNGRID_X;

		if (SwordEngine::isPsx()) {
			gridH *= 2; // This will correct the PSX sprite being cut at half height
			gridW *= 2; // and masking problems when sprites are stretched in width

			uint16 bottomSprPos = (*pSprY + (*pSprHeight) * 2); //Position of bottom line of sprite
			if (bottomSprPos > _scrnSizeY) { //Check that resized psx sprite isn't drawn outside of screen boundaries
				uint16 outScreen = bottomSprPos - _scrnSizeY;
				*pSprHeight -= (outScreen % 2) ? (outScreen + 1) / 2 : outScreen / 2;
			}

		}

		uint16 gridX = sprX / SCRNGRID_X;
		uint16 gridY = sprY / SCRNGRID_Y;
		uint8 *gridBuf = _screenGrid + gridX + gridY * _gridSizeX;
		if (gridX + gridW > _gridSizeX)
			gridW = _gridSizeX - gridX;
		if (gridY + gridH > _gridSizeY)
			gridH = _gridSizeY - gridY;

		for (uint16 cnty = 0; cnty < gridH; cnty++) {
			for (uint16 cntx = 0; cntx < gridW; cntx++)
				gridBuf[cntx] = 2;
			gridBuf += _gridSizeX;
		}
	}
}

void Screen::fnFlash(uint8 color) {
	warning("stub: Screen::fnFlash(%d)", color);
}

// ------------------- Menu screen interface ---------------------------

void Screen::showFrame(uint16 x, uint16 y, uint32 resId, uint32 frameNo, const byte *fadeMask, int8 fadeStatus) {
	uint8 frame[40 * 40];
	int i, j;

	if (SwordEngine::isPsx())
		memset(frame, 0, sizeof(frame)); // PSX top menu is black
	else
		memset(frame, 199, sizeof(frame)); // Dark gray background

	if (resId != 0xffffffff) {
		FrameHeader *frameHead = _resMan->fetchFrame(_resMan->openFetchRes(resId), frameNo);
		uint8 *frameData = ((uint8 *)frameHead) + sizeof(FrameHeader);

		if (SwordEngine::isPsx()) { //We need to decompress PSX frames
			uint8 *frameBufferPSX = (uint8 *)malloc(_resMan->getUint16(frameHead->width) *  _resMan->getUint16(frameHead->height) / 2);
			decompressHIF(frameData, frameBufferPSX);

			for (i = 0; i < _resMan->getUint16(frameHead->height) / 2; i++) {
				for (j = 0; j < _resMan->getUint16(frameHead->width); j++) {
					uint8 data = frameBufferPSX[i * _resMan->getUint16(frameHead->width) + j];
					frame[(i * 2 + 4) * 40 + j + 2] = data;
					frame[(i * 2 + 1 + 4) * 40 + j + 2] = data; //Linedoubling the sprite
				}
			}

			free(frameBufferPSX);
		} else {
			for (i = 0; i < _resMan->getUint16(frameHead->height); i++)
				for (j = 0; j < _resMan->getUint16(frameHead->height); j++)
					frame[(i + 4) * 40 + j + 2] = frameData[i * _resMan->getUint16(frameHead->width) + j];
		}

		_resMan->resClose(resId);
	}

	if (fadeMask) {
		for (i = 0; i < 40; i++) {
			for (j = 0; j < 40; j++) {
				if (fadeMask[((i % 8) * 8) + (j % 8)] >= fadeStatus)
					frame[i * 40 + j] = 0;
			}
		}
	}

	_system->copyRectToScreen(frame, 40, x, y, 40, 40);
}

// ------------------- router debugging code --------------------------------

void Screen::vline(uint16 x, uint16 y1, uint16 y2) {
	for (uint16 cnty = y1; cnty <= y2; cnty++)
		_screenBuf[x + _scrnSizeX * cnty] = 0;
}

void Screen::hline(uint16 x1, uint16 x2, uint16 y) {
	for (uint16 cntx = x1; cntx <= x2; cntx++)
		_screenBuf[y * _scrnSizeX + cntx] = 0;
}

void Screen::bsubline_1(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int x, y, ddx, ddy, e;
	ddx = ABS(x2 - x1);
	ddy = ABS(y2 - y1) << 1;
	e = ddx - ddy;
	ddx <<= 1;

	if (x1 > x2) {
		uint16 tmp;
		tmp = x1; x1 = x2; x2 = tmp;
		tmp = y1; y1 = y2; y2 = tmp;
	}

	for (x = x1, y = y1; x <= x2; x++) {
		_screenBuf[y * _scrnSizeX + x] = 0;
		if (e < 0) {
			y++;
			e += ddx - ddy;
		} else {
			e -= ddy;
		}
	}
}

void Screen::bsubline_2(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int x, y, ddx, ddy, e;
	ddx = ABS(x2 - x1) << 1;
	ddy = ABS(y2 - y1);
	e = ddy - ddx;
	ddy <<= 1;

	if (y1 > y2) {
		uint16 tmp;
		tmp = x1; x1 = x2; x2 = tmp;
		tmp = y1; y1 = y2; y2 = tmp;
	}

	for (y = y1, x = x1; y <= y2; y++) {
		_screenBuf[y * _scrnSizeX + x] = 0;
		if (e < 0) {
			x++;
			e += ddy - ddx;
		} else {
			e -= ddx;
		}
	}
}

void Screen::bsubline_3(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int x, y, ddx, ddy, e;
	ddx = ABS(x1 - x2) << 1;
	ddy = ABS(y2 - y1);
	e = ddy - ddx;
	ddy <<= 1;

	if (y1 > y2) {
		uint16 tmp;
		tmp = x1; x1 = x2; x2 = tmp;
		tmp = y1; y1 = y2; y2 = tmp;
	}

	for (y = y1, x = x1; y <= y2; y++) {
		_screenBuf[y * _scrnSizeX + x] = 0;
		if (e < 0) {
			x--;
			e += ddy - ddx;
		} else {
			e -= ddx;
		}
	}
}

void Screen::bsubline_4(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int x, y, ddx, ddy, e;
	ddy = ABS(y2 - y1) << 1;
	ddx = ABS(x1 - x2);
	e = ddx - ddy;
	ddx <<= 1;

	if (x1 > x2) {
		uint16 tmp;
		tmp = x1; x1 = x2; x2 = tmp;
		tmp = y1; y1 = y2; y2 = tmp;
	}

	for (x = x1, y = y1; x <= x2; x++) {
		_screenBuf[y * _scrnSizeX + x] = 0;
		if (e < 0) {
			y--;
			e += ddx - ddy;
		} else {
			e -= ddy;
		}
	}
}

void Screen::drawLine(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	if ((x1 == x2) && (y1 == y2)) {
		_screenBuf[x1 + y1 * _scrnSizeX] = 0;
	}
	if (x1 == x2) {
		vline(x1, MIN(y1, y2), MAX(y1, y2));
		return;
	}

	if (y1 == y2) {
		hline(MIN(x1, x2), MAX(x1, x2), y1);
		return;
	}

	float k = float(y2 - y1) / float(x2 - x1);

	if ((k >= 0) && (k <= 1)) {
		bsubline_1(x1, y1, x2, y2);
	} else if (k > 1) {
		bsubline_2(x1, y1, x2, y2);
	} else if ((k < 0) && (k >= -1)) {
		bsubline_4(x1, y1, x2, y2);
	} else {
		bsubline_3(x1, y1, x2, y2);
	}
}

} // End of namespace Sword1
