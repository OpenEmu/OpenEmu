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

// high level layer initializing

// the system supports:
//	1 optional background parallax layer
//	1 not optional normal backdrop layer
//	3 normal sorted layers
//	up to 2 foreground parallax layers


#include "common/rect.h"
#include "common/textconsole.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/logic.h"
#include "sword2/mouse.h"
#include "sword2/resman.h"
#include "sword2/screen.h"
#include "sword2/sound.h"

namespace Sword2 {

/**
 * This function is called when entering a new room.
 * @param res resource id of the normal background layer
 * @param new_palette 1 for new palette, otherwise 0
 */

void Screen::initBackground(int32 res, int32 new_palette) {
	int i;

	assert(res);

	_vm->_sound->clearFxQueue(false);
	waitForFade();

	debug(1, "CHANGED TO LOCATION \"%s\"", _vm->_resman->fetchName(res));

	// We have to clear this. Otherwise, if an exit warps back to the same
	// room (e.g. the jungle maze), clicking on the same exit again will be
	// misinterpreted as a double-click, and that only works if we're
	// actually walking towards that exit. Otherwise, the game would hang.

	_vm->_logic->writeVar(EXIT_CLICK_ID, 0);

	// if last screen was using a shading mask (see below)
	if (_thisScreen.mask_flag) {
		if (closeLightMask() != RD_OK)
			error("Could not close light mask");
	}

	// Close the previous screen, if one is open
	if (_thisScreen.background_layer_id)
		closeBackgroundLayer();

	_thisScreen.background_layer_id = res;
	_thisScreen.new_palette = new_palette;

	// ok, now read the resource and pull out all the normal sort layer
	// info/and set them up at the beginning of the sort list - why do it
	// each cycle

	byte *file = _vm->_resman->openResource(_thisScreen.background_layer_id);
	ScreenHeader screen_head;

	screen_head.read(_vm->fetchScreenHeader(file));

	// set number of special sort layers
	_thisScreen.number_of_layers = screen_head.noLayers;
	_thisScreen.screen_wide = screen_head.width;
	_thisScreen.screen_deep = screen_head.height;

	debug(2, "layers=%d width=%d depth=%d", screen_head.noLayers, screen_head.width, screen_head.height);

	// initialize the driver back buffer
	setLocationMetrics(screen_head.width, screen_head.height);

	for (i = 0; i < screen_head.noLayers; i++) {
		debug(3, "init layer %d", i);

		LayerHeader layer;

		layer.read(_vm->fetchLayerHeader(file, i));

		// Add the layer to the sort list. We only provide just enough
		// information so that it's clear that it's a layer, and where
		// to sort it in relation to other things in the list.

		_sortList[i].layer_number = i + 1;
		_sortList[i].sort_y = layer.y + layer.height;
	}

	// reset scroll offsets
	_thisScreen.scroll_offset_x = 0;
	_thisScreen.scroll_offset_y = 0;

	if (screen_head.width > _screenWide || screen_head.height > _screenDeep) {
		// The layer is larger than the physical screen. Switch on
		// scrolling. (2 means first time on screen)
		_thisScreen.scroll_flag = 2;

		// Note: if we've already set the player up then we could do
		// the initial scroll set here

		// Calculate the maximum scroll offsets to prevent scrolling
		// off the edge. The minimum offsets are both 0.

		_thisScreen.max_scroll_offset_x = screen_head.width - _screenWide;
		_thisScreen.max_scroll_offset_y = screen_head.height - (_screenDeep - (MENUDEEP * 2));
	} else {
		// The later fits on the phyiscal screen. Switch off scrolling.
		_thisScreen.scroll_flag = 0;
	}

	resetRenderEngine();

	// These are the physical screen coords where the system will try to
	// maintain George's actual feet coords.

	_thisScreen.feet_x = 320;
	_thisScreen.feet_y = 340;

	// shading mask

	MultiScreenHeader screenLayerTable;

	screenLayerTable.read(file + ResHeader::size());

	if (screenLayerTable.maskOffset) {
		SpriteInfo spriteInfo;

		spriteInfo.x = 0;
		spriteInfo.y = 0;
		spriteInfo.w = screen_head.width;
		spriteInfo.h = screen_head.height;
		spriteInfo.scale = 0;
		spriteInfo.scaledWidth = 0;
		spriteInfo.scaledHeight = 0;
		spriteInfo.type = 0;
		spriteInfo.blend = 0;
		spriteInfo.data = _vm->fetchShadingMask(file);
		spriteInfo.colorTable = 0;

		if (openLightMask(&spriteInfo) != RD_OK)
			error("Could not open light mask");

		// so we know to close it later! (see above)
		_thisScreen.mask_flag = true;
	} else {
		// no need to close a mask later
		_thisScreen.mask_flag = false;
	}

	// Background parallax layers

	for (i = 0; i < 2; i++) {
		if (screenLayerTable.bg_parallax[i])
			initializeBackgroundLayer(_vm->fetchBackgroundParallaxLayer(file, i));
		else
			initializeBackgroundLayer(NULL);
	}

	// Normal backround layer

	initializeBackgroundLayer(_vm->fetchBackgroundLayer(file));

	// Foreground parallax layers

	for (i = 0; i < 2; i++) {
		if (screenLayerTable.fg_parallax[i])
			initializeBackgroundLayer(_vm->fetchForegroundParallaxLayer(file, i));
		else
			initializeBackgroundLayer(NULL);
	}

	_vm->_resman->closeResource(_thisScreen.background_layer_id);
}

/**
 * This function is called when entering a new room, PSX edition
 * @param res resource id of the normal background layer
 * @param new_palette 1 for new palette, otherwise 0
 */

void Screen::initPsxBackground(int32 res, int32 new_palette) {
	int i;

	assert(res);

	_vm->_sound->clearFxQueue(false);
	waitForFade();

	debug(1, "CHANGED TO LOCATION \"%s\"", _vm->_resman->fetchName(res));

	_vm->_logic->writeVar(EXIT_CLICK_ID, 0);

	// Close the previous screen, if one is open
	if (_thisScreen.background_layer_id)
		closeBackgroundLayer();

	_thisScreen.background_layer_id = res;
	_thisScreen.new_palette = new_palette;

	// ok, now read the resource and pull out all the normal sort layer
	// info/and set them up at the beginning of the sort list - why do it
	// each cycle

	byte *file = _vm->_resman->openResource(_thisScreen.background_layer_id);
	ScreenHeader screen_head;

	screen_head.read(_vm->fetchScreenHeader(file));
	screen_head.height *= 2;

	// set number of special sort layers
	_thisScreen.number_of_layers = screen_head.noLayers;
	_thisScreen.screen_wide = screen_head.width;
	_thisScreen.screen_deep = screen_head.height;

	debug(2, "layers=%d width=%d depth=%d", screen_head.noLayers, screen_head.width, screen_head.height);

	// initialize the driver back buffer
	setLocationMetrics(screen_head.width, screen_head.height);

	for (i = 0; i < screen_head.noLayers; i++) {
		debug(3, "init layer %d", i);

		LayerHeader layer;

		layer.read(_vm->fetchLayerHeader(file, i));
		_sortList[i].layer_number = i + 1;
		_sortList[i].sort_y = layer.y + layer.height;
	}

	// reset scroll offsets
	_thisScreen.scroll_offset_x = 0;
	_thisScreen.scroll_offset_y = 0;

	if (screen_head.width > _screenWide || screen_head.height > _screenDeep) {
		_thisScreen.scroll_flag = 2;

		_thisScreen.max_scroll_offset_x = screen_head.width - _screenWide;
		_thisScreen.max_scroll_offset_y = screen_head.height - (_screenDeep - (MENUDEEP * 2));
	} else {
		// The later fits on the phyiscal screen. Switch off scrolling.
		_thisScreen.scroll_flag = 0;
	}

	resetRenderEngine();

	// These are the physical screen coords where the system will try to
	// maintain George's actual feet coords.

	_thisScreen.feet_x = 320;
	_thisScreen.feet_y = 340;

	// Background parallax layers
	initializePsxParallaxLayer(_vm->fetchBackgroundParallaxLayer(file, 0));
	initializePsxParallaxLayer(NULL);

	// Normal backround layer
	initializePsxBackgroundLayer(_vm->fetchBackgroundLayer(file));

	// Foreground parallax layers
	initializePsxParallaxLayer(_vm->fetchForegroundParallaxLayer(file, 1));
	initializePsxParallaxLayer(NULL);

	_vm->_resman->closeResource(_thisScreen.background_layer_id);

}

} // End of namespace Sword2
