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
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/palette.h"

#include "sky/disk.h"
#include "sky/logic.h"
#include "sky/screen.h"
#include "sky/compact.h"
#include "sky/sky.h"
#include "sky/skydefs.h"
#include "sky/struc.h"

namespace Sky {

uint8 Screen::_top16Colors[16*3] = {
	0, 0, 0,
	38, 38, 38,
	63, 63, 63,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	0, 0, 0,
	54, 54, 54,
	45, 47, 49,
	32, 31, 41,
	29, 23, 37,
	23, 18, 30,
	49, 11, 11,
	39, 5, 5,
	29, 1, 1,
	63, 63, 63
};

Screen::Screen(OSystem *pSystem, Disk *pDisk, SkyCompact *skyCompact) {
	_system = pSystem;
	_skyDisk = pDisk;
	_skyCompact = skyCompact;

	int i;
	uint8 tmpPal[VGA_COLORS * 3];

	_gameGrid = (uint8 *)malloc(GRID_X * GRID_Y * 2);
	forceRefresh();

	_currentScreen = NULL;
	_scrollScreen = NULL;

	//blank the first 240 colors of the palette
	memset(tmpPal, 0, GAME_COLORS * 3);

	//set the remaining colors
	for (i = 0; i < (VGA_COLORS-GAME_COLORS); i++) {
		tmpPal[3 * GAME_COLORS + i * 3 + 0] = (_top16Colors[i * 3 + 0] << 2) + (_top16Colors[i * 3 + 0] >> 4);
		tmpPal[3 * GAME_COLORS + i * 3 + 1] = (_top16Colors[i * 3 + 1] << 2) + (_top16Colors[i * 3 + 1] >> 4);
		tmpPal[3 * GAME_COLORS + i * 3 + 2] = (_top16Colors[i * 3 + 2] << 2) + (_top16Colors[i * 3 + 2] >> 4);
	}

	//set the palette
	_system->getPaletteManager()->setPalette(tmpPal, 0, VGA_COLORS);
	_currentPalette = 0;

	_seqInfo.nextFrame = _seqInfo.framesLeft = 0;
	_seqInfo.seqData = _seqInfo.seqDataPos = NULL;
	_seqInfo.running = false;
}

Screen::~Screen() {
	free(_gameGrid);
	free(_currentScreen);
	free(_scrollScreen);
}

void Screen::clearScreen() {
	memset(_currentScreen, 0, FULL_SCREEN_WIDTH * FULL_SCREEN_HEIGHT);
	_system->copyRectToScreen(_currentScreen, GAME_SCREEN_WIDTH, 0, 0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
	_system->updateScreen();
}

void Screen::setFocusRectangle(const Common::Rect& rect) {
	_system->setFocusRectangle(rect);
}

//set a new palette, pal is a pointer to dos vga rgb components 0..63
void Screen::setPalette(uint8 *pal) {
	convertPalette(pal, _palette);
	_system->getPaletteManager()->setPalette(_palette, 0, GAME_COLORS);
	_system->updateScreen();
}

void Screen::setPaletteEndian(uint8 *pal) {
#ifdef SCUMM_BIG_ENDIAN
	uint8 endPalette[VGA_COLORS * 3];
	for (uint16 cnt = 0; cnt < VGA_COLORS * 3; cnt++)
		endPalette[cnt] = pal[cnt ^ 1];
	convertPalette(endPalette, _palette);
#else
	convertPalette(pal, _palette);
#endif
	_system->getPaletteManager()->setPalette(_palette, 0, GAME_COLORS);
	_system->updateScreen();
}

void Screen::halvePalette() {
	uint8 halfPalette[VGA_COLORS * 3];

	for (uint8 cnt = 0; cnt < GAME_COLORS; cnt++) {
		halfPalette[cnt * 3 + 0] = _palette[cnt * 3 + 0] >> 1;
		halfPalette[cnt * 3 + 1] = _palette[cnt * 3 + 1] >> 1;
		halfPalette[cnt * 3 + 2] = _palette[cnt * 3 + 2] >> 1;
	}
	_system->getPaletteManager()->setPalette(halfPalette, 0, GAME_COLORS);
}

void Screen::setPalette(uint16 fileNum) {
	uint8 *tmpPal = _skyDisk->loadFile(fileNum);
	if (tmpPal) {
		setPalette(tmpPal);
		free(tmpPal);
	} else
		warning("Screen::setPalette: can't load file nr. %d",fileNum);
}

void Screen::showScreen(uint16 fileNum) {
	// This is only used for static images in the floppy and cd intro
	free(_currentScreen);
	_currentScreen = _skyDisk->loadFile(fileNum);
	// make sure the last 8 lines are forced to black.
	memset(_currentScreen + GAME_SCREEN_HEIGHT * GAME_SCREEN_WIDTH, 0, (FULL_SCREEN_HEIGHT - GAME_SCREEN_HEIGHT) * GAME_SCREEN_WIDTH);

	if (_currentScreen)
		showScreen(_currentScreen);
	else
		warning("Screen::showScreen: can't load file nr. %d",fileNum);
}

void Screen::showScreen(uint8 *pScreen) {
	_system->copyRectToScreen(pScreen, 320, 0, 0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
	_system->updateScreen();
}

//convert 3 byte 0..63 rgb to 3 byte 0..255 rgb
void Screen::convertPalette(uint8 *inPal, uint8* outPal) {
	int i;

	for (i = 0; i < VGA_COLORS; i++) {
		outPal[3 * i + 0] = (inPal[3 * i + 0] << 2) + (inPal[3 * i + 0] >> 4);
		outPal[3 * i + 1] = (inPal[3 * i + 1] << 2) + (inPal[3 * i + 1] >> 4);
		outPal[3 * i + 2] = (inPal[3 * i + 2] << 2) + (inPal[3 * i + 2] >> 4);
	}
}

void Screen::recreate() {
	// check the game grid for changed blocks
	if (!Logic::_scriptVariables[LAYER_0_ID])
		return;
	uint8 *gridPos = _gameGrid;
	uint8 *screenData = (uint8 *)SkyEngine::fetchItem(Logic::_scriptVariables[LAYER_0_ID]);
	if (!screenData) {
		error("Screen::recreate():\nSkyEngine::fetchItem(Logic::_scriptVariables[LAYER_0_ID](%X)) returned NULL", Logic::_scriptVariables[LAYER_0_ID]);
	}
	uint8 *screenPos = _currentScreen;

	for (uint8 cnty = 0; cnty < GRID_Y; cnty++) {
		for (uint8 cntx = 0; cntx < GRID_X; cntx++) {
			if (gridPos[0] & 0x80) {
				gridPos[0] &= 0x7F; // reset recreate flag
				gridPos[0] |= 1;    // set bit for flip routine
				uint8 *savedScreenY = screenPos;
				for (uint8 gridCntY = 0; gridCntY < GRID_H; gridCntY++) {
					memcpy(screenPos, screenData, GRID_W);
					screenPos += GAME_SCREEN_WIDTH;
					screenData += GRID_W;
				}
				screenPos = savedScreenY + GRID_W;
			} else {
				screenPos += GRID_W;
				screenData += GRID_W * GRID_H;
			}
			gridPos++;
		}
		screenPos += (GRID_H - 1) * GAME_SCREEN_WIDTH;
	}
}

void Screen::flip(bool doUpdate) {
	uint32 copyX, copyWidth;
	copyX = copyWidth = 0;
	for (uint8 cnty = 0; cnty < GRID_Y; cnty++) {
		for (uint8 cntx = 0; cntx < GRID_X; cntx++) {
			if (_gameGrid[cnty * GRID_X + cntx] & 1) {
				_gameGrid[cnty * GRID_X + cntx] &= ~1;
				if (!copyWidth)
					copyX = cntx * GRID_W;
				copyWidth += GRID_W;
			} else if (copyWidth) {
				_system->copyRectToScreen(_currentScreen + cnty * GRID_H * GAME_SCREEN_WIDTH + copyX, GAME_SCREEN_WIDTH, copyX, cnty * GRID_H, copyWidth, GRID_H);
				copyWidth = 0;
			}
		}
		if (copyWidth) {
			_system->copyRectToScreen(_currentScreen + cnty * GRID_H * GAME_SCREEN_WIDTH + copyX, GAME_SCREEN_WIDTH, copyX, cnty * GRID_H, copyWidth, GRID_H);
			copyWidth = 0;
		}
	}
	if (doUpdate)
		_system->updateScreen();
}

void Screen::fnDrawScreen(uint32 palette, uint32 scroll) {
	// set up the new screen
	fnFadeDown(scroll);
	forceRefresh();
	recreate();
	spriteEngine();
	flip(false);
	fnFadeUp(palette, scroll);
}

void Screen::fnFadeDown(uint32 scroll) {
	if (((scroll != 123) && (scroll != 321)) || (SkyEngine::_systemVars.systemFlags & SF_NO_SCROLL)) {
		uint32 delayTime = _system->getMillis();
		for (uint8 cnt = 0; cnt < 32; cnt++) {
			delayTime += 20;
			palette_fadedown_helper(_palette, GAME_COLORS);
			_system->getPaletteManager()->setPalette(_palette, 0, GAME_COLORS);
			_system->updateScreen();
			int32 waitTime = (int32)delayTime - _system->getMillis();
			if (waitTime < 0)
				waitTime = 0;
			_system->delayMillis((uint)waitTime);
		}
	} else {
		// scrolling is performed by fnFadeUp. It's just prepared here
		_scrollScreen = _currentScreen;
		_currentScreen = (uint8 *)malloc(FULL_SCREEN_WIDTH * FULL_SCREEN_HEIGHT);
		// the game will draw the new room into _currentScreen which
		// will be scrolled into the visible screen by fnFadeUp
		// fnFadeUp also frees the _scrollScreen
	}
}

void Screen::palette_fadedown_helper(uint8 *pal, uint num) {
	do {
		if (pal[0] >= 8)
			pal[0] -= 8;
		else
			pal[0] = 0;

		if (pal[1] >= 8)
			pal[1] -= 8;
		else
			pal[1] = 0;

		if (pal[2] >= 8)
			pal[2] -= 8;
		else
			pal[2] = 0;

		pal += 3;
	} while (--num);
}

void Screen::paletteFadeUp(uint16 fileNr) {
	uint8 *pal = _skyDisk->loadFile(fileNr);
	if (pal) {
		paletteFadeUp(pal);
		free(pal);
	} else
		warning("Screen::paletteFadeUp: Can't load palette #%d",fileNr);
}

void Screen::paletteFadeUp(uint8 *pal) {
	byte tmpPal[VGA_COLORS * 3];

	convertPalette(pal, tmpPal);

	uint32 delayTime = _system->getMillis();
	for (uint8 cnt = 1; cnt <= 32; cnt++) {
		delayTime += 20;

		for (uint8 colCnt = 0; colCnt < GAME_COLORS; colCnt++) {
			_palette[colCnt * 3 + 0] = (tmpPal[colCnt * 3 + 0] * cnt) >> 5;
			_palette[colCnt * 3 + 1] = (tmpPal[colCnt * 3 + 1] * cnt) >> 5;
			_palette[colCnt * 3 + 2] = (tmpPal[colCnt * 3 + 2] * cnt) >> 5;
		}

		_system->getPaletteManager()->setPalette(_palette, 0, GAME_COLORS);
		_system->updateScreen();

		int32 waitTime = (int32)delayTime - _system->getMillis();
		if (waitTime < 0)
			waitTime = 0;
		_system->delayMillis((uint)waitTime);
	}
}

void Screen::fnFadeUp(uint32 palNum, uint32 scroll) {
	//_currentScreen points to new screen,
	//_scrollScreen points to graphic showing old room
	if ((scroll != 123) && (scroll != 321))
		scroll = 0;

	if ((scroll == 0) || (SkyEngine::_systemVars.systemFlags & SF_NO_SCROLL)) {
		uint8 *palette = (uint8 *)_skyCompact->fetchCpt(palNum);
		if (palette == NULL)
			error("Screen::fnFadeUp: can't fetch compact %X", palNum);
#ifdef SCUMM_BIG_ENDIAN
		byte tmpPal[VGA_COLORS * 3];
		for (uint16 cnt = 0; cnt < VGA_COLORS * 3; cnt++)
			tmpPal[cnt] = palette[cnt ^ 1];
		paletteFadeUp(tmpPal);
#else
		paletteFadeUp(palette);
#endif
	} else if (scroll == 123) {	// scroll left (going right)
		assert(_currentScreen && _scrollScreen);
		uint8 *scrNewPtr, *scrOldPtr;
		for (uint8 scrollCnt = 0; scrollCnt < (GAME_SCREEN_WIDTH / SCROLL_JUMP) - 1; scrollCnt++) {
			scrNewPtr = _currentScreen + scrollCnt * SCROLL_JUMP;
			scrOldPtr = _scrollScreen;
			for (uint8 lineCnt = 0; lineCnt < GAME_SCREEN_HEIGHT; lineCnt++) {
				memmove(scrOldPtr, scrOldPtr + SCROLL_JUMP, GAME_SCREEN_WIDTH - SCROLL_JUMP);
				memcpy(scrOldPtr + GAME_SCREEN_WIDTH - SCROLL_JUMP, scrNewPtr, SCROLL_JUMP);
				scrNewPtr += GAME_SCREEN_WIDTH;
				scrOldPtr += GAME_SCREEN_WIDTH;
			}
			showScreen(_scrollScreen);
			waitForTick();
		}
		showScreen(_currentScreen);
	} else if (scroll == 321) {	// scroll right (going left)
		assert(_currentScreen && _scrollScreen);
		uint8 *scrNewPtr, *scrOldPtr;
		for (uint8 scrollCnt = 0; scrollCnt < (GAME_SCREEN_WIDTH / SCROLL_JUMP) - 1; scrollCnt++) {
			scrNewPtr = _currentScreen + GAME_SCREEN_WIDTH - (scrollCnt + 1) * SCROLL_JUMP;
			scrOldPtr = _scrollScreen;
			for (uint8 lineCnt = 0; lineCnt < GAME_SCREEN_HEIGHT; lineCnt++) {
				memmove(scrOldPtr + SCROLL_JUMP, scrOldPtr, GAME_SCREEN_WIDTH - SCROLL_JUMP);
				memcpy(scrOldPtr, scrNewPtr, SCROLL_JUMP);
				scrNewPtr += GAME_SCREEN_WIDTH;
				scrOldPtr += GAME_SCREEN_WIDTH;
			}
			showScreen(_scrollScreen);
			waitForTick();
		}
		showScreen(_currentScreen);
	}
	if (_scrollScreen) {
		free(_scrollScreen);
		_scrollScreen = NULL;
	}
}

void Screen::waitForTick() {
	uint32 start = _system->getMillis();
	uint32 end = start + 20 - (start % 20);
	uint32 remain;

	Common::EventManager *eventMan = _system->getEventManager();
	Common::Event event;

	while (true) {
		while (eventMan->pollEvent(event))
			;

		start = _system->getMillis();

		if (start >= end)
			return;

		remain = end - start;
		if (remain < 10) {
			_system->delayMillis(remain);
			return;
		}

		_system->delayMillis(10);
	}
}

void Screen::waitForSequence() {
	Common::EventManager *eventMan = _system->getEventManager();
	Common::Event event;

	while (_seqInfo.running) {
		processSequence();

		_system->delayMillis(20);
		while (eventMan->pollEvent(event))
			;
	}
}

void Screen::startSequence(uint16 fileNum) {
	_seqInfo.seqData = _skyDisk->loadFile(fileNum);
	_seqInfo.nextFrame = _system->getMillis() + 60;
	_seqInfo.framesLeft = _seqInfo.seqData[0];
	_seqInfo.seqDataPos = _seqInfo.seqData + 1;
	_seqInfo.running = true;
	_seqInfo.runningItem = false;
}

void Screen::startSequenceItem(uint16 itemNum) {
	_seqInfo.seqData = (uint8 *)SkyEngine::fetchItem(itemNum);
	_seqInfo.nextFrame = _system->getMillis() + 60;
	_seqInfo.framesLeft = _seqInfo.seqData[0] - 1;
	_seqInfo.seqDataPos = _seqInfo.seqData + 1;
	_seqInfo.running = true;
	_seqInfo.runningItem = true;
}

void Screen::stopSequence() {
	_seqInfo.running = false;
	waitForTick();
	waitForTick();
	_seqInfo.nextFrame = _seqInfo.framesLeft = 0;
	free(_seqInfo.seqData);
	_seqInfo.seqData = _seqInfo.seqDataPos = NULL;
}

void Screen::processSequence() {
	if (!_seqInfo.running)
		return;

	if (_system->getMillis() < _seqInfo.nextFrame)
		return;

	_seqInfo.nextFrame += 60;

	memset(_seqGrid, 0, 12 * 20);

	uint32 screenPos = 0;

	uint8 nrToSkip, nrToDo, cnt;
	do {
		do {
			nrToSkip = _seqInfo.seqDataPos[0];
			_seqInfo.seqDataPos++;
			screenPos += nrToSkip;
		} while (nrToSkip == 0xFF);

		do {
			nrToDo = _seqInfo.seqDataPos[0];
			_seqInfo.seqDataPos++;

			uint8 gridSta = (uint8)((screenPos / (GAME_SCREEN_WIDTH * 16))*20 + ((screenPos % GAME_SCREEN_WIDTH) >> 4));
			uint8 gridEnd = (uint8)(((screenPos+nrToDo) / (GAME_SCREEN_WIDTH * 16))*20 + (((screenPos+nrToDo) % GAME_SCREEN_WIDTH) >> 4));
			gridSta = MIN(gridSta, (uint8)(12 * 20 - 1));
			gridEnd = MIN(gridEnd, (uint8)(12 * 20 - 1));
			if (gridEnd >= gridSta)
				for (cnt = gridSta; cnt <= gridEnd; cnt++)
					_seqGrid[cnt] = 1;
			else {
				for (cnt = gridSta; cnt < (gridSta / 20 + 1) * 20; cnt++)
					_seqGrid[cnt] = 1;
				for (cnt = (gridEnd / 20) * 20; cnt <= gridEnd; cnt++)
					_seqGrid[cnt] = 1;
			}

			for (cnt = 0; cnt < nrToDo; cnt++) {
				_currentScreen[screenPos] = _seqInfo.seqDataPos[0];
				_seqInfo.seqDataPos++;
				screenPos++;
			}
		} while (nrToDo == 0xFF);
	} while (screenPos < (GAME_SCREEN_WIDTH * GAME_SCREEN_HEIGHT));
	uint8 *gridPtr = _seqGrid; uint8 *scrPtr = _currentScreen; uint8 *rectPtr = NULL;
	uint8 rectWid = 0, rectX = 0, rectY = 0;
	for (uint8 cnty = 0; cnty < 12; cnty++) {
		for (uint8 cntx = 0; cntx < 20; cntx++) {
			if (*gridPtr) {
				if (!rectWid) {
					rectX = cntx;
					rectY = cnty;
					rectPtr = scrPtr;
				}
				rectWid++;
			} else if (rectWid) {
				_system->copyRectToScreen(rectPtr, GAME_SCREEN_WIDTH, rectX << 4, rectY << 4, rectWid << 4, 16);
				rectWid = 0;
			}
			scrPtr += 16;
			gridPtr++;
		}
		if (rectWid) {
			_system->copyRectToScreen(rectPtr, GAME_SCREEN_WIDTH, rectX << 4, rectY << 4, rectWid << 4, 16);
			rectWid = 0;
		}
		scrPtr += 15 * GAME_SCREEN_WIDTH;
	}
	_system->updateScreen();
	_seqInfo.framesLeft--;

	if (_seqInfo.framesLeft == 0) {
		_seqInfo.running = false;
		if (!_seqInfo.runningItem)
			free(_seqInfo.seqData);
		_seqInfo.seqData = _seqInfo.seqDataPos = NULL;
	}
}

//- sprites.asm routines

void Screen::spriteEngine() {
	doSprites(BACK);
	sortSprites();
	doSprites(FORE);
}

void Screen::sortSprites() {
	StSortList sortList[30];
	uint32 currDrawList = DRAW_LIST_NO;
	uint32 loadDrawList;

	bool nextDrawList = false;
	while (Logic::_scriptVariables[currDrawList]) {
		// big_sort_loop
		uint32 spriteCnt = 0;
		loadDrawList = Logic::_scriptVariables[currDrawList];
		currDrawList++;

		do { // a_new_draw_list:
			uint16 *drawListData = (uint16 *)_skyCompact->fetchCpt(loadDrawList);
			nextDrawList = false;
			while ((!nextDrawList) && (drawListData[0])) {
				if (drawListData[0] == 0xFFFF) {
					loadDrawList = drawListData[1];
					nextDrawList = true;
				} else {
					// process_this_id:
					Compact *spriteComp = _skyCompact->fetchCpt(drawListData[0]);
					if ((spriteComp->status & 4) && // is it sortable playfield?(!?!)
						(spriteComp->screen == Logic::_scriptVariables[SCREEN])) { // on current screen
							DataFileHeader *spriteData =
								(DataFileHeader *)SkyEngine::fetchItem(spriteComp->frame >> 6);
							if (!spriteData) {
								debug(9,"Missing file %d", spriteComp->frame >> 6);
								spriteComp->status = 0;
							} else {
								sortList[spriteCnt].yCood = spriteComp->ycood + spriteData->s_offset_y + spriteData->s_height;
								sortList[spriteCnt].compact = spriteComp;
								sortList[spriteCnt].sprite = spriteData;
								spriteCnt++;
							}
					}
					drawListData++;
				}
			}
		} while (nextDrawList);
		// made_list:
		if (spriteCnt > 1) { // bubble sort
			for (uint32 cnt1 = 0; cnt1 < spriteCnt - 1; cnt1++)
				for (uint32 cnt2 = cnt1 + 1; cnt2 < spriteCnt; cnt2++)
					if (sortList[cnt1].yCood > sortList[cnt2].yCood) {
						StSortList tmp;
						tmp.yCood = sortList[cnt1].yCood;
						tmp.sprite = sortList[cnt1].sprite;
						tmp.compact = sortList[cnt1].compact;
						sortList[cnt1].yCood = sortList[cnt2].yCood;
						sortList[cnt1].sprite = sortList[cnt2].sprite;
						sortList[cnt1].compact = sortList[cnt2].compact;
						sortList[cnt2].yCood = tmp.yCood;
						sortList[cnt2].sprite = tmp.sprite;
						sortList[cnt2].compact = tmp.compact;
					}
		}
		for (uint32 cnt = 0; cnt < spriteCnt; cnt++) {
			drawSprite((uint8 *)sortList[cnt].sprite, sortList[cnt].compact);
			if (sortList[cnt].compact->status & 8)
				vectorToGame(0x81);
			else
				vectorToGame(1);
			if (!(sortList[cnt].compact->status & 0x200))
				verticalMask();
		}
	}
}

void Screen::doSprites(uint8 layer) {
	uint16 drawListNum = DRAW_LIST_NO;
	uint32 idNum;
	uint16* drawList;
	while (Logic::_scriptVariables[drawListNum]) { // std sp loop
		idNum = Logic::_scriptVariables[drawListNum];
		drawListNum++;

		drawList = (uint16 *)_skyCompact->fetchCpt(idNum);
		while (drawList[0]) {
			// new_draw_list:
			while ((drawList[0] != 0) && (drawList[0] != 0xFFFF)) {
				// back_loop:
				// not_new_list
				Compact *spriteData = _skyCompact->fetchCpt(drawList[0]);
				drawList++;
				if ((spriteData->status & (1 << layer)) &&
						(spriteData->screen == Logic::_scriptVariables[SCREEN])) {
					uint8 *toBeDrawn = (uint8 *)SkyEngine::fetchItem(spriteData->frame >> 6);
					if (!toBeDrawn) {
						debug(9, "Spritedata %d not loaded", spriteData->frame >> 6);
						spriteData->status = 0;
					} else {
						drawSprite(toBeDrawn, spriteData);
						if (layer == BACK)
							verticalMask();
						if (spriteData->status & 8)
							vectorToGame(0x81);
						else
							vectorToGame(1);
					}
				}
			}
			while (drawList[0] == 0xFFFF)
				drawList = (uint16 *)_skyCompact->fetchCpt(drawList[1]);
		}
	}
}

void Screen::drawSprite(uint8 *spriteInfo, Compact *sprCompact) {
	if (spriteInfo == NULL) {
		warning("Screen::drawSprite Can't draw sprite. Data %d was not loaded", sprCompact->frame >> 6);
		sprCompact->status = 0;
		return;
	}
	DataFileHeader *sprDataFile = (DataFileHeader *)spriteInfo;
	_sprWidth = sprDataFile->s_width;
	_sprHeight = sprDataFile->s_height;
	_maskX1 = _maskX2 = 0;
	uint8 *spriteData = spriteInfo + (sprCompact->frame & 0x3F) * sprDataFile->s_sp_size;
	spriteData += sizeof(DataFileHeader);
	int32 spriteY = sprCompact->ycood + sprDataFile->s_offset_y - TOP_LEFT_Y;
	if (spriteY < 0) {
		spriteY = -spriteY;
		if (_sprHeight <= (uint32)spriteY) {
			_sprWidth = 0;
			return;
		}
		_sprHeight -= spriteY;
		spriteData += sprDataFile->s_width * spriteY;
		spriteY = 0;
	} else {
		int32 botClip = GAME_SCREEN_HEIGHT - sprDataFile->s_height - spriteY;
		if (botClip < 0) {
			botClip = -botClip;
			if (_sprHeight <= (uint32)botClip) {
				_sprWidth = 0;
				return;
			}
			_sprHeight -= botClip;
		}
	}
	_sprY = (uint32)spriteY;
	int32 spriteX = sprCompact->xcood + sprDataFile->s_offset_x - TOP_LEFT_X;
	if (spriteX < 0) {
		spriteX = -spriteX;
		if (_sprWidth <= (uint32)spriteX) {
			_sprWidth = 0;
			return;
		}
		_sprWidth -= spriteX;
		_maskX1 = spriteX;
		spriteX = 0;
	} else {
		int32 rightClip = GAME_SCREEN_WIDTH - (sprDataFile->s_width + spriteX);
		if (rightClip < 0) {
			rightClip = (-rightClip) + 1;
			if (_sprWidth <= (uint32)rightClip) {
				_sprWidth = 0;
				return;
			}
			_sprWidth -= rightClip;
			_maskX2 = rightClip;
		}
	}
	_sprX = (uint32)spriteX;
	uint8 *screenPtr = _currentScreen + _sprY * GAME_SCREEN_WIDTH + _sprX;
	if ((_sprHeight > 192) || (_sprY > 192)) {
		_sprWidth = 0;
		return;
	}
	if ((_sprX + _sprWidth > 320) || (_sprY + _sprHeight > 192)) {
		warning("Screen::drawSprite fatal error: got x = %d, y = %d, w = %d, h = %d",_sprX, _sprY, _sprWidth, _sprHeight);
		_sprWidth = 0;
		return;
	}

	for (uint16 cnty = 0; cnty < _sprHeight; cnty++) {
		for (uint16 cntx = 0; cntx < _sprWidth; cntx++)
			if (spriteData[cntx + _maskX1])
				screenPtr[cntx] = spriteData[cntx + _maskX1];
		spriteData += _sprWidth + _maskX2 + _maskX1;
		screenPtr += GAME_SCREEN_WIDTH;
	}
	// Convert the sprite coordinate/size values to blocks for vertical mask and/or vector to game
	_sprWidth += _sprX + GRID_W-1;
	_sprHeight += _sprY + GRID_H-1;

	_sprX >>= GRID_W_SHIFT;
	_sprWidth >>= GRID_W_SHIFT;
	_sprY >>= GRID_H_SHIFT;
	_sprHeight >>= GRID_H_SHIFT;

	_sprWidth -= _sprX;
	_sprHeight -= _sprY;
}

void Screen::vectorToGame(uint8 gridVal) {
	if (_sprWidth == 0)
		return;
	uint8 *trgGrid = _gameGrid + _sprY * GRID_X +_sprX;
	for (uint32 cnty = 0; cnty < _sprHeight; cnty++) {
		for (uint32 cntx = 0; cntx < _sprWidth; cntx++)
			trgGrid[cntx] |= gridVal;
		trgGrid += GRID_X;
	}
}

void Screen::vertMaskSub(uint16 *grid, uint32 gridOfs, uint8 *screenPtr, uint32 layerId) {
	for (uint32 cntx = 0; cntx < _sprHeight; cntx++) { // start_x | block_loop
		if (grid[gridOfs]) {
			if (!(FROM_LE_16(grid[gridOfs]) & 0x8000)) {
				uint32 gridVal = FROM_LE_16(grid[gridOfs]) - 1;
				gridVal *= GRID_W * GRID_H;
				uint8 *dataSrc = (uint8 *)SkyEngine::fetchItem(Logic::_scriptVariables[layerId]) + gridVal;
				uint8 *dataTrg = screenPtr;
				for (uint32 grdCntY = 0; grdCntY < GRID_H; grdCntY++) {
					for (uint32 grdCntX = 0; grdCntX < GRID_W; grdCntX++)
						if (dataSrc[grdCntX])
							dataTrg[grdCntX] = dataSrc[grdCntX];
					dataSrc += GRID_W;
					dataTrg += GAME_SCREEN_WIDTH;
				}
			} // dummy_end:
			screenPtr -= GRID_H * GAME_SCREEN_WIDTH;
			gridOfs -= GRID_X;
		} else
			return;
	} // next_x
}

void Screen::verticalMask() {
	if (_sprWidth == 0)
		return;
	uint32 startGridOfs = (_sprY + _sprHeight - 1) * GRID_X + _sprX;
	uint8 *startScreenPtr = (_sprY + _sprHeight - 1) * GRID_H * GAME_SCREEN_WIDTH + _sprX * GRID_W + _currentScreen;

	for (uint32 layerCnt = LAYER_1_ID; layerCnt <= LAYER_3_ID; layerCnt++) {
		uint32 gridOfs = startGridOfs;
		uint8 *screenPtr = startScreenPtr;
		for (uint32 widCnt = 0; widCnt < _sprWidth; widCnt++) { // x_loop
			uint32 nLayerCnt = layerCnt;
			while (Logic::_scriptVariables[nLayerCnt + 3]) {
				uint16 *scrGrid;
				scrGrid = (uint16 *)SkyEngine::fetchItem(Logic::_scriptVariables[layerCnt + 3]);
				if (scrGrid[gridOfs]) {
					vertMaskSub(scrGrid, gridOfs, screenPtr, layerCnt);
					break;
				} else
					nLayerCnt++;
			}
			// next_x:
			screenPtr += GRID_W;
			gridOfs++;
		}
	}
}

void Screen::paintBox(uint16 x, uint16 y) {
	uint8 *screenPos = _currentScreen + y * GAME_SCREEN_WIDTH + x;
	memset(screenPos, 255, 8);
	for (uint8 cnt = 1; cnt < 8; cnt++) {
		*(screenPos + cnt * GAME_SCREEN_WIDTH) = 255;
		*(screenPos + cnt * GAME_SCREEN_WIDTH + 7) = 255;
	}
	memset(screenPos + 7 * GAME_SCREEN_WIDTH, 255, 7);
}

void Screen::showGrid(uint8 *gridBuf) {
	uint32 gridData = 0;
	uint8 bitsLeft = 0;
	for (uint16 cnty = 0; cnty < GAME_SCREEN_HEIGHT >> 3; cnty++) {
		for (uint16 cntx = 0; cntx < GAME_SCREEN_WIDTH >> 3; cntx++) {
			if (!bitsLeft) {
				bitsLeft = 32;
				gridData = *(uint32 *)gridBuf;
				gridBuf += 4;
			}
			if (gridData & 0x80000000)
				paintBox(cntx << 3, cnty << 3);
			bitsLeft--;
			gridData <<= 1;
		}
	}
	_system->copyRectToScreen(_currentScreen, GAME_SCREEN_WIDTH, 0, 0, GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);

}

} // End of namespace Sky
