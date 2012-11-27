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
#include "tsage/ringworld/ringworld_logic.h"

namespace TsAGE {

/*--------------------------------------------------------------------------*/

/**
 * This dialog class provides a simple message display with support for either one or two buttons.
 */
MessageDialog::MessageDialog(const Common::String &message, const Common::String &btn1Message,
							 const Common::String &btn2Message) : GfxDialog() {
	// Set up the message
	addElements(&_msg, &_btn1, NULL);

	_msg.set(message, 200, ALIGN_LEFT);
	_msg._bounds.moveTo(0, 0);
	_defaultButton = &_btn1;

	// Set up the first button
	_btn1.setText(btn1Message);
	_btn1._bounds.moveTo(_msg._bounds.right - _btn1._bounds.width(), _msg._bounds.bottom + 2);

	if (!btn2Message.empty()) {
		// Set up the second button
		add(&_btn2);
		_btn2.setText(btn2Message);
		_btn2._bounds.moveTo(_msg._bounds.right - _btn2._bounds.width(), _msg._bounds.bottom + 2);
		_btn1._bounds.translate(-(_btn2._bounds.width() + 4), 0);
	}

	// Do post setup for the dialog
	setDefaults();

	// Set the dialog's center
	setCenter(g_globals->_dialogCenter.x, g_globals->_dialogCenter.y);
}

int MessageDialog::show(const Common::String &message, const Common::String &btn1Message, const Common::String &btn2Message) {
	// Ensure that the cursor is the arrow
	g_globals->_events.setCursor(CURSOR_ARROW);

	int result = show2(message, btn1Message, btn2Message);

	g_globals->_events.setCursorFromFlag();
	return result;
}

int MessageDialog::show2(const Common::String &message, const Common::String &btn1Message, const Common::String &btn2Message) {
	MessageDialog *dlg = new MessageDialog(message, btn1Message, btn2Message);
	dlg->draw();

	GfxButton *defaultButton = !btn2Message.empty() ? &dlg->_btn2 : &dlg->_btn1;
	GfxButton *selectedButton = dlg->execute(defaultButton);
	int result =  (selectedButton == defaultButton) ? 1 : 0;

	delete dlg;
	return result;
}

/*--------------------------------------------------------------------------*/

ConfigDialog::ConfigDialog() : GUI::OptionsDialog("", "GlobalConfig") {
	//
	// Sound controllers
	//

	addVolumeControls(this, "GlobalConfig.");
	setVolumeSettingsState(true); // could disable controls by GUI options

	//
	// Add the buttons
	//

	new GUI::ButtonWidget(this, "GlobalConfig.Ok", _("~O~K"), 0, GUI::kOKCmd);
	new GUI::ButtonWidget(this, "GlobalConfig.Cancel", _("~C~ancel"), 0, GUI::kCloseCmd);
}

/*--------------------------------------------------------------------------*/

void ModalDialog::draw() {
	// Set the palette for use in the dialog
	setPalette();

	// Make a backup copy of the area the dialog will occupy
	Rect tempRect = _bounds;
	tempRect.collapse(-10, -10);
	_savedArea = surfaceGetArea(g_globals->_gfxManagerInstance.getSurface(), tempRect);

	_gfxManager.activate();

	// Fill in the contents of the entire dialog
	_gfxManager._bounds = Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	drawFrame();

	// Draw each element in the dialog in order
	GfxElementList::iterator i;
	for (i = _elements.begin(); i != _elements.end(); ++i) {
		(*i)->draw();
	}

	_gfxManager.deactivate();
}

void ModalDialog::drawFrame() {
	Rect origRect = _bounds;
	_bounds.collapse(-10, -10);

	// Fill the dialog area
	g_globals->gfxManager().fillRect(origRect, 54);

	// Draw top line
	GfxSurface surface = surfaceFromRes(8, 1, 7);
	for (int xp = _bounds.left + 10; xp < (_bounds.right - 20); xp += 10)
		surface.draw(Common::Point(xp, _bounds.top));
	surface.draw(Common::Point(_bounds.right - 20, _bounds.top));

	surface = surfaceFromRes(8, 1, 1);
	surface.draw(Common::Point(_bounds.left, _bounds.top));

	surface = surfaceFromRes(8, 1, 4);
	surface.draw(Common::Point(_bounds.right - 10, _bounds.top));

	// Draw vertical edges
	surface = surfaceFromRes(8, 1, 2);
	for (int yp = _bounds.top + 10; yp < (_bounds.bottom - 20); yp += 10)
		surface.draw(Common::Point(_bounds.left, yp));
	surface.draw(Common::Point(_bounds.left, _bounds.bottom - 20));

	surface = surfaceFromRes(8, 1, 5);
	for (int yp = _bounds.top + 10; yp < (_bounds.bottom - 20); yp += 10)
		surface.draw(Common::Point(_bounds.right - 10, yp));
	surface.draw(Common::Point(_bounds.right - 10, _bounds.bottom - 20));

	// Draw bottom line
	surface = surfaceFromRes(8, 1, 8);
	for (int xp = _bounds.left + 10; xp < (_bounds.right - 20); xp += 10)
		surface.draw(Common::Point(xp, _bounds.bottom - 10));
	surface.draw(Common::Point(_bounds.right - 20, _bounds.bottom - 10));

	surface = surfaceFromRes(8, 1, 3);
	surface.draw(Common::Point(_bounds.left, _bounds.bottom - 10));

	surface = surfaceFromRes(8, 1, 6);
	surface.draw(Common::Point(_bounds.right - 10, _bounds.bottom - 10));

	// Set the dialog's manager bounds
	_gfxManager._bounds = origRect;
}

/*--------------------------------------------------------------------------*/

void SoundDialog::execute() {
	ConfigDialog *dlg = new ConfigDialog();
	dlg->runModal();
	delete dlg;
	g_globals->_soundManager.syncSounds();
	g_globals->_events.setCursorFromFlag();
}

} // End of namespace TsAGE
