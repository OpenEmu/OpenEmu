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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#include "common/debug.h"
#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/game.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/display.h"
#include "hugo/mouse.h"
#include "hugo/inventory.h"
#include "hugo/parser.h"
#include "hugo/object.h"

namespace Hugo {

static const int kMaxDisp = (kXPix / kInvDx);       // Max icons displayable

InventoryHandler::InventoryHandler(HugoEngine *vm) : _vm(vm), _invent(0) {
	_firstIconId = 0;
	_inventoryState  = kInventoryOff;               // Inventory icon bar state
	_inventoryHeight = 0;                           // Inventory icon bar pos
	_inventoryObjId  = -1;                          // Inventory object selected (none)
	_maxInvent = 0;
}

void InventoryHandler::setInventoryObjId(int16 objId) {
	_inventoryObjId = objId;
}

void InventoryHandler::setInventoryState(Istate state) {
	_inventoryState = state;
}

void InventoryHandler::freeInvent() {
	free(_invent);
}

int16 InventoryHandler::getInventoryObjId() const {
	return _inventoryObjId;
}

Istate InventoryHandler::getInventoryState() const {
	return _inventoryState;
}

/**
 * Read _invent from Hugo.dat
 */
void InventoryHandler::loadInvent(Common::SeekableReadStream &in) {
	for (int varnt = 0; varnt < _vm->_numVariant; varnt++) {
		int16 numElem = in.readUint16BE();
		if (varnt == _vm->_gameVariant) {
			_maxInvent = numElem;
			_invent = (int16 *)malloc(sizeof(int16) * numElem);
			for (int i = 0; i < numElem; i++)
				_invent[i] = in.readSint16BE();
		} else {
			in.skip(numElem * sizeof(int16));
		}
	}
}

/**
 * Construct the inventory scrollbar in dib_i
 * imageTotNumb is total number of inventory icons
 * displayNumb is number requested for display
 * scrollFl is TRUE if scroll arrows required
 * firstObjId is index of first (scrolled) inventory object to display
 */
void InventoryHandler::constructInventory(const int16 imageTotNumb, int displayNumb, const bool scrollFl, int16 firstObjId) {
	debugC(1, kDebugInventory, "constructInventory(%d, %d, %d, %d)", imageTotNumb, displayNumb, (scrollFl) ? 0 : 1, firstObjId);

	// Clear out icon buffer
	memset(_vm->_screen->getIconBuffer(), 0, sizeof(_vm->_screen->getIconBuffer()));

	// If needed, copy arrows - reduce number of icons displayable
	if (scrollFl) { // Display at first and last icon positions
		_vm->_screen->moveImage(_vm->_screen->getGUIBuffer(), 0, 0, kInvDx, kInvDy, kXPix, _vm->_screen->getIconBuffer(), 0, 0, kXPix);
		_vm->_screen->moveImage(_vm->_screen->getGUIBuffer(), kInvDx, 0, kInvDx, kInvDy, kXPix, _vm->_screen->getIconBuffer(), kInvDx *(kMaxDisp - 1), 0, kXPix);
		displayNumb = MIN(displayNumb, kMaxDisp - kArrowNumb);
	} else  // No, override first index - we can show 'em all!
		firstObjId = 0;

	// Copy inventory icons to remaining positions
	int16 displayed = 0;
	int16 carried = 0;
	for (int16 i = 0; (i < imageTotNumb) && (displayed < displayNumb); i++) {
		if (_vm->_object->isCarried(_invent[i])) {
			// Check still room to display and past first scroll index
			if (displayed < displayNumb && carried >= firstObjId) {
				// Compute source coordinates in dib_u
				int16 ux = (i + kArrowNumb) * kInvDx % kXPix;
				int16 uy = (i + kArrowNumb) * kInvDx / kXPix * kInvDy;

				// Compute dest coordinates in dib_i
				int16 ix = ((scrollFl) ? displayed + 1 : displayed) * kInvDx;
				displayed++;                        // Count number displayed

				// Copy the icon
				_vm->_screen->moveImage(_vm->_screen->getGUIBuffer(), ux, uy, kInvDx, kInvDy, kXPix, _vm->_screen->getIconBuffer(), ix, 0, kXPix);
			}
			carried++;                              // Count number carried
		}
	}
}

/**
 * Process required action for inventory
 * Returns objId under cursor (or -1) for INV_GET
 */
int16 InventoryHandler::processInventory(const InvAct action, ...) {
	debugC(1, kDebugInventory, "processInventory(InvAct action, ...)");

	int16 imageNumb;                                // Total number of inventory items
	int displayNumb;                                // Total number displayed/carried
	// Compute total number and number displayed, i.e. number carried
	for (imageNumb = 0, displayNumb = 0; imageNumb < _maxInvent && _invent[imageNumb] != -1; imageNumb++) {
		if (_vm->_object->isCarried(_invent[imageNumb]))
			displayNumb++;
	}

	// Will we need the scroll arrows?
	bool scrollFl = displayNumb > kMaxDisp;
	va_list marker;                                 // Args used for D_ADD operation
	int16 cursorx, cursory;                         // Current cursor position
	int16 objId = -1;                               // Return objid under cursor

	switch (action) {
	case kInventoryActionInit:                      // Initialize inventory display
		constructInventory(imageNumb, displayNumb, scrollFl, _firstIconId);
		break;
	case kInventoryActionLeft:                      // Scroll left by one icon
		_firstIconId = MAX(0, _firstIconId - 1);
		constructInventory(imageNumb, displayNumb, scrollFl, _firstIconId);
		break;
	case kInventoryActionRight:                     // Scroll right by one icon
		_firstIconId = MIN(displayNumb, _firstIconId + 1);
		constructInventory(imageNumb, displayNumb, scrollFl, _firstIconId);
		break;
	case kInventoryActionGet:                       // Return object id under cursor
		// Get cursor position from variable argument list
		va_start(marker, action);                   // Initialize variable arguments
		cursorx = va_arg(marker, int);              // Cursor x
		cursory = va_arg(marker, int);              // Cursor y
		va_end(marker);                             // Reset variable arguments

		cursory -= kDibOffY;                        // Icon bar is at true zero
		if (cursory > 0 && cursory < kInvDy) {      // Within icon bar?
			int16 i = cursorx / kInvDx;             // Compute icon index
			if (scrollFl) {                         // Scroll buttons displayed
				if (i == 0) {                       // Left scroll button
					objId = kLeftArrow;
				} else {
					if (i == kMaxDisp - 1)          // Right scroll button
						objId = kRightArrow;
					else                            // Adjust for scroll
						i += _firstIconId - 1;      // i is icon index
				}
			}

			// If not an arrow, find object id - limit to valid range
			if (objId == -1 && i < displayNumb) {
				// Find objid by counting # carried objects == i+1
				int16 j;
				for (j = 0, i++; i > 0 && j < _vm->_object->_numObj; j++) {
					if (_vm->_object->isCarried(j)) {
						if (--i == 0)
							objId = j;
					}
				}
			}
		}
		break;
	}
	return objId;                                   // For the INV_GET action
}

/**
 * Process inventory state machine
 */
void InventoryHandler::runInventory() {
	Status &gameStatus = _vm->getGameStatus();

	debugC(1, kDebugInventory, "runInventory");

	switch (_inventoryState) {
	case kInventoryOff:                             // Icon bar off screen
		break;
	case kInventoryUp:                              // Icon bar moving up
		_inventoryHeight -= kStepDy;                // Move the icon bar up
		if (_inventoryHeight <= 0)                  // Limit travel
			_inventoryHeight = 0;

		// Move visible portion to _frontBuffer, restore uncovered portion, display results
		_vm->_screen->moveImage(_vm->_screen->getIconBuffer(), 0, 0, kXPix, _inventoryHeight, kXPix, _vm->_screen->getFrontBuffer(), 0, kDibOffY, kXPix);
		_vm->_screen->moveImage(_vm->_screen->getBackBufferBackup(), 0, _inventoryHeight + kDibOffY, kXPix, kStepDy, kXPix, _vm->_screen->getFrontBuffer(), 0, _inventoryHeight + kDibOffY, kXPix);
		_vm->_screen->displayRect(0, kDibOffY, kXPix, _inventoryHeight + kStepDy);

		if (_inventoryHeight == 0) {                // Finished moving up?
			// Yes, restore dibs and exit back to game state machine
			_vm->_screen->moveImage(_vm->_screen->getBackBufferBackup(), 0, 0, kXPix, kYPix, kXPix, _vm->_screen->getBackBuffer(), 0, 0, kXPix);
			_vm->_screen->moveImage(_vm->_screen->getBackBuffer(), 0, 0, kXPix, kYPix, kXPix, _vm->_screen->getFrontBuffer(), 0, 0, kXPix);
			_vm->_object->updateImages();           // Add objects back into display list for restore
			_inventoryState = kInventoryOff;
			gameStatus._viewState = kViewPlay;
		}
		break;
	case kInventoryDown:                            // Icon bar moving down
		// If this is the first step, initialize dib_i
		// and get any icon/text out of _frontBuffer
		if (_inventoryHeight == 0) {
			processInventory(kInventoryActionInit); // Initialize dib_i
			_vm->_screen->displayList(kDisplayRestore); // Restore _frontBuffer
			_vm->_object->updateImages();           // Rebuild _frontBuffer without icons/text
			_vm->_screen->displayList(kDisplayDisplay); // Blit display list to screen
		}

		_inventoryHeight += kStepDy;                // Move the icon bar down
		if (_inventoryHeight > kInvDy)              // Limit travel
			_inventoryHeight = kInvDy;

		// Move visible portion to _frontBuffer, display results
		_vm->_screen->moveImage(_vm->_screen->getIconBuffer(), 0, 0, kXPix, _inventoryHeight, kXPix, _vm->_screen->getFrontBuffer(), 0, kDibOffY, kXPix);
		_vm->_screen->displayRect(0, kDibOffY, kXPix, _inventoryHeight);

		if (_inventoryHeight == kInvDy) {           // Finished moving down?
			// Yes, prepare view dibs for special inventory display since
			// we can't refresh objects while icon bar overlayed...
			// 1. Save backing store _backBuffer in temporary dib_c
			// 2. Make snapshot of _frontBuffer the new _backBuffer backing store
			// 3. Reset the display list
			_vm->_screen->moveImage(_vm->_screen->getBackBuffer(), 0, 0, kXPix, kYPix, kXPix, _vm->_screen->getBackBufferBackup(), 0, 0, kXPix);
			_vm->_screen->moveImage(_vm->_screen->getFrontBuffer(), 0, 0, kXPix, kYPix, kXPix, _vm->_screen->getBackBuffer(), 0, 0, kXPix);
			_vm->_screen->displayList(kDisplayInit);
			_inventoryState = kInventoryActive;
		}
		break;
	case kInventoryActive:                          // Inventory active
		_vm->_parser->charHandler();                // Still allow commands
		_vm->_screen->displayList(kDisplayRestore); // Restore previous background
		_vm->_screen->displayList(kDisplayDisplay); // Blit the display list to screen
		break;
	}
}


/**
 * Find index of dragged icon
 */
int16 InventoryHandler::findIconId(int16 objId) {
	int16 iconId = 0;
	for (; iconId < _maxInvent; iconId++) {
		if (objId == _invent[iconId])
			break;
	}

	return iconId;
}

} // End of namespace Hugo
