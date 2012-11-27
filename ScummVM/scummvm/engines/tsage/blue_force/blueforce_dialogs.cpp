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
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/ringworld/ringworld_logic.h"

namespace TsAGE {

namespace BlueForce {

/**
 * This dialog implements the right-click dialog
 */
RightClickDialog::RightClickDialog() : GfxDialog() {
	// Setup button areas
	_rectList1[0] = Rect(7, 50, 41, 67);
	_rectList1[1] = Rect(13, 27, 50, 50);
	_rectList1[2] = Rect(49, 27, 84, 50);
	_rectList1[3] = Rect(56, 50, 90, 67);
	_rectList1[4] = Rect(26, 68, 69, 99);

	_rectList3[0] = Rect(12, 49, 27, 64);
	_rectList3[1] = Rect(27, 31, 42, 46);
	_rectList3[2] = Rect(56, 31, 71, 46);
	_rectList3[3] = Rect(72, 50, 87, 65);
	_rectList3[4] = Rect(41, 81, 56, 96);

	// Set the palette and change the cursor
	GfxSurface cursor = surfaceFromRes(1, 5, 9);
	BF_GLOBALS._events.setCursor(cursor);

	setPalette();

	// Get the dialog image
	_surface = surfaceFromRes(1, 1, 1);

	// Set the dialog position
	Rect dialogRect;
	dialogRect.resize(_surface, 0, 0, 100);
	dialogRect.center(g_globals->_events._mousePos.x, g_globals->_events._mousePos.y);

	// Ensure the dialog will be entirely on-screen
	Rect screenRect = g_globals->gfxManager()._bounds;
	screenRect.collapse(4, 4);
	dialogRect.contain(screenRect);

	// Load selected button images
	_btnImages.setVisage(1, 2);

	_bounds = dialogRect;
	_gfxManager._bounds = _bounds;

	_highlightedAction = -1;
	_selectedAction = -1;
}

RightClickDialog::~RightClickDialog() {
}

void RightClickDialog::draw() {
	// Save the covered background area
	_savedArea = surfaceGetArea(g_globals->_gfxManagerInstance.getSurface(), _bounds);

	// Draw the dialog image
	g_globals->gfxManager().copyFrom(_surface, _bounds.left, _bounds.top);

	// Pre-process rect lists
	for (int idx = 0; idx < 5; ++idx) {
		_rectList2[idx] = _rectList1[idx];
		_rectList4[idx] = _rectList3[idx];

		_rectList2[idx].translate(_bounds.left, _bounds.top);
		_rectList4[idx].translate(_bounds.left, _bounds.top);
	}
}

bool RightClickDialog::process(Event &event) {
	switch (event.eventType) {
	case EVENT_MOUSE_MOVE: {
		// Check whether a button is highlighted
		int buttonIndex = 0;
		while ((buttonIndex < 5) && !_rectList1[buttonIndex].contains(event.mousePos))
			++buttonIndex;
		if (buttonIndex == 5)
			buttonIndex = -1;

		// If selection has changed, handle it
		if (buttonIndex != _highlightedAction) {
			if (_highlightedAction != -1) {
				// Another button was previously selected, so restore dialog
				_gfxManager.copyFrom(_surface, 0, 0);
			}

			if (buttonIndex != -1) {
				// Draw newly selected button
				GfxSurface btn = _btnImages.getFrame(buttonIndex + 1);
				_gfxManager.copyFrom(btn, _rectList3[buttonIndex].left, _rectList3[buttonIndex].top);
			}

			_highlightedAction = buttonIndex;
		}

		event.handled = true;
		return true;
	}

	case EVENT_BUTTON_DOWN:
		// Specify the selected action
		_selectedAction = (_highlightedAction == -1) ? 5 : _highlightedAction;
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

	// Deactivate the graphics manager used for the dialog
	_gfxManager.deactivate();

	// Execute the specified action
	CursorType cursorNum = CURSOR_NONE;
	switch (_selectedAction) {
	case 0:
		// Walk action
		cursorNum = BF_GLOBALS._player._canWalk ? CURSOR_WALK : CURSOR_USE;
		break;
	case 1:
		// Use action
		cursorNum = CURSOR_USE;
		break;
	case 2:
		// Look action
		cursorNum = CURSOR_LOOK;
		break;
	case 3:
		// Talk action
		cursorNum = CURSOR_TALK;
		break;
	case 4:
		// Options dialog
		BlueForce::OptionsDialog::show();
		break;
	}

	if (cursorNum != CURSOR_NONE)
		BF_GLOBALS._events.setCursor(cursorNum);
}

/*--------------------------------------------------------------------------*/

AmmoBeltDialog::AmmoBeltDialog() : GfxDialog() {
	_cursorNum = BF_GLOBALS._events.getCursor();
	_inDialog = -1;
	_closeFlag = false;

	// Get the dialog image
	_surface = surfaceFromRes(9, 5, 2);

	// Set the dialog position
	_dialogRect.resize(_surface, 0, 0, 100);
	_dialogRect.center(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	_bounds = _dialogRect;
	_gfxManager._bounds = _bounds;
	_savedArea = NULL;

	// Set up area rects
	_gunRect.set(0, 0, 82, 48);
	_clip1Rect.set(90, 6, _bounds.width(), 39);
	_clip2Rect.set(90, 40, _bounds.width(), _bounds.height());
	_loadedRect.set(50, 40, 60, 50);
}

AmmoBeltDialog::~AmmoBeltDialog() {
	BF_GLOBALS._events.setCursor(_cursorNum);
}

void AmmoBeltDialog::execute() {
	// Draw the dialog
	draw();

	// Dialog event handler loop
	_gfxManager.activate();

	while (!g_vm->shouldQuit() && !_closeFlag) {
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
}

bool AmmoBeltDialog::process(Event &event) {
	switch (event.eventType) {
	case EVENT_MOUSE_MOVE: {
		// Handle updating cursor depending on whether cursor is in dialog or not
		int inDialog = Rect(0, 0, _bounds.width(), _bounds.height()).contains(event.mousePos);
		if (inDialog != _inDialog) {
			// Update cursor
			BF_GLOBALS._events.setCursor(inDialog ? CURSOR_USE : CURSOR_EXIT);
			_inDialog = inDialog;
		}
		return true;
	}

	case EVENT_BUTTON_DOWN:
		if (!_inDialog)
			// Clicked outside dialog, so flag to close it
			_closeFlag = true;
		else {
			int v = (BF_GLOBALS.getFlag(fGunLoaded) ? 1 : 0) * (BF_GLOBALS.getFlag(fLoadedSpare) ? 2 : 1);

			// Handle first clip
			if ((v != 1) && _clip1Rect.contains(event.mousePos)) {
				if (BF_GLOBALS.getFlag(fGunLoaded)) {
					event.mousePos.x = event.mousePos.y = 0;
				}

				BF_GLOBALS.setFlag(fGunLoaded);
				BF_GLOBALS.clearFlag(fLoadedSpare);
			}

			// Handle second clip
			if ((v != 2) && _clip2Rect.contains(event.mousePos)) {
				if (BF_GLOBALS.getFlag(fGunLoaded)) {
					event.mousePos.x = event.mousePos.y = 0;
				}

				BF_GLOBALS.setFlag(fGunLoaded);
				BF_GLOBALS.setFlag(fLoadedSpare);
			}

			if (_gunRect.contains(event.mousePos) && BF_GLOBALS.getFlag(fGunLoaded)) {
				BF_GLOBALS.clearFlag(fGunLoaded);
				v = (BF_GLOBALS.getFlag(fGunLoaded) ? 1 : 0) * (BF_GLOBALS.getFlag(fLoadedSpare) ? 2 : 1);

				if (v != 2)
					BF_GLOBALS.clearFlag(fLoadedSpare);
			}

			draw();
		}

		return true;

	case EVENT_KEYPRESS:
		if ((event.kbd.keycode == Common::KEYCODE_ESCAPE) || (event.kbd.keycode == Common::KEYCODE_RETURN)) {
			// Escape pressed, so flag to close dialog
			_closeFlag = true;
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

void AmmoBeltDialog::draw() {
	Rect bounds = _bounds;

	if (!_savedArea) {
		// Save the covered background area
		_savedArea = surfaceGetArea(g_globals->_gfxManagerInstance.getSurface(), _bounds);
	} else {
		bounds.moveTo(0, 0);
	}

	// Draw the dialog image
	g_globals->gfxManager().copyFrom(_surface, bounds.left, bounds.top);

	// Setup clip flags
	bool clip1 = true, clip2 = true;
	bool gunLoaded = BF_GLOBALS.getFlag(fGunLoaded);
	if (gunLoaded) {
		// A clip is currently loaded. Hide the appropriate clip
		if (BF_GLOBALS.getFlag(fLoadedSpare))
			clip2 = false;
		else
			clip1 = false;
	}

	// Draw the first clip if necessary
	if (clip1) {
		GfxSurface clipSurface = surfaceFromRes(9, 6, BF_GLOBALS._clip1Bullets + 1);
		_clip1Rect.resize(clipSurface, _clip1Rect.left, _clip1Rect.top, 100);
		g_globals->gfxManager().copyFrom(clipSurface, bounds.left + _clip1Rect.left,
			bounds.top + _clip1Rect.top);
	}

	// Draw the second clip if necessary
	if (clip2) {
		GfxSurface clipSurface = surfaceFromRes(9, 6, BF_GLOBALS._clip2Bullets + 1);
		_clip2Rect.resize(clipSurface, _clip2Rect.left, _clip2Rect.top, 100);
		g_globals->gfxManager().copyFrom(clipSurface, bounds.left + _clip2Rect.left,
			bounds.top + _clip2Rect.top);
	}

	// If a clip is loaded, draw the 'loaded' portion of the gun
	if (gunLoaded) {
		GfxSurface loadedSurface = surfaceFromRes(9, 7, 1);
		_loadedRect.resize(loadedSurface, _loadedRect.left, _loadedRect.top, 100);
		g_globals->gfxManager().copyFrom(loadedSurface, bounds.left + _loadedRect.left,
			bounds.top + _loadedRect.top);
	}
}

/*--------------------------------------------------------------------------*/

RadioConvDialog::RadioConvDialog() : GfxDialog() {
	int idx;

	// Set up the list of buttons
	int maxWidth = 0;
	for (idx = 0; idx < 8; ++idx) {
		_buttons[idx].setText(RADIO_BTN_LIST[idx]);
		maxWidth = MAX(maxWidth, (int)_buttons[idx]._bounds.width());

		add(&_buttons[idx]);
	}

	// Set up the button positions and add them to the dialog
	for (idx = 0; idx < 8; ++idx) {
		_buttons[idx]._bounds.moveTo((idx % 2) * maxWidth + 2,
				idx / 2 * _buttons[idx]._bounds.height() + 2);
		_buttons[idx]._bounds.setWidth(maxWidth);

		add(&_buttons[idx]);
	}

	// Set the dialog size and position
	setDefaults();
	setTopLeft(8, 92);

	BF_GLOBALS._events.setCursor(CURSOR_ARROW);
}

RadioConvDialog::~RadioConvDialog() {
	BF_GLOBALS._events.setCursor(CURSOR_WALK);
}

int RadioConvDialog::execute() {
	GfxButton *btn = GfxDialog::execute();

	// Get which button was pressed
	int btnIndex = -1;
	for (int idx = 0; idx < 8; ++idx) {
		if (btn == &_buttons[idx]) {
			btnIndex = idx;
			break;
		}
	}

	return btnIndex;
}

int RadioConvDialog::show() {
	// Show the dialog
	RadioConvDialog *dlg = new RadioConvDialog();
	dlg->draw();

	int btnIndex = dlg->execute();

	// Close the dialog
	dlg->remove();
	delete dlg;

	return btnIndex;
}

/*--------------------------------------------------------------------------*/

void OptionsDialog::show() {
	OptionsDialog *dlg = new OptionsDialog();
	dlg->draw();

	// Show the dialog
	GfxButton *btn = dlg->execute();

	// Get which button was pressed
	int btnIndex = -1;
	if (btn == &dlg->_btnRestore)
		btnIndex = 0;
	else if (btn == &dlg->_btnSave)
		btnIndex = 1;
	else if (btn == &dlg->_btnRestart)
		btnIndex = 2;
	else if (btn == &dlg->_btnQuit)
		btnIndex = 3;
	else if (btn == &dlg->_btnSound)
		btnIndex = 4;

	// Close the dialog
	dlg->remove();
	delete dlg;

	// Execute the given selection
	if (btnIndex == 0) {
		// Restore button
		g_globals->_game->restoreGame();
	} else if (btnIndex == 1) {
		// Save button
		g_globals->_game->saveGame();
	} else if (btnIndex == 2) {
		// Restart game
		g_globals->_game->restartGame();
	} else if (btnIndex == 3) {
		// Quit game
		if (MessageDialog::show(QUIT_CONFIRM_MSG, CANCEL_BTN_STRING, QUIT_BTN_STRING) == 1) {
			g_vm->quitGame();
		}
	} else if (btnIndex == 4) {
		// Sound dialog
		SoundDialog::execute();
	}
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
	_bounds.collapse(-6, -6);
	setCenter(160, 90);
}

} // End of namespace BlueForce

} // End of namespace TsAGE
