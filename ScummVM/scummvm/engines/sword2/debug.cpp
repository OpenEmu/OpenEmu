/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/rect.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/console.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/memory.h"
#include "sword2/mouse.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/screen.h"

namespace Sword2 {

void Debugger::clearDebugTextBlocks() {
	uint8 blockNo = 0;

	while (blockNo < MAX_DEBUG_TEXTS && _debugTextBlocks[blockNo] > 0) {
		// kill the system text block
		_vm->_fontRenderer->killTextBloc(_debugTextBlocks[blockNo]);

		// clear this element of our array of block numbers
		_debugTextBlocks[blockNo] = 0;

		blockNo++;
	}
}

void Debugger::makeDebugTextBlock(char *text, int16 x, int16 y) {
	uint8 blockNo = 0;

	while (blockNo < MAX_DEBUG_TEXTS && _debugTextBlocks[blockNo] > 0)
		blockNo++;

	assert(blockNo < MAX_DEBUG_TEXTS);

	_debugTextBlocks[blockNo] = _vm->_fontRenderer->buildNewBloc((byte *)text, x, y, 640 - x, 0, RDSPR_DISPLAYALIGN, CONSOLE_FONT_ID, NO_JUSTIFICATION);
}

void Debugger::buildDebugText() {
	char buf[128];

	int32 showVarNo;		// for variable watching
	int32 showVarPos;
	int32 varNo;

	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	// clear the array of text block numbers for the debug text
	clearDebugTextBlocks();

	// mouse coords
	// print mouse coords beside mouse-marker, if it's being displayed
	if (_displayMouseMarker) {
		int mouseX, mouseY;

		_vm->_mouse->getPos(mouseX, mouseY);

		sprintf(buf, "%d,%d", mouseX + screenInfo->scroll_offset_x, mouseY + screenInfo->scroll_offset_y);
		if (mouseX > 560)
			makeDebugTextBlock(buf, mouseX - 50, mouseY - 15);
		else
			makeDebugTextBlock(buf, mouseX + 5, mouseY - 15);
	}

	// mouse area coords

	// defining a mouse area the easy way, by creating a box on-screen
	if (_draggingRectangle || _vm->_logic->readVar(SYSTEM_TESTING_ANIMS)) {
		// so we can see what's behind the lines
		_rectFlicker = !_rectFlicker;

		sprintf(buf, "x1=%d", _rectX1);
		makeDebugTextBlock(buf, 0, 120);

		sprintf(buf, "y1=%d", _rectY1);
		makeDebugTextBlock(buf, 0, 135);

		sprintf(buf, "x2=%d", _rectX2);
		makeDebugTextBlock(buf, 0, 150);

		sprintf(buf, "y2=%d", _rectY2);
		makeDebugTextBlock(buf, 0, 165);
	}

	// testingSnR indicator

	if (_testingSnR) {		// see fnAddHuman()
		sprintf(buf, "TESTING LOGIC STABILITY!");
		makeDebugTextBlock(buf, 0, 105);
	}

	// debug info at top of screen - enabled/disabled as one complete unit

	if (_displayTime) {
		int32 time = _vm->getMillis();

		if ((time - _startTime) / 1000 >= 10000)
			_startTime = time;

		time -= _startTime;
		sprintf(buf, "Time %.2d:%.2d:%.2d.%.3d", (time / 3600000) % 60, (time / 60000) % 60, (time / 1000) % 60, time % 1000);
		makeDebugTextBlock(buf, 500, 360);
		sprintf(buf, "Game %d", _vm->_gameCycle);
		makeDebugTextBlock(buf, 500, 380);
	}

	// current text number & speech-sample resource id

	if (_displayTextNumbers) {
		if (_textNumber) {
			if (_vm->_logic->readVar(SYSTEM_TESTING_TEXT)) {
				if (_vm->_logic->readVar(SYSTEM_WANT_PREVIOUS_LINE))
					sprintf(buf, "backwards");
				else
					sprintf(buf, "forwards");

				makeDebugTextBlock(buf, 0, 340);
			}

			sprintf(buf, "res: %d", _textNumber / SIZE);
			makeDebugTextBlock(buf, 0, 355);

			sprintf(buf, "pos: %d", _textNumber & 0xffff);
			makeDebugTextBlock(buf, 0, 370);

			sprintf(buf, "TEXT: %d", _vm->_logic->_officialTextNumber);
			makeDebugTextBlock(buf, 0, 385);
		}
	}

	// resource number currently being checking for animation

	if (_vm->_logic->readVar(SYSTEM_TESTING_ANIMS)) {
		sprintf(buf, "trying resource %d", _vm->_logic->readVar(SYSTEM_TESTING_ANIMS));
		makeDebugTextBlock(buf, 0, 90);
	}

	// general debug info

	if (_displayDebugText) {
/*
		// CD in use
		sprintf(buf, "CD-%d", currentCD);
		makeDebugTextBlock(buf, 0, 0);
*/

		// mouse coords & object pointed to

		if (_vm->_logic->readVar(CLICKED_ID))
			sprintf(buf, "last click at %d,%d (id %d: %s)",
				_vm->_logic->readVar(MOUSE_X),
				_vm->_logic->readVar(MOUSE_Y),
				_vm->_logic->readVar(CLICKED_ID),
				_vm->_resman->fetchName(_vm->_logic->readVar(CLICKED_ID)));
		else
			sprintf(buf, "last click at %d,%d (---)",
				_vm->_logic->readVar(MOUSE_X),
				_vm->_logic->readVar(MOUSE_Y));

		makeDebugTextBlock(buf, 0, 15);

		uint32 mouseTouching = _vm->_mouse->getMouseTouching();

		int mouseX, mouseY;

		_vm->_mouse->getPos(mouseX, mouseY);

		if (mouseTouching)
			sprintf(buf, "mouse %d,%d (id %d: %s)",
				mouseX + screenInfo->scroll_offset_x,
				mouseY + screenInfo->scroll_offset_y,
				mouseTouching,
				_vm->_resman->fetchName(mouseTouching));
		else
			sprintf(buf, "mouse %d,%d (not touching)",
				mouseX + screenInfo->scroll_offset_x,
				mouseY + screenInfo->scroll_offset_y);

		makeDebugTextBlock(buf, 0, 30);

		// player coords & graphic info
		// if player objct has a graphic

		if (_graphAnimRes)
			sprintf(buf, "player %d,%d %s (%d) #%d/%d",
				screenInfo->player_feet_x,
				screenInfo->player_feet_y,
				_vm->_resman->fetchName(_graphAnimRes),
				_graphAnimRes,
				_graphAnimPc,
				_graphNoFrames);
		else
			sprintf(buf, "player %d,%d --- %d",
				screenInfo->player_feet_x,
				screenInfo->player_feet_y,
				_graphAnimPc);

		makeDebugTextBlock(buf, 0, 45);

		// frames-per-second counter

		sprintf(buf, "fps %d", _vm->_screen->getFps());
		makeDebugTextBlock(buf, 440, 0);

		// location number

		sprintf(buf, "location=%d", _vm->_logic->readVar(LOCATION));
		makeDebugTextBlock(buf, 440, 15);

		// "result" variable

		sprintf(buf, "result=%d", _vm->_logic->readVar(RESULT));
		makeDebugTextBlock(buf, 440, 30);

		// no. of events in event list

		sprintf(buf, "events=%d", _vm->_logic->countEvents());
		makeDebugTextBlock(buf, 440, 45);

		// sprite list usage

		sprintf(buf, "bgp0: %d/%d", _vm->_screen->getCurBgp0(), MAX_bgp0_sprites);
		makeDebugTextBlock(buf, 560, 0);

		sprintf(buf, "bgp1: %d/%d", _vm->_screen->getCurBgp1(), MAX_bgp1_sprites);
		makeDebugTextBlock(buf, 560, 15);

		sprintf(buf, "back: %d/%d", _vm->_screen->getCurBack(), MAX_back_sprites);
		makeDebugTextBlock(buf, 560, 30);

		sprintf(buf, "sort: %d/%d", _vm->_screen->getCurSort(), MAX_sort_sprites);
		makeDebugTextBlock(buf, 560, 45);

		sprintf(buf, "fore: %d/%d", _vm->_screen->getCurFore(), MAX_fore_sprites);
		makeDebugTextBlock(buf, 560, 60);

		sprintf(buf, "fgp0: %d/%d", _vm->_screen->getCurFgp0(), MAX_fgp0_sprites);
		makeDebugTextBlock(buf, 560, 75);

		sprintf(buf, "fgp1: %d/%d", _vm->_screen->getCurFgp1(), MAX_fgp1_sprites);
		makeDebugTextBlock(buf, 560, 90);

		// largest layer & sprite

		// NB. Strings already constructed in Build_display.cpp
		makeDebugTextBlock(_vm->_screen->getLargestLayerInfo(), 0, 60);
		makeDebugTextBlock(_vm->_screen->getLargestSpriteInfo(), 0, 75);

		// "waiting for person" indicator - set form fnTheyDo and
		// fnTheyDoWeWait

		if (_speechScriptWaiting) {
			sprintf(buf, "script waiting for %s (%d)",
				_vm->_resman->fetchName(_speechScriptWaiting),
				_speechScriptWaiting);
			makeDebugTextBlock(buf, 0, 90);
		}

		// variable watch display

		showVarPos = 115;	// y-coord for first showVar

		for (showVarNo = 0; showVarNo < MAX_SHOWVARS; showVarNo++) {
			varNo = _showVar[showVarNo];	// get variable number

			// if non-zero ie. cannot watch 'id' but not needed
			// anyway because it changes throughout the logic loop

			if (varNo) {
				sprintf(buf, "var(%d) = %d", varNo, _vm->_logic->readVar(varNo));
				makeDebugTextBlock(buf, 530, showVarPos);
				showVarPos += 15;	// next line down
			}
		}

		// memory indicator - this should come last, to show all the
		// sprite blocks above!

		uint32 totAlloc = _vm->_memory->getTotAlloc();
		int16 numBlocks = _vm->_memory->getNumBlocks();

		if (totAlloc < 1024)
			sprintf(buf, "%u bytes in %d memory blocks", totAlloc, numBlocks);
		else if (totAlloc < 1024 * 1024)
			sprintf(buf, "%uK in %d memory blocks", totAlloc / 1024, numBlocks);
		else
			sprintf(buf, "%.02fM in %d memory blocks", totAlloc / 1048576., numBlocks);

		makeDebugTextBlock(buf, 0, 0);
	}
}

void Debugger::drawDebugGraphics() {
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();
	// walk-grid

	if (_displayWalkGrid)
		_vm->_logic->_router->plotWalkGrid();

	// player feet coord marker

	if (_displayPlayerMarker)
		plotCrossHair(screenInfo->player_feet_x, screenInfo->player_feet_y, 215);

	// mouse marker & coords

	if (_displayMouseMarker) {
		int mouseX, mouseY;

		_vm->_mouse->getPos(mouseX, mouseY);

		plotCrossHair(mouseX + screenInfo->scroll_offset_x, mouseY + screenInfo->scroll_offset_y, 215);
	}

	// mouse area rectangle / sprite box rectangle when testing anims

	if (_vm->_logic->readVar(SYSTEM_TESTING_ANIMS)) {
		// draw box around current frame
		drawRect(_rectX1, _rectY1, _rectX2, _rectY2, 184);
	} else if (_draggingRectangle) {
		// defining a mouse area the easy way, by creating a box
		// on-screen
		if (_rectFlicker)
			drawRect(_rectX1, _rectY1, _rectX2, _rectY2, 184);
	}
}

void Debugger::plotCrossHair(int16 x, int16 y, uint8 pen) {
	_vm->_screen->plotPoint(x, y, pen);

	_vm->_screen->drawLine(x - 2, y, x - 5, y, pen);
	_vm->_screen->drawLine(x + 2, y, x + 5, y, pen);

	_vm->_screen->drawLine(x, y - 2, x, y - 5, pen);
	_vm->_screen->drawLine(x, y + 2, x, y + 5, pen);
}

void Debugger::drawRect(int16 x1, int16 y1, int16 x2, int16 y2, uint8 pen) {
	_vm->_screen->drawLine(x1, y1, x2, y1, pen);	// top edge
	_vm->_screen->drawLine(x1, y2, x2, y2, pen);	// bottom edge
	_vm->_screen->drawLine(x1, y1, x1, y2, pen);	// left edge
	_vm->_screen->drawLine(x2, y1, x2, y2, pen);	// right edge
}

} // End of namespace Sword2
