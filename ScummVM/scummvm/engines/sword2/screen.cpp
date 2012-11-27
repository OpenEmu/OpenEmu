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

// ---------------------------------------------------------------------------
// BUILD_DISPLAY.CPP	like the old spr_engi but slightly more aptly named
// ---------------------------------------------------------------------------


#include "common/system.h"
#include "common/events.h"
#include "common/textconsole.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/console.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/mouse.h"
#include "sword2/object.h"
#include "sword2/resman.h"
#include "sword2/screen.h"
#include "sword2/sound.h"

namespace Sword2 {

Screen::Screen(Sword2Engine *vm, int16 width, int16 height) {
	_vm = vm;

	_dirtyGrid = _buffer = NULL;

	_screenWide = width;
	_screenDeep = height;

	_gridWide = width / CELLWIDE;
	_gridDeep = height / CELLDEEP;

	if ((width % CELLWIDE) || (height % CELLDEEP))
		error("Bad cell size");

	_dirtyGrid = (byte *)calloc(_gridWide, _gridDeep);
	if (!_dirtyGrid)
		error("Could not initialize dirty grid");

	_buffer = (byte *)malloc(width * height);
	if (!_buffer)
		error("Could not initialize display");

	for (int i = 0; i < ARRAYSIZE(_blockSurfaces); i++)
		_blockSurfaces[i] = NULL;

	_lightMask = NULL;
	_needFullRedraw = false;

	memset(&_thisScreen, 0, sizeof(_thisScreen));

	_fps = 0;
	_frameCount = 0;
	_cycleTime = 0;

	_lastPaletteRes = 0;

	_scrollFraction = 16;

	_largestLayerArea = 0;
	_largestSpriteArea = 0;

	strcpy(_largestLayerInfo,  "largest layer:  none registered");
	strcpy(_largestSpriteInfo, "largest sprite: none registered");

	_fadeStatus = RDFADE_NONE;
	_renderAverageTime = 60;

	_layer = 0;

	_dimPalette = false;

	_pauseTicks = 0;
	_pauseStartTick = 0;

	// Clean the cache for PSX version SCREENS.CLU
	_psxScrCache[0] = NULL;
	_psxScrCache[1] = NULL;
	_psxScrCache[2] = NULL;
	_psxCacheEnabled[0] = true;
	_psxCacheEnabled[1] = true;
	_psxCacheEnabled[2] = true;
}

Screen::~Screen() {
	flushPsxScrCache();
	free(_buffer);
	free(_dirtyGrid);
	closeBackgroundLayer();
	free(_lightMask);
}

uint32 Screen::getTick() {
	return _vm->getMillis() - _pauseTicks;
}

void Screen::pauseScreen(bool pause) {
	if (pause) {
		_pauseStartTick = _vm->_system->getMillis();
	} else {
		_pauseTicks += (_vm->_system->getMillis() - _pauseStartTick);
	}
}

/**
 * @return the graphics detail setting
 */

int8 Screen::getRenderLevel() {
	return _renderLevel;
}

void Screen::setRenderLevel(int8 level) {
	_renderLevel = level;

	switch (_renderLevel) {
	case 0:
		// Lowest setting: no fancy stuff
		_renderCaps = 0;
		break;
	case 1:
		// Medium-low setting: transparency-blending
		_renderCaps = RDBLTFX_SPRITEBLEND;
		break;
	case 2:
		// Medium-high setting: transparency-blending + shading
		_renderCaps = RDBLTFX_SPRITEBLEND | RDBLTFX_SHADOWBLEND;
		break;
	case 3:
		// Highest setting: transparency-blending + shading +
		// edge-blending + improved stretching
		_renderCaps = RDBLTFX_SPRITEBLEND | RDBLTFX_SHADOWBLEND | RDBLTFX_EDGEBLEND;
		break;
	}
}

/**
 * Tell updateDisplay() that the scene needs to be completely updated.
 */

void Screen::setNeedFullRedraw() {
	_needFullRedraw = true;
}

/**
 * Mark an area of the screen as dirty, first generation.
 */

void Screen::markAsDirty(int16 x0, int16 y0, int16 x1, int16 y1) {
	int16 gridX0 = x0 / CELLWIDE;
	int16 gridY0 = y0 / CELLDEEP;
	int16 gridX1 = x1 / CELLWIDE;
	int16 gridY1 = y1 / CELLDEEP;

	for (int16 i = gridY0; i <= gridY1; i++)
		for (int16 j = gridX0; j <= gridX1; j++)
			_dirtyGrid[i * _gridWide + j] = 2;
}

/**
 * This function has two purposes: It redraws the scene, and it handles input
 * events, palette fading, etc. It should be called at a high rate (> 20 per
 * second), but the scene is usually only redrawn about 12 times per second,
 * except when then screen is scrolling.
 *
 * @param redrawScene If true, redraw the scene.
 */

void Screen::updateDisplay(bool redrawScene) {
	_vm->parseInputEvents();
	fadeServer();

	if (redrawScene) {
		int i;

		// Note that the entire scene is always rendered, which is less
		// than optimal, but at least we can try to be intelligent
		// about updating the screen afterwards.

		if (_needFullRedraw) {
			// Update the entire screen. This is necessary when
			// scrolling, fading, etc.

			_vm->_system->copyRectToScreen(_buffer + MENUDEEP * _screenWide, _screenWide, 0, MENUDEEP, _screenWide, _screenDeep - 2 * MENUDEEP);
			_needFullRedraw = false;
		} else {
			// Update only the dirty areas of the screen

			int j, x, y;
			int stripWide;

			for (i = 0; i < _gridDeep; i++) {
				stripWide = 0;

				for (j = 0; j < _gridWide; j++) {
					if (_dirtyGrid[i * _gridWide + j]) {
						stripWide++;
					} else if (stripWide) {
						x = CELLWIDE * (j - stripWide);
						y = CELLDEEP * i;
						_vm->_system->copyRectToScreen(_buffer + y * _screenWide + x, _screenWide, x, y, stripWide * CELLWIDE, CELLDEEP);
						stripWide = 0;
					}
				}

				if (stripWide) {
					x = CELLWIDE * (j - stripWide);
					y = CELLDEEP * i;
					_vm->_system->copyRectToScreen(_buffer + y * _screenWide + x, _screenWide, x, y, stripWide * CELLWIDE, CELLDEEP);
					stripWide = 0;
				}
			}
		}

		// Age the dirty cells one generation. This way we keep track
		// of both the cells that were updated this time, and the ones
		// that were updated the last time.

		for (i = 0; i < _gridWide * _gridDeep; i++)
			_dirtyGrid[i] >>= 1;
	}

	// We always need to update because of fades, menu animations, etc.
	_vm->_system->updateScreen();
}

/**
 * Fill the screen buffer with palette color zero. Note that it does not
 * touch the menu areas of the screen.
 */

void Screen::clearScene() {
	memset(_buffer + MENUDEEP * _screenWide, 0, _screenWide * RENDERDEEP);
	_needFullRedraw = true;
}

void Screen::buildDisplay() {
	if (_thisScreen.new_palette) {
		// start the layer palette fading up
		startNewPalette();

		// should be reset to zero at start of each screen change
		_largestLayerArea = 0;
		_largestSpriteArea = 0;
	}

	// Does this ever happen?
	if (!_thisScreen.background_layer_id)
		return;

	// there is a valid screen to run

	setScrollTarget(_thisScreen.scroll_offset_x, _thisScreen.scroll_offset_y);
	_vm->_mouse->animateMouse();
	startRenderCycle();

	byte *file = _vm->_resman->openResource(_thisScreen.background_layer_id);

	MultiScreenHeader screenLayerTable;
	memset(&screenLayerTable, 0, sizeof(screenLayerTable));

	if (!Sword2Engine::isPsx()) // On PSX version, there would be nothing to read here
		screenLayerTable.read(file + ResHeader::size());

	// Render at least one frame, but if the screen is scrolling, and if
	// there is time left, we will render extra frames to smooth out the
	// scrolling.

	do {
		// first background parallax + related anims
		if (Sword2Engine::isPsx() || screenLayerTable.bg_parallax[0]) { // No need to check on PSX version
			renderParallax(_vm->fetchBackgroundParallaxLayer(file, 0), 0);
			drawBackPar0Frames();
		}

		// second background parallax + related anims
		if (!Sword2Engine::isPsx() && screenLayerTable.bg_parallax[1]) { // Nothing here in PSX version
			renderParallax(_vm->fetchBackgroundParallaxLayer(file, 1), 1);
			drawBackPar1Frames();
		}

		// normal backround layer (just the one!)
		renderParallax(_vm->fetchBackgroundLayer(file), 2);

		// sprites & layers
		drawBackFrames();	// background sprites
		drawSortFrames(file);	// sorted sprites & layers
		drawForeFrames();	// foreground sprites

		// first foreground parallax + related anims

		if (Sword2Engine::isPsx() || screenLayerTable.fg_parallax[0]) {
			renderParallax(_vm->fetchForegroundParallaxLayer(file, 0), 3);
			drawForePar0Frames();
		}

		// second foreground parallax + related anims

		if (!Sword2Engine::isPsx() && screenLayerTable.fg_parallax[1]) {
			renderParallax(_vm->fetchForegroundParallaxLayer(file, 1), 4);
			drawForePar1Frames();
		}

		_vm->_debugger->drawDebugGraphics();
		_vm->_fontRenderer->printTextBlocs();
		_vm->_mouse->processMenu();

		updateDisplay();

		_frameCount++;
		if (getTick() > _cycleTime) {
			_fps = _frameCount;
			_frameCount = 0;
			_cycleTime = getTick() + 1000;
		}
	} while (!endRenderCycle());

	_vm->_resman->closeResource(_thisScreen.background_layer_id);

}

/**
 * Fades down and displays a message on the screen.
 * @param text The message
 * @param time The number of seconds to display the message, or 0 to display it
 *             until the user clicks the mouse or presses a key.
 */

void Screen::displayMsg(byte *text, int time) {
	byte pal[256 * 3];
	byte oldPal[256 * 3];

	debug(2, "DisplayMsg: %s", text);

	if (getFadeStatus() != RDFADE_BLACK) {
		fadeDown();
		waitForFade();
	}

	_vm->_mouse->setMouse(0);
	_vm->_mouse->setLuggage(0);
	_vm->_mouse->closeMenuImmediately();

	clearScene();

	byte *text_spr = _vm->_fontRenderer->makeTextSprite(text, 640, 187, _vm->_speechFontId);

	FrameHeader frame;

	frame.read(text_spr);

	SpriteInfo spriteInfo;

	spriteInfo.x = _screenWide / 2 - frame.width / 2;
	if (!time)
		spriteInfo.y = _screenDeep / 2 - frame.height / 2 - MENUDEEP;
	else
		spriteInfo.y = 400 - frame.height;
	spriteInfo.w = frame.width;
	spriteInfo.h = frame.height;
	spriteInfo.scale = 0;
	spriteInfo.scaledWidth = 0;
	spriteInfo.scaledHeight	= 0;
	spriteInfo.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION | RDSPR_TRANS;
	spriteInfo.blend = 0;
	spriteInfo.data = text_spr + FrameHeader::size();
	spriteInfo.colorTable = 0;
	spriteInfo.isText = true;

	uint32 rv = drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x (in DisplayMsg)", rv);

	memcpy(oldPal, _palette, sizeof(oldPal));
	memset(pal, 0, sizeof(pal));

	pal[187 * 3 + 0] = 255;
	pal[187 * 3 + 1] = 255;
	pal[187 * 3 + 2] = 255;

	setPalette(0, 256, pal, RDPAL_FADE);
	fadeUp();
	free(text_spr);
	waitForFade();

	if (time > 0) {
		uint32 targetTime = _vm->_system->getMillis() + (time * 1000);
		_vm->sleepUntil(targetTime);
	} else {
		while (!_vm->shouldQuit()) {
			MouseEvent *me = _vm->mouseEvent();
			if (me && (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN)))
				break;

			if (_vm->keyboardEvent())
				break;

			updateDisplay();
			_vm->_system->delayMillis(50);
		}
	}

	fadeDown();
	waitForFade();
	clearScene();
	setPalette(0, 256, oldPal, RDPAL_FADE);
	fadeUp();
}

void Screen::drawBackPar0Frames() {
	// frame attached to 1st background parallax
	for (uint i = 0; i < _curBgp0; i++)
		processImage(&_bgp0List[i]);
}

void Screen::drawBackPar1Frames() {
	// frame attached to 2nd background parallax
	for (uint i = 0; i < _curBgp1; i++)
		processImage(&_bgp1List[i]);
}

void Screen::drawBackFrames() {
	// background sprite, fixed to main background
	for (uint i = 0; i < _curBack; i++)
		processImage(&_backList[i]);
}

void Screen::drawSortFrames(byte *file) {
	uint i, j;

	// Sort the sort list. Used to be a separate function, but it was only
	// called once, right before calling drawSortFrames().

	if (_curSort > 1) {
		for (i = 0; i < _curSort - 1; i++) {
			for (j = 0; j < _curSort - 1; j++) {
				if (_sortList[_sortOrder[j]].sort_y > _sortList[_sortOrder[j + 1]].sort_y) {
					SWAP(_sortOrder[j], _sortOrder[j + 1]);
				}
			}
		}
	}

	// Draw the sorted frames - layers, shrinkers & normal flat sprites

	for (i = 0; i < _curSort; i++) {
		if (_sortList[_sortOrder[i]].layer_number) {
			// it's a layer - minus 1 for true layer number
			// we need to know from the BuildUnit because the
			// layers will have been sorted in random order
			processLayer(file, _sortList[_sortOrder[i]].layer_number - 1);
		} else {
			// it's a sprite
			processImage(&_sortList[_sortOrder[i]]);
		}
	}
}

void Screen::drawForeFrames() {
	// foreground sprite, fixed to main background
	for (uint i = 0; i < _curFore; i++)
		processImage(&_foreList[i]);
}

void Screen::drawForePar0Frames() {
	// frame attached to 1st foreground parallax
	for (uint i = 0; i < _curFgp0; i++)
		processImage(&_fgp0List[i]);
}

void Screen::drawForePar1Frames() {
	// frame attached to 2nd foreground parallax
	for (uint i = 0; i < _curFgp1; i++)
		processImage(&_fgp1List[i]);
}

void Screen::processLayer(byte *file, uint32 layer_number) {

	LayerHeader layer_head;

	layer_head.read(_vm->fetchLayerHeader(file, layer_number));

	SpriteInfo spriteInfo;

	spriteInfo.x = layer_head.x;
	spriteInfo.y = layer_head.y;
	spriteInfo.w = layer_head.width;
	spriteInfo.scale = 0;
	spriteInfo.scaledWidth = 0;
	spriteInfo.scaledHeight = 0;
	spriteInfo.h = layer_head.height;
	spriteInfo.isText = false;

	// Layers are uncompressed in PSX version, RLE256 compressed
	// in PC version.
	if (Sword2Engine::isPsx()) {
		spriteInfo.type = RDSPR_TRANS | RDSPR_NOCOMPRESSION;
		spriteInfo.data = file + layer_head.offset;
	} else {
		spriteInfo.type = RDSPR_TRANS | RDSPR_RLE256FAST;
		spriteInfo.data = file + ResHeader::size() + layer_head.offset;
	}

	spriteInfo.blend = 0;
	spriteInfo.colorTable = 0;

	// check for largest layer for debug info

	uint32 current_layer_area = layer_head.width * layer_head.height;

	if (current_layer_area > _largestLayerArea) {
		_largestLayerArea = current_layer_area;
		sprintf(_largestLayerInfo,
			"largest layer:  %s layer(%d) is %dx%d",
			_vm->_resman->fetchName(_thisScreen.background_layer_id),
			layer_number, layer_head.width, layer_head.height);
	}

	uint32 rv = drawSprite(&spriteInfo);
	if (rv)
		error("Driver Error %.8x in processLayer(%d)", rv, layer_number);
}

void Screen::processImage(BuildUnit *build_unit) {

	// We have some problematic animation frames in PSX demo (looks like there is missing data),
	// so we just skip them.
	if ( (Sword2Engine::isPsx() &&  _vm->_logic->readVar(DEMO)) &&
		 ((build_unit->anim_resource == 369 && build_unit->anim_pc == 0) ||
		 (build_unit->anim_resource == 296 && build_unit->anim_pc == 5)  ||
		 (build_unit->anim_resource == 534 && build_unit->anim_pc == 13)) )
		return;

	byte *file = _vm->_resman->openResource(build_unit->anim_resource);
	byte *colTablePtr = NULL;

	byte *frame = _vm->fetchFrameHeader(file, build_unit->anim_pc);

	AnimHeader anim_head;
	CdtEntry cdt_entry;
	FrameHeader frame_head;

	anim_head.read(_vm->fetchAnimHeader(file));
	cdt_entry.read(_vm->fetchCdtEntry(file, build_unit->anim_pc));
	frame_head.read(frame);

	// so that 0-color is transparent
	uint32 spriteType = RDSPR_TRANS;

	if (anim_head.blend)
		spriteType |= RDSPR_BLEND;

	// if the frame is to be flipped (only really applicable to frames
	// using offsets)
	if (cdt_entry.frameType & FRAME_FLIPPED)
		spriteType |= RDSPR_FLIP;

	if (cdt_entry.frameType & FRAME_256_FAST) {
		// scaling, shading & blending don't work with RLE256FAST
		// but the same compression can be decompressed using the
		// RLE256 routines!

		// NOTE: If this restriction refers to drawSprite(), I don't
		// think we have it any more. But I'm not sure.

		if (build_unit->scale || anim_head.blend || build_unit->shadingFlag)
			spriteType |= RDSPR_RLE256;
		else
			spriteType |= RDSPR_RLE256FAST;
	} else {
		switch (anim_head.runTimeComp) {
		case NONE:
			spriteType |= RDSPR_NOCOMPRESSION;
			break;
		case RLE256:
			spriteType |= RDSPR_RLE256;
			break;
		case RLE16:
			spriteType |= RDSPR_RLE16;
			// points to just after last cdt_entry, ie.
			// start of color table
			colTablePtr = _vm->fetchAnimHeader(file) + AnimHeader::size() + anim_head.noAnimFrames * CdtEntry::size();
			if (Sword2Engine::isPsx())
				colTablePtr++; // There is one additional byte to skip before the table in psx version
			break;
		}
	}

	// if we want this frame to be affected by the shading mask,
	// add the status bit
	if (build_unit->shadingFlag)
		spriteType |= RDSPR_SHADOW;

	SpriteInfo spriteInfo;

	spriteInfo.x = build_unit->x;
	spriteInfo.y = build_unit->y;
	spriteInfo.w = frame_head.width;
	spriteInfo.h = frame_head.height;
	spriteInfo.scale = build_unit->scale;
	spriteInfo.scaledWidth = build_unit->scaled_width;
	spriteInfo.scaledHeight	= build_unit->scaled_height;
	spriteInfo.type = spriteType;
	spriteInfo.blend = anim_head.blend;
	// points to just after frame header, ie. start of sprite data
	spriteInfo.data = frame + FrameHeader::size();
	spriteInfo.colorTable = colTablePtr;
	spriteInfo.isText = false;

	// check for largest layer for debug info
	uint32 current_sprite_area = frame_head.width * frame_head.height;

	if (current_sprite_area > _largestSpriteArea) {
		_largestSpriteArea = current_sprite_area;
		sprintf(_largestSpriteInfo,
			"largest sprite: %s frame(%d) is %dx%d",
			_vm->_resman->fetchName(build_unit->anim_resource),
			build_unit->anim_pc,
			frame_head.width,
			frame_head.height);
	}

	if (_vm->_logic->readVar(SYSTEM_TESTING_ANIMS)) { // see anims.cpp
		// bring the anim into the visible screen
		// but leave extra pixel at edge for box
		if (spriteInfo.x + spriteInfo.scaledWidth >= 639)
			spriteInfo.x = 639 - spriteInfo.scaledWidth;

		if (spriteInfo.y + spriteInfo.scaledHeight >= 399)
			spriteInfo.y = 399 - spriteInfo.scaledHeight;

		if (spriteInfo.x < 1)
			spriteInfo.x = 1;

		if (spriteInfo.y < 1)
			spriteInfo.y = 1;

		// create box to surround sprite - just outside sprite box
		_vm->_debugger->_rectX1 = spriteInfo.x - 1;
		_vm->_debugger->_rectY1 = spriteInfo.y - 1;
		_vm->_debugger->_rectX2 = spriteInfo.x + spriteInfo.scaledWidth;
		_vm->_debugger->_rectY2 = spriteInfo.y + spriteInfo.scaledHeight;
	}

	uint32 rv = drawSprite(&spriteInfo);
	if (rv) {
		error("Driver Error %.8x with sprite %s (%d, %d) in processImage",
			rv,
			_vm->_resman->fetchName(build_unit->anim_resource),
			build_unit->anim_resource, build_unit->anim_pc);
	}

	// release the anim resource
	_vm->_resman->closeResource(build_unit->anim_resource);
}

void Screen::resetRenderLists() {
	// reset the sort lists - do this before a logic loop
	// takes into account the fact that the start of the list is pre-built
	// with the special sortable layers

	_curBgp0 = 0;
	_curBgp1 = 0;
	_curBack = 0;
	// beginning of sort list is setup with the special sort layers
	_curSort = _thisScreen.number_of_layers;
	_curFore = 0;
	_curFgp0 = 0;
	_curFgp1 = 0;

	if (_curSort) {
		// there are some layers - so rebuild the sort order list
		for (uint i = 0; i < _curSort; i++)
			_sortOrder[i] = i;
	}
}

void Screen::registerFrame(byte *ob_mouse, byte *ob_graph, byte *ob_mega, BuildUnit *build_unit) {
	ObjectGraphic obGraph(ob_graph);
	ObjectMega obMega(ob_mega);

	assert(obGraph.getAnimResource());

	byte *file = _vm->_resman->openResource(obGraph.getAnimResource());

	AnimHeader anim_head;
	CdtEntry cdt_entry;
	FrameHeader frame_head;

	anim_head.read(_vm->fetchAnimHeader(file));
	cdt_entry.read(_vm->fetchCdtEntry(file, obGraph.getAnimPc()));
	frame_head.read(_vm->fetchFrameHeader(file, obGraph.getAnimPc()));

	// update player graphic details for on-screen debug info
	if (_vm->_logic->readVar(ID) == CUR_PLAYER_ID) {
		_vm->_debugger->_graphType = obGraph.getType();
		_vm->_debugger->_graphAnimRes = obGraph.getAnimResource();
		// counting 1st frame as 'frame 1'
		_vm->_debugger->_graphAnimPc = obGraph.getAnimPc() + 1;
		_vm->_debugger->_graphNoFrames = anim_head.noAnimFrames;
	}

	// fill in the BuildUnit structure for this frame

	build_unit->anim_resource = obGraph.getAnimResource();
	build_unit->anim_pc = obGraph.getAnimPc();
	build_unit->layer_number = 0;

	// Affected by shading mask?
	if (obGraph.getType() & SHADED_SPRITE)
		build_unit->shadingFlag = true;
	else
		build_unit->shadingFlag = false;

	// Check if this frame has offsets ie. this is a scalable mega frame

	int scale = 0;

	if (cdt_entry.frameType & FRAME_OFFSET) {
		scale = obMega.calcScale();

		// calc final render coordinates (top-left of sprite), based
		// on feet coords & scaled offsets

		// add scaled offsets to feet coords
		build_unit->x = obMega.getFeetX() + (cdt_entry.x * scale) / 256;
		build_unit->y = obMega.getFeetY() + (cdt_entry.y * scale) / 256;

		// Work out new width and height. Always divide by 256 after
		// everything else, to maintain accurary
		build_unit->scaled_width = ((scale * frame_head.width) / 256);
		build_unit->scaled_height = ((scale * frame_head.height) / 256);
	} else {
		// It's a non-scaling anim. Get render coords for sprite, from cdt
		build_unit->x = cdt_entry.x;
		build_unit->y = cdt_entry.y;

		// Get width and height
		build_unit->scaled_width = frame_head.width;
		build_unit->scaled_height = frame_head.height;
	}

	// either 0 or required scale, depending on whether 'scale' computed
	build_unit->scale = scale;

	// calc the bottom y-coord for sorting purposes
	build_unit->sort_y = build_unit->y + build_unit->scaled_height - 1;

	if (ob_mouse) {
		// passed a mouse structure, so add to the _mouseList
		_vm->_mouse->registerMouse(ob_mouse, build_unit);

	}

	_vm->_resman->closeResource(obGraph.getAnimResource());
}

void Screen::registerFrame(byte *ob_mouse, byte *ob_graph, byte *ob_mega) {
	ObjectGraphic obGraph(ob_graph);

	// check low word for sprite type
	switch (obGraph.getType() & 0x0000ffff) {
	case BGP0_SPRITE:
		assert(_curBgp0 < MAX_bgp0_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_bgp0List[_curBgp0]);
		_curBgp0++;
		break;
	case BGP1_SPRITE:
		assert(_curBgp1 < MAX_bgp1_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_bgp1List[_curBgp1]);
		_curBgp1++;
		break;
	case BACK_SPRITE:
		assert(_curBack < MAX_back_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_backList[_curBack]);
		_curBack++;
		break;
	case SORT_SPRITE:
		assert(_curSort < MAX_sort_sprites);
		_sortOrder[_curSort] = _curSort;
		registerFrame(ob_mouse, ob_graph, ob_mega, &_sortList[_curSort]);
		_curSort++;
		break;
	case FORE_SPRITE:
		assert(_curFore < MAX_fore_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_foreList[_curFore]);
		_curFore++;
		break;
	case FGP0_SPRITE:
		assert(_curFgp0 < MAX_fgp0_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_fgp0List[_curFgp0]);
		_curFgp0++;
		break;
	case FGP1_SPRITE:
		assert(_curFgp1 < MAX_fgp1_sprites);
		registerFrame(ob_mouse, ob_graph, ob_mega, &_fgp1List[_curFgp1]);
		_curFgp1++;
		break;
	default:
		// NO_SPRITE no registering!
		break;
	}
}

// FIXME:
//
// The original credits used a different font. I think it's stored in the
// font.clu file, but I don't know how to interpret it.
//
// The original used the entire screen. This version cuts off the top and
// bottom of the screen, because that's where the menus would usually be.
//
// The original had some sort of smoke effect at the bottom of the screen.

enum {
	LINE_LEFT,
	LINE_CENTER,
	LINE_RIGHT
};

struct CreditsLine {
	char *str;
	byte type;
	int top;
	int height;
	byte *sprite;

	CreditsLine() {
		str = NULL;
		sprite = NULL;
	}

	~CreditsLine() {
		free(str);
		free(sprite);
		str = NULL;
		sprite = NULL;
	}
};

#define CREDITS_FONT_HEIGHT 25
#define CREDITS_LINE_SPACING 20

void Screen::rollCredits() {
	uint32 loopingMusicId = _vm->_sound->getLoopingMusicId();

	// Prepare for the credits by fading down, stoping the music, etc.

	_vm->_mouse->setMouse(0);

	_vm->_sound->muteFx(true);
	_vm->_sound->muteSpeech(true);

	waitForFade();
	fadeDown();
	waitForFade();

	_vm->_mouse->closeMenuImmediately();

	// There are three files which I believe are involved in showing the
	// credits:
	//
	// credits.bmp  - The "Smacker" logo, stored as follows:
	//
	//     width     2 bytes, little endian
	//     height    2 bytes, little endian
	//     palette   3 * 256 bytes
	//     data      width * height bytes
	//
	//     Note that the maximum color component in the palette is 0x3F.
	//     This is the same resolution as the _paletteMatch table. I doubt
	//     that this is a coincidence, but let's use the image palette
	//     directly anyway, just to be safe.
	//
	// credits.clu  - The credits text (credits.txt in PSX version)
	//
	//     This is simply a text file with CRLF line endings.
	//     '^' is not shown, but used to mark the center of the line.
	//     '@' is used as a placeholder for the "Smacker" logo. At least
	//     when it appears alone.
	//     Remaining lines are centered.
	//     The German version also contains character code 9 for no
	//     apparent reason. We ignore them.
	//
	// fonts.clu    - The credits font?
	//
	//     FIXME: At this time I don't know how to interpret fonts.clu. For
	//     now, let's just the standard speech font instead.

	SpriteInfo spriteInfo;
	Common::File f;
	int i;

	spriteInfo.isText = false;

	// Read the "Smacker" logo

	uint16 logoWidth = 0;
	uint16 logoHeight = 0;
	byte *logoData = NULL;
	byte palette[256 * 3];

	if (f.open("credits.bmp")) {
		logoWidth = f.readUint16LE();
		logoHeight = f.readUint16LE();

		for (i = 0; i < 256; i++) {
			palette[i * 3 + 0] = f.readByte() << 2;
			palette[i * 3 + 1] = f.readByte() << 2;
			palette[i * 3 + 2] = f.readByte() << 2;
		}

		logoData = (byte *)malloc(logoWidth * logoHeight);

		f.read(logoData, logoWidth * logoHeight);
		f.close();
	} else {
		warning("Can't find credits.bmp");
		memset(palette, 0, sizeof(palette));
		palette[14 * 3 + 0] = 252;
		palette[14 * 3 + 1] = 252;
		palette[14 * 3 + 2] = 252;
	}

	setPalette(0, 256, palette, RDPAL_INSTANT);

	// Read the credits text

	Common::Array<CreditsLine *> creditsLines;

	int lineCount = 0;
	int lineTop = 400;
	int paragraphStart = 0;
	bool hasCenterMark = false;

	if (Sword2Engine::isPsx()) {
		if (!f.open("credits.txt")) {
			warning("Can't find credits.txt");

			free(logoData);
			return;
		}
	} else {
		if (!f.open("credits.clu")) {
			warning("Can't find credits.clu");

			free(logoData);
			return;
		}
	}

	while (1) {
		char buffer[80];
		char *line = f.readLine(buffer, sizeof(buffer));

		if (line) {
			// Replace invalid character codes prevent the 'dud'
			// symbol from showing up in the credits.

			for (byte *ptr = (byte *)line; *ptr; ptr++) {
				switch (*ptr) {
				case 9:
					// The German credits contain these.
					// Convert them to spaces.
					*ptr = 32;
					break;
				case 10:
					// LF is treated as end of line.
					*ptr = 0;
					break;
				case 170:
					// The Spanish credits contain these.
					// Convert them to periods.
					*ptr = '.';
				default:
					break;
				}
			}
		}

		if (!line || *line == 0) {
			if (!hasCenterMark) {
				for (i = paragraphStart; i < lineCount; i++)
					creditsLines[i]->type = LINE_CENTER;
			}
			paragraphStart = lineCount;
			hasCenterMark = false;
			if (paragraphStart == lineCount)
				lineTop += CREDITS_LINE_SPACING;

			if (!line)
				break;

			continue;
		}

		char *center_mark = strchr(line, '^');

		if (center_mark) {
			// The current paragraph has at least one center mark.
			hasCenterMark = true;

			if (center_mark != line) {
				creditsLines.push_back(new CreditsLine);

				// The center mark is somewhere inside the
				// line. Split it into left and right side.
				*center_mark = 0;

				creditsLines[lineCount]->top = lineTop;
				creditsLines[lineCount]->height = CREDITS_FONT_HEIGHT;
				creditsLines[lineCount]->type = LINE_LEFT;
				creditsLines[lineCount]->str = strdup(line);

				lineCount++;
				*center_mark = '^';
			}

			line = center_mark;
		}

		creditsLines.push_back(new CreditsLine);

		creditsLines[lineCount]->top = lineTop;

		if (*line == '^') {
			creditsLines[lineCount]->type = LINE_RIGHT;
			line++;
		} else
			creditsLines[lineCount]->type = LINE_LEFT;

		if (strcmp(line, "@") == 0) {
			creditsLines[lineCount]->height = logoHeight;
			lineTop += logoHeight;
		} else {
			creditsLines[lineCount]->height = CREDITS_FONT_HEIGHT;
			lineTop += CREDITS_LINE_SPACING;
		}

		creditsLines[lineCount]->str = strdup(line);
		lineCount++;
	}

	f.close();

	// We could easily add some ScummVM stuff to the credits, if we wanted
	// to. On the other hand, anyone with the attention span to actually
	// read all the credits probably already knows. :-)

	// Start the music and roll the credits

	// The credits music (which can also be heard briefly in the "carib"
	// cutscene) is played once.

	_vm->_sound->streamCompMusic(309, false);

	clearScene();
	fadeUp(0);

	spriteInfo.scale = 0;
	spriteInfo.scaledWidth = 0;
	spriteInfo.scaledHeight = 0;
	spriteInfo.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION | RDSPR_TRANS;
	spriteInfo.blend = 0;

	int startLine = 0;
	int scrollPos = 0;

	bool abortCredits = false;

	int scrollSteps = lineTop + CREDITS_FONT_HEIGHT;
	uint32 musicStart = getTick();

	// Ideally the music should last just a tiny bit longer than the
	// credits. Note that musicTimeRemaining() will return 0 if the music
	// is muted, so we need a sensible fallback for that case.

	uint32 musicLength = MAX((int32)(1000 * (_vm->_sound->musicTimeRemaining() - 3)), 25 * (int32)scrollSteps);

	while (scrollPos < scrollSteps && !_vm->shouldQuit()) {
		clearScene();

		for (i = startLine; i < lineCount; i++) {
			if (!creditsLines[i])
				continue;

			// Free any sprites that have scrolled off the screen

			if (creditsLines[i]->top + creditsLines[i]->height < scrollPos) {
				debug(2, "Freeing line %d: '%s'", i, creditsLines[i]->str);

				delete creditsLines[i];
				creditsLines[i] = NULL;

				startLine = i + 1;
			} else if (creditsLines[i]->top < scrollPos + 400) {
				if (!creditsLines[i]->sprite) {
					debug(2, "Creating line %d: '%s'", i, creditsLines[i]->str);
					creditsLines[i]->sprite = _vm->_fontRenderer->makeTextSprite((byte *)creditsLines[i]->str, 600, 14, _vm->_speechFontId, 0);
				}

				FrameHeader frame;

				frame.read(creditsLines[i]->sprite);

				spriteInfo.y = creditsLines[i]->top - scrollPos;
				spriteInfo.w = frame.width;
				spriteInfo.h = frame.height;
				spriteInfo.data = creditsLines[i]->sprite + FrameHeader::size();
				spriteInfo.isText = true;

				switch (creditsLines[i]->type) {
				case LINE_LEFT:
					spriteInfo.x = RENDERWIDE / 2 - 5 - frame.width;
					break;
				case LINE_RIGHT:
					spriteInfo.x = RENDERWIDE / 2 + 5;
					break;
				case LINE_CENTER:
					if (strcmp(creditsLines[i]->str, "@") == 0) {
						spriteInfo.data = logoData;
						spriteInfo.x = (RENDERWIDE - logoWidth) / 2;
						spriteInfo.w = logoWidth;
						spriteInfo.h = logoHeight;
					} else
						spriteInfo.x = (RENDERWIDE - frame.width) / 2;
					break;
				}

				if (spriteInfo.data)
					drawSprite(&spriteInfo);
			} else
				break;
		}

		updateDisplay();

		KeyboardEvent *ke = _vm->keyboardEvent();

		if (ke && ke->kbd.keycode == Common::KEYCODE_ESCAPE) {
			if (!abortCredits) {
				abortCredits = true;
				fadeDown();
			}
		}

		if (abortCredits && getFadeStatus() == RDFADE_BLACK)
			break;

		_vm->sleepUntil(musicStart + (musicLength * scrollPos) / scrollSteps + _pauseTicks);
		scrollPos++;
	}

	// We're done. Clean up and try to put everything back where it was
	// before the credits.

	for (i = 0; i < lineCount; i++) {
		delete creditsLines[i];
	}

	free(logoData);

	if (!abortCredits) {
		fadeDown();

		// The music should either have stopped or be about to stop, so
		// wait for it to really happen.

		while (_vm->_sound->musicTimeRemaining() && !_vm->shouldQuit()) {
			updateDisplay(false);
			_vm->_system->delayMillis(100);
		}
	}

	if (_vm->shouldQuit())
		return;

	waitForFade();

	_vm->_sound->muteFx(false);
	_vm->_sound->muteSpeech(false);

	if (loopingMusicId)
		_vm->_sound->streamCompMusic(loopingMusicId, true);
	else
		_vm->_sound->stopMusic(false);

	if (!_vm->_mouse->getMouseStatus() || _vm->_mouse->isChoosing())
		_vm->_mouse->setMouse(NORMAL_MOUSE_ID);

	if (_vm->_logic->readVar(DEAD))
		_vm->_mouse->buildSystemMenu();
}

// This image used to be shown by CacheNewCluster() while copying a data file
// from the CD to the hard disk. ScummVM doesn't do that, so the image is never
// shown. It'd be nice if we could do something useful with it some day...

void Screen::splashScreen() {
	byte *bgfile = _vm->_resman->openResource(2950);

	initializeBackgroundLayer(NULL);
	initializeBackgroundLayer(NULL);
	initializeBackgroundLayer(_vm->fetchBackgroundLayer(bgfile));
	initializeBackgroundLayer(NULL);
	initializeBackgroundLayer(NULL);

	_vm->fetchPalette(bgfile, _palette);
	setPalette(0, 256, _palette, RDPAL_FADE);
	renderParallax(_vm->fetchBackgroundLayer(bgfile), 2);

	closeBackgroundLayer();

	byte *loadingBar = _vm->_resman->openResource(2951);
	byte *frame = _vm->fetchFrameHeader(loadingBar, 0);

	AnimHeader animHead;
	CdtEntry cdt;
	FrameHeader frame_head;

	animHead.read(_vm->fetchAnimHeader(loadingBar));
	cdt.read(_vm->fetchCdtEntry(loadingBar, 0));
	frame_head.read(_vm->fetchFrameHeader(loadingBar, 0));

	SpriteInfo barSprite;

	barSprite.x = cdt.x;
	barSprite.y = cdt.y;
	barSprite.w = frame_head.width;
	barSprite.h = frame_head.height;
	barSprite.scale = 0;
	barSprite.scaledWidth = 0;
	barSprite.scaledHeight = 0;
	barSprite.type = RDSPR_RLE256FAST | RDSPR_TRANS;
	barSprite.blend = 0;
	barSprite.colorTable = 0;
	barSprite.data = frame + FrameHeader::size();
	barSprite.isText = false;

	drawSprite(&barSprite);

	fadeUp();
	waitForFade();

	for (int i = 0; i < animHead.noAnimFrames; i++) {
		frame = _vm->fetchFrameHeader(loadingBar, i);
		barSprite.data = frame + FrameHeader::size();
		drawSprite(&barSprite);
		updateDisplay();
		_vm->_system->delayMillis(30);
	}

	_vm->_resman->closeResource(2951);

	fadeDown();
	waitForFade();
}

// Following functions are used to manage screen cache for psx version.

void Screen::setPsxScrCache(byte *psxScrCache, uint8 level) {
		if (level < 3) {
			if (psxScrCache)
				_psxCacheEnabled[level] = true;
			else
				_psxCacheEnabled[level] = false;

			_psxScrCache[level] = psxScrCache;
		}
}

byte *Screen::getPsxScrCache(uint8 level) {
	if (level > 3) {
		level = 0;
	}

	if (_psxCacheEnabled[level])
		return _psxScrCache[level];
	else
		return NULL;
}

bool Screen::getPsxScrCacheStatus(uint8 level) {
	if (level > 3) {
		level = 0;
	}

	return _psxCacheEnabled[level];
}

void Screen::flushPsxScrCache() {
	for (uint8 i = 0; i < 3; i++) {
		free(_psxScrCache[i]);
		_psxScrCache[i] = NULL;
		_psxCacheEnabled[i] = true;
	}
}

} // End of namespace Sword2
