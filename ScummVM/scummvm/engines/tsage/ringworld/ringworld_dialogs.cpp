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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/translation.h"

#include "gui/dialog.h"
#include "gui/widget.h"

#include "tsage/tsage.h"
#include "tsage/core.h"
#include "tsage/dialogs.h"
#include "tsage/staticres.h"
#include "tsage/globals.h"
#include "tsage/ringworld/ringworld_dialogs.h"
#include "tsage/ringworld/ringworld_logic.h"

namespace TsAGE {

namespace Ringworld {

/*--------------------------------------------------------------------------*/

#define BUTTON_WIDTH 28
#define BUTTON_HEIGHT 29

RightClickButton::RightClickButton(int buttonIndex, int xp, int yp) : GfxButton() {
	_buttonIndex = buttonIndex;
	this->_bounds.left = xp;
	this->_bounds.top = yp;
	this->_bounds.setWidth(BUTTON_WIDTH);
	this->_bounds.setHeight(BUTTON_HEIGHT);
	_savedButton = NULL;
}

void RightClickButton::highlight() {
	if (_savedButton) {
		// Button was previously highlighted, so de-highlight by restoring saved area
		g_globals->gfxManager().copyFrom(*_savedButton, _bounds.left, _bounds.top);
		delete _savedButton;
		_savedButton = NULL;
	} else {
		// Highlight button by getting the needed highlighted image resource
		_savedButton = surfaceGetArea(g_globals->gfxManager().getSurface(), _bounds);

		uint size;
		byte *imgData = g_resourceManager->getSubResource(7, 2, _buttonIndex, &size);

		GfxSurface btnSelected = surfaceFromRes(imgData);
		g_globals->gfxManager().copyFrom(btnSelected, _bounds.left, _bounds.top);

		DEALLOCATE(imgData);
	}
}

/*--------------------------------------------------------------------------*/

/**
 * This dialog implements the right-click dialog
 */
RightClickDialog::RightClickDialog() : GfxDialog(),
		_walkButton(1, 48, 12), _lookButton(2, 31, 29), _useButton(3, 65, 29),
		_talkButton(4, 14, 47), _inventoryButton(5, 48, 47), _optionsButton(6, 83, 47) {
	Rect rectArea, dialogRect;

	// Set the palette and change the cursor
	_gfxManager.setDialogPalette();
	g_globals->_events.setCursor(CURSOR_ARROW);

	// Get the dialog image
	_surface = surfaceFromRes(7, 1, 1);

	// Set the dialog position
	dialogRect.resize(_surface, 0, 0, 100);
	dialogRect.center(g_globals->_events._mousePos.x, g_globals->_events._mousePos.y);

	// Ensure the dialog will be entirely on-screen
	Rect screenRect = g_globals->gfxManager()._bounds;
	screenRect.collapse(4, 4);
	dialogRect.contain(screenRect);

	_bounds = dialogRect;
	_gfxManager._bounds = _bounds;

	_highlightedButton = NULL;
	_selectedAction = -1;
}

RightClickDialog::~RightClickDialog() {
}

RightClickButton *RightClickDialog::findButton(const Common::Point &pt) {
	RightClickButton *btnList[] = {  &_walkButton, &_lookButton, &_useButton, &_talkButton, &_inventoryButton, &_optionsButton };

	for (int i = 0; i < 6; ++i) {
		btnList[i]->_owner = this;

		if (btnList[i]->_bounds.contains(pt))
			return btnList[i];
	}

	return NULL;
}

void RightClickDialog::draw() {
	// Save the covered background area
	_savedArea = surfaceGetArea(g_globals->_gfxManagerInstance.getSurface(), _bounds);

	// Draw the dialog image
	g_globals->gfxManager().copyFrom(_surface, _bounds.left, _bounds.top);
}

bool RightClickDialog::process(Event &event) {
	switch (event.eventType) {
	case EVENT_MOUSE_MOVE: {
		// Check whether a button is highlighted
		RightClickButton *btn = findButton(event.mousePos);

		if (btn != _highlightedButton) {
			// De-highlight any previously selected button
			if (_highlightedButton) {
				_highlightedButton->highlight();
				_highlightedButton = NULL;
			}
			if (btn) {
				// Highlight the new button
				btn->highlight();
				_highlightedButton = btn;
			}
		}
		event.handled = true;
		return true;
	}

	case EVENT_BUTTON_DOWN:
		// If a button is highlighted, then flag the selected button index
		if (_highlightedButton)
			_selectedAction = _highlightedButton->_buttonIndex;
		else
			_selectedAction = _lookButton._buttonIndex;
		event.handled = true;
		return true;

	default:
		break;
	}

	return false;
}

void RightClickDialog::execute() {
	// Draw the dialog
	draw();

	// Dialog event handler loop
	_gfxManager.activate();

	while (!g_vm->shouldQuit() && (_selectedAction == -1)) {
		Event evt;
		while (g_globals->_events.getEvent(evt, EVENT_MOUSE_MOVE | EVENT_BUTTON_DOWN)) {
			evt.mousePos.x -= _bounds.left;
			evt.mousePos.y -= _bounds.top;

			process(evt);
		}

		g_system->delayMillis(10);
		GLOBALS._screenSurface.updateScreen();
	}

	_gfxManager.deactivate();

	// Execute the specified action
	switch (_selectedAction) {
	case 1:
		// Look action
		g_globals->_events.setCursor(CURSOR_LOOK);
		break;
	case 2:
		// Walk action
		g_globals->_events.setCursor(CURSOR_WALK);
		break;
	case 3:
		// Use cursor
		g_globals->_events.setCursor(CURSOR_USE);
		break;
	case 4:
		// Talk cursor
		g_globals->_events.setCursor(CURSOR_TALK);
		break;
	case 5:
		// Inventory dialog
		InventoryDialog::show();
		break;
	case 6:
		// Dialog options
		Ringworld::OptionsDialog::show();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void OptionsDialog::show() {
	OptionsDialog *dlg = new OptionsDialog();
	dlg->draw();

	GfxButton *btn = dlg->execute();

	if (btn == &dlg->_btnQuit) {
		// Quit game
		if (MessageDialog::show(QUIT_CONFIRM_MSG, CANCEL_BTN_STRING, QUIT_BTN_STRING) == 1) {
			g_vm->quitGame();
		}
	} else if (btn == &dlg->_btnRestart) {
		// Restart game
		g_globals->_game->restartGame();
	} else if (btn == &dlg->_btnSound) {
		// Sound dialog
		SoundDialog::execute();
	} else if (btn == &dlg->_btnSave) {
		// Save button
		g_globals->_game->saveGame();
	} else if (btn == &dlg->_btnRestore) {
		// Restore button
		g_globals->_game->restoreGame();
	}

	dlg->remove();
	delete dlg;
}

OptionsDialog::OptionsDialog() {
	// Set the element text
	_gfxMessage.set(OPTIONS_MSG, 140, ALIGN_LEFT);
	_btnRestore.setText(RESTORE_BTN_STRING);
	_btnSave.setText(SAVE_BTN_STRING);
	_btnRestart.setText(RESTART_BTN_STRING);
	_btnQuit.setText(QUIT_BTN_STRING);
	_btnSound.setText(SOUND_BTN_STRING);
	_btnResume.setText(RESUME_BTN_STRING);

	// Set position of the elements
	_gfxMessage._bounds.moveTo(0, 1);
	_btnRestore._bounds.moveTo(0, _gfxMessage._bounds.bottom + 1);
	_btnSave._bounds.moveTo(0, _btnRestore._bounds.bottom + 1);
	_btnRestart._bounds.moveTo(0, _btnSave._bounds.bottom + 1);
	_btnQuit._bounds.moveTo(0, _btnRestart._bounds.bottom + 1);
	_btnSound._bounds.moveTo(0, _btnQuit._bounds.bottom + 1);
	_btnResume._bounds.moveTo(0, _btnSound._bounds.bottom + 1);

	// Set all the buttons to the widest button
	GfxButton *btnList[6] = {&_btnRestore, &_btnSave, &_btnRestart, &_btnQuit, &_btnSound, &_btnResume};
	int16 btnWidth = 0;
	for (int idx = 0; idx < 6; ++idx)
		btnWidth = MAX(btnWidth, btnList[idx]->_bounds.width());
	for (int idx = 0; idx < 6; ++idx)
		btnList[idx]->_bounds.setWidth(btnWidth);

	// Add the items to the dialog
	addElements(&_gfxMessage, &_btnRestore, &_btnSave, &_btnRestart, &_btnQuit, &_btnSound, &_btnResume, NULL);

	// Set the dialog size and position
	frame();
	setCenter(160, 100);
}

/*--------------------------------------------------------------------------*/

bool GfxInvImage::process(Event &event) {
	if (!event.handled && (event.eventType == EVENT_BUTTON_DOWN)) {
		event.handled = _bounds.contains(event.mousePos);
		return event.handled;
	}

	return false;
}

/*--------------------------------------------------------------------------*/

void InventoryDialog::show() {
	// Determine how many items are in the player's inventory
	int itemCount = 0;
	SynchronizedList<InvObject *>::iterator i;
	for (i = RING_INVENTORY._itemList.begin(); i != RING_INVENTORY._itemList.end(); ++i) {
		if ((*i)->inInventory())
			++itemCount;
	}

	if (itemCount == 0) {
		MessageDialog::show(INV_EMPTY_MSG, OK_BTN_STRING);
		return;
	}

	InventoryDialog *dlg = new InventoryDialog();
	dlg->draw();
	dlg->execute();
	delete dlg;
}

InventoryDialog::InventoryDialog() {
	// Determine the maximum size of the image of any item in the player's inventory
	int imgWidth = 0, imgHeight = 0;

	SynchronizedList<InvObject *>::iterator i;
	for (i = RING_INVENTORY._itemList.begin(); i != RING_INVENTORY._itemList.end(); ++i) {
		InvObject *invObject = *i;
		if (invObject->inInventory()) {
			// Get the image for the item
			GfxSurface itemSurface = surfaceFromRes(invObject->_displayResNum, invObject->_rlbNum, invObject->_cursorNum);

			// Maintain the dimensions of the largest item image
			imgWidth = MAX(imgWidth, (int)itemSurface.getBounds().width());
			imgHeight = MAX(imgHeight, (int)itemSurface.getBounds().height());

			// Add the item to the display list
			GfxInvImage *img = new GfxInvImage();
			_images.push_back(img);
			img->setDetails(invObject->_displayResNum, invObject->_rlbNum, invObject->_cursorNum);
			img->_invObject = invObject;
			add(img);
		}
	}
	assert(_images.size() > 0);

	// Figure out the number of columns/rows to show all the items
	int cellsSize = 3;
	while ((cellsSize * cellsSize) < (int)_images.size())
		++cellsSize;

	// Set the position of each inventory item to be displayed
	int cellX = 0;
	Common::Point pt(0, 0);

	for (uint idx = 0; idx < _images.size(); ++idx) {
		if (cellX == cellsSize) {
			// Move to the start of the next line
			pt.x = 0;
			pt.y += imgHeight + 2;
			cellX = 0;
		}

		_images[idx]->_bounds.moveTo(pt.x, pt.y);

		pt.x += imgWidth + 2;
		++cellX;
	}

	// Set up the buttons
	pt.y += imgHeight + 2;
	_btnOk.setText(OK_BTN_STRING);
	_btnOk._bounds.moveTo((imgWidth + 2) * cellsSize - _btnOk._bounds.width(), pt.y);
	_btnLook.setText(LOOK_BTN_STRING);
	_btnLook._bounds.moveTo(_btnOk._bounds.left - _btnLook._bounds.width() - 2, _btnOk._bounds.top);
	addElements(&_btnLook, &_btnOk, NULL);

	frame();
	setCenter(SCREEN_CENTER_X, SCREEN_CENTER_Y);
}

InventoryDialog::~InventoryDialog() {
	for (uint idx = 0; idx < _images.size(); ++idx)
		delete _images[idx];
}

void InventoryDialog::execute() {
	if ((RING_INVENTORY._selectedItem) && RING_INVENTORY._selectedItem->inInventory())
		RING_INVENTORY._selectedItem->setCursor();

	GfxElement *hiliteObj;
	bool lookFlag = false;
	_gfxManager.activate();

	while (!g_vm->shouldQuit()) {
		// Get events
		Event event;
		while (!g_globals->_events.getEvent(event) && !g_vm->shouldQuit()) {
			g_system->delayMillis(10);
			GLOBALS._screenSurface.updateScreen();
		}
		if (g_vm->shouldQuit())
			break;

		hiliteObj = NULL;
		if ((event.eventType == EVENT_BUTTON_DOWN) && !_bounds.contains(event.mousePos))
			break;

		// Pass event to elements
		event.mousePos.x -= _gfxManager._bounds.left;
		event.mousePos.y -= _gfxManager._bounds.top;

		for (GfxElementList::iterator i = _elements.begin(); i != _elements.end(); ++i) {
			if ((*i)->process(event))
				hiliteObj = *i;
		}

		if (!event.handled && event.eventType == EVENT_KEYPRESS) {
			if ((event.kbd.keycode == Common::KEYCODE_RETURN) || (event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
				// Exit the dialog
				//hiliteObj = &_btnOk;
				break;
			}
		}

		if (hiliteObj == &_btnOk) {
			// Ok button clicked
			if (lookFlag)
				g_globals->_events.setCursor(CURSOR_WALK);
			break;
		} else if (hiliteObj == &_btnLook) {
			// Look button clicked
			if (_btnLook._message == LOOK_BTN_STRING) {
				_btnLook._message = PICK_BTN_STRING;
				lookFlag = 1;
				g_globals->_events.setCursor(CURSOR_LOOK);
			} else {
				_btnLook._message = LOOK_BTN_STRING;
				lookFlag = 0;
				g_globals->_events.setCursor(CURSOR_WALK);
			}

			hiliteObj->draw();
		} else if (hiliteObj) {
			// Inventory item selected
			InvObject *invObject = static_cast<GfxInvImage *>(hiliteObj)->_invObject;
			if (lookFlag) {
				g_globals->_screenSurface.displayText(invObject->_description);
			} else {
				RING_INVENTORY._selectedItem = invObject;
				invObject->setCursor();
			}
		}
	}

	_gfxManager.deactivate();
}

} // End of namespace Ringworld

} // End of namespace TsAGE
